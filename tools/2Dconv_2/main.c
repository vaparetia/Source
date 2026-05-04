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

int conv_to_bin(char * name, char * outdir, int tri_build)
{
  int ret;
  char * newname;
  FILE * rfp;
  FILE * wfp;
  sprLayout * layout;
  char * buf;
  int len, slash, fbase;

  if(NULL == (rfp = fopen(name, "r"))) return -1;

  ret = (NULL == (layout = sprReadLayoutScript(rfp)))
    ? EXIT_FAILURE : EXIT_SUCCESS;
  fclose(rfp);

  /*
   * 読み込まれたレイアウト情報を、MGS2バイナリ形式で出力する
   */

  /* ファイル名の変換 : suffix を付け変える */
  if(NULL == (newname = fnmMakeSuffixExchanged(name, ".o2d")))
    {
      ret = EXIT_FAILURE;
      goto err0;
    }

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
    {
      ret = EXIT_FAILURE;
      goto err1;
    }
  buf[0] = 0;
  if(outdir != NULL)
    {
      strcpy(buf, outdir);
      if(slash) strcat(buf, "/");
    }
  strcat(buf, newname + fbase);
  
  if(NULL == (wfp = fopen(buf, "wb")))
    {
      ret = EXIT_FAILURE;
      goto err2;
    }

  /* 出力 */
  ret = mgs2_Output2D(layout, wfp);
  fclose(wfp);

  /* TRI 出力情報に基づき、TRI を生成する。 */
  if(tri_build) mgs2_BuildTRI(layout, dirSearch, outdir);
  
 err2:
  free(buf);
 err1:
  free(newname);
 err0:
  sprDestroyReadData(layout);
  return ret;
}

static int option(char * param)
{
  if(!strcmp(param, "-v"))
    {
      printf("layout2d yacc version 0.01\n");
      return 0;
    }
  if(!strcmp(param, "-h"))
    {
      printf("usage: layout2d [-tri [-tex texdir]] -o outputdir filename.l2d\n");
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

  for(i = 1; i < argc; i++)
    {
      if(*argv[i] == '-')
	{ 
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
	  if(!strcmp(argv[i], "-tri"))
	    {
	      tri_build = 1;
	      continue;
	    }
	  if(option(argv[i]))
	    {
	      ret = EXIT_FAILURE;
	      break;
	    }
	  continue;
	}
      if(pool_init())
	{
	  fprintf(stderr, "not enough memory.\n");
	  return EXIT_FAILURE;
	}
      if(conv_to_bin(argv[i], outdir, tri_build)) ret = EXIT_FAILURE;
    }
  return ret;
}
