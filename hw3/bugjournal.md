# Bug 1

## A) How is your program acting differently than you expect it to?
- After finishing WriteIndex and running the test suite, Test_WriteIndex.Basic 
  fails due to a mismatch between the actual file size and the header's stored
  offsets; the file size is 12 bytes less than what the header says it should be.

## B) Brainstorm a few possible causes of the bug
- We are writing too little bytes to the file (maybe a forgotten fwrite)
- We are incorrectly adding the bytes (mistyped return values)

## C) How you fixed the bug and why the fix was necessary
- We fixed the bug by tracing through the code and making sure
  every step for the hash table header, element header, and elements
  themselves were written correctly. Upon closer inspection, we realized that we were forgetting to write the element header for the docIDtable, which was 8 + 4 bytes. This accounted for the mismatch.


# Bug 2

## A) How is your program acting differently than you expect it to?
- Test_WriteIndex.Basic is failing; the index table checks    were failing due to the word positions not being properly inserted (word_positions[i] < word_positions[i + 1])

## B) Brainstorm a few possible causes of the bug
- Check that docIDtable elements are being written in 
  network order.
- Check that the positions are being truncated properly.


## C) How you fixed the bug and why the fix was necessary
- We fixed the bug by realizing that the header of each element within the docIDtable was not being written in network
order, which messed up all the bytes for the actual positions.


# Bug 3

## A) How is your program acting differently than you expect it to?
- When running QueryProcessor tests, queries are being output with the incorrect ranks.
  Additionally, running filesearchshell.cc, files that should not have matches when 
  compared to the program transcript provided in the spec.

## B) Brainstorm a few possible causes of the bug
- filesearchshell.cc is not properly tokenizing user input for QueryProcessor::ProcessQuery().
- QueryProcessor::ProcessQuery() is failing to update the vector that keeps track of which
  files still have valid matches from the query

## C) How you fixed the bug and why the fix was necessary
- We fixed this by making a vector of all the files that had matches with the current token, 
  then finding the intersection between this vector and another vector that has the files with
  matches with all previous tokens. The final results will then only have files that have all 
  the query tokens in it. This was necessary as the logic in ProcessQuery() originally was not 
  able to update this vector of files when a file no longer had matches.
