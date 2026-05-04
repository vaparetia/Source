/*
	xprim2.c
	チャンネル処理ユニット／新プリミティブ処理ルーチン

	2002/02/12 K.Takabe
	$Id: wprim2.c,v 1.23 2002/12/23 14:01:02 takaki Exp $

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

#ifdef _XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include <stdio.h>
#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"   

#include	"def_dma.h"

#include	"shader.h"

extern	int					DG_CurrentBackBuffer ;
extern LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;

static	FVECTOR	_cam_unit_vec_x ;	// Camera座標空間X軸単位ベクトル-->World
static	FVECTOR	_cam_unit_vec_y ;	// Camera座標空間Y軸単位ベクトル-->World

static int DG_MakePrimPacksVBuffer_Line( DG_PRIM2 *prim ) ;
static int DG_MakePrimPacksVBuffer_Poly( DG_PRIM2 *prim ) ;
static int DG_MakePrimPacksVBuffer_Sprt( DG_PRIM2 *prim ) ;
static int DG_MakePrimPacksVBuffer_RSprt(DG_PRIM2 *prim ) ;

static int DG_MakePrimPacksVBuffer_Line_NVS( DG_PRIM2 *prim ) ;
static int DG_MakePrimPacksVBuffer_Poly_NVS( DG_PRIM2 *prim ) ;
static int DG_MakePrimPacksVBuffer_Sprt_NVS( DG_PRIM2 *prim ) ;
static int DG_MakePrimPacksVBuffer_ScreenSprt_NVS( DG_PRIM2 *prim ) ;
static int DG_MakePrimPacksVBuffer_RSprt_NVS(DG_PRIM2 *prim ) ;
static int DG_MakePrimPacksVBuffer_ScreenRSprt_NVS( DG_PRIM2 *prim ) ;

static void DG_RemakePrimPacksVBuffer_Sprt_NVS( DG_PRIM2_PACKET *packet ) ;
static void DG_RemakePrimPacksVBuffer_RSprt_NVS( DG_PRIM2_PACKET *packet ) ;

static void DG_WritePrimPacks_Line( DG_PRIM2_PACKET *packet ) ;
static void DG_WritePrimPacks_Poly( DG_PRIM2_PACKET *packet ) ;
static void DG_WritePrimPacks_Sprt( DG_PRIM2_PACKET *packet ) ;

static void DG_WritePrimPacks_Line_NVS( DG_PRIM2_PACKET *packet ) ;
static void DG_WritePrimPacks_Poly_NVS( DG_PRIM2_PACKET *packet ) ;
static void DG_WritePrimPacks_Sprt_NVS( DG_PRIM2_PACKET *packet ) ;

/* ---------------------------------------------------------------- */
/*
	フレームテクスチャパラメータ
*/
extern float				DG_BackBufferWidthAdjustCoef ;	// RendWidth/Width
extern float				DG_BackBufferHeightAdjustCoef ;	// RendHeight/Height


#ifndef _WINDOWS
#ifdef NTSC	/*  */
static FVECTOR	frame_tex_param[] = {
	{ DRAW_WIDTH / 4096.0f, DRAW_HEIGHT / ( 4096.0f * 448 / 512 ), 1, 1 },
	{ 0.0f, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f, 0, 0 },	/* テクスチャオフセット */
};
#else
static FVECTOR	frame_tex_param[] = {
	{ DRAW_WIDTH / 4096.0f, DRAW_HEIGHT / 4096.0f, 1, 1 },	/* テクスチャスケール */
	{ 0.0f, (DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f, 0, 0 },	/* テクスチャオフセット */
};
#endif

#else	// Windiows
static FVECTOR	frame_tex_param[] = {
	{ 1.0f / 4096.0f, 1.0f / 4096.0f , 1, 1 },
	{
		((float)(DISPLAY_WIDTH-DRAW_WIDTH)*0.5f)/(float)DISPLAY_WIDTH,
		((float)(DISPLAY_HEIGHT-DRAW_HEIGHT)*0.5f)/(float)DISPLAY_HEIGHT,
		0, 0
	},	/* テクスチャオフセット */
};
#endif

#ifndef _WINDOWS
static FMATRIX	frame_tex_param_mtx =
#ifdef NTSC	/*  */
{
	DRAW_WIDTH/4096.0f,	0.0f, 							0.0f,	0.0f,
	0.0f,				DRAW_HEIGHT/(4096.0f*448/512),	0.0f,	0.0f,
	0.0f,				(DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f,1.0f,	0.0f,
	0.0f,				0.0f,							0.0f,	1.0f
} ;
#else
{
	DRAW_WIDTH/4096.0f,	0.0f, 							0.0f,	0.0f,
	0.0f,				DRAW_HEIGHT/4096.0f,			0.0f,	0.0f,
	0.0f,				(DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f,1.0f,	0.0f,
	0.0f,				0.0f,							0.0f,	1.0f
} ;
#endif

#else	// Windows
static FMATRIX	frame_tex_param_mtx =
{
	1.0f/4096.0f,	0.0f,			0.0f,	0.0f,
	0.0f,			1.0f/4096.0f,	0.0f,	0.0f,
	((float)(DISPLAY_WIDTH-DRAW_WIDTH)*0.5f)/(float)DISPLAY_WIDTH,
	((float)(DISPLAY_HEIGHT-DRAW_HEIGHT)*0.5f)/(float)DISPLAY_HEIGHT,	1.0f,	0.0f,
	0.0f,			0.0f,			0.0f,	1.0f
} ;
#endif

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
#define SET_COLOR( _r, _g, _b, _a ) ( (_b)|((_g)<<8)|((_r)<<16)|((unsigned int)(_a)<<24) )
//#define SET_COLOR( _r, _g, _b, _a ) ( (_r)|((_g)<<8)|((_b)<<16)|((unsigned int)(_a)<<24) )
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

typedef struct _DG_VERTEX_PRIM2POLY_FVF {
	float			vx, vy, vz ;
	DWORD			rgba ;
	float			u, v ;
} DG_VERTEX_PRIM2POLY_FVF ;
#define	D3DFVF_DG_VERTEX_PRIM2POLY_FLAG	( D3DFVF_XYZ		\
										| D3DFVF_DIFFUSE	\
										| D3DFVF_TEX1		\
										| D3DFVF_TEXCOORDSIZE2(0))

typedef struct _DG_VERTEX_PRIM2SPRT_FVF {
	float			vx, vy, vz ;			/* 頂点オフセットも加算 */
	DWORD			rgba ;
	float			u, v ;
} DG_VERTEX_PRIM2SPRT_FVF ;
#define	D3DFVF_DG_VERTEX_PRIM2SPRT_FLAG	( D3DFVF_XYZ		\
										| D3DFVF_DIFFUSE	\
										| D3DFVF_TEX1		\
										| D3DFVF_TEXCOORDSIZE2(0))



DG_VERTEXSHADER	DG_Prim2VertexShader[2] ;
extern unsigned char VERTEX_SHADER_prm2poly[];
extern unsigned char VERTEX_SHADER_prm2sprt[];
void DG_InitPrim2VertexShader(void)
{
	DWORD dwObjDecl_00[] = {
		/* ポリゴン＆ライン用 */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT3),		/* position */
//		D3DVSD_STREAM(1),
		//D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
		D3DVSD_END()
	};
	DWORD dwObjDecl_01[] = {
		/* スプライト用 */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT3),		/* position */
		D3DVSD_REG(2,  D3DVSDT_SHORT2),		/* normal */
		//D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),	/* diffuse */
		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0 */
		D3DVSD_END()
	};

	if( DG_CheckUseVertexShader() )
	{
		/* ポリゴン＆ライン用 */
		DG_MakeVertexShader( &DG_Prim2VertexShader[0], VSHT_wprm2poly, dwObjDecl_00 );
		/* スプライト用 */
		DG_MakeVertexShader( &DG_Prim2VertexShader[1], VSHT_wprm2sprt, dwObjDecl_01 );
	}

	/* フレームテクスチャパラメータ補正 */
	frame_tex_param[0].vx *= DG_BackBufferWidthAdjustCoef ;
	frame_tex_param[0].vy *= DG_BackBufferHeightAdjustCoef ;
	frame_tex_param[1].vx *= DG_BackBufferWidthAdjustCoef ;
	frame_tex_param[1].vy *= DG_BackBufferHeightAdjustCoef ;

	frame_tex_param_mtx.m[0][0] *= DG_BackBufferWidthAdjustCoef ;
	frame_tex_param_mtx.m[1][1] *= DG_BackBufferHeightAdjustCoef ;
	frame_tex_param_mtx.m[2][0] *= DG_BackBufferWidthAdjustCoef ;
	frame_tex_param_mtx.m[2][1] *= DG_BackBufferHeightAdjustCoef ;
}

void DG_ReleasePrim2VertexShader(void)
{
	DG_KillVertexShader( &DG_Prim2VertexShader[0] );
	DG_KillVertexShader( &DG_Prim2VertexShader[1] );
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
	ScrpadWork	*work = (ScrpadWork *)SCRPAD_ADDR ;
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
	ScrpadWork	*work = (ScrpadWork *)SCRPAD_ADDR ;
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

static int DG_MakePrimPacksVBuffer_Line( DG_PRIM2 *prim )
{
	unsigned short		*index ;
	FVECTOR				*pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int					i, j ;
	DG_VERTEX_PRIM2POLY	*vertex ;
	int					count, v_count ;

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2POLY),
					prim->packet_verts * prim->n_prims );
	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;

	/* パケット処理開始 */
	index = prim->ibuff ;
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		pos = packet->pos_addr ;
		uvrgb = packet->uvrgb_addr ;

		v_count = 0 ;
		packet->vbuff = vertex ;	/* 頂点バッファ先頭記録 */

		packet->ibuff = index ;		/* インデックスデータ記録位置を保存 */
		/* 頂点バッファ生成＆インデックス生成 */
		count = 0 ;
		for ( i = 0 ; i < packet->n_verts ; i++, pos++, uvrgb++ ){
			PREFETCH( pos );
			PREFETCH( uvrgb );

			vertex->vx = pos->vx ;
			vertex->vy = pos->vy ;
			vertex->vz = pos->vz ;

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

	return( 0 ) ;
}

static int DG_MakePrimPacksVBuffer_Line_NVS( DG_PRIM2 *prim )
{
	unsigned short		*index ;
	FVECTOR				*pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int					i, j ;
	DG_VERTEX_PRIM2POLY_FVF	*vertex ;
	int					count, v_count ;

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2POLY_FVF),
					prim->packet_verts * prim->n_prims );
	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;

	/* パケット処理開始 */
	index = prim->ibuff ;
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		pos = packet->pos_addr ;
		uvrgb = packet->uvrgb_addr ;

		v_count = 0 ;
		packet->vbuff = vertex ;	/* 頂点バッファ先頭記録 */

		packet->ibuff = index ;		/* インデックスデータ記録位置を保存 */
		/* 頂点バッファ生成＆インデックス生成 */
		count = 0 ;
		for ( i = 0 ; i < packet->n_verts ; i++, pos++, uvrgb++ ){
			PREFETCH( pos );
			PREFETCH( uvrgb );

			vertex->vx = pos->vx ;
			vertex->vy = pos->vy ;
			vertex->vz = pos->vz ;

			vertex->rgba = SET_COLOR( uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a );
			vertex->u = (float)uvrgb->u ;
			vertex->v = (float)uvrgb->v ;
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

	return( 0 ) ;
}


static int DG_MakePrimPacksVBuffer_Poly( DG_PRIM2 *prim )
{
	unsigned short		*index ;
	FVECTOR				*pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int					i, j ;
	DG_VERTEX_PRIM2POLY	*vertex ;
	int					count, v_count ;

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2POLY),
									prim->packet_verts * prim->n_prims );
	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;

	/* パケット処理開始 */
	index = prim->ibuff ;
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		pos = packet->pos_addr ;
		uvrgb = packet->uvrgb_addr ;

		v_count = 0 ;
		packet->vbuff = vertex ;	/* 頂点バッファ先頭記録 */

		packet->ibuff = index ;		/* インデックスデータ記録位置を保存 */
		/* 頂点バッファ生成＆インデックス生成 */
		count = 0 ;
		for ( i =  packet->n_verts ; i > 0 ; i--, pos++, uvrgb++ ){
			PREFETCH( pos );
			PREFETCH( uvrgb );

			vertex->vx = pos->vx ;
			vertex->vy = pos->vy ;
			vertex->vz = pos->vz ;

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

	return(0) ;
}

static int DG_MakePrimPacksVBuffer_Poly_NVS( DG_PRIM2 *prim )
{
	unsigned short		*index ;
	FVECTOR				*pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int					i, j ;
	DG_VERTEX_PRIM2POLY_FVF	*vertex ;
	int					count, v_count ;

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2POLY_FVF),
									prim->packet_verts * prim->n_prims );
	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;

	/* パケット処理開始 */
	index = prim->ibuff ;
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		pos   = packet->pos_addr ;
		uvrgb = packet->uvrgb_addr ;

		v_count = 0 ;
		packet->vbuff = vertex ;	/* 頂点バッファ先頭記録 */

		packet->ibuff = index ;		/* インデックスデータ記録位置を保存 */
		/* 頂点バッファ生成＆インデックス生成 */
		count = 0 ;
		for ( i =  packet->n_verts ; i > 0 ; i--, pos++, uvrgb++ ){
			PREFETCH( pos );
			PREFETCH( uvrgb );

			vertex->vx = pos->vx ;
			vertex->vy = pos->vy ;
			vertex->vz = pos->vz ;

			vertex->rgba = SET_COLOR( uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a );
			vertex->u = (float)uvrgb->u ;
			vertex->v = (float)uvrgb->v ;
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

	return(0) ;
}

static int DG_MakePrimPacksVBuffer_Sprt( DG_PRIM2 *prim )
{
	unsigned short		*index ;
	FVECTOR				*pos ;
	FVECTOR				cur_pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	DG_PRIM2_UVRGBWH	cur_uvrgbwh ;
	int					i, j ;
	u_int				rgba ;
	DG_VERTEX_PRIM2SPRT	*vertex ;

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2SPRT),
							prim->packet_verts * 6 * prim->n_prims );
	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;
	/* パケット処理開始 */
	index = prim->ibuff ;
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		pos = packet->pos_addr ;
		uvrgbwh = packet->uvrgb_addr ;
		packet->vbuff = vertex ;		/* 頂点バッファ先頭記録 */

		/* 頂点バッファ生成 */
		if( packet->n_verts == 1 )
		{
			rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
			cur_pos     = *pos ;
			cur_uvrgbwh = *uvrgbwh ;

			/* 0 */
			vertex->vx = cur_pos.vx ;
			vertex->vy = cur_pos.vy ;
			vertex->vz = cur_pos.vz ;
			vertex->dx = -cur_uvrgbwh.w ;
			vertex->dy = -cur_uvrgbwh.h ;
			vertex->rgba = rgba ;
			vertex->u = cur_uvrgbwh.u0 ;
			vertex->v = cur_uvrgbwh.v0 ;
			//vertex->q = 1.0f ;
			vertex++ ;
			/* 1 */
			vertex->vx = cur_pos.vx ;
			vertex->vy = cur_pos.vy ;
			vertex->vz = cur_pos.vz ;
			vertex->dx =  cur_uvrgbwh.w ;
			vertex->dy = -cur_uvrgbwh.h ;
			vertex->rgba = rgba ;
			vertex->u = cur_uvrgbwh.u1 ;
			vertex->v = cur_uvrgbwh.v0 ;
			//vertex->q = 1.0f ;
			vertex++ ;
			/* 2 */
			vertex->vx = cur_pos.vx ;
			vertex->vy = cur_pos.vy ;
			vertex->vz = cur_pos.vz ;
			vertex->dx = -cur_uvrgbwh.w ;
			vertex->dy =  cur_uvrgbwh.h ;
			vertex->rgba = rgba ;
			vertex->u = cur_uvrgbwh.u0 ;
			vertex->v = cur_uvrgbwh.v1 ;
			//vertex->q = 1.0f ;
			vertex++ ;
			/* 3 */
			vertex->vx = cur_pos.vx ;
			vertex->vy = cur_pos.vy ;
			vertex->vz = cur_pos.vz ;
			vertex->dx =  cur_uvrgbwh.w ;
			vertex->dy =  cur_uvrgbwh.h ;
			vertex->rgba = rgba ;
			vertex->u = cur_uvrgbwh.u1 ;
			vertex->v = cur_uvrgbwh.v1 ;
			//vertex->q = 1.0f ;
			vertex++ ;
		}
		else
		{
			for ( i = packet->n_verts ; i > 0 ; i--, pos++, uvrgbwh++ ){
				PREFETCH( pos );
				PREFETCH( uvrgbwh );
				rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
				cur_pos     = *pos ;
				cur_uvrgbwh = *uvrgbwh ;

				/* 0 */
				vertex->vx = cur_pos.vx ;
				vertex->vy = cur_pos.vy ;
				vertex->vz = cur_pos.vz ;
				vertex->dx = -cur_uvrgbwh.w ;
				vertex->dy = -cur_uvrgbwh.h ;
				vertex->rgba = rgba ;
				vertex->u = cur_uvrgbwh.u0 ;
				vertex->v = cur_uvrgbwh.v0 ;
				//vertex->q = 1.0f ;
				vertex++ ;
				/* 1 */
				vertex->vx = cur_pos.vx ;
				vertex->vy = cur_pos.vy ;
				vertex->vz = cur_pos.vz ;
				vertex->dx =  cur_uvrgbwh.w ;
				vertex->dy = -cur_uvrgbwh.h ;
				vertex->rgba = rgba ;
				vertex->u = cur_uvrgbwh.u1 ;
				vertex->v = cur_uvrgbwh.v0 ;
				//vertex->q = 1.0f ;
				vertex[2]    = *vertex ;	// /* 3 */
				vertex++ ;
				/* 2 */
				vertex->vx = cur_pos.vx ;
				vertex->vy = cur_pos.vy ;
				vertex->vz = cur_pos.vz ;
				vertex->dx = -cur_uvrgbwh.w ;
				vertex->dy =  cur_uvrgbwh.h ;
				vertex->rgba = rgba ;
				vertex->u = cur_uvrgbwh.u0 ;
				vertex->v = cur_uvrgbwh.v1 ;
				//vertex->q = 1.0f ;
				vertex[2]    = *vertex ;	// /* 4 */
				vertex +=3 ;					// --> /* 5 */
				/* 5 */
				vertex->vx = cur_pos.vx ;
				vertex->vy = cur_pos.vy ;
				vertex->vz = cur_pos.vz ;
				vertex->dx =  cur_uvrgbwh.w ;
				vertex->dy =  cur_uvrgbwh.h ;
				vertex->rgba = rgba ;
				vertex->u = cur_uvrgbwh.u1 ;
				vertex->v = cur_uvrgbwh.v1 ;
				//vertex->q = 1.0f ;
				vertex++ ;
			}
		}
	}
	return(0) ;
}

static int DG_MakePrimPacksVBuffer_Sprt_NVS( DG_PRIM2 *prim )
{
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	float				u0, v0, u1, v1 ;
	int					i, j ;
	u_int				rgba ;
	DG_VERTEX_PRIM2SPRT_FVF	*vertex ;

	/* Screen上の物は動作が違う */
	if( prim->flag & DG_PRIM2_ON_CAMERA )
	{
		return( DG_MakePrimPacksVBuffer_ScreenSprt_NVS(prim) ) ;
	}

	/* World Matrix検査 */
	if(  (prim->world.m[0][0] != 1.0f)
	  || (prim->world.m[0][1] != 0.0f)
	  || (prim->world.m[0][2] != 0.0f)
	  || (prim->world.m[1][0] != 0.0f)
	  || (prim->world.m[1][1] != 1.0f)
	  || (prim->world.m[1][2] != 0.0f)
	  || (prim->world.m[2][0] != 0.0f)
	  || (prim->world.m[2][1] != 0.0f)
	  || (prim->world.m[2][2] != 0.0f) )
	{
		prim->flag &= ~DG_PRIM2_WORLDMTX_UNITROT ;
	}
	else
	{
		prim->flag |= DG_PRIM2_WORLDMTX_UNITROT ;
	}

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2SPRT_FVF),
							prim->packet_verts * 6 * prim->n_prims );
	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;
	/* パケット処理開始 */
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		uvrgbwh = packet->uvrgb_addr ;
		packet->vbuff = vertex ;		/* 頂点バッファ先頭記録 */

		/* 頂点バッファ生成 */
		if( packet->n_verts == 1 )
		{
			rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
			u0 = uvrgbwh->u0 ;
			v0 = uvrgbwh->v0 ;
			u1 = uvrgbwh->u1 ;
			v1 = uvrgbwh->v1 ;

			/* 0 */
			vertex->rgba = rgba ;
			vertex->u    = u0 ;
			vertex->v    = v0 ;
			vertex++ ;
			/* 1 */
			vertex->rgba = rgba ;
			vertex->u    = u1 ;
			vertex->v    = v0 ;
			vertex++ ;
			/* 2 */
			vertex->rgba = rgba ;
			vertex->u    = u0 ;
			vertex->v    = v1 ;
			vertex++ ;
			/* 3 */
			vertex->rgba = rgba ;
			vertex->u    = u1 ;
			vertex->v    = v1 ;
			vertex++ ;
		}
		else
		{
			for ( i = packet->n_verts ; i > 0 ; i--, uvrgbwh++ ){
				rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
				u0 = uvrgbwh->u0 ;
				v0 = uvrgbwh->v0 ;
				u1 = uvrgbwh->u1 ;
				v1 = uvrgbwh->v1 ;

				/* 0 */
				vertex->rgba = rgba ;
				vertex->u    = u0 ;
				vertex->v    = v0 ;
				vertex++ ;
				/* 1 */
				vertex->rgba = rgba ;
				vertex->u    = u1 ;
				vertex->v    = v0 ;
				vertex[2]    = *vertex ;	// /* 3 */
				vertex++ ;
				/* 2 */
				vertex->rgba = rgba ;
				vertex->u    = u0 ;
				vertex->v    = v1 ;
				vertex[2]    = *vertex ;	// /* 4 */
				vertex += 3 ;	// --> /* 5 */
				/* 5 */
				vertex->rgba = rgba ;
				vertex->u    = u1 ;
				vertex->v    = v1 ;
				vertex++ ;
			}
		}
	}
	return(0) ;
}


static int DG_MakePrimPacksVBuffer_ScreenSprt_NVS( DG_PRIM2 *prim )
{
	unsigned short		*index ;
	FVECTOR				*pos ;
	FVECTOR				cur_pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	float				lx, ly, rx, ry;
	float				u0, v0, u1, v1 ;
	int					i, j ;
	u_int				rgba ;
	DG_VERTEX_PRIM2SPRT_FVF	*vertex ;

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2SPRT_FVF),
							prim->packet_verts * 6 * prim->n_prims );
	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;
	/* パケット処理開始 */
	index = prim->ibuff ;
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		pos = packet->pos_addr ;
		uvrgbwh = packet->uvrgb_addr ;
		packet->vbuff = vertex ;		/* 頂点バッファ先頭記録 */

		/* 頂点バッファ生成 */
		if( packet->n_verts == 1 )
		{
			rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
			cur_pos     = *pos ;

			{
				float	w, h ;

				w = (float)uvrgbwh->w ;
				h = (float)uvrgbwh->h ;

				lx = cur_pos.vx - w ;
				ly = cur_pos.vy - h ;
				rx = cur_pos.vx + w ;
				ry = cur_pos.vy + h ;
			}

			u0 = uvrgbwh->u0 ;
			v0 = uvrgbwh->v0 ;
			u1 = uvrgbwh->u1 ;
			v1 = uvrgbwh->v1 ;

			/* 0 */
			/* 0 */
			vertex->vx   = lx ;
			vertex->vy   = ly ;
			vertex->vz   = cur_pos.vz ;
			vertex->rgba = rgba ;
			vertex->u    = u0 ;
			vertex->v    = v0 ;
			vertex++ ;
			/* 1 */
			vertex->vx   = rx ;
			vertex->vy   = ly ;
			vertex->vz   = cur_pos.vz ;
			vertex->rgba = rgba ;
			vertex->u    = u1 ;
			vertex->v    = v0 ;
			vertex++ ;
			/* 2 */
			vertex->vx   = lx ;
			vertex->vy   = ry ;
			vertex->vz   = cur_pos.vz ;
			vertex->rgba = rgba ;
			vertex->u    = u0 ;
			vertex->v    = v1 ;
			vertex++ ;
			/* 3 */
			vertex->vx   = rx ;
			vertex->vy   = ry ;
			vertex->vz   = cur_pos.vz ;
			vertex->rgba = rgba ;
			vertex->u    = u1 ;
			vertex->v    = v1 ;
			vertex++ ;
		}
		else
		{
			for ( i = packet->n_verts; i>0 ; i--, pos++, uvrgbwh++ ){
				rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
				cur_pos     = *pos ;

				{
					float	w, h ;
					w = (float)uvrgbwh->w ;
					h = (float)uvrgbwh->h ;

					lx = cur_pos.vx - w ;
					ly = cur_pos.vy - h ;
					rx = cur_pos.vx + w ;
					ry = cur_pos.vy + h ;
				}

				u0 = uvrgbwh->u0 ;
				v0 = uvrgbwh->v0 ;
				u1 = uvrgbwh->u1 ;
				v1 = uvrgbwh->v1 ;

				vertex->vx   = lx ;
				vertex->vy   = ly ;
				vertex->vz   = cur_pos.vz ;
				vertex->rgba = rgba ;
				vertex->u    = u0 ;
				vertex->v    = v0 ;
				vertex++ ;
				/* 1 */
				vertex->vx   = rx ;
				vertex->vy   = ly ;
				vertex->vz   = cur_pos.vz ;
				vertex->rgba = rgba ;
				vertex->u    = u1 ;
				vertex->v    = v0 ;
				vertex[2]    = *vertex ;	// /* 3 */
				vertex++ ;
				/* 2 */
				vertex->vx   = lx ;
				vertex->vy   = ry ;
				vertex->vz   = cur_pos.vz ;
				vertex->rgba = rgba ;
				vertex->u    = u0 ;
				vertex->v    = v1 ;
				vertex[2]    = *vertex ;	// /* 4 */
				vertex += 3 ;	// --> /* 5 */
				/* 5 */
				vertex->vx   = rx ;
				vertex->vy   = ry ;
				vertex->vz   = cur_pos.vz ;
				vertex->rgba = rgba ;
				vertex->u    = u1 ;
				vertex->v    = v1 ;
				vertex++ ;
			}
		}
	}
	return(0) ;
}


static void DG_RemakePrimPacksVBuffer_Sprt_NVS( DG_PRIM2_PACKET *packet )
{
	FVECTOR				*pos ;
	FVECTOR				cur_pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	FVECTOR				vec[4] ;
	int					i ;
	DG_VERTEX_PRIM2SPRT_FVF	*vertex ;
	FVECTOR				uvec_w, uvec_h ;

	/* パケット処理開始 */
	{
		DG_PRIM2	*prim ;

		prim = packet->prim ;
		if( prim->flag & DG_PRIM2_WORLDMTX_UNITROT )
		{
			/* WORLDマトリクスが単位回転 */
			uvec_w = _cam_unit_vec_x ;
			uvec_h = _cam_unit_vec_y ;
		}
		else
		{
			FVECTOR	vec ;
			FMATRIX	mtx ;

			/* WORLDマトリクス補正 */
			mtx = prim->world ;
			vec = _cam_unit_vec_x ;
			uvec_w.vx = (vec.vx * mtx.m[0][0])
					  + (vec.vy * mtx.m[0][1])
					  + (vec.vz * mtx.m[0][2]) ;
			uvec_w.vy = (vec.vx * mtx.m[1][0])
					  + (vec.vy * mtx.m[1][1])
					  + (vec.vz * mtx.m[1][2]) ;
			uvec_w.vz = (vec.vx * mtx.m[2][0])
					  + (vec.vy * mtx.m[2][1])
					  + (vec.vz * mtx.m[2][2]) ;
			uvec_w.vw = 0.0f ;

			vec = _cam_unit_vec_y ;
			uvec_h.vx = (vec.vx * mtx.m[0][0])
					  + (vec.vy * mtx.m[0][1])
					  + (vec.vz * mtx.m[0][2]) ;
			uvec_h.vy = (vec.vx * mtx.m[1][0])
					  + (vec.vy * mtx.m[1][1])
					  + (vec.vz * mtx.m[1][2]) ;
			uvec_h.vz = (vec.vx * mtx.m[2][0])
					  + (vec.vy * mtx.m[2][1])
					  + (vec.vz * mtx.m[2][2]) ;
			uvec_h.vw = 0.0f ;
		}
	}

	pos     = packet->pos_addr ;
	uvrgbwh = packet->uvrgb_addr ;
	vertex  = packet->vbuff ;		/* 頂点バッファ */


	/* 頂点バッファ生成 */
	if( packet->n_verts == 1 )
	{
		cur_pos = *pos ;
		{
			float	w, h ;
			FVECTOR	ww, hh ;

			w = (float)uvrgbwh->w ;
			h = (float)uvrgbwh->h ;

			ww.vx = uvec_w.vx * w ;
			ww.vy = uvec_w.vy * w ;
			ww.vz = uvec_w.vz * w ;
			hh.vx = uvec_h.vx * h ;
			hh.vy = uvec_h.vy * h ;
			hh.vz = uvec_h.vz * h ;

			vec[1].vx =  ww.vx - hh.vx ;
			vec[1].vy =  ww.vy - hh.vy ;
			vec[1].vz =  ww.vz - hh.vz ;
			vec[0].vx = -ww.vx - hh.vx ;
			vec[0].vy = -ww.vy - hh.vy ;
			vec[0].vz = -ww.vz - hh.vz ;
			vec[2].vx = cur_pos.vx - vec[1].vx ;
			vec[2].vy = cur_pos.vy - vec[1].vy ;
			vec[2].vz = cur_pos.vz - vec[1].vz ;
			vec[3].vx = cur_pos.vx - vec[0].vx ;
			vec[3].vy = cur_pos.vy - vec[0].vy ;
			vec[3].vz = cur_pos.vz - vec[0].vz ;
			vec[0].vx += cur_pos.vx ;
			vec[0].vy += cur_pos.vy ;
			vec[0].vz += cur_pos.vz ;
			vec[1].vx += cur_pos.vx ;
			vec[1].vy += cur_pos.vy ;
			vec[1].vz += cur_pos.vz ;
		}

		/* 0 */
		vertex->vx   = vec[0].vx ;
		vertex->vy   = vec[0].vy ;
		vertex->vz   = vec[0].vz ;
		vertex++ ;
		/* 1 */
		vertex->vx   = vec[1].vx ;
		vertex->vy   = vec[1].vy ;
		vertex->vz   = vec[1].vz ;
		vertex++ ;
		/* 2 */
		vertex->vx   = vec[2].vx ;
		vertex->vy   = vec[2].vy ;
		vertex->vz   = vec[2].vz ;
		vertex++ ;
		/* 3 */
		vertex->vx   = vec[3].vx ;
		vertex->vy   = vec[3].vy ;
		vertex->vz   = vec[3].vz ;
		vertex++ ;
	}
	else
	{
		for ( i = packet->n_verts ; i > 0 ; i--, pos++, uvrgbwh++ ){
			cur_pos     = *pos ;

			{
				float	w, h ;
				FVECTOR	ww, hh ;

				w = (float)uvrgbwh->w ;
				h = (float)uvrgbwh->h ;

				ww.vx = uvec_w.vx * w ;
				ww.vy = uvec_w.vy * w ;
				ww.vz = uvec_w.vz * w ;
				hh.vx = uvec_h.vx * h ;
				hh.vy = uvec_h.vy * h ;
				hh.vz = uvec_h.vz * h ;

				vec[1].vx =  ww.vx - hh.vx ;
				vec[1].vy =  ww.vy - hh.vy ;
				vec[1].vz =  ww.vz - hh.vz ;
				vec[0].vx = -ww.vx - hh.vx ;
				vec[0].vy = -ww.vy - hh.vy ;
				vec[0].vz = -ww.vz - hh.vz ;
				vec[2].vx = cur_pos.vx - vec[1].vx ;
				vec[2].vy = cur_pos.vy - vec[1].vy ;
				vec[2].vz = cur_pos.vz - vec[1].vz ;
				vec[3].vx = cur_pos.vx - vec[0].vx ;
				vec[3].vy = cur_pos.vy - vec[0].vy ;
				vec[3].vz = cur_pos.vz - vec[0].vz ;
				vec[0].vx += cur_pos.vx ;
				vec[0].vy += cur_pos.vy ;
				vec[0].vz += cur_pos.vz ;
				vec[1].vx += cur_pos.vx ;
				vec[1].vy += cur_pos.vy ;
				vec[1].vz += cur_pos.vz ;
			}

			/* 0 */
			vertex->vx   = vec[0].vx ;
			vertex->vy   = vec[0].vy ;
			vertex->vz   = vec[0].vz ;
			vertex++ ;
			/* 1 */
			vertex->vx   = vec[1].vx ;
			vertex->vy   = vec[1].vy ;
			vertex->vz   = vec[1].vz ;
			vertex[2]    = *vertex ;	// /* 3 */
			vertex++ ;
			/* 2 */
			vertex->vx   = vec[2].vx ;
			vertex->vy   = vec[2].vy ;
			vertex->vz   = vec[2].vz ;
			vertex[2]    = *vertex ;	// /* 4 */
			vertex += 3 ;	// --> /* 5 */
			/* 5 */
			vertex->vx   = vec[3].vx ;
			vertex->vy   = vec[3].vy ;
			vertex->vz   = vec[3].vz ;
			vertex++ ;
		}
	}
}


static int DG_MakePrimPacksVBuffer_RSprt( DG_PRIM2 *prim )
{
	FVECTOR				*pos ;
	FVECTOR				cur_pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	DG_PRIM2_UVRGBWH	cur_uvrgbwh ;
	int					i, j ;
	u_int				rgba ;
	DG_VERTEX_PRIM2SPRT	*vertex ;

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2SPRT),
								prim->packet_verts * 6 * prim->n_prims );

	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;
	/* パケット処理開始 */
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		pos = packet->pos_addr ;
		uvrgbwh = packet->uvrgb_addr ;
		packet->vbuff = vertex ;		/* 頂点バッファ先頭記録 */
		/* 頂点バッファ生成 */

		if( packet->n_verts == 1 )
		{
			PREFETCH( pos );
			PREFETCH( uvrgbwh );
			rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
			cur_pos     = *pos ;
			cur_uvrgbwh = *uvrgbwh ;

			/* 0 */
			vertex->vx = cur_pos.vx ;
			vertex->vy = cur_pos.vy ;
			vertex->vz = cur_pos.vz ;
			vertex->dx = -cur_uvrgbwh.w ;
			vertex->dy = -cur_uvrgbwh.h ;
			vertex->rgba = rgba ;
			vertex->u = cur_uvrgbwh.u0 ;
			vertex->v = cur_uvrgbwh.v0 ;
			//vertex->q = 1.0f ;
			vertex++ ;
			/* 1 */
			vertex->vx = cur_pos.vx ;
			vertex->vy = cur_pos.vy ;
			vertex->vz = cur_pos.vz ;
			vertex->dx =  cur_uvrgbwh.h ;
			vertex->dy = -cur_uvrgbwh.w ;
			vertex->rgba = rgba ;
			vertex->u = cur_uvrgbwh.u1 ;
			vertex->v = cur_uvrgbwh.v0 ;
			//vertex->q = 1.0f ;
			vertex++ ;
			/* 2 */
			vertex->vx = cur_pos.vx ;
			vertex->vy = cur_pos.vy ;
			vertex->vz = cur_pos.vz ;
			vertex->dx = -cur_uvrgbwh.h ;
			vertex->dy =  cur_uvrgbwh.w ;
			vertex->rgba = rgba ;
			vertex->u = cur_uvrgbwh.u0 ;
			vertex->v = cur_uvrgbwh.v1 ;
			//vertex->q = 1.0f ;
			vertex++ ;
			/* 3 */
			vertex->vx = cur_pos.vx ;
			vertex->vy = cur_pos.vy ;
			vertex->vz = cur_pos.vz ;
			vertex->dx =  cur_uvrgbwh.w ;
			vertex->dy =  cur_uvrgbwh.h ;
			vertex->rgba = rgba ;
			vertex->u = cur_uvrgbwh.u1 ;
			vertex->v = cur_uvrgbwh.v1 ;
			//vertex->q = 1.0f ;
			vertex++ ;
		}
		else
		{
			for ( i = packet->n_verts; i > 0 ; i--, pos++, uvrgbwh++ ){
				PREFETCH( pos );
				PREFETCH( uvrgbwh );
				rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
				cur_pos     = *pos ;
				cur_uvrgbwh = *uvrgbwh ;

				/* 0 */
				vertex->vx = cur_pos.vx ;
				vertex->vy = cur_pos.vy ;
				vertex->vz = cur_pos.vz ;
				vertex->dx = -cur_uvrgbwh.w ;
				vertex->dy = -cur_uvrgbwh.h ;
				vertex->rgba = rgba ;
				vertex->u = cur_uvrgbwh.u0 ;
				vertex->v = cur_uvrgbwh.v0 ;
				//vertex->q = 1.0f ;
				vertex++ ;
				/* 1 */
				vertex->vx = cur_pos.vx ;
				vertex->vy = cur_pos.vy ;
				vertex->vz = cur_pos.vz ;
				vertex->dx =  cur_uvrgbwh.h ;
				vertex->dy = -cur_uvrgbwh.w ;
				vertex->rgba = rgba ;
				vertex->u = cur_uvrgbwh.u1 ;
				vertex->v = cur_uvrgbwh.v0 ;
				//vertex->q = 1.0f ;
				vertex[2]    = *vertex ;	// /* 3 */
				vertex++ ;
				/* 2 */
				vertex->vx = cur_pos.vx ;
				vertex->vy = cur_pos.vy ;
				vertex->vz = cur_pos.vz ;
				vertex->dx = -cur_uvrgbwh.h ;
				vertex->dy =  cur_uvrgbwh.w ;
				vertex->rgba = rgba ;
				vertex->u = cur_uvrgbwh.u0 ;
				vertex->v = cur_uvrgbwh.v1 ;
				//vertex->q = 1.0f ;
				vertex[2]   = *vertex ;	// /* 4 */
				vertex     += 3 ;	// --> /* 5 */
				/* 5 */
				vertex->vx = cur_pos.vx ;
				vertex->vy = cur_pos.vy ;
				vertex->vz = cur_pos.vz ;
				vertex->dx =  cur_uvrgbwh.w ;
				vertex->dy =  cur_uvrgbwh.h ;
				vertex->rgba = rgba ;
				vertex->u = cur_uvrgbwh.u1 ;
				vertex->v = cur_uvrgbwh.v1 ;
				//vertex->q = 1.0f ;
				vertex++ ;
			}
		}
	}
	return(0) ;
}

static int DG_MakePrimPacksVBuffer_RSprt_NVS( DG_PRIM2 *prim )
{
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	int					i,j ;
	DWORD				rgba ;
	float				u0, v0, u1, v1 ;
	FVECTOR				uvec_w, uvec_h ;
	DG_VERTEX_PRIM2SPRT_FVF	*vertex ;

	/* Screen上の物は動作が違う */
	if( prim->flag & DG_PRIM2_ON_CAMERA )
	{
		return( DG_MakePrimPacksVBuffer_ScreenRSprt_NVS(prim) ) ;
	}

	/* World Matrix検査 */
	if(  (prim->world.m[0][0] != 1.0f)
	  || (prim->world.m[0][1] != 0.0f)
	  || (prim->world.m[0][2] != 0.0f)
	  || (prim->world.m[1][0] != 0.0f)
	  || (prim->world.m[1][1] != 1.0f)
	  || (prim->world.m[1][2] != 0.0f)
	  || (prim->world.m[2][0] != 0.0f)
	  || (prim->world.m[2][1] != 0.0f)
	  || (prim->world.m[2][2] != 0.0f) )
	{
		prim->flag &= ~DG_PRIM2_WORLDMTX_UNITROT ;
	}
	else
	{
		prim->flag |= DG_PRIM2_WORLDMTX_UNITROT ;
	}

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2SPRT_FVF),
								prim->packet_verts * 6 * prim->n_prims );

	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;
	/* パケット処理開始 */
	uvec_w = _cam_unit_vec_x ;
	uvec_h = _cam_unit_vec_y ;
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		uvrgbwh = packet->uvrgb_addr ;
		packet->vbuff = vertex ;		/* 頂点バッファ先頭記録 */
		/* 頂点バッファ生成 */

		if( packet->n_verts == 1 )
		{
			rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
			u0   = uvrgbwh->u0 ;
			v0   = uvrgbwh->v0 ;
			u1   = uvrgbwh->u1 ;
			v1   = uvrgbwh->v1 ;

			/* 0 */
			vertex->rgba = rgba ;
			vertex->u    = u0 ;
			vertex->v    = v0 ;
			vertex++ ;
			/* 1 */
			vertex->rgba = rgba ;
			vertex->u    = u1 ;
			vertex->v    = v0 ;
			vertex++ ;
			/* 2 */
			vertex->rgba = rgba ;
			vertex->u    = u0 ;
			vertex->v    = v1 ;
			vertex++ ;
			/* 3 */
			vertex->rgba = rgba ;
			vertex->u    = u1 ;
			vertex->v    = v1 ;
			vertex++ ;
		}
		else
		{
			for ( i = packet->n_verts; i>0 ; i--, uvrgbwh++ ){
				rgba = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
				u0   = uvrgbwh->u0 ;
				v0   = uvrgbwh->v0 ;
				u1   = uvrgbwh->u1 ;
				v1   = uvrgbwh->v1 ;

				/* 0 */
				vertex->rgba = rgba ;
				vertex->u    = u0 ;
				vertex->v    = v0 ;
				vertex++ ;
				/* 1 */
				vertex->rgba = rgba ;
				vertex->u    = u1 ;
				vertex->v    = v0 ;
				vertex[2]    = *vertex ;	// /* 3 */
				vertex++ ;
				/* 2 */
				vertex->rgba = rgba ;
				vertex->u    = u0 ;
				vertex->v    = v1 ;
				vertex[2]    = *vertex ;	// /* 4 */
				vertex += 3 ;	// --> /* 5 */
				/* 5 */
				vertex->rgba = rgba ;
				vertex->u    = u1 ;
				vertex->v    = v1 ;
				vertex++ ;
			}
		}
	}
	return(0) ;
}

static int DG_MakePrimPacksVBuffer_ScreenRSprt_NVS( DG_PRIM2 *prim )
{
	FVECTOR				*pos ;
	FVECTOR				cur_pos ;
	DG_PRIM2_PACKET		*packet ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	float				u0, v0, u1, v1 ;
	int					i, j ;
	u_int				rgba ;
	float				w, h ;
	DG_VERTEX_PRIM2SPRT_FVF	*vertex ;

	/* 動的頂点バッファを確保 */
	vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_PRIM2SPRT_FVF),
								prim->packet_verts * 6 * prim->n_prims );

	if ( vertex == NULL ) return ( -1 );
	prim->vbuff = vertex ;
	/* パケット処理開始 */
	packet = prim->packet[ prim->buffer_clock ] ;
	for ( j = prim->n_prims ; j > 0 ; j--, packet++ ){
		pos = packet->pos_addr ;
		uvrgbwh = packet->uvrgb_addr ;
		packet->vbuff = vertex ;		/* 頂点バッファ先頭記録 */
		/* 頂点バッファ生成 */

		if( packet->n_verts == 1 )
		{
			rgba    = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
			cur_pos = *pos ;
			w       = (float)uvrgbwh->w ;
			h       = (float)uvrgbwh->h ;

			u0 = uvrgbwh->u0 ;
			v0 = uvrgbwh->v0 ;
			u1 = uvrgbwh->u1 ;
			v1 = uvrgbwh->v1 ;

			/* 0 */
			vertex->vx   = cur_pos.vx - w ;
			vertex->vy   = cur_pos.vy - h ;
			vertex->vz   = cur_pos.vz ;
			vertex->rgba = rgba ;
			vertex->u    = u0 ;
			vertex->v    = v0 ;
			vertex++ ;
			/* 1 */
			vertex->vx   = cur_pos.vx + h ;
			vertex->vy   = cur_pos.vy - w ;
			vertex->vz   = cur_pos.vz ;
			vertex->rgba = rgba ;
			vertex->u    = u1 ;
			vertex->v    = v0 ;
			vertex++ ;
			/* 2 */
			vertex->vx   = cur_pos.vx - h ;
			vertex->vy   = cur_pos.vy + w ;
			vertex->vz   = cur_pos.vz ;
			vertex->rgba = rgba ;
			vertex->u    = u0 ;
			vertex->v    = v1 ;
			vertex++ ;
			/* 3 */
			vertex->vx   = cur_pos.vx + w ;
			vertex->vy   = cur_pos.vy + h ;
			vertex->vz   = cur_pos.vz ;
			vertex->rgba = rgba ;
			vertex->u    = u1 ;
			vertex->v    = v1 ;
			vertex++ ;
		}
		else
		{
			for ( i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ ){
				rgba    = SET_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
				cur_pos = *pos ;
				w       = (float)uvrgbwh->w ;
				h       = (float)uvrgbwh->h ;

				u0 = uvrgbwh->u0 ;
				v0 = uvrgbwh->v0 ;
				u1 = uvrgbwh->u1 ;
				v1 = uvrgbwh->v1 ;

				/* 0 */
				vertex->vx   = cur_pos.vx - w ;
				vertex->vy   = cur_pos.vy - h ;
				vertex->vz   = cur_pos.vz ;
				vertex->rgba = rgba ;
				vertex->u    = u0 ;
				vertex->v    = v0 ;
				vertex++ ;
				/* 1 */
				vertex->vx   = cur_pos.vx + h ;
				vertex->vy   = cur_pos.vy - w ;
				vertex->vz   = cur_pos.vz ;
				vertex->rgba = rgba ;
				vertex->u    = u1 ;
				vertex->v    = v0 ;
				vertex[2]    = *vertex ;	// /* 3 */
				vertex++ ;
				/* 2 */
				vertex->vx   = cur_pos.vx - h ;
				vertex->vy   = cur_pos.vy + w ;
				vertex->vz   = cur_pos.vz ;
				vertex->rgba = rgba ;
				vertex->u    = u0 ;
				vertex->v    = v1 ;
				vertex[2]    = *vertex ;	// /* 4 */
				vertex += 3 ;	// --> /* 5 */
				/* 5 */
				vertex->vx   = cur_pos.vx + w ;
				vertex->vy   = cur_pos.vy + h ;
				vertex->vz   = cur_pos.vz ;
				vertex->rgba = rgba ;
				vertex->u    = u1 ;
				vertex->v    = v1 ;
				vertex++ ;
			}
		}
	}
	return(0) ;
}

static void DG_RemakePrimPacksVBuffer_RSprt_NVS( DG_PRIM2_PACKET *packet )
{
	FVECTOR				*pos ;
	FVECTOR				cur_pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	int					i ;
	FVECTOR				vec[4] ;
	DG_VERTEX_PRIM2SPRT_FVF	*vertex ;
	FVECTOR	uvec_w, uvec_h ;

	/* パケット処理開始 */

	{
		DG_PRIM2	*prim ;

		prim = packet->prim ;
		if( prim->flag & DG_PRIM2_WORLDMTX_UNITROT )
		{
			/* WORLDマトリクスが単位回転 */
			uvec_w = _cam_unit_vec_x ;
			uvec_h = _cam_unit_vec_y ;
		}
		else
		{
			FVECTOR	vec ;
			FMATRIX	mtx ;

			/* WORLDマトリクス補正 */
			mtx = prim->world ;
			vec = _cam_unit_vec_x ;
			uvec_w.vx = (vec.vx * mtx.m[0][0])
					  + (vec.vy * mtx.m[0][1])
					  + (vec.vz * mtx.m[0][2]) ;
			uvec_w.vy = (vec.vx * mtx.m[1][0])
					  + (vec.vy * mtx.m[1][1])
					  + (vec.vz * mtx.m[1][2]) ;
			uvec_w.vz = (vec.vx * mtx.m[2][0])
					  + (vec.vy * mtx.m[2][1])
					  + (vec.vz * mtx.m[2][2]) ;
			uvec_w.vw = 0.0f ;

			vec = _cam_unit_vec_y ;
			uvec_h.vx = (vec.vx * mtx.m[0][0])
					  + (vec.vy * mtx.m[0][1])
					  + (vec.vz * mtx.m[0][2]) ;
			uvec_h.vy = (vec.vx * mtx.m[1][0])
					  + (vec.vy * mtx.m[1][1])
					  + (vec.vz * mtx.m[1][2]) ;
			uvec_h.vz = (vec.vx * mtx.m[2][0])
					  + (vec.vy * mtx.m[2][1])
					  + (vec.vz * mtx.m[2][2]) ;
			uvec_h.vw = 0.0f ;
		}
	}

	pos     = packet->pos_addr ;
	uvrgbwh = packet->uvrgb_addr ;
	vertex  = packet->vbuff ;

	/* 頂点バッファ生成 */
	if( packet->n_verts == 1 )
	{
		cur_pos = *pos ;

		{
			float	w, h ;
			FVECTOR	ww, wh, hw, hh ;

			w = (float)uvrgbwh->w ;
			h = (float)uvrgbwh->h ;

			ww.vx = uvec_w.vx * w ;
			ww.vy = uvec_w.vy * w ;
			ww.vz = uvec_w.vz * w ;
			wh.vx = uvec_w.vx * h ;
			wh.vy = uvec_w.vy * h ;
			wh.vz = uvec_w.vz * h ;
			hw.vx = uvec_h.vx * w ;
			hw.vy = uvec_h.vy * w ;
			hw.vz = uvec_h.vz * w ;
			hh.vx = uvec_h.vx * h ;
			hh.vy = uvec_h.vy * h ;
			hh.vz = uvec_h.vz * h ;

			vec[0].vx = cur_pos.vx + (-ww.vx - hh.vx) ;
			vec[0].vy = cur_pos.vy + (-ww.vy - hh.vy) ;
			vec[0].vz = cur_pos.vz + (-ww.vz - hh.vz) ;
			vec[1].vx = cur_pos.vx + ( wh.vx - hw.vx) ;
			vec[1].vy = cur_pos.vy + ( wh.vy - hw.vy) ;
			vec[1].vz = cur_pos.vz + ( wh.vz - hw.vz) ;
			vec[2].vx = cur_pos.vx + (-wh.vx + hw.vx) ;
			vec[2].vy = cur_pos.vy + (-wh.vy + hw.vy) ;
			vec[2].vz = cur_pos.vz + (-wh.vz + hw.vz) ;
			vec[3].vx = cur_pos.vx + ( ww.vx + hh.vx) ;
			vec[3].vy = cur_pos.vy + ( ww.vy + hh.vy) ;
			vec[3].vz = cur_pos.vz + ( ww.vz + hh.vz) ;
		}

		/* 0 */
		vertex->vx   = vec[0].vx ;
		vertex->vy   = vec[0].vy ;
		vertex->vz   = vec[0].vz ;
		vertex++ ;
		/* 1 */
		vertex->vx   = vec[1].vx ;
		vertex->vy   = vec[1].vy ;
		vertex->vz   = vec[1].vz ;
		vertex++ ;
		/* 2 */
		vertex->vx   = vec[2].vx ;
		vertex->vy   = vec[2].vy ;
		vertex->vz   = vec[2].vz ;
		vertex++ ;
		/* 3 */
		vertex->vx   = vec[3].vx ;
		vertex->vy   = vec[3].vy ;
		vertex->vz   = vec[3].vz ;
		vertex++ ;
	}
	else
	{
		for ( i = packet->n_verts; i>0 ; i--, pos++, uvrgbwh++ ){
			cur_pos = *pos ;
			{
				float	w, h ;
				FVECTOR	ww, wh, hw, hh ;

				w = (float)uvrgbwh->w ;
				h = (float)uvrgbwh->h ;

				ww.vx = uvec_w.vx * w ;
				ww.vy = uvec_w.vy * w ;
				ww.vz = uvec_w.vz * w ;
				wh.vx = uvec_w.vx * h ;
				wh.vy = uvec_w.vy * h ;
				wh.vz = uvec_w.vz * h ;
				hw.vx = uvec_h.vx * w ;
				hw.vy = uvec_h.vy * w ;
				hw.vz = uvec_h.vz * w ;
				hh.vx = uvec_h.vx * h ;
				hh.vy = uvec_h.vy * h ;
				hh.vz = uvec_h.vz * h ;

				vec[0].vx = cur_pos.vx + (-ww.vx - hh.vx) ;
				vec[0].vy = cur_pos.vy + (-ww.vy - hh.vy) ;
				vec[0].vz = cur_pos.vz + (-ww.vz - hh.vz) ;
				vec[1].vx = cur_pos.vx + ( wh.vx - hw.vx) ;
				vec[1].vy = cur_pos.vy + ( wh.vy - hw.vy) ;
				vec[1].vz = cur_pos.vz + ( wh.vz - hw.vz) ;
				vec[2].vx = cur_pos.vx + (-wh.vx + hw.vx) ;
				vec[2].vy = cur_pos.vy + (-wh.vy + hw.vy) ;
				vec[2].vz = cur_pos.vz + (-wh.vz + hw.vz) ;
				vec[3].vx = cur_pos.vx + ( ww.vx + hh.vx) ;
				vec[3].vy = cur_pos.vy + ( ww.vy + hh.vy) ;
				vec[3].vz = cur_pos.vz + ( ww.vz + hh.vz) ;
			}

			/* 0 */
			vertex->vx   = vec[0].vx ;
			vertex->vy   = vec[0].vy ;
			vertex->vz   = vec[0].vz ;
			vertex++ ;
			/* 1 */
			vertex->vx   = vec[1].vx ;
			vertex->vy   = vec[1].vy ;
			vertex->vz   = vec[1].vz ;
			vertex[2]    = *vertex ;	// /* 3 */
			vertex++ ;
			/* 2 */
			vertex->vx   = vec[2].vx ;
			vertex->vy   = vec[2].vy ;
			vertex->vz   = vec[2].vz ;
			vertex[2]    = *vertex ;	// /* 4 */
			vertex += 3 ;	// --> /* 5 */
			/* 5 */
			vertex->vx   = vec[3].vx ;
			vertex->vy   = vec[3].vy ;
			vertex->vz   = vec[3].vz ;
			vertex++ ;
		}
	}
}

static void DG_WritePrimPacks_Line( DG_PRIM2_PACKET *packet )
{
	static unsigned short	Index[ 64 * 3 ] ;
	DWORD					sofs, idx_sofs ;

	if( !packet->n_indices ){ return ; }

#if FALSE
	DG_SetVertexShaderConstant(CV_SCREEN, &(((DG_PRIM2 *)packet->prim)->screen), 4 );
#endif
	DG_SelectVertexShader( &DG_Prim2VertexShader[0] );

	if( DG_CheckIndexPrimitiveUseable() )
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2POLY),
							packet->n_verts, &sofs) ;
		DG_SetDynamicIndexBuffer(packet->ibuff, packet->n_indices, sofs, &idx_sofs) ;
		DG_DrawIndexedPrimitive(D3DPT_LINELIST, 0, packet->n_verts,
						idx_sofs, packet->n_indices>>1) ;
	}
	else
	{
		/*-- Index未対応版 --*/
		BYTE	*dst_vbuff ;
		DWORD	size ;

		size = sizeof(DG_VERTEX_PRIM2POLY) * packet->n_indices ;
		ASSERT( size < DG_GetWorkBufferSize() ) ;

		if( size < DG_GetWorkBufferSize() )
		{
			dst_vbuff = DG_GetWorkBuffer() ;

			DG_ArrangeVertexByIndex(dst_vbuff,
					(void *)packet->vbuff, sizeof(DG_VERTEX_PRIM2POLY),
					(void *)packet->ibuff, packet->n_indices) ;	// 頂点の整列

			DG_SetDynamicVertexBuffer(dst_vbuff, sizeof(DG_VERTEX_PRIM2POLY),
							packet->n_indices, &sofs) ;
			DG_DrawPrimitive(D3DPT_LINELIST, sofs, packet->n_indices>>1) ;
		}
	}
}

static void DG_WritePrimPacks_Line_NVS( DG_PRIM2_PACKET *packet )
{
	static unsigned short	Index[ 64 * 3 ] ;
	DWORD					sofs, idx_sofs ;

	if( !packet->n_indices ){ return ; }

#if FALSE
#if FALSE
	DG_SetTransform(D3DTS_WORLD, &(((DG_PRIM2 *)packet->prim)->screen)) ;
#else
	DG_SetTransform(D3DTS_WORLD, &DG_UnitMatrix) ;
#endif
#endif
	DG_SetVertexShader( D3DFVF_DG_VERTEX_PRIM2POLY_FLAG );

	if( DG_CheckIndexPrimitiveUseable() )
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2POLY_FVF),
							packet->n_verts, &sofs) ;
		DG_SetDynamicIndexBuffer(packet->ibuff, packet->n_indices, sofs, &idx_sofs) ;
		DG_DrawIndexedPrimitive(D3DPT_LINELIST, 0, packet->n_verts,
						idx_sofs, packet->n_indices>>1) ;
	}
	else
	{
		/*-- Index未対応版 --*/
		BYTE	*dst_vbuff ;
		DWORD	size ;

		size = sizeof(DG_VERTEX_PRIM2POLY_FVF) * packet->n_indices ;
		ASSERT( size < DG_GetWorkBufferSize() ) ;
		ASSERT( packet->n_indices > 0 ) ;

		if( size < DG_GetWorkBufferSize() )
		{
			dst_vbuff = DG_GetWorkBuffer() ;

			DG_ArrangeVertexByIndex(dst_vbuff,
					(void *)packet->vbuff, sizeof(DG_VERTEX_PRIM2POLY_FVF),
					(void *)packet->ibuff, packet->n_indices) ;	// 頂点の整列

			DG_SetDynamicVertexBuffer(dst_vbuff, sizeof(DG_VERTEX_PRIM2POLY_FVF),
							packet->n_indices, &sofs) ;
			DG_DrawPrimitive(D3DPT_LINELIST, sofs, packet->n_indices>>1) ;
		}
	}
}

static void DG_WritePrimPacks_Poly( DG_PRIM2_PACKET *packet )
{
	DG_PRIM2	*prim ;
	DWORD		sofs, idx_sofs ;

	prim = packet->prim ;

#if FALSE
	DG_SetVertexShaderConstant(CV_SCREEN, &prim->screen, 4);
#endif
	DG_SelectVertexShader( &DG_Prim2VertexShader[0] );

	if( DG_CheckIndexPrimitiveUseable() )
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2POLY),
							packet->n_verts, &sofs) ;
		DG_SetDynamicIndexBuffer(packet->ibuff, packet->n_indices, sofs, &idx_sofs) ;
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, packet->n_verts,
						idx_sofs, packet->n_indices - 2) ;
	}
	else
	{
		/*-- Index未対応版 --*/
		BYTE	*dst_vbuff ;
		DWORD	size ;

		size = sizeof(DG_VERTEX_PRIM2POLY) * packet->n_indices ;
		ASSERT( size < DG_GetWorkBufferSize() ) ;

		if( size < DG_GetWorkBufferSize() )
		{
			dst_vbuff = DG_GetWorkBuffer() ;

			DG_ArrangeVertexByIndex(dst_vbuff,
					(void *)packet->vbuff, sizeof(DG_VERTEX_PRIM2POLY),
					(void *)packet->ibuff, packet->n_indices) ;	// 頂点の整列

			DG_SetDynamicVertexBuffer(dst_vbuff, sizeof(DG_VERTEX_PRIM2POLY),
							packet->n_indices, &sofs) ;
			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, packet->n_indices - 2) ;

		}
	}
}

static void DG_WritePrimPacks_Poly_NVS( DG_PRIM2_PACKET *packet )
{
	DG_PRIM2	*prim ;
	DWORD		sofs, idx_sofs ;

	prim = packet->prim ;

#if FALSE
#if FALSE
	DG_SetTransform(D3DTS_WORLD, &prim->screen) ;
#else
	DG_SetTransform(D3DTS_WORLD, &DG_UnitMatrix) ;
#endif
#endif
	DG_SetVertexShader(D3DFVF_DG_VERTEX_PRIM2POLY_FLAG) ;

	if( DG_CheckIndexPrimitiveUseable() )
	{

		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2POLY_FVF),
							packet->n_verts, &sofs) ;
		DG_SetDynamicIndexBuffer(packet->ibuff, packet->n_indices, sofs, &idx_sofs) ;
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, packet->n_verts,
						idx_sofs, packet->n_indices - 2) ;
	}
	else
	{
		/*-- Index未対応版 --*/
		BYTE	*dst_vbuff ;
		DWORD	size ;

		size = sizeof(DG_VERTEX_PRIM2POLY_FVF) * packet->n_indices ;
		ASSERT( size < DG_GetWorkBufferSize() ) ;

		if( size < DG_GetWorkBufferSize() )
		{
			dst_vbuff = DG_GetWorkBuffer() ;

			DG_ArrangeVertexByIndex(dst_vbuff,
					(void *)packet->vbuff, sizeof(DG_VERTEX_PRIM2POLY_FVF),
					(void *)packet->ibuff, packet->n_indices) ;	// 頂点の整列

			DG_SetDynamicVertexBuffer(dst_vbuff, sizeof(DG_VERTEX_PRIM2POLY_FVF),
							packet->n_indices, &sofs) ;
			DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, packet->n_indices - 2) ;

		}
	}
}

static void DG_WritePrimPacks_Sprt( DG_PRIM2_PACKET *packet )
{
	int			n_verts ;
	DWORD		sofs ;
	DG_PRIM2	*prim ;

	prim = packet->prim ;

#if FALSE
	DG_SetVertexShaderConstant(CV_SCREEN, &prim->screen, 4);
#endif
	DG_SelectVertexShader( &DG_Prim2VertexShader[1] );

	n_verts = packet->n_verts ;
	if( n_verts == 1 )
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2SPRT),
							4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	else
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2SPRT),
							n_verts * 6, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, packet->n_verts << 1) ;
	}
}

static void DG_WritePrimPacks_Sprt_NVS( DG_PRIM2_PACKET *packet )
{
	int			n_verts ;
	DWORD		sofs ;
	DG_PRIM2	*prim ;

	prim = packet->prim ;

	/* Screen上以外の物は頂点再設定の必要有り */
	if( !(prim->flag & DG_PRIM2_ON_CAMERA) )
	{
		DG_RemakePrimPacksVBuffer_Sprt_NVS(packet) ;
	}

	DG_SetVertexShader( D3DFVF_DG_VERTEX_PRIM2SPRT_FLAG );

	n_verts = packet->n_verts ;
	if( n_verts == 1 )
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2SPRT_FVF),
							4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	else
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2SPRT_FVF),
							n_verts * 6, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, packet->n_verts << 1) ;
	}
}

static void DG_WritePrimPacks_RSprt_NVS( DG_PRIM2_PACKET *packet )
{
	int			n_verts ;
	DWORD		sofs ;
	DG_PRIM2	*prim ;

	prim = packet->prim ;

	/* Screen上以外の物は頂点再設定の必要有り */
	if( !(prim->flag & DG_PRIM2_ON_CAMERA) )
	{
		DG_RemakePrimPacksVBuffer_RSprt_NVS(packet) ;
	}

	DG_SetVertexShader( D3DFVF_DG_VERTEX_PRIM2SPRT_FLAG );

	n_verts = packet->n_verts ;
	if( n_verts == 1 )
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2SPRT_FVF),
							4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	else
	{
		DG_SetDynamicVertexBuffer(packet->vbuff, sizeof(DG_VERTEX_PRIM2SPRT_FVF),
							n_verts * 6, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLELIST, sofs, packet->n_verts << 1) ;
	}
}

void DG_InitWritePrimPacks(DG_CHANL *cp)
{
	if( !DG_CheckUseVertexShader() )
	{
		FVECTOR		vec ;

		/* カメラ座標空間X,Y軸単位ベクトル-->WORLD座標空間 */

		vec.vx = 1.0f ;
		vec.vy = 0.0f ;
		vec.vz = 0.0f ;
		vec.vw = 0.0f ;
		_sceVu0ApplyMatrix(&_cam_unit_vec_x, &cp->eye, &vec) ;
		vec.vx = 0.0f ;
		vec.vy = 1.0f ;
		_sceVu0ApplyMatrix(&_cam_unit_vec_y, &cp->eye, &vec) ;
	}

}


static	void	(*_write_prim_packs_func_tbl[])(DG_PRIM2_PACKET *) =
{
	NULL,
	DG_WritePrimPacks_Line,	// DG_PRIM2_LINE(0x00000001)
	DG_WritePrimPacks_Poly,	// DG_PRIM2_POLY(0x00000002)
	DG_WritePrimPacks_Poly,	// DG_PRIM2_CULLPOLY(0x00000003)
	DG_WritePrimPacks_Sprt,	// DG_PRIM2_SPRT(0x00000004)
	DG_WritePrimPacks_Sprt,	// DG_PRIM2_RSPRT(0x00000005)
} ;
#define	WRITE_PRIM_PACKS_FUNC_TBL_SIZE	(sizeof(_write_prim_packs_func_tbl)/sizeof(_write_prim_packs_func_tbl[0]))

static	void	(*_write_prim_packs_func_nvs_tbl[])(DG_PRIM2_PACKET *) =
{
	NULL,
	DG_WritePrimPacks_Line_NVS,	// DG_PRIM2_LINE(0x00000001)
	DG_WritePrimPacks_Poly_NVS,	// DG_PRIM2_POLY(0x00000002)
	DG_WritePrimPacks_Poly_NVS,	// DG_PRIM2_CULLPOLY(0x00000003)
	DG_WritePrimPacks_Sprt_NVS,	// DG_PRIM2_SPRT(0x00000004)
	DG_WritePrimPacks_RSprt_NVS,// DG_PRIM2_RSPRT(0x00000005)
} ;

static FVECTOR	tex_param[] = {
	{ 1.0f / 4096.0f, 1.0f / 4096.0f, 1, 1 },	/* テクスチャスケール */
	{ 0.0f, 0.0f, 0, 0 },	/* テクスチャオフセット */
};

static FMATRIX	tex_param_mtx =
{
	1.0f/4096.0f,	0.0f, 			0.0f,	0.0f,
	0.0f,			1.0f/4096.0f,	0.0f,	0.0f,
	0.0f,			0.0f,			1.0f,	0.0f,
	0.0f,			0.0f,			0.0f,	1.0f
} ;

int DG_WritePrimPacks( DG_PRIM2_PACKET *packet, DG_CHANL *addr, int which )
{
	static unsigned short	Index[ 64 * 3 ] ;
	DG_PRIM2				*prim ;
#if FALSE
	FVECTOR					*pos ;
#endif
	DG_CHANL 				*cp = addr ;
	DWORD					flag ;

	if ( packet->n_verts == 0 ) return ( 0 );
	prim = packet->prim ;
	flag = (DWORD)prim->flag ;

	/* テクスチャ設定 */
	if ( flag & DG_PRIM2_TEX ){
		//DG_SetTexture(0, &prim->tex_trans);
		DG_SetTextureDirect( 0, prim->tex_trans.ptex );
	} else {
		DG_SetTexture(0, NULL );
	}

	/* PRIM2用の描画環境設定 */
	if ( !( flag & ( DG_PRIM2_CW|DG_PRIM2_CCW ) ) ){
		DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	} else {
		if ( flag & DG_PRIM2_CW ){
			/* 時計周り */
			DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );	/* MGS2とDirectXは逆なので */
		} else {
			/* 反時計周り */
			DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );	/* MGS2とDirectXは逆なので */
		}
	}

	/* 半透明モードの指定 */
	if ( flag & ( DG_PRIM2_ALPHA | DG_PRIM2_ANTIALIASING ) ){
		DG_SetAlphaMode( prim->tex_trans.alpha.data );	/* DG_SetTexture()内で設定されているので */
		//DG_SetAlphaMode(SCE_GS_SET_ALPHA(0,2,0,1,0) & SCE_GS_ALPHA_MASK);
	} else {
		DG_SetAlphaMode(0);
	}
	/* フォグの有無 */
	if ( flag & DG_PRIM2_FOG ){
		DG_SetRenderState(D3DRS_FOGENABLE, TRUE );
	} else {
		DG_SetRenderState(D3DRS_FOGENABLE, FALSE );
	}

	/* フレームバッファテクスチャを使用 */
	{
		FVECTOR	*param ;

		if ( !( flag & DG_PRIM2_FRAMETEX ) ){
			param = tex_param;
		} else {
			param = frame_tex_param ;
		}

		DG_SetVertexShaderConstant( CV_TEX0_SCALE, param, 2 );
	}
	
	DG_SetPixelShader( NULL );

	{
		void	(*func)(DG_PRIM2_PACKET *) ;

		ASSERT( (prim->type > 0) && (prim->type < WRITE_PRIM_PACKS_FUNC_TBL_SIZE) )

		func = _write_prim_packs_func_tbl[prim->type] ;
		ASSERT( func ) ;

		func(packet) ;
	}

	return ( 0 );
}

int DG_WritePrimPacksNVS( DG_PRIM2_PACKET *packet, DG_CHANL *addr, int which )
{
	DG_PRIM2				*prim ;
#if FALSE
	FVECTOR					*pos ;
#endif
	DG_CHANL 				*cp = addr ;
	DWORD					flag ;

	if ( packet->n_verts == 0 ) return ( 0 );
	prim = packet->prim ;
	flag = (DWORD)prim->flag ;

	/* テクスチャ設定 */
	if ( flag & DG_PRIM2_TEX ){
		//DG_SetTextureNVS(0, &prim->tex_trans);
		DG_SetTextureDirect( 0, prim->tex_trans.ptex );
	} else {
		DG_SetTextureNVS(0, NULL );
	}

	/* PRIM2用の描画環境設定 */
	if ( !( flag & ( DG_PRIM2_CW|DG_PRIM2_CCW ) ) ){
		DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	} else {
		if ( flag & DG_PRIM2_CW ){
			/* 時計周り */
			DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );	/* MGS2とDirectXは逆なので */
		} else {
			/* 反時計周り */
			DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );	/* MGS2とDirectXは逆なので */
		}
	}

	/* 半透明モードの指定 */
	if ( flag & ( DG_PRIM2_ALPHA | DG_PRIM2_ANTIALIASING ) ){
		DG_SetAlphaMode( prim->tex_trans.alpha.data );	/* DG_SetTexture()内で設定されているので */
		//DG_SetAlphaMode(SCE_GS_SET_ALPHA(0,2,0,1,0) & SCE_GS_ALPHA_MASK);
	} else {
		DG_SetAlphaMode(0);
	}

	/* フォグの有無 */
	if ( flag & DG_PRIM2_FOG ){
		DG_SetRenderState(D3DRS_FOGENABLE, TRUE );
	} else {
		DG_SetRenderState(D3DRS_FOGENABLE, FALSE );
	}

	/* VertexShader非対応時の固有設定 */
	DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;

	/* フレームバッファテクスチャを使用 */
	{
		FMATRIX	*param ;

		/* VertexShader非対応版 */
		if ( !( flag & DG_PRIM2_FRAMETEX) ){ param = &tex_param_mtx ; }
		else{ param = &frame_tex_param_mtx ; }
		DG_SetTransformTexCoord(D3DTS_TEXTURE0, (void *)param) ;
	}
	
	DG_SetPixelShader( NULL );

	{
		void	(*func)(DG_PRIM2_PACKET *) ;

		ASSERT( (prim->type > 0) && (prim->type < WRITE_PRIM_PACKS_FUNC_TBL_SIZE) )

		func = _write_prim_packs_func_nvs_tbl[prim->type] ;
		ASSERT( func ) ;

		func(packet) ;
	}

	return ( 0 );
}

/* Primitive2接続(List形式) */
int DG_WritePrimPacksList( DG_PRIM2_PACKET *packet, DG_PRIM2_PACKET *term, DG_CHANL *addr, int which )
{
	DG_PRIM2	*prim ;
	DWORD		flag ;
	void		*cur_tex ;
	void		*last_tex ;
	DWORD		cur_cull ;
	DWORD		last_cull ;
	u_long		last_alpha ;
	u_long		cur_alpha ;
	DWORD		cur_fog ;
	DWORD		last_fog ;
	void		*cur_tex_param ;
	void		*last_tex_param ;
	BOOL		cur_on_camera ;
	BOOL		last_on_camera ;
	void		(*func)(DG_PRIM2_PACKET *) ;
	void		*cur_backbuffer_tex ;

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;

	DG_SetPixelShader( NULL );

	{
		float fog_start = 0.0f;
		float fog_end = 1.0f;
		DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
		DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
		DG_SetRenderState(D3DRS_FOGEND,   *(DWORD *)&fog_end);
	}
	DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;

	cur_backbuffer_tex = DG_BackBufferTexture[DG_CurrentBackBuffer] ;

	/* 最初のPacket処理 */
	while( packet != term )
	{
		/* 頂点無し判定 */
		if( !packet->n_verts )
		{
			packet = packet->next_addr ;
			continue ;
		}

		prim = packet->prim ;
		flag = (DWORD)prim->flag ;

		if( prim->tex_trans.ptex == cur_backbuffer_tex )
		{
			/* 41.09のドライバが何故か変な動作するので、現在の描画バッファは使用禁止 */
#ifdef DEBUG_MODE
			printf("[ Prim Warning ]Can't Use Current BackBuffer Texture Direct!!\n") ;
#endif
			packet = packet->next_addr ;
			continue ;
		}

		/*-- テクスチャ設定 ------------------------------------------*/

		if ( flag & DG_PRIM2_TEX ){
			last_tex = prim->tex_trans.ptex ;
		} else {
			last_tex = NULL ;
		}
		/*------------------------------------------------------------*/

		if ( !( flag & ( DG_PRIM2_CW|DG_PRIM2_CCW ) ) ){
			last_cull = D3DCULL_NONE ;
		} else {
			if ( flag & DG_PRIM2_CW ){
				/* 時計周り */
				last_cull = D3DCULL_CCW ;	/* MGS2とDirectXは逆なので */
			} else {
				/* 反時計周り */
				last_cull = D3DCULL_CW ;	/* MGS2とDirectXは逆なので */
			}
		}
		/*------------------------------------------------------------*/

		/*-- 半透明モードの指定 --------------------------------------*/

		if ( flag & ( DG_PRIM2_ALPHA | DG_PRIM2_ANTIALIASING ) ){
			last_alpha = prim->tex_trans.alpha.data ;
											/* DG_SetTexture()内で設定されているので */
		} else {
			last_alpha = 0 ;
		}
		/*------------------------------------------------------------*/

		/*-- フォグの有無 --------------------------------------------*/

		if ( flag & DG_PRIM2_FOG ){
			last_fog = TRUE ;
		} else {
			last_fog = FALSE ;
		}
		/*------------------------------------------------------------*/

		/*-- フレームバッファテクスチャを使用 ------------------------*/

		if ( !( flag & DG_PRIM2_FRAMETEX) ){ last_tex_param = &tex_param ; }
		else{ last_tex_param = &frame_tex_param ; }
		/*------------------------------------------------------------*/

		/*-- 変換基準 ------------------------------------------------*/

		last_on_camera = (flag & DG_PRIM2_ON_CAMERA) ;
		/*------------------------------------------------------------*/

		/*-- 描画環境構築 --------------------------------------------*/

		DG_SetTextureDirect(0, last_tex) ;
		DG_SetRenderState(D3DRS_CULLMODE, last_cull) ;
		DG_SetRenderState(D3DRS_FOGENABLE, last_fog) ;
		DG_SetAlphaMode(last_alpha) ;
		DG_SetVertexShaderConstant(CV_TEX0_SCALE, last_tex_param, 2) ;
		DG_SetVertexShaderConstant(CV_SCREEN, &prim->screen, 4);
		/*------------------------------------------------------------*/

		/*-- 関数実行 ------------------------------------------------*/

		func = _write_prim_packs_func_tbl[prim->type] ;
		ASSERT( func ) ;

		func(packet) ;
		/*------------------------------------------------------------*/

		packet = packet->next_addr ;
		break ;
	}

	/* 2番目以降のPacket */
	while( packet != term )
	{
		/* 頂点無し判定 */
		if( !packet->n_verts )
		{
			packet = packet->next_addr ;
			continue ;
		}

		prim = packet->prim ;
		flag = (DWORD)prim->flag ;

		if( prim->tex_trans.ptex == cur_backbuffer_tex )
		{
			/* 41.09のドライバが何故か変な動作するので、現在の描画バッファは使用禁止 */
#ifdef DEBUG_MODE
			printf("[ Prim Warning ]Can't Use Current BackBuffer Texture Direct!!\n") ;
#endif
			packet = packet->next_addr ;
			continue ;
		}

		/*-- テクスチャ設定 ------------------------------------------*/

		if ( flag & DG_PRIM2_TEX ){
			cur_tex = prim->tex_trans.ptex ;
		} else {
			cur_tex = NULL ;
		}
		/*------------------------------------------------------------*/

		if ( !( flag & ( DG_PRIM2_CW|DG_PRIM2_CCW ) ) ){
			cur_cull = D3DCULL_NONE ;
		} else {
			if ( flag & DG_PRIM2_CW ){
				/* 時計周り */
				cur_cull = D3DCULL_CCW ;	/* MGS2とDirectXは逆なので */
			} else {
				/* 反時計周り */
				cur_cull = D3DCULL_CW ;	/* MGS2とDirectXは逆なので */
			}
		}
		/*------------------------------------------------------------*/

		/*-- 半透明モードの指定 --------------------------------------*/

		if ( flag & ( DG_PRIM2_ALPHA | DG_PRIM2_ANTIALIASING ) ){
			cur_alpha = prim->tex_trans.alpha.data ;
											/* DG_SetTexture()内で設定されているので */
		} else {
			cur_alpha = 0 ;
		}
		/*------------------------------------------------------------*/

		/*-- フォグの有無 --------------------------------------------*/

		if ( flag & DG_PRIM2_FOG ){
			cur_fog = TRUE ;
		} else {
			cur_fog = FALSE ;
		}
		/*------------------------------------------------------------*/

		/*-- フレームバッファテクスチャを使用 ------------------------*/

		if ( !( flag & DG_PRIM2_FRAMETEX) ){ cur_tex_param = &tex_param ; }
		else{ cur_tex_param = &frame_tex_param ; }
		/*------------------------------------------------------------*/

		/*-- 変換基準 ------------------------------------------------*/

		cur_on_camera = (flag & DG_PRIM2_ON_CAMERA) ;
		/*------------------------------------------------------------*/

		/*-- 描画環境構築 --------------------------------------------*/

		if( last_tex  != cur_tex  )
		{
			last_tex = cur_tex ;
			DG_SetTextureDirect(0, cur_tex) ;
		}
		if( last_cull != cur_cull )
		{
			last_cull = cur_cull ;
			DG_SetRenderState(D3DRS_CULLMODE, last_cull) ;
		}
		if( last_fog != cur_fog )
		{
			last_fog = cur_fog ;
			DG_SetRenderState(D3DRS_FOGENABLE, last_fog) ;
		}
		if( last_alpha != cur_alpha )
		{
			last_alpha = cur_alpha ;
			DG_SetAlphaMode(last_alpha) ;
		}
		if( last_tex_param != cur_tex_param )
		{
			last_tex_param = cur_tex_param ;
			DG_SetVertexShaderConstant(CV_TEX0_SCALE, last_tex_param, 2) ;
		}
		if( !cur_on_camera || (last_on_camera != cur_on_camera) )
		{
			last_on_camera = cur_on_camera ;
			DG_SetVertexShaderConstant(CV_SCREEN, &prim->screen, 4);
		}
		/*------------------------------------------------------------*/

		/*-- 関数実行 ------------------------------------------------*/

		func = _write_prim_packs_func_tbl[prim->type] ;
		ASSERT( func ) ;

		func(packet) ;
		/*------------------------------------------------------------*/

		/*-- 次のPacket ----------------------------------------------*/

		packet = packet->next_addr ;
		/*------------------------------------------------------------*/
	}

	return ( 0 );
}

int DG_WritePrimPacksListNVS( DG_PRIM2_PACKET *packet, DG_PRIM2_PACKET *term, DG_CHANL *addr, int which )
{
	DG_PRIM2	*prim ;
	DWORD		flag ;
	void		*cur_tex ;
	void		*last_tex ;
	DWORD		cur_cull ;
	DWORD		last_cull ;
	u_long		last_alpha ;
	u_long		cur_alpha ;
	DWORD		cur_fog ;
	DWORD		last_fog ;
	void		*cur_tex_param ;
	void		*last_tex_param ;
	BOOL		cur_on_camera ;
	BOOL		last_on_camera ;
	void		(*func)(DG_PRIM2_PACKET *) ;
	void		*cur_backbuffer_tex ;

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;

	DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;
	DG_SetPixelShader(NULL) ;
	DG_SetTransform(D3DTS_WORLD, &DG_UnitMatrix) ;
	DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;

	DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
	DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;

	DG_SetTransform(D3DTS_TEXTURE0, &DG_UnitMatrix) ;	// UV直接
	DG_SetTextureDirect( 1, NULL );

	cur_backbuffer_tex = DG_BackBufferTexture[DG_CurrentBackBuffer] ;

	/* 最初のPacket処理 */
	while( packet != term )
	{
		/* 頂点無し判定 */
		if( !packet->n_verts )
		{
			packet = packet->next_addr ;
			continue ;
		}

		prim = packet->prim ;
		flag = (DWORD)prim->flag ;

		if( prim->tex_trans.ptex == cur_backbuffer_tex )
		{
			/* 41.09のドライバが何故か変な動作するので、現在の描画バッファは使用禁止 */
#ifdef DEBUG_MODE
			printf("[ Prim Warning ]Can't Use Current BackBuffer Texture Direct!!\n") ;
#endif
			packet = packet->next_addr ;
			continue ;
		}
		/*-- テクスチャ設定 ------------------------------------------*/

		if ( flag & DG_PRIM2_TEX ){
			last_tex = prim->tex_trans.ptex ;
		} else {
			last_tex = NULL ;
		}
		/*------------------------------------------------------------*/

		if ( !( flag & ( DG_PRIM2_CW|DG_PRIM2_CCW ) ) ){
			last_cull = D3DCULL_NONE ;
		} else {
			if ( flag & DG_PRIM2_CW ){
				/* 時計周り */
				last_cull = D3DCULL_CCW ;	/* MGS2とDirectXは逆なので */
			} else {
				/* 反時計周り */
				last_cull = D3DCULL_CW ;	/* MGS2とDirectXは逆なので */
			}
		}
		/*------------------------------------------------------------*/

		/*-- 半透明モードの指定 --------------------------------------*/

		if ( flag & ( DG_PRIM2_ALPHA | DG_PRIM2_ANTIALIASING ) ){
			last_alpha = prim->tex_trans.alpha.data ;
											/* DG_SetTexture()内で設定されているので */
		} else {
			last_alpha = 0 ;
		}
		/*------------------------------------------------------------*/

		/*-- フォグの有無 --------------------------------------------*/

		if ( flag & DG_PRIM2_FOG ){
			last_fog = TRUE ;
		} else {
			last_fog = FALSE ;
		}
		/*------------------------------------------------------------*/

		/*-- フレームバッファテクスチャを使用 ------------------------*/

		if ( !( flag & DG_PRIM2_FRAMETEX) ){ last_tex_param = &tex_param_mtx ; }
		else{ last_tex_param = &frame_tex_param_mtx ; }
		/*------------------------------------------------------------*/

		/*-- 変換基準 ------------------------------------------------*/

		last_on_camera = (flag & DG_PRIM2_ON_CAMERA) ;
		/*------------------------------------------------------------*/

		/*-- 描画環境構築 --------------------------------------------*/

		DG_SetTextureDirect(0, last_tex) ;
		DG_SetRenderState(D3DRS_CULLMODE, last_cull) ;
		DG_SetRenderState(D3DRS_FOGENABLE, last_fog) ;
		DG_SetAlphaMode(last_alpha) ;
		DG_SetTransformTexCoord(D3DTS_TEXTURE0, last_tex_param) ;
		DG_SetTransform(D3DTS_VIEW,     &prim->screen) ;
		/*------------------------------------------------------------*/

		/*-- 関数実行 ------------------------------------------------*/

		func = _write_prim_packs_func_nvs_tbl[prim->type] ;
		ASSERT( func ) ;

		func(packet) ;
		/*------------------------------------------------------------*/

		packet = packet->next_addr ;
		break ;
	}

	/* 2番目以降のPacket */
	while( packet != term )
	{
		/* 頂点無し判定 */
		if( !packet->n_verts )
		{
			packet = packet->next_addr ;
			continue ;
		}

		prim = packet->prim ;
		flag = (DWORD)prim->flag ;

		if( prim->tex_trans.ptex == cur_backbuffer_tex )
		{
			/* 41.09のドライバが何故か変な動作するので、現在の描画バッファは使用禁止 */
#ifdef DEBUG_MODE
			printf("[ Prim Warning ]Can't Use Current BackBuffer Texture Direct!!\n") ;
#endif
			packet = packet->next_addr ;
			continue ;
		}

		/*-- テクスチャ設定 ------------------------------------------*/

		if ( flag & DG_PRIM2_TEX ){
			cur_tex = prim->tex_trans.ptex ;
		} else {
			cur_tex = NULL ;
		}
		/*------------------------------------------------------------*/

		if ( !( flag & ( DG_PRIM2_CW|DG_PRIM2_CCW ) ) ){
			cur_cull = D3DCULL_NONE ;
		} else {
			if ( flag & DG_PRIM2_CW ){
				/* 時計周り */
				cur_cull = D3DCULL_CCW ;	/* MGS2とDirectXは逆なので */
			} else {
				/* 反時計周り */
				cur_cull = D3DCULL_CW ;	/* MGS2とDirectXは逆なので */
			}
		}
		/*------------------------------------------------------------*/

		/*-- 半透明モードの指定 --------------------------------------*/

		if ( flag & ( DG_PRIM2_ALPHA | DG_PRIM2_ANTIALIASING ) ){
			cur_alpha = prim->tex_trans.alpha.data ;
											/* DG_SetTexture()内で設定されているので */
		} else {
			cur_alpha = 0 ;
		}
		/*------------------------------------------------------------*/

		/*-- フォグの有無 --------------------------------------------*/

		if ( flag & DG_PRIM2_FOG ){
			cur_fog = TRUE ;
		} else {
			cur_fog = FALSE ;
		}
		/*------------------------------------------------------------*/

		/*-- フレームバッファテクスチャを使用 ------------------------*/

		if ( !( flag & DG_PRIM2_FRAMETEX) ){ cur_tex_param = &tex_param_mtx ; }
		else{ cur_tex_param = &frame_tex_param_mtx ; }
		/*------------------------------------------------------------*/

		/*-- 変換基準 ------------------------------------------------*/

		cur_on_camera = (flag & DG_PRIM2_ON_CAMERA) ;
		/*------------------------------------------------------------*/

		/*-- 描画環境構築 --------------------------------------------*/

		if( last_tex != cur_tex  )
		{
			last_tex = cur_tex ;
			DG_SetTextureDirect(0, cur_tex) ;
		}
		if( last_cull != cur_cull )
		{
			last_cull = cur_cull ;
			DG_SetRenderState(D3DRS_CULLMODE, last_cull) ;
		}
		if( last_fog != cur_fog )
		{
			last_fog = cur_fog ;
			DG_SetRenderState(D3DRS_FOGENABLE, last_fog) ;
		}
		if( last_alpha != cur_alpha )
		{
			last_alpha = cur_alpha ;
			DG_SetAlphaMode(last_alpha) ;
		}
		if( last_tex_param != cur_tex_param )
		{
			last_tex_param = cur_tex_param ;
			DG_SetTransformTexCoord(D3DTS_TEXTURE0, last_tex_param) ;
		}
		if( !cur_on_camera || (last_on_camera != cur_on_camera) )
		{
			last_on_camera = cur_on_camera ;
			DG_SetTransform(D3DTS_VIEW, &prim->screen) ;
		}

		/*------------------------------------------------------------*/

		/*-- 関数実行 ------------------------------------------------*/

		func = _write_prim_packs_func_nvs_tbl[prim->type] ;
		ASSERT( func ) ;

		func(packet) ;
		/*------------------------------------------------------------*/

		/*-- 次のPacket ----------------------------------------------*/

		packet = packet->next_addr ;
		/*------------------------------------------------------------*/
	}

	/*-- 後始末 ------------------------------------------------------*/

	DG_SetTransform(D3DTS_VIEW, &addr->eye_inv) ;
	/*----------------------------------------------------------------*/

	return ( 0 );
}


/* プリミティブ用頂点バッファ生成関数テーブル */
static	int	(*_make_prim_vtxbuff_func_tbl[])(DG_PRIM2 *) =
{
	NULL,
	DG_MakePrimPacksVBuffer_Line,	// DG_PRIM2_LINE(0x00000001)
	DG_MakePrimPacksVBuffer_Poly,	// DG_PRIM2_POLY(0x00000002)
	DG_MakePrimPacksVBuffer_Poly,	// DG_PRIM2_CULLPOLY(0x00000003)
	DG_MakePrimPacksVBuffer_Sprt,	// DG_PRIM2_SPRT(0x00000004)
	DG_MakePrimPacksVBuffer_RSprt,	// DG_PRIM2_RSPRT(0x00000005)
} ;
#define	MAKE_PRIM_VTXBUFF_FUNC_TBL_SIZE	(sizeof(_make_prim_vtxbuff_func_tbl)/sizeof(_make_prim_vtxbuff_func_tbl[0]))

static	int	(*_make_prim_vtxbuff_func_nvs_tbl[])(DG_PRIM2 *) =
{
	NULL,
	DG_MakePrimPacksVBuffer_Line_NVS,	// DG_PRIM2_LINE(0x00000001)
	DG_MakePrimPacksVBuffer_Poly_NVS,	// DG_PRIM2_POLY(0x00000002)
	DG_MakePrimPacksVBuffer_Poly_NVS,	// DG_PRIM2_CULLPOLY(0x00000003)
	DG_MakePrimPacksVBuffer_Sprt_NVS,	// DG_PRIM2_SPRT(0x00000004)
	DG_MakePrimPacksVBuffer_RSprt_NVS,	// DG_PRIM2_RSPRT(0x00000005)
} ;

/* プリミティブ用の頂点バッファを生成する */
#if FALSE
static int DG_MakePrimPacksVBuffer( DG_CHANL *cp, DG_PRIM2 *prim )
{
	int	(*func)(DG_PRIM2 *prim) ;

	ASSERT( (prim->type > 0) && (prim->type < MAKE_PRIM_VTXBUFF_FUNC_TBL_SIZE) )

	if( DG_CheckUseVertexShader() ){ func = _make_prim_vtxbuff_func_tbl[prim->type] ; }
	else{ func = _make_prim_vtxbuff_func_nvs_tbl[prim->type] ; }
	ASSERT( func ) ;

	return( func(prim) ) ;
}
#endif


/*----------------------------------------------------------------*/
static FMATRIX	_prim2_mat ;
static inline void _SetMatrix( FMATRIX *m )
{
	_prim2_mat = *m ;
}

static void CalcSortZ( DG_PRIM2_PACKET *packet )
{
	FVECTOR				total_verts, *src ;
#ifdef _USE_SSE	// SSE使用
	FVECTOR				v ;
#endif
	register float		d ;
	int					i ;

#ifdef _USE_SSE	// SSE使用
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
#ifndef _WINDOWS
	int		and_flag, flag ;
	int		or_flag ;	// 要るのかな？
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

#else	// 以降WINDOWS版
	int		and_flag, flag ;
	FVECTOR		*verts ;
	FVECTOR		tmp_v ;
	float		w ;
	int			i ;
	FVECTOR		verts_buff[8] ;

	verts_buff[0].vx = bound[0].vx ;
	verts_buff[0].vy = bound[0].vy ;
	verts_buff[0].vz = bound[0].vz ;
	verts_buff[0].vw = 1.0f ;
	verts_buff[1].vx = bound[1].vx ;
	verts_buff[1].vy = bound[1].vy ;
	verts_buff[1].vz = bound[1].vz ;
	verts_buff[1].vw = 1.0f ;
	verts_buff[2].vx = verts_buff[1].vx ;
	verts_buff[2].vy = verts_buff[0].vy ;
	verts_buff[2].vz = verts_buff[0].vz ;
	verts_buff[2].vw = 1.0f ;
	verts_buff[3].vx = verts_buff[0].vx ;
	verts_buff[3].vy = verts_buff[1].vy ;
	verts_buff[3].vz = verts_buff[0].vz ;
	verts_buff[3].vw = 1.0f ;
	verts_buff[4].vx = verts_buff[1].vx ;
	verts_buff[4].vy = verts_buff[1].vy ;
	verts_buff[4].vz = verts_buff[0].vz ;
	verts_buff[4].vw = 1.0f ;
	verts_buff[5].vx = verts_buff[0].vx ;
	verts_buff[5].vy = verts_buff[0].vy ;
	verts_buff[5].vz = verts_buff[1].vz ;
	verts_buff[5].vw = 1.0f ;
	verts_buff[6].vx = verts_buff[1].vx ;
	verts_buff[6].vy = verts_buff[0].vy ;
	verts_buff[6].vz = verts_buff[1].vz ;
	verts_buff[6].vw = 1.0f ;
	verts_buff[7].vx = verts_buff[0].vx ;
	verts_buff[7].vy = verts_buff[1].vy ;
	verts_buff[7].vz = verts_buff[1].vz ;
	verts_buff[7].vw = 1.0f ;

	and_flag = CLIP_FLAG ;
	verts = verts_buff ;
	for ( i = 8 ; i > 0 ; i--, verts++ ){
		_sceVu0ApplyMatrix( &tmp_v, mat, verts );
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

		if( !and_flag ){ break ; }
	}

	if ( and_flag ) return ( 2 );	/* 画面外 */
#endif	// _WINDOWS

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
	ScrpadWork		*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	int				make_vbuff_flag = 0, chanl_check_flag = 0 ;
#ifdef DEBUG_MODE
	int				pos_init_error_flag ;
#endif
	BOOL			use_vertexshader ;
	void	*make_func_tbl[MAKE_PRIM_VTXBUFF_FUNC_TBL_SIZE] ;
	int		(*make_func)(DG_PRIM2 *) ;

	MARK( __FILE__ );
	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->prim2_buffer ;
	/*
		キュー数のチェック
	*/
	if ( ( i = obj_buff->n_queue ) == 0 ) return ;

	DG_PerfStartD(DGPERF_CTGR_PRIM2) ;

	/*
		スクラッチパッド上のＯＴ初期化
	*/
	GV_ZeroMemory( scrpad->ot3, sizeof(int)*64*3 );

	/*
		パース変換パラメータ／クリッピング領域を設定
	*/
	pque = (DG_PRIM2**)obj_buff->queue ;
	c_gid = DG_CurrentGroupID ;

	*PERS_MAT = cp->raise_pers ;
	*EYE_PERS = cp->raise_eye_pers ;
	// *EYE_PERS2 = cp->raise_eye_pers2 ;
	SCALE->vx = (float)(cp->width / 2) ;
	SCALE->vy = (float)(cp->height / 2) ;
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

	use_vertexshader = DG_CheckUseVertexShader() ;

	/*
		関数テーブル選択
	*/
	{
		void			*src_tbl ;
#if FALSE
		DG_CHANL_EX		*cexp ;
#endif
		if( use_vertexshader )
		{
			/* Vertex Shader Version */
			src_tbl = _make_prim_vtxbuff_func_tbl ;
		}
		else
		{
			/* Vertex Shader 非対応版 */
#if FALSE
			cexp = DG_ChanlEx(cp) ;	// 拡張パラメータ取得
			_cam_unit_vec_x = cexp->cam_unit_vec_x ;
			_cam_unit_vec_y = cexp->cam_unit_vec_y ;
#endif
			src_tbl = _make_prim_vtxbuff_func_nvs_tbl ;

		}

		memcpy(make_func_tbl, src_tbl, sizeof(make_func_tbl)) ;
	}


	for ( ; i > 0 ; i--, pque++ ){
		prim2 = *pque ;

		/* 頂点バッファの構築 */
		if ( make_vbuff_flag ){
			if ( ( prim2->flag & chanl_check_flag ) != chanl_check_flag ){
				prim2->vbuff = NULL ;	// 実際に必要になった時点で作成
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

		/* 頂点バッファの構築 */
		if( !prim2->vbuff )
		{
			make_func = make_func_tbl[prim2->type] ;
			if( make_func(prim2) < 0 ){ continue ; }
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
			if ( 0xABCDEF01ABCDEF01ui64 == *(u_long*)packet->pos_addr ){
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
#ifdef DEBUG_MODE
		if ( pos_init_error_flag ){
			if ( !( prim2->flag & DG_PRIM2_DEBUGCHECKED ) ){
				printf("%s:pos init error (%s)\n", __FILE__, prim2->fname );
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

	/* パフォーマンス検査 */
	DG_PerfEndD(DGPERF_CTGR_PRIM2) ;
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
