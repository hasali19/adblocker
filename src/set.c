#include "set.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "utils.h"

#define INCREASE_FACTOR 2
#define START_SIZE 6
#define LOAD_FACTOR 0.75

typedef struct Entry Entry;

struct Entry {
  char *value;
  Entry *next;
};

struct Set {
  Entry **entries;
  size_t entries_count;
  size_t bucket_count;
};

/*
 * Increase the size of the hash map by factor of INCREASE_FACTOR
 */
void set_increase(Set *set) {
  const size_t bucket_count = set->bucket_count;
  // Create a new entries array with double the previous size
  Entry **new_entries = calloc(bucket_count * 2, sizeof(Entry *));
  CHECK_ALLOC(new_entries);
  Entry **old_entries = set->entries;

  set->entries = new_entries;
  set->entries_count = 0;
  set->bucket_count = bucket_count * INCREASE_FACTOR;

  // Recurse through the old entries and put them all into the map
  for (size_t i = 0; i < bucket_count; i++) {
    Entry *entry = old_entries[i];
    while (entry != NULL) {
      set_add(set, entry->value);
      Entry *old_entry = entry;
      entry = old_entry->next;
      free(old_entry);
    }
  }

  free(old_entries);
}

Set *set_new(void) {
  Entry **entries = calloc(START_SIZE, sizeof(Entry *));
  CHECK_ALLOC(entries);
  Set *set = malloc(sizeof(Set));
  CHECK_ALLOC(set);

  set->entries = entries;
  set->entries_count = 0;
  set->bucket_count = START_SIZE;
  return set;
}

void set_free_opts(Set *set, bool free_vals) {
  for (int i = 0; i < set->bucket_count; i++) {
    Entry *entry = set->entries[i];
    while (entry != NULL) {
      Entry *old_entry = entry;
      entry = old_entry->next;
      if (free_vals) {
        free((char*) old_entry->value);
      }
      free(old_entry);
    }
  }

  free(set->entries);
  free(set);
}

void set_free(Set *set) {
  set_free_opts(set, false);
}

void set_free_vals(Set *set) {
    set_free_opts(set, true);
}



bool set_contains(const Set *set, const char *value) {
  const size_t bucket = hash(value) % (set->bucket_count);
  // Pulls the entry from the bucket we expect the symbol to be in
  Entry *entry = set->entries[bucket];
  while (entry != NULL) {
    // Check if the value matches
    if (!strcmp(entry->value, value)) {
      return true;
    } else {
      // If the value doesn't match move along the linked list
      entry = entry->next;
    }
  }

  // No entry with searched value exists
  return false;
}

void set_add(Set *set, char *value) {
  // If we have too many entries for our map, increase the size
  if ((((float)set->entries_count+1) / (float)set->bucket_count) > LOAD_FACTOR) {
    set_increase(set);
  }

  const size_t bucket = hash(value) % (set -> bucket_count);
  // Initialize new_entry_ptr with the address of a pointer to the first entry of the bucket
  Entry **new_entry_ptr = (set->entries) + bucket;
  // Initialize entry with pointer to the first entry of the bucket
  Entry *entry = (set->entries)[bucket];
  while (entry != NULL) {
    if (!strcmp(entry->value, value)) {
      // Entry with inserted value already exists - update
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
  new_entry->value = value;
  new_entry->next = NULL;
  set->entries_count++;
}

void set_remove(Set *set, char *value, bool free_val) {
  const size_t bucket = hash(value) % (set -> bucket_count);
  // Initialize pointers required for removal
  Entry *pred = NULL;
  Entry *curr = (set->entries)[bucket];
  // Locate removed entry
  while (curr != NULL) {
    if (!strcmp(curr->value, value)) {
      // Found searched entry
      break;
    }

    pred = curr;
    curr = curr->next;
  }

  if (curr == NULL) {
    // Removed entry is not in the set
    return;
  }

  if (pred == NULL) {
    // Removed entry is at the beginning of the bucket list
    set->entries[bucket] = curr->next;
  } else {
    // Removed entry has a predecessor - update pointers to exclude curr from the list
    pred->next = curr->next;
  }
  if (free_val) {
    free(curr->value);
  }
  free(curr);
}
