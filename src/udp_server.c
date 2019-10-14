#define _POSIX_C_SOURCE 199309L

#include "udp_server.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utils.h"

struct UDPServer {
  int socket;
  UDPClient *client;
};

sig_atomic_t terminate = 0;

void handle_signal(int signal) {
  terminate = 1;
}

UDPServer *server_create(const UDPServerConfig *config) {
  assert(config != NULL);

  int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (s == -1) {
    fprintf(stderr, "[UDPServer] Failed to create socket with error: %d\n", errno);
    return NULL;
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(config->port)
  };

  if (config->address == NULL) {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    addr.sin_addr.s_addr = inet_addr(config->address);
  }

  if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
    fprintf(stderr, "[UDPServer] Failed to bind socket to port %d with error: %d\n", config->port, errno);
    return NULL;
  }

  UDPServer *server = malloc(sizeof(UDPServer));
  CHECK_ALLOC(server);

  server->socket = s;
  server->client = client_create();

  printf("[UDPServer] Server listening on port %d...\n", config->port);

  return server;
}

Message *server_receive(UDPServer* server) {
  Message *request = malloc(sizeof(Message));
  CHECK_ALLOC(request);

  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  request->length = recvfrom(server->socket, &request->data, MAX_MESSAGE_LENGTH, 0,
      (struct sockaddr *) &addr, &addrlen);

  if (request->length == -1) {
    free(request);
    return NULL;
  }

  request->sender.address = addr.sin_addr.s_addr;
  request->sender.port = addr.sin_port;

  return request;
}

bool server_respond(UDPServer *server, const Message *response) {
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = response->recipient.port,
    .sin_addr = { response->recipient.address }
  };

  ssize_t sent = sendto(server->socket, response->data, response->length, 0,
      (struct sockaddr *) &addr, sizeof(addr));

  if (sent == -1) {
    fprintf(stderr, "[UDPServer] Failed to send response to %s:%d with error: %d.\n",
        inet_ntoa(addr.sin_addr), htons(addr.sin_port), errno);
    return false;
  }

  if (sent != response->length) {
    fprintf(stderr, "[UDPServer] Failed to send response to %s:%d. %ld bytes were sent.\n",
        inet_ntoa(addr.sin_addr), htons(addr.sin_port), sent);
    return false;
  }

  return true;
}

void server_run(UDPServer *server, RequestHandler handler, void *context) {
  assert(server != NULL);
  assert(handler != NULL);

  struct sigaction sa = {
    .sa_handler  = handle_signal,
    .sa_flags    = 0,
    .sa_restorer = NULL
  };

  sigemptyset(&sa.sa_mask);

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  while (!terminate) {
    Message *request = server_receive(server);

    if (request == NULL) {
      continue;
    }

    Message *response = calloc(1, sizeof(Message));
    CHECK_ALLOC(response);

    if (handler(server, request, response, context)) {
      response->recipient = request->sender;
      server_respond(server, response);
    }

    free(request);
    free(response);
  }
}

UDPClient *server_getclient(UDPServer *server) {
  return server->client;
}

void server_returnclient(UDPServer *server, UDPClient *client) {
  // Do nothing. This will probably be used if we implement multithreading.
}

void server_destroy(UDPServer *server) {
  client_destroy(server->client, true);
  close(server->socket);
  free(server);
}
