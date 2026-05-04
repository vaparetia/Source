#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _objstat_c_
#include "debug.h"
#include "my_malloc.h"
#include "util.h"
#include "token.h"
#include "label.h"
#include "command.h"
#include "objdef.h"
#include "objstat.h"


static int stat_release_status(statBlock * parent, statStatus * stat)
{

  if(stat->stat_name != NULL) FREE(stat->stat_name);
  if(stat->vertex != NULL) FREE(stat->vertex);
  if(stat->rgba != NULL) FREE(stat->rgba);

  if(parent->d.st.stat_begin == stat) parent->d.st.stat_begin = stat->next;
  if(parent->d.st.stat_end   == stat) parent->d.st.stat_end   = stat->prev;
  if(stat->prev != NULL) stat->prev->next = stat->next;
  if(stat->next != NULL) stat->next->prev = stat->prev;

  FREE(stat);
  return 0;
}

static int stat_release_block(statBlock * block)
{
  if(block->mode == MODE_STATUS)
    {
      statStatus *stat, *next;
      stat = block->d.st.stat_begin;
      while(NULL != stat)
	{
	  next = stat->next;
	  stat_release_status(block, stat);
	  stat = next;
	}
    }

  if(statLink.begin == block) statLink.begin = block->next;
  if(statLink.end   == block) statLink.end   = block->prev;
  if(block->prev != NULL) block->prev->next = block->next;
  if(block->next != NULL) block->next->prev = block->prev;

  FREE(block);
  return 0;
}

/*
 * 状態定義作業域初期化
 */
int statInitObjectStatus(void)
{
  statBlock *stat, *nxt;

  stat = statLink.begin;
  /* 登録済のものはすべて開放する */
  while(NULL != stat)
    {
      nxt = stat->next;
      stat_release_block(stat);
      stat = nxt;
    }

  statLink.begin = statLink.end = NULL;
  statLink.now = NULL;

  return 0;
}

/*
 * statStatus 構造体の登録
 */
static int stat_regist_status(statBlock * block, statStatus * stat)
{
  stat->prev = block->d.st.stat_end;
  if(NULL != block->d.st.stat_end) block->d.st.stat_end->next = stat;
  block->d.st.stat_end = stat;
  stat->next = NULL;
  if(NULL == block->d.st.stat_begin) block->d.st.stat_begin = stat;

  return 0;
}

static int stat_regist_block(statBlock * block)
{
  block->prev = statLink.end;
  if(statLink.end != NULL) statLink.end->next = block;
  statLink.end = block;
  block->next = NULL;
  if(statLink.begin == NULL) statLink.begin = block;
  return 0;
}

/*
 * 参照 tri 指定命令
 */
int statRefTri(char * cmd, tknFILE * tkn, lblBlock * block, FILE * wfp)
{
  statBlock * stat;
  char * triname;
  int code;
  int i;
  if(NULL == (triname = cmdGetToken(tkn, TKN_TOKEN))) return cmdStatus;

  /* suffix を除去した文字列の StrCode を求める */
  for(i = 0; triname[i]; i++)
    if(triname[i] == '.')
      {
	triname[i] = 0;
	break;
      }

  code = strcode(triname);
  FREE(triname);
  if(NULL == (stat = cmdMalloc(sizeof(statBlock)))) return cmdStatus;

  stat->mode = MODE_REFTRI;
  stat->d.tri.tri_code = code;

  return stat_regist_block(stat);
}

static int stat_init_status(statStatus * stat, char * stat_name, int id_obj)
{
  int vertex;
  int i;

  stat->stat_name = stat_name;

  /* 指定されたオブジェクトの頂点数を得る */
  if(0 > (vertex = defGetObjVertexNum(id_obj)))
    return (cmdStatus = CMD_ERR_SYNTAX);

  stat->disp = 0;   /* デフォルトは非表示 */
  stat->alpha = 0;  /* デフォルトはアルファブレンディング無し */
  stat->tex = -1;   /* テクスチャ未設定 */
  stat->vertex_num = vertex;
  if(NULL == (stat->vertex = cmdMalloc(sizeof(statPoint) * vertex)))
    return cmdStatus;

  if(NULL == (stat->rgba = cmdMalloc(sizeof(statRGBA) * vertex)))
    {
      FREE(stat->vertex);
      return cmdStatus;
    }

  /* 頂点の値は、座標、RGBA 値ともに設定されていないことにする。 */
  for(i = 0; i < vertex; i++)
    {
      stat->rgba[i].flag = 0;
      stat->vertex[i].flag = 0;
    }
  stat->width = 0.0F;
  stat->height = 0.0F;

  /* テクスチャパラメータ */
  stat->u = 0.0F;
  stat->v = 0.0F;

  stat->uw = 1.0F;
  stat->vh = 1.0F;

  return 0;
}

/*
 * テクスチャ指定
 */
static int stat_cmd_tex(char * cmd, tknFILE * tkn,
			lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;
  char * tex_name;
  char * lf;
  int code;

  if(NULL == (tex_name = cmdGetToken(tkn, TKN_QUOTED))) return cmdStatus;
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM)))
    {
      FREE(tex_name);
      return cmdStatus;
    }
  if(*lf != '\n') 
    {
      FREE(tex_name);
      FREE(lf);
      return cmdStatus;
    }
  FREE(lf);

  code = strcode(tex_name);
  FREE(tex_name);

  stat->tex = code;

  return 0;
}

/*
 * 座標値を取得する
 */
static int stat_get_vertex(tknFILE * tkn, statPoint * vertex)
{
  char *left, *right;
  char *X, *Y, *sep;

  left = right = X = Y = sep = NULL;
  if(NULL == (left = cmdGetToken(tkn, TKN_DELIM))) goto err;
  if(NULL == (X = cmdGetToken(tkn, TKN_TOKEN))) goto err; 
  if(NULL == (sep = cmdGetToken(tkn, TKN_DELIM))) goto err;
  if(NULL == (Y = cmdGetToken(tkn, TKN_TOKEN))) goto err;
  if(NULL == (right = cmdGetToken(tkn, TKN_DELIM))) goto err;

  if((*left != '(') || (*sep != ',') || (*right != ')')) goto err1;

  if(str_to_float(&vertex->x, X)) goto err;
  if(str_to_float(&vertex->y, Y)) goto err;
  vertex->flag = 1;
  DBG("\tvertex:(%f, %f)\n", vertex->x, vertex->y);


  FREE(left);
  FREE(X);
  FREE(sep);
  FREE(Y);
  FREE(right);
  return 0;
  
 err1:
  cmdStatus = CMD_ERR_SYNTAX;
 err:
  if(left != NULL) FREE(left);
  if(right != NULL) FREE(right);
  if(X != NULL) FREE(X);
  if(Y != NULL) FREE(Y);
  if(sep != NULL) FREE(sep);

  return cmdStatus;
}

/*
 * 頂点座標値指定
 */
static int stat_cmd_vertex(char * cmd, tknFILE * tkn,
			   lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;
  char * str_begin;
  char * lf;
  int begin;

  /* 設定開始頂点番号を得る */
  if(NULL == (str_begin = cmdGetToken(tkn, TKN_TOKEN))) goto err;
  if(str_to_int(&begin, str_begin)) goto err1;
  FREE(str_begin);

  DBG("begin = %d  vertex_num = %d\n", begin, stat->vertex_num);

  lf = NULL;
  do {
    if(lf != NULL) FREE(lf);
    if(begin >= stat->vertex_num) goto err;
    stat_get_vertex(tkn, &stat->vertex[begin]);
    begin++;
    if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM))) goto err;
  } while(*lf == '!');

  if(*lf != '\n') 
    {
      FREE(lf);
      goto err;
    }

  FREE(lf);
  return 0;
  
 err1:
  FREE(str_begin);
 err:
  return cmdStatus;
}

/*
 * 一組みの X,Y 方向データを、指定されたポインタの float 型領域に格納する
 */
static int stat_set_possizeuv(tknFILE * tkn, float * wxu, float * hyv)
{
  char * WXU;
  char * HYV;
  char * lf;


  if(NULL == (WXU = cmdGetToken(tkn, TKN_TOKEN))) goto err0;
  if(NULL == (HYV = cmdGetToken(tkn, TKN_TOKEN))) goto err1;
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM))) goto err2;
  if(*lf != '\n') goto err3;

  if(str_to_float(wxu, WXU)) goto err4;
  if(str_to_float(hyv, HYV)) goto err4;

  FREE(lf);
  FREE(WXU);
  FREE(HYV);

  return 0;

 err4:
  cmdStatus = CMD_ERR_SYNTAX;
 err3:
  FREE(lf);
 err2:
  FREE(HYV);
 err1:
  FREE(WXU);
 err0:
  return cmdStatus;

}


/*
 * 表示サイズ指定
 */
static int stat_cmd_size(char * cmd, tknFILE * tkn,
			 lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;

  return stat_set_possizeuv(tkn, &stat->width, &stat->height);
 }

/*
 * 正規化UV値指定
 */
static int stat_cmd_uv(char * cmd, tknFILE * tkn, lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;

  return stat_set_possizeuv(tkn, &stat->u, &stat->v);
}

/*
 * 正規化 UV サイズ指定
 */
static int stat_cmd_uvsize(char * cmd, tknFILE * tkn,
			   lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;

  return stat_set_possizeuv(tkn, &stat->uw, &stat->vh);
}

/*
 * テクスチャ使用禁止
 */
static int stat_cmd_texoff(char * cmd, tknFILE * tkn,
			   lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;
  stat->tex = -1;
  return 0;
}

static int stat_get_rgba(tknFILE * tkn, statRGBA * rgba)
{
  char *num[4], *sep[4];
  int RGBA[4];
  int i;

  for(i = 0; i < 4; i++) num[i] = sep[i] = NULL;
  /* とりあえず、一群のトークンを取得してみる */
  for(i = 0; i < 4; i++)
    {
      if(NULL == (num[i] = cmdGetToken(tkn, TKN_TOKEN))) goto err;
      if(NULL == (sep[i] = cmdGetToken(tkn, TKN_DELIM))) goto err;
    }

  for(i = 0; i < 3; i++) if(*sep[i] != ',') goto err_syntax;
  if(*sep[3] != '}') goto err_syntax;

  for(i = 0; i < 4; i++)
    if(str_to_int(&RGBA[i], num[i])) goto err_syntax;

  rgba->r = RGBA[0];  rgba->g = RGBA[1];
  rgba->b = RGBA[2];  rgba->a = RGBA[3];
  rgba->flag = 1;   /* 値が設定されていることを意味するフラグのセット */

  for(i = 0; i < 4; i++)
    {
      FREE(num[i]);
      FREE(sep[i]);
    }

  return 0;
  
 err_syntax:
  cmdStatus = CMD_ERR_SYNTAX;
 err:
  for(i = 0; i < 4; i++)
    {
      if(num[i] != NULL) FREE(num[i]);
      if(sep[i] != NULL) FREE(sep[i]);
    }
  return cmdStatus;
}

/*
 * 頂点 RGBA 値設定
 */
static int stat_cmd_color(char * cmd, tknFILE * tkn,
			  lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;
  char * str_begin;
  char * lf;
  int begin;    /* 開始頂点番号 */

  /* 設定を開始する頂点番号を得る */
  if(NULL == (str_begin = cmdGetToken(tkn, TKN_TOKEN))) goto err0;
  if(str_to_int(&begin, str_begin)) goto err1;

  /* 改行を検出するまでの間、RGBA 値を取得し、設定する */
  while(NULL != (lf = cmdGetToken(tkn, TKN_DELIM)))
    {
      if(*lf == '\n') break;
      if(*lf != '{') goto err1;
      if(begin >= stat->vertex_num) goto err2; /* 設定できる頂点の数を越えた */
      stat_get_rgba(tkn, &stat->rgba[begin]);
      begin++;
    }

  FREE(str_begin);
  return 0;

 err2:
  cmdStatus = CMD_ERR_SYNTAX;
 err1:
  FREE(str_begin);
 err0:
  return cmdStatus;
}

static int stat_cmd_alpha(char * cmd, tknFILE * tkn,
			  lblBlock * block, FILE * wfp)
{
  static char * col_tbl[] = {"Cs", "Cd", "0", NULL};
  static char * alp_tbl[] = {"As", "Ad", "FIX", NULL};
  statStatus * stat = statLink.now->d.st.stat_end;
  char * lf;
  char * p;
  int i, colA, colB, alpC, colD, FIX ;

  colA = colB = alpC = colD = 0;
  FIX = 128;

  /* A */
  if(NULL == (p = cmdGetToken(tkn, TKN_TOKEN))) return cmdStatus;
  for(i = 0; col_tbl[i] != NULL; i++)
    if(!strcmp(col_tbl[i], p)) { colA = i;  break; }
  FREE(p);
  if(col_tbl[i] == NULL) return (cmdStatus = CMD_ERR_SYNTAX);

  /* B */
  if(NULL == (p = cmdGetToken(tkn, TKN_TOKEN))) return cmdStatus;
  for(i = 0; col_tbl[i] != NULL; i++)
    if(!strcmp(col_tbl[i], p)) { colB = i;  break; }
  FREE(p);
  if(col_tbl[i] == NULL) return (cmdStatus = CMD_ERR_SYNTAX);

  /* C */
  if(NULL == (p = cmdGetToken(tkn, TKN_TOKEN))) return cmdStatus;
  for(i = 0; alp_tbl[i] != NULL; i++)
    if(!strcmp(alp_tbl[i], p)) { alpC = i;  break; }
  FREE(p);
  if(alp_tbl[i] == NULL) return (cmdStatus = CMD_ERR_SYNTAX);

  /* D */
  if(NULL == (p = cmdGetToken(tkn, TKN_TOKEN))) return cmdStatus;
  for(i = 0; col_tbl[i] != NULL; i++)
    if(!strcmp(col_tbl[i], p)) { colD = i;  break; }
  FREE(p);
  if(col_tbl[i] == NULL) return (cmdStatus = CMD_ERR_SYNTAX);

  /* FIX */
  if(alpC == 2)
    {
      if(NULL == (p = cmdGetToken(tkn, TKN_TOKEN))) return cmdStatus;
      if(str_to_int(&FIX, p))
	{
	  FREE(p);
	  return cmdStatus;
	}
      FREE(p);
    }
  
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM))) return cmdStatus;
  if(*lf != '\n')
    {
      FREE(lf);
      return (cmdStatus = CMD_ERR_SYNTAX);
    }
  FREE(lf);
  stat->alpha_flg = 1;
  stat->alpha =
    ((FIX & 0xff) << 16) | (colD << 6) | (alpC << 4) | (colB << 2) | colA;
  
  return 0;  


}
/*
 * 非表示
 */
static int stat_cmd_hide(char * cmd, tknFILE * tkn,
			 lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;
  char * lf;
  
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM))) return cmdStatus;
  if(*lf != '\n')
    {
      FREE(lf);
      return (cmdStatus = CMD_ERR_SYNTAX);
    }
  FREE(lf);
  stat->disp = 0;
  return 0;  
}

/*
 * 表示
 */
static int stat_cmd_show(char * cmd, tknFILE * tkn,
			 lblBlock * block, FILE * wfp)
{
  statStatus * stat = statLink.now->d.st.stat_end;
  char * lf;
  
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM))) return cmdStatus;
  if(*lf != '\n')
    {
      FREE(lf);
      return (cmdStatus = CMD_ERR_SYNTAX);
    }
  
  stat->disp = 1;
  return 0;  
}

/*
 * 状態定義の終端チェック
 */
static int stat_term_check(char * token)
{
  if(!strcmp(token, "endmode")) return 1;
  if(!strcmp(token, "endstat"))
    {
      cmdStatus = CMD_ERR_SYNTAX;
      return 1;
    }
  return 0;
}

/*
 * ステータス定義命令
 */
int statDefStatus(char * cmd, tknFILE * tkn, lblBlock * block, FILE * wfp)
{
  statBlock * stblk;
  statStatus * stat;
  int    id_obj, ret;
  char * label;
  char * lf;
  char * pre;
  char * stat_name;  /* 状態名称 */
  static cmdIndex stat_list[] = {
    { "tex",    stat_cmd_tex },
    { "vertex", stat_cmd_vertex },
    { "size",   stat_cmd_size },
    { "uv",     stat_cmd_uv },
    { "uvsize", stat_cmd_uvsize },
    { "texoff", stat_cmd_texoff },
    { "color",  stat_cmd_color },
    { "alpha",  stat_cmd_alpha },
    { "hide",   stat_cmd_hide },
    { "show",   stat_cmd_show },
    { NULL, NULL }   /* 終端 */
  };

  if(NULL == (label = cmdGetToken(tkn, TKN_QUOTED))) return cmdStatus;
  if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM)))
    {
      FREE(label);
      return cmdStatus;
    }
  if(*lf != '\n')   /* 改行でなければエラー */
    {
      FREE(label);
      FREE(lf);
      return cmdStatus;
    }
  FREE(lf);

  /* 取得した文字列をラベルとして持つオブジェクトを操作対象とする */
  if(0 > (id_obj = lblRefLabel(cmdLabel, label)))
    {
      FREE(label);
      return (cmdStatus = CMD_ERR_SYNTAX);
    }

  FREE(label);

  /* ブロックを作成 */
  if(NULL == (stblk = cmdMalloc(sizeof(statBlock)))) return cmdStatus;

  stblk->mode = MODE_STATUS;
  stblk->d.st.id_obj = id_obj;
  stblk->d.st.stat_begin = stblk->d.st.stat_end = NULL;

  /* 現在の操作対象として登録 */
  stat_regist_block(stblk);
  statLink.now = stblk;

  ret = cmdStatus = 0;
  while(NULL != (pre = cmdGetToken(tkn, TKN_TOKEN))) 
    {
      if(!strcmp(pre, "endstat")) break;

      if(strcmp(pre, "mode"))
	{
	  ret = CMD_ERR_SYNTAX; 
	  break;
	}

      if(NULL == (stat_name = cmdGetToken(tkn, TKN_QUOTED))) 
	{
	  ret = cmdStatus;
	  break;
	}

      if(NULL == (stat = cmdMalloc(sizeof(statStatus))))
	{
	  FREE(stat_name);
	  ret = cmdStatus;
	  break;
	}

      /* 状態を初期化する */
      stat_init_status(stat, stat_name, id_obj);
      /* 状態を登録する */
      stat_regist_status(stblk, stat);
      /* この命令以下で使用できる命令の処理 */
      ret = cmdConvert(tkn, stat_list, stat_term_check);
      if(NULL == (lf = cmdGetToken(tkn, TKN_DELIM)))
	ret = cmdStatus;
      else
	if(*lf != '\n') ret = cmdStatus;
      FREE(lf);
      FREE(pre);
    }
  if(pre != NULL) FREE(pre);
  else ret = cmdStatus;

  return ret;
}
