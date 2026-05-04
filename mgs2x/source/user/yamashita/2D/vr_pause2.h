/*******************************************************************************
 * vr_pause2 - vr_pause2.h
 * ＶＲポーズ２  *NewVRPause2
 * 2002/07/25 S.Yamashita
 * $Id: vr_pause2.h,v 1.1.1.3 2002/11/19 11:51:44 Yoshizawa1 Exp $
 */

#ifndef __INC_VR_PAUSE2__
#define __INC_VR_PAUSE2__

/******************************************************************************
 * defines
 */

#define VRPAU2_STRCODE (12976504)		/* vr_pause2 */

/* シグナルタイプ */
enum
{
	VRPAU2_OFF = VRPAU2_STRCODE,	/* 終了 */
	VRPAU2_ON,						/* 開始 */
};

/*******************************************************************************
 * functions
 */

void *NewVRPause2(int proc_restart, int proc_exit);

/*******************************************************************************
 */

#endif	/* __INC_VR_PAUSE2__ */
