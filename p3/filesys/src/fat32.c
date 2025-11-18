#include "fat32.h"
#include <stdlib.h>
#include <string.h>

// Global context instance
FAT32_Context *ctx = NULL;

// Mount a FAT32 image file and parse its boot sector
bool fat32_mount(const char *image_path) {
    // Allocate context
    ctx = (FAT32_Context *)malloc(sizeof(FAT32_Context));
    if (ctx == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for FAT32 context\n");
        return false;
    }
    
    // Initialize context
    memset(ctx, 0, sizeof(FAT32_Context));
    
    // Open the image file in read/write mode
    ctx->fp = fopen(image_path, "r+");
    if (ctx->fp == NULL) {
        fprintf(stderr, "Error: Cannot open image file '%s'\n", image_path);
        free(ctx);
        ctx = NULL;
        return false;
    }
    
    // Extract filename from path for prompt (C11 only)
    const char *filename = image_path;
    const char *last_slash = strrchr(image_path, '/');
    if (last_slash != NULL) {
        filename = last_slash + 1;
    }
    strncpy(ctx->image_name, filename, sizeof(ctx->image_name) - 1);
    ctx->image_name[sizeof(ctx->image_name) - 1] = '\0';
    
    // Read the boot sector (512 bytes at offset 0)
    fseek(ctx->fp, 0, SEEK_SET);
    size_t bytes_read = fread(&ctx->bpb, sizeof(BPB), 1, ctx->fp);
    if (bytes_read != 1) {
        fprintf(stderr, "Error: Failed to read boot sector\n");
        fclose(ctx->fp);
        free(ctx);
        ctx = NULL;
        return false;
    }
    
    // Validate boot signature (0xAA55 at offset 510)
    if (ctx->bpb.Signature != 0xAA55) {
        fprintf(stderr, "Error: Invalid boot signature (expected 0xAA55, got 0x%04X)\n", 
                ctx->bpb.Signature);
        fclose(ctx->fp);
        free(ctx);
        ctx = NULL;
        return false;
    }
    
    // Calculate FAT32 layout parameters
    // First FAT sector = Reserved sectors
    ctx->first_fat_sector = ctx->bpb.BPB_RsvdSecCnt;
    
    // First data sector = Reserved + (NumFATs * FATSz32)
    ctx->first_data_sector = ctx->bpb.BPB_RsvdSecCnt + 
                             (ctx->bpb.BPB_NumFATs * ctx->bpb.BPB_FATSz32);
    
    // Total data sectors
    uint32_t total_sectors = (ctx->bpb.BPB_TotSec16 != 0) ? 
                             ctx->bpb.BPB_TotSec16 : ctx->bpb.BPB_TotSec32;
    uint32_t data_sectors = total_sectors - ctx->first_data_sector;
    
    // Total clusters in data region
    ctx->total_clusters = data_sectors / ctx->bpb.BPB_SecPerClus;
    
    // Bytes per cluster
    ctx->bytes_per_cluster = ctx->bpb.BPB_BytsPerSec * ctx->bpb.BPB_SecPerClus;
    
    return true;
}

// Unmount the FAT32 image and free resources
void fat32_unmount(void) {
    if (ctx != NULL) {
        if (ctx->fp != NULL) {
            fclose(ctx->fp);
        }
        free(ctx);
        ctx = NULL;
    }
}

// Get the first FAT sector
uint32_t fat32_get_first_fat_sector(void) {
    return ctx->first_fat_sector;
}

// Get the first data sector
uint32_t fat32_get_first_data_sector(void) {
    return ctx->first_data_sector;
}

// Get total number of clusters
uint32_t fat32_get_total_clusters(void) {
    return ctx->total_clusters;
}
