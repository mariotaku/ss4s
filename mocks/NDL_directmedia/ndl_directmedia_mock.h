#pragma once

#include <stdbool.h>

extern bool ndl_mock_init;
extern bool audio_opened, video_opened;

void mock_ndl_lock(const char *func);

void mock_ndl_unlock(const char *func);