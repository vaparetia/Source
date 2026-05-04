#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _label_c_
#include "analyze.h"
#include "label.h"
#include "text.h"
#include "util.h"

#ifdef _LABEL_DEBUG_
#ifndef _DEBUG_
#define _DEBUG_
#endif /* _DEBUG_ */
#endif /* _LABEL_DEBUG_ */


#define ABS(n)  (((n) < 0) ? -(n) : (n))

static double lab_distance(double *a, double *r, int M);
static double lab_distance_WVCUF(anaParam **p_list, int * idx, double **R);
static double lab_distance_X(anaParam **p_list, int * idx, double **R);
static double lab_distance_SV(anaParam **p_list, int * idx, double **R);
static double lab_distance_CWUW(anaParam **p_list, int * idx, double **R);
static void lab_set_group(anaParam **p_list, int top, int len);
static labVList * lab_create_border_list(double skip_time,
					 anaParam ** p_list, int nums);
static int lab_group2num(vGroup g);
static int lab_cmp_group(vGroup sample, vGroup cmp);
static int lab_check_group(txtPhono *tx, vGroup g);
static lab_labeling(double skip_time,
		    txtPhono **t_list, anaParam **p_list, int nums);
static int lab_matching(txtPhono ** t_list, int t_top, int t_len,
			labVList *  v_list, int v_top, int v_len);



static double lab_distance(double *a, double *r, int M)
{
  int m;
  double ans = 0.0;
  double x;

  for(m = 0; m < M; m++)
    {
      x = a[m] - r[m];
      ans += x * x;
    }
  return 4.343 * sqr(ans);
}

/* =========================================================================
 * リファレンスパターンとの距離計算関数群
 *
 * 各音素セグメントのタイプに併せて、特徴ベクトルとリファレンスパターンとの
 * 距離を計算する。
 *
 * 注意事項: 
 *   原則として計算に用いる入力フレームは 5フレーム分であるが、
 *   計算の都合上、前後各1フレーム分を含む、計7フレーム分を idx にセット
 *   しておくこと。
 * ========================================================================= */

/* W, V, C, U, F 型の D(A, R) を求める */
static double lab_distance_WVCUF(anaParam **p_list, int * idx, double **R)
{
  double D;
  double d[5];
  int i;

  for(i = 0; i < 5; i++)
    {
      d[i] = lab_distance(p_list[idx[i + 1]]->gi, R[i], MEL_CEPSTRUM_M);
      d[i] *= d[i];
    }

  d[1] *= 2;
  d[2] *= 2;
  d[3] *= 2;

  D = 0;
  for(i = 0; i < 5; i++) D += d[i];

  return sqrt(D / 8);
}

/* X 型 */
static double lab_distance_X(anaParam **p_list, int * idx, double **R)
{
  double D;
  double d[5];
  int i;

  for(i = 0; i < 5; i++)
    {
      d[i] = lab_distance(p_list[idx[i + 1]]->gi, R[i], MEL_CEPSTRUM_M);
      d[i] *= d[i];
    }

  D = 0;
  for(i = 0; i < 5; i++) D += d[i];

  return sqrt(D / 5);
}

/* SV 型 */
static double lab_distance_SV(anaParam **p_list, int * idx, double **R)
{
  int i, j, k;
  double D[3];
  double d[5];
  double DD;

  for(j = 0; j < 3; j++)
    {
      k = j - 1;
      for(i = 0; i < 5; i++)
	{
	  d[i] = lab_distance(p_list[idx[i + k]]->gi, R[i], MEL_CEPSTRUM_M);
	  d[i] *= d[i];
	}
      D[j] = 0;
      for(i = 0; i < 5; i++) D[j] += d[i];
      D[j] = sqrt(D[j] / 5);
    }
  DD = D[0];
  for(j = 1; j < 3; j++) if(DD > D[j]) DD = D[j];
  return DD;    
}

/* CW, UW 型 */
static double lab_distance_CWUW(anaParam **p_list, int * idx, double **R)
{
  int i, j, k;
  double D[3];
  double d[5];
  double DD;
  
  for(j = 0; j < 3; j++)
    {
      k = j - 1;
      for(i = 0; i < 5; i++)
	{
	  d[i] = lab_distance(p_list[idx[i + k]]->gi, R[i], MEL_CEPSTRUM_M);
	  d[i] *= d[i];
	}
      
      d[1] *= 2;
      d[2] *= 2;
      d[3] *= 2;
      
      D[j] = 0;
      for(i = 0; i < 5; i++) D[j] += d[i];
      D[j] = sqrt(D[j] / 8);
    }
  DD = D[0];
  for(j = 1; j < 3; j++) if(DD > D[j]) DD = D[j];
  return DD;    
}

/*
 * ラベリングモジュールの初期化
 */
void labInit(void)
{
  labP.VminAi = LBL_V_minAi;
  labP.VmaxFi = LBL_V_maxFi;
  labP.VminFi = LBL_V_minFi;
  labP.VmaxEi = LBL_V_maxEi;
  labP.NmaxFi = LBL_N_maxFi;
  labP.NmaxZi = LBL_N_maxZi;
}


/*
 * 音素群の判別
 */
static void lab_set_group(anaParam **p_list, int top, int len)
{
  int i;
  double maxZi, maxFi, maxEi, minAi, minFi;
  anaParam *param;

  /*
   * 音節内における、Zi, Ei の最大値と、Ai の最小値を求めておく
   */
  maxZi = p_list[top]->Zi;
  maxFi = p_list[top]->Fi;
  maxEi = p_list[top]->Ei;
  minAi = p_list[top]->Ai;
  minFi = maxFi;
  for(i = 1; i < len; i++)
    {
      param = p_list[top + i];
      if(param->Zi > maxZi) maxZi = param->Zi;
      if(param->Fi > maxFi) maxFi = param->Fi;
      if(param->Ei > maxEi) maxEi = param->Ei;
      if(param->Ai < minAi) minAi = param->Ai; 
      if(param->Fi < minFi) minFi = param->Fi;
    }
  
  if(p_list[top]->status & ST_VOICED)           /* 有声音として扱う */
    {
      int fC, fW;

      p_list[top]->group = V_V;
      fC = fW = 0;
      if((minAi < labP.VminAi) || (minFi < labP.VminFi)) fW = 1;
      if((maxEi > labP.VmaxEi) || (maxFi > labP.VmaxFi)) fC = 1;

      /* 両方の条件を満たしているときは、分類 V のままリターン */
      if(fW && fC) return;
      if(fW) p_list[top]->group = V_W;
      if(fC)
	{
	  p_list[top]->group = V_C;
	  if(maxFi > labP.VmaxFi) p_list[top]->group = V_D;
	}
    }
  else if(!(p_list[top]->status & ST_SILENT))   /* 無声音として扱う */
    {
      p_list[top]->group = V_U;
      if(maxZi > labP.NmaxZi) p_list[top]->group = V_F;
    }
  else                                          /* 無音部として扱う */
    p_list[top]->group = V_S;
 
}
     
/*
 * 音素ラベリングを行なう
 */
int labLabeling(double skip_time,
		txtPhono ** tx_list, anaParam **p_list, int nums)
{
  int t;
  int i, j, k;
  int top, len;

  for(t = 0; t < nums; t++)
    {
      /* 音素境界でなければ処理をスキップ */
      if(!IsVoiceSegment(p_list[t]->status) &&
	 !(p_list[t]->status & ST_SILENT)) continue;

      /* 無音状態であれば、次の有音部までをセグメントとする */
      if(p_list[t]->status & ST_SILENT)
	{
	  p_list[t]->group = V_S;
	  continue;
	}

      /*
       * 音素領域についての情報をまとめる
       */
      /* 次の音素境界までのフレーム数をカウントする */
      top = t;
      for(i = t + 1; i < nums; i++)
	if(IsVoiceSegment(p_list[i]->status) ||
	   (p_list[i]->status & ST_SILENT)) break;
      len = i - top;

      /* 音素群を判別する */
      lab_set_group(p_list, top, len);
    }

  /*
   * 音素群ラベルをたよりに、音素列化されたテキスト文字列を、
   * 時系列に当てはめ,口パクのシーケンスに必要な出力を作り出す。
   */
  lab_labeling(skip_time, tx_list, p_list, nums);

  return 0;
}

/*
   基本的に最大の処理単位は、「無音から無音までの間」である。

   例)
      テキストから得られる情報:

        音素列: (*)  s u n e e k u  *  s o k o wa ...
        音素群: (S)  F W C W W F W  S  F W F W W  ...

	※ '*'は、無音部分があることを示す。テキストの先頭には、
	   明記されない無音部分があると考えられる。

      音声から得られる情報

        音素境界: |   S   |U| U | W |C|C|W| W | F | U |  S  |


      この二つの情報を元に、各音素の位置についてあたりをつける。


      音素境界        |   S   |U| U | W |C|C|W| W | F | U |  S  |

      音素列割当結果  |   *   |  s  | u | n |e| e |   k   |  *  |

      ※ 音素群 U は音素群 F を含む。
      ※ 無声摩擦音を子音とする発音では、母音が無声化することがある。

      以上のように、元々何を言っているのかが判明している場合、
      巨大なリファレンス用データベースは(ほぼ)不要である。



   音素の脱落について

      音声認識の都合上、どうしても欠落する音素がある。母音ですら、
      無声化することで、母音的セグメントとしては認識できないことがある。

      

*/









/*
 * テキスト文字列を参照しながら、各音素境界に対してあたりをつけ、
 * ラベリングを行なう(音素境界の欠落、挿入等も考慮する)
 */
static int lab_labeling(double skip_time,
			txtPhono **t_list, anaParam **p_list, int nums)
{
  int t;
  int add;
  labLattice * lattice;
  void * vpt;
  int lat_nums;
  int l_top;
  int t_top;
  int i, j, k;


  lat_nums = 0;
  lattice = NULL;
  /* 音素境界のリストを作成 */
  for(t = 0; t < nums; t++)
    {
      add = 0;

      /* 無音区間、無声音区間の立ち上がりを捕らえたら、
	 それも音素境界として考える */

      /* 最初のフレームは、ほぼ間違いなく無音区間の立ち上がりと考える */
      if(!t) add = 1;
      else if(!(p_list[t - 1]->status & ST_SILENT) &&
	      (p_list[t]->status & ST_SILENT)) add = 1;

      /* その他、音素境界であれば記録する */
      if(IsVoiceSegment(p_list[t]->status)) add = 1;

      if(!add) continue;

      if(NULL == (vpt = utilAddBuf(lattice, sizeof(labLattice), lat_nums)))
	{
	  free(lattice);
	  return -1;
	}
      lattice = vpt;

      lattice[lat_nums].t       = t;
      lattice[lat_nums].group   = p_list[t]->group;
      lattice[lat_nums].sc_nums = 0;
      lattice[lat_nums].score   = NULL;
      
      lat_nums++;
    }

  /* 各音素セグメントの長さを求め、格納する */
  for(t = 0; t < lat_nums - 1; t++)
    lattice[t].dt = lattice[t + 1].t = lattice[t].t;
  lattice[t].dt = nums = lattice[t].t;  /* 最後の音素セグメントの長さは、
					   音声データの末尾まで          */


  /* 無音から無音のブロックに分割し、その音素時系列表を出力する */
  l_top = 0;
  t_top = 0;
  while(l_top < lat_nums)
    {
      /* 無音部を読み飛ばす */
      while(lattice[l_top].group == V_S) l_top++;
      j = 1;
      while(j)
	{
	  j = 0;
	  for(i = 0; i < t_list[t_top]->gr_nums; i++)
	    if(t_list[t_top]->group[i] == V_S)
	      {
		t_top++;
		j = 1;
		break;
	      }
	}

      /* 有音部の長さを計る */
      l_len = 0;
      while(lattice[l_top + l_len].group != V_S) l_len++;

      /* 発話単位の音素数をカウントする */
      t_len = 0;
      j = 1;
      while(j)
	{
	  j = 0;
	  for(i = 0; i < t_list[t_top + t_len]->gr_nums; i++)
	    {
	      t_len++;
	      j = 1;
	      break;
	    }
	}
      
      /* 無音部に挟まれた有音部のラベリングを行なう */      
      
      
      
      l_top += l_len;
    }

}

/*
 * 有音部のラベリングを行なう
 */
static lab_matching(labLattice * lattice, int l_top, int l_len,
		    txtPhono ** t_list, int t_top, t_len)
{
  
}
