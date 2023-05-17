/*
 * Copyright ©2023 Justin Hsia.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include "./QueryProcessor.h"

#include <iostream>
#include <algorithm>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>

extern "C" {
  #include "./libhw1/CSE333.h"
}

using std::list;
using std::sort;
using std::string;
using std::vector;
using std::map;
using std::unordered_set;

namespace hw3 {

// Deletes pairs from the first map that do no intersect with the second map
//
// Returns true if returned map is non-empty; false otherwise
static map<string, int> IntersectMaps(const map<string, int>& map1,
                                      const map<string, int>& map2);

QueryProcessor::QueryProcessor(const list<string>& index_list, bool validate) {
  // Stash away a copy of the index list.
  index_list_ = index_list;
  array_len_ = index_list_.size();
  Verify333(array_len_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader* [array_len_];
  itr_array_ = new IndexTableReader* [array_len_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::const_iterator idx_iterator = index_list_.begin();
  for (int i = 0; i < array_len_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = fir.NewDocTableReader();
    itr_array_[i] = fir.NewIndexTableReader();
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (int i = 0; i < array_len_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

// This structure is used to store a index-file-specific query result.
typedef struct {
  DocID_t doc_id;  // The document ID within the index file.
  int     rank;    // The rank of the result so far.
} IdxQueryResult;

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string>& query) const {
  Verify333(query.size() > 0);

  // STEP 1.
  // (the only step in this file)
  vector<QueryProcessor::QueryResult> final_result;
  for (int curr_index = 0; curr_index < array_len_; curr_index++) {
    DocTableReader* curr_dtr = dtr_array_[curr_index];
    IndexTableReader* curr_itr = itr_array_[curr_index];
    map<string, int> curr_index_result;
    DocIDTableReader* didtr;
    list<DocIDElementHeader> word_postings;

    didtr = curr_itr->LookupWord(query[0]);

    // Word not found, move onto next index
    if (didtr == nullptr) {
      delete didtr;
      continue;
    }

    word_postings = didtr->GetDocIDList();
    // Store file name and word count for each file
    for (const DocIDElementHeader& word_posting : word_postings) {
      string file_name;
      curr_dtr->LookupDocID(word_posting.doc_id, &file_name);
      curr_index_result[file_name] = word_posting.num_positions;
    }

    delete didtr;

    // If query is done, insert results into final_result
    // and move onto next index
    if (query.size() == 1) {
      for (const auto& pair : curr_index_result) {
        QueryResult res;
        res.document_name = pair.first;
        res.rank = pair.second;
        final_result.push_back(res);
      }
      continue;
    }

    // Parse through the remaining words in the query
    for (int i = 1; i < static_cast<int>(query.size()); i++) {
      // Secondary map to store results for current word
      map<string, int> curr_word_result;
      didtr = curr_itr->LookupWord(query[i]);

      // Word not found, move onto next index
      if (didtr == nullptr) {
        delete didtr;
        curr_index_result.clear();
        break;
      }

      // Store file name and word count for each file
      word_postings = didtr->GetDocIDList();
      for (const DocIDElementHeader& word_posting : word_postings) {
        string file_name;
        curr_dtr->LookupDocID(word_posting.doc_id, &file_name);
        curr_word_result[file_name] = word_posting.num_positions;
      }
      delete didtr;
      // Find which files contain all proccessed words from the query
      curr_index_result = IntersectMaps(curr_index_result, curr_word_result);
      // Move onto next index if no files contain the query
      if (curr_index_result.size() == 0) {
        break;
      }
    }
    // Insert results into final_result and move onto next index
    if (curr_index_result.size() != 0) {
      for (const auto& pair : curr_index_result) {
        QueryResult res;
        res.document_name = pair.first;
        res.rank = pair.second;
        final_result.push_back(res);
      }
    }
  }

  // Sort the final results.
  sort(final_result.begin(), final_result.end());
  return final_result;
}

static map<string, int> IntersectMaps(const map<string, int>& map1,
                                      const map<string, int>& map2) {
  map<string, int> new_map;
  for (const auto& pair : map1) {
    string s = pair.first;
    // If both maps contain the same key, combine their
    // their values and insert into new map
    if (map2.count(s) == 1) {
      new_map[s] = pair.second + map2.at(s);
    }
  }
  return new_map;
}


}  // namespace hw3

