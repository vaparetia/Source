#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _database_c_
#include "database.h"
#include "token.h"


static char buf[256];

int dbLoadDatabase(char *fname)
{
  char *name;
  TOKEN *tkn;

  name = (NULL == fname) ? DEFAULT_DBASE : fname;
  if(NULL == (tkn = tknOpen(name))) return -1;

  line = NULL;






}

/* トークンの読み込み */
static char * db_read_token(TOKEN *tkn)
{
  if(NULL == tknReadToken(tkn, buf, 256, "<>")) return NULL;
  if(buf[0] == '<')
    if(NULL == tknReadWhileTerminator(tkn, buf + 1, 255, ">")) return NULL;
  return buf;  
}

/* <phonome> ～ </phonome> セクションの読み込み */
idbPhonome * read_phonome(TOKEN *tkn)
{
  dbPhonome * ph;
  int doing;

  if(NULL == (ph = malloc(sizeof(dbPhonome)))) return NULL;
  doing = 1;

  while(doing)
    {
      db_read_token(tkn);
      
    }

}
