/*
	watcher.c
	見張り兵ヘッダ

	1997/07/07 Y.Korekado
	$Id: watcher.h,v 1.1.1.3 2002/11/19 11:44:30 Yoshizawa1 Exp $
	
*/
#ifndef __WATCHER___
#define	__WATCHER___ 1


/*-----	外部変数	-----*/
extern	int		GM_PlayerAddress ;
extern	FVECTOR	GM_PlayerPosition;

/*-----	定数定義	-----*/
/*----- ワーク定義 -----*/
typedef	struct	{
	ENE_ARGS	notice_proc ;	/* 気づいた時に呼ばれるプロック */
} WATCHWORK ;

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

	/* 固有ワーク */
	WATCHWORK	watchwork ;
} Work ;


/*----- 外部関数宣言 -----*/
extern void ENE_Watcher_Think1_Alert( ENETHINK *entk ) ;
extern void ENE_WatcherStartModeAlert( ENETHINK *entk ) ;
extern void ENE_Watcher_Think1_Sneak( ENETHINK *entk ) ;
extern void ENE_WatcherStartModeSneak( ENETHINK *entk ) ;

extern void ENE_WatcherResurrect( ENETHINK *entk ) ;
extern void ENE_Watcher_Think1_Avoid( ENETHINK *entk ) ;
extern void ENE_WatcherResurrectionMode( ENETHINK *entk ) ;
extern void ENE_WatcherStartModeAvoidDamage( ENETHINK *entk ) ;
extern void ENE_WatcherStartModeAvoid( ENETHINK *entk ) ;

#endif
