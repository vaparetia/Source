#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#define _util_c_
#include "util.h"
#include "debug.h"

int pool_init(void)
{
  if(NULL != poolbuf) free(poolbuf);
  pool_siz = 0;
  poolcnt = 0;
  if(NULL == (poolbuf = malloc(INIT_POOL_SIZE))) return -1;
  pool_siz = INIT_POOL_SIZE;
  return 0;
}

int pool(char * str)
{
  int i, ret;
  void *vpt;

  DBG("entering pool()\n");
  ret = poolcnt;
  while((i = poolcnt + strlen(str) + 1) >= pool_siz)
    {
      if(NULL == (vpt = realloc(poolbuf, pool_siz + POOL_ADD)))
	{
	  fprintf(stderr, "not enough memory.\n");
	  return -1;
	}
      poolbuf = vpt;
      pool_siz += POOL_ADD;
    }
  strcpy(poolbuf + poolcnt, str);
  poolcnt = i;
  DBG("pool() was success.\n");
  return ret;
}
