#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct SS4S_Pacer SS4S_Pacer;

typedef int (*SS4S_PacerCallback)(void *arg, const uint8_t *data, size_t size);

SS4S_Pacer *SS4S_PacerCreate(size_t healthyBufferCount, size_t bufferSizeLimit, uint32_t intervalUs,
                             SS4S_PacerCallback callback, void *arg);

/**
 * Feed data to the pacer buffer
 *
 * @param pacer Pacer instance
 * @param data Data to be written to the pacer buffer
 * @param size Size of the data to be written to the pacer buffer
 * @return Size of data written to the pacer buffer,
 *         0 if the buffer is full,
 *         -1 if the size is larger than the frame size limit
 */
int SS4S_PacerFeed(SS4S_Pacer *pacer, const uint8_t *data, size_t size);

size_t SS4S_PacerGetBufferCount(SS4S_Pacer *pacer);

/**
 * Clear the pacer buffer
 *
 * @param pacer Pacer instance
 */
void SS4S_PacerClear(SS4S_Pacer *pacer);

void SS4S_PacerDestroy(SS4S_Pacer *pacer);