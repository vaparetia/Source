/*
 * 顔のモーションデータセットの読み込み
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "mtnlib.h"

#define _vmotion_c_
#include "vmotion.h"
#include "util.h"

#ifdef _DEBUG_
#define DBG(args...)  { fprintf(stderr, args);  fflush(stderr); }
#else
#define DBG(args...)
#endif /* _DEBUG_ */


/* 母音文字列から、対応する母音番号を返す。 */
static int vmtn_label2id(char *label)
{
  char *v;

  v = label + strlen(label) - 1;
  if(!strcmp(label, "sil")) return VOWEL_S;   /* 無音 */
  if(!strcmp(v, "a"))   return VOWEL_A;   /* /a/  */
  if(!strcmp(v, "i"))   return VOWEL_I;   /* /i/  */
  if(!strcmp(v, "u"))   return VOWEL_U;   /* /u/  */
  if(!strcmp(v, "e"))   return VOWEL_E;   /* /e/  */
  if(!strcmp(v, "o"))   return VOWEL_O;   /* /o/  */
  if(!strcmp(v, "N"))   return VOWEL_N;   /* /N/  */
  if(!strcmp(v, "v"))   return VOWEL_V;   /* /v/  */

  /* 母音として認識しえないものは VOWEL_unknown を返す */
  return VOWEL_unknown;
}


/*
 * 母音モーションセット構造体を作成する
 */
static vmtnVowel * vmtn_make_vowel(void)
{
  vmtnVowel * vowel;
  int i;

  if(NULL != (vowel = malloc(sizeof(vmtnVowel))))
    {
      for(i = 0; i < VOWEL_NUMS; i++)
	{
	  vowel->mtn_len[i] = 0;
	  vowel->mtn_vowel[i] = NULL;
	}
      vowel->set_label = NULL;
      vowel->joints = 0;
    }

  return vowel;
}


/*
 * 母音モーションセットを開放する
 */
void vmtnReleaseVowel(vmtnVowel *vowel)
{
  int i;

  DBG("entering vmtnReleaseVower()\n");
  for(i = 0; i < VOWEL_NUMS; i++)
    {
      if(vowel->mtn_vowel[i] != NULL)
	MTN_FreeMotion(vowel->mtn_vowel[i]);
    }
  if(vowel->set_label != NULL) free(vowel->set_label);

  free(vowel);
  DBG("success\n");
}

/*
 * 母音モーションセットを開放する
 */
void vmtnReleaseMotion(vmtnSet *set)
{
  vmtnVowel * vowel;
  vmtnVowel * next;

  DBG("entering vmtnReleaseMotion()\n");
  vowel = set->begin;
  while(vowel != NULL)
    {
      next = vowel->next;
      vmtnReleaseVowel(vowel);
      vowel = next;
    }
  free(set);
}

static void vmtn_regist_vowel(vmtnSet * set, vmtnVowel * vowel)
{
  vowel->prev = set->end;
  vowel->next = NULL;
  if(set->end != NULL) set->end->next = vowel;
  set->end = vowel;
  if(set->begin == NULL) set->begin = vowel;
}



/*
 * 各母音に対応するモーションデータを読み込む
 */
vmtnSet * vmtnReadMotion(char *infoname)
{
  MTN_MOTION * mtn_tmp;
  vmtnSet   * set;
  vmtnVowel * vowel;
  FILE *rfp;
  char *line;
  char *index, *fname;
  int i, id;

  DBG("entering vmtnReadMotion()\n");
  if(NULL == (rfp = fopen(infoname, "r")))
    {
      fprintf(stderr, "could not open '%s'\n", infoname);
      return NULL;
    }


  DBG("reading...\n");


  /* 構造体領域の確保と初期化 */
  if(NULL == (set = malloc(sizeof(vmtnSet))))
    {
      fprintf(stderr, "not enough memory.\n");
      fclose(rfp);
      return NULL;
    }
  set->begin = set->end = NULL;
  set->speaker_name = NULL;
  set->bias = 1.0F;
  vowel = NULL;

  /* infoname は、読み込むモーションファイルの名前を列挙したテキストファイル */
  while(NULL != (line = utilReadLine(rfp)))
    {
      /* 
       * 空行、注釈行であれば、読み飛ばす
       */
      /* 注釈以下を無効にする */
      for(i = 0; line[i]; i++)
	if(line[i] == '#')
	  {
	    line[i] = 0;
	    break;
	  }

      /* 行末まで、空白以外の文字があるかどうかを確認する */
      for(i = 0; line[i]; i++)
	if(line[i] != ' ' && line[i] != '\t') break;

      /* 空白以外存在しない行であれば、次の行の処理に移る */
      if(line[i] == 0)
	{
	  free(line);
	  continue;
	}

      
      /* ':' を探す */
      for(i = 0; line[i]; i++) if(line[i] == ':') break;
      if(!line[i]) goto err_1;  /* ':' がなければエラー */

      /* ':' を境界として、文字列を二つに分ける */
      index = malloc(i + 1);
      fname = malloc(strlen(line + i));
      if(index == NULL || fname == NULL)goto err_2;
      strncpy(index, line, i);
      index[i] = 0;
      strcpy(fname, line + i + 1);

      /* 分割した文字列の、先頭および末尾にある空白を除去する */
      utilStrFips(index);
      utilStrFips(fname);

      /* ':' の左側が "name" であれば、話者名の指定であるとする */
      if(!strcmp(index, "name"))
	{
	  /* EUC-JP で書かれていると想定されるので、変換は必要無い */
	  /* strcode を求め、そのモーション素材を使用する話者として保持する */
	  set->speaker_code = get_strcode(fname);
	}
      /* ':' の左側が "idname" であれば出力ファイル名につく話者名部分とする */
      else if(!strcmp(index, "idname"))
	{
	  set->speaker_name = strdup(fname);
	}
      else if(!strcmp(index, "bias"))
	{
	  set->bias = atof(fname);
	}
      else if(!strcmp(index, "set"))
	{
	  /* モーション素材セット開始 */
	  if(NULL == (vowel = vmtn_make_vowel()))
	    {
	      for(vowel = set->begin; vowel != NULL; vowel = vowel->next)
		vmtnReleaseVowel(vowel);
	      free(set);
	      goto err_2;
	    }

	  /* 右側の文字列を、モーションセット名として登録 */
	  if(NULL == (vowel->set_label = strdup(fname)))  goto err_2;

	  vmtn_regist_vowel(set, vowel);  /* vmtnVowel 構造体を登録する */
	  DBG("vowel registed <1>: %s\n", fname);

	}
      else
	{
	  /* まだモーションセットに入っていなければ、
	     最初のモーションセットとして登録する */
	  if(vowel == NULL)
	    {
	      /* 旧書式の場合、最初のセットとして登録する。 */
	      /* モーション素材セット開始 */
	      if(NULL == (vowel = vmtn_make_vowel()))
		{
		  for(vowel = set->begin; vowel != NULL; vowel = vowel->next)
		    vmtnReleaseVowel(vowel);
		  free(set);
		  goto err_2;
		}
	      
	      /* 自動生成の場合、名称は"auto_default"になる。*/
	      if(NULL == (vowel->set_label = strdup("auto_default")))
		goto err_2;

	      
	      vmtn_regist_vowel(set, vowel);  /* vmtnVowel 構造体を登録する */
	      DBG("vowel registed <2>\n");
	    }

	  DBG("read motion to vowel\n");
	  /* ':' の左側を、母音記号とみなし、ID を取得する。 */
	  if(VOWEL_unknown == (id = vmtn_label2id(index))) goto err_2;

	  /* すでに同じ母音に対応するものとして
	     モーションが読み込まれていればエラー */
	  if(vowel->mtn_vowel[id] != NULL) goto err_2;
	  
	  /* ':' の右側をモーションファイル名とみなし、読み込む */
	  if(NULL == (mtn_tmp = MTN_OpenMTNFile(fname))) goto err_2;
	  
	  /* モーションの関節数が、顔モーションの関節数、
	     全身モーションの関節数のいずれとも異なっていればエラー */
	  if((mtn_tmp->header.motion_joints != FACE_JOINTS) &&
	     (mtn_tmp->header.motion_joints != BODY_JOINTS)) goto err_2;

	  if(!vowel->joints) /* まだ関節数が登録されていない場合 */
	    {
	      vowel->joints = mtn_tmp->header.motion_joints;
	      DBG("--------------- joints = %d\n", vowel->joints);
	    }
	  else
	    if(vowel->joints != mtn_tmp->header.motion_joints) goto err_2;

	  /* 読み込んだモーションを、先ほど取得した ID の母音に
	     対応するものとして、構造体に登録する */
	  vowel->mtn_vowel[id] = mtn_tmp;

	  /* モーションのフレーム数を取得し、構造体に格納する */
	  vowel->mtn_len[id] = mtn_tmp->header.motion_length;
	}
      free(index);
      free(fname);
      free(line);
    }

  /* 読み込まれていない母音モーションがないかどうか確認する */
  for(vowel = set->begin; vowel != NULL; vowel = vowel->next)
    {
      for(i = 0; i < VOWEL_NUMS; i++)
	if((vowel->mtn_vowel[i] == NULL) && (i != VOWEL_S)) goto err_0;
      
      /* 無音状態のモーションは設定しなくてもよい
	 (設定されていない場合は、ニュートラルモーションが
	 無音状態のモーションになる) */
      if(vowel->mtn_vowel[VOWEL_S] == NULL)
	{
	  /* 無音状態のモーションが読み込まれていない場合、
	     無音状態を表す空モーションを作り、割り当てる */
	  //vowel->mtn_vowel[VOWEL_S] = MTN_AllocMotion(FACE_JOINTS, 1);
	  DBG("joints = %d\n", vowel->joints);
	  vowel->mtn_vowel[VOWEL_S] = MTN_AllocMotion(vowel->joints, 1);
	  vowel->mtn_len[VOWEL_S] = 1;
	}
    }

  fclose(rfp);

  DBG("vmtnReadMotion() is success.\n");
  return set;
  
 err_2:
  if(index != NULL) free(index);
  if(fname != NULL) free(fname);
 err_1:
  free(line);
 err_0:
  /* 登録されているモーションはすべて開放 */
  vmtnReleaseMotion(set);
  fclose(rfp);
  DBG("vmtnReadMotion() is failed.\n");
  return NULL;  
}


