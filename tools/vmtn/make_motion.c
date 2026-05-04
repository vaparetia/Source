/*
 * 与えられた音素 ID 二つから、その二つの音素のモーション間を任意レートで
 * 補間したモーションのフレームデータを得るためのモジュール。
 *
 * ファイルで読み込んだ音素のブレンド情報を手がかりにオリジナルモーションを
 * 加工、補間し、そのフレーム分のデータを返す。
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _make_motion_c_
#include "make_motion.h"
#include "mtn_list.h"

#include "mtnlib.h"
#include "mtn_blend.h"
#include "vmotion.h"

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)   { printf(args);  fflush(stdout); }
#endif /* _DEBUG_ */


static FACE_MOTION * get_cel_motion(FACE_MOTION * ret,
				    mtn_cel * cel, vmtnVowel * vow)
{
  FACE_MOTION mtn_a;
  FACE_MOTION mtn_b;

  DBG("Py-0\n");
  /* 元になる基本モーションを取得する */
  GetMotion(vow->mtn_vowel[cel->mtn_a], 0, &mtn_a);
  DBG("Py-1\n");
  GetMotion(vow->mtn_vowel[cel->mtn_b], 0, &mtn_b);
  DBG("Py-2\n");

  /* 基本モーションを cel で指定されているレートでブレンドする */
  if(vow->joints == FACE_JOINTS)
    BlendMotion(ret, &mtn_a, &mtn_b, cel->rate);
  else
    BlendBodyMotion(ret, &mtn_a, &mtn_b, cel->rate);
  DBG("Py-3\n");

  return ret;
}

static FACE_MOTION * make_motion(FACE_MOTION * ret, vmtnVowel * vowel,
				 mtn_cel * cel1, mtn_cel * cel2, float rate)
{
  FACE_MOTION mtn1;
  FACE_MOTION mtn2;

  DBG("Px-0\n");
  /* cel1 のモーションを mtn1 として作成する */
  get_cel_motion(&mtn1, cel1, vowel);

  DBG("Px-1\n");
  /* cel2 のモーションを mtr2 として作成する */
  get_cel_motion(&mtn2, cel2, vowel);

  DBG("Px-2\n");
  /* mtn1 と mtn2 を、rate で補間した結果を求める */
  if(vowel->joints == FACE_JOINTS)
    BlendMotion(ret, &mtn1, &mtn2, rate);
  else
    BlendBodyMotion(ret, &mtn1, &mtn2, rate);

  DBG("Px-3\n");
  return ret;
}



/*
 * 指定された音素における、指定されたフレームのモーションを取得する。
 */
FACE_MOTION * mkMakeMotion(FACE_MOTION * ret,
			   lipMotionList * list,
			   vmtnVowel * vowel,
			   int pre_id, int id,
			   float frame,
			   float total_frame)
{
  mtn_voice * mtn;
  mtn_cel   * cel;
  mtn_voice * pre_mtn;
  mtn_cel   * pre_cel;
  float       d_time, m_time;
  int i, j;

  DBG("frame = %8.3f,  total_frame = %8.3f\n", frame, total_frame);
  /* 音素 ID が正しいかどうかをチェックする */
  mtn = NULL;
  for(i = 0; i < list->voices; i++)
    if(id == list->mtn[i].vid)
      {
	mtn = &list->mtn[i];
	break;
      }

  pre_mtn = NULL;
  for(i = 0; i < list->voices; i++)
    if(pre_id == list->mtn[i].vid)
      {
	pre_mtn = &list->mtn[i];
	break;
      }
  
  /* リストに指定された音素 ID が含まれていなければ、エラー */
  if(NULL == mtn)
    {
      fprintf(stderr, "unknown voice element .");
      return NULL;
    }
  if(NULL == pre_mtn)
    {
      fprintf(stderr, "unknown voice element .");
      return NULL;
    }

  /* 音素内での時系列を正規化する */
  d_time = frame / total_frame;
  DBG("d_time = %8.3f\n", d_time);
  cel = NULL;
  for(i = 0; mtn->mtn[i].time < 1.0F; i++)
    if(d_time >= mtn->mtn[i].time)
      {
	cel = &mtn->mtn[i];
	/* 直前のモーションに相当する cel のポインタを得る */
	if(i == 0)
	  {
	    DBG("Case A\n");
	    /* 今回の音素最初のモーションである場合は、前回の音素の
	       最終モーションを採用する。 */
	    for(j = 0; pre_mtn->mtn[j].time < 1.0; j++)
	      pre_cel = &pre_mtn->mtn[j];
	  }
	else
	  {
	    DBG("Case B\n");
	    /* 今回の音素で二番目以降である場合は、直前の音素を採用する */
	    pre_cel = &mtn->mtn[i - 1];
	  }
      }
  DBG("P-0\n");
  /*
   * cel が NULL の場合は、frame が total_frame を追い抜いてしまっているか、
   * モーション表がおかしいのでエラー。
   */
  if(NULL == cel) return NULL;

  DBG("P-1\n");
  /*
   * そのモーションの時間内での正規化時系列を求める
   */
  {
    float cel_total;
    float pos_time;

    DBG("P-2\n");
    cel_total = (cel + 1)->time - cel->time;
    pos_time  = d_time - cel->time;

    m_time = pos_time / cel_total;
  }

  DBG("P-3\n");

  /* 直前のモーションと、今回のモーションを、
     m_time をレートとしてブレンドした結果を返す */
  return make_motion(ret, vowel, pre_cel, cel, m_time);
}

FACE_MOTION * mkMotionPower(FACE_MOTION * ret,
			    vmtnVowel * vowel,
			    FACE_MOTION * mtn, float power)
{
  FACE_MOTION nmtn;

  GetMotion(vowel->mtn_vowel[VOWEL_S], 0, &nmtn);
  if(vowel->joints == FACE_JOINTS)
    BlendMotion(ret, &nmtn, mtn, power);  
  else
    BlendBodyMotion(ret, &nmtn, mtn, power);

  return ret;
}

/*
 * 音素ラベルを、音素 ID に変換する
 */
int mkLabel2ID(lipMotionList * list,  char *label)
{
  int i;

  for(i = 0; i < list->voices; i++)
    if(!strcmp(label, list->mtn[i].label))
      return list->mtn[i].vid;

  return -1;   /* 見付からない場合はエラー */
}

