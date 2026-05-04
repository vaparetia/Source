//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	opack.c
	モデル用パケット処理ルーチン

	1999/07/07 K.Takabe
	$Id: opack.c,v 1.1.1.3 2002/11/19 11:42:16 Yoshizawa1 Exp $

*/
/*

	このファイルはまだ未整理なので注意！

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"

extern void *DG_WritePacket_CoverShadowModelGifInit( void *addr );

#define USE_FOG	1
#define USE_ABE	1
#define USE_AA1	0


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

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	(sizeof(_v)/sizeof(int))
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))


/*----------------------------------------------------------------*/

#if 0
/* 描画パラメータ初期化ＧＩＦパケット（シャドーボリューム用設定） */
static ALIGN16_PRE u_long64 init_gif_data3[] ALIGN16_POST = { 
	SCE_GIF_SET_TAG(1, 1, 0, 0, 0, 1),
	GS_REGS_AD,
	SCE_GS_SET_ZBUF(ZBUFFER_PAGE()/2048, 1, 1),
	SCE_GS_ZBUF_1
};
#endif


/* マイクロプログラムの使用する固定変数群 */
typedef struct _vu1_work {
	FVECTOR	clamp_min ;		/* クランプ処理用最小値（現在未使用） */
	FVECTOR	clamp_max ;		/* クランプ処理用最大値（現在未使用） */
	float	fog_param1, fog_param2, fog_param3, fog_param4 ;	/* フォグパラメータ */
	float	color_clip, poly_alpha, specular_mul, specular_clip ;	/* 各種定数 */
	FVECTOR	scale ;			/* プリミティブ座標算出用スケール値（xyz=pvec*scale+offset） */
	FVECTOR	offset ;		/* プリミティブ座標算出用オフセット */
	float	param1, param2, param3, param4 ;					/* 各種定数２ */
	FVECTOR	aspect ;		/* プリミティブ用アスペクト比設定 */
	FMATRIX	pers ;			/* 透視変換マトリクス */
} Vu1Work ;

static Vu1Work	default_vu1_work = {
	/*.clamp_min =*/ {/*.vx =*/ 0.0f, /*.vy = */0.0f, /*.vz = */0.0f, /*.vw = */-1.0f },
	/*.clamp_max =*/ {4095.0f, 4095.0f, 65536.0f * 256.0f, 1.0f },
	/*.fog_param1 = */0, /*.fog_param2 = */0, /*.fog_param3 = */0, /*.fog_param4 = */0,
	/*.color_clip = */255.0f, /*.poly_alpha = */128.0f, /*.specular_mul = */0.0f, /*.specular_clip = */0.8f, 
	/*.scale = */{ DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f },
	/*.offset = */{ 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f },
	/*.param1 = */1.0f / 4096.0f, /*.param2 = */0.5f, /*.param3 = */0.0f, /*.param4 = */0.0f, 
	/*.aspect = */{0.0f, 0.0f, 1.0f, 1.0f },
   /*.pers =*/ { 0 }
};


/* マイクロプログラム読み込みタグ */
extern qword Vu1DrawObject0 ;
extern qword Vu1DrawObject1 ;
#define SELECT_VU1_PROGRAM Vu1DrawObject1
/* マイクロプログラム実行アドレスリスト */
extern int Vu1DrawObject0_Func[] ;
extern int Vu1DrawObject1_Func[] ;
#define SELECT_VU1_PROGRAM_FUNC Vu1DrawObject1_Func

#if 1
#define DEBUG_CODE( _n ) {\
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );\
	tag[2] = SCE_VIF1_SET_NOP( 0 ) ;\
	tag[3] = SCE_VIF1_SET_MARK( (_n), 0 ) ;\
	tag +=4 ;\
					  }
#else
#define DEBUG_CODE( _n ) /* _n */
#endif

/*----------------------------------------------------------------*/

	/*
		オブジェクトパケット用初期化パケットデータの書き出し
	*/
int DG_WriteObjsPacketInit( void *tag_addr, DG_CHANL *cp, int flag )
{
#if 0 //BP_PS2
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/*
		初期化データを設定
	*/
	tag = DG_WritePacket_VIF1Init( tag );

	/*
		描画用初期化パケットの登録
	*/
	tag = DG_WritePacket_DefaultModelGifInit( tag );

	/*
		マイクロプログラムロード設定パケットの登録
	*/
	tag = DG_WritePacket_Vu1ModelProgEx( tag, flag );

	/*
		基本パラメータ設定パケットの登録
	*/
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag++ ;

	/*
		パラメータの書き込み
	*/
	src = (u_long128*)&default_vu1_work ;
	dst = (u_long128*)tag ;
#if 1 //BP_PS2
   memcpy( dst, src, sizeof( Vu1Work ) );
#else
   for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- )
   {
      *dst++ = *src++ ;
   }
#endif

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = cp->width / 2 ;
	vu1_work->scale.vy = cp->height / 2 ;
	vu1_work->fog_param1 = DG_FogParam1 ;
	vu1_work->fog_param2 = DG_FogParam2 ;
	vu1_work->pers = cp->eye_pers ;
	vu1_work->aspect.vx = cp->screen * ASPECT_X() ;
	vu1_work->aspect.vy = cp->screen * ASPECT_Y() * cp->width / cp->height ;
	tag = (DG_DMATAG*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 /* 4 + 8 */ );
#else
   SBP_OBJ_InitPacket* pBPData = (SBP_OBJ_InitPacket*)BP_RB_Alloc(sizeof(SBP_OBJ_InitPacket));
   memcpy(pBPData, &default_vu1_work, sizeof(Vu1Work));

   pBPData->scale.vx = cp->width / 2 ;
   pBPData->scale.vy = cp->height / 2 ;
   pBPData->fog_param1 = DG_FogParam1 ;
   pBPData->fog_param2 = DG_FogParam2 ;
   pBPData->pers = cp->eye_pers ;
   pBPData->aspect.vx = cp->screen * ASPECT_X() ;
   pBPData->aspect.vy = cp->screen * ASPECT_Y() * cp->width / cp->height ;

   BP_RB_AddCommand(kCmd_Obj_InitPacket, (char*)pBPData);

   return 0;
#endif
}

	/*
		オブジェクトパケット用初期化パケットデータの書き出し（半透明オブジェクト用）
	*/
int DG_WriteObjsPacketInit2( void *tag_addr, DG_CHANL *cp )
{
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/*
		初期化データを設定
	*/
	tag = DG_WritePacket_VIF1Init( tag );

	/*
		描画用初期化パケットの登録
	*/
	tag = DG_WritePacket_DefaultSemiTransModelGifInit( tag );

	/*
		マイクロプログラムロード設定パケットの登録
	*/
	tag = DG_WritePacket_Vu1ModelProgEx( tag, 0 );

	/*
		基本パラメータ設定パケットの登録
	*/
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag++ ;

	/*
		パラメータの書き込み
	*/
	src = (u_long128*)&default_vu1_work ;
	dst = (u_long128*)tag ;
#if 1 //BP_PS2
   memcpy( dst, src, sizeof( Vu1Work ) );
#else
	for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- )
   {
      *dst++ = *src++ ;
   }
#endif

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = cp->width / 2 ;
	vu1_work->scale.vy = cp->height / 2 ;
	vu1_work->fog_param1 = DG_FogParam1 ;
	vu1_work->fog_param2 = DG_FogParam2 ;
	vu1_work->pers = cp->eye_pers ;
	vu1_work->aspect.vx = cp->screen * ASPECT_X() ;
	vu1_work->aspect.vy = cp->screen * ASPECT_Y() * cp->width / cp->height ;
	tag = (DG_DMATAG*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 /* 4 + 8 */ );
}


	/*
		オブジェクトパケット用初期化パケットデータの書き出し（影生成用）
	*/
int DG_WriteObjsPacketInit_MakeShadow( void *tag_addr, DG_CHANL *cp )
{
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/* 描画用初期化パケットの登録 */
	tag = DG_WritePacket_MakeShadowModelGifInit( tag );

	/* 基本パラメータ設定パケットの登録 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag++ ;

	/* パラメータの書き込み */
	src = (u_long128*)&default_vu1_work ;
	dst = (u_long128*)tag ;
#if 1 //BP_PS2
   memcpy( dst, src, sizeof( Vu1Work ) );
#else
   for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- )
   {
      *dst++ = *src++ ;
   }
#endif

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = 256 / 2 ;
	vu1_work->scale.vy = 256 / 2 ;
	vu1_work->fog_param1 = DG_FogParam1 ;
	vu1_work->fog_param2 = DG_FogParam2 ;
	vu1_work->pers = cp->eye_pers ;
	vu1_work->poly_alpha = 128.0f ;
	tag = (DG_DMATAG*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 );
}

	/*
		オブジェクトパケット用初期化パケットデータの書き出し（スポット生成用）
	*/
int DG_WriteObjsPacketInit_MakeSpot( void *tag_addr, DG_CHANL *cp, int color, void *img_addr, int type )
{
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/* 前の処理が完了するまでウェイトを入れる */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	//tag->vifcode[1] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	tag++ ;

	if ( type == 0 ){
		/* 描画用初期化パケットの登録（スポットライト影用） */
		tag = DG_WritePacket_MakeSpotModelGifInit( tag );
		/* スポットライト描画用 */
		if ( img_addr == NULL ){
			/* プリミティブによるサークルグラデーション */
			tag = DG_WritePacket_Circle( tag, color );
		} else {
			/* イメージデータのアドレスが指定されていればスポットライトの背景として登録する */
			tag = DG_WritePacket_TransSpotImage( tag, img_addr, color );
		}
	} else {
		/* 描画用初期化パケットの登録（黒影用） */
		tag = DG_WritePacket_MakeSpotShadowModelGifInit( tag );
	}

	/* 基本パラメータ設定パケットの登録 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag++ ;

	/* パラメータの書き込み */
	src = (u_long128*)&default_vu1_work ;
	dst = (u_long128*)tag ;
#if 1 //BP_PS2
   memcpy( dst, src, sizeof( Vu1Work ) );
#else
   for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- )
   {
      *dst++ = *src++ ;
   }
#endif

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = 128.0f ;
	vu1_work->scale.vy = 128.0f ;
	vu1_work->fog_param1 = DG_FogParam1 ;
	vu1_work->fog_param2 = DG_FogParam2 ;
	vu1_work->pers = cp->eye_pers ;
	//vu1_work->poly_alpha = 0.0f ;	/* ブレンドの場合 */
	vu1_work->poly_alpha = 128.0f ;	/* 加算の場合 */
	tag = (DG_DMATAG*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 );
}

	/*
		オブジェクトパケット用初期化パケットデータの書き出し（影＆スポット投影用）
	*/
int DG_WriteObjsPacketInit_WriteShadow( void *tag_addr, DG_CHANL *cp, float fog_param1, float fog_param2 )
{
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/* 前の処理が完了するまでウェイトを入れる */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	//tag->vifcode[1] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	tag++ ;

	/* テクスチャぼかし */
	tag = DG_WritePacket_MakeSoftTexture( tag );

	/* 描画環境の復元 */
	tag = DG_PopDefaultDrawEnv( cp, tag );

	/* 描画用初期化パケットの登録 */
	tag = DG_WritePacket_WriteShadowModelGifInit( tag );

	/* 基本パラメータ設定パケットの登録 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag++ ;

	/* パラメータの書き込み */
	src = (u_long128*)&default_vu1_work ;
	dst = (u_long128*)tag ;
#if 1 //BP_PS2
   memcpy( dst, src, sizeof( Vu1Work ) );
#else
   for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- )
   {
      *dst++ = *src++ ;
   }
#endif

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = cp->width / 2 ;
	vu1_work->scale.vy = cp->height / 2 ;
	vu1_work->fog_param1 = fog_param1 ;
	vu1_work->fog_param2 = fog_param2 ;
	vu1_work->pers = cp->eye_pers ;
	tag = (DG_DMATAG*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 );
}

	/*
		オブジェクトパケット用初期化パケットデータの書き出し（影及びスポットライト描画マスク生成用）
	*/
int DG_WriteObjsPacketInit_MakeShadowMask( void *tag_addr, DG_CHANL *cp, int color )
{
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/* 前の処理が完了するまでウェイトを入れる */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	//tag->vifcode[1] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	tag++ ;

	/* 描画環境の復元 */
	tag = DG_PopDefaultDrawEnv( cp, tag );

	/* 描画用初期化パケットの登録 */
	tag = DG_WritePacket_MakeShadowMaskGifInit( tag );

	/* 基本パラメータ設定パケットの登録 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag++ ;

	/* パラメータの書き込み */
	src = (u_long128*)&default_vu1_work ;
	dst = (u_long128*)tag ;
#if 1 //BP_PS2
   memcpy( dst, src, sizeof( Vu1Work ) );
#else
   for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- )
   {
      *dst++ = *src++ ;
   }
#endif

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = 128.0f ;
	vu1_work->scale.vy = 128.0f ;
	vu1_work->fog_param1 = DG_FogParam1 ;
	vu1_work->fog_param2 = DG_FogParam2 ;
	vu1_work->pers = cp->eye_pers ;
	vu1_work->poly_alpha = 128.0f ;
	tag = (DG_DMATAG*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 );
}


	/*
		オブジェクトパケット用初期化パケットデータの書き出し（影生成用）
	*/
int DG_WriteObjsPacketInit_CoverShadow( void *tag_addr, DG_CHANL *cp )
{
	u_long128		*src, *dst ;
	DG_DMATAG		*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (DG_DMATAG*)tag_addr ;

	/* 描画用初期化パケットの登録 */
	tag = DG_WritePacket_CoverShadowModelGifInit( tag );

	/* 基本パラメータ設定パケットの登録 */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag->vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag++ ;

	/* パラメータの書き込み */
	src = (u_long128*)&default_vu1_work ;
	dst = (u_long128*)tag ;
#if 1 //BP_PS2
   memcpy( dst, src, sizeof( Vu1Work ) );
#else
   for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- )
   {
      *dst++ = *src++ ;
   }
#endif

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = cp->width / 2 ;
	vu1_work->scale.vy = cp->height / 2 ;
	vu1_work->fog_param1 = DG_FogParam1 ;
	vu1_work->fog_param2 = DG_FogParam2 ;
	vu1_work->pers = cp->eye_pers ;
	vu1_work->aspect.vx = cp->screen * ASPECT_X() ;
	vu1_work->aspect.vy = cp->screen * ASPECT_Y() * cp->width / cp->height ;
	vu1_work->poly_alpha = 128.0f ;
	tag = (DG_DMATAG*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 );
}

	/*
		オブジェクトパケット用終端パケット書き込み
	*/
int DG_WriteObjsPacketEnd( void *tag_addr )
{
	unsigned int	*tag ;	

	tag = (unsigned int*)tag_addr ;

	/* 描画終了待ちコードを設定 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, 1 );
	tag[2] = SCE_VIF1_SET_MSKPATH3(0x8000,0) ;
	tag[3] = SCE_VIF1_SET_FLUSHA(0) ;
	tag[4] = SCE_VIF1_SET_MSKPATH3(0,0) ;
	tag[5] = SCE_VIF1_SET_NOP(0) ;
	tag[6] = SCE_VIF1_SET_NOP(0) ;
	tag[7] = SCE_VIF1_SET_NOP(0) ;
	tag += 8 ;

	return ( 2 );

}
/*----------------------------------------------------------------*/

	/*
		テクスチャ入れ替えＤＭＡパケット書き込み
	*/
int DG_WriteTextureChangePacks( void *tag_addr, void *tex_packet )
{
	/* オブジェクトパケットのＤＭＡデータを接続する */
	DG_DMATAG		*tag ;
	static ALIGN16_PRE u_long64 tex_flush_packet[6] ALIGN16_POST = { 
		SCE_GIF_SET_TAG(2, 1, 0, 0, 0, 1),
		GS_REGS_1(GS_REGS_AD),
		0,
		SCE_GS_TEXFLUSH,/*0x3f*/
		SCE_GS_SET_TEX2( SCE_GS_PSMT8, 0, 0, 0, 0, 0x2 ),
		SCE_GS_TEX2_1
	};

	tag = (DG_DMATAG*)tag_addr ;
	/*  */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
	tag->addr = tex_packet ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	tag++ ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 3 );
	tag->addr = tex_flush_packet ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSH(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(3,0) ;
	tag++ ;

	return ( 2 );

}

	/*
		テクスチャ入れ替えＤＭＡパケット書き込み
		（テクスチャ部分差し替え対応版）
	*/
int DG_WriteTextureChangePacks2( void *tag_addr, DG_TEXTURE_LIST *tex_list, int which )
{
	/* オブジェクトパケットのＤＭＡデータを接続する */
	static ALIGN16_PRE u_long64 tex_flush_packet[6] ALIGN16_POST = { 
		SCE_GIF_SET_TAG(2, 1, 0, 0, 0, 1),
		GS_REGS_1(GS_REGS_AD),
		0,
		SCE_GS_TEXFLUSH,/*0x3f*/
		SCE_GS_SET_TEX2( SCE_GS_PSMT8, 0, 0, 0, 0, 0x2 ),
		SCE_GS_TEX2_1
	};
	DG_DMATAG		*dmatag ;
	int				size = 2 ;
	void			*tex_packet ;

	dmatag = (DG_DMATAG*)tag_addr ;
	tex_packet = &tex_list->tex_packet[ which ] ;

	/* メインテクスチャ転送 */
    dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
	dmatag->addr = tex_packet ;
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	dmatag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	dmatag++ ;

	/* 差し替えパケットのチェック */
	if ( tex_list->move_link != NULL ){
		DG_TEX_MOVEREPLACE	*replace ;
		replace = tex_list->move_link ;
		while ( replace != NULL ){
			dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
			dmatag->addr = replace ;
			dmatag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
			dmatag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
			dmatag++ ;
			size++ ;
			replace = replace->next ;
		}
	}
	if ( tex_list->load_link != NULL ){
		DG_TEX_LOADREPLACE	*replace ;
		replace = tex_list->load_link ;
		while ( replace != NULL ){
			dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
			dmatag->addr = replace ;
			dmatag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
			dmatag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
			dmatag++ ;
			size++ ;
			replace = replace->next ;
		}
	}

	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 3 );
	dmatag->addr = tex_flush_packet;
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSH(0) ;
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT(3,0) ;
	dmatag++ ;

	return ( size );

}

/*----------------------------------------------------------------*/
	/*
		フォグ変更ＤＭＡパケット書き込み
	*/
int DG_WriteFogChangePacks( void *tag_addr, int fogcol )
{
	DG_DMATAG	*dmatag ;
	DG_GIFTAG	*giftag ;
	DG_GSREG	*gsreg ;

	dmatag = (DG_DMATAG*)tag_addr ;
	/*  */
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 );
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT( 2, 0 ) ;
	dmatag++ ;
	/*  */
	giftag = (DG_GIFTAG*)dmatag ;
	giftag->tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, 1);
	giftag->regs = 0x0e ;
	giftag++ ;
	/*  */
	gsreg = (DG_GSREG*)giftag ;
	gsreg->reg = SCE_GS_FOGCOL ;
	gsreg->data = fogcol ;
	gsreg++ ;

	return ( 3 );

}
/*----------------------------------------------------------------*/
	/*
		ラスタアドレスマスク変更ＤＭＡパケット書き込み
	*/
int DG_WriteRasterMaskPacks( void *tag_addr, int mask )
{
#if 0 //BP_PS2
	DG_DMATAG	*dmatag ;
	DG_GIFTAG	*giftag ;
	DG_GSREG	*gsreg ;

	dmatag = (DG_DMATAG*)tag_addr ;
	/*  */
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 );
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT( 2, 0 ) ;
	dmatag++ ;
	/*  */
	giftag = (DG_GIFTAG*)dmatag ;
	giftag->tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, 1);
	giftag->regs = 0x0e ;
	giftag++ ;
	/*  */
	gsreg = (DG_GSREG*)giftag ;
	gsreg->reg = SCE_GS_SCANMSK ;
	gsreg->data = ( mask ) ? ( 2 + DG_Clock ) : 0 ;
	gsreg++ ;

	return ( 3 );
#else
   BP_RB_AddCommand(kCmd_SetRasterMask, (char*)mask);
   return 0;
#endif
}

/*----------------------------------------------------------------*/
	/*
		ミップマップ設定ＤＭＡパケット書き込み
	*/
int DG_WriteMipmapSettingPacks( void *tag_addr, DG_MIPMAP *mipmap )
{
	DG_DMATAG	*dmatag ;
	DG_GIFTAG	*giftag ;
	DG_GSREG	*gsreg ;

	dmatag = (DG_DMATAG*)tag_addr ;
	/*  */
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 4 );
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT( 4, 0 ) ;
	dmatag++ ;
	/*  */
	giftag = (DG_GIFTAG*)dmatag ;
	giftag->tag = SCE_GIF_SET_TAG(3, 1, 0, 0, 0, 1);
	giftag->regs = 0x0e ;
	giftag++ ;
	/*  */
	gsreg = (DG_GSREG*)giftag ;
	if ( mipmap != NULL ){
		gsreg->reg = SCE_GS_TEX1_1 ;
		gsreg->data = mipmap->tex1.data ;
		gsreg++ ;
		gsreg->reg = SCE_GS_MIPTBP1_1 ;
		gsreg->data = mipmap->miptbp1.data ;
		gsreg++ ;
		gsreg->reg = SCE_GS_MIPTBP2_1 ;
		gsreg->data = mipmap->miptbp2.data ;
		gsreg++ ;
	} else {
		gsreg->reg = SCE_GS_TEX1_1 ;
		gsreg->data = SCE_GS_SET_TEX1(1, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0) ;
		gsreg++ ;
		gsreg->reg = SCE_GS_NOP ;
		gsreg->data = 0 ;
		gsreg++ ;
		gsreg->reg = SCE_GS_NOP ;
		gsreg->data = 0 ;
		gsreg++ ;
	}

	return ( 5 );
}







