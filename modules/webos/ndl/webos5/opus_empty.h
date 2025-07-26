#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct SS4S_OpusEmpty SS4S_OpusEmpty;

typedef int (*SS4S_NDLOpusEmptyFeedFunc)(void *arg, const unsigned char *data, size_t size);

/**
 * Create an instance of the empty frame generator.
 * @param channels Number of channels
 * @param streams Number of opus streams
 * @param coupled Number of coupled streams
 * @return Frame generator instance
 */
SS4S_OpusEmpty *SS4S_OpusEmptyCreate(uint32_t channels, uint32_t streams, uint32_t coupled);

/**
 * Feed 1 empty frame
 * @param instance Frame generator instance
 * @param feed Feed function
 * @param feedArg Context for the feed function
 */
int SS4S_OpusEmptyFeed(SS4S_OpusEmpty *instance, SS4S_NDLOpusEmptyFeedFunc feed, void *feedArg);

void SS4S_OpusEmptyDestroy(SS4S_OpusEmpty *instance);