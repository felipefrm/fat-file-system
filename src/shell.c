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
    int start = 0;

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

        if (start == 0 && !STR_EQUAL(command, "init") && 
            !STR_EQUAL(command, "load") && !STR_EQUAL(command, "init")) {
            fprintf(stderr, 
            "Antes de executar qualquer operação com a FAT é necessário "
            "inicializar (init) ou carregar (load) a partição.\n");
        }

        else {
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
            start = 1;
        }
    }
    fat_fs_free(fs);
    return 0;
}
