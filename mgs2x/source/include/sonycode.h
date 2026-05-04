/*
	sonycode.h
	SCEからの商品番号に依存するマクロ定義
	$Id: sonycode.h,v 1.1.1.3 2002/11/19 11:42:00 Yoshizawa1 Exp $
*/

#ifdef ENGLISH
// 英語版
#define SONY_CODE		"SLUS-20144"		// メモリーカード,HDD用
#define DISC1_SONY_CODE	"SLUS_201.44"		// ブートファイル用
#else
// 日本版
#define SONY_CODE		"SLPM-65078"		// メモリーカード,HDD用
#define DISC1_SONY_CODE	"SLPM_650.78"		// ブートファイル用
#endif

