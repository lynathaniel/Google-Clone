# Bug 1

## A) How is your program acting differently than you expect it to?
- The hashtable built up in FileParser is storing "" (null) rather than
  the actual word that was found in the file.

## B) Brainstorm a few possible causes of the bug
- We are parsing the word incorrectly in InsertContent
- We did not allocate space for the word in the hashtable (gets freed)
- 

## C) How you fixed the bug and why the fix was necessary
- Alloc'd space and copied the word over to a new string. It was 
  necessary since the original word string would be freed after the
  call to AddWordPosition would end.


# Bug 2

## A) How is your program acting differently than you expect it to?
- Our maximum doc_id is greater than the total number of documents in
  DocTable.

## B) Brainstorm a few possible causes of the bug
- We are incrementing max doc_id too many times.
- We are storing the incorrect value doc_id value in our key for doctable.
- 

## C) How you fixed the bug and why the fix was necessary
- We were not storing a pointer to a doc_id, rather its dereferenced value. It was necessary since a pointer cast to an integer is much greater than the max doc_id in any table.


# Bug 3

## A) How is your program acting differently than you expect it to?
- Our program is double freeing a path_name

## B) Brainstorm a few possible causes of the bug
- We are not parsing path_names correctly
- Our program is freeing a path_name before it is done being used.
- 

## C) How you fixed the bug and why the fix was necessary
- Our InserContent method in FileParser was incorrectly parsing
  words to put in our WordPositions which would then cause memory
  issues, causing the program to jump around. We fixed it by rewriting our while loop to find alphabetic words in file_contents.
