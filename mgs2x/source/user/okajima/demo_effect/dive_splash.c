//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dive_splash.c
	ドボン水飛沫
	2000/09/26 S.Okajima
	$Id: dive_splash.c,v 1.1.1.3 2002/11/19 11:46:56 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libmt.h"
#include        "libutl.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"


/*----------------------------------------------------------------*/

#define	SCR_LENGTH		( 0x4000 )

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(4)

#define	MAX_LENGTH		(150.0f)

#define	BASE_RGB	(96)
#define	BASE_R		(BASE_RGB + 0)
#define	BASE_G		(BASE_RGB + 16)
#define	BASE_B		(BASE_RGB + 32)
#define	MAX_ALPHA		(128)

#define	LIFE_TIME		(120)

#define	SCALE			(80.0f)
#define	WIDTH			(5.0f)
#define	UPPER			(10.0f)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#define	OUTER_SPEED		(50.0f)

typedef	struct	{
	GV_ACT_EX		actor ;

	int			pattern;
	int			flag;

	DG_PRIM2	*prim ;
	FVECTOR		center;

	int			life;
	FVECTOR		vec[N_PRIMS*N_VERTS] ;

} Work ;

/* 座標計算データを初期化する */
static	void	InitVectors( Work *work )
{
	int	i,j;
	int		iparam0;
	int		iparam1;
	float	fparam0;
	float	fparam1;
	float	fparam2;
	float	ftemp0;
	float	ftemp1;
	float	ftemp2;
	float	len;
	float	shift;
	SVECTOR	rot;
	FVECTOR	fvtemp;
	FVECTOR	*pos;
	FVECTOR	*vec;

	if( work->flag ){
		shift = OUTER_SPEED;
	}else{
		shift = 0.0f;
	}


	switch( work->pattern ){
	  case 0:
	  default:
		iparam0 = 16;
		iparam1 = 32;
		fparam0 = 0.75f;
		fparam1 = 0.05f;
		fparam2 = 0.25f;
		break;
	  case 1:
		iparam0 = 128;
		iparam1 = 512;

		fparam0 = 0.35f;
		fparam1 = 0.10f;
		fparam2 = 0.00f;
		break;
	}

	pos = SCR_POS;
	vec = work->vec;
	for( j=0; j<N_PRIMS; j++ ){
		for( i=0; i<N_POLYS; i++ ){
			fvtemp.vx = work->center.vx + MAX_LENGTH * frnd();
			fvtemp.vy = work->center.vy + MAX_LENGTH * frnd();
			fvtemp.vz = work->center.vz + MAX_LENGTH * frnd();

			DG_COPY_VEC( pos, &fvtemp );	pos++;
			DG_COPY_VEC( pos, &fvtemp );	pos++;
			DG_COPY_VEC( pos, &fvtemp );	pos++;
			DG_COPY_VEC( pos, &fvtemp );	pos++;

			len    = MAX_LENGTH * ( rnd()*0.99f + 0.01f);
			ftemp0 = len * fparam0;
			ftemp1 = len * fparam1;
			ftemp2 = len * fparam2;

			vec[0].vx = 0.0f;
			vec[0].vy = 0.0f;
			vec[0].vz = len;

			vec[1].vx = ftemp1;
			vec[1].vy = 0.0f;
			vec[1].vz = ftemp0;

			vec[2].vx =-ftemp1;
			vec[2].vy = 0.0f;
			vec[2].vz = ftemp0;

			vec[3].vx = 0.0f;
			vec[3].vy = 0.0f;
			vec[3].vz = ftemp2;

			rot.vx = -(1024 + iparam0 + irnd()%iparam1);
			rot.vy = irnd()%4096;
			rot.vz = irnd()%4096;
			DG_SetPos2( &DG_ZeroVector, &rot );
			DG_RotVector( vec, vec, 4 );

			vec->vz += shift;
			(vec++)->vw = P_GRAVITY;	/* 後で使用 */
			vec->vz += shift;
			(vec++)->vw = P_GRAVITY;	/* 後で使用 */
			vec->vz += shift;
			(vec++)->vw = P_GRAVITY;	/* 後で使用 */
			vec->vz += shift;
			(vec++)->vw = P_GRAVITY;	/* 後で使用 */
//printf("%f %f %f \n",vec->vx,vec->vy,vec->vz);
		}
	}
	OK_Scr_Mem( work->prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( work->prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);

}


/* ポリゴンの座標データを更新する */
static	void	UpdateVectors( Work *work )
{
	int	i,j;
	FVECTOR	*pos;
	FVECTOR	*vec;
	DG_PRIM2_UVRGB	*uvrgb_r ;
	DG_PRIM2_UVRGB	*uvrgb ;
	u_char	col;
	int	clock;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	col=(u_char)( work->life * MAX_ALPHA/LIFE_TIME );
	for( j=0; j<N_PRIMS; j++ ){
		OK_Mem_Scr( SCR_POS, (void *)&work->prim->pos[1-clock][N_VERTS*j], sizeof(FVECTOR),        N_VERTS);
		OK_Mem_Scr( SCR_VEC, (void *)&work->vec[N_VERTS*j],                sizeof(FVECTOR),        N_VERTS);
		uvrgb_r = work->prim->uvrgb[clock];
		uvrgb_r += N_VERTS*j;
		OK_Mem_Scr( SCR_UVS, uvrgb_r, sizeof(DG_PRIM2_UVRGB), N_VERTS);

		pos   = SCR_POS;
		vec   = SCR_VEC;
		uvrgb = SCR_UVS;
		for( i=0; i<N_POLYS; i++ ){
			/* α */
			uvrgb->a = col;	uvrgb++;
			uvrgb->a = col;	uvrgb++;
			uvrgb->a = col;	uvrgb++;
			uvrgb->a = col;	uvrgb++;

#if 0
			/* 座標更新 */
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			vec->vy += P_GRAVITY;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			vec->vy += P_GRAVITY;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			vec->vy += P_GRAVITY;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			vec->vy += P_GRAVITY;
			pos++;
			vec++;
#else
#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf8 ,0x00(%0)
				lqc2		vf9 ,0x10(%0)
				lqc2		vf10,0x20(%0)
				lqc2		vf11,0x30(%0)
				lqc2		vf12,0x00(%1)
				lqc2		vf13,0x10(%1)
				lqc2		vf14,0x20(%1)
				lqc2		vf15,0x30(%1)

				vadd.xyz	vf8, vf8, vf12
				vadd.xyz	vf9, vf9, vf13
				vadd.xyz	vf10,vf10,vf14
				vadd.xyz	vf11,vf11,vf15

				vaddw.y		vf12,vf12,vf12
				vaddw.y		vf13,vf13,vf13
				vaddw.y		vf14,vf14,vf14
				vaddw.y		vf15,vf15,vf15

				sqc2		vf8 ,0x00(%0)
				sqc2		vf9 ,0x10(%0)
				sqc2		vf10,0x20(%0)
				sqc2		vf11,0x30(%0)
				sqc2		vf12,0x00(%1)
				sqc2		vf13,0x10(%1)
				sqc2		vf14,0x20(%1)
				sqc2		vf15,0x30(%1)
				"::"r"(pos),"r"(vec):"memory"
			);
			pos += 4;
			vec += 4;
#else
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			vec->vy += vec->vw; //InitVectors()でP_GRAVITYがセットされてる;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			vec->vy += vec->vw;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			vec->vy += vec->vw;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			vec->vy += vec->vw;
			pos++;
			vec++;
#endif
#endif
		}
		OK_Scr_Mem( (void *)&work->vec[N_VERTS*j],                SCR_VEC, sizeof(FVECTOR),        N_VERTS);
		OK_Scr_Mem( (void *)&work->prim->pos[clock][N_VERTS*j],   SCR_POS, sizeof(FVECTOR),        N_VERTS);
		OK_Scr_Mem( uvrgb_r, SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_VERTS);
	}

}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	if( work->life > 0 ){
		GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
		UpdateVectors( work ) ;
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	uvrgb = SCR_UVS ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_R ;
			uvrgb->g = BASE_G ;
			uvrgb->b = BASE_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_R ;
			uvrgb->g = BASE_G ;
			uvrgb->b = BASE_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_R ;
			uvrgb->g = BASE_G ;
			uvrgb->b = BASE_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_R ;
			uvrgb->g = BASE_G ;
			uvrgb->b = BASE_B ;
			uvrgb->a = 0 ;
			uvrgb++;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	return 1;
}

static int GetResources( Work *work, FVECTOR *center, int pattern, int flag )
{
	DG_PRIM2		*prim ;
	DG_TEX		*tex ;


	work->flag = flag;

	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		GM_SeSetMode( SD_P_DOBOON01, center, GM_SEMODE_BOMB ) ;
	}

	work->life = LIFE_TIME;

	work->pattern = pattern;
	DG_COPY_VEC( &work->center, center );

	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	InitVectors( work );
	DG_VisiblePrim2( work->prim ) ;
	return 0 ;
}

/*
FVECTOR *center:発生場所
*/
void *NewDiveSplash( FVECTOR *center, int pattern )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, center, pattern, 1 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/*
FVECTOR *center:発生場所
*/
void *NewDiveSplash2( FVECTOR *center, int pattern )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, center, pattern, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/*
FVECTOR *center:発生場所
*/
void *NewDiveSplashScn( int name, int where )
{
	FVECTOR		center;
	int			flag;

	/* 設置座標 */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		center.vx = (float)GCL_GetNextInt() ;
		center.vy = (float)GCL_GetNextInt() ;
		center.vz = (float)GCL_GetNextInt() ;
		center.vw = 1.0f ;
	}

	flag = 0;
	if ( GCL_GetOption( 'f' ) != NULL ) {
		flag = GCL_GetNextInt();
	}

	if( flag ){
		NewDiveSplash( &center, 0 );
		NewDiveSplash( &center, 0 );
		NewDiveSplash( &center, 0 );
		NewDiveSplash( &center, 0 );
		NewDiveSplash( &center, 1 );
		NewDiveSplash( &center, 1 );
		NewDiveSplash( &center, 1 );
		NewDiveSplash( &center, 1 );
	}else{
		NewDiveSplash2( &center, 0 );
		NewDiveSplash2( &center, 0 );
		NewDiveSplash2( &center, 0 );
		NewDiveSplash2( &center, 0 );
		NewDiveSplash2( &center, 1 );
		NewDiveSplash2( &center, 1 );
		NewDiveSplash2( &center, 1 );
		NewDiveSplash2( &center, 1 );
	}
	return (void *)(1) ;
}

//add shibata 
//				call demo
typedef	struct	{
	GV_ACT_EX		actor ;
} Work_parent ;

static void Act_parent( Work_parent *work ){}
static void Die_parent( Work *work ){}

static int GetResources_parent( Work_parent *work, FVECTOR *pos )
{
	int		i;
	void	*p0;
	void	*p1;

	for( i = 0; i < 4; i++ ){
		p0 = NewDiveSplash( pos, 0 );
		p1 = NewDiveSplash( pos, 1 );
		if(p0) GV_SetActorChild(work,p0);
		if(p1) GV_SetActorChild(work,p1);
	}
	return 0;
}
void *NewDiveSplash_Parent( FVECTOR *center )
{
	Work_parent		*work ;

	work = (Work_parent *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work_parent ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act_parent, Die_parent ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources_parent( work, center ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

