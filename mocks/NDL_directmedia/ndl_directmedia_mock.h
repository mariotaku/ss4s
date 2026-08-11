#pragma once

#include <stdbool.h>

extern bool ndl_mock_init;
extern bool audio_opened, video_opened;

void mock_ndl_lock(const char *func);

void mock_ndl_unlock(const char *func);

/**
 * Shared body of NDL_DirectMediaInit. The function itself is defined per API version,
 * because API 1 takes a release callback and API 2 does not.
 */
int mock_ndl_init(const char *app_id);