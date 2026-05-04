/*
	gllpre.c
	ゴルルゴン思考前処理

	2002/04/04 Y.Korekado
	$Id: gllpre.c,v 1.1.1.3 2002/11/19 11:44:13 Yoshizawa1 Exp $
*/
enum {
	GLL_MESSAGE_ROUTE_CHANGE = 1,
	GLL_MESSAGE_STOP ,
	GLL_MESSAGE_TUB_KILL
} ;

static	void	CheckMessage( Work *work )
{
    GV_MSG	*msg ;
	int n_msg, code ;
	CONTROL	*ctrl ;

	ctrl = &work->control ;

    n_msg = ctrl->n_msg ;
	msg = ctrl->msg ;

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case GLL_MESSAGE_ROUTE_CHANGE :
			{
				ROUTENAVI	*rnavi ;

printf( "gll: GLL_MESSAGE_ROUTE_CHANGE \n" ) ;
				rnavi = work->npc.rnavi ;
				rnavi->next_route = (short)msg->message[ 1 ] ;
				rnavi->chang_node = ( msg->message_len >= 3 ) ? (u_char)msg->message[ 2 ] : -1 ;
			}
			break ;
		  case GLL_MESSAGE_STOP :
printf( "gll: GLL_MESSAGE_STOP \n" ) ;
		  	work->status |= GLL_STATUS_STOP ;
			break;

		  case GLL_MESSAGE_TUB_KILL :
printf( "gll: GLL_MESSAGE_TUB_KILL \n" ) ;
		  	GLL_GAME_STATUS |= GLL_GS_TUB_KILL ;
			break;

			default :
			break ;
		}
		msg++ ;
	}
}

#define SE_COS_10	(0.9848077530122f)
static	void	InfoCheck( Work *work )
{
	FVECTOR	vec, head ;
	SVECTOR	search_rot ;
	NPCWORK	*npc ;
	int		x_rot, diff ;

	npc = &work->npc ;

//	_sceVu0SubVector(  &vec, &GM_PlayerPosition, &npc->ctrl->mov ) ;
	KR_FMatToFvec( &BODYWORLD(npc->body, HUMAN21_ATAMA), &head ) ;
	_sceVu0SubVector(  &vec, &GM_PlayerPosition, &head ) ;
	work->pl_dis = _FVecLen3( &vec ) ;	/* プレイヤーとの距離 */
	work->pl_dir = _FVecDir2( &vec ) ;	/* プレイヤーへの方向 */

#if 1
	{
		FVECTOR	p, p2, v ;
		SVECTOR	d ;
		DG_SetPos( &BODYWORLD(npc->body, HUMAN21_ATAMA) ) ;
		p.vx = 0.0; p.vy = 0.0; p.vz = 1000.0 ;
		DG_PutVector( &p, &p2, 1 );
		_sceVu0SubVector(  &v, &p2, &head ) ;
		_FVecToRotXY( &v, &d ) ;
		d.vx -= 1024 ;
		
		work->facedir_x = d.vx ;
		work->facedir = d.vy ;
		
		DG_SetPos2( &head, &d ) ;
		DG_GetPos( &work->sight_world ) ;
	}
#else
	work->facedir = 4095 & MatToYRot( &BODYWORLD( npc->body, HUMAN21_ATAMA ) ) ;
	work->facedir_x = MatToXRot( &BODYWORLD( npc->body, HUMAN21_ATAMA ) ) ;
	
	search_rot.vx = work->facedir_x ;
	search_rot.vy = work->facedir ;
	search_rot.vz = 0 ;

	DG_SetPos2( &head, &search_rot ) ;
	DG_GetPos( &work->sight_world ) ;
#endif
//printf(" eye info check !! \n");
//printf(" facedir y[%d] x[%d] \n",work->facedir,work->facedir_x);

	work->sight = EYE_INFO_SIGHT_OUT ;

	/* 視力判定１ */
	if ( work->pl_dis > work->eye_length ) {
		goto end ;
	}

	if ( StatusFightGME( work->status ) ) {//メカゴルルゴン、サーチライト判定
		static FVECTOR norm_vec = {0.0, 0.0, 100.0, 0 } ;
		FMATRIX m ;
		FVECTOR *p, *v1, *v2 ;
	    FVECTOR	d1, d2 ;
	    float 	f ;

		KR_FMatToFvec( &work->search_world, &head ) ;

		DG_SetPos( &work->search_world ) ;
		DG_MovePos( &norm_vec ) ;
		DG_GetPos( &m ) ;

		p = (FVECTOR *)&work->search_world.m[3][0] ;
		v1 = (FVECTOR *)&m.m[3][0] ;
		v2 = &GM_PlayerPosition ;

	    _sceVu0SubVector( &d1, p, v1 ) ;
	    _sceVu0SubVector( &d2, p, v2 ) ;
	    _sceVu0Normalize( &d1, &d1 ) ;
	    _sceVu0Normalize( &d2, &d2 ) ;
	    f = _sceVu0InnerProduct( &d1, &d2 ) ;

//printf("f[%f]\n",f);
		if ( f <= SE_COS_10 ) {
			goto end ;
		}
	} else {
		/* 視界判定は頭の位置から */
		KR_FMatToFvec( &BODYWORLD(npc->body, HUMAN21_ATAMA), &head ) ;
		_sceVu0SubVector(  &vec, &GM_PlayerPosition, &head ) ;

		/* 視野判定 */
//printf("pl_dir[%d] facedir[%d] diff[%d] range[%d]\n",work->pl_dir,work->facedir,_DiffDirAbs( work->facedir, work->pl_dir ), work->eye_range);
		if ( _DiffDirAbs( work->facedir, work->pl_dir ) > work->eye_range ) {
			goto end ;
		}

		/* 高低差判定 */
		x_rot = _FVecDirX( &vec ) - 1024 ;
		diff = GV_DiffDirS( work->facedir_x, x_rot ) ;
//printf(" x_rot[%d] diff[%d]\n",x_rot, diff );
		if ( diff > UNDER_EYE_SIGHT || diff < UPPER_EYE_SIGHT ) {
			goto end ;
		}
	}

	/* 視力判定２ */
	if ( HZX_OnlineHazardCheck( npc->ctrl->hzx_id, &GM_PlayerPosition, &head,
				HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ) {
//		work->sight = EYE_INFO_SIGHT_OUT_HZD ;
	} else {
		work->sight = EYE_INFO_SIGHT_IN ;
	}

#ifdef DEBUG_MODE
if ( debug_flag & GLL_DEBUG_EYE_VIEW ) {
	extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
	FVECTOR	lin[2] ;
	lin[0] = head ;
	lin[1] = GM_PlayerPosition ;
	if ( work->sight == EYE_INFO_SIGHT_IN ) {
		NewLineView(  &lin[0],1,255,0,0) ;
	} else {
		NewLineView(  &lin[0],1,0,0,255) ;
	}
}
#endif

end : ;

	if ( GM_VRStatus & GM_VR_CLEAR ) {
		work->sight = EYE_INFO_SIGHT_OUT ;
	}

#ifdef DEBUG_MODE
if ( debug_flag & GLL_DEBUG_EYE_CLOSE ) {
	work->sight = EYE_INFO_SIGHT_OUT ;
}
//work->sight = EYE_INFO_SIGHT_OUT ;
#endif

}

static FVECTOR	EyeShift[2] = {
	{ 0.0, 0.0, 0.0 },
	{ 0.0, 375.0, 1245.0 },
} ;

#define EYE_COS_30	(0.8660254037844f)
#define EYE_COS_45	(0.7071067811865f)

static FVECTOR *BookFound( void )
{
	ENEFINDLIST	*efl ;
	ENEFIND *ef ;
	FVECTOR	vec, head ;
	int 	dis ;

	efl = &GM_EneFindList ;

	if( efl->start == NULL ) return NULL ;

	ef = efl->start ;
	while( ef != NULL ) {
//printf("EF ef->id[%d]  ef->type[%x]\n",ef->id, ef->type)  ;
		if( !(ef->type & (EF_TYPE_NO_FIND|EF_TYPE_FOUND)) ) {
			if ( ef->type & EF_TYPE_ADULT ) {
				return &ef->pos ;
			}
		}
		ef = ef->next ;
	}

	return NULL ;
}

static	void	EyeBeemCheck( Work *work )
{
	FVECTOR	pos[2], *trg ;
	float ip ;
	int	dis, flag ;

	flag = GllEyebeemFlag ;
	GllEyebeemFlag = 0 ;
	if ( flag & GLL_EYEBEEM_MOVE ) {
		DG_SetPos ( &GllEyebeemWorld ) ;
		DG_PutVector( EyeShift, pos, 2 ) ;

		/* 変装スネークチェック */
		if ( (PL_GetPlayerItem() == IT_Uniform) &&
			 (PL_GetPlayerWeapon() == WP_Aks ) ) {
			trg = &GM_PlayerPosition ;

			dis = _FVecTrgDis( &pos[1], trg ) ;
			ip = KR_InnerProduct( &pos[0], &pos[1], trg ) ;

			if ( (ip > EYE_COS_45) && (dis < 2000) ) {
				if ( !HZX_OnlineHazardCheck( work->control.hzx_id, trg, &pos[0],
						HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ) {
					SET_FLAG( work->find_flag, GLL_FIND_BUDY ) ;
				}
			}
		}

		/* エロ本チェック */
		if ( (trg = BookFound()) != NULL ) {
			dis = _FVecTrgDis( &pos[1], trg ) ;
			ip = KR_InnerProduct( &pos[0], &pos[1], trg ) ;

//printf("ip[%f] dis[%d] \n",ip,dis) ;
			if ( (ip > EYE_COS_45) && (dis < 2500) ) {
//				KR_FMatToFvec( &BODYWORLD(&work->body, HUMAN21_ATAMA), &head ) ;
				if ( !HZX_OnlineHazardCheck( work->control.hzx_id, trg, &pos[0],
						HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ) {
					FVECTOR vec ;

					/* 見えた！！ */
				 	_sceVu0SubVector(  &vec, trg, &work->control.mov ) ;
					work->book_dir = _FVecDir2( &vec ) ;
					work->book_pos = *trg ;
					SET_FLAG( work->find_flag, GLL_FIND_BOOK ) ;
printf("!!!!!!Book Find!!!! \n") ;
				}
			}
		}
	}
}
/*----------------------------------------------------------------*/
static void PreProcess( Work *work )
{
	CheckMessage( work ) ;

	InfoCheck( work ) ;		/* 周りの情報チェック */
	if ( StatusFightGLL( work->status ) ) {
		EyeBeemCheck( work ) ;	/* 目ビームチェック */
	}
}


static void AfterProcess( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;
	/* 顔の向き */
	npc->action.face_dir = npc->ctrl->rot.vy ;

	/* ライフゲージ更新 */
	if ( work->gage.value != npc->action.life ) {
		work->gage.value = npc->action.life ;
	}

	/* 心臓の鼓動 */
	if ( !(GV_Time%110) ) {
//		GM_SeSetMode( SD_E_EHEART01, &npc->ctrl->mov, GM_SEMODE_MIC ) ;
		/* エフェクト */
		{
			FVECTOR	shi ;
			
			shi.vx = 0.0 ;
			shi.vy = 0.0 ;
			shi.vx = 1000.0 ;
//			NewBreath( &npc->body->objs->objs[ 12 ].world, &shi, &work->sw_breth ) ;
			work->sw_breth = 1 ;
		}

	}
}


static	void	GLL_AfterProcess( Work *work ) 
{
	NPCTARGET	*npctrg ;
	NPCACT		*act ;
	TARGET		*def_child ;
	int i ;
	NPCWORK	*npc ;

	npc = &work->npc ;
	act = &npc->action ;
	if ( npc->target.deftrg != NULL ) {
		npctrg = &npc->target ;

	    def_child = npctrg->def_child ;
		for( i=0; i<npctrg->child_trg_num ; i++ ) {
			if ( !StatusFightGLL( work->status ) ) {
				GM_MoveTarget2Map( def_child, &(BODYWORLD( npc->body, npctrg->connect_obj[i])),
					npc->ctrl->map ) ;
			} else {
				FMATRIX w ;
				DG_SetPos( &(BODYWORLD( npc->body, npctrg->connect_obj[i])) ) ;
				DG_MovePos( &Cheild_Target_Shift[i] ) ;
				DG_GetPos( &w ) ;
				GM_MoveTarget2Map( def_child, &w, npc->ctrl->map ) ;
			}

			def_child++ ;
		}

	}

	/* レーダー */
	if ( npc->rctrl != NULL ) {
		int	range ;
		float sight ;

//		GM_RadarSetFlag( npc->rctrl, RADAR_VISIBLE ) ;
		
		range = act->eye_range ;
		sight = (float)act->eye_sight ;
		if ( act->status & NPC_ACT_STATUS_EYE_CLOSE ) {
			range = 0 ;
			sight = 0.0f ;
		}
		GM_RadarSetSight( npc->rctrl, act->face_dir, range, sight, act->radar_color );
	}

	/* マルチウェイトモデル切替え */
	if ( npc->body->evmobj != NULL ) {
		DG_EVMOBJ *evm ;
		int dis, lod ;

		evm = npc->body->evmobj ;
		dis = KR_CameraDis( &npc->ctrl->mov ) ;

		lod = ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) 
			? npc->lod * 2 : npc->lod ;

		if ( dis > lod ) {
			UNSET_FLAG( npc->body->objs->flag, DG_FLAG_INVISIBLE ) ;
			SET_FLAG( evm->flag, DG_EVMOBJ_INVISIBLE ) ;
		} else {
			SET_FLAG( npc->body->objs->flag, DG_FLAG_INVISIBLE ) ;
			UNSET_FLAG( evm->flag, DG_EVMOBJ_INVISIBLE ) ;
		}
		if ( npc->inf_name ) {
			extern void GM_MouthAnimation( int talk_name, DG_EVMOBJ *evmobj  ) ;
			GM_MouthAnimation( npc->inf_name, evm ) ;
		}
	}
}


