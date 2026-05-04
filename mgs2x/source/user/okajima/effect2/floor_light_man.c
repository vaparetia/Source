//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	floor_light_man.c
	床の照り返し制御

	2001/02/05 S.Okajima
	$Id: floor_light_man.c,v 1.1.1.3 2002/11/19 11:47:20 Yoshizawa1 Exp $
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
void *NewFloorLightMan( int name, int map );
/*----------------------------------------------------------------*/

#define	RAISE			(0)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)

//#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
//#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(8)
//以上でスクラッチパッドぎりぎり
#define	N_LOOPS		(4)

#define	LIFE		(8.0f)	/* 寿命に影響 */

#define	MIN_SIZE_RATIO	(0.01f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			act_count;
	int			activate_num;

	DG_PRIM2	*prim ;
} Work ;

static Work *OK_FloorLightWork = NULL;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	FVECTOR			*pos;
	FVECTOR			*dest_pos;
	FVECTOR			*dest_pos_before;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*dest_uvrgb;
	DG_PRIM2_UVRGB	*dest_uvrgb_before;
	int		i, j ;
	int		clock;
	int		alpha;
	int		count;

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	if( work->act_count <= 0 ){
		DG_InvisiblePrim2( prim );
		return;
	}else{
		DG_VisiblePrim2( prim );
	}


	if( GV_PauseLevel != 0 ){
		dest_pos          = prim->pos[  clock];
		dest_pos_before   = prim->pos[1-clock];
		dest_uvrgb        = prim->uvrgb[  clock];
		dest_uvrgb_before = prim->uvrgb[1-clock];
		for ( i = 0 ; i < N_LOOPS ; i++ ){
			OK_Mem_Scr( SCR_POS, dest_pos_before,   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
			OK_Mem_Scr( SCR_UVR, dest_uvrgb_before, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
			OK_Scr_Mem( dest_pos,   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
			OK_Scr_Mem( dest_uvrgb, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
			dest_pos         += N_PRIMS*N_VERTS;
			dest_pos_before  += N_PRIMS*N_VERTS;
			dest_uvrgb       += N_PRIMS*N_VERTS;
			dest_uvrgb_before+= N_PRIMS*N_VERTS;
		}
		return;
	}

	count = N_LOOPS*N_PRIMS*N_POLYS;
	dest_pos          = prim->pos[  clock];
	dest_pos_before   = prim->pos[1-clock];
	dest_uvrgb        = prim->uvrgb[  clock];
	dest_uvrgb_before = prim->uvrgb[1-clock];
	for ( i = 0 ; i < N_LOOPS ; i++ ){
		OK_Mem_Scr( SCR_POS, dest_pos_before,   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Mem_Scr( SCR_UVR, dest_uvrgb_before, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		pos   = SCR_POS;
		uvrgb = SCR_UVR;
		for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
			if( uvrgb->a > 0 ){
				alpha = uvrgb->a - 1;
				(uvrgb++)->a = alpha;
				(uvrgb++)->a = alpha;
				(uvrgb++)->a = alpha;
				(uvrgb++)->a = alpha;
				pos+=4;
			}else{
				count--;
#ifdef BP_PSX2_ASM	/// ？ 
				asm volatile ("
					lqc2		vf8 ,0x00(%0)
					vmove.xyz	vf9 ,vf8
					vmove.xyz	vf10,vf8
					vmove.xyz	vf11,vf9
					sqc2		vf9 ,0x10(%0)
					sqc2		vf10,0x20(%0)
					sqc2		vf11,0x30(%0)
					"::"r"(pos):"memory"
				);
#else		
				*(pos + 1) = *pos;
				*(pos + 2) = *pos;
				*(pos + 3) = *pos;
#endif				
				uvrgb+=4;
				pos+=4;
			}
		}
		OK_Scr_Mem( dest_pos,   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		dest_pos         += N_PRIMS*N_VERTS;
		dest_pos_before  += N_PRIMS*N_VERTS;
		dest_uvrgb       += N_PRIMS*N_VERTS;
		dest_uvrgb_before+= N_PRIMS*N_VERTS;
	}

	if( count <= 0 ){
		work->act_count--;
	}

}

/*----------------------------------------------------------------*/
int	OK_PutFloorLight( FVECTOR *center, float floor_height, float radius, CVECTOR col )
{
	Work			*work;
	DG_PRIM2		*prim ;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		clock;
	int		num;
	int		alpha;
	float	ftemp;
	float	size;
	float	size_min;

	if( OK_FloorLightWork==NULL ){
//		return 0;
		NewFloorLightMan( 1, 1 );
		if( OK_FloorLightWork==NULL ) return 0;
	}
	work = OK_FloorLightWork;

	work->act_count = 8;

	ftemp = DG_FABS( center->vy - floor_height );
	if( ftemp >= radius ) return 0;
	floor_height += 10.0f;

	size  = ftemp ;
//	ftemp = (radius - ftemp) / radius;
	ftemp = ftemp / radius;
	ftemp*= ftemp;

	ftemp = 1.0f - ftemp;

	alpha = (int)((float)col.cd * ftemp);
	if( alpha<=0 ) alpha = 1;

//	size = bp_sqrtf( radius*radius - ftemp*ftemp ) * ; //BP_MATH - emulate PS2 sqrtf

	size_min = radius*MIN_SIZE_RATIO;
	if( size < size_min ) size = size_min;

	prim = work->prim;
	clock = prim->buffer_clock;

	pos          = prim->pos[clock];
	uvrgb        = prim->uvrgb[clock];
	num = work->activate_num*4;
	pos         += num;
	uvrgb       += num;

	uvrgb->r = col.r;
	uvrgb->g = col.g;
	uvrgb->b = col.b;
	uvrgb->a = alpha;
	uvrgb++;
	uvrgb->r = col.r;
	uvrgb->g = col.g;
	uvrgb->b = col.b;
	uvrgb->a = alpha;
	uvrgb++;
	uvrgb->r = col.r;
	uvrgb->g = col.g;
	uvrgb->b = col.b;
	uvrgb->a = alpha;
	uvrgb++;
	uvrgb->r = col.r;
	uvrgb->g = col.g;
	uvrgb->b = col.b;
	uvrgb->a = alpha;
	uvrgb++;

//printf("cc:%d %d %d %d\n",col.r,col.g,col.b,alpha);
//printf("ss:%f\n",size);


	pos->vx = center->vx + size;
	pos->vy = floor_height;
	pos->vz = center->vz + size;
	pos++;

	pos->vx = center->vx - size;
	pos->vy = floor_height;
	pos->vz = center->vz + size;
	pos++;

	pos->vx = center->vx + size;
	pos->vy = floor_height;
	pos->vz = center->vz - size;
	pos++;

	pos->vx = center->vx - size;
	pos->vy = floor_height;
	pos->vz = center->vz - size;
//	pos++;

/*
{
	FVECTOR	fvtemp;
	AN_Test_Eye2( center, 2 );
	fvtemp.vx = center->vx;
	fvtemp.vy = floor_height;
	fvtemp.vz = center->vz;
	AN_Test_Eye2( &fvtemp, 2 );
}
*/

	work->activate_num++;
	work->activate_num = (work->activate_num < N_LOOPS*N_PRIMS*N_POLYS)? work->activate_num: 0;

	return 1;
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_FloorLightWork = NULL;
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

	dest_pos0   = prim->pos[ 0 ];
	dest_pos1   = prim->pos[ 1 ];
	dest_uvrgb0 = prim->uvrgb[ 0 ];
	dest_uvrgb1 = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS ; i++ ){
		pos   = SCR_POS;
		uvrgb = SCR_UVR;
		for ( j = 0 ; j < N_PRIMS ; j++ ){
			for ( k = 0 ; k < N_POLYS ; k++ ){
				DG_COPY_VEC( pos, &DG_ZeroVector );
				pos->vw = -10.0f;
				pos++;
				DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
				DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
				DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x8fff ;
				uvrgb->r = 0;
				uvrgb->g = 0;
				uvrgb->b = 0;
				uvrgb->a = 0;
				uvrgb++;

				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x8fff ;
				uvrgb->r = 0;
				uvrgb->g = 0;
				uvrgb->b = 0;
				uvrgb->a = 0;
				uvrgb++;

				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x0fff ;
				uvrgb->r = 0;
				uvrgb->g = 0;
				uvrgb->b = 0;
				uvrgb->a = 0;
				uvrgb++;

				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x0fff ;
				uvrgb->r = 0;
				uvrgb->g = 0;
				uvrgb->b = 0;
				uvrgb->a = 0;
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

	work->act_count = 0;

//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 7744599 /*"hamon09_add_alp"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS*N_LOOPS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	work->activate_num = 0;

	return 0 ;
}

void *NewFloorLightMan( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

//	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
//	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
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

		OK_FloorLightWork = work;

	}
	return (void *)work ;
}

