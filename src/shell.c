#include <stdio.h>
#include <string.h>
#include "fat.h"
#include "utils.h"

#define BUFFER_SIZE 256
#define STR_EQUAL(a, b) strcmp(a, b) == 0

int main() {
    char buffer[BUFFER_SIZE];
    char command[16], arg1[120], arg2[120];
    FILE* fat_part=NULL;
    while(1) {
        printf("fatshell~$ ");
        setbuf(stdin, NULL);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        setbuf(stdin, NULL);

        command[0] = '\0';
        arg1[0] = '\0';
        arg2[0] = '\0';

        char *token = strtok(buffer, " ");
        for (int i=0; token != NULL; i++) {
            if (i == 0)
                strcpy(command, token);
            else if (i == 1)
                strcpy(arg1, token);
            else if (i == 2)
                strcpy(arg2, token);
            token = strtok(NULL, " ");
        }

        if (STR_EQUAL(command, "init")) {
            init(&fat_part);
        }
        else if (STR_EQUAL(command, "load")){
            if (fat_part == NULL) 
                load(&fat_part);
            else 
                fprintf(stderr, "fat must be initialized.\n");

        }
        else if (STR_EQUAL(command, "ls")){
            if (arg1[0] != '\0') {
              ls(fat_part,arg1);
            }
            else
                fprintf(stderr, "ls [path/directory]\n");
        }
        else if (STR_EQUAL(command, "mkdir")){
          if (arg1[0] != '\0')
                mkdir(fat_part, arg1);
          else
                fprintf(stderr, "mkdir [path/directory]\n");
        }
        else if (STR_EQUAL(command, "create")){
          create(fat_part,arg1);
        }
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
