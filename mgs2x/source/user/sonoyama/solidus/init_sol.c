/*
   init_sol.c
   ソリダス初期化
   
   2001/03/21	M.Sonoyama
   $Id: init_sol.c,v 1.1.1.3 2002/11/19 11:51:04 Yoshizawa1 Exp $
*/

/* コントロール初期化 */
static	int	InitControl( Work *work, int name, int where )
{
	CONTROL			*ctrl ;

	ctrl = &work->control ;
	if ( GM_InitControl( ctrl, name, where ) < 0 ) return -1 ;
	PL_GetOptionFV( 'p', &ctrl->mov ) ;
	PL_GetOptionSV( 'r', &ctrl->rot ) ; 
	ctrl->turn = ctrl->rot ;
	ctrl->seg_flag |= HZX_SEG_NO_ENEMY ;
	ctrl->flr_flag |= HZX_FLOOR_NO_ENEMY ;
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
	GM_ConfigControlObject( ctrl, &work->body ) ;
	GM_ConfigControlHazard( ctrl, 1000, NORMAL_SPHERE, CHECK_SPHERE ) ;
	{
		float			level ;
		int				flag ;

		flag = HZX_LevelHazardCheck( ctrl->hzx_id, &ctrl->mov, HZX_CHK_ALL,
									 ctrl->flr_flag ) ;
		if ( flag & 1 ) {
			level = HZX_GetFloorLevel() ;
			GM_ConfigControlHzxHeight( ctrl, NEAR_HEIGHT_STAND, level ) ;
		} else {
			GM_ConfigControlHzxHeight( ctrl, NEAR_HEIGHT_STAND, work->control.mov.vy ) ;
		}
	} 
	SOL_SolControl = ctrl ;
	return 0 ;
}

/*----------------------------------------------------------------*/

/* フェイスガードの位置計算 */
static	void	ExprFaceGuardPos( Work *work )
{
	FVECTOR		shift = { 0.0F, 0.0F, 35.0F } ;

	DG_SetPos( &work->body.objs->objs[ HUMAN21_MUNE ].world ) ;
	DG_MovePos( &shift ) ;
	DG_GetPos( &work->faceguard->world ) ;
}

/* オブジェクトの初期化 */
static	int	InitObject( Work *work, int where )
{
	OBJECT			*body ;

	body = &work->body ;
	/* モデル */
	GM_InitObject( body, GCL_GetOptionValue( 'm', 0 ), BODY_FLAG ) ;
	ASSERT( body->objs != NULL ) ;
	GM_SetCurrentMap( where ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	GM_ActObject( body ) ;
	/* モーション */
	GM_ConfigObjectMotion( body, 2, GCL_GetOptionValue( 'T', 0 ), MT_FLAG_HUMAN2 ) ;
	GM_ConfigObjectAction( body, 0, Mkwt_idle, 0, 0xfffff, 0 ) ;
	GM_ConfigObjectStep( body, &work->control.step ) ;

	/* マルチウェイト */
	GM_ConfigObjectEvm( body, GCL_GetOptionValue( 'e', 0 ), 0 ) ;

	/* マルチウェイト設定の後じゃないとダメ */
	GM_ConfigObjectLight( body, work->lights ) ;

	/* 眼帯 */
	{
		DG_DEF		*def ;
		DG_OBJS		*objs ;

		def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "sol_gantai_obj" ), 'k' ) ) ;
		ASSERT( def != NULL ) ;
		work->gantai = objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
		objs->root = &body->objs->objs[ HUMAN21_ATAMA ].world ;
	}
	/* フェイスガード */
	{
		DG_DEF		*def ;
		DG_OBJS		*objs ;
		
		def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "sol_faceguard" ), 'k' ) ) ;
		ASSERT( def != NULL ) ;
		work->faceguard = objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
		ExprFaceGuardPos( work ) ;
	}

	/* 鞘 */
	{
		DG_DEF		*def ;
		DG_OBJS		*objs ;

		def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "demo_kwt_sht" ), 'k' ) ) ;
		ASSERT( def != NULL ) ;
		work->kwt_sht = objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
		objs->root = &body->objs->objs[ HUMAN21_KOSHI ].world ;

		def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "demo_mst_sht" ), 'k' ) ) ;
		ASSERT( def != NULL ) ;
		work->mst_sht = objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
		objs->root = &body->objs->objs[ HUMAN21_KOSHI ].world ;
	}

	return 0 ;
}

/*----------------------------------------------------------------*/

/* 各子ターゲットの初期化 */
static	void	SetTarget2Joint( Work *work, TARGET *t, int where, OBJECT *body, int joint,
								 FVECTOR *size_def, FVECTOR *offset_def )
{
	FVECTOR		size, offset ;
	DG_MDL		*mdl ;

	mdl = body->objs->objs[ joint ].model ;
	if ( size_def == NULL ) {
		size.vx = ( mdl->ux - mdl->lx ) / 2.0F ;
		size.vy = ( mdl->uy - mdl->ly ) / 2.0F ;
		size.vz = ( mdl->uz - mdl->lz ) / 2.0F ;
		offset.vx = ( mdl->ux + mdl->lx ) / 2.0F ;
		offset.vy = ( mdl->uy + mdl->ly ) / 2.0F ;
		offset.vz = ( mdl->uz + mdl->lz ) / 2.0F ;
	} else {
		DG_COPY_VEC( &size, size_def ) ;
		DG_COPY_VEC( &offset, offset_def ) ;
	}
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER | TARGET_ROTATE, where, ENEMY_SIDE, 
				  &size, &offset ) ;
	GM_SetTargetName( t, joint ) ;	/* 名前に関節番号 */
	t->power = &work->pt_defense ;
	GM_SetTargetCallBack( t, DefenseCallback, work ) ;
	UpdateJointTarget( t, where, body ) ;
}

/* ターゲットの初期化 */
static	int	InitTarget( Work *work, int name, int where )
{
	TARGET			*t ;
	FVECTOR			size, offset ;

	/* 親 */
	t = &work->defense ;
	GV_SetVec3( &size, 1000.0F, 1250.0F, 1000.0F ) ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_ROTATE | TARGET_CHILD_ALWAYS, where, 
				  ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	GM_SetTargetName( t, name ) ;
	GM_SetPowerTarget( t, &work->pt_defense, POWER_DECREASE, 100, 0, 0,
					   &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, DefenseCallback, work ) ;
	GM_MoveTargetMap( t, &work->control.mov, where ) ;
	GM_PutTarget( t ) ;
	/* 子供 */
	GM_TargetConnectChild( t, &work->child[ 0 ], 3, 0 ) ;
	GM_TargetConnectChild( t, &work->child[ 3 ], 8, 1 ) ;
	GM_TargetConnectChild( t, &work->child[ 11 ], 2, 2 ) ;
	/* ---- レベル０ ----*/
	/* 頭 */
	SetTarget2Joint( work, &work->child[ 0 ], where, &work->body, 
					 HUMAN21_ATAMA, NULL, NULL ) ;
	/* 心臓 */
	GV_SetVec3( &size, 50.0F, 50.0F, 50.0F ) ;
	GV_SetVec3( &offset, 75.0F, 250.0F, 0.0F ) ;
	SetTarget2Joint( work, &work->child[ 1 ], where, &work->body, 
					 HUMAN21_MUNE, &size, &offset ) ;
	/* 股間 */
	GV_SetVec3( &size, 50.0F, 50.0F, 50.0F ) ;
	GV_SetVec3( &offset, 0.0F, 0.0F, 50.0F ) ;
	SetTarget2Joint( work, &work->child[ 2 ], where, &work->body, 
					 HUMAN21_KOSHI, &size, &offset ) ;
	/* ---- レベル１ ----*/	
	SetTarget2Joint( work, &work->child[ 3 ], where, &work->body, 
					 HUMAN21_MIGI_UDE1, NULL, NULL ) ;
	SetTarget2Joint( work, &work->child[ 4 ], where, &work->body, 
					 HUMAN21_MIGI_UDE2, NULL, NULL ) ;
	SetTarget2Joint( work, &work->child[ 5 ], where, &work->body, 
					 HUMAN21_HIDARI_UDE1, NULL, NULL ) ;
	SetTarget2Joint( work, &work->child[ 6 ], where, &work->body, 
					 HUMAN21_HIDARI_UDE2, NULL, NULL ) ;
	SetTarget2Joint( work, &work->child[ 7 ], where, &work->body, 
					 HUMAN21_MIGI_ASHI1, NULL, NULL ) ;
	SetTarget2Joint( work, &work->child[ 8 ], where, &work->body, 
					 HUMAN21_MIGI_ASHI2, NULL, NULL ) ;
	SetTarget2Joint( work, &work->child[ 9 ], where, &work->body, 
					 HUMAN21_HIDARI_ASHI1, NULL, NULL ) ;
	SetTarget2Joint( work, &work->child[ 10 ], where, &work->body, 
					 HUMAN21_HIDARI_ASHI2, NULL, NULL ) ;
	/* ---- レベル２ ----*/	
	SetTarget2Joint( work, &work->child[ 11 ], where, &work->body, 
					 HUMAN21_KOSHI, NULL, NULL ) ;
	SetTarget2Joint( work, &work->child[ 12 ], where, &work->body, 
					 HUMAN21_MUNE, NULL, NULL ) ;

	/* ホーミング */
	{
		HOMING_TRG		*hom ;

		hom = &work->homing_trg ;
#if 1	//ファイティングポーズをとらせるためにHOMING_ENEMYをいれる
		GM_SetHomingTrg( hom, &work->body.objs->world, &work->body, &work->control.map,
						 &work->control, HOMING_ENEMY ) ;
#else
		GM_SetHomingTrg( hom, &work->body.objs->world, &work->body, &work->control.map,
						 &work->control, 0 ) ;
#endif
		GM_PutHomingTrg( hom ) ;
	}
	return 0 ;
}

/* ゲージの初期化 */
static	int	InitGage( Work *work )
{
	GM_GageSet		*gs ;
	int				max, cur ;

	GCL_GetOption( 'l' ) ;
	cur = GCL_GetNextInt() ;
	max = GCL_GetNextInt() ;

	/* 難易度別 */
	switch( SOL_GameLevel ) {
	case GM_LEVEL_VERYEASY :
		cur -= 0 ;
		max -= 0 ;
		break ;
	case GM_LEVEL_EASY :
		cur -= 0 ;
		max -= 0 ;
		break ;
	case GM_LEVEL_HARD :
		cur += 20 ;
		max += 20 ;
		break ;
	case GM_LEVEL_EXTREME :
		cur += 20 ;
		max += 20 ;
		break ;
	case GM_LEVEL_NORMAL :
	default :
		if ( !GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
			cur += 25 ;
			max += 25 ;
		}
	}

	gs = &work->life_gage ;
	GM_InitGageSet( gs, "SOLIDUS", 16, 240, GM_DEFAULT_GAGE_HEIGHT,
				    cur, max, 0, 30, 2 ) ;
	GM_SetGageColor( gs, 0,0,0, 40,128,118, 110,190,118, 255,0,0 ) ;

	GM_InitGageM9( gs, cur, max, 0, 30 ) ;

	GM_AppendGageSet( gs ) ;
	GM_VisibleGage( gs ) ;
	return 0 ;
}

/* 頂点アニメワークの初期化 */
static	int	InitVAnime( Work *work )
{
	VERTEX_ANIME_WORK	*va_work ;	
	CV2_DEF				*cv2_def1, *cv2_def2 ;
	int					i ;
	float				t = 0.0F ;

	GCL_GetOption( 'c' ) ;
	cv2_def1 = ( CV2_DEF * )GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'c' ) ) ;
	ASSERT( cv2_def1 != NULL ) ;
	cv2_def2 = ( CV2_DEF * )GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'c' ) ) ;
	ASSERT( cv2_def2 != NULL ) ;

	for ( i = 0; i < N_UNITS; i ++ ) {
		va_work = work->va_work[ i ] 
			= InitVertexAnimation( &work->body.objs->objs[ i ], &cv2_def1->models[ i ],
								   DG_VANIME_VERTS | DG_VANIME_NORMS, 
								   2 ) ;
		va_work->key[ 0 ] = &cv2_def1->models[ i ] ;		
		va_work->key[ 1 ] = &cv2_def2->models[ i ] ;
		va_work->p[ 0 ] = t ;
		va_work->p[ 1 ] = 1.0F - t ;
		SimpleVertexAnimation( va_work ) ;
	}

	work->va_t = t ;
	return 0 ;
}
