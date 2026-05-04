//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_surface_gravity_man.c
	水飛沫制御

	2001/02/27 S.Okajima
	$Id: splush_surface_gravity_man.c,v 1.1.1.3 2002/11/19 11:47:25 Yoshizawa1 Exp $
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

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"./bubble.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
void *NewSplushSurface2Man( int name, int map );
/*----------------------------------------------------------------*/

#define	RAISE			(0)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_VEC		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)

//#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
//#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(8)
//以上でスクラッチパッドぎりぎり
#define	N_LOOPS		(32)

#define	TOTAL_PARTS		(N_LOOPS*N_PRIMS*N_POLYS)	/* 水飛沫の個数 */


#define	PART_LIFE		(64)		/* 一つの水飛沫の寿命 */
#define	MAX_SPLUSH		(TOTAL_PARTS/PART_LIFE)	/* 一フレーム内に処理できる最大 */


#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	SUB_ALPHA	(4)			/* 寿命に影響 */
#define	MAX_ALPHA	(SUB_ALPHA * PART_LIFE)

#define	BACK_SCALE	(8.0f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			activate_num;

	FVECTOR		vec[N_LOOPS*N_PRIMS*N_VERTS/2];	// 頂点速度は随時計算
	DG_PRIM2	*prim ;
} Work ;

static Work *OK_SplushSurface2ManWork = NULL;
static int	OK_SplushSurface2Count=0;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	FVECTOR			*vec;
	FVECTOR			*fvptemp;
	FVECTOR			*pos;
	FVECTOR			*pos_before;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*uvrgb_before;
	int		i;
	int		clock;
	int		alpha;
	int		flag;
	FVECTOR			fv_grav0;
	FVECTOR			fv_grav1;

	OK_SplushSurface2Count=0;

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	 //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	pos          = prim->pos[  clock];
	pos_before   = prim->pos[1-clock];
	uvrgb        = prim->uvrgb[  clock];
	uvrgb_before = prim->uvrgb[1-clock];
	vec          = work->vec;

	fv_grav0.vx = P_GRAVITY;
	fv_grav0.vy = P_GRAVITY;
	fv_grav0.vz = P_GRAVITY;
	fv_grav0.vw = P_GRAVITY;
	fv_grav1.vx = BACK_SCALE;
	fv_grav1.vy = BACK_SCALE;
	fv_grav1.vz = BACK_SCALE;
	fv_grav1.vw = BACK_SCALE;
#ifdef BP_PSX2_ASM	
	asm volatile ("
		lqc2		vf10,0x00(%0)
		lqc2		vf9, 0x00(%1)
		"::"r"(&fv_grav0),"r"(&fv_grav1):"memory"
	);
#endif	

	flag = 0;
	for ( i = 0 ; i < TOTAL_PARTS ; i++ ){
//		if( uvrgb_before->a != 0 ){
		if( vec->vw > 0.0f ){
			flag = 1;
			OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), 4);
			fvptemp = SCR_POS;

			if( (uvrgb_before)->a < SUB_ALPHA ){
				uvrgb_before += 4;
				(uvrgb++)->a = 0;
				(uvrgb++)->a = 0 ;
				(uvrgb++)->a = 0 ;
				(uvrgb++)->a = 0 ;
				vec->vw = -1.0f;
			}else{
				alpha = (uvrgb_before)->a - SUB_ALPHA;
				uvrgb_before += 4;
				(uvrgb++)->a = alpha;
				(uvrgb++)->a = alpha ;
				(uvrgb++)->a = alpha ;
				(uvrgb++)->a = alpha ;
			}

#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf11,0x00(%0)
				lqc2		vf12,0x00(%1)
				lqc2		vf13,0x10(%0)
				lqc2		vf14,0x10(%1)

				vadd.xyz	vf11, vf11, vf12
				vadd.xyz	vf13, vf13, vf14

				vadd.y		vf12, vf12, vf10
				vadd.y		vf14, vf14, vf10

				sqc2		vf11,0x00(%0)
				sqc2		vf13,0x10(%0)
				sqc2		vf12,0x00(%1)
				sqc2		vf14,0x10(%1)

				vsubw.y		vf11, vf11, vf11
				vsubw.y		vf13, vf13, vf11

				sqc2		vf11,0x20(%0)
				sqc2		vf13,0x30(%0)
				"::"r"(fvptemp),"r"(vec):"memory"
			);
#else
			BP_Vec3_AddVec( fvptemp,  fvptemp,  vec ); 
			BP_Vec3_AddVec( (fvptemp + 1),  (fvptemp + 1),  (vec + 1) ); 
			vec->vy += P_GRAVITY;
			(vec + 1)->vy += P_GRAVITY;

			*(fvptemp + 2) = *fvptemp;
			(fvptemp + 2)->vy -= fvptemp->vw;
			*(fvptemp + 3) = *(fvptemp + 1);
			(fvptemp + 3)->vy -= fvptemp->vw;

#endif			
			OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), 4);

			pos->vy = (pos->vy > GM_WaterLevel)? pos->vy: GM_WaterLevel;	pos++;
			pos->vy = (pos->vy > GM_WaterLevel)? pos->vy: GM_WaterLevel;	pos++;
			pos->vy = (pos->vy > GM_WaterLevel)? pos->vy: GM_WaterLevel;	pos++;
			pos->vy = (pos->vy > GM_WaterLevel)? pos->vy: GM_WaterLevel;	pos++;

			vec        += 2;
//			pos        += 4;
			pos_before += 4;
		}else{
#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf11,0x00(%0)
				sqc2		vf11,0x10(%0)
				sqc2		vf11,0x20(%0)
				sqc2		vf11,0x30(%0)
				"::"r"(pos):"memory"
			);
#else
			*(pos + 3) = *(pos + 2) = *(pos + 1) = *pos;
#endif
			vec          += 2;
			pos          += 4;
			uvrgb        += 4;
			uvrgb_before += 4;
			pos_before   += 4;
		}
	}

	if( !flag ) GV_DestroyActor( work ) ;
}

/*----------------------------------------------------------------*/
static	void	CulcVector( Work *work, FVECTOR *center, FVECTOR *force, float intense, int input_alpha )
{
	DG_PRIM2		*prim ;
	FVECTOR			fvtemp;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGB	*uvrgb;
	int		alpha;
	int		clock;
	int		num;

	prim = work->prim;
	clock = prim->buffer_clock;
#if 0
	if( work->act_clock == (GV_Time&1) ){	// 現在のフレームact処理前にここに来た
		clock = 1 - clock;	// 変更すべきフレームデータは裏である
	}
#endif

//	len = GV_VecLen3F( force );

	fvtemp.vx = force->vz;
	fvtemp.vy = 0.0f;
	fvtemp.vz = force->vx;
	fvtemp.vw = 1.0f;
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, intense*0.125f );

	vec   = work->vec;
	pos   = prim->pos[clock];
	uvrgb = prim->uvrgb[clock];

	num = work->activate_num * 2;
	vec  += num;
	num  *= 2;

	pos  += num;
	uvrgb+= num;

	vec->vx = (force->vx - fvtemp.vx);
	vec->vy = force->vy              ;
	vec->vz = (force->vz + fvtemp.vz);
	vec->vw = PART_LIFE;
	vec++;

	vec->vx = (force->vx + fvtemp.vx);
	vec->vy = force->vy              ;
	vec->vz = (force->vz - fvtemp.vz);
//	vec++;

	DG_COPY_VEC( pos, center );
	pos->vw = intense * (8.0f + 8.0f*rnd());
	pos++;
	DG_COPY_VEC( pos, center );
	pos++;
	DG_COPY_VEC( pos, center );
	pos++;
	DG_COPY_VEC( pos, center );
	pos++;

	if( input_alpha==0 ){
		alpha = MAX_ALPHA/2 + (irnd()>>8)%MAX_ALPHA/2;
	}else{
		alpha = input_alpha/2 + (irnd()>>8)%MAX_ALPHA/2;
	}

	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;

	work->activate_num++;
	work->activate_num = (work->activate_num < TOTAL_PARTS)? work->activate_num: 0;
}

/*----------------------------------------------------------------*/
int	OK_PutSplushSurface2( FVECTOR *center, FVECTOR *force, float intense )
{
	Work			*work;

	if( OK_SplushSurface2Count++ >= MAX_SPLUSH ) return 0;
	if( OK_SplushSurface2ManWork==NULL ){
		NewSplushSurface2Man( 1, 1 );
		if( OK_SplushSurface2ManWork==NULL ) return 0;
	}
	work = OK_SplushSurface2ManWork;

	CulcVector( work, center, force, intense, 0 );

	return 1;
}

/*----------------------------------------------------------------*/
int	OK_PutSplushSurface2_Alpha( FVECTOR *center, FVECTOR *force, float intense, int alpha )
{
	Work			*work;

	if( OK_SplushSurface2Count++ >= MAX_SPLUSH ) return 0;
	if( OK_SplushSurface2ManWork==NULL ){
		NewSplushSurface2Man( 1, 1 );
		if( OK_SplushSurface2ManWork==NULL ) return 0;
	}
	work = OK_SplushSurface2ManWork;

	CulcVector( work, center, force, intense, alpha&255 );

	return 1;
}

/*----------------------------------------------------------------*/



static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_SplushSurface2Count = 0;
	OK_SplushSurface2ManWork = NULL;
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos;
	FVECTOR			*dest_pos0;
	FVECTOR			*dest_pos1;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*dest_uvrgb0;
	DG_PRIM2_UVRGB	*dest_uvrgb1;
	int		i, j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	dest_pos0   = prim->pos[ 0 ];
	dest_pos1   = prim->pos[ 1 ];
	dest_uvrgb0 = prim->uvrgb[ 0 ];
	dest_uvrgb1 = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS ; i++ ){
		pos   = SCR_POS;
		uvrgb = SCR_UVR;
		for ( j = 0 ; j < N_PRIMS ; j++ ){
			for ( k = 0 ; k < N_POLYS ; k++ ){
				DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
				DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
				DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
				DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x8fff ;
				uvrgb->r = COL_R ;
				uvrgb->g = COL_G ;
				uvrgb->b = COL_B ;
				uvrgb->a = MAX_ALPHA ;
				uvrgb++;

				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x8fff ;
				uvrgb->r = COL_R ;
				uvrgb->g = COL_G ;
				uvrgb->b = COL_B ;
				uvrgb->a = MAX_ALPHA ;
				uvrgb++;

				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x0fff ;
				uvrgb->r = COL_R ;
				uvrgb->g = COL_G ;
				uvrgb->b = COL_B ;
				uvrgb->a = MAX_ALPHA ;
				uvrgb++;

				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x0fff ;
				uvrgb->r = COL_R ;
				uvrgb->g = COL_G ;
				uvrgb->b = COL_B ;
				uvrgb->a = MAX_ALPHA ;
				uvrgb++;
			}
		}
		OK_Scr_Mem( dest_pos0,   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_pos1,   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb0, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb1, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		dest_pos0  += N_PRIMS*N_VERTS;
		dest_pos1  += N_PRIMS*N_VERTS;
		dest_uvrgb0+= N_PRIMS*N_VERTS;
		dest_uvrgb1+= N_PRIMS*N_VERTS;
	}
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS*N_LOOPS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	work->activate_num = 0;

	return 0 ;
}

void *NewSplushSurface2Man( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map  = map;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

		OK_SplushSurface2ManWork = work;

	}
	return (void *)work ;
}
