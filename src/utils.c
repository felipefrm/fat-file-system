#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"

void set_args(const char *frm, const char *to, char *command, char *arg1,
             char *arg2, int *iter) {
  int commandIter = 0, commandArg1 = 0, commandArg2 = 0;
  if ((*iter) == 0) {
    while (frm < to)
      command[commandIter++] = *frm++;
    command[commandIter] = '\0';
  } else if ((*iter) == 1) {
    while (frm < to)
      arg1[commandArg1++] = *frm++;
    arg1[commandArg1] = '\0';
  } else if ((*iter) == 2) {
    while (frm < to)
      arg2[commandArg2++] = *frm++;
    arg2[commandArg2] = '\0';
  }
  (*iter)++;
}

void reset_args(char* command, char* arg1, char* arg2) {
  command[0] = '\0';
  arg1[0] = '\0';
  arg2[0] = '\0';
}

void split_user_input(const char *s, char *command, char *arg1, char *arg2) {
  reset_args(command, arg1, arg2);
  const char *start;
  int iter = 0;
  int state = ' ';
  while (*s) {
    switch (state) {
    case '\n': // Could add various white-space here like \f \t \r \v
    case ' ':  // Consuming spaces
      if (*s == '\"') {
        start = s;
        state = '\"'; // begin quote
      } else if (*s != ' ') {
        start = s;
        state = 'T';
      }
      break;
    case 'T': // non-quoted text
      if (*s == ' ') {
        set_args(start, s, command, arg1, arg2, &iter);
        state = ' ';
      } else if (*s == '\"') {
        state = '\"'; // begin quote
      }
      break;
    case '\"': // Inside a quote
      if (*s == '\"') {
        state = 'T'; // end quote
      }
      break;
    }
    s++;
  } // end while
  if (state != ' ') {
    set_args(start, s, command, arg1, arg2, &iter);
  }

}

void remove_quotes(char *str) {
    size_t len = strlen(str);
    assert(len >= 2);
    memmove(str, str+1, len-2);
    str[len-2] = '\0';
}

void print_pipe(int j, int count) {
      if (count == 1 || (j > 0 && j < count-1))
        printf("%s├ %s", BLUE, RESET);
      else if (j == 0)
        printf("%s┌ %s", BLUE, RESET);
      else if (j == count-1)
        printf("%s└ %s", BLUE, RESET);
}