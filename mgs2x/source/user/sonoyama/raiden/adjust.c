/*
   adjust.c 
   ライデン／各種、位置角度補正コード
   めりこみ防止などもある
   
   2000/04/25 M.Sonoyama
   $Id: adjust.c,v 1.1.1.3 2002/11/19 11:50:53 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------*/

/* グローバル */

static void	GroundIK2( Work *, float ) ;

void	PL_GroundIK2( PlayerWork *work, float height )
{
	GroundIK2( work, height ) ;
}

static	int	AvoidSink( Work *work, float height, float len ) ;

int		PL_AvoidSink( PlayerWork *work, float height, float len )
{
	return AvoidSink( work, height, len ) ;
}

static int CheckGroundEnableDir( PlayerWork *work ) ;

int		PL_CheckGroundEnableDir( PlayerWork *work )
{
	return CheckGroundEnableDir( work ) ;
}

/*----------------------------------------------------------------*/

/* ＳＶＥＣＴＯＲが０ベクトルかどうかチェック */
static	inline	int		CheckZeroSVector( v )
SVECTOR			*v ;
{
#if 1//BP_MATH - replaced with version that works for both little and big endian
   return( (v->vx == 0) && (v->vy == 0) && (v->vz == 0) );
#else
	return ( ( ( *( long64 * )v & I64(0xffffffffffff) ) ) ? 0 : 1 ) ;
#endif
}

/* アジャスト値のリセット */
static	void	ResetRotAdjust( work )
Work			*work ;
{
	MOTION_CONTROL	*m_ctrl ;
	SVECTOR			*adj, *tadj ;
	int				i, flag, flag2 ;

	adj = work->rot_adjusts ;
	tadj = work->turn_adjusts ;
	/* 角度 */
	m_ctrl = work->body.m_ctrl ;
	flag = m_ctrl->adjust_flag ;
	flag2 = 0 ;
	GV_ZeroMemory( tadj, sizeof( SVECTOR ) * 24 ) ;
	for ( i = 0; i < N_UNITS; i ++ ) {
		if ( ( flag & 1 ) && !CheckZeroSVector( &adj[ i ] ) ) {
			flag2 |= ( 1 << i ) ;
		}
		flag >>= 1 ;
	}
	m_ctrl->adjust_flag = flag2 ;
	/* 高さ */
	/* 何もしなくていい */
}

/* アジャスト値キープ */
static	void	KeepRotAndHeightAdjust( Work *work )
{
	if ( !CheckZeroSVector( &work->rot_adjusts[ HUMAN21_KOSHI ] ) ) {
		work->turn_adjusts[ HUMAN21_KOSHI ] = work->rot_adjusts[ HUMAN21_KOSHI ] ;
		work->body.m_ctrl->adjust_flag |= ( 1 << HUMAN21_KOSHI ) ;
	}
	if ( !CheckZeroSVector( &work->rot_adjusts[ HUMAN21_MIGI_ASHI1 ] ) ) {
		work->turn_adjusts[ HUMAN21_MIGI_ASHI1 ] = work->rot_adjusts[ HUMAN21_MIGI_ASHI1 ] ;
		work->body.m_ctrl->adjust_flag |= ( 1 << HUMAN21_MIGI_ASHI1 ) ;
	}
	if ( !CheckZeroSVector( &work->rot_adjusts[ HUMAN21_HIDARI_ASHI1 ] ) ) {
		work->turn_adjusts[ HUMAN21_HIDARI_ASHI1 ] = work->rot_adjusts[ HUMAN21_HIDARI_ASHI1 ] ;
		work->body.m_ctrl->adjust_flag |= ( 1 << HUMAN21_HIDARI_ASHI1 ) ;
	}
	work->turn_adjusts[ 22 ].vy = work->rot_adjusts[ 22 ].vy ;
	work->turn_adjusts[ 23 ].vy = work->rot_adjusts[ 23 ].vy ;
}

/* アジャスト値計算 */
static	void	RotAndHeightAdjust( work )
Work			*work ;
{
	FVECTOR			quat ;
	MOTION_CONTROL	*m_ctrl ;
	SVECTOR			*adj, *tadj ;
	DG_OBJS			*objs ;
	float			h ;
	int				i, flag, c ;

	adj = work->rot_adjusts ;
	tadj = work->turn_adjusts ;
	/* 角度 */
	m_ctrl = work->body.m_ctrl ;
	flag = m_ctrl->adjust_flag ;

	for ( i = 0; i < N_UNITS; i ++ ) {
		if ( flag & 1 ) {
			if ( Flag2( FLAG2_ADJUST_SLOW ) && i >= HUMAN21_MIGI_ASHI1 ) {
				GV_NearExp8PV( &adj[ i ], &tadj[ i ], 3 ) ;
			} else {
				GV_NearExp4PV( &adj[ i ], &tadj[ i ], 3 ) ;
			}
			if ( adj[ i ].vy != 0 ) {
				GM_RotToQuat( &adj[ i ], &quat ) ;
			} else {
				GM_RotToQuatXAfterY( &adj[ i ], &quat ) ;
			}
			DG_COPY_VEC( &m_ctrl->adjust[ i ], &quat ) ;
		}
		flag >>= 1 ;
	}

	/* 高さ */
	c = 0 ;
	objs = work->body.objs ;
	adj[ 22 ].vy = GV_NearExp8( adj[ 22 ].vy, tadj[ 22 ].vy ) ;
	adj[ 23 ].vy = GV_NearExp8( adj[ 23 ].vy, tadj[ 23 ].vy ) ;
	if ( adj[ 22 ].vy || adj[ 23 ].vy ) {
		h = ( float )( adj[ 22 ].vy + adj[ 23 ].vy ) ;
		objs->world.m[ 3 ][ 1 ] += h ;
		for ( i = 0; i < N_UNITS; i ++ ) {
			objs->objs[ i ].world.m[ 3 ][ 1 ] += h ;
		}		
	}
	UnsetFlag2( FLAG2_ADJUST_SLOW ) ;
}

/* ＸＺアジャスト */
/* グローバル */
void			PL_AdjustXZ( PlayerWork *work, FVECTOR *shift )
{
	int			i ;
	DG_OBJS		*objs ;

	objs = work->body.objs ;
	objs->world.m[ 3 ][ 0 ] += shift->vx ;
	objs->world.m[ 3 ][ 2 ] += shift->vz ;
	for ( i = 0; i < N_UNITS; i ++ ) {
		objs->objs[ i ].world.m[ 3 ][ 0 ] += shift->vx ;
		objs->objs[ i ].world.m[ 3 ][ 2 ] += shift->vz ;
	}		
}

void			PL_AdjustXZFromTo( PlayerWork *work, FVECTOR *prev, FVECTOR *next )
{
	FVECTOR		diff ;

	_sceVu0SubVector( &diff, next, prev ) ;
	PL_AdjustXZ( work, &diff ) ;
}

static	void	PL_AdjustXYZ( Work *work, FVECTOR *shift )
{
	int			i ;
	DG_OBJS		*objs ;

	PL_AdjustXZ( work, shift ) ;
	objs = work->body.objs ;
	objs->world.m[ 3 ][ 1 ] += shift->vy ;
	for ( i = 0; i < N_UNITS; i ++ ) {
		objs->objs[ i ].world.m[ 3 ][ 1 ] += shift->vy ;
	}		
}

/* 角度アジャスト値セット */
static	void	SetRotAdjust( Work *work, SVECTOR *rot, int n )
{
	work->turn_adjusts[ n ] = *rot ;
	work->body.m_ctrl->adjust_flag |= 1LL /* BP_WARNING - Added LL */ << n ;
}

/* 高さアジャスト値セット */
static	inline	void	SetHeightAdjust( work, height )
Work			*work ;
int				height ;
{
	work->turn_adjusts[ 23 ].vy = height ;
}

static	inline	void	SetHeightAdjust2( work, height )
Work			*work ;
int				height ;
{
	work->turn_adjusts[ 22 ].vy = height ;
}

/* 階段床の上から２５０以内は特殊処理 */
static	float	StepHeightAdjust( HZX_FLR *flr, float level )
{
	float		max ;

	max = flr->p1.y ;
	if ( max < flr->p2.y ) max = flr->p2.y ;
	if ( max < flr->p3.y ) max = flr->p3.y ;
	if ( max < flr->p4.y ) max = flr->p4.y ;
	if ( max - level < 250.0F ) return ( max - level ) ;
	return 250.0F ;
}

/* 階段の上の方チェック */
static	int		UpsideStep( Work *work )
{
	if ( !( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ) return 1 ;
	if ( StepHeightAdjust( work->control.level[ 0 ], 
						   work->control.levels[ 0 ] ) < 250.0F ) return 1 ;
	return 0 ;
}

/* 床傾き計算 */
#define	GetGRot( _w, _z )	GetGRotEx( _w, _z, (_w)->control.rot.vy )

static	int	GetGRotEx( Work *work, float z, int ry )
{
    FVECTOR	head, vec ;
    SVECTOR	rot ;
    float	hh, diff ;
    int		turn ;	

    if ( ( work->control.level[ 0 ] == NULL ) ||
		( work->floor_atr & HZX_FLOOR_FLAT ) ) return 0 ;
    rot.vx = 0 ; rot.vy = ry ; rot.vz = 0 ;
    DG_SetPos2( &( work->control.mov ), &rot ) ;
    head.vx = head.vy = 0.0F ;
    head.vz = z ; 
    DG_PutVector( &head, &head, 1 ) ;
    HZX_SlopeFloorLevel( &hh, &head, work->control.level[ 0 ] ) ;
    diff = hh - work->control.levels[ 0 ] ;
    vec.vx = diff ; 
    vec.vz = z ;
    turn = - GV_VecDir2( &vec ) ;
    if ( turn < -2048 ) turn += 4096 ;
    return turn ;
}


#if 0
static	short	GetGRot2( work, level, atr, p1, p2 )
Work		*work ;
HZX_FLR		*level ;
u_int		atr ;
FVECTOR		*p1, *p2 ;
{
    FVECTOR	vec ;
    short	turn ;
    float	h1, h2 ;

    if ( level == NULL || ( atr & HZX_FLOOR_FLAT ) ) return 0 ;

    HZX_SlopeFloorLevel( &h1, p1, level ) ;
    HZX_SlopeFloorLevel( &h2, p2, level ) ;

    vec.vx = h2 - h1 ; 
    vec.vz = GV_VecLen3F2( p1, p2 ) ;
    turn = - GV_VecDir2( &vec ) ;
    if ( turn < -2048 ) turn += 4096 ;
    return turn ;    
}
#endif

#if 0
/* 匍匐時傾き */
static	void	SlopeGround( work )
Work		*work ;
{
#if 0
    work->g_rot = 0 ;
    return ;
#else
	//    if ( !Status( PLAYER_GROUND ) ) work->g_rot = 0 ;
    work->g_rot = GetGRot( work, 750.0F ) ;
    work->control.turn.vx = work->g_rot ;
#endif
}
#endif

/* 左右めりこみチェック */
/* はりつき専用 */
//static	FVECTOR	RightAdj, LeftAdj ;
//static	float	RightLen, LeftLen ;
static	int	CheckLRWidth( Work *work, SVECTOR *rotate, 
						  float llen, float rlen, float height ) 
{
    int		res ;
    FVECTOR	mov, ptr, vec ;
    SVECTOR	rot ;
    float	left, right ;

    res = 0 ;
    DG_COPY_VEC( &mov, &work->control.mov ) ;
    mov.vy = work->control.hzx_base + height ;
    rot.vx = rot.vz = 0 ;
    /* 左 */
    rot.vy = ( rotate->vy + 1024 ) & 4095 ;
    if ( CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector, &rot,
							   llen, HZX_CHK_SEGMENT, HZX_SEG_NO_PLAYER, 0 ) ) {
		HZX_GetOnlineVector( &vec ) ;
		HZX_GetOnlinePoint( &ptr ) ;
		left = LeftLen = GV_VecLen3F( &vec ) ;
		if ( left < llen ) {
			res |= 1 ;
			GV_LenVec3F( &vec, &vec, 0.0F, llen + 1.0F ) ;
			_sceVu0SubVector( &ptr, &ptr, &vec ) ;
			_sceVu0SubVector( &LeftAdj, &ptr, &mov ) ;
			LeftAdj.vy = 0.0F ;
		}
    }
    /* 右 */
    rot.vy = ( rotate->vy - 1024 ) & 4095 ;
    if ( CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector, &rot,
							   rlen, HZX_CHK_SEGMENT, HZX_SEG_NO_PLAYER, 0 ) ) {
		HZX_GetOnlineVector( &vec ) ;
		HZX_GetOnlinePoint( &ptr ) ;
		right = RightLen = GV_VecLen3F( &vec ) ;
		if ( right < rlen ) {
			res |= 2 ;
			GV_LenVec3F( &vec, &vec, 0.0F, rlen + 1.0F ) ;
			_sceVu0SubVector( &ptr, &ptr, &vec ) ;
			_sceVu0SubVector( &RightAdj, &ptr, &mov ) ;
			RightAdj.vy = 0.0F ;
		}
    }    
    return res ;
}

/* 前後めりこみチェック */
//static 	FVECTOR	FrontAdj, BackAdj ;
//static	float	FrontLen, BackLen ;
#if 0
static	int	CheckFBWidth( Work *work, SVECTOR *rotate, 
						  float len, float flen, float blen, float height )
{
    SVECTOR	rot ;
    FVECTOR	vec, mov, ptr ;
    float	front, back ;
    int		res ;

    res = 0 ;
    front = back = len + 1.0F ;
    DG_COPY_VEC( &mov, &work->control.mov ) ;
    mov.vy = work->control.hzx_base + height ;
    /* 前方 */
    if ( CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector, rotate,
							   len + 1.0F, HZX_CHK_SEGMENT, 
							   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineVector( &vec ) ;
		HZX_GetOnlinePoint( &ptr ) ;
		front = FrontLen = GV_VecLen3F( &vec ) ;
		if ( front < flen ) {
			res |= 1 ;
			GV_LenVec3F( &vec, &vec, 0.0F, flen + 1.0F ) ;
			_sceVu0SubVector( &ptr, &ptr, &vec ) ;
#if 1
			_sceVu0SubVector( &FrontAdj, &ptr, &mov ) ;
			FrontAdj.vy = 0.0F ;
#else
			DG_COPY_VEC( &FrontAdj, &ptr ) ;
			FrontAdj.vy = work->control.mov.vy ;
#endif
		}
    }
    /* 後方 */
    rot = *rotate ; rot.vy = ( rot.vy + 2048 ) & 4095 ; rot.vx = -rot.vx ;
    if ( CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector, &rot,
							   len + 1.0F, HZX_CHK_SEGMENT,
							   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineVector( &vec ) ;
		HZX_GetOnlinePoint( &ptr ) ;
		back = BackLen = GV_VecLen3F( &vec ) ;
		if ( back < blen ) {
			res |= 2 ;
			GV_LenVec3F( &vec, &vec, 0.0F, blen + 1.0F ) ;
			_sceVu0SubVector( &ptr, &ptr, &vec ) ;
#if 1
			_sceVu0SubVector( &BackAdj, &ptr, &mov ) ;
			BackAdj.vy = 0.0F ;
#else
			DG_COPY_VEC( &BackAdj, &ptr ) ;
			BackAdj.vy = work->control.mov.vy ;
#endif
		}
    }    
    if ( front + back < len ) res |= 4 ;
    return res ;
}
#endif

#define	CheckFBWidth2( _w, _l, _f, _b, _h )	CheckFBWidth3( _w, -1, _l, _f, _b, _h )

static	int	CheckFBWidth3( Work *work, int ry, 
						   float len, float flen, float blen, float height )
{
    SVECTOR	rot ;
    FVECTOR	vec, mov, ptr, shift ;
    float	front, back ;
    int		res ;

    res = 0 ;
	front = back = len + 1.0F ;
    DG_COPY_VEC( &mov, &work->control.mov ) ;
    mov.vy = work->control.hzx_base + height ;
	if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) mov.vy += 250.0F ;
	rot.vz = 0 ;
	if ( ry < 0 ) {
		rot.vx = work->control.rot.vx + work->rot_adjusts[ HUMAN21_KOSHI ].vx ;
		rot.vy = work->control.rot.vy ;
	} else {
		if ( work->stance == GROUND ) {
			rot.vx = work->control.rot.vx + work->rot_adjusts[ HUMAN21_KOSHI ].vx ;
		} else {
			rot.vx = GetGRotEx( work, 500.0F, ry ) ;
		}
		rot.vy = ry ;
	}
    /* 前方 */
	GV_SetVec3( &shift, 0.0F, 0.0F, flen ) ;
	DG_SetPos2( &mov, &rot ) ;
	DG_PutVector( &shift, &work->front_chk_pos, 1 ) ;

    if ( CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector, &rot,
							   len, HZX_CHK_SEGMENT, 
							   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
		HZX_GetOnlineVector( &vec ) ;
		HZX_GetOnlinePoint( &ptr ) ;

		/* 間に床があるときはＯＫとする */
		if ( !HZX_OnlineHazardCheck( work->control.hzx_id, &mov, &ptr, HZX_CHK_FLOOR,
								     0, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
			front = FrontLen = GV_VecLen3F( &vec ) ;
			if ( front < flen ) {
				res |= 1 ;
				GV_LenVec3F( &vec, &vec, 0.0F, flen + 1.0F ) ;
				_sceVu0SubVector( &ptr, &ptr, &vec ) ;
#if 1
				_sceVu0SubVector( &FrontAdj, &ptr, &mov ) ;
				FrontAdj.vy = 0.0F ;
#else
				DG_COPY_VEC( &FrontAdj, &ptr ) ;
				FrontAdj.vy = work->control.mov.vy ;
#endif
				DG_COPY_VEC( &work->front_chk_pos, &ptr ) ;
			} 
		}
    } else {
		//DG_COPY_VEC( &work->front_chk_pos, &MovRotLenChkTo ) ;
	}

    /* 後方 */
	if ( work->stance == GROUND ) {
		rot.vx += work->rot_adjusts[ HUMAN21_MIGI_ASHI1 ].vx ;
	}
	rot.vx = -rot.vx ;
	rot.vy += 2048 ;

	GV_SetVec3( &shift, 0.0F, 0.0F, blen ) ;
	DG_SetPos2( &mov, &rot ) ;
	DG_PutVector( &shift, &work->back_chk_pos, 1 ) ;

    if ( CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector, &rot,
							   len, HZX_CHK_SEGMENT,
							   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
		HZX_GetOnlineVector( &vec ) ;
		HZX_GetOnlinePoint( &ptr ) ;

		/* 間に床があるときはＯＫとする */
		if ( !HZX_OnlineHazardCheck( work->control.hzx_id, &mov, &ptr, HZX_CHK_FLOOR,
								     0, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
			back = BackLen = GV_VecLen3F( &vec ) ;
			if ( back < blen ) {
				res |= 2 ;
				GV_LenVec3F( &vec, &vec, 0.0F, blen + 1.0F ) ;
				_sceVu0SubVector( &ptr, &ptr, &vec ) ;
#if 1
				_sceVu0SubVector( &BackAdj, &ptr, &mov ) ;
				BackAdj.vy = 0.0F ;
#else
				DG_COPY_VEC( &BackAdj, &ptr ) ;
				BackAdj.vy = work->control.mov.vy ;
#endif
				DG_COPY_VEC( &work->back_chk_pos, &ptr ) ;
			}
		}
    } else {
		//DG_COPY_VEC( &work->back_chk_pos, &MovRotLenChkTo ) ;
	}
    if ( front + back < len ) res |= 4 ;
	//printf( "%d %f %f : %f : %x\n", ry, front, back, len, res ) ;
    return res ;
}

#if 0
/* 2点の中間位置で床チェックし、床高さを返す */
static	float	LevelHeightofMiddlePoint( p1, p2, id, type, flag )
FVECTOR			*p1, *p2 ;
HZX_GROUP_ID	id ;
int				type, flag ;
{
	FVECTOR		p3 ;
	int			res, atrs[ 2 ] ;
	HZX_FLR		level[ 2 ] ;
	float		levels[ 2 ] ;

	_sceVu0AddVector( &p3, p1, p2 ) ;
	_sceVu0ScaleVector( &p3, &p3, 0.50F ) ;
	res = HZX_LevelHazardCheck( id, &p3, type, flag ) ;
	levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	if ( res & 1 ) {
		HZX_GetLevelHeight( levels ) ;
		HZX_GetLevelHazard( level, atrs ) ;
		if ( atrs[ 0 ] & HZX_FLOOR_STEP ) {
			levels[ 0 ] += StepHeightAdjust( &level[ 0 ], levels[ 0 ] ) ;
		}
	}
	return levels[ 0 ] ;
}
#endif

#if 0
/* 現在の向きで匍匐して、段差がないかどうか調べる */
/* 幅はあるという前提 */
/* ０：ＯＫ ８：腰ＮＧ １６：前ＮＧ ３２：後ろＮＧ */
static	int		CheckGroundFloor( work, mov, rot1, rot2 )
Work			*work ;
FVECTOR			*mov ;
SVECTOR			*rot1, *rot2 ;
{
#if 0
	FVECTOR		head, foot ;
	float		levels[ 2 ], l0, l1, l2, org, adj ;
	float		mdif, middle ;
	int			res, flag, atrs[ 2 ] ;
	int			chk_type, flr_flag ;
	HZX_FLR		flrs[ 2 ] ;
	HZX_GROUP_ID	hzx_id ;

	res = 0 ;
	org = work->control.levels[ 0 ] ;
	if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
		org += StepHeightAdjust( work->control.level[ 0 ], 
								 work->control.levels[ 0 ] ) ;
	}
	hzx_id = work->control.hzx_id ;
	chk_type = work->control.hzx_check_type ;
	flr_flag = work->control.flr_flag ;
	flag = HZX_LevelHazardCheck( hzx_id, mov, chk_type, flr_flag ) ;
	if ( flag & 1 ) {
		HZX_GetLevelHeight( levels ) ;
		HZX_GetLevelHazard( flrs, atrs ) ;
		l0 = levels[ 0 ] ;
		if ( atrs[ 0 ] & HZX_FLOOR_STEP ) {
			adj = StepHeightAdjust( &flrs[ 0 ], levels[ 0 ] ) ;
			l0 += adj ;
		}
		mdif = DG_FABS( work->control.mov.vy - mov->vy ) ;
		/* すでに段差違い */
		if ( DG_FABS( org - l0 ) >= 500.0F ) {
			middle = LevelHeightofMiddlePoint( &work->control.mov, mov, 
											   hzx_id, chk_type, flr_flag ) ;
			if ( DG_FABS( org - middle ) >= 500.0F ||
				 DG_FABS( l0 - middle ) >= 500.0F ) return 0x8 ;
		}
	} else {
		return 0 ;
	}
	l1 = l2 = GM_WORLD_LIMIT_BOTTOM ;
	DG_COPY_VEC( &head, &DG_ZeroVector ) ;
	DG_COPY_VEC( &foot, &DG_ZeroVector ) ;
	head.vz = 750.0F ;
	foot.vz = -950.0F ;
	DG_SetPos2( mov, rot1 ) ;
	DG_PutVector( &head, &head, 1 ) ;
	DG_SetPos2( mov, rot2 ) ;
	DG_PutVector( &foot, &foot, 1 ) ;
	if ( HZX_OnlineHazardCheck( hzx_id, mov, &head, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
							    flr_flag ) ) {
		HZX_GetOnlinePoint( &head ) ;
	}
	adj = 0.0F ;
	flag = HZX_LevelHazardCheck( hzx_id, &head, chk_type, flr_flag ) ;
	if ( flag & 1 ) {
		HZX_GetLevelHeight( levels ) ;
		HZX_GetLevelHazard( flrs, atrs ) ;
		l1 = levels[ 0 ] ;
		if ( atrs[ 0 ] & HZX_FLOOR_STEP ) {
			adj = StepHeightAdjust( &flrs[ 0 ], levels[ 0 ] ) ;
			l1 += adj ;
		}
	}
	mdif = DG_FABS( mov->vy - head.vy ) ;
//printf( "1 : %f %f %f\n", l1, l0, mdif ) ;
	if ( DG_FABS( l1 - l0 ) >= 500.0F ) {
		middle = LevelHeightofMiddlePoint( mov, &head,
										   hzx_id, chk_type, flr_flag ) ;
		if ( DG_FABS( l0 - middle ) >= 500.0F ||
			 DG_FABS( l1 - middle ) >= 500.0F ) res |= 0x10 ;
	}
	if ( HZX_OnlineHazardCheck( hzx_id, mov, &foot, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
							    flr_flag ) ) {
		HZX_GetOnlinePoint( &foot ) ;
	}
	flag = HZX_LevelHazardCheck( hzx_id, &foot, chk_type, flr_flag ) ;
	adj = 0.0F ;
	if ( flag & 1 ) {
		HZX_GetLevelHeight( levels ) ;
		HZX_GetLevelHazard( flrs, atrs ) ;
		l2 = levels[ 0 ] ;
		if ( atrs[ 0 ] & HZX_FLOOR_STEP ) {
			adj = StepHeightAdjust( &flrs[ 0 ], levels[ 0 ] ) ;
			l2 += adj ;
		}
	}
	mdif = DG_FABS( mov->vy - foot.vy ) ;
//printf( "2 : %f %f %f\n", l2, l0, mdif ) ;
	if ( DG_FABS( l2 - l0 ) >= 500.0F ) {
		middle = LevelHeightofMiddlePoint( mov, &foot,
										   hzx_id, chk_type, flr_flag ) ;
		if ( DG_FABS( l0 - middle ) >= 500.0F ||
			 DG_FABS( l2 - middle ) >= 500.0F ) res |= 0x20 ;
	}
	return res ;
#else
	return 0 ;
#endif
}
#endif

/* 段差チェック（２） */
static	int	CheckGroundFloor2( Work *work, FVECTOR *front, FVECTOR *back ) 
{
	int		res = 0 ;
	float	levels[ 2 ], l[ 3 ] ;
	HZX_HZD	floor[ 2 ] ;
	int		atrs[ 2 ], atr[ 2 ] ;
	FVECTOR	fchk, bchk, mov ;
	
	DG_COPY_VEC( &mov, &work->control.mov ) ;
	mov.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;

	DG_COPY_VEC( &fchk, front ) ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &mov, &fchk,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
		HZX_GetOnlinePoint( &fchk ) ;
	} 
	DG_COPY_VEC( &bchk, back ) ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &mov, &bchk,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
		HZX_GetOnlinePoint( &bchk ) ;
	} 

	l[ 0 ] = work->control.levels[ 0 ] ;

	levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	atr[ 0 ] = 0 ;
	if ( HZX_LevelHazardCheck( HZX_CurrentGroupID, &fchk, HZX_CHK_ALL, 
							   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) & 1 ) {
		HZX_GetLevelHeight( levels ) ;
		HZX_GetLevelHazard( floor, atrs ) ;
		atr[ 0 ] = atrs[ 0 ] ;
	}
	l[ 1 ] = levels[ 0 ] ;
	atr[ 1 ] = 0 ;
	levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	if ( HZX_LevelHazardCheck( HZX_CurrentGroupID, &bchk, HZX_CHK_ALL, 
							   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) & 1 ) {
		HZX_GetLevelHeight( levels ) ;
		HZX_GetLevelHazard( floor, atrs ) ;
		atr[ 1 ] = atrs[ 0 ] ;
	}
	//NewPointView( &fchk, 32, 232, 32 ) ;
	//NewPointView( &bchk, 32, 32, 232 ) ;
	l[ 2 ] = levels[ 0 ] ;
	if ( DG_FABS( l[ 0 ] - l[ 1 ] ) > 2000.0F ) res |= 0x10 ;
	if ( DG_FABS( l[ 0 ] - l[ 2 ] ) > 2000.0F ) res |= 0x20 ;
	if ( l[ 0 ] - l[ 1 ] > 100.0F && l[ 0 ] - l[ 2 ] > 100.0F ) {
		if ( !( ( atr[ 0 ] | atr[ 1 ] ) & HZX_FLOOR_STEP ) ) res |= 0x40 ;
	}
	if ( l[ 0 ] - l[ 1 ] < -100.0F && l[ 0 ] - l[ 2 ] < -100.0F ) {
		if ( !( ( atr[ 0 ] | atr[ 1 ] ) & HZX_FLOOR_STEP ) ) res |= 0x80 ;
	}

	//printf( "[%d]--- %x --- %f %f %f\n", GV_Time, res, l[ 0 ], l[ 1 ], l[ 2 ] ) ;

	return res ;
}

/* 現在の向きで、前後に匍匐できる幅があるか調べる */
#if 0
static	int	CheckGroundWidth2( Work *work, SVECTOR *rotate, float len )
{
	int			c ;
    float		chk_len, front, back ;

	GetModelLength( work, &front, &back ) ;
	/* 微調整 */
	front += 50.0F ;
	back += 50.0F ;
	chk_len = front + back ;
	if ( chk_len < len ) chk_len = len ;
	c = CheckFBWidth3( work, rotate->vy & 4095, chk_len, front, back, NEAR_HEIGHT_GROUND ) ;
	if ( c == 0 ) {
		c = CheckGroundFloor2( work, &work->front_chk_pos, &work->back_chk_pos ) ;
	}
	return c ;
}
#endif

static	int	CheckGroundWidth3( Work *work, SVECTOR *rotate, float len )
{
	int			c ;
    float		chk_len, front, back ;

	if ( work->stance == GROUND ) {
		GetModelLength( work, &front, &back ) ;
		/* 微調整 */
		front += 50.0F ;
		back += 50.0F ;
		chk_len = front + back ;
		if ( chk_len < len ) chk_len = len ;
	} else {
		front = 900.0F ;
		back = 1100.0F ;
		chk_len = len ;
	}
	c = CheckFBWidth3( work, rotate->vy & 4095, chk_len, front, back, NEAR_HEIGHT_GROUND ) ;
	if ( c == 0 ) {
		c = CheckGroundFloor2( work, &work->front_chk_pos, &work->back_chk_pos ) ;
	}
	return c ;
}

static	int	CheckGroundWidth( work, rotate )
Work		*work ;
SVECTOR		*rotate ;
{
	int			c ;
//	int			c2 ;
//	int			g_rot ;
//	FVECTOR		mov ;
//	SVECTOR		rot, rot2 ;

	c = CheckFBWidth3( work, rotate->vy & 4095, 1999.0F, 800.0F, 1100.0F, NEAR_HEIGHT_GROUND ) ;
	if ( c == 0 ) c |= CheckGroundFloor2( work, &work->front_chk_pos, &work->back_chk_pos ) ;
	return c ;
//	if ( ( c & 3 ) == 3 || ( c & 4 ) ) return c ;
#if 0
	g_rot = work->g_rot ;
	if ( g_rot == 0 && 
		!( work->control.flr_atrs[ 0 ] & HZX_FLOOR_FLAT ) ) {
		g_rot = GetGRot( work, 500.0F ) ;
	}
	rot = rot2 = work->control.rot ; 
	rot.vx = g_rot ;
	rot2.vx = ( work->g_rot != 0 ) ? 
		work->rot_adjusts[ HUMAN21_MIGI_ASHI1 ].vx + g_rot : g_rot ;
	DG_COPY_VEC( &mov, &work->control.mov ) ;
	if ( c & 1 ) _sceVu0AddVector( &mov, &mov, &FrontAdj ) ;
	else if ( c & 2 ) _sceVu0AddVector( &mov, &mov, &BackAdj ) ;
	c2 = CheckGroundFloor( work, &mov, &rot, &rot2 ) ;	
	if ( c2 ) c |= 4 | c2 ;
#endif
//	return c ;
}

/* どの方向になら匍匐できるか調べる */
static	int	CheckGroundEnableDir( work )
Work		*work ;
{
    SVECTOR		def, rot, rot2 ;
    int			c, c2 ;
#if 0
    int		i, dir, step ;
    /* 8方向バージョン */
    step = 512 ; dir = 1 ;
    def = work->control.turn ; def.vx = def.vz = 0 ;
    def.vy = ( def.vy / 512 * 512 ) & 4095 ;
    rot = def ; 
    for ( i = 0; i < 4; i ++ ) {
		c = CheckGroundWidth( work, &rot ) ;
		if ( !( c & 4 ) ) return ( rot.vy & 4095 ) ;
		rot.vy = def.vy + step * dir ;
		if ( i & 1 ) step *= 2 ;
		dir *= -1 ;
    }
#else
    /* ４方向バージョン */
	def.vx = work->g_rot ;
    def.vy = work->control.turn.vy ; 
	def.vz = 0 ;
    def.vy = ( def.vy / 1024 * 1024 ) & 4095 ;
    rot = def ; 
    c = CheckGroundWidth3( work, &rot, 2400.0F ) ;
//    if ( !( c & 4 ) && ( ( c & 3 ) != 3 ) ) return ( rot.vy & 4095 ) ;
    if ( c == 0 ) return ( rot.vy & 4095 ) ;
    rot.vy = def.vy + 1024 ;
    c = CheckGroundWidth3( work, &rot, 2400.0F ) ;
//    if ( !( c & 4 ) && ( ( c & 3 ) != 3 ) ) return ( rot.vy & 4095 ) ;
    if ( c == 0 ) return ( rot.vy & 4095 ) ;
#endif
	//printf( "no dir -- omoi shori suruzo!\n" ) ;
	/* 重い処理 */
	c2 = 0 ;
	{
		int			adj ;

		adj = 64 ;
		while ( adj < 2048 ) {
			rot.vy = def.vy + adj ;
			c = CheckGroundWidth3( work, &rot, 2400.0F ) ;
			if ( !( c & 4 ) && ( ( c & 3 ) != 3 ) ) {
				if ( c == 0 ) {
//					printf( "ok dir %d\n", rot.vy & 4095 ) ;
					return ( rot.vy & 4095 ) ;
				} else {
					rot2 = rot ;
					c2 = 1 ;
				}
			}
			adj += 64 ;
		}
	}
	if ( c2 ) {
//		printf( "shouganai dir %d\n", rot2.vy & 4095 ) ;
		return ( rot2.vy & 4095 ) ;
	}
    return -1 ;
}

/* 汎用めり込み防止ルーチン */
static	int	AvoidSink( Work *work, float height, float len )
{
    int		res ;
    float	chk_len, front, back ;

	GetModelLength( work, &front, &back ) ;
	/* 微調整 */
	front += 50.0F ;
	back += 50.0F ;
	chk_len = front + back ;
	if ( chk_len < len ) chk_len = len ;
    //res = CheckFBWidth2( work, chk_len, front, back, height ) ;
	res = CheckFBWidth3( work, work->control.rot.vy & 4095, 
						 chk_len, front, back, height ) ;
	if ( res == 0 ) res |= CheckGroundFloor2( work, &work->front_chk_pos, &work->back_chk_pos ) ;	
    if ( ( res & 4 ) || ( ( res & 3 ) == 3 ) || ( res & 0xf0 ) ) {
		//printf( "no width %x\n", res ) ;
		work->control.turn.vy = work->pre_turn.vy ;
    } else if ( res & 1 ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;
		_sceVu0AddVector( &work->control.step, &work->control.step, 
						 &FrontAdj ) ;	
		AdjustXZ( work, &FrontAdj ) ;
    } else if ( res & 2 ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;
		_sceVu0AddVector( &work->control.step, &work->control.step, 
						 &BackAdj ) ;
		AdjustXZ( work, &BackAdj ) ;	
    }
    return res ;
}

static	int	AvoidSink2( Work *work, float height, float front, float back, float len )
{
    int		res ;

	if ( work->down_dir == DOWN_BACK ) {
		float	tmp ;

		tmp = front ; front = back ; back = tmp ;
	}
//    res = CheckFBWidth( work, &work->control.rot, len, front, back, height ) ;
    res = CheckFBWidth2( work, len, front, back, height ) ;
	if ( res == 0 ) res |= CheckGroundFloor2( work, &work->front_chk_pos, &work->back_chk_pos ) ;
    if ( ( res & 4 ) || ( ( res & 3 ) == 3 ) || ( res & 0xf0 ) ) {
		//printf( "no width\n" ) ;
		work->control.turn.vy = work->pre_turn.vy ;
    } else if ( res & 1 ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;
		_sceVu0AddVector( &work->control.step, &work->control.step, 
						 &FrontAdj ) ;	
		AdjustXZ( work, &FrontAdj ) ;
    } else if ( res & 2 ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;
		_sceVu0AddVector( &work->control.step, &work->control.step, 
						 &BackAdj ) ;
		AdjustXZ( work, &BackAdj ) ;	
    }
    return res ;
}

#if 0
static	void	GroundRotAdjust( m_ctrl, adj, rot, n )
MOTION_CONTROL	*m_ctrl ;
short		*adj ;
SVECTOR		*rot ;
int		n ;
{
    FVECTOR	quat ;
    SVECTOR	rotate ;

    rotate.vy = rotate.vz = 0 ;
    adj[ n ] = GV_NearExp8P( adj[ n ], rot->vx ) ;
    rotate.vx = adj[ n ] ;
    GM_RotToQuatXAfterY( &rotate, &quat ) ;
	m_ctrl->adjust[ n ] = quat ;
	m_ctrl->adjust_flag |= 1 << n ;    
}

static	void	GroundHeightAdjust( DG_OBJS *objs, short *adj, float height )
{
    int		i ;
    float	tmp ;

    tmp = adj[ 23 ] = GV_NearExp8F( adj[ 23 ], height ) ;
    objs->world.m[ 3 ][ 1 ] += tmp ;
    for ( i = 0; i < 21; i ++ ) {
		objs->objs[ i ].world.m[ 3 ][ 1 ] += tmp ;                                   
    }
}

static	void	GroundHeightAdjust2( DG_OBJS *objs, short *adj, float height )
{
    int		i ;
    float	tmp ;

    tmp = adj[ 22 ] = GV_NearExp8F( adj[ 22 ], height ) ;
    objs->world.m[ 3 ][ 1 ] += tmp ;
    for ( i = 0; i < 21; i ++ ) {
		objs->objs[ i ].world.m[ 3 ][ 1 ] += tmp ;                                   
    }
}

/* アジャストのリセット */
static	void	ResetGroundAdjust( work )
Work		*work ;
{
    MOTION_CONTROL	*m_ctrl ;
    DG_OBJS		*objs ;
    short		*adj ;

    m_ctrl = work->body.m_ctrl ;
    objs = work->body.objs ;
    adj = work->ground_adjusts ;
    GroundHeightAdjust( objs, adj, 0.0F ) ;    
    GroundHeightAdjust2( objs, adj, 0.0F ) ;    
    GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 0 ) ;    
    GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 2 ) ;    
    GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 13 ) ;    
    GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 17 ) ;
}
#endif

#if 0
/* 匍匐ＩＫのような何か */
//#define	GIK_DEBUG
#ifdef GIK_DEBUG
#define	gik_printf( a )	printf( a )
#else
#define	gik_printf( a )
#endif
#define	GIK_FLOOR_FLAG	(HZX_FLOOR_IK | HZX_FLOOR_NO_PLAYER)
//#define	GIK_FLOOR_FLAG	(HZX_FLOOR_NO_PLAYER) /* ＩＫ床も見る */

//#define	IGNORE_STEP	/* 階段でも普通にやる */

static	void	GroundIK( work )
Work		*work ;
{
    FVECTOR		from, hand, foot, shift ;
    FVECTOR		vec, vec2, vec3 ;
    SVECTOR		rot, rot2 ;
    HZX_GROUP_ID	hzx_id ;
    DG_OBJS		*objs ;
    int			c1, c2, dir ;
    float		levels0, levels1[ 2 ], levels2[ 2 ] ;
    float		height, ftmp ;
    u_int		atr1, atr2 ;
    MOTION_CONTROL	*m_ctrl ;
    short		*adj, g_rot ;

    m_ctrl = work->body.m_ctrl ;

	//    if ( GV_PadData->status & PAD_L1 ) return ;

    dir = work->down_dir ;
    adj = work->ground_adjusts ;
    g_rot = work->g_rot ;

    hzx_id = work->control.hzx_id ;
    objs = work->body.objs ;
    height = work->control.height ;

    GroundHeightAdjust( objs, adj, 0.0F ) ;

    DG_COPY_VEC( &from, &work->control.mov ) ;
    levels0 = work->control.levels[ 0 ] ;
#ifndef IGNORE_STEP
if ( GV_PadData->status & PAD_L1 ) {
    if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {	
		levels0 += 250.0F ;
		from.vy += 250.0F ;
		GroundHeightAdjust2( objs, adj, 250.0F ) ;
    } else {
		GroundHeightAdjust2( objs, adj, 0.0F ) ;
    }
}
#endif
    DG_SetPos2( &from, &work->control.turn ) ;

    shift.vx = 0.0F ; 
    shift.vy = 0.0F ;
    if ( dir == DOWN_BACK ) shift.vz = -1000.0F ;
    else		    shift.vz = 700.0F ; 
    DG_PutVector( &shift, &hand, 1 ) ;
    if ( dir == DOWN_BACK ) shift.vz = 700.0F ;
    else 		    shift.vz = -1000.0F ;
    DG_PutVector( &shift, &foot, 1 ) ;

    hand.vy += 25.0F ;			/* 微調整 */
    foot.vy += 25.0F ;

    /* 腰と手の間に床があるかチェック */
    c1 = HZX_OnlineHazardCheck( hzx_id, &from, &hand,
							   HZX_CHK_FLOOR, 0, GIK_FLOOR_FLAG ) ;
    if ( c1 ) {
		/* 上半身調整 */
		ftmp = hand.vy ;
		hand.vy += 1500.0F ;
		c2 = HZX_LevelHazardCheck( hzx_id, &hand, 
								  HZX_CHK_FLOOR, GIK_FLOOR_FLAG ) ;
		if ( c2 & 1 ) {
			HZX_GetLevelHeight( levels1 ) ;
			atr1 = HZX_GetLevelAtr() ;
		} else {
			levels1[ 0 ] = levels0 ;
			atr1 = work->control.flr_atrs[ 0 ] ;
		}
#ifndef IGNORE_STEP
if ( GV_PadData->status & PAD_L1 ) {
		if ( atr1 & HZX_FLOOR_STEP ) levels1[ 0 ] += 250.0F ;
}
#endif
		if ( c2 ) {
			if ( levels1[ 0 ] > levels0 ) {
				/* 腕の方が上 */
				/* あおむけ、うつぶせ共通 */
				gik_printf( "ude ue\n" ) ;
				hand.vy = levels1[ 0 ] + 200.0F ;
				vec.vx = hand.vy - from.vy ;
				vec.vz = GV_VecLen3F2( &from, &hand ) ;
				rot.vx = - GV_VecDir2( &vec ) ;
				if ( rot.vx < -2048 ) rot.vx += 4096 ;
				if ( dir == DOWN_BACK ) rot.vx = - rot.vx ;
				rot.vy = rot.vz = 0 ;
				rot.vx = rot.vx - g_rot ;
                GroundRotAdjust( m_ctrl, adj, &rot, 0 ) ;
				rot.vx = - rot.vx ;
                GroundRotAdjust( m_ctrl, adj, &rot, 13 ) ;
                adj[ 17 ] = adj[ 13 ] ;
				m_ctrl->adjust[ 17 ] = m_ctrl->adjust[ 13 ] ;
				m_ctrl->adjust_flag |= 1 << 17 ;
				GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 2 ) ;
			} else {
				/* 腕の方が下 */
				gik_printf( "ude shita\n" ) ;
				if ( dir == DOWN_BACK ) {	
					/* 仰向け */
					hand.vy = levels1[ 0 ] + 160.0F ;
					vec.vx = from.vy - hand.vy ;
					vec.vz = GV_VecLen3F2( &from, &hand ) ;
					rot.vx = - GV_VecDir2( &vec ) ;
					if ( rot.vx < -2048 ) rot.vx += 4096 ;
					rot.vy = rot.vz = 0 ;
					rot.vx = rot.vx - g_rot ;
					GroundRotAdjust( m_ctrl, adj, &rot, 2 ) ;
					GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 0 ) ;
					GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 13 ) ;
					GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 17 ) ;
					return ;
				} 
				/* うつぶせ */
                ftmp = ( levels1[ 0 ] - ftmp ) ;
                from.vy += ftmp ;
				vec.vx = from.vy - foot.vy ;
				vec.vz = GV_VecLen3F2( &from, &foot ) ;
				rot.vx = - GV_VecDir2( &vec ) ;
				if ( rot.vx < -2048 ) rot.vx += 4096 ;
				rot2.vy = rot2.vz = 0 ;
				rot2.vx = rot.vx - g_rot ;
                GroundRotAdjust( m_ctrl, adj, &rot2, 0 ) ;
                hand.vy = levels1[ 0 ] + 200.0F ;
				vec.vx = hand.vy - from.vy ;
				vec.vz = GV_VecLen3F2( &from, &hand ) ;
				rot2.vx = - GV_VecDir2( &vec ) ;
				if ( rot2.vx < -2048 ) rot2.vx += 4096 ;
				rot2.vx = rot2.vx - rot.vx ; 
                rot2.vx -= 128 ;
                GroundRotAdjust( m_ctrl, adj, &rot2, 2 ) ;
                rot.vx = 0 ;
                GroundRotAdjust( m_ctrl, adj, &rot, 13 ) ;
                GroundRotAdjust( m_ctrl, adj, &rot, 17 ) ;
                GroundHeightAdjust( objs, adj, ftmp ) ;
			}
		}
		return ;
    }
    /* 腕の位置の床高さ */
    c2 = HZX_LevelHazardCheck( hzx_id, &hand, 
							  HZX_CHK_FLOOR, GIK_FLOOR_FLAG ) ;
    if ( c2 & 1 ) {
		HZX_GetLevelHeight( levels1 ) ;    
		atr1 = HZX_GetLevelAtr() ;
    } else {
		levels1[ 0 ] = levels0 ;
		atr1 = work->control.flr_atrs[ 0 ] ;
    }
#ifndef IGNORE_STEP
if ( GV_PadData->status & PAD_L1 ) {
    if ( atr1 & HZX_FLOOR_STEP ) levels1[ 0 ] += 250.0F ;
}
#endif

    /* 腰と足の間に床があるかチェック */
    c1 = HZX_OnlineHazardCheck( hzx_id, &from, &foot,
							   HZX_CHK_FLOOR, 0, GIK_FLOOR_FLAG ) ;
    if ( c1 ) {
		/* 下半身調整 */
		ftmp = foot.vy ;
		foot.vy += 1500.0F ;	
		c2 = HZX_LevelHazardCheck( hzx_id, &foot, 
								  HZX_CHK_FLOOR, GIK_FLOOR_FLAG ) ;
		if ( c2 & 1 ) {
			HZX_GetLevelHeight( levels2 ) ;
			atr2 = HZX_GetLevelAtr() ;
		} else {
			levels2[ 0 ] = levels0 ;
			atr2 = work->control.flr_atrs[ 0 ] ;
		}
#ifndef IGNORE_STEP
if ( GV_PadData->status & PAD_L1 ) {
		if ( atr2 & HZX_FLOOR_STEP ) levels2[ 0 ] += 250.0F ;
}
#endif
		if ( c2 ) {
			if ( levels2[ 0 ] > levels0 ) {
				/* 足の方が上 */
				gik_printf( "ashi ue\n" ) ;
				if ( dir == DOWN_BACK ) {
					/* 仰向け */
					foot.vy = levels2[ 0 ] + 250.0F ;
					vec.vx = foot.vy - from.vy ;
					vec.vz = GV_VecLen3F2( &from, &foot ) ;
					rot.vx = - GV_VecDir2( &vec ) ;
					if ( rot.vx < -2048 ) rot.vx += 4096 ;
					rot2.vy = rot2.vz = 0 ;
					rot2.vx = rot.vx - g_rot ;
					GroundRotAdjust( m_ctrl, adj, &rot2, 13 ) ;
					GroundRotAdjust( m_ctrl, adj, &rot2, 17 ) ;
					GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 0 ) ;
					GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 2 ) ;
					return ;
				}
                ftmp = ( levels2[ 0 ] - ftmp ) / 3.0F ;
                from.vy += ftmp ;
				foot.vy = levels2[ 0 ] + 200.0F ;
				vec.vx = from.vy - foot.vy ;
				vec.vz = GV_VecLen3F2( &from, &foot ) ;
				rot.vx = - GV_VecDir2( &vec ) ;
				if ( rot.vx < -2048 ) rot.vx += 4096 ;
				rot2.vy = rot2.vz = 0 ;
				rot2.vx = rot.vx - g_rot ;
                GroundRotAdjust( m_ctrl, adj, &rot2, 0 ) ;
                hand.vy = levels1[ 0 ] + 200.0F ;
				vec.vx = hand.vy - from.vy ;
				vec.vz = GV_VecLen3F2( &from, &hand ) ;
				rot2.vx = - GV_VecDir2( &vec ) ;
				if ( rot2.vx < -2048 ) rot2.vx += 4096 ;
				rot2.vx = rot2.vx - rot.vx ; 
                rot2.vx -= 64 ;
                GroundRotAdjust( m_ctrl, adj, &rot2, 2 ) ;
                rot.vx = 0 ;
                GroundRotAdjust( m_ctrl, adj, &rot, 13 ) ;
                GroundRotAdjust( m_ctrl, adj, &rot, 17 ) ;

                GroundHeightAdjust( objs, adj, ftmp ) ;
			} else {
				/* 足の方が下 */
				/* 仰向け、うつぶせ共通 */
				gik_printf( "ashi shita\n" ) ;
				foot.vy = levels2[ 0 ] + 200.0F ;
				vec.vx = from.vy - foot.vy ;
				vec.vz = GV_VecLen3F2( &from, &foot ) ;
				rot.vx = - GV_VecDir2( &vec ) ;
				if ( dir == DOWN_BACK ) rot.vx = -rot.vx ;
				if ( rot.vx < -2048 ) rot.vx += 4096 ;
				rot.vy = rot.vz = 0 ;
				rot.vx = rot.vx - g_rot ;
                GroundRotAdjust( m_ctrl, adj, &rot, 13 ) ;
                adj[ 17 ] = adj[ 13 ] ;
				m_ctrl->adjust[ 17 ] = m_ctrl->adjust[ 13 ] ;
				m_ctrl->adjust_flag |= 1 << 17 ;			

				GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 0 ) ;	
				GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 2 ) ;	
			}
		}
		return ;	
    }

    /* 足の位置の床高さ */
    c1 = HZX_LevelHazardCheck( hzx_id, &foot, 
							  HZX_CHK_FLOOR, GIK_FLOOR_FLAG ) ;
    if ( c1 & 1 ) {
		HZX_GetLevelHeight( levels2 ) ;
		atr2 = HZX_GetLevelAtr() ;
    } else {
		levels2[ 0 ] = levels0 ;
		atr2 = work->control.flr_atrs[ 0 ] ;
    }
#ifndef IGNORE_STEP
if ( GV_PadData->status & PAD_L1 ) {
    if ( atr2 & HZX_FLOOR_STEP ) levels2[ 0 ] += 250.0F ;
}
#endif

    /* 違いすぎチェック */
    if ( ( DG_FABS( levels0 - levels1[ 0 ] ) > 950.0F ) ||
		( DG_FABS( levels0 - levels2[ 0 ] ) > 1250.0F ) ) {
		//printf( "tigaisugi\n" ) ;
		rot.vx = 0 ;
		GroundRotAdjust( m_ctrl, adj, &rot, 0 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 2 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 13 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 17 ) ;
		return ;	
    }

    /* 同一高さチェック */
    if ( levels0 == levels1[ 0 ] &&
		levels1[ 0 ] == levels2[ 0 ] ) {
		rot.vx = 0 ;
		GroundRotAdjust( m_ctrl, adj, &rot, 0 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 2 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 13 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 17 ) ;
		return ;
    }

    /* 同一平面チェック */
    DG_COPY_VEC( &vec, &from ) ;
    DG_COPY_VEC( &vec2, &hand ) ;
    DG_COPY_VEC( &vec3, &foot ) ;
    vec.vy = levels0 ;
    vec2.vy = levels1[ 0 ] ;
    vec3.vy = levels2[ 0 ] ;
    _sceVu0SubVector( &vec3, &vec2, &vec3 ) ;    
    _sceVu0SubVector( &vec2, &vec2, &vec ) ;
    _sceVu0OuterProduct( &vec, &vec2, &vec3 ) ;    
    if ( GV_VecLen3F( &vec ) < 16.00F ) {
		rot.vx = 0 ;
		GroundRotAdjust( m_ctrl, adj, &rot, 0 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 2 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 13 ) ;
		GroundRotAdjust( m_ctrl, adj, &rot, 17 ) ;
		return ;
    }

    if ( levels1[ 0 ] > levels2[ 0 ] ) {
		/* 手の方が上 */
		if ( levels0 == levels1[ 0 ] ) {
			rot.vx = 0 ;
			GroundRotAdjust( m_ctrl, adj, &rot, 0 ) ;
			GroundRotAdjust( m_ctrl, adj, &rot, 2 ) ;
			GroundRotAdjust( m_ctrl, adj, &rot, 13 ) ;
			GroundRotAdjust( m_ctrl, adj, &rot, 17 ) ;
			return ;
		}
		gik_printf( "ude ue 2\n" ) ;
        foot.vy = levels2[ 0 ] + height ;
        vec.vx = from.vy - foot.vy ;
		vec.vz = GV_VecLen3F2( &from, &foot ) ;
		if ( dir == DOWN_BACK ) {
			rot.vx = GV_VecDir2( &vec ) ;
			if ( rot.vx < -2048 ) rot.vx += 4096 ;
			rot2.vy = rot2.vz = 0 ;
			rot2.vx = rot.vx - g_rot ;
			GroundRotAdjust( m_ctrl, adj, &rot2, 0 ) ;
		} else {
			rot.vx = - GV_VecDir2( &vec ) ;
			if ( rot.vx < -2048 ) rot.vx += 4096 ;
			rot.vx -= 128 ;
			rot2.vy = rot2.vz = 0 ;
			rot2.vx = rot.vx - g_rot ;
			GroundRotAdjust( m_ctrl, adj, &rot2, 0 ) ;
			hand.vy = levels1[ 0 ] + height ;
			vec.vx = hand.vy - from.vy ;
			vec.vz = GV_VecLen3F2( &from, &hand ) ;
			rot2.vx = - GV_VecDir2( &vec ) ;
			if ( rot2.vx < -2048 ) rot2.vx += 4096 ;
			rot2.vx = rot2.vx - rot.vx ;
			//	    rot2.vx += 96 ;
			GroundRotAdjust( m_ctrl, adj, &rot2, 2 ) ;
		}
		GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 13 ) ;
		GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 17 ) ;	
    } else {
		/* 足の方が上 */
		gik_printf( "ashi ue 2\n" ) ;
        foot.vy = levels2[ 0 ] + height ;
        vec.vx = from.vy - foot.vy ;
		vec.vz = GV_VecLen3F2( &from, &foot ) ;
		if ( dir == DOWN_BACK ) {
			rot.vx = GV_VecDir2( &vec ) ;
			if ( rot.vx < -2048 ) rot.vx += 4096 ;
			rot2.vy = rot2.vz = 0 ;
			rot2.vx = rot.vx - g_rot ;
			GroundRotAdjust( m_ctrl, adj, &rot2, 0 ) ;	    
		} else {
			rot.vx = - GV_VecDir2( &vec ) ;
			if ( rot.vx < -2048 ) rot.vx += 4096 ;
			rot.vx += 128 ;
			rot2.vy = rot2.vz = 0 ;
			rot2.vx = rot.vx - g_rot ;
			GroundRotAdjust( m_ctrl, adj, &rot2, 0 ) ;
			hand.vy = levels1[ 0 ] + height ;
			vec.vx = hand.vy - from.vy ;
			vec.vz = GV_VecLen3F2( &from, &hand ) ;
			rot2.vx = - GV_VecDir2( &vec ) ;
			if ( rot2.vx < -2048 ) rot2.vx += 4096 ;
			rot2.vx = rot2.vx - rot.vx ;
			rot2.vx -= 64 ;
			GroundRotAdjust( m_ctrl, adj, &rot2, 2 ) ;
		}
		GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 13 ) ;	
		GroundRotAdjust( m_ctrl, adj, &DG_ZeroSVector, 17 ) ;	
    }
}
#endif

/* 匍匐ＩＫ２ 傾き処理も一緒にやる */

#define	GIK2_HEAD	(650.0F)
#define	GIK2_FOOT	(850.0F)
#define	GIK2_FOOT2	(450.0F)
static	void	GroundIK2( Work *work, float height )
{
	int				c1, c2, step, mode ;
	int				down, atr, atrs[ 2 ] ;
	HZX_GROUP_ID	hzx_id ;
	float			level0, level1, level2, levels[ 2 ] ;
	float			height_adjust, front, back ;
	float			gik2_foot, len ;
	SVECTOR			rot, rot2 ;
	FVECTOR			from, to ;
	HZX_FLR			flrs[ 2 ] ;
	MOTION_CONTROL	*m_ctrl ;
    SVECTOR			*adj ;

	/*主観移動中は階段以外の場所ではやらない*/
	if ( PL_SubjectMove && !(work->control.flr_atrs[0] & HZX_FLOOR_STEP) ) {
		return ;
	}

	step = 0 ;
    m_ctrl = work->body.m_ctrl ;
	adj = work->rot_adjusts ;
    down = work->down_dir ;
	hzx_id = work->control.hzx_id ;
	rot = work->control.rot ; rot.vx = rot.vz = 0 ;
	level0 = work->control.levels[ 0 ] ;
	height_adjust = 0.0F ;

	GetModelLength( work, &front, &back ) ;
	work->g_rot = 0 ;

	if ( front + back > 1200.0F ) mode = 0 ;
	else if ( work->body.height < height ) mode = 1 ;
	else goto gik2_abnormal_end ;

	if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
		step = 1 ;
		height_adjust = StepHeightAdjust( work->control.level[ 0 ], level0 ) ;
	}
	/* まず頭腰間チェック */
	if ( down == DOWN_BACK ) rot.vy += 2048 ;
	DG_COPY_VEC( &from, &work->control.mov ) ;
	DG_SetPos2( &from, &rot ) ;
	to.vx = to.vy = 0.0F ; 
	to.vz = GIK2_HEAD ;
	DG_PutVector( &to, &to, 1 ) ;
//	c1 = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
//							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
#if 0
	c1 = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_FLOOR, 0, HZX_FLOOR_NO_PLAYER ) ;
#else
	c1 = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_ALL, 
							    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ;
	if ( c1 ) {
		if ( HZX_GetOnlineHazardType() == HZX_ONLINE_TYPE_SEGMENT ) {
			c1 = 0 ;
			HZX_GetOnlineVector( &to ) ;
			len = GV_VecLen3F( &to ) ;
			/* ちょっと手前にする */
			if ( len > 2.0F ) GV_LenVec3F( &to, &to, len, len - 2.0F ) ;
			_sceVu0AddVector( &to, &from, &to ) ;
		}
	}
#endif
	if ( c1 ) {
		/* toの位置で天井チェック */
		c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL, 
								   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
		if ( !( c2 & 2 ) ) {
			/* 天井がない */
//			printf( "groun ik 2 - 1 : no ceil\n" ) ;
            goto gik2_abnormal_end ;
		}
	} else {
		/* toの位置で床チェック */
		c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL | HZX_CHK_NOCHECK_CEIL, 
								   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
		if ( !( c2 & 1 ) ) {
			/* 床がない */
//			printf( "groun ik 2 - 1 : no floor\n" ) ;
            goto gik2_abnormal_end ;
		}
	}
	HZX_GetLevelHazard( flrs, atrs ) ;
	HZX_GetLevelHeight( levels ) ;
	level1 = ( c1 ) ? levels[ 1 ] : levels[ 0 ] ;
	atr = ( c1 ) ? atrs[ 1 ] : atrs[ 0 ] ;
	if ( atr & HZX_FLOOR_STEP ) {
		step |= 2 ;
		level1 += StepHeightAdjust( ( c1 ) ? flrs + 1 : flrs, level1 ) ;
		if ( ( step & 1 ) == 0 ) height_adjust = 125.0F ;
	}
	
	/* 腰足チェック */
	to.vx = to.vy = 0.0F ; 
	to.vz = -GIK2_FOOT ;
	DG_PutVector( &to, &to, 1 ) ;
//	c1 = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
//							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
#if 0
	c1 = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_FLOOR, 0, HZX_FLOOR_NO_PLAYER ) ;
#else
	c1 = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_ALL, 
							    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ;

	if ( c1 ) {
		if ( HZX_GetOnlineHazardType() == HZX_ONLINE_TYPE_SEGMENT ) {
			c1 = 0 ;
			HZX_GetOnlineVector( &to ) ;
			len = GV_VecLen3F( &to ) ;
			if ( len > 2.0F ) GV_LenVec3F( &to, &to, len, len - 2.0F ) ;
			_sceVu0AddVector( &to, &from, &to ) ;
		}
	}
#endif
	gik2_foot = GIK2_FOOT ;
	if ( c1 ) {
#if 1
		float		len ;
		FVECTOR		vect ;
		
		HZX_GetOnlineVector( &vect ) ;
		len = GV_VecLen3F( &vect ) ;
		if ( len < GIK2_FOOT2 ) {
			to.vx = to.vy = 0.0F ; 
			to.vz = -GIK2_FOOT2 ;
			DG_PutVector( &to, &to, 1 ) ;			
			gik2_foot = GIK2_FOOT2 ;
		}
#endif
		/* toの位置で天井チェック */
		c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL, 
								   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
		if ( !( c2 & 2 ) ) {
			/* 天井がない */
//			printf( "groun ik 2 - 2 : no ceil\n" ) ;
            goto gik2_abnormal_end ;
		}
	} else {
		/* toの位置で床チェック */
		c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL,
								   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
		if ( !( c2 & 1 ) ) {
			/* 床がない */
//			printf( "groun ik 2 - 2 : no floor\n" ) ;
            goto gik2_abnormal_end ;
		}
	}
	HZX_GetLevelHazard( flrs, atrs ) ;
	HZX_GetLevelHeight( levels ) ;
	level2 = ( c1 ) ? levels[ 1 ] : levels[ 0 ] ;
	atr = ( c1 ) ? atrs[ 1 ] : atrs[ 0 ] ;
	if ( atr & HZX_FLOOR_STEP ) {
		step |= 4 ;
		level2 += StepHeightAdjust( ( c1 ) ? flrs + 1 : flrs, level1 ) ;
		if ( ( step & 1 ) == 0 ) height_adjust = 125.0F ;
	}

	if ( step ) {
		level0 += height_adjust ;
		SetHeightAdjust2( work, ( int )height_adjust ) ;
	} 
	rot = rot2 = DG_ZeroSVector ;
	if ( DG_FABS( level1 - level0 ) < 1000.0F || 
		( ( step & 3 ) && DG_FABS( level1 - level0 ) < 4000.0F ) ) {
		rot.vx = -( short )( atan2f( ( level1 - level0 ), GIK2_HEAD ) 
							* 2048.0F / ( float )PI ) ;
		if ( down == DOWN_BACK ) rot.vx = - rot.vx ;
		//printf( "%f %f : ", level1, level0 ) ;
		//DumpSVec( &rot ) ;
	}
	SetRotAdjust( work, &rot, HUMAN21_KOSHI ) ;

	if ( mode == 0 ) {
		if ( DG_FABS( level2 - level0 ) < 1000.0F || 
			( ( step & 5 ) && DG_FABS( level2 - level0 ) < 4000.0F ) ) {
			rot2.vx = ( short )( atan2f( ( level2 - level0 ), gik2_foot ) 
								* 2048.0F / ( float )PI ) ;
			if ( down == DOWN_BACK ) rot2.vx = - rot2.vx ;
			rot2.vx -= rot.vx ;
		}
		SetRotAdjust( work, &rot2, HUMAN21_MIGI_ASHI1 ) ;
		SetRotAdjust( work, &rot2, HUMAN21_HIDARI_ASHI1 ) ;
	} 
//	work->g_rot = adj[ HUMAN21_KOSHI ].vx ;
	work->g_rot = work->turn_adjusts[ HUMAN21_KOSHI ].vx ;
	return ;
gik2_abnormal_end :
    ;	
}

/* ダンボール用傾け処理 */
#define	CBBI_FRONT	(250.0F)
#define	CBBI_BACK	(250.0F)
static	int		CB_BoxIncline( work )
Work			*work ;
{
	int				c1, c2, step ;
	int				atr, atrs[ 2 ] ;
	HZX_GROUP_ID	hzx_id ;
	float			level1, level2, levels[ 2 ] ;
	float			height_adjust, diff ;
	SVECTOR			rot ;
	FVECTOR			from, to ;
	HZX_FLR			flrs[ 2 ] ;
	MOTION_CONTROL	*m_ctrl ;
    SVECTOR			*adj ;

	step = 0 ;
	work->g_rot = 0 ;
    m_ctrl = work->body.m_ctrl ;
	adj = work->rot_adjusts ;
	hzx_id = work->control.hzx_id ;
	rot = work->control.rot ; rot.vx = rot.vz = 0 ;
	height_adjust = 0.0F ;
	if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
		step = 1 ;
		height_adjust = StepHeightAdjust( work->control.level[ 0 ], 
										  work->control.levels[ 0 ] ) ;
	}
	DG_COPY_VEC( &from, &work->control.mov ) ;
	DG_SetPos2( &from, &rot ) ;
	to.vx = to.vy = 0.0F ; 
	to.vz = CBBI_FRONT ;
	DG_PutVector( &to, &to, 1 ) ;
	c1 = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
	if ( c1 ) {
		/* toの位置で天井チェック */
		c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL, 
								   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
		if ( !( c2 & 2 ) ) {
			/* 天井がない */
            goto cbbi_abnormal_end ;
		}
	} else {
		/* toの位置で床チェック */
		c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL | HZX_CHK_NOCHECK_CEIL, 
								   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
		if ( !( c2 & 1 ) ) {
			/* 床がない */
            goto cbbi_abnormal_end ;
		}
	}
	HZX_GetLevelHazard( flrs, atrs ) ;
	HZX_GetLevelHeight( levels ) ;
	level1 = ( c1 ) ? levels[ 1 ] : levels[ 0 ] ;
	atr = ( c1 ) ? atrs[ 1 ] : atrs[ 0 ] ;
	if ( atr & HZX_FLOOR_STEP ) {
#if 0
		step |= 2 ;
		level1 += StepHeightAdjust( ( c1 ) ? flrs + 1 : flrs, level1 ) ;
//		if ( ( step & 1 ) == 0 ) height_adjust = 125.0F ;
#else 
		return 2 ; /* 階段近く */
#endif
	}
	
	/* 腰足チェック */
	to.vx = to.vy = 0.0F ; 
	to.vz = - CBBI_BACK ;
	DG_PutVector( &to, &to, 1 ) ;
	c1 = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
							    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
	if ( c1 ) {
		/* toの位置で天井チェック */
		c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL, 
								   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
		if ( !( c2 & 2 ) ) {
			/* 天井がない */
            goto cbbi_abnormal_end ;
		}
	} else {
		/* toの位置で床チェック */
		c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL,
								   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
		if ( !( c2 & 1 ) ) {
			/* 床がない */
            goto cbbi_abnormal_end ;
		}
	}
	HZX_GetLevelHazard( flrs, atrs ) ;
	HZX_GetLevelHeight( levels ) ;
	level2 = ( c1 ) ? levels[ 1 ] : levels[ 0 ] ;
	atr = ( c1 ) ? atrs[ 1 ] : atrs[ 0 ] ;
	if ( atr & HZX_FLOOR_STEP ) {
#if 0
		step |= 4 ;
		level2 += StepHeightAdjust( ( c1 ) ? flrs + 1 : flrs, level1 ) ;
//		if ( ( step & 1 ) == 0 ) height_adjust = 125.0F ;
#else
		return 2 ; /* 階段近く */
#endif
	}
	rot = DG_ZeroSVector ;
	diff = DG_FABS( level2 - level1 ) ;
	if ( diff > 0.10F && diff < 1000.0F ) {
		rot.vx = ( short )( atan2f( ( level2 - level1 ), CBBI_FRONT + CBBI_BACK ) 
							* 2048.0F / ( float )PI ) ;
	}
	if ( step ) {
		SetHeightAdjust2( work, ( int )height_adjust ) ;
	} 
	SetRotAdjust( work, &rot, HUMAN21_KOSHI ) ;
	work->g_rot = adj[ HUMAN21_KOSHI ].vx ;
	return 0 ;
cbbi_abnormal_end :
	return 1 ;
}

#ifdef DEBUG_MODE
/* アジャスト君 */
static	void	MrAdjust( work )
Work		*work ;
{
    static int	OnOff = 0 ;
    static int	Object = 0 ;
    static SVECTOR	Adj[ 21 ] ;
    GV_PAD	*pad ;
    MOTION_CONTROL	*m_ctrl ;
    FVECTOR		quat ;
    int			i ;

    return ;

    if ( Status( PLAYER_FORCE | PLAYER_BEYOND ) ) return ;

    pad = GV_PadData ;

    if ( ( pad->status & PAD_L1 ) &&
		( pad->press & PAD_Y ) ) {
		OnOff = 1 - OnOff ;
    }

    if ( OnOff == 0 ) {
		UnsetStatus( PLAYER_PAD_OFF ) ;
		work->body.m_ctrl->adjust_flag = 0 ;
		for ( i = 0; i < 21; i ++ ) Adj[ i ] = DG_ZeroSVector ;
		work->control.skip_flag &= ~CTRL_SKIP_HZX ;
		return ;
    }

    work->control.skip_flag |= CTRL_SKIP_HZX ;
    SetStatus( PLAYER_PAD_OFF ) ;
    MENU_Locate( 260, 160, 0 ) ;
    MENU_Printf( "object %d : %d\n", Object, Adj[ Object ].vx ) ;
    
    if ( pad->press & PAD_R ) Object ++ ;
    if ( pad->press & PAD_L ) Object -- ;
    if ( pad->status & PAD_U ) Adj[ Object ].vx += 8 ;
    if ( pad->status & PAD_D ) Adj[ Object ].vx -= 8 ;
    if ( Object >= 21 ) Object = 0 ;
    if ( Object < 0 ) Object = 20 ;

    if ( pad->status & PAD_X ) work->control.mov.vy += 16 ;
    if ( pad->status & PAD_B ) work->control.mov.vy -= 16 ;

    m_ctrl = work->body.m_ctrl ;
    for ( i = 0; i < 21; i ++ ) {
		GM_RotToQuatXAfterY( &Adj[ i ], &quat ) ;
		m_ctrl->adjust[ i ] = quat ;
		m_ctrl->adjust_flag |= 1 << i ;
    }
}
#endif


