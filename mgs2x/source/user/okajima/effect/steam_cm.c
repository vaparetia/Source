//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	steam_cm.c
	水蒸気によるレンズ曇り

	2000/11/10 S.Okajima
	$Id: steam_cm.c,v 1.1.1.3 2002/11/19 11:47:15 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
#define	N_VERTS			(4)
#define	SCREEN_NEAR		( 51.0f )
#define	P_RGB_MAX		(128)
#define	P_ALP_MAX		(64)
#define	SIZE			(10)
#define	LIMIT_DISTANCE	(3000.0f)
#define	SCN_CONT_TIME	(60*10)

#define	RAISE			(60000)

enum {
	FLAG_PROG,
	FLAG_SCN
};


typedef	struct{
	GV_ACT_EX	actor ;
	int			name;
	int			where;

	DG_PRIM2	*prim ;
	FVECTOR		check[2];
	int			life_max;
	int			life_birth;
	int			life;
	int			cycle_count;
	int			life_parts[N_VERTS];
	int			activate_num;
	int			prog_scn_flag;
	int			death;
} Work ;

static Work *OK_STEAM_CAMERA_WORK = NULL;

/*----------------------------------------------------------------*/
enum {
	REQ_KILL = -1,
	REQ_NO
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
		  case REQ_KILL:
		  default:
			work->death = 1;
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		clock,i;
	int		itemp;
	int		count;
	FVECTOR	fvtemp;
	FVECTOR	cam;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;			/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh_before ;	/* スプライト用 */
	int		*life_parts;
	float	screen_near_x=0.0f;
	float	screen_near_y=0.0f;
	float	length0=0.0f;
	float	length1=0.0f;

	if( work->prog_scn_flag == FLAG_SCN ){
		CheckMesgParam( work );
	}

	if( work->death ){
		DG_InvisiblePrim2( work->prim );
		GV_DestroyActor( work ) ;
		return;
	}

	count = 0;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	pos            = work->prim->pos[     clock ];
	pos_before     = work->prim->pos[ 1 - clock ];
	uvrgbwh        = work->prim->uvrgb[     clock ];
	uvrgbwh_before = work->prim->uvrgb[ 1 - clock ];
	life_parts     = work->life_parts;
	work->life--;
	if( work->life <= 0 ){
		work->life = 0;
		for( i=0; i<work->activate_num; i++ ){
			itemp = uvrgbwh_before->a;
			if(GV_Time%(work->activate_num+1)==i) itemp--;
			if( itemp <= 0 ){
				count++;
				itemp = 0;
			}
			uvrgbwh->a = itemp;
			uvrgbwh++;
			uvrgbwh_before++;
		}
		if( count >= work->activate_num ) GV_DestroyActor( work ) ;
	}else{	//受け付け時間内
		for( i=0; i<work->activate_num; i++ ){
			DG_COPY_VEC( pos, pos_before );
			uvrgbwh->w = uvrgbwh_before->w ;
			uvrgbwh->h = uvrgbwh_before->h ;
			uvrgbwh->a = uvrgbwh_before->a;

			if( work->prog_scn_flag == FLAG_SCN ){
				uvrgbwh->a = P_ALP_MAX;
			}else{
				if( (*life_parts) < work->life_birth ){	// 立ち上げ
					(*life_parts)++;
				}
				uvrgbwh->a = P_ALP_MAX * (*life_parts) / work->life_birth;
			}

			pos++;
			pos_before++;
			uvrgbwh++;
			uvrgbwh_before++;
			life_parts++;
		}
	}


	if( GM_CheckPlayerStatus(PLAYER_INTRUDE|PLAYER_WATCH)  ||  work->prog_scn_flag == FLAG_SCN ){
		DG_VisiblePrim2( work->prim );
	}else{
		DG_InvisiblePrim2( work->prim );
		return;
	}


	if( work->life > 0 ){		//受け付け時間内
		if( work->life > 2
		 && GM_CheckPlayerStatus(PLAYER_INTRUDE|PLAYER_WATCH) ){
			DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );
			_sceVu0SubVector( &fvtemp, &work->check[0], &cam ) ;
			length0 = GV_VecLen3F( &fvtemp );
			_sceVu0SubVector( &fvtemp, &work->check[1], &cam ) ;
			length1 = GV_VecLen3F( &fvtemp );
			length0 = (length0 < length1)? length0: length1;
			if( length0 < LIMIT_DISTANCE ){
				work->cycle_count++;
				if( work->cycle_count >= work->life_birth ){
					work->cycle_count = 0;
					if( work->activate_num < N_VERTS ){
						pos     = work->prim->pos[   clock ];
						uvrgbwh = work->prim->uvrgb[ clock ];
						pos    += work->activate_num;
						uvrgbwh+= work->activate_num;

						screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls->screen;
						screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

						screen_near_x*= 1.2f;
						screen_near_y*= 1.2f;

						pos->vz = SCREEN_NEAR;
						pos->vx = screen_near_x * frnd();
						pos->vy = screen_near_y * frnd();

						uvrgbwh->w = SCREEN_NEAR ;
						uvrgbwh->h = SCREEN_NEAR ;
						uvrgbwh->a = 0;

						work->life_parts[ work->activate_num ] = 0;	// 立ち上げスタート

						work->activate_num++;
					}
				}
			}
		}
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );

	OK_STEAM_CAMERA_WORK = NULL;
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int	i;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for( i=0; i<N_VERTS; i++ ){
		work->life_parts[i] = work->life_birth;
		uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
		uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
		uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
		uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

		uvrgbwh0->w = uvrgbwh1->w = 0 ;
		uvrgbwh0->h = uvrgbwh1->h = 0 ;

		uvrgbwh0->r = uvrgbwh1->r = P_RGB_MAX ;
		uvrgbwh0->g = uvrgbwh1->g = P_RGB_MAX ;
		uvrgbwh0->b = uvrgbwh1->b = P_RGB_MAX ;
		uvrgbwh0->a = uvrgbwh1->a = P_ALP_MAX ;

		uvrgbwh0++;
		uvrgbwh1++;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_VERTS );

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->death = 0;

	work->activate_num = 0;
	work->cycle_count = 0;


	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, 1, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewSteamOnCamera( FVECTOR *center, SVECTOR *rot, float size, int life )
{
	Work		*work ;
	int	i;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	OPERATOR() ;

	if( OK_STEAM_CAMERA_WORK != NULL ){
		work = OK_STEAM_CAMERA_WORK;
		work->check[0].vx = 0.0f;
		work->check[0].vy = 0.0f;
		work->check[0].vz = 0.0f;
		work->check[1].vx = 0.0f;
		work->check[1].vy = 0.0f;
		work->check[1].vz =-size;
		DG_SetPos2( center, rot );
		DG_PutVector( work->check, work->check, 2 );
		work->life = work->life_max = life;
		work->life_birth = life / N_VERTS;
		if( work->life_birth <= 0 ) work->life_birth = 1;

		uvrgbwh = work->prim->uvrgb[ 0 ] ;
		for( i=0; i<work->activate_num; i++ ){
			work->life_parts[ i ] = uvrgbwh->a * work->life_birth / P_ALP_MAX;
			uvrgbwh++;
		}


		return NULL;
	}

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->check[0].vx = 0.0f;
		work->check[0].vy = 0.0f;
		work->check[0].vz = 0.0f;
		work->check[1].vx = 0.0f;
		work->check[1].vy = 0.0f;
		work->check[1].vz =-size;
		DG_SetPos2( center, rot );
		DG_PutVector( work->check, work->check, 2 );
		work->life = work->life_max = life;
		work->life_birth = life / N_VERTS;
		if( work->life_birth <= 0 ) work->life_birth = 1;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	work->prog_scn_flag = FLAG_PROG;

	OK_STEAM_CAMERA_WORK = work;

	return (void *)work ;
}


void *NewSteamOnCameraScenario( int name, int where )
{
	Work	*work ;
	int		i;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;			/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh_before ;	/* スプライト用 */
	float	screen_near_x;
	float	screen_near_y;
	int		life;

	OPERATOR() ;

	life = SCN_CONT_TIME;
	if ( GCL_GetOption( 't' ) != NULL ){
		life = GCL_GetNextInt();
	}
	life = DIRECT_TICK( life ) ;
	if( life < 1 ) life = 1;


	if( OK_STEAM_CAMERA_WORK != NULL ){
		work = OK_STEAM_CAMERA_WORK;
		work->name  = name;
		work->where = where;

		work->life = work->life_max = life;
		work->life_birth = work->life / N_VERTS;
		if( work->life_birth <= 0 ) work->life_birth = 1;
		return (void *)work ;
	}else{
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
	}
	work->prog_scn_flag = FLAG_SCN;

	work->life = work->life_max = life;
	work->life_birth = work->life / N_VERTS;
	if( work->life_birth <= 0 ) work->life_birth = 1;


	if( DG_Chanls->screen == 0.0f  ||  DG_Chanls->width  ||  DG_Chanls->height ){
		screen_near_x = SCREEN_NEAR / (ASPECT_X()                                 ) / 2.0f;
		screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DISPLAY_WIDTH / DISPLAY_HEIGHT) / 2.0f;
	}else{
		screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls->screen;
		screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;
	}

	pos        = work->prim->pos[0];
	pos_before = work->prim->pos[1];
	uvrgbwh        = work->prim->uvrgb[0];
	uvrgbwh_before = work->prim->uvrgb[1];
	screen_near_x*= 1.2f;
	screen_near_y*= 1.2f;
	for( i=0; i<N_VERTS; i++ ){
		pos_before->vz = pos->vz = SCREEN_NEAR;
		pos_before->vx = pos->vx = screen_near_x * frnd();
		pos_before->vy = pos->vy = screen_near_y * frnd();
		uvrgbwh_before->w = uvrgbwh->w = SCREEN_NEAR ;
		uvrgbwh_before->h = uvrgbwh->h = SCREEN_NEAR ;
		uvrgbwh_before->a = uvrgbwh->a = 0;
		work->life_parts[ i ] = 0;	// 立ち上げスタート
		pos++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}
	work->activate_num = N_VERTS;

	OK_STEAM_CAMERA_WORK = work;

	return (void *)work ;
}

