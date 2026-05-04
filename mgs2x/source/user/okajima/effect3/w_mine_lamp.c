//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	w_mine_lamp.c
	水中機雷の点滅
	2001/05/15 S.Okajima
	$Id: w_mine_lamp.c,v 1.1.1.3 2002/11/19 11:47:39 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2 ///
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"../etc/ok_util.h"
#include	"camera.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(6)
#define N_PRIMS		(1)

#define	COL_R		(255)
#define	COL_G		(28)
#define	COL_B		(0)
#define	COL_A		(255)

#define	BACK_COL_R	(128)
#define	BACK_COL_G	(64)
#define	BACK_COL_B	(32)
#define	BACK_ALPHA	(40)

#define	SIZE		(50)
#define	BACK_SIZE	(1000)

#define	CYCLE_TIME	(120)

#define	SHIFT_BACK	(100.0f)

typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		*world;

	int			count0;
	int			count1;
	DG_PRIM2	*prim_back ;
	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static FVECTOR Data[N_VERTS] = {
	{ 200.0f, 240.0f, 120.0f, 0.0f },
	{-200.0f,-240.0f,-120.0f, 0.0f },
	{   0.0f, 240.0f,-240.0f, 0.0f },
	{-200.0f, 240.0f, 120.0f, 0.0f },
	{ 200.0f,-240.0f,-120.0f, 0.0f },
	{   0.0f,-240.0f, 240.0f, 0.0f }
};




/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR				fvtemp;
	DG_PRIM2			*prim;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int					clock;
	int					i;
	int					alpha0;
	int					alpha1;
	float				ratio;

	prim = work->prim;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );

	clock = prim->buffer_clock;

	if( work->count0++ >= CYCLE_TIME ) work->count0 = 0;
	if( work->count1++ >= CYCLE_TIME ) work->count1 = 0;

	ratio = (float)(CYCLE_TIME-work->count0) / (float)CYCLE_TIME;
	ratio*= ratio*ratio;
	alpha0 = (int)((float)COL_A * ratio);

	ratio = (float)(CYCLE_TIME-work->count1) / (float)CYCLE_TIME;
	ratio*= ratio*ratio;
	alpha1 = (int)((float)COL_A * ratio);

	uvrgbwh = prim->uvrgb[clock];
	for ( i=0; i<N_VERTS/2; i++ ){
		uvrgbwh->a = (int)alpha0;
		uvrgbwh++ ;
	}
	for (    ; i<N_VERTS  ; i++ ){
		uvrgbwh->a = (int)alpha1;
		uvrgbwh++ ;
	}



	//--------------------------------

	prim = work->prim_back;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );

	clock = prim->buffer_clock;
	uvrgbwh = prim->uvrgb[clock];

	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = SHIFT_BACK;
	DG_SetPos( work->world );

	_sceVu0SubVector( &fvtemp, (FVECTOR *)work->world->m[3], (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, SHIFT_BACK );
	_sceVu0AddVector( prim->pos[clock], &fvtemp, (FVECTOR *)work->world->m[3] ) ;

}

static void Die( Work *work )
{
	work->prim      = OK_FreePrim2( work->prim );
	work->prim_back = OK_FreePrim2( work->prim_back );
}


static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*data;
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		k;
	float	angle;

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	angle = 0.0f;

	data    = Data;
	pos     = SCR_POS;
	uvrgbwh = SCR_TMP;
	for ( k=0; k<N_VERTS; k++ ){
		DG_COPY_VEC( pos, data );
		uvrgbwh->w  = 0;
		uvrgbwh->h  = 0;
		uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;

		uvrgbwh->w  = uvrgbwh->h  = SIZE ;

		uvrgbwh->r  = COL_R ;
		uvrgbwh->g  = COL_G ;
		uvrgbwh->b  = COL_B ;
		uvrgbwh->a  = 0 ;

		pos++;
		uvrgbwh++ ;
		data++;
	}
	OK_Scr_Mem( prim->pos[0],   SCR_POS, sizeof(FVECTOR),          N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( prim->pos[1],   SCR_POS, sizeof(FVECTOR),          N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[0], SCR_TMP, sizeof(DG_PRIM2_UVRGBWH), N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[1], SCR_TMP, sizeof(DG_PRIM2_UVRGBWH), N_VERTS*N_PRIMS ) ;

	return 1;
}

static int InitPacket_Back( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	uvrgbwh = SCR_TMP;
	uvrgbwh->w  = 0;
	uvrgbwh->h  = 0;
	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;

	uvrgbwh->w = uvrgbwh->h = BACK_SIZE;

	uvrgbwh->r  = BACK_COL_R ;
	uvrgbwh->g  = BACK_COL_G ;
	uvrgbwh->b  = BACK_COL_B ;
	uvrgbwh->a  = BACK_ALPHA ;


	OK_Scr_Mem( prim->uvrgb[0], SCR_TMP, sizeof(DG_PRIM2_UVRGBWH), 1 ) ;
	OK_Scr_Mem( prim->uvrgb[1], SCR_TMP, sizeof(DG_PRIM2_UVRGBWH), 1 ) ;

	return 1;
}

static int GetResources( Work *work, FMATRIX *world )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->count0 = 0;
	work->count1 = CYCLE_TIME/2;
	work->world = world;

//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 2998348 /*"xlit04a_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim );
	prim->root = world;
	if( !InitPacket( work, prim, tex ) ) return -1;


	tex = DG_GetTexture( 4046924 /*"xlit04b_alp"*/ );
	prim = work->prim_back = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ALPHA|DG_PRIM2_FOG, 1, 1 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim );
	if( !InitPacket_Back( work, prim, tex ) ) return -1;

	return 0 ;
}


void *NewWaterMineLamp( FMATRIX *world )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, world ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
