#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ad_list.h"
#include "udp_server.h"
#include "utils.h"

typedef struct {
  ProgramOptions options;
  Set *domain_set;
} HandlerContext;

void generate_localhost_response(Message *message, const uint16_t transaction_id, const size_t name_size, const uint8_t *name) {
  size_t message_length = 32 + name_size;
  message->length = message_length;
  
  uint8_t *data = message->data;
  *((uint16_t *)data) = transaction_id; // To match up the response with the request
  data[2] = 0x81; // Response flags
  data[3] = 0x80;
  data[5] = 0x1; // One query contained
  data[7] = 0x1; // One response contained
  memcpy(data+12, name, name_size); // Domain name
  size_t offset = 12 + name_size;
  data[offset + 1] = 0x01; // A record type
  data[offset + 3] = 0x01; // IN record class
  data[offset + 4] = 0xc0; // Response start, pointer to domain name
  data[offset + 5] = 0x0c;
  data[offset + 7] = 0x01; // A
  data[offset + 9] = 0x01; // IN
  data[offset + 12] = 0x01; // TTL
  data[offset + 13] = 0x18;
  data[offset + 15] = 4; // IP length
  // Using 0.0.0.0 for the IP as it's non-routable
}

bool handle_server_request(UDPServer *server, const Message *request, Message *response, void *context) {
  HandlerContext *hcontext = (HandlerContext *) context;

  size_t name_length;
  char *domain = parse_dns_domain(request, &name_length);

  Set *domain_set = hcontext->domain_set;
  bool ad_domain = set_contains(domain_set, domain);
  if (ad_domain) {
    printf("Blocking DNS request: %s\n", domain);
    uint16_t transaction_id = *((uint16_t *) request->data);
    generate_localhost_response(response, transaction_id, name_length, request->data+QUESTION_START_BYTE);
  } else {
    printf("Forwarding DNS request: %s\n", domain);
    UDPClient *client = server_getclient(server);

    // Create external request by copying request data and sending to external provider
    Message forwarded_request = { .length = request->length };
    set_message_address(&forwarded_request.recipient, DEFAULT_DNS_PORT, hcontext->options.provider_address);
    memcpy(forwarded_request.data, request->data, forwarded_request.length);

    // Send request and write response
    client_send(client, &forwarded_request, response);
    server_returnclient(server, client);
  }

  free(domain);

  return true;
}

int main(int argc, char **argv) {
  ProgramOptions options;
  if (!parse_options(argc, argv, &options)) {
    return EXIT_FAILURE;
  }

  AdListsInfo *lists_info = create_default_adlists_info(options.disable_defaults, options.blocklist, options.whitelist);
  Set *domain_set = set_new();
  load_active_lists(lists_info, domain_set);

  HandlerContext context = { options, domain_set };

  UDPServerConfig config = {
    .port    = options.server_port,
    .address = options.server_address
  };

  UDPServer *server = server_create(&config);
  server_run(server, handle_server_request, &context);
  server_destroy(server);

  free_adlists(lists_info);
  set_free_vals(domain_set);

  return EXIT_SUCCESS;
}
