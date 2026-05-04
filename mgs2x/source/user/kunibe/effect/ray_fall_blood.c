//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ray_fall_blood.c
  ＲＡＹダメージ血
  2001/07/26 Yuuta Kunibe
  $Id: ray_fall_blood.c,v 1.1.1.3 2002/11/19 11:44:47 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"



#define	N_PRIMS				(256)
#define	N_VERTS				(4)

#define	N_PRIMS2			(64)
#define	N_VERTS2			(4)

#define BLOOD_TEX			(10972307)//( GV_StrCode( "chi01_alp" ) )

#define BASE_RGB			(12)
#define BASE_R				(27)
#define BASE_G				(5)
#define BASE_B				(5)
#define ALPHA				(64)
#define	SPEED				(400.0f)

#define MAX_RADIUS			(100.0f)
#define	MAX_SPLASH_PARAM	(15.0f)
#define	MAX_SPEED			(50.0f)

#define	LENGTH_PARAM		(20.0f)

#define	SCR_POS				(SCRPAD_ADDR)
#define	SCR_UVS				(SCRPAD_ADDR + 0x2000)


#define	FLOOR_CHECK_NUM		(3)
#define	FLOOR_HEIGHT		(4000.0f)
#define	FLOOR_CHECK_Z		(22000.0f)

enum {
	BLOOD_ACTIVE = 0,	// 流れ落ち
	BLOOD_FLOOR,		// フロア着き
	BLOOD_STOP,			// 処理停止１
	BLOOD_SLEEP,		// 処理停止２
};

enum {
	BLOOD_FLOOR_NOCHECK = 0,
	BLOOD_FLOOR_THROUGH,
	BLOOD_FLOOR_HIT,
};

typedef struct {
    FVECTOR	vec;
    int		alpha;
	int		floor_check_flag;
	int		flag;
} Blood_Param;



typedef	struct	{

    GV_ACT_EX	actor;
	int			name;

    DG_PRIM2	*prim;
	DG_PRIM2	*prim2;

    FMATRIX		*mat;
	FMATRIX		mat_rot;
	FMATRIX		matrix;

    EFTCONTROL  *eft_ctrl;	
	
	SVECTOR		rot;

    int			init_alpha;

	float		radius;
	float		speed;
	float		splash_param;

	int			count;

	int			phase;
	float		add_radius;	

	FMATRIX		floor_check_matrix[3];
	
    Blood_Param	param[N_PRIMS2];
    Blood_Param	param_part[N_PRIMS];

} Work ;


extern void *NewRayBloodSmoke( FVECTOR *center );



static void RecieveMessage( Work *work )
{

    GV_MSG*	msg;
    int 	n_msg;
	int		fade_count;

    if ( work->name == 0 ) {
		return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

		switch ( msg->message[ 0 ] ) {
		case 0:			/* 吹き出し減衰 */
			work->phase = 1;
			break;
		case 1:			/* 流血減衰 */
			work->phase = 1;
			fade_count = msg->message[1];
			if ( fade_count != 0 ) {
				work->add_radius = MAX_RADIUS / (float)fade_count;
			}
			break;
		}

		msg++;

    }

}





static void MakeSplashMatrix( Work *work )
{


	if ( work->name == 0 ) {	/* game */

		_sceVu0MulMatrix( &work->matrix, work->mat, &work->mat_rot );

		/* Y軸上向ける(本当はダメ) */
		if ( work->matrix.m[1][1] < 0.0f ) {
			SVECTOR	rot;
			rot.vx = 0;
			rot.vy = 0;
			rot.vz = 2048;
		
			DG_SetPos( &work->matrix );
			DG_RotatePos( &rot );
			DG_GetPos( &work->matrix );
		
		}
	}
	else {

		DM_EftControlMatrix( work->eft_ctrl, &work->matrix );

	}

    //HZX_ViewMatrix( &work->matrix, 1000.0f );

}




static int ActSplash( Work *work )
{

	DG_PRIM2		*prim;

    int				i,j;
    int				clock;
    FVECTOR			*pos;
    FVECTOR			*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;

    Blood_Param		*param;

	float			ftmp;

	float			angle;
	float			fsin;
	float			fcos;
	float			radius;
	float			width;

	int				sleep_num;



	prim = work->prim2;


	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

    
    clock     = prim->buffer_clock;
    pos       = prim->pos[clock];
    pos_pre   = prim->pos[1-clock];    
    uvrgb     = prim->uvrgb[clock];
    uvrgb_pre = prim->uvrgb[1-clock];
    param     = work->param;

	sleep_num = 0;

    for ( i = 0 ; i < N_PRIMS2 ; i++ ) {

		param->alpha -= 16;

		if ( param->alpha < 0 ) {

			if ( work->speed > 20.0f ) {
			
				angle = PI/2.0f + frnd() * PI/2.0f;	// 0 ～ PI
				fsin = vu0_Sin( angle );
				fcos = vu0_Cos( angle );

				ftmp   = work->radius * 0.5f + frnd() * work->radius * 0.5f;
				radius = work->radius * 0.75f + ftmp;
				width  = work->radius * 2.0f - ftmp;

				ftmp = work->speed*0.5f + frnd()*work->speed*0.5f;
				param->vec.vx = ftmp * fcos;
				param->vec.vy = ftmp * fsin;
				param->vec.vz = work->speed * 2.0f + frnd()*work->speed;
				_sceVu0ScaleVector( &param->vec, &param->vec, work->splash_param );

				pos->vx = radius * fcos - width * fsin;
				pos->vy = radius * fsin + width * fcos;
				pos->vz = 0.0f;
				pos->vw = 1.0f;
				pos++;

				pos->vx = radius * fcos + width * fsin;
				pos->vy = radius * fsin - width * fcos;
				pos->vz = 0.0f;
				pos->vw = 1.0f;
				pos++;

				_sceVu0AddVector( pos, ( pos-2 ), &param->vec );
				pos++;
	
				_sceVu0AddVector( pos, ( pos-2 ), &param->vec );
				pos++;


				param->alpha = work->init_alpha + irnd() % work->init_alpha;
		
				uvrgb->a = param->alpha;
				uvrgb++;
				uvrgb->a = param->alpha;
				uvrgb++;
				uvrgb->a = param->alpha;
				uvrgb++;
				uvrgb->a = param->alpha;
				uvrgb++;

				param->flag = BLOOD_ACTIVE;

			}
			else {

				if ( param->flag == BLOOD_ACTIVE ) {

					for ( j = 0 ; j < 4 ; j++ ) {
						DG_COPY_VEC( pos, &DG_ZeroVector );
						pos++;
						uvrgb->a = 0;
						uvrgb++;
					}
				
					param->flag = BLOOD_STOP;

				}
				else {
					pos += 4;
					uvrgb += 4;
					sleep_num++;
				}

				param->alpha = work->init_alpha + irnd() % work->init_alpha;

			}

			pos_pre += 4;

		}
		else {

			if ( param->flag == BLOOD_ACTIVE ) {
				for ( j = 0 ; j < N_VERTS ; j++ ) {

					if ( j == 2 || j == 3 ) {
						_sceVu0AddVector( pos, pos_pre, &param->vec );
					}
					else {
						DG_COPY_VEC( pos, pos_pre );
					}

					pos++;
					pos_pre++;

					uvrgb->a = param->alpha;
					uvrgb++;
				}

			}
			else {

				if ( param->flag == BLOOD_STOP ) {
					for ( j = 0 ; j < 4 ; j++ ) {
						DG_COPY_VEC( pos, &DG_ZeroVector );
						pos++;
						uvrgb->a = 0;
						uvrgb++;
					}
					pos_pre += 4;

					param->flag = BLOOD_SLEEP;
					
				}
				else {
					pos += 4;
					pos_pre += 4;
					uvrgb += 4;
				}

				sleep_num++;

			}

		}

		param++;
	
    }


	return (sleep_num);

}



static int ActPart( Work *work )
{

	DG_PRIM2		*prim;
    int				i,j;
    int				clock;
    FVECTOR			*pos;
    FVECTOR			*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;

	FVECTOR			vectmp;
    Blood_Param		*param;
	float			ftmp;
	FVECTOR			position[4];

	float			angle;
	float			fsin;
	float			fcos;
	float			radius;
	float			width;
	FVECTOR			vectmp2;
	
	int				hit;
	int				sleep_num;
	

	prim = work->prim;


	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

    
    clock     = prim->buffer_clock;
    pos       = prim->pos[clock];
    pos_pre   = prim->pos[1-clock];    
    uvrgb     = prim->uvrgb[clock];
    uvrgb_pre = prim->uvrgb[1-clock];
    param     = work->param_part;

	sleep_num = 0;

    for ( i = 0 ; i < N_PRIMS ; i++ ) {

		param->alpha -= 2;

		if ( param->alpha < 0 ) {

			if ( work->radius > 0.0f ) {

				angle = PI/2.0f + frnd()*PI/2.0f;	// 0 ～ PI
				fsin = vu0_Sin( angle );
				fcos = vu0_Cos( angle );

				ftmp = work->radius*0.5f + frnd()*work->radius*0.5f;
				radius = work->radius + ftmp;
				width  = work->radius*2.0f - ftmp*1.5f;

				position[0].vx = radius * fcos - width * fsin;
				position[0].vy = radius * fsin + width * fcos;
				position[0].vz = 0.0f;
				position[0].vw = 1.0f;

				position[1].vx = radius * fcos + width * fsin;
				position[1].vy = radius * fsin - width * fcos;
				position[1].vz = 0.0f;
				position[1].vw = 1.0f;


				vectmp.vx = work->speed * 0.2f * fcos;
				vectmp.vy = work->speed * 0.2f * fsin;
				vectmp.vz = work->speed + work->speed * fsin;
				_sceVu0ScaleVector( &vectmp, &vectmp, 0.3f );


				_sceVu0ScaleVector( &vectmp2, &vectmp, LENGTH_PARAM );
				_sceVu0AddVector( &position[2], &position[0], &vectmp2 );
				_sceVu0AddVector( &position[3], &position[1], &vectmp2 );
			


				/* 速度絶対化 */
				DG_SetPos( &work->matrix );
				DG_RotVector( &vectmp, &param->vec, 1 );

				/* 初期位置いっぺんに絶対化 */
				DG_SetPos( &work->matrix );
				DG_PutVector( position, pos, 4 );

				pos += 4;


				param->alpha = work->init_alpha + irnd()%work->init_alpha;
		
				uvrgb->a = param->alpha;
				uvrgb++;
				uvrgb->a = param->alpha;
				uvrgb++;
				uvrgb->a = param->alpha;
				uvrgb++;
				uvrgb->a = param->alpha;
				uvrgb++;

				param->floor_check_flag = BLOOD_FLOOR_NOCHECK;
				param->flag = BLOOD_ACTIVE;

			}
			else {

				if ( param->flag == BLOOD_ACTIVE || param->flag == BLOOD_FLOOR ) {
					for ( j = 0 ; j < 4 ; j++ ) {
						DG_COPY_VEC( pos, &DG_ZeroVector );
						pos++;
						uvrgb->a = 0;
						uvrgb++;
					}
					param->flag = BLOOD_STOP;
				}
				else {
					pos += 4;
					uvrgb += 4;
					sleep_num++;
				}					
				
				param->alpha = work->init_alpha + irnd() % work->init_alpha;
				
			}

			pos_pre += 4;			

		}
		else {

			switch ( param->flag ) {
			case BLOOD_ACTIVE:	/* 流れ落ちる */

				/* 根本位置更新 */
				_sceVu0ScaleVector( &vectmp, &param->vec, 1.5f );
				_sceVu0AddVector( &pos[0], &pos_pre[0], &vectmp );
				_sceVu0AddVector( &pos[1], &pos_pre[1], &vectmp );

				switch ( param->floor_check_flag ) {
				case BLOOD_FLOOR_NOCHECK:
					/* 先端位置更新 */
					_sceVu0ScaleVector( &vectmp, &param->vec, LENGTH_PARAM );
					_sceVu0AddVector( &pos[2], &pos[0], &vectmp );
					_sceVu0AddVector( &pos[3], &pos[1], &vectmp );

					param->vec.vx *= 0.98f;
					param->vec.vy -= 2.0f;
					param->vec.vz *= 0.98f;

					/* 先端がフロア高さ以下になった時に一回だけフロアチェック */
					if ( pos[2].vy < FLOOR_HEIGHT ) {
						hit = 0;
						for ( j = 0 ; j < 3 ; j++ ) {
							DG_SetPos( &work->floor_check_matrix[j] );
							DG_PutVector( &pos[2], &vectmp, 1 );
							if ( vectmp.vz < FLOOR_CHECK_Z && vectmp.vz > -FLOOR_CHECK_Z ) {
								hit++;
							}
						}

						if ( hit < 3 ) {
							param->floor_check_flag = BLOOD_FLOOR_THROUGH;
						}
						else {
							param->floor_check_flag = BLOOD_FLOOR_HIT;
							pos[2].vy = FLOOR_HEIGHT;
							pos[3].vy = FLOOR_HEIGHT;
						}							
						
					}					
					break;
				case BLOOD_FLOOR_THROUGH:	/* 床判定通過しているのでそのまま先端処理 */
					/* 先端位置更新 */
					_sceVu0ScaleVector( &vectmp, &param->vec, LENGTH_PARAM );
					_sceVu0AddVector( &pos[2], &pos[0], &vectmp );
					_sceVu0AddVector( &pos[3], &pos[1], &vectmp );

					param->vec.vx *= 0.98f;
					param->vec.vy -= 2.0f;
					param->vec.vz *= 0.98f;
					break;
				case BLOOD_FLOOR_HIT:
					/* 床着地 */
					if ( pos[0].vy < FLOOR_HEIGHT ) {	/* 完全着地 */

						pos[0].vx += frnd()*500.0f;
						pos[0].vz += frnd()*500.0f;
					
						DG_COPY_VEC( &pos[1], &pos[0] );
						pos[1].vx -= 1000.0f;
						pos[1].vy = FLOOR_HEIGHT;
						pos[1].vz += 1000.0f;

						DG_COPY_VEC( &pos[2], &pos[0] );
						pos[2].vx += 1000.0f;
						pos[2].vy = FLOOR_HEIGHT;
						pos[2].vz -= 1000.0f;

						DG_COPY_VEC( &pos[3], &pos[0] );
						pos[3].vx -= 1000.0f;
						pos[3].vy = FLOOR_HEIGHT;
						pos[3].vz -= 1000.0f;					

						pos[0].vx += 1000.0f;
						pos[0].vy = FLOOR_HEIGHT;
						pos[0].vz += 1000.0f;

						param->flag = BLOOD_FLOOR;
						
					}
					else {								/* 着地途中 */
						_sceVu0ScaleVector( &vectmp, &param->vec, LENGTH_PARAM );
						_sceVu0AddVector( &pos[2], &pos[0], &vectmp );
						pos[2].vy = FLOOR_HEIGHT;
						_sceVu0AddVector( &pos[3], &pos[1], &vectmp );
						pos[3].vy = FLOOR_HEIGHT;
					}
					break;

				}

				pos += 4;
				pos_pre += 4;				

				/* アルファ更新 */
				for ( j = 0 ; j < N_VERTS ; j++ ) {
					uvrgb->a = param->alpha;
					uvrgb++;
				}

				break;
			case BLOOD_FLOOR:	/* 床に着いた */
				for ( j = 0 ; j < N_VERTS ; j++ ) {
					DG_COPY_VEC( pos, pos_pre );
					pos++;
					pos_pre++;
				
					uvrgb->a = param->alpha;
					uvrgb++;
				}
				break;
			default:
				if ( param->flag == BLOOD_STOP ) {
					for ( j = 0 ; j < 4 ; j++ ) {
						DG_COPY_VEC( pos, &DG_ZeroVector );
						pos++;
						uvrgb->a = 0;
						uvrgb++;
					}
					
					param->flag = BLOOD_SLEEP;
					pos_pre += 4;
					
				}
				else {
					pos += 4;
					pos_pre += 4;
					uvrgb += 4;
				}

				sleep_num++;
				break;

			}

		}

		param++;
	
    }


	return (sleep_num);

}



static void Act( Work *work )
{

	int		splash_sleep;
	int		fall_sleep;



	MakeSplashMatrix( work );



	if ( work->name == 0 ) {

		work->splash_param = work->splash_param * 0.95f + 2.0f * 0.05f;	// 0.98f

		if ( work->count > 60 ) {	// 90
			work->radius -= 0.50f;	// 0.50f
			if ( work->radius < 0.0f ) {
				work->radius = 0.0f;
			}
		}
		
	}
	else {

		RecieveMessage( work );

		if ( work->phase == 1 ) {

			work->splash_param = work->splash_param * 0.98f + 2.0f * 0.02f;
			
			work->radius -= work->add_radius;
			if ( work->radius < 0.0f ) {
				work->radius = 0.0f;
			}
		}

	}

			
	work->speed = MAX_SPEED * work->radius / MAX_RADIUS;

	/* １フレーム目で血煙呼び出し */
	if ( work->count == 1 ) {
		NewRayBloodSmoke( (FVECTOR *)work->matrix.m[3] );
		GM_SeSetMode( SD_E_RBLOOD01, (FVECTOR *)work->matrix.m[3], GM_SEMODE_BOMB );
	}

	work->count++;

    fall_sleep   = ActPart( work );
    splash_sleep = ActSplash( work );

    if ( fall_sleep >= N_PRIMS && splash_sleep >= N_PRIMS2 ) {
		GV_DestroyActor( work );
	}

}



static void Die( Work *work )
{
	if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
	if ( work->prim2 ) work->prim2 = OK_FreePrim2( work->prim2 );
}





static int InitPacket( Work *work )
{

    int				i,j;
    DG_PRIM2		*prim;
    DG_TEX			*tex;
    FVECTOR			*pos;
    DG_PRIM2_UVRGB	*uvrgb;
    Blood_Param		*param;


    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG,
									  N_PRIMS, N_VERTS );

    if ( prim == NULL ) {
		return 0;
    }



    tex = DG_GetTexture( BLOOD_TEX );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


    param = work->param_part;

	for ( j = 0 ; j < 2 ; j++ ) {

		pos   = SCR_POS;
		uvrgb = SCR_UVS;
	
		for ( i = 0 ; i < N_PRIMS/2 ; i++ ) {

			DG_COPY_VEC( pos, &DG_ZeroVector );
			pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );
			pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );
			pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );
			pos++;


			if ( i==0 ) {
				param->alpha = 0;
			}
			else {
				param->alpha = irnd()%ALPHA;
			}


			switch (i%4) {
			case 0:
				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x8fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x8fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x0fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x0fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
				break;
			case 1:
				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x8fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x8fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x0fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x0fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
				break;
			case 2:
				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x8fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x8fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x0fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x0fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
				break;
			case 3:
				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x8fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x8fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x0fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
	
				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096;
				uvrgb->f = 0x0fff;
				uvrgb->r = BASE_R;
				uvrgb->g = BASE_G;
				uvrgb->b = BASE_B;
				uvrgb->a = 0;
				uvrgb++;
				break;

			}

			param->flag = BLOOD_SLEEP;
			
			param++;

		}	
    
		OK_Scr_Mem( &prim->pos[ 0 ][j*N_PRIMS*N_VERTS/2], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS/2 );
		OK_Scr_Mem( &prim->pos[ 1 ][j*N_PRIMS*N_VERTS/2], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS/2 );
		OK_Scr_Mem( &( (DG_PRIM2_UVRGB *)( prim->uvrgb[ 0 ] ) )[j*N_PRIMS*N_VERTS/2],
					SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS/2 );
		OK_Scr_Mem( &( (DG_PRIM2_UVRGB *)( prim->uvrgb[ 1 ] ) )[j*N_PRIMS*N_VERTS/2],
					SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS/2 );

	}
 
    return 1;

}


static int InitPacket2( Work *work )
{

    int				i;
    DG_PRIM2		*prim;
    DG_TEX			*tex;
    FVECTOR			*pos;
    DG_PRIM2_UVRGB	*uvrgb;
    Blood_Param		*param;


    prim = work->prim2 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG,
									   N_PRIMS2, N_VERTS2 );
    if ( prim == NULL ) {
		return 0;
    }


    prim->root = &work->matrix;

    tex = DG_GetTexture( BLOOD_TEX );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );



    pos   = SCR_POS;
    uvrgb = SCR_UVS;
    param = work->param;	

    for ( i = 0 ; i < N_PRIMS2 ; i++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos++;


		if ( i==0 ) {
			param->alpha = 0;
		}
		else {
			param->alpha = irnd()%ALPHA;
		}
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.5F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = param->alpha;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.5F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = param->alpha;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = param->alpha;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = param->alpha;
		uvrgb++;


		param->floor_check_flag = BLOOD_FLOOR_NOCHECK;
		param->flag = BLOOD_SLEEP;
		
		param++;


    }	

    
    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS2 * N_VERTS2 );
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS2 * N_VERTS2 );
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS2 * N_VERTS2 );
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS2 * N_VERTS2 );
 
    return 1;

}





static int GetResources( Work *work )
{

	int		i;
	SVECTOR	rot;
	
	/* 放出マトリクス算出 */
	MakeSplashMatrix( work );

	
	/* 各パラメータの初期化 */
	work->init_alpha = ALPHA;
	work->count      = 0;

	work->radius = MAX_RADIUS;
	work->splash_param = MAX_SPLASH_PARAM;

	/* 床当たりチェック用マトリクス生成 */
	for ( i=0; i<3; i++ ) {
		rot.vx = 0;
		rot.vy = -(2048/3) + (2048/3)*i;
		rot.vz = 0;

		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_GetPos( &work->floor_check_matrix[i] );		
	}
	
	if ( !( InitPacket( work ) ) ) {
	    return -1;
	}
	if ( !( InitPacket2( work ) ) ) {
		return -1;
	}
	return 0;

}




void *NewRayFallBlood( FMATRIX *mat, FMATRIX *mat_rot )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->name = 0;

		/* 放出マトリクス確保 */
		work->mat = mat;
		DG_COPY_MAT( &work->mat_rot, mat_rot );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL ;
		}
	}
	return (void *)work ;

}



/* デモ用呼びだし口 */
void *NewDemoRayFallBlood( int name, int con_name )
{

	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {

		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->name = name;
		work->phase = 0;
		work->add_radius = 0.0f;

		work->eft_ctrl = DM_GetEftControl( con_name );
		DM_EftControlMatrix( work->eft_ctrl, &work->matrix );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL ;
		}
	}
	return (void *)work ;

}
