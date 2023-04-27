/*
 * Copyright ©2023 Chris Thachuk.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc
#define MAX_QUERY_WORDS 255
#define MAX_QUERY_LENGTH 1024

static void Usage(void);

// Processes a query to the inverted index.
//
// Arguments:
// - dt: the DocTable created from the root directory.
// - mi: the MemIndex created from the root directory.

// Returns:
// - NULL: if no matching documents were found
// - a non-NULL LinkedList of SearchResult's
static LinkedList* ProcessQueries(DocTable* dt, MemIndex* mi);

// Converts a string to lower-case and removes the newline.
//
// Arguments:
// - str: the string to normalized.
static void NormalizeString(char* str);

// Deallocation function usable in LinkedList_Free(). Used to
// free the list of SearchResult's returned by ProcessQueries.
static void FreeSearchResult(LLPayload_t payload) {
  SearchResult* sr = (SearchResult*) payload;
  free(sr);
}


//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char** argv) {
  if (argc != 2) {
    Usage();
  }

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - Crawl from a directory provided by argv[1] to produce and index
  //  - Prompt the user for a query and read the query from stdin, in a loop
  //  - Split a query into words (check out strtok_r)
  //  - Process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.
  //
  // Note that you should make sure the fomatting of your
  // searchshell output exactly matches our solution binaries
  // to get full points on this part.
  DocTable* doctable;
  MemIndex* memindex;
  LinkedList* results;
  LLIterator* results_itr;
  SearchResult* res;
  char* dir_path = argv[1];

  // Go through the file tree, and populate our inverted index.
  printf("Indexing '%s'\n", dir_path);
  if (!CrawlFileTree(dir_path, &doctable, &memindex)) {
    fprintf(stderr, "Could not open directory.\n");
    exit(EXIT_FAILURE);
  }

  // Keep reading until the user has passed in EoF to stdin.
  while (true) {
    results = ProcessQueries(doctable, memindex);
    if (results == NULL) {
      continue;
    }

    // Print out results for user.
    results_itr = LLIterator_Allocate(results);
    for (int i = 0; i < LinkedList_NumElements(results); i++) {
      LLIterator_Get(results_itr, (LLPayload_t*) &res);
      char* doc_name = DocTable_GetDocName(doctable, res->doc_id);
      printf("  %s (%d)\n", doc_name, res->rank);
      LLIterator_Next(results_itr);
    }

    // Free results for the next query to be handled.
    LLIterator_Free(results_itr);
    LinkedList_Free(results, FreeSearchResult);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}

static LinkedList* ProcessQueries(DocTable* dt, MemIndex* mi) {
  char input[MAX_QUERY_LENGTH];
  char* word;
  LinkedList* results;

  printf("enter query:\n");
  if (!fgets(input, sizeof(input), stdin)) {
    // The user has passed in EoF, free all relevant memory.
    printf("shutting down...\n");
    DocTable_Free(dt);
    MemIndex_Free(mi);
    exit(EXIT_SUCCESS);
  }

  // If the user has passed in any numbers, fail.
  if (sscanf("%d", &input) == 1) {
    return NULL;
  }

  NormalizeString(input);
  char** query_split = (char**) malloc(sizeof(char*) * 1024);
  Verify333(query_split != NULL);

  int num_words = 0;
  char* input_copy = input;
  while ((word = strtok_r(input_copy, " ", &input_copy)) != NULL) {
    query_split[num_words] = word;
    num_words++;
  }

  results = MemIndex_Search(mi, query_split, num_words);
  free(query_split);
  return results;
}

static void NormalizeString(char* str) {
  for (int i = 0; str[i]; i++) {
    str[i] = tolower(str[i]);
  }

  str[strcspn(str, "\n")] = 0;
}
