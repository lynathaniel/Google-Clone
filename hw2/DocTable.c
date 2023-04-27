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
#include "./DocTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libhw1/CSE333.h"
#include "libhw1/HashTable.h"

#define HASHTABLE_INITIAL_NUM_BUCKETS 2

// Deallocation function used for freeing the values
// within the name_to_id table.
static void FreeDocId(HTValue_t payload) {
  DocID_t* doc_id = (DocID_t*) payload;
  free(doc_id);
}

// Deallocation function used for freeing the values
// within the id_to_name table.
static void FreeDocName(HTValue_t payload) {
  char* doc_name = (char*) payload;
  free(doc_name);
}

// This structure represents a DocTable; it contains two hash tables, one
// mapping from document id to document name, and one mapping from
// document name to document id.
struct doctable_st {
  HashTable* id_to_name;  // mapping document id to document name
  HashTable* name_to_id;  // mapping document name to document id
  DocID_t    max_id;      // max doc ID allocated so far
};

DocTable* DocTable_Allocate(void) {
  DocTable* dt = (DocTable*) malloc(sizeof(DocTable));
  Verify333(dt != NULL);

  dt->id_to_name = HashTable_Allocate(HASHTABLE_INITIAL_NUM_BUCKETS);
  dt->name_to_id = HashTable_Allocate(HASHTABLE_INITIAL_NUM_BUCKETS);
  dt->max_id = 1;  // we reserve max_id = 0 for the invalid docID

  return dt;
}

void DocTable_Free(DocTable* table) {
  Verify333(table != NULL);

  // STEP 1.
  HashTable_Free(table->id_to_name, &FreeDocName);
  HashTable_Free(table->name_to_id, &FreeDocId);
  free(table);
}

int DocTable_NumDocs(DocTable* table) {
  Verify333(table != NULL);
  return HashTable_NumElements(table->id_to_name);
}

DocID_t DocTable_Add(DocTable* table, char* doc_name) {
  char *doc_copy;
  DocID_t *doc_id;
  DocID_t res;
  HTKeyValue_t kv, old_kv;

  Verify333(table != NULL);

  // STEP 2.
  // Check to see if the document already exists.  Then make a copy of the
  // doc_name and allocate space for the new ID.
  res = DocTable_GetDocID(table, doc_name);
  if (res != INVALID_DOCID) {
    return res;
  }

  // Malloc space for key/value copies.
  doc_copy = (char*) malloc(strlen(doc_name) + 1);
  Verify333(doc_copy != NULL);
  strncpy(doc_copy, doc_name, strlen(doc_name) + 1);
  doc_id = (DocID_t*) malloc(sizeof(DocID_t));
  Verify333(doc_id != NULL);

  *doc_id = table->max_id;
  table->max_id++;

  // STEP 3.
  // Set up the key/value for the id->name mapping, and do the inser
  kv.key = (HTKey_t) *doc_id;
  kv.value = (HTValue_t) doc_copy;
  HashTable_Insert(table->id_to_name, kv, &old_kv);

  // STEP 4.
  // Set up the key/value for the name->id, and/ do the insert.
  // Be careful about how you calculate the key for this mapping.
  // You want to be sure that how you do this is consistent with
  // the provided code.
  kv.key = FNVHash64((unsigned char*) doc_name, strlen(doc_name));
  kv.value = (HTValue_t) doc_id;
  HashTable_Insert(table->name_to_id, kv, &old_kv);

  return *doc_id;
}

DocID_t DocTable_GetDocID(DocTable* table, char* doc_name) {
  HTKey_t key;
  HTKeyValue_t kv;
  DocID_t res;

  Verify333(table != NULL);
  Verify333(doc_name != NULL);

  // STEP 5.
  // Try to find the passed-in doc in name_to_id table.
  key = FNVHash64((unsigned char*) doc_name, strlen(doc_name));
  bool found = HashTable_Find(table->name_to_id, key, &kv);
  if (found) {
    return *((DocID_t*) kv.value);
  }

  return INVALID_DOCID;
}

char* DocTable_GetDocName(DocTable* table, DocID_t doc_id) {
  HTKeyValue_t kv;

  Verify333(table != NULL);
  Verify333(doc_id != INVALID_DOCID);

  // STEP 6.
  // Lookup the doc_id in the id_to_name table,
  // and either return the string (i.e., the (char *)
  // saved in the value field for that key) or
  // NULL if the key isn't in the table.
  bool found = HashTable_Find(table->id_to_name, (HTKey_t) doc_id, &kv);
  if (found) {
    return (char*) kv.value;
  }

  return NULL;
}

HashTable* DT_GetIDToNameTable(DocTable* table) {
  Verify333(table != NULL);
  return table->id_to_name;
}

HashTable* DT_GetNameToIDTable(DocTable* table) {
  Verify333(table != NULL);
  return table->name_to_id;
}
