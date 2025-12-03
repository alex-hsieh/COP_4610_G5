#pragma once

#include "lexer.h"

// Command dispatcher - routes tokens to appropriate command handler
void command_dispatch(tokenlist *tokens);

// Command implementations
void cmd_info(tokenlist *tokens);
void cmd_exit(tokenlist *tokens);
void cmd_cd(tokenlist *tokens);
void cmd_ls(tokenlist *tokens);

