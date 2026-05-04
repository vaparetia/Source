//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_clb.c 
   オルガ コールバック処理関数群

   1999/12/18 T.Morita
   $Id: orga_clb.c,v 1.1.1.3 2002/11/19 11:46:20 Yoshizawa1 Exp $
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

#include "include/orga.h"


static void SetNeedle( OBJECT *body, int n_obj, FVECTOR *pos, FVECTOR *dir )
{
    extern void VertexSearch( FVECTOR *vans, FVECTOR *nans,
			      DG_OBJS *objs, int objnum, FVECTOR *target) ;
    extern void *NewAttachment4_called( int model_name, FVECTOR *v, SVECTOR *r,
					OBJECT *target, int objnum, FVECTOR *x,int angle_limit,
					int frames ) ;
    FVECTOR	vans, nans ;
    int model_name ;

	/* 麻酔弾 */
    model_name = GV_StrCode( "m92_bul2" ) ;
    VertexSearch( &vans, &nans, body->objs, n_obj, pos );
    NewAttachment4_called( model_name, dir, NULL, body, n_obj, &vans, 512, 4 );
}

void ORG_DamageVitality( Work *work, int damage, u_long64 m9 )
{
    int v ;

	if ( m9 )
	  v = work->vitality_m9 - damage ;
	else 
	  v = work->vitality     - damage ;

#if 0/*ダメージ受けた用のカットイン*/
    /* 半分まで弱っている */
    if ( v <= ORGA_MAX_VITALITY/2 && work->vitality > ORGA_MAX_VITALITY/2 )
	;//work->act_flg = ORGA_F_HALF_ATTK, work->flag |= ORGA_F_NVR_RESET ;
    /* 3/4まで弱っている */
    else if ( v <= ORGA_MAX_VITALITY*3/4 && work->vitality > ORGA_MAX_VITALITY*3/4 )
	printf( "3/4 comes\n" ) ,
	work->act_flg = ORGA_F_THIRD_ATTK, work->flag |= ORGA_F_NVR_RESET ;
    /* かなり弱っている */
    else if ( v <= ORGA_MAX_VITALITY/8 && work->vitality > ORGA_MAX_VITALITY/8 )
printf( "1/8 comes\n" ) ,
	work->act_flg = ORGA_F_EIGHT_ATTK, work->flag |= ORGA_F_NVR_RESET ;
    /* 弱っている */
    else if ( v <= ORGA_MAX_VITALITY/4 && work->vitality > ORGA_MAX_VITALITY/4 )
	;//work->act_flg = ORGA_F_QUAD_ATTK, work->flag |= ORGA_F_NVR_RESET ;
#endif

	if ( m9 )
	  work->vitality_m9 = v ;
	else 
	  work->vitality = v ;

}

static void DamageRoutine( Work *work, PART *p, TARGET *off )
{
    /* M92だったら刺さる音 */
    if ( off->weapon_type & WP_M92 )
	GM_SeSetMode( SD_A_M_NINE01, &work->control.mov, GM_SEMODE_BOMB ) ;

    if ( off->weapon_type & (WP_BULLET|WP_BLOW|WP_BLAST|WP_M92|WP_STUNFAR|WP_STUNGRENADE) )
    {
	/* やられ声 */
	work->voice_tim = 0 ;
	GM_SeSetMode( irnd()&0x10 ? ORGA_VO_DAMAGE0 : ORGA_VO_DAMAGE1,
		      &work->control.mov, GM_SEMODE_BOMB ) ;

printf( "orga_clb.c : damaged %d weapon %x %x\n", p->damage,
	   (off->weapon_type>>32),off->weapon_type ) ;

	/* ダメージ量を計算し 特別行動を発行するかどうかを検討する */
	ORG_DamageVitality( work, p->damage,
					   off->weapon_type & (WP_M92|WP_STUNFAR|WP_STUNGRENADE)) ;

	/* EASYだったら麻酔銃を打った時少しずつ減っていく */
	if ( GM_GameLevel<=GM_LEVEL_EASY && off->weapon_type & WP_M92 )
	    work->bit_dmg = work->vitality_m9 - ORGA_MAX_VITALITY/2 ;
    }
    /* 死んでいないか */
    if ( !(off->weapon_type & WP_THROWG) )
    {
	if ( work->vitality > 0 && work->vitality_m9 > 0 )
	    ORG_ActDamageStart( work, p->motion ) ;
	else
	{
		if ( work->vitality < 0 )
		  work->vitality = 0 ;/* 負の時は ０にする */
		if ( work->vitality_m9 < 0 )
		  work->vitality_m9 = 0 ;/* 負の時は ０にする */

	    /* スネークの方がゲームオーバーになっている？ */
	    if ( !GM_IsGameOver() ) {
		if ( work->procs[ORGA_P_PASTOUT] ) {
		    int usp = work->vitality==0 ? 1 : 0 ;
		    GCL_ARGS arg = { 1, &usp } ;

		    GCL_ExecProc( work->procs[ORGA_P_PASTOUT], &arg ) ;
		}
		work->flag |= ORGA_F_DEMO_MOVIE ;	    
		work->head_mark = HMK2_TYPE_KILL ;/* 頭の上のマークは消す */
		ORG_ActDamageOverStart( work ) ;
	    }
	}
    }
    /* ストリーム音声がなっていたら止める */
    if ( work->str_hdl )
	GM_StreamStop( work->str_hdl ) ;
    work->voice_vox |=  7 ;/* やられた場合は次のセリフを急ぐ */
}

void ORG_PartsTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    PART *p = (PART *)ptr ;
    Work *work = p->work ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	/* 主観 ビハインド 飛び込み 以外はダメージにならない */
	if ( (PL_SubjectMove || GM_PlayerStatus & PLAYER_WATCH || GM_PlayerStatus & PLAYER_BEHIND) &&   //BP_CAMERA - added PL_SubjectMove (which we use for 1st person movement mode) so Ogla can be damaged
	     !(work->flag & ORGA_F_NVR_DAMAGED) )
	{
	    /* M９２だったら針をつける */
	    if ( (p->part_id != HUMAN21_ATAMA) && (off->weapon_type & WP_M92) )
		SetNeedle( &work->body, p->part_id, &def->hit, &off->power->force ) ;

	    DamageRoutine( work, p, off ) ;
	}
    }
}


void ORG_MainTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    PART *p = (PART *)ptr ;
    Work *work = p->work ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	/* 主観 ビハインド 飛び込み はダメージにならない */
	if ( !(PL_SubjectMove || GM_PlayerStatus & PLAYER_WATCH || GM_PlayerStatus & PLAYER_BEHIND) &&  //BP_CAMERA - added PL_SubjectMove (which we use for 1st person movement mode) so Ogla can be damaged
	     !(work->flag & ORGA_F_NVR_DAMAGED) )
	    DamageRoutine( work, p, off ) ;
    }
}
