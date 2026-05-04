/*
 * vsc の出力した、音声時系列表から、口アニメーションのモーションを作成する。
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _main_c_
#include "mtnlib.h"
#include "proc.h"
#include "main.h"


int main(int argc, char **argv)
{
  int ret = 0;
  char * seqname;
  char * prefix;

  /* 必要な全データの用意 */
  ret |= procLoadAllData(argc, argv);
  if(ret)
    {
      fprintf(stderr,
	      "usage: vmtn -l languageinfo \\\n"
	      "            -s lipsequence \\\n"
	      "            -i trackinfo \\\n"
	      "            -o outputdir \\\n"
	      "            -m motioninfo[:setname] [-m motioninfo[:setname] [...]]\n\n"
	      "\tlanguageinfo\tLip blending rate infomation\n"
	      "\tlipsequence\tVoice timetable sequence\n"
	      "\ttrackinfo\tMGS2 demo trackdata including speaker information\n"
	      "\tmotioninfo\tMotion element information\n"
	      "\toutputdir\tOutput destination directory.\n");

      return EXIT_FAILURE;
    }

  /* 時系列ファイルの名称を得る */
  seqname = procGetSeqName();

  {
    int pos;

    /* ファイル名の先頭を prefix で指すようにする。
       (それ以前のディレクトリ名は省く) */
    for(pos = strlen(seqname) - 1; pos > 0; pos--)
      if(seqname[pos - 1] == '/') break;
    prefix = seqname + pos;
    /* suffix を除去する */
    for(pos = 0; prefix[pos]; pos++) if(prefix[pos] == '.') break;
    prefix[pos] = 0;
    printf("prefix = %s\n", prefix);
  }

  
  /* 話者毎のモーションを出力する */
  ret |= procMakeMotion(prefix);

  /* 全データ領域を開放する */
  procReleaseAllData();

  ret = ret ? EXIT_FAILURE : EXIT_SUCCESS;
  return ret;
}

