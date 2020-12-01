#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include <stdio.h>

#define CLUSTER_SIZE 1024
#define FAT_ENTRIES 4096
#define DATA_CLUSTERS_SIZE 4086
#define ENTRY_SIZE 32
#define BOOT_BLOCK_CLUSTER 0
#define FIRST_FAT_CLUSTER 1
#define ROOT_DIR_CLUSTER 9
#define FIRST_DATA_CLUSTER 10

/* entrada de diretorio, 32 bytes cada */
typedef struct {
    uint8_t filename[18];
    uint8_t attributes;
    uint8_t reserved[7];
    uint16_t first_block;
    uint32_t size;
} dir_entry_t;

typedef struct {
    uint16_t fat[FAT_ENTRIES];
    dir_entry_t root_dir[ENTRY_SIZE];
    FILE *fat_part;
} fat_fs;


// FILE* openFAT();
fat_fs* fat_fs_init();
fat_fs* fat_fs_load();
void fat_fs_free(fat_fs* fs);
void fat_fs_mkdir(fat_fs* fs, char *dir);
void fat_fs_ls(fat_fs* fs, char* dir);
void fat_fs_create(fat_fs* fs,char* name);
void fat_fs_unlink(fat_fs *fs, char *name);

void fat_fs_write(fat_fs *fs, char *string, char *name);
void fat_fs_read(fat_fs *fs, char *name);
void fat_fs_append(fat_fs *fs, char *string, char *name);
#endif
