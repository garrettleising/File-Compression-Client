#include "io.h"
#include "endian.h"
#include "word.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAGIC 0x8badbeef // Program’s magic number.
#define BUFFER_LENGTH 4096
#define INITIAL -1
#define STARTING_INDEX -1
#define STARTING_POINT 0
#define BYTE_LENGTH 8
#define MASK 1

uint64_t concluding_filesize = 0;

// encode
int32_t in_character_index = STARTING_INDEX;
int32_t in_character_buffer_length = INITIAL;
uint8_t in_character_buffer[BUFFER_LENGTH];

int32_t out_pair_byte_index = STARTING_POINT;
int32_t out_pair_bit_index = STARTING_POINT;
uint8_t out_pair_buffer[BUFFER_LENGTH];

// decode
int32_t out_character_index = STARTING_POINT;
int32_t out_character_buffer_length = BUFFER_LENGTH;
uint8_t out_character_buffer[BUFFER_LENGTH];

int32_t in_pair_byte_index = STARTING_POINT;
int32_t in_pair_bit_index = STARTING_POINT;
int32_t in_pair_buffer_length = INITIAL;
uint8_t in_pair_buffer[BUFFER_LENGTH];

// works fine
void read_header(int infile, FileHeader *header) {
  int32_t error = read(infile, header, sizeof(FileHeader));
  if (error < 0) {
    perror("Failed to write to the outfile in write_header");
  }
  if (is_big()) {
    header->magic = swap32(header->magic);
    header->protection = swap16(header->protection);
  }
}

// works fine
void write_header(int outfile, FileHeader *header) {
  if (is_big()) {
    header->magic = swap32(header->magic);
    header->protection = swap16(header->protection);
  }
  int32_t error = write(outfile, header, sizeof(FileHeader));
  if (error < 0) {
    perror("Failed to write to the outfile in write_header");
  }
}

// Works perfectly
bool read_sym(int infile, uint8_t *sym) {
  if (in_character_index == -1 ||
      in_character_index >= in_character_buffer_length - 1) {
    in_character_index = -1;
    in_character_buffer_length =
        read(infile, in_character_buffer, BUFFER_LENGTH);
    if (in_character_buffer_length <= 0) {
      return false;
    }
  }
  in_character_index++;
  *sym = in_character_buffer[in_character_index];
  return true;
}

// not working as intended
void buffer_pair(int outfile, uint16_t code, uint8_t sym, uint8_t bitlen) {
  uint8_t bitmask = MASK;
  uint16_t holder_code = 0;
  uint16_t other_holder_code = 0;
  int error;
  for (uint32_t i = 0; i < bitlen; i++) {
    if (out_pair_bit_index == 0) {
      out_pair_buffer[out_pair_byte_index] = 0;
    }

    other_holder_code = code >> i;
    holder_code = (bitmask & other_holder_code);
    holder_code = (holder_code << (out_pair_bit_index));
    out_pair_buffer[out_pair_byte_index] =
        out_pair_buffer[out_pair_byte_index] | holder_code;
    holder_code = 0;

    out_pair_bit_index++;

    if (out_pair_bit_index == BYTE_LENGTH) {
      out_pair_bit_index = 0;
      out_pair_byte_index++;
      concluding_filesize++;

      if (out_pair_byte_index >= BUFFER_LENGTH) {
        error = write(outfile, out_pair_buffer, BUFFER_LENGTH);

        out_pair_byte_index = STARTING_POINT;
        if (error == -1) {
          perror("An error occured");
          exit(0);
        }
      }
    }
  }

  uint8_t holder_sym = 0;
  uint16_t other_holder_sym = 0;
  for (uint32_t i = 0; i < BYTE_LENGTH; i++) {
    if (out_pair_bit_index == 0) {
      out_pair_buffer[out_pair_byte_index] = 0;
    }
    other_holder_sym = sym >> i;
    holder_sym = (bitmask & other_holder_sym);
    holder_sym = (holder_sym << (out_pair_bit_index));
    out_pair_buffer[out_pair_byte_index] =
        out_pair_buffer[out_pair_byte_index] | holder_sym;
    holder_sym = 0;

    out_pair_bit_index++;

    if (out_pair_bit_index == BYTE_LENGTH) {
      out_pair_bit_index = 0;
      out_pair_byte_index++;
      concluding_filesize++;

      if (out_pair_byte_index >= BUFFER_LENGTH) {
        error = write(outfile, out_pair_buffer, BUFFER_LENGTH);
        out_pair_byte_index = STARTING_POINT;
        if (error == -1) {
          perror("An error occured");
          exit(0);
        }
      }
    }
  }
}

// works fine
void flush_pairs(int outfile) {
  if (out_pair_byte_index < BUFFER_LENGTH) {
    int32_t error = write(outfile, out_pair_buffer, out_pair_byte_index);
    if (error == -1) {
      perror("An error occured");
      exit(0);
    }
  }
}

// works fine
bool read_pair(int infile, uint16_t *code, uint8_t *sym, uint8_t bitlen) {
  if (in_pair_buffer_length == -1) {
    in_pair_buffer_length = read(infile, in_pair_buffer, BUFFER_LENGTH);
  }

  *code = 0;
  uint8_t bitmask = MASK;
  uint16_t holder_code = 0;
  for (uint32_t i = 0; i < bitlen; i++) {
    holder_code =
        (bitmask & (in_pair_buffer[in_pair_byte_index] >> in_pair_bit_index));
    holder_code = (holder_code << i);
    *code = *code | holder_code;
    holder_code = 0;
    in_pair_bit_index++;

    if (in_pair_bit_index == BYTE_LENGTH) {
      in_pair_bit_index = 0;
      in_pair_byte_index++;
      concluding_filesize++;

      if (in_pair_byte_index >= BUFFER_LENGTH) {
        in_pair_buffer_length = read(infile, in_pair_buffer, BUFFER_LENGTH);
        in_pair_byte_index = STARTING_POINT;
      }
    }
  }

  if (*code == 0) {
    return false;
  }

  *sym = 0;
  uint8_t holder_sym = 0;
  for (uint32_t i = 0; i < BYTE_LENGTH; i++) {
    holder_sym =
        (bitmask & (in_pair_buffer[in_pair_byte_index] >> in_pair_bit_index));
    holder_sym = (holder_sym << i);
    *sym = *sym | holder_sym;
    holder_sym = 0;
    in_pair_bit_index++;

    if (in_pair_bit_index == BYTE_LENGTH) {
      in_pair_bit_index = 0;
      in_pair_byte_index++;
      concluding_filesize++;

      if (in_pair_byte_index >= BUFFER_LENGTH) {
        in_pair_buffer_length = read(infile, in_pair_buffer, BUFFER_LENGTH);
        in_pair_byte_index = STARTING_POINT;
      }
    }
  }

  return true;
}

// works fine
void buffer_word(int outfile, Word *w) {
  int error;
  if (w != NULL) {
    for (uint32_t i = 0; i < (w->len); i++) {
      if (out_character_index >= out_character_buffer_length) {
        error = write(outfile, out_character_buffer, BUFFER_LENGTH);
        if (error == -1) {
          perror("An error occured");
          exit(0);
        }
        out_character_index = 0;
      }

      out_character_buffer[out_character_index] = w->syms[i];
      out_character_index++;
    }
  }
}

// works fine
void flush_words(int outfile) {
  if (out_character_index < out_character_buffer_length) {
    int32_t error = write(outfile, out_character_buffer, out_character_index);
    if (error == -1) {
      perror("An error occured");
      exit(0);
    }
  }
}
