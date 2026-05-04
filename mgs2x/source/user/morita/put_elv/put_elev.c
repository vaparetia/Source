//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_elev.c 
   昇降機

   2001/01/16 T.Morita
   $Id: put_elev.c,v 1.1.1.3 2002/11/19 11:46:31 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#include "../include/util.h"
#include "../include/libdg_x.h"
#include "../brk_utl/brk_utl.x"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct objs_t
{
    CONTROL  control ;
    OBJECT   body    ;
    
    FVECTOR  pos  ;
    FVECTOR  vel  ;
    SVECTOR  rot  ;
} OBJS ;

enum flag_t
{
    PUT_ELEV_USE_IN_W11  = 0x0001,
    PUT_ELEV_USE_IN_W12  = 0x0002,
    PUT_ELEV_CLEAR_STAT  = 0x3003,

    PUT_ELEV_GO_UP       = 0x0010,
    PUT_ELEV_GO_DOWN     = 0x0020,
    PUT_ELEV_STOP_MOTION = 0x0040,

    PUT_ELEV_OPEN_DOOR   = 0x0080,
    PUT_ELEV_CLOSE_DOOR  = 0x0100,
    PUT_ELEV_STOP_DOOR   = 0x0200,

    PUT_ELEV_PLY_INSIDE  = 0x0400,/* プレーヤーがエレベータの中にいる */

    PUT_ELEV_STOP_SE     = 0x0800,/* SEを止める */

    PUT_ELEV_VIB_ALWAYS  = 0x1000,/* 必ず振動する */
    PUT_ELEV_PLY_ON_ELEV = 0x2000,/* 必ず振動する */
} ;

#define PUT_ELV_N_PROCS 2 
#define PUT_ELV_N_TRAPS 2
#define PUT_ELV_MARGINE 200.0f

/* 動き始めてから振動するフレーム */
#define PUT_ELV_VIBFRM 4

typedef struct Work_t
{
    GV_ACT_EX actor ;

    FVECTOR   top    ;
    FVECTOR   bottom ;
    float     vel    ;
    DG_OBJS  *elevator ;

    FVECTOR   size   ;
    FVECTOR   offset ;

    OBJS     *objs     ;
    int       n_objs   ;
    int       proc[PUT_ELV_N_PROCS] ;
    int       trap[PUT_ELV_N_TRAPS] ;/* 敵兵がいるかトラップの登録 */

    HZX_D_SEGMENT *segs[2+3] ; //2 for door, 3 for elev
    HZX_D_FLOOR   *flrs    ;
    R_INTRPT       r_intrpt ;

    int       move_vib ;
	int       move_se  ;
    int       door     ;
    int       flag     ;
    int       name     ;
} Work ;

static Work *PUT_ELV_Work = NULL ;
static FVECTOR PUT_Elv_W11Pos[] = {
    {   0.0f,-45000.0f,-10125.0f,1.0f},/*エレベータ位置*/
    {1765.0f,-43436.0f, -9930.0f,1.0f},/*ボタン位置    */
    {1765.0f,-43536.0f, -9930.0f,1.0f},/*ボタン位置    */
    {   0.0f,-44000.0f,-10000.0f,1.0f},/*ゾーン位置    */
 } ;
static FVECTOR PUT_Elv_W12Pos[] = {
    {   0.0f, -1050.0f,  0.0f,1.0f},
    {1747.0f,   541.0f, -9930.0f,1.0f},
    {1747.0f,   441.0f, -9930.0f,1.0f},
    {   0.0f,  5000.0f,-10000.0f,1.0f},/*ゾーン位置    */
} ;


extern void *NewFortPutMotion_Called( int name, int where,
				      int kms_id, int evm_id, int mar_id, int mar_num,
				      FVECTOR *pos, SVECTOR *rot,
				      int flag, void *callback ) ;

extern void *NewFortElevatorButtonCalled( int name,
					  FVECTOR *up,
					  FVECTOR *down,
					  int mode ) ;


#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoDEG(_a) ((int)(_a)*180/32768)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)


/* エレベータの現在の位置を取るコマンド */
int NewComPUT_ELV_GetPos()
{
    Work *work ;

    if ( (work = PUT_ELV_Work) )
    {
#if 0
	GCL_VAR_REF ref ; /* 配列への参照データ */
	if ( GCL_NextStr() )
	{
	    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
	    GCL_SetVarRef( &ref, 0, work->elevator->world.m[W][Y] ) ;
	    return 1 ;
	}
#else
	return (int)work->elevator->world.m[W][Y] ;
#endif
    }

    return 0 ;
}

/*

  プレーヤがエレベータにいるかどうか

*/
int NewComPUT_ELV_PlayerInside()
{
    if ( PUT_ELV_Work )
	return PUT_ELV_Work->flag & PUT_ELEV_PLY_INSIDE ;
    return 0 ;
}

/*  

    エレベータ内のボム系を除去する

*/
int NewComPUT_ELV_EraseBomb()
{
    GM_BOMB *b ;
    Work *work ;
    FVECTOR  v ;
    
    if ( (work = PUT_ELV_Work) )
    {
	_sceVu0ApplyMatrix( &v, &work->elevator->world, &work->offset ) ;

	/* エレベータ内のボムを消す */
	for( b=GM_BombList.next ; b!=NULL ; b=b->next )
	{
	    float x = v.vx + work->elevator->world.m[W][X] ;
	    float z = v.vz + work->elevator->world.m[W][Z] ;

	    if ( b->mov->vx > x-work->size.vx &&
		 b->mov->vx < x+work->size.vx && 
		 b->mov->vz > z-work->size.vz &&
		 b->mov->vz < z+work->size.vz )
		b->flag |= GM_BMB_FLAG_DESTROY ;
	}
	return 1 ;
    }
    return  0 ;
}





static void PUT_ELV_MakeHzxFloor( Work *work, IVECTOR *p )
{
    FVECTOR v ;
    int     i ;

    /* エレベータの床 */
    for ( i=4 ; --i>=0 ; )
    {
	if ( i&2 )
	    v.vx = work->offset.vx + work->size.vx + PUT_ELV_MARGINE ;
	else
	    v.vx = work->offset.vx - work->size.vx - PUT_ELV_MARGINE ;
	v.vy = work->offset.vy + work->size.vy ;
	if ( i==1||i==2 )
	    v.vz = work->offset.vz + work->size.vz + PUT_ELV_MARGINE ;
	else
	    v.vz = work->offset.vz - work->size.vz - PUT_ELV_MARGINE ;
	v.vw = 1.0f ;
	_sceVu0ApplyMatrix( &v, &work->elevator->world, &v ) ;
	_sceVu0FTOI0Vector( &p[i], &v ) ;
    }
}




static void PUT_ELV_CheckPlayerOn( Work *work )
{
    FVECTOR v ;

    _sceVu0ApplyMatrix( &v, &work->elevator->world, &work->offset ) ;

    if ( v.vx + work->size.vx > GM_PlayerControl->mov.vx &&
	 v.vx - work->size.vx < GM_PlayerControl->mov.vx &&
	 v.vz + work->size.vz > GM_PlayerControl->mov.vz && 
	 v.vz - work->size.vz < GM_PlayerControl->mov.vz )
	work->flag |=  PUT_ELEV_PLY_ON_ELEV ;
    else
	work->flag &= ~PUT_ELEV_PLY_ON_ELEV ;
}

static int PUT_ELV_CheckPlayerInside( Work *work )
{
    if ( work->trap[0] )
    {
	if ( GM_CheckInsideTrap( GM_PlayerControl, &work->trap[0], 1, 0 ) )
	    work->flag |=  PUT_ELEV_PLY_INSIDE ;
	else
	    work->flag &= ~PUT_ELEV_PLY_INSIDE ;
    }
    return 0 ;
}


static int PUT_ELV_CheckForMove( Work *work )
{
    HOMING_TRG *hom ;

    /* 危険モードだったら閉められない。 */
    if ( GM_AlertMode == ALERT_MODE_ALERT )
	return 0 ;
    for( hom=GM_GetHoming() ; hom ; hom=hom->next )
	if ( hom->status & HOMING_ENEMY || hom->status & HOMING_DEAD )
	    if ( GM_CheckInsideTrap( hom->ctrl, &work->trap[1], 1, 0 ) )
	    {
		printf( "ENEMY EXIST Close Abort : NewPutElevator\n" ) ;
		return 0 ;
	    }
    return 1 ;
}


static void SeSet( Work *work, int id )
{
    FVECTOR v1, v2 ;

    _sceVu0ITOF0Vector( &v1, &work->flrs->p1 ) ;
    _sceVu0ITOF0Vector( &v2, &work->flrs->p3 ) ;
    _sceVu0AddVector ( &v1, &v1, &v2 ) ;
    _sceVu0ScaleVector( &v1, &v1, 0.5f ) ;
    GM_SeSetMode( id, &v1, GM_SEMODE_NORMAL ) ;
}

#if 0
static void PUT_ELV_SendDebug( int name, int com )
{
    GV_MSG msg ;
    int buffer = com ;

    msg.message = &buffer ;
    msg.message_len = 1 ;
    msg.address = name ;
    GV_SendMessage( &msg ) ;
}
#endif

static void PUT_ELV_BtnBlue( Work *work )
{
    GV_MSG msg ;
    int buffer = 2 ;

    if ( work->flag & PUT_ELEV_USE_IN_W12 )
	return ;

    printf( "PUT_ELV_BtnUpGreen()  : NewPutElevator\n" ) ;
    msg.message = &buffer ;
    msg.message_len = 1 ;
    msg.address = 6551818 /* 昇降機スイッチ */ ;
    GV_SendMessage( &msg ) ;
}

static void PUT_ELV_BtnUp( Work *work )
{
    GV_MSG msg ;
    int buffer = 0 ;

    if ( work->flag & PUT_ELEV_USE_IN_W12 )
	return ;

    printf( "PUT_ELV_BtnUpGreen()  : NewPutElevator\n" ) ;
    msg.message = &buffer ;
    msg.message_len = 1 ;
    msg.address = 6551818 /* 昇降機スイッチ */ ;
    GV_SendMessage( &msg ) ;
}


static void PUT_ELV_BtnDown( Work *work )
{
    GV_MSG msg ;
    int buffer = 1 ;

    if ( work->flag & PUT_ELEV_USE_IN_W12 )
	return ;

    printf( "PUT_ELV_BtnDownGreen()  : NewPutElevator\n" ) ;
    msg.message = &buffer ;
    msg.message_len = 1 ;
    msg.address = 6551818 /* 昇降機スイッチ */ ;
    GV_SendMessage( &msg ) ;
}

static void PUT_ELV_OpenShutter( Work *work )
{
    GV_MSG msg ;
    int buffer[3] = { 0,0,0 } ;

    if ( !(work->flag & PUT_ELEV_USE_IN_W12) )
    {
	buffer[0] = 3 ;

	/* 扉の動き始め 振動（開く閉じ共通）w11acのみ */
	if ( work->flag & (PUT_ELEV_PLY_ON_ELEV|PUT_ELEV_VIB_ALWAYS) )
	    NewPadVibration2( 3830880/*dock_ele_01*/, 0 ) ;
    }

    /* 扉キャラに開くメッセージを送る */
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    msg.address = 5181087  /* elevator_door */ ;
    GV_SendMessage( &msg ) ;

    work->flag |= PUT_ELEV_OPEN_DOOR ;
    printf( "PUT_ELV_OpenShutter()  : NewPutElevator\n" ) ;
}


static void PUT_ELV_CloseShutter( Work *work )
{
    GV_MSG msg ;
    int buffer[3] = { 1, 1, 0 } ;

    /* OKプロックを発動 */
    if ( work->proc[0] )
	GCL_ExecProc( work->proc[0], NULL ) ;

    /* エレベーターが上にあるかどうか(ボタンはW11のみ) */
    if ( !(work->flag & PUT_ELEV_USE_IN_W12) )
    {
	if ( PUT_ELV_Work->elevator->world.m[W][Y] == PUT_ELV_Work->bottom.vy )
	    PUT_ELV_BtnUp( work ) ;
	else
	    PUT_ELV_BtnDown( work ) ;
    }
    if ( !(work->flag & PUT_ELEV_USE_IN_W12) )
    {
	buffer[0] = 3 ;

	/* 扉の動き始め 振動（開く閉じ共通） */
	if ( work->flag & (PUT_ELEV_PLY_ON_ELEV|PUT_ELEV_VIB_ALWAYS) )
	    NewPadVibration2( 3830880/*dock_ele_01*/, 0 ) ;
    }

    /* ゾーンを閉じる */
    work->r_intrpt.status &= ~ROOT_INTRPT_OPEN ;
    work->r_intrpt.status |= ROOT_INTRPT_CLOSE ;
    printf( "ROOT_INTRPT_CLOSE  : NewPutElevator\n" ) ;

    /* 扉キャラに閉じるメッセージを送る */
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    msg.address = 5181087  /* elevator_door */ ;
    GV_SendMessage( &msg ) ;

    work->flag |= PUT_ELEV_CLOSE_DOOR ;
    printf( "PUT_ELV_CloseShutter()  : NewPutElevator\n" ) ;
}

static void RotateTransMatrix( FMATRIX *mtx, FVECTOR *pos, short rot[] )
{
    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, ANGtoRAD(rot[Y]) ) ;
    _sceVu0RotMatrixX( mtx, mtx           , ANGtoRAD(rot[X]) ) ;
    _sceVu0RotMatrixZ( mtx, mtx           , ANGtoRAD(rot[Z]) ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], pos ) ;
}

static inline void UpdateObject( Work *work )
{
    FVECTOR pos ;
    int  i ;
    FMATRIX *mtx = &work->elevator->world ;
    OBJS *o ;

    o = work->objs ;
    for ( i=work->n_objs ; --i>=0 ; o++ )
    {
	if ( !(work->flag & PUT_ELEV_STOP_MOTION) )
	    GM_ActMotion( &o->body ) ;
	_sceVu0AddVector( &o->pos, &o->pos, &o->vel ) ;
	_sceVu0CopyVector( &pos, &o->pos ) ;
	if ( !(work->flag & PUT_ELEV_STOP_MOTION) )
	    pos.vy += o->body.m_ctrl->height ;
	pos.vw = 0.0f ;
	DG_SetPos( mtx ) ;
	DG_MovePos( &pos ) ;
	DG_RotatePos( &o->rot ) ;
	GM_ActObject2( &o->body ) ;
    }
}

static void PUT_ELV_LetGoUp( Work *work )
{
    if ( !(work->flag & PUT_ELEV_USE_IN_W12) &&
	 work->elevator->world.m[W][Y] == work->bottom.vy )
	PUT_ELV_CloseShutter( work ) ;
    else if ( work->elevator->world.m[W][Y] == work->bottom.vy )
    {
	PUT_ELV_Work->flag |=  PUT_ELEV_GO_UP   ;
	PUT_ELV_Work->flag &= ~PUT_ELEV_GO_DOWN ;
	PUT_ELV_BtnUp( work ) ;
    }
}

static void PUT_ELV_LetGoDown( Work *work )
{
    if ( work->flag & PUT_ELEV_USE_IN_W12 &&
	 work->elevator->world.m[W][Y] == work->top.vy )
	PUT_ELV_CloseShutter( work ) ;
    else if ( work->elevator->world.m[W][Y] == work->top.vy )
    {
	PUT_ELV_Work->flag |=  PUT_ELEV_GO_DOWN ;
	PUT_ELV_Work->flag &= ~PUT_ELEV_GO_UP   ;
	PUT_ELV_BtnDown( work ) ;
    }
}

static inline void ActRecieveMessage( Work *work )
{
    GV_MSG *msg ;
    int     i   ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 0:
	    SeSet( work, SD_A_EVSTART1 ) ;
	    /* 動き始め 振動 */
	    PUT_ELV_Work->move_vib = PUT_ELV_VIBFRM ;
	    printf( "start fort_elev.c  : NewPutElevator\n" ) ;
	    work->flag &= ~PUT_ELEV_STOP_SE ;
	    work->flag |=  PUT_ELEV_GO_DOWN ;
	    work->flag &= ~PUT_ELEV_GO_UP   ;
	    break ;
	case 1:
	    SeSet( work, SD_A_EVSTART1 ) ;
	    /* 動き始め 振動 */
	    PUT_ELV_Work->move_vib = PUT_ELV_VIBFRM ;
	    printf( "start fort_elev.c  : NewPutElevator\n" ) ;
	    work->flag &= ~PUT_ELEV_STOP_SE ;
	    work->flag |=  PUT_ELEV_GO_UP   ;
	    work->flag &= ~PUT_ELEV_GO_DOWN ;
	    break ;
	case 2:
	    work->flag |=  PUT_ELEV_STOP_MOTION ;
	    break ;
	case 3:
	    work->flag &= ~PUT_ELEV_STOP_MOTION ;
	    break ;

	case 4: /* 下に動かす */
	    PUT_ELV_LetGoDown( work ) ;
	    break ;

	case 5: /* 上に動かす */
	    PUT_ELV_LetGoUp( work ) ;
	    break ;

	case 6: /* 上下のトグル */
	    if ( work->flag & PUT_ELEV_USE_IN_W12 ? 
		 work->elevator->world.m[W][Y] == work->top.vy :
		 work->elevator->world.m[W][Y] == work->bottom.vy )
		PUT_ELV_CloseShutter( work ) ;
	    else if ( work->elevator->world.m[W][Y] == work->top.vy )
	    {
		work->flag &= ~PUT_ELEV_STOP_SE ;
		PUT_ELV_Work->flag |=  PUT_ELEV_GO_DOWN ;
		PUT_ELV_Work->flag &= ~PUT_ELEV_GO_UP   ;
		PUT_ELV_BtnDown( work ) ;
	    }
	    else
	    {
		work->flag &= ~PUT_ELEV_STOP_SE ;
		PUT_ELV_Work->flag |=  PUT_ELEV_GO_UP   ;
		PUT_ELV_Work->flag &= ~PUT_ELEV_GO_DOWN ;
		PUT_ELV_BtnUp( work ) ;
	    }
	    break ;

	case 7: /* 扉は開き切った */
	    work->r_intrpt.status |= ROOT_INTRPT_OPEN ;
	    work->r_intrpt.status &= ~ROOT_INTRPT_CLOSE ;
	    printf( "ROOT_INTRPT_OPEN  : NewPutElevator\n" ) ;
	    break ;

	    /* 振動をさせるかどうか */
	case 8:
	    work->flag |=  PUT_ELEV_VIB_ALWAYS ;
	    break ;

	case 9:
	    work->flag &= ~PUT_ELEV_VIB_ALWAYS ;
	    break ;
	}
}


/* 止まる時の処理 */
static void ActStopElevator( Work *work )
{
    work->vel = 0.0f ;
    work->flag &= PUT_ELEV_CLEAR_STAT ;
    work->door = 10*5/TIME_BASE ;

    if ( work->flag & (PUT_ELEV_PLY_ON_ELEV|PUT_ELEV_VIB_ALWAYS) )
	NewPadVibration2( 3830882/*dock_ele_03*/, 0 ) ;

    if ( work->proc[1] )
	GCL_ExecProc( work->proc[1], NULL ) ;
}

/* 動いている共通の処理 */
static void ActMoveElevator( Work *work )
{
    work->elevator->world.m[W][Y] += work->vel ;

    if ( !(GM_StagePlayTime % (14*5/TIME_BASE)) ) {
#ifdef KP_XBOX
		if ( (work->move_se^=1) ) {
			SeSet( work, SD_A_EVMOTOR1 ) ;
		} else {
			SeSet( work, SD_A_EVMOTOR2 ) ;
		}
#else
		SeSet( work, SD_A_EVMOTOR1 ) ;
#endif
	}
}

/* ハザードを動かす */
static void ActMoveHzx( Work *work, float vely )
{
    int     i ;
    IVECTOR shift = { 0, (int)vely, 0, 0} ;
    IVECTOR p[4] ;

    for ( i=3 ; --i>=0 ; )
	HZX_ShiftDynamicSegment( work->segs[i], &shift ) ;

    /* エレベータ床の形計算 */
    PUT_ELV_MakeHzxFloor( work, p ) ;
    HZX_MoveDynamicFloor( work->flrs,
			  &p[0],&p[1],&p[2],&p[3] ) ;
    //HZX_ShiftDynamicFloor( work->flrs, &shift ) ;
}

/* 扉を開け閉め時の処理 */
static void ActOpenCloseHzx( Work *work )
{
    FVECTOR *pos ;

    if ( work->flag & PUT_ELEV_OPEN_DOOR )
    {
	if ( work->flag & PUT_ELEV_STOP_DOOR )
	{
	    /* ゾーンを開く */
	    work->r_intrpt.status |= ROOT_INTRPT_OPEN ;
	    work->r_intrpt.status &= ~ROOT_INTRPT_CLOSE ;
	    printf( "ROOT_INTRPT_OPEN  : NewPutElevator\n" ) ;

	    work->flag &= ~(PUT_ELEV_OPEN_DOOR| PUT_ELEV_STOP_DOOR) ;
	    pos = work->flag&PUT_ELEV_USE_IN_W12 ? PUT_Elv_W12Pos : PUT_Elv_W11Pos ;
	    work->segs[3]->p1.vx = work->segs[3]->p2.vx = (int)(pos->vx - 1500) ;
	    work->segs[4]->p2.vx = work->segs[4]->p1.vx = (int)(pos->vx + 1500) ;
	}
	else
	{
	    work->segs[3]->p2.vx -= 25 ;
	    work->segs[4]->p1.vx += 25 ;
	}

	HZX_MoveDynamicSegment( work->segs[3],
				&work->segs[3]->p1, &work->segs[3]->p2 ) ;
	HZX_MoveDynamicSegment( work->segs[4],
				&work->segs[4]->p1, &work->segs[4]->p2 ) ;
    }
    else if ( work->flag & PUT_ELEV_CLOSE_DOOR )
    {
	if ( work->flag & PUT_ELEV_STOP_DOOR )
	{
	    work->flag &= ~(PUT_ELEV_CLOSE_DOOR| PUT_ELEV_STOP_DOOR) ;
	    pos = work->flag&PUT_ELEV_USE_IN_W12 ? PUT_Elv_W12Pos : PUT_Elv_W11Pos ;
	    work->segs[3]->p1.vx = (int)(pos->vx - 1500) ;
	    work->segs[3]->p2.vx = (int)(pos->vx) ;
	    work->segs[4]->p2.vx = (int)(pos->vx + 1500) ;
	    work->segs[4]->p1.vx = (int)(pos->vx) ;
	}
	else
	{
	    work->segs[3]->p2.vx += 25 ;
	    work->segs[4]->p1.vx -= 25 ;
	}

	HZX_MoveDynamicSegment( work->segs[3],
				&work->segs[3]->p1, &work->segs[3]->p2 ) ;
	HZX_MoveDynamicSegment( work->segs[4],
				&work->segs[4]->p1, &work->segs[4]->p2 ) ;
    }
}


/* エレベータ全処理 */
static void Act( Work *work )
{
    if ( !GM_IsGameOver() )
    {
	/* メッセージ受取 */
	ActRecieveMessage( work ) ;

	/* プレーヤーが乗っているかどうか */
	PUT_ELV_CheckPlayerInside( work ) ;
	PUT_ELV_CheckPlayerOn( work ) ;

	/* 動き始め 振動 */
	if ( work->move_vib > 0 )
	    if ( --work->move_vib == 0 )
		if ( work->flag & (PUT_ELEV_PLY_ON_ELEV|PUT_ELEV_VIB_ALWAYS) )
		    NewPadVibration2( 3830881/*dock_ele_02*/, 0 ) ;

	/* ドアの開閉状況 */
	if ( work->door > 0 )
	    if ( --work->door == 0 )
	    {
		FVECTOR *pos = (FVECTOR *)work->elevator->world.m[W] ;

		SeSet( work, SD_A_EVCALL01 ) ;
		if ( work->flag & PUT_ELEV_USE_IN_W12 ?
		     (pos->vy == work->top.vy   ) :
		     (pos->vy == work->bottom.vy) )
		    PUT_ELV_OpenShutter( work ) ;
		else
		    PUT_ELV_BtnBlue( work ) ;
	    }

	/* 自動的に動くかどうか？ */
	if ( !(work->flag & (PUT_ELEV_OPEN_DOOR |
			     PUT_ELEV_CLOSE_DOOR|
			     PUT_ELEV_GO_UP     |
			     PUT_ELEV_GO_DOWN )) )
	    if ( work->flag & PUT_ELEV_PLY_INSIDE )
		if ( PUT_ELV_CheckForMove( work ) )
		{
		    if ( work->flag & PUT_ELEV_USE_IN_W12 )
			PUT_ELV_LetGoDown( work ) ;
		    else
			PUT_ELV_LetGoUp( work ) ;
		}

	{
#if 0
	    int i ;
	    for ( i=5 ; --i>=0 ; )
		HZX_ViewDynamicSegment( work->segs[i] ) ;
	    HZX_ViewDynamicFloor( work->flrs, 4 ) ;

	    if ( GV_PadData[0].press&PAD_R2 ) PUT_ELV_SendDebug(work->name,5);
	    if ( GV_PadData[0].press&PAD_L2 ) PUT_ELV_SendDebug(work->name,4);
#endif
	}

	/*降下*/
	if ( work->flag & PUT_ELEV_GO_DOWN )
	{
	    if ( work->elevator->world.m[W][Y] > work->bottom.vy + 500.0f )
		work->vel -= 1.0f ;
	    else
		work->vel += 1.0f ;
	    work->vel = (work->vel<-50.0f ?-50.0f :
			 work->vel> -1.0f ? -1.0f : work->vel) ;
	    ActMoveElevator( work ) ;

	    if ( work->elevator->world.m[W][Y] < work->bottom.vy + 200.0f )
		if ( !(work->flag & PUT_ELEV_STOP_SE) )
		{
		    SeSet( work, SD_A_EVSTOP01 ) ;
		    work->flag |= PUT_ELEV_STOP_SE ;
		}
	    if ( work->elevator->world.m[W][Y] < work->bottom.vy )
	    {
		work->elevator->world.m[W][Y] = work->bottom.vy ;
		ActStopElevator( work ) ;
	    }
	    ActMoveHzx( work, work->vel ) ;
	}
	
	/*上昇*/
	else if ( work->flag & PUT_ELEV_GO_UP )
	{
	    if ( work->elevator->world.m[W][Y] < work->top.vy - 500.0f )
		work->vel += 1.0f ;
	    else
		work->vel -= 1.0f ;
	    work->vel = (work->vel>50.0f ? 50.0f :
			 work->vel< 1.0f ?  1.0f : work->vel) ;
	    ActMoveElevator( work ) ;
	    if ( work->elevator->world.m[W][Y] > work->top.vy - 300.0f )
		if ( !(work->flag & PUT_ELEV_STOP_SE) )
		{
		    SeSet( work, SD_A_EVSTOP01 ) ;
		    work->flag |= PUT_ELEV_STOP_SE ;
		}
	    if ( work->elevator->world.m[W][Y] > work->top.vy )
	    {
		work->elevator->world.m[W][Y] = work->top.vy ;
		ActStopElevator( work ) ;
	    }
	    ActMoveHzx( work, work->vel ) ;
	}

	/* 停止時 */
	else
	    ActOpenCloseHzx( work ) ;

	/* 乗っているもの位置などを更新する */
	if ( work->flag & (PUT_ELEV_GO_DOWN|PUT_ELEV_GO_UP) ||
	     !(work->flag & PUT_ELEV_STOP_MOTION) )
	    UpdateObject( work ) ;
    }

}

static void Die( Work *work )
{
    int  i ;

    /* エレベータモデルを解放 */
    if ( work->elevator )
    {
	DG_DequeueObjs( work->elevator ) ;
	DG_FreeObjs( work->elevator ) ;
    }

    /* 搭乗モデルを解放 */
    if ( work->objs )
    {
	for ( i=work->n_objs ; --i>=0 ; )
	{
	    GM_FreeControl( &work->objs[i].control ) ;
	    GM_FreeObject( &work->objs[i].body ) ;
	}
	GV_Free( work->objs ) ;
    }

    /* ダイナミックハザードを解放 */
    for ( i=5 ; --i>=0 ; )
	if ( work->segs[i] )
	    HZX_RemoveDynamicSegment( work->segs[i] ) ;
    if ( work->flrs )
	HZX_RemoveDynamicFloor( work->flrs ) ;
    GM_FreeRouteIntrpt( &work->r_intrpt ) ;	

    PUT_ELV_Work = NULL ;
}

int PUT_ELV_MakeHzxFromOBJS( Work *work, DG_OBJS *objs, int where )
{
    IVECTOR p[4] ;
    HZX_GROUP_ID hzx_id ;
    FVECTOR uv = { objs->def->ux, objs->def->uy, objs->def->uz, 0.0f } ;
    FVECTOR lv = { objs->def->lx, objs->def->ly, objs->def->lz, 0.0f } ;
    int     i ;
    FVECTOR v ;

    ASSERT( where ) ;

    /* モデルからサイズを取得 */
    _sceVu0SubVector  ( &work->size  , &uv, &lv ) ;
    _sceVu0ScaleVector( &work->size  , &work->size  , 0.5f ) ;
    work->size.vw = 0.0f ;
    _sceVu0AddVector  ( &work->offset, &uv, &lv ) ;
    _sceVu0ScaleVector( &work->offset, &work->offset, 0.5f ) ;
    work->offset.vx += objs->def->tx ;
    work->offset.vy += objs->def->ty ;
    work->offset.vz += objs->def->tz ;
    work->offset.vw  = 1.0f ;

    /* エレベータの周りに壁を貼る */
    hzx_id = GM_GetHzxGroupID( where ) ;
    for ( i=3 ; --i>=0 ; )
    {
	v.vx = work->offset.vx + (i   ? -work->size.vx : work->size.vx) ;
	v.vy = work->offset.vy + work->size.vy ;
	v.vz = work->offset.vz + (i>1 ? -work->size.vz : work->size.vz) ;
	v.vw = 1.0f ;
	_sceVu0ApplyMatrix( &v, &objs->world, &v ) ;
	v.vw = 1800.0f ;
	_sceVu0FTOI0Vector( &p[0], &v ) ;

	v.vx = work->offset.vx + (i&1 ? -work->size.vx : work->size.vx) ;
	v.vy = work->offset.vy + work->size.vy ;
	v.vz = work->offset.vz - work->size.vz ;
	v.vw = 1.0f ;
	_sceVu0ApplyMatrix( &v, &objs->world, &v ) ;
	v.vw = 1800.0f ;
	_sceVu0FTOI0Vector( &p[1], &v ) ;

	work->segs[i] = HZX_AddDynamicSegment( hzx_id, &p[0], &p[1], 
					       HZX_SEG_NO_BULLET|
					       HZX_FLOOR_NO_BULLETHOLE|
					       HZX_FLOOR_NO_C4|
					       HZX_FLOOR_NO_SPRAY |
					       HZX_SEG_NO_MISSILE|
					       HZX_SEG_NO_KNOCK_SE ) ;
	/*レーダーに映すため*/
	work->segs[i]->atr &= ~HZX_SEG_NO_DISP_RADAR ;
    }

    /* エレベータ床の形計算 */
    PUT_ELV_MakeHzxFloor( work, p ) ;
    work->flrs = HZX_AddDynamicFloor( hzx_id, &p[0],&p[1],&p[2],&p[3], 4,
				      HZX_FLOOR_NO_BLOOD|
				      HZX_FLOOR_NO_C4|
				      HZX_FLOOR_NO_BULLETHOLE|
				      HZX_FLOOR_NO_SPRAY |
				      HZX_FLOOR_NO_CLAYMORE ) ;

    /* 扉 */
    _sceVu0CopyVector( &v, (work->flag&PUT_ELEV_USE_IN_W12 ?
			    PUT_Elv_W12Pos : PUT_Elv_W11Pos) ) ;
    v.vw = 1800.0f ;
    _sceVu0FTOI0Vector( &p[0], &v ) ;
    _sceVu0FTOI0Vector( &p[1], &v ) ;
    p[1].vx -= 1500 ;
    work->segs[3] = HZX_AddDynamicSegment( hzx_id, &p[0], &p[1],
					   HZX_SEG_NO_BULLETHOLE|
					   HZX_SEG_NO_RECOIL ) ;
    p[0].vx *= -1 ;
    p[1].vx *= -1 ;
    work->segs[4] = HZX_AddDynamicSegment( hzx_id, &p[0], &p[1],
					   HZX_SEG_NO_BULLETHOLE|
					   HZX_SEG_NO_RECOIL ) ;
    if ( work->flag & PUT_ELEV_USE_IN_W12 )
    {
	work->segs[3]->atr |= HZX_SEG_SKIP|(3<<28) ;
	work->segs[4]->atr |= HZX_SEG_SKIP|(3<<28) ;
    }
    else
    {
	work->segs[3]->atr |= HZX_SEG_DOOR|(4<<28) ;
	work->segs[4]->atr |= HZX_SEG_DOOR|(4<<28) ;
	work->segs[3]->atr &= ~HZX_SEG_NO_DISP_RADAR ;
	work->segs[4]->atr &= ~HZX_SEG_NO_DISP_RADAR ;
    }

    return 0 ;
}



/* ゾーン遮断 */
static void InitZoneInterrupt( Work *work )
{
    HZX_ZONE_ADD zone[2] ;
    HZX_ZON     *z[2] ;
    FVECTOR     *pos ;

    pos = ( work->flag & PUT_ELEV_USE_IN_W12 ?
	    &PUT_Elv_W12Pos[3] : &PUT_Elv_W11Pos[3] ) ;
    HZX_GetInterruptZone( pos, 1024, 250, zone ) ;
    z[0] = HZX_GetZoneFromAdd( zone[0] ) ;
    z[1] = HZX_GetZoneFromAdd( zone[1] ) ;
    if ( z[0] != NULL )
	z[0]->flag |= HZX_ZONE_SLIDEDOOR ;
    if ( z[1] != NULL )
	z[1]->flag |= HZX_ZONE_SLIDEDOOR ;
    GM_SetRouteIntrpt( &work->r_intrpt, zone[0], zone[1], pos, 
		       1024, ROOT_INTRPT_ELV|ROOT_INTRPT_CLOSE, 0 ) ;
    GM_PutRoteIntrpt( &work->r_intrpt ) ;
}

static void ButtonCallBack( int mot_num )
{
    PUT_ELV_Work->flag &= ~(PUT_ELEV_GO_DOWN | PUT_ELEV_GO_UP) ;


    if ( PUT_ELV_Work->flag & (PUT_ELEV_OPEN_DOOR | PUT_ELEV_CLOSE_DOOR) )
	PUT_ELV_Work->flag |= PUT_ELEV_STOP_DOOR ;
    switch ( mot_num )
    {
    case 0:
	PUT_ELV_BtnBlue( PUT_ELV_Work ) ;
	break ;
    case 1:
	/* 動き始め 音 */
	SeSet( PUT_ELV_Work, SD_A_EVSTART1 ) ;
	PUT_ELV_Work->move_vib = PUT_ELV_VIBFRM ;

	PUT_ELV_Work->flag &= ~PUT_ELEV_STOP_SE ;
	if ( PUT_ELV_Work->elevator->world.m[W][Y] == PUT_ELV_Work->bottom.vy )
	    PUT_ELV_Work->flag |= PUT_ELEV_GO_UP   ;
	else
	    PUT_ELV_Work->flag |= PUT_ELEV_GO_DOWN ;
	break ;
    }
}

static int GetResourcesCalled( Work *work, int name, int where,
			       int flag, int *procs, int *traps,
			       int e_id, FVECTOR *e_top, FVECTOR *e_bottom,
			       FVECTOR *e_pos, SVECTOR *e_rot, int e_dir,
			       int id_e[], int id_k[],int id_m[],
			       FVECTOR *pos, SVECTOR rot[], int names[], int n_objs )
{
    int      i ;
    DG_DEF  *kdef ;
    static SVECTOR shutr_rot = { 0, 2048, 0, 0 } ;
    int      motion ;

    work->name = name ;
    work->flag = flag ;

	work->move_se = 0 ;

    /* 昇降機モデル */
    if ( !(kdef = GV_GetCache( GV_CacheID( e_id, 'k' ) )) )
	PERROR( "No Elevator Model<%d> : NewPutElevator\n", e_id ) ;
    if ( !( work->elevator = DG_MakeObjs( kdef, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot make OBJS(maybe no memory) : NewPutElevator" ) ;
    DG_QueueObjs( work->elevator ) ;

    e_rot->vx *= 16 ;
    e_rot->vy *= 16 ;
    e_rot->vz *= 16 ;
    _sceVu0CopyVector( &work->top   , e_top    ) ;
    _sceVu0CopyVector( &work->bottom, e_bottom ) ;
    if ( !e_dir )
	e_pos = (work->flag & PUT_ELEV_OPEN_DOOR ?
		 (work->flag & PUT_ELEV_USE_IN_W12 ? &work->top : &work->bottom) :
		 (work->flag & PUT_ELEV_USE_IN_W12 ? &work->bottom : &work->top)) ;
    else if ( e_dir > 0 )
        work->flag |= PUT_ELEV_GO_UP   ;
    else
	work->flag |= PUT_ELEV_GO_DOWN ;

    RotateTransMatrix( &work->elevator->world, e_pos, (short *)e_rot ) ;
    work->vel = 0.0f ;

    if ( PUT_ELV_MakeHzxFromOBJS( work, work->elevator, where ) )
	    PERROR( "Cannot Make HZX Segment :NewPutElevator\n" ) ;

    /* 搭乗物のメモリー確保 */
    work->n_objs = n_objs ;
    if ( !(work->objs = GV_Malloc( n_objs * sizeof(OBJS) )) )
	PERROR( "No memory for alloc work->objs : NewPutElevator\n" ) ;
    for ( i=n_objs ; --i>=0 ; )
    {
	work->objs[i].body.m_ctrl = NULL ;
	work->objs[i].body.objs   = NULL ;
	work->objs[i].body.evmobj = NULL ;
	work->objs[i].pos.vw = 0.0f ;
    }

    /* 搭乗物の初期化 */
    for ( i=n_objs ; --i>=0 ; )
    {
	OBJS *obj = &work->objs[i] ;
	OBJECT *o = &obj->body ;

	GM_InitControl( &obj->control, names[i], where ) ;
	GM_ConfigControlObject( &obj->control, &obj->body ) ;
	GM_InitObject( o, id_k[i], DG_FLAG_SHADE|DG_FLAG_FINISHCALC ) ;

	_sceVu0CopyVectorXYZ( &obj->pos, &pos[i] ) ;
	obj->rot.vx = rot[i].vx ;
	obj->rot.vy = rot[i].vy ;
	obj->rot.vz = rot[i].vz ;

	if ( id_m[i] )
	{
	    GM_ConfigObjectMotion( o, 0, id_m[i], MT_FLAG_HUMAN2 ) ;
	    GM_ConfigObjectStep( o, &obj->vel ) ;
	    GM_ConfigObjectAction( o, 0, 0, 0,0xfffff,0 ) ;
	    printf( "ELEV HEIGHT %f  : NewPutElevator\n", o->m_ctrl->height ) ;
	}
	if ( id_e[i] )
	{
	    GM_ConfigObjectEvm( o, id_e[i], 0 ) ;
	    DG_InvisibleObjs( o->objs ) ;
	}
	DG_SetPos2( &obj->pos, &DG_ZeroSVector ) ;
	GM_ActObject2( o ) ;
    }

    /* トラップの登録 */
    for( i=0 ; i<PUT_ELV_N_TRAPS ; i++ )
	work->trap[i] = traps[i] ;

    /* プロックの登録 */
    for( i=0 ; i<PUT_ELV_N_PROCS ; i++ )
	work->proc[i] = procs[i] ;

    /* ゾーンの遮断を初期化 */
    InitZoneInterrupt( work ) ;

    /* シャッターキャラを起動 */
    e_pos  = work->flag & PUT_ELEV_USE_IN_W12 ? PUT_Elv_W12Pos : PUT_Elv_W11Pos ;
    if ( work->flag & PUT_ELEV_USE_IN_W12 )
    {
	extern void *NewPutElevatorFence( int name, int where,
					  FVECTOR *top, FVECTOR *bottom, SVECTOR *rot,
					  int flag, void *callback ) ;
	extern void *NewPutElevatorShadow( FMATRIX *elev, FVECTOR *top ) ;

	NewPutElevatorFence( 5181087  /* elevator_door name */, where,
			     &DG_ZeroVector, e_pos, &DG_ZeroSVector,
			     flag, ButtonCallBack ) ;

	NewPutElevatorShadow( &work->elevator->world, &work->top ) ;
    }
    else
    {
	motion = work->flag & PUT_ELEV_OPEN_DOOR  ? 3 : 2 ;
	NewFortPutMotion_Called( 5181087  /* elevator_door name */, where,
				 11041784 /* w11a2_shatter.kms  */, 0,
				 10482826 /* w11_shatter.mar    */, motion,
				 e_pos, &DG_ZeroSVector,
				 0x04, ButtonCallBack ) ;
	NewFortPutMotion_Called( 5181087  /* elevator_door name */, where,
				 11041784 /* w11a2_shatter.kms  */, 0,
				 10482826 /* w11_shatter.mar    */, motion,
				 e_pos, &shutr_rot,
				 0x04, NULL ) ;

	/* ボタンキャラを起動 */
	motion = work->flag & PUT_ELEV_OPEN_DOOR ?
	    (work->flag & PUT_ELEV_USE_IN_W12 ? 1 : 0) :
	    (work->flag & PUT_ELEV_USE_IN_W12 ? 0 : 1) ;
	NewFortElevatorButtonCalled( 6551818 /* 昇降機スイッチ */,
				     e_pos+1, e_pos+2, motion ) ;
    }

    return 0 ;
}

#define ELEV_N_IDS 10
static int GetResources( Work *work, int name, int where )
{
    int     buf[XYZW] ;
    int     id_e[ELEV_N_IDS] ;
    int     id_k[ELEV_N_IDS] ;
    int     id_m[ELEV_N_IDS] ;
    int     names[ELEV_N_IDS] ;
    SVECTOR rot[ELEV_N_IDS] ;
    FVECTOR pos[ELEV_N_IDS] ;
    FVECTOR e_top ;
    FVECTOR e_bottom ;
    FVECTOR e_pos ;
    int     e_dir ;
    SVECTOR e_rot ;
    int     n_id, i ;
    int     flag ;
    int     procs[PUT_ELV_N_PROCS] ;
    int     traps[PUT_ELV_N_TRAPS] ;

    n_id = 0 ;
    while( (i = GCL_GetNextOption()) )
        switch( i )
        {
        case 'k':
	    if ( n_id >= ELEV_N_IDS )
		PERROR( "Too many objects on the Elevator : NewPutElevator\n" ) ;
	    id_k[n_id] = GCL_GetNextInt() ;
	    id_e[n_id] = 0 ;
	    id_m[n_id] = 0 ;
            break ;

        case 'e':
	    id_e[n_id] = GCL_GetNextInt() ;
            break ;

        case 'm':
	    id_m[n_id] = GCL_GetNextInt() ;
            break ;

        case 'n':
	    names[n_id] = GCL_GetNextInt() ;
            break ;

        case 'r':
	    GCL_GetSV( GCL_NextStr(), (short *)&rot[n_id] ) ;
            break ;

        case 'p':
	    GCL_GetIV( GCL_NextStr(), buf ) ;
	    vu0_IV0toFV( (IVECTOR *)buf, &pos[n_id++] ) ;
            break ;
	}
    flag = GCL_GetOptionValue( 'f', 0 ) ;

    if ( GCL_GetOption( 'T' ) != NULL )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &e_top ) ;
    }
    if ( GCL_GetOption( 'B' ) != NULL )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &e_bottom ) ;
    }
    if ( GCL_GetOption( 'P' ) != NULL )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &e_pos ) ;
    }
    if ( GCL_GetOption( 'R' ) != NULL )
	GCL_GetSV( GCL_NextStr(), (short *)&e_rot ) ;

    if ( GCL_GetOption( 'v' ) != NULL )
	work->flag |= PUT_ELEV_VIB_ALWAYS ;

    e_dir = GCL_GetOptionValue( 'D', 0 ) ;

    /* プロックの登録 */
    if ( GCL_GetOption( 'O' ) )
	for( i=0 ; i<PUT_ELV_N_PROCS ; i++ )
	    procs[i] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;
    for( ; i<PUT_ELV_N_PROCS ; i++ )
	procs[i] = 0 ;

    /* トラップの登録 */
    if ( GCL_GetOption( 't' ) )
	for( i=0 ; i<PUT_ELV_N_TRAPS ; i++ )
	    traps[i] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;
    for( ; i<PUT_ELV_N_TRAPS ; i++ )
	traps[i] = 0 ;


    if ( GetResourcesCalled( work, name, where,
			     flag, procs, traps,
			     GCL_GetOptionValue( 'M', GV_StrCode( "" ) ),
			     &e_top, &e_bottom, &e_pos, &e_rot, e_dir,
			     id_e, id_k, id_m,
			     pos, rot, names, n_id  ) < 0 )
	return -1 ;

    return 0 ;
}

void *NewPutElevator( int name, int where )
{
    /* エレベータは一つしか上がらない */
    if ( PUT_ELV_Work )
    {
	printf( "Elevator must be only one : NewPutElevator" ) ;
	return NULL ;
    }
    PUT_ELV_Work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( PUT_ELV_Work != NULL )
    {
        GV_SetActor( &PUT_ELV_Work->actor, Act, Die ) ;
        GV_ActorEX( &PUT_ELV_Work->actor ) ;
        if ( GetResources( PUT_ELV_Work, name, where ) < 0 )
	{
            GV_DestroyActor( PUT_ELV_Work ) ;
            return NULL ;
        }
    }
    return (void *)PUT_ELV_Work ;
}
