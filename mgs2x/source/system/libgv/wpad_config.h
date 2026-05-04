/*--------------------------------------------------------------*/
/*	wpad_config.h												*/
/*					入力コンフィグ管理							*/
/*						2002/11/09	Takaki Eiji					*/
/*--------------------------------------------------------------*/
#ifndef	__WPAD_CONFIG_H__
#define	__WPAD_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __WPAD_CONFIG_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

/*--------------------------------------------------------------*/
/*	Typedef														*/
/*--------------------------------------------------------------*/

	/*----------------------------------------------------------*/
	/*	Keyboard												*/
	/*----------------------------------------------------------*/
typedef struct	WPAD_KEYBOARD_CNV_
{
	DWORD	key_id ;	// 入力キー(DIK_A等)
	DWORD	button_id ;	// 変換ボタン(PAD_A等)
} WPAD_KEYBOARD_CNV;


typedef struct	WPAD_KEYBOARD_CFG_
{
	DWORD				cnv_num ;	// 変換テーブル数
	WPAD_KEYBOARD_CNV	*cnv;		// 変換テーブル
} WPAD_KEYBOARD_CFG;

	/*----------------------------------------------------------*/
	/*	PAD														*/
	/*----------------------------------------------------------*/
typedef struct	WPAD_PAD_BUTTON_CNV_
{
	DWORD	button_id ;	// 変換ボタン(PAD_A等)
} WPAD_PAD_BUTTON_CNV;

typedef struct	WPAD_PAD_ANALOG_CNV_
{
	#define	WPAD_PAD_ANALOG_CNV_OFS_UNUSE	(0xffffffff)	// 未使用フラグ
	DWORD	ofs ;		// 設定数値の入っているオフセット(DIJOYSTATE用)
	DWORD	ofs2 ;		// 設定数値の入っているオフセット(DIJOYSTATE2用)

	DWORD	reverse ;	// 入力数値反転
} WPAD_PAD_ANALOG_CNV;

typedef struct	WPAD_PAD_CFG_
{
	#define	 WPAD_PAD_BUTTON_CNV_NUM	(32)
	WPAD_PAD_BUTTON_CNV	button_cnv[WPAD_PAD_BUTTON_CNV_NUM] ;

	#define	 WPAD_PAD_ANALOG_CNV_NUM	(4)
	WPAD_PAD_ANALOG_CNV	analog_cnv[WPAD_PAD_ANALOG_CNV_NUM] ;
} WPAD_PAD_CFG;

/*--------------------------------------------------------------*/
/*	extern														*/
/*--------------------------------------------------------------*/

EXTERN	WPAD_KEYBOARD_CFG	GV_KeyboardCfg ;
EXTERN	WPAD_KEYBOARD_CFG	GV_KeyboardCfgS ;

EXTERN	WPAD_PAD_CFG		GV_PadCfg ;
EXTERN	WPAD_PAD_CFG		GV_PadCfgS ;

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/
extern	BOOL	GV_InitWPadConfig(void) ;
extern	BOOL	GV_ReleaseWPadConfig(void) ;

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif	/* !defined(__WPAD_CONFIG_H__) */
