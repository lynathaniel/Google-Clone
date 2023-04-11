# Bug 1

## A) How is your program acting differently than you expect it to?
- I am getting a segfault inside LinkedList_Remove, when I set the 
  "prev" value of the node following the head to NULL.

## B) Brainstorm a few possible causes of the bug
- Segfaults are usually because of a bad memory access.
- I need to check if I somehow messed up freeing or
  setting pointers improperly.

## C) How you fixed the bug and why the fix was necessary
- I was able to fix this bug by figuring out when I am 
  actually supposed to free the node that is being removed.
  Since I was freeing the node at the beginning of the method,
  any access (like node->next) would result in a segfault, so
  all I did was free the node at the end of the method.


# Bug 2

## A) How is your program acting differently than you expect it to?
- I am failing the assertion at line 240 of test_hashtable.cc. This
  assertion makes sure that the key the HTIterator is currently pointing
  at is being seen for the first time after insertion.  

## B) Brainstorm a few possible causes of the bug
- There has to be something wrong with HTIterator_Get.
- Maybe I am not properly storing the current key in the output parameter.
- Or something is wrong with HTIterator_Next; it could be not moving to
  the next node correctly.

## C) How you fixed the bug and why the fix was necessary
- I was able to fix this bug by debugging and tracing through the 
  code execution. What I found was that I had not properly implemented
  HTIterator_Next; when I had changed iter->bucket_it, I had not 
  also updated iter->curr_idx, so it would be looking at the beginning
  of the current chain again. By updating the metadata and also
  refactoring my code to utilize a for-loop, I was able to get the 
  iterator to actually go through the elements in the table correctly.


# Bug 3

## A) How is your program acting differently than you expect it to?
- I ran valgrind on the test suite, which is detecting memory leaks
  in HashTable_Insert, specifically when I malloc a copy of the 
  newkeyvalue. 

## B) Brainstorm a few possible causes of the bug
- I need to figure out why the copy of newkeyvalue is not being freed. 
- Nodes are removed only in FindKey(), so maybe look there.
- Check calls to LLIterator_Remove to determine if something is messed up.

## C) How you fixed the bug and why the fix was necessary
- I was able to fix this bug by changing the second argument to my call
  to LLIterator_Remove within FindKey(). Previously, I had it set to the
  function pointer LLNoOpFree, which didn't do anything. So, when
  LLIterator_Remove called LLNoOpFree, nothing would actually be freed.
  Therefore, I had to create a new static function called LLFreePayload
  that would actually free the payload (which was the copy of newkeyvalue).
