#include <stdlib.h>
#include "shell.h"
#include "fat32.h"
#include "lexer.h"
#include "commands.h"
#include <stdio.h>
#include <string.h>

// Run the interactive shell loop
void shell_run(void) {
    while (1) {
        // Display prompt
        printf("%s", shell_get_prompt());
        fflush(stdout);
        
        // Get user input
        char *input = get_input();
        
        // Tokenize input
        tokenlist *tokens = get_tokens(input);
        
        // Dispatch command if tokens exist
        if (tokens->size > 0) {
            command_dispatch(tokens);
        }
        
        // Clean up
        free(input);
        free_tokens(tokens);
    }
}

// Get the current shell prompt string based on mounted image and current path
const char* shell_get_prompt(void) {
    static char prompt[600];
    
    if (ctx != NULL && ctx->fp != NULL) {
        if (ctx->current_path[0] != '\0') {
            // We're in a subdirectory: "fat32.img/RED/>"
            snprintf(prompt, sizeof(prompt), "%s%s/>", ctx->image_name, ctx->current_path);
        } else {
            // We're in root: "fat32.img/>"
            snprintf(prompt, sizeof(prompt), "%s/>", ctx->image_name);
        }
    } else {
        snprintf(prompt, sizeof(prompt), "/>");
    }
    
    return prompt;
}
