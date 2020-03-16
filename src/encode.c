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
#define PERCENT 37
#define OPTIONS "vi:o:"

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

  if (file_header == NULL) {
    perror("Failed to write file header");
    return EXIT_FAILURE;
  }

  file_header->magic = MAGIC;
  file_header->protection = buffer.st_mode;
  original_filesize = buffer.st_size;

  if (output != NULL) {
    outfile_descriptor =
        open(output, O_WRONLY | O_CREAT, file_header->protection);
    if (outfile_descriptor < 0) {
      perror("Could not allocate outfile");
      return EXIT_FAILURE;
    }
  }

  write_header(outfile_descriptor, file_header);

  TrieNode *root = trie_create();
  TrieNode *curr_node = root;
  TrieNode *prev_node = NULL;

  uint8_t curr_sym = 0;
  uint8_t prev_sym = 0;
  uint16_t bit_length = 0;

  uint16_t next_code = START_CODE;

  while (read_sym(infile_descriptor, &curr_sym)) {
    TrieNode *next_node = trie_step(curr_node, curr_sym);
    if (next_node != NULL) {
      prev_node = curr_node;
      curr_node = next_node;
    } else {
      if (next_code == 0) {
        bit_length = 1;
      } else {
        bit_length = log2(next_code) + 1;
      }

      if (curr_node != NULL) {
        buffer_pair(outfile_descriptor, curr_node->code, curr_sym, bit_length);
        curr_node->children[curr_sym] = trie_node_create(next_code);
      }
      curr_node = root;
      next_code++;
    }

    if (next_code == MAX_CODE) {
      trie_reset(root);
      curr_node = root;
      next_code = START_CODE;
    }
    prev_sym = curr_sym;
  }
  if (curr_node != root) {
    if (next_code == 0) {
      bit_length = 1;
    } else {
      bit_length = log2(next_code) + 1;
    }
    buffer_pair(outfile_descriptor, prev_node->code, prev_sym, bit_length);
    next_code = (next_code + 1) % MAX_CODE;
  }

  if (next_code == 0) {
    bit_length = 1;
  } else {
    bit_length = log2(next_code) + 1;
  }
  buffer_pair(outfile_descriptor, STOP_CODE, 0, bit_length);
  flush_pairs(outfile_descriptor);

  if (statistics && output != NULL) {
    printf("Compressed file size: %lu bytes\n", concluding_filesize);
    printf("Uncompressed file size: %lu bytes\n", original_filesize);
    if (original_filesize > 0) {
      printf("Compression ration: %.2f%c\n",
             100 * (1 - (float)concluding_filesize / (float)original_filesize),
             PERCENT);
    } else {
      printf("Compression ratio: 0%c\n", PERCENT);
    }
  }

  trie_delete(root);
  free(file_header);

  if (input != NULL) {
    close(infile_descriptor);
  }
  if (output != NULL) {
    close(outfile_descriptor);
  }

  return EXIT_SUCCESS;
}
