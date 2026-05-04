/*
	corps.h
	死体

	2000/05/30 Y.Korekado
	$Id: corps.h,v 1.1.1.3 2002/11/19 11:44:02 Yoshizawa1 Exp $
*/

#define	CORPS_ST_ACT_CTRL		0x00000001	/* 動かされている */
#define	CORPS_ST_ROTTEN_START	0x00000002	/* 腐りはじめている */
#define	CORPS_ST_WATCH			0x00000004	/* 発見された */
#define	CORPS_ST_ROTTEN			0x00000008	/* 腐った */

#define	CORPS_ST_SPOT_SHADOW	0x00000010	/* スポットライトで影が出る */
#define	CORPS_ST_LIVE			0x00000020	/* 生きている */
#define	CORPS_ST_FACE_MOT		0x00000040	/* farを使い目を瞑る */

typedef void	( *CORPS_CALLBACK )( void *, OBJECT *, CONTROL * ) ;
