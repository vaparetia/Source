//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   zako_title.c
   ザコサバイバルスタート画面
   
   2001/09/18	S.Kobayashi
   2002/07/18	Retouch Sigeno
   $Id: zako_title.c,v 1.1.1.3 2002/11/19 11:50:23 Yoshizawa1 Exp $
*/


#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <string.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"

#include    "zako_survival.h"

#define		LAYOUT		  (L2D_zako_survival) // zako_survival.l2d 
#define     STR_DEFAULT   (566267)

#define	ACTOR_PRIO		(254)



//#define KEY_MAX (4)
#define KEY_MAX (6)

// enum -> #define  chaned by T.Morita 2002.05.23
#define	SK_INITIALIZE_OK      		I64(0x0000000000000002)
#define	SK_ACTION_START             I64(0x0000000400000000)
#define	SK_ACTION_OK                I64(0x0000000800000000)
#define	SK_MOVE_OK                  I64(0x0000020000000000)


#define STR_PLAY_NAME        (L2D_zako_survival_OBJ_playerName)
#define STR_PLAY_NAME2        (L2D_zako_survival_OBJ_playerName2)
#define STR_PLAY_FACE        (L2D_zako_survival_OBJ_playerFace)


#define STR_BOSS_NAME        (L2D_zako_survival_OBJ_enemyName)
#define STR_BOSS_NAME2       (L2D_zako_survival_OBJ_enemyName2)
#define STR_BOSS_FACE        (L2D_zako_survival_OBJ_enemyFace)

#define STR_KEY_SNAKE        (6549577) /*faceSNA*/
#define STR_KEY_RAIDEN       (6548145) /*faceRAI*/
#define STR_KEY_MGS1		(13527445) /*faceSNAmgs1*/

#define STR_KEY_SOL			(6549620) /*faceSOL*/
#define STR_KEY_SOLS		(8261343) /*faceSOLS*/
#define STR_KEY_TNG			(6550607) /*faceTNG*/


#define STR_KEY_NAME_SNAKE   (258634)		/*nameSNA*/
#define STR_KEY_NAME_RAIDEN  (257202)		/*nameRAI*/
#define STR_KEY_NAME_MGS1SNAKE  (8073137)	/*nameMGS1*/

#define STR_KEY_NAME2_MGS1SNAKE  (7458951)	/*playerName2-key-4-1*/
#define STR_KEY_ENE_NAME2_TNG		(12516583)	/*enemyName2-key-4-1*/


#define STR_KEY_ENE_NAME_SOL	(258677)	/*nameSOL*/
#define STR_KEY_ENE_NAME_SOLS	(8277747)	/*nameSOLS*/
#define STR_KEY_ENE_NAME_TNG	(259664)	/*nameTNG*/

#define STR_VS_OLGA    (2710445)
#define STR_VS_FATMAN  (12939552)
#define STR_VS_HARRIER (9487338)
#define STR_VS_VAMP    (2928764)
#define STR_VS_RAY     (6378892)
#define STR_VS_SOLIDUS (2897095)

#define STR_OBJ_VS			(3891)
#define STR_OBJ_PLNAME1		(12040484)
#define STR_OBJ_PLNAME2		(16196808)
#define STR_OBJ_ENENAME1		(2169895)
#define STR_OBJ_ENENAME2	(2327830)


typedef struct {
	float   p;
	int     count;
	int     code1;
	int     code2;
	int     morf_parts;
	u_long64  flag;
} Hokan;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	GV_PAD              *pad; 
	int                 action;
	int                 base_u;
	int                 base_v;
	int                 pad_status;
	int                 pad_check;
	int                 proc_prev;
	int                 proc_next;
	int                 name;
	int	delay ;
	u_long64              flag;

	Hokan               boss_pic[ KEY_MAX ];
	int                 boss_name;
	int                 disp_mode;
	int                 loop_count;
	int                 player;               
	int					name_init_flag ;
	void  ( *act )( struct _work * );
} Work ;

static char SK_WakeFlag;

// プロトタイプ
static int AnimationAct( Work * ); // アニメーション
static inline int MorfAct( Work *work , Hokan *pHokan ); // これのみ実行
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan *hokan );
static void KeySetPic( Work *work ); // キーを設定する関数
static void KeySetName( Work *work ); // キーを設定する関数

static void ShowHideControl( int handle_2d , int strcode  , int mode )
{
	SPR_OBJ		*spr;

	spr = L2D_GetObject( handle_2d , strcode );
	if ( spr == NULL ){
		return;
	}
	switch ( mode ){
	case 0 : // hide
		SPR_HIDE( spr );
		break;
	case 1 : // show
		SPR_SHOW( spr );
		break;
	}
}

static void BossNameDisp( Work *work , int flag )
{
	switch ( work->boss_name ){
		case 0 : // 一人兵
			ShowHideControl( work->handle_2d , STR_OBJ_VS , flag );
			break;
		case 1 : // 集団兵
			ShowHideControl( work->handle_2d , 3891 , flag );
			break;
		case 2 : // 天狗＆集団兵
			ShowHideControl( work->handle_2d , 3891 , flag );
			break;
		default : 
			break;
	}
}

static void CharaNameDisp( Work *work , int flag )
{
	ShowHideControl( work->handle_2d , STR_OBJ_VS , flag );
	ShowHideControl( work->handle_2d , STR_OBJ_PLNAME1 , flag );
	ShowHideControl( work->handle_2d , STR_OBJ_ENENAME1 , flag );
	if( work->boss_name == 2){
		ShowHideControl( work->handle_2d , STR_OBJ_ENENAME2 , flag );
	}
	if ( work->player == 2 ){
		ShowHideControl( work->handle_2d , STR_OBJ_PLNAME2 , flag );
	}
}



static int SprInit( Work *work ) // Sprite初期化
{
	if ( work->flag & SK_INITIALIZE_OK ){
		return ( 0 );
	}
//	BossNameDisp( work , 1 );
	CharaNameDisp( work , 1 );
	work->flag |= SK_INITIALIZE_OK;

	return( 0 );
}


static void Act( Work *work )
{
	int i;

//	work->act( work );
	AnimationAct( work );
	if ( work->disp_mode == 0 ){
		// key
		for ( i = 0 ; i < KEY_MAX ; i ++ ){
			MorfAct( work , &work->boss_pic[ i ] );
		}
		if(work->name_init_flag <= 10 ){
			CharaNameDisp( work , 0 );
			work->name_init_flag++ ;
		}else {
			CharaNameDisp( work , 1 );
		}
	}
}

static inline int MorfAct( Work *work , Hokan *pHokan ) // これのみ実行
{
	if ( !( pHokan->flag & SK_MOVE_OK ) ){
		MorfL2d( work->handle_2d , pHokan->morf_parts  , pHokan->code2  , pHokan->code1  , pHokan );
	} else {
		return ( 0 );
	}
	return( 1 );
}

// モーフィングを設定してくれる関数
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan *hokan )
{
	void *parts;

	if ( strcode == 0 ){
		printf("return\n");
		return;
	}
	parts = L2D_GetParts( handle , strcode );    /* パーツポインタの取得 */

	if ( parts == NULL ){
		return;
	}
	hokan->p += ( 1.0f - hokan->p ) / hokan->count;
	hokan->count--;

	L2D_MorfObject( parts , code1 , code2 , hokan->p );
	if ( ( hokan->p == 1.0f ) || ( hokan->count == 0 ) ){
		hokan->flag = SK_MOVE_OK;
	}
}

#define STR_SHOWREADY (13266348)
#define STR_LOOPFIGHT (376345)
#define STR_HIDEFIGHT (101187)
#define STR_SHOWBOSSNAME (11663840)
#define STR_SHOWWIN (2950949)
#define STR_LOOPWIN (2721139)
#define STR_HIDEWINNORM (9439987)
#define STR_HIDEWINFINISH (13225207)
static void DefaultNextAction( Work *work )
{
	switch ( work->disp_mode ){
	case 0 :
		DG_Chanl( 0 )->flag = 0;
		work->action = STR_SHOWBOSSNAME;
		KeySetPic( work ); // キーを設定する関数
		KeySetName( work ); // キーを設定する関数
		break;
	case 1 :
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_B_FIGHT1 );
		work->action = STR_LOOPFIGHT;
		break;
	case 2 :
	case 3 :
		work->action = STR_SHOWWIN;
		break;
	}		
}

static void KeySetPic( Work *work ) // キーを設定する関数
{
	work->boss_pic[ 0 ].morf_parts = STR_PLAY_FACE;
	work->boss_pic[ 2 ].morf_parts = STR_BOSS_FACE;
	// Snake or Raiden
	if ( work->player == 0 ){
		// Snake
		work->boss_pic[ 0 ].code1 = STR_KEY_SNAKE;
		work->boss_pic[ 0 ].code2 = STR_KEY_SNAKE;
	}else if ( work->player == 1 ){
		//Raiden
		work->boss_pic[ 0 ].code1 = STR_KEY_RAIDEN;
		work->boss_pic[ 0 ].code2 = STR_KEY_RAIDEN;
	} else {
		//MGS1 Snake
		work->boss_pic[ 0 ].code1 = STR_KEY_MGS1;
		work->boss_pic[ 0 ].code2 = STR_KEY_MGS1;
	}
	// boss
	switch ( work->boss_name ){
		case 0 : // ひとり兵
			work->boss_pic[ 2 ].code1 = STR_KEY_SOL;
			work->boss_pic[ 2 ].code2 = STR_KEY_SOL;
			break;
		case 1 : // 集団兵
			work->boss_pic[ 2 ].code1 = STR_KEY_SOLS;
			work->boss_pic[ 2 ].code2 = STR_KEY_SOLS;
			break;
		case 2 : // 天狗兵
			work->boss_pic[ 2 ].code1 = STR_KEY_TNG;
			work->boss_pic[ 2 ].code2 = STR_KEY_TNG;
		break;
	}
	work->boss_pic[ 0 ].flag = 0;
	work->boss_pic[ 0 ].p = 0.0f;
	work->boss_pic[ 0 ].count = 1;
	work->boss_pic[ 2 ].flag = 0;
	work->boss_pic[ 2 ].p = 0.0f;
	work->boss_pic[ 2 ].count = 1;
}

static void KeySetName( Work *work ) // キーを設定する関数
{
	work->boss_pic[ 1 ].morf_parts = STR_PLAY_NAME;
	work->boss_pic[ 3 ].morf_parts = STR_BOSS_NAME;

	work->boss_pic[ 4 ].morf_parts = STR_PLAY_NAME2;
	work->boss_pic[ 5 ].morf_parts = STR_BOSS_NAME2;


	if ( work->player == 0 ){
		// Snake
		work->boss_pic[ 1 ].code1 = STR_KEY_NAME_SNAKE;
		work->boss_pic[ 1 ].code2 = STR_KEY_NAME_SNAKE;

		work->boss_pic[ 4 ].code1 = STR_KEY_NAME_SNAKE;
		work->boss_pic[ 4 ].code2 = STR_KEY_NAME_SNAKE;

	} else if ( work->player == 1 ){
		//Raiden
		work->boss_pic[ 1 ].code1 = STR_KEY_NAME_RAIDEN;
		work->boss_pic[ 1 ].code2 = STR_KEY_NAME_RAIDEN;

		work->boss_pic[ 4 ].code1 = STR_KEY_NAME_RAIDEN;
		work->boss_pic[ 4 ].code2 = STR_KEY_NAME_RAIDEN;

	} else {
		//MGS1SNAKE
		work->boss_pic[ 1 ].code1 = STR_KEY_NAME_MGS1SNAKE;
		work->boss_pic[ 1 ].code2 = STR_KEY_NAME_MGS1SNAKE;

		work->boss_pic[ 4 ].code1 = STR_KEY_NAME2_MGS1SNAKE;
		work->boss_pic[ 4 ].code2 = STR_KEY_NAME2_MGS1SNAKE;

	}
	// boss
	switch ( work->boss_name ){
		case 0 : // 一人兵
			work->boss_pic[ 3 ].code1 = STR_KEY_ENE_NAME_SOL;
			work->boss_pic[ 3 ].code2 = STR_KEY_ENE_NAME_SOL;

			work->boss_pic[ 5 ].code1 = STR_KEY_ENE_NAME_SOL;
			work->boss_pic[ 5 ].code2 = STR_KEY_ENE_NAME_SOL;

			break;
		case 1 : // 集団兵
			work->boss_pic[ 3 ].code1 = STR_KEY_ENE_NAME_SOLS;
			work->boss_pic[ 3 ].code2 = STR_KEY_ENE_NAME_SOLS;

			work->boss_pic[ 5 ].code1 = STR_KEY_ENE_NAME_SOLS;
			work->boss_pic[ 5 ].code2 = STR_KEY_ENE_NAME_SOLS;

			break;
		case 2 : // 天狗兵
			work->boss_pic[ 3 ].code1 = STR_KEY_ENE_NAME_TNG;
			work->boss_pic[ 3 ].code2 = STR_KEY_ENE_NAME_TNG;

			work->boss_pic[ 5 ].code1 = STR_KEY_ENE_NAME2_TNG;
			work->boss_pic[ 5 ].code2 = STR_KEY_ENE_NAME2_TNG;

			break;
	}
	work->boss_pic[ 1 ].flag = 0;
	work->boss_pic[ 1 ].p = 0.0f;
	work->boss_pic[ 1 ].count = 1;

	work->boss_pic[ 3 ].flag = 0;
	work->boss_pic[ 3 ].p = 0.0f;
	work->boss_pic[ 3 ].count = 1;

	work->boss_pic[ 4 ].flag = 0;
	work->boss_pic[ 4 ].p = 0.0f;
	work->boss_pic[ 4 ].count = 1;

	work->boss_pic[ 5 ].flag = 0;
	work->boss_pic[ 5 ].p = 0.0f;
	work->boss_pic[ 5 ].count = 1;

}

static int AnimationAct( Work *work ) // 通常実行
{
	int status;

//	CharaNameDisp( work , 0 );

	status = L2D_ActionStatus( work->handle_2d );
	if ( ( status != L2D_STAT_BUSY ) && ( work->flag & SK_ACTION_OK ) ){
		switch ( work->action ){
		case STR_DEFAULT : // default
			work->flag |= SK_ACTION_START;
			DefaultNextAction( work );
			break;
		case STR_SHOWBOSSNAME : // default
//			BossNameDisp( work , 0 );
//			CharaNameDisp( work , 0 );
			work->action = STR_SHOWREADY;
			work->flag |= SK_ACTION_START;
			break;
		case STR_SHOWWIN : // default
			work->action = STR_LOOPWIN;
			work->flag |= SK_ACTION_START;
			break;
		case STR_LOOPFIGHT : // default
			if ( work->loop_count < 1 ){
				work->action = STR_LOOPFIGHT;
				work->flag |= SK_ACTION_START;
				work->loop_count++;
			} else {
				work->action = STR_HIDEFIGHT;
				work->flag |= SK_ACTION_START;
			}				
			break;
		case STR_LOOPWIN : // default
			if ( work->loop_count < 1 ){
				work->action = STR_LOOPWIN;
				work->flag |= SK_ACTION_START;
				work->loop_count++;
			} else {
				if ( work->disp_mode == 2 ){ // normal
					work->action = STR_HIDEWINNORM;
					work->flag |= SK_ACTION_START;
				} else { // finish
					work->action = STR_HIDEWINFINISH;
					work->flag |= SK_ACTION_START;
				}
			}				
			break;
		case STR_HIDEFIGHT :
		case STR_HIDEWINNORM :
		case STR_HIDEWINFINISH :
			if ( work->proc_next != 0 ){
				GM_ExecProc( work->proc_next , NULL );
			} else {
				printf("proc none\n");
			}
			GV_DestroyActor( work );
			break;
		case STR_SHOWREADY :
			if ( work->proc_next != 0 ){
//				GV_PauseOffActorSystem( GV_PAUSE_PAUSE ) ;
				GM_ExecProc( work->proc_next , NULL );
			} else {
				printf("proc none\n");
			}
			GV_DestroyActor( work );
			break;
		}
		work->flag &= ~SK_ACTION_OK;
		return ( 0 );
	}else {
		if ( work->action == STR_DEFAULT){
			if ( status == L2D_STAT_BUSY ) {
				printf("L2D_STAT_BUSY \n") ;
			}
			if(!( work->flag & SK_ACTION_OK )) {
				printf("SK_ACTION_OK NG!!!! \n") ;
			}
		}
	}

	if ( (status != L2D_STAT_BUSY) && (work->flag & SK_ACTION_START )){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( work->handle_2d , work->action );
			work->flag |= SK_ACTION_OK;
			work->flag &= ~SK_ACTION_START;
		}
	}

	return ( 0 );
}

#define SIG_SHOWPOINT (11505383)
#define SIG_SD_S_N_START1 (2666147)
#define SIG_SD_S_B_READY1 (3675968)
#define SIG_SD_S_B_FIGHT1 (8070792)
#define SIG_SD_S_TWINKY01 (13497358)
#define SIG_SD_S_TWINKY02 (13497359)
#define SIG_SD_S_LINEMOV1 (4099416)
static void Signal( void *work , int sign , int value )
{
	work = ( Work * )work;

	switch ( sign ){
	case SIG_SHOWPOINT : // node frame action 実行
		SprInit( work );
		break;
	case SIG_SD_S_N_START1 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_N_START1 );
		break;
	case SIG_SD_S_B_READY1 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_B_READY1 );
		break;
	case SIG_SD_S_B_FIGHT1 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_B_FIGHT1 );
		break;
	case SIG_SD_S_TWINKY01 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_TWINKY01 );
		break;
	case SIG_SD_S_TWINKY02 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_TWINKY02 );
		break;
	case SIG_SD_S_LINEMOV1 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_LINEMOV1 );
		break;
	}
}

static	void	Die( Work *work )
{
//	ASSERT(0) ;
	DG_Chanl( 0 )->flag = 1;
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d );
	}
}

/*----------------------------------------------------------------*/
// mode = 0 : ready , 1 : loop fight
static	int	GetResources( Work *work )
{
	int		handle;
	int     i;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout2( LAYOUT , DG_CHANL_MENU , 0 , 0 , GV_PAUSE_STOP ) ; // 後に黒枠がくるため
	if ( handle < 0 ){
		printf("L2D_Load error\n");
		return -1 ;
	}
	work->handle_2d = handle ;

	// シグナルの登録
	// l2d
	L2D_SetSignalHandle( work->handle_2d , work , Signal );

	// etc
	work->action = STR_DEFAULT;
//	work->act = ( void * )AnimationAct;
	work->pad_status = 0;
	work->pad_check = 0;
	work->pad = &GV_PadDataDirect[ 0 ];
	work->loop_count = 0;

	work->disp_mode = GCL_GetOptionValue( 'm' , 0 );
	work->boss_name = GCL_GetOptionValue( 'b' , 0 );
	work->proc_next = GCL_GetOptionValue( 'n' , 0 );
	work->player    = GCL_GetOptionValue( 'p' , 0 );
	work->delay    = DIRECT_TICK(GCL_GetOptionValue( 'd' , 0 ));
	for ( i = 0 ; i < KEY_MAX ; i ++ ){
		work->boss_pic[ i ].flag = 0;	
		work->boss_pic[ i ].p = 0.0f;	
		work->boss_pic[ i ].code1 = 0;	
		work->boss_pic[ i ].code2 = 0;
	}
	work->flag = SK_ACTION_START;

	SK_WakeFlag = 0;
	work->name_init_flag = 0 ;
	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewZakoTelop( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), ACTOR_PRIO ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->name = name;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
