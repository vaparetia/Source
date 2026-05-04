/*
   wt_door2.c
   水密ドアモーションバージョン

   2000/07/19	M.Sonoyama
   $Id: wt_door2.c,v 1.1.1.3 2002/11/19 11:50:52 Yoshizawa1 Exp $
*/

/*-------- 注意 --------

   ・モーションで開いたドアを閉めることはできない 
   ・最初から開いているドアはメッセージで閉めることができる 
*/

#define	BODY_FLAG2	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT | \
					 DG_FLAG_FINISHCALC)

static	int	MotionExist = 0 ;
static	int	PlayerMotion = 0 ;
static	int	DoorMotion = 0 ;
static	int	HandleMotion = 0 ;

static	int	RotateFast = 0 ;

static	int	SeNo ;

enum {
	PMloop = 0,
	PMloop_rev,
	PMr_pull,
	PMr_push,
	PMl_pull,
	PMl_push,
	PMnot_open,
	MAX_PLAYER_HATCH_MOTIONS
} ;

enum {
	DMr_pull = 0,
	DMr_push,
	DMl_pull,
	DMl_push,
	MAX_HATCH_DOOR_MOTIONS
} ;

enum {
	HMloop = 0,
	HMloop_rev,
	HMr_pull,
	HMr_push,
	HMl_pull,
	HMl_push,
	HMnot_open,
	HMloop2,
	HMloop2_rev,
	HMloop2_180,
	MAX_HATCH_HANDLE_MOTIONS
} ;

#define	SLEEP_MOT( _m )		((_m)->mt3_ctrl[ 0 ].flag |= MT3_SLEEP)
#define	ACTIVE_MOT( _m )	((_m)->mt3_ctrl[ 0 ].flag &= ~MT3_SLEEP)

#define	QUICK	(10)
#define	FAST	(2.00F)

static	void	WtDoorDmgCallback( PlayerWork *work, TARGET *off, TARGET *def ) ;

/*----------------------------------------------------------------*/

/* 壁の回転 */
static	void	RotateSegment2( work )
Work			*work ;
{
	IVECTOR		pole ;
	FMATRIX		*world ;

	world = &work->body_door.objs->objs[ 0 ].world ;
	pole.vx = world->m[ 3 ][ 0 ] ;
	pole.vy = world->m[ 3 ][ 1 ] ;
	pole.vz = world->m[ 3 ][ 2 ] ;
	HZX_RotateDynamicSegment2( work->segment, &pole, world ) ;
	HZX_RotateDynamicSegment2( work->segment2, &pole, world ) ;
}

static	void		MatrixRotZYX( mat, rot )
FMATRIX				*mat ;
SVECTOR				*rot ;
{
	float			sx, cx, sy, cy, sz, cz ;
	float			rx, ry, rz ;

	sy = -mat->m[ 0 ][ 2 ] ;
	cy = sqrtf( 1.0F * 1.0F - sy * sy ) ;
	if ( cy < 0.00001F ) {
		ry = ( sy > 0.0F ) ? PI / 2.0F : -PI / 2.0F ;
		rx = atan2f( - mat->m[ 2 ][ 1 ], mat->m[ 1 ][ 1 ] ) ;
		rz = 0.0F ;
	} else {
		ry = atan2f( sy, cy ) ;
		sx = mat->m[ 1 ][ 2 ] / cy ;
		cx = mat->m[ 2 ][ 2 ] / cy ;
		rx = atan2f( sx, cx ) ;
		sz = mat->m[ 0 ][ 1 ] / cy ;
		cz = mat->m[ 0 ][ 0 ] / cy ;
		rz = atan2f( sz, cz ) ;
	}
	rot->vx = ( int )( rx * 2048.0F / PI ) & 4095 ;
	rot->vy = ( int )( ry * 2048.0F / PI ) & 4095 ;
	rot->vz = ( int )( rz * 2048.0F / PI ) & 4095 ;
//	if ( rot->vx >= 2048 ) rot->vx -= 4096 ;
//	if ( rot->vy >= 2048 ) rot->vy -= 4096 ;
//	if ( rot->vz >= 2048 ) rot->vz -= 4096 ;
}

static	void	GetHandleRotate( bh, rot )
OBJECT			*bh ;
SVECTOR			*rot ;
{
	FMATRIX		m1, m2 ;
		
	DG_COPY_MAT( &m1, &bh->objs->world ) ;
	DG_COPY_MAT( &m2, &bh->objs->objs[ 0 ].world ) ;
	_sceVu0InversMatrix( &m1, &m1 ) ;
	_sceVu0MulMatrix( &m1, &m1, &m2 ) ;
	MatrixRotZYX( &m1, rot ) ;
}

static	void	NearSpeed( from, to, v )
short			*from, *to ;
int				v ;
{
	if ( GV_DiffDirAbs( *from + v, *to ) <= ( int )fabsf( ( float )v ) ) {
		*from = *to ;
	} else {
		*from = *from + v ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act2( work )
Work			*work ;
{
	int			n_msg, code ;
	int			buf[ 4 ], i ;
	GV_MSG		*msg, Msg ;
	OBJECT		*bd, *bh ;

	/* 水中でははりつかない属性を切る */
	if ( work->body_handle.objs->world.m[ 3 ][ 1 ] < GM_WaterLevel ) {
		HZX_DynamicSegmentSetAttribute( work->segment3, 
									    work->segment3->atr & ~HZX_SEG_NO_HARITSUKI ) ;
	}

	/* 赤外線ゴーグルでハンドル光る */
	if ( PL_GetPlayerItem() == IT_Thermal ) {
		work->body_handle.objs->flag &= ~( DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT ) ;
		work->body_handle.objs->flag |= DG_FLAG_IRREACTION ;
		for ( i = 0; i < work->body_handle.objs->n_models; i ++ ) {
			work->body_handle.objs->objs[ i ].flag &= ~( DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT ) ;
			work->body_handle.objs->objs[ i ].flag |= DG_FLAG_IRREACTION ;
		}
	} else {
		work->body_handle.objs->flag |= ( DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT ) ;
		work->body_handle.objs->flag &= ~DG_FLAG_IRREACTION ;
		for ( i = 0; i < work->body_handle.objs->n_models; i ++ ) {
			work->body_handle.objs->objs[ i ].flag |= ( DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT ) ;
			work->body_handle.objs->objs[ i ].flag &= ~DG_FLAG_IRREACTION ;
		}
	}


	/* メッセージ受信 */
	if ( MsgBuf[ 7 ] == work->name ) {
		MsgBuf[ 7 ] = -1 ;
		n_msg = 1 ;
		msg = &Msg ;
		Msg.message = MsgBuf ;
		Msg.message_len = 7 ;
	} else {
		n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	}
	bd = &work->body_door ;
	bh = &work->body_handle ;
	while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
		case MSG_HOLD :		/* つかみまわし開始 */
			if ( work->status == STATE_CLOSE ) {
				work->status = STATE_WORKING ;
				work->chara = msg->message[ 1 ] ;
				work->door_motion = -1 ;
				work->handle_motion = msg->message[ 2 ] ;
				work->handle_time = 0.0F ;
				ACTIVE_MOT( bh->m_ctrl ) ;
				GM_ConfigObjectAction( bh, 0, work->handle_motion, 0, 0xfffff, 0 ) ;				

				if ( msg->message_len >= 7 && msg->message[ 6 ] != 0 ) {
					work->motion_speed = ( float )TIME_BASE / ( float )msg->message[ 6 ] ;
				} else {
					work->motion_speed = ( float )TIME_BASE ;
				}
				MT_SetMotionSpeed( bh->m_ctrl, work->motion_speed ) ;

				buf[ 0 ] = EXEC_TRIG_HOLD_START ;
				buf[ 1 ] = work->chara ;
				ExecProc( work, buf, 2 ) ;
				work->flag |= WT_DOOR_FLAG_FIRST_HANDLE_ROT ;
				work->flag |= WT_DOOR_FLAG_HANDLE_ROTATE_DIR_EXPR ;
			}
			break ;
		case MSG_HOLD_END :	/* つかみ離し */
			if ( work->status == STATE_WORKING ) {
				work->status = STATE_CLOSE ;
				buf[ 0 ] = EXEC_TRIG_HOLD_RELEASE ;
				buf[ 1 ] = work->chara ;
				ExecProc( work, buf, 2 ) ;				
				if ( work->handle_time > 0.0F && 
					!( work->flag & WT_DOOR_FLAG_HANDLE_ROTATE_DIR_EXPR ) ) {
					work->status = STATE_HANDLE_RETURNING ;
					/* 何度回転したかを求める */
					GetHandleRotate( bh, &work->handle_current_rot ) ;
					/* 離したときに、スタートと回転が同じなら
					   一周したとみなす */
					if ( work->handle_current_rot.vx == work->handle_start_rot.vx &&
						 work->handle_rotate_dir.vx != 0 ) {
						work->handle_current_rot.vx += 4096 ;
					}
					if ( work->handle_current_rot.vz == work->handle_start_rot.vz &&
						 work->handle_rotate_dir.vz != 0 ) {
						work->handle_current_rot.vz += 4096 ;
					}
					/* モーションは最初のフレームに戻す */
					GM_ConfigObjectAction( bh, 0, work->handle_motion, 0, 0xfffff, 0 ) ;
				} else {
					SLEEP_MOT( bh->m_ctrl ) ;
					work->handle_motion = -1 ;
				}
			}
			break ;
		case MSG_OPEN_START :	/* ドア開き開始 */
			if ( work->status == STATE_WORKING ) {
				work->handle_motion = -1 ;
				SLEEP_MOT( bh->m_ctrl ) ;
				work->door_motion = msg->message[ 2 ] ;
				ACTIVE_MOT( bd->m_ctrl ) ;
				GM_ConfigObjectAction( bd, 0, work->door_motion, 0, 0xfffff, 100 ) ;

				if ( msg->message_len >= 7 && msg->message[ 6 ] != 0 ) {
					work->motion_speed = ( float )TIME_BASE / ( float )msg->message[ 6 ] ;
				} else {
					work->motion_speed = ( float )TIME_BASE ;
				}
				MT_SetMotionSpeed( bd->m_ctrl, work->motion_speed ) ;

				buf[ 0 ] = EXEC_TRIG_OPEN_START ;
				buf[ 1 ] = msg->message[ 1 ] ;
				ExecProc( work, buf, 2 ) ;
				/* 開き開始とともにsegment3は消える */
				HZX_DynamicSegmentSetSkip( work->segment3 ) ;
			}			
			break ;
		case MSG_OPEN_END :		/* ドア開き終了 */
			if ( work->status == STATE_WORKING ) {
				work->status = STATE_OPEN ;
				buf[ 0 ] = EXEC_TRIG_OPEN_END ;
				buf[ 1 ] = work->chara ;
				ExecProc( work, buf, 2 ) ;
				/* インタラプトリセット */
				work->r_intrpt.status &= ~( ROOT_INTRPT_CLOSE | ROOT_INTRPT_NOENE ) ;
			}	
			break ;
		case MSG_NOT_OPEN :		/* 開かない */
			if ( work->status == STATE_CLOSE ) {
				work->status = STATE_NOT_OPEN_WORKING ;
				work->chara = msg->message[ 1 ] ;
				work->door_motion = -1 ;
				work->handle_motion = msg->message[ 2 ] ;
				ACTIVE_MOT( bh->m_ctrl ) ;
				GM_ConfigObjectAction( bh, 0, work->handle_motion, 0, 0xfffff, 0 ) ;

				if ( msg->message_len >= 7 && msg->message[ 6 ] != 0 ) {
					work->motion_speed = ( float )TIME_BASE / ( float )msg->message[ 6 ] ;
				} else {
					work->motion_speed = ( float )TIME_BASE ;
				}
				MT_SetMotionSpeed( bh->m_ctrl, work->motion_speed ) ;

				buf[ 0 ] = EXEC_TRIG_HOLD_START ;
				buf[ 1 ] = work->chara ;
				ExecProc( work, buf, 2 ) ;
			}
			break ;
		case MSG_NOT_OPEN_END :	/* 開かない動作終了（モーション途中） */
			if ( work->status == STATE_NOT_OPEN_WORKING ) {
				work->status = STATE_CLOSE ;
				work->handle_motion = -1 ;
				SLEEP_MOT( bh->m_ctrl ) ;
				buf[ 0 ] = EXEC_TRIG_HOLD_RELEASE ;
				buf[ 1 ] = work->chara ;
				ExecProc( work, buf, 2 ) ;
			}
			break ;
		case MSG_CLOSE :  		/* 開いているドアを閉める */
			if ( work->status == STATE_OPEN ) {
				if ( !( work->flag & WT_DOOR_FLAG_EXIST_NOT_MOTION_ROTATE ) ) {
					SVECTOR		rotate = { 0, 0, 0 } ;
					if ( work->type == 0 ) rotate.vy = -1024 ;
					else 			 	   rotate.vy = 1024 ;
					work->rotate = rotate ;
					/* モーションを開く前にもどす */
					GM_ConfigObjectAction( bd, 0, DMr_pull, 0, 0xfffff, 0 ) ;
					SLEEP_MOT( bd->m_ctrl ) ;
					work->flag |= WT_DOOR_FLAG_EXIST_NOT_MOTION_ROTATE ;
				}
				work->status = STATE_CLOSING ;
				/* 閉まり始めと同時にsegment3効力発揮 */
				HZX_DynamicSegmentResetSkip( work->segment3 ) ;
				/* インタラプトセット */
				work->r_intrpt.status |= ROOT_INTRPT_CLOSE | ROOT_INTRPT_NOENE ;
			}
			break ;
		case MSG_HANDLE_VISIBLE :	/* ハンドルの表示／非表示 */
			if ( msg->message[ 1 ] == 0 ||
				 msg->message[ 1 ] == GV_StrCode( "off" ) ) {
				DG_InvisibleObjs( bh->objs ) ;
			} else {
				DG_VisibleObjs( bh->objs ) ;
			}
			break ;
		case MSG_OPEN :			/* シナリオで開ける（モーション不使用） */
			if ( work->status == STATE_CLOSE ) {
				work->status = STATE_OPENING ;
				buf[ 0 ] = EXEC_TRIG_OPEN_START ;
				buf[ 1 ] = 0 ;
				ExecProc( work, buf, 2 ) ;
				/* 開き開始とともにsegment3は消える */
				HZX_DynamicSegmentSetSkip( work->segment3 ) ;
				work->rotate.vy = 0 ;
			}
			break ;
		default :
		  ;
		}
		msg ++ ;
	}

	/* 開かない動作終了（モーション終了） */
	if ( work->status == STATE_NOT_OPEN_WORKING &&
		 MT_CHECK_LAST1( bh->m_ctrl, 0 ) ) {
		work->status = STATE_CLOSE ;
		work->handle_motion = -1 ;
		SLEEP_MOT( bh->m_ctrl ) ;
		buf[ 0 ] = EXEC_TRIG_HOLD_RELEASE ;
		buf[ 1 ] = work->chara ;
		ExecProc( work, buf, 2 ) ;						
	}

	GM_SetCurrentMap( work->map ) ;
	GM_ActMotion( bd ) ;

	DG_SetPos( &bd->objs->world ) ;
	GM_ActObject2( bd ) ;

	if ( MT_CHECK_LAST1( bd->m_ctrl, 0 ) ) {
		work->door_motion = -1 ;
		SLEEP_MOT( bd->m_ctrl ) ;
	}

	/* 閉まる１ */
	if ( work->status == STATE_CLOSING ) {
		GV_NearSpeedPV( &work->rotate, &DG_ZeroSVector, 3, 24 ) ;
		work->flag |= WT_DOOR_FLAG_CHANGE_NOT_MOTION_ROTATE ;
	} else if ( work->status == STATE_OPENING ) {
		SVECTOR			rot ;
		/* シナリオで開ける１ */
		rot.vx = rot.vz = 0 ;
		if ( work->type == 0 ) rot.vy = -1024 ;
		else 			 	   rot.vy = 1024 ;
		GV_NearSpeedPV( &work->rotate, &rot, 3, 24 ) ;
		work->flag |= WT_DOOR_FLAG_CHANGE_NOT_MOTION_ROTATE |
			          WT_DOOR_FLAG_EXIST_NOT_MOTION_ROTATE ;
	}

	/* モーション以外での回転設定 */
	if ( work->flag & WT_DOOR_FLAG_EXIST_NOT_MOTION_ROTATE ) {
		FMATRIX		m1 ;

		DG_SetPos2( &DG_ZeroVector, &work->rotate ) ;
		DG_GetPos( &m1 ) ;
		_sceVu0MulMatrix( &work->body_door.objs->objs[ 0 ].world, 
						  &work->body_door.objs->objs[ 0 ].world, &m1 ) ;
	}

	if ( work->door_motion != -1 ||
		 ( work->flag & WT_DOOR_FLAG_CHANGE_NOT_MOTION_ROTATE ) ) RotateSegment2( work ) ;

//	HZX_ViewDynamicSegment( work->segment ) ;
//	HZX_ViewDynamicSegment( work->segment2 ) ;
//	HZX_ViewDynamicSegment( work->segment3 ) ;

	/* 閉まる２ */
	if ( work->status == STATE_CLOSING ) {
		if ( work->rotate.vx == 0 &&
			 work->rotate.vy == 0 &&
			 work->rotate.vz == 0 ) {
			work->flag &= ~( WT_DOOR_FLAG_CHANGE_NOT_MOTION_ROTATE |
							 WT_DOOR_FLAG_EXIST_NOT_MOTION_ROTATE ) ;
			work->status = STATE_CLOSE ;
			GM_SeSetMode( SD_A_MITCLO01, ( FVECTOR * )bh->objs->world.m[ 3 ], GM_SEMODE_NORMAL ) ;
			buf[ 0 ] = EXEC_TRIG_CLOSE_END ;
			buf[ 1 ] = work->chara ;
			ExecProc( work, buf, 2 ) ;
			/* segment3効力発揮 */
			HZX_DynamicSegmentResetSkip( work->segment3 ) ;
		}
	} else if ( work->status == STATE_OPENING ) {
		/* シナリオで開ける２ */
		if ( ( work->type == 0 && work->rotate.vy <= -1024 ) ||
			 ( work->type == 1 && work->rotate.vy >= 1024 ) ) {
			work->flag &= ~WT_DOOR_FLAG_CHANGE_NOT_MOTION_ROTATE ;
			work->status = STATE_OPEN ;
			buf[ 0 ] = EXEC_TRIG_OPEN_END ;
			buf[ 1 ] = 0 ;
			ExecProc( work, buf, 2 ) ;
		}
	}

	if ( RotateFast ) {
		MT_SetMotionSpeed( bh->m_ctrl, work->motion_speed * FAST ) ;
	} else {
		MT_SetMotionSpeed( bh->m_ctrl, work->motion_speed ) ;
	}

	GM_ActMotion( bh ) ;
	/* 左右反転 */
	if ( ( ( work->handle_motion == HMloop2_180 &&  work->type == 0 ) ||
		   ( work->handle_motion == HMloop2_rev &&  work->type == 1 ) ) &&
		 !( bh->m_ctrl->mt3_ctrl[ 0 ].flag & MT3_SLEEP ) ) {
		bh->m_ctrl->abs_rots[ 0 ].vx *= -1 ;
		bh->m_ctrl->abs_rots[ 0 ].vw *= -1 ;
	}
	DG_SetPos( &bd->objs->objs[ 0 ].world ) ;
	DG_MovePos( &work->shift ) ;
#if 0
	if ( work->type == 1 ) {
		SVECTOR		r2 = { 0, 2048, 0 } ;

		DG_RotatePos( &r2 ) ;
	}
#endif
	GM_ActObject2( bh ) ;

	if ( work->status != STATE_HANDLE_RETURNING ) {
		work->handle_time = ( float )( bh->m_ctrl->mt3_ctrl->play_time /
									   bh->m_ctrl->mt3_ctrl->motion_time_base ) ;
		/* ハンドルの回転方向を求める */
		if ( work->handle_time >= 2.50F &&
			 ( work->flag & WT_DOOR_FLAG_HANDLE_ROTATE_DIR_EXPR ) &&
			 !( work->flag & WT_DOOR_FLAG_FIRST_HANDLE_ROT ) ) {
			SVECTOR		rot ;
			int			d1, d2 ;

			GetHandleRotate( bh, &rot ) ;
			if ( ( rot.vx & 4095 ) != work->handle_start_rot.vx ||
				 ( rot.vz & 4095 ) != work->handle_start_rot.vz ) {
				d1 = GV_DiffDirS( work->handle_start_rot.vx, rot.vx ) ;
				d2 = GV_DiffDirS( work->handle_start_rot.vz, rot.vz ) ;
				work->handle_rotate_dir.vx = ( d1 > 0 ) ? 1 : -1 ;
				if ( d1 == 0 ) work->handle_rotate_dir.vx = 0 ;
				work->handle_rotate_dir.vz = ( d2 > 0 ) ? 1 : -1 ;
				if ( d2 == 0 ) work->handle_rotate_dir.vz = 0 ;
				work->flag &= ~WT_DOOR_FLAG_HANDLE_ROTATE_DIR_EXPR ;
			}
		}
	} else {	/* ハンドル戻り中 */
		FMATRIX		m1 ;

		SLEEP_MOT( bh->m_ctrl ) ; /* 最初のフレームに戻してから止める */
//		GV_NearSpeedV( &work->handle_current_rot, &work->handle_start_rot, 32, 3 ) ;
		if ( work->handle_rotate_dir.vx != 0 ) {
			NearSpeed( &work->handle_current_rot.vx, &work->handle_start_rot.vx, 
					   -64 * work->handle_rotate_dir.vx ) ;
		} else {
			work->handle_current_rot.vx = work->handle_start_rot.vx ;
		}
		if ( work->handle_rotate_dir.vz != 0 ) {
			NearSpeed( &work->handle_current_rot.vz, &work->handle_start_rot.vz, 
					   -64 * work->handle_rotate_dir.vz ) ;
		} else {
			work->handle_current_rot.vz = work->handle_start_rot.vz ;
		}
		DG_SetPos( &DG_UnitMatrix ) ;
		DG_RotatePosZYX( &work->handle_current_rot ) ;
		DG_GetPos( &m1 ) ;
		_sceVu0MulMatrix( &m1, &bh->objs->world, &m1 ) ;
		DG_COPY_MAT( &bh->objs->objs[ 0 ].world, &m1 ) ;
		if ( ( work->handle_current_rot.vx & 4095 ) == ( work->handle_start_rot.vx & 4095 ) &&
			 ( work->handle_current_rot.vz & 4095 ) == ( work->handle_start_rot.vz & 4095 ) ) {
			work->handle_motion = -1 ;
			work->status = STATE_CLOSE ;
		}
	}
	/* 回転角度保存 */
	if ( work->flag & WT_DOOR_FLAG_FIRST_HANDLE_ROT ) {
		GetHandleRotate( bh, &work->handle_start_rot ) ;
		work->flag &= ~WT_DOOR_FLAG_FIRST_HANDLE_ROT ;
	}
	/* ハンドルが動いているときは常にプリシェード */
	if ( !( bh->m_ctrl->mt3_ctrl[ 0 ].flag & MT3_SLEEP ) ||
		 work->status == STATE_HANDLE_RETURNING ||
		 ( work->flag & WT_DOOR_FLAG_FIRST ) ) {
#if 0
		FMATRIX		buf, lit_back[ 2 ] ;
		LIT_DEF		*def ;
		def = bh->objs->fix_light ;
		if ( def != NULL ) {
			DG_COPY_MAT( &buf, &bh->objs->world ) ;
			DG_COPY_MAT( &bh->objs->world, &bh->objs->objs[ 0 ].world ) ;
			DG_COPY_MAT( &lit_back[ 0 ], &DG_LightMatrix ) ;
			DG_COPY_MAT( &lit_back[ 1 ], &DG_ColorMatrix ) ;
			DG_ColorMatrix.m[ 3 ][ 0 ] = ( float )def->ambient.r ;
			DG_ColorMatrix.m[ 3 ][ 1 ] = ( float )def->ambient.g ;
			DG_ColorMatrix.m[ 3 ][ 2 ] = ( float )def->ambient.b ;
			DG_LightMatrix.m[ 0 ][ 0 ] = def->dir.vx ;
			DG_LightMatrix.m[ 1 ][ 0 ] = def->dir.vy ;
			DG_LightMatrix.m[ 2 ][ 0 ] = def->dir.vz ;
			DG_LightMatrix.m[ 3 ][ 0 ] = def->dir.vw ;
			DG_ColorMatrix.m[ 0 ][ 0 ] = ( float )def->color.r ;
			DG_ColorMatrix.m[ 0 ][ 1 ] = ( float )def->color.g ;
			DG_ColorMatrix.m[ 0 ][ 2 ] = ( float )def->color.b ;			
			DG_MakePreshade( bh->objs, GM_GetMap( work->map )->light ) ;	
			DG_COPY_MAT( &bh->objs->world, &buf ) ;
			DG_COPY_MAT( &DG_LightMatrix, &lit_back[ 0 ] ) ;
			DG_COPY_MAT( &DG_ColorMatrix, &lit_back[ 1 ] ) ;
		}
#endif
		work->flag &= ~WT_DOOR_FLAG_FIRST ;
	}
}

static	void	Die2( work )
Work			*work ;
{
#ifdef KOREADD
	GM_FreeRouteIntrpt( &work->r_intrpt ) ;
#endif
	DG_FreePreshade( work->body_door.objs ) ;
	work->body_handle.objs->flag |= DG_FLAG_PAINT ;
	DG_FreePreshade( work->body_handle.objs ) ;
	GM_FreeObject( &work->body_door ) ;
	GM_FreeObject( &work->body_handle ) ;
	HZX_RemoveDynamicSegment( work->segment ) ;
	HZX_RemoveDynamicSegment( work->segment2 ) ;
	HZX_RemoveDynamicSegment( work->segment3 ) ;
}

/*----------------------------------------------------------------*/

static int	GetResources2( work, name, where )
Work		*work ;
int			name, where ;
{
	OBJECT			*body ;
	IVECTOR			iv ;
	FVECTOR			mov ;
	SVECTOR			rot ;
	HZX_GROUP_ID	hzx_id ;

	/* 名前 */
	work->map = where ;
	work->name = name ;
	/* ドアモデル */
	GCL_GetOption( 'k' ) ;
	body = &work->body_door ;
	GM_InitObject( body, GCL_GetNextInt(), BODY_FLAG2 ) ;
	ASSERT( body->objs != NULL ) ;
    GM_GroupObjs( body->objs, GM_CurrentMap ) ;
	body->map_name = GM_CurrentMap ;
	/* モーション */
	GM_ConfigObjectMotion( body, 1, DoorMotion, MT_FLAG_HUMAN2 ) ;
	GM_ConfigObjectStep( body, &work->door_step ) ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;
	SLEEP_MOT( body->m_ctrl ) ;
	
	/* ハンドルモデル */
	body = &work->body_handle ;
	GM_InitObject( body, GCL_GetNextInt(), BODY_FLAG2 ) ;
	ASSERT( body->objs != NULL ) ;
    GM_GroupObjs( body->objs, GM_CurrentMap ) ;
	body->map_name = GM_CurrentMap ;
	/* モーション */
	GM_ConfigObjectMotion( body, 1, HandleMotion, MT_FLAG_HUMAN2 ) ;
	GM_ConfigObjectStep( body, &work->handle_step ) ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;
	SLEEP_MOT( body->m_ctrl ) ;
	//GM_ActMotion( &work->body_handle ) ;

	/* タイプ */
	work->type = GCL_GetOptionValue( 't', 0 ) ;

	/* ドアモデル位置設定 */
	GCL_GetOption( 'r' ) ;
	GCL_GetNextIV( ( int * )&iv ) ;
	GV_ConvVec3( &iv, &rot ) ;
	GCL_GetOption( 'p' ) ;
	GCL_GetNextIV( ( int * )&iv ) ;
	GV_ConvVec3( &iv, &mov ) ;
	DG_SetPos2( &mov, &rot ) ;
	DG_PutObjs( work->body_door.objs ) ;
	DG_COPY_MAT( &work->body_door.objs->objs[ 0 ].world, &work->body_door.objs->world ) ;
	DG_MakePreshade( work->body_door.objs, GM_GetMap( where )->light ) ;
	work->def_rot = rot.vy ;

	/* ハンドルモデル位置設定 */
	if ( work->type == 0 ) {
		DG_COPY_VEC( &work->shift, &Shift ) ;
	} else {
		DG_COPY_VEC( &work->shift, &Shift2 ) ;
	}
	DG_MovePos( &work->shift ) ;
#if 0
	if ( work->type == 1 ) {
		SVECTOR		r2 ;

		r2 = rot ; r2.vy += 2048 ;
		DG_RotatePos( &r2 ) ;
	}
#endif
	{
		//FVECTOR	tmpMat ;

		//GM_ActObject2( &work->body_handle ) ;
		//DG_COPY_MAT( &tmpMat, &work->body_handle.objs->world ) ;
		//DG_COPY_MAT( &work->body_handle.objs->world, &work->body_handle.objs->objs[ 0 ].world ) ;
		DG_PutObjs( work->body_handle.objs ) ;
		DG_COPY_MAT( &work->body_handle.objs->objs[ 0 ].world, &work->body_handle.objs->world ) ;
		DG_MakePreshade( work->body_handle.objs, GM_GetMap( where )->light ) ;	
		//DG_COPY_MAT( &work->body_handle.objs->world, &tmpMat ) ;
	}


	/* 壁 */
	{
		IVECTOR		ip[ 6 ] ;
		FVECTOR		p[ 6 ] ;
		int			zn ;
		DG_MDL		*md ;

		hzx_id = GM_GetHzxGroupID( where ) ;
		hzx_id = HZX_GetHzxIDbyZone( hzx_id, &mov, &zn ) ;

		md = &work->body_door.objs->def->models[ 0 ] ;
		GV_InitVec3( &p[ 0 ], md->lx, 0.0F, md->lz ) ;
		GV_InitVec3( &p[ 1 ], md->ux, 0.0F, md->lz ) ;
		GV_InitVec3( &p[ 2 ], md->lx, 0.0F, md->uz ) ;
		GV_InitVec3( &p[ 3 ], md->ux, 0.0F, md->uz ) ;

		if ( GCL_GetOption( 'h' ) != NULL ) {
			GCL_GetNextIV( ( int * )&ip[ 4 ] ) ;
			GCL_GetNextIV( ( int * )&ip[ 5 ] ) ;
			ip[ 4 ].vw = ip[ 5 ].vw = 2225 ;
			work->segment3 = HZX_AddDynamicSegment( hzx_id, &ip[ 4 ], &ip[ 5 ],
												   HZX_SEG_NO_HARITSUKI | HZX_SEG_NO_C4 | 
												   HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_BULLET | 
												   HZX_SEG_NO_MISSILE | SeNo ) ;
			HZX_DynamicSegmentSetAttribute( work->segment3, 
										   work->segment3->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
			ASSERT( work->segment3 != NULL ) ;			
		} else {
			GV_InitVec3( &p[ 4 ], md->lx, 0.0F, 0.0F ) ;
			GV_InitVec3( &p[ 5 ], md->ux, 0.0F, 0.0F ) ;
			_sceVu0AddVector( &p[ 4 ], &p[ 4 ], &mov ) ;
			_sceVu0AddVector( &p[ 5 ], &p[ 5 ], &mov ) ;
			p[ 4 ].vw = p[ 5 ].vw = 2225.0F ;
			GV_FVtoIV( &p[ 4 ], &ip[ 4 ], 8 ) ;
			work->segment3 = HZX_AddDynamicSegment( hzx_id, &ip[ 4 ], &ip[ 5 ],
												   HZX_SEG_NO_HARITSUKI | HZX_SEG_NO_C4 | 
												   HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_BULLET | 
												   HZX_SEG_NO_MISSILE | HZX_SEG_NO_SPRAY | SeNo ) ;
			HZX_DynamicSegmentSetAttribute( work->segment3, 
										   work->segment3->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
			ASSERT( work->segment3 != NULL ) ;
			{
				IVECTOR		pole ;
				FMATRIX		*world ;

				world = &work->body_door.objs->objs[ 0 ].world ;
				pole.vx = world->m[ 3 ][ 0 ] ;
				pole.vy = world->m[ 3 ][ 1 ] ;
				pole.vz = world->m[ 3 ][ 2 ] ;
				HZX_RotateDynamicSegment2( work->segment3, &pole, world ) ;
			}
		}

		_sceVu0AddVector( &p[ 0 ], &p[ 0 ], &mov ) ;
		_sceVu0AddVector( &p[ 1 ], &p[ 1 ], &mov ) ;
		_sceVu0AddVector( &p[ 2 ], &p[ 2 ], &mov ) ;
		_sceVu0AddVector( &p[ 3 ], &p[ 3 ], &mov ) ;

		p[ 0 ].vw = p[ 1 ].vw = 2225.0F ;
		p[ 2 ].vw = p[ 3 ].vw = 2225.0F ;

		GV_FVtoIV( p, ip, 16 ) ;
		work->segment = HZX_AddDynamicSegment( hzx_id, &ip[ 0 ], &ip[ 1 ],
											   HZX_SEG_NO_HARITSUKI | HZX_SEG_NO_C4 | 
											   HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_PLAYER |
											   HZX_SEG_NO_ENEMY | HZX_SEG_DOOR | SeNo ) ;
		ASSERT( work->segment != NULL ) ;
		HZX_DynamicSegmentSetAttribute( work->segment, work->segment->atr & ~HZX_SEG_NO_DISP_RADAR ) ;

		work->segment2 = HZX_AddDynamicSegment( hzx_id, &ip[ 2 ], &ip[ 3 ],
											   HZX_SEG_NO_HARITSUKI | HZX_SEG_NO_C4 | 
											   HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_PLAYER |
											   HZX_SEG_NO_ENEMY | HZX_SEG_DOOR | SeNo ) ;
		ASSERT( work->segment2 != NULL ) ;
		HZX_DynamicSegmentSetAttribute( work->segment2, 
									    work->segment2->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
		RotateSegment2( work ) ;
	}

	/* 付属プロック */
	if ( GCL_GetOption( 'e' ) != NULL ) {
		work->exec = GCL_GetNextInt() ;
	} else if ( GCL_GetOption( 'O' ) != NULL ) {
		work->proc = GCL_GetNextInt() ;
	}

	/* フラグ */
	work->flag = GCL_GetOptionValue( 'f', 0 ) ;
	work->flag |= WT_DOOR_FLAG_FIRST ;

	work->status = STATE_CLOSE ;
	work->door_motion = -1 ;
	work->handle_motion = -1 ;

	/* 開いた状態からスタート */
	if ( work->flag & WT_DOOR_FLAG_OPEN_START ) {
		SVECTOR		rot = { 0, 0, 0 } ;
		FMATRIX		m1 ;

		if ( work->type == 0 ) rot.vy = -1024 ;
		else 			 rot.vy = 1024 ;

		DG_SetPos2( &DG_ZeroVector, &rot ) ;
		DG_GetPos( &m1 ) ;
		_sceVu0MulMatrix( &work->body_door.objs->objs[ 0 ].world, 
						  &work->body_door.objs->objs[ 0 ].world, &m1 ) ;
		RotateSegment2( work ) ;
		/* segment3がない状態からスタート */
		HZX_DynamicSegmentSetSkip( work->segment3 ) ;

		work->rotate = rot ;
		work->status = STATE_OPEN ;
		work->flag &= ~WT_DOOR_FLAG_OPEN_START ;
		work->flag |= WT_DOOR_FLAG_EXIST_NOT_MOTION_ROTATE ;
	}

#ifdef KOREADD
	{
		HZX_ZONE_ADD zone[2] ;
		int dir, status ;

		dir = rot.vy + 1024 ;

printf("2:type[%d] pos[%f][%f][%f] dir[%d]\n",work->type,mov.vx,mov.vy,mov.vz,dir);

		if ( work->status == STATE_OPEN ) {
			status = ROOT_INTRPT_HINGED ;
		} else {
			status = ROOT_INTRPT_HINGED | ROOT_INTRPT_CLOSE | ROOT_INTRPT_NOENE ;
		}
		HZX_GetInterruptZone( &mov, dir, 500, &zone[0] ) ;
		GM_SetRouteIntrpt( &work->r_intrpt, zone[0], zone[1], &mov, dir,
						   status, 0 ) ;
		GM_PutRoteIntrpt( &work->r_intrpt ) ;
	}
#endif

	work->motion_speed = ( float )TIME_BASE ;

	return 1 ;
}


/*----------------------------------------------------------------*/

static	void	SetMsgBuf( name, a, b, c, d, e, f, g )
int				name ;
int				a, b, c, d, e, f, g ;
{
	MsgBuf[ 7 ] = name ;
	MsgBuf[ 0 ] = a ;
	MsgBuf[ 1 ] = b ;
	MsgBuf[ 2 ] = c ;
	MsgBuf[ 3 ] = d ;
	MsgBuf[ 4 ] = e ;
	MsgBuf[ 5 ] = f ;
	MsgBuf[ 6 ] = g ;
}

/* プレイヤー行動 */

static	int		NikaitenProc = 0 ;
static	int		NikaitenMode = 0 ;

/* 終了 */
static	void	EndWtDoorHold2( work )
PlayerWork		*work ;
{
	SetFlag( FLAG_FORCE_END ) ;
	PL_Force->e_turn = -1 ;
	PL_Force->flag = 0 ;
	work->control.skip_flag &= ~CTRL_SKIP_NEAR_CHECK ;
	if ( work->dmg_callback == WtDoorDmgCallback ) work->dmg_callback = NULL ;
	PL_UnsetInvincible( work ) ;
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	GM_VctrlSetVibrationScale( &work->vctrl, 1.0F ) ;
}

/* 終了モード設定 */
static	void	SetReturnMode( PlayerWork *work )
{
	void	( *action )( PlayerWork *, int ) ;

	if ( PL_ReturnMode == NULL ) SetMode( work, PL_StillMode[ STAND ] ) ;
	else {
		action = PL_ReturnMode ;
		( *action )( work, 0 ) ;
	}
}

/* プレイヤー行動 */
static	void	WtDoorHold2( work, time )
PlayerWork		*work ;
int				time ;
{
	int				act, dact ;
	int				interp, dir ;
	float			speed_adj ;

	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE | FLAG_NO_IK |
			 FLAG_CANNOT_ATTACK | FLAG_NO_TOUCH_DAMAGE | FLAG_NO_CAUTION ) ;
	SetStatus( PLAYER_WEAPON_INVISIBLE | PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;

#ifdef DEBUG_MODE
	if ( GV_PadData->status & PAD_L1 ) {
		speed_adj = 1.0F ;
	} else {
		if ( Status( PLAYER_IN_THE_WATER ) ) speed_adj = 2.0F ;
		else								 speed_adj = 1.0F ;
	}
#else
	if ( Status( PLAYER_IN_THE_WATER ) ) speed_adj = 2.0F ;
	else								 speed_adj = 1.0F ;
#endif

	if ( time == 0 ) {
		PL_UnsetInvincible( work ) ;
		PL_LeaveSubject( work ) ;
		work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK ;
		/* モーションファイルチェンジ */
		PL_ChangeMotionArc( work, PlayerMotion ) ;
		interp = ( Status( PLAYER_IN_THE_WATER ) ) ? 0 : 6 ;
		dir = work->control.rot.vy ;
		if ( interp == 0 ) {
			float	levels[ 2 ] ;
			/* 水中 */
			HZX_LevelHazardCheck( work->control.hzx_id, &work->control.mov, 
								  HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER ) ;
			HZX_GetLevelHeight( levels ) ;
			work->control.step.vy = levels[ 0 ] + 750.0F - work->control.mov.vy ;
		}
		if ( WtDoorOpen == 0 ) {	/* 表 */
			SetAction( work, PMloop, interp ) ;
			SetMsgBuf( work->send_to, MSG_HOLD, work->control.name, HMloop2_180, 
					   0, 0, 0, ( int )speed_adj ) ;
		} else {					/* 裏 */
			SetAction( work, PMloop_rev, interp ) ;
			SetMsgBuf( work->send_to, MSG_HOLD, work->control.name, HMloop2_rev, 
					   0, 0, 0, ( int )speed_adj ) ;
		}
		work->control.turn.vy = dir ;
		work->control.rot.vy = dir ;
		GM_VctrlSetLoop( &work->vctrl ) ;
		work->data2 = 0 ;
		work->idata = -1 ;
		work->sv.vx = 0 ;
		RotateFast = 0 ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / speed_adj ) ;
		GM_VctrlSetVibrationScale( &work->vctrl, 1.0F / speed_adj ) ;

		work->control.rot.vx = work->control.turn.vx = 0 ;
		work->control.rot.vz = work->control.turn.vz = 0 ;
	}
	if ( work->pad->release & PL_PAD_ACTION ) {
		work->idata = GV_Time ;
		work->sv.vx = 1 ;
	}
	switch( work->data ) {
	case 0 :	/* つかみまわし */
		if ( NikaitenMode > 0 ||
			 ( work->pad->status & PL_PAD_ACTION ) || 
			 ( work->sv.vx == 1 && GV_Time - work->idata < QUICK ) ) {
			if ( PL_CheckMotionRate( work ) < 0.85F &&
				 work->sv.vx == 1 && ( work->pad->press & PL_PAD_ACTION ) ) {
				MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * FAST / speed_adj ) ;
				GM_VctrlSetVibrationScale( &work->vctrl, 1.0F / speed_adj ) ;
				work->idata = GV_Time ;
				work->sv.vx = 2 ;
				RotateFast = 1 ;
			} else if ( PL_CheckMotionRate( work ) < 0.85F &&
					    work->sv.vx == 2 && GV_Time - work->idata > QUICK - 4 ) {
				MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / speed_adj ) ;
				GM_VctrlSetVibrationScale( &work->vctrl, 1.0F / speed_adj ) ;
				work->sv.vx = 0 ;
                RotateFast = 0 ;
			}

			if ( NikaitenMode > 0 ) {
				if ( NikaitenMode == 1 ) {
					MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / speed_adj ) ;
					GM_VctrlSetVibrationScale( &work->vctrl, 1.0F / speed_adj ) ;
					work->sv.vx = 0 ;
					RotateFast = 0 ;
				} else {
					MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * FAST / speed_adj ) ;
					GM_VctrlSetVibrationScale( &work->vctrl, 1.0F / speed_adj ) ;
					work->sv.vx = 2 ;
					RotateFast = 1 ;
				}
			}

			if ( EndMotion( work ) ) {
				if ( ++ work->data2 >= 3 ) {
					if ( WtDoorType == 0 ) {	/* 左軸 */
						if ( WtDoorOpen == 0 ) {	/* 引き */
							act = PMr_pull ;
							dact = DMr_pull ;
						} else {					/* 押し */
							act = PMl_push ;
							dact = DMl_push ;
						}
					} else {					/* 右軸 */
						if ( WtDoorOpen == 0 ) {	/* 引き */
							act = PMl_pull ;
							dact = DMl_pull ;
						} else {					/* 押し */
							act = PMr_push ;
							dact = DMr_push ;
						}
					}
					SetAction( work, act, 6 ) ;
					MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / speed_adj ) ;
					GM_VctrlSetVibrationScale( &work->vctrl, 1.0F / speed_adj ) ;
					RotateFast = 0 ;
					SetMsgBuf( work->send_to, MSG_OPEN_START, work->control.name,
							   dact, 0, 0, 0, ( int )speed_adj ) ;
                    /* 開き中無敵 */
					PL_SetInvincible( work, 0 ) ;
					work->data ++ ;
				}
				if ( work->data2 == 2 && NikaitenProc > 0 ) {
					GM_ExecProc( NikaitenProc, NULL ) ;
					PL_SetInvincible( work, 0 ) ;
					NikaitenProc = 0 ;
					NikaitenMode = 1 + RotateFast ;
				}
			}
		} else {
			EndWtDoorHold2( work ) ;
			SetMsgBuf( work->send_to, MSG_HOLD_END, work->control.name, 
					  0, 0, 0, 0, ( int )speed_adj ) ;
			SetReturnMode( work ) ;
		}
		break ;
	case 1 :		/* 開き */
		if ( EndMotion( work ) ) {
			EndWtDoorHold2( work ) ;
			SetMsgBuf( work->send_to, MSG_OPEN_END, work->control.name, 0, 
					  0, 0, 0, ( int )speed_adj ) ;
			SetReturnMode( work ) ;
			NikaitenProc = 0 ;
			NikaitenMode = 0 ;
		}
	}
}

/* 開かない */
static	void	WtDoorNotOpen2( work, time )
PlayerWork		*work ;
int				time ;
{
	float		speed_adj ;
	int			interp, dir ;

	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE | FLAG_NO_IK |
			 FLAG_CANNOT_ATTACK | FLAG_NO_TOUCH_DAMAGE | FLAG_NO_CAUTION ) ;
	SetStatus( PLAYER_WEAPON_INVISIBLE | PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;

	if ( Status( PLAYER_IN_THE_WATER ) ) speed_adj = 2.0F ;
	else								 speed_adj = 1.0F ;

	if ( time == 0 ) {
		PL_SetInvincible( work, DIRECT_TICK( 48 ) ) ;
		PL_LeaveSubject( work ) ;
		work->control.skip_flag |= CTRL_SKIP_NEAR_CHECK ;
		/* モーションファイルチェンジ */
		PL_ChangeMotionArc( work, PlayerMotion ) ;
		interp = ( Status( PLAYER_IN_THE_WATER ) ) ? 0 : 6 ;
		dir = work->control.rot.vy ;
		if ( interp == 0 ) {
			float	levels[ 2 ] ;
			/* 水中 */
			HZX_LevelHazardCheck( work->control.hzx_id, &work->control.mov, 
								  HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER ) ;
			HZX_GetLevelHeight( levels ) ;
			work->control.step.vy = levels[ 0 ] + 750.0F - work->control.mov.vy ;
		}
		SetAction( work, PMnot_open, interp ) ;
		SetMsgBuf( work->send_to, MSG_NOT_OPEN, work->control.name, HMnot_open, 
				   0, 0, 0, ( int )speed_adj ) ;
		if ( Status( PLAYER_IN_THE_WATER ) ) speed_adj = 2.0F ;
		else								 speed_adj = 1.0F ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / speed_adj ) ;
		GM_VctrlSetVibrationScale( &work->vctrl, 1.0F / speed_adj ) ;

		work->control.turn.vy = dir ;
		work->control.rot.vy = dir ;

		work->control.rot.vx = work->control.turn.vx = 0 ;
		work->control.rot.vz = work->control.turn.vz = 0 ;
	}	
	if ( !( work->pad->status & PL_PAD_ACTION ) ) {	
		if ( ++ work->data >= 16 ) {
			/* PAD_Aはアクションボタン（シナリオと違わないように注意） */
			SetMsgBuf( work->send_to, MSG_NOT_OPEN_END, work->control.name, 0, 
					   0, 0, 0, ( int )speed_adj ) ;
			EndWtDoorHold2( work ) ;
			SetReturnMode( work ) ;
			return ;
		}
	} else {
		work->data = 0 ;
	}
	if ( EndMotion( work ) ) {
		EndWtDoorHold2( work ) ;
		SetReturnMode( work ) ;
	}
}

/* ダメージコールバック */
static	void	WtDoorDmgCallback( PlayerWork *work, TARGET *off, TARGET *def  )
{
	EndWtDoorHold2( work ) ;
	if ( work->action == WtDoorNotOpen2 ) {
		SetMsgBuf( work->send_to, MSG_NOT_OPEN_END, work->control.name, 0, 
				   0, 0, 0, 1 ) ;
	} else {
		SetMsgBuf( work->send_to, MSG_HOLD_END, work->control.name, 0, 0, 0, 0, 0 ) ;
	}
	/* 水中特殊 */
	if ( Status( PLAYER_IN_THE_WATER ) ) {
		extern void *NewBloodWater( FVECTOR *pos, FVECTOR *vec, float pow, int mode ) ;
		FVECTOR		pos, force ;

		UnsetFlag( FLAG_BLOOD_SPLASH ) ;
		PL_ObjPos( work, HUMAN21_KUBI, &pos ) ;
		_sceVu0SubVector( &force, &pos, &off->center ) ;
		/* 水中血 */
		NewBloodWater( &pos, &force, 16, 0 ) ;
	}
}

/*----------------------------------------------------------------*/

/* ハンドル２回転後にプロックを呼ぶ。
   その後は自動でハンドルをまわす */
int		PL_COM_WtDoorHandle2TurnProc( void )
{
	NikaitenMode = 0 ;
	NikaitenProc = GCL_GetOptionValue( 'p', 0 ) ;	
	return 0 ;
}

static	Work	*SearchWtDoor( int ) ;

/* 水密ドアハンドル止める */
void   PL_WtDoorHoldEnd( int to, int name ) 
{
	if ( SearchWtDoor( to ) != NULL ) {
		SetMsgBuf( to, MSG_HOLD_END, name, 0, 0, 0, 0, 0 ) ;
	}
}
