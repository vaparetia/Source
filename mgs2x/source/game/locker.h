/*
   locker.h
   ロッカー関係定義ファイル

   2000/01/27 M.Sonoyama
   $Id: locker.h,v 1.1.1.3 2002/11/19 11:41:52 Yoshizawa1 Exp $
*/

#ifndef _locker_h_
#define	_locker_h_

/*---------------------------------------------------------------

  ロッカー管理

---------------------------------------------------------------*/

typedef	ALIGN16_DECL(struct) _GM_Locker {
    FVECTOR		rots[ 4 ] ;
    FVECTOR		axis ; /* 回転軸 */
    IVECTOR		hand ; /* 手持ち位置 */
    struct _GM_Locker	*next ;
    DG_OBJS		*objs ;

    int			name ;
    int			status ;
    int			command ;
    int			count ;

    int			proc ;
    int			exec ;
    int			chara ;
    short		def_rot ;
    short		rot ;

    int			sound[ 3 ] ;
	void		*shadow_obj ;

    HZX_D_SEGMENT	*segment ;
    HZX_D_SEGMENT	*segment2 ;
    HZX_D_SEGMENT	*segment3 ;
    HZX_D_FLOOR		*floor ;

	FMATRIX		dummy[ 16 ] ;
} LOCKER  ;

enum {
    LOCKER_SE_MOVING = 0,
    LOCKER_SE_CLOSE,
} ;

enum {
    LOCKER_STATE_WORKING =	0x0000, /* 動作中 */
    LOCKER_STATE_CLOSE = 	0x0001,
    LOCKER_STATE_OPEN  = 	0x0002,
    LOCKER_STATE_SELF_CLOSE =	0x0004,
    LOCKER_STATE_PLAYER_IN =	0x0010,
    LOCKER_STATE_CORPSE_IN = 	0x0020,
    LOCKER_STATE_ENEMY_IN = 	0x0040,
    LOCKER_STATE_ROTATE = 	0x0080,
    LOCKER_STATE_ROTATE_D = 	0x0100,
    LOCKER_STATE_FROM_OUT =	0x0200,
    LOCKER_STATE_FROM_IN =	0x0400,
	LOCKER_STATE_DONOT_OFF_SEGMENT4 =	0x0800,
    LOCKER_STATE_END =		0x1000,
	LOCKER_STATE_BROKEN = 	0x2000,
    LOCKER_STATE_DESTROY =	0x4000,
	LOCKER_STATE_INVINCIBLE = 0x8000,
	LOCKER_STATE_POSTER =   0x00010000,	/* ポスター */
	LOCKER_STATE_LEAN_CLOSE = 0x00020000,
	LOCKER_STATE_NO_OPEN_SEG =	0x00040000,	/* 開いているときの扉プレイヤーあたりなくす */
	LOCKER_STATE_MOTION_FAST =	0x00080000,	/* モーション早再生（開いている状態で開始用） */
	LOCKER_STATE_BROKEN_FAST =	0x00100000,	/* モーション早再生（壊れている状態で開始用） */
	LOCKER_STATE_LEAN_START =	0x00200000,	/* 傾き状態からスタート */
	LOCKER_STATE_SEARCH_ENEMY =	0x00400000,	/* 起動時に自分に入っている敵を探す */
} ;

enum {
    LOCKER_COMMAND_NOTHING = 0,
    LOCKER_COMMAND_CLOSE,
    LOCKER_COMMAND_OPEN,
    LOCKER_COMMAND_ENEMY_IN,
    LOCKER_COMMAND_CHANGE,
	LOCKER_COMMAND_MOVING,
} ;

enum {
    LOCKER_MSG_OPEN = 0,
    LOCKER_MSG_CLOSE,
    LOCKER_MSG_PUT_ENEMY,
    LOCKER_MSG_PUT_CORPSE,
    LOCKER_MSG_END,
    LOCKER_MSG_ROTATE,
    LOCKER_MSG_ROTATE_D,
	LOCKER_MSG_MOVE,
	LOCKER_MSG_NOT_OPEN,
} ;

/* フラグ */
enum {
	LOCKER_FLAG_SHADOWDROP	= 			0x0001,
	LOCKER_FLAG_SELFSUBJECT	= 			0x0002,
	LOCKER_FLAG_SHADOWWRITE	=			0x0004,
	LOCKER_FLAG_DOWNOUTSIDE =			0x0008,
	LOCKER_FLAG_TOILET =				0x1000,
} ;

/*------------------------------------------------------------------*/

#endif
