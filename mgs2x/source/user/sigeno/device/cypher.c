//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	cypher.c								*/
/*	サイファ	*NewCypher						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cypher.c,v 1.1.1.3 2002/11/19 11:49:28 Yoshizawa1 Exp $									*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

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
#include <float.h>

#include "gameheader.h"
#include "libutl.h"

#include "tmp_inc.h"

#include "korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"
#include "eyecheck.h"
#define		CAMERA_PARTS (1)
#define		CYP_EYE_S_DEF (6000)
#define		CYP_DIR_Y_MAX	(512)
#define		DECAY_RATE		(0.990F)
#define		BRAKE_RATE		(0.975F)
#define		BRAKE_RATE2		(0.975F)
#define		CYP_SPEED_LOW_LIMIT (0.005F)
#define		CYP_ACCELE	(2.0F)
#define		DEF_MAX_SPEED	(80) /*最高速*/
#define		ROT_SPEED	(300)
#define		CYP_VITAL	(16)


/********************************************************************************/
/*	extern									*/
/********************************************************************************/

extern void CYP_SetNextnode( ROUTENAVI	*rnavi, TRGPOINT	*trgp );

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
/*ポイントタイムの値がこの時、mesg受信まで待機*/
#define		P_TIME_WAIT	(-3)	


/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	int			name ;
	OBJECT		body ;
	FMATRIX		lights[2] ;
	CONTROL		control;
	ROUTENAVI	rnavi;	/*ルート誘導*/
	TRGPOINT	trg;	/*移動目標情報*/
	int			node ;	/*ノード番号*/
	int			route ; /*ルート番号*/
	int			headmark ;
	EYEPARAM	eye ;
	DEV_CTRL	b_ctrl ; /*胴体傾き用簡易CTRL*/
	FVECTOR		camera ;	/*視界の開始点*/
	int			alert ;

	short		rot ;		/*羽回転*/
	short		count;		/*本体揺らしカウント*/

	u_char		mode ;
	u_char		type ;
	u_short		still_se_num ;

	u_short		se_num ;
	u_short		se_sw;

	int			face_y_buf;

	TARGET			b_trg;
	POWER_TARGET	b_power;

	TARGET			h_trg;
	POWER_TARGET	h_power;

	FVECTOR		shake ; 
	FVECTOR		d_shake ; /*揺れ角速度*/

	FVECTOR		nowpos ; /*現在*/

	float		max_speed ; 
	int			status ;
} Work ;
enum {
	MOVE_MODE,ACTION_MODE
};
enum {
	MSG_ROUTE_CHANGE = 1
};
enum {
	NORMAL_TYPE,GUN_TYPE
};
enum {
	CYP_ACTIVE,CYP_WAIT
};

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetPointAction( Work *work ){
	ROUTENAVI *rnavi ;
	rnavi = &work->rnavi ;
	rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node];
	rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
	rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node];
	rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node];
	rnavi->p_actstatus = rnavi->pa_flag[(int)rnavi->next_node];

	if(rnavi->p_acttime == P_TIME_WAIT){
		work->status = CYP_WAIT ;
	}

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code  ;
    n_msg = work->control.n_msg ;
	msg = work->control.msg ;

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case MSG_ROUTE_CHANGE :
				work->route = msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->route += GM_RouteOffset ; 
				}
				ENE_InitRouteNavi( &work->rnavi, work->route, 0 );
				SetPointAction( work );
				work->trg.pos = work->rnavi.nodes[ 0 ] ;
				work->trg.map = work->rnavi.mapbit[ 0 ] ;
				work->trg.addr = HZX_GetAddress( work->trg.map,
					&work->trg.pos, -1 ) ;
				work->mode = MOVE_MODE ;
				break ;
			case ENE_MSG_POINT_ACTION_START :
				/*移動許可*/
				work->status = CYP_ACTIVE ;
				break ;
		}
		msg++ ;
	}
}

/*揺らし系 係数*/
#define	BULLET_FORCE	(30.0F) 
#define	DIR_RATE		(0.03F) /*位置エネルギ*/
#define	DIR_DECAY_RATE	(0.95F) /*自然減衰*/
#define	DIR_MIN			(1.0F)		/*切り捨て*/
#define	 SPEED_RATE		(0.01F)
#define	 ACCEL_RATE		(3.0F)


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*絶対座標をwolrd上の相対座標に変換*/
static void HitPos2Rot( FVECTOR *pos,FVECTOR *res,FMATRIX *world )
{
	FVECTOR	sub;
	DG_SetPos( world );
	/*hit場所の相対位置*/
	sub.vx = pos->vx - world->m[ 3 ][ 0 ];
	sub.vy = pos->vy - world->m[ 3 ][ 1 ];
	sub.vz = pos->vz - world->m[ 3 ][ 2 ];
	/*world座標に配置*/
	DG_RotVector( &sub, res, 1 );
}



/*胴体アタリサイズ*/
#define	BODY_SIZE_XZ	(700.0F)
#define	BODY_SIZE_Y	(150.0F)
/*カメラアタリサイズ*/
#define	HEAD_SIZE_XZ	(150.0F)
#define	HEAD_SIZE_Y	(100.0F)
#define HEAD_SHAKE_RATE		(3.0F)

#define CYP_SHAKE_LIMIT		(200)


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*FVECに指定範囲内で乱数増減させる*/
static void SetRandFvec(FVECTOR *pos,int wide){
	float rand;

	pos->vx -= (float) wide ;
	pos->vy -= (float) wide ;
	pos->vz -= (float) wide ;

	rand = (float) (irnd() % (wide*2) ) ;
	pos->vx += rand;
	rand = (float) (irnd() % (wide*2) ) ;
	pos->vy += rand;
	rand = (float) (irnd() % (wide*2) ) ;
	pos->vz += rand;

}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void ShakeDirSet_Body( work )
Work	*work;
{
	FVECTOR	tmp ;
	FVECTOR	force;
	float	len ,sign;
	
	HitPos2Rot( &work->b_trg.hit, &tmp ,&work->body.objs->objs[0].world );

	if(tmp.vy >= 0) sign = 1.0F ;
	else sign = -1.0F ;

	/*乱数ゆらし*/
	SetRandFvec(&tmp,200);
	/**/
	len = GV_VecLen3F( &tmp ) ;
	if( len >  BODY_SIZE_XZ ) len = BODY_SIZE_XZ ;
	GV_LenVec3F( &tmp, &force, 0.0F, len );
	
	work->d_shake.vx += sign*(force.vz / BODY_SIZE_XZ ) * BULLET_FORCE ;
	work->d_shake.vz -= sign*(force.vx / BODY_SIZE_XZ ) * BULLET_FORCE ;

}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void ShakeDirSet_Head( work )
Work	*work;
{
	FVECTOR	tmp ;
	FVECTOR	force;
	
	/*揺らしは胴体部分を重心として判定*/
	HitPos2Rot( &work->h_trg.hit, &tmp ,&work->body.objs->objs[0].world );

	work->h_power.force.vy = 0.0F ;
	GV_LenVec3F( &work->h_power.force, &force, 0.0F, BODY_SIZE_XZ );

	work->d_shake.vx += HEAD_SHAKE_RATE
		*(force.vz / BODY_SIZE_XZ ) * BULLET_FORCE ;
	work->d_shake.vz -= HEAD_SHAKE_RATE
		*(force.vx / BODY_SIZE_XZ ) * BULLET_FORCE ;
}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* ターゲットコールバック関数 */
static	void	ChildTargCallBack_Body( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = off->power->force ;
		 	if( def->weapon_type & WP_BULLET){
				ShakeDirSet_Body(work);
				work->control.step.vx += def->power->force.vx * 0.0200F;
				work->control.step.vz += def->power->force.vz * 0.0200F;
				if ( def->power->vital <= 0 ) {
					/*破壊された*/
				}
			}else if( def->weapon_type & WP_BLAST){
				/*爆発系ヒット*/
			}
		}
	}
	def->weapon_type = 0 ;
}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	ChildTargCallBack_Head					*/
/*	引数:	TARGET	*off   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	ChildTargCallBack_Head( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = off->power->force ;
		 	if(
		 	( def->weapon_type & WP_BULLET)
		 	){
				ShakeDirSet_Head(work);
				work->control.step.vx += def->power->force.vx * 0.0200F;
				work->control.step.vz += def->power->force.vz * 0.0200F;
				if ( def->power->vital <= 0 ) {
					/*破壊された*/
				}
			}
		}
	}
	def->weapon_type = 0 ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetTarget							*/
/*	引数:	Work *work   						       	*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetTarget(work)
Work *work;
{
	int flag,map;
	/*ちょっと厚めにしとく*/
	FVECTOR	b_size = { BODY_SIZE_XZ, BODY_SIZE_Y, BODY_SIZE_XZ } ;
	FVECTOR	h_size = { HEAD_SIZE_XZ, HEAD_SIZE_Y, HEAD_SIZE_XZ } ;

	/*あたりサイズ 厚み 高さ 横幅 の順*/

	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON
//	|TARGET_ROTATE
	|TARGET_POWER);
	map = work->control.map;
	GM_SetTarget( &work->b_trg, flag,map, ENEMY_SIDE, 
		&b_size, &DG_ZeroVector ) ;
	GM_SetTarget( &work->h_trg, flag,map, ENEMY_SIDE, 
		&h_size, &DG_ZeroVector ) ;

	GM_SetTargetCallBack( &work->b_trg, ChildTargCallBack_Body, work ) ;
	GM_SetTargetCallBack( &work->h_trg, ChildTargCallBack_Head, work ) ;

	GM_SetTargetWeaponType(&work->b_trg,0);
	GM_SetTargetWeaponType(&work->h_trg,0);

	GM_SetPowerTarget( &work->b_trg, &work->b_power,
	POWER_DECREASE, CYP_VITAL, 0, 0, &DG_ZeroVector );

	GM_SetPowerTarget( &work->h_trg, &work->h_power,
	POWER_DECREASE, CYP_VITAL, 0, 0, &DG_ZeroVector );

	GM_PutTarget( &work->b_trg );
	GM_PutTarget( &work->h_trg );
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void DecayStep							*/
/*	引数:	CONTROL	*ctrl							*/
/*		float	rate							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void DecayStep(CONTROL *ctrl ,float rate){

	FVECTOR *step;
	step = &ctrl->step ;

	step->vx *= rate ;
	step->vy *= rate ;
	step->vz *= rate ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void LowLimtStep						*/
/*	引数:	CONTROL	*ctrl							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void LowLimtStep(CONTROL *ctrl){

	FVECTOR *step;
	step = &ctrl->step ;
	if( abs(step->vx) < CYP_SPEED_LOW_LIMIT) step->vx = 0.0F ;
	if( abs(step->vy) < CYP_SPEED_LOW_LIMIT) step->vy = 0.0F ;
	if( abs(step->vz) < CYP_SPEED_LOW_LIMIT) step->vz = 0.0F ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	float AbsLimit							*/
/*	引数:	float	base							*/
/*		float	limit							*/	
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static float AbsLimit(float base,float limit){
	if(abs(base) > limit ) {
		if(base >= 0 ) return limit ;
		else return (- limit) ;
	}else {
		return base ;
	}
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	float	AccelForce						*/
/*	引数:	float	step						       	*/
/*	説明:						       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static float AccelForce( float step){
	return (step * ACCEL_RATE) ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int DirectTrace3D						*/
/*	引数:	Work	*work  						       	*/
/*		float	range							*/
/*		int	*brake							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int DirectTrace3D(Work *work, float range,int *brake){

	FVECTOR		shift ,*pos,accele ;
	float		dx,dy, dz ,dis,rate;
	TRGPOINT *trgp ;
	CONTROL *ctrl ;

	trgp = &work->trg ;
	ctrl = &work->control ;

	pos = &ctrl->mov;
	shift.vx = dx = trgp->pos.vx - pos->vx ;
	shift.vy = dy = trgp->pos.vy - pos->vy ;
	shift.vz = dz = trgp->pos.vz - pos->vz ;

	if ( dx > -range && dx < range 
		&& dy > -range && dy < range 
		&& dz > -range && dz < range )
	{
		/*到達*/
		LowLimtStep(ctrl) ;
		trgp->h_dis = 0 ;
		return -1 ;
	}
	trgp->dir = _FVecDir2( &shift ) ;
	trgp->h_dis = (int)_FVecLen2( &shift ) ;

	dis = GV_VecLen3F( &shift ) ;

	if( dis > 3000.0F) {
		rate = 1.0F ;
		if(brake != NULL){
			*brake = 0 ;
		}
	}else {
		rate = (dis / 2000.0F) ;
		if(brake != NULL){
			*brake = 1 ;
		}
	}

	rate *= 0.8F;
	if(work->eye.alert_time) rate = 0.0F;

	if(brake != NULL){
		if( dis > 1000.0F) {
			*brake = 0 ;
		}else {
			*brake = 1 ;
		}
	}

	if( dis > 3000.0F) {
		DecayStep(ctrl,DECAY_RATE) ;
	}else {
		DecayStep(ctrl,BRAKE_RATE) ;
	}
	/*現在の速度*/
	GV_LenVec3F( &shift, &accele, 0.0F, CYP_ACCELE*rate ) ;
	ctrl->step.vx += accele.vx ;
	ctrl->step.vx = AbsLimit(ctrl->step.vx,work->max_speed);

	ctrl->step.vy += accele.vy ;
	ctrl->step.vy = AbsLimit(ctrl->step.vy,work->max_speed);

	ctrl->step.vz += accele.vz ;
	ctrl->step.vz = AbsLimit(ctrl->step.vz,work->max_speed);

	work->d_shake.vx += AccelForce(accele.vz) ;
	work->d_shake.vz -= AccelForce(accele.vx) ;

	return 0 ;

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int MoveRoute							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ルートを廻る				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int MoveRoute(Work *work){
	ROUTENAVI *rnavi ;
	FVECTOR aim,line[2];
	int brake = 0;

//extern void *NewLineView(FVECTOR * ,int,u_char,u_char,u_char) ;

	rnavi = &work->rnavi ;
	/*移動制御と到達チェック*/
	if(DirectTrace3D(work,100.0F,&brake) < 0 ){
		/*ポイント到達*/
		if(rnavi->p_acttime == 0 ){
	/*待機時間無しなのですぐに次ポイントへ*/
			work->nowpos = work->trg.pos ;
//			ENE_SetTrgpNextnode( rnavi, &work->trg ) ;
			CYP_SetNextnode( rnavi, &work->trg ) ;
#ifdef NAVI_RISE
	work->trg.pos.vy += 2000.0F ;
#endif
			SetPointAction( work );
		}else {
	/*待機モードへ*/
			work->mode = ACTION_MODE ;
		}
	}else {
		/*移動中の顔方向チェック*/
		/*発見していなければ*/
		/*注視点見ながら移動*/
		if(rnavi->p_actstatus & PA_CON_AIM_FACE_MOVE){
			aim = rnavi->aimnodes[(int)rnavi->next_node];
			SIG_VecDir(&work->camera,&aim,&work->control.turn);
			line[0] = work->camera ;
			line[1] = aim ;
			NewLineView( &line[0] ,1,0,255,0) ;
		}else {
			/*進行方向*/
			work->control.turn.vy = work->trg.dir ;
			work->control.turn.vx = 0 ;
		}
	}
	return	brake ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void ActionSeq							*/
/*	引数:	Work *work   						       	*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void ActionSeq(Work *work)
{
	if(work->rnavi.p_dir >=0 ) {
		work->control.turn.vy = work->rnavi.p_dir ;
		work->control.turn.vx = 0 ;
	}
	if(
	(work->rnavi.p_acttime>0)
	&&(work->rnavi.p_acttime != P_TIME_WAIT)
	){
/*待機時間中*/
		work->rnavi.p_acttime--;
	}else if(
	(work->rnavi.p_acttime <= 0)
	&&(work->rnavi.p_acttime != P_TIME_WAIT)
	){
/*待機終了 次のポイントセット*/
		work->nowpos = work->trg.pos ;
		CYP_SetNextnode( &work->rnavi, &work->trg ) ;

		SetPointAction( work );
		work->mode = MOVE_MODE ;
	}else if (work->rnavi.p_acttime == P_TIME_WAIT){
		/*メッセージ受信するまで待機*/	
		if(work->status == CYP_ACTIVE ){
			/*待機終了 次のポイントセット*/
			work->nowpos = work->trg.pos ;
			CYP_SetNextnode( &work->rnavi, &work->trg ) ;
			SetPointAction( work );
			work->mode = MOVE_MODE ;
		}
	}
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetFaceDir							*/
/*	引数:	Work *work   						       	*/
/*	説明:						       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetFaceDir(Work *work){
	work->eye.rot = work->control.rot ;
	KR_FMatToFvec( &BODYWORLD(&work->body, CAMERA_PARTS),
		&work->camera ) ;
}
#define SHAKE_LIMIT		(512)

static float DecayRot(float *dir){
	float res = 0 ;
	res = *dir ;
	if(res !=0.0F ) {
		res *= DIR_DECAY_RATE ;
	}
	if(abs(res)< DIR_MIN ) res = 0 ;
	*dir = res ;
	return (short) res ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	float	SpeedForce						*/
/*	引数:	float	step							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static float SpeedForce( float step){
	return (step * SPEED_RATE) ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	float	PosForce						*/
/*	引数:	float	dir							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static float PosForce( float dir){
	short sign ;

	if(dir >= 2048 ) sign = (dir-4096) ; 
	else sign = dir ; 

	return ((float)sign * (- DIR_RATE )) ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	SignSet							*/
/*	引数:	short	*dir							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SignSet(short *dir){
	short sign ;
	if(*dir >= 2048) sign = *dir - 4096 ; 
	else sign = *dir ;
	if(abs(sign) > CYP_SHAKE_LIMIT){
		if(sign >= 0) sign = CYP_SHAKE_LIMIT ;
		else sign = -CYP_SHAKE_LIMIT ;
	}
	*dir = sign ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_DurLimit						*/
/*	引数:	SVWCTOR	*dir							*/	
/*	説明:	向ける方向の限界を設定する
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetDirLimit(SVECTOR	*dir){

	dir->vx &=4095 ;
	dir->vy &=4095 ;
	dir->vz &=4095 ;

	SignSet(&dir->vx);
	SignSet(&dir->vz);
} ;

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetBodyRot							*/
/*	引数:	Work *work   						       	*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetBodyRot(Work *work){
	FVECTOR		*shake ;
	FVECTOR		*d_shake ;

	shake = &work->shake ;
	d_shake = &work->d_shake ;

	d_shake->vx += PosForce(shake->vx) ;
	d_shake->vz += PosForce(shake->vz) ;

	d_shake->vx += SpeedForce(work->control.step.vz) ;
	d_shake->vz -= SpeedForce(work->control.step.vx) ;

	shake->vx += DecayRot(&d_shake->vx) ;
	shake->vz += DecayRot(&d_shake->vz) ;

	work->b_ctrl.turn.vx = (short) work->shake.vx ;
	work->b_ctrl.turn.vz = (short) work->shake.vz ;

	SetDirLimit(&work->b_ctrl.turn) ;
	DEV_CopySvec2Fvec(&work->b_ctrl.turn ,&work->shake ) ;
}


#if 0
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void ShiftShake							*/
/*	引数:	Work *work   						       	*/
/*	説明:						       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void ShiftShake(Work *work){
	int dir ;
	float force;

	dir = irnd()&4095;
	force = (float) ((irnd()%12)+1);

//	work->control.step.vy -= force * 0.60F ;
	work->d_shake.vx = _RcosF( dir) * (force*1.5F);
	work->d_shake.vz = _RsinF( dir) * (force*1.5F);

//	SetRandFvec( &work->d_shake, 8);
//	SetRandFvec( &work->control.step, 4);
}
#endif
#if 0
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CYP_SetCameraSe						*/
/*	引数:	Work *work   						       	*/
/*	説明:	カメラのSE再生				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CYP_SetCameraSe(Work *work) {
	/*カメラ部首振り音処理*/
	int dirsub ;
	if((DG_TickCount % 9 )==work->se_sw){
		dirsub = DEV_CheckDirSub(work->face_y_buf,
		work->control.rot.vy) ;
		if(abs(dirsub)> 16){
			GM_SeSetMode( SD_E_CAMMOV01 ,
			&work->control.mov,GM_SEMODE_NORMAL ) ;
		}
	}
	work->face_y_buf = work->control.rot.vy ;
}
#endif

static void SetObjs(Work *work){
	SVECTOR rot ;
	FVECTOR	pos;
	FMATRIX	world;

	/*羽回転*/
	rot = DG_ZeroSVector ;
	rot.vy = work->rot; 

	/*胴体サインカーブ揺らし*/
	pos = DG_ZeroVector ;
	pos.vy = (_RsinF( (int)work->count )) * 60.0F ;
	DG_SetPos2( &work->control.mov, &work->b_ctrl.rot ) ;
	/*揺らし後基準位置*/
	DG_MovePos( &pos ) ;
	DG_GetPos( &world ) ;

	/*胴体*/
	DG_SetPos( &world ) ;
	DG_PutObjs( work->body.objs );
	GM_GroupObjs( work->body.objs, work->control.map ) ;
	DG_GetPos( &work->body.objs->objs[0].world  ) ;
	/*頭*/
	DG_SetPos( &world ) ;
	DG_MovePos( &work->body.objs->objs[1].trans ) ;
	DG_RotatePos( &work->control.rot ) ;

	DG_GetPos( &work->body.objs->objs[1].world  ) ;
	/*羽*/
	DG_SetPos( &world ) ;
	DG_MovePos( &work->body.objs->objs[2].trans ) ;
	DG_RotatePos( &rot ) ;
	DG_GetPos( &work->body.objs->objs[2].world  ) ;

	work->rot += ROT_SPEED;
	work->rot &= 4095 ;
	work->count += 32 ;
	work->count &= 4095 ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Act							*/
/*	引数:	Work *work   						       	*/
/*	説明:	実行関数				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Act(Work *work)
{
	OBJECT *body ;
	int		brake = 0;
	
	body = &work->body ;
	SetFaceDir(work);
	SetBodyRot(work);
#ifdef DEBUG_MODE
	DEV_DebugMode(&work->eye);
#endif
	DEV_EyeInfoCheck( &work->eye ,work->control.hzx_id ) ;
	if(work->eye.discv_time == 0 ) DEV_CameraSe(&work->eye) ;

	DEV_HeadMarkCheck(&work->eye,
		&work->body.objs->objs[CAMERA_PARTS].world,&work->headmark) ;
	DEV_Security(&work->eye) ;

	switch (work->type){
		case NORMAL_TYPE :
			/*通報*/
			DEV_Security(&work->eye) ;
			break ;
		case GUN_TYPE :
			if((work->eye.discv_time <= HEADMARK_SET )
				&&(work->eye.alert_time == SIGHT_ALERT)){
				if( (irnd()&15) == 0 ) {
					DEV_Bullet(&work->eye,
					&work->body.objs->objs[CAMERA_PARTS].world);
				}
			}
			break ;
	}

	if(work->eye.alert_time > 0 ){
		/*発見中*/
		if(DirectTrace3D(work,100.0F,NULL)<0){
			/*止める*/
		}else {
		}
		SIG_VecDir(work->eye.eyepos,work->eye.trgpos,
		&work->control.turn);
		DecayStep(&work->control,BRAKE_RATE2) ;

	}else {
		/*巡回中*/
		switch(work->mode){
			case MOVE_MODE :
				brake = MoveRoute(work);
				work->b_ctrl.interp = 16 ;
				break;
			case ACTION_MODE :
/*待機場所へ補正*/
				if(DirectTrace3D(work,100.0F,NULL)<0){
					/*止める*/
				}else {
				}
				ActionSeq(work);
				break;
		}
	}
	/**/
	DEV_DirLimitX(&work->control.turn,CYP_DIR_Y_MAX,CYP_DIR_Y_MAX ) ;
	work->control.interp = 8 ;

	if((DG_TickCount % 10)== work->se_sw ){
		if(
		(work->mode == MOVE_MODE )
		&&(brake == 0)
		&&(work->control.step.vy >= -10.0F)
		){
			GM_SeSetMode( work->se_num , &work->control.mov,
			GM_SEMODE_NORMAL ) ;
		}else {
			GM_SeSetMode( work->still_se_num , &work->control.mov,
			GM_SEMODE_NORMAL ) ;
		}
	}

	GM_ActControl(&work->control) ;
/*DEBUG*/

	DEV_InterpRot(&work->b_ctrl) ;
	DG_GetLightMatrix( &work->control.mov, work->lights );
	SetObjs(work);

	GM_MoveTarget3Map( &work->b_trg,&work->body.objs->world,
		work->control.map);
	GM_MoveTarget3Map( &work->h_trg,
	&work->body.objs->objs[1].world,work->control.map);



#if 0
	NewTargetView2( &work->b_trg, 232, 32, 232 ) ;
	NewTargetView2( &work->h_trg, 32, 232, 32 ) ;
#endif
	CheckMessage(work) ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ワークの解放をする			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Die(Work *work)
{
	GM_FreeObject(&(work->body));
	GM_FreeControl( &work->control);
	GM_FreeTarget( &work->h_trg ) ;
	GM_FreeTarget( &work->b_trg ) ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void GetResources						*/
/*	引数:	Work *work   						       	*/
/*		int  name							*/
/*		int  where							*/	
/*	説明:	ワークの確保をする			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int GetResources(Work *work, int name, int where)
{

    int model;
	SVECTOR	range ;
	float	length ;
	work->name = name ;
	GM_InitControl( &work->control, name, where );
	GM_ConfigControlMessageCheck( &work->control ) ;
	GM_ConfigControlMapCheck( &work->control ) ;
	work->control.interp = 16 ;
	DEV_InitDevCtrl(&work->b_ctrl,work->control.interp) ;

	/*CONTROLフラグ*/
	work->control.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK
	|CTRL_SKIP_GET_ADDRESS
	|CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK
	|CTRL_SKIP_TRAP
	); 
	/*胴体の方向とゆれを乱数セット*/
	work->rot = (irnd()&4096);
	work->count = (irnd()& 4095);
	/*傾き*/
	work->shake = DG_ZeroVector ;
	work->d_shake = DG_ZeroVector ;

	/*最高速度*/
	work->max_speed = (float) GCL_GetOptionValue( 's', DEF_MAX_SPEED ) ;

	/*装備タイプ*/
	work->type = GCL_GetOptionValue( 't', NORMAL_TYPE ) ;

	/*探査目標*/
	work->eye.trgpos = &GM_PlayerFindPos ;
	/*顔の位置*/
	work->eye.eyepos = &work->camera ;
	/*顔の向き*/
	work->eye.rot  = DG_ZeroSVector ;
	work->face_y_buf = (int) work->eye.rot.vy ;
	/*視野角*/

	range.vx = GCL_GetOptionValue( 'x', 512 ) ; 
	range.vy = GCL_GetOptionValue( 'y', 512 ) ;
	range.vz = 0;

	/*視力*/
	length = GCL_GetOptionValue( 'i', CYP_EYE_S_DEF  );
	DEV_InitEyeParam(&work->eye,&range,length);
	work->alert = 0 ;
	work->mode = MOVE_MODE ;

	/* モデル */
	model = GV_StrCode("cyp") ;

	/*ルート設定*/
	work->node = 0;
	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->route = GCL_GetNextInt() ;

		if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
			work->route += GM_RouteOffset ; 
		}
		if( GCL_NextStr() != NULL ){
			work->node = GCL_GetNextInt() ;
		}
		ENE_InitRouteNavi( &work->rnavi, work->route, work->node );
		SetPointAction( work );
	}else {
		return 0 ;
	}

	/*ルートポイントに配置*/
	work->trg.pos = work->control.mov = work->rnavi.nodes[ work->node ] ;
	work->nowpos = work->control.mov ;

	work->se_sw = (work->route%5)*2 ;
	switch (work->route%4) {
		case 0 :
			work->se_num = SD_E_PHYFAR01 ;
			work->still_se_num = SD_E_PHYFAR05 ;
		break;
		case 1 :
			work->se_num = SD_E_PHYFAR02 ;
			work->still_se_num = SD_E_PHYFAR06 ;
		break;
		case 2 :
			work->se_num = SD_E_PHYFAR03 ;
			work->still_se_num = SD_E_PHYFAR07 ;
		break;
		case 3 :
			work->se_num = SD_E_PHYFAR04 ;
			work->still_se_num = SD_E_PHYFAR08 ;
		break;
		default :
			work->se_num = SD_E_PHYFAR01 ;
			work->still_se_num = SD_E_PHYFAR05 ;
	}

	GM_InitObject(&(work->body),model,OBJECT_FLAG );
	/*ctrlからobjectを参照*/
	GM_ConfigControlObject( &work->control, &work->body ) ;
	GM_ConfigObjectLight(&(work->body),work->lights) ;
	GM_ConfigControlMapID( &work->control ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( work->body.objs );

	/*ターゲット設定*/
	SetTarget(work);
	return 1;
}


/*******************************<Global function>********************************/
/*	名前:	void *NewCypher							*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewCypher( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
