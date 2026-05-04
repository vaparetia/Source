//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    waterripple.c
    バンプ波紋

    2001/01/17 S.Okajima
    $Id: waterripple.c,v 1.1.1.3 2002/11/19 11:47:55 Yoshizawa1 Exp $
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
#include	"../../okajima/etc/ok_util.h"
#include	"../../okajima/effect2/bubble.h"
#include	"../../mode/demo/eft_con.h"

/*----------------------------------------------------------------*/
extern int OK_RB_Num;
extern int OK_EX0_Flag;
extern FVECTOR OK_EX0_Pos;
/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
#define	RAISE_SUB	(-3000)
#define	RAISE_ADD	( 3000)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS_ADD		(SCRPAD_ADDR)
#define	SCR_POS_SUB		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVR		    (SCRPAD_ADDR + 0x2000)

//#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
//#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(4)
//以上でスクラッチパッドぎりぎり
#define	N_LOOPS		(4)

//#define	COL_R		(255)
//#define	COL_G		(255)
//#define	COL_B		(255)

#define	COL_R		(96)
#define	COL_G		(96)
#define	COL_B		(96)
#define	MAX_ALPHA	(32.0f)	/* 寿命に影響 */

#define	MINIMUM_SIZE	(10.0f)
#define	MIDDLE_SIZE		(100.0f)
#define	STABLE_SIZE		(100.0f)
#define	SIZE_ADD		(100.0f)

#define	WATERRIPPLE_SIZE_MAX		(STABLE_SIZE+SIZE_ADD)

//#define	DECAY_VEC		(0.97f)
//#define	DECAY_VEC		(0.99f)
#define	DECAY_VEC		(1.00f)
#define	VEC_RATIO_TOP	(0.10f)
#define	VEC_RATIO_DOWN	(0.06f)

#define	DELAY_NUM	(32)
#define	DELAY_SKIP	(12)
#define	INITIAL_SCALE	(6.0f)

#define	INFLUENCE_HEIGHT_UPPER	(100.0f)
#define	INFLUENCE_HEIGHT_BELOW	(2000.0f)

typedef	struct	{
    GV_ACT_EX	actor;
    int		name;

    int		my_num;

    float	inf_upper;
    float	inf_below;

    int		act_clock;
    int		activate_num;

    FVECTOR*	pvecRefer;
    
    FVECTOR	pre_pos[DELAY_NUM];
    FVECTOR	vec[N_PRIMS*N_LOOPS*N_VERTS];

    // 描画系
    DG_PRIM2*	prim_add ;
    DG_PRIM2*	prim_sub ;
} Work ;

/*----------------------------------------------------------------*/
static void InitVectors( Work *work,
                  FVECTOR *center,
                  FVECTOR *force/*XZ平面投影後の方向と大きさを使用*/,
                  float length,
                  int clock )
{
	DG_PRIM2		*prim_add ;
	DG_PRIM2		*prim_sub ;
	FVECTOR			*pos_add;
	FVECTOR			*pos_sub;
	FVECTOR			*vec;
	DG_PRIM2_UVRGB	*uvrgb_add;
	DG_PRIM2_UVRGB	*uvrgb_sub;
	FVECTOR			local_force;
	FVECTOR			fvtemp0;
	FVECTOR			fvtemp1;
	FVECTOR			fvtemp2;
	FVECTOR			fvtemp3;
	FVECTOR			calc_vec;
	int		num;
	int		alpha;
//	float			length;
	float			param;


	DG_COPY_VEC( &calc_vec, center );
	if( (     (calc_vec.vy < GM_WaterLevel - work->inf_below)
	       || (calc_vec.vy > GM_WaterLevel + work->inf_upper)  )
	 || (     (GV_Time&1) && (OK_RB_Num!=0) ) ){
		if( OK_EX0_Flag
		 && (GV_Time%OK_RB_Num==work->my_num) ){	// バンプ戦特別
			DG_COPY_VEC( &calc_vec, &OK_EX0_Pos );
			DG_COPY_VEC( &local_force, force );
		}else{
			return;
		}
	}else{
		if( calc_vec.vy < GM_WaterLevel ){
			_sceVu0ScaleVector( &local_force, force, (GM_WaterLevel - calc_vec.vy) / work->inf_below );
		}else{
			_sceVu0ScaleVector( &local_force, force, (calc_vec.vy - GM_WaterLevel) / work->inf_upper );
		}
	}

	if( length > WATERRIPPLE_SIZE_MAX ){
		length = WATERRIPPLE_SIZE_MAX;
	}

	prim_add = work->prim_add;
	prim_sub = work->prim_sub;

	/* ＸＺ平面投影 */
	param = frnd();
	fvtemp0.vx = local_force.vx  - param*local_force.vz;
	fvtemp0.vy = 0.0f;
	fvtemp0.vz = local_force.vz  + param*local_force.vx;
	fvtemp0.vw = 1.0f;
	_sceVu0Normalize( &fvtemp0, &fvtemp0 );
	_sceVu0ScaleVector( &fvtemp0, &fvtemp0, length*0.75f );

	/* 直交 */
	fvtemp1.vx =-fvtemp0.vz * 0.5f;
	fvtemp1.vy = 0.0f;
	fvtemp1.vz = fvtemp0.vx * 0.5f;

	fvtemp2.vx = fvtemp0.vx + fvtemp1.vx;
	fvtemp2.vy = 0.0f;
	fvtemp2.vz = fvtemp0.vz + fvtemp1.vz;

	fvtemp3.vx = fvtemp0.vx - fvtemp1.vx;
	fvtemp3.vy = 0.0f;
	fvtemp3.vz = fvtemp0.vz - fvtemp1.vz;

	_sceVu0ScaleVector( &fvtemp1, &fvtemp1, 0.25f );



	vec          = work->vec;
	pos_add      = prim_add->pos[clock];
	pos_sub      = prim_sub->pos[clock];
	uvrgb_add    = prim_add->uvrgb[clock];
	uvrgb_sub    = prim_sub->uvrgb[clock];
	num = work->activate_num*4;
	work->activate_num++;
	work->activate_num = (work->activate_num < N_LOOPS*N_PRIMS*N_POLYS)? work->activate_num: 0;
	vec         += num;
	pos_add     += num;
	pos_sub     += num;
	uvrgb_add   += num;
	uvrgb_sub   += num;

	param = 1.0f;
//	if(1){
//	if( (irnd()>>8)%2==0 ){
		_sceVu0ScaleVector( vec++, &fvtemp2, VEC_RATIO_DOWN );
		_sceVu0ScaleVector( vec++, &fvtemp2, VEC_RATIO_TOP );
		_sceVu0ScaleVector( vec++, &fvtemp3, VEC_RATIO_DOWN );
		_sceVu0ScaleVector( vec++, &fvtemp3, VEC_RATIO_TOP );
//	}else{
//		param = rnd()*0.25f + 0.25f;
//		_sceVu0ScaleVector( &fvtemp0, &fvtemp1, frnd() );
//		_sceVu0ScaleVector( vec, &fvtemp3,-VEC_RATIO_DOWN * param );
//		_sceVu0AddVector( vec, vec, &fvtemp0 );	vec++;
//		_sceVu0ScaleVector( vec, &fvtemp3,-VEC_RATIO_TOP  * param );
//		_sceVu0AddVector( vec, vec, &fvtemp0 );	vec++;
//		_sceVu0ScaleVector( vec, &fvtemp2,-VEC_RATIO_DOWN * param );
//		_sceVu0AddVector( vec, vec, &fvtemp0 );	vec++;
//		_sceVu0ScaleVector( vec, &fvtemp2,-VEC_RATIO_TOP  * param );
//		_sceVu0AddVector( vec, vec, &fvtemp0 );	vec++;
//	}
//	alpha = (int)(MAX_ALPHA * length * param / WATERRIPPLE_SIZE_MAX);
//	alpha = (int)(MAX_ALPHA * length / WATERRIPPLE_SIZE_MAX);
	alpha = (int)(MAX_ALPHA * (0.5f + length*0.5f / WATERRIPPLE_SIZE_MAX) );

//printf("0:%d\n",alpha);
//alpha = 255;

	(uvrgb_add++)->a = alpha;
	(uvrgb_add++)->a = alpha;
	(uvrgb_add++)->a = alpha;
	(uvrgb_add++)->a = alpha;

	(uvrgb_sub++)->a = alpha;
	(uvrgb_sub++)->a = alpha;
	(uvrgb_sub++)->a = alpha;
	(uvrgb_sub++)->a = alpha;

//printf("1:%f:%f:%f\n",fvtemp1.vx,fvtemp1.vy,fvtemp1.vz);
//printf("2:%f:%f:%f\n",fvtemp2.vx,fvtemp2.vy,fvtemp2.vz);

	fvtemp0.vx = calc_vec.vx;
	fvtemp0.vy = GM_WaterLevel;
	fvtemp0.vz = calc_vec.vz;

	_sceVu0AddVector( pos_add, &fvtemp0, &fvtemp1 ) ;
	_sceVu0AddVector( pos_add, pos_add, &fvtemp2 ) ;
	DG_COPY_VEC( pos_sub++, pos_add++ ) ;

	_sceVu0AddVector( pos_add, &fvtemp0, &fvtemp1 ) ;
	_sceVu0AddVector( pos_add, pos_add, &fvtemp2 ) ;
	DG_COPY_VEC( pos_sub++, pos_add++ ) ;

	_sceVu0SubVector( pos_add, &fvtemp0, &fvtemp1 ) ;
	_sceVu0AddVector( pos_add, pos_add, &fvtemp3 ) ;
	DG_COPY_VEC( pos_sub++, pos_add++ ) ;

	_sceVu0SubVector( pos_add, &fvtemp0, &fvtemp1 ) ;
	_sceVu0AddVector( pos_add, pos_add, &fvtemp3 ) ;
	DG_COPY_VEC( pos_sub++, pos_add++ ) ;
}

/* ---------------------------------------------------------------- */
#if 0
enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
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
			work->on_flag = 0;
			break;
		  case REQ_ON:
			work->on_flag = 1;
			break;
		  case REQ_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			  break;
		}
		msg--;
	}
}
#endif

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim_add ;
	DG_PRIM2		*prim_sub ;
	FVECTOR			*pos_add;
	FVECTOR			*pos_sub;
	FVECTOR			*vec;
	FVECTOR			*dest_pos_add;
	FVECTOR			*dest_pos_sub;
	FVECTOR			*dest_pos_before;
	FVECTOR			fvtemp0;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*dest_uvrgb;
	DG_PRIM2_UVRGB	*dest_uvrgb_sub;
	DG_PRIM2_UVRGB	*dest_uvrgb_before;

	int		i, j ;
	int		count;
	int		clock;
	float	param;
	float	angle;
	float	len;

	prim_add = work->prim_add;
	prim_sub = work->prim_sub;

	GM_GroupPrim2( prim_add, GM_CurrentStageMap ) ;
	GM_GroupPrim2( prim_sub, GM_CurrentStageMap ) ;
	DG_VisiblePrim2( prim_add ) ;
	DG_VisiblePrim2( prim_sub ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim_add ) )
   {
      return;
   }
	DG_SwitchBuffPrim2( prim_sub );
	clock = prim_add->buffer_clock;



//	GM_WaterLevel = -3000.0f;

	count = work->act_clock;
	vec               = work->vec;
	dest_pos_add      = prim_add->pos[    clock];
	dest_pos_sub      = prim_sub->pos[    clock];
	dest_pos_before   = prim_add->pos[  1-clock];
	dest_uvrgb        = prim_add->uvrgb[  clock];
	dest_uvrgb_sub    = prim_sub->uvrgb[  clock];
	dest_uvrgb_before = prim_add->uvrgb[1-clock];
	for ( i = 0 ; i < N_LOOPS ; i++ ){
		OK_Mem_Scr( SCR_POS_ADD, dest_pos_before,   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Mem_Scr( SCR_UVR, dest_uvrgb_before, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		pos_add = SCR_POS_ADD;
		pos_sub = SCR_POS_SUB;
		uvrgb   = SCR_UVR;
		for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
			if( uvrgb->a != 0 ){
				if( (count&1)==0 ){
					(uvrgb++)->a--;
					(uvrgb++)->a--;
					(uvrgb++)->a--;
					(uvrgb++)->a--;
				}else{
					uvrgb  += 4;
				}

				_sceVu0ScaleVector( &fvtemp0, vec, 2.0f );

				pos_add->vx += vec->vx;
				pos_add->vy  = GM_WaterLevel;
				pos_add->vz += vec->vz;
				_sceVu0AddVector( pos_sub++, pos_add++, &fvtemp0 ) ;
				vec++;

				pos_add->vx += vec->vx;
				pos_add->vy  = GM_WaterLevel;
				pos_add->vz += vec->vz;
				_sceVu0AddVector( pos_sub++, pos_add++, &fvtemp0 ) ;
				vec++;

				pos_add->vx += vec->vx;
				pos_add->vy  = GM_WaterLevel;
				pos_add->vz += vec->vz;
				_sceVu0AddVector( pos_sub++, pos_add++, &fvtemp0 ) ;
				vec++;

				pos_add->vx += vec->vx;
				pos_add->vy  = GM_WaterLevel;
				pos_add->vz += vec->vz;
				_sceVu0AddVector( pos_sub++, pos_add++, &fvtemp0 ) ;
				vec++;

			}else{
#ifdef BP_PSX2_ASM //yano /*未検証*/
				asm volatile ("
					lqc2		vf11,0x00(%0)
					sqc2		vf11,0x10(%0)
					sqc2		vf11,0x20(%0)
					sqc2		vf11,0x30(%0)
					sqc2		vf11,0x00(%1)
					sqc2		vf11,0x10(%1)
					sqc2		vf11,0x20(%1)
					sqc2		vf11,0x30(%1)
					"::"r"(pos_add),"r"(pos_sub):"memory"
				);
#else
				_sceVu0CopyVector( pos_add + 1, pos_add );
				_sceVu0CopyVector( pos_add + 2, pos_add );
				_sceVu0CopyVector( pos_add + 3, pos_add );
				_sceVu0CopyVector( pos_sub    , pos_add );
				_sceVu0CopyVector( pos_sub + 1, pos_add );
				_sceVu0CopyVector( pos_sub + 2, pos_add );
				_sceVu0CopyVector( pos_sub + 3, pos_add );
				
#endif
				pos_add+= 4;
				pos_sub+= 4;
				uvrgb  += 4;
				vec    += 4;
			}
			count++;
		}
		OK_Scr_Mem( dest_pos_add,   SCR_POS_ADD, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_pos_sub,   SCR_POS_SUB, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb,     SCR_UVR,     sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb_sub, SCR_UVR,     sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		dest_pos_add     += N_PRIMS*N_VERTS;
		dest_pos_sub     += N_PRIMS*N_VERTS;
		dest_pos_before  += N_PRIMS*N_VERTS;
		dest_uvrgb       += N_PRIMS*N_VERTS;
		dest_uvrgb_sub   += N_PRIMS*N_VERTS;
		dest_uvrgb_before+= N_PRIMS*N_VERTS;
	}



	DG_COPY_VEC( &work->pre_pos[0], work->pvecRefer ) ;

	work->act_clock++;


//////

	if ( GM_CheckPlayerStatus(PLAYER_BEYOND)
	  && GM_CheckPlayerStatus(PLAYER_WATCH)  ) {
		return;
	}

	pos_add = &work->pre_pos[DELAY_NUM-2];
	pos_sub = &work->pre_pos[DELAY_NUM-1];
	for( i=0; i<DELAY_NUM-1; i++ ){
		_sceVu0SubVector( &fvtemp0, pos_add, pos_sub ) ;
		DG_COPY_VEC( pos_sub, pos_add ) ;

		if(i%DELAY_SKIP==0){
			param = GV_VecLen3F( &fvtemp0 ) * INITIAL_SCALE * ((float)(i)/(float)(DELAY_NUM)*0.7f + 0.3f);

			if( param < MINIMUM_SIZE ){
				angle = rnd()*TPI;
				fvtemp0.vx = sinf(angle);
				fvtemp0.vy = 0;
				fvtemp0.vz = cosf(angle);
				InitVectors( work, pos_add, &fvtemp0, rnd()*STABLE_SIZE*0.25f + STABLE_SIZE*0.75f, clock );
			}else{
				if( (irnd()>>8)&1 ){
					angle = rnd()*TPI;
					len = DG_MAX( fvtemp0.vx, fvtemp0.vz ) * 0.5f;
//					len = GV_VecLen3F( &fvtemp0 )*0.5f;
					fvtemp0.vx = len * sinf(angle);
					fvtemp0.vy = 0;
					fvtemp0.vz = len * cosf(angle);
//					param*= 0.75f;
				}
				InitVectors( work, pos_add, &fvtemp0, STABLE_SIZE+param, clock );
			}
		}
		pos_add--;
		pos_sub--;
	}



}

static void Die( Work *work )
{
    work->prim_add = OK_FreePrim2( work->prim_add );
    work->prim_sub = OK_FreePrim2( work->prim_sub );

    OK_EX0_Flag = 0;
    OK_RB_Num--;
    if( OK_RB_Num < 0 ) OK_RB_Num = 0;
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int add_sub )
{
	FVECTOR			*pos;
	FVECTOR			*dest_pos0;
	FVECTOR			*dest_pos1;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*dest_uvrgb0;
	DG_PRIM2_UVRGB	*dest_uvrgb1;
	int		i, j ;

	DG_ConfigPrim2Tex( prim, tex );
	if( add_sub ){
		prim->raise = RAISE_SUB;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}else{
		prim->raise = RAISE_ADD;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}

	prim->buffer_clock = 0;


	dest_pos0   = prim->pos[ 0 ];
	dest_pos1   = prim->pos[ 1 ];
	dest_uvrgb0 = prim->uvrgb[ 0 ];
	dest_uvrgb1 = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS ; i++ ){
		pos   = SCR_POS_ADD;
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
		OK_Scr_Mem( dest_pos0,   SCR_POS_ADD, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_pos1,   SCR_POS_ADD, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb0, SCR_UVR,     sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb1, SCR_UVR,     sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		dest_pos0  += N_PRIMS*N_VERTS;
		dest_pos1  += N_PRIMS*N_VERTS;
		dest_uvrgb0+= N_PRIMS*N_VERTS;
		dest_uvrgb1+= N_PRIMS*N_VERTS;
	}

}

static int GetResources( Work *work, int name, FVECTOR* pvecRefer )
{
    DG_PRIM2*	prim;
    DG_TEX*	tex;

    work->pvecRefer = pvecRefer;

    work->name = name;

    tex = DG_GetTexture( 12895813 /*"ripple02_alp"*/ );
    prim = work->prim_add = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 
					  N_PRIMS*N_LOOPS, N_VERTS );
    if(prim==NULL){
	printf("null prim\n");
	return -1;
    }
    InitPacket2( work, prim, tex, 0 );

    prim = work->prim_sub = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 
					  N_PRIMS*N_LOOPS, N_VERTS );
    if(prim==NULL){
	printf("null prim\n");
	return -1;
    }
    InitPacket2( work, prim, tex, 1 );

    work->act_clock = 0;
    work->activate_num = 0;

    return 0 ;
}

void* NewWaterRippleMan( int name, FVECTOR* pvecRefer)
{
    Work*	work;

    OPERATOR();

    work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor );
	work->inf_upper = INFLUENCE_HEIGHT_UPPER;
	work->inf_below = INFLUENCE_HEIGHT_BELOW;

	work->my_num = OK_RB_Num;
	OK_RB_Num++;
	if ( GetResources( work, name, pvecRefer ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}


