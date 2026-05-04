#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _main_c_
#include "main.h"
#include "yacc_parser.h"
#include "2d_data.h"
#include "fname.h"
#include "futil.h"
#include "util.h"
#include "mgs2.h"
#include "info.h"
#include "tri.h"
#include "xti.h"


/*
 * ファイル名本体と、出力ディレクトリ,suffix から,出力パスを生成する。
 */
static char * create_path(char * fname, char * outdir, char * suffix)
{
  char * newname;
  char * buf;
  int fbase, len, slash;

  if(NULL == (newname = fnmMakeSuffixExchanged(fname, suffix)))
    return NULL;

  fbase = len = slash = 0;
  if(NULL != outdir)
    {
      len = strlen(outdir);
      if(len > 0) if(outdir[len - 1] != '/') slash = 1;

      /* ファイル名本体の開始される位置を得る */
      for(fbase = strlen(newname); fbase > 0; fbase--)
	if(newname[fbase - 1] == '/') break;
    }
  if(NULL == (buf = malloc(len + slash + strlen(newname + fbase) + 1)))
    goto err1;

  buf[0] = 0;
  if(outdir != NULL)
    {
      strcpy(buf, outdir);
      if(slash) strcat(buf, "/");
    }
  strcat(buf, newname + fbase);
 err1:
  free(newname);
  return buf;
}



int conv_to_bin(char * name, char * outdir, int tex_build, int prev_info, int opt)
{
  int ret;
  FILE * rfp;
  FILE * wfp;
  sprLayout * layout;
  char * buf;

  if(NULL == (rfp = fopen(name, "r"))) return -1;

  ret = (NULL == (layout = sprReadLayoutScript(rfp)))
    ? EXIT_FAILURE : EXIT_SUCCESS;
  fclose(rfp);

  /*
   * 読み込まれたレイアウト情報を、MGS2バイナリ形式で出力する
   */
  if(NULL == (buf = create_path(name, outdir, ".o2d")))
    {
      ret = EXIT_FAILURE;
      goto err0;
    }

  if(NULL == (wfp = fopen(buf, "wb")))
    {
      printf("buf = %s\n", buf);
      ret = EXIT_FAILURE;
      fclose(wfp);
      free(buf);
      goto err0;
    }

  /* 出力 */
  ret = mgs2_Output2D(layout, wfp);
  fclose(wfp);


  /* TRI 出力情報に基づき、TRI を生成する。 */
  if(tex_build) ret = mgs2_BuildTRI(layout, dirSearch, outdir, opt);

  /* プレビュー環境用アクション名情報を出力する */
  free(buf);
  if(NULL == (buf = create_path(name, outdir, ".i2d")))
    {
      ret = EXIT_FAILURE;
      goto err0;
    }
  
  if(prev_info) infoOutputInfo(buf, layout);
  
  free(buf);
 err0:
  sprDestroyReadData(layout);
  return ret;
}

static int option(char * param)
{
  if(!strcmp(param, "-v"))
    {
      fprintf(stderr, "layout2d yacc version 0.10\n");
      return 0;
    }
  if(!strcmp(param, "-h"))
    {
      fprintf(stderr,
	      "usage: layout2d [-trad][-tri -xti [-cm2] [-tex texdir]] -o outputdir filename.l2d\n"
		"\t-xti output XTI-file for XBOX\n"
		"\t-tri output TRI-file for PS2\n");
      return 0;
    }
  return -1;
}

int main(int argc, char **argv)
{
  int i;
  int ret = EXIT_SUCCESS;
  char * outdir = NULL;
  int    tri_build = 0;
  int    xti_build = 0;
  int    prev_info = 0;
  int    tri_opt = 0;

  tri_Init();
  xti_Init();

  trad_flag = 0;
  for(i = 1; i < argc; i++)
    {
      if(*argv[i] == '-')
	{ 
	  /* オプション -p は、プレビュー用アクション名情報を出力する */
	  if(!strcmp(argv[i], "-p"))
	    {
	      prev_info = 1;
	      continue;
	    }
	  /* オプション -o は、出力先ディレクトリ指定 */
	  if(!strcmp(argv[i], "-o"))
	    {
	      i++;
	      outdir = argv[i];
	      continue;
	    }
	  /* オプション -tex は、テクスチャ元画像ディレクトリ指定 */
	  if(!strcmp(argv[i], "-tex"))
	    {
	      i++;
	      dirSearch = argv[i];
	      continue;
	    }
	  /* オプション -tri があれば、tri を生成する */
	  if(!strcmp(argv[i], "-tri"))
	    {
	      tri_build = 1;
	      continue;
	    }
	  /* オプション -cm2 は、tri 生成時に使用した .cm2 を消去しない */
	  if(!strcmp(argv[i], "-cm2"))
	    {
	      tri_opt |= TRIOPT_CM2;
	      continue;
	    }
	  /* オプション -xti があれば、xti を生成する */
	  if(!strcmp(argv[i], "-xti"))
	    {
	      xti_build = 2;
	      continue;
	    }

	  /* オプション -trad があれば、旧来と同じく,
	     同じ名称の状態があってもエラーで止めない(処理上は危険) */
	  if(!strcmp(argv[i], "-trad"))
	    {
	      trad_flag = 1;
	      continue;
	    }
	      
	  if(option(argv[i]))
	    {
	      fprintf(stderr, "Bad Option: [%s]\n", argv[i]);
	      ret = EXIT_FAILURE;
	      return ret;
	      break;
	    }
	  continue;
	}

      if(pool_init())
	{
	  fprintf(stderr, "not enough memory.\n");
	  return EXIT_FAILURE;
	}
      if(conv_to_bin(argv[i], outdir, tri_build|xti_build, prev_info, tri_opt))
	{
	  fprintf(stderr, "conv error.\n");
	  ret = EXIT_FAILURE;
	  return ret;
	}
    }

  if(ret != EXIT_SUCCESS)
    {
      fprintf(stderr, "<DEBUG-R> ret = %d\n", ret);
      exit(EXIT_FAILURE);
    }

  /* 処理が終ったら, 必要とされるテクスチャを作りにかかる */
  if ( tri_build )
	if(tri_OutputTRI(dirSearch, outdir, tri_opt))
	  ret = EXIT_FAILURE;

  if(ret != EXIT_SUCCESS)
    {
      fprintf(stderr, "<DEBUG-T> ret = %d\n", ret);
      exit(EXIT_FAILURE);
    }

  if ( xti_build )
	if(xti_OutputXTI(dirSearch, outdir, tri_opt))
	  ret = EXIT_FAILURE;
  //return ret;
  if(ret != EXIT_SUCCESS)
    {
      fprintf(stderr, "<DEBUG-2> ret = %d\n", ret);
      return ret;
    }

  exit(ret);
}
