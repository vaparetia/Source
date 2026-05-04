//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_surface_man.c
	水飛沫制御

	2001/02/27 S.Okajima
	$Id: splush_surface_man.c,v 1.1.1.3 2002/11/19 11:47:25 Yoshizawa1 Exp $
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
void *NewSplushSurfaceMan( int name, int map );
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
#define	N_PRIMS		(4)
//以上でスクラッチパッドぎりぎり
#define	N_LOOPS		(16)

//#define	N_PARTS		(N_POLYS/8)	/* 一つの水飛沫の中の枚数 */
#define	N_PARTS		(N_POLYS/16)	/* 一つの水飛沫の中の枚数 */

#define	TOTAL_PARTS		(N_LOOPS*N_PRIMS*N_POLYS/N_PARTS)	/* 水飛沫の個数 */


#define	PART_LIFE		(64)		/* 一つの水飛沫の寿命 */
#define	MAX_SPLUSH		(TOTAL_PARTS/PART_LIFE)	/* 一フレーム内に処理できる最大 */


#define	COL_R		(128)
#define	COL_G		(128)
#define	COL_B		(128)
#define	SUB_ALPHA	(4)			/* 寿命に影響 */
#define	MAX_ALPHA	(SUB_ALPHA * PART_LIFE)

#define	SURFACE_DECAY_VEC	(0.95f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			activate_num;

	FVECTOR		vec[N_LOOPS*N_PRIMS*N_VERTS];	// 頂点速度は随時計算
	FVECTOR		dummy;
	DG_PRIM2	*prim ;
} Work ;

static Work *OK_SplushSurfaceManWork = NULL;
static int	OK_SplushSurfaceCount=0;

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
	int		i, j ;
	int		clock;
	int		alpha;
	int		flag;
	int		surface_flag;
	FVECTOR			fv_grav;

	OK_SplushSurfaceCount=0;

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

	fv_grav.vx = P_GRAVITY;
	fv_grav.vy = P_GRAVITY;
	fv_grav.vz = P_GRAVITY;
	fv_grav.vw = P_GRAVITY;
#ifdef BP_PSX2_ASM	
	asm volatile ("
		lqc2		vf10,0x00(%0)
		"::"r"(&fv_grav):"memory"
	);
#endif	

	flag = 0;
	for ( i = 0 ; i < TOTAL_PARTS ; i++ ){
//		if( uvrgb_before->a != 0 ){
		if( vec->vw > 0.0f ){
			flag = 1;
			OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), N_PARTS*4);
			fvptemp = SCR_POS;
			for ( j = 0 ; j < N_PARTS ; j++ ){


#if 0
				vec->vy += P_GRAVITY;
				_sceVu0AddVector( pos++, pos_before++, vec++ );
				vec->vy += P_GRAVITY;
				_sceVu0AddVector( pos++, pos_before++, vec++ );
				vec->vy += P_GRAVITY;
				_sceVu0AddVector( pos++, pos_before++, vec++ );
				vec->vy += P_GRAVITY;
				_sceVu0AddVector( pos++, pos_before++, vec++ );
#else
#ifdef BP_PSX2_ASM				
				asm volatile ("
					lqc2		vf11,0x00(%0)
					lqc2		vf12,0x00(%1)
					lqc2		vf13,0x10(%0)
					lqc2		vf14,0x10(%1)
					lqc2		vf15,0x20(%0)
					lqc2		vf16,0x20(%1)
					lqc2		vf17,0x30(%0)
					lqc2		vf18,0x30(%1)

					vadd.xyz	vf11, vf11, vf12
					vadd.xyz	vf13, vf13, vf14
					vadd.y		vf12, vf12, vf10
					vadd.y		vf14, vf14, vf10
					vadd.xyz	vf15, vf15, vf16
					vadd.xyz	vf17, vf17, vf18
					vadd.y		vf16, vf16, vf10
					vadd.y		vf18, vf18, vf10

					sqc2		vf11,0x00(%0)
					sqc2		vf12,0x00(%1)
					sqc2		vf13,0x10(%0)
					sqc2		vf14,0x10(%1)
					sqc2		vf15,0x20(%0)
					sqc2		vf16,0x20(%1)
					sqc2		vf17,0x30(%0)
					sqc2		vf18,0x30(%1)
					"::"r"(fvptemp),"r"(vec):"memory"
				);
#else
				BP_Vec3_AddVec( fvptemp,  fvptemp,  vec ); 
				vec->vy += P_GRAVITY;
				BP_Vec3_AddVec( (fvptemp + 1),  (fvptemp + 1),  (vec + 1) ); 
				(vec + 1)->vy += P_GRAVITY;
				BP_Vec3_AddVec( (fvptemp + 2),  (fvptemp + 2),  (vec + 2) ); 
				(vec + 2)->vy += P_GRAVITY;
				BP_Vec3_AddVec( (fvptemp + 3),  (fvptemp + 3),  (vec + 3) ); 
				(vec + 3)->vy += P_GRAVITY;
#endif				

				surface_flag = 0;
				if( fvptemp->vy <= GM_WaterLevel ){
					fvptemp->vy  = GM_WaterLevel;
					surface_flag = 1;
				}
				fvptemp++;
				if( fvptemp->vy <= GM_WaterLevel ){
					fvptemp->vy  = GM_WaterLevel;
					surface_flag = 1;
				}
				fvptemp++;
				if( fvptemp->vy <= GM_WaterLevel ){
					fvptemp->vy  = GM_WaterLevel;
					surface_flag = 1;
				}
				fvptemp++;
				if( fvptemp->vy <= GM_WaterLevel ){
					fvptemp->vy  = GM_WaterLevel;
					surface_flag = 1;
				}
				fvptemp++;

				if( surface_flag ){
					if( (uvrgb_before)->a < SUB_ALPHA*2 ){
						uvrgb_before += 4;
						(uvrgb++)->a = 0;
						(uvrgb++)->a = 0 ;
						(uvrgb++)->a = 0 ;
						(uvrgb++)->a = 0 ;
						vec->vw = -1.0f;
					}else{
						alpha = (uvrgb_before)->a - SUB_ALPHA*2 ;
						uvrgb_before += 4;
						(uvrgb++)->a = alpha;
						(uvrgb++)->a = alpha ;
						(uvrgb++)->a = alpha ;
						(uvrgb++)->a = alpha ;
					}

					_sceVu0ScaleVector( vec, vec, SURFACE_DECAY_VEC );
					vec++;
					_sceVu0ScaleVector( vec, vec, SURFACE_DECAY_VEC );
					vec++;
					_sceVu0ScaleVector( vec, vec, SURFACE_DECAY_VEC );
					vec++;
					_sceVu0ScaleVector( vec, vec, SURFACE_DECAY_VEC );
					vec++;
				}else{
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

					vec+=4;
				}
#endif


			}
			OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_PARTS*4);

			pos        += N_PARTS*4;
			pos_before += N_PARTS*4;
		}else{
			for ( j = 0 ; j < N_PARTS ; j++ ){
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

				pos += 4;
			}
			uvrgb        += N_PARTS*4;
			uvrgb_before += N_PARTS*4;
			pos_before   += N_PARTS*4;
			vec          += N_PARTS*4;
		}
	}

	if( !flag ){
		if ( !GM_CheckGameStatus( STATE_DEMO ) ) GV_DestroyActor( work ) ;
	}
}

/*----------------------------------------------------------------*/
static	void	CulcVector( Work *work, FVECTOR *center, FVECTOR *force )
{
	DG_PRIM2		*prim ;
	FVECTOR			fvtemp;
	FVECTOR			v0;
	FVECTOR			v1;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i;
	int		clock;
	int		num;

	prim = work->prim;
	clock = prim->buffer_clock;

	fvtemp.vx = force->vz * 0.25f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = force->vx * 0.25f;

	num = work->activate_num * 4 * N_PARTS;
	vec          = work->vec;
	pos          = prim->pos[clock];
	uvrgb        = prim->uvrgb[clock];
	vec         += num;
	pos         += num;
	uvrgb       += num;

	for( i=0; i<N_PARTS; i++ ){

		vec->vx = v0.vx = force->vx - fvtemp.vx;
		vec->vy = v0.vy = force->vy            ;
		vec->vz = v0.vz = force->vz + fvtemp.vz;
		vec->vw = PART_LIFE;
		_sceVu0AddVector( pos++, center, vec++ );

		vec->vx = v1.vx =  force->vx + fvtemp.vx;
		vec->vy = v1.vy =  force->vy            ;
		vec->vz = v1.vz =  force->vz - fvtemp.vz;
		_sceVu0AddVector( pos++, center, vec++ );

		vec->vx = v0.vx * 0.75f;
		vec->vy = v0.vy * 0.75f;
		vec->vz = v0.vz * 0.75f;
		_sceVu0AddVector( pos++, center, vec++ );

		vec->vx = v1.vx * 0.75f;
		vec->vy = v1.vy * 0.75f;
		vec->vz = v1.vz * 0.75f;
		_sceVu0AddVector( pos++, center, vec++ );

		(uvrgb++)->a = MAX_ALPHA;
		(uvrgb++)->a = MAX_ALPHA;
		(uvrgb++)->a = MAX_ALPHA;
		(uvrgb++)->a = MAX_ALPHA;
	}

	work->activate_num++;
	work->activate_num = (work->activate_num < TOTAL_PARTS)? work->activate_num: 0;
}

/*----------------------------------------------------------------*/
int	OK_PutSplushSurface( FVECTOR *center, FVECTOR *force )
{
	Work			*work;

	if( OK_SplushSurfaceManWork==NULL ){
		if ( GM_CheckGameStatus( STATE_DEMO ) ){
//			printf("ERR:--------------------------------------------------\n");
//			printf("ERR:デモ中なんで先に水飛沫を常駐する必要があるんです。\n");
//			printf("ERR:Pls. set splush manager at first!!\n");
//			printf("ERR:--------------------------------------------------\n");
			return 0;
		}
		NewSplushSurfaceMan( 1, 1 );
		if( OK_SplushSurfaceManWork==NULL ) return 0;
	}
	if( OK_SplushSurfaceCount >= MAX_SPLUSH-1 ) return 0;
	OK_SplushSurfaceCount++;
	work = OK_SplushSurfaceManWork;

	CulcVector( work, center, force );

	return 1;
}

/*----------------------------------------------------------------*/

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_SplushSurfaceCount = 0;
	OK_SplushSurfaceManWork = NULL;
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		i ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos0   = prim->pos[ 0 ];
	pos1   = prim->pos[ 1 ];
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS*N_PRIMS*N_POLYS ; i++ ){
		DG_COPY_VEC( pos0, &DG_ZeroVector );	pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector );	pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector );	pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector );	pos0++;
		DG_COPY_VEC( pos1, &DG_ZeroVector );	pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector );	pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector );	pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector );	pos1++;

		uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x8fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x8fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x0fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x0fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
		uvrgb0++;
		uvrgb1++;
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

void *NewSplushSurfaceMan( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_PREV, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map  = map;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

		OK_SplushSurfaceManWork = work;

	}
	return (void *)work ;
}
