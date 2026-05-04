//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_splash_floor.c
	床用水飛沫（シナリオ起動）
	2000/04/09 S.Okajima
	$Id: d_splash_floor.c,v 1.1.1.3 2002/11/19 11:46:55 Yoshizawa1 Exp $
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

#define	SIZE_MIN	(10.0f)

#define	WIN_MIN			(200.0f)


#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)

#define	BASE_RGB		(255)
#define	MAX_ALPHA		(80)

#define	COUNT_MIN		(8)
#define	COUNT_RND		(8)

#define	HEIGHT			(50.0f)
#define	SIZE			(300.0f)

#define	ANGLE_LOWER		(64)
#define	ANGLE_UPPER		(256)

#define	CAMERA_BACK_LENGTH		(-1000.0f)
#define	CAMERA_FRONT_LENGTH		(10000.0f)

extern	FVECTOR G_wind;
extern	int		G_wind_intense;
extern	int		G_wind_intense_max;
extern	SVECTOR	G_wind_rot;
extern	int		G_wind_sw;
extern	SVECTOR	OK_rain_fall_rot;

/*----------------------------------------------------------------*/
extern void *NewSplashParts_Demo( FVECTOR *center, SVECTOR *rot, float intense );

/*----------------------------------------------------------------*/
#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_POS_CAL		(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#define	SCR_W0			(SCRPAD_ADDR + 0x3f00)
#define	SCR_W1			(SCRPAD_ADDR + 0x3f10)
#define	SCR_W2			(SCRPAD_ADDR + 0x3f20)
#define	SCR_W3			(SCRPAD_ADDR + 0x3f30)
#define	SCR_W4			(SCRPAD_ADDR + 0x3f40)
#define	SCR_W5			(SCRPAD_ADDR + 0x3f50)
#define	SCR_W6			(SCRPAD_ADDR + 0x3f60)
#define	SCR_W7			(SCRPAD_ADDR + 0x3f70)
#define	SCR_W8			(SCRPAD_ADDR + 0x3f80)
#define	SCR_W9			(SCRPAD_ADDR + 0x3f90)
#define	SCR_Wa			(SCRPAD_ADDR + 0x3fa0)
#define	SCR_Wb			(SCRPAD_ADDR + 0x3fb0)

#define	scw_rot		((SVECTOR*)SCR_W0)
#define	scw_center	((FVECTOR*)SCR_W1)
#define	scw_fvzero	((FVECTOR*)SCR_W2)
#define	scw_fvtemp0	((FVECTOR*)SCR_W3)
#define	scw_fvtemp1	((FVECTOR*)SCR_W4)
#define	scw_fvtemp2	((FVECTOR*)SCR_W5)
#define	scw_fvcalc0	((FVECTOR*)SCR_W6)


typedef	struct	{
	int			count;
	int			count_max;
	FVECTOR		vec[N_VERTS];
} Unit ;

typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;
	int			where;

	float		wind_intense_ratio;
	int			upper_angle;

	DG_PRIM2	*prim ;

	FVECTOR		bound0;
	FVECTOR		bound1;
	FVECTOR		diff;
	SVECTOR		vec_rot;
	SVECTOR		rot;

	int			invisible_flag;

	int			repeat_num;
	Unit		unit[0];
} Work ;

/*----------------------------------------------------------------*/
/* 座標計算データを初期化する */
static	void	InitVectors( Work *work, int prim_num, int clock )
{
	int		i;
	int		num;
	float	len;
	FVECTOR	*vec;
	FVECTOR	*pos,*tmp;
	DG_PRIM2_UVRGB	*uvrgb ;

	DG_COPY_VEC( scw_fvzero,  &DG_ZeroVector );
	DG_COPY_VEC( scw_fvtemp0, &work->bound0 );
	DG_COPY_VEC( scw_fvtemp1, &work->diff );

	work->unit[prim_num].count_max = work->unit[prim_num].count = COUNT_MIN + irnd()%COUNT_RND;

	num   = prim_num*N_VERTS;
	uvrgb = work->prim->uvrgb[ clock ];
	uvrgb+= num;

   pos   = work->prim->pos[ clock ];
   pos  += num;

   vec   = work->unit[prim_num].vec;

   for( i=0; i<N_POLYS; i++ )
   {
		scw_center->vx = scw_fvtemp0->vx + scw_fvtemp1->vx * rnd();
		scw_center->vy = scw_fvtemp0->vy;
		scw_center->vz = scw_fvtemp0->vz + scw_fvtemp1->vz * rnd();
		DG_COPY_VEC( pos, scw_center );	pos++;
		DG_COPY_VEC( pos, scw_center );	pos++;
		DG_COPY_VEC( pos, scw_center );	pos++;
		DG_COPY_VEC( pos, scw_center );	pos++;

      len   = -SIZE * (rnd() + 1.0f);
		scw_fvtemp2->vx = len;
		scw_fvtemp2->vy = len * 0.55f;
		scw_fvtemp2->vz = len * 0.25f;
		scw_fvtemp2->vw = len * 0.10f;

      tmp = scw_fvcalc0 ;
		//vf8
		tmp->vx = 0.0f; tmp->vy = 0.0f; tmp->vz = scw_fvtemp2->vx;
		tmp++;
		//vf9
		tmp->vx = scw_fvtemp2->vz; tmp->vy = 0.0f; tmp->vz = scw_fvtemp2->vy;
		tmp++;
		//vf10
		tmp->vx = -scw_fvtemp2->vz; tmp->vy = 0.0f; tmp->vz = scw_fvtemp2->vy;
		tmp++;
		//vf11
		tmp->vx = 0.0f; tmp->vy = 0.0f; tmp->vz = scw_fvtemp2->vw;

		scw_rot->vx = work->vec_rot.vx + irnd()%work->upper_angle  + 128;
		scw_rot->vy = work->vec_rot.vy + (irnd()&1023) - 512;
		scw_rot->vz = irnd()&4095;
		DG_SetPos2( scw_fvzero, scw_rot );
		DG_RotVector( scw_fvcalc0, vec, 4 );

		uvrgb++;
		vec+=4;
	}
}

/*----------------------------------------------------------------*/
/* ポリゴンの座標データを更新する */
static	void	UpdateVectors( Work *work, int prim_num, int clock )
{
	int	j;
	int	num;
	FVECTOR			*d_pos;
	DG_PRIM2_UVRGB	*d_uvrgb ;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR			*vec;
	u_char	col;


	col=(u_char)( work->unit[prim_num].count * MAX_ALPHA / work->unit[prim_num].count_max );
	work->unit[prim_num].count--;

	num     = prim_num*N_VERTS;
	d_pos   = work->prim->pos[1-clock];
	d_pos  += num;
   pos     = work->prim->pos[clock];
   pos    += num;
	uvrgb   = work->prim->uvrgb[clock];
	uvrgb  += num;
	vec     = work->unit[prim_num].vec;
   memcpy(pos, d_pos, 4*N_POLYS*sizeof(FVECTOR));
	DG_SetPos( &DG_Chanls->eye_inv );
	for( j=0; j<N_POLYS; j++ )
   {
		DG_PutVector( pos, scw_fvtemp2, 1 );

		if( scw_fvtemp2->vz < CAMERA_BACK_LENGTH
		 || scw_fvtemp2->vz > CAMERA_FRONT_LENGTH )
      {
         uvrgb->a = 0;	uvrgb++;
         uvrgb->a = 0;	uvrgb++;
         uvrgb->a = 0;	uvrgb++;
         uvrgb->a = 0;	uvrgb++;

         pos[3].vx = pos[2].vx = pos[1].vx = pos[0].vx;
         pos[3].vy = pos[2].vy = pos[1].vy = pos[0].vy;
         pos[3].vz = pos[2].vz = pos[1].vz = pos[0].vz;
		}
      else
      {
         uvrgb->a = col;	uvrgb++;
         uvrgb->a = col;	uvrgb++;
         uvrgb->a = col;	uvrgb++;
         uvrgb->a = col;	uvrgb++;

         pos[0].vx += vec[0].vx;
         pos[0].vy += vec[0].vy;
         pos[0].vz += vec[0].vz;

         pos[1].vx += vec[1].vx;
         pos[1].vy += vec[1].vy;
         pos[1].vz += vec[1].vz;

         pos[2].vx += vec[2].vx;
         pos[2].vy += vec[2].vy;
         pos[2].vz += vec[2].vz;

         pos[3].vx += vec[3].vx;
         pos[3].vy += vec[3].vy;
         pos[3].vz += vec[3].vz;
      }
      pos += 4;
      vec += 4;
	}
}

enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_PARAM,
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
		  case REQ_OFF:
			work->invisible_flag = 1;
			break;
		  case REQ_ON:
			work->invisible_flag = 0;
			break;
		  case REQ_KILL:
		  case GV_MESSAGE_KILL:
			GV_DestroyActor( work ) ;
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
	int		i;
	int		clock;
	Unit	*unit;

	CheckMesgParam( work );

	if( work->invisible_flag ){
		DG_InvisiblePrim2( work->prim );
		return;
	}else{
		DG_VisiblePrim2( work->prim );
	}

	scw_fvtemp0->vx = 0.0f;
	scw_fvtemp0->vz = 0.0f;
	scw_fvtemp0->vy = 4096.0f;
	OK_DirectionSmoother( &work->rot, &G_wind_rot, 0.993f );
	DG_SetPos2( &DG_ZeroVector, &work->rot );
	DG_PutVector( scw_fvtemp0, scw_fvtemp0, 1 );
	scw_fvtemp0->vy = -HEIGHT;
	OK_DirVecXY( &DG_ZeroVector, scw_fvtemp0, &work->vec_rot );
#if 0
	// 多分ちゃんと動いていないのですが、怖いので安全方面に修正。
	if( G_wind_intense_max==0 ){
		work->wind_intense_ratio = (float)G_wind_intense / (float)G_wind_intense_max;
	}else{
		work->wind_intense_ratio = 0.5f;
	}
#else
	work->wind_intense_ratio = 0.5f;
#endif
	work->wind_intense_ratio = ( work->wind_intense_ratio > 0 )?work->wind_intense_ratio:-work->wind_intense_ratio;
	work->upper_angle = (int)((float)ANGLE_UPPER * work->wind_intense_ratio);
	if( work->upper_angle < ANGLE_LOWER ) work->upper_angle = ANGLE_LOWER;
	if( work->upper_angle > ANGLE_UPPER ) work->upper_angle = ANGLE_UPPER;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;
	unit = work->unit;
	for ( i = 0 ; i < work->repeat_num ; i++ ){
		if( unit->count==0 ){
			InitVectors( work, i, clock );
		}else{
			UpdateVectors( work, i, clock );
		}
		unit++;
	}
}


static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


/* 初期設定値を取得 */
static	int	GetOptionValue( Work *work )
{
	if ( GCL_GetOption( 'b' ) != NULL ){
		scw_fvtemp0->vx = (float)GCL_GetNextInt();
		scw_fvtemp0->vy = (float)GCL_GetNextInt();
		scw_fvtemp0->vz = (float)GCL_GetNextInt();
		scw_fvtemp1->vx = (float)GCL_GetNextInt();
		scw_fvtemp1->vy = (float)GCL_GetNextInt();
		scw_fvtemp1->vz = (float)GCL_GetNextInt();
	}else{
		return -1;
	}
	if( scw_fvtemp0->vx < scw_fvtemp1->vx ){
		work->bound0.vx = scw_fvtemp0->vx;
		work->bound1.vx = scw_fvtemp1->vx;
	}else{
		work->bound0.vx = scw_fvtemp1->vx;
		work->bound1.vx = scw_fvtemp0->vx;
	}
	if( scw_fvtemp0->vy < scw_fvtemp1->vy ){
		work->bound0.vy = scw_fvtemp0->vy;
		work->bound1.vy = scw_fvtemp1->vy;
	}else{
		work->bound0.vy = scw_fvtemp1->vy;
		work->bound1.vy = scw_fvtemp0->vy;
	}
	if( scw_fvtemp0->vz < scw_fvtemp1->vz ){
		work->bound0.vz = scw_fvtemp0->vz;
		work->bound1.vz = scw_fvtemp1->vz;
	}else{
		work->bound0.vz = scw_fvtemp1->vz;
		work->bound1.vz = scw_fvtemp0->vz;
	}

	work->diff.vx = work->bound1.vx - work->bound0.vx;
	work->diff.vy = work->bound1.vy - work->bound0.vy;
	work->diff.vz = work->bound1.vz - work->bound0.vz;
	return 0;
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0 ;
	DG_PRIM2_UVRGB	*uvrgb1 ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	uvrgb0 = prim->uvrgb[ 0 ] ;
	uvrgb1 = prim->uvrgb[ 1 ] ;
	for ( i = 0 ; i < work->repeat_num ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB ;
			uvrgb1->g = uvrgb0->g = BASE_RGB ;
			uvrgb1->b = uvrgb0->b = BASE_RGB ;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB ;
			uvrgb1->g = uvrgb0->g = BASE_RGB ;
			uvrgb1->b = uvrgb0->b = BASE_RGB ;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB ;
			uvrgb1->g = uvrgb0->g = BASE_RGB ;
			uvrgb1->b = uvrgb0->b = BASE_RGB ;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB ;
			uvrgb1->g = uvrgb0->g = BASE_RGB ;
			uvrgb1->b = uvrgb0->b = BASE_RGB ;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;
		}
	}


	return 1;
}

static int GetResources( Work *work, int name, int where )
{
	int		i;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->name  = name;
	work->where = where;

	work->invisible_flag = 0;

	work->upper_angle = ANGLE_LOWER;

	if( GetOptionValue( work ) < 0 ) return -1;

	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 5455955 /*"rcm_l"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->repeat_num, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	for( i=0; i<work->repeat_num; i++ ){
		InitVectors( work, i, 0 );
		InitVectors( work, i, 1 );
	}
	DG_VisiblePrim2( work->prim ) ;

	return 0 ;
}

void *NewSplashFloor_Scn( int name, int where )
{
	Work		*work ;
	int			buf_size;
	int			unit_num;


	OPERATOR() ;

	unit_num=1;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		unit_num = GCL_GetNextInt() ;
	}

	buf_size = sizeof( Work ) + sizeof( Unit ) * unit_num;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->repeat_num = unit_num;

		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
