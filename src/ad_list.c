#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ad_list.h"
#include "utils.h"
#include "string.h"

/*
 * Downloads file from a given address and saves it to dest
 * Returns true on success, false on failure
 */
bool download_file(const char *url, const char *dest) {

  // Backup original file contents prior to download
  FILE *file = fopen(dest, "rb");
  uint8_t *contents = NULL;
  size_t file_size;
  bool file_exists = false;
  if (file) {
    file_exists = true;
    if (fseek(file, 0, SEEK_END)) {
      fprintf(stderr, "[AdList] Failed to seek end of file %s", dest);
      fclose(file);
      return false;
    }

    file_size = ftell(file);
    rewind(file);
    contents = malloc(file_size);
    CHECK_ALLOC(contents);
    fread(contents, 1, file_size, file);
    fclose(file);
  }

  // Overwrite the file with downloaded contents
  file = fopen(dest, "wb");

  if (!file) {
    fprintf(stderr, "[AdList] Failed to open file %s!\n", dest);
    free(contents);
    return false;
  }

  // Set up curl and set its options
  CURL *curl = curl_easy_init();
  if (!curl) {
    perror("[AdList] Failed to initialize curl easy handle!\n");
    free(contents);
    fclose(file);
    return false;
  }
  char error_buffer[CURL_ERROR_SIZE];
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1l);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2l);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5l);
  if (curl_easy_perform(curl)) {
    fprintf(stderr, "[AdList] An error occurred while downloading remote file from %s!\n", url);
    fprintf(stderr, "[AdList] curl perform operation failed with an error: %s\n", error_buffer);
    // Restore original contents of the file if the download failed
    if (file_exists) {
      fwrite(contents, 1, file_size, file);
    }
    fclose(file);
    curl_easy_cleanup(curl);
    free(contents);
    return false;
  }

  fclose(file);
  curl_easy_cleanup(curl);
  free(contents);
  return true;
}

AdListsInfo *create_default_adlists_info(bool disable_defaults, char *blocklist, char *whitelist) {
  AdListsInfo *ad_lists = (AdListsInfo *) malloc(sizeof(AdListsInfo));
  CHECK_ALLOC(ad_lists);
  Map *lists_map = map_new();
  AdListInfo **lists = (AdListInfo **) calloc(MAX_LISTS, sizeof(AdListInfo *));
  ad_lists->lists_map = lists_map;
  ad_lists->num_lists = 6;
  ad_lists->whitelists_lower_id = 5;
  ad_lists->lists = lists;

  uint32_t list_id = 0;
  map_put(lists_map, "custom_blocklist", list_id);
  AdListInfo *custom_blocklist = (AdListInfo *) malloc(sizeof(AdListInfo));
  CHECK_ALLOC(custom_blocklist);
  custom_blocklist->id = list_id;
  custom_blocklist->name = "custom_blocklist";
  custom_blocklist->pattern = "%[^\n\r]%c";
  custom_blocklist->delimiters = "\n\r";
  custom_blocklist->active = true;
  custom_blocklist->online = false;
  custom_blocklist->domain = NULL;
  if (blocklist) {
    custom_blocklist->path = blocklist;
  } else {
    custom_blocklist->path = "./lists/blocklist.txt";
  }
  lists[list_id] = custom_blocklist;

  if (!disable_defaults) {
    list_id = 1;
    map_put(lists_map, "easylist_adservers", list_id);
    AdListInfo *easylist_adservers = (AdListInfo *) malloc(sizeof(AdListInfo));
    CHECK_ALLOC(easylist_adservers);
    easylist_adservers->id = list_id;
    easylist_adservers->name = "easylist_adservers";
    easylist_adservers->pattern = "||%[^^\n\r/|]%c";
    easylist_adservers->delimiters = "^";
    easylist_adservers->active = true;
    easylist_adservers->online = true;
    easylist_adservers->domain = "https://raw.githubusercontent.com/easylist/easylist/master/easylist/easylist_adservers.txt";
    easylist_adservers->path = "./lists/easylist_adservers.txt";
    lists[list_id] = easylist_adservers;

    list_id = 2;
    map_put(lists_map, "yoyo_adservers_hosts", list_id);
    AdListInfo *yoyo_adservers_hosts = (AdListInfo *) malloc(sizeof(AdListInfo));
    CHECK_ALLOC(yoyo_adservers_hosts);
    yoyo_adservers_hosts->id = list_id;
    yoyo_adservers_hosts->name = "yoyo_adservers_hosts";
    yoyo_adservers_hosts->pattern = "127.0.0.1 %[^\n\r/]%c";
    yoyo_adservers_hosts->delimiters = "\n";
    yoyo_adservers_hosts->active = true;
    yoyo_adservers_hosts->online = true;
    yoyo_adservers_hosts->domain = "https://pgl.yoyo.org/adservers/serverlist.php?hostformat=hosts&showintro=0&mimetype=plaintext";
    yoyo_adservers_hosts->path = "./lists/yoyo_adservers_hosts.txt";
    lists[list_id] = yoyo_adservers_hosts;

    list_id = 3;
    map_put(lists_map, "easylist_thirdparty", list_id);
    AdListInfo *easylist_thirdparty = (AdListInfo *) malloc(sizeof(AdListInfo));
    CHECK_ALLOC(easylist_adservers);
    easylist_thirdparty->id = list_id;
    easylist_thirdparty->name = "easylist_thirdparty";
    easylist_thirdparty->pattern = "||%[^^\n\r/|]%c";
    easylist_thirdparty->delimiters = "^";
    easylist_thirdparty->active = false;
    easylist_thirdparty->online = true;
    easylist_thirdparty->domain = "https://raw.githubusercontent.com/easylist/easylist/master/easylist/easylist_thirdparty.txt";
    easylist_thirdparty->path = "./lists/easylist_thirdparty.txt";
    lists[list_id] = easylist_thirdparty;

    list_id = 4;
    map_put(lists_map, "stevenblack", list_id);
    AdListInfo *stevenblack_hosts = (AdListInfo *) malloc(sizeof(AdListInfo));
    CHECK_ALLOC(stevenblack_hosts);
    stevenblack_hosts->id = list_id;
    stevenblack_hosts->name = "stevenblack";
    stevenblack_hosts->pattern = "0.0.0.0 %[^\n\r/]%c";
    stevenblack_hosts->delimiters = "\n";
    stevenblack_hosts->active = true;
    stevenblack_hosts->online = true;
    stevenblack_hosts->domain = "https://raw.githubusercontent.com/StevenBlack/hosts/master/hosts";
    stevenblack_hosts->path = "./lists/stevenblack.txt";
    lists[list_id] = stevenblack_hosts;
  }

  list_id = 5;
  map_put(lists_map, "custom_whitelist", list_id);
  AdListInfo *custom_whitelist = (AdListInfo *) malloc(sizeof(AdListInfo));
  CHECK_ALLOC(custom_whitelist);
  custom_whitelist->id = list_id;
  custom_whitelist->name = "custom_whitelist";
  custom_whitelist->pattern = "%[^\n\r]%c";
  custom_whitelist->delimiters = "\n\r";
  custom_whitelist->active = true;
  custom_whitelist->online = false;
  custom_whitelist->domain = NULL;
  if (whitelist) {
    custom_whitelist->path = whitelist;
  } else {
    custom_whitelist->path = "./lists/whitelist.txt";
  }
  lists[list_id] = custom_whitelist;

  return ad_lists;
}

void free_adlists(AdListsInfo *ad_lists) {
  for (uint32_t id = 0; id < ad_lists->num_lists; id++) {
    free(ad_lists->lists[id]);
  }
  free(ad_lists->lists);
  map_free(ad_lists->lists_map);
  free(ad_lists);
}

bool load_list_by_id(uint32_t id, AdListsInfo *ad_lists, Set *domain_set) {
  if (id >= ad_lists->num_lists) {
    fprintf(stderr, "[AdList] No filter with id %" SCNu32, id);
    return false;
  }

  struct stat s = {0};
  if (stat("./lists", &s) == -1) {
    mkdir("./lists", 0755);
    fprintf(stderr, "[AdList] The lists directory did not exist and was automatically created!\n");
  }

  AdListInfo *ad_list_info = ad_lists->lists[id];
  char *path = ad_list_info->path;
  char *ad_list_name = ad_list_info->name;
  if (ad_list_info->online) {
    // Loaded list has online version - attempt to update
    char *domain = ad_list_info->domain;
    if (!download_file(domain, path)) {
      fprintf(stderr, "[AdList] Could not update ad list %s, blocking rules might be obsolete!\n",
          ad_list_name);
    }
  }

  FILE *file = fopen(path, "r");

  if (!file) {
    fprintf(stderr, "[AdList] Failed to open file with blocking rules %s!\n", path);
    return false;
  }

  // Load all domains matching the pattern to the set
  char buffer[500];
  char curr_domain[270];
  char *pattern = ad_list_info->pattern;
  char *delimiters = ad_list_info->delimiters;
  char next_char;
  uint32_t whitelist_lower_id = ad_lists->whitelists_lower_id;
  while (fgets(buffer, sizeof(buffer), file)) {
    if (sscanf(buffer, pattern, curr_domain, &next_char) && strchr(delimiters, next_char)) {
      char *domain_string = calloc(strlen(curr_domain) + 1, sizeof(char));
      strcpy(domain_string, curr_domain);
      if (id < whitelist_lower_id) {
        if (set_contains(domain_set, domain_string)) {
          free(domain_string);
          continue;
        }
        set_add(domain_set, domain_string);
      } else {
        set_remove(domain_set, domain_string, true);
        free(domain_string);
      }
    }
  }

  // Check that whole file has been read
  if (!feof(file)) {
    fprintf(stderr, "[AdList] Failed to read the whole blocking rules file %s!\n", path);
    fclose(file);
    return false;
  }

  fclose(file);
  return true;
}

bool load_list_by_name(char *name, AdListsInfo *ad_lists, Set *domain_set) {
  uint32_t id;
  if (!map_get(ad_lists->lists_map, name, &id)) {
    fprintf(stderr, "[AdList] Unknown filter name %s!\n", name);
    return false;
  }
  return load_list_by_id(id, ad_lists, domain_set);
}

bool load_active_lists(AdListsInfo *ad_lists, Set *domain_set) {
  bool success = true;
  for (uint32_t id = 0; id < ad_lists->num_lists; id++) {
    if (ad_lists->lists[id]->active) {
      success = load_list_by_id(id, ad_lists, domain_set) && success;
    }
  }
  return success;
}
