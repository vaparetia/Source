/*
   nearx.c
   ニアハザードチェック
   
   1999/11/18 M.Sonoyama
   $Id: nearx.c,v 1.1.1.3 2002/11/19 11:42:49 Yoshizawa1 Exp $
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

#include 	<devvif0.h>
#include 	<devvu0.h>

#include	"gameheader.h"
#include	"../libdg/def_dma.h"
#include	"libutl.h"

/*----------------------------------------------------------------------*/

#ifdef DEBUG_MODE
int		HZX_NearDebug = 0 ;
#endif

//#define	NEW

#define	MAX_TAG_SIZE 	(512)

typedef	struct	{
    FVECTOR			react ;
    u_int			hit ;

    HZX_HDL			*hzd ;
    HZX_GRP			*grp[ 2 ] ;
    HZX_BLOCK		*blk[ 2 ] ;
    SVECTOR			*segs[ 2 ] ;
    int				atrs[ 2 ] ;

    HZX_SEG			segments[ 2 ] ;
    int				attributes[ 2 ] ;

    HZX_D_SEGMENT	*dynamic[ 2 ] ;

    sceDmaTag		*dmatag ;
    int				n_insideblocks ;
    int				insideblock[ 256 ] ;
    int				First, First2 ;
    u_int			dataptr ;
    u_int			*predataptr ;

	int				tagsize ;
	int				size ;

    u_long128		tag[ MAX_TAG_SIZE ] ;
} ScrPad ;

#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)

#define	Vu0Tag		(SCRPAD->tag)
#define	DmaTag		(SCRPAD->dmatag)
#define	N_InsideBlocks	(SCRPAD->n_insideblocks)
#define	InsideBlock	(SCRPAD->insideblock)
#define	First		(SCRPAD->First)
#define	First2		(SCRPAD->First2)
#define	DataPtr		(SCRPAD->dataptr)
#define	PreDataPtr	(SCRPAD->predataptr)

#define	FIRST		(&(SCRPAD->first))
#define	SECOND		(&(SCRPAD->second))
#define	HIT		(&(SCRPAD->hit))
#define	REACT		(&(SCRPAD->react))

#define	HZX_PTR		(SCRPAD->hzd)
#define	HZX_GRP_PTR	(SCRPAD->grp)
#define	HZX_BLOCK_PTR	(SCRPAD->blk)
#define	HZX_SEGS_PTR	(SCRPAD->segs)
#define	HZX_ATR_PTR	(SCRPAD->atrs)

#define	SEGS_PTR	(SCRPAD->segments)
#define	ATR_PTR		(SCRPAD->attributes)

#define	DYNAMIC		(SCRPAD->dynamic)

#define	TAGSIZE		(&(SCRPAD->tagsize))
#define	SIZE		(&(SCRPAD->size))

/*----------------------------------------------------------------------*/

#define	VU0_MEM_ADDR	(0x11004000)
#define	VU0_PARAM_PTR	(0x30)
#define	VU0_DATA_PTR	(0x33)
//#define	VU0_DATA_PTR2	(VU0_DATA_PTR + 96)
#define	VU0_DATA_PTR2	(0x93)

#define	VU0_HIT_PTR		(0x32)

#define	VU0_REACT_PTR		(0x00)

#define	VU0_FIRST_PTR		(0x10)
#define	VU0_FIRST_EDGE_PTR	(0x11)
#define	VU0_FIRST_VEC_PTR      	(0x12)

#define	VU0_SECOND_PTR		(0x20)
#define	VU0_SECOND_EDGE_PTR	(0x21)
#define	VU0_SECOND_VEC_PTR      (0x22)

/*----------------------------------------------------------------------*/

extern	qword		Vu0CheckNearHazardX ;
extern	qword		Vu0CheckNearHazardFuncX ;

u_int				HZX_DP_ADDR[ 2 ] = { VU0_DATA_PTR, VU0_DATA_PTR2 } ;
int					HZX_DP_WHICH ;

#define	DP_ADDR		HZX_DP_ADDR
#define	DP_WHICH	HZX_DP_WHICH

/*----------------------------------------------------------------------*/

#if 0
/* デバッグ */
static	void	DebugPrint( void )
{
    int		*res ;
    float	*fres ;

	if ( !( GV_PadData->press & PAD_L1 ) ) return ;

    FlushCache( 0 ) ;
    printf( "---- debug print ----\n" ) ;
    fres = ( float * )( VU0_MEM_ADDR + 0x04 * 16 ) ;
    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
    fres = ( float * )( VU0_MEM_ADDR + 0x05 * 16 ) ;
    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
    fres = ( float * )( VU0_MEM_ADDR + 0x06 * 16 ) ;
    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
    fres = ( float * )( VU0_MEM_ADDR + 0x07 * 16 ) ;
    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
    printf( "---- debug print end ----\n" ) ;
}
#endif

/*----------------------------------------------------------------------*/

/* 結果格納用 */
static	inline	int	StoreHit( void )
{
    IVECTOR	hit ;

    asm volatile ("
    sqc2	vf30,0x00(%0)
    " : : "r"(&hit) ) ;
    return hit.vx ;
}

static	inline	void	StoreFirst( res )
int		*res ;
{
    asm volatile ("
    sqc2	vf21,0x00(%0)
    " : : "r"(res) ) ;
}

static	inline	void	StoreSecond( res )
int		*res ;
{
    asm volatile ("
    sqc2	vf24,0x00(%0)
    " : : "r"(res) ) ;
}

static	inline	void	StoreFirstVec( v )
FVECTOR		*v ;
{
    asm volatile ("
    sqc2	vf23,0x00(%0)
    " : : "r"(v) ) ;
}

static	inline	void	StoreSecondVec( v )
FVECTOR		*v ;
{
    asm volatile ("
    sqc2	vf26,0x00(%0)
    " : : "r"(v) ) ;
}

static	inline	int	StoreFirstIsEdge( void )
{
    IVECTOR	hit ;

    asm volatile ("
    sqc2	vf22,0x00(%0)
    " : : "r"(&hit) ) ;
    return hit.vx ;
}

static	inline	int	StoreSecondIsEdge( void )
{
    IVECTOR	hit ;

    asm volatile ("
    sqc2	vf25,0x00(%0)
    " : : "r"(&hit) ) ;
    return hit.vx ;
}

static	inline	void	StoreReactVec( v )
FVECTOR		*v ;
{
    asm volatile ("
    sqc2	vf31,0x00(%0)
    " : : "r"(v) ) ;
}

/*----------------------------------------------------------------------*/

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

/* 計算用パラメータをＶｕ０にセット */
static	void	SetCheckSegParamator( from, sphere, r_sphere )
FVECTOR		*from ;
int		sphere, r_sphere ;
{
    u_int	*tag ;
    float	*ftag ;
    
    DmaTag = ( sceDmaTag * )Vu0Tag ;
	*TAGSIZE = 4 ;
    tag = ( u_int * )DmaTag ;
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 ) ;
    tag[ 2 ] = SCE_VIF0_SET_STCYCL( 1, 1, 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( VU0_PARAM_PTR, 2, VIF_DATA128, 0 ) ;
    tag += 4 ;
    /* 検索座標 */
    ftag = ( float * )tag ;
    ftag[ 0 ] = from->vx ;
    ftag[ 1 ] = from->vy ;
    ftag[ 2 ] = from->vz ;
    ftag[ 3 ] = 0.0F ;
    tag += 4 ;
    /* 検索半径 */
    ftag = ( float * )tag ;
    ftag[ 0 ] = ( float )sphere ;
    ftag[ 1 ] = ( float )r_sphere ;
    tag += 4 ;
    /* マイクロプログラムロード */
    *( u_long128 * )tag = *( u_long128 * )Vu0CheckNearHazardX ;
    tag += 4 ;
    DmaTag = ( sceDmaTag * )tag ;
}	

/* １ストリップのタグを生成 */
static	void	MakeOneSegmentStripTag( b, seg )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
{
    u_int	*tag ;
    int		n ;

    tag = ( u_int * )DmaTag ;
    n = seg->size ;
    DataPtr = DP_ADDR[ DP_WHICH ] ;
    DP_WHICH = 1 - DP_WHICH ;

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
		tag += 4 ;    	
		tag[ 0 ] = b->tx ;
		tag[ 1 ] = b->ty ;
		tag[ 2 ] = b->tz ;
		tag[ 3 ] = 0 ;
		tag += 4 ;
    }

    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, 1 ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->tag ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; 
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr, 1, VIF_DATA128, 0 ) ;
    tag += 4 ;    

    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, n ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->verts ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x01, 0 ) ; /* 加算展開書き込み */
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr + 1, n * 2, 0x0d, 0 ) ; /* V4-16bit */
    tag += 4 ;

    /* プログラム実行タグ */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckNearHazardFuncX[ 1 - First ], 0 ) ;
    tag += 4 ;

    DmaTag = ( sceDmaTag * )tag ;
}

/* １ブロックチェック */
static	void	CheckOneBlock( b, seg, n_segs, from, seg_flag, sphere, r_sphere )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		n_segs ;
FVECTOR		*from ;
int		seg_flag, sphere, r_sphere ;
{
    SVECTOR	b1, b2, svfrom ;

    First2 = 1 ;
    /* 自分のバウンディング作成 */	
    svfrom.vx = ( short )( from->vx - ( float )b->tx ) ;
    svfrom.vy = ( short )( from->vy - ( float )b->ty ) ;
    svfrom.vz = ( short )( from->vz - ( float )b->tz ) ;
    b1.vx = svfrom.vx - sphere ;
    b1.vy = svfrom.vy - sphere ;
    b1.vz = svfrom.vz - sphere ;
    b2.vx = svfrom.vx + sphere ;
    b2.vy = svfrom.vy + sphere ;
    b2.vz = svfrom.vz + sphere ;
    while( -- n_segs >= 0 ) {
		/* フラグチェック */
		if ( seg->atr & seg_flag ) {
#ifdef DEBUG_MODE
			if ( HZX_NearDebug == 1 ) HZX_ViewVuSegRGB( b, seg, 32, 232, 32 ) ;
#endif
			goto near_check_skip_strip ;
		}
		/* バウンディングチェック */
		if ( seg->b1.vx > b2.vx || seg->b2.vx < b1.vx ||
			seg->b1.vy > b2.vy || seg->b2.vy < b1.vy ||
			seg->b1.vz > b2.vz || seg->b2.vz < b1.vz ) goto near_check_skip_strip ;
#ifdef DEBUG_MODE
		if ( HZX_NearDebug == 1 ) HZX_ViewVuSeg( b, seg ) ;
#endif
		if ( First ) SetCheckSegParamator( from, sphere, r_sphere ) ;
		MakeOneSegmentStripTag( b, seg ) ;
		First = 0 ;
		First2 = 0 ;
		near_check_skip_strip :
		seg ++ ;
    }
}

#if 0
static	void	ViewOneBlock( b, seg, n_segs, from, seg_flag, sphere, r_sphere )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		n_segs ;
FVECTOR		*from ;
int		seg_flag, sphere, r_sphere ;
{
    while( -- n_segs >= 0 ) {
		HZX_ViewVuSeg( b, seg ) ;
		seg ++ ;
	}
}
#endif

/* 動的壁のチェック */

static	void	MakeDynamicSegmentTag( seg )
HZX_D_SEGMENT	*seg ;
{
    u_int	*tag ;

    tag = ( u_int * )DmaTag ;
    DataPtr = DP_ADDR[ DP_WHICH ] ;
    DP_WHICH = 1 - DP_WHICH ;

	if ( *TAGSIZE + 2 >= MAX_TAG_SIZE - 4 ) {
		/* タグサイズオーバー 
		   ここまでで一回実行する。 */
		tag = ExecHalfway( tag ) ;
	}
	*TAGSIZE = *TAGSIZE + 2 ;

    /* タグと頂点をまとめて転送 */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, 3 ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->tag ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; 
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr, 3, VIF_DATA128, 0 ) ;
    tag += 4 ;    
    /* プログラム実行タグ */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckNearHazardFuncX[ 1 - First ], 0 ) ;
    tag += 4 ;

    DmaTag = ( sceDmaTag * )tag ;
}

static	void	CheckDynamicSegment( segs, from, seg_flag, sphere, r_sphere )
HZX_D_SEGMENT		*segs ;
FVECTOR			*from ;
int			seg_flag, sphere, r_sphere ;
{
    HZX_D_SEGMENT	*seg ;
    FVECTOR		b1, b2 ;
    int			all, all2 ;

    all = all2 = 0 ;
    /* 自分のバウンディング作成 */	
    b1.vx = from->vx - ( float )sphere ;
    b1.vy = from->vy - ( float )sphere ;
    b1.vz = from->vz - ( float )sphere ;
    b2.vx = from->vx + ( float )sphere ;
    b2.vy = from->vy + ( float )sphere ;
    b2.vz = from->vz + ( float )sphere ;
    seg = segs ;

//	FlushCache( 0 ) ;

    while ( seg != NULL ) {
		all2 += 1 ;
		/* フラグチェック */
		if ( seg->atr & ( seg_flag | HZX_SEG_SKIP ) ) goto near_check_skip_strip_d ;
		/* バウンディングチェック */
		if ( ( float )seg->b1.vx > b2.vx || ( float )seg->b2.vx < b1.vx ||
			( float )seg->b1.vy > b2.vy || ( float )seg->b2.vy < b1.vy ||
			( float )seg->b1.vz > b2.vz || ( float )seg->b2.vz < b1.vz ) goto near_check_skip_strip_d ;
		all += 1 ;
		if ( First ) SetCheckSegParamator( from, sphere, r_sphere ) ;
#ifdef DEBUG_MODE
		if ( HZX_NearDebug == 1 ) HZX_ViewDynamicSegment( seg ) ;
#endif
		MakeDynamicSegmentTag( seg ) ;
//printf( "%d %d %d %d\n", seg->tag[ 0 ], seg->tag[ 1 ], seg->tag[ 2 ], seg->tag[ 3 ] ) ;
		First = 0 ;
		near_check_skip_strip_d :
        seg = seg->next ;
    }    
}

/* 結果の格納 */
static	inline	void	SetResult( hit )
int		hit ;
{
#ifndef NEW
    int		*tag ;
#else
    ALIGN16_PRE int ALIGN16_POST	tag[ 4 ] ;
#endif
    int		grp, blk, sgm, ptp ;
    HZX_VuSEG	*vsg ;
    HZX_D_SEGMENT	*dseg ;

    if ( hit == 0 ) return ;
#ifndef NEW
    tag = ( u_int * )( VU0_MEM_ADDR + VU0_FIRST_PTR * 16 ) ;
#else
    StoreFirst( tag ) ;
#endif
    ptp = tag[ 0 ] ; blk = tag[ 1 ] ; sgm = tag[ 2 ] ; grp = tag[ 3 ] ;
//printf( "dbg1 %d %d %d %d\n", tag[ 0 ], tag[ 1 ], tag[ 2 ], tag[ 3 ] ) ;
    HZX_GRP_PTR[ 0 ] = HZX_PTR->def->groups + grp ;
    if ( blk >= 0 ) {
		HZX_BLOCK_PTR[ 0 ] = HZX_GRP_PTR[ 0 ]->blocks + blk ;
		if ( sgm >= HZX_RECOIL_TYPE_SHIFT ) {
			vsg = HZX_BLOCK_PTR[ 0 ]->bul_segs + sgm - HZX_RECOIL_TYPE_SHIFT ;
		} else {
			vsg = HZX_BLOCK_PTR[ 0 ]->segs + sgm ;
		}
		HZX_ATR_PTR[ 0 ] = vsg->atr ;
		HZX_SEGS_PTR[ 0 ] = vsg->verts + ( vsg->b1.pad - ptp ) ;
    } else {
		//printf( "dynamic0 %d : %d\n", grp, sgm ) ;
        dseg = HZX_GRP_PTR[ 0 ]->dynamics->segs ;
        sgm = HZX_GRP_PTR[ 0 ]->dynamics->n_segs - sgm - 1 ;
        while( -- sgm >= 0 ) dseg = dseg->next ;
        DYNAMIC[ 0 ] = dseg ;
        HZX_ATR_PTR[ 0 ] = dseg->atr ;
    }
    if ( hit == 1 ) return ;
#ifndef NEW
    tag = ( u_int * )( VU0_MEM_ADDR + VU0_SECOND_PTR * 16 ) ;    
#else
    StoreSecond( tag ) ;
#endif
    ptp = tag[ 0 ] ; blk = tag[ 1 ] ; sgm = tag[ 2 ] ; grp = tag[ 3 ] ;
//printf( "dbg2 %d %d %d %d\n", tag[ 0 ], tag[ 1 ], tag[ 2 ], tag[ 3 ] ) ;
    HZX_GRP_PTR[ 1 ] = HZX_PTR->def->groups + grp ;
    if ( blk >= 0 ) {
		HZX_BLOCK_PTR[ 1 ] = HZX_GRP_PTR[ 1 ]->blocks + blk ;
		if ( sgm >= HZX_RECOIL_TYPE_SHIFT ) {
			vsg = HZX_BLOCK_PTR[ 1 ]->bul_segs + sgm - HZX_RECOIL_TYPE_SHIFT ;
		} else {
			vsg = HZX_BLOCK_PTR[ 1 ]->segs + sgm ;
		}
		HZX_ATR_PTR[ 1 ] = vsg->atr ;
		HZX_SEGS_PTR[ 1 ] = vsg->verts + ( vsg->b1.pad - ptp ) ;
    } else {
		//printf( "dynamic1 %d : %d\n", grp, sgm ) ;
        dseg = HZX_GRP_PTR[ 1 ]->dynamics->segs ;
        sgm = HZX_GRP_PTR[ 1 ]->dynamics->n_segs - sgm - 1 ;
        while( -- sgm >= 0 ) dseg = dseg->next ;
		DYNAMIC[ 1 ] = dseg ;
        HZX_ATR_PTR[ 1 ] = dseg->atr ;
    }
}

/* チェック実行 */
static	int	ExecNearCheck( void )
{
    u_int	*tag ;
    int		hit ;

    tag = ( u_int * )DmaTag ;
#if 1
	/* 反発計算する  */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckNearHazardFuncX[ 2 ], 0 ) ;

    tag[ 4 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 6 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; /* 加算展開書き込みＯＦＦ */
    tag[ 7 ] = SCE_VIF0_SET_FLUSHE( 0 ) ;

    tag[ 8 ] = DMATAG_SET_QWC( DMATAG_ID_END, 0 ) ;
    tag[ 10 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 11 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag += 12 ;    
#else
	/* 反発計算はやらない （control.cでやる）*/
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; /* 加算展開書き込みＯＦＦ */
    tag[ 3 ] = SCE_VIF0_SET_FLUSHE( 0 ) ;

    tag[ 4 ] = DMATAG_SET_QWC( DMATAG_ID_END, 0 ) ;
    tag[ 6 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 7 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag += 8 ;    
#endif
	HZXD( if ( GV_PadData->status & PAD_L1 ) FlushCache( 0 ) ; ) ;
	sceDmaSend( HZX_Vif0Dma, ( sceDmaTag * )( ( u_int )Vu0Tag | 0x80000000 ) ) ;

    /* ＳＥＮＤ終了待ちの間に */
    HZX_GRP_PTR[ 0 ] = HZX_GRP_PTR[ 1 ] = NULL ;
    HZX_BLOCK_PTR[ 0 ] = HZX_BLOCK_PTR[ 1 ] = NULL ;
    HZX_SEGS_PTR[ 0 ] = HZX_SEGS_PTR[ 1 ] = NULL ;
    DYNAMIC[ 0 ] = DYNAMIC[ 1 ] = NULL ;

    /* DMA Send 終了待ち */
    while ( sceDmaSync( HZX_Vif0Dma, 0, 0 ) ) ;    
    /* マイクロプログラム終了待ち */
    asm( "qmfc2.i    $0, vf01" ) ;
    while( *VIF0_STAT & VIF0_STAT_VEW_M ) ;  

    /* タグ開放 */
    DmaTag = ( sceDmaTag * )Vu0Tag ;
	HZXD( if ( GV_PadData->status & PAD_L1 ) FlushCache( 0 ) ; )
#ifdef NEW
	hit = StoreHit() ;
#else
    hit = *( u_int * )( VU0_MEM_ADDR + VU0_HIT_PTR * 16 ) ;
	if ( hit > 2 ) hit = 2 ;
#endif
//    DebugPrint() ;
    SetResult( hit ) ;
    return hit ;
}

/*----------------------------------------------------------------------*/

/* ブロックチェック */
static	inline	void	CheckBlockBound( grp, from, sphere, mode )
HZX_GRP		*grp ;
FVECTOR		*from ;
int		sphere ;
int		mode ;
{
    int		i, j, k, x1, x2, y1, y2, z1, z2, block ;
    int		min, size, dx, dy, dz ;
    HZX_BLOCK	*blk ;

    blk = grp->blocks ;
    min = grp->bound_min_x ; size = grp->block_size_x ;
    x1 = ( ( int )from->vx - sphere - min ) / size ;
    x2 = ( ( int )from->vx + sphere - min ) / size ;
    min = grp->bound_min_z ; size = grp->block_size_z ;
    z1 = ( ( int )from->vz - sphere - min ) / size ;
    z2 = ( ( int )from->vz + sphere - min ) / size ;
    min = grp->bound_min_y ; size = grp->block_size_y ;
    if ( mode == 0 ) {
		y1 = ( ( int )from->vy - min ) / size ;
		y2 = y1 ;
    } else {
		y1 = ( ( int )from->vy - sphere - min ) / size ;
		y2 = ( ( int )from->vy + sphere - min ) / size ;
    }
    N_InsideBlocks = 0 ;
    dx = grp->div_x ; dz = grp->div_z ; dy = grp->div_y ;

	if ( x1 < 0 ) x1 = 0 ; 
	if ( x1 >= dx ) x1 = dx - 1 ;
	if ( x2 < 0 ) x2 = 0 ; 
	if ( x2 >= dx ) x2 = dx - 1 ;
	if ( y1 < 0 ) y1 = 0 ; 
	if ( y1 >= dy ) y1 = dy - 1 ;
	if ( y2 < 0 ) y2 = 0 ; 
	if ( y2 >= dy ) y2 = dy - 1 ;
	if ( z1 < 0 ) z1 = 0 ; 
	if ( z1 >= dz ) z1 = dz - 1 ;
	if ( z2 < 0 ) z2 = 0 ; 
	if ( z2 >= dz ) z2 = dz - 1 ;

    for ( k = y1; k <= y2; k ++ ) {
		for ( j = z1; j <= z2; j ++ ) {
			for ( i = x1; i <= x2; i ++ ) {
//				if ( i < 0 || j < 0 || i >= dx || j >= dz || k < 0 || k >= dy ) continue ;
				block = grp->div_x * j + i + ( k * grp->div_x * grp->div_z ) ;
				if ( block >= 0 && block < grp->n_blocks ) {
#ifdef DEBUG
					if ( N_InsideBlocks == 256 ) {
						printf( "too many inside blocks\n" ) ;
						break ;
					}
#endif
					if ( N_InsideBlocks && 
						( block <= InsideBlock[ N_InsideBlocks - 1 ] ) ) continue ;
					if ( ( blk + block )->n_segs == 0 && 
						 ( blk + block )->n_bul_segs == 0 ) continue ;
					InsideBlock[ N_InsideBlocks ] = block ;
					N_InsideBlocks ++ ;
				}
			}
		}
		if ( mode == 0 ) break ;
    }
}

/*----------------------------------------------------------------------*/

/* ニアハザードチェック */
int	HZX_NearHazardCheck( HZX_GROUP_ID id, FVECTOR *from, int sphere
						, int chk_flag, int seg_flag, int r_sphere )
{
    HZX_HDL	*hzd ;
    int		hit, i, group, recoil_check, only_recoil ;
    HZX_GRP	*grp ;
    HZX_BLOCK	*blk, *b ;

    hit = 0 ;
    hzd = HZX_GetCurrentHzx() ;
    HZX_PTR = hzd ;
    First = First2 = 1 ;
    DataPtr = VU0_DATA_PTR ;
    DP_WHICH = 0 ;

	if ( chk_flag & HZX_CHK_RECOIL_TYPE_ONLY ) {
		only_recoil = recoil_check = 1 ;
	} else {
		only_recoil = 0 ;
		recoil_check 
			= ( ( seg_flag & HZX_SEG_RECOIL_TYPE ) || ( chk_flag & HZX_CHK_RECOIL_TYPE ) ) ? 1 : 0 ;
	}

	/* サイファー専用チェック */
	if ( chk_flag & HZX_CHK_CYPHER ) {
		seg_flag &= ~HZX_SEG_CYPHER ;
	} else {
		seg_flag |= HZX_SEG_CYPHER ;
	}
	/* 崖あたりチェック */
	if ( chk_flag & HZX_CHK_CLIFF ) {
		seg_flag &= ~HZX_SEG_CLIFF ;
	} else {
		seg_flag |= HZX_SEG_CLIFF ;
	}

	id = HZX_AddGroupID( id ) ;

    while( id != 0 ){
		group = GV_GetNo( id );
		id &= ~GV_GetBit( group );

		//ASSERT( group >= 0 && group < hzd->def->n_groups ) ;
		if ( group < 0 || group >= hzd->def->n_groups ) {
			continue ;
		}
		grp = hzd->def->groups + group ;
		if ( grp->n_blocks == 0 ) {
			continue ;
		}
	
		blk = grp->blocks ;
		/* 固定壁チェック */
		if ( chk_flag & HZX_CHK_F_SEGMENT ) {
			/* ブロックチェック */
			CheckBlockBound( grp, from, sphere, 0 ) ;
			for ( i = 0; i < N_InsideBlocks; i ++ ) {
				b = blk + InsideBlock[ i ] ;
				if ( !only_recoil && b->n_segs > 0 ) 
					CheckOneBlock( b, b->segs, b->n_segs, from, seg_flag, sphere, r_sphere ) ;
				if ( recoil_check && b->n_bul_segs > 0 ) 
					CheckOneBlock( b, b->bul_segs, b->n_bul_segs, from, seg_flag, sphere, r_sphere ) ;
#if 0
#ifdef DEBUG_MODE
				if ( HZX_NearDebug == 2 ) {
					ViewOneBlock( b, b->segs, b->n_segs, from, seg_flag, sphere, r_sphere ) ;
				}
#endif				
#endif
			}
		}
		/* 動的壁チェック */
		if ( ( chk_flag & HZX_CHK_D_SEGMENT ) &&
			( grp->dynamics->n_segs > 0 ) ) {
			CheckDynamicSegment( grp->dynamics->segs, from, 
								seg_flag, sphere, r_sphere ) ;
		}
    }
    if ( First == 0 ) hit = ExecNearCheck() ;
    *HIT = hit ;
    return hit ;
}

/*----------------------------------------------------------------------*/

static	int	SetSegment( seg, b, v )
HZX_SEG		*seg ;
HZX_BLOCK	*b ;
SVECTOR		*v ;
{
    if ( b == NULL || v == NULL ) return -1 ;
    seg->p1.x = v->vx + b->tx ;
    seg->p1.y = v->vy + b->ty ;
    seg->p1.z = v->vz + b->tz ;
    seg->p1.h = v->pad ;	v ++ ;
    seg->p2.x = v->vx + b->tx ;
    seg->p2.y = v->vy + b->ty ;
    seg->p2.z = v->vz + b->tz ;
    seg->p2.h = v->pad ;
    return 0 ;
}

static	void	SetSegmentD( seg, dseg )
HZX_SEG		*seg ;
HZX_D_SEGMENT	*dseg ;
{
    float		tmp ;

    GV_IVtoFV( &dseg->p1, &seg->p1, 4 ) ;
    GV_IVtoFV( &dseg->p2, &seg->p2, 4 ) ;
    tmp = seg->p1.y ; seg->p1.y = seg->p1.z ; seg->p1.z = tmp ;
    tmp = seg->p2.y ; seg->p2.y = seg->p2.z ; seg->p2.z = tmp ;
}

/* 結果を取得 */
void	HZX_GetNearHazard( HZX_SEG *segs, int *atrs ) 
{
    if ( DYNAMIC[ 0 ] == NULL ) {
		SetSegment( segs, HZX_BLOCK_PTR[ 0 ], HZX_SEGS_PTR[ 0 ] ) ;
    } else {
		SetSegmentD( segs, DYNAMIC[ 0 ] ) ;
		segs->ptr = DYNAMIC[ 0 ] ;
    }
	segs->attribute = HZX_ATR_PTR[ 0 ] ;
    if ( atrs != NULL ) atrs[ 0 ] = HZX_ATR_PTR[ 0 ] ;
	segs->type = HZX_TYPE_SEGMENT ;
	segs ++ ;
    if ( DYNAMIC[ 1 ] == NULL ) {
		SetSegment( segs, HZX_BLOCK_PTR[ 1 ], HZX_SEGS_PTR[ 1 ] ) ;
    } else {
		SetSegmentD( segs, DYNAMIC[ 1 ] ) ;
		segs->ptr = DYNAMIC[ 1 ] ;
    }
	segs->attribute = HZX_ATR_PTR[ 1 ] ;
	segs->type = HZX_TYPE_SEGMENT ;
    if ( atrs != NULL ) atrs[ 1 ] = HZX_ATR_PTR[ 1 ] ;
}

/* 結果のポインタを取得 */
void		HZX_GetNearHazardPtr( SVECTOR **segs, HZX_BLOCK **blk )
{
    segs[ 0 ] = HZX_SEGS_PTR[ 0 ] ;
    segs[ 1 ] = HZX_SEGS_PTR[ 1 ] ;
    blk[ 0 ] = HZX_BLOCK_PTR[ 0 ] ;
    blk[ 1 ] = HZX_BLOCK_PTR[ 1 ] ;
}

/* 結果のポインタからＨＺＤ＿ＳＥＧを生成 */
void		HZX_MakeNearHazard( HZX_SEG *segs, SVECTOR **v, HZX_BLOCK **blk )
{
    SetSegment( segs, blk[ 0 ], v[ 0 ] ) ;
    SetSegment( segs + 1, blk[ 1 ], v[ 1 ] ) ;
}

/* 近接壁へのベクトルを取得 */
void	HZX_GetNearVector( FVECTOR *vect_ptr )
{
#ifdef NEW
    StoreFirstVec( &vect_ptr[ 0 ] ) ;
    StoreFirstVec( &vect_ptr[ 1 ] ) ;
#else
    vect_ptr[ 0 ] = *( FVECTOR * )( VU0_MEM_ADDR + VU0_FIRST_VEC_PTR * 16 ) ;
    vect_ptr[ 1 ] = *( FVECTOR * )( VU0_MEM_ADDR + VU0_SECOND_VEC_PTR * 16 ) ;
#endif
}

/* 近接壁への最近点がエッジであるかどうか */
void	HZX_GetIsEdge( signed char *ie )
{
#ifdef NEW
    ie[ 0 ] = StoreFirstIsEdge() ;
    ie[ 1 ] = StoreSecondIsEdge() ;
#else
    ie[ 0 ] = *( int * )( VU0_MEM_ADDR + VU0_FIRST_EDGE_PTR * 16 ) ;
    ie[ 1 ] = *( int * )( VU0_MEM_ADDR + VU0_SECOND_EDGE_PTR * 16 ) ;
#endif
}

/* 反発ベクトルを取得 */
void	HZX_GetReactVector( FVECTOR *react )
{
#ifdef NEW
    FVECTOR	v ;

    StoreReactVec( v ) ;
    react->vx = v.vx ;
    react->vz = v.vz ;
#else
    FVECTOR	*v ;

    v = ( FVECTOR * )( VU0_MEM_ADDR + VU0_REACT_PTR * 16 ) ;
    react->vx = v->vx ;
	react->vy = 0.0F ;
    react->vz = v->vz ;
#endif
	
//	if ( HZX_NearDebug && (GV_PadData->press&PAD_L1)) {
//		DumpVec( react ) ;
//	}
}

/* 結果のアトリビュートを取得 */
void	HZX_GetNearHazardAtr( int *atrs )
{
    atrs[ 0 ] = HZX_ATR_PTR[ 0 ] ;
    atrs[ 1 ] = HZX_ATR_PTR[ 1 ] ;
}

/* 以前のバージョン用 */
void	HZX_GetNearHazardF( HZX_SEG **segs )
{
    HZX_GetNearHazard( SEGS_PTR, ATR_PTR ) ;
    segs[ 0 ] = SEGS_PTR ;
    segs[ 1 ] = SEGS_PTR + 1 ;
}

/*----------------------------------------------------------------------*/

/*
   レーダー用
   */
int	*HZX_GetNearBlockID( HZX_GRP *grp, FVECTOR *from, int sphere, int *n_blocks )
{
    /* ブロック単位でのバウンディングチェック */
    N_InsideBlocks = 0 ;
    if ( grp->n_blocks > 0 ) CheckBlockBound( grp, from, sphere, 1 ) ;
    *n_blocks = N_InsideBlocks;
    return InsideBlock;
}

/*----------------------------------------------------------------------*/




