//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	item_box.c
	アイテムボックス
	1999/10/14 S.Okajima
	$Id: item_box.c,v 1.1.1.3 2002/11/19 11:47:40 Yoshizawa1 Exp $

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

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"




//#define	ITEM_FIND_ON








#define	BODY_FLAG	(DG_FLAG_ONEPIECE|DG_FLAG_IRREACTION)
#define	LABEL_FLAG	(DG_FLAG_ONEPIECE|DG_FLAG_IRREACTION)
#define	SHADOW_FLAG	(DG_FLAG_ONEPIECE|DG_FLAG_SHADE)




#define	SCREEN_NEAR	( 51.0f )


#define	N_PRIMS		(1)
#define	N_VERTS		(32)

#define	FULL_ANIM_TIME			(60)
#define	SHAKE_WIDTH				(200.0f)

#define	DEFAULT_WIDTH			(200.0f)
#define	DEFAULT_HEIGHT			(100.0f)
#define	DEFAULT_LENGTH			(10000)
#define	DEFAULT_STABLE_LENGTH	(3000)
#define	SIZE			(100)
#define	DEFAULT_ALPHA	(24)
#define	P_RGB_MAX		(92)

#define	CANT_TAKE_TIME	( 30 )
#define	LIFE_TIME		( 1800 )

#define	THINK_COUNT1	(120)
#define	THINK_COUNT2	(10)
#define	THINK_COUNT3	(10)

#define	SINGLE_CHECK_COUNT	(60)
#define	CHECK_EDGE_COUNT	(40)

#define	STABLE_COUNT		(3)

#define	INIT_SPEED				(80.0f)
#define	INIT_SPEED_DOG			(70.0f)

#define	ROT_SPEED_MAX		(128)
#define	ROT_SPEED_STATIC	(32)

#define	INFO_NUM		(4)

//#define	DECAY_RATIO	( 0.6f )
#define	DECAY_RATIO	( 0.6f )

#define	SCALE_UP_Y	( 1.0001f )
#define	SCALE_UP_XZ	( 1.2f )

#define	UNDER_LIMIT	( 20.0f )

#define	SHIFT_SEG	( 1.0f )

////////////////////////////////
extern float GM_WaterLevel;
extern int GM_GetItem( int spec, int id, int n );
extern int ok_flush_status;
extern int DG_GetLightMatrixFix( FVECTOR *pos, FMATRIX *light );
extern FVECTOR OK_ItemGetBoundary;
////////////////////////////////

enum {
	FREE_FALL_PHASE = 0,
	PRE_STATIC_PHASE,
	STATIC_PHASE,
	DISAPEAR_PHASE1,
	DISAPEAR_PHASE2,
	END_PHASE
};


typedef	struct	{
	GV_ACT_EX	actor ;
	int			item_info[ INFO_NUM ] ;	// 0:item or weapon  1:item_id  2:item_num
	int			name;
	int			where;
	int			dogtag_name;
	int			dogtag_id;
	int			watch_invisible;
	int			display_information_flag;
	int			display_information_count;
	int			already_get_flag;
	int			re_in_flag;
	int			re_in_count;
	int			scn_prog_flag;
	int			invisible_flag;
	int			invisible_shadow_flag;
	int			splash;
	int			phase;
	int			exec ;
	int			proc ;
	int			auto_vanish_proc;
	int			before_status;
	int			count;
	int			think_count;
	int			dg_group_id;
	int			pat;
	int			rot_temp;
	int			locate_flag;
	int			locate_x;
	int			locate_y;
	int			free_fall_flag;
	int			hzx_group_id;

	float		tmp_height;
	float		height;
	float		height_scn;
	float		scale_param;
	float		level;
	float		obj_size;

	FMATRIX		lights[ 2 ] ;
	FMATRIX		shadow_matrix ;

	FVECTOR		temp;
	FVECTOR		floor_fvec;
	FVECTOR		pos;
	FVECTOR		force;
	FVECTOR		center_shift;
	FVECTOR		display_position_on_world;
	FVECTOR		pos_for_dog_tag;

	SVECTOR		rot_vec;

	DG_OBJS		*objs ;
	DG_OBJS		*objs_label ;
	DG_OBJS		*shadow ;
	DG_PRIM2	*prim ;
	char		*display_message;

	CONTROL_NOEVT		control;
#ifdef ITEM_FIND_ON
	int			ef_flag;
	ENEFIND 	ef ;
#endif

} Work ;

static	FMATRIX	lights_shadow[2] = {
	{ {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}} },
	{ {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}} }
	} ;

static	void NearSegShift( FVECTOR *vector, FVECTOR *mov, float size, int id )
{
	if( HZX_NearHazardCheck( id,
							 mov,
							 size,
							 //HZX_CHK_ALL,0,	// 修正 kunibe 2001.08.24
							 //HZX_SEG_NO_PLAYER | HZX_FLOOR_NO_PLAYER, 0,
							 HZX_SEG_NO_PLAYER, 0,
							 size) ){
		HZX_GetReactVector( vector );
		_sceVu0Normalize( vector, vector );
		return;
	}
	DG_COPY_VEC( vector, &DG_ZeroVector );
}


static void WatchHeight( Work *work )
{
	int	floor_flag;
	float		flr_height[2];
	FVECTOR		fvtemp;

	DG_COPY_VEC( &fvtemp, &work->control.mov );
	fvtemp.vy += work->height;

	floor_flag = HZX_LevelHazardCheck(
	                 work->hzx_group_id,
	                 &fvtemp,
	                 HZX_CHK_FLOOR,
	                 HZX_FLOOR_NO_PLAYER|HZX_FLOOR_PITFALL );
	if( floor_flag & 1 ){
		HZX_GetLevelHeight( flr_height );
		work->control.mov.vy = flr_height[0] + 1.0f;
		work->height = fvtemp.vy - flr_height[0];
	}
}

/* 取得時実行 */
static	void	ExecEndFunction( Work *work )
{
	GCL_ARGS	args ;
	int			output_information[INFO_NUM + 1];

	output_information[0] = work->item_info[0];
	output_information[1] = work->item_info[1];
	output_information[2] = work->item_info[2];
	output_information[3] = work->name ;
	output_information[4] = work->dogtag_id ;

	args.argv = output_information ;
	args.argc = INFO_NUM + 1;
	if ( work->exec != 0 ) {
		GM_ExecBlock( ( char * )work->exec, &args ) ;
	} else if ( work->proc > 0 ) {
		GM_ExecProc( work->proc, &args ) ;
	}
}

/* 自然消滅時実行 */
static	void	ExecAutoVanishFunction( Work *work )
{
	GCL_ARGS	args ;
	int			output_information[INFO_NUM + 1];

	if ( work->auto_vanish_proc > 0 ) {

//printf("ExecAutoVanishFunction::::::::::::::::::::::come\n");

		output_information[0] = work->item_info[0];
		output_information[1] = work->item_info[1];
		output_information[2] = work->item_info[2];
		output_information[3] = work->name ;
		output_information[4] = work->dogtag_id ;

		args.argv = output_information ;
		args.argc = INFO_NUM + 1;
		GM_ExecProc( work->auto_vanish_proc, &args ) ;
	}
}


static FVECTOR lgt = { 0.1f, 1.0f, 0.1f, 0.0f } ;

static	void MakeRandRotXZ( Work *work )
{
	float	ftemp;

	ftemp = (work->control.step.vy < 0.0f)? -work->control.step.vy: work->control.step.vy;
	if( ftemp < UNDER_LIMIT ) ftemp = 0.0f;
	ftemp = (ftemp < INIT_SPEED)? ftemp/INIT_SPEED: 1.0f;
//printf("ftemp:%f\n",ftemp);
	work->rot_vec.vx = (int)(frnd()*32.0f * ftemp);
	work->rot_vec.vy = (int)(frnd()*64.0f * ftemp);
	work->rot_vec.vz = 0;
//printf("work->rot_vec.vx:%d\n",work->rot_vec.vx);
//printf("work->rot_vec.vy:%d\n",work->rot_vec.vy);
}

static	void PutSplash( Work *work )
{
	int		j,k;
	int		clock;
	int		flag;
	int		alpha=0;
	float	param0=0.0f;
	float	param1=0.0f;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;
	FVECTOR	*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	if( ok_flush_status!=0 ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim ) ;
	}

	DG_COPY_VEC( &fvtemp1, &work->control.mov );
//	fvtemp1.vy += work->height;
//	fvtemp1.vy = fvtemp1.vy + work->tmp_height + work->obj_size*0.5f;
	fvtemp1.vy += work->tmp_height;

   //AR_PARTICLE_FULL
   DG_SwitchBuffPrim2( work->prim );
   clock = work->prim->buffer_clock;

	DG_SetPos( &DG_Chanls->eye_pers );
	

	DG_PutVector( &fvtemp1, &fvtemp0, 1 );

	flag=1;

	if( (fvtemp0.vz > fvtemp0.vw) ){
		flag=0;
	}else{
		if(fvtemp0.vw > 0.0f){
			param0 = fvtemp0.vw;
		}else{
			param0 = 0.0f;
			fvtemp0.vw = -fvtemp0.vw;
		}

		fvtemp0.vw *= 2.0f;

		alpha = (param0 < param1)? (int)param0: (int)param1;
		if( alpha > DEFAULT_LENGTH ){
			 flag=0;
		}else if( fvtemp0.vx < -fvtemp0.vw ){
			 flag=0;
		}else if( fvtemp0.vx >  fvtemp0.vw ){
			 flag=0;
		}else if( fvtemp0.vy < -fvtemp0.vw ){
			 flag=0;
		}else if( fvtemp0.vy >  fvtemp0.vw ){
			 flag=0;
		}
	}

	if( flag ){
		DG_VisiblePrim2( work->prim );
		if( alpha < DEFAULT_STABLE_LENGTH){
			alpha = DEFAULT_ALPHA;
		}else{
			alpha = DEFAULT_ALPHA * (DEFAULT_LENGTH - alpha) / (DEFAULT_LENGTH - DEFAULT_STABLE_LENGTH);
		}

		pos     = work->prim->pos[clock] ;
		uvrgbwh = work->prim->uvrgb[clock] ;
		for ( j = 0 ; j < N_PRIMS ; j++ ){
			for ( k = 0 ; k < N_VERTS ; k++ ){
				pos->vx = fvtemp1.vx + DEFAULT_WIDTH*frnd();
				pos->vy = fvtemp1.vy + work->obj_size*0.5f + DEFAULT_HEIGHT*frnd();
				pos->vz = fvtemp1.vz + DEFAULT_WIDTH*frnd();

//if(GV_Time%300==0)printf("%f %f %f\n",pos->vx,pos->vy,pos->vz);

				pos++;

				uvrgbwh->a = alpha;
				uvrgbwh++;
			}
		}
	}else{
		if( work->prim!=NULL ) DG_InvisiblePrim2( work->prim ) ;
	}

}



static	void MakeShadowMatrix( Work *work, FMATRIX *out_mat, FMATRIX *src_mat )
{
	FMATRIX	tmp_mat;

/*
	_sceVu0ScaleVector( (FVECTOR *)src_mat->m[0], (FVECTOR *)src_mat->m[0], 10.0f );
	_sceVu0ScaleVector( (FVECTOR *)src_mat->m[1], (FVECTOR *)src_mat->m[1], 10.0f );
	_sceVu0ScaleVector( (FVECTOR *)src_mat->m[2], (FVECTOR *)src_mat->m[2], 10.0f );
*/

	_sceVu0DropShadowMatrix( &tmp_mat,
			     &lgt,
			     work->floor_fvec.vx,
			     work->floor_fvec.vy,
			     work->floor_fvec.vz,
			     0/*平行光源*/ ) ;
	_sceVu0MulMatrix( out_mat, &tmp_mat, src_mat );
	out_mat->m[3][0] = work->pos.vx;
	out_mat->m[3][1] = work->level;
	out_mat->m[3][2] = work->pos.vz;
}

static	void ItemPutObjs( Work *work )
{
	FMATRIX	mat0;
	FVECTOR	fvtemp;
	FVECTOR	shake;
	CONTROL	*ctrl;

	ctrl = (CONTROL *)&work->control;

	if( work->objs       != NULL ) GM_GroupObjs( work->objs,       ctrl->map );
	if( work->objs_label != NULL ) GM_GroupObjs( work->objs_label, ctrl->map );
	if( work->shadow     != NULL ) GM_GroupObjs( work->shadow,     ctrl->map );
	if( work->splash             ) GM_GroupPrim2( work->prim,      ctrl->map );

	DG_COPY_VEC( &fvtemp, &ctrl->mov );
	fvtemp.vy = fvtemp.vy + work->tmp_height - work->obj_size;

	DG_SetPos2( &fvtemp, &ctrl->rot ) ;
	DG_MovePos( &work->center_shift ) ;
	if( work->re_in_count > 0 ){
		work->re_in_count--;
		shake.vx = SHAKE_WIDTH * frnd() * (float)work->re_in_count / (float)FULL_ANIM_TIME;
		shake.vy = 0.0f;
		shake.vz = SHAKE_WIDTH * frnd() * (float)work->re_in_count / (float)FULL_ANIM_TIME;
	}else{
		DG_COPY_VEC( &shake, &DG_ZeroVector );
	}

	if( work->phase >= DISAPEAR_PHASE1 ){
		DG_GetPos( &mat0 ) ;
		fvtemp.vx = fvtemp.vz = (2.0f - work->scale_param);
		fvtemp.vy = work->scale_param;
//printf("%f %f\n",fvtemp.vx,fvtemp.vy);
		_sceVu0ScaleVector( (FVECTOR *)mat0.m[0], (FVECTOR *)mat0.m[0], fvtemp.vx );
		_sceVu0ScaleVector( (FVECTOR *)mat0.m[1], (FVECTOR *)mat0.m[1], fvtemp.vy );
		_sceVu0ScaleVector( (FVECTOR *)mat0.m[2], (FVECTOR *)mat0.m[2], fvtemp.vz );
		DG_SetPos( &mat0 ) ;
		DG_MovePos( &shake ) ;

		DG_GetPos( &mat0 ) ;
		DG_COPY_VEC( &work->pos_for_dog_tag, (FVECTOR *)mat0.m[3] ); // ドッグタグキラリ用マトリクス

		DG_PutObjs( work->objs );
		if( work->objs_label!=NULL )DG_PutObjs( work->objs_label );

//		if( ctrl->grounded & 1 ){
			MakeShadowMatrix( work, &work->shadow_matrix, &mat0 );
			DG_SetPos( &work->shadow_matrix ) ;
			DG_PutObjs( work->shadow );
//		}
	}else{
		DG_MovePos( &shake ) ;

		DG_GetPos( &mat0 ) ;
		DG_COPY_VEC( &work->pos_for_dog_tag, (FVECTOR *)mat0.m[3] ); // ドッグタグキラリ用マトリクス

		DG_PutObjs( work->objs );
		if( work->objs_label!=NULL )DG_PutObjs( work->objs_label );

//		if( ctrl->grounded & 1 ){
			if( work->phase != STATIC_PHASE ){
				MakeShadowMatrix( work, &work->shadow_matrix, &DG_UnitMatrix );
			}else{
				work->shadow_matrix.m[3][0] = ctrl->mov.vx;
				work->shadow_matrix.m[3][1] = work->level;
				work->shadow_matrix.m[3][2] = ctrl->mov.vz;
			}
//		}

#ifndef PSX2
		/* パカパカをなくすため */
		work->shadow_matrix.m[3][1] += 2.0f ;
#endif

		DG_SetPos( &work->shadow_matrix ) ;
		DG_MovePos( &shake ) ;
		DG_RotatePos( &ctrl->rot );
		DG_PutObjs( work->shadow );
	}


}

static	int GetCheck( FVECTOR *fv0, FVECTOR *fv1, float height )
{
//printf("a0:%f %f %f\n",fv0->vx,fv0->vy,fv0->vz);
//printf("a1:%f %f %f\n",fv1->vx,fv1->vy,fv1->vz);
//printf("a2:%f\n",height);

	if( fv0->vy - OK_ItemGetBoundary.vy < fv1->vy+height
	 && fv0->vy + OK_ItemGetBoundary.vy > fv1->vy+height ){	// 同じ平面とみなす
		if( fv0->vx - OK_ItemGetBoundary.vx < fv1->vx
		 && fv0->vx + OK_ItemGetBoundary.vx > fv1->vx ){
			if( fv0->vz - OK_ItemGetBoundary.vz < fv1->vz
			 && fv0->vz + OK_ItemGetBoundary.vz > fv1->vz ){	// アイテムゲット可能
				return 1;
			}
		}
	}
	return 0;
}


static void GoPreStatic( Work *work )
{
	DG_COPY_VEC( &work->control.step, &DG_ZeroVector );
	work->rot_temp = work->rot_vec.vy;
	work->phase = PRE_STATIC_PHASE;		// 次のフェーズへ
	work->think_count = THINK_COUNT1;
}

#ifdef ITEM_FIND_ON
static void InformationForEnemy( Work *work )
{
	work->ef_flag = 1;
	GM_SetEneFind( &work->ef, &work->pos, work->control.addr, EF_TYPE_ITEM ) ;
	GM_PutEneFind( &work->ef ) ;
}
#endif


// 2001.08.24 Yuuta Kunibe
// ハザード検索した位置をずらしてしまうと
// 床をすり抜けて見えるタイミングがあるので
// 移動はActControlで算出されたmovを有効とし
// ここではstepの更新のみとすること！
static void FreeFallSub( Work *work, FVECTOR *before_pos )
{
	int		hit;
	float	f_abs;
	FVECTOR	fvtemp1;
	HZX_SEG	*seg ;
	CONTROL	*ctrl;

	ctrl = (CONTROL *)&work->control;
	hit = 0;


	
#if 1
	if ( ctrl->n_touches != 0 ) {
//printf("seg:::::::::::::::::\n");
		seg=ctrl->segs[0];
		fvtemp1.vx = seg->p2.z - seg->p1.z ;
		fvtemp1.vy = 0.0f ;
		fvtemp1.vz = seg->p1.x - seg->p2.x ;
//printf("%f:%f\n",fvtemp1.vx,fvtemp1.vz);
//		if( fvtemp1.vx!=0.0f || fvtemp1.vz!=0.0f ){
			/* 離れつつある時は反射させない */
		_sceVu0Normalize( &fvtemp1, &fvtemp1 ) ;
//			if ( _sceVu0InnerProduct( &fvtemp1, &ctrl->step ) >= 0 ) {
		MakeRandRotXZ( work );
		DG_ReflectVector( &fvtemp1, &ctrl->step, &ctrl->step );
//				ctrl->step.vy *= -DECAY_RATIO;
#if 0
		ctrl->step.vx  = -ctrl->step.vx;
		ctrl->step.vz  = -ctrl->step.vz;
#else
		ctrl->step.vx *= 0.95f;
		ctrl->step.vz *= 0.95f;
#endif
//			}
//		}
//printf("before:::::::::::::::::::::::::::::::::\n");
//		DG_COPY_VEC( &ctrl->mov, before_pos );
	}
#endif

	if ( (ctrl->grounded & 1) && ( work->free_fall_flag== 0) ){					/* FLOOR */

		hit = 1;
		MakeRandRotXZ( work );

		if( work->before_status > STABLE_COUNT ){
			GoPreStatic( work );
#ifdef ITEM_FIND_ON
			InformationForEnemy( work );
#endif
			return;
		}else{
#if 0
			if(  OK_CheckFloorEdge( &fvtemp1, &ctrl->mov, work->obj_size*2.0f, ctrl->level[0] ) ){	/* エッジだった */
				ctrl->step.vy *= -DECAY_RATIO;
				if( ctrl->step.vy < -UNDER_LIMIT*2.0f ){
					ctrl->step.vy = -UNDER_LIMIT*2.0f;
				}else if( ctrl->step.vy > UNDER_LIMIT*2.0f ){
					ctrl->step.vy =  UNDER_LIMIT*2.0f;
				}
//				ctrl->step.vx  =  ctrl->step.vx*0.8f + fvtemp1.vx*INIT_SPEED*0.1f;
//				ctrl->step.vz  =  ctrl->step.vz*0.8f + fvtemp1.vz*INIT_SPEED*0.1f;
				ctrl->step.vx += UNDER_LIMIT * frnd();
				ctrl->step.vz += UNDER_LIMIT * frnd();
			}else{
				ctrl->step.vy *= -DECAY_RATIO;
				ctrl->step.vx  =  ctrl->step.vx* DECAY_RATIO;
				ctrl->step.vz  =  ctrl->step.vz* DECAY_RATIO;
			}
#else
			ctrl->step.vy *= -DECAY_RATIO;
			ctrl->step.vx  =  ctrl->step.vx* DECAY_RATIO;
			ctrl->step.vz  =  ctrl->step.vz* DECAY_RATIO;
#endif


			NearSegShift( &fvtemp1, &ctrl->mov, work->obj_size, work->hzx_group_id );
			_sceVu0ScaleVector( &fvtemp1, &fvtemp1, SHIFT_SEG );
			_sceVu0AddVector( &ctrl->step, &ctrl->step, &fvtemp1 );

		}
		f_abs=( ctrl->step.vy > 0 )?ctrl->step.vy:-ctrl->step.vy;
		if( f_abs < UNDER_LIMIT ){
			work->before_status++;
		}else{
         int pan;
         float bp_angle;
         pan = GM_SeGetPan( &ctrl->mov, GM_SEMODE_BOMB, &bp_angle );
			if( work->item_info[0]==0  &&  work->item_info[1]==33  ){	// DOGTAG
				GM_SeSet3D( pan, GM_MAX_VOL, SD_A_DOGTAG01, bp_angle );	/* 跳ね返り音 */
			}else{
				GM_SeSet3D( pan, GM_MAX_VOL, SD_W_MAGAZI01, bp_angle );	/* 跳ね返り音 */
			}
			work->before_status = 0;
		}
	}

	if( hit ){
//		ctrl->step.vx += frnd()*ctrl->step.vx*0.5f;
//		ctrl->step.vz += frnd()*ctrl->step.vz*0.5f;
	}else{
		ctrl->step.vy += P_GRAVITY;
	}

	if( work->dogtag_name != -1){	// ドッグタグ
		ctrl->turn.vx = 0;
		ctrl->rot.vx  = 0;
	}else{
		ctrl->turn.vx &= 4095;
		if( ctrl->turn.vx < 2048 ){
			ctrl->turn.vx = ctrl->turn.vx * 15 / 16;
		}else{
			ctrl->turn.vx = 4096 - ((4096 - ctrl->turn.vx) * 15 / 16);
		}
		ctrl->turn.vx += work->rot_vec.vx;
		ctrl->rot.vx   = ctrl->turn.vx;
	}

	ctrl->mov.vw = 1.0f;

}

enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_SHADOW_OFF,
	REQ_WATCH_HZD,
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
			work->invisible_shadow_flag = 1;
			break;
		  case REQ_ON:
			work->invisible_flag = 0;
			work->invisible_shadow_flag = 0;
			break;
		  case REQ_SHADOW_OFF:
			work->invisible_flag = 0;
			work->invisible_shadow_flag = 1;
			break;
		  case REQ_WATCH_HZD:
			work->free_fall_flag = 1;
			work->rot_vec.vx = 60.0f;
			work->rot_vec.vy = 250.0f;
			work->rot_vec.vz = 0;
			work->rot_vec.vy = ROT_SPEED_MAX;
			work->rot_temp = work->rot_vec.vy;
			work->control.turn.vx = irnd()%4096;
			work->control.turn.vy = irnd()%4096;
			work->control.turn.vz = 0;
			work->phase = FREE_FALL_PHASE;
			work->before_status = -30;
			work->control.step.vx = 0.0f;
			work->control.step.vy = 0.0f;
			work->control.step.vz = 0.0f;
			work->control.mov.vy+= work->tmp_height;
			work->tmp_height = work->height = work->height_scn = 0.0f;
#if 0
			{
				int	floor_flag;
				float		flr_height[2];
				FVECTOR		fvtemp;

				DG_COPY_VEC( &fvtemp, &work->control.mov );
				fvtemp.vy += work->tmp_height;

				floor_flag = HZX_LevelHazardCheck(
	                 work->hzx_group_id,
	                 &fvtemp,
	                 HZX_CHK_FLOOR,
	                 HZX_FLOOR_NO_PLAYER|HZX_FLOOR_PITFALL );
				if( floor_flag & 1 ){
					HZX_GetLevelHeight( flr_height );
					work->control.mov.vy = flr_height[0];
					work->tmp_height = work->height = work->height_scn;
				}
			}
#else
#endif

			break;
		  case REQ_KILL:
		  case -1:
		  case GV_MESSAGE_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			break;
		}
		msg--;
	}
}

static	void SetDisplayInfomation( Work *work, int num )
{
	work->display_information_flag = num;
	DG_COPY_VEC( &work->display_position_on_world, &work->control.mov );
	work->display_position_on_world.vy += work->tmp_height;
	work->display_information_count = OK_DISPLAY_INFORMATION_TIME0;
}


static	void Print3D_perse( Work *work, FVECTOR *pos, int num, int alpha )
{
	float		tmp ;
	FVECTOR		ret ;   
	char	*strings_i;
	char	*strings_w;
	DG_CHANL	*cp ;
	int		flag;
	extern int PL_SubjectMove ;

	if( alpha <= 0 ) return;

	if( GM_CheckGameStatus(STATE_PLAY_DEMO) ) return;

	flag = 0;

	/* 位置決定 */
	cp = DG_Chanl(0) ;
	tmp = pos->vw ;
	pos->vw = 1.0F ;
	_sceVu0ApplyMatrix(&ret, &cp->eye_pers, pos) ;
	pos->vw = tmp ;

	/* カメラの範囲内に入っているかどうか */
	if(ret.vz > ret.vw) flag = 1 ;
	if(ret.vw < 0) ret.vw = - ret.vw ;

	if((ret.vx > ret.vw*2.0f) || (ret.vx < (-ret.vw)*2.0f)) flag = 1 ;
	if((ret.vy > ret.vw) || (ret.vy < (-ret.vw))) flag = 1 ;

	if( GM_CheckPlayerStatus(PLAYER_INTRUDE) ){ // イントルード時はＯＮ
		flag = 0;
	}

	if( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ){
		if( ret.vy > ret.vw*0.5f ){
			ret.vy = ret.vw*0.5f;
		}else if( ret.vy < -ret.vw*0.5f ){
			ret.vy =-ret.vw*0.5f;
		}
		if( ret.vx > ret.vw*0.5f ){
			ret.vx = ret.vw*0.5f;
		}else if( ret.vx < -ret.vw*0.5f ){
			ret.vx =-ret.vw*0.5f;
		}
	}

	if ( PL_SubjectMove ) {
		work->locate_x = DRAW_WIDTH/2 ;
		work->locate_y = DRAW_HEIGHT/2 ;
	} else if( !flag ){
		work->locate_flag = 1;	// 表示すべき位置計算初期化された
		work->locate_x = (int)((ret.vx / ret.vw) * DRAW_WIDTH/2) + DRAW_WIDTH/2 ;
		work->locate_y = (int)((ret.vy / ret.vw) * DRAW_HEIGHT/2) + DRAW_HEIGHT/2 - OK_SHIFT_UPPER;
	}else if( !work->locate_flag ){
		return;
	}

//	if( work->display_message == "" ){ /*yano 2002.03.27 環境依存なので変更しました。*/
	if( work->display_message[0] == '\0' ){
		strings_i = GM_ItemNames[work->item_info[1]];
		strings_w = GM_WeaponNames[work->item_info[1]];
	}else{
		strings_i = work->display_message;
		strings_w = work->display_message;
	}

	if( work->dogtag_name != -1 ){
		strings_i = (char *)work->dogtag_name;
		strings_w = (char *)work->dogtag_name;
	}


	if( work->locate_x > DRAW_WIDTH*2.0f ){
		work->locate_x = DRAW_WIDTH*2.0f;
	}else if( work->locate_x <-DRAW_WIDTH*2.0f ){
		work->locate_x =-DRAW_WIDTH*2.0f;
	}
	if( work->locate_y > DRAW_WIDTH*2.0f ){
		work->locate_y = DRAW_WIDTH*2.0f;
	}else if( work->locate_y <-DRAW_WIDTH*2.0f ){
		work->locate_y =-DRAW_WIDTH*2.0f;
	}

	MENU_SetAlphaMode( 0, 1, 0, 1, 0 );
	MENU_Locate( work->locate_x, work->locate_y, MENU_MODE_CENTER );
	switch( num ){
	  case 0:	//アイテム
		MENU_Color( OK_MENU_COL_R, OK_MENU_COL_G, OK_MENU_COL_B, alpha );
		MENU_Printf( "%s\n",strings_i ) ;
		break;
	  case 1:	//弾
		MENU_Color( OK_MENU_COL_R, OK_MENU_COL_G, OK_MENU_COL_B, alpha );
		MENU_Printf( "%s BULLET x %d\n",strings_w, work->item_info[2] ) ;
		break;
	  case 2:	//武器
		MENU_Color( OK_MENU_COL_R, OK_MENU_COL_G, OK_MENU_COL_B, alpha );
		MENU_Printf( "%s\n",strings_w ) ;
		break;

	  case 10:	//アイテム
		MENU_Color( OK_MENU_FULL_COL_R, OK_MENU_FULL_COL_G, OK_MENU_FULL_COL_B, alpha );
		MENU_Printf( "%s  FULL\n",strings_i ) ;
		break;
	  case 11:	//弾
		MENU_Color( OK_MENU_FULL_COL_R, OK_MENU_FULL_COL_G, OK_MENU_FULL_COL_B, alpha );
		MENU_Printf( "%s BULLET  FULL\n",strings_w ) ;
		break;
	  case 12:	//武器
		MENU_Color( OK_MENU_FULL_COL_R, OK_MENU_FULL_COL_G, OK_MENU_FULL_COL_B, alpha );
		MENU_Printf( "%s  FULL\n",strings_w ) ;
		break;

	  case 21:	//先に武器を取って下さい
		MENU_Color( OK_MENU_FULL_COL_R, OK_MENU_FULL_COL_G, OK_MENU_FULL_COL_B, alpha );
		MENU_Printf( "GET %s FIRST\n",strings_w ) ;
		break;
	}

}

static	void DisplayInfomation( Work *work )
{
	int	itemp=0;

	switch( work->display_information_flag ){
	  case 1:
		itemp = OK_MENU_COL_A * ( OK_DISPLAY_INFORMATION_TIME0 - work->display_information_count ) / OK_DISPLAY_INFORMATION_TIME0;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 2;
			work->display_information_count = OK_DISPLAY_INFORMATION_TIME1;
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0], itemp );
		break;
	  case 2:
		itemp = OK_MENU_COL_A;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 3;
			work->display_information_count = OK_DISPLAY_INFORMATION_TIME2;
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0], itemp );
		break;
	  case 3:
		itemp = OK_MENU_COL_A * work->display_information_count / OK_DISPLAY_INFORMATION_TIME2;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			GV_DestroyActor( work ) ;
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0], itemp );
		break;


	  case 11:
		itemp = OK_MENU_COL_A * ( OK_DISPLAY_INFORMATION_TIME0 - work->display_information_count ) / OK_DISPLAY_INFORMATION_TIME0;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 12;
			work->display_information_count = OK_DISPLAY_INFORMATION_TIME1;
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0]+10, itemp );
		break;
	  case 12:
		itemp = OK_MENU_COL_A;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 13;
			work->display_information_count = OK_DISPLAY_INFORMATION_TIME2;
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0]+10, itemp );
		break;
	  case 13:
		itemp = OK_MENU_COL_A * work->display_information_count / OK_DISPLAY_INFORMATION_TIME2;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 0 ; // 元に戻る
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0]+10, itemp );
		break;


	  case 21:
		itemp = OK_MENU_COL_A * ( OK_DISPLAY_INFORMATION_TIME0 - work->display_information_count ) / OK_DISPLAY_INFORMATION_TIME0;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 22;
			work->display_information_count = OK_DISPLAY_INFORMATION_TIME1;
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0]+20, itemp );
		break;
	  case 22:
		itemp = OK_MENU_COL_A;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 23;
			work->display_information_count = OK_DISPLAY_INFORMATION_TIME2;
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0]+20, itemp );
		break;
	  case 23:
		itemp = OK_MENU_COL_A * work->display_information_count / OK_DISPLAY_INFORMATION_TIME2;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 0 ; // 元に戻る
		}
		Print3D_perse( work, &work->display_position_on_world, work->item_info[0]+20, itemp );
		break;
	}

//printf("%d:%d:\n",itemp,work->display_information_flag);

}

static	void Act( Work *work )
{
	int	i;
	int	dg_group_id;
	CONTROL	*ctrl;
//	CONTROL	*where_ctrl;
	FVECTOR	before_pos;

	FVECTOR	step_tmp;


	work->watch_invisible = 0;

	CheckMesgParam( work );

	ctrl = (CONTROL *)&work->control;

	if( work->display_information_flag ) DisplayInfomation( work );

	if( work->phase == FREE_FALL_PHASE ){
		work->dg_group_id = dg_group_id = GM_GetDGGroupID( ctrl->map );
	}else{
		dg_group_id = work->dg_group_id;
	}

//printf("item:%d :%d %d:::%d\n",work->pat,work->phase,STATIC_PHASE,dg_group_id);

//	if( (   work->pat   == 0
//		 && work->phase == STATIC_PHASE
////		 && !( dg_group_id & DG_CurrentGroupID )
//		 ) || (
//		    work->invisible_flag
//		 ) ){
//		work->objs->flag       |= DG_FLAG_INVISIBLE ;
//		work->objs_label->flag |= DG_FLAG_INVISIBLE ;
//		work->shadow->flag     |= DG_FLAG_INVISIBLE ;
//		return ;
//	}else{
		work->objs->flag       &= ~DG_FLAG_INVISIBLE ;
		if( work->objs_label!=NULL )work->objs_label->flag &= ~DG_FLAG_INVISIBLE ;
		if( work->invisible_shadow_flag ){
			work->shadow->flag     |= DG_FLAG_INVISIBLE ;
		}else{
			work->shadow->flag     &= ~DG_FLAG_INVISIBLE ;
		}
//	}

	GM_GroupObjs( work->objs, ctrl->map ) ;
	GM_GroupObjs( work->objs_label, ctrl->map ) ;
	GM_GroupObjs( work->shadow, ctrl->map ) ;

	if( ctrl->mov.vy < GM_WORLD_LIMIT_BOTTOM ){
		GV_DestroyActor( work ) ;
		return;
	}

	if( !work->already_get_flag ){
		switch( work->phase ){
		  case FREE_FALL_PHASE:
//printf("FREE_FALL_PHASE:%f %f %f\n",ctrl->mov.vx,ctrl->mov.vy,ctrl->mov.vz);
			DG_COPY_VEC( &before_pos, &ctrl->mov );


			// stepを確保しておく
			DG_COPY_VEC( &step_tmp, &ctrl->step );
			// 移動はActControlに任せる
			GM_ActControl( ctrl );
			DG_COPY_VEC( &ctrl->step, &step_tmp );


			FreeFallSub( work, &before_pos );

			work->floor_fvec.vx = ctrl->level[0]->p1.h;
			work->floor_fvec.vy = ctrl->level[0]->p3.h;
			work->floor_fvec.vz = ctrl->level[0]->p2.h;
			DG_COPY_VEC( &work->pos, &ctrl->mov );
			work->level = ctrl->levels[0] + 5.0f;
			DG_GetLightMatrix( &ctrl->mov, work->lights );

			if( ( (work->pat!=0) || (work->free_fall_flag) )
			 && (ctrl->mov.vy < GM_WaterLevel ) ){
//				GoPreStatic( work );
				ExecAutoVanishFunction( work );
				GV_DestroyActor( work ) ;
				return;
			}
			break;
		  case PRE_STATIC_PHASE:
			work->think_count--;
			work->tmp_height = work->tmp_height*0.7f + work->height*0.3f;

			work->rot_vec.vy = ROT_SPEED_STATIC + (work->rot_temp-ROT_SPEED_STATIC) * work->think_count / THINK_COUNT1;

			ctrl->turn.vx &= 4095;
			if( ctrl->turn.vx < 2048 ){
				ctrl->turn.vx = ctrl->turn.vx * 15 / 16;
			}else{
				ctrl->turn.vx = 4096 - ((4096 - ctrl->turn.vx) * 15 / 16);
			}
			ctrl->rot.vx = ctrl->turn.vx;

/*
			NearSegShift( &fvtemp, &ctrl->mov, work->obj_size, work->hzx_group_id );
			_sceVu0ScaleVector( &ctrl->step, &fvtemp, SHIFT_SEG );
			GM_ActControl( ctrl );
*/

//printf("a:%f %f %f\n",ctrl->step.vx,ctrl->step.vy,ctrl->step.vz);
//printf("b:%f %f %f\n",ctrl->mov.vx,ctrl->mov.vy,ctrl->mov.vz);

//			if( work->think_count <= 0  ||  work->pat != 2 ){
			if( work->think_count <= 0 ){
				work->phase = STATIC_PHASE;
//				GM_FreeControl( (CONTROL *)&work->control ) ;
				DG_GetLightMatrixFix( &ctrl->mov, work->lights );
			}else{
				DG_GetLightMatrix( &ctrl->mov, work->lights );
			}
			break;
		  case STATIC_PHASE:

			work->tmp_height = work->height;
			work->rot_vec.vy = ROT_SPEED_STATIC;
			ctrl->rot.vx = ctrl->turn.vx = 0;
			if( work->count > LIFE_TIME  &&  work->pat != 0 ){
				work->phase = DISAPEAR_PHASE1;
				work->think_count = THINK_COUNT2;
//				GM_SeSet( GM_SeGetPan( &ctrl->mov, GM_SEMODE_BOMB ), GM_MAX_VOL, SD_A_ITEMKIE1 );	/* アイテム消滅音 */
				GM_SeSetMode( SD_A_ITEMKIE1, &ctrl->mov, GM_SEMODE_NORMAL ) ;	/* アイテム消滅音 */
			}

			if( work->pat!=0 ){
				int	floor_flag;
				float		flr_height[2];
				FVECTOR		fvtemp;

				DG_COPY_VEC( &fvtemp, &work->control.mov );
				fvtemp.vy += work->height;

				floor_flag = HZX_LevelHazardCheck(
	                 work->hzx_group_id,
	                 &fvtemp,
	                 HZX_CHK_FLOOR,
	                 HZX_FLOOR_PITFALL );
				if( floor_flag & 1 ){
					HZX_GetLevelHeight( flr_height );
//printf("a:%f\n",flr_height[0]);
					if( work->control.mov.vy - flr_height[0] > 1000.0f ){	// 落ちます
//printf("COME\n");
#if 1
						work->count = LIFE_TIME+1;
#else
						work->phase = FREE_FALL_PHASE;
						MakeRandRotXZ( work );
						work->rot_vec.vy = ROT_SPEED_MAX;
						work->rot_temp = work->rot_vec.vy;
						work->control.turn.vx = irnd()%4096;
						work->control.turn.vy = irnd()%4096;
						work->control.turn.vz = 0;
#endif
					}
				}
			}


			break;
		  case DISAPEAR_PHASE1:
			work->scale_param = (float)work->think_count * 0.75f / (float)THINK_COUNT2 + 0.25f;
			if( work->think_count-- <= 0 ){
				work->phase = DISAPEAR_PHASE2;
				work->think_count = THINK_COUNT3;
			}
			break;
		  case DISAPEAR_PHASE2:
			work->scale_param = (float)(THINK_COUNT2 - work->think_count) * 2.0f * 0.75f / (float)THINK_COUNT2 + 0.25f;
			if( work->think_count-- <= 0 ){
				ExecAutoVanishFunction( work );
				GV_DestroyActor( work ) ;
			}
			break;
		}
	}
	ctrl->turn.vy += work->rot_vec.vy;
	ctrl->rot.vy   = ctrl->turn.vy;
	ctrl->mov.vw  = 1.0f;	/* 投影マトリクス作成時に参照されてしまう（らしい） */

#ifdef DEBUG_MODE
	if( !work->already_get_flag && GM_VRStatus & GM_VR_CLEAR ){
		if( work->count > CANT_TAKE_TIME ){
			if( GetCheck( &GM_PlayerPosition, &ctrl->mov, work->tmp_height ) ){
				printf( "Damerashiiyo??????????????????????????????????? kore ga kitara Yamashita san he\n" ) ; 
			}
		}
	}
#endif /*DEBUG_MODE*/

	if( !work->already_get_flag && !(GM_VRStatus & GM_VR_CLEAR) ){
		if( work->count > CANT_TAKE_TIME ){
			if( GetCheck( &GM_PlayerPosition, &ctrl->mov, work->tmp_height ) ){
            int pan;
            float bp_angle;
            pan = GM_SeGetPan( &ctrl->mov, GM_SEMODE_BOMB, &bp_angle );
				work->watch_invisible = 1;
				switch( GM_GetItem( work->item_info[0], work->item_info[1], work->item_info[2] ) ){
				  case GM_ITEMGET_OK:
					GM_SeSet3D( pan, GM_MAX_VOL, SD_S_IGET02, bp_angle );	/* アイテム収得音 */
					SetDisplayInfomation( work, 1 );
					ExecEndFunction( work ) ;
//					GV_DestroyActor( work ) ;
					work->already_get_flag = 1;
					i = GM_N_WhereList;
					break;
				  case GM_ITEMGET_FULL:
					if( work->re_in_count <=0  &&  !work->re_in_flag ){
						work->re_in_flag = 1;
						work->re_in_count = FULL_ANIM_TIME;
						GM_SeSet3D( pan, GM_MAX_VOL, SD_S_FULL0005, bp_angle );	/* アイテム満杯 */
						SetDisplayInfomation( work, 11 );
					}
					i = GM_N_WhereList;
					break;
				  case GM_ITEMGET_GET_WEAPON_FIRST:
					if( work->re_in_count <=0  &&  !work->re_in_flag ){
						work->re_in_flag = 1;
						work->re_in_count = FULL_ANIM_TIME;
						GM_SeSet3D( pan, GM_MAX_VOL, SD_S_FULL0005, bp_angle );	/* アイテム満杯 */
						SetDisplayInfomation( work, 21 );
					}
					i = GM_N_WhereList;
					break;
				  case GM_ITEMGET_ALREADY:
					if( work->re_in_count <=0  &&  !work->re_in_flag ){
						work->re_in_flag = 1;
						work->re_in_count = FULL_ANIM_TIME;
						GM_SeSet3D( pan, GM_MAX_VOL, SD_S_FULL0005, bp_angle );	/* アイテム満杯 */
						SetDisplayInfomation( work, 31 );
					}
					i = GM_N_WhereList;
					break;
				}
			}else{
				if( work->re_in_count <=0 ){
					work->re_in_flag = 0;
				}
				work->locate_flag = 0;	// 表示すべき位置計算初期化許可
			}
		}


		if( !(work->watch_invisible && GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE)) ){
			if( work->splash ) PutSplash( work );
		}
	}else{
		if( work->objs!=NULL ) work->objs->flag       |= DG_FLAG_INVISIBLE ;
		if( work->objs_label!=NULL ) work->objs_label->flag |= DG_FLAG_INVISIBLE ;
		if( work->shadow!=NULL ) work->shadow->flag     |= DG_FLAG_INVISIBLE ;
		if( work->prim!=NULL ) DG_InvisiblePrim2( work->prim ) ;
	}

	ItemPutObjs( work );

	if( work->watch_invisible && GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) ){
		if( work->objs!=NULL ) work->objs->flag       |= DG_FLAG_INVISIBLE ;
		if( work->objs_label!=NULL ) work->objs_label->flag |= DG_FLAG_INVISIBLE ;
		if( work->shadow!=NULL ) work->shadow->flag     |= DG_FLAG_INVISIBLE ;
		if( work->prim!=NULL ) DG_InvisiblePrim2( work->prim ) ;
	}



	if( work->dogtag_name != -1 && !work->already_get_flag ){
		if( work->count%60==0 ){
			extern void *NewPointDogtagFlash( FVECTOR *pos );
			void *p0;
			p0 = NewPointDogtagFlash( &work->pos_for_dog_tag );
			GV_SetActorChild( work, p0 ) ;
		}
	}



	work->count++;




}

static	void Die( Work *work )
{
	if( work->objs       != NULL){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
	if( work->objs_label != NULL){
		DG_DequeueObjs( work->objs_label );
		DG_FreeObjs( work->objs_label );
	}
	if( work->shadow     != NULL){
		DG_DequeueObjs( work->shadow );
		DG_FreeObjs( work->shadow );
	}
	GM_FreeControl( (CONTROL *)&work->control ) ;

#ifdef ITEM_FIND_ON
	if( work->ef_flag ) GM_FreeEneFind( &work->ef ) ;
#endif

	if( work->splash ){
		work->prim = OK_FreePrim2( work->prim );
	}


}

/* 初期設定値を取得 */
static	int	GetOptionValue( Work *work )
{
	int	data1;
	DG_DEF		*def ;

	work->display_message = "";
	/* 収得時メッセージ */
	if ( GCL_GetOption( 'g' ) != NULL ){
		work->display_message = GCL_GetNextString();
	}

	work->splash = 0;
	if ( GCL_GetOption( 'r' ) != NULL ){
		work->splash = GCL_GetNextInt();
	}

	work->dogtag_name = -1;
	if ( GCL_GetOption( 't' ) != NULL ){
		work->dogtag_name = GCL_GetNextInt();
	}

	work->dogtag_id = -1;
	if ( GCL_GetOption( 'u' ) != NULL ){
		work->dogtag_id = GCL_GetNextInt();
	}

	if ( GCL_GetOption( 'k' ) != NULL ){
		data1 = GCL_GetNextInt();
		def   = (DG_DEF*)GV_GetCache( GV_CacheID( data1, 'k' ) ) ;
		ASSERT( def != NULL );
		if ( def == NULL ){
			printf("ERR!! NO ITEM MODEL!!\n");
			return 0;
		}
		work->objs   = DG_MakeObjs( def, BODY_FLAG, 0 );
	}else{
		printf("ERR!! NO ITEM MODEL!!\n");
		return 0;
	}

	if ( GCL_GetOption( 's' ) != NULL ){
		data1 = GCL_GetNextInt();
		def   = (DG_DEF*)GV_GetCache( GV_CacheID( data1, 'k' ) ) ;
		ASSERT( def != NULL );
		if ( def == NULL ){
			printf("ERR!! NO ITEM SHADOW MODEL!!\n");
			return 0;
		}
		work->shadow = DG_MakeObjs( def, SHADOW_FLAG, 0 );
	}else{
		printf("ERR!! NO ITEM SHADOW MODEL!!\n");
		//アイテム本体を流用
		def   = (DG_DEF*)GV_GetCache( GV_CacheID( data1, 'k' ) ) ;
		ASSERT( def != NULL );
		if ( def == NULL ){
			printf("ERR:item_box.c:g2\n");
			return 0;
		}
		work->shadow = DG_MakeObjs( def, SHADOW_FLAG, 0 );
//		return 0;
	}

//	work->obj_size = (def->ux > def->uz)?def->ux*2.0f:def->uz*2.0f;
	work->obj_size = (def->ux > def->uz)?def->ux:def->uz;
	work->center_shift.vx = -(def->ux + def->lx) * 0.5f;
	work->center_shift.vy = -(def->uy + def->ly) * 0.5f;
	work->center_shift.vz = -(def->uz + def->lz) * 0.5f;

//printf("-----------::%f\n",work->obj_size);


	if ( GCL_GetOption( 'l' ) != NULL ){
		data1 = GCL_GetNextInt();
		def   = (DG_DEF*)GV_GetCache( GV_CacheID( data1, 'k' ) ) ;
		ASSERT( def != NULL );
		if ( def == NULL ){
			printf("ERR!! NO ITEM'S LABEL MODEL!!\n");
			return 0;
		}
		work->objs_label = DG_MakeObjs( def, LABEL_FLAG, 0 );
	}else{
		printf("ERR!! NO ITEM'S LABEL MODEL!!\n");
		work->objs_label = NULL;
//		return 0;
	}
	if ( GCL_GetOption( 'p' ) != NULL ){
		work->control.mov.vx = (float)GCL_GetNextInt() ;
		work->control.mov.vy = (float)GCL_GetNextInt() + 1.0f ;
		work->control.mov.vz = (float)GCL_GetNextInt() ;
		work->control.mov.vw = 1.0f;
	}else{
		printf("-------------:ERR:set item -p option\n");
		return 0;
	}

	work->height = 0.0f;
	if ( GCL_GetOption( 'h' ) != NULL ){
		work->height = (float)GCL_GetNextInt();
		work->height_scn = work->height;
	}

	if ( GCL_GetOption( 'f' ) != NULL ){		// アイテム・武器
		work->item_info[0] = GCL_GetNextInt();
	}else{
		printf("-------------:ERR:set item -f option\n");
		return 0;
	}
	if ( GCL_GetOption( 'i' ) != NULL ){		// ＩＤ
		work->item_info[1] = GCL_GetNextInt();
	}else{
		printf("-------------:ERR:set item -i option\n");
		return 0;
	}
	if ( GCL_GetOption( 'n' ) != NULL ){		// 個数
		work->item_info[2] = GCL_GetNextInt();
	}else{
		printf("-------------:ERR:set item -n option\n");
		return 0;
	}

//printf(":::::::::::::::::%d %d %d \n",work->item_info[0],work->item_info[1],work->item_info[2]);

	work->pat = 0;
	if ( GCL_GetOption( 'b' ) != NULL ){
		work->pat = GCL_GetNextInt();
	}


	switch( work->pat ){
	  case 0:
	  default:
		work->rot_vec.vy = ROT_SPEED_STATIC;

//		GoPreStatic( work );

		DG_COPY_VEC( &work->control.step, &DG_ZeroVector );
		work->rot_temp = work->rot_vec.vy;
		work->phase = STATIC_PHASE;		// 次のフェーズへ
		work->think_count = 0;

		WatchHeight( work );

		work->tmp_height = work->height;
//		GM_FreeControl( (CONTROL *)&work->control ) ;

		break;
	  case 1:
//		GM_SeSet( GM_SeGetPan( &work->control.mov, GM_SEMODE_BOMB ), GM_MAX_VOL, SD_S_ITEM0003 );
//		work->rot_vec.vy = ROT_SPEED_STATIC;
//		GoPreStatic( work );
//		break;
	  case 2:
        {
           int pan;
           float bp_angle;
           pan = GM_SeGetPan( &work->control.mov, GM_SEMODE_BOMB, &bp_angle );
		   GM_SeSet3D( pan, GM_MAX_VOL, SD_S_ITEM0003, bp_angle );	/* アイテム発生音 */
		   work->phase = FREE_FALL_PHASE;
		   MakeRandRotXZ( work );
		   work->rot_vec.vy = ROT_SPEED_MAX;
		   work->rot_temp = work->rot_vec.vy;
		   work->control.turn.vx = irnd()%4096;
		   work->control.turn.vy = irnd()%4096;
		   work->control.turn.vz = 0;
        }
		break;
	}


	/* 終了時実行 */
	work->exec = 0 ;
	work->proc = 0 ;
	if ( GCL_GetOption( 'x' ) != NULL ) {
		work->exec = GCL_GetNextInt() ;
	} else if ( GCL_GetOption( 'e' ) != NULL ) {
		work->proc = GCL_GetNextInt() ;
	}

	/* 自然消滅時実行 */
	work->auto_vanish_proc = 0;
	if ( GCL_GetOption( 'z' ) != NULL ) {
		work->auto_vanish_proc = GCL_GetNextInt() ;
	}

	return 1;

}

static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
			uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
			uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
			uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

			uvrgbwh0->w = uvrgbwh1->w = SIZE ;
			uvrgbwh0->h = uvrgbwh1->h = SIZE ;

			uvrgbwh0->r = uvrgbwh1->r = P_RGB_MAX ;
			uvrgbwh0->g = uvrgbwh1->g = P_RGB_MAX ;
			uvrgbwh0->b = uvrgbwh1->b = P_RGB_MAX ;
			uvrgbwh0->a = uvrgbwh1->a = DEFAULT_ALPHA ;

			uvrgbwh0++;	uvrgbwh1++;
		}
	}
}


static	int GetResources( Work *work, int name, int where )
{
	FVECTOR	fvtemp;
	SVECTOR	svtemp;
	FMATRIX	mat0;
	CONTROL	*ctrl;

	work->free_fall_flag = 0;

	work->re_in_flag = 0;
	work->re_in_count = 0;

	work->already_get_flag = 0;
	work->display_information_flag = 0;
	work->display_information_count = 0;
	work->invisible_flag = 0;
	work->invisible_shadow_flag = 0;


	work->name  = name;
	work->where = where;
	work->scale_param = 1.0f;

#ifdef ITEM_FIND_ON
	work->ef_flag = 0;
#endif

	if( work->scn_prog_flag==0 ){
		if ( GCL_GetOption( 'm' ) != NULL ) {
			work->where = GM_GetMapID( GCL_GetNextInt() ) ;
			printf("item_box.c: map information = OPTION:M");
		}
	}

	work->dg_group_id = GM_GetDGGroupID( work->where );
	work->hzx_group_id = GM_GetHzxGroupID( where );


	ctrl = (CONTROL *)&work->control;
	if ( GM_InitControl( ctrl, work->name, work->where ) < 0 ){
		printf("ERR:item_box.c:GM_InitControl:\n");
		return -1 ;
	}
	GM_ConfigControlMapCheck( ctrl );

	if( work->scn_prog_flag==0 ){
		if( !GetOptionValue( work ) ){
			printf("ERR:item_box.c:GetOptionValue:\n");
			return -1;
		}
	}else{
		work->display_message = "";
		DG_COPY_VEC( &work->control.mov, &work->temp );
	}
	GM_ConfigControlMapID( ctrl );
	GM_ConfigControlHazard( ctrl, (int)work->obj_size, (int)work->obj_size, (int)work->obj_size ) ;
	//GM_ConfigControlHzxHeight( ctrl, work->obj_size+1.0f, ctrl->mov.vy ) ;
	ctrl->seg_flag = HZX_SEG_NO_PLAYER ;
	ctrl->flr_flag = HZX_FLOOR_NO_PLAYER;//HZX_SEG_NO_PLAYER ;
	ctrl->skip_flag |= (CTRL_SKIP_TRAP | CTRL_SKIP_MESSAGE );
	GM_ActControl( ctrl );
//	GM_ConfigControlMapCheck( ctrl );
	work->floor_fvec.vx = work->control.level[0]->p1.h;
	work->floor_fvec.vy = work->control.level[0]->p3.h;
	work->floor_fvec.vz = work->control.level[0]->p2.h;
	work->level = work->control.levels[0] + 5.0f;

	if( work->pat == 1 ){
		ctrl->step.vx = 0.0f;
		ctrl->step.vy = rnd()*INIT_SPEED*0.2f + INIT_SPEED*0.8f;
		ctrl->step.vz = 0.0f;
	}else if( work->pat == 2 ){
		if( work->force.vw == 0.0f ){
			fvtemp.vx = 0.0f;
			fvtemp.vy = 0.0f;
			if( work->dogtag_id == -1 ){
				fvtemp.vz = rnd()*INIT_SPEED*0.2f + INIT_SPEED*0.8f;
				svtemp.vx = -(irnd()%512 - 256  + 1024 );
			}else{
				fvtemp.vz = rnd()*INIT_SPEED_DOG*0.2f + INIT_SPEED_DOG*0.8f;
				svtemp.vx = -(irnd()%128 - 64  + 1024 );
			}
			svtemp.vy = irnd()%4096;
			svtemp.vz = 0;
			DG_SetPos2( &DG_ZeroVector, &svtemp );
			DG_PutVector( &fvtemp, &ctrl->step, 1 );
		}else{
			_sceVu0Normalize( &work->force, &work->force );
			_sceVu0ScaleVector( &ctrl->step, &work->force, rnd()*INIT_SPEED*0.2f + INIT_SPEED*0.8f );
		}
	}else{
		ctrl->step.vx = 0.0f;
		ctrl->step.vy = 0.0f;
		ctrl->step.vz = 0.0f;
	}

	DG_QueueObjs( work->objs );
	DG_SetLightMatrix( work->objs, work->lights );
	DG_QueueObjs( work->shadow );
	DG_SetLightMatrix( work->shadow, lights_shadow );
	if( work->objs_label!=NULL )DG_QueueObjs( work->objs_label );
	if( work->objs_label!=NULL )DG_SetLightMatrix( work->objs_label, work->lights );

	work->before_status = 0;

	work->count = 0;

	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_GetPos( &mat0 ) ;
	MakeShadowMatrix( work, &work->shadow_matrix, &mat0 );

	DG_GetLightMatrix( &work->control.mov, work->lights );

	if( work->splash ){
		DG_PRIM2	*prim ;
		DG_TEX		*tex ;

		tex = DG_GetTexture( 7733153 /*"splash06_alp"*/ );
//		tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
		prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
		if(prim==NULL){
			printf("ERR:item_box.c:GM_MakePrim2:NULL\n");
			return -1;
		}
		InitPacket2( work, prim, tex );
		DG_VisiblePrim2( prim );
	}

	return 0 ;
}


void *NewItemBox( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->scn_prog_flag = 0;
		DG_COPY_VEC( &work->force, &DG_ZeroVector );
		work->force.vw = 0.0f;

		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


void *NewItemBox_Prog( int name,
                       int where,
                       int splash,
                       int model_name,
                       int shadow_name,
                       int label_name,
                       FVECTOR *pos,
                       float height,
                       int spec,
                       int id,
                       int num,
                       int pattern,
                       FVECTOR *force
                        )
{
	Work		*work ;
	DG_DEF		*def ;
   int pan;
   float bp_angle;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->scn_prog_flag = 1;
		work->dogtag_id = -1;
		work->dogtag_name = -1;

		def   = (DG_DEF*)GV_GetCache( GV_CacheID( model_name, 'k' ) ) ;
		ASSERT( def != NULL );
		if ( def == NULL ) return NULL;
		work->objs   = DG_MakeObjs( def, BODY_FLAG, 0 );
		def   = (DG_DEF*)GV_GetCache( GV_CacheID( shadow_name, 'k' ) ) ;
		ASSERT( def != NULL );
		if ( def == NULL ) return NULL;
		work->shadow = DG_MakeObjs( def, SHADOW_FLAG, 0 );
		work->obj_size = (def->ux > def->uz)?def->ux:def->uz;
		work->center_shift.vx = -(def->ux + def->lx) * 0.5f;
		work->center_shift.vy = -(def->uy + def->ly) * 0.5f;
		work->center_shift.vz = -(def->uz + def->lz) * 0.5f;
		def   = (DG_DEF*)GV_GetCache( GV_CacheID( label_name, 'k' ) ) ;
		ASSERT( def != NULL );
		if ( def == NULL ) return NULL;
		work->objs_label = DG_MakeObjs( def, LABEL_FLAG, 0 );
		work->temp.vx = pos->vx ;
		work->temp.vy = pos->vy + 1.0f ;
		work->temp.vz = pos->vz ;
		work->temp.vw = 1.0f;
		work->height = height;
		work->item_info[0] = spec;
		work->item_info[1] = id;
		work->item_info[2] = num;
		work->pat = pattern;

      pan = GM_SeGetPan( &work->control.mov, GM_SEMODE_BOMB, &bp_angle );

		switch( work->pat ){
		  case 0:
		  default:
			work->rot_vec.vy = ROT_SPEED_STATIC;
			GoPreStatic( work );
			break;
		  case 1:
			GM_SeSet3D( pan, GM_MAX_VOL, SD_S_ITEM0003, bp_angle );	/* アイテム発生音 */
			work->rot_vec.vy = ROT_SPEED_STATIC;
			GoPreStatic( work );
			break;
		  case 2:
			GM_SeSet3D( pan, GM_MAX_VOL, SD_S_ITEM0003, bp_angle );	/* アイテム発生音 */
			work->phase = FREE_FALL_PHASE;
			MakeRandRotXZ( work );
			work->rot_vec.vy = ROT_SPEED_MAX;
			work->rot_temp = work->rot_vec.vy;
			work->control.turn.vx = irnd()%4096;
			work->control.turn.vy = irnd()%4096;
			work->control.turn.vz = 0;
			break;
		}
		/* 終了時実行 */
		work->exec = 0 ;
		work->proc = 0 ;
		work->auto_vanish_proc = 0;
		if( force == NULL ){
			DG_COPY_VEC( &work->force, &DG_ZeroVector );
			work->force.vw = 0.0f;
		}else{
			DG_COPY_VEC( &work->force, force );
			work->force.vw = 1.0f;
		}

		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

int OK_ResetGetBoundary( void )
{
	if ( GCL_NextStr() == NULL ) return -1;
	OK_ItemGetBoundary.vx = (float)GCL_GetNextInt();
	OK_ItemGetBoundary.vy = (float)GCL_GetNextInt();
	OK_ItemGetBoundary.vz = (float)GCL_GetNextInt();

	return 1 ;
}


