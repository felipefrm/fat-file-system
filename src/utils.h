#ifndef UTILS_H
#define UTILS_H

#define BLUE "\e[1;34m"
#define GREEN "\e[1;32m"
#define RESET "\e[0m" 

void split_user_input(const char *s, char *command, char *arg1, char *arg2);
void remove_quotes(char *string);
void print_pipe(int j, int count);

#endif