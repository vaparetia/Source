/*
    rankenc.h
	ランクエンコーダーのヘッダー
	2000/11/10 T.Shibata

	$Id: rankenc.h,v 1.1.1.3 2002/11/19 11:48:44 Yoshizawa1 Exp $
*/

#ifndef RANKENC_H
#define RANKENC_H

typedef struct {
	int data;		// 32bitまで
	int len;		// 32まで
} INPUT_DATA;

extern void decode( INPUT_DATA *input, int num, char *string );
extern void encode( char *result, INPUT_DATA *input, int num, int seed );

#endif
