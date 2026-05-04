//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	water_wind.c
	ヘリの水飛沫制御

	2001/02/28 S.Okajima
	$Id: water_wind.c,v 1.1.1.3 2002/11/19 11:47:27 Yoshizawa1 Exp $
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
#include	"../../mode/demo/eft_con.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
/*----------------------------------------------------------------*/

#define	RAISE			(0)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_VEC		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)
#define	SCR_TMP0	(SCRPAD_ADDR + 0x3f00)
#define	SCR_TMP1	(SCRPAD_ADDR + 0x3f10)
#define	SCR_TMP2	(SCRPAD_ADDR + 0x3f20)
#define	SCR_TMP3	(SCRPAD_ADDR + 0x3f30)
#define	SCR_TMP4	(SCRPAD_ADDR + 0x3f40)

//#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
//#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(4)
#define	N_LOOPS		(32)

#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	MAX_ALPHA	(128)
#define	SUB_ALPHA	(MAX_ALPHA/N_LOOPS)


typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			delay_time;

	EFTCONTROL	control;
	FMATRIX		*world;
	float		inf_height;
	float		max_width;
	int			*flag;
	int			destroy_timer;

	int			activate_num;

	FVECTOR		vec[N_LOOPS*N_PRIMS*N_VERTS];	// 頂点速度は随時計算
	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
void OK_SpecialFade( Work *work )
{
#if 0
	work->inf_height-= 100.0f;
	if( work->inf_height < 0.0f ) work->inf_height = 0.0f;
#else
	work->inf_height = 0.0f;
	work->destroy_timer = 60;
#endif
}

/*----------------------------------------------------------------*/
static	void	CulcVector( Work *work, FMATRIX *world )
{
	DG_PRIM2		*prim ;
	float			intense;
	float			angle;
	float			width;
	FVECTOR			turb;
	FVECTOR			*force;
	FVECTOR			*fvtemp;
	FVECTOR			*center;
	FVECTOR			*v0;
	FVECTOR			*v1;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			*sc_pos;
	FVECTOR			*sc_vec;
	DG_PRIM2_UVRGB	*sc_uvrgb;
	int		i;
	int		clock;
	int		num;
	int		alpha;

	if( work->destroy_timer > 0 ){
		work->destroy_timer--;
		if( work->destroy_timer<= 0 ){
			GV_DestroyActor( work ) ;
		}
	}


	force  = SCR_TMP0;
	fvtemp = SCR_TMP1;
	center = SCR_TMP2;
	v0     = SCR_TMP3;
	v1     = SCR_TMP4;


	DG_COPY_VEC( center, (FVECTOR *)world->m[3] );
	intense = DG_FABS(center->vy - GM_WaterLevel);	//発生点からの高さ
	if( intense > work->inf_height ) return;

	prim = work->prim;
	clock = prim->buffer_clock;


	intense = DG_FABS( work->inf_height - intense ) / work->inf_height; //強さの割合に変換

	width = work->max_width * intense;
	alpha = MAX_ALPHA/2 + (int)((float)(MAX_ALPHA/2) * intense);


	angle = TPI*rnd();
	turb.vx = width*sinf(angle);
	turb.vy = 0.0f;
	turb.vz = width*cosf(angle);

	center->vy = GM_WaterLevel;


//	AN_Test_Eye2( center, 2 );



	num = work->activate_num * N_PRIMS*N_VERTS;
	vec    = work->vec;
	pos    = prim->pos[clock];
	uvrgb  = prim->uvrgb[clock];
	vec   += num;
	pos   += num;
	uvrgb += num;

	OK_Mem_Scr( SCR_POS, pos,   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Mem_Scr( SCR_VEC, vec,   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Mem_Scr( SCR_UVR, uvrgb, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	sc_pos   = SCR_POS;
	sc_vec   = SCR_VEC;
	sc_uvrgb = SCR_UVR;
#ifdef BP_PSX2_ASM	
	asm volatile ("
		lqc2		vf10,0x00(%0)
		"::"r"(center):"memory"
	);
#endif	
	for( i=0; i<N_PRIMS*N_POLYS; i++ ){
		angle = TPI*rnd();
		force->vx = width*sinf(angle);
		force->vy = width*0.25f*rnd();
		force->vz = width*cosf(angle);
		_sceVu0ScaleVector( force, force, 0.15f + 0.85f*rnd() );
		_sceVu0AddVector( force, force, &turb ) ;

#if 0
		fvtemp->vx = force->vz * 0.25f;
		fvtemp->vy = 0.0f;
		fvtemp->vz = force->vx * 0.25f;

		sc_vec->vx = v0->vx = force->vx - fvtemp->vx;
		sc_vec->vy = v0->vy = force->vy            ;
		sc_vec->vz = v0->vz = force->vz + fvtemp->vz;
		sc_vec->vw = 1.0f;
		sc_vec++;

		sc_vec->vx = v1->vx = force->vx + fvtemp->vx;
		sc_vec->vy = v1->vy = force->vy            ;
		sc_vec->vz = v1->vz = force->vz - fvtemp->vz;
		sc_vec++;

		sc_vec->vx = v0->vx * 0.75f;
		sc_vec->vy = v0->vy * 0.75f;
		sc_vec->vz = v0->vz * 0.75f;
		sc_vec++;

		sc_vec->vx = v1->vx * 0.75f;
		sc_vec->vy = v1->vy * 0.75f;
		sc_vec->vz = v1->vz * 0.75f;
		sc_vec++;

		DG_COPY_VEC( sc_pos++, center );
		DG_COPY_VEC( sc_pos++, center );
		DG_COPY_VEC( sc_pos++, center );
		DG_COPY_VEC( sc_pos++, center );
#else
		fvtemp->vx =-force->vz * 0.25f;
		fvtemp->vy = 0.0f;
		fvtemp->vz = force->vx * 0.25f;
		fvtemp->vw = 0.75f;

#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2		vf11,0x00(%0)
			lqc2		vf12,0x00(%1)
			vmove.xyz	vf13, vf11
			vadd.xz		vf13, vf13, vf12
			vmove.xyz	vf14, vf11
			vsub.xz		vf14, vf14, vf12

			vmulw.xyz	vf15, vf13, vf12
			vmulw.xyz	vf16, vf14, vf12

			sqc2		vf13,0x00(%3)
			sqc2		vf10,0x00(%2)
			sqc2		vf14,0x10(%3)
			sqc2		vf10,0x10(%2)
			sqc2		vf15,0x20(%3)
			sqc2		vf10,0x20(%2)
			sqc2		vf16,0x30(%3)
			sqc2		vf10,0x30(%2)
			"::"r"(force),"r"(fvtemp),"r"(sc_pos),"r"(sc_vec):"memory"
		);
#else
		{
			sc_pos[0] = sc_pos[1] = sc_pos[2] = sc_pos[3] = *center;

			sc_vec[0] = sc_vec[1] = *force;
			sc_vec[0].vx += fvtemp->vx; sc_vec[0].vz += fvtemp->vz;
			sc_vec[1].vx -= fvtemp->vx; sc_vec[1].vz -= fvtemp->vz;

			_sceVu0ScaleVectorXYZ( sc_vec + 2, sc_vec, fvtemp->vw );
			_sceVu0ScaleVectorXYZ( sc_vec + 3, sc_vec + 1, fvtemp->vw );
		}
#endif		
		sc_pos+= 4;
		sc_vec+= 4;
#endif

		(sc_uvrgb++)->a = alpha;
		(sc_uvrgb++)->a = alpha;
		(sc_uvrgb++)->a = alpha;
		(sc_uvrgb++)->a = alpha;
	}
	OK_Scr_Mem( pos,   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( vec,   SCR_VEC, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( uvrgb, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);

	work->activate_num++;
	work->activate_num = (work->activate_num < N_LOOPS)? work->activate_num: 0;
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	FMATRIX			tmp_world;
	FMATRIX			*world;
	FVECTOR			*vec;
	FVECTOR			*sc_vec;
	FVECTOR			*fvptemp;
	FVECTOR			*pos;
	FVECTOR			*pos_before;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*uvrgb_before;
	int		i, j ;
	int		clock;
	int		alpha;
	int		count;
	FVECTOR			fv_grav;
	float			ftemp;


	if( work->name==0 ){
		world = work->world;
	}else{
		DM_EftControlMatrix( &work->control, &tmp_world ) ;
		world = &tmp_world;
	}


//HZX_ViewMatrix( world, 8000.0f );


	ftemp = world->m[3][1] - GM_WaterLevel;	//発生点からの高さ


//printf("%f:%f:%f:\n",ftemp,world->m[3][1],GM_WaterLevel);


//printf("%d\n",work->delay_time);

	prim = work->prim;
	if( DG_FABS(ftemp) > work->inf_height ){
		if( work->delay_time < 0 ){
			DG_InvisiblePrim2( prim ) ;
			return;
		}else{
			work->delay_time--;
		}
	}else{
		work->delay_time = N_LOOPS+MAX_ALPHA;
	}

	DG_VisiblePrim2( prim ) ;
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
#ifdef BP_PSX2_ASM		////　PS2 でも#if0されているので必要ないのでは？
	asm volatile ("
		lqc2		vf10,0x00(%0)
		"::"r"(&fv_grav):"memory"
	);
#endif	

	count = N_LOOPS - work->activate_num + 1;
	for ( i = 0 ; i < N_LOOPS; i++ ){
		OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), N_PRIMS*N_VERTS);
		OK_Mem_Scr( SCR_VEC, vec,        sizeof(FVECTOR), N_PRIMS*N_VERTS);
		fvptemp = SCR_POS;
		sc_vec  = SCR_VEC;

		alpha = (uvrgb_before->a > SUB_ALPHA)? uvrgb_before->a - SUB_ALPHA: 0;

/*
		alpha = MAX_ALPHA * count / (N_LOOPS+1);
		count++;
		count = (count>N_LOOPS)? 0: count;
*/
		for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
			(uvrgb++)->a = alpha;
			(uvrgb++)->a = alpha ;
			(uvrgb++)->a = alpha ;
			(uvrgb++)->a = alpha ;

#if 0
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
				"::"r"(fvptemp),"r"(sc_vec):"memory"
			);

			fvptemp->vy = (fvptemp->vy > GM_WaterLevel)? fvptemp->vy: GM_WaterLevel;
			fvptemp++;
			fvptemp->vy = (fvptemp->vy > GM_WaterLevel)? fvptemp->vy: GM_WaterLevel;
			fvptemp++;
			fvptemp->vy = (fvptemp->vy > GM_WaterLevel)? fvptemp->vy: GM_WaterLevel;
			fvptemp++;
			fvptemp->vy = (fvptemp->vy > GM_WaterLevel)? fvptemp->vy: GM_WaterLevel;
			fvptemp++;
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
				vadd.xyz	vf15, vf15, vf16
				vadd.xyz	vf17, vf17, vf18

				sqc2		vf11,0x00(%0)
				sqc2		vf13,0x10(%0)
				sqc2		vf15,0x20(%0)
				sqc2		vf17,0x30(%0)
				"::"r"(fvptemp),"r"(sc_vec):"memory"
			);
#else
			{
				_sceVu0AddVector( fvptemp, fvptemp, sc_vec );
				_sceVu0AddVector( fvptemp + 1, fvptemp + 1, sc_vec + 1 );
				_sceVu0AddVector( fvptemp + 2, fvptemp + 2, sc_vec + 2 );
				_sceVu0AddVector( fvptemp + 3, fvptemp + 3, sc_vec + 3 );

			}
#endif			
			fvptemp+= 4;
#endif


			sc_vec+=4;
		}
		OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS);

		vec          += N_PRIMS*N_VERTS;
		pos          += N_PRIMS*N_VERTS;
		pos_before   += N_PRIMS*N_VERTS;
		uvrgb_before += N_PRIMS*N_VERTS;
	}

	CulcVector( work, world );

}


/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	if( work->name != 0 ){
		DM_EftControlDelList( &work->control );
	}
	work->prim = OK_FreePrim2( work->prim );
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
				uvrgb->a = 0;
				uvrgb++;

				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x8fff ;
				uvrgb->r = COL_R ;
				uvrgb->g = COL_G ;
				uvrgb->b = COL_B ;
				uvrgb->a = 0;
				uvrgb++;

				uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x0fff ;
				uvrgb->r = COL_R ;
				uvrgb->g = COL_G ;
				uvrgb->b = COL_B ;
				uvrgb->a = 0;
				uvrgb++;

				uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb->q = 4096 ;
				uvrgb->f = 0x0fff ;
				uvrgb->r = COL_R ;
				uvrgb->g = COL_G ;
				uvrgb->b = COL_B ;
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

	work->destroy_timer = -1;
	work->activate_num = 0;

	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS*N_LOOPS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	return 0 ;
}

/*
inf_height は 水面からの高さで、
center と水面との垂直距離が inf_height 以下になると影響が出始める
*/
void *NewWaterWindSplush( FMATRIX *world, float inf_height, float max_width, int *flag )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name       = 0;
		work->world      = world;
		work->inf_height = inf_height;
		work->max_width  = max_width / (float)(N_LOOPS);
		work->flag       = flag;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}

/*
デモ用
inf_height は 水面からの高さで、
center と水面との垂直距離が inf_height 以下になると影響が出始める
*/
void *NewWaterWindSplush_DEMO( int name, float inf_height, float max_width )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		DM_EftControlAddList( name, &work->control );

		work->inf_height = inf_height;
		work->max_width  = max_width / (float)(N_LOOPS);
		work->flag       = NULL;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}
