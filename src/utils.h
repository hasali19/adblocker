#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>

#define DEFAULT_DNS_PORT 53

typedef struct {
  uint16_t server_port;
  const char *server_address;
  uint32_t provider_address;
  bool disable_defaults;
  char *blocklist;
  char *whitelist;
} ProgramOptions;

/*
 * Compute hash using djb2 algorithm created by Daniel J. Bernstein
 */
uint64_t hash(const char *str);

/*
 * Prints the formated error message to stderr and exits the program.
 */
noreturn void fatal_error(const char *fmt, ...);

/*
 * Checks that a memory allocation was successful, exiting if not.
 */
#define CHECK_ALLOC(ptr) \
  do { \
    if (!(ptr)) { \
      fatal_error("Failed to allocate memory at: %s:%s:%d", __FILE__, __func__, __LINE__); \
    } \
  } while (0)

/*
 * Parses options passed on the command line, returning false on failure.
 */
bool parse_options(int argc, char **argv, ProgramOptions *options);
