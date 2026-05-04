/*
	head.h
	
	2002/8/2 Y.Korekado
	$Id: head.h,v 1.3 2002/11/23 12:42:27 Yoshizawa1 Exp $
*/

/* head */
#define	GLL_HEAD_JOIN		0x00000001
#define	GLL_HEAD_TRG_SKIP	0x00000002
#define	GLL_HEAD_IN_BODY	0x00000004
#define	GLL_HEAD_OUT_BODY	0x00000008
#define	GLL_HEAD_BREAK		0x00000010
#define	GLL_HEAD_REGENE		0x00000020
#define	GLL_HEAD_LEFT_BODY	0x00000040
#define	GLL_HEAD_LIT_BREAK	0x00000080

/* gll */
#define	GLL_HEAD_OFF		0x00010000
#define	GLL_HEAD_NEXT		0x00020000
#define	GLL_HEAD_NO_LIT		0x00040000
