#include "fat.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
// static uint16_t fat[FAT_ENTRIES];
// static dir_entry_t root_dir[ENTRY_SIZE];
union {
  dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
  uint8_t data[CLUSTER_SIZE];
} data_cluster;
/* diretorios (incluindo ROOT), 32 entradas de diretorio com 32 bytes cada =
 * 1024 bytes ou bloco de dados de 1024 bytes*/

void fat_fs_free(fat_fs *fat_fs) {
  if (fat_fs != NULL) {
    if (fat_fs->fat_part != NULL) {
      fclose(fat_fs->fat_part);
    }
    free(fat_fs);
  }
}
fat_fs *fat_fs_init() {
  fat_fs *fs = malloc(sizeof(fat_fs));
  if (fs == NULL) {
    return NULL;
  }

  fs->fat_part = fopen("fat.part", "wb+");

  if (fs->fat_part == NULL) {
    fprintf(stderr, "Erro ao abrir disco FAT fat.part.\n");
    fat_fs_free(fs);
    return NULL;
  }

  fs->fat[0] = 0xfffd;
  for (int i = 1; i < 9; i++) {
    fs->fat[i] = 0xfffe;
  }

  fs->fat[9] = 0xffff;
  for (int i = 10; i < FAT_ENTRIES; i++) {
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

fat_fs *fat_fs_load() {
  fat_fs *fs = malloc(sizeof(fat_fs));

  fs->fat_part = fopen("fat.part", "rb+");
  //setvbuf(fs->fat_part, (char*)NULL, _IONBF, 0);
  if (fs->fat_part == NULL) {
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

char *fat_fs_find_base_dir(fat_fs *fs, char *dir, dir_entry_t *current_dir,
                           int *dir_block, int tokens_offset) {
  int n = strlen(dir), i, j, num_tokens = 0;
  *dir_block = 9;
  char *dir_copy = malloc((n + 1) * sizeof(char));
  // dir_entry_t current_dir[ENTRY_SIZE];

  memcpy(current_dir, fs->root_dir, sizeof(fs->root_dir));
  strcpy(dir_copy, dir);
  char *token = strtok(dir_copy, "/");
  while (token != NULL) {
    num_tokens++;
    token = strtok(NULL, "/");
  }
  free(dir_copy);
  dir_copy = NULL;

  token = strtok(dir, "/");
  for (i = 0; i < num_tokens - tokens_offset; i++) {
    for (j = 0; j < ENTRY_SIZE; j++) {
      if (current_dir[j].first_block != 0 && current_dir[j].attributes == 1 &&
          strcmp((char *)current_dir[j].filename, token) == 0) {
        break;
      }
    }

    if (j == 32) {
      // printf("entrei\n");
      *dir_block = -1;
      return NULL;
    };
    
    *dir_block = current_dir[j].first_block;
    fseek(fs->fat_part, *dir_block * CLUSTER_SIZE, SEEK_SET);
    fread(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
    token = strtok(NULL, "/");
  }

  return token;
}

void fat_fs_mkdir(fat_fs *fs, char *dir) {

  int n = strlen(dir), i, j, empty_entry;
  dir_entry_t current_dir[ENTRY_SIZE];
  // memcpy(current_dir, fs->root_dir, sizeof(current_dir));
  int dir_block;
  char *last_name;
  last_name = fat_fs_find_base_dir(fs, dir, current_dir, &dir_block, 1);
  if (dir_block == -1 && last_name == NULL) {
    fprintf(stderr, "Não foi possível criar o diretorio especificado.\n");
    return;
  }
  // printf("%d\n", dir_block);
  for (j = 0; j < ENTRY_SIZE; j++) {
    if (current_dir[j].first_block != 0 &&
        strcmp((char *)current_dir[j].filename, last_name) == 0) {
      fprintf(
          stderr,
          "Não foi possível criar o diretorio especificado, ele já existe.\n");
      return;
    }
  }

  for (empty_entry = 0; empty_entry < ENTRY_SIZE; empty_entry++)
    if (current_dir[empty_entry].first_block == 0)
      break;

  if (empty_entry == 32) {
    fprintf(stderr, "Não foi possível criar o diretorio especificado.\n");
    return;
  }
  // printf("EWQEsssss\n");
  strcpy((char *)current_dir[empty_entry].filename, last_name);
  // printf("EWQEsssss\n");
  current_dir[empty_entry].attributes = 1;

  int first_block;
  for (first_block = 10; first_block < 4096; first_block++) {
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
  if (dir_block == 9) {
    memcpy(fs->root_dir, current_dir, sizeof(fs->root_dir));
  }

  fseek(fs->fat_part, first_block * CLUSTER_SIZE, SEEK_SET);
  fwrite(new_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
}

void fat_fs_ls(fat_fs *fs, char *dir) {

  int n = strlen(dir), i, dir_block;
  dir_entry_t current_dir[ENTRY_SIZE];
  // memcpy(current_dir, fs->root_dir, sizeof(current_dir));

  char* last_name = fat_fs_find_base_dir(fs, dir, current_dir, &dir_block, 0); 
 
  //printf("%s %d\n", last_name, dir_block);

  if (dir_block == -1 && last_name == NULL) {
    fprintf(stderr, "Não foi possível localizar o diretório especificado.\n");
    return;
  }
  
  int count = 0;
  for (i = 0; i < ENTRY_SIZE; i++) {
    if (current_dir[i].first_block != 0) {
      count++;
    }
  }

  for (int i = 0, j = 0; i < ENTRY_SIZE; i++) {
    if (current_dir[i].first_block != 0) {
      
      print_pipe(j, count);

      if (current_dir[i].attributes == 0) {
        printf("%s\n", current_dir[i].filename);
      } else if (current_dir[i].attributes == 1) {
        printf("%s/\n", current_dir[i].filename);
      }
      j++;
    }
  }
}

void fat_fs_create(fat_fs *fs, char *name) {

  int n = strlen(name), i, j, empty_entry;
  dir_entry_t current_dir[ENTRY_SIZE];
  // memcpy(current_dir, fs->root_dir, sizeof(current_dir));
  int dir_block;
  char *last_name;
  last_name = fat_fs_find_base_dir(fs, name, current_dir, &dir_block, 1);
  if (dir_block == -1 && last_name == NULL) {
    fprintf(stderr, "Não foi possível criar o arquivo especificado.\n");
    return;
  }

  for (j = 0; j < ENTRY_SIZE; j++) {
    if (current_dir[j].first_block != 0 &&
        strcmp((char *)current_dir[j].filename, last_name) == 0) {
      fprintf(
          stderr,
          "Não foi possível criar o arquivo especificado, ele já existe.\n");
      return;
    }
  }

  for (empty_entry = 0; empty_entry < ENTRY_SIZE; empty_entry++) {
    if (current_dir[empty_entry].first_block == 0) {
      break;
    }
  }

  if (empty_entry == 32) {
    fprintf(stderr, "Não foi possível criar o arquivo especificado.\n");
    return;
  }

  // int first_block;
  // for (first_block = 10; first_block < 4096; first_block++)
  // {
  //   if (fs->fat[first_block] == 0x0000)
  //     break;
  // }

  // fs->fat[first_block] = 0xffff;

  strcpy((char *)current_dir[empty_entry].filename, last_name);
  current_dir[empty_entry].attributes = 0;
  current_dir[empty_entry].size = 0;
  current_dir[empty_entry].first_block = 0xffff;

  // dir_entry_t new_dir[ENTRY_SIZE];
  // memset(new_dir, 0, ENTRY_SIZE * sizeof(dir_entry_t));

  fseek(fs->fat_part, CLUSTER_SIZE, SEEK_SET);
  fwrite(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);

  fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
  fwrite(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
  if (dir_block == 9) {
    memcpy(fs->root_dir, current_dir, sizeof(fs->root_dir));
  }
}

void fat_fs_unlink(fat_fs *fs, char *name) {

  int n = strlen(name), i, j, empty_entry;
  dir_entry_t current_dir[ENTRY_SIZE];
  int dir_block;
  char *last_name;
  last_name = fat_fs_find_base_dir(fs, name, current_dir, &dir_block, 1);
  if (dir_block == -1 && last_name == NULL) {
    fprintf(stderr, "Não foi possível deletar o arquivo/diretório especificado.\n");
    return;
  }
  for (i = 0; i < ENTRY_SIZE; i++) {
    if (current_dir[i].first_block != 0 &&
        strcmp((char *)current_dir[i].filename, last_name) == 0) {
      break;
    }
  }

  if (i == 32) {
    fprintf(stderr, "Não foi possível criar o arquivo especificado.\n");
    return;
  }
  if (current_dir[i].attributes == 0) {
    // file
    // dir_entry_t file;
    // fseek(fs->fat_part, current_dir[i].first_block * CLUSTER_SIZE, SEEK_SET);
    // fread(candidate_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
    uint16_t *block = &(current_dir[i].first_block),
             *next_block = &(fs->fat[*block]);
    uint8_t empty_cluster[CLUSTER_SIZE];
    memset(empty_cluster, 0, CLUSTER_SIZE * sizeof(uint8_t));
    while (*block != 0xffff) {
      fseek(fs->fat_part, *block * CLUSTER_SIZE, SEEK_SET);
      fwrite(empty_cluster, sizeof(uint8_t), CLUSTER_SIZE, fs->fat_part);
      *block = 0x0000;
      block = next_block;
      next_block = &(fs->fat[*block]);
    }
    fseek(fs->fat_part, CLUSTER_SIZE, SEEK_SET);
    fwrite(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);
    /* fseek(fs->fat_part, current_dir[i].first_block * CLUSTER_SIZE, SEEK_SET); */
    /* fread(candidate_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part); */
    memset(&current_dir[i], 0, sizeof(dir_entry_t));
  
    fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
    fwrite(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
    if (dir_block == 9) {
      memcpy(fs->root_dir, current_dir, sizeof(fs->root_dir));
    }
    
  } else if (current_dir[i].attributes == 1) {
    // dir
    dir_entry_t candidate_dir[ENTRY_SIZE];
    fseek(fs->fat_part, current_dir[i].first_block * CLUSTER_SIZE, SEEK_SET);
    fread(candidate_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
    for (j = 0; j < ENTRY_SIZE; j++) {
      if (candidate_dir[j].first_block != 0x0000){
        break;
      }
    }
    if (j == ENTRY_SIZE) {
      int first_block = current_dir[i].first_block;
      fs->fat[first_block] = 0x0000;
      memset(&current_dir[i], 0, sizeof(dir_entry_t));
      memset(candidate_dir, 0, sizeof(dir_entry_t) * ENTRY_SIZE);
      fseek(fs->fat_part, CLUSTER_SIZE, SEEK_SET);
      fwrite(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);

      fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
      fwrite(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
      
      if (dir_block == 9) {
        memcpy(fs->root_dir, current_dir, sizeof(fs->root_dir));
      }

      fseek(fs->fat_part, first_block * CLUSTER_SIZE, SEEK_SET);
      fwrite(candidate_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
    } else {
      fprintf(stderr, "Não é possível deletar o diretório.\n");
    }
  }
  
}

void fat_fs_write(fat_fs *fs, char *string, char *name) {
  int i, j, empty_entry, num_characters = strlen(string) + 1;
  int num_required_blocks = ceil((double)num_characters * sizeof(char) / CLUSTER_SIZE);
  //int num_characters_to_write = num_characters;
  dir_entry_t current_dir[ENTRY_SIZE];
  int dir_block;
  char *last_name;
  last_name = fat_fs_find_base_dir(fs, name, current_dir, &dir_block, 1);
  if (dir_block == -1 && last_name == NULL) {
    fprintf(stderr, "Não foi possível escrever no arquivo especificado.\n");
    return;
  }
  for (i = 0; i < ENTRY_SIZE; i++) {
    if (current_dir[i].first_block != 0 &&
        strcmp((char *)current_dir[i].filename, last_name) == 0) {
      break;
    }
  }

  if (i == 32) {
    fprintf(stderr, "Não foi possível escrever no arquivo especificado.\n");
    return;
  }
  uint16_t *block = &current_dir[i].first_block, *next_block = NULL;
  i = 0;
  int fat_next_block = 10;
  uint8_t empty_cluster[CLUSTER_SIZE];
  memset(empty_cluster, 0, CLUSTER_SIZE * sizeof(uint8_t));
  // fseek(fs->fat_part,old_block,SEEK_SET);
  while (*block != 0xffff || i < num_required_blocks) {
    if (i < num_required_blocks) {
      // remove blocks that are not going to be used
      if (*block == 0xffff) {
        for (; fat_next_block < 4096; fat_next_block++) {
          if (fs->fat[fat_next_block] == 0x0000)
            break;
        }
        *block = fat_next_block;
      }
      fseek(fs->fat_part, *block * CLUSTER_SIZE, SEEK_SET);
      fwrite(string, sizeof(char), num_characters,
             fs->fat_part);
      num_characters -= MIN(num_characters,CLUSTER_SIZE);
    }
    next_block = &(fs->fat[*block]);
    if (i >= num_required_blocks) {
      fseek(fs->fat_part, *block * CLUSTER_SIZE, SEEK_SET);
      fwrite(empty_cluster, sizeof(uint8_t), CLUSTER_SIZE, fs->fat_part);
      *block = 0x0000;
    }
    block = next_block;
    if(num_characters == 0)
      *block = 0xffff;
    i++;
  }
  if (num_required_blocks == 0) {
    current_dir[i].first_block = 0xffff;
  }
  fseek(fs->fat_part, dir_block * CLUSTER_SIZE, SEEK_SET);
  fwrite(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);    
  if (dir_block == 9) {
      memcpy(fs->root_dir, current_dir, sizeof(fs->root_dir));
  }
  if (i >= num_required_blocks) {
    fseek(fs->fat_part, CLUSTER_SIZE, SEEK_SET);
    fwrite(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);
  }
}

void fat_fs_read(fat_fs *fs, char *name) {
  int i, j, empty_entry;
  dir_entry_t current_dir[ENTRY_SIZE];
  int dir_block;
  char *last_name;
  last_name = fat_fs_find_base_dir(fs, name, current_dir, &dir_block, 1);
  if (dir_block == -1 && last_name == NULL) {
    fprintf(stderr, "Não foi possível ler o arquivo especificado.\n");
    return;
  }
  for (i = 0; i < ENTRY_SIZE; i++) {
    if (current_dir[i].first_block != 0 &&
        current_dir[i].attributes == 0 &&
        strcmp((char *)current_dir[i].filename, last_name) == 0) {
      break;
    }
  }
   if (i == 32) {
    fprintf(stderr, "Não foi possível ler o arquivo especificado.\n");
    return;
  }
   
  uint16_t block =current_dir[i].first_block;
  char block_content[CLUSTER_SIZE];
  while (block != 0xffff) {
    fseek(fs->fat_part, block*CLUSTER_SIZE, SEEK_SET);
    fread(block_content, sizeof(char), CLUSTER_SIZE, fs->fat_part);
    for(int i = 0; i < CLUSTER_SIZE; i++){
      if(block_content[i]=='\0')
        break;
      printf("%c",block_content[i]);
    }
    block = fs->fat[block];
  }
}
void fat_fs_append(fat_fs *fs, char *string, char *name) {
  int i, j, empty_entry, num_characters = strlen(string) + 2;
  int num_required_blocks = ceil((double)num_characters * sizeof(char) / CLUSTER_SIZE);
  //int num_characters_to_write = num_characters;
  dir_entry_t current_dir[ENTRY_SIZE];
  int dir_block;
  char *last_name;
  last_name = fat_fs_find_base_dir(fs, name, current_dir, &dir_block, 1);
  char* newstring = malloc(num_characters);
  strcpy(newstring,string);
  strcat(newstring,"\n");

  if (dir_block == -1 && last_name == NULL) {
    fprintf(stderr, "Não foi possível escrever no arquivo especificado.\n");
    return;
  }
  for (i = 0; i < ENTRY_SIZE; i++) {
    if (current_dir[i].first_block != 0 &&
        strcmp((char *)current_dir[i].filename, last_name) == 0) {
      break;
    }
  }

  if (i == 32) {
    fprintf(stderr, "Não foi possível escrever no arquivo especificado.\n");
    return;
  }
  
  uint16_t block = &current_dir[i].first_block, *next_block = &fs->fat[block];
  i = 0;
  int fat_next_block = 10;
  uint8_t empty_cluster[CLUSTER_SIZE];
  char block_content[CLUSTER_SIZE];
  memset(empty_cluster, 0, CLUSTER_SIZE * sizeof(uint8_t));
  // fseek(fs->fat_part,old_block,SEEK_SET);
  while (block != 0xffff) {
    fseek(fs->fat_part, block*CLUSTER_SIZE, SEEK_SET);
    fread(block_content, sizeof(char), CLUSTER_SIZE, fs->fat_part);
    block = next_block;
    next_block = &(fs->fat[block]);
    if(num_characters == 0 ||i > num_required_blocks)
      block = 0xffff;
    if(block = 0xffff)
       break;
    i++;
  }
  int end = strlen(&fs->fat[i]);

  fseek(fs->fat_part, dir_block * CLUSTER_SIZE + end, SEEK_SET);
  fwrite(current_dir, sizeof(dir_entry_t), ENTRY_SIZE, fs->fat_part);
  int letters = 0;  
  for (j = 0; j < CLUSTER_SIZE; j ++){
		if (letters >= strlen(newstring)){
            break;
            }
			if (&fs->fat[j] == 0x0000){
				fs->fat[j] = newstring[letters];
		letters++;
	}
    
  if (dir_block == 9) {
      memcpy(fs->root_dir, current_dir, sizeof(fs->root_dir));
  }
  if (i >= num_required_blocks) {
    fseek(fs->fat_part, CLUSTER_SIZE, SEEK_SET);
    fwrite(fs->fat, sizeof(uint16_t), FAT_ENTRIES, fs->fat_part);
    }
  }
}