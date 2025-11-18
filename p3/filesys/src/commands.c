#include "commands.h"
#include "fat32.h"
#include <stdio.h>
#include <string.h>
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
