/*
	spprepro.c
	サポート兵前処理

	1999/08/30 Y.Korekado
	$Id: spprepro.c,v 1.1.1.3 2002/11/19 11:44:25 Yoshizawa1 Exp $
	
*/

static void PreProcess( Work *work )
{
	ENETHINK	*entk ;
	
	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;

	entk = &work->enethink ;
	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;
	ENE_EyeInfoCheck( entk, &entk->bd_eyei ) ;
	ENE_NoticeCheck( entk ) ;					/* noticeチェック */
	ENE_SetAlertLevel( entk ) ;					/* 危険値計算 */
}
