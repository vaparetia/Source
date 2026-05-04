/*
   lckcnct.c
   ロッカー接合

   2000/09/01 M.Sonoyama
   $Id: lckcnct.c,v 1.1.1.3 2002/11/19 11:50:49 Yoshizawa1 Exp $
*/

typedef	struct	{
	int		right_name ;
	int		left_name ;
} LCKCNCT ;

#define	MAX_LCKCNCTS	(4)

static	int		N_LockerConnections = 0 ;
static	LCKCNCT	LockerConnection[ MAX_LCKCNCTS ] ;
static	LOCKER2	*Next = NULL ;

enum {
	LCKCNCT_R = 0,
	LCKCNCT_L = 1,
} ;

/* シナリオコマンド */
int		NewLockerConnection( void )
{
	LCKCNCT		*cnct ;

	ASSERT( N_LockerConnections < MAX_LCKCNCTS ) ;
	cnct = &LockerConnection[ N_LockerConnections ] ;
	
	GCL_GetOption( 'c' ) ;
	cnct->right_name = GCL_GetNextInt() ;
	cnct->left_name = GCL_GetNextInt() ;

	N_LockerConnections ++ ;

	return 1 ;
}

static LOCKER2	*SearchLocker2( int name ) ;

/* 指定ロッカーの指定方向につながっている
   ロッカー構造体を返す */
static	LOCKER2	*SearchConnectLocker( int name, int dir )
{
	LOCKER2			*find ;
	int				i ;
	LCKCNCT			*cnct ;

	if ( N_LockerConnections == 0 ) return NULL ;
	find = NULL ;
	cnct = LockerConnection ;
	for ( i = 0; i < N_LockerConnections; i ++, cnct ++ ) {
		if ( cnct->right_name == name && dir == LCKCNCT_L ) {
			printf( "find left %s\n", cnct->left_name ) ;
			find = SearchLocker2( cnct->left_name ) ;
		} else if ( cnct->left_name == name && dir == LCKCNCT_R ) {
			printf( "find right %s\n", cnct->right_name ) ;
			find = SearchLocker2( cnct->right_name ) ;
		}
		if ( find != NULL ) {
			if ( !( find->status & LOCKER_STATE_CLOSE ) ) find = NULL ;
		}
	}
	return find ;
}

