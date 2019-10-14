#pragma once

#include <stdbool.h>

typedef struct Set Set;

/*
 * Creates a new set on a heap
 */
Set *set_new(void);

/*
 * Searches for an item in a set and returns true iff it's in the set
 */
bool set_contains(const Set *set, const char *value);

/*
 * Adds an item to the set
 */
void set_add(Set *set, char *value);

/*
 * Removes an item from the set, freeing entry and optionally value stored in the entry
 */
void set_remove(Set *set, char *value, bool free_val);

/*
  * Deletes the map, preserving the elements allocated within it
 */
void set_free(Set *set);

/*
 * Deletes the map and all the values stored in it
 */
void set_free_vals(Set *set);