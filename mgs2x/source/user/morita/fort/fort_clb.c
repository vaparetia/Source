//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_clb.c 
   フォーチュン コールバック処理関数群

   1999/12/18 T.Morita
   $Id: fort_clb.c,v 1.1.1.3 2002/11/19 11:46:06 Yoshizawa1 Exp $
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


void FRT_HitTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work= (Work *)ptr ;

    printf( "Fort: TRGT %.0f, %.0f, %.0f", work->trgt_aim->vx, work->trgt_aim->vy, work->trgt_aim->vz  ) ;
    if ( FRT_AIM_VerifyAimSpot( def, &work->trgt_aim_pos ) )
    {
	printf( "-> %.0f, %.0f, %.0f changed", work->trgt_aim_pos.vx, work->trgt_aim_pos.vy, work->trgt_aim_pos.vz  ) ;
	work->trgt_aim = &work->trgt_aim_pos ;
    }
    printf( "\n" ) ;
}


void FRT_MainTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    extern void *NewFortBulletCalled( int id, float rot_vy, float speed,
				      FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid, int life ) ;
    extern void *NewFortSplineBulletCalled( FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid ) ;
    float d ;
    static int voice[] = {
	SD_V_FORTUN03,  //フォーチュン弾逸れ「だめね」//fortun03 781
	SD_V_FORTUN04,  //フォーチュン弾逸れ「はずれ」//fortun04 782
	SD_V_FORTUN05,  //フォーチュン弾逸れ「あたら～」//fortun05 783
	SD_V_FORTUN06,  //フォーチュン弾逸れ「お前に～」//fortun06 784
    } ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;


	/* 弾避けカットが呼ばれる */
        if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    /* 表示する弾の位置を計算 */
	    _sceVu0SubVector( &work->ply_shoot,
			      &def->hit,
			      FRT_MiscPlayerPosNoHide( HUMAN21_MIGI_TE ) ) ;
	    d = _sceVu0InnerProduct( &work->ply_shoot, &work->ply_shoot ) ;
	    d = sceVu0Sqrt( d ) ;
	    d = d>3000.0f ? 3000.0f : d ;

	    _sceVu0Normalize( &work->ply_shoot, &off->power->force ) ;
	    _sceVu0ScaleVector( &work->ply_shoot, &work->ply_shoot, d ) ;
	    _sceVu0SubVector( &work->ply_shoot, &def->hit, &work->ply_shoot ) ;

	    NewFortSplineBulletCalled( &work->ply_shoot, &off->power->force,
				       BODYPOS( &work->body, HUMAN21_KOSHI ) ) ;
	    /* 跳弾 */
	    GM_SeSetMode( SD_A_RICFOR01, &work->control.mov, GM_SEMODE_BOMB ) ;

	    /* 場所が見えていなかったら */
	    if ( work->flag & FRT_F_UNRECOG_POS )
	    {
		work->flag &= ~FRT_F_UNRECOG_POS ;

		GM_SeSetMode( SD_V_FORTUN10, &work->control.mov, GM_SEMODE_BOMB ) ; //「はっ!」

		work->head_mark = HMK2_TYPE_RED_AT ; /* ！マーク */
		work->head_time = 10 ;/*この秒数だけ,ビックリした時以外のビックリマークは出ない*/
	    }

	    /* プレイヤーが撃ったはずなので */
	    work->ply_aim_cnt++ ;

	    work->voice = voice[GM_StagePlayTime&3] ;
	    work->voice_tim = 40 ;

	    /* ドラム缶壊しフェーズ前ならカウンターアタックをする */
	    if ( work->act_phase < 0x0d00 )
		work->act_flg |= FRT_F_COUNTER_ATTK ;

	    if ( !(work->flag & FRT_F_SHOOT_AT_ME) )
		if ( FRT_PLY_PLAYERPOS.vz >= FRT_DEAD_ZONE_Z )
		{
		    /* フェーズが最後の方でないならプロックを実行 */
		    if ( work->act_phase < 0x0d00 )
			FRT_RecogExecProc( work, FRT_P_FIRE_AT_FORTUNE ) ;

		    /* 弾を一度でも避けたことになる */
		    work->flag |= FRT_F_SHOOT_AT_ME ;
		}
	}
    }
}
