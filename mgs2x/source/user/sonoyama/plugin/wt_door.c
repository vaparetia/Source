//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   wt_door.c
   
   水密ドア
   2000/02/22 M.Sonoyama
   $Id: wt_door.c,v 1.1.1.3 2002/11/19 11:50:52 Yoshizawa1 Exp $
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX 
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

//#define	NO_SEGMENT
#define	PRESHADE
#define	ACTOR_LEVEL	(GV_ACTOR_USER)
#define	ACTOR_PRIO	(254)

#ifdef PRESHADE
extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
#endif

/*------------------------------------------------------------------*/

#ifndef PRESHADE
#define	BODY_FLAG	(DG_FLAG_ONEPIECE | DG_FLAG_SHADE)
#else
#define	BODY_FLAG	(DG_FLAG_ONEPIECE | DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT)
#endif

/*------------------------------------------------------------------*/

enum {
    MSG_HOLD = 0,
    MSG_HDL_ROTATE,
    MSG_OPEN,
    MSG_HOLD_END,
    MSG_OPEN_END,
    MSG_HDL_ROTATE2,
    MSG_DOOR_ROTATE,
    MSG_DOOR_ROTATE2,
    MSG_DOOR_ROTATE3,
	MSG_OPEN_START,
	MSG_NOT_OPEN,
	MSG_NOT_OPEN_END,
	MSG_CLOSE,
	MSG_HANDLE_VISIBLE,
} ;

enum {
	EXEC_TRIG_OPEN_END = 0,
	EXEC_TRIG_HOLD_START = 1,
	EXEC_TRIG_HOLD_RELEASE = 2,
	EXEC_TRIG_OPEN_START = 3,
	EXEC_TRIG_CLOSE_END = 4,
} ;

enum {
    STATE_CLOSE = 0,
    STATE_OPEN,
    STATE_WORKING,
    STATE_CLOSING,			/* シナリオで閉じる */
	STATE_NOT_OPEN_WORKING,
	STATE_HANDLE_RETURNING,
	STATE_OPENING,			/* シナリオで開ける */
} ;

enum {
	WT_DOOR_FLAG_NOT_OPEN = 0x0001,					/* 開かない */
	WT_DOOR_FLAG_OPEN_START = 0x0002,				/* 開いた状態からスタート */
	WT_DOOR_FLAG_CHANGE_NOT_MOTION_ROTATE = 0x0100,
	WT_DOOR_FLAG_FIRST_HANDLE_ROT = 0x1000,			
	WT_DOOR_FLAG_HANDLE_ROTATE_DIR_EXPR = 0x2000,
	WT_DOOR_FLAG_EXIST_NOT_MOTION_ROTATE = 0x4000,	/* モーション設定でない回転がある */
	WT_DOOR_FLAG_FIRST = 0x8000
} ;

#define KOREADD

typedef	struct {
    GV_ACT		actor ;
    FMATRIX		light[ 2 ] ;
    FMATRIX		light2[ 2 ] ;
    FMATRIX		def_mat ;
    FVECTOR		normal ;
	FVECTOR		shift ;
    SVECTOR		rotate ;
    DG_OBJS		*door ;
    DG_OBJS		*handle ;

    int			name ;
    int			status ;
    int			type ;
    int			exec ;

    int			proc ;
    int			rotv ;
    int			def_rot ;
    int			chara ;

    int			count ;
    HZX_D_SEGMENT	*segment ;
	int			map ;
	int			flag ;

	OBJECT		body_door ;
	OBJECT		body_handle ;
	FVECTOR		door_step ;
	FVECTOR		handle_step ;

    HZX_D_SEGMENT	*segment2 ;
	int			door_motion ;
	int			handle_motion ;
	float		handle_time ;

	SVECTOR		handle_start_rot ;
	SVECTOR		handle_current_rot ;
	SVECTOR		handle_rotate_dir ;

	HZX_D_SEGMENT	*segment3 ;
	float			motion_speed ;
#ifdef KOREADD
	R_INTRPT		r_intrpt ;	/* ゾーンインタラプト */
#endif

} Work ;

/*------------------------------------------------------------------*/

static	int	N_Doors, MAX_Doors ;
static	Work	**Doors ;

//static FVECTOR Shift = { 575.0F, 1200.0F -40.0F } ;
static FVECTOR Shift = { 575.0F, 1200.0F, -37.5F } ;
static FVECTOR Shift2 = { 575.0F, 1200.0F, 37.5F } ;

/* メッセージ先取り用 */
static	int		MsgBuf[ 8 ] ;

/* 水密ドアモード設定 */
static  FVECTOR	WtDoorMov ;
static	int	WtDoorOpen ;
static	int	WtDoorType ;

#if 0
static FVECTOR	MovShift = { 568.0F, 0.0F, 647.5F } ;
static FVECTOR	MovShift_2 = { -568.0F, 0.0F, 705.5F } ;
static FVECTOR	MovShift2 = { -578.0F, 0.0F, 600.5F } ;
static FVECTOR	MovShift2_2 = { 568.0F, 0.0F, 667.5F } ;
#else
/* スネーク */
static FVECTOR	SMovShiftD = { 560.0F, 0.0F, 600.0F } ;		/* 左引き r_pull */
static FVECTOR	SMovShiftD_2 = { -567.5F-15.0f, 0.0F, 705.0F } ;	/* 左押し r_push */
static FVECTOR	SMovShiftD2 = { -573.0F, 0.0F, 600.4F } ;	/* 右引き l_pull */
static FVECTOR	SMovShiftD2_2 = { 558.8F, 0.0F, 677.2F+40.0f } ;	/* 右押し l_push */

static FVECTOR	SMovShift = { 560.0F, 0.0F, 630.0F } ;		/* 左引き r_pull */
static FVECTOR	SMovShift_2 = { -557.5F-15.0f, 0.0F, 705.0F } ;	/* 左押し r_push */
static FVECTOR	SMovShift2 = { -563.0F, 0.0F, 630.4F } ;		/* 右引き l_pull */
static FVECTOR	SMovShift2_2 = { 558.8F, 0.0F, 677.2F+40.0f } ;	/* 右押し l_push */

/* ライデン */
static FVECTOR	RMovShiftD = { 595.0F-15.0f, 0.0F, 656.4F-15.0f } ;				/* 左引き r_pull */
static FVECTOR	RMovShiftD_2 = { -595.0F+15.0f, 0.0F, 656.4F + 75.0F-15.0f } ;	/* 左押し r_push */
static FVECTOR	RMovShiftD2 = { -573.9F, 0.0F, 635.9F } ;			/* 右引き l_pull */
static FVECTOR	RMovShiftD2_2 = { 573.9F, 0.0F, 635.9F + 75.0F } ;	/* 右押し l_push */

static FVECTOR	RMovShift = { 595.0F-15.0f, 0.0F, 656.4F-15.0f } ;				/* 左引き r_pull */
static FVECTOR	RMovShift_2 = { -595.0F+15.0f, 0.0F, 656.4F + 75.0F-15.0f } ;	/* 左押し r_push */
static FVECTOR	RMovShift2 = { -573.9F, 0.0F, 635.9F } ;			/* 右引き l_pull */
static FVECTOR	RMovShift2_2 = { 573.9F, 0.0F, 635.9F + 75.0F } ;	/* 右押し l_push */

static FVECTOR	MovShiftD, MovShiftD_2, MovShiftD2, MovShiftD2_2 ;
static FVECTOR	MovShift, MovShift_2, MovShift2, MovShift2_2 ;

#endif

/*------------------------------------------------------------------*/

#if 0
static	void	Local_SendMessage( to, buf, n )
int				to, *buf, n ;
{
	memcpy( MsgBuf, buf, sizeof( int ) * n ) ;
	MsgBuf[ 7 ] = to ;
}
#endif

/* ブロック実行 */
static	void	ExecProc( work, buf, n )
Work		*work ;
int		*buf, n ;
{
    GCL_ARGS	args ;

    args.argv = buf ;
    args.argc = n ;
    if ( work->exec != 0 ) {
		GM_ExecBlock( ( char * )work->exec, &args ) ;
    } else if ( work->proc != 0 ) {
		GM_ExecProc( work->proc, &args ) ;
    }
}

/*------------------------------------------------------------------*/

#include	"wt_door2.c"

/*------------------------------------------------------------------*/

/* 壁の回転 */
static	void	RotateSegment( work, rot )
Work		*work ;
SVECTOR		*rot ;
{
#ifndef NO_SEGMENT
    HZX_RotateDynamicSegment( work->segment, &work->segment->def[ 0 ], rot ) ;
#endif
}

/* 回転セット */
static	void	SetHandleRotate( work, rot )
Work		*work ;
short		rot ;
{
    SVECTOR	rotate ;
    FVECTOR	mov ;
    FMATRIX	mat ;
    short	rotvz ;

    rotvz = work->rotate.vz + rot ;
    if ( rotvz < 0 && work->rotate.vz > 0 ) rotvz = 0 ;
    else if ( rotvz < 0 && work->rotate.vz > 0 ) rotvz = 0 ;
    work->rotate.vz = rotvz ;

    DG_SetPos( &work->def_mat ) ;
    rotate.vx = rotate.vz = 0 ; rotate.vy = work->rotate.vy ;
    DG_RotatePos( &rotate ) ;
    DG_MovePos( &work->shift ) ;
    DG_GetPos( &mat ) ;

    DG_SetPos( &work->def_mat ) ;
    DG_RotatePos( &work->rotate ) ;
    DG_GetPos( &work->handle->world ) ;
    work->handle->world.m[ 3 ][ 0 ] = mat.m[ 3 ][ 0 ] ;
    work->handle->world.m[ 3 ][ 1 ] = mat.m[ 3 ][ 1 ] ;
    work->handle->world.m[ 3 ][ 2 ] = mat.m[ 3 ][ 2 ] ;
    
    GV_MatToVec( &work->handle->world, &mov ) ;
#ifndef PRESHADE
    DG_GetLightMatrix( &mov, work->light2 ) ;
#endif
}

static	void	SetHandleRotate2( work, msg )
Work		*work ;
GV_MSG		*msg ;
{
    FVECTOR	pmov, mov, axis, diff ;
    short	rot, rot2 ;

    pmov.vx = msg->message[ 1 ] ;
    pmov.vy = msg->message[ 2 ] ;
    pmov.vz = msg->message[ 3 ] ;
    mov.vx = msg->message[ 4 ] ;
    mov.vy = msg->message[ 5 ] ;
    mov.vz = msg->message[ 6 ] ;

    GV_MatToVec( &work->handle->world, &axis ) ;
    _sceVu0SubVector( &diff, &pmov, &axis ) ;
    diff.vx = _sceVu0InnerProduct( &diff, &work->normal ) ;
    diff.vz = diff.vy ;
    rot = GV_VecDir2( &diff ) ;
    _sceVu0SubVector( &diff, &mov, &axis ) ;
    diff.vx = _sceVu0InnerProduct( &diff, &work->normal ) ;
    diff.vz = diff.vy ;
    rot2 = GV_VecDir2( &diff ) ;
    SetHandleRotate( work, -( rot2 - rot ) ) ;
}

/*------------------------------------------------------------------*/

/* ドアを現在位置からrotだけ回転する */
static	void	SetDoorRotate( work, rot )
Work		*work ;
short		rot ;
{
    SVECTOR	rotate ;
    FMATRIX	mat ;
    short	rotvy ;

    rotvy = ( work->rotate.vy + rot ) & 4095 ;
    if ( rotvy >= 2048 ) rotvy -= 4096 ;
    if ( work->rotate.vy < 0 ) {
		if ( rotvy > 0 ) rotvy = 0 ;
    } else if ( work->rotate.vy > 0 ) {
		if ( rotvy < 0 ) rotvy = 0 ;
    }
    work->rotate.vy = rotvy ;

	//printf( "rotate %d\n", work->rotate.vy ) ;
    rotate = work->rotate ;
    rotate.vx = rotate.vz = 0 ;

    RotateSegment( work, &rotate ) ;

    DG_SetPos( &work->def_mat ) ;
    DG_RotatePos( &rotate ) ;
    DG_GetPos( &work->door->world ) ;

    DG_MovePos( &work->shift ) ;
    DG_GetPos( &mat ) ;
    
    DG_SetPos( &work->def_mat ) ;
    DG_RotatePos( &work->rotate ) ;
    DG_GetPos( &work->handle->world ) ;
    work->handle->world.m[ 3 ][ 0 ] = mat.m[ 3 ][ 0 ] ;
    work->handle->world.m[ 3 ][ 1 ] = mat.m[ 3 ][ 1 ] ;
    work->handle->world.m[ 3 ][ 2 ] = mat.m[ 3 ][ 2 ] ;
}

/* 開けたキャラの手の座標の移動距離から
   ドアの回転角度を求め、回転させる */
static	void	SetDoorRotate2( work, msg )
Work		*work ;
GV_MSG		*msg ;
{
    FVECTOR	ax, v1, v2, df ;
    short	rot, rot2 ;

    GV_MatToVec( &work->door->world, &ax ) ;
    v1.vx = msg->message[ 1 ] ;
    v1.vz = msg->message[ 2 ] ;
    v2.vx = msg->message[ 3 ] ;
    v2.vz = msg->message[ 4 ] ;
    _sceVu0SubVector( &df, &v1, &ax ) ;
    rot = GV_VecDir2( &df ) ;
    _sceVu0SubVector( &df, &v2, &ax ) ;
    rot2 = GV_VecDir2( &df ) ;
	//printf( "rot2 : " ) ;
    SetDoorRotate( work, GV_DiffDirS( rot, rot2 ) ) ;
}

/* 開けたキャラの現在の手の位置から
   ドアの回転角度を求め、回転させる */
static	void	SetDoorRotate3( work, msg )
Work		*work ;
GV_MSG		*msg ;
{
    FVECTOR	ax, v1, df ;
    short	rot, adj ;

    GV_MatToVec( &work->door->world, &ax ) ;
    v1.vx = msg->message[ 1 ] ;
    v1.vz = msg->message[ 2 ] ;
    adj = msg->message[ 3 ] ;
    _sceVu0SubVector( &df, &v1, &ax ) ;
    rot = ( GV_VecDir2( &df ) + adj - work->def_rot - 1024 ) & 4095 ;
	//printf( "rot3 : %d %d %d %d", work->rotate.vy, GV_VecDir2( &df ), work->def_rot, adj ) ;
    SetDoorRotate( work, GV_DiffDirS( work->rotate.vy, rot ) ) ;
}

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    int		n_msg, code ;
    int		buf[ 4 ] ;
    GV_MSG	*msg, Msg ;

    /* 光源更新 */
#ifndef PRESHADE
    {
		FVECTOR	mov ;
		GV_MatToVec( &work->door->world, &mov ) ;
		DG_GetLightMatrix( &mov, work->light ) ;
		GV_MatToVec( &work->handle->world, &mov ) ;
		DG_GetLightMatrix( &mov, work->light2 ) ;
    }
#endif   
	//    HZX_ViewDynamicSegment( work->segment ) ;

	/* メッセージ先取り */
	if ( MsgBuf[ 7 ] == work->name ) {
		MsgBuf[ 7 ] = -1 ;
		n_msg = 1 ;
		msg = &Msg ;
		Msg.message = MsgBuf ;
		Msg.message_len = 7 ;
	} else {
		n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	}
    while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
		case MSG_HOLD :
			if ( work->status == STATE_CLOSE ) { 
				work->status = STATE_WORKING ;
				work->chara = msg->message[ 1 ] ;
				buf[ 0 ] = EXEC_TRIG_HOLD_START ;
				buf[ 1 ] = work->chara ;
				ExecProc( work, buf, 2 ) ;
			}
			break ;
		case MSG_HDL_ROTATE :
			if ( work->status != STATE_WORKING ) break ;
			SetHandleRotate( work, msg->message[ 1 ] ) ;
			break ;
		case MSG_HDL_ROTATE2 :
			if ( work->status != STATE_WORKING ) break ;
			SetHandleRotate2( work, msg ) ;
			break ;
		case MSG_OPEN :
			break ;
		case MSG_HOLD_END :
			work->status = STATE_CLOSE ;
			buf[ 0 ] = EXEC_TRIG_HOLD_RELEASE ;
			buf[ 1 ] = work->chara ;
			ExecProc( work, buf, 2 ) ;
			break ;
		case MSG_OPEN_END :
#if 1
			work->status = STATE_OPEN ;
			work->count = 0 ;
#else
			work->status = STATE_CLOSE ;
			work->door->world = work->def_mat ;
			DG_SetPos( &work->def_mat ) ;
			DG_MovePos( &work->shift ) ;
			DG_GetPos( &work->handle->world ) ;
			work->rotate.vy = work->rotate.vz = 0 ;
			RotateSegment( work, &work->rotate ) ;
#endif
			buf[ 0 ] = EXEC_TRIG_OPEN_END ;
			buf[ 1 ] = work->chara ;
			ExecProc( work, buf, 2 ) ;
			break ;	    
		case MSG_DOOR_ROTATE :
			SetDoorRotate( work, msg->message[ 1 ] ) ;
			break ;
		case MSG_DOOR_ROTATE2 :
			SetDoorRotate2( work, msg ) ;
			break ;
		case MSG_DOOR_ROTATE3 :
			SetDoorRotate3( work, msg ) ;
			break ;	    
		case MSG_OPEN_START :
			buf[ 0 ] = EXEC_TRIG_OPEN_START ;
			buf[ 1 ] = msg->message[ 1 ] ;
			ExecProc( work, buf, 2 ) ;			
			break ;
		}
		msg ++ ;
    }
    if ( work->status == STATE_CLOSE &&
		work->rotate.vz != 0 ) {
		if ( work->rotate.vz < 0 ) SetHandleRotate( work, 32 ) ;
		else			   SetHandleRotate( work, -32 ) ;
    }
    if ( work->status == STATE_OPEN ) {
		if ( ++ work->count == 60 ) {
			work->count = 0 ;
			work->status = STATE_CLOSING ;
		}
    }
    if ( work->status == STATE_CLOSING ) {
		if ( work->rotate.vy < 0 ) {
			SetDoorRotate( work, 4 + work->count ) ;
		} else {
			SetDoorRotate( work, -( 4 + work->count ) ) ;
		}
		++ work->count ;
		if ( work->rotate.vy == 0 ) {
			work->rotate.vz = 0 ;
			work->status = STATE_CLOSE ;
			{
				FVECTOR	mov ;

				GV_MatToVec( &work->def_mat, &mov ) ;
				GM_SeSetMode( SD_A_MITCLO01, &mov, GM_SEMODE_NORMAL ) ;
			}
		}
    }
}

static	void	Die( work )
Work		*work ;
{
#ifdef KOREADD
	GM_FreeRouteIntrpt( &work->r_intrpt ) ;
#endif
#ifdef PRESHADE
	DG_FreePreshade( work->door ) ;
	DG_FreePreshade( work->handle ) ;
#endif
    DG_DequeueObjs( work->door ) ;
    DG_FreeObjs( work->door ) ;
    DG_DequeueObjs( work->handle ) ;
    DG_FreeObjs( work->handle ) ;
#ifndef NO_SEGMENT
    HZX_RemoveDynamicSegment( work->segment ) ;
#endif
}

/*------------------------------------------------------------------*/

static	inline	int	GetResources( work, name, where )
Work		*work ;
int		name, where ;
{
    DG_DEF	*def ;
    DG_OBJS	*objs ;
    IVECTOR	iv, ip[ 2 ] ;
    FVECTOR	mov, p[ 2 ] ;
    SVECTOR	rot ;
	HZX_GROUP_ID	hzx_id ;
	int				zn ;

    /* 名前 */
    work->name = name ;
    /* モデル */
    GCL_GetOption( 'k' ) ;
    def = ( DG_DEF * )GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) ) ;
    ASSERT( def != NULL ) ;
    work->door = DG_MakeObjs( def, BODY_FLAG, 0 ) ;
    def = ( DG_DEF * )GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) ) ;
    ASSERT( def != NULL ) ;
    work->handle = DG_MakeObjs( def, BODY_FLAG, 0 ) ;
    /* モデル位置設定 */
    objs = work->door ;
    ASSERT( objs != NULL ) ;
    GCL_GetOption( 'r' ) ;
    GCL_GetNextIV( ( int * )&iv ) ;
    rot.vx = iv.vx ;
    rot.vy = work->def_rot = iv.vy ;
    rot.vz = iv.vz ;
    GCL_GetOption( 'p' ) ;
    GCL_GetNextIV( ( int * )&iv ) ;
    GV_IVtoFV( &iv, &mov, 3 ) ;
    DG_SetPos2( &mov, &rot ) ;

    DG_GetPos( &objs->world ) ;
    DG_GetPos( &work->def_mat ) ;
    GM_GroupObjs( objs, where ) ;
    DG_QueueObjs( objs ) ;
#ifndef PRESHADE
	//    DG_GetLightMatrix( &mov, work->light ) ;
    DG_SetLightMatrix( objs, work->light ) ;
#else
    DG_MakePreshade( objs, GM_GetMap( where )->light ) ;
#endif
    /* タイプ */
    if ( GCL_GetOption( 't' ) != NULL ) {
		work->type = GCL_GetNextInt() ;
    } else {
		work->type = 0 ;
    }
    /* ハンドル */
    objs = work->handle ;
	if ( work->type == 0 ) {
		DG_COPY_VEC( &work->shift, &Shift ) ;
	} else {
		DG_COPY_VEC( &work->shift, &Shift2 ) ;
	}
    DG_MovePos( &work->shift ) ;
    DG_GetPos( &objs->world ) ;
    GM_GroupObjs( objs, where ) ;
    DG_QueueObjs( objs ) ;
    GV_MatToVec( &objs->world, &mov ) ;
#ifndef PRESHADE
//  DG_GetLightMatrix( &mov, work->light2 ) ;
    DG_SetLightMatrix( objs, work->light2 ) ;
#else
    DG_MakePreshade( objs, GM_GetMap( where )->light ) ;
#endif

    /* 面方向ベクトル */
    work->normal.vx = 1.0F ;
    work->normal.vy = work->normal.vz = 0.0F ;
    DG_RotVector( &work->normal, &work->normal, 1 ) ;

    /* 壁 */
    p[ 0 ].vy = p[ 1 ].vy = 0.0F ;
    p[ 0 ].vz = p[ 1 ].vz = 30.0F ;
    p[ 0 ].vx = -35.0F ;
    p[ 1 ].vx = 1100.0F ;

    DG_SetPos( &work->door->world ) ;
    DG_PutVector( p, p, 2 ) ;
    p[ 0 ].vw = p[ 1 ].vw = 2225.0F ;
    GV_FVtoIV( p, ip, 8 ) ;
#ifndef NO_SEGMENT
	hzx_id = GM_GetHzxGroupID( where ) ;
	hzx_id = HZX_GetHzxIDbyZone( hzx_id, &p[ 0 ], &zn ) ;
    work->segment = HZX_AddDynamicSegment( hzx_id, &ip[ 0 ], &ip[ 1 ], 
										   HZX_SEG_NO_HARITSUKI | HZX_SEG_NO_C4 ) ;
#endif
    /* 付属ブロック */
    if ( GCL_GetOption( 'e' ) != NULL ) {
		work->exec = GCL_GetNextInt() ;
    } else if ( GCL_GetOption( 'O' ) != NULL ) {
		work->proc = GCL_GetNextInt() ;
    }    
    work->status = STATE_CLOSE ;


#ifdef KOREADD
	{
		HZX_ZONE_ADD zone[2] ;
		int dir ;

		dir = rot.vy + 1024 ;
printf("1:pos[%f][%f][%f] dir[%d]\n",mov.vx,mov.vy,mov.vz,dir);

		HZX_GetInterruptZone( &mov, dir, 500, &zone[0] ) ;
		GM_SetRouteIntrpt( &work->r_intrpt, zone[0], zone[1], &mov, dir,
				ROOT_INTRPT_HINGED|ROOT_INTRPT_CLOSE|ROOT_INTRPT_NOENE, 0 ) ;
		GM_PutRoteIntrpt( &work->r_intrpt ) ;
	}
#endif



    return 0 ;
}

/*------------------------------------------------------------------*/

/* 水密ドア起動 */
void		*NewWatertightDoor( name, where )
int		name, where ;
{
    Work	*work ;

    ASSERT( N_Doors < MAX_Doors ) ;
	work = ( Work * )GV_CreateActor( ACTOR_LEVEL, GV_CLASS_OBJECT,
									sizeof( Work ), ACTOR_PRIO ) ;
    if ( work != NULL ) {
		if ( MotionExist ) {
			GV_SetActor( &( work->actor ), Act2, Die2 ) ;
			if ( GetResources2( work, name, where ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			if ( GetResources( work, name, where ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
    }
    Doors[ N_Doors ] = work ;
    N_Doors ++ ;
    return work ;
}

/* 水密ドア検索 */
static	Work	*SearchWtDoor( name )
int				name ;
{
	int			i ;
	Work		**d ;
	
	d = Doors ;
	for ( i = 0; i < N_Doors; i ++ ) {
		if ( d[ i ]->name == name ) return d[ i ] ;
	}
	return NULL ;
}

/*------------------------------------------------------------------
  
  プレイヤー行動
  
  ----------------------------------------------------------------*/

#if 0
/* 水密ドアつかむ */
static	void	WtDoorHold( work, time )
PlayerWork	*work ;
int		time ;
{
    static FVECTOR     	p_mov ;
    static int		loop_count ;
    PL_MOTION_SET	*MS ;
    FVECTOR		mov, sft ;
    SVECTOR		rot ;
    int			ftime ;
    int			buf[ 7 ] ;

    /*
       enum {
       MSG_HOLD = 0,
       MSG_HDL_ROTATE,
       MSG_OPEN,
       MSG_HOLD_END,
       MSG_OPEN_END,
       MSG_HDL_ROTATE2,
       MSG_DOOR_ROTATE,
       MSG_DOOR_ROTATE2,
       MSG_DOOR_ROTATE3,
       } ;
	*/
    MS = PL_MotionSet ;
    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE | FLAG_NO_IK ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		SetAction( work, MS->shared[ Mhatch_loop ], 0 ) ;
		if ( WtDoorOpen == 1 ) work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1 ;
		buf[ 0 ] = MSG_HOLD ;
		buf[ 1 ] = work->control.name ;
		Local_SendMessage( work->send_to, buf, 2 ) ;
		loop_count = 0 ;
		work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK ;
		return ;
    }

    switch( work->data ) {
    case 0 :
		if ( work->pad->status & PL_PAD_ACTION ) {
			GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world, &mov ) ;
			if ( work->data2 >= 1 && work->data2 <= 47 ) {
				buf[ 0 ] = MSG_HDL_ROTATE2 ; 
				buf[ 1 ] = p_mov.vx ;
				buf[ 2 ] = p_mov.vy ;
				buf[ 3 ] = p_mov.vz ;
				buf[ 4 ] = mov.vx ;
				buf[ 5 ] = mov.vy ;
				buf[ 6 ] = mov.vz ;
				Local_SendMessage( work->send_to, buf, 7 ) ;
			}
			DG_COPY_VEC( &p_mov, &mov ) ;
			work->data2 ++ ;
		} else {
			//	    SetMode( work, StandStill ) ;
			SetMode( work, PL_StillMode[ 0 ] ) ;
			work->control.skip_flag &= ~CTRL_SKIP_NEAR_CHECK ;
			UnsetStatus( PLAYER_FORCE ) ;
			PL_UnsetInvincible( work ) ;	
			buf[ 0 ] = MSG_HOLD_END ;
			Local_SendMessage( work->send_to, buf, 1 ) ;
			return ;
		}
//		if ( EndMotion( work ) ) {
//			work->data2 = 0 ;
//			if ( ++ loop_count == 3 ) {
		if ( EndMotion( work ) ) {
			work->data2 = 0 ;
			++ loop_count ;
		}
		if ( ( WtDoorOpen == 0 && loop_count == 3 ) ||
			 ( WtDoorOpen == 1 && loop_count == 2 && 
			   !EndMotion( work ) && MotionTime( work ) >= 48 ) ) {
			buf[ 0 ] = MSG_OPEN_START ;
			buf[ 1 ] = work->control.name ;
			Local_SendMessage( work->send_to, buf, 2 ) ;
//			if ( ++ loop_count == 3 ) {
				if ( WtDoorType == 0 ) { /* 左軸 */
					if ( WtDoorOpen == 0 ) { /* 引き */
						work->data = 1 ;
						SetAction( work, MS->shared[ Mhatch_open_r_pull ], 6 ) ;
					} else {	/* 押し */
						work->data = 4 ;
						SetAction( work, MS->shared[ Mhatch_open_l_push ], 6 ) ;
					}
				} else {		/* 右軸 */
					if ( WtDoorOpen == 0 ) { /* 引き */
						work->data = 3 ;
						SetAction( work, MS->shared[ Mhatch_open_l_pull ], 6 ) ;
					} else {	/* 押し */
						work->data = 2 ;
						SetAction( work, MS->shared[ Mhatch_open_r_push ], 6 ) ;
					}
				}
				work->data2 = 0 ;
				rot.vx = rot.vz = 0 ; rot.vy = work->control.rot.vy + 2048 ;
				DG_SetPos2( &WtDoorMov, &rot ) ;
				if ( WtDoorType == 0 ) {	/* 左軸 */
					if ( WtDoorOpen == 0 ) {		/* 引き */
						DG_COPY_VEC( &sft, &MovShift ) ;
					} else {						/* 押し */
						DG_COPY_VEC( &sft, &MovShift_2 ) ;
					}
				} else {					/* 右軸 */
					if ( WtDoorOpen == 0 ) {		/* 引き */
						DG_COPY_VEC( &sft, &MovShift2 ) ;
					} else {						/* 押し */
						DG_COPY_VEC( &sft, &MovShift2_2 ) ;
					}
				}
				DG_PutVector( &sft, &mov, 1 ) ;
				mov.vy = work->control.mov.vy ;
				GM_ResetControlPosition( &work->control, &mov ) ;
//			}
        }
		break ;
    case 1 :					/* 左軸、引き */
		if ( work->data2 < 55 ) {
			GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world, &mov ) ;
		} else {
			GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_TE ].world, &mov ) ;
		}
//		GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world, &mov ) ;
//		printf( "time %d\n", work->data2 ) ;
		if ( work->data2 < 7 ) {
			/* 何もしない */
		} else if ( work->data2 < 28 ) {
			buf[ 0 ] = MSG_DOOR_ROTATE3 ;
			buf[ 1 ] = mov.vx ;
			buf[ 2 ] = mov.vz ;
            buf[ 3 ] = 196 ;
			Local_SendMessage( work->send_to, buf, 4 ) ;
		} else if ( work->data2 < 55 ) {
			buf[ 0 ] = MSG_DOOR_ROTATE ;
			buf[ 1 ] = -18 ;
			Local_SendMessage( work->send_to, buf, 2 ) ;
		} else if ( work->data2 < 69 ) {
			buf[ 0 ] = MSG_DOOR_ROTATE3 ;
			buf[ 1 ] = mov.vx ;
			buf[ 2 ] = mov.vz ;
			buf[ 3 ] = -72 ;
			Local_SendMessage( work->send_to, buf, 4 ) ;
		} else {
			buf[ 0 ] = MSG_DOOR_ROTATE ;
			buf[ 1 ] = -10 ;
			Local_SendMessage( work->send_to, buf, 2 ) ;
		}
		DG_COPY_VEC( &p_mov, &mov ) ;
		work->data2 ++ ;
		break ;
    case 2 :					/* 左軸、押し */
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world, &mov ) ;
		if ( work->data2 < 38 ) {
			/* 何もしない */
		} else {
			buf[ 0 ] = MSG_DOOR_ROTATE ;
			buf[ 1 ] = 16 ;
			Local_SendMessage( work->send_to, buf, 2 ) ;
		}
		DG_COPY_VEC( &p_mov, &mov ) ;
		work->data2 ++ ;
		break ;
    case 3 :					/* 右軸、引き */
		if ( work->data2 < 55 ) {
			GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_TE ].world, &mov ) ;
		} else {
			GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world, &mov ) ;
		}
		//printf( "time %d\n", work->data2 ) ;
		if ( work->data2 < 7 ) {
			/* 何もしない */
		} else if ( work->data2 < 28 ) {
			buf[ 0 ] = MSG_DOOR_ROTATE3 ;
			buf[ 1 ] = mov.vx ;
			buf[ 2 ] = mov.vz ;
            buf[ 3 ] = -156 ;
			Local_SendMessage( work->send_to, buf, 4 ) ;
		} else if ( work->data2 < 55 ) {
			buf[ 0 ] = MSG_DOOR_ROTATE ;
			buf[ 1 ] = 20 ;
			Local_SendMessage( work->send_to, buf, 2 ) ;
		} else if ( work->data2 < 76 ) {
			buf[ 0 ] = MSG_DOOR_ROTATE3 ;
			buf[ 1 ] = mov.vx ;
			buf[ 2 ] = mov.vz ;
			buf[ 3 ] = 96 ;
			Local_SendMessage( work->send_to, buf, 4 ) ;
		} else {
			buf[ 0 ] = MSG_DOOR_ROTATE ;
			buf[ 1 ] = 4 ;
			Local_SendMessage( work->send_to, buf, 2 ) ;
		}
		DG_COPY_VEC( &p_mov, &mov ) ;
		work->data2 ++ ;
		break ;
    case 4 :					/* 右軸、押し */
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world, &mov ) ;
		if ( work->data2 < 38 ) {
			/* 何もしない */
		} else {
			buf[ 0 ] = MSG_DOOR_ROTATE ;
			buf[ 1 ] = -16 ;
			Local_SendMessage( work->send_to, buf, 2 ) ;
		}
		DG_COPY_VEC( &p_mov, &mov ) ;
		work->data2 ++ ;

    }
    if ( work->data >= 1 ) {
		if ( EndMotion( work ) ) {
			//	    SetMode( work, StandStill ) ;
			SetMode( work, PL_StillMode[ 0 ] ) ;
			work->control.skip_flag &= ~CTRL_SKIP_NEAR_CHECK ;
			UnsetStatus( PLAYER_FORCE ) ;
			PL_UnsetInvincible( work ) ;	
			buf[ 0 ] = MSG_OPEN_END ;
			Local_SendMessage( work->send_to, buf, 1 ) ;
		}	
    }
}
#endif

/* 水密ドアモード設定 */
static	int	SetWtDoorAct( work, msg, len )
PlayerWork	*work ;
GV_MSG		*msg ;
int		len ;
{
    int			dir, range, flag ;
    FVECTOR		mov, sft ;
    SVECTOR		rot ;

    /* 以下の時は無効 */
    if ( Status( PLAYER_SQUAT | PLAYER_GROUND | PLAYER_DAMAGED | PLAYER_DOWNED |
				PLAYER_INTRUDE | PLAYER_FORCE | PLAYER_BEYOND | PLAYER_CB_BOX |
				PLAYER_DEAD | PLAYER_LOCKER | PLAYER_ENEMY_HANG | PLAYER_ENEMY_PULL ) || 
		( WeaponType( work ) & ( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ) ||
		( ItemType( work ) & ( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ) ) return -1 ;
    work->send_to = msg->message[ 1 ] ;
    /* 状態チェック */
    if ( ( msg->message[ 2 ] & 7 ) != 0 ) return -1 ; /* 0 == 閉まっている */
    WtDoorType = ( msg->message[ 2 ] >> 4 ) & 1 ;

    /* 方向チェック */
    dir = msg->message[ 3 ] ;
    range = msg->message[ 4 ] ;
    if ( GV_DiffDirAbs( work->control.rot.vy, dir ) > range ) return -1 ;

#ifdef BP_PSX2_ASM
	asm volatile ( "NOP ###########################" );
#endif

    WtDoorMov.vx = mov.vx = msg->message[ 5 ] ;
    WtDoorMov.vz = mov.vz = msg->message[ 6 ] ;
    WtDoorMov.vy = 0.0F ;

    /* 開け方向 */
#if 0
    if ( len >= 8 ) WtDoorOpen = msg->message[ 7 ] ;
    else	    WtDoorOpen = 0 ;
#else
	{
		Work		*door ;

		door = SearchWtDoor( msg->message[ 1 ] ) ;
		ASSERT( door != NULL ) ;
		if ( WtDoorType == 0 ) {
			if ( dir == door->def_rot ) WtDoorOpen = 1 ;
			else						WtDoorOpen = 0 ;
		} else {
			if ( dir == door->def_rot ) WtDoorOpen = 0 ;
			else						WtDoorOpen = 1 ;
		}
		flag = door->flag ;

		if ( MotionExist ) {
			WtDoorMov.vx = mov.vx = door->body_door.objs->world.m[ 3 ][ 0 ] ;
			WtDoorMov.vz = mov.vz = door->body_door.objs->world.m[ 3 ][ 2 ] ;
		}
	}
#endif
    mov.vy = work->control.mov.vy ;
    rot.vx = rot.vz = 0 ;
    rot.vy = ( dir + 2048 ) & 4095 ;
    DG_SetPos2( &mov, &rot ) ;
	if ( WtDoorType == 0 ) {
		if ( WtDoorOpen == 0 ) {
			DG_COPY_VEC( &sft, &MovShift ) ;
		} else {
			DG_COPY_VEC( &sft, &MovShift_2 ) ;
		}
	} else {
		if ( WtDoorOpen == 0 ) {
			DG_COPY_VEC( &sft, &MovShift2 ) ;
		} else {
			DG_COPY_VEC( &sft, &MovShift2_2 ) ;
		}
	}
printf( "wt_doot.c : %s%s ", 
	WtDoorType ?  "R-"  : "L-" ,
	WtDoorOpen ? "Push" : "Pull"  ) ;
printf( " Shift(%.2f %.2f %.2f)->",sft.vx, sft.vy, sft.vz ) ;
    DG_PutVector( &sft, &mov, 1 ) ;
printf( "(%.2f %.2f %.2f)\n", mov.vx, mov.vy, mov.vz ) ;
    GM_ResetControlPosition( &work->control, &mov ) ;
    work->control.turn.vy = dir ;
    work->control.rot.vy = dir ;
    PL_SetInvincible( work, 0 ) ;
	PL_LeaveCaution( work ) ;
	PL_ClearCaptureTarget( work ) ;
	PL_UnequipSpecials() ;
	PL_SetMode2( work, NULL ) ;
    work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK ;
	work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
    SetFlag( FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_FORCE ) ;

	if ( MotionExist == 0 ) {
//		SetMode( work, WtDoorHold ) ;
	} else {
		work->dmg_callback = WtDoorDmgCallback ;
		if ( flag & WT_DOOR_FLAG_NOT_OPEN ) {
			SetMode( work, WtDoorNotOpen2 ) ;
		} else {
			SetMode( work, WtDoorHold2 ) ;
		}
	}
    return 1 ;
}

/*------------------------------------------------------------------
  
  水密ドアマネージャー起動
  
  ----------------------------------------------------------------*/

typedef	struct {
    GV_ACT		actor ;
    PL_PluginSet	plugin ;
} ManagerWork ;

/* 水密ドア管理 */
static	void	ManagerDie( work )
ManagerWork	*work ;
{
    GV_Free( Doors ) ;

    // AS(JM) - This is likely a BSS bug, but we should be able to fix it
    // by setting N_Doors to 0. Should stop things from iterating through
    // watertight doors after they're dead. MGSTWO-3474

    N_Doors = 0;
}

void	*NewWatertightDoorManager( name, where )
int	name, where ;
{	
    int		n ;
    ManagerWork	*work ;

	SeNo = 0 ;

	/* 音番号 */
	SeNo = GCL_GetOptionValue( 'N', 0 ) ;
	SeNo = ( SeNo & 0xf ) << 28 ;

	if ( GCL_GetOption( 'm' ) != NULL ) {
		/* モーションバージョン */
		MotionExist = 1 ;
		PlayerMotion = GCL_GetNextInt() ;
		DoorMotion = GCL_GetNextInt() ;
		HandleMotion = GCL_GetNextInt() ;
	} else {
		MotionExist = 0 ;
	}

    GCL_GetOption( 'n' ) ;
    n = MAX_Doors = GCL_GetNextInt() ;
    N_Doors = 0 ;
    Doors = ( Work ** )GV_Malloc( sizeof( Work * ) * n ) ;
	GV_ZeroMemory( Doors, sizeof( Work * ) * n ) ;

	/* プラント編ならライデン用設定 */
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
			DG_COPY_VEC( &MovShiftD, &SMovShiftD ) ;
			DG_COPY_VEC( &MovShiftD_2, &SMovShiftD_2 ) ;
			DG_COPY_VEC( &MovShiftD2, &SMovShiftD2 ) ;
			DG_COPY_VEC( &MovShiftD2_2, &SMovShiftD2_2 ) ;
			DG_COPY_VEC( &MovShift, &SMovShift ) ;
			DG_COPY_VEC( &MovShift_2, &SMovShift_2 ) ;
			DG_COPY_VEC( &MovShift2, &SMovShift2 ) ;
			DG_COPY_VEC( &MovShift2_2, &SMovShift2_2 ) ;
		} else {
			DG_COPY_VEC( &MovShiftD, &RMovShiftD ) ;
			DG_COPY_VEC( &MovShiftD_2, &RMovShiftD_2 ) ;
			DG_COPY_VEC( &MovShiftD2, &RMovShiftD2 ) ;
			DG_COPY_VEC( &MovShiftD2_2, &RMovShiftD2_2 ) ;
			DG_COPY_VEC( &MovShift, &RMovShift ) ;
			DG_COPY_VEC( &MovShift_2, &RMovShift_2 ) ;
			DG_COPY_VEC( &MovShift2, &RMovShift2 ) ;
			DG_COPY_VEC( &MovShift2_2, &RMovShift2_2 ) ;
		}
	} else if ( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
		DG_COPY_VEC( &MovShiftD, &SMovShiftD ) ;
		DG_COPY_VEC( &MovShiftD_2, &SMovShiftD_2 ) ;
		DG_COPY_VEC( &MovShiftD2, &SMovShiftD2 ) ;
		DG_COPY_VEC( &MovShiftD2_2, &SMovShiftD2_2 ) ;
		DG_COPY_VEC( &MovShift, &SMovShift ) ;
		DG_COPY_VEC( &MovShift_2, &SMovShift_2 ) ;
		DG_COPY_VEC( &MovShift2, &SMovShift2 ) ;
		DG_COPY_VEC( &MovShift2_2, &SMovShift2_2 ) ;
	} else {
		DG_COPY_VEC( &MovShiftD, &RMovShiftD ) ;
		DG_COPY_VEC( &MovShiftD_2, &RMovShiftD_2 ) ;
		DG_COPY_VEC( &MovShiftD2, &RMovShiftD2 ) ;
		DG_COPY_VEC( &MovShiftD2_2, &RMovShiftD2_2 ) ;
		DG_COPY_VEC( &MovShift, &RMovShift ) ;
		DG_COPY_VEC( &MovShift_2, &RMovShift_2 ) ;
		DG_COPY_VEC( &MovShift2, &RMovShift2 ) ;
		DG_COPY_VEC( &MovShift2_2, &RMovShift2_2 ) ;
	}

	/* 位置調整 */
	if ( GCL_GetOption( 'a' ) != NULL ) {
		float		vx, vz ;

		vx = ( float )GCL_GetNextInt() ;
		vz = ( float )GCL_GetNextInt() ;
		MovShift.vx = MovShiftD.vx + vx ;
		MovShift.vz = MovShiftD.vz + vz ;
		vx = ( float )GCL_GetNextInt() ;
		vz = ( float )GCL_GetNextInt() ;
		MovShift_2.vx = MovShiftD_2.vx + vx ;
		MovShift_2.vz = MovShiftD_2.vz + vz ;
		vx = ( float )GCL_GetNextInt() ;
		vz = ( float )GCL_GetNextInt() ;
		MovShift2.vx = MovShiftD2.vx + vx ;
		MovShift2.vz = MovShiftD2.vz + vz ;
		vx = ( float )GCL_GetNextInt() ;
		vz = ( float )GCL_GetNextInt() ;
		MovShift2_2.vx = MovShiftD2_2.vx + vx ;
		MovShift2_2.vz = MovShiftD2_2.vz + vz ;
	}

    work = ( ManagerWork * )GV_NewActor( GV_ACTOR_AFTER, sizeof( ManagerWork ) ) ;
    if ( work != NULL ) GV_SetActor( &( work->actor ), NULL, ManagerDie ) ;
    PL_AddPlugin( &work->plugin, PL_MSG_WT_DOOR, SetWtDoorAct, NULL ) ;
	NikaitenProc = 0 ;
	NikaitenMode = 0 ;
    return work ;
}

/* あるドアの状態を返す */
int	NewWatertightDoorStatus( void )
{
    int		name, i, res ;
    Work	*work ;

    GCL_GetOption( 'n' ) ;
    name = GCL_GetNextInt() ;
    for ( i = 0; i < N_Doors; i ++ ) {
		work = Doors[ i ] ;
		if ( work->name == name ) {
			res = ( work->status & 7 ) | ( ( work->type & 1 ) << 4 ) ;
			return res ;
		}
    }
    return -1 ;
}

/*---------------------------------------------------------------------*/

/* 手のモーフィング */
#define	HAND_CHANGE_ACT	GV_StrCode( "プレイヤー手アニメ水密ドア" ) 
#define	N_WT_MOTIONS		(8)
#define	MAX_TIMINGS			(16)

extern	void *NewHandsChange_called(int name,DG_OBJS *human,
									int r_hand_model,int *r_cv2_models,int r_cv2_size,
									int l_hand_model,int *l_cv2_models,int l_cv2_size) ;

typedef	struct	{
	u_char	time, pattern ;
} TIMING ;

typedef	struct	{
	GV_ACT		actor ;
	void		*vanime_work ;
	int			flag ;
	int			count ;
	int			ptime ;
	TIMING		rtiming[ N_WT_MOTIONS ][ MAX_TIMINGS ] ;
	TIMING		ltiming[ N_WT_MOTIONS ][ MAX_TIMINGS ] ;
} HandAnimeWork ;

static	void	CheckTiming( work, mode, ptime, mtime, timing )
Work			*work ;
int				mode ;
int				ptime ;
int				mtime ;
TIMING			*timing ;
{
	int			i, buf[ 4 ] ;

	for ( i = 0; i < MAX_TIMINGS; i ++ ) {
		if ( ptime < timing[ i ].time && mtime >= timing[ i ].time ) break ;
		else if ( mtime < timing[ i ].time ) return ;
	}
	ASSERT( i < MAX_TIMINGS ) ;
	buf[ 0 ] = mode ;
	buf[ 1 ] = timing[ i ].pattern ;
	buf[ 2 ] = 4 ;
	PL_SendMessage( HAND_CHANGE_ACT, buf, 3 ) ;
//printf( "mode %d time %d pattern %d\n", mode, mtime, buf[ 1 ] ) ;
}

static	void	HandAnimeAct( work )
HandAnimeWork	*work ;
{
	int				msgbuf[ 4 ], motion, mtime ;
	TIMING			*rtiming, *ltiming ;
	PlayerWork		*pwork ;
	PL_MOTION_SET	*ms ;

	pwork = GM_PlayerWork ;
    ms = PL_MotionSet ;
#if 0
	if ( pwork->action != WtDoorHold && 
		 pwork->action != WtDoorHold2 && 
		 pwork->action != WtDoorNotOpen2 ) {
#else
	if ( pwork->action != WtDoorHold2 && 
		 pwork->action != WtDoorNotOpen2 ) {
#endif
		if ( work->flag == 1 ) {
			work->flag = 0 ;
			work->count = 6 ;
		}
		if ( work->count > 0 ) {
			if ( -- work->count == 0 ) {
				msgbuf[ 1 ] = 0 ;	/* 非表示 */
				msgbuf[ 0 ] = 1 ;
				PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
				msgbuf[ 0 ] = 3 ;
				PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
			}
		}
		return ;
	}

	if ( work->flag == 0 ) {
		/* 表示 */
		msgbuf[ 1 ] = 1 ;
		msgbuf[ 0 ] = 1 ;
		PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
		msgbuf[ 0 ] = 3 ;
		PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
		work->ptime = -1 ;
	}

	work->flag = 1 ;
	motion = GM_PlayerMotion ;
	mtime = PL_MotionTime( pwork ) ;

	if ( MotionExist == 0 ) {
#if 0
		if ( motion == ms->shared[ Mhatch_loop ] ) {
			rtiming = work->rtiming[ 0 ] ;
			ltiming = work->ltiming[ 0 ] ;
		} else if ( motion == ms->shared[ Mhatch_open_r_pull ] ) {
			rtiming = work->rtiming[ 1 ] ;
			ltiming = work->ltiming[ 1 ] ;
		} else if ( motion == ms->shared[ Mhatch_open_l_push ] ) {
			rtiming = work->rtiming[ 2 ] ;
			ltiming = work->ltiming[ 2 ] ;
		} else if ( motion == ms->shared[ Mhatch_open_l_pull ] ) {
			rtiming = work->rtiming[ 3 ] ;
			ltiming = work->ltiming[ 3 ] ;
		} else if ( motion == ms->shared[ Mhatch_open_r_push ] ) {
			rtiming = work->rtiming[ 4 ] ;
			ltiming = work->ltiming[ 4 ] ;
		} else return ;
#endif
		return ;
	} else {
		if ( motion == PMloop ) {
			rtiming = work->rtiming[ 0 ] ;
			ltiming = work->ltiming[ 0 ] ;
		} else if ( motion == PMr_pull ) {
			rtiming = work->rtiming[ 1 ] ;
			ltiming = work->ltiming[ 1 ] ;
		} else if ( motion == PMl_push ) {
			rtiming = work->rtiming[ 2 ] ;
			ltiming = work->ltiming[ 2 ] ;
		} else if ( motion == PMl_pull ) {
			rtiming = work->rtiming[ 3 ] ;
			ltiming = work->ltiming[ 3 ] ;
		} else if ( motion == PMr_push ) {
			rtiming = work->rtiming[ 4 ] ;
			ltiming = work->ltiming[ 4 ] ;
		} else if ( motion == PMloop_rev ) {
			rtiming = work->rtiming[ 5 ] ;
			ltiming = work->ltiming[ 5 ] ;
		} else if ( motion == PMnot_open ) {
			rtiming = work->rtiming[ 6 ] ;
			ltiming = work->ltiming[ 6 ] ;
		} else return ;
	}
	CheckTiming( work, 0, work->ptime, mtime, rtiming ) ;
	CheckTiming( work, 2, work->ptime, mtime, ltiming ) ;
	work->ptime = mtime ;
}

static	void	HandAnimeDie( work )
HandAnimeWork	*work ;
{
	GV_DestroyActor( work->vanime_work ) ;
}

static	int		HandAnimeGetResources( work )
HandAnimeWork	*work ;
{
	int		rhandmdls[ 16 ], lhandmdls[ 16 ] ;
	int		n_r_models, n_l_models ;
	int		i, j ;
	int		motion, cur[ N_WT_MOTIONS ] ;

	GCL_GetOption( 'p' ) ;
	n_r_models = GCL_GetNextInt() ;
	n_l_models = GCL_GetNextInt() ;

	GCL_GetOption( 'r' ) ;
	for ( i = 0; i < n_r_models; i ++ ) {
		rhandmdls[ i ] = GCL_GetNextInt() ;
	}
	GCL_GetOption( 'l' ) ;
	for ( i = 0; i < n_l_models; i ++ ) {
		lhandmdls[ i ] = GCL_GetNextInt() ;
	}
	work->vanime_work = NewHandsChange_called( HAND_CHANGE_ACT, GM_PlayerBody->objs,
											  rhandmdls[ 0 ], rhandmdls, n_r_models,
											  lhandmdls[ 0 ], lhandmdls, n_l_models ) ;
	if ( work->vanime_work == NULL ) return -1 ;
	
	/* タイミングデータの読み込み */
	for ( i = 0; i < N_WT_MOTIONS; i ++ ) cur[ i ] = 0 ;
	for ( i = 0; i < N_WT_MOTIONS; i ++ ) {
		for ( j = 0; j < MAX_TIMINGS; j ++ ) {
			work->rtiming[ i ][ j ].time = 255 ;
			work->ltiming[ i ][ j ].time = 255 ;
		}
	}

	GCL_GetOption( 't' ) ;
	while( GCL_NextStr() != NULL ) {
		motion = GCL_GetNextInt() ;
		ASSERT( cur[ motion ] < MAX_TIMINGS ) ;
		work->rtiming[ motion ][ cur[ motion ] ].time = GCL_GetNextInt() ;
		work->rtiming[ motion ][ cur[ motion ] ].pattern = GCL_GetNextInt() ;
		cur[ motion ] ++ ;
	}

	for ( i = 0; i < N_WT_MOTIONS; i ++ ) cur[ i ] = 0 ;
	GCL_GetOption( 'I' ) ;
	while( GCL_NextStr() != NULL ) {
		motion = GCL_GetNextInt() ;
		ASSERT( cur[ motion ] < MAX_TIMINGS ) ;
		work->ltiming[ motion ][ cur[ motion ] ].time = GCL_GetNextInt() ;
		work->ltiming[ motion ][ cur[ motion ] ].pattern = GCL_GetNextInt() ;
		cur[ motion ] ++ ;
	}
	{
		int		msgbuf[ 2 ] ;
		/* 最初は非表示 */
		msgbuf[ 1 ] = 0 ;
		msgbuf[ 0 ] = 1 ;
		PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
		msgbuf[ 0 ] = 3 ;
		PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
	}
	work->flag = 0 ;
	return 0 ;
}

void	*NewWtDoorPlayerHandMofingManager( name, where )
int		name, where ;
{	
    HandAnimeWork	*work ;

    work = ( HandAnimeWork * )GV_NewActor( GV_ACTOR_AFTER, sizeof( HandAnimeWork ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), HandAnimeAct, HandAnimeDie ) ;
		if ( HandAnimeGetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
    return work ;
}
