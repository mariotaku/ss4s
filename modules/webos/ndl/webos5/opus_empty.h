#pragma once

typedef struct SS4S_NDLOpusEmpty SS4S_NDLOpusEmpty;

SS4S_NDLOpusEmpty *SS4S_NDLOpusEmptyCreate(int channels, int streams, int coupled);

void SS4S_NDLOpusEmptyDestroy(SS4S_NDLOpusEmpty *instance);

/**
 * Create a thread to wait for feed empty audio samples.
 * Once the first video frame is received, audio samples will start being played.
 */
void SS4S_NDLOpusEmptyMediaLoaded(SS4S_NDLOpusEmpty *instance);

/**
 * Destroy the thread that feeds empty audio samples.
 */
void SS4S_NDLOpusEmptyMediaUnloaded(SS4S_NDLOpusEmpty *instance);

/**
 * Notify the thread to start feeding empty audio samples.
 */
void SS4S_NDLOpusEmptyMediaVideoReady(SS4S_NDLOpusEmpty *instance);

/**
 * Stop feeding empty audio samples.
 */
void SS4S_NDLOpusEmptyMediaAudioReady(SS4S_NDLOpusEmpty *instance);
