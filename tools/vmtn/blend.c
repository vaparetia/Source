/*
 * 音素時系列をモーションデータに変換
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "mtnlib.h"  /* モーション関連のヘッダファイル */


#include "vmotion.h"
#include "scedule.h"
#include "mtnfunc.h"
#include "voice.h"

#define _blend_c_
#include "blend.h"


/* -------------------------------------------------------------------------
 *                               Prototype
 * ------------------------------------------------------------------------- */
static void SetMotion( MTN_MOTION *motion, int frame, FACE_MOTION *res );
static void GetMotion( MTN_MOTION *motion, int frame, FACE_MOTION *res );
static void BlendMotion( FACE_MOTION *res,
			FACE_MOTION *m0, FACE_MOTION *m1, float t );


/* 与えられた母音モーション群と、音素時系列から、口パクモーションを生成する */
MTN_MOTION * bldCreateMotion(scScedule **sc_list, vmtnVowel *vowel)
{
  MTN_MOTION *ret_motion;  /* 生成されるモーションを格納するモーション構造体 */
  FACE_MOTION   pre_face;  /* 直前の音素に対応するモーションフレームデータ   */
  FACE_MOTION   new_face;  /* 処理中の音素をあらわすモーションフレームデータ */
  FACE_MOTION   tmp_face;  /* パワーを考慮しないモーションフレームデータ     */
  FACE_MOTION   nul_face;  /* 平常時のモーションフレームデータ               */
  FACE_MOTION   fix_face;  /* 処理結果格納用のモーションフレームデータ領域   */
  int    i, id;
  int    video_term;      /* 音声時系列から求められるモーションのフレーム数  */
  int    voice_term;      /* 音声時系列のフレーム数                          */
  int    video_t;         /* 画面のフレーム番号                              */
  double voice_t;         /* 音声側の論理フレーム番号
			     (画面とは基準が違うため、小数点以下まで求める   */
  double vt;              /* 音素セグメント内の時刻                          */
  double blend;           /* blend率                                         */
  int    T, vpos;
  int    f;               /* 音素セグメント内での画面フレーム                */
  int    vf;
  int    repeat;
  /* 音声時系列のフレーム数を求める(最後の無音部の時刻) */
  for(i = 0; sc_list[i] != NULL; i++) voice_term = sc_list[i]->t;

  /* 音声と同じだけの時間をあらわす、画面のフレーム数を求める */
  video_term = (double)SEC_VIDEO * (0.5 + voice_term) / SEC_VOICE
    + CLOSE_SPEED;

  /* 画面フレーム分の長さのモーション領域を作成する */
  ret_motion = MTN_AllocMotion(FACE_JOINTS, video_term);
  if(NULL == ret_motion) return NULL;

  /* 初期状態では無音として、モーションフレームデータを取得しておく */
  GetMotion(vowel->mtn_vowel[VOWEL_S], 0, &pre_face);
  GetMotion(vowel->mtn_vowel[VOWEL_S], 0, &nul_face);

  /*
   * 再生時間にわたり、くまなくモーションを作成する。
   */
  vpos = 0;
  f = 0;
  for(video_t = 0; video_t < video_term; video_t++)
    {
      fprintf(stderr, "1");
      /* 画面フレームの表示時刻に該当する音声側のフレーム値を求める   */
      voice_t = (double)SEC_VOICE * (double)video_t / SEC_VIDEO;

      /* その音声フレームにおける、音素とセグメント内経過時刻を求める */
      for(i = vpos; sc_list[i] != NULL; i++)
	if(sc_list[i]->t < voice_t)
	  {
	    /* 音素境界を乗り越えたら直前のモーションを変更する */
	    if(vpos != i)
	      {
		id = voiLabel2ID(sc_list[vpos]->label);
		/* 直前が /wa/ だったら/a/ として扱う */
		do {
		  repeat = 0;
		  if(id == VOWEL_uA) id = VOWEL_A;
		  if(id == VOWEL_unknown)
		    {
		      if(VOWEL_unknown ==
			 (id = voiLabel2ID(sc_list[i]->label)))
			id = VOWEL_U;
		      repeat = 1;
		    }
		} while(repeat);
		GetMotion(vowel->mtn_vowel[id], 0, &pre_face);
		f = -1;
	      }
	    vt = voice_t - (double)sc_list[i]->t;
	    vpos = i; /* 次回の探索開始個所(現在処理中の音素でもある) */
	    f++;
	  }
	else
	  {
	    T = sc_list[i]->t - sc_list[vpos]->t;  /* 音素セグメントの長さ*/
	    break;
	  }

      fprintf(stderr, "2");
      /* 音素に該当するモーションのフレームデータを取得 */
      id = voiLabel2ID(sc_list[vpos]->label);  /* 音素に該当する口の形のID */
      do {
	repeat = 0;
	switch(id)
	  {
	  case VOWEL_uA:       /* 半母音用の特殊モーション */
	    blend = 2 * mtfBlendRate(vt, T);
	    if(blend < 1.0) /* この段階では、/u/ を目標に変動する */
	      {
		/* 0フレーム目を取得 */
		GetMotion(vowel->mtn_vowel[VOWEL_U], 0, &new_face);
		
		/* 求めたblend率で、直前の音素に該当するモーションとの
		   中間モーションを作成 */
		BlendMotion(&tmp_face, &pre_face, &new_face, blend);
	      }
	    else            /* ここからは /a/ を目標に変動する    */
	      {
		/* 強制的に、直前のモーションを /u/ にする */
		vf = (T == 0) ? 0 : (f * vowel->mtn_len[VOWEL_U] / T);
		GetMotion(vowel->mtn_vowel[VOWEL_U], vt, &pre_face);
		
		/* /a/ の フレームを取得 */
		vf = (T == 0) ? 0 : (f * vowel->mtn_len[VOWEL_A] / T);
		GetMotion(vowel->mtn_vowel[VOWEL_A], vt, &new_face);
		
		/* 求めたblend率で、直前の音素に該当するモーションとの
		   中間モーションを作成 */
		BlendMotion(&tmp_face, &pre_face, &new_face, blend - 1.0);
	      }
	    break;
	  case VOWEL_unknown:  /* 後続音と同じ形状。
				  後続もVOWEL_unknownであれば、
				  /u/ の形状を取る */
	    if(sc_list[vpos + 1] != NULL)
	      {
		if(VOWEL_unknown == (id = voiLabel2ID(sc_list[vpos + 1]->label)))
		  id = VOWEL_U;
	      }
	    else
	      id = VOWEL_U;
	    repeat = 1;
	    break;
	  default:
	    vf = (T == 0) ? 0 : (f * vowel->mtn_len[id] / T);
	    GetMotion(vowel->mtn_vowel[id], vf, &new_face);
	    
	    /* 音素セグメント内の経過時間から、blend率を求める */
	    blend = mtfBlendRate(vt, T);
	    /* 経過時間は、小数点以下までありうる。*/
	    
	    /* 求めたblend率で、直前の音素に該当するモーションとの
	       中間モーションを作成 */
	    BlendMotion(&tmp_face, &pre_face, &new_face, blend);
	    break;
	  }
      } while(repeat);

      fprintf(stderr, "3");

      /* パワーに従って、モーションの大きさを抑える。 */
      BlendMotion(&fix_face, &nul_face, &tmp_face, sc_list[vpos]->power);
      /* 出力モーションに設定 */
      SetMotion(ret_motion, video_t, &fix_face);
      fprintf(stderr, "4\n");
    }


  /* 作成したモーションを戻り値としてリターン */

  return ret_motion;  
}

/* -------------------------------------------------------------------------
 * 指定したフレームのモーションを取得
 * ------------------------------------------------------------------------- */
static void GetMotion( MTN_MOTION *motion, int frame, FACE_MOTION *res )
{
  MTN_GetMotionData( motion, frame, NULL, res->trans, res->rots );
}

/* -------------------------------------------------------------------------
 * 指定したフレームのモーションを設定
 * ------------------------------------------------------------------------- */
static void SetMotion( MTN_MOTION *motion, int frame, FACE_MOTION *res )
{
  MTN_SetMotionData( motion, frame, NULL, res->trans, res->rots );
}

/* -------------------------------------------------------------------------
 * ２つのモーションの中間モーションを生成
 * ------------------------------------------------------------------------- */
static void BlendMotion( FACE_MOTION *res,
			FACE_MOTION *m0, FACE_MOTION *m1, float t )
{
  int i ;
  
  for(i = 0; i < FACE_JOINTS; i++)
    {
      /* 移動成分の補間処理 */
      GTE_InterpolateVector(&res->trans[ i ],
			    &m0->trans[ i ], &m1->trans[ i ], t );
      /* 回転成分の補間処理 */
      MT_QuatSlerp(&res->rots[ i ], &m0->rots[ i ], &m1->rots[ i ], t);
    }
}
