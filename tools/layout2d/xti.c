#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _xti_c_
#include "xti.h"
#include "2d_data.h"
#include "debug.h"

static _xti_info * begin = NULL;
static _xti_info * end   = NULL;

#define ERMSG(args...)   fprintf(stderr, args);

static void xti_tex_free(_xti_info * info, _xti_tex * tex)
{
  free(tex->tex_name);

  if(tex->prev != NULL) tex->prev->next = tex->next;
  if(tex->next != NULL) tex->next->prev = tex->prev;

  if(info->begin == tex) info->begin = tex->next;
  if(info->end == tex) info->end = tex->prev;
  free(tex);
}

static void xti_info_free(_xti_info * info)
{
  _xti_tex * tex;
  _xti_tex * next;

  tex = info->begin;
  while(NULL != info->begin)
    xti_tex_free(info, info->begin);

  if(NULL != info->prev) info->prev->next = info->next;
  if(NULL != info->next) info->next->prev = info->prev;

  if(begin == info) begin = info->next;
  if(end == info) end = info->prev;
}


int xti_Init(void)
{
  /* 現在確保されているものを全て開放 */
  while(begin != NULL) xti_info_free(begin);
}

static _xti_info * xti_new_xti(char * xti_name)
{
  _xti_info * info;

  if(NULL == (info = malloc(sizeof(_xti_info)))) return NULL;

  info->xti_name = strdup(xti_name);
  info->begin = info->end = NULL;

  info->next = NULL;
  info->prev = end;
  if(NULL != end) end->next = info;
  end = info;
  if(NULL == begin) begin = info;
  return info;
}


int xti_AddTexture(char * xti_name, char * tex_name)
{
  _xti_info * info;
  _xti_tex * tex;

  /* 同じ名称の XTI を探す */
  for(info = begin; NULL != info; info = info->next)
    if(!strcmp(xti_name, info->xti_name)) break;

  if(NULL == info) info = xti_new_xti(xti_name);
  if(NULL == info) return -1;  /* 追加できなかった */

  /*
   * 指定された XTI 中に、同じ名称が含まれていなければ追加
   */
  for(tex = info->begin; tex != NULL; tex = tex->next)
    if(!strcmp(tex->tex_name, tex_name)) break;

  /* 既に含まれていれば何もしない */
  if(NULL != tex) return 0;
  if(NULL == (tex = malloc(sizeof(_xti_tex)))) return -1;

  tex->tex_name = strdup(tex_name);
  tex->next = NULL;
  tex->prev = info->end;
  if(NULL != info->end) info->end->next = tex;
  info->end = tex;
  if(NULL == info->begin) info->begin = tex;

  return 0;
}

static char * make_cmdline(char ** form, int margin,
			   char * sdir, char * odir, char * name, char * body)
{
  size_t len;
  char * buf;
  char * ptr;
  int i, j;

  /* pass-1 全体の長さを求める */
  len = margin + 1;  /* 終端サイズ */
  for(i = 0; form[i] != NULL; i++)
    if(*form[i] == '%')
      {
	ptr = NULL;
	switch(*(form[i] + 1))
	  {
	  case 's':   /* source directory      */
	    len += strlen(sdir);
	    break;
	  case 'o':   /* destination directory */
	    len += strlen(odir);
	    break;
	  case 'n':   /* name                  */
	    len += strlen(name);
	    break;
	  case 'b':   /* body                  */
	    for(j = 0; body[j]; j++)
	      {
		if(body[j] == '.') break;
		len++;
	      }

	    break;
	  }
      }
    else
      len += strlen(form[i]);

  if(NULL == (buf = malloc(len))) return NULL;
  strcpy(buf, "");
  for(i = 0; form[i] != NULL; i++)
    {
      if(*form[i] == '%')
	{
	  ptr = NULL;
	  switch(*(form[i] + 1))
	    {
	    case 's':   /* source directory      */
	      strcat(buf, sdir);
	      break;
	    case 'o':   /* destination directory */
	      strcat(buf, odir);
	      break;
	    case 'n':   /* name                  */
	      strcat(buf, name);
	      break;
	    case 'b':   /* body                  */
	      ptr = buf + strlen(buf);
	      for(j = 0; body[j]; j++)
		{
		  if(body[j] == '.') break;
		  *(ptr++) = body[j];
		}
	      *ptr = 0;
	      break;
	    }
	}
      else
	strcat(buf, form[i]);
    }

  return buf;
}

static char * dir_form(char * dir)
{
  char * ret;
  int len = strlen(dir);
  int last = 0;

  DBG("dir = %s\n", dir);
  if(len > 0) 
    if(*(dir + len - 1) != '/')
      {
	len ++;
	last = 1;
      }

  if(NULL == (ret = malloc(len + 1))) return NULL;

  strcpy(ret, dir);
  DBG("ret = %s\n", ret);
  if(last) strcat(ret, "/");
  DBG("ret = %s\n", ret);

  return ret;
}



static int xti_output(_xti_info * info, char * source, char * output, int opt)
{
	static char * cmd_cp[]  = { "cp ", NULL };
	static char * cmd_cm2[] = { "makexti6", " -o ", "%o", "%b", " -f DXT3 ", " -i", NULL };
	static char * cmd_tri[] = { "make_xti", " -s -o ", "%o",
				    "%n", " -i ",
				    "%o", "%b", "*.tpk", NULL };
	static char * cmd_rm[]  = { "rm ", "%o", "%b", "*.tpk", NULL };
	_xti_tex * tex;
	char * sdir;
	char * odir;
	size_t len_sdir;
	size_t len_odir;
	char * cm2_line;
	char * tri_line;
	int len_tri;
	int ret;
	int len_cm2;

	/*makexti5 -s -o __tmp/2D_tex -i __tmp/cur/*.bmp -d  -c DXT2 > /dev/null*/

	if(NULL == source) source = "";
	sdir = dir_form(source);
	DBG("sdir: %s\n", sdir);
	
	if(NULL == output) output = "";
	odir = dir_form(output);
	DBG("odir: %s\n", odir);
	
	len_sdir = strlen(sdir);
	len_odir = strlen(odir);

	len_cm2 = 0;
	for(tex = info->begin; NULL != tex; tex = tex->next)
	len_cm2 += len_sdir + strlen(tex->tex_name) + 256;

	cm2_line = make_cmdline(cmd_cm2, len_cm2, sdir, odir, NULL, info->xti_name);
	for(tex = info->begin; NULL != tex; tex = tex->next)
	{
	    strcat(cm2_line, " ");
	    strcat(cm2_line, sdir);
	    strcat(cm2_line, tex->tex_name);
	    strcat(cm2_line, ".*");
	}
	strcat(cm2_line, " > /dev/null");

	/* makexti6 を実行 */
	{
	    int ret;
	    
	    //ERMSG("%s\n", cm2_line);
	    ret = system(cm2_line);
	    FREE(cm2_line);
	    printf("makext6 status = %d\n", ret);
	    if(ret != 0)  /* 終了値がエラーの場合 */
	    {
		ERMSG("texture source image not found.\n");
		for(tex = info->begin; NULL != tex; tex = tex->next)
		    ERMSG("\t%s%s.(bmp|pic)\n", sdir, tex->tex_name);

		return ret;
	    }
	}

	/*
	 * XTI を作りにかかる
	 */
	tri_line = make_cmdline(cmd_tri, 0, sdir, odir,
				info->xti_name, info->xti_name);
	{
	    int ret;
	    char *s ;

	    if ( (s = strstr( tri_line, ".tri" )) )
		*++s = 'x', *++s = 't' ;

	    ERMSG("%s\n", tri_line);
	    ret = system(tri_line);
	    FREE(tri_line);

	    if(!(opt & XTIOPT_CM2))
	    {
		tri_line = make_cmdline(cmd_rm, 0, sdir, odir, NULL, info->xti_name);
		system(tri_line);  /* *.cm2 を消去 */
		if(ret != 0)  /* 作成に失敗したら .tri を消去 */
		{
		    sprintf(tri_line, "rm %s", info->xti_name);
		    system(tri_line);
		}
		FREE(tri_line);
	    }

	    /* make_tri の終了値がエラーであれば、終了 */
	    if(ret != 0)
	    {
		ERMSG("could not create .xti file.\n");
		return ret;
	    }
	}
	
	free(sdir);
	free(odir);
	return 0;
}

int xti_OutputXTI(char * sdir, char * odir, int opt)
{
  _xti_info * info;
  int ret = 0;

  /* 登録されている XTI 情報すべてを出力する */
  for(info = begin; NULL != info; info = info->next)
    ret |= xti_output(info, sdir, odir, opt);

  return ret;
}
