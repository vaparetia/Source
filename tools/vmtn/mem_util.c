#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _mem_util_c_
#include "mem_util.h"

void * mem_add(void * pre_buf, size_t single_siz, int now_nums)
{
  void * vpt;

  vpt = (!now_nums)
    ? malloc(single_siz)
    : realloc(pre_buf, single_siz * (now_nums + 1));

  if(NULL == vpt) return NULL;
  return vpt;
}
