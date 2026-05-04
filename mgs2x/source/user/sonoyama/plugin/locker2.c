/*
   locker2.c
   扉のみロッカー

   2000/06/06 M.Sonoyama
   $Id: locker2.c,v 1.1.1.3 2002/11/19 11:50:49 Yoshizawa1 Exp $
*/

#include "BP_TrophySystem.h"
#include "BP_Debug.h"   //BP_INPUT

#if defined(BP_VITA)
#include "gesture_vta.h"
#endif

#define	BODY_FLAG2	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT | \
					 DG_FLAG_FINISHCALC )

//static	int	SetLockerAct2( PlayerWork *, GV_MSG *, int ) ;

/*------------------------------------------------------------------*/

static	int			LockerMotion = 0 ;

//static 	int			MsgBuf[ 6 ] = { 0 } ;

/*------------------------------------------------------------------*/

extern	void 	*NewMakeObjectBoundHazard2( HZX_GROUP_ID hzx_id, DG_OBJS *objs, 
										   int seg_atr, int flr_atr, int disp ) ;
extern	void	PL_UpdateObjectBoundHazard( void *work, FMATRIX *world ) ;
extern	void	PL_ObjectBoundHazardSetAttribute( void *ptr, int seg_atr, int flr_atr ) ;
extern	void	PL_ObjectBoundHazardSetTarget( void *ptr, TARGET *target ) ;

/*----------------------------------------------------------------*/

/* 扉だけバージョン */

/*----------------------------------------------------------------*/

enum {
	DMopen_out = 0,
	DMclose_out,
	DMopen_in,
	DMclose_in,	
   	DMclose_out_turn,
	DMenemy_reserved,
	DMnot_open,
	DMbreak,
	DMput_body,
	DMput_leg_f,
	DMput_leg_b,
	DMbreak_long,
	DMbreak_inside,
	DMopen_out_behind,
} ;

#define	SLEEP_MOT( _m )		((_m)->mt3_ctrl[ 0 ].flag |= MT3_SLEEP)
#define	ACTIVE_MOT( _m )		((_m)->mt3_ctrl[ 0 ].flag &= ~MT3_SLEEP)
#define	SETARG( _n, _v )	(LockerWork->buf[ _n ] = _v)
#define	SETVEC( _v, _x, _y, _z )	{(_v)->vx = _x ; (_v)->vy = _y ; (_v)->vz = _z;}

static	LOCKER2		*This ;

static FVECTOR	Shift[] = { 
	{ -500.0F, 0.0F, 950.0F }, { -500.0F, 0.0F, -300.0F },
	{ -450.1F, 0.0F, 303.0F }
} ;

static	inline int	SameRot( SVECTOR *r1, SVECTOR *r2 )
{
	if ( ( r1->vx == r2->vx ) && ( r1->vy == r2->vy ) && ( r1->vz == r2->vz ) ) return 1 ;
	return 0 ;
}

/*----------------------------------------------------------------*/

static	inline	void	SetMsgBuf( this, a, b, c, d, e )
LOCKER2			*this ;
int				a, b, c, d, e ;
{
	int			*MsgBuf ;

	MsgBuf = this->msg_buf ;
	MsgBuf[ 0 ] = a ;
	MsgBuf[ 1 ] = b ;
	MsgBuf[ 2 ] = c ;
	MsgBuf[ 3 ] = d ;
	MsgBuf[ 4 ] = e ;
	MsgBuf[ 5 ] = this->name ;
//	PL_SendMessage( This->name, MsgBuf, 5 ) ;
}

static	void	ExecProc2EX( this, n, mode )
LOCKER2			*this ;
int				n ;
int				mode ;
{
	Work		*work ;

	work = LockerWork ;
	work->args.argc = 4 ;
	work->args.argv[ 3 ] = this->name ;
	if ( mode == 0 ) {
		if ( this->proc != 0 ) {
			GM_ExecProc( this->proc, &work->args ) ;
		} else if ( this->exec != 0 ) {
			GM_ExecBlock( ( char * )this->exec, &work->args ) ;
		}
	} else {
		if ( this->proc != 0 ) {
			GM_ForceExecProc( this->proc, &work->args ) ;
		} else if ( this->exec != 0 ) {
			GM_ForceExecBlock( ( char * )this->exec, &work->args ) ;
		}
	}
}

#define	ExecProc2( _t, _n )		ExecProc2EX( _t, _n, 0 )

/*----------------------------------------------------------------*/

/* ドアを傾ける */
static	void	LeanDoor( this, rot )
LOCKER2			*this ;
SVECTOR			*rot ;
{
	FMATRIX		m1, m2 ;
	int			i ;
	OBJECT		*body ;

	body = &this->body ;

	GV_NearExp2PV( &this->lean_rot, rot, 3 ) ;
//	GV_NearSpeedPV( &this->lean_rot, rot, 3, 48 ) ;
	DG_SetPos2( &DG_ZeroVector, &this->lean_rot ) ;
	DG_GetPos( &m1 ) ;
	DG_COPY_MAT( &m2, &this->def_world ) ;
	for ( i = 0; i < body->objs->def->n_models; i ++ ) {
		_sceVu0MulMatrix( &body->objs->objs[ i ].world, &m2, &m1 ) ;
	}
	PL_UpdateObjectBoundHazard( this->objhzx, &body->objs->objs[ 0 ].world ) ;
}

/* ロッカーゾーン内に敵がいないかチェック */
static	int	CheckEnemyInZone( this )
LOCKER2		*this ;
{
	if ( GM_ZoneIntrpt( this->zones[ 0 ], 0, 
					    ZONE_INTRPT_ENEMY | ZONE_INTRPT_OBJ | ZONE_INTRPT_CORP ) != NULL ) {
		//printf( "locker : something is in the zone\n" ) ;
		return 1 ;
	}
	if ( GM_ZoneIntrpt( this->zones[ 1 ], 0, 
					    ZONE_INTRPT_ENEMY | ZONE_INTRPT_OBJ | ZONE_INTRPT_CORP ) != NULL ) {
		//printf( "locker : something is in the zone\n" ) ;
		return 1 ;	
	}
	return 0 ;
}

static	int	CheckEnemyInZoneExceptThis( LOCKER2 *this, CONTROL *ctrl )
{
	if ( GM_SearchZIntrptExceptThis( this->zones[ 0 ], 0,
									 ZONE_INTRPT_ENEMY | ZONE_INTRPT_OBJ | ZONE_INTRPT_CORP,
									ctrl ) != NULL ) {
//		printf( "locker : something is in the zone\n" ) ;
		return 1 ;
	}
	if ( GM_SearchZIntrptExceptThis( this->zones[ 1 ], 0,
									 ZONE_INTRPT_ENEMY | ZONE_INTRPT_OBJ | ZONE_INTRPT_CORP,
									ctrl ) != NULL ) {
//		printf( "locker : something is in the zone\n" ) ;
		return 1 ;
	}
	return 0 ;
}

/* ロッカー内にグレネードなどがあるかチェック */
static	int		CheckBomb( LOCKER2 *this )
{
#if 0
	GM_BOMB		*bthis, *list ;
	FVECTOR		pos = { 0.0F, 0.0F, 200.0F } ;
	SVECTOR		rot ;
	
	GV_SetVec3( &rot, 0, this->def_rot, 0 ) ;
	DG_SetPos2( &this->center, &rot ) ;
	DG_PutVector( &pos, &pos, 1 ) ;
	
	list = GM_BombList.next ;
	while( list != NULL ) {
		bthis = list ;
		list = list->next ;
		if ( bthis->mov != NULL && 
			 bthis->weapon != WP_Magazine &&
			 GV_VecLen3F2( bthis->mov, &pos ) < 550.0F ) {
			return 1 ;
		}	
	}
	return 0 ;
#else
	return 0 ;
#endif
}

/* 自分に入っている敵を探す */
static	void	SearchEnemy( LOCKER2 *this )
{
	FVECTOR		chkpos ;
	HOMING_TRG	*hom ;
	
	DG_COPY_VEC( &chkpos, &this->center ) ;

	if ( this->flag & LOCKER_FLAG_TOILET ) {
		FVECTOR		shift = { 0.0F, 0.0F, -750.0 } ;
		SVECTOR		rot = { 0.0F, this->def_rot, 0.0F } ;

		DG_SetPos2( &chkpos, &rot ) ;
		DG_PutVector( &shift, &chkpos, 1 ) ;
	}

	hom = GM_GetHoming() ;
	while( hom != NULL ) {
		if ( ( hom->status & ( HOMING_ENEMY | HOMING_DEAD ) ) &&
			 !( hom->status & HOMING_UNREAL ) && 
			 ( hom->trg->capture->flag & CAPTURE_LOCKER ) ) {
			if ( DG_FABS( hom->world->m[ 3 ][ 1 ] - chkpos.vy ) < 2000.0F ) {
				chkpos.vy = hom->world->m[ 3 ][ 1 ] ;
				printf( "Len %f\n", GV_VecLen3F2( &chkpos, ( FVECTOR * )hom->world->m[ 3 ] ) ) ;
				if ( GV_VecLen3F2( &chkpos, ( FVECTOR * )hom->world->m[ 3 ] ) < 200.0F ) {
					this->capture = hom->trg->capture ;
					printf( "locker in enemy found!!!\n" ) ;
					this->status |= LOCKER_STATE_CORPSE_IN ;
					break ;
				}
			}
		}
		hom = hom->next ;		
	}
	if ( this->capture == NULL ) {
		printf( "search enemy not found\n" ) ;
		this->status &= ~LOCKER_STATE_CORPSE_IN ;
	}
}

/* 吹き飛ばしターゲット */
static	void	SetBlowTarget( LOCKER2 *this, int side )
{
	TARGET			*t ;
	POWER_TARGET	*p ;
	FVECTOR			size = { 300.0F, 1000.0F, 250.0F } ;
	FVECTOR			force = { 0.0F, 0.0F, 48.0F } ;
	FVECTOR			shift = { 0.0F, 1100.0F, 500.0F } ;
	SVECTOR			rot ;
	FVECTOR			v ;
	int				dir, diff ;

	if ( this->puttargettime == GV_Time ) {
		printf( "locker offense target has already put!\n" ) ;
		return ;
	}

	if ( !( this->status & LOCKER_STATE_BROKEN ) ) {
		GV_SetVec3( &v, -10.0F, 0.0F, 0.0F ) ;
		DG_SetPos( &this->body.objs->objs[ 0 ].world ) ;
		DG_RotVector( &v, &v, 1 ) ;
		dir = GV_VecDir2( &v ) ;
		diff = GV_DiffDirAbs( dir, this->def_rot ) ;
		if ( diff < 32 || diff > 768 ) return ;
	} else {
		size.vz = 1000.0F ;
		shift.vz = 750.0F ;
	}

	t = &this->attack ;
	p = &this->power ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ROTATE, 0, side, &size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_WALLCRASH | WP_NOBLOOD ) ;
	rot.vx = rot.vz = 0 ; rot.vy = this->def_rot ;
	DG_SetPos2( &this->center, &rot ) ;
	DG_RotVector( &force, &force, 1 ) ;
	DG_PutVector( &shift, &shift, 1 ) ;
	GM_SetPowerTarget( t, p, POWER_CONST, 255, 10, 0, &force ) ;
	GM_MoveTargetMap( t, &shift, GM_CurrentMap | GM_CurrentStageMap ) ;
	GM_PutTarget( t ) ;
	this->puttargettime = GV_Time ;

	//GM_SetCurrentMap( GM_PlayerMap ) ;
	//NewTargetView2( t, 32, 32, 232 ) ;
}

/* 壁ステータス */
static	void	SetDynamicSegAtr( LOCKER2 *this, int atr )
{
	this->segment->atr |= atr ;
	//this->segment2->atr |= atr ;
	//this->segment3->atr |= atr ;
	*( int * )GV_UNCACHE( &this->segment->atr ) = this->segment->atr ;
	// *( int * )GV_UNCACHE( &this->segment2->atr ) = this->segment2->atr ;
	// *( int * )GV_UNCACHE( &this->segment3->atr ) = this->segment3->atr ;
}

static	void	ResetDynamicSegAtr( LOCKER2 *this, int atr )
{
	this->segment->atr &= ~atr ;
	//this->segment2->atr &= ~( atr & ~( HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY ) ) ;
	//this->segment3->atr &= ~( atr & ~( HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY ) ) ;
	*( int * )GV_UNCACHE( &this->segment->atr ) = this->segment->atr ;
	// *( int * )GV_UNCACHE( &this->segment2->atr ) = this->segment2->atr ;
	// *( int * )GV_UNCACHE( &this->segment3->atr ) = this->segment3->atr ;
}

static	inline	void	SEGMENT4_ON( LOCKER2 *this )
{
	this->segment4->atr &= ~HZX_SEG_SKIP ;
	*( int * )GV_UNCACHE( &this->segment4->atr ) = this->segment4->atr ;
}

static	inline	void	SEGMENT4_OFF( LOCKER2 *this )
{
	if ( this->status & LOCKER_STATE_DESTROY ) {
		this->segment4->atr |= HZX_SEG_SKIP ;
	} else {
		/* NO_PLAYER はよりかかり閉じ用 */
		this->segment4->atr |= HZX_SEG_SKIP | HZX_SEG_NO_PLAYER ;
	}
	*( int * )GV_UNCACHE( &this->segment4->atr ) = this->segment4->atr ;
}

/* segment5は扉が開いているとき出現 */
static	inline	void	SEGMENT5_ON( LOCKER2 *this )
{
	this->segment5->atr &= ~HZX_SEG_SKIP ;
	*( int * )GV_UNCACHE( &this->segment5->atr ) = this->segment5->atr ;
}

static	inline	void	SEGMENT5_OFF( LOCKER2 *this )
{
	this->segment5->atr |= HZX_SEG_SKIP ;
	*( int * )GV_UNCACHE( &this->segment5->atr ) = this->segment5->atr ;
}

/* 壁無効 */
static	inline	void	SetDynamicSegSkip( LOCKER2 *this )
{
	SetDynamicSegAtr( this, HZX_SEG_SKIP ) ;
}

/* 壁有効 */
static	inline	void	ResetDynamicSegSkip( LOCKER2 *this )
{
	ResetDynamicSegAtr( this, HZX_SEG_SKIP ) ;
}

/* 壊れ後の床生成 */
static	void	MakeBrokenDoorHazard( LOCKER2 *this )
{
	OBJECT			*body ;
	FVECTOR			p[ 4 ], max, min ;
	HZX_GROUP_ID	hzx_id ;
	int				map ;

	if ( this->floor != NULL ) return ;

	body = &this->body ;
	max.vx = body->objs->def->models[ 0 ].ux ;
	max.vy = body->objs->def->models[ 0 ].uy ;
	max.vz = body->objs->def->models[ 0 ].uz ;
	min.vx = body->objs->def->models[ 0 ].lx ;
	min.vy = body->objs->def->models[ 0 ].ly ;
	min.vz = body->objs->def->models[ 0 ].lz ;	

	if ( !( this->status & LOCKER_STATE_DESTROY ) ) {
		GV_SetVec3( &p[ 0 ], min.vx, min.vy, min.vz ) ;
		GV_SetVec3( &p[ 1 ], min.vx, max.vy, min.vz ) ;
		GV_SetVec3( &p[ 2 ], max.vx, max.vy, min.vz ) ;
		GV_SetVec3( &p[ 3 ], max.vx, min.vy, min.vz ) ;
	} else {
		GV_SetVec3( &p[ 0 ], min.vx, min.vy, max.vz ) ;
		GV_SetVec3( &p[ 1 ], min.vx, max.vy, max.vz ) ;
		GV_SetVec3( &p[ 2 ], max.vx, max.vy, max.vz ) ;
		GV_SetVec3( &p[ 3 ], max.vx, min.vy, max.vz ) ;
	}
	
	DG_SetPos( &body->objs->objs[ 0 ].world ) ;
	DG_PutVector( p, p, 4 ) ;

	map = GM_GetMapIDfromPos( body->map_name, ( FVECTOR * )body->objs->world.m[ 3 ] ) ;
	hzx_id = GM_GetHzxGroupID( map ) ;

	if ( !( this->status & LOCKER_STATE_DESTROY ) ) {
		p[ 0 ].vy += 150.0F ;
		p[ 1 ].vy += 150.0F ;
		if ( HZX_LevelHazardCheck( hzx_id, &p[ 0 ], HZX_CHK_ALL, 0 ) & 1 ) {
			float		levels[ 2 ] ;

			HZX_GetLevelHeight( levels ) ;
			p[ 0 ].vy = ( float)( ( int )levels[ 0 ] + 1 ) ;
			p[ 1 ].vy = p[ 2 ].vy = p[ 3 ].vy = p[ 0 ].vy ;
		} else if ( HZX_LevelHazardCheck( hzx_id, &p[ 1 ], HZX_CHK_ALL, 0 ) & 1 ) {
			float		levels[ 2 ] ;

			HZX_GetLevelHeight( levels ) ;
			p[ 0 ].vy = ( float)( ( int )levels[ 0 ] + 1 ) ;
			p[ 1 ].vy = p[ 2 ].vy = p[ 3 ].vy = p[ 0 ].vy ;
		} else {
			printf( "???????????????????????????????????????\n" ) ;
			p[ 0 ].vy -= 150.0F ;
			p[ 1 ].vy = p[ 2 ].vy = p[ 3 ].vy = p[ 0 ].vy ;			
		}
		this->floor = HZX_AddDynamicFloorF( hzx_id, p, 4, 	
										    HZX_FLOOR_NO_BULLETHOLE | HZX_SeCode( this->sefloor ) ) ;
	} else {
		this->floor = HZX_AddDynamicFloorF( hzx_id, p, 4, 	
										    HZX_FLOOR_NO_BULLETHOLE | HZX_FLOOR_NO_PLAYER |
										    HZX_FLOOR_NO_ENEMY |
										    HZX_SeCode( this->sefloor ) ) ;
	}
}

/* 壊れフェイズ */
static	void	BrokenPhase( LOCKER2 *this )
{
	int			phase, time, brk_start ;
	OBJECT		*body ;
	FVECTOR		pos, shift ;

	phase = this->brk_phase ;
	body = &this->body ;

	brk_start = this->status & LOCKER_STATE_BROKEN_FAST ;

	switch( phase ) {
	case 0 :
		ACTIVE_MOT( body->m_ctrl ) ;
//		GM_ConfigObjectAction( body, 0, DMbreak, 0, 0xfffff, 0 ) ;
		/* 開かないが外側に倒れる奴 */
		if ( ( this->status & LOCKER_STATE_DESTROY ) &&
			 ( this->flag & LOCKER_FLAG_DOWNOUTSIDE ) ) {
			this->status &= ~LOCKER_STATE_DESTROY ;
		}

		if ( brk_start ) body->m_ctrl->sar_ctrl = NULL ;	/* 音を鳴らさない為 */

		if ( this->status & LOCKER_STATE_DESTROY ) {
			GM_ConfigObjectAction( body, 0, DMbreak_inside, 0, 0xfffff, 0 ) ;
			/* 開かないロッカーの場合は、ＳＥＧ４は消さない */
			SEGMENT4_ON( this ) ;
		} else {
			GM_ConfigObjectAction( body, 0, DMbreak_long, 0, 0xfffff, 0 ) ;
			SEGMENT4_OFF( this ) ;
		}

		if ( brk_start ) {
			float	speed ;
			/* モーション再生速度調整 */
			speed = body->m_ctrl->mt3_ctrl[ 0 ].motion_total_time - ( float )TIME_BASE * 2 ;
			//printf( "broken locker set motion speed %f\n", speed ) ;
			MT_SetMotionSpeed( body->m_ctrl, speed ) ;
			/* 倒れシフト位置 */
			DG_COPY_VEC( &this->down_shift_cur, &this->down_shift ) ;		   
		}

		SEGMENT5_OFF( this ) ;
		this->lean_rot.vx = this->lean_rot.vz = 0 ;
		this->lean_rot.vy = this->def_rot ;
		if ( this->status & LOCKER_STATE_OPEN ) this->lean_rot.vy += 1024 ;

		/* ルート遮断開放 */
		if ( this->status & LOCKER_STATE_DESTROY ) {
			this->r_intrpt.status = ROOT_INTRPT_CLOSE | ROOT_INTRPT_HINGED | ROOT_INTRPT_BREAK ;
		} else {
			this->r_intrpt.status = ROOT_INTRPT_OPEN | ROOT_INTRPT_HINGED | ROOT_INTRPT_BREAK ;
		}

		/* 壁無効 */
		SetDynamicSegSkip( this ) ;
		HZX_DynamicSegmentSetSkip( this->segment6 ) ;

		/* ターゲット無効 */
		GM_FreeTarget( &this->target ) ;
		/* 敵放出 */
		if ( this->status & LOCKER_STATE_CORPSE_IN ) {
			if ( this->capture != NULL ) this->capture->flag &= ~CAPTURE_LOCKER ;
		}
		this->status &= ~LOCKER_STATE_CORPSE_IN ;

		/* プロック呼び出し */
		if ( this->status & LOCKER_STATE_CLOSE ) {
			SETARG( 0, 0 ) ;
			SETARG( 1, 0 ) ;
			SETARG( 2, 4 ) ;
			ExecProc2( this, 3 ) ;
		}
		/* 壊れた */
		SETARG( 0, 0 ) ;
		SETARG( 1, 0 ) ;
		SETARG( 2, 7 ) ;
		ExecProc2( this, 3 ) ;

		this->brk_phase = 1 ;
		break ;
	case 1 :
		/* モーション処理 */

		/* モーション移動量は、閉じた状態のものを使う */
	    {
			SVECTOR		tmpRot = { 0, this->def_rot, 0 } ;

			DG_SetPos2( ( FVECTOR * )body->objs->world.m[ 3 ], &tmpRot ) ;
			DG_GetPos( &body->objs->world ) ;
		}

		GM_ActMotion( body ) ;
		time = ( int )( body->m_ctrl->mt3_ctrl->play_time / ( float )TIME_BASE ) ;

		GV_MatToVec( &body->objs->world, &pos ) ;
		pos.vx += body->step->vx ;
		pos.vz += body->step->vz ;
		pos.vy = this->def_world.m[ 3 ][ 1 ] ;
		if ( brk_start ) {
			this->lean_rot = this->down_rot ;
			pos.vx += this->down_shift_cur.vx ;
			pos.vz += this->down_shift_cur.vz ;
			DG_COPY_VEC( &this->down_shift_cur, &DG_ZeroVector ) ;
		} else if ( ( this->status & LOCKER_STATE_OPEN ) || time > 100 ) {
			DG_COPY_VEC( &shift, &this->down_shift_cur ) ;
			GV_NearExp16VF( &this->down_shift_cur, &this->down_shift, 3 ) ;
			pos.vx += this->down_shift_cur.vx - shift.vx ;
			pos.vz += this->down_shift_cur.vz - shift.vz ;
			GV_NearSpeedPV( &this->lean_rot, &this->down_rot, 3, 12 ) ;
		}

		/* トイレ時音きり変え用 */
		{
			int		seNoSeg = -1 ;
			if ( this->flag & LOCKER_FLAG_TOILET ) seNoSeg = 1 ;
			MT_SetMotionSeTable( body->m_ctrl, body->map_name, 0, -1, seNoSeg ) ;
		}

		DG_SetPos2( &pos, &this->lean_rot ) ;
		GM_ActObject2( body ) ;
		GV_DestroyOtherActor( this->objhzx ) ;
		this->objhzx = NULL ;

//		if ( MT_CHECK_MOTION_TIME( body->m_ctrl, 0, TIME_BASE * 60 ) ) {
//			SetBlowTarget( this, BOTH_SIDE ) ;
//		}

		if ( !brk_start && !( this->status & LOCKER_STATE_DESTROY ) ) {
			if ( body->m_ctrl->mt3_ctrl->play_time > TIME_BASE * 90 &&
				body->m_ctrl->mt3_ctrl->play_time < TIME_BASE * 130 ) {
				SetBlowTarget( this, BOTH_SIDE ) ;
			}
//printf( "%f\n", body->m_ctrl->mt3_ctrl->play_time / ( float )TIME_BASE ) ;
		}

		if ( MT_CHECK_END( body->m_ctrl, 0 ) ) {
			if ( !( this->status & LOCKER_STATE_DESTROY ) ) {
				/* 開かないロッカーの場合は、ＳＥＧ４は消さない */
				SEGMENT4_OFF( this ) ;
			}
			SLEEP_MOT( body->m_ctrl ) ;		
			this->brk_phase = 2 ;			
		}
		break ;
	case 2 :
		GV_MatToVec( &body->objs->world, &pos ) ;
		pos.vy = this->def_world.m[ 3 ][ 1 ] ;
		DG_COPY_VEC( &shift, &this->down_shift_cur ) ;
		GV_NearExp16VF( &this->down_shift_cur, &this->down_shift, 3 ) ;
		pos.vx += this->down_shift_cur.vx - shift.vx ;
		pos.vz += this->down_shift_cur.vz - shift.vz ;
		GV_NearSpeedPV( &this->lean_rot, &this->down_rot, 3, 12 ) ;
		DG_SetPos2( &pos, &this->lean_rot ) ;
		GM_ActObject2( body ) ;
		/* 床を生成 */
		MakeBrokenDoorHazard( this ) ;
	}	
}

/*----------------------------------------------------------------*/

static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	LOCKER2		*this ;
	FVECTOR		defc ;
	long64		wp ;
	int			level ;

	if ( !( def->damaged & TARGET_POWER ) ) {
		def->weapon_type = 0 ;
		return ;
	}
	this = ( LOCKER2 * )ptr ;
	if ( this->status & ( LOCKER_STATE_BROKEN | LOCKER_STATE_INVINCIBLE ) ) {
		def->weapon_type = 0 ;
		return ;
	}
	/* 開いてるか、しまっているかのときしかあたらない */
	if ( !( this->status & ( LOCKER_STATE_CLOSE | LOCKER_STATE_OPEN ) ) ) {
		def->weapon_type = 0 ;
		return ;
	}
	_sceVu0ApplyMatrix( &defc, &def->world, &def->offset ) ;
	_sceVu0AddVector( &defc, &def->center, &defc ) ;

	wp = off->weapon_type ;
	if ( wp & WP_PLAYER ) GM_VctrlResetSkip( &GM_PlayerWork->vctrl ) ;

	if ( ( def != &this->target ) && ( wp & WP_BULLET ) ) {
		this->hit_count ++ ;
	} else if ( ( def == &this->target ) && ( wp & ( WP_BLAST | WP_KICK | 
													 WP_PUNCHALL | WP_SHOTGUN_NEAR ) ) ) {
		/* 壁チェック */
		if ( HZX_OnlineHazardCheck( HZX_AllMapID, &off->center, &defc,
								    HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR | HZX_CHK_RECOIL_TYPE,
								    HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) ) {
			def->weapon_type = 0 ;
			return ;		
		}

		if ( GV_VecLen3F2( &off->center, &defc ) < 1600.0F ) {
			if ( wp & WP_PUNCHALL ) this->hit_count += 1 ;
			else if ( wp & WP_KICK ) this->hit_count += 5 ;
			else				this->hit_count += 20 ;

			if ( wp & ( WP_KICK | WP_PUNCHALL ) ) {
				if ( this->flag & LOCKER_FLAG_TOILET ) {
					GM_SeSetMode( SD_A_TOILATK1, &this->center, GM_SEMODE_BOMB ) ;
				} else {
					GM_SeSetMode( SD_A_LOCKATK1, &this->center, GM_SEMODE_BOMB ) ;
				}
				GM_SetNoise( NOISE_S, &this->center, GM_PlayerMap ) ;
				if ( wp & WP_KICK ) {
					if ( PL_GetPlayerWeapon() == WP_Nikita ) {
						NewPadVibration2( GV_StrCode( "rai_punch_03" ), 0 ) ;
						this->hit_count += 5 ;
					} else if ( wp & WP_KICK1 ) {
						NewPadVibration2( GV_StrCode( "rai_kick_01" ), 0 ) ;
						this->hit_count -= 2 ;
					} else {
						NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ;
					}
				} else {
					if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ) {
						NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ;
						this->hit_count += 1 ;
					} else {
						NewPadVibration2( GV_StrCode( "rai_punch_01" ), 0 ) ;
					}
				}
			}
		}
	} 

	level = this->lean_level ;

	if ( this->hit_count >= 15 ) this->lean_level = 3 ;
	else if ( this->hit_count >= 10 ) this->lean_level = 2 ;
	else if ( this->hit_count >= 5 ) this->lean_level = 1 ;
	if ( this->lean_level != level ) {
		SETARG( 0, 0 ) ;
		SETARG( 1, 0 ) ;
		SETARG( 2, 9 ) ;
		ExecProc2( this, 3 ) ;
		this->lean = 0 ;
	}

	if ( ( this->lean_level >= 3 ) && 
		 ( this->status & ( LOCKER_STATE_OPEN | LOCKER_STATE_CLOSE ) ) ) {
		this->status |= LOCKER_STATE_BROKEN ;
	}
	GM_ClearTargetDamage( def ) ;
}

static	inline	int		AdjustValue( int v )
{
	v += ( v < 0 ) ? -2 : 2 ;
	return v / 5 * 5 ;
}

static	void	AdjustSegment( HZX_D_SEGMENT *seg )
{
	IVECTOR		p[ 2 ] ;

	p[ 0 ].vx = AdjustValue( seg->p1.vx ) ;
	p[ 0 ].vy = AdjustValue( seg->p1.vy ) ;
	p[ 0 ].vz = AdjustValue( seg->p1.vz ) ;
	p[ 0 ].vw = AdjustValue( seg->p1.vw ) ;
	p[ 1 ].vx = AdjustValue( seg->p2.vx ) ;
	p[ 1 ].vy = AdjustValue( seg->p2.vy ) ;
	p[ 1 ].vz = AdjustValue( seg->p2.vz ) ;
	p[ 1 ].vw = AdjustValue( seg->p2.vw ) ;
	HZX_MoveDynamicSegment( seg, &p[ 0 ], &p[ 1 ] ) ;
}

static	void	RotateSegment2( this )
LOCKER2			*this ;
{
	IVECTOR		pole ;
	FMATRIX		*world ;

	world = &this->body.objs->objs[ 0 ].world ;
	pole.vx = world->m[ 3 ][ 0 ] ;
	pole.vy = world->m[ 3 ][ 1 ] ;
	pole.vz = world->m[ 3 ][ 2 ] ;
	HZX_RotateDynamicSegment2( this->segment, &pole, world ) ;
	//HZX_RotateDynamicSegment2( this->segment2, &pole, world ) ;
	//HZX_RotateDynamicSegment2( this->segment3, &pole, world ) ;
	PL_UpdateObjectBoundHazard( this->objhzx, world ) ;
	if ( this->gravure != NULL ) {
		HZX_RotateDynamicSegment2( this->gravure->special1, &pole, world ) ;
		HZX_RotateDynamicSegment2( this->gravure->special2, &pole, world ) ;
	}
	AdjustSegment( this->segment ) ;
	//AdjustSegment( this->segment2 ) ;
	//AdjustSegment( this->segment3 ) ;
}

static 	int		CloseLockerbyLeaning2( this ) 
LOCKER2			*this ;
{
    HZX_SEG		*seg ;
    HZX_D_SEGMENT	*dseg ;
    FVECTOR		from, to, pos ;

	if ( !( this->status & LOCKER_STATE_OPEN ) ) return 0 ;
	if ( GM_PlayerWork != NULL && 
		( GM_CheckPlayerStatus( PLAYER_CAUTION ) ) &&
		GM_PlayerWork->control.n_touches != 0 &&
		( GM_PlayerWork->control.atrs[ 0 ] & HZX_SEG_DYNAMIC ) &&
		!CheckEnemyInZone( this ) ) {
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
#if 1	/* 必要ないかもしれないけど、あったほうが安全 */
			DG_COPY_VEC( &from, &GM_PlayerWork->control.mov ) ;
			{
				FVECTOR		center = { -450.0F, 0.0F, -250.0F } ;
				DG_SetPos( &this->body.objs->world ) ;
				DG_PutVector( &center, &to, 1 ) ;
			}
			to.vy = from.vy ;
			if ( HZX_OnlineHazardCheckOneSegment( seg, &from, &to ) ) {
				/* 閉じる */
				GV_MatToVec( &this->body.objs->world, &pos ) ;
				//GM_SeSetMode( this->sound[ LOCKER_SE_MOVING ], &pos, GM_SEMODE_NORMAL ) ;
				return 1 ;
			}
#else
			return 1 ;
#endif
		}
	}	
	return 0 ;
}

static 	int		CheckMessage2( this )
LOCKER2			*this ;
{
	int			n_msg ;
	int			*MsgBuf ;
	GV_MSG		*msg, Msg ;
	
	MsgBuf = this->msg_buf ;

	/* メッセージ先取り */
	if ( MsgBuf[ 5 ] == this->name ) {
		MsgBuf[ 5 ] = 0 ;
		n_msg = 1 ;
		msg = &Msg ;
		msg->message = MsgBuf ;
		msg->message_len = 5 ;
	} else {
		n_msg = GV_ReceiveMessage( this->name, &msg ) ;
	}
	if ( n_msg == 0 ) return 0 ;
	while ( -- n_msg >= 0 ) {
		switch( msg->message[ 0 ] ) {
		case LOCKER_MSG_NOT_OPEN :
			if ( !( this->status & LOCKER_STATE_CLOSE ) ) break ;
			this->next = LOCKER_STATE_CLOSE ;
			goto locker_move_setting ;			
		case LOCKER_MSG_OPEN :
			if ( !( this->status & LOCKER_STATE_CLOSE ) ) break ;
			this->next = LOCKER_STATE_OPEN ;
			goto locker_move_setting ;
		case LOCKER_MSG_PUT_CORPSE :
			if ( !( this->status & LOCKER_STATE_OPEN ) ) break ;
			this->next = LOCKER_STATE_CLOSE ;
			goto locker_move_setting ;
			break ;
		case LOCKER_MSG_CLOSE :
			if ( !( this->status & LOCKER_STATE_OPEN ) ) break ;
			this->next = LOCKER_STATE_CLOSE ;
locker_move_setting :
			this->chara = msg->message[ 1 ] ;
			this->status &= ~( LOCKER_STATE_FROM_OUT | LOCKER_STATE_FROM_IN ) ;
			if ( msg->message[ 2 ] == 0 ) this->status |= LOCKER_STATE_FROM_OUT ;
			else			  this->status |= LOCKER_STATE_FROM_IN ;
			this->motion = msg->message[ 3 ] ;
			this->command = LOCKER_COMMAND_CHANGE ;
			break ;
		case LOCKER_MSG_PUT_ENEMY :
			break ;
		default :
		  ;
		}
		msg ++ ;			
	}
	return 1 ;
}

/*----------------------------------------------------------------*/

static	void	ActDoor( work )
Work			*work ;
{
	LOCKER2		*this ;
	OBJECT		*body ;
	FVECTOR		v ;
	int			i, com, status, dir, diff ;

	for ( i = 0; i < N_Lockers; i ++ ) {
		this = ( LOCKER2 * )( Lockers + i ) ;
		
		//if ( this->floor != NULL ) HZX_ViewDynamicFloor( this->floor, 4 ) ;

		if ( ( this->status & LOCKER_STATE_SEARCH_ENEMY ) ) {
			/* 自分に入っている敵を探す */
			/* どのロッカーも最初に一回やるように変更 */
			SearchEnemy( this ) ;
			this->status &= ~LOCKER_STATE_SEARCH_ENEMY ;
		}

		body = &this->body ;
		/* 壊れ処理 */
		if ( this->status & LOCKER_STATE_BROKEN ) {
			this->command = LOCKER_COMMAND_NOTHING ;
			goto no_message ;
		}
		/* よりかかり閉じ */
		if ( CloseLockerbyLeaning2( this ) ) {
			this->next = LOCKER_STATE_CLOSE ;
			this->motion = DMclose_in ;
			this->command = LOCKER_COMMAND_CHANGE ;
			this->status &= ~LOCKER_STATE_FROM_IN ;
			this->status |= LOCKER_STATE_FROM_OUT | LOCKER_STATE_LEAN_CLOSE ;
			this->chara = GM_PlayerControl->name ;
		} else {
			/* メッセージチェック */
			if ( CheckMessage2( this ) == 0 ) goto no_message ;
		}
		com = this->command ;
		switch( com ) {
		case LOCKER_COMMAND_NOTHING :
			break ;
		case LOCKER_COMMAND_CHANGE :
			status = this->status ;
			this->command = LOCKER_COMMAND_MOVING ;
			ACTIVE_MOT( body->m_ctrl ) ;
			GM_ConfigObjectAction( body, 0, this->motion, 0, 0xfffff, 0 ) ;
			if ( this->status & LOCKER_STATE_MOTION_FAST ) {
				/* 早再生 */
				this->status &= ~LOCKER_STATE_MOTION_FAST ;
				GM_ConfigObjectAction( body, 0, this->motion, 
									   body->m_ctrl->mt3_ctrl[ 0 ].motion_total_time - TIME_BASE,
									   0xfffff, 0 ) ;			   
				body->m_ctrl->interp_flag = 0 ;
			}
			this->status &= ~( LOCKER_STATE_CLOSE | LOCKER_STATE_OPEN ) ;
			this->count = 0 ;
			/* 開始時プロック呼び出し */
			SETARG( 0, 1 ) ;
			SETARG( 1, this->chara ) ;
			if ( this->status & LOCKER_STATE_FROM_IN ) {
				if ( this->next == LOCKER_STATE_OPEN ) SETARG( 2, 2 ) ;
				else								   SETARG( 2, 3 ) ;
			} else {
				if ( this->next == LOCKER_STATE_OPEN ) SETARG( 2, 0 ) ;
				else								   SETARG( 2, 1 ) ;
			}

			/* ルート遮断状態更新 */
			if ( !( this->status & LOCKER_STATE_DESTROY ) ) {
				this->r_intrpt.status = ROOT_INTRPT_MOVE | ROOT_INTRPT_HINGED ;
			}
			SEGMENT4_ON( this ) ;
			SEGMENT5_ON( this ) ;
			ExecProc2( this, 3 ) ;
			if ( !( status & LOCKER_STATE_LEAN_CLOSE ) ) {
				SetDynamicSegAtr( this, HZX_SEG_NO_PLAYER ) ;
			} else {
				/* よりかかり閉じのときは、
				   ＳＥＧ４でプレイヤーの進入を防ぐ */
				this->segment4->atr &= ~HZX_SEG_NO_PLAYER ;
				*( int * )GV_UNCACHE( &this->segment4->atr ) = this->segment4->atr ;
			}
			this->status &= ~LOCKER_STATE_LEAN_CLOSE ;
			break ;
		case LOCKER_COMMAND_MOVING :
			break ; 
		default :
		  ;
		}
no_message :
	    GM_SetCurrentMap( body->map_name ) ;
		GM_GroupObjs( body->objs, GM_CurrentMap ) ;
#if 0
		/* モーション処理 */
		GM_ActMotion( body ) ;
		DG_SetPos( &body->objs->world ) ;
		GM_ActObject2( body ) ;
		RotateSegment2( this ) ;
#endif
		if ( this->command == LOCKER_COMMAND_MOVING ) {
			/* 可動中フラグ設定 */
			if ( !( this->status & ( LOCKER_STATE_BROKEN | LOCKER_STATE_DESTROY ) ) ) {
				GV_SetVec3( &v, -10.0F, 0.0F, 0.0F ) ;
				DG_SetPos( &body->objs->objs[ 0 ].world ) ;
				DG_RotVector( &v, &v, 1 ) ;
				dir = GV_VecDir2( &v ) ;
				diff = GV_DiffDirAbs( dir, this->def_rot ) ;
				if ( diff < 32 || diff > 768 ) {
					if ( !( this->status & LOCKER_STATE_DONOT_OFF_SEGMENT4 ) ) {
						/* 念のため、敵兵がゾーン内にいないかチェック */
						if ( !CheckEnemyInZone( this ) ) {
							ResetDynamicSegAtr( this, HZX_SEG_NO_ENEMY_EYES | HZX_SEG_NO_ENEMY ) ;
						} else {
							/* 敵がいたら危険なので、当たりを消す */
							SetDynamicSegAtr( this, HZX_SEG_NO_ENEMY ) ;
							//printf( "locker door active : enemy in locker zones.\n" ) ;
						}
//						printf( "reset enemy eyes\n" ) ;
					}
					if ( this->status & LOCKER_STATE_NO_OPEN_SEG ) {
						/* 壁際ロッカー */
						if ( diff < 32 ) {
							SetDynamicSegAtr( this, HZX_SEG_NO_ENEMY_EYES | HZX_SEG_NO_ENEMY ) ;
						}
					}
					/* 敵放出 */
					if ( !( this->status & LOCKER_STATE_FROM_IN ) && 
						this->next == LOCKER_STATE_OPEN &&
						diff < 1024 - 56 ) {
						if ( this->status & LOCKER_STATE_CORPSE_IN ) {
							if ( this->capture != NULL ) this->capture->flag &= ~CAPTURE_LOCKER ;
						}
						this->status &= ~LOCKER_STATE_CORPSE_IN ;
					}
					/* 敵開けの時は、SEGMENT4でプレイヤーの外からの進入を防ぐ */
					if ( !( this->status & LOCKER_STATE_FROM_IN ) && 
						this->chara == 0 &&
						this->next == LOCKER_STATE_OPEN ) {
						if ( diff > 768 ) {
							this->segment4->atr &= ~HZX_SEG_NO_PLAYER ;
							*( int * )GV_UNCACHE( &this->segment4->atr ) = this->segment4->atr ;
						} else if ( diff < 32 ) {
							this->segment4->atr |= HZX_SEG_NO_PLAYER ;
							*( int * )GV_UNCACHE( &this->segment4->atr ) = this->segment4->atr ;
						}
					}
				} else {
					if ( this->chara == 0 ) SetBlowTarget( this, PLAYER_SIDE ) ;
					SetDynamicSegAtr( this, HZX_SEG_NO_ENEMY_EYES | HZX_SEG_NO_ENEMY ) ;
//					printf( "set enemy eyes\n" ) ;
				}
			}
#if 1
			/* モーション処理 */
			GM_ActMotion( body ) ;

			/* トイレ時音きり変え用 */
			{
				int		seNoSeg = -1 ;
				if ( this->flag & LOCKER_FLAG_TOILET ) seNoSeg = 1 ;
				MT_SetMotionSeTable( body->m_ctrl, body->map_name, 0, -1, seNoSeg ) ;
			}

			DG_SetPos( &body->objs->world ) ;
			GM_ActObject2( body ) ;
			RotateSegment2( this ) ;
#endif
			if ( MT_CHECK_END( body->m_ctrl, 0 ) ) {
				SLEEP_MOT( body->m_ctrl ) ;
				if ( this->status & LOCKER_STATE_DONOT_OFF_SEGMENT4 ) {
					this->status &= ~LOCKER_STATE_DONOT_OFF_SEGMENT4 ;
				} else {
					SEGMENT4_OFF( this ) ;
				}
				ResetDynamicSegAtr( this, HZX_SEG_NO_PLAYER | 
								    HZX_SEG_NO_ENEMY_EYES | HZX_SEG_NO_ENEMY ) ;
				if ( this->status & LOCKER_STATE_NO_OPEN_SEG ) {
					/* 壁際ロッカー */
					if ( this->next == LOCKER_STATE_OPEN ) {
						SetDynamicSegAtr( this, HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY_EYES |
										  HZX_SEG_NO_ENEMY ) ;
					}
				}
				this->command = LOCKER_COMMAND_NOTHING ;
				this->status |= this->next ;
				/* 終了時プロック呼び出し */
				SETARG( 0, 0 ) ;
				ExecProc2( this, 3 ) ;
				if ( this->next == LOCKER_STATE_OPEN ) {
					//this->segment3->atr &= ~HZX_SEG_SKIP ;
					if ( this->status & LOCKER_STATE_NO_OPEN_SEG ) {
						/* 壁際ロッカー */
						SEGMENT5_OFF( this ) ;
					} else {
						SEGMENT5_ON( this ) ;
					}
				} else {
					//this->segment3->atr |= HZX_SEG_SKIP ;
					SEGMENT5_OFF( this ) ;
				}
				//*( int * )GV_UNCACHE( &this->segment3->atr ) 
				//	= this->segment3->atr ;
				/* ルート遮断状態更新 */
				if ( !( this->status & LOCKER_STATE_DESTROY ) ) {
					if ( this->next == LOCKER_STATE_OPEN ) {
						this->r_intrpt.status = ROOT_INTRPT_OPEN | ROOT_INTRPT_HINGED ;
					} else {
						this->r_intrpt.status = ROOT_INTRPT_CLOSE | ROOT_INTRPT_HINGED ;
					}
				}
			}
			this->lean = 0 ;
			DG_COPY_MAT( &this->def_world, &this->body.objs->objs[ 0 ].world ) ;
			if ( this->lean_level != 0 ) LeanDoor( this, &DG_ZeroSVector ) ;
		} else {
			if ( this->status & LOCKER_STATE_BROKEN ) {
				/* 壊れ処理 */
				BrokenPhase( this ) ;
			} else if ( this->lean_level != 0 ) {
				SVECTOR	rot = { 0, 48, 8 } ;

				rot.vz *= this->lean_level ;
				if ( this->lean == 0 ) LeanDoor( this, &rot ) ;
				if ( SameRot( &this->lean_rot, &rot ) ) {
					if ( this->lean == 0 ) {
						if ( !( this->status & LOCKER_STATE_LEAN_START ) ) {
							//GM_SeSetMode( SD_A_LOCKDMG1, &this->center, GM_SEMODE_BOMB ) ;
							if ( this->flag & LOCKER_FLAG_TOILET ) {
								GM_SeSetMode( SD_A_TOILDWN2, &this->center, GM_SEMODE_BOMB ) ;
							} else {
								GM_SeSetMode( SD_A_LOCKDWN2, &this->center, GM_SEMODE_BOMB ) ;
							}
						}
						this->status &= ~LOCKER_STATE_LEAN_START ;
					}
					this->lean = 1 ;
				}
			}
		}

		if ( this->shadow_obj != NULL ) {
			FVECTOR	shift = DG_ZeroVector ;

			shift.vz = 2.0F ;
			DG_SetPos( &body->objs->objs[ 0 ].world ) ;
			DG_MovePos( &shift ) ;
			DG_GetPos( &this->shadow_world ) ;
		}
#if 0
		HZX_ViewDynamicSegment( this->segment ) ;
		//HZX_ViewDynamicSegment( this->segment2 ) ;
		//HZX_ViewDynamicSegment( this->segment3 ) ;
#endif
#if 0
		if ( !( this->segment4->atr & ( HZX_SEG_SKIP | HZX_SEG_NO_ENEMY ) ) ) {
			HZX_ViewDynamicSegment( this->segment4 ) ;
		}
#endif
#if 0
		if ( !( this->segment5->atr & HZX_SEG_SKIP ) ) {
			HZX_ViewDynamicSegment( this->segment5 ) ;
		}
#endif
#if 0
		if ( !( this->segment->atr & HZX_SEG_NO_ENEMY ) ) {
			HZX_ViewDynamicSegment( this->segment ) ;
		}
#endif
		//HZX_ViewDynamicSegment( this->segment6 ) ;

		/* ターゲット移動 */
#if 0
		{
			FVECTOR		shift = { -450.0F, 1100.0F, 0.0F } ;
		
			DG_SetPos( &body->objs->objs[ 0 ].world ) ;
			DG_PutVector( &shift, &shift, 1 ) ;
			GM_MoveTarget( &this->target, &shift ) ;
//			NewTargetView2( &this->target, 32, 232, 32 ) ;
		}
#else
		if ( !( this->status & LOCKER_STATE_BROKEN ) )
		{
			TARGET		*t ;
			FVECTOR		close_size = 	{ 450.0F, 1000.0F, 100.0F } ;
			FVECTOR		close_shift = 	{ -450.0F, 1100.0F, 100.0F } ;
			FVECTOR		open_size =  	{ 450.0F, 1000.0F, 100.0F } ;
			FVECTOR		open_shift = 	{ -450.0F, 1100.0F, 0.0F } ;

			t = &this->target ;

			if ( this->status & LOCKER_STATE_CLOSE ) {
				DG_COPY_VEC( &t->offset, &close_shift ) ;
				GM_SetTargetSize( t, &close_size ) ;
			} else {
				DG_COPY_VEC( &t->offset, &open_shift ) ;
				GM_SetTargetSize( t, &open_size ) ;
			}

			GM_MoveTarget2( t, &body->objs->objs[ 0 ].world ) ;
			//NewTargetView2( t, 32, 232, 32 ) ;
		}
#endif
		/* ポヨヨン */
		if ( !( this->status & LOCKER_STATE_BROKEN ) && 
			 ( this->status & LOCKER_STATE_POSTER ) ) {
#ifdef DEBUG_MODE
			if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW ) {
				HZX_ViewDynamicSegment( this->gravure->special1 ) ;
				HZX_ViewDynamicSegment( this->gravure->special2 ) ;
			}
#endif
			if ( GM_PlayerWork != NULL && PL_Flag( FLAG_KNOCK_WALL ) ) {
				if ( PL_KnockSegment.attribute & HZX_SEG_DYNAMIC ) {
//printf( "%x %x %x\n", PL_KnockSegment.ptr, this->gravure->special1,
//	     this->gravure->special2 ) ;
					GM_SetCurrentMap( this->body.map_name ) ;
					if ( PL_KnockSegment.ptr == ( void * )this->gravure->special1 ) {
//						printf( "special1!!\n" ) ;						
						GM_SeSetMode( SD_A_LOCKMIL1, &this->center, GM_SEMODE_BOMB ) ;
					} else if ( PL_KnockSegment.ptr == ( void * )this->gravure->special2 ) {
						extern void	AN_HeadMark( FMATRIX *, int ) ;
						FMATRIX		*m ;

//						printf( "special2!!\n" ) ;						
						GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_E_BIKKRI01 ) ;
						GM_SetNoise( NOISE_L, &this->center, GM_CurrentMap ) ;
//						DG_COPY_MAT( &m, &DG_UnitMatrix ) ;
//						m.m[ 3 ][ 0 ] = ( float )( this->gravure->special2->p1.vx + rand() % 256 ) ;
//						m.m[ 3 ][ 1 ] = ( float )( this->gravure->special2->p1.vy + rand() % 256 ) ;
//						m.m[ 3 ][ 2 ] = ( float )( this->gravure->special2->p1.vz + rand() % 256 ) ;
						m = &GM_PlayerBody->objs->objs[ PL_KnockJoint ].world ;
						AN_HeadMark( m, 0 ) ;
						AN_HeadMark( m, 0x4000 ) ;
					}
				} 
			}
		}
	}
}

static	void	DieDoor( work )
Work			*work ;
{
    int		i ;
	LOCKER2	*l ;
    if ( Lockers != NULL ) {
		for ( i = 0; i < N_Lockers; i ++ ) {
			l = ( LOCKER2 * )( Lockers + i ) ;
			DG_FreePreshade( l->body.objs ) ;
			l->body.m_ctrl->sar_ctrl = l->sar_ctrl ;
			GM_FreeObject( &l->body ) ;
			HZX_RemoveDynamicSegment( l->segment ) ;
			//HZX_RemoveDynamicSegment( l->segment2 ) ;
			//HZX_RemoveDynamicSegment( l->segment3 ) ;
			HZX_RemoveDynamicSegment( l->segment4 ) ;
			HZX_RemoveDynamicSegment( l->segment5 ) ;
			HZX_RemoveDynamicSegment( l->segment6 ) ;
			HZX_RemoveDynamicFloor( l->floor ) ;
			if ( l->objhzx != NULL ) GV_DestroyOtherActor( l->objhzx ) ;
			if ( l->box != NULL ) {
				DG_FreePreshade( l->box ) ;
				DG_DequeueObjs( l->box ) ;
				DG_FreeObjs( l->box ) ;
				HZX_RemoveDynamicSegment( l->b_seg1 ) ;
				HZX_RemoveDynamicSegment( l->b_seg2 ) ;
				HZX_RemoveDynamicFloor( l->b_flr ) ;
			}
			if ( l->shadow_obj != NULL ) {
				GV_DestroyActor( l->shadow_obj ) ;
			}
			if ( l->gravure != NULL ) {
				HZX_RemoveDynamicSegment( l->gravure->special1 ) ;
				HZX_RemoveDynamicSegment( l->gravure->special2 ) ;
				GV_DelayedFree( l->gravure ) ;
			}
			if ( l->flag & LOCKER_FLAG_SHADOWWRITE ) {
				GM_FreeObject( &l->shadow_write ) ;
			}
			GM_FreeTarget( &l->target ) ;
			SETARG( 0, 0 ) ;
			SETARG( 1, 0 ) ;
			SETARG( 2, 8 ) ;
			/* status の上位にhit_countを入れる */
			l->status |= ( ( l->hit_count & 0xff ) << 24 ) ;
			ExecProc2EX( l, 3, 1 ) ;
		}
		GV_DelayedFree( Lockers ) ;
    }	
}

/*----------------------------------------------------------------*/

#ifdef KOREADD
int		RIntrptCallBack( int mot, int num, int order, void *ptr )
{
	LOCKER2	*this ;
	int		*MsgBuf ;

	this = ( LOCKER2 * )ptr ;

	MsgBuf = this->msg_buf ;
	if ( MsgBuf[ 5 ] == this->name ) {
		printf( "locker warning : rintrpt callback is failed.\n" ) ;
		return 0 ;
	}

	MsgBuf[ 0 ] = order ;
	MsgBuf[ 1 ] = 0 ;
	MsgBuf[ 2 ] = 0 ;
	MsgBuf[ 3 ] = num ;
	MsgBuf[ 4 ] = 0 ;
	MsgBuf[ 5 ] = this->name ;

printf("locker: mot[%d] num[%d] order[%d]\n",mot,num,order ) ;
	return 1 ;
}
#endif

/* 登録 */
static	void	*NewLockerDoor( name, where )
int				name, where ;
{
    LOCKER2			*this ;
	OBJECT			*body ;
    IVECTOR			iv ;
    FVECTOR			pos ;
	FVECTOR			max, min ;
	IVECTOR			p[ 2 ] ;
	SVECTOR			rot ;
    int				zn, model, flag, bflag, i ;
	int				seNo, start_corpse_in = 0 ;
	int				load_status, status ;
	HZX_GROUP_ID	hzx_id ;

//	ASSERT( sizeof( LOCKER ) > sizeof( LOCKER2 ) ) ;
    ASSERT( LockerWork != NULL ) ;
    ASSERT( N_Lockers < MAX_Lockers ) ;

	flag = 0 ;
	if ( GCL_GetOption( 'f' ) != NULL ) flag = GCL_GetNextInt() ;

    this = ( LOCKER2 * )&( Lockers[ N_Lockers ] ) ;
    this->name = name ;
	this->flag = flag ;

	model = GCL_GetOptionValue( 'k', 0 ) ;

	body = &this->body ;
	bflag = BODY_FLAG2 ;
//	if ( flag & LOCKER_FLAG_SHADOWDROP ) bflag |= DG_FLAG_SHADOWMAKE ;
	GM_InitObject( body, model, bflag ) ;
//	DG_InvisibleObjs( body->objs ) ;
    ASSERT( body->objs != NULL ) ;

    // AS(JM) - Removing the locker door from the MSAA scene makes the "slit" that you
    // look through not be disconnected from the locker itself.
    body->objs->flag |= DG_FLAG_NOMSAA;

	if ( flag & LOCKER_FLAG_SHADOWWRITE ) {
		GM_InitObject( &this->shadow_write, model, DG_FLAG_SHADOWWRITE | DG_FLAG_ONEPIECE ) ;
		GM_ConfigObjectRoot( &this->shadow_write, body, 0 ) ;
        this->shadow_write.objs->shadow_id = GCL_GetOptionValue( 'l', 0 ) ;
	}

	/* モーション */
	GM_ConfigObjectMotion( body, 1, LockerMotion, MT_FLAG_HUMAN2 ) ;
	GM_ConfigObjectStep( body, &this->step ) ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;
	SLEEP_MOT( body->m_ctrl ) ;
	this->sar_ctrl = body->m_ctrl->sar_ctrl ;

    /* 位置設定 */
    if ( GCL_GetOption( 'r' ) == NULL ) iv = DG_ZeroIVector ;
    else {
		GCL_GetNextIV( ( int * )&iv ) ;
    }
	rot.vx = iv.vx ;
	rot.vy = iv.vy ;
	rot.vz = iv.vz ;
    if ( GCL_GetOption( 'p' ) == NULL ) iv = DG_ZeroIVector ;
    else {
		GCL_GetNextIV( ( int * )&iv ) ;
    }
	GV_IVtoFV( &iv, &pos, 3 ) ;
	DG_SetPos2( &pos, &rot ) ;
	DG_PutObjs( body->objs ) ;
	for ( i = 0; i < body->objs->n_models; i ++ ) {
		DG_GetPos( &body->objs->objs[ i ].world ) ;
	}
	DG_GetPos( &this->def_world ) ;
	this->def_rot = rot.vy ;

	/* 倒れたときの角度 */
    if ( GCL_GetOption( 'd' ) == NULL ) {
		GV_ConvVec3( &rot, &this->down_rot ) ;
    } else {
		GCL_GetNextIV( ( int * )&iv ) ;
		GV_ConvVec3( &iv, &this->down_rot ) ;
    }
	/* 倒れたときのシフト量 */
	if ( GCL_GetOption( 'W' ) == NULL ) {
		DG_COPY_VEC( &this->down_shift, &DG_ZeroVector ) ;
	} else {
		GCL_GetNextIV( ( int * )&iv ) ;
		GV_ConvVec3( &iv, &this->down_shift ) ;
	}
	DG_COPY_VEC( &this->down_shift_cur, &DG_ZeroVector ) ;

#ifdef KOREADD
	{
		HZX_ZONE_ADD zone[2] ;

		HZX_GetInterruptZone( &pos, rot.vy-1024, 250, &zone[0] ) ;
		GM_SetRouteIntrpt( &this->r_intrpt, zone[0], zone[1], &pos, rot.vy-1024,
				ROOT_INTRPT_HINGED|ROOT_INTRPT_CLOSE, 0 ) ;
		GM_PutRoteIntrpt( &this->r_intrpt ) ;
		GM_SetRIntrptCallBack( this, &this->r_intrpt,  RIntrptCallBack ) ;

		this->zones[ 0 ] = zone[ 0 ] ;
		this->zones[ 1 ] = zone[ 1 ] ;
	}
#endif

	hzx_id = GM_GetHzxGroupID( where ) ;
	hzx_id = HZX_GetHzxIDbyZone( hzx_id, &pos, &zn ) ;
    /* プリシェード */
    DG_MakePreshade( body->objs, GM_GetMap( where )->light ) ;
    /* グループ化 */
    GM_GroupObjs( body->objs, where ) ;
	body->map_name = where ;

	/* 壁音番号 */
	seNo = GCL_GetOptionValue( 'N', 0 ) ;
	seNo = HZX_SeCode( seNo ) ;

    /* 壁 */
	max.vx = body->objs->def->models[ 0 ].ux + 0.50F ;
	max.vy = body->objs->def->models[ 0 ].uy ;
	max.vz = body->objs->def->models[ 0 ].uz ;
	min.vx = body->objs->def->models[ 0 ].lx - 0.50F ;
	min.vy = body->objs->def->models[ 0 ].ly ;
	min.vz = body->objs->def->models[ 0 ].lz ;

	{
		int			tmp ;

		tmp = ( int )max.vx ; tmp += ( tmp < 0 ) ? -2 : 2 ; 
		tmp = tmp / 5 * 5 ; max.vx = ( float )tmp ;
		tmp = ( int )max.vy ; tmp += ( tmp < 0 ) ? -2 : 2 ; 
		tmp = tmp / 5 * 5 ; max.vy = ( float )tmp ;
		tmp = ( int )max.vz ; tmp += ( tmp < 0 ) ? -2 : 2 ; 
		tmp = tmp / 5 * 5 ; max.vz = ( float )tmp ;
		tmp = ( int )min.vx ; tmp += ( tmp < 0 ) ? -2 : 2 ; 
		tmp = tmp / 5 * 5 ; min.vx = ( float )tmp ;
		tmp = ( int )min.vy ; tmp += ( tmp < 0 ) ? -2 : 2 ; 
		tmp = tmp / 5 * 5 ; min.vy = ( float )tmp ;
		tmp = ( int )min.vz ; tmp += ( tmp < 0 ) ? -2 : 2 ; 
		tmp = tmp / 5 * 5 ; min.vz = ( float )tmp ;
	}
	
	max.vx += 50.0F ;
	min.vx -= 50.0F ;
	min.vy -= 200.0F ;

	/* 扉が開いているとき用の端の小さな壁 */
	{
		FVECTOR		fv[ 2 ] ;

		GV_SetVec3( &fv[ 0 ], max.vx, min.vy, max.vz ) ; 
		GV_SetVec3( &fv[ 1 ], max.vx - 50.0F, min.vy, max.vz ) ; 
		DG_SetPos( &body->objs->objs[ 0 ].world ) ;
		DG_PutVector( fv, fv, 2 ) ;
		GV_FVtoIV( fv, p, 8 ) ;

		p[ 0 ].vw = p[ 1 ].vw = 2200.0F ;
		this->segment5 = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], HZX_SEG_NO_C4 ) ;
		ASSERT( this->segment5 != NULL ) ;	
		SEGMENT5_OFF( this ) ;		
	}
	/* 扉が開いているときの内部用の壁 */
	{
		FVECTOR		fv[ 2 ] ;

		GV_SetVec3( &fv[ 0 ], max.vx, min.vy, max.vz - 500.0F ) ; 
		//GV_SetVec3( &fv[ 1 ], max.vx - 50.0F, min.vy, max.vz ) ; 
		GV_SetVec3( &fv[ 1 ], max.vx - 50.0F, min.vy, max.vz - 50.0F ) ; 
		DG_SetPos( &body->objs->objs[ 0 ].world ) ;
		DG_PutVector( fv, fv, 2 ) ;
		GV_FVtoIV( fv, p, 8 ) ;
		p[ 0 ].vw = p[ 1 ].vw = 2200.0F ;
		this->segment6 = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 
											    HZX_SEG_ATR_ALL & ~( HZX_SEG_NO_PLAYER | 
																	 HZX_SEG_NO_ENEMY |
																	 HZX_SEG_NO_ENEMY_EYES ) ) ;
		ASSERT( this->segment6 != NULL ) ;	
	}

	_sceVu0AddVector( &max, &max, &pos ) ;
	_sceVu0AddVector( &min, &min, &pos ) ;

	p[ 0 ].vw = p[ 1 ].vw = 2200.0F ;
	/* 実験中のため、扉当たりを下げる */
//	p[ 0 ].vw = p[ 1 ].vw = 1400.0F ;

	p[ 0 ].vx = min.vx ;
	p[ 0 ].vy = min.vy ;
//	p[ 0 ].vz = min.vz ;
	p[ 0 ].vz = max.vz - 50.0F ;
	p[ 1 ].vx = max.vx ;
	p[ 1 ].vy = min.vy ;
//	p[ 1 ].vz = min.vz ;
	p[ 1 ].vz = max.vz - 50.0F ;
#if 0
	this->segment2 = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 
										   HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY | 
										   HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_C4 | seNo ) ;
	ASSERT( this->segment2 != NULL ) ;
#endif
	p[ 0 ].vz = max.vz ;
	p[ 1 ].vz = max.vz ;

	this->segment = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], HZX_SEG_NO_BULLETHOLE | 
										   HZX_SEG_NO_BULLET | HZX_SEG_NO_MISSILE |
										   HZX_SEG_NO_C4 | seNo ) ;
	ASSERT( this->segment != NULL ) ;	
	HZX_DynamicSegmentSetAttribute( this->segment, this->segment->atr & ~HZX_SEG_NO_DISP_RADAR ) ;

	/* ３枚目は張り付き不可 */
	p[ 0 ].vx = min.vx ;
	p[ 0 ].vy = min.vy ;
//	p[ 0 ].vz = min.vz ;
	p[ 0 ].vz = max.vz - 50.0F ;
	p[ 1 ].vx = min.vx ;
	p[ 1 ].vy = min.vy ;
	p[ 1 ].vz = max.vz ;

	p[ 0 ].vx = ( p[ 0 ].vx + 2 ) / 5 * 5 ;
	p[ 0 ].vz = ( p[ 0 ].vz + 2 ) / 5 * 5 ;
	p[ 1 ].vx = ( p[ 1 ].vx + 2 ) / 5 * 5 ;
	p[ 1 ].vz = ( p[ 1 ].vz + 2 ) / 5 * 5 ;
#if 0
	this->segment3 = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 
										    HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY | 
										    HZX_SEG_NO_HARITSUKI | HZX_SEG_SKIP | 
										    HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_C4 | seNo ) ;
	ASSERT( this->segment3 != NULL ) ;
#endif

	/* segment2, segment3 を廃止して、
	   ObjectBoundHazardを使用する */
	this->objhzx = NewMakeObjectBoundHazard2( hzx_id, body->objs, 							
											 HZX_SEG_RECOIL_TYPE |	/* これを付けるとobjs[0]でみる */
											 HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY | 
											 HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_C4 | seNo,
											 HZX_FLOOR_NO_PLAYER | HZX_FLOOR_NO_ENEMY |
											 HZX_FLOOR_NO_BULLETHOLE | HZX_FLOOR_NO_C4 |
											 HZX_FLOOR_NO_CLAYMORE | HZX_FLOOR_NO_BLOOD |
											 HZX_FLOOR_NO_OBJECT | seNo, 1 ) ;
	ASSERT( this->objhzx != NULL ) ;
	PL_ObjectBoundHazardSetTarget( this->objhzx, &this->t1 ) ;

	RotateSegment2( this ) ;

	this->segment4 = HZX_AddDynamicSegment( hzx_id, &this->segment->p1, 
										   &this->segment->p2,
										   HZX_SEG_ATR_ALL & ~( HZX_SEG_NO_ENEMY | 
															    HZX_SEG_NO_ENEMY_EYES ) ) ;
	ASSERT( this->segment4 != NULL ) ;	
	SEGMENT4_OFF( this ) ;

	GM_SetTarget( &this->t1, TARGET_DEFENSE | TARGET_POWER | TARGET_THROUGH | TARGET_HAZARD, 
				  0, BOTH_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargettoDynamicHazard( this->segment, &this->t1, HZX_TYPE_SEGMENT ) ;
	//GM_SetTargettoDynamicHazard( this->segment2, &this->t1, HZX_TYPE_SEGMENT ) ;
	//GM_SetTargettoDynamicHazard( this->segment3, &this->t1, HZX_TYPE_SEGMENT ) ;
	GM_SetTargetCallBack( &this->t1, Hit, this ) ;

	/* 爆発受け止め用ターゲット */
#if 0
	{
		FVECTOR		size = { 100.0F, 100.0F, 100.0F }, pos ;

		GM_SetTarget( &this->target, TARGET_DEFENSE | TARGET_POWER | TARGET_THROUGH, 
					  0, BOTH_SIDE, &size, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( &this->target, Hit, this ) ;
		pos.vx = ( float )( this->segment->p1.vx + this->segment->p2.vx ) / 2.0F ;
		pos.vy = ( float )( this->segment->p1.vy + ( float )this->segment->p1.vw / 2.0F ) ;
		pos.vz = ( float )( this->segment->p1.vz + this->segment->p2.vz ) / 2.0F ;
		GM_MoveTarget( &this->target, &pos ) ;
		GM_PutTarget( &this->target ) ;	
	}
#else
	{
		TARGET			*t ;
		FVECTOR			size = { 450.0F, 1000.0F, 200.0F } ;
		FVECTOR			shift = { -450.0F, 1100.0F, 0.0F } ;
		
		t = &this->target ;
		GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER | TARGET_ROTATE | TARGET_THROUGH,
					  0, BOTH_SIDE, &size, &shift ) ;
		GM_MoveTarget2( t, &this->body.objs->objs[ 0 ].world ) ;
		GM_SetTargetCallBack( t, Hit, this ) ;
		GM_PutTarget( t ) ;
	}	
#endif

#if 0
	pos3.vx = 0.0F ;
	pos3.vy = pos3.vz = 0.0F ;
	DG_SetPos2( &pos, &rot ) ;
	DG_PutVector( &pos3, &pos3, 1 ) ;	
	pos2.vx = 1000.0F ;
	pos2.vy = -200.0F ;
	pos2.vz = 0.0F ;
	DG_SetPos2( &pos3, &DG_ZeroSVector ) ;
	DG_PutVector( &pos2, &pos2, 1 ) ;
	pos3.vy = pos2.vy ;
	pos.vw = pos2.vw = 2000.0F ;
	GV_FVtoIV( &pos3, &p[ 0 ], 4 ) ;
	GV_FVtoIV( &pos2, &p[ 1 ], 4 ) ;
	/* 実験中のため、扉当たりを下げる */
	p[ 0 ].vw = p[ 1 ].vw = 1400.0F ;
	this->segment = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 0 ) ;
	ASSERT( this->segment != NULL ) ;
	RotateSegment2( this ) ;
#endif

    /* 初期状態 */
	status = GCL_GetOptionValue( 's', LOCKER_STATE_CLOSE ) ;
	if ( !( status & ( LOCKER_STATE_CLOSE | LOCKER_STATE_OPEN ) ) ) status |= LOCKER_STATE_CLOSE ;
	load_status = GCL_GetOptionValue( 'A', status ) ;

	printf( "--------------- locker state get %x %x\n", 
		     load_status, status ) ;

	if ( load_status == 0 ) load_status = status ;

	load_status &= ( LOCKER_STATE_OPEN | LOCKER_STATE_CLOSE | LOCKER_STATE_CORPSE_IN |
					 LOCKER_STATE_BROKEN ) | 0xff000000 ;

	if ( load_status & LOCKER_STATE_BROKEN ) {
		status &= ~( LOCKER_STATE_OPEN | LOCKER_STATE_CLOSE ) ;
		status |= LOCKER_STATE_BROKEN | LOCKER_STATE_BROKEN_FAST ;
		load_status &= ~LOCKER_STATE_CORPSE_IN ;
	} else if ( ( load_status & LOCKER_STATE_CLOSE ) &&
			    ( status & LOCKER_STATE_OPEN ) ) {
		status &= ~LOCKER_STATE_OPEN ;
		status |= LOCKER_STATE_CLOSE ;
	} else if ( ( load_status & LOCKER_STATE_OPEN ) &&
			    ( status & LOCKER_STATE_CLOSE ) ) {
		status &= ~LOCKER_STATE_CLOSE ;
		status |= LOCKER_STATE_OPEN ;		
		load_status &= ~LOCKER_STATE_CORPSE_IN ;
	}
	if ( status & LOCKER_STATE_CORPSE_IN ) {
		start_corpse_in = 1 ;
	} else if ( ( load_status & LOCKER_STATE_CORPSE_IN ) &&
		 !( status & LOCKER_STATE_CORPSE_IN ) ) {
		status |= LOCKER_STATE_CORPSE_IN | LOCKER_STATE_SEARCH_ENEMY ;
	} else if ( !( load_status & LOCKER_STATE_CORPSE_IN ) &&
			    ( status & LOCKER_STATE_CORPSE_IN ) ) {
		status &= ~LOCKER_STATE_CORPSE_IN ;
	}

	this->hit_count = ( load_status & 0xff000000 ) >> 24 ;
	if ( this->hit_count >= 15 ) this->lean_level = 3 ;
	else if ( this->hit_count >= 10 ) this->lean_level = 2 ;
	else if ( this->hit_count >= 5 ) this->lean_level = 1 ;	
	if ( this->lean_level > 0 ) status |= LOCKER_STATE_LEAN_START ;
	
	printf( "--------------- locker start state %x %x %d/%d\n", 
		     load_status, status, this->hit_count, this->lean_level ) ;

	/* どのロッカーも最初に一回敵をさがす */
	/* ロッカー死体設定時はしない */
	if ( start_corpse_in == 0 ) status |= LOCKER_STATE_SEARCH_ENEMY ;

	this->status = status ;
	if ( !( this->status & 3 ) ) this->status |= LOCKER_STATE_CLOSE ;

	/* 開いている */
	if ( this->status & LOCKER_STATE_OPEN ) {
		int			buf[ 5 ] ;

		buf[ 0 ] = LOCKER_MSG_OPEN ;
		buf[ 1 ] = buf[ 2 ] = buf[ 4 ] = 0 ;
		buf[ 3 ] = DMopen_out ;
		PL_SendMessage( this->name, buf, 5 ) ;

		this->status &= ~LOCKER_STATE_OPEN ;
		this->status |= LOCKER_STATE_CLOSE | LOCKER_STATE_MOTION_FAST ;
	}

	/* 開かないロッカー設定 */
	if ( this->status & LOCKER_STATE_DESTROY ) {
		this->r_intrpt.status |= ROOT_INTRPT_BREAK ;
		/* 壁属性追加 */
		this->segment4->atr &= ~HZX_SEG_NO_PLAYER ;
		*( int * )GV_UNCACHE( &this->segment4->atr ) = this->segment4->atr ;
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
			int		spot_name ;

			spot_name = GCL_GetOptionValue( 'n', 0 ) ;
			DG_COPY_MAT( &this->shadow_world, &body->objs->objs[ 0 ].world ) ;
			this->shadow_obj 
				= NewSpotDrawObject( spot_name, &this->shadow_world, model ) ;
		}
	}

	/* ロッカーの中心 */
	GV_SetVec3( &this->center, -450.0F, 0.0F, -250.0F ) ;
	DG_SetPos( &this->body.objs->world ) ;
	DG_PutVector( &this->center, &this->center, 1 ) ;

	this->capture = NULL ;

	/* グラビア設定 */
	if ( this->status & LOCKER_STATE_POSTER ) {
		GRAVURE_SET		*g ;
		FVECTOR			*pos, f1, f2 ;
		IVECTOR			p1, p2 ;

		pos = ( FVECTOR * )( &this->body.objs->world.m[ 3 ] ) ;
		this->gravure = g = GV_Malloc( sizeof( GRAVURE_SET ) ) ;
		GV_ZeroMemory( g, sizeof( GRAVURE_SET ) ) ;
		if ( g == NULL ) this->status &= ~LOCKER_STATE_POSTER ;
		else {
			GCL_GetOption( 'g' ) ;
			/* キス音範囲 */
			GCL_GetNextIV( ( int * )&iv ) ;
			iv.vz -= 80 ;
			GV_IVtoFV( &iv, &g->kissb1, 3 ) ;
			GCL_GetNextIV( ( int * )&iv ) ;
			iv.vz -= 80 ;
			GV_IVtoFV( &iv, &g->kissb2, 3 ) ;
			/* 胸範囲 */
			GCL_GetNextIV( ( int * )&iv ) ;
			GV_IVtoFV( &iv, &f1, 3 ) ;			
			GCL_GetNextIV( ( int * )&iv ) ;
			GV_IVtoFV( &iv, &f2, 3 ) ;		
			f1.vz -= 80.0F ;
			f2.vz -= 80.0F ;
			_sceVu0AddVector( &f1, &f1, pos ) ;
			_sceVu0AddVector( &f2, &f2, pos ) ;
			GV_SetVec3( &p1, f1.vx, f1.vy, f1.vz ) ; p1.vw = ( int )( f2.vy - f1.vy ) ;
			GV_SetVec3( &p2, f2.vx, f1.vy, f2.vz ) ; p2.vw = p1.vw ;
			g->special1 = HZX_AddDynamicSegment( hzx_id, &p1, &p2, 
												 HZX_SEG_ATR_ALL & ~HZX_SEG_NO_KNOCK_SE ) ;
			/* 秘所範囲 */
			GCL_GetNextIV( ( int * )&iv ) ;
			GV_IVtoFV( &iv, &f1, 3 ) ;			
			GCL_GetNextIV( ( int * )&iv ) ;
			GV_IVtoFV( &iv, &f2, 3 ) ;			
			f1.vz -= 80.0F ;
			f2.vz -= 80.0F ;
			_sceVu0AddVector( &f1, &f1, pos ) ;
			_sceVu0AddVector( &f2, &f2, pos ) ;
			GV_SetVec3( &p1, f1.vx, f1.vy, f1.vz ) ; p1.vw = ( int )( f2.vy - f1.vy ) ;
			GV_SetVec3( &p2, f2.vx, f1.vy, f2.vz ) ; p2.vw = p1.vw ;
			g->special2 = HZX_AddDynamicSegment( hzx_id, &p1, &p2, 
												 HZX_SEG_ATR_ALL & ~HZX_SEG_NO_KNOCK_SE ) ;
			RotateSegment2( this ) ;
		}
	} else {
		this->gravure = NULL ;
	}

	/* 箱がある */
	if ( GCL_GetOption( 'b' ) != NULL ) {
		DG_OBJS		*box ;
		DG_DEF		*def ;
		FVECTOR		shift = { -450.0F, 0.0F, -250.0F } ;
		IVECTOR		flr[ 4 ], pole ;

		def = ( DG_DEF * )GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) ) ;
		ASSERT( def != NULL ) ;
		box = this->box = DG_MakeObjs( def, DG_FLAG_PAINT | DG_FLAG_ONEPIECE, 0 ) ;
		ASSERT( box != NULL ) ;
		DG_SetPos( &this->body.objs->world ) ;
		DG_MovePos( &shift ) ;
		DG_GetPos( &box->world ) ;
		DG_MakePreshade( box, GM_GetMap( where )->light ) ;
		GM_GroupObjs( box, where ) ;
		DG_QueueObjs( box ) ;

		/* 壁床 */
		GV_FVtoIV( &pos, &pole, 3 ) ;
		max.vx = box->def->models[ 0 ].ux ;
		max.vy = box->def->models[ 0 ].uy ;
		max.vz = box->def->models[ 0 ].uz ;
		min.vx = box->def->models[ 0 ].lx ;
		min.vy = box->def->models[ 0 ].ly ;
		min.vz = box->def->models[ 0 ].lz ;
		DG_SetPos( &box->world ) ;
		DG_PutVector( &max, &max, 1 ) ;
		DG_PutVector( &min, &min, 1 ) ;
		SETVEC( &p[ 0 ], min.vx, min.vy, min.vz ) ;
		SETVEC( &p[ 1 ], min.vx, min.vy, max.vz ) ;
		p[ 0 ].vw = p[ 1 ].vw = max.vy - min.vy ;
		this->b_seg1 = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 0 ) ;
		SETVEC( &p[ 0 ], max.vx, min.vy, min.vz ) ;
		SETVEC( &p[ 1 ], max.vx, min.vy, max.vz ) ;
		this->b_seg2 = HZX_AddDynamicSegment( hzx_id, &p[ 0 ], &p[ 1 ], 0 ) ;
		min.vy += 200.0F ;
		SETVEC( &flr[ 0 ], min.vx, min.vy, min.vz ) ;
		SETVEC( &flr[ 1 ], min.vx, min.vy, max.vz ) ;
		SETVEC( &flr[ 2 ], max.vx, min.vy, max.vz ) ;
		SETVEC( &flr[ 3 ], max.vx, min.vy, min.vz ) ;
		this->b_flr = HZX_AddDynamicFloor( hzx_id, &flr[ 0 ], &flr[ 1 ], 
										   &flr[ 2 ], &flr[ 3 ], 4, 0 ) ;
	}

	/* 倒れ時床音番号 */
	this->sefloor = GCL_GetOptionValue( 'F', 0 ) ;

    N_Lockers ++ ;
    return ( void * )this ;
}

/*------------------------------------------------------------------*/

/* 検索 */
static 	LOCKER2	*SearchLocker2( int name )
{
    int			i ;
	LOCKER2		*this ;
    for ( i = 0; i < N_Lockers; i ++ ) {
		this = ( LOCKER2 * )( Lockers + i ) ;
		if ( this->name == name ) return this ;
    }    
    return NULL ;
}

/*----------------------------------------------------------------*/

/* プレイヤー行動 */

static	int		HeartBeatTime ;

static	void	LockerOutOpen2( PlayerWork *, int ) ;
static	void	LockerOutClose2( PlayerWork *, int ) ;
static	void	LockerInClose2( PlayerWork *, int ) ;
static	void	LockerInside2( PlayerWork *, int ) ;
static	void	LockerInOpen2( PlayerWork *, int ) ;
static	void	LockerOpenedbyEnemy2( PlayerWork *, int ) ;
static	void	LockerNotOpen2( PlayerWork *, int ) ;
static	void	LockerMoveInside2( PlayerWork *work, int time ) ;

/* 終了 */
static	void	EndLockerAct2( work ) 
PlayerWork		*work ;
{
	SetFlag( FLAG_FORCE_END ) ;
	PL_Force->e_turn = -1 ;
	PL_Force->flag = 0 ;
	UnsetStatus( PLAYER_LOCKER | PLAYER_FORCE ) ;
	work->control.skip_flag &= ~( CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_SEG_CHECK |
								  CTRL_SKIP_FLR_CHECK ) ;
	PL_UnsetInvincible( work ) ;
	work->stance = STAND ;
	SetMode( work, PL_StillMode[ 0 ] ) ;
}

/* ロッカー外から開ける */
static	void	LockerOutOpen2( work, time )
PlayerWork	*work ;
int		time ;
{
    int		ftime ;
    FVECTOR	mov ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		if ( work->data == 0 ) {
			SetAction( work, Mopen_out, 6 ) ;
			work->control.turn.vy = Locker.dir ;
			work->control.rot.vy = Locker.dir ;
		} else {
			SetAction( work, Mopen_out_behind, 6 ) ;
			work->control.turn.vy = Locker.dir + 2048 ;
			work->control.rot.vy = Locker.dir + 2048 ;
		}
		work->data = 0 ;
		mov.vx = Locker.mov_x ;
		mov.vz = Locker.mov_z ;
		mov.vy = work->control.mov.vy ;
		PL_AdjustXZFromTo( work, &work->control.mov, &mov ) ;
		GM_ResetControlPosition( &work->control, &mov ) ;

		if ( This->status & LOCKER_STATE_CORPSE_IN ) {
			work->data = 1 ;
		}
    }

	/* 敵吹っ飛ばし */
	SetBlowTarget( This, ENEMY_SIDE ) ;

    switch( work->data ) {
    case 0 :
		if ( EndMotion( work ) ) EndLockerAct2( work ) ;
		break ;
    case 1 :
//		if ( PL_CheckMotionTime( work, 48 ) ) {
	    if ( work->data2 == 0 && PL_CheckMotionRate( work ) > 0.80F ) {
			SetAction( work, Msurprise, 6 ) ;	    
			work->data2 = 1 ;
		}
		if ( EndMotion( work ) && work->motion1 == Msurprise ) {
			EndLockerAct2( work ) ;
		}
    }
}

/* ロッカー外から閉める */
static	void	LockerOutClose2( work, time )
PlayerWork	*work ;
int		time ;
{
    int		ftime ;
    FVECTOR	mov ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		SetAction( work, Mclose_out, 6 ) ;
		mov.vx = Locker.mov_x ;
		mov.vz = Locker.mov_z ;
		mov.vy = work->control.mov.vy ;
		PL_AdjustXZFromTo( work, &work->control.mov, &mov ) ;
		GM_ResetControlPosition( &work->control, &mov ) ;
		work->control.turn.vy = Locker.dir ;
		work->control.rot.vy = Locker.dir ;
    }

	/* 敵吹っ飛ばし */
	SetBlowTarget( This, ENEMY_SIDE ) ;

    if ( EndMotion( work ) ) {
		EndLockerAct2( work ) ;
    } 
}

/* ロッカー内から閉める */
static	void	LockerInClose2( work, time )
PlayerWork	*work ;
int		time ;
{
    int		ftime ;
    FVECTOR	mov ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE | FLAG_NO_IK ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		SetAction( work, Mclose_in, 0 ) ; /* 補完なし */
		mov.vx = Locker.mov_x ;
		mov.vz = Locker.mov_z ;
		mov.vy = work->control.mov.vy ;
		PL_AdjustXZFromTo( work, &work->control.mov, &mov ) ;
		GM_ResetControlPosition( &work->control, &mov ) ;
		PL_LeaveSubject( work ) ;
		PL_LeaveCaution( work ) ; /* 張り付き状態解除 */
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ;
    }

	/* 敵吹っ飛ばし */
	SetBlowTarget( This, ENEMY_SIDE ) ;

    PL_LevelCheck( work ) ;
    if ( ftime > 52 && 
		 !( This->flag & LOCKER_FLAG_TOILET ) ) DG_InvisibleObjs( work->body.objs ) ;
    if ( EndMotion( work ) ) {
		SetMode( work, LockerInside2 ) ;
		//UnsetStatus( PLAYER_FORCE ) ;
    }
}

/* ポスターチェック */
static	int		CheckPoster( LOCKER2 *this, FVECTOR *pos, SVECTOR *rot, int ry ) 
{
	FVECTOR		to = { 0.0F, 0.0F, 1000.0F } ;
	FVECTOR		p1, p2 ;
	HZX_HZD		seg ;
	GRAVURE_SET	*g ;

	if ( !( this->status & LOCKER_STATE_POSTER ) ) return 0 ;
	if ( GV_DiffDirAbs( this->def_rot, ry ) > 480 ) return 0 ;
	g = this->gravure ;
	DG_SetPos( &this->body.objs->world ) ;
	DG_PutVector( &g->kissb1, &p1, 1 ) ;
	DG_PutVector( &g->kissb2, &p2, 1 ) ;

	seg.attribute = 0 ;
	seg.ptr = NULL ;
	seg.type = HZX_TYPE_SEGMENT ;
	seg.p1.x = p1.vx ;
	seg.p1.y = p1.vy ;
	seg.p1.z = p1.vz ;
	seg.p1.h = p2.vy - p1.vy ;
	seg.p2.x = p2.vx ;
	seg.p2.y = p1.vy ;
	seg.p2.z = p2.vz ;
	seg.p2.h = p2.vy - p1.vy ;

	DG_SetPos2( pos, rot ) ;
	DG_PutVector( &to, &to, 1 ) ;

//HZX_ViewSegment( &seg ) ;
	if ( HZX_OnlineHazardCheckOneSegment( &seg, pos, &to ) ) {
		return 1 ;
	}
	return 0 ;
}

/* ロッカーの中 */
static	void	LockerInside2( work, time )
PlayerWork	*work ;
int		time ;
{
	LOCKER2		*l ;
	int			cvy ;

   float       lockerZoomSpeed = 16.0f;   //BP_INPUT - used by X360 controls
   float       touchPressure = 0.0f;

#if defined(BP_VITA)
   float l_pressure = 0.0f;
   float r_pressure = 0.0f;
#endif

	l = This ;
    SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
	SetFlag( FLAG_RECOVER_ENABLE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE | PLAYER_DARK_AREA | PLAYER_LOCKER |
			   PLAYER_NARROW ) ;
    if ( !( l->flag & LOCKER_FLAG_TOILET ) ) DG_InvisibleObjs( work->body.objs ) ; 
    if ( time == 0 ) {
		work->stance = STAND ;
//		GM_SetNoise( NOISE_S, &work->control.mov, l->body.map_name ) ; /* 扉閉まりノイズ */
		SetAction( work, Min_still, 0 ) ; /* 補完なし */
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		PL_IntoSubject( work ) ;
		work->sv.vx = 0 ;
		work->sv.vy = 0 ;
		HeartBeatTime = 0 ;
		work->sv.vz = 0 ;
		work->idata = 0 ;
		UnsetStatus( PLAYER_FORCE ) ;
    }

	/* 立つ、しゃがむ */
	if ( work->pad->press & PL_PAD_SQUAT ) {
		work->stance = ( work->stance == STAND ) ? SQUAT : STAND ;
		GM_SeSetMode( SD_P_STAND02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
	}
	/* コンコン */
	if ( work->pad->press & PL_PAD_PUNCH ) {
		if ( l->flag & LOCKER_FLAG_TOILET ) {
			GM_SeSetMode( SD_A_TOILHIT1, &work->control.mov, GM_SEMODE_NORMAL ) ;
		} else {
			GM_SeSetMode( SD_A_LOCKHIT1, &work->control.mov, GM_SEMODE_NORMAL ) ;
		}
		GM_SetNoise( NOISE_S, &work->control.mov, l->body.map_name ) ;
		SETARG( 0, 1 ) ;
		SETARG( 1, work->control.name ) ;
		SETARG( 2, 6 ) ;
		ExecProc2( l, 3 ) ;
	}

    if ( Status( PLAYER_WATCH ) ) {
		FVECTOR		center = { -450.0F, 0.0F, -450.0F } ;
		FVECTOR		cam_start, cam_aim ;
		SVECTOR		cr ;
		GM_CameraSet	*now ;
		FVECTOR	forward = { 0.0F, 0.0F, 296.0F }, now_pos ;
		float	len = 0.0F ;
      
		SetFlag( FLAG_CANNOT_PEEP | FLAG_SUBJECT_HORIZON_LIMIT ) ;
		GM_SubjectHMaxTmp[ 0 ] = 320 ;
		GM_SubjectHMaxTmp[ 1 ] = -320 ;

		/* 主観カメラ位置固定 */
		DG_SetPos( &l->body.objs->world ) ;
		DG_PutVector( &center, &work->camera, 1 ) ;
		if ( l->flag & LOCKER_FLAG_TOILET ) {
			cvy = ( work->stance == STAND ) ? 1580.0F : 150.0F ;
			GM_SubjectVMaxTmp[ 0 ] = ( work->stance == STAND ) ? 640 : 64 ;
		} else {
			cvy = ( work->stance == STAND ) ? 1580.0F : 1000.0F ;
			GM_SubjectVMaxTmp[ 0 ] = 640 ;
			GM_SubjectVMaxTmp[ 1 ] = -640 ;
		}
		work->camera.vy = work->control.levels[ 0 ] + cvy ;
		DG_COPY_VEC( &cam_start, &work->camera ) ; 
		DG_COPY_VEC( &cam_aim, &work->camera ) ; 
		if ( work->sv.vx == 0 ) {
			DG_COPY_VEC( &work->fv, &cam_start ) ;
			work->sv.vx = 1 ;
		}

#if defined(BP_VITA)
      touchPressure = GestureGetRightDragAmount(kGesture_TouchScreen_Front, PL_PadGetGestureRelease(work), 0.0f, 10.0f);

      work->pad->status |= GestureGetTriggerButtonMask(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work));
      l_pressure = GestureGetLeftDragAmount(kGesture_TouchScreen_Back, 0.0f, 1.0f, PL_PadGetGestureRelease(work));
      r_pressure = GestureGetRightDragAmount(kGesture_TouchScreen_Back, 0.0f, 1.0f, PL_PadGetGestureRelease(work));
      if (touchPressure > 0.0f) {
#else
      if ( work->pad->status & PL_PAD_LOCKER_ZOOM) {
#endif
		/* 主観ボタンで、ぐっと寄る */
			if ( work->pad->status & ( PAD_L2 | PAD_R2 ) ) {
				/* 覗き込み中 */
				GV_SetVec3( &cr, 0, work->control.rot.vy, 0 ) ;
			} else {
				GV_SetVec3( &cr, work->camdir.vx, work->camdir.vy, 0 ) ;
			}
			DG_SetPos2( &work->camera, &cr ) ;
			DG_PutVector( &forward, &cam_aim, 1 ) ;
			/* ゆっくり寄りならノイズなし */
			if ( work->idata == 0 ) 
         {
            //BP_INPUT - use new X360 controller logic?
            if( PlayerPad.enable )
            {
#if !defined(BP_VITA)
               // Press RB = very slow movement
               // Press RB and < 50% [Right stick] upwards deflection = faster movement
               // Press RB and > 50% [Right stick] upwards deflection = very faster movement
               if( work->pad->status & PL_PAD_PRESS_LOCKER )
               {
                  // Which speed?
                  const float stick = ((255.0f - work->pad->right_dy) - 128.0f)/128.0f; // +1 = up, 0=middle, -1=down
                  if( stick >= PL_PAD_ZOOM_LOCKER_FAST_TH )
                  {
                     // very fast
                     lockerZoomSpeed = PL_ZOOM_LOCKER_FAST_SPEED;
                     work->data2 = 1;
                  }
                  else if( stick >= PL_PAD_ZOOM_LOCKER_MEDIUM_TH )
                  {
                     // medium
                     lockerZoomSpeed = PL_ZOOM_LOCKER_MEDIUM_SPEED;
                  }
                  else
                  {
                     // slow
                     lockerZoomSpeed = PL_ZOOM_LOCKER_SLOW_SPEED;
                  }
               }
#else
               if (touchPressure > 9.9f)
               {
                  lockerZoomSpeed = PL_ZOOM_LOCKER_FAST_SPEED;
                  work->data2 = 1;
               }
               else
               {
                  lockerZoomSpeed = PL_ZOOM_LOCKER_MEDIUM_SPEED;
               }
#endif
            }
            else
            //BP_INPUT - use new X360 controller logic?
            {
				   if ( work->pad->pressure[ PL_PAD_PRESS_LOCKER ] > 128 ) work->data2 = 1 ;
            }
			}
         //BP_INPUT - variable zoom speed for X360 controls
         if( PlayerPad.enable )
         {
            GV_NearExpNVF( &work->fv, &cam_aim, lockerZoomSpeed, 3 ) ;  
         }
         else
         //BP_INPUT - variable zoom speed for X360 controls
         {
			   if ( work->data2 == 0 && work->idata == 0 ) 
            {
				   GV_NearExp16VF( &work->fv, &cam_aim, 3 ) ;   // medium
			   } 
            else 
            {
				   DG_COPY_VEC( &work->fv, &cam_aim ) ;   // fast
			   }
         }
			DG_COPY_VEC( &work->camera, &work->fv ) ;
		} else {
			if ( work->data > 0 ) {
				work->camdir.vx = 0 ;
				work->camdir.vy = work->control.rot.vy ;
			}
			DG_COPY_VEC( &work->fv, &work->camera ) ;
		}

		if ( work->pad->status & ( PAD_R2 | PAD_L2 ) ) {
			GV_PAD	*pad ;
			FVECTOR	right = { 96.0F, 0.0F, 24.0F } ;
			FVECTOR	left = { -96.0F, 0.0F, 24.0F } ;
			FVECTOR	up = { 0.0F, 64.0F, 0.0F } ;
			FVECTOR	aim, diff ;
			short	turn_diff ;
			float	interp ;
			int		p ;
			/* ロッカー特殊覗き込み */
			if ( work->stance == STAND && ( l->flag & LOCKER_FLAG_TOILET ) ) up.vy = 780.0F ;
			work->camdir.vx = 0 ;
			pad = work->pad ;
			DG_SetPos2( &cam_aim, &work->control.rot ) ;
			switch ( pad->status & ( PAD_R2 | PAD_L2 ) ) {
			case PAD_R2 :
				DG_PutVector( &right, &aim, 1 ) ;

#if defined(BP_VITA)
            interp = r_pressure;
#else
				interp = ( float )pad->pressure[ PAD_PRESS_R2 ] / 255.0F ;	
#endif
				turn_diff = -400 ;
				break ;
			case PAD_L2 :
				DG_PutVector( &left, &aim, 1 ) ;

#if defined(BP_VITA)
				interp = l_pressure;
#else
            interp = ( float )pad->pressure[ PAD_PRESS_L2 ] / 255.0F ;
#endif
				turn_diff = 400 ;
				break ;
			default :
				DG_PutVector( &up, &aim, 1 ) ;
#if defined(BP_VITA)
            if (r_pressure < l_pressure)
            {
               interp = l_pressure;
            }
            else
            {
               interp = r_pressure;
            }
#else
				p = pad->pressure[ PAD_PRESS_R2 ] ;
				if ( p < pad->pressure[ PAD_PRESS_L2 ] ) p = pad->pressure[ PAD_PRESS_L2 ] ;
            interp = ( float )p / 255.0F ;
#endif
				turn_diff = 0 ;
				work->camdir.vx = ( short )( 240.0F * interp ) ;
			}
			_sceVu0SubVector( &diff, &aim, &cam_aim ) ;
			_sceVu0ScaleVector( &diff, &diff, interp ) ;
			_sceVu0AddVector( &work->camera, &work->camera, &diff ) ;
			work->camdir.vy = work->control.rot.vy + ( short )( ( float )turn_diff * interp ) ;
			work->data = 1 ;
		} else {
			/* つながりロッカー処理 */
			if ( !( work->pad->status & PL_PAD_LOCKER_ZOOM ) ) {
				if ( ( work->pad->press & PAD_L ) && 
					( Next = SearchConnectLocker( This->name, LCKCNCT_L ) ) ) {
					SetMode( work, LockerMoveInside2 ) ;
					work->data = LCKCNCT_L ;
					return ;
				} else if ( ( work->pad->press & PAD_R ) && 
						   ( Next = SearchConnectLocker( This->name, LCKCNCT_R ) ) ) {
					SetMode( work, LockerMoveInside2 ) ;
					work->data = LCKCNCT_R ;
					return ;
				}
			}
			/* 主観回転 */
			if ( work->data == 1 ) {
				work->camdir.vx = GM_CameraDir.vx = 0 ;
				work->camdir.vy = work->control.rot.vy ;
				work->data = 0 ;
			}
			if ( work->pad->status & PL_PAD_LOCKER_ZOOM ) {
				GM_SubjectVStepTmp = 4 ;
				GM_SubjectHStepTmp = 8 ;
			}
			PL_SubjectTurn( work ) ;
		}
		
		{
#if defined(BP_VITA)
         if (touchPressure > 0.0f) {
#else
			if ( work->pad->status & PL_PAD_LOCKER_ZOOM ) {
#endif
				GV_SetVec3( &cr, work->camdir.vx, work->camdir.vy, 0 ) ;
				now = GM_GetCurrentCamera( work->chanl ) ;
				DG_COPY_VEC( &now_pos, &now->position ) ;
				len = GV_VecLen3F2( &now_pos, &cam_start ) ;
				if ( work->idata == 0 && len > 260.0F ) {
					work->idata = 1 ;
					if ( CheckPoster( l, &work->camera, &cr, work->camdir.vy ) ) {
						if ( work->camdir.vx < 640 ) {
							GM_SeSetMode( SD_V_KISS_GO, &work->control.mov, GM_SEMODE_NORMAL ) ;
                     BP_TrophySystem_UnlockTrophy( kTRP_KissPoster );
						}
					} else {
						if ( work->camdir.vx < 640 &&
							!( ( l->flag & LOCKER_FLAG_TOILET ) && work->stance == SQUAT ) ) {
							/* あんまり下向きのときは音ならない */
							/* トイレしゃがみも */
							if ( work->data2 == 1 ) {
								if ( l->flag & LOCKER_FLAG_TOILET ) {
									GM_SeSetMode( SD_A_TOILHIT1, &work->control.mov, 
												  GM_SEMODE_NORMAL ) ;
								} else {
									GM_SeSetMode( SD_A_LOCKHIT1, &work->control.mov, 
												  GM_SEMODE_NORMAL ) ;
								}
								GM_SetNoise( NOISE_S, &work->control.mov, l->body.map_name ) ;
							}
						}
					}
				}			
			} else {
				GV_SetVec3( &cr, work->camdir.vx, work->camdir.vy, 0 ) ;
				if ( work->idata != 0 && 
					CheckPoster( l, &work->camera, &cr, work->camdir.vy ) ) {
					if ( work->camdir.vx < 640 ) {
						GM_SeSetMode( SD_V_KISS_FIN, &work->control.mov, GM_SEMODE_NORMAL ) ;	
					}
				}				
				work->idata = 0 ;
				work->data2 = 0 ;	/* 寄りノイズ判定用 */
			}
		}			

    }	

	/* 心音 */
	{
		static u_char 	HeartBeatVib[] = { 176, 4, 0, 0 } ;
		int			  	decay ;
		float		  	len ;
		
		/* 振動センサー装備時はやらない */
		if ( work->item != IT_VibSensor ) {
			len = PL_NearestEnemyLen() ;
			if ( len > 4000.0F ) {
				if ( GM_AlertMode == ALERT_MODE_SNEAK ) decay = -1 ;
				else									decay = 300 / TIME_BASE ;
			} else {
				decay = 300 / TIME_BASE 
					+ ( int )( ( len - 4000.0F ) * ( float )( 300 / TIME_BASE / 2 ) / 3000.0F ) ;
				if ( decay < 300 / TIME_BASE / 2 ) decay = 300 / TIME_BASE / 2 ;
			}
			if ( decay > 0 && GV_Time - HeartBeatTime > decay ) {
				HeartBeatVib[ 0 ] = 176 + 300 / TIME_BASE - decay ;
				GM_SeSetMode( SD_P_HEARTB01, &work->control.mov, GM_SEMODE_BOMB ) ;
				HeartBeatTime = GV_Time ;
				work->sv.vz = 2 ;
			}
			if ( work->sv.vz > 0 ) {
				if ( -- work->sv.vz <= 0 ) {
					NewPadVibration( HeartBeatVib, 2 ) ;
					work->sv.vz = 0 ;
				}
			}
		}
	}

	SetFlag( FLAG_FINDPOS_IS_WAIST ) ;
	GM_PlayerFindPos.vy = work->camera.vy ;

	if ( ( l->flag & LOCKER_FLAG_TOILET ) && work->stance == STAND ) 
   {
#if defined(BP_VITA)
      int l = (int)(l_pressure * 255.0f);
      int r = (int)(r_pressure * 255.0f);
#endif
		GM_PlayerFindPos.vy = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] + 250.0F ;

#if !defined(BP_VITA)
		if (( work->pad->pressure[ PAD_PRESS_L2 ] > 0 ) && ( work->pad->pressure[ PAD_PRESS_R2 ] > 0 ) ) 
#else
      if (l > 0 && r > 0) 
#endif
      {
#if !defined(BP_VITA)
			if (( work->pad->pressure[ PAD_PRESS_L2 ] > 140 ) || ( work->pad->pressure[ PAD_PRESS_R2 ] > 140 ) ) 
#else
         if (l > 140 || r > 140)
#endif
         {
				if ( GM_GameStatus & STATE_CLEARING ) 
            {
					GM_PlayerFindPos.vy =
						work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] + 750.0F ;
				} else 
            {
					GM_PlayerFindPos.vy =
						work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] + 750.0F ;
				}
			}
		}
	}

	if ( ( work->pad->press & PAD_LOCKER ) &&
		 ( l->status & LOCKER_STATE_CLOSE ) &&
		 l->msg_buf[ 5 ] == 0 ) {
		SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
		//PL_LeaveSubject( work ) ;
		PL_SetInvincible( work, 0 ) ;
		SetStatus( PLAYER_FORCE ) ;
		SetMode( work, LockerInOpen2 ) ;
		SetMsgBuf( l, LOCKER_MSG_OPEN, work->control.name,
				   1, DMopen_in, 0 ) ;
		return ;
    }
	/* 開けられた、壊された */
	if ( !( This->r_intrpt.status & ROOT_INTRPT_CLOSE ) ) {
		SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
		SetMode( work, LockerOpenedbyEnemy2 ) ;
		return ;
	}
}

/* ロッカー中から開ける＆外へでる */
static	void	LockerInOpen2( work, time )
PlayerWork	*work ;
int		time ;
{
    int			ftime ;
	FVECTOR		mov ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE | FLAG_NO_IK ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		work->stance = STAND ;
		mov.vx = Locker.mov_x ;
		mov.vz = Locker.mov_z ;
		mov.vy = work->control.mov.vy ;
		PL_AdjustXZFromTo( work, &work->control.mov, &mov ) ;
		GM_ResetControlPosition( &work->control, &mov ) ;
		SetAction( work, Mopen_in, 0 ) ; /* 補完なし */
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ;
		This->status |= LOCKER_STATE_DONOT_OFF_SEGMENT4 ;
		PL_LeaveSubject( work ) ;
    }  

	/* 敵吹っ飛ばし */
	SetBlowTarget( This, ENEMY_SIDE ) ;

    PL_LevelCheck( work ) ;

    switch( work->data ) {
    case 0 :
		if ( !( This->flag & LOCKER_FLAG_TOILET ) ) {
			if ( ftime < 7 ) DG_InvisibleObjs( work->body.objs ) ;
			else			 DG_VisibleObjs( work->body.objs ) ;
		} else {
			if ( ftime < 3 ) DG_InvisibleObjs( work->body.objs ) ;
			else			 DG_VisibleObjs( work->body.objs ) ;
		}
#if 0
		if ( ftime < 7 && !( This->flag & LOCKER_FLAG_TOILET ) ) DG_InvisibleObjs( work->body.objs ) ;
		else			 DG_VisibleObjs( work->body.objs ) ;
#endif
		if ( EndMotion( work ) ) {
			SetAction( work, Mclose_out_turn, 0 ) ;
			SetMsgBuf( This, LOCKER_MSG_CLOSE, work->control.name,	
				   0, DMclose_out_turn, 0 ) ;
			work->control.turn.vy += 2048 ;
			work->control.rot.vy = work->control.turn.vy ;			
			work->data ++ ;
		}
		break ;
    case 1 :
		if ( EndMotion( work ) ) {
			EndLockerAct2( work ) ;
			SetFlag( FLAG_NO_WAIST_INTERP | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
		}	
    }
}

/* 敵に開けられた */
static	void	LockerOpenedbyEnemy2( work, time )
PlayerWork		*work ;
int				time ;
{
    int			ftime ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
    if ( time == 0 ) {
		work->stance = STAND ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveSubject( work ) ;
    }	
	if ( This->r_intrpt.status & ROOT_INTRPT_OPEN ) {
		PL_UnsetInvincible( work ) ;
		DG_VisibleObjs( work->body.objs ) ;
		EndLockerAct2( work ) ;
    }	
}

/* ロッカーあかない */
static	void	LockerNotOpen2( work, time )
PlayerWork		*work ;
int				time ;
{
    int		ftime ;
    FVECTOR	mov ;

    ftime = work->ftime ;
    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    if ( time == 0 ) {
		SetAction( work, Mnot_open, 6 ) ;
		mov.vx = Locker.mov_x ;
		mov.vz = Locker.mov_z ;
		mov.vy = work->control.mov.vy ;
		PL_AdjustXZFromTo( work, &work->control.mov, &mov ) ;
		GM_ResetControlPosition( &work->control, &mov ) ;
		work->control.turn.vy = Locker.dir ;
		work->control.rot.vy = Locker.dir ;
    }
	if ( EndMotion( work ) || ( work->pad->press & PAD_B ) ) EndLockerAct2( work ) ;
}

/* ロッカーの中を移動 */
static	void	LockerMoveInside2( PlayerWork *work, int time )
{
	LOCKER2		*l, *n ;
	FVECTOR		from, to ;

	l = This ;
	n = Next ;
    SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE | PLAYER_DARK_AREA | PLAYER_LOCKER |
			   PLAYER_NARROW ) ;
    if ( !( l->flag & LOCKER_FLAG_TOILET ) ) DG_InvisibleObjs( work->body.objs ) ; 
    if ( time == 0 ) {
		work->stance = STAND ;
		work->camdir.vx = 0 ;
		if ( work->data == LCKCNCT_R ) {
			SetAction( work, Mmove_r, 6 ) ; 
		} else {
			SetAction( work, Mmove_l, 6 ) ; 
		}
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
		PL_IntoSubject( work ) ;
    }
	DG_SetPos( &n->body.objs->world ) ;
	DG_PutVector( &Shift[ 1 ], &to, 1 ) ;
	work->camera.vy = work->control.levels[ 0 ] + 1580.0F ;
	DG_COPY_VEC( &from, &work->control.mov ) ;
	to.vy = from.vy ;

	if ( ( GV_VecLen3F2( &from, &to ) < 128.0F ) || time > 300 * 3 ) {
		GM_ResetControlPosition( &work->control, &to ) ;	
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		This = Next ;
		Locker.mov_x = to.vx ;
		Locker.mov_z = to.vz ;
		SetAction( work, Min_still, 6 ) ; 
		SetMode( work, LockerInside2 ) ;
		return ;
	}

	/* 開けられた、壊された */
	if ( !( This->r_intrpt.status & ROOT_INTRPT_CLOSE ) ) {
		GM_ResetControlPosition( &work->control, &to ) ;	
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		This = Next ;
		Locker.mov_x = to.vx ;
		Locker.mov_z = to.vz ;
		SetAction( work, Min_still, 6 ) ; 
		SetMode( work, LockerInside2 ) ;
	} else if ( !( Next->r_intrpt.status & ROOT_INTRPT_CLOSE ) ) {
		GM_ResetControlPosition( &work->control, &to ) ;	
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		This = Next ;
		Locker.mov_x = to.vx ;
		Locker.mov_z = to.vz ;
		SetAction( work, Min_still, 6 ) ; 
		SetMode( work, LockerOpenedbyEnemy2 ) ;
	}
}

/*----------------------------------------------------------------*/

static	int	CheckLockerActEnable( PlayerWork *work, GV_MSG *msg, int len,
								  int dir, int range, int status, int reserved ) 
{
    /* 以下の時は無効 */
    if ( Status( PLAYER_SQUAT | PLAYER_GROUND | PLAYER_DAMAGED | PLAYER_DOWNED |
				PLAYER_INTRUDE | PLAYER_BEYOND | PLAYER_CB_BOX |
				PLAYER_DEAD | PLAYER_LOCKER ) || 
		( Status( PLAYER_FORCE ) && !Flag( FLAG_SET_FORCE_NOW ) ) ||
		( WeaponType( work ) & ( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ) ||
		( ItemType( work ) & ( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ) ) return -1 ;
	if ( Flag( FLAG_BEHIND_PEEP | FLAG_BEHIND_ATTACK ) ) return -1 ;
	/* 誰かをつかんでいる */
	if ( work->capture.capture != NULL ) return -1 ;
    /* 状態チェック */
    if ( !( status & ( LOCKER_STATE_OPEN | LOCKER_STATE_CLOSE ) ) ) return -1 ;
	if ( status & LOCKER_STATE_BROKEN ) return -1 ;
	if ( reserved != 0 ) return -1 ;
	/* 閉めるとき */
	/* ロッカーゾーン内に敵がいないかどうかチェック */
	if ( ( status & LOCKER_STATE_OPEN ) && 
		 ( CheckEnemyInZone( This ) || CheckBomb( This ) ) ) return -1 ;
    /* 方向チェック */
    if ( GV_DiffDirAbs( work->control.rot.vy, dir ) > range ||
		 GV_DiffDirAbs( work->control.turn.vy, dir ) > range ) return -1 ;
	
    if ( Status( PLAYER_CAUTION ) ) {
		if ( status & LOCKER_STATE_CLOSE ) {
			/* 張付きから開ける */
			if ( !( work->pad->press & PAD_LOCKER ) ) return -1 ;
		}
    }
	return 1 ;
}


/* ロッカーのworldをDG_SetPosしておく必要があります */
static inline void LockerPosPlayerAdustment( void )
{
	FVECTOR adj ;

	if ( !GM_CheckPlayerStatus(PLAYER_SNAKE) &&
		 GM_Configuration & GM_CONFIG_STORY_TANKER ) {
		GV_SetVec3( &adj, 0.0F, 0.0F, -65.0F ) ;
		DG_RotVector( &adj, &adj, 1 ) ;
		Locker.mov_x += adj.vx ;
		Locker.mov_z += adj.vz ;
	}
}



/* ロッカーモードセット（扉のみ用） */
static	int	SetLockerAct2( PlayerWork *work,
						   GV_MSG	  *msg,
						   int		   len )
{
    int			dir, range, status ;
	int			cau = 0, reserved ;
    FVECTOR		mov, adj ;
    SVECTOR		rot ;

	This = SearchLocker2( msg->message[ 1 ] ) ;
	status = This->status ;
	reserved = This->msg_buf[ 5 ] ;

    Locker.dir = dir = msg->message[ 3 ] ;
    Locker.range = range = msg->message[ 4 ] ;
    if ( Status( PLAYER_CAUTION ) ) dir += 2048 ;

	if ( CheckLockerActEnable( work, msg, len, dir, range, status, reserved ) <= 0 ) return -1 ;
	if ( PL_PluginPreCheck ) return 1 ;

    rot.vx = rot.vz = 0 ;
    rot.vy = dir & 4095 ;
    DG_SetPos( &This->body.objs->world ) ;

    if ( Status( PLAYER_CAUTION ) ) {
		if ( status & LOCKER_STATE_OPEN ) {
			DG_PutVector( &Shift[ 1 ], &mov, 1 ) ;
			Locker.mov_x = mov.vx ;
			Locker.mov_z = mov.vz ;
			SetMode( work, LockerInClose2 ) ;
			SetMsgBuf( This, LOCKER_MSG_CLOSE, work->control.name,
					  1, DMclose_in, 0 ) ;
			work->control.turn.vy = work->control.rot.vy = dir ;
		} else if ( status & LOCKER_STATE_CLOSE ) {
			PL_LeaveCaution( work ) ;
			cau = 1 ;
			goto from_out ;
		}
    } else {
from_out :
	    DG_PutVector( &Shift[ 0 ], &mov, 1 ) ;
		Locker.mov_x = mov.vx ;
		Locker.mov_z = mov.vz ;
		if ( status & LOCKER_STATE_CLOSE ) {
			if ( status & LOCKER_STATE_DESTROY ) {
				GV_SetVec3( &adj, 0.0F, 0.0F, -20.0F ) ;
				DG_RotVector( &adj, &adj, 1 ) ;
				Locker.mov_x += adj.vx ;
				Locker.mov_z += adj.vz ;
				SetMode( work, LockerNotOpen2 ) ;
				SetMsgBuf( This, LOCKER_MSG_NOT_OPEN, work->control.name,
						   0, DMnot_open, 0 ) ;
			} else {
				SetMode( work, LockerOutOpen2 ) ;
				if ( cau ) {
					SetMsgBuf( This, LOCKER_MSG_OPEN, work->control.name,
							   0, DMopen_out_behind, 0 ) ;
					DG_PutVector( &Shift[ 2 ], &mov, 1 ) ;
					Locker.mov_x = mov.vx ;
					Locker.mov_z = mov.vz ;
					work->data = 1 ;
				} else {
					SetMsgBuf( This, LOCKER_MSG_OPEN, work->control.name,
							   0, DMopen_out, 0 ) ;
					work->data = 0 ;
				}
			}
		} else {
			SetMode( work, LockerOutClose2 ) ;
			SetMsgBuf( This, LOCKER_MSG_CLOSE, work->control.name,
					   0, DMclose_out, 0 ) ;
		}
		work->control.turn.vy = work->control.rot.vy = dir ;
    }

	LockerPosPlayerAdustment() ;

	PL_ClearCaptureTarget( work ) ;
    PL_SetInvincible( work, 0 ) ;
	PL_SetMode2( work, NULL ) ;
	PL_UnequipSpecials() ;
//    work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK ;
    SetFlag( FLAG_CANNOT_CHANGE ) ;
    SetStatus( PLAYER_FORCE ) ;
	/* モーションファイルチェンジ */
	PL_ChangeMotionArc( work, PlayerLockerMotion ) ;
    return 1 ;	
}

/*------------------------------------------------------------------*/

static	LOCKER2		*PutBodyLocker = NULL ;
static	int			PutMotion = 0, PutDMotion = 0 ;

static	FVECTOR		PutShift[] = {
	{ -443.0F, 0.0F, -177.0F },
	{ -443.0F, 0.0F, -208.0F },
	{ -443.0F, 0.0F, -208.0F }
} ;

/* 死体入れ */
static	void	PutBody( PlayerWork *work, int time )
{
	LOCKER2		*this ;
	FVECTOR		shift ;

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK |
			 FLAG_NO_GRAVITY | FLAG_CANNOT_ATTACK | FLAG_NO_TOUCH_DAMAGE |
			 FLAG_NO_CAUTION ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	GM_SetMenuStatus( MENU_MENU_OFF ) ;

	this = PutBodyLocker ;
	if ( time == 0 ) {
		DG_SetPos( &this->body.objs->world ) ;
		if ( PutMotion == Mbody_put ) {
			DG_COPY_VEC( &shift, &PutShift[ 0 ] ) ;
		} else {
			DG_COPY_VEC( &shift, &PutShift[ 1 ] ) ;
		}
		DG_PutVector( &shift, &shift, 1 ) ;
//		shift.vy = work->control.mov.vy + 200.0F ;
		shift.vy = this->center.vy + 200.0F + work->body.height ;
		PL_AdjustXZFromTo( work, &work->control.mov, &shift ) ;
		GM_ResetControlPosition( &work->control, &shift ) ;

		PL_ChangeMotionArc( work, PlayerLockerMotion ) ;
		PL_SetAction( work, PutMotion, 6 ) ;
		work->control.turn.vy = work->control.rot.vy = this->def_rot ;
		PL_LeaveSubject( work ) ;
		SetStatus( PLAYER_FORCE ) ;
		work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
		SETARG( 0, 0 ) ;
		SETARG( 1, work->control.name ) ;
		SETARG( 2, 5 ) ;
		ExecProc2( this, 3 ) ;
	}

	work->control.step.vy = 0.0F ;
	if ( EndMotion( work ) ) {
		PL_ReturnMotionArc( work ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		UnsetStatus( PLAYER_FORCE ) ;
		PL_UnsetInvincible( work ) ;
		this->status &= ~LOCKER_STATE_INVINCIBLE ;
		SETARG( 0, 1 ) ;
		SETARG( 1, work->control.name ) ;
		SETARG( 2, 5 ) ;
		ExecProc2( this, 3 ) ;
		SetMode( work, PL_StillMode[ STAND ] ) ;
		SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	}
}

/* ロッカーのどれかに死体を持ったまま
   入った？ */
static	LOCKER2	*IsInLockers( FVECTOR *mov, SVECTOR *rot, CONTROL *capture ) 
{
	int			i, dir ;
	LOCKER2		*this = NULL ;
	FVECTOR		dif = { 0.0F, 0.0F, 150.0F }, chk_pos ;
	SVECTOR		rt ;

	for ( i = 0; i < N_Lockers; i ++ ) {
		this = ( LOCKER2 * )( Lockers + i ) ;
		/* 開いてないとダメ */
		if ( ( this->status & LOCKER_STATE_BROKEN ) ||
			 !( this->status & LOCKER_STATE_OPEN ) ) continue ;
		/* 距離チェック */
		if ( DG_FABS( mov->vy - this->center.vy ) > 2000.0F ) continue ;

		GV_SetVec3( &rt, 0, this->def_rot, 0 ) ;
		DG_SetPos2( &this->center, &rt ) ;
		DG_PutVector( &dif, &chk_pos, 1 ) ;

//		printf( "%f %f\n", DG_FABS( mov->vx - chk_pos.vx ),
//			   DG_FABS( mov->vz - chk_pos.vz ) ) ;

		if ( DG_FABS( mov->vx - chk_pos.vx ) > 100.0F ||
			DG_FABS( mov->vz - chk_pos.vz ) > 100.0F ) continue ;
		/* 角度チェック */
		dir = GV_VecDir2FromTo( &this->center, mov ) ;
		if ( GV_DiffDirAbs( rot->vy, this->def_rot ) > 1024 ) continue ;
		if ( GV_DiffDirAbs( dir, this->def_rot ) > 256 ) continue ;
		/* 敵兵がいないかチェック */
		if ( CheckEnemyInZoneExceptThis( this, capture ) ) continue ;
		break ;
	}
	if ( i == N_Lockers ) return NULL ;
	return this ;
}

/* ポーリング関数 */
static	int	CheckPutBody( PlayerWork *work )
{
	LOCKER2		*this ;

	if ( Status( PLAYER_FORCE | PLAYER_DEAD | PLAYER_DAMAGED ) ||
		 !Status( PLAYER_ENEMY_PULL ) ) return 0 ;
	if ( work->capture.capture == NULL ||
		( work->capture.flag & CAPTURE_FREE ) ) return 0 ;
	/* ＮＰＣは入れられない */
	if ( work->capture.capture->ctrl->attribute & CTRL_ATR_NPC ) return 0 ;
	this = IsInLockers( &work->control.mov, &work->control.rot, work->capture.capture->ctrl ) ;
	if ( this == NULL ) return 0 ;

	PutBodyLocker = this ;

	this->status |= LOCKER_STATE_CORPSE_IN ;
	this->capture = work->capture.capture ;
	this->capture->flag |= CAPTURE_LOCKER ;

	PL_ClearCaptureTarget( work ) ;
	this->capture->flag &= ~CAPTURE_FREE ;
	PL_SetInvincible( work, 0 ) ;

	/* モーション決定 */
	if ( this->capture->flag & CAPTURE_HEAD ) {
		PutMotion = Mbody_put ;
		PutDMotion = DMput_body ;
	} else if ( this->capture->flag & CAPTURE_FRONT ) {
		PutMotion = Mleg_put_f ;
		PutDMotion = DMput_leg_f ;
	} else {
		PutMotion = Mleg_put_b ;
		PutDMotion = DMput_leg_b ;
	}

	this = This ;
	This = PutBodyLocker ;
	SetMsgBuf( PutBodyLocker, LOCKER_MSG_PUT_CORPSE, work->control.name,
			   0, PutDMotion, 0 ) ;
	This = this ;
	SetMode2( work, NULL ) ;
	SetMode( work, PutBody ) ;
	return 1 ;
}


/*------------------------------------------------------------------*/

/* モーション登録 */
int		NewSetLockerMotion( void ) 
{
	LockerMotion = GCL_GetOptionValue( 'm', 0 ) ;
	ASSERT( LockerMotion != 0 ) ;
	return 1 ;
}




