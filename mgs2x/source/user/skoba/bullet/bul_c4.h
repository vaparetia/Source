/*
   bul_c4.h
   Ｃ４管理ヘッダー

   2001/03/07	M.Sonoyama
   $Id: bul_c4.h,v 1.1.1.3 2002/11/19 11:50:00 Yoshizawa1 Exp $
*/

#ifndef __bul_c4_h__
#define	__bul_c4_h__

enum {
    FLAG_PLAYER = 		0x0001,
    FLAG_SCENARIO = 	0x0002,
	FLAG_KAITAI =		0x0004,
    FLAG_FREEZE =		0x0010,
    FLAG_DESTROY =		0x0020,
    FLAG_FREE =			0x0040,
	FLAG_NO_SMELL =		0x0080,
	FLAG_SMELL =		0x0100,
	FLAG_SWITCH =		0x0200,	
	FLAG_FALL =			0x0400,	/* その場に落ちる */
	FLAG_COUNTDOWN =	0x0800,
	FLAG_FATMAN =		0x1000,	/* ファットマンがしかける奴 */
	FLAG_NO_SPRAY =		0x2000,	/* スプレーが効かない */
	FLAG_BELTOBJ =		0x4000,	/* ベルトコンベア荷物についている */
	FLAG_NO_BLAST =		0x8000,

	FLAG_LIKE_CEILING =	0x10000, /* 自分の高さよりカメラが上になると非表示 */
} ;

enum {
	LAMP_STATE_FREEZING	= 		0x01,		/* 凍らされ中 */
	LAMP_STATE_SE_ENABLE =		0x02,		/* ランプＳＥ鳴らす */
	LAMP_SE_TYPE0 =				0x00,
	LAMP_SE_TYPE1 =				0x04,
	LAMP_SE_TYPE2 =				0x08,
	LAMP_SE_ALL =				0x0c,
} ;

#define	LAMP_BLINK_COUNT	(15)					/* ランプエフェクトの点滅間隔 */
#define	LAMP_COLOR_MAX		(LAMP_BLINK_COUNT-1)	/* ランプエフェクトの明るさ最大 */

/*------------------------------------------------------------*/

typedef	struct 	{
	FVECTOR		mov ;
	SVECTOR		rot ;
	int			model ;
	int			joint ;
} OBJINFO ;

typedef struct _Work {
    GV_ACT_EX		actor ;
    OBJECT			body ;
    FMATRIX			world ;
	FMATRIX			lights[ 2 ] ;
	FVECTOR			shift ;
	FVECTOR			lamp_shift ;
	RADAR_CTRL		radar ;
    TARGET			target ;	
	SVECTOR			shift_rot ;
	void			*stick ;
	OBJINFO			*objinfo ;
	int				model ;
    int				type ;
    int				side ;
    int				map ;
    int				flag ;
    int				delay ;
	int				cold_count ;
	int				id ;
	int				n_incremental ;
	int				freeze_disp ;
	int				disp_x, disp_y ;
	int				proc ;
	int				name ;
	int				enemy_joint ;
	int				blast_count ;
	float			scale ;
    struct _Work	*next ;
	ENEFIND 		ef ;
	GM_BOMB			this ;
	HZX_D_CALLBACK	dhcb ;
	int				effect_name ;
	FMATRIX			*effect_world ;
	DG_OBJS			*effect_objs ;
	int				*effect_map ;
	void			*effect_ice ;
	char			lamp_status ;
	char			lamp_count ;
} Work ;

typedef	struct	{
	GV_ACT			actor ;
	int				n_bombs ;
	int				n_incremental ;
	Work			list ;
} C4_MNG_WORK ;

extern	C4_MNG_WORK		*C4MngWork ;

#endif

