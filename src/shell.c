#include <stdio.h>
#include <string.h>
#include "fat.h"
#include "utils.h"

#define BUFFER_SIZE 256
#define STR_EQUAL(a, b) strcmp(a, b) == 0

int main() {
    char buffer[BUFFER_SIZE];
    char command[16], arg1[120], arg2[120];
    FILE* fat_part = openFAT();
    while(1) {
        printf("fatshell~$ ");
        getCommand(buffer, command, arg1, arg2);
        if (STR_EQUAL(command, "init")) {
            init(fat_part);
        }
        else if (STR_EQUAL(command, "load")){
            if (fat_part != NULL) 
                load(fat_part);
            else 
                fprintf(stderr, "fat must be initialized.\n");
        }
        else if (STR_EQUAL(command, "ls")){
            // ls("teste");
        }
        else if (STR_EQUAL(command, "mkdir")){
            mkdir(fat_part, "teste");
        }
        else if (STR_EQUAL(command, "create")){}
        else if (STR_EQUAL(command, "unlink")){}
        else if (STR_EQUAL(command, "write")){}
        else if (STR_EQUAL(command, "append")){}
        else if (STR_EQUAL(command, "read")){}
        else if (STR_EQUAL(command, "quit")){
            break;
        }
        else {
            printf("Unknown command.\n");
        }
    }
    fclose(fat_part);
    return 0;
}