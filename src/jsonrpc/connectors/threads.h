#ifndef THREADS_H
#define THREADS_H

#ifdef _WIN32
  #ifdef __INTIME__
    #include <windows.h>
    #include <iwin32.h>
  #else
	// We include Winsock here because Visual Studio can only handle this order
	// Order is important. Without order ... it crash
	#include <WinSock2.h>
    #include <Windows.h>
  #endif
  typedef HANDLE ThreadHandle;
  typedef HANDLE MutexHandle;
  typedef LPTHREAD_START_ROUTINE ThreadStartRoutine;

  #define THREAD_ROUTINE_RETURN DWORD WINAPI

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
