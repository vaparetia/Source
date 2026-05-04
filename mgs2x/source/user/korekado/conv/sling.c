//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sling.c
	オブジェクト設置

	1999/10/18 Y.Korekado
	$Id: sling.c,v 1.1.1.3 2002/11/19 11:44:05 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewSling( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-r	向き

----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#include <string.h>
#endif

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"

#include "bp_vector.h"

/*----------------------------------------------------------------*/
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

#define PRIO	0x60	/* 敵兵より後に処理 */


#define N_PRIMS	(4)		/* ４枚 */
#define N_VERTS (4)		/* 四角ポリゴン */

#define RGB_TEST	(1)
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT		sling_back ;
	OBJECT		adjust ;
	DG_PRIM2		*prim ;
	FMATRIX		*w1, *w2 ;
	FMATRIX		*lights ;

	FVECTOR		pos[ N_PRIMS*N_VERTS ] ;
	FVECTOR		adj_pos ;
	SVECTOR		adj_rot ;
#ifdef RGB_TEST
	FVECTOR		rgb_pos[ N_PRIMS*4 ] ;
#endif

#ifdef RGB_TEST
	FVECTOR		sft1[8] ;
#else
	FVECTOR		sft1[4] ;
#endif
	FVECTOR		sft2[4] ;

	OBJECT		*body ;
	
	int			count ;
} Work ;

static	FVECTOR	WakiTmp[2] ;


/*----------------------------------------------------------------*/
static void MakePos ( world, sft, pos )
FMATRIX	*world ;
FVECTOR	*sft ;
FVECTOR	*pos ;
{
	DG_SetPos( world ) ;
	DG_PutVector( sft, pos, 1 ) ;	/* 絶対座標へ変換 */
}

#define		GET_COL_B(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>8)&0xff)
#define		GET_COL_R(_rgba)	((_rgba)&0xff)

static void SetLights( Work *work, DG_PRIM2 *prim, FMATRIX *lights )
{
#if 1
	extern int GetRGBFromLightPos( FVECTOR *pos, FMATRIX *light );
	extern int GetRGBFromLightNorm( FVECTOR *norm, FMATRIX *light );
	DG_PRIM2_UVRGB	*uvrgb ;
	int i, j, rgb[4] ;

	for ( i=0; i<N_PRIMS; i++ ) {
#ifdef RGB_TEST
	    { // スリングのモデル部分とプリミティブ部分のつなぎをきれいにする Added by Okuta 2001/09/28
		FVECTOR vecNorm;
//FVECTOR vecNorm2;
#ifndef BP_PSX2_ASM
		FVECTOR vecDir[2];
                // sceライブラリVersion
		// つなぎ部分の頂点データから外積を用いて法線を求める
		_sceVu0SubVector( &vecDir[0], &work->rgb_pos[i*4+2], &work->rgb_pos[i*4+0]);
		_sceVu0SubVector( &vecDir[1], &work->rgb_pos[i*4+1], &work->rgb_pos[i*4+0]);
		_sceVu0Normalize( &vecDir[0], &vecDir[0]);
		_sceVu0Normalize( &vecDir[1], &vecDir[1]);
		_sceVu0OuterProduct( &vecNorm, &vecDir[0], &vecDir[1]);
		_sceVu0Normalize( &vecNorm, &vecNorm);
//_sceVu0CopyVector( &vecNorm2, &vecNorm);
#else
		/*
		  asm version : sceライブラリVersionより1.8倍ぐらいはやい
		  vf03 : 入力 work->rgb_pos[0]をロード
		  vf04 : 入力 work->rgb_pos[1]をロード
		  vf05 : 入力 work->rgb_pos[2]をロード
		  vf06 : 外積結果
		  vf07 : 出力 計算結果
		  vf08 : vf05 - vf03差分ベクトルの正規化されたもの
		  vf09 : vf04 - vf03差分ベクトルの正規化されたもの
		  vf12 : テンポラリ
		  vf13 : テンポラリ
		  vf14 : vf05 - vf03差分ベクトル
		  vf15 : vf04 - vf03差分ベクトル
		  vf18 : ZeroVector
		 */
		asm volatile ("
                # 入力
                lqc2		vf03,  0x00(%1)		# work->rgb_pos[0]をロード
                lqc2		vf04,  0x00(%2)		# work->rgb_pos[1]をロード
                lqc2		vf05,  0x00(%3)		# work->rgb_pos[2]をロード
                vmulx.wxyz	vf18,  vf00,  vf00x	# ZeroVector作成
                # 差分ベクトルを作成(１つめ)
                vsub.xyz	vf14,  vf03,  vf05 	# work->rgb_pos[2] - work->rgb_pos[0]
                # 正規化(１つめ)
                vmul.xyz        vf13,  vf14,  vf14	# 差分ベクトルの２乗
                vaddy.x		vf13,  vf13,  vf13y	# X成分 + Y成分
                vaddz.x		vf13,  vf13,  vf13z	# X成分 + z成分
                vsqrt		   Q,  vf13x		# 距離をQレジスタに格納
                vwaitq					# Qレジスタ待ち
                vaddq.x         vf12,  vf18x, Q		# 距離取得
                vnop					# EEのQをリードする命令とCOP2の除算命令とのバグで追加
                # 差分ベクトルを作成(２つめ)
                vsub.xyz	vf15,  vf03,  vf04 	# work->rgb_pos[1] - work->rgb_pos[0] (vnopの代替)
                vdiv               Q,  vf00w, vf12x	# 1.f / 距離
                vsub.wxyz	vf08,  vf00,  vf00	# クリア
                vmul.xyz        vf13,  vf15,  vf15	# 差分ベクトルの２乗 waitq命令のかわりに次の奴を先行計算
                vaddy.x		vf13,  vf13,  vf13y	# X成分 + Y成分
                vaddz.x		vf13,  vf13,  vf13z	# X成分 + z成分
                vwaitq					# Qレジスタ待ち(略可)
                vmulq.xyz	vf08,  vf14,  Q		# 正規化
                # 正規化(２つめ)
                vsqrt		   Q,  vf13x
                vwaitq					# Qレジスタ待ち
                vaddq.y         vf12,  vf18y, Q		# 距離取得
                vnop					# EEのQをリードする命令とCOP2の除算命令とのバグで追加
                vnop					# （2命令待たないと例外発生した時に命令が逆転するケースがある 2001/09/11）
                vdiv               Q,  vf00w, vf12y	# 1.f / 距離	
                vwaitq					# Qレジスタ待ち
                vmulq.xyz	vf09,  vf15,  Q		# 正規化
                # 外積
        	vopmula.xyz	ACC,   vf08,   vf09
        	vopmsub.xyz	vf06,  vf09,   vf08
        	vsub.w 		vf06,  vf06,   vf06	# vf6.xyz=0;
                # 正規化
                vmul.xyz        vf13,  vf06,  vf06	# 外積結果の２乗
                vaddy.x		vf13,  vf13,  vf13y	# X成分 + Y成分
                vaddz.x		vf13,  vf13,  vf13z	# X成分 + z成分
                vsqrt		   Q,  vf13x		# 距離をQレジスタに格納
                vwaitq					# Qレジスタ待ち
                vaddq.x         vf12,  vf18x, Q		# 距離取得
                vnop					# EEのQをリードする命令とCOP2の除算命令とのバグで追加
                vnop					# （2命令待たないと例外発生した時に命令が逆転するケースがある 2001/09/11）
                vdiv               Q,  vf00w, vf12x	# 1.f / 距離
                vwaitq					# Qレジスタ待ち
                vmulq.xyz	vf07,  vf06,  Q		# 正規化
                # 結果を出力
                sqc2		vf07, 0x00(%0)		
        	": : "r"(&vecNorm), "r" (&work->rgb_pos[i*4+0]) , "r" (&work->rgb_pos[i*4+1]) ,"r" (&work->rgb_pos[i*4+2]));
#endif
		// 法線とライトマトリクスからRGB値を取得
		rgb[i] = GetRGBFromLightNorm( &vecNorm , lights );
#if 0
		{
		    FVECTOR vecTmp;
		    FVECTOR vecDbg[2];

		    _sceVu0ScaleVector( &vecTmp, &vecNorm, 500.f);
		    _sceVu0CopyVector( &vecDbg[0], &work->rgb_pos[i*4]);
//		    _sceVu0SubVector( &vecDbg[0], &vecDbg[0], &vecTmp);
		    _sceVu0CopyVector( &vecDbg[1], &work->rgb_pos[i*4]);
		    _sceVu0AddVector( &vecDbg[1], &vecDbg[1], &vecTmp);
if(i==0)	    NewLineView( &vecDbg[0], 1, 0xff, 0x00, 0x00);
else if(i==1)       NewLineView( &vecDbg[0], 1, 0x00, 0xff, 0x00);
else if(i==2)       NewLineView( &vecDbg[0], 1, 0x00, 0x00, 0xff);
else if(i==3)       NewLineView( &vecDbg[0], 1, 0x00, 0xff, 0xff);
//		    AN_Test_Eye2( &work->rgb_pos[i*4], 3 );
		}
#endif
	    }
	}
#endif
	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;
	for ( i=0; i<N_PRIMS; i++ ) {
	    for ( j=0; j<N_VERTS; j++ ) {
		if ( j % 2 == 0 ){
		    uvrgb->r = GET_COL_R(rgb[i]) ;
		    uvrgb->g = GET_COL_G(rgb[i]) ;
		    uvrgb->b = GET_COL_B(rgb[i]) ;
		}else{
		    uvrgb->r = GET_COL_R(rgb[2]) ;
		    uvrgb->g = GET_COL_G(rgb[2]) ;
		    uvrgb->b = GET_COL_B(rgb[2]) ;
		}
		uvrgb ++ ;
	    }
	}
#else
	extern	void	Big_CalcRGB( UCVECTOR *rgb, FMATRIX *lights, FVECTOR *norm) ;
	DG_PRIM2_UVRGB	*uvrgb ;
	UCVECTOR rgb ;
	int i, j ;

	Big_CalcRGB( &rgb, lights, NULL ) ;

	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;
	for ( i=0; i<N_PRIMS; i++ ) {
		for ( j=0; j<N_VERTS; j++ ) {
			uvrgb->r = rgb.r ;
			uvrgb->g = rgb.g ;
			uvrgb->b = rgb.b ;
			uvrgb ++ ;
		}
	}
#endif
#if 0
	{
	    for ( i = 0; i < 16; i++){
		extern void 	SIG_NumPrint(FVECTOR*, int);
		SIG_NumPrint( (prim->pos[prim->buffer_clock]+i), i);
	    }
	}
#endif

}

#if 0
static	void	DivVector( vec1, vec2, n )
FVECTOR	*vec1, *vec2 ;
float	n ;
#else
static	void	DivVector( FVECTOR *vec1, FVECTOR *vec2, float n )
#endif
{
	vec1->vx = vec2->vx / n ;
	vec1->vy = vec2->vy / n ;
	vec1->vz = vec2->vz / n ;
}

//static	FVECTOR	SlingWidth = { 23.3, 26.11, 0 } ;
static	FVECTOR	SlingWidth = { 0, 35.0, 0 } ;

static	void	MakeAdjustPos( work )
Work	*work ;
{
	FVECTOR	addpos, vec ;
	FMATRIX	mat ;
	FVECTOR	pos1, pos2 ;

	/* 座標を銃後寄りにする */
	_sceVu0AddVector( &pos1, &work->pos[1], &work->pos[7] ) ; 	/* 銃後 ＋ 銃前 */
	DivVector( &pos1,  &pos1, 2.0 ) ;
	_sceVu0AddVector( &pos2, &work->pos[1], &work->pos[12] ) ; 	/* 銃後 ＋ 横腹 */
	DivVector( &pos2,  &pos2, 2.0 ) ;

	_sceVu0AddVector( &addpos, &work->pos[1], &pos1 ) ; 	/* 銃後 ＋ 銃前 */
	_sceVu0AddVector( &addpos, &addpos, &pos2 ) ;		/* ＋ 横腹 */

	/* アジャスト位置 */
	DivVector( &WakiTmp[0],  &addpos, 3.0 ) ;
	_sceVu0SubVector(  &vec, &WakiTmp[0], &work->pos[12] ) ;
	work->adj_rot.vx = _FVecDirX( &vec ) - 2048 ;
	work->adj_rot.vy = _FVecDir2( &vec ) - 1024 ;
	work->adj_rot.vz = 0 ;
	work->adj_pos = WakiTmp[0] ;

	DG_SetPos2( &work->adj_pos, &work->adj_rot ) ;
	DG_PutObjs( work->adjust.objs ) ;

	DG_MovePos( &SlingWidth ) ;
	DG_GetPos( &mat ) ;
	KR_FMatToFvec( &mat, &WakiTmp[1] ) ;
}

static	void	SetPosSring( work )
Work	*work ;
{
	/* 右肩から銃後ろ */
	MakePos( work->w1, &work->sft1[0], &work->pos[0] ) ;	/* 右肩 */
	MakePos( work->w1, &work->sft1[1], &work->pos[2] ) ;
	MakePos( work->w2, &work->sft2[0], &work->pos[1] ) ;	/* 銃後ろ */
	MakePos( work->w2, &work->sft2[1], &work->pos[3] ) ;

	/* 左胸から銃前 */
	MakePos( work->w2, &work->sft2[2], &work->pos[7] ) ;	/* 銃前 */
	MakePos( work->w2, &work->sft2[3], &work->pos[5] ) ;

	/* 左胸から銃後ろ */
	MakePos( work->w2, &work->sft2[0], &work->pos[9] ) ;	/* 銃後ろ */
	MakePos( work->w2, &work->sft2[1], &work->pos[11] ) ;

	/* 横腹から左胸 */
	MakePos( work->w1, &work->sft1[2], &work->pos[12] ) ;	/* 横腹 */
	MakePos( work->w1, &work->sft1[3], &work->pos[14] ) ;

	/* アジャスト部 */
	MakeAdjustPos( work ) ;
	/* 左胸から銃前 */
	work->pos[6]	=  WakiTmp[0] ;		/* 左胸 */
	work->pos[4]	=  WakiTmp[1] ;
	/* 左胸から銃後ろ */
	work->pos[10]	=  WakiTmp[0] ;		/* 左胸 */
	work->pos[8]	=  WakiTmp[1] ;
	/* 横腹から左胸 */
	work->pos[15]	=  WakiTmp[0] ;		/* 左胸 */
	work->pos[13]	=  WakiTmp[1] ;

	memcpy( work->prim->pos[work->prim->buffer_clock], &work->pos[0], sizeof(FVECTOR) * N_PRIMS*N_VERTS ) ;

#ifdef RGB_TEST
	MakePos( work->w1, &work->sft1[4], &work->rgb_pos[0] ) ;	/* 右肩 */
	MakePos( work->w1, &work->sft1[5], &work->rgb_pos[1] ) ;
	work->rgb_pos[2]	=  work->pos[0] ;		/* 右肩 */
	work->rgb_pos[3]	=  work->pos[2] ;

	work->rgb_pos[4]	=  work->pos[10] ;		/* 銃前 */
	work->rgb_pos[5]	=  work->pos[8] ;
	work->rgb_pos[6]	=  work->pos[7] ;		/* 銃前 */
	work->rgb_pos[7]	=  work->pos[5] ;

	work->rgb_pos[8]	=  work->pos[1] ;		/* 銃後ろ */
	work->rgb_pos[9]	=  work->pos[3] ;
	work->rgb_pos[10]	=  work->pos[8] ;
	work->rgb_pos[11]	=  work->pos[10] ;

	work->rgb_pos[12]	=  work->pos[12] ;		/* 横腹 */
	work->rgb_pos[13]	=  work->pos[14] ;
	MakePos( work->w1, &work->sft1[6], &work->rgb_pos[14] ) ;	/* 横腹 */
	MakePos( work->w1, &work->sft1[7], &work->rgb_pos[15] ) ;

#endif

}

static	void	SetInvisible( work )
Work	*work ;
{
	if ( work->body->objs->flag & (DG_FLAG_INVISIBLE|DG_FLAG_OPTCMF) ) {
		work->sling_back.objs->flag |= DG_FLAG_INVISIBLE ;
		work->adjust.objs->flag |= DG_FLAG_INVISIBLE ;
		work->prim->flag |= DG_PRIM2_INVISIBLE ;
	} else {
		work->sling_back.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->adjust.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->prim->flag &= ~DG_PRIM2_INVISIBLE ;
	}
	KR_GroupObject( &work->sling_back, work->body->map_name ) ;
	KR_GroupObject( &work->adjust, work->body->map_name ) ;
	GM_GroupPrim2( work->prim, work->body->map_name ) ;
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	SetInvisible( work ) ;

	DG_SwitchBuffPrim2( work->prim );
	SetPosSring( work ) ;
	SetLights( work, work->prim, work->lights ) ;

#if 0
if ( !(GV_Time & 0x80) ) {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[0], 3 );
	} else {
		AN_Test_Eye2( &work->pos[1], 3 );
	}
} else {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[2], 3 );
	} else {
		AN_Test_Eye2( &work->pos[3], 3 );
	}
}

if ( GV_Time & 0x80 ) {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[4], 3 );
	} else {
		AN_Test_Eye2( &work->pos[5], 3 );
	}
} else {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[6], 3 );
	} else {
		AN_Test_Eye2( &work->pos[7], 3 );
	}
}
#endif

}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &(work->sling_back) ) ;
    GM_FreeObject( &(work->adjust) ) ;
	GM_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
/* パケットの初期化 */
static void InitPacket( DG_PRIM2 *prim, DG_TEX *tex )
{
	int	i ;
	DG_PRIM2_UVRGB	*uvrgb ;

	uvrgb = SCRPAD_ADDR ;	/* スクラッチパッド利用 */
	for( i=0; i<N_PRIMS; i++ ) {
		/* ＵＶ値は1.3.12の固定小数点なので注意！ */
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;
	}
	KR_MemCopy( prim->uvrgb[ 0 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS ) ;
	KR_MemCopy( prim->uvrgb[ 1 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS ) ;
}

static	int	GetResources( work, body, weapon, sft1, sft2, lights, data )
Work	*work ;
OBJECT		*body, *weapon ;
FVECTOR		*sft1, *sft2 ;
FMATRIX		*lights ;
int data;
{
	DG_TEX	*tex ;
	DG_PRIM2	*prim ;
	int		i ;

	/* 背中側スリングモデル */
	GM_InitObject( &(work->sling_back), data , DG_FLAG_SHADE|DG_FLAG_ONEPIECE );

	GM_ConfigObjectRoot( &(work->sling_back), body, HUMAN21_MUNE ) ;

	/* アジャスターモデル */
	GM_InitObject( &(work->adjust), GV_StrCode("gbs_kanagu") , DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	DG_SetLightMatrix( work->adjust.objs, body->objs->light );
	
	work->body = body ;
	work->w1 = &BODYWORLD( body, HUMAN21_MUNE ) ;
	work->w2 = &weapon->objs->world ;
	work->lights = lights ;

	for ( i = 0 ; i < 8 ; i++ ){
	    work->sft1[i] = *sft1 ;
	    sft1++ ;
	}
	for ( i = 0 ; i < 4 ; i++ ){
	    work->sft2[i] = *sft2 ;
	    sft2++ ;
	}

	prim = work->prim = GM_MakePrim2( DG_PRIM2_FOG|DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_SHADE, N_PRIMS, N_VERTS );
	tex = DG_GetTexture( GV_StrCode("gbs_ak_sling2") );	/* ak_sling.tri */
	DG_ConfigPrim2Tex( prim, tex );

	InitPacket( prim, tex ) ;

	SetPosSring( work ) ;

	work->count = 0 ;

	return 0 ;
}

void		*NewSling( body, weapon, sft1, sft2, lights, data )
OBJECT		*body, *weapon ;
FVECTOR		*sft1, *sft2 ;
FMATRIX		*lights ;
int			data ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, weapon, sft1, sft2, lights, data ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

void	NewBodySling( body, weapon, lights )
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	FVECTOR	sft1[4], sft2[4] ;
	
	/* スリング */
/*	A：39.5, -55.5, 72.6
	B：39.5, -55.5, 37.6
	C：20.8, -452, 121.4
	D：51.1, -452, 103.9
	*/
	/* 右肩 */
	sft1[0].vx = -89.28 ; sft1[0].vy = 308.22 ; sft1[0].vz = 15.59 ;
	sft1[1].vx = -112.58 ; sft1[1].vy = 282.1 ; sft1[1].vz = 15.59 ;
	/* 左横腹 */
	sft1[2].vx = 205.86 ; sft1[2].vy = 44.79 ; sft1[2].vz = 59.88 ;
	sft1[3].vx = 182.56 ; sft1[3].vy = 18.67 ; sft1[3].vz = 59.88 ;

	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 20.8 ; sft2[2].vy = -452.0 ; sft2[2].vz = 121.4 ;
	sft2[3].vx = 51.0 ; sft2[3].vy = -452.0 ; sft2[3].vz = 103.9 ;

	NewSling( body, weapon, sft1, sft2, lights, GV_StrCode("gbs_sling") ) ;
}
