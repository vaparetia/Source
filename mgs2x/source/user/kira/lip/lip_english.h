#ifndef _lip_english_h_
#define _lip_english_h_

#define ENG_AE_RATE   0.5    /* ae のブレンド率。
				値が大きければ /E/, 
				小さければ /A/ に近くなる(0～1の間で設定) */

/* 各音素に対応する ID 
 * 注釈の表記は Klattese を用いている。マクロ名についてもこれに準ずるが、
 * マクロ名として使用できない記号については、別のものに置き換えてある。
 */
#define PHEN_at    1      /* @  */
#define PHEN_E     2      /* E  */
#define PHEN_I     3      /* I  */
#define PHEN_i     4      /* i  */
#define PHEN_A     5      /* A  */
#define PHEN_R     6      /* R  */
#define PHEN_c     7      /* c  */
#define PHEN_a     8      /* a  */
#define PHEN_U     9      /* U  */
#define PHEN_u     10     /* u  */
#define PHEN_aI    11     /* aI */
#define PHEN_aU    12     /* aU */
#define PHEN_EI    13     /* EI */
#define PHEN_cI    14     /* cI */
#define PHEN_cU    15     /* cU */
#define PHEN_l     16     /* l  */
#define PHEN_y     17     /* y  */
#define PHEN_w     18     /* w  */
#define PHEN_r     19     /* r  */
#define PHEN_bar   20     /* |  */
#define PHEN_x     21     /* x  */
#define PHEN_X     22     /* X  */
#define PHEN_L     23     /* L  */
#define PHEN_M     24     /* M  */
#define PHEN_N     25     /* N  */
#define PHEN_m     26     /* m  */
#define PHEN_n     27     /* n  */
#define PHEN_G     28     /* G  */
#define PHEN_V     29     /* V  */
#define PHEN_D     30     /* D  */
#define PHEN_z     31     /* z  */
#define PHEN_Z     32     /* Z  */
#define PHEN_J     33     /* J  */
#define PHEN_b     34     /* b  */
#define PHEN_d     35     /* d  */
#define PHEN_g     36     /* g  */
#define PHEN_h     37     /* h  */
#define PHEN_C     38     /* C  */
#define PHEN_S     39     /* S  */
#define PHEN_s     40     /* s  */
#define PHEN_T     41     /* T  */
#define PHEN_f     42     /* f  */
#define PHEN_p     43     /* p  */
#define PHEN_t     44     /* t  */
#define PHEN_k     45     /* k  */


#endif /* _lip_english_h_ */

