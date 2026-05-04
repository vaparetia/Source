#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define _main_c_
#include "main.h"

#include "pcmdata.h"
#include "frame.h"
#include "analyze.h"
#include "segmentation.h"
#include "label.h"
#include "util.h"
#include "text.h"
#include "vowel.h"
#include "pickup_vec.h"
#include "ref_load.h"

#define OUTPUT_SCEDULE  0
#define OUTPUT_PARAMS   1
#define OUTPUT_STATUS   2
#define OUTPUT_WAVE     3
#define OUTPUT_GROUP    4
#define OUTPUT_CEPSTRUM 5
#define OUTPUT_VECTOR   6
#define OUTPUT_DIC      7

static int process_pcm(char *wav, char *text);
static int output_process(txtPhono ** tx_list, anaParam **p_list,
			  int frame_num, int base);


static PcmFILE *pfp;
static int      output_mode = OUTPUT_SCEDULE;
static double   stat_size = 1.0;

int main(int argc, char **argv)
{
  int i, j = 0;
  int ret = EXIT_SUCCESS;
  char *home, *opt_cnf_fname, *home_cnf_fname, *global_cnf_fname;
  char *fname;
  char *data_file[2];
  char * dicname = "jp_male.dic";

  mVoiceMode = VMODE_MALE;
  mLangMode  = LANG_JAPANESE;

  strcpy(mDicDir, DICTIONARY_DIR);
  segInit();   /* セグメンテーション閾値の初期化 */
  labInit();   /* ラベリング閾値の初期化         */
  vowelInit(); /* 母音ベクトルの初期化           */


  fprintf(stderr, "dic read.\n");

  opt_cnf_fname = NULL;
  global_cnf_fname = _SHAREDIR_ "/Voice";

#ifdef PERSONAL
  home = getenv("HOME");
#else
  home = CONFIG_DIR;
#endif
  home_cnf_fname = utilPathAppend(home, CONFIG_FILE);

  for(i = 1; i < argc; i++)
    {
      if('-' == *argv[i])
	{
	  if(!strncmp("-p", argv[i], 2)) output_mode = OUTPUT_PARAMS;
	  else if(!strncmp("-sc", argv[i], 3)) output_mode = OUTPUT_SCEDULE;
	  else if(!strncmp("-st", argv[i], 3)) output_mode = OUTPUT_STATUS;
	  else if(!strncmp("-v",  argv[i], 2)) mVarbose = 1;
	  else if(!strncmp("-w",  argv[i], 2)) output_mode = OUTPUT_WAVE;
	  else if(!strncmp("-g",  argv[i], 2)) output_mode = OUTPUT_GROUP;
	  else if(!strncmp("-D",  argv[i], 2)) output_mode = OUTPUT_DIC;
	  else if(!strcmp("-V", argv[i])) output_mode = OUTPUT_VECTOR;
	  else if(!strcmp("-c",  argv[i])) output_mode = OUTPUT_CEPSTRUM;
	  else if(!strcmp("-F",  argv[i])) mVoiceMode = VMODE_FEMALE;
	  else if(!strcmp("-M",  argv[i])) mVoiceMode = VMODE_MALE;
	  else if(!strcmp("-e", argv[i]))
	    {
	      dicname = "english.dic";
	      mLangMode = LANG_ENGLISH;
	    }
	  else if(!strcmp("-C",  argv[i]))
	    {
	      /* Other config file. */
	      opt_cnf_fname = utilDupStr(argv[i + 1]);
	      i++;
	    }
	  else if(!strcmp("-s", argv[i]))
	    {
	      if(argc > i + 1)
		{
		  stat_size = atof(argv[i + 1]);
		  i++;
		}
	      else
		{
		  fprintf(stderr, "%s: option '%s' was not given parameter.\n",
			  argv[0], argv[i]);
		  exit(EXIT_FAILURE);
		}
	    }
	  continue;
	}
      if(j < 2)
	{
	  data_file[j] = argv[i];
	  j++;
	}
    }

  /* 言語、性別から辞書名を作成する */
  {
    char * lang  = "jp";
    char * voice = "male";

    switch(mLangMode)
      {
      case LANG_JAPANESE:  lang = "jp";  break;
      case LANG_ENGLISH:   lang = "en";  break;
      }
    switch(mVoiceMode)
      {
      case VMODE_MALE:   voice = "male";   break;
      case VMODE_FEMALE: voice = "female"; break;
      }
    dicname = malloc(strlen(voice) + strlen(lang) + 6);
    if(dicname == NULL)
      {
	fprintf(stderr, "not enough memory.\n");
	return EXIT_FAILURE;
      }

    sprintf(dicname, "%s_%s.dic", lang, voice);
  }

  /*
   * 設定ファイルから読み込んだ閾値で、初期値を上書きする
   */
  segLoadParams(opt_cnf_fname,
		home_cnf_fname,
		global_cnf_fname);
  if(NULL != opt_cnf_fname) free(opt_cnf_fname);
  free(home_cnf_fname);
  fname = utilPathAppend(mDicDir, dicname);
  free(dicname);
  refLoadRef(fname);
  free(fname);
  if(process_pcm(data_file[0], data_file[1])) ret = EXIT_FAILURE;
  return ret;
}

static void param_output(anaParam **p_list, int frame_num, int base)
{
  int t, t2;
  anaParam *param;
  int skip;

  skip = (int)pfp->skip_time + base;

  /*
   * パラメータの出力
   */  
  printf("#  t"
	 "        Ci"
	 "        Vi"
	 "        Zi"
	 "        Ai"
	 "        Ei"
	 "        Fi"
	 "        Si"
	 "\n");

  for(t = 0; t < frame_num; t++)
    {
      t2 = t + skip;
      param = p_list[t];
      printf("%4d  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f\n",
	     t2,
	     param->Ci,
	     param->Vi,
	     param->Zi,
	     param->Ai,
	     param->Ei,
	     param->Fi,
	     param->Si);
    }
}

/*
 * 音声データの処理を行なう
 */
static int process_pcm(char *wav, char *text)
{
  txtPhono ** tx_list;
  anaParam * param;
  anaParam ** p_list;
  int frame_num;
  int t, i;
  int begin, end;   /* 処理開始および終了のフレーム */
  int sil_cnt, snd_flg;
  int s;
  void *vpt;
  FormID format;


  /* テキストで与えられた文字列を音素とその大分類に分解 */
  /*
    fprintf(stderr, "Text reading...\n");
    if(NULL == (tx_list = txtSplitText(text))) return -1;
  */
  tx_list = NULL;

  /* Wave データを開く */
  format = pcmDitectFormat(wav);

  if(format == FORM_unknown)
    {
      fprintf(stderr, "unknown PCM format.\n");
      return -1;
    }
  if(NULL == (pfp = pcmOpen(wav, format))) return -1;
  frame_num = (pfp->frames - FRAME_WINDOW) / FRAME_STEP;
  fprintf(stderr, "frame_num = %d\n", frame_num);

  frmInit(pfp);  /* 最初のフレームをバッファにロードする */

  fprintf(stderr, "Analyzing...\n");

  begin = end = 0;
  snd_flg = 0;
  sil_cnt = 0;
  p_list = NULL;
  for(t = 0; t < frame_num; t++)
    {

      /* バッファの追加 */
      if(NULL ==(vpt = utilAddBuf(p_list, sizeof(anaParam *), t)))
	{
	  if(p_list != NULL) free(p_list);
	  return -1;
	}
      p_list = vpt;
      
      /* 
	 fprintf(stderr, "frame: %d (time = %d) : ", t, t + (int)pfp->skip_time);
	 fflush(stderr);
      */

      /* この検出フレームをあらわす音響パラメータ構造体の作成 */
      param = anaParamNew(t);
      if(NULL == param)
	{
	  fprintf(stderr, "vsc: Not enough memory.\n");
	  exit(EXIT_FAILURE);
	}

      p_list[t] = param;   /* リストに登録 */
      
      /* 各フレームで独立した音響パラメータの計算 */
      anaCalcParam(param, frmWindowBuf, FRAME_WINDOW);
      frmSlideWindow(pfp);
      
    }
  /*
   * 隣り合ったフレーム単位での処理が必要な
   * セグメンテーションパラメータの算出
   * (Ai など)
   */
  anaLatticeParameter(p_list, t);

  fprintf(stderr, "\nSegmentation...\n");

  /* セグメンテーションを行なう */
  segSegmentation(p_list, t);

  fprintf(stderr, "Labeling...\n");

  /* とりあえず、現在までの時系列表を出力 */
  output_process(tx_list, p_list, t, begin);

  /* 時系列バッファを解放 */
  for(i = 0; i < t; i++)free(p_list[i]);
  free(p_list);
  p_list = NULL;

  fprintf(stderr, "completed.\n");
  pcmClose(pfp);
  return 0;
}

/*
 * 処理単位の内容を、出力形態に合わせて出力する
 */
static int output_process(txtPhono ** tx_list,  anaParam **p_list,
			  int frame_num, int base)
{
  int skip;

  skip = (int)pfp->skip_time + base;

  switch(output_mode)
    {
    case OUTPUT_PARAMS:
      /* セグメンテーション用パラメータの出力 */
      param_output(p_list, frame_num, base);
      break;
    case OUTPUT_STATUS:

      {
	int i, t;
	static int mask[3] = { ST_VOICED, ST_SILENT, ST_BORDER };
	static double level[3] = {0.8, 0.8, 1.0};
	for(t = 0; t < frame_num; t++)
	  {
	    printf("%5d", t + skip);
	    for(i = 0; i < 3; i++)
	      printf(" %f",
		     (p_list[t]->status & mask[i]) ?
		     (level[i] * stat_size) : 0);
	    printf("\n");
	  }
      }
      break;
    case OUTPUT_WAVE:
      {
	int t;

	/* まとめた波形を表示する。
	   波形は、各フレームの先頭 10[ms] 分における最大値で表示。 */
	printf("# Min(1), Max(1):   1 / %f\n"
	       "# Min(2), Max(2):   1 / %f\n#\n",
	       (double)WAV_SCALE_1, (double)WAV_SCALE_2);
	
	printf("#   t    Min(1)    Max(1)    Min(2)    Max(2)\n");
	for(t = 0; t < frame_num; t++)
	  {
	    printf("%5d",  t + skip);
	    printf("  %8.3f  %8.3f",
		   p_list[t]->w_min / WAV_SCALE_1,
		   p_list[t]->w_max / WAV_SCALE_1);
	    printf("  %8.3f  %8.3f\n",
		   p_list[t]->w_min / WAV_SCALE_2,
		   p_list[t]->w_max / WAV_SCALE_2);
	  }
      }
      break;
    case OUTPUT_DIC:
      /* 辞書編集用の出力を行う */
      mDicDisp = 1;
      labLabeling(skip, tx_list, p_list, frame_num, mVoiceMode);
      break;
    case OUTPUT_GROUP:
      /* 各音素境界の、音素群ラベルを表示する   */
      mGroupDisp = 1;
    case OUTPUT_SCEDULE:
      /* ラベリングを行ない、時系列表を出力する */
      labLabeling(skip, tx_list, p_list, frame_num, mVoiceMode);
      break;
    case OUTPUT_CEPSTRUM:
      {
	int i, t;

	/* 各分析フレームにおいて求めたメルケプストラム係数を出力する */
	printf("#   t"
	       "      gi[0]"
	       "      gi[1]"
	       "      gi[2]"
	       "      gi[3]"
	       "      gi[4]"
	       "      gi[5]"
	       "      gi[6]\n");
	
	for(t = 0; t < frame_num; t++)
	  {
	    printf("%5d", t + skip);
	    for(i = 0; i < 7; i++)
	      printf("  %8.3f", p_list[t]->gi[i]);
	    printf("\n");
	  }
      }
      break;
    case OUTPUT_VECTOR:
      {
	pvcPickupVector(skip, p_list, frame_num);
      }
      break;
    }

  return 0;
}
  
