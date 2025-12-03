#include "commands.h"
#include "fat32.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

// Dispatch command based on first token
void command_dispatch(tokenlist *tokens) {
    if (tokens->size == 0) {
        return;
    }
    
    char *cmd = tokens->items[0];
    
    if (strcmp(cmd, "info") == 0) {
        cmd_info(tokens);
    } else if (strcmp(cmd, "exit") == 0) {
        cmd_exit(tokens);
    } else if (strcmp(cmd, "cd") == 0) {
        cmd_cd(tokens);
    } else if (strcmp(cmd, "ls") == 0) {
        cmd_ls(tokens);
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", cmd);
    }
}

// Display FAT32 file system information
void cmd_info(tokenlist *tokens) {
    // Validate argument count
    if (tokens->size != 1) {
        fprintf(stderr, "Error: info takes no arguments\n");
        return;
    }
    
    // Validate that image is mounted
    if (ctx == NULL || ctx->fp == NULL) {
        fprintf(stderr, "Error: No image mounted\n");
        return;
    }
    
    // Calculate FAT entries count
    // Each FAT entry is 4 bytes, total entries = (FATSz32 * BytsPerSec) / 4
    uint32_t fat_entries = (ctx->bpb.BPB_FATSz32 * ctx->bpb.BPB_BytsPerSec) / 4;
    
    // Calculate image size in bytes
    uint32_t total_sectors = (ctx->bpb.BPB_TotSec16 != 0) ? 
                             ctx->bpb.BPB_TotSec16 : ctx->bpb.BPB_TotSec32;
    uint64_t image_size = (uint64_t)total_sectors * ctx->bpb.BPB_BytsPerSec;
    
    // Print file system information
    printf("Root cluster position: %u\n", ctx->bpb.BPB_RootClus);
    printf("Bytes per sector: %u\n", ctx->bpb.BPB_BytsPerSec);
    printf("Sectors per cluster: %u\n", ctx->bpb.BPB_SecPerClus);
    printf("Total clusters: %u\n", ctx->total_clusters);
    printf("FAT entries count: %u\n", fat_entries);
    printf("Image size (bytes): %lu\n", image_size);
}

// Exit the shell cleanly
void cmd_exit(tokenlist *tokens) {
    // Validate argument count
    if (tokens->size != 1) {
        fprintf(stderr, "Error: exit takes no arguments\n");
        return;
    }
    
    // Unmount and free resources
    fat32_unmount();
    
    // Exit successfully
    exit(0);
}

// Change directory command
void cmd_cd(tokenlist *tokens) {
    // Validate argument count
    if (tokens->size != 2) {
        fprintf(stderr, "Error: cd requires exactly one argument\n");
        return;
    }
    
    // Validate that image is mounted
    if (ctx == NULL || ctx->fp == NULL) {
        fprintf(stderr, "Error: No image mounted\n");
        return;
    }
    
    char *dirname = tokens->items[1];
    
    // Handle "." (current directory) - do nothing
    if (strcmp(dirname, ".") == 0) {
        return;
    }
    
    // Handle ".." (parent directory)
    if (strcmp(dirname, "..") == 0) {
        // Read current directory to find ".." entry
        size_t num_entries = 0;
        DirEntry *entries = fat32_read_directory(ctx->current_cluster, &num_entries);
        
        if (entries == NULL) {
            fprintf(stderr, "Error: Failed to read directory\n");
            return;
        }
        
        // Find the ".." entry
        uint32_t parent_cluster = 0;
        bool found = false;
        for (size_t i = 0; i < num_entries; i++) {
            if (entries[i].DIR_Name[0] == '.' && entries[i].DIR_Name[1] == '.') {
                parent_cluster = fat32_get_first_cluster(&entries[i]);
                found = true;
                break;
            }
        }
        
        free(entries);
        
        if (!found) {
            // We're in root directory (no ".." entry)
            // Already at root, so do nothing
            return;
        }
        
        // If parent cluster is 0, it means we're going to root
        if (parent_cluster == 0) {
            parent_cluster = ctx->bpb.BPB_RootClus;
        }
        
        // Update current cluster
        ctx->current_cluster = parent_cluster;
        
        // Update path - remove last directory component
        char *last_slash = strrchr(ctx->current_path, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
        } else {
            ctx->current_path[0] = '\0';  // Back to root
        }
        
        return;
    }
    
    // Regular directory change - search for dirname in current directory
    size_t num_entries = 0;
    DirEntry *entries = fat32_read_directory(ctx->current_cluster, &num_entries);
    
    if (entries == NULL) {
        fprintf(stderr, "Error: Failed to read directory\n");
        return;
    }
    
    // Search for the directory entry
    bool found = false;
    uint32_t target_cluster = 0;
    
    for (size_t i = 0; i < num_entries; i++) {
        char entry_name[13];
        fat32_parse_dir_name(entries[i].DIR_Name, entry_name, sizeof(entry_name));
        
        // Case-insensitive comparison (FAT32 is case-insensitive)
        if (strcasecmp(entry_name, dirname) == 0) {
            // Check if it's a directory
            if (!(entries[i].DIR_Attr & ATTR_DIRECTORY)) {
                fprintf(stderr, "Error: '%s' is not a directory\n", dirname);
                free(entries);
                return;
            }
            
            target_cluster = fat32_get_first_cluster(&entries[i]);
            found = true;
            break;
        }
    }
    
    free(entries);
    
    if (!found) {
        fprintf(stderr, "Error: Directory '%s' not found\n", dirname);
        return;
    }
    
    // Update current cluster
    ctx->current_cluster = target_cluster;
    
    // Update path
    if (ctx->current_path[0] != '\0') {
        strncat(ctx->current_path, "/", sizeof(ctx->current_path) - strlen(ctx->current_path) - 1);
    }
    strncat(ctx->current_path, dirname, sizeof(ctx->current_path) - strlen(ctx->current_path) - 1);
}

// List directory contents command
void cmd_ls(tokenlist *tokens) {
    // Validate argument count
    if (tokens->size != 1) {
        fprintf(stderr, "Error: ls takes no arguments\n");
        return;
    }
    
    // Validate that image is mounted
    if (ctx == NULL || ctx->fp == NULL) {
        fprintf(stderr, "Error: No image mounted\n");
        return;
    }
    
    // Read directory entries
    size_t num_entries = 0;
    DirEntry *entries = fat32_read_directory(ctx->current_cluster, &num_entries);
    
    if (entries == NULL) {
        fprintf(stderr, "Error: Failed to read directory\n");
        return;
    }
    
    // Check if we're in root directory (no "." or ".." entries)
    bool is_root = true;
    for (size_t i = 0; i < num_entries; i++) {
        if (entries[i].DIR_Name[0] == '.' && entries[i].DIR_Name[1] == ' ') {
            is_root = false;
            break;
        }
    }
    
    // Print all entries
    for (size_t i = 0; i < num_entries; i++) {
        char entry_name[13];
        fat32_parse_dir_name(entries[i].DIR_Name, entry_name, sizeof(entry_name));
        
        // Skip "." and ".." in root directory
        if (is_root && entry_name[0] == '.') {
            continue;
        }
        
        printf("%s\n", entry_name);
    }
    
    free(entries);
}
