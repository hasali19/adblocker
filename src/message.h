#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MAX_MESSAGE_LENGTH 512
#define QUESTION_START_BYTE 12

typedef struct {
  uint16_t port;
  uint32_t address;
} Address;

typedef struct {
  union {
    Address sender;
    Address recipient;
  };
  uint8_t data[MAX_MESSAGE_LENGTH];
  size_t length;
} Message;

/*
 * Sets the address and port in an Address structure. The values should
 * be passed in the host byte order and will be set in network byte order.
 */
void set_message_address(Address *result, uint16_t port, uint32_t address);

/*
 * Sets the address and port in an Address structure. The values should
 * be passed in the host byte order and will be set in network byte order.
 * The address should be specified in the convential numbers-and-dots notation.
 */
void set_message_address_str(Address *result, uint16_t port, const char* address);

/*
 * Get the QTYPE code from a DNS request.
 */
uint16_t get_qtype_code(Message *message);

/*
 * Parses the domain name from a DNS request.
 */
char *parse_dns_domain(const Message *message, size_t *length);
