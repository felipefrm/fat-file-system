#include <stdio.h>
#include <string.h>
#include "fat.h"

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
static uint16_t fat[FAT_ENTRIES];
static dir_entry_t root_dir[ENTRY_SIZE];

/* diretorios (incluindo ROOT), 32 entradas de diretoriocom 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
union {
    dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
    uint8_t data[CLUSTER_SIZE];
} data_cluster;

int init() {
    FILE *fat_part = fopen("fat.part", "wb");
    
    if (fat_part == NULL) {
        fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n");
        return 0;
    }

    fat[0] = 0xfffd;
    for (int i = 1; i < 9; i++) {
        fat[i] = 0xfffe;
    }

    fat[9] = 0xffff;
    for (int i = 10; i < FAT_ENTRIES; i++) {
        fat[i] = 0x0000;
    }

    uint16_t boot = 0xbbbb;
    for (int i = 0; i < 512; i++)
        fwrite(&boot, sizeof(boot), 1, fat_part);

    fwrite(fat, sizeof(uint16_t), FAT_ENTRIES, fat_part);

    memset(root_dir, 0, ENTRY_SIZE*sizeof(dir_entry_t));
    fwrite(root_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_part);

    uint8_t t[CLUSTER_SIZE];
    memset(t, 0, CLUSTER_SIZE);
    
    for (int i = 0; i < DATA_CLUSTERS_SIZE; i++) 
        fwrite(t, 1, CLUSTER_SIZE, fat_part);


    fclose(fat_part);
    return 1;
}

int load() {
    FILE *fat_part = fopen("fat.part", "rb+");
    
    if (fat_part == NULL) {
        fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n");
        return 0;
    }

    uint8_t dummy[1024];
    fread(dummy, 1, 1024, fat_part);

    fread(fat, sizeof(uint16_t), FAT_ENTRIES, fat_part);

    fread(root_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_part);

    fclose(fat_part);
    return 1;
}

int mkdir(char *dir) {

    int empty_entry = 0;
    for (int empty_entry = 0; empty_entry < ENTRY_SIZE; empty_entry++) {
        if (root_dir[empty_entry].first_block == 0) {
            break;
        }
    }

    if (empty_entry == 32) { 
        // nao tem nada vazio aqui
    }
    
    strcpy(root_dir[empty_entry].filename, dir);
    root_dir[empty_entry].attributes = 1;

    int first_block;
    for (first_block = 10; first_block < 4096; first_block++) {
        if(fat[first_block] == 0x0000) break;
    }

    fat[first_block] = 0xffff;

    root_dir[empty_entry].first_block = first_block;

    dir_entry_t new_dir[ENTRY_SIZE];
    memset(new_dir, 0, ENTRY_SIZE*sizeof(dir_entry_t));

    FILE *fat_part = fopen("fat.part", "rb+");
    
    if (fat_part == NULL) {
        fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n");
        return 0;
    }

    fseek(fat_part, CLUSTER_SIZE, SEEK_SET);
    fwrite(fat, sizeof(uint16_t), FAT_ENTRIES, fat_part);

    fwrite(root_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_part);

    fseek(fat_part, (10-first_block)*CLUSTER_SIZE, SEEK_CUR);
    fwrite(new_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_part);

}