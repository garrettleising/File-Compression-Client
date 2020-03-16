#include "code.h"
#include "io.h"
#include "trie.h"
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_LENGTH 4096
#define OPTIONS "vi:o:"
#define PERCENT 37

extern uint64_t concluding_filesize;

int main(int argc, char **argv) {
  int opt_value = 0;
  bool statistics = false;
  int infile_descriptor = STDIN_FILENO;
  char *input = NULL;
  int outfile_descriptor = STDOUT_FILENO;
  char *output = NULL;
  uint64_t original_filesize = 0;

  while ((opt_value = getopt(argc, argv, OPTIONS)) != -1) {
    switch (opt_value) {
    case 'v':
      statistics = true;
      break;
    case 'i':
      input = optarg;
      break;
    case 'o':
      output = optarg;
      break;
    default:
      break;
    }
  }

  if (input != NULL) {
    infile_descriptor = open(input, O_RDONLY);
    if (infile_descriptor < 0) {
      perror("Could not allocate infile");
      return EXIT_FAILURE;
    }
  }

  struct stat buffer;
  FileHeader *file_header = malloc(sizeof(FileHeader));

  fstat(infile_descriptor, &buffer);

  if (output != NULL) {
    outfile_descriptor =
        open(output, O_WRONLY | O_CREAT, file_header->protection);
    if (outfile_descriptor < 0) {
      perror("Could not allocate outfile");
      return EXIT_FAILURE;
    }
  }

  read_header(infile_descriptor, file_header);

  if (file_header != NULL) {
    if (file_header->magic != MAGIC) {
      printf("Bad magic number\n");
      return EXIT_FAILURE;
    }
  }

  WordTable *table = wt_create();
  uint16_t curr_code = 0;
  uint8_t curr_sym = 0;

  uint16_t next_code = START_CODE;

  uint16_t bit_length = log2(next_code) + 1;

  while (read_pair(infile_descriptor, &curr_code, &curr_sym, bit_length)) {
    table[next_code] = word_append_sym(table[curr_code], curr_sym);
    buffer_word(outfile_descriptor, table[next_code]);
    next_code++;
    if (next_code == MAX_CODE) {
      wt_reset(table);
      next_code = START_CODE;
    }
    if (next_code == 0) {
      bit_length = 1;
    } else {
      bit_length = log2(next_code) + 1;
    }
  }

  flush_words(outfile_descriptor);
  wt_delete(table);
  free(file_header);

  if (input != NULL) {
    close(infile_descriptor);
  }
  if (output != NULL) {
    close(outfile_descriptor);
  }

  if (statistics && output != NULL) {

    outfile_descriptor =
        open(output, O_WRONLY | O_CREAT, file_header->protection);
    if (outfile_descriptor < 0) {
      perror("Could not allocate outfile");
      return EXIT_FAILURE;
    }

    struct stat oribuffer;
    fstat(outfile_descriptor, &oribuffer);
    original_filesize = oribuffer.st_size;

    printf("Compressed file size: %lu bytes\n", concluding_filesize + 1);
    printf("Uncompressed file size: %lu bytes\n", original_filesize);
    if (original_filesize > 0) {
      printf("Compression ration: %.2f%c\n",
             100 * (1 - (float)concluding_filesize / (float)original_filesize),
             PERCENT);
    } else {
      printf("Compression ratio: 0%c\n", PERCENT);
    }

    if (output != NULL) {
      close(outfile_descriptor);
    }
  }

  return EXIT_SUCCESS;
}
