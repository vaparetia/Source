/*
	security.h
	防御システム（監視カメラ、赤外線センサー、サイファー）
	
	1999/12/21 Y.Korekado
	$Id: security.h,v 1.1.1.3 2002/11/19 11:44:10 Yoshizawa1 Exp $
	
*/
#ifndef __SECURITY___
#define	__SECURITY___ 1

typedef	struct	_security__{
	short	secure_id ;
	short	alert_level ;	/* 危険値 */
} SECURITY ;
#endif
