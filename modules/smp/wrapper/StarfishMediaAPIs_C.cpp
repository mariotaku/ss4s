#include "StarfishMediaAPIs_C.h"

#include "StarfishMediaAPIs.h"

extern "C" {

struct StarfishMediaAPIs_C {
    StarfishMediaAPIs inner;
};

StarfishMediaAPIs_C *StarfishMediaAPIs_create(const char *uid) {
    return new StarfishMediaAPIs_C{
            .inner = StarfishMediaAPIs(uid)
    };
}

bool StarfishMediaAPIs_feed(StarfishMediaAPIs_C *api, const char *payload, char *result, size_t resultLen) {
    auto ret = api->inner.Feed(payload);
    auto nCopied = ret.copy(result, resultLen - 1, 0);
    result[nCopied] = '\0';
    return true;
}

bool StarfishMediaAPIs_load(StarfishMediaAPIs_C *api, const char *payload, StarfishMediaAPIs_callbackWithData *callback,
                            void *data) {
    return api->inner.Load(payload, callback, data);
}

bool StarfishMediaAPIs_notifyForeground(StarfishMediaAPIs_C *api) {
    return api->inner.notifyForeground();
}

bool StarfishMediaAPIs_notifyBackground(StarfishMediaAPIs_C *api) {
    return api->inner.notifyBackground();
}

bool StarfishMediaAPIs_pause(StarfishMediaAPIs_C *api) {
    return api->inner.Pause();
}

bool StarfishMediaAPIs_play(StarfishMediaAPIs_C *api) {
    return api->inner.Play();
}

bool StarfishMediaAPIs_pushEOS(StarfishMediaAPIs_C *api) {
    return api->inner.pushEOS();
}

bool StarfishMediaAPIs_unload(StarfishMediaAPIs_C *api) {
    return api->inner.Unload();
}

void StarfishMediaAPIs_destroy(StarfishMediaAPIs_C *api) {
    delete api;
}

}