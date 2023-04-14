// Copyright 2023 Basit Ali
// Email: basitali@uw.edu
#include <stdio.h>    // for snprintf
#include <stdlib.h>   // for EXIT_SUCCESS, NULL
#include <string.h>   // for strrchr, strcmp, strlen
#include <stdbool.h>  // for bool
#include <dirent.h>

#include "ro_file.h"

#define READSIZE 128

/*** HELPER FUNCTION DECLARATIONS ******************************************/

// Returns whether or not a filename ends in ".txt".
bool IsTxtFile(char* filename);

// Concatenate the directory and file names into a full path. The caller is
// responsible for freeing the allocated string. Exits if an error occurs.
char* Concatenate(char* dirname, char* filename);


/*
 * This program:
 * - Accepts a directory name as a command-line argument.
 * - Scans through the directory looking for all files whose names end with
 *   the four characters ".txt".
 * - For every such file found, write the contents of those files to stdout
 *   without adding any additional characters or formatting.
 *   Eventually reading the files with ro_file module.
 */
int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: ./ex4 <dir>\n");
    return EXIT_FAILURE;
  }

  DIR* dirp = opendir(argv[1]);
  if (dirp == NULL) {
    fprintf(stderr, "Could not open directory\n");
    return EXIT_FAILURE;
  }

  struct dirent* entry;
  entry = readdir(dirp);
  while (entry != NULL) {
    if (IsTxtFile(entry->d_name)) {
      char buf[READSIZE];
      FILE* curr_file;
      size_t readlen;
      char* full_path = Concatenate(argv[1], entry->d_name);
      curr_file = fopen(full_path, "r");
      free(full_path);

      if (curr_file == NULL) {
        perror("fopen for read failed.");
        closedir(dirp);
        return EXIT_FAILURE;
      }

      while ((readlen = fread(buf, 1, READSIZE, curr_file)) > 0) {
        if (ferror(curr_file)) {
          perror("fread failed.");
          fclose(curr_file);
          closedir(dirp);
          return EXIT_FAILURE;
        }

        if (fwrite(buf, 1, readlen, stdout) < readlen) {
          perror("fwrite failed");
          fclose(curr_file);
          closedir(dirp);
          return EXIT_FAILURE;
        }
      }

      fclose(curr_file);
    }

    entry = readdir(dirp);
  }

  closedir(dirp);
  return EXIT_SUCCESS;
}


/*** HELPER FUNCTION DEFINITIONS *******************************************/

bool IsTxtFile(char* filename) {
  char* dot = strrchr(filename, '.');
  return dot && !strcmp(dot, ".txt");
}

char* Concatenate(char* dirname, char* filename) {
  bool has_trailing_slash = dirname[strlen(dirname) - 1] == '/';
  // Concatenate directory and file name.
  size_t dlen = strlen(dirname);
  size_t flen = strlen(filename);
  // Malloc space for full path name:
  // dlen + strlen("/") + flen + strlen('\0') = dlen + flen + 2
  int size_to_malloc = has_trailing_slash ? dlen + flen + 1 : dlen + flen + 2;
  char* fullpath = (char*) malloc(sizeof(char) * (size_to_malloc));
  if (fullpath == NULL) {
    fprintf(stderr, "Error on malloc.\n");
    exit(EXIT_FAILURE);
  }
  if (has_trailing_slash) {
    snprintf(fullpath, size_to_malloc, "%s%s", dirname, filename);
  } else {
    snprintf(fullpath, size_to_malloc, "%s/%s", dirname, filename);
  }
  return fullpath;
}
