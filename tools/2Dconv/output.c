#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _output_c_
#include "debug.h"
#include "my_malloc.h"
#include "util.h"
#include "token.h"
#include "label.h"
#include "command.h"
#include "objdef.h"
#include "objstat.h"
#include "objact.h"

#include "output.h"


int outInitOutputModule(void)
{

  return 0;
}


static int out_write_object_define(FILE *wfp, defObject * obj)
{
  unsigned char buf[16];
  int i;

  /* 一旦配列を初期化 */
  for(i = 0; i < 16; i++) buf[i] = 0;

  /* オブジェクト名の StrCode を格納 */
  buf[0] = obj->code & 0xff;
  buf[1] = (obj->code >> 8) & 0xff;
  buf[2] = (obj->code >> 16) & 0xff;
  buf[3] = (obj->code >> 24) & 0xff;
  
  /* インデックス番号 */
  buf[4] = obj->id_label & 0xff;
  buf[5] = (obj->id_label >> 8) & 0xff;

  /* 親オブジェクトのインデックス番号 */
  buf[6] = obj->id_parent & 0xff;
  buf[7] = (obj->id_parent >> 8) & 0xff;

  /* 属性 */
  buf[8] = obj->id_attrib;

  /* 頂点数 */
  buf[9] = obj->id_vertex;   /* 属性が lstrip 以外の場合は意味を持たない。 */

  /* 以降は未定義領域 */


  if(16 != fwrite(buf, 1, 16, wfp))   /* 定義ブロックを書き込む */
    return -1;

  return 0;
}

/*
 * オブジェクト定義セクションの出力
 */
int outOutputDefineSection(FILE * wfp)
{
  defObject * obj;
  int cnt = 0;

  /* 登録されているオブジェクトの数をカウントする */
  for(obj = defLink.begin; obj != NULL; obj = obj->next) cnt++;

  /* オブジェクト数を書き込む */
  if(EOF == fputl((long)cnt, wfp)) return -1;

  /* オブジェクトを定義ブロックとして書き込む */
  for(obj = defLink.begin; obj != NULL; obj = obj->next)
    if(out_write_object_define(wfp, obj)) return -1;

  return 0;
}

static unsigned long out_float_to_seq(float x, float y)
{
  unsigned short X, Y;
  unsigned long ans;

  X = short_float(x);
  Y = short_float(y);

  ans = ((unsigned long)Y << 16) | (unsigned long)X;
  return ans;
}

static unsigned long out_rgba_to_seq(statRGBA * rgba)
{
  unsigned long ret;

  ret  =  rgba->r & 0xff;
  ret |= (rgba->g & 0xff) << 8;
  ret |= (rgba->b & 0xff) << 16;
  ret |= (rgba->a & 0xff) << 24;

  return ret;
}

/*
 * 頂点座標値を設定するシーケンスを書き込む
 */
static int out_write_vertex_list(FILE *wfp, statStatus * stat)
{
  int begin, cnt, i;
  unsigned long cmd;
  unsigned long pos;

  begin = 0;
  DBG("out_write_vertex_list(): vertex num: %d\n", stat->vertex_num);
  while(begin < stat->vertex_num)
    {
      if(!stat->vertex[begin].flag)
	{
	  begin++;
	  continue;
	}
      DBG("--------------\n");
      cnt = 0;
      while(((begin + cnt) < stat->vertex_num) &&
	    (stat->vertex[begin + cnt].flag)) cnt++;
      cmd = SEQ_STAT_VERTEX | ((cnt & 0xff) << 8) | (begin & 0xff);
      DBG("cmd = %08lx\n", cmd);
      fputl(cmd, wfp);
      
      for(i = 0; i < cnt; i++)
	{
	  pos = out_float_to_seq(stat->vertex[begin + i].x,
				 stat->vertex[begin + i].y);
	  fputl(pos, wfp);
	}
      begin += cnt;
    }
  return 0;  
}

/*
 * 頂点 RGBA 値を設定するシーケンスを書き込む
 */
static int out_write_rgba_list(FILE *wfp, statStatus * stat)
{
  int begin, cnt, i;
  unsigned long cmd;
  unsigned long rgba;

  begin = 0;
  while(begin < stat->vertex_num)
    {
      if(!stat->rgba[begin].flag)
	{
	  begin++;
	  continue;
	}
      cnt = 0;
      while(((begin + cnt) < stat->vertex_num) &&
	    (stat->rgba[begin + cnt].flag)) cnt++;
      cmd = SEQ_STAT_RGBA | ((cnt & 0xff) << 8) | (begin & 0xff);
      fputl(cmd, wfp);
      
      for(i = 0; i < cnt; i++)
	{
	  rgba = out_rgba_to_seq(&stat->rgba[begin + i]);
	  fputl(rgba, wfp);
	}
      begin += cnt;
    }
  return 0;  
}

static int out_write_status(FILE * wfp, statStatus * stat, int obj_id)
{
  unsigned long code;
  unsigned long xy_uv_wh;
  unsigned long disp_cmd;

  code = strcode(stat->stat_name);
  fputl(code, wfp);

  /* ステータスを確認し、設定されているものについてのみ
     シーケンスとしてまとめる */

  /* 表示 / 非表示 */
  disp_cmd = SEQ_STAT_DISP | (stat->disp & 1);
  fputl(disp_cmd, wfp);

  /* アルファブレンディング設定 */
  if(stat->alpha_flg) fputl(stat->alpha | SEQ_STAT_ALPHA, wfp);

  if(stat->tex >= 0)
    {
      /* 使用テクスチャ */
      fputl(SEQ_STAT_TEX | stat->tex, wfp);

      /* UV 値 */
      if((stat->u >= 0.0F) && (stat->v >= 0.0F))
	{
	  xy_uv_wh = out_float_to_seq(stat->u, stat->v);
	  fputl(SEQ_STAT_UV, wfp);
	  fputl(xy_uv_wh, wfp);
	}

      /* テクスチャサイズ */
      if((stat->uw >= 0.0F) && (stat->vh >= 0.0F))
	{
	  xy_uv_wh = out_float_to_seq(stat->uw, stat->vh);
	  fputl(SEQ_STAT_TXSIZ, wfp);
	  fputl(xy_uv_wh, wfp);
	}
    }
  
  /* 表示サイズ */
  if((stat->width >= 0.0F) && (stat->height >= 0.0F))
    {
      xy_uv_wh = out_float_to_seq(stat->width, stat->height);
      fputl(SEQ_STAT_SIZE, wfp);
      fputl(xy_uv_wh, wfp);
    }

  out_write_vertex_list(wfp, stat);  /* 頂点座標値リスト */
  out_write_rgba_list(wfp, stat);    /* 頂点色リスト     */

  fputl(SEQ_STAT_END, wfp);          /* 設定シーケンス終了 */
  
  return 0;
}

static int out_write_status_block(FILE * wfp, statBlock * block)
{
  unsigned long code;

  switch(block->mode)
    {
    case MODE_REFTRI:
      fputl(0xffffffffL, wfp);
      code = block->d.tri.tri_code;
      code |= 0x80000000;
      fputl(code, wfp);
      break;

    case MODE_STATUS:
      {
	statStatus * stat;
	int cnt;

	fputw(block->d.st.id_obj, wfp);
	cnt = 0;
	for(stat = block->d.st.stat_begin; stat != NULL; stat = stat->next)
	  cnt++;
	fputw(cnt, wfp);
	for(stat = block->d.st.stat_begin; stat != NULL; stat = stat->next)
	  out_write_status(wfp, stat, block->d.st.id_obj);
      }
      break;
    }
  return 0;
}


/*
 * 状態定義セクションの出力
 */
int outOutputStatusSection(FILE * wfp)
{
  statBlock * block;
  int stat_cnt;

  /* ブロック情報の数を数える */
  stat_cnt = 0;
  for(block = statLink.begin; block != NULL; block = block->next) stat_cnt++;

  fputl(stat_cnt, wfp);   /* ブロック数の書き込み */
  
  for(block = statLink.begin; block != NULL; block = block->next)
    out_write_status_block(wfp, block);

  return 0;
}

/*
 * トラックデータの出力
 */
static int out_write_action_track(FILE * wfp, actTrack * track)
{
  actStep * step;
  int id_obj;
  int code;
  int cnt;

  id_obj = lblRefLabel(cmdLabel, track->trk_name);
  fputw(id_obj, wfp); /* 対象オブジェクト     */

  /* トラックデータのステップ数(終端含む)をカウント */
  cnt = 0;
  for(step = track->begin; step != NULL; step = step->next) cnt++;

  /* トラックデータのステップ数を書き込む。
   * サイズではなくステップ数であることに注意 */
  fputw(cnt,    wfp);

  for(step = track->begin; step != NULL; step = step->next)
    {
      switch(step->mode)
	{
	case STEP_WAIT:
	  fputl(SEQ_ACT_WAIT | (step->st_time & 0xffff), wfp);
	  break;
	case STEP_SET:
	  code = strcode(step->st_name);
	  fputl(SEQ_ACT_SET  | (code & 0xffffff), wfp);
	  break;
	case STEP_MORF:
	  code = strcode(step->st_name);
	  fputl(SEQ_ACT_MORF | (step->st_time & 0xffff), wfp);
	  fputl(code & 0xffffff, wfp);
	  break;
	}
    }
  fputl(SEQ_ACT_END, wfp);  /* トラック終了 */
  return 0;
}

/*
 * アクションデータの出力
 */
static int out_write_action(FILE *wfp, actAction * act)
{
  int code;
  int track_cnt;
  actTrack * track;

  /* アクション名の StrCode を求める */
  code = strcode(act->act_label);

  /* トラックの数を求める */
  track_cnt = 0;
  for(track = act->begin; track != NULL; track = track->next) track_cnt++;

  fputl(code, wfp);      /* アクション名書き込み */
  fputl(track_cnt, wfp); /* トラック数書き込み   */

  /* 各トラックデータの出力 */
  for(track = act->begin; track != NULL; track = track->next)
    out_write_action_track(wfp, track);

  return 0;
} 



/*
 * アクション定義セクションの出力
 */
int outOutputActionSection(FILE * wfp)
{
  actAction * act;
  unsigned long act_cnt;

  act_cnt = 0;
  for(act = actLink.begin; act != NULL; act = act->next) act_cnt++;

  fputl(act_cnt, wfp);   /* アクションの数を記録 */
  /* アクションデータを一つ一つ書き込む */
  for(act = actLink.begin; act != NULL; act = act->next)
    out_write_action(wfp, act);

  return 0;
}

/*
 * 出力
 */
int outOutputMain(FILE * wfp)
{
  unsigned long pos[2];

  fputl(VERSION, wfp);     /* 現状のバージョン                       */
  fputl(0x00000010, wfp);  /* オブジェクト定義セクションアドレス     */
  fputl(0, wfp);           /* 状態定義セクションアドレスエリア       */
  fputl(0, wfp);           /* アクション定義セクションアドレスエリア */

  outOutputDefineSection(wfp);

  pos[0] = ftell(wfp);
  outOutputStatusSection(wfp);

  pos[1] = ftell(wfp);
  outOutputActionSection(wfp);

  fseek(wfp, 8L, SEEK_SET);
  fputl(pos[0], wfp);
  fputl(pos[1], wfp);

  return 0;
}
