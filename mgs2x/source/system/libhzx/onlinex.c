/*
   onlinex.c
   オンラインチェック バージョンＸ
   
   1999/11/25
   $Id: onlinex.c,v 1.1.1.3 2002/11/19 11:42:49 Yoshizawa1 Exp $
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

/*------------------------------------------------------------*/

#ifdef DEBUG_MODE
int	HZX_OnlineDebugFlag = 0 ;
//int	OnlineCheckSegs ;
//int	OnlineCheckFlrs ;
#endif

extern	qword		Vu0CheckOnlineHazardX ;
extern	qword		Vu0CheckOnlineHazardFuncX ;

#define	ALL_VU

#define	MAX_INSIDE_BLOCKS	(64)

#define	VU0_MEM_ADDR	(0x11004000)
#define	VU0_PARAM_PTR	(0x30)
//#define	VU0_BOUND_PTR	(0x32)
#define	VU0_DATA_PTR	(0x35)
#define	VU0_DATA_PTR2	(0x95)

#define	VU0_BOUND_PTR	(0xF6)
#define	VU0_BOUND_PTR2	(0xF8)

#define	VU0_HIT_PTR	(0x00)
#define	VU0_PTP_PTR	(0x01)
#define	VU0_LEN_PTR	(0x02)
#define	VU0_SEG_PTR	(0x03)
#define	VU0_TYPE_PTR	(0x06)

static	u_int	HZX_DP_ADDR[ 2 ] = { VU0_DATA_PTR, VU0_DATA_PTR2 } ;
static	u_int	HZX_BD_ADDR[ 2 ] = { VU0_BOUND_PTR, VU0_BOUND_PTR2 } ;

static	int	HZX_BD_WHICH ;

extern	int	HZX_DP_WHICH ;


typedef	struct	{
    FVECTOR	from ;
    FVECTOR	to ;
    FVECTOR	cut ;
    FVECTOR	b1 ;
    FVECTOR	b2 ;
    FVECTOR	bb1 ;
    FVECTOR	bb2 ;
    FVECTOR	v1, v2, v3, v4 ;
    FVECTOR	normal ;
    FVECTOR	diff ;
    FVECTOR	p1p2 ;
    FVECTOR	fromp1 ;
    FVECTOR	cross ;
    FVECTOR	mincross ;
    FVECTOR	mv1, mv2, mv3, mv4 ;

    float	len ;
    float	minlen ;
    int		hit ;
    int		tx, ty, tz ;
    HZX_BLOCK	*cblk ;
    HZX_VuSEG	*cvsg ;
    SVECTOR	*cseg ;
    HZX_HDL	*hzd ;
    HZX_BLOCK	*blk ;
    HZX_VuSEG	*vsg ;
    SVECTOR	*seg ;
    int		atr ;
    int		group ;
    HZX_BLOCK	*blk2 ;
    HZX_VuSEG	*vsg2 ;
    SVECTOR	*seg2 ;
    int		atr2 ;
    int		group2 ;
    
    HZX_FLR	segment ;
    int		attribute ;
    int		which ;
    HZX_D_FLOOR	*dynamic ;
    HZX_D_FLOOR	*cdynamic ;
    sceDmaTag	*dmatag ;
    int		first ;
    int		first2 ;
    int		checkflrvu ;
    int		dataptr ;
    int		*predataptr ;
    int		n_groups ;
    int		insideblockcurrent ;
	int		its_a_new_group ;

    int		Group[ HZX_MAX_GROUPS ] ;
    int		n_insideblocks[ HZX_MAX_GROUPS ] ;
    int		insideblock[ 64 ] ;
    FVECTOR	froms[ 64 ] ;
    FVECTOR	tos[ 64 ] ;

    int		tagsize ;
    int		size ;
    u_long128	tag[ 780 ] ;
} ScrPad ;

#define	MAX_TAG_SIZE	(760)

#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)

#define	Vu0Tag		(SCRPAD->tag)
#define	DmaTag		(SCRPAD->dmatag)
#define	First		(SCRPAD->first)
#define	First2		(SCRPAD->first2)
#define	CheckFlrVU	(SCRPAD->checkflrvu)
#define	DataPtr		(SCRPAD->dataptr)
#define	PreDataPtr	(SCRPAD->predataptr)
#define	N_Groups	(SCRPAD->n_groups)
#define	Group		(SCRPAD->Group)
#define	N_InsideBlocks	(SCRPAD->n_insideblocks)
#define	InsideBlock	(SCRPAD->insideblock)
#define	InsideBlockCurrent	(SCRPAD->insideblockcurrent)
#define	NEW_GROUP	(SCRPAD->its_a_new_group)

#define	FROM		(&(SCRPAD->from))
#define	TO		(&(SCRPAD->to))
#define	CUT		(&(SCRPAD->cut))
#define	BMIN		(&(SCRPAD->b1))
#define	BMAX		(&(SCRPAD->b2))
#define	BBMIN		(&(SCRPAD->bb1))
#define	BBMAX		(&(SCRPAD->bb2))

#define	HIT		(&(SCRPAD->hit))
#define	V1		(&(SCRPAD->v1))
#define	V2		(&(SCRPAD->v2))
#define	V3		(&(SCRPAD->v3))
#define	V4		(&(SCRPAD->v4))
#define	NORMAL		(&(SCRPAD->normal))
#define	DIFF		(&(SCRPAD->diff))
#define	FROM_P1		(&(SCRPAD->fromp1))
#define	LEN		(&(SCRPAD->len))
#define	MINLEN		(&(SCRPAD->minlen))
#define	CROSS		(&(SCRPAD->cross))
#define	MINCROSS	(&(SCRPAD->mincross))

#define	TX		(SCRPAD->tx)
#define	TY		(SCRPAD->ty)
#define	TZ		(SCRPAD->tz)
#define	GRPNO		(&(SCRPAD->group_no))
#define	BLKNO		(&(SCRPAD->block_no))
#define	STRIPNO		(&(SCRPAD->strip_no))

#define	MV1		(&(SCRPAD->mv1))
#define	MV2		(&(SCRPAD->mv2))
#define	MV3		(&(SCRPAD->mv3))
#define	MV4		(&(SCRPAD->mv4))

#define	C_BLOCK_PTR	(SCRPAD->cblk)
#define	C_VSG_PTR	(SCRPAD->cvsg)
#define	C_SEG_PTR	(SCRPAD->cseg)
#define	C_DYNAMIC	(SCRPAD->cdynamic)

#define	HZX_PTR		(SCRPAD->hzd)
#define	HZX_BLOCK_PTR	(SCRPAD->blk)
#define	HZX_VSG_PTR	(SCRPAD->vsg)
#define	HZX_SEG_PTR	(SCRPAD->seg)
#define	HZX_ATR_PTR	(&(SCRPAD->atr))
#define	HZX_GRP_PTR	(&(SCRPAD->group))

#define	HZX_BLOCK_PTR2	(SCRPAD->blk2)
#define	HZX_VSG_PTR2	(SCRPAD->vsg2)
#define	HZX_SEG_PTR2	(SCRPAD->seg2)
#define	HZX_ATR_PTR2	(&(SCRPAD->atr2))
#define	HZX_GRP_PTR2	(&(SCRPAD->group2))

#define	SEG_PTR		(&(SCRPAD->segment))
#define	ATR_PTR		(&(SCRPAD->attribute))
#define	WHICH		(&(SCRPAD->which))

#define	DYNAMIC		(SCRPAD->dynamic)

#define	TAGSIZE		(&(SCRPAD->tagsize))
#define	SIZE		(&(SCRPAD->size))

#define	FROMS		(SCRPAD->froms)
#define	TOS		(SCRPAD->tos)

/*------------------------------------------------------------*/

enum {
	MP_SegStart = 0,
	MP_SegNewGroup,
	MP_SegNewBlock,
	MP_SegNormal,
	MP_FlrStart,
	MP_FlrNewGroup,
	MP_FlrNewBlock,
	MP_FlrNormal
} ;

/*------------------------------------------------------------*/

//#define	DEBUG_PRINT	

#ifdef DEBUG_PRINT
/* デバッグ */
static	void	DebugPrint( void )
{
    int		*res ;
    float	*fres ;
    
    //    FlushCache( 0 ) ;
	if ( !HZX_OnlineDebugFlag ) return ;

    printf( "---- debug print ----\n" ) ;
#if 1
    fres = ( float * )( VU0_MEM_ADDR + 0x0a * 16 ) ;
    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
    fres = ( float * )( VU0_MEM_ADDR + 0x0b * 16 ) ;
    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
    fres = ( float * )( VU0_MEM_ADDR + 0x0c * 16 ) ;
    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
//    fres = ( float * )( VU0_MEM_ADDR + 0x0d * 16 ) ;
//    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
//    res = ( int * )( VU0_MEM_ADDR + 0x0d * 16 ) ;
//    printf( "%d %d %d %d\n", res[ 0 ], res[ 1 ], res[ 2 ], res[ 3 ] ) ;
//    fres = ( float * )( VU0_MEM_ADDR + 0x0e * 16 ) ;
//    printf( "%f %f %f %f\n", fres[ 0 ], fres[ 1 ], fres[ 2 ], fres[ 3 ] ) ;
#endif
    printf( "---- debug print end ----\n" ) ;
}
#endif

/*------------------------------------------------------------*/

static	inline	void	MakeBound( FVECTOR *b1, FVECTOR *b2,
								  FVECTOR *v1, FVECTOR *v2 ) {
    asm __volatile__("
	lqc2    	vf4,0x0(%2)
	lqc2    	vf5,0x0(%3)
    vmini.xyzw	vf6,vf4,vf5
    vmax.xyzw	vf7,vf4,vf5
	sqc2    	vf6,0x0(%0)
	sqc2    	vf7,0x0(%1)
	": : "r" (b1), "r" (b2), "r" (v1), "r" (v2) : "memory" ) ;
}

static	inline	void 	CopyVector( FVECTOR *v0, FVECTOR *v1 )
{
    asm __volatile__("
	lq    $6,0x0(%1)
	sq    $6,0x0(%0)
	": : "r" (v0) , "r" (v1):"$6", "memory" );
}

static	inline	void 	ScaleVector( FVECTOR *v0, FVECTOR *v1, float t )
{
	asm __volatile__("
	lqc2    vf4,0x0(%1)
    mfc1    $8,%2
    qmtc2    $8,vf5
	vmulx.xyzw	vf6,vf4,vf5
	sqc2    vf6,0x0(%0)
	": : "r" (v0) , "r" (v1), "f" (t):"$8", "memory" );
}

static	inline	void	SwapVector( FVECTOR *v1, FVECTOR *v2 ) 
{
    asm __volatile__("
	lq    $6,0x0(%0)
	lq    $7,0x0(%1)
	sq    $6,0x0(%1)
	sq    $7,0x0(%0)
	": : "r" (v1) , "r" (v2): "$6","$7", "memory" );
}

static	inline	void	ScalingAddVector( FVECTOR *v1, FVECTOR *v2, 
										 FVECTOR *v3, float t )
{
    asm __volatile__("
	lqc2    vf7,0x0(%1)
	lqc2    vf4,0x0(%2)
    mfc1    $8,%3
    qmtc2    $8,vf5
	vmulx.xyzw	vf6,vf4,vf5
    vadd.xyzw	vf7,vf7,vf6
	sqc2    vf7,0x0(%0)
	": : "r" (v1) , "r" (v2), "r" (v3), "f" (t):"$8", "memory" );
}

/*------------------------------------------------------------*/

/* 直線を直方体で切る */
static	int	CutLinebyBlock( b1, b2, nf, nt )
FVECTOR		*b1, *b2 ;
FVECTOR		*nt, *nf ;
{
    FVECTOR	diff ;
    float	va, vb, vc ;
    int		which ;

    which = 1 ;
    CopyVector( nf, FROM ) ;
    CopyVector( nt, TO ) ;    
    CopyVector( &diff, DIFF ) ;
    va = DIFF->vx ;
    if ( va != 0.0F ) {
		if ( va < 0.0F ) {
			SwapVector( nf, nt ) ;
			ScaleVector( &diff, &diff, -1.0F ) ;
			which *= -1 ;
			va = -va ;
		}
		vb = ( b1->vx - nf->vx ) ;
		vc = ( b2->vx - nf->vx ) ;
		if ( vb > va || vc < 0.0F ) return 0 ;
		if ( vb < 0.0F ) vb = 0.0F ;
		if ( vc > va ) vc = va ;
		ScalingAddVector( nt, nf, &diff, vc / diff.vx ) ;
		ScalingAddVector( nf, nf, &diff, vb / diff.vx ) ;
		_sceVu0SubVector( &diff, nt, nf ) ;
    }
    va = nt->vy - nf->vy ;
    if ( va != 0.0F ) {
		if ( va < 0.0F ) {
			SwapVector( nf, nt ) ;
			ScaleVector( &diff, &diff, -1.0F ) ;
			which *= -1 ;
			va = -va ;
		}
		vb = ( b1->vy - nf->vy ) ;
		vc = ( b2->vy - nf->vy ) ;
		if ( vb > va || vc < 0.0F ) return 0 ;
		if ( vb < 0.0F ) vb = 0.0F ;
		if ( vc > va ) vc = va ;
		ScalingAddVector( nt, nf, &diff, vc / diff.vy ) ;
		ScalingAddVector( nf, nf, &diff, vb / diff.vy ) ;
		_sceVu0SubVector( &diff, nt, nf ) ;
    }
    va = nt->vz - nf->vz ;
    if ( va != 0.0F ) {
		if ( va < 0.0F ) {
			SwapVector( nf, nt ) ;
			ScaleVector( &diff, &diff, -1.0F ) ;
			which *= -1 ;
			va = -va ;
		}
		vb = ( b1->vz - nf->vz ) ;
		vc = ( b2->vz - nf->vz ) ;
		if ( vb > va || vc < 0.0F ) return 0 ;
		if ( vb < 0.0F ) vb = 0.0F ;
		if ( vc > va ) vc = va ;
		ScalingAddVector( nt, nf, &diff, vc / diff.vz ) ;
		ScalingAddVector( nf, nf, &diff, vb / diff.vz ) ;
    }
    if ( which == -1 ) SwapVector( nf, nt ) ;
    return 1 ;
}

/*------------------------------------------------------------*/

#if 0
/* ブロックチェック */
static	int	OnlineCheckBlock( from, to, sx, sy, sz, ccx, ccy, ccz ) 
FVECTOR		*from, *to ;
float		sx, sy, sz, ccx, ccy, ccz ;
{
    float	cx, cy, cz, df, f0, c1, c2 ;
    FVECTOR	diff ;
    
    cx = ccx ;
    cy = ccy ;
    cz = ccz ;
    
    _sceVu0SubVector( &diff, to, from ) ;
    
    /* Ｘ切断を試みる */
    f0 = from->vx ;
    if ( ( df = to->vx - f0 ) == 0.0F ) goto cutx_failed ;
    if ( cx < f0 ) {
		cx += sx ;
		if ( cx > f0 ) cx = f0 ;
    } else {
		cx -= sx ;
		if ( cx < f0 ) cx = f0 ;
    }
    df = ( cx - f0 ) / df ;
    c1 = from->vy + diff.vy * df ;
    if ( c1 < cy - sy || c1 > cy + sy ) {
		goto cutx_failed ;
    }
    c2 = from->vz + diff.vz * df ;
    if ( c2 < cz - sz || c2 > cz + sz ) {
		goto cutx_failed ;
    }
    CUT->vx = cx ;
    CUT->vy = c1 ;
    CUT->vz = c2 ;
    return 1 ;
	cutx_failed :
    /* Ｙ切断を試みる */
    cx = ccx ;
    cy = ccy ;
    cz = ccz ;
    f0 = from->vy ;
    if ( ( df = to->vy - f0 ) == 0.0F ) goto cuty_failed ;
    if ( cy < f0 ) {
		cy += sy ;
		if ( cy > f0 ) cy = f0 ;
    } else {
		cy -= sy ;
		if ( cy < f0 ) cy = f0 ;
    }
    df = ( cy - f0 ) / df ;
    c1 = from->vz + diff.vz * df ;
    if ( c1 < cz - sz || c1 > cz + sz ) {
		goto cuty_failed ;
    }
    c2 = from->vx + diff.vx * df ;
    if ( c2 < cx - sx || c2 > cx + sx ) {
		goto cuty_failed ;
    }
    CUT->vx = c2 ;
    CUT->vy = cy ;
    CUT->vz = c1 ;
    return 1 ;
	cuty_failed :
    /* Ｚ切断を試みる */
    cx = ccx ;
    cy = ccy ;
    cz = ccz ;
    f0 = from->vz ;
    if ( ( df = to->vz - f0 ) == 0.0F ) goto cutz_failed ;
    if ( cz < f0 ) {
		cz += sz ;
		if ( cz > f0 ) cz = f0 ;
    } else {
		cz -= sz ;
		if ( cz < f0 ) cz = f0 ;
    }
    df = ( cz - f0 ) / df ;
    c1 = from->vx + diff.vx * df ;
    if ( c1 < cx - sx || c1 > cx + sx ) {
		goto cutz_failed ;
    }
    c2 = from->vy + diff.vy * df ;
    if ( c2 < cy - sy || c2 > cy + sy ) {
		goto cutz_failed ;
    }
    CUT->vx = c1 ;
    CUT->vy = c2 ;
    CUT->vz = cz ;
    return 1 ;
	cutz_failed :
    return 0 ;    
}
#endif

/*------------------------------------------------------------*/

/* 途中実行 */
static	u_int	*ExecHalfway( tag )
u_int		*tag ;
{
//	printf( "tag size over exec\n" ) ;
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

/*------------------------------------------------------------*/

/* 初期設定用タグを生成 */
static	void	SetCheckParamator( void ) 
{
    u_int	*tag ;
    float	*ftag ;

    DmaTag = ( sceDmaTag * )Vu0Tag ;
    *TAGSIZE = 6 ;
    tag = ( u_int * )DmaTag ;
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 4 ) ;
    tag[ 2 ] = SCE_VIF0_SET_STCYCL( 1, 1, 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( VU0_PARAM_PTR, 4, VIF_DATA128, 0 ) ;
    tag += 4 ;
    /* 検索座標 */
    ftag = ( float * )tag ;
    ftag[ 0 ] = FROM->vx ;
    ftag[ 1 ] = FROM->vy ;
    ftag[ 2 ] = FROM->vz ;
	ftag[ 3 ] = -0.05F ;			/* 計算につかえるかも */
    ftag[ 4 ] = TO->vx ;
    ftag[ 5 ] = TO->vy ;
    ftag[ 6 ] = TO->vz ;
    ftag[ 7 ] = 0.05F ;			/* 計算につかえるかも */
    ftag[ 8 ] = BMIN->vx ;
    ftag[ 9 ] = BMIN->vy ;
    ftag[ 10 ] = BMIN->vz ;
    ftag[ 11 ] = 0.0F ;
    ftag[ 12 ] = BMAX->vx ;
    ftag[ 13 ] = BMAX->vy ;
    ftag[ 14 ] = BMAX->vz ;
    ftag[ 15 ] = 0.0F ;
    tag += 16 ;
    /* マイクロプログラムロード */
    *( u_long128 * )tag = *( u_long128 * )Vu0CheckOnlineHazardX ;
    tag += 4 ;
    DmaTag = ( sceDmaTag * )tag ;
}

/* １ストリップのタグを生成 */
static	void	MakeOneSegmentStripTag( b, seg, no )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		no ;
{
    u_int	*tag ;
	u_int	bd ;
    int		n, prg_no ;

    tag = ( u_int * )DmaTag ;
    n = seg->size ;
    DataPtr = HZX_DP_ADDR[ HZX_DP_WHICH ] ;

    HZX_DP_WHICH = 1 - HZX_DP_WHICH ;

    *SIZE = 3 ;
    if ( First2 ) *SIZE = *SIZE + 5 ;
    if ( *TAGSIZE + *SIZE >= MAX_TAG_SIZE ) {
		/* ここまでで一回実行してしまう */
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

		bd = HZX_BD_ADDR[ HZX_BD_WHICH ] ;
		HZX_BD_WHICH = 1 - HZX_BD_WHICH ;

		/* ブロックバウンディングロード */
		tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 ) ;
		tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ;
		tag[ 3 ] = SCE_VIF0_SET_UNPACK( bd, 2, VIF_DATA128, 0 ) ;
#if 1
		*( FVECTOR * )&tag[ 4 ] = *BBMIN ;
		*( FVECTOR * )&tag[ 8 ] = *BBMAX ;
#else
		{
			float	*ftag ;

			ftag = ( float * )&tag[ 4 ] ;
			ftag[ 0 ] = BBMIN->vx - 0.050F ;
			ftag[ 1 ] = BBMIN->vy - 0.050F ;
			ftag[ 2 ] = BBMIN->vz - 0.050F ;
			ftag[ 4 ] = BBMAX->vx + 0.050F ;
			ftag[ 5 ] = BBMAX->vy + 0.050F ;
			ftag[ 6 ] = BBMAX->vz + 0.050F ;
		}
#endif
		tag += 12 ;
    }

    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, 1 ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->tag ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr, 1, VIF_DATA128, 0 ) ;
    tag[ 4 ] = DMATAG_SET_QWC( DMATAG_ID_REF, n ) ;
    tag[ 5 ] = ( u_int )DMATAG_SET_ADDR( seg->verts ) ;
    tag[ 6 ] = SCE_VIF0_SET_STMOD( 0x01, 0 ) ; /* 加算展開書き込み */
    tag[ 7 ] = SCE_VIF0_SET_UNPACK( DataPtr + 1, n * 2, 0x0d, 0 ) ; /* V4-16bit */
    tag += 8 ;
    
    /* プログラム実行タグ */
	prg_no = 1 - First ;
	if ( prg_no && !NEW_GROUP ) {
		prg_no = ( First2 ) ? MP_SegNewBlock : MP_SegNormal ;
	}
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckOnlineHazardFuncX[ prg_no ], 0 ) ;
    tag += 4 ;

    DmaTag = ( sceDmaTag * )tag ;
}

/* １ブロックの壁をチェック */
static	void	CheckOneBlockSeg( b, seg, n_segs, seg_flag )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		n_segs ;
int		seg_flag ;
{
    int		i ;
    
    First2 = 1 ;
    for ( i = 0; i < n_segs; i ++ ) {
		/* フラグチェック */
		if ( seg->atr & seg_flag ) goto online_check_skip_strip ;
		/* バウンディングチェック */
		if ( ( float )( seg->b1.vx + b->tx ) > BBMAX->vx ||
			( float )( seg->b1.vy + b->ty ) > BBMAX->vy ||
			( float )( seg->b1.vz + b->tz ) > BBMAX->vz ||
			( float )( seg->b2.vx + b->tx ) < BBMIN->vx ||
			( float )( seg->b2.vy + b->ty ) < BBMIN->vy ||
			( float )( seg->b2.vz + b->tz ) < BBMIN->vz ) {
			goto online_check_skip_strip ;
		}
		if ( First ) SetCheckParamator() ;
		MakeOneSegmentStripTag( b, seg, i ) ;
#ifdef DEBUG_MODE
		//if ( HZX_OnlineDebugFlag == 2 ) HZX_ViewVuSeg( b, seg ) ;
#endif
//		OnlineCheckSegs += seg->b1.pad - 1 ;
		First = First2 = NEW_GROUP = 0 ;
		online_check_skip_strip :
        seg ++ ;
    }
}

/* 動的壁チェック */
static	void	MakeDynamicSegmentTag( seg )
HZX_D_SEGMENT	*seg ;
{
    u_int	*tag ;
    int		prg_no ;

    tag = ( u_int * )DmaTag ;
    DataPtr = HZX_DP_ADDR[ HZX_DP_WHICH ] ;
    HZX_DP_WHICH = 1 - HZX_DP_WHICH ;    

    *SIZE = 2 ;
    if ( *TAGSIZE + *SIZE >= MAX_TAG_SIZE ) {
		/* ここまでで一回実行してしまう */
		tag = ExecHalfway( tag ) ;
    }
    *TAGSIZE = *TAGSIZE + *SIZE ;
    
    /* タグと頂点を同時転送 */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, 3 ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->tag ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr, 3, VIF_DATA128, 0 ) ;
    tag += 4 ;
    
    /* プログラム実行タグ */
	prg_no = ( First ) ? MP_SegStart : MP_SegNormal ;
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckOnlineHazardFuncX[ prg_no ], 0 ) ;
    tag += 4 ;
    DmaTag = ( sceDmaTag * )tag ;
}

static	void	CheckDynamicSegment( seg, seg_flag )
HZX_D_SEGMENT	*seg ;
int		seg_flag ;
{
    while ( seg != NULL ) {
		/* フラグチェック */
		if ( seg->atr & ( seg_flag | HZX_SEG_SKIP ) ) goto online_check_skip_strip_d ;
		/* バウンディングチェック */
		if ( ( float )( seg->b1.vx ) > BMAX->vx ||
			( float )( seg->b1.vy ) > BMAX->vy ||
			( float )( seg->b1.vz ) > BMAX->vz ||
			( float )( seg->b2.vx ) < BMIN->vx ||
			( float )( seg->b2.vy ) < BMIN->vy ||
			( float )( seg->b2.vz ) < BMIN->vz ) {
			goto online_check_skip_strip_d ;
		}
		if ( First ) SetCheckParamator() ;
		MakeDynamicSegmentTag( seg ) ;
#ifdef DEBUG_MODE
		if ( HZX_OnlineDebugFlag == 2 ) HZX_ViewDynamicSegment( seg ) ;
#endif
		First = NEW_GROUP = 0 ;
online_check_skip_strip_d :
        seg = seg->next ;
    }
}

/* ＶＵ実行 */
static	void	ExecOnlineCheck( void )
{
    u_int	*tag ;
    int		hit ;
    
    hit = 0 ;
    tag = ( u_int * )DmaTag ;

#ifdef DEBUG_MODE
    {
		int		size ;
		size = ( ( u_int )DmaTag - ( u_int )Vu0Tag ) / sizeof( u_long128 ) ;
		if ( size >= MAX_TAG_SIZE ) {
			printf( "tag size over : %d %d\n", size, *TAGSIZE ) ;
		} 
    }
#endif

    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; /* 加算展開書き込みＯＦＦ */
    tag[ 3 ] = SCE_VIF0_SET_FLUSHE( 0 ) ;
    tag[ 4 ] = DMATAG_SET_QWC( DMATAG_ID_END, 0 ) ;
    tag[ 6 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 7 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag += 8 ;    
	HZXD( if ( GV_PadData->status & PAD_L1 ) FlushCache( 0 ) ; )
		/* Ｓｅｎｄして終了 */
		sceDmaSend( HZX_Vif0Dma, ( sceDmaTag * )( ( u_int )Vu0Tag | 0x80000000 ) ) ;
    HZX_BLOCK_PTR = NULL ;
    HZX_SEG_PTR = NULL ;
}

/*------------------------------------------------------------*/

/* １ストリップのタグを生成 */
static	void	MakeOneFloorStripTag( b, seg, no )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		no ;
{
    u_int	*tag ;
	u_int	bd ;
    int		n, prg_no ;
    
    tag = ( u_int * )DmaTag ;
    n = seg->size ;
    DataPtr = HZX_DP_ADDR[ HZX_DP_WHICH ] ;
    HZX_DP_WHICH = 1 - HZX_DP_WHICH ;

    *SIZE = 3 ;
    if ( First2 ) *SIZE = *SIZE + 5 ;
    if ( *TAGSIZE + *SIZE >= MAX_TAG_SIZE ) {
		/* ここまでで一回実行してしまう */
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

		bd = HZX_BD_ADDR[ HZX_BD_WHICH ] ;
		HZX_BD_WHICH = 1 - HZX_BD_WHICH ;

		/* ブロックバウンディングロード */
		tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 ) ;
		tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ;
		tag[ 3 ] = SCE_VIF0_SET_UNPACK( bd, 2, VIF_DATA128, 0 ) ;
#if 0
		*( FVECTOR * )&tag[ 4 ] = *BBMIN ;
		*( FVECTOR * )&tag[ 8 ] = *BBMAX ;
#else
		{
			float	*ftag ;

			ftag = ( float * )&tag[ 4 ] ;
			ftag[ 0 ] = BBMIN->vx - 0.050F ;
			ftag[ 1 ] = BBMIN->vy - 0.050F ;
			ftag[ 2 ] = BBMIN->vz - 0.050F ;
			ftag[ 4 ] = BBMAX->vx + 0.050F ;
			ftag[ 5 ] = BBMAX->vy + 0.050F ;
			ftag[ 6 ] = BBMAX->vz + 0.050F ;
		}
#endif
		tag += 12 ;
    }

    /* 床データ */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, 2 ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->tag ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr, 2, VIF_DATA128, 0 ) ;
    tag[ 4 ] = DMATAG_SET_QWC( DMATAG_ID_REF, n ) ;
    tag[ 5 ] = ( u_int )DMATAG_SET_ADDR( seg->verts ) ;
    tag[ 6 ] = SCE_VIF0_SET_STMOD( 0x01, 0 ) ; /* 加算展開書き込み */
    tag[ 7 ] = SCE_VIF0_SET_UNPACK( DataPtr + 2, n * 2, 0x0d, 0 ) ; /* V4-16bit */
    tag += 8 ;
    
    /* プログラム実行タグ */
	prg_no = MP_FlrNewGroup - First ;
	if ( prg_no == MP_FlrNewGroup && !NEW_GROUP ) {
		prg_no = ( First2 ) ? MP_FlrNewBlock : MP_FlrNormal ;
	}

    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckOnlineHazardFuncX[ prg_no ], 0 ) ;
    tag += 4 ;

    DmaTag = ( sceDmaTag * )tag ;
}

/* １ブロックの床をチェック */
static	void	CheckOneBlockFlr( b, seg, n_segs, flag )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		n_segs ;
int		flag ;
{
    int		i ;

    First2 = 1 ;
    for ( i = 0; i < n_segs; i ++ ) {
		/* フラグチェック */
		if ( seg->atr & flag ) goto online_check_flr_skip_strip ;
		/* バウンディングチェック */
		if ( ( float )( seg->b1.vx + b->tx ) > BBMAX->vx ||
			( float )( seg->b1.vy + b->ty ) > BBMAX->vy ||
			( float )( seg->b1.vz + b->tz ) > BBMAX->vz ||
			( float )( seg->b2.vx + b->tx ) < BBMIN->vx ||
			( float )( seg->b2.vy + b->ty ) < BBMIN->vy ||
			( float )( seg->b2.vz + b->tz ) < BBMIN->vz ) {
			goto online_check_flr_skip_strip ;
		}
#if 0
		/* この処理、した方が遅い */
		sx = ( float )( seg->b2.vx - seg->b1.vx ) / 2.0F ;
		sy = ( float )( seg->b2.vy - seg->b1.vy ) / 2.0F ;
		sz = ( float )( seg->b2.vz - seg->b1.vz ) / 2.0F ;
		cx = ( float )( seg->b1.vx + b->tx ) + sx ;
		cy = ( float )( seg->b1.vy + b->ty ) + sy ;
		cz = ( float )( seg->b1.vz + b->tz ) + sz ;
		if ( !OnlineCheckBlock( FROM, TO, sx, sy, sz, cx, cy, cz ) ) 
			goto online_check_flr_skip_strip ;
#endif
		if ( First ) SetCheckParamator() ;
		MakeOneFloorStripTag( b, seg, i ) ;
#ifdef DEBUG_MODE
		//if ( HZX_OnlineDebugFlag == 2 ) HZX_ViewVuFlr( b, seg ) ;
#endif
//		OnlineCheckFlrs += seg->b1.pad / seg->b2.pad ;
		First = First2 = NEW_GROUP = 0 ;
online_check_flr_skip_strip :
        seg ++ ;
    }
}

#ifdef DEBUG_MODE
#if 0
static	void	ViewBlockHazardR( HZX_BLOCK *blk )
{
    int			i, n_segs ;
    HZX_VuSEG	*seg ;
	
	n_segs = blk->n_bul_segs ;
	seg = blk->bul_segs ;
    for ( i = 0; i < n_segs; i ++ ) {
		HZX_ViewVuSeg( blk, seg ) ;
        seg ++ ;
    }	
#if 0
	n_segs = blk->n_bul_flrs ;
	seg = blk->bul_flrs ;
    for ( i = 0; i < n_segs; i ++ ) {
		HZX_ViewVuFlr( blk, seg ) ;
        seg ++ ;
    }
#endif	
}
#endif
#endif

/* 動的床チェック */
static	void	MakeDynamicFloorTag( seg )
HZX_D_FLOOR	*seg ;
{
    u_int	*tag ;
    int		n, prg_no ;
    
    tag = ( u_int * )DmaTag ;
    n = seg->tag[ 0 ] ;
    DataPtr = HZX_DP_ADDR[ HZX_DP_WHICH ] ;
    HZX_DP_WHICH = 1 - HZX_DP_WHICH ;

    *SIZE = 3 ;
    if ( *TAGSIZE + *SIZE >= MAX_TAG_SIZE ) {
		/* ここまでで一回実行してしまう */
		tag = ExecHalfway( tag ) ;
    }
    *TAGSIZE = *TAGSIZE + *SIZE ;
    /* タグ転送 */
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_REF, 2 ) ;
    tag[ 1 ] = ( u_int )DMATAG_SET_ADDR( seg->tag ) ;
    tag[ 2 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_UNPACK( DataPtr, 2, VIF_DATA128, 0 ) ;
    /* 頂点転送 */
    tag[ 4 ] = DMATAG_SET_QWC( DMATAG_ID_REF, n ) ;
    tag[ 5 ] = ( u_int )DMATAG_SET_ADDR( &seg->p1 ) ;
    tag[ 6 ] = SCE_VIF0_SET_STMOD( 0x00, 0 ) ; /* 加算展開書き込み */
    tag[ 7 ] = SCE_VIF0_SET_UNPACK( DataPtr + 2, n, VIF_DATA128, 0 ) ; 
    tag += 8 ;
    /* プログラム実行タグ */
	prg_no = ( First ) ? MP_FlrStart : MP_FlrNormal ;
    tag[ 0 ] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
    tag[ 2 ] = SCE_VIF0_SET_NOP( 0 ) ;
    tag[ 3 ] = SCE_VIF0_SET_MSCAL( Vu0CheckOnlineHazardFuncX[ prg_no ], 0 ) ;
    tag += 4 ;
    DmaTag = ( sceDmaTag * )tag ;
}

static	void	CheckDynamicFloor( seg, flag )
HZX_D_FLOOR	*seg ;
int		flag ;
{
    while( seg != NULL ) {
		/* フラグチェック */
		if ( seg->atr & ( flag | HZX_FLOOR_SKIP ) ) goto online_check_flr_skip_strip_d ;
		/* バウンディングチェック */
		if ( ( float )seg->b1.vx > BMAX->vx ||
			( float )seg->b1.vy > BMAX->vy ||
			( float )seg->b1.vz > BMAX->vz ||
			( float )seg->b2.vx < BMIN->vx ||
			( float )seg->b2.vy < BMIN->vy ||
			( float )seg->b2.vz < BMIN->vz ) {
			goto online_check_flr_skip_strip_d ;
		}
		if ( First ) SetCheckParamator() ;
#ifdef DEBUG_MODE
		if ( HZX_OnlineDebugFlag == 2 ) HZX_ViewDynamicFloor( seg, seg->tag[ 0 ] ) ;
#endif
		MakeDynamicFloorTag( seg ) ;
		First = NEW_GROUP = 0 ;
		online_check_flr_skip_strip_d :
        seg = seg->next ;
    }
}

/*------------------------------------------------------------*/

#ifndef ALL_VU
/* 床チェックＣＰＵ */

static	inline	void	SVtoFV_Trans( sv, fv )
SVECTOR		*sv ;
FVECTOR		*fv ;
{
    fv->vx = ( float )sv->vx + ( float )TX ;
    fv->vy = ( float )sv->vy + ( float )TY ;
    fv->vz = ( float )sv->vz + ( float )TZ ;
    fv->vw = ( float )sv->pad ;
}

static	inline	void	IVtoFV( iv, fv )
IVECTOR		*iv ;
FVECTOR		*fv ;
{
    fv->vx = iv->vx ;
    fv->vy = iv->vy ;
    fv->vz = iv->vz ;
    fv->vw = iv->vw ;
}

/* 交点までの距離を計算 */
static	int	FloorDistance( type )
int		type ;
{
    float	depth, x, y, z ;
    float	fa_n, ft_n, tmp ;
    
    if ( type & HZX_FLOOR_FLAT ) { /* FLAT_FLOOR */
		if ( FROM->vy == TO->vy ) return -1 ;
		depth = ( V1->vy - FROM->vy ) / ( TO->vy - FROM->vy ) ;
		if ( depth < 0.0F || depth > 1.0F ) return -1 ;
		CROSS->vx = FROM->vx + ( TO->vx - FROM->vx ) * depth ;
		CROSS->vz = FROM->vz + ( TO->vz - FROM->vz ) * depth ;
		CROSS->vy = V1->vy ;
    } else {					/* SLOPE_FLOOR */
		FROM_P1->vx = V1->vx - FROM->vx ;
		FROM_P1->vy = V1->vy - FROM->vy ;
		FROM_P1->vz = V1->vz - FROM->vz ;
		/* 法線 */
		NORMAL->vx = V1->vw ;
		NORMAL->vy = V2->vw ;
		NORMAL->vz = V3->vw ;
		ft_n = DIFF->vx * NORMAL->vx + DIFF->vy * NORMAL->vy + DIFF->vz * NORMAL->vz ;
		fa_n = FROM_P1->vx * NORMAL->vx + FROM_P1->vy * NORMAL->vy + FROM_P1->vz * NORMAL->vz ;
		tmp = fa_n * ft_n ;
		if ( tmp <= 0.0F ) return -1 ;
		fa_n = fa_n / ft_n ;
		if ( fa_n <= 0.0F ) return -1 ;
		CROSS->vx = FROM->vx + DIFF->vx * fa_n ;
		CROSS->vy = FROM->vy + DIFF->vy * fa_n ;
		CROSS->vz = FROM->vz + DIFF->vz * fa_n ;
    }
    x = CROSS->vx - FROM->vx ; if ( x < 0.0F ) x = -x ; 
    y = CROSS->vy - FROM->vy ; if ( y < 0.0F ) y = -y ;
    z = CROSS->vz - FROM->vz ; if ( z < 0.0F ) z = -z ;
    *LEN = x + y + z ;
    return 1 ;
}

/* 交点が床に含まれるかチェック */
static	int	CheckInsideFloor( type, n_v )
int		type, n_v ;
{
    float	opz ;
    FVECTOR	p1, p2, min, max ;
    
    if ( type & HZX_FLOOR_RECT ) {
		if ( V1->vx < V3->vx ) {
			min.vx = V1->vx ; max.vx = V3->vx ;
		} else {
			min.vx = V3->vx ; max.vx = V1->vx ;
		}
		if ( V1->vz < V3->vz ) {
			min.vz = V1->vz ; max.vz = V3->vz ;
		} else {
			min.vz = V3->vz ; max.vz = V1->vz ;
		}
		if ( CROSS->vx < min.vx || CROSS->vx > max.vx ||
			CROSS->vz < min.vz || CROSS->vz > max.vz ) return -1 ;
		return 1 ;
    }
    
    p1.vx = CROSS->vx - V1->vx ;
    p1.vz = CROSS->vz - V1->vz ;
    p2.vx = V2->vx - V1->vx ;
    p2.vz = V2->vz - V1->vz ;
    opz = p1.vz * p2.vx - p1.vx * p2.vz ;
    if ( opz < 0.0F ) return -1 ;
    p1.vx = CROSS->vx - V2->vx ;
    p1.vz = CROSS->vz - V2->vz ;
    p2.vx = V3->vx - V2->vx ;
    p2.vz = V3->vz - V2->vz ;
    opz = p1.vz * p2.vx - p1.vx * p2.vz ;
    if ( opz < 0.0F ) return -1 ;
    if ( n_v == 3 ) {
		p1.vx = CROSS->vx - V3->vx ;
		p1.vz = CROSS->vz - V3->vz ;
		p2.vx = V1->vx - V3->vx ;
		p2.vz = V1->vz - V3->vz ;
		opz = p1.vz * p2.vx - p1.vx * p2.vz ;
		if ( opz < 0.0F ) return -1 ;
		return 1 ;
    }
    p1.vx = CROSS->vx - V3->vx ;
    p1.vz = CROSS->vz - V3->vz ;
    p2.vx = V4->vx - V3->vx ;
    p2.vz = V4->vz - V3->vz ;
    opz = p1.vz * p2.vx - p1.vx * p2.vz ;
    if ( opz < 0.0F ) return -1 ;    
    p1.vx = CROSS->vx - V4->vx ;
    p1.vz = CROSS->vz - V4->vz ;
    p2.vx = V1->vx - V4->vx ;
    p2.vz = V1->vz - V4->vz ;
    opz = p1.vz * p2.vx - p1.vx * p2.vz ;
    if ( opz < 0.0F ) return -1 ;    
    return 1 ;
}


/* １ストリップチェック */
static	void	CheckOneFloorStrip( seg, n ) 
HZX_VuSEG	*seg ;
int		n ;
{
    int		n_points, n_v ;
    int		type ;
    SVECTOR	*verts ;
    
    n_points = seg->b1.pad ;
    n_v = seg->b2.pad ;
    verts = seg->verts ;
    type = seg->atr ;
    while( n_points > 0 ) {
		SVtoFV_Trans( verts, V1 ) ;
		SVtoFV_Trans( verts + 1, V2 ) ;	
		SVtoFV_Trans( verts + 2, V3 ) ;
		SVtoFV_Trans( verts + 3, V4 ) ;
		if ( FloorDistance( type ) < 0 ) goto check_one_flr_skip ;
		if ( *LEN > *MINLEN ) goto check_one_flr_skip ;
		if ( CheckInsideFloor( type, n_v ) < 0 ) goto check_one_flr_skip ;
		*MINCROSS = *CROSS ;
		*MINLEN = *LEN ;
		HZX_BLOCK_PTR2 = C_BLOCK_PTR ;
		HZX_VSG_PTR2 = C_VSG_PTR ;
		HZX_SEG_PTR2 = seg->verts + ( seg->b1.pad - n_points ) ;
		*HZX_ATR_PTR2 = seg->atr ;
		*HIT = *HIT + 1 ;
		*HZX_GRP_PTR2 = seg->tag[ 3 ] ;
#if 0
		*MV1 = *V1 ;
		*MV2 = *V2 ;
		*MV3 = *V3 ;
		*MV4 = *V4 ;
#endif	
		check_one_flr_skip :
        n_points -= n_v ;
		verts += n_v ;
    }
}

/* １ブロックの床をチェック */
static	void	CheckOneBlockFlrCPU( b, seg, n_segs, flag )
HZX_BLOCK	*b ;
HZX_VuSEG	*seg ;
int		n_segs ;
int		flag ;
{
    int		i ;
    
    First2 = 1 ;
    TX = b->tx ; TY = b->ty ; TZ = b->tz ;
    C_BLOCK_PTR = b ;
    
    for ( i = 0; i < n_segs; i ++ ) {
		/* フラグチェック */
		if ( seg->atr & flag ) goto online_check_flr_skip_strip_cpu ;
		/* バウンディングチェック */
		if ( ( float )( seg->b1.vx + TX ) > BMAX->vx ||
			( float )( seg->b1.vy + TY ) > BMAX->vy ||
			( float )( seg->b1.vz + TZ ) > BMAX->vz ||
			( float )( seg->b2.vx + TX ) < BMIN->vx ||
			( float )( seg->b2.vy + TY ) < BMIN->vy ||
			( float )( seg->b2.vz + TZ ) < BMIN->vz ) {
			goto online_check_flr_skip_strip_cpu ;
		}
#if 0
		/* この処理、した方が遅い */
		sx = ( float )( seg->b2.vx - seg->b1.vx ) / 2.0F ;
		sy = ( float )( seg->b2.vy - seg->b1.vy ) / 2.0F ;
		sz = ( float )( seg->b2.vz - seg->b1.vz ) / 2.0F ;
		cx = ( float )( seg->b1.vx + b->tx ) + sx ;
		cy = ( float )( seg->b1.vy + b->ty ) + sy ;
		cz = ( float )( seg->b1.vz + b->tz ) + sz ;
		if ( !OnlineCheckBlock( FROM, TO, sx, sy, sz, cx, cy, cz ) ) 
			goto online_check_flr_skip_strip_cpu ;
#endif
		C_VSG_PTR = seg ;
		CheckOneFloorStrip( seg, i ) ;
		online_check_flr_skip_strip_cpu :
        seg ++ ;
    }
}

/* 動的床チェックＣＰＵ */
static	void	CheckOneDynamicFloor( seg )
HZX_D_FLOOR	*seg ;
{
    int		type ;
    
    type = seg->atr ;
    IVtoFV( &seg->p1, V1 ) ;
    IVtoFV( &seg->p2, V2 ) ;	
    IVtoFV( &seg->p3, V3 ) ;
    IVtoFV( &seg->p4, V4 ) ;
    if ( FloorDistance( type ) < 0 ) return ;
    if ( *LEN > *MINLEN ) return ;
    if ( CheckInsideFloor( type, seg->tag[ 0 ] ) < 0 ) return ;
    *MINCROSS = *CROSS ;
    *MINLEN = *LEN ;
    C_DYNAMIC = seg ;
    *HZX_ATR_PTR2 = seg->atr ;
    *HZX_GRP_PTR2 = seg->tag[ 3 ] ;
    *HIT = *HIT + 1 ;
}

static	void	CheckDynamicFloorCPU( seg, flag )
HZX_D_FLOOR	*seg ;
int		flag ;
{
    while( seg != NULL ) {
		/* フラグチェック */
		if ( seg->atr & flag ) goto online_check_flr_skip_strip_d_cpu ;
		/* バウンディングチェック */
		if ( ( float )seg->b1.vx > BMAX->vx ||
			( float )seg->b1.vy > BMAX->vy ||
			( float )seg->b1.vz > BMAX->vz ||
			( float )seg->b2.vx < BMIN->vx ||
			( float )seg->b2.vy < BMIN->vy ||
			( float )seg->b2.vz < BMIN->vz ) {
			goto online_check_flr_skip_strip_d_cpu ;
		}
		CheckOneDynamicFloor( seg ) ;
		online_check_flr_skip_strip_d_cpu :
        seg = seg->next ;
    }
}
#endif /* ALL_VU */

/*------------------------------------------------------------*/

/* チェックすべきブロックを検索 */
static	void	CheckBlockBound( grp, group )
HZX_GRP		*grp ;
int		group ;
{
    int		block, box ;
    int		dx, dy, dz, n_blocks ;
    float	bx, by, bz, mx, my, mz ;
    int		x1, x2, y1, y2, z1, z2 ;
	int		pre_block ;
	int		loop ;
    HZX_BLOCK	*b, *blk ;
    FVECTOR	from, to ;
    FVECTOR	b1, b2 ;

    N_InsideBlocks[ group ] = 0 ;
    
    bx = ( float )grp->block_size_x ;
    by = ( float )grp->block_size_y ;
    bz = ( float )grp->block_size_z ;
    
    mx = ( float )grp->bound_min_x ;
    my = ( float )grp->bound_min_y ;
    mz = ( float )grp->bound_min_z ;
    
    dx = grp->div_x ;
    dy = grp->div_y ;
    dz = grp->div_z ;
    
    n_blocks = grp->n_blocks ;
    b = grp->blocks ;

    /* ＦＲＯＭ－ＴＯベクトルを
       グループに入るように切る */
    b1.vx = mx + 1.0F ;
    b1.vy = my + 1.0F ;
    b1.vz = mz + 1.0F ;
    b2.vx = mx + bx * ( float )dx - 1.0F ;
    b2.vy = my + by * ( float )dy - 1.0F ;
    b2.vz = mz + bz * ( float )dz - 1.0F ;
    if ( !CutLinebyBlock( &b1, &b2, &from, &to ) ) return ;
    x1 = ( int )( ( from.vx - mx ) / bx ) ;
    z1 = ( int )( ( from.vz - mz ) / bz ) ;
    y1 = ( int )( ( from.vy - my ) / by ) ;

    block = dx * z1 + x1 + ( y1 * dx * dz ) ;
    if ( block < 0 || block >= n_blocks ) { 
//		printf( "?????1 %d %d %d ", x1, y1, z1 ) ; 
//		printf( ":( %d %d %d )\n", dx, dy, dz ) ;
		return ;
    }

	pre_block = -1 ;
	loop = 0 ;
    while( 1 ) {
		block = dx * z1 + x1 + ( y1 * dx * dz ) ;
		if ( pre_block == block ) {
			printf( "online warning : this block has already checked %d %d\n",
				     block, group ) ;
			break ;
		}

		pre_block = block ;
		blk = b + block ;
		b1.vx = mx + ( float )x1 * bx - 1.0F ; b2.vx = b1.vx + bx + 2.0F ;
		b1.vy = my + ( float )y1 * by - 1.0F ; b2.vy = b1.vy + by + 2.0F ;
		b1.vz = mz + ( float )z1 * bz - 1.0F ; b2.vz = b1.vz + bz + 2.0F ;
		if ( !CutLinebyBlock( &b1, &b2, &from, &to ) ) break ;

		if ( blk->n_segs != 0 ||
			blk->n_bul_segs != 0 ||
			blk->n_flrs != 0 ||
			blk->n_bul_flrs != 0 ) {
			box = InsideBlockCurrent + N_InsideBlocks[ group ] ;
			if ( box == MAX_INSIDE_BLOCKS ) {
				printf( "inside blocks over\n" ) ;
				break ;
			}	
			MakeBound( &FROMS[ box ], &TOS[ box ], &from, &to ) ;
			InsideBlock[ box ] = block ;
			N_InsideBlocks[ group ] ++ ;
		}

		x2 = ( int )( ( to.vx - mx ) / bx ) ;
		z2 = ( int )( ( to.vz - mz ) / bz ) ;
		y2 = ( int )( ( to.vy - my ) / by ) ;
		if ( ( x1 == x2 && y1 == y2 && z1 == z2 ) ||
			( x2 < 0 || x2 >= dx || y2 < 0 || y2 >= dy || z2 < 0 || z2 >= dz ) ) break ;
		x1 = x2 ; y1 = y2 ; z1 = z2 ;
		if ( ++ loop >= grp->n_blocks ) {
			printf( "warning : online check block : loop over!\n" ) ;
			break ;
		}
    }
    InsideBlockCurrent += N_InsideBlocks[ group ] ;
}

/*------------------------------------------------------------*/

/* 結果の格納 */
#ifndef ALL_VU
static	void	SetResult( hit, hit2, which )
int		hit, hit2, which ;
{
    HZX_VuSEG	*s ;
    HZX_D_SEGMENT	*dseg ;
    HZX_D_FLOOR		*dflr ;
    HZX_GRP	*g ;
    int		*tag ; 
    int		blk, seg, ptp, grp ;
    
    if ( which == 0 ) return ;
    if ( hit == 0 && hit2 == 0 ) return ;
    if ( which == 1 || ( which == 2 && CheckFlrVU == 1 ) ) {
		tag = ( u_int * )( VU0_MEM_ADDR + VU0_SEG_PTR * 16 ) ;
		ptp = tag[ 0 ] ; blk = tag[ 1 ] ; seg = tag[ 2 ] ; grp = tag[ 3 ] ;
		*HZX_GRP_PTR = grp ;
		g = HZX_PTR->def->groups + grp ;
		if ( blk >= 0 ) {
			HZX_BLOCK_PTR = g->blocks + blk ;
			if ( seg >= HZX_RECOIL_TYPE_SHIFT ) {
				seg -= HZX_RECOIL_TYPE_SHIFT ;
				if ( CheckFlrVU == 1 ) s = HZX_BLOCK_PTR->bul_flrs + seg ;
				else		       s = HZX_BLOCK_PTR->bul_segs + seg ;
			} else {
				if ( CheckFlrVU == 1 ) s = HZX_BLOCK_PTR->flrs + seg ;
				else		       s = HZX_BLOCK_PTR->segs + seg ;
			}
			HZX_VSG_PTR = s ;
			HZX_SEG_PTR = s->verts + ( s->b1.pad - ptp ) ;
			*HZX_ATR_PTR = s->atr ;
		} else {
			if ( CheckFlrVU == 0 ) {
				dseg = g->dynamics->segs ;
				while( -- seg >= 0 ) dseg = dseg->next ;
				DYNAMIC = ( HZX_D_FLOOR * )dseg ;
				*HZX_ATR_PTR = dseg->atr ;
			} else {
				dflr = g->dynamics->flrs ;
				while( -- seg >= 0 ) dflr = dflr->next ;
				DYNAMIC = dflr ;
				*HZX_ATR_PTR = dflr->atr ;		
			}
		}
    } else {
		DYNAMIC = C_DYNAMIC ;
		HZX_BLOCK_PTR = HZX_BLOCK_PTR2 ;
		HZX_VSG_PTR = HZX_VSG_PTR2 ;
		HZX_SEG_PTR = HZX_SEG_PTR2 ;
		*HZX_ATR_PTR = *HZX_ATR_PTR2 ;
		*HZX_GRP_PTR = *HZX_GRP_PTR2 ;
    }
}
#else
static	void	SetResult2( hit )
int		hit ;
{
    HZX_VuSEG	*s ;
    HZX_D_SEGMENT	*dseg ;
    HZX_D_FLOOR		*dflr ;
    HZX_GRP	*g ;
    int		*tag ; 
    int		blk, seg, ptp, grp ;

#ifdef DEBUG_PRINT
	DebugPrint() ;
#endif

    if ( hit == 0 ) return ;
    tag = ( u_int * )( VU0_MEM_ADDR + VU0_SEG_PTR * 16 ) ;
    ptp = tag[ 0 ] ; blk = tag[ 1 ] ; seg = tag[ 2 ] ; grp = tag[ 3 ] ;
    *HZX_GRP_PTR = grp ;
    g = HZX_PTR->def->groups + grp ;
    if ( blk >= 0 ) {
		HZX_BLOCK_PTR = g->blocks + blk ;
		if ( seg >= HZX_RECOIL_TYPE_SHIFT ) {
			seg -= HZX_RECOIL_TYPE_SHIFT ;
			if ( *WHICH == 1 ) s = HZX_BLOCK_PTR->bul_flrs + seg ;
			else	       s = HZX_BLOCK_PTR->bul_segs + seg ;
		} else {
			if ( *WHICH == 1 ) s = HZX_BLOCK_PTR->flrs + seg ;
			else	       s = HZX_BLOCK_PTR->segs + seg ;
		}
		HZX_VSG_PTR = s ;
		HZX_SEG_PTR = s->verts + ( s->b1.pad - ptp ) ;
		*HZX_ATR_PTR = s->atr ;
#ifdef DEBUG_MODE
		if ( HZX_OnlineDebugFlag == 2 ) {
			//ViewBlockHazardR( HZX_BLOCK_PTR ) ;
			if ( *WHICH == 1 ) HZX_ViewVuFlr( HZX_BLOCK_PTR, s ) ;
			else 			   HZX_ViewVuSeg( HZX_BLOCK_PTR, s ) ;
		}
#endif
    } else {
		if ( *WHICH == 0 ) {
			dseg = g->dynamics->segs ;
			seg = g->dynamics->n_segs - seg - 1 ;
			while( -- seg >= 0 ) dseg = dseg->next ;
			DYNAMIC = ( HZX_D_FLOOR * )dseg ;
			*HZX_ATR_PTR = dseg->atr ;
		} else {
			dflr = g->dynamics->flrs ;
			seg = g->dynamics->n_flrs - seg - 1 ;
			while( -- seg >= 0 ) dflr = dflr->next ;
			DYNAMIC = dflr ;
			*HZX_ATR_PTR = dflr->atr ;		
		}
    }
    *WHICH = *WHICH + 1 ;
}
#endif

/*------------------------------------------------------------*/

/* オンラインチェック */
int 	HZX_OnlineHazardCheck( HZX_GROUP_ID id, FVECTOR *from, FVECTOR *to,
							  int chk_flag, int seg_flag, int flr_flag )
{
    HZX_HDL	*hzd ;
    int		i, hit, recoil_check, only_recoil ;
    int		group, bit, bit2, n, box ;
    HZX_BLOCK	*b, *blk ;
    HZX_GRP	*grp ;

    if ( DG_FABS( from->vx - to->vx ) < 0.01F &&
		DG_FABS( from->vy - to->vy ) < 0.01F &&
		DG_FABS( from->vz - to->vz ) < 0.01F ) return 0 ;

    hzd = HZX_GetCurrentHzx() ;
    HZX_PTR = hzd ;
    
//    OnlineCheckSegs = 0 ;
//    OnlineCheckFlrs = 0 ;

    *WHICH = 0 ;
    CopyVector( FROM, from ) ;
    CopyVector( TO, to ) ;
    
    /* バウンディング作成 */
#if 0
    if ( FROM->vx < TO->vx ) {
		BMIN->vx = FROM->vx ; BMAX->vx = TO->vx ;
    } else {
		BMAX->vx = FROM->vx ; BMIN->vx = TO->vx ;
    }
    if ( FROM->vy < TO->vy ) {
		BMIN->vy = FROM->vy ; BMAX->vy = TO->vy ;
    } else {
		BMAX->vy = FROM->vy ; BMIN->vy = TO->vy ;
    }
    if ( FROM->vz < TO->vz ) {
		BMIN->vz = FROM->vz ; BMAX->vz = TO->vz ;
    } else {
		BMAX->vz = FROM->vz ; BMIN->vz = TO->vz ;
    }
#else
    MakeBound( BMIN, BMAX, FROM, TO ) ;
#endif

    /* 検索直線ベクトル */
    _sceVu0SubVector( DIFF, TO, FROM ) ;
    
    hit = 0 ;
    *HIT = 0 ;
    *WHICH = 0 ;
    First = 1 ;
    CheckFlrVU = 1 ;
    DYNAMIC = C_DYNAMIC = NULL ;
    DataPtr = VU0_DATA_PTR ;
    HZX_DP_WHICH = 0 ;
	HZX_BD_WHICH = 0 ;
    
    /* ブロックチェック */
    InsideBlockCurrent = 0 ;
    n = 0 ;

	/* サイファー専用チェック */
	if ( chk_flag & HZX_CHK_CYPHER ) {
		seg_flag &= ~HZX_SEG_CYPHER ;
		flr_flag &= ~HZX_FLOOR_CYPHER ;
	} else {
		seg_flag |= HZX_SEG_CYPHER ;
		flr_flag |= HZX_FLOOR_CYPHER ;
	}
	/* 崖あたりチェック */
	if ( chk_flag & HZX_CHK_CLIFF ) {
		seg_flag &= ~HZX_SEG_CLIFF ;
	} else {
		seg_flag |= HZX_SEG_CLIFF ;
	}

	id = HZX_AddGroupID( id ) ;
    bit = HZX_AddGroupID( HZX_CurrentGroupID ) | id ;
    while( bit != 0 ) {
		group = GV_GetNo( bit ) ; 
		bit2 = GV_GetBit( group ) ;
		if ( group >= 0 && group < hzd->def->n_groups ) {
			grp = hzd->grp + group ;
			if ( grp->n_blocks > 0 ) {
				Group[ n ] = group ; n ++ ;
				CheckBlockBound( grp, group ) ;
			}
		}
		bit &= ~bit2 ;
    }

    N_Groups = n ;

	if ( chk_flag & HZX_CHK_RECOIL_TYPE_ONLY ) {
		only_recoil = recoil_check = 1 ;
	} else {
		only_recoil = 0 ;
		recoil_check 
			= ( ( seg_flag & HZX_SEG_RECOIL_TYPE ) || ( chk_flag & HZX_CHK_RECOIL_TYPE ) ) ? 1 : 0 ;
	}

    InsideBlockCurrent = 0 ;
    for ( n = 0; n < N_Groups; n ++ ) {
		group = Group[ n ] ;
		grp = hzd->grp + group ;
		NEW_GROUP = 1 ;
		/* 固定壁チェック */
		if ( chk_flag & HZX_CHK_F_SEGMENT ) {
			blk = grp->blocks ;
			for ( i = 0; i < N_InsideBlocks[ group ]; i ++ ) {
				box = InsideBlockCurrent + i ;
				b = blk + InsideBlock[ box ] ;
#if 1
                CopyVector( BBMIN, &FROMS[ box ] ) ;
                CopyVector( BBMAX, &TOS[ box ] ) ;
#else
                CopyVector( BBMIN, BMIN ) ;
                CopyVector( BBMAX, BMAX ) ;
#endif
				if ( !only_recoil && b->n_segs > 0 ) 
					CheckOneBlockSeg( b, b->segs, b->n_segs, seg_flag ) ;
				if ( recoil_check && b->n_bul_segs > 0 ) {
					CheckOneBlockSeg( b, b->bul_segs, b->n_bul_segs, seg_flag ) ;
				}
			}
			InsideBlockCurrent += N_InsideBlocks[ group ] ;
		} 
		/* 動的壁チェック */
		if ( ( chk_flag & HZX_CHK_D_SEGMENT ) && 
			( grp->dynamics->n_segs > 0 ) ) {
			CheckDynamicSegment( grp->dynamics->segs, seg_flag ) ;
		}
    }
#ifndef ALL_VU    
    /* 壁をチェック */
    if ( First == 0 ) {
		ExecOnlineCheck() ;
		CheckFlrVU = 0 ;
    } 
    /* ＣＰＵ床チェック準備 */
    if ( CheckFlrVU == 0 ) {
		float	x, y, z ;
	
		HZX_BLOCK_PTR2 = NULL ;
		HZX_SEG_PTR2 = NULL ;
		HZX_VSG_PTR2 = NULL ;
	
		x = DIFF->vx ;
		y = DIFF->vy ;
		z = DIFF->vz ;
		if ( x < 0.0F ) x = -x ;
		if ( y < 0.0F ) y = -y ;
		if ( z < 0.0F ) z = -z ;
		*MINLEN = x + y + z ;
    } 
#endif
	if ( only_recoil == 0 ) {
		recoil_check 
			= ( ( flr_flag & HZX_FLOOR_RECOIL_TYPE ) || ( chk_flag & HZX_CHK_RECOIL_TYPE ) ) ? 1 : 0 ;
	}	

    InsideBlockCurrent = 0 ;

    for ( n = 0; n < N_Groups; n ++ ) {
		group = Group[ n ] ;
		grp = hzd->def->groups + group ;
		NEW_GROUP = 1 ;
		/* 固定床チェック */
		if ( chk_flag & HZX_CHK_F_FLOOR ) {
			blk = grp->blocks ;
			for ( i = 0; i < N_InsideBlocks[ group ]; i ++ ) {
				box = InsideBlockCurrent + i ;
				b = blk + InsideBlock[ box ] ;
#if 1
                CopyVector( BBMIN, &FROMS[ box ] ) ;
                CopyVector( BBMAX, &TOS[ box ] ) ;
#else
                CopyVector( BBMIN, BMIN ) ;
                CopyVector( BBMAX, BMAX ) ;
#endif
#ifndef ALL_VU
				if ( CheckFlrVU ) {
					if ( !only_recoil && b->n_flrs > 0 ) 
						CheckOneBlockFlr( b, b->flrs, b->n_flrs, flr_flag ) ;
					if ( recoil_check && b->n_bul_flrs > 0 ) 
						CheckOneBlockFlr( b, b->bul_flrs, b->n_bul_flrs, flr_flag ) ;
				} else {
					if ( !only_recoil && b->n_flrs > 0 ) 
						CheckOneBlockFlrCPU( b, b->flrs, b->n_flrs, flr_flag ) ;
					if ( recoil_check && b->n_bul_flrs > 0 ) 
						CheckOneBlockFlrCPU( b, b->bul_flrs, b->n_bul_flrs, flr_flag ) ;
				}
#else
				if ( !only_recoil && b->n_flrs > 0 ) {
					CheckOneBlockFlr( b, b->flrs, b->n_flrs, flr_flag ) ;
				}
				if ( recoil_check && b->n_bul_flrs > 0 ) {
					CheckOneBlockFlr( b, b->bul_flrs, b->n_bul_flrs, flr_flag ) ;
				}
#endif
			}
			InsideBlockCurrent += N_InsideBlocks[ group ] ;
		}
		/* 動的床チェック */
		if ( ( chk_flag & HZX_CHK_D_FLOOR ) && 
			( grp->dynamics->n_flrs > 0 ) ) {
#ifndef ALL_VU
			if ( CheckFlrVU ) CheckDynamicFloor( grp->dynamics->flrs, flr_flag ) ;
			else	      CheckDynamicFloorCPU( grp->dynamics->flrs, flr_flag ) ;
#else
			CheckDynamicFloor( grp->dynamics->flrs, flr_flag ) ;
#endif
		}
    }

#if 0
    if ( CheckFlrVU == 0 ) {
		printf( "--- floor cpu ---\n" ) ;
		printf( "%d hit\n", *HIT ) ;
		DumpVec( MINCROSS ) ;
		DumpVec( MV1 ) ;
		DumpVec( MV2 ) ;
		DumpVec( MV3 ) ;
		DumpVec( MV4 ) ;
		printf( "--- floor end ---\n" ) ;
    }
#endif
    /* 床チェック */
    if ( First == 0 && CheckFlrVU == 1 ) {
		ExecOnlineCheck() ;
    }
    /* Ｖｕ０終了待ち */
    while ( sceDmaSync( HZX_Vif0Dma, 0, 0 ) ) ;
    /* マイクロプログラム終了待ち */
    asm( "qmfc2.i    $0, vf01" ) ;
    while( *VIF0_STAT & VIF0_STAT_VEW_M ) ;  
    /* タグ開放 */
    DmaTag = ( sceDmaTag * )Vu0Tag ;

	//printf( "OnlineSegs %d %d\n", OnlineCheckSegs, OnlineCheckFlrs ) ;

    if ( First ) return 0 ;
	HZXD( if ( GV_PadData->status & PAD_L1 ) FlushCache( 0 ) ; )
	hit = *( u_int * )( VU0_MEM_ADDR + VU0_HIT_PTR * 16 ) ;
#ifndef ALL_VU
    if ( CheckFlrVU == 0 && *HIT == 0 ) *WHICH = ( hit > 0 ) ; /* 壁のみ */
    if ( CheckFlrVU == 1 && hit > 0 ) *WHICH = 2 ; /* 床のみ */
    if ( CheckFlrVU == 0 && *HIT > 0 ) { /* 床壁 */
		FVECTOR		v ;
		float		len1, len2 ;
		if ( hit == 0 ) *WHICH = 2 ; /* 床のみ */
		else {
			len1 = *( float * )( VU0_MEM_ADDR + VU0_LEN_PTR * 16 ) ;
			v = *MINCROSS ;
			_sceVu0SubVector( &v, &v, FROM ) ;
			len2 = GV_VecLen3F( &v ) ;
			if ( len1 < len2 ) *WHICH = 1 ;
			else	       *WHICH = 2 ;
		}
    }
    SetResult( hit, *HIT, *WHICH ) ;
#else
    *WHICH = *( u_int * )( VU0_MEM_ADDR + VU0_TYPE_PTR * 16 ) ;
    SetResult2( hit ) ;
#endif
    return *WHICH ;
}

/*-------------------------------------------------------------------------*/

/* ある壁とのオンラインチェック */
int		HZX_OnlineHazardCheckOneSegment( seg, from, to )
HZX_SEG		*seg ;
FVECTOR		*from, *to ;
{
    static HZX_D_SEGMENT	dseg ;
    IVECTOR			p[ 2 ] ;
    int				hit ;

    if ( DG_FABS( from->vx - to->vx ) < 0.10F &&
		DG_FABS( from->vy - to->vy ) < 0.10F &&
		DG_FABS( from->vz - to->vz ) < 0.10F ) return 0 ;
    CopyVector( FROM, from ) ;
    CopyVector( TO, to ) ;
    MakeBound( BMIN, BMAX, FROM, TO ) ;    
    _sceVu0SubVector( DIFF, TO, FROM ) ;
    *WHICH = 0 ;
    First = 1 ;
    DYNAMIC = C_DYNAMIC = NULL ;
    DataPtr = VU0_DATA_PTR ;
    HZX_DP_WHICH = 0 ;
	HZX_BD_WHICH = 0 ;

    p[ 0 ].vx = ( int )seg->p1.x ;
    p[ 0 ].vy = ( int )seg->p1.y ;
    p[ 0 ].vz = ( int )seg->p1.z ;
    p[ 0 ].vw = ( int )seg->p1.h ;
    p[ 1 ].vx = ( int )seg->p2.x ;
    p[ 1 ].vy = ( int )seg->p2.y ;
    p[ 1 ].vz = ( int )seg->p2.z ;
    p[ 1 ].vw = ( int )seg->p2.h ;
	dseg.callback = NULL ;
    dseg.tag[ 0 ] = 2 ;
    dseg.tag[ 1 ] = -1 ;
    dseg.tag[ 2 ] = 0 ;
    dseg.tag[ 3 ] = 0 ;
    dseg.next = NULL ; 
    dseg.atr = 0 ;

    HZX_MoveDynamicSegment( &dseg, &p[ 0 ], &p[ 1 ] ) ;

    CheckDynamicSegment( &dseg, 0 ) ;
    if ( First == 1 ) return 0 ;
    ExecOnlineCheck() ;
    while ( sceDmaSync( HZX_Vif0Dma, 0, 0 ) ) ;
    asm( "qmfc2.i    $0, vf01" ) ;
    while( *VIF0_STAT & VIF0_STAT_VEW_M ) ;  
    DmaTag = ( sceDmaTag * )Vu0Tag ;
    hit = *( u_int * )( VU0_MEM_ADDR + VU0_HIT_PTR * 16 ) ;
    return hit ;
}

/*-------------------------------------------------------------------------*/

static	void	SetFloor( flr, b, v, n )
HZX_FLR		*flr ;
HZX_BLOCK	*b ;
SVECTOR		*v ;
int		n ;
{
    float	tmp ;
    
    if ( b == NULL || v == NULL ) return ;
	flr->ptr = NULL ;

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
    
    GV_IVtoFV( ( int * )&dflr->p1, ( float * )&flr->p1, 4 ) ;
    GV_IVtoFV( ( int * )&dflr->p2, ( float * )&flr->p2, 4 ) ;
    GV_IVtoFV( ( int * )&dflr->p3, ( float * )&flr->p3, 4 ) ;
    GV_IVtoFV( ( int * )&dflr->p4, ( float * )&flr->p4, 3 ) ;
    tmp = flr->p1.y ; flr->p1.y = flr->p1.z ; flr->p1.z = tmp ;
    tmp = flr->p2.y ; flr->p2.y = flr->p2.z ; flr->p2.z = tmp ;
    tmp = flr->p3.y ; flr->p3.y = flr->p3.z ; flr->p3.z = tmp ;
    tmp = flr->p4.y ; flr->p4.y = flr->p4.z ; flr->p4.z = tmp ;
    tmp = flr->p2.h ; flr->p2.h = flr->p3.h ; flr->p3.h = tmp ;
	flr->p4.h = dflr->tag[ 0 ] ;
	flr->ptr = dflr ;
}

static	void	SetSegment( seg, b, v )
HZX_SEG		*seg ;
HZX_BLOCK	*b ;
SVECTOR		*v ;
{
    if ( b == NULL || v == NULL ) return ;
	seg->ptr = NULL ;

    seg->p1.x = v->vx + b->tx ;
    seg->p1.y = v->vy + b->ty ;
    seg->p1.z = v->vz + b->tz ;
    seg->p1.h = v->pad ;	v ++ ;
    seg->p2.x = v->vx + b->tx ;
    seg->p2.y = v->vy + b->ty ;
    seg->p2.z = v->vz + b->tz ;
    seg->p2.h = v->pad ;
}

static	void	SetSegmentD( seg, dseg )
HZX_SEG		*seg ;
HZX_D_SEGMENT	*dseg ;
{
    float		tmp ;
    
    GV_IVtoFV( ( int * )&dseg->p1, ( float * )&seg->p1, 4 ) ;
    GV_IVtoFV( ( int * )&dseg->p2, ( float * )&seg->p2, 4 ) ;
    tmp = seg->p1.y ; seg->p1.y = seg->p1.z ; seg->p1.z = tmp ;
    tmp = seg->p2.y ; seg->p2.y = seg->p2.z ; seg->p2.z = tmp ;
	seg->ptr = dseg ;
}

/* 結果を取得 */
void	HZX_GetOnlineHazard( HZX_FLR *seg, int *atr )
{
    int		which ;
    
    if ( ( which = *WHICH ) == 0 ) return ;
    if ( which == 1 ) {
		if ( DYNAMIC == NULL ) SetSegment( ( HZX_SEG * )seg, HZX_BLOCK_PTR, HZX_SEG_PTR ) ;
		else		       SetSegmentD( ( HZX_SEG * )seg, ( HZX_D_SEGMENT * )DYNAMIC ) ;
		( ( HZX_SEG * )seg )->attribute = *HZX_ATR_PTR ;
		seg->type = HZX_TYPE_SEGMENT ;
    } else {
		if ( DYNAMIC == NULL ) SetFloor( seg, HZX_BLOCK_PTR, HZX_SEG_PTR, HZX_VSG_PTR->b2.pad ) ;
		else		       SetFloorD( seg, DYNAMIC ) ;
		seg->attribute = *HZX_ATR_PTR ;
		seg->type = HZX_TYPE_FLOOR ;
    }
    if ( atr != NULL ) *atr = *HZX_ATR_PTR ;
}

void	HZX_GetOnlineHazard2( HZX_HZD *seg )
{
    int		which ;
    
    if ( ( which = *WHICH ) == 0 ) return ;
    if ( which == 1 ) {
		if ( DYNAMIC == NULL ) SetSegment( ( HZX_SEG * )seg, HZX_BLOCK_PTR, HZX_SEG_PTR ) ;
		else		       SetSegmentD( ( HZX_SEG * )seg, ( HZX_D_SEGMENT * )DYNAMIC ) ;
		( ( HZX_SEG * )seg )->attribute = *HZX_ATR_PTR ;
		seg->type = HZX_TYPE_SEGMENT ;
    } else {
		if ( DYNAMIC == NULL ) SetFloor( seg, HZX_BLOCK_PTR, HZX_SEG_PTR, HZX_VSG_PTR->b2.pad ) ;
		else		       SetFloorD( seg, DYNAMIC ) ;
		seg->attribute = *HZX_ATR_PTR ;
		seg->type = HZX_TYPE_FLOOR ;
    }
}

int	HZX_GetOnlineHazardFloorType( void )
{
    if ( DYNAMIC == NULL ) return HZX_VSG_PTR->b2.pad ;
    return ( DYNAMIC->tag[ 4 ] + 3 ) ;
}

/* 結果のポインタを取得 */
void	HZX_GetOnlineHazardPtr( SVECTOR *v, HZX_BLOCK *blk )
{
    v = HZX_SEG_PTR ;
    blk = HZX_BLOCK_PTR ;
}

/* ＦＲＯＭ－交差点ベクトルを取得 */
void	HZX_GetOnlineVector( FVECTOR *vect_ptr )
{
    FVECTOR	*cross ;
    
    if ( *WHICH == 2 && CheckFlrVU == 0 ) {
		cross = MINCROSS ;
    } else {
		cross = ( FVECTOR * )( VU0_MEM_ADDR + VU0_PTP_PTR * 16 ) ;
    }
    vect_ptr->vx = cross->vx - FROM->vx ;
    vect_ptr->vy = cross->vy - FROM->vy ;
    vect_ptr->vz = cross->vz - FROM->vz ;
}

/* 結果の交差点座標を取得 */
void	HZX_GetOnlinePoint( FVECTOR *ptp_ptr )
{
    FVECTOR	*cross ;
    
    if ( *WHICH == 2 && CheckFlrVU == 0 ) {
		cross = MINCROSS ;
    } else {
		cross = ( FVECTOR * )( VU0_MEM_ADDR + VU0_PTP_PTR * 16 ) ;
    }
    *ptp_ptr = *cross ;
}

/* 結果のアトリビュート取得 */
int	HZX_GetOnlineHazardAtr( void )
{
    return *HZX_ATR_PTR ;
}

/* 当たったのが壁か床か */
int	HZX_GetOnlineHazardType( void )
{
    return *WHICH ;
}

/* 当たった壁のグループ */
int	HZX_GetOnlineHazardGroup( void )
{
    return *HZX_GRP_PTR ;
}

/* 以前のバージョン用 */
HZX_SEG		*HZX_GetOnlineHazardF( void )
{
    HZX_SEG	*seg ;
    
    if ( *WHICH == 0 ) return NULL ;
    HZX_GetOnlineHazard( SEG_PTR, ATR_PTR ) ;
    seg = ( HZX_SEG * )SEG_PTR ;
    if ( *WHICH == 2 ) seg = ( HZX_SEG * )( ( u_int )seg | 0x80000000 ) ;
    return seg ;
}

/*----------------------------------------------------------------*/

/* シナリオオンラインチェック */
int		HZX_COM_OnlineCheck( void )
{
	FVECTOR		from, to ;

	GCL_GetOption( 'f' ) ;
	from.vx = ( float )GCL_GetNextInt() ;
	from.vy = ( float )GCL_GetNextInt() ;
	from.vz = ( float )GCL_GetNextInt() ;
	to.vx = ( float )GCL_GetNextInt() ;
	to.vy = ( float )GCL_GetNextInt() ;
	to.vz = ( float )GCL_GetNextInt() ;
	if ( HZX_OnlineHazardCheck( HZX_AllMapID, &from, &to,
							    HZX_CHK_ALL, HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
		return 1 ;
	}
	return 0 ;
}
