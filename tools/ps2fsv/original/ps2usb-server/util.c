#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _util_c_
#include "util.h"

char * dup_string(char * str)
{
  char *dup;

  if(NULL == (dup = malloc(strlen(str) + 1))) return NULL;

  strcpy(dup, str);
  return dup;
}
