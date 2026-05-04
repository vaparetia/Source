/*
 * 出力データのチェック用プログラム
 *
 * デバッグ用であるため、結構いい加減。
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _view_c_
#include "view.h"

static int check_def(long ver, long begin, FILE * rfp);
static int check_stat(long ver, long begin, FILE * rfp);
static int check_act(long ver, long begin, FILE * rfp);

/*
 * バージョンチェック
 */
static int ver_check(long ver, int major, int minor, int patch)
{
  int mj, mn, pt;
  int v1, v2;

  mj = (ver >> 8) & 0xff;
  mn = (ver >> 16) & 0xff;
  pt = (ver >> 24) & 0xff;

  v1 = (major << 16) + (minor << 8) + patch;
  v2 = (mj << 16) + (mn << 8) + pt;

  return v2 >= v1;
}


/*
 * リトルエンディアンとして、4bytesのデータを 32bits 整数として読み込む
 */
int fgetl(unsigned long * l, FILE * rfp)
{
  unsigned char tmp[4];
  int i;

  if(fread(tmp, 1, 4, rfp) < 4) return EOF;
  *l = 0;
  for(i = 0; i < 4; i++)
    *l |= (unsigned long)tmp[i] << (8 * i);

  return 0;
}

/*
 * リトルエンディアンとして、2bytes のデータを 16bits 整数として読み込む
 */
int fgetw(unsigned short * w, FILE * rfp)
{
  unsigned char tmp[2];
  int i;

  if(fread(tmp, 1, 2, rfp) < 2) return EOF;
  *w = 0;
  for(i = 0; i < 2; i++)
    *w |= (unsigned short)tmp[i] << (8 * i);

  return 0;
}

int check_data(char * fname)
{
  FILE * rfp;
  long ver, def, stat, act;
  int ret;

  if(NULL == (rfp = fopen(fname, "rb"))) return EXIT_FAILURE;

  /* オブジェクト宣言、状態定義、アクション設定の
     各セクションが開始されるファイル上のアドレスを得る */
  fgetl(&ver,  rfp);  /* バージョン情報 */
  fgetl(&def,  rfp);
  fgetl(&stat, rfp);
  fgetl(&act,  rfp);

  if(EXIT_FAILURE == (ret = check_def(ver, def, rfp))) return EXIT_FAILURE;
  if(EXIT_FAILURE == (ret = check_stat(ver, stat, rfp))) return EXIT_FAILURE;
  if(EXIT_FAILURE == (ret = check_act(ver, act, rfp))) return EXIT_FAILURE;

  fclose(rfp);
  return EXIT_SUCCESS;
}

/*
 * メイン関数 ( みりゃわかる )
 */
int main(int argc, char **argv)
{
  int i;
  int ret;

  ret = EXIT_SUCCESS;
  for(i = 1; i < argc; i++)
    if(EXIT_FAILURE == (ret = check_data(argv[i]))) break;

  return ret;
}





/*
 * オブジェクト定義部のチェック
 */
static int check_def(long ver, long begin, FILE *rfp)
{
  long count, code;
  int i, ver0_1_0, ver0_1_1;
  short id, parent;
  int attrib, vertex_cnt;
  static char * attrib_name[] = {
    "Empty", "Point", "Line", "LineStrip", "Box", "Sprite", "Zoom"
  };

  ver0_1_0 = ver_check(ver, 0, 1, 0);
  ver0_1_1 = ver_check(ver, 0, 1, 1);

  /* 先頭に移動 */
  fseek(rfp, begin, SEEK_SET);

  fgetl(&count, rfp);   /* 定義個数を取得 */
  
  /* 各オブジェクトの情報を表示する */
  for(i = 0; i < count; i++)
    {
      if(ver0_1_0)
	{
	  fgetl(&code, rfp);
	  printf("StrCode: %8lx\n", code);
	}

      fgetw(&id, rfp);         /* オブジェクト ID                   */
      fgetw(&parent, rfp);     /* 親オブジェクトの ID               */
      attrib = fgetc(rfp);     /* オブジェクト属性                  */
      vertex_cnt = fgetc(rfp); /* 頂点数(line strip のみ意味をもつ) */
      fgetc(rfp);              /* 未定義領域を読み飛ばす */
      fgetc(rfp);              /* 未定義領域を読み飛ばす */
      if(ver0_1_0)
	{
	  fgetc(rfp);              /* 未定義領域を読み飛ばす */
	  fgetc(rfp);              /* 未定義領域を読み飛ばす */
	  fgetc(rfp);              /* 未定義領域を読み飛ばす */
	  fgetc(rfp);              /* 未定義領域を読み飛ばす */
	}

      printf("ID: %d ( parent: %d )"
	     "  object: %s  vertex: %d\n",
	     id, parent, attrib_name[attrib], vertex_cnt);
    }
  return EXIT_SUCCESS;
}

#define E_BIAS (8)
/*
 * 16bit 浮動小数点型を float 型に変換する
 */
static float float_short(unsigned short mf)
{
  unsigned int    s_dat, e_dat, f_dat ;
  float                   f ;
  
  /* 符合取りだし */
  s_dat = ( mf & 0x8000 ) << 16 ;
  /* 指数取りだし */
  e_dat = ( mf & 0x7c00 ) ;
  if ( e_dat != 0 ){
    e_dat += ( 127 - E_BIAS ) << 10 ;
    e_dat <<= 13 ;
  }
  /* 仮数部取りだし */
  f_dat = ( mf & 0x03ff ) << 13 ;
  /* ビット合成 */
  *(unsigned int*)&f = s_dat | e_dat | f_dat ;
  
  return ( f );
}

static int check_stat(long ver, long begin, FILE *rfp)
{
  long count;
  short id, stat_cnt;
  int i;

  fseek(rfp, begin, SEEK_SET);  /* 先頭に移動 */
  fgetl(&count, rfp);           /* 状態定義ブロック個数を得る */

  for(i = 0; i < count; i++)
    {
      fgetw(&id, rfp);
      fgetw(&stat_cnt, rfp);

      if(id < 0)   /* tri 定義 */
	{
	  long code;

	  fgetl(&code, rfp);
	  printf("TRI define: strcode = %08lx\n", code);
	  continue;
	}

      /*
       * オブジェクト状態定義
       */
      {
	long code;
	unsigned long cmd;
	int st;
	printf("Object = %d\n", id);

	for(st = 0; st < stat_cnt; st++)
	  {
	    fgetl(&code, rfp);   /* 状態名の StrCode */
	    printf("\tstatus: code = %08lx\n", code);
	
	    while(fgetl(&cmd, rfp), (cmd & SEQ_STAT_MASK) != SEQ_STAT_END)
	      {
		switch(cmd & SEQ_STAT_MASK)
		  {
		  case SEQ_STAT_DISP:  /* 表示 / 非表示切替え */
		    if(cmd & 1)
		      printf("\t\tshow\n");
		    else
		      printf("\t\thide\n");
		    break;
		  case SEQ_STAT_TEX:  /* 使用テクスチャ設定 */
		    printf("\t\ttex: code = %08lx\n", cmd & 0xffffff);
		    break;
		  case SEQ_STAT_VERTEX:  /* 頂点設定           */
		    {
		      int begin, num;
		      int j;
		      unsigned short x, y;
		      float fx, fy;
		      
		      printf("\t\tvertex position:\n");
		      begin = cmd & 0xff;
		      num = (cmd & 0xff00) >> 8;
		      
		      for(j = 0; j < num; j++)
			{
			  fgetw(&x, rfp);  /* 16bit 浮動小数点 */
			  fgetw(&y, rfp);  /* 16bit 浮動小数点 */
			  fx = float_short(x);
			  fy = float_short(y);
			  
			  printf("\t\t\tvertex [%d]: (%8.3f, %8.3f)\n",
				 j + begin, fx, fy);
			}
		    }
		    break;
		  case SEQ_STAT_RGBA:  /* 頂点色設定         */
		    {
		      int begin, num;
		      int j;
		      int r, g, b, a;
		      
		      printf("\t\tvertex color:\n");
		      begin = cmd & 0xff;
		      num = (cmd & 0xff00) >> 8;
		      
		      for(j = 0; j < num; j++)
			{
			  r = fgetc(rfp);   g = fgetc(rfp);
			  b = fgetc(rfp);   a = fgetc(rfp);
			  
			  printf("\t\t\tvertex [%d]: [ %02x, %02x, %02x, %02x ]\n",
				 j + begin, r, g, b, a);
			}
		    }
		    break;
		  case SEQ_STAT_SIZE:  /* 表示サイズ指定    */
		    {
		      unsigned short w, h;
		      float fw, fh;
		      
		      fgetw(&w, rfp);
		      fgetw(&h, rfp);
		      fw = float_short(w);
		      fh = float_short(h);
		      
		      printf("\t\tsize: %8.3f  %8.3f\n", fw, fh);
		    }
		    break;
		  case SEQ_STAT_UV:
		    {
		      unsigned short u, v;
		      float fu, fv;
		      
		      fgetw(&u, rfp);
		      fgetw(&v, rfp);
		      fu = float_short(u);
		      fv = float_short(v);
		      
		      printf("\t\tuv: %8.3f  %8.3f\n", fu, fv);
		    }
		    break;
		  case SEQ_STAT_TXSIZ:  /* テクセルサイズ */
		    {
		      unsigned short u, v;
		      float fu, fv;
		      
		      fgetw(&u, rfp);
		      fgetw(&v, rfp);
		      fu = float_short(u);
		      fv = float_short(v);
		      
		      printf("\t\tuv-size: %8.3f  %8.3f\n", fu, fv);
		    }
		    break;
		  case SEQ_STAT_ALPHA:
		    {
		      int a, b, c, d, fix;
		      a = cmd & 3;
		      b = (cmd & 0x0c) >> 2;
		      c = (cmd & 0x30) >> 4;
		      d = (cmd & 0xc0) >> 6;
		      fix = (cmd & 0xff0000) >> 16;
		      printf("\t\talpha: A[%d] B[%d] C[%d] D[%d] FIX[%d]\n",
			     a, b, c, d, fix);
		    }
		    break;
		  case SEQ_STAT_PRI:
		    printf("\t\tPriority: +%ld\n", (cmd & 7));
		    break;

		  case SEQ_STAT_CENTER:
		    {
		      unsigned short scx, scy;
		      float cx, cy;

		      fgetw(&scx, rfp);
		      fgetw(&scy, rfp);
		      cx = float_short(scx);
		      cy = float_short(scy);
		      printf("\t\tCenter: (%8.3f, %8.3f)\n", cx, cy);
		    }
		    break;
		  case SEQ_STAT_ANGLE:
		    {
		      float angle;

		      angle = float_short(cmd & 0xffff);
		      printf("\t\tAngle: %8.3f\n", angle);
		    }
		    break;
		  }
	      }
	  }
      }
    }
  return EXIT_SUCCESS;
}

static int check_act(long ver, long begin, FILE *rfp)
{
  int i;
  long count;

  fseek(rfp, begin, SEEK_SET);  /* 先頭に移動 */
  fgetl(&count, rfp);  /* アクションの数を取得 */
  
  for(i = 0; i < count; i++)
    {
      long code;
      long tracks;
      int j;

      fgetl(&code, rfp);   /* アクション識別用の StrCode を得る    */
      fgetl(&tracks, rfp); /* アクションに含まれるトラック数を得る */

      printf("Action ID: %08lx\n", code);

      for(j = 0; j < tracks; j++)
	{
	  short obj_id;
	  short dummy;
	  unsigned long cmd;

	  fgetw(&obj_id, rfp);
	  fgetw(&dummy, rfp);
	  
	  printf("\tObject ID: %d\n", obj_id);
	  /*
	   * そのオブジェクトのトラックシーケンスを表示する
	   */
	  while(fgetl(&cmd, rfp), (cmd & SEQ_ACT_MASK) != SEQ_ACT_END)
	    {
	      long t;
	      long code;

	      switch(cmd & SEQ_ACT_MASK)
		{
		case SEQ_ACT_WAIT:  /* wait         */
		  t = cmd & 0xffff;
		  printf("\t\twait: %ld\n", t);
		  break;
		case SEQ_ACT_SET:  /* set          */
		  code = cmd & 0xffffff;
		  printf("\t\tset: %08lx\n", code);
		  break;
		case SEQ_ACT_MORF:  /* morf         */
		  t = cmd & 0xffff;
		  fgetl(&code, rfp);
		  printf("\t\tmorf: time = %ld, status=%08lx\n", t, code);
		  break;
		}
	    }
	}
    }
  
  return EXIT_SUCCESS;
}
