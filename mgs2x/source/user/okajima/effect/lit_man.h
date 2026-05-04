/*
	lit_man.h
	光源位置制御ヘッダ

	1999/10/16 S.Okajima
	$Id: lit_man.h,v 1.1.1.3 2002/11/19 11:47:07 Yoshizawa1 Exp $

*/

#define	MAX_LIGHT_NUM	(64)

extern	int			ok_lit_data_num;				/* 実データの個数 */
extern	FVECTOR		*ok_lit_pos[ MAX_LIGHT_NUM ] ;
extern	SVECTOR		*ok_lit_rot[ MAX_LIGHT_NUM ] ;
extern	FVECTOR		*ok_lit_pos_sorted[ MAX_LIGHT_NUM ] ;	/* 前詰め */
extern	SVECTOR		*ok_lit_rot_sorted[ MAX_LIGHT_NUM ] ;	/* 前詰め */

