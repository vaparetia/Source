//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bind.c
   トラップイベント処理ルーチン
   
   1999/07/21 M.Sonoyama
   $Id: bind.c,v 1.1.1.3 2002/11/19 11:42:46 Yoshizawa1 Exp $
   */

#ifdef PSX2
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

/*----------------------------------------------------------------*/

static	int	N_Binds ;
static	HZX_BND	Bind[ HZX_MAX_BINDS ] ;
static	char	*BindPtr[ HZX_MAX_BINDS ] ;

#if 0
typedef	struct	{
    HZX_BND	*inside2[ HZX_MAX_INSIDE_BINDS ] ;
    HZX_BND	*outside2[ HZX_MAX_INSIDE_BINDS ] ;
    u_int	n_inside2 ;
    u_int	n_outside2 ;
} ScrPad ;

#define	SCRPAD		((ScrPad * )SCRPAD_ADDR)
#define	N_INSIDE2	(SCRPAD->n_inside2)
#define	N_OUTSIDE2	(SCRPAD->n_outside2)
#define	INSIDE2		(SCRPAD->inside2)
#define	OUTSIDE2	(SCRPAD->outside2)
#else
static	u_int	N_INSIDE2 ;
static	HZX_BND	*INSIDE2[ HZX_MAX_INSIDE_BINDS ] ;
static	u_int	N_OUTSIDE2 ;
static	HZX_BND	*OUTSIDE2[ HZX_MAX_INSIDE_BINDS ] ;
#endif

/*----------------------------------------------------------------*/

/*
   イベント構造体のバインド侵入情報を
   ワークにコピーする
   */
static	inline	void	CopyInsideList( ev )
HZX_EVT			*ev ;		/* 障害ベクトル		*/
{
    HZX_BND	**from, **to ;
    int		i ;

    from = ev->inside2 ;
    to = INSIDE2 ;
    N_INSIDE2 = ev->n_inside2 ;
    for ( i = ev->n_inside2 ; i > 0 ; -- i ) {
		*( to ++ ) = *( from ++ ) ;
    }
}

/*
   バインド侵入リストから削除
   */
static	void		DeleteInsideList( ev )
HZX_EVT			*ev ;
{
    HZX_BND		**inside, **outside ;
    int			i, j, c, n ;

    CopyInsideList( ev ) ;
    inside = INSIDE2 ;
    n = 0 ;
    for ( i = N_INSIDE2 ; i > 0 ; -- i ) {
		c = 0 ;
		outside = OUTSIDE2 ;
		for ( j = N_OUTSIDE2 ; j > 0 ; -- j ) {
			if ( *inside == *( outside ++ ) ) {
				c = 1 ; break ;
			}
		}
		if ( c == 0 ) {
			ev->inside2[ n ] = *inside ;
			n ++ ;
		}
		inside ++ ;
    }
    ev->n_inside2 = n ;
}

/* 
   バインド侵入リストに存在するか調べる
   */
static	int		FindInsideList( bnd )
HZX_BND			*bnd ;
{
    HZX_BND		**inside ;
    int			i ;

    inside = INSIDE2 ;
    for ( i = N_INSIDE2 ; i > 0 ; -- i ) {
		if ( *( inside ++ ) == bnd ) return 1 ;
    }
    return 0 ;
}

/* 
   今回出たバインドリストを作成
   */
static	void	AppendOutsideList( bnd )
HZX_BND		*bnd ;
{
    HZX_BND	**outside ;
    int		i ;
#ifdef DEBUG
	if ( N_OUTSIDE2 >= HZX_MAX_INSIDE_BINDS ) {
		printf( "warning : out binds over\n" ) ;
#ifdef DEBUG_MODE
		ASSERT( 0 ) ;
#endif
		return ;
	}
#endif
    outside = OUTSIDE2 ;
    for ( i = 0; i < (int)N_OUTSIDE2; i ++ ) {
		if ( *( outside ++ ) == bnd ) return ;
    }
    *outside = bnd ;
    N_OUTSIDE2 ++ ;
}

/*
   バインド侵入リストに追加
   すでにリストに存在すれば何もしない
   */
static	int		AppendInsideList( inside, n_inside, bnd )
HZX_BND			**inside ;	/* トラップ侵入リスト	*/
int			n_inside ;	/* トラップ侵入リスト長	*/
HZX_BND			*bnd ;		/* トラップ名		*/
{
    int		i ;

    for ( i = n_inside ; i > 0 ; -- i ) {
		if ( *( inside ++ ) == bnd ){
			/* すでにリストに存在している */
			return n_inside ;
		}
    }
#ifdef DEBUG
	if ( n_inside >= HZX_MAX_INSIDE_BINDS ) {
		printf( "warning : inside binds over\n" ) ;
#ifdef DEBUG_MODE
		ASSERT( 0 ) ;
#endif
		return n_inside ;
	}
#endif	
    *inside = bnd ;
    return n_inside + 1 ;
}

/*----------------------------------------------------------------*/

u_short HZX_ButtonMask[] = {
    PAD_A, PAD_Y, PAD_B, PAD_X, PAD_U, PAD_D, PAD_L, PAD_R, 
	PAD_L1, PAD_L2, PAD_R1, PAD_R2, PAD_AL, PAD_AR, PAD_STA, PAD_SEL
} ;	

#define	ButtonMask	HZX_ButtonMask

long64 	HZX_PlayerStateMask[] = {
    HZX_PLAYER_NORMAL, PLAYER_SQUAT, PLAYER_GROUND,
    PLAYER_WATCH, PLAYER_CAUTION, PLAYER_BEYOND,
    PLAYER_FORCE, PLAYER_LOCKER, PLAYER_BEHIND_CAMERA_ENABLE,
    PLAYER_LADDER, PLAYER_ENEMY_PULL, PLAYER_IN_THE_WATER,
	PLAYER_BEHIND, PLAYER_ROLLING, PLAYER_INTRUDE, PLAYER_CB_BOX,
	PLAYER_WATER_SURFACE, PLAYER2_ELUDE_FALL
} ;

#define	StateMask	HZX_PlayerStateMask

/* 状態チェック、論理積モード */
static	int	CheckConditionAnd( bnd, ev, id, chk )
HZX_BND		*bnd ;
HZX_EVT		*ev ;
HZX_GROUP_ID	id ;
int		chk ;
{
    long64	p_mask, v, p_mask2 ;
    int		i, j, n, c, isplayer ;

    if ( chk & HZX_BND_CHECK_DIR ) n = bnd->dir[ 0 ] ;
    else if ( chk & HZX_BND_CHECK_BUTTON ) n = bnd->button[ 0 ] ;
    else if ( chk & HZX_BND_CHECK_STATE ) n = bnd->state[ 0 ] ;
    else if ( chk & HZX_BND_CHECK_ITEM ) n = bnd->item[ 0 ] ;
    else if ( chk & HZX_BND_CHECK_WEAPON ) n = bnd->weapon[ 0 ] ;
    else return 0 ;

    c = 0 ;

	isplayer = ( GM_PlayerControl != NULL && ev->chara == GM_PlayerControl->name ) ? 1 : 0 ;

	p_mask = GM_CheckPlayerStatusEX( PLAYER_NORMAL_MASK, I64(0) ) ;
	if ( p_mask == 0 ) p_mask |= HZX_PLAYER_NORMAL ;
	p_mask2 = GM_CheckPlayerStatusEX( I64(0), PLAYER2_NORMAL_MASK ) ;

    for ( i = 0, j = 1; i < n; i ++, j ++ ) {
		/* 方向チェック */
		if ( isplayer && ( chk & HZX_BND_CHECK_DIR ) &&
			( bnd->dir[ j ] >= 0 ) ) {
			v = GM_PlayerControl->rot.vy & 4095 ;
			if ( GV_DiffDirAbs( bnd->dir[ j ], (int)v ) > bnd->dir_w[ j ] ) continue ; 
		} 
		/* ボタンチェック */
		if ( isplayer && ( chk & HZX_BND_CHECK_BUTTON ) &&
			( bnd->button[ j ] >= 0 ) ) {
			v = ButtonMask[ ( int )bnd->button[ j ] ] ;
			if ( !( GV_PadData[ bnd->which ].press & v ) ) continue ; 
//		} else if ( ( chk & HZX_BND_CHECK_KEY ) &&
//				    ( bnd->button[ j ] != 0 ) ) {
//			if ( !( GV_PadData[ bnd->which ].press & bnd->button[ j ] ) ) continue ;
		}
		/* 状態チェック */
		if ( isplayer && ( chk & HZX_BND_CHECK_STATE ) && 
			( bnd->state[ j ] >= 0 ) ) {
			v = StateMask[ ( int )bnd->state[ j ] ] ;
			if ( v & PLAYER_STATUS_EX ) {
				if ( !( p_mask2 & v ) ) continue ;
			} else {
				if ( !( p_mask & v ) ) continue ;
			}
		}
		/* アイテムチェック */
		if ( isplayer && ( chk & HZX_BND_CHECK_ITEM ) && 
			( bnd->item[ j ] >= 0 ) ) {
			v = PL_GetPlayerItem() ;
			if ( v != bnd->item[ j ] ) continue ;
		}
		/* 武器チェック */
		if ( isplayer && ( chk & HZX_BND_CHECK_WEAPON ) && 
			( bnd->weapon[ j ] >= 0 ) ) {
			v = PL_GetPlayerWeapon() ;
			if ( v != bnd->weapon[ j ] ) continue ;
		}
		/* アラートモードチェック */
		if ( ( chk & HZX_BND_CHECK_ALERT ) &&
			 ( bnd->alert != GM_AlertMode ) ) {
			continue ;
		}
		c = 1 ; break ;
    }
    return c ;
}

/* 条件チェック */
static	int	CheckCondition( HZX_BND *bnd, HZX_EVT *ev, HZX_GROUP_ID id )
{
    int		chk, isplayer ;
    u_int	mask ;
    int		i, n, v ;

    /* マップチェック */
    if ( !( bnd->id & id ) ) return 0 ;
    /* カメラチェック */
    if ( ( bnd->type & HZX_BND_TYPE_CAMERA ) &&
		( ev->flag & HZX_EVT_FLAG_NO_CAMERA ) ) return 0 ;
    if ( !( bnd->type & HZX_BND_TYPE_CAMERA ) &&
		( ev->flag & HZX_EVT_FLAG_CAMERA_ONLY ) ) return 0 ;
	/* ミサイルチェック */
	if ( !( bnd->type & HZX_BND_TYPE_MISSILE ) &&
		 ( ev->flag & HZX_EVT_FLAG_MISSILE ) ) {
		/* マップコネクトチェックタイプでもない */
		if ( !( ev->flag & HZX_EVT_FLAG_MAPCONNECT ) ||
			 !( bnd->type & HZX_BND_TYPE_MAPCONNECT ) ) {
			return 0 ;
		}
	}
	if ( ( bnd->type & HZX_BND_TYPE_MISSILE ) &&
		!( ev->flag & HZX_EVT_FLAG_MISSILE ) ) return 0 ;

    chk = bnd->check ;

	isplayer = ( GM_PlayerControl != NULL && ev->chara == GM_PlayerControl->name ) ? 1 : 0 ;

    /* 論理積モード */
    if ( chk & HZX_BND_CHECK_AND ) {
		v = CheckConditionAnd( bnd, ev, id, chk ) ;
		return v ;
    }

    /* 方向チェック */
    if ( chk & HZX_BND_CHECK_DIR ) {
		int	dir ;
		dir = GM_PlayerControl->rot.vy & 4095 ;
		n = bnd->dir[ 0 ] ;
		for ( i = 0; i < n; i ++ ) {
			if ( GV_DiffDirAbs( bnd->dir[ i + 1 ], dir ) <= bnd->dir_w[ i + 1 ] ) break ;
		}
		if ( i == n ) return 0 ;
    }
    /* ボタンチェック */
    if ( isplayer && ( chk & HZX_BND_CHECK_BUTTON ) ) {
		if ( GM_CheckPlayerStatus( PLAYER_PAD_OFF ) ) return 0 ;
		mask = 0 ; n = bnd->button[ 0 ] ;
		for ( i = 0; i < n; i ++ ) {
			v = bnd->button[ i + 1 ] ;
			mask |= ButtonMask[ v ] ;
		}
		if ( !( GV_PadData[ bnd->which ].press & mask ) ) return 0 ;
//    } else if ( chk & HZX_BND_CHECK_KEY ) {
//		if ( GM_CheckPlayerStatus( PLAYER_PAD_OFF ) ) return 0 ;
//		mask = 0 ; n = bnd->button[ 0 ] ;
//		for ( i = 0; i < n; i ++ ) {
//			mask |= bnd->button[ i + 1 ] ;
//		}
//		if ( !( GV_PadData[ bnd->which ].press & mask ) ) return 0 ;
	}
    /* 状態チェック */
    if ( isplayer && ( chk & HZX_BND_CHECK_STATE ) ) {
		long64	p_mask, mask64 ;
		long64	p_mask2, mask642, s ;

		p_mask = GM_CheckPlayerStatusEX( PLAYER_NORMAL_MASK, I64(0) ) ;
		if ( p_mask == 0 ) p_mask |= HZX_PLAYER_NORMAL ;
		p_mask2 = GM_CheckPlayerStatusEX( I64(0), PLAYER2_NORMAL_MASK ) ;

		n = bnd->state[ 0 ] ;
		mask64 = mask642 = 0 ;
		for ( i = 0; i < n; i ++ ) {
			v = bnd->state[ i + 1 ] ;
			s = StateMask[ v ] ;
			if ( s & PLAYER_STATUS_EX ) mask642 |= s ;
			else						mask64 |= s ;
		}	
		if ( !( mask64 & p_mask ) && !( mask642 & p_mask2 ) ) return 0 ;
    }
    /* アイテムチェック */
    if ( isplayer && ( chk & HZX_BND_CHECK_ITEM ) ) {
		v = PL_GetPlayerItem() ;
		n = bnd->item[ 0 ] ;
		for ( i = 0; i < n; i ++ ) {
			if ( v == bnd->item[ i + 1 ] ) break ;
		}	
		if ( i == n ) return 0 ;
    }
    /* 武器チェック */
    if ( isplayer && ( chk & HZX_BND_CHECK_WEAPON ) ) {
		v = PL_GetPlayerWeapon() ;
		n = bnd->weapon[ 0 ] ;
		for ( i = 0; i < n; i ++ ) {
			if ( v == bnd->weapon[ i + 1 ] ) break ;
		}	
		if ( i == n ) return 0 ;
    }
	/* アラートモードチェック */
	if ( ( chk & HZX_BND_CHECK_ALERT ) &&
		( bnd->alert != GM_AlertMode ) ) {
		return 0 ;
	}
    return 1 ;
}

/* 実行ブロック実行 */
static	void	HZX_ExecBindEx( HZX_BND *bnd, HZX_EVT *ev, u_int event, u_int type )
{
    GCL_ARGS	args ;
    int		buf[ 8 ], *p ;

    p = buf ;
    *( p ++ ) = ev->object ;
    *( p ++ ) = ev->chara ;
    *( p ++ ) = event ;    
    *( p ++ ) = (int)ev->mov->vx ;
    *( p ++ ) = (int)ev->mov->vy ;
    *( p ++ ) = (int)ev->mov->vz ;
	*( p ++ ) = type ;
    args.argc = 7 ;
    args.argv = buf ;
    /* Delayには未対応 */
    if ( bnd->type & HZX_BND_TYPE_PROC ) {
		GM_ExecProc( ( int )bnd->command, &args ) ;
    } else {
		GM_ExecBlock( ( char * )bnd->command, &args ) ;
    }
}

#define	HZX_ExecBind( _bnd, _evt, _event )	HZX_ExecBindEx( _bnd, _evt, _event, 0 )

/* イベント実行 */
int		HZX_ExecEvent( HZX_GROUP_ID id, HZX_EVT *ev, u_int mode )
{
    int		n_binds, i, c, a ;
	int		chara, namebuf, namechange ;
    HZX_BND	*bnd ;

    N_OUTSIDE2 = 0 ;
    CopyInsideList( ev ) ;
    n_binds = N_Binds ;
    bnd = Bind ; 
    a = 0 ;

	chara = namebuf = ev->chara ;
	if ( ( ev->flag & HZX_EVT_FLAG_MAPCONNECT ) &&
		 GM_PlayerControl != NULL ) {
		namechange = 1 ;
	} else {
		namechange = 0 ;
	}

    for ( i = 0; i < n_binds; i ++, bnd ++ ) {
		if ( namechange ) {
			if ( bnd->type & HZX_BND_TYPE_MAPCONNECT ) {
				/* マップコネクトトラップのときは名前を変える */
				chara = GM_PlayerControl->name ;
			} else {
				chara = namebuf ;
			}	
			a = 0 ;	/* 名前が変わるので */
		}
		/* 名前チェック */
		if ( ( bnd->name & 0x00ffffff ) != ( ev->object & 0x00ffffff ) ) {
			if ( a ) break ;
			continue ;
		}
		/* 番号付き名前チェック */
		if ( ( bnd->name & 0xff000000 ) != 0 &&
			( bnd->name != ev->object ) ) {
			continue ;
		}
		a = 1 ;
		if ( bnd->chara != EV_CHARA_EVERY &&
			/* 番号つきname対応 */
			( ( bnd->chara & 0xff000000 ) == 0
			 ? ( bnd->chara != ( chara & 0x00ffffff ) )
			 : ( bnd->chara != chara ) ) ) {
			continue ;
		}
		switch ( mode ) {
		case EV_ENTER :
			/* 条件不成立中のイベントのみ */
			if ( FindInsideList( bnd ) ) continue ;
			/* 条件チェック */
			if ( !CheckCondition( bnd, ev, id ) ) continue ;
			/* 条件成立リストに登録 */
			ev->n_inside2 = AppendInsideList( &( ev->inside2 ), ev->n_inside2,
											 bnd ) ;
			/* 実行条件が 入る／？／いる／＊ のとき実行 */
			if ( bnd->event == EV_LEAVE ) continue ;	
			//printf( "exec enter\n" ) ;
			HZX_ExecBind( bnd, ev, EV_ENTER ) ; 
			break ;
		case EV_LEAVE :
			/* 条件成立中のイベントのみ */
			if ( !FindInsideList( bnd ) ) continue ;
			/* 条件成立リストから削除 */
			AppendOutsideList( bnd ) ;
			/* 実行条件が 出る／？／＊ の場合実行 */
			if ( bnd->event != EV_BOTH && 
				bnd->event != EV_BOTH2 && 
				bnd->event != EV_LEAVE ) continue ;
			//printf( "exec leave\n" ) ;    
			HZX_ExecBind( bnd, ev, EV_LEAVE ) ;
			break ;
		case EV_INSIDE :
			c = CheckCondition( bnd, ev, id ) ;
			if ( c ) {			/* 条件成立 */
				/* いままで条件不成立 */	
				if ( !FindInsideList( bnd ) ) {
					/* 条件成立リストに登録 */
					ev->n_inside2 = AppendInsideList( &( ev->inside2 ), 
													 ev->n_inside2, bnd ) ;
					/* 実行条件が 入る／？／いる／＊ の場合実行 */
					if ( bnd->event != EV_LEAVE ) {
						//printf( "exec enter2\n" ) ;    
						HZX_ExecBind( bnd, ev, EV_ENTER ) ;
					}
					/* いままで条件成立 */
				} else {
					/* 実行条件が いる／＊ の場合実行 */
					if ( bnd->event == EV_INSIDE ||
						bnd->event == EV_BOTH2 ) {
						//printf( "exec inside\n" ) ;    
						HZX_ExecBind( bnd, ev, EV_INSIDE ) ;
					}
				}
			} else {			/* 条件不成立 */
				/* いままで条件成立 */
				if ( FindInsideList( bnd ) ) { 
					/* 条件成立リストから削除 */
					AppendOutsideList( bnd ) ;
					/* 実行条件が 出る／？／＊ の場合実行 */
					if ( bnd->event == EV_LEAVE ||
						bnd->event == EV_BOTH2 ||
						bnd->event == EV_BOTH ) {
						//printf( "exec leave2\n" ) ;    
						HZX_ExecBindEx( bnd, ev, EV_LEAVE, 1 ) ;
					}
				}
			}
		}
    }
    DeleteInsideList( ev ) ;
	ev->chara = namebuf ;
    return 1 ;
}

/*----------------------------------------------------------------*/

/* トラップ進入構造体のセット */
void		HZX_SetEvent( HZX_EVT *ev, int chara, FVECTOR *mov, SVECTOR *rot )
{
	GV_ZeroMemory( ev, sizeof( HZX_EVT ) ) ;
    ev->chara = chara ;
    ev->object = 0 ;
    ev->n_inside = ev->n_inside2 = 0 ;
    ev->flag = HZX_EVT_FLAG_NO_CAMERA ;
    ev->mov = mov ;
    ev->rot = rot ;
}

/* トラップイベントを登録 */
void		HZX_SetBind( HZX_BND *bnd, char *top )
{
    int		i, j, c ;

    ASSERT( N_Binds < HZX_MAX_BINDS ) ;
#if 1
	/* 重複チェック */
    for ( i = N_Binds; i > 0; i -- ) {
		if ( BindPtr[ i ] == top && 
			 Bind[ i ].name == bnd->name &&
			 Bind[ i ].id == bnd->id &&
			 Bind[ i ].event == bnd->event &&
			 Bind[ i ].chara == bnd->chara ) {
			/* もう存在している */
#ifdef DEBUG_MODE
			printf( "warning : this trap has already binded %x\n", top ) ;
#endif
			return ;
		}
	}	
#endif
    /* 同じ名前のトラップは並ぶように */
    c = 0 ;
    for ( i = N_Binds; i > 0; i -- ) {
		if ( ( bnd->name & 0x00ffffff ) 
			== ( Bind[ i - 1 ].name & 0x00ffffff ) ) {
			c = 1 ;
			break ;
		}
    }
    if ( !c ) {
		Bind[ N_Binds ] = *bnd ;
		BindPtr[ N_Binds ] = top ;
	} else {
		for ( j = N_Binds; j > i; j -- ) {
			Bind[ j ] = Bind[ j - 1 ] ;
			BindPtr[ j ] = BindPtr[ j - 1 ] ;
		}
		Bind[ i ] = *bnd ;
		BindPtr[ i ] = top ;
    }
    N_Binds ++ ;
}

/* トラップイベント配列初期化 */
void	HZX_InitTrapBind( void )
{
    int		i ;

    N_Binds = 0 ;
    for ( i = 0; i < HZX_MAX_BINDS; i ++ ) {
		Bind[ i ].name = 0 ;
		BindPtr[ i ] = NULL ;
    }
}

/*----------------------------------------------------------------*/

/* トラップイベントを削除 */
int		NewRemoveTrapBind( void )
{
	int			name_id ;
	int			n_binds, n_binds2 ;
	int			i, has_name ;
	char		**bindptr ;
	HZX_BND		*bind ;

	n_binds = N_Binds ;
	GCL_GetOption( 'n' ) ;
	while( GCL_NextStr() != NULL ) {
		bind = Bind ;
		bindptr = BindPtr ;
		n_binds2 = 0 ;
		name_id = GCL_GetNextInt() ;
		has_name = ( name_id & 0xff000000 ) ? 1 : 0 ;
		for ( i = 0; i < n_binds; i ++, bind ++, bindptr ++ ) {
			if ( ( has_name && ( name_id == bind->name ) ) ||
				 ( !has_name && ( name_id == ( bind->name & 0x00ffffff ) ) ) ) {
				continue ;
			}
			Bind[ n_binds2 ] = *bind ;
			BindPtr[ n_binds2 ] = *bindptr ;
			n_binds2 ++ ;
		}
		n_binds = n_binds2 ;
	}
	N_Binds = n_binds ;
	return 0 ;
}

