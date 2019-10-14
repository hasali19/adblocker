#include <stdbool.h>
#include <inttypes.h>

#include "set.h"
#include "map.h"

#define MAX_LISTS 100

/*
 * Auxiliary data structures describing a domain block list
 */
typedef struct {
  uint32_t id;
  char* name;
  char* pattern;
  char* delimiters;
  bool active;
  bool online;
  char* domain;
  char* path;
} AdListInfo;

typedef struct {
  Map *lists_map;
  uint32_t num_lists;
  uint32_t whitelists_lower_id;
  AdListInfo **lists;
} AdListsInfo;

/*
 * - Loads domains from a block list identified by name and stores them in domain_set
 * - Returns true on success, false on failure
 */
bool load_list_by_name(char *name, AdListsInfo *ad_lists, Set *domain_set);

/*
 * - Loads domains from a block list identified by id and stores them in domain_set
 * - Returns true on success, false on failure
 */
bool load_list_by_id(uint32_t id, AdListsInfo *ad_lists, Set *domain_set);

/*
 * - Loads domains from lists that are marked as active in AdListsInfo and stores them in domain_set
 * - Returns true on success, false on failure
 */
bool load_active_lists(AdListsInfo *ad_lists, Set *domain_set);

/*
 * Generates default AdListsInfo structure storing information about block lists
 * supported by default
 */
AdListsInfo *create_default_adlists_info(bool disable_defaults, char *blocklist, char *whitelist);

/*
 * Frees memory allocated for ad lists
 */
void free_adlists(AdListsInfo *ad_lists);
