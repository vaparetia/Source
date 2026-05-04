//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   c_box.c
   ダンボール
   
   1999/12/10 M.Sonoyama
   $Id: c_box.c,v 1.1.1.3 2002/11/19 11:50:14 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

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
#include	"libutl.h"
#include	"../../sonoyama/raiden/pl_work.h"

/*------------------------------------------------------------------*/

extern void *NewBoxPartsFall( FMATRIX *mat, FVECTOR *frots, int model_name, int map, int level, int max_level );
extern void *NewBreakBoxBody( int body_name, FMATRIX *world, FVECTOR *rots, int joint_num, int invisible_flags, int map );
extern void *NewCBoxSplash( FMATRIX *world, float size_x, float size_z, float size_h, float shift_z, int *flag, int pat );
extern void *NewCB_HoleLight( FMATRIX *world );
extern void *NewCB_WetEffect( DG_OBJS *, int ) ;
extern void *NewBoxBreakDust( FVECTOR *pos, FVECTOR *force, int map );
extern void CB_WetEffectChangeObjs( void *_work, DG_OBJS *objs );
extern void CB_WetEffectAdd( void *, int, FVECTOR *, float ) ;
extern int  OK_BodySplashFlag;

/*------------------------------------------------------------------*/

#define	BODY_FLAG		(DG_FLAG_SHADE|DG_FLAG_SHADOWMAKE)
#define	BODY_FLAG_SUB	(DG_FLAG_SHADE|DG_FLAG_NOFOG)

#define	BODY_NAME		( GV_StrCode( "cbx_a_mt" ) )	
#define	FUTA_NAME1		( GV_StrCode( "cbx_a_futa1_mt" ) )	
#define	FUTA_NAME2		( GV_StrCode( "cbx_a_futa2_mt" ) )	
#define	FUTA_NAME3		( GV_StrCode( "cbx_a_futa3_mt" ) )	
#define	FUTA_NAME4		( GV_StrCode( "cbx_a_futa4_mt" ) )	

#define	BODY_NAME_SUB	(11629709)	/* cbx_sub.kms */
//#define	BODY_NAME_SUB	(10979782)	/* cbx_a_sub.kms */
#define	INVISIBLE_FLAG	(0x00e01ffe)

#define	HUTA_PARAM	(30.0f)
#define	HUTA_WIDTH	(350.0f)
#define	HEIGHT		(800.0f)
#define	WIDTH_0		(700.0f)
#define	WIDTH_1		(900.0f)

#define	P00	(0x00000001)
#define	P01	(0x00000002)
#define	P02	(0x00000004)
#define	P03	(0x00000008)
#define	P04	(0x00000010)
#define	P05	(0x00000020)
#define	P06	(0x00000040)
#define	P07	(0x00000080)
#define	P08	(0x00000100)
#define	P09	(0x00000200)
#define	P10	(0x00000400)
#define	P11	(0x00000800)
#define	P12	(0x00001000)
#define	P13	(0x00002000)
#define	P14	(0x00004000)
#define	P15	(0x00008000)
#define	P16	(0x00010000)
#define	P17	(0x00020000)
#define	P18	(0x00040000)

#define	MAX_MODEL	(19)


#define	BOX_ALL_ON	(P00|P01|P02|P03|P04|P05|P06|P07|P08|P09|P10|P11|P12|P13|P14|P15|P16|P17|P18)
#define	BOX_DEFAULT	(P00|P01                                            |P13|P14|P15|P16        )
#define	BOX_BREAK1	(P00    |P02                                            |P14|P15|P16|P17|P18)
#define	BOX_BREAK2	(P00        |P03                                            |P15|P16|P17|P18)
#define	BOX_BREAK3	(P00            |P04    |P06    |P08    |P10    |P12            |P16|P17|P18)
#define	BOX_BREAK4	(P00                |P05|P06|P07|P08|P09|P10|P11|P12                |P17    )
#define	BOX_BREAK5	(                    P05|P06|P07|P08|P09|P10|P11|P12                |P17    )
#define	BOX_BREAK6	(0                                                                          )

#define	DAMAGE_LIMIT_1	(20)
#define	DAMAGE_LIMIT_2	(15)
#define	DAMAGE_LIMIT_3	(10)
#define	DAMAGE_LIMIT_4	(5)
#define	DAMAGE_LIMIT_5	(0)

#define	DAMAGE_SHIFT_Y	(50.0f)
#define	DAMAGE_ROTATE	(512.0f)

#define	MAX_WET_LEVEL	(256)
#define	MIN_WET_RANGE	(180.0f)
#define	MAX_WET_RANGE	(220.0f)

#define	SHIFT_MODEL_Y	(-200.0f)

#define	MAX_WET_POINT	(9)
#define	WET_STEP_TIME	(60*10)

#define	KAKATO_SIZE		(60.0f)

#define	CHECKANGLE_STEP	(PI/64.0f)
#define	ANGLE_LIMIT		(PI*(0.5f - 0.02f))

#define	HV_HEIGHT	(350.0f)
#define	HV_WIDTH_L	(450.0f)
#define	HV_WIDTH_S	(350.0f)

#define	FUTA_ANGLE_RATIO	(0.3f)

static	FVECTOR	HutaVerts[4][4]={
	{
		{ HV_HEIGHT,     0.0f, HV_WIDTH_L, 1.0f },
		{ HV_HEIGHT,     0.0f,-HV_WIDTH_L, 1.0f },
		{      0.0f,     0.0f, HV_WIDTH_L, 1.0f },
		{      0.0f,     0.0f,-HV_WIDTH_L, 1.0f },
	},
	{
		{-HV_HEIGHT,     0.0f, HV_WIDTH_L, 1.0f },
		{-HV_HEIGHT,     0.0f,-HV_WIDTH_L, 1.0f },
		{      0.0f,     0.0f, HV_WIDTH_L, 1.0f },
		{      0.0f,     0.0f,-HV_WIDTH_L, 1.0f },
	},
	{
		{ HV_WIDTH_S,    0.0f, HV_HEIGHT,  1.0f },
		{-HV_WIDTH_S,    0.0f, HV_HEIGHT,  1.0f },
		{ HV_WIDTH_S,    0.0f,      0.0f,  1.0f },
		{-HV_WIDTH_S,    0.0f,      0.0f,  1.0f },
	},
	{
		{ HV_WIDTH_S,    0.0f,-HV_HEIGHT,  1.0f },
		{-HV_WIDTH_S,    0.0f,-HV_HEIGHT,  1.0f },
		{ HV_WIDTH_S,    0.0f,      0.0f,  1.0f },
		{-HV_WIDTH_S,    0.0f,      0.0f,  1.0f },
	},
};
static	float	FutaRots[4];
static	float	FutaDest[4];
static	float	FutaStep[4];

enum {
	BOX_STATE_INITIAL=0,
	BOX_STATE_BREAK_1,
	BOX_STATE_BREAK_2,
	BOX_STATE_BREAK_3,
	BOX_STATE_BREAK_4,
	BOX_STATE_BREAK_5,
	BOX_STATE_FINISH
};

static	OBJECT	*BoxBody = NULL ;
static	int		BoxModel = 0 ;
static	int		BoxFlag     = 0 ;
static	int		BoxItemNo = 0 ;	/* ２つ以上同時に装備される（対戦など）とマズイ */
static	int		InvisibleFlags = 0 ;

static	int		OK_HoleFlag = 0;
static	int		OK_WetLevel = 0;

void *OK_cb_wet ; /* 濡れエフェクトワークへのポインタ */
/*------------------------------------------------------------------*/
typedef	struct {
	GV_ACT_EX		actor ;

	int			now_damage;
	int			before_damage;

	OBJECT		equip ;
	OBJECT		equip_sub ;

	float		body_damage_shift;
	float		body_damage_rot;
	float		body_damage_radian;

	FVECTOR		rots[MAX_MODEL];
	FVECTOR		before_mov;
	float		before_angle;
	int			break_mode;
	int			invisible_flags;

	CONTROL		*ctrl ;
	OBJECT		**body ;
	int			*unit ;
	int			*trigger ;
	int			rot_vx ;
	int			chanl ;
	int			time ;
	float		shift_z ;

	FVECTOR		size;
	void		*fall_body;
	int			timer;
	int			itNo ;

	int			bullet_efect_flag;

	DG_TEX_MOVEREPLACE *tex_replace ;
	int			futaname[ 4 ] ;
	int			model_name ;
} Work ;



/*------------------------------------------------------------------*/
static void CalcMat( FMATRIX *fmat_output, FMATRIX *fmat_input, int num, float angle )
{


	_sceVu0UnitMatrix( fmat_output );
	switch( num ){
	  case 0:
		_sceVu0RotMatrixZ( fmat_output, fmat_output,-angle );
		break;
	  case 1:
		_sceVu0RotMatrixZ( fmat_output, fmat_output, angle );
		break;
	  case 2:
		_sceVu0RotMatrixX( fmat_output, fmat_output, angle );
		break;
	  case 3:
		_sceVu0RotMatrixX( fmat_output, fmat_output,-angle );
		break;
	}
	_sceVu0MulMatrix( fmat_output, fmat_input, fmat_output );
}

/*------------------------------------------------------------------*/
static int CheckSeg( FVECTOR *pos, int id )
{
	int	flag;

	// 横線
	flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
		id,
		&pos[0],
		&pos[1],
		HZX_CHK_SEGMENT,
		HZX_SEG_NO_PLAYER,
		HZX_FLOOR_NO_PLAYER );
	if( !flag ){
		flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
			id,
			&pos[2],
			&pos[3],
			HZX_CHK_SEGMENT,
			HZX_SEG_NO_PLAYER,
			HZX_FLOOR_NO_PLAYER );
#if 1
		if( !flag ){
			flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
				id,
				&pos[2],
				&pos[0],
				HZX_CHK_SEGMENT,
				HZX_SEG_NO_PLAYER,
				HZX_FLOOR_NO_PLAYER );
			if( !flag ){
				flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
					id,
					&pos[1],
					&pos[3],
					HZX_CHK_SEGMENT,
					HZX_SEG_NO_PLAYER,
					HZX_FLOOR_NO_PLAYER );
			}
		}
#endif
	}
	return flag;
}


static void CheckFutaAtari( Work *work )
{
	int	i;
	int	map;
	int	id;
	FMATRIX	fmat;
	FVECTOR	fvtemp[4];
	DG_OBJ	*objs;

	map = work->equip.map_name;
	id = GM_GetHzxGroupID( map );
	objs = &work->equip.objs->objs[13];

	for( i=0; i<4; i++ ){
		//現在をチェック
		DG_SetPos( &objs[i].world );
		DG_PutVector( HutaVerts[i], fvtemp, 4 );
		if( CheckSeg( fvtemp, id ) ){	//まずいので上げる
			FutaRots[i]-= FutaStep[i];
			if( FutaRots[i] <-ANGLE_LIMIT ){
				FutaRots[i] = -ANGLE_LIMIT;
			}
			FutaStep[i]+= CHECKANGLE_STEP;
			if( FutaStep[i] > ANGLE_LIMIT ){
				FutaStep[i] = ANGLE_LIMIT;
				FutaRots[i] =-ANGLE_LIMIT;
			}
		}else{
			//下げられるかチェック
			CalcMat( &fmat, &objs[i].world, i, FutaStep[i] );
			DG_SetPos( &fmat );
			DG_PutVector( HutaVerts[i], fvtemp, 4 );
			if( !CheckSeg( fvtemp, id ) ){	//下げる
				FutaRots[i]+= FutaStep[i];
				FutaStep[i]*= 0.95f;
				if( FutaStep[i] < CHECKANGLE_STEP ){
					FutaStep[i] = CHECKANGLE_STEP;
				}
				if( FutaRots[i] >  ANGLE_LIMIT ){
					FutaRots[i] =  ANGLE_LIMIT;
				}
			}else{
				FutaStep[i]*= 0.5f;
				if( FutaStep[i] < CHECKANGLE_STEP ){
					FutaStep[i] = CHECKANGLE_STEP;
				}
			}
		}
	}
}

/*------------------------------------------------------------------*/

/* バンダナ消し */
static	void	VisibleBandana( int id )
{
	int			buf[ 2 ] ;

	if ( id < 0 ) buf[ 1 ] = 1 ;
	else if ( GM_ItemNum( id ) <= 10 ) {
		buf[ 1 ] = 1 ;
	} else {
		buf[ 1 ] = 0 ;
	}
	buf[ 0 ] = 0 ;
	PL_SendMessage( GV_StrCode( "バンダナ１" ), buf, 2 ) ;
	PL_SendMessage( GV_StrCode( "バンダナ２" ), buf, 2 ) ;
}

/*------------------------------------------------------------------*/

//calc_vec.vx 幅
//calc_vec.vy 幅
//calc_vec.vz 幅
//calc_vec.vw Ｚ方向シフト
void PutWet( FMATRIX *world, FVECTOR *calc_vec, void *cb_wet, int wet_level )
{
	int	i;
	FVECTOR	calc[MAX_WET_POINT];
	FVECTOR	width;
	FVECTOR	shift;
	FVECTOR	shift_half;
	float	ratio;
	float	range;

	ratio = (float)(wet_level)/(float)(MAX_WET_LEVEL);
	range = (MAX_WET_RANGE - MIN_WET_RANGE) * ratio + MIN_WET_RANGE;

	_sceVu0ScaleVector( &width, calc_vec, 0.5f );
	_sceVu0ScaleVector( &shift, calc_vec, ratio );
	_sceVu0ScaleVector( &shift_half, &shift, 0.25f );

	calc[0].vx = width.vx     - shift.vx;
	calc[0].vy = calc_vec->vy - shift.vy                   + SHIFT_MODEL_Y;
	calc[0].vz = width.vz                     - calc_vec->vw;

	calc[1].vx =-width.vx;
	calc[1].vy = calc_vec->vy - shift.vy                   + SHIFT_MODEL_Y;
	calc[1].vz = width.vz     - shift.vz      - calc_vec->vw;

	calc[2].vx = width.vx;
	calc[2].vy = calc_vec->vy - shift.vy                   + SHIFT_MODEL_Y;
	calc[2].vz =-width.vz     + shift.vz      - calc_vec->vw;

	calc[3].vx =-width.vx     + shift.vx;
	calc[3].vy = calc_vec->vy - shift.vy                   + SHIFT_MODEL_Y;
	calc[3].vz =-width.vz                     - calc_vec->vw;


	calc[4].vx = width.vx     + shift_half.vx;
	calc[4].vy = 0.0f                                       + SHIFT_MODEL_Y;
	calc[4].vz = width.vz     - shift.vz      - calc_vec->vw;

	calc[5].vx =-width.vx     + shift.vx;
	calc[5].vy = 0.0f                                       + SHIFT_MODEL_Y;
	calc[5].vz = width.vz     + shift_half.vz - calc_vec->vw;

	calc[6].vx = width.vx     - shift.vx;
	calc[6].vy = 0.0f                                       + SHIFT_MODEL_Y;
	calc[6].vz =-width.vz     - shift_half.vz - calc_vec->vw;

	calc[7].vx =-width.vx     - shift_half.vx;
	calc[7].vy = 0.0f                                       + SHIFT_MODEL_Y;
	calc[7].vz =-width.vz     + shift.vz      - calc_vec->vw;


	calc[8].vx = 0.0f;
	calc[8].vy = calc_vec->vy  + SHIFT_MODEL_Y;
	calc[8].vz = -width.vz     + shift.vz - calc_vec->vw;

	DG_SetPos( world );
	DG_PutVector( calc, calc, MAX_WET_POINT );

	for( i=0; i<MAX_WET_POINT-1; i++ ){
//		AN_Test_Eye2( &calc[i], 3 );
		CB_WetEffectAdd( cb_wet, 0, &calc[i], range ) ;
	}
//	AN_Test_Eye2( &calc[MAX_WET_POINT-1], 3 );
	CB_WetEffectAdd( cb_wet, 0, &calc[MAX_WET_POINT-1], MIN_WET_RANGE ) ;

}

/*------------------------------------------------------------------*/
static void MakeHole( Work *work )
{
	DG_OBJ	*obj_base;
	DG_OBJ	*obj;
	int		n_models;
	int		num_init;
	int		num;
	int		flag;
	int		bit_flag;

	flag = 0;

	n_models = work->equip_sub.objs->n_models;
	num_init = num = ( (irnd()>>8) % (n_models-1) ) + 1;

	obj_base = work->equip_sub.objs->objs;
	obj = obj_base + num_init;

	while( obj->flag & DG_FLAG_INVISIBLE ){
		num++;
		if( num>=n_models ) num=1;	//０番は本体
//printf("%d %d\n",num,num_init);
		if( num==num_init ){
			flag = 1;
			break;
		}
		obj = obj_base + num;
	}

	if( !flag ){
		GM_SeSetMode( SD_A_RICDAN01, (FVECTOR *)&obj->world.m[3], GM_SEMODE_NORMAL );	/* 消火器くねくね噴出 */

		GV_SetActorChild( work,
			NewCB_HoleLight( &obj->world )
		);
		obj->flag |= DG_FLAG_INVISIBLE;

		bit_flag = 1;
		OK_HoleFlag |= (bit_flag<<num);
	}
}

/*------------------------------------------------------------------*/
static	void	BodyAdjust( objs, v )
DG_OBJS			*objs ;
FVECTOR			*v ;
{
	int		i ;
	FVECTOR	v2 ;
#if 0
	if ( GM_PlayerTarget != NULL ) {
		_sceVu0AddVector( &GM_PlayerTarget->offset, &GM_PlayerTarget->offset, 
						  v ) ;
		GM_PlayerTarget->offset.vw = 1.0F ;
//		NewTargetView2( GM_PlayerTarget, 32, 232, 32 ) ;
	}
#endif
	DG_RotVector( v, &v2, 1 ) ;
	objs->world.m[ 3 ][ 0 ] += v2.vx ;
	objs->world.m[ 3 ][ 1 ] += v2.vy ;
	objs->world.m[ 3 ][ 2 ] += v2.vz ;
	for ( i = 0; i < 21; i ++ ) {
		objs->objs[ i ].world.m[ 3 ][ 0 ] += v2.vx ;
		objs->objs[ i ].world.m[ 3 ][ 1 ] += v2.vy ;
		objs->objs[ i ].world.m[ 3 ][ 2 ] += v2.vz ;
	}
}

/*------------------------------------------------------------------*/
static void FallParts( Work *work, int num, int name, FVECTOR *shift )
{
	FMATRIX	mat;

	DG_SetPos( &work->equip.objs->objs[num].world );
	DG_MovePos( shift );
	DG_GetPos( &mat );

	NewBoxPartsFall( &mat, &work->rots[num], name, work->equip.map_name, OK_WetLevel, MAX_WET_LEVEL );
}

/*------------------------------------------------------------------*/
static void ModelTrick( Work *work )
{
	int		i;
	int		flag ;
	FVECTOR		mov;
	FVECTOR		fvtemp;
	float		angle;
	float		angle2;
	float		angle3;
	float		height0;
	float		height1;
	DG_OBJ		*obj;

	work->body_damage_shift *= 0.95f;
	work->body_damage_rot   *= 0.95f;
	if( work->before_damage > GM_ItemNum( BoxItemNo ) ){ // ダメージ食らった
		work->before_damage = GM_ItemNum( BoxItemNo ) ;
		work->bullet_efect_flag = 1;

		work->body_damage_shift = DAMAGE_SHIFT_Y*rnd() + DAMAGE_SHIFT_Y*0.5f;
		work->body_damage_rot   = DAMAGE_ROTATE*frnd();
		MakeHole( work );	//穴空ける
	}

	work->now_damage = work->before_damage;

	DG_COPY_VEC( &mov, &work->ctrl->mov );

	//床との接触をみる
	height0 = mov.vy - work->ctrl->levels[0]  - 200.0f;
	angle  = work->before_mov.vy - mov.vy;
	angle3 = atan2f( HUTA_PARAM, angle*1.5f ) - PI*0.7f;
	angle  = atan2f( HUTA_PARAM, angle ) - PI*0.6f;
	angle2 = angle * 0.25f;
	if( height0 > HUTA_WIDTH ){
		angle  += TPI*0.22f;
		angle3 += TPI*0.22f;
	}
	if( angle2 > 0.0f ) angle2 = 0.0f;
//printf("%f:%f:%f:\n",angle,angle2,angle3);
	angle = work->before_angle*0.85f + angle*0.15f;
	work->before_angle = angle;

	//足上げに引っかからないようにする
	height0 = (*work->body)->objs->objs[HUMAN21_MIGI_KAKATO].world.m[3][1];
	height1 = (*work->body)->objs->objs[HUMAN21_HIDARI_KAKATO].world.m[3][1];

	height0 = (height0 > height1)? height0+KAKATO_SIZE: height1+KAKATO_SIZE;

	height0 -= work->equip.objs->objs[16].world.m[3][1];
	height0 = (height0 > HUTA_WIDTH)? HUTA_WIDTH: height0;
	height0 = (height0 <-HUTA_WIDTH)?-HUTA_WIDTH: height0;

	height0 = acosf( height0 / HUTA_WIDTH ) - PI*0.5f;
	height0 = (height0 > angle)? angle: height0;


	CheckFutaAtari( work );

	// 上のフタ
	work->rots[ 6].vz = angle2;
	work->rots[ 8].vz =-angle2;

	if( GM_CheckPlayerStatus( PLAYER_CB_BOX_STAND ) ){
		// フタ
		work->rots[13].vz =-angle;
		work->rots[14].vz = angle;
		work->rots[15].vx = angle;
//		work->rots[16].vx =-angle;
		work->rots[16].vx =-height0;
	}else{
//printf("%f:%f %f %f %f\n",angle,FutaRots[0],FutaRots[1],FutaRots[2],FutaRots[3]);
		if( angle < FutaRots[0] ){
			FutaDest[0] = FutaRots[0] = angle;
		}else{
			FutaDest[0] = FutaDest[0]*FUTA_ANGLE_RATIO + FutaRots[0]*(1.0f - FUTA_ANGLE_RATIO);
		}
		work->rots[13].vz =-FutaDest[0];

		if( angle < FutaRots[1] ){
			FutaDest[1] = FutaRots[1] = angle;
		}else{
			FutaDest[1] = FutaDest[1]*FUTA_ANGLE_RATIO + FutaRots[1]*(1.0f - FUTA_ANGLE_RATIO);
		}
		work->rots[14].vz = FutaDest[1];

		if( angle < FutaRots[2] ){
			FutaDest[2] = FutaRots[2] = angle;
		}else{
			FutaDest[2] = FutaDest[2]*FUTA_ANGLE_RATIO + FutaRots[2]*(1.0f - FUTA_ANGLE_RATIO);
		}
		work->rots[15].vx = FutaDest[2];

		if( height0 < FutaRots[3] ){
			FutaDest[3] = FutaRots[3] = height0;
		}else{
			FutaDest[3] = FutaDest[3]*FUTA_ANGLE_RATIO + FutaRots[3]*(1.0f - FUTA_ANGLE_RATIO);
		}
		work->rots[16].vx =-FutaDest[3];

	}




	// 小さく残ったフタ
	work->rots[17].vz =-angle3;
	work->rots[18].vz =-angle3;

	switch( work->break_mode ){
		case BOX_STATE_INITIAL:
			work->invisible_flags = BOX_DEFAULT;
			if( work->now_damage <= DAMAGE_LIMIT_1){
				work->break_mode = BOX_STATE_BREAK_1;
				fvtemp.vx = 220.0f;
				fvtemp.vy = 0.0f;
				fvtemp.vz = 0.0f;
				FallParts( work, 13, work->futaname[ 0 ], &fvtemp );
			}
			break;
		case BOX_STATE_BREAK_1:
			work->invisible_flags = BOX_BREAK1;
			if( work->now_damage <= DAMAGE_LIMIT_2){
				work->break_mode = BOX_STATE_BREAK_2;
				FallParts( work, 14, work->futaname[ 1 ], &DG_ZeroVector );
			}
			break;
		case BOX_STATE_BREAK_2:
			work->invisible_flags = BOX_BREAK2;
			if( work->now_damage <= DAMAGE_LIMIT_3){
				work->break_mode = BOX_STATE_BREAK_3;
				FallParts( work, 15, work->futaname[ 2 ], &DG_ZeroVector );
			}
			break;
		case BOX_STATE_BREAK_3:
			work->invisible_flags = BOX_BREAK3;
			if( work->now_damage <= DAMAGE_LIMIT_4){
				work->break_mode = BOX_STATE_BREAK_4;
				FallParts( work, 16, work->futaname[ 3 ], &DG_ZeroVector );
			}
			break;
		case BOX_STATE_BREAK_4:
			work->invisible_flags = BOX_BREAK4;
			if( work->now_damage <= DAMAGE_LIMIT_5){
				work->break_mode = BOX_STATE_BREAK_5;
				work->fall_body = NewBreakBoxBody( work->model_name, &work->equip.objs->world, work->rots, MAX_MODEL, BOX_BREAK5, work->equip.map_name );
				work->invisible_flags = BOX_BREAK6;
				GM_SeSetMode( SD_A_DAMBRA01, (FVECTOR *)&work->equip.objs->world.m[3], GM_SEMODE_BOMB ) ;
			}
			break;
		case BOX_STATE_BREAK_5:
			work->invisible_flags = BOX_BREAK6;
			break;
		default:
			work->invisible_flags = BOX_DEFAULT;
			break;
	}

	//同一モデル内での関節のＯＮ・ＯＦＦ
	flag = work->invisible_flags;
	obj = work->equip.objs->objs;
	for( i=0; i<MAX_MODEL; i++ ){
		if( !(flag & 1) ){
			obj->flag |= DG_FLAG_INVISIBLE;
		}else{
			obj->flag &=~DG_FLAG_INVISIBLE;
		}
		flag >>= 1;
		obj++;
	}
}

static	void	CopyInvisibleFlag( DG_OBJS *objs )
{
	int			i, vflag ;
	DG_OBJ		*obj ;

	vflag = objs->flag & ( DG_FLAG_INVISIBLE0 | DG_FLAG_INVISIBLE1 ) ;
	if ( !vflag ) return ;
	obj = objs->objs ;
	for ( i = 0; i < objs->n_models; i ++ ) {
		obj->flag |= vflag ;
		obj ++ ;
	}
}

/*------------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		trg ;
	FVECTOR		shift, mov ;
	SVECTOR	rot ;

	if( GM_PlayerBody == NULL ) return;
	if( GM_PlayerBody->objs == NULL ) return;
	if( &work->equip == NULL ) return;
	if( work->equip.objs == NULL ) return;

	/* 武器非表示 */
	if ( GM_PlayerWeaponBody != NULL ) {
		DG_InvisibleObjs( GM_PlayerWeaponBody->objs ) ;
	} 
	if ( GM_PlayerSubWeaponBody != NULL ) {
		DG_InvisibleObjs( GM_PlayerSubWeaponBody->objs ) ;
	} 

	/* マップをオリジナルに合わせる */
	work->equip.objs->group_id = GM_PlayerBody->objs->group_id ;
	work->equip_sub.objs->group_id = GM_PlayerBody->objs->group_id ;

	/* 装備前ＯＲアイテム化後 */
	if ( !( GM_CheckPlayerStatus( PLAYER_CB_BOX ) ) || BoxFlag == 2 ) {
		DG_InvisibleObjsChanl( work->equip.objs, work->chanl ) ;
		DG_InvisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;
		CopyInvisibleFlag( work->equip.objs ) ;
		return ;
	}

	ModelTrick( work );

	trg = *( work->trigger ) ;
	rot = work->ctrl->rot ; 
	
	DG_COPY_VEC( &mov, &work->ctrl->mov );

	rot.vx = work->rot_vx = GV_NearExp2P( work->rot_vx, trg ) ;
	mov.vy = GM_PlayerBody->objs->world.m[ 3 ][ 1 ] ;
	DG_SetPos2( &mov, &rot ) ;

	if ( GM_CheckPlayerStatus( PLAYER_CB_BOX_STAND ) ) {
		FVECTOR		shiftL = { 0.0F, 160.0F, 0.0F } ;
		work->equip.objs->root = NULL ;
		work->equip_sub.objs->root = NULL ;
		work->equip.map_name = GM_PlayerBody->map_name ;
		work->equip_sub.map_name = GM_PlayerBody->map_name ;
		GM_GroupObjs( work->equip.objs, work->equip.map_name ) ;
		GM_GroupObjs( work->equip_sub.objs, work->equip_sub.map_name ) ;
		shiftL.vz = work->shift_z ;
		DG_SetPos2( &work->ctrl->mov, &work->ctrl->rot ) ;
		DG_MovePos( &shiftL ) ;
		DG_PutObjs( work->equip.objs ) ;
		DG_PutObjs( work->equip_sub.objs ) ;
		work->before_angle = TPI*0.22f;

	} else {
		GM_ConfigObjectRoot( &work->equip, *work->body, *work->unit ) ;
		GM_ConfigObjectRoot( &work->equip_sub, *work->body, *work->unit ) ;
		DG_COPY_VEC( &shift, &DG_ZeroVector ) ;
		shift.vx = 0.0f;
		shift.vy = work->body_damage_shift * (work->body_damage_radian+1.0f)*rnd();
		shift.vz = work->shift_z ;
		BodyAdjust( GM_PlayerBody->objs, &shift ) ;
		/* ターゲットをダンボールの形にする */
		{
			TARGET		*t ;
			FVECTOR		size ;
			DG_MDL		*mdl ;
			
			t = GM_PlayerTarget ;
			if ( t != NULL ) {
				mdl = &work->equip.objs->def->models[ 1 ] ;
				size.vx = ( mdl->ux - mdl->lx ) / 2.0F ;
				size.vy = ( mdl->uy - mdl->ly ) / 2.0F ;
				size.vz = ( mdl->uz - mdl->lz ) / 2.0F ;
				DG_COPY_VEC( &t->offset, &DG_ZeroVector ) ;
				t->offset.vy = mdl->uy - size.vy ;
				GM_SetTargetSize( t, &size ) ;
//				NewTargetView2( t, 32, 232, 32 ) ;
			}
		}
	}

	DG_COPY_VEC( &work->before_mov, &mov ) ;
	work->before_mov.vy = GM_PlayerBody->objs->world.m[ 3 ][ 1 ];

    if ( BoxFlag == 1 ) {
		/* 持ち上げられ中 */
		DG_InvisibleObjsChanl( work->equip.objs, work->chanl ) ;		
		DG_InvisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;		
		CopyInvisibleFlag( work->equip.objs ) ;
		return ;
	}

	if ( ( *work->body )->objs->flag & ( DG_FLAG_INVISIBLE0 << work->chanl ) &&
		 GM_PlayerSubjectCamera[ 0 ]->on == 1 ) {
		DG_InvisibleObjsChanl( work->equip.objs, work->chanl ) ;
		DG_VisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;
		if ( !GM_CheckPlayerStatus( PLAYER_CB_BOX_STAND ) ) {		
			shift.vz += 200.0F ; /* 怪しい微調整 */
			BodyAdjust( GM_PlayerBody->objs, &shift ) ;
		}
	} else {
		DG_VisibleObjsChanl( work->equip.objs, work->chanl ) ;
		DG_InvisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;
	}
	if ( ++ work->time < 8 ) {
		DG_InvisibleObjsChanl( work->equip.objs, work->chanl ) ;
		DG_InvisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;
	}
	InvisibleFlags = work->invisible_flags ;

	work->timer++;
	if( work->timer%WET_STEP_TIME==0 ){
		if( OK_BodySplashFlag  &&  (OK_cb_wet != NULL) ){
			if( OK_WetLevel < MAX_WET_LEVEL ){
				OK_WetLevel++;
				PutWet( &work->equip.objs->world, &work->size, OK_cb_wet, OK_WetLevel );
			}
		}
	}

#if 0 /* うーん、きっと要らなかったんでしょう。なんでコメントアウト */
	if( work->now_damage <= 0 ){
//		GV_CallChildSignalFunc( work, GV_SIGNAL_KILL, 0 );
	}


	{
		TARGET *ptrg;
		ptrg = (TARGET *)(GM_PlayerTarget->power->reserved);
		if( ptrg!=NULL ){
			if( (ptrg->weapon_type & WP_BULLET)  &&  work->bullet_efect_flag ){
				work->bullet_efect_flag = 0;
				_sceVu0SubVector( &shift, &ptrg->hit, &ptrg->center ) ;
//				NewBoxBreakDust( &ptrg->hit, &shift, GM_PlayerBody->map_name );
			}
		}
	}
#endif

	VisibleBandana( BoxItemNo ) ;
	/* 本体の非表示フラグを関節にコピー */
#if 0
	/* 主観時はやらない */
	if ( GM_PlayerBody != NULL && 
		( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) &&
		GM_CheckPlayerStatus( PLAYER_WATCH ) ) return ;
#endif
	CopyInvisibleFlag( work->equip.objs ) ;
}

static	void	Die( Work *work )
{
	if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( ( *work->body )->objs, work->equip.objs ) ;
	}
	GM_FreeObject( &( work->equip ) ) ;
	GM_FreeObject( &( work->equip_sub ) ) ;

	//濡れエフェクト
	if( OK_cb_wet != NULL  &&  work->fall_body == NULL ){
		if( !(GV_IsStageDestroy( work )) ){
			GV_DestroyActorQuick( OK_cb_wet ) ;
		}
	}

	/* 武器非表示 */
	if ( GM_PlayerWeaponBody != NULL ) {
		DG_VisibleObjs( GM_PlayerWeaponBody->objs ) ;
	} 
	if ( GM_PlayerSubWeaponBody != NULL ) {
		DG_VisibleObjs( GM_PlayerSubWeaponBody->objs ) ;
	} 
	VisibleBandana( -1 ) ;
    GM_PlayerBoxBody = NULL ;

	BoxBody = NULL ;
	OK_HoleFlag = 0;

    if ( work->tex_replace != NULL ) {
		DG_ResetMoveReplaceTexture( work->tex_replace ) ;
		DG_FreeMoveReplacePacket( work->tex_replace ) ;
	}
}

static	void	InitRots( void )
{
	FutaDest[0] = FutaRots[0] = -ANGLE_LIMIT;
	FutaDest[1] = FutaRots[1] = -ANGLE_LIMIT;
	FutaDest[2] = FutaRots[2] = -ANGLE_LIMIT;
	FutaDest[3] = FutaRots[3] = -ANGLE_LIMIT;
	FutaStep[0] =  ANGLE_LIMIT;
	FutaStep[1] =  ANGLE_LIMIT;
	FutaStep[2] =  ANGLE_LIMIT;
	FutaStep[3] =  ANGLE_LIMIT;
}

/*------------------------------------------------------------------*/
static	int	GetResources( Work *work, CONTROL *ctrl, OBJECT **body, int *unit, int texname )
{
	OBJECT	*equip ;
	FVECTOR	fvtemp;
	int		model ;

	InitRots();

#if 0
	if ( texname == GV_StrCode( "cbx_a" ) ) {
		work->tex_replace = NULL ;
	} else {
		work->tex_replace 
			= DG_MakeMoveReplacePacket( GV_StrCode( "c_box" ), 
									    GV_StrCode( "cbx_a" ), texname ) ;
		DG_SetMoveReplaceTexture( work->tex_replace ) ;
	}
#else

	work->tex_replace = NULL ;

#endif

	/* 美しくないがいまのところしょうがない */
	model = GV_StrCode( "cbx_a_mt" ) ;
	if ( texname == GV_StrCode( "cbx_a" ) ) {
		work->futaname[ 0 ] = GV_StrCode( "cbx_a_futa1_mt" ) ;
		work->futaname[ 1 ] = GV_StrCode( "cbx_a_futa2_mt" ) ;
		work->futaname[ 2 ] = GV_StrCode( "cbx_a_futa3_mt" ) ;
		work->futaname[ 3 ] = GV_StrCode( "cbx_a_futa4_mt" ) ;
		model = GV_StrCode( "cbx_a_mt" ) ;
	} else if ( texname == GV_StrCode( "cbx_b" ) ) {
		work->futaname[ 0 ] = GV_StrCode( "cbx_b_futa1_mt" ) ;
		work->futaname[ 1 ] = GV_StrCode( "cbx_b_futa2_mt" ) ;
		work->futaname[ 2 ] = GV_StrCode( "cbx_b_futa3_mt" ) ;
		work->futaname[ 3 ] = GV_StrCode( "cbx_b_futa4_mt" ) ;
		model = GV_StrCode( "cbx_b_mt" ) ;
	} else if ( texname == GV_StrCode( "cbx_c" ) ) {
		work->futaname[ 0 ] = GV_StrCode( "cbx_c_futa1_mt" ) ;
		work->futaname[ 1 ] = GV_StrCode( "cbx_c_futa2_mt" ) ;
		work->futaname[ 2 ] = GV_StrCode( "cbx_c_futa3_mt" ) ;
		work->futaname[ 3 ] = GV_StrCode( "cbx_c_futa4_mt" ) ;
		model = GV_StrCode( "cbx_c_mt" ) ;
	} else if ( texname == GV_StrCode( "cbx_d" ) ) {
		work->futaname[ 0 ] = GV_StrCode( "cbx_d_futa1_mt" ) ;
		work->futaname[ 1 ] = GV_StrCode( "cbx_d_futa2_mt" ) ;
		work->futaname[ 2 ] = GV_StrCode( "cbx_d_futa3_mt" ) ;
		work->futaname[ 3 ] = GV_StrCode( "cbx_d_futa4_mt" ) ;
		model = GV_StrCode( "cbx_d_mt" ) ;
	} else if ( texname == GV_StrCode( "cbx_e" ) ) {
		work->futaname[ 0 ] = GV_StrCode( "cbx_e_futa1_mt" ) ;
		work->futaname[ 1 ] = GV_StrCode( "cbx_e_futa2_mt" ) ;
		work->futaname[ 2 ] = GV_StrCode( "cbx_e_futa3_mt" ) ;
		work->futaname[ 3 ] = GV_StrCode( "cbx_e_futa4_mt" ) ;
		model = GV_StrCode( "cbx_e_mt" ) ;
	}
	work->model_name = model ;

	work->bullet_efect_flag = 0;

	OK_WetLevel = 0;

	work->timer = 0;

//	temp_damage = 25 ;
//	GM_SetItemNum( BoxItemNo, temp_damage ) ;

	work->before_damage = work->now_damage = GM_ItemNum( BoxItemNo ) ;
	work->body_damage_radian = 1.0f;

	if( work->before_damage <= DAMAGE_LIMIT_5 ){
		work->break_mode = BOX_STATE_BREAK_5;
	}else if( work->before_damage <= DAMAGE_LIMIT_4 ){
		work->break_mode = BOX_STATE_BREAK_4;
	}else if( work->before_damage <= DAMAGE_LIMIT_3 ){
		work->break_mode = BOX_STATE_BREAK_3;
	}else if( work->before_damage <= DAMAGE_LIMIT_2 ){
		work->break_mode = BOX_STATE_BREAK_2;
	}else if( work->before_damage <= DAMAGE_LIMIT_1 ){
		work->break_mode = BOX_STATE_BREAK_1;
	}else{
		work->break_mode = BOX_STATE_INITIAL;
	}

	work->body_damage_shift = 0.0f;
	work->body_damage_rot   = 0.0f;

	equip = BoxBody = &( work->equip ) ;
	BoxModel = model ;
	GM_InitObject( equip, model, BODY_FLAG ) ;
	if ( equip->objs == NULL ) return -1 ;
	/* 親のいるSHADOWMAKEオブジェは、shadow_idをいじる */
	equip->objs->shadow_id = 1 ;
	
	GM_ConfigObjectJoint( equip, work->rots );
	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	DG_COPY_VEC( &work->before_mov, &ctrl->mov ) ;
	GM_ConfigObjectRoot( equip, *body, *unit ) ;
	DG_ConnectObjs( ( *body )->objs, equip->objs ) ;

//	flag = INVISIBLE_FLAG ;
//	for ( i = 0; i < ( *body )->objs->def->n_x_models; i ++ ) {
//		if ( flag & 1 ) {
//			( *body )->objs->objs[ i ].flag |= DG_FLAG_INVISIBLE ;
//		}
//		flag >>= 1 ;
//	}
	work->rot_vx = 0 ;
	work->shift_z 
		= -( equip->objs->def->models[ 1 ].uz 
			- ( equip->objs->def->models[ 1 ].uz - equip->objs->def->models[ 1 ].lz ) / 2.0F ) ;

	/* 主観モデル */
	equip = &work->equip_sub ;
	GM_InitObject( equip, BODY_NAME_SUB, BODY_FLAG_SUB ) ;
	if ( equip->objs == NULL ) return -1 ;
	GM_ConfigObjectJoint( equip, work->rots ) ;
	GM_ConfigObjectRoot( equip, *body, *unit ) ;	
	DG_InvisibleObjs( equip->objs ) ;

	OK_cb_wet = NULL;
	work->fall_body = NULL;
	/* 段ボール水飛沫 */
	if( OK_BodySplashFlag ){
		fvtemp.vx = work->equip.objs->def->models[1].ux - work->equip.objs->def->models[1].lx;
		fvtemp.vy = work->equip.objs->def->models[1].uy - work->equip.objs->def->models[1].ly;
		fvtemp.vz = work->equip.objs->def->models[1].uz - work->equip.objs->def->models[1].lz;

		GV_SetActorChild( work,
		    NewCBoxSplash( &work->equip.objs->world, fvtemp.vx, fvtemp.vz, fvtemp.vy, work->shift_z, &ctrl->level_found, 0 )
		);

		work->size.vx = fvtemp.vx;
		work->size.vy = fvtemp.vy;
		work->size.vz = fvtemp.vz;
		work->size.vw = work->shift_z;
		OK_cb_wet = NewCB_WetEffect( work->equip.objs, BODY_NAME ) ;
	}


	{
		DG_OBJ		*obj;
		int			i, flags;

		flags = OK_HoleFlag;
		obj = work->equip_sub.objs->objs;
		obj++;
		for( i=1; i<work->equip_sub.objs->n_models; i++ ){
			flags >>= 1;
			if( flags&1 ){
				GV_SetActorChild( work,
					NewCB_HoleLight( &obj->world )
				);
				obj->flag |= DG_FLAG_INVISIBLE;
			}
			obj++;
		}
	}

	GM_PlayerBoxBody = &work->equip ;

	VisibleBandana( BoxItemNo ) ;


	return 0 ;
}

/*------------------------------------------------------------------*/

void *NewC_Box(
	CONTROL *ctrl,
	OBJECT  **body,
	int     *unit,
	int     *trigger,
	int		texname
){
	Work	*work ;

	BoxItemNo = GM_Item ;
	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, ctrl, body, unit, texname ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->chanl = GM_CurrentCameraChanl ;
	}
	return work ;
}

void	*NewC_BoxA( CONTROL *ctrl, OBJECT **body, int *unit, int *trigger )
{
	return NewC_Box( ctrl, body, unit, trigger, GV_StrCode( "cbx_a" ) ) ;
}

void	*NewC_BoxB( CONTROL *ctrl, OBJECT **body, int *unit, int *trigger )
{
	return NewC_Box( ctrl, body, unit, trigger, GV_StrCode( "cbx_b" ) ) ;
}

void	*NewC_BoxC( CONTROL *ctrl, OBJECT **body, int *unit, int *trigger )
{
	return NewC_Box( ctrl, body, unit, trigger, GV_StrCode( "cbx_c" ) ) ;
}

void	*NewC_BoxD( CONTROL *ctrl, OBJECT **body, int *unit, int *trigger )
{
	return NewC_Box( ctrl, body, unit, trigger, GV_StrCode( "cbx_d" ) ) ;
}

void	*NewC_BoxE( CONTROL *ctrl, OBJECT **body, int *unit, int *trigger )
{
	return NewC_Box( ctrl, body, unit, trigger, GV_StrCode( "cbx_e" ) ) ;
}

/*------------------------------------------------------------------*/
typedef	struct {
	GV_ACT_EX		actor ;
	OBJECT		equip ;

	FMATRIX		world ;
	FVECTOR		shift ;
	FVECTOR		rots[MAX_MODEL];
	FVECTOR		before_mov;
	float		before_angle;

	int			id ;
	int			vital ;

	CONTROL		*ctrl ;
	OBJECT		*body ;
	int			unit ;
	int			*flag ;

	int			rot_vx ;

	void		*cb_wet ;		/* 濡れエフェクト */
	FVECTOR		size;
} Work2 ;

static	void	SetInvisibleFlag( DG_OBJ *obj, int flag )
{
	int			i ;

	for ( i = 0; i < MAX_MODEL; i ++ ) {
		if ( !( flag & 1 ) ) {
			obj->flag |= DG_FLAG_INVISIBLE ;
		} else {
			obj->flag &= ~DG_FLAG_INVISIBLE ;
		}
		flag >>= 1 ;
		obj ++ ;
	}
}

static	void	ChangeToItemBox( Work2 *work )
{
	FVECTOR	fvtemp ;
	FVECTOR	force ;
	extern void *NewItemBox_Prog( int name, int where, int splash, int model_name,
								  int shadow_name, int label_name, FVECTOR *pos,
								  float height, int spec, int id, int num, int pattern,
								  FVECTOR *force ) ;

	if ( work->vital <= 0 ) return ;

	DG_SetPos2( &work->ctrl->mov, &work->ctrl->rot ) ;
	GV_SetVec3( &fvtemp, 0.0F, 0.0F, 500.0F ) ;
	DG_PutVector( &fvtemp, &fvtemp, 1 ) ;	

	GV_SetVec3( &force, 0.0F, 500.0F, 1000.0F ) ;
	DG_RotVector( &force, &force, 1 ) ;
	NewItemBox_Prog( 0,	GM_CurrentStageMap,	0,	GV_StrCode( "box2_ibox" ),
					 GV_StrCode( "box2_ibox_sh" ), GV_StrCode( "cbx_label" ),
					 &fvtemp, 300.0f, GM_ITEM_BOX, work->id, work->vital, 2, &force	) ;
	if ( GM_CheckPlayerStatus( PLAYER_CB_BOX ) ) GM_Item = IT_None ;
	GM_SetItemNum( work->id, 0 ) ;
}

static	void	Act2(
	Work2	*work
){
	FVECTOR		mov, pos, root ;
	float		angle;
	float		angle2;
	float		angle3;

	GM_SetPlayerStatus( PLAYER_WEAPON_INVISIBLE | PLAYER_CB_BOX_CANCELED ) ;

	if ( *work->flag != 0 ) {
		/* アイテム化 */
		ChangeToItemBox( work ) ;
		BoxFlag = 2 ;
		GV_DestroyActor( work ) ;
		return ;
	}

	
	GM_SetCurrentMap( work->body->map_name ) ;
	GV_MatToVec( &work->body->objs->objs[ work->unit ].world, &root ) ;
	DG_SetPos2( &root, &work->ctrl->rot ) ;
	DG_PutVector( &work->shift, &pos, 1 ) ;
	GV_VecToMat( &pos, &work->world ) ;
	DG_SetPos( &work->world ) ;

	GM_ActObject( &work->equip ) ;	

	GV_MatToVec( &work->equip.objs->world, &mov ) ;




	angle  = work->before_mov.vy - mov.vy;
	angle3 = atan2f( HUTA_PARAM, angle*1.5f ) - PI*0.7f;
	angle  = atan2f( HUTA_PARAM, angle ) - PI*0.6f;
	angle2 = angle * 0.25f;
	if( angle2 > 0.0f ) angle2 = 0.0f;
	angle = work->before_angle*0.85f + angle*0.15f;
	work->before_angle = angle;


	// 上のフタ
//printf(" angle2::%f\n ",angle2);
	work->rots[ 6].vz = angle2;
	work->rots[ 8].vz =-angle2;

	// フタ
	work->rots[13].vz =-angle;
	work->rots[14].vz = angle;
	work->rots[15].vx = angle;
	work->rots[16].vx =-angle;

	// 小さく残ったフタ
	work->rots[17].vz =-angle3;
	work->rots[18].vz =-angle3;

	VisibleBandana( -1 ) ;
}

static	void	Die2(
	Work2 *work
){

//printf("CB_BOX::::::::::::::::::::::::::::::Die2\n");

	DG_DisconnectObjs( work->body->objs, work->equip.objs ) ;
	GM_FreeObject( &( work->equip ) ) ;

	GM_ResetPlayerStatus( PLAYER_CB_BOX_CANCELED ) ;
	BoxFlag = 0 ;

//	if( work->cb_wet != NULL ) GV_DestroyActorQuick( work->cb_wet ) ;
	VisibleBandana( -1 ) ;
}

static	int	GetResources2( Work2 *work, CONTROL *ctrl, OBJECT *body, int unit,
						   FVECTOR *shift )
{
	OBJECT	*equip ;	
	SVECTOR	rot ;
	FVECTOR	pos, root ;
	FMATRIX	world ;
	FVECTOR	fvtemp;

	InitRots();

	work->id = PL_GetPlayerItem() ;
	if ( GM_PlayerBody == NULL || BoxBody == NULL ||
		!( GM_ItemTypes[ work->id ] & IT_TYPE_CBBOX ) ) return -1 ;
	work->vital = GM_ItemNum( work->id ) ;

	equip = &( work->equip ) ;

	//GM_InitObject( equip, BODY_NAME, BODY_FLAG ) ;
	GM_InitObject( equip, BoxModel, BODY_FLAG ) ;

	if ( equip->objs == NULL ) return -1 ;
	/* 親のいるSHADOWMAKEオブジェは、shadow_idをいじる */
	equip->objs->shadow_id = 1 ;

	if( OK_cb_wet != NULL ) CB_WetEffectChangeObjs( OK_cb_wet, equip->objs );	// 水濡れコピー
	DG_ConnectObjs( body->objs, equip->objs ) ;
	GM_ConfigObjectJoint( equip, work->rots ) ;
	GM_ConfigObjectLight( equip, body->objs->light ) ;


	rot = ctrl->rot ; rot.vy += 2048 ;
	GV_MatToVec( &GM_PlayerBody->objs->world, &pos ) ;
	DG_SetPos2( &pos, &rot ) ;
	DG_GetPos( &world ) ;
	DG_COPY_MAT( &work->world, &world ) ;

	GV_MatToVec( &body->objs->objs[ unit ].world, &root ) ;
	DG_SetPos2( &root, &ctrl->rot ) ;
	DG_PutVector( shift, &pos, 1 ) ;
	GV_VecToMat( &pos, &world ) ;

	DG_SetPos( &world ) ;
	DG_PutObjs( equip->objs ) ;

	GV_MatToVec( &equip->objs->world, &work->before_mov ) ;
	work->rot_vx = 0 ;	

	BoxFlag = 1 ;
	if ( BoxBody != NULL ) DG_InvisibleObjs( BoxBody->objs ) ;

	SetInvisibleFlag( equip->objs->objs, InvisibleFlags ) ;

	/* 段ボール水飛沫 */
	if( OK_BodySplashFlag ){
		fvtemp.vx = equip->objs->def->models[1].ux - equip->objs->def->models[1].lx;
		fvtemp.vy = equip->objs->def->models[1].uy - equip->objs->def->models[1].ly;
		fvtemp.vz = equip->objs->def->models[1].uz - equip->objs->def->models[1].lz;
		fvtemp.vw = -350.0f;

//printf("%f %f %f\n",work->shift.vx,work->shift.vy,work->shift.vz);
		work->size.vx = fvtemp.vx;
		work->size.vy = fvtemp.vy;
		work->size.vz = fvtemp.vz;
		work->size.vw = fvtemp.vw;

		GV_SetActorChild( work,
		    NewCBoxSplash( &equip->objs->world, fvtemp.vx, fvtemp.vz, fvtemp.vy, fvtemp.vw, &ctrl->level_found, 1 )
		);
	}
	VisibleBandana( -1 ) ;
	return 0 ;
}

void *NewC_BoxLifted(
	CONTROL *ctrl,
	OBJECT  *body,
	int     unit,
	FVECTOR *shift,
	int     *flag
){
	Work2	*work ;

	/* プレイヤーより後でメニューより前 */
	/* アイテム変更不可にする関係で */
	work = ( Work2 * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work2 ), PLAYER_ITEM_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		DG_COPY_VEC( &work->shift, shift ) ;
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->flag = flag ;

		GV_SetActor( &( work->actor ), Act2, Die2 ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources2( work, ctrl, body, unit, shift ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	GM_SetPlayerStatus( PLAYER_CB_BOX_CANCELED ) ;
	return work ;
}

static FVECTOR OK_Cbox_rots[MAX_MODEL];

void *PutCBoxBreakBody( FMATRIX *world, int map ){
	int			body_name ;
	
	switch( PL_GetPlayerItem() ) {
	case IT_CBBox :
	case IT_CBBoxWet :
		body_name = GV_StrCode( "cbx_a_mt" ) ;
		break ;
	case IT_CBBoxB :
		body_name = GV_StrCode( "cbx_b_mt" ) ;
		break ;
	case IT_CBBoxC :
		body_name = GV_StrCode( "cbx_c_mt" ) ;
		break ;
	case IT_CBBoxD :
		body_name = GV_StrCode( "cbx_d_mt" ) ;
		break ;
	case IT_CBBoxE :
		body_name = GV_StrCode( "cbx_e_mt" ) ;
		break ;
	default :
		return NULL ;
	}
	return NewBreakBoxBody( body_name, world, OK_Cbox_rots, MAX_MODEL, BOX_BREAK5, map );
}

