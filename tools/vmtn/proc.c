#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _proc_c_
#include "mtnlib.h"
#include "proc.h"
#include "make_motion.h"
#include "read_seq.h"
#include "vmotion.h"
#include "vmtn_multi.h"
#include "inf_seq.h"
#include "mem_util.h"
#include "mtn_list.h"
#include "mtn_filter.h"

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)   { printf(args);  fflush(stdout); }
#endif /* _DEBUG_ */



/* 変換に使用する情報 */
static vmtnContainer * mtncont  = NULL; /* 各話者毎のモーション素材         */
static lipMotionList * mtnlist  = NULL; /* 各音素のモーション組み合わせ情報 */
static seqStep       * lipseq   = NULL; /* 全体を通した口パク情報           */
static TalkArea      * talkarea = NULL; /* 時間ごとの話者情報               */
static char          * seqname  = NULL; /* シーケンスデータ名               */
static char          * outdir   = NULL; /* 出力先ディレクトリ               */
static int             release  = 10;   /* リリースフレーム
					   このフレーム数だけ、末尾に余白
					   モーションを追加し、ニュートラルまで
					   減衰させる。                     */
					   
static char * strndup(char * str, int len)
{
  char * ptr;

  if(NULL == (ptr = malloc(len + 1))) return NULL;

  strncpy(ptr, str, len);
  *(ptr + len) = 0;
  return ptr;
}

static int add_motion(char * str)
{
  char * fname = NULL;
  char * setname = NULL;
  int i;
  vmtnSpeaker * sp;

  /* モーション指定ファイル名部分と、セット名を分離する */
  for(i = 0; str[i]; i++)
    if(str[i] == ':')
      {
	fname = strndup(str, i);
	setname = strdup(str + i + 1);
	break;
      }
  if(!str[i])
    {
      fname = strdup(str);
      setname = NULL;
    }
  if(fname == NULL)
    {
      if(setname != NULL) free(setname);
      return -1;
    }
  sp = vmmReadSpeakerMotion(fname);
  if(NULL == vmmRegistSpeaker(mtncont, sp))
    {
      if(NULL != fname) free(fname);
      if(NULL != setname) free(setname);
      return -1;
    }

  if(NULL != setname)
    vmmSelectMotionSet(mtncont, sp->code, setname);

  return 0;
}

char * procGetSeqName(void)
{
  return seqname;
}

int procLoadAllData(int argc, char **argv)
{
  int i;
  enum {
    md_motions,
    md_lang,
    md_sequence,
    md_inf,
    md_output,
    md_release
  } mode;
  char *list, *seq;

  /* あらかじめモーションコンテナを作っておく */
  mtncont = vmmCreateContainer();

  DBG("<1>\n");

  list = seq = NULL;
  for(i = 1; i < argc; i++)
    {
      if(*argv[i] == '-')
	{
	  switch(*(argv[i] + 1))
	    {
	    case 'm': mode = md_motions;  break;
	    case 'l': mode = md_lang;     break;
	    case 's': mode = md_sequence; break;
	    case 'i': mode = md_inf;      break;
	    case 'o': mode = md_output;   break;
	    case 'r': mode = md_release;  break;
	    }
	  continue;
	}
      switch(mode)
	{
	case md_motions:
	  DBG("[a]\n");
	  if(add_motion(argv[i])) goto err;
	  DBG("]a[\n");
	  break;
	case md_lang:      /* 各音素と、モーション組合せのリスト読み込み */
	  DBG("[b]\n");
	  list = argv[i];
	  break;
	case md_sequence:
	  DBG("[c]\n");
	  seq = argv[i];
	  break;
	case md_inf:
	  DBG("[d]\n");
	  if(NULL == (talkarea = infReadSpeaker(argv[i]))) goto err;
	  break;
	case md_output:
	  DBG("[e]\n");
	  {
	    size_t len;
	    if(outdir != NULL) free(outdir);

	    len = strlen(argv[i]);
	    if(len > 0)
	      {
		if(*(argv[i] + len - 1) != '/') len ++;
		if(NULL == (outdir = malloc(len + 1))) goto err;
		strcpy(outdir, argv[i]);
		*(outdir + len - 1) = '/';
		*(outdir + len) = 0;
	      }
	    else
	      {
		if(NULL == (outdir = malloc(2))) goto err;
		strcpy(outdir, "");
	      }
	    DBG("Output Directory = [%s]\n", outdir);
	  }
	  break;
	case md_release:
	  DBG("[r]\n");
	  {
	    if(utilStr2Int(&release, argv[i]))
	      release = 10; /* 読めない場合はデフォルト値 */
	  }
	  break;
	}
    }
  DBG("list = %p, seq = %p\n", list, seq);
  if(list == NULL || seq == NULL) goto err;

  DBG("<2>\n");

  if(NULL == (mtnlist = lipLoadMotionList(list))) goto err;

  DBG("<3>\n");

  seqname = seq;
  if(NULL == (lipseq  = rseqReadLipSequence(mtnlist, seq))) goto err;

  DBG("<4>\n");

  printf("Load Completed.\n");
  return 0;

  /* ここで必要な全データが揃っているかどうかを確認する */

 err:
  if(mtnlist != NULL) lipReleaseMotionList(mtnlist);
  if(lipseq != NULL) rseqReleaseSequence(lipseq);
  if(talkarea != NULL) infReleaseSpeaker(talkarea);
  DBG("<0>\n");
  vmmReleaseContainer(mtncont);

  return -1;
}


int procMakeMotion(char * prefix)
{
  FILE * wfp;
  int i, j, cnt, flg;
  int * people;
  char ** pnames;
  void * vpt;
  int step;
  int code, speaker;
  char * name;
  int vid, pre_vid;
  int frame, max_frame;
  float f_time;
  float bias;
  vmtnVowel  * vowel;
  MTN_MOTION * mtn_track;
  FACE_MOTION  face;

  DBG("entering procMakeMotion()\n");

  /* PASS-1: 音声中に登場する話者の数を数える。*/
  if(NULL == talkarea)
    {
      /* 話者情報が指定されていない場合は、話者を一人として扱う */
      cnt = 1;
      people = malloc(sizeof(int));
      pnames = malloc(sizeof(char *));
      /* モーションが設定されている最初の話者を話者とする */
      people[0] = mtncont->mtn_block[0]->code;
      pnames[0] = mtncont->mtn_block[0]->name;
    }
  else
    {
      cnt = 0;
      people = NULL;
      pnames = NULL;
      for(i = 0; talkarea[i].begin >= 0; i++)
	{
	  flg = 1;
	  for(j = 0; j < cnt; j++)
	    if(people[j] == talkarea[i].speaker) flg = 0;
	  if(!flg) continue;
	  
	  /* フラグがクリアされない = 新しい話者 */
	  if(NULL == (vpt = mem_add(people, sizeof(int), cnt)))
	    {
	      if(people != NULL) free(people);
	      return -1;
	    }
	  people = vpt;
	  if(NULL == (vpt = mem_add(pnames, sizeof(char *), cnt)))
	    {
	      if(people != NULL) free(people);
	      if(pnames != NULL) free(pnames);
	      return -1;
	    }
	  pnames = vpt;
	  people[cnt] = talkarea[i].speaker;
	  pnames[cnt] = talkarea[i].spname;
	  cnt ++;
	}
    }

  /* cnt が話者の数。モーション素材を知らない話者がいないかどうかチェック */
  DBG("Number of speaker = %d\n", cnt);
  for(i = 0; i < cnt; i++)
    {
      DBG("people[%d] = %d\n", i, people[i]);
      flg = 0;
      for(j = 0; j < mtncont->nums; j++)
	if(mtncont->mtn_block[j]->code == people[i]) flg = 1;
    }
  if(!flg)
    {
      fprintf(stderr, "Unknown speaker: strcode %d\n", people[i]);
      exit(EXIT_FAILURE);
    }

  /* PASS-2: 各話者毎にモーショントラックを生成、ファイルに出力する。 */
  vowel = NULL;
  for(i = 0; i < cnt; i++)
    {
      pre_vid = 0;
      /*
	各音素の時系列を話者情報と照合し、現在処理中の話者であれば
	その音素を元にモーションの生成を行う。

	現在処理中の話者の発言でなければ、無音として扱う
       */
      code = people[i];  /* 話者を特定   */
      name = name;
      DBG("code =%d: name = %s\n", code, name);

      DBG("mtncont->nums = %d\n", mtncont->nums);

      /* モーション素材群の用意 */
      for(j = 0; j < mtncont->nums; j++)
	{
	  if(code == mtncont->mtn_block[j]->code)
	    {
	      DBG("vowel: %p\n", mtncont->mtn_block[j]->motions);
	      vowel = mtncont->mtn_block[j]->motions;
	      name = mtncont->mtn_block[j]->name;
	      bias = mtncont->mtn_block[j]->set->bias;
	      break;
	    }
	}
      DBG("speaker code = %d: vowel=%p\n", code, vowel);

      if(NULL == vowel) continue;

      DBG("joints = %d\n", vowel->joints);
      /*
       * モーション格納域の確保
       */
      /* 音声のトータルフレーム数を得る */
      DBG("lipseq = %p\n", lipseq);
      for(j = 0; lipseq[j].time >= 0; j++)
	max_frame = lipseq[j].time;

      DBG("lipseq time max = %d\n", max_frame);
      max_frame = max_frame * 3 / TIME_BASE;  /* 最大画面フレーム数 */
      DBG("max_frame = %d\n", max_frame);

      //mtn_track = MTN_AllocMotion(FACE_JOINTS, max_frame);
      mtn_track = MTN_AllocMotion(vowel->joints, max_frame);

      for(frame = 0; frame < max_frame; frame++)
	{
	  /* 音声の分析フレーム数単位に直す */
	  f_time = (float)(frame * TIME_BASE) / 3;
	  DBG("f_time = %8.3f\n", f_time);
	  /* そのフレームが含まれる音素シーケンスの位置を得る */
	  step = -1;
	  j = 0;
	  while(f_time > (float)lipseq[j].time) step = j, j++;
	  DBG("step = %d\n", step);

	  /* まだ先頭の無音部分である場合 */
	  if(step < 0)
	    /* この段階ではモーションが無いのでそのまま次のフレームの処理へ */
	    continue;

	  /* 現在のフレームの話者を得る。
	     話者情報が設定されていない場合は、常に現在処理中の
	     人物が話者であるとして処理する */
	  j = 0;
	  if(talkarea != NULL)
	    {
	      for(j = 0; talkarea[j].begin >= 0; j++)
		if(lipseq[step].time >= talkarea[j].begin)
		  speaker = talkarea[j].speaker;
	    }
	  else
	    speaker = code;

	  /* 音素 ID を得る。話者が自分では無い場合、
	     無音をあらわす 0 として扱う */
	  vid = (speaker != code) ? 0 : lipseq[step].id;

	  
	  /* 正規化時刻をレートとし、直前の音素と、
	     今回の音素のモーション間で補間をとり、その結果を
	     今回のフレームのモーションとする */
	  {
	    FACE_MOTION * fm;

	    fm = mkMakeMotion(&face, mtnlist, vowel, pre_vid, vid,
			      f_time - (float)lipseq[step].time,
			      (float)(lipseq[step + 1].time -
				      lipseq[step].time));

	    if(NULL == fm) exit(EXIT_FAILURE);
	  }


	  /* モーションをパワーで補正する */
	  {
	    float power;

	    power = (float)lipseq[step].power / 255.0F;
	    power = power * power * bias;
	    if(power > 1.0F) power = 1.0F;
	    mkMotionPower(&face, vowel, &face, power);
	  }

	  /* 作成したモーションを、今回のフレームとして登録 */
	  SetMotion(mtn_track, frame, &face);
	  pre_vid = vid;
	}

      /* 完成したモーションに、補正フィルタをかける */
      MotionSmooth(mtn_track);

      /* モーションをファイルに出力する */
      {
	char *fname;

	/* ファイル名の作成 */
	if(outdir != NULL)
	  {
	    if(NULL == (fname = malloc(1 + strlen(outdir) + strlen(prefix) + 25))) goto err;
	    if(NULL == name)
	      sprintf(fname, "%s%s_%d.mtn", outdir, prefix, code);
	    else
	      sprintf(fname, "%s%s_%s_%s.mtn", outdir, prefix, name, vowel->set_label);
	  }
	else
	  {
	    if(NULL == (fname = malloc(1 + strlen(prefix) + 25))) goto err;
	    if(NULL == name)
	      sprintf(fname, "%s_%d.mtn", prefix, code);
	    else
	      sprintf(fname, "%s_%s_%s.mtn", prefix, name, vowel->set_label);
	  }
	DBG("Output file = [%s]\n", fname);
	/* モーションの出力 */
	MTN_SaveMTNFile(fname, mtn_track);
	DBG("Complete.\n");
	/* モーションの開放 */
	free(fname);
      }
      /* モーション領域を開放 */
      MTN_FreeMotion(mtn_track);
    }
  DBG("------------------ end ------------------------- \n");
  return 0;
  
 err:
  MTN_FreeMotion(mtn_track);
  return -1;  
}

  void procReleaseAllData(void)
{
  DBG("entering procReleaseAllData()\n");
  vmmReleaseContainer(mtncont);
  DBG("<1>\n");
  lipReleaseMotionList(mtnlist);
  DBG("<2>\n");

  rseqReleaseSequence(lipseq);
  DBG("<3>\n");
  infReleaseSpeaker(talkarea);
  DBG("<4>\n");
}
