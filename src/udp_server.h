#pragma once

#include <stdbool.h>

#include "udp_client.h"
#include "message.h"

typedef struct UDPServer UDPServer;

typedef struct {
  uint16_t port;
  const char *address;
} UDPServerConfig;

/*
 * Function pointer type that is invoked by a server on receiving a request.
 * The response should be written to response->data, and response->length should
 * be set to the number of bytes written. The maximum size of the response is
 * given by MAX_MESSAGE_LENGTH.
 * If the handler returns false, no response is sent.
 */
typedef bool (*RequestHandler)(UDPServer *server, const Message *request, Message *response, void *context);

/*
 * Creates a new server, binding it to the host and port
 * specified by config.
 */
UDPServer *server_create(const UDPServerConfig *config);

/*
 * Runs the server until the process receives a SIGINT or SIGTERM signal.
 * The request handler is invoked whenever a request is received,
 * with the context pointer passed on as an argument.
 */
void server_run(UDPServer *server, RequestHandler handler, void *context);

/*
 * Returns a client for making separate udp requests.
 */
UDPClient *server_getclient(UDPServer *server);

/*
 * Returns a client gotten from server_getclient to the server
 * for cleanup.
 */
void server_returnclient(UDPServer *server, UDPClient *client);

/*
 * Destroys the server, freeing its resources.
 */
void server_destroy(UDPServer *server);
