#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _util_c_
#include "util.h"

char * utilDupString(char *str)
{
  char * buf;

  if(NULL == (buf = malloc(strlen(str) + 1))) return NULL;
  strcpy(buf, str);
  return buf;
}
