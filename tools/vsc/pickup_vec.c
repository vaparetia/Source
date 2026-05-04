/*
 * セグメンテーション済のデータから,
 * 各音素セグメントのメルケプストラムによる特徴ベクトルを,
 * 全体型、定常部型,過渡部型の三種にわたって取り出し、出力する。
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _pickup_vec_c_
#include "env.h"
#include "analyze.h"
#include "cepstrum.h"
#include "pickup_vec.h"

/*
 * セグメンテーション済のデータから、各セグメントの特徴ベクトルを抽出し、
 * 標準出力に出力する
 */
int pvcPickupVector(int skip, anaParam **p_list, int nums)
{
  int t, n;
  int i, j, k;
  int fftN;
  int begin, end;   /* セグメントの始点, 終点を保持するための変数 */
  int vecidx[3][5];   /* 各タイプのベクトルを抽出するフレーム番号保持用配列 */
  cepVector * veclist[3][5];
  static char *label[3] = {"all", "static", "border"};


  n = 1, fftN = 2;
  while(fftN < FRAME_WINDOW) fftN *= 2, n++;

  for(t = 0; t < nums; t++)
    {
      /* 音素境界でなければ処理をスキップ */
      if(!(p_list[t]->status & ST_VSEG)) continue;

      begin = t;
      
      /* セグメントの終了フレームを得る */
      end = -1;
      for(i = t + 1; i < nums; i++)
	{
	  end = i;
	  if(p_list[i]->status & ST_VSEG) break;
	}
      if(end < 0)
	{
	  fprintf(stderr, "next segment is not found.\n");
	  exit(EXIT_FAILURE);
	}
      end--;
      fprintf(stderr, "time %d-%d\n", begin + skip, end + skip);
      /* 全体型ベクトルを抽出するフレーム番号を選別する */
      /* 全体型ベクトルは,フレーム全体を 6等分したその境界にある。 */
      {
	double step;

	step = (double)(end + 1 - begin) / 6;

	for(i = 1; i < 6; i++)
	  vecidx[0][i-1] = begin + step * i;
      }


      /* 定常部型ベクトルを抽出するフレーム番号を選別する */
      /* 定常部型ベクトルは,スペクトル包絡時間変化が最も安定したフレームを
	 中心に, 1フレームおきに取得する */
      {
	double bestEi;
	int bestT;

	/* 最もスペクトル包絡時間変化が安定した箇所を探す。
	   安定している箇所は,もっとも値が 0 に近い場所,
	   つまり最も絶対値が低い場所となる。 */
	bestEi = 100;
	bestT = -1;
	for(i = begin; i <= end; i++)
	  if(fabs(p_list[i]->Ei) < bestEi)
	    {
	      bestEi = fabs(p_list[i]->Ei);
	      bestT = i;
	    }
	for(i = 0; i < 5; i++)
	  {
	    j = (i - 2) * 2;
	    vecidx[1][i] = bestT + j;
	  }
      }
      
      /* 過渡部型ベクトルを抽出するフレーム番号を選別する */
      /* 過渡部型ベクトルは,音素セグメントの最終フレームを中心に、
	 前後に対し1フレームおきに抽出する。 */
      {
	for(i = 0; i < 5; i++)
	  {
	    j = (i - 2) * 2;
	    vecidx[2][i] = end + j;
	  }
      }
      /* 値が分析フレームの範囲を逸脱していると困るので、
	 それらを補正する。 */
      for(i = 0; i < 3; i++)
	for(j = 0; j < 5; j++)
	  {
	    if(vecidx[i][j] < 0) vecidx[i][j] = 0;
	    if(vecidx[i][j] >= nums) vecidx[i][j] = nums - 1;
	  }

      /*
       * 対数スペクトルの不偏推定法によりメル対数スペクトル包絡は
       * 既に計算済なので、そのメル対数スペクトル包絡をもとに
       * 10次のメルケプストラムを求め,これを特徴ベクトルとして採用する
       */
      for(i = 0; i < 3; i++)
	for(j = 0; j < 5; j++)
	  {
	    veclist[i][j] =
	      cepCalcVector(NULL, p_list[vecidx[i][j]]->spectrum, fftN);
	  }

      /* その音素セグメントの三種の特徴ベクトルの抽出ができたので、
	 これを出力する。 */

      printf("<vector>\n"
	     "  <time>%d-%d</time>\n", begin + skip, end + skip);

      for(i = 0; i < 3; i++)
	{
	  printf("  <%s>\n", label[i]);
	  for(j = 0; j < 5; j++)
	    {
	      printf("    <vec>");
	      for(k = 0; k < cepVEC_DIM; k++)
		{
		  if(!(k % 5)) printf("\n    ");
		  printf("  %8.3f", veclist[i][j]->c[k]);
		}
	      printf("\n    </vec>\n");
	    }
	  printf("  </%s>\n", label[i]);
	}
      printf("</vector>\n\n");
      for(i = 0; i < 3; i++)
	for(j = 0; j < 5; j++)
	  free(veclist[i][j]);
    }
  return 0;
}
