#include <stdio.h>
#include <string.h>
#include "fat.h"

#define BUFFER_SIZE 256
#define STR_EQUAL(a, b) strcmp(a, b) == 0

int main() {
    char buffer[BUFFER_SIZE];
    char command[10];
    while(1) {
        printf("fatshell~$ ");
        setbuf(stdin, NULL);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        setbuf(stdin, NULL);

        for (int i = 0; buffer[i] != ' '; i++)
            command[i] = buffer[i];        

        if (STR_EQUAL(command, "init")) {
            init();
        }
        else if (STR_EQUAL(command, "load")){
            load();
        }
        else if (STR_EQUAL(command, "ls")){}
        else if (STR_EQUAL(command, "mkdir")){
            mkdir("teste");
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
    return 0;
}