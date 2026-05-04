//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_cm.c
	カメラ画面に飛び散る血
	1999/09/01 S.Okajima
	$Id: blood_cm.c,v 1.1.1.3 2002/11/19 11:46:58 Yoshizawa1 Exp $
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

#include	"gameheader.h"
#include	"libmt.h"
#include	"blood.h"
#include	"../etc/ok_util.h"

extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern void *NewBloodCamera( void );

/*----------------------------------------------------------------*/
#define	SCREEN_NEAR	( 51.0f )


#define	COL_R			(32)
#define	COL_G			(100)
#define	COL_B			(100)

#ifdef ENGLISH
#define	COL_A			(64)
#else
#define	COL_A			(BLOOD_ALPHA_0)
#endif

#define	SC_DOWN			(0.001f)

#define	MAX_SIZE		(16.0f)
#define	SIZE_RANDAM		(4.0f)


#define	DIVIDE			(4)
#define	CONST			(65536*2)
#define	DATA			(CONST*(DIVIDE-1)/DIVIDE)

#define	LIFE_TIME		(60)

#define	SPREAD_RANGE	(1)

#define	MAX_TEX			(5)
//#define	N_VERTS			(64)
#define	N_VERTS			(16)
#define	N_POLYS			(N_VERTS/4)
#define	N_PRIMS			(1)


#define	R_POS		(1)
#define	R_UVS		(2)

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

#define	PARAM1		(MAX_FVECTOR / (R_POS + R_UVS  +  R_POS + R_UVS))
#define	PARAM2		(PARAM1 * 16)

#define	SCR_POS0	(SCRPAD_ADDR)
#define	SCR_POS1	(SCR_POS0 + PARAM2)
#define	SCR_UVS0	(SCR_POS1 + PARAM2 )
#define	SCR_UVS1	(SCR_UVS0 + PARAM2 + PARAM2 )

typedef	struct	{
	DG_PRIM2			*prim ;
	FVECTOR				sc_pers[  N_PRIMS*N_POLYS];
	float				sc_width[ N_PRIMS*N_POLYS];
	float				sc_height[N_PRIMS*N_POLYS];
} Unit ;


typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	int			prim_count;

	Unit		unit[MAX_TEX];
} Work ;

Work	*OK_BLOOD_CM_WORK = NULL;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j;
	FVECTOR	*pos;
	FVECTOR	*sc_pers;
	FVECTOR	fvtemp;
	float	*sc_width;
	float	*sc_height;
	float		screen_near_x;
	float		screen_near_y;
	int		clock;


	if( !GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE)
	 || GM_CheckGameStatus( STATE_DEMO ) ){
		for( i=0; i<MAX_TEX; i++ ){
			DG_InvisiblePrim2( work->unit[i].prim ) ;
			for( i=0; i<MAX_TEX; i++ ){
				sc_pers   = work->unit[i].sc_pers;
				for ( j=0; j<N_PRIMS*N_POLYS; j++ ){
					(sc_pers++)->vz = 0.0f;
				}
			}
		}
	}else{
		for( i=0; i<MAX_TEX; i++ ){
			DG_VisiblePrim2( work->unit[i].prim ) ;
			GM_GroupPrim2( work->unit[i].prim, GM_CurrentStageMap ) ;
			DG_SwitchBuffPrim2( work->unit[i].prim );
		}
	}

// yano /*0除算回避*/
#if 0
	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                       ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;
#else
	if( DG_Chanls->screen == 0.0f ){ DG_Chanls->screen = 0.000001f; }
	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                       ) / DG_Chanls->screen;
	if( DG_Chanls->height == 0.0f ){ DG_Chanls->height = 0.000001f; }
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;
#endif
	clock = work->unit[0].prim->buffer_clock;
	for( i=0; i<MAX_TEX; i++ ){
		sc_pers   = work->unit[i].sc_pers;
		sc_width  = work->unit[i].sc_width;
		sc_height = work->unit[i].sc_height;
		pos       = work->unit[i].prim->pos[clock];
		for ( j=0; j<N_PRIMS*N_POLYS; j++ ){
//			if(sc_pers->vz==SCREEN_NEAR){
			if(sc_pers->vz > 1.0f){
//printf("%d %d\n",i,j);
				fvtemp.vx = sc_pers->vx*screen_near_x;
				fvtemp.vy = sc_pers->vy*screen_near_y;
				fvtemp.vz = SCREEN_NEAR;
//printf("%f %f %f %f\n",fvtemp.vx,fvtemp.vy,(*sc_width),(*sc_height));
				pos->vx = fvtemp.vx - (*sc_width);
				pos->vy = fvtemp.vy;
				pos->vz = fvtemp.vz;
				pos++;
				pos->vx = fvtemp.vx + (*sc_width);
				pos->vy = fvtemp.vy;
				pos->vz = fvtemp.vz;
				pos++;
				pos->vx = fvtemp.vx - (*sc_width);
				pos->vy = fvtemp.vy + (*sc_height);
				pos->vz = fvtemp.vz;
				pos++;
				pos->vx = fvtemp.vx + (*sc_width);
				pos->vy = fvtemp.vy + (*sc_height);
				pos->vz = fvtemp.vz;
				pos++;

				if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
				{
				if( sc_pers->vy > 2.0f ) sc_pers->vz=0.0f;
				}
				else
				{
				sc_pers->vy  += SC_DOWN;
				if( sc_pers->vy > 2.0f ) sc_pers->vz=0.0f;
				(*sc_height) += SC_DOWN * SCREEN_NEAR * 2.0f;
				}
//if(i==0 && j==0) printf("%f\n",(*sc_height));
			}else{
				DG_COPY_VEC( pos++, &DG_ZeroVector );
				DG_COPY_VEC( pos++, &DG_ZeroVector );
				DG_COPY_VEC( pos++, &DG_ZeroVector );
				DG_COPY_VEC( pos++, &DG_ZeroVector );
			}
			sc_width++;
			sc_height++;
			sc_pers++;
		}
	}
}

static void Die( Work *work )
{
	int	i;

	for( i=0; i<MAX_TEX; i++ ){
		work->unit[i].prim = OK_FreePrim2( work->unit[i].prim );
	}
	OK_BLOOD_CM_WORK = NULL;
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int		i;
	int		u0,u1,v0,v1;
	int		du0=0;
	int		du1=0;
	int		dv0=0;
	int		dv1=0;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	u0 = FTOI12( tex->u_offset );
	v0 = FTOI12( tex->v_offset );
	u1 = FTOI12( tex->u_scale + tex->u_offset );
	v1 = FTOI12( tex->v_scale + tex->v_offset );

	pos     = SCR_POS0 ;
	uvrgb   = SCR_UVS0 ;
	for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );

		switch(i%4){
		  case 0:
			du0 = u0;
			dv0 = v0;
			du1 = u1;
			dv1 = v1;
			break;
		  case 1:
			du0 = u1;
			dv0 = v1;
			du1 = u0;
			dv1 = v0;
			break;
		  case 2:
			du0 = u1;
			dv0 = v0;
			du1 = u0;
			dv1 = v1;
			break;
		  case 3:
			du0 = u0;
			dv0 = v1;
			du1 = u1;
			dv1 = v0;
			break;
		}


		uvrgb->u = du0;
		uvrgb->v = dv0;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = COL_A ;
		uvrgb++;

		uvrgb->u = du1;
		uvrgb->v = dv0;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = COL_A ;
		uvrgb++;

		uvrgb->u = du0;
		uvrgb->v = dv1;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = COL_A ;
		uvrgb++;

		uvrgb->u = du1;
		uvrgb->v = dv1;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = COL_A ;
		uvrgb++;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS0, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS0, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS ) ;

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	int	i,j;
	DG_PRIM2		*prim ;
	DG_TEX		*tex[MAX_TEX] ;

#if 1
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	tex[0] = DG_GetTexture(  9373287 /*"vr2_chi01_alp"*/ );
	tex[1] = DG_GetTexture( 10421863 /*"vr2_chi02_alp"*/ );
	tex[2] = DG_GetTexture( 11470439 /*"vr2_chi03_alp"*/ );
	tex[3] = DG_GetTexture( 12519015 /*"vr2_chi04_alp"*/ );
	tex[4] = DG_GetTexture( 13567591 /*"vr2_chi05_alp"*/ );
	}
	else
	{
	tex[0] = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	tex[1] = DG_GetTexture( 12033390 /*"chi02_msk"*/ );
	tex[2] = DG_GetTexture( 13081966 /*"chi03_msk"*/ );
	tex[3] = DG_GetTexture( 14130542 /*"chi04_msk"*/ );
	tex[4] = DG_GetTexture( 15179118 /*"chi05_msk"*/ );
	}
#else
	tex[0] = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	tex[1] = DG_GetTexture( 12020883 /*"chi02_alp"*/ );
	tex[2] = DG_GetTexture( 13069459 /*"chi03_alp"*/ );
	tex[3] = DG_GetTexture( 14118035 /*"chi04_alp"*/ );
	tex[4] = DG_GetTexture( 15166611 /*"chi05_alp"*/ );
#endif

//printf("nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn\n");


	for( i=0; i<MAX_TEX; i++ ){
//		prim = work->unit[i].prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS, N_VERTS );
		prim = work->unit[i].prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS, N_VERTS );
//		prim = work->unit[i].prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS, N_VERTS );

		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacket2( work, prim, tex[i] );
//		prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

		for(j=0; j<N_PRIMS*N_POLYS; j++){
			work->unit[i].sc_pers[j].vx = 0.0f;
			work->unit[i].sc_pers[j].vy = 0.0f;
			work->unit[i].sc_pers[j].vz = 0.0f;
		}
	}

	work->prim_count=0;

	return 0 ;
}

void PutCameraBlood( FVECTOR *pos_pers, int white )
{
	Work	*work;
	int	num0;
	int	num1;
	float	ftemp;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return;

	num0 = PL_CurrentItem();
	if( (num0 == IT_DummyScope)
	 || (num0 == IT_NightVision)
	 || (num0 == IT_Thermal)
	 || (num0 == IT_Scope)
	 || (num0 == IT_Camera)
	  ) return;

	num0 = PL_CurrentWeapon();
	if( (num0 == WP_Psg1)
	 || (num0 == WP_Nikita)
	 || (num0 == WP_Stinger)
	 || (num0 == WP_Psg1T)
	  ) return;

//	if(white) return;

	if( OK_BLOOD_CM_WORK==(void *)NULL ){
		if( NewBloodCamera()==(void *)1 ){
			printf("OK_BLOOD_CM_WORK was NULL\n");
			return;
		}
	}
	work = OK_BLOOD_CM_WORK;

	num0 = work->prim_count % MAX_TEX;
	num1 = work->prim_count / MAX_TEX;
	if( work->unit[num0].sc_pers[num1].vz > 1.0f ){
		work->prim_count++;
		if( work->prim_count >= MAX_TEX*N_PRIMS*N_POLYS ) work->prim_count=0;
		return;
	}

//printf("%d::%d::%d\n",num0,num1,work->prim_count);
#if 0
	work->unit[num0].sc_pers[num1].vx = frnd();
	work->unit[num0].sc_pers[num1].vy = -rnd();
	work->unit[num0].sc_pers[num1].vz = SCREEN_NEAR;
#else
	work->unit[num0].sc_pers[num1].vx = pos_pers->vx + frnd()*0.25f;
	work->unit[num0].sc_pers[num1].vy = pos_pers->vy -  rnd()*0.25f;
	work->unit[num0].sc_pers[num1].vz = SCREEN_NEAR;
#endif


	ftemp = MAX_SIZE - rnd()*SIZE_RANDAM;
	work->unit[num0].sc_width[ num1] = ftemp;
	work->unit[num0].sc_height[num1] = ftemp * 2.0f;

	work->prim_count++;
	if( work->prim_count >= MAX_TEX*N_PRIMS*N_POLYS ) work->prim_count=0;


	if( pos_pers->vx <  1.0f
	 && pos_pers->vx > -1.0f
	 && pos_pers->vy <  1.0f
	 && pos_pers->vy > -1.0f ){
		GM_SeSetMode( SD_E_BLOOD_S1 , (FVECTOR *)DG_Chanls->eye.m[3], GM_SEMODE_NORMAL );	/* 血付着音 */
	}

}


void *NewBloodCamera( void )
{
	Work		*work ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return (void *)1;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		OK_BLOOD_CM_WORK=work;
	}
	return (void *)work ;
}
