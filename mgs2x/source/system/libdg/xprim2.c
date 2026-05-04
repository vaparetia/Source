//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xprim2.c
	チャンネル処理ユニット／新プリミティブ処理ルーチン

	2002/02/12 K.Takabe
	$Id: xprim2.c,v 1.28 2002/09/11 04:15:17 usr02774 Exp $

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

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"   

#include	"def_dma.h"

#include	"shader.h"


/* ---------------------------------------------------------------- */
/*
	ＸＢＯＸ関連
*/
/* プリフェッチ */
//#define PREFETCH(_a)	__asm{ prefetchnta (_a)}	/* L1に移動 */
//#define PREFETCH(_a)	__asm{ prefetcht0 (_a)}		/* L1&L2に移動 */
//#define PREFETCH(_a)	__asm{ prefetcht1 (_a)}		/* L2に移動 */
//#define PREFETCH(_a)	__asm{ prefetcht2 (_a)}		/* t1と同じ */
#define PREFETCH(_a)
#if 0
static inline void PREFETCH( void *addr )
{
	if ( ( (int)addr & 0x1f ) == 0 ){
		addr = (char*)addr + 32 ;
		//__asm{ prefetchnta (addr)}		/* L1に移動 */
		//__asm{ prefetcht0 (addr)}		/* L1&L2に移動 */
		//__asm{ prefetcht1 (addr)}		/* L2に移動 */
		//__asm{ prefetcht2 (addr)}		/* t1と同じ */
	}
}
#endif
/* PS2形式からXBOX形式へ変換 */
#define EXCHG_COLOR(_c)	( ((_c)&0xff00ff00) | (((_c)&0xff)<<16) | (((_c)&0xff0000)>>16) )
#if 1
//#define SET_COLOR( _r, _g, _b, _a ) ( (_b)|((_g)<<8)|((_r)<<16)|((unsigned int)(_a)<<24) )
#define SET_COLOR( _r, _g, _b, _a ) ( (_r)|((_g)<<8)|((_b)<<16)|((unsigned int)(_a)<<24) )
#else
static inline u_int SET_COLOR( u_char r, u_char g, u_char b, u_char a )
{
	union {
		struct {
			u_char	r, g, b, a ;
		} rgba;
		u_int	col ;
	} tmp ;
	tmp.r = r ; tmp.g = g ; tmp.b = b ; tmp.a = a ;
	return ( tmp.col );
}
#endif
typedef struct _DG_PRIM2_VERTEX {
	D3DXVECTOR3		v ;
	D3DCOLOR		rgba ;
	float			tu0, tv0 ;//, tq0 ;
} DG_PRIM2_VERTEX ;
#define D3DFVF_PRIM2 (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

typedef struct _DG_VERTEX_PRIM2POLY {
	float			vx, vy, vz ;
	u_int			rgba ;
	short			u, v ;
} DG_VERTEX_PRIM2POLY ;
typedef struct _DG_VERTEX_PRIM2SPRT {
	float			vx, vy, vz ;
	short			dx, dy ;				/* 頂点オフセット */
	u_int			rgba ;
	short			u, v ;
} DG_VERTEX_PRIM2SPRT ;

DG_VERTEXSHADER	DG_Prim2VertexShader[2] ;
DG_VERTEXFORMAT	DG_Prim2VertexFormat[2] ;
extern unsigned char VERTEX_SHADER_prm2poly[];
extern unsigned char VERTEX_SHADER_prm2sprt[];
void DG_InitPrim2VertexShader(void)
{
#if 0 //BP_RENDER
	DWORD dwObjDecl_00[] = {
		/* ポリゴン＆ライン用 */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT3),		/* position */
//		D3DVSD_STREAM(1),
		//D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(3,  D3DVSDT_PBYTE4),	/* diffuse */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
		D3DVSD_END()
	};
	DWORD dwObjDecl_01[] = {
		/* スプライト用 */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT3),		/* position */
		D3DVSD_REG(2,  D3DVSDT_SHORT2),		/* normal */
		//D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(3,  D3DVSDT_PBYTE4),	/* diffuse */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
		D3DVSD_END()
	};

	/* 頂点フォーマット生成 */
	DG_MakeVertexFormat( &DG_Prim2VertexFormat[0], dwObjDecl_00 );
	DG_MakeVertexFormat( &DG_Prim2VertexFormat[1], dwObjDecl_01 );

	/* ポリゴン＆ライン用 */
	DG_MakeVertexShader( &DG_Prim2VertexShader[0], VERTEX_SHADER_prm2poly, dwObjDecl_00 );
	/* スプライト用 */
	DG_MakeVertexShader( &DG_Prim2VertexShader[1], VERTEX_SHADER_prm2sprt, dwObjDecl_01 );
#endif

}


/* ---------------------------------------------------------------- */


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
typedef	struct	{
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

/*----------------------------------------------------------------*/
	/*
		プリミティブのソートリストを作成する
	*/
static	void	MakeSortListPrim( DG_PRIM2_PACKET *packet, int raise )
{
	ScrpadWork	*work = SCRPAD_ADDR ;
	u_int		z ;
	void		**ot ;

	/* ソート用データを作成 */
	z = ( (u_int)( packet->sort_z + 0x7fffff - raise ) >> 6 ) ;/* あらかじめ６４で割っておく */
	packet->sort_z = z ; z &= 63 ;
	ot = work->ot1 + z ;
	packet->next_addr = *ot ; *ot = packet ;
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

#if 1
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
#else
	/* 順番を崩さないように小さい方からソートしていく */
	for ( i = 0 ; i < 64   ; i++ ){
		next_addr = work->ot2[i] ;
		while ( next_addr != NULL ){
			packet = next_addr ;
			next_addr = packet->next_addr ;
			z = packet->sort_z ; z = ( z >> 12 ) & 63 ;
			ot3 = DG_Prim2OT + z ;
			packet->next_addr = *ot3 ;
			*ot3 = packet ;
		}
	}
#endif
}

int DG_WritePrimPacks( DG_PRIM2_PACKET *packet, void *addr, int which )
{
	static unsigned short		Index[ 64 * 3 ] ;
	FVECTOR				tmp_vec ;
	unsigned short		*index ;
	DG_PRIM2	*prim ;
	FVECTOR				*pos ;
	DG_PRIM2_UVRGB		*uvrgb ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	DG_CHANL *cp = addr ;

	if ( packet->n_verts == 0 ) return ( 0 );
	prim = packet->prim ;

	/* テクスチャ設定 */
	if ( prim->flag & DG_PRIM2_TEX ){
		//DG_SetTexture(0, &prim->tex_trans);
		DG_SetTextureDirect( 0, prim->tex_trans.ptex );
	} else {
		DG_SetTexture(0, NULL );
	}

	/* PRIM2用の描画環境設定 */
	if ( !( prim->flag & ( DG_PRIM2_CW|DG_PRIM2_CCW ) ) ){
		DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	} else {
		if ( prim->flag & DG_PRIM2_CW ){
			/* 時計周り */
			DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );	/* MGS2とDirectXは逆なので */
		} else {
			/* 反時計周り */
			DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );	/* MGS2とDirectXは逆なので */
		}
	}

	/* 半透明モードの指定 */
	if ( prim->flag & ( DG_PRIM2_ALPHA | DG_PRIM2_ANTIALIASING ) ){
		DG_SetAlphaMode( prim->tex_trans.alpha.data );	/* DG_SetTexture()内で設定されているので */
		//DG_SetAlphaMode(SCE_GS_SET_ALPHA(0,2,0,1,0) & SCE_GS_ALPHA_MASK);
	} else {
		DG_SetAlphaMode(0);
	}
	/* フォグの有無 */
	if ( prim->flag & DG_PRIM2_FOG ){
		DG_SetRenderState(D3DRS_FOGENABLE, TRUE );
	} else {
		DG_SetRenderState(D3DRS_FOGENABLE, FALSE );
	}
	/* フレームバッファテクスチャを使用 */
	if ( !( prim->flag & DG_PRIM2_FRAMETEX ) ){
		static FVECTOR	tex_param[] = {
			{ 1.0f / 4096.0f, 1.0f / 4096.0f, 1, 1 },	/* テクスチャスケール */
			{ 0.0f, 0.0f, 0, 0 },	/* テクスチャオフセット */
		};
		DG_SetVertexShaderConstant( CV_TEX0_SCALE, tex_param, 2 );
	} else {
#ifdef NTSC	/*  */
		static FVECTOR	tex_param[] = {
			{ DRAW_WIDTH / 4096.0f, DRAW_HEIGHT / ( 4096.0f * 448 / 512 ), 1, 1 },	/* テクスチャスケール */
			{ 0.0f, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f, 0, 0 },	/* テクスチャオフセット */
		};
#else
		static FVECTOR	tex_param[] = {
			{ DRAW_WIDTH / 4096.0f, DRAW_HEIGHT / 4096.0f, 1, 1 },	/* テクスチャスケール */
			{ 0.0f, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f, 0, 0 },	/* テクスチャオフセット */
		};
#endif
		DG_SetVertexShaderConstant( CV_TEX0_SCALE, tex_param, 2 );
	}

	DG_SetPixelShader( NULL );

	pos = packet->pos_addr ;
	switch ( packet->type ){
	  case DG_PRIM2_LINE:
		DG_SetVertexShaderConstant(CV_SCREEN, &prim->screen, 4 );
		DG_SelectVertexShader( &DG_Prim2VertexShader[0], &DG_Prim2VertexFormat[ 0 ] );
		/* インデックスが頂点バッファの先頭から始まっているため、パケットの先頭バッファではないので注意！ */
		//DG_SetVertexBuffer( 0, pos, sizeof(FVECTOR) );
		DG_SetVertexBuffer( 0, prim->vbuff, sizeof(DG_VERTEX_PRIM2POLY) );
		DG_SetVertexStream( &DG_Prim2VertexFormat[ 0 ] );	/* 順番が重要！頂点バッファ設定後に呼ぶ必要がある */
		DG_DrawIndexedVertices( D3DPT_LINELIST, packet->n_indices, packet->ibuff );
		break ;
	  case DG_PRIM2_POLY:
	  case DG_PRIM2_CULLPOLY:
		DG_SetVertexShaderConstant(CV_SCREEN, &prim->screen, 4);
		DG_SelectVertexShader( &DG_Prim2VertexShader[0], &DG_Prim2VertexFormat[ 0 ] );
		/* インデックスが頂点バッファの先頭から始まっているため、パケットの先頭バッファではないので注意！ */
		//DG_SetVertexBuffer( 0, pos, sizeof(FVECTOR) );
		DG_SetVertexBuffer( 0, prim->vbuff, sizeof(DG_VERTEX_PRIM2POLY) );
		DG_SetVertexStream( &DG_Prim2VertexFormat[ 0 ] );	/* 順番が重要！頂点バッファ設定後に呼ぶ必要がある */
		DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, packet->n_indices, packet->ibuff );
		break ;
	  case DG_PRIM2_SPRT:
		DG_SetVertexShaderConstant(CV_SCREEN, &prim->screen, 4);
		DG_SelectVertexShader( &DG_Prim2VertexShader[1], &DG_Prim2VertexFormat[ 1 ] );
		DG_SetVertexBuffer( 0, packet->vbuff, sizeof(DG_VERTEX_PRIM2SPRT) );
		DG_SetVertexStream( &DG_Prim2VertexFormat[ 1 ] );	/* 順番が重要！頂点バッファ設定後に呼ぶ必要がある */
#if 0 //BP_RENDER
		DG_DrawVertices( D3DPT_QUADLIST,	/* ＸＢＯＸ拡張！ */
						0, packet->n_verts * 4 );
#endif
		break ;
	  case DG_PRIM2_RSPRT:
		DG_SetVertexShaderConstant(CV_SCREEN, &prim->screen, 4);
		DG_SelectVertexShader( &DG_Prim2VertexShader[1], &DG_Prim2VertexFormat[ 1 ] );
		DG_SetVertexBuffer( 0, packet->vbuff, sizeof(DG_VERTEX_PRIM2SPRT) );
		DG_SetVertexStream( &DG_Prim2VertexFormat[ 1 ] );	/* 順番が重要！頂点バッファ設定後に呼ぶ必要がある */
#if 0 //BP_RENDER
      DG_DrawVertices( D3DPT_QUADLIST,	/* ＸＢＯＸ拡張！ */
						0, packet->n_verts * 4 );
#endif
		break ;
	}

	return ( 0 );
}

/* プリミティブ用の頂点バッファを生成する */
static int DG_MakePrimPacksVBuffer( DG_CHANL *cp, DG_PRIM2 *prim )
{
	FVECTOR				tmp_vec ;
	unsigned short		*index ;
	FVECTOR				*pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGB		*uvrgb ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	int					i, j ;
	u_int				rgba ;

	/* ＶＵ１プログラム選択 */
	switch ( prim->type ){
	  case DG_PRIM2_LINE:
		{
			DG_VERTEX_PRIM2POLY	*vertex ;
			int			count, v_count ;
			/* 動的頂点バッファを確保 */
			vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2POLY), prim->packet_verts * prim->n_prims );
			if ( vertex == NULL ) return ( -1 );
			prim->vbuff = vertex ;
			v_count = 0 ;
			/* パケット処理開始 */
			index = prim->ibuff ;
			packet = prim->packet[ prim->buffer_clock ] ;
			for ( j = 0 ; j < prim->n_prims ; j++, packet++ ){
				pos = packet->pos_addr ;
				uvrgb = packet->uvrgb_addr ;
				packet->vbuff = vertex ;	/* 頂点バッファ先頭記録 *//* 実は必要ない・・・ */
				packet->ibuff = index ;		/* インデックスデータ記録位置を保存 */
				/* 頂点バッファ生成＆インデックス生成 */
				count = 0 ;
				for ( i = 0 ; i < packet->n_verts ; i++, pos++, uvrgb++ ){
					PREFETCH( pos );
					PREFETCH( uvrgb );
#if 1
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
#endif
					vertex->rgba = SET_COLOR( uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a );
					vertex->u = uvrgb->u ;
					vertex->v = uvrgb->v ;
					//vertex->tq0 = (float)uvrgb->q * (1.0f/4096.0f) ;
					vertex++ ;
					/* インデックスデータも生成 */
					if ( ( i != 0 ) && ( !( uvrgb->f & 0x8000 ) ) ){
						index[ 0 ] = v_count - 1 ;
						index[ 1 ] = v_count ;
						index += 2 ;
						count += 2 ;
					}
					v_count++ ;
				}
				packet->n_indices = count ;
			}
		}
		break ;
	  case DG_PRIM2_POLY:
	  case DG_PRIM2_CULLPOLY:
		{
			DG_VERTEX_PRIM2POLY	*vertex ;
			int			count, v_count ;
			/* 動的頂点バッファを確保 */
			vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2POLY), prim->packet_verts * prim->n_prims );
			if ( vertex == NULL ) return ( -1 );
			prim->vbuff = vertex ;
			v_count = 0 ;
			/* パケット処理開始 */
			index = prim->ibuff ;
			packet = prim->packet[ prim->buffer_clock ] ;
			for ( j = 0 ; j < prim->n_prims ; j++, packet++ ){
				pos = packet->pos_addr ;
				uvrgb = packet->uvrgb_addr ;
				packet->vbuff = vertex ;	/* 頂点バッファ先頭記録 *//* 実は必要ない・・・ */
				packet->ibuff = index ;		/* インデックスデータ記録位置を保存 */
				/* 頂点バッファ生成＆インデックス生成 */
				count = 0 ;
				for ( i = 0 ; i < packet->n_verts ; i++, pos++, uvrgb++ ){
					PREFETCH( pos );
					PREFETCH( uvrgb );
#if 1
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
#endif
					vertex->rgba = SET_COLOR( uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a );
					vertex->u = uvrgb->u ;
					vertex->v = uvrgb->v ;
					//vertex->tq0 = uvrgb->q ;
					vertex++ ;
					/* インデックスデータも生成 */
					if ( ( count >= 2 ) && ( uvrgb->f & 0x8000 ) ){
						*index++ = v_count - 1 ;
						count++ ;
					}
					*index++ = v_count++ ;
					count++ ;
				}
				packet->n_indices = count ;
			}
		}
		break ;
	  case DG_PRIM2_SPRT:
		{
			DG_VERTEX_PRIM2SPRT	*vertex ;
			/* 動的頂点バッファを確保 */
			vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2SPRT), prim->packet_verts * 4 * prim->n_prims );
			if ( vertex == NULL ) return ( -1 );
			prim->vbuff = vertex ;
			/* パケット処理開始 */
			index = prim->ibuff ;
			packet = prim->packet[ prim->buffer_clock ] ;
			for ( j = 0 ; j < prim->n_prims ; j++, packet++ ){
				pos = packet->pos_addr ;
				uvrgbwh = packet->uvrgb_addr ;
				packet->vbuff = vertex ;		/* 頂点バッファ先頭記録 */
				/* 頂点バッファ生成 */
				for ( i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ ){
#if 0
					vertex[0].vx = vertex[1].vx = vertex[2].vx = vertex[3].vx = pos->vx ;
					vertex[0].vy = vertex[1].vy = vertex[2].vy = vertex[3].vy = pos->vy ;
					vertex[0].vz = vertex[1].vz = vertex[2].vz = vertex[3].vz = pos->vz ;
					vertex[0].rgba = vertex[1].rgba = vertex[2].rgba = vertex[3].rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a ) ;
					vertex[0].dx = vertex[3].dx = -uvrgbwh->w ;
					vertex[1].dx = vertex[2].dx = uvrgbwh->w ;
					vertex[0].dy = vertex[1].dy = -uvrgbwh->h ;
					vertex[2].dy = vertex[3].dy = uvrgbwh->h ;
					vertex[0].u = vertex[3].u = uvrgbwh->u0 ;
					vertex[1].u = vertex[2].u = uvrgbwh->u1 ;
					vertex[0].v = vertex[1].v = uvrgbwh->v0 ;
					vertex[2].v = vertex[3].v = uvrgbwh->v1 ;
					vertex += 4 ;
#else
					PREFETCH( pos );
					PREFETCH( uvrgbwh );
					rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
					/* 0 */
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
					vertex->dx = -uvrgbwh->w ;
					vertex->dy = -uvrgbwh->h ;
					vertex->rgba = rgba ;
					vertex->u = uvrgbwh->u0 ;
					vertex->v = uvrgbwh->v0 ;
					//vertex->q = 1.0f ;
					vertex++ ;
					/* 1 */
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
					vertex->dx =  uvrgbwh->w ;
					vertex->dy = -uvrgbwh->h ;
					vertex->rgba = rgba ;
					vertex->u = uvrgbwh->u1 ;
					vertex->v = uvrgbwh->v0 ;
					//vertex->q = 1.0f ;
					vertex++ ;
					/* 2 */
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
					vertex->dx =  uvrgbwh->w ;
					vertex->dy =  uvrgbwh->h ;
					vertex->rgba = rgba ;
					vertex->u = uvrgbwh->u1 ;
					vertex->v = uvrgbwh->v1 ;
					//vertex->q = 1.0f ;
					vertex++ ;
					/* 3 */
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
					vertex->dx = -uvrgbwh->w ;
					vertex->dy =  uvrgbwh->h ;
					vertex->rgba = rgba ;
					vertex->u = uvrgbwh->u0 ;
					vertex->v = uvrgbwh->v1 ;
					//vertex->q = 1.0f ;
					vertex++ ;
#endif
				}
			}
		}
		break ;
	  case DG_PRIM2_RSPRT:
		{
			DG_VERTEX_PRIM2SPRT	*vertex ;
			/* 動的頂点バッファを確保 */
			vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2SPRT), prim->packet_verts * 4 * prim->n_prims );
			if ( vertex == NULL ) return ( -1 );
			prim->vbuff = vertex ;
			/* パケット処理開始 */
			packet = prim->packet[ prim->buffer_clock ] ;
			for ( j = 0 ; j < prim->n_prims ; j++, packet++ ){
				pos = packet->pos_addr ;
				uvrgbwh = packet->uvrgb_addr ;
				packet->vbuff = vertex ;		/* 頂点バッファ先頭記録 */
				/* 頂点バッファ生成 */
				for ( i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ ){
#if 0
					vertex[0].vx = vertex[1].vx = vertex[2].vx = vertex[3].vx = pos->vx ;
					vertex[0].vy = vertex[1].vy = vertex[2].vy = vertex[3].vy = pos->vy ;
					vertex[0].vz = vertex[1].vz = vertex[2].vz = vertex[3].vz = pos->vz ;
					vertex[0].rgba = vertex[1].rgba = vertex[2].rgba = vertex[3].rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a ) ;
					vertex[0].dx = -uvrgbwh->w ;
					vertex[1].dx = uvrgbwh->h ;
					vertex[2].dx = uvrgbwh->w ;
					vertex[3].dx = -uvrgbwh->h ;
					vertex[0].dy = -uvrgbwh->h ;
					vertex[1].dy = -uvrgbwh->w ;
					vertex[2].dy = uvrgbwh->h ;
					vertex[3].dy = uvrgbwh->w ;
					vertex[0].u = vertex[3].u = uvrgbwh->u0 ;
					vertex[1].u = vertex[2].u = uvrgbwh->u1 ;
					vertex[0].v = vertex[1].v = uvrgbwh->v0 ;
					vertex[2].v = vertex[3].v = uvrgbwh->v1 ;
					vertex += 4 ;
#else
					PREFETCH( pos );
					PREFETCH( uvrgbwh );
					rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
					/* 0 */
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
					vertex->dx = -uvrgbwh->w ;
					vertex->dy = -uvrgbwh->h ;
					vertex->rgba = rgba ;
					vertex->u = uvrgbwh->u0 ;
					vertex->v = uvrgbwh->v0 ;
					//vertex->q = 1.0f ;
					vertex++ ;
					/* 1 */
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
					vertex->dx =  uvrgbwh->h ;
					vertex->dy = -uvrgbwh->w ;
					vertex->rgba = rgba ;
					vertex->u = uvrgbwh->u1 ;
					vertex->v = uvrgbwh->v0 ;
					//vertex->q = 1.0f ;
					vertex++ ;
					/* 2 */
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
					vertex->dx =  uvrgbwh->w ;
					vertex->dy =  uvrgbwh->h ;
					vertex->rgba = rgba ;
					vertex->u = uvrgbwh->u1 ;
					vertex->v = uvrgbwh->v1 ;
					//vertex->q = 1.0f ;
					vertex++ ;
					/* 3 */
					vertex->vx = pos->vx ;
					vertex->vy = pos->vy ;
					vertex->vz = pos->vz ;
					vertex->dx = -uvrgbwh->h ;
					vertex->dy =  uvrgbwh->w ;
					vertex->rgba = rgba ;
					vertex->u = uvrgbwh->u0 ;
					vertex->v = uvrgbwh->v1 ;
					//vertex->q = 1.0f ;
					vertex++ ;
#endif
				}
			}
		}
		break ;
	}

	return ( 0 );
}

/*----------------------------------------------------------------*/
static FMATRIX	_prim2_mat ;
static inline void _SetMatrix( FMATRIX *m )
{
	_prim2_mat = *m ;
}

static void CalcSortZ( DG_PRIM2_PACKET *packet )
{
	FVECTOR				tmp_verts, total_verts, v, *src ;
	register float		d ;
	int					i ;

#if 1
	/* パケット中の座標の合計を求める */
	d = 1.0f / (float)packet->n_verts ;
	src = packet->pos_addr ;
	__asm {
		mov		edx, src
		movups	xmm0, [ edx + 00h ]
	}
	src++ ;
	for ( i = 1 ; i < packet->n_verts ; i++ ){
		__asm {
			mov		edx, src
			movups	xmm1, [ edx + 00h ]
			addps	xmm0, xmm1
		}
		src++ ;
	}
	__asm {
		movups	[ v + 00h ], xmm0
	}
	_sceVu0ScaleVector( &v, &v, d );
	v.vw = 1.0f ;

	/* 合計値を透視変換し、最後に頂点の総数で除算することで重心の変換座標（の奥行き）を求める */
	_sceVu0ApplyMatrix( &total_verts, &_prim2_mat, &v );
	//d *= total_verts.vw ;
	d = total_verts.vw ;
	packet->sort_z = DG_FTOI( d );
#else
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
#endif
}

/* バウンディングチェック */
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


/*----------------------------------------------------------------*/
void DG_Prim2Chanl( DG_CHANL *cp, int which )
{
	DG_OBJ_QUEUE	*queue ;
	DG_OBJ_BUFFER	*obj_buff ;
	DG_PRIM2		**pque, *prim2 ;
	DG_PRIM2_PACKET	*packet ;
	int		i, j, gid, c_gid, invisible_flag ;
	ScrpadWork		*scrpad = SCRPAD_ADDR ;
	int				make_vbuff_flag = 0, chanl_check_flag = 0 ;
#ifdef DEBUG_MODE
	int				pos_init_error_flag ;
#endif


	MARK( __FILE__ );
	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->prim2_buffer ;
	/*
		キュー数のチェック
	*/
	if ( ( i = obj_buff->n_queue ) == 0 ) return ;

   BP_RB_PushRegionMarker(kProfileColor_Prim, "Prim");
   BP_Debug_PushCPUMarker( "Prim" );

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
	// *EYE_PERS2 = cp->raise_eye_pers2 ;
	SCALE->vx = cp->width / 2 ;
	SCALE->vy = cp->height / 2 ;
	invisible_flag = DG_PRIM2_INVISIBLE0 << cp->chanl_num ;
#ifdef LIBDG_PERFORMANCE
	scrpad->prim_poly = DG_PerformanceData.prim_poly ;
	scrpad->prim_line = DG_PerformanceData.prim_line ;
	scrpad->prim_sprt = DG_PerformanceData.prim_sprt ;
	scrpad->prim_rsprt = DG_PerformanceData.prim_rsprt ;
#endif
	{/* 頂点バッファの構築し直しを判断 */
		static int	last_gv_time = 0 ;
		if ( GV_Time != last_gv_time ){
			make_vbuff_flag = 1 ;
			last_gv_time = GV_Time ;
			chanl_check_flag = 0 ;
			if ( DG_Chanls[0].flag ) chanl_check_flag |= DG_PRIM2_INVISIBLE0 ;
			if ( DG_Chanls[1].flag ) chanl_check_flag |= DG_PRIM2_INVISIBLE1 ;
			if ( DG_Chanls[2].flag ) chanl_check_flag |= DG_PRIM2_INVISIBLE2 ;
			if ( DG_Chanls[3].flag ) chanl_check_flag |= DG_PRIM2_INVISIBLE3 ;
		}
	}

	for ( ; i > 0 ; i--, pque++ ){
		prim2 = *pque ;

		if ( make_vbuff_flag ){/* 頂点バッファの構築 */
			if ( ( prim2->flag & chanl_check_flag ) != chanl_check_flag ){
				if ( DG_MakePrimPacksVBuffer( cp, prim2 ) < 0 ) continue ;
			}
		}

		if ( prim2->flag & invisible_flag ) continue ;

		/* 表示グループチェック */
		if ( ( gid = prim2->group_id ) != 0 && !( gid & c_gid ) ) continue ;

		/* プリミティブ設置マトリクス確定 */
		if ( prim2->root != NULL ) prim2->world = *( prim2->root ) ;
		scrpad->world = prim2->world ;

		/* 変換基準チェック */
		if ( prim2->flag & DG_PRIM2_ON_CAMERA ){
			/* カメラ相対 */
			scrpad->screen = scrpad->pers ;
			/* prim2->screenはＸＢＯＸのＴ＆Ｌを使う関係で透視変換マトリクスを通さない値を設定 */
			prim2->screen = DG_UnitMatrix ;
		} else {
			/* 通常計算 */
			_sceVu0MulMatrix( &scrpad->screen, &scrpad->eye_pers, &scrpad->world );
			/* prim2->screenはＸＢＯＸのＴ＆Ｌを使う関係で透視変換マトリクスを通さない値を設定 */
			_sceVu0MulMatrix( &prim2->screen, &cp->eye_inv, &scrpad->world );
		}
		//prim2->screen = scrpad->screen ;
		//prim2->prim_param[ which ].screen = scrpad->screen ;

		_SetMatrix( &scrpad->screen );
		/* バウンディングチェック */
		if ( prim2->flag & DG_PRIM2_BOUNDCHECK ){
			if ( BoundCheck( &scrpad->screen, &prim2->bound_min ) == 2 ){
				//printf("skip\n");
				continue ;
			}
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
			/* ソート用Ｚ値計算 */
			CalcSortZ( packet );
			/* ＯＴに接続 */
			MakeSortListPrim( packet, prim2->raise );
#ifdef LIBDG_PERFORMANCE
			scrpad->trg_prim_info->n_verts += packet->n_verts ;
#endif
		}
#ifdef DEBUG_MODE
		if ( pos_init_error_flag ){
			if ( !( prim2->flag & DG_PRIM2_DEBUGCHECKED ) ){
				printf("%s:packet init error (%s)!!!!!!!!!!!!!!!!!\n", __FILE__, prim2->fname );
				prim2->flag |= DG_PRIM2_DEBUGCHECKED ;
			}
		}
#endif
	}

	/* 全プリミティブパケットのソートを行なう */
	SortListPrims();

	/* 表示実験用 */
	//SortChainPrims( cp );
#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.prim_poly = scrpad->prim_poly ;
	DG_PerformanceData.prim_line = scrpad->prim_line ;
	DG_PerformanceData.prim_sprt = scrpad->prim_sprt ;
	DG_PerformanceData.prim_rsprt = scrpad->prim_rsprt ;
#endif

   BP_Debug_PopCPUMarker();
   BP_RB_PopRegionMarker();

}
/*----------------------------------------------------------------*/
DG_PRIM2 *DG_MakePrim2( int flag, int n_prims, int n_verts, int chanl )
{
	DG_PRIM2		*prim2 ;
	DG_PRIM2_PACKET	*packet ;
	FVECTOR			*pos ;
	void			*uvrgb ;
	int			type, size, n_buffer, uvrgb_size, packet_flag, index_size ;
	int			i, j ;
	u_long64		prim ;

	packet_flag = flag & 0xffff ;
	flag |= DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 ;
	type = flag & DG_PRIM2_TYPEMASK ;

	if ( flag & DG_PRIM2_SINGLEBUFF )	n_buffer = 1 ;
	else								n_buffer = 2 ;
	if ( type < DG_PRIM2_SPRT )	uvrgb_size = sizeof(DG_PRIM2_UVRGB) ;
	else						uvrgb_size = sizeof(DG_PRIM2_UVRGBWH) ;
	/* インデックス記録メモリのサイズ計算 */
	if ( type < DG_PRIM2_SPRT )	index_size = n_prims * n_verts * 2 * sizeof(short) ;
	else						index_size = 0 ;

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

	if ( ( prim2 = GV_Malloc( size + index_size ) ) == NULL ) return ( NULL ) ;
	GV_ZeroMemory( prim2, sizeof(DG_PRIM2) );
	//GV_ZeroMemory( prim2, size );	/* 原因不明不具合一時的回避の為 */

	/* 構造体の初期化 */
	prim2->flag = flag ;
	prim2->world = DG_UnitMatrix ;
	prim2->group_id = 0 ;
	prim2->chanl = chanl ;
	prim2->type = type ;
	prim2->n_prims = n_prims ;
	prim2->packet_verts = n_verts ;
	prim2->ibuff = (void*)( (char*)prim2 + size );

	/* ポインタの設定 */
	packet = (DG_PRIM2_PACKET*)&prim2[1] ;
	prim2->packet[ 0 ] = &packet[ n_prims * ( 0 ) ] ;
	prim2->packet[ 1 ] = &packet[ n_prims * ( n_buffer - 1 ) ] ;

	if ( !( flag & DG_PRIM2_NOBUFFER ) ){
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
#ifdef DEBUG_MODE
				((u_long64*)packet->pos_addr)[packet->n_verts-1] = 0xABCDEF01ABCDEF01ui64 ;
				*(u_long64*)((char*)packet->uvrgb_addr + uvrgb_size*(packet->n_verts-1) ) = 0xABCDEF01ABCDEF01ui64 ;
#endif
				packet++ ;
				pos += n_verts ;
				uvrgb = (void*)( (int)uvrgb + uvrgb_size * n_verts ) ;
			}
		}
	}

#if 0
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
#endif
#if 0
	/* デフォルトテクスチャ設定（ＤＭＡパケット初期化のため） */
	{
		//extern DG_TEX	DG_TexBuffer[];
		//prim2->tex_trans = DG_TexBuffer[0].tex_trans ;
		extern DG_TEX	DG_DefaultTexture ;
		prim2->tex_trans = DG_DefaultTexture.tex_trans ;
	}
#endif

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

#ifdef DEBUG_MODE

DG_PRIM2 *DG_MakePrim2D( int flag, int n_prims, int n_verts, int chanl, char *fname )
{
	DG_PRIM2	*prim2 ;
	prim2 = DG_MakePrim2( flag, n_prims, n_verts, chanl );
	if ( prim2 != NULL ) prim2->fname = fname ;
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
