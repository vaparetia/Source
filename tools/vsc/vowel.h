#ifndef _vowel_h_
#define _vowel_h_

/*
 * 第一、第二フォルマントの位置(デフォルト基準値)
 */
/* 男声 */
#define VOW_M_1st_A  750
#define VOW_M_2nd_A  1100

#define VOW_M_1st_I  300
#define VOW_M_2nd_I  2180

#define VOW_M_1st_U  310
#define VOW_M_2nd_U  1050

#define VOW_M_1st_E  480
#define VOW_M_2nd_E  1820

#define VOW_M_1st_O  470
#define VOW_M_2nd_O  830

/* 女声 */
#define VOW_F_1st_A  1100
#define VOW_F_2nd_A  1500

#define VOW_F_1st_I  310
#define VOW_F_2nd_I  2750

#define VOW_F_1st_U  350
#define VOW_F_2nd_U  1200

#define VOW_F_1st_E  550
#define VOW_F_2nd_E  2400

#define VOW_F_1st_O  530
#define VOW_F_2nd_O  900


typedef struct voice_vector {
  double formant_1st;
  double formant_2nd;
} VoiceVector;


typedef struct vector_set {
  VoiceVector vA;   /* /a/ のベクトル */
  VoiceVector vI;   /* /i/ のベクトル */
  VoiceVector vU;   /* /u/ のベクトル */
  VoiceVector vE;   /* /e/ のベクトル */
  VoiceVector vO;   /* /o/ のベクトル */
} VectorSet;

typedef struct vowel_distsance {
  int id;   /* 判定結果の母音 ID */
  double distance;
} vowelDistance;

#ifdef _vowel_c_
#define EXT
#define INIT(n)  = n
#else
#define EXT extern
#define INIT(n)
#endif

EXT VectorSet vecMale;    /* 男声用母音特徴ベクトル */
EXT VectorSet vecFemale;  /* 女声用母音特徴ベクトル */

#undef EXT
#undef INIT

void vowelInit(void);
int  vowelJudge(anaParam **p_list, int nums, int begin, int end,
		vowelDistance * male, vowelDistance * female);

#endif /* _vowel_h_ */
