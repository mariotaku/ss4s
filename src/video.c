#include <assert.h>
#include "ss4s.h"
#include "library.h"

bool SS4S_GetVideoCapabilities(SS4S_VideoCapabilities *capabilities) {
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL || driver->GetCapabilities == NULL) {
        return false;
    }
    return driver->GetCapabilities(capabilities);
}

SS4S_VideoCodec SS4S_GetVideoPreferredCodecs(const SS4S_VideoInfo *info) {
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL || driver->GetPreferredCodecs == NULL) {
        return SS4S_VIDEO_NONE;
    }
    return driver->GetPreferredCodecs(info);
}

SS4S_VideoOpenResult SS4S_PlayerVideoOpen(SS4S_Player *player, const SS4S_VideoInfo *info) {
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL) {
        return SS4S_VIDEO_OPEN_ERROR;
    }
    assert(driver->Open != NULL);
    SS4S_MutexLock(player->mutex);
    SS4S_VideoExtraInfo extraInfo = {
            .viewportWidth = player->viewportWidth,
            .viewportHeight = player->viewportHeight,
    };
    SS4S_VideoInstance *instance = NULL;
    SS4S_VideoOpenResult result = driver->Open(info, &extraInfo, &instance, player->context.video);
    if (result == SS4S_VIDEO_OPEN_OK) {
        assert(instance != NULL);
        size_t statsCapacity = 120;
        if (info->frameRateNumerator > 0 && info->frameRateDenominator > 0) {
            size_t fps = info->frameRateNumerator / info->frameRateDenominator;
            if (fps > 0) {
                statsCapacity = fps * 2;
            }
        }
        SS4S_StatsCounterInit(&player->stats.video, statsCapacity);
        if (!SS4S_FeedGuardOpen(&player->video_guard, instance)) {
            SS4S_StatsCounterDeinit(&player->stats.video);
            SS4S_MutexUnlock(player->mutex);
            driver->Close(instance);
            return SS4S_VIDEO_OPEN_ERROR;
        }
    }
    SS4S_MutexUnlock(player->mutex);
    return result;
}

SS4S_VideoFeedResult SS4S_PlayerVideoFeed(SS4S_Player *player, const unsigned char *data, size_t size,
                                          SS4S_VideoFeedFlags flags) {
    SS4S_VideoInstance *video = SS4S_FeedGuardAcquire(&player->video_guard);
    if (video == NULL) {
        return SS4S_VIDEO_FEED_NOT_READY;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    assert(driver != NULL);
    assert(driver->Feed != NULL);
    SS4S_VideoFeedResult result = driver->Feed(video, data, size, flags);
    SS4S_FeedGuardRelease(&player->video_guard);
    return result;
}

bool SS4S_PlayerVideoSizeChanged(SS4S_Player *player, int width, int height) {
    /* SizeChanged on ndl and lgnc backends internally does a destructive
     * Close+Open of the decoder. Drain in-flight Feeds and block new
     * ones for the duration so they can't race with the teardown. */
    SS4S_VideoInstance *video = SS4S_FeedGuardBeginExclusive(&player->video_guard);
    if (video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    bool result = false;
    if (driver != NULL && driver->SizeChanged != NULL) {
        result = driver->SizeChanged(video, width, height);
    }
    SS4S_FeedGuardEndExclusive(&player->video_guard);
    return result;
}

bool SS4S_PlayerVideoSetHDRInfo(SS4S_Player *player, const SS4S_VideoHDRInfo *info) {
    /* SetHDRInfo on ndl-webos5 with info==NULL triggers an internal
     * Unload+Load of the NDL pipeline. Treat the entire call as
     * exclusive so an in-flight Feed cannot run on the half-destroyed
     * decoder. */
    SS4S_VideoInstance *video = SS4S_FeedGuardBeginExclusive(&player->video_guard);
    if (video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    bool result = false;
    if (driver != NULL && driver->SetHDRInfo != NULL) {
        result = driver->SetHDRInfo(video, info);
    }
    SS4S_FeedGuardEndExclusive(&player->video_guard);
    return result;
}

bool SS4S_PlayerVideoSetDisplayArea(SS4S_Player *player, const SS4S_VideoRect *src, const SS4S_VideoRect *dst) {
    SS4S_VideoInstance *video = SS4S_FeedGuardAcquire(&player->video_guard);
    if (video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL || driver->SetDisplayArea == NULL) {
        SS4S_FeedGuardRelease(&player->video_guard);
        return false;
    }
    bool result = driver->SetDisplayArea(video, src, dst);
    SS4S_FeedGuardRelease(&player->video_guard);
    return result;
}

bool SS4S_PlayerVideoSetFrameCallback(SS4S_Player *player, SS4S_VideoFrameCallback *callback, void *userdata) {
    SS4S_VideoInstance *video = SS4S_FeedGuardAcquire(&player->video_guard);
    if (video == NULL) {
        return false;
    }
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    if (driver == NULL || driver->SetFrameCallback == NULL) {
        SS4S_FeedGuardRelease(&player->video_guard);
        return false;
    }
    bool result = driver->SetFrameCallback(video, callback, userdata);
    SS4S_FeedGuardRelease(&player->video_guard);
    return result;
}

bool SS4S_VideoFrameRetain(SS4S_VideoOutputFrame *frame) {
    if (frame == NULL || frame->_private.retain == NULL) {
        return false;
    }
    return frame->_private.retain(frame);
}

void SS4S_VideoFrameRelease(SS4S_VideoOutputFrame *frame) {
    if (frame == NULL || frame->_private.release == NULL) {
        return;
    }
    frame->_private.release(frame);
    frame->_private.retain = NULL;
    frame->_private.release = NULL;
    frame->_private.handle = NULL;
}

bool SS4S_PlayerVideoClose(SS4S_Player *player) {
    SS4S_VideoInstance *video = SS4S_FeedGuardClose(&player->video_guard);
    if (video == NULL) {
        return false;
    }
    SS4S_MutexLock(player->mutex);
    SS4S_StatsCounterDeinit(&player->stats.video);
    SS4S_MutexUnlock(player->mutex);
    const SS4S_VideoDriver *driver = SS4S_GetVideoDriver();
    assert(driver != NULL);
    assert(driver->Close != NULL);
    driver->Close(video);
    return true;
}
