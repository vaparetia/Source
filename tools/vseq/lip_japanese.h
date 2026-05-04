#ifndef _lip_japanese_h_
#define _lip_japanese_h_

/*
 * 組み込み用のシーケンスでは音素記号を文字列では扱わず,
 * 音素記号に対応する ID で扱う
 */
/* 母音 */
#define LJP_a      1
#define LJP_i      2
#define LJP_u      3
#define LJP_e      4
#define LJP_o      5
#define LJP_N      6

/* 半母音を含む母音 */

/* /wa/ は、/u/ の形状から母音形状へ */
#define LJP_wa     7

/* /ya/,/yu/,/yo/ は /i/ の形状から母音形状へ */
#define LJP_ya     8
#define LJP_yu     9
#define LJP_yo    10

/* 子音 */
/* 以下の音素は、後続母音の形状になる。*/
#define LJP_k     11
#define LJP_h     12
#define LJP_r     13
#define LJP_g     14

/* 以下の音素は、/u/ の形状から母音形状へ変化する
   (音素の形状は /u/ として扱う ) */
#define LJP_s     15
#define LJP_t     16
#define LJP_n     17
#define LJP_z     18
#define LJP_ts    19
#define LJP_d     20
#define LJP_w     21

/* 以下の音素は、閉じ形状から母音形状へ変化する */
#define LJP_m     22
#define LJP_b     23
#define LJP_p     24

/* 以下の音素は,/i/ の形状から母音形状へ変化する */
#define LJP_y     25
#define LJP_j     26

#endif /* _lip_japanese_h_ */
