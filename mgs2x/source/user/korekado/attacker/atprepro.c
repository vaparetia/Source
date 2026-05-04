/*
	atprepro.c
	攻撃兵、思考前処理

	1997/07/07 Y.Korekado
	$Id: atprepro.c,v 1.1.1.3 2002/11/19 11:43:59 Yoshizawa1 Exp $
	
*/

static void PreProcess( Work *work )
{
	ENETHINK	*entk ;
	
	entk = &work->enethink ;
	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;
	ENE_SetAlertLevel( entk ) ;
}
