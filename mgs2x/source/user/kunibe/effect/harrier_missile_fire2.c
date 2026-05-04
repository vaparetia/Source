//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_missile_fire2.c
  アムラーム炎
  2001/04/10 Yuuta Kunibe	
  $Id: harrier_missile_fire2.c,v 1.1.1.3 2002/11/19 11:44:42 Yoshizawa1 Exp $
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


#define COL_R	(128)
#define COL_G	(96)
#define COL_B	(32)
#define ALPHA	(60)

#define N_PRIMS	(16)
#define N_VERTS	(1)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)


extern void *NewHarrierMissileSmokeLine2( FMATRIX *mat, char *flag );
extern void *NewHarrierFlare( FVECTOR *pos );


typedef	struct	{

    GV_ACT_EX	actor;
    int 	name;
    int		code;

    DG_PRIM2	*prim;

    FMATRIX	*mat;
    FVECTOR	*vec;

    int 	life;
    float	size;

    char	*flag;
    char	pre_flag;

    int		color1;
    int		color2;
    
    GV_MSG	*msg;

} Work;





static int RecieveMessage( Work *work )
{
    GV_MSG*	msg;
    int 	n_msg;


    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &work->msg ); 

    msg = work->msg;

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case 0:
	    return 1;
	    break;
	}
	msg++;
    }

    return 0;

}


static void Act( Work *work )
{

    int			i;
    int	  		clock;
    FVECTOR  		*pos;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		vectmp;
    float		scale;


    /* デバッグメッセージ送信 */
    /*if ( GV_PadData[1].press & PAD_X ) {

	GV_MSG		msg;

	work->code = 0;

	msg.address = work->name;
	msg.message = &work->code;
	msg.message_len = 1;
	GV_SendMessage( &msg );

	printf("send_message : missile_hit\n");

    }*/



    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }
    clock = work->prim->buffer_clock;

    pos = work->prim->pos[clock];
    uvrgbwh = work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];


#if 0    
    printf("life : %d\n",work->life);
#endif    


    /*if ( work->life == 60 ) work->size = 5000.f + frnd() * 500.f;
    else if ( work->life > 60 ) work->size = work->size * 0.95f + 800.f * 0.05f;
    else work->size = work->size* 0.8f + 600.f * 0.2f;*/

    /* アムラームのフェーズに合わせてバックファイアーのサイズ更新 */
    if ( work->flag != NULL ) {

	if ( *work->flag < 2 ) {
	    work->size = work->size* 0.8f + 600.f * 0.2f;
	}
	else {
	    if ( work->pre_flag == 1 ) {
		work->size = 5000.f + frnd() * 500.f;
		NewHarrierMissileSmokeLine2( work->mat, work->flag );
	    }
	    else {
		work->size = work->size * 0.95f + 800.f * 0.05f;
	    }
	}
	work->pre_flag = *work->flag;

    }
    else {
	work->size = 800.f;
    }


    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	DG_COPY_VEC( &vectmp, (FVECTOR *)work->mat->m[2] );
	scale = 1500.f + 100.f * (float)i;
	_sceVu0ScaleVector( &vectmp, &vectmp, scale );
	_sceVu0SubVector( pos, (FVECTOR *)work->mat->m[3], &vectmp );

	pos->vx += 50.f * frnd();
	pos->vy += 50.f * frnd();
	pos->vz += 50.f * frnd();

	if ( i <= 10 ) {
	    uvrgbwh->w  = 200.f + work->size * (float)i / N_PRIMS;
	    uvrgbwh->h  = 200.f + work->size * (float)i / N_PRIMS;
	}
	else {
	    uvrgbwh->w  = 200.f + work->size * (float)( 20 - i ) / N_PRIMS;
	    uvrgbwh->h  = 200.f + work->size * (float)( 20 - i ) / N_PRIMS;
	}

	pos++;
	uvrgbwh++;

    }


    work->life++;

    /* 終了処理 */
    if ( work->name == 0 ) {
	if ( *work->flag == 3 ) {
	    GV_DestroyActor( work );
	}
    }
    else {
	if ( RecieveMessage( work ) ) {
	    GV_DestroyActor( work );
	}
    }
	    
    
}




static void Die(Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}



/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int		i;
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FVECTOR		vectmp;
	float		scale;

	int		r1,g1,b1,a1;
	int		r2,g2,b2,a2;


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



	r1 = ( ( work->color1 >> 24 ) & 0xff );
	g1 = ( ( work->color1 >> 16 ) & 0xff );
	b1 = ( ( work->color1 >>  8 ) & 0xff );
	a1 = ( work->color1 & 0xff );
	
	r2 = ( ( work->color2 >> 24 ) & 0xff );
	g2 = ( ( work->color2 >> 16 ) & 0xff );
	b2 = ( ( work->color2 >>  8 ) & 0xff );
	a2 = ( work->color2 & 0xff );
	    


	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( &vectmp, (FVECTOR *)work->mat->m[2] );
	    scale = 750.f + 100.f * (float)i;
	    _sceVu0ScaleVector( &vectmp, &vectmp, scale );
	    _sceVu0SubVector( pos, (FVECTOR *)work->mat->m[3], &vectmp );

	    pos->vx += 50.f * frnd();
	    pos->vy += 50.f * frnd();
	    pos->vz += 50.f * frnd();

	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;


	    if ( work->flag != NULL ) {
		uvrgbwh->r  = r1 * ( N_PRIMS - i ) / N_PRIMS + r2 * i / N_PRIMS;
		uvrgbwh->g  = g1 * ( N_PRIMS - i ) / N_PRIMS + g2 * i / N_PRIMS;
		uvrgbwh->b  = b1 * ( N_PRIMS - i ) / N_PRIMS + b2 * i / N_PRIMS;
		uvrgbwh->a  = a1 * ( N_PRIMS - i ) / N_PRIMS + a2 * i / N_PRIMS;
	    }
	    else {
		uvrgbwh->r  = COL_R;
		uvrgbwh->g  = COL_G * ( N_PRIMS - i ) / N_PRIMS;
		uvrgbwh->b  = COL_B;
		uvrgbwh->a  = ALPHA * ( N_PRIMS - i ) / N_PRIMS;
	    }

	    uvrgbwh->w  = 200.f + 250.0f * (float)i / N_PRIMS;
	    uvrgbwh->h  = 200.f + 250.0f * (float)i / N_PRIMS;

	    if ( i <= 10 ) {
		uvrgbwh->w  = 200.f + work->size * (float)i / N_PRIMS;
		uvrgbwh->h  = 200.f + work->size * (float)i / N_PRIMS;
	    }
	    else {
		uvrgbwh->w  = 200.f + work->size * (float)( 20 - i ) / N_PRIMS;
		uvrgbwh->h  = 200.f + work->size * (float)( 20 - i ) / N_PRIMS;
	    }
			
	    pos++;
	    uvrgbwh++ ;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FMATRIX *mat )
{
    
    DG_PRIM2		*prim ;
    DG_TEX		*tex ;


    work->mat = mat;
    work->size = 4000.f + frnd() * 500.f;
    work->life = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, tex );    

    return 0;
}


void *NewHarrierMissileFire2( FMATRIX *mat, char *flag, int color1, int color2 )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->flag = flag;
		work->pre_flag = *work->flag;
		work->color1 = color1;
		work->color2 = color2;
		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}

void *NewDemoHarrierMissileFire2( int name, FMATRIX *mat )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->flag = NULL;
		work->color1 = 0;
		work->color2 = 0;
		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}




