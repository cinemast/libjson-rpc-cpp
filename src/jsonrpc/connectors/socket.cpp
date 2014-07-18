#include "socket.h"

#ifdef _WIN32
static int CONVERT_HANDLE_TO_STATUS(HANDLE h) {
	return h != NULL ? 0 : -1;
}

int pthread_create(pthread_t *thread, const void *attr,
	LPTHREAD_START_ROUTINE start_routine, void *arg)
{
	*thread = CreateThread(NULL, 0, start_routine, arg, 0, NULL);
	return CONVERT_HANDLE_TO_STATUS(*thread);
}

int pthread_join(pthread_t thread, void **value_ptr)
{
	return WaitForSingleObject(thread, INFINITE) != WAIT_FAILED ? 0 : -1;
}

int pthread_mutex_init(pthread_mutex_t *mutex,
    const void *attr)
{
	*mutex = CreateMutex(NULL, FALSE, NULL);
	return CONVERT_HANDLE_TO_STATUS(*mutex);
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	CloseHandle(*mutex);
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	return WaitForSingleObject( *mutex, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	return ReleaseMutex(*mutex) != 0 ? 0 : -1;
}

#endif
