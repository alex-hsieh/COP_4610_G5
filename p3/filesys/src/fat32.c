#include "fat32.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    
    // Initialize current directory to root
    ctx->current_cluster = ctx->bpb.BPB_RootClus;
    ctx->current_path[0] = '\0';  // Root directory = empty path
    
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

// Convert cluster number to first sector number
// Formula: FirstSectorOfCluster = FirstDataSector + (ClusterNumber - 2) * SecPerClus
uint32_t fat32_cluster_to_sector(uint32_t cluster) {
    if (cluster < 2) {
        return 0;  // Invalid cluster
    }
    return ctx->first_data_sector + (cluster - 2) * ctx->bpb.BPB_SecPerClus;
}

// Get the next cluster in the FAT chain
uint32_t fat32_get_next_cluster(uint32_t cluster) {
    // Calculate FAT offset (each entry is 4 bytes)
    uint32_t fat_offset = cluster * 4;
    
    // Calculate FAT sector and offset within sector
    uint32_t fat_sector = ctx->first_fat_sector + (fat_offset / ctx->bpb.BPB_BytsPerSec);
    uint32_t entry_offset = fat_offset % ctx->bpb.BPB_BytsPerSec;
    
    // Read the FAT entry
    uint32_t next_cluster;
    fseek(ctx->fp, fat_sector * ctx->bpb.BPB_BytsPerSec + entry_offset, SEEK_SET);
    fread(&next_cluster, sizeof(uint32_t), 1, ctx->fp);
    
    // Mask off upper 4 bits (only lower 28 bits are used)
    return next_cluster & 0x0FFFFFFF;
}

// Check if cluster value indicates end of chain
bool fat32_is_end_of_chain(uint32_t cluster_value) {
    // EOC is >= 0x0FFFFFF8 (after masking upper 4 bits)
    return (cluster_value >= 0x0FFFFFF8);
}

// Parse directory name from 11-byte DIR_Name field to null-terminated string
// Handles both "NAME    EXT" and "NAME       " formats
void fat32_parse_dir_name(const uint8_t *dir_name, char *output, size_t output_size) {
    if (output_size < 12) return;  // Need at least 12 bytes for "NAME.EXT\0"
    
    char name[9] = {0};
    char ext[4] = {0};
    
    // Copy name (first 8 bytes), trimming trailing spaces
    int name_len = 0;
    for (int i = 0; i < 8 && dir_name[i] != ' '; i++) {
        name[name_len++] = dir_name[i];
    }
    name[name_len] = '\0';
    
    // Copy extension (last 3 bytes), trimming trailing spaces
    int ext_len = 0;
    for (int i = 8; i < 11 && dir_name[i] != ' '; i++) {
        ext[ext_len++] = dir_name[i];
    }
    ext[ext_len] = '\0';
    
    // Combine name and extension
    if (ext_len > 0) {
        snprintf(output, output_size, "%s.%s", name, ext);
    } else {
        snprintf(output, output_size, "%s", name);
    }
}

// Get the first cluster number from a directory entry
uint32_t fat32_get_first_cluster(const DirEntry *entry) {
    return ((uint32_t)entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
}

// Read all directory entries from a directory cluster chain
// Returns array of DirEntry and sets num_entries
// Caller must free the returned array
DirEntry* fat32_read_directory(uint32_t cluster, size_t *num_entries) {
    if (num_entries == NULL) return NULL;
    
    *num_entries = 0;
    
    // Allocate initial buffer for entries
    size_t capacity = 64;  // Start with space for 64 entries
    DirEntry *entries = (DirEntry *)malloc(capacity * sizeof(DirEntry));
    if (entries == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for directory entries\n");
        return NULL;
    }
    
    // Allocate buffer for reading one cluster at a time
    uint8_t *cluster_buffer = (uint8_t *)malloc(ctx->bytes_per_cluster);
    if (cluster_buffer == NULL) {
        fprintf(stderr, "Error: Failed to allocate cluster buffer\n");
        free(entries);
        return NULL;
    }
    
    // Traverse the cluster chain
    uint32_t current_cluster = cluster;
    while (!fat32_is_end_of_chain(current_cluster)) {
        // Read the entire cluster
        uint32_t sector = fat32_cluster_to_sector(current_cluster);
        fseek(ctx->fp, sector * ctx->bpb.BPB_BytsPerSec, SEEK_SET);
        size_t bytes_read = fread(cluster_buffer, 1, ctx->bytes_per_cluster, ctx->fp);
        
        if (bytes_read != ctx->bytes_per_cluster) {
            fprintf(stderr, "Error: Failed to read cluster %u\n", current_cluster);
            break;
        }
        
        // Parse directory entries (32 bytes each)
        size_t entries_per_cluster = ctx->bytes_per_cluster / sizeof(DirEntry);
        for (size_t i = 0; i < entries_per_cluster; i++) {
            DirEntry *entry = (DirEntry *)(cluster_buffer + i * sizeof(DirEntry));
            
            // Check for end of directory (0x00 in first byte)
            if (entry->DIR_Name[0] == 0x00) {
                goto done_reading;
            }
            
            // Skip deleted entries (0xE5 in first byte)
            if (entry->DIR_Name[0] == 0xE5) {
                continue;
            }
            
            // Skip long name entries (check ATTR_LONG_NAME)
            if ((entry->DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) {
                continue;
            }
            
            // Skip volume ID entries
            if (entry->DIR_Attr & ATTR_VOLUME_ID) {
                continue;
            }
            
            // Expand array if needed
            if (*num_entries >= capacity) {
                capacity *= 2;
                DirEntry *new_entries = (DirEntry *)realloc(entries, capacity * sizeof(DirEntry));
                if (new_entries == NULL) {
                    fprintf(stderr, "Error: Failed to expand directory entries array\n");
                    free(entries);
                    free(cluster_buffer);
                    return NULL;
                }
                entries = new_entries;
            }
            
            // Copy this entry
            memcpy(&entries[*num_entries], entry, sizeof(DirEntry));
            (*num_entries)++;
        }
        
        // Get next cluster in chain
        current_cluster = fat32_get_next_cluster(current_cluster);
    }
    
done_reading:
    free(cluster_buffer);
    
    // Shrink array to actual size
    if (*num_entries > 0) {
        DirEntry *final_entries = (DirEntry *)realloc(entries, (*num_entries) * sizeof(DirEntry));
        if (final_entries != NULL) {
            entries = final_entries;
        }
    }
    
    return entries;
}
