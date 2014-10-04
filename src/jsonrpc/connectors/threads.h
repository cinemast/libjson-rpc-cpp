#ifndef THREADS_H
#define THREADS_H

#ifdef _WIN32
  #ifdef __INTIME__
    #include <windows.h>
    #include <iwin32.h>
  #else
    // We want to use WinSock2 so we don't want to include WinSock when include Windows.h
    #define _WINSOCKAPI_
    #include <windows.h>
  #endif
  
  typedef HANDLE ThreadHandle;
  typedef HANDLE MutexHandle;
  typedef LPTHREAD_START_ROUTINE ThreadStartRoutine;

  #define THREAD_ROUTINE_RETURN DWORD WINAPI
  //#endif
#else
  #include <pthread.h>
  typedef pthread_t ThreadHandle;
  typedef pthread_mutex_t MutexHandle;
  typedef void *(*ThreadStartRoutine)(void*);
  #define THREAD_ROUTINE_RETURN void*
#endif

int threadCreate(ThreadHandle*, ThreadStartRoutine, void*);

int threadJoin(ThreadHandle);

int mutexCreate(MutexHandle*);
int mutexDestroy(MutexHandle*);
int mutexLock(MutexHandle*);
int mutexUnlock(MutexHandle*);

#endif //THREADS_H
