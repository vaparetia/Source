//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    tobcon.c
    トゥービーコンテヌー
	2000/10/05 T.Shibata

	$Id: tobcon.c,v 1.1.1.3 2002/11/19 11:48:46 Yoshizawa1 Exp $
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
#include	"../util/dma_set.h"

#define	ACTOR_PRIO			(254)

#define	N_LINE				(166)
#define	N_LINE_ALLVERTS		(N_LINE * 2)

#define N_POLY				(12)
#define N_POLY_ALLVERTS		(N_POLY*4)

//#define	TRI_CODE			(GV_StrCode("gover"))
//#define TEX_CODE			(GV_StrCode("over_menu_alp_ovl"))

#define	TRI_CODE			(11101464)
#define TEX_CODE			(9697079)

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


#define NO_CONTINUE_EXIT_X0			( 160 - 64 / 2 )


#define CONTINUE_X0		128
#define CONTINUE_Y0		121

static short gameover_logo[] = {
	166,
	19, 93, 39, 93,
	19, 93, 18, 98,
	39, 93, 38, 98,
	31, 98, 38, 98,
	31, 98, 25, 129,
	25, 129, 19, 129,
	42, 93, 60, 93,
	42, 93, 41, 98,
	60, 93, 59, 98,
	41, 98, 59, 98,
	40, 102, 37, 117,
	37, 117, 55, 117,
	55, 117, 58, 102,
	40, 102, 46, 102,
	52, 102, 58, 102,
	44, 112, 46, 102,
	44, 112, 50, 112,
	52, 102, 50, 112,
	18, 98, 25, 98,
	25, 98, 19, 129,
	68, 98, 69, 93,
	69, 93, 85, 93,
	68, 98, 80, 98,
	77, 102, 67, 102,
	67, 102, 64, 117,
	64, 117, 79, 117,
	79, 117, 84, 113,
	85, 109, 82, 105,
	87, 98, 85, 93,
	87, 98, 82, 105,
	80, 98, 77, 102,
	71, 112, 76, 112,
	76, 112, 79, 110,
	71, 112, 72, 107,
	72, 107, 77, 107,
	79, 110, 77, 107,
	85, 109, 84, 113,
	86, 117, 102, 117,
	91, 93, 107, 93,
	90, 98, 106, 98,
	89, 102, 105, 102,
	89, 102, 86, 117,
	91, 93, 90, 98,
	107, 93, 106, 98,
	104, 107, 94, 107,
	105, 102, 104, 107,
	93, 112, 103, 112,
	103, 112, 102, 117,
	93, 112, 94, 107,
	122, 93, 135, 93,
	122, 93, 112, 105,
	130, 117, 116, 117,
	125, 98, 134, 98,
	135, 93, 134, 98,
	125, 98, 119, 105,
	119, 105, 121, 112,
	121, 112, 131, 112,
	131, 112, 130, 117,
	112, 105, 116, 117,
	157, 93, 139, 93,
	134, 117, 152, 117,
	138, 98, 156, 98,
	143, 102, 141, 112,
	141, 112, 147, 112,
	149, 102, 147, 112,
	137, 102, 143, 102,
	149, 102, 155, 102,
	139, 93, 138, 98,
	137, 102, 134, 117,
	155, 102, 152, 117,
	157, 93, 156, 98,
	161, 93, 160, 98,
	161, 93, 167, 93,
	170, 102, 172, 106,
	174, 98, 175, 93,
	175, 93, 181, 93,
	179, 102, 176, 117,
	156, 117, 162, 117,
	162, 117, 165, 103,
	165, 103, 171, 117,
	171, 117, 176, 117,
	160, 98, 169, 98,
	167, 93, 169, 98,
	159, 102, 170, 102,
	159, 102, 156, 117,
	174, 98, 180, 98,
	173, 102, 172, 106,
	173, 102, 179, 102,
	180, 98, 181, 93,
	184, 93, 183, 98,
	204, 93, 184, 93,
	204, 93, 203, 98,
	203, 98, 183, 98,
	189, 102, 186, 117,
	195, 102, 192, 117,
	186, 117, 192, 117,
	189, 102, 195, 102,
	205, 102, 202, 117,
	207, 93, 213, 93,
	211, 102, 208, 117,
	202, 117, 208, 117,
	206, 98, 212, 98,
	205, 102, 211, 102,
	207, 93, 206, 98,
	213, 93, 212, 98,
	215, 102, 212, 117,
	212, 117, 218, 117,
	218, 117, 221, 103,
	217, 93, 223, 93,
	223, 93, 225, 98,
	225, 98, 216, 98,
	215, 102, 226, 102,
	226, 102, 228, 106,
	221, 103, 227, 117,
	227, 117, 232, 117,
	228, 106, 229, 102,
	229, 102, 235, 102,
	235, 102, 232, 117,
	217, 93, 216, 98,
	230, 98, 236, 98,
	230, 98, 231, 93,
	236, 98, 237, 93,
	231, 93, 237, 93,
	239, 102, 236, 117,
	241, 93, 247, 93,
	236, 117, 254, 117,
	254, 117, 257, 102,
	243, 112, 249, 112,
	243, 112, 245, 102,
	249, 112, 251, 102,
	240, 98, 246, 98,
	252, 98, 258, 98,
	239, 102, 245, 102,
	251, 102, 257, 102,
	253, 93, 259, 93,
	241, 93, 240, 98,
	247, 93, 246, 98,
	253, 93, 252, 98,
	259, 93, 258, 98,
	258, 117, 274, 117,
	258, 117, 261, 102,
	263, 93, 279, 93,
	277, 102, 276, 107,
	262, 98, 278, 98,
	261, 102, 277, 102,
	263, 93, 262, 98,
	279, 93, 278, 98,
	266, 107, 276, 107,
	275, 112, 265, 112,
	266, 107, 265, 112,
	275, 112, 274, 117,
	281, 102, 278, 117,
	283, 93, 298, 93,
	298, 93, 302, 104,
	302, 104, 293, 117,
	293, 117, 278, 117,
	282, 98, 293, 98,
	293, 98, 295, 104,
	295, 104, 289, 112,
	289, 112, 285, 112,
	285, 112, 287, 102,
	281, 102, 287, 102,
	283, 93, 282, 98,
	30, 120, 28, 129,
	30, 120, 291, 120,
	28, 129, 285, 129,
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

	float		norms[ N_LINE * 2 ];

	LINE_VERTS_PACK		line_verts[2];
	POLY_VERTS_PACK		poly_verts[2];
	int					n_lines;
	int					n_polys;
	int					timer;
	DG_DMAPACK			*dmapack;

	GMOVER_PACK			*gmover_pack[2];
	int					flag;
	int 				name;
	int					proc_id;
	int					handle;
	int					tsymbol;

#ifndef PSX2
	void				*packet_mem;
	void                *prim ;
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
static void SetLine( Work *work, int rgba0, int rgba1, float x0, float y0, float x1, float y1 )
{
#ifdef PSX2
	LINE_VERT_DATA	*verts = &work->line_verts[DG_Clock].verts[work->n_lines];
	int				ix0,iy0,ix1,iy1;
	if(work->n_lines >= N_LINE ){ printf("line packet over!!\n"); return; }
	
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

#else /*PSX2*/


	work->prim = DG_SetDmapackAlpha( work->prim, LINE_ALPHA ) ;
	work->prim = DG_SetDmapackLine( work->prim,
								    MIN_X + (((float)(DRAW_WIDTH) * (x0 / 320.0f))*SHIBATA_FSHIFT),
								    MIN_Y + (((float)(DRAW_HEIGHT) * (y0 / 240.0f))*SHIBATA_FSHIFT),
								    rgba0,
								    MIN_X + (((float)(DRAW_WIDTH) * (x1 / 320.0f))*SHIBATA_FSHIFT),
								    MIN_Y + (((float)(DRAW_HEIGHT) * (y1 / 240.0f))*SHIBATA_FSHIFT),
								    rgba1 ) ;

#endif /*PSX2*/
}



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


#ifdef PSX2
//0xaabbggrr
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
	SetTexUV( &u, &v, tex, poly->u0, poly->v0 ) ;
	work->prim = DG_SetDmapackVertex( work->prim,
							   MIN_X + ((float)(DRAW_WIDTH) * (poly->x0 / 320.0f)*SHIBATA_FSHIFT),
							   MIN_Y + ((float)(DRAW_HEIGHT) * (poly->y0 / 240.0f)*SHIBATA_FSHIFT),
							   u,
							   v,
							   poly->rgba0   ) ;
	SetTexUV( &u, &v, tex, poly->u1, poly->v1 ) ;
	work->prim = DG_SetDmapackVertex( work->prim,
							   MIN_X + ((float)(DRAW_WIDTH) * (poly->x1 / 320.0f)*SHIBATA_FSHIFT),
							   MIN_Y + ((float)(DRAW_HEIGHT) * (poly->y1 / 240.0f)*SHIBATA_FSHIFT),
							   u,
							   v,
							   poly->rgba1   ) ;
	SetTexUV( &u, &v, tex, poly->u2, poly->v2 ) ;
	work->prim = DG_SetDmapackVertex( work->prim,
							   MIN_X + ((float)(DRAW_WIDTH) * (poly->x2 / 320.0f)*SHIBATA_FSHIFT),
							   MIN_Y + ((float)(DRAW_HEIGHT) * (poly->y2 / 240.0f)*SHIBATA_FSHIFT),
							   u,
							   v,
							   poly->rgba2   ) ;
	SetTexUV( &u, &v, tex, poly->u3, poly->v3 ) ;
	work->prim = DG_SetDmapackVertex( work->prim,
							   MIN_X + ((float)(DRAW_WIDTH) * (poly->x3 / 320.0f)*SHIBATA_FSHIFT),
							   MIN_Y + ((float)(DRAW_HEIGHT) * (poly->y3 / 240.0f)*SHIBATA_FSHIFT),
							   u,
							   v,
							   poly->rgba3   ) ;
}
#endif /*PSX2*/

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
	
	i = N_LINE;

	if ( count > 0 && !(work->timer % 3)) work->line_count = ( count >= i + 16 ) ? 0 : count + 2 ;

	update = 1 ;
	
	if ( count == 0 ) {
		update = 0 ;
		count = 16 ;
	}
	
	for ( i = 0; i < N_LINE ; i++ ) {
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

	if( work->line_count == 0 && work->flag >= 1){
		/* セレクトフェーズ */
		/* この時には、Rediant は用済みなので、それを使う */
		long64 			col1, br;
		long64 			time;
		POLY_INFO	   	con,exi;
		DG_TEX 			*tex;
		DG_TEXTURE_LIST	*texlist;

		texlist = DG_GetTextureList( TRI_CODE );
		tex = DG_GetTexture2( TRI_CODE, TEX_CODE );
   
		InitSetTexDraw( &work->gmover_pack[DG_Clock]->poly_draw, 0, POLY_PRIM2 );
		TexDataSet( work->gmover_pack[DG_Clock]->tex_packet, &texlist->tex_packet[DG_Clock],
					&work->gmover_pack[DG_Clock]->poly_draw, &tex->tex_trans,
					POLY_ALPHA2 );
		if( work->flag == 3){
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
		con.x1 = CONTINUE_X0 + 64.0f;
		con.y1 = CONTINUE_Y0;
		con.x2 = CONTINUE_X0;
		con.y2 = CONTINUE_Y0 + 7.0f;
		con.x3 = CONTINUE_X0 + 64.0f;
		con.y3 = CONTINUE_Y0 + 7.0f;
		
		con.u0 = work->con_u[0];
		con.v0 = work->con_v[0];
		con.u1 = work->con_u[1];
		con.v1 = work->con_v[0];
		con.u2 = work->con_u[0];
		con.v2 = work->con_v[1];
		con.u3 = work->con_u[1];
		con.v3 = work->con_v[1];

		//if( work->continue_flag ){
		col1 = ( 0 | ( br << 8 ) | ( br << 16 ) | (0x80<<24) ) ;
		
		con.rgba0 = col1;
		con.rgba1 = col1;
		con.rgba2 = col1;
		con.rgba3 = col1;

		SetPoly( work, &con );

		//} else {
		//work->current = 1;
		//col2 = ( 0 | ( br << 8 ) | ( br << 16 ) ) ;
		//next = set_sprt_tex( next, NO_CONTINUE_EXIT_X0, EXIT_Y0, TEX_EXIT, col2, ot );
		//}

		if(work->flag == 3){
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

	//セットタグ
	SetLineDmaTag( work, work->gmover_pack[DG_Clock], DG_Clock );
	SetPolyDmaTag( work, work->gmover_pack[DG_Clock], DG_Clock );
}

static void Fade_Act( Work *work, long64 fade )
{

	if( fade > 128 ) fade = 128;
	//if( fade > 64 ) fade = 64;
#ifdef PSX2
	InitFrameClear( &work->gmover_pack[DG_Clock]->fade_pack,FADE_TEST,
					SCE_GS_SET_RGBAQ(0x00,0x00,0x00,fade,0),FADE_ALPHA,0 );
#else
	work->prim = InitFrameClear( work->prim,FADE_TEST,
								SCE_GS_SET_RGBAQ(0x00,0x00,0x00,fade,0),FADE_ALPHA,0 );
#endif
}

static void Act( Work *work )
{
	GV_PAD		*pad = GV_PadDataDirect;


	DG_UnDrawFrameCount = 0;
//	if(work->timer++ % 3 && work->count > 0) return;	//三割り込み
	
	if(work->line_count == 0 ){
		switch(work->flag){
		case 0:
			//ストリームスタート
			if( work->tsymbol ){
				if(GM_StreamStatus( work->handle ) == GM_STREAM_STATE_WAIT){
					GM_StreamStart( work->handle );
					work->flag++;
				}
			}else{
				work->flag++;
			}
			break;
		case 1:
            //ストリーム終了チェック
			if( !work->tsymbol || GM_StreamStatus( work->handle ) == GM_STREAM_STATE_END){
				work->flag++;
			}
		case 2:
            //EXITセレクト
			if( pad->press & ( PAD_OK | PAD_STA | PAD_CANCEL)) {
				if( work->tsymbol ) GM_StreamStop(work->handle);
				work->count = 255;
				work->flag = 3;
			
				GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
			}
			break;
		default:
            //フェード

			break;
		}
	}

#ifndef PSX2
	work->prim = work->dmapack->autopacket ;
#endif

	Fade_Act( work, work->timer );
	GameOverPrims_Act( work );

#ifndef PSX2			
	DG_SetDmapackEnd( work->prim );
#endif

//	if ( work->line_count > 0 && !(work->timer % 3))
//		work->line_count = ( work->line_count >= N_LINE + 16 ) ? 0 : work->line_count + 2 ;
	work->timer++;
	
	if ( work->count > 0 ) {
		if ( -- work->count > 0 ){
			return ;
		}
		if(work->flag == 3){
			//GM_GameOverRestart( work->which ) ;
			GCL_ExecProc( work->proc_id, NULL );
			GV_DestroyActor( work ) ;
			return;
		}else{
			//GM_GameOverClear() ;
			printf("Game Over Select Start\n");
		}
	}
#if 0
	if(work->line_count == 0 ){
		switch(work->flag){
		case 0:
			//ストリームスタート
			if(work->handle != -1){
				if(GM_StreamStatus( work->handle ) == GM_STREAM_STATE_WAIT){
					GM_StreamStart( work->handle );
					work->flag++;
				}
			}else{
				work->flag++;
			}
			break;
		case 1:
            //ストリーム終了チェック
			if(GM_StreamStatus( work->handle ) == GM_STREAM_STATE_END){
				work->flag++;
			}else if(pad->press & ( PAD_CANCEL )){
				GM_StreamStop(work->handle);
				work->flag++;
			}
			break;
		case 2:
            //EXITセレクト
			if( pad->press & ( PAD_OK | PAD_STA )) {
				work->count = 255;
				work->flag++;
			
				GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
			}
			break;
		default:
            //フェード

			break;
		}
	}
#endif
//	GameOverPrims_Act( work );
//	Fade_Act( work, work->timer );

}

static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	
	/* パケットメモリ開放 */
#ifdef PSX2
	if(work->gmover_pack[0]) GV_DelayedFree(work->gmover_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
	//GV_PauseOffActorSystem(GV_PAUSE_MENU);
	//GM_ResetGameStatus( STATE_PAUSE_DISABLE );

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

	for( i = 0; i < N_LINE; i++ ){
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

static int GetResources( Work *work )
{
	DG_DMAPACK		*dmapack;
	GMOVER_PACK		*packet;
	DG_TEX			*tex;
	DG_TEXTURE_LIST *texlist;
	
	int				i;
	
//	work->mode = mode;
	work->count = 60;
	work->line_count = 1;
	work->timer = 0;
	work->n_lines = 0;
	work->n_polys = 0;
	work->flag = 0;
	
	work->proc_id = GCL_GetOptionValue( 'p', 0 );
	work->tsymbol = GCL_GetOptionValue( 'r', 0 );
	if(work->tsymbol){
		work->handle = GM_VoxStream( work->tsymbol, GM_STREAM_PLAY_WAIT );
	}

	texlist = DG_GetTextureList( TRI_CODE );
	tex = DG_GetTexture2( TRI_CODE, TEX_CODE );

	{
#if 1
		//work->con_u[0] = tex->u_offset;
		//work->con_v[0] = tex->v_scale / 18.0f * 2.0f + tex->v_offset;
		//work->con_u[1] = tex->u_offset + tex->u_scale;
		//work->con_v[1] = work->con_v[0] + tex->v_scale / 18.0f;

		work->con_u[0] = 0.0f;
		work->con_v[0] = (16.0f + 0.5f)*16.0f;
		work->con_u[1] = 160.0f * 16.0f;
		work->con_v[1] = (32.0f - 0.5f)*16.0f;

#else
		work->con_u[0] = tex->u_offset;
		work->con_v[0] = tex->v_offset;
		work->con_u[1] = tex->u_offset + tex->u_scale;
		work->con_v[1] = tex->v_offset + tex->v_scale;

#endif

	}
	SetNormVector( work );
	
	// ＤＭＡパケット型オブジェクト作成

	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU,DG_DMAPACK_PHASE_AFTER,128 );
	
//	work->dmapack= dmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
//											 DG_DMAPACK_PHASE_AFTER );
	
	if(dmapack==NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	DG_QueueDmapack( dmapack );

#ifdef PSX2	
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

	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_OVER04);
	
//	GV_PauseOnActorSystem(GV_PAUSE_MENU);
	

//    GM_SetGameStatus( STATE_PAUSE_DISABLE );

	return 0;
}

void *NewToBeContinued( int name, int map )
{
	Work		*work;

	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
								  sizeof( Work ), ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		printf("Game Over Start\n");
	}

	return work ;
}
