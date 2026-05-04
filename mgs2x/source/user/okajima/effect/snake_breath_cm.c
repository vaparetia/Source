//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_breath_cm.c
	ＮＰＣスネークの息によるレンズ曇り

	2001/08/18 S.Okajima
	$Id: snake_breath_cm.c,v 1.1.1.3 2002/11/19 11:47:14 Yoshizawa1 Exp $
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
#define	N_VERTS			(8)
#define	SCREEN_NEAR		( 51.0f )
#define	P_RGB_MAX		(16)
#define	P_ALP_MAX		(32)
#define	SIZE			(10)
#define	LIMIT_DISTANCE	(3000.0f)
#define	SCN_CONT_TIME	(60*10)

#define	LIFE0		(30)
#define	LIFE1		(300)
#define	LIFE_MAX	(LIFE0 + LIFE1)

typedef	struct{
	GV_ACT_EX	actor ;
	DG_PRIM2	*prim ;
	int			life_max;
	int			life;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		clock,i;
	int		alpha_max;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	work->life--;
	if( work->life <= 0 ){
		work->life = 0;
		GV_DestroyActor( work ) ;
	}else{
		if( work->life > LIFE_MAX - LIFE0 ){
			alpha_max = P_ALP_MAX * (LIFE_MAX - work->life) / LIFE0;
		}else{
			alpha_max = P_ALP_MAX * work->life / LIFE1;
		}
		uvrgbwh = work->prim->uvrgb[clock];
		for( i=0; i<N_VERTS; i++ ){
			uvrgbwh->a = alpha_max;
			uvrgbwh++;
		}
	}

	if( GM_CheckPlayerStatus(PLAYER_INTRUDE|PLAYER_WATCH) ){
		DG_VisiblePrim2( work->prim );
	}else{
		DG_InvisiblePrim2( work->prim );
		GV_DestroyActor( work ) ;
		return;
	}
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center )
{
	FVECTOR		pos;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	float	screen_near_x;
	float	screen_near_y;
	float	ftemp0;
	float	ftemp1;
	float	angle;
	int	i;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( center, &pos, 1 );

	pos.vw = (pos.vw  > 0.0f)? pos.vw: -pos.vw;
	pos.vz = SCREEN_NEAR;
	pos.vx /= pos.vw;
	pos.vy /= pos.vw;

	ftemp0 = DG_FABS( pos.vx );
	ftemp1 = DG_FABS( pos.vy );


	if( ftemp0 > ftemp1 ){
		if( ftemp0 > 1.0f ){
			pos.vx/= ftemp0;
			pos.vy/= ftemp0;
		}
	}else{
		if( ftemp1 > 1.0f ){
			pos.vx/= ftemp1;
			pos.vy/= ftemp1;
		}
	}

//	pos.vx*= screen_near_x;
//	pos.vy*= screen_near_y;

	pos0     = prim->pos[0];
	pos1     = prim->pos[1];
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for( i=0; i<N_VERTS; i++ ){
		pos1->vz = pos0->vz = SCREEN_NEAR;
		pos1->vx = pos0->vx = (pos.vx + 0.25f*frnd())*screen_near_x;
		pos1->vy = pos0->vy = (pos.vy + 0.25f*frnd())*screen_near_y;

		uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
		uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
		uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
		uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

		angle = TPI*rnd();
		uvrgbwh0->h = uvrgbwh1->h =(int)(cosf( angle ) * SCREEN_NEAR*(0.5f + 0.25f*rnd()));
		uvrgbwh0->w = uvrgbwh1->w =(int)(sinf( angle ) * SCREEN_NEAR*(0.5f + 0.25f*rnd()));

		uvrgbwh0->r = uvrgbwh1->r = P_RGB_MAX ;
		uvrgbwh0->g = uvrgbwh1->g = P_RGB_MAX ;
		uvrgbwh0->b = uvrgbwh1->b = P_RGB_MAX ;
		uvrgbwh0->a = uvrgbwh1->a = 0 ;

		pos0++;
		pos1++;
		uvrgbwh0++;
		uvrgbwh1++;
	}

	return 1;
}

static int GetResources( Work *work, FVECTOR *pos )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex=NULL ;

	work->life = LIFE_MAX;

//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
//	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

#if 1
	switch( (irnd()>>8)%5 ){
	  case 0:
		tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
		break;
	  case 1:
		tex = DG_GetTexture( 12020883 /*"chi02_alp"*/ );
		break;
	  case 2:
		tex = DG_GetTexture( 13069459 /*"chi03_alp"*/ );
		break;
	  case 3:
		tex = DG_GetTexture( 14118035 /*"chi04_alp"*/ );
		break;
	  case 4:
		tex = DG_GetTexture( 15166611 /*"chi05_alp"*/ );
		break;
	}
#else
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
#endif

	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, 1, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex, pos ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

#define NPC_SNAKE_LENGTH (800.0f)
void *NewSnakeBreathOnCamera2( FMATRIX *world )
{
	Work	*work ;
	FVECTOR	fvtemp;
	FVECTOR	cam;
	float	inner;
	float	len;

	OPERATOR() ;

	if( !GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) ) return NULL;

	_sceVu0SubVector( &fvtemp, (FVECTOR *)DG_Chanls->eye.m[3], (FVECTOR *)world->m[3] ) ;
	len = GV_VecLen3F( &fvtemp );
	if( len > NPC_SNAKE_LENGTH ) return NULL;

	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = 1.0f;
	fvtemp.vw = 1.0f;

	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &fvtemp, &cam, 1 );

	DG_SetPos( world );
	DG_RotVector( &fvtemp, &fvtemp, 1 );

	inner = _sceVu0InnerProduct( &fvtemp, &cam );	// -1.0f で正面
	if( inner > -0.75f ) return NULL;

///////
#if 0
	DG_SetPos( &DG_Chanls->eye_inv );
	DG_RotVector( &fvtemp, &fvtemp, 1 );			// カメラから見た進行方向
	DG_PutVector( (FVECTOR *)world->m[3], &pos, 1 );	// カメラから見た発生点

	_sceVu0ScaleVector( &fvtemp, &fvtemp, len*0.25f);
	_sceVu0AddVector( &fvtemp, &fvtemp, &pos ) ;	// カメラから見た到達点
#else
	_sceVu0ScaleVector( &fvtemp, &fvtemp, len*0.25f);
	_sceVu0AddVector( &fvtemp, &fvtemp, (FVECTOR *)world->m[3] ) ;
#endif

///////

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, &fvtemp ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}

