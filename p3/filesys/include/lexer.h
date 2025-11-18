#pragma once

#include <stddef.h>

// Token list structure for storing parsed command tokens
typedef struct {
    char **items;    // Array of token strings
    size_t size;     // Number of tokens
} tokenlist;

// Get input from stdin
char *get_input(void);

// Create a new empty tokenlist
tokenlist *new_tokenlist(void);

// Add a token to the tokenlist
void add_token(tokenlist *tokens, char *item);

// Parse input string into tokens (split by spaces)
tokenlist *get_tokens(char *input);

// Free memory allocated for tokenlist
void free_tokens(tokenlist *tokens);
