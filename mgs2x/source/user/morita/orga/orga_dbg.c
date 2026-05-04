//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_dsp.c 
   オルガ デバッグ用関数群

   1999/12/22 T.Morita
   $Id: orga_dbg.c,v 1.1.1.3 2002/11/19 11:46:20 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "include/orga.h"


#ifdef DEBUG_MODE
int ORG_DbgAction = 0 ;/* 撮影用デバグアクション 本当に要らない */
int ORG_PrevAction = 0 ;
int ORG_DebugFlag = 0 ;


void ORG_DummyFunc( char *fmt, ... )
{
}

void ORG_Debug( Work *work )
{
    if( GM_DebugModeEnable )
    {
#if 0
	if ( GV_PadData[1].press & PAD_U )
	    ORG_PrevAction++ ;
	if ( GV_PadData[1].press & PAD_D )
	    ORG_PrevAction-- ;
	if ( GV_PadData[1].press & PAD_X )
	    ORG_DbgAction = ORG_PrevAction ;
	if ( GV_PadData[1].press & PAD_R1 )
	    ORG_SendHoloMessage( 4 ) ;
#endif


#if 0
	/*限界位置のテスト*/
	if ( work->trgt_pos )
	{
	    FVECTOR v ;

	    _sceVu0CopyVector( &v, work->trgt_pos ) ;
	    v.vz =  -ORGA_HIDE_V_SPHERE - 12000.0f + 0.25f*(v.vx+12000.0f) ;
	    AN_Test_Eye2( &v, 2) ;
	}
#endif

	/*for debug EYES*/
	if ( ORG_DebugFlag & 1 )
	{
	    if ( work->trgt_pos )
		AN_Test_Eye2( work->trgt_pos, 2 ) ;
	    if ( work->trgt_aim )
		AN_Test_Eye2( work->trgt_aim, 2 ) ;

#if 0
	    /*スネークの狙撃場所のチェックのため*/
	    {
		static FVECTOR tst[] = {
		    {-9000, 12000, -16400}, {-7500, 12000, -17300},
		    {-9000, 12000, -17300}, {-7500, 12000, -16400},
		} ;
		static FVECTOR box_p = {0,12000,0,0};
		static float box[] = { -10550, -16500, -10550 , -16500, -10550, -17300, -13500 , -17500 } ;

		AN_Test_Eye2( &tst[0], 2 ), AN_Test_Eye2( &tst[1], 2 ) ;
		AN_Test_Eye2( &tst[2], 2 ), AN_Test_Eye2( &tst[3], 2 ) ;

		if ( work->trgt_pos == &ORG_Ply_AttkPosL )
		{
		    box_p.vx = box[0] ;
		    box_p.vz = box[1] ;
		    AN_Test_Eye2( &box_p, 2 ) ;
		    box_p.vx = box[2] ;
		    box_p.vz = box[3] ;
		    AN_Test_Eye2( &box_p, 2 ) ;
		}
		else if ( work->trgt_pos == &ORG_Ply_AttkPosR )
		{
		    box_p.vx = box[4] ;
		    box_p.vz = box[5] ;
		    AN_Test_Eye2( &box_p, 2 ) ;
		    box_p.vx = box[6] ;
		    box_p.vz = box[7] ;
		    AN_Test_Eye2( &box_p, 2 ) ;
		}
	    }
#endif

#if 0
	    /*オルガ隠れ場所の四隅チェック*/
	    {
		int i ;
		for ( i=work->n_hide_pool ; --i>=0 ; )
		{
		    FVECTOR a = { 0, 13000, 0, 0 } ;
		    a.vx = work->hide_pool[i].p[0][0] ;
		    a.vz = work->hide_pool[i].p[0][1] ;
		    AN_Test_Eye2( &a, 2 ) ;
		    a.vx = work->hide_pool[i].p[1][0] ;
		    a.vz = work->hide_pool[i].p[1][1] ;
		    AN_Test_Eye2( &a, 2 ) ;
		    a.vx = work->hide_pool[i].p[2][0] ;
		    a.vz = work->hide_pool[i].p[2][1] ;
		    AN_Test_Eye2( &a, 2 ) ;
		    a.vx = work->hide_pool[i].p[3][0] ;
		    a.vz = work->hide_pool[i].p[3][1] ;
		    AN_Test_Eye2( &a, 2 ) ;
		}
	    }
#endif

	}

    }

}




#undef printf
#define SOUND_TEST 0
#define MAX_FUNCLIST 20 

typedef struct
{
    GV_ACT  actor ;
    int name ;
} DbgWork  ;

extern void ORG_DispParams( Work *work ) ;
extern struct dbglst_t { void *adr ; char *name ; } ORG_ActionFunctionLists[] ;
static void (**FuncList[MAX_FUNCLIST])( Work * ) = {
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL, NULL,
} ;

static int stllmtn[] = {
    0,0,0,0,0,  0,0,0,0,0, 
    0,0,0,0,0,  0,0,0,0,0, 
} ;

static int    nFuncList = 0 ;

static void Act( DbgWork *work )
{
    int i, j ;
    struct dbglst_t *lst ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch ( msg->message[0] )
	{
	case 0:
	    ORG_DebugFlag |= 2 ;
	}
    if( GM_DebugModeEnable )
    {
	if ( GV_PadData[1].press & PAD_L2 ) ORG_DebugFlag ^= 1 ;
	if ( ORG_DebugFlag & 1 )
	    ORG_DispParams( ORG_Work ) ;
	if ( FuncList[nFuncList] != ORG_Work->act )
	    FuncList[ nFuncList = (nFuncList + 1) % MAX_FUNCLIST ] = ORG_Work->act, stllmtn[nFuncList] = ORG_Work->stll_mtn ;

#if SOUND_TEST
	/* voice test */
	if ( ORG_DebugFlag & 1 )
	{
	    static int voice_id = 0 ;
	    static int voice_ids[] = 
	    {
		ORGA_VO_THROUGH_SGR0, //オルガ、手榴弾投げる「食らえ！」
		ORGA_VO_THROUGH_SGR1, //オルガ、気合「ィヤッ！」
		ORGA_VO_THROUGH_SGR2, //オルガ、マスト切り気合「ティッ！」

		ORGA_VO_DAMAGE0,  //オルガ、ダメージ１「ウッ！」
		ORGA_VO_DAMAGE1,  //オルガ、ダメージ２「アァッ！」
		ORGA_VO_PASTOUT,  //オルガ、気を失うやられ「ア…ッアハゥ」

		ORGA_VO_SHOTTEASE0, //オルガ、「やるわねぇ」
		ORGA_VO_SHOTTEASE1, //オルガ、「まだよ！」
		  
		ORGA_VO_HIDETEASE0, //オルガ、「隠れてるだけ！？」
		ORGA_VO_HIDETEASE1, //オルガ、「たいした事無いわね！」
		ORGA_VO_HIDETEASE2, //オルガ、「出て来なさい！」
		  
		ORGA_VO_FINDTEASE0, //オルガ、「丸見えよ！」
		ORGA_VO_FINDTEASE1, //オルガ、「そこね！」
		ORGA_VO_FINDTEASE2, //オルガ、「逃げてるつもり？」

		ORGA_VO_MISSTEASE0, //オルガ、「その程度？」
		ORGA_VO_MISSTEASE1, //オルガ、「どこを狙ってるの？」

		ORGA_VO_TALK0,  //オルガ、「私は生まれた時から…」
		ORGA_VO_TALK1,  //オルガ、「戦場の中で私は育った」
		ORGA_VO_TALK2,  //オルガ、「戦いと勝利が私を育てた」
		ORGA_VO_TALK3,  //オルガ、「部隊と共に私は戦い…」
		ORGA_VO_TALK4,  //オルガ、「生も死も全てを分かち…」
		ORGA_VO_TALK5,  //オルガ、「部隊の他には家族も…」
		ORGA_VO_TALK6,  //オルガ、「部隊は私の全てだ」
		ORGA_VO_TALK7,  //オルガ、「お前が誰であろうと…」

		ORGA_VO_AIRJUMP, //オルガ、マスト切り気合「ティッ！」
	       
		ORGA_VO_UTTER0 , //オルガ、気合「ィヤッ！」
		ORGA_VO_UTTER1 , //オルガ、舌打ち「チィッ！」
	       
		ORGA_VO_TIRED  , //オルガ、息切れ「ッッハァハァ」
	       
		ORGA_VO_FOUND  , //オルガ、見つかった！？「ハッ！」

		ORGA_SE_BRK_LIGHT,   //投光器破壊音
		ORGA_SE_BLW_HOLO1,   //布めくれ
		ORGA_SE_BLW_HOLO2,   //布バタバタ2
		ORGA_SE_BLW_HOLO3,   //布バタバタ3
		   
		ORGA_SE_WLL_SCAR1,   //オルガ戦用金属跳弾（鉄板、鉄柵）
		ORGA_SE_WLL_SCAR2,   //オルガ戦用鉄板跳弾（コンテナ）
		ORGA_SE_WLL_SCAR3,   //オルガ戦用通常跳弾（床、壁）
		ORGA_SE_WLL_SCAR4 ,  //オルガ戦用木製跳弾（木箱）
	    } ;

	    if ( GV_PadData[1].press & PAD_U )
		voice_id++, printf( "%d\n", voice_id ) ;
	    if ( GV_PadData[1].press & PAD_D )
		voice_id--, printf( "%d\n", voice_id ) ;
	    if ( voice_id >= sizeof(voice_ids)/sizeof(int) )
		voice_id = 0 ;
	    if ( voice_id < 0 )
		voice_id = sizeof(voice_ids)/sizeof(int)-1 ;
	    if ( GV_PadData[1].press & PAD_A )
		GM_SeSetMode( voice_ids[voice_id], &ORG_Work->control.mov, GM_SEMODE_BOMB ) ;
	}
#endif

	if ( GV_PadData[1].press & PAD_L1 || ORG_DebugFlag & 2 )
	{
	    printf( "\n-----------OrgaInfo DebugDump-----------\n" ) ;
	    printf( "sizeof(Work)=%d sizeof(PART)=%d sizeof(HIDE)=%d \n",
		    sizeof(Work), sizeof(PART), sizeof(HIDE) ) ;

	    if ( ORG_Work )
	    {
		printf( "Control mov%.1f %.1f %.1f\n",
			ORG_Work->control.mov.vx , ORG_Work->control.mov.vy , ORG_Work->control.mov.vz
		    ) ;
		printf( "        rot  %4d %4d %4d\n        turn %4d %4d %4d \n",
			ORG_Work->control.rot.vx , ORG_Work->control.rot.vy , ORG_Work->control.rot.vz ,
			ORG_Work->control.turn.vx , ORG_Work->control.turn.vy , ORG_Work->control.turn.vz 
		    ) ;
		printf( "Object layer0 mtn%d plytime%f flag%x\n       layer1 mtn%d plytime%f flag %x\n",
			ORG_Work->body.m_ctrl->mt3_ctrl[0].motion_num,
			ORG_Work->body.m_ctrl->mt3_ctrl[0].play_time,
			ORG_Work->body.m_ctrl->mt3_ctrl[0].flag,
			ORG_Work->body.m_ctrl->mt3_ctrl[1].motion_num,
			ORG_Work->body.m_ctrl->mt3_ctrl[1].play_time,
			ORG_Work->body.m_ctrl->mt3_ctrl[1].flag
		    ) ;
		printf( "PosAdj %.1f %.1f %.1f  w%.0f \n",
			ORG_Work->pos_adj.vx , ORG_Work->pos_adj.vy , ORG_Work->pos_adj.vz, ORG_Work->pos_adj.vw 
		    ) ;
		printf( "  Flag %08x\n  AvoidFlag %04x\n  AimFlag %04x\n  ActFlag %04x\n",
			ORG_Work->flag,  ORG_Work->avoid_flg, ORG_Work->aim_flg,  ORG_Work->act_flg
		    ) ;

		printf( "Rage %d  Scared %d\n",  ORG_Work->rage,  ORG_Work->scared    ) ;
#if 1
		if ( ORG_Work->act )
		{
		    for( j=0, i=nFuncList ; j<MAX_FUNCLIST ; j++, i=(i+(MAX_FUNCLIST-1))%MAX_FUNCLIST )
		    {
			for ( lst=ORG_ActionFunctionLists ; lst->adr ; lst++ )
			    if ( FuncList[i] )
				if ( *FuncList[i] == lst->adr )
				    break ;
			printf( "%2d Act %s ", j, lst->name ) ;
			for ( lst=ORG_ActionFunctionLists ; lst->adr ; lst++ )
			    if ( FuncList[i] )
				if ( FuncList[i] == lst->adr )
				    break ;
			if ( strcmp( lst->name, "Undefined Function" ) )
			    printf( "[%s]", lst->name ) ;
			printf( "%d\n", stllmtn[i] ) ;
		    }
		}
#endif

		printf( "StllMtn %d StllTim %d WaitSec %d Vitality %d\n",
			ORG_Work->stll_mtn, ORG_Work->stll_tim, ORG_Work->wait_sec, ORG_Work->vitality
		    ) ;

		printf( "NonDmg %d BitDmg %d\n",  ORG_Work->non_dmg, ORG_Work->bit_dmg   ) ;

		printf( " WeapBlt %d\n WeapUsp %.0f %.0f %.0f %.0f\n"
			" WeapAmo %d\n WeapSgr %.0f %.0f %.0f %.0f\n",
			ORG_Work->weap_blt,
			ORG_Work->weap_usp.vx,ORG_Work->weap_usp.vy,ORG_Work->weap_usp.vz,ORG_Work->weap_usp.vw,
			ORG_Work->weap_amo,
			ORG_Work->weap_sgr.vx,ORG_Work->weap_sgr.vy,ORG_Work->weap_sgr.vz,ORG_Work->weap_sgr.vw
		    ) ;


		if ( ORG_Work->trgt_eye )
		    printf( "TrgtEye %.0f %.0f %.0f\n",
			    ORG_Work->trgt_eye->vx,ORG_Work->trgt_eye->vy,ORG_Work->trgt_eye->vz ) ;
		if (ORG_Work->trgt_aim )
		    printf( "TrgtAim %.0f %.0f %.0f\n",
			    ORG_Work->trgt_aim->vx,ORG_Work->trgt_aim->vy,ORG_Work->trgt_aim->vz ) ;
		printf( "TrgtAimPOS %.0f %.0f %.0f\n",
			ORG_Work->trgt_aim_pos.vx,ORG_Work->trgt_aim_pos.vy,ORG_Work->trgt_aim_pos.vz ) ;
		if (ORG_Work->trgt_pos )
		    printf( "TrgtPos %.0f %.0f %.0f\n",
			    ORG_Work->trgt_pos->vx,ORG_Work->trgt_pos->vy,ORG_Work->trgt_pos->vz ) ;
		if (ORG_Work->trgt_nxt )
		    printf( "TrgtNxt %.0f %.0f %.0f\n",
			    ORG_Work->trgt_nxt->vx,ORG_Work->trgt_nxt->vy,ORG_Work->trgt_nxt->vz ) ;
		printf( "TrgtHid %.0f %.0f %.0f\n",
			ORG_Work->trgt_hid.vx,ORG_Work->trgt_hid.vy,ORG_Work->trgt_hid.vz ) ;
		printf( "TrgtHist " ) ;
		for( i=0 ; i<ORGA_N_ROUTE_HIST ; i++ ) 
		    printf( "%d ", ORG_Work->trgt_his[i] ) ;
		printf( "\n" ) ;
		printf( " TrgtNHist %d\n", ORG_Work->n_trgt_his ) ;
		printf( "TrgtNse %.0f %.0f %.0f %.1f\n",
			ORG_Work->trgt_nse.vx,ORG_Work->trgt_nse.vy,ORG_Work->trgt_nse.vz,ORG_Work->trgt_nse.vw ) ;

	    
		printf( " HideHist " ) ;
		for( i=0 ; i<ORGA_N_HIDE_HIST ; i++ ) 
		    printf( ORG_Work->hide_hist[i]==&ORG_Work->hide_pool[0] ? "ID0 " :
			    ORG_Work->hide_hist[i]==&ORG_Work->hide_pool[1] ? "ID1 " :
			    ORG_Work->hide_hist[i]==&ORG_Work->hide_pool[2] ? "ID2 " : "IDx " ) ;
		printf( "\n" ) ;
		printf( " HideStat %d", ORG_Work->hide_stat ) ;
		printf( " HideSpot %s\n",
			ORG_Work->hide_spot==&ORG_Work->hide_pool[0] ? "ID0 " :
			ORG_Work->hide_spot==&ORG_Work->hide_pool[1] ? "ID1 " :
			ORG_Work->hide_spot==&ORG_Work->hide_pool[2] ? "ID2 " : "IDx " ) ;
		printf( " Exposed %d\n", ORG_Work->exposed ) ;
		printf( "Misc HoloStat %d\n", ORG_Work->misc_holo_stat ? *ORG_Work->misc_holo_stat : -1) ;
		printf( "Misc SpotStat %d\n", ORG_Work->misc_spot_stat ? *ORG_Work->misc_spot_stat : -1) ;
	    }

	    printf( "\n-----------PlayerInfo DebugDump-----------\n" ) ;
	    printf( "Control mov%.1f %.1f %.1f\n",
		    GM_PlayerControl->mov.vx , GM_PlayerControl->mov.vy , GM_PlayerControl->mov.vz
		) ;
	    printf( "        rot  %4d %4d %4d\n        turn %4d %4d %4d \n",
		    GM_PlayerControl->rot.vx , GM_PlayerControl->rot.vy , GM_PlayerControl->rot.vz ,
		    GM_PlayerControl->turn.vx, GM_PlayerControl->turn.vy, GM_PlayerControl->turn.vz 
		) ;
	    printf( "Status %08x \n", GM_PlayerStatus ) ;
	    printf( "Waist Diff %f\n", ORGA_PLY_CAMERAPOS.vy - GM_PlayerControl->mov.vy ) ;

	    /* プレイヤー情報 */
	    printf( "  ply_shoot %d\n",ORG_Work->ply_shoot) ;
	    printf( "  ply_time  %d\n",ORG_Work->ply_time );
	    printf( "  ply_hide  HIDING%d HIDLNG%d HIDTIM%d HIDEXP%d HIDPOS%d\n",
		    (ORG_Work->ply_hide & ORGA_F_PLYR_HIDING)>>15,
		    (ORG_Work->ply_hide & ORGA_F_PLYR_HIDLNG)>>14,
		    (ORG_Work->ply_hide & ORGA_F_PLYR_HIDTIM),
		    (ORG_Work->ply_hide & ORGA_F_PLYR_HIDEXP)>>12,
		    (ORG_Work->ply_hide & ORGA_F_PLYR_HIDPOS)>>6 ) ;
	    printf( "  ply_stop  %d\n",ORG_Work->ply_stop );
	    printf( "  ply_locate%08x\n",ORG_Work->ply_locate);

	}
    }
    ORG_DebugFlag &= ~2 ;
}

void *NewOrgaDebugDump( int name, int where )
{
    DbgWork *work ;

    if ( (work = (DbgWork *)GV_NewActor( GV_ACTOR_PREV2, sizeof( DbgWork ) )) )
    {
	GV_SetActor( &work->actor, Act, NULL ) ;
	work->name = name ;
    }
    return work ;
}


/*
  Display parameters
 */
void ORG_DispParams( Work *work )
{
    //int  i, j ;
    struct dbglst_t *lst ;
    static char func_name[128] = { 0 } ;

    DEBUG_Locate( 260, 12, 0 ) ;
    DEBUG_Printf( "ACT%08x FLG%08x\n", work->act_flg, work->flag ) ;

    if ( work->ply_hide & ORGA_F_PLYR_HIDING )
	DEBUG_Printf( "PLAYER HIDE %d ID%d ",
		     work->ply_hide & ORGA_F_PLYR_HIDTIM,
		     (work->ply_hide & ORGA_F_PLYR_HIDPOS)>>6 ) ;
    else
	DEBUG_Printf( "PLAYER HIDEEXP %d", (work->ply_hide & ORGA_F_PLYR_HIDEXP) ) ;


    {
	extern int hid_id ;/*DEBUG*/
	DEBUG_Printf( "HID%d\n", hid_id ) ;
    }

    if ( work->flag & ORGA_F_MODIST )
	DEBUG_Printf( "CAREFULL " ) ;
    else if ( work->flag & ORGA_F_OFFENCE )
	DEBUG_Printf( "ATTACK   " ) ;
    else			 
	DEBUG_Printf( "MOVE     " ) ;
    DEBUG_Printf( "Time%d", work->act_time ) ;
    DEBUG_Printf( work->flag&ORGA_F_NOMISS_SHOT ? "NOMISS\n" : "\n" ) ;

    DEBUG_Printf( "STLL_TIM%d ", work->stll_tim ) ;

    {
	extern int ORG_PrevAction ;
	DEBUG_Printf( "DbgAct%d ", ORG_PrevAction ) ;
    }

    DEBUG_Printf( "PlyLoc%x\n", work->ply_locate ) ;

    /* 現在の関数を表示 */
    for ( lst=ORG_ActionFunctionLists ; lst->adr ; lst++ )
	if ( lst->adr == ORG_Work->act )
	{
	    strcpy( func_name, lst->name ) ;
	    break ;
	}
    if ( strlen( func_name ) > 1 )
	DEBUG_Printf( "Act %s ", func_name ) ;

#if 0
    {
	FVECTOR *pos = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
	DEBUG_Printf( "RecgPos %d %d %d\n", (int)pos->vx, (int)pos->vy, (int)pos->vz ) ;
    }
#endif
}


int NewCom_EatMemory()
{
    int *cur, *prv ;

    prv = NULL ;
    while( (cur = GV_Malloc( 1024*10 )) )
	prv = cur ;
    if ( prv )
	GV_Free( prv ) ;
    return 1 ;
}


#endif  /*一番上の DEBUG_MODE */
