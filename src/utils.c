#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

uint64_t hash(const char *str) {
  unsigned char *to_hash = (unsigned char *)(str);
  uint32_t hash = 5381;
  int32_t c;

  while ( (c = *to_hash++) ) {
    hash = ((hash << 5u) + hash) + c;
  }

  return hash;
}

void fatal_error(const char *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  fputs("Fatal error: ", stderr);
  vfprintf(stderr, fmt, valist);
  fputs("\n", stderr);
  exit(EXIT_FAILURE);
}

bool validate_ip4_address(const char *address, uint32_t *result) {
  struct in_addr addr;
  bool success = inet_pton(AF_INET, address, &addr) == 1;
  if (result && success) *result = ntohl(addr.s_addr);
  return success;
}

bool parse_options(int argc, char **argv, ProgramOptions *options) {
  // Set default options
  options->server_port = DEFAULT_DNS_PORT;
  options->server_address = NULL;
  options->provider_address = ntohl(inet_addr("1.1.1.1")); // Cloudflare DNS provider
  options->disable_defaults = false;
  options->blocklist = NULL;
  options->whitelist = NULL;

  for (int i = 0; i < argc; i++) {
    // Parse port number argument
    if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")) {
      if (argc <= i + 1) {
        fprintf(stderr, "Missing value for port option.\n");
        return false;
      }

      options->server_port = atoi(argv[i + 1]);

      if (!options->server_port) {
        fprintf(stderr, "Invalid port specified.\n");
        return false;
      }
    }

    // Parse server address argument
    if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--address")) {
      if (argc <= i + 1) {
        fprintf(stderr, "Missing value for address option.\n");
        return false;
      }

      if (!validate_ip4_address(argv[i + 1], NULL)) {
        fprintf(stderr, "Invalid server address specified.\n");
        return false;
      }

      options->server_address = argv[i + 1];
    }

    // Parse DNS provider argument
    if (!strcmp(argv[i], "--provider")) {
      if (argc <= i + 1) {
        fprintf(stderr, "Missing value for DNS provider option.\n");
        return false;
      }

      if (!validate_ip4_address(argv[i + 1], &options->provider_address)) {
        fprintf(stderr, "Invalid DNS provider address specified.\n");
        return false;
      }
    }

    // Parse disable default filters argument
    if (!strcmp(argv[i], "--disable-defaults")) {
      options->disable_defaults = true;
    }

    // Parse blocklist path argument
    if (!strcmp(argv[i], "--blocklist")) {
      if (argc <= i + 1) {
        fprintf(stderr, "Missing value for blocklist path option.\n");
        return false;
      }

      options->blocklist = argv[i + 1];
    }

    if (!strcmp(argv[i], "--whitelist")) {
      if (argc <= i + 1) {
        fprintf(stderr, "Missing value for whitelist path option.\n");
        return false;
      }

      options->whitelist = argv[i + 1];
    }
  }

  return true;
}
