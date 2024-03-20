#pragma once

typedef struct SS4S_Mutex SS4S_Mutex;

SS4S_Mutex *SS4S_MutexCreate();

void SS4S_MutexLock(SS4S_Mutex *mutex);

void SS4S_MutexUnlock(SS4S_Mutex *mutex);

void SS4S_MutexDestroy(SS4S_Mutex *mutex);
