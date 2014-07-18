#ifndef JSONRPC_CONNECTORS_SOCKET_H_
#define JSONRPC_CONNECTORS_SOCKET_H_

#define CHECK(status) if ((status) != 0) goto error;
#define CHECK_READSIZE(read) if((read) < 0) goto error;
#define CHECK_PTR(ptr) if ((ptr) == NULL) goto error;

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <io.h>
static const int BOTH_DIRECTION=SD_BOTH;
typedef HANDLE pthread_t;
typedef HANDLE pthread_mutex_t;

static int CONVERT_HANDLE_TO_STATUS(HANDLE h);

int pthread_create(pthread_t *thread, const void *attr, LPTHREAD_START_ROUTINE start_routine, void *arg);
int pthread_join(pthread_t thread, void **value_ptr);

int pthread_mutex_init(pthread_mutex_t *mutex, const void *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

// WinUser.h has a preprocessor macro to replace SendMessage with SendMessageW or SendMessageA
// We need to undef this macro to be sure that our AbstractClientConnector::SendMessage methods are not
// modified by this preprocessor macro
#ifdef _WINUSER_
	#ifdef SendMessage
	#undef SendMessage
	#endif
#endif
#define CHECK_STATUS(s) if ((s) == SOCKET_ERROR) goto error;
#define CHECK_SOCKET(s) if ((s) == INVALID_SOCKET) goto error;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <netdb.h>
static const int BOTH_DIRECTION=SHUT_RDWR;
#define closesocket(s) close((s))
#define CHECK_STATUS(s) if ((s) < 0) goto error;
#define CHECK_SOCKET(s) if ((s) == -1) goto error;
#include <pthread.h>

#endif


#endif // JSONRPC_CONNECTORS_SOCKET_H_
