#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct SS4S_OpusEmpty SS4S_OpusEmpty;

typedef int (*SS4S_NDLOpusEmptyFeedFunc)(const unsigned char *data, size_t size);

SS4S_OpusEmpty *SS4S_OpusEmptyCreate(int channels, int streams, int coupled);

void SS4S_OpusEmptyDestroy(SS4S_OpusEmpty *instance);

bool SS4S_OpusIsEmptyFrame(const SS4S_OpusEmpty *instance, const unsigned char *data, size_t size);

int SS4S_OpusEmptyPlay(const SS4S_OpusEmpty *instance, SS4S_NDLOpusEmptyFeedFunc feed);