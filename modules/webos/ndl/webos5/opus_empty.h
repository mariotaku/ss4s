#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct SS4S_OpusEmpty SS4S_OpusEmpty;

typedef int (*SS4S_NDLOpusEmptyFeedFunc)(void *arg, const unsigned char *data, size_t size);

SS4S_OpusEmpty *SS4S_OpusEmptyCreate(uint32_t channels, uint32_t streams, uint32_t coupled, uint32_t frameDurationUs);

void SS4S_OpusEmptyStart(SS4S_OpusEmpty *instance, SS4S_NDLOpusEmptyFeedFunc feed, void *feedArg);

/**
 * Pause empty frame generation, and wait for 3 missing frames.
 * @param instance
 */
bool SS4S_OpusEmptyFrameArrived(SS4S_OpusEmpty *instance);

void SS4S_OpusEmptyDestroy(SS4S_OpusEmpty *instance);

typedef struct SS4S_NDLOpusFix SS4S_NDLOpusFix;