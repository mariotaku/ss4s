#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#ifdef HAVE_LIBAVUTIL
#include <libavutil/frame.h>
#include <libavutil/hwcontext.h>
#include <libavutil/pixfmt.h>
#endif

#include "esplayer-datasrc.h"

#include "ss4s.h"
#include "os_info.h"
#include "ss4s_modules.h"
#include "array_list.h"

static SS4S_Player *player = NULL;

/* Set true when the selected video module decodes in-process and we
 * have to display frames via SDL ourselves. Set in main(); read by
 * videoPreroll to decide whether to register a frame callback. */
static bool render_self = false;

/* Single-slot frame queue. Callback writes; main thread reads. If the
 * slot already holds an unrendered frame when a new one arrives, the
 * older frame is Released and replaced — keeps latency bounded at the
 * cost of dropping frames under back-pressure. */
static SDL_mutex *frame_mtx = NULL;
static SS4S_VideoOutputFrame pending_frame;
static bool pending_has = false;

/* Render-side state, owned by the main thread. */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    Uint32 tex_fmt;
    int tex_w, tex_h;
#ifdef HAVE_LIBAVUTIL
    AVFrame *hw_dst;
#endif
} render_state_t;

static void frame_cb(const SS4S_VideoOutputFrame *frame, void *userdata) {
    (void) userdata;
    SS4S_VideoOutputFrame keep = *frame;
    if (!SS4S_VideoFrameRetain(&keep)) {
        return;
    }
    SDL_LockMutex(frame_mtx);
    if (pending_has) {
        SS4S_VideoFrameRelease(&pending_frame);
    }
    pending_frame = keep;
    pending_has = true;
    SDL_UnlockMutex(frame_mtx);
}

static bool ensure_texture(render_state_t *r, Uint32 fmt, int w, int h) {
    if (r->texture != NULL && r->tex_fmt == fmt && r->tex_w == w && r->tex_h == h) {
        return true;
    }
    if (r->texture != NULL) {
        SDL_DestroyTexture(r->texture);
        r->texture = NULL;
    }
    r->texture = SDL_CreateTexture(r->renderer, fmt, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (r->texture == NULL) {
        SDL_Log("SDL_CreateTexture: %s", SDL_GetError());
        return false;
    }
    r->tex_fmt = fmt;
    r->tex_w = w;
    r->tex_h = h;
    return true;
}

static void present(render_state_t *r) {
    SDL_SetRenderDrawColor(r->renderer, 0, 0, 0, 0);
    SDL_RenderClear(r->renderer);
    SDL_RenderCopy(r->renderer, r->texture, NULL, NULL);
    SDL_RenderPresent(r->renderer);
}

static void render_yuv_i420(render_state_t *r, uint8_t *const *data, const int *linesize, int w, int h) {
    if (!ensure_texture(r, SDL_PIXELFORMAT_IYUV, w, h)) {
        return;
    }
    SDL_UpdateYUVTexture(r->texture, NULL,
                         data[0], linesize[0],
                         data[1], linesize[1],
                         data[2], linesize[2]);
    present(r);
}

#ifdef HAVE_LIBAVUTIL
static void render_avframe(render_state_t *r, struct AVFrame *src) {
    if (r->hw_dst == NULL) {
        r->hw_dst = av_frame_alloc();
        if (r->hw_dst == NULL) {
            return;
        }
    }
    if (av_hwframe_transfer_data(r->hw_dst, src, 0) < 0) {
        SDL_Log("av_hwframe_transfer_data failed");
        return;
    }
    AVFrame *f = r->hw_dst;
    if (f->format == AV_PIX_FMT_YUV420P) {
        render_yuv_i420(r, f->data, f->linesize, f->width, f->height);
    } else if (f->format == AV_PIX_FMT_NV12) {
#if SDL_VERSION_ATLEAST(2, 0, 16)
        if (ensure_texture(r, SDL_PIXELFORMAT_NV12, f->width, f->height)) {
            SDL_UpdateNVTexture(r->texture, NULL,
                                f->data[0], f->linesize[0],
                                f->data[1], f->linesize[1]);
            present(r);
        }
#else
        SDL_Log("NV12 frames require SDL >= 2.0.16");
#endif
    } else {
        SDL_Log("unsupported hwframe pixel format %d", f->format);
    }
    av_frame_unref(f);
}
#endif

static void render_pending(render_state_t *r) {
    SS4S_VideoOutputFrame f;
    bool have;
    SDL_LockMutex(frame_mtx);
    have = pending_has;
    if (have) {
        f = pending_frame;
        pending_has = false;
    }
    SDL_UnlockMutex(frame_mtx);
    if (!have) {
        return;
    }
    if (f.format == SS4S_VIDEO_OUTPUT_FORMAT_YUV) {
        render_yuv_i420(r, f.yuv.data, f.yuv.linesize, f.yuv.width, f.yuv.height);
    }
#ifdef HAVE_LIBAVUTIL
    else if (f.format == SS4S_VIDEO_OUTPUT_FORMAT_AVFRAME) {
        render_avframe(r, f.avframe.frame);
    }
#endif
    SS4S_VideoFrameRelease(&f);
}

static void drain_pending(void) {
    SDL_LockMutex(frame_mtx);
    if (pending_has) {
        SS4S_VideoFrameRelease(&pending_frame);
        pending_has = false;
    }
    SDL_UnlockMutex(frame_mtx);
}

int videoPreroll(int width, int height, int framerate) {
    (void) framerate;
    SS4S_VideoInfo info = {
            .codec = SS4S_VIDEO_H264,
            .width = width,
            .height = height,
    };
    SS4S_VideoOpenResult result = SS4S_PlayerVideoOpen(player, &info);
    if (result != SS4S_VIDEO_OPEN_OK) {
        return result;
    }
    if (render_self) {
        SS4S_PlayerVideoSetFrameCallback(player, frame_cb, NULL);
    } else {
        SS4S_VideoRect src = {0, 0, width, 840 * height / 1080};
        SS4S_VideoRect dst = {0, 0, 1920, 840};
        SS4S_PlayerVideoSetDisplayArea(player, &src, &dst);
    }
    return result;
}

int videoSample(const void *data, size_t size, int flags) {
    SS4S_VideoFeedFlags vflags = 0;
    if (flags & VIDEO_FLAG_FRAME_START) {
        vflags |= SS4S_VIDEO_FEED_DATA_FRAME_START;
    }
    if (flags & VIDEO_FLAG_FRAME_END) {
        vflags |= SS4S_VIDEO_FEED_DATA_FRAME_END;
    }
    if (flags & VIDEO_FLAG_FRAME_KEYFRAME) {
        vflags |= SS4S_VIDEO_FEED_DATA_KEYFRAME;
    }
    return SS4S_PlayerVideoFeed(player, data, size, vflags);
}

void videoEos() {
    SS4S_PlayerVideoClose(player);
}

int audioPreroll(int channels, int sampleFreq) {
    SS4S_AudioInfo info = {
            .codec = SS4S_AUDIO_PCM_S16LE,
            .numOfChannels = channels,
            .sampleRate = sampleFreq,
            .samplesPerFrame = 240,
    };
    return SS4S_PlayerAudioOpen(player, &info);
}

int audioSample(const void *data, size_t size) {
    return SS4S_PlayerAudioFeed(player, data, size);
}

void audioEos() {
    SS4S_PlayerAudioClose(player);
}

void pipelineQuit(int error) {
    (void) error;
    SDL_Event quit = {SDL_QUIT};
    SDL_PushEvent(&quit);
}

int main(int argc, char *argv[]) {
    datasrc_init(argc, argv);

    os_info_t os_info = {0};
    os_info_get(&os_info);
    array_list_t modules = {0};
    if (SS4S_ModulesList(&modules, &os_info) != 0) {
        fprintf(stderr, "Failed to list SS4S modules\n");
        return 1;
    }
    SS4S_ModulePreferences preferences = {
            .audio_module = getenv("SS4S_AUDIO_DRIVER"),
            .video_module = getenv("SS4S_VIDEO_DRIVER"),
    };
    SS4S_ModuleSelection selected = {0};
    if (!SS4S_ModulesSelect(&modules, &preferences, &selected, true)) {
        fprintf(stderr, "No suitable SS4S modules available\n");
        SS4S_ModulesListClear(&modules);
        return 1;
    }
    fprintf(stderr, "Selected audio module: %s\n", SS4S_ModuleInfoGetName(selected.audio_module));
    fprintf(stderr, "Selected video module: %s\n", SS4S_ModuleInfoGetName(selected.video_module));

    SS4S_Config config = {
            .audioDriver = SS4S_ModuleInfoGetId(selected.audio_module),
            .videoDriver = SS4S_ModuleInfoGetId(selected.video_module),
            .loggingFunction = SS4S_DefaultLoggingFunction(),
    };

    SDL_Init(SDL_INIT_VIDEO);

    if (SS4S_Init(argc, argv, &config) != 0) {
        SS4S_ModulesListClear(&modules);
        return 1;
    }

    SS4S_VideoCapabilities caps = {0};
    SS4S_GetVideoCapabilities(&caps);
    render_self = (caps.output & SS4S_VIDEO_CAP_OUTPUT_DIRECT) == 0;

    render_state_t rs = {0};
    Uint32 window_flags = render_self ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN;
    rs.window = SDL_CreateWindow("SS4S", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 1280, 720, window_flags);
    rs.renderer = SDL_CreateRenderer(rs.window, -1,
                                     SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    frame_mtx = SDL_CreateMutex();

    SS4S_PostInit(argc, argv);
    player = SS4S_PlayerOpen();
    struct DATASRC_CALLBACKS dscb = {
            .videoPreroll = videoPreroll,
            .videoSample = videoSample,
            .videoEos = videoEos,
            .audioPreroll = audioPreroll,
            .audioSample = audioSample,
            .audioEos = audioEos,
            .pipelineQuit = pipelineQuit,
    };
    datasrc_start(&dscb);
    while (!SDL_QuitRequested()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                datasrc_stop();
                continue;
            }
        }
        if (render_self) {
            render_pending(&rs);
        } else {
            SDL_SetRenderDrawColor(rs.renderer, 0, 0, 0, 0);
            SDL_RenderClear(rs.renderer);
            SDL_RenderPresent(rs.renderer);
            SDL_Delay(16);
        }
    }

    SS4S_PlayerClose(player);
    player = NULL;

    drain_pending();
    if (rs.texture != NULL) {
        SDL_DestroyTexture(rs.texture);
    }
#ifdef HAVE_LIBAVUTIL
    if (rs.hw_dst != NULL) {
        av_frame_free(&rs.hw_dst);
    }
#endif
    SDL_DestroyMutex(frame_mtx);
    SDL_DestroyRenderer(rs.renderer);
    SDL_DestroyWindow(rs.window);

    datasrc_destroy();

    SS4S_Quit();

    SS4S_ModulesListClear(&modules);

    SDL_Quit();
}
