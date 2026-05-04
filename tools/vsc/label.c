#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

/*
#define _NO_VOWEL_
#undef _NO_VOWEL_
*/
#define _label_c_
#include "analyze.h"
#include "label.h"
#include "text.h"
#include "util.h"
#include "main.h"
#include "vowel.h"
#include "matching.h"
#include "debug.h"

#define ABS(n)  (((n) < 0) ? -(n) : (n))

static labVList * lab_create_border_list(int skip, 
					 anaParam ** p_list, int nums);
static int lab_group2num(vGroup g);
static int lab_cmp_group(vGroup sample, vGroup cmp);
static int lab_check_group(txtPhono *tx, vGroup g);
static int lab_labeling(int skip,
			txtPhono **t_list, anaParam **p_list, int nums);
static int lab_matching(int skip, anaParam ** p_list, int nums,
			txtPhono ** t_list, int t_top, int t_len,
			labVList *  v_list, int v_top, int v_len);

static int lab_matching_consonant(txtPhono ** t_list, int t_top, int t_len,
				  labVList *  v_list, int v_top, int v_len);


static int _lab_voice_mode;

/*
 * ラベリングモジュールの初期化
 */
void labInit(void)
{
  labP.CmaxAi = LBL_C_maxAi;
  labP.WmaxEi = LBL_W_maxEi;
  labP.DmaxFi = LBL_D_maxFi;
  labP.NmaxFi = LBL_N_maxFi;
  labP.NmaxZi = LBL_N_maxZi;
}
     
/*
 * 音素ラベリングを行なう
 */
int labLabeling(int skip, txtPhono ** tx_list,
		anaParam **p_list, int nums,
		int voice_mode)
{
  int t;
  int i, j, k;
  int top, len;

  _lab_voice_mode = voice_mode;

  for(t = 0; t < nums; t++)
    {
      /* 音素境界でなければ処理をスキップ */
      if(!(p_list[t]->status & ST_VSEG)) continue;

      /*
       * 音素領域についての情報をまとめる
       */
      /* 次の音素境界までのフレーム数をカウントする */
      top = t;
      len = 0;
      for(i = t + 1; i < nums; i++)
	{
	  len++;
	  if(p_list[i]->status & ST_VSEG) break;
	}

      if(mGroupDisp)
	{
	  printf("%5d:", top + skip);
	  switch(p_list[top]->group)
	    {
	    case V_W:  printf("  W"); break;
	    case V_D:  printf("  D"); break;
	    case V_C:  printf("  C"); break;
	    case V_V:  printf("  V"); break;
	    case V_F:  printf("  F"); break;
	    case V_U:  printf("  U"); break;
	    case V_S:  printf("  S"); break;
	    }
	  printf("\n");
	}
    }

  /*
   * 音素群ラベルをたよりに、音素列化されたテキスト文字列を、
   * 時系列に当てはめ,口パクのシーケンスに必要な出力を作り出す。
   */
  if(!mGroupDisp)
    lab_labeling(skip, tx_list, p_list, nums);

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
      …と考える。

*/

/*
 * 音素境界リストをつくる
 */
static labVList * lab_create_border_list(int skip,
					 anaParam ** p_list, int nums)
{
  labVList * v_list;
  int        v_nums;
  int t;
  int i, j;
  double power, max_power;

  v_nums = 0;

  for(t = 0; t < nums; t++)
    if(IsVoiceSegment(p_list[t]->status)) v_nums++;
  /* 
    else if(p_list[t]->status & ST_SILENT)
      {
	if(t > 0)
	  {
	    if(!(p_list[t - 1]->status & ST_SILENT))
	      v_nums++;
	  }
	else v_nums++;
      }
  */

  v_nums += 2;  /* 先頭無音部と、末尾無音部の分を追加 */

  if(NULL == (v_list = malloc(sizeof(labVList) * (v_nums + 1))))
    return NULL;

  /* 先頭空白をつくる */
  v_list[0].label = NULL;
  v_list[0].group = V_S;
  v_list[0].t     = skip;
  v_list[0].time  = 0;
  i = 1;
  max_power = 0;
  for(t = 0; t < nums; t++)
    if(IsVoiceSegment(p_list[t]->status))
      {
	v_list[i].label = NULL;
	v_list[i].group = p_list[t]->group;
	v_list[i].t     = t + skip;
	v_list[i].time  = 1;
	power = p_list[t]->Ci;
	for(j = t + 1; j < nums && !(p_list[j]->status & ST_VSEG); j++)
	  {
	    v_list[i].time++;
	    power += p_list[j]->Ci + 4;
	  }
	/* 零次メルケプストラム係数平均をパワーとして用いる */
	v_list[i].power = power / v_list[i].time;
	/* 最大のパワー値を得る */
	if(v_list[i].power > max_power) max_power = v_list[i].power;
	i++;
      }

  /* 末尾空白をつくる */
  v_list[i].label = NULL;
  v_list[i].group = V_S;
  v_list[i].t     = nums - 1;
  v_list[i].time  = 0;
  v_list[i].power = 0;

  i++;
  /* パワー値を、最大 1.0 として、全体的な比を求める */
  for(j = 0; j < i; j++)
    {
      if(v_list[j].power > 0)
	v_list[j].power = v_list[j].power / max_power;
      else
	v_list[j].power = 0;
    }

  /* データ終端をつくる */
  v_list[i].group = V_unknown;
  v_list[i].t = -1;

  fprintf(stderr, "create list.\n"); fflush(stderr);
  return v_list;
}

static int lab_group2num(vGroup g)
{
  int ret = -1;

  switch(g)
    {
    case V_W: ret = 0; break;
    case V_D: ret = 1; break;
    case V_C: ret = 2; break;
    case V_V: ret = 3; break;
    case V_F: ret = 4; break;
    case V_U: ret = 5; break;
    case V_S: ret = 6; break;
    case V_unknown: ret = -1; break;
    }
  return ret;
}

/*
 * 音素群を比較する。
 * sample で指定したものが、cmp の範疇に含まれるかどうかを判定する。
 *
 * 結果:
 *      -1     エラー
 *       0     全くの別物
 *       1     一応含まれる
 *       2     そのもの
 */
static int lab_cmp_group(vGroup sample, vGroup cmp)
{
  int n_cmp, n_sample;
  int ret = 0;
  static int matrix[7][7] = {
    2, 0, 0, 1, 0, 0, 0,
    0, 2, 1, 1, 0, 0, 0,
    0, 0, 2, 1, 0, 0, 0,
    0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 2, 1, 0,
    0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 2
  };

  if((n_cmp    = lab_group2num(cmp)) < 0) return -1;
  if((n_sample = lab_group2num(sample)) < 0) return -1;

  return matrix[n_sample][n_cmp];
}

/* 指定したテキスト音素に、指定した音素群属性がつけられているかを判定 */
static int lab_check_group(txtPhono *tx, vGroup g)
{
  int i;
  int ret;

  ret = 0;
  for(i = 0; tx->gr_nums; i++)
    if(ret = lab_cmp_group(tx->group[i], g)) return ret;
  return 0;
}


/*
 * ラベリングを行なう(音素境界の欠落、挿入等も考慮する)
 */
static int lab_labeling(int skip,
			txtPhono **t_list, anaParam **p_list, int nums)
{
  labVList * v_list;
  int v_len;
  int i, j;

  /* 音素区間リストを作成 */
  v_list = lab_create_border_list(skip, p_list, nums);

  /* 音素区間リストを、無音区間のある箇所で区切り,
     区切られた範囲に含まれる音素セグメントを,
     パターンマッチングでラベリングする */
  i = 0;
  while(v_list[i].t >= 0)
    {
      if(v_list[i].group != V_S)
	{
	  i++;
	  continue;
	}
      v_list[i].label = "sil";

      for(j = i + 1; v_list[j].t >= 0; j++)
	if(v_list[j].group == V_S) break;

      v_len = j - i;


      /* 無音に区切られた区間内にあるセグメントのラベリング */
      matMatching(skip, p_list, nums, v_list, i, v_len);
      i += v_len;
    }

  /*
   * 割り当てられたラベルを時系列とともに表示
   */
  if(!mDicDisp)
    {
      for(i = 0; v_list[i].t >= 0; i++)
	if(v_list[i].label != NULL)
	  printf("%5d:  %2s\t%8.3f\n",
		 v_list[i].t, v_list[i].label, v_list[i].power);
	else
	  printf("%5d:  sil\t%8.3f\n", v_list[i].t, 0);
    }
  else
    {
      int num;
      char * g;
      static char *group[] = { "W", "D", "C", "V", "F", "U", "S" };
      for(i = 0; v_list[i].t >= 0; i++)
	{
	  num = lab_group2num(v_list[i].group);
	  g = (num < 0) ? "S" : group[ num ];
	  if(v_list[i].label != NULL)
	    printf("%5d\t%s\t%2s\t%8.3f\n",
		   v_list[i].t, g, v_list[i].label, v_list[i].power);
	  else
	    printf("%5d\t%s\tsil\t%8.3f\n", v_list[i].t, g, 0.0F);
	}
	
    }
  free(v_list);

  return 0;
}

/*
 * 音素ラベルから、母音 ID を求めて返す
 */
static int lab_label2id(char *label)
{
  size_t pos;

  /* 検査対象とするのは、音素ラベルの最後の一文字のみ */
  pos = strlen(label) - 1;

  if(!strcmp(label + pos, "a")) return 0;
  else if(!strcmp(label + pos, "i")) return 1;
  else if(!strcmp(label + pos, "u")) return 2;
  else if(!strcmp(label + pos, "e")) return 3;
  else if(!strcmp(label + pos, "o")) return 4;

  return -1;  /* 五つの母音の中には含まれない( /N/ など ) */
}

/*
 * 母音 ID から、対応する音素ラベルを求める
 */
static char * lab_id2label(int id)
{
  static char * list[] = {"a", "i", "u", "e", "o"};
  if(id > 4 || id < 0) return NULL;
  return list[id];
}


/*
 * 空白で区切られた有音区間の音素と音素境界のマッピングを行なう
 */
static int lab_matching(int skip, anaParam **p_list, int nums,
			txtPhono ** t_list, int t_top, int t_len,
			labVList *  v_list, int v_top, int v_len)
{
  int v_wcnt;  /* 音声側母音的セグメントのカウンタ     */
  int t_wcnt;  /* テキスト側母音的セグメントのカウンタ */
  int i, j, k;
  labVowel *t_vow;
  labVowel * v_vow;
  double     best, score;
  int best_c, id;
  int pre, pre_time;
  int pre_v, pre_t;
  vowelDistance *vowel;
  int vow_id;
  int vp;
  int ret;
  int begin, end;
  double **lattice;
  int pre_id;
  int term_vid, term_tid;

  /*
   * デバッグのために,テキスト側音素列を表示させてみる
   */
#ifdef _DEBUG_
  printf("debug: ");
  for(i = 0; i < t_len; i++)
    printf(" %s", t_list[i + t_top]->label);
  printf("\n");
#endif

  term_tid = t_top + t_len;
  term_vid = v_top + v_len;

  /*
   * それぞれの母音的セグメントの数を数える
   */
  v_wcnt = 0;
  pre = 0;
  pre_t = 0;
  for(i = 0; i <= v_len; i++)
    {
      if((v_list[v_top + i].group == V_W) ||
	 (v_list[v_top + i].group == V_V))
	{
	  DEBUG("segment %d - %d\n", v_list[v_top + i].t, v_list[v_top + i].time);
	  /*
	   * 母音的セグメントの音韻識別を行い、結果を保持しておく
	   */
#ifndef _NO_VOWEL_
	  ret = vowelJudge(p_list, nums,
			   v_list[v_top + i].t - skip,
			   v_list[v_top + i + 1].t - skip,
			   &(v_list[v_top + i].male),
			   &(v_list[v_top + i].female));
#endif /* _NO_VOWEL_ */
	  v_wcnt++;
	}
    }
  
  DEBUG("v_list counted.\n");
  
  t_wcnt = 0;
  for(i = 0; i <= t_len; i++)
    for(j = 0; j < t_list[t_top + i]->gr_nums; j++)
      if(t_list[t_top + i]->group[j] == V_W) {  t_wcnt++;  break; }
  
  DEBUG("t_list counted.\n");

  /* 記録用バッファを確保する */
  if(NULL == (v_vow = malloc(sizeof(labVowel) * v_wcnt))) goto err_0;
  if(NULL == (t_vow = malloc(sizeof(labVowel) * t_wcnt))) goto err_1;

  DEBUG("memory alloced.\n");

  /* 位置を記録する */  
  v_wcnt = 0;
  pre = 0;
  pre_time = 0;
  begin = v_list[v_top].t;
  end   = v_list[v_top + v_len].t;
  for(i = 0; i < v_len; i++)
    if((v_list[v_top + i].group == V_W) ||
       (v_list[v_top + i].group == V_V))
      {

#ifdef _VPOS_WITH_TIME_
	/* 音素セグメント位置として,時間まで考慮した位置を使用する場合 */
	v_vow[v_wcnt].position =
	  (double)(v_list[v_top + i].t + v_list[v_top + i].time / 2 - begin) / (double)(end - begin);
#else
	/* 音素セグメント位置として,特に時間を考慮しない場合 */
	v_vow[v_wcnt].position = (double)i / (double)v_len;
#endif /* _VPOS_WITH_TIME_ */
	v_vow[v_wcnt].id = i + v_top;
	v_wcnt++;
      }

  DEBUG("v_list position setuped.\n");

  t_wcnt = 0;
  for(i = 0; i < t_len; i++)
    for(j = 0; j < t_list[t_top + i]->gr_nums; j++)
      if(t_list[t_top + i]->group[j] == V_W)
	{
	  t_vow[t_wcnt].position = (double)i / (double)t_len;
	  t_vow[t_wcnt].id = t_top + i;
	  t_wcnt++;  break;
	}

  DEBUG("t_list position setuped.\n");

  /*
   * 音素セグメント数 x テキスト文音素から,2次元の採点用ラティスを作成する
   */
  /* 行数(テキスト文母音的音素数) */
  if(NULL == (lattice = malloc(sizeof(double *) * t_wcnt))) goto err_2;
  
  /* 列数(母音的音素セグメント数) */
  for(i = 0; i < t_wcnt; i++)
    {
      if(NULL == (lattice[i] = malloc(sizeof(double) * v_wcnt)))
	{
	  for(j = 0; j < i; j++) free(lattice[j]);
	  goto err_3;
	}
      /* Lattice を初期化しておく */
      for(j = 0; j < v_wcnt; j++) lattice[i][j] = 0.0;
    }

  /* 位置関係の照合、音韻認識などで採点し、lattice 内を満たしていく */
  /*
    判断基準は以下の通り:

    [1]位置的に似通っている

    [2]音韻識別の結果が,与えられているラベルに等しい,
       もしくは極めて近い(/a/ と /wa/, /ya/ など)
  */
  for(i = 0; i < v_wcnt; i++)
    {
#ifndef _NO_VOWEL_
      vowel = (_lab_voice_mode == VMODE_MALE)
	? &v_list[v_vow[i].id].male
	: &v_list[v_vow[i].id].female;
#endif /* _NO_VOWEL_ */
      best = 1.0;
      best_c = -1;
      for(j = 0; j < t_wcnt; j++)
	{
	  
#ifndef _NO_VOWEL_
	  
	  /* 音韻識別の結果と照合する */
	  if((vowel->distance < DIST_MAX) || (vowel->id < 0))
	    {
	      /* 音韻識別の際の距離が DIST_MAX 以上の場合,
		 識別結果は信頼性に欠けるとみなし,比較対象にしない。 */
	      if(lab_label2id(t_list[t_vow[j].id]->label) != vowel->id)
		lattice[j][i] += DITECT_WAIT;  /* 音韻識別の結果が異なる場合は
						  ペナルティ値を与える */
	    }
	  
#endif /* _NO_VOWEL_ */
	  
	  /* 位置を比較し,照合対象との距離差をそのまま得点とする */
	  score = ABS(v_vow[i].position - t_vow[j].position);
	  lattice[j][i] += score;
	}
    }

  /* 採点の結果, もっとも低い得点のものを割り当てていく
   * (得点が低い程,照合対象に近いという意味合になる)    */
  pre_id = -1;
  for(i = 0; i < v_wcnt; i++)
    {
      best = lattice[0][i];
      best_c = 0;
      for(j = 0; j < t_wcnt; j++)
	if(lattice[j][i] < best)
	  {
	    best = lattice[j][i];
	    best_c = j;
	  }

      /* 直前の音素よりも前の音素が割り当てられているならば,
	 直前の音素とおなじものであるとみなす */
      if(best_c < pre_id) best_c = pre_id;


      /* 直前の音素と同じテキスト文中音素が割り当てられていれば,
	 同じ音素が継続されているとみなす */
      if(best_c == pre_id) continue;

      /* 直前の母音的音素と,今回の母音的音素の間にはさまれた子音を処理する */
      {
	int vb, vlen;
	int tb, tlen;

	tb = (pre_id < 0) ? t_top : (t_vow[pre_id].id + 1);
	tlen = t_vow[best_c].id - tb;
	
	vb = (i == 0) ? v_top : v_vow[i - 1].id;
	vlen = v_vow[i].id - vb;
	lab_matching_consonant(t_list, tb, tlen, v_list, vb, vlen);
      }
      
      /* 最高得点の音素を,そのセグメントの音素として割り当て,出力する */
      printf("%5d: %2s\t%8.3f\n",
	     v_list[v_vow[i].id].t,
	     t_list[t_vow[best_c].id]->label,
	     v_list[v_vow[i].id].power);
      pre_id = best_c;
    }

  /* 最後のの母音的音素と,区切りとなる無音状態の間にはさまれた子音を処理する */
  {
    int vb, vlen;
    int tb, tlen;
    
    tb = (pre_id < 0) ? t_top : (t_vow[pre_id].id + 1);
    tlen = term_tid - tb;
    
    vb = (i == 0) ? v_top : v_vow[i - 1].id;
    vlen = term_vid - vb;
    lab_matching_consonant(t_list, tb, tlen, v_list, vb, vlen);
  }
  

  /* メモリリソースの開放 */
  for(i = 0; i < t_wcnt; i++) free(lattice[i]);
  free(lattice);
  free(t_vow);
  free(v_vow);
  return 0;  

 err_3:
  free(lattice);
 err_2:
  free(t_vow);
 err_1:
  free(v_vow);
 err_0:
  return -1;
}


/*
 * 母音～母音に囲まれた領域内の音素セグメントに、テキストから
 * 得られた子音のラベルを与える。
 */
static int lab_matching_consonant(txtPhono ** t_list, int t_top, int t_len,
				  labVList *  v_list, int v_top, int v_len)
{
  int v_cnt, t_cnt;  /* テキスト側子音、音声側子音的セグメントのカウンタ */
  int i, j, k;
  labConsonant * v_con;
  labConsonant * t_con;
  double best, score;
  int    best_id;
  double **lattice;

  /*
   * 音声側子音的セグメントをカウントする。
   * 子音的セグメントとしてカウントされるのは、
   *    D  有声子音セグメント
   *    C  有声子音的セグメント
   *    V  有声音的セグメント
   *    F  無声摩擦音的セグメント
   *    U  無声子音的セグメント
   * の 5つ。
   */
  v_cnt = 0;
  for(i = 0; i < v_len; i++)
    if((v_list[v_top + i].group == V_D) ||
       (v_list[v_top + i].group == V_C) ||
       (v_list[v_top + i].group == V_V) ||
       (v_list[v_top + i].group == V_F) ||
       (v_list[v_top + i].group == V_U)) v_cnt++;


  /*
   * テキスト側の子音をカウントする。
   */
  t_cnt = 0;
  for(i = 0; i < t_len; i++)
    for(j = 0; j < t_list[t_top + i]->gr_nums; j++)
      if((t_list[t_top + i]->group[j] == V_D) ||
	 (t_list[t_top + i]->group[j] == V_C) ||
	 (t_list[t_top + i]->group[j] == V_V) ||
	 (t_list[t_top + i]->group[j] == V_F) ||
	 (t_list[t_top + i]->group[j] == V_U))
	{
	  t_cnt++;
	  break;	  
	}

  if(!t_cnt) return 0;  /* テキスト側に子音が無いのであれば、
			   そのまま正常終了とする            */

  /*
   * 位置バッファの確保
   */
  if(NULL == (t_con = malloc(sizeof(labConsonant) * t_cnt))) goto err_0;
  if(NULL == (v_con = malloc(sizeof(labConsonant) * v_cnt))) goto err_1;

  /*
   * 位置バッファに、それぞれの位置を記録する
   */
  v_cnt = 0;
  for(i = 0; i < v_len; i++)
    if((v_list[v_top + i].group == V_D) ||
       (v_list[v_top + i].group == V_C) ||
       (v_list[v_top + i].group == V_V) ||
       (v_list[v_top + i].group == V_F) ||
       (v_list[v_top + i].group == V_U)) 
      {
	v_con[v_cnt].position = (double)i / (double)v_len;
	v_con[v_cnt].id = i + v_top;
	v_cnt++;
      }


  /*
   * テキスト側の子音を位置バッファに登録
   */
  t_cnt = 0;
  for(i = 0; i < t_len; i++)
    for(j = 0; j < t_list[t_top + i]->gr_nums; j++)
      if((t_list[t_top + i]->group[j] == V_D) ||
	 (t_list[t_top + i]->group[j] == V_C) ||
	 (t_list[t_top + i]->group[j] == V_V) ||
	 (t_list[t_top + i]->group[j] == V_F) ||
	 (t_list[t_top + i]->group[j] == V_U))
	{
	  t_con[t_cnt].position = (double)i / (double)t_len;
	  t_con[t_cnt].id       = i + t_top;
	  t_cnt++;
	  break;
	}



  /*
   * 得られた位置から、子音的セグメントに対応する子音ラベルを推測する
   */

  /* 得点ラティス用バッファを確保 */
  if(NULL == (lattice = malloc(sizeof(double *) * t_cnt))) goto err_2;
  for(i = 0; i < t_cnt; i++)
    {
      if(NULL == (lattice[i] = malloc(sizeof(double) * v_cnt)))
	{
	  for(j = 0; j < i; j++) free(lattice[j]);
	  goto err_3;
	}
      for(j = 0; j < v_cnt; j++) lattice[i][j] = 0.0; /* 初期化 */
    }

  /* ラティスにスコアを格納 */
  for(i = 0; i < v_cnt; i++)
    {
      for(j = 0; j < t_cnt; j++)
	{
	  score = ABS(t_con[j].position - v_con[i].position);
	  lattice[j][i] = score;
	}
    }

  /* 各音素セグメントについて、位置的に最も似通っている子音を割り当てる */
  for(i = 0; i < v_cnt; i++)
    {
      best = 1.0;
      best_id = -1;
      for(j = 0; j < t_cnt; j++)
	if(lattice[j][i] < best)
	  {
	    best = lattice[j][i];
	    best_id = j;
	  }

      /* 割り当てられた音素のラベルを出力 */
      printf("%5d: %2s\t%8.3f\n",
	     v_list[v_con[i].id].t,
	     t_list[t_con[best_id].id]->label,
	     v_list[v_con[i].id].power);
      
    }

  for(i = 0; i < t_cnt; i++) free(lattice[i]);
  free(lattice);
  free(v_con);
  free(t_con);
  return 0;
 err_3:
  free(lattice);
 err_2:
  free(v_con);
 err_1:
  free(t_con);
 err_0:
  return -1;
}
