/*
	subtitle.h
		タイトル画面共通の字幕の配置

	2001/06/15 K.Kano
	$Id: subtitle.h,v 1.1.1.3 2002/11/19 11:43:41 Yoshizawa1 Exp $
*/


#ifndef _subtitle_h_
#define _subtitle_h_


#define SUBTITLE_FONT_WIDTH				24
#define SUBTITLE_FONT_HEIGHT			24
#define SUBTITLE_FONT_DISP_WIDTH		16
#define SUBTITLE_FONT_DISP_HEIGHT		16

#define SUBTITLE_X		60
#define SUBTITLE_Y		((350-4)-12)      //BP JG - bump up the subtitles to avoid the safe zone.

#define SUBTITLE_TEXTCOLOR				0x80808080


#endif
