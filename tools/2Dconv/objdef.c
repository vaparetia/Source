#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _objdef_c_
#include "debug.h"
#include "my_malloc.h"
#include "util.h"
#include "token.h"
#include "label.h"

#include "command.h"
#include "objdef.h"

#define DEF_TKNBUF  256

static char tkn_buf[ DEF_TKNBUF ];
static const char * def_attrib_list[] = {
  "empty",  "point",  "line",
  "lstrip", "box",    "sprite",
  "zoom", NULL
};

static const int def_vertex_num[] = {
  1,  /* empty  */
  1,  /* point  */
  2,  /* line   */
  0,  /* lstrip */
  2,  /* box    */
  1,  /* sprite */
  2   /* zoom   */
};

int defInitObjectDefine(void)
{
  defObject *obj, *nxt;

  /* 登録済のオブジェクトがある場合、すべて開放する */
  if(NULL != (obj = defLink.begin))
    while(obj != NULL)
      {
	nxt = obj->next;
	FREE(obj);
	obj = nxt;
      }
  defLink.begin = defLink.end = NULL;
  return 0;
}

static int def_regist_object(int code,
			     int id_label, int id_attrib,
			     int id_vertex, int id_parent)
{
  defObject * obj;
  
  /* LineStrip 以外で、頂点数が指定されている場合はエラー */
  if(strcmp(def_attrib_list[id_attrib], "lstrip") && id_vertex >= 0)
    {
      cmdStatus = CMD_ERR_SYNTAX;
      return -1;
    }
  if(NULL == (obj = cmdMalloc(sizeof(defObject)))) return -1;

  if(defLink.end != NULL) defLink.end->next = obj;
  obj->prev = defLink.end;
  obj->next = NULL;
  defLink.end = obj;
  if(NULL == defLink.begin) defLink.begin = obj;

  obj->code      = code;
  obj->id_label  = id_label;
  obj->id_attrib = id_attrib;
  obj->id_vertex = id_vertex;
  obj->id_parent = id_parent;

  return 0;
}


static int def_get_attrib(char * atlabel)
{
  int i;

  for(i = 0; def_attrib_list[i] != NULL; i++)
    if(!strcmp(def_attrib_list[i], atlabel)) return i;

  return -1;   /* 指定された文字列が、オブジェクト属性として不正である場合 */
}


/*
 * オブジェクト定義命令
 */
int defObjDefine(char * cmd, tknFILE * tkn, lblBlock * block, FILE *wfp)
{
  char * obj_label;    /* オブジェクトラベル     */
  char * obj_attrib;   /* オブジェクト属性       */

  char * obj_parent;   /* 親オブジェクトのラベル */
  char * obj_vertex;   /* LineStrip の頂点数     */

  int code;
  int id_label;
  int id_attrib;
  int id_parent;
  int id_vertex;

  tknStat stat;

  /* コマンド名の確認 */
  if(strcmp(cmd, "obj")) return CMD_ERR_SYNTAX;
  
  DBG("entering defObjDefine()\n");

  /* オブジェクト名取得 */
  if(NULL == (obj_label = cmdGetToken(tkn, TKN_QUOTED))) goto err0;
  DBG("obj_label = [%s]\n", obj_label);
  code = strcode(obj_label);  /* オブジェクトの StrCode を計算 */
  DBG("StrCode = %d\n", code);

  /* オブジェクト属性取得 */
  if(NULL == (obj_attrib = cmdGetToken(tkn, TKN_TOKEN))) goto err1;


  DBG("obj %s %s", obj_label, obj_attrib);


  /*
   * ここから先は、4つのパターンがある。
   *
   * (1) 頂点数指定も、親オブジェクト指定も無い場合
   * (3) vertex <NUM>
   * (3) parent "parent label"
   * (4) vertex <NUM> parent "parent label"
   */
  obj_parent = obj_vertex = NULL;   /* 初期化 */
  while(NULL != tknGetToken(tkn, tkn_buf, DEF_TKNBUF))
    {
      stat = tknGetStat(tkn);
      if((stat == TKN_DELIM) && !strcmp(tkn_buf, "\n")) break;
      if(stat != TKN_TOKEN) goto err3;

      if(!strcmp(tkn_buf, "vertex") && (obj_vertex == NULL))
	{
	  if(NULL == (obj_vertex = cmdGetToken(tkn, TKN_TOKEN))) goto err3;
	  DBG(" vertex %s", obj_vertex);
	  continue;
	}
      if(!strcmp(tkn_buf, "parent"))
	{
	  if(NULL == (obj_parent = cmdGetToken(tkn, TKN_QUOTED))) goto err3;
	  DBG(" parent %s", obj_parent);
	  continue;
	}
      else goto err3;
    }

  DBG("\n");

  /*
   * obj 命令のパラメータ取得終了。解析段階。
   */
  /* 頂点数が整数として正しい表記であるか確認のうえ、その値を頂点数とする。*/
  if(NULL == obj_vertex)
    id_vertex = -1;   /* 頂点数指定が無い場合 */
  else
    if(str_to_int(&id_vertex, obj_vertex)) goto err3;

  DBG("id_vertex = %d\n", id_vertex);

  /* 親オブジェクトとして指定されているオブジェクトが、
     既に登録されているかどうか確認する */
  if(NULL == obj_parent)
    id_parent = -1;   /* 親オブジェクト指定が無い場合 */
  else
    if(LBLERR_UNDEFINED == (id_parent = lblRefLabel(cmdLabel, obj_parent)))
      goto err3;

  DBG("id_parent = %d\n", id_parent);

  /* オブジェクト属性が、許可されている属性であるかどうかを確認する */
  if(0 > (id_attrib = def_get_attrib(obj_attrib))) goto err3;

  /* オブジェクトを指定された名称で登録できるかどうかを確認する */
  if(0 > (id_label = lblDefineLabel(cmdLabel, obj_label)))
    {
      switch(id_label)
	{
	case LBLERR_NO_MEMORY:  /* メモリが足りなくて登録できない */
	  cmdStatus = CMD_ERR_NO_MEMORY;
	  break;
	case LBLERR_REGISTED:   /* 既に同じ文字列のラベルが登録済 */
	  cmdStatus = CMD_ERR_DUPLICATED;
	  break;
	}
      goto err3;
    }
  DBG("id_label = %d\n", id_label);

  /* 使用オブジェクトとして登録する */
  def_regist_object(code, id_label, id_attrib, id_vertex, id_parent);

  DBG("finish!\n");
  return 0;
  
 err3:
  if(NULL != obj_vertex) FREE(obj_vertex);
  if(NULL != obj_parent) FREE(obj_parent);
  FREE(obj_attrib);
 err1:
  FREE(obj_label);
 err0:
  ABORT(EXIT_FAILURE, "error in defObjDefine()\n");
  return cmdStatus;
}

int defGetObjVertexNum(int id_obj)
{
  defObject * obj;
  int num;

  num = -1;
  for(obj = defLink.begin; obj != NULL; obj = obj->next)
    if(id_obj == obj->id_label)
      {
	num = def_vertex_num[obj->id_attrib];
	if(!num) num = obj->id_vertex;
	break;
      }

  return num;
}
