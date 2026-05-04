/*
	attachment_debug.h
		装備品揺らしデバッグパラメータ

	1999/11/05 K.Kano
	$Id: attachment_debug.h,v 1.1.1.3 2002/11/19 11:43:04 Yoshizawa1 Exp $
*/

#ifndef _attachment_debug_h_
#define _attachment_debug_h_

#define CLOCK_PER_HSYNC		(300000000.0f/60.0f/270.0f)
#define DEBUG_BUTTON_CHECK()	(GV_PadData[1].press & PAD_L1)

#endif
