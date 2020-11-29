#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fat.h"

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
//static uint16_t fat[FAT_ENTRIES];
//static dir_entry_t root_dir[ENTRY_SIZE];
union
{
  dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
  uint8_t data[CLUSTER_SIZE];
} data_cluster;
/* diretorios (incluindo ROOT), 32 entradas de diretorio com 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
/* FILE *openFAT() { */
/*     FILE *fat_part = fopen("fat.part", "wb+"); */
/*     if (fat_part == NULL) { */
/*         fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n"); */
/*         return NULL; */
/*     } */
/*     return fat_part; */
/* } */
void fat_fs_free(fat_fs* fat_fs){

  if(fat_fs != NULL){
    if(fat_fs->fat_part!=NULL){
      fclose(fat_fs->fat_part);
    }
    free(fat_fs);
  } 
}
fat_fs *fat_fs_init()
{
  /* FILE *fat_part = *fat_part_; */
  /*   fat_part = fopen("fat.part", "wb+"); */
  fat_fs *fs = malloc(sizeof(fs));
  if(fs == NULL){
    return NULL;
  }
  //printf("%p\n",fat_fs);


    // FILE* fat_part = fopen("fat.part", "wb+");
    // uint16_t xxx = 0xbbbb;
    // printf("%p\n",fat_part);  
    // for (int i = 0; i < 512; i++)
    //     fwrite(&xxx, sizeof(xxx), 1, fat_part);
    // fclose(fat_part);

  fs->fat_part = fopen("fat.part", "wb+");

      
  //FILE *fat_part = fopen("fat.part","wb");
  if (fs->fat_part == NULL)
  {
    fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n");
    fat_fs_free(fs);
    return NULL;
  }

// for (int i = 0; i < 512; i++)
//         fwrite(&xxx, sizeof(xxx), 1, fs->fat_part);
  fs->fat[0] = 0xfffd;
  for (int i = 1; i < 9; i++)
  {
    fs->fat[i] = 0xfffe;
  }

  fs->fat[9] = 0xffff;
  for (int i = 10; i < FAT_ENTRIES; i++)
  {
    fs->fat[i] = 0x0000;
  }

  uint16_t boot = 0xbbbb;
  for (int i = 0; i < 512; i++)
    fwrite(&boot, sizeof(boot), 1, fs->fat_part);

  fwrite(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);

  memset(fs->root_dir, 0, ENTRY_SIZE * sizeof(dir_entry_t));
  fwrite(fs->root_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);

  uint8_t t[CLUSTER_SIZE];
  memset(t, 0, CLUSTER_SIZE);

  for (int i = 0; i < DATA_CLUSTERS_SIZE; i++)
    fwrite(t, 1, CLUSTER_SIZE, fs->fat_part);

  return fs;
}

fat_fs *fat_fs_load()
{
  fat_fs *fat_fs = malloc(sizeof(fat_fs));
  
  fat_fs->fat_part = fopen("fat.part", "rb+");
  if (fat_fs->fat_part == NULL)
  {
    fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n");
    fat_fs_free(fat_fs);
    return 0;
  }

  uint8_t dummy[1024];
  fread(dummy, 1, 1024, fat_fs->fat_part);

  fread(fat_fs->fat, sizeof(uint16_t), FAT_ENTRIES, fat_fs->fat_part);

  fread(fat_fs->root_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_fs->fat_part);

  return 1;
}

void fat_fs_mkdir(fat_fs *fat_fs, char *dir)
{
  int n = strlen(dir), i, j, empty_entry;
  dir_entry_t current_dir[ENTRY_SIZE];
  //printf("%d\n",sizeof(current_dir));
  //printf("%p %p\n",current_dir,root_dir);
  memcpy(current_dir, fat_fs->root_dir, sizeof(current_dir));
  int dir_block = 9;
  int num_tokens = 0;
  char *dir_copy = malloc(sizeof(char) * (n + 1));
  strcpy(dir_copy, dir);
  char *token = strtok(dir_copy, "/");
  while (token != NULL)
  {
    num_tokens++;
    token = strtok(NULL, "/");
  }
  free(dir_copy);
  dir_copy = NULL;

  token = strtok(dir, "/");
  for (i = 0; i < num_tokens - 1; i++)
  {
    for (j = 0; j < ENTRY_SIZE; j++)
    {
      if (current_dir[j].first_block != 0 &&
          current_dir[j].attributes == 1 &&
          strcmp(current_dir[j].filename, token) == 0)
      {
        break;
      }
    }

    if (j == 32)
    {
      fprintf(stderr, "Não foi possível criar o diretorio especificado. Não existe entrada disponivel.\n");
      return 1;
    }
    dir_block = current_dir[j].first_block;
    fseek(fat_fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
    fread(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_fs->fat_part);
    //current_dir = dir_block;
    token = strtok(NULL, "/");
  }
  printf("%d\n", dir_block);
  for (empty_entry = 0; empty_entry < ENTRY_SIZE; empty_entry++)
  {
    if (current_dir[j].first_block != 0 &&
        current_dir[j].attributes == 1 &&
        strcmp(current_dir[j].filename, token) == 0)
    {
      fprintf(stderr, "Não foi possível criar o diretorio especificado, ele já existe.\n");
      return 1;
    }
  }
  for (empty_entry = 0; empty_entry < ENTRY_SIZE; empty_entry++)
  {
    if (current_dir[empty_entry].first_block == 0)
    {
      break;
    }
  }

  if (empty_entry == 32)
  {
    fprintf(stderr, "Não foi possível criar o diretorio especificado, máximo de arquivos atingido no diretorio.\n");
    return 1;
  }

  strcpy(current_dir[empty_entry].filename, token);
  current_dir[empty_entry].attributes = 1;

  int first_block;
  for (first_block = 10; first_block < 4096; first_block++)
  {
    if (fat_fs->fat[first_block] == 0x0000)
      break;
  }

  fat_fs->fat[first_block] = 0xffff;

  /* printf("%d\n",first_block); */
  current_dir[empty_entry].first_block = first_block;

  dir_entry_t new_dir[ENTRY_SIZE];
  memset(new_dir, 0, ENTRY_SIZE * sizeof(dir_entry_t));

  fseek(fat_fs->fat_part, CLUSTER_SIZE, SEEK_SET);
  fwrite(fat_fs->fat, sizeof(uint16_t), FAT_ENTRIES, fat_fs->fat_part);

  fseek(fat_fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
  fwrite(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_fs->fat_part);

  fseek(fat_fs->fat_part, first_block * CLUSTER_SIZE, SEEK_CUR);
  fwrite(new_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_fs->fat_part);
  return 0;
}

void fat_fs_ls(fat_fs *fat_fs, char *dir)
{
  int n = strlen(dir), i, j, empty_entry;
  dir_entry_t current_dir[ENTRY_SIZE];
  //memcpyroot_dir;
  memcpy(current_dir, fat_fs->root_dir, sizeof(current_dir));
  int dir_block = 9;
  int num_tokens = 0;
  char *dir_copy = malloc(sizeof(char) * (n + 1));
  strcpy(dir_copy, dir);
  char *token = strtok(dir_copy, "/");
  printf("USANDO LS \"%s\"\n", dir);
  while (token != NULL)
  {
    num_tokens++;
    token = strtok(NULL, "/");
  }
  free(dir_copy);
  dir_copy = NULL;

  token = strtok(dir, "/");
  for (i = 0; i < num_tokens; i++)
  {
    for (j = 0; j < ENTRY_SIZE; j++)
    {
      if (current_dir[j].first_block != 0 &&
          current_dir[j].attributes == 1 &&
          strcmp(current_dir[j].filename, token) == 0)
      {
        break;
      }
    }

    if (j == 32)
    {
      fprintf(stderr, "Não foi possível criar o diretorio especificado.\n");
      return 1;
    }
    dir_block = current_dir[j].first_block;
    fseek(fat_fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
    fread(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fat_fs->fat_part);
    //current_dir = dir_block;
    token = strtok(NULL, "/");
    printf("%d\n", dir_block);
  }

  for (i = 0; i < ENTRY_SIZE; i++)
  {
    if (current_dir[i].first_block != 0)
    {
      if (current_dir[i].attributes == 0)
      {
        printf("%s\n", current_dir[i].filename);
      }
      else if (current_dir[i].attributes == 1)
      {
        printf("%s/\n", current_dir[i].filename);
      }
    }
  }
}

void fat_fs_create(fat_fs *fat_fs, char *name)
{
}
