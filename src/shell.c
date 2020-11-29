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
    
    while(1) {
        printf("fatshell~$ ");
        setbuf(stdin, NULL);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        setbuf(stdin, NULL);

        command[0] = '\0';
        arg1[0] = '\0';
        arg2[0] = '\0';

        splitUserInput(buffer, command, arg1, arg2);
        printf("command: %s\narg1: %s\narg2: %s\n", command, arg1, arg2);



        // int spaces = 0;
        // for (int i = 0; i < strlen(buffer); i++) {
        //     if (buffer[i] == ' ') {
        //         while (buffer[i] == ' ')
        //             i++;
        //         spaces++;
        //     }
        // }

        // // printf("Espaços: %d\n", spaces);

        // char *token = strtok(buffer, " ");
        // strcpy(command, token);
        // int start = -1, end;
        // if (STR_EQUAL(command, "write") || STR_EQUAL(command, "append")) {
        //     for (int i = 0; i < strlen(buffer); i++) {
        //         printf("indice: %d / char: %c\n", i, buffer[i]);
        //         if (buffer[i] == '"' && start < 0) {
        //             start = i;
        //         }
        //         if (buffer[i] == '"' && start > 0) {
        //             end = i;
        //         }
        //     }
        //     printf("%d %d\n", start, end);
        //     for (int i = start+1, j = 0; i < end; i++, j++) {
        //         arg1[j] = buffer[i];
        //     }
        // }

        // else {
        //     for (int j=0; token != NULL; j++) {
        //         if (j == 0)
        //             strcpy(command, token);
        //         else if (j == 1) {
        //             strcpy(arg1, token);
        //         }
        //         else if (j == spaces)
        //             strcpy(arg2, token);
        //         else {
        //             sprintf(arg1, "%s %s", arg1, token);
        //         }
        //         token = strtok(NULL, " ");
        //     }
        // }

        // printf("command: %s\narg1: %s\narg2: %s\n", command, arg1, arg2);

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
            else {
                strcpy(arg1, removeQuotes(arg1));
                fat_fs_write(fs,arg1,arg2);
            }
        }
        else if (STR_EQUAL(command, "append")){
            if (arg1[0] != '"' || arg1[strlen(arg1)-1] != '"') {
                fprintf(stderr, "A string deve estar entre aspas.\n");
            }
            else {
                strcpy(arg1, removeQuotes(arg1));
                //fat_fs_append(fs,arg1,arg2);
            }
        }
        else if (STR_EQUAL(command, "read")){
            fat_fs_read(fs, arg1);
        }
        else if (STR_EQUAL(command, "quit")){
            break;
        }
        else {
            printf("Unknown command.\n");
        }
        fflush(fs->fat_part);
    }
    fat_fs_free(fs);
    return 0;
}
