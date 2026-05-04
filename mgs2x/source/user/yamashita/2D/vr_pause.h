/*******************************************************************************
 * vr_pause - vr_pause.h
 * ＶＲポーズ  *NewVRPause
 * 2002/05/24 S.Yamashita
 * $Id: vr_pause.h,v 1.1.1.3 2002/11/19 11:51:44 Yoshizawa1 Exp $
 */

#ifndef __INC_VR_PAUSE__
#define __INC_VR_PAUSE__

/******************************************************************************
 * defines
 */

#define VRPAU_STRCODE (3551242)		/* vr_pause */

/* シグナルタイプ */
enum
{
	VRPAU_OFF = VRPAU_STRCODE,	/* 終了 */
	VRPAU_ON,					/* 開始 */
};

/*******************************************************************************
 * functions
 */

void *NewVRPause(int proc_restart, int proc_exit);

/*******************************************************************************
 */

#endif	/* __INC_VR_PAUSE__ */
