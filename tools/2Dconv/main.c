#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _main_c_
#include "debug.h"
#include "my_malloc.h"
#include "main.h"
#include "token.h"
#include "label.h"
#include "command.h"
#include "fname.h"

#define SIZ 256

static void syntax_error(void)
{
  exit(EXIT_FAILURE);
}

static void no_mem_error(void)
{
  fprintf(stderr, "Not enough memory.\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
  char * src_name;
  char * dst_name;
  int ret;

  if(argc < 2)
    {
      fprintf(stderr, "usage: %s filename\n", argv[0]);
      return EXIT_FAILURE;
    }
  src_name = argv[1];
  if(NULL == (dst_name = fnmMakeSuffixExchanged(src_name, DST_SUFFIX)))
    no_mem_error();

  /* 構文解析初期化 */
  cmdInit(src_name, dst_name);
  ret = cmdConvMain(src_name, dst_name);
  FREE(dst_name);
  
  switch(ret)
    {
    case CMD_ERR_SYNTAX:
      syntax_error();
      break;   /* 一応書いておく */
    case CMD_ERR_NO_MEMORY:
      no_mem_error();
      break;   /* 一応書いておく */
    case CMD_ERR_DUPLICATED:
      syntax_error();
      break;
    }

  return EXIT_SUCCESS;
}
