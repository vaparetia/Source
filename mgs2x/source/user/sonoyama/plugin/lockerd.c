//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   lockerd.c
   
   ロッカー管理
   1999/12/14 M.Sonoyama
   $Id: lockerd.c,v 1.1.1.3 2002/11/19 11:50:49 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
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
#include	"locker.h"
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

/*------------------------------------------------------------------*/

//#define	DOOR_ONLY

#define	BODY_FLAG	(DG_FLAG_PAINT)
#define	LOCKER_TRAP	(2308593)	/* "locker" */

#define	PAD_LOCKER	(PL_PAD_ACTION)

/*------------------------------------------------------------------*/

/* 新ロッカー構造体 */

#define KOREADD

typedef	struct	{
	FVECTOR			kissb1, kissb2 ;
	HZX_D_SEGMENT	*special1 ;
	HZX_D_SEGMENT	*special2 ;
} GRAVURE_SET ;

typedef	ALIGN16_DECL(struct) _LOCKER2 {
	OBJECT		body ;
	FMATRIX		shadow_world ;
	FMATRIX		def_world ;
	FVECTOR		step ;
	FVECTOR		center ;
	FVECTOR		down_shift ;
	FVECTOR		down_shift_cur ;
	TARGET		target ;
	TARGET		attack ;
	POWER_TARGET	power ;

    int			name ;
    int			status ;
    int			command ;
	int			motion ;

    int			count ;
    int			proc ;
    int			exec ;
    int			chara ;

    short		def_rot ;
    short		rot ;
    int			sound[ 3 ] ;

	void		*shadow_obj ;
    HZX_D_SEGMENT	*segment ;
    //HZX_D_SEGMENT	*segment2 ;
    //HZX_D_SEGMENT	*segment3 ;
	void		*objhzx ;
	int			reserved ;

	TARGET			t1 ;

	int				next ;
	DG_OBJS			*box ;
    HZX_D_SEGMENT	*b_seg1 ;
    HZX_D_SEGMENT	*b_seg2 ;	

	SVECTOR			lean_rot ;
	SVECTOR			down_rot ;

	HZX_D_FLOOR		*b_flr ;
	int				lean ;
	int				lean_level ;
	int				hit_count ;

	int				flag ;

	int				brk_phase ;
	HZX_ZONE_ADD	zones[ 2 ] ;
#ifdef KOREADD
	R_INTRPT		r_intrpt ;	/* ゾーンインタラプト */
#endif
	HZX_D_SEGMENT	*segment4 ;
	HZX_D_SEGMENT	*segment5 ;
	HZX_D_SEGMENT	*segment6 ;
	int				sefloor ;
	HZX_D_FLOOR		*floor ;
	CAPTURE_TARGET	*capture ;

	GRAVURE_SET		*gravure ;
	OBJECT			shadow_write ;

	SAR_CONTROL		*sar_ctrl ;
	int				msg_buf[ 6 ] ;

	int				puttargettime ;
} LOCKER2  ;

/*------------------------------------------------------------------*/

/* ロッカー設定用構造体 */
typedef	struct	{
    int		name ;
    short	flag ;
    short	status ;
    short	dir ;
    short	range ;
    float	mov_x, mov_z ;
} LockerSet ;

enum {
    LS_FLAG_MOVE = 0x0001,
} ;

typedef	struct {
    GV_ACT		actor ;
    u_short		N_Lockers ;
    u_short		MAX_Lockers ;
    LOCKER2		*Lockers ;
    GCL_ARGS	args ;
    int			buf[ 8 ] ;
    PL_PluginSet	plugin ;
    PL_PollingSet	polling ;
} Work ;

static	Work		*LockerWork ;

#define	N_Lockers	(LockerWork->N_Lockers)
#define	MAX_Lockers	(LockerWork->MAX_Lockers)
#define	Lockers		(LockerWork->Lockers)
#define	ARGS		(&(LockerWork->args))
#define	ARGBUF		(LockerWork->buf)

#define	OPEN_COUNT	(30)

#define	OPEN_TIME_S	(10)
#define	OPEN_TIME	(OPEN_TIME_S + OPEN_COUNT)
#define	OPEN_TIME_E	(OPEN_TIME + 20)
#define	OPEN_ROT	(( float )M_PI * 80.0F / 180.0F)

/*------------------------------------------------------------------*/

static	int			PlayerLockerMotion = 0 ;
//static	int			Type = 0 ;
static	LockerSet	Locker ;

enum {
	Mopen_out = 0,
	Mclose_out,
	Mopen_in,
	Mclose_in,
	Mclose_out_turn,
	Menemy_reserved,
	Mnot_open,
	Msurprise,
	Min_still,
	Mmove_r,
	Mmove_l,
	Mbody_put,
	Mleg_put_f,
	Mleg_put_b,
	Mopen_out_behind
} ;

/*------------------------------------------------------------------*/

extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;

//static	int	SetLockerAct( PlayerWork *, GV_MSG *, int ) ;
static	int	SetLockerAct2( PlayerWork *, GV_MSG *, int ) ;

/*------------------------------------------------------------------*/

#include	"lckcnct.c"
#include	"locker2.c"

/*------------------------------------------------------------------*/

#if 0
static	int	CheckMessage( this )
LOCKER		*this ;
{
    int		n_msg ;
    GV_MSG	*msg ;

    n_msg = GV_ReceiveMessage( this->name, &msg ) ;
    if ( n_msg == 0 ) return 0 ;
    while( -- n_msg >= 0 ) {
		switch( msg->message[ 0 ] ) {
		case LOCKER_MSG_OPEN :
		case LOCKER_MSG_CLOSE :
			this->chara = msg->message[ 1 ] ;
			this->status &= ~( LOCKER_STATE_FROM_OUT | LOCKER_STATE_FROM_IN ) ;
			if ( msg->message[ 2 ] == 0 ) this->status |= LOCKER_STATE_FROM_OUT ;
			else			  this->status |= LOCKER_STATE_FROM_IN ;
			this->command = LOCKER_COMMAND_CHANGE ;
			break ;
		case LOCKER_MSG_PUT_ENEMY :
			break ;
		case LOCKER_MSG_PUT_CORPSE :
			break ;
		case LOCKER_MSG_END :
			this->status |= LOCKER_STATE_END ;
			break ;
		case LOCKER_MSG_ROTATE :
			this->status |= LOCKER_STATE_ROTATE ;
			this->hand.vx = msg->message[ 1 ] ;
			this->hand.vz = msg->message[ 2 ] ;
			this->hand.vy = msg->message[ 3 ] ;
			break ;
		case LOCKER_MSG_ROTATE_D :
			this->status |= LOCKER_STATE_ROTATE_D ;
			this->rot += msg->message[ 1 ] ;
			break ;
		default :
		}
		msg ++ ;
    }
    return 1 ;
}

/*------------------------------------------------------------------*/

/* 壁の回転 */
static	void	RotateSegment( this, rot )
LOCKER		*this ;
int		rot ;
{
    SVECTOR	rotate ;

    rotate.vx = rotate.vz = 0 ;
    rotate.vy = rot ;
    HZX_RotateDynamicSegment( this->segment, &this->segment->def[ 0 ], &rotate ) ;
}

/* 回転セット */
static	inline	void	SetRotate( this, next )
LOCKER		*this ;
int		next ;
{
    FVECTOR	hand, diff ;
    short	rot ;
    float	rad ;

    GV_IVtoFV( &this->hand, &hand, 3 ) ;
    _sceVu0SubVector( &diff, &hand, &this->axis ) ;
    rot = ( GV_VecDir2( &diff ) + 3072 - this->def_rot + this->hand.vy ) & 4095 ;
    if ( rot >= 2048 ) rot -= 4096 ;
    if ( rot < 0 ) rot = 0 ;	/* －回転禁止 */
    if ( rot > 1024 ) rot = 1024 ; /* ９０度以上禁止 */
    this->rot = rot ;
    rad = ( float )rot * ( float )M_PI / 2048.0F ;
    if ( rad < 0.0F ) rad = 0.0F ;
    this->rots[ 1 ].vy = rad ;
    this->rots[ 2 ].vy = rad ;
    this->rots[ 3 ].vy = rad ;
    RotateSegment( this, rot ) ;
}

static	inline	void	SetRotate2( this )
LOCKER		*this ;
{
    short	rot ;
    float	rad ;

    rot = this->rot & 4095 ;
    if ( rot >= 2048 ) rot -= 4096 ;
    if ( rot < 0 ) rot = 0 ;	/* －回転禁止 */
    if ( rot > 1024 ) rot = 1024 ; /* ９０度以上禁止 */
    this->rot = rot ;	
    rad = ( float )rot * ( float )M_PI / 2048.0F ;
    if ( rad <= 0.0F ) rad = 0.0F ;
    this->rots[ 1 ].vy = rad ;
    this->rots[ 2 ].vy = rad ;
    this->rots[ 3 ].vy = rad ;
    RotateSegment( this, rot ) ;
}

/*------------------------------------------------------------------*/

/* ロッカー開閉 */
static	void	MoveLocker( this, next )
LOCKER		*this ;
int		next ;
{
    int		status ;

    status = this->status ;
    if ( status & LOCKER_STATE_ROTATE ) SetRotate( this, next ) ;
    else if ( status & LOCKER_STATE_ROTATE_D ) SetRotate2( this ) ;
    if ( status & LOCKER_STATE_END ) {
		this->status |= next ;
		this->command = LOCKER_COMMAND_NOTHING ;
		this->count = 0;
		ARGS->argc = 3 ;
		ARGBUF[ 0 ] = 0 ;
		ARGBUF[ 1 ] = this->chara ;
		ARGBUF[ 2 ] = ( next == LOCKER_STATE_OPEN ) ? 0 : 1 ;
		if ( status & LOCKER_STATE_FROM_IN ) ARGBUF[ 2 ] += 2 ;
		if ( this->proc != 0 ) {
			GM_ExecProc( this->proc, ARGS ) ;
		} else if ( this->exec != 0 ) {
			GM_ExecBlock( ( char * )this->exec, ARGS ) ;
		}
		if ( next == LOCKER_STATE_CLOSE ) {
			this->rots[ 1 ].vy = 0.0F ;
			this->rots[ 2 ].vy = 0.0F ;
			this->rots[ 3 ].vy = 0.0F ;
			this->rot = 0 ;
			RotateSegment( this, this->rot ) ;
		} else {
			this->status &= ~LOCKER_STATE_CORPSE_IN ;
		}
        this->status &= ~LOCKER_STATE_SELF_CLOSE ;
    }
}

/* 開ける */
static	void	OpenLocker( this ) 
LOCKER		*this ;
{
    MoveLocker( this, LOCKER_STATE_OPEN ) ;
}

/* 閉まる */
static	void	CloseLocker( this )
LOCKER		*this ;
{
    MoveLocker( this, LOCKER_STATE_CLOSE ) ;
}

/* 寄りかかられ閉じ */
static	int	CloseLockerbyLeaning( this )
LOCKER		*this ;
{
    HZX_SEG		*seg ;
    HZX_D_SEGMENT	*dseg ;
    FVECTOR		from, to ;

    /* 開き状態で張り付かれたら閉まる */
    if ( this->status & LOCKER_STATE_OPEN ) {
		if ( GM_PlayerWork != NULL && 
			( GM_CheckPlayerStatus( PLAYER_CAUTION ) ) &&
			GM_PlayerWork->control.n_touches != 0 &&
			( GM_PlayerWork->control.atrs[ 0 ] & HZX_SEG_DYNAMIC ) ) {
			seg = GM_PlayerWork->control.segs[ 0 ] ;
			dseg = this->segment ;
			ASSERT( seg != NULL && dseg != NULL ) ;
			if ( DG_FABS( seg->p1.x - ( float )dseg->p1.vx ) < 0.10F &&

				DG_FABS( seg->p1.z - ( float )dseg->p1.vz ) < 0.10F &&
				DG_FABS( seg->p1.y - ( float )dseg->p1.vy ) < 0.10F &&
				DG_FABS( seg->p1.h - ( float )dseg->p1.vw ) < 0.10F &&
				DG_FABS( seg->p2.x - ( float )dseg->p2.vx ) < 0.10F &&
				DG_FABS( seg->p2.z - ( float )dseg->p2.vz ) < 0.10F &&
				DG_FABS( seg->p2.y - ( float )dseg->p2.vy ) < 0.10F &&
				DG_FABS( seg->p2.h - ( float )dseg->p2.vw ) < 0.10F ) {
				DG_COPY_VEC( &from, &GM_PlayerWork->control.mov ) ;
				GV_IVtoFV( &dseg->def[ 1 ], &to, 3 ) ;
				to.vy = from.vy ;
				if ( HZX_OnlineHazardCheckOneSegment( seg, &from, &to ) ) {
					/* 閉じる */
					this->status &= ~LOCKER_STATE_OPEN ;
					this->status |= LOCKER_STATE_SELF_CLOSE | LOCKER_STATE_ROTATE_D ;
					GM_SeSetMode( this->sound[ LOCKER_SE_MOVING ], &this->axis, GM_SEMODE_NORMAL ) ;
				}
			}
		}
    }
    /* よりかかられ閉じ */
    if ( this->status & LOCKER_STATE_SELF_CLOSE ) {
		this->rot -= 32 ;
		CloseLocker( this ) ;
		if ( this->rot == 0 && 
			( this->status & LOCKER_STATE_SELF_CLOSE ) ) {
			GM_SeSetMode( this->sound[ LOCKER_SE_CLOSE ], &this->axis, GM_SEMODE_NORMAL ) ;
			this->status |= LOCKER_STATE_END ;
		}
		return 1 ;
    }
    return 0 ;
}


/*------------------------------------------------------------------*/

static	void	ActBox( work )
Work		*work ;
{
    LOCKER	*this ;
    int		i, com ;

    /* 開ける閉める等を一括管理 */
    this = Lockers ;
    for ( i = 0; i < N_Lockers; i ++, this ++ ) {
		//HZX_ViewDynamicSegment( this->segment ) ;	
		/* よりかかり閉じ */
		if ( CloseLockerbyLeaning( this ) ) continue ;
		if ( CheckMessage( this ) == 0 ) continue ;
		com = this->command ;
		switch( com ) {
		case LOCKER_COMMAND_NOTHING :
			break ;
		case LOCKER_COMMAND_CLOSE :
			CloseLocker( this ) ;
			break ;
		case LOCKER_COMMAND_OPEN :
			OpenLocker( this ) ;
			break ;
		case LOCKER_COMMAND_CHANGE :
			if ( this->status & LOCKER_STATE_CLOSE ) {
				this->command = LOCKER_COMMAND_OPEN ;	
				if ( this->status & LOCKER_STATE_FROM_IN ) work->buf[ 2 ] = 2 ;
				else					   work->buf[ 2 ] = 0 ;
			} else if ( this->status & LOCKER_STATE_OPEN ) {
				this->command = LOCKER_COMMAND_CLOSE ;
				if ( this->status & LOCKER_STATE_FROM_IN ) work->buf[ 2 ] = 3 ;
				else					   work->buf[ 2 ] = 1 ;
			} else {
				this->command = LOCKER_COMMAND_NOTHING ;
			}
			if ( this->command != LOCKER_COMMAND_NOTHING ) {
				this->status &= ~( LOCKER_STATE_CLOSE | LOCKER_STATE_OPEN ) ;
				this->count = 0 ;
				work->args.argc = 3 ;
				work->buf[ 0 ] = 1 ;
				work->buf[ 1 ] = this->chara ;
				if ( this->proc != 0 ) {
					GM_ExecProc( this->proc, &work->args ) ;
				} else if ( this->exec != 0 ) {
					GM_ExecBlock( ( char * )this->exec, &work->args ) ;
				}
			}
			break ;
		default :
		}
		this->status &= ~( LOCKER_STATE_END | LOCKER_STATE_ROTATE | LOCKER_STATE_ROTATE_D ) ;
    }
}

static	void	DieBox( work )
Work		*work ;
{
    int		i ;

    if ( Lockers != NULL ) {
		for ( i = 0; i < N_Lockers; i ++ ) {
			DG_FreePreshade( Lockers[ i ].objs ) ;
			DG_DequeueObjs( Lockers[ i ].objs ) ;
			DG_FreeObjs( Lockers[ i ].objs ) ;
			HZX_RemoveDynamicSegment( Lockers[ i ].segment ) ;
			HZX_RemoveDynamicSegment( Lockers[ i ].segment2 ) ;
			HZX_RemoveDynamicSegment( Lockers[ i ].segment3 ) ;
			HZX_RemoveDynamicFloor( Lockers[ i ].floor ) ;
			if ( Lockers[ i ].shadow_obj != NULL ) {
				GV_DestroyActor( Lockers[ i ].shadow_obj ) ;
			}
		}
		GV_DelayedFree( Lockers ) ;
    }
}

#endif

static	void	Act( work )
Work			*work ;
{
//	if ( Type == 0 ) {
//		ActBox( work ) ;
//	} else {
		ActDoor( work ) ;
//	}
}

static	void	Die( work )
Work			*work ;
{
//	if ( Type == 0 ) {
//		DieBox( work ) ;
//	} else {
		DieDoor( work ) ;
//	}
}

/*------------------------------------------------------------------*/

static	inline	int	GetResources( work )
Work		*work ;
{
	int			size ;

    Lockers = NULL ;
    N_Lockers = 0 ;
    if ( GCL_GetOption( 'n' ) == NULL ) return -1 ;
    MAX_Lockers = GCL_GetNextInt() ;
	size = sizeof( LOCKER2 ) * MAX_Lockers ;
	Lockers = ( LOCKER2 * )GV_Malloc( size ) ;
    if ( Lockers == NULL ) return -1 ;
    GV_ZeroMemory( Lockers, size ) ;
    work->args.argv = work->buf ;
    return 0 ;
}

/*------------------------------------------------------------------*/

/* ロッカー管理起動 */
void		*NewLockerDaemon( name, where )
int		name, where ;
{
    Work	*work ;

//	Type = GCL_GetOptionValue( 't', 0 ) ;
    work = LockerWork = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	
    }
//	if ( Type == 0 ) {
//		PL_AddPlugin( &work->plugin, PL_MSG_LOCKER, SetLockerAct, NULL ) ;
//	} else {
		PL_AddPlugin( &work->plugin, PL_MSG_LOCKER, SetLockerAct2, NULL ) ;
//	}

	/* 死体入れ */
	PL_AddPollingFunc( &work->polling, CheckPutBody ) ;

	/* ロッカー接合を初期化 */
	N_LockerConnections = 0 ;
    return work ;
}

/*------------------------------------------------------------------*/

#if 0
/* ロッカー登録 */
static	void	*NewLockerBox( name, where )
int		name, where ;
{
    LOCKER	*this ;
    DG_DEF	*def ;
    DG_OBJS	*objs ;
    IVECTOR	iv ;
    FMATRIX	*mat ;
    FVECTOR	axis, pos ;
    int		rot, zn, model, flag, bflag ;
    float	r ;
	HZX_GROUP_ID	hzx_id ;

    ASSERT( LockerWork != NULL ) ;
    ASSERT( N_Lockers < MAX_Lockers ) ;

	flag = 0 ;
	if ( GCL_GetOption( 'f' ) != NULL ) flag = GCL_GetNextInt() ;

    this = &( Lockers[ N_Lockers ] ) ;
    this->name = name ;
    if ( GCL_GetOption( 'k' ) == NULL ) return NULL ;
	model = GCL_GetNextInt() ;

    def = ( DG_DEF * )GV_GetCache( GV_CacheID( model, 'k' ) ) ;
    ASSERT( def != NULL ) ;

	bflag = BODY_FLAG ;
	if ( flag & LOCKER_FLAG_SHADOWDROP ) bflag |= DG_FLAG_SHADOWMAKE ;
    objs = this->objs = DG_MakeObjs( def, bflag, 0 ) ;
    ASSERT( objs != NULL ) ;
    /* 回転設定 */
    objs->rots = this->rots ;

    if ( GCL_GetOption( 'r' ) == NULL ) iv = DG_ZeroIVector ;
    else {
		GCL_GetNextIV( ( int * )&iv ) ;
    }
    mat = &objs->world ;
    _sceVu0UnitMatrix( mat ) ;
    rot = iv.vz ;
    rot = ( rot & 0x0800 ) ? ( rot | 0xfffff000 ) : ( rot & 0xfff ) ;
    r = ( float )rot * ( float )M_PI / 2048.0F ;
    _sceVu0RotMatrixZ( mat, mat, r ) ;
    rot = iv.vx ;
    rot = ( rot & 0x0800 ) ? ( rot | 0xfffff000 ) : ( rot & 0xfff ) ;
    r = ( float )rot * ( float )M_PI / 2048.0F ;
    _sceVu0RotMatrixX( mat, mat, r ) ;
    rot = iv.vy ; 
    rot = ( rot & 0x0800 ) ? ( rot | 0xfffff000 ) : ( rot & 0xfff ) ;
    this->def_rot = rot ;
    r = ( float )rot * ( float )M_PI / 2048.0F ;
    _sceVu0RotMatrixY( mat, mat, r ) ;
    /* 位置設定 */
    if ( GCL_GetOption( 'p' ) == NULL ) iv = DG_ZeroIVector ;
    else {
		GCL_GetNextIV( ( int * )&iv ) ;
    }
	GV_IVtoFV( &iv, &pos, 3 ) ;
	GV_VecToMat( &pos, mat ) ;

	hzx_id = GM_GetHzxGroupID( GM_CurrentMap ) ;
	hzx_id = HZX_GetHzxIDbyZone( hzx_id, &pos, &zn ) ;

    /* プリシェード */
    DG_MakePreshade( objs, GM_GetMap( GM_CurrentMap )->light ) ;
    /* グループ化 */
    GM_GroupObjs( objs, GM_CurrentMap ) ;
    DG_QueueObjs( objs ) ;

    /* 軸位置計算 */
    axis.vx = objs->def->models[ 1 ].tx ;
    axis.vy = objs->def->models[ 1 ].ty ;
    axis.vz = objs->def->models[ 1 ].tz ;
    DG_SetPos( mat ) ;
    DG_PutVector( &axis, &( this->axis ), 1 ) ;

    /* 壁 */
    {
		FVECTOR		door, side, side2 ;
		IVECTOR		p[ 2 ] ;

		DG_COPY_VEC( &door, &axis ) ;
		door.vx = axis.vx + 1000.0F ;
		DG_PutVector( &door, &door, 1 ) ;	
		this->axis.vw = door.vw = 2000.0F ;
		GV_FVtoIV( &this->axis, &p[ 0 ], 4 ) ;
		GV_FVtoIV( &door, &p[ 1 ], 4 ) ;

		{
			/* 実験中のため、扉当たりを下げる */
			p[ 0 ].vw = p[ 1 ].vw = 1400.0F ;
		}	

		this->segment = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 0 ) ;
		ASSERT( this->segment != NULL ) ;

		DG_COPY_VEC( &side, &axis ) ;
		side.vz += 500.0F ;
		DG_PutVector( &side, &side, 1 ) ;	
		this->axis.vw = side.vw = 2000.0F ;
		GV_FVtoIV( &this->axis, &p[ 0 ], 4 ) ;
		GV_FVtoIV( &side, &p[ 1 ], 4 ) ;
		this->segment2 = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 0 ) ;
		ASSERT( this->segment2 != NULL ) ;

		DG_COPY_VEC( &side, &axis ) ;
		side.vx += 1000.0F ;
		DG_COPY_VEC( &side2, &side ) ;
		side2.vz += 500.0F ;
		DG_PutVector( &side, &side, 1 ) ;
		DG_PutVector( &side2, &side2, 1 ) ;

		side.vw = side2.vw = 2000.0F ;
		GV_FVtoIV( &side, &p[ 0 ], 4 ) ;
		GV_FVtoIV( &side2, &p[ 1 ], 4 ) ;
		this->segment3 = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 0 ) ;
		ASSERT( this->segment3 != NULL ) ;
    }
    /* 床 */
    {
		IVECTOR		p[ 4 ] ;
		FVECTOR		v[ 4 ] ;

		v[ 0 ].vx = -500.0F ; v[ 0 ].vy = 200.0F ; v[ 0 ].vz = -250.0F ;
		v[ 1 ].vx = 500.0F ; v[ 1 ].vy = 200.0F ; v[ 1 ].vz = -250.0F ;
		v[ 2 ].vx = 500.0F ; v[ 2 ].vy = 200.0F ; v[ 2 ].vz = 250.0F ;
		v[ 3 ].vx = -500.0F ; v[ 3 ].vy = 200.0F ; v[ 3 ].vz = 250.0F ;
		DG_PutVector( v, v, 4 ) ;
		GV_FVtoIV( v, p, 16 ) ;
		this->floor = HZX_AddDynamicFloor( hzx_id,
										  &p[ 0 ], &p[ 1 ], &p[ 2 ], &p[ 3 ], 4, 0 ) ;
		ASSERT( this->floor != NULL ) ;	
    }

    /* 初期状態 */
    if ( GCL_GetOption( 's' ) != NULL ) {
		this->status = GCL_GetNextInt() ;
		if ( !( this->status & 3 ) ) this->status |= LOCKER_STATE_CLOSE ;
    } else {
		this->status = LOCKER_STATE_CLOSE ;
    }

    /* 付属ブロック */
    if ( GCL_GetOption( 'e' ) != NULL ) {
		this->exec = GCL_GetNextInt() ;
    }
    if ( GCL_GetOption( 'o' ) != NULL ) {
		this->proc = GCL_GetNextInt() ;
    }    
    
    /* 音 */
    if ( GCL_GetOption( 'c' ) != NULL ) {
		this->sound[ LOCKER_SE_MOVING ] = GCL_GetNextInt() ;
		this->sound[ LOCKER_SE_CLOSE ] = GCL_GetNextInt() ;
    } else {
		this->sound[ LOCKER_SE_MOVING ] = SD_A_LOCKMOV1 ;
		this->sound[ LOCKER_SE_CLOSE ] = SD_A_LOCKCLO1 ;
    }

	/* フラグ解釈 */
	this->shadow_obj = NULL ;
	{
		extern	void *NewSpotDrawObject( int name, FMATRIX *world, int model ) ;

		if ( flag & LOCKER_FLAG_SHADOWDROP ) {
			this->shadow_obj = NewSpotDrawObject( name, &this->objs->world, model ) ;
		}
	}

    N_Lockers ++ ;
    return ( void * )this ;
}
#endif

/*------------------------------------------------------------------*/

#if 0
static 	LOCKER	*SearchLocker( name )
int		name ;
{
    int		i ;

    for ( i = 0; i < N_Lockers; i ++ ) {
		if ( Lockers[ i ].name == name ) return &Lockers[ i ] ;
    }    
    return NULL ;
}
#endif

/* ロッカーの状態を返す */
int	NewLockerStatus( void )
{
    int		name ;
//    LOCKER	*l ;
	LOCKER2	*l2 ;

	name = GCL_GetOptionValue( 'n', 0 ) ;
//	if ( Type == 0 ) {
//		l = SearchLocker( name ) ;
//		if ( l != NULL ) return l->status ;
//	} else {
		l2 = SearchLocker2( name ) ;
		if ( l2 != NULL ) return l2->status ;
//	}
    return -1 ;
}

#if 0
/* 指定ロッカーの動的床をＯＦＦ */
static	void	LockerFloorOff( name )
int		name ;
{
    LOCKER	*l ;

    l = SearchLocker( name ) ;
    if ( l != NULL ) l->floor->atr |= HZX_FLOOR_SKIP ;
}

/* 指定ロッカーの動的床をＯＮ */
static	void	LockerFloorOn( name )
int		name ;
{
    LOCKER	*l ;

    l = SearchLocker( name ) ;
    if ( l != NULL ) l->floor->atr &= ~HZX_FLOOR_SKIP ;
}
#endif

/*------------------------------------------------------------------*/

#if 0
/* プレイヤー行動 */

static	PL_MOTION_SET	*MS ;

static	void	LockerOutOpen( PlayerWork *, int ) ;
static	void	LockerOutClose( PlayerWork *, int ) ;
static	void	LockerInClose( PlayerWork *, int ) ;
static	void	LockerInside( PlayerWork *, int ) ;
static	void	LockerInOpen( PlayerWork *, int ) ;

/* 終了 */
static	void	EndLockerAct( work ) 
PlayerWork		*work ;
{
	SetFlag( FLAG_FORCE_END ) ;
	PL_Force->e_turn = -1 ;
	PL_Force->flag &= ~( FA_USE_DEFAULT | FA_NO_RECHECK_TRP ) ;
	UnsetStatus( PLAYER_LOCKER | PLAYER_FORCE ) ;
	work->control.skip_flag &= ~CTRL_SKIP_NEAR_CHECK ;
	PL_UnsetInvincible( work ) ;
	SetMode( work, PL_StillMode[ 0 ] ) ;
}

/* ロッカー外から開ける */
static	void	LockerOutOpen( work, time )
PlayerWork	*work ;
int		time ;
{
    int		ftime ;
    int		buf[ 4 ] ;
    FVECTOR	mov ;
    LOCKER	*l ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		SetAction( work, Mopen_out, 6 ) ;
		buf[ 0 ] = LOCKER_MSG_OPEN ;
		buf[ 1 ] = work->control.name ;
		buf[ 2 ] = 0 ;
		PL_SendMessage( Locker.name, buf, 3 ) ;
		if ( Locker.flag & LS_FLAG_MOVE ) {
			mov.vx = Locker.mov_x ;
			mov.vz = Locker.mov_z ;
			mov.vy = work->control.mov.vy ;
			GM_ResetControlPosition( &work->control, &mov ) ;
			Locker.flag &= ~LS_FLAG_MOVE ;
		}
		work->control.turn.vy = Locker.dir ;
		work->control.rot.vy = Locker.dir ;

		l = SearchLocker( Locker.name ) ;
		if ( l->status & LOCKER_STATE_CORPSE_IN ) {
			work->data = 1 ;
		}
    }

    switch( work->data ) {
    case 0 :
		if ( ftime >= 16 && ftime <= 66 ) {
//			printf( "time %d\n", ftime ) ;
			if ( ftime == 66 ) {
				buf[ 0 ] = LOCKER_MSG_END ;
				PL_SendMessage( Locker.name, buf, 1 ) ;
			} else {
				buf[ 0 ] = LOCKER_MSG_ROTATE ;
				buf[ 1 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 0 ] ) ;
				buf[ 2 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 2 ] ) ;
				if ( ftime < 24 ) buf[ 3 ] = -64 ;
				else if ( ftime < 28 ) buf[ 3 ] = -64 - ( 8 * ( ftime - 24 ) ) ;
				else			  buf[ 3 ] = -96 ;
				PL_SendMessage( Locker.name, buf, 4 ) ;
			}
		}
		if ( EndMotion( work ) ) { /* ftime > 66 */
			EndLockerAct( work ) ;
		}
		break ;
    case 1 :
		if ( ftime >= 16 && ftime <= 66 ) {
			if ( ftime == 66 ) {
				buf[ 0 ] = LOCKER_MSG_END ;
				PL_SendMessage( Locker.name, buf, 1 ) ;
			} else if ( ftime < 48 ) {
				buf[ 0 ] = LOCKER_MSG_ROTATE ;
				buf[ 1 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 0 ] ) ;
				buf[ 2 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 2 ] ) ;
				if ( ftime < 24 ) buf[ 3 ] = -64 ;
				else if ( ftime < 28 ) buf[ 3 ] = -64 - ( 8 * ( ftime - 24 ) ) ;
				else			  buf[ 3 ] = -96 ;
				PL_SendMessage( Locker.name, buf, 4 ) ;
			} else {
				buf[ 0 ] = LOCKER_MSG_ROTATE_D ;
				buf[ 1 ] = 20 ;
				PL_SendMessage( Locker.name, buf, 2 ) ;		
			}
		}    
		if ( ftime == 48 ) {
			SetAction( work, Msurprise, 6 ) ;	    
		}
		if ( EndMotion( work ) && 
			work->motion1 == Msurprise ) {
			EndLockerAct( work ) ;
		}
    }
}

/* ロッカー外から閉める */
static	void	LockerOutClose( work, time )
PlayerWork	*work ;
int		time ;
{
    int		ftime ;
    int		buf[ 4 ] ;
    FVECTOR	mov ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		SetAction( work, Mclose_out, 6 ) ;
		buf[ 0 ] = LOCKER_MSG_CLOSE ;
		buf[ 1 ] = work->control.name ;
		buf[ 2 ] = 0 ;
		PL_SendMessage( Locker.name, buf, 3 ) ;
		if ( Locker.flag & LS_FLAG_MOVE ) {
			mov.vx = Locker.mov_x ;
			mov.vz = Locker.mov_z ;
			mov.vy = work->control.mov.vy ;
			GM_ResetControlPosition( &work->control, &mov ) ;
			Locker.flag &= ~LS_FLAG_MOVE ;
		}
		work->control.turn.vy = Locker.dir ;
		work->control.rot.vy = Locker.dir ;
    }
//	printf( "time %d\n", ftime ) ;
    if ( ftime >= 27 && ftime <= 69 ) {
		if ( ftime == 69 ) {
			buf[ 0 ] = LOCKER_MSG_END ;
			PL_SendMessage( Locker.name, buf, 1 ) ;
		} else {
			buf[ 0 ] = LOCKER_MSG_ROTATE ;
			buf[ 1 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 0 ] ) ;
			buf[ 2 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 2 ] ) ;
			if ( ftime < 36 ) buf[ 3 ] = -48 ;
			else if ( ftime < 40 ) buf[ 3 ] = -48 - ( 23 * ( ftime - 36 ) ) ;
			else	buf[ 3 ] = -140 ;
			PL_SendMessage( Locker.name, buf, 4 ) ;
		}
    }
    if ( EndMotion( work ) ) {
		EndLockerAct( work ) ;
    }
}

/* ロッカー内から閉める */
static	void	LockerInClose( work, time )
PlayerWork	*work ;
int		time ;
{
    int		ftime ;
    int		buf[ 4 ] ;
    FVECTOR	mov ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		SetAction( work, Mclose_in, 0 ) ; /* 補完なし */
		buf[ 0 ] = LOCKER_MSG_CLOSE ;
		buf[ 1 ] = work->control.name ;
		buf[ 2 ] = 1 ;
		PL_SendMessage( Locker.name, buf, 3 ) ;
		if ( Locker.flag & LS_FLAG_MOVE ) {
			mov.vx = Locker.mov_x ;
			mov.vz = work->control.mov.vz ;
			mov.vy = work->control.mov.vy ;
			GM_ResetControlPosition( &work->control, &mov ) ;
			Locker.flag &= ~LS_FLAG_MOVE ;
		}
		PL_LeaveCaution( work ) ; /* 張り付き状態解除 */
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
    }

    PL_LevelCheck( work ) ;

    if ( ftime >= 25 && ftime <= 52 ) {
		if ( ftime == 52 ) {
			buf[ 0 ] = LOCKER_MSG_END ;
			PL_SendMessage( Locker.name, buf, 1 ) ;
		} else {
			buf[ 0 ] = LOCKER_MSG_ROTATE ;
			buf[ 1 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 0 ] ) ;
			buf[ 2 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 2 ] ) ;
			buf[ 3 ] = 130 ;
			PL_SendMessage( Locker.name, buf, 4 ) ;
		}
    }
    if ( ftime > 52 ) DG_InvisibleObjs( work->body.objs ) ;

    if ( EndMotion( work ) ) {
		//	PL_UnsetInvincible( work ) ;
		SetMode( work, LockerInside ) ;
		UnsetStatus( PLAYER_FORCE ) ;
    }
}

/* ロッカーの中 */
static	void	LockerInside( work, time )
PlayerWork	*work ;
int		time ;
{
    SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
	SetFlag( FLAG_RECOVER_ENABLE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE | PLAYER_DARK_AREA ) ;
    if ( time == 0 ) {
		SetAction( work, Min_still, 0 ) ; /* 補完なし */
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		PL_IntoSubject( work ) ;
    }

    if ( Status( PLAYER_WATCH ) ) {
		LOCKER	*l ;

		SetFlag( FLAG_CANNOT_PEEP | FLAG_SUBJECT_HORIZON_LIMIT ) ;
		/* 主観カメラ位置固定 */
		work->camera.vy = work->control.levels[ 0 ] + 1580.0F ;
		l = SearchLocker( Locker.name ) ;
		work->camera.vx = l->objs->world.m[ 3 ][ 0 ] ;
		work->camera.vz = l->objs->world.m[ 3 ][ 2 ] ;
		/* 主観ボタンで、ぐっと寄る */
		if ( work->pad->status & PAD_R1 ) {
			FVECTOR	forward = { 0.0F, 0.0F, 96.0F } ;

			DG_SetPos2( &work->camera, &work->control.rot ) ;
			DG_PutVector( &forward, &work->camera, 1 ) ;
		} else {
			SetFlag( FLAG_FINDPOS_IS_WAIST ) ;
			GM_PlayerFindPos.vy = work->control.mov.vy ;
		}
		if ( work->pad->status & ( PAD_R2 | PAD_L2 ) ) {
			GV_PAD	*pad ;
			FVECTOR	right = { 96.0F, 0.0F, 40.0F } ;
			FVECTOR	left = { -96.0F, 0.0F, 40.0F } ;
			FVECTOR	up = { 0.0F, 64.0F, 0.0F } ;
			FVECTOR	aim, diff ;
			short	turn_diff ;
			float	interp ;
			int		p ;
			/* ロッカー特殊覗き込み */
			work->camdir.vx = 0 ;
			pad = work->pad ;
			DG_SetPos2( &work->camera, &work->control.rot ) ;
			switch ( pad->status & ( PAD_R2 | PAD_L2 ) ) {
			case PAD_R2 :
				DG_PutVector( &right, &aim, 1 ) ;
				interp = ( float )pad->pressure[ PAD_PRESS_R2 ] / 255.0F ;	
				turn_diff = -400 ;
				break ;
			case PAD_L2 :
				DG_PutVector( &left, &aim, 1 ) ;
				interp = ( float )pad->pressure[ PAD_PRESS_L2 ] / 255.0F ;
				turn_diff = 400 ;
				break ;
			default :
				DG_PutVector( &up, &aim, 1 ) ;
				p = pad->pressure[ PAD_PRESS_R2 ] ;
				if ( p < pad->pressure[ PAD_PRESS_L2 ] ) p = pad->pressure[ PAD_PRESS_L2 ] ;
				interp = ( float )p / 255.0F ;
				turn_diff = 0 ;
				work->camdir.vx = ( short )( 240.0F * interp ) ;
			}
			_sceVu0SubVector( &diff, &aim, &work->camera ) ;
			_sceVu0ScaleVector( &diff, &diff, interp ) ;
			_sceVu0AddVector( &work->camera, &work->camera, &diff ) ;
			work->camdir.vy = work->control.rot.vy + ( short )( ( float )turn_diff * interp ) ;
			work->data = 1 ;
		} else {
			/* 主観回転 */
			if ( work->data == 1 ) {
				work->camdir.vx = 0 ;
				work->camdir.vy = work->control.rot.vy ;
				work->data = 0 ;
			}
			PL_SubjectTurn( work ) ;
		}
    }	
    if ( work->pad->press & PAD_LOCKER ) {
		SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
		PL_LeaveSubject( work ) ;
		PL_SetInvincible( work, 0 ) ;
		SetStatus( PLAYER_FORCE ) ;
		SetMode( work, LockerInOpen ) ;
    }
    DG_InvisibleObjs( work->body.objs ) ;
}

/* ロッカー中から開ける＆外へでる */
static	void	LockerInOpen( work, time )
PlayerWork	*work ;
int		time ;
{
    int		ftime ;
    int		buf[ 4 ] ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		SetAction( work, Mopen_in, 0 ) ; /* 補完なし */
		buf[ 0 ] = LOCKER_MSG_OPEN ;
		buf[ 1 ] = work->control.name ;
		buf[ 2 ] = 1 ;
		PL_SendMessage( Locker.name, buf, 3 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
    }	

    PL_LevelCheck( work ) ;

    switch( work->data ) {
    case 0 :
		if ( ftime < 7 ) DG_InvisibleObjs( work->body.objs ) ;
		if ( ftime >= 7 && ftime <= 37 ) {
			DG_VisibleObjs( work->body.objs ) ;
			buf[ 0 ] = LOCKER_MSG_ROTATE ;
			buf[ 1 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 0 ] ) ;
			buf[ 2 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 2 ] ) ;
			buf[ 3 ] = 208 ;
			PL_SendMessage( Locker.name, buf, 4 ) ;
		}
		if ( ftime > 37 && ftime <= 60 ) {
			if ( ftime == 60 ) {
				buf[ 0 ] = LOCKER_MSG_END ;
				PL_SendMessage( Locker.name, buf, 1 ) ;	    
			} else {
				buf[ 0 ] = LOCKER_MSG_ROTATE_D ;
				buf[ 1 ] = 20 ;
				PL_SendMessage( Locker.name, buf, 2 ) ;
			}
		}
		if ( EndMotion( work ) ) {
			work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
		}
		if ( ftime == 62 ) {
			work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
			SetAction( work, Mclose_out_turn, 0 ) ;
			work->control.turn.vy += 2048 ;
			work->control.rot.vy = work->control.turn.vy ;
			buf[ 0 ] = LOCKER_MSG_CLOSE ;
			buf[ 1 ] = work->control.name ;
			buf[ 2 ] = 0 ;
			PL_SendMessage( Locker.name, buf, 3 ) ;
			work->data ++ ;
			work->data2 = 0 ;
		}
		break ;
    case 1 :
		ftime = ++ work->data2 ;
		if ( ftime >= 66 && ftime <= 93 ) {
			if ( ftime == 93 ) {
				buf[ 0 ] = LOCKER_MSG_END ;
				PL_SendMessage( Locker.name, buf, 1 ) ;
			} else {
				buf[ 0 ] = LOCKER_MSG_ROTATE ;
				buf[ 1 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 0 ] ) ;
				buf[ 2 ] = ( int )( work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 2 ] ) ;
				buf[ 3 ] = -96 ;
				PL_SendMessage( Locker.name, buf, 4 ) ;
			}
		}	
		if ( EndMotion( work ) ) {
			EndLockerAct( work ) ;
			SetFlag( FLAG_NO_WAIST_INTERP | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
		}	
    }
}

/* ロッカーモードセット */
static	int	SetLockerAct( work, msg, len ) 
PlayerWork	*work ;
GV_MSG		*msg ;
int		len ;
{
    int			dir, range, status ;
    static FVECTOR	Shift[] = { 
		{ 0.0F, 0.0F, -1200.0F }, { 0.0F, 0.0F, -50.0F }
    } ;
    FVECTOR		mov ;
    SVECTOR		rot ;

    /* 以下の時は無効 */
    if ( Status( PLAYER_SQUAT | PLAYER_GROUND | PLAYER_DAMAGED | PLAYER_DOWNED |
				PLAYER_INTRUDE | PLAYER_FORCE | PLAYER_BEYOND | PLAYER_CB_BOX |
				PLAYER_DEAD | PLAYER_LOCKER ) || 
		( WeaponType( work ) & ( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ) ||
		( ItemType( work ) & ( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ) ) return 0 ;
    Locker.name = msg->message[ 1 ] ;
    Locker.status = status = msg->message[ 2 ] ;
    /* 状態チェック */
    if ( !( Locker.status & LOCKER_STATE_OPEN ) &&
		!( Locker.status & LOCKER_STATE_CLOSE ) ) return 0 ;
    Locker.dir = dir = msg->message[ 3 ] ;
    Locker.range = range = msg->message[ 4 ] ;
    /* 方向チェック */
    if ( Status( PLAYER_CAUTION ) ) dir += 2048 ;
    if ( GV_DiffDirAbs( work->control.rot.vy, dir ) > range ) return 0 ;

    Locker.flag = LS_FLAG_MOVE ;
    mov.vx = msg->message[ 5 ] ;
    mov.vz = msg->message[ 6 ] ;
    rot.vx = rot.vz = 0 ;
    rot.vy = dir & 4095 ;
    DG_SetPos2( &mov, &rot ) ;

    if ( Status( PLAYER_CAUTION ) ) {
		if ( status & LOCKER_STATE_CLOSE ) return 0 ;
		DG_PutVector( &Shift[ 1 ], &mov, 1 ) ;
		Locker.mov_x = mov.vx ;
		Locker.mov_z = mov.vz ;
		SetMode2( work, NULL ) ;
		SetMode( work, LockerInClose ) ;
		work->control.turn.vy = work->control.rot.vy = dir ;
    } else {
		DG_PutVector( &Shift[ 0 ], &mov, 1 ) ;
		Locker.mov_x = mov.vx ;
		Locker.mov_z = mov.vz ;
		if ( status & LOCKER_STATE_CLOSE ) {
			SetMode( work, LockerOutOpen ) ;
		} else {
			SetMode( work, LockerOutClose ) ;
		}
		work->control.turn.vy = work->control.rot.vy = dir ;
    }
    PL_SetInvincible( work, 0 ) ;
    work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK ;
    SetFlag( FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_LOCKER | PLAYER_FORCE ) ;
    MS = PL_MotionSet ;
	/* モーションファイルチェンジ */
	PL_ChangeMotionArc( work, PlayerLockerMotion ) ;
    return 1 ;
}
#endif

/*----------------------------------------------------------------*/

/* モーション登録 */
int		NewSetPlayerLockerMotion( void ) 
{
	PlayerLockerMotion = GCL_GetOptionValue( 'm', 0 ) ;
	ASSERT( PlayerLockerMotion != 0 ) ;
	return 1 ;
}

/*----------------------------------------------------------------*/

/* ロッカー登録 */
void		*NewLocker( name, where )
int			name, where ;
{
//	if ( Type == 0 ) {
//		return NewLockerBox( name, where ) ;
//	} else {
		return NewLockerDoor( name, where ) ;
//	}
}
