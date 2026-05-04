/*
	subwin.c
		子画面

	1999/10/07 K.Uehara
	$Id: subwin.c,v 1.1.1.3 2002/11/19 11:51:38 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef _XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#endif

#include "gameheader.h"

typedef struct {
	GV_ACT actor;
	int name;
	int x0, y0, x1, y1;
} Work;

typedef struct {
	sceGsRgbaq rgba;
	sceGsPrim prim;
	DG_PRIM_XY xy1;
	DG_PRIM_XY xy2;
	DG_PRIM_XY xy3;
	DG_PRIM_XY xy4;
	DG_PRIM_XY xy5;
} MENU_LINE_BOX;

static void Act( Work *work )
{
	GV_MSG *msg;
	int n, pflag ;
	DG_CHANL *cp;

	cp = DG_Chanl( 1 );
	pflag = cp->flag ;
	if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
	    for( ; n > 0; n-- ){
			cp->flag = msg->message[ 0 ];
			msg++;
	    }
	}
	if ( pflag != cp->flag ) {
	    if ( cp->flag ){
			GM_SetMenuStatus( MENU_SUBWIN_ON ) ;
			GM_UseCamera( 1 ) ;
		} else {
			GM_ResetMenuStatus( MENU_SUBWIN_ON ) ;
			GM_NoUseCamera( 1 ) ;
		}
	}

	if( cp->flag ){
		/* 子画面のフレーム表示 */
		MENU_PRIM *prim;
		sceGifTag *giftag;
		MENU_LINE_BOX *box;

		MENU_SetAlphaMode( 0,1,0,1,0x80 ) ;

		prim = MENU_OpenPrim();

		MENU_NEW_TAG( giftag, prim );
		DG_SET_GIFTAG( giftag, .FLG = SCE_GIF_REGLIST, .NREG = 7, .NLOOP = 1
					   , .REGS0 = GS_REGS_RGBA, GS_REGS_PRIM
					   , GS_REGS_XYZ3, GS_REGS_XYZ2, GS_REGS_XYZ2
					   , GS_REGS_XYZ2, GS_REGS_XYZ2 );
		MENU_NEW_PRIM( box, prim );
//		DG_SET_GS_REG( &box->prim, .PRIM = SCE_GS_PRIM_LINESTRIP );
		DG_SET_GS_REG( &box->prim, .PRIM = SCE_GS_PRIM_TRIFAN, .ABE = 1 );
		DG_SET_GS_REG( &box->rgba, 130, 189, 255, 18, 0 );

		box->xy1.x = DG_POS_X( work->x0 );
		box->xy1.y = DG_POS_Y( work->y0 );
		box->xy5.x = DG_POS_X( work->x0 );
		box->xy5.y = DG_POS_Y( work->y0 );
		box->xy2.x = DG_POS_X( work->x1 );
		box->xy2.y = DG_POS_Y( work->y0 );
		box->xy3.x = DG_POS_X( work->x1 );
		box->xy3.y = DG_POS_Y( work->y1 );
		box->xy4.x = DG_POS_X( work->x0 );
		box->xy4.y = DG_POS_Y( work->y1 );

		MENU_ClosePrim();
	}
}

static void Die( Work *work )
{
	DG_CHANL *cp;

	cp = DG_Chanl( 1 );
	cp->flag = 0;
	GM_NoUseCamera( 1 ) ;
}

static int GetResources( Work *work )
{
	int x0, y0, x1, y1;
	int xofs, yofs, width, height;
	DG_CHANL *cp;

	cp = DG_Chanl( 1 );

	if( GCL_GetOption( 'w' ) != NULL ){
		work->x0 = x0 = GCL_GetNextInt();
		work->y0 = y0 = GCL_GetNextInt();
		work->x1 = x1 = GCL_GetNextInt();
		work->y1 = y1 = GCL_GetNextInt();
	} else {
		printf( "subwin:unsetWindow\n" );
		return -1;
	}

	width = x1 - x0;
	height = y1 - y0;
	xofs = ( x0 - DRAW_WIDTH / 2 ) + width / 2;
	yofs = ( y0 - DRAW_HEIGHT / 2 ) + height / 2;

	DG_SetDrawEnv( cp, xofs, yofs, width, height );
#if 1
   BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
	DG_SetCamera2( cp
				   , &( FVECTOR ){ 1526.0F, 2876.0F, 8790.0F }
				   , &( FVECTOR ){ 0.0F, 2508.0F, 3000.0F }
				   , 2.0F );
#endif
	return 0;
}

void *NewSubWindow( int name, int map )
{
	Work *work;

	work = ( Work * )GV_NewActor( GV_ACTOR_PREV2, sizeof( Work ) );

	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );

		work->name = name;
		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
}
