//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   c_box.c
   汎用ダンボール(誰にでもかぶせられる)
   主観ダンボールは無し

   1999/12/10 M.Sonoyama
   2001/04/04 Copyed And Modified by K.Kano
   $Id: c_box.c,v 1.1.1.3 2002/11/19 11:43:20 Yoshizawa1 Exp $
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
#include <libutl.h>
#endif

#include	"gameheader.h"
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
extern void PutWet( FMATRIX *world, FVECTOR *calc_vec, void *cb_wet, int wet_level );

/*------------------------------------------------------------------*/

#define	BODY_FLAG		(DG_FLAG_SHADE)
#define	BODY_FLAG_SUB	(DG_FLAG_SHADE|DG_FLAG_NOFOG)

#define BOX_TYPE_MAX	5

#define	BODY_NAME		0x00253b8a		/* ( GV_StrCode( "cbx_a_mt" ) ) */
#define	FUTA_NAME1		0x0078b5cb		/* ( GV_StrCode( "cbx_a_futa1_mt" ) ) */
#define	FUTA_NAME2		0x007935cb		/* ( GV_StrCode( "cbx_a_futa2_mt" ) ) */
#define	FUTA_NAME3		0x0079b5cb		/* ( GV_StrCode( "cbx_a_futa3_mt" ) ) */
#define	FUTA_NAME4		0x007a35cb		/* ( GV_StrCode( "cbx_a_futa4_mt" ) ) */

#define	BODY_NAME1		0x0025bb8a		/* ( GV_StrCode( "cbx_b_mt" ) )	*/
#define	FUTA_NAME11		0x0098b5cb		/* ( GV_StrCode( "cbx_b_futa1_mt" ) ) */
#define	FUTA_NAME12		0x009935cb		/* ( GV_StrCode( "cbx_b_futa2_mt" ) ) */
#define	FUTA_NAME13		0x0099b5cb		/* ( GV_StrCode( "cbx_b_futa3_mt" ) ) */
#define	FUTA_NAME14		0x009a35cb		/* ( GV_StrCode( "cbx_b_futa4_mt" ) ) */

#define	BODY_NAME2		0x00263b8a		/* ( GV_StrCode( "cbx_c_mt" ) )	*/
#define	FUTA_NAME21		0x00b8b5cb		/* ( GV_StrCode( "cbx_c_futa1_mt" ) ) */
#define	FUTA_NAME22		0x00b935cb		/* ( GV_StrCode( "cbx_c_futa2_mt" ) ) */
#define	FUTA_NAME23		0x00b9b5cb		/* ( GV_StrCode( "cbx_c_futa3_mt" ) ) */
#define	FUTA_NAME24		0x00ba35cb		/* ( GV_StrCode( "cbx_c_futa4_mt" ) ) */

#define	BODY_NAME3		0x0026bb8a		/* ( GV_StrCode( "cbx_d_mt" ) )	*/
#define	FUTA_NAME31		0x00d8b5cb		/* ( GV_StrCode( "cbx_d_futa1_mt" ) ) */
#define	FUTA_NAME32		0x00d935cb		/* ( GV_StrCode( "cbx_d_futa2_mt" ) ) */
#define	FUTA_NAME33		0x00d9b5cb		/* ( GV_StrCode( "cbx_d_futa3_mt" ) ) */
#define	FUTA_NAME34		0x00da35cb		/* ( GV_StrCode( "cbx_d_futa4_mt" ) ) */

#define	BODY_NAME4		0x00273b8a		/* ( GV_StrCode( "cbx_e_mt" ) )	*/
#define	FUTA_NAME41		0x00f8b5cb		/* ( GV_StrCode( "cbx_e_futa1_mt" ) ) */
#define	FUTA_NAME42		0x00f935cb		/* ( GV_StrCode( "cbx_e_futa2_mt" ) ) */
#define	FUTA_NAME43		0x00f9b5cb		/* ( GV_StrCode( "cbx_e_futa3_mt" ) ) */
#define	FUTA_NAME44		0x00fa35cb		/* ( GV_StrCode( "cbx_e_futa4_mt" ) ) */

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

#define MAX_DAMAGE		(25)
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

enum {
	BOX_STATE_INITIAL=0,
	BOX_STATE_BREAK_1,
	BOX_STATE_BREAK_2,
	BOX_STATE_BREAK_3,
	BOX_STATE_BREAK_4,
	BOX_STATE_BREAK_5,
	BOX_STATE_FINISH
};

#if 0

static	OBJECT	*BoxBody = NULL ;
static	int		BoxFlag     = 0 ;
static	int		BoxItemNo = 0 ;	/* ２つ以上同時に装備される（対戦など）とマズイ */
static	int		InvisibleFlags = 0 ;

static	int		OK_HoleFlag = 0;
static	int		OK_WetLevel = 0;

void *OK_cb_wet ; /* 濡れエフェクトワークへのポインタ */

#endif

/*------------------------------------------------------------------*/
typedef	struct {
	GV_ACT_EX		actor ;

	int			now_damage;
	// int			before_damage;

	OBJECT		equip ;
	// OBJECT		equip_sub ;

	float		body_damage_shift;
	float		body_damage_rot;
	float		body_damage_radian;

	FVECTOR		rots[MAX_MODEL];
	FVECTOR		before_mov;
	float		before_angle;
	int			break_mode;
	int			invisible_flags;

	CONTROL		*ctrl;
	OBJECT		*body;
	DG_OBJS		*objs;

	HOMING_TRG	homing;
	FMATRIX homing_trg_mat;

	int			box_type;
	int			unit;
	// int			*trigger ;
	int			rot_vx ;
	int			chanl ;
	int			time ;
	float		shift_z ;

	FVECTOR		size;
	void		*fall_body;
	// int			timer;

	int			bullet_efect_flag;

	TARGET		target;
	void		(*Damaged)(void *param_work);
	void		*param_work;
} Work ;

/*------------------------------------------------------------------*/

static const int boxname[]={
	BODY_NAME,
	BODY_NAME1,
	BODY_NAME2,
	BODY_NAME3,
	BODY_NAME4,
};

static const int futaname1[]={
	FUTA_NAME1,
	FUTA_NAME11,
	FUTA_NAME21,
	FUTA_NAME31,
	FUTA_NAME41,
};

static const int futaname2[]={
	FUTA_NAME2,
	FUTA_NAME21,
	FUTA_NAME22,
	FUTA_NAME23,
	FUTA_NAME24,
};

static const int futaname3[]={
	FUTA_NAME3,
	FUTA_NAME31,
	FUTA_NAME32,
	FUTA_NAME33,
	FUTA_NAME34,
};

static const int futaname4[]={
	FUTA_NAME4,
	FUTA_NAME41,
	FUTA_NAME42,
	FUTA_NAME43,
	FUTA_NAME44,
};


/*------------------------------------------------------------------*/


/*------------------------------------------------------------------*/

#if 0

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

/*
	for( i=1; i<work->equip_sub.objs->n_models; i++ ){
		GV_SetActorChild( work,
			NewCB_HoleLight( &obj->world )
		);
		obj->flag |= DG_FLAG_INVISIBLE;
		obj++;
	}
*/
}

#endif


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

	// NewBoxPartsFall( &mat, &work->rots[num], name, work->equip.map_name, OK_WetLevel, MAX_WET_LEVEL );
	NewBoxPartsFall( &mat, &work->rots[num], name, work->equip.map_name, 0, MAX_WET_LEVEL );
}

/*------------------------------------------------------------------*/

static void CalcHomingMat(Work *work)
{
	FVECTOR ofs;
	DG_MDL *mdl;
			
	mdl=&(work->equip.objs->def->models[1]);

	ofs.vx=(mdl->ux+mdl->lx)/2;
	ofs.vy=(mdl->uy+mdl->ly)/2;
	ofs.vz=(mdl->uz+mdl->lz)/2;
	ofs.vw=1.0f;

	vu0_Ldv0(&ofs);
	vu0_Ldm0(&(work->objs->objs[work->unit].world));

	vu0_Mulv0m0v0();

	vu0_Stm0(&(work->homing_trg_mat));
	vu0_Stv0((FVECTOR *)&(work->homing_trg_mat.m[3][0]));
}

/*------------------------------------------------------------------*/


#define BLOW_DAMAGE		10
#define BULLET_DAMAGE	5

static void target_callback(TARGET *ofs,TARGET *def,void *param)
{
    Work *work=(Work *)param;

	if(work->break_mode==BOX_STATE_BREAK_5){
		def->damaged=0;
		return;
	}

	if(TARGET_POWER & def->damaged){
		long64 weapon = def->weapon_type ;

		if(weapon & WP_BLOW){
			work->now_damage-=BLOW_DAMAGE;
			work->bullet_efect_flag=1;
			work->body_damage_shift = DAMAGE_SHIFT_Y*rnd() + DAMAGE_SHIFT_Y*0.5f;
			work->body_damage_rot   = DAMAGE_ROTATE*frnd();

			(*work->Damaged)(work->param_work);
		}
		else{
			work->now_damage-=BULLET_DAMAGE;
			work->bullet_efect_flag=1;
			work->body_damage_shift = DAMAGE_SHIFT_Y*rnd() + DAMAGE_SHIFT_Y*0.5f;
			work->body_damage_rot   = DAMAGE_ROTATE*frnd();

			(*work->Damaged)(work->param_work);
		}
	}

    def->damaged=0;
}

static void MakeNewBox( Work *work,int box_type )
{
	OBJECT	*equip ;
	DG_OBJS *objs;
	CONTROL *ctrl;
	OBJECT *body;
	int unit;

	ctrl = work->ctrl;
	body = work->body;
	objs = work->objs;
	unit = work->unit;

	// printf("Check 1\n");

	equip = &( work->equip ) ;
	GM_InitObject( equip, boxname[box_type], BODY_FLAG ) ;

	// printf("Check 2\n");

	if ( equip->objs == NULL ) return ;

	work->box_type=box_type;

	GM_ConfigObjectJoint( equip, work->rots );
	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	DG_COPY_VEC( &work->before_mov, &ctrl->mov ) ;
	GM_ConfigObjectRoot( equip, body, unit ) ;
	DG_ConnectObjs( objs, equip->objs ) ;
}

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

#if 0
	if( work->before_damage > GM_ItemNum( BoxItemNo ) ){ // ダメージ食らった
		work->before_damage = GM_ItemNum( BoxItemNo ) ;
		work->bullet_efect_flag = 1;

		work->body_damage_shift = DAMAGE_SHIFT_Y*rnd() + DAMAGE_SHIFT_Y*0.5f;
		work->body_damage_rot   = DAMAGE_ROTATE*frnd();
		// MakeHole( work );	//穴空ける
	}
#endif

#if 0
	if ( GV_PadData[0].press & PAD_X ){
		temp_damage -= 5;
		if( temp_damage <= -5 ){
			temp_damage = 25;
//			work->break_mode = BOX_STATE_INITIAL;
			GM_Item = IT_None ;
		}
	}
//	GM_SetItemNum( BoxItemNo, temp_damage ) ;
#else
	// work->now_damage = work->before_damage;
#endif

	DG_COPY_VEC( &mov, &work->ctrl->mov );

	//床との接触をみる
	height0 = mov.vy - work->ctrl->levels[0]  - 200.0f;
	angle  = work->before_mov.vy - mov.vy;
//	angle3 = atan2f( HUTA_PARAM, angle*1.5f ) - PI*0.6f;
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
	height0 = work->objs->objs[HUMAN21_MIGI_KAKATO].world.m[3][1];
	height1 = work->objs->objs[HUMAN21_HIDARI_KAKATO].world.m[3][1];

	height0 = (height0 > height1)? height0+KAKATO_SIZE: height1+KAKATO_SIZE;

	height0 -= work->equip.objs->objs[16].world.m[3][1];
	height0 = (height0 > HUTA_WIDTH)? HUTA_WIDTH: height0;
	height0 = (height0 <-HUTA_WIDTH)?-HUTA_WIDTH: height0;

	height0 = acosf( height0 / HUTA_WIDTH ) - PI*0.5f;
	height0 = (height0 > angle)? angle: height0;


	// 上のフタ
	work->rots[ 6].vz = angle2;
	work->rots[ 8].vz =-angle2;

	// フタ
	work->rots[13].vz =-angle;
	work->rots[14].vz = angle;
	work->rots[15].vx = angle;
//	work->rots[16].vx =-angle;
	work->rots[16].vx =-height0;

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
			FallParts( work, 13, futaname1[work->box_type], &fvtemp );
		}
		break;
	case BOX_STATE_BREAK_1:
		work->invisible_flags = BOX_BREAK1;
		if( work->now_damage <= DAMAGE_LIMIT_2){
			work->break_mode = BOX_STATE_BREAK_2;
			FallParts( work, 14, futaname2[work->box_type], &DG_ZeroVector );
		}
		break;
	case BOX_STATE_BREAK_2:
		work->invisible_flags = BOX_BREAK2;
		if( work->now_damage <= DAMAGE_LIMIT_3){
			work->break_mode = BOX_STATE_BREAK_3;
			FallParts( work, 15, futaname3[work->box_type], &DG_ZeroVector );
		}
		break;
	case BOX_STATE_BREAK_3:
		work->invisible_flags = BOX_BREAK3;
		if( work->now_damage <= DAMAGE_LIMIT_4){
			work->break_mode = BOX_STATE_BREAK_4;
			FallParts( work, 16, futaname4[work->box_type], &DG_ZeroVector );
		}
		break;
	case BOX_STATE_BREAK_4:
		work->invisible_flags = BOX_BREAK4;
		if( work->now_damage <= DAMAGE_LIMIT_5){
			work->fall_body=NewBreakBoxBody(boxname[work->box_type],&work->equip.objs->world,
											work->rots,MAX_MODEL,BOX_BREAK5,
											work->equip.map_name);

			GM_SeSetMode(SD_A_DAMBRA01,(FVECTOR *)&work->equip.objs->world.m[3],GM_SEMODE_BOMB);

#if 0
			work->break_mode=BOX_STATE_BREAK_5;
			work->invisible_flags=BOX_BREAK6;
#else
			work->break_mode=BOX_STATE_INITIAL;
			work->invisible_flags=BOX_DEFAULT;
			work->now_damage=MAX_DAMAGE;

			GM_FreeObject(&(work->equip));
			work->box_type++;
			if(work->box_type>=BOX_TYPE_MAX) work->box_type-=BOX_TYPE_MAX;
			MakeNewBox(work,work->box_type);
#endif
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

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	int		trg ;
	FVECTOR		shift, mov ;
	SVECTOR	rot ;

#if 0
	if( GM_PlayerBody == NULL ) return;
	if( GM_PlayerBody->objs == NULL ) return;
	if( &work->equip == NULL ) return;
#endif

	if( work->equip.objs == NULL ) return;

#if 0
	/* 武器非表示 */
	if ( GM_PlayerWeaponBody != NULL ) {
		DG_InvisibleObjs( GM_PlayerWeaponBody->objs ) ;
	} 
	if ( GM_PlayerSubWeaponBody != NULL ) {
		DG_InvisibleObjs( GM_PlayerSubWeaponBody->objs ) ;
	} 
#endif

	/* マップをオリジナルに合わせる */
	// work->equip.objs->group_id = GM_PlayerBody->objs->group_id ;
	work->equip.objs->group_id = work->objs->group_id;
	// work->equip_sub.objs->group_id = GM_PlayerBody->objs->group_id ;

#if 0
	/* 装備前ＯＲアイテム化後 */
	if ( !( GM_CheckPlayerStatus( PLAYER_CB_BOX ) ) || BoxFlag == 2 ) {
		DG_InvisibleObjsChanl( work->equip.objs, work->chanl ) ;
		DG_InvisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;
		return ;
	}
#endif

	ModelTrick( work );

	// trg = *( work->trigger ) ;
	trg = 0;
	rot = work->ctrl->rot ; 
	
	DG_COPY_VEC( &mov, &work->ctrl->mov );

	rot.vx = work->rot_vx = GV_NearExp2P( work->rot_vx, trg ) ;
	// mov.vy = GM_PlayerBody->objs->world.m[ 3 ][ 1 ] ;
	mov.vy = work->objs->world.m[ 3 ][ 1 ] ;
	DG_SetPos2( &mov, &rot ) ;

#if 0
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

	}
	else
#endif

	{
		// GM_ConfigObjectRoot( &work->equip, *work->body, *work->unit ) ;
		// GM_ConfigObjectRoot( &work->equip_sub, *work->body, *work->unit ) ;
		DG_COPY_VEC( &shift, &DG_ZeroVector ) ;
		shift.vx = 0.0f;
		shift.vy = work->body_damage_shift * (work->body_damage_radian+1.0f)*rnd();
		shift.vz = work->shift_z ;
		// BodyAdjust( GM_PlayerBody->objs, &shift ) ;
		BodyAdjust( work->objs, &shift ) ;

#if 0
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
#endif

	}

	DG_COPY_VEC( &work->before_mov, &mov ) ;
	// work->before_mov.vy = GM_PlayerBody->objs->world.m[ 3 ][ 1 ];
	work->before_mov.vy = work->objs->world.m[ 3 ][ 1 ];

#if 0
    if ( BoxFlag == 1 ) {
		/* 持ち上げられ中 */
		DG_InvisibleObjsChanl( work->equip.objs, work->chanl ) ;		
		DG_InvisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;		
		return ;
	}
#endif

	if ( work->objs->flag & ( DG_FLAG_INVISIBLE0 << work->chanl ) ) {
		DG_InvisibleObjsChanl( work->equip.objs, work->chanl ) ;
		// DG_VisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;
		shift.vz += 200.0F ; /* 怪しい微調整 */
		// BodyAdjust( GM_PlayerBody->objs, &shift ) ;
		BodyAdjust( work->objs, &shift ) ;
	}
	else {
		DG_VisibleObjsChanl( work->equip.objs, work->chanl ) ;
		// DG_InvisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;
	}
	if ( ++ work->time < 8 ) {
		DG_InvisibleObjsChanl( work->equip.objs, work->chanl ) ;
		// DG_InvisibleObjsChanl( work->equip_sub.objs, work->chanl ) ;
	}
	// InvisibleFlags = work->invisible_flags ;

#if 0

//	if( GV_PadData[0].press & PAD_AR ){
	work->timer++;
	if( work->timer%WET_STEP_TIME==0 ){
		if( OK_BodySplashFlag  &&  (OK_cb_wet != NULL) ){
			if( OK_WetLevel < MAX_WET_LEVEL ){
				OK_WetLevel++;
				PutWet( &work->equip.objs->world, &work->size, OK_cb_wet, OK_WetLevel );
			}
		}
	}
#endif


	if( work->now_damage <= 0 ){
//		GV_CallChildSignalFunc( work, GV_SIGNAL_KILL, 0 );
	}


#if 0
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

	GM_MoveTarget2(&(work->target),&(work->objs->objs[work->unit].world));

	CalcHomingMat(work);

	// VisibleBandana( BoxItemNo ) ;
}

static	void	Die( work )
Work		*work ;
{
//	int		i ;

	if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( work->objs, work->equip.objs ) ;
	}
	GM_FreeObject( &( work->equip ) ) ;
//	GM_FreeObject( &( work->equip_sub ) ) ;
//	for ( i = 0; i < work->objs->def->n_x_models; i ++ ) {
//		work->objs->objs[ i ].flag &= ~DG_FLAG_INVISIBLE ;
//	}

#if 0
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
#endif

	// VisibleBandana( -1 ) ;


	// BoxBody = NULL ;
	// OK_HoleFlag = 0;


	GM_FreeTarget(&(work->target));
	GM_FreeHomingTrg(&(work->homing));
}

/*------------------------------------------------------------------*/

static int GetResources(Work *work,CONTROL *ctrl,OBJECT *body,int unit,
						void (*Damaged)(void *w),void *w)
{
	OBJECT	*equip ;
	// FVECTOR	fvtemp;
	DG_OBJS *objs;

	work->bullet_efect_flag = 0;

	// OK_WetLevel = 0;

	// work->timer = 0;

	work->ctrl = ctrl ;
	work->body = body ;
	objs = work->objs = body->objs ;
	work->unit = unit ;
	work->chanl = GM_CurrentCameraChanl ;
	work->Damaged=Damaged;
	work->param_work=w;


//	temp_damage = 25 ;
//	GM_SetItemNum( BoxItemNo, temp_damage ) ;

#if 0
	work->before_damage = work->now_damage = GM_ItemNum( BoxItemNo ) ;
	work->body_damage_radian = 1.0f;

	if( work->before_damage <= DAMAGE_LIMIT_5 ){
		work->break_mode = BOX_STATE_BREAK_5;
	}
	else if( work->before_damage <= DAMAGE_LIMIT_4 ){
		work->break_mode = BOX_STATE_BREAK_4;
	}
	else if( work->before_damage <= DAMAGE_LIMIT_3 ){
		work->break_mode = BOX_STATE_BREAK_3;
	}
	else if( work->before_damage <= DAMAGE_LIMIT_2 ){
		work->break_mode = BOX_STATE_BREAK_2;
	}
	else if( work->before_damage <= DAMAGE_LIMIT_1 ){
		work->break_mode = BOX_STATE_BREAK_1;
	}
	else{
		work->break_mode = BOX_STATE_INITIAL;
	}
#else
	work->now_damage = MAX_DAMAGE ;
	work->body_damage_radian = 1.0f;
	work->break_mode = BOX_STATE_INITIAL;
#endif

	work->body_damage_shift = 0.0f;
	work->body_damage_rot   = 0.0f;

#if 0
	// equip = BoxBody = &( work->equip ) ;
	equip = &( work->equip ) ;
	GM_InitObject( equip, BODY_NAME, BODY_FLAG ) ;
	if ( equip->objs == NULL ) return -1 ;

	GM_ConfigObjectJoint( equip, work->rots );
	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	DG_COPY_VEC( &work->before_mov, &ctrl->mov ) ;
	GM_ConfigObjectRoot( equip, body, unit ) ;
	DG_ConnectObjs( objs, equip->objs ) ;
#else
	work->box_type=0;
	equip = &( work->equip ) ;
	MakeNewBox(work,work->box_type);
#endif

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

#if 0
	/* 主観モデル */
	equip = &work->equip_sub ;
	GM_InitObject( equip, BODY_NAME_SUB, BODY_FLAG_SUB ) ;
	if ( equip->objs == NULL ) return -1 ;
	GM_ConfigObjectJoint( equip, work->rots ) ;
	GM_ConfigObjectRoot( equip, *body, *unit ) ;	
	DG_InvisibleObjs( equip->objs ) ;
#endif

	// OK_cb_wet = NULL;
	work->fall_body = NULL;

#if 0
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
#endif


#if 0
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
#endif


	{
		TARGET *target=&(work->target);
		FVECTOR ofs,sz;
		DG_MDL *mdl;
			
		mdl=&(work->equip.objs->def->models[1]);

		sz.vx=(mdl->ux-mdl->lx)/2;
		sz.vy=(mdl->uy-mdl->ly)/2;
		sz.vz=(mdl->uz-mdl->lz)/2;
		ofs.vx=mdl->lx+sz.vx;
		ofs.vy=mdl->ly+sz.vy;
		ofs.vz=mdl->lz+sz.vz;

		GM_SetTarget(target,
					 TARGET_DEFENSE|TARGET_ROTATE|TARGET_POWER|TARGET_LOCKON,
					 0,BOTH_SIDE,&sz,&ofs);
		GM_MoveTarget2(target,&(work->objs->objs[unit].world));
		GM_SetTargetCallBack(target,target_callback,(void *)work);
		GM_PutTarget(target);
	}


	GM_SetHomingTrg(&(work->homing),&(work->homing_trg_mat),&(work->equip),
					&(work->ctrl->map),NULL,HOMING_ENEMY);
	GM_SetHomingTrgTarget(&(work->homing),&(work->target));

	GM_PutHomingTrg(&(work->homing));

	CalcHomingMat(work);

	// VisibleBandana( BoxItemNo ) ;


	return 0 ;
}

/*------------------------------------------------------------------*/

void *NewC_Box_ForNPC(CONTROL *ctrl,OBJECT *body,int unit,void (*Damaged)(void *w),void *w)
{
	Work	*work ;

	// BoxItemNo = GM_Item ;
#if 0
	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
#else
	work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
#endif

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if(GetResources(work,ctrl,body,unit,Damaged,w) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

#if 0
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->chanl = GM_CurrentCameraChanl ;
#endif

	}
	return work ;
}
