/*
 * vsc の出力した時系列データを、組み込み用の
 * シーケンスデータに変換する
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define _main_c_
#include "main.h"
#include "lang_env.h"
#include "lip.h"
#include "binseq.h"

static LANG  lang_mode = JAPANESE;  /* デフォルトは日本語モード */

static struct {
  char    * name;
  LANG      val;
  BOOLEAN   adjust;
} lang_opt_list[] = {
  {"--japanese", JAPANESE, TRUE  },
  {"--english",  ENGLISH,  TRUE  },
#if 0
  {"--french",   FRENCH,   FALSE },
  {"--italian",  ITALIAN,  TRUE  },
  {"--germany",  GERMANY,  FALSE },
#endif
  { NULL, unknown }
};

static void usage(char *prog)
{
  fprintf(stderr,
	  "\nusage: %s [options] infile outfile\n\n"
	  
	  "Options:\n"
	  "  --japanese\tJapanese voice mode [default]\n"
	  "  --english\tEnglish voice mode\n"
	  /*
	    "  --french\tFrench voice mode\n"
	    "  --italian\tItalian voice mode\n"
	    "  --germany\tGermany voice mode\n"
	  */
	  "\n", prog);
  exit(EXIT_FAILURE);
}

/*
 * オプション処理
 */
static int option(char * opt)
{
  int i;

  for(i = 0; lang_opt_list[i].name != NULL; i++)
    if(!strcmp(lang_opt_list[i].name, opt))
      {
	lang_mode = lang_opt_list[i].val;
	return 0;
      }

  return -1;
}

char ** split(char *line, int nums)
{
  int i, j, k;
  char **list;
  char *tmp;
  void *vpt;
  int c, len;

  c = 0;
  i = 0;
  while(line[i])
    {
      if(line[i] == ' ' || line[i] == '\t')
	{
	  i++;
	  continue;
	}
      len = 0;
      for(j = i; line[j] && line[j] != ' ' && line[j] != '\t'; j++) len++;
  
      if(NULL == (tmp = malloc(len + 1))) goto err_0;
      strncpy(tmp, line + i, len);
      tmp[len] = 0;
      vpt = (c == 0)
	? malloc(sizeof(char *))
	: realloc(list, sizeof(char *) * (c + 1));
      if(NULL == vpt) goto err_1;

      list = vpt;
      list[c] = tmp;
      c++;
      i += len;
    }
  
  vpt = (c == 0)
    ? malloc(sizeof(char *))
    : realloc(list, sizeof(char *) * (c + 1));
  if(NULL == vpt) goto err_0;
  list = vpt;
  list[c] = NULL;

  return list;

 err_1:
  free(tmp);
 err_0:
  for(i = 0; i < c; i++) free(list[i]);
  if(c > 0) free(list);

  return NULL;
}

Seq ** read_seq(FILE *rfp)
{
  Seq * seq;
  Seq ** list;
  void *vpt;
  int nums;
  char line[256];
  int i, j;
  char **para;

  nums = 0;
  while(NULL != fgets(line, 256, rfp))
    {
      /* 改行コードとコロンを除去する */
      for(i = 0; line[i]; i++)
	{
	  if(line[i] == ':') line[i] = ' ';
	  if(line[i] == '\n') line[i] = '\0';
	}
      if(NULL == (para = split(line, 3))) goto err_0;
      if(NULL == (seq = malloc(sizeof(Seq)))) goto err_0;
      seq->t = atoi(para[0]);
      seq->phone = para[1];
      seq->power = atof(para[2]);
      free(para[0]);
      free(para[2]);
      free(para);

      vpt = (!nums)
	? malloc(sizeof(Seq *))
	: realloc(list, sizeof(Seq *) * (nums + 1));

      if(NULL == vpt) goto err_1;
      list = vpt;
      list[nums] = seq;
      nums++;
    }

  vpt = (!nums)
    ? malloc(sizeof(Seq *))
    : realloc(list, sizeof(Seq *) * (nums + 1));
  if(NULL == vpt) goto err_1;
  
  list = vpt;
  
  list[nums] = NULL;

  return list;
  
 err_1:
  free(seq->phone);
  free(seq);
 err_0:
  for(i = 0; i < nums; i++)
    {
      free(list[i]->phone);
      free(list[i]);
    }
  free(list);
  return NULL;
}


void disp_seq(Seq ** seq)
{
  int i;

  for(i = 0; seq[i] != NULL; i++)
    fprintf(stderr,
	    "%5d  %8s\t%8.3f\n",
	   seq[i]->t,
	   seq[i]->phone,
	   seq[i]->power);
}

int main(int argc, char **argv)
{
  FILE *rfp, *wfp;
  Seq ** seq;
  BinSeq * bin;
  int i, nums;
  int in, out;
  char * infile, * outfile;
  int error = 0;
  LangENV * env;

  in = out = -1;
  for(i = 1; i < argc; i++)
    {
      if(*argv[i] == '-')   /* オプション */
	{
	  if(option(argv[i])) error = 1;
	}
      else
	if(in < 0)
	  in = i;
	else if(out < 0)
	  out = i;
	else
	  error = 1;
    }
  if(in < 0 || out < 0) error = 1;
  if(error) usage(argv[0]);

  infile = argv[in];
  outfile = argv[out];

  rfp = (!strcmp(infile, "-")) ? stdin : fopen(infile, "r");
  if(NULL == rfp)
    {
      fprintf(stderr, "%s: could not open '%s'.\n", argv[0], infile);
      return EXIT_FAILURE;
    }

  wfp = (!strcmp(outfile, "-")) ? stdout : fopen(outfile, "wb");
  if(NULL == wfp)
    {
      if(stdin != rfp) fclose(rfp);
      fprintf(stderr, "%s: coult not create '%s'.\n", argv[0], outfile);
      return EXIT_FAILURE;
    }

  /* 言語依存環境を得る */
  env = envGetLanguageEnvironment(lang_mode);

  /* 時系列の取得 */
  seq = read_seq(rfp);

  /* 結合されている音素の分割,調整 */
  if(NULL == (seq = (env->adjust_func)(seq)))
    {
      fprintf(stderr, "could not adjust.\n");
      return EXIT_FAILURE;
    }

  /* 結果シーケンスを出力 */
  disp_seq(seq);

  /* シーケンスをバイナリに変換 */
  if(NULL == (bin = seq2bin(&nums, seq, env->table)))
    {
      fprintf(stderr, "could not conversion to binaly.\n");
      return EXIT_FAILURE;
    }
  for(i = 0; i < nums; i++)
    fprintf(stderr, "%5d %4d\t%3df\n",
	    bin[i].t, bin[i].phone_id, bin[i].power);

  fwrite(bin, sizeof(BinSeq), nums, wfp);

  if(wfp != stdout) fclose(wfp);
  if(rfp != stdin) fclose(rfp);

  return EXIT_SUCCESS;  
}
