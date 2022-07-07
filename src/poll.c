//
// poll.c
//

#include "poll.h"
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

static inline void set_nonblocking(int fd);

static inline void set_nonblocking(int fd)
{
	int opts = fcntl(fd, F_GETFL);
  assert(opts != -1);
	opts = opts | O_NONBLOCK;
  assert(fcntl(fd, F_SETFL, opts) != -1);
}

poll_t poll_create(void)
{
  poll_t poll;
#ifdef __linux__
  poll = epoll_create1(0);
#elif __APPLE__
  poll = kqueue();
#endif
  return poll;
}

int poll_close(poll_t poll)
{
  return close(poll);
}

int poll_add(poll_t poll, int fd, int flags)
{
  set_nonblocking(fd);
  poll_event_t ev;
#ifdef __linux__
  ev.events = flags;
  ev.data.fd = fd;
  return epoll_ctl(poll, EPOLL_CTL_ADD, fd, &ev);
#elif __APPLE__
  EV_SET(&ev, fd, flags, EV_ADD | EV_ENABLE, 0, 0, NULL);
  return kevent(poll, &ev, 1, NULL, 0, NULL);
#endif
}

int poll_delete(poll_t poll, int fd)
{
  poll_event_t ev;
#ifdef __linux__
  ev.data.fd = fd;
  return epoll_ctl(poll, EPOLL_CTL_DEL, fd, &ev);
#elif __APPLE__
  EV_SET(&ev, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
  return kevent(poll, &ev, 1, NULL, 0, NULL);
#endif
}

int poll_update(poll_t poll, int fd, int flags)
{
  poll_event_t ev;
#ifdef __linux__
  ev.events = flags;
  ev.data.fd = fd;
  return epoll_ctl(poll, EPOLL_CTL_MOD, fd, &ev);
#elif __APPLE__
  EV_SET(&ev, fd, flags, EV_ADD | EV_ENABLE, 0, 0, NULL);
  return kevent(poll, &ev, 1, NULL, 0, NULL);
#endif
}

int poll_wait(poll_t poll, poll_event_t *events, int size, int timeout)
{
  size = size ? size : POLL_DEFAULT_SIZE;
  timeout = timeout ? timeout : POLL_DEFAULT_TIMEOUT;
  int n;
#ifdef __linux__
  n = epoll_wait(poll, events, size, timeout);
#elif __APPLE__
  struct timespec ts;
  ts.tv_sec = timeout / 1000;
  ts.tv_nsec = (timeout % 1000) * 1000000;
  n = kevent(poll, NULL, 0, events, size, &ts);
#endif
  return n;
}
