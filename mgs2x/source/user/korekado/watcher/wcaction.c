/*
	wcaction.c
	見張り兵のアクションコントロール

	1999/07/07 Y.Korekado
	$Id: wcaction.c,v 1.1.1.3 2002/11/19 11:44:30 Yoshizawa1 Exp $
	
*/
/*--------------------------------------------------------------------*/

void ENE_GunAction( ENETHINK *entk );

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

	ENE_GunAction( entk ) ;

	ENE_ActStatusCheck( entk ) ;

	ENE_Gravitation( entk ) ;
}
