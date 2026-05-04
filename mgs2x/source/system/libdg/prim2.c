//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	prim2.c
	チャンネル処理ユニット／新プリミティブ処理ルーチン

	1999/12/22 K.Takabe
	$Id: prim2.c,v 1.1.1.3 2002/11/19 11:42:20 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／新プリミティブ処理ルーチン



	void DG_Prim2Chanl( DG_CHANL *cp, int which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされたプリミティブ２のセットアップ、ソートなどを行なう


	------------------------------------------------

	DG_PRIM2 *DG_MakePrim2( int flag, int n_prims, int n_verts, int chanl )
	int		flag ;		生成フラグ
	int		n_prims ;	プリミティブ数
	int		n_verts ;	１プリミティブ中に含まれる頂点数
	int		chanl ;		対象チャンネル

		プリミティブ２オブジェクトの生成を行なう
		ソートはプリミティブ単位で各プリミティブのＺ中央値が参照される
		プリミティブ中に含まれる頂点数の最大はプリミティブの種類によって
		異なる（ライン、ポリゴン：６４・スプライト：３２・回転スプライト：１６）


	void DG_FreePrim2( DG_PRIM2 *prim )
	DG_PRIM2	*prim ;	オブジェクトハンドル

		プリミティブ２オブジェクトの開放


	------------------------------------------------

	void DG_ConfigPrim2Tex( DG_PRIM2 *prim, DG_TEX *tex )
	DG_PRIM2	*prim ;	オブジェクトハンドル
	DG_TEX		*tex ;	テクスチャ

		プリミティブにテクスチャを設定する



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

#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"
#include	"utl_dma.h"

#include "bp_matrix.h"

#include "BP_Renderer.h"
#include "BP_RenderPrim.h"
#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"
#include "BP_boundvu.h"
#include "BP_Asm.h"
#include "BP_vu.h"
#include "BP_ee.h"
#include "BP_Debug.h"

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	(sizeof(_v)/sizeof(int))
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#define GS_REGS_0()
#define GS_REGS_1(r0) \
((r0) << 0x00)
#define GS_REGS_2(r0, r1) \
((r0) << 0x00 | (r1) << 0x04)
#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_4(r0, r1, r2, r3) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)
#define GS_REGS_6(r0, r1, r2, r3, r4, r5) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10 | (r5) << 0x14)

#define USE_FOG	0
#define USE_ABE	1
#define USE_AA1	0


	/*
		スクラッチパッド割り当て
	*/
typedef	struct _prim2_ScrpadWork	{
	FMATRIX		eye_pers ;
	FMATRIX		eye_pers2 ;
	FMATRIX		pers ;
	FMATRIX		world ;
	FMATRIX		screen ;
	FVECTOR		bound[8] ;
	FVECTOR		scale ;
	FVECTOR		tmp_vec ;
	void		*ot3[64] ;
	void		*ot2[64] ;
	void		*ot1[64] ;
#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	prim_poly ;
	PERFORMANCE_PACKET_INFO	prim_line ;
	PERFORMANCE_PACKET_INFO	prim_sprt ;
	PERFORMANCE_PACKET_INFO	prim_rsprt ;
	PERFORMANCE_PACKET_INFO	*trg_prim_info ;
#endif
} ScrpadWork ;


#define	SCRPAD		((ScrpadWork*)SCRPAD_ADDR)
#define	PERS_MAT	(&(SCRPAD->pers))
#define	EYE_PERS	(&(SCRPAD->eye_pers))
#define	EYE_PERS2	(&(SCRPAD->eye_pers2))
#define	WORLD		(&(SCRPAD->world))
#define BOUNDS		(SCRPAD->bound)
#define SCALE		(&SCRPAD->scale)
#define SORT_Z		(SCRPAD->sort_z)
#define TMP_VEC		(SCRPAD->tmp_vec)
#define OT1			(SCRPAD->ot1)
#define OT2			(SCRPAD->ot2)
#define OT3			(SCRPAD->ot3)

/*----------------------------------------------------------------*/

void	*DG_Prim2OT[64] ;

/*----------------------------------------------------------------*/

static void bp_prim_render_ult( SULTParam const *param )
{
   BP_Prim_Render_Execute( (struct SPrimRenderState const *) param );
}

#define skMaxPendingPrimUlt (10)

char sPrimUltDataStorage[ C_PRIMRENDERSTATE_SIZE * skMaxPendingPrimUlt ];
int sNumPendingPrimUlt = 0;

static void bp_prim_flush_ult()
{
   if ( sNumPendingPrimUlt )
   {
      BP_Render_PostUltWorks( bp_prim_render_ult, &sPrimUltDataStorage, C_PRIMRENDERSTATE_SIZE, sNumPendingPrimUlt );

      sNumPendingPrimUlt = 0;
   }
}

static inline void bp_prim_post_ult( void *param )
{
   if ( BP_Prim_Render_MakeState( (struct SPrimRenderState *) (sPrimUltDataStorage + C_PRIMRENDERSTATE_SIZE * sNumPendingPrimUlt), param ) )
   {
      if ( sNumPendingPrimUlt == skMaxPendingPrimUlt - 1 )
      {
         BP_Render_PostUltWorks( bp_prim_render_ult, &sPrimUltDataStorage, C_PRIMRENDERSTATE_SIZE, skMaxPendingPrimUlt );
         sNumPendingPrimUlt = 0;
      }
      else
      {
         ++sNumPendingPrimUlt;
      }
   }
}

/*----------------------------------------------------------------*/
	/*
		プリミティブのソートリストを作成する
	*/
static	void	MakeSortListPrim( DG_PRIM2_PACKET *packet, int raise )
{
	ScrpadWork	*work = SCRPAD_ADDR ;
	u_int		z ;

	/* ソート用データを作成 */
	z = ( (u_int)( packet->sort_z + 0x7fffff - raise ) >> 6 ) ;/* あらかじめ６４で割っておく */
	packet->sort_z = z ;
   z &= 63 ;
	packet->next_addr = work->ot1[z] ;
   work->ot1[z] = packet ;
}

	/*
		ソートを行う
	*/
static	void	SortListPrims( void )
{
	ScrpadWork	*work = SCRPAD_ADDR ;
	int			i ;
	u_int		z ;
	DG_PRIM2_PACKET	*packet ;
	void		**ot2, **ot3, *next_addr ;

	/* 順番を崩さないように大きい方からソートしていく */
	for ( i = 63 ; i >= 0  ; i-- ){
		next_addr = work->ot1[i] ;
		while ( next_addr != NULL ){
			packet = next_addr ;
			next_addr = packet->next_addr ;
			z = packet->sort_z ; z = ( z >> 6 ) & 63 ;
			ot2 = work->ot2 + z ;
			packet->next_addr = *ot2 ;
			*ot2 = packet ;
		}
	}

	/* 順番を崩さないように小さい方からソートしていく */
	for ( i = 0 ; i < 64   ; i++ ){
		next_addr = work->ot2[i] ;
		while ( next_addr != NULL ){
			packet = next_addr ;
			next_addr = packet->next_addr ;
			z = packet->sort_z ; z = ( z >> 12 ) & 63 ;
			ot3 = work->ot3 + z ;
			packet->next_addr = *ot3 ;
			*ot3 = packet ;
		}
	}

	{/* 最終ＯＴをメインメモリに転送する */
		u_long128	*dst, *src ;
		src = (u_long128*)work->ot3 ;
		dst = (u_long128*)DG_Prim2OT ;
		for ( i = 64/4 ; i > 0 ; i-- ){
			*dst++ = *src++ ;
		}
	}
}
	/*
		ＤＭＡに接続する
	*/
#if 0 //BP_PS2
int DG_WritePrimPacks( DG_PRIM2_PACKET *packet, void *addr, int which )
{
	//extern int Vu1DrawPrim_Func[] ;
#if 0 //BP_PS2
	extern int Vu1DrawPrim2_Func[] ;
#endif
	DG_DMATAG	*tag ;
	DG_PRIM2	*prim ;
	int			prog_no = 0 ;

	tag = addr ;

	prim = packet->prim ;

	/* ＶＵ１プログラム選択 */
	switch ( packet->type ){
	  case DG_PRIM2_LINE:
		prog_no = 2 ;
		break ;
	  case DG_PRIM2_POLY:
		prog_no = 0 ;
		break ;
	  case DG_PRIM2_CULLPOLY:
		prog_no = 1 ;
		break ;
	  case DG_PRIM2_SPRT:
		prog_no = 3 ;
		break ;
	  case DG_PRIM2_RSPRT:
		prog_no = 4 ;
		break ;
	}

	if ( packet->type < DG_PRIM2_SPRT ){
		/* 頂点データの転送 */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet->n_verts ) ;
		tag->addr = packet->pos_addr ;
		tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x100+0, packet->n_verts, VIF_DATA128, 0 ) ;
		tag++ ;
		/* ＵＶ、ＲＧＢデータの転送 */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet->n_verts ) ;
		tag->addr = packet->uvrgb_addr ;
		tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 2, 3, 0 ) ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x100+1, packet->n_verts * 2, VIF_VERT_PACK, 0 ) ;
		tag++ ;
	} else if ( packet->type == DG_PRIM2_SPRT ){
		/* 頂点データの転送 */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet->n_verts ) ;
		tag->addr = packet->pos_addr ;
		tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 6, 0 ) ;/* １つおきに書き込む */
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x100+0, packet->n_verts, VIF_DATA128, 0 ) ;
		tag++ ;
		/* ＵＶ、ＲＧＢデータの転送 */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet->n_verts * 2 ) ;
		tag->addr = packet->uvrgb_addr ;
		tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 2, 3, 0 ) ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x100+1, packet->n_verts * 4, VIF_VERT_PACK, 0 ) ;
		tag++ ;
	} else {
		/* 頂点データの転送 */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet->n_verts ) ;
		tag->addr = packet->pos_addr ;
		tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 12, 0 ) ;/* ３つおきに書き込む */
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x100+0, packet->n_verts, VIF_DATA128, 0 ) ;
		tag++ ;
		/* ＵＶ、ＲＧＢデータの転送 */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, packet->n_verts * 2 ) ;
		tag->addr = packet->uvrgb_addr ;
		tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 4, 12, 0 ) ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x100+1, packet->n_verts * 4, VIF_VERT_PACK, 0 ) ;
		tag++ ;
	}

	/* テクスチャ用パラメータの転送 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) ) ;
	tag->addr = &prim->tex_trans ;
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0xe8, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;
	tag++ ;

	/* 描画用パラメータの転送 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 4 ) ;
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x10, 4, VIF_DATA128, 0 ) ;
	tag++ ;

	*(FMATRIX*)tag = prim->prim_param[which].screen ;
	tag += 4 ;

	/* 描画用パラメータの転送 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_PRIM2_PARAM)-4 ) ;
	tag->addr = &prim->prim_param[which].n_verts ;
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x14, SIZEOF_QWORD(DG_PRIM2_PARAM)-4, VIF_DATA128, 0 ) ;
	tag++ ;

	/* プログラム開始命令の転送 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
	tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
#if 0 //BP_PS2
	tag->vifcode[1] = SCE_VIF1_SET_MSCAL( Vu1DrawPrim2_Func[prog_no], 0 ) ;
#endif
	tag++ ;

	return ( ( (int)tag - (int)addr ) / sizeof(u_long128) );
}
#elif 0
int DG_WritePrimPacks( DG_CHANL *cp, DG_PRIM2_PACKET *packet, void *addr, int which)
{
   //BP TODO: Need to do this globally once before prims get rendered (both prim channel as well as SortChain.. in chain.c)
   {
      DG_PRIM2* prim = (DG_PRIM2*)packet->prim;

      SBP_Prim_InitPacket* pBPData = (SBP_Prim_InitPacket*)BP_RB_Alloc(sizeof(SBP_Prim_InitPacket));
      pBPData->fogParam = BP_FogParam;
      pBPData->pers = cp->raise_pers;
      pBPData->screen = prim->screen;

      BP_RB_AddCommand(kCmd_Prim_InitPacket, (char*)pBPData);
   }

   {
      DG_PRIM2* prim = (DG_PRIM2*)packet->prim;
      SBP_PrimPacket_Render* pPrimPacket_Render = BP_RB_Alloc(sizeof(SBP_PrimPacket_Render));
      DG_TEX_BP* pTex = BP_RB_Alloc(sizeof(DG_TEX_BP));

      pPrimPacket_Render->tex_trans_alpha_data = prim->tex_trans.alpha.data;
      pPrimPacket_Render->tex_trans_clamp_data = prim->tex_trans.clamp.data;
      pPrimPacket_Render->flag = prim->flag;
      pPrimPacket_Render->fname = prim->fname;
      pPrimPacket_Render->BP_vertexBuffer = prim->BP_vertexBuffer;
      pPrimPacket_Render->BP_indexBuffer = prim->BP_indexBuffer;
      pPrimPacket_Render->BP_vertexCount = prim->BP_vertexCount;
      pPrimPacket_Render->BP_startIndex = packet->BP_startIndex;
      pPrimPacket_Render->BP_indexCount = packet->BP_indexCount;
      pPrimPacket_Render->BP_forceDisableAlphaTest = prim->BP_forceDisableAlphaTest;
      pPrimPacket_Render->BP_forceZWrite = prim->BP_forceZWrite;
      pPrimPacket_Render->BP_tex = pTex;
      BP_CopyDGTexToDGTexBP(pPrimPacket_Render->BP_tex, prim->BP_tex);
      pPrimPacket_Render->type = prim->type;
      BP_RB_AddCommand(kCmd_Prim_RenderPacket, (char*)pPrimPacket_Render);
   }

   return 0;
}
#endif

void DG_BP_WritePrimInitPacket( DG_CHANL *cp )
{
   {
      SBP_Prim_InitPacket* pBPData = (SBP_Prim_InitPacket*)BP_RB_Alloc(sizeof(SBP_Prim_InitPacket));

      pBPData->fogParam = BP_FogParam;
      pBPData->pers = cp->raise_pers;

      BP_RB_AddCommand(kCmd_Prim_InitPacket, (char*)pBPData);
   }
}

void DG_BP_WritePrimLocalParam( DG_PRIM2* prim )
{
   SBP_Prim_LocalParam* pBPData = (SBP_Prim_LocalParam*)BP_RB_Alloc(sizeof(SBP_Prim_LocalParam));
   
   pBPData->screen = prim->screen;
#if BP_PRIM_DEBUGGING
   pBPData->prim = prim;
#endif

   BP_RB_AddCommand(kCmd_Prim_LocalParam, (char*)pBPData);
}

int DG_BP_WritePrimPacksAndPendRender( DG_CHANL *cp, DG_PRIM2_PACKET *packet, int clone )
{
   DG_PRIM2* prim = (DG_PRIM2*)packet->prim;

   {
#if !BP_VITA
      DG_BP_WritePrimInitPacket( cp );
      DG_BP_WritePrimLocalParam( prim );
#endif
   }

   BP_Prim_Update_AddRenderWrapCommand( packet, clone );

   return 0;
}

//----------------------------------------------------------------------------

void DG_BP_WritePrimPacksFinalize()
{
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
static FMATRIX	_prim2_mat ;
static inline void _SetMatrix( FMATRIX *m )
{
#if 1 //BP_TODO - bring over bp_boundvu.c from MGS3
   BP_ASM_TODO_BOUND_BREAK;
   _prim2_mat = *m ;
#else
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	": : "r"(m) );
#endif
}

static void CalcSortZ( DG_PRIM2_PACKET *packet )
{
#if 1 //BP_ASM
   FVECTOR				tmp_verts, total_verts, v, *src ;
   register float		d ;
   int					i ;

   /* パケット中の座標の合計を求める */
   d = 1.0f / (float)packet->n_verts ;
   total_verts = DG_ZeroVector ;
   src = packet->pos_addr ;
   for ( i = 0 ; i < packet->n_verts ; i++ ){
      total_verts.vx += src->vx ;
      total_verts.vy += src->vy ;
      total_verts.vz += src->vz ;
      src++ ;
   }
   total_verts.vx *= d ;
   total_verts.vy *= d ;
   total_verts.vz *= d ;
   _sceVu0ApplyMatrix( &total_verts, &_prim2_mat, &total_verts );
   d = total_verts.vw ;
   packet->sort_z = DG_FTOI( d );

#else
	register float		d ;
	//_SetMatrix( packet->prim->world );
	d = 1.0f / (float)packet->n_verts ;
	asm volatile ("
		move			$12,%1
		move			$13,%2
		lqc2			vf1,0(%1)
		vmulax.xyzw		ACC,vf0,vf0		# 最初にACCを０で初期化
0:		vmaddax.xyzw	ACC,vf4,vf1
		vmadday.xyzw	ACC,vf5,vf1
		vmaddaz.xyzw	ACC,vf6,vf1
		vmaddaw.xyzw	ACC,vf7,vf0		# 全ての計算結果をＡＣＣ上で加算
		lqc2			vf1,16($12)		# 次の頂点読み出し
		addi			$13,$13,-1		# ループカウンタのデクリメント
		addi			$12,$12,16		# アドレスのインクリメント
		bgtz			$13,0b			# ループ
		nop								# 本当はこの場所に上のaddi $13,$13,-1を入れたい
		vmaddx.xyzw		vf1,vf0,vf0
		sqc2			vf1,0(%0)
	"::"r"(&TMP_VEC),"r"(packet->pos_addr), "r"(packet->n_verts):"$12","$13","memory");

	d *= TMP_VEC.vw ;
	packet->sort_z = DG_FTOI( d );
#endif
}

#if 1//BP - Optimize - copied from xprim.c.
static int BoundCheck( FMATRIX *mat, FVECTOR *bound )
{
	int		and_flag, or_flag, flag ;
	FVECTOR		verts, tmp_v ;
	float		w ;
	int			i ;

	and_flag = 0xffffffff ;
	or_flag = 0 ;

	for ( i = 8 ; i > 0 ; i-- ){
		verts.vx = ( i & 1 ) ? bound[0].vx : bound[1].vx ;
		verts.vy = ( i & 2 ) ? bound[0].vy : bound[1].vy ;
		verts.vz = ( i & 4 ) ? bound[0].vz : bound[1].vz ;
		verts.vw = 1.0F ;
		_sceVu0ApplyMatrix( &tmp_v, mat, &verts );
		w = DG_FABS( tmp_v.vw );

		/* クリップチェック */
		flag = 0 ;
		if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
		if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
		if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
		if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
		if ( tmp_v.vz >  w ) flag |= CLIP_Z0_FLAG;
		if ( tmp_v.vz < -w ) flag |= CLIP_Z1_FLAG;
		and_flag &= flag ;
		or_flag |= flag ;

	}
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */

	return ( 0 );	/* 完全画面内 */
}

#else

/* バウンディングチェック */
static int BoundCheck( FMATRIX *mat, FVECTOR *bound )
{
#if 1  //BP_ASM - brought over from MGS3/bp_boundvu.c
	//int		and_flag, or_flag, prim_over_flag ;
	int		and_flag ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

   // call the AltiVec optimized bounding box check.
//	vcallms		0x00			# バウンディングチェック
   BP_boundvu_BoundCheck( mat, bound + 0, bound + 1, &scrpad->scale );

//	"::"r"(mat),"r"(bound),"r"(&scrpad->scale) );

   // get the results.
   ee_ctc2_short( vi02, &and_flag );

	/* バウンディング中央値のＺ座標取得 */
	//SORT_Z = DG_FTOI(BOUNDS[0].vw) ;
	/* バウンディング判定 */
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	//if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
	//if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */

#else

	//int		and_flag, or_flag, prim_over_flag ;
	int		and_flag ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

	/* バウンディングチェックマイクロプログラム呼び出し */
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf1,0x00(%1)
	lqc2		vf2,0x10(%1)
	lqc2		vf3,0(%2)
	vcallms		0x00			# バウンディングチェック
	"::"r"(mat),"r"(bound),"r"(&scrpad->scale) );
	/* 計算結果の取得 */
	asm ("
	cfc2.i		%0,$2 /*vi2*/
	":"=r"(and_flag) );

	/* バウンディング中央値のＺ座標取得 */
	//SORT_Z = DG_FTOI(BOUNDS[0].vw) ;
	/* バウンディング判定 */
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	//if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
	//if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
#endif
	return ( 0 );	/* 完全画面内 */

}
#endif

/*----------------------------------------------------------------*/
void _DG_Prim2Chanl( DG_CHANL *cp, int which, int draw_mode )
{
	DG_OBJ_QUEUE	*queue ;
	DG_OBJ_BUFFER	*obj_buff ;
	DG_PRIM2		**pque, *prim2 ;
	DG_PRIM2_PACKET	*packet ;
	int		i, j, gid, c_gid, invisible_flag ;
	ScrpadWork		*scrpad = SCRPAD_ADDR ;
#ifdef DEBUG_MODE
	int				pos_init_error_flag ;
#endif

#if 0 && BP_PRIM_DEBUGGING && BP_VITA
   extern int gDumpingPrims;
   extern int gDumpPrims;
   if (gDumpingPrims)
      gDumpingPrims = 0;
   if (gDumpPrims)
   {
      gDumpingPrims = 1;
      gDumpPrims = 0;
   }
#endif

   BP_Debug_PushCPUMarker( "Prim2" );
	MARK( "prim2.c" );
	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->prim2_buffer ;
	/*
		キュー数のチェック
	*/
	if ( ( i = obj_buff->n_queue ) == 0 ) return ;

   BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;

	/* ＶＵ０マイクロプログラム準備 */
	DG_StartBoundingCheckSupport();

	/*
		スクラッチパッド上のＯＴ初期化
	*/
	GV_ZeroMemory( scrpad->ot3, sizeof(int)*64*3 );

	/*
		パース変換パラメータ／クリッピング領域を設定
	*/
	pque = (DG_PRIM2**)obj_buff->queue ;
	c_gid = cp->group_id ;

	*PERS_MAT = cp->raise_pers ;
	*EYE_PERS = cp->raise_eye_pers ;
	*EYE_PERS2 = cp->raise_eye_pers2 ;
	SCALE->vx = cp->width / 2 ;
	SCALE->vy = cp->height / 2 ;
	invisible_flag = DG_PRIM2_INVISIBLE0 << cp->chanl_num ;
#ifdef LIBDG_PERFORMANCE
	scrpad->prim_poly = DG_PerformanceData.prim_poly ;
	scrpad->prim_line = DG_PerformanceData.prim_line ;
	scrpad->prim_sprt = DG_PerformanceData.prim_sprt ;
	scrpad->prim_rsprt = DG_PerformanceData.prim_rsprt ;
#endif

	for ( ; i > 0 ; i--, pque++ ){
		prim2 = *pque ;
		if ( prim2->flag & invisible_flag ) continue ;

		/* 表示グループチェック */
		if ( ( gid = prim2->group_id ) != 0 && !( gid & c_gid ) ) continue ;

		/* プリミティブ設置マトリクス確定 */
		if ( prim2->root != NULL ) prim2->as_world = *( prim2->root ) ;
		scrpad->world = prim2->as_world ;

		/* 変換基準チェック */
		if ( prim2->flag & DG_PRIM2_ON_CAMERA )
      {
			/* カメラ相対 */
			scrpad->screen = scrpad->pers;

#if 1 //BP_XBOX
         prim2->screen = DG_UnitMatrix ;
#endif
		} 
      else 
      {
			/* 通常計算 */
			_sceVu0MulMatrix( &scrpad->screen, &scrpad->eye_pers, &scrpad->world );
#if 1 //BP_XBOX
         _sceVu0MulMatrix( &prim2->screen, &cp->eye_inv, &scrpad->world );
#endif
		}

      if ( draw_mode == 0 ){
         if ( prim2->flag & DG_PRIM2_NOMSAA ) continue ;
      } else if ( draw_mode == 1 ){
         if ( !( prim2->flag & DG_PRIM2_NOMSAA ) ) continue ;
      }

#if 0 //BP_PS2
		prim2->screen = scrpad->screen ;
		prim2->prim_param[ which ].screen = scrpad->screen ;
#endif

		_SetMatrix( &scrpad->screen );
		/* バウンディングチェック */
		if ( prim2->flag & DG_PRIM2_BOUNDCHECK ){
			if ( BoundCheck( &scrpad->screen, &prim2->bound_min ) == 2 ){
				//printf("skip\n");
				continue ;
			}
		}
#if BP_ENABLE_TESTNODE
      if ( !BP_Debug_EvalTestNode() ) continue;
#endif
      // Send BP render command to generate buffers for prim
      {
         //BP_Prim_Render(prim2);
         bp_prim_post_ult( prim2 );

      }

		packet = prim2->packet[ prim2->buffer_clock ];
#ifdef LIBDG_PERFORMANCE
		switch ( packet->type ){
		  case DG_PRIM2_LINE:
			scrpad->trg_prim_info = &scrpad->prim_line ;
			break ;
		  case DG_PRIM2_POLY:
		  case DG_PRIM2_CULLPOLY:
			scrpad->trg_prim_info = &scrpad->prim_poly ;
			break ;
		  case DG_PRIM2_SPRT:
			scrpad->trg_prim_info = &scrpad->prim_sprt ;
			break ;
		  case DG_PRIM2_RSPRT:
			scrpad->trg_prim_info = &scrpad->prim_rsprt ;
			break ;
		}
		scrpad->trg_prim_info->n_obj++ ;
		scrpad->trg_prim_info->n_packs += prim2->n_prims ;
#endif
#ifdef DEBUG_MODE
		pos_init_error_flag = 0 ;	/* 未初期化チェック */
#endif
		for ( j = prim2->n_prims ; j > 0 ; j--, packet++ ){
			/* ローカルフラグチェック */
			if ( packet->flag & invisible_flag ) continue ;
#ifdef DEBUG_MODE
			if ( 0xABCDEF01ABCDEF01LL == *(u_long64*)packet->pos_addr ){
				GV_ERROR( GV_ERROR_NO_INIT_PRIM2 );	/* 未初期化頂点使用の検出 */
				pos_init_error_flag = 1 ;
			}
#endif
			/* ソート用Ｚ値計算 */
			CalcSortZ( packet );
			/* ＯＴに接続 */
			MakeSortListPrim( packet, prim2->raise );
#ifdef LIBDG_PERFORMANCE
			scrpad->trg_prim_info->n_verts += packet->n_verts ;
#endif
		}
	}

   bp_prim_flush_ult();

	/* 全プリミティブパケットのソートを行なう */
	SortListPrims();

   BP_Debug_PopCPUMarker( "Prim2" );

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.prim_poly = scrpad->prim_poly ;
	DG_PerformanceData.prim_line = scrpad->prim_line ;
	DG_PerformanceData.prim_sprt = scrpad->prim_sprt ;
	DG_PerformanceData.prim_rsprt = scrpad->prim_rsprt ;
#endif
}

void DG_Prim2Chanl( DG_CHANL *cp, int which )
{
   _DG_Prim2Chanl( cp, which, 0 );
}

void DG_Prim2FirstChanl( DG_CHANL *cp, int which )
{
   _DG_Prim2Chanl( cp, which, 1 );
}

/*----------------------------------------------------------------*/
DG_PRIM2 *DG_MakePrim2( int flag, int n_prims, int n_verts, int chanl )
{
	DG_PRIM2		*prim2 ;
	DG_PRIM2_PACKET	*packet ;
	FVECTOR			*pos ;
	void			*uvrgb ;
	int			type, size, n_buffer, uvrgb_size, packet_flag ;
	int			i, j ;
	u_long64		prim ;

	packet_flag = flag & 0xffff ;
	flag |= DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 ;
	type = flag & DG_PRIM2_TYPEMASK ;

	if ( flag & DG_PRIM2_SINGLEBUFF )	n_buffer = 1 ;
	else								n_buffer = 2 ;
	if ( type < DG_PRIM2_SPRT )	uvrgb_size = sizeof(DG_PRIM2_UVRGB) ;
	else						uvrgb_size = sizeof(DG_PRIM2_UVRGBWH) ;

	/* メモリサイズの計算 */
	if ( !( flag & DG_PRIM2_NOBUFFER ) ){
		size = sizeof(DG_PRIM2) +
		  sizeof(DG_PRIM2_PACKET) * n_prims * n_buffer +
			sizeof(FVECTOR) * n_prims * n_verts * n_buffer +
			  uvrgb_size * n_prims * n_verts * n_buffer ;
	} else {
		/* 頂点用バッファを確保しない */
		size = sizeof(DG_PRIM2) +
		  sizeof(DG_PRIM2_PACKET) * n_prims * n_buffer ;
	}

	if ( ( prim2 = GV_Malloc( size ) ) == NULL ) return ( NULL ) ;
	GV_ZeroMemory( prim2, sizeof(DG_PRIM2) );
	//GV_ZeroMemory( prim2, size );	/* 原因不明不具合一時的回避の為 */

	/* 構造体の初期化 */
	prim2->flag = flag ;
	prim2->as_world = DG_UnitMatrix ;
	prim2->group_id = 0 ;
	prim2->chanl = chanl ;
	prim2->type = type ;
	prim2->n_prims = n_prims ;
	prim2->packet_verts = n_verts ;

	/* ポインタの設定 */
	packet = (DG_PRIM2_PACKET*)&prim2[1] ;
	prim2->packet[ 0 ] = &packet[ n_prims * ( 0 ) ] ;
	prim2->packet[ 1 ] = &packet[ n_prims * ( n_buffer - 1 ) ] ;

	if ( !( flag & DG_PRIM2_NOBUFFER ) )
   {
		pos = (FVECTOR*)&packet[ n_prims * ( n_buffer ) ] ;
		prim2->pos[ 0 ] = &pos[ n_prims * n_verts * ( 0 ) ] ;
		prim2->pos[ 1 ] = &pos[ n_prims * n_verts * ( n_buffer - 1 ) ] ;

		uvrgb = (void*)&pos[ n_prims * n_verts * ( n_buffer ) ] ;
		prim2->uvrgb[ 0 ] = (void*)( (int)uvrgb +  uvrgb_size * n_prims * n_verts * ( 0 ) ) ; 
		prim2->uvrgb[ 1 ] = (void*)( (int)uvrgb +  uvrgb_size * n_prims * n_verts * ( n_buffer - 1 ) ) ; 

		/* パケットデータの初期化 */
		for ( i = 0 ; i < 2 ; i++ ){
			packet = prim2->packet[ i ] ;
			pos = prim2->pos[ i ] ;
			uvrgb = prim2->uvrgb[ i ] ;
			for ( j = 0 ; j < n_prims ; j++ ){
				packet->prim = prim2 ;
				packet->type = type ;
				packet->flag = packet_flag ;
				packet->n_verts = n_verts ;
				packet->pos_addr = pos ;
				packet->uvrgb_addr = uvrgb ;
            // AS addition for threaded EndFrame
            packet->BP_startIndex = 0;
            packet->BP_indexCount = 0;
				packet++ ;
				pos += n_verts ;
				uvrgb = (void*)( (int)uvrgb + uvrgb_size * n_verts ) ;
			}
		}
	}

	/* 描画環境の初期化（テクスチャ以外） */
	//prim = SCE_GS_SET_PRIM( 0, 1, 1, USE_FOG, USE_ABE, USE_AA1, 0, 0, 0) ;
	prim = SCE_GS_SET_PRIM( 0,
						   ( ( flag & DG_PRIM2_SHADE ) != 0 ),
						   ( ( flag & DG_PRIM2_TEX ) != 0 ),
						   ( ( flag & DG_PRIM2_FOG ) != 0 ),
						   ( ( flag & DG_PRIM2_ALPHA ) != 0 ),
						   ( ( flag & DG_PRIM2_ANTIALIASING ) != 0 ),
						   0, 0, 0) ;
	{
		DG_PRIM2_PARAM	*param ;
		param = &prim2->prim_param[0] ;

		param->n_verts = n_verts ;
		param->flag = 0 ;

		switch ( type ){
		  case DG_PRIM2_LINE:
			param->giftag_normal.tag =
			  SCE_GIF_SET_TAG( n_verts, 1, 1, prim | SCE_GS_PRIM_LINESTRIP, SCE_GIF_PACKED, 3) ;
			param->giftag_normal.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;
			param->giftag_clip.tag =
			  SCE_GIF_SET_TAG( n_verts, 1, 1, prim | SCE_GS_PRIM_LINE, SCE_GIF_PACKED, 3) ;
			param->giftag_clip.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;
			break ;
		  case DG_PRIM2_POLY:
		  case DG_PRIM2_CULLPOLY:
			param->giftag_normal.tag =
			  SCE_GIF_SET_TAG( n_verts, 1, 1, prim | SCE_GS_PRIM_TRISTRIP, SCE_GIF_PACKED, 3) ;
			param->giftag_normal.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;
			param->giftag_clip.tag =
			  SCE_GIF_SET_TAG( n_verts, 1, 1, prim | SCE_GS_PRIM_TRIFAN, SCE_GIF_PACKED, 3) ;
			param->giftag_clip.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;
			break ;
		  case DG_PRIM2_SPRT:
			prim &= ~SCE_GS_SET_PRIM( 0, 1, 0, 0, 0, 0, 0, 0, 0) ;/* グロー禁止 */
			param->giftag_normal.tag =
			  SCE_GIF_SET_TAG( n_verts, 1, 1, prim | SCE_GS_PRIM_SPRITE, SCE_GIF_PACKED, 6) ;
			param->giftag_normal.regs = GS_REGS_6( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2,
												   GS_REGS_STQ, GS_REGS_NOP, GS_REGS_XYZF2 ) ;
			param->giftag_clip.tag =
			  SCE_GIF_SET_TAG( n_verts, 1, 1, prim | SCE_GS_PRIM_SPRITE, SCE_GIF_PACKED, 6) ;
			param->giftag_clip.regs = GS_REGS_6( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2,
												 GS_REGS_STQ, GS_REGS_NOP, GS_REGS_XYZF2 ) ;
			break ;
		  case DG_PRIM2_RSPRT:
			prim &= ~SCE_GS_SET_PRIM( 0, 1, 0, 0, 0, 0, 0, 0, 0) ;/* グロー禁止 */
			param->giftag_normal.tag =
			  SCE_GIF_SET_TAG( n_verts*4, 1, 1, prim | SCE_GS_PRIM_TRISTRIP, SCE_GIF_PACKED, 3) ;
			param->giftag_normal.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;
			param->giftag_clip.tag =
			  SCE_GIF_SET_TAG( n_verts*4, 1, 1, prim | SCE_GS_PRIM_TRIFAN, SCE_GIF_PACKED, 3) ;
			param->giftag_clip.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;
			break ;
		  default:
			ASSERT(0);
			break ;
		}
		prim2->prim_param[1] = *param ;
	}
	/* デフォルトテクスチャ設定（ＤＭＡパケット初期化のため） */
	{
		//extern DG_TEX	DG_TexBuffer[];
		//prim2->tex_trans = DG_TexBuffer[0].tex_trans ;
		extern DG_TEX	DG_DefaultTexture ;
		prim2->tex_trans = DG_DefaultTexture.tex_trans ;
      prim2->BP_tex = &DG_DefaultTexture;
	}

	return prim2;
}

void DG_FreePrim2( DG_PRIM2 *prim )
{
	if ( prim == NULL ) return ;
	GV_DelayedFree( prim );
}
/*----------------------------------------------------------------*/
	/*
		プリミティブのテクスチャを設定する
	*/
void DG_ConfigPrim2Tex( DG_PRIM2 *prim, DG_TEX *tex )
{
	if ( prim == NULL ) return ;
	prim->tex_trans = tex->tex_trans ;
   prim->BP_tex = tex;
}

	/*
		DG_PRIM2_NOBUFFERフラグ付きで初期化したプリミティブに対し
		ユーザーの指定したメモリをバッファに割り当てる
	*/
void DG_SetPrim2Buffer( DG_PRIM2 *prim2, FVECTOR *pos0, FVECTOR *pos1, void *uvrgb0, void *uvrgb1 )
{
	int		i, j, n_verts, uvrgb_size, type ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	prim2->pos[0] = pos0 ;
	prim2->pos[1] = pos1 ;
	prim2->uvrgb[0] = uvrgb0 ;
	prim2->uvrgb[1] = uvrgb1 ;
	/* パケットデータの初期化 */
	n_verts = prim2->packet_verts ;
	type = prim2->flag & DG_PRIM2_TYPEMASK ;
	if ( type < DG_PRIM2_SPRT )	uvrgb_size = sizeof(DG_PRIM2_UVRGB) ;
	else						uvrgb_size = sizeof(DG_PRIM2_UVRGBWH) ;
	for ( i = 0 ; i < 2 ; i++ ){
		DG_PRIM2_PACKET		*packet ;
		packet = prim2->packet[ i ] ;
		pos = prim2->pos[ i ] ;
		uvrgb = prim2->uvrgb[ i ] ;
		for ( j = 0 ; j < prim2->n_prims ; j++ ){
			packet->prim = prim2 ;
			packet->type = prim2->type ;
			packet->flag = prim2->flag & 0xffff ;
			packet->n_verts = n_verts ;
			packet->pos_addr = pos ;
			packet->uvrgb_addr = uvrgb ;
			packet++ ;
			pos += n_verts ;
			uvrgb = (void*)( (int)uvrgb + uvrgb_size * n_verts ) ;
		}
	}
}

/*----------------------------------------------------------------*/

#ifdef BP_MAKEPRIM_DEBUG_INFO

DG_PRIM2 *DG_MakePrim2D( int flag, int n_prims, int n_verts, int chanl, char *fname )
{
	DG_PRIM2	*prim2 ;
	prim2 = DG_MakePrim2( flag, n_prims, n_verts, chanl );
	if ( prim2 != NULL ) 
      prim2->fname = fname ;
	return ( prim2 );
}

#include "gameheader.h"
//#include "g_macro.h"
	/*
		機能追加型 DG_MakePrim2()
	*/
DG_PRIM2	*GM_MakePrim2ChanlD( int type, int n_prims, int n_verts, int chanl, char *fname )
{
	DG_PRIM2		*prim ;
	extern int GM_CurrentMap ;

	prim = DG_MakePrim2D( type, n_prims, n_verts, chanl, fname ) ;
	if ( prim != NULL ) {
		if ( DG_QueuePrim2( prim ) == -1 ){
			//DG_FreePrim2( prim );
			//return ( NULL );
		}
		if ( chanl != DG_CHANL_MENU ) GM_GroupPrim2( prim, GM_CurrentMap ) ;
	}
	return prim ;
}

	/*
		機能追加型 DG_MakePrim2()
		（３Ｄチャンネル固定）
	*/
DG_PRIM2	*GM_MakePrim2D( int type, int n_prims, int n_verts, char *fname )
{
	return GM_MakePrim2ChanlD( type, n_prims, n_verts, 0, fname ) ;
}

#endif
