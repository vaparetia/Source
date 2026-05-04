/*
	watcher.c
	見張り兵ヘッダ

	1997/07/07 Y.Korekado
	$Id: support.h,v 1.1.1.3 2002/11/19 11:44:25 Yoshizawa1 Exp $
	
*/
#ifndef __WATCHER___
#define	__WATCHER___ 1


/*-----	外部変数	-----*/
/*-----	定数定義	-----*/
/*----- ワーク定義 -----*/
typedef	struct	{
	GV_ACT_EX		actor ;
	CONTROL		control ;
	OBJECT		body ;
	OBJECT		weapon ;
	OBJECT		sub_weapon ;
	OBJECT		sling ;
	FMATRIX		lights[2] ;

	/* 暫定モーションデータ */
	int			motion_data[ MOTION_MAX ];

	/* アクション */
	ACTION		action ;

	/* ルートナビ */
	ROUTENAVI	routenavi ;

	/* ゾーンナビ */
	ZONENAVI	zonenavi ;

	/* 敵兵思考 */
	ENETHINK	enethink ;

} Work ;

#endif
