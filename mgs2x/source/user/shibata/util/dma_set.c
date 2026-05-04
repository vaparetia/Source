//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
     dma_set.c
	 DMAやらGIFやらの設定関数など

	 2000/07/11 T.Shibata

	 $Id: dma_set.c,v 1.1.1.3 2002/11/19 11:48:53 Yoshizawa1 Exp $
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
#include "libutl.h"

#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"../util/ts_util.h"
#include	"dma_set.h"
#include "dmapack.h"

// モデルの頂点数
int GetMdlVerts(DG_DEF *def)
{
	DG_MDL *mdl = NULL;
	DG_OBJPACK *objpack = NULL;
	int 	i,n_verts = 0;
	
	mdl = def->models;
	objpack = mdl->packs;
	
//	printf("N_MDL = %d\n",def->n_models);
//	printf("N_PACK = %d\n",mdl->n_packs);
#ifdef PSX2
	for( i = 0; i < mdl->n_packs; i++ ){
		//printf("N_VERTS[%d]_OBJ = %d\n",i,objpack->n_verts);
		n_verts += objpack->n_verts;
		objpack++;
	
	}
#else //KP_XBOX
	for( i = 0; i < mdl->n_packs; i++ ){
		n_verts += objpack->n_indices;
		objpack++;	
	}	
#endif
//	printf("N_VERTS = %d\n",n_verts);
//	printf("N_POLYS = %d\n",n_polys);
	
	return n_verts;
}

// モデルの頂点数
int GetMdlVertsJoint( DG_DEF *def, int joint )
{
	DG_MDL *mdl = NULL;
	DG_OBJPACK *objpack = NULL;
	int 	i,n_verts = 0;

	if( def->n_models <= joint ) return 0;

	mdl = &def->models[joint];
	objpack = mdl->packs;
	
//	printf("N_MDL = %d\n",def->n_models);
//	printf("N_PACK = %d\n",mdl->n_packs);
#ifdef PSX2
	for( i = 0; i < mdl->n_packs; i++ ){
		//printf("N_VERTS[%d]_OBJ = %d\n",i,objpack->n_verts);
		n_verts += objpack->n_verts;
		objpack++;
	
	}
#else //KP_XBOX
	for( i = 0; i < mdl->n_packs; i++ ){
		n_verts += objpack->n_indices;
		objpack++;	
	}
#endif
//	printf("N_VERTS = %d\n",n_verts);
//	printf("N_POLYS = %d\n",n_polys);
	
	return n_verts;
}


// モデルの頂点数
int GetMdlVerts_CV2(CV2_DEF *def)
{
	CV2_MDL *mdl = NULL;
	int 	i,n_verts = 0;
	
	mdl = def->models;
	//objpack = mdl->packs;
	
	printf("N_MDL = %d\n",def->n_models);
	for( i = 0; i < def->n_models; i++ ){
		//printf("N_VERTS[%d]_OBJ = %d\n",i,objpack->n_verts);
		n_verts += mdl->n_verts;
		mdl++;
	}
	printf("N_VERTS = %d\n",n_verts);
//	printf("N_POLYS = %d\n",n_polys);
	
	return n_verts;
}

#if 0 //BP_PS2	///madakore
void InitDrowFlush( DROW_FLUSH *drow_frush )
{
	static DROW_FLUSH _init_drow_frush = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _drow_flush_data), 1, 0, 0, 0, 1),
					.regs = GS_REGS_AD },
		.data = {
			.texflush0 = { .reg = SCE_GS_TEXFLUSH, .data = 0 },
		}
	};
	
	*drow_frush = _init_drow_frush;
}
#else
void *InitDrowFlush( void *prim )
{
	return prim ; /* XBOX では、いらない */
}
#endif

//画面退避
#if 0 //BP_PS2	///madakore
void InitTrBuffer( TR_BUFFER *tr_buffer, int which )
{
	static TR_BUFFER _init_tr_buffer = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _tr_buffer_data), 1, 0, 0, 0, 1),
					.regs = GS_REGS_AD },
		.data = {			
			.bitbltbuf = { .reg = SCE_GS_BITBLTBUF,
						   .data = SCE_GS_SET_BITBLTBUF( BUFFER_PAGE(0)/64, DRAW_WIDTH/64, FRAME_BUFFER_COLOR_MODE(),
														 TEXTURE_TOP_PAGE()/64, DRAW_WIDTH/64, FRAME_BUFFER_COLOR_MODE()) },
			.trxpos = { .reg = SCE_GS_TRXPOS, .data = SCE_GS_SET_TRXPOS(0,0,0,0,0) },
			.trxreg = { .reg = SCE_GS_TRXREG, .data = SCE_GS_SET_TRXREG(DRAW_WIDTH,DRAW_HEIGHT) },
			.trxdir = { .reg = SCE_GS_TRXDIR, .data = SCE_GS_SET_TRXDIR(2) },
			.texflush0 = { .reg = SCE_GS_TEXFLUSH, .data = 0 },
			.texflush1 = { .reg = SCE_GS_TEXFLUSH, .data = 0 },
		}
	};
	
	*tr_buffer = _init_tr_buffer;

	tr_buffer->data.bitbltbuf.data =
		SCE_GS_SET_BITBLTBUF( BUFFER_PAGE(which)/64, DRAW_WIDTH/64, FRAME_BUFFER_COLOR_MODE(),
							  TEXTURE_TOP_PAGE()/64, DRAW_WIDTH/64, FRAME_BUFFER_COLOR_MODE());
}
#else
void *InitTrBuffer( void *packet, int which )
{
	extern void *DG_SetDmapackBackupFrame( void *addr, int page ) ;

	/*which	ページ指定（０で現在描画中のフレームバッファ、１で表示中のフレームバッファ）*/
	packet = DG_SetDmapackBackupFrame( packet, which ) ;
	///* 念のためアルファテストを無効にする */
	//packet = DG_SetDmapackModeEnable( packet, DG_DMAPACK_MODE_NO_ALPHATEST );
	return packet ;
}
#endif

//フレームクリアー(フェードにも使えるかも)
#if 0 //BP_PS2	///madakore
void InitFrameClear( FRAME_CLEAR *frame_clear, u_long64 test, u_long64 rgba, u_long64 alpha, long64 z )
{
	static FRAME_CLEAR _init_frame_clear = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _frame_clear_data), 1, 0, 0, 0, 1),
					.regs = GS_REGS_AD },
		.data = {
			.test  = { .reg = SCE_GS_TEST_1,
					   .data = SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 1 )},
			.rgbq  = { .reg = SCE_GS_RGBAQ,
					   .data = SCE_GS_SET_RGBAQ(0,0,0,0x80,0)},
			.alpha = { .reg = SCE_GS_ALPHA_1,
					   .data = SCE_GS_SET_ALPHA(2,2,2,0,0x80)},
			.prim  = { .reg = SCE_GS_PRIM,
					   .data = SCE_GS_SET_PRIM(6,0,0,0,1,0,1,0,0)},
			.xyz0  = { .reg = SCE_GS_XYZ2,
					   .data = SCE_GS_SET_XYZ( 0,0,0)},
			.xyz1  = { .reg = SCE_GS_XYZ2,
					   .data = SCE_GS_SET_XYZ( 0,0,0)},
		}
	};

	*frame_clear = _init_frame_clear;

	if( test )frame_clear->data.test.data = test;
	frame_clear->data.alpha.data = alpha;
	frame_clear->data.rgbq.data = rgba;
	
	frame_clear->data.xyz0.data = SCE_GS_SET_XYZ( MIN_X,MIN_Y,z);
	frame_clear->data.xyz1.data = SCE_GS_SET_XYZ( MAX_X,MAX_Y,z);
}
#else
void *InitFrameClear( void *packet, u_long64 test, u_long64 rgba, u_long64 alpha, long64 z )
{
	packet = DG_SetDmapackAlpha( packet, alpha ) ;
	packet = DG_SetDmapackBox( packet, MIN_X,MIN_Y, MAX_X,MAX_Y, DG_MakeDmaPackColorFromInt( (unsigned int) rgba ) );

	return packet ;
}
#endif


//退避したフレームバッファーをテクスチャーとしたモデル描画の設定
#if 0 //BP_PS2
void InitMdlDraw(MDL_DRAW *mdl_draw, u_long64 test, u_long64 alpha, u_long64 prim )
{
	static MDL_DRAW _init_mdl_draw = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _mdl_data), 1, 0, 0, 0, 1),
					.regs = GS_REGS_AD },
		.data = {
			.test  = { .reg = SCE_GS_TEST_1,
					   .data = SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 )},
			.clamp = { .reg = SCE_GS_CLAMP_1,
					   .data = SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1,1,DRAW_HEIGHT-2)},
			.tex0  = { .reg = SCE_GS_TEX0_1,
					   .data = SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE()/64,
												BUFFER_WIDTH/64,
												FRAME_BUFFER_COLOR_MODE(),
												10,10,0,0,0,0,0,0,0)},
			.rgbq  = { .reg = SCE_GS_RGBAQ,
					   .data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0x3F800000)},
			.alpha = { .reg = SCE_GS_ALPHA_1,
					   .data = SCE_GS_SET_ALPHA(2,2,2,0,0x80)},
			.prim  = { .reg = SCE_GS_PRIM,
					   .data = SCE_GS_SET_PRIM(4,1,1,0,0,0,1,0,0)}
		}
	};
	
	*mdl_draw = _init_mdl_draw;

	mdl_draw->data.test.data = test;
	mdl_draw->data.alpha.data = alpha;
	mdl_draw->data.prim.data = prim;
}
#else
void *InitMdlDraw( void *packet, u_long64 test, u_long64 alpha, u_long64 prim )
{
	packet = DG_SetDmapackUseFrameTex( packet, 2 ) ;
	packet = DG_SetDmapackAlpha( packet, alpha ) ;
	if ( alpha == 0 ){
		/* 念のためアルファテストを無効にする */
		packet = DG_SetDmapackModeEnable( packet, DG_DMAPACK_MODE_NO_ALPHATEST );
	}
	
	return packet ;
}
#endif

//テクスチャー初期化
void InitSetTexDraw( TEX_DRAW *tex_draw, u_long64 rgba, u_long64 prim )
{
#if 0 //BP_PS2	///madakore
	static TEX_DRAW _init_tex_draw = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _tex_draw_data), 1, 0, 0, 0, 1),
					.regs = GS_REGS_AD },
		.data = {
			.clamp = { .reg = SCE_GS_CLAMP_1,
					   .data = SCE_GS_SET_CLAMP( 1, 1, 0, 0, 0, 0 )},
			.test  = { .reg = SCE_GS_TEST_1,
					   .data = SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 1 )},
			.tex2  = { .reg = SCE_GS_TEX0_1,
					   .data = SCE_GS_SET_TEX0( 0,0,0,10,10,0,0,0,0,0,0,0)},
			.tex0  = { .reg = SCE_GS_TEX0_1,
					   .data = SCE_GS_SET_TEX0( 0,0,0,10,10,0,0,0,0,0,0,0)},
			.alpha = { .reg = SCE_GS_ALPHA_1,
					   .data = SCE_GS_SET_ALPHA(0,1,0,1,0x80)},
			.rgbq  = { .reg = SCE_GS_RGBAQ,
					   .data = SCE_GS_SET_RGBAQ(0,0,0,0,0)},
			.prim  = { .reg = SCE_GS_PRIM,
					   .data = SCE_GS_SET_PRIM(6,0,1,0,1,0,0,0,0) },
			.texflush = { .reg = SCE_GS_TEXFLUSH, .data = 0 },
		}
	};
	
	*tex_draw = _init_tex_draw;
	tex_draw->data.rgbq.data = rgba;
	tex_draw->data.prim.data = prim;
#endif
}

//ピクセルテスト復元
void InitEndPacket( PACKET_END *packet )
{
#if 0 //BP_PS2	///madakore
	static PACKET_END _init_packet  = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _packet_end_data), 1, 0, 0, 0, 1),
					.regs = GS_REGS_AD },
		.data = {
			.test  = { .reg = SCE_GS_TEST_1,
					   .data = SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 1 )},
		}
	};

	*packet = _init_packet;
#endif
}

//テクスチャー転送パケット設定
void TexDataSet( void *dst, DG_TEX_PACKET *src,
				 TEX_DRAW *tex_draw, DG_TEX_TRANS *tex_trans, u_long64 alpha )
{
	struct _tex_draw_data *texdata = &tex_draw->data;

	DG_WriteTextureChangePacks( dst, src );

	texdata->clamp = tex_trans->clamp;
	texdata->tex2 = tex_trans->tex2;
	texdata->tex0 = tex_trans->tex0;
	texdata->alpha = tex_trans->alpha;

	if(alpha) texdata->alpha.data = alpha;

}
