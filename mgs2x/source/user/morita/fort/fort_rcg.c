//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_rcg.c 
   オレガ 認知情報 関数群

   2000/12/22 T.Morita
   $Id: fort_rcg.c,v 1.1.1.3 2002/11/19 11:46:07 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

#include "libutl.h"
#include "include/fort.h"









/*

  ストリーム再生

*/
void FRT_RecogStartStream( Work *work )
{
    /* ハンドラを取得 */
//    work->str_hdl = GM_VoxStream( work->str_id[work->voice_vox>>12], 0 ) ;
    work->str_hdl = GM_VoxStream( work->str_id[work->voice_vox>>12], GM_STREAM_FLAG_3D ) ;

    /*マルチウェイト表示*/
    work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
    DG_InvisibleObjs( work->body.objs ) ;
}
void FRT_RecogStopStream( Work *work )
{
    /* ハンドラを初期化する（ストリームが使われていないことを意味する） */
    work->str_hdl = 0 ;

    /*シングルウェイト表示*/
    work->body.evmobj->flag |=  DG_EVMOBJ_INVISIBLE ;
    DG_VisibleObjs( work->body.objs ) ;
}


/*

  プレイヤーを狙うのは
  プレーヤの高さに合わせる。

*/
FVECTOR  *FRT_RecogAimPos( Work *work )
{
    return work->trgt_aim ;
}


/*

  ちょっと適当に狙っている

*/
static inline void FRT_RecogMissAim( Work *work )
{

    if (
#if 1
	GM_GameLevel >= GM_LEVEL_EXTREME ||
#endif
	 ( (work->act_phase & 0xff00) >= 0x0200 &&
	   (work->flag & FRT_F_ENB_WATCHATK) &&
	   !work->ply_hide && GM_CheckPlayerStatus( PLAYER_WATCH )) )
	_sceVu0CopyVectorXYZ( &work->trgt_mis_pos, FRT_MiscPlayerPosNoHide( HUMAN21_KUBI ) ) ;
    else if ( work->shot_tim >= 0 )
    {
	_sceVu0CopyVectorXYZ( &work->trgt_mis_pos, FRT_MiscPlayerPosNoHide( HUMAN21_KUBI ) ) ;

	/* 初めての主観撃ちの場合 必ず外す */
	if ( GM_CheckPlayerStatus( PLAYER_WATCH ) && !(work->flag & FRT_F_ENB_WATCHATK) )
	    work->trgt_mis_pos.vx += work->shot_cnt&1 ? 1000.0f : -1000.0f ;
	else if ( GM_GameLevel <= GM_LEVEL_NORMAL )
	    work->trgt_mis_pos.vx += work->trgt_mis_pos.vw ;

	if ( work->shot_tim-- == 0 )/* ２重にチェックを避けるため */
	    FRT_AIM_CheckAimSpot( work ) ;
    }
}



/*

  フォーチュンがスネークを見えているかどうか（正確さは要らない軽い奴）
  狙えるかどうかは,ちゃんと壁に遮られているのかを検知している。

*/
static inline void FRT_RecogICanSeeHim( Work *work )
{
    if ( !(work->ply_hide & FRT_F_PLYR_HIDING) )
    {
	/*少なくとも2秒以上見られると隠れていないことになる*/
	if ( (work->ply_hide & FRT_F_PLYR_HIDEXP) > FRT_F_PLYR_HIDTIC*2 ||
	     !(work->ply_hide & FRT_F_PLYR_HIDLNG) )
	{
	    work->trgt_hid = *FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ) ; 
	    work->ply_hide &= ~FRT_F_PLYR_HIDLNG ;
	    work->ply_hide &=  FRT_F_PLYR_HIDCLR ;
	    return ;
	}
	/*0.25秒ごとにカウントアップする*/
	if ( !(GV_Time & 0x0f) )
	    if ( (work->ply_hide & FRT_F_PLYR_HIDEXP) != FRT_F_PLYR_HIDEXP )
		work->ply_hide += FRT_F_PLYR_HIDTIC ;
    }

    /*少なくとも１秒以上見えない状態を作らないと見えないことにはならない*/
    if ( !(GV_Time & 0x3f) )
	if ( (work->ply_hide++ & FRT_F_PLYR_HIDTIM) > 1 )
	{
	    work->ply_hide |= FRT_F_PLYR_HIDLNG ;
	    work->flag |=  FRT_F_UNRECOG_POS ;
	}
}


/*

  ボムが自分の周りにあったら無効にする。

*/
static inline void FRT_RecogNearBomb( Work *work )
{
    GM_BOMB *b ;
    FVECTOR  v ;
    extern void *NewLineSmoke( FMATRIX *world, FVECTOR *shift, int life,
			       float pow, float max_size ) ;

    /*グレネードが止まったら カットプロックを発動する*/
    if ( !(work->flag & (FRT_F_THROW_AT_ME|FRT_F_DEMO_MOVIE)) )
	if ( work->ply_grenade )
	{
	    if ( work->ply_throw.vx == work->ply_grenade->mov->vx &&
		 work->ply_throw.vy == work->ply_grenade->mov->vy &&
		 work->ply_throw.vz == work->ply_grenade->mov->vz )
	    {
		if ( work->ply_throw.vx < -3500.0f )
		    work->ply_throw.vx = -3500.0f ;
		if ( work->ply_throw.vx >  3500.0f )
		    work->ply_throw.vx =  3500.0f ;
		if ( work->ply_throw.vz < -9500.0f )
		    work->ply_throw.vz = -9500.0f ;
		if ( work->ply_throw.vz > -3500.0f )
		    work->ply_throw.vz = -3500.0f ;

		_sceVu0CopyVector( &work->ply_throw, work->ply_grenade->mov ) ;
		NewLineSmoke( NULL, &work->ply_throw,
			      60*8*TIME_BASE/5, 16.0f, 32.0f ) ;
		//FRT_RecogExecProc( work, FRT_P_THROW_AT_FORTUNE ) ;
		work->ply_grenade = NULL ;
		work->ply_aim_cnt++ ;

		work->flag |= FRT_F_THROW_AT_ME ;
	    }
	    else
		_sceVu0CopyVector( &work->ply_throw, work->ply_grenade->mov ) ;
	}

    /* ボムが近くにあるのか調べる */
    for( b=GM_BombList.next ; b!=NULL ; b=b->next )
	if ( !(b->flag & GM_BMB_FLAG_NOBLAST) )
	{
	    _sceVu0SubVector( &v, b->mov, &work->control.mov ) ;

	    if ( _sceVu0InnerProduct( &v, &v ) < 4000.0f*4000.0f )
	    {
		b->flag |= GM_BMB_FLAG_NOBLAST ;

		if ( b->weapon == WP_StunGrenade  ||
		     b->weapon == WP_ChaffGrenade ||
		     b->weapon == WP_Grenade      )
		    if ( !work->ply_grenade )
		    {
			work->ply_grenade = b ;
			_sceVu0CopyVector( &work->ply_throw,
					   work->ply_grenade->mov ) ;
		    }
	    }
	}
}





/*

  フォーチュンがプレイヤーの位置を追っているかどうか

*/
static inline void FRT_RecogUnrecognizing( Work *work )
{
    /*この条件は fort_thk_attk.h の条件と同じでなければならない */
    if ( work->head_time>0 )
	if ( !(GV_Time & 0x3f) )
	    work->head_time-- ;
#if 0

SD_V_FORDMG01,  //フォーチュンダメージ「うぉっ」//fordmg01 775
SD_V_FORDMG02,  //フォーチュンダメージ「ぉあっ」//fordmg02 776
SD_V_FORDMG03,  //フォーチュンダメージ「えぇは」//fordmg03 777
SD_V_FORDMG04,  //フォーチュンダメージ「ぅえひ」//fordmg04 778
SD_V_FORTUN01,  //フォーチュン嘲笑「あはははっ」//fortun01 779
SD_V_FORTUN02,  //フォーチュン挑発「出てらっ～」//fortun02 780
SD_V_FORTUN03,  //フォーチュン弾逸れ「だめね」//fortun03 781
SD_V_FORTUN04,  //フォーチュン弾逸れ「はずれ」//fortun04 782
SD_V_FORTUN05,  //フォーチュン弾逸れ「あたら～」//fortun05 783
SD_V_FORTUN06,  //フォーチュン弾逸れ「お前に～」//fortun06 784
SD_V_FORTUN07,  //フォーチュン気合「ふんっ」//fortun07 785
SD_V_FORTUN08,  //フォーチュン気合「はぁっ」//fortun08 786
SD_V_FORTUN09,  //フォーチュン気合「でぇえぃ」//fortun09 787
SD_V_FORTUN10,  //フォーチュン気合「はっ」//fortun10 788
SD_V_FORTUN11,  //フォーチュン「早く私を殺して」//fortun11 789
SD_V_FORTUN12,  //フォーチュン「何してるの？～」//fortun12 790
SD_V_FORTUN13,  //フォーチュン「殺してみなさ～」//fortun13 791
SD_V_FORTUN14,  //フォーチュン「貴方に私の悲～」//fortun14 792
SD_V_FORTUN15,  //フォーチュン「当たってないわ」//fortun15 793
SD_V_FORTUN16,  //フォーチュン「当たらないわ」//fortun16 794

#endif

}



/*

  プレイヤーのプレイの仕方

*/
static inline void FRT_RecogPlayer( Work *work )
{
    float d ;

    if ( (++work->ply_tic & 0x0f) == TIME_BASE )
    {
	if ( (work->ply_tic += 0x10-TIME_BASE) >= 0xa0 )
	    work->ply_time++, work->ply_tic = 0 ;

	/* プレイヤーが動いたかどうか ５frm以内に２５cm動けばOK */
	_sceVu0SubVector( &work->ply_prev, &work->ply_prev, &GM_PlayerControl->mov ) ;
	d = _sceVu0InnerProduct( &work->ply_prev, &work->ply_prev ) ;
	if ( d < 250.0f*250.0f )
	    work->ply_stop++   ;
	else
	    work->ply_stop = 0 ;
	_sceVu0CopyVector( &work->ply_prev, &GM_PlayerControl->mov ) ;
    }

    if ( GM_WeaponChanged )
	work->flag &= ~(FRT_F_SHOOT_AT_ME|FRT_F_THROW_AT_ME) ;
}



/*

  特別行動に移るかどうかの判定

*/
static inline void FRT_RecogAbortCurrentAction( Work *work )
{
    /* カットイン用の特別行動のみ残す */
    if ( work->flag & FRT_F_DEMO_MOVIE ||
	 work->ply_time > 15 ||
	 work->act_phase >= 0x0d00 )
    {
	work->act_flg = 0 ;
	return ;
    }

    /* 来ては行けないゾーンなので撃ちまくる */
    if ( FRT_PLY_PLAYERPOS.vz < FRT_DEAD_ZONE_Z )
    {
	if ( !(work->act_flg & FRT_F_STAND_ATTK) )
	    work->act = FRT_ActionReset ; /* 実行を中断 */
	work->act_flg |=  FRT_F_STAND_ATTK ;
    }
    else
	work->act_flg &= ~FRT_F_STAND_ATTK ;

    /* ３秒以上物影で止まっているので

       ドラム缶がないか 詣でに2つ以上の炎が上がっている場合は,この行動
       は,無効となる
     */
    work->act_flg &= ~FRT_F_GASTANK_ATTK ;
    if ( !(work->flag & FRT_F_NOMORE_DRUM) &&
	 !FRT_GasExplode && FRT_GasFire < 2 )
    {
	if ((FRT_PLY_PLAYERPOS.vx>-2500.0f || FRT_PLY_PLAYERPOS.vx<-3700.0f ||
	     FRT_PLY_PLAYERPOS.vz> 5000.0f || FRT_PLY_PLAYERPOS.vz< 3000.0f )&&
	    (FRT_PLY_PLAYERPOS.vx> 4161.0f || FRT_PLY_PLAYERPOS.vx< 2416.0f ||
	     FRT_PLY_PLAYERPOS.vz> 5000.0f || FRT_PLY_PLAYERPOS.vz< 3000.0f ))
	    work->drum_tim = 0 ;
	else if ( !work->ply_tic )
	    work->drum_tim++ ;
	if ( work->drum_tim >=3 )/*  3 秒以上いるので */
	    work->act_flg |= FRT_F_GASTANK_ATTK ;
    }

    /* じっと止まりすぎか 弾を撃たないので,たくさん撃つモードに移る */
    if ( work->ply_stop > 8*10*5/TIME_BASE && work->ply_time < 30
	 /*|| (work->ply_time > 20 && !work->ply_aim_cnt)*/ )
    {
	if ( !(work->act_flg & FRT_F_RAGING_ATTK) )
	    work->act = FRT_ActionReset ; /* 実行を中断 */
	work->act_flg |=  FRT_F_RAGING_ATTK ;
    }
    else
	work->act_flg &= ~FRT_F_RAGING_ATTK ;

    /* 影に隠れているので前まで来て撃つ */
    if ( (FRT_PLY_PLAYERPOS.vx > 5200.0f && FRT_PLY_PLAYERPOS.vz < 1000.0f) ||
	 (FRT_PLY_PLAYERPOS.vx <-5200.0f && FRT_PLY_PLAYERPOS.vz < 1000.0f) ||
	 (FRT_PLY_PLAYERPOS.vx > 4400.0f && FRT_PLY_PLAYERPOS.vz <-2400.0f) ||
	 (FRT_PLY_PLAYERPOS.vx <-4400.0f && FRT_PLY_PLAYERPOS.vz <-2400.0f) )
    {
	if ( !(work->act_flg & FRT_F_HIDING_ATTK) )
	    work->act = FRT_ActionReset ; /* 実行を中断 */
	work->act_flg |=  FRT_F_HIDING_ATTK ;
    }
    else
	work->act_flg &= ~FRT_F_HIDING_ATTK ;
}
 
/*

  エネルギー充填音を入れるかどうか

*/
static inline void FRT_RecogSetEnergySe( Work *work )
{
    extern int OK_LinerFlag;

    /* エネルギー溜める音を鳴らす */
    if ( work->flag & FRT_F_ENB_ENERGY_SE )
    {
	if ( work->wait_sec == 60*5/TIME_BASE-1 )
	{
	    GM_SeSetMode( SD_E_LGUNTAME, &work->control.mov, GM_SEMODE_BOMB ) ;

	    /* 照明を消す */
	    FRT_SendLightOffMessage( work ) ;

	    OK_LinerFlag = 1 ; /* 岡嶋さんのリニアガンフラグ */
	}
	if ( work->wait_sec <= 60*5/TIME_BASE-1 )
	    work->flag &= ~FRT_F_ENB_ENERGY_SE ;
    }

    /* ライトをつけるメッセージ */
    if ( work->lgt_on >=0 )
	if ( --work->lgt_on == 0 )
	{
	    OK_LinerFlag = 0 ;/* 岡嶋さんのリニアガンフラグ */

	    FRT_SendLightOnMessage( work ) ;
	}
}


/*

  プロックを実行する

*/
void FRT_RecogExecProc( Work *work, int id )
{
    if ( work->procs[id] && !GM_IsGameOver() )
	GCL_ExecProc( work->procs[id], NULL ) ;
}



/*

  爆発カウンタを管理

*/
static inline void FRT_RecogDecExplodeCounter( Work *work )
{
    if ( FRT_GasExplode > 0 )
	FRT_GasExplode-- ;
}


/*
  行動を取るための情報収集。

  ほぼ全ての情報がここで分かりやすい情報に加工される。
  フラグとしての情報でいい場合ものは,work->flagのビットして情報の加工が行なわれている。

  Recognizing whole my state to prepare for thinking!!

*/
void FRT_Recognition( Work *work )
{
    /* 生きている時しか関知しなくていいもの */
    if ( !GM_IsGameOver() && work->vitality > 0 )
    {
	/* プレイヤーの情報                       */
	FRT_RecogPlayer( work ) ;

#if 0
	work->flag &= ~FRT_F_PLYER_SHOOT ;
	if ( FRT_PLY_SHOOT() )
	    work->flag |= FRT_F_PLYER_SHOOT ;
	/*今回のフレームの弾数を取っておく*/
	FRT_PLY_EQUATESHOT() ;

	/* 近くにマガジンが落ちたかどうかを調べる */
	FRT_RecogHeardNoise( work ) ;

	/* ストリームをさせるかどうか */
	FRT_RecogStreaming( work ) ;
#endif

#if DEBUG_MODE	
	{
	    extern int FRT_DbgAgingTest ;/* エイジグテスト用 */
	    if (  FRT_DbgAgingTest )
	    {
		static float v = 1 ;
		GM_PlayerControl->mov.vx += v*5.0f ;
		if ( GM_PlayerControl->mov.vx >  4500.0f ||
		     GM_PlayerControl->mov.vx < -4500.0f )
		     v *= -1.0f ;
	    }
	}
#endif


	/* ミスショット用の計算 */
	FRT_RecogMissAim( work ) ;

	/* フォーチュンの近くにボム系が投げ込まれたかどうか */
	FRT_RecogNearBomb( work ) ;

	/* フォーチュンからスネークが見えているかどうか */
	FRT_RecogICanSeeHim( work ) ;

	/* プレイヤーの位置をフォーチュンが捉えているか */
	FRT_RecogUnrecognizing( work ) ;

	/* 充填音を入れるの？ */
	FRT_RecogSetEnergySe( work ) ;

	/* 爆発カウンタを更新 */
	FRT_RecogDecExplodeCounter( work ) ;

	/* 行動の中断をするべきかの判断           */
	if ( !(work->act_flg & FRT_F_INVALID_ATTK) && !(work->flag & FRT_F_NVR_RESET) )
	    FRT_RecogAbortCurrentAction( work ) ;
    }
}
