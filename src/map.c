#include "map.h"

#include <string.h>
#include <stdlib.h>

#include "utils.h"

#define INCREASE_FACTOR 2
#define START_SIZE 6
#define LOAD_FACTOR 0.75

typedef struct Entry Entry;

struct Entry {
  const char *key;
  uint32_t value;
  Entry *next;
};

struct Map {
  Entry **entries;
  size_t entries_count;
  size_t bucket_count;
};

/*
 * Increase the size of the hash map by factor of INCREASE_FACTOR
 */
void map_increase(Map *map) {
  const size_t bucket_count = map->bucket_count;
  // Create a new entries array with double the previous size
  Entry **new_entries = calloc(bucket_count * 2, sizeof(Entry *));
  CHECK_ALLOC(new_entries);
  Entry **old_entries = map->entries;

  map->entries = new_entries;
  map->entries_count = 0;
  map->bucket_count = bucket_count * INCREASE_FACTOR;

  // Recurse through the old entries and put them all into the map
  for (size_t i = 0; i < bucket_count; i++) {
    Entry *entry = old_entries[i];
    while (entry != NULL) {
      map_put(map, entry->key, entry->value);
      Entry *old_entry = entry;
      entry = old_entry->next;
      free(old_entry);
    }
  }

  free(old_entries);
}

Map *map_new(void) {
  Entry **entries = calloc(START_SIZE, sizeof(Entry *));
  CHECK_ALLOC(entries);
  Map *map = malloc(sizeof(Map));
  CHECK_ALLOC(map);

  map->entries = entries;
  map->entries_count = 0;
  map->bucket_count = START_SIZE;
  return map;
}

void map_free(Map *map) {
  map_free_opts(map, false);
}

void map_free_opts(Map *map, bool free_keys) {
  for (int i = 0; i < map->bucket_count; i++) {
    Entry *entry = map->entries[i];
    while (entry != NULL) {
      Entry *old_entry = entry;
      entry = old_entry->next;
      if (free_keys) {
        free((char*) old_entry->key);
      }
      free(old_entry);
    }
  }

  free(map->entries);
  free(map);
}

bool map_get(const Map *map, const char *key, uint32_t *value) {
  const size_t bucket = hash(key) % (map->bucket_count);
  // Pulls the entry from the bucket we expect the symbol to be in
  Entry *entry = map->entries[bucket];
  while (entry != NULL) {
    // Check if the key matches
    if (!strcmp(entry->key, key)) {
      *value = entry->value;
      return true;
    } else {
      // If the key doesn't match move along the linked list
      entry = entry->next;
    }
  }

  // No entry with searched key exists
  return false;
}

void map_put(Map *map, const char *key, uint32_t value) {
  // If we have too many entries for our map, increase the size
  if ((((float)map->entries_count+1) / (float)map->bucket_count) > LOAD_FACTOR) {
    map_increase(map);
  }

  const size_t bucket = hash(key) % (map -> bucket_count);
  // Initialize new_entry_ptr with the address of a pointer to the first entry of the bucket
  Entry **new_entry_ptr = (map->entries) + bucket;
  // Initialize entry with pointer to the first entry of the bucket
  Entry *entry = (map->entries)[bucket];
  while (entry != NULL) {
    if (!strcmp(entry->key, key)) {
      // Entry with inserted key already exists - update
      entry->value = value;
      return;
    }

    // Get pointer to the next entry
    Entry *next = entry->next;
    if (next == NULL) {
      // Current entry does not have a next entry set - save pointer to the next field
      new_entry_ptr = &(entry->next);
      break;
    }
    // Move to next entry
    entry = next;
  }

  // Set pointer to the newly created entry
  *new_entry_ptr = malloc(sizeof(Entry));
  CHECK_ALLOC(*new_entry_ptr);
  // Initialize the newly created entry
  Entry *new_entry = *new_entry_ptr;
  new_entry->key = key;
  new_entry->value = value;
  new_entry->next = NULL;
  map->entries_count++;
}
