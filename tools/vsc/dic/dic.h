#ifndef _dic_h_
#define _dic_h_

#include "vec.h"
#include "group.h"

typedef enum {
  SS_W,    /* W, /N/, /w/ の定常部                   */
  SS_C,    /* C, D, V (W 除く) の定常部              */
  SS_U,    /* U, F の定常部                          */
  SS_SV,   /* S-(V,W) の境界の過渡部                 */
  SS_CW,   /* (C,D)-(V,W) , V-W の境界の過渡部       */
  SS_UW,   /* (U,F)-(V,W) の境界の過渡部             */
  SS_UU,   /* (U,F)-(U,F) の境界の過渡部             */
  SS_F,    /* 後続音がある6フレーム以上の F,U の全体 */
  SS_unknown
} vSubset;


/*
 * リファレンスパターン項目
 */
typedef struct _dicREF {
  vSubset   subset;     /* 辞書のサブセット                           */
  char    * label;      /* 音素ラベル                                 */
  vecCep    vector[5];  /* リファレンスパターンとなるメルケプストラム */
} dicREF;

/*
 * リファレンスパターンをまとめたサブセット
 */
typedef struct _dicSubSET {
  int       nums;  /* 収録数                             */
  dicREF ** ref;   /* 収録されているリファレンスパターン */
} dicSubSET;

typedef struct _dicDicROOT {
  dicSubSET W;   /* リファレンスサブセット W  */
  dicSubSET C;   /* リファレンスサブセット C  */
  dicSubSET U;   /* リファレンスサブセット U  */
  dicSubSET SV;  /* リファレンスサブセット SV */
  dicSubSET CW;  /* リファレンスサブセット CW */
  dicSubSET UW;  /* リファレンスサブセット UW */
  dicSubSET UU;  /* リファレンスサブセット UU */
  dicSubSET F;   /* リファレンスサブセット F  */
} dicDicROOT;

/*
 * functions
 */
dicREF     ** dicMatchVectorAndGroup(vecVector ** v_list, grGroup **g_list);
int           dicAppendDictionaly(dicDicROOT *root, dicREF **r_list);
dicDicROOT  * dicReadDictionaly(TOKEN *tkn);
int           dicWriteDictionaly(FILE *wfp, dicDicROOT *dic);
dicDicROOT  * dicNewDictionaly(void);

#endif /* _dic_h_ */
