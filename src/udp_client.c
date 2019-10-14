#include "udp_client.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include "utils.h"

#define SOCKET_TIMEOUT_SECONDS      5
#define SOCKET_TIMEOUT_MICROSECONDS 0

struct UDPClient {
  int socket;
};

UDPClient *client_create() {
  int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (s == -1) {
    fprintf(stderr, "[UDPClient] Failed to create socket with error: %d\n", errno);
    return NULL;
  }

  struct timeval tv = { SOCKET_TIMEOUT_SECONDS, SOCKET_TIMEOUT_MICROSECONDS };

  if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
    fprintf(stderr, "[UDPClient] Failed to set socket timeout with error: %d\n", errno);
  }

  return client_create_from_socket(s);
}

UDPClient *client_create_from_socket(int fd) {
  UDPClient *client = malloc(sizeof(UDPClient));
  CHECK_ALLOC(client);
  client->socket = fd;
  return client;
}

bool client_send(UDPClient *client, const Message *request, Message *response) {
  if (!client_send_message(client, request)) {
    return false;
  }

  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  response->length = recvfrom(client->socket, &response->data, MAX_MESSAGE_LENGTH, 0,
      (struct sockaddr *) &addr, &addrlen);

  if (response->length == -1) {
    return false;
  }

  response->sender.address = addr.sin_addr.s_addr;
  response->sender.port = addr.sin_port;

  return true;
}

bool client_send_message(UDPClient *client, const Message *message) {
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = message->recipient.port,
    .sin_addr = { message->recipient.address }
  };

  ssize_t sent = sendto(client->socket, message->data, message->length, 0,
      (struct sockaddr *) &addr, sizeof(addr));
  
  if (sent == -1) {
    fprintf(stderr, "[UDPClient] Failed to send message to %s:%d with error: %d.\n",
        inet_ntoa(addr.sin_addr), htons(addr.sin_port), errno);
    return false;
  }

  if (sent != message->length) {
    fprintf(stderr, "[UDPClient] Failed to send message to %s:%d. %ld bytes were sent.\n",
        inet_ntoa(addr.sin_addr), htons(addr.sin_port), sent);
    return false;
  }

  return true;
}

void client_destroy(UDPClient *client, bool close_socket) {
  if (close_socket) {
    close(client->socket);
  }
  free(client);
}
