//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_sys.c
	2002/02/12 K.Sigeno
	$Id: vr_sys.c,v 1.5 2003/01/09 14:32:12 Yoshizawa1 Exp $
*/

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
#include <libvu0.h>

#endif

#include	"gameheader.h"
#include	"vr.h"

#include "BP_Misc.h"

extern void *NewSIG_3DPrintf(int) ;

//#include	"../../../../scn/vr_stage_id.h"
//#include	"vr_stage_id.h"


#ifdef DEBUG_MODE
#include	"debugmenu.h"

#define DEBUG_VR_NOP (0x0000)
#define DEBUG_VR_FLAG_VIEW (0x0001)

enum {
	VR_ST_VIEW_OFF = 0 ,
	VR_ST_VIEW_ON,
} ;

static int vr_sys_debug ;
#ifdef PSX2
static GM_DEBUG_MENU vr_sys_debug_menu = {
	class:	"VR SYS",
	menu:	"FLAG VIEW",
	max:    2,
	items:  (char * []){ "OFF", "ON" },
	values: (int []){ DEBUG_VR_NOP, DEBUG_VR_FLAG_VIEW },
	target: &vr_sys_debug,	// intの変数へのポインタ。代入される。
	mask:   0xffffffff,
	type:   0,
} ;
#else
static char *vr_sys_event_items[] = { "OFF", "ON" }; /*items*/
static int  vr_sys_event_values[] = { DEBUG_VR_NOP, DEBUG_VR_FLAG_VIEW} ;/*values*/
static GM_DEBUG_MENU vr_sys_debug_menu = {
	NULL, /* next */
	"VR SYS", /* class */
	"FLAG VIEW", /* menu */
	vr_sys_event_items,/*items*/
	vr_sys_event_values, /* values */
 	&vr_sys_debug ,/*target*/
	0xffffffff,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

#endif


static char *status_event_items[] = { "OFF", "ON"}; /*items*/
static int  status_event_values[] = { VR_ST_VIEW_OFF, VR_ST_VIEW_ON} ;/*values*/
//static int status_debug_flag = 0 ;
int status_debug_flag = 0 ;;
static GM_DEBUG_MENU status_debug_menu = {
	NULL, /* next */
	"VR SYS", /* class */
	"COUNTER VIEW", /* menu */
	status_event_items,/*items*/
	status_event_values, /* values */
 	&status_debug_flag ,/*target*/
	0xffffffff,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

static void VR_SYS_DebugSet(void){
	GM_AddDebugMenu(&vr_sys_debug_menu);
	GM_AddDebugMenu(&status_debug_menu) ;
	vr_sys_debug = 0 ;
}
#endif


/*ルール*/
enum {
	VR_SYS_GOAL_TARGET	= 0,/*的全滅でゴールが開く*/
	VR_SYS_GOAL_FREE	= 1,/*最初からゴールが開いている*/
	VR_SYS_GOAL_ENEMY	= 2,/*敵全滅でゴールが開く*/
	VR_SYS_SNEAK		= 3,/*潜入モード中はゴールが開く*/
	VR_SYS_TIME			= 4,/*タイマーがゼロになるとゴールが開く*/
	VR_SYS_SCN			= 5,/*敵兵行動不能でゴールが開く*/
	VR_SYS_NO_GOAL		= 6,/*ゴール存在しない*/
	VR_SYS_DEFUSE_BOMB	= 7,/*爆弾解体でクリア*/
};

static int	VR_TIMER_STATUS ;
enum {
	VR_TIMER_ST_NORMAL 	= 0x0000 ,
	VR_TIMER_ST_PAUSE 	= 0x0001 ,
};

void VR_TimerPause(void){
	VR_TIMER_STATUS |= VR_TIMER_ST_PAUSE ;
}
void VR_TimerStart(void){
	VR_TIMER_STATUS &= ~VR_TIMER_ST_PAUSE ;
}

/*追加ルール*/
#define	VR_RULE_FLAG			(0xff00)
#define VR_SYS_COUNT_DOWN		(0x0100)	/*タイマーはカウントダウン*/
#define VR_TIME_OVER			(0x0200)	/*タイマーゼロでゲームオーバー処理*/
#define VR_NO_GOAL_SE			(0x0400)	/*タイマーゼロでゲームオーバー処理*/


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );

#endif


int	VR_SCORE ;
int	VR_GOAL_FLAG ;
int	VR_TIME ;
int	VR_TIME_Original ;
int VR_TARGET_NUM ;
int VR_TARGET_MAX ;


int VR_BOMBS_NUM ;
int VR_BOMBS_MAX ;


int VR_TARGET01_LIFE_MAX ;
int VR_WALL_LIFE_MAX ;
int VR_CUBE_LIFE_MAX ;
int VR_COMBO_CNT  ;
int VR_COMBO_LIMIT  ;
int VR_COMBO_RATE  ;

int VR_COMBO_CHAIN  ;
int VR_COMBO_CHAIN_MAX  ;

int VR_TARGET_HIDE_CYCLE ;
int VR_TARGET_APPEAR_CYCLE ;


#if 0
int		VR_ENEMY_NUM ;
int		VR_ENEMY_MAX ;
//VRキルカウント＆見つかったカウント
int		VR_DiscoverCount ;
int		VR_KillCount ;
void VR_SurrenderEnemy(void){
	VR_ENEMY_NUM++ ;
}
void VR_AddEnemy(void){
	VR_ENEMY_MAX++ ;
}
#endif

int		VR_RetryCount ;
int		VR_ContinueCount ;

float	VR_TARGET01_SCALE ;

float	VR_TRG_SPEED_RATE ;

int		VR_BLAST_RANGE ;

int		VR_STAGE_ID	;

int	VR_TRG_STRENGTH_01[VR_TRG_TYPE_MAX] ;
int	VR_TRG_STRENGTH_02[VR_TRG_TYPE_MAX] ;
int	VR_TRG_STRENGTH_03[VR_TRG_TYPE_MAX] ;

int	VR_TRG_SCORE_01[VR_TRG_TYPE_MAX] ;
int	VR_TRG_SCORE_02[VR_TRG_TYPE_MAX] ;
int	VR_TRG_SCORE_03[VR_TRG_TYPE_MAX] ;

int	VR_TRG_SCORE_NG ;

int		VR_COMBO_SC_LIST[COMBO_DEFAULT_LIMIT] ;
int		VR_COMBO_LIST_MAX ;

void *VR_TRG_COMB_DISP ;



#ifdef DEBUG_MODE
#include	"debugmenu.h"

#define DEBUG_TMR_NOP (0x0000)
#define DEBUG_TMR_5SEC (0x0001)
#define DEBUG_TMR_3MIN (0x0002)
#define DEBUG_TMR_10HOUR (0x0004)

static int timer_debug_flag;
#ifdef PSX2
static GM_DEBUG_MENU debug_menu = {
	class:	"VR TIMER",
	menu:	"LEFT TIME",
	max:    4,
	items:  (char * []){ "NORMAL", "5 SECS", "3 MINS ", "10 HOURS " },
	values: (int []){ DEBUG_TMR_NOP, DEBUG_TMR_5SEC , DEBUG_TMR_3MIN, DEBUG_TMR_10HOUR},
	target: &timer_debug_flag,	// intの変数へのポインタ。代入される。
	mask:   0xffffffff,
	type:   0,
} ;
#else
static char *event_items[] = { "NORMAL", "5 SECS", "3 MINS ", "10 HOURS " }; /*items*/
static int  event_values[] = { DEBUG_TMR_NOP, DEBUG_TMR_5SEC , DEBUG_TMR_3MIN, DEBUG_TMR_10HOUR} ;/*values*/
static GM_DEBUG_MENU debug_menu = {
	NULL, /* next */
	"VR TIMER", /* class */
	"LEFT TIME", /* menu */
	event_items,/*items*/
	event_values, /* values */
 	&timer_debug_flag ,/*target*/
	0xffffffff,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	4, /*max*/
	0, /*current*/
	0  /*padding */
} ;

#endif






static void TimerDebugSet(void){
	GM_AddDebugMenu(&debug_menu);
	timer_debug_flag = 0 ;
}
static void CheckDebugMenu(void){
	if(timer_debug_flag & DEBUG_TMR_5SEC ){
		VR_TIME = DIRECT_TICK(60*5) ;
		timer_debug_flag &= ~DEBUG_TMR_5SEC ;
	}
	if(timer_debug_flag & DEBUG_TMR_3MIN ){
		VR_TIME = DIRECT_TICK(60*60*3) ;
		timer_debug_flag &= ~DEBUG_TMR_3MIN ;
	}
	if(timer_debug_flag & DEBUG_TMR_10HOUR ){
		VR_TIME = DIRECT_TICK(60*60*60*10) ;
		timer_debug_flag &= ~DEBUG_TMR_10HOUR ;
	}

	if(vr_sys_debug == DEBUG_VR_NOP ){
		/*NOP*/
	}else if(vr_sys_debug & DEBUG_VR_FLAG_VIEW ){
		/*フラグ値を表示*/
		MENU_Color( 180,180,180,0x80 );
		MENU_Locate( 50, 50, MENU_MODE_LEFT );
		MENU_Printf( "GM_VRStatus [%x]\n",GM_VRStatus);
		MENU_Locate( 50, 50+15, MENU_MODE_LEFT );
		MENU_Printf( "GM_GameStatus [%x]\n",GM_GameStatus);
	}

	if(status_debug_flag == VR_ST_VIEW_ON){
		MENU_Color( 180,180,180,0x28 );
		MENU_Locate( 50, 270, MENU_MODE_LEFT );
		MENU_Printf( "VR_KillCount [%d]\n",VR_KillCount);
		MENU_Locate( 50, 270+15, MENU_MODE_LEFT );
		MENU_Printf( "VR_DiscoverCount[%d]\n",VR_DiscoverCount);
	}else {
	}

}
#endif

static void VR_InitTargetStrength(void) {
	int i;
	for(i=0;i<VR_TRG_TYPE_MAX;i++){
		VR_TRG_STRENGTH_01[i] = 1 ;
		VR_TRG_STRENGTH_02[i] = 1 ;
		VR_TRG_STRENGTH_03[i] = 1 ;
	}
}
static void VR_InitTargetScore(void) {
	int i;
	for(i=0;i<VR_TRG_TYPE_MAX;i++){
		VR_TRG_SCORE_01[i] = 10 ;
		VR_TRG_SCORE_02[i] = 5 ;
		VR_TRG_SCORE_03[i] = 1 ;
	}
	VR_TRG_SCORE_NG = -10;

}

#define MAX_CPROC	(4)
typedef struct {
	GV_ACT_EX	actor ;
	int			name ;
	int			proc_id ;
	int			goal_proc_id ;
	int			cproc[MAX_CPROC] ;
	int			cproc_time[MAX_CPROC] ;
	int			tab_ene_proc ;
	int inter_s ; /*内部ステータス*/
	int disp_time ; /*表示上の数値*/
	int	rule;
	int	rule_2;
	int	open_delay ;
//	int	combo_limit ;
//	int	debug_cnt ;
} Work;


int	VR_GetVrTime(void)
{
   if ( BP_IsPAL()==TRUE )
	   return (VR_TIME*6)/5 ;
   else
   	return VR_TIME ;
}

void VR_PhotoClear(int score){
	GM_VRStatus |= (GM_VR_CLEAR|GM_VR_POSE_CLEAR) ;
}

void VR_SetScore(int score){
	VR_SCORE = score ;
}
void VR_AddScore(int add){
	VR_SCORE += add ;
	if(VR_SCORE <0) VR_SCORE = 0 ;
}
void VR_AddTarget(void){
	VR_TARGET_MAX++ ;
}
void VR_ClearTarget(void){
	VR_TARGET_NUM++ ;
}
void VR_StageStart(void){
	GM_VRStatus &= ~GM_VR_IDLE ;
}
void VR_StagePause(void){
	GM_VRStatus |= GM_VR_IDLE ;
}
void VR_NoContinue(void){
	GM_VRStatus |= GM_VR_NO_CONTINUE ;
}


static void VR_OpenGoal(Work *work){
	if(VR_GOAL_FLAG & VR_GOAL_OPEN){
		return ;
	}
	VR_GOAL_FLAG |= VR_GOAL_OPEN ;

	if(!(work->rule_2 & VR_NO_GOAL_SE )){
		GM_SdSet( SD_A_V_GOALAP ) ;
	}
}

/*強制ゴールインフラグ*/
void VR_GoalIn(void){
	if ( !GM_IsGameOver( ) ) {
		VR_GOAL_FLAG |= VR_GOAL_IN ;
	}
}
static void VR_DefusedBombCheck(void){
	if((VR_BOMBS_MAX>0)&&(VR_BOMBS_NUM==VR_BOMBS_MAX)){
		VR_GoalIn() ;
	}
}
static void VR_CloseGoal(){
	VR_GOAL_FLAG &= ~VR_GOAL_OPEN ;
}
static void VR_TimeOverFunc(Work *work){
	GV_PadReleaseOn( 0 ) ;
	GM_VRStatus |= GM_VR_GAME_OVER ;
}

static void VR_init(void){
	int	i ;
//	VR_DiscoverCount = 0 ;
//	VR_KillCount = 0 ;

	VR_TARGET_HIDE_CYCLE = 0 ;
	VR_TARGET_APPEAR_CYCLE = 0 ;
	VR_InitTargetStrength() ;
	VR_InitTargetScore() ;
	GM_GameStatus |= STATE_VR_ANOTHER ;
	GM_AnotherPlayer = GM_ANOTHER_PLAYER_SNAKE;
	VR_SCORE = 0;
	VR_GOAL_FLAG = 0;
	VR_TIME = 0;
	VR_TARGET_NUM = 0;
	VR_TARGET_MAX = 0;
	VR_TARGET01_LIFE_MAX = 10;
	VR_WALL_LIFE_MAX = 5;
	VR_CUBE_LIFE_MAX = 5;
	VR_TRG_SPEED_RATE = 1.0f ;
	VR_COMBO_LIMIT = 0;
	VR_COMBO_CNT = 0 ;
	VR_COMBO_RATE = 0 ;
	VR_TARGET01_SCALE = 1.000000f;
	VR_BLAST_RANGE = 3000;
	VR_COMBO_CHAIN = 0 ;
	VR_COMBO_CHAIN_MAX = 0 ;
	VR_STAGE_ID = 0 ;

	VR_ENEMY_NUM = 0;
	VR_ENEMY_MAX = 0;

	VR_BOMBS_NUM = 0;
	VR_BOMBS_MAX = 0;

	for(i=0;i<COMBO_DEFAULT_LIMIT;i++){
		VR_COMBO_SC_LIST[i] = 0 ;
	}
	VR_COMBO_LIST_MAX = 0 ;
	VR_TRG_COMB_DISP = NULL ;
}

//#define VERTS_MAX (64)
//#define VERTS_MAX (128)
//#define VERTS_MAX (256)


static void Act( Work *work )
{
	int i;

#ifdef DEBUG_MODE
	CheckDebugMenu() ;
#endif

	if(GM_VRStatus & (GM_VR_GAME_OVER|GM_VR_CLEAR|GM_VR_IDLE) ){
		/*勝敗が確定したので一切の処理を禁止*/
		/*ゲージ非表示*/
#if 0
		if(GM_VRStatus & (GM_VR_CLEAR) ){
			GM_SetMenuStatus( MENU_GAGE_OFF ) ;
		}
#endif
		return ;
	}
	if ( GM_IsGameOver( ) ) {
		GM_VRStatus |= GM_VR_GAME_OVER ;
		return ;
	}

//printf("work->debug_cnt [%d]\n",work->debug_cnt);
//work->debug_cnt++;
	/*ゴールオープン条件*/
	if(!(VR_GOAL_FLAG & VR_GOAL_OPEN)){
		switch (work->rule){
			case VR_SYS_GOAL_TARGET :
				if(VR_TARGET_NUM >= VR_TARGET_MAX){
					VR_OpenGoal(work);
				}
				break;
			case VR_SYS_GOAL_ENEMY :
				if(GM_VRStatus & GM_VR_ENEMY_END){
					VR_OpenGoal(work);
				}
				break;
			case VR_SYS_TIME :
				if(VR_TIME<=0) {
					VR_OpenGoal(work);
				}
				break;
			case VR_SYS_SNEAK :
				if(	GM_AlertMode == ALERT_MODE_SNEAK ){
					VR_OpenGoal(work);
				}else {
					VR_CloseGoal();
				}
				break;
			case VR_SYS_SCN :
				if(VR_ENEMY_NUM >= VR_ENEMY_MAX){
					if(work->open_delay > 0){
						work->open_delay-- ;
					}else {
						VR_OpenGoal(work);
					}
				}
				break;
			case VR_SYS_DEFUSE_BOMB :
				VR_DefusedBombCheck() ;
				break;
			case VR_SYS_NO_GOAL :
				break;
		}
	}
	/*ゴールオープン時にコールするproc*/
	if(VR_GOAL_FLAG & VR_GOAL_OPEN){
		if(work->goal_proc_id != 0){
			GM_ExecProc( work->goal_proc_id, NULL );
			work->goal_proc_id= 0;
		}
	}

	if(GM_VRStatus & GM_VR_TAB_ENEMY_END){
		//殺してはいけない敵を殺した
		if(work->tab_ene_proc != 0){
			GM_ExecProc( work->tab_ene_proc, NULL );
			work->tab_ene_proc = 0;
			GV_PadReleaseOn( 0 ) ;
			GM_VRStatus |= GM_VR_GAME_OVER ;
			return ;
		}
	}

	/*タイマープロック*/
	for(i=0;i<MAX_CPROC;i++){
		if(work->cproc[i] != 0){
			if(work->cproc_time[i] == VR_TIME){
				GM_ExecProc( work->cproc[i], NULL );
				work->cproc[i]= 0;
			}
		}
	}
	/*タイマーインクリ タイマープロックの後にやるべし*/
	if((!(GM_VRStatus & GM_VR_CLEAR))&&(!(VR_TIMER_STATUS & VR_TIMER_ST_PAUSE))){
		if((work->rule_2&VR_SYS_COUNT_DOWN)||( work->rule  == VR_SYS_TIME)){
			if(VR_TIME>0) {
				VR_TIME-- ;
				if(VR_TIME<=  DIRECT_TICK(60*10)){
					if(((VR_TIME%DIRECT_TICK(60))==0)&&(VR_TIME!=0)){
						GM_SdSet( SD_S_COUNTDW1 ) ;
					}
				}
			}else if(VR_TIME<=0){
				if((work->rule_2 & VR_TIME_OVER)||(work->rule == VR_SYS_DEFUSE_BOMB)){
					VR_TimeOverFunc(work) ;
					GM_SdSet( SD_S_COUNTDW0 ) ;
					return ;
				}
			}
		}else {
			VR_TIME++ ;
		}
	}

	/*爆弾解体ミッション*/
#if 0
	VR_DefusedBombCheck() ;
#endif
	/*ゴールイン およびシナリオ強制ゴールイン*/
	if(VR_GOAL_FLAG & VR_GOAL_IN ){
		if(work->proc_id != 0){
			if ( !GM_IsGameOver( ) ) {
				GM_VRStatus |= GM_VR_CLEAR ;
				GV_PadReleaseOn( 0 ) ;

				GM_ExecProc( work->proc_id, NULL );
				work->proc_id= 0;
			}
		}
		return ;
	}

	if(VR_COMBO_CNT> -1){
		VR_COMBO_CNT-- ;
	}
	if(VR_COMBO_CNT <= -1) {
		VR_COMBO_RATE = 0 ;
		VR_COMBO_CHAIN = 0 ;
	}
}


static void Die( Work *work )
{
}
static int GetResources( Work *work ,int name, int where )
{
	extern void VRFUNC_Set( void ) ;
	int i;
//	NewScoreCount( VR_SCORE ) ;
	VR_init() ;
	VRFUNC_Set( ) ;

	work->open_delay = 0 ;
	if ( GCL_GetOption( 'p' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->proc_id  = GCL_GetNextInt();
		}else {
			work->proc_id  = 0;
		}
	}
	if ( GCL_GetOption( 'm' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			GM_AnotherPlayer = GCL_GetNextInt();
		}
	}

	if ( GCL_GetOption( 'c' ) != NULL ){
		for(i=0;i<MAX_CPROC;i++){
			work->cproc[i] = GCL_GetNextInt();
			work->cproc_time[i] = DIRECT_TICK(GCL_GetNextInt());
			if( GCL_NextStr() == NULL ){
				break ;
			}
		}
	}

	if ( GCL_GetOption( 'g' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->goal_proc_id  = GCL_GetNextInt();
		}else {
			work->goal_proc_id  = 0;
		}
	}
	if ( GCL_GetOption( 'e' ) != NULL ){
		work->tab_ene_proc = GCL_GetNextInt();
	}else {
		work->tab_ene_proc = 0;
	}

	if ( GCL_GetOption( 'r' ) != NULL ){
		work->rule  = GCL_GetNextInt();
	}else {
		work->rule  = VR_SYS_GOAL_TARGET ;
	}
	/*ルール指定からフラグ部分を分離*/
	work->rule_2 = work->rule & VR_RULE_FLAG ;
	work->rule &= ~VR_RULE_FLAG ;

	if(work->rule == VR_SYS_GOAL_FREE){
		/*最初からゴールが開いてる*/
		VR_GOAL_FLAG |= VR_GOAL_OPEN ;
//		VR_OpenGoal() ; /*これで開くとSEなるのでとりあえずフラグ立てのみ*/
	}

	/*敵兵排除ではゴールオープンタイミングを少しずらす*/
	if(work->rule == VR_SYS_SCN){
		work->open_delay = DIRECT_TICK(60) ;
	}

	if ( GCL_GetOption( 't' ) != NULL ){
		if( GCL_NextStr() != NULL )
      {
			VR_TIME  = DIRECT_TICK(GCL_GetNextInt());
         VR_TIME_Original = VR_TIME;
		}
	}
	if ( GCL_GetOption( 's' ) != NULL ){
		i=0 ;
		while (GCL_NextStr() != NULL){
			VR_COMBO_SC_LIST[i] = GCL_GetNextInt();
			i++ ;
		}
		VR_COMBO_LIST_MAX = i;
	}else {
		VR_COMBO_LIST_MAX = 0;
	}


//	NewNegaFilter( 0,0.0f ,0.0f,0,0) ;
	VR_TRG_COMB_DISP = (void *)NewSIG_3DPrintf(COMBO_PRINT_SHIFT)  ;

//	work->debug_cnt = 0;
	VR_TIMER_STATUS = 0 ;
#ifdef DEBUG_MODE
	TimerDebugSet() ;
#endif

#ifdef DEBUG_MODE

	VR_SYS_DebugSet() ;
#endif

	return 0;
}
/*各種変数セットのみを行い、表示系プログラムをリンクしないようにする*/
void *NewVRSys( int name, int where )
{
	Work		*work;
	work = (Work *)GV_CreateActor( GV_ACTOR_PREV, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources( work,name ,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

//	NewScoreCount( 0 ) ;

/****************************************************/

void VR_ExecProcName(int proc_id,int name){
	if(proc_id !=0 ){
		GCL_ARGS	args ;
		int			buf[ 1 ] ;
		args.argv = buf ;
		args.argc = 1 ;
		buf[ 0 ] = name ;
		GM_ExecProc( proc_id, &args );
	}
}

/*シナリオコマンド*/
#define PERM_ST	(GM_VR_EFFECT|GM_VR_ENEMY_POLY_VANISH|GM_VR_EXIT_TO_SPECIAL|GM_VR_PHOTOGRAPH|GM_VR_LOW_BLAST_FX)
/****************/
void SetVRStatus(void){
	GM_VRStatus |= (GCL_GetNextInt()&PERM_ST);

#ifdef KP_WINDOWS
	/* 敵兵体破壊エフェクト抑制(For Test) */
	if ( !DG_CheckUseVRBreakBody() ) {
		
		if(GM_VRStatus & GM_VR_ENEMY_POLY_VANISH){
			GM_VRStatus &= ~GM_VR_ENEMY_POLY_VANISH;	// Test
			GM_VRStatus |= GM_VR_ENEMY_POLY_VANISH_OFF;	// Test
		}else{
			GM_VRStatus &= ~GM_VR_ENEMY_POLY_VANISH_OFF;	// Test
		}
	}
#endif
}
void SetGameStatusVROnly(void){
	GM_GameStatus |= STATE_VR_ONLY ;
}

/*殺してはいけない敵をころしたフラグセット*/
void SetTabEnemyEnd(void){
	GM_VRStatus |= GM_VR_TAB_ENEMY_END ;
}

void SetVRBOMBS(void){
	VR_BOMBS_NUM = GCL_GetNextInt() ;
	VR_BOMBS_MAX = GCL_GetNextInt() ;
}
void SetVR_BOMBS_NUM(void){
	VR_BOMBS_NUM = GCL_GetNextInt() ;
}

void SetVR_ENEKILL_CNT(void){
	VR_KillCount = GCL_GetNextInt() ;
}
void SetVR_DISCOVERY_CNT(void){
	VR_DiscoverCount = GCL_GetNextInt() ;
}

void VR_ScnAddEnemy(void){
	VR_ENEMY_MAX += GCL_GetNextInt();
}
void VR_AddScoreScn(void ){
	VR_SCORE += GCL_GetNextInt();
	if(VR_SCORE <0) VR_SCORE = 0 ;
}
