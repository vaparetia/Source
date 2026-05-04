/*
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _main_c_
#include "main.h"
#include "split.h"

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)   fprintf(stderr, args); fflush(stderr)
#endif /* _DEBUG_ */

/*
 * オプションの内容を保持するための変数群
 */
static int packet_size = DEFAULT_PACKET_SIZE;
static char * fname[2];

static struct {
  char * option;
  enum {
    vINT,
    vSTR,
    vDBL,
    vSET,
    vRST
  } mode;
  void * ptr;
} opt_list[] = {
  /* パケットサイズの設定 */
  { "--packet", vINT, &packet_size },
  { "-p",       vINT, &packet_size },

  /* リトルエンディアンモード */
  { "--little", vRST, &big_endian },
  { "--l",      vRST, &big_endian },

  /* ビッグエンディアンモード */
  { "--big",    vSET, &big_endian },
  { "-b",       vSET, &big_endian },

  /* パケット収録最大サイズ */
  { "--max",    vINT, &max_size },
  { "-x",       vINT, &max_size },

  /* パケットの余白を詰める */
  { "--fips",   vSET, &fips_mode },
  { "-f",       vSET, &fips_mode },

  { NULL,       vINT, NULL }
};


/*
 * オプション処理
 */
static int options(int argc, char **argv)
{
  int i, j;
  int cnt = 0;

  for(i = 0; i < 2; i++) fname[i] = NULL;
  for(i = 1; i < argc; i++)
    if(*argv[i] == '-' && strcmp(argv[i], "-"))
      {
	/* オプションを見付けた場合 */
	for(j = 0; opt_list[j].option != NULL; j++)
	  if(!strcmp(opt_list[j].option, argv[i]))
	    {
	      switch(opt_list[j].mode)
		{
		case vINT:  /* int 型であれば、型変換して
			       ポインタで指定された領域に放り込む */
		  *((int *)opt_list[j].ptr) = atoi(argv[i + 1]);
		  break;
		case vDBL:  /* double 型であれば、型変換して
			       ポインタで指定された領域に放り込む */
		  *((double *)opt_list[j].ptr) = atof(argv[i + 1]);
		  break;
		case vSTR:  /* 文字列をそのまま使用するオプションの場合、
			       ポインタで示されたポインタに、
			       ポインタをコピーする(ややこしい) */
		  *((char **)opt_list[j].ptr) = argv[i + 1];
		  break;
		case vSET:  /* 指定されたフラグを ON にする */
		  *(int *)opt_list[j].ptr = 1;
		  i--;
		  break;
		case vRST:  /* 指定されたフラグを OFF にする */
		  *(int *)opt_list[j].ptr = 0;
		  i--;
		  break;
		}
	      i++;
	      break;	      
	    }
      }
    else
      {
	if(cnt >= 2) return -1;
	fname[cnt] = argv[i];
	cnt++;
      }

  return 0;
}

static void  usage(char *argv0)
{
  fprintf(stderr,
	  "\nusage: %s [options] infile [outfile] \n\n"
	  "options:\n", argv0);

  fprintf(stderr,
	  "\t-p,--packet n\tpacket size[default: %d]\n", DEFAULT_PACKET_SIZE);

  fprintf(stderr, 
	  "\t-x,--max n   \tmax size in a packet [default: %d]\n", MAX_SIZE);

  fprintf(stderr, 
	  "\t-f,--fips    \tFips packet blank area.\n\n"
	  "\t-l,--little  \toutput Little Endian [default]\n"
	  "\t-b,--big     \toutput Big Endian\n"
	  "\n"
	  "if infile is \"-\", input from stdin.\n"
	  "if outfile is \"-\" or none, output to stdout.\n\n");

  exit(EXIT_FAILURE);
}
	  

int main(int argc, char **argv)
{
  FILE * rfp, * wfp;
  VoiceSeq * seq;
  VoiceSeq ** sp_seq;
  int ret;

  ret = options(argc, argv);   /* オプション処理 */

  if(NULL == fname[0] || ret < 0) usage(argv[0]);

  /* ファイル名として '-' が指定されている場合は、
     入力元を標準入力とする */
  if(!strcmp(fname[0], "-"))
    rfp = stdin;
  else
    if(NULL == (rfp = fopen(fname[0], "rb")))
      {
	fprintf(stderr, "%s: cannot open file '%s'.\n", argv[0], fname[0]);
	return EXIT_FAILURE;
      }

  /*
   * バイナリ化された生シーケンスデータを読み込む
   */
  seq = splLoadSequence(rfp);
  if(rfp != stdin) fclose(rfp);
  if(NULL == seq)
    {
      fprintf(stderr, "%s: not enough memory.\n", argv[0]);
      return EXIT_FAILURE;
    }
  /* シーケンスをパケット単位に分割する */
  sp_seq = splAutoSplit(seq, packet_size);

  if(sp_seq == NULL) return EXIT_FAILURE;

  if(NULL == fname[1] || !strcmp(fname[1], "-"))
    wfp = stdout;
  else
    if(NULL == (wfp = fopen(fname[1], "wb")))
      {
	splRelease(sp_seq);
	fprintf(stderr, "%s: cannot create file '%s'.\n", argv[0], fname[1]);
	return EXIT_FAILURE;
      }

  /* 分割されたシーケンスを出力する */
  splOutput(wfp, sp_seq, packet_size);

  splRelease(sp_seq);
  if(wfp != stdout) fclose(wfp);

  return EXIT_SUCCESS;  
}
