//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	elevator_lamp.c
	エレベータランプ
	2001/05/17 S.Okajima
	$Id: elevator_lamp.c,v 1.1.1.3 2002/11/19 11:47:30 Yoshizawa1 Exp $
*/

#ifdef PSX2	///
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
#define	N_NUM			(3)
#define	N_PRIMS			(1)

#define	RAISE_DISP		(0)
#define	RAISE_UNDER		(-2000)
#define	RAISE_NUM		(1000)

#define	COLOR_R			(128)
#define	COLOR_G			(128)
#define	COLOR_B			(128)
#define	COLOR_A			(128)

#define	OFF_R			(32)
#define	OFF_G			(32)
#define	OFF_B			(32)
#define	OFF_A			(255)

#define	ON_R			(255)
#define	ON_G			(255)
#define	ON_B			(128)
#define	ON_A			(255)


typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;

	int			kind ;
	int			floor_num ;
	int			floor_select ;
	int			max_lamp;

	DG_PRIM2		*prim_disp ;
	DG_PRIM2		*prim_under ;
	DG_PRIM2		*prim_num ;
} Work ;

/* ---------------------------------------------------------------- */
enum {
	FLOOR_1F=0,
	FLOOR_B1,
	FLOOR_B2
};

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case FLOOR_1F:
		  case FLOOR_B1:
		  case FLOOR_B2:
			work->floor_select = num;
			break;
		  default:
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int				clock;
	int				i,j;

	CheckMesgParam( work );

	if(work->floor_select < 0){
		work->floor_select = 0;
	}else if(work->floor_select >= work->max_lamp ){
		work->floor_select = work->max_lamp-1;
	}


/*
	prim = work->prim_disp;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );

	prim = work->prim_under;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
*/


	prim = work->prim_num;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;


	uvrgb = prim->uvrgb[clock];
	for( i=0; i<work->max_lamp; i++ ){
		if( i!=work->floor_select ){
			for( j=0; j<4; j++ ){
				uvrgb->r = OFF_R;
				uvrgb->g = OFF_G;
				uvrgb->b = OFF_B;
				uvrgb->a = OFF_A;
				uvrgb++;
			}
		}else{
			for( j=0; j<4; j++ ){
				uvrgb->r = ON_R;
				uvrgb->g = ON_G;
				uvrgb->b = ON_B;
				uvrgb->a = ON_A;
				uvrgb++;
			}
		}
	}



}

static void Die( Work *work )
{
	work->prim_disp  = OK_FreePrim2( work->prim_disp  );
	work->prim_under = OK_FreePrim2( work->prim_under );
	work->prim_num   = OK_FreePrim2( work->prim_num   );
}


/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int num, CVECTOR col, int mode )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i;

	DG_ConfigPrim2Tex( prim, tex );
	if( mode==0 ){
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else{
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i=0; i<num; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = col.r ;
		uvrgb0->g = uvrgb1->g = col.g ;
		uvrgb0->b = uvrgb1->b = col.b ;
		uvrgb0->a = uvrgb1->a = col.cd;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = col.r ;
		uvrgb0->g = uvrgb1->g = col.g ;
		uvrgb0->b = uvrgb1->b = col.b ;
		uvrgb0->a = uvrgb1->a = col.cd;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = col.r ;
		uvrgb0->g = uvrgb1->g = col.g ;
		uvrgb0->b = uvrgb1->b = col.b ;
		uvrgb0->a = uvrgb1->a = col.cd;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = col.r ;
		uvrgb0->g = uvrgb1->g = col.g ;
		uvrgb0->b = uvrgb1->b = col.b ;
		uvrgb0->a = uvrgb1->a = col.cd;
		uvrgb0++;
		uvrgb1++;
	}

	return 1;
}

#define SHIFT_X (   0.0f)
#define SHIFT_Y (-420.0f)
#define SHIFT_Z (   1.0f)


static ALIGN16_PRE float ALIGN16_POST Data[] = {
	    0.0f,          0.0f,           1.0f,        80.0f,  80.0f,
	 SHIFT_X,       SHIFT_Y,        SHIFT_Z,       100.0f, 170.0f,
	 SHIFT_X-47.0f, SHIFT_Y+126.0f, SHIFT_Z+1.0f,   30.0f,  25.0f,
	 SHIFT_X-47.0f, SHIFT_Y+ 43.0f, SHIFT_Z+1.0f,   30.0f,  25.0f,
	 SHIFT_X-47.0f, SHIFT_Y- 40.0f, SHIFT_Z+1.0f,   30.0f,  25.0f,
};


/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim_disp ;
	DG_PRIM2	*prim_under ;
	DG_PRIM2	*prim_num ;
	DG_TEX		*tex=NULL;
	FVECTOR		*p_fv_data;
	FVECTOR		*p_fv;
	FVECTOR		*p_fv_sub;
	FVECTOR		pos;
	FVECTOR		calc_pos[ (N_NUM + 2) * 4 ];
	SVECTOR		rot;
	CVECTOR		col;
	CVECTOR		panel_col;
	int			i;

	if ( GCL_GetOption( 'c' ) != NULL ){
		panel_col.r = GCL_GetNextInt();
		panel_col.g = GCL_GetNextInt();
		panel_col.b = GCL_GetNextInt();
	}else{
		panel_col.r = 48;
		panel_col.g = 48;
		panel_col.b = 48;
	}

	if ( GCL_GetOption( 'k' ) != NULL ){
		work->kind = GCL_GetNextInt();
		if( work->kind!=0 && work->kind!=1 ){
			printf("elevator_lamp.c::K:0\n");
			ASSERT(0)
		}
	}else{
		printf("elevator_lamp.c::K:1\n");
		ASSERT(0)
	}

	if ( GCL_GetOption( 'i' ) != NULL ){
		work->floor_num = GCL_GetNextInt();
		if(work->kind==0){
			if( work->floor_num != FLOOR_1F
			 && work->floor_num != FLOOR_B1
			 && work->floor_num != FLOOR_B2
			  ){
				printf("elevator_lamp.c::i:0\n");
				ASSERT(0)
			}
			work->max_lamp = 3;
		}else{
			if( work->floor_num != FLOOR_1F
			 && work->floor_num != FLOOR_B1
			  ){
				printf("elevator_lamp.c::i:1\n");
				ASSERT(0)
			}
			work->max_lamp = 2;
		}
	}else{
		printf("elevator_lamp.c::i:2\n");
		ASSERT(0)
	}

	work->floor_select = work->floor_num;


	// パネル：現在の回数表示
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	switch( work->floor_num ){
	  case FLOOR_1F:
		tex = DG_GetTexture( 12924821 /*"elvt_pane_1_alp"*/ );
		break;
	  case FLOOR_B1:
		tex = DG_GetTexture( 12961479 /*"elvt_pane_B1_alp"*/ );
		break;
	  case FLOOR_B2:
		tex = DG_GetTexture( 14010055 /*"elvt_pane_B2_alp"*/ );
		break;
	}
	prim_disp = work->prim_disp = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 4 );
	if(prim_disp==NULL){
		return -1;
	}
	col.r = COLOR_R;
	col.g = COLOR_G;
	col.b = COLOR_B;
	col.cd= COLOR_A;
	InitPacket2( work, prim_disp, tex, 1, col, 0 );
	prim_disp->raise = RAISE_DISP;

	// パネル：階数表示下地
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	switch( work->kind ){
	  case 0:
	  default:
		tex = DG_GetTexture( 5064400 /*"elvt_nwall3pb2B_alp"*/ );
		break;
	  case 1:
		tex = DG_GetTexture( 5064398 /*"elvt_nwall3pb1B_alp"*/ );
		break;
	}
	prim_under = work->prim_under = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 4 );
	if(prim_under==NULL){
		return -1;
	}
	col.r = panel_col.r;
	col.g = panel_col.g;
	col.b = panel_col.b;
	col.cd= 128;
	InitPacket2( work, prim_under, tex, 1, col, 1 );
	prim_under->raise = RAISE_UNDER;

	// パネル：階数表示の上に重ねる
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 298333 /*"elvt_pane_lig_add_alp"*/ );
	prim_num = work->prim_num = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, work->max_lamp*4 );
	if(prim_num==NULL){
		return -1;
	}
	col.r = 0;
	col.g = 0;
	col.b = 0;
	col.cd= 0;
	InitPacket2( work, prim_num, tex, N_PRIMS*work->max_lamp, col, 0 );
	prim_num->raise = RAISE_NUM;


	if ( GCL_GetOption( 'r' ) != NULL ){
		rot.vx = 0;
		rot.vy = GCL_GetNextInt();
		rot.vz = 0;
	}else{
		printf("elevator_lamp.c::D\n");
		ASSERT(0)
	}
	if ( GCL_GetOption( 'p' ) != NULL ){
		pos.vx = (float)GCL_GetNextInt();
		pos.vy = (float)GCL_GetNextInt();
		pos.vz = (float)GCL_GetNextInt();
	}else{
		printf("elevator_lamp.c::P\n");
		ASSERT(0)
	}

	p_fv = calc_pos;
	for( i=0; i<N_NUM+2; i++ ){
		p_fv->vx = Data[i*5 + 0] - Data[i*5 + 3];
		p_fv->vy = Data[i*5 + 1] + Data[i*5 + 4];
		p_fv->vz = Data[i*5 + 2];
		p_fv++;
		p_fv->vx = Data[i*5 + 0] + Data[i*5 + 3];
		p_fv->vy = Data[i*5 + 1] + Data[i*5 + 4];
		p_fv->vz = Data[i*5 + 2];
		p_fv++;
		p_fv->vx = Data[i*5 + 0] - Data[i*5 + 3];
		p_fv->vy = Data[i*5 + 1] - Data[i*5 + 4];
		p_fv->vz = Data[i*5 + 2];
		p_fv++;
		p_fv->vx = Data[i*5 + 0] + Data[i*5 + 3];
		p_fv->vy = Data[i*5 + 1] - Data[i*5 + 4];
		p_fv->vz = Data[i*5 + 2];
		p_fv++;
	}

	DG_SetPos2( &pos, &rot );
	DG_PutVector( calc_pos, calc_pos, (N_NUM+2)*4 );


	p_fv_data = calc_pos;
	///////////////////
	p_fv      = work->prim_disp->pos[0];
	p_fv_sub  = work->prim_disp->pos[1];
	for( i=0; i<4; i++ ){
		DG_COPY_VEC( p_fv,     p_fv_data );
		DG_COPY_VEC( p_fv_sub, p_fv_data );
		p_fv++;
		p_fv_sub++;
		p_fv_data++;
	}

	///////////////////
	p_fv      = work->prim_under->pos[0];
	p_fv_sub  = work->prim_under->pos[1];
	for( i=0; i<4; i++ ){
		DG_COPY_VEC( p_fv,     p_fv_data );
		DG_COPY_VEC( p_fv_sub, p_fv_data );
		p_fv++;
		p_fv_sub++;
		p_fv_data++;
	}

	///////////////////
	p_fv     = work->prim_num->pos[0];
	p_fv_sub = work->prim_num->pos[1];
	for( i=0; i<work->max_lamp*4; i++ ){
		DG_COPY_VEC( p_fv,     p_fv_data );
		DG_COPY_VEC( p_fv_sub, p_fv_data );
		p_fv++;
		p_fv_sub++;
		p_fv_data++;
	}



	DG_VisiblePrim2( prim_disp );
	DG_VisiblePrim2( prim_under );
	DG_VisiblePrim2( prim_num );

	return 0 ;
}

void *NewElavatorLamp( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

