#include "word.h"
#include "code.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

Word *word_create(uint8_t *syms, uint32_t len) {
  Word *w = (Word *)malloc(sizeof(Word));
  if (w) {
    w->len = len;
    w->syms = (uint8_t *)calloc(len, sizeof(uint8_t));
    if (w->syms != NULL) {
      for (uint32_t i = 0; i < len; i++) {
        w->syms[i] = syms[i];
      }
    }
    return w;
  }
  return NULL;
}

Word *word_append_sym(Word *w, uint8_t sym) {
  if (w != NULL) {
    Word *word = (Word *)malloc(sizeof(Word));
    if (word) {
      word->len = w->len + 1;
      word->syms = (uint8_t *)calloc(word->len, sizeof(uint8_t));
      if (word->syms != NULL) {
        for (uint32_t i = 0; i < w->len; i++) {
          word->syms[i] = w->syms[i];
        }
        word->syms[word->len - 1] = sym;
      }
      return word;
    }
  }
  return NULL;
}

void word_delete(Word *w) {
  if (w != NULL) {
    if (w->syms != NULL) {
      free(w->syms);
      w->syms = NULL;
    }
    free(w);
  }
  w = NULL;
}

WordTable *wt_create(void) {
  WordTable *wt = (WordTable *)calloc(MAX_CODE, sizeof(WordTable));
  if (wt) {
    Word *empty_word = word_create(NULL, 0);
    wt[EMPTY_CODE] = empty_word;
    return wt;
  }
  return NULL;
}

void wt_reset(WordTable *wt) {
  if (wt) {
    for (int i = (EMPTY_CODE + 1); i < MAX_CODE; i++) {
      if (wt[i] != NULL) {
        word_delete(wt[i]);
        wt[i] = NULL;
      }
    }
  }
}

void wt_delete(WordTable *wt) {
  if (wt) {
    for (int i = 0; i < MAX_CODE; i++) {
      if (wt[i] != NULL) {
        word_delete(wt[i]);
        wt[i] = NULL;
      };
    }
    free(wt);
  }
}
