#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct Map Map;

/*
 * Creates a new map on a heap
 */
Map *map_new(void);

/*
 * Searches for an item in a map, retrieving the value and returning true iff the key is found
 */
bool map_get(const Map *map, const char *key, uint32_t *value);

/*
 * Puts an item to a map with specified value
 */
void map_put(Map *map, const char *key, uint32_t value);

/*
 * Deletes the map and frees the space allocated for map on the heap
 */
void map_free(Map *map);

/*
 * - Deletes the map and frees the space allocated for map on the heap
 * - Frees the keys iff free_keys argument is true
 */
void map_free_opts(Map* map, bool free_keys);
