#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setArgs(const char *frm, const char *to, char *command, char *arg1,
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

void splitUserInput(const char *s, char *command, char *arg1, char *arg2) {
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
        setArgs(start, s, command, arg1, arg2, &iter);
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
    setArgs(start, s, command, arg1, arg2, &iter);
  }

}

char* removeQuotes(const char *string) {
    char *p = string;
    p++;
    p[strlen(p)-1] = 0;
    printf("%s\n", p);
    return p;
}