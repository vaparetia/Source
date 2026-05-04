//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xradar.c
	ＸＢＯＸ用レーダー表示ルーチン

	2002/05/19	K.Takabe
	$Id: xradar.c,v 1.14 2002/11/23 12:09:24 Yoshizawa1 Exp $

*/

#include "BP_EndianSupport.h"

#include "gameheader.h"

#include "dmapack.h"
#include "libdg.cnf"

#include	"radsprite.h"

#include	"radsize.h"

#include        "radar_config.h"
#include        "radar_macro.h"
#include        "bomb_sensor.h"
#include        "bomb.h"
#include        "outrange.h"
#include        "rdr_movie.h"

#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"
#include "BP_RenderRadar.h"

/* ---------------------------------------------------------------- */
/* radar.cよりコピーしてきているのでradar.cの宣言で変更があった場合には注意！ */
typedef struct {
	unsigned int qwc;
	unsigned int addr;
	unsigned int vif1;
	unsigned int vif2;
} DMA_TAG;

typedef struct {
	u_long64 data;
	u_long64 addr;
} GIF_AD_DATA;

typedef struct {
	sceGifTag	gif_tag;
	GIF_AD_DATA alpha;
	GIF_AD_DATA	offset;
	GIF_AD_DATA zbuf;
	GIF_AD_DATA	scissor;
	GIF_AD_DATA test;
} RADAR_WINDOW_SET;

typedef struct {
	sceGifTag	gif_tag;
	sceGsPrim	prim;
	u_long64			rgba;
	u_long64			xy1;
	u_long64			xy2;
} RADAR_TILE_SET;

typedef struct {
	sceGifTag gif_tag;
	GIF_AD_DATA test;
} RADAR_DRAWPARAM_SET;

typedef struct {
	// ソート用ヘッダ
	DG_DMATAG	dma_tag;
  
	// 描画環境セット
	RADAR_WINDOW_SET window;
	// タイル描画
	RADAR_TILE_SET	base_tile;
	// 描画パラメータセット
	RADAR_DRAWPARAM_SET	drparam;
	sceGifTag end_tag;
	GIF_AD_DATA	 alpha;
} PACK_HEADER;

typedef ALIGN16_DECL(struct) {
	int color[ 4 ];
} LCOLOR ;

typedef ALIGN16_DECL(struct) {
	/* size max = 0xb00 = 2816 */
	float matrix[ 4 ][ 4 ];
	/* 壁描画用 */
	FVECTOR yclip;
	int bright_z, dark_z, pad1, pad2;
	LCOLOR bright_color;
	LCOLOR dark_color;
	LCOLOR door_color;
	sceGifTag giftag;
	/* 位置描画用 */
//	int player_z, enemy_z, pad11, pad12;
//	sceGifTag pos_giftag;
	sceGifTag sight_giftag;
	sceGifTag no_sight_giftag;
	/* 視界計算用 */
	FVECTOR rot_table[ MAX_ROT_TABLE ];
	/*視界、キャラの表示色 */
	struct {
		LCOLOR sight_bright_top;
		LCOLOR sight_bright_end;
		LCOLOR sight_dark_top;
		LCOLOR sight_dark_end;
	} chara_color[ MAX_CHARA_COLOR ];
} VU1_PARAM ;

typedef struct {
	SPR_OBJ		* empty;	/* リンクのルート  */
	SPR_OBJ		* mode ;	/* alert  */
	SPR_OBJ		* use ;	/* 下部の NO USE表示  */
	SPR_OBJ		* clearing ;	/* クリアリング中 */
	SPR_OBJ		* noise ;	/* 妨害エフェクト */
	SPR_OBJ		* radio ;	/* 無線エフェクト */
	SPR_OBJ		* level ;	/* 危険値グラフ */
	SPR_OBJ		* dot ;		/* modeゲージの点滅部*/

   SPR_OBJ		* frameT ;	/* ?*/
   SPR_OBJ		* frameL ;	/* ?*/
   SPR_OBJ		* frameR ;	/* ?*/
   SPR_OBJ		* frameB ;	/* ?*/
//BP JG - IF YOU ADD TO THIS STRUCTURE YOU MUST UPDATE THE SAME STRUCTURE IN RADAR.C
   SPR_POS		sp_pos;
#ifdef XBOXBACK /*背景に緑のポリゴン。vu部分が移植されたら削除 2002.04.10ynao*/
	SPR_OBJ		* box ;	/* 背景 */
#endif
} RAD_SPR ;


typedef struct xradar_work {
	GV_ACT_EX actor;
	VU1_PARAM param;
	PACK_HEADER head;
	FVECTOR zoom_rate;
	RAD_SPR		rad_spr ;
	int name;
	int display_flag;
	int prev_flag;
	int prev_frame;
	DG_DMAPACK *dmapack;
	u_short		count ;
	u_char		alert ;
	u_char		noise ;
	float		height ;
	int		status ;
	int		nowmode ;
	u_short	nomcnt	;
	short	base_rot ;	/*表示方向*/
	int		tri_id ;
#ifdef DEBUG_MODE
	int		debug ;
#endif
} Work;
/* ---------------------------------------------------------------- */
typedef struct _radar_vertex {
	short					x, y, z, h ;
} RADAR_VERTEX ;
typedef struct _radar_prim_vertex {
	float				x, y, z ;
	u_int				col ;
} RADAR_PRIM_VERTEX ;

#if 0 //BP_XBOX
static DG_VERTEXFORMAT		RadarVertexFormat[3] ;
static DG_VERTEXSHADER		RadarVertexShader[3] ;
extern unsigned char		VERTEX_SHADER_vs_radar[] ;
extern unsigned char		VERTEX_SHADER_vs_radar2[] ;
#endif
static int					InitReference = 0 ;

//static int					TestVertexList[ 1 * 1024 * 1024 ];

#define CV_ZERO				0
#define CV_ONE				1
#define CV_HALF				2
#define CV_BLOCK_OFFSET		3
#define CV_TRANS_MAT		4

#define CV_RADAR_COLOR0		16
#define CV_RADAR_COLOR1		17

#ifdef KP_WINDOWS
#define	setColorElem(r_, g_, b_, a_)	{(b_), (g_), (r_), (a_)}
#else
#define	setColorElem(r_, g_, b_, a_)	{(r_), (g_), (b_), (a_)}
#endif

static unsigned char default_chara_colors[][ 4 ][ 4 ] = {
	// RADAR_COLOR_PLAYER
	{
      setColorElem( 0, 160, 72, 0x80 ),
      setColorElem( 0, 0, 0, 0x80 ),
      setColorElem( 0, 160, 72, 0x80 ),
      setColorElem( 0, 0, 0, 0x80 ),
	},
	// RADAR_COLOR_BLUE
	{
		// 同一階の時。
		setColorElem( 0x00, 0x80, 0x80, 0x80 ),
		setColorElem( 0x00, 0x00, 0x10, 0x80 ),
		// 違う階の時。
		setColorElem( 0x00, 0x20, 0x20, 0x80 ),
		setColorElem( 0x00, 0x00, 0x10, 0x80 ),
	},
	// RADAR_COLOR_RED
	{
		setColorElem( 0xa0, 0x00, 0x00, 0x80 ),
		setColorElem( 0x10, 0x00, 0x00, 0x80 ),
		setColorElem( 0xa0/4, 0x00, 0x00, 0x80 ),
		setColorElem( 0x10, 0x00, 0x00, 0x80 ),
	},
	// RADAR_COLOR_YELLOW
	{
		setColorElem( 0xa0, 0xa0, 0x00, 0x80 ),
		setColorElem( 0x08, 0x08, 0x00, 0x80 ),
		setColorElem( 0xa0/4, 0xa0/4, 0x00, 0x80 ),
		setColorElem( 0x08, 0x08, 0x00, 0x80 ),
	},
};
static float		radar_point_size ;
/* ---------------------------------------------------------------- */
/* 外部宣言 */
#if 0 //BP_XBOX
extern D3DVIEWPORT8 DG_CurrentViewport;
#endif
/* ---------------------------------------------------------------- */
/* プロトタイプ宣言 */
static void* DrawSegmentData( VU1_PARAM *param, void* pRenderRadarBuffer );
static void* DrawRadarPrimitive( Work *work, void* pRenderRadarBuffer );
static void* DrawBackPrimitive( void* pRenderRadarBuffer );
/* ---------------------------------------------------------------- */
/* レーダー描画関連初期化 */
void RADAR_InitRadar( void )
{
#if 0 //BP_XBOX
	static DWORD dwDecl_00[] = {
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),		/* position */
		D3DVSD_END()
	};
	static DWORD dwDecl_01[] = {
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT4),		/* position */
		D3DVSD_END()
	};
	static DWORD dwDecl_02[] = {
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT3),		/* position */
		D3DVSD_REG(3,  D3DVSDT_PBYTE4),	   /* diffuse */
		D3DVSD_END()
	};
#endif

	InitReference++ ;
	if ( InitReference > 1 ) return ;

#if 0 //BP_XBOX
	DG_MakeVertexFormat( &RadarVertexFormat[0], dwDecl_00 );
	DG_MakeVertexFormat( &RadarVertexFormat[1], dwDecl_01 );
	DG_MakeVertexFormat( &RadarVertexFormat[2], dwDecl_02 );
	DG_MakeVertexShader( &RadarVertexShader[0], VERTEX_SHADER_vs_radar, dwDecl_00 );
	DG_MakeVertexShader( &RadarVertexShader[2], VERTEX_SHADER_vs_radar2, dwDecl_02 );
#endif
}
/* レーダー描画関連終了処理 */
void RADAR_EndRadar( void )
{
	InitReference-- ;
	if ( InitReference != 0 ) 
      return;
}
/* ＡＣＴ中での実際の描画用セットアップ処理 */
void RADAR_ActDrawRadar( Work *work )
{
}

/* ---------------------------------------------------------------- */
extern void *DG_DrawVerticesDirect( u_int type, int v_size, int count );
extern void DG_DrawVerticesDirectEnd( void );

typedef struct  
{
   char *pRenderData;
   int mRenderDataSize;
}
SRenderRadarWork;

static void *RADAR_BufferRadarDataInternal( Work *work, void *pRenderRadarBuffer )
{
	FMATRIX		mat ;
	float		aspect = 1.0f ;
   SBP_RadarPacket_Init *pInitPacket;

   // Create init packet
   pInitPacket = pRenderRadarBuffer;
   pInitPacket->cmd = kBP_RadarInit;
   pRenderRadarBuffer = pInitPacket + 1;

	/*
		ラインによる壁の描画
	*/
	{/* ビューポートの設定 */
      pInitPacket->viewX = ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + RADAR_WINDOW_X0;
      pInitPacket->viewY = ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + RADAR_WINDOW_Y0;
      pInitPacket->viewW = RADAR_WINDOW_W;
      pInitPacket->viewH = RADAR_WINDOW_H;

      pInitPacket->viewW+=1;        //BP JG - offset to stop the radar lines from bleeding into the 'use' area.
      pInitPacket->viewH-=1;        //BP JG - offset to stop the radar lines from bleeding into the 'use' area.
	}

	{/* 座標変換パラメータ生成 */
		FVECTOR		*pos, tmp_vec ;
		float		x_scale, z_scale, y_scale, zoom_rate ;
		/* レーダー表示関連パラメータ生成 */
		aspect = (float)RADAR_WINDOW_W * PIXEL_ASPECT / RADAR_WINDOW_H ;
		aspect = ( (float)RADAR_WINDOW_W / RADAR_WINDOW_H ) / aspect ;	/* 正規化同時座標で処理するので複雑・・・ */
      aspect = 1.0f / aspect;
		zoom_rate = work->zoom_rate.vx / RADAR_ZOOM_RATE_W ;
		x_scale = 2.0f / RADAR_VIEW_W * zoom_rate ;		/* レーダー表示エリアにはRADAR_VIEW_Wの範囲が見える */
		z_scale = 2.0f / RADAR_VIEW_W * zoom_rate * aspect ;
		y_scale = 1.0f / ( ( UPPER_RANGE - DOWN_RANGE ) / 2 ) ;
		radar_point_size = 2.0f / ( x_scale * ( RADAR_WINDOW_X1 - RADAR_WINDOW_X0 ) ) ;	/* １ピクセル分の距離 */
		/* 変換マトリクスの作成 *//* この変換を行うと正規化同時座標になるマトリクスを生成 */
		GV_ZeroMemory( &mat, sizeof(FMATRIX) );
		mat.m[0][0] = x_scale ; mat.m[1][0] = 0.0f ; mat.m[2][0] = 0.0f ; mat.m[3][0] = 0.0f ;
		mat.m[0][1] = 0.0f ; mat.m[1][1] = 0.0f ; mat.m[2][1] = -z_scale ; mat.m[3][1] = 0.0f ;
		mat.m[0][2] = 0.0f ; mat.m[1][2] = y_scale ; mat.m[2][2] = 0.0f ; mat.m[3][2] = 0.0f ;
		mat.m[0][3] = 0.0f ; mat.m[1][3] = 0.0f ; mat.m[2][3] = 0.0f ; mat.m[3][3] = 1.0f ;
		{/* 回転の設定 */
			FMATRIX	rot_mat ;
			float	rot_y ;
			rot_mat = DG_UnitMatrix ;
			rot_y = work->base_rot * ( PI / 2048.0f );
			_sceVu0RotMatrixY( &rot_mat, &rot_mat, rot_y );
			_sceVu0MulMatrix( &mat, &mat, &rot_mat );
		}
		/* 中心位置設定 */
		pos = GM_RadarGetCenter();
		tmp_vec.vx = pos->vx ;
		tmp_vec.vy = pos->vy + ( UPPER_RANGE + DOWN_RANGE ) / 2 ;
		tmp_vec.vz = pos->vz ;
		tmp_vec.vw = 1.0f ;
      
		_sceVu0ApplyMatrix( &tmp_vec, &mat, &tmp_vec );
		mat.m[3][0] = -tmp_vec.vx ;
		mat.m[3][1] = -tmp_vec.vy ;
		mat.m[3][2] = -tmp_vec.vz ;

      pInitPacket->matrix = mat;
	}

   // Begin back primitive data
   {
      SBP_RadarPacket_BeginBackPrimitive * pRenderPacket;
      pRenderPacket = pRenderRadarBuffer;
      pRenderPacket->cmd = kBP_RadarBeginBackPrimitive;
      pRenderRadarBuffer = pRenderPacket + 1;
   }

	/* 背景描画 */
	pRenderRadarBuffer = DrawBackPrimitive(pRenderRadarBuffer);

	/* 視界、壁などの描画処理 */
	if( work->display_flag & RADAR_D_HZX )
   {
      // Begin segment data
      {
         SBP_RadarPacket_BeginSegments * pRenderPacket;
         pRenderPacket = pRenderRadarBuffer;
         pRenderPacket->cmd = kBP_RadarBeginSegments;
         pRenderRadarBuffer = pRenderPacket + 1;
      }
		pRenderRadarBuffer = DrawSegmentData( &work->param, pRenderRadarBuffer );

      // Begin Radar Primitive
      {
         SBP_RadarPacket_BeginPrimitive* pPacket = pRenderRadarBuffer;
         pPacket->cmd = kBP_RadarBeginPrimitives;
         pRenderRadarBuffer = pPacket + 1;
      }
		pRenderRadarBuffer = DrawRadarPrimitive( work, pRenderRadarBuffer );
	}

   // Add end packet
   {
      SBP_RadarPacket_End* pPacket = pRenderRadarBuffer;
      pPacket->cmd = kBP_RadarEnd;
      pRenderRadarBuffer = pPacket + 1;
   }

   return pRenderRadarBuffer;
}

void *RADAR_BufferRadarCallback( void *param, DG_DMAPACK *pDstDmapack )
{
   SRenderRadarWork *pRenderRadarWork;
   void *pEndData;

   pRenderRadarWork = DG_AS_SceneBufferAlloc(sizeof(SRenderRadarWork));
   pRenderRadarWork->pRenderData = (char *)(pRenderRadarWork + 1);

   pEndData = RADAR_BufferRadarDataInternal( (Work *) param, pRenderRadarWork->pRenderData );

   pRenderRadarWork->mRenderDataSize = (char *) pEndData - pRenderRadarWork->pRenderData;
   DG_AS_SceneBufferAlloc( pRenderRadarWork->mRenderDataSize );

   return pRenderRadarWork;
}

void RADAR_BufferRadarWithRenderBuffer( Work *work )
{
   void* pRenderRadarBufferBegin = BP_RB_GetCurrentPtr();
   
   void *pRenderRadarBufferEnd = RADAR_BufferRadarDataInternal( work, pRenderRadarBufferBegin );

   BP_RB_SetCurrentPtr(pRenderRadarBufferEnd);
   BP_RB_AddCommand(kCmd_RenderRadar, pRenderRadarBufferBegin);
}

void RADAR_RenderRadarCallback(void *pParam)
{
   if ( gAS_UsedBufferSceneNOP )
   {
      RADAR_BufferRadarWithRenderBuffer( (Work *) pParam );
   }
   else
   {
      SRenderRadarWork* pRenderRadarWork = (SRenderRadarWork *) pParam;
      char *renderData = BP_RB_Alloc(pRenderRadarWork->mRenderDataSize);
      memcpy(renderData, pRenderRadarWork->pRenderData, pRenderRadarWork->mRenderDataSize);
      BP_RB_AddCommand(kCmd_RenderRadar, renderData);
   }
}

/* ---------------------------------------------------------------- */
#if 0 //BP_XBOX
static SetVertexShaderConstantCVECTOR( int reg, IVECTOR *col )
{
	FVECTOR		vec ;
	vec.vx = (float)col->vx / 255.0f ;
	vec.vy = (float)col->vy / 255.0f ;
	vec.vz = (float)col->vz / 255.0f ;
	vec.vw = (float)col->vw / 255.0f ;
	DG_SetVertexShaderConstant( reg, &vec, 1 );
}
#endif

/* 通常壁データリストの作成 */
static void* MakeSegmentVertexList( void* pRenderRadarBuffer, HZX_BLOCK *block, HZX_VuSEG *seg, int n_segs )
{
	int				i, j, n_verts ;
	SVECTOR			*src ;
#if 0 //BP_XBOX
   RADAR_VERTEX	*dst ;
#else
   SBP_RadarVertexPos* dst;
#endif

	FVECTOR			tmp_vec ;

	tmp_vec.vx = (float)block->tx ;	/* ブロックの中心位置 */
	tmp_vec.vy = (float)block->ty ;	/* ブロックの中心位置 */
	tmp_vec.vz = (float)block->tz ;	/* ブロックの中心位置 */
	tmp_vec.vw = 0.0f ;

#if 0 //BP_XBOX
	DG_SetVertexShaderConstant( CV_BLOCK_OFFSET, &tmp_vec, 1);
#endif

	/* プッシュバッファ直接書き込みバージョン */
	for ( i = 0; i < n_segs; i ++, seg ++ )
   {
      if ( seg->atr & HZX_SEG_NO_DISP_RADAR ) 
         continue ;

		n_verts = seg->b1.pad ;

      src = seg->verts;

      /* 描画開始 */
#if 0 //BP_XBOX
		dst = DG_DrawVerticesDirect( D3DPT_LINESTRIP, sizeof(SVECTOR), n_verts );
		
      for ( j = 0 ; j < n_verts ; j++, src++, dst++ )
      {
			*(SVECTOR*)&dst[ 0 ] = src[ 0 ];
		}

		/* 描画終了 */
		DG_DrawVerticesDirectEnd();
#elif BP_VITA
      // Setup packet, converting linestrip to a line list, as the vita doesn't do strips
      {
         SBP_RadarPacket_GeomPos* pPacket = pRenderRadarBuffer;
         pPacket->cmd = kBP_RadarGeomPos;
         pPacket->shader = kBP_RadarShader1;
         pPacket->prim = kBP_RadarPrim_LineList;
         pPacket->vertexCount = (n_verts-1) * 2;
         dst = pPacket->verts;

         pRenderRadarBuffer = pPacket->verts + pPacket->vertexCount;
      }

      // Copy verts (add block offsets in as we go instead of doing it in the shader for simplicity)
      for ( j = 0 ; j < n_verts-1 ; j++, src++, dst += 2 )
      {
         dst[0].x = src[0].vx + tmp_vec.vx;
         dst[0].y = src[0].vy + tmp_vec.vy;
         dst[0].z = src[0].vz + tmp_vec.vz;
         dst[0].h = src[0].pad;
         dst[1].x = src[1].vx + tmp_vec.vx;
         dst[1].y = src[1].vy + tmp_vec.vy;
         dst[1].z = src[1].vz + tmp_vec.vz;
         dst[1].h = src[1].pad;
      }
#else
      // Setup packet
      {
         SBP_RadarPacket_GeomPos* pPacket = pRenderRadarBuffer;
         pPacket->cmd = kBP_RadarGeomPos;
         pPacket->shader = kBP_RadarShader1;
         pPacket->prim = kBP_RadarPrim_LineStrip;
         pPacket->vertexCount = n_verts;
         dst = pPacket->verts;

         pRenderRadarBuffer = pPacket->verts + pPacket->vertexCount;
      }

      // Copy verts (add block offsets in as we go instead of doing it in the shader for simplicity)
      for ( j = 0 ; j < n_verts ; j++, src++, dst++ )
      {
         dst->x = src->vx + tmp_vec.vx;
         dst->y = src->vy + tmp_vec.vy;
         dst->z = src->vz + tmp_vec.vz;
         dst->h = src->pad;
      }
#endif
	}
   return pRenderRadarBuffer;

}
/* 動的壁データリストの作成 */
static void* MakeDynamicSegmentVertexList( void* pRenderRadarBuffer, HZX_D_SEGMENT *segs, VU1_PARAM *param )
{
	HZX_D_SEGMENT		*seg ;
	int count1, count2;
#if 0 //BP_XBOX
	FVECTOR	*dst ;
#else
   SBP_RadarVertexPos* dst;
#endif

#if 0 //BP_XBOX
   FVECTOR  tmp_vec;

	/* シェーダープログラムは共通なので不要なパラメータは初期化しておく */
	tmp_vec.vx = 0 ;	/* ブロックの中心位置 */
	tmp_vec.vy = 0 ;	/* ブロックの中心位置 */
	tmp_vec.vz = 0 ;	/* ブロックの中心位置 */
	tmp_vec.vw = 0.0f ;
	DG_SetVertexShaderConstant( CV_BLOCK_OFFSET, &tmp_vec, 1);
#endif

	seg = segs ;
	count1 = 0;
	count2 = 0;

	/* 必要な長点数の算出 */
	for(seg = segs; seg != NULL; seg = seg->next)
   {
		if (seg->atr & (HZX_SEG_SKIP | HZX_SEG_NO_DISP_RADAR)) 
         continue;

		if( seg->atr & HZX_SEG_DOOR )
			count2++ ;
      else 
			count1++ ;
    }

	/* プッシュバッファ直接書き込みバージョン */
	if ( count1 != 0 )
   {
      /* 通常動的壁 */
		/* 描画開始 */
#if 0 //BP_XBOX
		SetVertexShaderConstantCVECTOR( CV_RADAR_COLOR0, &param->bright_color );
		SetVertexShaderConstantCVECTOR( CV_RADAR_COLOR1, &param->dark_color );

      dst = DG_DrawVerticesDirect( D3DPT_LINELIST, sizeof(FVECTOR), count1 * 2 ) ;
#else
      // Add color change packet
      {
         SBP_RadarPacket_Color* pPacket = pRenderRadarBuffer;
         pPacket->cmd = kBP_RadarColor;
         pPacket->color1.vx = param->bright_color.color[0] / 128.0f;
         pPacket->color1.vy = param->bright_color.color[1] / 128.0f;
         pPacket->color1.vz = param->bright_color.color[2] / 128.0f;
         pPacket->color1.vw = param->bright_color.color[3] / 128.0f;
         pPacket->color2.vx = param->dark_color.color[0] / 128.0f;
         pPacket->color2.vy = param->dark_color.color[1] / 128.0f;
         pPacket->color2.vz = param->dark_color.color[2] / 128.0f;
         pPacket->color2.vw = param->dark_color.color[3] / 128.0f;
         pRenderRadarBuffer = pPacket + 1;
      }

      // Setup packet
      {
         SBP_RadarPacket_GeomPos* pPacket = pRenderRadarBuffer;
         pPacket->cmd = kBP_RadarGeomPos;
         pPacket->shader = kBP_RadarShader1;
         pPacket->prim = kBP_RadarPrim_LineList;
         pPacket->vertexCount = count1 * 2;
         dst = pPacket->verts;

         pRenderRadarBuffer = pPacket->verts + pPacket->vertexCount;
      }

#endif

		for(seg = segs; seg != NULL; seg = seg->next)
      {
			if (seg->atr & (HZX_SEG_SKIP | HZX_SEG_NO_DISP_RADAR)) 
            continue;
			
         if( seg->atr & HZX_SEG_DOOR ) 
            continue ;

			dst[0].x = seg->p1.vx ;
			dst[0].y = seg->p1.vy ;
			dst[0].z = seg->p1.vz ;
			dst[0].h = seg->p1.vw ;
			dst[1].x = seg->p2.vx ;
			dst[1].y = seg->p2.vy ;
			dst[1].z = seg->p2.vz ;
			dst[1].h = seg->p2.vw ;
			dst += 2 ;
		}
#if 0 //BP_XBOX
		/* 描画終了 */
		DG_DrawVerticesDirectEnd();
#endif
   }
	
   if ( count2 != 0 )
   {/* ドア用動的壁 */

      /* 描画開始 */
#if 0 //BP_XBOX
		SetVertexShaderConstantCVECTOR( CV_RADAR_COLOR0, &param->door_color );
		SetVertexShaderConstantCVECTOR( CV_RADAR_COLOR1, &param->dark_color );

      dst = DG_DrawVerticesDirect( D3DPT_LINELIST, sizeof(FVECTOR), count2 * 2 ) ;
#else
      // Add color change packet
      {
         SBP_RadarPacket_Color* pPacket = pRenderRadarBuffer;
         pPacket->cmd = kBP_RadarColor;
         pPacket->color1.vx = param->door_color.color[0] / 128.0f;
         pPacket->color1.vy = param->door_color.color[1] / 128.0f;
         pPacket->color1.vz = param->door_color.color[2] / 128.0f;
         pPacket->color1.vw = param->door_color.color[3] / 128.0f;
         pPacket->color2.vx = param->dark_color.color[0] / 128.0f;
         pPacket->color2.vy = param->dark_color.color[1] / 128.0f;
         pPacket->color2.vz = param->dark_color.color[2] / 128.0f;
         pPacket->color2.vw = param->dark_color.color[3] / 128.0f;
         pRenderRadarBuffer = pPacket + 1;
      }

      // Setup packet
      {
         SBP_RadarPacket_GeomPos* pPacket = pRenderRadarBuffer;
         pPacket->cmd = kBP_RadarGeomPos;
         pPacket->shader = kBP_RadarShader1;
         pPacket->prim = kBP_RadarPrim_LineList;
         pPacket->vertexCount = count2 * 2;
         dst = pPacket->verts;

         pRenderRadarBuffer = pPacket->verts + pPacket->vertexCount;
      }
#endif

		for(seg = segs; seg != NULL; seg = seg->next){
			if (seg->atr & (HZX_SEG_SKIP | HZX_SEG_NO_DISP_RADAR)) continue;
			if( !( seg->atr & HZX_SEG_DOOR ) ) continue ;

			dst[0].x = seg->p1.vx ;
			dst[0].y = seg->p1.vy /* - 100000 */ ;
			dst[0].z = seg->p1.vz ;
			dst[0].h = seg->p1.vw /* + 200000 */ ;
			dst[1].x = seg->p2.vx ;
			dst[1].y = seg->p2.vy /* - 100000 */ ;
			dst[1].z = seg->p2.vz ;
			dst[1].h = seg->p2.vw /* + 200000 */ ;
			dst += 2 ;
		}
#if 0 //BP_XBOX
		/* 描画終了 */
		DG_DrawVerticesDirectEnd();
#endif
    }

   return pRenderRadarBuffer;
}
/* 頂点データリストの作成 */
static void* DrawSegmentData( VU1_PARAM *param, void* pRenderRadarBuffer )
{
	HZX_HDL *hzd;
	int i, n;
	int *inside;
	HZX_GRP *grp;
	HZX_BLOCK *block;
	int	bit, bit2, group ;
	FVECTOR		*pos ;

	pos = GM_RadarGetCenter();

	hzd = HZX_GetCurrentHzx() ;
	bit = HZX_AddGroupID( HZX_CurrentGroupID ) ; /* 園山追加(2001.01.26) */

	/* 可視になっているグループを全て表示 */
	while( bit != 0 ){
		group = GV_GetNo( bit ) ;
		bit2 = GV_GetBit( group ) ;
		if ( group >= 0 && group < hzd->def->n_groups ){
			/* グループに存在する全てのレーダー表示用ブロックを処理 */
			grp = hzd->def->groups + group ;
			inside = HZX_GetNearBlockID( grp, pos, RADAR_VIEW_W, &n ) ;
			/* 固有描画設定 */
#if 0 //BP_XBOX
			SetVertexShaderConstantCVECTOR( CV_RADAR_COLOR0, &param->bright_color );
			SetVertexShaderConstantCVECTOR( CV_RADAR_COLOR1, &param->dark_color );
			DG_SelectVertexShader( &RadarVertexShader[0], &RadarVertexFormat[0] );
#else
         // Add color change packet
         {
            SBP_RadarPacket_Color* pPacket = pRenderRadarBuffer;
            pPacket->cmd = kBP_RadarColor;
            pPacket->color1.vx = param->bright_color.color[0] / 128.0f;
            pPacket->color1.vy = param->bright_color.color[1] / 128.0f;
            pPacket->color1.vz = param->bright_color.color[2] / 128.0f;
            pPacket->color1.vw = param->bright_color.color[3] / 128.0f;
            pPacket->color2.vx = param->dark_color.color[0] / 128.0f;
            pPacket->color2.vy = param->dark_color.color[1] / 128.0f;
            pPacket->color2.vz = param->dark_color.color[2] / 128.0f;
            pPacket->color2.vw = param->dark_color.color[3] / 128.0f;
            pRenderRadarBuffer = pPacket + 1;
         }
#endif

			for( i = 0; i < n; i++ )
         {
				block = grp->blocks + *inside;
				inside ++ ;
				if ( block->n_segs > 0 )
            {
					pRenderRadarBuffer = MakeSegmentVertexList( pRenderRadarBuffer, block, block->segs, block->n_segs );
				}
			}
			/* ダイナミックハザード */
			/* 固有描画設定 */
#if 0 //BP_XBOX
			DG_SelectVertexShader( &RadarVertexShader[0], &RadarVertexFormat[1] );
#endif
			if ( grp->dynamics->n_segs > 0 ){
				pRenderRadarBuffer = MakeDynamicSegmentVertexList( pRenderRadarBuffer, grp->dynamics->segs, param );
			}
		}
		bit &= ~bit2 ;
	}
   return pRenderRadarBuffer;
}
/* ---------------------------------------------------------------- */
/* レーダー関連プリミティブの描画 */
extern void DG_SinCos( float *s, float *c, float rot );
static void* DrawRadarPrimitive( Work *work, void* pRenderRadarBuffer )
{
	int i, hight_offset ;
	RADAR_CTRL *p;
   FVECTOR		*radar_center = GM_RadarGetCenter() ;
   float toWideScreen = 4.0f/3.0f;

#if 0 //BP_XBOX
   RADAR_PRIM_VERTEX	*verts ;
   /* シェーダーの切り替え */
	DG_SelectVertexShader( &RadarVertexShader[2], &RadarVertexFormat[2] );
	DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 128 ) );
#else
   SBP_RadarVertexPosCol* verts;
   int shader = kBP_RadarShader2;
   u_long64 alpha = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 128 );
#endif

	/* プレイヤーと敵兵の表示 */
	if( work->display_flag & ( RADAR_D_PLAYER | RADAR_D_MINE | RADAR_D_ENEMY ) )
   {
		if ( ( GV_Time % 16 ) < 3 ) 
         return pRenderRadarBuffer;

		for( p = GM_RadarControlGetTop(); p != NULL; p = p->next )
      {
			const int w = ( 4096 / MAX_ROT_TABLE );
			int roty, angle, width, col;
			float length, dir_cos, dir_sin, diff_y ;

			if( !( p->flag & RADAR_VISIBLE ) )
				continue;

			if( p->col == RADAR_COLOR_PLAYER )
         {
				// Player
			} 
         else 
         {
				if( ( p->flag & RADAR_MINE ) )
            {
					if( ( work->display_flag & RADAR_D_MINE ) == 0 )
						continue;
				} 
            else 
            {
					if( ( work->display_flag & RADAR_D_ENEMY ) == 0 )
						continue;
				}
			}

			diff_y = p->pos->vy - radar_center->vy ;
			diff_y = ( diff_y * p->range_zoom_rate ) + p->range_center ;
			
         if( DG_FABS( diff_y ) * p->same_floor_rate < 1.0F )
         {
				hight_offset = 0 ;
			} 
         else 
         {
				hight_offset = 2 ;
			}

         // fill the box
         {
            SBP_RadarPacket_GeomPosCol *pPacket = pRenderRadarBuffer;
            float scale = 0.7f;
            int i;
            int col = *(int *)&default_chara_colors[p->col][hight_offset];
            pPacket->cmd = kBP_RadarGeomPosCol;
            pPacket->shader = shader;
            pPacket->prim = kBP_RadarPrim_TriStrip;
            pPacket->vertexCount = 4;
            verts = pPacket->verts;

            for (i = 0; i < 4; ++i)
            {
               float dx = (i&1) ? 1.0f : -1.0f;
               float dy = (i&2) ? 1.0f : -1.0f;
               verts->x = p->pos->vx + scale*dx*radar_point_size;
               verts->y = 1.0f; // = Z
               verts->z = p->pos->vz + scale*dy*toWideScreen*radar_point_size;
               verts->col = col;
               ++verts;
            }

            pRenderRadarBuffer = verts;
         }

         // draw the box around the radar blip
         // this is the best way to get them to align
         {
            SBP_RadarPacket_GeomPosCol *pPacket = pRenderRadarBuffer;
            float scale = 0.6f;
            int i;
            int col = *(int *)&p->face->box.col;
            pPacket->cmd = kBP_RadarGeomPosCol;
            pPacket->shader = shader;
            pPacket->prim = kBP_RadarPrim_TriStrip;
            pPacket->vertexCount = 10;
            verts = pPacket->verts;

            for (i = 0; i < 5; ++i)
            {
               float dx = ((i+1)&2) ? 1.0f : -1.0f;
               float dy = (i&2) ? 1.0f : -1.0f;
               verts->x = p->pos->vx + dx*radar_point_size;
               verts->y = 1.0f; // = Z
               verts->z = p->pos->vz + dy*toWideScreen*radar_point_size;
               verts->col = col;
               ++verts;
               verts->x = p->pos->vx + scale*dx*radar_point_size;
               verts->y = 1.0f; // = Z
               verts->z = p->pos->vz + scale*dy*toWideScreen*radar_point_size;
               verts->col = col;
               ++verts;
            }

            pRenderRadarBuffer = verts;
         }

#if 0 //BP_XBOX
			DG_DrawVerticesDirectEnd();
#endif

			if( !( p->flag & RADAR_SIGHT ) ) 
            continue ;

			/* パラメータの取り出し */
			angle = p->angle;

			if( p->flag & RADAR_NOFIX_SIGHT)
				length = p->range;
         else 
				length = RAD_RANGE_FIX;

#if 0 //BP_XBOX
         verts = DG_DrawVerticesDirect( D3DPT_TRIANGLEFAN, sizeof(RADAR_PRIM_VERTEX), 6 ) ;
#else
         {
            SBP_RadarPacket_GeomPosCol* pPacket = pRenderRadarBuffer;
            pPacket->cmd = kBP_RadarGeomPosCol;
            pPacket->shader = shader;
            pPacket->prim = kBP_RadarPrim_TriFan;
            pPacket->vertexCount = 6;
            verts = pPacket->verts;
            pRenderRadarBuffer = pPacket->verts + pPacket->vertexCount;
         }
#endif
         verts->x = p->pos->vx ;
			verts->y = -0.2f ;	/* ＝Ｚ値 */
			verts->z = p->pos->vz ;
			verts->col = *(int*)default_chara_colors[ p->col ][ 0 + hight_offset ] ;
			verts++ ;
			for ( i = 0 ; i < 5 ; i++ )
         {
				DG_SinCos( &dir_sin, &dir_cos, ( p->dir + p->angle * ( i - 2 ) / 4 ) * ( PI / 2048.0f ) );
				verts->x = p->pos->vx + length * dir_sin ;
				verts->y = 1.0f ;	/* ＝Ｚ値 */
				verts->z = p->pos->vz + length * dir_cos ;
				verts->col = *(int*)default_chara_colors[ p->col ][ 1 + hight_offset ] ;
				verts++ ;
			}

#if 0 //BP_XBOX
			DG_DrawVerticesDirectEnd();
#endif
		}
	}

   return pRenderRadarBuffer;
}

static void* DrawBackPrimitive( void* pRenderRadarBuffer )
{
	FVECTOR		*radar_center = GM_RadarGetCenter() ;
	u_int		col = 0x401c1c00;

#if 0 //BP_XBOX
   RADAR_PRIM_VERTEX	*verts ;

	/* シェーダーの切り替え */
	DG_SelectVertexShader( &RadarVertexShader[2], &RadarVertexFormat[2] );
	DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );

   /* 位置の表示 */
   verts = DG_DrawVerticesDirect( D3DPT_TRIANGLESTRIP, sizeof(RADAR_PRIM_VERTEX), 4 ) ;

#else
   SBP_RadarVertexPosCol* verts;
   SBP_RadarPacket_GeomPosCol* pPacket = pRenderRadarBuffer;
   pPacket->cmd = kBP_RadarGeomPosCol;
   pPacket->shader = kBP_RadarShader2;
   pPacket->prim = kBP_RadarPrim_TriStrip;
   pPacket->vertexCount = 4;
   verts = pPacket->verts;
#endif

   BP_LE_SwapUInt_Inp( &col );

	verts->x = radar_center->vx - RADAR_VIEW_W * 2 ;
	verts->y = 0.0f ;	/* ＝Ｚ値 */
	verts->z = radar_center->vz - RADAR_VIEW_W * 2 ;
	verts->col = col ;
	verts++ ;
	verts->x = radar_center->vx + RADAR_VIEW_W * 2 ;
	verts->y = 0.0f ;	/* ＝Ｚ値 */
	verts->z = radar_center->vz - RADAR_VIEW_W * 2 ;
	verts->col = col ;
	verts++ ;
	verts->x = radar_center->vx - RADAR_VIEW_W * 2 ;
	verts->y = 0.0f ;	/* ＝Ｚ値 */
	verts->z = radar_center->vz + RADAR_VIEW_W * 2 ;
	verts->col = col ;
	verts++ ;
	verts->x = radar_center->vx + RADAR_VIEW_W * 2 ;
	verts->y = 0.0f ;	/* ＝Ｚ値 */
	verts->z = radar_center->vz + RADAR_VIEW_W * 2 ;
	verts->col = col ;
	verts++ ;

#if 0 //BP_XBOX
	DG_DrawVerticesDirectEnd();
#else
   pRenderRadarBuffer = pPacket->verts + pPacket->vertexCount;
#endif

   return pRenderRadarBuffer;
}
