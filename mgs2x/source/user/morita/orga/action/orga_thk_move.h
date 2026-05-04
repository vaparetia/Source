/*
   orga_thk_move.h
   オルガ 思考関数 移動モード

   2000/01/05 T.Morita
   $Id: orga_thk_move.h,v 1.1.1.3 2002/11/19 11:46:23 Yoshizawa1 Exp $
*/





#if DEBUG_MODE
int hid_id = 0 ;/*DEBUG*/
extern int ORG_DbgAction ;
#endif


/*

  特別行動の際の移動を制御する
  必ず ORG_SearchHidePlace() から呼ばれる。

*/
static inline void MoveForSpecialAct( Work *work )
{
    work->stll_mtn = 0 ;/*デフォルトは立ち（基本的に立ちが多いため）*/

    if ( work->act_flg & ORGA_F_BIBLIO_ATTK )
    {
	/* 最後の語りはビハインドでやる */
	if ( (work->voice_vox>>12) > 2 )
	    work->stll_mtn = ORGA_F_IS_BEHIND ;
	work->trgt_pos  = &ORG_Ply_AttkBib[work->voice_vox>>12] ;
    }
    else if ( work->act_flg & ORGA_F_ERUDE_ATTK )
	work->trgt_pos  = &ORG_ShootErudePos ;

    else if ( work->act_flg & ORGA_F_GOUND_ATTK )
    {
	work->stll_mtn = ORGA_F_IS_SQUAT ;
	work->stll_tim = ORGA_STILL_THINKTIME ;
	work->trgt_pos = &ORG_Ply_AttkGrnd ;
    }
    else if ( work->act_flg & ORGA_F_STAND_ATTK )
    {
	work->stll_tim = ORGA_STILL_THINKTIME ;
	work->trgt_pos = &ORG_Ply_AttkStnd ;
    }

    else if ( work->act_flg & ORGA_F_HOLO_RUN_ATTK )
    {
	work->stll_tim = ORGA_STILL_THINKTIME ;
	if ( work->trgt_pos == &ORG_ShootHoloRun2 )
	    work->trgt_pos = &ORG_ShootHoloRun1 ;
	else
	    work->trgt_pos = &ORG_ShootHoloRun2 ;
    }

    /* ホロを前側を飛ばす */
    else if ( work->act_flg & ORGA_F_HOLO_BLOW_ATTK )
    {
	if ( work->misc_holo_stat )
	{
	    if ( *work->misc_holo_stat != ORGA_HOL_PLY_FREL )
		work->trgt_aim = &ORG_ShootHoloStr4 ;
	    else if ( *work->misc_holo_stat != ORGA_HOL_PLY_FRER )
		work->trgt_aim = &ORG_ShootHoloStr3 ;
	    FaceEyeAt( work, work->trgt_aim ) ;
	    work->stll_mtn = ORGA_F_IS_BEHIND| ORGA_F_IS_SQUAT ;
	    work->trgt_pos = &ORG_Ply_AttkHoloBlow ;
	}
    }

    /* 投光器をまぶしくさせてスネークを困らせる */
    else if ( work->act_flg & ORGA_F_SPOTLGT_ATTK )
    {
	work->stll_tim = 0 ;
	if ( work->trgt_pos == &ORG_Ply_AttkLightR )
	    work->trgt_pos = &ORG_Ply_AttkLightL ;
	else
	    work->trgt_pos = &ORG_Ply_AttkLightR ;
    }

    /* ホロの後ろに隠れて攻撃する */
    else if ( work->act_flg & ORGA_F_HOLO_HIDE_ATTK )
    {
	if ( work->hide_hist[1] )
#if 0
	if ( work->trgt_pos != &work->hide_pool[2].right && 
	     work->trgt_pos != &ORG_Ply_AttkHoloL )
#endif
//printf( "KOKONI KITERUUUUUUUUUU\n" ),	    
	    work->stll_tim = 0 ;
	if ( work->ply_locate>=0x170 )
	{
	    work->trgt_pos = &work->hide_pool[2].right ;
	    ORG_RenewHidePosZ( &work->hide_pool[2],
			       ORG_RecogPlayerPosNoHide( ORGA_GET_PLY_CONTROL ),
			       work->stll_mtn ) ;
	}
	else
	    work->trgt_pos = &ORG_Ply_AttkHoloL ;
    }

    /* スネークがすき間から撃っているところに攻撃するための移動 */
    else if ( work->act_flg & ORGA_F_GAP_ATTK )
    {
	static float box[] = { -10550, -16500,
			       -10550, -16500,
			       -10550, -17300,
			       -13500, -17500 } ;

	/* 左右に移動して攻撃する */
	if ( work->trgt_pos == &ORG_Ply_AttkPosL )
	    work->trgt_pos = &ORG_Ply_AttkPosR ;
	else if ( work->trgt_pos == &ORG_Ply_AttkPosR )
	    work->trgt_pos = &ORG_Ply_AttkPosL ;
	else
	    work->trgt_pos = work->stll_tim&1 ? &ORG_Ply_AttkPosL : &ORG_Ply_AttkPosR ;

	/* 立ち場所を再計算 */
	if ( work->trgt_pos == &ORG_Ply_AttkPosL )
	    work->trgt_pos->vz = ORG_CalcNewHidePosZ( &box[0], &box[2],
						      &GM_PlayerControl->mov,
						      ORGA_HIDE_SQUAT_SPHERE,
						      work->trgt_pos->vx ) ;
	else
	    work->trgt_pos->vz = ORG_CalcNewHidePosZ( &box[4], &box[6],
						      &GM_PlayerControl->mov,
						      -ORGA_HIDE_SQUAT_SPHERE,
						      work->trgt_pos->vx ) ;
	work->stll_mtn = ORGA_F_IS_SQUAT ;
	work->stll_tim = ORGA_STILL_THINKTIME ;
    }

    /* ホロを撃ちに行く時に攻撃されていたら場所を変える */
    else if ( work->act_flg & ORGA_F_HOLO_ATTK )
	work->trgt_pos = work->flag & ORGA_F_DMG_HOROFIRE ?
	    &ORG_ShootHoloPos2 : &ORG_ShootHoloPos ;
}



/*

  新しいハイドをランダムに検索する。
  この関数は,このファイル以外からも呼ばれている。

  
*/
int ORG_SearchHidePlace( Work *work, int stll_mtn )
{
    HIDE *h ;
    FVECTOR *pos = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
    int  i    ; 
    int  mask ;    /*予め決められた行ける場所*/
    int  flag = 0 ;/*プレイヤーから隠れた場所*/ ;


    /* 特別行動を優先する */
    if ( work->act_flg & ~(ORGA_F_HALF_ATTK| ORGA_F_QUAD_ATTK| ORGA_F_EIGHT_ATTK| ORGA_F_THIRD_ATTK) )
    {
	/* フラグクリア */
	work->flag &= ~( ORGA_F_SAME_HIDE  |
			 ORGA_F_DANGR_STAND|
			 ORGA_F_DANGR_LEFT |
			 ORGA_F_DANGR_RIGHT )  ;
	MoveForSpecialAct( work ) ;
	h = NULL ;
    }

    /* ここからは 普通の移動 */
    else
    {

#ifdef DEBUG_MODE
	mask = ORG_DebugMaskID() ;
	if ( !hid_id )
#else
	mask = 0x3f ;
#endif

	    /* プレイヤーの大まかな位置からオルガのいるべき場所を決める */
	    switch( work->ply_locate )
	    {
	    case 0x1ff: mask = 0x2a ; break ;
	    case 0x1fb: mask = 0x22 ; break ;
	    case 0x1fa: mask = 0x20 ; break ;
	    case 0x1f2: mask = 0x1c ; break ;
	    case 0x1f0: mask = 0x1c ; break ;
	    case 0x170: mask = 0x1e ; break ;
	    case 0x160: mask = 0x32 ; break ;
	    case  0x60: mask = 0x30 ; break ;
	    case  0x20: mask = (pos->vz > -17400) ? 0x18 : 0x05 ; break ;
	    case  0x00: mask = 0x05 ; break ;
	    }

	/* ライフがなくなってきた */
	if ( work->flag & ORGA_F_GOT_WORSE )
	    mask &= 0x3c ;

#if 0
	/* ホロが舞っているからホロに隠れる */
	if ( work->hide_spot &&
	     (work->misc_holo_stat ? *work->misc_holo_stat == ORGA_HOL_ORG_FREE : 0) )
	{
	    if ( work->hide_spot->id==2 && work->flag & ORGA_F_DANGR_LEFT )
		mask &= 0x3c ;
	    else if ( work->hide_spot->id==0 )
		mask &= 0x3c ;
	}
#endif

	if ( work->trgt_pos == &ORG_Ply_AttkGrnd )
	    mask &= 0x30 ;

	else if ( work->flag & ORGA_F_UNRECOG_POS )
	    mask &= 0x3c ;

	/* 全てのハイドを計算し直し, HIDEの有効ビットも計算する */
	h = work->hide_pool ;
	for ( i=ORGA_N_HIDE_DFLT ; --i>=0 ; h++ )
	    flag <<= 2, 
		flag |= ORG_RenewHidePosZ( h, pos, i==1 ? stll_mtn & ~ORGA_F_IS_BEHIND : stll_mtn ) ;
#if DEBUG_MODE
	printf( "FLAG(%x & %x = %x) ", flag, mask, flag & mask ) ;
#endif
	if ( !(flag &= mask) )
	    return 1 ; /* どこにも行けない */

	/* フラグクリア */
	work->flag &= ~( ORGA_F_SAME_HIDE  |
			 ORGA_F_DANGR_STAND|
			 ORGA_F_DANGR_LEFT |
			 ORGA_F_DANGR_RIGHT )  ;

	/* 乱数により 有効ビットを間引く */
	while( i=flag, flag&=irnd() ) ;
	/* 有効ビットより ハイドを選ぶ このとき h は上のFOR文でh[work->n_hide_pool]と同義 */
	for ( h-- ; !(i&3) ; i>>=2, h-- ) ;
	if ( i==3 ? irnd()&0x100 : i&1 )
	    work->trgt_pos = &h->left , work->flag |= ORGA_F_DANGR_LEFT  ;
	else
	    work->trgt_pos = &h->right, work->flag |= ORGA_F_DANGR_RIGHT ;

	if ( h->height < ORGA_DANGER_HEIGHT )
	    work->flag |= ORGA_F_DANGR_STAND ;
	if ( work->hide_spot == h )
	    work->flag |= ORGA_F_SAME_HIDE ;
    }

    /* 履歴を取る */
    work->hide_hist[1] = work->hide_hist[0] ;
    work->hide_hist[0] = work->hide_spot ;
    work->hide_spot = h ;

    work->exposed    = 0 ; /* 隠れていない時間をクリアする */
    work->n_trgt_his = 0 ; /* 移動用のヒストリーをクリアー */

    /* 必ず隠れ場所に入らなければおかしいので保険 */
    ORG_CalcHidePos( work->trgt_pos ) ;

#if DEBUG_MODE
    if ( work->trgt_pos )
    {
	printf( "X%.0f Z%.0f POS%.0f %.0f\n",
		work->trgt_pos->vx, work->trgt_pos->vz,	pos->vx, pos->vz ) ;
    }
#endif

    return 0 ;
}






static int ORG_ActThinkToMove( Work *work, void (***list)( Work * ) )
{
    int next_stll ;

    if ( (work->flag & ORGA_F_LEVEL_MSK) >= ORGA_F_LEVEL_HARD || !(irnd() & 0x3) )
	ChangeFlag( work, ORGA_F_ENB_ROUTE|ORGA_F_OFFENCE, ORGA_F_NONE ) ;/* 攻撃モードへ */
    else
	ChangeFlag( work, ORGA_F_ENB_ROUTE|ORGA_F_MODIST , ORGA_F_NONE ) ;


    /* 行動時間をリセットする */
    work->act_time = 0 ;
    work->act_tic  = 0 ;

    work->scared = 0 ;
#if DEBUG_MODE
    if ( !ORG_DebugMoveAction( work, list ) )
	return 0 ;
#endif
    next_stll = irnd()&3 ;/* 次の静止行動 */
    if ( work->rage )/*怒ったらビハインドはまずしない。*/
	next_stll &= ~ORGA_F_IS_BEHIND ;
    if ( work->act_flg & (ORGA_F_HALF_ATTK| ORGA_F_QUAD_ATTK| ORGA_F_EIGHT_ATTK) )
	next_stll |= ORGA_F_IS_BEHIND ;

    if ( ORG_SearchHidePlace( work, next_stll ) ||/*移動先が見つからない？*/
	 work->act_flg & ORGA_F_MAGAZIN_ATTK )    /*移動しない？*/
    {
	/* 条件にそぐわず 場所が近くであったら動くことはしない */
	if ( work->trgt_pos ?
	     ((int)((work->control.mov.vx - work->trgt_pos->vx)/100.0f) &&
	      (int)((work->control.mov.vz - work->trgt_pos->vz)/100.0f) ) : 1 )
	{
	    *list = ORG_ActionAbort ;
	    return 0 ;
	}
	*list++ = ORG_ActionFaceAndRun ;
    }

    /* ホロ撃ちから戻ってくる時は,ホロを飛ばす */
    else if ( work->act_flg & ORGA_F_HOLO_RUN_ATTK )
	*list++ = ORG_ActionAimmingHoloDash ;

    /* ビハインド移動 */
    else if ( (work->stll_mtn & ORGA_F_IS_BEHIND) &&
	      (next_stll      & ORGA_F_IS_BEHIND) &&
	     !(work->flag     & ORGA_F_DANGR_STAND) &&
	      (work->flag     & ORGA_F_SAME_HIDE  ) )
	*list++ = ORG_ActionBehindWalk ;

    /* しゃがみ移動 */
    else if ( work->flag & ORGA_F_DANGR_STAND && work->flag & ORGA_F_SAME_HIDE )
	*list++ = ORG_ActionCrowRun ;

    else
    { 
//printf( "MOVING %d %d\n", work->flag & ORGA_F_DMG_HOROATTK, ORG_RecogCheckHoloValid( work ) ) ;

	/* ヘトヘトなので普通移動しか出さない */
	if ( work->flag & ORGA_F_GOT_WORSE )
	    *list++ = ORG_ActionFaceAndRun ;

	/* 隠れていなかったら ダッシュ撃ち */
	else if ( /*work->flag & ORGA_F_UNRECOG_POS  &&*/ !(work->flag & ORGA_F_UNRECOG_AIM) &&
	     (work->hide_spot && work->hide_hist[0] ?
	      (	( work->hide_hist[0]->id==1 && work->hide_spot->id==0 ) ||
		( work->hide_hist[0]->id==0 && work->hide_spot->id==1 ) ) : 0) )
	{
	    if ( (work->voice_vox>>12)>=ORGA_N_STREAM )
		*list++ = ORG_ActionRolloutRun ;
	    *list++ = work->rage  ? ORG_ActionAimmingDashWithRage : ORG_ActionAimmingDash ;
	}

	else if ( /*work->flag & ORGA_F_UNRECOG_POS  &&*/ !(work->flag & ORGA_F_UNRECOG_AIM) &&
	     (work->hide_spot && work->hide_hist[0] ?
	      ( ( work->hide_hist[0]->id==2 && work->hide_spot->id==0 ) ||
		( work->hide_hist[0]->id==2 && work->hide_spot->id==1 ) ||
		( work->hide_hist[0]->id==1 && work->hide_spot->id==0 ) ||
		( work->hide_hist[0]->id==0 && work->hide_spot->id==1 ) ) : 0) )
	    *list++ = work->rage ? ORG_ActionAimmingDashWithRage : ORG_ActionAimmingDash ;

	/* 人生語り中なので普通移動 */
	else if ( (work->voice_vox>>12) < ORGA_N_STREAM )
	{
	    if ( (work->flag & ORGA_F_LEVEL_MSK) > ORGA_F_LEVEL_NORM )
		*list++ = ORG_ActionFaceAndDash ;
	    else
		*list++ = ORG_ActionFaceAndRun ;
	}
#if 0 /* とりあえず 2001.4.11 */
	else if ( (work->hide_spot && work->hide_hist[0] ?
		   ( ( work->hide_hist[0]->id==2 && work->hide_spot->id==0 ) ||
		     ( work->hide_hist[0]->id==1 && work->hide_spot->id==0 ) ||
		     ( work->hide_hist[0]->id==0 && work->hide_spot->id==1 ) ) : 0) )
	    *list++ = ORG_ActionRunSliding ;
#endif
	/*普通の移動*/
	else if ( GV_Time & 1 )
	{
	    if ( (work->flag & ORGA_F_LEVEL_MSK) > ORGA_F_LEVEL_NORM )
		*list++ = ORG_ActionFaceAndDash ;
	    else
		*list++ = ORG_ActionFaceAndRun ;
	}
	else
	{
#if 0 /* とりあえず 2001.4.11 */
	    *list++ = ORG_ActionRunTumble, *list++ = ORG_ActionDashTumble ;
#endif /* とりあえず 2001.4.11 */
	    *list++ = ORG_ActionCrowRun ;
	}
    }
    *list = NULL ;
    work->stll_mtn = next_stll ;

    return 1 ;
}
