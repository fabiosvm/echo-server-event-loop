//
// poll.h
//

#ifndef POLL_H
#define POLL_H

#ifdef __linux__
  #include <sys/epoll.h>
#elif __APPLE__
  #include <sys/event.h>
#endif

#define POLL_DEFAULT_SIZE    256
#define POLL_DEFAULT_TIMEOUT 500

#ifdef __linux__
  #define POLL_FLAG_READ  (EPOLLIN | EPOLLET)
  #define POLL_FLAG_WRITE (EPOLLOUT | EPOLLET)
#elif __APPLE__
  #define POLL_FLAG_READ  EVFILT_READ
  #define POLL_FLAG_WRITE EVFILT_WRITE
#endif

#ifdef __linux__
  #define poll_event_get_fd(ev)   ((ev).data.fd)
  #define poll_event_is_read(ev)  ((ev).events & EPOLLIN)
  #define poll_event_is_write(ev) ((ev).events & EPOLLOUT)
#elif __APPLE__
  #define poll_event_get_fd(ev)   ((ev).ident)
  #define poll_event_is_read(ev)  ((ev).filter == EVFILT_READ)
  #define poll_event_is_write(ev) ((ev).filter == EVFILT_WRITE)
#endif

#ifdef __linux__
  #define poll_event_t struct epoll_event
#elif __APPLE__
  #define poll_event_t struct kevent
#endif

typedef int poll_t;

poll_t poll_create(void);
int poll_close(poll_t poll);
int poll_add(poll_t poll, int fd, int flags);
int poll_delete(poll_t poll, int fd);
int poll_update(poll_t poll, int fd, int flags);
int poll_wait(poll_t poll, poll_event_t *events, int size, int timeout);

#endif // POLL_H
