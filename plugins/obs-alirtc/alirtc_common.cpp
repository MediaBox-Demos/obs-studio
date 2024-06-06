#include "alirtc_common.h"
#include <obs.h>

static AlivcLivePusher *g_rtc_pusher = nullptr;
static std::mutex g_rtc_pusher_mutex;


AlivcLivePusher *obs_alirtc_get_instace() {
    std::lock_guard<std::mutex> lock(g_rtc_pusher_mutex);
    if (!g_rtc_pusher) {
        blog(LOG_INFO, "Alirtc Pusher Create");
        g_rtc_pusher = AlivcLivePusher::Create("");
        AlivcLivePushConfig config;
        g_rtc_pusher->init(config);
    }

    return g_rtc_pusher;
}

void obs_alirtc_destory_instace() {
    std::lock_guard<std::mutex> lock(g_rtc_pusher_mutex);
    if (g_rtc_pusher) {
        blog(LOG_INFO, "Alirtc Pusher destory");
        AlivcLivePusher::destroy();
        g_rtc_pusher = nullptr;
    }
}