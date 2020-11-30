#include <stdio.h>
#include <string.h>
#include "fat.h"
#include "utils.h"

#define BUFFER_SIZE 256
#define COMMAND_SIZE 16
#define MAX_NUM_ARGS 2
#define ARG_SIZE (BUFFER_SIZE-COMMAND_SIZE)/MAX_NUM_ARGS
#define STR_EQUAL(a, b) strcmp(a, b) == 0

int main() {
    char buffer[BUFFER_SIZE];
    char command[COMMAND_SIZE], arg1[ARG_SIZE], arg2[ARG_SIZE];
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

        split_user_input(buffer, command, arg1, arg2);

        if (start == 0 && !STR_EQUAL(command, "init") && 
            !STR_EQUAL(command, "load") && !STR_EQUAL(command, "quit")) {
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
                    fprintf(stderr, "ls [caminho/diretório]\n");
            }

            else if (STR_EQUAL(command, "mkdir")){
                if (arg1[0] != '\0')
                    fat_fs_mkdir(fs, arg1);
                else
                    fprintf(stderr, "mkdir [caminho/diretório]\n");
            }

            else if (STR_EQUAL(command, "create")){
                if (arg1[0] != '\0')
                    fat_fs_create(fs,arg1);
                else
                    fprintf(stderr, "create [caminho/arquivo]\n");
            }

            else if (STR_EQUAL(command, "unlink")){
                if (arg1[0] != '\0')
                    fat_fs_unlink(fs,arg1);
                else
                    fprintf(stderr, "unlink [caminho/diretorio/arquivo]\n");
            }

            else if (STR_EQUAL(command, "write")){
                if (arg2[0] == '\0' || arg1[0] == '\0')
                    fprintf(stderr, "write \"texto\" [caminho/arquivo]\n");   
                else if (arg1[0] != '"' || arg1[strlen(arg1)-1] != '"') 
                    fprintf(stderr, "A string deve estar entre aspas.\n");   
                else {
                    remove_quotes(arg1);
                    printf("%s\n",arg1);
                    fat_fs_write(fs,arg1,arg2);
                }
            }

            else if (STR_EQUAL(command, "append")){
                if (arg2[0] == '\0' || arg1[0] == '\0')
                    fprintf(stderr, "append \"texto\" [caminho/arquivo]\n");  
                else if (arg1[0] != '"' || arg1[strlen(arg1)-1] != '"') 
                    fprintf(stderr, "A string deve estar entre aspas.\n");
                else {
                    //snprintf(arg1, sizeof(arg1), "%s", removeQuotes(arg1));
                    remove_quotes(arg1);
                    //fat_fs_append(fs,arg1,arg2);
                }
            }

            else if (STR_EQUAL(command, "read")){
                if (arg1[0] != '\0')
                    fat_fs_read(fs, arg1);
                else  
                    fprintf(stderr, "read [caminho/arquivo]\n");
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
