#include <stdio.h>
#include <string.h>
#include "fat.h"
#include "utils.h"

#define BUFFER_SIZE 256
#define STR_EQUAL(a, b) strcmp(a, b) == 0

int main() {
    char buffer[BUFFER_SIZE];
    char command[16], arg1[120], arg2[120];
    fat_fs* fs=NULL;
    // FILE* fat_part = fopen("fat.part", "wb+");
    // uint16_t boot = 0xbbbb;
    // printf("%p\n",fat_part);  
    // for (int i = 0; i < 512; i++)
    //     fwrite(&boot, sizeof(boot), 1, fat_part);
    // fclose(fat_part);
    while(1) {
        printf("fatshell~$ ");
        setbuf(stdin, NULL);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        setbuf(stdin, NULL);

        command[0] = '\0';
        arg1[0] = '\0';
        arg2[0] = '\0';

        int spaces = 0;
        for (int i = 0; i < strlen(buffer); i++) {
            if (buffer[i] == ' ') {
                while (buffer[i] == ' ')
                    i++;
                spaces++;
            }
        }

        // printf("Espaços: %d\n", spaces);

        char *token = strtok(buffer, " ");
        for (int j=0; token != NULL; j++) {
            if (j == 0)
                strcpy(command, token);
            else if (j == 1) {
                strcpy(arg1, token);
            }
            else if (j == spaces)
                strcpy(arg2, token);
            else {
                sprintf(arg1, "%s %s", arg1, token);
            }
            token = strtok(NULL, " ");
        }

        // printf("command: %s\narg1: %s\narg2: %s\n", command, arg1, arg2);

        // printf("%c  %c", arg1[0], arg1[strlen(arg1)-1]);

        if (STR_EQUAL(command, "init")) {
            if(fs != NULL)
                fat_fs_free(fs);
            fs=fat_fs_init();
        }
        else if (STR_EQUAL(command, "load")){
            if(fs != NULL)
                fat_fs_free(fs);
            fs=fat_fs_load();
        }
        else if (STR_EQUAL(command, "ls")){
            if (arg1[0] != '\0') {
              fat_fs_ls(fs,arg1);
            }
            else
                fprintf(stderr, "ls [path/directory]\n");
        }
        else if (STR_EQUAL(command, "mkdir")){
          if (arg1[0] != '\0')
                fat_fs_mkdir(fs, arg1);
          else
                fprintf(stderr, "mkdir [path/directory]\n");
        }
        else if (STR_EQUAL(command, "create")){
          //create(fat_part,arg1);
          if (arg1[0] != '\0')
            fat_fs_create(fs,arg1);
          else
            fprintf(stderr, "create [file name]\n");
        }
        else if (STR_EQUAL(command, "unlink")){
            fat_fs_unlink(fs,arg1);
        }
        else if (STR_EQUAL(command, "write")){
            if (arg1[0] != '"' || arg1[strlen(arg1)-1] != '"') {
                fprintf(stderr, "A string deve estar entre aspas.\n");
            }
        }
        else if (STR_EQUAL(command, "append")){
            if (arg1[0] != '"' || arg1[strlen(arg1)-1] != '"') {
                fprintf(stderr, "A string deve estar entre aspas.\n");
            }
        }
        else if (STR_EQUAL(command, "read")){}
        else if (STR_EQUAL(command, "quit")){
            break;
        }
        else {
            printf("Unknown command.\n");
        }
    }
    fat_fs_free(fs);
    return 0;
}
