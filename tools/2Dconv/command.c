#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _command_c_
#include "debug.h"
#include "my_malloc.h"
#include "util.h"
#include "token.h"
#include "label.h"

#include "command.h"

#include "objdef.h"   /* オブジェクト定義セクションの命令処理 */
#include "objstat.h"  /* 状態定義セクションの命令処理         */
#include "objact.h"   /* アクション定義セクションの命令処理   */

#include "output.h"

static char tkn_buf[ CMD_TKNBUF ];


static tknFILE * cmdSrc;    /* 入力元             */
static FILE    * cmdDst;    /* 出力先             */

static cmdIndex cmd_list[] = {
  /* オブジェクト定義セクション命令 */
  { "obj",    defObjDefine },  /* オブジェクト定義 */

  /* 状態定義セクション命令 */
  { "reftri", statRefTri },    /* 参照 .tri 指定   */
  { "status", statDefStatus }, /* 状態定義         */

  /* アクション定義セクション命令 */
  { "action", actDefAction },  /* アクション定義開始 */

  { NULL,     NULL }
};


void * cmdMalloc(size_t size)
{
  void * ptr;

  if(NULL == (ptr = MALLOC(size)))
    cmdStatus = CMD_ERR_NO_MEMORY;
  return ptr;
}

char * cmdStrDup(char * str)
{
  size_t len;
  char * buf;

  len = strlen(str);
  if(NULL != (buf = cmdMalloc(len + 1))) strcpy(buf, str);
  return buf;
}

/*
 * コマンドとして認識する文字列を取得する。
 * コマンドは必ず TKN_TOKEN でなければならない。
 *
 * コマンド部の取得は必ずこの関数を使うこと。
 * パラメータの途中に改行が入ることは文法上無いが、コマンドの前に空行が入ることは
 * 充分にありうる。
 */
char * cmdGetCommand(tknFILE * tkn)
{
  char * cmd;

  /* 改行のみの行は無視する。*/
  do {
    if(NULL == tknGetToken(tkn, tkn_buf, CMD_TKNBUF)) return NULL;
  } while(tkn_buf[0] == '\n');

  if(TKN_TOKEN != tknGetStat(tkn)) goto syntax_err;
  if(NULL == (cmd = cmdStrDup(tkn_buf))) return NULL;
  return cmd;

 syntax_err:
  cmdStatus = CMD_ERR_SYNTAX;
  return NULL;
}

char * cmdGetToken(tknFILE * tkn, tknStat stat)
{
  char * buf;

  if(NULL == tknGetToken(tkn, tkn_buf, CMD_TKNBUF))
    {
      cmdStatus = CMD_ERR_SYNTAX;
      return NULL;
    }
  if(stat != tknGetStat(tkn))
    {
      cmdStatus = CMD_ERR_SYNTAX;
      return NULL;
    }
  /* クォート文字列の場合、クォート記号を除去する */
  if(stat == TKN_QUOTED) tknStripQuote(tkn, tkn_buf);
  buf = cmdStrDup(tkn_buf);
  return buf;
}

int cmdInit(char * src, char * dst)
{
  /* 入出力ファイルのオープン */
  if(NULL == (cmdSrc = tknOpen(src))) return -1;
  if(NULL == (cmdDst = fopen(dst, "wb"))) goto err0;
  DBG("src = %s\ndst = %s\n", src, dst);

  DBG("cmd[1]\n");

  /* ラベル領域の確保 */
  if(NULL == (cmdLabel = lblCreateLabelBlock())) goto err1;

  DBG("cmd[2]\n");

  /* デリミタ, 空白類の設定 */
  tknSetDelim(cmdSrc, CMD_DELIM, CMD_BLANK, CMD_QUOTE);
  cmdStatus = 0;

  DBG("cmd[3]\n");

  /* 作業領域の初期化 */
  defInitObjectDefine();   /* オブジェクト定義領域の初期化 */
  statInitObjectStatus();  /* 状態定義領域の初期化         */
  actInitObjectAction();   /* アクション定義領域の初期化   */

  return 0;
  
 err1:
  fclose(cmdDst);
 err0:
  tknClose(cmdSrc);
  DBG("initialize error.\n");
  exit(EXIT_FAILURE);
  return -1;
}

/*
 * 与えられたコマンドテーブルに従い、適切な関数を呼び出して処理する
 */
int cmdConvert(tknFILE * tkn, cmdIndex * cmd_list, int (*func)(char * token))
{
  int i, stat;
  char * cmd;

  cmdStatus = 0;
  while(NULL != (cmd = cmdGetCommand(tkn)))
    {
      if(NULL != func)
	if((func)(cmd)) break;

      /* 該当するコマンドの処理関数を呼ぶ */
      stat = CMD_ERR_SYNTAX;
      for(i = 0; cmd_list[i].cmd != NULL; i++)
	if(!strcmp(cmd, cmd_list[i].cmd))
	  {
	    DBG("accept command '%s':\n", cmd);
	    stat = (cmd_list[i].func)(cmd, cmdSrc, cmdLabel, cmdDst);
	    break;
	  }
      FREE(cmd);
      cmdStatus = stat;
      /* 該当するコマンドがリスト中に無かった場合の処理 */
      if(cmd_list[i].cmd == NULL)
	{
	  cmdStatus = CMD_ERR_SYNTAX;
	  break;
	}
    }
  
  return cmdStatus;
}


int cmdConvMain(char * src, char * dst)
{
  int ret;
  int lnum;

  ret = cmdConvert(cmdSrc, cmd_list, NULL);
  if(ret < 0)
    {
      lnum = tknGetLine(cmdSrc);
      switch(ret)
	{
	case CMD_ERR_SYNTAX:
	  fprintf(stderr, "%d: syntax error\n", lnum);
	  break;
	case CMD_ERR_NO_MEMORY:
	case CMD_ERR_DUPLICATED:
	  break;
	}
      return ret;
    }

  /* 問題なくテキストを読めたら、出力形式での出力を行う */
  ret = outOutputMain(cmdDst);

  fclose(cmdDst);
  tknClose(cmdSrc);

  return ret;
}
