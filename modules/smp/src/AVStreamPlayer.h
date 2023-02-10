#pragma once

#include <memory>

#include <StarfishMediaAPIs.h>
#include "ss4s/modapi.h"

#ifdef USE_ACB

#include <Acb.h>

#endif

namespace SMP_DECODER_NS {
    static SS4S_LoggingFunction *Log = nullptr;

    class AVStreamPlayer;

    struct AVStreamPlayerFactory;

    class AVStreamPlayer {
    public:
        AVStreamPlayer();

        ~AVStreamPlayer();

        bool load();

        SS4S_VideoFeedResult submitVideo(const unsigned char *sampleData, size_t sampleLength,
                                         SS4S_VideoFeedFlags flags);

        void submitAudio(const unsigned char *sampleData, size_t sampleLength);

        void sendEOS();

        void setHdr(bool hdr);

        SS4S_VideoInfo videoConfig;
        SS4S_AudioInfo audioConfig;

        static const AVStreamPlayerFactory PlayerFactory;

    private:
        enum PlayerState {
            UNINITIALIZED,
            UNLOADED,
            LOADED,
            PLAYING,
            EOS,
        };

        std::string makeLoadPayload(SS4S_VideoInfo &vidCfg, SS4S_AudioInfo &audCfg, uint64_t time);

        bool submitBuffer(const void *data, size_t size, uint64_t pts, int esData);

        void SetMediaAudioData(const char *data);

        void SetMediaVideoData(const char *data);

        void LoadCallback(int type, int64_t numValue, const char *strValue);

        static void LoadCallback(int type, int64_t numValue, const char *strValue, void *data);

        std::unique_ptr<StarfishMediaAPIs> starfish_media_apis_;
        std::string app_id_;
        PlayerState player_state_;
        char *video_buffer_;
        unsigned long long video_pts_;
        bool request_interrupt_;
        bool hdr_;
#ifdef USE_ACB

        void AcbHandler(long acb_id, long task_id, long event_type, long app_state, long play_state, const char *reply);

        std::unique_ptr<Acb> acb_client_;
#endif
#ifdef USE_SDL_WEBOS
        const char *window_id_;
#endif
    };

}

struct SS4S_PlayerContext {
public:
    SMP_DECODER_NS::AVStreamPlayer *player;
};
