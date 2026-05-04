//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	door_lamp.c
	ドアランプ

//world/door/door_lamp_alp.bmp 

	2001/05/17 S.Okajima
	$Id: door_lamp.c,v 1.1.1.3 2002/11/19 11:47:29 Yoshizawa1 Exp $
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
#define	N_VERTS		(4)
#define	N_PRIMS		(1)

#define	RAISE		(0)

#define	RED_R		(255)
#define	RED_G		(32)
#define	RED_B		(16)
#define	RED_A		(255)

#define	BLUE_R		(16)
#define	BLUE_G		(32)
#define	BLUE_B		(255)
#define	BLUE_A		(255)

#define	YELLOW_R		(128)
#define	YELLOW_G		(128)
#define	YELLOW_B		(16)
#define	YELLOW_A		(255)

#define	THIN		(100.0f)

#define	SCR_POS		(SCRPAD_ADDR)

#define	WIDTH		(180.0f)
#define	HEIGHT		(130.0f)
#define	SHIFT_Z		(1.0f)

typedef	struct	{
	GV_ACT_EX	actor ;
	int			name ;
	int			where ;

	int			crush;
	int			blink;
	CVECTOR		col;

	DG_PRIM2	*prim ;
} Work ;

/* ---------------------------------------------------------------- */
enum {
	REQ_OFF=0,
	REQ_RED,
	REQ_RED_BLINK,
	REQ_BLUE,
	REQ_CRUSH,
	REQ_YELLOW,
	REQ_YELLOW_BLINK,
	REQ_NO
};

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;
	int itemp;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
			work->blink = 0;
			work->col.r = 0;
			work->col.g = 0;
			work->col.b = 0;
			work->col.cd= 0;
			break;
		  case REQ_RED:
			work->blink = 0;
			work->col.r = RED_R;
			work->col.g = RED_G;
			work->col.b = RED_B;
			work->col.cd= RED_A;
			break;
		  case REQ_RED_BLINK:
			itemp = msg->message[1];
			if( itemp >= 0 ){
				work->blink = DIRECT_TICK( msg->message[1] );
			}else{
				work->blink = -1;
			}
			work->col.r = RED_R;
			work->col.g = RED_G;
			work->col.b = RED_B;
			work->col.cd= RED_A;
			break;
		  case REQ_BLUE:
			work->blink = 0;
			work->col.r = BLUE_R;
			work->col.g = BLUE_G;
			work->col.b = BLUE_B;
			work->col.cd= BLUE_A;
			break;
		  case REQ_CRUSH:
			work->crush = 1;
			break;
		  case REQ_YELLOW:
			work->blink = 0;
			work->col.r = YELLOW_R;
			work->col.g = YELLOW_G;
			work->col.b = YELLOW_B;
			work->col.cd= YELLOW_A;
			break;
		  case REQ_YELLOW_BLINK:
			itemp = msg->message[1];
			if( itemp >= 0 ){
				work->blink = DIRECT_TICK( msg->message[1] );
			}else{
				work->blink = -1;
			}
			work->col.r = YELLOW_R;
			work->col.g = YELLOW_G;
			work->col.b = YELLOW_B;
			work->col.cd= YELLOW_A;
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
	int				j;

	CheckMesgParam( work );

	prim = work->prim;
	GM_GroupPrim2( prim, work->where ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	uvrgb = prim->uvrgb[clock];
	if( work->crush ){

		for( j=0; j<4; j++ ){
			uvrgb->r = 0;
			uvrgb->g = 0;
			uvrgb->b = 0;
			uvrgb->a = 0;
			uvrgb++;
		}
	}else{
		if( (work->blink==-1  &&  GV_Time%8<4)
		 || (work->blink!=0   &&  GV_Time%8<4) ){
			for( j=0; j<4; j++ ){
				uvrgb->a = 0;
				uvrgb++;
			}
		}else{
			for( j=0; j<4; j++ ){
				uvrgb->r = work->col.r;
				uvrgb->g = work->col.g;
				uvrgb->b = work->col.b;
				uvrgb->a = work->col.cd;
				uvrgb++;
			}
		}
	}


	if( work->blink > 0 ){
		work->blink--;
		if( work->blink < 0 ){
			work->blink = 0;
		}
	}

}

static void Die( Work *work )
{
	work->prim   = OK_FreePrim2( work->prim   );
}


/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i=0; i<N_PRIMS; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = 0 ;
		uvrgb0->g = uvrgb1->g = 0 ;
		uvrgb0->b = uvrgb1->b = 0 ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = 0 ;
		uvrgb0->g = uvrgb1->g = 0 ;
		uvrgb0->b = uvrgb1->b = 0 ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = 0 ;
		uvrgb0->g = uvrgb1->g = 0 ;
		uvrgb0->b = uvrgb1->b = 0 ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = 0 ;
		uvrgb0->g = uvrgb1->g = 0 ;
		uvrgb0->b = uvrgb1->b = 0 ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;
	}

	return 1;
}


/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex;
	FVECTOR		*p_fv;
	FVECTOR		pos;
	SVECTOR		rot;

	work->crush = 0;
	work->blink = 0;

	if ( GCL_GetOption( 'r' ) != NULL ){
		rot.vx = 0;
		rot.vy = GCL_GetNextInt();
		rot.vz = 0;
	}else{
		printf("door_lamp.c::r\n");
		ASSERT(0)
	}
	if ( GCL_GetOption( 'p' ) != NULL ){
		pos.vx = (float)GCL_GetNextInt();
		pos.vy = (float)GCL_GetNextInt();
		pos.vz = (float)GCL_GetNextInt();
	}else{
		printf("door_lamp.c::p\n");
		ASSERT(0)
	}

	work->col.r = RED_R;
	work->col.g = RED_G;
	work->col.b = RED_B;
	work->col.cd= RED_A;

	tex = DG_GetTexture( 14287963 /*"door_lamp_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		return -1;
	}

	InitPacket2( work, prim, tex );
	prim->raise = RAISE;

	p_fv = SCR_POS;
	p_fv->vx =-WIDTH;
	p_fv->vy = HEIGHT;
	p_fv->vz = SHIFT_Z;
	p_fv++;
	p_fv->vx = WIDTH;
	p_fv->vy = HEIGHT;
	p_fv->vz = SHIFT_Z;
	p_fv++;
	p_fv->vx =-WIDTH;
	p_fv->vy =-HEIGHT;
	p_fv->vz = SHIFT_Z;
	p_fv++;
	p_fv->vx = WIDTH;
	p_fv->vy =-HEIGHT;
	p_fv->vz = SHIFT_Z;
	p_fv++;

	p_fv = SCR_POS;
	DG_SetPos2( &pos, &rot );
	DG_PutVector( p_fv, p_fv, 4 );

	OK_Scr_Mem( prim->pos[ 0 ], SCR_POS, sizeof(FVECTOR), 4 );
	OK_Scr_Mem( prim->pos[ 1 ], SCR_POS, sizeof(FVECTOR), 4 );

	DG_VisiblePrim2( prim );

	return 0 ;
}


void *NewDoorLamp( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name;
		work->where = where;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

