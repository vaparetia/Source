/*

	bubble.h
	泡共通ヘッダ
	2000/12/26 S.Okajima
	$Id: bubble.h,v 1.1.1.3 2002/11/19 11:47:17 Yoshizawa1 Exp $

*/

#if 1
#define	BUBBLE_COL_R			(96)
#define	BUBBLE_COL_G			(102)
#define	BUBBLE_COL_B			(128)
#else
#define	BUBBLE_COL_R			(128)
#define	BUBBLE_COL_G			(128)
#define	BUBBLE_COL_B			(128)
#endif

#define	BUBBLE_ALPHA			(32)

#define	BUBBLE_SIZE_MIN			(10.0f)
#define	BUBBLE_SIZE_RAND		(10.0f)
#define	BUBBLE_SPEED			( -P_GRAVITY / 4.0f )

