#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// FAT32 Boot Parameter Block structure (512 bytes)
typedef struct __attribute__((packed)) {
    uint8_t  BS_jmpBoot[3];        // Offset 0: Jump instruction
    uint8_t  BS_OEMName[8];        // Offset 3: OEM name
    uint16_t BPB_BytsPerSec;       // Offset 11: Bytes per sector
    uint8_t  BPB_SecPerClus;       // Offset 13: Sectors per cluster
    uint16_t BPB_RsvdSecCnt;       // Offset 14: Reserved sector count
    uint8_t  BPB_NumFATs;          // Offset 16: Number of FATs
    uint16_t BPB_RootEntCnt;       // Offset 17: Root entry count (0 for FAT32)
    uint16_t BPB_TotSec16;         // Offset 19: Total sectors (if < 65536)
    uint8_t  BPB_Media;            // Offset 21: Media descriptor
    uint16_t BPB_FATSz16;          // Offset 22: FAT size (0 for FAT32)
    uint16_t BPB_SecPerTrk;        // Offset 24: Sectors per track
    uint16_t BPB_NumHeads;         // Offset 26: Number of heads
    uint32_t BPB_HiddSec;          // Offset 28: Hidden sectors
    uint32_t BPB_TotSec32;         // Offset 32: Total sectors (if >= 65536)
    uint32_t BPB_FATSz32;          // Offset 36: FAT size for FAT32
    uint16_t BPB_ExtFlags;         // Offset 40: Extended flags
    uint16_t BPB_FSVer;            // Offset 42: File system version
    uint32_t BPB_RootClus;         // Offset 44: Root directory cluster
    uint16_t BPB_FSInfo;           // Offset 48: FSInfo sector
    uint16_t BPB_BkBootSec;        // Offset 50: Backup boot sector
    uint8_t  BPB_Reserved[12];     // Offset 52: Reserved
    uint8_t  BS_DrvNum;            // Offset 64: Drive number
    uint8_t  BS_Reserved1;         // Offset 65: Reserved
    uint8_t  BS_BootSig;           // Offset 66: Boot signature (0x29)
    uint32_t BS_VolID;             // Offset 67: Volume ID
    uint8_t  BS_VolLab[11];        // Offset 71: Volume label
    uint8_t  BS_FilSysType[8];     // Offset 82: File system type
    uint8_t  Reserved[420];        // Offset 90: Reserved
    uint16_t Signature;            // Offset 510: Boot signature (0xAA55)
} BPB;

typedef struct __attribute__((packed)) {
    uint8_t  DIR_Name[11];      // Offset 0: Name (8) + Extension (3)
    uint8_t  DIR_Attr;          // Offset 11: Attributes
    uint8_t  DIR_NTRes;         // Offset 12: Reserved
    uint8_t  DIR_CrtTimeTenth;  // Offset 13: Creation time
    uint16_t DIR_CrtTime;       // Offset 14: Creation time
    uint16_t DIR_CrtDate;       // Offset 16: Creation date
    uint16_t DIR_LstAccDate;    // Offset 18: Last access date
    uint16_t DIR_FstClusHI;     // Offset 20: High word of cluster
    uint16_t DIR_WrtTime;       // Offset 22: Write time
    uint16_t DIR_WrtDate;       // Offset 24: Write date
    uint16_t DIR_FstClusLO;     // Offset 26: Low word of cluster
    uint32_t DIR_FileSize;      // Offset 28: File size in bytes
} DirEntry;

// directory attribute flags
#define ATTR_READ_ONLY	0x01
#define ATTR_HIDDEN 	0X02
#define ATTR_SYSTEM	0X04
#define ATTR_VOLUME_ID	0X08
#define ATTR_DIRECTORY	0x10
#define ATTR_ARCHIVE	0x20
#define ATTR_LONG_NAME	(ATTR_READ_ONLY) | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)


// FAT32 context structure holding all mounted image state
typedef struct {
    FILE *fp;                      // File pointer to the image
    BPB bpb;                       // Boot parameter block
    char image_name[256];          // Image filename for prompt
    uint32_t first_fat_sector;     // First FAT sector
    uint32_t first_data_sector;    // First data sector
    uint32_t total_clusters;       // Total number of clusters
    uint32_t bytes_per_cluster;    // Bytes per cluster

    uint32_t current_cluster;	  // Current directory's cluster number
    char current_path[256];       // Current path for display

} FAT32_Context;

// Global context - accessible to all modules
extern FAT32_Context *ctx;

// FAT32 operations
bool fat32_mount(const char *image_path);
void fat32_unmount(void);
uint32_t fat32_get_first_fat_sector(void);
uint32_t fat32_get_first_data_sector(void);
uint32_t fat32_get_total_clusters(void);

// helper functions for phase 2
uint32_t fat32_cluster_to_sector(uint32_t cluster);
uint32_t fat32_get_next_cluster(uint32_t cluster);
bool fat32_is_end_of_chain(uint32_t cluster_value);
DirEntry* fat32_read_directry(uint32_t cluster, size_t *num_entries);
void fat32_t_parse_dir_name(const uint8_t *dir_name, char *output, size_t output_size);
uint32_t fat32_get_first_cluster(const DirEntry *entry);

