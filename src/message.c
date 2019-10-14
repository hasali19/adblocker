#include "message.h"

#include <arpa/inet.h>

#include "utils.h"

void set_message_address(Address *result, uint16_t port, uint32_t address) {
  result->port = htons(port);
  result->address = htonl(address);
}

void set_message_address_str(Address *result, uint16_t port, const char *address) {
  result->port = htons(port);
  result->address = inet_addr(address);
}

uint16_t get_qtype_code(Message *message) {
  uint16_t qtype_code = 0;
  uint8_t curr = QUESTION_START_BYTE;
  // If not zero, check next
  while(message->data[curr]) {
    curr++;
  }
  // Current byte has value 0, so next 2 bytes hold QTYPE
  qtype_code = *(uint16_t *)(&message->data[curr + 1]);
  return qtype_code;
}

char *parse_dns_domain(const Message *message, size_t *length) {
  char *domain = NULL;
  const uint8_t *cur = &message->data[QUESTION_START_BYTE];

  *length = 0;

  while (*cur) {
    if (domain) {
      // Allocate space for (current label) + (.) + (next label) + (\0)
      domain = realloc(domain, *length + 1 + *cur + 1);
      domain[(*length)++] = '.';
    } else {
      domain = malloc(*cur + 1);
    }

    CHECK_ALLOC(domain);

    memcpy(domain + *length, cur + 1, *cur);
    *length += *cur;
    domain[*length] = '\0';
    cur += *cur + 1;
  }

  *length += 2;
  return domain;
}
