//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   vrpitfall
   VR用落とし穴
   fallflr.cを参考

   2002/02/22	Y.Korekado
   $Id: vrpitfall.c,v 1.1.1.3 2002/11/19 11:44:29 Yoshizawa1 Exp $
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
#include	"rand.h"

/* S.Yamashita Modified 2002.03.15  下記のインクルードを変更 */
#include "../../yamashita/pitfall_flr/pitfall_flr.h"
//#include "../../morita/brk_swg/brk_flr_def.h"
/* T.Morita Added 2001.6.1

   上のインクルードを参照して下さい。
   いろいろ崩落表示キャラについて書いてあります。
   void *NewFallingFloor( int id, FVECTOR *pos, SVECTOR *rot ) ;
 */

/*----------------------------------------------------------------*/

extern	int		DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
extern	void	DG_FreePreshade( DG_OBJS * ) ;

#define	MAX_FALL_FLOORS		(16)

/*----------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT|DG_FLAG_ONEPIECE)

#define	FALL_TIME	(300/TIME_BASE*4)

typedef	struct	{
	FVECTOR			defmov ;
	TARGET			offense ;
	POWER_TARGET	pt_offense ;
	OBJECT			*body ;
	FVECTOR			*mov ;
	SVECTOR			*rot ;
	int				phase ;
	int				count ;	
	int				data ;
	int				data2 ;
	int				k ;
	int				kdir ;
	float			level ;
	int				global_count ;
} FallWork ;

typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT			body ;
	TARGET			defense ;
	FVECTOR			def_mov ;
	FVECTOR			ctrl_mov ;
	FVECTOR			mov ;
	FVECTOR			step ;
	HZX_D_FLOOR		*floor ;
	SVECTOR			rot ;
	int				status ;
	int				count ;
	int				fall_count ;
	int				name ;
	int				proc ;
	int				fall_proc ;
	int				id ;
	int				player_rolling_ride_count ;
	int				fall_pattern ;
	FallWork		fallwork ;
} Work ;

enum {
	FF_STATE_NORMAL = 0,
	FF_STATE_RIDED,
	FF_STATE_FALLING,
} ;

enum {
	FF_MSG_FALL	 	= 0,
} ;

/*----------------------------------------------------------------*/

static	int	 		N_FallFloors = 0 ;
static	Work		*FallFloorWork[ MAX_FALL_FLOORS ] = { NULL } ;

/*----------------------------------------------------------------*/

static	void	SetPos2XYZ( FVECTOR *mov, SVECTOR *rot )
{
	float		rot_x, rot_y, rot_z ;
	int			tmp ;
	FMATRIX		world ;

	tmp = rot->vz ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_z = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vx ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_x = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vy ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_y = (float)tmp * (float)M_PI / 2048.0F ;
	_sceVu0UnitMatrix( &world );
	_sceVu0RotMatrixX( &world, &world, rot_x );
	_sceVu0RotMatrixY( &world, &world, rot_y );
	_sceVu0RotMatrixZ( &world, &world, rot_z );
	world.m[ 3 ][ 0 ] = mov->vx ;
	world.m[ 3 ][ 1 ] = mov->vy ;
	world.m[ 3 ][ 2 ] = mov->vz ;
	DG_SetPos( &world ) ;
}

static	int		UpdateK( FallWork *fw )
{
	fw->k += fw->kdir ;
	if ( fw->k >= 32 || fw->k <= -32 ) fw->kdir *= -1 ;
	return fw->k ;
}

static	void	FallWorkAct( FallWork *fw )
{
	int			phase, ymaxNo, yminNo ;
	int			i, c ;
	FVECTOR		v[ 4 ], v2[ 4 ], shift ;
	float		ymax, ymin ;

	phase = fw->phase ;
	c = 0 ;

	switch( phase ) {
	case 0 :
	case 1 :
	case 2 :
	case 3 :
		if ( phase == 0 ) {
			fw->data = GV_NearSpeedP( fw->data, 1024, 32 ) ;
			fw->rot->vx = fw->data + UpdateK( fw ) * 8 ;
		} else if ( phase == 1 ) {
			fw->data = GV_NearSpeedP( fw->data, 256, 12 ) ;
			fw->rot->vz = fw->data + UpdateK( fw ) * 8 ;
		} else if ( phase == 2 ) {
			if ( DG_FABS( fw->level - GM_WORLD_LIMIT_BOTTOM ) < 1.0F ||
				 fw->level < GM_WaterLevel ) {
				int			k ;
				/* 接地しないので自由に回転できる */
				k = UpdateK( fw ) ;
				fw->rot->vx += k * irnd() % 2 ;
				fw->rot->vy += k * irnd() % 2 ;
				fw->rot->vz += k * irnd() % 2 ;
			}
		} else if ( phase == 3 ) {
			fw->rot->vx = GV_NearSpeed( fw->rot->vx, 2048, 48 ) ;
			fw->rot->vz = GV_NearSpeedP( fw->rot->vz, 0, 24 ) ;
		}

		GV_SetVec3( &v[ 0 ], fw->body->objs->def->lx, 
				   fw->body->objs->def->uy, fw->body->objs->def->lz ) ;
		GV_SetVec3( &v[ 1 ], fw->body->objs->def->ux, 
				   fw->body->objs->def->uy, fw->body->objs->def->lz ) ;
		GV_SetVec3( &v[ 2 ], fw->body->objs->def->lx, 
				   fw->body->objs->def->uy, fw->body->objs->def->uz ) ;
		GV_SetVec3( &v[ 3 ], fw->body->objs->def->ux, 
				   fw->body->objs->def->uy, fw->body->objs->def->uz ) ;
		SetPos2XYZ( &DG_ZeroVector, fw->rot ) ;
		DG_PutVector( v, v2, 4 ) ;
		ymaxNo = 0 ; ymax = v2[ 0 ].vy ;
		yminNo = 0 ; ymin = v2[ 0 ].vy ;
		for( i = 1; i < 4; i ++ ) {
			if ( v2[ i ].vy > ymax ) {
				ymax = v2[ i ].vy ;
				ymaxNo = i ;
			} 
			if ( v2[ i ].vy < ymin ) {
				ymin = v2[ i ].vy ;
				yminNo = i ;
			}
		}
		if ( phase != 3 ) {
			_sceVu0SubVector( &shift, &v2[ ymaxNo ], &v[ ymaxNo ] ) ;
		} else {
			yminNo = fw->data2 ;
			ymin = v2[ yminNo ].vy ;
			_sceVu0SubVector( &shift, &v2[ yminNo ], &v[ yminNo ] ) ;
		}

		if ( phase == 2 ) {
			fw->defmov.vy -= 16.0F * fw->count ;
			if ( fw->defmov.vy - shift.vy + ymin <= fw->level ) {
				fw->defmov.vy = fw->level + shift.vy - ymin + 25.0F ;
				c = 1 ;
			}
		}

		_sceVu0SubVector( fw->mov, &fw->defmov, &shift ) ;

		if ( phase == 0 || phase == 1 ) {
			/* 重なり防止調整 */
			fw->mov->vy -= 25.0F ;
		}

		SetPos2XYZ( fw->mov, fw->rot ) ;
		DG_PutObjs( fw->body->objs ) ;		

		fw->count ++ ;

		if ( phase == 0 ) {
			if ( GV_DiffDirAbs( fw->data, 1024 ) < 4 && 
				++ fw->data2 > DIRECT_TICK( 32 ) ) {
				fw->phase ++ ;
				fw->data = 0 ;
				fw->data2 = 0 ;
				fw->count = 0 ;
			}
		} else if ( phase == 1 ) {
			if ( GV_DiffDirAbs( fw->data, 256 ) < 4 &&
				( ++ fw->data2 > DIRECT_TICK( 32 ) ) ) {
				fw->phase ++ ;
				fw->data = 0 ;
				fw->data2 = 0 ;
				fw->count = 0 ;
				/* 落下地点の高さを計算しておく */
				_sceVu0AddVector( &v2[ yminNo ], &v2[ yminNo ], fw->mov ) ;
				if ( ( HZX_LevelHazardCheck( HZX_AllMapID, &v2[ yminNo ], 
											 HZX_CHK_F_FLOOR, 0 ) & 1 ) ) {
					fw->level = HZX_GetFloorLevel() ;
				} else {
					fw->level = GM_WORLD_LIMIT_BOTTOM ;
				}
			}
		} else if ( phase == 2 ) {
			if ( c == 1 ) {
				fw->phase ++ ;
				fw->data = 0 ;
				fw->data2 = yminNo ;
				fw->count = 0 ;
				_sceVu0SubVector( &v[ yminNo ], &v2[ yminNo ], &v[ yminNo ] ) ;
				_sceVu0AddVector( &fw->defmov, fw->mov, &v[ yminNo ] ) ;
			}
		} else if ( phase == 3 ) {
#if 0
			if ( fw->rot->vx > 1200 ) {
				TARGET			*t ;
				POWER_TARGET	*p ;	
				FVECTOR			size = { 500.0F, 500.0F, 500.0F } ;
				FVECTOR			force = { 128.0F, 0.0F, 0.0F } ;

				/* つぶしターゲット */
				t = &fw->offense ;
				p = &fw->pt_offense ;
				GM_SetTarget( t, TARGET_OFFENSE, 0, BOTH_SIDE, &size, &DG_ZeroVector ) ;
				GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 10, 0, &force ) ;
				GM_SetTargetWeaponType( t, WP_WALLCRASH|WP_NOBLOOD ) ;
				GM_MoveTargetMap( t, fw->mov, GM_CurrentStageMap ) ;
				GM_PutTarget( t ) ;
			}
#endif
			if ( GV_DiffDirAbs( fw->rot->vx, 2048 ) < 4 ) {
				fw->phase ++ ;
				fw->data = 0 ;
				fw->data2 = 0 ;
				fw->count = 0 ;				
			}
		}
		break ;
	case 4 :
	  ;
	}
	++ fw->global_count ;
	if ( fw->global_count > DIRECT_TICK( 100 ) &&
		 fw->global_count < DIRECT_TICK( 130 ) ) {
		TARGET			*t ;
		POWER_TARGET	*p ;	
		FVECTOR			size = { 500.0F, 500.0F, 500.0F } ;
		FVECTOR			force = { 128.0F, 0.0F, 0.0F } ;
		FVECTOR			offset = { 0.0F, -1500.0F, 0.0F } ;

		/* つぶしターゲット */
		t = &fw->offense ;
		p = &fw->pt_offense ;
		GM_SetTarget( t, TARGET_OFFENSE, 0, BOTH_SIDE, &size, &offset ) ;
		GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 10, 0, &force ) ;
		GM_SetTargetWeaponType( t, WP_WALLCRASH|WP_NOBLOOD ) ;
		GM_MoveTargetMap( t, fw->mov, GM_CurrentStageMap ) ;
		GM_PutTarget( t ) ;
		//NewTargetView2( t, 32, 32, 232 ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	ExecProc( Work *work )
{
	GCL_ARGS arg ;

	if ( work->proc <= 0 ) return ;
#if 0
	if ( GM_CheckPlayerStatus( PLAYER_FORCE ) ) {
		return ;
	}
#endif
	arg.argc = 1 ;
	arg.argv = &work->name ;

	GM_ExecProc( work->proc, &arg ) ;
}

static	void	ExecProcFall( Work *work )
{
	if ( work->fall_proc <= 0 ) return ;
	GM_ExecProc( work->fall_proc, NULL ) ;
}

static	float	MinHeight( DG_OBJS *objs ) 
{
	float		min, height ;
	int			i ;

	min = 1000000.0F ;
	for ( i = 0; i < objs->n_models; i ++ ) {
		height = objs->objs[ i ].world.m[ 3 ][ 1 ] ;
		if ( height < min ) min = height ;
	}
	return min ;
}

static	int		CheckRided( Work *work )
{
	CONTROL		*ctrl ;
	HZX_HZD		*floor ;
	int			i, rided, rolling_rided ;

	/* ゲームオーバー中でも実行 */
	//if ( GM_IsGameOver() ) return 0 ;

	rided = 0 ;
	rolling_rided = 0 ;
	for ( i = 0; i < GM_N_WhereList; i ++ ) {
		ctrl = GM_WhereList[ i ] ;
		if ( !( ctrl->attribute & CTRL_ATR_PITFALLCHECK ) ) continue ;
		if ( ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) continue ;
		if ( !( ctrl->grounded & 1 ) ) continue ;
		floor = ctrl->level[ 0 ] ;
		if ( floor == NULL || 
			!( floor->attribute & HZX_FLOOR_DYNAMIC ) ||
			( u_int )floor->ptr != ( u_int )work->floor ) continue ;
		if ( i == 0 ) {
			float		height, charaheight ;
			/* プレイヤーローリング特殊 */
			/* 体の一番低い部分が床高さ＋25以下 */
			if ( GM_CheckPlayerStatus( PLAYER_ROLLING ) ) {
				height = ctrl->levels[ 0 ] ;
				charaheight = MinHeight( ctrl->object->objs ) ;
				if ( charaheight < height + 25.0F ) {
					rolling_rided = 1 ;
					if ( ++ work->player_rolling_ride_count > 8 ) {
						rided = 1 ;
						break ;
					}
				}
			} else {
				rided = 1 ;
				break ;
			}
		} else {
			rided = 2 ;
			break ;
		}
	}	

	if ( rolling_rided == 0 ) work->player_rolling_ride_count = 0 ;
	return rided ;
}

static	void	Hitted( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;

	work = ( Work * )ptr ;

	if ( off->weapon_type & WP_BLAST ) {
		/* 落とす */
		if ( work->status != FF_STATE_FALLING ) {
			/* プレイヤーが乗っているとき */
			if ( CheckRided( work ) == 1 ) ExecProc( work ) ;
#if 0
			/* Ｃ４等を落とす */
			HZX_DynamicFloorExecCallback( work->floor, HZX_CALLBACK_FLAG_PITFALL ) ;
#endif
			GV_CallChildSignalFunc( work, BRK_FLR_ModeShakeStart, work->fall_count ) ;
			ExecProcFall( work ) ;

			work->status = FF_STATE_FALLING ;
			work->count = 0 ;
			GM_TargetSetSkip( def ) ;
		}
	}
	GM_ClearTargetDamage( def ) ;
}

static	void	CheckMessage( Work *work )
{
	int			n_msg, code ;
	GV_MSG		*msg ;

	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		if ( code == FF_MSG_FALL ) {
			if ( work->status != FF_STATE_FALLING ) {
				/* プレイヤーが乗っているとき */
				if ( CheckRided( work ) == 1 ) ExecProc( work ) ;
#if 0
				/* Ｃ４等を落とす */
				HZX_DynamicFloorExecCallback( work->floor, HZX_CALLBACK_FLAG_PITFALL ) ;
#endif
				work->status = FF_STATE_FALLING ;
				work->count = 0 ;

				GV_CallChildSignalFunc( work, BRK_FLR_ModeSwingStart, 
									    work->fall_pattern ) ;/* T.Morita Added 2001.5.31 */
				ExecProcFall( work ) ;
				break ;
			}
		}
		msg ++ ;
	}
}

static	void	NormalAct( Work * work, int rided )
{
#if 0	/* 一度のったら落ちるのみ、にする */
	if ( rided == 0 ) {
		work->ctrl_mov.vy = work->def_mov.vy - 50.0F ;
		work->count = 0 ;	/* リセットしてまう */
		return ;
	}
#endif
	if ( work->status == FF_STATE_NORMAL ) {
		if ( rided == 0 ) return ;
		work->status = FF_STATE_RIDED ;
		/* ＳＥ、振動などをここにセット */		
		/* プレイヤーのとき(rided == 1)のときだけ */

		GV_CallChildSignalFunc( work, BRK_FLR_ModeShakeStart, work->fall_count ) ;/* T.Morita Added 2001.5.31 */
		ExecProcFall( work ) ;

		/* 少し下げる */
		work->ctrl_mov.vy = -50.0F ;
		work->mov.vy -= 50.0F ;
		DG_SetPos2( &work->mov, &work->rot ) ;
		DG_PutObjs( work->body.objs ) ;
		/* 敵がのったらすぐ落とす */
		if ( rided == 2 ) work->count = work->fall_count ;
	} else {
		/* 敵がのったらすぐ落とす */
		if ( rided == 2 ) work->count = work->fall_count ;
		++ work->count ;
		/* さらに下げる */
		work->ctrl_mov.vy -= 0.5F ;
		if ( work->mov.vy >= work->ctrl_mov.vy ) {
			work->mov.vy -= 0.5F ;
		}
		DG_SetPos2( &work->mov, &work->rot ) ;
		DG_PutObjs( work->body.objs ) ;
		if ( work->count >= work->fall_count ) {
			if ( rided == 1 ) ExecProc( work ) ;
#if 0
			/* Ｃ４等を落とす（下に移動） */
			HZX_DynamicFloorExecCallback( work->floor, HZX_CALLBACK_FLAG_PITFALL ) ;
#endif
			work->status = FF_STATE_FALLING ;
			work->count = 0 ;
		}
	}
}

static	void	Fall( Work *work )
{
	++ work->count ;

	GM_TargetSetSkip( &work->defense ) ;

	if ( work->count == ( 300 / TIME_BASE ) * 1 / 2 ) {
	    HZX_DynamicFloorSetAttribute( work->floor, 
					  work->floor->atr | HZX_FLOOR_PITFALL |
					  HZX_FLOOR_NO_MISSILE | HZX_FLOOR_NO_BULLET |
					  HZX_FLOOR_NO_RECOIL | 
					  HZX_FLOOR_NO_C4 | HZX_FLOOR_NO_CLAYMORE |
					  HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_NO_SPRAY ) ;
	}
	if ( work->count == ( 300 / TIME_BASE ) * 1 / 2 ) {
	    /* Ｃ４等を落とす */
		HZX_DynamicFloorExecCallback( work->floor, HZX_CALLBACK_FLAG_PITFALL ) ;
	}

	/* プレイヤーが乗ってたらプロック */
	if ( CheckRided( work ) == 1 ) ExecProc( work ) ;

	/* とりあえず4秒落ちる */
	if ( work->count < FALL_TIME ) {
#if 0
		work->mov.vy -= 96.0F ;
		work->rot.vx += 4 ;
		work->rot.vz += 4 ;
#endif
		FallWorkAct( &work->fallwork ) ;

		//DG_SetPos2( &work->mov, &work->rot ) ;
		//DG_PutObjs( work->body.objs ) ;		
	} else {
		//DG_InvisibleObjs( work->body.objs ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{	
	int			rided ;

	CheckMessage( work ) ;

	if ( PL_GetPlayerItem() == IT_Thermal ) {
		work->body.objs->flag &= ~( DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT ) ;
		work->body.objs->flag |= DG_FLAG_IRREACTION ;
	} else {
		work->body.objs->flag |= ( DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT ) ;
		work->body.objs->flag &= ~DG_FLAG_IRREACTION ;
	}

	switch( work->status ) {
	case FF_STATE_NORMAL :
	case FF_STATE_RIDED :
		rided = CheckRided( work ) ;
		NormalAct( work, rided ) ;
		break ;
	case FF_STATE_FALLING :
		Fall( work ) ;
	}
}

static	void	Die( Work *work )
{
	work->body.objs->flag |= DG_FLAG_PAINT ;
	DG_FreePreshade( work->body.objs ) ;
	GM_FreeObject( &work->body ) ;
	GM_FreeTarget( &work->defense ) ;
	HZX_RemoveDynamicFloor( work->floor ) ;

	N_FallFloors -- ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work, int name, int where )
{
	FVECTOR		mov ;
	SVECTOR		rot ;
	int			model, motion, thermal ;

	work->name = name ;

	if ( GCL_GetOption( 'A' ) != NULL ) {
		where = GM_GetMapID( GCL_GetNextInt() ) ;
	}
	PL_GetOptionFV( 'p', &mov ) ;
	PL_GetOptionSV( 'r', &rot ) ;
	work->mov = mov ;
	work->def_mov = mov ;
	work->ctrl_mov = mov ;
	work->rot = rot ;
	if ( GCL_GetOption( 'm' ) == NULL )
	{
	    printf( "-m option not found : NewFallFloor\n" ) ;
	    return -1 ;
	}
	model = GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    thermal = GCL_GetNextInt() ;
	else
	    thermal = 0 ;

	motion = GCL_GetOptionValue( 'O', 0 ) ;
	work->fall_count = DIRECT_TICK( GCL_GetOptionValue( 'f', 0 ) ) ;
	work->proc = GCL_GetOptionValue( 'R', 0 ) ;
	work->fall_proc = GCL_GetOptionValue( 'L', 0 ) ;

	/* モデル初期化 */
	GM_InitObject( &work->body, model, BODY_FLAG ) ;
	DG_SetPos2( &mov, &rot ) ;
	GM_SetCurrentMap( where ) ;
	GM_ActObject( &work->body ) ;
	DG_InvisibleObjs( work->body.objs ) ;
	DG_MakePreshade( work->body.objs, GM_GetMap( where )->light ) ;

	/* ハザード初期化 */
	{
		IVECTOR		pt[ 4 ], mm[2], *p ;
		int			n, se ;

		GCL_GetOption( 'h' ) ;
		p = mm ; n = 0 ;
		while( GCL_NextStr() != NULL ) {
			p->vx = GCL_GetNextInt() ;
			p->vy = GCL_GetNextInt() ;
			p->vz = GCL_GetNextInt() ;
			n ++ ; p ++ ;
		}
		ASSERT( n == 2 ) ;
		pt[0] = mm[0] ;
		pt[1].vx = mm[1].vx ; pt[1].vy = mm[0].vy ;pt[1].vz = mm[0].vz ;
		pt[2] = mm[1] ;
		pt[3].vx = mm[0].vx ; pt[3].vy = mm[1].vy ;pt[3].vz = mm[1].vz ;
		n = 4 ;

		se = GCL_GetOptionValue( 'N', 0 ) ;
		/* 血などはつかない */
		work->floor = HZX_AddDynamicFloor( GM_GetHzxGroupID( where ), 
										   &pt[ 0 ], &pt[ 1 ], &pt[ 2 ], &pt[ 3 ],
										   n, 
										   ( HZX_FLOOR_ATR_ALL & ~( HZX_FLOOR_NO_PLAYER |
																    HZX_FLOOR_IK |
																    HZX_FLOOR_STEP |
																    HZX_FLOOR_NO_ENEMY_EYES |
																    HZX_FLOOR_NO_ENEMY |
																    HZX_FLOOR_NO_MISSILE |
																    HZX_FLOOR_NO_BULLET |
																    HZX_FLOOR_NO_RECOIL |
																    HZX_FLOOR_NO_C4 |
																    HZX_FLOOR_NO_CLAYMORE |
																    HZX_FLOOR_NO_SPRAY ) ) | ( se << 28 ) ) ;

		/* 落下ワークの初期化 */
		{
			FallWork	*fw ;

			fw = &work->fallwork ;
			fw->body = &work->body ;
			fw->mov = &work->mov ;
			fw->rot = &work->rot ;
			fw->phase = 0 ;
			fw->count = 0 ;
			fw->data = fw->data2 = 0 ;
			DG_COPY_VEC( &fw->defmov, fw->mov ) ;
			fw->kdir = 1 ;
		}
	}

	/* 爆発受け止めターゲット */
	{
		TARGET			*t ;
		FVECTOR			size = { 450.0F, 25.0F, 450.0F } ;

		t = &work->defense ;
		GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH, 0, BOTH_SIDE, &size,
					  &DG_ZeroVector ) ;
		GM_MoveTarget3( t, &work->body.objs->world ) ;
		GM_SetTargetCallBack( t, Hitted, work ) ;
		GM_PutTarget( t ) ;
		t->center.vy += 5.0F ;
	}

	if ( GCL_GetOptionValue( 's', 0 ) > 0 ) {
		work->status = FF_STATE_FALLING ;
		work->count = FALL_TIME ;
		GM_TargetSetSkip( &work->defense ) ;
		DG_InvisibleObjs( work->body.objs ) ;

	    HZX_DynamicFloorSetAttribute( work->floor, 
					  work->floor->atr | HZX_FLOOR_PITFALL |
					  HZX_FLOOR_NO_MISSILE | HZX_FLOOR_NO_BULLET |
					  HZX_FLOOR_NO_RECOIL | 
					  HZX_FLOOR_NO_C4 | HZX_FLOOR_NO_CLAYMORE |
					  HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_NO_SPRAY ) ;
	} else {
		work->status = FF_STATE_NORMAL ;
	}

	/* 落ちパターン */
	work->fall_pattern = GCL_GetOptionValue( 'T', -1 ) ;
	if ( work->fall_pattern < 0 || work->fall_pattern > 7 ) {
		work->fall_pattern = irnd() % 8 ;
	}

	/* T.Morita Added 2001.5.31 */
	if ( work->status == FF_STATE_NORMAL ) {
		GV_SetActorChild( work, 
						/* S.Yamashita Modified 2002.03.15  下記の呼び出す関数(キャラ)を変更 */
						/* NewFallingFloor( model, thermal, &work->mov, &work->rot ) ) ; */
						NewPitfallFloor(&work->mov, &work->rot ) ) ;
		GV_CallChildSignalFunc( work, BRK_FLR_SetSeMode, GCL_GetOptionValue( 'P', 0 ) ) ;/* T.Morita Added 2001.8.1 */
		GV_CallChildSignalFunc( work, BRK_FLR_SetFallType, work->fall_pattern ) ;/* T.Morita Added 2001.8.1 */
	}

	work->id = N_FallFloors ;
	FallFloorWork[ work->id ] = work ;

	N_FallFloors ++ ;

	return 0 ;
}

void	*NewVrPitFall( int name, int where )
{
	Work			*work ;

	ASSERT( N_FallFloors < MAX_FALL_FLOORS ) ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, name, where ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

/*----------------------------------------------------------------*/

int		Command_GetVrPitFallStatus( void )
{
	int				name, i ;

	name = GCL_GetOptionValue( 'n', 0 ) ;
	for ( i = 0; i < N_FallFloors; i ++ ) {
		if ( FallFloorWork[ i ]->name == name ) {
			return FallFloorWork[ i ]->status ;
		}
	}
	return -1 ;
}
