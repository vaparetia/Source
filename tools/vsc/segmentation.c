/*
 * 分析データを元に、音素境界で分割、
 * その時系列を記録する。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define _segmentation_c_
#include "main.h"
#include "segmentation.h"
#include "analyze.h"
#include "label.h"
#include "util.h"
#include "voice.h"

#ifdef _SEG_DEBUG_
#ifndef _DEBUG_
#define _DEBUG_
#endif /* _DEBUG_ */
#endif /* _SEG_DEBUG_ */

#include "debug.h"

#define L 2
#define ABS(n)   (((n) < 0) ? -(n) : (n))

static struct {
  char * res;
  double * var;
} segResources[] = {
  /* セグメンテーション用閾値の設定リソース */
  {"Tvl", &segP.Tvl},
  {"Tvh", &segP.Tvh},

  {"Ta1", &segP.Ta1},
  {"Ta2", &segP.Ta2},
  {"Ta3", &segP.Ta3},
  {"Te1", &segP.Te1},
  {"Te2", &segP.Te2},
  {"Td",  &segP.Td },
  {"Tcl", &segP.Tcl},
  {"Tch", &segP.Tch},
  {"Tzl", &segP.Tzl},
  {"Tzh", &segP.Tzh},
  {"Ts",  &segP.Ts },
  {"Tf",  &segP.Tf },
  {"Tf2", &segP.Tf2},

  /* 終了 */
  {NULL,  NULL     }
};


/*
 * 有声音区間と、非有声音区間のクラス分け
 */
static int seg_voiced_filter(anaParam ** p_list, int nums)
{
  int * V;
  int i, j;
  int Tv;
  int Vi, tmp;
  int Vt[5];
  double vi;

  if(NULL == (V = malloc(sizeof(int) * nums))) return -1;
  for(i = 0; i < nums; i++)
    {
      vi = p_list[i]->Vi;

      V[i] = U(vi - p_list[i]->Tvl) + U(vi - p_list[i]->Tvh);
      p_list[i]->V = V[i];
    }

  Tv = 0;
  Vi = 0;
  for(i = 0; i < nums; i++)
    {
      tmp = 0;
      for(j = 0; j < 5; j++)
	{
	  if((i + j - 2 < 0) || (i + j - 2 >= nums)) continue;
	  tmp += V[i + j - 2];
	}
      Tv = 5 - Vi;
      Vi = U(tmp - Tv);

      if(Vi)
	{
	  p_list[i]->status |= ST_VOICED;  /* 有声音フラグをセット */
	  /*
	    直前のフレームに有声音フラグがなければ、音素境界としても扱う
	  */
	  /*
	    if(i > 0)
	    if((p_list[i - 1]->status & ST_VOICED) == 0)
	      p_list[i]->status |= ST_VSEG;
	  */
	}
    }
  free(V);
  return 0;
}


/*
 * 有声音区間における音素境界検出および音素群ラベリング
 */
static int seg_voiced_segmentation(anaParam ** p_list, int nums)
{
  int i, j, k, l, m, n;
  int final;
  double m_buf[L * 2 + 1];
  double *A, *E;
  vGroup group;

#ifdef _DEBUG_
  /* nums の値まで, 全てデータが満たしているかをチェック */
  for(i = 0; i < nums; i++) if(p_list[i] == NULL) break;
  printf("1: max nums = %d\n", i);
#endif

  if(NULL == (A = malloc(sizeof(double) * nums))) return -1;
  if(NULL == (E = malloc(sizeof(double) * nums)))
    {
      free(A);
      return -1;
    }

  for(i = 0; i < nums; i++) A[i] = E[i] = 0;
  for(i = 0; (i < nums) && (p_list[i] != NULL); i++)
    {
      /* 有声音区間のフレームで無い場合、処理を行なわない */
      if(!(p_list[i]->status & ST_VOICED)) continue;

      /* パラメータ $a_i$ の極値関数 $A_i$ の値を求める */
      for(j = 0; j < 3; j++)
	{
	  k = j - 1;
	  if((i + k< 0) || ((i + k) >= nums) || (p_list[i + k] == NULL))
	    {
	      m_buf[j] = 0;
	      continue;
	    }

	  m_buf[j] = p_list[i + k]->Ai;
	}
      
      /* $A_i$ の値は、$a_i$ の極大もしくは極小点の値 */
      A[i] = ((m_buf[1] > 0 &&(m_buf[1] > m_buf[0] && m_buf[1] >= m_buf[2])) ||
	      (m_buf[1] < 0 &&(m_buf[1] < m_buf[0] && m_buf[1] <= m_buf[2])))
	? p_list[i]->Ai : 0;

      /* パラメータ $e_i$ の極値関数 $E_i$ の値を求める */
      for(j = 0; j < 3; j++)
	{
	  k = j - 1;
	  if((i + k < 0) || (i + k >= nums) || (p_list[i + k] == NULL))
	    {
	      m_buf[j] = 0;
	      continue;
	    }

	  m_buf[j] = p_list[i + k]->Ei;
	}

      /* $E_i$ の値は、 $e_i$ の極大点の値 */
      E[i] = ((m_buf[1] > 0)&&(m_buf[1] > m_buf[0])&&(m_buf[1] >= m_buf[2]))
	? p_list[i]->Ei : 0;
    }

#ifdef _DEBUG_
  /* nums の値まで, 全てデータが満たしているかをチェック */
  for(i = 0; i < nums; i++) if(p_list[i] == NULL) break;
  printf("2: max nums = %d\n", i);
#endif


  /* 極値関数 Ai の値を修正する */
  for(i = 0; (i < nums) && (p_list[i] != NULL); i++)
    {
      for(l = i - 5; l < i; l++)
	if((A[i] > 0) && (segP.Ta1 > A[i]) &&
	   (A[l] != 0) && (A[i] - A[l] < segP.Ta2)) A[i] = 0;

      for(m = i + 1; (m <= i + 7) && (m < nums); m++)
	if((A[i] < 0) && (-segP.Ta1 < A[i]) &&
	   (A[m] != 0) && (A[m] - A[i] < segP.Ta2)) A[i] = 0;
    }

#ifdef _DEBUG_
  /* nums の値まで, 全てデータが満たしているかをチェック */
  for(i = 0; i < nums; i++) if(p_list[i] == NULL) break;
  printf("3: max nums = %d\n", i);
#endif


  /* 極値関数 Ei の値を修正する */
  for(i = 0; (i < nums) && (p_list[i] != NULL); i++)
    {
      /* 極値関数 $E_i$ を修正する */
      if((0 < E[i]) && (E[i] <= segP.Te1)) E[i] = 0;
    }

#ifdef _DEBUG_
  /* nums の値まで, 全てデータが満たしているかをチェック */
  for(i = 0; i < nums; i++) if(p_list[i] == NULL) break;
  printf("4: max nums = %d\n", i);
#endif

  /*
   * 各有声音区間において、音素境界の併合、選択を行う
   */
  i = 0;
  while( (i < nums ) && (p_list[i] != NULL) )
    {
      
      DBG("  [1]: i = %d (nums = %d, p_list[%d] = %p)\n", i, nums, i, p_list[i]);
      /*
       * フレームが有声音区間でなければ、スキップする。
       */
      if(!(p_list[i]->status & ST_VOICED))
	{
	  i++;
	  continue;
	}
      DBG("z\n");

      DBG("  [2]: i = %d\n", i);

      /* 有声音区間の始端,終端を求める */
      k = i;                                                        /* 始端 */
      while((i < nums) && (p_list[i] != NULL))                      /* 終端 */
	{
	  if(!(p_list[i]->status & ST_VOICED)) break;
	  n = i++;
	}

      DBG("  [3]: n = %d\n", n);
      
      /* 有声音区間の最初のフレームには、必ず音素境界をおく */
      p_list[k]->status |= ST_VSEG;

      DBG("  [4]\n");

      /* [規則1] 有声音区間の始端部と終端部における修正 */
      A[k] = 1;
      if(k + 1 < nums) A[k + 1] = E[k + 1] = 0;

      DBG("  [5]\n");

      for(j = n - 7; j <= n; j++)
	{
	  if((j < 0) || j >= nums) continue;
	  if(((A[j] > 0) && (j >= (n - 5))) || (A[j] < 0)) A[j] = 0;
	}

      DBG("  [6]\n");

      if(n - k >= 15)
	for(j = n - 3; j <= n; j++) E[j] = 0;
      else
	for(j = n - 4; j <= n; j++)
	  if(j >= 0) E[j] = 0;

      DBG("  [7]\n");

      /* [規則2] Ai と Ei の併合 */
      for(j = k; j <= n; j++)
	if(A[j] != 0)
	  for(m = j - 2; m <= j + 2; m++)
	    {
	      if((m < 0) || (m >= nums)) continue;
	      E[m] = 0;
	    }

      DBG("  [8]\n");

      /* [規則3] 有声子音の先行部における修正 */
      for(i = k; i <= n; i++)
	for(j = i + 1; j <= n; j++)
	  for(m = i - 4; m < i; m++)
	    for(l = m - 10; l < m; l++)
	      {
		if((l < 0) || (l >= nums))continue;
		if((m < 0) || (m >= nums))continue;
		if((i < 0) || (i >= nums))continue;
		if((j < 0) || (j >= nums))continue;
		if(A[l] > 0 && A[m] > 0 && A[i] < 0 && A[j] > 0) A[m] = 0;
	      }

      DBG("  [9]\n");

      /* [規則4] 有声子音の中間部における修正 */
      for(m = k; i <= n; i++)
	{
	  if(E[m] == 0) continue;
	  for(i = m - 4; i < m; i++)
	    {
	      if(i < 0 || i >= nums) continue;
	      for(j = m + 1; j <= m + 10; j++)
		{
		  if(j < 0 || j >= nums) continue;
		  if(A[i] < 0 && A[j] > 0) E[m] = 0;
		}
	    }
	}

      DBG("  [10]\n");

      /* [規則5] 有声子音の後続部における修正 */
      for(j = k; j + 4 <= n; j++)
	if(A[j] > 0)
	  for(i = k; i < j; i++)
	    if(A[i] < 0)
	      for(l = j + 1; l <= j + 4; l++)
		if(((segP.Ta3 > A[l]) && (A[l] > 0)) ||
		   ((segP.Te2 > E[l]) && (E[l] > 0)))
		  A[l] = E[l] = 0;

      DBG("  [11]\n");

      for(j = k; j < n; j++)
	if(A[j] > 0)
	  for(i = k; i < j; i++)
	    if(A[i] < 0)
	      for(l = j + 1; l <= j + 4; l++)
		if(A[l] < 0)
		  for(m = l + 1; m <= n; m++)
		    if(A[m] <= 0) A[l] = 0;

      DBG("  [12]\n");
      i = n + 1;
    }
  DBG(" {1}\n");

  /* 最終的に,Ai あるいは Ei が零ではない分析フレームを,有声音素境界とする */
  for(i = 0; (i < nums) && (NULL != p_list[i]); i++)
    if(A[i] != 0 || E[i] != 0) p_list[i]->status |= ST_VSEG;

  DBG(" {2}\n");

#ifdef _DEBUG_
  /* nums の値まで, 全てデータが満たしているかをチェック */
  for(i = 0; i < nums; i++) if(p_list[i] == NULL) break;
  printf("5: max nums = %d\n", i);
#endif


  /* 有声音区間の音素群ラベリングを行う */
  l = -1;
  i = 0;
  final = 0;

  while((i < nums) && (NULL != p_list[i]))
    {
      final = 0;

      DBG("  [1]: p_list[%d] = %p\n", i, p_list[i]);

      /* 有声音区間以外は無視する */
      /* 音素境界でなければスキップ */
      if(!(p_list[i]->status & ST_VOICED) || !(p_list[i]->status & ST_VSEG))
	{
	  DBG("[x]\n");
	  i++;
	  DBG("next p_list[%d] = %p\n", i, p_list[i]);
	  continue;
	}

      DBG("  [2]\n");

      /* 次の音素境界を見付ける */
      for(j = i + 1; (j < nums) && (p_list[j] != NULL); j++)
	{
	  if(!(p_list[j]->status & ST_VOICED))
	    {
	      final = 1;
	      break;
	    }
	  
	  if(p_list[j]->status & ST_VSEG) break;
	}
      if(j >= nums)
	{
	  i++;
	  continue;
	}

      DBG("  [3]\n");

      /*
       * l   直前の音素境界
       *     (その有声音区間内に,直前音素境界が無い場合は -1)
       *
       * i   現在チェックの対象となっている音素セグメントの始点境界
       *
       * j   後続の音素セグメントとの境界
       */
      group = V_V;

      DBG("  [4]\n");

      /* 母音的セグメント(W)のチェック */
      if((A[i] > 0) && (A[j] < 0)) group = V_W;

      if(l >= 0)
	if((A[l] < 0) && (A[i] > 0) && (A[j] > 0)) group = V_W;

      /* 有声音的セグメント(V) のチェック */
      if((A[i] < 0) && ((A[j] < 0) || (E[j] > 0))) group = V_V;

      if(l >= 0)
	if(((A[l] > 0) || (E[l] > 0)) &&
	   ((A[i] > 0) || (E[i] > 0)) &&
	   (A[j] > 0))
	  group = V_V;

      
      /* 有声子音的セグメント(C) のチェック */
      /* 有声子音セグメント(D) のチェック   */
      if(A[i] < 0 && A[j] > 0)
	group = ((A[j] - A[i]) / (j - i) <= segP.Td) ? V_C : V_D;

      DBG("  [5]\n");

      /* ------------------------------------------------------------------- 
       * 日本語の遷移的特性を考え,補正を行う。
       * 以下は、日本語以外の言語音を認識させるとき以外は、
       * 他の処理に置き換える必要がある
       * ------------------------------------------------------------------- */
      if(mLangMode == LANG_JAPANESE)
	{
	  /* C の区間の長さが 8 フレーム以上の場合は V とする */
	  if((group == V_C) && (j - i >= 8)) group = V_V;
	  
	  /* 有声音区間の最初のセグメントの長さが 3 フレーム以下のときは、
	     そのセグメントを V として扱う */
	  
	  if((l < 0) && (j - i <= 3)) group = V_V;
	  
	  /* 有声音区間の最後のセグメントは W とする */
	  if(final)	group = V_W;
	}

      DBG("  [6]\n");

      /* -------------------------------------------------------------------
       * ここまで
       * ------------------------------------------------------------------- */

      /* 決定した音素群ラベルで、セグメント区間内を塗りつぶす */
      for(n = i; n < j; n++)
	p_list[n]->group = group;

      l = i;
      i = j;
      DBG("  [7]\n");

    }

#ifdef _DEBUG_
  /* nums の値まで, 全てデータが満たしているかをチェック */
  for(i = 0; i < nums; i++) if(p_list[i] == NULL) break;
  printf("6: max nums = %d\n", i);
#endif


  free(E);
  free(A);

  return 0;
}

/*
 * 非有声音区間における音素境界検出および音素群ラベリング
 */
static int seg_no_voiced_segmentation(anaParam ** p_list, int nums)
{
  int *Ct, *Zt, *Si;
  int  i, j, k, l, m, n;
  double *f, *e;
  double *F;
  int tmp;
  vGroup group;

  if(NULL == (Ct  = malloc(sizeof(int) * nums))) goto err_0;
  if(NULL == (Zt  = malloc(sizeof(int) * nums))) goto err_1;
  if(NULL == (Si  = malloc(sizeof(int) * nums))) goto err_2;
  if(NULL == (F   = malloc(sizeof(double) * nums))) goto err_3;
  if(NULL == (f   = malloc(sizeof(double) * nums))) goto err_4;
  if(NULL == (e   = malloc(sizeof(double) * nums))) goto err_5;

  DBG("[0]\n");
  for(i = 0; i < nums; i++)
    {
      Ct[i] = Zt[i] = Si[i] = 0;
      F[i] = 0;
      f[i] = e[i] = 0.0;
    }

  DBG("[1]\n");
  for(i = 0; i < nums; i++)
    {
      Ct[i] = U(p_list[i]->Ci - segP.Tcl) + U(p_list[i]->Ci - segP.Tch);
      Zt[i] = U(p_list[i]->Zi - segP.Tzl) + U(p_list[i]->Zi - segP.Tzh);

      /* 有声音区間は、最初のフレームを除き Ct[i] を 0 とする */
      if(p_list[i]->status & ST_VOICED)
	if(i > 0)
	  if(!(p_list[i]->status & ST_VOICED))
	    {
	      Ct[i] = 0;
	      break;
	    }
    }
  DBG("[2]\n");

  /* 孤立データを除去する */
  for(i = 1; i < nums - 1; i++)
    {
      if(Ct[i - 1] + Ct[i] + Ct[i + 1] <= 1) Ct[i] = 0;
      if(Zt[i - 1] + Zt[i] + Zt[i + 1] <= 1) Zt[i] = 0;
    }


  DBG("[3]\n");

  /* 有音無音理論値 Si を求める */
  for(i = 0; i < nums; i++)
    {
      Si[i] = 0;
      tmp = 0;
      for(k = -2; k <= 2; k++)
	{
	  if(i + k < 0 || i + k >= nums) continue;
	  tmp += Ct[i + k];
	}
      tmp += Zt[i];
      Si[i] = U((double)tmp - segP.Ts);
      if(!Si[i] && !(p_list[i]->status & ST_VOICED))
	p_list[i]->status |= ST_SILENT;

      if(i > 0)
	if(Si[i] && !(p_list[i]->status & ST_VOICED) &&
	   (p_list[i - 1]->status & (ST_VOICED | ST_SILENT)))
	  p_list[i]->status |= ST_VSEG;
    }

  DBG("[4]\n");

  
  /* 摩擦音判別理論値 F を求める */
  i = 0;
  while(i < nums)
    {
      DBG("[5] i = %d\n", i);
      if(!Si[i])
	{
	  /* 無音部,有声音部は対象としない */

	  /* 直前が無音でなければ、音素境界として扱う */
	  if(i > 0)
	    if(Si[i-1]) p_list[i]->status |= ST_VSEG;

	  p_list[i]->group = V_S;
	  i++;
	  continue;
	}

      DBG("[6] i = %d\n", i);

      /* 直前が無音であれば, 音素境界として扱う */
      if(i > 0)
	if(!Si[i - 1]) p_list[i]->status |= ST_VSEG;

      DBG("[7] i = %d\n", i);

      /* 無声有音部の始端,終端を求める */
      k = i;  /* k = 始端 */
      n = i + 1;
      for(j = i + 1; j < nums; j++)
	{
	  if(!Si[j]) break;
	  n = j;   /* n = 終端 */
	}
      tmp = 0;
      for(i = k; i <= n; i++) tmp += Zt[i];
      if(i >= nums) i = nums - 1;

      F[i] = U((double)tmp / (double)(n - k + 1) - segP.Tf);

      DBG("[8] i = %d\n", i);

      /* 無声有音部を、$f_i$ と $e_i$ の極大値
	 ならびに $f_i$ の極小値を検出する形で無声音区間の
	 音素境界候補を求める*/
      for(i = k; i <= n; i++)
	{
	  /* Fi が極大もしくは極小の場合は, f にその値を収納する。
	     そうでない場合は, f には 0 を収納する */

	  f[i] = p_list[i]->Fi;
	  if(f[i] < 0)  /* 負の値の場合は、極小のチェックを行う */
	    {
	      if(i > 0)
		if(p_list[i]->Fi >= p_list[i - 1]->Fi) f[i] = 0;
	      if(i < nums - 1)
		if(p_list[i]->Fi > p_list[i + 1]->Fi) f[i] = 0;
	    }
	  else       /* 0 もしくは正の値の場合は,極大のチェックを行う */
	    {
	      if(i > 0)
		if(p_list[i]->Fi <= p_list[i - 1]->Fi) f[i] = 0;
	      if(i < nums - 1)
		if(p_list[i]->Fi < p_list[i + 1]->Fi) f[i] = 0;
	    }
	  
	  /* f の値が閾値を越えているかを確認し,
	     越えていなければ 0 として扱う */
	  if(fabs(f[i]) <= segP.Tf2) f[i] = 0;

	  /* Ei の極大を求める */
	  e[i] = p_list[i]->Ei;
	  if(i > 0)
	    if(p_list[i]->Ei <= p_list[i - 1]->Ei) e[i] = 0;
	  if(i < nums - 1)
	    if(p_list[i]->Ei < p_list[i + 1]->Ei) e[i] = 0;
	    
	  /* e の値が閾値を越えているかを確認し,
	     越えていなければ 0 として扱う */
	  if(e[i] <= segP.Te1) e[i] = 0;
	}
      i = n + 1;
      DBG("[9] i = %d\n", i);
    }

  /* 音素境界を決定する */
  for(i = 0; i < nums; i++)
    {
      if(p_list[i]->status & ST_VOICED) continue;
      if(f[i] < 0)
	{
	  p_list[i]->status |= ST_VSEG;
	  continue;
	}
      if(e[i] > 0)
	for(j = i; j < i + 4 && j < nums; j++)
	  if(f[j] > 0)
	    p_list[i]->status |= ST_VSEG;

      /* 境界となったフレームの音素群ラベルを設定する */
      if(p_list[i]->status & ST_SILENT) p_list[i]->group = V_S;
      else
	p_list[i]->group = (F[i] != 0) ? V_F : V_U;
    }

  /* 音素群の補正を行う */
  l = -1;
  i = 0;
  while(i < nums)
    {
      /* 有声音区間は対象としない */
      /* 音素境界でなければスキップ */
      if((p_list[i]->status & ST_VOICED) || !(p_list[i]->status & ST_VSEG))
	{
	  i++;
	  continue;
	}
      /*
       * 有声音→非有声音 および
       * 非有声音→有声音 の音素境界は,有声音のセグメンテーションの段階で
       * 検出されている。
       */

      /* 次の音素境界を探す */
      for(j = i + 1; j < nums - 1; j++)
	if(p_list[j]->status & (ST_VSEG|ST_VOICED)) break;

      /* j は次の音素境界 */

      if(p_list[i]->group == V_U)
	{
	  /* U, S の順で続く場合は、U を S に変え、両区間をまとめて S とする */
	  if(p_list[j]->group == V_S)
	    {
	      p_list[i]->group = V_S;
	      p_list[j]->status &= 0xffff^ST_VSEG;
	    }
	  
	  /* U の長さが 2 フレーム以下で、S,U の順で続き、
	     その後 5フレーム以下の音素的単位 V で始まる有声音区間に先行
	     するときは、U を V に変え,両区間をまとめて S とする */
	  if(l >= 0)
	    if((j - i <= 2) &&
	       (p_list[l]->group == V_S) &&
	       (p_list[j]->group == V_V))
	      {
		for(m = j + 1; m < nums; m++)
		  if(p_list[m]->status & ST_VSEG) break;

		if(m - j <= 5)
		  {
		    p_list[i]->group = V_V;
		    p_list[j]->status &= 0xffff^ST_VSEG;
		    p_list[i]->status &= 0xffff^ST_VSEG;
		  }
	      }
	}
      l = i;
      i = j;
    }
  
  /*
   * セグメント属性を,セグメント先頭の属性で統一する
   */
  group = V_S;
  for(i = 0; i < nums; i++)
    {
      if(p_list[i]->status & ST_VOICED) continue;
      if(p_list[i]->status & ST_VSEG)
	{
	  group = p_list[i]->group;
	  continue;
	}
      if(p_list[i]->status & ST_SILENT) group = V_S;
      p_list[i]->group = group;
    }

  free(e);
  free(f);
  free(F);
  free(Si);
  free(Zt);
  free(Ct);
  return 0;
  
 err_5:
  free(f);
 err_4:
  free(F);
 err_3:
  free(Si);
 err_2:
  free(Zt);
 err_1:
  free(Ct);
 err_0:

  return -1;
}

/*
 * セグメンテーション用閾値の初期化
 */
void segInit(void)
{
  /* 有声音/無声音区分 */
  segP.Tvl = SEG_Tvl;
  segP.Tvh = SEG_Tvh;

  segP.Ta1 = SEG_Ta1;

  segP.Ta2 = SEG_Ta2;
  segP.Ta3 = SEG_Ta3;
    
  segP.Te1 = SEG_Te1;
  segP.Te2 = SEG_Te2;

  segP.Td  = SEG_Td;

  segP.Tcl = SEG_Tcl;
  segP.Tch = SEG_Tch;
  segP.Tzl = SEG_Tzl;
  segP.Tzh = SEG_Tzh;

  segP.Ts  = SEG_Ts;
  segP.Tf  = SEG_Tf;
  segP.Tf2 = SEG_Tf2;
}

/*
 *
 * 近すぎる音素境界を一つにまとめる。
 *
 * 隣り合ったフレームで、連続して音素境界マークがついているとき、
 * 多くの場合それは一つの音素境界を指す。よって、これは統合する必要がある。
 *
 */
static int seg_border_sharpen(anaParam ** p_list, int nums)
{
  double *score;
  int i, j, k;

  if(NULL == (score = malloc(sizeof(double) * nums))) return -1;

  for(i = 0; i < nums; i++) score[i] = 0.0;  /* スコアの初期化 */

  /* 全フレームに渡り、"音素境界密度値" を求める */
  for(i = 0; i < nums; i++)
    if(p_list[i]->status & ST_VSEG)
      {
	if(i > 0) score[i - 1] += 0.75;
	score[i] += 1;
	if(i + 1 < nums) score[i + 1] += 0.75;
      }	    

  /* 一旦全ての音素境界マークを除去する */
  for(i = 0; i < nums; i++) p_list[i]->status &= 0xffff^ST_VSEG;

  /* 音素境界密度の極大点を新たな音素境界としてマークしなおす */
  for(i = 1; i < nums - 1; i++)
    {
      if((score[i] > score[i - 1]) && 
	 (score[i] >= score[i + 1]))
	p_list[i]->status |= ST_VSEG;
    }

  free(score);
  return 0;
}

/*
 * セグメント単位での有声音、無声音、無音の区分を考慮し、
 * セグメント内全体をおなじ属性で埋める。
 */
static int seg_attribute_by_segment(anaParam ** p_list, int nums)
{
  int i, j;
  int top, at;
  int cnt_v;  /* セグメント内の、有声音フレームのカウンタ */
  int cnt_nv; /* セグメント内の、無声音フレームのカウンタ */
  int cnt_sl; /* セグメント内の、無音フレームのカウンタ   */

  top = 0;
  cnt_v = cnt_nv = cnt_sl = 0;

  for(i = 0; i < nums; i++)
    {
      /* セグメント境界もしくは最終フレームが来たら、
	 それまでのカウンタ数に基づいて,セグメント内を
	 おなじ属性で満たす */
      if((p_list[i]->status & ST_VSEG) || (i == nums - 1))
	{
	  at = 0;
	  if(cnt_v > cnt_nv && cnt_v > cnt_sl) at = ST_VOICED;
	  if(cnt_sl > cnt_v && cnt_sl > cnt_nv) at = ST_SILENT;
	  for(j = top; j < i; j++)
	    {
	      p_list[j]->status &= 0xffff ^ (ST_VOICED | ST_SILENT);
	      p_list[j]->status |= at;
	    }
	  top = i;
	  cnt_v = cnt_nv = cnt_sl = 0;
	}
      if(p_list[i]->status & ST_VOICED) { cnt_v++;  continue; }
      if(p_list[i]->status & ST_SILENT) { cnt_sl++; continue; }
      cnt_nv++;
    }
  return 0;
}


void segSegmentation(anaParam **p_list, int nums)
{
  int i;
  double Tvl, Tvh;

  DBG("entering segSegmentation()\n");
  
  DBG("call voiAutoAdjustLevel()\n");
   /* 有声音/非有声音区間のクラス分けに必要な閾値の値を算出する。 */
  voiAutoAdjustLevel(p_list, nums);

  DBG("call seg_voiced_filter()\n");

  /* 有声音区間、非有声音区間のクラス分け */
  seg_voiced_filter(p_list, nums);

  DBG("call seg_voiced_segmentation()\n");

  /* 有声音区間におけるセグメンテーション(音素境界検出) */
  seg_voiced_segmentation(p_list, nums);

  DBG("call seg_no_voiced_segmentation()\n");

  /* 非有声音区間におけるセグメンテーション(音素境界検出) */
  seg_no_voiced_segmentation(p_list, nums);

#if 0
  /* 近すぎる音素境界を一つにまとめる */
  seg_border_sharpen(p_list, nums);

  /* セグメント単位での有声音,非有声音の区分を重視する(フレーム単位ではない) */
  seg_attribute_by_segment(p_list, nums);
#endif

  DBG("segSegmentation() is completed.\n");
}

/*
 * 閾値の値を設定ファイルから読み込む
 */
int segLoadParams(char *opt_file, char *home_file, char *global_file)
{
  FILE * rfp;
  char * line;
  char * class;
  char * value;
  int i, j;
  int ret = 0;

  /*
   * 閾値の設定ファイルを、
   *
   *    1) コマンドラインオプションで指定されたファイル
   *    2) ホームディレクトリに存在する .voice ファイル
   *    3) グローバルに配置された設定ファイル(/usr/local/share/vsc/Voice)
   *
   * の順にチェックし、見付かった時点のものを設定ファイルとして使用する。
   */
  rfp = NULL;
  if(opt_file != NULL) rfp = fopen(opt_file, "r");
  if(NULL == rfp) rfp = fopen(home_file, "r");
  if(NULL == rfp) rfp = fopen(global_file, "r");

  if(NULL == rfp) return -1;

  while(!ret && (NULL != (line = utilReadLine(rfp))))
    {
      /* 注釈部(#以降)をカットする */
      for(i = 0; line[i] != 0; i++)
	if(line[i] == '#')
	  {
	    line[i] = 0;
	    break;
	  }

      class = value = NULL;
      utilSplit(&class, &value, line);
      if(NULL == class || NULL == value) continue;
      
#ifdef _DEBUG_
      fprintf(stderr, "[%s] = [%s]\n", class, value);
#endif /* _DEBUG_ */
      
      if(!strcmp(class, "DIC"))
	{
	  /* 音韻リファレンスが配置されているディレクトリの指定 */
	  strcpy(mDicDir, value);
	}
      else
	for(i = 0; segResources[i].res != NULL; i++)
	  if(!strcmp(class, segResources[i].res))
	    {
	      *(segResources[i].var) = atof(value);
	      break;
	    }
      free(class);
      free(value);
      free(line);
    }
  
  fclose(rfp);
  return ret;
}
