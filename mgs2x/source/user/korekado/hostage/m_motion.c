/*
	m_motion.c
	モーションメモリ再生処理

	2001/03/21 Y.Korekado
	$Id: m_motion.c,v 1.1.1.3 2002/11/19 11:44:18 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------*/

typedef struct {
	int			mot_name ;		/* モーション名 */
	int			mot_num ;		/* モーション数 */
	int			model ;			/* モデル名 */
	int			mot_flam ;		/* 総モーションフレーム */
	OBJECT		*body_mtbuff ;	/* モーションバッファ用 */
	FVECTOR		*step_buff ;

	u_short		*m_ptr ;
	u_short		*m_len ;
	u_short		*m_height ;
	FVECTOR		*m_buff ;
	FVECTOR		*m_step ;
} MEMMOT ;

typedef struct {
	int interp_count
} MEMMOT_CTRL ;

/*----------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	MMT_JOINT	(21)	/* モデル関節数 */

/*----------------------------------------------------------------*/

typedef struct {
	FVECTOR			prev, c_prev ;	/* 補間元クォータニオン＆補正用クォータニオン */
	FVECTOR			next, c_next ;	/* 補間先クォータニオン＆補正用クォータニオン */
	FVECTOR			t ;
} SQUAD_WORK ;

typedef struct {
	FVECTOR	from[4] ;
	FVECTOR	to[4] ;
	float	t[4] ;
	float	tmp[4], param_x[4], param_y[4] ;
	float	scale_from[4], scale_to[4] ;
	float	omega[4] ;
	float	cosom[4] ;
	float	sinom[4] ;
} SLERP4_WORK;

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
	SLERP4_WORK	slerp4_work ;
	SQUAD_WORK	squad_work[64] ;
} ScrPadWork ;

/*----------------------------------------------------------------*/
FVECTOR	*MMT_GetAbsRot( MEMMOT_CTRL *mmt_ctrl )
{
	int mas, m_time ;
	FVECTOR	*abs ;

	mas = mmt_ctrl->mot_num ;

	m_time = mmt_ctrl->m_time % Memmot->m_len[mas] ;

	if ( mmt_ctrl->reverse_flag ) {
		m_time = Memmot->m_len[mas] - m_time ;
//if ( hld->id == 0 )printf("reverse m_time [%d]\n",m_time ) ;
	}
	abs = Memmot->m_buff + (MMT_JOINT*(Memmot->m_ptr[mas]+m_time)) ;

	return abs ;
}

void	MMT_SetMotion( MEMMOT_CTRL *mmt_ctrl, int num, int intrp )
{
	FVECTOR	*abs ;

	if ( mmt_ctrl->m_time>0) mmt_ctrl->m_time-- ;
	abs = MMT_GetAbsRot( mmt_ctrl ) ;

	mmt_ctrl->mot_num = num ; 
	mmt_ctrl->m_time = 0;
	mmt_ctrl->interp_count = intrp ;
	mmt_ctrl->interp_inc = intrp ;
	mmt_ctrl->interp_time = 0.0f ;
	mmt_ctrl->reverse_flag = 0 ;

	KR_MemCopy( &mmt_ctrl->abs_rots[0], abs, sizeof(FVECTOR), MMT_JOINT ) ;
}

	/*
		クォータニオンの線形補間（正規化しないので注意）
	*/
static	void MMT_QuatSlerp( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
{
	float	to1[4] ;
	float	cosom  ;	/* 本当はdouble型の方がいい */
	float	scale0, scale1 ;		/* 本当はdouble型の方がいい */

	/* 内積を求める */
	cosom = from->vx * to->vx + from->vy * to->vy + from->vz * to->vz + from->vw * to->vw ;

	/* 符号をそろえる */
	if ( cosom < 0.0F ){
		cosom = -cosom ;
		to1[0] = - to->vx ;
		to1[1] = - to->vy ;
		to1[2] = - to->vz ;
		to1[3] = - to->vw ;
	} else  {
		to1[0] = to->vx ;
		to1[1] = to->vy ;
		to1[2] = to->vz ;
		to1[3] = to->vw ;
	}

	/* 係数を求める */
	/* ２つの角度の差が小さすぎるときには線形補間で求める */
	scale0 = 1.0F - t ;
	scale1 = t ;

	/* 係数を使って計算 */
	to1[0] = scale0 * from->vx + scale1 * to1[0] ;
	to1[1] = scale0 * from->vy + scale1 * to1[1] ;
	to1[2] = scale0 * from->vz + scale1 * to1[2] ;
	to1[3] = scale0 * from->vw + scale1 * to1[3] ;
	res->vx = to1[0] ;
	res->vy = to1[1] ;
	res->vz = to1[2] ;
	res->vw = to1[3] ;
}

static	FVECTOR	*MMT_ActMotion( MMT_CTRL *mmt_ctrl, abs )
{
	FVECTOR	*old_abs ;
	int	i ;

	if ( mmt_ctrl->interp_count <= 0 ) return abs ;

	/* 補間処理 */
	old_abs = &mmt_ctrl->abs_rots[0] ;
	for ( i = MMT_JOINT ; i > 0 ; i-- ){
		MMT_QuatSlerp( old_abs, old_abs, abs, mmt_ctrl->interp_time );
		abs++ ;
		old_abs++ ;
	}

	if ( mmt_ctrl->interp_count ){
		mmt_ctrl->interp_time += (float)TIME_BASE / (float)mmt_ctrl->interp_inc ;
		if ( ( mmt_ctrl->interp_count -= TIME_BASE ) <= 0 ){
			mmt_ctrl->interp_count = 0 ;
		}
	}

	return &mmt_ctrl->abs_rots[0] ;
}

static FVECTOR	*MMT_AdjustMotion( MMT_CTRL *mmt_ctrl, FVECTOR *abs )
{
	FVECTOR	quat, *old_abs ;
	SVECTOR rot ;

	if ( (mmt_ctrl->adj_y == 0) && (mmt_ctrl->adj_turn_y == 0) 
			&& (mmt_ctrl->adj_x == 0) && (mmt_ctrl->adj_turn_x == 0) ) return abs ;

	if ( mmt_ctrl->adj_turn_y != mmt_ctrl->adj_y ) {
		mmt_ctrl->adj_y = GV_NearExp8P ( mmt_ctrl->adj_y, mmt_ctrl->adj_turn_y ) ;
	} 
	if ( mmt_ctrl->adj_turn_x != mmt_ctrl->adj_x ) {
		mmt_ctrl->adj_x = GV_NearExp8P ( mmt_ctrl->adj_x, mmt_ctrl->adj_turn_x ) ;
	}

	/* アジャスト処理 */
	old_abs = &mmt_ctrl->abs_rots[ HUMAN21_ATAMA ] ;
	abs += HUMAN21_ATAMA ;

	rot.vx = hld->adj_x ;
	rot.vy = hld->adj_y ;
	rot.vz = 0 ;

	GM_RotToQuat( &rot, &quat ) ;
	MT_QuatMul( old_abs, &quat, abs );

	return &mmt_ctrl->abs_rots[0] ;
}

static void MMT_ActMotion2( MOTION_CONTROL *m_ctrl, DG_OBJS *objs, FVECTOR *rots )
{
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;

	if ( objs->root != NULL )	scrpad->root_mat = *( objs->root );
	else						scrpad->root_mat = objs->world ;

	MT_StartMemToSpr( scrpad->joints, rots, m_ctrl->n_joints );
	MT_WaitMemToSpr();

	{/* オブジェクトにマトリクスを設定する */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		DG_DEF	*def ;
		DG_MDL	*mdl ;
		DG_OBJ	*obj = objs->objs ;
		int		i ;

		def = objs->def ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = def->n_models ; i > 0 ; i-- ){	/* 拡張モデルは無視する */
				FMATRIX	*parent ;
				mdl = obj->model ;
				MT_QuatToMat( mats, joints );
				sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
				scrpad->vec.vx = mdl->tx ;
				scrpad->vec.vy = mdl->ty ;
				scrpad->vec.vz = mdl->tz ;
				parent = &scrpad->mats[ mdl->parent ] ;
				sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
				obj->world = *mats ;
				obj++ ;
				mats++ ;
				joints++ ;
			}
		}
	}
}

void	MMT_MakeMotion( MMT_CTRL *mmt_ctrl )
{
	FVECTOR	*abs ;
	abs = MMT_GetAbsRot( mmt_ctrl ) ;

	abs = MMT_ActMotion( mmt_ctrl, abs ) ;
	abs = MMT_AdjustMotion( mmt_ctrl, abs ) ;
	MMT_ActMotion2( Memmot->body_mtbuff[mmt_ctrl->mot_num].m_ctrl, 
				mmt_ctrl->body->objs, abs ) ;

	mmt_ctrl->m_time ++ ;
}

int MMT_MotionPreCalloc( MEMMOT *mmt, int motion, int mot_num, int model, int mot_flam )
{
	int	i, len, sum ;
	OBJECT	*body, *body_mtbuff ;
	FVECTOR	*p, *s, *step_buff ;
	u_short *h ;

	mmt->mot_num = mot_num ;

	/* オブジェクトバッファ確保 １モーションに１オブジェクト必要 */
    if( (mmt->body_mtbuff = (FVECTOR *)GV_Malloc(sizeof(OBJECT) * mot_num ))==NULL) {
		printf(" NO MEMORY !! Memory Motion Object Buffer !!\n");
		return -1 ;
	}
	/* ステップバッファ確保 １モーションに１つ必要 */
    if( (mmt->step_buff = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * mot_num ))==NULL) {
		printf(" NO MEMORY !! Memory Motion Step Buffer !!\n");
		return -1 ;
	}

	/* モーション保管用オブジェクト初期化 */
	body_mtbuff = mmt->body_mtbuff ;
	step_buff = mmt->step_buff ;
	for( i=0; i<mot_num; i++ ) {
		GM_InitObject( body_mtbuff, model, BODY_FLAG );
		GM_ConfigObjectMotion( body_mtbuff, 1, motion, MT_FLAG_HUMAN2 );
		GM_ConfigObjectStep( body_mtbuff, step_buff ) ;
		body_mtbuff->objs->flag |= DG_FLAG_INVISIBLE ;

		/* sar ファイルを使用しない プリ計算時にカメラ関係の初期化がまだなので落ちちゃう */
		if ( body_mtbuff->m_ctrl->sar_ctrl != NULL ) {
			GV_Free( body_mtbuff->m_ctrl->sar_ctrl ) ;
			body_mtbuff->m_ctrl->sar_ctrl = NULL ;
		}
	}

	/* モーションバッファ */
    if( (mmt->m_ptr = (u_short *)GV_Malloc(sizeof(u_short) * mot_num))==NULL) {
		printf(" NO MEMORY !! Memory Motion Pointer !!\n");
		return -1 ;
	}
    GV_ZeroMemory(mmt->m_ptr, sizeof(u_short) * mot_num );

    if( (mmt->m_len = (u_short *)GV_Malloc(sizeof(u_short) * mot_num))==NULL) {
		printf(" NO MEMORY !! Memory Motion Length !!\n");
		return -1 ;
	}
    GV_ZeroMemory(mmt->m_len, sizeof(u_short) * mot_num );

    if( (mmt->m_buff = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * MMT_JOINT * mot_flam ))==NULL) {
		printf(" NO MEMORY !! Memory Motion Motion Buffer !!\n");
		return -1 ;
	}
    GV_ZeroMemory(mmt->m_buff,	sizeof(FVECTOR) * MMT_JOINT * mot_flam );

    if( (mmt->m_step = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * mot_flam ))==NULL) {
		printf(" NO MEMORY !! Memory Motion Step Buffer !!\n");
		return -1 ;
	}
    GV_ZeroMemory(mmt->m_step,sizeof(FVECTOR)*mot_flam );

    if( (mmt->m_height = (u_short *)GV_Malloc(sizeof(u_short) * mot_flam))==NULL) {
		printf(" NO MEMORY !! Memory Motion Height Buffer !!\n");
		return -1 ;
	}
    GV_ZeroMemory(mmt->m_height,sizeof(u_short) * mot_flam );

	/* ダミーSEモーションテーブル */
    MT_SetMotionSeTable( mmt->body_mtbuff->m_ctrl, GM_CurrentMap, 1, 0, 0 ) ;

	sum = 0 ;
	p = mmt->m_buff ;
	s = mmt->m_step ;
	h = mmt->m_height ;
	body = mmt->body_mtbuff ;
	DG_SetPos( &DG_UnitMatrix ) ;
	for( i=0; i<mot_num; i++ ) {
		mmt->m_ptr[i] = sum ;
printf(" MOTION [%d] [%d] \n",i,mmt->m_ptr[i]) ;
		GM_ConfigObjectAction( body, 0, i, 0, 0xfffff, 0 );
		len = COUNT_VMODE(body->m_ctrl->mt3_ctrl[0].file_header->motion_length) ;
		mmt->m_len[i] = len ;
		sum += len ;
		while( len-- > 0 ){
			DG_SetPos( &DG_UnitMatrix ) ;
			GM_ActObject( body ) ;

			KR_MemCopy( p, body->m_ctrl->abs_rots, sizeof(FVECTOR), MMT_JOINT ) ;
			KR_MemCopy( s, body->step, sizeof(FVECTOR), 1 ) ;
			*h = body->height ;

			p += MMT_JOINT ;
			s++ ;
			h++ ;
			body++ ;
		}
	}
	
	printf(" MOTION DATA [%d] \n",sum) ;
	if( sum >= mot_flam ) {
		printf("hostage: Err Motion Buff Over [%d] / [%d]!!\n",sum, mot_flam );
	}

	return 0 ;
}

