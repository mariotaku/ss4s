#pragma once

typedef struct SS4S_NDLOpusEmpty SS4S_NDLOpusEmpty;

SS4S_NDLOpusEmpty *SS4S_NDLOpusEmptyCreate(int channels, int streams, int coupled);

void SS4S_NDLOpusEmptyDestroy(SS4S_NDLOpusEmpty *instance);

int SS4S_NDLOpusEmptyPlay(const SS4S_NDLOpusEmpty *instance);