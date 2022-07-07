//
// socket.h
//

#ifndef SOCKET_H
#define SOCKET_H

#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif

#ifdef _WIN32
  #define socket_t  SOCKET
#else
  #define socket_t       int
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR   -1
#endif

#ifndef __APPLE__
  #define socklen_t int
#endif

int socket_startup(void);
int socket_cleanup(void);
int socket_get_last_error(void);
int socket_close(socket_t sock);
int socket_send(socket_t sock, const void *buf, int len, int flags);
int socket_recv(socket_t sock, void *buf, int len, int flags);

#endif // SOCKET_H
