#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _info_c_
#include "info.h"
#include "2d_data.h"

int infoOutputInfo(char * path, sprLayout * layout)
{
  FILE * wfp;
  sprAction * act;
  int num;
  size_t len;

  /* アクションの数を数える */
  num = 0;
  for(act = layout->actBegin; act != NULL; act = act->next) num++;

  if(NULL == (wfp = fopen(path, "wb"))) return -1;

  fputl(num, wfp);   /* 個数を収録 */
  for(act = layout->actBegin; act != NULL; act = act->next)
    {
      len = strlen(act->label);  /* アクション名の長さ */
      fwrite(act->label, 1, len + 1, wfp); /* 文字列を最後の 0 まで書き込む */
    }
  
  fclose(wfp);
  return 0;
}
