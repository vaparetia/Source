/*
	blur.c
	自由形プリミティブ使用実験プログラム（ブラーエフェクト）

	1999/07/07 K.Takabe
	$Id: blur.c,v 1.1.1.3 2002/11/19 11:51:28 Yoshizawa1 Exp $

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
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"


#define N_PRIMS	(1)
#define N_PACKETS	(2)

typedef	struct	{
	GV_ACT		actor ;

	DG_PRIM		*prim ;
	void		*prim_work ;

} Work ;

typedef struct {
	u_long			gif_tag ;
	u_long			gif_tag_regs ;
	u_long			alpha ;
	u_long			alpha_addr ;
	u_long			clamp ;
	u_long			clamp_addr ;
	u_long			tex0 ;
	u_long			tex0_addr ;
	u_long			test1 ;
	u_long			test1_addr ;
	u_long			prim ;
	u_long			prim_addr ;
	u_long			rgbaq ;
	u_long			rgbaq_addr ;
	u_long			uv1 ;
	u_long			uv1_addr ;
	u_long			xyz1 ;
	u_long			xyz1_addr ;
	u_long			uv2 ;
	u_long			uv2_addr ;
	u_long			xyz2 ;
	u_long			xyz2_addr ;
	u_long			test2 ;
	u_long			test2_addr ;
} SCREEN_DRAW ALIGN16;

static SCREEN_DRAW def_screen_draw = {
	SCE_GIF_SET_TAG(sizeof(SCREEN_DRAW)/16-1, 1, 0, 0, 0, 1),GS_REGS_AD,
	SCE_GS_SET_ALPHA(0,1,0,1,64),0x42,/* alpha */
	SCE_GS_SET_CLAMP(1,0,0,0,0,0),0x08,/* clamp */
	SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0),0x06,/* tex0 */
	SCE_GS_SET_TEST(0,0,0,0,0,0,0,0),0x47,/* test */
	SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0),0x00,/* prim */
	SCE_GS_SET_RGBAQ(129,129,129,64,0),0x01,/* rgbaq */
	SCE_GS_SET_UV(0+8,0+8),0x03,/* uv */
	SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16+8), ((2048-DRAW_HEIGHT/2)*16+8), 0xffffffff),0x05,/* xyz2 */
	SCE_GS_SET_UV(DRAW_WIDTH*16+16-8,DRAW_HEIGHT*16+16-8),0x03,/* uv */
	SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16-8), ((2048+DRAW_HEIGHT/2)*16-8), 0xffffffff),0x05,/* xyz2 */
	SCE_GS_SET_TEST(1,7,0,1,0,0,0,0),0x47,/* test */
};

typedef struct {
	DG_PRIM_HEADER		header ;		/* ソート用ワーク */
	DG_DMATAG			dma_tag ;		/* ＤＭＡタグ */
	SCREEN_DRAW			prim ;
} PRIM_PACKET ;

static void Act( Work *work )
{
}

static void Die( Work *work )
{
	GV_Free( work->prim_work );
	GM_FreePrim( work->prim );
}

static int GetResources( Work *work )
{
	DG_PRIM	*prim ;
	PRIM_PACKET	*packet ;
	int		i ;

	prim = work->prim = GM_MakePrim( DG_PRIM_SORTONLY|DG_PRIM_FREEPRIM, 1, 1, NULL, NULL );
	packet = work->prim_work = GV_Malloc( sizeof(PRIM_PACKET) * 2 );

	prim->packet_size = sizeof(PRIM_PACKET);
	prim->near_z = -100000 ;
	for ( i = 0 ; i < 2 ; i++ ){
		prim->packs[i] = (void*)&packet[i] ;
		packet[i].header.sort_z = -100000 + 1 ;
		packet[i].dma_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, sizeof(SCREEN_DRAW)/sizeof(u_long128) );
		packet[i].dma_tag.addr = NULL ;
		packet[i].dma_tag.vifcode[0] = SCE_VIF1_SET_NOP(0) ;
		packet[i].dma_tag.vifcode[1] = SCE_VIF1_SET_DIRECT( sizeof(SCREEN_DRAW)/sizeof(u_long128), 0) ;
		packet[i].prim = def_screen_draw ;
		packet[i].prim.tex0 = SCE_GS_SET_TEX0(
											  BUFFER_PAGE(1-i) / 64 ,
											  BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),10,10,0,0,0,0,0,0,0);
	}

	return (0);
}


void *NewBlurTest( void )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
