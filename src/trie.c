#include "trie.h"
#include "code.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define ALPHABET 256

TrieNode *trie_node_create(uint16_t code) {
  TrieNode *t = (TrieNode *)malloc(sizeof(TrieNode));
  if (t) {
    t->code = code;
    for (uint32_t i = 0; i < ALPHABET; i++) {
      t->children[i] = NULL;
    }
    return t;
  }
  return NULL;
}

void trie_node_delete(TrieNode *n) {
  if (n != NULL) {
    free(n);
    n = NULL;
  }
}

TrieNode *trie_create(void) {
  TrieNode *t = trie_node_create(EMPTY_CODE);
  return t;
}

void trie_reset(TrieNode *root) {
  if (root != NULL) {
    for (int32_t i = 0; i < ALPHABET; i++) {
      if (root->children[i] != NULL) {
        trie_delete(root->children[i]);
        root->children[i] = NULL;
      }
    }
  }
}

void trie_delete(TrieNode *n) {
  if (n != NULL) {
    for (int32_t i = 0; i < ALPHABET; i++) {
      if (n->children[i] != NULL) {
        trie_delete(n->children[i]);
      }
    }
    trie_node_delete(n);
  }
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
  if (n != NULL) {
    return n->children[sym];
  }
  return NULL;
}

void trie_print(TrieNode *n, uint8_t val) {
  if (n != NULL) {
    printf("%c", val);
    for (int32_t i = 0; i < ALPHABET; i++) {
      if (n->children[i] != NULL) {
        trie_print(n->children[i], i);
      }
    }
  }
}
