/*
   levelx.c
   床／天井チェック バージョン２
   
   1999/11/24 M.Sonoyama
   $Id: levelx.c,v 1.1.1.3 2002/11/19 11:42:47 Yoshizawa1 Exp $
   */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <devgif.h>

#include	"gameheader.h"
#include	"../libdg/def_dma.h"

/*---------------------------------------------------------------*/

#ifdef DEBUG_MODE
int		HZX_LevelDebug = 0 ;
#endif

extern	qword		Vu0CheckLevelHazardX ;
extern	qword		Vu0CheckLevelHazardFuncX ;

extern	u_int		HZX_DP_ADDR[] ;
extern	int			HZX_DP_WHICH ;

#define	VU0_MEM_ADDR	(0x11004000)
#define	VU0_PARAM_PTR	(0x30)
#define	VU0_HIT_ADDR	(0x32)

#define	VU0_DATA_PTR	(0x33)
#define	VU0_DATA_PTR2	(0x93)

#define	VU0_CEIL_ADDR	(0x10)
#define	VU0_FLOOR_ADDR	(0x20)

#define	VU0_CEIL_HEIGHT_ADDR	(0x11)
#define	VU0_FLOOR_HEIGHT_ADDR	(0x21)

/*---------------------------------------------------------------*/

typedef struct	{
    FVECTOR	from ;
    SVECTOR	*flr_ptr[ 2 ] ;
    float	lvl_ptr[ 2 ] ;
    int		atr[ 2 ] ;
    int		flag ;

    HZX_HDL	*hzd ;
    HZX_GRP	*grp[ 2 ] ;
    HZX_BLOCK	*blk[ 2 ] ;
    SVECTOR	*flr[ 2 ] ;
    int		type[ 2 ] ;
    int		atrs[ 2 ] ;
    
    HZX_FLR	flrs[ 2 ] ;

    HZX_D_FLOOR	*dynamic[ 2 ] ;

    sceDmaTag	*dmatag ;
    int		n_insideblocks ;
    int		insideblock[ 256 ] ;
    int		First, First2 ;
    u_int	dataptr ;
    u_int	*predataptr ;

	int				tagsize ;
	int				size ;

    u_long128	tag[ 512 ] ;
} ScrPad ;

#define	MAX_TAG_SIZE	(512)

#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)

#define	Vu0Tag		(SCRPAD->tag)
#define	DmaTag		(SCRPAD->dmatag)
#define	N_InsideBlocks	(SCRPAD->n_insideblocks)
#define	InsideBlock	(SCRPAD->insideblock)
#define	First		(SCRPAD->First)
#define	First2		(SCRPAD->First2)
#define	DataPtr		(SCRPAD->dataptr)
#define	PreDataPtr	(SCRPAD->predataptr)

#define	FROM		(&(SCRPAD->from))
#define	HIT_PTR		(SCRPAD->flr_ptr)
#define	LVL_PTR		(SCRPAD->lvl_ptr)
#define	ATR_PTR		(SCRPAD->atr)
#define	FLAG		(&(SCRPAD->flag))

#define	HZX_PTR		(SCRPAD->hzd)
#define	HZX_GRP_PTR	(SCRPAD->grp)
#define	HZX_BLOCK_PTR	(SCRPAD->blk)
#define	HZX_FLR_PTR	(SCRPAD->flr)
#define	HZX_TYPE_PTR	(SCRPAD->type)
#define	HZX_ATR_PTR	(SCRPAD->atrs)

#define	FLOOR_PTR	(&(SCRPAD->flrs[ 0 ]))
#define	CEIL_PTR	(&(SCRPAD->flrs[ 1 ]))

#define	DYNAMIC		(SCRPAD->dynamic)

#define	TAGSIZE		(&(SCRPAD->tagsize))
#define	SIZE		(&(SCRPAD->size))

/*---------------------------------------------------------------*/

#if 0
/* デバッグ */
static	void	DebugPrint( void )
{
    int		*res ;
    float	*fres ;

	//    FlushCache( 0 ) ;
	
	if ( HZX_LevelDebug == 0 ) return ;
	HZX_LevelDebug = 0 ;

	if ( !( GV_PadData->status & PAD_L1 ) ) return ;

    printf( "---- debug print ----\n" ) ;
    DumpFloat( VU0_MEM_ADDR + 0x0a * 16 ) ;
    DumpFloat( VU0_MEM_ADDR + 0x0b * 16 ) ;
    DumpFloat( VU0_MEM_ADDR + 0x0c * 16 ) ;
    DumpFloat( VU0_MEM_ADDR + 0x0d * 16 ) ;
    DumpFloat( VU0_MEM_ADDR + 0x0e * 16 ) ;
    DumpFloat( VU0_MEM_ADDR + 0x0f * 16 ) ;
    printf( "---- debug print end ----\n" ) ;
}
#else
#define	DebugPrint()
#endif

/*---------------------------------------------------------------*/

/* 途中実行 */
static	u_int	*ExecHalfway( tag )
u_int			*tag ;
{
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; /* 加算展開書き込みＯＦＦ */
    tag[ 3 ] = SCE_VIF0_SET_FLUSHE( 0 ) ;
    tag[ 4 ] = DMATAG_SET_QWC( DMATAG_ID_END, 0 ) ;
    tag[ 6 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 7 ] = SCE_VIF0_SET_NOP( 0 ) ;
    sceDmaSend( HZX_Vif0Dma, ( sceDmaTag * )( ( u_int )Vu0Tag | 0x80000000 ) ) ;
    while ( sceDmaSync( HZX_Vif0Dma, 0, 0 ) ) ;
    DmaTag = ( sceDmaTag * )Vu0Tag ;
    *TAGSIZE = 0 ;
    return ( u_int * )DmaTag ;
}

/* 初期設定タグを生成 */
static	void	SetCheckLevelParamator()
{
    int		*tag ;
    float	*ftag ;

    DmaTag = ( sceDmaTag * )Vu0Tag ;
	*TAGSIZE = 3 ;
    tag = ( u_int * )DmaTag ;
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 1 ) ;
    tag[ 2 ] = SCE_VIF0_SET_STCYCL( 1, 1, 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( VU0_PARAM_PTR, 1, VIF_DATA128, 0 ) ;
    tag += 4 ;
    /* 検索座標 */
    ftag = ( float * )tag ;
    ftag[ 0 ] = FROM->vx ;
    ftag[ 1 ] = FROM->vy ;
    ftag[ 2 ] = FROM->vz ;
    ftag[ 3 ] = 0.0F ;
    tag += 4 ;
    /* マイクロプログラムロード */
    *( u_long128 * )tag = *( u_long128 * )Vu0CheckLevelHazardX ;
    tag += 4 ;
    DmaTag = ( sceDmaTag * )tag ;
}

/* １ストリップのタグを生成 */
static	void	MakeOneLevelStripTag( b, seg, no )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		no ; /* ストリップナンバー */
{
    u_int	*tag ;
    int		n ;

    tag = ( u_int * )DmaTag ;
    n = seg->size ;
    DataPtr = HZX_DP_ADDR[ HZX_DP_WHICH ] ;
    HZX_DP_WHICH = 1 - HZX_DP_WHICH ;

	*SIZE = ( First2 ) ? 5 : 3 ;
	if ( *TAGSIZE + *SIZE >= MAX_TAG_SIZE - 4 ) {
		/* タグサイズオーバー 
		   ここまでで一回実行する。 */
		tag = ExecHalfway( tag ) ;
	}
	*TAGSIZE = *TAGSIZE + *SIZE ;

    if ( First2 ) {
		/* ブロックセンターを加算書き込み用レジスタに設定 */
		tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 1 ) ;
		tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
		tag[ 3 ] = SCE_VIF0_SET_STROW( 0 ) ; /* 加算書き込みレジスタにセット */
		tag[ 4 ] = b->tx ;
		tag[ 5 ] = b->ty ;
		tag[ 6 ] = b->tz ;
		tag[ 7 ] = 0 ;
		tag += 8 ;
    }

    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, 3 ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->tag ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; 
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr, 3, VIF_DATA128, 0 ) ;
    tag += 4 ;

    /* 頂点データ */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, n ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->verts ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x01, 0 ) ; 
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr + 3, n * 2, 0x0d, 0 ) ; /* V4-16bit */
    tag += 4 ;
    /* プログラム実行タグ */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckLevelHazardFuncX[ 1 - First ], 0 ) ;
    tag += 4 ;

    DmaTag = ( sceDmaTag * )tag ;
}

/* １ブロックチェック */
static	void	CheckOneBlock( b, seg, n_segs, flr_flag )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		n_segs ;
int		flr_flag ;
{
    int		i ;
    SVECTOR	svfrom ;
    int		all, all2 ;

    First2 = 1 ;
    all = all2 = 0 ;
    svfrom.vx = ( short )( FROM->vx - ( float )b->tx ) ;
    svfrom.vz = ( short )( FROM->vz - ( float )b->tz ) ;
    for ( i = 0; i < n_segs; i ++ ) {
		all2 += seg->b1.pad / seg->b2.pad ;
		/* フラグチェック */
		if ( seg->atr & flr_flag ) goto level_check_skip_strip ;
		/* バウンディングチェック */
		if ( seg->b1.vx > svfrom.vx || seg->b2.vx < svfrom.vx ||
			seg->b1.vz > svfrom.vz || seg->b2.vz < svfrom.vz ) goto level_check_skip_strip ;
#ifdef DEBUG_MODE
		if ( HZX_LevelDebug == 1 ) HZX_ViewVuFlr( b, seg ) ;
#endif
		all += seg->b1.pad / seg->b2.pad ;
		if ( First ) SetCheckLevelParamator() ;
		MakeOneLevelStripTag( b, seg, i ) ;
		First = First2 = 0 ;
		level_check_skip_strip :
        seg ++ ;
    }
}

#ifdef DEBUG_MODE
#if 0
static	void	ViewOneBlock( b, seg, n_segs, flr_flag )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		n_segs ;
int		flr_flag ;
{
    int		i ;

    for ( i = 0; i < n_segs; i ++ ) {
		HZX_ViewVuFlr( b, seg ) ;
		seg ++ ;
    }
}
#endif
#endif

/* 動的床登録 */
static	void	MakeDynamicFloorTag( seg )
HZX_D_FLOOR	*seg ;
{
    u_int	*tag ;
    int		n ;

    tag = ( u_int * )DmaTag ;
    n = seg->tag[ 0 ] ;

    DataPtr = HZX_DP_ADDR[ HZX_DP_WHICH ] ;
    HZX_DP_WHICH = 1 - HZX_DP_WHICH ;

	if ( *TAGSIZE + 2 >= MAX_TAG_SIZE - 4 ) {
		/* タグサイズオーバー 
		   ここまでで一回実行する。 */
		tag = ExecHalfway( tag ) ;
	}
	*TAGSIZE = *TAGSIZE + 2 ;

    /* タグと頂点を一気に転送 */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, 3 + n ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->tag ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; 
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr, 3 + n, VIF_DATA128, 0 ) ;
    tag += 4 ;
    /* プログラム実行タグ */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckLevelHazardFuncX[ 1 - First ], 0 ) ;
    tag += 4 ;

    DmaTag = ( sceDmaTag * )tag ;
}

static	void	CheckDynamicFloor( flrs, flr_flag )
HZX_D_FLOOR	*flrs ;
int		flr_flag ;
{
    HZX_D_FLOOR		*flr ;
    int			all, all2 ;

    all = all2 = 0 ;
    flr = flrs ;
    while( flr != NULL ) {
		all2 += 1 ;
		/* フラグチェック */
		if ( flr->atr & ( flr_flag | HZX_FLOOR_SKIP ) ) goto level_check_skip_strip_d ;
		/* バウンディングチェック */
		if ( flr->b1.vx > FROM->vx || flr->b2.vx < FROM->vx ||
			flr->b1.vz > FROM->vz || flr->b2.vz < FROM->vz ) goto level_check_skip_strip_d ;
		all += 1 ;
		if ( First ) SetCheckLevelParamator() ;
#ifdef DEBUG_MODE
		if ( HZX_LevelDebug == 1 ) HZX_ViewDynamicFloor( flr, flr->tag[ 0 ] ) ;
#endif
		MakeDynamicFloorTag( flr ) ;
		First = 0 ;
		level_check_skip_strip_d :
        flr = flr->next ;
    }
}

/* 結果を格納 */
static	inline	void	SetResult( hit )
int		hit ;
{
    HZX_VuSEG	*s ;
    HZX_GRP	*g ;
    int		*tag ; 
    int		blk, seg, ptp, grp ;
    HZX_D_FLOOR	*dseg ;

    if ( hit & 1 ) {
		tag = ( u_int * )( VU0_MEM_ADDR + VU0_FLOOR_ADDR * 16 ) ;
		ptp = tag[ 0 ] ; blk = tag[ 1 ] ; seg = tag[ 2 ] ; grp = tag[ 3 ] ;
		HZX_GRP_PTR[ 0 ] = g = HZX_PTR->def->groups + grp ;
		//printf( "%d %d %d %d\n", ptp, blk, seg, grp ) ;
		if ( blk >= 0 ) {
			HZX_BLOCK_PTR[ 0 ] = g->blocks + blk ;
			if ( seg >= HZX_RECOIL_TYPE_SHIFT ) {
				s = HZX_BLOCK_PTR[ 0 ]->bul_flrs + seg - HZX_RECOIL_TYPE_SHIFT ;
			} else {
				s = HZX_BLOCK_PTR[ 0 ]->flrs + seg ;
			}
			HZX_FLR_PTR[ 0 ] = s->verts + ( s->b1.pad - ptp ) ;
			HZX_TYPE_PTR[ 0 ] = s->b2.pad ;
			HZX_ATR_PTR[ 0 ] = s->atr ;
		} else {
			dseg = HZX_GRP_PTR[ 0 ]->dynamics->flrs ;
			seg = HZX_GRP_PTR[ 0 ]->dynamics->n_flrs - seg - 1 ;
			while( -- seg >= 0 ) dseg = dseg->next ;
			DYNAMIC[ 0 ] = dseg ;
			HZX_ATR_PTR[ 0 ] = dseg->atr ;
		}
    }
    if ( hit & 2 ) {
		tag = ( u_int * )( VU0_MEM_ADDR + VU0_CEIL_ADDR * 16 ) ;
		ptp = tag[ 0 ] ; blk = tag[ 1 ] ; seg = tag[ 2 ] ; grp = tag[ 3 ] ;
		HZX_GRP_PTR[ 1 ] = g = HZX_PTR->def->groups + grp ;
		if ( blk >= 0 ) {
			HZX_BLOCK_PTR[ 1 ] = g->blocks + blk ;
			if ( seg >= HZX_RECOIL_TYPE_SHIFT ) {
				s = HZX_BLOCK_PTR[ 1 ]->bul_flrs + seg - HZX_RECOIL_TYPE_SHIFT ;
			} else {
				s = HZX_BLOCK_PTR[ 1 ]->flrs + seg ;
			}
			HZX_FLR_PTR[ 1 ] = s->verts + ( s->b1.pad - ptp ) ;
			HZX_TYPE_PTR[ 1 ] = s->b2.pad ;
			HZX_ATR_PTR[ 1 ] = s->atr ;
		} else {
			dseg = HZX_GRP_PTR[ 1 ]->dynamics->flrs ;
			seg = HZX_GRP_PTR[ 1 ]->dynamics->n_flrs - seg - 1 ;
			while( -- seg >= 0 ) dseg = dseg->next ;
			DYNAMIC[ 1 ] = dseg ;
			HZX_ATR_PTR[ 1 ] = dseg->atr ;
		}
    }
}

/* チェック実行 */
static	ALIGN16_PRE u_int	ExecCheckData[] ALIGN16_POST = {
    DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ), 
    0,
    SCE_VIF0_SET_STMOD( 0x00, 0 ),
    SCE_VIF0_SET_FLUSHE( 0 ),
    DMATAG_SET_QWC( DMATAG_ID_END, 0 ),
    0,
    SCE_VIF0_SET_NOP( 0 ),
    SCE_VIF0_SET_NOP( 0 )
} ;

static	int	ExecLevelCheck( void )
{
    int		hit, i ;
    u_long128	*dst, *src ;

    dst = ( u_long128 * )DmaTag ;
    src = ( u_long128 * )ExecCheckData ;
    for ( i = 0; i < 2; i ++ ) *( dst ++ ) = *( src ++ ) ;
	HZXD( if ( GV_PadData->status & PAD_L1 ) FlushCache( 0 ) ; )
		sceDmaSend( HZX_Vif0Dma, ( sceDmaTag * )( ( u_int )Vu0Tag | 0x80000000 ) ) ;

    /* Ｓｅｎｄ終了待ちの間に */
    HZX_BLOCK_PTR[ 0 ] = HZX_BLOCK_PTR[ 1 ] = NULL ;
    HZX_FLR_PTR[ 0 ] = HZX_FLR_PTR[ 1 ] = NULL ;
    HZX_GRP_PTR[ 0 ] = HZX_GRP_PTR[ 1 ] = NULL ;
    DYNAMIC[ 0 ] = DYNAMIC[ 1 ] = NULL ;

#ifdef DEBUG_MODE
    {
		int		size ;
		size = ( ( u_int )DmaTag - ( u_int )Vu0Tag ) / sizeof( u_long128 ) ;
		if ( size >= MAX_TAG_SIZE ) {
			printf( "tag size over : %d \n", size ) ;
		} 
    }
#endif

    /* DMA Send 終了待ち */
    while ( sceDmaSync( HZX_Vif0Dma, 0, 0 ) ) ;    
    /* マイクロプログラム終了待ち */
    asm( "qmfc2.i    $0, vf01 " ) ;
    while( *VIF0_STAT & VIF0_STAT_VEW_M ) ;  
    /* タグ開放 */
    DmaTag = ( sceDmaTag * )Vu0Tag ;
	HZXD( if ( GV_PadData->status & PAD_L1 ) FlushCache( 0 ) ; )
#if 1
	DebugPrint() ;
#endif
    hit = *( u_int * )( VU0_MEM_ADDR + VU0_HIT_ADDR * 16 ) ;
    SetResult( hit ) ;
    return hit ;
}

/*---------------------------------------------------------------*/

/* ブロックチェック */
static	inline	void	CheckBlockBound( grp, chk_flag )
HZX_GRP		*grp ;
int		chk_flag ;
{
    int		i, x, z, block ;
    int		size, end ;

    x = ( ( int )FROM->vx - grp->bound_min_x ) / grp->block_size_x ;
    z = ( ( int )FROM->vz - grp->bound_min_z ) / grp->block_size_z ;
    if ( x < 0 || z < 0 || x >= grp->div_x || z >= grp->div_z ) return ;

    size = grp->div_x * grp->div_z ;
	end = grp->div_y ;
	if ( chk_flag & HZX_CHK_NOCHECK_CEIL ) {
		end = ( FROM->vy - grp->bound_min_y ) / grp->block_size_y + 1 ;
		if ( end >= grp->div_y ) end = grp->div_y ;
	} 
    for ( i = 0; i < end; i ++ ) {
		block = grp->div_x * z + x + i * size ;
		if ( block >= 0 && block < grp->n_blocks ) {
#ifdef DEBUG
			if ( N_InsideBlocks == 512 ) {
				printf( "too many inside blocks\n" ) ;
				break ;
			}
#endif
			InsideBlock[ N_InsideBlocks ] = block ;
			N_InsideBlocks ++ ;
		}
    }
}

/*---------------------------------------------------------------*/

/* 床／天井チェック */
int		HZX_LevelHazardCheck( HZX_GROUP_ID id, FVECTOR *from, int chk_flag, int flr_flag )
{
    HZX_HDL	*hzd ;
    int		hit, i, group, recoil_check, only_recoil ;
    HZX_GRP	*grp ;
    HZX_BLOCK	*blk, *b ;

    hzd = HZX_GetCurrentHzx() ;
    HZX_PTR = hzd ;
    hit = 0 ;
    First = First2 = 1 ;
    DataPtr = VU0_DATA_PTR ;
    HZX_DP_WHICH = 0 ;

	if ( chk_flag & HZX_CHK_RECOIL_TYPE_ONLY ) {
		only_recoil = recoil_check = 1 ;
	} else {
		only_recoil = 0 ;
		recoil_check 
			= ( ( flr_flag & HZX_FLOOR_RECOIL_TYPE ) || ( chk_flag & HZX_CHK_RECOIL_TYPE ) ) ? 1 : 0 ;
	}

	/* サイファー専用チェック */
	if ( chk_flag & HZX_CHK_CYPHER ) {
		flr_flag &= ~HZX_FLOOR_CYPHER ;
	} else {
		flr_flag |= HZX_FLOOR_CYPHER ;
	}

	id = HZX_AddGroupID( id ) ;

    while( id != 0 ){
		group = GV_GetNo( id );
		id &= ~GV_GetBit( group );
#ifdef DEBUG
		if ( group < 0 || group >= hzd->def->n_groups ) {
			//printf( "warning : groupNo < 0 || groupNo > Max\n : %d", group ) ;
			continue ;
		}
#endif
		grp = hzd->grp + group ;
		if ( grp->n_blocks == 0 ) continue ;
		blk = grp->blocks ;
		*FROM = *from ;
		/* 固定床チェック */
		if ( chk_flag & HZX_CHK_F_FLOOR ) {
			/* ブロックチェック */
			N_InsideBlocks = 0 ;
			CheckBlockBound( grp, chk_flag ) ;
			for ( i = 0; i < N_InsideBlocks; i ++ ) {
				b = blk + InsideBlock[ i ] ;
				if ( !only_recoil && b->n_flrs > 0 ) CheckOneBlock( b, b->flrs, b->n_flrs, flr_flag ) ;
				if ( recoil_check && b->n_bul_flrs > 0 ) {
					CheckOneBlock( b, b->bul_flrs, b->n_bul_flrs, flr_flag ) ;
				}
#if 0
#ifdef DEBUG_MODE
				if ( HZX_LevelDebug == 2 ) {
					ViewOneBlock( b, b->bul_flrs, b->n_bul_flrs, flr_flag ) ;
				}
#endif
#endif
			}
		}
		/* 動的床チェック */
		if ( ( chk_flag & HZX_CHK_D_FLOOR ) &&
			( grp->dynamics->n_flrs > 0 ) ) {
			CheckDynamicFloor( grp->dynamics->flrs, flr_flag ) ;
		}
    }
    if ( First == 0 ) hit = ExecLevelCheck() ;
    return hit ;
}

/*----------------------------------------------------------------*/

static	void	SetFloor( flr, b, v, n )
HZX_FLR		*flr ;
HZX_BLOCK	*b ;
SVECTOR		*v ;
int		n ;
{
    float	tmp ;

    if ( b == NULL || v == NULL ) return ;
    flr->p1.x = v->vx + b->tx ;
    flr->p1.y = v->vy + b->ty ;
    flr->p1.z = v->vz + b->tz ; 
    flr->p1.h = v->pad ;
    v ++ ;
    flr->p2.x = v->vx + b->tx ; 
    flr->p2.y = v->vy + b->ty ;
    flr->p2.z = v->vz + b->tz ; 
    flr->p2.h = v->pad ;
    v ++ ;
    flr->p3.x = v->vx + b->tx ;
    flr->p3.y = v->vy + b->ty ;
    flr->p3.z = v->vz + b->tz ; 
    flr->p3.h = v->pad ;
    v ++ ;
    tmp = flr->p2.h ; flr->p2.h = flr->p3.h ; flr->p3.h = tmp ;
	flr->p4.h = n ;
    if ( n == 3 ) {
		flr->p4.x = flr->p3.x ;
		flr->p4.y = flr->p3.y ;
		flr->p4.z = flr->p3.z ;
		return ;
	}
    flr->p4.x = v->vx + b->tx ;
    flr->p4.y = v->vy + b->ty ;
    flr->p4.z = v->vz + b->tz ;
}

static	void	SetFloorD( flr, dflr )
HZX_FLR		*flr ;
HZX_D_FLOOR	*dflr ;
{
    float	tmp ;

    GV_IVtoFV( &dflr->p1, &flr->p1, 4 ) ;
    GV_IVtoFV( &dflr->p2, &flr->p2, 4 ) ;
    GV_IVtoFV( &dflr->p3, &flr->p3, 4 ) ;
    GV_IVtoFV( &dflr->p4, &flr->p4, 3 ) ;
    tmp = flr->p1.y ; flr->p1.y = flr->p1.z ; flr->p1.z = tmp ;
    tmp = flr->p2.y ; flr->p2.y = flr->p2.z ; flr->p2.z = tmp ;
    tmp = flr->p3.y ; flr->p3.y = flr->p3.z ; flr->p3.z = tmp ;
    tmp = flr->p4.y ; flr->p4.y = flr->p4.z ; flr->p4.z = tmp ;
    tmp = flr->p2.h ; flr->p2.h = flr->p3.h ; flr->p3.h = tmp ;
	flr->p4.h = dflr->tag[ 0 ] ;
}

/* 結果を取得 */
void	HZX_GetLevelHazard( HZX_FLR *flr, int *atr )
{	
    if ( DYNAMIC[ 0 ] == NULL ) {
		SetFloor( flr, HZX_BLOCK_PTR[ 0 ], HZX_FLR_PTR[ 0 ], HZX_TYPE_PTR[ 0 ] ) ;
		flr->ptr = NULL ;
    } else {
		SetFloorD( flr, DYNAMIC[ 0 ] ) ;
		flr->ptr = DYNAMIC[ 0 ] ;
    }
	if ( atr != NULL ) atr[ 0 ] = HZX_ATR_PTR[ 0 ] ;
    flr->attribute = HZX_ATR_PTR[ 0 ] ;
	flr->type = HZX_TYPE_FLOOR ;
	flr ++ ;
    if ( DYNAMIC[ 1 ] == NULL ) {
		SetFloor( flr, HZX_BLOCK_PTR[ 1 ], HZX_FLR_PTR[ 1 ], HZX_TYPE_PTR[ 1 ] ) ;
		flr->ptr = NULL ;
    } else {
		SetFloorD( flr, DYNAMIC[ 1 ] ) ;	
		flr->ptr = DYNAMIC[ 1 ] ;
    }
	if ( atr != NULL ) atr[ 1 ] = HZX_ATR_PTR[ 1 ] ;
    flr->attribute = HZX_ATR_PTR[ 1 ] ;
	flr->type = HZX_TYPE_FLOOR ;
}

/* 結果のポインタを取得 */
void	HZX_GetLevelHazardPtr( SVECTOR **flr, HZX_BLOCK **blk )
{
    flr[ 0 ] = HZX_FLR_PTR[ 0 ] ;
    flr[ 1 ] = HZX_FLR_PTR[ 1 ] ;
    blk[ 0 ] = HZX_BLOCK_PTR[ 0 ] ;
    blk[ 1 ] = HZX_BLOCK_PTR[ 1 ] ;
}

/* 結果の形状を取得（３ ＯＲ ４） */
void	HZX_GetLevelHazardType( int *type )
{
    type[ 0 ] = HZX_TYPE_PTR[ 0 ] ;
    type[ 1 ] = HZX_TYPE_PTR[ 1 ] ;
}

/* 結果の高さのみ取得 */
void	HZX_GetLevelHeight( float *lvl_ptr )
{
    lvl_ptr[ 0 ] = *( float * )( VU0_MEM_ADDR + VU0_FLOOR_HEIGHT_ADDR * 16 ) ;
    lvl_ptr[ 1 ] = *( float * )( VU0_MEM_ADDR + VU0_CEIL_HEIGHT_ADDR * 16 ) ;
}

/* 結果の床高さを取得 */
float	HZX_GetFloorLevel( void )
{
    return *( float * )( VU0_MEM_ADDR + VU0_FLOOR_HEIGHT_ADDR * 16 ) ;
}

/* 結果の床アトリビュートを取得 */
int	HZX_GetLevelAtr( void )
{
    return HZX_ATR_PTR[ 0 ] ;
}

/* 以前のバージョン用 */
void	HZX_GetLevelHazardF( HZX_FLR **flrs )
{
    HZX_GetLevelHazard( FLOOR_PTR, ATR_PTR ) ;
    flrs[ 0 ] = FLOOR_PTR ;
    flrs[ 1 ] = CEIL_PTR ;
}

/*----------------------------------------------------------------*/

/*
   ３Ｄベクトルを障害ベクトルに変換
   */
static	void	FV_to_HV( sv, hv )
FVECTOR		*sv ;
HZX_FVEC	*hv ;
{
    hv->x = sv->vx ;
    hv->y = sv->vy ;
    hv->z = sv->vz ;
}

/*
   傾斜型ユカの高さを計算
   */
static	float	SlopeFloorLevel( flr, from )
HZX_FLR		*flr ;
HZX_FVEC	*from ;
{
    float		pf_n, h ;
    FDVECTOR		flr_p1, p1_from, normal, *vfrom ;

    flr_p1.vx = flr->p1.x ;
    flr_p1.vy = flr->p1.z ;

    vfrom = ( FDVECTOR * )from ;
    p1_from.vx = vfrom->vx - flr_p1.vx ;
    p1_from.vy = vfrom->vy - flr_p1.vy ;

    normal.vx = flr->p1.h ;
    normal.vy = flr->p2.h ;
    {
		FVECTOR		f1, f2, f0 ;

		f1.vx = p1_from.vx ;
		f1.vy = p1_from.vy ;
		f1.vz = f1.vw = 0 ;
		f2.vx = - normal.vy ;
		f2.vy = normal.vx ;
		f2.vz = f2.vw = 0 ;
		_sceVu0OuterProduct( &f0, &f1, &f2 ) ;
		pf_n = f0.vz ;
    }
    h = ( float )flr->p1.y - pf_n / ( float )flr->p3.h ;
    return h ;
}

/* 傾斜床の高さを調べる２ */
void	HZX_SlopeFloorLevel( float *h, FVECTOR *mov, HZX_FLR *flr )
{
    HZX_FVEC	from ;

    FV_to_HV( mov, &from ) ;
    *h = SlopeFloorLevel( flr, &from ) ;
}

/*
   現在のブロック
   */
HZX_BLOCK	*HZX_GetInsideBlock( HZX_GROUP_ID id, FVECTOR *mov )
{
    HZX_GRP	*grp ;
    HZX_BLOCK	*blk ;
	HZX_HDL *hzd;
    int		i, flag ;
    int		x, y, z, block ;

	hzd = HZX_GetCurrentHzx();
    grp = hzd->grp ;
    for ( i = 0 ;i < hzd->def->n_groups; i ++, grp ++ ) {
		flag = id & GM_GetBit( i ) ;
		if ( !flag || grp->n_blocks == 0 ) continue ;
		blk = grp->blocks ;
		x = ( int )( ( mov->vx - grp->bound_min_x ) / grp->block_size_x ) ;
		z = ( int )( ( mov->vz - grp->bound_min_z ) / grp->block_size_z ) ;
		y = ( int )( ( mov->vy - grp->bound_min_y ) / grp->block_size_y ) ;
		if ( x < 0 || y < 0 || z < 0 || x >= grp->div_x ||
			z >= grp->div_z || z >= grp->div_z ) return NULL ;
		block = grp->div_x * z + x + y * grp->div_x * grp->div_z ;
		if ( block >= 0 && block < grp->n_blocks ) return ( blk + block ) ;
    }
    return NULL ;
}

/* バウンディング計算 */
void	HZX_GetBounding( HZX_HZD *hzd, FVECTOR *max, FVECTOR *min )
{
	float		y1, y2 ;

	if ( hzd->type == HZX_TYPE_SEGMENT ) {
		max->vx = DG_MAX( hzd->p1.x, hzd->p2.x ) ;
		max->vz = DG_MAX( hzd->p1.z, hzd->p2.z ) ;
		y1 = hzd->p1.y + hzd->p1.h ;
		y2 = hzd->p2.y + hzd->p2.h ;
		max->vy = DG_MAX( y1, y2 ) ;
		min->vx = DG_MIN( hzd->p1.x, hzd->p2.x ) ;
		min->vz = DG_MIN( hzd->p1.z, hzd->p2.z ) ;
		min->vy = DG_MIN( hzd->p1.y, hzd->p2.y ) ;
	} else {
		max->vx = DG_MAX( hzd->p1.x, hzd->p2.x ) ;
		max->vy = DG_MAX( hzd->p1.y, hzd->p2.y ) ;
		max->vz = DG_MAX( hzd->p1.z, hzd->p2.z ) ;
		min->vx = DG_MIN( hzd->p1.x, hzd->p2.x ) ;
		min->vy = DG_MIN( hzd->p1.y, hzd->p2.y ) ;
		min->vz = DG_MIN( hzd->p1.z, hzd->p2.z ) ;

		max->vx = DG_MAX( max->vx, hzd->p3.x ) ;
		max->vy = DG_MAX( max->vy, hzd->p3.y ) ;
		max->vz = DG_MAX( max->vz, hzd->p3.z ) ;
		min->vx = DG_MIN( min->vx, hzd->p3.x ) ;
		min->vy = DG_MIN( min->vy, hzd->p3.y ) ;
		min->vz = DG_MIN( min->vz, hzd->p3.z ) ;

		if ( hzd->p4.h == 3 ) return ;

		max->vx = DG_MAX( max->vx, hzd->p4.x ) ;
		max->vy = DG_MAX( max->vy, hzd->p4.y ) ;
		max->vz = DG_MAX( max->vz, hzd->p4.z ) ;
		min->vx = DG_MIN( min->vx, hzd->p4.x ) ;
		min->vy = DG_MIN( min->vy, hzd->p4.y ) ;
		min->vz = DG_MIN( min->vz, hzd->p4.z ) ;
	}
}
