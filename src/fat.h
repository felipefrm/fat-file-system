#ifndef FAT
#define FAT

#include <stdint.h>

#define CLUSTER_SIZE 1024
#define FAT_ENTRIES 4096
#define DATA_CLUSTERS_SIZE 4086
#define ENTRY_SIZE 32

/* entrada de diretorio, 32 bytes cada */
typedef struct {
    uint8_t filename[18];
    uint8_t attributes;
    uint8_t reserved[7];
    uint16_t first_block;
    uint32_t size;
} dir_entry_t;

int init();
int load();
int mkdir(char *dir);

#endif