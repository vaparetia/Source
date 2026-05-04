//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	node_lamp.c
	プレイヤーが近づくとノードの画面が起動し、上部のランプが光る

	2001/07/26 S.Okajima
	$Id: node_lamp.c,v 1.1.1.3 2002/11/19 11:47:34 Yoshizawa1 Exp $
*/
//se_set


#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"


/*----------------------------------------------------------------*/
#define	N_PRIMS		(1)
#define	N_VERTS		(4)

#define	N_PRIMS_DOT		(1)
#define	N_VERTS_DOT		(2)

#define	RAISE_BLACK		(500)
#define	RAISE		(1000)

#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	COL_A		(64)

#define	COL_DOT_R_S		(32)
#define	COL_DOT_G_S		(8)
#define	COL_DOT_B_S		(4)
#define	COL_DOT_R		(COL_DOT_R_S*7)
#define	COL_DOT_G		(COL_DOT_G_S*7)
#define	COL_DOT_B		(COL_DOT_B_S*7)
#define	COL_DOT_A		(255)


#define	SCR_POS		(SCRPAD_ADDR)

#define	BLINK_COUNT	(60)
#define	BLINK_COUNT_DOT	(30)

#define	REACT_LENGTH		(1300.0f)

#define	SHIFT_CAM	(30.0f)
#define	SIZE_DOT	(15.0f)

#if 0
static FVECTOR Shift_Black[] = {
	{ -305.0f, 1194.0f, 130.0f, 1.0f},
	{   50.0f, 1194.0f, 130.0f, 1.0f},
	{ -305.0f,  913.0f, 236.0f, 1.0f},
	{   50.0f,  913.0f, 236.0f, 1.0f},
};

static FVECTOR Shift[] = {
	{ -305.0f, 1196.0f, 132.0f, 1.0f},
	{   50.0f, 1196.0f, 132.0f, 1.0f},
	{ -305.0f,  915.0f, 238.0f, 1.0f},
	{   50.0f,  915.0f, 238.0f, 1.0f},
};
#else
static FVECTOR Shift_Black[] = {
	{ -305.0f, 1194.0f, 126.0f, 1.0f},
	{   50.0f, 1194.0f, 126.0f, 1.0f},
	{ -305.0f,  913.0f, 232.0f, 1.0f},
	{   50.0f,  913.0f, 232.0f, 1.0f},
};

static FVECTOR Shift[] = {
	{ -305.0f, 1196.0f, 128.0f, 1.0f},
	{   50.0f, 1196.0f, 128.0f, 1.0f},
	{ -305.0f,  915.0f, 234.0f, 1.0f},
	{   50.0f,  915.0f, 234.0f, 1.0f},
};
#endif

static FVECTOR Dot = { 250.0f, 2200.0f, 35.0f, 1.0f };

typedef	struct	{
	GV_ACT_EX	actor ;
	int			where ;

	int			blink_count;
	int			blink_count_dot;
	int			sd_flag;

	RADAR_CTRL	rctrl ;
	FVECTOR		pos;
	FVECTOR		norm;

	DG_PRIM2	*prim_black ;
	DG_PRIM2	*prim ;
	DG_PRIM2	*prim_dot ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB		*uvrgb ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	int				clock;
	int				alpha;
	float			len;
	FVECTOR			fvtemp;

	prim = work->prim_black;
	GM_GroupPrim2( prim, work->where ) ;
	DG_SwitchBuffPrim2( prim );

	prim = work->prim;
	GM_GroupPrim2( prim, work->where ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;


#if 0
	fvtemp.vx = work->pos.vx;
	fvtemp.vy = GM_PlayerPosition.vy;
	fvtemp.vz = work->pos.vz;
#else
	fvtemp.vx = work->pos.vx;
	fvtemp.vy = work->pos.vy;
	fvtemp.vz = work->pos.vz;
#endif
	len = OK_RectLen( &fvtemp, &GM_PlayerPosition );
	if( len > REACT_LENGTH ){
		work->blink_count--;
		if( work->blink_count < 0 ){
			work->sd_flag = 0;
			work->blink_count = 0;
		}
	}else{	// 近くには来ている
		_sceVu0SubVector( &fvtemp, &fvtemp, &GM_PlayerPosition ) ;
		_sceVu0Normalize( &fvtemp, &fvtemp );
		if( _sceVu0InnerProduct( &fvtemp, &work->norm ) < -0.5f ){
			if( work->sd_flag==0 ){
				work->sd_flag = 1;
#if 1 ///
				GM_SeSetMode( SD_A_NORD_ON1, &work->pos, GM_SEMODE_NORMAL ) ;
#endif
			}
			work->blink_count+=2;
			if( work->blink_count > BLINK_COUNT ) work->blink_count = BLINK_COUNT;
		}
	}

	alpha = COL_A * work->blink_count / BLINK_COUNT;

	uvrgb = prim->uvrgb[clock];
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;

	work->blink_count_dot--;
	if( work->blink_count_dot < 0 ) work->blink_count_dot = BLINK_COUNT_DOT;

	alpha = COL_DOT_A * work->blink_count_dot / BLINK_COUNT_DOT;
	uvrgbwh = work->prim_dot->uvrgb[clock];
	(uvrgbwh++)->a = alpha;
	(uvrgbwh++)->a = alpha;


}

static void Die( Work *work )
{
	work->prim_black = OK_FreePrim2( work->prim_black );
	work->prim       = OK_FreePrim2( work->prim       );
	work->prim_dot   = OK_FreePrim2( work->prim_dot   );
	GM_FreeRadarControl(&work->rctrl);
}


/*----------------------------------------------------------------*/
static int InitPacket2_black( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i;

	DG_ConfigPrim2Tex( prim, tex );

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i=0; i<N_PRIMS; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = 16 ;
		uvrgb0->g = uvrgb1->g = 16 ;
		uvrgb0->b = uvrgb1->b = 16 ;
		uvrgb0->a = uvrgb1->a = 128 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = 16 ;
		uvrgb0->g = uvrgb1->g = 16 ;
		uvrgb0->b = uvrgb1->b = 16 ;
		uvrgb0->a = uvrgb1->a = 128 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = 16 ;
		uvrgb0->g = uvrgb1->g = 16 ;
		uvrgb0->b = uvrgb1->b = 16 ;
		uvrgb0->a = uvrgb1->a = 128 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = 16 ;
		uvrgb0->g = uvrgb1->g = 16 ;
		uvrgb0->b = uvrgb1->b = 16 ;
		uvrgb0->a = uvrgb1->a = 128 ;
		uvrgb0++;
		uvrgb1++;
	}

	return 1;
}


/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i;

	DG_ConfigPrim2Tex( prim, tex );

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i=0; i<N_PRIMS; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;
	}

	return 1;
}


/* ---------------------------------------------------------------- */
static void InitPacketSPRT( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh = SCR_POS;
	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;
	uvrgbwh->h = uvrgbwh->w = SIZE_DOT;
	uvrgbwh->r = COL_DOT_R;
	uvrgbwh->g = COL_DOT_G;
	uvrgbwh->b = COL_DOT_B;
	uvrgbwh->a = 0 ;
	uvrgbwh++;


	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;
	uvrgbwh->h = uvrgbwh->w = SIZE_DOT*3;
	uvrgbwh->r = COL_DOT_R_S;
	uvrgbwh->g = COL_DOT_G_S;
	uvrgbwh->b = COL_DOT_B_S;
	uvrgbwh->a = 0 ;

	OK_Scr_Mem( prim->uvrgb[ 0 ],SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS_DOT * N_VERTS_DOT);
	OK_Scr_Mem( prim->uvrgb[ 1 ],SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS_DOT * N_VERTS_DOT);
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex;
	FVECTOR		fvtemp;
	SVECTOR		rot;

	work->sd_flag = 0;
	work->rctrl.col = 3 ;
	work->blink_count = 0;
	work->blink_count_dot = 0;

	if ( GCL_GetOption( 'p' ) != NULL ){
		work->pos.vx = (float)GCL_GetNextInt();
		work->pos.vy = (float)GCL_GetNextInt();
		work->pos.vz = (float)GCL_GetNextInt();
	}else{
		printf("node_lamp.c::p\n");
		ASSERT(0)
	}

	GM_InitRadarControl(&work->rctrl,&work->pos,(RADAR_VISIBLE|RADAR_NODE) , work->where ) ;
	GM_RadarSetVRange( &work->rctrl, 3000 , -3000 );



	if ( GCL_GetOption( 'r' ) != NULL ){
		rot.vx = 0;
		rot.vy = GCL_GetNextInt();
		rot.vz = 0;
	}else{
		printf("node_lamp.c::r\n");
		ASSERT(0)
	}

	tex = DG_GetTexture( 14252186 /*"node_scr_awake_alp"*/ );
//	tex = DG_GetTexture( 14287963 /*"door_lamp_alp"*/ );
	prim = work->prim_black = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		return -1;
	}
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 2, 1, 0x80 ) );
	InitPacket2_black( work, prim, tex );
	prim->raise = RAISE_BLACK;
	DG_SetPos2( &work->pos, &rot );
	DG_PutVector( Shift_Black, SCR_POS, 4 );
	OK_Scr_Mem( prim->pos[ 0 ], SCR_POS, sizeof(FVECTOR), 4 );
	OK_Scr_Mem( prim->pos[ 1 ], SCR_POS, sizeof(FVECTOR), 4 );
	DG_VisiblePrim2( prim );



	tex = DG_GetTexture( 14252186 /*"node_scr_awake_alp"*/ );
//	tex = DG_GetTexture( 14287963 /*"door_lamp_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		return -1;
	}
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	InitPacket2( work, prim, tex );
	prim->raise = RAISE;
	DG_SetPos2( &work->pos, &rot );
	DG_PutVector( Shift, SCR_POS, 4 );
	OK_Scr_Mem( prim->pos[ 0 ], SCR_POS, sizeof(FVECTOR), 4 );
	OK_Scr_Mem( prim->pos[ 1 ], SCR_POS, sizeof(FVECTOR), 4 );
	DG_VisiblePrim2( prim );


	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim_dot = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS_DOT, N_VERTS_DOT );
	if(prim==NULL){
		return -1;
	}
	InitPacketSPRT( work, prim, tex );
	prim->raise = RAISE;
	DG_PutVector( &Dot, &fvtemp, 1 );
	DG_COPY_VEC( &prim->pos[0][0], &fvtemp );
	DG_COPY_VEC( &prim->pos[0][1], &fvtemp );
	DG_COPY_VEC( &prim->pos[1][0], &fvtemp );
	DG_COPY_VEC( &prim->pos[1][1], &fvtemp );
	DG_VisiblePrim2( prim );

	work->norm.vx = 0.0f;
	work->norm.vy = 0.0f;
	work->norm.vz = 1.0f;
	DG_RotVector( &work->norm, &work->norm, 1 );

	return 0 ;
}





void *NewNodeLamp( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->where = where;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

