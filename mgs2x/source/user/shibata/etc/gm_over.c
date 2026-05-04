//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    gm_over.c
    ゲームオーバー
	2000/08/28 T.Shibata

	$Id: gm_over.c,v 1.1.1.3 2002/11/19 11:48:43 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"../util/ts_util.h"

#undef PSX2 // BP_RENDER - We want to use XBox version of MAX_U/V etc...
#include	"../util/dma_set.h"
#define PSX2 // BP_RENDER - We want to use XBox version of MAX_U/V etc...

extern void ShowExitGameWarning(int *pResult);

#define	ACTOR_PRIO			(254)

#define	N_LINE				(120)
#define	N_LINE_ALLVERTS		(240)
#define N_LINE_PRIMS		(4)
#define N_LINE_VERTS		(60)

#define N_POLY				(12)
#define N_POLY_ALLVERTS		(N_POLY*4)
#define N_POLY_PRIMS		(1)
#define N_POLY_VERTS		(48)

#define	TRI_CODE			(11101464)
//#define	TRI_CODE			(GV_StrCode("gover"))
#define TEX_CODE			(9697079)
//#define TEX_CODE			(GV_StrCode("over_menu_alp_ovl"))

#define FADE_PRIM	(SCE_GS_SET_PRIM(6,0,0,0,1,0,0,0,0))
#define FADE_TEST	(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define FADE_ALPHA	(SCE_GS_SET_ALPHA(0,1,0,1,0x00))
#define FADE_RGBAQ	(SCE_GS_SET_RGBAQ(0x00,0x00,0x00,0x00,0))

#define LINE_PRIM	(SCE_GS_SET_PRIM(1,1,0,0,1,1,0,0,0))
#define LINE_TEST	(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define LINE_ALPHA	(SCE_GS_SET_ALPHA(0,1,0,1,0x00))

#define POLY_PRIM	(SCE_GS_SET_PRIM(4,1,0,0,1,0,0,0,0))
#define POLY_TEST	(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define POLY_ALPHA	(SCE_GS_SET_ALPHA(0,2,0,1,0x00))

#define POLY_ALPHA2	(SCE_GS_SET_ALPHA(0,1,0,1,0x00))
#define POLY_PRIM2	(SCE_GS_SET_PRIM(4,1,1,0,1,0,1,0,0))


#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define CONTINUE_X0		70+14
#define CONTINUE_Y0		125+1
#define EXIT_X0			199
#define EXIT_Y0			125+1

//#define NO_CONTINUE_EXIT_X0			( 160 - 64 / 2 )
//#define NO_CONTINUE_EXIT_X0			( (((EXIT_X0 - CONTINUE_X0)-32)/2) + CONTINUE_X0 )

extern	int	GM_GameOverVox ;

static short gameover_logo[] = {
	120,
	37, 88, 25, 104,
	25, 104, 31, 121,
	31, 121, 40, 121,
	40, 121, 32, 135,
	32, 135, 42, 135,
	69, 88, 37, 88,
	64, 96, 41, 96,
	41, 96, 35, 104,
	35, 104, 38, 113,
	38, 113, 45, 113,
	62, 100, 45, 100,
	45, 100, 40, 108,
	40, 108, 48, 108,
	48, 108, 45, 113,
	69, 88, 64, 96,
	55, 121, 65, 121,
	74, 88, 84, 88,
	67, 100, 85, 100,
	69, 96, 85, 96,
	67, 100, 55, 121,
	69, 96, 74, 88,
	84, 88, 85, 96,
	85, 100, 87, 121,
	87, 121, 78, 121,
	73, 108, 77, 108,
	78, 121, 77, 108,
	73, 108, 65, 121,
	62, 100, 42, 135,
	89, 121, 98, 121,
	97, 88, 108, 88,
	118, 88, 130, 88,
	122, 121, 113, 121,
	113, 121, 117, 104,
	117, 104, 108, 121,
	108, 121, 103, 121,
	103, 121, 102, 104,
	102, 104, 98, 121,
	95, 96, 108, 96,
	114, 96, 128, 96,
	94, 100, 108, 100,
	112, 100, 127, 100,
	97, 88, 95, 96,
	94, 100, 89, 121,
	108, 88, 108, 96,
	108, 100, 108, 108,
	112, 100, 108, 108,
	114, 96, 118, 88,
	127, 100, 122, 121,
	128, 96, 130, 88,
	134, 88, 156, 88,
	126, 121, 148, 121,
	132, 96, 154, 96,
	131, 100, 153, 100,
	138, 107, 151, 107,
	138, 107, 137, 113,
	150, 113, 137, 113,
	134, 88, 132, 96,
	131, 100, 126, 121,
	150, 113, 148, 121,
	151, 107, 153, 100,
	154, 96, 156, 88,
	173, 121, 199, 121,
	181, 88, 207, 88,
	179, 96, 205, 96,
	187, 100, 184, 113,
	184, 113, 192, 113,
	195, 100, 192, 113,
	207, 88, 205, 96,
	204, 100, 199, 121,
	181, 88, 179, 96,
	178, 100, 173, 121,
	178, 100, 187, 100,
	195, 100, 204, 100,
	213, 121, 224, 121,
	241, 88, 231, 88,
	210, 88, 219, 88,
	211, 96, 220, 96,
	211, 100, 220, 100,
	225, 100, 235, 100,
	227, 96, 237, 96,
	225, 100, 221, 108,
	231, 88, 227, 96,
	235, 100, 224, 121,
	237, 96, 241, 88,
	211, 100, 213, 121,
	210, 88, 211, 96,
	219, 88, 220, 96,
	220, 100, 221, 108,
	244, 88, 266, 88,
	236, 121, 258, 121,
	242, 96, 264, 96,
	241, 100, 263, 100,
	247, 113, 260, 113,
	248, 108, 261, 108,
	248, 108, 247, 113,
	244, 88, 242, 96,
	241, 100, 236, 121,
	263, 100, 261, 108,
	260, 113, 258, 121,
	266, 88, 264, 96,
	270, 88, 292, 88,
	268, 96, 286, 96,
	292, 88, 295, 97,
	267, 100, 282, 100,
	275, 107, 277, 107,
	262, 121, 272, 121,
	275, 107, 272, 121,
	277, 107, 285, 135,

	285, 135, 295, 135,
	286, 106, 295, 135,
	286, 106, 288, 106,
	295, 97, 288, 106,
	282, 100, 286, 96,

	267, 100, 262, 121,
	268, 96, 270, 88,
	67, 100, 55, 121,
	53, 125, 47, 135,
	53, 125, 279, 125,
	282, 135, 47, 135,
	282, 135, 279, 125,
};

typedef struct {
	DG_DMATAG		dmatag_fade;		// ＧＩＦ接続ＤＭＡタグ			フェード
	FRAME_CLEAR		fade_pack;			//								フェード
	
	u_long128		tex_packet[2];		//								テクスチャー転送
	
	DG_DMATAG		dmatag_ldraw;		// ＧＩＦ接続ＤＭＡタグ			ライン
	MDL_DRAW		line_draw;			//								ライン
	DG_DMATAG		dmatag_lverts;		// ＧＩＦ接続ＤＭＡタグ			ライン頂点
	
	DG_DMATAG		dmatag_pdraw;		// ＧＩＦ接続ＤＭＡタグ			ポリゴン
	TEX_DRAW		poly_draw;			//								ポリゴン
//	MDL_DRAW		poly_draw;			//								ポリゴン
	DG_DMATAG		dmatag_pverts;		// ＧＩＦ接続ＤＭＡタグ			ポリゴン頂点

	//終了パケット達
//	DG_DMATAG		dmatag_test;		// ＧＩＦ接続ＤＭＡタグ
//	PACKET_END		end_paket;			//								ピクセルテスト復元
//	DG_DMATAG		dmatag_offset;		// オフセット環境復元用
	DG_DMATAG		dmatag_end;			// RETタグ
}GMOVER_PACK;

typedef struct {
	DG_GIFTAG		giftag;
	LINE_VERT_DATA	verts[N_LINE];
}LINE_VERTS_PACK;

typedef struct {
	DG_GIFTAG		giftag;
	POLY_VERT_DATA	verts[N_POLY];
}POLY_VERTS_PACK;

typedef	struct {
	GV_ACT_EX	actor;
	int			mode;
	int			count;
	int			which;
	int			line_count;

	float		con_u[2];
	float		con_v[2];
	float		exi_u[2];
	float		exi_v[2];

	float		norms[ 120 * 2 ];

	LINE_VERTS_PACK		line_verts[2];
	POLY_VERTS_PACK		poly_verts[2];
	int					n_lines;
	int					n_polys;
	int					timer;
	DG_DMAPACK			*dmapack;
	GMOVER_PACK			*gmover_pack[2];
	int					flag;

	int					stream_handler ;
	int					stream_flag;

	int					dbg_timer;

#if 1 //BP_RENDER #ifndef PSX2
	void				*packet_mem;
	void                *prim ;
	int 				confirm;	/* 確認画面とのやりとりする関数 */
#endif
} Work;

static void InitDmaTags( Work *work, GMOVER_PACK *packet, int which )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		描画設定	フェード
	packet->dmatag_fade.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(FRAME_CLEAR) ) ;
	packet->dmatag_fade.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_fade.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(FRAME_CLEAR), 0) ;
	// ＤＭＡタグ		描画設定	ライン
	packet->dmatag_ldraw.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(MDL_DRAW)) ;
	packet->dmatag_ldraw.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_ldraw.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(MDL_DRAW), 0) ;
	// ＤＭＡタグ		頂点	ライン
	packet->dmatag_lverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_GIFTAG) ) ;
	packet->dmatag_lverts.addr = &work->line_verts[ which ];
	packet->dmatag_lverts.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_lverts.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_GIFTAG), 0) ;

	// ＤＭＡタグ		描画設定	ポリ
	packet->dmatag_pdraw.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TEX_DRAW)) ;
	packet->dmatag_pdraw.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_pdraw.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TEX_DRAW), 0) ;
	// ＤＭＡタグ		頂点	ポリ
	packet->dmatag_pverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_GIFTAG) ) ;
	packet->dmatag_pverts.addr = &work->poly_verts[ which ];
	packet->dmatag_pverts.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_pverts.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_GIFTAG) , 0) ;

	// ＤＭＡタグ		ピクセルテスト復元
//	packet->dmatag_test.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PACKET_END) ) ;
//	packet->dmatag_test.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
//	packet->dmatag_test.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PACKET_END), 0) ;
	// オフセット環境復元用
//	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
//	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] ) ;
//	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
//	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0) ;
	// RETタグ
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
}

static void SetLineDmaTag( Work *work, GMOVER_PACK *packet, int which )
{
	LINE_VERTS_PACK		*line_verts = &work->line_verts[ which ];
	
	//printf("n_lines = %d\n",work->n_lines);

	packet->dmatag_lverts.qwc =
		DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_GIFTAG) + SIZEOF_QWORD(LINE_VERT_DATA) * work->n_lines ) ;
	packet->dmatag_lverts.vifcode[1] =
		SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_GIFTAG) + SIZEOF_QWORD(LINE_VERT_DATA) * work->n_lines, 0) ;
	line_verts->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(LINE_VERT_DATA) * work->n_lines , 1, 0, 0, 0, 1);
	line_verts->giftag.regs = GS_REGS_AD;
}


static void SetPolyDmaTag( Work *work, GMOVER_PACK *packet, int which )
{
	POLY_VERTS_PACK		*poly_verts = &work->poly_verts[ which ];
	
	//printf("n_polys = %d\n",work->n_polys);
	
	packet->dmatag_pverts.qwc =
		DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_GIFTAG) + SIZEOF_QWORD(POLY_VERT_DATA) * work->n_polys ) ;
	packet->dmatag_pverts.vifcode[1] =
		SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_GIFTAG) + SIZEOF_QWORD(POLY_VERT_DATA) * work->n_polys , 0) ;
	poly_verts->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(POLY_VERT_DATA) * work->n_polys , 1, 0, 0, 0, 1);
	poly_verts->giftag.regs = GS_REGS_AD;
}

//rgba	0xaabbggrr
#if 0 //BP_RENDER #ifdef PSX2
static void SetLine( Work *work, int rgba0, int rgba1, float x0, float y0, float x1, float y1 )
{
	LINE_VERT_DATA	*verts = &work->line_verts[DG_Clock].verts[work->n_lines];
	int				ix0,iy0,ix1,iy1;
	if(work->n_lines >= 120 ){ printf("line packet over!!\n"); return; }
	
	work->n_lines++;
#if 1
	ix0 = MIN_X + DG_FTOI(((float)(DRAW_WIDTH) * (x0 / 320.0f))*SHIBATA_FSHIFT);
	iy0 = MIN_Y + DG_FTOI(((float)(DRAW_HEIGHT) * (y0 / 240.0f))*SHIBATA_FSHIFT);
	ix1 = MIN_X + DG_FTOI(((float)(DRAW_WIDTH) * (x1 / 320.0f))*SHIBATA_FSHIFT);
	iy1 = MIN_Y + DG_FTOI(((float)(DRAW_HEIGHT) * (y1 / 240.0f))*SHIBATA_FSHIFT);

#else

#endif
	verts->rgbq0.data = rgba0 | 0x3F80000000000000;
	verts->rgbq1.data = rgba1 | 0x3F80000000000000;
	verts->xyz0.data = SCE_GS_SET_XYZ( ix0, iy0,0);
	verts->xyz1.data = SCE_GS_SET_XYZ( ix1, iy1,0);

}
#else
static void SetLine( Work *work, int rgba0, int rgba1, float x0, float y0, float x1, float y1 )
{
	if(work->n_lines >= 120 ){ printf("line packet over!!\n"); return ; }

	work->prim = DG_SetDmapackAlpha( work->prim, LINE_ALPHA ) ;
	work->prim = DG_SetDmapackLine( work->prim,
								    MIN_X + (((float)(DRAW_WIDTH) * (x0 / 320.0f))*SHIBATA_FSHIFT),
								    MIN_Y + (((float)(DRAW_HEIGHT) * (y0 / 240.0f))*SHIBATA_FSHIFT),
								    DG_MakeDmaPackColorFromInt(rgba0),
								    MIN_X + (((float)(DRAW_WIDTH) * (x1 / 320.0f))*SHIBATA_FSHIFT),
								    MIN_Y + (((float)(DRAW_HEIGHT) * (y1 / 240.0f))*SHIBATA_FSHIFT),
								    DG_MakeDmaPackColorFromInt(rgba1) ) ;
}
#endif /*PSX2*/

typedef struct {
	float	x0,y0;
	float	x1,y1;
	float	x2,y2;
	float	x3,y3;
	float	u0,v0;
	float	u1,v1;
	float	u2,v2;
	float	u3,v3;
	int		rgba0;
	int		rgba1;
	int		rgba2;
	int		rgba3;
}POLY_INFO;

//0xaabbggrr
#if 0 //BP_RENDER #ifdef PSX2
static void SetPoly( Work *work, POLY_INFO *poly )
{
	POLY_VERT_DATA	*verts = &work->poly_verts[DG_Clock].verts[work->n_polys];
	int				ix0,iy0,ix1,iy1,ix2,iy2,ix3,iy3;
	int				is0,it0,is1,it1,is2,it2,is3,it3;

	if( work->n_polys >= N_POLY ){ printf("poly packet over!!\n"); return; }
	work->n_polys++;
#if 0
	is0 = *(int*)&poly->u0;
	it0 = *(int*)&poly->v0;
	is1 = *(int*)&poly->u1;
	it1 = *(int*)&poly->v1;
	is2 = *(int*)&poly->u2;
	it2 = *(int*)&poly->v2;
	is3 = *(int*)&poly->u3;
	it3 = *(int*)&poly->v3;
	verts->uv0.data = SCE_GS_SET_ST(is0,it0);
	verts->uv1.data = SCE_GS_SET_ST(is1,it1);
	verts->uv2.data = SCE_GS_SET_ST(is2,it2);
	verts->uv3.data = SCE_GS_SET_ST(is3,it3);
#endif
	is0 = (int)poly->u0;
	it0 = (int)poly->v0;
	is1 = (int)poly->u1;
	it1 = (int)poly->v1;
	is2 = (int)poly->u2;
	it2 = (int)poly->v2;
	is3 = (int)poly->u3;
	it3 = (int)poly->v3;

	ix0 = MIN_X + DG_FTOI((float)(DRAW_WIDTH) * (poly->x0 / 320.0f)*SHIBATA_FSHIFT);
	iy0 = MIN_Y + DG_FTOI((float)(DRAW_HEIGHT) * (poly->y0 / 240.0f)*SHIBATA_FSHIFT);
	ix1 = MIN_X + DG_FTOI((float)(DRAW_WIDTH) * (poly->x1 / 320.0f)*SHIBATA_FSHIFT);
	iy1 = MIN_Y + DG_FTOI((float)(DRAW_HEIGHT) * (poly->y1 / 240.0f)*SHIBATA_FSHIFT);
	ix2 = MIN_X + DG_FTOI((float)(DRAW_WIDTH) * (poly->x2 / 320.0f)*SHIBATA_FSHIFT);
	iy2 = MIN_Y + DG_FTOI((float)(DRAW_HEIGHT) * (poly->y2 / 240.0f)*SHIBATA_FSHIFT);
	ix3 = MIN_X + DG_FTOI((float)(DRAW_WIDTH) * (poly->x3 / 320.0f)*SHIBATA_FSHIFT);
	iy3 = MIN_Y + DG_FTOI((float)(DRAW_HEIGHT) * (poly->y3 / 240.0f)*SHIBATA_FSHIFT);

	verts->rgbq0.data = poly->rgba0 | I64(0x3F80000000000000);
	verts->rgbq1.data = poly->rgba1 | I64(0x3F80000000000000);
	verts->rgbq2.data = poly->rgba2 | I64(0x3F80000000000000);
	verts->rgbq3.data = poly->rgba3 | I64(0x3F80000000000000);
	verts->xyz0.data = SCE_GS_SET_XYZ( ix0, iy0,0);
	verts->xyz1.data = SCE_GS_SET_XYZ( ix1, iy1,0);
	verts->xyz2.data = SCE_GS_SET_XYZ( ix2, iy2,0);
	verts->xyz3.data = SCE_GS_SET_XYZ( ix3, iy3,0);
	verts->uv0.data = SCE_GS_SET_UV(is0,it0);
	verts->uv1.data = SCE_GS_SET_UV(is1,it1);
	verts->uv2.data = SCE_GS_SET_UV(is2,it2);
	verts->uv3.data = SCE_GS_SET_UV(is3,it3);
#if 0
	verts->uv0.reg = SCE_GS_UV ;
	verts->uv1.reg = SCE_GS_UV ;
	verts->uv2.reg = SCE_GS_UV ;
	verts->uv3.reg = SCE_GS_UV ;
	verts->uv0.data = SCE_GS_SET_UV(0,0);
	verts->uv1.data = SCE_GS_SET_UV(511*16,0);
	verts->uv2.data = SCE_GS_SET_UV(0,223*16);
	verts->uv3.data = SCE_GS_SET_UV(511*16,223*16);
#endif
//	printf("poly set\n");
}
#else /*PSX2*/

void SetTexUV( float *uo, float *vo, DG_TEX *tex, float ui, float vi )
{
	int	tw, th ;

	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	*uo = (ui + tex->u_offset)/(float)(1 << (tw+4)) ;
	*vo = (vi + tex->v_offset)/(float)(1 << (th+4)) ;
}

static void SetPoly( Work *work, POLY_INFO *poly )
{
	DG_TEX *tex ;
	float u, v ;

	if( work->n_polys >= N_POLY ){ printf("poly packet over!!\n"); return ; }
	work->n_polys++;

	tex = DG_GetTexture2( TRI_CODE, TEX_CODE ) ;

	work->prim = DG_SetDmapackTriangleStrip( work->prim, 4 );
	SetTexUV( &u, &v, tex, poly->u0+8.0f, poly->v0+8.0f ) ;
	work->prim = DG_SetDmapackVertex( work->prim,
							   MIN_X + ((float)(DRAW_WIDTH) * (poly->x0 / 320.0f)*SHIBATA_FSHIFT),
							   MIN_Y + ((float)(DRAW_HEIGHT) * (poly->y0 / 240.0f)*SHIBATA_FSHIFT),
							   u,
							   v,
							   DG_MakeDmaPackColorFromInt(poly->rgba0)   ) ;
	SetTexUV( &u, &v, tex, poly->u1-8.0f, poly->v1+8.0f ) ;
	work->prim = DG_SetDmapackVertex( work->prim,
							   MIN_X + ((float)(DRAW_WIDTH) * (poly->x1 / 320.0f)*SHIBATA_FSHIFT),
							   MIN_Y + ((float)(DRAW_HEIGHT) * (poly->y1 / 240.0f)*SHIBATA_FSHIFT),
							   u,
							   v,
							   DG_MakeDmaPackColorFromInt(poly->rgba1)   ) ;
	SetTexUV( &u, &v, tex, poly->u2+8.0f, poly->v2-8.0f ) ;
	work->prim = DG_SetDmapackVertex( work->prim,
							   MIN_X + ((float)(DRAW_WIDTH) * (poly->x2 / 320.0f)*SHIBATA_FSHIFT),
							   MIN_Y + ((float)(DRAW_HEIGHT) * (poly->y2 / 240.0f)*SHIBATA_FSHIFT),
							   u,
							   v,
							   DG_MakeDmaPackColorFromInt(poly->rgba2)   ) ;
	SetTexUV( &u, &v, tex, poly->u3-8.0f, poly->v3-8.0f ) ;
	work->prim = DG_SetDmapackVertex( work->prim,
							   MIN_X + ((float)(DRAW_WIDTH) * (poly->x3 / 320.0f)*SHIBATA_FSHIFT),
							   MIN_Y + ((float)(DRAW_HEIGHT) * (poly->y3 / 240.0f)*SHIBATA_FSHIFT),
							   u,
							   v,
							   DG_MakeDmaPackColorFromInt(poly->rgba3)   ) ;
}
#endif /*PSX2*/


#if 1 //BP_RENDER #ifndef PSX2
static void TexDataSetXBOX( Work *work, DG_TEX *tex )
{
	work->prim = DG_SetDmapackTex( work->prim, tex ) ;
}
#endif

static	int	WeaponShade( int power, int count )
{
	int		r, gb ;

	if ( count < 8 ) {
//		r = gb = 255 ;
		gb = 255 ;
		r = 128 ;
	} else {
		count -= 8 ;
		if ( count > 8 ) count = 8 ;
//		r = 255 - 255 * count / 8 ;
		r = 128 - 128 * count / 8 ;
		gb = 255 - ( 255 - power ) * count / 8 ;
	}
	return ( r | ( gb << 8 ) | ( gb << 16 ) | (0x80 << 24) ) ;
}


static void GameOverPrims_Act( Work *work )
{
	short			*data;
	float			*norms ;
	int				i, update, count;
	POLY_INFO		poly_info;

	memset( &poly_info,0,sizeof(POLY_INFO));
	//初期化
	work->n_lines = 0;
	work->n_polys = 0;
	
	data = &gameover_logo[1];
	norms = work->norms;
	count = work->line_count;
	
	i = 120;

	if ( count > 0 && !(work->timer % 3)) work->line_count = ( count >= i + 16 ) ? 0 : count + 2 ;

	update = 1 ;
	
	if ( count == 0 ) {
		update = 0 ;
		count = 16 ;
	}


#if 1 //BP_RENDER #ifndef PSX2
#if 0 //BP_RENDER // force untextured mode to match ps2 prims
	TexDataSetXBOX( work, DG_GetTexture2( TRI_CODE, TEX_CODE ) ) ; /* Defined in local file */
#else
   TexDataSetXBOX( work, NULL ); 
#endif
#endif
	
	for ( i = 0; i < 120 ; i++ ) {
		float		x1, x2, y1, y2, vx, vy ;
		int			c1, c2 ;

		vx = vy = 0 ;
		if ( update ) {
			if ( count < 8 ) {
				vx = norms[ 0 ] * powf( -2.0f, (float)count );
				vy = norms[ 1 ] * powf( -2.0f, (float)count );
				//vx = norms[ 0 ] >> count ;
				//vy = norms[ 1 ] >> count ;
//				if(i==0)printf("count = %d\n",count);
				if( i % 4 == 2 ){
					vx = -vx;
					vy = -vy;
				}
			}
			if ( -- count < 0 ) break ;
		}

		x1 = (float)data[ 0 ] + vx ;
		y1 = (float)data[ 1 ] + vy ;
		x2 = (float)data[ 2 ] + vx * 2.0f ;
		y2 = (float)data[ 3 ] + vy * 2.0f ;

      x1 *= 0.75f;
      x2 *= 0.75f;
   
      x1 += 40;
      x2 += 40;


		c1 = (int)x1 - 160;
		c2 = (int)x2 - 160;
		if( c1 < 0 ) c1 = -c1;
		if( c2 < 0 ) c2 = -c2;
		c1 = 255 - c1;
		c2 = 255 - c2;
		if ( c1 > 255 ) c1 = 255 ;
		if ( c2 > 255 ) c2 = 255 ;

		c1 = WeaponShade( c1, count );
		c2 = WeaponShade( c2, count );

		SetLine( work, c1, c2, x1, y1, x2, y2 );
		if ( count >= 8 && count < 14 ) {
		//ポリゴンのピカー
			int		count2 ;

#if 1 //BP_RENDER #ifndef PSX2
		    work->prim = DG_SetDmapackAlpha( work->prim, POLY_ALPHA ) ;
#endif

			count2 = count - 6 ;
			
			vx = -norms[ 1 ] * powf( -2.0f, (float)count2 );
			vy = norms[ 0 ] * powf( -2.0f, (float)count2 );
			
			poly_info.x0 = x2 - vx;
			poly_info.y0 = y2 - vy;
			poly_info.x1 = x1 - vx;
			poly_info.y1 = y1 - vy;
			poly_info.x2 = ( x1 + x2 ) / 2.0f;
			poly_info.y2 = ( y1 + y2 ) / 2.0f;
			poly_info.x3 = x1 + vx;
			poly_info.y3 = y1 + vy;
			
			poly_info.rgba0 = 0x80000000;
			poly_info.rgba1 = 0x80000000;
			poly_info.rgba2 = 0x80804000;
			poly_info.rgba3 = 0x80000000;
			SetPoly( work, &poly_info );
			
			poly_info.x0 = x2 - vx;
			poly_info.y0 = y2 - vy;
			poly_info.x1 = x2 + vx;
			poly_info.y1 = y2 + vy;
			poly_info.x2 = ( x1 + x2 ) / 2.0f;
			poly_info.y2 = ( y1 + y2 ) / 2.0f;
			poly_info.x3 = x1 + vx;
			poly_info.y3 = y1 + vy;
			SetPoly( work, &poly_info );
		}
		data += 4 ;
		norms += 2 ;
	}

	if( work->line_count == 0 ){
		/* セレクトフェーズ */
		/* この時には、Rediant は用済みなので、それを使う */
		long64 			col1, col2, br;
		long64 			time;
		POLY_INFO	   	con,exi;
		DG_TEX 			*tex;
		DG_TEXTURE_LIST	*texlist;

		tex = DG_GetTexture2( TRI_CODE, TEX_CODE );   
#if 0 //BP_RENDER #ifdef PSX2
		texlist = DG_GetTextureList( TRI_CODE );

		InitSetTexDraw( &work->gmover_pack[DG_Clock]->poly_draw, 0, POLY_PRIM2 );
		TexDataSet( work->gmover_pack[DG_Clock]->tex_packet, &texlist->tex_packet[DG_Clock],
					&work->gmover_pack[DG_Clock]->poly_draw, &tex->tex_trans,
					POLY_ALPHA2 );
#else
		TexDataSetXBOX( work, tex ) ; /* Defined in local file */
		work->prim = DG_SetDmapackAlpha( work->prim, POLY_ALPHA2 ) ;
#endif
		if( work->flag ){
			time = 31;
		}else{
			time = (work->timer%192)/3;
		}
		if( time > 32 ){
			br = 48 + ( 64 - time ) * 4;
		} else {
			br = 48 + time * 4;
		}

		con.x0 = CONTINUE_X0;
		con.y0 = CONTINUE_Y0;
		con.x1 = CONTINUE_X0 + (64.0f*TARGET_ASPECT_X);
		con.y1 = CONTINUE_Y0;
		con.x2 = CONTINUE_X0;
		con.y2 = CONTINUE_Y0 + 7.0f;
		con.x3 = CONTINUE_X0 + (64.0f*TARGET_ASPECT_X);
		con.y3 = CONTINUE_Y0 + 7.0f;
		
		con.u0 = work->con_u[0];
		con.v0 = work->con_v[0];
		con.u1 = work->con_u[1];
		con.v1 = work->con_v[0];
		con.u2 = work->con_u[0];
		con.v2 = work->con_v[1];
		con.u3 = work->con_u[1];
		con.v3 = work->con_v[1];
		
		exi.x0 = EXIT_X0;
		exi.y0 = EXIT_Y0;
		exi.x1 = EXIT_X0 + (64.0f*TARGET_ASPECT_X);
		exi.y1 = EXIT_Y0;
		exi.x2 = EXIT_X0;
		exi.y2 = EXIT_Y0 + 7.0f;
		exi.x3 = EXIT_X0 + (64.0f*TARGET_ASPECT_X);
		exi.y3 = EXIT_Y0 + 7.0f;
		
		exi.u0 = work->exi_u[0];
		exi.v0 = work->exi_v[0];
		exi.u1 = work->exi_u[1];
		exi.v1 = work->exi_v[0];
		exi.u2 = work->exi_u[0];
		exi.v2 = work->exi_v[1];
		exi.u3 = work->exi_u[1];
		exi.v3 = work->exi_v[1];

		//if( work->continue_flag ){
		if( work->which == 0 ){
			col1 = ( 0 | ( br << 8 ) | ( br << 16 ) | (0x80<<24) ) ;
			col2 = ( 0 | ( 48 << 8 ) | ( 48 << 16 ) | (0x80<<24) ) ;
		} else {
			col1 = ( 0 | ( 48 << 8 ) | ( 48 << 16 ) | (0x80<<24) ) ;
			col2 = ( 0 | ( br << 8 ) | ( br << 16 ) | (0x80<<24) ) ;
		}
		con.rgba0 = col1;
		con.rgba1 = col1;
		con.rgba2 = col1;
		con.rgba3 = col1;
		exi.rgba0 = col2;
		exi.rgba1 = col2;
		exi.rgba2 = col2;
		exi.rgba3 = col2;

		SetPoly( work, &con );
		SetPoly( work, &exi );

		//} else {
		//work->current = 1;
		//col2 = ( 0 | ( br << 8 ) | ( br << 16 ) ) ;
		//next = set_sprt_tex( next, NO_CONTINUE_EXIT_X0, EXIT_Y0, TEX_EXIT, col2, ot );
		//}

		if(work->flag){
			time = 255 - work->count;
			//printf("time = %d\n",time);
			con.x0 = 0.0f;
			con.y0 = 0.0f;
			con.x1 = 160.0f;
			con.y1 = 0.0f;
			con.x2 = 0.0f;
			con.y2 = 240.0f;
			con.x3 = 160.0f;
			con.y3 = 240.0f;

			con.u0 = 3.0f*16.0f; 
			con.v0 = 7.0f*16.0f; 
			con.u1 = 4.0f*16.0f;
			con.v1 = 7.0f*16.0f;
			con.u2 = 3.0f*16.0f;
			con.v2 = 8.0f*16.0f;
			con.u3 = 4.0f*16.0f;
			con.v3 = 8.0f*16.0f;

			exi.x0 = 160.0f;
			exi.y0 = 0.0f;
			exi.x1 = 320.0f;
			exi.y1 = 0.0f;
			exi.x2 = 160.0f;
			exi.y2 = 240.0f;
			exi.x3 = 320.0f;
			exi.y3 = 240.0f;

			exi.u0 = 3.0f*16.0f;
			exi.v0 = 7.0f*16.0f;
			exi.u1 = 4.0f*16.0f;
			exi.v1 = 7.0f*16.0f;
			exi.u2 = 3.0f*16.0f;
			exi.v2 = 8.0f*16.0f;
			exi.u3 = 4.0f*16.0f;
			exi.v3 = 8.0f*16.0f;

			con.rgba0 = ((time * 2 > 255)? 255:time*2)<<24;
			con.rgba1 = (time)<<24;
			con.rgba2 = ((time * 2 > 255)? 255:time*2)<<24;
			con.rgba3 = (time)<<24;
			exi.rgba0 = (time)<<24;
			exi.rgba1 = ((time * 2 > 255)? 255:time*2)<<24;
			exi.rgba2 = (time)<<24;
			exi.rgba3 = ((time * 2 > 255)? 255:time*2)<<24;

			SetPoly( work, &con );
			SetPoly( work, &exi );
		}

	}



#if 0 //BP_RENDER #ifdef PSX2			
	//セットタグ
	SetLineDmaTag( work, work->gmover_pack[DG_Clock], DG_Clock );
	SetPolyDmaTag( work, work->gmover_pack[DG_Clock], DG_Clock );
#endif
}

static void Fade_Act( Work *work, long64 fade )
{

	if( fade > 128 ){
		fade = 128;
		DG_Chanl(0)->flag = 0 ;
		DG_Chanl(1)->flag = 0 ;
	}
	//if( fade > 64 ) fade = 64;

#if 0 //BP_RENDER #ifdef PSX2			
	InitFrameClear( &work->gmover_pack[DG_Clock]->fade_pack,FADE_TEST,
					SCE_GS_SET_RGBAQ(0x00,0x00,0x00,fade,0),FADE_ALPHA,0 );
#else
	work->prim = InitFrameClear( work->prim, FADE_TEST, SCE_GS_SET_RGBAQ(0x00,0x00,0x00,fade,0),FADE_ALPHA,0 );
#endif
}

extern int DG_DBG_DMA ;

static void Act( Work *work )
{
	GV_PAD		*pad = GV_PadDataDirect;

#if 1 //BP_RENDER #ifndef PSX2
	work->prim = work->dmapack->autopacket ;
#endif
	//DG_DBG_DMA = work->prim ;

	Fade_Act( work, work->timer );
	GameOverPrims_Act( work );

#if 1 //BP_RENDER #ifndef PSX2			
	DG_SetDmapackEnd( work->prim );
#endif

	if ( work->timer > 16 && !work->stream_flag) {
		/* 音声再生 */
		/* いいタイミングでスタートしてください */
		/* 多重呼びしないように注意してください */
		if ( GM_GameOverVox >= 0 && GM_StreamStatus( work->stream_handler ) == GM_STREAM_STATE_WAIT ) {

			printf("GameOverStream Start\n");
			GM_StreamStart( work->stream_handler );
			work->stream_flag = 1;
			printf("stream wait[%d]", work->dbg_timer );
			//printf("GM_GameOverVox = %x\n",GM_GameOverVox);
		}else{
			work->dbg_timer++;
		}
	}

//	if ( work->line_count > 0 && !(work->timer % 3))
//		work->line_count = ( work->line_count >= 120 + 16 ) ? 0 : work->line_count + 2 ;
	work->timer++;

#ifdef KP_XBOX
	/* 確認画面中はパッドなどの情報をみる部分前にreturnさせる */
	if( work->confirm != 0 ){
		if( work->confirm == -1 ){
			/* 起動中 */
			return;
		} else if( work->confirm == 3 ){
			/* NO選択、確認画面から制御戻る */
			work->confirm = 0;
			work->flag = 0;
			return;
		}
		return;
	}
#endif
	
	if ( work->count > 0 ) {
		if ( -- work->count > 0 ){
			return ;
		}
		if(work->flag){
			if ( GM_StreamIsPlay() == 0 ) {
				GM_GameOverRestart( work->which ) ;
				GV_DestroyActor( work ) ;
			} else {
				work->count = 1 ;
			}
		}else{
			GM_GameOverClear() ;
			printf("Game Over Select Start\n");
		}
	}

#if 0
	if ( work->which == 0 ) {
		DEBUG_Locate( 256, 128, MENU_MODE_CENTER ) ;
		DEBUG_SetColor( 255, 128, 128 ) ;
		DEBUG_Printf( "CONTINUE" ) ;
		DEBUG_Locate( 256, 144, MENU_MODE_CENTER ) ;
		DEBUG_SetColor( 255, 255, 255 ) ;
		DEBUG_Printf( "EXIT" ) ;
	} else {
		DEBUG_Locate( 256, 128, MENU_MODE_CENTER ) ;
		DEBUG_SetColor( 255, 255, 255 ) ;
		DEBUG_Printf( "CONTINUE" ) ;
		DEBUG_Locate( 256, 144, MENU_MODE_CENTER ) ;
		DEBUG_SetColor( 255, 128, 128 ) ;
		DEBUG_Printf( "EXIT" ) ;
	}
#endif
	if(work->line_count == 0 && !work->flag){
		if ( pad->press & PAD_L ){
			work->which = 0 ;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);
		}else if ( pad->press & PAD_R ){
			work->which = 1;
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);
		}else if ( pad->press & ( PAD_OK | PAD_STA ) )
      {
         int bp_doFinish = 1;
			if(work->which)
         {
				/* EXIT */
#ifdef PSX2
            //BP - added system dialog on X360 to prevent destructive action
            //without confirmation.
            int confirmRet;
            ShowExitGameWarning( &confirmRet );
            if( confirmRet == 0 )   //yes
            {
               GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
            }
            else
            {
               //Pretend the user did not ever hit the button.
               bp_doFinish = 0;
            }
#endif
#ifdef KP_XBOX
			{
				/* 確認画面を出す */
				/* confirm::: 0:起動前 -1:起動中 1:YES選択 2:NO選択フェード始め 3:NO選択のち確認画面死んだ */
				extern void *NewGameOverExitConfirm( int *confirm  );
				void *res;
				res = NewGameOverExitConfirm( &work->confirm );
				if( res != NULL ){
					work->flag = 0;/* 確認画面に行くのでこのフラグは元の通りにしておく */
					work->count = 0;
					GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);
					return;
				}
			}
#endif				
			}else{
				/* CONTINUE */
				GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START001);
			}

         if( bp_doFinish )
         {
            work->count = 255;
            work->flag = 1;
         }
		}
	}
}

static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}

	/* パケットメモリ開放 */
#if 0 //BP_RENDER #ifdef PSX2
	if(work->gmover_pack[0]) GV_DelayedFree(work->gmover_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
}

static void SetNormVector( Work *work )
{
	short 	*point;
	float 	*norms;
	FVECTOR	fvtemp;
	long64	i;
	
	point = &( gameover_logo[ 1 ] );
	norms = work->norms;
	
	fvtemp.vz = 0.0f ;
	fvtemp.vw = 0.0f ;

	for( i = 0; i < 120; i++ ){
		fvtemp.vx = (float)(point[ 0 ] - point[ 2 ]);
		fvtemp.vy = (float)(point[ 1 ] - point[ 3 ]);
//		printf("fvtemp vx %f: vy %f: vz %f: vw %f:",fvtemp.vx,fvtemp.vy,fvtemp.vz,fvtemp.vw);
#if 0
		fpu_VectorNormal(&fvtemp);
#else
		_sceVu0Normalize(&fvtemp,&fvtemp);
#endif
//		printf("fvtemp vx %f: vy %f: vz %f: vw %f:",fvtemp.vx,fvtemp.vy,fvtemp.vz,fvtemp.vw);
		norms[ 0 ] = -fvtemp.vx;// / 8.0f;
		norms[ 1 ] = -fvtemp.vy;// / 8.0f;
//		printf("norms[%3d] x %f:y %f\n", i, norms[ 0 ], norms[ 1 ]);
		point += 4;
		norms += 2;
	}
}

static void InitLinePackes( Work *work )
{
	int 			i,j;
	LINE_VERTS_PACK *line_verts_pack;
	LINE_VERT_DATA	*verts;

	for( i = 0; i < 2; i++ ){
		line_verts_pack = &work->line_verts[i];
		line_verts_pack->giftag.tag = SCE_GIF_SET_TAG( 0, 1, 0, 0, 0, 1);
		line_verts_pack->giftag.regs = GS_REGS_AD;
		verts = line_verts_pack->verts;
		for( j = 0; j < N_LINE; j++ ){
			
			verts->rgbq0.reg = SCE_GS_RGBAQ;
			verts->rgbq1.reg = SCE_GS_RGBAQ;
			verts->xyz0.reg = SCE_GS_XYZ3;
			verts->xyz1.reg = SCE_GS_XYZ2;
			verts->rgbq0.data = 0;
			verts->rgbq1.data = 0;
			verts->xyz0.data = 0;
			verts->xyz1.data = 0;
			
			verts++;
		}
	}
}

static void InitPolyPackes( Work *work )
{
	int 			i,j;
	POLY_VERTS_PACK *poly_verts_pack;
	POLY_VERT_DATA	*verts;

	for( i = 0; i < 2; i++ ){
		poly_verts_pack = &work->poly_verts[i];
		poly_verts_pack->giftag.tag = SCE_GIF_SET_TAG( 0, 1, 0, 0, 0, 1);
		poly_verts_pack->giftag.regs = GS_REGS_AD;
		verts = poly_verts_pack->verts;
		for( j = 0; j < N_POLY; j++ ){
			verts->rgbq0.reg = SCE_GS_RGBAQ;
			verts->rgbq1.reg = SCE_GS_RGBAQ;
			verts->rgbq2.reg = SCE_GS_RGBAQ;
			verts->rgbq3.reg = SCE_GS_RGBAQ;
			verts->xyz0.reg = SCE_GS_XYZ3;
			verts->xyz1.reg = SCE_GS_XYZ3;
			verts->xyz2.reg = SCE_GS_XYZ2;
			verts->xyz3.reg = SCE_GS_XYZ2;
			verts->uv0.reg = SCE_GS_UV;
			verts->uv1.reg = SCE_GS_UV;
			verts->uv2.reg = SCE_GS_UV;
			verts->uv3.reg = SCE_GS_UV;
			verts->rgbq0.data = 0;
			verts->rgbq1.data = 0;
			verts->rgbq2.data = 0;
			verts->rgbq3.data = 0;
			verts->xyz0.data = 0;
			verts->xyz1.data = 0;
			verts->xyz2.data = 0;
			verts->xyz3.data = 0;
			verts->uv0.data = 0;
			verts->uv1.data = 0;
			verts->uv2.data = 0;
			verts->uv3.data = 0;
			
			verts++;
		}
	}
}

static int GetResources( Work *work, int mode )
{
	DG_DMAPACK		*dmapack;
	GMOVER_PACK		*packet;
	DG_TEX			*tex;
	DG_TEXTURE_LIST *texlist;
	
	int				i;
	
	work->mode = mode;
	work->count = 60;
	work->line_count = 1;
	work->timer = 0;
	work->n_lines = 0;
	work->n_polys = 0;
	work->flag = 0;
	work->stream_flag = 0;

	texlist = DG_GetTextureList( TRI_CODE );
	tex = DG_GetTexture2( TRI_CODE, TEX_CODE );

	{
#if 1
		//work->con_u[0] = tex->u_offset;
		//work->con_v[0] = tex->v_scale / 18.0f * 2.0f + tex->v_offset;
		//work->con_u[1] = tex->u_offset + tex->u_scale;
		//work->con_v[1] = work->con_v[0] + tex->v_scale / 18.0f;

		work->con_u[0] = 0.0f;
		work->con_v[0] = 0.0f	* 16.0f;
		work->con_u[1] = 160.0f * 16.0f;
		work->con_v[1] = 17.0f	* 16.0f;

		work->exi_u[0] = 0.0f;
		work->exi_v[0] = 17.0f	* 16.0f;
		work->exi_u[1] = 160.0f * 16.0f;
		work->exi_v[1] = 34.0f	* 16.0f;
#else
		work->con_u[0] = tex->u_offset;
		work->con_v[0] = tex->v_offset;
		work->con_u[1] = tex->u_offset + tex->u_scale;
		work->con_v[1] = tex->v_offset + tex->v_scale;

#endif

	}
	SetNormVector( work );
	
	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU,DG_DMAPACK_PHASE_LAST,254 );
	
//	work->dmapack= dmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
//											 DG_DMAPACK_PHASE_AFTER );
	
	if(dmapack==NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	DG_QueueDmapack( dmapack );
	
#if 0 //BP_RENDER #ifdef PSX2
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(GMOVER_PACK) * 2 );
	if(packet == NULL){ printf("ERR!! MALLOC WARP_PACK!!\n"); return -1; }


	work->gmover_pack[0] = dmapack->packet[0] = &packet[0] ;
	work->gmover_pack[1] = dmapack->packet[1] = &packet[1] ;
	InitLinePackes( work );
	InitPolyPackes( work );
	for ( i = 0 ; i < 2 ; i++ ){

		InitDmaTags( work, &packet[i], i );
		InitFrameClear( &packet[i].fade_pack,FADE_TEST,FADE_RGBAQ,FADE_ALPHA,0 );
		InitMdlDraw( &packet[i].line_draw, LINE_TEST, LINE_ALPHA, LINE_PRIM );
		InitSetTexDraw( &packet[i].poly_draw, 0, POLY_PRIM );
		TexDataSet( packet[i].tex_packet, &texlist->tex_packet[i],
					&packet[i].poly_draw, &tex->tex_trans,
					POLY_ALPHA );
 		//packet[i].poly_draw.data.tex0.data = SCE_GS_SET_TEX0( 0, 8, 0, 9, 8, 1, 0, 0, 0, 0, 0, 0 );
		//packet[i].poly_draw.data.clamp.data = SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 );
		//InitEndPacket( &packet[i].end_paket );
	}
#else
	if ( !(work->packet_mem = GV_Malloc( 256*256 )) ) {
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( work->packet_mem );
#endif
	
	//GCL_ExecProc( , NULL );

	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_OVER04);
	
	return 0;
}

void *NewGameOverWin2( int mode )
{
	Work		*work;

	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
								  sizeof( Work ), ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GM_StreamStopAll() ;
		if ( GM_GameOverVox >= 0 ) {
			work->stream_handler = GM_VoxStream( GM_GameOverVox, GM_STREAM_PLAY_WAIT ) ;
		} else {
			work->stream_handler = -1 ;
		}
		printf("stream_handler %d\n",work->stream_handler);
		printf("Game Over Start\n");
	}

	return work ;
}

/*----------------------------------------------------------------*/

/* ゲームオーバー音声設定（2000/10/11 M.Sonoyama） */
int		NewSetGameOverVox( void )
{
	GM_GameOverVox = GCL_GetOptionValue( 'v', -1 ) ;
	return 0 ;
}
