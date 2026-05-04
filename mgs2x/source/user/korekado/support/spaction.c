/*
	spaction.c
	見張り兵のアクションコントロール

	1999/08/30 Y.Korekado
	$Id: spaction.c,v 1.1.1.3 2002/11/19 11:44:24 Yoshizawa1 Exp $
	
*/
/*--------------------------------------------------------------------*/
static	void	Action ( work )
Work	*work ;
{
	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	OBJECT		*body ;

	entk = &work->enethink ;
	ctrl = &work->control ;
	body = &work->body ;

	ENE_ActInit( entk ) ;
	AT_Action( &work->action ) ;
	ENE_ActStatusCheck( entk ) ;

	ENE_Gravitation( entk ) ;
}
