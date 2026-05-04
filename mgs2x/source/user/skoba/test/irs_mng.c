//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	複数赤外線センサー  main.c
	2001/03/14 S.Kobayashi
	$Id: irs_mng.c,v 1.1.1.3 2002/11/19 11:50:24 Yoshizawa1 Exp $
*/
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include    "../../kano/attachment/attachment_called.h"
#include    "../test/etc.h"
#include    "newirs.h"

typedef struct {
	GV_ACT_EX       actor;

	int name;
	int mode;
	int proc[ 2 ];
	int model_name[ 3 ];
	int obj_count;
	int objnum;
	int proc_act_flag;
	char flag;

	FVECTOR sensor_pos; // se 専用

} Work;

typedef struct {
	int *put_name;
	int *point_name;
	int *prt_id;
	int num;
} Attach;

enum {
	SK_SENSER_SE   = 0x1 ,
	SK_PARENT_CALL = 0x02 ,
	SK_SENSER_DIE  = 0x04 ,
	SK_ATTACH_DIE  = 0x08 , 
	SK_ALL_DIE     = 0x10 , 
	SK_SINGLE_DIE  = 0x20 , 
	SK_PARENT_CALL_CHILD_NAME = 0x40 ,
	SK_PROC_FREE   = 0x80 ,
};

void *NewIrsBox( FVECTOR * , SVECTOR * , int * , Attach * , int , int , int , u_long64 );
void *NewSenser( FVECTOR * , SVECTOR * , int , int , int , int , int );

static u_long64 SK_DestroyBoxID; // 破壊されている場所は 1
static int SK_BeforeDestroyStrcode; // 最後に破壊された箱のstrcode

static int SignalFunc( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_PARENT_CALL :
		pWork->obj_count -= 1;
		SK_DestroyBoxID |= value;
		break;
	case SK_PROC_FREE :
		pWork->proc_act_flag = 0;
		break;
	case SK_PARENT_CALL_CHILD_NAME :
		SK_BeforeDestroyStrcode = value;
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		return ( -1 );
	}
	return ( 0 );
}

// メッセージを受けとると殺す敵が死んだ時に利用
static int MsgDie( Work *pWork )
{
	GV_MSG		*msg;
	int      msg_num;
	int          num;
	int         name;

	msg_num = GV_ReceiveMessage( pWork->name , &msg );
	msg += msg_num - 1;
	name = 0;
	while( --msg_num >= 0 ){
		num = msg->message[ 0 ];
		if( num == SAIFA_DIE_FLG ){
			GV_CallChildSignalFunc( pWork , SK_ATTACH_DIE , msg->message[ 1 ] );			
		}
		if( num == ALL_DIE_FLG ){
			pWork->proc_act_flag = 1;
			pWork->flag = 0x1;
			GV_CallChildSignalFunc( pWork , SK_ALL_DIE , 0 );			
			printf("all die-------------------------------------\n");
		}
		if( num == SINGLE_DIE_FLG ){
			pWork->proc_act_flag = 1;
			GV_CallChildSignalFunc( pWork , SK_SINGLE_DIE , msg->message[ 1 ] );			
			printf("single die-------------------------------------\n");
		}
		msg--;
	}
	return ( 0 );
}

static int ParmGet( Work *pWork  , int where )
{
	FVECTOR pos;
	SVECTOR rot;
	Attach  atmp;
	int name;
	int *proc;
	int i , num;

	num = GCL_GetOptionValue( 'o' , 0 );  // スイッチの数取得
	// 領域の確保
	proc = GV_Malloc( ( sizeof( int ) * pWork->objnum ) );
	if ( proc == NULL ){
		return( -1 );
	}
	GV_ZeroMemory( proc , sizeof( int ) * pWork->objnum );
	if ( num != 0 ){ // zero なら付けない
		atmp.put_name = GV_Malloc( ( sizeof( int ) * num ) );	
		if ( atmp.put_name == NULL ){
			GV_DelayedFree( proc );
			atmp.point_name = NULL;
			atmp.prt_id = NULL;
			return ( -1 );
		}
		atmp.point_name = GV_Malloc( ( sizeof( int ) * num ) );	
		if ( atmp.point_name == NULL ){
			GV_DelayedFree( proc );
			GV_DelayedFree( atmp.put_name );
			atmp.prt_id = NULL;
			return ( -1 );
		}
		atmp.prt_id = GV_Malloc( ( sizeof( int ) * num ) );	
		if ( atmp.prt_id == NULL ){
			GV_DelayedFree( proc );
			GV_DelayedFree( atmp.put_name );
			GV_DelayedFree( atmp.point_name );
			return ( -1 );
		}
		GV_ZeroMemory( atmp.put_name , sizeof( int ) * num );
		GV_ZeroMemory( atmp.point_name , sizeof( int ) * num );
		GV_ZeroMemory( atmp.prt_id , sizeof( int ) * num );

		i = 0;
		GCL_GetOption( 'l' );
		while( GCL_NextStr() != NULL && i < num) { // 登録オブジェクトを越えるようならそれ以上入力しない
			atmp.put_name[ i ] = GCL_GetNextInt();
			atmp.point_name[ i ] = GCL_GetNextInt();
			atmp.prt_id[ i ] = GCL_GetNextInt();
			i++;
		}
		atmp.num = i;
	} else {
		atmp.put_name = NULL;
		atmp.point_name = NULL;
		atmp.prt_id = NULL;
		atmp.num = 0;
	}
	i = 0;
	GCL_GetOption( 'm' );
	while( GCL_NextStr() != NULL && i < MODEL_MAX ){
		pWork->model_name[ i ] = GCL_GetNextInt();          // モデルNAMEの取得
		i++;
	}
	// 個別procの取得
	i = 0;
	if ( GCL_GetOption( 'G' ) != NULL ){
		while ( GCL_NextStr() != NULL && i < pWork->objnum ){
			proc[ i ] = GCL_GetNextInt();
			i++;
		}
	}
	// box登録
	i = 0;
	GCL_GetOption( 'p' );
	while( GCL_NextStr() != NULL && i < pWork->objnum ){
		pos.vx = GCL_GetNextInt();	
		pos.vy = GCL_GetNextInt();	
		pos.vz = GCL_GetNextInt();
		rot.vx = GCL_GetNextInt();
		rot.vy = GCL_GetNextInt();
		rot.vz = GCL_GetNextInt();
		name   = GCL_GetNextInt();          // NAMEの取得
		GV_SetActorChild( pWork , NewIrsBox( &pos , &rot , pWork->model_name , &atmp , name , where , proc[ i ] , I64(1) << i ) );
		i++;
	}
	// 解放
	if ( atmp.num != 0 ){
		GV_DelayedFree( atmp.put_name );
		GV_DelayedFree( atmp.point_name );
		GV_DelayedFree( atmp.prt_id );
	}
	GV_DelayedFree( proc );

	return (0);
}

static int InitSenser( Work *pWork , int name , int where )
{
	FVECTOR *pFtmp;
	SVECTOR *pStmp;
	int *pMode;
	int i;
	int num;
	int tmp;

	num = GCL_GetOptionValue( 'b' , 0 );
	if ( num == 0 ){
#ifdef DEBUG
		printf("sensor_num?\n");
#endif
		return ( -1 );
	}
	pFtmp = GV_Malloc( ( sizeof( FVECTOR ) * ( num * N_IRS_SENSOR ) ) );	
	pStmp = GV_Malloc( ( sizeof( SVECTOR ) * ( num * N_IRS_SENSOR ) ) );	
	pMode = GV_Malloc( ( sizeof( int ) * num ) );	
	// 下の赤外線をつけるかどうか
	i = 0;
	GCL_GetOption( 'd' );
	while( GCL_NextStr() != NULL || i < num ){
		tmp = GCL_GetNextInt();
		pMode[ i ] = tmp;
#ifdef DEBUG
		printf("sensor open\n");
#endif
		i++;
	}
	if ( i < num ){ // 補正
		pMode[ i ] = 0;
#ifdef DEBUG
		printf("sensor close\n");
#endif
		i++;
	}
	i = 0;
	GCL_GetOption( 'a' );
	while( GCL_NextStr() != NULL || i < N_IRS_SENSOR * num){
		pFtmp[ i ].vx = GCL_GetNextInt();	
		pFtmp[ i ].vy = GCL_GetNextInt();	
		pFtmp[ i ].vz = GCL_GetNextInt();
		pStmp[ i ].vx = GCL_GetNextInt();
		pStmp[ i ].vy = GCL_GetNextInt();
		pStmp[ i ].vz = GCL_GetNextInt();
		i++;
	}
	for ( i = 0 ; i < num ; i++ ){
		GV_SetActorChild( pWork , NewSenser( &pFtmp[ (i * N_IRS_SENSOR) ] , &pStmp[ (i * N_IRS_SENSOR) ] ,
						  pWork->proc[ 0 ] , pMode[ i ]  , pWork->model_name[ ModelSenser ] ,  name , where ) );
	}
	// 解放
	GV_DelayedFree( pFtmp );
	GV_DelayedFree( pStmp );
	GV_DelayedFree( pMode );
	return ( 0 );
}
//		if(GCL_GetOption('d') != NULL){

static int GetResources( Work *pWork , int name , int where )
{
	// scn
	SK_DestroyBoxID = 0;
	SK_BeforeDestroyStrcode = 0;
	// main
	pWork->name = name;
	pWork->proc_act_flag = 0;
	pWork->flag = 0x0;
	pWork->proc[ 0 ] = GCL_GetOptionValue( 'e' , 0 ); // 赤外線接触時のproc
	pWork->proc[ 1 ] = GCL_GetOptionValue( 's' , 0 ); // スイッチ破壊時のproc

	pWork->obj_count = pWork->objnum = GCL_GetOptionValue( 'n' , 0 );  // スイッチの数取得

	// シグナルの登録
	GV_SetActorSignalFunc( pWork , SignalFunc );
	// ボックスの初期化
	if ( ParmGet( pWork  , where ) < 0 ){
		return ( -1 );
	}
	if ( InitSenser( pWork , name , where ) < 0 ){
		return ( -1 );
	}

	return 0;
}

static inline void IrsCrush( Work *pWork ) // スイッチをすべて破壊した時のプロシージャコール
{
	if( pWork->obj_count > 0 || pWork->proc_act_flag == 1 ){
		return;
	}
	if( pWork->proc[ 1 ] != 0 ){
		GV_CallChildSignalFunc( pWork , SK_SENSER_DIE , 0 );
		GCL_ExecProc( pWork->proc[ 1 ] , NULL );
		// SenserLamp SE
		if ( !( pWork->flag & 0x1 ) ){
			GV_CallChildSignalFunc( pWork , SK_SENSER_SE , 0 );
		}
//		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_A_OFFSENS1 );
		pWork->proc[ 1 ] = 0;
	}
}


static void Act( Work *pWork )
{
	MsgDie( pWork );
	IrsCrush( pWork );
}

static void Die( Work *pWork )
{
}

void *NewIrs2( int name , int where )
{
	Work *pWork;

	pWork = ( Work * )GV_NewActor( GV_ACTOR_USER , sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &pWork->actor , Act , Die );
		GV_ActorEX( &pWork->actor );
		if ( GetResources( pWork , name , where ) != 0 ) {
			GV_DestroyActor( pWork );
			return NULL;
		}
	}
	return pWork;
}

// シナリオcommand
int SK_ScnDestoryBox( void ) // その番号の物が壊されていたら 1
{
	u_long64 num;

	num = GCL_GetOptionValue( 'n' , 0 );
	if ( SK_DestroyBoxID & ( I64(1) << num ) ) {
		return ( 1 );
	}
	return ( 0 );
}

int SK_ScnBeforeDestroyBox( void ) // 最後に壊された箱のstrcodeをかえす
{
	return ( SK_BeforeDestroyStrcode );
}

