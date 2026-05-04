//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ema_pre.c
  エマ 前処理

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_utl.c,v 1.1.1.3 2002/11/19 11:46:00 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/emma.h"




/*

  ゲームオーバー＆ライフ関係

*/
int EMA_GameOverCheck( Work *work, int pad )
{
    if ( GM_IsGameOver() )
	return 0 ;

    if ( work->npc.action.life <= 0 ||
	 work->npc.action.status & NPC_ACT_STATUS_DEATH ||
	 (EMA_Flag(EMA_F_EVENT_SNIPE) && work->control.mov.vy < -43000.0f) )
    {
	/* ゲームオーバ処理は1回だけ */
	if ( !EMA_Flag( EMA_F_GAMEOVER ) )
	{
	    int voice = 0 ;

	    if ( EMA_Flag(EMA_F_HURT_BY_PLY) ||
		 work->mar_mtn == MAR(EMA_CAPT,CAP_HANG) )
		if ( ++GM_KillCount > 30000 ) /* エマを殺した */
		    GM_KillCount = 30000 ;
	    if ( work->npc.action.current_mar != EMA_BASE )
		voice = SD_V_EMAOUT01 ;

	    /* ヘッドマークは消す */
	    NPC_CallHeadMark( &work->npc, HMK2_TYPE_KILL ) ;
	    /* ゲームオーバー処理を開始 */
	    GM_GameOverProcStart( work ) ;
	    /* ゲームオーバ プロックがあれば呼ぶ */
	    if ( work->proc[EMA_P_GAMEOVER] )
		GCL_ExecProc( work->proc[EMA_P_GAMEOVER], NULL ) ;

	    switch ( work->npc.action.set_pad )
	    {
	    case DAMG_DROPOFF:
	    case DAMG_DOWN_OVER:
	    case SNIPE_DOWN_FALL:
	    case SNIPE_SLEEP_FALL:
		break ;

	    default:
		if ( pad )
		{
		    voice = SD_V_EMAOUT01 ;
		    NPC_SetModeFromPad( &work->npc,
					EMA_ActDamage, work->npc.base_mar,
					pad, pad ) ;
		}
		else if ( work->mar_mtn != MAR(EMA_CAPT,CAP_HANG) )
		{
		    if ( work->npc.action.status & NPC_ACT_STATUS_HANG )
			NPC_SetModeFromPad( &work->npc,
					    EMA_ActDamage, work->npc.base_mar,
					    DAMG_FALL, DAMG_FALL ) ;
		    else
			NPC_SetModeFromPad( &work->npc,
					    EMA_ActDamage, work->npc.base_mar,
					    DAMG_DOWN_OVER, DAMG_DOWN_OVER ) ;

		    work->control.skip_flag &= ~(CTRL_SKIP_FLR_CHECK |
						 CTRL_SKIP_SEG_CHECK) ;

		    if ( EMA_Flag(EMA_F_HURT_BY_VMP) &&
			 !EMA_Flag(EMA_F_HURT_BY_PLY ) )
			 voice = 0 ;

		}
		else
		    voice = SD_V_EMAOUH02 ;
	    }

	    if ( voice )
	    {
		work->voice_tim = 0 ;
		GM_SeSetMode( voice, &work->control.mov, GM_SEMODE_BOMB ) ;
	    }
	}
	work->npc.action.life = 0 ;
	work->flag |= EMA_F_GAMEOVER ;
	return 0 ;
    }
    return 1 ;
}

void EMA_SetInvincible( Work *work )
{
    if ( GM_GameLevel>=GM_LEVEL_HARD )
	work->non_damage = 10*5/TIME_BASE ;
    else if ( EMA_Flag( EMA_F_EVENT_SNIPE ) )
	work->non_damage = EMA_INVINCIBLE_TIME ;
    else
	work->non_damage = EMA_INVINCIBLE_TIME ;
}

int EMA_DamageVitality( Work *work, int damage, int pad )
{
#if DEBUG_MODE
    if ( EMA_DbgInvincible )
	damage = 0 ;
#endif

    if ( GM_IsGameOver() || GM_CheckGameStatus( STATE_SCN_DEMO ) )
	return 1 ;

    if ( damage )
    {
	if ( pad )
	    work->voice_id = SD_V_EMADMG01, work->voice_tim = 1 ;

	if ( EMA_Flag( EMA_F_HURT_BY_PLY ) )
	{
	    if ( !EMA_Flag( EMA_F_HURT_BY_VMP ) )
		damage *= 5 ;
	    else
		damage *= 2 ;
	}

	EMA_SetInvincible( work ) ;

	work->vib_rate = 7*60*5/TIME_BASE ;/* 7秒間 鼓動を速くする */

#if DEBUG_MODE
	printf( "Emma: Woops, I've got a damage. %d %x\n", damage,
		EMA_Flag( EMA_F_HURT_BY_PLY ) ) ;
#endif
    }

    /* ライフをダメージ分だけ減らす */
    work->npc.action.life -= damage ;
    return EMA_GameOverCheck( work, pad ) ;
}

int EMA_DamageGameOver( Work *work, int pad )
{
    return EMA_DamageVitality( work, work->vital_max, pad ) ;
}

void EMA_RecoverVitality( Work *work, int life )
{
    if ( EMA_Flag( EMA_F_ENB_RECOVER ) )
	 /* 1秒ごとでなければメーターは上がらない */
	 if ( !(GM_StagePlayTime & 0x3f) )
	 {
	     work->npc.action.life += life ;
	     /* vital_maxは越えない */
	     if ( work->npc.action.life > work->vital_max )
		 work->npc.action.life = work->vital_max ;
	 }
}


/*

  目的地ゾーンの設定

*/
int EMA_SetNaviTarget( Work *work, int addr, FVECTOR *pos )
{
    /* 目的のゾーンは,無効なゾーンでなく,自分のいる所でない場合のみ */
    if ( HZX_Zone1( addr ) != 0xff && 
	 HZX_Zone1( addr ) != HZX_Zone1( work->control.addr ) )
    {
	work->navitrg.addr = addr ;

	/* ナビゲートを初期化し直す */
	GM_ReSetNavi( &work->navigate ) ;

	if ( pos )
	    _sceVu0CopyVector( &work->navitrg.pos, pos ) ;
	else
	{
	    HZX_ZON *zone = HZX_GetZoneFromAdd( work->navitrg.addr ) ;

	    work->navitrg.pos.vx = zone->x ;
	    work->navitrg.pos.vy = zone->y ;
	    work->navitrg.pos.vz = zone->z ;
	}
	return 1 ;
    }
    return  0 ;
}

/*

  aimに向かって位置をframesのフレームで補間する

*/
void EMA_SetAdjustPosition( Work *work, FVECTOR *aim, int flames )
{
    if ( work->pos_adjust.vw <= 0.0f )
    {
	_sceVu0SubVector( &work->pos_adjust, aim, &work->control.mov ) ;
	work->pos_adjust.vw = flames ;
	_sceVu0ScaleVector( &work->pos_adjust, &work->pos_adjust, 1.0f/work->pos_adjust.vw ) ;
    }
}


/*

  ルート情報

*/
void EMA_SetRoutePatrol( Work *work, int route_id )
{
    ASSERT( HZX_CurrentHzx ) ;

    work->r_patrol = HZX_GetCurrentHzx()->def->patrols ;
    work->r_patrol += route_id ;
}

void EMA_SetRoutePoint( Work *work, int point_id )
{
    if ( work->r_patrol->n_points > point_id )
	work->r_point = work->r_patrol->points + point_id ;
    else
	work->r_point = NULL ;
}

int EMA_GetDirFromRoute( Work *work, float length )
{
    float x, z ;

    /* 差分計算（方向ベクトル） */
    x = work->r_point->x - work->control.mov.vx ;
    z = work->r_point->z - work->control.mov.vz ;

    /* 向きを設定 */
    work->control.turn.vy = (short)(2048.0f / M_PI * atan2f( x, z )) ;

    if ( x*x + z*z < length*length )
    {
	work->route = work->r_point++ ; /* 通過ルートを登録して,次のルートへ */
	return 1 ;
    }
    return 0 ;
}


/*

  石橋を叩いて渡れ

  橋の乗り降りの時のみ,先を見て判断する
*/
float EMA_CheckDestFloor( Work *work, FVECTOR *dest )
{
    FVECTOR v ;
    float   level = 0.0f ;

    _sceVu0ApplyMatrix( &v, &work->body.objs->world, dest ) ;
    /* 初期床高さ */
    if ( HZX_LevelHazardCheck( work->control.hzx_id, &v, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
    {
	level = HZX_GetFloorLevel() ;
	level -= work->control.levels[0] ;
    }
    else
	ASSERT( 0 );

    return level ;
}



/*

  おされる方向を計算する

 */
void EMA_Pushed( Work *work, int se )
{
    float force ;

    if ( work->offtrg )
	_sceVu0SubVector( &work->vel_adjust,
			  &work->offtrg->center,
			  &work->control.mov ) ;
    work->vel_adjust.vy = 0.0f ;
    work->vel_adjust.vw = 0.0f ;

    force = 40.0f*1000.0f/work->pl_dis ;
    if ( force > 150.0f )
	force = 150.0f ;

    _sceVu0Normalize( &work->vel_adjust, &work->vel_adjust ) ;
    _sceVu0ScaleVector( &work->vel_adjust, &work->vel_adjust, force ) ;
    work->control.turn.vy = work->pl_dir ;

    work->voice_id  = se ;
    work->voice_tim = 1  ;
}



/*

  最も近いハイドを捜す

 */
int EMA_GetNearestHide( Work *work, float *min )
{
    float   d ;
    FVECTOR v ;
    int     idx, i ;

    //*min = 80000.0f*80000.0f ; /* 索敵範囲 最大距離 */
    *min = 400000.0f*400000.0f ; /* 索敵範囲 最大距離 */
    idx = -1 ;
    for ( i=work->n_hide ; --i>=0 ; )
    {
	_sceVu0SubVector( &v, &work->control.mov, &work->hide[i] ) ;
	d = _sceVu0InnerProduct( &v, &v ) ;
	if ( d < *min )
	    idx = i, *min = d ;
    }
#ifdef DEBUG_MODE
    if ( *min > 8000.0f*8000.0f )
    {
	printf( "Emma : Isn't it too far ?? %f\n", sceVu0Sqrt( *min ) ) ;
    }
#endif
    return idx ;
}



/*

  近傍コントロール全検索

*/
int EMA_CheckControl( Work *work, CONTROL *c, float dist, FVECTOR *v  )
{
    if ( c->map & work->control.map )
	if ( fpu_Abs( c->mov.vy - work->control.mov.vy ) < 1100.0f )
	{
	    v->vx = c->mov.vx - work->control.mov.vx ;
	    v->vz = c->mov.vz - work->control.mov.vz ;

	    if ( v->vx*v->vx + v->vz*v->vz < dist )
		return 1 ;
	}
    return 0 ;
}

CONTROL* EMA_NearControl( Work *work, float dist, FVECTOR *v )
{
    int	     i ;
    CONTROL *c ;

    dist *= dist ;/* ２乗で比較する */
    for ( i=GM_N_WhereList ; --i>=0 ; )
    {
	c = GM_WhereList[i] ;

	/* 自分と同じマップで,自分以外のコントロールを検索 */
	if ( c != &work->control )
	    if ( EMA_CheckControl( work, c, dist, v ) )
		return c ;
    }

    return NULL ;
}

CONTROL *EMA_NearEneControl( Work *work, float dist, FVECTOR *v )
{
    HOMING_TRG	*h ;

    dist *= dist ;/* ２乗で比較する */

    if ( EMA_CheckControl( work, GM_PlayerControl, dist, v ) )
	return GM_PlayerControl ;

    for ( h=GM_GetHoming() ; h ; h=h->next )
    {
	if ( h->status & HOMING_SKIP || !(h->status & HOMING_ENEMY) )
	    continue ;
	if ( EMA_CheckControl( work, h->ctrl, dist, v ) )
	    return h->ctrl ;
    }

    return NULL ;
}





/*

  主観時に画面に映っているかどうか

 */
int EMA_CheckScreen( Work *work )
{
     FVECTOR head, foot ;

     if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )
     {
	  _sceVu0ApplyMatrix( &head,
			      &DG_Chanl(0)->eye_pers,
			      BODYPOS(&work->body, HUMAN21_ATAMA           ) ) ;
	  _sceVu0ApplyMatrix( &foot,
			      &DG_Chanl(0)->eye_pers,
			      BODYPOS(&work->body, HUMAN21_HIDARI_TSUMASAKI) ) ;
	  foot.vy /= foot.vw ;
	  head.vy /= head.vw ;
	  head.vx /= head.vw ;
	  if ( fpu_Abs(foot.vy - head.vy) > 0.8f &&
	       head.vx<1.0f && head.vx>-1.0f )
	       return 1 ;
     }
     return 0 ;
}



/*

  敵がいるかどうか

*/
int EMA_ThinkCheckEnemy( Work *work )
{
    return work->enemy_flg ;
}


/*

  敵またはプレーヤーを自動的に避ける

*/
int EMA_Navi( Work *work )
{
     FVECTOR  shift ;/* 障害物までのベクトル*/
     FVECTOR  vecs  ;/* 壁までのベクトル    */
     FVECTOR  erect ;/* えまが進むベクトル  */
     float    leng, ang, b ;
     //int      i ;
     CONTROL *ctrl ;
     int      dir ;

#if 0
     R_INTRPT   *r ;

     /* ドアがいきなり閉まった場合に備えて */
     if ( work->stage >= 0 )
     r = GM_GetRIntrpt( work->control.addr, work->navigate.next_addr ) ;
     if ( r )
     {
	 printf( "Emma : GM_GetRIntrpt stat %x\n", r->status & (ROOT_INTRPT_NONPC|ROOT_INTRPT_CLOSE) ) ;
	 if ( r->status & (ROOT_INTRPT_NONPC|ROOT_INTRPT_CLOSE) )
	     if ( work->navigate.next_addr != work->navitrg.addr )
		 return 1 ;
     }
#endif

     if ( GM_Navi( &work->navigate, &work->navitrg, 300 ) )
	 return 1 ;

     /* 障害物を検索 */
     ctrl = EMA_NearEneControl( work, 1200.0f, &shift ) ;
     if ( ctrl )
     {
	  shift.vy = 0.0f ;
	  dir = work->navigate.next_dir & 0x0fff ;
	  ang = ANG2RAD( dir - 4096*(dir>>11) ) ;

	  /* 目標の進行方向に対して障害物が邪魔しているか */
	  leng =  shift.vx*shift.vx + shift.vz*shift.vz ;
	  leng = sceVu0Sqrt( leng ) ;
	  /* 障害物を避けるため、XとZの入れ換え進行ベクトルを垂直にする*/
	  erect.vx =  shift.vz / leng ;
	  erect.vz = -shift.vx / leng ;
	  if ( erect.vz*sinf(ang) - erect.vx*cosf(ang) < 0.0f )
	  {
	       leng = 1600.0f ;

	       /* 障害物と壁で進めないことがないように逃げる方向を決める */
	       if ( work->control.n_touches )
	       {
		    /*障害物と壁の距離 b を求める */
		    _sceVu0Normalize( &vecs, &work->control.vecs[0] ) ;
		    _sceVu0ScaleVector( &shift, &vecs,
					_sceVu0InnerProduct( &vecs, &shift ) ) ;
		    _sceVu0SubVector( &shift, &shift,
				      &work->control.vecs[0] ) ;
		    b = _sceVu0InnerProduct( &shift, &shift ) ;

#if 0
		    _sceVu0AddVector( &shift, &shift,
				      &ctrl->mov ) ;
		    AN_Test_Eye2( &shift, 2 ) ;
		    printf( "b%.3f\n", sceVu0Sqrt( b ) ) ;
		    printf( "touch%.3f ", 
			    _sceVu0InnerProduct( &erect, &vecs ) ) ;
#endif

		    /*進行方向と壁の方向が同じかどうかを見る */
		    if ( _sceVu0InnerProduct( &erect, &vecs ) > -0.2f )
			 /*壁との距離が80cm以上であれば反対方向に行く */
			 if ( b < 800.0f*800.0f )
			      leng = -1600.0f ;
	       }
	       _sceVu0ScaleVector( &erect, &erect, leng ) ;
	       _sceVu0AddVector( &erect, &erect, &ctrl->mov ) ;

#if 0
	       printf( "%.0f\n", leng ) ;
	       AN_Test_Eye2( &erect, 2 ) ;
#endif

	       _sceVu0SubVector( &erect, &erect, &work->control.mov ) ;
	       work->navigate.next_dir = GV_VecDir2( &erect ) ;
	  }
     }

     return 0 ;
}


/*

  ストリーム関係

*/
void EMA_UtilStopStream( Work *work )
{
    /* ハンドラを初期化する（ストリームが使われていないことを意味する） */
    if ( work->voice_str >= 0 )
    {
	/* 止める時もprocがあれば呼ぶ */
	if ( work->voice_proc )
	{
	    int i = 1 ;
	    GCL_ARGS arg = { 1, &i } ;
	    GCL_ExecProc( work->voice_proc, &arg ) ;
	}

	printf( "EMA_UtilStopStream%x\n", work->voice_code ) ;

	GM_StreamStop( work->voice_str ) ;
	work->voice_str  = -1 ;
	work->voice_code = 0 ;
	work->voice_flg  = 0 ;
	work->voice_proc = 0 ;
    }
}

int EMA_UtilStartStream( Work *work, int id )
{
    /* ハンドラを取得 */
    /* スネークが喋っている場合は */
    if ( work->voice_flg == EMA_VCF_SNIPER_SNAKE_WISP )
	EMA_UtilStopStream( work ) ;

    /* エマが喋っている場合も */
    if ( work->voice_flg == EMA_VCF_SNIPER_EMMA_WISP )
	EMA_UtilStopStream( work ) ;

    if ( work->voice_str >= 0 )
	return -1 ;

    printf( "EMA_UtilStartStream%d %d\n", work->voice_str, id ) ;
    if ( !EMA_Flag(EMA_F_EVENT_SNIPE) )
	if ( work->pl_dis > 3000.0f ||
	     GM_AlertMode == ALERT_MODE_ALERT ||
	     GM_AlertMode == ALERT_MODE_AVOID )
	    return -1 ;

    if ( EMA_Flag(EMA_F_EVENT_SNIPE) )
//	work->voice_str = GM_VoxStream( id, GM_STREAM_PAUSE_MENU ) ;
	work->voice_str = GM_VoxStream( id, GM_STREAM_PAUSE_MENU | GM_STREAM_FLAG_3D ) ;
    else
//	work->voice_str = GM_VoxStream( id, 0 ) ;
	work->voice_str = GM_VoxStream( id, GM_STREAM_FLAG_3D ) ;
    if ( work->voice_str >= 0 )
    {
	work->voice_code = id ;
	work->voice_cnt++ ;
    }

    return work->voice_str ;
}

int EMA_UtilStartStreamIdx( Work *work, int idx )
{
    if ( idx >=0 )
	if ( work->voice_chk & (1<<idx) )
	    return -1 ;
    return EMA_UtilStartStream( work, work->voice[idx] ) ;
}


/* 

   弾だし(本揺らしための)

 */
void EMA_UtilBullet( Work *work, FVECTOR *aim )
{
    static FVECTOR From = { 0.0f, 0.0f, 700.0f, 1.0f } ;
    FMATRIX dir = DG_UnitMatrix ;

    work->non_damage = 2 ;
    _sceVu0ApplyMatrix( (FVECTOR*)dir.m[W], 
			&BODYWORLD(&work->body, HUMAN21_KOSHI), &From ) ;
    _sceVu0SubVector( (FVECTOR*)dir.m[Y], (FVECTOR*)dir.m[W], aim ) ;
    _sceVu0Normalize( (FVECTOR*)dir.m[Y], (FVECTOR*)dir.m[Y] ) ;

    NewBullet( &dir, BUL_TYPE_HIT_CHILD, ENEMY_SIDE,
	       25, 10, 15000, 800, WP_Usp ) ;
}


/*

  エマの状態をリセットする。
  基本的には,手を離した状態になる

*/
void EMA_UtilFullReset( Work *work )
{
    EMA_ResetSetFlag( EMA_F_ENB_ARM_IK, EMA_F_ENB_IK|EMA_F_ENB_LINK ) ;
    EMA_ResetPuppetIK( work->arm_ik ) ;

    work->tic     = 0 ;
    work->time    = 0 ;

    work->prv_trg = 0 ;
    work->cold_stare = 0 ;
    work->cold_count = 0 ;

    /* ステップをリセット   */
    _sceVu0CopyVector( &work->control.step, &DG_ZeroVector ) ;
    /* ホーミングをリセット */
    _sceVu0CopyVector( &work->head_dir    , &DG_ZeroVector ) ;
    _sceVu0CopyVector( &work->breast_dir  , &DG_ZeroVector ) ;

    /* 全てpadに合わせる */
    work->npc.action.set_pad = work->npc.action.pad ;

    /* 最初のモーションにする */
    GM_ConfigObjectAction( &work->body, 0,
			   work->npc.action.pad, 0, 0x001fffff, 0 ) ;
    NPC_SetModeFromPad( &work->npc, EMA_ActLoopMotion,
			work->npc.base_mar,
			work->npc.action.pad,
			work->npc.action.pad ) ;

    work->mar_mtn = MAR( work->npc.action.current_mar,
			 work->npc.action.current_mot ) ;

    work->deftrg.class &= ~TARGET_SKIP ;/* ターゲット有効 */

    printf( "Emma : I'm Reseted.\n" ) ;
}

/* 

   強制移動

 */
void EMA_UtilForceMove( Work *work, float x, float y, float z )
{
    CONTROL *ctrl ;

    printf( "Emma : I moved to (%f %f %f)\n", x, y, z ) ;

    ctrl = &work->control ;
    ctrl->mov.vx = x ;
    ctrl->mov.vy = y ;
    ctrl->mov.vz = z ;

    /* システムで高さを自動的にリセットさせる */
    if ( HZX_LevelHazardCheck( ctrl->hzx_id,
			       &ctrl->mov,
			       HZX_CHK_ALL, HZX_FLOOR_ALL ) )
	ctrl->mov.vy = HZX_GetFloorLevel() ;
    GM_ConfigControlHzxHeight( ctrl, 750.0F, ctrl->mov.vy ) ;

    work->npc.action.pad = PAL_IDLE ;

    EMA_UtilFullReset( work ) ;
}


/*

  手繋ぎ専用関数

  手を繋ぐために目標とする場所を計算する。

*/
void EMA_ThinkDestinatePos( Work *work,
			    FVECTOR *diff, FVECTOR *aim, FMATRIX *world,
			    int onstep )
{
    int     turn ;

    /*右後ろを目標にしている*/
    turn = EMA_RecalcDir( GM_PlayerControl->turn.vy ) ;
    _sceVu0RotMatrixY( world, &DG_UnitMatrix, ANG2RAD( turn ) ) ;
    _sceVu0CopyVector( (FVECTOR *)world->m[W],
		       (FVECTOR *)GM_PlayerBody->objs->world.m[W] ) ;
    if ( EMA_Flag( EMA_F_IS_ON_STEP ) )
	_sceVu0ApplyMatrix( aim, world, &EMA_Shifts[onstep] ) ;
    else
	_sceVu0ApplyMatrix( aim, world, &EMA_Shifts[0] ) ;

    /*目標位置との差分(方向,距離を計算するため)*/
    _sceVu0SubVector( diff, aim, &work->control.mov ) ;
}




/*

  手繋ぎ専用関数

  目的地が壁に向かっているかどうか

*/
int EMA_UtilDestinateToWall( Work *work, FVECTOR *dir/*単位ベクトル*/ )
{
    FVECTOR diff, aim ;
    FMATRIX world ;
    float   x, z, d ;

    /* 壁に当たったら止まる */
    if ( work->control.n_touches )
    {
	/* 壁との距離 */
	x = work->control.vecs[0].vx ;
	z = work->control.vecs[0].vz ;
	if ( (d = x*x + z*z) < 500.0f*500.0f )
	{
	    /* 両方の壁に囲まれている */
	    if ( work->control.n_touches == 2 )
	    {
		x = work->control.vecs[1].vx ;
		z = work->control.vecs[1].vz ;
		if ( x*x + z*z < 500.0f*500.0f )
		    return 1 ;
	    }

	    if ( dir )
		d = (dir->vx*x + dir->vz*z) / sceVu0Sqrt( d ) ;
	    else
	    {
		EMA_ThinkDestinatePos( work, &diff, &aim, &world, 0 ) ;
		_sceVu0Normalize( &diff, &diff ) ;
		d = (diff.vx*x + diff.vz*z) / sceVu0Sqrt( d ) ;
	    }

//printf( "%f %f %d\n", d, work->wall_rot,  d > work->wall_rot );

	    /* 向きを考慮するのは目的方向と 80°以内なら手を繋げない */
	    if ( d > work->wall_rot )
	    {
		work->wall_rot = 0.5f ;
		return 1 ;
	    }
	}
    }
    work->wall_rot = 0.82f ;
    return 0 ;
}

