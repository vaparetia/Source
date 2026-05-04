#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _path_c_
#include "path.h"

char * pathAddDirectory(char * retbuf, char * dir, char * file)
{
  char * buf;
  size_t = len;
  buf = (NULL != retbuf) ? retbuf
    : malloc(strlen(dir) + strlen(file) + 2);
  if(NULL == buf) return NULL;

  strcpy(buf, dir);
  len = strlen(dir);
  if(len > 0)
    if(*(dir + len - 1) != '/')  strcat(buf, "/");

  strcat(buf, file);

  return buf;
}
