//
// server.c
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "poll.h"
#include "socket.h"

#define DEFAULT_PORT    9000
#define MAX_MESSAGE_LEN 1023

static poll_t poll;

static inline socket_t socket_server(int port);

static inline socket_t socket_server(int port)
{
  socket_t server;

	if ((server = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket: %d\n", socket_get_last_error());
    poll_close(poll);
    socket_cleanup();
    return EXIT_FAILURE;
	}

  int on = 1;
  if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == SOCKET_ERROR)
  {
    printf("Could not set socket option: %d\n", socket_get_last_error());
    poll_close(poll);
    socket_cleanup();
    return EXIT_FAILURE;
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		printf("Bind failed: %d\n", socket_get_last_error());
    poll_close(poll);
    socket_cleanup();
    return EXIT_FAILURE;
	}

  if (listen(server, SOMAXCONN) == SOCKET_ERROR)
  {
    printf("Listen failed: %d\n", socket_get_last_error());
    poll_close(poll);
    socket_cleanup();
    return EXIT_FAILURE;
  }

  return server;
}

int main(int argc, char *argv[])
{
  int port = argc > 1 ? atoi(argv[1]) : 0;
  port = port ? port : DEFAULT_PORT;

  poll = poll_create();

	if (socket_startup())
	{
		printf("Socket startup failed: %d\n", socket_get_last_error());
    poll_close(poll);
		return EXIT_FAILURE;
	}

  socket_t server = socket_server(port);
  poll_add(poll, server, POLL_FLAG_READ);

  printf("Listening on port %d\n", port);

  for (;;)
  {
    poll_event_t events[POLL_DEFAULT_SIZE];
    int n = poll_wait(poll, events, 0, 0);

    char buffer[MAX_MESSAGE_LEN + 1];
    int nbytes;

    for (int i = 0; i < n; ++i)
    {
      if (poll_event_get_fd(events[i]) == server)
      {
        socket_t client;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        printf("Waiting for connection...\n");

        if ((client = accept(server, (struct sockaddr *) &client_addr, &client_addr_len)) == INVALID_SOCKET)
        {
          printf("Accept failed: %d\n", socket_get_last_error());          
          poll_close(poll);
          socket_cleanup();
          return EXIT_FAILURE;
        }

        printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));

        poll_add(poll, client, POLL_FLAG_READ);
        break;
			}

      if (poll_event_is_read(events[i]))
      {
        socket_t client = poll_event_get_fd(events[i]);
        if ((nbytes = socket_recv(client, buffer, MAX_MESSAGE_LEN, 0)) == SOCKET_ERROR)
        {
          printf("Receive failed: %d\n", socket_get_last_error());
          poll_close(poll);
          socket_cleanup();
          return EXIT_FAILURE;
        }

        if (!nbytes)
        {
          printf("Connection closed\n");
          poll_delete(poll, client);
          socket_close(client);
          continue;
        }

        buffer[nbytes] = '\0';
        printf("Received: %s\n", buffer);

        poll_update(poll, client, POLL_FLAG_WRITE);
        break;
			}

			if (poll_event_is_write(events[i]))
      {
        socket_t client = poll_event_get_fd(events[i]);
        if ((nbytes = socket_send(client, buffer, nbytes, 0)) == SOCKET_ERROR)
        {
          printf("Send failed: %d\n", socket_get_last_error());
          poll_close(poll);
          socket_cleanup();
          return EXIT_FAILURE;
        }
        printf("Sent: %s\n", buffer);

        poll_update(poll, client, POLL_FLAG_READ);
        break;
			}

    }

  }

  poll_close(poll);
  socket_cleanup();
  return EXIT_SUCCESS;
}
