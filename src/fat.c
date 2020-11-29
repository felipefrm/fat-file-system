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
  fat_fs *fs = malloc(sizeof(fat_fs));
  if(fs == NULL){
    return NULL;
  }

  fs->fat_part = fopen("fat.part", "wb+");

  if (fs->fat_part == NULL)
  {
    fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n");
    fat_fs_free(fs);
    return NULL;
  }

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
  fat_fs *fs = malloc(sizeof(fat_fs));
  
  fs->fat_part = fopen("fat.part", "rb+");
  if (fs->fat_part == NULL)
  {
    fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n");
    fat_fs_free(fs);
    return 0;
  }

  uint8_t dummy[1024];
  fread(dummy, 1, 1024, fs->fat_part);

  fread(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);

  fread(fs->root_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
  return fs;
}

void fat_fs_mkdir(fat_fs *fs, char *dir)
{
  
  int n = strlen(dir), i, j, empty_entry;
  dir_entry_t current_dir[ENTRY_SIZE];
  memcpy(current_dir, fs->root_dir, sizeof(current_dir));
  int dir_block = 9;
  int num_tokens = 0;
  char *dir_copy = malloc((n + 1)*sizeof(char));
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
          strcmp((char*)current_dir[j].filename, token) == 0)
      {
        break;
      }
    }

    if (j == 32)
    {
      fprintf(stderr, "Não foi possível criar o diretorio especificado.\n");
      return;
    }
    dir_block = current_dir[j].first_block;
    fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
    fread(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
    

    //current_dir = dir_block;
    token = strtok(NULL, "/");
  }
  //printf("%d\n", dir_block);
  for (j = 0; j < ENTRY_SIZE; j++)
  {
    if (current_dir[j].first_block != 0 &&
        current_dir[j].attributes == 1 &&
        strcmp((char*)current_dir[j].filename, token) == 0)
    {
      fprintf(stderr, "Não foi possível criar o diretorio especificado, ele já existe.\n");
      return;
    }
  }
  //printf("EWQEsssss\n");
  for (empty_entry = 0; empty_entry < ENTRY_SIZE; empty_entry++)
  {
    if (current_dir[empty_entry].first_block == 0)
    {
      break;
    }
  }

  if (empty_entry == 32)
  {
    fprintf(stderr, "Não foi possível criar o diretorio especificado.\n");
    return;
  }
// printf("EWQEsssss\n");
  strcpy((char*)current_dir[empty_entry].filename, token);
  // printf("EWQEsssss\n");
  current_dir[empty_entry].attributes = 1;

  int first_block;
  for (first_block = 10; first_block < 4096; first_block++)
  {
    if (fs->fat[first_block] == 0x0000)
      break;
  }

  fs->fat[first_block] = 0xffff;
// printf("EWQEWQ\n");
  /* printf("%d\n",first_block); */
  current_dir[empty_entry].first_block = first_block;
// printf("EWQEWQ\n");
  dir_entry_t new_dir[ENTRY_SIZE];
  memset(new_dir, 0, ENTRY_SIZE * sizeof(dir_entry_t));
// printf("EWQEWQ\n");
  fseek(fs->fat_part, CLUSTER_SIZE, SEEK_SET);
  fwrite(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);

  fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
  fwrite(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
  if(dir_block == 9){  
    memcpy(fs->root_dir,current_dir, sizeof(current_dir));
  }

  fseek(fs->fat_part, first_block * CLUSTER_SIZE, SEEK_SET);
  fwrite(new_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
}

void fat_fs_ls(fat_fs *fs, char *dir)
{
  int n = strlen(dir), i, j;
  dir_entry_t current_dir[ENTRY_SIZE];
  memcpy(current_dir, fs->root_dir, sizeof(current_dir));
  int dir_block = 9;
  int num_tokens = 0;
  char *dir_copy = malloc(sizeof(char) * (n + 1));
  strcpy(dir_copy, dir);
  char *token = strtok(dir_copy, "/");
  //printf("USANDO LS \"%s\"\n", dir);
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
          strcmp((char*)current_dir[j].filename, token) == 0)
      {
        break;
      }
    }

    if (j == 32)
    {
      fprintf(stderr, "Não foi possível criar o diretorio especificado.\n");
      return;
    }
    dir_block = current_dir[j].first_block;
    fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
    fread(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
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

void fat_fs_create(fat_fs *fs, char *name)
{
 
  int n = strlen(name), i, j, empty_entry;
  dir_entry_t current_dir[ENTRY_SIZE];
  memcpy(current_dir, fs->root_dir, sizeof(current_dir));
  int dir_block = 9;
  int num_tokens = 0;
  char *name_copy = malloc((n + 1)*sizeof(char));
  strcpy(name_copy, name);
  char *token = strtok(name_copy, "/");
  while (token != NULL)
  {
    num_tokens++;
    token = strtok(NULL, "/");
  }
  free(name_copy);
  name_copy = NULL;

  token = strtok(name, "/");
  for (i = 0; i < num_tokens - 1; i++)
  {
    for (j = 0; j < ENTRY_SIZE; j++)
    {
      if (current_dir[j].first_block != 0 &&
          current_dir[j].attributes == 1 &&
          strcmp((char*)current_dir[j].filename, token) == 0)
      {
        break;
      }
    }

    if (j == 32)
    {
      fprintf(stderr, "Não foi possível criar o arquivo especificado.\n");
      return;
    }
    dir_block = current_dir[j].first_block;
    fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
    fread(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
    token = strtok(NULL, "/");
  }

  for (j = 0; j < ENTRY_SIZE; j++)
  {
    if (current_dir[j].first_block != 0 &&
        current_dir[j].attributes == 0 &&
        strcmp((char*)current_dir[j].filename, token) == 0)
    {
      fprintf(stderr, "Não foi possível criar o arquivo especificado, ele já existe.\n");
      return;
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
    fprintf(stderr, "Não foi possível criar o arquivo especificado.\n");
    return;
  }
  
  int first_block;
  for (first_block = 10; first_block < 4096; first_block++)
  {
    if (fs->fat[first_block] == 0x0000)
      break;
  }

  fs->fat[first_block] = 0xffff;

  strcpy((char*)current_dir[empty_entry].filename, token);
  current_dir[empty_entry].attributes = 0;
  current_dir[empty_entry].size = 0;
  current_dir[empty_entry].first_block = first_block;

  dir_entry_t new_dir[ENTRY_SIZE];
  memset(new_dir, 0, ENTRY_SIZE * sizeof(dir_entry_t));

  fseek(fs->fat_part, CLUSTER_SIZE, SEEK_SET);
  fwrite(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);

  fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
  fwrite(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
  if(dir_block == 9){  
    memcpy(fs->root_dir,current_dir, sizeof(current_dir));
  }

  // fseek(fs->fat_part, first_block * CLUSTER_SIZE, SEEK_SET);
  // fwrite(new_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
}
