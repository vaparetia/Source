//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_man.c
	水飛沫制御

	2001/01/05 S.Okajima
	$Id: splush_man.c,v 1.1.1.3 2002/11/19 11:47:24 Yoshizawa1 Exp $
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
extern void *NewSplushMan( int name, int map );
/*----------------------------------------------------------------*/
int	OK_SplushManFlag = 1;
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

#define	N_PARTS		(N_POLYS/8)	/* 一つの水飛沫の中の枚数 */

#define	TOTAL_PARTS		(N_LOOPS*N_PRIMS*N_POLYS/N_PARTS)	/* 水飛沫の個数 */


#define	PART_LIFE		(32)		/* 一つの水飛沫の寿命 */
#define	MAX_SPLUSH		(TOTAL_PARTS/PART_LIFE)	/* 一フレーム内に処理できる最大 */


#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	SUB_ALPHA	(7)			/* 寿命に影響 */
#define	MAX_ALPHA	(SUB_ALPHA * PART_LIFE)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			activate_num;

	FVECTOR		vec[N_LOOPS*N_PRIMS*N_VERTS];	// 頂点速度は随時計算
	DG_PRIM2	*prim ;
} Work ;

static Work *OK_SplushManWork = NULL;
static int	OK_SplushCount=0;

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
	FVECTOR			fv_grav;

	if( !OK_SplushManFlag ){
		OK_SplushManFlag = 1;
		GV_DestroyActor( work ) ;
		DG_InvisiblePrim2( work->prim );
		return;
	}else{
		DG_VisiblePrim2( work->prim );
	}

   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }

	OK_SplushCount=0;

//	if(GV_Time%2==0){
	if(0){
		FVECTOR	fff;
		extern int OK_PutSplush( FVECTOR *center, FVECTOR *vec );

		fff.vx = frnd()*40.0f;
		fff.vy =  rnd()*20.0f+150.0f;
		fff.vz = frnd()*40.0f;
		OK_PutSplush( &GM_PlayerPosition, &fff );
		OK_PutSplush( &GM_PlayerPosition, &fff );
		OK_PutSplush( &GM_PlayerPosition, &fff );
		OK_PutSplush( &GM_PlayerPosition, &fff );
	}
//	if( GV_Time%128 < 16 ){
	if(0){
		FVECTOR	fvtemp0;
		FVECTOR	fvtemp1;
		SVECTOR	rot;
		extern void *PutSplushLine( FVECTOR *pos0, FVECTOR *pos1, SVECTOR *rot, float size, int num );

		fvtemp0.vx = GM_PlayerPosition.vx - 1000.0f;
		fvtemp0.vy = GM_PlayerPosition.vy;
		fvtemp0.vz = GM_PlayerPosition.vz;
		fvtemp1.vx = GM_PlayerPosition.vx + 1000.0f;
		fvtemp1.vy = GM_PlayerPosition.vy;
		fvtemp1.vz = GM_PlayerPosition.vz;

		rot.vx = -1024-256-(irnd()>>8)%256;
		rot.vy = 2048 + (irnd()>>8)%256-128;
		rot.vz = 0;
		PutSplushLine( &fvtemp0, &fvtemp1, &rot, rnd()*100.0f + 100.0f, 8 );
	}
	if(0){
		FVECTOR	fvtemp0;
		FVECTOR	fvtemp1;
		extern void *NewSplushPlane( FVECTOR *pos0, FVECTOR *pos1, float height, int side );

		fvtemp0.vx = GM_PlayerPosition.vx - 500.0f;
		fvtemp0.vy = GM_PlayerPosition.vy;
		fvtemp0.vz = GM_PlayerPosition.vz + (float)(GV_Time%30)*100.0f;
		fvtemp1.vx = GM_PlayerPosition.vx + 500.0f;
		fvtemp1.vy = GM_PlayerPosition.vy;
		fvtemp1.vz = GM_PlayerPosition.vz + (float)(GV_Time%30)*100.0f;

		NewSplushPlane( &fvtemp0, &fvtemp1, rnd()*100.0f + 100.0f, 0 );
	}





	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
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

	for ( i = 0 ; i < TOTAL_PARTS ; i++ ){
//		if( uvrgb_before->a != 0 ){
		if( vec->vw > 0.0f ){
			OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), N_PARTS*4);
			fvptemp = SCR_POS;
			for ( j = 0 ; j < N_PARTS ; j++ ){
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

#if 1
				if( fvptemp->vy <= GM_WaterLevel ){
					fvptemp->vy  = GM_WaterLevel;
				}
				fvptemp++;
				if( fvptemp->vy <= GM_WaterLevel ){
					fvptemp->vy  = GM_WaterLevel;
				}
				fvptemp++;
				if( fvptemp->vy <= GM_WaterLevel ){
					fvptemp->vy  = GM_WaterLevel;
				}
				fvptemp++;
				if( fvptemp->vy <= GM_WaterLevel ){
					fvptemp->vy  = GM_WaterLevel;
				}
				fvptemp++;
#else
				fvptemp+=4;
#endif

//				fvptemp+=4;
				vec+=4;
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

}

/*----------------------------------------------------------------*/
static	void	CulcVector( Work *work, FVECTOR *center, SVECTOR *rot, float length )
{
	DG_PRIM2		*prim ;
	FVECTOR			*pos;
	FVECTOR			*vec;
	FVECTOR			*sc_pos;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR	fvec[4];
	int		i;
	int		clock;
	int		num;
	SVECTOR	svtemp;
	float	ftemp1;
	float	len;

	prim = work->prim;
	clock = prim->buffer_clock;
#if 0
	if( work->act_clock == (GV_Time&1) ){	// 現在のフレームact処理前にここに来た
		clock = 1 - clock;	// 変更すべきフレームデータは裏である
	}
#endif

	num = work->activate_num * 4 * N_PARTS;
	vec          = work->vec;
	pos          = prim->pos[clock];
	uvrgb        = prim->uvrgb[clock];
	vec         += num;
	pos         += num;
	uvrgb       += num;

	svtemp.vx = rot->vx;
	svtemp.vy = rot->vy;
	for( i=0; i<N_PARTS; i++ ){

		sc_pos = fvec;

		len   = length * (rnd()*0.9f + 0.1f);
		sc_pos->vx = 0.0f;
		sc_pos->vy = 0.0f;
		sc_pos->vz = len;
		sc_pos++;

		ftemp1 = len * (rnd()*0.1f + 0.1f);
		len  *=0.5f;

		sc_pos->vx = ftemp1;
		sc_pos->vy = 0.0f;
		sc_pos->vz = len;
		sc_pos++;

		sc_pos->vx =-ftemp1;
		sc_pos->vy = 0.0f;
		sc_pos->vz = len;
		sc_pos++;

		sc_pos->vx = 0.0f;
		sc_pos->vy = 0.0f;
		sc_pos->vz = len*0.5f;
//		sc_pos++;

		svtemp.vz = (irnd()>>8)&4095;

		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_RotVector( fvec, vec, 4 );
		vec->vw = 1.0f;

		_sceVu0AddVector( pos++, center, vec++ );
		_sceVu0AddVector( pos++, center, vec++ );
		_sceVu0AddVector( pos++, center, vec++ );
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
/*----------------------------------------------------------------*/
// 中心と方向ベクトル（長さ有り）から
int	OK_PutSplush( FVECTOR *center, FVECTOR *force )
{
	Work			*work;
	FVECTOR	fvtemp;
	SVECTOR	rot;
	SVECTOR	svtemp;
	float	ftemp0;

	if( OK_SplushCount++ >= MAX_SPLUSH ) return 0;
	if( OK_SplushManWork==NULL ){
		NewSplushMan( 1, 1 );
		if( OK_SplushManWork==NULL ) return 0;
	}
	work = OK_SplushManWork;

	// force の 方向を求める
	ftemp0 = atan2f( force->vx, force->vz ) ;
	rot.vy = 4095 & ( short )( ( ftemp0 * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */
	svtemp.vx = 0.0f;
	svtemp.vy = -rot.vy;
	svtemp.vz = 0.0f;
	DG_SetPos2( &DG_ZeroVector, &svtemp );
	DG_PutVector( force, &fvtemp, 1 );
	ftemp0 = -atan2f( fvtemp.vy, fvtemp.vz ) ;
	rot.vx = 4095 & ( short )( ( ftemp0 * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */
	rot.vz = 0 ;


	CulcVector( work, center, &rot, GV_VecLen3F( force ) );

	return 1;
}

/*----------------------------------------------------------------*/
// 中心と回転と長さから
int	OK_PutSplush2( FVECTOR *center, SVECTOR *rot, float length )
{
	Work			*work;

	if( OK_SplushCount++ >= MAX_SPLUSH ) return 0;
	if( OK_SplushManWork==NULL ){
		NewSplushMan( 1, 1 );
		if( OK_SplushManWork==NULL ) return 0;
	}
	work = OK_SplushManWork;

	CulcVector( work, center, rot, length );

	return 1;
}

/*----------------------------------------------------------------*/
void *PutSplushLine( FVECTOR *pos0, FVECTOR *pos1, SVECTOR *rot, float size, int num )
{
	Work			*work;
	FVECTOR	diff;
	FVECTOR	fvtemp;
	int	i;

	if( OK_SplushManWork==NULL ){
		NewSplushMan( 1, 1 );
		if( OK_SplushManWork==NULL ) return 0;
	}
	work = OK_SplushManWork;

	_sceVu0SubVector( &diff, pos1, pos0 ) ;
	DG_COPY_VEC( &fvtemp, pos0 );
	for( i=0; i<num; i++ ){
		_sceVu0ScaleVector( &fvtemp, &diff, rnd() );
		_sceVu0AddVector( &fvtemp, &fvtemp, pos0 );
		CulcVector( work, &fvtemp, rot, size );
	}

	return (void *)work;
}
/*----------------------------------------------------------------*/



static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_SplushCount = 0;
	OK_SplushManWork = NULL;
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
	int		i, j ;

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
		for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
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
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = 0 ;
			uvrgb++;
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

void *NewSplushMan( int name, int map )
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

		OK_SplushManWork = work;

	}
	return (void *)work ;
}

void *NewSplushManProg( void )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = 1;
		work->map  = 1;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

		OK_SplushManWork = work;

	}
	return (void *)work ;
}
