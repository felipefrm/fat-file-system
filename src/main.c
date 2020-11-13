#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CLUSTER_SIZE 1024

/* entrada de diretorio, 32 bytes cada */
typedef struct {
    uint8_t filename[18];
    uint8_t attributes;
    uint8_t reserved[7];
    uint16_t first_block;
    uint32_t size;
} dir_entry_t;

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
uint16_t fat[4096];

/* diretorios (incluindo ROOT), 32 entradas de diretoriocom 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
union {
    dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
    uint8_t data[CLUSTER_SIZE];
} data_cluster;

int main() {
    char buffer[100];
    char command[10];
    while (1) {
        setbuf(stdin, NULL);
        fgets(buffer, 100, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        setbuf(stdin, NULL);

        for (int i = 0; buffer[i] != ' '; i++)
            command[i] = buffer[i];        

        if (strcmp(command, "init") == 0) {}
        else if (strcmp(command, "load") == 0){}
        else if (strcmp(command, "ls") == 0){}
        else if (strcmp(command, "mkdir") == 0){}
        else if (strcmp(command, "create") == 0){}
        else if (strcmp(command, "unlink") == 0){}
        else if (strcmp(command, "write") == 0){}
        else if (strcmp(command, "append") == 0){}
        else if (strcmp(command, "read") == 0){}
        else if (strcmp(command, "quit") == 0) {
            break;
        }
        else {
            printf("Unknown command.\n");
        }
    }
    return 0;
}