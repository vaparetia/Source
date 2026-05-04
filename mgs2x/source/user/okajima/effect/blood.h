/*
	blood.h
	血ヘッダ
	1999/10/08 S.Okajima
	$Id: blood.h,v 1.1.1.3 2002/11/19 11:46:57 Yoshizawa1 Exp $
*/

/* 赤色の血の設定値（減算前提） */
/*----------------------------------------------------------------*/

#if 0
// 減算用（前回パラメータ）
#define	COLOR_R			(32)
#define	COLOR_G			(100)
#define	COLOR_B			(100)
#else
#define	COLOR_R			(16)
#define	COLOR_G			(0)
#define	COLOR_B			(0)
#endif

#ifdef ENGLISH
#define	BLOOD_ALPHA_DEMO	(32)
#define	BLOOD_ALPHA_0		(64)
#define	BLOOD_ALPHA_1		(128)	// 壁血、バイオ血
#else
#define	BLOOD_ALPHA_DEMO	(16)
#define	BLOOD_ALPHA_0		(32)
#define	BLOOD_ALPHA_1		(64)	// 壁血、バイオ血
#endif

/*----------------------------------------------------------------*/

#define	BLOOD_MODE_OFF	(0)
#define	BLOOD_MODE_ON	(1)

extern int option_blood_mode;


