#include "AVStreamPlayer.h"

#include <pbnjson.hpp>

#ifdef USE_SDL_WEBOS

#include <SDL.h>

#endif

// 2MB decode size should be fairly enough for everything
#define DECODER_BUFFER_SIZE 2048 * 1024

using SMP_DECODER_NS::AVStreamPlayer;
using SMP_DECODER_NS::AVStreamPlayerFactory;
namespace pj = pbnjson;

std::string base64_encode(const unsigned char *src, size_t len);

AVStreamPlayer::AVStreamPlayer() : videoConfig({}), audioConfig({}), player_state_(PlayerState::UNINITIALIZED),
                                   video_pts_(0), request_interrupt_(false) {
    app_id_ = getenv("APPID");
    video_buffer_ = (char *) malloc(DECODER_BUFFER_SIZE);
#ifdef USE_ACB
    acb_client_.reset(new Acb());
    if (!acb_client_)
        return;
    auto acbHandler = std::bind(&AVStreamPlayer::AcbHandler, this, acb_client_->getAcbId(),
                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                std::placeholders::_4, std::placeholders::_5);
    if (!acb_client_->initialize(ACB::PlayerType::MSE, app_id_, acbHandler)) {
        Log(SS4S_LogLevelError, "SMP", "Acb::initialize() failed!");
        return;
    }
#endif

#ifdef USE_SDL_WEBOS
    window_id_ = SDL_webOSCreateExportedWindow(0);
#endif
    player_state_ = PlayerState::UNLOADED;
}

AVStreamPlayer::~AVStreamPlayer() {
#ifdef USE_ACB
    acb_client_->setState(ACB::AppState::FOREGROUND, ACB::PlayState::UNLOADED);
    acb_client_->finalize();
#endif

    starfish_media_apis_.reset(nullptr);
    free(video_buffer_);
#ifdef USE_SDL_WEBOS
    SDL_webOSDestroyExportedWindow(window_id_);
#endif
}

bool AVStreamPlayer::load() {
    if (player_state_ != PlayerState::UNLOADED) {
        if (player_state_ == PlayerState::PLAYING) {
            sendEOS();
            starfish_media_apis_->Unload();
        }
        player_state_ = PlayerState::UNLOADED;
#ifdef USE_ACB
        acb_client_->setState(ACB::AppState::FOREGROUND, ACB::PlayState::UNLOADED);
#endif
    }
    starfish_media_apis_.reset(new StarfishMediaAPIs());
    starfish_media_apis_->notifyForeground();
    std::string payload = makeLoadPayload(videoConfig, audioConfig, video_pts_);
    Log(SS4S_LogLevelDebug, "SMP", "StarfishMediaAPIs::Load(%s)", payload.c_str());
    if (!starfish_media_apis_->Load(payload.c_str(), &LoadCallback, this)) {
        Log(SS4S_LogLevelError, "SMP", "StarfishMediaAPIs::Load() failed!");
        return false;
    }
    player_state_ = PlayerState::LOADED;

#ifdef USE_ACB
    if (!acb_client_->setDisplayWindow(0, 0, videoConfig.width, videoConfig.height, true)) {
        Log(SS4S_LogLevelError, "SMP", "Acb::setDisplayWindow() failed!");
        return false;
    }
#endif

#ifdef USE_SDL_WEBOS
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    SDL_Rect src = {0, 0, videoConfig.width, videoConfig.height};
    SDL_Rect dst = {0, 0, dm.w, dm.h};
    SDL_webOSSetExportedWindow(window_id_, &src, &dst);
#endif
    return true;
}

SS4S_VideoFeedResult AVStreamPlayer::submitVideo(const unsigned char *sampleData, size_t sampleLength,
                                                 SS4S_VideoFeedFlags flags) {
//    if (request_interrupt_) {
//        return DR_INTERRUPT;
//    }
//    if (decodeUnit->fullLength > DECODER_BUFFER_SIZE) {
//        Log(SS4S_LogLevelWarn, "SMP", "Video decode buffer too small, skip this frame");
//        return DR_NEED_IDR;
//    }
//    unsigned long long ms = 0;
//    // In nanoseconds.
//    video_pts_ = ms * 1000000ULL;
//
//    if (!submitBuffer(sampleData, sampleLength, video_pts_, 1))
//        return DR_NEED_IDR;
//
//    return DR_OK;
    return SS4S_VIDEO_FEED_OK;
}

void AVStreamPlayer::submitAudio(const unsigned char *sampleData, size_t sampleLength) {
    submitBuffer(sampleData, sampleLength, 0, 2);
}

bool AVStreamPlayer::submitBuffer(const void *data, size_t size, uint64_t pts, int esData) {
    if (player_state_ == EOS) {
        // Player has marked as end of stream, ignore all data
        return true;
    } else if (player_state_ != LOADED && player_state_ != PLAYING) {
        Log(SS4S_LogLevelError, "SMP", "Player not ready to feed");
        return false;
    }
    char payload[256];
    snprintf(payload, sizeof(payload), R"({"bufferAddr":"%p","bufferSize":%u,"pts":%llu,"esData":%d})",
             data, size, pts, esData);
    std::string result = starfish_media_apis_->Feed(payload);
    std::size_t found = result.find(std::string("Ok"));
    if (found == std::string::npos) {
        found = result.find(std::string("BufferFull"));
        if (found != std::string::npos) {
            Log(SS4S_LogLevelWarn, "SMP", "Buffer is full");
            return true;
        }
        Log(SS4S_LogLevelWarn, "SMP", "Buffer submit returned error: %s", result.c_str());
        return false;
    }
    if (player_state_ == LOADED) {
#ifdef USE_ACB
        if (!acb_client_->setState(ACB::AppState::FOREGROUND, ACB::PlayState::SEAMLESS_LOADED))
            Log(SS4S_LogLevelError, "SMP", "Acb::setState(FOREGROUND, SEAMLESS_LOADED) failed!");
#endif
        player_state_ = PLAYING;
    }
    return true;
}

void AVStreamPlayer::sendEOS() {
    if (player_state_ != PLAYING)
        return;
    player_state_ = PlayerState::EOS;
    starfish_media_apis_->pushEOS();
}

void AVStreamPlayer::setHdr(bool hdr) {
    hdr_ = hdr;
}

std::string AVStreamPlayer::makeLoadPayload(SS4S_VideoInfo &vidCfg, SS4S_AudioInfo &audCfg, uint64_t time) {
    pj::JValue payload = pj::Object();
    pj::JValue args = pj::Array();
    pj::JValue arg = pj::Object();
    pj::JValue option = pj::Object();
    pj::JValue contents = pj::Object();
    pj::JValue bufferingCtrInfo = pj::Object();
    pj::JValue externalStreamingInfo = pj::Object();
    pj::JValue codec = pj::Object();
    pj::JValue videoSink = pj::Object();
    pj::JValue avSink = pj::Object();
    videoSink.put("type", "main_video");

    avSink.put("videoSink", videoSink);

    arg.put("mediaTransportType", "BUFFERSTREAM");
    pj::JValue adaptiveStreaming = pj::Object();
    adaptiveStreaming.put("maxWidth", vidCfg.width);
    adaptiveStreaming.put("maxHeight", vidCfg.height);
    adaptiveStreaming.put("maxFrameRate", 60 /* TODO: add this value */);

    switch (vidCfg.codec) {
        case SS4S_VIDEO_H264:
            codec.put("video", "H264");
            break;
        case SS4S_VIDEO_H265:
            codec.put("video", "H265");
            break;
        default:
            return "";
    }

    if (audCfg.codec == SS4S_AUDIO_OPUS) {
        pj::JValue audioSink = pj::Object();
        audioSink.put("type", "main_sound");
        avSink.put("audioSink", audioSink);
        codec.put("audio", "OPUS");
        pj::JValue opusInfo = pj::Object();
        opusInfo.put("channels", std::to_string(audCfg.numOfChannels));
        opusInfo.put("sampleRate", static_cast<double>(audCfg.sampleRate) / 1000.0);
        if (audCfg.codecData != nullptr) {
            opusInfo.put("streamHeader", base64_encode(audCfg.codecData, audCfg.codecDataLen));
        }
        contents.put("opusInfo", opusInfo);
    }
    contents.put("codec", codec);

    pj::JValue esInfo = pj::Object();
    esInfo.put("ptsToDecode", static_cast<int64_t>(time));
    esInfo.put("seperatedPTS", true);
    esInfo.put("pauseAtDecodeTime", true);
    contents.put("esInfo", esInfo);

    contents.put("format", "RAW");
    // contents.put("provider", "Chrome");

    bufferingCtrInfo.put("bufferMaxLevel", 0);
    bufferingCtrInfo.put("bufferMinLevel", 0);
    bufferingCtrInfo.put("preBufferByte", 0);
    bufferingCtrInfo.put("qBufferLevelAudio", 0);
    bufferingCtrInfo.put("qBufferLevelVideo", 0);

    pj::JValue srcBufferLevelAudio = pj::Object();
    srcBufferLevelAudio.put("maximum", 10);
    srcBufferLevelAudio.put("minimum", 1);
    bufferingCtrInfo.put("srcBufferLevelAudio", srcBufferLevelAudio);

    pj::JValue srcBufferLevelVideo = pj::Object();
    srcBufferLevelVideo.put("maximum", DECODER_BUFFER_SIZE);
    srcBufferLevelVideo.put("minimum", 1);
    bufferingCtrInfo.put("srcBufferLevelVideo", srcBufferLevelVideo);

    externalStreamingInfo.put("contents", contents);
    externalStreamingInfo.put("bufferingCtrInfo", bufferingCtrInfo);

    // Must have this on webOS 4
    pj::JValue transmission = pj::JObject();
    transmission.put("contentsType", "WEBRTC");

    option.put("adaptiveStreaming", adaptiveStreaming);
    option.put("appId", app_id_);
    option.put("externalStreamingInfo", externalStreamingInfo);
    // Seems useful on webOS 5+
    option.put("lowDelayMode", true);
    option.put("transmission", transmission);
    option.put("needAudio", audCfg.codec != SS4S_AUDIO_NONE);
#ifdef USE_SDL_WEBOS
    option.put("windowId", window_id_);
#endif

    arg.put("option", option);
    args.append(arg);
    payload.put("args", args);

    return pbnjson::JGenerator::serialize(payload, pbnjson::JSchemaFragment("{}"));
}

void AVStreamPlayer::SetMediaAudioData(const char *data) {
    Log(SS4S_LogLevelDebug, "SMP", "AVStreamPlayer::SetMediaAudioData %s", data);
}

void AVStreamPlayer::SetMediaVideoData(const char *data) {
    pj::JSchema schema = pj::JSchema::AllSchema();
    pj::JValue parsed = pj::JDomParser::fromString(data, schema);
    pj::JValue video = parsed["video"];
    if (hdr_ && video["hdrType"].asString() != "HDR10") {
        video.put("hdrType", "HDR10");
        Log(SS4S_LogLevelInfo, "SMP", "Add missing HDR type");
    }
    const std::string &modified = pj::JGenerator::serialize(parsed, schema);

    Log(SS4S_LogLevelDebug, "SMP", "AVStreamPlayer::SetMediaVideoData %s", modified.c_str());
#ifdef USE_ACB
    acb_client_->setMediaVideoData(modified.c_str());
#endif
}

void AVStreamPlayer::LoadCallback(int type, int64_t numValue, const char *strValue) {
    switch (type) {
        case 0:
            break;
        case PF_EVENT_TYPE_STR_ERROR:
            Log(SS4S_LogLevelWarn, "SMP", "LoadCallback PF_EVENT_TYPE_STR_ERROR, numValue: %lld, strValue: %p\n",
                numValue, strValue);
            break;
        case PF_EVENT_TYPE_INT_ERROR: {
            Log(SS4S_LogLevelWarn, "SMP", "LoadCallback PF_EVENT_TYPE_INT_ERROR, numValue: %lld, strValue: %p\n",
                numValue, strValue);
            if (player_state_ == PLAYING) {
                request_interrupt_ = true;
            }
            break;
        }
        case PF_EVENT_TYPE_STR_BUFFERFULL:
            Log(SS4S_LogLevelWarn, "SMP", "LoadCallback PF_EVENT_TYPE_STR_BUFFERFULL\n");
            break;
        case PF_EVENT_TYPE_STR_STATE_UPDATE__LOADCOMPLETED:
#ifdef USE_ACB
            acb_client_->setSinkType(ACB::StateSinkType::SINK_AUTO);
            acb_client_->setMediaId(starfish_media_apis_->getMediaID());
            acb_client_->setState(ACB::AppState::FOREGROUND, ACB::PlayState::LOADED);
#endif
            starfish_media_apis_->Play();
            break;
        case PF_EVENT_TYPE_STR_STATE_UPDATE__PLAYING:
            break;
        case PF_EVENT_TYPE_STR_AUDIO_INFO:
            SetMediaAudioData(strValue);
            break;
        case PF_EVENT_TYPE_STR_VIDEO_INFO:
            SetMediaVideoData(strValue);
            break;
        case PF_EVENT_TYPE_INT_SVP_VDEC_READY:
            break;
        default:
            Log(SS4S_LogLevelWarn, "SMP", "LoadCallback unhandled 0x%02x\n", type);
            break;
    }
}

#ifdef USE_ACB

void AVStreamPlayer::AcbHandler(long acb_id, long task_id, long event_type, long app_state, long play_state,
                                const char *reply) {
    Log(SS4S_LogLevelDebug, "SMP",
        "AcbHandler acbId = %ld, taskId = %ld, eventType = %ld, appState = %ld,playState = %ld, reply = %s EOL\n",
        acb_id, task_id, event_type, app_state, play_state, reply);
}

#endif

void AVStreamPlayer::LoadCallback(int type, int64_t numValue, const char *strValue, void *data) {
    static_cast<AVStreamPlayer *>(data)->LoadCallback(type, numValue, strValue);
}
