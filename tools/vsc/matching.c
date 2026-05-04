#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _matching_c_
#include "env.h"
#include "analyze.h"
#include "label.h"
#include "ref_load.h"
#include "matching.h"
#include "util.h"
#include "cepstrum.h"

#include "debug.h"

#define ABS(n)    (((n) < 0) ? -(n) : (n))

/*
 * 特徴ベクトル距離尺度関数(使用するものを一つだけ #if 0 ～ #endif の中より取り出して使用)
 */
#define DISTANCE(a, b)  cepDistanceEuclidian((a), (b))   /* ユークリッド距離 */
#if 0
#define DISTANCE(a, b)  cepDistanceHamming((a), (b))     /* ハミング距離     */
#define DISTANCE(a, b)  cepDistanceChebychev((a), (b))   /* チェビシェフ距離 */
#define DISTANCE(a, b)  cepDistanceNonlinear2D((a), (b)) /* 二次非線形距離   */
#endif


static 
matScore ** mat_create_lattice(anaParam ** p_list, /* フレームパラメータ   */
			       int p_nums,         /* 総フレーム数         */
			       int vIndex[7],      /* ベクトル抽出フレーム */
			       matchSubset *subset,/* 参照するサブセット群 */
			       int sub_nums);          /* サブセットの数       */


static int fftN;


/*
 * 与えられたラベルが、/s/, /i/, /u/ のいずれかであることを確認する
 */
static int mat_check_label(char *label)
{
  static char * list[] = { "s", "i", "u", NULL };
  int i;

  DEBUG("mat_check_label(\"%s\") entering...\n", label);
  if(label == NULL) return 0;
  /*
   * リストにあげられているうちの音素のいずれかであれば、非0 を返す
   */
  for(i = 0; list[i] != NULL; i++)
    if(!strcmp(list[i], label)) return 1;

  /* リスト中に同じ音素記号がなければ、0 を返す */
  DEBUG("mat_check_label() terminate.\n");
  return 0;  
}


/*
 * 補正用テキスト文字列を用いず,リファレンスパターンを用いて音韻識別を行う
 */
int matMatching(int skip, anaParam **p_list, int p_nums,
		labVList * v_list, int top, int len)
{
  int sub_nums;
  mSubset subset[3];
  vGroup preG; /* 先行音素セグメントのセグメントカテゴリ */
  vGroup nowG; /* 現行音素セグメントのセグメントカテゴリ */
  vGroup nxtG; /* 後続音素セグメントのセグメントカテゴリ */
  char * preLabel; /* 先行音素セグメントの最有力候補となったラベル */
  int vIndex[3][7];
  int segBegin, segEnd, segLen;
  int i, j, k;
  int seg, s;
  int center;
  matScore score;
  matScore ** sc_list[3];
  matchSubset sub[2];
  int sc_cnt, sub_cnt;

  DEBUG("*** Labeling Start ***(%d - %d)\n", top, top + len);

  fftN = 2, i = 1;
  while(fftN < FRAME_WINDOW) i++, fftN *= 2;

  preG = V_S;  /* 初期状態は無音 */
  preLabel = "sil";
  for(seg = 0; seg < len; seg++)
    {
      DEBUG("point 1\n");
      s = top + seg;

      segBegin = v_list[s].t - skip;       /* セグメント開始フレーム */
      segLen   = v_list[s].time;    /* セグメント長           */
      segEnd   = segBegin + segLen; /* セグメント終了フレーム */

      DEBUG("[a]\n");

      /* 現行音素セグメントのセグメントカテゴリを得る */
      nowG = v_list[s].group;

      /* 後続セグメントのセグメントカテゴリを得る */
      nxtG = (s + 1 >= len) ? V_unknown : v_list[s + 1].group;

      DEBUG("[b]\n");

      /*	
       * あらかじめ、3つのパターンの特徴ベクトル抽出フレームを割り出しておく
       */
      /* 全体型特徴ベクトル抽出フレームのリストアップ 
	 全体型特徴ベクトルは,音素セグメント全体を6等分した境界の
	 各フレームから取得する。*/
      {
	double step;

	step = segLen / 6;

	for(i = 0; i < 7; i++)
	  vIndex[0][i] = segBegin + (int)(step * i);
      }

      DEBUG("[c]\n");

      /* 定常部型特徴ベクトル抽出フレームのリストアップ
	 定常部型特徴ベクトルは,スペクトル的に最も安定した箇所周辺のフレームを
	 1フレームおきに取得する。 */
      {
	double bestEi;

	DEBUG("segBegin = %d\n", segBegin);
	DEBUG("segEnd = %d\n", segEnd);

	/* セグメント内でスペクトル的に最も安定したフレームを探す */
	bestEi = ABS(p_list[segBegin]->Ei);
	center = segBegin;
	for(i = segBegin; i < segEnd; i++)
	  {
	    DEBUG("i = %d\n", i);
	    /* スペクトル包絡時間変化の絶対値が最も小さなフレームが,
	       最もスペクトル的に安定している。 */
	    if(bestEi > ABS(p_list[i]->Ei))
	      {
		bestEi = ABS(p_list[i]->Ei);
		center = i;
	      }
	  }
	
	/* そのフレームを中心に,周辺フレームをベクトル抽出フレームとして登録 */
	for(i = 0; i < 7; i++)
	  {
	    k = center + (i - 3) * 2;
	    if(k < 0) k = 0;
	    if(k >= p_nums) k = p_nums - 1;
	    vIndex[1][i] = k;
	  }
      }
      
      DEBUG("[d]\n");

      /* 過渡部型特徴ベクトル抽出フレームのリストアップ
	 過渡部型特徴ベクトルは,セグメントの最終フレームを中心として、
	 その前後を1フレームおきに取得したもの。 */
      {
	int center;

	center = segEnd - 1;

	/* そのフレームを中心に,周辺フレームをベクトル抽出フレームとして登録 */
	for(i = 0; i < 7; i++)
	  {
	    k = center + (i - 3) * 2;
	    if(k < 0) k = 0;
	    if(k >= p_nums) k = p_nums - 1;
	    vIndex[2][i] = k;
	  }
      }

      DEBUG("point 2\n");

      /*
       * この段階で, vIndexにはそれぞれ、全体型,定常部型, 過渡部型の
       * ベクトル抽出フレーム番号が登録されている。
       *
       *   vIndex[0]   全体型
       *   vIndex[1]   定常部型
       *   vIndex[2]   過渡部型
       */


      /*
       * 音素ラティスを作成
       */
      sc_list[0] = sc_list[1] = sc_list[2] = NULL;
      sc_cnt = 0;
      switch(nowG)
	{
	case V_W:    /* 母音的セグメント       */
	  {
	    DEBUG("Seg: V_W\n");
	    sub[0] = SS_W;
	    sub_cnt = 1;

	    if((preG == V_F) && mat_check_label(v_list[s-1].label))
	      {
		sub[1] = SS_C;
		sub_cnt++;
	      }

	    sc_list[0] = mat_create_lattice(p_list, p_nums,
					    vIndex[1], sub, sub_cnt);
	    sc_cnt = 1;
	  }
	  break;

	case V_C:    /* 有声子音セグメント     */
	case V_D:    /* 有声子音的セグメント   */
	  {
	    DEBUG("Seg: V_C / V_D\n");
	    sub[0] = SS_C;
	    sub_cnt = 1;
	    if(v_list[s].group == V_C && center == segBegin)
	      {
		sub[1] = SS_W;
		sub_cnt++;
	      }

	    sc_list[0] = mat_create_lattice(p_list, p_nums,
					    vIndex[1], sub, sub_cnt);


	    sub[0] = SS_CW;
	    sub_cnt = 1;
	    sc_list[1] = mat_create_lattice(p_list, p_nums,
					    vIndex[2], sub, sub_cnt);

	    sc_cnt = 2;

	  }
	  break;
	case V_V:    /* 有声音的セグメント     */
	  {
	    DEBUG("Seg: V_V\n");

	    /* 定常部型 ref = (W, C)*/
	    sub[0] = SS_W, sub[1] = SS_C, sub_cnt = 2;
	    sc_list[0] = mat_create_lattice(p_list, p_nums,
					    vIndex[1], sub, sub_cnt);

	    /* 過渡部型 ref = (CW) */
	    sub[0] = SS_CW, sub_cnt = 1;
	    sc_list[1] = mat_create_lattice(p_list, p_nums,
					    vIndex[2], sub, sub_cnt);
	    sc_cnt = 2;
	  }
	  break;

	case V_U:    /* 無声摩擦音的セグメント */
	case V_F:    /* 無声子音的セグメント   */
	  {
	    DEBUG("Seg: V_U/V_F\n");

	    /* 定常部型 */
	    sub[0] = SS_U, sub_cnt = 1;
	    sc_list[0] = mat_create_lattice(p_list, p_nums,
					    vIndex[1], sub, sub_cnt);

	    /* 過渡部型 */
	    sub[0] = SS_UW, sub_cnt = 1;
	    sc_list[1] = mat_create_lattice(p_list, p_nums,
					    vIndex[2], sub, sub_cnt);

	    /* 全体型 */
	    sub_cnt = 1;
	    sub[0] = (s == (len - 1)) ? SS_X : SS_F;
	    sc_list[2] = mat_create_lattice(p_list, p_nums,
					    vIndex[0], sub, sub_cnt);

	    sc_cnt = 3;
	  }
	  break;
	case V_S:    /* 無音セグメント         */
	  {
	    DEBUG("Seg: V_S\n");

	    sub[0] = SS_SV; sub_cnt = 1;
	    sc_list[0] = mat_create_lattice(p_list, p_nums,
					    vIndex[2], sub, sub_cnt);
	    sc_cnt = 1;
	  }
	  break;
	}

      if(!sc_cnt) continue;
      DEBUG("point 3\n");

      /* 完成した音素ラティスをチェックし、
	 スコアの最も高い音素を割り当てる */
      {
	double best;
	char *label;
	
	best = 0;
	label = NULL;
	for(i = 0; i < sc_cnt; i++)
	  if(sc_list[i][0] != NULL)
	    if(best < sc_list[i][0]->score)
	      {
		best = sc_list[i][0]->score;
		label = sc_list[i][0]->label;
	      }

	/* 割り当てられた音素ラベルを登録 */
	if((label == NULL) ||
	   (p_list[v_list[s].t]->status & ST_SILENT)) label = "sil";
	v_list[s].label = label;
      }

      DEBUG("point 4\n");
      DEBUG("sc_cnt = %d\n", sc_cnt);
      for(i = 0; i < sc_cnt; i++)
	if(sc_list[i] != NULL)
	  {
	    DEBUG("sc_list[%d] is not NULL.\n", i);
	    for(j = 0; sc_list[i][j] != NULL; j++) free(sc_list[i][j]);
	    free(sc_list[i]);
	  }
      preG = nowG;
      DEBUG("point 5\n");
    }

  DEBUG("point 6\n");
  return 0;
}


static double mat_D_for_W_V_C_U_F(vecA *A, refR *R)
{
  double d[5];
  double D;
  int i;

  for(i = 0; i < 5; i++) d[i] = 4.343 * DISTANCE(&(A->a[i + 1]), &(R->r[i]));

  D = 0;
  for(i = 0; i < 5; i++) D += d[i] * d[i];
  for(i = 1; i < 4; i++) D += d[i] * d[i];

  D = sqrt(D / 8);

  return D;
}

static double mat_D_for_X(vecA *A, refR *R)
{
  double d[5];
  double D;
  int i;

  for(i = 0; i < 5; i++) d[i] = DISTANCE(&(A->a[i + 1]), &(R->r[i]));

  D = 0;
  for(i = 0; i < 5; i++) D += d[i] * d[i];

  D = sqrt(D / 5);

  return D;
}

static double mat_D_for_SV(vecA *A, refR *R)
{
  double d[5];
  int i, k;
  double D, bestD;

  for(k = 0; k <= 2; k++)
    {
      for(i = 0; i < 5; i++) d[i] = DISTANCE(&(A->a[i + k]), &(R->r[i]));
      D = 0;
      for(i = 0; i < 5; i++) D += d[i] * d[i];
      D = sqrt(D / 5);
      if((k == 0) || (D < bestD)) bestD = D;
    }
  return bestD;
}

static double mat_D_for_CW_UW(vecA *A, refR *R)
{
  double d[5];
  int i, k;
  double D, bestD;

  for(k = 0; k <= 2; k++)
    {
      for(i = 0; i < 5; i++) d[i] = DISTANCE(&(A->a[i + k]), &(R->r[i]));
      D = 0;
      for(i = 0; i < 5; i++) D += d[i] * d[i];
      for(i = 1; i < 4; i++) D += d[i] * d[i];
      D = sqrt(D / 8);
      if((k == 0) || (D < bestD)) bestD = D;
    }
  return bestD;
}

/*
 * 距離が短い順に matLattice ポインタの配列をソートする
 */
static int mat_cmpD(const void *va, const void *vb)
{
  matLattice *a, *b;

  a = *(matLattice **)va;
  b = *(matLattice **)vb;
  
  if(a->D < b->D) return -1;
  if(a->D > b->D) return 1;

  return 0;  
}

/*
 * スコアが大きい順にmatScore ポインタの配列をソートする
 */
static int mat_cmpScore(const void *va, const void *vb)
{
  matScore *a, *b;

  a = *(matScore **)va;
  b = *(matScore **)vb;

  if(a->score > b->score) return -1;
  if(a->score < b->score) return 1;
  return 0;
}

/* ------------------------------------------------------------------------- 
 * 入力パターンとリファレンスに基づいてスコアを集計する。
 * 
 * ※同じ入力パターンを持つ,複数のリファレンスパターンにまたがる
 *   スコアを処理する。
 * ------------------------------------------------------------------------- */
static 
matScore ** mat_create_lattice(anaParam ** p_list, /* フレームパラメータ   */
			       int p_nums,         /* 総フレーム数         */
			       int vIndex[7],      /* ベクトル抽出フレーム */
			       matchSubset *subset,/* 参照するサブセット群 */
			       int sub_nums)       /* サブセットの数       */
{
  int           nums, sc_nums;
  int           i, j, k, sub;
  int           fftN, n, max;
  double        d, UL;
  double (*func_D)(vecA *A, refR *R);
  matLattice ** lt_list;
  matScore   ** sc_list, *score;
  refBlock    * block;
  vecPosition   pos;
  vecA          vec;
  void        * vpt;
  int ref;

  DEBUG("*** Create Lattice ***\n");

  fftN = 2; n = 1;
  while(fftN < FRAME_WINDOW) fftN *= 2, n++;

  /*
   * 特徴ベクトル抽出フレーム列から、特徴ベクトルを生成する
   */
  for(i = 0; i < 7; i++)
    cepCalcVector(&(vec.a[i]), p_list[vIndex[i]]->spectrum, fftN);
  
  nums = 0;
  for(sub = 0; sub < sub_nums; sub++)
    {
      /* リファレンスパターンの選択 */
      switch(subset[sub])
	{
	case SS_W:  block = &refDICT.W;  func_D = mat_D_for_W_V_C_U_F; break;
	case SS_C:  block = &refDICT.C;  func_D = mat_D_for_W_V_C_U_F; break;
	case SS_U:  block = &refDICT.U;  func_D = mat_D_for_W_V_C_U_F; break;
	case SS_SV: block = &refDICT.SV; func_D = mat_D_for_SV;        break;
	case SS_CW: block = &refDICT.CW; func_D = mat_D_for_CW_UW;     break;
	case SS_UW: block = &refDICT.UW; func_D = mat_D_for_CW_UW;     break;
	case SS_F:  block = &refDICT.F;  func_D = mat_D_for_W_V_C_U_F; break;
	case SS_X:  block = &refDICT.X;  func_D = mat_D_for_X;         break;
	}

      {
	char * label;
	switch(subset[sub])
	  {
	  case SS_W:  label = "W";  break;
	  case SS_C:  label = "C";  break;
	  case SS_U:  label = "U";  break;
	  case SS_SV: label = "SV"; break;
	  case SS_CW: label = "CW"; break;
	  case SS_UW: label = "UW"; break;
	  case SS_F:  label = "F";  break;
	  case SS_X:  label = "X";  break;
	  }
      }

      /* 格納用ポインタ列を確保 */
      if(nums + block->nums == 0) continue;

      vpt = (!nums)
	? malloc(sizeof(matLattice *) * block->nums)
	: realloc(lt_list, sizeof(matLattice *) * (nums + block->nums));

      if(vpt == NULL)
	{
	  fprintf(stderr, "not enough memory.\n");
	  exit(EXIT_FAILURE);
	}
      lt_list = vpt;

      /* まず、リファレンスパターン全てに対する距離を求める */
      DEBUG("block->nums = %d\n", block->nums);

      for(i = 0; i < block->nums; i++)
	{
	  if(NULL == (lt_list[nums + i] =
		      malloc(sizeof(matLattice)))) goto err_0;
	  
	  /* リファレンスパターンサブセット情報を登録 */
	  lt_list[nums + i]->subset = subset[sub];
	  
	  /* ラベルについてはポインタをコピーするだけで、
	     メモリの確保等は行わない */
	  lt_list[nums + i]->label = block->refs[i]->label;
	  
	  /* 距離 D(A, R) を求める。
	     A … 音素セグメントから得られる特徴ベクトル
	     R …リファレンスパターンの特徴ベクトル */
	  lt_list[nums + i]->D = (func_D)(&vec, block->refs[i]);
	  DEBUG("D = %8.3f\n", lt_list[nums + i]->D);
	}
      nums += block->nums;
    }
  DEBUG("nums = %d\n", nums);

  /*
   * 求められた距離を基準にソートする
   */
  if(nums > 0)
    qsort(lt_list, nums, sizeof(matLattice *), mat_cmpD);

  DEBUG("-1-\n");


  /*
   * 最も特徴ベクトルに近いリファレンスパターンの距離を D1 とし、
   * (D1 + 0.5)[db] 以内にある音素リファレンスパターンの総数を求める
   */
  max = nums;

  for(i = 1; i < nums; i++)
    {
      d = fabs(lt_list[0]->D - lt_list[i]->D);
      if(d > 500.0)
	{
	  /* それ以降のものをカット */
	  for(j = i; j < nums; j++) free(lt_list[j]);
	  lt_list = realloc(lt_list, sizeof(matLattice *) * i);
	  max = i;
	  break;
	}
    }

  DEBUG("-2-\n");
  
  /* 類似度を求める */
  UL = 0;
  for(i = 0; i < max; i++)
    {
      lt_list[i]->U = (double)1 / (1 + lt_list[i]->D - lt_list[0]->D);
      UL += lt_list[i]->U;
    }

  DEBUG("-3-\n");
  
  /* 同一の音素ラベルを持つものの類似度を合計し,一つの候補として扱う */
  sc_nums = 0;
  sc_list = NULL;
  for(i = 0; i < max; i++)
    {
      for(j = 0; j < sc_nums; j++)
	if(!strcmp(sc_list[j]->label, lt_list[i]->label))
	  {
	    sc_list[j]->Uj += lt_list[i]->U;
	    break;
	  }
      if(j < sc_nums) continue;
      
      if(NULL == (score = malloc(sizeof(matScore)))) goto err_1;
      
      score->label = lt_list[j]->label;
      score->Uj    = lt_list[j]->U;
      
      vpt = (!sc_nums)
	? malloc(sizeof(matScore *))
	: realloc(sc_list, sizeof(matScore *) * (sc_nums + 1));
      
      if(vpt == NULL) goto err_1;
      
      sc_list = vpt;
      sc_list[sc_nums] = score;
      
      sc_nums++;
    }

  DEBUG("-4-\n");
  
  /*
   * 求められた合計類似度から、スコアを算出する
   */
  for(i = 0; i < sc_nums; i++)
    sc_list[i]->score = sc_list[i]->Uj / UL;

  for(i = 0; i < max; i++) free(lt_list[i]);
  if(max > 0) free(lt_list);

  DEBUG("-5-\n");

  /* スコアの高いもの順にソート */
  if(sc_nums > 0) qsort(sc_list, sc_nums , sizeof(matScore *), mat_cmpScore);

  DEBUG("-6-\n");

  /* 終端となる NULL を書き込む */
  vpt = (!sc_nums)
    ? malloc(sizeof(matScore *))
    : realloc(sc_list, sizeof(matScore *) * (sc_nums + 1));
  
  if(vpt == NULL)
    {
      for(i = 0; i < sc_nums; i++) free(sc_list[i]);
      free(sc_list);
      return NULL;
    }
  sc_list = vpt;
  sc_list[sc_nums] = NULL;

  DEBUG("-7-\n");
  
  return sc_list;

 err_1:
  for(i = 0; i < sc_nums; i++) free(sc_list[i]);
  free(sc_list);
  
 err_0:
  for(i = 0; lt_list[i] != NULL; i++) free(lt_list[i]);
  free(lt_list);
  return NULL;
}

