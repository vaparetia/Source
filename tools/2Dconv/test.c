#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _test_c_
#include "token.h"
#include "label.h"
#include "command.h"

static char tkn_buf[256];

int main(int argc, char **argv)
{
  tknFILE * tkn;
  tknStat   stat;
  char * attrib;

  if(argc < 2)
    return EXIT_FAILURE;

  if(NULL == (tkn = tknOpen(argv[1])))
    {
      fprintf(stderr, "could not open '%s'\n", argv[1]);
      return EXIT_FAILURE;
    }

  tknSetDelim(tkn, CMD_DELIM, CMD_BLANK, CMD_QUOTE);
  while(NULL != tknGetToken(tkn, tkn_buf, 256))
    {
      if(*tkn_buf == '\n') strcpy(tkn_buf, "\\n");
      stat = tknGetStat(tkn);
      attrib = "unknown";
      switch(stat)
	{
	case TKN_DELIM:   attrib = "delim";  break;
	case TKN_TOKEN:   attrib = "token";  break;
	case TKN_QUOTED:  attrib = "quoted"; break;
	case TKN_UNKNOWN: attrib = "unknown"; break;
	}
      printf("[%s](%s)\n", tkn_buf, attrib);
    }
  
  tknClose(tkn);
  return EXIT_SUCCESS;
}
