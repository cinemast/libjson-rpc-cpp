#include "threads.h"

#ifdef __INTIME__
  static int CONVERT_HANDLE_TO_STATUS(RTHANDLE h) {
    return h != BAD_RTHANDLE ? 0 : -1;
  }

  int threadCreate(ThreadHandle* thread, ThreadStartRoutine routine, void *args)
  {
    return threadCreate(thread, routine, args, 1024);
  }

  int threadCreate(ThreadHandle* thread, ThreadStartRoutine routine, void *args, DWORD stack_size)
  {
      char priority = GetRtThreadPriority(GetRtThreadHandles(THIS_THREAD));
    priority++;
    *thread = CreateRtThread(priority, routine, stack_size, args);
    return CONVERT_HANDLE_TO_STATUS(*thread);
  }

  int threadJoin(ThreadHandle thread)
  {
    return SuspendRtThread(thread) ? 0 : -1;
  }

  int mutexCreate(MutexHandle* mutex)
  {
    *mutex = CreateRtSemaphore(1, 1, FIFO_QUEUING);
    return CONVERT_HANDLE_TO_STATUS(*mutex);
  }

  int mutexDestroy(MutexHandle* mutex)
  {
    return DeleteRtSemaphore(*mutex) ? 0 : -1;
  }

  int mutexLock(MutexHandle* mutex)
  {
    return WaitForRtSemaphore(*mutex, 1, WAIT_FOREVER) != WAIT_FAILED ? 0 : -1;
  }

  int mutexUnlock(MutexHandle* mutex)
  {
    return ReleaseRtSemaphore(*mutex, 1) ? 0 : -1;
  }
#else
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
    return WaitForSingleObject(thread, INFINITE) != WAIT_FAILED ? 0 : -1;
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

#endif
