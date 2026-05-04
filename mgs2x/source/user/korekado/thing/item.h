/*
	item.h

	2000/06/17 Y.Korekado
	$Id: item.h,v 1.1.1.3 2002/11/19 11:44:28 Yoshizawa1 Exp $
*/

#ifndef __utilh_____
#define __utilh_____

#define KR_MAX_ITEM	4
typedef	struct	{
	int		proc[ KR_MAX_ITEM ] ;
	u_char	probability[ KR_MAX_ITEM ] ;
	short	c_proc ;
	short	n_proc ;
	int		name ;
	char	*str ;
} ITEM_PROC ;


extern	void	KRTH_GetItemProc( ITEM_PROC	*item_proc, int name ) ;
extern	int		KRTH_PutDogTagItemProc( ITEM_PROC	*item_proc, FVECTOR *pos, int status, float n, int ) ;
extern	void	KRTH_PutItemProc( ITEM_PROC	*item_proc, FVECTOR *pos, int status, float n ) ;
#endif
