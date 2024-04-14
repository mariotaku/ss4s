#include <malloc.h>

#include "opus_fix.h"
#include "ndl_common.h"

#include <opus_multistream.h>
#include <assert.h>

static const unsigned char webos_mapping[6] = {0, 1, 4, 5, 2, 3};

struct SS4S_NDLOpusFix {
    OpusConfig config;
    float *buffer;
    uint8_t output[8192];
    OpusMSDecoder *decoder;
    OpusMSEncoder *encoder;
};

SS4S_NDLOpusFix *SS4S_NDLOpusFixCreate(const OpusConfig *config) {
    assert(config->channels == 6);
    SS4S_NDLOpusFix *instance = calloc(1, sizeof(SS4S_NDLOpusFix));
    instance->config = *config;
    instance->buffer = malloc(config->sampleRate * config->channels * sizeof(float));
    int error = 0;
    instance->decoder = opus_multistream_decoder_create((int) config->sampleRate, config->channels, config->streamCount,
                                                        config->coupledCount, config->mapping, &error);
    if (instance->decoder == NULL) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelError, "NDL", "Failed to create decoder: %s", opus_strerror(error));
        SS4S_NDLOpusFixDestroy(instance);
        return NULL;
    }
    instance->encoder = opus_multistream_encoder_create((int) config->sampleRate, config->channels, 4, 2, webos_mapping,
                                                        OPUS_APPLICATION_RESTRICTED_LOWDELAY, NULL);
    if (instance->encoder == NULL) {
        SS4S_NDL_webOS5_Log(SS4S_LogLevelError, "NDL", "Failed to create encoder");
        SS4S_NDLOpusFixDestroy(instance);
        return NULL;
    }
    return instance;
}

int SS4S_NDLOpusFixProcess(SS4S_NDLOpusFix *instance, const unsigned char *data, size_t size) {
    int decode_len = opus_multistream_decode_float(instance->decoder, data, (int) size, instance->buffer,
                                                   (int) instance->config.sampleRate, 0);
    return opus_multistream_encode_float(instance->encoder, instance->buffer, decode_len, instance->output,
                                         sizeof(instance->output));
}

const uint8_t *SS4S_NDLOpusFixGetBuffer(SS4S_NDLOpusFix *instance) {
    return instance->output;
}

void SS4S_NDLOpusFixDestroy(SS4S_NDLOpusFix *instance) {
    if (instance->decoder != NULL) {
        opus_multistream_decoder_destroy(instance->decoder);
    }
    if (instance->encoder != NULL) {
        opus_multistream_encoder_destroy(instance->encoder);
    }
    free(instance->buffer);
    free(instance);
}