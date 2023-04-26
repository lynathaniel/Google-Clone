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
static void ProcessQueries(DocTable* dt, MemIndex* mi);
static int GetNextLine(FILE* f, char** ret_str);

static void LLPayloadFree(LLPayload_t payload) {
  free(payload);
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
  char* dir_path = argv[1];
  printf("Indexing '%s'\n", dir_path);
  if (!CrawlFileTree(dir_path, &doctable, &memindex)) {
    fprintf(stderr, "Could not open directory.\n");
    exit(EXIT_FAILURE);
  }

  // while user has not done ctrl-d
  while (1) {
    char input[MAX_QUERY_LENGTH];
    int test_input;
    printf("enter query:\n");
    if (!fgets(input, sizeof(input), stdin)) {
      fprintf(stderr, "Could not process query.\n");
      exit(EXIT_FAILURE);
    }

    // If a number is passed in to the input, fail.
    if (sscanf("%d", &test_input) == 1) {
      fprintf(stderr,"Invalid input.\n");
      exit(EXIT_FAILURE);
    }

    input[strcspn(input, "\n")] = 0;

    char* query_split[MAX_QUERY_LENGTH];
    int num_words = 0;
    char* word;
    char* input_copy = input;
    while ((word = strtok_r(input_copy, " ", &input_copy)) != NULL) {
      query_split[num_words] = word;
      num_words++;
    }
    
    LinkedList* results;
    LLIterator* lit;

    results = MemIndex_Search(memindex, query_split, num_words);
    lit = LLIterator_Allocate(results);
    for (int i = 0; i < LinkedList_NumElements(results); i++) {
      SearchResult* res;

      LLIterator_Get(lit, (LLPayload_t*) &res);
      char* doc_name = DocTable_GetDocName(doctable, res->doc_id);
      printf("  %s (%d)\n", doc_name, res->rank);
      LLIterator_Next(lit);
    }

    // free results
    LinkedList_Free(results, LLPayloadFree);
  }

  // free memindex, doctable
  DocTable_Free(doctable);
  MemIndex_Free(memindex);
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

static void ProcessQueries(DocTable* dt, MemIndex* mi) {
}

static int GetNextLine(FILE *f, char **ret_str) {
  return -1;  // you may want to change this
}
