/*
	endamact.c
	敵兵、ダメージアクション

	1997/07/28 Y.Korekado
	$Id: endamact.c,v 1.2 2003/01/09 14:32:12 Yoshizawa1 Exp $
	
*/

#include "BP_TrophyLogicMGS2.h"
#include "BP_TrophySystem.h"

//#define	NO_CORPS (1)

#define JIK_KUBISAKE	(1)

#define PUT_ITEM	(1)
#define	HANG_SHILD	(1)
#define	BODY_ATTACK	(1)
//#define	DURABLE_RESET (1)	/* 防御値復活 */

#define LEAVE_C4_DAMAGE_CLEAR	(1)	

#define KIZETU_KOE	(1)
#define ZZZ_VER_Z	(1)
#define JIK_STEAM_DAM	(1)
#define LEGS_DAM	(1)

#define DAM_BUG	(1)

#define	STAND_DAM	16
#define	STAND_DAM_MAX	32

/*-------------------------------------------------------------------*/

extern	int	CM_ZZZ_TIME ;
extern	int	CM_FAINT_TIME ;

extern void VR_SurrenderEnemy(void) ;
/*-------------------------------------------------------------------*/
static	int FaintSetStepNear( ACTION *act, int time ) ;
static	int FaintSetStep( ACTION *act, int time ) ;
static void UnsetLeaveC4CaptureFlag( ACTION *act ) ;
/*-------------------------------------------------------------------*/
static	void ActFaintWakeup( ACTION *, int ) ;
static	void ActFaintTake( ACTION *, int ) ;
static	void ActFaintMove( ACTION *, int ) ;
static	void ActFaintDown( ACTION *, int ) ;
static	void ActFaintIntoLocker( ACTION *, int ) ;
static	void ActFaintIntoToilet( ACTION *, int ) ;
void ActFaintStandLocker( ACTION *, int ) ;
static	void ActFaintDownLocker( ACTION *, int ) ;
static	void ActHangAvoid( ACTION *, int ) ;
static	void ActHang( ACTION *, int ) ;
static	void ActHangIdle( ACTION *, int ) ;
static	void ActHangMove( ACTION *, int ) ;
static	void ActHangStruggle( ACTION *, int ) ;
static	void ActHangEscape( ACTION *, int ) ;
void ActHangDie( ACTION *, int ) ;
static	void ActHangFree( ACTION *, int ) ;
static	void ActHangFall( ACTION *, int ) ;
static	void ActFall( ACTION *, int ) ;
static	void ActDirFall( ACTION *, int ) ;
static	void ActWallClash( ACTION *, int ) ;
static	void ActWallFall( ACTION *, int ) ;
static	void ActFenceFall( ACTION *, int ) ;
static	void ActCliffFall( ACTION *, int ) ;
static	void ActFallDown( ACTION *, int ) ;
static	void ActFallDownEnd( ACTION *, int ) ;
static	void ActStairFallStart( ACTION *, int ) ;
static	void ActStairFallDown( ACTION *, int ) ;
static	void ActStairFallDownEnd( ACTION *, int ) ;
static	void ActDropp( ACTION *, int ) ;
void endamact_ActDown( ACTION *, int ) ;
static	void ActDownDamage( ACTION *, int ) ;
static	void ActFaint( ACTION *, int ) ;
static	void ActWakeup( ACTION *, int ) ;
static	void ActDeath( ACTION *, int ) ;
static	void ActGhost( ACTION *, int ) ;
static	void ActDamage( ACTION *, int ) ;
static	void ActSteam( ACTION *, int ) ;
static	void ActMortally( ACTION *, int ) ;
static	void ActHoldupExit( ACTION *, int ) ;
static	void ActFaintExit( ACTION *, int ) ;

/*-------------------------------------------------------------------*/
#define	HANG_FAINT 1
#define	PUNCH_FAINT 1
#define	KICK_FAINT 3
#define	BLADE_FAINT 6

#define ONE_FAINT_DOWN (100)
#define NIKITA_FAINT_DOWN (10)
#define	THROW_FAINT 5
#define	HANG_STRUGGLE_TIME COUNT_VMODE(180)
#define	HANG_OFF_TIME COUNT_VMODE(120)

#define USP_DAM			1000
#define BOMB_DAM		3000
#define VR_BOMB_DAM		6000
#define VR_BOMB_BIGDAM	10000
//#define BLADE_CUT_DAM	1000
#define BLADE_CUT_DAM	1500
#define BLADE_THRUST_DAM	1000

#define DOWN_FAINT_DAMAGE 60*30

#define VPOINT_ANES		3000
#define BODY_ANES		1000
#define LIMBS_ANES		500

static int WallOutCheck( act, dir )
ACTION	*act ;
int		dir ;
{
	float	levels[2] ;
	int flag, n_touches, flr_flag ;
	FVECTOR	pos, pos2 ;
	CONTROL	*ctrl ;

	ctrl = act->ctrl ;

	pos.vx = ctrl->mov.vx + (1000.0f * _RsinF( dir )) ;
	pos.vy = ctrl->levels[0] + 500 ;
	pos.vz = ctrl->mov.vz + (1000.0f * _RcosF( dir )) ;

	/* 壁の向こうの周りの壁チェック */
    n_touches = HZX_NearHazardCheck( ctrl->hzx_id, &pos, 650, 
				     ctrl->hzx_check_type, ctrl->seg_flag, 600 ) ;

//printf("kore4[%d]\n", n_touches );
	if ( n_touches > 1 ) return 0 ;

	/* 落ちる高さチェック */
	pos.vy = ctrl->levels[0] + 1500 ;
	flr_flag = ctrl->flr_flag ;
    flag = HZX_LevelHazardCheck( act->ctrl->hzx_id, &pos,
				 act->ctrl->hzx_check_type, flr_flag ) ;
//printf("kore5[%d]\n", flag );

	if ( flag != 0 ) {
		HZX_GetLevelHeight( levels ) ; 
		if ( !( 1 & flag ) ) levels[ 0 ] = 0.0F ;
	} else {
		return 0 ;
	}
//printf("kore6 posvy[%f] level0[%f]\n",pos.vy, levels[ 0 ] );
	if ( (pos.vy - levels[ 0 ]) < 3000+1500 ) return 0 ;

	/* 乗り越える高さのオンラインチェック */
	pos2.vx = ctrl->mov.vx ;
	pos2.vy = pos.vy ;
	pos2.vz = ctrl->mov.vz ;
//printf("kore7\n" );
    if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &pos, &pos2, ctrl->hzx_check_type, 
			        ctrl->seg_flag, ctrl->flr_flag ) ) {
		return 0 ;
	}

	return 1 ;
}

static int HeadHight( act )
ACTION	*act ;
{
	FVECTOR	pos ;
	
	KR_FMatToFvec( &BODYWORLD( act->body, HUMAN21_ATAMA), &pos ) ;
	
	return pos.vy - act->ctrl->levels[0] ;
}

static int FenceCheck( act )
ACTION	*act ;
{
	CONTROL	*ctrl ;
	float height ;
	int		wall_dir, dirabs, type ;

	ctrl = act->ctrl ;

#if 1	//01.9.12
	if ( act->bodyp.type & ENE_TYPE_USHOLD ) return 0 ;
#endif

	/* 壁にぶつかった */
	if ( ctrl->n_touches <= 0 ) return 0 ;

	/* 壁の高さが一定以上 */
	height = ctrl->segs[0]->p1.y + ctrl->segs[0]->p1.h - ctrl->levels[0] ;
//printf("y=%f h=%f floor=%f hight=%f\n",ctrl->segs[0]->p1.y, ctrl->segs[0]->p1.h , ctrl->levels[0], height) ;
	if ( height > 1010.0F ) return 0 ;
	/* 斜め壁ではない */
	if ( DG_FABS(ctrl->segs[0]->p1.y+ctrl->segs[0]->p1.h - ctrl->segs[0]->p2.y-ctrl->segs[0]->p2.h)
		 > 100.0f) {
		return 0 ;
	}

/*
    if ( ( ctrl->level[ 0 ] == NULL ) ) return 0 ;
    flr = ctrl->level[ 0 ] ;
	if ( DG_FABS(flr->p1.h) > 0.1 || DG_FABS(flr->p2.h) > 0.1 ) return 0 ;
*/
	/* 頭の位置が床から１０００以上高い */
	if ( HeadHight( act ) < 1000 ) return 0 ;

	/* 方向 */
	wall_dir = GV_VecDir2( &ctrl->vecs[ 0 ] ) ;
	dirabs = GV_DiffDirAbs( wall_dir, ctrl->turn.vy ) ;
//printf( "dirabs[%d] turn.vy[%d]  wall_dir[%d]\n",dirabs, act->ctrl->turn.vy, wall_dir ) ;
	type = -1 ;
	if ( dirabs > 1536 ) type = 1 ;	/* 後ろ向き */
	if ( dirabs < 512 ) type = 0 ;	/* 前向き */
	if ( type < 0 ) return 0 ;

	/* 壁の向こう側チェック */
	if ( !WallOutCheck( act, wall_dir ) ) return 0 ;

	switch( type ) {
		case 0 :
			act->ctrl->turn.vy  = wall_dir ;
			act->down_s = DownFront ;
			break ;
		case 1 :
			act->ctrl->turn.vy  = 4095 & (wall_dir + 2048) ;
			act->down_s = DownBack ;
			break ;
	}

printf(" Fence Fall Start1! \n") ;
	return 1 ;
}

static int CliffCheck( act )
ACTION	*act ;
{
    int   atr[2] ;
    HZX_SEG seg[2] ;
	CONTROL	*ctrl ;
	int		wall_dir, dirabs, type, n_touches ;
	FVECTOR pos ;
	FVECTOR	vect_ptr[2] ;

	ctrl = act->ctrl ;

	pos.vx = ctrl->mov.vx ;
	pos.vy = ctrl->levels[0] + 500 ;
	pos.vz = ctrl->mov.vz ;
	
	/* 壁の向こうの周りの壁チェック */
    n_touches = HZX_NearHazardCheck( ctrl->hzx_id, &pos, 650, 
				     (HZX_CHK_F_SEGMENT|HZX_CHK_CLIFF), 0, 600 ) ;
	/* 壁にぶつかった */
	if ( n_touches <= 0 ) return 0 ;

	HZX_GetNearHazard( seg, atr ) ;
	HZX_GetNearVector( vect_ptr ) ;

	/* 崖壁じゃなかった */
	if ( !(atr[0] & HZX_SEG_CLIFF) ) return 0 ;

#if 0
	/* 斜め壁ではない */
	if ( DG_FABS(seg[0]->p1.y+seg[0]->p1.h - seg[0]->p2.y-seg[0]->p2.h) > 100.0f) {
		return 0 ;
	}
#endif

	/* 頭の位置が床から１０００以上高い */
	if ( HeadHight( act ) < 1000 ) return 0 ;

	/* 方向 */
	wall_dir = GV_VecDir2( &vect_ptr[ 0 ] ) ;
	dirabs = GV_DiffDirAbs( wall_dir, ctrl->turn.vy ) ;
//printf( "turn.vy[%d]  wall_dir[%d]\n",act->ctrl->turn.vy, wall_dir ) ;
	type = -1 ;
	if ( dirabs > 1536 ) type = 1 ;	/* 後ろ向き */
	if ( dirabs < 512 ) type = 0 ;	/* 前向き */
	if ( type < 0 ) return 0 ;

	/* 壁の向こう側チェック */
	if ( !WallOutCheck( act, wall_dir ) ) return 0 ;

	switch( type ) {
		case 0 :
			act->ctrl->turn.vy  = wall_dir ;
			act->down_s = DownFront ;
			break ;
		case 1 :
			act->ctrl->turn.vy  = 4095 & (wall_dir + 2048) ;
			act->down_s = DownBack ;
			break ;
	}

printf(" Cliff Fall Start1! \n") ;
	return 1 ;
}

static int LevelDeathCheck( act )
ACTION	*act ;
{
	CONTROL	*ctrl ;

//return 0 ;

	ctrl = act->ctrl ;

	/* 落下加速度がー１００００を超えたら死亡 */
	if ( ctrl->step.vy < -10000 ) {
		if ( act->bodyp.life > 0 ) {
			if ( !(GM_GameStatus & STATE_VR_ANOTHER) ) {
				act->bodyp.last_weapon = WP_NOPLAYER ;
			}
		}
		return 1 ;
	}

	if ( act->bodyp.type & ENE_TYPE_TNG_A ) return 0 ;

    if ( ( ctrl->level[ 0 ] == NULL ) ) {
		printf("enemy: WARNING LEVEL NULL!!!!!!\n");
		return 0 ;
	}

	if ( (ctrl->mov.vy - ctrl->levels[0]) > 5000 ) {
		if ( act->bodyp.life > 0 ) {
			if ( !(GM_GameStatus & STATE_VR_ANOTHER) ) {
				act->bodyp.last_weapon = WP_NOPLAYER ;
			}
		}
		return 1 ;
	}

	return 0 ;
}

static int StairCheck( act )
ACTION	*act ;
{
	CONTROL *ctrl ;
	HZX_FLR *flr ;
	FVECTOR	norm, pos, pos2 ;
	int dir, type, dirabs ;
	float	h, hight ;

#if 1	//01.9.12
	if ( act->bodyp.type & ENE_TYPE_USHOLD ) return 0 ;
#endif

	ctrl = act->ctrl ;
    if ( ( ctrl->level[ 0 ] == NULL ) ) return 0 ;
    flr = ctrl->level[ 0 ] ;

    norm.vx = flr->p1.h ;
    norm.vz = flr->p2.h ;
    norm.vy = 0 ;

	dir = _FVecDir2( &norm ) ;
//printf( "turn.vy[%d]  dir[%d]\n",act->ctrl->turn.vy, dir ) ;
	/* 転がり方向チェック */
	dirabs = GV_DiffDirAbs( dir, ctrl->turn.vy ) ;
	type = -1 ;
	if ( dirabs > 1536 ) type = 1 ;	/* 後ろ向き */
	if ( dirabs < 512 ) type = 0 ;	/* 前向き */
	if ( type < 0 ) return 0 ;

	/* 転がり傾斜チェック */
	pos.vx = ctrl->mov.vx + (2000.0f * _RsinF( dir )) ;
	pos.vy = ctrl->levels[0] ;
	pos.vz = ctrl->mov.vz + (2000.0f * _RcosF( dir )) ;
    HZX_SlopeFloorLevel( &h, &pos, ctrl->level[ 0 ] ) ;
    hight = ctrl->levels[0] - h ;
//printf( "hight[%f] h[%f]\n",hight, h ) ;
    if ( hight < 1000.0f )	return 0 ;

	/* 障害物チェック */
	pos.vy = h+1500.0f ;

	pos2.vx = ctrl->mov.vx ;
	pos2.vy = ctrl->levels[0] + 1500.0f ;
	pos2.vz = ctrl->mov.vz ;
    if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &pos, &pos2, ctrl->hzx_check_type, 
			        ctrl->seg_flag, ctrl->flr_flag ) ) {
		return 0 ;
	}

	if ( type == 1 ) {
		act->ctrl->turn.vy  = 4095 & (2048+dir) ;
		act->down_s = DownBack ;
	} else {
		act->ctrl->turn.vy  = dir ;
		act->down_s = DownFront ;
	}
printf( "start Stair Fall[%d] style[%d]\n",hight, type ) ;
	return 1 ;
}

static int WallCheck( act ) 
ACTION *act ;
{
	CONTROL	*ctrl ;
	int wall_dir ;

	ctrl = act->ctrl ;

#if 1	//01.9.12
	if ( act->bodyp.type & ENE_TYPE_USHOLD ) return 0 ;
#endif

	if ( ctrl->n_touches > 0 ) {
		wall_dir = GV_VecDir2( &ctrl->vecs[ 0 ] ) ;

		if ( GV_DiffDirAbs( wall_dir, ctrl->turn.vy ) > 1536 ) {
			printf( "turn.vy[%d]  wall_dir[%d]\n",act->ctrl->turn.vy, wall_dir ) ;
			act->ctrl->turn.vy  = wall_dir + 2048 ;
			return 1 ;
		}
	}

	return 0 ;
}

static void BombDamageEffect( act )
ACTION	*act ;
{
	extern void TAKABE_OozeBloodAdd( void *, int, FVECTOR *, float ) ;
	static FVECTOR Shift = { 0.0f, 0.0f, 1000.0f } ;
	FVECTOR	vec ;
	SVECTOR	rot ;

	_sceVu0SubVector(  &vec, &act->bodyp.damtrg->center, &act->bodyp.off_center ) ;
	_FVecToRotXY( &vec, &rot ) ;
	
	DG_SetPos2( &act->bodyp.damtrg->center, &rot ) ;
	DG_PutVector( &Shift, &vec, 1 ) ;
//printf("1:BOMB!! [%f][%f][%f]!!!\n",vec.vx,vec.vy,vec.vz );

	TAKABE_OozeBloodAdd( act->oozeblood, HUMAN21_MUNE, &vec, 1000.0F ) ;

//printf("BOMB!! [%f][%f][%f]!!!\n",act->bodyp.off_center.vx,act->bodyp.off_center.vy,act->bodyp.off_center.vz );

}

/* モーションの向きが正面か真後ろ向きの場合のみ使用すること！！ */
static void SetSlopeRotX( act )
ACTION	*act ;
{
	int x ;
	x = ENE_GetGRot( act->ctrl, 1000.0f ) ;
//printf( " slope Rot [%d]\n",x);	
	act->ctrl->turn.vx = x ;
}
static void ReSetSlopeRotX( act )
ACTION	*act ;
{
	act->ctrl->turn.vx = 0 ;
}

static void	_HitEffect( act )
ACTION	*act ;
{
	SVECTOR	rot ;
	FMATRIX	w ;
	extern void *NewSonicWave( FVECTOR *pos /* 中心 */, FVECTOR *nrm /* 法線 */, float radius, CVECTOR col );
//	CVECTOR col={16, 24, 32, 32};
	CVECTOR col={16, 24, 32, 128};

	_FVecToRotXY( &act->bodyp.damtrg->power->force, &rot ) ;
	DG_SetPos2( &act->bodyp.damtrg->hit, &rot ) ;
	DG_GetPos( &w ) ;

	GM_SeSetMode( SD_W_RICOCH02, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;

//	NewSpark( &w ) ;

	NewSonicWave( &act->bodyp.damtrg->hit, &act->bodyp.damtrg->power->force, 100.0f, col );
}

static void BloodBio( ACTION *act )
{
	extern void *NewBloodBioDead( FVECTOR *pos, int life, float size, int white, int map ) ;
	FVECTOR	pos ;

	KR_FMatToFvec( &(act->body->objs->objs[0].world), &pos ) ;
	NewBloodBioDead( &pos, 3000, 500.0, 0, act->ctrl->map ) ;

	KR_FMatToFvec( &(act->body->objs->objs[2].world), &pos ) ;
	NewBloodBioDead( &pos, 3000, 500.0, 0, act->ctrl->map ) ;

	KR_FMatToFvec( &(act->body->objs->objs[11].world), &pos ) ;
	NewBloodBioDead( &pos, 3000, 500.0, 0, act->ctrl->map ) ;
}

static void BloodM9( ACTION *act,int  n_obj, FVECTOR *pos )
{
	extern void *NewBlood_M9( FMATRIX *world, FVECTOR *mov, FVECTOR *norm ) ;
	extern void *NewHiTechFaceBlood( DG_OBJS *objs ) ;
	FVECTOR	vans, nans ;

	if ( act->bodyp.type & ENE_TYPE_HITECH ) {
		GV_SetActorChild( act->w, act->head_blood = NewHiTechFaceBlood( act->body->objs ) ) ;
	} else {
		VertexSearch( &vans, &nans, act->body->objs, n_obj, pos );
		NewBlood_M9( &BODYWORLD( act->body, n_obj), &vans, &nans ) ;
	}
}

static void Blood( act, mode, param )
ACTION *act ;
int		mode, param ;
{
	extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
	FMATRIX	*world ;
	TARGET	*dam ;

	dam = act->bodyp.damtrg ;
	world = &(BODYWORLD( act->body, act->bodyp.n_damobj )) ;
//printf("bloood damtrg force =  [%f] [%f] [%f] \n",dam->power->force.vx,dam->power->force.vy,dam->power->force.vz) ;
	GV_SetActorChild( act->w, NewBlood( world, &dam->center, &dam->power->force, mode, 0 ) ) ;
}

static void BloodBlade( ACTION *act )
{
	extern void *NewPointSplashBlood( FVECTOR *damage_pos, FVECTOR *force, 
		FMATRIX *world, int joint, int model_name ) ;

	NewPointSplashBlood( &act->bodyp.damtrg->hit, &act->bodyp.damtrg->power->force, 
		&(BODYWORLD( act->body, act->bodyp.n_damobj )), act->bodyp.n_damobj, act->name_id->body ) ;
	Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
}

#if 1
static	void HangSetStep( act )
ACTION	*act ;
{
//    static FVECTOR 	Shift = { 34.0F, -465.0F, 263.0F } ;
//    static FVECTOR 	Shift = { 34.0F, 65.0F, 263.0F } ;
    static FVECTOR 	Shift = { 34.0F, 65.0F, 223.0F } ;
     CAPTURE_TARGET	*cap ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov, mov2 ;
    float	     	len ;

	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	cap = &(act->bodyp.capture) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;

	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	DG_PutVector( &Shift, &mov, 1 ) ;

//printf("hang step p_ctrl[%f][%f][%f]\n",pl_ctrl->mov.vx,pl_ctrl->mov.vy,pl_ctrl->mov.vz);
//printf("hang mov2 [%f][%f][%f]\n",mov2.vx,mov2.vy,mov2.vz);
//printf("hang mov [%f][%f][%f]\n",mov.vx,mov.vy,mov.vz);

	/* 当たりチェック高さはプレイヤーと同じにする */
	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;
	if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &mov2, &mov,
		HZX_CHK_ALL,HZX_TYPE_ENEMY,HZX_FLOOR_NO_ENEMY ) ) {
	    /* 間に壁がある場合、壁の少し手前を引きずられ位置にする */
	    HZX_GetOnlineVector( &vec ) ;
	    len = GV_VecLen3F( &vec ) - 8.0F ;
	    if ( len <= 0.0F ) len = 0.0F ;
	    GV_LenVec3F( &vec, &vec, 1.0F, len ) ;
	    _sceVu0AddVector( &mov, &mov2, &vec ) ;
//printf("hang wall [%f][%f][%f]\n",mov.vx,mov.vy,mov.vz);
	}

    act->ctrl->turn.vy = act->bodyp.capture.capture->ctrl->rot.vy ;	    

	act->ctrl->step.vx += mov.vx - act->ctrl->mov.vx ;
//	act->ctrl->step.vy = mov.vy - act->ctrl->mov.vy ;
	act->ctrl->mov.vy = GV_NearTimeF( act->ctrl->mov.vy, mov.vy, 0 ) ;
	act->ctrl->step.vz += mov.vz - act->ctrl->mov.vz ;
    GM_MoveTarget( &( act->bodyp.deftrg ), &( act->ctrl->mov ) ) ;

//printf("hang mov [%f][%f][%f]\n",mov.vx,mov.vy,mov.vz);
//printf("hang pos [%f][%f][%f]\n",act->ctrl->mov.vx,act->ctrl->mov.vy,act->ctrl->mov.vz);
//printf("hang step [%f][%f][%f]\n\n",act->ctrl->step.vx,act->ctrl->step.vy,act->ctrl->step.vz);
}
#else
static	void HangSetStep( act )
ACTION	*act ;
{
    static FVECTOR 	Shift = { 34.0F, -465.0F, 263.0F } ;
     CAPTURE_TARGET	*cap ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov, mov2 ;
    float	     	len ;

	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	cap = &(act->bodyp.capture) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;

	/* 当たりチェック高さはプレイヤーと同じにする */
	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;
	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	DG_PutVector( &Shift, &mov, 1 ) ;

printf("hang step p_ctrl[%f][%f][%f]\n",pl_ctrl->mov.vx,pl_ctrl->mov.vy,pl_ctrl->mov.vz);
printf("hang mov2 [%f][%f][%f]\n",mov2.vx,mov2.vy,mov2.vz);
printf("hang mov [%f][%f][%f]\n",mov.vx,mov.vy,mov.vz);

	/* 暫定 */
	if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &mov2, &mov,
		HZX_CHK_ALL,HZX_TYPE_ENEMY,HZX_FLOOR_NO_ENEMY ) ) {
	    /* 間に壁がある場合、壁の少し手前を引きずられ位置にする */
	    HZX_GetOnlineVector( &vec ) ;
	    len = GV_VecLen3F( &vec ) - 8.0F ;
	    if ( len <= 0.0F ) len = 0.0F ;
	    GV_LenVec3F( &vec, &vec, 1.0F, len ) ;
	    _sceVu0AddVector( &mov, &mov2, &vec ) ;
printf("hang mov2 [%f][%f][%f]\n",mov.vx,mov.vy,mov.vz);
	}
printf("hang mov3 [%f][%f][%f]\n",mov.vx,mov.vy,mov.vz);

    act->ctrl->turn.vy = act->bodyp.capture.capture->ctrl->rot.vy ;	    

	act->ctrl->step.vx += mov.vx - act->ctrl->mov.vx ;
	act->ctrl->step.vy += mov.vy - act->ctrl->mov.vy ;
	act->ctrl->step.vz += mov.vz - act->ctrl->mov.vz ;
    GM_MoveTarget( &( act->bodyp.deftrg ), &( act->ctrl->mov ) ) ;

printf("hang step mov[%f][%f][%f]\n",act->ctrl->mov.vx,act->ctrl->mov.vy,act->ctrl->mov.vz);
printf("hang step step[%f][%f][%f]\n\n",act->ctrl->step.vx,act->ctrl->step.vy,act->ctrl->step.vz);
}
#endif

static int Hang_CheckDamage( act )
ACTION	*act ;
{
	TARGET	*def ;
    CAPTURE_TARGET	*cap ;
	long64		weapon ;

	def = &(act->bodyp.deftrg) ;
	cap = &(act->bodyp.capture) ;

//printf( " cap flag [%x] \n ",cap->flag ) ;

	if ( cap->flag & CAPTURE_BREAK ) {		/* 首折れる */
		cap->capture->flag |= CAPTURE_FREE ;
		cap->capture = NULL ;	
#ifdef LEAVE_C4_DAMAGE_CLEAR
		UnsetLeaveC4CaptureFlag( act ) ;
#else
		cap->flag = 0 ;
#endif
		AT_SetMode( act, ActHangDie ) ;
		act->ctrl->step.vy = 0 ;
		return 1 ;
	}

	if ( TARGET_POWER & def->damaged ) {
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.damtrg = def ;
		def->damaged = FLAG_CLEAR ;
		def->weapon_type = 0 ;
//printf("hang damage weapon type [%lx]  \n",weapon);
		/* このフレームではダメージを受けない */
		AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
//		if ( weapon & WP_M92 ) {
		if ( 1 ) {
			if( act->bodyp.durable[ DURABLE_AREA1 ] > 0 ) {
				act->bodyp.durable[ DURABLE_AREA1 ] -- ;
				_HitEffect( act ) ;
				act->status |= ACT_STATUS_MECABREAK ;
				act->adj_piku_time = ADJ_PIKU_TIME ;
			} else {
#ifdef DURABLE_RESET
				act->bodyp.durable[ DURABLE_AREA1 ] = act->bodyp.max_durable[ DURABLE_AREA1 ] ;
#endif
				act->bodyp.dam_level_num[PTARGET_LEVEL3] ++ ;
				act->bodyp.life -= USP_DAM ;
				if ( act->bodyp.life <= 0 ) {
					cap->capture->flag |= CAPTURE_FREE ;
					cap->capture = NULL ;	
#ifdef LEAVE_C4_DAMAGE_CLEAR
					UnsetLeaveC4CaptureFlag( act ) ;
#else
					cap->flag = 0 ;
#endif
					AT_SetMode( act, ActHangFall ) ;
					act->ctrl->step.vy = 0 ;
					return 1 ;
				} else {
					act->status |= ACT_STATUS_MECABREAK ;
					act->adj_piku_time = ADJ_PIKU_TIME ;
				}
			}
		}
	}

	if ( cap->flag & CAPTURE_HANG ) {	/* 絞め直し */
		cap->flag &= ~CAPTURE_HANG ;
		AT_SetMode( act, ActHang ) ;
		return 1 ;
	}
	if ( cap->flag & CAPTURE_FREE ) {	/* 首絞め解除 */
		cap->capture->flag |= CAPTURE_FREE ;
		cap->capture = NULL ;	
#ifdef LEAVE_C4_DAMAGE_CLEAR
		UnsetLeaveC4CaptureFlag( act ) ;
#else
		cap->flag = 0 ;
#endif
		/* このフレームではダメージを受けない */
		AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
		if ( act->bodyp.faint <= 0 || act->bodyp.anesthesia < 0 || act->bodyp.blood <= 0 ) {
			AT_SetMode( act, ActHangFall ) ;
		} else {
			AT_SetMode( act, ActHangFree ) ;
		}
		act->ctrl->step.vy = 0 ;
		return 1 ;
	}


	return 0 ;
}

static void FaintFloorThrough( ACTION *act )
{
	act->ctrl->step = DG_ZeroVector ;
	if ( act->bodyp.capture.capture != NULL ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
		act->bodyp.capture.capture = NULL ;	
		act->bodyp.capture.flag &= ~CAPTURE_FREE ;
	}
	AT_SetMode( act, ActFaintDown ) ;
}

static int FaintCaptureCheck( act )
ACTION	*act ;
{
    CAPTURE_TARGET	*cap ;

	cap = &(act->bodyp.capture) ;
	if ( cap->flag & CAPTURE_FREE ) { /* ひきずられ終わり */
		cap->capture = NULL ;	
		cap->flag &= ~CAPTURE_FREE ;
		AT_SetMode( act, ActFaintDown ) ;

	    return 1 ;
	}
	if ( cap->flag & CAPTURE_LOCKER ) { /* ロッカー入れられる */
		FaintSetStep( act, 0 ) ;
		cap->capture = NULL ;	
		cap->flag &= ~CAPTURE_FREE ;
		AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
		AT_SetMode( act, ActFaintIntoLocker ) ;
	    return 1 ;
	}


	return 0 ;
}

static	void FaintSetPositionNear( act, mov )
ACTION	*act ;
FVECTOR	*mov ;
{
    static FVECTOR 	Shift[] = {
		/* モーションが始まって１６フレーム目のシフト座標 */
		{ 0.0F, -421.119F, 1192.692F },	/* 仰向け足 */
		{ 26.483F, -393.755F, 940.002F },	/* 仰向け頭 */
		{ 0.0F, -346.867F, 1192.629F },	/* うつぶせ足 */
		{ 45.761F, -317.266F, 946.003F },	/* うつ伏せ頭 */
    } ;
    CAPTURE_TARGET	*cap ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov2, *sft ;
    float	     	len ;

	cap = &(act->bodyp.capture) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;

	/* 当たりチェック高さはプレイヤーと同じにする */
//	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;

	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				sft = Shift + 1 ;
		    } else {
				sft = Shift + 3 ;
		    }
	} else {
	    if ( act->down_s == DownFront || act->down_s == DownWall ) {
			sft = Shift + 0 ;
	    } else {
			sft = Shift + 2 ;
	    }
	}
	DG_PutVector( sft, mov, 1 ) ;
	if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &mov2, mov,
				     HZX_CHK_ALL, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
	    /* 間に壁がある場合、壁の少し手前を引きずられ位置にする */
	    HZX_GetOnlineVector( &vec ) ;
	    len = GV_VecLen3F( &vec ) - 8.0F ;
	    if ( len <= 0.0F ) len = 0.0F ;
	    GV_LenVec3F( &vec, &vec, 1.0F, len ) ;
	    _sceVu0AddVector( mov, &mov2, &vec ) ;
	} 
}

static	void FaintSetPosition( act, mov )
ACTION	*act ;
FVECTOR	*mov ;
{
    static FVECTOR 	Shift[] = {
	{ 0.0F, -382.115F, 422.438F },	/* 仰向け足 */
	{ 0.0F, -113.487F, 291.057F },	/* 仰向け頭 */
	{ 0.0F, -294.030F, 463.501F },	/* うつぶせ足 */
	{ -35.0F, 0.0F, 372.0F }	/* うつぶせ頭 */ /* いらない */
    } ;
    CAPTURE_TARGET	*cap ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov2, *sft ;
    float	     	len ;

	cap = &(act->bodyp.capture) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;
	/* 当たりチェック高さはプレイヤーと同じにする */
//	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;
	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
	    if ( act->down_s == DownFront || act->down_s == DownWall ) {
			sft = Shift + 1 ;
	    } else {
			sft = Shift + 1 ;
	    }
	} else {
	    if ( act->down_s == DownFront || act->down_s == DownWall ) {
			sft = Shift + 0 ;
	    } else {
			sft = Shift + 2 ;
	    }
	}
	DG_PutVector( sft, mov, 1 ) ;
	if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &mov2, mov,
				     HZX_CHK_ALL, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
	    /* 間に壁がある場合、壁の少し手前を引きずられ位置にする */
	    HZX_GetOnlineVector( &vec ) ;
	    len = GV_VecLen3F( &vec ) - 8.0F ;
	    if ( len <= 0.0F ) len = 0.0F ;
	    GV_LenVec3F( &vec, &vec, 1.0F, len ) ;
	    _sceVu0AddVector( mov, &mov2, &vec ) ;
	} 
}

/* モーションが始まって１６フレーム目の総体座標に合わせる */
static	int FaintSetStepNear( act, time )
ACTION	*act ;
int 	time ;
{
	FVECTOR	mov, to ;

	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	FaintSetPositionNear( act, &mov ) ;

	to.vx = GV_NearTimeF( act->ctrl->mov.vx, mov.vx, time ) ;
	to.vz = GV_NearTimeF( act->ctrl->mov.vz, mov.vz, time ) ;

	act->ctrl->step.vx += to.vx - act->ctrl->mov.vx ;
	act->ctrl->step.vz += to.vz - act->ctrl->mov.vz ;

	SetSlopeRotX( act ) ;

	/* 高さ */
	if ( act->ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		to.vy = GV_NearTimeF( act->ctrl->mov.vy, mov.vy, time ) ;
		if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &to, &act->ctrl->mov,
					     HZX_CHK_ALL, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
			printf("!!!!!!!!!!!!!!!!!WARNING flor through !!!!!\n" ) ;
			FaintFloorThrough( act ) ;
			return 1 ;
		}
		act->ctrl->mov.vy = to.vy ;
	}
	return 0 ;
}

static	int FaintSetStep( act, time )
ACTION	*act ;
int 	time ;
{
	FVECTOR	mov, to ;
	CONTROL	*ctrl ;
	int		pose ;

	ctrl = act->ctrl ;
	
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	FaintSetPosition( act, &mov ) ;

//printf("play[%f][%f]  s1mov[%f][%f]\n",GM_PlayerPosition.vx, GM_PlayerPosition.vz, mov.vx,mov.vz ) ;
	/* 持ち上げた後 */
	pose = 0 ;
	if ( (act->c_motion_num[0] == EM_dead_carry_leg_idle_f) ||
		 (act->c_motion_num[0] == EM_dead_carry_leg_start_f) ||
		 (act->c_motion_num[0] == EM_dead_carry_leg_walk_f) ) {
		pose = 1 ;	/* 階段補正反対 */
	} else if ( (act->c_motion_num[0] == EM_dead_carry_leg_idle_b) ||
		 (act->c_motion_num[0] == EM_dead_carry_leg_walk_b) ) {
		pose = 2 ;	/* 階段補正反対 */
	}

	if ( time >= 0 ) {
		if ( act->c_motion_num[0] == EM_dead_carry_leg_idle_f || 
			act->c_motion_num[0] == EM_dead_carry_leg_walk_f ) {
		    act->ctrl->turn.vy = act->ctrl->rot.vy = 
			act->bodyp.capture.capture->ctrl->rot.vy + 2048 ;
		} else if (
			act->c_motion_num[0] == EM_dead_carry_body_idle ||
			act->c_motion_num[0] == EM_dead_carry_leg_idle_b ||
			act->c_motion_num[0] == EM_dead_carry_body_walk ||
			act->c_motion_num[0] == EM_dead_carry_leg_walk_b ){
		    act->ctrl->turn.vy = act->ctrl->rot.vy = 
			act->bodyp.capture.capture->ctrl->rot.vy ;	    
		} else {
			
		}
	}

	if ( time < 0 ) time = 0 ;
	to.vx = GV_NearTimeF( act->ctrl->mov.vx, mov.vx, time ) ;
	to.vz = GV_NearTimeF( act->ctrl->mov.vz, mov.vz, time ) ;

	act->ctrl->step.vx += to.vx - act->ctrl->mov.vx ;
	act->ctrl->step.vz += to.vz - act->ctrl->mov.vz ;

	if ( act->ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
#if 1
		float	f, sub ;
		CONTROL	*pl ;
		
		pl = act->bodyp.capture.capture->ctrl ;
		f = KR_GetGRotFromPos( &ctrl->mov, ctrl->hzx_id, ctrl->hzx_check_type, ctrl->flr_flag ) ;
		if ( (sub = f - pl->levels[0]) > 0.0f ) {
			if ( sub > 450.0f ) sub = 450.0f ;
			switch ( pose ) {
				case 0 :
					mov.vy += (sub / 2) ;
					ctrl->turn.vx = -384 * (sub/450) ;
				break ;
				case 1 :
					mov.vy += (sub / 2) ;
					ctrl->turn.vx = 384 * (sub/450) ;
				break ;
				default :
					mov.vy += (sub ) ;
					ctrl->turn.vx = -384 * (sub/450) * 2 ;
				break ;
			}
//printf("faint move sub+[%f]  vx[%d] \n",sub/2, ctrl->turn.vx );
		} else {
			ctrl->rot.vx = GV_NearTimeF( ctrl->rot.vx, 0, time ) ;
		}
#endif		
		to.vy = GV_NearTimeF( act->ctrl->mov.vy, mov.vy, time ) ;
		if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &to, &act->ctrl->mov,
					     HZX_CHK_ALL, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
			printf("!!!!!!!!!!!!!!!!!WARNING flor through !!!!!\n" ) ;
			FaintFloorThrough( act ) ;
			return 1 ;
		}
		act->ctrl->mov.vy = to.vy ;
		{/* レベル差自殺防止 */
		    int	flag ;

		    flag = HZX_LevelHazardCheck( act->ctrl->hzx_id, &act->ctrl->mov,
						 act->ctrl->hzx_check_type, act->ctrl->flr_flag ) ;
			if ( flag != 0 ) {
				HZX_GetLevelHeight( &act->ctrl->levels[0] ) ; 
			}
			if ( !( 1 & flag ) ) act->ctrl->levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
			if ( !( 2 & flag ) ) act->ctrl->levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
//printf("s2pos[%f][%f][%f]\n",act->ctrl->mov.vx,act->ctrl->mov.vy,act->ctrl->mov.vz ) ;
		}
//printf("s3 level[%f]\n",act->ctrl->levels[0] ) ;
	}
	return 0 ;
}

static	int FaintWakeCheck( act )
ACTION	*act ;
{
	if ( act->bodyp.blood < 0  ) {
		if ( act->headmark2 ) {
			ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
		}
	} else if ( act->bodyp.faint_time < COUNT_VMODE(30) ) {
		if ( (act->headmark2 == ACT_HEADMARK2_FAINT) && (act->bodyp.anesthesia < 0) ) {
			act->bodyp.faint_time = CM_ZZZ_TIME ;
			ENE_SetHeadMark2( act, HEADMARK_ANA, ACT_HEADMARK2_ANES ) ;
			act->sw->eye_anim = EYE_TUBIRI ;
		} else {
			act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
			act->bodyp.capture.capture = NULL ;	
			act->bodyp.capture.flag &= ~CAPTURE_FREE ;
			AT_SetMode( act, ActFaintDown ) ;
			return 1 ;
		}
	}
	return 0 ;
}

static	int FrontCheck( act )
ACTION	*act ;
{
	FMATRIX	mat, *w ;
	static FVECTOR Front = { 0.0, 0.0, 100.0 } ;

	w = &act->body->objs->objs[0].world ;
	DG_SetPos( w ) ;
	DG_MovePos( &Front ) ;
	DG_GetPos( &mat ) ;

printf("m [%f] w [%f] \n", mat.m[3][1], w->m[3][1] ) ;
	if ( mat.m[3][1] > w->m[3][1] ) {
		return 1 ;
	}

	return 0 ;
}

static	void AnyBlood( act )
ACTION	*act ;
{
	int	n ;
	extern void PutShieldEffectPos_BL( OBJECT * ,int ) ;

	if ( act->bodyp.type & ENE_TYPE_SHIELD ) {
		n = act->bodyp.n_damobj ;
		if ( n<0 ) n=0 ;
		if ( n>20 ) n=20 ;
		PutShieldEffectPos_BL( act->sub_obj , n ) ;
	}
}

#if 0
static	void Vomit( act )
ACTION	*act ;
{
	extern void TAKABE_OozeBloodAdd( void *, int, FVECTOR *, float ) ;
	FMATRIX	mat, *w ;
	static FVECTOR Front = { 0.0, 5.0, 100.0 } ;
	FVECTOR	pos ;

	w = &act->body->objs->objs[12].world ;
	DG_SetPos( w ) ;
	DG_MovePos( &Front ) ;
	DG_GetPos( &mat ) ;

	KR_FMatToFvec( &mat, &pos ) ;
	printf("pos[%f][%f][%f]\n",pos.vx,pos.vy,pos.vz ) ;
	printf("mov[%f][%f][%f]\n",act->ctrl->mov.vx,act->ctrl->mov.vy,act->ctrl->mov.vz ) ;
	TAKABE_OozeBloodAdd( act->oozeblood, 12, &pos, 1000.0F ) ;
}
#endif

/* 全てのダメージフラグをクリア */
static void DamageFlagClear( ACTION *act )
{
	TARGET	*def ;
	int i ;
	
	def = &(act->bodyp.def_child1[0]) ;
	for( i=0; i<PTARGET_LEVEL1_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def->weapon_type = 0 ;
		def++ ;
	}
	def = &(act->bodyp.def_child2[0]) ;
	for( i=0; i<PTARGET_LEVEL2_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def->weapon_type = 0 ;
		def++ ;
	}
	def = &(act->bodyp.def_child3[0]) ;
	for( i=0; i<PTARGET_LEVEL3_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def->weapon_type = 0 ;
		def++ ;
	}
	def = &(act->bodyp.deftrg) ;
	def->damaged = FLAG_CLEAR ;
	def->weapon_type = 0 ;

	def = &(act->bodyp.pushtrg) ;
	def->damaged = FLAG_CLEAR ;

}

static void CaptureFlagClear( ACTION *act )
{
	if ( act->bodyp.capture.capture != NULL ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
		act->bodyp.capture.capture = NULL ;	
	}
	/* C4落とす意味もある */
	act->bodyp.capture.flag = 0 ;
}

static void UnsetLeaveC4CaptureFlag( ACTION *act )
{
	/* C4を残す */
	act->bodyp.capture.flag &= CAPTURE_C4EXIST ;
}
static void LeaveC4CaptureFlagClear( ACTION *act )
{
	if ( act->bodyp.capture.capture != NULL ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
		act->bodyp.capture.capture = NULL ;	
	}
	/* C4を残す */
	UnsetLeaveC4CaptureFlag( act ) ;
}


static void DamageCaptureFlagClear( ACTION *act )
{
	DamageFlagClear( act ) ;
	CaptureFlagClear( act ) ;
}

static void LeaveC4DamageCaptureFlagClear( ACTION *act )
{
	DamageFlagClear( act ) ;
	LeaveC4CaptureFlagClear( act ) ;
}

/* 体験版直前バグ対処 */
static void _DamageCaptureFlagClear( ACTION *act )
{
	DamageFlagClear( act ) ;
	if ( act->bodyp.capture.capture != NULL ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
		act->bodyp.capture.capture = NULL ;	
		act->bodyp.capture.flag = 0 ;
	}
}

static int ChildTargetCheck( child, n )
TARGET	*child ;
int		n ;
{
	int i ;

	for( i=0; i<n; i++ ) {
		if ( TARGET_POWER & child->damaged ) {
			return i ;
		} else {
			child->weapon_type = 0 ;
		}
		child++ ;
	}
	return -1 ;
}

static void DownAnesHit( act, def )
ACTION	*act ;
TARGET	*def ;
{
	int faint ;

	if ( act->headmark2 == ACT_HEADMARK2_FAINT ) {
#ifdef ZZZ_VER_Z
		act->bodyp.faint_time = COUNT_VMODE(30)-1 ;
#endif
		act->bodyp.anesthesia = -1 ;
	} else {
		faint = act->bodyp.faint_time + CM_ZZZ_TIME /2 ;
		if ( faint > CM_ZZZ_TIME ) faint = CM_ZZZ_TIME ;
		act->bodyp.faint_time = faint ;
	}
	ENE_SetNeedl( act->body, act->bodyp.n_damobj, &act->bodyp.damtrg->hit ) ;
//	act->adj_piku_time = ADJ_PIKU_TIME ;
	act->status |= ACT_STATUS_TARGET_SKIP ;	/* このフレームではダメージを受けない */
	act->status |= ACT_STATUS_IK_PIKU ;
	SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_ENEMY_SLEEP ) ;
}

static int	DownTrgCheck( act )
ACTION	*act ;
{
	extern void TAKABE_OozeBloodAdd( void *, int, FVECTOR *, float ) ;
	CAPTURE_TARGET	*cap ;
	TARGET	*def ;
	PTARGET_INFO	*pinfo ;
	int		child_num ;
	long64	weapon ;

	def = &(act->bodyp.def_child1[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL1] ;

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		act->keep_mot = EM_dam_out ;
		/* このフレームではダメージを受けない */
		AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
		AT_SetMode( act, ActDownDamage ) ;

		return 1 ;
	}

	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL1_NUM )) >= 0 ) {
printf("down child1 weapontype [%lx]\n",def->weapon_type);
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = pinfo->obj_num ;

		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
#ifdef DAM_BUG	
		_DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif

		if ( weapon & WP_M92 ) {
			DownAnesHit( act, def ) ;
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;

			return 0 ;
		} else if ( weapon & WP_THROWG ) {
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else {
			if ( weapon & (WP_BULLET) ) {
				act->bodyp.dam_level_num[PTARGET_LEVEL1] ++ ;
				act->bodyp.life = 0 ;
				Blood( act, 0, 1 ) ;
				TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
			}
			act->keep_mot = pinfo->dam_motion ;
			/* このフレームではダメージを受けない */
			AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
			AT_SetMode( act, ActDownDamage ) ;

			return 1 ;
		}
	}

	def = &(act->bodyp.def_child2[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL2] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL2_NUM )) >= 0 ) {
printf("down child2 weapontype [%lx]\n",def->weapon_type);
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = pinfo->obj_num ;

		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
#ifdef DAM_BUG	
		_DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif

		if ( weapon & WP_M92 ) {
			DownAnesHit( act, def ) ;
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			return 0 ;
		} else if ( weapon & WP_THROWG ) {
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else if ( weapon & (WP_BULLET) ) {
			act->bodyp.dam_level_num[PTARGET_LEVEL2] ++ ;
			act->bodyp.life -= USP_DAM/2 ;
			Blood( act, 0, 1 ) ;
			TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
			act->keep_mot = pinfo->dam_motion ;
			if ( child_num == PTARGET_LEGR1 || child_num == PTARGET_LEGR2 ) {
				act->bodyp.pbreak |= PBREAK_LEG_R ;
				if ( act->bodyp.pbreak & PBREAK_LEG_L ) {
					act->bodyp.life = 0 ;	/* 両足撃たれたら死亡 */
				}
				act->keep_mot = EM_dam_legs ;
			}
			if ( child_num == PTARGET_LEGL1 || child_num == PTARGET_LEGL2 ) {
				act->bodyp.pbreak |= PBREAK_LEG_L ;
				if ( act->bodyp.pbreak & PBREAK_LEG_R ) {
					act->bodyp.life = 0 ;	/* 両足撃たれたら死亡 */
				}
				act->keep_mot = EM_dam_legs ;
			}
			if ( child_num == PTARGET_ARMR1 || child_num == PTARGET_ARMR2 ) {
				act->bodyp.pbreak |= PBREAK_ARM_R ;
				if ( act->bodyp.pbreak & PBREAK_ARM_L ) {
					act->bodyp.life = 0 ;	/* 両手撃たれたら死亡 */
				}
				act->keep_mot = EM_dam_bomb_b ;
			}
			if ( child_num == PTARGET_ARML1 || child_num == PTARGET_ARML2 ) {
				act->bodyp.pbreak |= PBREAK_ARM_L ;
				if ( act->bodyp.pbreak & PBREAK_ARM_R ) {
					act->bodyp.life = 0 ;	/* 両手撃たれたら死亡 */
				}
				act->keep_mot = EM_dam_bomb_b ;
			}

			/* このフレームではダメージを受けない */
			AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
			AT_SetMode( act, ActDownDamage ) ;

			return 1 ;
		} else {
			act->keep_mot = EM_dam_bomb_b ;
			/* このフレームではダメージを受けない */
			AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
			AT_SetMode( act, ActDownDamage ) ;

			return 1 ;
		}
	}

	def = &(act->bodyp.def_child3[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL3] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL3_NUM )) >= 0 ) {
printf("down child3 weapontype [%lx]\n",def->weapon_type);
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = pinfo->obj_num ;

		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
#ifdef DAM_BUG	
		_DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif

		if ( weapon & WP_M92 ) {
			DownAnesHit( act, def ) ;
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;

			return 0 ;
		} else if ( weapon & WP_THROWG ) {
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else {
			if ( weapon & (WP_BULLET) ) {
				act->bodyp.dam_level_num[PTARGET_LEVEL3] ++ ;
				act->bodyp.life -= USP_DAM ;
				Blood( act, 0, 1 ) ;
				TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
			}
			if ( act->bodyp.life <= 0 ) {
				act->keep_mot = EM_dam_out ;
			} else {
				act->keep_mot = ENE_NokezoriDamMotion(act) ;
			}
			/* このフレームではダメージを受けない */
			AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
			AT_SetMode( act, ActDownDamage ) ;

			return 1 ;
		}
	}


	def = &(act->bodyp.deftrg) ;
	if ( TARGET_POWER & def->damaged ) {
printf("down oya weapontype [%lx]\n",def->weapon_type);
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = HUMAN21_MUNE ;
//printf("def weapon type [%x]  \n",def->weapon_type);

		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
#ifdef DAM_BUG	
		_DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif

		if ( weapon & WP_M92 ) {
			DownAnesHit( act, def ) ;
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;

			return 0 ;
		} else if ( weapon & (WP_MECABREAK|WP_STUNGRENADE|WP_STUNFAR|WP_WALLCRASH) ) {
			return 0 ;
		} else if ( weapon & (WP_THROWG) ) {
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;

		} else if ( weapon & (WP_COLDSPRAY) ) {
			/* このフレームではダメージを受けない */
			AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
#if 0
			act->keep_mot = EM_dam_bomb_b ;
			AT_SetMode( act, ActDownDamage ) ;
			return 1 ;
#else
			if ( !(*(act->ene_status) & ENE_STATUS_EVER_ZZZ) ) {
				if ( act->bodyp.faint_time > COUNT_VMODE(100) ) {
					act->bodyp.faint_time -= COUNT_VMODE(100) ;
				}
			}
         // Armature fix:
         // if running at 30, time = 1 is not enough to show the reaction
         // so I have to make it 3 or 4 and then reduce the size of the reaction
         // fixes MGSTWO-3181
         if (AS_IsRunningAt30Fps())
         {
            if (act->adj_piku_time == 0)
            {
               act->adj_piku_time = 3 ;
               // index into table of rotation amount of spine
               act->adj_piku_value = 1 ;
            }
         }
         else
         {
            act->adj_piku_time = 1 ;
         }
			return 0 ;
#endif
		} else {
			if ( weapon & (WP_BULLET) ) {
				extern void TAKABE_OozeBloodAdd( void *, int, FVECTOR *, float ) ;
				act->bodyp.dam_level_num[PTARGET_LEVEL3] ++ ;
				act->bodyp.life -= USP_DAM ;
				act->keep_mot = ( act->bodyp.life <= 0 ) ? EM_dam_out : EM_dam_bomb_b ;
				Blood( act, 0, 1 ) ;
				TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
			} else if ( weapon & (WP_PUNCHR|WP_PUNCHL|WP_KICK|WP_KICK1|WP_STUNFAR|WP_STUNGRENADE) ) {
				act->keep_mot = EM_dam_bomb_b ;
			} else if ( weapon & (WP_STEAM) ) {
#ifdef JIK_STEAM_DAM
				act->bodyp.life -= USP_DAM/2 ;
#endif
				act->keep_mot = EM_dam_bomb_b ;
				if ( !(*(act->ene_status) & ENE_STATUS_EVER_ZZZ) ) {
					if ( act->bodyp.faint_time > COUNT_VMODE(1300) ) {
						act->bodyp.faint_time -= COUNT_VMODE(1200) ;
					}
				}
			} else if ( weapon & WP_STAMP ) {	/* 味方に起こされる */
				GM_SeSetMode( SD_E_ATARU02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				act->keep_mot = EM_dam_bomb_b ;
				if ( weapon & WP_NOPLAYER ) {
					if ( !(*(act->ene_status) & ENE_STATUS_EVER_ZZZ) ) {
						if ( act->bodyp.faint_time > COUNT_VMODE(100) ) {
							act->bodyp.faint_time = COUNT_VMODE(100) ;
						}
					}
				}
			} else if ( weapon & (WP_BLAST) ) {
				act->keep_mot = EM_dam_bomb_b ;
				if ( GM_GameStatus & STATE_VR_ANOTHER ) {
					if ( COM_StageKind( ) & ENE_STAGE_VRBOM_BIGDAM ) {
						act->bodyp.life -= VR_BOMB_BIGDAM ;
					} else {
						act->bodyp.life -= VR_BOMB_DAM ;
					}
				} else {
					act->bodyp.life -= BOMB_DAM ;
				}
			} else {
#if 1
				act->keep_mot = EM_dam_bomb_b ;
#else
				act->bodyp.life -= USP_DAM ;
				act->keep_mot = ( act->bodyp.life <= 0 ) ? EM_dam_out : EM_dam_bomb_b ;
#endif
			}
			/* このフレームではダメージを受けない */
			AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
			AT_SetMode( act, ActDownDamage ) ;

			return 1 ;
		}
	}

	cap = &(act->bodyp.capture) ;
    if ( cap->capture != NULL ) { /* 捕まった */
		DamageFlagClear( act ) ;
		act->status |= ACT_STATUS_TARGET_SKIP ;	/* このフレームではダメージを受けない */
		AT_SetMode( act, ActFaintWakeup ) ;
		return 1 ;
	}

	return 0 ;
}

#ifdef JIK
/* ダメージ中にさらにダメージ */
static int	FallTrgCheck( act )
ACTION	*act ;
{
	extern void TAKABE_OozeBloodAdd( void *, int, FVECTOR *, float ) ;
	CAPTURE_TARGET	*cap ;
	TARGET	*def ;
	PTARGET_INFO	*pinfo ;
	int		child_num ;
	long64	weapon ;

	def = &(act->bodyp.def_child1[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL1] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL1_NUM )) >= 0 ) {
//printf("down child1 weapontype [%lx]\n",def->weapon_type);
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = pinfo->obj_num ;


		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
		def->damaged = FLAG_CLEAR ;

		act->adj_piku_time = ADJ_PIKU_TIME ;

		if ( weapon & (WP_BULLET) ) {
			Blood( act, 0, 2 ) ;
			TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
		}

		return 0 ;
	}

	def = &(act->bodyp.def_child2[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL2] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL2_NUM )) >= 0 ) {
//printf("down child2 weapontype [%lx]\n",def->weapon_type);
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = pinfo->obj_num ;

		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
		def->damaged = FLAG_CLEAR ;

		act->adj_piku_time = ADJ_PIKU_TIME ;
		if ( weapon & (WP_BULLET) ) {
			Blood( act, 0, 2 ) ;
			TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
		}

		return 0 ;
	}

	def = &(act->bodyp.def_child3[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL3] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL3_NUM )) >= 0 ) {
//printf("down child3 weapontype [%lx]\n",def->weapon_type);
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = pinfo->obj_num ;

		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
		def->damaged = FLAG_CLEAR ;

		act->adj_piku_time = ADJ_PIKU_TIME ;
		if ( weapon & (WP_BULLET) ) {
			Blood( act, 0, 2 ) ;
			TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
		}

		return 0 ;
	}


	def = &(act->bodyp.deftrg) ;
	if ( TARGET_POWER & def->damaged ) {
//printf("down oya weapontype [%lx]\n",def->weapon_type);
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = HUMAN21_MUNE ;
//printf("def weapon type [%x]  \n",def->weapon_type);

		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
		def->damaged = FLAG_CLEAR ;

		act->adj_piku_time = ADJ_PIKU_TIME ;

		if ( weapon & (WP_BULLET) ) {
			Blood( act, 0, 2 ) ;
			TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
		}

		return 0 ;
	}

	return 0 ;
}
#endif

/*-------------------------------------------------------------------*/
static	void ActFaintWakeup( act, time )
ACTION	*act ;
int		time ;
{
	int	left ;

	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_FAINT|ACT_STATUS_CAPTURE
					|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP  ) ;

	if ( act->CheckPad( act ) > 0 ) return ;

    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		AT_SetActStatus( act, ACT_STATUS_IK_FOOT ) ;
	} else {
		AT_SetActStatus( act, ACT_STATUS_IK_HAND ) ;
	}

	if ( time == 0 ) {
		FVECTOR	vec ;
		int		dir ;

		_sceVu0SubVector(  &vec, &act->bodyp.capture.capture->ctrl->mov, &act->ctrl->mov ) ;
		dir = _FVecDir2( &vec ) ;
		if ( act->down_s == DownFront ) {
		    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
				AT_SetAction( act, 0, EM_dead_carry_body_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir + 2048 ;
				act->tmp_time = COUNT_VMODE(50) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(50) ;
		    }
		} else if ( act->down_s == DownWall ) {
		    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
				AT_SetAction( act, 0, EM_dead_carry_body_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir + 2048 ;
				act->tmp_time = COUNT_VMODE(50) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(50) ;
		    }
		} else {
		    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
				AT_SetAction( act, 0, EM_dead_carry_body_start_b, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(90) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_start_b, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir + 2048 ;
				act->tmp_time = COUNT_VMODE(50) ;
		    }
		}

		ReSetSlopeRotX( act ) ;
	}

//printf(" time[%d]/tmo_time[%d] \n",time,act->tmp_time);
//printf("motion time player[%d] enemy[%d]\n",
//	GM_PlayerBody->m_ctrl->mt3_ctrl[ 0 ].time,act->body->m_ctrl->mt3_ctrl[ 0 ].time ) ;

	if ( FaintCaptureCheck( act ) ) {
//		AT_SetMode( act, ActFaint ) ;
		return ;
	}

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		FaintFloorThrough( act ) ;
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
	    return ;
	}

#if 1
	if ( time < 16 ) {
		if ( FaintSetStepNear( act, 16-time ) ) {
			return ;
		}
	} else {
		left = KR_LeftMotion( &act->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
		if ( FaintSetStep( act, left ) ) {
			return ;
		}
printf( "  left [%d]\n",left) ;
	}
printf( " wakeup time[%d]\n",time) ;
#else
	if ( time < 16 ) {
		if ( FaintSetStepNear( act, 16-time ) ) {
			return ;
		}
	}
	left = KR_LeftMotion( &act->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
	if ( left < 16 ) {
		if ( FaintSetStep( act, left ) ) {
			return ;
		}
	}
#endif

	/* フロアチェック無しは最後にセット */
	if ( time > act->tmp_time ) AT_SetActStatus( act, ACT_STATUS_FLR_OFF ) ;
	AT_SetActStatus( act, ACT_STATUS_WAKEUP ) ;

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		AT_SetMode( act, ActFaintTake ) ;
		return ;
	}
}

static	void ActFaintTake( act, time )
ACTION	*act ;
int		time ;
{
// printf("ActFaintTake[%d]\n",time) ;
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_FAINT|
		ACT_STATUS_CAPTURE|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP  ) ;

	AT_SetActStSt( act, ACT_STST_DRAG ) ;

	if ( act->CheckPad( act ) > 0 ) return ;

    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		AT_SetActStatus( act, ACT_STATUS_IK_FOOT ) ;
	} else {
		AT_SetActStatus( act, ACT_STATUS_IK_HAND ) ;
	}

	if ( time == 0 ) {
		if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		    AT_SetAction( act, 0, EM_dead_carry_body_idle, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				AT_SetAction( act, 0, EM_dead_carry_leg_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_idle_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
	}
	if ( FaintWakeCheck( act ) ) {
		return ;
	}
	if ( FaintCaptureCheck( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		if ( FaintSetStep( act, -1 ) ) {
			return ;
		}
	} else {
		if ( FaintSetStep( act, 0 ) ) {
			return ;
		}
	}

	/* フロアチェック無しは最後にセット */
	AT_SetActStatus( act, ACT_STATUS_FLR_OFF ) ;

	if ( act->bodyp.capture.flag & CAPTURE_MOVE ) {
	    AT_SetMode( act, ActFaintMove ) ;
	}
}

static	void ActFaintMove( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActFaintMove\n");
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_FAINT|ACT_STATUS_CAPTURE
				|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP  ) ;
	AT_SetActStSt( act, ACT_STST_DRAG ) ;

    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		AT_SetActStatus( act, ACT_STATUS_IK_FOOT ) ;
	} else {
		AT_SetActStatus( act, ACT_STATUS_IK_HAND ) ;
	}

	if ( act->CheckPad( act ) > 0 ) return ;

	if ( time == 0 ) {
		if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		    AT_SetAction( act, 0, EM_dead_carry_body_walk, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				AT_SetAction( act, 0, EM_dead_carry_leg_walk_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_walk_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
	}
	if ( FaintWakeCheck( act ) ) {
		return ;
	}
	if ( FaintCaptureCheck( act ) ) {
		return ;
	}

	if ( FaintSetStep( act, 0 ) ) {
		return ;
	}
	if ( !( act->bodyp.capture.flag & CAPTURE_MOVE ) ) {
	    AT_SetMode( act, ActFaintTake ) ;
	}
	/* フロアチェック無しは最後にセット */
	AT_SetActStatus( act, ACT_STATUS_FLR_OFF ) ;
}

static	void ActFaintDown( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_FAINT
				|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP  ) ;
	AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;

	/* したい捨ての為 */
	if ( time < 4 ) AT_SetActStatus( act, ACT_STATUS_CAPTURE ) ;

	if ( act->CheckPad( act ) > 0 ) return ;

	if ( time == 0 ) {
		if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
			AT_SetAction( act, 0, EM_dead_carry_body_end, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				AT_SetAction( act, 0, EM_dead_carry_leg_end_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_end_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
		MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*2.0f ) ;
	}
//printf("faint down time[%d] \n",time);
	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act ) ;
	}
#ifdef PUT_ITEM
	if ( time == COUNT_VMODE(30/2) ) {
		float n ;
		
		n = ( act->bodyp.capture.flag & CAPTURE_HEAD )? 1.0f : 2.0f ;
		KRTH_PutItemProc( &act->item, &act->ctrl->mov, *(act->ene_status), n ) ;
	}
#endif

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
	    return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
#if 1 //抱き上げで速く目覚める実験
		if ( !(*(act->ene_status) & ENE_STATUS_EVER_ZZZ) ) {
			if ( act->bodyp.faint_time > COUNT_VMODE(1300) ){
				act->bodyp.faint_time -= COUNT_VMODE(1200) ;
			}
		}
#endif
	    if ( FrontCheck( act ) ) {
			act->bodyp.capture.flag = CAPTURE_FRONT ;
			act->down_s = DownFront ;
		} else {
			act->bodyp.capture.flag = CAPTURE_BACK ;
			act->down_s = DownBack ;
		}

		if ( act->bodyp.faint_time < COUNT_VMODE(30) ) {
			if ( act->bodyp.blood < 0 ) {
				act->bodyp.life = 0 ;
				act->act_end = 1 ;
				AT_SetMode( act, ActDeath ) ;
			} else if ( act->headmark2 == ACT_HEADMARK2_FAINT && act->bodyp.anesthesia < 0 ) {
				act->bodyp.faint_time = CM_ZZZ_TIME ;
				ENE_SetHeadMark2( act, HEADMARK_ANA, ACT_HEADMARK2_ANES ) ;
			    AT_SetMode( act, ActFaint ) ;
				act->sw->eye_anim = EYE_TUBIRI ;
			} else {
#ifdef LEGS_DAM
			if ( (act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
				act->bodyp.life = 0 ;
				act->act_end = 1 ;
				AT_SetMode( act, ActDeath ) ;
			} else {
				AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
				act->bodyp.faint_time = COUNT_VMODE(30) ;
				AT_SetMode( act, ActWakeup ) ;
			}
#else
				act->bodyp.faint_time = COUNT_VMODE(30) ;
				AT_SetMode( act, ActWakeup ) ;
#endif
			}
		} else {
		    AT_SetMode( act, ActFaint ) ;
		}
	    return ;
	}
}

static	void ActFaintIntoLocker( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_FAINT
				|ACT_STATUS_CAPTURE|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_LOCKER  ) ;

	AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
//	AT_SetActStatus( act, ACT_STATUS_SEG_OFF) ;

	if ( time == 0 ) {
		if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
			AT_SetAction( act, 0, EM_dead_carry_body_put_locker,0,MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				AT_SetAction( act, 0, EM_dead_carry_leg_put_locker_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_put_locker_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
		ReSetSlopeRotX( act ) ;
	}

	if ( !(act->bodyp.capture.flag & CAPTURE_LOCKER) ) { /* ロッカー入れられる */
	    AT_SetMode( act, ActFaintDownLocker ) ;
	    return ;
	}

	if ( ENE_GameStatus & ENE_GMSTATUS_TOILET ) {
		if ( act->c_motion_num[0] == EM_dead_carry_body_put_locker ) {
			if ( time>74 ) {
			    AT_SetMode( act, ActFaintIntoToilet ) ;
			    return ;
			}
		} else if ( act->c_motion_num[0] == EM_dead_carry_leg_put_locker_f ) {
			if ( time>130 ) {
			    AT_SetMode( act, ActFaintIntoToilet ) ;
			    return ;
			}
		} else if ( act->c_motion_num[0] == EM_dead_carry_leg_put_locker_b ) {
			if ( time>138 ) {
			    AT_SetMode( act, ActFaintIntoToilet ) ;
			    return ;
			}
		}
	} else {
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		    AT_SetMode( act, ActFaintStandLocker ) ;
		    return ;
		}
	}
}

static	void ActFaintIntoToilet( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_FAINT
				|ACT_STATUS_CAPTURE|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_LOCKER  ) ;

	AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
//	AT_SetActStatus( act, ACT_STATUS_SEG_OFF) ;

	if ( time == 0 ) {
		AT_SetAction( act, 0, EM_dead_carry_put_toilet,0,MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}

	if ( !(act->bodyp.capture.flag & CAPTURE_LOCKER) ) { /* ロッカー入れられる */
	    AT_SetMode( act, ActFaintDownLocker ) ;
	    return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
	    AT_SetMode( act, ActFaintStandLocker ) ;
	    return ;
	}
}

void ActFaintStandLocker( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_FAINT
				|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_LOCKER  ) ;
	AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
	if ( !(ENE_GameStatus & ENE_GMSTATUS_TOILET) ) {
		AT_SetActStatus( act, ACT_STATUS_INVISIBLE) ;
	}

	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {/* 1320.875 */
		AT_SetAction( act, 0, EM_dead_carry_locker_idle, 0, MOTION_MASK_FULL, 0 ) ;
	}

	if ( !(act->bodyp.capture.flag & CAPTURE_LOCKER) ) {
	    AT_SetMode( act, ActFaintDownLocker ) ;
	    return ;
	}
}

static	void ActFaintDownLocker( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_FAINT
				|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_LOCKER  ) ;

	if ( time < 30 ) {
//		AT_SetActStatus( act, ACT_STATUS_INVISIBLE ) ;
	}

	if ( time > COUNT_VMODE(140) ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
		AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
	} else {
		AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
	}

	if ( time == 0 ) {
		AT_SetAction( act, 0, EM_dead_carry_locker_surprise, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}
	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act ) ;
	}

#ifdef PUT_ITEM
	if ( time == COUNT_VMODE(130) ) {
		float n ;

		n = ( act->bodyp.capture.flag & CAPTURE_HEAD )? 1.0f : 2.0f ;
		KRTH_PutItemProc( &act->item, &act->ctrl->mov, *(act->ene_status), n ) ;
	}
#endif

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
	    if ( FrontCheck( act ) ) {
			act->bodyp.capture.flag = CAPTURE_FRONT ; /* 一度つかまれると仰向け */
			act->down_s = DownFront ;
		} else {
			act->bodyp.capture.flag = CAPTURE_BACK ;
			act->down_s = DownBack ;
		}
		if ( act->bodyp.faint_time < COUNT_VMODE(30) ) {
			if ( act->bodyp.blood < 0 ) {
				act->bodyp.life = 0 ;
				act->act_end = 1 ;
				AT_SetMode( act, ActDeath ) ;
			} else if ( act->headmark2 == ACT_HEADMARK2_FAINT && act->bodyp.anesthesia < 0 ) {
				act->bodyp.faint_time = CM_ZZZ_TIME ;
				ENE_SetHeadMark2( act, HEADMARK_ANA, ACT_HEADMARK2_ANES ) ;
			    AT_SetMode( act, ActFaint ) ;
				act->sw->eye_anim = EYE_TUBIRI ;
			} else {
#ifdef LEGS_DAM
			if ( (act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
				act->bodyp.life = 0 ;
				act->act_end = 1 ;
				AT_SetMode( act, ActDeath ) ;
			} else {
				AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
				act->bodyp.faint_time = COUNT_VMODE(30) ;
				AT_SetMode( act, ActWakeup ) ;
			}
#else
				act->bodyp.faint_time = COUNT_VMODE(30) ;
				AT_SetMode( act, ActWakeup ) ;
#endif
			}
		} else {
		    AT_SetMode( act, ActFaint ) ;
		}
	    return ;
	}
}

static	void ActHangAvoid( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_PUSHT_SKIP ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_avoid_hang, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;

		act->bodyp.capture.capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
		act->bodyp.capture.capture = NULL ;	
#ifdef LEAVE_C4_DAMAGE_CLEAR
		UnsetLeaveC4CaptureFlag( act ) ;
#else
		act->bodyp.capture.flag = 0 ;
#endif
//MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*16.0f ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}

static	void ActHang( act, time )
ACTION	*act ;
int		time ;
{
#ifdef HANG_SHILD
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TRG_PLAYER
			|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_CAPTURE|ACT_STATUS_PUSHT_SKIP ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;
#endif
	AT_SetActStatus( act, ACT_STATUS_FLR_OFF ) ;
	AT_SetActStSt( act, ACT_STST_HANG ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_nom_iya_tie, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		if ( act->bodyp.faint == 1 ) act->sw->eye_anim = EYE_KURURI ;
		act->bodyp.faint -= HANG_FAINT ;
		act->tmp_time = 0 ;
		GM_SeSetMode( SD_V_GBSNEC01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;/* 「あうっ」 */
	}

	if ( time == COUNT_VMODE(15) ) {
		if ( act->bodyp.faint > 0 ) {
			act->sw->eye_anim = EYE_DAMAGE ;
		} else {
			act->sw->eye_anim = EYE_SIROME ;
		}
	}

	/* 首絞め専用のチェックダメージをする */
	if ( Hang_CheckDamage( act ) ) {
		return ;
	}

	HangSetStep( act ) ;

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		AT_SetMode( act, ActHangIdle ) ;
		return ;
	}

}

static	void ActHangIdle( act, time )
ACTION	*act ;
int		time ;
{
#ifdef HANG_SHILD
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TRG_PLAYER
			|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_CAPTURE|ACT_STATUS_PUSHT_SKIP ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;
#endif
	AT_SetActStatus( act, ACT_STATUS_FLR_OFF ) ;
	AT_SetActStSt( act, ACT_STST_HANG ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_nom_iya_idle, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( Hang_CheckDamage( act ) ) {
		return ;
	}

	HangSetStep( act ) ;

	if ( act->tmp_time > HANG_STRUGGLE_TIME  ) {
		if ( act->bodyp.faint > 0 && act->bodyp.anesthesia >= 0 && act->bodyp.blood >= 0) {
			AT_SetMode( act, ActHangStruggle ) ;
			return ;
		}
	}

	if ( act->tmp_time == (HANG_STRUGGLE_TIME+HANG_OFF_TIME  ) ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
	}

	if ( act->bodyp.capture.flag & CAPTURE_MOVE ) {
		AT_SetMode( act, ActHangMove ) ;
		return ;
	}


	act->tmp_time ++ ;
}

static	void ActHangMove( act, time )
ACTION	*act ;
int		time ;
{
#ifdef HANG_SHILD
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TRG_PLAYER
			|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_CAPTURE|ACT_STATUS_PUSHT_SKIP ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;
#endif
	AT_SetActStatus( act, ACT_STATUS_FLR_OFF ) ;
	AT_SetActStSt( act, ACT_STST_HANG ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_nom_iya_walk, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( Hang_CheckDamage( act ) ) {
		return ;
	}

	HangSetStep( act ) ;

	if ( act->tmp_time > HANG_STRUGGLE_TIME  ) {
		if ( act->bodyp.faint > 0 && act->bodyp.anesthesia >= 0 && act->bodyp.blood >= 0 ) {
			AT_SetMode( act, ActHangStruggle ) ;
			return ;
		}
	}

	if ( act->tmp_time == (HANG_STRUGGLE_TIME+HANG_OFF_TIME) ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
	}

	if ( !(act->bodyp.capture.flag & CAPTURE_MOVE) ) {
		AT_SetMode( act, ActHangIdle ) ;
		return ;
	}

	{/* レベル差自殺防止 */
	    int	flag ;

	    flag = HZX_LevelHazardCheck( act->ctrl->hzx_id, &act->ctrl->mov,
					 act->ctrl->hzx_check_type, act->ctrl->flr_flag ) ;
		if ( flag != 0 ) {
			HZX_GetLevelHeight( &act->ctrl->levels[0] ) ; 
		}
		if ( !( 1 & flag ) ) act->ctrl->levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
		if ( !( 2 & flag ) ) act->ctrl->levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
//printf("s2pos[%f][%f][%f]\n",act->ctrl->mov.vx,act->ctrl->mov.vy,act->ctrl->mov.vz ) ;
	}

	act->tmp_time ++ ;
}

static	void ActHangStruggle( act, time )
ACTION	*act ;
int		time ;
{
#ifdef HANG_SHILD
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TRG_PLAYER
			|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_CAPTURE|ACT_STATUS_PUSHT_SKIP ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;
#endif
	AT_SetActStatus( act, ACT_STATUS_FLR_OFF ) ;
	AT_SetActStSt( act, ACT_STST_HANG ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_nom_iya_resist, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		if( act->name_id->motion != 0 ) {
			GM_VarSetVibration( NULL, act->name_id->motion, 0, &act->sw->vibration ) ;
		}
		SET_FLAG( act->sw->vibration, VAR_FLAG_LOOP ) ; 
	}

	if ( Hang_CheckDamage( act ) ) {
		SET_FLAG( act->sw->vibration, VAR_FLAG_PLAY_STOP ) ; 
		return ;
	}

	HangSetStep( act ) ;

	if ( time == HANG_OFF_TIME  ) {
		SET_FLAG( act->sw->vibration, VAR_FLAG_PLAY_STOP ) ; 
		act->bodyp.capture.capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
		act->bodyp.capture.capture = NULL ;	
#ifdef LEAVE_C4_DAMAGE_CLEAR
		UnsetLeaveC4CaptureFlag( act ) ;
#else
		act->bodyp.capture.flag = 0 ;
#endif
/*ACTが変わり無敵になる場合は変わる前のフレームから無敵にする事！！*/
		AT_SetActStatus( act, ACT_STATUS_PUSHT_SKIP ) ;
		AT_SetMode( act, ActHangEscape ) ;
		return ;
	}
}

static	void ActHangEscape( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_CAPTURE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_nom_iya_escape, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}
	if ( time == 0 || time == 16 || time == 32) {
//		COM_HangVibration( ) ;
	}

	if ( time < COUNT_VMODE(30) ) {
		/* プッシュ判定無し */
		AT_SetActStatus( act, ACT_STATUS_PUSHT_SKIP ) ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_MABATAKI ;
		return ;
	}

	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		act->sw->eye_anim = EYE_MABATAKI ;

		return ;
	}
}

static	void ActHangFree( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_CAPTURE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_iya_release, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( time < COUNT_VMODE(30) ) {
		/* プッシュ判定無し */
		AT_SetActStatus( act, ACT_STATUS_PUSHT_SKIP ) ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_MABATAKI ;
		return ;
	}

	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		act->sw->eye_anim = EYE_MABATAKI ;

		return ;
	}
}

static	void ActHangFall( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
				|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_CAPTURE ) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_iya_release_down, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act ) ;
	}
	if ( time > COUNT_VMODE(40) ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
		AT_SetActStatus( act, ACT_STATUS_DOWN ) ;
	}

	if ( time > COUNT_VMODE(127) ) {
		AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
	}

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->down_s = DownFront ;
		AT_SetMode( act, endamact_ActDown ) ;

		return ;
	}
}

void ActHangDie( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_CAPTURE ) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_nom_iya_hang, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->bodyp.life = 0 ;
		act->sw->eye_anim = EYE_SIROME ;
		/* 「あうっ」 */
		GM_SeSetMode( SD_V_GBSNEB01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		act->bodyp.last_weapon = (WP_PLAYER|WP_HANG) ;
	}

	if ( time > COUNT_VMODE(110) ) {
		AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
	}

	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act ) ;
	}
	if ( time > COUNT_VMODE(80) ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		if ( LevelDeathCheck( act ) ) {
			act->bodyp.life = 0 ;
			AT_SetMode( act, ActDropp ) ;
			return ;
		}
		act->down_s = DownFront ;
		act->act_end = 1 ;
		AT_SetMode( act, ActDeath ) ;

		return ;
	}
}

static	void ActThrow( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act ) ;
	}
	if ( time > COUNT_VMODE(50) ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
	}

	if ( time == 0 ) {
//		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		AT_SetAction( act, 0, EM_dam_throw, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	    act->ctrl->turn.vy = 
		act->bodyp.capture.capture->ctrl->rot.vy  ;	    
		act->bodyp.faint -= THROW_FAINT ;
		act->sw->eye_anim = EYE_DAMAGE ;
	}
#ifdef BODY_ATTACK
	if( time>COUNT_VMODE(40) && time<COUNT_VMODE(50) ) ENE_SetOffenseTargetEne( act, 1 ) ;
#endif

	if ( time == COUNT_VMODE(50) ) GM_SeSetMode( SD_V_GBSDWN01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;

	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( time > COUNT_VMODE(50) ) {
		AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->bodyp.capture.capture = NULL ;	
#ifdef LEAVE_C4_DAMAGE_CLEAR
		UnsetLeaveC4CaptureFlag( act ) ;
#else
		act->bodyp.capture.flag = 0 ;
#endif
		act->down_s = DownFront ;
		AT_SetMode( act, endamact_ActDown ) ;
#ifdef KIZETU_KOE
		if ( act->bodyp.faint <= 0 ) {
			GM_SeSetMode( SD_V_GBSSLP01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
#endif
printf("Act throw correct[%d] rot[%d]\n",act->body->m_ctrl->rot_correct,act->ctrl->rot.vy);

		return ;
	}
}

#define DEATH_RSPHERE 400
#define DEF_RSPHERE 400
#define NEAR_WALL_RSPHERE 200

static	void ActSleepDown( act, time )
ACTION	*act ;
int		time ;
{
	int	dir ;

	AT_SetActStatus( act, ACT_STATUS_FALL|ACT_STATUS_HOMING_SKIP|ACT_STATUS_TARGET_SKIP|
				ACT_STATUS_SLEEP|ACT_STATUS_EYE_CLOSE ) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;

//printf( " [%d]:slope Rot [%d]\n",time,act->ctrl->turn.vx);	
	if ( time > COUNT_VMODE(60) ) {	/* 前のモーションが腰を落としていた場合の対処 */
		if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
			SetSlopeRotX( act ) ;
		}
		if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
			AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
		}
	}

	switch( act->keep_mot ) {
		case EM_sleep_b :
			if ( time > COUNT_VMODE(90) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
			if ( time > COUNT_VMODE(115) ) {
				AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
				CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
			}
		break ;
		case EM_sleep_f :
			if ( time > COUNT_VMODE(190) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
			if ( time > COUNT_VMODE(98) ) {
				AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
				CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
			}
		break ;
	}

	if ( time == 0 ) {
		if ( act->keep_mot == EM_sleep_b ) {
			AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		}
		act->sw->eye_anim = EYE_DAMAGE ;
		GM_SeSetMode( SD_V_GBSSLP01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	switch ( act->keep_mot ) {
		case EM_sleep_b :
			if ( time > COUNT_VMODE(100) ) {
				if ( ( dir = KR_GetDownDir( act->ctrl, act->ctrl->turn.vy ) ) > 0 ) {
//					act->ctrl->turn.vy = dir ;
				}
			}
			break ;
	}

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->bodyp.faint = 0 ;/*気絶状態にする*/
		AT_SetMode( act, endamact_ActDown ) ;
		return ;
	}
}

static	void ActFall( act, time )
ACTION	*act ;
int		time ;
{
	int dir ;

//printf("ActFall[%d]\n",time ) ;

	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;
#ifndef JIK
	AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
#else
	if ( act->bodyp.life > 0 ) {
		AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
	}
#endif

	if ( time < 30 ) AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/


	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act ) ;
		AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
	} else {
		if ( act->keep_mot == EM_dam_out_b_cap01 ) {
			if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) > 800 ) {
				if ( act->bodyp.life <= 0 ) {
					if ( StairCheck( act ) ) {
						AT_SetMode( act, ActStairFallStart ) ;
#ifdef JIK
						AT_UnSetAllChildTargetClass( act, TARGET_THROUGH ) ;
#endif
						return ;
					}
				}
			}
		}
	}

	if ( FenceCheck( act ) ) {
		AT_SetMode( act, ActFenceFall ) ;
#ifdef JIK
		AT_UnSetAllChildTargetClass( act, TARGET_THROUGH ) ;
#endif
		return ;
	}

	if ( CliffCheck( act ) ) {
		AT_SetMode( act, ActCliffFall ) ;
		return ;
	}

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( time == 0 ) {
printf( " fall mot [%d]\n",act->keep_mot );
		if ( act->keep_mot == EM_dam_bomb_b || act->keep_mot == EM_dam_legs ) {
			AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		}
		act->sw->eye_anim = EYE_DAMAGE ;

		switch ( act->keep_mot ) {
			case EM_dam_out :
				act->tmp_time = COUNT_VMODE(30) ;
printf( " fall ik time %d\n",act->tmp_time );
				break ;
			case EM_dam_out_f :
				act->tmp_time = COUNT_VMODE(160) ;
				break ;
		}
		if ( act->bodyp.life > 0 ) {
			GM_SeSetMode( SD_V_GBSNEC01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_V_GBSOUT01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
#ifdef JIK
		if ( act->bodyp.life <= 0 ) {
			AT_SetTargetClass( act, TARGET_THROUGH ) ;
			AT_SetAllChildTargetClass( act, TARGET_THROUGH ) ;
		}
#endif
	}

	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
	}
#ifdef JIK
FallTrgCheck( act ) ;
#endif

	switch ( act->keep_mot ) {
		case EM_dam_out :
			if ( time > COUNT_VMODE(10) ) {
				if ( ( dir = KR_GetDownDir( act->ctrl, act->ctrl->turn.vy ) ) > 0 ) {
					act->ctrl->turn.vy = dir ;
				}
			}
			break ;
		case EM_dam_out_f :
			if ( time > COUNT_VMODE(100) ) {
				if ( ( dir = KR_GetDownDir( act->ctrl, act->ctrl->turn.vy ) ) > 0 ) {
					act->ctrl->turn.vy = dir ;
				}
			}
			break ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		if ( 	act->bodyp.type & ENE_TYPE_SHIELD ) {
			/* 盾兵は部位ダメージ無し */
			//ENE_ClearPDamage( act ) ;
			ENE_ClearPDamageMask( act ,PBREAK_LEGS) ;
		}
		if ( act->bodyp.type & ENE_TYPE_NO_PBREAK ){
			ENE_ClearPDamage( act ) ;
		}

		AT_SetMode( act, endamact_ActDown ) ;
#ifdef KIZETU_KOE
	if ( act->bodyp.faint <= 0 ) {
		GM_SeSetMode( SD_V_GBSSLP01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
	}
#endif

printf(" act fall end\n");
#ifdef JIK
		AT_UnSetAllChildTargetClass( act, TARGET_THROUGH ) ;
#endif
		return ;
	}
}

static	void ActDirFall( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActDirFall[%d]\n",time ) ;


	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;
	if ( time < 30 ) AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act ) ;
	}
	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
		AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
	}

	if ( time == 0 ) {
		int dir ;
		
//		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		AT_ReSetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;

		dir = GV_VecDir2( &act->bodyp.damtrg->power->force ) ;
//ゲーム的にランダムやめる		dir += KR_RandS( 256 ) ;
		act->ctrl->turn.vy  = dir ;
//printf( "dir fall  turn[%d] mot[%d]\n ",act->ctrl->turn.vy, act->keep_mot ) ;
		if ( (act->keep_mot == EM_dam_bomb_f) || 
			 (act->keep_mot == EM_dam_bomb_fly_b) ||
			 (act->keep_mot == EM_dam_kick_t) ) {
			act->ctrl->turn.vy += 2048 ;
			act->ctrl->rot.vy = act->ctrl->turn.vy ;
//printf( "turn change futtobi[%d] \n ",act->ctrl->turn.vy ) ;
		}
		if( time < COUNT_VMODE(30) ) {
			AT_SetActStatus( act, ACT_STATUS_SPHERE_200 ) ;
		}
		act->sw->eye_anim = EYE_DAMAGE ;

		if ( act->keep_mot == EM_dam_bomb_fly_b || 
			 act->keep_mot == EM_dam_bomb_fly_f ) {
			BombDamageEffect( act ) ;
		}
		if ( act->bodyp.life > 0 ) {
			GM_SeSetMode( SD_V_GBSDWN01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_V_GBSOUT01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
		GM_SetNoise( NOISE_SCREAM , &act->ctrl->mov, act->ctrl->map ) ;
	}

	switch ( act->keep_mot ) {
		case EM_dam_kick_t :
			if ( time < COUNT_VMODE(14) ) AT_SetActStatus( act, ACT_STATUS_VANIME_HEAD ) ;
#ifdef BODY_ATTACK
			if( time>COUNT_VMODE(10) && time<COUNT_VMODE(30) ) ENE_SetOffenseTargetEne( act, 1 ) ;
#endif
			break ;
	}

	if ( FenceCheck( act ) ) {
		AT_SetMode( act, ActFenceFall ) ;
		return ;
	}

	if( time < COUNT_VMODE(15) ) {
		if( WallCheck( act ) ) {
			AT_SetMode( act, ActWallClash ) ;
			return ;
		}
	}

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		if ( 	act->bodyp.type & ENE_TYPE_SHIELD ) {
			ENE_ClearPDamageMask( act ,PBREAK_LEGS) ;
		}
		if ( 	act->bodyp.type & ENE_TYPE_NO_PBREAK){
			ENE_ClearPDamage( act ) ;
		}
		AT_SetMode( act, endamact_ActDown ) ;
#ifdef KIZETU_KOE
	if ( act->bodyp.faint <= 0 ) {
		GM_SeSetMode( SD_V_GBSSLP01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
	}
#endif
		return ;
	}
//printf(" kick dir = %d \n",act->ctrl->rot.vy ) ;
}

static	void ActWallClash( act, time )
ACTION	*act ;
int		time ;
{
#if 0
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_DAMAGE|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE  ) ;
	if ( act->CheckDamage( act ) ) {
			act->keep_mot = EM_dam_kick_t ;
			AT_SetMode( act, ActDirFall ) ;
				return ;
	}
#else
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE) ;
#endif

	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time > COUNT_VMODE(2) ) {
		AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
	}

	if ( time == 0 ) {
//		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		AT_ReSetAction( act, 0, EM_dam_wall_move_d, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		/* 強制的に方向補正をしているので、モーション自動補正はスキップ */
		act->body->m_ctrl->rot_correct = 0;
	}

	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		AT_SetMode( act, ActWallFall ) ;
		return ;
	}
//printf(" kick dir = %d \n",act->ctrl->rot.vy ) ;
}

static	void ActWallFall( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActWallFall[%d]\n",time ) ;

	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE
					|ACT_STATUS_TARGET_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE) ;
	if ( act->bodyp.life <= 0 ) {
//		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
	}

	if ( time < COUNT_VMODE(30) ) AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
//		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		AT_SetAction( act, 0, EM_dam_wall_down, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}

	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->down_s = DownWall ;
		AT_SetMode( act, endamact_ActDown ) ;
#ifdef KIZETU_KOE
		if ( act->bodyp.faint <= 0 ) {
			GM_SeSetMode( SD_V_GBSSLP01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
		}
#endif
		return ;
	}
//printf(" kick dir = %d \n",act->ctrl->rot.vy ) ;
}

static	void ActFenceFall( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActFenceFall[%d]\n",time ) ;

	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_SEG_OFF) ;
	AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
	AT_SetActStSt( act, ACT_STST_FALL_DOWN ) ;

	if ( time < COUNT_VMODE(30) ) AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		if ( act->down_s == DownFront ) {
			AT_ReSetAction( act, 0, EM_dam_fence_f_start, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			AT_ReSetAction( act, 0, EM_dam_fence_start, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		}
		act->ctrl->step.vy = 0.0F ;
		act->ctrl->grounded &= ~1 ; /* 床チェックを解除するので０にしておく */
		
		act->body->m_ctrl->rot_correct = 0 ;	/* 壁方向固定のため方向転換フラグは無視する */
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
//	if ( time == 250 ) {
		AT_SetMode( act, ActFallDown ) ;
		return ;
	}
}

static	void ActCliffFall( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActCliffFall[%d]\n",time ) ;

	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_SEG_OFF) ;
	AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
	AT_SetActStSt( act, ACT_STST_FALL_DOWN ) ;
	if ( time < COUNT_VMODE(30) ) AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		if ( act->down_s == DownFront ) {
			AT_ReSetAction( act, 0, EM_dam_cliff_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			AT_ReSetAction( act, 0, EM_dam_cliff_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		}
		act->ctrl->step.vy = 0.0F ;
		act->ctrl->grounded &= ~1 ; /* 床チェックを解除するので０にしておく */
		
		act->body->m_ctrl->rot_correct = 0 ;	/* 壁方向固定のため方向転換フラグは無視する */
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		AT_SetMode( act, ActFallDown ) ;
		return ;
	}
}

static	void ActFallDown( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActFallDown[%d]\n",time ) ;

	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE) ;
	AT_SetActStSt( act, ACT_STST_FALL_DOWN ) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;

	if ( time == 0 ) {
//		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		if ( act->down_s == DownFront ) {
			AT_ReSetAction( act, 0, EM_dam_fence_f_fall, 0, MOTION_MASK_FULL, 0 ) ;
		} else {
			AT_ReSetAction( act, 0, EM_dam_fence_fall, 0, MOTION_MASK_FULL, 0 ) ;
		}
		/* 高さ補正 */
		act->ctrl->skip_flag |= CTRL_RESET_HZX_BASE ;
		GM_SeSetMode( SD_V_GBSFAL01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if( time>COUNT_VMODE(4) ) ENE_SetOffenseTargetEne( act, 2 ) ;

	if ( act->ctrl->grounded & 1 ) {
		AT_SetMode( act, ActFallDownEnd ) ;
		return ;
	}
}

static	void ActFallDownEnd( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActFallDownEnd[%d]\n",time ) ;

	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE) ;
	AT_SetActStSt( act, ACT_STST_FALL_DOWN ) ;
	AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;

	if ( time == 0 ) {
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
		if ( act->down_s == DownFront ) {
			AT_ReSetAction( act, 0, EM_dam_fence_f_down, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			AT_ReSetAction( act, 0, EM_dam_fence_down, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		}
//		act->ctrl->skip_flag |= CTRL_RESET_HZX_BASE ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		if ( act->bodyp.life > 0 ) {
			if ( !(GM_GameStatus & STATE_VR_ANOTHER) ) {
				act->bodyp.last_weapon = WP_NOPLAYER ;
			}
		}
		act->bodyp.life = 0 ;
		act->act_end = 1 ;
		AT_SetMode( act, ActDeath ) ;
		return ;
	}
}

static	void ActStairFallStart( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE|ACT_STATUS_SEG_OFF) ;

	if ( time < COUNT_VMODE(30) ) AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		if ( act->down_s == DownFront ) {
			AT_ReSetAction( act, 0, EM_dam_stair_start, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			AT_ReSetAction( act, 0, EM_dam_stair_start_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
			act->down_s = DownFront ;/* 前向きになる */
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		AT_SetMode( act, ActStairFallDown ) ;
		return ;
	}
}

static	void ActStairFallDown( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE) ;

	if ( time == 0 ) {
#if 1
		/* この時点では全て前向きに落ちる */
		act->keep_mot = EM_dam_stair_roll_1 ;
		AT_ReSetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 0 ) ;
		act->down_s = DownFront ;
#else
		if ( act->down_s == DownFront ) {
			act->keep_mot = EM_dam_stair_roll_1 ;
			AT_ReSetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 0 ) ;
		} else {
			AT_ReSetAction( act, 0, EM_dam_fence_f_fall, 0, MOTION_MASK_FULL, 0 ) ;
		}
#endif
	}

	if ( time > COUNT_VMODE(6) ) {
		AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		if ( !StairCheck( act ) ) {/* もう一回転？ */
			AT_SetMode( act, ActStairFallDownEnd ) ;
			return ;
		}

		if ( act->down_s == DownFront ) {
			act->keep_mot = (act->keep_mot == EM_dam_stair_roll_1)?EM_dam_stair_roll_2:EM_dam_stair_roll_1 ;
		}
		AT_ReSetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 0 ) ;
	}

	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step.vx = STEP_VMODE(64.0f) * _RsinF( (int)act->ctrl->turn.vy ) ;
	act->ctrl->step.vz = STEP_VMODE(64.0f) * _RcosF( (int)act->ctrl->turn.vy ) ;
}

static	void ActStairFallDownEnd( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE) ;

	if ( time == 0 ) {
		if ( act->down_s == DownFront ) {
			AT_ReSetAction( act, 0, EM_dam_stair_end, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			AT_ReSetAction( act, 0, EM_dam_fence_f_down, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		}
	}

	SetSlopeRotX( act ) ;

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		if ( act->bodyp.life > 0 ) {
			act->bodyp.last_weapon = 0 ;
		}
		act->bodyp.life = 0 ;
		act->act_end = 1 ;
		AT_SetMode( act, ActDeath ) ;
		return ;
	}
}

static	void ActDropp( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE) ;
	AT_SetActStSt( act, ACT_STST_FALL_DOWN|ACT_STST_DROP ) ;

	if ( time == 0 ) {
		if ( act->down_s == DownFront ) {
			AT_SetAction( act, 0, EM_dead_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
		} else if ( act->down_s == DownWall ) {
			AT_SetAction( act, 0, EM_dam_wall_down_p, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
		} else if ( (act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
			AT_SetAction( act, 0, EM_legs_idle, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
		} else {
			if ( act->keep_mot == EM_dam_kick_t ) {
				AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, 1 ) ;
			} else {
				AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			}
		}
	}

	if ( act->ctrl->grounded & 1 ) {
		AT_SetMode( act, ActFallDownEnd ) ;
		return ;
	}
}

void endamact_ActDown( act, time )
ACTION	*act ;
int		time ;
{
//printf("endamact_ActDown[%d]\n",time ) ;

	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_GUN_FREE  ) ;

	AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;

	if ( act->bodyp.anesthesia < 0 || act->bodyp.faint <= 0 ) {
		AT_SetActStatus( act, ACT_STATUS_FAINT ) ;
	}

//	if ( act->down_s != DownWall && act->c_motion_num[0]==EM_legs_idle ) {
	if ( act->down_s != DownWall ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
	}

	if ( time == 0 ) {
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
		if ( act->down_s == DownFront ) {
			AT_SetAction( act, 0, EM_dead_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
		} else if ( act->down_s == DownWall ) {
			AT_SetAction( act, 0, EM_dam_wall_down_p, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
		} else if ( (act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
			AT_SetAction( act, 0, EM_legs_idle, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
		} else {
#if 0
			AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
#else
if ( act->keep_mot == EM_dam_kick_t ) {
//printf("?????????????????????? kroekorkerkekrkroekroekor\n");
//	Corpse_ChangeAction( act->ik ) ;
	AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, 1 ) ;
} else {
			AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
}
#endif
		}
//printf("act down correct[%d] rot[%d]\n",act->body->m_ctrl->rot_correct,act->ctrl->rot.vy);
		act->sw->eye_anim = EYE_TUBIRI ;
	}
	SetSlopeRotX( act ) ;

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( time > COUNT_VMODE(30) ) {
		if ( act->bodyp.life <= 0 ) {
			act->act_end = 1 ;
			AT_SetMode( act, ActDeath ) ;
			return ;
		}

		if ( act->bodyp.blood < 0 ) {
			act->bodyp.life = 0 ;
			act->act_end = 1 ;
			AT_SetMode( act, ActDeath ) ;
#ifdef ZZZ_VER_Z
		} else if ( act->bodyp.anesthesia < 0 ||
			act->bodyp.ane_level_num[0] ||
			act->bodyp.ane_level_num[1] ||
			act->bodyp.ane_level_num[2] ||
			act->bodyp.ane_level_num[3] ) {
			act->bodyp.anesthesia = -1 ;
#else
		} else if ( act->bodyp.anesthesia < 0 ) {
#endif
         bp_trophy_tranqed_enemy();
			act->sw->eye_anim = EYE_TUBIRI ;
			act->bodyp.faint_time = CM_ZZZ_TIME ;
			ENE_SetHeadMark2( act, HEADMARK_ANA, ACT_HEADMARK2_ANES ) ;
			act->bodyp.faint = act->bodyp.m_faint ;	/* 気絶値再セット */
			if ( ENE_FaintExit( act ) ) {
				AT_SetMode( act, ActFaintExit ) ;
			} else {
				AT_SetMode( act, ActFaint ) ;
			}
			SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_ENEMY_SLEEP ) ;
		} else if ( act->bodyp.faint <= 0 ) {
			act->bodyp.faint_time = CM_FAINT_TIME + KR_RandU( COUNT_VMODE(64) );
			ENE_SetHeadMark2( act, HEADMARK_PIYO, ACT_HEADMARK2_FAINT ) ;
			act->bodyp.faint = act->bodyp.m_faint ;	/* 気絶値再セット */
			if ( ENE_FaintExit( act ) ) {
				AT_SetMode( act, ActFaintExit ) ;
			} else {
				AT_SetMode( act, ActFaint ) ;
			}
			SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_ENEMY_FAINT ) ;
		} else {
			if ( (act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
				act->act_end = 1 ;
//				AT_SetMode( act, ENE_ActStandStill ) ;
				AT_SetMode( act, ActMortally ) ;
//printf("korekroekokokokokokokaaaa----\n");
				act->sw->eye_anim = EYE_MABATAKI ;
			} else {
				AT_SetMode( act, ActWakeup ) ;
			}
		}

		return ;
	}
}


static	void ActDownDamage( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DOWN | ACT_STATUS_FAINT | ACT_STATUS_HOMING_SKIP | ACT_STATUS_EYE_CLOSE
					|ACT_STATUS_GUN_FREE|ACT_STATUS_TARGET_SKIP  ) ;
	AT_SetActStSt( act, ACT_STST_DOWN_DAM ) ;

	if ( act->down_s != DownWall ) 	AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;

	SetSlopeRotX( act ) ;

	if ( time == 0 ) {
		if ( act->keep_mot == EM_dam_leg_r && !(act->bodyp.pbreak & PBREAK_LEG_L) ) {
			act->bodyp.pbreak &= ~PBREAK_LEG_R ;
		}else if ( act->keep_mot == EM_dam_leg_l && !(act->bodyp.pbreak & PBREAK_LEG_R) ) {
			act->bodyp.pbreak &= ~PBREAK_LEG_L ;
		}
//		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;

printf("EM_dam_arm_r [%d] EM_dam_arm_l[%d] [%d][%d]\n ",EM_dam_arm_r,EM_dam_arm_l,EM_dam_leg_r, EM_dam_leg_l ) ;
printf("act->keep_mot[%d] break[%x]\n ",act->keep_mot,act->bodyp.pbreak ) ;
		switch ( act->keep_mot ) {
			case EM_dam_arm_r :
				act->bodyp.pbreak |= PBREAK_ARM_R ;
				break ;
			case EM_dam_arm_l :
				act->bodyp.pbreak |= PBREAK_ARM_L ;
				break ;
			case EM_dam_leg_r :
				act->bodyp.pbreak |= PBREAK_LEG_R ;
				break ;
			case EM_dam_leg_l :
				act->bodyp.pbreak |= PBREAK_LEG_L ;
				break ;
		}
		if ( (act->bodyp.life > 0) && ( act->bodyp.blood >= 0 ) ) {
			if ( !(*(act->ene_status) & ENE_STATUS_EVER_ZZZ) ) {
				act->bodyp.faint_time -= DOWN_FAINT_DAMAGE ;
            if( act->bodyp.faint_time < COUNT_VMODE(30) )
            {
               //BP_TODO: this trophy needs refinement.  Currently does not check the source of the damage
               //nor the extra conditions in ActFaint() that look like they could cause the enemy to die or not wake up.

               // Removed in MGS_2_and_3_Trophy_Info_110615rev.xls
               // Remove BP_TODO after next round of feedback from KP
               //BP_TrophySystem_UnlockTrophy( kTRP_WakeTranqedEnemy );
            }
			}
		} else {
			ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
		}
	}

	if ( act->bodyp.life <= 0 ) {
		if ( time<COUNT_VMODE(32)  && !(time%COUNT_VMODE(8)) ) {
			AT_SetActStatus( act, ACT_STATUS_IK_PIKU ) ;
		}
	} else {
		if ( time == 0 ) {
			AT_SetActStatus( act, ACT_STATUS_IK_PIKU ) ;
		}
		if ( DownTrgCheck( act ) ) {
			return ;
		}
	}

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( act->bodyp.life > 0 ) {
		if ( act->bodyp.blood < 0 ) {
			act->bodyp.life = 0 ;
			act->act_end = 1 ;
			AT_SetMode( act, ActDeath ) ;
		} else if ( time>COUNT_VMODE(30)  ) {

#if 1 //rev 01.12.10
			if ( 	act->bodyp.type & ENE_TYPE_SHIELD ) {
				/* 盾兵は部位ダメージ無し */
				//ENE_ClearPDamage( act ) ;
				ENE_ClearPDamageMask( act ,PBREAK_LEGS) ;
			}
			if ( act->bodyp.type & ENE_TYPE_NO_PBREAK ){
				ENE_ClearPDamage( act ) ;
			}
#endif
			AT_SetMode( act, ActFaint ) ;
			return ;
		}
	} else {
		if ( time>COUNT_VMODE(120)  ) {
			act->act_end = 1 ;
			AT_SetMode( act, ActDeath ) ;
printf ( "act->down_s =[%d]\n", act->down_s ) ;
			return ;
		}
	}
}

static	void ActFaint( act, time )
ACTION	*act ;
int		time ;
{

	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE
				|ACT_STATUS_FAINT|ACT_STATUS_GUN_FREE  ) ;

	if ( act->down_s != DownWall ) 	AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;

	if ( DownTrgCheck( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		if ( act->down_s == DownFront ) {
			AT_SetAction( act, 0, EM_dead_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
			act->bodyp.capture.flag = CAPTURE_FRONT ;
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_DOWN ) ;
		} else if ( act->down_s == DownWall ) {
			AT_SetAction( act, 0, EM_dam_wall_down_p, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
			act->bodyp.capture.flag = CAPTURE_FRONT ;
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_WALLDOWN ) ;
		} else {
			AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
			act->bodyp.capture.flag = CAPTURE_BACK ;
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_DOWN ) ;
		}


//		act->sw->eye_anim = EYE_SIROME ;
		act->sw->eye_anim = EYE_TUBIRI ;
	}

	if ( act->bodyp.blood < 0 ) {
		AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
		act->bodyp.life = 0 ;
		act->act_end = 1 ;
		AT_SetMode( act, ActDeath ) ;
		return ;
	}

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( act->bodyp.faint_time < COUNT_VMODE(30) ) {
		if ( act->headmark2 == ACT_HEADMARK2_FAINT && act->bodyp.anesthesia < 0 ) {
			act->bodyp.faint_time = CM_ZZZ_TIME ;
			ENE_SetHeadMark2( act, HEADMARK_ANA, ACT_HEADMARK2_ANES ) ;
			act->sw->eye_anim = EYE_TUBIRI ;
		} else {
#ifdef LEGS_DAM
			if ( (act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
				act->bodyp.life = 0 ;
				act->act_end = 1 ;
				AT_SetMode( act, ActDeath ) ;
			} else {
				AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
				act->bodyp.faint_time = COUNT_VMODE(30) ;
				AT_SetMode( act, ActWakeup ) ;
			}
#else
			AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
			act->bodyp.faint_time = COUNT_VMODE(30) ;
			AT_SetMode( act, ActWakeup ) ;
#endif
		}
		return ;
	}
}

static	void ActMortally( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DOWN | ACT_STATUS_HOMING_SKIP | ACT_STATUS_GUN_FREE  ) ;
	AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		AT_SetAction( act, 0, EM_legs_idle, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
		act->sw->eye_anim = EYE_MABATAKI ;
	}
	if ( time >= COUNT_VMODE(120) ) {
		if ( time == COUNT_VMODE(120) ) {
			AT_SetAction( act, 0, EM_legs_dam_out, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			act->down_s = DownFront ;
		}
		if ( time > COUNT_VMODE(200) ) AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;

		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			act->bodyp.life = 0 ;
			act->act_end = 1 ;
			AT_SetMode( act, ActDeath ) ;
			return ;
		}
	}
}

static	void ActBledToDeath( act, time )
ACTION	*act ;
int		time ;
{
	int	dir ;
	
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_HOMING_SKIP|ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP
					|ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;

	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act ) ;
	}
	if ( time > COUNT_VMODE(60) ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
	}

	if ( time == 0 ) {
		if ( act->keep_mot == EM_sleep_b ) {
			AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		}
		act->sw->eye_anim = EYE_DAMAGE ;
	}

	switch ( act->keep_mot ) {
		case EM_sleep_b :
			if ( time > COUNT_VMODE(100) ) {
				if ( ( dir = KR_GetDownDir( act->ctrl, act->ctrl->turn.vy ) ) > 0 ) {
					act->ctrl->turn.vy = dir ;
				}
			}
			break ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ActDeath ) ;
		return ;
	}

}

static	void ActDeath( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DEATH | ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_TARGET_SKIP
				|ACT_STATUS_GUN_FREE  ) ;
	if ( act->down_s != DownWall ) 	AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;

	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
		if ( act->down_s == DownFront ) {
			AT_SetAction( act, 0, EM_dead_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else if ( act->down_s == DownWall ) {
			AT_SetAction( act, 0, EM_dam_wall_down_p, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		}
		act->sw->eye_anim = EYE_SIROME ;

		ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
		ENE_ClearNeedl( act ) ;

		if ( act->bodyp.type & ENE_TYPE_DONT_KILL ) {
			SET_FLAG( GM_VRStatus, GM_VR_TAB_ENEMY_END ) ;
printf("aaaaaaaaaaaaaaaaaaaaaaaaDeattttthhhhh!!\n");
		}

printf("Deattttthhhhh!!\n");
	}

	if ( time > COUNT_VMODE(15) ) {
		extern void *NewMakeCorps( NAME_ID *, FVECTOR *, SVECTOR *, OBJECT *, 
			int, int, void *, int, ACTION *, int );
		void	*corp ;
		int		mot, flag, size ;

		act->act_end = 1 ;

		if ( ENE_FaintExit( act ) ) {
			if ( GV_GetMaxFreeMemory( GV_NORMAL_MEMORY ) > GV_MEM_EFFECT_LIMIT_SIZE ) {
				if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH ) {
					if ( ENE_BreakBody( act, 2, act->oozeblood ) != NULL ) {
						extern void *NewOozeBlood( DG_OBJS *, int ) ;
						if (!( act->bodyp.type & ENE_TYPE_TNG_A )) {
							/* 新しい滲み血 */
							act->oozeblood = NewOozeBlood( act->body->objs, act->name_id->body ) ;
						}
					}
				} else {
					extern void *NewOldCorp( int, CONTROL *, OBJECT *, FMATRIX *, int ) ;
					NewOldCorp( act->name_id->body, act->ctrl, act->body, act->lights, act->name_id->weapon ) ;
#ifdef KP_WINDOWS
					if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH_OFF ) {
						GM_SeSetMode( SD_A_V_SHITAI, (FVECTOR *)& act->body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
					}
#endif
				}
			}
			/* 新しいユニークID取得 */
			AT_SetActStSt( act, ACT_STST_CHANGE_UNIQ_ID ) ;
			if ( act->bodyp.type & ENE_TYPE_FAINT_EXIT ) VR_SurrenderEnemy( ) ;
#if 0
if ( act->down_s == DownBack ) {
	NewDeadLine( act->name_id->body, act->body, act->ctrl, 1 ) ;
} else if ( act->down_s == DownWall ) {
	NewDeadLine( act->name_id->body, act->body, act->ctrl, 2 ) ;
} else {
	NewDeadLine( act->name_id->body, act->body, act->ctrl, 0 ) ;
}
#endif

		} else if ( !(act->bodyp.type & ENE_TYPES_NO_CORP) ) {
			if ( act->ctrl->mov.vx < 1000000.0f ) {
				corp = NULL ;
#ifndef NO_CORPS
				size = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY ) ;
				if ( size > GV_MEM_EFFECT_LIMIT_SIZE ) {
					if ( act->down_s == DownFront ) {
						mot = EM_dead_idle_f ;
					} else if ( act->down_s == DownWall ) {
						mot = EM_dam_wall_down_p ;
					} else {
						mot = EM_dam_out_pause ;
					}
					flag = 0 ;
					if ( act->head_blood != NULL ) {
						SET_FLAG( flag, CORPS_FLAG_HEADBLOOD ) ;
					}
					corp = NewMakeCorps( act->name_id, &act->ctrl->mov,&act->ctrl->rot, act->body,
							EM_dam_out_pause,act->down_s, act->oozeblood, *(act->ene_status), act, flag ) ;
		//			corp = NewEneCorps( act->name_id, &act->ctrl->mov,&act->ctrl->rot, act->body,EM_dam_out_pause,
		//					act->down_s, act->oozeblood, *(act->ene_status), act ) ;
				}
#endif
				if ( corp != NULL ) {
					extern void *NewOozeBlood( DG_OBJS *, int ) ;
					if (!( act->bodyp.type & ENE_TYPE_TNG_A )) {
						/* 新しい滲み血 */
						act->oozeblood = NewOozeBlood( act->body->objs, act->name_id->body ) ;
					}
				} else {
#if 1
					extern void TAKABE_OozeBloodClear( void *_work, float scale ) ;
					if (!( act->bodyp.type & ENE_TYPE_TNG_A )) {
						if ( act->oozeblood != NULL ) {
							TAKABE_OozeBloodClear( act->oozeblood, 0.0f ) ;
						}
					}
#endif
				}
			} else {
				printf("endamact.c: DANGER DANGER DANGER !!!!!!!!\n");
			}
		}

		if ( !(act->bodyp.last_weapon & (WP_HANG|WP_STEAM)) ) {
//		if ( act->bodyp.dammode != DAM_MODE_HANG ) {
			BloodBio( act ) ;
		}
		/* 新しいユニークID取得 */
		AT_SetActStSt( act, ACT_STST_CHANGE_UNIQ_ID ) ;

		AT_SetMode( act, ActGhost ) ;

		if ( !(act->bodyp.last_weapon & (WP_NOPLAYER)) ) {
			KR_KillCount() ;
		}
		return ;
	}
}

static	void ActGhost( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DEATH|ACT_STATUS_HOMING_SKIP|
				ACT_STATUS_EYE_CLOSE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_GUN_FREE  ) ;

	if ( !(act->bodyp.type & ENE_TYPES_NO_CORP) ) {
		AT_SetActStatus( act, ACT_STATUS_GHOST ) ;
	}

	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		static FVECTOR	NextWorld = { 1000000.0f, 1000000.0f, 1000000.0f } ;

		DamageCaptureFlagClear( act ) ;
		if ( !(act->bodyp.type & ENE_TYPES_NO_CORP) ) {
			GM_ResetControlPosition( act->ctrl, &NextWorld ) ;/* 彼の世へ移動 */
			ReSetSlopeRotX( act ) ;
			ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
			if ( act->bodyp.type & ENE_TYPE_HITECH ) {
				if ( act->head_blood != NULL ) {
					GV_DestroyActor( act->head_blood ) ;
					act->head_blood = NULL ;
				}
			}
		}
	}

	/* 体の移動タイミングはthinkにて */
	if ( act->CheckPad( act ) < 0 ) {
		AT_UnSetTargetClass( act, TARGET_SKIP ) ;
		act->body->objs->flag &= ~DG_FLAG_INVISIBLE ;
		AT_SetActStatus( act, ACT_STATUS_RESURRECT  ) ;
		ReSetSlopeRotX( act ) ;
		act->sw->eye_anim = EYE_MABATAKI ;
		return ;
	}
}

static	void ActWakeup( act, time )
ACTION	*act ;
int		time ;
{
//	AT_SetActStatus( act, ACT_STATUS_DOWN | ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_TARGET_SKIP ) ;
	/* フィル用 */
	AT_SetActStatus( act, ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_TARGET_SKIP ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		if ( act->down_s == DownFront ) {
			if ( act->bodyp.faint_time > 0 ) {
				act->keep_mot = (act->bodyp.type & ENE_TYPE_HITECH)? EM_htc_ak_nom_okiru_f_slow:EM_okiru_aomuke_slow ;
			} else {
				act->keep_mot = EM_okiru_aomuke ;
			}
		} else {
			if ( act->bodyp.faint_time > 0 ) {
				act->keep_mot = EM_okiru_utubuse_slow ;
			} else {
				act->keep_mot = EM_okiru_utubuse ;
			}
		}
//		ENE_SetActionPBreak( act,0,act->keep_mot,0,MOTION_MASK_FULL,ACT_INTERP_M,PBREAK_OVER_BASE ) ;
		ENE_SetActionPBreak( act,0,act->keep_mot,0,MOTION_MASK_FULL,ACT_INTERP_M,PBREAK_OVER_BASE_HAND_ONLY ) ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		act->sw->eye_anim = EYE_DAMAGE ;

		ReSetSlopeRotX( act ) ;
		ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
	}

	switch( act->keep_mot ) {
		case EM_okiru_aomuke_slow :
		case EM_htc_ak_nom_okiru_f_slow :
			if ( time < COUNT_VMODE(224) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
//			if ( time == 190 )  ENE_SetOffenseTarget( act, act->keep_mot ) ;
		break ;
		case EM_okiru_aomuke :
			if ( time < COUNT_VMODE(98) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
//			if ( time == 70 )  ENE_SetOffenseTarget( act, act->keep_mot ) ;
		break ;
		case EM_okiru_utubuse_slow :
			if ( time < COUNT_VMODE(127) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
//			if ( time == 70 )  ENE_SetOffenseTarget( act, act->keep_mot ) ;
		break ;
		case EM_okiru_utubuse :
			if ( time < COUNT_VMODE(80) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
//			if ( time == 44 )  ENE_SetOffenseTarget( act, act->keep_mot ) ;
		break ;
	}

	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		if ( act->bodyp.anesthesia <= 0) act->act_end = 2 ;	/* 眠りから復活 */
		ENE_ClearNeedl( act ) ;

		AT_SetMode( act, ENE_ActStandStill ) ;
		act->sw->eye_anim = EYE_MABATAKI ;
		return ;
	}
	AT_SetActStatus( act, ACT_STATUS_WAKEUP ) ;
}

static	void ActGoofy( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_EYE_CLOSE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_dam_okaji, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_MIHIRAKI ;
		ENE_SetHeadMark( act, 0, HMK2_TYPE_BREAK_AUTO ) ;
	}
	if ( time == COUNT_VMODE(12) ) {
		ENE_SetHeadMark2( act, HEADMARK_POWA, ACT_HEADMARK2_POWA ) ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_MABATAKI ;
		ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
		return ;
	}

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return ;
	}


//	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
	if ( time > COUNT_VMODE(600) ) {
		act->act_end = 1 ;
		act->sw->eye_anim = EYE_MABATAKI ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;

		return ;
	}
}

static	void ActSteam( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_DAMAGE|ACT_STATUS_EYE_CLOSE ) ;

	if ( time < 30 ) AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		if ( (act->c_motion_num[0] != act->keep_mot) || 
			(KR_MotionTime( &act->body->m_ctrl->mt3_ctrl[ 0 ] ) > 60 ) ) {
			ENE_SetActionPBreak( act, 0, act->keep_mot, 
				0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		}
		act->sw->eye_anim = EYE_DAMAGE ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_MABATAKI ;
		return ;
	}
	if ( time > COUNT_VMODE(90) ) {	/* モーションの長さ１９８ */
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			act->act_end = 1 ;
			act->sw->eye_anim = EYE_MABATAKI ;
			AT_SetMode( act, ENE_ActStandStill ) ;

			return ;
		}
	}
}

static	void ActStun( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_DAMAGE|ACT_STATUS_EYE_CLOSE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_DAMAGE ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_MABATAKI ;
		return ;
	}

//	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
	if ( time > COUNT_VMODE(180) ) {
		act->act_end = 1 ;
		act->sw->eye_anim = EYE_MABATAKI ;
		AT_SetMode( act, ENE_ActStandStill ) ;

		return ;
	}
}

#if 0
static	void	SetBandage( act, parts )
ACTION	*act ;
int		parts ;
{
	/* 包帯 */
	switch ( parts ) {
		case BANDAGE_LH_2 :
			GM_InitObject( &act->bandage[4], GV_StrCode("bandage"), DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
			GM_ConfigObjectRoot( &act->bandage[4], act->body, HUMAN21_HIDARI_UDE1 ) ;
			break ;
	}

}
#endif

#include "../../kano/attachment/attachment_called.h"
static	void ActDamage( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_DAMAGE|ACT_STATUS_EYE_CLOSE ) ;

	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( act->old_status & ACT_STATUS_GUN_FREE ) {
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE  ) ;
	}

/*sigeno add begin ダメージ中無敵フラグ*/
	if ( act->bodyp.type & ENE_TYPE_DMG_MUTEKI){
		AT_SetActStatus( act,ACT_STATUS_TARGET_SKIP);
	}
/*sigeno add end */	

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		/*PS2版からは本編も直す*/
		act->ctrl->step.vx = 0.0 ;
		act->ctrl->step.vz = 0.0 ;
	} else {
		act->ctrl->step = DG_ZeroVector ;
	}

	if ( time == 0 ) {
		extern void TAKABE_OozeBloodAdd( void *, int, FVECTOR *, float ) ;
		FVECTOR	pos ;
		int see ;

		if ( act->keep_mot == EM_dam_leg_r && !(act->bodyp.pbreak & PBREAK_LEG_L) ) {
			act->bodyp.pbreak &= ~PBREAK_LEG_R ;
		}else if ( act->keep_mot == EM_dam_leg_l && !(act->bodyp.pbreak & PBREAK_LEG_R) ) {
			act->bodyp.pbreak &= ~PBREAK_LEG_L ;
		}

		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;

		switch ( act->keep_mot ) {
			case EM_dam_arm_r :
				act->bodyp.pbreak |= PBREAK_ARM_R ;
				AnyBlood( act ) ;
				Blood( act, 0, 2 ) ;
				TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
				break ;
			case EM_dam_arm_l :
				act->bodyp.pbreak |= PBREAK_ARM_L ;
				AnyBlood( act ) ;
				Blood( act, 0, 2 ) ;
				TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
				break ;
			case EM_dam_leg_r :
				act->bodyp.pbreak |= PBREAK_LEG_R ;
				AnyBlood( act ) ;
				Blood( act, 0, 2 ) ;
				TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
				break ;
			case EM_dam_leg_l :
				act->bodyp.pbreak |= PBREAK_LEG_L ;
				AnyBlood( act ) ;
				Blood( act, 0, 2 ) ;
				TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &act->bodyp.damtrg->hit, 90.0F ) ;
				break ;
			case EM_dam_punch_r :
			case EM_dam_punch_l :
			case EM_shl_nom_dam_gun :
				KR_FMatToFvec( &(act->body->objs->objs[HUMAN21_ATAMA].world), &pos ) ;
				break ;
		}

		act->sw->eye_anim = EYE_DAMAGE ;

#if 0
		GM_SeSetMode( SD_V_GBSDMG01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
#else
		see = SD_V_GBSDMG01+(act->name_id->voice%4) ;
		/* 連続して呼ばれるので違う声のほうが良い感じ */
		if ( act->bodyp.stand != STAND_DAM-1 ) {
			if ( act->keep_mot != EM_dam_punch_r ) {
				/* 但し音声キャンセルのためはじめの一声は本人 */
				see = SD_V_GBSDMG01+(KR_RandU(4)) ;
			}
		}
		GM_SeSetMode( see, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
#endif

		GM_SetNoise( NOISE_SCREAM , &act->ctrl->mov, act->ctrl->map ) ;

	}

	switch ( act->keep_mot ) {
		case EM_dam_punch_r :
		case EM_dam_punch_l :
		case EM_shl_nom_dam_gun :
			if ( time < COUNT_VMODE(14) ) AT_SetActStatus( act, ACT_STATUS_VANIME_HEAD ) ;
			if( time < COUNT_VMODE(4) ) {
				int dir ;

				dir  = GV_VecDir2( &act->bodyp.damtrg->power->force )+2048 ;
				act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
				act->ctrl->step.vx = STEP_VMODE(64.0f) * _RsinF( dir ) ;
				act->ctrl->step.vz = STEP_VMODE(64.0f) * _RcosF( dir ) ;
			}
			break ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = 1 ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		if ( 	act->bodyp.type & ENE_TYPE_SHIELD ) {
			//ENE_ClearPDamage( act ) ;
			ENE_ClearPDamageMask( act ,PBREAK_LEGS) ;
		}
		if ( 	act->bodyp.type & ENE_TYPE_NO_PBREAK){
			ENE_ClearPDamage( act ) ;
		}
		if ( act->bodyp.faint < 0 ) {
			act->keep_mot = EM_dam_punch2down ;
			act->down_s = DownFront ;
			AT_SetMode( act, ActFall ) ;
		} else {
			act->act_end = 1 ;
			act->sw->eye_anim = EYE_MABATAKI ;
			act->keep_mot = 0 ;
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}
}

static	void ActSqueeze( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_DAMAGE|ACT_STATUS_EYE_CLOSE ) ;
	if ( time <COUNT_VMODE(12) ) AT_SetActStatus( act, ACT_STATUS_PUSHT_SKIP ) ;
	AT_SetActStSt( act, ACT_STST_NOW_DAMAGE ) ;/*クリアリング用*/

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_DAMAGE ;

		GM_SeSetMode( SD_V_GBSDMG01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
//		GM_SeSetMode( SD_V_GBSDMG01+(KR_RandU(4)), &act->ctrl->mov, GM_SEMODE_BOMB ) ;

		GM_SetNoise( NOISE_SCREAM , &act->ctrl->mov, act->ctrl->map ) ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = 1 ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->sw->eye_anim = EYE_MABATAKI ;
		act->keep_mot = 0 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}


	if( time < COUNT_VMODE(4) ) {
		int dir ;

		dir = _FVecTrgDir2( &act->ctrl->mov, &act->bodyp.damtrg->hit )+ 256 ;
		act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		act->ctrl->step.vx = STEP_VMODE(64.0f) * _RsinF( dir ) ;
		act->ctrl->step.vz = STEP_VMODE(64.0f) * _RcosF( dir ) ;
	}

}

static	void ActHoldupExit( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act,ACT_STATUS_HOMING_SKIP | ACT_STATUS_TARGET_SKIP|ACT_STATUS_GUN_FREE  ) ;

	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
		ENE_SetActionPBreak( act, 0, EM_idle_holdup, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_HOLDUP ) ;

		ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
		ENE_ClearNeedl( act ) ;
printf("Hold Up Exit!!\n");
	}

	if ( time > COUNT_VMODE(15) ) {
		act->act_end = 1 ;

		if ( GV_GetMaxFreeMemory( GV_NORMAL_MEMORY ) > GV_MEM_EFFECT_LIMIT_SIZE ) {
			if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH ) {
				if ( ENE_BreakBody( act, 3, act->oozeblood ) != NULL ) {
					extern void *NewOozeBlood( DG_OBJS *, int ) ;
					if (!( act->bodyp.type & ENE_TYPE_TNG_A )) {
						/* 新しい滲み血 */
						act->oozeblood = NewOozeBlood( act->body->objs, act->name_id->body ) ;
					}
				}
			} else {
				extern void *NewOldCorp( int, CONTROL *, OBJECT *, FMATRIX *, int ) ;
				NewOldCorp( act->name_id->body, act->ctrl, act->body, act->lights, act->name_id->weapon ) ;
#ifdef KP_WINDOWS
				if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH_OFF ) {
					GM_SeSetMode( SD_A_V_HOLDUP, (FVECTOR *)& act->body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
				}
#endif
			}
		}

		/* 新しいユニークID取得 */
		AT_SetActStSt( act, ACT_STST_CHANGE_UNIQ_ID ) ;
		VR_SurrenderEnemy( ) ;

		AT_SetMode( act, ActGhost ) ;
		return ;
	}
}

static	void ActFaintExit( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DEATH | ACT_STATUS_HOMING_SKIP|ACT_STATUS_EYE_CLOSE|ACT_STATUS_TARGET_SKIP
				|ACT_STATUS_GUN_FREE  ) ;
	if ( act->down_s != DownWall ) 	AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;

	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		CaptureFlagClear( act ) ;	/* C4 落とすーーーーー */
		if ( act->down_s == DownFront ) {
			AT_SetAction( act, 0, EM_dead_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else if ( act->down_s == DownWall ) {
			AT_SetAction( act, 0, EM_dam_wall_down_p, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		}
		act->sw->eye_anim = EYE_SIROME ;

		ENE_ClearNeedl( act ) ;
printf("Faint Exit!!\n");
	}

	if ( time > COUNT_VMODE(30) ) {
		act->bodyp.life = 0 ;
		act->act_end = 1 ;

		ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
		if ( GV_GetMaxFreeMemory( GV_NORMAL_MEMORY ) > GV_MEM_EFFECT_LIMIT_SIZE ) {
			if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH ) {
				if ( ENE_BreakBody( act, 2, act->oozeblood ) != NULL ) {
					extern void *NewOozeBlood( DG_OBJS *, int ) ;
					if (!( act->bodyp.type & ENE_TYPE_TNG_A )) {
						/* 新しい滲み血 */
						act->oozeblood = NewOozeBlood( act->body->objs, act->name_id->body ) ;
					}
				}
			} else {
				extern void *NewOldCorp( int, CONTROL *, OBJECT *, FMATRIX *, int ) ;
				NewOldCorp( act->name_id->body, act->ctrl, act->body, act->lights, act->name_id->weapon ) ;
#ifdef KP_WINDOWS
				if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH_OFF ) {
					GM_SeSetMode( SD_A_V_SHITAI, (FVECTOR *)& act->body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
				}
#endif
			}
		}

		/* 新しいユニークID取得 */
		AT_SetActStSt( act, ACT_STST_CHANGE_UNIQ_ID ) ;
		if ( act->bodyp.type & ENE_TYPE_FAINT_EXIT ) VR_SurrenderEnemy( ) ;

		AT_SetMode( act, ActGhost ) ;
		return ;
	}
}

/*--------------------------------------------------------------------*/
int	EneEnemyDamageCheckPad( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_MORTALLY :
			AT_SetMode( act, ActMortally ) ;
			return 1 ;
		break ;
		case SP_DIE_ERASE :
			ENE_ClearNeedl( act ) ;
			ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
			HZX_FlashTrap( act->ctrl->hzx_id, &act->ctrl->evt ) ;
			/* 新しいユニークID取得 */
			AT_SetActStSt( act, ACT_STST_CHANGE_UNIQ_ID ) ;
			if ( act->bodyp.type & ENE_TYPE_FAINT_EXIT ) VR_SurrenderEnemy( ) ;
			AT_SetMode( act, ActGhost ) ;
			AT_SetAction( act, 0, EM_dead_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
			KR_KillCount() ;
			if ( act->bodyp.type & ENE_TYPE_DONT_KILL ) {
//				SET_FLAG( GM_VRStatus, GM_VR_TAB_ENEMY_END ) ;
printf("SP_DIE_ERASE!!\n");
			}
			return 1 ;
		break ;
		case SP_RESURRECTION :
			AT_SetMode( act, ENE_ActStandStill ) ;
			return -1 ;
		break ;
	}
	return 0 ;
}

static int BladePower( ACTION *act )
{
	int dis ;

return 0 ;
	dis = _FVecTrgDis( &act->bodyp.damtrg->center, &act->bodyp.off_center ) ;

	printf("sworddddddddddddddddddddddd dis[%d]\n",dis ) ;

	return 1 ;
}

static int	ChildTrgCheck( act )
ACTION	*act ;
{
	TARGET	*def ;
	PTARGET_INFO	*pinfo, *pi ;
	int		child_num ;
	long64	weapon ;

	def = &(act->bodyp.def_child1[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL1] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL1_NUM )) >= 0 ) {
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.n_damobj = pinfo->obj_num ;
printf("HIT OBJ [%d]\n",act->bodyp.n_damobj ) ;
printf("child1 weapontype [%lx]\n",def->weapon_type);

		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
		def->damaged = FLAG_CLEAR ;

		if ( weapon & WP_M92 ) {
			act->bodyp.anesthesia -= VPOINT_ANES ;
			act->bodyp.ane_level_num[PTARGET_LEVEL1] ++ ;
			ENE_SetNeedl( act->body, act->bodyp.n_damobj, &act->bodyp.damtrg->hit ) ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			/* このフレームではダメージを受けない */
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MASUI_SASARU ;
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;

			return 0 ;
		} else if( weapon & (WP_BLADE|WP_BLADESTAB) ){
	BladePower( act ) ;
			act->bodyp.life -= BLADE_CUT_DAM ;
			if( weapon & WP_BLADE ){
				GM_SeSetMode( SD_A_SWORDCUT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_01" ), 0 ) ; /* 切り振動 */
			}else {
				GM_SeSetMode( SD_A_SWORDCUT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_02" ), 0 ) ; /* 突き振動 */
			}
			if ( act->bodyp.life <= 0 ) {
				BloodBlade( act ) ;
				act->keep_mot = EM_dam_out_b_cap03 ;
				act->down_s = DownBack ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_NokezoriDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & WP_BLADEFAINT ) {
			/*刀 峰打ち*/
			act->bodyp.faint -= BLADE_FAINT ;
			GM_SeSetMode( SD_A_SWORDHIT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_katana_03" ), 0 ) ; /* 峯打ち振動 */
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_PunchDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & WP_THROWG ) {
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else if ( weapon & WP_HEADMARK ) {
			AT_SetMode( act, ActGoofy ) ;
		} else if ( weapon & WP_PUNCHR ) {
			if ( child_num == 2 ) {
				act->bodyp.faint -= ONE_FAINT_DOWN ;/* very hard でも気絶するように */
				act->down_s = DownBack ;
				act->keep_mot = EM_dam_ball_cap01 ;
				AT_SetMode( act, ActFall ) ;
				if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
					GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				} else {
					GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				}
			} else {
				if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
					GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
					act->bodyp.faint -= ONE_FAINT_DOWN ;/* very hard でも気絶するように */
//					act->bodyp.faint -= PUNCH_FAINT*2 ;
				} else if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
					GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
					act->bodyp.faint -= ONE_FAINT_DOWN ;/* very hard でも気絶するように */
//					act->bodyp.faint -= PUNCH_FAINT*3 ;
				} else {
					GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
					act->bodyp.faint -= PUNCH_FAINT ;
				}
				if ( act->bodyp.faint < 0 ) {
					act->keep_mot = EM_dam_punch2down ;
					act->down_s = DownFront ;
					act->status |= ACT_STATUS_TARGET_SKIP ;
					AT_SetMode( act, ActFall ) ;
				} else {
					act->keep_mot = EM_dam_punch_r ;
					AT_SetMode( act, ActDamage ) ;
				}
			}

			return 1 ;
		} else if ( weapon & WP_PUNCHL ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				act->bodyp.faint -= ONE_FAINT_DOWN ;/* very hard でも気絶するように */
//				act->bodyp.faint -= PUNCH_FAINT*3 ;
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
			} else {
				act->bodyp.faint -= PUNCH_FAINT ;
				GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
			}
			if ( child_num == 2 ) {
				act->bodyp.faint -= ONE_FAINT_DOWN ;/* very hard でも気絶するように */
				act->down_s = DownBack ;
				act->keep_mot = EM_dam_ball_cap01 ;
				AT_SetMode( act, ActFall ) ;
			} else {
				if ( act->bodyp.faint < 0 ) {
					act->keep_mot = EM_dam_punch2down ;
					act->down_s = DownFront ;
					act->status |= ACT_STATUS_TARGET_SKIP ;
					AT_SetMode( act, ActFall ) ;
				} else {
					act->keep_mot = ENE_PunchDamMotion(act) ;
					AT_SetMode( act, ActDamage ) ;
				}
			}
			return 1 ;
		} else if ( weapon & (WP_KICK1) ) {
			if ( child_num == 2 ) {
				act->bodyp.faint -= ONE_FAINT_DOWN ;/* very hard でも気絶するように */
			} else {
				act->bodyp.faint -= PUNCH_FAINT ;
			}

			GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = EM_dam_punch_r ;
				AT_SetMode( act, ActDamage ) ;
			}
		} else if ( weapon & WP_KICK ) {
			if ( (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) &&
				!(GM_PlayerStatus & PLAYER_ROLLING) ) {
				act->bodyp.faint -= NIKITA_FAINT_DOWN ;
				GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ; /* 巨大殴り振動 */
			} else {
				if ( child_num == 2 ) {
					act->bodyp.faint -= ONE_FAINT_DOWN ;/* very hard でも気絶するように */
				} else {
					act->bodyp.faint -= KICK_FAINT ;
				}
				GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			}
#if 1		/* 飛ぶのはプレイヤーと反対方向 */
			_sceVu0SubVector(  &act->bodyp.damtrg->power->force, 
//			&act->ctrl->mov, &GM_PlayerControl->mov ) ;
			&act->ctrl->mov, &act->bodyp.off_center ) ;
#endif
			act->keep_mot = EM_dam_kick_t ;
			act->down_s = DownBack ;
			AT_SetMode( act, ActDirFall ) ;
			return 1 ;
		} else {
printf("child_num [%d] durable0[%d]durable1[%d]\n",
child_num,act->bodyp.durable[ DURABLE_AREA0 ],act->bodyp.durable[ DURABLE_AREA1 ]);
			if ( child_num == 0 ) {
				if( act->bodyp.durable[ DURABLE_AREA0 ] > 0 ) {
					act->bodyp.durable[ DURABLE_AREA0 ] -- ;
					_HitEffect( act ) ;
					act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
					act->adj_piku_time = ADJ_PIKU_TIME ;
					return 0 ;
				} else {
#ifdef DURABLE_RESET
					act->bodyp.durable[ DURABLE_AREA0 ] = act->bodyp.max_durable[ DURABLE_AREA0 ] ;
#endif
				}
			} else {
				if( act->bodyp.durable[ DURABLE_AREA1 ] > 0 ) {
					act->bodyp.durable[ DURABLE_AREA1 ] -- ;
					_HitEffect( act ) ;
					act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
					act->adj_piku_time = ADJ_PIKU_TIME ;
					return 0 ;
				} else {
#ifdef DURABLE_RESET
					act->bodyp.durable[ DURABLE_AREA1 ] = act->bodyp.max_durable[ DURABLE_AREA1 ] ;
#endif
				}
			}

			act->bodyp.life = 0 ;
			act->bodyp.dam_level_num[PTARGET_LEVEL1] ++ ;
			act->keep_mot = pinfo->dam_motion ;
			act->down_s = pinfo->down_s ;

			if ( child_num == 0 ) {
				BloodM9( act, HUMAN21_ATAMA, &act->bodyp.damtrg->hit ) ; 
			}
			if ( child_num == 1 ) {
				if ( weapon & (WP_BULLET) ) {
					AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
				}
			}

//			if ( weapon & (WP_BULLET) ) {
				if ( act->bodyp.life <= 0 ) {
					Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
					AnyBlood( act ) ;
				}
				if ( FenceCheck( act ) ) {
					AT_SetMode( act, ActFenceFall ) ;
				} else {
					AT_SetMode( act, ActFall ) ;
				}
//			}
			return 1 ;
		}
	}

	def = &(act->bodyp.def_child2[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL2] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL2_NUM )) >= 0 ) {
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
printf("child2 weapontype [%lx]\n",def->weapon_type);

		act->bodyp.n_damobj = pinfo->obj_num ;
		act->bodyp.damtrg = def ;
		def->weapon_type = 0 ;
		def->damaged = FLAG_CLEAR ;


		if ( weapon & WP_M92 ) {
			act->bodyp.anesthesia -= LIMBS_ANES ;
			act->bodyp.ane_level_num[PTARGET_LEVEL2] ++ ;
			ENE_SetNeedl( act->body, act->bodyp.n_damobj, &act->bodyp.damtrg->hit ) ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MASUI_SASARU ;	/* このフレームではダメージを受けない */
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;

			return 0 ;
		} else if ( weapon & WP_THROWG ) {
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else if ( weapon & WP_PUNCHR ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->bodyp.faint -= PUNCH_FAINT*2 ;
			} else if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				act->bodyp.faint -= PUNCH_FAINT*3 ;
			} else {
				GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->bodyp.faint -= PUNCH_FAINT ;
			}
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = EM_dam_punch_r ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & WP_PUNCHL ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				act->bodyp.faint -= PUNCH_FAINT*3 ;
			} else {
				act->bodyp.faint -= PUNCH_FAINT ;
				GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
			}
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_PunchDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & (WP_KICK1) ) {
			GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			act->bodyp.faint -= PUNCH_FAINT ;
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = EM_dam_punch_r ;
				AT_SetMode( act, ActDamage ) ;
			}
		} else if ( weapon & (WP_KICK) ) {
			if ( (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) &&
				!(GM_PlayerStatus & PLAYER_ROLLING) ) {
				act->bodyp.faint -= NIKITA_FAINT_DOWN ;
				GM_SeSetMode( SD_P_NKTPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ; /* 巨大殴り振動 */
			} else {
				act->bodyp.faint -= KICK_FAINT ;
				GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			}
			act->keep_mot = EM_dam_kick_t ;
			act->down_s = DownBack ;
			AT_SetMode( act, ActDirFall ) ;
			return 1 ;
		} else if( weapon & (WP_BLADE|WP_BLADESTAB) ){
		BladePower( act ) ;
			act->bodyp.life -= BLADE_CUT_DAM ;
			if( weapon & WP_BLADE ){
				GM_SeSetMode( SD_A_SWORDCUT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_01" ), 0 ) ; /* 切り振動 */
			}else {
				GM_SeSetMode( SD_A_SWORDCUT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_02" ), 0 ) ; /* 突き振動 */
			}
			if ( act->bodyp.life <= 0 ) {
				BloodBlade( act ) ;
				act->keep_mot = EM_dam_out_b_cap03 ;
				act->down_s = DownBack ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_NokezoriDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & WP_BLADEFAINT ) {
			/*刀 峰打ち*/
			act->bodyp.faint -= BLADE_FAINT ;
			GM_SeSetMode( SD_A_SWORDHIT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_katana_03" ), 0 ) ; /* 峯打ち振動 */
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_PunchDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & (WP_BULLET) ) {
			if( act->bodyp.durable[ DURABLE_AREA2 ] > 0 ) {
				act->bodyp.durable[ DURABLE_AREA2 ] -- ;
				_HitEffect( act ) ;
				act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
				act->adj_piku_time = ADJ_PIKU_TIME ;
				return 0 ;
			}
#ifdef DURABLE_RESET
			 else {
				act->bodyp.durable[ DURABLE_AREA2 ] = act->bodyp.max_durable[ DURABLE_AREA2 ] ;
			}
#endif
			act->bodyp.dam_level_num[PTARGET_LEVEL2] ++ ;
			act->keep_mot = pinfo->dam_motion ;
			act->bodyp.stand += STAND_DAM ;
#ifdef NO_BLOOD_OUT
			{
				if(act->bodyp.type & ENE_TYPE_DMG_HAND_LEG){
					act->bodyp.life -= USP_DAM ;
				} else {
					act->bodyp.life -= USP_DAM/2 ;
				}
#else
/*sigeno add begin*/
			if(act->bodyp.type & ENE_TYPE_DMG_HAND_LEG){
				act->bodyp.life -= USP_DAM ;
#endif
				if ( act->bodyp.life <= 0 ) {
					Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
					AnyBlood( act ) ;
					if ( FenceCheck( act ) ) {
						AT_SetMode( act, ActFenceFall ) ;
					} else {
						pi = Ptarg.info[PTARGET_FALL] ;
						pi += GV_Time%PTARGET_FALL_NUM ;
						act->keep_mot = pi->dam_motion ;
						act->down_s = pi->down_s ;
						AT_SetMode( act, ActFall ) ;
					}
					return 1 ;
				}
			}

			if ( child_num == PTARGET_LEGR1 || child_num == PTARGET_LEGR2 ) {
				if ( act->bodyp.pbreak & PBREAK_LEG_L && !(act->bodyp.pbreak & PBREAK_LEG_R) ) {
					act->bodyp.pbreak |= PBREAK_LEG_R ;
					act->keep_mot = EM_dam_legs ;
					act->down_s = DownFront ;
					if ( act->bodyp.life <= 0 ) {
						Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
						AnyBlood( act ) ;
					}
					AT_SetMode( act, ActFall ) ;
					return 1 ;
				}
			}
			if ( child_num == PTARGET_LEGL1 || child_num == PTARGET_LEGL2 ) {
				if ( act->bodyp.pbreak & PBREAK_LEG_R && !(act->bodyp.pbreak & PBREAK_LEG_L) ) {
					act->bodyp.pbreak |= PBREAK_LEG_L ;
					act->keep_mot = EM_dam_legs ;
					act->down_s = DownFront ;
					if ( act->bodyp.life <= 0 ) {
						Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
						AnyBlood( act ) ;
					}
					AT_SetMode( act, ActFall ) ;
					return 1 ;
				}
			}

			if ( child_num == PTARGET_ARMR1 || child_num == PTARGET_ARMR2 ) {
				if ( (act->bodyp.pbreak & PBREAK_ARM_L) && !(act->bodyp.pbreak & PBREAK_ARM_R) ) {
					act->bodyp.life = 0 ;
					act->bodyp.pbreak |= PBREAK_ARM_R ;
					act->keep_mot = EM_dam_bomb_b ;
					act->down_s = DownFront ;
					if ( act->bodyp.life <= 0 ) {
						Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
						AnyBlood( act ) ;
					}
					AT_SetMode( act, ActFall ) ;
					return 1 ;
				}
			}

			if ( child_num == PTARGET_ARML1 || child_num == PTARGET_ARML2 ) {
				if ( (act->bodyp.pbreak & PBREAK_ARM_R) && !(act->bodyp.pbreak & PBREAK_ARM_L) ) {
					act->bodyp.life = 0 ;
					act->bodyp.pbreak |= PBREAK_ARM_L ;
					act->keep_mot = EM_dam_bomb_b ;
					act->down_s = DownFront ;
					if ( act->bodyp.life <= 0 ) {
						Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
						AnyBlood( act ) ;
					}
					AT_SetMode( act, ActFall ) ;
					return 1 ;
				}
			}

			if ( act->bodyp.stand > STAND_DAM_MAX ) {
				act->keep_mot = EM_dam_bomb_b ;
				act->down_s = DownFront ;
				AT_SetMode( act, ActFall ) ;
				return 1 ;
			}
			AT_SetMode( act, ActDamage ) ;
			return 1 ;
		}
	}

	def = &(act->bodyp.def_child3[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL3] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL3_NUM )) >= 0 ) {
printf("child3 weapontype [%lx]\n",def->weapon_type);
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;

		act->bodyp.n_damobj = pinfo->obj_num ;
		act->bodyp.damtrg = def ;
		def->weapon_type = 0 ;
		def->damaged = FLAG_CLEAR ;

		printf("Body Shot !!\n");
		if ( weapon & WP_M92 ) {
			act->bodyp.anesthesia -= BODY_ANES ;
			act->bodyp.ane_level_num[PTARGET_LEVEL3] ++ ;

			ENE_SetNeedl( act->body, act->bodyp.n_damobj, &act->bodyp.damtrg->hit ) ;
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MASUI_SASARU ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;

			return 0 ;
		} else if ( weapon & WP_MECABREAK ) {
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else if ( weapon & WP_THROWG ) {
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else if ( weapon & WP_HEADMARK ) {
			AT_SetMode( act, ActGoofy ) ;
		} else if ( weapon & WP_PUNCHR ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->bodyp.faint -= PUNCH_FAINT*2 ;
			} else if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				act->bodyp.faint -= PUNCH_FAINT*3 ;
			} else {
				GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->bodyp.faint -= PUNCH_FAINT ;
			}
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = EM_dam_punch_r ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & WP_PUNCHL ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				act->bodyp.faint -= PUNCH_FAINT*3 ;
			} else {
				act->bodyp.faint -= PUNCH_FAINT ;
				GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
			}
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_PunchDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & (WP_KICK1) ) {
			GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			act->bodyp.faint -= PUNCH_FAINT ;
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = EM_dam_punch_r ;
				AT_SetMode( act, ActDamage ) ;
			}
		} else if ( weapon & (WP_KICK) ) {
			if ( (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) &&
				!(GM_PlayerStatus & PLAYER_ROLLING) ) {
				act->bodyp.faint -= NIKITA_FAINT_DOWN ;
				GM_SeSetMode( SD_P_NKTPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ; /* 巨大殴り振動 */
			} else {
				act->bodyp.faint -= KICK_FAINT ;
				GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			}
			act->keep_mot = EM_dam_kick_t ;
			act->down_s = DownBack ;
			AT_SetMode( act, ActDirFall ) ;
			return 1 ;
		} else if( weapon & (WP_BLADE|WP_BLADESTAB) ){
		BladePower( act ) ;
			act->bodyp.life -= BLADE_CUT_DAM ;
			if( weapon & WP_BLADE ){
				GM_SeSetMode( SD_A_SWORDCUT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_01" ), 0 ) ; /* 切り振動 */
			}else {
				GM_SeSetMode( SD_A_SWORDCUT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_02" ), 0 ) ; /* 突き振動 */
			}
			if ( act->bodyp.life <= 0 ) {
				BloodBlade( act ) ;
				act->keep_mot = EM_dam_out_b_cap03 ;
				act->down_s = DownBack ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_NokezoriDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & WP_BLADEFAINT ) {
			/*刀 峰打ち*/
//			act->bodyp.faint -= PUNCH_FAINT ;
			act->bodyp.faint -= BLADE_FAINT ;
			GM_SeSetMode( SD_A_SWORDHIT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_katana_03" ), 0 ) ; /* 峯打ち振動 */
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_PunchDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
			return 1 ;
		} else if ( weapon & (WP_BULLET) ) {
			if( act->bodyp.durable[ DURABLE_AREA1 ] > 0 ) {
				act->bodyp.durable[ DURABLE_AREA1 ] -- ;
				_HitEffect( act ) ;
				act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
				act->adj_piku_time = ADJ_PIKU_TIME ;
				return 0 ;
			}
#ifdef DURABLE_RESET
			 else {
					act->bodyp.durable[ DURABLE_AREA1 ] = act->bodyp.max_durable[ DURABLE_AREA1 ] ;
			}
#endif
			act->bodyp.life -= USP_DAM ;
			act->bodyp.dam_level_num[PTARGET_LEVEL3] ++ ;
			AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
			if ( act->bodyp.life <= 0 ) {
				Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
				AnyBlood( act ) ;
				if ( FenceCheck( act ) ) {
					AT_SetMode( act, ActFenceFall ) ;
				} else {
					pi = Ptarg.info[PTARGET_FALL] ;
					pi += GV_Time%PTARGET_FALL_NUM ;
					act->keep_mot = pi->dam_motion ;
					act->down_s = pi->down_s ;
					AT_SetMode( act, ActFall ) ;
				}
			} else {
				if ( act->keep_mot == EM_dam_gun_3 ) {
					act->keep_mot = ENE_NokezoriDamMotion(act) ;
				} else if ( act->keep_mot == ENE_NokezoriDamMotion(act) ) {
					act->keep_mot = EM_dam_gun_2 ;
				} else {
					act->keep_mot = EM_dam_gun_3 ;
				}
				AT_SetMode( act, ActDamage ) ;
			}
		} else {
			if ( GM_GameStatus & STATE_VR_ANOTHER ) {
				if ( COM_StageKind( ) & ENE_STAGE_VRBOM_BIGDAM ) {
					act->bodyp.life -= VR_BOMB_BIGDAM ;
				} else {
					act->bodyp.life -= VR_BOMB_DAM ;
				}
			} else {
				act->bodyp.life -= BOMB_DAM ;
			}
			act->keep_mot = EM_dam_bomb_f ;
			act->down_s = DownFront ;
			AT_SetMode( act, ActDirFall ) ;
		}
		return 1 ;
	}

	return 0 ;
}

#define CAP_PUSH_BUG	(1)	//PALから 掴み系とプッシュ同時発生で出るバグ対処

#define NO_STUN_FAINT_VALUE	(3)
/* ターゲットの処理順番に注意 */
/* 子ターゲット＞パワーターゲット＞キャプチャーターゲット＞タッチターゲット */
/* 返り値０が優先 (おかしいかな)*/
int	ENE_EnemyDamagePad( act )
ACTION	*act ;
{
	PTARGET_INFO	*pi ;
	CAPTURE_TARGET	*cap ;
	TARGET	*def ;
	long64		weapon ;
	int			no_stun_type ;

	/* 落下死亡 */
	if ( LevelDeathCheck( act ) ) {
		AT_SetActStSt( act, ACT_STST_DROP ) ;
		act->bodyp.life = 0 ;
		AT_SetMode( act, ActDropp ) ;
		return 1 ;
	}

	/* 麻酔がact > thinkの順番なのでそれより前に判定 */
	/* どのダメージ処理よりも先にチェック */
	if ( act->bodyp.type & ENE_TYPE_HOLD_EXIT ) {
		if ( *(act->thk_status) & THK_STATUS_HOLD_UP ) {
			AT_SetMode( act, ActHoldupExit ) ;
			act->status |= ACT_STATUS_TARGET_SKIP ;	/* このフレームではダメージを受けない */

			return 1 ;
		}
	}

	/* 出血死 */
	if ( act->bodyp.blood < 0 ) {
		act->keep_mot = EM_dam_out_bleed ;
		act->down_s = DownBack ;
		act->bodyp.life = 0 ; 
		AT_SetMode( act, ActBledToDeath ) ;
		act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_BLEDTODEATH ;	/* このフレームではダメージを受けない */
#ifdef LEAVE_C4_DAMAGE_CLEAR
		LeaveC4DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif
		return 1 ;
	}

	if ( ChildTrgCheck( act ) ) {
		AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
#ifdef LEAVE_C4_DAMAGE_CLEAR
		LeaveC4DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif
		return 1 ;
	}

	def = &(act->bodyp.deftrg) ;
	if ( TARGET_POWER & def->damaged ) {
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.damtrg = def ;
printf("def weapon type [%lx]  \n",weapon);

#ifdef LEAVE_C4_DAMAGE_CLEAR
		LeaveC4DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif
		def->weapon_type = 0 ;

		/* このフレームではダメージを受けない */
		AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
		if ( weapon & WP_M92 ) {
			act->bodyp.anesthesia -= BODY_ANES ;
			act->bodyp.ane_level_num[PTARGET_LEVEL3] ++ ;

			ENE_SetNeedl( act->body, act->bodyp.n_damobj, &act->bodyp.damtrg->hit ) ;
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MASUI_SASARU ;
			act->adj_piku_time = ADJ_PIKU_TIME ;

			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;

			return 0 ;
		} else if ( weapon & WP_MECABREAK ) {
printf(" WP_MECABREAK!! \n");
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else if ( weapon & WP_THROWG ) {
printf(" WP_THROWG!! \n");
			act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
			act->adj_piku_time = ADJ_PIKU_TIME ;
			return 0 ;
		} else if ( weapon & WP_HEADMARK ) {
			AT_SetMode( act, ActGoofy ) ;
		} else if ( weapon & (WP_BULLET) ) {
			if( act->bodyp.durable[ DURABLE_AREA1 ] > 0 ) {
				act->bodyp.durable[ DURABLE_AREA1 ] -- ;
				_HitEffect( act ) ;
				act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MECABREAK ;
				act->adj_piku_time = ADJ_PIKU_TIME ;
				return 0 ;
			}
#ifdef DURABLE_RESET
			 else {
					act->bodyp.durable[ DURABLE_AREA1 ] = act->bodyp.max_durable[ DURABLE_AREA1 ] ;
			}
#endif

			AT_SetActStSt( act, ACT_STST_KAITAIC4_BOMB ) ;
			act->bodyp.dam_level_num[PTARGET_LEVEL3] ++ ;
			act->bodyp.life -= USP_DAM ;
			if ( act->bodyp.life <= 0 ) {
				if ( FenceCheck( act ) ) {
					AT_SetMode( act, ActFenceFall ) ;
				} else {
					pi = Ptarg.info[PTARGET_FALL] ;
					pi += GV_Time%PTARGET_FALL_NUM ;
					act->keep_mot = pi->dam_motion ;
					act->down_s = pi->down_s ;
					if ( act->bodyp.life <= 0 ) {
						Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
						AnyBlood( act ) ;
					}
					AT_SetMode( act, ActFall ) ;
				}
			} else {
				if ( act->keep_mot == EM_dam_gun_3 ) {
					act->keep_mot = ENE_NokezoriDamMotion(act) ;
				} else if ( act->keep_mot == ENE_NokezoriDamMotion(act) ) {
					act->keep_mot = EM_dam_gun_2 ;
				} else {
					act->keep_mot = EM_dam_gun_3 ;
				}
				AT_SetMode( act, ActDamage ) ;
			}
		} else if ( weapon & WP_BLAST ) {
			if ( GM_GameStatus & STATE_VR_ANOTHER ) {
				if ( COM_StageKind( ) & ENE_STAGE_VRBOM_BIGDAM ) {
					act->bodyp.life -= VR_BOMB_BIGDAM ;
				} else {
					act->bodyp.life -= VR_BOMB_DAM ;
				}
			} else {
				act->bodyp.life -= BOMB_DAM ;
			}
			act->keep_mot = ( act->ctrl->rot.vy % 2 ) ? EM_dam_bomb_fly_b : EM_dam_bomb_fly_f ;/* randam */
			act->down_s = DownFront ;
			AT_SetMode( act, ActDirFall ) ;
//			GM_SeSetMode( SD_V_PDMG01, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
		} else if ( weapon & (WP_STUNGRENADE) ) {
			/*2002.08.14 sigeno VRモードとテイルズではスタン耐性対象が違う*/

			if(GM_VRStatus & GM_VR_SNAKETALES) {
				/*テイルズ中*/
				no_stun_type = ENE_TYPES_NO_STUN_TALES ;
			}else if(GM_GameStatus & STATE_VR_ONLY){
				/*ＶＲ中*/
				no_stun_type = ENE_TYPES_NO_STUN_VR ;
			}else if ( GM_GameStatus & STATE_VR_ANOTHER ) {
				/*アナザー*/
				no_stun_type = ENE_TYPES_NO_STUN_ANOTHER ;
			}else if(GM_GameStatus & STATE_BOSS_SURVIVAL){
				/*ボスサバイバル*/
				no_stun_type = ENE_TYPES_NO_STUN_BOSS_SURVIVAL ;
			}else {
				/*通常本編*/
				no_stun_type = ENE_TYPES_NO_STUN ;
			}

			if( act->bodyp.type & no_stun_type ){
				act->bodyp.faint -= NO_STUN_FAINT_VALUE ;
			} else {
				act->bodyp.faint = 0 ;
			}

			act->keep_mot = EM_dam_stun_near ;
			act->down_s = DownBack ;
			AT_SetMode( act, ActFall ) ;
		} else if ( weapon & (WP_STEAM) ) {
#ifdef JIK_STEAM_DAM
			act->bodyp.life -= USP_DAM/2 ;
			if ( act->bodyp.life <= 0 ) {
				act->keep_mot = EM_dam_out ;
				act->down_s = DownBack ;
            bp_trophy_steam_enemy();
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = EM_dam_fire_ex ;
				AT_SetMode( act, ActSteam ) ;
			}
#else
			act->keep_mot = EM_dam_fire_ex ;
			AT_SetMode( act, ActSteam ) ;
#endif
		} else if ( weapon & (WP_EXTINGUISHER) ) {
			act->keep_mot = EM_dam_fire_ex ;
         BP_TrophySystem_UnlockTrophy( kTRP_FireExtinuisher );
			AT_SetMode( act, ActSteam ) ;
		} else if ( weapon & (WP_COLDSPRAY) ) {
			act->keep_mot = EM_dam_fire_ex ;
			AT_SetMode( act, ActSteam ) ;
		} else if ( weapon & (WP_STUNFAR) ) {
			act->keep_mot = (act->bodyp.type & ENE_TYPE_HITECH)? EM_htc_ak_nom_dam_stun_light:EM_dam_stun_far ;
			AT_SetMode( act, ActStun ) ;
		} else if ( weapon & WP_PUNCHR ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & (WP_TYPE_PUNCH_M|WP_TYPE_PUNCH_S) ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->bodyp.faint -= PUNCH_FAINT*2 ;
			} else if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				act->bodyp.faint -= PUNCH_FAINT*3 ;
			} else {
				GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
				act->bodyp.faint -= PUNCH_FAINT ;
			}
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = EM_dam_punch_r ;
				AT_SetMode( act, ActDamage ) ;
			}
		} else if ( weapon & WP_PUNCHL ) {
			if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
				GM_SeSetMode( SD_P_GUNPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ; /* 殴り中振動 */
				act->bodyp.faint -= PUNCH_FAINT*3 ;
			} else {
				act->bodyp.faint -= PUNCH_FAINT ;
				GM_SeSetMode( SD_P_PUNCH02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ; /* 殴り小振動 */
			}
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_PunchDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
		} else if ( weapon & (WP_KICK1) ) {
			GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			act->bodyp.faint -= PUNCH_FAINT ;
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = EM_dam_punch_r ;
				AT_SetMode( act, ActDamage ) ;
			}
		} else if ( weapon & (WP_KICK) ) {
			if ( (GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_LL) &&
				!(GM_PlayerStatus & PLAYER_ROLLING) ) {
				act->bodyp.faint -= NIKITA_FAINT_DOWN ;
				GM_SeSetMode( SD_P_NKTPNC01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ; /* 巨大殴り振動 */
			} else {
				act->bodyp.faint -= KICK_FAINT ;
				GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ; /* けり振動 */
			}
#if 1		/* 飛ぶのはプレイヤーと反対方向 */
			_sceVu0SubVector(  &act->bodyp.damtrg->power->force, 
//			&act->ctrl->mov, &GM_PlayerControl->mov ) ;
			&act->ctrl->mov, &act->bodyp.off_center ) ;
#endif
			act->keep_mot = EM_dam_kick_t ;
			act->down_s = DownBack ;
			AT_SetMode( act, ActDirFall ) ;
		} else if ( weapon & WP_WALLCRASH ) {
			act->bodyp.faint -= KICK_FAINT ;
			act->keep_mot = EM_dam_kick_t ;
			act->down_s = DownBack ;
			AT_SetMode( act, ActDirFall ) ;
			GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
		} else if ( weapon & WP_STAMP ) {
			NewPadVibration2( GV_StrCode( "push" ), 0 ) ; /* 振動 */
			act->bodyp.faint -= ONE_FAINT_DOWN ;/* very hard でも気絶するように */
			act->keep_mot = EM_dam_elude ;
			act->down_s = DownBack ;
			AT_SetMode( act, ActDirFall ) ;
			GM_SeSetMode( SD_P_KICK02, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
#if 1 
// sigeno add
/*刀ダメージ*/
//WP_BLADE WP_BLADEFAINT WP_BLADESTAB
//SD_A_SWORDHIT
		}else if( weapon & (WP_BLADE|WP_BLADESTAB) ){
printf("WP_BLADE !!!!!!!!!!\n");
	BladePower( act ) ;
			act->bodyp.life -= BLADE_CUT_DAM ;
			if( weapon & WP_BLADE ){
				GM_SeSetMode( SD_A_SWORDCUT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_01" ), 0 ) ; /* 切り振動 */
			}else {
				GM_SeSetMode( SD_A_SWORDCUT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_02" ), 0 ) ; /* 突き振動 */
			}
			if ( act->bodyp.life <= 0 ) {
				act->keep_mot = EM_dam_out_b_cap03 ;
				act->down_s = DownBack ;
				if ( act->bodyp.life <= 0 ) {
					Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
					AnyBlood( act ) ;
				}
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_NokezoriDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
		} else if ( weapon & WP_BLADEFAINT ) {
/*刀 峰打ち*/
//			act->bodyp.faint -= PUNCH_FAINT ;
			act->bodyp.faint -= BLADE_FAINT ;
			GM_SeSetMode( SD_A_SWORDHIT, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_katana_03" ), 0 ) ; /* 峯打ち振動 */
			if ( act->bodyp.faint < 0 ) {
				act->keep_mot = EM_dam_punch2down ;
				act->down_s = DownFront ;
				act->status |= ACT_STATUS_TARGET_SKIP ;
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_PunchDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
#endif
		} else {
			/* ダメージ計算 */
			act->bodyp.life -= USP_DAM ;
			if ( act->bodyp.life <= 0 ) {
//				if (  GV_Time % 8  ) {
				if (  1  ) {
					act->keep_mot = EM_dam_out ;
					act->down_s = DownBack ;
				} else {
					act->keep_mot = EM_dam_out_f ;
					act->down_s = DownFront ;
				}
				if ( act->bodyp.life <= 0 ) {
					Blood( act, 1, 17 ) ;//dam_trg 入っていないと落ちる
					AnyBlood( act ) ;
				}
				AT_SetMode( act, ActFall ) ;
			} else {
				act->keep_mot = ENE_NokezoriDamMotion(act) ;
				AT_SetMode( act, ActDamage ) ;
			}
		}

		return 1 ;
	}

	if ( GM_GameLevel < GM_LEVEL_NORMAL ) {
		/*2002.08.14 sigeno VRモードとテイルズではスタン耐性対象が違う*/
		if(GM_VRStatus & GM_VR_SNAKETALES) {
			/*テイルズ中*/
			no_stun_type = ENE_TYPES_NO_STUN_TALES ;
		}else if(GM_GameStatus & STATE_VR_ONLY){
			/*ＶＲ中*/
			no_stun_type = ENE_TYPES_NO_STUN_VR ;
		}else if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			/*アナザー*/
			no_stun_type = ENE_TYPES_NO_STUN_ANOTHER ;
		}else if(GM_GameStatus & STATE_BOSS_SURVIVAL){
			/*ボスサバイバル*/
			no_stun_type = ENE_TYPES_NO_STUN_BOSS_SURVIVAL ;
		}else {
			/*通常本編*/
			no_stun_type = ENE_TYPES_NO_STUN ;
		}

		if ( !(act->bodyp.type & no_stun_type) ){

//			if ( !(act->bodyp.deftrg.class & TARGET_SKIP) ) {
			if ( !(act->old_status &  (ACT_STATUS_TARGET_SKIP|ACT_STATUS_UNREAL) ) ) {
				if ( GM_GameStatus & STATE_STUN ) {
					act->bodyp.faint = 0 ;
					act->keep_mot = EM_dam_stun_near ;
					act->down_s = DownBack ;
					act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_DAMAGE_NOW ;
					act->bodyp.damtrg = &(act->bodyp.deftrg) ;	/* Blood 用 */
					AT_SetMode( act, ActFall ) ;
					return 1 ;
				}
			}
		}
	}

	def = &(act->bodyp.deftrg) ;
	if ( TARGET_CAPTURE & def->damaged ) {
#ifndef CAP_PUSH_BUG
		DamageFlagClear( act ) ;
#endif
		cap = &(act->bodyp.capture) ;
	    if ( cap->capture != NULL ) { /* 捕まった */
	//printf( "GV_Time[%d] capture %d !!1\n",GV_Time,cap->flag ) ;
			if ( cap->flag & CAPTURE_HANG ) {	/* 首絞め */
#ifdef CAP_PUSH_BUG
				DamageFlagClear( act ) ;
#endif
				/* このフレームではダメージを受けない */
				AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
#ifdef JIK_KUBISAKE
				if ( *(act->thk_status) & THK_STATUS_SIGHT_IN ) {
					AT_SetMode( act, ActHangAvoid ) ;
				} else {
					AT_SetMode( act, ActHang ) ;
				}
#else
				AT_SetMode( act, ActHang ) ;
#endif
#ifdef LEAVE_C4_DAMAGE_CLEAR
				UnsetLeaveC4CaptureFlag( act ) ;
#else
				cap->flag = 0 ;
#endif
				return 1 ;
			} else if ( cap->flag & CAPTURE_THROW ) {	/* 投げ */
#ifdef CAP_PUSH_BUG
				DamageFlagClear( act ) ;
#endif
				/* このフレームではダメージを受けない */
				AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
				AT_SetMode( act, ActThrow ) ;
#ifdef LEAVE_C4_DAMAGE_CLEAR
				UnsetLeaveC4CaptureFlag( act ) ;
#else
				cap->flag = 0 ;
#endif
				return 1 ;
			}
#ifdef CAP_PUSH_BUG
			def->damaged &= ~TARGET_CAPTURE ;
			cap->capture = NULL ;
printf(" capture !! but no hang no throw !!!!!!!\n");
#endif
		}
	}


	def = &(act->bodyp.pushtrg) ;
	if ( TARGET_TOUCH & def->damaged ) {
		weapon = def->weapon_type ;
		act->bodyp.last_weapon = weapon ;
		act->bodyp.damtrg = def ;
printf("touch def weapon type [%lx]  \n",weapon);

#ifdef LEAVE_C4_DAMAGE_CLEAR
		LeaveC4DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif
		def->weapon_type = 0 ;
		/* このフレームではダメージを受けない */
		AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP ) ;
		if ( weapon & WP_BODY ) {	/* 押され */
			int dir ;
			dir = _FVecTrgDir2( &act->ctrl->mov, &def->hit ) ;
printf("touch dir = %d\n",dir ) ;
			act->keep_mot = (GV_DiffDirAbs( act->ctrl->turn.vy, dir ) > 1024) ? EM_squeezed_f :EM_squeezed_b ;
			AT_SetMode( act, ActSqueeze ) ;
		}

		return 1 ;
	}

	if ( act->bodyp.anesthesia < 0 ) {
		if ( GV_Time%2 ) {
			act->keep_mot = EM_sleep_b ;
			act->down_s = DownBack ;
		} else {
			act->keep_mot = EM_sleep_f ;
			act->down_s = DownFront ;
		}
		AT_SetMode( act, ActSleepDown ) ;
		act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MASUI_KIKU ;	/* このフレームではダメージを受けない */

		return 1 ;
	}

	return 0 ;
}


void ENE_EnemyDown( act )
ACTION	*act ;
{
	act->keep_mot = EM_sleep_f ;
	act->down_s = DownFront ;
	AT_SetMode( act, ActSleepDown ) ;
	act->status |= ACT_STATUS_TARGET_SKIP|ACT_STATUS_MASUI_KIKU ;
}

void ENE_SetEnemyDown( ACTION *act, int faint, int time, int pose )
{
	if ( faint == ENEMEM_ST_HELL ) {
		AT_SetActStatus( act, ACT_STATUS_DEATH|ACT_STATUS_HOMING_SKIP|
					ACT_STATUS_EYE_CLOSE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_GUN_FREE  ) ;
		AT_SetActStatus( act, ACT_STATUS_GHOST ) ;
		AT_SetMode( act, ActGhost ) ;
		return ;
	}

	AT_SetActStatus( act, ACT_STATUS_FAINT ) ;
	act->sw->eye_anim = EYE_TUBIRI ;

	act->bodyp.faint_time = time ;
	if ( faint == ENEMEM_ST_SLEEP ) {
		ENE_SetHeadMark2( act, HEADMARK_ANA, ACT_HEADMARK2_ANES ) ;
		act->bodyp.anesthesia = -1 ;
	} else {
		ENE_SetHeadMark2( act, HEADMARK_PIYO, ACT_HEADMARK2_FAINT ) ;
	}

	switch( pose ) {
		case ENEMEM_PS_FRONT :
			act->keep_mot = EM_dead_idle_f ;
			act->down_s = DownFront ;
		break ;
		case ENEMEM_PS_BACK :
			act->keep_mot = EM_dam_out_pause ;
			act->down_s = DownBack ;
		break ;
		case ENEMEM_PS_WALL :
			act->keep_mot = EM_dam_wall_down_p ;
			act->down_s = DownWall ;
		break ;
		case ENEMEM_PS_LOCKER :
			SET_FLAG( act->bodyp.capture.flag, CAPTURE_LOCKER ) ;
			AT_SetAction( act, 0, EM_dead_carry_locker_idle, 0, MOTION_MASK_FULL, 0 ) ;
		    AT_SetMode( act, ActFaintStandLocker ) ;
			AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
			SET_FLAG( act->ctrl->skip_flag,  (CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK) ) ;
			return ;
		break ;
	}

	AT_SetMode( act, ActFaint ) ;
	AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, 0 ) ;
}
