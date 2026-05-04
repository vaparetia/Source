/*
	capture.h
		デジカメで画像を取り込み圧縮するためのモジュール
	2001/05/21	K.Uehara
	$Id: capture.h,v 1.1.1.3 2002/11/19 11:50:14 Yoshizawa1 Exp $
*/	


#ifndef _capture_h_
#define _capture_h_

#define THUMB_W						48
#define	THUMB_H						32

#ifdef KP_XBOX
#define THUMB_W_XBOX				64
#define	THUMB_H_XBOX				32
#endif
											// r_tnk0.gclを変更する必要あり

#define TANKER_MAX_CAPTURE			6		// タンカー編での最大キャプチャ数
											// r_tnk0.gclを変更する必要あり
#define TANKER_MAX_CODE				(16*1024)
											// r_tnk0.gclを変更する必要あり

/* デジカメでキャプチャ */
void *NewCaptureStart( int save_flag, void *thumbnail, void *code, int max_code, int photoNum );

#define CAPTURE_NO_SAVE	0
#define CAPTURE_SAVE	1

#define CAPTURE_DIGITAL_PHOTO_NUM (-1)


/*
	save_flag	: セーブにいくかどうか。
	thumbnail	: サムネイル格納領域。(THUMB_W * THUMB_H *sizeof(short))
	code		: 圧縮データ格納領域。( max_code )
	max_code	: 圧縮データ格納領域の最大サイズ。

	thumbnail, code, max_codeはタンカー編デジカメのみ指定。
	普通のデジカメはNULL, 0で設定しておくこと。
*/


#endif
