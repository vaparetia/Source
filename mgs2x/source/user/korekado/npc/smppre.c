/*
	smppre.c
	ＮＰＣサンプル思考前処理

	2000/02/09 Y.Korekado
	$Id: smppre.c,v 1.1.1.3 2002/11/19 11:44:23 Yoshizawa1 Exp $
*/
enum {
	SMP_MESSAGE_MODE_CHANGE,
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
			case SMP_MESSAGE_MODE_CHANGE :
				work->mode = msg->message[ 1 ] ;
				if ( work->mode == SMP_MODE_BOMB ) {
					work->bomb_zone = msg->message[ 2 ] ;
				}
			break ;
			default :
			break ;
		}
		msg++ ;
	}
}

static	void	InfoCheck( Work *work )
{
	FVECTOR	vec ;
	NPCWORK	*npc ;

	npc = &work->npc ;

	_sceVu0SubVector(  &vec, &GM_PlayerPosition, &npc->ctrl->mov ) ;
	work->pl_dis = _FVecLen3( &vec ) ;	/* プレイヤーとの距離 */
	work->pl_dir = _FVecDir2( &vec ) ;	/* プレイヤーへの方向 */
}
/*----------------------------------------------------------------*/
static void PreProcess( Work *work )
{
	CheckMessage( work ) ;

	InfoCheck( work ) ;		/* 周りの情報チェック */
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
		GM_SeSetMode( SD_E_EHEART01, &npc->ctrl->mov, GM_SEMODE_MIC ) ;
	}
}
