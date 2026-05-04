//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   trap.c	
   トラップ進入判定ルーチン
   
   1999/07/07 M.Sonoyama
   $Id: trap.c,v 1.1.1.3 2002/11/19 11:42:49 Yoshizawa1 Exp $			      
   
   PROJECT OF METAL GEAR SOLID2 ( since 1999 )
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
#include "BP_Camera.h"  //BP_CAMERA

/*----------------------------------------------------------------*/

#ifdef HZX_DTRP
#define	DTRP
#endif

typedef	struct	{
    SVECTOR	b1, b2 ;
} SVECTOR2 ;

typedef	ALIGN16_DECL(struct)	{
    FVECTOR	b1, b2 ;
} FVECTOR2  ;

typedef	struct	{
    /*
       検索用ワーク
	   */
    FVECTOR			from ;
    FVECTOR			from_global ;
    FVECTOR2		currentf ;
    SVECTOR2		current ;
    HZX_BLOCK		*block ;
	HZX_D_TRP		*d_trap ;
    /*
       侵入情報計算用ワーク
	   */
    u_int			n_inside ;
    u_int			inside[ HZX_MAX_TRAPS ] ;
} ScrPad ;

static	ScrPad		TrapWorkBuf ;

/*
 * 連続読みだし用
 */
static  int             FinalReadTrpNum = -1;
static  int             FinalReadBlkNum = -1;
static  HZX_GROUP_ID    FinalGroupID  = 0;
static  int             FinalName = -1;

//#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)
#define	SCRPAD		(&TrapWorkBuf)

#define	FROM		(&(SCRPAD->from))
#define	GFROM		(&(SCRPAD->from_global))
#define	CURRENTF	(&(SCRPAD->currentf))
#define	CURRENT		(&(SCRPAD->current))
#define	BLOCK		(SCRPAD->block)
#define	DYNAMIC		(SCRPAD->d_trap)

#define	N_INSIDE	(&(SCRPAD->n_inside))
#define	INSIDE		(SCRPAD->inside)

/*----------------------------------------------------------------*/

/*
   イベント構造体のトラップ侵入情報を
   ワークにコピーする
   */
static	inline	void	CopyInsideList( ev )
HZX_EVT			*ev ;		/* 障害ベクトル		*/
{
    u_int	*from, *to ;
    int		i ;

    from = ev->inside ;
    to = INSIDE ;
    *N_INSIDE = ev->n_inside ;
    for ( i = ev->n_inside ; i > 0 ; -- i ) {
		*( to ++ ) = *( from ++ ) ;
    }
}

/*
   トラップ侵入リストから削除
   リストに存在しなければ何もしない
   */
static	inline	int	DeleteInsideList( name )
int			name ;		/* トラップ名		*/
{
    u_int		*inside ;
    int			i ;

    inside = INSIDE ;
    for ( i = *N_INSIDE ; i > 0 ; -- i ) {
		if ( *( inside ++ ) == name ) {
			-- ( *N_INSIDE ) ;
			inside[ -1 ] = INSIDE[ *N_INSIDE ] ;
			return 1 ;
		}
    }
    return 0 ;
}

/*
   トラップ侵入リストに追加
   すでにリストに存在すれば何もしない
   */
static	inline	int	AppendInsideList( inside, n_inside, name )
u_int			*inside ;	/* トラップ侵入リスト	*/
int			n_inside ;	/* トラップ侵入リスト長	*/
int			name ;		/* トラップ名		*/
{
    int		i ;

    for ( i = n_inside ; i > 0 ; -- i ) {
		if ( *( inside ++ ) == name ){
			/* すでにリストに存在している */
			return n_inside ;
		}
    }
#ifdef DEBUG_MODE
    if ( n_inside >= HZX_MAX_TRAPS ) {
		printf( "error : inside trap over\n" ) ;
		ASSERT( 0 ) ;
		return n_inside ;
    }
#endif    
    *inside = name ;
    return n_inside + 1 ;
}

/* トラップスイッチのチェック */
static	inline	int	NoCheckTrap( int atr )
{
	int			c ;

	c = atr & ( HZX_TRP_ATR_CHECK_SCN | HZX_TRP_ATR_NO_CHECK_SCN ) ;
	if ( c ) return ( atr & HZX_TRP_ATR_NO_CHECK_SCN ) ;
	return ( atr & HZX_TRP_ATR_NO_CHECK ) ;
}

/*----------------------------------------------------------------*/

/* トラップ侵入判定 */
static	inline	int	InsideTrap()
{
    float	d ;

    /*
       x, z は大きい方を線上なしにする。
	   */
    d = FROM->vx ;
    if ( d < CURRENT->b1.vx || d >= CURRENT->b2.vx ) return 0 ;
    d = FROM->vz ;
    if ( d < CURRENT->b1.vz || d >= CURRENT->b2.vz ) return 0 ;
    d = FROM->vy ;
    if ( d < CURRENT->b1.vy || d >= CURRENT->b2.vy ) return 0 ;
    return 1 ;
}

static	inline	int	InsideTrapF()
{
    float	d ;

    /*
       x, z は大きい方を線上なしにする。
	   */
    d = GFROM->vx ;
    if ( d < CURRENTF->b1.vx || d >= CURRENTF->b2.vx ) return 0 ;
    d = GFROM->vz ;
    if ( d < CURRENTF->b1.vz || d >= CURRENTF->b2.vz ) return 0 ;
    d = GFROM->vy ;
    if ( d < CURRENTF->b1.vy || d >= CURRENTF->b2.vy ) return 0 ;
    return 1 ;
}

/*
   ENTER イベントを発生する
   */
static	void	ExecEnterEvent( HZX_EVT *ev, HZX_GROUP_ID id )
{
    HZX_TRP		*trp ;
    int			i, n_inside ;
	int		 	name ;

    n_inside = 0 ;
#ifdef DTRP
	if ( BLOCK == NULL ) goto check_dynamic ;
#endif
    trp = BLOCK->traps ;
    for ( i = BLOCK->n_traps; i > 0 ; -- i, trp ++ ) {
		/* ＯＦＦトラップをスキップ */
		if ( NoCheckTrap( trp->b1.pad ) ) continue ;
		*CURRENT = *( SVECTOR2 * )trp ;
		if ( !InsideTrap() ) continue ;
		/*
		   前回のトラップ侵入情報リストから削除
		   前回のリストになければ ENTER イベント発生
		   */
		name = trp->name_id ;
		ev->object = name ;
		if( !DeleteInsideList( name ) ){
			/* 入った */
#if 0 
			if ( !( trp->b1.vw & 1 ) ) HZX_ExecEvent( id, ev, EV_ENTER ) ;
#else
			HZX_ExecEvent( id, ev, EV_ENTER ) ;
#endif
		} else {
			/* 入っている */
#if 0
			if ( !( trp->b1.vw & 1 ) ) HZX_ExecEvent( id, ev, EV_INSIDE ) ;
#else
			HZX_ExecEvent( id, ev, EV_INSIDE ) ;
#endif
		}
		/*
		   今回のトラップ侵入情報リストに追加する
		   */
		n_inside = AppendInsideList( ev->inside, n_inside, name ) ;
    }
#ifdef DTRP
check_dynamic :
	/* 動的トラップ */
	if ( DYNAMIC != NULL ) {
		HZX_D_TRP		*trp ;

		trp = DYNAMIC ;
		do {
			if ( NoCheckTrap( trp->attribute ) ) continue ;
			*CURRENTF = *( FVECTOR2 * )trp ;
			if ( !InsideTrapF() ) continue ;
			name = trp->name_id ;
			ev->object = name ;
			if( !DeleteInsideList( name ) ){
#if 0 
				if ( !( trp->b1.vw & 0x1 ) ) HZX_ExecEvent( id, ev, EV_ENTER ) ;
#else
				HZX_ExecEvent( id, ev, EV_ENTER ) ;
#endif
			} else {
#if 0
				if ( !( trp->b1.vw & 0x1 ) ) HZX_ExecEvent( id, ev, EV_INSIDE ) ;
#else
				HZX_ExecEvent( id, ev, EV_INSIDE ) ;
#endif
			}
			n_inside = AppendInsideList( ev->inside, n_inside, name ) ;
		} while( ( trp = trp->next ) != NULL ) ;
	}
#endif
    ev->n_inside = n_inside ;
}

/*
   LEAVE イベントを発生する
   */
static	void	ExecLeaveEvent( HZX_EVT *ev, HZX_GROUP_ID id )
{
    u_int		*inside ;
    int			i ;

    /*
       前回のトラップ侵入情報リストに残っているのは
       離脱したトラップなので、LEAVE イベントを発生
	   */
    inside = INSIDE ;
    for ( i = *N_INSIDE ; i > 0 ; -- i ) {
		ev->object = *( inside ++ ) ;
		HZX_ExecEvent( id, ev, EV_LEAVE ) ;
    }
}

/* ブロックチェック */
static	void	CheckInsideBlock( grp )
HZX_GRP		*grp ;
{
    int		x, y, z, n ;

    BLOCK = NULL ;
    if ( grp->n_blocks == 0 ) return ;
    x = ( ( int )FROM->vx - grp->bound_min_x ) / grp->block_size_x ;
    y = ( ( int )FROM->vy - grp->bound_min_y ) / grp->block_size_y ;
    z = ( ( int )FROM->vz - grp->bound_min_z ) / grp->block_size_z ;
    if ( x < 0 || y < 0 || z < 0 ||
		x >= grp->div_x || y >= grp->div_y || z >= grp->div_z ) return ;
    n = x + y * grp->div_x * grp->div_z + z * grp->div_x ;
    if ( n >= 0 && n < grp->n_blocks ) BLOCK = grp->blocks + n ;
}

/* 一番近いブロックを検索 */
static	void	SearchNearBlock( HZX_GRP *grp )
{
	float		minlen, len ;
	HZX_BLOCK	*blk, *minblk ;
	FVECTOR		blkpos ;
	int			i, minblkno ;

	minlen = 10000000.0F ;
	minblkno = -1 ;
	minblk = NULL ;
	blk = grp->blocks ;
	for ( i = 0; i < grp->n_blocks; i ++, blk ++ ) {
		blkpos.vx = ( float )blk->tx ;
		blkpos.vy = ( float )blk->ty ;
		blkpos.vz = ( float )blk->tz ;
		len = GV_VecLen3F2( FROM, &blkpos ) ;
		if ( len < minlen ) {
			minblk = blk ;
			minlen = len ;
			minblkno = i ;
		}
	}
	printf( "search near blobk -> %d\n", minblkno ) ;
	BLOCK = minblk ;
}

/* トラップ進入判定＆イベント発生 */

void HZX_EnterTrap( HZX_GROUP_ID id, HZX_EVT *ev )
{
	int			mapbuf ;
	HZX_GRP		*grp ;

	DG_COPY_VEC( FROM, ev->mov ) ;
    CopyInsideList( ev ) ;
	grp = HZX_GetGroup( id ) ;
    /* ブロックチェック */
#ifdef DTRP
	DYNAMIC = grp->dynamics->traps ;
#endif
    CheckInsideBlock( grp ) ;
#ifdef DTRP
    if ( BLOCK == NULL && DYNAMIC == NULL ) {
#else
    if ( BLOCK == NULL ) {
#endif
		if ( ev->flag & HZX_EVT_FLAG_CHECKALL ) {
			/* 一番近いブロックをチェック */
			SearchNearBlock( grp ) ;
			if ( BLOCK == NULL ) {
				/* 全部出たことにする */
				HZX_FlashTrap( id, ev ) ;
				return ;
			}
		} else {
			/* 全部出たことにする */
			HZX_FlashTrap( id, ev ) ;
			return ;
		}
	}
#ifdef DTRP
	DG_COPY_VEC( GFROM, FROM ) ;
#endif
#ifdef DTRP
	if ( BLOCK != NULL ) {
#endif
		FROM->vx -= ( float )BLOCK->tx ;
		FROM->vy -= ( float )BLOCK->ty ;
		FROM->vz -= ( float )BLOCK->tz ;
#ifdef DTRP
	}
#endif
    /* イベント発生 */
	mapbuf = GM_ScriptCurrentMap ;
	GM_ScriptCurrentMap = GM_CurrentMap ;

    BP_Camera_SetScriptEventType( kScriptEvent_EnterTrap ); //BP_CAMERA - let camera tweak system know event type
    ExecEnterEvent( ev, id ) ;

    BP_Camera_SetScriptEventType( kScriptEvent_LeaveTrap ); //BP_CAMERA - let camera tweak system know event type
    ExecLeaveEvent( ev, id ) ;

    BP_Camera_SetScriptEventType( kScriptEvent_Default ); //BP_CAMERA - let camera tweak system know event type

	GM_ScriptCurrentMap = mapbuf ;
}

int	HZX_EnterTrapNoExecEvent( HZX_GROUP_ID id, HZX_EVT *ev )
{
	HZX_TRP			*trp ;
#ifdef DTRP
	HZX_D_TRP		*dtrp ;
#endif
	FVECTOR			*pos ;
	int				n_inside, i ;
	
	ev->n_inside = n_inside = 0 ;
    CopyInsideList( ev ) ;
	pos = ev->mov ;
	DG_COPY_VEC( FROM, pos ) ;
#ifdef DTRP
	DYNAMIC = ( HZX_GetGroup( id ) )->dynamics->traps ;
#endif
    CheckInsideBlock( HZX_GetGroup( id ) ) ;
#ifdef DTRP
    if ( BLOCK == NULL && DYNAMIC == NULL ) return 0 ;
#else
    if ( BLOCK == NULL ) return 0 ;
#endif
#ifdef DTRP
	DG_COPY_VEC( GFROM, FROM ) ;
#endif
#ifdef DTRP
	if ( BLOCK != NULL ) {
#endif
		FROM->vx -= ( float )BLOCK->tx ;
		FROM->vy -= ( float )BLOCK->ty ;
		FROM->vz -= ( float )BLOCK->tz ;
		trp = BLOCK->traps ;
		for ( i = BLOCK->n_traps; i > 0 ; -- i, trp ++ ) {
			if ( NoCheckTrap( trp->b1.pad ) ) continue ;
			*CURRENT = *( SVECTOR2 * )trp ;
			if ( !InsideTrap() ) continue ;
			n_inside = AppendInsideList( ev->inside, n_inside, trp->name_id ) ;		
		}
#ifdef DTRP
	}
#endif
#ifdef DTRP
	if ( DYNAMIC != NULL ) {
		dtrp = DYNAMIC ;
		do {
			if ( NoCheckTrap( dtrp->attribute ) ) continue ;
			*CURRENTF = *( FVECTOR2 * )dtrp ;
			if ( !InsideTrapF() ) continue ;
			n_inside = AppendInsideList( ev->inside, n_inside, dtrp->name_id ) ;
		} while( ( dtrp = dtrp->next ) != NULL ) ;
	}
#endif
	ev->n_inside = n_inside ;
	return n_inside ;
}

int		HZX_CheckInsideTrap( HZX_GROUP_ID hzx_id, FVECTOR *pos, int name_id )
{
	HZX_GROUP_ID	id ;
	HZX_TRP			*trp ;
#ifdef DTRP
	HZX_D_TRP		*dtrp ;
#endif
	int				i, no ; 

	DG_COPY_VEC( FROM, pos ) ;

	while( hzx_id != 0 ) {
		no = GV_GetNo( hzx_id ) ;
		id = GV_GetBit( no ) ;
		hzx_id &= ~id ;
		if ( no >= HZX_CurrentHzx->def->n_groups ) continue ;
#ifdef DTRP
		DYNAMIC = ( HZX_GetGroup( id ) )->dynamics->traps ;
#endif
		CheckInsideBlock( HZX_GetGroup( id ) ) ;
#ifdef DTRP
		if ( BLOCK == NULL && DYNAMIC == NULL ) continue ;
#else
		if ( BLOCK == NULL ) continue ;
#endif
#ifdef DTRP
		DG_COPY_VEC( GFROM, FROM ) ;
#endif
#ifdef DTRP
		if ( BLOCK != NULL ) {
#endif
			FROM->vx -= ( float )BLOCK->tx ;
			FROM->vy -= ( float )BLOCK->ty ;
			FROM->vz -= ( float )BLOCK->tz ;
			trp = BLOCK->traps ;
			for ( i = BLOCK->n_traps; i > 0 ; -- i, trp ++ ) {
				if ( trp->name_id != name_id ) continue ;
				if ( NoCheckTrap( trp->b1.pad ) ) continue ;
				*CURRENT = *( SVECTOR2 * )trp ;
				if ( InsideTrap() ) return 1 ;
			}
#ifdef DTRP
		}
#endif
#ifdef DTRP
		if ( DYNAMIC != NULL ) {
			dtrp = DYNAMIC ;
			do {
				if ( dtrp->name_id != name_id ) continue ;
				if ( NoCheckTrap( dtrp->attribute ) ) continue ;
				*CURRENTF = *( FVECTOR2 * )dtrp ;
				if ( InsideTrapF() ) return 1 ;
			} while( ( dtrp = dtrp->next ) != NULL ) ;
		}
#endif
    }
	return 0 ;
}

int		HZX_CheckInsideAllTrap( FVECTOR *pos, int name_id )
{
	HZX_GRP			*hzx_grp ;
	HZX_TRP			*trp ;
#ifdef DTRP
	HZX_D_TRP		*dtrp ;
#endif
	int				i, g ; 
	int				j ;

	DG_COPY_VEC( FROM, pos ) ;

	for ( g=0; g<HZX_CurrentHzx->def->n_groups; g++ ) {
		hzx_grp = HZX_CurrentHzx->def->groups + g ;
#ifdef DTRP
		DYNAMIC = hzx_grp->dynamics->traps ;
#endif

#if 0	/* ブロック判定はせずに全トラップをチェック */
	    CheckInsideBlock( hzx_grp ) ;
#ifdef DTRP
	    if ( BLOCK == NULL && DYNAMIC == NULL ) continue ;
#else
	    if ( BLOCK == NULL ) continue ;
#endif
#ifdef DTRP
		DG_COPY_VEC( GFROM, FROM ) ;
#endif

#ifdef DTRP
		if ( BLOCK != NULL ) {
#endif
			FROM->vx -= ( float )BLOCK->tx ;
			FROM->vy -= ( float )BLOCK->ty ;
			FROM->vz -= ( float )BLOCK->tz ;
			trp = BLOCK->traps ;
			for ( i = BLOCK->n_traps; i > 0 ; -- i, trp ++ ) {
				//if ( NoCheckTrap( trp->b1.pad ) ) continue ;
                *CURRENT = *( SVECTOR2 * )trp ;
                if ( trp->name_id == name_id ) {
					if ( InsideTrap() ) return 1 ;
				}
			}
#ifdef DTRP
		}
#endif

#else	/* ブロック判定はせずに全トラップをチェック */

		if ( hzx_grp->n_blocks == 0 ) continue ;
		BLOCK = hzx_grp->blocks ; 
		for ( j = 0; j < hzx_grp->n_blocks; j ++, BLOCK ++ ) {
			if ( BLOCK->n_traps == 0 ) continue ;
			DG_COPY_VEC( FROM, pos ) ;
			DG_COPY_VEC( GFROM, FROM ) ;
			FROM->vx -= ( float )BLOCK->tx ;
			FROM->vy -= ( float )BLOCK->ty ;
			FROM->vz -= ( float )BLOCK->tz ;
			trp = BLOCK->traps ;
			for ( i = BLOCK->n_traps; i > 0 ; -- i, trp ++ ) {
                *CURRENT = *( SVECTOR2 * )trp ;
                if ( trp->name_id == name_id ) {
					if ( InsideTrap() ) return 1 ;
				}
			}			
		}
#endif

#ifdef DTRP
		if ( DYNAMIC != NULL ) {
			dtrp = DYNAMIC ;
			do {
//				if ( NoCheckTrap( dtrp->attribute ) ) continue ;
				*CURRENTF = *( FVECTOR2 * )dtrp ;
				if ( dtrp->name_id == name_id ) {
					if ( InsideTrapF() ) return 1 ;
				}
			} while( ( dtrp = dtrp->next ) != NULL ) ;
		}
#endif
	}
	return 0 ;
}

/* トラップフラッシュ */
void HZX_FlashTrap( HZX_GROUP_ID id, HZX_EVT *ev )
{
	int		mapbuf ;

    CopyInsideList( ev ) ;
	mapbuf = GM_ScriptCurrentMap ;
	GM_ScriptCurrentMap = GM_CurrentMap ;
    ExecLeaveEvent( ev, id ) ;
    ev->n_inside = 0 ;
	GM_ScriptCurrentMap = mapbuf ;
}

/* 指定トラップへの構造体とそのトラップの入っているブロック構造体を得る */
/* 複数ある場合は、最初にヒットしたものになる。 */
/* シナリオトラップ非対応 */
int	HZX_FindTrap( HZX_GROUP_ID hzx_id, int name, HZX_BLOCK **blkp, HZX_TRP **trpp )
{
	HZX_HDL		*hdl ;
	HZX_GRP		*grp ;
	HZX_BLOCK	*blk ;
	HZX_TRP		*trp ;
	int			n_groups, n_blocks, n_traps ;
	int			bit, no ;

	hdl = HZX_GetCurrentHzx() ;
	n_groups = hdl->def->n_groups ;

	*trpp = NULL ;
	*blkp = NULL ;

	if ( hzx_id == 0 ) hzx_id = HZX_AllMapID ;
	while( hzx_id != 0 ) {
		no = GV_GetNo( hzx_id ) ;
		bit = GV_GetBit( no ) ;
		FinalGroupID = hzx_id;
		hzx_id &= ~bit ;
		if ( no >= HZX_CurrentHzx->def->n_groups ) continue ;
		grp = hdl->def->groups + no ;
		n_blocks = grp->n_blocks ;
		blk = grp->blocks ;
		while( -- n_blocks >= 0 ) {
			n_traps = blk->n_traps ;
			trp = blk->traps ;
			while( -- n_traps >= 0 ) {
				if ( trp->name_id == name ) {
					*trpp = trp ;
					*blkp = blk ;
					/* 最後に読みだしたトラップを保持 */
					FinalReadTrpNum = n_traps;
					FinalReadBlkNum = n_blocks;
					FinalName = name;
//					printf("trp: %p\n", trp);
					return 1 ;
				}
				trp ++ ;
			}
			blk ++ ;
		}	
	}
	FinalReadTrpNum = -1;
	FinalReadBlkNum = -1;
	FinalGroupID = 0;
	FinalName = -1;

	printf( "warning : trap %x not found.\n", name ) ;

	return 0 ;
}


/*
 * 最後に実行した HZX_FindTrap と同じ条件で、同一の名前を持つトラップを検索する
 */
int	HZX_GetNextTrap( HZX_BLOCK **blkp, HZX_TRP **trpp )
{
	HZX_HDL		*hdl ;
	HZX_GRP		*grp ;
	HZX_BLOCK	*blk ;
	HZX_TRP		*trp ;
	int			n_groups, n_blocks, n_traps ;
	int			bit, no ;
	int			name = FinalName;
	HZX_GROUP_ID		hzx_id;


	if(name < 0) return 0;

//	printf("trap.c: HZX_GetNextTrap()\n");

	hdl = HZX_GetCurrentHzx() ;
	n_groups = hdl->def->n_groups ;

	hzx_id = FinalGroupID;
	while( hzx_id != 0 ) {
		no = GV_GetNo( hzx_id ) ;
		bit = GV_GetBit( no ) ;
		FinalGroupID = hzx_id;
		hzx_id &= ~bit ;
		if ( no >= HZX_CurrentHzx->def->n_groups ) continue ;
		grp = hdl->def->groups + no ;
		n_blocks = (FinalReadBlkNum >= 0)
		  ? FinalReadBlkNum : grp->n_blocks ;
		FinalReadBlkNum = -1;
		blk = grp->blocks + grp->n_blocks - (n_blocks + 1);
		while( n_blocks >= 0 ) {
			n_traps =(FinalReadTrpNum >= 0)
			  ? FinalReadTrpNum : blk->n_traps ;
			trp = blk->traps + blk->n_traps - n_traps;
			FinalReadTrpNum = -1;
			while( --n_traps >= 0 ) {
				if ( trp->name_id == name ) {
					*trpp = trp ;
					*blkp = blk ;
					/* 最後に読みだしたトラップを保持 */
					FinalReadTrpNum = n_traps;
					FinalReadBlkNum = n_blocks;
					FinalName = name;
//					printf("trp: %p\n", trp);
					return 1 ;
				}
				trp ++ ;
			}
			blk ++ ;
			n_blocks--;
		}	
	}
	FinalReadTrpNum = -1;
	FinalReadBlkNum = -1;
	FinalGroupID = 0;
	FinalName = -1;
	
	return 0 ;
}



/* トラップスイッチをデフォルトに戻す */
/* ステージ開始時に呼ばれる */
void	HZX_SwitchTrapDefault( void )
{
	int			i, n_groups, n_blocks, n_traps ;
	HZX_GRP		*grp ;
	HZX_BLOCK	*blk ;
	HZX_TRP		*trp ;
	HZX_HDL		*hdl ;

	hdl = HZX_GetCurrentHzx() ;
	if ( hdl == NULL || hdl->def == NULL ) return ;
	n_groups = hdl->def->n_groups ;
	grp = hdl->def->groups ;
	for ( i = 0; i < n_groups; i ++, grp ++ ) {
		n_blocks = grp->n_blocks ;
		blk = grp->blocks ;
		while( -- n_blocks >= 0 ) {
			n_traps = blk->n_traps ;
			trp = blk->traps ;
			while( -- n_traps >= 0 ) {
				trp->b1.pad &= ~( HZX_TRP_ATR_NO_CHECK_SCN | HZX_TRP_ATR_CHECK_SCN ) ;
				trp ++ ;
			}
			blk ++ ;
		}
	}
}

/* トラップスイッチ切り替え（即時式） */
void	HZX_SwitchTrapQuick( hzx_id, id, on )
HZX_GROUP_ID	hzx_id ;
int				id, on ;
{
	HZX_GRP		*grp ;
	HZX_BLOCK	*blk ;
	HZX_TRP		*trp ;
#ifdef DTRP
	HZX_D_TRP	*dtrp ;
#endif
	HZX_HDL		*hdl ;
	int			n_groups, n_blocks, n_traps ;
	int			bit, no ;
	int			has_name ;

	has_name = ( id & 0xff000000 ) ? 1 : 0 ;

	hdl = HZX_GetCurrentHzx() ;
	n_groups = hdl->def->n_groups ;

//	if ( hzx_id == 0 ) hzx_id = ( 0x7fffffff ) & ~( 0x7fffffff >> n_groups ) ;
	if ( hzx_id == 0 ) hzx_id = HZX_AllMapID ;
	while( hzx_id != 0 ) {
		no = GV_GetNo( hzx_id ) ;
		bit = GV_GetBit( no ) ;
		hzx_id &= ~bit ;
		if ( no >= HZX_CurrentHzx->def->n_groups ) continue ;
		grp = hdl->def->groups + no ;
		n_blocks = grp->n_blocks ;
		blk = grp->blocks ;
		while( -- n_blocks >= 0 ) {
			n_traps = blk->n_traps ;
			trp = blk->traps ;
			while( -- n_traps >= 0 ) {
				if ( ( !has_name && ( ( trp->name_id & 0x00ffffff ) == id ) ) || 
					 ( has_name && ( trp->name_id == id ) ) ) {
					if ( !on ) {
						printf( "trap %x is OFF\n", trp->name_id ) ;
						trp->b1.pad |= HZX_TRP_ATR_NO_CHECK_SCN ;
						trp->b1.pad &= ~HZX_TRP_ATR_CHECK_SCN ;
					} else {
						printf( "trap %x is ON\n", trp->name_id ) ;
						trp->b1.pad |= HZX_TRP_ATR_CHECK_SCN ;
						trp->b1.pad &= ~HZX_TRP_ATR_NO_CHECK_SCN ;
					}
				}
				trp ++ ;
			}
			blk ++ ;
		}
#ifdef DTRP
		dtrp = grp->dynamics->traps ;
		while( dtrp != NULL ) {
			if ( ( !has_name && ( ( dtrp->name_id & 0x00ffffff ) == id ) ) || 
				( has_name && ( dtrp->name_id == id ) ) ) {
				if ( !on ) {
					printf( "trap %x is OFF\n", dtrp->name_id ) ;
					dtrp->attribute |= HZX_TRP_ATR_NO_CHECK_SCN ;
					dtrp->attribute &= ~HZX_TRP_ATR_CHECK_SCN ;
				} else {
					printf( "trap %x is ON\n", dtrp->name_id ) ;
					dtrp->attribute |= HZX_TRP_ATR_CHECK_SCN ;
					dtrp->attribute &= ~HZX_TRP_ATR_NO_CHECK_SCN ;
				}
			}
			dtrp = dtrp->next ;
		}
#endif
	}
}

/*----------------------------------------------------------------*/

/* トラップビハインド
   1ステージに４つまでＯＫ */
#define	MAX_TRAP_BEHINDS		(4)	/* 32以下にすること */
static	HZX_BEHIND		TrapBehind[ MAX_TRAP_BEHINDS ] ;
static	int				TrapBehindName[ MAX_TRAP_BEHINDS ] ;
static	int				N_TrapBehinds = 0 ;
static	int				TrapBehindsUsed = 0 ;

void	HZX_InitTrapBehindSystem( void )
{
	N_TrapBehinds = 0 ;
	TrapBehindsUsed = 0 ;
}

/* トラップビハインド登録 */
int		HZX_AddTrapBehind( void )
{
	HZX_BEHIND		*bh ;
	HZX_BLOCK		*blk ;
	HZX_TRP			*trp ;
	HZX_GROUP_ID	hzx_id ;
	int				i, used, name, map, res ;

	ASSERT( N_TrapBehinds < MAX_TRAP_BEHINDS ) ;
	used = TrapBehindsUsed ;
	for ( i = 0; i < MAX_TRAP_BEHINDS; i ++ ) {
		if ( ( used & 1 ) == 0 ) break ;
		used >>= 1 ;
	}
	if ( i == MAX_TRAP_BEHINDS ) ASSERT( 0 ) ;
	bh = &TrapBehind[ i ] ;
	TrapBehindsUsed |= ( 1 << i ) ;

	name = TrapBehindName[ i ] = GCL_GetOptionValue( 't', 0 ) ;
	map = GCL_GetOptionValue( 'm', 0 ) ;
	hzx_id = GM_GetHzxGroupID( GM_GetMapID( map ) ) ;
	
	res = HZX_FindTrap( hzx_id, name, &blk, &trp ) ;
	ASSERT( res != 0 ) ;

	/* バウンディング取得 */
	bh->b1.vx = (float)blk->tx + trp->b1.vx ;
	bh->b1.vy = (float)blk->ty + trp->b1.vy ;
	bh->b1.vz = (float)blk->tz + trp->b1.vz ;
	bh->b2.vx = (float)blk->tx + trp->b2.vx ;
	bh->b2.vy = (float)blk->ty + trp->b2.vy ;
	bh->b2.vz = (float)blk->tz + trp->b2.vz ;

	/* カメラデータ取得 */
	GCL_GetOption( 'c' ) ;
	bh->v[ 0 ] = ( float )GCL_GetNextInt() ;
	bh->v[ 1 ] = ( float )GCL_GetNextInt() ;
	bh->v[ 2 ] = ( float )GCL_GetNextInt() ;
	bh->v[ 3 ] = ( float )GCL_GetNextInt() ;
	bh->v[ 4 ] = ( float )GCL_GetNextInt() ;
	bh->v[ 5 ] = ( float )GCL_GetNextInt() ;

	GCL_GetOption( 'r' ) ;
	bh->right[ 0 ] = ( float )GCL_GetNextInt() ;
	bh->right[ 1 ] = ( float )GCL_GetNextInt() ;
	bh->right[ 2 ] = ( float )GCL_GetNextInt() ;
	bh->right[ 3 ] = ( float )GCL_GetNextInt() ;
	bh->right[ 4 ] = ( float )GCL_GetNextInt() ;
	bh->right[ 5 ] = ( float )GCL_GetNextInt() ;

	GCL_GetOption( 'l' ) ;
	bh->left[ 0 ] = ( float )GCL_GetNextInt() ;
	bh->left[ 1 ] = ( float )GCL_GetNextInt() ;
	bh->left[ 2 ] = ( float )GCL_GetNextInt() ;
	bh->left[ 3 ] = ( float )GCL_GetNextInt() ;
	bh->left[ 4 ] = ( float )GCL_GetNextInt() ;
	bh->left[ 5 ] = ( float )GCL_GetNextInt() ;

	bh->value = GCL_GetOptionValue( 'd', -1 ) ;	
	bh->flag = GCL_GetOptionValue( 'f', 0 ) ;	
	if ( bh->value != -1 ) bh->flag |= HZX_BEHIND_CHECK_DIR ;

	N_TrapBehinds ++ ;

	return 0 ;
}

/* トラップビハインド削除 */
int		HZX_RemoveTrapBehind( void )
{
	int			name, i ;

	name = GCL_GetOptionValue( 't', 0 ) ;
	for ( i = 0; i < MAX_TRAP_BEHINDS; i ++ ) {
		if ( name == TrapBehindName[ i ] ) break ;
	}
	if ( i == MAX_TRAP_BEHINDS ) return 0 ;
	TrapBehindsUsed &= ~( 1 << i ) ;
	N_TrapBehinds -- ;
	return 0 ;
}

/*----------------------------------------------------------------*/

/* ビハインドモード */
int HZX_CheckBehind( HZX_GROUP_ID id, HZX_BEHIND **bhs, FVECTOR *pos, SVECTOR *rot )
{
    int		i, n, n_behinds ;
	int		id2, g ;
    HZX_BEHIND	*bh ;
    HZX_GRP	*grp ;

	DG_COPY_VEC( GFROM, pos ) ;
	/* 追加登録に対応 */
	id = HZX_AddGroupID( id ) ;
	n = 0 ;

	while( id != 0 ) {
		g = GV_GetNo( id ) ;
		id2 = GV_GetBit( g ) ;
		id &= ~id2 ;
		if ( g >= HZX_CurrentHzx->def->n_groups ) {
			continue ;
		}
		grp = HZX_GetGroup( id2 ) ;
		bh = grp->behinds ;
		n_behinds = grp->n_behinds ;
		for ( i = 0; i < n_behinds ; i ++, bh ++ ) {
			*CURRENTF = *( FVECTOR2 * )bh ;	
			if ( InsideTrapF() ) {
				//printf( "behind[ %d ]\n", i ) ;
				/* 方向指定チェック */
				if ( bh->flag & HZX_BEHIND_CHECK_DIR ) {
					if ( GV_DiffDirAbs( rot->vy, bh->value ) > 256 ) continue ;
				} 
				bhs[ n ] = bh ; 
				if ( ++ n == 2 ) break ;
			}
		}
	}
	if ( n < 2 && N_TrapBehinds > 0 ) {
		/* トラップビハインドの検索 */
		for ( i = 0; i < MAX_TRAP_BEHINDS; i ++ ) {
			if ( TrapBehindsUsed & ( 1 << i ) ) {
				bh = &TrapBehind[ i ] ;
				*CURRENTF = *( FVECTOR2 * )bh ;	
				if ( InsideTrapF() ) {
					/* 方向指定チェック */
					if ( bh->flag & HZX_BEHIND_CHECK_DIR ) {
						if ( GV_DiffDirAbs( rot->vy, bh->value ) > 256 ) continue ;
					} 
					bhs[ n ] = bh ; 
					if ( ++ n == 2 ) break ;
				}				
			}
		}
	}
    return n ;
}
