//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shield.c
	攻撃兵用盾
	
	1999/11/11 K.Sigeno
	$Id: shield.c,v 1.1.1.3 2002/11/19 11:49:51 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif


#include	"gameheader.h"
#include	"libutl.h"

#include "korekado/enemy/enemy.h"
#include	"../attacker/at_thk.h"

extern float _RcosF( int);
extern float _RsinF( int);

extern void *NewSpark_White( FMATRIX * );
extern void PutShieldEffectPos( OBJECT* ,FVECTOR* );
extern void *NewShieldEffectControl( void );


#define MODEL_NAME "shl_acr"
#define MAX_MODEL 3	/*切り替えモデル数*/
#define MODEL_CHANGE (3) /*何発あたるごとにモデル切り替わるか*/
#define MODEL_CHANGE_VERY_EASY (1) /*何発あたるごとにモデル切り替わるか*/
#define SHL_VITAL	(MODEL_CHANGE*MAX_MODEL)	/*耐久力*/
#define SHL_VITAL_VERY_EASY	(MODEL_CHANGE_VERY_EASY*MAX_MODEL)	/*耐久力*/

typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		*body ; /*親 人体OBJECT*/
	OBJECT		parts ; 
	OBJECT		light_unit ; /*付属ライトモデル*/
	DG_DEF		*normal_light ;
	DG_DEF		*broken_light ;
	FMATRIX		light[2] ; 
	TARGET			trg;
	POWER_TARGET	power;
	TARGET			light_trg;
	POWER_TARGET	light_power;
	TARGET			offense;	/*破壊時の衝撃を敵本体に与える*/
	POWER_TARGET	offense_power;

	SVECTOR		shakerot;
	ENETHINK	*entk;

	int 		*shield;

	int			now_model;
	int			light_flag;	/*付属ライト制御用*/
	int			light_status;	/*付属ライト制御用*/


	int equip_req ;


	char		count; /*モデル切り替えタイミング*/
	char		dam_time;
	char		pl_status; /*プレイヤ状態検出スイッチ*/
	char		rand_time;	/*乱数ゆらし持続時間*/

}Work ;



#if 0
/*本体の状態によってアタリ判定無し*/
#define	ACT_STATUS_DOWN		/* 倒れ状態 */
#define	ACT_STATUS_FAINT	/* 気絶中 */
#define	ACT_STATUS_GHOST	/* 幽霊 */
#define	ACT_STATUS_DEATH	/* 死亡 */
#define	ACT_STATUS_UNREAL	/* 存在しない */
#define	ACT_STATUS_FALL		/* 倒れている最中 */
#define	ACT_STATUS_SLEEP	/* 眠っている */
#define	ACT_STATUS_HOLD_UP	/* 降参している */
#endif

#define AT_ST_NO_SHIELD (ACT_STATUS_DOWN|ACT_STATUS_FAINT|ACT_STATUS_GHOST|ACT_STATUS_DEATH|ACT_STATUS_UNREAL|ACT_STATUS_FALL|ACT_STATUS_SLEEP|ACT_STATUS_HOLD_UP)



//static void SetOffenseTarget( TARGET *trg ,ENETHINK *entk)
static void SetOffenseTarget( Work *work )
{
	GM_SetTarget( &work->offense, TARGET_OFFENSE, 0, ENEMY_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( &work->offense, 0) ;
	GM_SetPowerTarget( &work->offense, &work->offense_power, POWER_ONCE, 255, 0, 0, &DG_ZeroVector ) ;
	GM_PutTarget( &work->offense ) ;
	/* 直接指定で特定のターゲットに攻撃を当てる */
	GM_TargetSetDirectAttack( &work->offense, &work->entk->act->bodyp.deftrg );
}

/* nop状態にする */
static void SleepShield( work )
Work	*work;
{
	extern void PutShieldEffectBreak( OBJECT*, int );
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)work->entk->character ;

	/*エフェクト非表示*/
	PutShieldEffectBreak( &work->parts, 1 );
	/*モデル非表示*/
	DG_InvisibleObjs( work->parts.objs );
	if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
		DG_InvisibleObjs( work->light_unit.objs );
	}
	/*ターゲットスキップ*/
	work->trg.class |= TARGET_SKIP ;
	work->light_trg.class |= TARGET_SKIP ;
	work->light_flag = 0;
}

static void ShieldBreak( work )
Work	*work;
{
	*work->shield = SHL_ST_BREAK; /*壊れ状態を本体に知らせる*/
//	*work->shield = SHL_ST_NOP; /*壊れ状態を本体に知らせる*/
	work->light_flag = 0;
	work->light_status = 0 ;
	work->light_status = 0 ;
	/*手を広げる*/
	work->entk->sw.finger = 0 ;

	GM_SeSetMode( SD_E_SHIELD01,&work->entk->ctrl->mov,
	GM_SEMODE_NORMAL ) ;

	{
		/* PutShieldEffectPos と 共通のアドレスを指定 */
		extern void PutShieldEffectBreak( OBJECT*, int );
		PutShieldEffectBreak( &work->parts, 0 );	/* 粉々破壊 */
	}
	SleepShield( work ) ;
}

static void RefreshShield( work )
Work	*work;
{
extern void PutShieldEffectBreak( OBJECT*, int );
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)work->entk->character ;

	/*付着エフェクト消し*/
	PutShieldEffectBreak( &work->parts, 1 );
	/*モデル表示*/
	work->parts.flag &= ~DG_FLAG_INVISIBLE ;
	DG_VisibleObjs( work->parts.objs );
	if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
		/*付属ライト*/
		work->light_status = 1 ;
		DG_VisibleObjs( work->light_unit.objs );
		/*壊れ前のモデル*/
	    DG_ChangeModelObj( work->light_unit.objs, &work->light_unit.objs->objs[0], work->normal_light, &work->normal_light->models[0] ) ;
	}
	work->trg.class &= ~TARGET_SKIP ;
	work->light_trg.class &= ~TARGET_SKIP ;

	/*初期状態モデル以外を非表示*/
	work->parts.objs->objs[0].flag &= ~DG_FLAG_INVISIBLE ;
	work->parts.objs->objs[1].flag |= DG_FLAG_INVISIBLE ;
	work->parts.objs->objs[2].flag |= DG_FLAG_INVISIBLE ;
	work->now_model = 0;
	work->count =0;
	work->pl_status = 0;
	work->shakerot.vx = 0; 
	work->shakerot.vy = 0; 
	work->shakerot.vz = 0; 
	work->rand_time = 0;
	
	DG_SetPos( &work->body->objs->objs[HUMAN21_HIDARI_TE].world ) ;
	DG_PutObjs( work->parts.objs ) ;
	if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
		DG_PutObjs( work->light_unit.objs ) ;
	}
	/*ターゲット設定*/
	if ( GM_GameLevel < GM_LEVEL_EASY ) {
		work->power.vital = SHL_VITAL_VERY_EASY ;
	}else {
		work->power.vital = SHL_VITAL ;
	}

}

/*盾持ってる人が死んだ*/
static void EnemyDie( work )
Work	*work;
{
	SleepShield( work ) ;
#if 0
	work->light_flag = 0;
	GV_DestroyActor( work ) ;
	{
		/* PutShieldEffectPos と 共通のアドレスを指定 */
		extern void PutShieldEffectBreak( OBJECT*, int );
//		PutShieldEffectBreak( &work->parts, 0 );	/* 粉々破壊 */
		PutShieldEffectBreak( &work->parts, 1 );	/* 消し */
	}
#endif
}

/* アタリエフェクト */
static void CallSpark( work )
Work	*work;
{
	SVECTOR	spark_rot = { 0,0,1024};
	FMATRIX	spark_m;
	FVECTOR	*hit ;
	TARGET	*trg;


	trg = &work->trg;
	hit = &work->trg.hit;
	DG_SetPos( &work->body->objs->objs[HUMAN21_HIDARI_TE].world );
	DG_RotatePos(&spark_rot);
//	DG_SetPos2( hit, &spark_rot );
	DG_GetPos( &spark_m );

	spark_m.m[ 3 ][ 0 ] = hit->vx ;
	spark_m.m[ 3 ][ 1 ] = hit->vy ;
	spark_m.m[ 3 ][ 2 ] = hit->vz ;

	GV_SetActorChild( work , NewSpark_White( &spark_m ));
	PutShieldEffectPos( &work->parts, hit );
}
#define SHAKE_DIR 50
#define SHAKE_DIRY 400
#define SHAKE_DIRZ 400
#define BLAST_TIME 90
#define	BLAST_RATE 0.9F
/*爆風ゆれ*/
static void RandShakeDirSet(work)
Work	*work;
{
	short range_x,range_y,range_z;
	int dir;
//	float sin;
	
	dir = irnd()&4095;

//	range_x = (short)((float)(work->rand_time) *(1.2F) );
	range_x = 0;
	range_y = (short)((float)(work->rand_time) * _RsinF( dir )*BLAST_RATE );
	range_z = (short)((float)(work->rand_time) * _RcosF( dir )*BLAST_RATE );

	
//	work->shakerot.vx = (short) (-range_x+(irnd()%range_x))%4095;
	work->shakerot.vx = range_x;
	/*横回転*/
//	work->shakerot.vy = (short) (-range_y+(irnd()%range_y))%4095; 
	work->shakerot.vy = range_y; 
	/*縦回転*/
//	work->shakerot.vz = (short) (-range_z+(irnd()%range_z))%4095;
	work->shakerot.vz = range_z;
	work->dam_time = BLAST_TIME;	
}

static void ShakeDirSet( work )
Work	*work;
{
	FVECTOR	tmp,sub;
//	FMATRIX	shake_m,hit_m;
	FMATRIX	shield_m ;
	FVECTOR	*hit ;
	TARGET	*trg;

	trg = &work->trg;
	hit = &work->trg.hit;
	DG_SetPos( &work->body->objs->objs[HUMAN21_HIDARI_TE].world );
	DG_COPY_MAT( &shield_m, &work->body->objs->objs[HUMAN21_HIDARI_TE].world );
	/*hit場所の相対位置*/
	sub.vx = hit->vx - shield_m.m[ 3 ][ 0 ];
	sub.vy = hit->vy - shield_m.m[ 3 ][ 1 ];
	sub.vz = hit->vz - shield_m.m[ 3 ][ 2 ];

	/*盾のworld座標に配置*/
	DG_RotVector( &sub, &tmp, 1 );
	/**/
	work->shakerot.vx = 0;
	/*横回転*/
	work->shakerot.vy = (short)((-1.6F)*(tmp.vz)); 
	/*縦回転*/
	work->shakerot.vz = (short)((0.9F)*(tmp.vy)) ;
	/*乱数ゆらし*/
	work->shakerot.vx += (-(SHAKE_DIR/2)+(irnd()%SHAKE_DIR)); 
	work->shakerot.vy += (-(SHAKE_DIR/2)+(irnd()%SHAKE_DIR)); 
	work->shakerot.vz += (-(SHAKE_DIR/2)+(irnd()%SHAKE_DIR)); 
	work->dam_time = 60;	
}



static void ShakeShield(work)
Work *work;
{
	FVECTOR	shift;
	/* Z->X->Y */
	shift = DG_ZeroVector;
	if(work->dam_time>0) {
		work->dam_time--;
//		shift.vx = (-work->dam_time*1);
//		DG_MovePos( &shift ) ;
		DG_RotatePos( &work->shakerot ) ;
		/*減衰 floatのほうが早い？*/
#if 1
		work->shakerot.vx *= 7;
		work->shakerot.vy *= 7;
		work->shakerot.vz *= 7;
		work->shakerot.vx >>= 3; /* (7/8) */
		work->shakerot.vy >>= 3;;
		work->shakerot.vz >>= 3;;
#else
		work->shakerot.vx = (short)(0.9F*(float)work->shakerot.vx);
		work->shakerot.vy = (short)(0.9F*(float)work->shakerot.vy);
		work->shakerot.vz = (short)(0.9F*(float)work->shakerot.vz);
#endif
	}else {
		work->shakerot.vx =0;
		work->shakerot.vy =0;
		work->shakerot.vz =0;
	}
}

static void SetPlBlood( work )
Work		*work ;
{
//	FMATRIX		*shield_m;
//	FVECTOR		shield_pos;

extern void PutShieldBloodFromPlayer( OBJECT* );

	if(!work->pl_status){
		if (!( work->entk->act->status & ACT_STATUS_UNREAL )) {
			if(GM_PlayerStatus & PLAYER_DAMAGED){
				work->pl_status = 1;
				/*ＣａｌｌＢｌｏｏｄ*/
				if(work->entk->pl_eyei.dis < 2000){
					PutShieldBloodFromPlayer( &work->parts );
				}
			}
		}
	}else {
		if(!(GM_PlayerStatus & PLAYER_DAMAGED)){
			work->pl_status = 0;
		}
	}
}

static	void	Act( work )
Work		*work ;
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) work->entk->character ;

	/*持ってる人が死んだ*/
	if(*work->shield == SHL_ST_ENE_DEATH){
		/*破片出さずに盾消し*/
		EnemyDie( work );
		*work->shield = SHL_ST_NOP; /*盾無し状態に初期化*/
		return;
	}
	if(*work->shield == SHL_ST_BREAK){
//		SetOffenseTarget(&work->offense,work->entk) ;
		SetOffenseTarget(work) ;
//printf("ACT SHL_ST_BREAK \n");
		ShieldBreak(work);
		*work->shield = SHL_ST_NOP; /*壊れ状態を本体に知らせる*/
		return;
	}
	if(*work->shield == SHL_ST_NOP){
		SleepShield( work ) ;
		return;
	}
	if(*work->shield == SHL_ST_MAKE_NEW){
		RefreshShield( work ) ;
		*work->shield = SHL_ST_ACTIVE ;
		return;
	}
	if(work->count> MODEL_CHANGE) {
//		DG_InvisibleObjs( work->parts.objs );
		work->parts.objs->objs[work->now_model].flag |= DG_FLAG_INVISIBLE ;
		work->count = 0;
		work->now_model++;
		if(work->now_model >= MAX_MODEL){
			work->now_model = (MAX_MODEL-1);
//			work->now_model = (0);
		}
//		DG_VisibleObjs( work->parts.objs );
		work->parts.objs->objs[work->now_model].flag &= ~DG_FLAG_INVISIBLE ;
//		printf("NOW MODEL %d\n",work->now_model);
	}

	/*プレイヤ血 チェック*/
	SetPlBlood( work );
	/*オブジェ移動*/
	DG_SetPos( &work->body->objs->objs[HUMAN21_HIDARI_TE].world ) ;
	/*親の非表示に対応*/
	if(work->body->objs->flag & DG_FLAG_INVISIBLE ){
		DG_InvisibleObjs( work->parts.objs );
		work->parts.flag |= DG_FLAG_INVISIBLE ;
		if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
			if(work->light_flag == 1){
				work->light_flag = 0;
			}
			DG_InvisibleObjs( work->light_unit.objs );
		}
	}else {
		DG_VisibleObjs( work->parts.objs );
		work->parts.flag &= ~DG_FLAG_INVISIBLE ;
		if((work->equip_req == AT_EQUIP_LIGHT_SHIELD)&&(work->light_status==1)){
			if(work->light_flag == 0){
				work->light_flag = 1;
			}
			DG_VisibleObjs( work->light_unit.objs );
		}
	}






	if(( work->entk->act->status & AT_ST_NO_SHIELD )
	||(work->body->objs->flag & DG_FLAG_INVISIBLE )
	){
		work->trg.class |= TARGET_SKIP ;
		work->light_trg.class |= TARGET_SKIP ;
	}else {
		work->trg.class &= ~TARGET_SKIP ;
		if(work->light_status ==1) {
			work->light_trg.class &= ~TARGET_SKIP ;
		}
	}

	if(work->rand_time>0){
		if(!(work->rand_time%3)) RandShakeDirSet(work);
		work->rand_time--;
	}
	ShakeShield(work); /*rotateかけてから角度減衰*/

#if 1
	work->light[0] = work->body->objs->light[0];
	work->light[1] = work->body->objs->light[1];
#else
	DG_GetLightMatrix( &work->entk->ctrl->mov, work->light );
#endif
	/*マップ切り替え対応 2000.01.24*/
	work->parts.map_name = work->body->map_name ; /*兵士のマップ*/
	GM_GroupObjs( work->parts.objs, work->body->map_name ) ;
	/*マップ切り替え対応 2002.09.24*/
	/*ライト部分*/
	if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
		work->light_unit.map_name = work->body->map_name ; /*兵士のマップ*/
		GM_GroupObjs( work->light_unit.objs, work->body->map_name ) ;
	}

	DG_PutObjs( work->parts.objs ) ;
	/*ターゲット移動*/
	GM_MoveTarget2Map( &work->trg,&work->parts.objs->world,
		work->entk->ctrl->map);
	if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
		DG_PutObjs( work->light_unit.objs ) ;
		GM_MoveTarget2Map( &work->light_trg,
			&work->parts.objs->world,work->entk->ctrl->map);
	}
}

static	void	Die( work )
Work		*work ;
{
	AT_THK *at_thk ;
	at_thk = (AT_THK *) work->entk->character ;

	GM_FreeObject( &work->parts );
	GM_FreeTarget( &work->trg );
	/*付属ライト*/
	if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
		GM_FreeObject( &work->light_unit );
		GM_FreeTarget( &work->light_trg );
	}
}


extern void *NewSpark2(int,FMATRIX*, float,float,float,SVECTOR*,
	SVECTOR*,FVECTOR*,float,int);

#if 0
int          n_prims,         /* 火花の数 */
FMATRIX      *world,           /* マトリックス*/
float        min_speed,        /* 最小スピード */
float        speed_wide,        /* 幅スピード */
float        gravity,           /* 重力 */
SVECTOR      *rot,              /* 回転 */
SVECTOR      *rot_wide,         /* 幅 */
FVECTOR      *color,            /* 色 */ 
float        length,            /* スピードに対する火の長さの割合 */		
int          count              /* カウント */ 
#endif

static void CrashLight(Work *work){
	extern void	*NewShieldFlashLight(FMATRIX *);
	int i ;
//	GV_SetActorChild( work , NewShieldFlashLight(&work->body->objs->objs[HUMAN21_HIDARI_TE].world));
	/*壊れモデル*/
	for(i=0;i<work->light_unit.objs->n_models;i++){
		DG_ChangeModelObj( work->light_unit.objs, &work->light_unit.objs->objs[i], work->broken_light, &work->broken_light->models[0] ) ;
	}
	work->light_flag = 0 ;
	work->light_status = 0 ;
	work->light_trg.class |= TARGET_SKIP ;

}

/* ターゲットコールバック関数 */
static	void	ChildTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;

	work = ( Work * )ptr ;

    if ( def->damaged & TARGET_POWER ) {
//printf("SHL VITAL %d\n",def->power->vital);
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			if(off->power != NULL ) {
				def->power->force = off->power->force ;
			}
		 	if(
		 	(( def->weapon_type & WP_KICK)
			&& (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) 
			&&(!(GM_PlayerStatus & PLAYER_ROLLING) ))
			||(def->weapon_type & WP_BLADALL)
			){ 
				/*一発気絶武器で殴られた*/
				def->power->vital = -1;
				if ( def->power->vital <= 0 ) {
					printf("SHIELD BROKEN !!\n");
//					ShieldBreak(work);
	*work->shield = SHL_ST_BREAK; /*壊れ状態を本体に知らせる*/

				}
			}
			if( def->weapon_type & WP_BULLET){
				CallSpark(work);
				ShakeDirSet(work);
				if(work->now_model < (MAX_MODEL-1)){
					/*モデル変更カウンタ*/
					work->count++;
				}
				def->power->vital--;
				if ( def->power->vital <= 0 ) {
					printf("SHIELD BROKEN !!\n");
//					ShieldBreak(work);
	*work->shield = SHL_ST_BREAK; /*壊れ状態を本体に知らせる*/

				}
			}else if(def->weapon_type & WP_M92){
				ShakeDirSet(work);
			}else if( def->weapon_type & WP_BLAST){
				work->rand_time = BLAST_TIME;
			}
		}
	}
	def->weapon_type = 0 ;
}
/* 付属ライトターゲットコールバック関数 */
static	void	LightTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;

	work = ( Work * )ptr ;

    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			if(off->power != NULL ) {
				def->power->force = off->power->force ;
			}
		 	if( def->weapon_type & WP_BULLET){
//				def->power->vital--;
				CrashLight(work);
				/*盾本体を揺らす*/
				work->trg.hit = work->light_trg.hit;
				ShakeDirSet( work );
				/*ＳＥコール*/
				GM_SeSetMode( SD_E_SLIGHT01,
				&work->light_trg.hit,GM_SEMODE_NORMAL ) ;
			}
		}
	}
	def->weapon_type = 0 ;
}

/*------------------------------------------------*/
static void SetDefTarget(work)
Work *work;
{
	int shield_flag,map;
	/*ちょっと厚めにしとく*/
	FVECTOR	shl_size = { 90.0, 400.0, 250.0 } ;

//	FVECTOR	light_size = { 60.0, 120.0, 120.0 } ;
	FVECTOR	light_size = { 60.0, 60.0, 62.0 } ;
	FVECTOR	light_shift = { 54.0, -100.0, 203.0 } ;
	/*あたりサイズ 厚み 高さ 横幅 の順*/
	AT_THK *at_thk ;
	at_thk = (AT_THK *) work->entk->character ;


	/*ターゲット設定*/
	/*盾*/
	shield_flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON
	|TARGET_POWER|TARGET_ROTATE);
	map = work->entk->ctrl->map;
	GM_SetTarget( &work->trg, shield_flag,map, ENEMY_SIDE, &shl_size, &DG_ZeroVector ) ;

	GM_SetTargetCallBack( &work->trg, ChildTargCallBack, work ) ;
	GM_SetTargetWeaponType(&work->trg,0);

	if ( GM_GameLevel < GM_LEVEL_EASY ) {
		GM_SetPowerTarget( &work->trg, &work->power,POWER_DECREASE, SHL_VITAL_VERY_EASY, 0, 0, &DG_ZeroVector );
	}else {
		GM_SetPowerTarget( &work->trg, &work->power,POWER_DECREASE, SHL_VITAL, 0, 0, &DG_ZeroVector );
	}

	GM_PutTarget( &work->trg );

	if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
		GM_SetTarget( &work->light_trg, shield_flag,map,
			 ENEMY_SIDE, &light_size, &light_shift ) ;
		GM_SetTargetCallBack( &work->light_trg, 
			LightTargCallBack, work ) ;
		GM_SetTargetWeaponType(&work->light_trg,0);
		if ( GM_GameLevel < GM_LEVEL_EASY ) {
			GM_SetPowerTarget( &work->light_trg, &work->light_power,POWER_DECREASE, SHL_VITAL_VERY_EASY, 0, 0, &DG_ZeroVector );
		}else {
			GM_SetPowerTarget( &work->light_trg, &work->light_power,POWER_DECREASE, SHL_VITAL, 0, 0, &DG_ZeroVector );
		}
		GM_PutTarget( &work->light_trg );
	}
printf("SHIELD TRG SET ID %d\n",work->entk->id);

}

//shl_lit1
static	int	GetResources( work, body ,entk,act_obj)
Work	*work ;
OBJECT	*body ;	/* ボディーオブジェクト */
ENETHINK	*entk;
OBJECT	**act_obj; /*actからobjctを参照*/
{
extern void *NewCircleLight( FMATRIX*, int*, int, int* );

	int model_code,light_code;
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	/*胴体オブジェクト*/

	work->body = body ;

	/*制御フラグ*/
//	work->shield = &(at_thk->shield) ;
	work->shield = &(entk->sw.shield) ;

	work->equip_req = at_thk->equip_req ;

	*work->shield = SHL_ST_NOP ;

	work->entk = entk;


	/*actionからobjctを参照する*/
	*act_obj = &work->parts;

	/*モデルID*/
	model_code = GV_StrCode( MODEL_NAME ) ;

	printf("SHIELD OBJ INIT\n");
	GM_InitObject( &work->parts,model_code,
		DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	if(work->parts.objs == NULL) {
		printf("shield obj init failed\n");
		return -1;
	}

	/*付属ライト*/
	if(work->equip_req == AT_EQUIP_LIGHT_SHIELD){
		light_code = GV_StrCode( "shl_lit1" );
		GM_InitObject( &work->light_unit,light_code,
		DG_FLAG_SHADE|DG_FLAG_ONEPIECE) ;
		DG_SetLightMatrix( work->light_unit.objs, work->light );
		work->light_flag = 1;
		GV_SetActorChild( work , NewCircleLight( &work->light_unit.objs->world,
		&work->light_flag , 2, &work->body->map_name ));
		DG_PutObjs( work->light_unit.objs ) ;
		work->normal_light= (DG_DEF*) GV_GetCache( GV_CacheID( GV_StrCode( "shl_lit1" ), 'k' ) );
		work->broken_light= (DG_DEF*) GV_GetCache( GV_CacheID( GV_StrCode( "shl_lit2" ), 'k' ) );
		work->light_status = 1 ;
	}else {
		work->light_status = 0 ;
	}

	DG_SetLightMatrix( work->parts.objs, work->light );
	/*初期状態モデル以外を非表示*/
	work->parts.objs->objs[1].flag |= DG_FLAG_INVISIBLE ;
	work->parts.objs->objs[2].flag |= DG_FLAG_INVISIBLE ;

	work->now_model = 0;
	work->count =0;
	work->pl_status = 0;
	work->shakerot.vx = 0; 
	work->shakerot.vy = 0; 
	work->shakerot.vz = 0; 
	work->rand_time = 0;
	
	DG_SetPos( &work->body->objs->objs[HUMAN21_HIDARI_TE].world ) ;
	DG_PutObjs( work->parts.objs ) ;


	/*ターゲット設定*/
	SetDefTarget(work);

	*work->shield = SHL_ST_ACTIVE; /*リソース確保に成功*/
	/* 何回呼んでも２回目以後は無視される */
//	GV_SetActorChild( work ,NewShieldEffectControl());
	//ステージの最後までいていいプログラムなのでchildにはしない。
	NewShieldEffectControl() ;
	return 0 ;
}
void	*NewAT_Shield( body ,entk,act_obj)
OBJECT	*body ;	/* 人体オブジェクト */
ENETHINK	*entk;	/* 敵兵思考 */
OBJECT	**act_obj; /*actからobjctを参照*/
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), AFTER_ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body ,entk,act_obj) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
