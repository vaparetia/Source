//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_wing_smoke.c
  ハリアー旋回筋雲
  2001/04/03 Yuuta Kunibe
	
  $Id: harrier_missile_smoke2.c,v 1.1.1.3 2002/11/19 11:44:42 Yoshizawa1 Exp $
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


#define N_PRIMS		(4)
#define N_VERTS 	(64)
#define CENTER_NUM 	(N_PRIMS * N_VERTS/2)

#define COLOR		(128)
#define ALPHA		(32.0f)

#define WIDTH	(250.f)

#define	SPEED		(1000.0f)
#define	SMOKE_TEX	( GV_StrCode( "smoke_lp1_alp" ) )

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define	OFFSET		(1800.0f)

#define LIFE		(200)


typedef struct {
    FMATRIX	mat;
    float	speed;
    float	width;
} SMOKE_PARAM;


typedef	struct	{

    GV_ACT_EX	actor;

    
    DG_PRIM2	*prim1;
    DG_PRIM2	*prim2;

    FMATRIX	*mat;
    FMATRIX	hit_mat;
    SMOKE_PARAM	param[CENTER_NUM];

    SVECTOR	rot;

    float	offset;
    float	speed;
    float	rot_param;
    int		speed_flag;
    
    float	*p_width;
    float	width;
    
    float	alpha_param;
    int		alpha_off_flag;
    int		rot_y_max;

    int		loop_flag;
    
    int 	id;
    int		name;
    char		*flag;
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

    int 		i,j,n;
    int	  		clock1;
    int	  		clock2;
    FVECTOR		*pos1;
    FVECTOR		*pos2;
    DG_PRIM2_UVRGB	*uvrgb1;
    DG_PRIM2_UVRGB	*uvrgb2;

    FVECTOR		vectmp;
    FVECTOR		vec1;
    FVECTOR		vec2;
    int 		alpha;

    SMOKE_PARAM		*param;



    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim1 );
    DG_SwitchBuffPrim2( work->prim2 );
    clock1 = work->prim1->buffer_clock;
    clock2 = work->prim2->buffer_clock;



    /* 着弾処理 */
    if ( work->alpha_off_flag == 0 ) {

	/* ゲーム中着弾処理 */
	if ( work->name == 0 ) {
	    if ( *work->flag == 3 && work->alpha_off_flag == 0 ) {
		work->alpha_off_flag = 1;
		DG_COPY_MAT( &work->hit_mat, work->mat );
		work->mat = &work->hit_mat;
	    }
	}
	/* デモ呼び出し用着弾 */
	else {	    
	    if ( RecieveMessage( work ) && work->alpha_off_flag == 0 ) {	/* メッセージチェック */
		work->alpha_off_flag = 1;
		DG_COPY_MAT( &work->hit_mat, work->mat );
		work->mat = &work->hit_mat;
	    }
	}

    }




    /* 着弾後のフェードアウト処理 */
    if ( work->alpha_off_flag ) {
	work->alpha_param -= 0.015f;
	if ( work->alpha_param < 0.0f ) {
	    work->alpha_param = 0.0f;
	}
    }




    /* ポリゴン繋ぎ処理 */
    param = work->param;
    if ( work->id == 0 ) {
	DG_COPY_MAT( &param[work->id].mat, &param[CENTER_NUM-1].mat );
	param[work->id].speed = param[CENTER_NUM-1].speed;

	work->id++;
	
    }
    else if ( !(work->id%32) ) {
	DG_COPY_MAT( &param[work->id].mat, &param[work->id-1].mat );
	param[work->id].speed = param[CENTER_NUM-1].speed;

	work->id++;
    }

    if ( work->alpha_off_flag ) {	

	    work->rot.vx += (short)( (float)(2048/15) * work->rot_param );
	    work->rot.vy += work->rot_y_max/30 * work->rot_param;
	    param[work->id].speed = work->speed;

	    if ( work->rot.vx > 1024 ) {
		work->speed_flag = 1;
	    }
	    
	    if ( work->speed_flag == 0 ) {
		if ( work->rot_param < 1.0f ) {
		    work->rot_param += 0.20f;
		}

		else {
		    work->rot_param = 1.0f;
		}
	    }
	    else {
		work->rot_param *= 0.95f;
	    }

	    work->speed *= 0.9f;
	    //work->offset *= 1.01f;
	    work->width *= 1.02f;
	
    }
    else {

	work->rot.vz += 4096/20;
	if ( work->rot.vz > 4096 ) {
	    work->rot.vz -= 4096;
	}
	param[work->id].speed = work->speed;

    }
    
    DG_SetPos( work->mat );
    DG_RotatePos( &work->rot );
    DG_GetPos( &param[work->id].mat );
    //_sceVu0ScaleVector( &vectmp, (FVECTOR *)param[work->id].mat.m[2], OFFSET );
    _sceVu0ScaleVector( &vectmp, (FVECTOR *)param[work->id].mat.m[2], work->offset );
    _sceVu0SubVector( (FVECTOR *)param[work->id].mat.m[3], (FVECTOR *)param[work->id].mat.m[3], &vectmp );




#if 0    
    printf("id : %d\n",work->id);
#endif    




    pos1   = work->prim1->pos[clock1];
    pos2   = work->prim2->pos[clock2];
    uvrgb1 = work->prim1->uvrgb[clock1];
    uvrgb2 = work->prim2->uvrgb[clock2];
    param  = work->param;

    if ( work->alpha_param > 0.0f ) {			

	for ( i = 0 ; i < N_PRIMS ; i++ ) {
	    
	    for ( j = 0 ; j < N_VERTS/2 ; j++ ) {

		n = i*32+j;
	    
		if ( i != 0 && j == 0 ) {

		    DG_COPY_VEC( pos1, (pos1-2) );
		    pos1++;
		    DG_COPY_VEC( pos1, (pos1-2) );
		    pos1++;

		    DG_COPY_VEC( pos2, (pos2-2) );
		    pos2++;
		    DG_COPY_VEC( pos2, (pos2-2) );
		    pos2++;


		}
		else {
		    
		    vectmp.vx = work->width;
		    vectmp.vy = 0.0f;
		    vectmp.vz = 0.0f;
		    vectmp.vw = 1.0f;

		    DG_SetPos( &param->mat );
		    DG_RotVector( &vectmp, &vec1, 1 );

		    _sceVu0AddVector( pos1, (FVECTOR *)param->mat.m[3], &vec1 );
		    pos1++;

		    _sceVu0SubVector( pos1, (FVECTOR *)param->mat.m[3], &vec1 );
		    pos1++;


		    vectmp.vx = 0.0f;
		    vectmp.vy = work->width;
		    vectmp.vz = 0.0f;
		    vectmp.vw = 1.0f;

		    DG_SetPos( &param->mat );
		    DG_RotVector( &vectmp, &vec2, 1 );

		    _sceVu0AddVector( pos2, (FVECTOR *)param->mat.m[3], &vec2 );
		    pos2++;

		    _sceVu0SubVector( pos2, (FVECTOR *)param->mat.m[3], &vec2 );
		    pos2++;

		}

		_sceVu0ScaleVector( &vectmp, (FVECTOR *)param->mat.m[2], param->speed );
		_sceVu0SubVector( (FVECTOR *)param->mat.m[3], (FVECTOR *)param->mat.m[3], &vectmp );
		param->speed *= 0.90f+frnd()*0.02f;
		param++;



		if ( !work->loop_flag ) {
		    if ( work->id != 0 && n <= work->id ) {
			alpha = (int)( ALPHA * (float)n / (float)work->id * work->alpha_param );
		    }
		    else {
			alpha = 0;
		    }
		}
		else {		
		    alpha = work->id - n;
		    if ( alpha < 0 ) alpha += CENTER_NUM;
		    alpha = CENTER_NUM - alpha;
		    alpha = (int)( (float)alpha / (float)CENTER_NUM * ALPHA * work->alpha_param );
		}
	    
		if ( work->id == CENTER_NUM-2 ) {
		    if ( n == CENTER_NUM-1 || n == 0 || n == 1 ) {
			uvrgb1->f = 0x8fff;
			uvrgb1->a = alpha;
			uvrgb1++;
			uvrgb1->f = 0x8fff;
			uvrgb1->a = alpha;
			uvrgb1++;

			uvrgb2->f = 0x8fff;
			uvrgb2->a = alpha;
			uvrgb2++;
			uvrgb2->f = 0x8fff;
			uvrgb2->a = alpha;
			uvrgb2++;
		    }
		    else {
			uvrgb1->f = 0x0fff;
			uvrgb1->a = alpha;
			uvrgb1++;
			uvrgb1->f = 0x0fff;
			uvrgb1->a = alpha;
			uvrgb1++;

			uvrgb2->f = 0x0fff;
			uvrgb2->a = alpha;
			uvrgb2++;
			uvrgb2->f = 0x0fff;
			uvrgb2->a = alpha;
			uvrgb2++;
		    }

		}
		else if ( work->id == CENTER_NUM-1 ) {
		    if ( n == 0 || n == 1 ) {
			uvrgb1->f = 0x8fff;
			uvrgb1->a = alpha;
			uvrgb1++;
			uvrgb1->f = 0x8fff;
			uvrgb1->a = alpha;
			uvrgb1++;

			uvrgb2->f = 0x8fff;
			uvrgb2->a = alpha;
			uvrgb2++;
			uvrgb2->f = 0x8fff;
			uvrgb2->a = alpha;
			uvrgb2++;
		    }
		    else {
			uvrgb1->f = 0x0fff;
			uvrgb1->a = alpha;
			uvrgb1++;
			uvrgb1->f = 0x0fff;
			uvrgb1->a = alpha;
			uvrgb1++;

			uvrgb2->f = 0x0fff;
			uvrgb2->a = alpha;
			uvrgb2++;
			uvrgb2->f = 0x0fff;
			uvrgb2->a = alpha;
			uvrgb2++;
		    }
		}
		else if ( work->id%32 == 31 ) {
		    if ( n == work->id+1 || n == work->id+2 ) {
			uvrgb1->f = 0x8fff;
			uvrgb1->a = alpha;
			uvrgb1++;
			uvrgb1->f = 0x8fff;
			uvrgb1->a = alpha;
			uvrgb1++;

			uvrgb2->f = 0x8fff;
			uvrgb2->a = alpha;
			uvrgb2++;
			uvrgb2->f = 0x8fff;
			uvrgb2->a = alpha;
			uvrgb2++;
		    }
		    else {
			uvrgb1->f = 0x0fff;
			uvrgb1->a = alpha;
			uvrgb1++;
			uvrgb1->f = 0x0fff;
			uvrgb1->a = alpha;
			uvrgb1++;

			uvrgb2->f = 0x0fff;
			uvrgb2->a = alpha;
			uvrgb2++;
			uvrgb2->f = 0x0fff;
			uvrgb2->a = alpha;
			uvrgb2++;
		    }
		}
		else {		    		    
		    if ( n == work->id+1 ) {
			uvrgb1->f = 0x8fff;
			uvrgb1->a = alpha;
			uvrgb1++;
			uvrgb1->f = 0x8fff;
			uvrgb1->a = alpha;
			uvrgb1++;

			uvrgb2->f = 0x8fff;
			uvrgb2->a = alpha;
			uvrgb2++;
			uvrgb2->f = 0x8fff;
			uvrgb2->a = alpha;
			uvrgb2++;
		    }
		    else {
			uvrgb1->f = 0x0fff;
			uvrgb1->a = alpha;
			uvrgb1++;
			uvrgb1->f = 0x0fff;
			uvrgb1->a = alpha;
			uvrgb1++;

			uvrgb2->f = 0x0fff;
			uvrgb2->a = alpha;
			uvrgb2++;
			uvrgb2->f = 0x0fff;
			uvrgb2->a = alpha;
			uvrgb2++;
		    }
		}
		
	    }

	}

    }

    if ( ++work->id >= CENTER_NUM ) {
	work->loop_flag = 1;
	work->id = 0;
    }


    if ( work->alpha_param <= 0.0f ) {
	GV_DestroyActor( work );
    }
    
}


static void Die(Work *work )
{
    /* メモリ解放 */
    if ( work->prim1 ) work->prim1 = OK_FreePrim2( work->prim1 );
    if ( work->prim2 ) work->prim2 = OK_FreePrim2( work->prim2 );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work )
{

    DG_PRIM2		*prim1;
    DG_PRIM2		*prim2;
    DG_TEX		*tex;
    
    int			i,j;
    FVECTOR		*pos;	
    DG_PRIM2_UVRGB	*uvrgb;
    SMOKE_PARAM		*param;




    tex = DG_GetTexture( SMOKE_TEX );


    prim1 = work->prim1 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_TEX|DG_PRIM2_SHADE, N_PRIMS, N_VERTS );
    if ( prim1 == NULL ) {
	return 0;
    }
    

    prim1->raise = 0;

    DG_ConfigPrim2Tex( prim1, tex );
    DG_SetPrim2Alpha( prim1, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


    prim2 = work->prim2 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_TEX|DG_PRIM2_SHADE, N_PRIMS, N_VERTS );
    if ( prim2 == NULL ) {
	return 0;
    }

    prim2->raise = 0;

    DG_ConfigPrim2Tex( prim2, tex );
    DG_SetPrim2Alpha( prim2, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



    pos   = SCR_POS;
    uvrgb = SCR_UVS;
    param = work->param;

    for ( i = 0 ; i < N_PRIMS; i++ ) {

	pos   = SCR_POS;
	uvrgb = SCR_UVS;

	for ( j = 0 ; j < N_VERTS ; j++ ) {

	    DG_COPY_VEC( pos, (FVECTOR *)work->mat->m[3] );
	    pos++;

	    if ( j%2 ) {
		uvrgb->u = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    }
	    else {
		uvrgb->u = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    }

	    uvrgb->v = FTOI12( 1.0f / (float)CENTER_NUM * (float)(i*32+j/2) * tex->v_scale + tex->v_offset );

	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->b = COLOR;
	    uvrgb->r = COLOR;
	    uvrgb->g = COLOR;
	    uvrgb->a = 0;
	    
	    uvrgb++;
	    
	    if ( j % 2 ) {
		DG_COPY_MAT( &param->mat, work->mat );
		param++;
	    }

	}

	OK_Scr_Mem( &prim1->pos[ 0 ][i*N_VERTS], SCR_POS, sizeof(FVECTOR), N_VERTS );
	OK_Scr_Mem( &prim1->pos[ 1 ][i*N_VERTS], SCR_POS, sizeof(FVECTOR), N_VERTS );
	OK_Scr_Mem( &((DG_PRIM2_UVRGB*)(prim1->uvrgb[ 0 ]))[i*N_VERTS], SCR_UVS,
		    sizeof(DG_PRIM2_UVRGB), N_VERTS );
	OK_Scr_Mem( &((DG_PRIM2_UVRGB*)(prim1->uvrgb[ 1 ]))[i*N_VERTS], SCR_UVS,
		    sizeof(DG_PRIM2_UVRGB), N_VERTS );

	OK_Scr_Mem( &prim2->pos[ 0 ][i*N_VERTS], SCR_POS, sizeof(FVECTOR), N_VERTS );
	OK_Scr_Mem( &prim2->pos[ 1 ][i*N_VERTS], SCR_POS, sizeof(FVECTOR), N_VERTS );
	OK_Scr_Mem( &((DG_PRIM2_UVRGB*)(prim2->uvrgb[ 0 ]))[i*N_VERTS], SCR_UVS,
		    sizeof(DG_PRIM2_UVRGB), N_VERTS );
	OK_Scr_Mem( &((DG_PRIM2_UVRGB*)(prim2->uvrgb[ 1 ]))[i*N_VERTS], SCR_UVS,
		    sizeof(DG_PRIM2_UVRGB), N_VERTS );

	    
    }

    prim1->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );
    prim2->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

    return 1;

}



static int GetResources( Work *work, FMATRIX *mat )
{
    
    work->mat  = mat;

    work->alpha_param = 1.0f;
    work->alpha_off_flag = 0;

    work->loop_flag = 0;
    
    work->rot.vx = 0;
    work->rot.vy = 0;
    work->rot.vz = 0;

    work->rot_param = -0.40f;
    work->rot_y_max = irnd()%4096-2048;
    work->offset = OFFSET;
    work->speed  = SPEED;
    work->width  = WIDTH;
    work->speed_flag = 0;
    
    if ( InitPacket( work ) ) {    
	return 0;
    }
    else {
	return -1;
    }

}




void *NewHarrierMissileSmokeLine2( FMATRIX *mat, char *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->flag = flag;	/* 着弾取得用フラグ */

		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



void *NewDemoHarrierMissileSmokeLine2( int name, FMATRIX *mat )
{
	Work		*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;	/* 着弾メッセージ受信用 */
		work->flag = NULL;	

		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}
