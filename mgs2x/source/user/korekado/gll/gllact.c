/*
	gllact.c
	ゴルルゴン行動処理

	2002/04/04 Y.Korekado
	$Id: gllact.c,v 1.1.1.3 2002/11/19 11:44:11 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------*/

#include "BP_Misc.h"

extern void *NewEyeBeem(FMATRIX *world, FVECTOR *trg, int mode) ;
extern void *NewGllBlast3( FVECTOR *, int, int ,int ,int, int, int, int ) ;
extern void VR_SurrenderEnemy(void) ;
extern void *NewGllAchoo( FMATRIX *world, int mode );

/*----------------------------------------------------------------*/
#define GLL_PUNCH_DAM	(20)

#define BOMB_DAM	(32)
#define HEAD_DAM	(43)
#define REGENE_TIME	(6000)
//#define GLL_REGENE_TIME	(180)
#define ADJ_INTRPT	STEP_VMODE(4)
/*----------------------------------------------------------------*/
/* アクション */
enum {
	PAD_NONE,
	PAD_SQUAT,	/* しゃがむ */
	PAD_SLIDING,	/* 滑り込み */
	PAD_SUMMER,		/* サマーソルト */
	PAD_FAT_RUN,		/* */
	PAD_FAT_READY,		/* */
	PAD_FAT_SHOT,		/* */
	PAD_WAKEUP,		/* */

} ;
/*-------------------------------------------------------------*/
static FVECTOR RightFootVec ={ -3500.634766, 0.0f, -1068.320312 } ;
static FVECTOR LeftFootVec ={ 3415.921875, 0.0f, 1359.835938 } ;

/* 0:right punch 1:left punch */
static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;

    work = ( Work * )ptr ;
    work->attack_flag = 0 ;
	GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_GNLP_HIT ) ;
}
static int AttackTarget( Work *work, int n )
{
	static FVECTOR p_force = { 0.0F,-400.0F,0.0F } ;
	static FVECTOR p_shift = { 0.0F,-500.0F,0.0F } ;
	static FVECTOR p_size = { 1000.0F,1000.0F,1000.0F } ;
	TARGET	*off ;
	POWER_TARGET	*power ;
	FVECTOR			v, *f, *sz, *sh ;
	int				flag, obj ;
	long64			weapon_type ;
	OBJECT			*body ;

	off = &( work->attack ) ;
	power = &( work->power ) ;
	body = &work->body ;
	obj = 0 ;

	flag = TARGET_OFFENSE ;
	f = &DG_ZeroVector ;
	sz = &p_size ;
	sh = &DG_ZeroVector ;
	weapon_type = WP_NOPLAYER ;
	if ( n == 0 ) {
		obj = HUMAN21_MIGI_TE ;
		sh = &p_shift ;
		weapon_type |= WP_BLAST ;
	} else if ( n==1 ) {
		obj = HUMAN21_HIDARI_TE ;
		sh = &p_shift ;
		weapon_type |= WP_BLAST ;
	}
	GM_SetTarget( off, flag, 0, PLAYER_SIDE, sz, f ) ;
	GM_SetTargetWeaponType( off, weapon_type ) ; 	

	DG_SetPos( &( BODYWORLD(body, obj) ) ) ;
    DG_RotVector( &p_force, &v, 1 ) ;

	GM_MoveTarget3( off, &( BODYWORLD(body, obj ) ) ) ;
	GM_SetPowerTarget( off, power, POWER_ONCE, 255, 0, GLL_PUNCH_DAM, &v ) ;
    GM_SetTargetCallBack( off, Hit, work ) ;
	GM_PutTarget( off ) ;

//NewTargetView( off, 200, 34, 184 ) ;
	return 0 ;
}



static FVECTOR	Bloodforce = { 0.0, -100.0, 500.0 } ;
static FVECTOR	left_nose = { 0.0, 100.0, 1000.0 } ;
static void Blood( Work *work, int mode )
{
	extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
	FMATRIX	*world, m ;
	FVECTOR f, pos ;

	world = &work->body.objs->objs[12].world ;

	DG_SetPos( world ) ;
	DG_MovePos( &left_nose ) ;
	DG_GetPos( &m ) ;
	KR_FMatToFvec( &m, &pos ) ;

	DG_PutVector( &Bloodforce, &f, 1 );
	_sceVu0SubVector(  &f, &f, &pos ) ;
	f.vx = f.vx + (float)KR_RandS( 256 ) ;
	f.vy = f.vy + (float)KR_RandS( 256 ) ;
	f.vz = f.vz + (float)KR_RandS( 256 ) ;
	GV_SetActorChild( work, NewBlood( world, &pos, &f, mode, 0 ) ) ;
}

FVECTOR PlasmaColor = {
	32.0, 100.0, 190.0, 64.0
} ;

static void CallPlasma( NPCWORK *npc, float width, int obj_1, int obj_2, int obj_3, float scale )
{
	extern void *NewSpritePlasma2( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color, int life );
    FVECTOR	vectmp;

	_sceVu0ScaleVector( &vectmp, (FVECTOR *)npc->body->objs->objs[obj_1].world.m[2], scale );
	_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)npc->body->objs->objs[obj_1].world.m[3] );
	NewSpritePlasma2( (FVECTOR *)npc->body->objs->objs[obj_2].world.m[3],
					  &vectmp,
					  (FVECTOR *)npc->body->objs->objs[obj_3].world.m[3],
					  width, &PlasmaColor, 8 );
}

static void ChaffPlasma( NPCWORK *npc )
{
    float	width;
    int		s ;

	width = 100.0f + (float)KR_RandS( 50 ) ;

	s = KR_RandU(6) ;
printf("plasma rnad1[%d]\n",s);
	switch ( s ) {
		case 0 :
		 CallPlasma( npc, width, 14, 0, 15, 1000.0 ) ;
		 break ;
		case 1 :
		 CallPlasma( npc, width, 18, 0, 19, 1000.0 ) ;
		 break ;
		case 2 :
		 CallPlasma( npc, width, 4, 11, 6, 1000.0	 ) ;
		 break ;
		case 3 :
		 CallPlasma( npc, width, 8, 11, 10, 2000.0 ) ;
		 break ;
		case 4 :
		 CallPlasma( npc, width, 2, 0, 3, 3000.0 ) ;
		 break ;
		case 5 :
		 CallPlasma( npc, width, 2, 0, 7, -1000.0 ) ;
		 break ;
	}

	s = KR_RandU(6) ;
printf("plasma rnad2[%d]\n",s);
	switch ( s ) {
		case 0 :
		 CallPlasma( npc, width, 14, 0, 15, -1000.0 ) ;
		 break ;
		case 1 :
		 CallPlasma( npc, width, 18, 0, 19, 1000.0 ) ;
		 break ;
		case 2 :
		 CallPlasma( npc, width, 4, 12, 6, 500.0	 ) ;
		 break ;
		case 3 :
		 CallPlasma( npc, width, 8, 12, 10, -500.0 ) ;
		 break ;
		case 4 :
		 CallPlasma( npc, width, 2, 12, 13, -1000.0 ) ;
		 break ;
		case 5 :
		 CallPlasma( npc, width, 2, 12, 17, -1000.0 ) ;
		 break ;
	}

}


//static FVECTOR	MouthShift = { 0.0, -300.0, 800.0 } ;
//static SVECTOR	MouthRot = { 0, 0, 0 } ;
static FVECTOR	AchooShiftL = { -1000.0, 1000.0, -14000.0 } ;
static SVECTOR	AchooRotL = { 0, -64, 0 } ;
static SVECTOR	AchooRotL2 = { 0, -256, 0 } ;
static FVECTOR	AchooShiftR = { 1000.0, 1000.0, -14000.0 } ;
static SVECTOR	AchooRotR = { 0, 64, 0 } ;
static SVECTOR	AchooRotR2 = { 0, 256, 0 } ;
static void Achoo( Work *work )
{
	FMATRIX	mat ;

#if 1
	DG_SetPos2( &AchooShiftL, &AchooRotL ) ;
	DG_GetPos( &mat );
	NewGllAchoo( &mat, 0 );
	DG_SetPos2( &AchooShiftL, &AchooRotL2 ) ;
	DG_GetPos( &mat );
	NewGllAchoo( &mat, 0 );

	DG_SetPos2( &AchooShiftR, &AchooRotR ) ;
	DG_GetPos( &mat );
	NewGllAchoo( &mat, 0 );
	DG_SetPos2( &AchooShiftR, &AchooRotR2 ) ;
	DG_GetPos( &mat );
	NewGllAchoo( &mat, 0 );
#else
	DG_SetPos( &work->body.objs->objs[HUMAN21_ATAMA].world );
	DG_MovePos( &MouthShift );
	DG_RotatePos( &MouthRot );
	DG_GetPos( &mat );
	NewGllAchoo( &mat, 0 );
#endif
}
/*-------------------------------------------------------------*/
static void ActStandStill( NPCWORK *npc, int time ) ;
static void ActMove( NPCWORK *npc, int time ) ;
static void ActFire( NPCWORK *npc, int time ) ;
static void ActSonic( NPCWORK *npc, int time ) ;
static void ActHowl( NPCWORK *npc, int time ) ;
static void ActDetect ( NPCWORK *npc, int time ) ;
static void ActDam ( NPCWORK *npc, int time ) ;
static void ActGllDam ( NPCWORK *npc, int time ) ;
static void ActDown ( NPCWORK *npc, int time ) ;
static void ActDeath ( NPCWORK *npc, int time ) ;
static void ActChaffIn ( NPCWORK *npc, int time ) ;
static void ActChaff ( NPCWORK *npc, int time ) ;
/*-------------------------------------------------------------*/
static void ActStandStill( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, NPC_MOT_STAND ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;
	if ( npc->CheckPad( npc ) )	 return ;

	if ( act->dir >= 0 ) {
		npc->ctrl->turn.vy = GV_NearSpeedP( npc->ctrl->rot.vy, act->dir, STEP_VMODE(32) ) ;
		NPC_SetActMode( npc, ActMove ) ;
		return ;
	}

if(0){
	FVECTOR vec, pos1, pos2, pos3 ;

	npc->ctrl->turn.vy = npc->ctrl->rot.vy = 0 ;
	KR_FMatToFvec( &BODYWORLD( npc->body, 0 ), &pos1 ) ;
	KR_FMatToFvec( &BODYWORLD( npc->body, 15 ), &pos2 ) ;
	KR_FMatToFvec( &BODYWORLD( npc->body, 19 ), &pos3 ) ;
	pos1.vy = pos2.vy = pos3.vy = 0.0f ;

	_sceVu0SubVector(  &vec, &pos2, &pos1 ) ;
printf("right vec x[%f] z[%f] \n",vec.vx, vec.vz ) ;
	_sceVu0SubVector(  &vec, &pos3, &pos1 ) ;
printf("left vec x[%f] z[%f] \n",vec.vx, vec.vz ) ;
}
}

static void ActMove( NPCWORK *npc, int time )
{
	CONTROL	*ctrl ;
	NPCACT *act ;
	Work *work ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	ctrl = npc->ctrl ;

SET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_FLR_CHECK ) ;
SET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_ONLINE_CHECK ) ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE  ) ;

	if ( time == 0 ) {
//printf("act->current_mot[%d] != GLL_MOT_WALK[%d]\n",act->current_mot, GLL_MOT_WALK ) ;
		if ( act->current_mot != GLL_MOT_WALK ) {
			NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_START ) ;
		}
		work->mot_count = KR_MotionTime( &npc->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
		work->mot_count = COUNT_VMODE(work->mot_count) ;
	}

	if ( time == COUNT_VMODE(239) ) {
//printf("act->current_mot[%d] != GLL_MOT_WALK[%d]\n",act->current_mot, GLL_MOT_WALK ) ;
		if ( act->current_mot != GLL_MOT_WALK ) {
			NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_WALK ) ;
			work->mot_count = 0 ;
		}
	}

	if ( npc->CheckDamage( npc ) ) {
		return ;
	}
	if ( npc->CheckPad( npc ) ) {
		return ;
	}
	if ( act->dir < 0 ) {
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}

	{
		int p_time ;

      if ( BP_IsPAL() == TRUE )
		   p_time = work->mot_count ;
      else
		   p_time = KR_MotionTime( &npc->body->m_ctrl->mt3_ctrl[ 0 ] ) ;

      if ( (p_time == COUNT_VMODE(120)) || (p_time == COUNT_VMODE(360)) ) {
			extern void *NewShakeCameraGLL( int chanl, int intense, int time, FVECTOR *pos ) ;
			NewShakeCameraGLL( 0, 512, COUNT_VMODE(10), &npc->ctrl->mov );

			if ( work->pl_dis < work->eye_length+4000 ) {
				GLL_WalkVibration( ) ;
			}
		}
	}

	/* 進行方向 */
	ctrl->turn.vy = GV_NearSpeedP( npc->ctrl->rot.vy, act->dir, STEP_VMODE(32) ) ;
//printf("turn.vy[%d] rot.vy[%d] act.dir[%d] diff[%d]\n",ctrl->turn.vy,npc->ctrl->rot.vy, act->dir,GV_DiffDirAbs( ctrl->turn.vy, ctrl->rot.vy ) ) ;
	if ( GV_DiffDirAbs( npc->ctrl->rot.vy, act->dir ) > 64 ) {
		npc->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		ctrl->step.vx = 0.0f ;
		ctrl->step.vz = 0.0f ;
	}

	work->mot_count ++ ;

#if 0
	if ( speed != 0.0F ) {
		act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
		ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;
	}
#endif
}

extern void *NewShakeCameraGLL( int chanl, int intense, int time, FVECTOR *pos ) ;

static void ActMoveEnd( NPCWORK *npc, int time )
{
	CONTROL	*ctrl ;
	NPCACT *act ;
	int p_time ;
	Work *work ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	ctrl = npc->ctrl ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE  ) ;

	if ( time == 0 ) {
		if ( (act->current_mot != GLL_MOT_START ) && ( act->current_mot != GLL_MOT_WALK ) ) {
			NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_WALK ) ;
		}
//		NPC_SetActMotionEX( npc, LAYER_BASE,
//			npc->base_mar, NPC_MOT_STAND, MOTION_MASK_FULL, 60*TIME_BASE, (float)TIME_BASE ) ;
		work->mot_count = KR_MotionTime( &npc->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
		work->mot_count = COUNT_VMODE(work->mot_count) ;
	}

   if ( BP_IsPAL()==TRUE )
	   p_time = work->mot_count ;
   else
	   p_time = KR_MotionTime( &npc->body->m_ctrl->mt3_ctrl[ 0 ] ) ;


	if ( npc->CheckDamage( npc ) ) {
		return ;
	}
	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActStandStill ) ;
		}
		return ;
	}

	if ( act->current_mot == GLL_MOT_START ) {
		if ( p_time == COUNT_VMODE(238) ) {
			NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_END ) ;
			work->mot_count = 0 ;	/* モーションカウント */
		}
		if ( p_time == COUNT_VMODE(120) ) {
			NewShakeCameraGLL( 0, 512, COUNT_VMODE(10), &npc->ctrl->mov );
			if ( work->pl_dis < work->eye_length+4000 ) {
				GLL_WalkVibration( ) ;
			}
		}
	}

//printf("end mot[%d] p_time [%d] \n",act->current_mot, p_time ) ;
	if ( act->current_mot == GLL_MOT_WALK ) {
		if ( p_time == COUNT_VMODE(241) ) {
			NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_END_L ) ;
			work->mot_count = 0 ;	/* モーションカウント */
		}
		if ( p_time == COUNT_VMODE(1) ) {
			NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_END ) ;
			work->mot_count = 0 ;	/* モーションカウント */
		}
		if ( (p_time == COUNT_VMODE(120)) || (p_time == COUNT_VMODE(360)) ) {
			NewShakeCameraGLL( 0, 512, COUNT_VMODE(10), &npc->ctrl->mov );
			if ( work->pl_dis < work->eye_length+4000 ) {
				GLL_WalkVibration( ) ;
			}
		}
		if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
			work->mot_count = 0 ;	/* モーションカウント */
		}
	}
	if ( (act->current_mot == GLL_MOT_END) || (act->current_mot == GLL_MOT_END_L) ) {
		int dir, flag_x, flag_z ;
		FVECTOR	vec, foot_pos, trg_pos ;
		float	step_x, step_z ;

		if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
//printf("end trg[%f][%f] ctrl[%f][%f]\n",npc->nvtrg->pos.vx,npc->nvtrg->pos.vz,npc->ctrl->mov.vx,npc->ctrl->mov.vz ) ;
			act->act_end = 1 ;

			npc->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
			ctrl->step.vx = 0.0f ;
			ctrl->step.vz = 0.0f ;
			ctrl->mov.vx = npc->nvtrg->pos.vx ;
			ctrl->mov.vz = npc->nvtrg->pos.vz ;

			NPC_SetActMode( npc, ActStandStill ) ;
			return ;
		}
		if ( (p_time == COUNT_VMODE(120)) ) {
			NewShakeCameraGLL( 0, 512, COUNT_VMODE(10), &npc->ctrl->mov );
			if ( work->pl_dis < work->eye_length+4000 ) {
				GLL_WalkVibration( ) ;
			}
		}

		dir = npc->ctrl->rot.vy ;
		if ( act->current_mot == GLL_MOT_END_L ) {
			trg_pos.vx = (RightFootVec.vx * _RcosF( dir )) + (RightFootVec.vz * _RsinF( dir )) ;
			trg_pos.vz = -(RightFootVec.vx * _RsinF( dir )) + (RightFootVec.vz * _RcosF( dir )) ;
			KR_FMatToFvec( &BODYWORLD( npc->body, HUMAN21_MIGI_KAKATO ), &foot_pos ) ;
		} else {
			trg_pos.vx = (LeftFootVec.vx * _RcosF( dir )) + (LeftFootVec.vz * _RsinF( dir )) ;
			trg_pos.vz = - (LeftFootVec.vx * _RsinF( dir )) + (LeftFootVec.vz * _RcosF( dir )) ;
			KR_FMatToFvec( &BODYWORLD( npc->body, HUMAN21_HIDARI_KAKATO ), &foot_pos ) ;
		}

		_sceVu0AddVector(  &trg_pos, &trg_pos, &npc->nvtrg->pos ) ;
		_sceVu0SubVector(  &vec, &trg_pos, &foot_pos ) ;
//printf(" trg x[%f] foot x[%f] vec x[%f]\n",trg_pos.vx, foot_pos.vx, vec.vx );
//printf(" trg z[%f] foot z[%f] vec z[%f]\n",trg_pos.vz, foot_pos.vz, vec.vz );
		flag_x = flag_z = 0 ;
		if ( vec.vx > 16.0f ){ vec.vx = 16.0f ; flag_x = 1 ; }
		if ( vec.vx < -16.0f ){ vec.vx = -16.0f ; flag_x = 1 ; }
		if ( vec.vz > 16.0f ){ vec.vz = 16.0f ; flag_z = 1 ; }
		if ( vec.vz < -16.0f ){ vec.vz = -16.0f ; flag_z = 1 ; }

		vec.vx = STEP_VMODE(vec.vx) ;
		vec.vz = STEP_VMODE(vec.vz) ;

		npc->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
//printf(" motion step x[%f] z[%f] \n",npc->body->m_ctrl->step.vx, npc->body->m_ctrl->step.vz );
		
		step_x = npc->body->m_ctrl->step.vx*10.0 ;
		step_z = npc->body->m_ctrl->step.vz*10.0 ;
			
		if ( flag_x ) {
			if ( vec.vx > 0.0 ) {
				ctrl->step.vx = ( step_x > 0.0 ) ? (step_x+vec.vx) : vec.vx ;
			} else {
				ctrl->step.vx = ( step_x <= 0.0 ) ? (step_x+vec.vx) : vec.vx ;
			}
		} else {
//			ctrl->step.vx = vec.vx ;
			ctrl->step.vx = 0.0 ;
		}
		if ( flag_z ) {
			if ( vec.vz > 0.0 ) {
				ctrl->step.vz = ( step_z > 0.0 ) ? (step_z+vec.vz) : vec.vz ;
			} else {
				ctrl->step.vz = ( step_z <= 0.0 ) ? (step_z+vec.vz) : vec.vz ;
			}
		} else {	/* 差が16.0以下なら到着 */
//			ctrl->step.vz = vec.vz ;
			ctrl->step.vz = 0.0 ;
		}
		
//printf(" ctrl step x[%f] z[%f] \n",ctrl->step.vx, ctrl->step.vz );
	}

	work->mot_count ++ ;	/* モーションカウント */
	/* 進行方向 */
	ctrl->turn.vy = ctrl->rot.vy = GV_NearSpeedP( npc->ctrl->rot.vy, act->dir, STEP_VMODE(8) ) ;
//printf("turn.vy[%d] rot.vy[%d] act.dir[%d] diff[%d]\n",ctrl->turn.vy,npc->ctrl->rot.vy, act->dir,GV_DiffDirAbs( ctrl->turn.vy, ctrl->rot.vy ) ) ;
}

static void ActHowl( NPCWORK *npc, int time )
{
	NPCACT *act ;
	Work *work ;

	work = (Work *)npc->character ;
	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		if ( work->pl_dis < work->eye_length ) {
//			NewPadVibration2( 2695146 /*gol_howl*/, 0 ) ;
		}
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActStandStill ) ;
		}
		return ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}


	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

#define	GLL_MOT_INTERP_M		60*1 /* 1/300単位 */
static int	MotionTotalTime( MT3_CONTROL *mt3_ctrl )
{
	float left ;

	left = mt3_ctrl->motion_total_time  / mt3_ctrl->motion_time_base ;

	return (int)left ;
}

static int ReturnMotTime( int mot )
{
	switch( mot ) {
		case GLL_MOT_LOOK_L_12 :
		 return COUNT_VMODE(190) ;	/*257*/
		case GLL_MOT_LOOK_R_12 :
		 return COUNT_VMODE(190) ;	/*257*/
		case GLL_MOT_LOOK_L_7 :
		 return COUNT_VMODE(190) ;	/*257 */
		case GLL_MOT_LOOK_R_7 :
		 return COUNT_VMODE(190) ;
		case GLL_MOT_LOOK_F_12 :
		 return COUNT_VMODE(190) ;
		case GLL_MOT_LOOK_F_16 :
		 return COUNT_VMODE(190) ;
		case GLL_MOT_LOOK_F_7 :
		 return COUNT_VMODE(190) ;
	}
	
	return -1 ;
}

static int LoopMotTime( int mot )
{
	switch( mot ) {
		case GLL_MOT_LOOK_L_12 :
		 return COUNT_VMODE(166) ;	/*257*/
		case GLL_MOT_LOOK_R_12 :
		 return COUNT_VMODE(166) ;	/*257*/
		case GLL_MOT_LOOK_L_7 :
		 return COUNT_VMODE(166) ;	/*257 */
		case GLL_MOT_LOOK_R_7 :
		 return COUNT_VMODE(166) ;
		case GLL_MOT_LOOK_F_12 :
		 return COUNT_VMODE(166) ;
		case GLL_MOT_LOOK_F_16 :
		 return COUNT_VMODE(166) ;
		case GLL_MOT_LOOK_F_7 :
		 return COUNT_VMODE(166) ;
	}
	
	return -1 ;
}

static void ActLook( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		/*戻りモーション開始の時間 0以下ならエンドまで*/
		act->tmp_time = npc->rnavi->p_acttime - ReturnMotTime( act->current_mot ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActStandStill ) ;
		}
		return ;
	}

	if ( act->tmp_time > 0 ) {
		int r_time, m_time, total, l_time ;
		MOTION_CONTROL *m_ctrl ;

		m_ctrl = npc->body->m_ctrl ;
//printf("act->tmp_time [%d] time[%d]\n",act->tmp_time, time ) ;
		if ( time == act->tmp_time ) {

			total = MotionTotalTime( &m_ctrl->mt3_ctrl[ 0 ] ) ;
			r_time = total - ReturnMotTime( act->current_mot ) ;
			MT_SetMotionInterp( m_ctrl, GLL_MOT_INTERP_M, MOTION_MASK_FULL ) ;
			MT_SetMotionData( m_ctrl, 0, act->set_mot, r_time*TIME_BASE, 0 ) ;
		} else if ( time < act->tmp_time ) {/*Loop*/
			m_time = KR_MotionTime( &m_ctrl->mt3_ctrl[ 0 ] ) ;
			total = MotionTotalTime( &m_ctrl->mt3_ctrl[ 0 ] ) ;
			r_time = total - ReturnMotTime( act->current_mot ) ;
			if ( m_time >= r_time ) {
				l_time = LoopMotTime( act->current_mot ) ;
				MT_SetMotionInterp( m_ctrl, GLL_MOT_INTERP_M, MOTION_MASK_FULL ) ;
				MT_SetMotionData( m_ctrl, 0, act->set_mot, l_time*TIME_BASE, 0 ) ;
			}
		}
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}

//printf("act LOOOOK act->dir[%d]  time[%d]\n", act->dir, time ) ;
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = GV_NearSpeedP( npc->ctrl->rot.vy, act->dir, STEP_VMODE(32) ) ;
}

static void ActFire( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT *act ;

	work = (Work *)npc->character ;
	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		{
			FVECTOR trg ;
			
			trg.vx = GM_PlayerPosition.vx ;
			trg.vy = GM_PlayerPosition.vy - 1000.0 ;
			trg.vz = GM_PlayerPosition.vz ;
			NewEyeBeem(&(npc->body->objs->objs[12].world), &trg, 0) ;
		}
	}

	if ( time == COUNT_VMODE(60) ) {
		if ( StatusFightGME( work->status ) ) {
			GM_SeSetMode( SD_E_MGNHOWL1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else if ( StatusFightGLL( work->status ) ) {
			GM_SeSetMode( SD_E_GLLHOWL1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_E_GLLHOWL12, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
	}

	if ( npc->CheckDamage( npc ) ) {
		return ;
	}

	if ( npc->CheckPad( npc ) ) {
		return ;
	}

	if ( time > COUNT_VMODE( 120 ) ) {
//	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}
}

static void ActFireF( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT *act ;

	work = (Work *)npc->character ;
	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NewEyeBeem(&(npc->body->objs->objs[12].world), &work->tmp_trgpos, 0) ;
	}

	if ( time == COUNT_VMODE(60) ) {
		if ( StatusFightGME( work->status ) ) {
			GM_SeSetMode( SD_E_MGNHOWL1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else if ( StatusFightGLL( work->status ) ) {
			GM_SeSetMode( SD_E_GLLHOWL1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_E_GLLHOWL12, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
	}

	if ( npc->CheckDamage( npc ) ) {
		return ;
	}

	if ( npc->CheckPad( npc ) ) {
		return ;
	}

//	if ( time > COUNT_VMODE( 180 ) ) {
	if ( time > COUNT_VMODE( 120 ) ) {
//	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}
}

static void ActSonic( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		GM_SeSetMode( SD_E_GLLHOWL1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( (time == COUNT_VMODE(10)) || (time == COUNT_VMODE(15) ) || (time == COUNT_VMODE(20)) ) {
		extern void *NewSonic(FMATRIX *world) ;
		NewSonic(&(npc->body->objs->objs[12].world)) ;
	}

	if ( npc->CheckDamage( npc ) ) {
		return ;
	}

	if ( npc->CheckPad( npc ) ) {
		return ;
	}

//	if ( time > COUNT_VMODE( 180 ) ) {
	if ( time > COUNT_VMODE( 120 ) ) {
//	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}
}

static void ActPunch( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT *act ;
	MOTION_CONTROL *m_ctrl ;
	int	m_time ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	m_ctrl = npc->body->m_ctrl ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		if ( act->current_mot == GLL_MOT_PUNCH_R ) {
			if ( work->gunhand == 0 ) ChangeGun ( work, 1 ) ;
		} else {
			if ( work->gunhand == 1 ) ChangeGun ( work, 0 ) ;
		}
		GM_SeSetMode( SD_E_GLLBRESS2, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*3.0f ) ;
		work->attack_flag = 1 ;
	}
	m_time = KR_MotionTime( &m_ctrl->mt3_ctrl[ 0 ] ) ;

	if ( work->attack_flag ) {
		if ( act->current_mot == GLL_MOT_PUNCH_R ) {
			AttackTarget( work, 0 ) ;
		} else {
			AttackTarget( work, 1 ) ;
		}
	}

	if ( m_time == COUNT_VMODE(60) ) {
//		GM_SeSetMode( SD_E_GLLHOWL12, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( npc->CheckDamage( npc ) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		work->head_flag = 0 ;
		if ( work->gunhand == 1 ) ChangeGun ( work, 0 ) ;
		return ;
	}

	if ( npc->CheckPad( npc ) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		work->head_flag = 0 ;
		if ( work->gunhand == 1 ) ChangeGun ( work, 0 ) ;
		return ;
	}

	if ( time == COUNT_VMODE(145 / 3) ) {
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_GNLPUNCH ) ;
	}

	if ( m_time > COUNT_VMODE(280) ) {
		work->head_flag = 0 ;
	} else if ( m_time > COUNT_VMODE(145) ) {
		work->head_flag = 1 ;
	}

	if ( m_time > COUNT_VMODE(260) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*4.0f ) ;
	} else if ( m_time > COUNT_VMODE(160) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*8.0f ) ;
	}

//printf("punch m_time[%d] time[%d]\n",m_time, time ) ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		work->head_flag = 0 ;
		NPC_SetActMode( npc, ActStandStill ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		if ( work->gunhand == 1 ) ChangeGun ( work, 0 ) ;
		return ;
	}
}

static void ActDetect ( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
#if 0
		NPC_SetActMotion( npc, npc->base_mar, NPC_MOT_STAND ) ;
#endif
		MT_SetMotionSpeed( npc->body->m_ctrl, 0.0f ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;
	if ( npc->CheckPad( npc ) ) return ;

	if ( time == COUNT_VMODE(60) ) {
#if 0
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.25f ) ;
#endif
	}

	if ( time > COUNT_VMODE(180) ) {
		act->act_end = 1 ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}

	/* 進行方向 */
	if ( time > COUNT_VMODE(60) ) {
		npc->ctrl->turn.vy = GV_NearSpeedP( npc->ctrl->rot.vy, act->dir, STEP_VMODE(12) ) ;
	}
}

static void ActRegene ( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT	*act ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_HOWL ) ;
//		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_STAND ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*1.0 ) ;
		act->tmp_time = 0 ;
	}

	if ( npc->CheckDamage( npc ) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}

	if ( work->damaged & GLL_DAM_HIT_HEART ) {
		act->tmp_time += COUNT_VMODE(18) ;
	}

	{
		MOTION_CONTROL *m_ctrl ;

		m_ctrl = npc->body->m_ctrl ;
		if ( act->tmp_time > 0 ) act->tmp_time -- ;
		MT_SetMotionData( m_ctrl, 0, GLL_MOT_HOWL, act->tmp_time, 0 ) ;

		npc->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		npc->ctrl->step.vx = 0.0f ;
		npc->ctrl->step.vz = 0.0f ;
	}

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActStandStill ) ;
		}
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}
}

static void ActChaffDam ( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT	*act ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_STAND ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*1.0 ) ;
		act->tmp_time = 0 ;
	}

	if ( npc->CheckDamage( npc ) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}

	if ( !(time%COUNT_VMODE(8)) ) {
		act->tmp_time += COUNT_VMODE(128) ;
	}
	if ( !(time%COUNT_VMODE(8)) ) {
		GM_SeSetMode( SD_A_MG_INCHA, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( !(time % COUNT_VMODE(32)) ) {
		ChaffPlasma( npc ) ;
	}

	if ( !(time % COUNT_VMODE(8)) ) {
		if ( KR_RandU(2) ) {
			GM_SeSetMode( SD_A_MG_INCHA, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
	}

	{
		MOTION_CONTROL *m_ctrl ;

		m_ctrl = npc->body->m_ctrl ;
		if ( act->tmp_time > 0 ) act->tmp_time -= COUNT_VMODE(8) ;
		MT_SetMotionData( m_ctrl, 0, GLL_MOT_STAND, act->tmp_time, 0 ) ;

		npc->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		npc->ctrl->step.vx = 0.0f ;
		npc->ctrl->step.vz = 0.0f ;
	}

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActStandStill ) ;
		}
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}
}

static void ActChaffBeem ( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT	*act ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NewEyeBeem(&(npc->body->objs->objs[12].world), &work->tmp_trgpos, 0) ;
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_FIRE ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*1.0 ) ;
		act->tmp_time = 0 ;
	}

	if ( npc->CheckDamage( npc ) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}
	if ( npc->CheckPad( npc ) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}

	if ( !(time%COUNT_VMODE(8)) ) {
		act->tmp_time += COUNT_VMODE(16)*TIME_BASE ;
	}
	if ( !(time%COUNT_VMODE(8)) ) {
		GM_SeSetMode( SD_A_MG_INCHA, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( !(time % COUNT_VMODE(32)) ) {
		ChaffPlasma( npc ) ;
	}

	if ( !(time % COUNT_VMODE(8)) ) {
		if ( KR_RandU(2) ) {
			GM_SeSetMode( SD_A_MG_INCHA, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
	}

	{
		MOTION_CONTROL *m_ctrl ;

		m_ctrl = npc->body->m_ctrl ;
		if ( act->tmp_time > 0 ) act->tmp_time -= TIME_BASE ;
		MT_SetMotionData( m_ctrl, 0, GLL_MOT_FIRE, act->tmp_time, 0 ) ;

		npc->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		npc->ctrl->step.vx = 0.0f ;
		npc->ctrl->step.vz = 0.0f ;
	}

//	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	if ( time >= COUNT_VMODE(300) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}
}

static void ActBdmh ( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT	*act ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_BDMH_START ) ;
	}

	if ( npc->CheckDamage( npc ) ) {
		return ;
	}

	if ( npc->CheckPad( npc ) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}

	if ( act->current_mot == GLL_MOT_BDMH_START ) {
		if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
			NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_BDMH_LOOP ) ;
		}
	} else if ( act->current_mot == GLL_MOT_BDMH_LOOP ) {
		if ( time > COUNT_VMODE(360) ) {
			NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_BDMH_END ) ;
		}
	} else if ( act->current_mot == GLL_MOT_BDMH_END ) {
		if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
			act->act_end = 1 ;
			NPC_SetActMode( npc, ActStandStill ) ;
			return ;
		}
	}
}

static void ActBook( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT *act ;
	int i ;

	work = (Work *)npc->character ;

	SET_FLAG ( work->status2, GLL_STATUS2_NO_TRG ) ;
	SET_FLAG ( work->status2, GLL_STATUS2_NO_ZLIMIT ) ;
	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_STAND ) ;
		for( i=0; i<GLL_MAX_HEART ; i++ ) {
			SET_FLAG( work->gllheart[i], GLL_BODY_SPCLEAR) ;
		}
	}

	if ( time == COUNT_VMODE(60) ) {
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), WHT_QE_GENORA );
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_BOOK ) ;
		NewPadVibration2( 13940909 /*gol_collapse*/, 0 ) ;
	}

	if ( (time == COUNT_VMODE(1135+60)) || (time == COUNT_VMODE(1263+60)) || 
		 (time == COUNT_VMODE(1390+60)) || (time == COUNT_VMODE(1885+60)) ) {
		NewShakeCameraGLL( 0, 220, 10, &DG_ZeroVector );
	}


	if ( time < COUNT_VMODE(60) )  {
		npc->ctrl->turn.vy = GV_NearSpeedP( npc->ctrl->rot.vy, act->dir, STEP_VMODE(64) ) ;
//printf("npc->ctrl->turn.vy[%d] act->dir[%d]\n",npc->ctrl->turn.vy, act->dir ) ;
	}

	if ( time == COUNT_VMODE(70+60) ) {
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), RED_AT_GENORA );
		GLL_DetectVibration( ) ;
	}


//	if( time == 1595 ) {
	if( time == COUNT_VMODE(1655) ) {
		GM_SeSetMode( SD_A_GLLHANAD, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( time >= COUNT_VMODE(1595+60) && time < COUNT_VMODE(1600+60) ) {
		Blood( work, 1 ) ;
	}

	if ( time >= COUNT_VMODE(1650+60) && time < COUNT_VMODE(2000+60) ) {
		if ( !(time%COUNT_VMODE(6)) ) Blood( work, 1 ) ;
	}

	if ( time >= COUNT_VMODE(1200+60) ) {
		npc->ctrl->turn.vy &= 4095 ;
		if ( npc->ctrl->turn.vy < 2048 ) {
			if ( npc->ctrl->turn.vy > 256 ) npc->ctrl->turn.vy -- ;
		}  else {
			if ( npc->ctrl->turn.vy < 3840 ) npc->ctrl->turn.vy ++ ;
		} 
	}

	if ( time > COUNT_VMODE(60) ) {
		if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
			MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.0f ) ;
			if ( work->end_proc ) {
				GCL_ExecProc( work->end_proc, NULL ) ;
				work->end_proc = 0 ;
			}
			return ;
		}
	}

}

static void ActMyFriend( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT *act ;
	int i ;

	work = (Work *)npc->character ;

	SET_FLAG ( work->status2, GLL_STATUS2_NO_TRG ) ;
	SET_FLAG ( work->status2, GLL_STATUS2_NO_ZLIMIT ) ;
	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		for( i=0; i<GLL_MAX_HEART ; i++ ) {
			SET_FLAG( work->gllheart[i], GLL_BODY_SPCLEAR) ;
		}
	}
	if ( time == COUNT_VMODE(60) ) {
		GM_SeSetMode( SD_E_GLLBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
//		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), WHT_AT_GENORA );
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), RED_AT_GENORA );
		GLL_DetectVibration( ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.0f ) ;
	}

	if ( time == COUNT_VMODE(480) ) {
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), HEART_GENORA );
//		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), RED_AT_GENORA );
	}

	/* 保険 */
	if ( time > COUNT_VMODE(760) ) {
		NPC_SetActMode( npc, ActStandStill ) ;
		return ;
	}

}

static void ActAchoo( NPCWORK *npc, int time )
{
	MOTION_CONTROL *m_ctrl ;
	NPCACT	*act ;
	Work	*work ;
	int		m_time, mot_flame ;

	act = &npc->action ;
	work = (Work *)npc->character ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		act->tmp_time = 0 ;
		work->mot_reverse = 0 ;	/* 順再生 */
		work->mot_count = 0 ;	/* モーションカウント */
	}

	if ( npc->CheckDamage( npc ) ) {
		UNSET_FLAG( GLL_GAME_STATUS, GLL_GS_ACHOO_TIME ) ;
		return ;
	}

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, ActStandStill ) ;
		}
		UNSET_FLAG( GLL_GAME_STATUS, GLL_GS_ACHOO_TIME ) ;
		return ;
	}

	m_ctrl = npc->body->m_ctrl ;

   if ( BP_IsPAL()==TRUE )
   	m_time = work->mot_count ;
   else
	   m_time = KR_MotionTime( &m_ctrl->mt3_ctrl[ 0 ] ) ;

//printf("reverse [%d] m_time [%d] achoo[%d] \n",work->mot_reverse, m_time, GLL_ACHOO_TIME ) ;
//printf("reverse [%d] m_time [%d] mot time[%d] \n",work->mot_reverse, m_time, KR_MotionTime( &m_ctrl->mt3_ctrl[ 0 ] ) ) ;

	if ( (m_time > COUNT_VMODE(167)) && (m_time < COUNT_VMODE(233)) ) {
		SET_FLAG( GLL_GAME_STATUS, GLL_GS_ACHOO_TIME ) ;
	} else {
		UNSET_FLAG( GLL_GAME_STATUS, GLL_GS_ACHOO_TIME ) ;
	}

	if ( !(work->mot_reverse) ) {
		if ( m_time == COUNT_VMODE(30) ) {	/*１段階目 */
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_GNLHAT_1 ) ;
		}
		if ( m_time == COUNT_VMODE(130) ) {	/*２段階目 */
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_GNLHAT_2 ) ;
		}

		if ( m_time == COUNT_VMODE(70) ) {	/*１段階目 */
			if ( (GLL_ACHOO_TIME < COUNT_VMODE(300)) ) {
				work->mot_reverse = 1 ;
			}
		}
		if ( m_time == COUNT_VMODE(170) ) {	/*２段階目 */
			if ( (GLL_ACHOO_TIME < COUNT_VMODE(500)) || (GLL_GAME_STATUS & GLL_GS_TUB_DORP) ) {
				work->mot_reverse = 1 ;
			} else {
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_GNLHAT_2 ) ;
				if ( GV_Time % 2 ) {/*隠し味にランダム*/
					GLL_ACHOO_TIME -= COUNT_VMODE(450) ;
				}
				GLL_AchooVibration();
			}
		}

		if ( m_time == COUNT_VMODE(200) ) {/* 発射 */
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_GNLHACTI ) ;
		}
		if ( m_time == COUNT_VMODE(230) ) {/* 発射 */
			SET_FLAG( GLL_GAME_STATUS, GLL_GS_ACHOO ) ;
		}
		if ( m_time > COUNT_VMODE(210) && m_time < COUNT_VMODE(270) ) {/* 発射 */
			Achoo( work ) ;
		}
	} else {/* 逆再生中 */
		if ( m_time == COUNT_VMODE(10) ) {	/*１段階目 */
			if ( (GLL_ACHOO_TIME > 0) ) {
				work->mot_reverse = 0 ;
			}
		}
		if ( m_time == COUNT_VMODE(110) ) {	/*２段階目 */
			if ( (GLL_ACHOO_TIME > COUNT_VMODE(200)) ) {
				work->mot_reverse = 0 ;
			}
		}
	}

	if ( work->mot_reverse ) {
		mot_flame = (m_time-3) * TIME_BASE ;
		MT_SetMotionData( m_ctrl, 0, GLL_MOT_SNEEZE, mot_flame, 0 ) ;
		if ( mot_flame == 0  ){
			act->act_end = 1 ;
			NPC_SetActMode( npc, ActStandStill ) ;
			UNSET_FLAG( GLL_GAME_STATUS, GLL_GS_ACHOO_TIME ) ;
			return ;
		}
		work->mot_count -- ;
	} else {
		if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
			if ( (GLL_ACHOO_TIME > COUNT_VMODE(500)) ) {
				mot_flame = COUNT_VMODE(160) * TIME_BASE ;
				MT_SetMotionInterp( m_ctrl, GLL_MOT_INTERP_M, MOTION_MASK_FULL ) ;
				MT_SetMotionData( m_ctrl, 0, GLL_MOT_SNEEZE, mot_flame, 0 ) ;
				work->mot_count = COUNT_VMODE(160) ;
			} else if ( (GLL_ACHOO_TIME > COUNT_VMODE(200)) ) {
				mot_flame = COUNT_VMODE(110) * TIME_BASE ;
				MT_SetMotionInterp( m_ctrl, GLL_MOT_INTERP_M, MOTION_MASK_FULL ) ;
				MT_SetMotionData( m_ctrl, 0, GLL_MOT_SNEEZE, mot_flame, 0 ) ;
				work->mot_count = COUNT_VMODE(110) ;
			} else {
				act->act_end = 1 ;
				NPC_SetActMode( npc, ActStandStill ) ;
				return ;
			}
		}
		work->mot_count ++ ;
	}

	npc->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	npc->ctrl->step.vx = 0.0f ;
	npc->ctrl->step.vz = 0.0f ;

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

/*-------------------------------------------------------------*/
static int	GllCheckPad( NPCWORK *npc )
{
	NPCACT	*act ;

	act = &npc->action ;
	if ( act->pad == PAD_NONE ) return 0 ;

	switch ( act->pad ) {
		case SP_HOWL :
			NPC_SetModeFromPad( npc, ActHowl, npc->base_mar, GLL_MOT_HOWL, act->pad ) ;
		break ;
		case SP_DETECT :
			NPC_SetModeFromPad( npc, ActDetect, npc->base_mar, GLL_MOT_STAND, act->pad ) ;
		break ;
		case SP_LOOK_L_12 :
			NPC_SetModeFromPad( npc, ActLook, npc->base_mar, GLL_MOT_LOOK_L_12, act->pad ) ;
		break ;
		case SP_LOOK_R_12 :
			NPC_SetModeFromPad( npc, ActLook, npc->base_mar, GLL_MOT_LOOK_R_12, act->pad ) ;
		break ;
		case SP_LOOK_L_7 :
			NPC_SetModeFromPad( npc, ActLook, npc->base_mar, GLL_MOT_LOOK_L_7, act->pad ) ;
		break ;
		case SP_LOOK_R_7 :
			NPC_SetModeFromPad( npc, ActLook, npc->base_mar, GLL_MOT_LOOK_R_7, act->pad ) ;
		break ;
		case SP_LOOK_F_12 :
			NPC_SetModeFromPad( npc, ActLook, npc->base_mar, GLL_MOT_LOOK_F_12, act->pad ) ;
		break ;
		case SP_LOOK_F_16 :
			NPC_SetModeFromPad( npc, ActLook, npc->base_mar, GLL_MOT_LOOK_F_16, act->pad ) ;
		break ;
		case SP_LOOK_F_7 :
			NPC_SetModeFromPad( npc, ActLook, npc->base_mar, GLL_MOT_LOOK_F_7, act->pad ) ;
		break ;
		case SP_MOVE_END :
			NPC_SetModeFromPad( npc, ActMoveEnd, npc->base_mar, NPC_MOT_STAND, act->pad ) ;
		break ;
		case SP_FIRE :
			NPC_SetModeFromPad( npc, ActFire, npc->base_mar, GLL_MOT_FIRE, act->pad ) ;
		break ;
		case SP_FIRE_F :
			NPC_SetModeFromPad( npc, ActFireF, npc->base_mar, GLL_MOT_FIRE, act->pad ) ;
		break ;
		case SP_PUNCH_R :
			NPC_SetModeFromPad( npc, ActPunch, npc->base_mar, GLL_MOT_PUNCH_R, act->pad ) ;
		break ;
		case SP_PUNCH_L :
			NPC_SetModeFromPad( npc, ActPunch, npc->base_mar, GLL_MOT_PUNCH_L, act->pad ) ;
		break ;
		case SP_SONIC :
			NPC_SetModeFromPad( npc, ActSonic, npc->base_mar, GLL_MOT_STAND, act->pad ) ;
		break ;
		case SP_REGENE :
			NPC_SetModeFromPad( npc, ActRegene, npc->base_mar, GLL_MOT_STAND, act->pad ) ;
		break ;
		case SP_BDMH :
			NPC_SetModeFromPad( npc, ActBdmh, npc->base_mar, GLL_MOT_STAND, act->pad ) ;
		break ;
		case SP_BOOK :
			NPC_SetModeFromPad( npc, ActBook, npc->base_mar, GLL_MOT_BOOK, act->pad ) ;
		break ;
		case SP_MYFRIEND :
			NPC_SetModeFromPad( npc, ActMyFriend, npc->base_mar, GLL_MOT_STAND, act->pad ) ;
		break ;
		case SP_CHAFFDAM :
			NPC_SetModeFromPad( npc, ActChaffDam, npc->base_mar, GLL_MOT_STAND, act->pad ) ;
		break ;
		case SP_CHAFFBEEM :
			NPC_SetModeFromPad( npc, ActChaffBeem, npc->base_mar, GLL_MOT_STAND, act->pad ) ;
		break ;
		case SP_ACHOO :
			NPC_SetModeFromPad( npc, ActAchoo, npc->base_mar, GLL_MOT_SNEEZE, act->pad ) ;
		break ;
	}
	return 1 ;
}

/*----- ダメージアクション -----------------------------------------------------*/
#define SMP_PUNCH_DAM (1)
#define SMP_KICK_DAM (3)
#define SMP_THROW_DAM (5)


static void ActDam ( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT *act ;

	work = (Work *)npc->character ;
	act = &npc->action ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_DAM ) ;
//		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.25f ) ;
		if ( StatusFightGME( work->status ) ) {
			GM_SeSetMode( SD_E_MGNBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else if ( StatusFightGLL( work->status ) ) {
			GM_SeSetMode( SD_E_GLLBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_E_GLLBRESS2, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
	}

#if 0
	if ( npc->CheckDamage( npc ) ) {
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}
#endif

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_ActStatus( act, GLL_ACT_STATUS_TRGTIME_CLEAR  ) ;
		NPC_SetActMode( npc, ActStandStill ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}

}

static void ActGllDam ( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT *act ;

	work = (Work *)npc->character ;
	act = &npc->action ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_DAM ) ;
//		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.25f ) ;
		if ( StatusFightGME( work->status ) ) {
			GM_SeSetMode( SD_E_MGNBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else if ( StatusFightGLL( work->status ) ) {
			GM_SeSetMode( SD_E_GLLBRESS, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_E_GLLBRESS2, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, ActStandStill ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE ) ;
		return ;
	}

}

static void ActChaffIn ( NPCWORK *npc, int time )
{
	extern void *NewBlastChaff( FMATRIX *world ) ;
	Work *work ;
	NPCACT	*act ;
	GM_BOMB *b ;
//	int	next_flag ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF|NPC_ACT_STATUS_DAMAGE|GLL_ACT_STATUS_CAHFF_DAMAGE ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_STAND ) ;
		if ( work->clear_proc ) {
printf("gme special game clear proc call!!" ) ;
			GCL_ExecProc( work->clear_proc, NULL ) ;
			work->clear_proc = 0 ;
		}
	}

#if 1
	if ( time >= COUNT_VMODE( 60*6 ) ) {
		NewBlastChaff( &BODYWORLD( npc->body, HUMAN21_MUNE ) ) ;
		NPC_SetActMode( npc, ActChaff ) ;
		return ;
	}

    for( b=GM_BombList.next ; b!=NULL ; b=b->next ) {
		if ( b->flag & GM_BMB_FLAG_MGN_IN ) {
			if ( time > COUNT_VMODE( 15) ){
				KR_FMatToFvec( &BODYWORLD( npc->body, HUMAN21_MUNE ), b->mov ) ;
			}
		}
	}
#else
	next_flag = 1 ;

    for( b=GM_BombList.next ; b!=NULL ; b=b->next ) {
		if ( b->flag & GM_BMB_FLAG_MGN_IN ) {
			next_flag = 0 ;
//AN_Test_Eye2(b->mov,2);
			if ( b->flag & GM_BMB_FLAG_BLAST ) {
				next_flag = 1 ;
				break ;
			}
			if ( time > COUNT_VMODE( 15) ){
				KR_FMatToFvec( &BODYWORLD( npc->body, HUMAN21_MUNE ), b->mov ) ;
			}
			if ( time == COUNT_VMODE( 60*6 ) ){
				UNSET_FLAG( b->flag, GM_BMB_FLAG_NOBLAST ) ;
				SET_FLAG( b->flag, GM_BMB_FLAG_BLAST ) ;
			}
		}
	}

	if ( next_flag ) {
		NPC_SetActMode( npc, ActChaff ) ;
		return ;
	}
#endif
}

static int	BodyBombList[21] = {
	16, 20, 15, 19, 14, 18, 13, 17, 0, 6, 1, 10, 5, 9, 4, 2, 8, 3, 7, 11, 12
} ;
static void ActChaff ( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF|NPC_ACT_STATUS_DAMAGE|GLL_ACT_STATUS_CAHFF_DAMAGE ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_DAM_CHAF ) ;
		act->tmp_time = 0 ;
		SET_FLAG ( GLL_GAME_STATUS, GLL_GS_CHAFF_CLEAR ) ;
	}

	if ( time < COUNT_VMODE( 24 ) ) {
	} else if ( (time < COUNT_VMODE( 220 )) ) {
		if ( time > act->tmp_time ) {
			//チャフ苦しみ(4,8,12,16四択ﾗﾝﾀﾞﾑ)
			GM_SeSetMode( SD_A_MG_INCHA, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
			act->tmp_time = COUNT_VMODE( time + 4*(KR_RandU(4)+1) ) ;
		}
	} else {
		if ( time > act->tmp_time ) {
			//チャフ苦しみ(4,8,12,16四択ﾗﾝﾀﾞﾑ)
			GM_SeSetMode( SD_A_MG_INCHA, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
			act->tmp_time = COUNT_VMODE( time + 4*(KR_RandU(4)+1) + ((time-220)/2) );
		}
	}

	if ( !(time % COUNT_VMODE(4) ) ) {
		ChaffPlasma( npc ) ;
	}

	
	if ( time >= COUNT_VMODE( 377 ) ) {
		FVECTOR pos ;
		int num ;

		num = time - COUNT_VMODE(377) ;
		if ( num > 20 ) num = 20 ;

		KR_FMatToFvec( &BODYWORLD( npc->body, BodyBombList[num] ), &pos ) ;
		NewGllBlast3( &pos, BOTH_SIDE, 2000, 3000, 0, FNT_BLAST, WP_C4Bomb, BLAST_TYPE_ONETARG|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER ) ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
//	if ( !(GM_GameStatus & STATE_CHAFF) ) {
//		act->act_end = 1 ;
		NPC_SetActMode( npc, ActDown ) ;
		return ;
	}

}

static void ActDown ( NPCWORK *npc, int time )
{
	Work *work ;
	NPCACT	*act ;

	work = (Work *)npc->character ;
	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
	SET_FLAG ( work->status2, GLL_STATUS2_NO_ZLIMIT ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, GLL_MOT_DOWN ) ;
//		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.25f ) ;
		if ( StatusFightGME( work->status ) ) {
			GM_SeSetMode( SD_E_MGNHOWL1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else if ( StatusFightGLL( work->status ) ) {
			GM_SeSetMode( SD_E_GLLHOWL1, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_E_GLLHOWL12, &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
		NewPadVibration2( 2564076 /*gol_down*/, 0 ) ;
		if ( StatusFightGNO( work->status ) ) {
			NewShakeCameraGLL( 0, 512, 32, &npc->ctrl->mov ) ;
		}
	}

	if ( time == COUNT_VMODE(152) ) {
		if ( GLL_GAME_STATUS & GLL_GS_CHAFF_CLEAR ) {
			NewShakeCameraGLL( 0, 256, 32, &npc->ctrl->mov );
		} else {
			NewShakeCameraGLL( 0, 512, 32, &npc->ctrl->mov );
		}
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		VR_SurrenderEnemy( ) ;
		NPC_SetActMode( npc, ActDeath ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.0f ) ;
		if ( work->end_proc ) {
			GCL_ExecProc( work->end_proc, NULL ) ;
			work->end_proc = 0 ;
		}
		return ;
	}

}

static void ActDeath ( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

	if ( time == 0 ) {
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	}
}
/*----------------------------------------------------------------*/

static FVECTOR CenterChkShift = {0.0, 3000.0, 0.0 } ;
static int Grenade( Work *work )
{
	FVECTOR	chk_center, body_center, v, v2 ;
	int dis, dir ;
	float	speed ;
	GM_BOMB *b ;

	DG_SetPos( &work->body.objs->objs[HUMAN21_MUNE].world ) ;
	DG_PutVector( &CenterChkShift, &chk_center, 1 ) ;

    /* ボムが近くにあるのか調べる */
    for( b=GM_BombList.next ; b!=NULL ; b=b->next ) {
		if ( !(b->flag & GM_BMB_FLAG_BLAST) && 
			 !(b->flag & GM_BMB_FLAG_MGN_IN)	)	{
		    _sceVu0SubVector( &v, b->mov, &chk_center ) ;
		    if ( _sceVu0InnerProduct( &v, &v ) < 1000.0f*1000.0f ) {
//AN_Test_Eye2(b->mov,2);

				KR_FMatToFvec( &BODYWORLD( &work->body, HUMAN21_MUNE ), &body_center ) ;
			    _sceVu0SubVector( &v2, &body_center, b->mov ) ;
				dis = _FVecLen2( &v2 ) ;
				dir = _FVecDir2( &v2 ) ;
//				speed = (float)dis / 15.0 ;
				speed = (float)dis / 1.0 ;
				b->step->vx = speed * _RsinF( dir ) ;
				b->step->vz = speed * _RcosF( dir ) ;
				b->step->vx = STEP_VMODE(b->step->vx) ;
				b->step->vz = STEP_VMODE(b->step->vz) ;
				GM_SeSetMode( SD_A_MG_INFAL, b->mov, GM_SEMODE_BOMB ) ;

				SET_FLAG( b->flag, GM_BMB_FLAG_INVISIBLE ) ;
				if ( b->weapon == WP_ChaffGrenade ) {
					SET_FLAG( b->flag, GM_BMB_FLAG_MGN_IN ) ;
					SET_FLAG( b->flag, GM_BMB_FLAG_NOBLAST ) ;
					return 1 ;
				} else {
				}
			}
		}
	}
	return 0 ;
}


/* 跳弾 */
static void CallSpark( FVECTOR *pos ,FVECTOR *force )
{
	extern void *NewSpark( FMATRIX *world ) ;
    SVECTOR	rot ;
    FMATRIX	w ;

    _FVecToRotXY( force, &rot ) ;
    DG_SetPos2( pos, &rot ) ;
    DG_GetPos( &w ) ;
    NewSpark( &w ) ;
}

static void	GNO_NearTargetHit( Work *work, int n )
{
	switch ( n ) {
		case SMP_TARGET_CHILD_ARMR1 :
		case SMP_TARGET_CHILD_ARMR2 :
			work->trg_time[ SMP_TARGET_CHILD_ARMR1 ] = REGENE_TIME ;
			work->trg_time[ SMP_TARGET_CHILD_ARMR2 ] = REGENE_TIME ;
		break ;
		case SMP_TARGET_CHILD_ARML1 :
		case SMP_TARGET_CHILD_ARML2 :
			work->trg_time[ SMP_TARGET_CHILD_ARML1 ] = REGENE_TIME ;
			work->trg_time[ SMP_TARGET_CHILD_ARML2 ] = REGENE_TIME ;
		break ;
		case SMP_TARGET_CHILD_LEGR1 :
		case SMP_TARGET_CHILD_LEGR2 :
			work->trg_time[ SMP_TARGET_CHILD_LEGR1 ] = REGENE_TIME ;
			work->trg_time[ SMP_TARGET_CHILD_LEGR2 ] = REGENE_TIME ;
		break ;
		case SMP_TARGET_CHILD_LEGL1 :
		case SMP_TARGET_CHILD_LEGL2 :
			work->trg_time[ SMP_TARGET_CHILD_LEGL1 ] = REGENE_TIME ;
			work->trg_time[ SMP_TARGET_CHILD_LEGL2 ] = REGENE_TIME ;
		break ;
	}
}


/* ダメージを受けたチャイルドターゲットにカウントを入れる */
/* obj -> hart */
static int ObjToHeart[GLL_TARGET_NUM] = {
	0,2,2,3,3,4,4,5,5,1,
} ;

static void HeartToObjDamage( Work *work, int obj_n )
{
	switch( obj_n ) {
		case 0 :
		 work->trg_time[ 0 ] = -1 ;
		 break ;
		case 1 :
		 work->trg_time[ 9 ] = -1 ;
		 break ;
		case 2 :
		 work->trg_time[ 1 ] = -1 ;
		 work->trg_time[ 2 ] = -1 ;
		 break ;
		case 3 :
		 work->trg_time[ 3 ] = -1 ;
		 work->trg_time[ 4 ] = -1 ;
		 break ;
		case 4 :
		 work->trg_time[ 5 ] = -1 ;
		 work->trg_time[ 6 ] = -1 ;
		 break ;
		case 5 :
		 work->trg_time[ 7 ] = -1 ;
		 work->trg_time[ 8 ] = -1 ;
		 break ;
	}
}


static void GllHeartCheck( Work *work, int obj_n )
{
//printf(" obj[%d] : time[%d] flag[%x]\n",obj_n,work->trg_time[ obj_n ],work->gllheart[ ObjToHeart[obj_n] ] ) ;
	if ( work->gllheart[ ObjToHeart[obj_n] ] & GLL_HEART_DAMAGE ) {
		work->trg_time[ obj_n ] = COUNT_VMODE(GLL_REGENE_TIME) ;
	}
}

static void	GLL_ChildTargetDamCount( NPCWORK	*npc )
{
	NPCTARGET	*npctrg ;
	TARGET		*def_child ;
	Work		*work ;
	int			i, dam ;
	
	work = ( Work * )npc->character ;
	work->damaged = 0 ;
	npctrg = &npc->target ;
	def_child = npctrg->def_child ;
	dam = 1 ;
	for( i=0; i<npctrg->child_trg_num ; i++ ) {
		if ( !StatusFightGME( work->status ) ) {
			if ( TARGET_POWER & def_child->damaged ) {
				if ( def_child->weapon_type & (WP_BLAST) ) {
					SET_FLAG( work->damaged, GLL_DAM_TRGONE ) ;
					work->trg_time[ i ] = REGENE_TIME ;
					if ( StatusFightGLL( work->status ) ) {
						work->trg_time[ i ] = COUNT_VMODE(GLL_REGENE_TIME) ;
					}
					if ( StatusFightGNO( work->status ) ) {
						GNO_NearTargetHit( work, i ) ;
					}
				}
			}
		}
		if ( StatusFightGLL( work->status ) ) {
			GllHeartCheck( work, i ) ;
			dam = 0 ;
		} else {
			if ( work->trg_time[ i ] == 0 ) {
				dam = 0 ;
			}
		}
		def_child ++ ;
	}

	if ( dam ) {
		SET_FLAG( work->damaged, GLL_DAM_TRGALL ) ;
	}
}

/* ダメージを受けたチャイルドターゲット番号を返す */
static int	GLL_ChildTargetCheck( NPCWORK	*npc )
{
	NPCTARGET	*npctrg ;
	TARGET		*def_child ;
	int			i ;
	
	npctrg = &npc->target ;
	def_child = npctrg->def_child ;
	for( i=0; i<npctrg->child_trg_num ; i++ ) {
		if ( TARGET_POWER & def_child->damaged ) {
			return i ;
		}
		def_child ++ ;
	}

	return -1 ;
}

static	int	GllCheckDamage( NPCWORK	*npc )
{
	NPCACT	*act ;
	NPCTARGET	*trg ;
	long64	weapon ;
	int		dam_child_num, i ;
	Work	*work ;

	work = ( Work * )npc->character ;
	act = &npc->action ;
	trg = &npc->target ;
	weapon = 0 ;

	if ( StatusFightGNO ( work->status ) ) {
		if ( GLL_GAME_STATUS & GLL_GS_TUB_HEAD ) {
			SET_FLAG( work->damaged, GLL_DAM_TUB ) ;
			NPC_SetActMode( npc, ActDown ) ;
			return 1 ;
		}
	}

	if ( !StatusFightGNO ( work->status ) ) {
		GLL_ChildTargetDamCount( npc ) ;
	}
	/* 跳弾*/
	if ( (dam_child_num = GLL_ChildTargetCheck( npc )) >= 0 ) {
		trg->dam_trg = trg->def_child + dam_child_num ;
		weapon = trg->dam_trg->weapon_type ;
		NPC_DamageFlagClear( npc ) ;
#if 0
printf("dam_child_num = [%d]\n", dam_child_num ) ;
if ( weapon & (WP_M92) ) {
NPC_SetActMode( npc, ActDown ) ;
return 1 ;
}
#endif
		if ( weapon & (WP_BULLET) ) {
			CallSpark( &trg->dam_trg->hit , &trg->dam_trg->power->force ) ;
		}
		if ( StatusFightGME( work->status ) ) {
			if ( (weapon & (WP_BLAST)) && (dam_child_num == SMP_TARGET_CHILD_HEAD ) ) {
				SET_FLAG( work->damaged, GLL_DAM_HIT_HEAD ) ;
			}
		}
	}

	if ( work->damaged & GLL_DAM_TRGALL ) {
		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
		act->life -= BOMB_DAM ;
		if ( act->life > 0 ) {
			NPC_SetActMode( npc, ActDam ) ;
		} else {
			NPC_SetActMode( npc, ActDown ) ;
		}
		return 1 ;
	}

	if ( StatusFightGME ( work->status ) ) {
		if ( work->head_flag & GLL_HEAD_BREAK ) {
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
			act->life -= HEAD_DAM ;
			if ( act->life > 0 ) {
				NPC_SetActMode( npc, ActDam ) ;
			} else {
				NPC_SetActMode( npc, ActDown ) ;
			}
			return 1 ;
		}
		if ( work->status2 & GLL_STATUS2_NO_HEAD ) {
			if ( Grenade( work ) ) {
				SET_FLAG( work->damaged, GLL_DAM_CHAFF ) ;
				NPC_SetActMode( npc, ActChaffIn ) ;
printf("cHaFfFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n");
				return 2 ;	/* チャフダメージ */
			}
		}
		if ( GM_GameStatus & STATE_CHAFF ) {
//			if ( !(act->status & GLL_ACT_STATUS_CAHFF_DAMAGE) ) {
				SET_FLAG( work->damaged, GLL_DAM_CHAFF ) ;
//				NPC_SetActMode( npc, ActChaff ) ;
//				return 2 ;	/* チャフダメージ */
//			}
		}
	}

	if ( StatusFightGLL ( work->status ) ) {
		for( i=0; i<GLL_MAX_HEART ; i++ ) {
			if ( (work->gllheart[i] & GLL_HEART_DESTROY) &&
				 !(work->gllheart[i] & GLL_BODY_BREAK) ) {
				SET_FLAG( work->damaged, GLL_DAM_HEART ) ;
				SET_FLAG( work->gllheart[i], GLL_BODY_BREAK) ;
				HeartToObjDamage( work, i ) ;
				/* このフレームではダメージを受けない */
				NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
				act->life -= (SMP_LIFE+GLL_MAX_HEART+1)/GLL_MAX_HEART ;
				if ( act->life > 0 ) {
					NPC_SetActMode( npc, ActGllDam ) ;
				} else {
					VR_KillCount++ ;
					NPC_SetActMode( npc, ActDown ) ;
				}
				return 1 ;
			} else if ( work->gllheart[i] & GLL_HEART_DAMAGE) {
				SET_FLAG( work->damaged, GLL_DAM_HIT_HEART ) ;
//				act->adj_piku_time = COUNT_VMODE(2) ;
			}
		}
	}

	return 0 ;
}

/*----------------------------------------------------------------*/
static void GLL_MotionAdjustAim( body, trg_rot, dir, parts, interp )
OBJECT		*body ;
SVECTOR		*trg_rot ;
int			dir ;
int			parts ;
int			interp ;
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR		quat ;
	SVECTOR		rot ;

	m_ctrl = body->m_ctrl ;

	rot.vx = 0 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	if ( parts & ADJ_PARTS_WEST ) {
		rot.vy = trg_rot->vy ;
		rot.vx = trg_rot->vx ;
#if 1
		rot.vx /= 2 ;
		rot.vy /= 2 ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ HUMAN21_ATAMA ] = quat ;
		m_ctrl->adjust_flag |= 1 << HUMAN21_ATAMA ;
		m_ctrl->adjust[ HUMAN21_MUNE ] = quat ;
		m_ctrl->adjust_flag |= 1 << HUMAN21_MUNE ;
#else
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ HUMAN21_MUNE ] = quat ;
		m_ctrl->adjust_flag |= 1 << HUMAN21_MUNE ;
#endif
	} else if ( parts & ADJ_PARTS_HEAD ) {
		rot.vy = trg_rot->vy ;
		rot.vx = trg_rot->vx ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ HUMAN21_ATAMA ] = quat ;
		m_ctrl->adjust_flag |= 1 << HUMAN21_ATAMA ;
	}
}

static void GLL_MotionAdjustBDMH( body, trg_rot, dir, parts, interp )
OBJECT		*body ;
SVECTOR		*trg_rot ;
int			dir ;
int			parts ;
int			interp ;
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR		quat ;
	SVECTOR		rot ;

	m_ctrl = body->m_ctrl ;

#if 1
	rot.vx = trg_rot->vx ;
	rot.vy = trg_rot->vy ;
	rot.vz = 0 ;
	GM_RotToQuat( &rot, &quat ) ;
	m_ctrl->adjust[ HUMAN21_ONAKA ] = quat ;
	m_ctrl->adjust_flag |= 1 << HUMAN21_ONAKA ;

	rot.vx = -trg_rot->vx/2 ;
	rot.vy = -trg_rot->vy/2 ;
	rot.vz = 0 ;
	GM_RotToQuat( &rot, &quat ) ;
	m_ctrl->adjust[ HUMAN21_MUNE ] = quat ;
	m_ctrl->adjust_flag |= 1 << HUMAN21_MUNE ;
#else
	rot.vx = trg_rot->vx ;
	rot.vy = trg_rot->vy ;
	rot.vz = 0 ;
	GM_RotToQuat( &rot, &quat ) ;
//	m_ctrl->adjust[ HUMAN21_MUNE ] = quat ;
//	m_ctrl->adjust_flag |= 1 << HUMAN21_MUNE ;
	m_ctrl->adjust[ 1 ] = quat ;
	m_ctrl->adjust_flag |= 1 << 1 ;
#endif
}

static void Adjust( NPCWORK *npc )
{
//	static	ROTLIMIT	rlim = { -512, 512, -768, 768 } ;
	static	ROTLIMIT	rlim = { -1024, 1024, -768, 768 } ;
	Work *work ;
	NPCADJUST	*nadj ;
	FVECTOR		trg, pos, vec ;
	SVECTOR		rot ;
	FMATRIX		*w ;
	int			flag, interp ;

	work = ( Work * ) npc->character ;
	nadj = npc->nadj ;

#if 1
	if ( nadj->adj_status & NPC_ADJ_ON ) {
#else
	if (1) {
		nadj->aim_pos = GM_PlayerPosition ;
#endif
		w = &(npc->body->objs->objs[HUMAN21_ATAMA].world) ;
		KR_FMatToFvec( w, &pos ) ;
		trg = nadj->aim_pos ;

//printf("\ntrg x[%f] y[%f] z[%f]\n",trg.vx, trg.vy, trg.vz);
//printf("pos x[%f] y[%f] z[%f]\n",pos.vx, pos.vy, pos.vz);

		_sceVu0SubVector(  &vec, &trg, &pos ) ;
//printf("vec x[%f] y[%f] z[%f]\n",vec.vx, vec.vy, vec.vz);
		_FVecToRotXY( &vec, &rot ) ;

		rot.vx -= 1024 ;

//printf("ctrl y[%d] rot x[%d] y[%d] >",npc->ctrl->rot.vy, rot.vx,rot.vy);
		if ( rot.vx < rlim.upper ) rot.vx = rlim.upper ;
		if ( rot.vx > rlim.lower ) rot.vx = rlim.lower ;

		/* 体の向きからの角度に変換 */
		rot.vy = (short)GV_DiffDirS( npc->ctrl->rot.vy, rot.vy ) ;

		if ( rot.vy < rlim.right ) rot.vy = rlim.right ;
		if ( rot.vy > rlim.left )  rot.vy = rlim.left ;

//printf("x[%d] y[%d] \n",rot.vx,rot.vy);

		nadj->adj_rot.vx = GV_NearSpeedP( nadj->adj_rot.vx, rot.vx, ADJ_INTRPT ) ;
		nadj->adj_rot.vy = GV_NearSpeedP( nadj->adj_rot.vy, rot.vy, ADJ_INTRPT ) ;

//printf("gll adj rot vx[%d]>vx[%d] ",rot.vx,nadj->adj_rot.vx);
//printf(" vy[%d]>vy[%d] \n",rot.vy,nadj->adj_rot.vy);

		if ( StatusFightGNO( work->status ) ) {//メカゴルは通常で腰をまげる
			flag = ADJ_PARTS_HEAD ;
		} else {
			flag = ADJ_PARTS_WEST ;
		}

		interp = 0 ;
		GLL_MotionAdjustAim( npc->body, &nadj->adj_rot, npc->ctrl->rot.vy, flag , interp) ;
	} else if ( nadj->adj_status & NPC_ADJ_GME_BDMH ) {
		nadj->adj_rot.vx = GV_NearSpeedP( nadj->adj_rot.vx, 1024, ADJ_INTRPT ) ;
		nadj->adj_rot.vy = GV_NearSpeedP( nadj->adj_rot.vy, 0, ADJ_INTRPT ) ;
		flag = ADJ_PARTS_WEST ;
		interp = 0 ;
		GLL_MotionAdjustBDMH( npc->body, &nadj->adj_rot, npc->ctrl->rot.vy, flag , interp) ;
	} else {
		if ( StatusFightGME( work->status ) ) {//メカゴルは通常で腰をまげる
			npc->body->m_ctrl->adjust_flag = 0 ;
			nadj->adj_rot.vx = GV_NearSpeedP( nadj->adj_rot.vx, 330, ADJ_INTRPT ) ;
			nadj->adj_rot.vy = GV_NearSpeedP( nadj->adj_rot.vy, 0, ADJ_INTRPT ) ;
			flag = ADJ_PARTS_WEST ;
			interp = 0 ;
			GLL_MotionAdjustAim( npc->body, &nadj->adj_rot, npc->ctrl->rot.vy, flag , interp) ;
		} else {
			if ( nadj->adj_rot.vx != 0 || nadj->adj_rot.vy != 0 ) {
				nadj->adj_rot.vx = GV_NearSpeedP( nadj->adj_rot.vx, 0, ADJ_INTRPT ) ;
				nadj->adj_rot.vy = GV_NearSpeedP( nadj->adj_rot.vy, 0, ADJ_INTRPT ) ;
	//printf("gll adj rot return vx[%d] vy[%d] \n",nadj->adj_rot.vx,nadj->adj_rot.vy);
				flag = ADJ_PARTS_WEST ;
				interp = 0 ;
				GLL_MotionAdjustAim( npc->body, &nadj->adj_rot, npc->ctrl->rot.vy, flag , interp) ;
			} else {
				npc->body->m_ctrl->adjust_flag = 0 ;
			}
		}
	}
}

static FVECTOR spot_shift = { 0.0, 1100.0, 1300.0 } ;
static void SearchLight( Work *work )
{
	extern void	_FVecToRotXY( FVECTOR *vec, SVECTOR *rot ) ;
	FVECTOR vec, *head ;
	FMATRIX	w ;
	SVECTOR rot ;
	NPCWORK *npc ;
	int face_x, face_y ;

	npc = &work->npc ;
	w = npc->body->objs->objs[HUMAN21_ATAMA].world ;
	head = (FVECTOR *)(&w.m[3][0]) ;

	if ( work->status2 & GLL_STATUS2_SL_TRG ) {
		face_x = MatToXRot( &(work->search_world) ) ;
		face_y = MatToYRot( &(work->search_world) ) ;

//		_sceVu0SubVector(  &vec, &GM_PlayerPosition, head ) ;
		_sceVu0SubVector(  &vec, &npc->nadj->aim_pos, head ) ;
		_FVecToRotXY( &vec, &rot ) ;
		rot.vx -= 1028 ;

		rot.vx = GV_NearSpeedP( face_x, rot.vx, ADJ_INTRPT ) ;
		rot.vy = GV_NearSpeedP( face_y, rot.vy, ADJ_INTRPT ) ;

		DG_SetPos2( head, &rot ) ;
	} else {
		DG_SetPos( &w ) ;
		DG_MovePos( &spot_shift ) ;
	}
	DG_GetPos( &(work->search_world) ) ;

	UNSET_FLAG( GLL_GAME_STATUS, GLL_GS_SEARCH_LIGHT_IN ) ;
	if ( work->head_flag & GLL_HEAD_LIT_BREAK ) {
		work->spot_color = 0 ;
		work->sw_spot = 0 ;
	} else if ( work->status2 & GLL_STATUS2_SEARCH ) {
		work->spot_color = LightSneak ;
		work->sw_spot = 1 ;
	} else if ( work->status2 & GLL_STATUS2_ALERT ) {
		SET_FLAG( GLL_GAME_STATUS, GLL_GS_SEARCH_LIGHT_IN ) ;
		work->spot_color = LightAlert ;
		work->sw_spot = 2 ;
	} else if ( work->status2 & GLL_STATUS2_AVOID ) {
		work->spot_color = LightAvoid ;
		work->sw_spot = 3 ;
	} else if ( work->status2 & GLL_STATUS2_CHAFF_CRAZY ) {
		if ( !(GV_Time%4) ) {
			work->spot_color = LightChaff[KR_RandU(MAX_CHAFF_LIGHT)] ;
		}
		work->sw_spot = 4 ;
	} else {
		work->spot_color = 0 ;
		work->sw_spot = 0 ;
	}
//work->spot_color = LightSneak ;
}

static void AdjustPiku( NPCWORK *npc )
{
//	static short asjpiku_buff[]={ 16, 64, 128, 192, 256, 128} ;
//	static short asjpiku_buff[]={ 1, 64, 128, 192, 256, 128} ;
	SVECTOR	rot ;
	NPCACT	*act ;

	act = &npc->action ;

	act->adj_piku_time -- ;

	if ( act->adj_piku_time > COUNT_VMODE(5) ) return ;

//	rot.vx = -asjpiku_buff[ act->adj_piku_time ] ;
	rot.vx = -1 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	GM_AdjustRotBody( npc->body, &rot, 1  ) ;
//	rot.vx = asjpiku_buff[ act->adj_piku_time ] ;
//	GM_AdjustRotBody( npc->body, &rot, 11  ) ;
//	GM_AdjustRotBody( npc->body, &rot, 3  ) ;
//	GM_AdjustRotBody( npc->body, &rot, 7  ) ;
}

static void GLL_ActStatusCheck( NPCWORK *npc )
{
	NPCACT	*act ;
	CONTROL *ctrl ;
	Work *work ;
	TARGET		*def_child ;
	int	i, status ;

	work = ( Work * ) npc->character ;
	act = &npc->action ;
	status = act->status ;
	ctrl = npc->ctrl ;

	if ( status & GLL_ACT_STATUS_TRGTIME_CLEAR ) {
		for( i=0; i<GLL_TARGET_NUM; i++ ) {
			switch ( i ) {
				case 0 :
				case 4 :
					work->trg_time[ i ] = 0 ;
				break ;
				case 1 :
				case 3 :
					work->trg_time[ i ] = COUNT_VMODE(15) ;
				break ;
				case 2 :
				case 6 :
				case 8 :
					work->trg_time[ i ] = COUNT_VMODE(30) ;
				break ;
				case 5 :
				case 7 :
					work->trg_time[ i ] = COUNT_VMODE(45) ;
				break ;
				default :
					work->trg_time[ i ] = 0 ;
				break ;
			}
		}
	}

	def_child = npc->target.def_child ;
	BreakParts = 0 ;
	for( i=0; i<GLL_TARGET_NUM; i++ ) {
		SET_FLAG( def_child->class, TARGET_LOCKON ) ;
		UNSET_FLAG( def_child->class, TARGET_SKIP ) ;

//printf("work->status2[%x]  time [%d]\n",work->status2,work->trg_time[ i ] ) ;
		if ( StatusFightGLL ( work->status ) ) {
			if ( work->status2 & GLL_STATUS2_NO_TRG) {
				UNSET_FLAG( def_child->class, TARGET_LOCKON ) ;
				SET_FLAG( def_child->class, TARGET_SKIP ) ;
				if( work->trg_time[ i ] < 0 ) {
					work->vanim_flag[ i ] |= SW_BODY_DBROKEN ;
				}
			} else if ( work->trg_time[ i ] > 0 ) {
				if ( work->trg_time[ i ] == COUNT_VMODE(GLL_REGENE_TIME) ) work->vanim_flag[ i ] |= SW_BODY_DAMAGE ;
				SET_FLAG( work->damaged, GLL_DAM_REGENE ) ;
				UNSET_FLAG( def_child->class, TARGET_LOCKON ) ;
				SET_FLAG( def_child->class, TARGET_SKIP ) ;
				SET_FLAG( work->gllheart[ ObjToHeart[i] ], GLL_BODY_DAMAGE ) ;
//printf("target skip num = [%d]\n",i ) ;
				work->trg_time[ i ] -- ;
			} else if( work->trg_time[ i ] < 0 ) {
				UNSET_FLAG( def_child->class, TARGET_LOCKON ) ;
				SET_FLAG( def_child->class, TARGET_SKIP ) ;
				work->vanim_flag[ i ] |= SW_BODY_DBROKEN ;
				SET_FLAG( work->gllheart[ ObjToHeart[i] ], GLL_BODY_DAMAGE ) ;
			} else {
				UNSET_FLAG( work->gllheart[ ObjToHeart[i] ], GLL_BODY_DAMAGE ) ;
			}
		} else if ( StatusFightGME ( work->status ) ) {
			if ( (i == SMP_TARGET_CHILD_HEAD) &&
				 !(work->status2 & GLL_STATUS2_NO_HEAD) ) {
			} else {
				if ( i == SMP_TARGET_CHILD_HEAD ) {
					SET_FLAG( def_child->class, TARGET_SKIP ) ;
				}
				UNSET_FLAG( def_child->class, TARGET_LOCKON ) ;
			}
		} else if ( StatusFightGNO ( work->status ) ) {
			UNSET_FLAG( def_child->class, TARGET_LOCKON ) ;
		} else {
			if ( work->trg_time[ i ] > 0 ) {
				SET_FLAG( work->damaged, GLL_DAM_REGENE ) ;
				UNSET_FLAG( def_child->class, TARGET_LOCKON ) ;
				SET_FLAG( def_child->class, TARGET_SKIP ) ;
				work->trg_time[ i ] -- ;
				work->vanim_flag[ i ] = 1 ;
				SET_FLAG( work->gllheart[ ObjToHeart[i] ], GLL_BODY_DAMAGE ) ;
			} else if( work->trg_time[ i ] < 0 ) {
				UNSET_FLAG( def_child->class, TARGET_LOCKON ) ;
				SET_FLAG( def_child->class, TARGET_SKIP ) ;
				work->vanim_flag[ i ] = 1 ;
				SET_FLAG( work->gllheart[ ObjToHeart[i] ], GLL_BODY_DAMAGE ) ;
			} else {
				UNSET_FLAG( work->gllheart[ ObjToHeart[i] ], GLL_BODY_DAMAGE ) ;
				work->vanim_flag[ i ] = 0 ;
			}
		}

		if ( work->vanim_flag[ i ] & SW_BODY_DBROKEN ) {
			BreakParts |= 1<<npc->target.connect_obj[i] ;
		}

		/* ターゲット判定のＯＮ，ＯＦＦ */
		if ( status & NPC_ACT_STATUS_TRG_OFF ) {
			UNSET_FLAG( def_child->class, TARGET_LOCKON ) ;
			SET_FLAG( def_child->class, TARGET_SKIP ) ;
		}
	    def_child++ ;
	}

	/* 腰アジャスト */
	Adjust( npc ) ;

 	/* ピクアジャスト*/
	if ( act->adj_piku_time > 0 ) {
		AdjustPiku( npc ) ;
	}
	if ( StatusFightGME ( work->status ) ) {
		if ( work->status2 & GLL_STATUS2_NO_HEAD ) {
			npc->body->objs->objs[ 12 ].flag |= DG_FLAG_INVISIBLE ;
			npc->body->objs->objs[ 11 ].flag |= DG_FLAG_INVISIBLE ;
		} else {
			npc->body->objs->objs[ 12 ].flag &= ~DG_FLAG_INVISIBLE ;
			npc->body->objs->objs[ 11 ].flag &= ~DG_FLAG_INVISIBLE ;
		}
		/* サーチライト */
		SearchLight( work ) ;
	}


}

static void Action( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	UNSET_FLAG( GLL_GAME_STATUS, GLL_GS_ACHOO ) ;

	NPC_Action( npc ) ;
	GLL_ActStatusCheck( npc ) ;

	NPC_Gravitation( npc ) ;

	if ( npc->ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		/* Y移動量はモーションに任せる */
		npc->ctrl->step.vy = ((npc->body->height*10.0) - npc->old_body_height) ;
//printf("npc->ctrl->step.vy[%f]  [%f] - [%f]\n",npc->ctrl->step.vy,npc->body->height,npc->old_body_height);
	}
}
