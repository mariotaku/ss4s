#pragma once

#include <stdint.h>

typedef struct SS4S_NDLOpusFix SS4S_NDLOpusFix;

typedef struct OpusConfig {
    uint32_t sampleRate;
    uint8_t channels;
    uint8_t streamCount;
    uint8_t coupledCount;
    uint8_t mapping[6];
} OpusConfig;

SS4S_NDLOpusFix *SS4S_NDLOpusFixCreate(const OpusConfig *config);

int SS4S_NDLOpusFixProcess(SS4S_NDLOpusFix *instance, const unsigned char *data, size_t size);

const uint8_t *SS4S_NDLOpusFixGetBuffer(SS4S_NDLOpusFix *instance);

void SS4S_NDLOpusFixDestroy(SS4S_NDLOpusFix *instance);