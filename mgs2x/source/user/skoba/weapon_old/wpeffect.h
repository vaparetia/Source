/*
	wpeffect.h
	武器エフェクトヘッダ

	1999/12/03 M.Matsuzaki
	2001/06/04 S.Kobaysahi
*/

#ifndef ___WPEFFECT_H___
#define ___WPEFFECT_H___

enum {									/* blowback kinds */
	WEF_ID_NONE=0,
	WEF_ID_USP,
	WEF_ID_MKR,
	WEF_ID_FAMAS,
	WEF_ID_M92,
	WEF_ID_SOCOM,
	WEF_ID_SHOTGUN,
	WEF_ID_GLK,
	WEF_ID_ABK,
	WEF_ID_M92_Sea,
	WEF_ID_RGB6,
	WEF_ID_M4_GRD_HTC,
	WEF_ID_MAX
};

/* ---ブローバック情報構造体(内部使用)--- */
typedef struct _BLOWBACK_INFO {
	void 		(*func)(void *);		/* 武器関数 */
	int 		motionnum;					/* モーションのフレーム数 */
	int 		reload_waittime;			/* リロード用 */
} BLOWBACK_INFO ;

/* ---フラグその２(内部使用)--- */
#define WPEF_FLG2_SHOTFIRST	(0x00000001)	/* まだ一発も撃ってない常態 */
#define WPEF_FLG2_RELOAD	(0x00000002)	// RGB６等特殊reload常態 by koba4
//#define WPEF_FLG2_RELOADNOW	(0x00000002)	/* リロード中 */

/* リロードウェイト値 */
#define USP_WAIT_COUNT		(30)
#define GLK_WAIT_COUNT		(40)
#define AK_WAIT_COUNT		(40)
//#define M92_WAIT_COUNT		(80)
#define M92_WAIT_COUNT		(36)
#define SHOTGUN_WAIT_COUNT	(12)

#endif
