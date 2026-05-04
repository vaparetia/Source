#ifndef _lip_japanese_h_
#define _lip_japanese_h_

/*
 * 組み込み用のシーケンスでは音素記号を文字列では扱わず,
 * 音素記号に対応する ID で扱う
 */
/* 母音 */
#define PHJP_a      1
#define PHJP_i      2
#define PHJP_u      3
#define PHJP_e      4
#define PHJP_o      5
#define PHJP_N      6

/* 半母音を含む母音 */

/* /wa/ は、/u/ の形状から母音形状へ */
#define PHJP_wa     7

/* /ya/,/yu/,/yo/ は /i/ の形状から母音形状へ */
#define PHJP_ya     8
#define PHJP_yu     9
#define PHJP_yo    10

/* 子音 */
/* 以下の音素は、後続母音の形状になる。*/
#define PHJP_k     11
#define PHJP_h     12
#define PHJP_r     13
#define PHJP_g     14

/* 以下の音素は、/u/ の形状から母音形状へ変化する
   (音素の形状は /u/ として扱う ) */
#define PHJP_s     15
#define PHJP_t     16
#define PHJP_n     17
#define PHJP_z     18
#define PHJP_ts    19
#define PHJP_d     20
#define PHJP_w     21

/* 以下の音素は、閉じ形状から母音形状へ変化する */
#define PHJP_m     22
#define PHJP_b     23
#define PHJP_p     24

/* 以下の音素は,/i/ の形状から母音形状へ変化する */
#define PHJP_y     25
#define PHJP_j     26

#endif /* _lip_japanese_h_ */
