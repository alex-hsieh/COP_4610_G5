#include "fat32.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Validate command-line arguments
    if (argc != 2) {
        fprintf(stderr, "Error: Invalid number of arguments\n");
        fprintf(stderr, "Usage: %s <fat32_image>\n", argv[0]);
        return 1;
    }
    
    // Mount the FAT32 image
    if (!fat32_mount(argv[1])) {
        // Error message already printed by fat32_mount
        return 1;
    }
    
    // Run the interactive shell
    shell_run();
    
    // This point should not be reached (exit command terminates program)
    fat32_unmount();
    return 0;
}
