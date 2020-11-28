#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getCommand(char* buffer,char*  command,char*  arg1,char*  arg2) {
    setbuf(stdin, NULL);
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    setbuf(stdin, NULL);
    int i;
    for (i = 0; buffer[i] != ' '; i++) {
        command[i] = buffer[i];
    }
    command[i++] = '\0';
    for (int j = 0; buffer[i] != ' ' && buffer[i] != '\0'; j++, i++) {
        arg1[j] = buffer[i];
    }
    arg1[i++] = '\0';
    for (int j = 0; buffer[i] != '\0'; j++, i++) {
        arg2[j] = buffer[i];
    }
    arg2[i++] = '\0';
}