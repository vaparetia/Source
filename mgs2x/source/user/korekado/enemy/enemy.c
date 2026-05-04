//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	enemy.c
	敵兵用、汎用思考ルーチン

	1999/07/07 Y.Korekado
	$Id: enemy.c,v 1.4 2002/12/05 18:42:03 takaki Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <string.h>
#endif
 
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"libutl.h"

#include	"gameheader.h"
#include	"enemy.h"

#include "BP_Debug.h"   //BP_ENEMY
#include "BP_Misc.h"
#ifdef PSX2
#include	"../../kano/corpse/corpse_ik.x"
#endif

extern void VR_AddEnemy(void);

//#define PRINT_DEBUG
//#define NO_SHOT_ENE___ (1)

#define SHADOW_LOW_POLY	(1)

//#define SPS_DAMAGE	20
#define SPS_DAMAGE	40 /*2001.08.18 4倍に*/
#define	AK_DAMAGE	10

#define TAKABE_IK	(1)
#define NO_KANO_IK	(1)

extern	int	CM_ZZZ_TIME ;
extern	HZX_ZON *ENE_HZX_GetZone(int addr);

/*-----	外部変数	-----*/
/* ターゲットサイズ */
FVECTOR	EneBodyTrgSize[] = {
	{ 400.0F, 1000.0F, 400.0F },	/* 標準 */
	{ 700.0F, 650.0F, 700.0F },		/* しゃがみ */
	{ 1100.0F, 300.0F, 1100.0F },	/* 部位ダメージ以外もあたるようになったので小さめに */
	{ 250.0F, 1000.0F, 250.0F },	/* 盾立ち  sigeno追加*/
	{ 250.0F, 600.0F, 250.0F },		/* 盾しゃがみ sigeno追加*/
	{   0.0F,   0.0F,   0.0F },
	{   50.0F,   500.0F,   50.0F },		/* タッチターゲット */
	{ 700.0F, 550.0F, 700.0F },		/* ふせ */
	{ 1000.0F, 650.0F, 1000.0F },		/* 壁もたれダウン */
	{ 700.0F, 650.0F, 700.0F },		/* w25c 窓から飛び出し */
} ;

/*-----	-----*/
int ENE_FaintExit( ACTION *act )
{
	if ( COM_StageKind() & ENE_STAGE_FAINT_EXIT ) return 1 ;
	if ( act->bodyp.type & ENE_TYPE_FAINT_EXIT ) return 1 ;
	
	return 0 ;
}

void ENE_AppearEffect( ENETHINK *entk, int type )
{
	extern void VRFUNC_EnemyAppearEf(OBJECT *body,OBJECT *weapon,int type) ;
/*
現在の総フレーム数 15
type 0 出現
type 1 消失
*/
//printf("koreeeeeeeeeeeeENE_AppearEffect:id [%d] status[%lx] old[%lx] type[%d]\n",entk->id, entk->act->status, entk->act->old_status ,type);
	VRFUNC_EnemyAppearEf( entk->act->body, entk->weapon, type ) ;
}

/* NULL が返ると失敗でoozeは引き継がれていない */
void *ENE_BreakBody( ACTION *act, int type, void *ooze )
{
	extern void VRFUNC_SIG_EneEquipBreak(OBJECT *body ,int type) ;
	extern void *VRFUNC_NewENE_BreakBody(OBJECT *body,int type, void *ooze) ;

//type 1 崩れ 2 上り 3 分散 4高速消え
	VRFUNC_SIG_EneEquipBreak( act->body, act->bodyp.type ) ;
	return VRFUNC_NewENE_BreakBody( act->body, type, ooze ) ;
}

void ENE_TraceClear( ENETHINK *entk )
{
	if ( entk->c_notice & ENE_NOTICE_TRACE ) {
printf("enemy.c:trace flag clear!!\n");
		ENE_TraceEnd( entk ) ;
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_TRACE ) ;
	}
}

void ENE_GunAction( ENETHINK *entk )
{
	ACTGUN	*actgun ;
	
	actgun = &entk->act->actgun ;
	
	if ( --actgun->gun_brank < 0 ) {
		actgun->gun_brank = 0 ;
		if ( actgun->gun_pad & ACTGUN_PULL_TRIGGER ) {
			FVECTOR	*trg_pos ;
			
			trg_pos = ( actgun->gun_pad & ACTGUN_TARGET_AIMPOS ) ? &entk->act->aim_pos : NULL ;
			
			ENE_ShootBullet( entk->act, ENE_BULLET_NORMAL , trg_pos ) ;
			entk->act->actgun.gun_brank = 4 ;
		}
	}
}

int ENE_NoRadio( void )
{
	if ( COM_GetSearchLevel( ) > (DEF_SEARCH_LEVEL/4 * 3) ) return 1 ;
	if ( GM_AlertMode == ALERT_MODE_AVOID ) return 1; 

	return 0 ;
}

/* 見つけられない特殊なプレイヤー状態*/
int ENE_CheckPlayerHidden( void )
{
	if ( GM_CheckPlayerStatusEX((PLAYER_INTRUDE|PLAYER_ENEMY_HIDDEN),PLAYER2_NARROW_HIDDEN) ) {
		return 1 ;
	}
	return 0 ;
}


/* 最後に連絡をした場所 */
void ENE_SetLastRadioPos( ENETHINK *entk, FVECTOR *pos, int hzx_id )
{
	entk->last_radio_pos = *pos ;
	entk->last_radio_map = hzx_id ;
}

/* 純粋攻撃兵チェック */
/* 純粋攻撃兵とはモデルが攻撃兵であること */
int ENE_PureAttacker( ACTION *act )
{
	if ( (act->bodyp.type & ENE_TYPE_ATTACKER) &&
		!(act->bodyp.type & ENE_TYPE_CONVERT) &&
		!(act->bodyp.type & ENE_TYPE_EVENT_A) 
		) {
		return 1 ;
	}
	return 0 ;
}

/* 純粋警備兵チェック */
int ENE_PureWatcher( ACTION *act )
{
	if ( (act->bodyp.type & ENE_TYPE_WATCHER) &&
		!(act->bodyp.type & ENE_TYPE_CONVERT) ) {
		return 1 ;
	}
	return 0 ;
}


/* 肩についた無線機を使う兵士タイプチェック*/
int ENE_UseShoulderRadio( ENETHINK *entk)
{
	switch( entk->name_id.body ) {
		case ENE_MDL_NAME_GPA :
		case ENE_MDL_NAME_GBA :
		case ENE_MDL_NAME_TNG :
		case ENE_MDL_NAME_HTC :
		case ENE_MDL_NAME_VR_ATK :
			/*インカム無線*/
			return 1 ;
			break ;
		default :
			/*腰無線*/
			return 0 ;
			break ;
	}
}

/* 近くの仲間が無線をかけている */
int ENE_OtherEnemyRadioAction( ENETHINK *entk )
{
	ENETHINK	*other ;
	int dis ;

	other = COM_NearEnemyThkStatus( entk, THK_STATUS_RADIO, &dis ) ;
	if ( other == NULL ) return 0 ;
	if ( dis > entk->sense.hearing ) return 0 ;
	
	return 1 ;
}

/* 重い！要改良 */
int	ENE_InSightEnemyFromID( ENETHINK *entk, int uniq_id, long64 status, long64 not_status )
{
	ENETHINK 	*trgentk ;
	EYEINFO		eyei ;

	trgentk = COM_GetEnemyFromUniqID( uniq_id ) ;

	if( trgentk == NULL ) return 0 ;

	if ( status != -1 && !(trgentk->act->status & status) ) return 0 ;
	if ( trgentk->act->status & not_status ) return 0 ;

	ENE_SetEyeInfo( &eyei, &trgentk->ctrl->mov, &trgentk->ctrl->addr, 0, &trgentk->ctrl->hzx_id ) ;
	ENE_EyeInfoCheck( entk, &eyei ) ;
printf(" sight[%d] dis[%d] dir[%d] \n",eyei.sight, eyei.dis, eyei.dir ) ;
	if ( (eyei.sight == EYE_INFO_SIGHT_IN) && (eyei.dis < entk->sense.eye_s) ) { /* 双眼鏡は無視 */

printf("[%d] accident_id[%d] target_id [%d] x[%f] y[%f] z[%f]\n",
entk->uniq_id, uniq_id, trgentk->uniq_id,trgentk->ctrl->mov.vx,trgentk->ctrl->mov.vy,trgentk->ctrl->mov.vz);

		return 1 ;
	}

	return 0 ;
}

#define	CORP_AREA	(750.0f)
#define	CORP_AREA_H	(1250.0f)
void ENE_PlayerOnCorp( FVECTOR *pos )
{
	float	f ;
	
	f = pos->vx - GM_PlayerPosition.vx ;
	if ( f > CORP_AREA || f < -CORP_AREA ) return ;
	f = pos->vz - GM_PlayerPosition.vz ;
	if ( f > CORP_AREA || f < -CORP_AREA ) return ;
	f = pos->vy - GM_PlayerPosition.vy ;
	if ( f < -CORP_AREA_H || f > 0.0f ) return ;

	/* 壁チェック */
	if ( HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, pos, &GM_PlayerPosition,
			HZX_CHK_ALL, HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return ;

	GM_PlayerStatus |= PLAYER_ON_CORPSE ;
}

void ENE_ChangeRoute( entk, route )
ENETHINK	*entk ;
int			route ;
{
	ROUTENAVI	*rnavi ;
	
	rnavi = entk->rnavi ;
	ENE_ReadNodes( rnavi, route ) ;
	rnavi->c_route = route ;
	rnavi->next_node = 0 ;
}

void ENE_ChangeRoute2( entk, route )
ENETHINK	*entk ;
int			route ;
{
	ROUTENAVI	*rnavi ;
	
	rnavi = entk->rnavi2 ;
	ENE_ReadNodes( rnavi, route ) ;
	rnavi->c_route = route ;
	rnavi->next_node = 0 ;
}

/* posが指定ゾーンのnearに入っているかチェック */
int	ENE_InNearZone( FVECTOR	*pos, int zone, int grpid )
{
	HZX_ZON		*z ;
	u_char		*nears ;
	int			i, near ;

	z = HZX_GetZone( grpid, zone ) ;
	nears = z->nears ;

	for ( i = 0 ; i < 6 ; i++ ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		if ( HZX_RouteCourseNearFlag( z->near_flag[i] ) ) continue ;
		if ( HZX_InsideZone( grpid, pos, near ) ) return near ;
	}
	return -1 ;
}

void	ENE_StandPosRintrpt( R_INTRPT *r_intrpt, int mot, FVECTOR *pos, int *dir )
{
	SVECTOR rot ;
	FVECTOR	*shift ;
//		static FVECTOR locker_open={ 1020.0f, 0.0f, 720.0f} ;
	static FVECTOR locker_open={ 1049.9f, 0.0f, 510.3f} ;
	
	shift = &DG_ZeroVector ;
	switch( mot ) {
		case MOT_RINTRPT_LOCKER_OPEN :
			shift = &locker_open ;
			/* r_intrpt->kind によって方向を変える */
			*dir = (r_intrpt->dir-1024) & 4095 ;
		break ;
		default :
			printf(" RINTRPT NO MOTION !!\n") ;
		break ;
	}
	
	rot.vx = 0 ;
	rot.vy = r_intrpt->dir ;
	rot.vz = 0 ;
	DG_SetPos2( &r_intrpt->pos, &rot ) ;
	DG_PutVector( shift, pos, 1 ) ;
}

void	ENE_TraceEnd( ENETHINK *entk )
{
	ENEFINDLIST	*efl ;

	efl = GM_GetEneFindList( entk->efl_id )  ;
	if ( efl != NULL ) UNSET_FLAG( efl->listtype, EF_LSIT_TYPE_TRACE ) ;
}

int ENE_EneFindSightIn( entk, ef, o_dis, o_dir )
ENETHINK	*entk;
ENEFIND *ef ;
int		*o_dis ;
int		*o_dir ;
{
	FVECTOR	vec, head, *pos ;
	int		sight_dis, sight_range, x_rot, diff, dis, dir ;
	SENSEPARAM *sens ;
	EYEINFO		*eyei ;

	eyei = &entk->pl_eyei ;
	sens = &entk->sense ;

//AN_Test_Eye2( &ef->pos, 1 );
	if ( eyei->flag & EYE_INFO_FLAG_SKIP ) return 0 ;
	if ( entk->act->status & ACT_STATUS_EYE_CLOSE ) return 0 ;

	/* ゾーン判定 */
	if ( ENE_ReadOnlinInfo( entk->ctrl->addr, ef->zoneaddr ) ) return 0 ;

//AN_Test_Eye2( &ef->pos, 1 );
	/* 視界判定は頭の位置から */
	KR_FMatToFvec( &BODYWORLD(entk->act->body, HUMAN21_ATAMA), &head ) ;
	pos = &head ;
	_sceVu0SubVector(  &vec, &ef->pos, pos ) ;
	*o_dis = dis = _FVecLen3( &vec ) ;
	*o_dir = dir = _FVecDir2( &vec ) ;

	if ( ef->type & EF_TYPE_DARK_AREA 	/* 暗闇 */
		&& entk->sw.n_sight != 2 ) {	/* 暗視ゴーグルを付けていない */
		sight_dis = sens->eye_s/4 ;
		sight_range = sens->eye_r ;
	} else {
		sight_dis = sens->eye_s ;
		sight_range = sens->eye_r ;
	}

	/* 視力判定 */
	if ( dis > sight_dis ) return 0 ;

	/* 視野判定 */
	if ( _DiffDirAbs( sens->facedir, dir ) > sight_range ) return 0 ;

	/* 高低差判定 */
	x_rot = _FVecDirX( &vec ) - 1024 ;
	diff = GV_DiffDirS( sens->facedir_x, x_rot ) ;
	if ( diff > UNDER_EYE_SIGHT || diff < UPPER_EYE_SIGHT ) return 0 ;

	/* オンライン判定 （将来はけしたい) */
	if ( ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &head, &ef->pos ) ) return 0 ;

	/* 段ボールで見えない */
	if ( GM_PlayerStatus & PLAYER_CB_BOX ) {
		if ( ENE_InRange( &ef->pos, &GM_PlayerPosition, 500 ) )	return 0 ;
	}

	return 1 ;
}

int ENE_EyeOnlineCheck( hzx_id, from, to )
int hzx_id ;
FVECTOR *from ;
FVECTOR *to ;
{
#ifdef DEBUG_MODE
#if 0
	{
			int		c, atr ;
			HZX_HZD	seg ;
			FVECTOR	v ;
			c = HZX_OnlineHazardCheck( hzx_id, from, to,
						HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES ) ;
			if ( c ) {
				HZX_GetOnlineHazard( &seg, &atr ) ;
				HZX_GetOnlinePoint( &v ) ;
				DumpVec( &v ) ;
				if ( seg.type == HZX_TYPE_SEGMENT ) {
					DumpSegment( &seg ) ;
				} else {
					DumpFloor( &seg ) ;
				}
			}
			return c ;
	}
#else
#if 0
{
	extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
	FVECTOR	lin[2] ;
	lin[0] = *from ;
	lin[1] = *to ;
	NewLineView(  &lin[0],10,0,0,255) ;
}
#endif
	return HZX_OnlineHazardCheck( hzx_id, from, to,
				HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ;
#endif
#else
	return HZX_OnlineHazardCheck( hzx_id, from, to,
				HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ;
#endif
}

int ENE_BodyOnlineCheck( hzx_id, from, to )
int hzx_id ;
FVECTOR *from ;
FVECTOR *to ;
{
	return HZX_OnlineHazardCheck( hzx_id, from, to,
				HZX_CHK_ALL, HZX_SEG_NO_ENEMY, HZX_FLOOR_NO_ENEMY ) ;
}

void ENE_ProcCall( args )
ENE_ARGS	*args ;
{
	GCL_ARGS arg ;
	
	if( args->proc == 0 ) return ;
	
	if ( args->argc == 0 ) {
		ENE_ExecProc( args->proc, NULL ) ;
	} else {
		arg.argc = args->argc ;
		arg.argv = &args->argv[0] ;
		ENE_ExecProc( args->proc, &arg ) ;
	}
}

void ENE_DeathProc( ENE_ARGS *args )
{
	ENE_ProcCall( args ) ;
}


/* 違うルートへワープ */
void ENE_RouteWarp( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;
	FVECTOR		pos ;

	rnavi = entk->rnavi ;
	if ( rnavi->next_route == rnavi->c_route ) return ;

	ENE_ChangeRoute( entk,rnavi->next_route ) ;

	pos.vx = rnavi->nodes[ (short)rnavi->next_node ].vx ;
	pos.vy = rnavi->nodes[ (short)rnavi->next_node ].vy+1000 ;
	pos.vz = rnavi->nodes[ (short)rnavi->next_node ].vz ;

	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, rnavi->mapbit[ (short)rnavi->next_node ] ) ;

	DG_SetPos2( &entk->ctrl->mov, &entk->ctrl->rot ) ;
	GM_ActObject2( entk->act->body );
}

/* 指定場所へワープ */
void ENE_PosWarp( entk, pos, hzx_id )
ENETHINK	*entk ;
FVECTOR		*pos ;
int			hzx_id ;
{
	HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( entk->ctrl, pos, hzx_id ) ;

	DG_SetPos2( &entk->ctrl->mov, &entk->ctrl->rot ) ;
	GM_ActObject2( entk->act->body );
}

/* 開始位置も表示されていないかチェックしてワープ */
void ENE_CheckWarpNearPos( body, ctrl, addr, dis )
OBJECT	*body ;
CONTROL	*ctrl ;
int		addr ;
int		dis ;
{
	if ( HZX_CurrentGroupID & ctrl->hzx_id ) {
		if ( DG_BoundCheck( &body->objs->world, 
			&body->objs->bound_max, &body->objs->bound_min ) >= 0 ) {
			return ;
		}
	}

	ENE_WarpNearPos( ctrl, addr, dis ) ;
}

/* 開始位置から目標位置へ画面内手前までワープ */
void ENE_WarpNearPos( ctrl, addr, dis )
CONTROL	*ctrl ;
int		addr ;
int		dis ;
{
	int	warp_zone ;
	FVECTOR	pos ;
	HZX_ZON		*zone ;

printf(" ene warp: fromaddr [%x] > trg [%x]\n",ctrl->addr, addr ) ;
	warp_zone = HZX_BoundOutZoneCrossGroup( addr, ctrl->addr, dis ) ;

printf(" warp: from [%x] > to [%x]\n",addr,warp_zone ) ;

	if ( warp_zone == ctrl->addr ) return ;
#if 1
	if ( ENE_ZoneIntrptCheck( warp_zone ) ) {
		printf(" ene warp to intrpt zone!!! \n");
		return ;
	}
#endif

	zone = HZX_GetZoneNo( HZX_ZoneMapNo(warp_zone), HZX_Zone1(warp_zone) ) ;
	pos.vx = (float)zone->x ;
	pos.vy = (float)zone->y+1000 ;
	pos.vz = (float)zone->z ;

	HZX_FlashTrap( ctrl->hzx_id, &ctrl->evt ) ;
	GM_ResetControlPositionAndGroup( ctrl, &pos, GM_GetBit( HZX_ZoneMapNo(warp_zone) ) ) ;
	if ( ctrl->object != NULL ) {
		DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
		GM_ActObject2( ctrl->object );
	}
}

int ENE_GetAddrToAddrDir( int from_addr, int to_addr )
{
	HZX_ZON		*z1, *z2 ;
	FVECTOR		pos1, pos2 ;

	z1 = HZX_GetZone( GM_GetBit( HZX_ZoneMapNo( from_addr ) ), HZX_Zone1( from_addr ) ) ;
	z2 = HZX_GetZone( GM_GetBit( HZX_ZoneMapNo( to_addr ) ), HZX_Zone1( to_addr ) ) ;

	pos1.vx = (float)z1->x ;
	pos1.vy = (float)z1->y ;
	pos1.vz = (float)z1->z ;

	pos2.vx = (float)z2->x ;
	pos2.vy = (float)z2->y ;
	pos2.vz = (float)z2->z ;

	return _FVecTrgDir2( &pos1, &pos2 ) ;
}

int ENE_GetZoneToZoneDir( from, to, mapbit )
int from ;
int to ;
int	mapbit ;
{
	HZX_ZON		*z1, *z2 ;
	FVECTOR		pos1, pos2 ;

	z1 = HZX_GetZone( mapbit, from ) ;
	z2 = HZX_GetZone( mapbit, to ) ;

	pos1.vx = (float)z1->x ;
	pos1.vy = (float)z1->y ;
	pos1.vz = (float)z1->z ;

	pos2.vx = (float)z2->x ;
	pos2.vy = (float)z2->y ;
	pos2.vz = (float)z2->z ;

	return _FVecTrgDir2( &pos1, &pos2 ) ;
}

/* 指定方向に一番近いニアゾーン */
int ENE_GetMinDirNearZone( zoneadd, dir )
HZX_ZONE_ADD zoneadd ;
int	dir ;
{
	HZX_ZON		*z ;
	u_char		*nears, minnear, near ;
	int rot, minrot, zone, g_id, i, diff ;

	zone = HZX_Zone1( zoneadd ) ;
	g_id = HZX_ZoneGroupID( zoneadd ) ;
	z = HZX_GetZoneFromAdd( zoneadd ) ;
	nears = z->nears ;

	minrot = 50000 ;
	minnear = -1 ;
	for ( i = 0 ; i < 6 ; i++ ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		rot = ENE_GetZoneToZoneDir( zone, near, g_id ) ;
		diff = _DiffDirAbs( dir, rot ) ;
		if ( HZX_RouteCourseNearFlag( z->near_flag[i] ) ) {
			diff = 5000 ;	/*中止コース*/
		}
		if( diff < minrot ) {
			minrot = diff ;
			minnear = near ;
		}
	}

	return HZX_Address( g_id, minnear, minnear ) ;
}

int ENE_GetWideNearZone( zone, which, mapbit )
int zone ;
int which ;
int	mapbit ;
{
	HZX_ZON		*z ;
	int			near_num ;
	u_char		*nears ;
	int		i, j, near, max_wide, wide  ;
	struct	_data_ {
	    short	near ;
	    short	rot ;
	} near_rot[6] ;
	struct _data_ near_rot_tmp ;

	z = HZX_GetZone( mapbit, zone ) ;
	nears = z->nears ;

	for ( i = 0 ; i < 6 ; i++ ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		near_rot[ i ].rot = ENE_GetZoneToZoneDir( zone, near, mapbit ) ;
		near_rot[ i ].near = near ;
	}
	near_num = i ;
	
	/* 方向角度の小さいもの順にソート */
	for ( ; i > 0 ; --i ) {
		j = 0 ;
		while( j < i-2 ) {
			if ( near_rot[ j ].rot > near_rot[ j+1 ].rot ) {
				near_rot_tmp = near_rot[ j ] ;
				near_rot[ j ] = near_rot[ j+1 ] ;
				near_rot[ j+1 ] = near_rot_tmp ;
			}
			j++;
		}
	}

	/* 最大方向間を検索 */
	j = i = 0 ;
	max_wide = 0 ;
	while( i < near_num-1 ) {
		wide = GV_DiffDirU( near_rot[ i ].rot, near_rot[ i+1 ].rot ) ;
		if ( wide > max_wide ) {
			max_wide = wide ;
			j = i ;
		}
		i++ ;
	}

	if ( !which ) return near_rot[ j ].near ;
printf(" wode (0) = [%d] \n",near_rot[ j ].near ) ;
	if( ++j >= near_num ) j = 0 ;
printf(" wode (1) = [%d] \n",near_rot[ j ].near ) ;

/*今はつかってないので無視！！コース違いに注意*/
	return near_rot[ j ].near ;
}

/* 指定の安全地帯 */
int	ENE_GetSafeZone( HZX_ZONE_ADD zoneaddr, int num )
{
	return HZX_GetZoneFromAdd(zoneaddr)->safes[ num ];
}

/* 安全地帯を二つに分けて最短距離で探索できる組み合わせの最適解を求める */
int	ENE_GetPearSafeZone( entk, which )
ENETHINK *entk ;
int		which ;
{
	int i, j ;
	int	safe_num, safe_zone[SAFE_NUM], zone, tmpsafe ;
	int zones[SAFE_NUM+1], tsp[SAFE_NUM+2] ;
	int min, r1, r2, dev, big ;

	entk->avoinfo.check_map = entk->ctrl->hzx_id ;

	zone = entk->ctrl->addr&255 ;

	safe_num = 0 ;
	zones[0] = zone ;
	for(i=0;i<SAFE_NUM;i++){
		/*候補地を取得*/
		tmpsafe = ENE_HZX_GetZone(entk->ctrl->addr)->safes[i];

		/*異常値*/
		if(tmpsafe == 255) break;
		{
			R_INTRPT	*r_intrpt ;
			int			addr ;

			addr = HZX_Address( entk->ctrl->hzx_id, tmpsafe, tmpsafe ) ;
			r_intrpt = GM_GetRIntrptZ2Z( entk->ctrl->addr, addr ) ;
			if ( r_intrpt != NULL ) {
printf("safe area intrpt[%x] \n", addr );
				continue ;
			}
		}

		safe_zone[ safe_num ] = tmpsafe ;
		zones[safe_num+1] = tmpsafe ;
printf("[%d]safe_num[%d],s_zone[%d]\n", i,safe_num, tmpsafe);
		safe_num ++ ;
	}

	if ( safe_num == 0 ) {
		return 0 ;
	}

	if ( safe_num == 1 ) {
		if ( !which ) {
			entk->avoinfo.check_zone[ 0 ] = zones[ 1 ] ;
			return safe_num ;
		} else {
			return 0 ;
		}
	}

	/* safe_zone×safe_zone の表を作成 */
printf("safe num[%d]\n",safe_num);
	for(i=0;i<safe_num+1;i++){
		TSP_map[i][i] = 0 ;
		for(j=i+1;j<safe_num+1;j++){

printf("kore1!!!![%d][%d] zones[%d], zones[%d]\n",i,j,zones[i], zones[j]);
			TSP_map[i][j] = HZX_GetRoute( entk->ctrl->hzx_id, zones[i], zones[j] ) ;
			TSP_map[j][i] = TSP_map[i][j] ;
		}
	}

	min = TSP( safe_num+1, tsp ) ;

	for(i = 0; i < safe_num+1; i++){
		printf("%2d->", tsp[i]);
	}
	min += TSP_map[ 0 ][ tsp[i] ] ;
	printf("%2d cost[%d]\n", tsp[i], min);

	/* 二つに分ける */
	min = 255 ;
	dev = 0 ;
	for(i=1;i<safe_num+1;i++){
		r1 = r2 = 0 ;
		for(j=0; j<i; j++){
			r1 += TSP_map[ tsp[j] ][ tsp[j+1] ] ;
		}
		r1 += TSP_map[ tsp[j] ][ tsp[0] ] ;	/* スタート地点までの距離 */
		if ( tsp[0] != tsp[i+1] ) {
			r2 += TSP_map[ tsp[0] ][ tsp[i+1] ] ;
			for(j=i+1; j<safe_num+1; j++){
				r2 += TSP_map[ tsp[j] ][ tsp[j+1] ] ;
			}
		}
printf("dev[%d] r1=%d r2=%d \n",i,r1,r2 ) ;
		big = ( r1 > r2 ) ? r1:r2 ;
		if ( big < min ) {
			min = big ;
			dev = i ;
		}
	}

	printf(" dev = %d \n",dev ) ;

	if ( !which ) {
		for( i=0; i<dev; i++ ){
			entk->avoinfo.check_zone[ i ] = zones[ tsp[i+1] ] ;
printf(" check zone [%d] > ",entk->avoinfo.check_zone[ i ] ) ;
		}
printf("0: sun[%d]\n",i);
		return i ;
	}

	for( i=0; i<safe_num-dev; i++ ){
		entk->avoinfo.check_zone[ i ] = zones[ tsp[dev+i+1] ] ;
printf(" check zone [%d] > ",entk->avoinfo.check_zone[ i ] ) ;
	}
printf("1: sun[%d]\n",i);
	return i ;
}

/* 今いるゾーンの安全地帯数を調べて、確認するゾーン数とする */
int	ENE_GetSafeZoneNum( entk )
ENETHINK *entk ;
{
	int i, safe_num, tmpsafe, zone ;
	
	zone = entk->ctrl->addr&255 ;
	
	safe_num = 0 ;
	entk->avoinfo.check_map = entk->ctrl->hzx_id ;
	for(i=0;i<SAFE_NUM;i++){
		/*候補地を取得*/
		tmpsafe = ENE_HZX_GetZone(entk->ctrl->addr)->safes[i];

printf("zone [%d]   safe zone [%d]\n",zone, tmpsafe);
		/*異常値*/
		if(tmpsafe == 255) break;
		entk->avoinfo.check_zone[ safe_num ] = tmpsafe ;
		safe_num ++ ;
	}

	entk->avoinfo.check_count = 0 ;
	return safe_num ;
}

int	ENE_RunawayZone( CONTROL *run, CONTROL *from )
{
	HZX_GROUP_ID run_id, from_id ;
	int run_zone, from_zone ;
	int	from_dir, i, safe, safe_next_zone, res ;
	HZX_ZONE_ADD from_near_zone, run_near_zone ;

printf(" runaway safe zone " ) ;

	run_id = GM_GetBit( HZX_ZoneMapNo( run->addr ) ) ;
	from_id = GM_GetBit( HZX_ZoneMapNo( from->addr ) ) ;
	run_zone  = HZX_Zone1( run->addr ) ;
	from_zone = HZX_Zone1( from->addr ) ;

	/* fromへ向うゾーン方向 */
	if ( run_id != from_id ) {
		from_dir = _FVecTrgDir2( &run->mov, &from->mov ) ;
	} else if ( run_zone == from_zone ) {
		from_dir = _FVecTrgDir2( &run->mov, &from->mov ) ;
	} else {
		from_near_zone = HZX_NextZoneCrossGroup( run->addr, from->addr ) ;
		from_dir = ENE_GetAddrToAddrDir( run->addr, from_near_zone ) ;
	}

printf(" dir [%d] ",from_dir ) ;

	from_dir += 2048 ;
	from_dir &= 4095 ;
	/* fromから離れる方向へのゾーン */
	run_near_zone = ENE_GetMinDirNearZone( run->addr, from_dir ) ;

printf(" run_near_zone [%x] \n",run_near_zone ) ;

printf(" next ") ;
	/* run_near_zoneを通るセーフゾーン検索 */
	res = 255 ;
	for(i=0;i<SAFE_NUM;i++){
		/*候補地を取得*/
		safe = ENE_HZX_GetZone(run->addr)->safes[i];
		/*異常値*/
		if(safe == 255) break;
		{
			R_INTRPT	*r_intrpt ;
			int			addr ;

			addr = HZX_Address( run_id, safe, safe ) ;
			r_intrpt = GM_GetRIntrptZ2Z( run->addr, addr ) ;
			if ( r_intrpt != NULL ) {
printf("run away safe area intrpt[%x] \n", addr );
				continue ;
			}
		}

		res = safe ;

		safe_next_zone = HZX_NextZone( run_id, run_zone, res ) ;
printf(" [%d] %d>%d ",i,res,safe_next_zone ) ;
		if ( run_id == GM_GetBit( HZX_ZoneMapNo( run_near_zone ) ) && 
				safe_next_zone == HZX_Zone1(run_near_zone) ) {
			return res ;
		}
	}
	if ( res == 255 ) return HZX_Zone1( run->addr ) ;

	printf(" runaway no safe zone \n" ) ;
#if 1
	return HZX_Zone1( run->addr ) ;
#else
	return res ;
#endif
}

int	ENE_RunawayZoneaddr( CONTROL *run, CONTROL *from )
{
	HZX_GROUP_ID run_id, from_id ;
	int run_zone, from_zone ;
	int	from_dir, i, safe, safe_next_zone, res ;
	HZX_ZONE_ADD from_near_zone, run_near_zone ;

printf(" runaway safe zone " ) ;

	run_id = GM_GetBit( HZX_ZoneMapNo( run->addr ) ) ;
	from_id = GM_GetBit( HZX_ZoneMapNo( from->addr ) ) ;
	run_zone  = HZX_Zone1( run->addr ) ;
	from_zone = HZX_Zone1( from->addr ) ;

	/* fromへ向うゾーン方向 */
	if ( run_id != from_id ) {
		from_dir = _FVecTrgDir2( &run->mov, &from->mov ) ;
	} else if ( run_zone == from_zone ) {
		from_dir = _FVecTrgDir2( &run->mov, &from->mov ) ;
	} else {
		from_near_zone = HZX_NextZoneCrossGroup( run->addr, from->addr ) ;
		from_dir = ENE_GetAddrToAddrDir( run->addr, from_near_zone ) ;
	}

printf(" dir [%d] ",from_dir ) ;

	from_dir += 2048 ;
	from_dir &= 4095 ;
	/* fromから離れる方向へのゾーン */
	run_near_zone = ENE_GetMinDirNearZone( run->addr, from_dir ) ;

printf(" run_near_zone [%x] \n",run_near_zone ) ;

printf(" next ") ;
	/* run_near_zoneを通るセーフゾーン検索 */
	res = 255 ;
	for(i=0;i<SAFE_NUM;i++){
		/*候補地を取得*/
		safe = ENE_HZX_GetZone(run->addr)->safes[i];
		/*異常値*/
		if(safe == 255) break;
		{
			R_INTRPT	*r_intrpt ;
			int			addr ;

			addr = HZX_Address( run_id, safe, safe ) ;
			r_intrpt = GM_GetRIntrptZ2Z( run->addr, addr ) ;
			if ( r_intrpt != NULL ) {
printf("run away safe area intrpt[%x] \n", addr );
				continue ;
			}
		}

		res = safe ;

		safe_next_zone = HZX_NextZone( run_id, run_zone, res ) ;
printf(" [%d] %d>%d ",i,res,safe_next_zone ) ;
		if ( run_id == GM_GetBit( HZX_ZoneMapNo( run_near_zone ) ) && 
				safe_next_zone == HZX_Zone1(run_near_zone) ) {
			return HZX_Address( run_id, res, res ) ;
		}
	}
	if ( res == 255 ) return run->addr ;

	printf(" runaway no safe zone \n" ) ;

	return run->addr ;
}

int ENE_GetTrgRoute( entk )
ENETHINK	*entk ;
{
	int	route ;
	
	route = HZX_GetRouteCrossGroup( entk->ctrl->addr, entk->trgpoint.addr ) ;
	
	return route ;
}


int ENE_GetNearZone( zone, near, mapbit )
int		zone ;
int		near ;
int		mapbit ;
{
	HZX_ZON		*z ;

	z = HZX_GetZone( mapbit, zone ) ;
	return ( int )z->nears[ near ] ;
}


/* 指定ゾーンの場所 */
void ENE_ZonePos( pos, zone, mapbit )
FVECTOR	*pos ;
int		zone ;
int		mapbit ;
{
	HZX_ZON		*z ;

	z = HZX_GetZone( mapbit, zone ) ;
	pos->vx = (float)z->x ;
	pos->vy = (float)z->y ;
	pos->vz = (float)z->z ;
}
void ENE_Zoneadd2Pos( pos, zoneaddr )
FVECTOR	*pos ;
int		zoneaddr ;
{
	HZX_ZON		*z ;

	z = HZX_GetZoneFromAdd( zoneaddr ) ;
	pos->vx = (float)z->x ;
	pos->vy = (float)z->y ;
	pos->vz = (float)z->z ;
}

/* 指定ゾーンへの方向 */
int ENE_ZoneDir( pos, zone, mapbit )
FVECTOR	*pos ;
int		zone ;
int		mapbit ;
{
	HZX_ZON		*z ;
	FVECTOR		z_pos ;

	z = HZX_GetZone( mapbit, zone ) ;
	z_pos.vx = (float)z->x ;
	z_pos.vy = (float)z->y ;
	z_pos.vz = (float)z->z ;

	return _FVecTrgDir2( pos, &z_pos ) ;
}

/* 床傾き計算 */
int	ENE_GetGRot( CONTROL *ctrl, float z )
{
    FVECTOR	head, vec ;
    float	hh, diff ;
    int		turn ;	

    if ( ( ctrl->level[ 0 ] == NULL ) ) return 0 ;

    DG_SetPos2( &(ctrl->mov), &(ctrl->rot) ) ;
    head.vx = head.vy = 0.0F ;
    head.vz = z ; 
    DG_PutVector( &head, &head, 1 ) ;
    HZX_SlopeFloorLevel( &hh, &head, ctrl->level[ 0 ] ) ;
//printf(" hh=%f head=%f floor.y=%f!!\n",hh, head.vz, ctrl->levels[0]);
    diff = hh - ctrl->levels[ 0 ] ;
    vec.vx = diff ; 
    vec.vz = z ;
    turn = - GV_VecDir2( &vec ) ;
    if ( turn < -2048 ) turn += 4096 ;

    return turn ;
}

void	ENE_SetHeadMark( act, body_num, mark_num )
ACTION	*act ;
int body_num ;
int mark_num ;
{
	act->sw->headmark = mark_num ;

//	if( mark_num == HEADMARK_BW || 
//		mark_num == HEADMARK_QW ){
		act->sw->headmark |= HEADMARK_TARGET ;
//	}
//printf("headmark [%x]\n",mark_num ) ;
	
	CallActHeadMarks( act->headmarkwork, act->sw->headmark );

}


/* 傾く */
void	ENE_Incline( ctrl )
CONTROL		*ctrl ;
{
    int		incline ;

    incline = GV_DiffDirS( ctrl->turn.vy, ctrl->rot.vy ) ;
    if ( incline > 128 ) incline = 128 ;
    else if ( incline < -128 ) incline = -128 ;
    ctrl->turn.vz = incline ;    
}


void ENE_SetIk( entk )
ENETHINK	*entk ;
{
	extern void DummyFreeCorpse( void * ) ;
//	extern void Arm_IKcalc(OBJECT *, int, OBJECT *, int, FVECTOR * ) ;
	extern int FreeCorpse(void *) ;
	extern void PullCorpseWithShoulder(void *);
	extern void PullCorpseWithLegs(void *);


//	static	FVECTOR	shift = {87.5f, -110.0f , -97.5f, 1.0f} ;
	ACTION	*act ;
	
	act = entk->act ;
#ifdef DEBUG_MODE
if( GM_DebugModeEnable ){
	if ( entk->com->status & CMST_ENEMY_IK_STOP ) {
		DummyFreeCorpse( entk->act->ik ) ;
		return ;
	}
}
#endif
	if ( act->status & ACT_STATUS_IK_MUKADE ) {
#if 0
		static	FVECTOR	shift = {120.0f, -120.0f , -140.0f, 1.0f} ;
		Arm_IKcalc(entk->act->body, HUMAN21_HIDARI_TE, entk->buddy->act->body, HUMAN21_HIDARI_KATA, &shift) ;
#else
		if(entk->arm_ik != NULL ){
			MoveArmIK(entk->arm_ik, 1);
		}
#endif
	} else if ( act->status & ACT_STATUS_IK_DOWN ) {
		if ( entk->act->ik_time ) {
			entk->act->ik_time = AfterJumpCorpse( entk->act->ik ) ;
		} else {
			FreeCorpse( entk->act->ik ) ;
		}
	} else if ( act->status & ACT_STATUS_IK_FOOT ) {
		PullCorpseWithShoulder( entk->act->ik ) ;
	} else if ( act->status & ACT_STATUS_IK_HAND ) {
		PullCorpseWithLegs( entk->act->ik ) ;
	} else {
		if ( entk->act->ik_time ) {
//			entk->ik_time = AfterJumpCorpse( entk->act->ik ) ;
entk->act->ik_time = 0 ;
		} else {
			DummyFreeCorpse( entk->act->ik ) ;
		}
	}
}

void ENE_NewSetIk( ENETHINK	*entk )
{
	ACTION	*act ;

	act = entk->act ;

	if ( act->status & ACT_STATUS_IK_DOWN ) {
		TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->new_ik, 0 );
	} else if ( act->status & ACT_STATUS_IK_FOOT ) {
		TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->new_ik, 1 );
	} else if ( act->status & ACT_STATUS_IK_HAND ) {
		TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->new_ik, 2 );
	} else {
		TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->new_ik, -1 );
	}

	if ( act->status & ACT_STATUS_IK_PIKU ) {
		if ( act->bodyp.n_damobj == HUMAN21_MUNE || act->bodyp.n_damobj == HUMAN21_ATAMA 
		|| act->bodyp.n_damobj == HUMAN21_KOSHI || act->bodyp.n_damobj == HUMAN21_ONAKA ) {
			TAKABE_UtilPuppetIK_Piku( act->new_ik, -1 );
		} else {
			TAKABE_UtilPuppetIK_Piku( act->new_ik, act->bodyp.n_damobj );
		}
	}

	TAKABE_ActPuppetIK( act->new_ik );

}

int ENE_PlayerHoldCheck( entk )
ENETHINK	*entk ;
{
	int dir ;

	if ( entk->pl_eyei.dis > ENE_HOLD_DIS ) return 0 ;

	if ( !(GM_PlayerStatus & PLAYER_HOLD) ) return 0 ;

	dir = GV_DiffDirAbs( entk->pl_eyei.dir, GM_PlayerControl->rot.vy ) ;
	if ( dir < 1536 ) return 0 ;

//printf("dis[%d] dir[%d]\n", entk->pl_eyei.dis, dir ) ;
	return 1 ;
}

void ENE_SetBodyTargetSize( trg, n )
TARGET	*trg ;
int		n ;
{
#if 1
	GM_SetTargetSize( trg, &EneBodyTrgSize[ n ] ) ;
#else
	trg->size = EneBodyTrgSize[ n ] ;
#endif
}

int ENE_SetOffenseTarget( act, n )
ACTION	*act ;
int		n ;
{
	static FVECTOR kick_force = { 0.0F,-100.0F,100.0F } ;
//	static FVECTOR punch_force = { 0.0F,-200.0F,400.0F } ;
//	static FVECTOR sholder_force = { 0.0F,-200.0F,400.0F } ;
	static FVECTOR normal_size = { 500.0F,500.0F,500.0F } ;

	FVECTOR	*force, *size ;
	TARGET	*off ;
	POWER_TARGET	*power ;
	FVECTOR			v ;
	int		n_obj ;
	long64	type ;

	off = &( act->offense ) ;
	power = &( act->off_pow ) ;

	n_obj = HUMAN21_MIGI_KAKATO ;
	size = &normal_size ;
	force = &kick_force ;
	type = WP_PUNCH ;


	switch( n ) {
		case EM_attack_near :
			n_obj = HUMAN21_MIGI_ASHI2 ;
		break ;
#if 0
		case EM_near_punch :
			n_obj = HUMAN21_MIGI_UDE1 ;
		break ;
		case EM_near_shoulder :
			n_obj = HUMAN21_MIGI_KATA ;
		break ;
#endif
		case EM_htc_ak_nom_okiru_f_slow :
		case EM_okiru_aomuke_slow :
		case EM_okiru_aomuke :
		case EM_okiru_utubuse_slow :
		case EM_okiru_utubuse :
			type = WP_WALLCRASH ;
			n_obj = HUMAN21_KOSHI ;
			force = &DG_ZeroVector ;
		break ;
		case EM_ak_attack_near_kick_r :
			n_obj = HUMAN21_MIGI_TSUMASAKI ;
		break ;
		case EM_ak_attack_near_kick_l :
			n_obj = HUMAN21_HIDARI_TSUMASAKI ;
		break ;
		case EM_ak_attack_near_punch_r :
			n_obj = HUMAN21_MIGI_TE ;
		break ;
		case EM_ak_attack_near_punch_l :
			n_obj = HUMAN21_HIDARI_TE ;
		break ;
		case EM_nom_stomp :
			n_obj = HUMAN21_MIGI_TSUMASAKI ;
		break ;
		/*盾兵*/
		case EM_shl_nom_attack_near_shl : 
			n_obj = HUMAN21_HIDARI_TE ;
		break ;
		case EM_shl_nom_attack_near : 
			n_obj = HUMAN21_MIGI_TSUMASAKI ;
		break ;
	}
//	GM_SetTarget( off, TARGET_OFFENSE | TARGET_CHECK_ONE, 0, PLAYER_SIDE, size, &DG_ZeroVector ) ;
	GM_SetTarget( off, TARGET_OFFENSE , 0, PLAYER_SIDE, size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, type|WP_NOPLAYER ) ; 
	GM_MoveTarget3( off, &( BODYWORLD(act->body, n_obj ) ) ) ;
	DG_SetPos2( &act->ctrl->mov, &act->ctrl->rot ) ;
    DG_RotVector( force, &v, 1 ) ;

	GM_SetPowerTarget( off, power, POWER_ONCE, 255, 0, ENE_PUNCH_DMG, &v ) ;

//	GM_SetTargetCallBack( off, ComboCallBack, ( void * )work ) ;
	GM_PutTarget( off ) ;
#if 0
	NewTargetView( off, 34, 184, 200 ) ;
#endif
	return 0 ;
}

/* 同士討ち 
n=0 寝てる兵を起こす蹴り
n=1 ボディアタック
*/
int ENE_SetOffenseTargetEne( act, n )
ACTION	*act ;
int		n ;	/* 0:keri 1:body */
{
	static FVECTOR force1 = { 0.0F,200.0F,0.0F } ;
	static FVECTOR shift1 = { 0.0F,200.0F,600.0F } ;
	static FVECTOR size1 = { 300.0F,300.0F,300.0F } ;
//	static FVECTOR size2 = { 1300.0F,1300.0F,1300.0F } ;
	TARGET	*off ;
	POWER_TARGET	*power ;
	FVECTOR			v, *f, *sz, *sh ;
	int				flag, obj ;
	long64			weapon_type ;

	off = &( act->offense ) ;
	power = &( act->off_pow ) ;
	obj = 0 ;

	flag = TARGET_OFFENSE ;
	f = &DG_ZeroVector ;
	sz = &size1 ;
	sh = &DG_ZeroVector ;
	weapon_type = WP_NOPLAYER ;
	if ( n == 0 ) {
		/* 寝ている兵を起こす蹴り */
		flag |= TARGET_CHECK_ONE|TARGET_CHILD ;
		obj = HUMAN21_HIDARI_TSUMASAKI ;
		sh = &shift1 ;
		weapon_type |= WP_STAMP ;
	} else if ( n == 1 ) {
		/* ボディアタック */
		f = &force1 ;
		weapon_type |= WP_KICK ;
	} else if ( n == 2 ) {
		/* 落下アタック */
		f = &force1 ;
		weapon_type |= WP_STAMP ;
	}
	GM_SetTarget( off, flag, 0, ENEMY_SIDE, sz, f ) ;
	GM_SetTargetWeaponType( off, weapon_type ) ; 	

	GM_MoveTarget3( off, &( BODYWORLD(act->body, obj ) ) ) ;

	DG_SetPos2( &act->ctrl->mov, &act->ctrl->rot ) ;
    DG_RotVector( sh, &v, 1 ) ;

	GM_SetPowerTarget( off, power, POWER_ONCE, 255, 0, 0, &v ) ;
	GM_PutTarget( off ) ;

	return 0 ;
}

/*特定の相手に必ずあたる同士討ち*/
void ENE_SetTargetEneDirect( act,trg, mode )
ACTION	*act ;	/*自分*/
TARGET	*trg ;		/*目標*/
int		mode ;	/* 0:踏み */
{
	TARGET	*off ;
	POWER_TARGET	*power ;
	int				flag ;

	off = &( act->offense ) ;
	power = &( act->off_pow ) ;

//	flag = TARGET_CHECK_ONE|TARGET_CHILD ;
//	flag = TARGET_CHECK_ONE ;
	flag = TARGET_OFFENSE ;

	GM_SetTarget( off, flag, 0, ENEMY_SIDE, 
		&DG_ZeroVector, &DG_ZeroVector ) ;
	switch (mode){
		case 0 :
		GM_SetTargetWeaponType( off, WP_STAMP|WP_NOPLAYER ) ; 	/* 寝ている兵を起こす蹴り */
		break;
	}
	GM_SetPowerTarget( off, power, POWER_ONCE, 255, 0, 0, 
		&DG_ZeroVector ) ;
	GM_PutTarget( off ) ;
	/* 直接指定で特定のターゲットに攻撃を当てる */
	GM_TargetSetDirectAttack( off, trg );
//printf("DIRECT TARGET SET!!!!\n");
}

void ENE_RefreshParam( entk )
ENETHINK	*entk ;
{
	BODYPARAM	*bodyp ;
	int		i ;

	bodyp = &entk->act->bodyp ;

	bodyp->life = bodyp->m_life ;
	bodyp->faint = bodyp->m_faint ;
	bodyp->blood = bodyp->m_blood ;
	bodyp->anesthesia = bodyp->m_anesthesia ;

	bodyp->pbreak = 0 ;
	bodyp->loss_blood = 0 ;
	bodyp->faint_time = 0 ;
	bodyp->dam_level_num[0]=0 ;
	bodyp->dam_level_num[1]=0 ;
	bodyp->dam_level_num[2]=0 ;
	bodyp->dam_level_num[3]=0 ;
	bodyp->ane_level_num[0]=0 ;
	bodyp->ane_level_num[1]=0 ;
	bodyp->ane_level_num[2]=0 ;
	bodyp->ane_level_num[3]=0 ;

	for(i=0; i<MAX_DURABLE_AREA; i++ ) {
		bodyp->durable[i] = bodyp->max_durable[i] ;
    }
	entk->act->sw->eye_anim = EYE_NORMAL ;

}

#ifdef TARGET_PARTS_IS_LINK_LIST
TARGET_PARTS	*KR_GetTargetPart( TARGET *trg, int	i )
{
	TARGET_PARTS	*parts ;

	parts = trg->parts ;
	while( parts != NULL ) {
		if ( parts->level == i ) return parts ;
		parts = parts->next ;
	}

	return NULL ;
}

#endif

int ENE_Target2DurableArea( int p, int n )
{
	if( p ==  PTARGET_LEVEL1 ) {
			if( n == PTARGET_HEAD ) return DURABLE_AREA0 ;
			else 					return DURABLE_AREA1 ;
	} else if ( p == PTARGET_LEVEL2 ) {
		return DURABLE_AREA2 ;
	}
	return DURABLE_AREA1 ;
}

/* 処理するターゲットの順番に注意 */
/* 子ターゲット＞パワーターゲット＞キャプチャーターゲット＞タッチターゲット */
/* 返り値０が優先 (おかしいかな)*/

/* 変装見破り攻撃 */
#define WP_IKNOW_DRESSED	(WP_BULLET|WP_COLDSPRAY|WP_WPNONE|WP_BLADALL)
static int DamageCheck( entk )
ENETHINK	*entk ;
{
	TARGET_PARTS	*parts ;
	CAPTURE_TARGET	*cap ;
	TARGET			*def, *child ;
	int i, n, dur ;

	/* パワーターゲット */
	def = &(entk->act->bodyp.deftrg) ;

	/* 子ターゲットチェック */
	for ( i = 1; i < MAX_TARGET_PARTS_LEVELS; i ++ ) {
		parts = KR_GetTargetPart( def, i ) ;
		if ( parts == NULL || ( parts->flag & TARGET_SKIP ) ) continue ;
		n = parts->n_parts ;
		child = ( TARGET * )parts->parts ;
		while( -- n >= 0 ) {
			if ( child->damaged & TARGET_POWER ) {
				printf(" hit level[%d] n[%d] wp[%lx]\n", i,parts->n_parts-n-1,child->weapon_type);
				dur = ENE_Target2DurableArea( i, (parts->n_parts-1)-n ) ;
				if ( child->weapon_type & (WP_BULLET) ) {
printf("dur [%d] val[%d] \n",dur, entk->act->bodyp.durable[ dur ] ) ;
					if( entk->act->bodyp.durable[ dur ] > 0 ) {
						return 0 ;
					}
				}
				if ( child->weapon_type & (WP_M92|WP_MECABREAK|WP_THROWG) ) {
					return 0 ;
				}
				if ( child->weapon_type & WP_BLAST ) {
					entk->act->bodyp.dammode = DAM_MODE_BOMB ;
				} else {
					entk->act->bodyp.dammode = DAM_MODE_BULLET ;
				}
				/* 変装見破りフラグ*/
				/* 仲間からの攻撃の場合大丈夫か*/
				if ( child->weapon_type & WP_IKNOW_DRESSED ) {
					if ( entk->pl_eyei.dis < entk->sense.eye_s ) {
						SET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
					}
				}
				return 1 ;
			}
			child ++ ;
		}
	}

	/* 親ターゲットチェック */
	if ( def->damaged & (TARGET_POWER) ) {
		if ( def->weapon_type & (WP_M92|WP_MECABREAK|WP_THROWG) ) {
			return 0 ;
		}
		if ( def->weapon_type & (WP_BULLET) ) {
			if( entk->act->bodyp.durable[ DURABLE_AREA1 ] > 0 ) {
				return 0 ;
			}
		}
		if ( def->weapon_type & (WP_HEADMARK) ) {
			entk->act->bodyp.dammode = DAM_MODE_GOOFY ;
		} else if ( def->weapon_type & (WP_STEAM|WP_EXTINGUISHER) ) {
			entk->act->bodyp.dammode = DAM_MODE_SMOKE ;
		} else {
			entk->act->bodyp.dammode = DAM_MODE_BULLET ;
		}
		/* 変装見破りフラグ*/
		if ( def->weapon_type & WP_IKNOW_DRESSED ) {
			if ( entk->pl_eyei.dis < entk->sense.eye_s ) {
				SET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
			}
		}
		return 1 ;
	}

	/* スタンダメージ */
	if ( entk->act->status & ACT_STATUS_DAMAGE_NOW ) {
		entk->act->bodyp.dammode = DAM_MODE_BOMB ;
		return 1 ;
	}

	/* キャプチャーターゲット */
	cap = &(entk->act->bodyp.capture) ;
    if ( cap->capture != NULL ) { /* 捕まった */
		if ( cap->flag & CAPTURE_HANG ) {	/* 首絞め */
			entk->act->bodyp.dammode = DAM_MODE_HANG ;
			/* 変装見破りフラグ*/
			SET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
			return 1 ;
		} else  if ( cap->flag & CAPTURE_THROW ) {	/* 投げ */
			entk->act->bodyp.dammode = DAM_MODE_THROW ;
			/* 変装見破りフラグ*/
			SET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
			return 1 ;
		}
	}

	if ( entk->act->status & ACT_STATUS_MASUI_KIKU ) {
		entk->act->bodyp.dammode = DAM_MODE_MASUI_DOWN ;
		return 1 ;
	}

	if ( entk->act->status & ACT_STATUS_BLEDTODEATH ) {
		entk->act->bodyp.dammode = DAM_MODE_BLEDTODEATH ;
		return 1 ;
	}

	/* 落下死 */
	if ( entk->act->status_status & ACT_STST_DROP ) {
		entk->act->bodyp.dammode = DAM_MODE_DROP ;
		return 1 ;
	}

	/* プッシュターゲット */
	def = &(entk->act->bodyp.pushtrg) ;
	if ( def->damaged & TARGET_TOUCH ) {
		entk->act->bodyp.dammode = DAM_MODE_BULLET ;
		/* 変装見破りフラグ*/
		SET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
		return 1 ;
	}

	return 0 ;
}

static int ExitCheck( entk )
ENETHINK	*entk ;
{
	/* ホールドアップで退場 */
	if ( entk->act->bodyp.type & ENE_TYPE_HOLD_EXIT ) {
		if ( entk->thk_status & THK_STATUS_HOLD_UP ) {
			entk->act->bodyp.dammode = DAM_MODE_EXIT ;
			return 1 ;
		}
	}

	return 0 ;
}

/* noticeのフラグリセットをやるために改良 */
int ENE_DamageCheck( ENETHINK *entk )
{
	if ( DamageCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_HOLDUP )  UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
		return 1 ;
	}
	
	if ( ExitCheck( entk ) ) {
		if ( entk->c_notice & ENE_NOTICE_HOLDUP )  UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
		return 1 ;
	}

	return 0 ;
}


int ENE_DirectReachCheck( entk )
ENETHINK	*entk ;
{
	int	reach ;

	entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id,&entk->znavi->flore_pos,entk->znavi->this_addr ) ;
	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );
	if( reach > HZX_DIRECT_REACH ) {
		return 0 ;
	}
	return 1 ;
}


void _RotToQuatXAfterY( rot, quat )
SVECTOR	*rot ;
FVECTOR	*quat ;
{
	FVECTOR	radi ;
	float	r ;

	vu0_PSDegV2RadV( rot, &radi ) ;

	quat->vw = (float)cosf( radi.vx / 2.0F );
	r = bp_sqrtf( 1.0F - quat->vw * quat->vw ) ; //BP_MATH - emulate PS2 sqrtf

	if ( radi.vx < 0 )	r = -r ;

	quat->vx = cosf( radi.vy ) * r ;
	quat->vz = -sinf( radi.vy ) * r ;
	quat->vy = 0.0f ;
}

void ENE_AdjustPiku( ENETHINK	*entk )
{
	static short asjpiku_buff[]={ 16, 64, 128, 192, 256, 128} ;
	SVECTOR	rot ;
   short adj_piku_value;

   // Arm fix (for 30fps):
   // if value > 0, use it, otherwise use time
   // part of a fix for MGSTWO-3181
   if (entk->act->adj_piku_value > 0)
   {
      adj_piku_value = entk->act->adj_piku_value;
   }
   else
   {
      adj_piku_value = entk->act->adj_piku_time;
   }

	entk->act->adj_piku_time -- ;
   adj_piku_value -- ;

   // Arm fix (for 30fps):
   // reset the piku_value
   // part of a fix for MGSTWO-3181
   if (entk->act->adj_piku_time == 0)
   {
      entk->act->adj_piku_value = 0;
   }

	rot.vx = -asjpiku_buff[ adj_piku_value ] ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	GM_AdjustRotBody( entk->act->body, &rot, 1  ) ;
	rot.vx = asjpiku_buff[ adj_piku_value ] ;
	GM_AdjustRotBody( entk->act->body, &rot, 11  ) ;
	GM_AdjustRotBody( entk->act->body, &rot, 3  ) ;
	GM_AdjustRotBody( entk->act->body, &rot, 7  ) ;
}

void ENE_SetAim( entk, pos ) 
ENETHINK	*entk ;
FVECTOR	*pos ;
{
	entk->act->aim_pos = *pos ;
	entk->status2 |= ENE_STATUS2_AIM_GUN ;
}

void ENE_SetAimPlayer( entk )
ENETHINK	*entk ;
{
	int		obj_num ;
	
	if ( GM_PlayerStatus & PLAYER_WATCH ) {
		entk->act->aim_pos = GM_PlayerFindPos ;
	} else {
		if ( GM_PlayerStatus & PLAYER_GROUND ) {
			obj_num = HUMAN21_KOSHI ;
		} else {
			obj_num = HUMAN21_ATAMA ;
		}
		KR_FMatToFvec( &BODYWORLD( GM_PlayerBody, obj_num), &(entk->act->aim_pos) ) ;
	}

/*
	if ( entk->sw.n_sight == 2 ) {
		w = &(BODYWORLD( GM_PlayerBody, HUMAN21_ATAMA )) ;
	} else {
		if ( entk->id%2 ) {
			if ( entk->act->status & ACT_STATUS_SHAGAMI ){
				w = &(BODYWORLD( GM_PlayerBody, HUMAN21_MUNE )) ;
			} else {
				w = &(BODYWORLD( GM_PlayerBody, HUMAN21_ATAMA )) ;
			}
		} else {
			w = &(BODYWORLD( GM_PlayerBody, HUMAN21_MUNE )) ;
		}
	}

*/
}


void ENE_GunAdjust( entk )
ENETHINK	*entk;
{
	FVECTOR		trg, pos ;
	FMATRIX		*w ;

	trg = entk->act->aim_pos ;
	if ( entk->act->status & ACT_STATUS_SHOT_PIKU )	trg.vy += 200 ;	/* 手振れ */

	w = &(entk->act->body->objs->objs[HUMAN21_KUBI].world) ;
	KR_FMatToFvec( w, &pos ) ;

	if ( entk->status2 & ENE_STATUS2_AIM_NO_LIMIT ) {
		GM_TrgToNearRot( &pos, entk->ctrl->rot.vy, &trg, &entk->act->adj_rot ) ;
	} else {
//		GM_TrgToNearRotLimit( &pos, entk->ctrl->rot.vy, &trg, &entk->act->adj_rot ) ;
		GM_TrgToNearRotLimit2( &pos, entk->ctrl->rot.vy, &trg, &entk->act->adj_rot ) ;
	}

//if ( entk->id == 0 ) printf(" rot vx = [%d] vy[%d]\n",entk->act->adj_rot.vx,entk->act->adj_rot.vy ) ;
//	GM_MotionAdjustWest( entk->act->body, &entk->act->adj_rot, entk->ctrl->rot.vy, ADJ_PARTS_HOMING_XY ) ;
	GM_MotionAdjustWest( entk->act->body, &entk->act->adj_rot, entk->ctrl->rot.vy, 15 ) ;
/*
	if ( entk->sw.n_sight == 2 ) {
		SVECTOR	head ;
		GM_MotionAdjust21( entk->act->body, &entk->act->adj_rot, entk->ctrl->rot.vy, 14 ) ;
		
		head.vx = entk->act->adj_rot.vx -128;
		head.vy = entk->act->adj_rot.vy  ;
		head.vz = entk->act->adj_rot.vz ;
		head.pad = entk->act->adj_rot.pad ;
		GM_MotionAdjust21( entk->act->body, &head, entk->ctrl->rot.vy, ADJ_PARTS_HEAD ) ;
	} else {
		GM_MotionAdjustWest( entk->act->body, &entk->act->adj_rot, entk->ctrl->rot.vy, 7 ) ;
	}
*/
}

#define DEB_G_ID	50
#define DEB_U_ID	0
#define	DEB_ID		1
void ENE_AdjustAimPoint( entk )
ENETHINK	*entk;
{
	static	ROTLIMIT	no_rlim = { 2047, 2047, 2047, 2047 } ;
	static	ROTLIMIT	rlim = { 1024, 1024, 768, 768 } ;
	static	ROTLIMIT	s_rlim = { 1024, 1024, 768, 384 } ;
	static	ROTLIMIT	*rlimit ;
	FVECTOR		trg, pos ;
	FMATRIX		*w ;
	ACTION		*act ;
	int			flag ;
	int			interp, speed ;
	
	act = entk->act ;
//	interp = 0 ;
	interp = 2 ;
	speed = 0 ;
	if ( (entk->status2 & ENE_STATUS2_AIM) ){
		trg.vx = act->aim_pos.vx ;
		trg.vy = act->aim_pos.vy ;
		trg.vz = act->aim_pos.vz ;
#ifdef DEBUG_MODE
if ( GM_Debug2PMode == GM_DEBUG_MODE_ENEMY_THINK ) {
extern void AN_Test_Eye2( FVECTOR *, int );

static FVECTOR	s={0.0,0.0,1000.0} ;
FMATRIX	m ;

w = &(act->body->objs->objs[12].world) ;
DG_SetPos( w ) ;
DG_MovePos( &s ) ;
DG_GetPos( &m ) ;
KR_FMatToFvec( &m, &pos ) ;
AN_Test_Eye2( &pos, 1 );

AN_Test_Eye2( &trg, 1 );
if ( entk->g_id == DEB_G_ID && entk->u_id == DEB_U_ID && entk->id == DEB_ID){
printf("\n trg vx[%.2f] vy[%.2f] vz[%.2f]\n",trg.vx, trg.vy, trg.vz ) ;
}
}
#endif

		w = &(act->body->objs->objs[HUMAN21_KUBI].world) ;
		pos.vy = w->m[3][1] ;
		/* ＸＺはコントロール位置で固定 */
		pos.vx = entk->ctrl->mov.vx ;
		pos.vz = entk->ctrl->mov.vz ;

		if ( !(entk->status2 & (ENE_STATUS2_AIM_FACE_X|ENE_STATUS2_AIM_GUN|ENE_STATUS2_AIM_RHAND)) ){
			/* 高低のアジャストなし */
			trg.vy = pos.vy ;
		}

if ( entk->g_id == DEB_G_ID && entk->u_id == DEB_U_ID && entk->id == DEB_ID){
printf("\n trg vy[%.2f] pos vy[%.2f]\n",trg.vy, pos.vy ) ;
}

		if ( entk->status2 & ENE_STATUS2_AIM_TURN ){
			speed = 10 ;
		}

		if ( entk->status2 & ENE_STATUS2_AIM_NO_LIMIT ) {
			rlimit = &no_rlim ;
		} else if ( entk->status2 & ENE_STATUS2_AIM_S_LIMIT ) {
			rlimit = &s_rlim ;
		} else {
			rlimit = &rlim ;
		}

		if ( entk->status2 & ENE_STATUS2_AIM_DIR_FACE ){
			/* 方向指定 nearexp */
			GM_DirToNearRotAimLimit( &pos,entk->ctrl->rot.vy,act->before_dir,act->aim_dir,&act->adj_rot,
					interp, speed, rlimit ) ;
if ( entk->g_id == DEB_G_ID && entk->u_id == DEB_U_ID && entk->id == DEB_ID){
printf("rotvy=%d before=%d aim_dir=%d\n",entk->ctrl->rot.vy, act->before_dir, act->aim_dir ) ;
}
		} else {
			/* 座標指定 act->adj_rotに目標値が前のアジャスト値から補完された値が入る */
			GM_TrgToNearRotAimLimit( &pos, entk->ctrl->rot.vy, act->before_dir, &trg, 
			&act->adj_rot, interp, speed, rlimit ) ;
if ( entk->g_id == DEB_G_ID && entk->u_id == DEB_U_ID && entk->id == DEB_ID){
printf("adj vx=%d vy=%d \n",act->adj_rot.vx, act->adj_rot.vy ) ;
}
		}

		flag = ADJ_PARTS_HEAD ;
		if ( entk->status2 & ENE_STATUS2_AIM_GUN ){
			flag |= ADJ_PARTS_WEST|ADJ_PARTS_RARM|ADJ_PARTS_LARM ;
		} else if ( entk->status2 & ENE_STATUS2_AIM_RHAND ){
			flag |= ADJ_PARTS_RARM ;
			GM_AdjustRotBody( act->body, &act->adj_rot, HUMAN21_MIGI_KATA ) ;
			return ;
		} else if ( entk->status2 & ENE_STATUS2_AIM_FACE_ONLY ){
			flag |= ADJ_PARTS_WEST_HALF ;
		} else {
			flag |= ADJ_PARTS_WEST_HALF|ADJ_PARTS_RARM|ADJ_PARTS_LARM ;
		}
		if ( act->bodyp.type & ENE_TYPE_HITECH ) {
			GM_MotionAdjustWest( act->body, &act->adj_rot, entk->ctrl->rot.vy, 15 ) ;
		} else {
			GM_MotionAdjustAim( act->body, &act->adj_rot, entk->ctrl->rot.vy, flag , interp) ;
		}
//		GM_MotionAdjustWest( act->body, &act->adj_rot, entk->ctrl->rot.vy, 15 ) ;

		entk->act->adj_flag_old = flag ;
//printf("flag[%x] status2[%x] we[%d]\n",flag, entk->status2, act->adj_rot.vy ) ;

if ( entk->g_id == DEB_G_ID && entk->u_id == DEB_U_ID && entk->id == DEB_ID){
int di ;
di = _FVecTrgDir2( &entk->ctrl->mov, &trg ) ;
printf(" status2[%x] entk->ctrl->rot.vy[%d] trg y[%d]\n",entk->status2, entk->ctrl->rot.vy, di ) ;
printf("adj y[%d] x[%d] we[%d]\n", act->adj_rot.vy,act->adj_rot.vx, act->adj_rot.pad ) ;
}
if ( entk->g_id == DEB_G_ID && entk->u_id == DEB_U_ID && entk->id == DEB_ID){
int d ;
d = MatToYRot( &BODYWORLD( act->body, HUMAN21_KUBI ) ) ;
printf(" face dir=%d\n",d ) ;

printf(" motion num = %d\n",act->body->m_ctrl->mt3_ctrl->time ) ;
}
	} else {
		if ( entk->act->adj_rot.vx != 0 || entk->act->adj_rot.vy != 0 ) {
			/* 補完しながらもとに戻す */
			GV_NearExp4PV( &entk->act->adj_rot, &DG_ZeroSVector, 2 ) ;

			flag = entk->act->adj_flag_old ;
			//flag = ADJ_PARTS_HEAD|ADJ_PARTS_WEST_HALF|ADJ_PARTS_RARM|ADJ_PARTS_LARM ;
			GM_MotionAdjustAim( act->body, &act->adj_rot, entk->ctrl->rot.vy, flag, interp ) ;
		} else {
			entk->act->adj_flag_old = 0;
		}
if ( entk->g_id == DEB_G_ID && entk->u_id == DEB_U_ID && entk->id == DEB_ID){
printf("x[%d]y[%d]p[%d]\n",entk->act->adj_rot.vx,entk->act->adj_rot.vy, act->adj_rot.pad ) ;
}
	}
}

void ENE_FamasShoot( world )
FMATRIX	*world ;
{
}

static	FVECTOR	ShootPointAks[] = { { 20.0F, -712.0F, 90.0F } } ;

void ENE_ShootBullet( act, flag, pos )
ACTION	*act ;
int		flag ;
FVECTOR	*pos ;
{
	extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
	extern void AN_CartridgeMKR_E( FMATRIX *, OBJECT *, CONTROL * ) ;
	extern void AN_MazzleMKR( FMATRIX *, int, int ) ;
	extern void AN_CartridgeAKS_E( FMATRIX *, OBJECT *, CONTROL * ) ;
	extern void AN_MazzleAKS( FMATRIX *, int, int ) ;
	extern void AN_MazzleP90( FMATRIX *, int, int ) ;
	extern void MazzleM4A1_HI( FMATRIX *,int, int ) ;
	extern void	AN_MazzleABK(FMATRIX *world , int silence , int mode ) ;
	extern void AN_MazzleSPS(FMATRIX *world,int silence,int mode) ;

	extern void *NewSPS_MazzleFlush( FMATRIX * ) ;
	extern void *NewCartridgeP90ALL(int , int);
	extern void CartridgeP90( FMATRIX *, OBJECT *, CONTROL * ) ;
	extern void CartridgeM4A1_HI( FMATRIX * , OBJECT * , CONTROL * ) ;
	extern void CartridgeABK( FMATRIX * , OBJECT * , CONTROL * ) ;

	FMATRIX	w, *hand ;
    FVECTOR	from, vec ;
    SVECTOR	rot ;
    int		tmplight, type ;
    COMMANDER	*com ;

	if ( 0){
		int r ;

		r=KR_RandU( 12 ) ;
		if( r < 3 ) {
			r=KR_RandU( 32 ) ;
			GM_SeSetMode( SD_V_C01MAKI+r, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
		}
	}

#ifdef DEBUG_MODE
{
	com = COM_GetCommander() ;
	if ( com->status & CMST_ENEMY_GUN_CUT ) {
		return ;
	}
}
#endif

	/* 銃を持つ手のマトリックス */
	if ( flag & ENE_BULLET_LEFT ) {
		hand = &BODYWORLD( act->body, HUMAN21_HIDARI_TE ) ;
	} else {
		hand = &BODYWORLD( act->body, HUMAN21_MIGI_TE ) ;
	}
	
	w = *hand ;
    DG_SetPos( hand ) ;
	/* 銃口までシフト */
	DG_PutVector( &ShootPointAks[0], &from, 1 ) ;

	if ( pos != NULL ) {
		_sceVu0SubVector(  &vec, &from, pos ) ;
		_FVecToRotXY( &vec, &rot ) ;
		DG_SetPos2( &from, &rot ) ;
		DG_GetPos( &w ) ;
#if 0
	if ( GV_DiffDirAbs( rot.vx, MatToXRot( hand ) ) > 512 ) {
		/* 手の角度と発射方向の角度が違いすぎたら軌跡を消す */
		flag |= ENE_BULLET_NOLINE ;
	}
#endif
	} else {
		KR_FvecToMat( &from, &w ) ;
	}

//printf("bullet = %d \n",*act->sw->bullet ) ;

	type = 0 ;
	com = COM_GetCommander() ;
	if ( !(com->status & CMST_ENEMY_GUN_LINE) ) {
		/* 軌跡 */
		if ( !(flag & ENE_BULLET_NOLINE) ) {
//			if ( *act->sw->bullet >= MAX_MAGAZINE_MG-3 ) {
			if ( *act->sw->bullet >= (ENE_GetAmmoMax( act->name_id->weapon )-3) ) {
				if( act->bodyp.type & ENE_TYPE_HITECH){
					type = BUL_TYPE_VISIBLE|BUL_TYPE_RED ;
				}else {
					type = BUL_TYPE_VISIBLE|BUL_TYPE_GREEN ;
				}
			} else {
				type = BUL_TYPE_VISIBLE ;
			}
		}

		/* 跳弾 */
		type |= ( flag & ENE_BULLET_NOSPARK ) ? 0 : BUL_TYPE_SPARK ;
		
		/* 壁突き抜け */
		type |= ( flag & ENE_BULLET_NOWALL ) ? BUL_TYPE_NO_HZD : 0 ;
		
		/* 攻撃判定無し */
		type |= ( flag & ENE_BULLET_NOATTACK ) ? BUL_TYPE_NO_ATTACK : 0 ;
	}

	/* 敵兵の使用する武器には必ず立てる */
	type |= BUL_TYPE_NO_PLAYER ;

	if ( !(flag & ENE_BULLET_NORANDAM) ) {	/* 角度にランダム入れる */
		DG_SetPos( &w ) ;
//		rot.vx = KR_RandS( 64 )  ;
//		rot.vy = KR_RandS( 64 )  ;

		rot.vx = KR_RandS( 128 )  ;
		rot.vy = KR_RandS( 128 )  ;
		rot.vz = 0 ;
		DG_RotatePos( &rot ) ;
		DG_GetPos( &w ) ;
	}
#ifndef NO_SHOT_ENE___
	/*SPS 実験*/
	if
//	( act->name_id->weapon == E_WP_SPS)
	( act->bodyp.type & ENE_TYPE_SHOTGUN)
	{
		NewBullet( &w, type, PLAYER_SIDE, 25, SPS_DAMAGE,20000,1000, WP_ShotGun_Near) ;
	}else if( act->bodyp.type & ENE_TYPE_EVENT_32){
		/*狙撃イベントは射程伸ばす*/
		NewBullet( &w, type, PLAYER_SIDE, 25, AK_DAMAGE, 30000, 1000, WP_Aks ) ;
	}else {
		NewBullet( &w, type, PLAYER_SIDE, 25, AK_DAMAGE, 20000, 1000, WP_Aks ) ;
	}
#endif

	tmplight = 0 ;
#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		COMMANDER	*com ;
		com = COM_GetCommander() ;
		tmplight = com->status & (CMST_ENEMY_TMP_LIGHT1|CMST_ENEMY_TMP_LIGHT2) ;
//		tmplight = tmplight >> 28 ;
		tmplight = tmplight >> 25 ;
//printf("tmplight [%x]\n",tmplight ) ;
	}
#endif

	if ( act->sw->sub_weapon ) {
		switch ( act->name_id->sub_weapon ) {
			case E_WP_MKR :
				AN_CartridgeMKR_E( hand, act->body, act->ctrl ) ;
				AN_MazzleMKR( hand, 0, tmplight ) ;
				act->sw->wctrl.flag = WPEF_FLG_START ;
				GM_SeSetMode( SD_E_MACALL01 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				break ;
		}
	} else {
		switch ( act->name_id->weapon ) {
			case E_WP_AKS :
				AN_CartridgeAKS_E( hand, act->body, act->ctrl ) ;
				AN_MazzleAKS( hand, 0, tmplight ) ;
				GM_SeSetMode( SD_E_FAMAS03 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				break ;
			case E_WP_AKS_SP :
				AN_CartridgeAKS_E( hand, act->body, act->ctrl ) ;
				AN_MazzleAKS( hand, 1, tmplight ) ;
//NewConnectSmoke( &hand, &shift_center, 120, 16.0f );
				GM_SeSetMode( SD_E_FMSSIR01 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				break ;
			case E_WP_SPS :	/* ショットガン */
//void AN_MazzleSPS(FMATRIX *world,int silence,int mode)
				AN_MazzleSPS(hand, 0,tmplight) ;
//				NewSPS_MazzleFlush( hand ) ;
				GM_SeSetMode( SD_E_SHTFRE01 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				break ;
			case E_WP_P90 :	/* P90 */
				AN_MazzleP90( hand , 1 , tmplight );
				CartridgeP90( hand , act->body , act->ctrl );
				GM_SeSetMode( SD_E_TNGPFIRE , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				break ;
			case E_WP_ABAKAN :	/* abakan */
				AN_MazzleABK( hand , 1 , tmplight );
				CartridgeABK( hand , act->body , act->ctrl );
				GM_SeSetMode( SD_E_ABAKAN01 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				act->sw->wctrl2.flag = WPEF_FLG_START ;
				break ;
			case E_WP_M4_NM :	/* M4 */
				MazzleM4A1_HI( hand , 1 , tmplight );
				CartridgeM4A1_HI( hand , act->body , act->ctrl );
				GM_SeSetMode( SD_E_M4ENEM01 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
				act->sw->wctrl2.flag = WPEF_FLG_START ;
				break ;
		}
	}
}

void ENE_Shoot( world, act )
FMATRIX	*world ;
ACTION	*act ;
{
	extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
	extern void AN_CartridgeMKR_E( FMATRIX *, OBJECT *, CONTROL * ) ;
	extern void AN_MazzleMKR( FMATRIX *, int, int ) ;
	extern void AN_CartridgeAKS_E( FMATRIX *, OBJECT *, CONTROL * ) ;
	extern void AN_MazzleAKS( FMATRIX *, int, int ) ;
	FMATRIX	w ;
    FVECTOR	from ;
    int		tmplight, type ;
    COMMANDER	*com ;

#ifdef DEBUG_MODE
{
	com = COM_GetCommander() ;
	if ( com->status & CMST_ENEMY_GUN_CUT ) {
		return ;
	}
}
#endif



	w = *world ;
    DG_SetPos( world ) ;
	DG_PutVector( &ShootPointAks[0], &from, 1 ) ;
	w.m[ 3 ][ 0 ] = from.vx ;
	w.m[ 3 ][ 1 ] = from.vy ;
	w.m[ 3 ][ 2 ] = from.vz ;

//printf("bullet = %d \n",*act->sw->bullet ) ;

	com = COM_GetCommander() ;
	type = 0 ;
	if ( com->status & CMST_ENEMY_GUN_LINE ) {
		type = 0 ;
	} else {
		if ( *act->sw->bullet >= MAX_MAGAZINE_MG-3 ) {
			type = BUL_TYPE_VISIBLE|BUL_TYPE_GREEN|BUL_TYPE_SPARK ;
		} else {
			type = BUL_TYPE_VISIBLE|BUL_TYPE_SPARK ;
		}
	}
	NewBullet( &w, type, PLAYER_SIDE, 25, 5, 20000, 1000, WP_Famas ) ;

	tmplight = 0 ;
#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		COMMANDER	*com ;
		com = COM_GetCommander() ;
		tmplight = com->status & (CMST_ENEMY_TMP_LIGHT1|CMST_ENEMY_TMP_LIGHT2) ;
		tmplight = tmplight >> 28 ;
	}
#endif

	if ( act->sw->sub_weapon ) {
		switch ( act->name_id->sub_weapon ) {
			case E_WP_MKR :
				AN_CartridgeMKR_E( world, act->body, act->ctrl ) ;
				AN_MazzleMKR( world, 0, tmplight ) ;
				act->sw->wctrl.flag = WPEF_FLG_START ;
				break ;
		}
	} else {
		switch ( act->name_id->weapon ) {
			case E_WP_AKS :
				AN_CartridgeAKS_E( world, act->body, act->ctrl ) ;
				AN_MazzleAKS( world, 0, tmplight ) ;
				break ;
			case E_WP_AKS_SP :
				AN_CartridgeAKS_E( world, act->body, act->ctrl ) ;
				AN_MazzleAKS( world, 1, tmplight ) ;
				break ;
		}
	}
	GM_SeSetMode( SD_E_FAMAS03 , &act->ctrl->mov, GM_SEMODE_BOMB ) ;
}


void ENE_SetNeedl( body, n_obj, pos )
OBJECT	*body ;
int		n_obj ;
FVECTOR	*pos ;
{
	extern int MakeAttachment4_called(int ,FVECTOR *, SVECTOR *, OBJECT *, int, FVECTOR *, int, int );
	SVECTOR	rot ;
	FVECTOR	vans, nans ;
	int model_name ;

printf("needl n_obj[%d]\n",n_obj ) ;

	/* 麻酔弾 */
	model_name = GV_StrCode("m92_bul2") ;

	VertexSearch( &vans, &nans, body->objs, n_obj, pos );
	_FVecToRotXY( &nans, &rot ) ;
	rot.vx -= 1024 ; /* モデル固有補正 */
if(0){
extern void *NewBlood_M9( FMATRIX *world, FVECTOR *mov, FVECTOR *norm ) ;
	NewBlood_M9( &BODYWORLD( body, n_obj), &vans, &nans ) ;
}

	MakeAttachment4_called( model_name, NULL, &rot, body, n_obj, &vans, 512, 4 );
}

void ENE_SetNeedlV( body, n_obj, pos )
OBJECT	*body ;
int		n_obj ;
FVECTOR	*pos ;
{
	extern int MakeAttachment4V_called(int ,FVECTOR *, SVECTOR *, OBJECT *, int, FVECTOR *, int, int );
	SVECTOR	rot ;
	FVECTOR	vans, nans ;
	int model_name ;

printf("needl n_obj[%d]\n",n_obj ) ;

	/* 麻酔弾 */
	model_name = GV_StrCode("m92_bul2") ;

	VertexSearch( &vans, &nans, body->objs, n_obj, pos );
	_FVecToRotXY( &nans, &rot ) ;
	rot.vx -= 1024 ; /* モデル固有補正 */
if(0){
extern void *NewBlood_M9( FMATRIX *world, FVECTOR *mov, FVECTOR *norm ) ;
	NewBlood_M9( &BODYWORLD( body, n_obj), &vans, &nans ) ;
}

	MakeAttachment4V_called( model_name, NULL, &rot, body, n_obj, &vans, 512, 4 );
}


void ENE_ClearNeedlV( OBJECT *body )
{
	extern void SearchAndFallAttachment_called2(int ,OBJECT *,int,int ,int );
	SearchAndFallAttachment_called2(4,body,0,COUNT_VMODE(180), 16 ) ;
}

void ENE_ClearNeedl( act )
ACTION	*act ;
{
//	extern void SearchAndFallAttachment_called(OBJECT *,int func_type,int delay_count,int rnd_delay_count) ;
extern void SearchAndFallAttachment_called2(int ,OBJECT *,int,int ,int );

	BODYPARAM	*bodyp ;
	
	bodyp = &act->bodyp ;

//	SearchAndFallAttachment_called( act->body, 0, COUNT_VMODE(180), 16 ) ;
	SearchAndFallAttachment_called2(4,act->body,0,COUNT_VMODE(180), 16 ) ;

	bodyp->ane_level_num[0] = 0 ;
	bodyp->ane_level_num[1] = 0 ;
	bodyp->ane_level_num[2] = 0 ;
	bodyp->ane_level_num[3] = 0 ;

	bodyp->anesthesia = bodyp->m_anesthesia ;	/* 血液量と同じ */
}

void ENE_ClearPDamage( act )
ACTION	*act ;
{
	BODYPARAM	*bodyp ;
	
	bodyp = &act->bodyp ;

	act->bodyp.pbreak = 0 ;
	bodyp->dam_level_num[0] = 0 ;
	bodyp->dam_level_num[1] = 0 ;
	bodyp->dam_level_num[2] = 0 ;
	bodyp->dam_level_num[3] = 0 ;
}
/*sigeno add 2001.07.06*/
/*特定のフラグを残してクリア*/
void ENE_ClearPDamageMask( act ,mask )
ACTION	*act ;
int		mask ;
{
	BODYPARAM	*bodyp ;
	
	bodyp = &act->bodyp ;

	act->bodyp.pbreak &= mask ;
	bodyp->dam_level_num[0] = 0 ;
	bodyp->dam_level_num[1] = 0 ;
	bodyp->dam_level_num[2] = 0 ;
	bodyp->dam_level_num[3] = 0 ;
}


#define DARK_MIN_DIS	(2000)
void ENE_EyeInfoCheck( entk, eyei )
ENETHINK	*entk;
EYEINFO		*eyei ;
{
	FVECTOR	vec, head, *pos ;
	int		blurr_dis, sight_dis, sight_range, x_rot, diff ;
	SENSEPARAM *sens ;

	if ( COM_StageKind() & ENE_STAGE_NO_NOTICE ) {	/* 反応無し */
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	if ( entk->act->status & ACT_STATUS_UNREAL ) {
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}
	if ( eyei->flag & EYE_INFO_FLAG_SKIP ) goto end ;

	sens = &entk->sense ;
	pos = &entk->ctrl->mov ;

	_sceVu0SubVector(  &vec, eyei->pos, pos ) ;
	eyei->dis = _FVecLen3( &vec ) ;
	eyei->dir = _FVecDir2( &vec ) ;

	if ( entk->act->status & ACT_STATUS_EYE_CLOSE ) {
		if ( entk->act->status & (ACT_STATUS_FAINT|ACT_STATUS_DEATH) ) {
			UNSET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	if ( eyei->pos == &GM_PlayerFindPos ) {	/* プレイヤーの情報は */
		if ( ENE_CheckPlayerHidden() ) {
			if ( !(entk->iknow_flag & IKNOW_HIDDEN_PLAYER) ) {
				if ( eyei->sight == EYE_INFO_SIGHT_IN ) {
					SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
				}
			}
			if ( GM_PlayerStatus & (PLAYER_INTRUDE) ) {
				if ( !(entk->act->status & ACT_STATUS_GUNLIGHT_D) ) {
					eyei->sight = EYE_INFO_SIGHT_OUT ;
					goto end ;
				}
			} else {
				if ( !(entk->act->status & ACT_STATUS_GUNLIGHT_LR) ) {
					eyei->sight = EYE_INFO_SIGHT_OUT ;
					goto end ;
				}
#if 0	//いらないかも
			} else if ( GM_PlayerStatus & (PLAYER_LOCKER) ) {
				if ( !(entk->act->status_status & ACT_STST_PEEP_TOILET) ) {
				}
#endif
			}
			UNSET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}

	if ( (eyei->pos == &GM_PlayerFindPos) 		/* プレイヤーの情報は */
		&& (GM_PlayerStatus & PLAYER_DARK_AREA) /* 暗闇 */
		&& (entk->sw.n_sight != 2) ) {			/* 暗視ゴーグルを付けていない */
//		blurr_dis = sens->eye_s/2 ;
		blurr_dis = sens->eye_s/4 ;				/* 朧無しにしないと無限？になる*/
		if ( blurr_dis < DARK_MIN_DIS ) blurr_dis = DARK_MIN_DIS ;
		sight_dis = sens->eye_s/4 ;
		if ( sight_dis < DARK_MIN_DIS ) sight_dis = DARK_MIN_DIS ;
		if( GM_AlertMode == ALERT_MODE_ALERT ) {
			sight_range = 2000 ;
		} else {
			sight_range = sens->eye_r ;
		}
	} else {
		if ( entk->status2 & ENE_STATUS_GLASSES ) {	/* 双眼鏡を装着していたら */
			blurr_dis = sens->eye_s + sens->glasses_dis ;
			sight_dis = sens->eye_s + sens->glasses_dis ;
			sight_range = DEF_GLASSES_RANGE ;
//printf("glass[%d][%d][%d]!!\n",blurr_dis,sight_dis,sight_range) ;
		} else {
			blurr_dis = sens->eye_s + (sens->eye_s/2) ;
			sight_dis = sens->eye_s ;
			sight_range = sens->eye_r ;
			if (eyei->pos == &GM_PlayerFindPos) { 		/* プレイヤーの情報は */
				if ((PL_GetPlayerItem()==IT_Uniform) && !(GM_PlayerStateFlag & PL_GBSCAP_EXIST) ) {
					blurr_dis = blurr_dis*3/4 ;
					sight_dis = sight_dis*3/4 ;
				}
			}
		}
	}

	if ( (eyei->pos == &GM_PlayerFindPos) &&  	/* プレイヤーの情報は */
		 ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_DEEP_STEALTH ) ) ) {
		blurr_dis = DARK_MIN_DIS ;
		sight_dis = DARK_MIN_DIS ;
	}

	//おぼろ視界無し
	if ( entk->act->bodyp.type & ENE_TYPE_NO_BLURR){
		blurr_dis = sight_dis ;
	}
	if ( eyei->pos == &GM_PlayerFindPos ) {	/* プレイヤーの情報は */
		/* エルード中は朧視界無し */
		if ( GM_PlayerStatus & (PLAYER_BEYOND) ) {
			blurr_dis = sight_dis ;
		}
	}

	eyei->sight_dis = sight_dis ;
	eyei->blurr_dis = blurr_dis ;

	/* 視力判定１（かすかに見える距離＝視力×１．５） */
	if ( eyei->dis > blurr_dis ) {
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	/* 近距離判定 */
#if 1
	if ( ENE_InRangeXYZ( &entk->ctrl->mov, eyei->pos, EYE_NEARLENGTH/2, 1500, EYE_NEARLENGTH/2 ) ) {
#else
	if ( eyei->dis < EYE_NEARLENGTH ) {
#endif
		if ( ENE_EyeOnlineCheck( entk->ctrl->hzx_id, eyei->pos, pos ) ){
			eyei->sight = EYE_INFO_SIGHT_OUT_HZD ;
		} else {
			eyei->sight = EYE_INFO_SIGHT_IN ;
			entk->last_pos = *eyei->pos ;	/* 最後に見えた場所をセット */
			entk->last_map = *eyei->map ;	/* 最後に見えたマップをセット */
		}
		goto end ;
	}

#ifdef HEAD_TO_TARGET
	/* 視界判定は頭の位置から */
	KR_FMatToFvec( &BODYWORLD(entk->act->body, HUMAN21_ATAMA), &head ) ;
	pos = &head ;
if ( entk->act->status & ACT_STATUS_GUNLIGHT_D ) {
	head.vy -= 100.0f ;
}

	_sceVu0SubVector(  &vec, eyei->pos, pos ) ;
#endif

	/* 視野判定 */
	if ( _DiffDirAbs( sens->facedir, eyei->dir ) > sight_range ) {
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	/* 高低差判定 */
	x_rot = _FVecDirX( &vec ) - 1024 ;
	diff = GV_DiffDirS( sens->facedir_x, x_rot ) ;
if (0&&  (entk->g_id==0) && (entk->id==2) ) {
printf("y:eye[%f] pos[%f] vec[%f] \n",pos->vy,eyei->pos->vy, vec.vy );

printf("face[%d] x_rot[%d] diff[%d]\n",sens->facedir_x,x_rot,diff);
}
	if ( diff > UNDER_EYE_SIGHT || diff < UPPER_EYE_SIGHT ) {
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	/* 視力判定２ */
	if ( ENE_EyeOnlineCheck( entk->ctrl->hzx_id, eyei->pos, pos ) ){
		eyei->sight = EYE_INFO_SIGHT_OUT_HZD ;
	} else {
		if ( eyei->dis > sight_dis ) {
			if ( entk->act->bodyp.type & ENE_TYPE_NO_BLURR){
				eyei->sight = EYE_INFO_SIGHT_OUT ;
			}else {
				eyei->sight = EYE_INFO_SIGHT_BLURR ;
			}
		} else {
			eyei->sight = EYE_INFO_SIGHT_IN ;
		}
		entk->last_pos = *eyei->pos ;	/* 最後に見えた場所をセット */
		entk->last_map = *eyei->map ;	/* 最後に見えたマップをセット */
	}
#ifdef DEBUG_MODE
if ( entk->com->status & CMST_ENEMY_SIGHT_VIEW ) {
	extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
	FVECTOR	lin[2] ;
	lin[0] = *pos ;
	lin[1] = *eyei->pos ;
	if ( eyei->sight == EYE_INFO_SIGHT_IN ) {
		NewLineView(  &lin[0],1,255,0,0) ;
	} else {
		NewLineView(  &lin[0],1,0,0,255) ;
	}

//	if ( eyei->sight == EYE_INFO_SIGHT_IN ) {
//		printf("status[%lx] ene_st [%lx] iknow_flag[%x] sight[%d]\n",
//		GM_PlayerStatus, entk->act->status,entk->iknow_flag,eyei->sight ) ;
//	}

}
#endif

end :

	/* 段ボールなどの補正前の結果 */
	eyei->sight_real = eyei->sight ;

	if ( eyei->pos == &GM_PlayerFindPos ) {	/* プレイヤーの情報は */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF ) {
			eyei->sight = EYE_INFO_SIGHT_OUT ;
		}
		/* ステルスチェック */
		if ( !(GM_CheckPlayerStatusEX( I64(0), PLAYER2_DEEP_STEALTH )) ) {
			if ( GM_PlayerStatus & (PLAYER_STEALTH) ) {
				eyei->sight = EYE_INFO_SIGHT_OUT ;
			}
		}
	}
}

void ENE_EyeInfoCheckWhzxid( entk, eyei,w_hzx_id )
ENETHINK	*entk;
EYEINFO		*eyei ;
int 		w_hzx_id ;
{
	FVECTOR	vec, head, *pos ;
	int		blurr_dis, sight_dis, sight_range, x_rot, diff ;
	SENSEPARAM *sens ;

	if ( entk->act->status & ACT_STATUS_UNREAL ) {
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}
	if ( eyei->flag & EYE_INFO_FLAG_SKIP ) goto end ;

	sens = &entk->sense ;
	pos = &entk->ctrl->mov ;

	_sceVu0SubVector(  &vec, eyei->pos, pos ) ;
	eyei->dis = _FVecLen3( &vec ) ;
	eyei->dir = _FVecDir2( &vec ) ;

	if ( entk->act->status & ACT_STATUS_EYE_CLOSE ) {
		if ( entk->act->status & (ACT_STATUS_FAINT|ACT_STATUS_DEATH) ) {
			UNSET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	if ( eyei->pos == &GM_PlayerFindPos ) {	/* プレイヤーの情報は */
		if ( ENE_CheckPlayerHidden() ) {
			if ( !(entk->iknow_flag & IKNOW_HIDDEN_PLAYER) ) {
				if ( eyei->sight == EYE_INFO_SIGHT_IN ) {
					SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
				}
			}
			if ( GM_PlayerStatus & (PLAYER_INTRUDE) ) {
				if ( !(entk->act->status & ACT_STATUS_GUNLIGHT_D) ) {
					eyei->sight = EYE_INFO_SIGHT_OUT ;
					goto end ;
				}
			} else {
				if ( !(entk->act->status & ACT_STATUS_GUNLIGHT_LR) ) {
					eyei->sight = EYE_INFO_SIGHT_OUT ;
					goto end ;
				}
#if 0	//いらないかも
			} else if ( GM_PlayerStatus & (PLAYER_LOCKER) ) {
				if ( !(entk->act->status_status & ACT_STST_PEEP_TOILET) ) {
				}
#endif
			}
			UNSET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}else {
			UNSET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER_CLEARING_INTRUDE ) ;
		}
	}

	if ( (eyei->pos == &GM_PlayerFindPos) 		/* プレイヤーの情報は */
		&& (GM_PlayerStatus & PLAYER_DARK_AREA) /* 暗闇 */
		&& (entk->sw.n_sight != 2) ) {			/* 暗視ゴーグルを付けていない */
//		blurr_dis = sens->eye_s/2 ;
		blurr_dis = sens->eye_s/4 ;				/* 朧無しにしないと無限？になる*/
		if ( blurr_dis < DARK_MIN_DIS ) blurr_dis = DARK_MIN_DIS ;
		sight_dis = sens->eye_s/4 ;
		if ( sight_dis < DARK_MIN_DIS ) sight_dis = DARK_MIN_DIS ;
		if( GM_AlertMode == ALERT_MODE_ALERT ) {
			sight_range = 2000 ;
		} else {
			sight_range = sens->eye_r ;
		}
	} else {
		if ( entk->status2 & ENE_STATUS_GLASSES ) {	/* 双眼鏡を装着していたら */
			blurr_dis = sens->eye_s + sens->glasses_dis ;
			sight_dis = sens->eye_s + sens->glasses_dis ;
			sight_range = DEF_GLASSES_RANGE ;
//printf("glass[%d][%d][%d]!!\n",blurr_dis,sight_dis,sight_range) ;
		} else {
			blurr_dis = sens->eye_s + (sens->eye_s/2) ;
			sight_dis = sens->eye_s ;
			sight_range = sens->eye_r ;
			if (eyei->pos == &GM_PlayerFindPos) { 		/* プレイヤーの情報は */
				if ((PL_GetPlayerItem()==IT_Uniform) && !(GM_PlayerStateFlag & PL_GBSCAP_EXIST) ) {
					blurr_dis = blurr_dis*3/4 ;
					sight_dis = sight_dis*3/4 ;
				}
			}
		}
	}

	if ( (eyei->pos == &GM_PlayerFindPos) &&  	/* プレイヤーの情報は */
		 ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_DEEP_STEALTH ) ) ) {
		blurr_dis = DARK_MIN_DIS ;
		sight_dis = DARK_MIN_DIS ;
	}

	//おぼろ視界無し
	if ( entk->act->bodyp.type & ENE_TYPE_NO_BLURR){
		blurr_dis = sight_dis ;
	}
	if ( eyei->pos == &GM_PlayerFindPos ) {	/* プレイヤーの情報は */
		/* エルード中は朧視界無し */
		if ( GM_PlayerStatus & (PLAYER_BEYOND) ) {
			blurr_dis = sight_dis ;
		}
	}

	eyei->sight_dis = sight_dis ;
	eyei->blurr_dis = blurr_dis ;

	/* 視力判定１（かすかに見える距離＝視力×１．５） */
	if ( eyei->dis > blurr_dis ) {
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	/* 近距離判定 */
	if ( ENE_InRangeXYZ(&entk->ctrl->mov, eyei->pos, EYE_NEARLENGTH/2, 1500, EYE_NEARLENGTH/2)){
		
		if ( ENE_EyeOnlineCheck( w_hzx_id, eyei->pos, pos ) ){
			eyei->sight = EYE_INFO_SIGHT_OUT_HZD ;
		} else {
			eyei->sight = EYE_INFO_SIGHT_IN ;
			entk->last_pos = *eyei->pos ;	/* 最後に見えた場所をセット */
			entk->last_map = *eyei->map ;	/* 最後に見えたマップをセット */
		}
		goto end ;
	}

#ifdef HEAD_TO_TARGET
	/* 視界判定は頭の位置から */
	KR_FMatToFvec( &BODYWORLD(entk->act->body, HUMAN21_ATAMA), &head ) ;
	pos = &head ;
if ( entk->act->status & ACT_STATUS_GUNLIGHT_D ) {
	head.vy -= 100.0f ;
}

	_sceVu0SubVector(  &vec, eyei->pos, pos ) ;
#endif

	/* 視野判定 */
	if ( _DiffDirAbs( sens->facedir, eyei->dir ) > sight_range ) {
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	/* 高低差判定 */
	x_rot = _FVecDirX( &vec ) - 1024 ;
	diff = GV_DiffDirS( sens->facedir_x, x_rot ) ;
if (0&&  (entk->g_id==0) && (entk->id==2) ) {
printf("y:eye[%f] pos[%f] vec[%f] \n",pos->vy,eyei->pos->vy, vec.vy );

printf("face[%d] x_rot[%d] diff[%d]\n",sens->facedir_x,x_rot,diff);
}
	if ( diff > UNDER_EYE_SIGHT || diff < UPPER_EYE_SIGHT ) {
		eyei->sight = EYE_INFO_SIGHT_OUT ;
		goto end ;
	}

	/* 視力判定２ */
	if ( ENE_EyeOnlineCheck( w_hzx_id, eyei->pos, pos ) ){
		eyei->sight = EYE_INFO_SIGHT_OUT_HZD ;
	} else {
		if ( eyei->dis > sight_dis ) {
			if ( entk->act->bodyp.type & ENE_TYPE_NO_BLURR){
				eyei->sight = EYE_INFO_SIGHT_OUT ;
			}else {
				eyei->sight = EYE_INFO_SIGHT_BLURR ;
			}
		} else {
			eyei->sight = EYE_INFO_SIGHT_IN ;
		}
		entk->last_pos = *eyei->pos ;	/* 最後に見えた場所をセット */
		entk->last_map = *eyei->map ;	/* 最後に見えたマップをセット */
	}
#ifdef DEBUG_MODE
if ( entk->com->status & CMST_ENEMY_SIGHT_VIEW ) {
	extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
	FVECTOR	lin[2] ;
	lin[0] = *pos ;
	lin[1] = *eyei->pos ;
	if ( eyei->sight == EYE_INFO_SIGHT_IN ) {
		NewLineView(  &lin[0],1,255,0,0) ;
	} else {
		NewLineView(  &lin[0],1,0,0,255) ;
	}

//	if ( eyei->sight == EYE_INFO_SIGHT_IN ) {
//		printf("status[%lx] ene_st [%lx] iknow_flag[%x] sight[%d]\n",
//		GM_PlayerStatus, entk->act->status,entk->iknow_flag,eyei->sight ) ;
//	}

}
#endif

end :

	/* 段ボールなどの補正前の結果 */
	eyei->sight_real = eyei->sight ;

	if ( eyei->pos == &GM_PlayerFindPos ) {	/* プレイヤーの情報は */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF ) {
			eyei->sight = EYE_INFO_SIGHT_OUT ;
		}
		/* ステルスチェック */
		if ( !(GM_CheckPlayerStatusEX( I64(0), PLAYER2_DEEP_STEALTH )) ) {
			if ( GM_PlayerStatus & (PLAYER_STEALTH) ) {
				eyei->sight = EYE_INFO_SIGHT_OUT ;
			}
		}
	}
}

void ENE_SetAlertLevel( entk )
ENETHINK	*entk ;
{
	int	sight ;
	HZX_ZON *pl_zone;

	pl_zone = ENE_HZX_GetZone( entk->com->plzone_in_zone[PL_POS_CENTER] );

	sight = entk->pl_eyei.sight ;

	switch( sight ) {
		case EYE_INFO_SIGHT_OUT :
		case EYE_INFO_SIGHT_OUT_HZD :
			if ( !(entk->iknow_flag & IKNOW_HIDDEN_PLAYER) ) {
				if ( CM_TMPSNAKEAREA >= 0 || (pl_zone->flag & HZX_ZONE_SAFE) ) {
					entk->alert -= (int)STEP_VMODE(16) ;
//					entk->alert -= 4 ;
				} else {
					entk->alert -= 4 ;
				}
			}
		break ;
		case EYE_INFO_SIGHT_BLURR :
#if 1	//膠着になるので危険モードのみentk->alertをへらす
			if ( GM_AlertMode == ALERT_MODE_ALERT ) {
					entk->alert -= 1 ;
			} else {
				if ( entk->alert < 1 ) {
					entk->alert ++ ;
				}
			}
#else
			if ( entk->alert < 1 ) {
				entk->alert ++ ;
			}
#endif
		break ;
		case EYE_INFO_SIGHT_IN :
//			entk->alert += ENE_INDISTINCT_ALERT_LEVEL ;	/* いきなり識別 */
			entk->alert ++ ;
		break ;
	}
	if ( entk->alert < 0 ) entk->alert = 0 ;
	else if ( entk->alert > MAX_ALERT_LEVEL ) entk->alert = MAX_ALERT_LEVEL;
}

int	ENE_RoutePad( znavi, trgp, ctrl )
ZONENAVI	*znavi ;
TRGPOINT	*trgp ;
CONTROL		*ctrl ;
{
	HZX_ZON		*zone ;
	FVECTOR		*target, tmp ;
	int		this, going, next, reach ;

	going = trgp->addr ;
	target = &trgp->pos ;

	this = HZX_GetAddress( ctrl->hzx_id, &( znavi->flore_pos ), znavi->this_addr ) ;
	znavi->this_addr = this ;
	reach = HZX_ReachTo( this, znavi->next_addr );
#if 0
printf("This=0x%4x, Next=0x%4x, Targ=0x%4x, Reach=%d \n",
					 this, znavi->next_addr, going,reach );
#endif
	if ( going != znavi->going_addr || reach <= HZX_REACH ) {
		znavi->going_addr = going ;
		if ( GM_GetBit( HZX_ZoneMapNo( trgp->addr ) ) == ctrl->hzx_id 
				&& HZX_InsideZone( ctrl->hzx_id, &( znavi->flore_pos ), trgp->addr & 255 ) ) {
			znavi->next_zonepos = *target ;
			znavi->next_addr = going ;
			_sceVu0SubVector(  &tmp, &( znavi->next_zonepos ), &( znavi->flore_pos ) ) ;
			trgp->dir = _FVecDir2( &tmp ) ;

			return -1 ;
		} else {
			next = HZX_Navigate( this, going, &( znavi->flore_pos ) ) ;
			zone = HZX_GetZoneNo( HZX_ZoneMapNo(next), HZX_Zone1(next) ) ;
			znavi->next_zonepos.vx = (float)zone->x ;
			znavi->next_zonepos.vy = (float)zone->y ;
			znavi->next_zonepos.vz = (float)zone->z ;
			znavi->next_addr = next ;
		}
	}
	_sceVu0SubVector(  &tmp, &( znavi->next_zonepos ), &( znavi->flore_pos ) ) ;
	trgp->dir = _FVecDir2( &tmp ) ;

	return trgp->dir ;
}


/*
	同マップ内で同じゾーン内かチェック
	*/
int ENE_SameZone( pos1, pos2, mapbit )
FVECTOR		*pos1, *pos2 ;
int			mapbit ;
{
	int ad1, ad2 ;

	ad1 = HZX_GetAddress( mapbit, pos1, -1 ) ;
	ad2 = HZX_GetAddress( mapbit, pos2, -1 ) ;

	return ( ad1 == ad2 ) ;
}

/*----- 補助関数 --------------------------------------------------*/
int	ENE_InZone( FVECTOR *pos, int addr )
{
	HZX_GROUP_ID group_id ;
	int	z ;

	z = HZX_Zone1( addr ) ;
	group_id = HZX_ZoneGroupID( addr ) ;
	
	return HZX_InsideZone( group_id, pos, z ) ;
}

int	ENE_ZoneIntrptCheck( HZX_ZONE_ADD this )
{
	HZX_ZON	*z ;
	if( HZX_Zone1( this ) == 255 ) return 1;
	z = HZX_GetZoneFromAdd( this ) ;
	if ( z->flag & (HZX_ZONE_ZINTRPT|HZX_ZONE_INTRUDE) ) return 1;

	return 0 ;
}

int	ENE_IntrptCheck( HZX_ZONE_ADD this, HZX_ZONE_ADD next )
{
	HZX_ZON	*z;
	u_char	*nears, near ;
	int		n_zone, map1, map2, i ;

	/* マップチェック */
	map1 = HZX_ZoneMapNo( this ) ;
	map2 = HZX_ZoneMapNo( next ) ;
	if ( map1 != map2 ) return 1 ;	/* マップが違う */

	z = HZX_GetZoneFromAdd( this ) ;
	n_zone = HZX_Zone1( next ) ;

	nears = z->nears ;
	for ( i=0; i<6; i++ ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		if ( near == n_zone ) {
			if ( z->near_flag[i] & HZX_ROOT_RINTRPT ) return 1 ;
			else break ;
		}
	}

	return 0 ;
}

int	ENE_ZoneTrace( znavi, trgp, ctrl, time )
ZONENAVI	*znavi ;
TRGPOINT	*trgp ;
CONTROL		*ctrl ;
int			time ;
{
	if(!(time % 0x20)){
		znavi->this_addr = -1 ;	/* 自分の場所初期化 */
		znavi->going_addr = -1 ;	/* 行き先初期化 */
		ENE_RoutePad( znavi, trgp, ctrl );		/* ぴくぴく防止 */
	}

	trgp->dir = ENE_RoutePad( znavi, trgp, ctrl ) ;
	if( trgp->dir < 0 ) return -1 ;

	return 0 ;
}

/* インタラプトチェック付き */
int	ENE_ZoneTraceIntrpt( znavi, trgp, ctrl, time )
ZONENAVI	*znavi ;
TRGPOINT	*trgp ;
CONTROL		*ctrl ;
int			time ;
{
	if(!(time % 0x20)){
		znavi->this_addr = -1 ;	/* 自分の場所初期化 */
		znavi->going_addr = -1 ;	/* 行き先初期化 */
		ENE_RoutePad( znavi, trgp, ctrl );		/* ぴくぴく防止 */
	}

	trgp->dir = ENE_RoutePad( znavi, trgp, ctrl ) ;
	if( trgp->dir < 0 ) return -1 ;
	if( ENE_ZoneIntrptCheck( znavi->next_addr ) ) return -2 ;
	if( ENE_IntrptCheck( znavi->this_addr, znavi->next_addr ) ) return 1 ;

	return 0 ;
}

int	ENE_DirectTrace( trgp, pos, range )
TRGPOINT	*trgp ;
FVECTOR		*pos ;
int			range ;
{
	FVECTOR		shift ;
	float		dx, dz ;

	shift.vx = dx = trgp->pos.vx - pos->vx ;
	shift.vz = dz = trgp->pos.vz - pos->vz ;
//printf("dx=%d dz=%d\n",(int)dx,(int)dz ) ;
	if ( (int)dx > -range && (int)dx < range && (int)dz > -range && (int)dz < range ) return -1 ;

	trgp->dir = _FVecDir2( &shift ) ;
	trgp->h_dis = (int)_FVecLen2( &shift ) ;

	return 0 ;
}

int	ENE_PinpointTrace( trgp, pos, range )
TRGPOINT	*trgp ;
FVECTOR		*pos ;
int			range ;
{
	FVECTOR		shift ;
	float		dx, dz ;

	shift.vx = dx = trgp->pos.vx - pos->vx ;
	shift.vz = dz = trgp->pos.vz - pos->vz ;
//printf("dx=%d dz=%d\n",(int)dx,(int)dz ) ;
	if ( (int)dx > -range && (int)dx < range && (int)dz > -range && (int)dz < range ) return -1 ;

	trgp->dir = _FVecDir2( &shift ) ;
	trgp->h_dis = (int)_FVecLen2( &shift ) ;

	return 0 ;
}

/* range範囲かチェック。Y方向は＋－1500以内 */
int	ENE_InRange( pos, trg, range )
FVECTOR		*pos ;
FVECTOR		*trg ;
int			range ;
{
	int		dx, dz, dy ;

	dx = (int)(trg->vx - pos->vx) ;
	dy = (int)(trg->vy - pos->vy) ;
	dz = (int)(trg->vz - pos->vz) ;

	if ( dx < -range || dx > range || dz < -range || dz > range || dy < -1500 || dy > 1500 ) return 0 ;

	return 1 ;
}

int	ENE_InRangeXYZ( pos, trg, range_x, range_y, range_z )
FVECTOR		*pos ;
FVECTOR		*trg ;
int			range_x, range_y, range_z ;
{
	int		dx, dz, dy ;

	dx = (int)(trg->vx - pos->vx) ;
	dy = (int)(trg->vy - pos->vy) ;
	dz = (int)(trg->vz - pos->vz) ;

	if ( dx < -range_x || dx > range_x || dz < -range_z || dz > range_z ||
		 dy < -range_y || dy > range_y ) return 0 ;

	return 1 ;
}

int	ENE_InRangeOnline( pos, trg, range, hzx_id  )
FVECTOR		*pos ;
FVECTOR		*trg ;
int			range ;
int			hzx_id ;
{
	int		dx, dz, dy ;

	dx = (int)(trg->vx - pos->vx) ;
	dy = (int)(trg->vy - pos->vy) ;
	dz = (int)(trg->vz - pos->vz) ;

	if ( dx < -range || dx > range || dz < -range || dz > range || dy < -1500 || dy > 1500 ) return 0 ;

	if ( ENE_EyeOnlineCheck( hzx_id, trg, pos ) ) return 0 ;

	return 1 ;
}

int	ENE_InRangeToRange( FVECTOR *pos, FVECTOR *trg, int in_range, int out_range )
{
	int		dx, dz, dy ;

	dx = (int)DG_FABS(trg->vx - pos->vx) ;
	if ( dx < in_range || dx > out_range ) return 0 ;
	dy = (int)DG_FABS(trg->vy - pos->vy) ;
	if ( dy > 1500 ) return 0 ;
	dz = (int)DG_FABS(trg->vz - pos->vz) ;
	if ( dz < in_range || dz > out_range ) return 0 ;

	return 1 ;
}

/*----- ターゲットセット --------------------------------------------------*/
int	ENE_SearchNearRootPoint( rnavi, pos )
ROUTENAVI	*rnavi ;
FVECTOR		*pos ;
{
	int	i, min, len, p ;
	FVECTOR	tmp ;
	
	min = 30000 ;
	p = 0 ;
	for ( i=0; i<rnavi->n_nodes; i++ ) {
		_sceVu0SubVector(  &tmp, &( rnavi->nodes[ i ] ), pos ) ;
		len = _FVecLen3( &tmp ) ;
		if ( len < min ) {
			min = len ;
			p = i ;
		}
	}
	
	return p ;
}

/* 指定ノードの一番近いルートが優先され、それより近い場所でも
	レンジ以上離れていないと選択されない */
int	ENE_SearchInRangeRootPoint( rnavi, pos, node, range )
ROUTENAVI	*rnavi ;
FVECTOR		*pos ;
int node ;
int range ;
{
	int	i, min, len, p, n, n_len ;
	FVECTOR	tmp, node_pos ;
	
	p = node ;
	n = node ;
	node_pos = rnavi->nodes[ n ] ;
	_sceVu0SubVector(  &tmp, &( rnavi->nodes[ n ] ), pos ) ;
	min = _FVecLen3( &tmp ) ;
	for ( i=0; i<rnavi->n_nodes; i++ ) {
		_sceVu0SubVector(  &tmp, &( rnavi->nodes[ n ] ), pos ) ;
		len = _FVecLen3( &tmp ) ;
//printf("[%d] pos len = %d min=%d\n", n, len, min ) ;
		if ( len < min ) {
			_sceVu0SubVector(  &tmp, &( rnavi->nodes[ n ] ), &node_pos ) ;
			n_len = _FVecLen3( &tmp ) ;
//printf("[%d] range n_len = %d\n", n, n_len ) ;
			if ( n_len > range ) {
				min = len ;
				p = n ;
				node_pos = rnavi->nodes[ n ] ;
			}
		}
		if ( ++n >= rnavi->n_nodes ) n = 0 ;
	}
	
	return p ;
}

void ENE_SetNextnode( rnavi )
ROUTENAVI	*rnavi ;
{
	int		num ;

	num = rnavi->next_node + 1 ;
	if ( num >= rnavi->n_nodes ) {
		num = ( COM_StageKind() & ENE_STAGE_ROOT_NO_LOOP )? rnavi->n_nodes-1 : 0 ;
	}

	rnavi->next_node = num ;
}

void ENE_SetTrgpNextnode( rnavi, trgp )
ROUTENAVI	*rnavi ;
TRGPOINT	*trgp ;
{
	ENE_SetNextnode( rnavi ) ;
	trgp->pos = rnavi->nodes[ (short)rnavi->next_node ] ;
	trgp->map = rnavi->mapbit[ (short)rnavi->next_node ] ;
	trgp->addr = HZX_GetAddress( trgp->map, &trgp->pos, -1 ) ;
}

void ENE_SetTrgpNode( rnavi, trgp )
ROUTENAVI	*rnavi ;
TRGPOINT	*trgp ;
{
	if ( rnavi->next_node >= rnavi->n_nodes ) rnavi->next_node = rnavi->n_nodes-1 ;

	trgp->pos = rnavi->nodes[ (short)rnavi->next_node ] ;
	trgp->map = rnavi->mapbit[ (short)rnavi->next_node ] ;
	trgp->addr = HZX_GetAddress( trgp->map, &trgp->pos, -1 ) ;
}

/* 次のノード番号を返す */
int ENE_SetTrgpFromRnaviNode( rnavi, trgp, node )
ROUTENAVI	*rnavi ;
TRGPOINT	*trgp ;
int			node ;
{
	trgp->pos = rnavi->nodes[ node ] ;
	trgp->map = rnavi->mapbit[ node ] ;
	trgp->addr = HZX_GetAddress( trgp->map, &trgp->pos, -1 ) ;

	node ++ ;
	if ( node >= rnavi->n_nodes ) node = 0 ;

	return node ;
}

/* プレイヤー位置セット */
void ENE_SetTrgpPlayer( znavi, trgp, com )
ZONENAVI	*znavi ;
TRGPOINT	*trgp ;
COMMANDER	*com ;
{
	trgp->addr = com->plzone_in_zone[PL_POS_CENTER] ;
	trgp->pos  = com->plpos_in_zone[PL_POS_CENTER] ;
	trgp->map  = com->plmap_in_zone[PL_POS_CENTER] ;
}

void ENE_SetTrgpNearRootPoint( rnavi, znavi, trgp, pos )
ROUTENAVI	*rnavi ;
ZONENAVI	*znavi ;
TRGPOINT	*trgp ;
FVECTOR		*pos ;
{
#if 1
	rnavi->next_node = ENE_SearchInRangeRootPoint( rnavi, pos, rnavi->next_node, 1000 ) ;
#else
	rnavi->next_node = ENE_SearchNearRootPoint( rnavi, pos ) ;
#endif
	trgp->pos = rnavi->nodes[ (int)rnavi->next_node ] ;
	trgp->map = rnavi->mapbit[ (short)rnavi->next_node ] ;
	trgp->addr = HZX_GetAddress( trgp->map, &trgp->pos, -1  ) ;

printf("near root:node[%d] map[%x] addr[%x]\n",rnavi->next_node,trgp->map,trgp->addr ) ;
}

void ENE_SetTrgpPoint( trgp, pos, hzx_id )
TRGPOINT	*trgp ;
FVECTOR		*pos ;
int			hzx_id ;
{
	trgp->pos = *pos ;
	trgp->addr = HZX_GetAddress( hzx_id, pos, -1 ) ;
	trgp->map = hzx_id ;

//printf("map[%x]",hzx_id ) ;
//printf("pos[%f][%f][%f]",pos->vx,pos->vy,pos->vz ) ;
//printf("addr[%x]\n",trgp->addr ) ;

}

/* 場所とマップビットから */
void ENE_SetTrgpPosMap( trgp, pos, map )
TRGPOINT	*trgp ;
FVECTOR		*pos ;
int			map ;
{
	int	hzx_id, zone ;

	hzx_id = HZX_GetHzxIDbyZone( GM_GetHzxGroupID(map), pos, &zone ) ;

	trgp->pos = *pos ;
	trgp->addr = HZX_GetAddress( hzx_id, pos, -1 ) ;
	trgp->map = hzx_id ;

printf("map[%x]",hzx_id ) ;
printf("pos[%f][%f][%f]",pos->vx,pos->vy,pos->vz ) ;
printf("addr[%x]\n",trgp->addr ) ;
}

/* 待機所セット */
void ENE_SetTrgpWaitPoint( entk, trgp )
ENETHINK	*entk ;
TRGPOINT	*trgp ;
{
	FVECTOR	pos ;
	int map ;
	
	if ( (GM_GameStatus & STATE_VR_ANOTHER) &&
		 ( entk->waiting_pos_num >= 0 ) ){
		pos = entk->com->waiting_pos[entk->waiting_pos_num] ;
		map = entk->com->wait_map[entk->waiting_pos_num] ;
	} else {
		COM_WaitingPosition( &pos, &map ) ;
	}
	ENE_SetTrgpPoint( trgp, &pos, map ) ;
}


/* 任意敵兵位置セット */
void ENE_SetTrgpEnemy( entk, trgp )
ENETHINK	*entk ;
TRGPOINT	*trgp ;
{
	trgp->pos  = entk->ctrl->mov ;
	trgp->addr = entk->ctrl->addr ;
	trgp->map  = entk->ctrl->hzx_id ;
}

/* 視覚情報にセットされた目標の位置セット */
void ENE_SetTrgpEyei( eyei, trgp )
EYEINFO		*eyei ;
TRGPOINT	*trgp ;
{
	trgp->pos  = *eyei->pos ;
	trgp->addr = *eyei->addr ;
	trgp->map  = *eyei->map ;
}

void ENE_SetTrgpDefense( trgp, pos, grpid )
TRGPOINT	*trgp ;
FVECTOR		*pos ;
int			grpid ;
{
	trgp->pos = *pos ;
	trgp->addr = HZX_GetAddress( grpid, &trgp->pos, -1 ) ;
	trgp->map  = grpid ;
}

/* あとで */
/* クリアリングスタート地点セット */
void ENE_SetTrgpCSP( trgp, id, area )
TRGPOINT	*trgp ;
int			id ;
int			area ;
{
	HZX_CLE_PTP		*point ;

	point = ENE_GetClearPoint2( area, id, 0 ) ;

	trgp->pos.vx = point->x ;
	trgp->pos.vy = point->y ;
	trgp->pos.vz = point->z ;
//printf(" clear start pos [%f] [%f] [%f] \n",trgp->pos.vx, trgp->pos.vy, trgp->pos.vz ) ;
	trgp->map = GV_GetBit(point->group_id) ;
	trgp->addr = HZX_GetAddress( trgp->map, &trgp->pos, -1 ) ;

}

void ENE_SetTrgpZone( trgp, zone, grpid )
TRGPOINT	*trgp ;
int			zone ;
int			grpid ;
{
	HZX_ZON		*hzx_zone ;

	hzx_zone = HZX_GetZone( grpid, zone ) ;
	trgp->pos.vx = (float)hzx_zone->x ;
	trgp->pos.vy = (float)hzx_zone->y ;
	trgp->pos.vz = (float)hzx_zone->z ;

	trgp->addr = HZX_Address( grpid, zone, zone ) ;
	trgp->map  = grpid ;
}

void ENE_SetTrgpZoneaddr( trgp, zoneaddr )
TRGPOINT		*trgp ;
HZX_ZONE_ADD	zoneaddr ;
{
	HZX_ZON		*hzx_zone ;

	hzx_zone = HZX_GetZoneFromAdd( zoneaddr ) ;
	trgp->pos.vx = (float)hzx_zone->x ;
	trgp->pos.vy = (float)hzx_zone->y ;
	trgp->pos.vz = (float)hzx_zone->z ;

	trgp->addr = zoneaddr ;
	trgp->map  = GM_GetBit( HZX_ZoneMapNo( zoneaddr ) ) ;
}

/* 指定ゾーンからの安全地帯のいずれかをセット */
void ENE_SetTrgpSafeArea( trgp, zone, grpid )
TRGPOINT	*trgp ;
int			zone ;
int			grpid ;
{
	int		tmpsafe, safe, i, fromadd ;
	HZX_ZON		*hzx_zone ;

	fromadd = HZX_Address( grpid, zone, zone ) ;
	hzx_zone = HZX_GetZone( grpid, zone ) ;
	safe = -1 ;

	/*安全地帯候補4個から適切なものを検索*/
	for(i=0;i<SAFE_NUM;i++){
		/*候補地を取得*/
		tmpsafe = hzx_zone->safes[i];

		/*異常値*/
		if(tmpsafe == 255) continue;
		{
			R_INTRPT	*r_intrpt ;
			int			addr ;

			addr = HZX_Address( grpid, tmpsafe, tmpsafe ) ;
			r_intrpt = GM_GetRIntrptZ2Z( fromadd, addr ) ;
			if ( r_intrpt != NULL ) {
printf("safe area intrpt[%x] \n", addr );
				continue ;
			}
		}

		safe = tmpsafe ;
	}
	if ( safe >= 0 ) {
		ENE_SetTrgpZone( trgp, safe, grpid ) ;
	} else {
		ENE_SetTrgpZone( trgp, zone, grpid ) ;
	}
	
	printf(" zone[%x] > safe [%x] \n",zone, safe ) ;
}

/* 逃げゾーン */
void ENE_SetTrgpRunawayArea( trgp, zone, grpid )
TRGPOINT	*trgp ;
int			zone ;
int			grpid ;
{
	int		tmpsafe, safe, i, fromadd ;
	HZX_ZON		*hzx_zone ;

	fromadd = HZX_Address( grpid, zone, zone ) ;
	hzx_zone = HZX_GetZone( grpid, zone ) ;
	safe = -1 ;

	/*安全地帯候補4個から適切なものを検索*/
	for(i=0;i<SAFE_NUM;i++){
		/*候補地を取得*/
		tmpsafe = hzx_zone->safes[i];

		/*異常値*/
		if(tmpsafe == 255) continue;
		{
			R_INTRPT	*r_intrpt ;
			int			addr ;

			addr = HZX_Address( grpid, tmpsafe, tmpsafe ) ;
			r_intrpt = GM_GetRIntrptZ2Z( fromadd, addr ) ;
			if ( r_intrpt != NULL ) {
printf("safe area intrpt[%x] \n", addr );
				continue ;
			}
		}

		safe = tmpsafe ;
	}
	if ( safe >= 0 ) {
		ENE_SetTrgpZone( trgp, safe, grpid ) ;
	} else {
		ENE_SetTrgpZone( trgp, zone, grpid ) ;
	}
	
	printf(" zone[%x] > safe [%x] \n",zone, safe ) ;
}


void ENE_SetTrgp( trgp, pos, addr )
TRGPOINT	*trgp ;
FVECTOR		*pos ;
int			addr ;
{
	trgp->pos = *pos ;
	trgp->addr = addr  ;
	trgp->map = HZX_ZoneGroupID(addr) ;
}

void ENE_SetTrgpZoneSide( trgp, addr, n_side )
TRGPOINT	*trgp ;
HZX_ZONE_ADD addr ;
int n_side ;
{
	HZX_ZON	*z ;

	z = HZX_GetZoneFromAdd( addr ) ;
	trgp->pos.vy = z->y ;
	
	switch( n_side ) {
		case SIDE_D :
			trgp->pos.vx = z->x ;
			trgp->pos.vz = z->z + (float)z->h ;
		break ;
		case SIDE_R :
			trgp->pos.vx = z->x + (float)z->w ;
			trgp->pos.vz = z->z ;
		break ;
		case SIDE_U :
			trgp->pos.vx = z->x ;
			trgp->pos.vz = z->z - (float)z->h ;
		break ;
		case SIDE_L :
			trgp->pos.vx = z->x - (float)z->w ;
			trgp->pos.vz = z->z ;
		break ;
	}

	trgp->addr = addr  ;
	trgp->map = HZX_ZoneGroupID(addr) ;
}


/*-----  --------------------------------------------------*/
static void ENE_MotStepCorrect( ACTION *act ) ;

static void ENE_UnrealStatus( ENETHINK *entk )
{
	ACTION	*act ;
	BODYPARAM	*bodyp ;

	act = entk->act ;
	bodyp = &act->bodyp ;

	KR_SetAllObjsFlag( act->body->objs, DG_FLAG_INVISIBLE ) ;

	UNSET_FLAG( act->body->objs->flag, DG_FLAG_SHADOWMAKE ) ;
	/*重野追加 外部からunreal状態を検知できるようにフラグ追加*/
	SET_FLAG( entk->hom.status, HOMING_SKIP|HOMING_UNREAL ) ;
	AT_SetTargetClass( act, TARGET_SKIP ) ;
	SET_FLAG( act->bodyp.pushtrg.class, TARGET_SKIP ) ;
	/*sigeno add UNREALはスティンガーサイトに写らない*/
	UNSET_FLAG( bodyp->deftrg.class, TARGET_LOCKON ) ;
	UNSET_FLAG( entk->sw_gun, SW_FLAG_SWITCH2 ) ;
	act->body->objs->flag |= DG_FLAG_INVISIBLE ;
	entk->weapon->objs->flag |= DG_FLAG_INVISIBLE ;
	if(entk->name_id.sub_weapon != 0 ){
		entk->sub_weapon->objs->flag |= DG_FLAG_INVISIBLE ;
	}
	/*sigeno add************************************/
//	if ( ENE_PureAttacker( entk->act ) ) {

	/*可動部のある武器を消す*/
	if(entk->act->bodyp.type & ENE_EQUIP_MOVABLE) {
		act->sw->wctrl2.flag |= WPEF_ENEMY_UNREAL ;
//		WeaponEfAct( &(entk->sw.wctrl2) );
/*slinggun.c から呼ぶのに変更*/
	}
	/*サブウエポン組み込み兵サブウエポンにもアンリアルを設定*/
	if(entk->name_id.sub_weapon != 0 ){
		act->sw->wctrl.flag |= WPEF_ENEMY_UNREAL ;
		WeaponEfAct( &(entk->sw.wctrl) );
	}

	entk->sw_shadow = 0 ;
	entk->sw_light = 0 ;
	entk->alert = 0 ;
	UNSET_FLAG( entk->sense.rctrl.flag, RADAR_VISIBLE ) ;

	if ( act->sw->vibration & VAR_FLAG_LOOP ) {
		SET_FLAG( act->sw->vibration, VAR_FLAG_PLAY_STOP ) ; 
	}
}

void ENE_GM_Act( entk )
ENETHINK	*entk ;
{
	extern void MoveHumanMA(HUMANMA_WORK *work,int flag);
	ACTION	*act ;

	act = entk->act ;

	if ( (act->status & ACT_STATUS_UNREAL) ||
		 (entk->status & ENE_STATUS_EVER_UNREAL) ) {
		/* メッセージチェックのみ行う */
		entk->ctrl->n_msg = GV_ReceiveMessage( entk->ctrl->name, &( entk->ctrl->msg ) ) ;

		return ;
	}

	act->old_body_height = entk->act->body->height ;
    GM_ActMotion( entk->act->body ) ;
	entk->ctrl->height = entk->act->body->height ;
//	entk->ctrl->height = entk->act->body->height*10 ;

/* 重野追加 step値加工 */

	ENE_MotStepCorrect(entk->act ) ;

	GM_ActControl( entk->ctrl ) ;

#ifndef NO_KANO_IK
	ENE_SetIk( entk ) ;
#endif

	GM_ActObject2( entk->act->body );

#ifdef TAKABE_IK
	ENE_NewSetIk( entk ) ;
#endif

	DG_GetLightMatrix( &entk->ctrl->mov, entk->lights );

	if ( entk->vanime_head != NULL ) MoveHumanMA( entk->vanime_head, entk->vanime_flag);

//	if ( ENE_PureAttacker( entk->act ) ) {
	if ( entk->sw.sub_weapon ) {
		entk->sub_weapon->objs->flag &= ~DG_FLAG_INVISIBLE ;
		act->sw->wctrl.flag &= ~WPEF_ENEMY_UNREAL ;
		WeaponEfAct( &(entk->sw.wctrl) );
		/*本体と描画マップをあわせる*/
		KR_GroupObject( entk->sub_weapon, entk->act->body->map_name ) ;
	} else {
		if(entk->name_id.sub_weapon != 0 ){
			entk->sub_weapon->objs->flag |= DG_FLAG_INVISIBLE ;
		}
	}

/*可動部のある武器はWeaponEfActを呼ぶ*/
	if(entk->act->bodyp.type & ENE_EQUIP_MOVABLE) {
		act->sw->wctrl2.flag &= ~WPEF_ENEMY_UNREAL ;
//		WeaponEfAct( &(entk->sw.wctrl2) );
/*slinggun.c から呼ぶのに変更*/
	}

	/*グレネードランチャ薬室　フタ開閉制御*/
	if(entk->act->bodyp.type & ENE_TYPE_M4) {
		extern void SK_BlowBack_M4A1( void *p , char phase ) ;
		switch (entk->act->sw->chamber2){
			case CBR2_OPEN :
				SK_BlowBack_M4A1( &(act->sw->wctrl2),1 );
				break ;
			case CBR2_CLOSE :
				SK_BlowBack_M4A1( &(act->sw->wctrl2),2 );
				break ;
			default :
				SK_BlowBack_M4A1( &(act->sw->wctrl2),0 );
				break ;
		}
	}
    /* ＳＥ変換用 */
    MT_SetMotionSeTable( entk->act->body->m_ctrl, GM_CurrentMap, (entk->id%4)+1, 
			 ( entk->ctrl->flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 ) ;

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		if ( entk->com->status & CMST_ENEMY_BODY_LIGHT ) {
			entk->lights[1].m[3][0] = 128 ;
			entk->lights[1].m[3][1] = 128 ;
			entk->lights[1].m[3][2] = 128 ;
		}
		if ( entk->com->status & CMST_ENEMY_BODY_LIGHT ) {
			entk->head_num |= (1 << 2) ;
		} else {
			entk->head_num &= ~(1 << 2) ;
		}
	}
#endif

}

static	void	CheckMessage( entk )
ENETHINK	*entk ;
{
	GV_MSG	*msg ;
	int n_msg, code ;

    n_msg = entk->ctrl->n_msg ;
	msg = entk->ctrl->msg ;
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case ENE_MSG_TRAP_SHADOW_ON :
				SET_FLAG( entk->status, ENE_STATUS_TRAPSHADOW_ON ) ;
			break ;
			case ENE_MSG_TRAP_SHADOW_OFF :
				UNSET_FLAG( entk->status, ENE_STATUS_TRAPSHADOW_ON ) ;
			break ;
			case ENE_MSG_EVER_DOWN_OFF :
				UNSET_FLAG( entk->status, ENE_STATUS_EVER_ZZZ ) ;
			break ;
			case ENE_MSG_EVER_DOWN_ON :
				SET_FLAG( entk->status, ENE_STATUS_EVER_ZZZ ) ;
			break ;
			case ENE_MSG_SET_DOWN_TIME :
				if ( entk->act->status_status & (ACT_STST_PIYOPIYO|ACT_STST_ZZZ) ) {
					entk->act->bodyp.faint_time = msg->message[ 1 ] ;
					entk->act->bodyp.faint_time = COUNT_VMODE( entk->act->bodyp.faint_time ) ;
				} else {
					printf("ene message: Enemy No Down!!\n");
				}
printf("ene message: enemy faint time [%d]!!\n",msg->message[ 1 ]);
			break ;
			case ENE_MSG_UNREAL_ON :
				SET_FLAG( entk->status, ENE_STATUS_EVER_UNREAL ) ;
				ENE_UnrealStatus( entk ) ;
				ENE_SetHeadMark( entk->act, 0, HEADMARK_CLEAR ) ;
			break ;
			case ENE_MSG_UNREAL_OFF :
				UNSET_FLAG( entk->status, ENE_STATUS_EVER_UNREAL ) ;
				KR_UnsetAllObjsFlag( entk->act->body->objs, DG_FLAG_INVISIBLE ) ;
printf("ene message: ENE_MSG_UNREAL_OFF !!\n" );
			break ;
		}
		msg++ ;
	}
}

void ENE_PreProcess( entk )
ENETHINK	*entk ;
{
	int z1, z2 ;

	/* 敵兵共通メッセージの受け取り */
	CheckMessage( entk ) ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		entk->act->sw->headmark = 0 ;
	}

	if ( entk->status & ENE_STATUS_EVER_UNREAL ) return ;
	if ( entk->act->status & ACT_STATUS_UNREAL ) return ;

	/* フロア接地場所 */
	entk->znavi->flore_pos = entk->ctrl->mov ;
	entk->znavi->flore_pos.vy = entk->ctrl->levels[0] ;

    /* ゾーン処理 */
    /* あとで */
	z1 = 255 & entk->ctrl->addr ;
	z2 = 255 & ( entk->ctrl->addr >> 8 ) ;
    if ( z1 == z2 ) {
		if ( entk->ctrl->addr != entk->before_inzone[0] ) {
			 entk->before_inzone[ 3 ] = entk->before_inzone[ 2 ] ;
			 entk->before_inzone[ 2 ] = entk->before_inzone[ 1 ] ;
			 entk->before_inzone[ 1 ] = entk->before_inzone[ 0 ] ;
			 entk->before_inzone[ 0 ] = entk->ctrl->addr ;
		}
	}

	/* 視力セット */
	entk->sense.eye_s = entk->sense.eye_s_s[GM_AlertMode] ;
}

void ENE_AftProcess( entk )
ENETHINK	*entk ;
{
	BODYPARAM		*bodyp ;
	OBJECT		*body ;
	CONTROL	*ctrl ;

	entk->receive = FLAG_CLEAR ;
	if ( entk->status & ENE_STATUS_EVER_UNREAL ) return ;
	if ( entk->act->status & ACT_STATUS_UNREAL ) return ;

	ctrl = entk->ctrl ;
	bodyp = &( entk->act->bodyp ) ;
	body = entk->act->body ;

	/* ターゲットのマップはカレントマップがセットされる */
    GM_CurrentMap = ctrl->map ;

	/* ターゲットの移動 */
    GM_MoveTarget( &( bodyp->deftrg ), &( entk->ctrl->mov ) ) ;
    GM_MoveTarget( &( bodyp->pushtrg ), &( entk->ctrl->mov ) ) ;

	/* 子ターゲットの移動 */
	/* Level 0 */
    GM_MoveTarget2( &( bodyp->def_child0[ PTARGET_RADIO ] ), &(BODYWORLD( body, HUMAN21_KOSHI )) ) ;
    GM_MoveTarget2( &( bodyp->def_child0[ PTARGET_NVISION1 ] ), &(BODYWORLD( body, HUMAN21_ATAMA )) ) ;
    GM_MoveTarget2( &( bodyp->def_child0[ PTARGET_NVISION2 ] ), &(BODYWORLD( body, HUMAN21_ATAMA )) ) ;
	/* Level 1 */
    GM_MoveTarget2( &( bodyp->def_child1[ PTARGET_HEAD ] ), &(BODYWORLD( body, HUMAN21_ATAMA )) ) ;
	GM_MoveTarget2( &( bodyp->def_child1[ PTARGET_HART ] ), &(BODYWORLD( body, HUMAN21_MUNE )) ) ;
	GM_MoveTarget2( &( bodyp->def_child1[ PTARGET_GOLD ] ), &(BODYWORLD( body, HUMAN21_KOSHI )) ) ;
	/* Level 2 */
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARMR1 ] ), &(BODYWORLD( body, HUMAN21_MIGI_UDE1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARMR2 ] ), &(BODYWORLD( body, HUMAN21_MIGI_UDE2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARML1 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_UDE1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARML2 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_UDE2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGR1 ] ), &(BODYWORLD( body, HUMAN21_MIGI_ASHI1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGR2 ] ), &(BODYWORLD( body, HUMAN21_MIGI_ASHI2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGL1 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_ASHI1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGL2 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_ASHI2 )) ) ;
	/* Level 3 */
    GM_MoveTarget2( &( bodyp->def_child3[ PTARGET_BODY ] ), &(BODYWORLD( body, HUMAN21_MUNE )) ) ;

	/* 現フレームの方向を保存 */
	entk->act->before_dir = entk->act->ctrl->rot.vy ;

	/*VR モードロックオンサイト*/
	if ( entk->status_status & ENE_STST_VRBODY ) {
		extern	void VR_CheckHomingStatus(HOMING_TRG *hom_trg ,int *sw ) ;
		VR_CheckHomingStatus(&entk->hom ,&entk->lockon ) ;
	}

#ifdef DEBUG_MODE
{
	if ( entk->com->status & CMST_ENEMY_GUN_CUT ) {
		entk->bullet = 0 ;
	}
}
#endif
}

void	ENE_SaveEneMemory( ENETHINK *entk )
{
	ENEMEM enm ;
	ACTION *act ;
	CONTROL	*ctrl ;
	int status, pose, item ;

	act = entk->act ;
	ctrl = entk->ctrl ;

printf("save id[%d]\n",entk->id ) ;

	status = ENEMEM_ST_NORMAL ;
	pose = ENEMEM_PS_NORMAL ;
	item = 0 ;

	/* 彼の世 */
	if ( act->status & ACT_STATUS_GHOST ) {
		status = ENEMEM_ST_HELL ;
	}

	/* 気絶＆眠り */
	if ( act->status_status & (ACT_STST_PIYOPIYO|ACT_STST_ZZZ) ) {
		/* status */
		status=(entk->act->status_status & ACT_STST_PIYOPIYO) ? ENEMEM_ST_FAINT:ENEMEM_ST_SLEEP ;

		/* pose */
		if ( act->status & ACT_STATUS_LOCKER ) {
			pose = ENEMEM_PS_LOCKER ;
		} else if ( act->down_s == DownFront ) {
			pose = ENEMEM_PS_FRONT ;
		} else if ( act->down_s == DownWall ) {
			pose = ENEMEM_PS_WALL ;
		} else {
			pose = ENEMEM_PS_BACK ;
		}

		item = ENEMEM_ITEMS( act->item.c_proc, act->hold_item.c_proc ) ;
	}

	/* 通常 */
	ENEMEM_Set( &enm, GM_CurrentStage, ctrl->hzx_id, ctrl->name, &ctrl->mov, ctrl->rot.vy,
				status, pose, act->bodyp.faint_time, item,
				act->bodyp.life, act->bodyp.faint, act->bodyp.pbreak, 0 ) ;
	ENEMEM_Save( &enm ) ;
}

void	ENE_SaveDeadEneMemory( ENETHINK *entk )
{
	ENEMEM enm ;
	ACTION *act ;
	CONTROL	*ctrl ;
	int status, pose, item ;

	/* 生前の名前がなければセーブしない */
printf("dead ene save name[%d] faint[%d] \n",entk->alive_name, entk->act->bodyp.faint_time ) ;

	if ( entk->alive_name == 0 ) return ;
	if ( entk->act->bodyp.faint_time < 300 ) return ;

	act = entk->act ;
	ctrl = entk->ctrl ;


	status = ENEMEM_ST_NORMAL ;
	pose = ENEMEM_PS_NORMAL ;
	item = 0 ;

	/* 彼の世 */
	status = ENEMEM_ST_DEAD ;

	/* pose */
	if ( act->status & ACT_STATUS_LOCKER ) {
		pose = ENEMEM_PS_LOCKER ;
	} else if ( act->down_s == DownFront ) {
		pose = ENEMEM_PS_FRONT ;
	} else if ( act->down_s == DownWall ) {
		pose = ENEMEM_PS_WALL ;
	} else {
		pose = ENEMEM_PS_BACK ;
	}
	item = ENEMEM_ITEMS( act->item.c_proc, act->hold_item.c_proc ) ;

	/* 通常 */
	ENEMEM_Set( &enm, GM_CurrentStage, ctrl->hzx_id, ctrl->name, &ctrl->mov, ctrl->rot.vy,
				status, pose, act->bodyp.faint_time, item,
				act->bodyp.life, act->bodyp.faint, act->bodyp.pbreak, entk->alive_name ) ;
	ENEMEM_Save( &enm ) ;
}


#define LOAD_MEM_TEST	(1)

void ENE_FreeResources( entk )
ENETHINK	*entk ;
{
	extern void DG_DisconnectObjs( DG_OBJS *, DG_OBJS * ) ;
	extern void ExitCorpseIK(void *work);
	extern void SearchAndKillAttachment_called(OBJECT *obj) ;
	extern int NDETECT_Remove( void* pvNDetect) ;


#ifdef LOAD_MEM_TEST
	if ( (GV_IsStageDestroy( entk->w )) ) {
		if ( entk->status & ENE_STATUS_DEAD ) {
			ENE_SaveDeadEneMemory( entk ) ;	/* ステージロードのみ */
		} else {
			ENE_SaveEneMemory( entk ) ;	/* ステージロードのみ */
		}
	}
#endif

	if ( entk->d_name != NULL ) NDETECT_Remove( entk->d_name ) ;
	if ( entk->vanime_head != NULL )  ExitHumanMA( entk->vanime_head ) ;
	if ( entk->sub_weapon != NULL ) {
		DG_DisconnectObjs( entk->act->body->objs, entk->sub_weapon->objs ) ;
		GM_FreeObject( entk->sub_weapon ) ;
	}
	if ( entk->lowshadow != NULL ) {
		DG_FreeObjs( entk->lowshadow ) ;
	}

#ifdef TAKABE_IK
	TAKABE_FreePuppetIK( entk->act->new_ik ) ;
#endif
#ifndef NO_KANO_IK
	ExitCorpseIK( entk->act->ik ) ;
#endif	

	if ( !(GV_IsStageDestroy( entk->w )) ) {
		if ( entk->act->oozeblood != NULL ) {
			GV_DestroyActorQuick( entk->act->oozeblood ) ;
		}
	}

	SearchAndKillAttachment_called( entk->act->body ) ;
	GM_FreeZoneIntrpt( &(entk->z_intrpt) ) ;
	GM_FreeHomingTrg( &(entk->hom) ) ;
    GM_FreeRadarControl( &( entk->sense.rctrl ) ) ;
    GM_FreeControl( entk->ctrl ) ;
    GM_FreeObject( entk->act->body ) ;
    GM_FreeTarget( &entk->act->bodyp.deftrg ) ;	/* 親だけで良い */
    GM_FreeTarget( &entk->act->bodyp.pushtrg ) ;
	/*重野追加 ムカデＩＫワーク開放*/
	if(entk->arm_ik != NULL ){
		ExitArmIK(entk->arm_ik) ;
		entk->arm_ik = NULL ; 
	}
}

void	ENE_ActInit( entk )
ENETHINK	*entk ;
{
	BODYPARAM	*bodyp ;
	CONTROL		*ctrl ;
	OBJECT		*body ;
	int			ane, dam ;

	ctrl = entk->ctrl ;
	body = entk->act->body ;
	bodyp = &entk->act->bodyp ;
	bodyp->deftrg.class = ENE_DEF_TARGET_CLASS ;	/* ターゲットクラス初期化 */
	if(entk->status &ENE_STATUS_NO_CAPTURE){
		UNSET_FLAG( bodyp->deftrg.class, TARGET_CAPTURE ) ;
	}
	body->m_ctrl->adjust_flag = 0 ;	/* アジャストフラグ初期化 */
	body->flag = 0 ;				/* オブジェクトフラグ初期化 */
	UNSET_FLAG( entk->hom.status, HOMING_SKIP ) ;		/* ホーミングステータス初期化 */

	if ( --(bodyp->stand) < 0 ) bodyp->stand = 0 ;

	/* 出血量計算 */
	dam = 0 ;
#ifndef NO_BLOOD_OUT
	dam += bodyp->dam_level_num[0] * 0 ;	/* 体外 */
	dam += bodyp->dam_level_num[1] * 3000 ;	/* 急所 */
	dam += bodyp->dam_level_num[2] * 1 ;	/* 四肢 */
	dam += bodyp->dam_level_num[3] * 2 ;	/* 体 */
#endif
	bodyp->blood -= dam ;
	if ( bodyp->blood < 0 ) bodyp->blood = -1 ;

	/* 麻酔耐久度計算 */
	ane = 0 ;

	if ( GM_GameLevel < GM_LEVEL_EASY ) {
		ane += bodyp->ane_level_num[0] * 0 ;	/* 体外 */
		ane += bodyp->ane_level_num[1] * 3000 ;	/* 急所 */
		ane += bodyp->ane_level_num[2] * 3000 ;	/* 四肢 */
		ane += bodyp->ane_level_num[3] * 3000 ;	/* 体 */
	} else if ( GM_GameLevel < GM_LEVEL_NORMAL ) {
		ane += bodyp->ane_level_num[0] * 0 ;	/* 体外 */
		ane += bodyp->ane_level_num[1] * 3000 ;	/* 急所 */
#if 1	// JAPANESE_BP_IGNORE()から
		ane += bodyp->ane_level_num[2] * 3 ;	/* 四肢 */
		ane += bodyp->ane_level_num[3] * 6 ;	/* 体 */
#else
		ane += bodyp->ane_level_num[2] * 4 ;	/* 四肢 */
		ane += bodyp->ane_level_num[3] * 8 ;	/* 体 */
#endif
	} else {
		ane += bodyp->ane_level_num[0] * 0 ;	/* 体外 */
		ane += bodyp->ane_level_num[1] * 3000 ;	/* 急所 */
		ane += bodyp->ane_level_num[2] * 2 ;	/* 四肢 */
		ane += bodyp->ane_level_num[3] * 4 ;	/* 体 */
	}


	bodyp->anesthesia -= ane ;
	if ( bodyp->anesthesia < 0 ) bodyp->anesthesia = -1 ;

	/* 実験 */
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		SET_FLAG( entk->thk_status, THK_STATUS_SIGHT_IN ) ;
	}
}

void	ENE_SetHeadMark2( act, mark_num, mode  )
ACTION	*act ;
int		mark_num ;
int		mode ;
{
	act->sw->headmark = mark_num ;
	act->headmark2 = mode ;

	if ( mode == ACT_HEADMARK2_FAINT )	act->headmark2_num = 3 ;	/* 初期個数は３個 */
	if ( mode == ACT_HEADMARK2_ANES )	act->headmark2_num = 4 ;	/* 初期個数は４個 */

	CallActHeadMarks( act->headmarkwork, mode );
}


#define TRC_WALKMAN1 (16)
void	ENE_ActStatusCheck ( entk )
ENETHINK	*entk ;
{
	extern void JumpFreeCorpse(void *) ;

	ACTION	*act ;
	BODYPARAM	*bodyp ;
	int			len, range ;

	act = entk->act ;
	bodyp = &act->bodyp ;
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		SET_FLAG( act->status, ACT_STATUS_UNREAL )  ;
	}

	if ( act->status & ACT_STATUS_UNREAL ) {
		ENE_UnrealStatus( entk ) ;
		if ( !(act->old_status & ACT_STATUS_UNREAL) ) {
			if ( GM_GameStatus & STATE_VR_ANOTHER ) {
				if ( !(COM_StageKind() & ENE_STAGE_NO_APPER_EFFE) ) {
					ENE_SetHeadMark( entk->act, 0, HEADMARK_CLEAR ) ;
					ENE_AppearEffect( entk, 2 ) ;
				}
			}
		}

		return ;
	}
	if ( act->old_status & ACT_STATUS_UNREAL ) {
		KR_UnsetAllObjsFlag( act->body->objs, DG_FLAG_INVISIBLE ) ;
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			if ( !(COM_StageKind() & ENE_STAGE_NO_APPER_EFFE) ) {
				ENE_AppearEffect( entk, 0 ) ;
			}
		}
	}
	/*重野追加*/
	UNSET_FLAG( entk->hom.status, HOMING_UNREAL ) ;

	/* 心臓の鼓動 */
	{
		int beat ;

		beat = act->bodyp.heart_beat ;
		if ( act->status & ACT_STATUS_FAINT ) {
			beat = beat * 3 / 2 ;
		} else if ( (GM_AlertMode == ALERT_MODE_ALERT) || (GM_AlertMode == ALERT_MODE_AVOID)) {
			beat /= 2 ;
		}
		if ( !(GM_PlayTime % beat) ) {
			GM_SeSetMode( SD_E_EHEART01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_MIC ) ;
		}
	}

	if ( act->status & ACT_STATUS_FAINT ) {
		act->bodyp.deftrg.class = FAINT_TARGET_CLASS ;
		if(entk->status &ENE_STATUS_NO_CAPTURE){
			UNSET_FLAG( act->bodyp.deftrg.class, TARGET_CAPTURE ) ;
		}
		if ( act->headmark2 == ACT_HEADMARK2_ANES ) {/* 寝ていたら */
			AT_SetActStSt( act, ACT_STST_ZZZ ) ;
			if ( entk->status & ENE_STATUS_BIG_SNORE ) {
				COM_SetFlameFlag( CMFLAG_BIG_SNORE ) ;
			}

			if ( !GM_CheckGameStatus( STATE_GAMEOVER ) ) {
				if( (act->bodyp.faint_time > COUNT_VMODE(400)) ) {
					if( !(act->bodyp.faint_time%COUNT_VMODE(180)) ) {
						int snore ;

						snore = SD_V_GBSIBK01+(entk->id%4) ;
						if ( entk->status & ENE_STATUS_BIG_SNORE ) {
							snore = SD_V_GBSIBKS1 ;

							if ( Ply_GetPlayerWeapon() != WP_Mic ) {
								GM_SeSetMode( snore, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
							} else {
                        int pan;
                        float bp_angle;
                        pan = GM_SeGetPan( &act->ctrl->mov, GM_SEMODE_BOMB, &bp_angle);
								GM_SeSet3D( pan,GM_MAX_VOL, snore, bp_angle ) ;
							}
						} else {
							if ( !(entk->status & ENE_STATUS_EVER_ZZZ) ) {
								GM_SeSetMode( snore, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
							} else {
//								printf("koreeeeeeeeeeeeeeeeeeee\n");
							}
						}
					}
				}
			}
			if ( act->bodyp.faint_time < COUNT_VMODE(60*60) &&
				 act->headmark2_num != 1 ) {
				act->sw->headmark = HMK2_TYPE_VALUE|0 ;
				act->headmark2_num = 1 ;
				CallActHeadMarks( act->headmarkwork, act->sw->headmark );
printf("HEAD Mark Anes 1 !! \n");
			} else if ( act->bodyp.faint_time >= COUNT_VMODE(60*60) &&
						act->bodyp.faint_time < COUNT_VMODE(60*60*2) &&
						act->headmark2_num != 2 ) {
				act->sw->headmark = HMK2_TYPE_VALUE|1 ;
				act->headmark2_num = 2 ;
				CallActHeadMarks( act->headmarkwork, act->sw->headmark );
printf("HEAD Mark Anes 2 !! \n");
			} else if ( act->bodyp.faint_time >= COUNT_VMODE(60*60*2) &&
						act->bodyp.faint_time < COUNT_VMODE(60*60*3) &&
						act->headmark2_num != 3 ) {
				act->sw->headmark = HMK2_TYPE_VALUE|2 ;
				act->headmark2_num = 3 ;
				CallActHeadMarks( act->headmarkwork, act->sw->headmark );
printf("HEAD Mark Anes 3 !! \n");
			} else if ( act->bodyp.faint_time >= COUNT_VMODE(60*60*3) &&
						act->headmark2_num != 4 ) {
				act->sw->headmark = HMK2_TYPE_VALUE|3 ;
				act->headmark2_num = 4 ;
				CallActHeadMarks( act->headmarkwork, act->sw->headmark );
printf("HEAD Mark Anes 4 !! \n");
			}
		}

		if ( entk->status & ENE_STATUS_BIG_SNORE ) {
			if ( act->status & ACT_STATUS_STAND_ZZZ ) {
				COM_SetFlameFlag( CMFLAG_BIG_SNORE ) ;
			}
		}

		if ( act->headmark2 == ACT_HEADMARK2_FAINT ) {/* 気絶していたら */
			AT_SetActStSt( act, ACT_STST_PIYOPIYO ) ;
			if ( act->bodyp.faint_time < (FAINT_TIME/3) && act->headmark2_num != 1 ) {
printf("head mark num change 1 !! \n");
				act->sw->headmark = HMK2_TYPE_PIYO_VALUE|1 ;
				act->headmark2_num = 1 ;
				CallActHeadMarks( act->headmarkwork, act->sw->headmark );
			} else if ( act->bodyp.faint_time >= (FAINT_TIME/3) && 
						act->bodyp.faint_time < (FAINT_TIME*2/3) &&
						act->headmark2_num != 2 ) {
printf("head mark num change 2 !! \n");
				act->sw->headmark = HMK2_TYPE_PIYO_VALUE|2 ;
				act->headmark2_num = 2 ;
				CallActHeadMarks( act->headmarkwork, act->sw->headmark );
			}
		}
	}





	act->bodyp.deftrg.side = ( act->status & ACT_STATUS_TRG_PLAYER ) ? PLAYER_SIDE : ENEMY_SIDE ;

	if ( act->status & (ACT_STATUS_DOWN|ACT_STATUS_DEATH|ACT_STATUS_FALL) ) {
		act->body->objs->flag &= ~DG_FLAG_SHADOWMAKE ;
		entk->sw.splash = 0 ;
		ENE_PlayerOnCorp( &entk->ctrl->mov ) ;
	} else {
		if ( entk->status & ENE_STATUS_TRAPSHADOW ) {
			if ( entk->status & ENE_STATUS_TRAPSHADOW_ON ) {
				act->body->objs->flag |= DG_FLAG_SHADOWMAKE ;
			} else {
				act->body->objs->flag &= ~DG_FLAG_SHADOWMAKE ;
			}
		} else {
			act->body->objs->flag |= DG_FLAG_SHADOWMAKE ;
		}
		entk->sw.splash = 1 ;
	}

	if ( act->status & ACT_STATUS_FLR_OFF ) {
		act->ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;
	} else {
		act->ctrl->skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
	}
	if ( act->status & ACT_STATUS_SEG_OFF ) {
		act->ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK ;
	} else {
		act->ctrl->skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
	}
	if ( act->status & ACT_STATUS_TURN_SLOW ) {
//		GM_ConfigControlInterp( act->ctrl, 60 ) ;
	} else {
//		GM_ConfigControlInterp( act->ctrl, 0 ) ;
	}

	if ( entk->status2 & ENE_STATUS2_AIM_GUNSHOOT ) {
		ENE_GunAdjust( entk ) ;
	} else if ( act->status & ACT_STATUS_ADJ_X ) {
		ENE_SetAimPlayer( entk ) ;
		ENE_GunAdjust( entk ) ;
	} else {
		ENE_AdjustAimPoint( entk ) ;
	}
	if ( act->adj_piku_time > 0 ) {
		/* すべてのアジャスト操作後 */
		ENE_AdjustPiku( entk ) ;
	}
	if ( act->status & ACT_STATUS_HOMING_SKIP ) {
		entk->hom.status |= HOMING_SKIP ;
	}
	if ( act->status & ACT_STATUS_TARGET_SKIP ) {
		AT_SetTargetClass( act, TARGET_SKIP ) ;
	}
	if ( act->status & (ACT_STATUS_DOWN|ACT_STATUS_TARGET_SKIP|ACT_STATUS_PUSHT_SKIP|ACT_STATUS_FALL) ) {
		SET_FLAG( act->bodyp.pushtrg.class, TARGET_SKIP ) ;
	} else {
		UNSET_FLAG( act->bodyp.pushtrg.class, TARGET_SKIP ) ;
	}

#ifndef TAKABE_IK
#ifndef NO_KANO_IK
	if ( act->status & ACT_STATUS_IK_PIKU ) {
		if ( act->bodyp.n_damobj == HUMAN21_MUNE || act->bodyp.n_damobj == HUMAN21_ATAMA 
		|| act->bodyp.n_damobj == HUMAN21_KOSHI || act->bodyp.n_damobj == HUMAN21_ONAKA ) {
			JumpFreeCorpse( entk->act->ik ) ;
		} else {
			JumpFreeCorpse2( entk->act->ik,act->bodyp.n_damobj) ;
		}
		entk->act->ik_time = 1 ;
	}
#endif
#endif

	if ( act->status & ACT_STATUS_VANIME_HEAD ) {
		entk->vanime_flag = 1 ;
	} else {
		entk->vanime_flag = 0 ;
	}

	if ( act->status & ACT_STATUS_RESURRECT ) {
	}

	if ( act->status & ACT_STATUS_GLASS ) {
		entk->status2 |= ENE_STATUS_GLASSES ;
	}

	/* 装備品 */
	if ( entk->sw.sub_weapon ) act->status |= ACT_STATUS_GUN_FREE ;
	if ( entk->act->bodyp.pbreak & PBREAK_ARM_R ) act->status |= ACT_STATUS_GUN_FREE ;
	if ( act->status & ACT_STATUS_GUN_FREE ) {
		SET_FLAG( entk->sw_gun, SW_FLAG_SWITCH2 ) ;
	} else {
		UNSET_FLAG( entk->sw_gun, SW_FLAG_SWITCH2 ) ;
	}

	if ( act->status & (ACT_STATUS_GHOST|ACT_STATUS_INVISIBLE) ) {
		act->body->objs->flag |= DG_FLAG_INVISIBLE ;
		entk->weapon->objs->flag |= DG_FLAG_INVISIBLE ;
		entk->sw_shadow = 0 ;
		entk->sw_light = 0 ;
	} else {
		act->body->objs->flag &= ~DG_FLAG_INVISIBLE ;
		entk->weapon->objs->flag &= ~DG_FLAG_INVISIBLE ;

		entk->sw_shadow = ( act->status & ACT_STATUS_FAINT ) ? 0 : 1 ;

		if ( entk->com->status & CMST_ENEMY_GUNLIGHT_OFF ) {
			entk->sw_light = 0 ;
		} else {
			if ( (entk->status & ENE_STATUS_GUNLIGHT) ) {
				if ( act->status & ACT_STATUS_GUN_FREE ) {
					entk->sw_light =  0 ;
				} else {
					if ( act->status & ACT_STATUS_GUNLIGHT_OFF ) {
						entk->sw_light =  0 ;
					} else {
						if ( act->status & ACT_STATUS_GUNLIGHT_D ) {
							entk->sw_light =  2 ;
						} else {
							entk->sw_light =  1 ;
						}
					}
				}
			} else {
				if ( entk->status2 & ENE_STATUS2_GUNLIGHT ) {
					entk->sw_light =  1 ;
				} else {
					entk->sw_light =  0 ;
				}
			}
		}
	}

	/* 気絶値更新 */
	if ( !(entk->status & ENE_STATUS_EVER_ZZZ) ) {
		if ( --bodyp->faint_time < 0 )  bodyp->faint_time = 0 ;
	}

	{
		int correct;
		if( ( correct = act->body->m_ctrl->rot_correct ) ){
printf("correct[%d] rot[%d]\n",correct,act->ctrl->rot.vy);
			act->ctrl->rot.vy += correct;
			act->ctrl->rot.vy &=4095;
			act->ctrl->turn.vy = act->ctrl->rot.vy ;

			/* X軸方向の傾きは体が正面を向いている場合しかしない */
			if ( correct > 1024 || correct < -1024) {
				act->ctrl->rot.vx = act->ctrl->turn.vx = -act->ctrl->turn.vx ;
			}

			act->body->m_ctrl->rot_correct = 0;
		}
	}

	if ( entk->act->status & ACT_STATUS_EYE_CLOSE ) {
		len = 0 ;
		range = 0 ;
	} else if ( entk->status2 & ENE_STATUS_GLASSES ) {	/* 双眼鏡を装着していたら */
		len = entk->sense.eye_s + entk->sense.glasses_dis ;
		len = GV_NearExp4( entk->sense.rctrl.range, len ) ;
		range = DEF_GLASSES_RANGE * 2 ;
	} else {
		len = entk->sense.eye_s ;
		range = entk->sense.eye_r*2 ;
	}

	if ( entk->thk_status & (THK_STATUS_RADIO|THK_STATUS_ROOT_RADIO) ) {
		GM_RadarSetFlag( &entk->sense.rctrl, RADAR_RADIO ) ;
	} else {
		GM_RadarResetFlag( &entk->sense.rctrl, RADAR_RADIO ) ;
	}

	if ( entk->status2 & ENE_STATUS_GLASSES ) {	/* 双眼鏡を装着していたら */
		GM_RadarSetFlag( &entk->sense.rctrl, RADAR_NOFIX_SIGHT ) ;
	} else {
		GM_RadarResetFlag( &entk->sense.rctrl, RADAR_NOFIX_SIGHT ) ;
	}

	GM_RadarSetFlag( &entk->sense.rctrl, RADAR_VISIBLE ) ;
#if 1
//#define RADAR_VRANGE_UPPER	(1000.0F)
//#define RADAR_VRANGE_LOWER  (-800.0F)
//#define UNDER_EYE_SIGHT	512		/* 下方向視野 */
//#define UPPER_EYE_SIGHT	-512			/* 上方向視野 */
{
	int	upper, under ;

	if( entk->sense.facedir_x > 0 ) {
		upper = -(len * _RsinF( UPPER_EYE_SIGHT )) ;
		under = -(len * _RsinF( UNDER_EYE_SIGHT + entk->sense.facedir_x )) ;
	} else {
		upper = -(len * _RsinF( UPPER_EYE_SIGHT + entk->sense.facedir_x)) ;
		under = -(len * _RsinF( UNDER_EYE_SIGHT )) ;
	}

	if ( GM_GameStatus & STATE_VR_ONLY ) {
		upper += 1000 ;
		under -= 1000 ;
	} else {
		upper += 500 ;
		under -= 1500 ;
	}
	GM_RadarSetVRange( &entk->sense.rctrl, (float)upper, (float)under );
}
#endif
	GM_RadarSetSight( &entk->sense.rctrl, entk->sense.facedir, range, (float)len, entk->sense.status );

	/* ウォークマン兵ＢＧＭ調節 */
	if ( entk->status & ENE_STATUS_NORINORI ) {
		if ( (GM_AlertMode != ALERT_MODE_SNEAK) || 
			 ( act->status & ACT_STATUS_FAINT ) ||
			 !(HZX_CurrentGroupID & entk->ctrl->hzx_id) ) {
			GM_MixConvFader( entk->act->bgm_track, GM_PAN_CENTER, 0 ) ;
		} else {
			if ( entk->act->status & (ACT_STATUS_GHOST|ACT_STATUS_DEATH) ) {
				GM_MixConvFader( entk->act->bgm_track, GM_PAN_CENTER, 0 ) ;
			} else {
				GM_BgmFaderMode( entk->act->bgm_track, &(entk->ctrl->mov), GM_SEMODE_NORMAL ) ;
			}
		}
	}

	/* 顔の向きセット */
#if 0
	y = 0 ;
	if ( act->body->m_ctrl->mt3_ctrl[ 0 ].file_header->flag & MT3_FLAG_TURN_FLAG ){
		y = MatToYRot( &BODYWORLD( act->body, HUMAN21_KOSHI) ) ;
	}
	entk->sense.facedir = entk->ctrl->rot.vy+entk->act->adj_rot.vy+y ;
#else
	entk->sense.facedir = 4095 & MatToYRot( &BODYWORLD( entk->act->body, HUMAN21_ATAMA ) ) ;
#endif
	entk->sense.facedir_x = MatToXRot( &BODYWORLD( entk->act->body, HUMAN21_ATAMA ) ) ;

	/* スネーク重なりフラグ ??何用だったのだろう???*/
//	if ( entk->pl_eyei.dis < CROSS_DIS ) GM_PlayerStatus |= PLAYER_ON_CORPSE ;

	if ( act->status & ACT_STATUS_SPHERE_100 ) {
		act->ctrl->r_sphere = 100 ;
	} else if ( act->status & ACT_STATUS_SPHERE_200 ) {
		act->ctrl->r_sphere = 200 ;
	} else if ( act->status & ACT_STATUS_SPHERE_300 ) {
//		act->ctrl->r_sphere = 350 ;/*トイレ除き日本版修正*/
		act->ctrl->r_sphere = 250 ;/*トイレ除き日本版修正*/
	} else {
		act->ctrl->r_sphere = 400 ;
	}

	/* 倒れたときの低い床抜けチェック */
	if ( act->status_status & (ACT_STST_UNDER_NEARCHECK) ) {
		GM_ConfigControlNearCheck2( entk->ctrl, 195 ) ;
	}

	if ( act->status_status & ACT_STST_CHANGE_UNIQ_ID ) {
		entk->old_uniq_id = entk->uniq_id  ;
		entk->uniq_id = entk->name_id.uniq_id = COM_GetUniqID( ) ;
	}

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( entk->status_status & ENE_STST_OPTCAMOUFLAGE ) {
//			if ( Ply_GetPlayerItem() == IT_NightVision ) {
			if ( Ply_GetPlayerItem() == IT_Thermal ) {
				UNSET_FLAG ( act->body->objs->flag, DG_FLAG_OPTCMF ) ;
			} else {
				SET_FLAG ( act->body->objs->flag, DG_FLAG_OPTCMF ) ;
			}
		}
	}
}

void	ENE_Gravitation( entk )
ENETHINK	*entk ;
{
	if ( !(entk->act->ctrl->skip_flag & CTRL_SKIP_FLR_CHECK) ) {
		if ( !(entk->act->old_status & (ACT_STATUS_UNREAL|ACT_STATUS_GHOST)) ) {
			if ( entk->ctrl->grounded & 1 ) {
				entk->ctrl->step.vy = 0.0F ;
			}

         if ( BP_IsPAL()==TRUE )
			   entk->ctrl->step.vy -= (16.0F * 1.2f) ;
         else
	   		entk->ctrl->step.vy -= 16.0F ;

      } else {
			entk->ctrl->step.vy = 0.0F ;
		}
	} else {
		/* Y移動量はモーションに任せる */
		entk->ctrl->step.vy = entk->act->body->height - entk->act->old_body_height ;
	}
}


/*-----  --------------------------------------------------*/
int	ENE_ReadClearing( rnavi, area_n, route_n )
ROUTENAVI	*rnavi ;
int			area_n ;
int			route_n ;
{
	HZX_CLE_AREA	*area ;
	HZX_CLE_ROOT	*route ;
	HZX_CLE_PTP		*point ;
	int			i ;

#ifdef PRINT_DEBUG
printf("area [%d] route[%d]\n",area_n, route_n ) ;
#endif

	area = HZX_CurrentHzx->def->cle_areas ;
	area += area_n ;
	route = area->roots ;
	route += route_n ;

	rnavi->n_nodes = route->n_points ;
	if ( rnavi->n_nodes <= 0 ) return -1 ;
	point = route->points ;
#ifdef PRINT_DEBUG
printf("points [%d] \n",rnavi->n_nodes ) ;
#endif
	for ( i=0; i<rnavi->n_nodes ; i++ ) {
		rnavi->nodes[ i ].vx = (float)point->x ;
		rnavi->nodes[ i ].vy = (float)point->y ;
		rnavi->nodes[ i ].vz = (float)point->z ;
		rnavi->aimnodes[ i ].vx = (float)point->ax ;
		rnavi->aimnodes[ i ].vz = (float)point->az ;
		rnavi->aimnodes[ i ].vy = (float)point->ay ;

		rnavi->pa_action[ i ] = point->act ;
		rnavi->pa_time[ i ] = point->time ;
		rnavi->pa_dir[ i ] = point->dir ;
		rnavi->pa_con[ i ] = point->pad ;

//入れるかも、今はデータが無い		rnavi->pa_flag[ i ] = point->flag ;

#if 1
		rnavi->mapbit[ i ] = GV_GetBit(point->group_id) ;
#else

printf("data group id [%x]\n",point->group_id ) ;

		/* atode園山君対応後に処理 */
		{
			int g, z ;
			HZX_Pos2Zone( &rnavi->nodes[ i ], &g, &z );
			rnavi->mapbit[ i ] = GM_GetBit( g ) ;
		}
printf("calc group id [%x]\n",rnavi->mapbit[ i ] ) ;
#endif

#ifdef PRINT_DEBUG
printf("data group id [%d]->[%x]\n",point->group_id,rnavi->mapbit[ i ] ) ;

printf("act[%d] time[%d] dir[%d] con[%d]\n",
		rnavi->pa_action[i],rnavi->pa_time[i],rnavi->pa_dir[i],rnavi->pa_con[i]);
printf("n[%f][%f][%f]\n",rnavi->nodes[ i ].vx,rnavi->nodes[ i ].vy,rnavi->nodes[ i ].vz);
printf("a[%f][%f][%f]\n",rnavi->aimnodes[ i ].vx,rnavi->aimnodes[ i ].vy,rnavi->aimnodes[ i ].vz);
#endif
		point ++ ;
	}
	rnavi->c_route = route_n ;
	rnavi->next_node = 0 ;
	
	return	0 ;
}

int	ENE_ReadNodes( rnavi, route )
ROUTENAVI	*rnavi ;
int			route ;
{
	HZX_PAT		*patrols ;
	HZX_PTP		*point ;
	int			i ;

#ifndef KP_WINDOWS
printf("kokode: route[%d]\n",route);
printf("otirunoha:current hzx[%x]\n",HZX_CurrentHzx);
printf("nai route wo:def[%x]\n",HZX_CurrentHzx->def);
printf("sitei siterukara!!:n_patrols[%d]\n",HZX_CurrentHzx->def->n_patrols);
	ASSERT( route < HZX_CurrentHzx->def->n_patrols ) ;
#else
	if( route >= HZX_CurrentHzx->def->n_patrols )
	{
		printf("kokode: route[%d]\n",route);
		printf("otirunoha:current hzx[%x]\n",HZX_CurrentHzx);
		printf("nai route wo:def[%x]\n",HZX_CurrentHzx->def);
		printf("sitei siterukara!!:n_patrols[%d]\n",HZX_CurrentHzx->def->n_patrols);

		dbgErrMessPuts("kokode otirunoha nai route wo sitei siterukara!!?", S_OK) ;
		return(-1) ;
	}
#endif
	patrols = HZX_CurrentHzx->def->patrols ;
	patrols += route ;

	rnavi->n_nodes = patrols->n_points ;
	if ( rnavi->n_nodes <= 0 ) return -1 ;
	point = patrols->points ;
printf("root points[%d]\n",rnavi->n_nodes);
	for ( i=0; i<rnavi->n_nodes ; i++ ) {
		rnavi->nodes[ i ].vx = (float)point->x ;
		rnavi->nodes[ i ].vy = (float)point->y ;
		rnavi->nodes[ i ].vz = (float)point->z ;
#if 0
if ( route == 2 ){
	rnavi->nodes[ i ].vx = 6500.0F ;
	rnavi->nodes[ i ].vy = -8000.0F ;
	rnavi->nodes[ i ].vz = -15000.0F ;
}
#endif

#ifdef PRINT_DEBUG
printf("n[%d] n[%f][%f][%f]\n",i, rnavi->nodes[ i ].vx,rnavi->nodes[ i ].vy,rnavi->nodes[ i ].vz);
#endif

		rnavi->aimnodes[ i ].vx = (float)point->ax ;
		rnavi->aimnodes[ i ].vy = (float)point->ay ;
		rnavi->aimnodes[ i ].vz = (float)point->az ;
		rnavi->mapbit[ i ] = GV_GetBit(point->group_id) ;

#ifdef PRINT_DEBUG
printf("a[%f][%f][%f] ",rnavi->aimnodes[i].vx,rnavi->aimnodes[i].vy,rnavi->aimnodes[i].vz);
printf("grp_id[%d]->mapbit[%x]\n",point->group_id, rnavi->mapbit[ i ]);
#endif

		rnavi->pa_action[ i ] = point->act ;
		rnavi->pa_time[ i ] = point->time ;
		rnavi->pa_dir[ i ] = point->dir ;
		rnavi->pa_con[ i ] = point->pad ;
		rnavi->pa_flag[ i ] = point->flag ;


#ifdef PRINT_DEBUG
printf("n[%d] act[%d] time[%d] dir[%d] con[%d] flag[%x]\n",
	i,rnavi->pa_action[i],rnavi->pa_time[i],rnavi->pa_dir[i],rnavi->pa_con[i],rnavi->pa_flag[i]);
#endif
		point ++ ;
	}
	rnavi->c_route = route ;
	rnavi->next_node = 0 ;

	return	0 ;
}

void	ENE_InitTargPoint( trgp )
TRGPOINT	*trgp ;
{
	trgp->pos = DG_ZeroVector ;
	trgp->dir = -1 ;
	trgp->addr = -1 ;
	trgp->map = GM_CurrentMap ;
}

void	ENE_InitZoneNavi( znavi )
ZONENAVI	*znavi ;
{
	znavi->next_zonepos = DG_ZeroVector ;
	znavi->going_addr = znavi->next_addr = znavi->this_addr = -1 ;
}

void ENE_InitRouteNavi( rnavi, route, node )
ROUTENAVI	*rnavi ;
int			route ;
int			node ;
{
	rnavi->p_acttime = 0 ;
	rnavi->c_route = route ;
	rnavi->next_route = route ;

	ENE_ReadNodes( rnavi, route ) ;
	if ( node > rnavi->n_nodes - 1 ) {
		node = rnavi->n_nodes - 1 ;
	}
	rnavi->next_node = node ;
}

void ENE_InitRoute( rnavi )
ROUTENAVI	*rnavi ;
{
}

void	ENE_InitEneThink( etk, ctrl, rnavi, znavi, act )
ENETHINK	*etk ;
CONTROL		*ctrl ;
ROUTENAVI	*rnavi ;
ZONENAVI	*znavi ;
ACTION		*act ;
{
	etk->ctrl = ctrl ;
	etk->rnavi = rnavi ;
	etk->znavi = znavi ;
	etk->act = act ;

	etk->bullet = 0 ;

	etk->think1 = 0 ;
	etk->think2 = 0 ;
	etk->think3 = 0 ;
	etk->count3 = 0 ;

	etk->alert = 0 ;
	etk->avoid = 0 ;

	etk->seethrough_time = 0 ;

	etk->lockon = 0 ; /* VRロックオンカーソル制御*/
}

void	ENE_SetSenseParam( sens_p, facedir, eye_r, eye_s, hearing, smell )
SENSEPARAM	*sens_p ;
short		facedir ;	/* 顔の方向 */
short	 	eye_r ; 	/* 顔の向きを中心とした視野角度 */
int 		eye_s ;		/* 視力 */
int			hearing ;	/* 聴力 */
int			smell ;		/* 嗅覚 */
{
	sens_p->facedir = facedir ;
	sens_p->eye_r = eye_r ;
	sens_p->eye_s = eye_s ;
	sens_p->hearing = hearing ;
	sens_p->smell = smell ;
	sens_p->status = RADAR_COLOR_BLUE ;
}

void	ENE_SetEyeInfo( eyei, pos, addr, status, mapbit )
EYEINFO	*eyei ;
FVECTOR	*pos ;
int		*addr ;
int		*status ;
int		*mapbit ;
{
	eyei->pos = pos ;
	eyei->addr = addr ;
	eyei->status = status ;
	eyei->map = mapbit ;
	eyei->trgflag = 0 ;
	eyei->sight = 0 ;
	eyei->flag = 0 ;
}

void	ENE_InitControl( entk, ctrl, name )
ENETHINK	*entk ;
CONTROL	*ctrl ;
int		name ;
{
	extern void GM_InitRadarControl( RADAR_CTRL *, FVECTOR *, int , int  );
	extern void GM_RadarSetVRange( RADAR_CTRL *, float , float );

printf("enemy name = %d \n",name) ;

	GM_InitControl( ctrl, name, 0 ) ;
	ctrl->hzx_height = 750 ;
	ctrl->height = 1049.0F ;
	GM_ConfigControlHazard( ctrl, 1200, 450, 500 ) ;
	GM_ConfigControlTrapCheck( ctrl ) ;
	GM_ConfigControlMessageCheck( ctrl ) ;

	GM_ConfigControlAddressCheck( ctrl ) ;
	GM_ConfigControlMapCheck( ctrl ) ;

    ctrl->seg_flag |= HZX_TYPE_ENEMY ;
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;

	ctrl->flr_flag |= HZX_FLOOR_PITFALL ;
	GM_ConfigControlAttribute( ctrl, CTRL_ATR_PITFALLCHECK ) ;

	entk->ctrl = ctrl ;

	GM_InitRadarControl( &entk->sense.rctrl, &entk->ctrl->mov, RADAR_VISIBLE|RADAR_SIGHT, -1 );
	GM_RadarSetVRange( &entk->sense.rctrl, RADAR_VRANGE_UPPER , RADAR_VRANGE_LOWER );

	GM_SetZoneIntrpt( &entk->z_intrpt, ctrl, 0, 0, ZONE_INTRPT_ENEMY ) ;
	GM_PutZoneIntrpt( &entk->z_intrpt ) ;
}

void	ENE_SetMotionName( ENETHINK *entk, int base, int stage )
{
	entk->name_id.motion = base ;
	entk->name_id.mot_stage = stage ;
}

void	ENE_InitObject( entk, body, data, ctrl, lights )
ENETHINK	*entk ;
OBJECT		*body ;
int			data ;
CONTROL		*ctrl ;
FMATRIX		*lights ;
{
//	extern void *NewFaceAnim( OBJECT *, int * ) ;
//	extern void *NewMouthSys( OBJECT *human, int *sw ) ;
	extern void *NewFingerSign( OBJECT *human ,int *sw ) ;
	extern void *NewFingerSignUrban( OBJECT *human ,int *sw ) ;
	extern void *NewFingerSignHiTech( OBJECT *human ,int *sw ) ;

	extern void *NewEyeAnim( OBJECT *, int * ) ;
	extern void *NewEyeAnimUrban( OBJECT *, int * ) ;
	extern void *StartCorpseIK(CONTROL *, OBJECT * );

	entk->lights = lights ;
	entk->name_id.body = data ;
	/* 関節型モデルの表示 */
	GM_InitObject( body, data, DG_FLAG_IRREACTION|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC | DG_FLAG_SHADOWMAKE );
	GM_ConfigObjectLight( body, lights );

	entk->name_id.motion = GV_StrCode("gbs") ;
	GM_ConfigObjectMotion( body, 3, GV_StrCode("gbs"), MT_FLAG_HUMAN2 );

	GM_ConfigObjectStep( body, &ctrl->step );
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 );

	/* モーションオーバーライド実験 */
	GM_ConfigObjectAction( body, 1, 0, 0, 0x0, 0 );
	GM_ConfigObjectAction( body, 2, 0, 0, 0x0, 0 );
//	GM_ConfigObjectAction( body, 1, 1, 0, 0x780, 0 );
//	body->m_ctrl->merge_flag = 0x0002 ;

	/* object control の関連付け */
	GM_ConfigControlObject( ctrl, body ) ;

	/* 光学迷彩 */
	if ( entk->status_status & ENE_STST_OPTCAMOUFLAGE ) {
		extern void *NewOpticalCamouflage( DG_OBJS *objs, int flag );

		GV_SetActorChild( entk->w, NewOpticalCamouflage( body->objs, 0 ) ) ;
		SET_FLAG( entk->status, ENE_STATUS_SHADOW_OFF ) ;
		SET_FLAG( entk->status, ENE_STATUS_NO_EYEANIM ) ;
		SET_FLAG( entk->status, ENE_STATUS_NO_FINGER ) ;
	}

    /* 影起動 */
	if ( !(entk->status & ENE_STATUS_SHADOW_OFF) ) {
		extern void *NewShadow( DG_OBJ *, DG_OBJ *, CONTROL *, FMATRIX *, int * ) ;
	    GV_SetActorChild( entk->w, 
	    	(entk->shadow = NewShadow( &body->objs->objs[BODY21_RTOE], &body->objs->objs[BODY21_LTOE],
	    		 ctrl, lights, &entk->sw_shadow )) ) ;
	}

	if ( entk->status_status & ENE_STST_VRBODY ) {
		SET_FLAG( entk->status, ENE_STATUS_NO_EYEANIM ) ;
		SET_FLAG( entk->status, ENE_STATUS_NO_FINGER ) ;
	}
	/* 目アニメ */
	entk->sw.eye_anim = 0 ;
	if ( !(entk->status & ENE_STATUS_NO_EYEANIM) ) {
		entk->sw.eye_anim = 1 ;
		if ( COM_StageKind() & ENE_STAGE_GPS ) {
			GV_SetActorChild( entk->w, NewEyeAnimUrban( body, &entk->sw.eye_anim ) );
		} else {
			GV_SetActorChild( entk->w, NewEyeAnim( body, &entk->sw.eye_anim ) );
		}
	}

	/* 口パク */
	entk->sw.mouth = 0 ;

#if 0
/* フラグ不足により*/
	SET_FLAG(entk->status, ENE_STATUS_NO_MOUTH ) ;	/* 現在は口パクは無し */
	if ( !(entk->status & ENE_STATUS_NO_MOUTH) ) {
		extern void *NewMouthSys( OBJECT *human, int *sw ) ;
		entk->sw.mouth = 1 ;
		GV_SetActorChild( entk->w, NewMouthSys( body, &entk->sw.mouth ) );
	}
#endif

	/* フィンガーサイン */
	entk->sw.finger = 0 ;
	if ( !(entk->status & ENE_STATUS_NO_FINGER) ) {
//		if(entk->act->bodyp.type & ENE_TYPE_HITECH){
		if(data== ENE_MDL_NAME_HTC){
			GV_SetActorChild( entk->w, NewFingerSignHiTech( body, &entk->sw.finger ) );
		}else if( entk->status & ENE_STATUS_URBAN ){
			GV_SetActorChild( entk->w, NewFingerSignUrban( body, &entk->sw.finger ) );
		}else {
			GV_SetActorChild( entk->w, NewFingerSign( body, &entk->sw.finger ) );
		}
	}

	/* 頂点アニメ */
	entk->vanime_flag = 0 ;
	entk->vanime_head = NULL ;


#if 1
/*2002.08.09 本編のみ頭アニメ*/
	if (
	(!(entk->status & ENE_STATUS_NO_VANIM) )
	&&(!(GM_VRStatus & GM_VR_ENEMY_POLY_VANISH))
	&&(!(GM_GameStatus & STATE_VR_ANOTHER))
	) {
		CV2_DEF		*cv2_def ;
		extern HUMANMA_WORK *InitHumanHeadMA(DG_OBJS *objs,CV2_DEF *def,int flag,int frame);

		cv2_def = GV_GetCache( GV_CacheID( data, 'c' ) ) ;
		entk->vanime_head = InitHumanHeadMA( body->objs, cv2_def, DG_VANIME_VERTS | DG_VANIME_NORMS, 4 ) ;
	}
#endif
	/* おしっこ */
	entk->sw.hounyou = 0 ;
//SET_FLAG( entk->status, ENE_STATUS_HOUNYOU ) ;
	if ( entk->status & ENE_STATUS_HOUNYOU ) {
		void *NewEnemyPee( OBJECT  *body, FVECTOR *target_pos, int *flag, int kind ) ;
printf(" ENE_OBJ_INIT_ENE_STATUS_HOUNYOU \n");
//ASSERT(0) ;
		
		if ( ENE_GameStatus & ENE_GMSTATUS_TOILET ) {
			GV_SetActorChild(entk->w,NewEnemyPee(body,&GM_PlayerPosition,&entk->sw.hounyou, 0 ) ) ;
		} else {
			GV_SetActorChild(entk->w,NewEnemyPee(body,&GM_PlayerPosition,&entk->sw.hounyou, 1 ) ) ;
		}
	}

#ifdef SHADOW_LOW_POLY
	{
		DG_DEF		*def ;

		def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "gbs_shadow" ), 'k' ) ) ;
		if ( def != NULL ) {
			entk->lowshadow = DG_MakeObjs( def, DG_FLAG_FINISHCALC, 0 ) ;
			DG_SetLowObjs( body->objs, entk->lowshadow ) ;
		} else {
#ifdef DEBUG_MODE
			printf("[ no problem ] Low Poly Shadow \"gbs_shadow\" not found. \n") ;
#endif
			entk->lowshadow = NULL ;
		}
	}
#else
	entk->lowshadow = NULL ;
#endif
}

/*座標の変更は eneequip.c もすること！！！*/
void	ENE_InitSling( ptr, body, weapon, lights )
void		*ptr ;
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	extern void *NewSling( OBJECT *, OBJECT *, FVECTOR *, FVECTOR *, FMATRIX *, int ) ;

	FVECTOR	sft1[8], sft2[4] ;
	
	/* スリング */
	/* 右肩 */
	sft1[0].vx = -89.28 ; sft1[0].vy = 308.22 ; sft1[0].vz = 11.59 ;
	sft1[1].vx = -112.58 ; sft1[1].vy = 282.1 ; sft1[1].vz = 11.59 ;
	/* 左横腹 */
	sft1[2].vx = 205.86 ; sft1[2].vy = 44.79 ; sft1[2].vz = 59.88 ;
	sft1[3].vx = 182.56 ; sft1[3].vy = 18.67 ; sft1[3].vz = 59.88 ;
	/* 右肩奥 */
	sft1[4].vx = -89.0 ; sft1[4].vy = 308.0 ; sft1[4].vz = -47.0 ;
	sft1[5].vx = -112.0 ; sft1[5].vy = 282.0 ; sft1[5].vz = -47.0 ;
	/* 左横腹奥 */
	sft1[6].vx = 205.0 ; sft1[6].vy = 44.0 ; sft1[6].vz = 0.0 ;
	sft1[7].vx = 182.0 ; sft1[7].vy = 18.0 ; sft1[7].vz = 0.0 ;

	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 20.8 ; sft2[2].vy = -452.0 ; sft2[2].vz = 121.4 ;
	sft2[3].vx = 51.0 ; sft2[3].vy = -452.0 ; sft2[3].vz = 103.9 ;

	GV_SetActorChild( ptr, NewSling( body, weapon, sft1, sft2, lights, GV_StrCode("gbs_sling") ) ) ;
}

/*座標の変更は eneequip.c もすること！！！*/
void	ENE_InitSlingGBA( ptr, body, weapon, lights )
void		*ptr ;
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	extern void *NewSling( OBJECT *, OBJECT *, FVECTOR *, FVECTOR *, FMATRIX *, int ) ;

	FVECTOR	sft1[8], sft2[4] ;
/*
頂点座標  上首側(-115 317 59)
          上肩側(-141 293 59)
          下腕側(226 -45 94)
          下胴側(201 -73 94)
*/
	
	/* スリング */
	/* 右肩 */
	sft1[0].vx = -115.0 ; sft1[0].vy = 317.0 ; sft1[0].vz = 59.0 ;
	sft1[1].vx = -141.0 ; sft1[1].vy = 293.0 ; sft1[1].vz = 59.0 ;
	/* 左横腹 */
	sft1[2].vx = 226.0 ; sft1[2].vy = -45.0 ; sft1[2].vz = 94.0 ;
	sft1[3].vx = 201.0 ; sft1[3].vy = -73.0 ; sft1[3].vz = 94.0 ;
	/* 右肩奥 */
	sft1[4].vx = -129.0 ; sft1[4].vy = 333.0 ; sft1[4].vz = -10.0 ;
	sft1[5].vx = -155.0 ; sft1[5].vy = 308.0 ; sft1[5].vz = -10.0 ;
	/* 左横腹奥 */
	sft1[6].vx = 233.0 ; sft1[6].vy = -53.0 ; sft1[6].vz = 37.0 ;
	sft1[7].vx = 208.0 ; sft1[7].vy = -80.0 ; sft1[7].vz = 37.0 ;

	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 20.8 ; sft2[2].vy = -452.0 ; sft2[2].vz = 121.4 ;
	sft2[3].vx = 51.0 ; sft2[3].vy = -452.0 ; sft2[3].vz = 103.9 ;

	GV_SetActorChild( ptr, NewSling( body, weapon, sft1, sft2, lights, GV_StrCode("gba_sling")  ) ) ;
}

/* 座標はGBSと同じ */
/*座標の変更は eneequip.c もすること！！！*/
void	ENE_InitSlingGPS( ptr, body, weapon, lights )
void		*ptr ;
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	extern void *NewSling( OBJECT *, OBJECT *, FVECTOR *, FVECTOR *, FMATRIX *, int ) ;

	FVECTOR	sft1[8], sft2[4] ;
	
	/* スリング */
	/* 右肩 */
	sft1[0].vx = -89.28 ; sft1[0].vy = 308.22 ; sft1[0].vz = 11.59 ;
	sft1[1].vx = -112.58 ; sft1[1].vy = 282.1 ; sft1[1].vz = 11.59 ;
	/* 左横腹 */
	sft1[2].vx = 205.86 ; sft1[2].vy = 44.79 ; sft1[2].vz = 59.88 ;
	sft1[3].vx = 182.56 ; sft1[3].vy = 18.67 ; sft1[3].vz = 59.88 ;
	/* 右肩奥 */
	sft1[4].vx = -89.0 ; sft1[4].vy = 308.0 ; sft1[4].vz = -47.0 ;
	sft1[5].vx = -112.0 ; sft1[5].vy = 282.0 ; sft1[5].vz = -47.0 ;
	/* 左横腹奥 */
	sft1[6].vx = 205.0 ; sft1[6].vy = 44.0 ; sft1[6].vz = 0.0 ;
	sft1[7].vx = 182.0 ; sft1[7].vy = 18.0 ; sft1[7].vz = 0.0 ;

/*
アバカンのスリング位置です。
A(後上):39.5, -55.5, 72.6
B(後下):39.5, -55.5, 37.6
C(前上):41.5, -492, 95.5
D(前下):41.5, -492, 57.5
*/
	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 41.5 ; sft2[2].vy = -492.0 ; sft2[2].vz = 95.4 ;
	sft2[3].vx = 41.5 ; sft2[3].vy = -492.0 ; sft2[3].vz = 57.9 ;

	GV_SetActorChild( ptr, NewSling( body, weapon, sft1, sft2, lights, GV_StrCode("gps_sling") ) ) ;
}

/*座標の変更は eneequip.c もすること！！！*/
void	ENE_InitSlingGPA( ptr, body, weapon, lights )
void		*ptr ;
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	extern void *NewSling( OBJECT *, OBJECT *, FVECTOR *, FVECTOR *, FMATRIX *, int ) ;

	FVECTOR	sft1[8], sft2[4] ;
/*
頂点座標  上首側(-86 312 59)
          上肩側(-115 293 59)
          下腕側(221 27 49)
          下胴側(188 0 49)
*/
	
	/* スリング */
	/* 右肩 */
	sft1[0].vx = -86.0 ; sft1[0].vy = 312.0 ; sft1[0].vz = 59.0 ;
	sft1[1].vx = -115.0 ; sft1[1].vy = 293.0 ; sft1[1].vz = 59.0 ;
	/* 左横腹 */
	sft1[2].vx = 211.0 ; sft1[2].vy = 27.0 ; sft1[2].vz = 49.0 ;
	sft1[3].vx = 188.0 ; sft1[3].vy = 0.0 ; sft1[3].vz = 49.0 ;
	/* 右肩奥 */
	sft1[4].vx = -97.0 ; sft1[4].vy = 327.0 ; sft1[4].vz = -15.0 ;
	sft1[5].vx = -127.0 ; sft1[5].vy = 309.0 ; sft1[5].vz = -15.0 ;
	/* 左横腹奥 */
	sft1[6].vx = 219.0 ; sft1[6].vy = 27.0 ; sft1[6].vz = -10.0 ;
	sft1[7].vx = 194.0 ; sft1[7].vy = 0.0 ; sft1[7].vz = -10.0 ;

	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 41.5 ; sft2[2].vy = -492.0 ; sft2[2].vz = 95.4 ;
	sft2[3].vx = 41.5 ; sft2[3].vy = -492.0 ; sft2[3].vz = 57.9 ;

	GV_SetActorChild( ptr, NewSling( body, weapon, sft1, sft2, lights, GV_StrCode("gpa_sling")  ) ) ;
}
void	ENE_InitSlingGPA_SPS( ptr, body, weapon, lights )
void		*ptr ;
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	extern void *NewSling( OBJECT *, OBJECT *, FVECTOR *, FVECTOR *, FMATRIX *, int ) ;

	FVECTOR	sft1[8], sft2[4] ;
/*
頂点座標  上首側(-86 312 59)
          上肩側(-115 293 59)
          下腕側(221 27 49)
          下胴側(188 0 49)
*/
	
	/* スリング */
	/* 右肩 */
	sft1[0].vx = -86.0 ; sft1[0].vy = 312.0 ; sft1[0].vz = 59.0 ;
	sft1[1].vx = -115.0 ; sft1[1].vy = 293.0 ; sft1[1].vz = 59.0 ;
	/* 左横腹 */
	sft1[2].vx = 211.0 ; sft1[2].vy = 27.0 ; sft1[2].vz = 49.0 ;
	sft1[3].vx = 188.0 ; sft1[3].vy = 0.0 ; sft1[3].vz = 49.0 ;
	/* 右肩奥 */
	sft1[4].vx = -97.0 ; sft1[4].vy = 327.0 ; sft1[4].vz = -15.0 ;
	sft1[5].vx = -127.0 ; sft1[5].vy = 309.0 ; sft1[5].vz = -15.0 ;
	/* 左横腹奥 */
	sft1[6].vx = 219.0 ; sft1[6].vy = 27.0 ; sft1[6].vz = -10.0 ;
	sft1[7].vx = 194.0 ; sft1[7].vy = 0.0 ; sft1[7].vz = -10.0 ;

	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
//	sft2[2].vx = 41.5 ; sft2[2].vy = -492.0 ; sft2[2].vz = 95.4 ;
//	sft2[3].vx = 41.5 ; sft2[3].vy = -492.0 ; sft2[3].vz = 57.9 ;
	sft2[2].vx = 41.5 ; sft2[2].vy = -392.0 ; sft2[2].vz = 95.4 ;
	sft2[3].vx = 41.5 ; sft2[3].vy = -392.0 ; sft2[3].vz = 57.9 ;

	GV_SetActorChild( ptr, NewSling( body, weapon, sft1, sft2, lights, GV_StrCode("gpa_sling")  ) ) ;
}

static int sw_test ;
void	ENE_TestCali( body, n )
OBJECT	*body ;
int n ;
{
	extern void *NewCalibur( FMATRIX *, FVECTOR *, FVECTOR *, int * ) ;
	FVECTOR	sft[2] ;

	sft[0].vx = 0.0 ; sft[0].vy = 300.0 ; sft[0].vz = 0.0 ;
	sft[1].vx = 0.0 ; sft[1].vy = -300.0 ; sft[1].vz = 0.0 ;
	sw_test = 1 ;
//	NewCalibur( &body->objs->objs[n].world, &sft[0], &sft[1], &sw_test ) ;

}

void	ENE_InitSubWeapon( entk, body, weapon, data )
ENETHINK	*entk ;
OBJECT		*body ;
OBJECT		*weapon ;
int			data ;
{
	extern int WeaponEfInitObject ( 
		 WEAPON_EF_CTRL *,					/* 呼ぶ側のworkにとった管理構造体のポインタ */
		 OBJECT *,							/* 初期化するオブジェクトのポインタ */
		 OBJECT *,							/* rootオブジェクトポインタ */
		 int ,								/* ユニット番号 */
		 int ,								/* 武器種類 */
		 int								/* その武器のモデルコード */
		 );

	entk->name_id.sub_weapon = data ;
	entk->sub_weapon = weapon ;
//	if ( data == 0 || weapon == NULL ) {
	if ( weapon == NULL ) {
		entk->sw.sub_weapon = 0 ;
	} else  {	/* マカロフ */
		extern void DG_ConnectObjs( DG_OBJS *, DG_OBJS * ) ;
//
		entk->sw.sub_weapon = 1 ;
		WeaponEfInitObject( &(entk->sw.wctrl), weapon, body, 6, WP_Mkr, data ) ;
		DG_ConnectObjs( body->objs, weapon->objs ) ;
	}
}

/* 装備品 */
#include "../../kano/attachment/attachments.h"


extern int MakeAttachment_called(int model_name,FVECTOR *v,SVECTOR *r,OBJECT *target,int objnum,FVECTOR *x,int *a,int angle_limit,float oval_param);
extern int MakeAttachment2_called(int model_name,FVECTOR *v,SVECTOR *r,OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p);
extern int MakeAttachment3_called(int model_name,SVECTOR *r,OBJECT *target,int objnum,FVECTOR *x,int frames);

void *ENE_SetAttachment( void *ptr, OBJECT *body )
{
	void *res ;
	static const FVECTOR shift[]={
	    { -185.0f,  27.5f,  61.5f,   0.0f, },
	    {  -67.5f,  -2.5f, 152.5f,   0.0f, },
	    {  120.0f,  22.5f, -60.0f,   0.0f, },
	};
	static const FVECTOR shift2[]={
	    {  -65.0f, -10.0f,  42.5f,   0.0f, },
	    {   32.5f,-140.0f, 112.5f,   0.0f, },
	    {   20.0f, -92.5f,-117.5f,   0.0f, },
	};
	static const FVECTOR shift3[]={
	    {    0.0f, -40.0f,-140.0f,   0.0f, },
	};
#if 1
	static const ATTACHMENT_ARGUMENT2 arg2[]={
	    {
		0x291040 /* GV_StrCode("gbs_hlst") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)(shift+0),
		13,
		(FVECTOR *)(shift2+0),
	    },
	    {
		0x2a96f2 /* GV_StrCode("gbs_knif") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)(shift+1),
		13,
		(FVECTOR *)(shift2+1),
	    },
	    {
		0x615b12 /* GV_StrCode("gbs_mag") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)(shift+2),
		17,
		(FVECTOR *)(shift2+2),
	    },
	};
	static const ATTACHMENT_ARGUMENT3 arg3[]={
	    {
		0x5b0984 /* GV_StrCode("gbs_bp") */ ,
		NULL,
		2,
		(FVECTOR *)(shift3+0),
		2,
	    },
	};

	GV_SetActorChild( ptr, res = NewAttachments_called(body,NULL,0,arg2,3,arg3,1) ) ;
	return res ;
#else
	MakeAttachment2_called(0x291040,NULL,NULL,body,0,(FVECTOR *)(shift+0),13,(FVECTOR *)(shift2+0));
	MakeAttachment2_called(0x2a96f2,NULL,NULL,body,0,(FVECTOR *)(shift+1),13,(FVECTOR *)(shift2+1));
	MakeAttachment2_called(0x615b12,NULL,NULL,body,0,(FVECTOR *)(shift+2),17,(FVECTOR *)(shift2+2));
	MakeAttachment3_called(0x5b0984,NULL,body,2,(FVECTOR *)(shift3+0),2);
#endif

}

void *ENE_SetAttachmentPLT( void *ptr, OBJECT *body )
{
	void *res ;
	static const FVECTOR shift[]={
	    { -185.0f,  27.5f,  61.5f,   0.0f, },
	    {  -67.5f,  -2.5f, 152.5f,   0.0f, },
	    {  120.0f,  22.5f, -60.0f,   0.0f, },
	};
	static const FVECTOR shift2[]={
	    {  -65.0f, -10.0f,  42.5f,   0.0f, },
	    {   32.5f,-140.0f, 112.5f,   0.0f, },
	    {   20.0f, -92.5f,-117.5f,   0.0f, },
	};
#if 1
	static const ATTACHMENT_ARGUMENT2 arg2[]={
	    {
		0x291040 /* GV_StrCode("gbs_hlst") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)(shift+0),
		13,
		(FVECTOR *)(shift2+0),
	    },
	    {
		0x2a96f2 /* GV_StrCode("gbs_knif") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)(shift+1),
		13,
		(FVECTOR *)(shift2+1),
	    },
	    {
		0x615b12 /* GV_StrCode("gbs_mag") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)(shift+2),
		17,
		(FVECTOR *)(shift2+2),
	    },
	};
	GV_SetActorChild( ptr, res = NewAttachments_called(body,NULL,0,arg2,3,NULL,0) ) ;
	return res ;
#else 
	MakeAttachment2_called(0x291040,NULL,NULL,body,0,(FVECTOR *)(shift+0),13,(FVECTOR *)(shift2+0)) ;
	MakeAttachment2_called(0x2a96f2,NULL,NULL,body,0,(FVECTOR *)(shift+1),13,(FVECTOR *)(shift2+1)) ;
	MakeAttachment2_called(0x615b12,NULL,NULL,body,0,(FVECTOR *)(shift+2),17,(FVECTOR *)(shift2+2)) ;
#endif
}

void *ENE_SetAttachmentGPS( void *ptr, OBJECT *body )
{
	void *res ;
	static const FVECTOR shift[]={
	    { -185.0f,  27.5f,  61.5f,   0.0f, },
	    {  -67.5f,  -2.5f, 152.5f,   0.0f, },
	    {  140.0f,  37.0f, -49.0f,   0.0f, },
	};
	static const FVECTOR shift2[]={
	    {  -65.0f, -10.0f,  42.5f,   0.0f, },
	    {   32.5f,-140.0f, 112.5f,   0.0f, },
	    {   20.0f, -92.5f,-117.5f,   0.0f, },
	};

	static const SVECTOR rot[]={
	    {  -68, 0, 0, 0, },
	} ;

#if 1
	static const ATTACHMENT_ARGUMENT2 arg2[]={
	    {
		0x291040 /* GV_StrCode("gbs_hlst") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)(shift+0),
		13,
		(FVECTOR *)(shift2+0),
	    },
	    {
		5459626 /* GV_StrCode("gps_knife") */ ,
		NULL,
		(SVECTOR *)(rot+0),
		0,
		(FVECTOR *)(shift+1),
		13,
		(FVECTOR *)(shift2+1),
	    },
	    {
		0x615b12 /* GV_StrCode("gbs_mag") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)(shift+2),
		17,
		(FVECTOR *)(shift2+2),
	    },
	};
	GV_SetActorChild( ptr, res = NewAttachments_called(body,NULL,0,arg2,3,NULL,0) ) ;
	return res ;
#else 
	MakeAttachment2_called(0x291040,NULL,NULL,body,0,(FVECTOR *)(shift+0),13,(FVECTOR *)(shift2+0)) ;
	MakeAttachment2_called(5459626,NULL,(SVECTOR *)(rot+0),body,0,(FVECTOR *)(shift+1),13,(FVECTOR *)(shift2+1));
	MakeAttachment2_called(0x615b12,NULL,NULL,body,0,(FVECTOR *)(shift+2),17,(FVECTOR *)(shift2+2));
#endif
}

void *ENE_SetAttachmentGBA( void *ptr, OBJECT *body )
{
	void *res ;
	static const FVECTOR shift[]={
	    {  -152.0,  -106.0, 100.0,   0.0f, },
	};
	static const FVECTOR shift2[]={
	    {   -10.9f,-215.0f, 84.0f,   0.0f, },
	};
#if 1
	static const ATTACHMENT_ARGUMENT2 arg2[]={
	    {
		2791118 /* GV_StrCode("gba_knif") */ ,
		NULL,
		NULL,
		0,
		(FVECTOR *)shift,
		13,
		(FVECTOR *)shift2,
	    },
	};
	GV_SetActorChild( ptr, res = NewAttachments_called(body,NULL,0,arg2,1,NULL,0) ) ;
	return res ;
#else
	MakeAttachment2_called(2791118,NULL,NULL,body,0,(FVECTOR *)shift,13,(FVECTOR *)shift2);
#endif
//NewConnectObject( body, 2, shift3, NULL, 0x5b0984 ) ;
//NewConnectObject( body, 2, NULL, NULL, E_WP_AKS_SP ) ;
}


/*ショットガン装備のプラント攻撃兵*/
void *ENE_SetAttachmentGPA_SPS( void *ptr, OBJECT *body )
{
	void *res ;
	static const FVECTOR shift[]={
//	    { 170.0f,  20.0f, 70.0f, 0.0f, },
//	    { 180.0f,  20.0f, 30.0f, 0.0f, },
//	    { 170.0f,  20.0f,-10.0f, 0.0f, },
	    { 200.0f,  20.0f, 70.0f, 0.0f, },
	    { 210.0f,  20.0f, 30.0f, 0.0f, },
	    { 200.0f,  20.0f,-10.0f, 0.0f, },
	};
	static const FVECTOR shift2[]={
	    {  91.0f,-292.0f,  42.5f,   0.0f, },
	    { 101.0f,-292.0f, 2.5f,   0.0f, },
	    {  91.0f,-292.0f,-37.5f,   0.0f, },
	};

	static const SVECTOR rot[]={
	    {  0, 0, 74, 0, },
	} ;

#if 1
	static const ATTACHMENT_ARGUMENT2 arg2[]={
	    {
		MDL_SPS_AMO,
		NULL,
		(SVECTOR *)(rot+0),
		HUMAN21_KOSHI,
		(FVECTOR *)(shift+0),
		HUMAN21_HIDARI_ASHI1,
		(FVECTOR *)(shift2+0),
	    },
	    {
		MDL_SPS_AMO,
		NULL,
		(SVECTOR *)(rot+0),
		HUMAN21_KOSHI,
		(FVECTOR *)(shift+1),
		HUMAN21_HIDARI_ASHI1,
		(FVECTOR *)(shift2+1),
	    },
	    {
		MDL_SPS_AMO ,
		NULL,
		(SVECTOR *)(rot+0),
		HUMAN21_KOSHI,
		(FVECTOR *)(shift+2),
		HUMAN21_HIDARI_ASHI1,
		(FVECTOR *)(shift2+2),
	    },
	};
	GV_SetActorChild( ptr, res = NewAttachments_called(body,NULL,0,arg2,3,NULL,0) ) ;
	return res ;
#else
	MakeAttachment2_called(MDL_SPS_AMO,NULL,(SVECTOR *)(rot+0),body,HUMAN21_KOSHI,(FVECTOR *)(shift+0),HUMAN21_HIDARI_ASHI1,(FVECTOR *)(shift2+0)) ;
	MakeAttachment2_called(MDL_SPS_AMO,NULL,(SVECTOR *)(rot+0),body,HUMAN21_KOSHI,(FVECTOR *)(shift+1),HUMAN21_HIDARI_ASHI1,(FVECTOR *)(shift2+1)) ;
	MakeAttachment2_called(MDL_SPS_AMO,NULL,(SVECTOR *)(rot+0),body,HUMAN21_KOSHI,(FVECTOR *)(shift+2),HUMAN21_HIDARI_ASHI1,(FVECTOR *)(shift2+2)) ;
#endif

}

void *ENE_SetAttachmentTNGGUN( void *ptr, OBJECT *body )
{
	void *res ;
	static const FVECTOR shift3[]={
	    { 150.0f, 40.0f, 37.0f, 0.0f, },
	};
#if 1
	static const ATTACHMENT_ARGUMENT3 arg3[]={
	    {
		9531134 /* GV_StrCode("tng_magazine") */ ,
		NULL,
		0,
		(FVECTOR *)(shift3+0),
		1,
	    },
	};
	GV_SetActorChild( ptr, res = NewAttachments_called(body,NULL,0,NULL,0,arg3,1) ) ;
	return res ;
#else
//	ASSERT(MakeAttachment3_called(9531134,NULL,body,0,(FVECTOR *)(shift3+0),1) >=0) ;
	MakeAttachment3_called(9531134,NULL,body,0,(FVECTOR *)(shift3+0),1) ;
#endif

}

void *ENE_SetAttachmentTNGKATANA( void *ptr, OBJECT *body )
{
	void *res ;
	static const FVECTOR shift3[]={
	    { 164.0f, 40.0f, 6.0f, 0.0f, },
	    { 0.0f, 0.0f, 0.0f, 0.0f, },
	};
#if 1
	static const ATTACHMENT_ARGUMENT3 arg3[]={
	    {
		3067861 /* GV_StrCode("tng_saya") */ ,
		NULL,
		0,
		(FVECTOR *)(shift3+0),
		1,
	    },
	    {
		14901013 /* GV_StrCode("tng_strap") */ ,
		NULL,
		0,
		(FVECTOR *)(shift3+1),
		1,
	    },
	};
	GV_SetActorChild( ptr, res = NewAttachments_called(body,NULL,0,NULL,0,arg3,2) ) ;
	return res ;
#else
	MakeAttachment3_called(3067861,NULL,body,0,(FVECTOR *)(shift3+0),1) ;
	MakeAttachment3_called(14901013,NULL,body,0,(FVECTOR *)(shift3+1),1) ;
#endif

}

int KR_VectorInZone( FVECTOR *pos, HZX_ZONE_ADD zoneaddr, int dis, int dir )
{
	HZX_ZON	*z, *near_z, *lz ;
	FVECTOR	to_pos ;
	SVECTOR rot ;
	u_char	*nears, near ;
	int		grp_id, i ,g_no ;
	
	rot.vx = 0 ;
	rot.vy = dir ;
	rot.vz = 0 ;
	GV_DirVec3( &rot, dis, &to_pos ) ;
	_sceVu0AddVector( &to_pos, &to_pos, pos ) ;

	z = HZX_GetZoneFromAdd( zoneaddr ) ;
	grp_id = HZX_ZoneGroupID( zoneaddr ) ;

	nears = z->nears ;
	for ( i=0; i<6; i++ ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		near_z = HZX_GetZone( grp_id, near ) ;
		if ( near_z->flag & (HZX_ZONE_ZINTRPT|HZX_ZONE_INTRUDE) ) continue ;
		to_pos.vy = near_z->y ;	/* 高さはゾーンに合わす */
		if ( HZX_InsideZone( grp_id, &to_pos, near ) ) return 1 ;
	}

	/* リンクゾーンだったら */
	if ( z->flag & HZX_ZON_LINK ) {
		near = HZX_GetLinkZone( z ) ;
		g_no = HZX_GetLinkGroupNo( z ) ;
		lz = HZX_GetZoneNo( g_no, near ) ;
		if ( lz->flag & (HZX_ZONE_ZINTRPT|HZX_ZONE_INTRUDE) ) return 0 ;
		to_pos.vy = lz->y ;	/* 高さはゾーンに合わす */
		if ( HZX_InsideZone( GV_GetBit(g_no), &to_pos, near ) ) return 1 ;
	}
	return 0 ;
}


void	ENE_ResetBody( ENETHINK *entk )
{

    GM_ActMotion( entk->act->body ) ;
	entk->ctrl->height = entk->act->body->height ;
	GM_ActControl( entk->ctrl ) ;
	GM_ActObject2( entk->act->body );
	DG_GetLightMatrix( &entk->ctrl->mov, entk->lights );
}

void	ENE_ResetWeaponPosition( ENETHINK *entk )
{
	extern void	EneSlingResetPosition( OBJECT *body, OBJECT *weapon, int flag, int wp ) ;

	ENE_ResetBody( entk ) ;
	EneSlingResetPosition( entk->act->body, entk->weapon, entk->sw_gun, entk->name_id.weapon ) ;
}

void	ENE_InitWeapon( entk, body, weapon, data, trg )
ENETHINK	*entk ;
OBJECT		*body ;
OBJECT		*weapon ;
int			data ;
TARGET		*trg ;
{
	extern void *NewCircleLight( FMATRIX *, int *, int, int * ) ;
	extern void *NewSlingGun2( OBJECT *, OBJECT *, int, int *,  WEAPON_EF_CTRL * ) ;
	extern void *NewCartridgeControl( int *, OBJECT *, OBJECT *, int );
	extern void	*NewRadio( OBJECT *, int, int *, TARGET * ) ;
	extern void	*NewNightSight( OBJECT *, int, int, int * ) ;
	extern int WeaponEfInitObject ( WEAPON_EF_CTRL *, OBJECT *, OBJECT *, int , int , int );
	extern void NewBodyHitecChalkerSling( void *ptr, OBJECT *, OBJECT *weapon, FMATRIX *lights ) ;

	entk->name_id.weapon = data ;
	entk->weapon = weapon ;

	entk->sw.magg = 1 ;
	entk->sw.radio = SW_FLAG_VISIBLE ;
	entk->sw.sub_weapon = 0 ;
	entk->sw.bullet = &entk->bullet ;
	SET_FLAG( entk->sw_gun, SW_FLAG_VISIBLE ) ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( entk->status & ENE_STATUS_DEAD ) {	/* 死体 */
			SET_FLAG( entk->sw_gun, SW_FLAG_SWITCH2 ) ;	/* 死体のため始めは胸に出す */
		}
	}  else {
		SET_FLAG( entk->sw_gun, SW_FLAG_SWITCH2 ) ;	/* 死体のため始めは胸に出す */
	}

	if ( entk->status & ENE_STATUS_DEAD ) {	/* 死体 */
		SET_FLAG( entk->sw_gun, SW_FLAG_SWITCH6 ) ;	/* 死体が起動したのを知らせるため */
	}


	GV_SetActorChild( entk->w, NewSlingGun2( body, weapon, data, &entk->sw_gun, &(entk->sw.wctrl2) ) ) ;
	UNSET_FLAG( entk->sw_gun, SW_FLAG_SWITCH2 ) ;
	UNSET_FLAG( entk->sw_gun, SW_FLAG_SWITCH6 ) ;

	switch( entk->name_id.body ) {
		case ENE_MDL_NAME_GBA :
			ENE_InitSlingGBA( entk->w, body, weapon, entk->lights ) ;
		break ;
		case ENE_MDL_NAME_TNG :
		case ENE_MDL_NAME_VR :
		case ENE_MDL_NAME_VR_ATK :
		break ;
		case ENE_MDL_NAME_HTC :
			NewBodyHitecChalkerSling( entk->w, body, weapon, entk->lights ) ;
		break ;
		case ENE_MDL_NAME_GPS :
			ENE_InitSlingGPS( entk->w, body, weapon, entk->lights ) ;
		break ;
		case ENE_MDL_NAME_GPA :
			if(data == E_WP_SPS){
				ENE_InitSlingGPA_SPS( entk->w, body, weapon, entk->lights ) ;
			}else {
				ENE_InitSlingGPA( entk->w, body, weapon, entk->lights ) ;
			}
		break ;
		default :
			ENE_InitSling( entk->w, body, weapon, entk->lights ) ;
		break ;
	}

//(entk->act->bodyp.type & ENE_TYPE_SHOTGUN)

	switch ( data ) {
		case E_WP_AKS :
			GV_SetActorChild( entk->w, 
				NewCircleLight( &weapon->objs->world, &entk->sw_light, 1, &entk->ctrl->hzx_id ) ) ;
			if ( !(entk->status_status & ENE_STST_OPTCAMOUFLAGE) ) {
				GV_SetActorChild( entk->w, NewCartridgeControl( &entk->sw.magg, body, weapon, 0 ) ) ;
			}
			break ;
		case E_WP_AKS_SP :
			GV_SetActorChild( entk->w, 
				NewCircleLight( &weapon->objs->world, &entk->sw_light, 0, &entk->ctrl->hzx_id ) ) ;
			if ( !(entk->status_status & ENE_STST_OPTCAMOUFLAGE) ) {
				GV_SetActorChild( entk->w, NewCartridgeControl( &entk->sw.magg, body, weapon, 0 ) ) ;
			}
			break ;
		case E_WP_ABAKAN :
			GV_SetActorChild( entk->w, 
				NewCircleLight( &weapon->objs->world, &entk->sw_light, 5, &entk->ctrl->hzx_id ) ) ;
			if ( !(entk->status_status & ENE_STST_OPTCAMOUFLAGE) ) {
				GV_SetActorChild( entk->w, NewCartridgeControl( &entk->sw.magg, body, weapon, 6 ) ) ;
			}
			break ;
		case E_WP_SPS :
			GV_SetActorChild( entk->w, 
				NewCircleLight( &weapon->objs->world, &entk->sw_light, 7, &entk->ctrl->hzx_id ) ) ;
			GV_SetActorChild( entk->w, NewCartridgeControl( &entk->sw.magg, body, weapon, 11 ) ) ;
			break ;
		case E_WP_P90 :
			GV_SetActorChild( entk->w, NewCartridgeControl( &entk->sw.magg, body, weapon, 3 ) ) ;
			break ;
		case E_WP_M4_NM :
			GV_SetActorChild( entk->w, 
				NewCircleLight( &weapon->objs->world, &entk->sw_light, 6, &entk->ctrl->hzx_id ) ) ;
			GV_SetActorChild( entk->w, NewCartridgeControl( &entk->sw.magg, body, weapon, 9 ) ) ;
			break ;
	}
	if ( (entk->status & ENE_STATUS_GUNLIGHT) ) {
		entk->sw_light = 1 ;
	} else {
		entk->sw_light = 0 ;
	}

	switch( entk->name_id.body ) {
		case ENE_MDL_NAME_GPA :
		break ;
		case ENE_MDL_NAME_GBA :
		break ;
		case ENE_MDL_NAME_TNG :
		break ;
		case ENE_MDL_NAME_HTC :
		break ;
		case ENE_MDL_NAME_VR_ATK :
		break ;
		default :
			GV_SetActorChild( entk->w, NewRadio( body, GV_StrCode("rad"), &entk->sw.radio, trg ) ) ;
		break ;
	}

	entk->sw.n_sight = 0 ;
	if ( entk->status & (ENE_STATUS_NIGHT_SIGHT+ENE_STATUS_NIGHT_SIGHT2) ) {
		GV_SetActorChild( entk->w, 
			NewNightSight( body, GV_StrCode("gbs_nvg2"), GV_StrCode("gbs_nvg1"), &entk->sw.n_sight ) ) ;
		entk->sw.n_sight = (entk->status & ENE_STATUS_NIGHT_SIGHT)?1:2 ;
	}

	/* 体影 */
	if ( entk->status & ENE_STATUS_BODYSHADOW ) {
		extern void *NewBodyShadow( DG_OBJS * );

		GV_SetActorChild( entk->w, NewBodyShadow( body->objs ) ) ;
	}
	/* 体水飛沫 */
	if ( entk->status & ENE_STATUS_BODYSPLASH ) {
		extern void *NewBodySplash( DG_OBJS *, CONTROL *, int ) ;
//		extern void *NewFootSplash( OBJECT *body, CONTROL *control ) ;
		extern void *NewFootSplash_AddedFlag( OBJECT *body, CONTROL *control, int *flag ) ;

		GV_SetActorChild( entk->w, NewBodySplash( body->objs, entk->ctrl, entk->name_id.body ) ) ;
//		GV_SetActorChild( entk->w, NewFootSplash( body, entk->ctrl ) );
		entk->sw.splash = 1 ;
		GV_SetActorChild( entk->w, NewFootSplash_AddedFlag( body, entk->ctrl, &entk->sw.splash ) );
	}

	switch( entk->name_id.body ) {
		case ENE_MDL_NAME_GBA :
			if ( !(entk->status_status & ENE_STST_OPTCAMOUFLAGE) ) {
				entk->attachment = ENE_SetAttachmentGBA( (void *)entk->w, body ) ;
			}
		break ;
		case ENE_MDL_NAME_TNG :
			if ( data == E_WP_P90 ) {
				entk->attachment = ENE_SetAttachmentTNGGUN( (void *)entk->w, body ) ;
			} else {
				entk->attachment = ENE_SetAttachmentTNGKATANA( (void *)entk->w, body ) ;
			}
		break ;
		case ENE_MDL_NAME_HTC :
//			entk->attachment = ENE_SetAttachmentGBA( (void *)entk->w, body ) ;
		break ;
		case ENE_MDL_NAME_GPS :
			if ( !(entk->status_status & ENE_STST_OPTCAMOUFLAGE) ) {
				entk->attachment = ENE_SetAttachmentGPS( (void *)entk->w, body ) ;
			}
		break ;
		case ENE_MDL_NAME_GPA :
		case ENE_MDL_NAME_VR :
		case ENE_MDL_NAME_VR_ATK :
		break ;
		default :
			if ( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
				if ( !(entk->status_status & ENE_STST_OPTCAMOUFLAGE) ) {
					entk->attachment = ENE_SetAttachment( (void *)entk->w, body ) ;
				}
			} else {
				if ( !(entk->status_status & ENE_STST_OPTCAMOUFLAGE) ) {
					entk->attachment = ENE_SetAttachmentPLT( (void *)entk->w, body ) ;
				}
			}
		break ;
	}

	entk->max_bullet = ENE_GetAmmoMax( data ) ;
//NewConnectObject( body, HUMAN21_MUNE, NULL, NULL, GV_StrCode("dbg_dogtag" ) ) ;
}

void ENE_InitAction( entk, act, ctrl, body, life, faint, blood ,mt_array)
ENETHINK	*entk ;
ACTION	*act ;
CONTROL	*ctrl ;
OBJECT	*body ;
int		life ;
int		faint ;
int		blood ;
int		*mt_array ;
{
	extern void *StartCorpseIK(CONTROL *, OBJECT * );
	extern void *NewOozeBlood( DG_OBJS *, int ) ;
	extern void *NewCreateDogtag (OBJECT* obj, int model_name, FVECTOR* shift, int* flag);
	extern int SIG_CheckDogTagFlag( int ) ;
	extern void *NewDogtagFlash( OBJECT *object, int *flag ) ;


	int		act_num, i ;

	AT_InitAction( act, ctrl, body ) ;
	act->ene_status = &entk->status ;
	act->thk_status = &entk->thk_status ;
	act->name_id = &entk->name_id ;
	act->sw = &entk->sw ;
	act->lights = entk->lights ;
	AT_SetBodyParam( &(act->bodyp), life, faint, blood ) ;

#if 0 
/*Sigeno retouched */
	AT_SetActionMotion( act, NULL, (int *)EnemyMotionArray21 ) ;
#else
	if(mt_array == NULL){
		for( i=0; i<EM_STANDARD_MAX; i++ ) {
			EnemyMotionArray21[i] = i ;
		}
		AT_SetActionMotion( act, NULL, (int *)EnemyMotionArray21 ) ;
	}else {
		AT_SetActionMotion( act, NULL, mt_array ) ;
	}
#endif
	AT_SetMode( act, ENE_ActStandStill ) ;

	act_num = 0 ;
	act->c_motion_num[0] = act_num ;
	GM_ConfigObjectAction( act->body, 0, act->motion_table[act_num], 0, MOTION_MASK_FULL, 0 );

	/* にじみ血 */
printf("IN ENE INIT ACTION \n");
if(act->oozeblood == NULL){
	printf("oozeblood == NULL\n ");
}else {
	printf("oozeblood memory set\n ");
}

	if ( !(entk->status & ENE_STATUS_DEAD) ) {	/* 死体じゃなければ */
		if (!( act->bodyp.type & ENE_TYPE_TNG_A )) {
			if(!(entk->status_status & ENE_STST_NO_OOZEBLOOD)){
				act->oozeblood = NewOozeBlood( body->objs, entk->name_id.body ) ;
			}
		}
	}

	/* ＩＫ */
#ifdef TAKABE_IK
	act->new_ik = TAKABE_MakePuppetIK( ctrl, body );
#endif
#ifndef NO_KANO_IK
	act->ik = StartCorpseIK( ctrl, body ) ;
#endif
	act->ik_time = 0 ;

	/* アイテム */
	if ( !(entk->status & ENE_STATUS_DEAD) ) {	/* プログラム呼び出しの場合はだめ 後で修正！！*/
#if 1
		if ( (GM_GameLevel != GM_LEVEL_E_EXTREME) &&
			 (GCL_GetOption( 'q' ) != NULL) ){
			KRTH_GetItemProc( &act->item, ctrl->name ) ;
		} else {
			act->item.n_proc = 0 ;
			act->item.c_proc = 0 ;
		}
#else
		if ( GCL_GetOption( 'q' ) != NULL ){
			KRTH_GetItemProc( &act->item, ctrl->name ) ;
		} else {
			act->item.n_proc = 0 ;
			act->item.c_proc = 0 ;
		}
#endif
	}

	/* ホールドアップアイテム */
	if ( !(entk->status & ENE_STATUS_DEAD) ) {	/* プログラム呼び出しの場合はだめ 後で修正！！*/
#if 1
		if ( (GM_GameLevel != GM_LEVEL_E_EXTREME) &&
			 (GCL_GetOption( 'a' ) != NULL) ){
			KRTH_GetItemProc( &act->hold_item, ctrl->name ) ;
		} else {
			act->hold_item.n_proc = 0 ;
			act->hold_item.c_proc = 0 ;
		}
#else
		if ( GCL_GetOption( 'a' ) != NULL ){
			KRTH_GetItemProc( &act->hold_item, ctrl->name ) ;
		} else {
			act->hold_item.n_proc = 0 ;
			act->hold_item.c_proc = 0 ;
		}
#endif
	}

	/* ドッグタグ*/
	entk->sw.dogtag = 0 ;
	entk->d_name = NULL ;
	if ( !(entk->status & ENE_STATUS_DEAD) ) {	/* プログラム呼び出しの場合はだめ 後で修正！！*/
//#ifdef PAL
#if 1
		if ( (GCL_GetOption( 'j' ) != NULL) &&
			 (GM_GameLevel != GM_LEVEL_E_EXTREME) ){
#else
		if ( GCL_GetOption( 'j' ) != NULL ){
#endif
			char *ptr_le, *tmp, *country ;
			int	level, birth ;
			
			level = 0 ;
			switch ( GM_GameLevel ) {
				case GM_LEVEL_VERYEASY :
printf("enemy: game level very easy\n");
				break ;
				case GM_LEVEL_EASY :
printf("enemy: game level easy\n");
					level = 1 ;
				break ;
				case GM_LEVEL_NORMAL :
printf("enemy: game level normal\n");
					level = 2 ;
				break ;
				case GM_LEVEL_HARD :
printf("enemy: game level gard\n");
					level = 3 ;
				break ;
				case GM_LEVEL_EXTREME :
printf("enemy: game level extreme\n");
					level = 4 ;
				break ;
			}

			tmp = GCL_NextStr();	/* NextStr 先取り */
			act->dogtag_item.proc[ 0 ] = GCL_GetNextInt();

			ptr_le = GCL_GetNextResource_LE();
			for ( i=0; i<level; i++ ) {
				ptr_le = GCL_GetNextResource_LE();
			}

			GCL_SetArgTop( ptr_le );
			act->dogtag_id = GCL_GetNextInt();
ASSERT( act->dogtag_id >= 0 ) ;
#ifdef DOGTAG_DOUBLE
			if ( GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 ) {
				for ( i=0; i<DOGTAG_RES_NUM; i++ ) {
					GCL_GetNextInt();
				}
			}
#endif
			act->dogtag_item.str = GCL_GetNextString();
			country = GCL_GetNextString();
			birth = GCL_GetNextInt();

			act->dogtag_item.probability[ 0 ] = 100 ;
			act->dogtag_item.n_proc = 1 ;
			act->dogtag_item.c_proc = 0 ;
			if ( SIG_CheckDogTagFlag( act->dogtag_id ) ) {
				act->dogtag_item.c_proc = 1 ;	/* 取得済み */
			} else {
				GV_SetActorChild( entk->w, NewDogtagFlash( body, &entk->sw.dogtag ) ) ;
				GV_SetActorChild( entk->w, 
					NewCreateDogtag( body, E_DOGTAG_NAME, &DG_ZeroVector, &entk->sw.dogtag) ) ;
//				NewCreateDogtag( body, E_DOGTAG_NAME, &DG_ZeroVector, &entk->sw.dogtag) ;
			}
			{	/* 名前出力 */
				extern	void *NewNamePrint( OBJECT *body, char *str ,int id, int birth ) ;
				entk->d_name=NewNamePrint( act->body,act->dogtag_item.str, act->dogtag_id, birth) ;
			}
//			if ( !(GM_Configuration & GM_CONFIG_STORY_TANKER) ) {
//				if ( strcmp( country, "JPN" ) == 0 ) {
				if ( strcmp( country, "0" ) == 0 ) {
//printf("country is JPN!! You are Bluff Soldire!! \n" );
printf("country is MGS!! You are Bluff Soldire!! \n" );
					SET_FLAG( entk->status, ENE_STATUS_BLUFF ) ;
				}
//			}
printf("dogtagname[%s] country[%s]\n",act->dogtag_item.str, country);
		} else {
			act->dogtag_id = -1 ;
			act->dogtag_item.n_proc = 0 ;
			act->dogtag_item.c_proc = 0 ;
		}
	}

	/* ヘッドマーク */
//if ( entk->id !=0 ) SET_FLAG( entk->status, ENE_STATUS_NO_HEADMARK ) ;
	if ( !(entk->status & ENE_STATUS_NO_HEADMARK) ) {
		GV_SetActorChild( entk->w, (act->headmarkwork = NewControl_Headmark3( 
				&(BODYWORLD( body, HUMAN21_ATAMA )), &act->bodyp.deftrg, ctrl ) ) ) ;
	} else {
		printf("enemy.c: No Head Mark !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	}

}

int ENE_InitThink( entk, ctrl, rnavi, znavi, act, root, node )
ENETHINK	*entk ;
CONTROL		*ctrl ;
ROUTENAVI	*rnavi ;
ZONENAVI	*znavi ;
ACTION		*act ;
int			root ;
int			node ;
{
	extern void *NewBig_Add_Objnum( FVECTOR *, int * ) ;

	int		i, zone ;

	if ( entk->status_status & ENE_STST_FAINT_EXIT ) {
		AT_SetType( act, ENE_TYPE_FAINT_EXIT ) ;
	}
	if ( entk->status_status & ENE_STST_HOLD_EXIT ) {
		AT_SetType( act, ENE_TYPE_HOLD_EXIT ) ;
	}
	if ( entk->status_status & ENE_STST_DONT_KILL ) {
		AT_SetType( act, ENE_TYPE_DONT_KILL ) ;
	}

	ENE_InitRouteNavi( rnavi, root, 0 ) ;
	ENE_InitZoneNavi( znavi ) ;
	ENE_InitTargPoint( &entk->trgpoint ) ;
	ENE_InitEneThink( entk, ctrl, rnavi, znavi, act ) ;

	ctrl->mov = rnavi->nodes[ node ] ;
	rnavi->next_node = node ;
	entk->sense.facedir = ctrl->rot.vy = ctrl->turn.vy = rnavi->pa_dir[ node ] ;
	entk->sense.glasses_dis = DEF_GLASSES_DIS ;
	entk->act->old_body_height = entk->act->body->height ;


	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
	    GM_ConfigControlHzxHeight( ctrl, 750.0F, ctrl->mov.vy ) ;
	} else {
	    GM_ConfigControlHzxHeight( ctrl, 750.0F, ctrl->mov.vy + 100.0F ) ;
    }

	GM_ConfigControlMapID( ctrl ) ;

	if ( entk->act->bodyp.type & ENE_TYPE_HITECH ) {
		GM_ConfigControlAttribute( ctrl, (ctrl->attribute|CTRL_ATR_HANG_THROUGH) ) ;
	}

	if ( entk->status & ENE_STATUS_NO_BLURR ) {
		SET_FLAG ( entk->act->bodyp.type, ENE_TYPE_NO_BLURR ) ;
	}

	entk->last_radio_pos = ctrl->mov ;
	entk->last_radio_map = ctrl->hzx_id ;
printf("enemy: start addr[%x] pos[%f][%f][%f]\n",ctrl->addr,ctrl->mov.vx,ctrl->mov.vy,ctrl->mov.vz ) ;

	ENE_SetTrgpPoint( &(entk->trgpoint), &(ctrl->mov), ctrl->hzx_id) ;
	zone = HZX_GetAddress( ctrl->hzx_id, &ctrl->mov, -1 ) ;
	for( i=0; i<4; i++ ) {
		entk->before_inzone[i] = ctrl->addr ;
	}

	entk->name_id.g_id = entk->g_id ;
	entk->name_id.u_id = entk->u_id ;
	entk->name_id.id = entk->id ;
	entk->name_id.voice = entk->voice_chara ;
	entk->name_id.uniq_id = entk->uniq_id ;
	entk->name_id.name = entk->ctrl->name ;

	if ( entk->act->bodyp.type & (ENE_TYPE_HOLD_EXIT|ENE_TYPE_FAINT_EXIT) ) {
		VR_AddEnemy()  ;
	}

#ifdef DEBUG_MODE
	GV_SetActorChild( entk->w, NewBig_Add_Objnum( &(ctrl->mov), &(entk->head_num) ) ) ;
	entk->head_num = 0 ;	/* 攻撃兵 赤 */
	if ( entk->g_id == 0 ) {
		if ( entk->u_id == 0 ) {
			entk->head_num = 2 ;
		} else {
			entk->head_num = 3 ;
		}
	}
	entk->head_num |= entk->id << 3 ;

{
	extern void *NewEyeView3( FMATRIX *world,
			int len, int range, int upper, int under, COMMANDER *com, int *st, int gla ) ;
	NewEyeView3( &BODYWORLD( entk->act->body, HUMAN21_ATAMA ), entk->sense.eye_s,entk->sense.eye_r,
		 -UPPER_EYE_SIGHT, -UNDER_EYE_SIGHT, entk->com, &entk->status2, entk->sense.glasses_dis ) ;
}


if ( entk->g_id == 10 && entk->u_id == 0 && entk->id==0) {
	extern	void		*NewConnectObject( OBJECT *, int, FVECTOR *, SVECTOR *, int ) ;
	NewConnectObject( entk->act->body, 2, NULL, NULL, E_WP_AKS_SP ) ;
}
#endif

/*VRモード ロックオンカーソル*/
	if ( entk->status_status & ENE_STST_VRBODY ) {
		extern	void *NewHomingSight( int *,FMATRIX *,float,float,float ) ;
		GV_SetActorChild( entk->w,
		(void *)NewHomingSight( &entk->lockon,&BODYWORLD( entk->act->body, HUMAN21_KOSHI ),
		600.0f,800.0f,800.0f)) ;
	}

/*ＶＲモード 頭上の文字*/
#if 0
	if ( entk->status_status & ENE_STST_VRBODY ) {
		if ( entk->act->bodyp.type & (ENE_TYPE_HOLD_EXIT|ENE_TYPE_FAINT_EXIT) ) {
			extern void *NewSIG_3DPOSPrint(FMATRIX * ,int ,int *,float ) ;
			GV_SetActorChild( entk->w,
//				(void *)NewSIG_3DPOSPrint( &BODYWORLD( entk->act->body, HUMAN21_ATAMA ),
//				10,&entk->act->body->objs->flag,250.0f)) ;
				(void *)NewSIG_3DPOSPrint( &BODYWORLD( entk->act->body, HUMAN21_KOSHI ),
				10,&entk->act->body->objs->flag,750.0f)) ;
		}
	}
#endif
	return 0 ;
}

/* 敵兵情報当てはめ */
void ENE_LoadEneMemory( ENETHINK *entk )
{
	extern	void *MekeNewStageCorps( ENETHINK * ) ;
	ENEMEM *enm ;
	FVECTOR pos ;
	ACTION *act ;

	enm = ENEMEM_Load( GM_CurrentStage, entk->ctrl->name ) ;
	if ( enm == NULL ) return ;
printf("enemy:load enemy memory[%d] name[%d]\n",GM_CurrentStage, entk->ctrl->name ) ;

printf("load:stage[%d]\n",enm->stage) ;
printf("load:hzx_id[%d]\n",enm->hzx_id) ;
printf("load:x[%f]\n",enm->x) ;
printf("load:y[%f]\n",enm->y) ;
printf("load:z[%f]\n",enm->z) ;
printf("load:dir[%d]\n",enm->dir) ;
printf("load:status[%d]\n",enm->status) ;
printf("load:pose[%d]\n",enm->pose) ;
printf("load:time[%d]\n",enm->time) ;
printf("load:life[%d]\n",enm->life) ;
printf("load:faint[%d]\n",enm->faint) ;
printf("load:pbreak[%d]\n",enm->pbreak) ;
printf("load:item[%d]\n",enm->item) ;

	act = entk->act ;

	act->bodyp.life = enm->life ;
	act->bodyp.faint = enm->faint ;
	act->item.c_proc = ENEMEM_ITEM1(enm->item) ;
	act->hold_item.c_proc = ENEMEM_ITEM2(enm->item) ;

	if ( enm->status == ENEMEM_ST_NORMAL ) {
		if ( act->bodyp.life <= 0 ) act->bodyp.life = 10 ;
		if ( act->bodyp.faint <= 0 ) act->bodyp.faint = 1 ;
		return ;
	}

	/* 場所移動 */
	pos.vx = enm->x ;
	pos.vy = enm->y ;
	pos.vz = enm->z ;

	act->ctrl->height=0 ;/* 床リセット */
//enm->hzx_id = 0 ;//保険
	GM_ResetControlPositionAndGroup( entk->ctrl, &pos, enm->hzx_id ) ;
	entk->ctrl->rot.vy = entk->ctrl->turn.vy = enm->dir ;
#if 1
	act->ctrl->turn.vx = ENE_GetGRotFromPos( &act->ctrl->mov, &act->ctrl->rot, 1000.0f, 
				act->ctrl->hzx_id, HZX_CHK_ALL, act->ctrl->flr_flag ) ;
	act->ctrl->rot.vx = act->ctrl->turn.vx ;
#endif


	act->ctrl->skip_flag |= CTRL_RESET_HZX_BASE ;	/* 床補正 */

	switch ( enm->status ) {
		case ENEMEM_ST_SLEEP :
		case ENEMEM_ST_FAINT :
//			act->bodyp.pbreak = enm->pbreak ;
			ENE_SetEnemyDown( entk->act, enm->status, enm->time, enm->pose ) ;
			ENE_EnemyStartModeDamage( entk ) ;
//			COM_UnsetAccident( entk->uniq_id ) ;	/*定時連絡兵確認無し*/
		break ;
		case ENEMEM_ST_HELL :
			ENE_SetEnemyDown( entk->act, enm->status, enm->time, enm->pose ) ;
			ENE_EnemyStartModeDamageDeath( entk ) ;
#if 1
			MekeNewStageCorps( entk ) ;
			entk->old_uniq_id = entk->uniq_id  ;
			entk->uniq_id = entk->name_id.uniq_id = COM_GetUniqID( ) ;
#endif
		break ;
		case ENEMEM_ST_HOLDUP :
		break ;
	}

	/*高さ調整*/
	entk->act->body->height = 
		MT_GetMotionStartHeight( entk->act->body->m_ctrl, entk->act->c_motion_num[0] ) ;
    GM_ActMotion( entk->act->body ) ;

	entk->act->old_body_height = entk->act->body->height ;

	/*付属品調整*/
	SET_FLAG( entk->sw_gun, SW_FLAG_SWITCH2 ) ;	/* 死体のため始めは胸に出す */
}


/* ターゲットコールバック関数 */
static	void	ChildTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	ENETHINK	*entk ;

	entk = ( ENETHINK * )ptr ;

    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = DG_ZeroVector ;
			GM_DamageTarget( off, def ) ;
			entk->act->bodyp.off_center = off->center ;
//			def->power->force = off->power->force ;
//printf("child def->weapon_type[%lx] off[%lx]\n",def->weapon_type,off->weapon_type);
//printf("Ko [%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
}

/* 注！！！修正時はhold/holdene.c も修正すること！！ */
void	ENE_DefTargCallBack( off, def, entk )
TARGET		*off, *def ;
ENETHINK	*entk ;
{
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = DG_ZeroVector ;
			GM_DamageTarget( off, def ) ;
			entk->act->bodyp.off_center = off->center ;
//printf("oya call def->weapon_type[%x] off[%x]\n",def->weapon_type,off->weapon_type);
//printf("Oya [%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
}

void	ENE_PushTargCallBack( off, def, entk )
TARGET		*off, *def ;
ENETHINK	*entk ;
{
    if ( def->damaged & TARGET_TOUCH ) {
		if ( off->class & TARGET_TOUCH ) {	/* こいつに攻撃された */
			def->hit = off->center ;
//printf("oya call def->weapon_type[%x] off[%x]\n",def->weapon_type,off->weapon_type);
//printf("Oya [%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
}

int ENE_InitTarget( entk, ctrl, body, act )
ENETHINK	*entk ;
CONTROL		*ctrl ;
OBJECT		*body ;
ACTION		*act ;
{
	BODYPARAM		*bodyp ;
	TARGET			*trg, *child0, *child1, *child2, *child3 ;
	POWER_TARGET	*power, *power_child0, *power_child1, *power_child2, *power_child3 ;
    FVECTOR	size, offset ;
    int	i ;

	bodyp = &act->bodyp ;
	trg = &bodyp->deftrg ;
	power = &bodyp->power ;

	child0 = bodyp->def_child0 ;
	power_child0 = &bodyp->power_child0[0] ;
	child1 = bodyp->def_child1 ;
	power_child1 = &bodyp->power_child1[0] ;
	child2 = bodyp->def_child2 ;
	power_child2 = &bodyp->power_child2[0] ;
	child3 = bodyp->def_child3 ;
	power_child3 = &bodyp->power_child3[0] ;

	size = EneBodyTrgSize[ ENE_BODYTRGSIZE_STAND ] ;
    GM_SetTarget( trg, TARGET_DEFENSE|TARGET_SEEK, 1, ENEMY_SIDE,  &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( trg, 0 ) ;
    GM_SetPowerTarget( trg, power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;

	if(entk->status &ENE_STATUS_NO_CAPTURE){
		UNSET_FLAG( trg->class, TARGET_CAPTURE ) ;
	}else {
	    GM_SetCaptureTarget( trg, &( bodyp->capture ), ctrl, body ) ;
	}
    GM_PutTarget( trg ) ;
//	NewTargetView( &bodyp->deftrg, 34, 184, 200 ) ;

#if 1 //BP - seems we should leave this in even though they only added it for the windows version.
//#ifdef KP_WINDOWS
	size.vw   = 1.0F ;
	offset.vw = 1.0F ;
#endif
	for( i=0; i<PTARGET_LEVEL0_NUM; i++ ) {
		switch ( i ) {
			case PTARGET_RADIO:
				size.vx = 15.0F ; size.vy = 50.0F ; size.vz = 15.0F ;
				offset.vx = -120.0F ; offset.vy = -15.0F ; offset.vz = -100.0F ; 
				break ;
			case PTARGET_NVISION1:
				size.vx = 50.F ; size.vy = 40.0F ; size.vz = 60.0F ;
				offset.vx = 0.0F ; offset.vy = 190.0F ; offset.vz = 80.0F ; 
				break ;
			case PTARGET_NVISION2:
				size.vx = 50.F ; size.vy = 40.0F ; size.vz = 50.0F ;
				offset.vx = 0.0F ; offset.vy = 80.0F ; offset.vz = -80.0F ;
				break ;
		}
		GM_SetTarget( child0, CHILD_TRG_FLAG, 1, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child0, 0 ) ;
		GM_SetPowerTarget( child0, power_child0, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child0, ChildTargCallBack, entk ) ;

//		NewTargetView( child0, 200, 34, 184 ) ;

child0->class |= TARGET_SKIP ;/* デバッグ */

		child0++ ;
		power_child0++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child0, PTARGET_LEVEL0_NUM, 0 ) ;

	for( i=0; i<PTARGET_LEVEL1_NUM; i++ ) {
		switch ( i ) {
			case PTARGET_HEAD:
				size.vx = size.vz = 96.0F ; size.vy = 130.0F ;
				offset.vx = offset.vz = 0.0F ; offset.vy = 30.0F ;
				break ;
			case PTARGET_HART:
				size.vx = 50.F ; size.vz = 165.0F ; size.vy = 65.0F ;
				offset.vx = 100.0F ; offset.vz = 0.0F ; offset.vy = 100.0F ;
				break ;
			case PTARGET_GOLD:
				size.vx = 100.0F ; size.vz = 150.0F ; size.vy = 65.0F ;
				offset.vx = offset.vz = 0.0F ; offset.vy = -150.0F ;
				break ;
		}
		GM_SetTarget( child1, CHILD_TRG_FLAG, 1, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child1, 0 ) ;
		GM_SetPowerTarget( child1, power_child1, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child1, ChildTargCallBack, entk ) ;

//		NewTargetView( child1, 200, 34, 184 ) ;
#if 1
/*sigeno add hart&gold target skip */
		if(bodyp->type & ENE_TYPE_EVENT_32){
/*頭のみ一発死に*/
			if((i==PTARGET_HART)||(i==PTARGET_GOLD)){
				child1->class |= TARGET_SKIP ;
			}
		}
#endif
		child1++ ;
		power_child1++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child1, PTARGET_LEVEL1_NUM, 1 ) ;

	for( i=0; i<PTARGET_LEVEL2_NUM; i++ ) {
		switch ( i ) {
			case PTARGET_ARMR1:
			case PTARGET_ARMR2:
			case PTARGET_ARML1:
			case PTARGET_ARML2:
			size.vx = size.vz = 75.0F ; size.vy = 150.0F ;
			offset.vx = offset.vz = 0.0F ; offset.vy = -130.0F ;
			break ;

			case PTARGET_LEGL1:
			case PTARGET_LEGR1:
			size.vx = 85.0F ; size.vz = 120.0F ; size.vy = 200.0F ;
			offset.vx = offset.vz = -10.0F ; offset.vy = -200.0F ;
			break ;

			case PTARGET_LEGL2:
			case PTARGET_LEGR2:
			size.vx = 85.0F ; size.vz = 120.0F ; size.vy = 250.0F ;
			offset.vx = offset.vz = -20.0F ; offset.vy = -250.0F ;
			break ;
		}
		GM_SetTarget( child2, CHILD_TRG_FLAG, 1, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child2, 0 ) ;
		GM_SetPowerTarget( child2, power_child2, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child2, ChildTargCallBack, entk ) ;

//		NewTargetView( child2, 34, 184, 200 ) ;

		child2++ ;
		power_child2++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child2, PTARGET_LEVEL2_NUM, 2 ) ;

	for( i=0; i<PTARGET_LEVEL3_NUM; i++ ) {
		size.vx = 200.0F ; size.vz = 200.0F ; size.vy = 300.0F ;
		offset.vx = offset.vz = 0.0F ; offset.vy = 0.0F ;
		GM_SetTarget( child3, CHILD_TRG_FLAG, 1, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child3, 0 ) ;
		GM_SetPowerTarget( child3, power_child3, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child3, ChildTargCallBack, entk ) ;

//		NewTargetView( child3, 184, 200, 34 ) ;

		child3++ ;
		power_child3++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child3, PTARGET_LEVEL3_NUM, 3 ) ;

	/* プッシュターゲット */
	trg = &bodyp->pushtrg ;
	size = EneBodyTrgSize[ ENE_BODYTRGSIZE_TOUCH ] ;
    GM_SetTarget( trg, TARGET_TOUCH|TARGET_DEFENSE, 1, ENEMY_SIDE,  &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( trg, 0 ) ;
	GM_SetTargetCallBack( trg, ENE_PushTargCallBack, entk ) ;
    GM_PutTarget( trg ) ;
//	NewTargetView( trg, 34, 184, 200 ) ;

	GM_SetHomingTrg( &(entk->hom), &(BODYWORLD( body, HUMAN21_MUNE )), body, &(ctrl->hzx_id),
			ctrl, HOMING_ENEMY ) ;
	GM_SetHomingTrgTarget( &(entk->hom), &bodyp->deftrg ) ;
	GM_PutHomingTrg( &(entk->hom) ) ;
	
	return 0 ;
}

/**** 重野追加分 usr03875/sigeno ****/

/*********
ゾーンアドレスからHZX_ZONのアドレスを取得
GROUP_ID不要
***********/
HZX_ZON *ENE_HZX_GetZone(int addr){

	HZX_ZON		*zone;

	zone = HZX_GetGroupFromNo( HZX_ZoneMapNo( addr ) )->zones
		+HZX_Zone1(addr);
	return zone;
}

/*************************************
ゾーンaddr1からゾーンaddr2が見えるかどうかを返す。
見える時０、見えない時１が返る。
園山さん作成
enemy用に重野修正
****************************************/
int ENE_ReadOnlinInfo(int addr1,int addr2)
{
	HZX_GRP *group;
	int zone1,zone2,swap;
	u_int	*info ;	    /* grp->online_info */
	int	n_zones ;		/* グループ内ゾーン数 */
    int	add, bit, shf ;

	/*違うマップだ*/
	if(HZX_ZoneMapNo( addr1 ) != HZX_ZoneMapNo( addr2 ) ){
		return 1;
	}
	zone1 = HZX_Zone1(addr1);
	zone2 = HZX_Zone1(addr2);
	/* 同じゾーンなので見える */
	if(zone1 == zone2){
		return 0;
	}
	if(zone1 > zone2) {
		/*zone1 < zone2 にする*/
		swap = zone1 ;
		zone1 = zone2 ;
		zone2 = swap ;
	}
	group = HZX_GetGroupFromNo( HZX_ZoneMapNo( addr1 ) ) ;
	info  = group->online_info;
	n_zones = group->n_zones ;

	/*読み取り開始*/
	zone1 ++ ; zone2 ++ ;
	bit = ( ( n_zones - 1 ) + ( n_zones - ( zone1 - 1 ) ) )
		* ( zone1 - 1 ) / 2 + ( zone2 - zone1 - 1 ) ;
	add = bit / ( sizeof( u_int ) * 8 ) ;
	shf = 31 - ( bit % ( sizeof( u_int ) * 8 ) ) ;
	return ( *( info + add ) & ( 1 << shf ) ) ;
}

/*************************************
返り値 ゾーン経路の距離
引数のADDRは32bit型
引数のPOSが不要な場合はNULLを与えれば内部で
ゾーンの中心座標がセットされる
**************************************/
int ENE_GetRouteDis(firstpos,finalpos,firstaddr,finaladdr,max)
FVECTOR		*firstpos; /*スタート座標*/
FVECTOR		*finalpos; /*目標座標*/
int			firstaddr;/*スタートアドレス*/
int			finaladdr; /*目標アドレス*/
u_int		max;	/*距離がこの値をオーバーしたら計算を中断する*/
{
	FVECTOR		strpos,trgpos,goingpos,subvec;
	HZX_ZON		*zone;
	int			i,nowaddr,goingaddr,dis = 0,tmpdis;

	/*スタート座標セット*/
	if(firstpos == NULL){
		zone = ENE_HZX_GetZone( firstaddr ) ;
		strpos.vx = zone->x;
		strpos.vy = zone->y;
		strpos.vz = zone->z;
	}else {
		strpos = *firstpos;
	}
	/*目標座標セット*/
//printf("finaladdr[%d]\n",finaladdr);
	zone = ENE_HZX_GetZone( finaladdr ) ;
ASSERT(zone!= NULL) ;
	if(finalpos == NULL){
		trgpos.vx = zone->x;
		trgpos.vy = zone->y;
		trgpos.vz = zone->z;
	}else {
		trgpos = *finalpos;
		trgpos.vy = zone->y;
	}
	nowaddr = firstaddr;
	/************/
	/*最初の一回目*/

	goingaddr = HZX_Navigate( nowaddr,finaladdr, &strpos );


	if( goingaddr == finaladdr ){
//	if(HZX_ReachTo( goingaddr,finaladdr ) <= HZX_DIRECT_REACH){
		/* 隣接あるいは同一ゾーン内なので直接座標で比較 */
		_sceVu0SubVector(  &subvec, &strpos, &trgpos ) ;
		dis = _FVecLen3( &subvec ) ;
		return dis;
	}else {
		/*まだ到達していないので次のゾーン中心と比較*/
		zone = ENE_HZX_GetZone( goingaddr ) ;

		goingpos.vx = zone->x;
		goingpos.vy = zone->y;
		goingpos.vz = zone->z;
		_sceVu0SubVector(  &subvec, &strpos, &goingpos ) ;
		dis = _FVecLen3( &subvec ) ;
		nowaddr = goingaddr;
		strpos = goingpos;
	}
/*
2000.05.02 DEBUG
最終目標の手前ゾーンと座標比較させる
*/
	finaladdr = HZX_Navigate( finaladdr,firstaddr, &trgpos );

	while(1){
		goingaddr = HZX_Navigate( nowaddr,finaladdr, &strpos );
		if( goingaddr == finaladdr ){
//	if(HZX_ReachTo( goingaddr,finaladdr ) <= HZX_DIRECT_REACH){
			/*最終目標に到達した*/
			_sceVu0SubVector(  &subvec, &strpos, &trgpos ) ;
			dis += _FVecLen3( &subvec );
			return dis;
		}else {
			if(goingaddr == nowaddr ){
				/*ゾーン誘導で行けない場所に目標がある*/
				_sceVu0SubVector(  &subvec, &strpos, &trgpos ) ;
				dis += _FVecLen3( &subvec ) ;
				return dis;
			}
			/*まだ到達していないので次のゾーン中心と比較*/
			zone  = ENE_HZX_GetZone( nowaddr ) ;
			for(i=0;i<6;i++){
				if(zone->nears[i] == HZX_Zone1(goingaddr)){
					/*ゾーン間距離を復元*/
					tmpdis = (int)zone->dists[i];
					dis += (tmpdis-1)*500;
					/*上限オーバー*/
					if(dis > max) return dis;
					nowaddr = goingaddr;
					zone  = ENE_HZX_GetZone(  goingaddr ) ;
					strpos.vx = zone->x;
					strpos.vy = zone->y;
					strpos.vz = zone->z;
					break;
				}
			}
		}
	}
}

/*攻撃発生場所とタイプを指定可能*/
#if 0 /* yano 2002.03.11 */
void ENE_SetOffenseTarget2( act,type,parts,size )
ACTION	*act ;
long64	type; /*攻撃タイプ*/
int		parts; /*関節番号 HUMAN21_MIGI_TEなど*/
FVECTOR	*size; /*サイズ*/
#else
void ENE_SetOffenseTarget2( ACTION	*act,long64 type, int parts, FVECTOR *size )
#endif
{
//	static FVECTOR force = { 0.0F,-200.0F,400.0F } ;
	static FVECTOR force = { 0.0F,-100.0F,100.0F } ;

	ENE_SetOffenseTarget3( act,type,parts,size,&force) ;

}
/**/
#if 0 /* yano 2002.03.11 */
void ENE_SetOffenseTarget3( act,type,parts,size,force)
ACTION	*act ;
long64	type; /*攻撃タイプ*/
int		parts; /*関節番号 HUMAN21_MIGI_TEなど*/
FVECTOR	*size; /*サイズ*/
FVECTOR	*force ; /*サイズ*/
#else
void ENE_SetOffenseTarget3( ACTION	*act, long64 type, int parts, FVECTOR *size, FVECTOR *force)
#endif
{
//	static FVECTOR force = { 0.0F,-200.0F,400.0F } ;

	TARGET	*off ;
	POWER_TARGET	*power ;
	FVECTOR			v ;

	off = &( act->offense ) ;
	power = &( act->off_pow ) ;

	/*init部に移行すると思う 名前セット*/
	off->name = act->ctrl->name ;

	GM_SetTarget( off, TARGET_OFFENSE , 0,
	PLAYER_SIDE, size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( off, type|WP_NOPLAYER ) ; 

	GM_MoveTarget3( off, &( BODYWORLD(act->body, parts) ) ) ;

	DG_SetPos2( &act->ctrl->mov, &act->ctrl->rot ) ;
    DG_RotVector( force, &v, 1 ) ;

//	GM_SetPowerTarget( off, power, POWER_ONCE, 255, 0, 0, &v ) ;
	GM_SetPowerTarget( off, power, POWER_ONCE, 255, 0, ENE_PUNCH_DMG, &v ) ;
	GM_PutTarget( off ) ;
#if 0
	NewTargetView( off, 34, 184, 200 ) ;
#endif

}

static void ENE_MotStepCorrect( ACTION *act ){
	float speed ;
	FVECTOR	tmp ;
	/*モーション移動量を加工*/
	if(act->mot_speed_correct != 0.0F ){
		act->ctrl->step.vx *= act->mot_speed_correct ;
		act->ctrl->step.vz *= act->mot_speed_correct ;
	}
	/*モーション移動方向を加工*/
	if(act->mot_dir_correct >= 0){
		tmp.vx = act->ctrl->step.vx ;
		tmp.vy = 0.0F ;
		tmp.vz = act->ctrl->step.vz ;
		speed = GV_VecLen3F( &tmp ) ;

		act->ctrl->step.vx 
			= speed * _RsinF( (int) act->mot_dir_correct ) ;
		act->ctrl->step.vz 
			= speed * _RcosF( (int) act->mot_dir_correct ) ;
	}
	act->mot_speed_correct = 0.0F;
	act->mot_dir_correct = -1;

}
/*武器名(strcode)から装弾数を取得*/
short	ENE_GetAmmoMax(int wpname){
	switch(wpname) {
		case E_WP_AKS :
			return MAX_MAGAZINE_AK ;
			break ;
		case E_WP_AKS_SP :
			return MAX_MAGAZINE_AK ;
			break ;
		case E_WP_SPS :
			return MAX_MAGAZINE_SPS ;
			break ;
		case E_WP_MKR :
			return MAX_MAGAZINE_MKR ;
			break ;
		case E_WP_P90 :
			return MAX_MAGAZINE_P90 ;
			break ;
		case E_WP_ABAKAN :
			return MAX_MAGAZINE_AK ;
			break ;
		case E_WP_M4_NM :
			return MAX_MAGAZINE_AK ;
			break ;
		default :
//			printf("WARNING! UNKNOWN WP NAME!! strcode[%d]\n",wpname);
			/*刀天狗なのでP90用*/
			return MAX_MAGAZINE_P90 ;
			break ;
	}
	return MAX_MAGAZINE_AK ;
}
