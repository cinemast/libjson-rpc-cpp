#include "threads.h"

#if defined(_WIN32)
  static int CONVERT_HANDLE_TO_STATUS(HANDLE h) {
    return h != NULL ? 0 : -1;
  }

  int threadCreate(ThreadHandle* thread, ThreadStartRoutine routine, void *args)
  {
    *thread = CreateThread(NULL, 0, routine, args, 0, NULL);
    return CONVERT_HANDLE_TO_STATUS(*thread);
  }

  int threadJoin(ThreadHandle thread)
  {
  if (WaitForSingleObject(thread, INFINITE) != WAIT_FAILED) {
    CloseHandle(thread);
    return 0;
  }
  else
    return -1;
  }

  int mutexCreate(MutexHandle* mutex)
  {
    *mutex = CreateMutex(NULL, FALSE, NULL);
    return CONVERT_HANDLE_TO_STATUS(*mutex);
  }

  int mutexDestroy(MutexHandle* mutex)
  {
    return CloseHandle(*mutex) ? 0 : -1;
  }

  int mutexLock(MutexHandle* mutex)
  {
    return WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;
  }

  int mutexUnlock(MutexHandle* mutex)
  {
    return ReleaseMutex(*mutex) != 0 ? 0 : -1;
  }

#else

  int threadCreate(ThreadHandle* thread, ThreadStartRoutine routine, void *args)
  {
    return pthread_create(thread, NULL, routine, args);
  }

  int threadJoin(ThreadHandle thread)
  {
    return pthread_join(thread, NULL);
  }

  int mutexCreate(MutexHandle* mutex)
  {
    return pthread_mutex_init(mutex, NULL);
  }

  int mutexDestroy(MutexHandle* mutex)
  {
    return pthread_mutex_destroy(mutex);
  }

  int mutexLock(MutexHandle* mutex)
  {
    return pthread_mutex_lock(mutex);
  }

  int mutexUnlock(MutexHandle* mutex)
  {
    return pthread_mutex_unlock(mutex);
  }
#endif

