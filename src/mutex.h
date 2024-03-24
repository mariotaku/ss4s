#pragma once

typedef struct SS4S_Mutex SS4S_Mutex;

SS4S_Mutex *SS4S_MutexCreate();

void SS4S_MutexLockEx(SS4S_Mutex *mutex, const char *caller);

void SS4S_MutexUnlockEx(SS4S_Mutex *mutex, const char *caller);

void SS4S_MutexDestroy(SS4S_Mutex *mutex);

#define SS4S_MutexLock(mutex) SS4S_MutexLockEx(mutex, __FUNCTION__)

#define SS4S_MutexUnlock(mutex) SS4S_MutexUnlockEx(mutex, __FUNCTION__)