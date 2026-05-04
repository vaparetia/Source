#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _mgs2_c_
#include "2d_data.h"
#include "futil.h"
#include "mgs2.h"

/* 指数のバイアス値（2^0となる指数値） */
#define E_BIAS	(8)		/* メタルギアでは大きい値を重視する */

/*
 * float 型を、16bit 浮動小数点数値に変換する
 */
unsigned short short_float(float f)
{
  unsigned int	data, s_dat, e_dat, f_dat ;
  unsigned short	mf ;
  
  data = *(unsigned int*)&f ;
  /* 符合取りだし */
  s_dat = ( data & 0x80000000 ) >> 16 ;
  /* 指数取りだし */
  e_dat = ( data & 0x7f800000 ) ;
  if ( e_dat != 0 ){
    if ( e_dat <= ( 127 - E_BIAS ) << 23 ){
      //printf("convert error!!\n");
      return ( 0x0000 | s_dat );
    }
    if ( e_dat > ( 127 - E_BIAS + 31 ) << 23 ){
      //printf("convert error!!\n");
      return ( 0x7fff | s_dat );
    }
    e_dat -= ( 127 - E_BIAS ) << 23 ;
    e_dat >>= 13 ;
  }
  /* 仮数部取りだし */
  f_dat = ( data & 0x007fffff ) >> 13 ;
  
  /* ビット合成 */
  mf = s_dat | e_dat | f_dat ;
  
  return ( mf );
}

/*
 * 頂点 RGBA 値書き出し
 */
static int write_rgba(int max_rgba, sprStatus * stat, FILE * wfp)
{
  int start, len, i, j;
  unsigned long l;

  start = -1;
  for(i = 0; i < max_rgba; i++)
    {
      if(start < 0)
	{
	  if(stat->vertex[i].col) start = i, len = 1;
	  continue;
	}
      if(stat->vertex[i].col)
	{
	  len++;
	  continue;
	}
      l = (len << 8) | start;
      fputl(SEQ_STAT_RGBA | l, wfp);
      for(j = 0; j < len; j++)
	{
	  fputc(stat->vertex[start + j].r, wfp);
	  fputc(stat->vertex[start + j].g, wfp);
	  fputc(stat->vertex[start + j].b, wfp);
	  fputc(stat->vertex[start + j].a, wfp);
	}
      start = -1;
    }

  if(start < 0) return 0;

  l = (len << 8) | start;
  fputl(SEQ_STAT_RGBA | l, wfp);
  for(j = 0; j < len; j++)
    {
      fputc(stat->vertex[start + j].r, wfp);
      fputc(stat->vertex[start + j].g, wfp);
      fputc(stat->vertex[start + j].b, wfp);
      fputc(stat->vertex[start + j].a, wfp);
    }
  return 0;
}

/*
 * 頂点座標値書き出し
 */
static int write_vertex(int max_vertex, sprStatus * stat, FILE * wfp)
{
  int start, len, i, j;
  unsigned long l;

  start = -1;
  for(i = 0; i < max_vertex; i++)
    {
      if(start < 0)
	{
	  if(stat->vertex[i].pos) start = i, len = 1;
	  continue;
	}
      if(stat->vertex[i].pos)
	{
	  len++;
	  continue;
	}
      l = (len << 8) | start;
      fputl(SEQ_STAT_VERTEX | l, wfp);
      for(j = 0; j < len; j++)
	{
	  l = POS_PACK(stat->vertex[start + j].x, stat->vertex[start + j].y);
	  fputl(l, wfp);
	}
      start = -1;
    }

  if(start < 0) return 0;

  l = (len << 8) | start;
  fputl(SEQ_STAT_VERTEX | l, wfp);
  for(j = 0; j < len; j++)
    {
      l = POS_PACK(stat->vertex[start + j].x, stat->vertex[start + j].y);
      fputl(l, wfp);
    }
  return 0;
}

/*
 * オブジェクト定義部の出力 
 */
static unsigned long write_objdef(sprLayout * layout, FILE *wfp)
{
  unsigned long l;
  sprObject * obj;
  
  /* オブジェクトの数のカウント */
  l = 0;
  for(obj = layout->objBegin; obj != NULL; obj = obj->next) l++;
  
  fputl(l, wfp);   /* オブジェクト数 */
  
  for(obj = layout->objBegin; obj != NULL; obj = obj->next)
    {
      fputl(obj->code, wfp);              /* オブジェクト名 StrCode */
      fputw(obj->serial_id, wfp);         /* オブジェクト番号       */
      
      /* 親オブジェクトID */
      if(obj->parent != NULL)
	fputw(obj->parent->serial_id, wfp);
      else
	fputw(0xffff, wfp);
      
      fputc((int)obj->obj_id, wfp);       /* オブジェクト属性       */
      fputc(obj->vertex_num, wfp);        /* 頂点数                 */
      fput_zero_fill(6, wfp);             /* 詰め物                 */
    }
  return ftell(wfp);  /* 書き込みが終った直後のアドレス */
}


/*
 * 状態定義セクション書き込み
 */
static unsigned long write_status(sprLayout * layout, FILE *wfp)
{
  unsigned long l;
  sprStatBlock * block;
  sprStatus * stat;
  
  l = 0;
  for(block = layout->blockBegin; block != NULL; block = block->next) l++;
  fputl(l, wfp);   /* 状態定義ブロックの数 */
  
  for(block = layout->blockBegin; block != NULL; block = block->next)
    if(block->mode == MODE_REFTRI)
      {
	fputw(0xffff, wfp);
	fputw(0xffff, wfp);
	fputl(block->d.tri.code, wfp);
      }
    else
      {
	/* 状態設定対象オブジェクト ID */
	fputw(block->d.st.target->serial_id, wfp);
	l = 0;
	for(stat = block->d.st.begin; stat != NULL; stat = stat->next) l++;
	fputw(l, wfp);  /* ブロック内の状態数 */
	
	/* ブロックに含まれる、各状態を所定のプロトコルに従って書き込む */
	for(stat = block->d.st.begin; stat != NULL; stat = stat->next)
	  {
	    /* 状態名の StrCode を書き込む */
	    fputl(stat->code, wfp);
	    
	    /* 表示 / 非表示 */
	    fputl(SEQ_STAT_DISP | ((int)stat->disp & 1), wfp);
	   
	    /* プライオリティ補正値 */
	    if(stat->pri > 0)
	      fputl(SEQ_STAT_PRI | (stat->pri & 7), wfp);
	    
	    /* アルファブレンディング設定 */
	    if(stat->alpha >= 0)
	      {
		l = (stat->v_alpha << 16) | stat->alpha;
		fputl(SEQ_STAT_ALPHA | l, wfp);
	      }
	    
	    /* 子オブジェクトの座標値係数 */
	    if(stat->magni != 1.0F)
	      {
		l = SEQ_STAT_MAGNI | short_float(stat->magni);
		fputl(l, wfp);
	      }

	    /* テクスチャ関係 */
	    if(stat->tex_sw)
	      {
		/* テクスチャ名称 */
		if(stat->tex_name != NULL)
		  {
		    l = SEQ_STAT_TEX | stat->tex_code;
		    fputl(l, wfp);
		  }
		
		/* UV値 */
		fputl(SEQ_STAT_UV, wfp);
		l  = POS_PACK(stat->u, stat->v);
		fputl(l, wfp);
		
		/* 正規化テクセルサイズ */
		fputl(SEQ_STAT_TXSIZ, wfp);
		l = POS_PACK(stat->uw, stat->vh);
		fputl(l, wfp);
	      }
	    
	    /* スプライトなら、サイズを記録する */
	    if(block->d.st.target->obj_id == SP_SPRITE ||
	       block->d.st.target->obj_id == SP_SPIN)
	      {
		fputl(SEQ_STAT_SIZE, wfp);
		l = POS_PACK(stat->width, stat->height);
		fputl(l, wfp);
	      }

	    /* 回転オブジェクトならば、中心点と回転角を記録 */
	    if(block->d.st.target->obj_id == SP_SPIN)
	      {
		/* 回転中心点 */
		fputl(SEQ_STAT_CENTER, wfp);
		l = POS_PACK(stat->center_x, stat->center_y);
		fputl(l, wfp);

		/* 回転角設定 */
		l = SEQ_STAT_ANGLE |
		  (unsigned long)short_float(stat->angle);
		fputl(l ,wfp);
	      }


	    /* 頂点 RGBA 値 */
	    write_rgba(block->d.st.target->rgba_num, stat, wfp);
	    
	    /* 頂点座標 */
	    write_vertex(block->d.st.target->vertex_num, stat, wfp);
	    
	    /* 設定シーケンス終了 */
	    fputl(SEQ_STAT_END, wfp);
	  }
      }
  return ftell(wfp);   /* 書き込みが終った直後のアドレス */
}

/*
 * アクション定義部の書き込み
 */
static unsigned long write_action(sprLayout * layout, FILE *wfp)
{
  unsigned long l;
  sprAction * act;
  sprTrack * track;
  int i;

  /* アクションの数をカウント */
  l = 0;
  for(act = layout->actBegin; act != NULL; act = act->next) l++;
  fputl(l, wfp);

  /* アクションデータの書き込み */
  for(act = layout->actBegin; act != NULL; act = act->next)
    {
      fputl(act->code, wfp);       /* アクション名 StrCode */
      fputl(act->track_nums, wfp); /* アクションに含まれるトラックの本数 */

      /* トラックデータの書き込み */
      for(track = act->begin; track != NULL; track = track->next)
	{
	  fputw(track->target->serial_id, wfp);
	  fputw(track->seq_step, wfp);

	  /* トラックシーケンスの書き込み */
	  for(i = 0; i < track->seq_step; i++)
	    switch(track->seq[i].op)
	      {
	      case TR_WAIT:  /* 時間待ち     */
		fputl(SEQ_ACT_WAIT | track->seq[i].v_time, wfp);
		break;
	      case TR_SET:   /* 状態強制移行 */
		fputl(SEQ_ACT_SET | track->seq[i].v_stat->code, wfp);
		break;
	      case TR_MORF:  /* 状態補間     */
		l = SEQ_ACT_MORF | track->seq[i].v_time |
		  ((track->seq[i].spin & 1) << 16);
		fputl(l, wfp);
		fputl(track->seq[i].v_stat->code, wfp);
		break;
	      case TR_END:   /* 終了         */
		fputl(SEQ_ACT_END, wfp);
		break;
	      }
	}
    }
  
  return ftell(wfp);
}

int mgs2_Output2D(sprLayout * layout, FILE * wfp)
{
  unsigned long l;
  unsigned long stat, act, end;
  fputl(VERSION,    wfp);  /* バージョン値                   */
  fputl(0x00000010, wfp);  /* オブジェクト定義セクション開始 */
  fputl(0x00000000, wfp);  /* 状態定義セクション先頭(ダミー) */
  fputl(0x00000000, wfp);  /* アクション定義セクション先頭   */

  stat = write_objdef(layout, wfp);
  act = write_status(layout, wfp);
  end = write_action(layout, wfp);

  fseek(wfp, 8L, SEEK_SET);
  /* 各セクションの開始アドレスをヘッダに書き込む */
  fputl(stat, wfp);
  fputl(act,  wfp);

  return 0;
}


static char * dir_format(char * dir)
{
  size_t len, slash;
  char * tmp;

  if(NULL == dir)
    {
      if(NULL != (tmp = MALLOC(1)))
	*tmp = 0;
      return tmp;
    }

  len = strlen(dir);
  slash = 0;
  if(len > 0) if(dir[len - 1] != '/') slash = 1;
  if(NULL == (tmp = MALLOC(len + slash + 1))) return NULL;
  strcpy(tmp, dir);
  if(slash) strcat(tmp, "/");
  return tmp;
}


static int out_tri(triInfo * info, char * sdir, char * odir)
{
  triTex * tex;
  int i;
  static char * cmd_cm2[] = { "makecm2", " -out ", "%s", "l2d", " -in", NULL };
  static char * cmd_tri[] = { "make_tri", " -s -o ", "%o",
			      "%t", " -i ", "%o", "l2d*.cm2", NULL };
  char * cm2_line;
  char * tri_line;
  size_t len_cm2;
  size_t len_tri;
  size_t len_sdir = strlen(sdir);
  size_t len_odir = strlen(odir);
  char * add;

  /*
   * cm2 のコマンド列を作る
   */
  /* コマンド基本部分の長さ */
  len_cm2 = len_sdir + 1;/* 途中の出力先ディレクトリの長さと終端の1文字分 */
  for(i = 0; cmd_cm2[i] != NULL; i++)
    {
      if(*cmd_cm2[i] == '%') continue; 
      len_cm2 += strlen(cmd_cm2[i]);
    }
  /* 元画像ファイル分の長さ */
  for(tex = info->tex; NULL != tex; tex = tex->next)
    /* ファイル名と空白とワイルドカード分 */
    len_cm2 += len_sdir + strlen(tex->tex_name) + 3;

  /* 出来上がった長さ分メモリを確保 */
  if(NULL == (cm2_line = MALLOC(len_cm2))) return -1;

  cm2_line[0] = 0;
  /* 確保したメモリにコマンドラインを作る */
  for(i = 0; cmd_cm2[i] != NULL; i++)
    {
      if(!strcmp(cmd_cm2[i], "%s"))
	{
	  strcat(cm2_line, odir);
	  continue;
	}
      strcat(cm2_line, cmd_cm2[i]);
    }
  for(tex = info->tex; NULL != tex; tex = tex->next)
    {
      strcat(cm2_line, " ");
      strcat(cm2_line, sdir);
      strcat(cm2_line, tex->tex_name);
      strcat(cm2_line, ".*");
    }
  
  /* makecm2 を実行 */
  {
    int ret;

    printf("%s\n", cm2_line);
    ret = system(cm2_line);
    FREE(cm2_line);
    printf("makecm2 status = %d\n", ret);
    if(ret != 0)  /* 終了値がエラーの場合 */
      {
	printf("texture source image not found.\n");
	for(tex = info->tex; NULL != tex; tex = tex->next)
	  printf("\t%s%s.(bmp|pic)\n", sdir, tex->tex_name);
	exit(ret);   /* エラーコードを吐いて終る */
      }
  }

  /*
   * tri を作りにかかる
   */
  len_tri = len_odir + len_sdir + strlen(info->tri_name) + 1;
  for(i = 0; cmd_tri[i] != NULL; i++)
    {
      if(*cmd_tri[i] == '%') continue;
      len_tri += strlen(cmd_tri[i]);
    }

  if(NULL == (tri_line = MALLOC(len_tri)))  return -1;
  tri_line[0] = 0;
  for(i = 0; cmd_tri[i] != NULL; i++)
    {
      add = cmd_tri[i];
      if(!strcmp(cmd_tri[i], "%o")) add = odir;
      if(!strcmp(cmd_tri[i], "%s")) add = sdir;
      if(!strcmp(cmd_tri[i], "%t")) add = info->tri_name;
      strcat(tri_line, add);
    }

  {
    int ret;

    printf("%s\n", tri_line);
    ret = system(tri_line);
    FREE(tri_line);
    if(NULL == (tri_line = MALLOC(9 + strlen(sdir)))) return -1;
    sprintf(tri_line, "rm %s*.cm2", odir);
    system(tri_line);  /* *.cm2 を消去 */
    if(ret != 0)  /* 作成に失敗したら .tri を消去 */
      {
	sprintf(tri_line, "rm %s", info->tri_name);
	system(tri_line);
      }
    FREE(tri_line);

    /* make_tri の終了値がエラーであれば、終了 */
    if(ret != 0)
      {
	printf("could not create .tri file.\n");
	exit(ret);
      }
  }

  return 0;
}

/*
 * TRI生成情報に基づき、使用する画像を search_dir で指定されるディレクトリから
 * 検索し、 out_dir で指定されるディレクトリに .tri ファイルを生成する
 */
int mgs2_BuildTRI(sprLayout * layout, char * search_dir, char * out_dir)
{
  char * sdir;
  char * odir;
  triInfo * info;

  /* 検索ディレクトリを正しい形にする */
  if(NULL == (sdir = dir_format(search_dir))) return -1;
  if(NULL == (odir = dir_format(out_dir)))
    {
      FREE(sdir);
      return -1;
    }

  for(info = layout->tri_info; NULL != info; info = info->next)
    {
      printf("--------\n");
      out_tri(info, sdir, odir);
    }

  FREE(sdir);
  FREE(odir);

  return 0;
}
