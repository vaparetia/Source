//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    demo_dummypoint_blood.c
	デモ用ダミーポイント置き血
    2001/08/14 Yuuta Kunibe	
    $Id: demo_dummypoint_blood.c,v 1.1.1.3 2002/11/19 11:44:36 Yoshizawa1 Exp $
*/

#ifdef PSX2
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
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"


#define	N_PRIMS			(8)
#define	N_VERTS			(4)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define	COL_R			(12)
#define	COL_G			(0)
#define	COL_B			(0)
#define	ALPHA			(32)

#define	BLOOD_PATTERN	(5)

#define	OFFSET_Y		(10.0f)



static int BloodStrCode[] = {
	10972307,	//"chi01_alp"
	12020883,	//"chi02_alp"
	13069459,	//"chi03_alp"
	14118035,	//"chi04_alp"
	15166611,	//"chi05_alp"
};


typedef	struct {

    GV_ACT_EX	actor;
	int 		name;
	int			code[3];
		
    EFTCONTROL  *eft_ctrl;
	
    DG_PRIM2	*prim[BLOOD_PATTERN];	
	int			id[BLOOD_PATTERN];

	int 		type;
	float 		size;

} Work;



static int RecieveMessage( Work *work )
{

    GV_MSG*	msg;
    int 	n_msg;
	int		ret;


	ret = 0;
    if ( work->name == 0 ) {
		return ret;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

		switch ( msg->message[ 0 ] ) {
		case 0:
			work->type = msg->message[1];
			work->size = (float)msg->message[2];
			ret = 1;
			break;
		}
		msg++;
    }

	return ret;

}



static void UpdatePrims( Work *work )
{

	int	i,j,k;
	int	clock;
	DG_PRIM2	*prim;
	FVECTOR		*pos;
	FVECTOR		*pos_pre;

	
	for ( i = 0 ; i < BLOOD_PATTERN; i++ )
   {
      int buffSwitch;
      prim = work->prim[i];
      //AR_PARTICLE_FULL
		buffSwitch = DG_SwitchBuffPrim2( prim );
		
      clock = prim->buffer_clock;
		pos     = prim->pos[clock];
		pos_pre = prim->pos[buffSwitch ^ clock];
		
		for ( j = 0 ; j < N_PRIMS ; j++ )
      {
			for ( k = 0 ; k < N_VERTS ; k++ )
         {
				DG_COPY_VEC( pos, pos_pre );
				pos++;
				pos_pre++;
			}
		}
	}
}


static void MakeDummyPointBlood( Work *work )
{

	int			clock;
	DG_PRIM2	*prim;
	FVECTOR		*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	FMATRIX		mat;
	FVECTOR		pos_tmp[4];
	float		half_size;


	prim  = work->prim[work->type];
	clock = prim->buffer_clock;	
	pos   = &prim->pos[clock][ work->id[work->type]*N_VERTS ];
	uvrgb = &( (DG_PRIM2_UVRGB *)(prim->uvrgb[clock]) )[ work->id[work->type]*N_VERTS ];

	
	DM_EftControlMatrix( work->eft_ctrl, &mat );

	half_size = work->size / 2.0f;
	
	pos_tmp[0].vx = half_size;
	pos_tmp[0].vy = OFFSET_Y;
	pos_tmp[0].vz = half_size;
	pos_tmp[0].vw = 1.0f;

	pos_tmp[1].vx =-half_size;
	pos_tmp[1].vy = OFFSET_Y;
	pos_tmp[1].vz = half_size;
	pos_tmp[1].vw = 1.0f;

	pos_tmp[2].vx = half_size;
	pos_tmp[2].vy = OFFSET_Y;
	pos_tmp[2].vz =-half_size;
	pos_tmp[2].vw = 1.0f;

	pos_tmp[3].vx =-half_size;
	pos_tmp[3].vy = OFFSET_Y;
	pos_tmp[3].vz =-half_size;
	pos_tmp[3].vw = 1.0f;
		

	DG_SetPos( &mat );
	DG_PutVector( pos_tmp, pos, 4 );		


	if ( ++work->id[work->type] >= N_PRIMS ) {
		work->id[work->type] = 0;
	}


}

static void Act( Work *work )
{


#if 0	

	GV_MSG	msg;
	
    if ( GV_PadData[1].press & PAD_L1 ) {
	    work->code[0] = 0;	
	    work->code[1] = 0;
		work->code[2] = 200;

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 3;
	    GV_SendMessage( &msg );
	}
    else if ( GV_PadData[1].press & PAD_L2 ) {
	    work->code[0] = 0;	
	    work->code[1] = 1;
		work->code[2] = 300;

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 3;
	    GV_SendMessage( &msg );
	}
    else if ( GV_PadData[1].press & PAD_R2 ) {
	    work->code[0] = 0;	
	    work->code[1] = 2;
		work->code[2] = 400;

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 3;
	    GV_SendMessage( &msg );
	}
    else if ( GV_PadData[1].press & PAD_X ) {
	    work->code[0] = 0;	
	    work->code[1] = 3;
		work->code[2] = 500;

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 3;
	    GV_SendMessage( &msg );
	}
    else if ( GV_PadData[1].press & PAD_Y ) {
	    work->code[0] = 0;	
	    work->code[1] = 4;
		work->code[2] = 600;

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 3;
	    GV_SendMessage( &msg );
	}
	
#endif	
	
	UpdatePrims( work );

	if ( RecieveMessage( work ) ) {
		MakeDummyPointBlood( work );
	}


}


static void Die(Work *work)
{
	int i;
	for ( i = 0 ; i < BLOOD_PATTERN ; i++ ) {
		if ( work->prim[i] ) GM_FreePrim2( work->prim[i] );
	}
}



static DG_PRIM2 *InitPacket( DG_PRIM2 *prim, int tex_code )
{

	int				i;
	DG_TEX			*tex;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;


    prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );

	prim->raise = -10000;	// 高部さんのデモ煙より優先下げる

	tex = DG_GetTexture( tex_code );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


	pos   = SCR_POS;
	uvrgb = SCR_UVS;
	
	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = COL_R;
	    uvrgb->g = COL_G;
	    uvrgb->b = COL_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = COL_R;
	    uvrgb->g = COL_G;
	    uvrgb->b = COL_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = COL_R;
	    uvrgb->g = COL_G;
	    uvrgb->b = COL_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = COL_R;
	    uvrgb->g = COL_G;
	    uvrgb->b = COL_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
 
	return prim;

}





static int GetResources( Work *work )
{

	int i;
	
	for ( i = 0 ; i < BLOOD_PATTERN ; i++ ) {
		work->id[i] = 0;
		work->prim[i] = InitPacket( work->prim[i], BloodStrCode[i] );
	}

	return 0;

}



void *NewDemoDummyPointBlood( int name, int con_name )
{

	Work	*work;

	if ( GM_Configuration & GM_CONFIG_BLOOD_OFF ) {
	    return NULL;
	}
	
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->name = name;
		work->eft_ctrl = DM_GetEftControl( con_name );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}
