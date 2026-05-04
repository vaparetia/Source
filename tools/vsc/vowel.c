/*
 * 音素セグメントの内容が母音であると判断された場合、
 * 特徴ベクトルを求め、母音としては何に近いかを判定する。
 *
 * /a/, /i/, /u/, /e/, /o/ の各標本特徴ベクトルのうち、
 * もっとも近いものが何であるかを判定。
 *
 * 男声の場合、女声の場合の双方を判定する。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define _vowel_c_
#include "analyze.h"
#include "pcmdata.h"
#include "vowel.h"
#include "formant.h"
#include "mel.h"
#include "debug.h"

#define ABS(n)   (((n) < 0) ? -(n) : (n))

static int fftN = 512;

void vowelInit(void)
{
  /* 男声初期値 */
  vecMale.vA.formant_1st = VOW_M_1st_A;
  vecMale.vA.formant_2nd = VOW_M_2nd_A;
  
  vecMale.vI.formant_1st = VOW_M_1st_I;
  vecMale.vI.formant_2nd = VOW_M_2nd_I;

  vecMale.vU.formant_1st = VOW_M_1st_U;
  vecMale.vU.formant_2nd = VOW_M_2nd_U;

  vecMale.vE.formant_1st = VOW_M_1st_E;
  vecMale.vE.formant_2nd = VOW_M_2nd_E;

  vecMale.vO.formant_1st = VOW_M_1st_O;
  vecMale.vO.formant_2nd = VOW_M_2nd_O;

  /* 女声初期値 */
  vecFemale.vA.formant_1st = VOW_F_1st_A;
  vecFemale.vA.formant_2nd = VOW_F_2nd_A;
  
  vecFemale.vI.formant_1st = VOW_F_1st_I;
  vecFemale.vI.formant_2nd = VOW_F_2nd_I;

  vecFemale.vU.formant_1st = VOW_F_1st_U;
  vecFemale.vU.formant_2nd = VOW_F_2nd_U;

  vecFemale.vE.formant_1st = VOW_F_1st_E;
  vecFemale.vE.formant_2nd = VOW_F_2nd_E;

  vecFemale.vO.formant_1st = VOW_F_1st_O;
  vecFemale.vO.formant_2nd = VOW_F_2nd_O;
}

/*
 * 二つのベクトル間の距離を求める
 */
static int vowel_distance(VoiceVector * target, VoiceVector * sample)
{
  double f1, f2;
  double s1, s2;
  double dist;

  f1 = target->formant_1st;
  f2 = target->formant_2nd;

  s1 = melFreq2MelFreq(sample->formant_1st, fftN);
  s2 = melFreq2MelFreq(sample->formant_2nd, fftN);

  DEBUG("sample: %8.3f %8.3f\n", sample->formant_1st, sample->formant_2nd);
  DEBUG("mel   : %8.3f %8.3f  ", s1, s2);
  DEBUG("target: %8.3f %8.3f\n", f1, f2);
  /* サンプルとなる点との直線距離を求める */
  f1 = ABS(f1 - s1) * 2;
  f2 = ABS(f2 - s2);
  dist = sqrt(f1 * f1 + f2 * f2);
  DEBUG("dist = %f\n", dist);
  return dist;
}

/*
 * 指定された母音ベクトルセットとの距離を比較し、
 * どの母音が一番近いかを割出す。
 */
int vowel_judge(vowelDistance *ret, VectorSet *set, VoiceVector *vec)
{
  int i, best_id;
  double best_dist;
  double dist[5];

  dist[0] = vowel_distance(vec, &(set->vA));
  dist[1] = vowel_distance(vec, &(set->vI));
  dist[2] = vowel_distance(vec, &(set->vU));
  dist[3] = vowel_distance(vec, &(set->vE));
  dist[4] = vowel_distance(vec, &(set->vO));

  best_dist = dist[0];
  best_id = 0;
  for(i = 1; i < 5; i++)
    if(dist[i] < best_dist)
      {
	best_dist = dist[i];
	best_id   = i;
      }
  ret->id       = best_id;
  ret->distance = best_dist;

  return best_id;
}

/*
 * 母音識別を行なう
 */
int vowelJudge(anaParam **p_list, int nums, int begin, int end,
	       vowelDistance * male, vowelDistance *female)
{
  static char * list[] = {"a", "i", "u", "e", "o"};
  VoiceVector vec_total;
  double totalspectrum[ FORMANT_BUF ];
  double *spectrum;
  double formant[5];
  double bestEi;
  int bestF;
  int i, j, k;

  DEBUG("enter vowelJudge()\n");
#if 0
 
  /* スペクトル的に最も安定したフレームを見つけ出す */
  bestEi = ABS(p_list[begin]->Ei);
  bestF = begin;
  for(i = begin; i < end; i++)
    if(ABS(p_list[i]->Ei) < bestEi)
      bestEi = ABS(p_list[i]->Ei), bestF = i;
#else
  /* 音素セグメントの中心のフレームを基準とする */
  bestF = (begin + end) / 2;
#endif

  DEBUG("-step 1\n");

  /* 見付けたフレーム周辺のスペクトル包絡を使用する */
  for(i = 0; i < FORMANT_BUF; i++) totalspectrum[i] = 0;
#if 0
  for(i = -2; i <= 2; i++)
    {
      k = i * 2 + bestF;
      if(k < begin) continue;
      if(k >= end) continue;
      for(j = 0; j < FORMANT_BUF; j++)
	totalspectrum[j] += p_list[k]->spectrum[j];
    }
#else
  for(j = 0; j < FORMANT_BUF; j++)
    totalspectrum[j] = p_list[bestF]->spectrum[j];
#endif

  /* スペクトル包絡の微細な差を丸め、誤差が反映されないようにする */
  for(j = 0; j < FORMANT_BUF; j++)
    totalspectrum[j] = (double)(int)((totalspectrum[j] + 0.05) * 10) / 10;

  DEBUG("-step 2\n");

  /* フォルマントを抽出 */
  fmtGetFormant(formant, 5, totalspectrum, fftN);

  DEBUG("-step 3\n");

  vec_total.formant_1st = formant[0];
  vec_total.formant_2nd = formant[1];

  /* フォルマントが一つしか検出できなければ、エラーとする */
  if(formant[1] == 0.0) goto err_entry;
  
  vowel_judge(male,   &vecMale,   &vec_total);
  vowel_judge(female, &vecFemale, &vec_total);



  
  DEBUG("-step 4\n");
  /*
    fprintf(stderr, "%3d-%3d: (%8.3f, %8.3f)  M:[%s](%8.3f) / F:point[%s](%8.3f)\n",
    begin, end,
    vec_total.formant_1st, vec_total.formant_2nd,
    list[male->id], male->distance,
    list[female->id], female->distance);
  */
  return 0;

 err_entry:
  male->id = female->id = -1;
  return -1;
}
