#pragma once

#include <stdbool.h>

#include "message.h"

typedef struct UDPClient UDPClient;

/*
 * Creates a new client.
 */
UDPClient *client_create();

/*
 * Creates a new client from an existing socket.
 */
UDPClient *client_create_from_socket(int fd);

/*
 * Sends a request to a server. The value at response will be set to the
 * response message.
 * The return value indicates whether the request was successful.
 */
bool client_send(UDPClient *client, const Message *request, Message *response);

/*
 * Similar to client_send but does not wait for a response.
 */
bool client_send_message(UDPClient *client, const Message *message);

/*
 * Destroys a client, freeing its resources.
 */
void client_destroy(UDPClient *client, bool close_socket);
