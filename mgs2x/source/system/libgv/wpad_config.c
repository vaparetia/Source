/*--------------------------------------------------------------*/
/*	wpad_config.c												*/
/*					入力コンフィグ管理							*/
/*						2002/11/09	Takaki Eiji					*/
/*--------------------------------------------------------------*/
#define	__WPAD_CONFIG_C__

#include <stdio.h>
#include <stdlib.h>

#include <xtl.h>

#define DIRECTINPUT_VERSION         0x0800
#include <dinput.h>

#include "libfs.h"
#include "libgv.h"

#include "wpad_config.h"

/*--------------------------------------------------------------*/
/*	define														*/
/*--------------------------------------------------------------*/
#define	KBCFG_FILENAME		"kb.ini"	// Keyboard Config File
#define	KBCFGS_FILENAME		"kbs.ini"	// Keyboard Config File(主観モード)

#define	PADBTNCFG_FILENAME	"padbtn.ini"	// PAD Button Config File
#define	PADBTNCFGS_FILENAME	"padbtns.ini"	// PAD Button Config File(主観モード)

#define	PADANACFG_FILENAME	"padana.ini"	// PAD Analog Config File
#define	PADANACFGS_FILENAME	"padanas.ini"	// PAD Analog Config File(主観モード)

/*--------------------------------------------------------------*/
/*	macro														*/
/*--------------------------------------------------------------*/

#define	isSpaceChara(ch_)	((((ch_) > 0x00) && ((ch_) < 0x20)) || ((ch_) == (DWORD)' '))
#define	isTermChara(ch_)	((ch_) == 0)


#define	ALLOC(size_)	malloc((size_))
#define	FREE(addr_)		free((addr_))

/*--------------------------------------------------------------*/
/*	Default Keyboad Config										*/
/*--------------------------------------------------------------*/
static	WPAD_KEYBOARD_CNV	default_KeyboardCnvTbl[] =
{
	{	DIK_NUMPAD8,	PAD_U	},
	{	DIK_NUMPAD2,	PAD_D	},
	{	DIK_NUMPAD4,	PAD_L	},
	{	DIK_NUMPAD6,	PAD_R	},
	{	DIK_UP,			PAD_U	},
	{	DIK_DOWN,		PAD_D	},
	{	DIK_LEFT,		PAD_L	},
	{	DIK_RIGHT,		PAD_R	},

	{	DIK_Z,			PAD_B	},
	{	DIK_X,			PAD_A	},
	{	DIK_S,			PAD_Y	},
	{	DIK_D,			PAD_X	},
	{	DIK_C,			PAD_Y	},
	{	DIK_V,			PAD_X	},
	{	DIK_NUMPAD0,	PAD_B	},

	{	DIK_A,			PAD_L1	},
	{	DIK_F,			PAD_R1	},
	{	DIK_W,			PAD_L2	},
	{	DIK_E,			PAD_R2	},

	{	DIK_NUMPAD1,	PAD_L1	},
	{	DIK_NUMPAD7,	PAD_L1	},
	{	DIK_NUMPAD3,	PAD_R1	},
	{	DIK_NUMPAD9,	PAD_R1	},

	{	DIK_TAB,		PAD_STA	},
	{	DIK_RETURN,		PAD_SEL	},
	{	DIK_NUMPADENTER,PAD_SEL	},

	{	DIK_SPACE,		PAD_AL	},
	{	DIK_K,			PAD_AR	},

	{	DIK_NUMPAD5,	PAD_EX1	},	// トグル型主観ボタン

	{	DIK_LSHIFT,		PAD_EX2	},	// ゆっくり押し
	{	DIK_LMENU,		PAD_EX2	},
	{	DIK_NOCONVERT,	PAD_EX2	},

	{	DIK_LCONTROL,	PAD_EX3	},	// 弱押し
	{	DIK_B,			PAD_EX3	},

	{	DIK_I,			PAD_RU	},	// 右スティックエミュレート
	{	DIK_M,			PAD_RD	},
	{	DIK_J,			PAD_RL	},
	{	DIK_L,			PAD_RR	},

} ;
#define	DEFAULT_KEYBOARD_CNV_TBL_NUM	\
				(sizeof(default_KeyboardCnvTbl)/sizeof(default_KeyboardCnvTbl[0]))

/*--------------------------------------------------------------*/
/*	Default PAD Config											*/
/*--------------------------------------------------------------*/
static	WPAD_PAD_BUTTON_CNV	default_PadButtonCnvTbl[WPAD_PAD_BUTTON_CNV_NUM] =
{
	{ PAD_X },		// 00
	{ PAD_A },		// 01
	{ PAD_B },		// 02
	{ PAD_Y },		// 03
	{ PAD_L1 },		// 04
	{ PAD_R1 },		// 05
	{ PAD_X },		// 06
	{ PAD_AL },		// 07
	{ PAD_SEL },	// 08
	{ PAD_STA },	// 09
	{ PAD_EX1 },	// 0A
	{ PAD_AR },		// 0B
	{ PAD_U },		// 0C
	{ PAD_R },		// 0D
	{ PAD_D },		// 0E
	{ PAD_L },		// 0F
} ;

/*--------------------------------------------------------------*/
/*	ローカル関数プロトタイプ宣言								*/
/*--------------------------------------------------------------*/

	/*----------------------------------------------------------*/
	/*	ファイル解析											*/
	/*----------------------------------------------------------*/
static	BOOL	StartPerseFile(char *path, char *filename) ;
static	BOOL	EndPerseFile(void) ;
static	BOOL	StartPerseString(char *str) ;
static	BOOL	EndPerseString(void) ;
static	char	*GetNextStrPerse(char *buff, DWORD buff_size) ;
static	int		GetNextIntPerse(void) ;
static	BOOL	PerseEOF(void) ;

	/*----------------------------------------------------------*/
	/*	Utility													*/
	/*----------------------------------------------------------*/
static	DWORD	XBButtonStr2PADID(char *str) ;
static	void	SkipSpaceChara(char **buff) ;

	/*----------------------------------------------------------*/
	/*	Keyboard												*/
	/*----------------------------------------------------------*/
static	BOOL	InitKeyboardCfg(void) ;
static	BOOL	ReleaseKeyboardCfg(void) ;

	/*----------------------------------------------------------*/
	/*	PAD														*/
	/*----------------------------------------------------------*/
static	BOOL	InitPADCfg(void) ;
static	BOOL	ReleasePADCfg(void) ;

/*--------------------------------------------------------------*/
/*	GV_InitWPadConfig		設定管理初期化						*/
/*--------------------------------------------------------------*/
BOOL	GV_InitWPadConfig(void)
{
	InitKeyboardCfg() ;
	InitPADCfg() ;

	return(TRUE) ;
}

/*--------------------------------------------------------------*/
/*	GV_ReleaseWPadConfig	設定管理終了処理					*/
/*--------------------------------------------------------------*/
BOOL	GV_ReleaseWPadConfig(void)
{
	ReleasePADCfg() ;
	ReleaseKeyboardCfg() ;

	return(TRUE) ;
}

/*--------------------------------------------------------------*/
/*	ファイル解析												*/
/*--------------------------------------------------------------*/
typedef	struct	PerseSys_
{
	char	*alloc_buff ;

	char	*str ;
	char	*str_ptr ;
} PerseSys ;

static	PerseSys	_pers_sys ;

	/*----------------------------------------------------------*/
	/*	StartPerseFile	ファイル解析開始						*/
	/*----------------------------------------------------------*/
static	BOOL	StartPerseFile(char *path, char *filename)
{
	char	*buff ;
	int		size ;
	char	full[512] ;	// 多めに確保

	printf("Perse %s...\n", filename) ;

	/*-- 領域確保 ----------------------------------------------*/

	sprintf(full, "%s/%s", path, filename) ;
	size = FS_LoadRequestDirectPath(full) ;	// 読込要求
	if( size < 0 )
	{
		ASSERT(0) ;
		return(FALSE) ;		// ファイル無し
	}

	if( size == 0 )
	{
		FS_LoadStop() ;
		return(FALSE) ;
	}

	buff = ALLOC(size+1) ;
	if( !buff )
	{
		X2W_ErrorPrintf("Can't Open Configuration file %s\n", full) ;
		return(FALSE) ;
	}
	ZeroMemory(buff, size+1) ;
	/*----------------------------------------------------------*/

	/*-- ファイル読み込み --------------------------------------*/

	FS_LoadSet(buff, size) ;
	while( FS_LoadSync() > 0 ) ;	// 終了待ち

	buff[size] = 0 ;	// 終端(安全策)
	/*----------------------------------------------------------*/

	/*-- 文字列解析開始 ----------------------------------------*/

	_pers_sys.alloc_buff = buff ;
	StartPerseString(buff) ;
	/*----------------------------------------------------------*/

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	EndPerseFile	ファイル解析終了処理					*/
	/*----------------------------------------------------------*/
static	BOOL	EndPerseFile(void)
{
	EndPerseString() ;

	/*-- 領域解放 ----------------------------------------------*/

	if( _pers_sys.alloc_buff )
	{
		FREE(_pers_sys.alloc_buff) ;
		_pers_sys.alloc_buff = NULL ;
	}
	/*----------------------------------------------------------*/

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	StartPerseString	文字列解析開始						*/
	/*----------------------------------------------------------*/
static	BOOL	StartPerseString(char *str)
{
	_pers_sys.str     = str ;
	_pers_sys.str_ptr = str ;

	/*-- 空文字Skip --------------------------------------------*/

	SkipSpaceChara(&_pers_sys.str_ptr) ;
	/*----------------------------------------------------------*/

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	EndPerseString		文字列解析終了処理					*/
	/*----------------------------------------------------------*/
static	BOOL	EndPerseString(void)
{
	_pers_sys.str     = NULL ;
	_pers_sys.str_ptr = NULL ;

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	GetNextStrPerse		次の文字列を取得する				*/
	/*----------------------------------------------------------*/
static	char	*GetNextStrPerse(char *buff, DWORD buff_size)
{
	char	*ptr ;
	DWORD	size ;

	/*-- 空文字Skip --------------------------------------------*/

	SkipSpaceChara(&_pers_sys.str_ptr) ;
	/*----------------------------------------------------------*/

	/*-- 文字列複写 --------------------------------------------*/

	ptr = _pers_sys.str_ptr ;
	while( !isTermChara(*ptr) && !isSpaceChara(*ptr) )
	{
		ptr++ ;
	}


	size = (DWORD)ptr - (DWORD)_pers_sys.str_ptr ;
	if( size > (buff_size - 1) ){ size = (buff_size - 1) ; }

	if( !size )
	{
		buff[0] = 0 ;
	}
	else
	{
		memcpy(buff, _pers_sys.str_ptr, size) ;
		buff[size] = 0 ;
	}

	_pers_sys.str_ptr = ptr ;
	/*----------------------------------------------------------*/

	/*-- 空文字Skip --------------------------------------------*/

	SkipSpaceChara(&_pers_sys.str_ptr) ;
	/*----------------------------------------------------------*/

	return(buff) ;
}

	/*----------------------------------------------------------*/
	/*	GetNextIntPerse		次の数字を取得する					*/
	/*----------------------------------------------------------*/
static	int		GetNextIntPerse(void)
{
	char	str[128] ;
	char	*ptr ;
	int		ret ;
	DWORD	ch ;

	/*-- 次の文字列を取得 --------------------------------------*/

	GetNextStrPerse(str, 128) ;
	if( !*str ){ return(-1) ; }	// 取得失敗
	/*----------------------------------------------------------*/

	/*-- 文字列-->数字 -----------------------------------------*/

	ret = 0 ;
	ptr = str ;
	while( (ch = (DWORD)*ptr) )
	{
		ret <<= 4 ;

		if( (ch >= (DWORD)'0') && (ch <= (DWORD)'9') )
		{
			ret |= (ch - (DWORD)'0') ;
		}
		else if( (ch >= (DWORD)'A') && (ch <= (DWORD)'F') )
		{
			ret |= 10 + (ch - (DWORD)'A') ;
		}
		else
		{
			ret = -1 ;	// 異常
			break ;
		}

		ptr++ ;
	}
	/*----------------------------------------------------------*/

	return(ret) ;
}

	/*----------------------------------------------------------*/
	/*	PerseEOF			EOF判定								*/
	/*----------------------------------------------------------*/
static	BOOL	PerseEOF(void)
{
	return( !(*_pers_sys.str_ptr) ) ;
}

/*--------------------------------------------------------------*/
/*	Utility														*/
/*--------------------------------------------------------------*/
	/*----------------------------------------------------------*/
	/*	文字列変換テーブル										*/
	/*----------------------------------------------------------*/
typedef	struct	XBButtonStr2PADID_Cnv_
{
	char	*str ;
	DWORD	id ;
} XBButtonStr2PADID_Cnv ;

#undef	GV_XBOX_BUTTON_EQU
#define	GV_XBOX_BUTTON_EQU(name_, btnname_)	{ #name_, PAD_ ## btnname_ },
static	XBButtonStr2PADID_Cnv	_xbbutton2padid_cnv_tbl[] =
{
#include "wpad_config_equ.h"
} ;
#undef	GV_XBOX_BUTTON_EQU

#define	XBBUTTON2PADID_CNV_NUM	\
			(sizeof(_xbbutton2padid_cnv_tbl)/sizeof(_xbbutton2padid_cnv_tbl[0]))

typedef	struct	XBButtonStr2PADPRESSID_Cnv_
{
	char	*str ;
	int		id ;
} XBButtonStr2PADPRESSID_Cnv ;

#undef	GV_XBOX_PRESS_EQU
#define	GV_XBOX_PRESS_EQU(name_)	{ #name_, PAD_PRESS_ ## name_ },
static	XBButtonStr2PADPRESSID_Cnv	_xbbutton2padpressid_cnv_tbl[] =
{
#include "wpad_config_equ.h"
} ;
#undef	GV_XBOX_PRESS_EQU

#define	XBBUTTON2PADPRESSID_CNV_NUM	\
			(sizeof(_xbbutton2padpressid_cnv_tbl)/sizeof(_xbbutton2padpressid_cnv_tbl[0]))

	/*----------------------------------------------------------*/
	/*	XBButtonStr2PADID	ボタン文字列をPAD_??に変換			*/
	/*----------------------------------------------------------*/
static	DWORD	XBButtonStr2PADID(char *str)
{
	DWORD					i ;
	XBButtonStr2PADID_Cnv	*cnv ;

	cnv = _xbbutton2padid_cnv_tbl ;
	for(i=XBBUTTON2PADID_CNV_NUM; i>0; i--, cnv++)
	{
		if( !strcmp(cnv->str, str) ){ break ; }
	}
	if( !i ){ return(0) ; }	// 検索ヒット無し

	return(cnv->id) ;
}

	/*----------------------------------------------------------*/
	/*	XBButtonStr2PADPRESSID	ボタン文字列をPAD_??に変換		*/
	/*----------------------------------------------------------*/
static	int	XBButtonStr2PADPRESSID(char *str)
{
	DWORD						i ;
	XBButtonStr2PADPRESSID_Cnv	*cnv ;

	cnv = _xbbutton2padpressid_cnv_tbl ;
	for(i=XBBUTTON2PADPRESSID_CNV_NUM; i>0; i--, cnv++)
	{
		if( !strcmp(cnv->str, str) ){ break ; }
	}
	if( !i ){ return(-1) ; }	// 検索ヒット無し

	return(cnv->id) ;
}

	/*----------------------------------------------------------*/
	/*	SkipSpaceChara		空文字Skip							*/
	/*----------------------------------------------------------*/
static	void	SkipSpaceChara(char **buff)
{
	char	*ptr ;
	char	ch ;

	ptr = *buff ;
	ch = *ptr ;
	while( isSpaceChara(ch) )
	{
		ptr++ ;
		ch = *ptr ;
	}
	*buff = ptr ;
}

/*--------------------------------------------------------------*/
/*	Keyboard													*/
/*--------------------------------------------------------------*/
	/*----------------------------------------------------------*/
	/*	文字列変換テーブル										*/
	/*----------------------------------------------------------*/
typedef	struct	KBStr2DIKID_Cnv_
{
	char	*str ;
	DWORD	id ;
} KBStr2DIKID_Cnv ;

#undef	GV_KBCFG_EQU
#define	GV_KBCFG_EQU(name_)	{ #name_, DIK_ ## name_ },
static	KBStr2DIKID_Cnv	_kbstr2dikid_cnv_tbl[] =
{
#include "wpad_config_equ.h"
} ;
#undef	GV_KBCFG_EQU

#define	KBSTR2DIKID_CNV_NUM	(sizeof(_kbstr2dikid_cnv_tbl)/sizeof(_kbstr2dikid_cnv_tbl[0]))

	/*----------------------------------------------------------*/
	/*	static	func											*/
	/*----------------------------------------------------------*/
static	BOOL	PerseKeyboardCfg(WPAD_KEYBOARD_CFG *cfg, char *file) ;
static	DWORD	KBStr2DIKID(char *str) ;

	/*----------------------------------------------------------*/
	/*	InitKeyboardCfg			Keyboard設定初期化				*/
	/*----------------------------------------------------------*/
static	BOOL	InitKeyboardCfg(void)
{
	ZeroMemory(&GV_KeyboardCfg,  sizeof(GV_KeyboardCfg)) ;
	ZeroMemory(&GV_KeyboardCfgS, sizeof(GV_KeyboardCfgS)) ;

	/*-- 設定解析 ----------------------------------------------*/

	if( !PerseKeyboardCfg(&GV_KeyboardCfg, KBCFG_FILENAME) )
	{
		/*-- 解析失敗時はデフォルト設定使用 --------------------*/

		printf("Perse Failed!!\n") ;
		GV_KeyboardCfg.cnv_num = DEFAULT_KEYBOARD_CNV_TBL_NUM ;
		GV_KeyboardCfg.cnv     = default_KeyboardCnvTbl ;
		/*------------------------------------------------------*/
	}
	/*----------------------------------------------------------*/

	/*-- 設定解析(主観移動) ------------------------------------*/

	if( !PerseKeyboardCfg(&GV_KeyboardCfgS, KBCFGS_FILENAME) )
	{
		/*-- 解析失敗時はデフォルト設定使用 --------------------*/

		printf("Perse Failed!!\n") ;
		GV_KeyboardCfgS.cnv_num = DEFAULT_KEYBOARD_CNV_TBL_NUM ;
		GV_KeyboardCfgS.cnv     = default_KeyboardCnvTbl ;
		/*------------------------------------------------------*/
	}
	/*----------------------------------------------------------*/

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	ReleaseKeyboardCfg		Keyboard設定修了処理			*/
	/*----------------------------------------------------------*/
static	BOOL	ReleaseKeyboardCfg(void)
{
	/*-- 領域の解放 --------------------------------------------*/

	if(  GV_KeyboardCfg.cnv
	  && (GV_KeyboardCfg.cnv != default_KeyboardCnvTbl) )
	{
		FREE(GV_KeyboardCfg.cnv) ;
	}

	if(  GV_KeyboardCfgS.cnv
	  && (GV_KeyboardCfgS.cnv != default_KeyboardCnvTbl) )
	{
		FREE(GV_KeyboardCfgS.cnv) ;
	}
	/*----------------------------------------------------------*/

	ZeroMemory(&GV_KeyboardCfg,  sizeof(GV_KeyboardCfg)) ;
	ZeroMemory(&GV_KeyboardCfgS, sizeof(GV_KeyboardCfgS)) ;

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	PerseKeyboardCfg		Keyboard設定解析				*/
	/*----------------------------------------------------------*/
#define	PERSE_KBCFG_BUFF_NUM_UNIT	(128)
#define	PERSE_KBCFG_STR_SIZE		(128)

static	BOOL	PerseKeyboardCfg(WPAD_KEYBOARD_CFG *kb_cfg, char *file)
{
	DWORD				buff_max;
	WPAD_KEYBOARD_CNV	*buff;
	DWORD				cnv_num;
	WPAD_KEYBOARD_CNV	*cnv;
	char				str[PERSE_KBCFG_STR_SIZE] ;

	kb_cfg->cnv_num = 0 ;
	kb_cfg->cnv     = NULL ;
	/*-- 作業領域確保 ------------------------------------------*/

	buff_max = PERSE_KBCFG_BUFF_NUM_UNIT ;
	buff = (WPAD_KEYBOARD_CNV *)ALLOC(sizeof(WPAD_KEYBOARD_CNV) * buff_max) ;
	if( !buff ){ return(FALSE) ; }
	/*----------------------------------------------------------*/

	/*-- 解析 --------------------------------------------------*/

	if( !StartPerseFile(pcGetIniFilePath(), file) )
	{
		FREE(buff) ;
		return(FALSE) ;	// 解析失敗
	}

	cnv     = buff ;
	cnv_num = 0 ;
	while( !PerseEOF() )
	{
		/*-- キーボードキー文字列取得 --------------------------*/

		GetNextStrPerse(str, PERSE_KBCFG_STR_SIZE) ;
		cnv->key_id = KBStr2DIKID(str) ;
#ifdef DEBUG_MODE
		if( !cnv->key_id ){ printf("[ WARNING ] unknown key %s!!\n", str) ; }
#endif
		/*------------------------------------------------------*/

		/*-- ボタン文字列取得 ----------------------------------*/

		GetNextStrPerse(str, PERSE_KBCFG_STR_SIZE) ;
		cnv->button_id = XBButtonStr2PADID(str) ;
#ifdef DEBUG_MODE
		if( !cnv->button_id ){ printf("[ WARNING ] unknown X-button %s!!\n", str) ; }
#endif
		/*------------------------------------------------------*/

		/*-- 設定が上手く行っていなかったら格納しない ----------*/

		if( !cnv->key_id || !cnv->button_id ){ continue ; }
		/*------------------------------------------------------*/

		/*-- バッファ操作 --------------------------------------*/
		cnv_num++ ;
		if( cnv_num <= buff_max )
		{
			cnv++ ;
		}
		else
		{
			WPAD_KEYBOARD_CNV	*rp_buff ;

			/*-- バッファ再確保 --------------------------------*/

			rp_buff = ALLOC(sizeof(WPAD_KEYBOARD_CNV)
							* (buff_max + PERSE_KBCFG_BUFF_NUM_UNIT)) ;
			if( !rp_buff )
			{
				break ;	// 確保できなかったのでここで解析を強制中断
			}
			else
			{
				memcpy(rp_buff, buff, sizeof(WPAD_KEYBOARD_CNV) * buff_max) ;
				FREE(buff) ;

				buff_max += PERSE_KBCFG_BUFF_NUM_UNIT ;
				buff      = rp_buff ;

				cnv = &buff[cnv_num] ;
			}
			/*--------------------------------------------------*/
		}
		/*------------------------------------------------------*/
	}

	EndPerseFile() ;

	/*----------------------------------------------------------*/

	/*-- 設定 --------------------------------------------------*/

	if( !cnv_num )
	{
		/*-- 設定失敗 ------------------------------------------*/

		FREE(buff) ;
		/*------------------------------------------------------*/
	}
	else
	{
		if( cnv_num == buff_max )
		{
			kb_cfg->cnv_num = cnv_num ;
			kb_cfg->cnv     = buff ;
		}
		else
		{
			WPAD_KEYBOARD_CNV	*rp_buff ;

			/*-- メモリ配置最適化を試みる(逆に悪くなるかも...) -*/

			kb_cfg->cnv_num = cnv_num ;

			rp_buff = ALLOC(sizeof(WPAD_KEYBOARD_CNV) * cnv_num)  ;
			if( !rp_buff )
			{
				kb_cfg->cnv = buff ;
			}
			else
			{
				memcpy(rp_buff, buff, sizeof(WPAD_KEYBOARD_CNV) * cnv_num) ;
				FREE(buff) ;

				kb_cfg->cnv = rp_buff ;
			}
			/*--------------------------------------------------*/
		}
	}
	/*----------------------------------------------------------*/

	/*-- 結果出力(Debug) ---------------------------------------*/
#ifdef DEBUG_MODE
	{
		int	i ;

		cnv = kb_cfg->cnv ;
		for(i=kb_cfg->cnv_num; i>0; i--, cnv++)
		{
			printf("[ Keyboard ]%02X --> %08X\n", cnv->key_id, cnv->button_id ) ;
		}
	}
#endif
	/*----------------------------------------------------------*/

	printf("kb_cfg->cnv    :0x%08X\n", kb_cfg->cnv) ;
	printf("kb_cfg->cnv_num:%d\n", kb_cfg->cnv_num) ;
	return( kb_cfg->cnv != NULL ) ;	// 解析終了
}

	/*----------------------------------------------------------*/
	/*	KBStr2DIKID		Keyboard文字列をDIK_??に変更			*/
	/*----------------------------------------------------------*/
static	DWORD	KBStr2DIKID(char *str)
{
	DWORD			i ;
	KBStr2DIKID_Cnv	*cnv ;

	cnv = _kbstr2dikid_cnv_tbl ;
	for(i=KBSTR2DIKID_CNV_NUM; i>0; i--, cnv++)
	{
		if( !strcmp(cnv->str, str) ){ break ; }
	}
	if( !i ){ return(0) ; }	// 検索ヒット無し

	return(cnv->id) ;
}

/*--------------------------------------------------------------*/
/*	PAD															*/
/*--------------------------------------------------------------*/

typedef	struct	PADAnalogSetting_
{
	char	*name ;
	DWORD	ofs ;		// 設定数値の入っているオフセット(DIJOYSTATE用)
	DWORD	ofs2 ;		// 設定数値の入っているオフセット(DIJOYSTATE2用)
} PADAnalogSetting ;

	/*----------------------------------------------------------*/
	/*	Analog Setting Table									*/
	/*----------------------------------------------------------*/
#undef	GV_PADCFG_ANA_EQU
#define	GV_PADCFG_ANA_EQU(name_, mb_)			\
	{											\
		#name_,									\
		(DWORD)&((DIJOYSTATE *)0)-> ## mb_,		\
		(DWORD)&((DIJOYSTATE2 *)0)-> ## mb_,	\
	},
static	PADAnalogSetting	_pad_ana_setting_tbl[] =
{
#include "wpad_config_equ.h"
} ;
#undef	GV_PADCFG_ANA_EQU

#define	PAD_ANA_SETTIN_TBL_NUM	(sizeof(_pad_ana_setting_tbl)/sizeof(_pad_ana_setting_tbl[0]))
	/*----------------------------------------------------------*/
	/*	static	func											*/
	/*----------------------------------------------------------*/
static	BOOL	PersePADButtonCfg(WPAD_PAD_CFG *cfg, char *file) ;
static	BOOL	PersePADAnalogCfg(WPAD_PAD_CFG *cfg, char *file) ;
static	BOOL	SetDefaultPADAnalogCfg(WPAD_PAD_CFG *cfg) ;

static	PADAnalogSetting	*GetPADAnalogSetting(char *name) ;

	/*----------------------------------------------------------*/
	/*	InitPADCfg		PAD設定初期化							*/
	/*----------------------------------------------------------*/
static	BOOL	InitPADCfg(void)
{
	BOOL	ret ;

	ZeroMemory(&GV_PadCfg,  sizeof(GV_PadCfg)) ;
	ZeroMemory(&GV_PadCfgS, sizeof(GV_PadCfgS)) ;

	/*-- 設定解析 ----------------------------------------------*/

	ret = PersePADButtonCfg(&GV_PadCfg, PADBTNCFG_FILENAME) ;
	if( !ret )
	{
		/*-- 解析失敗時はデフォルト設定使用 --------------------*/

		memcpy(GV_PadCfg.button_cnv, default_PadButtonCnvTbl, sizeof(GV_PadCfg.button_cnv)) ;
		/*------------------------------------------------------*/
	}

	ret = PersePADAnalogCfg(&GV_PadCfg, PADANACFG_FILENAME) ;
	if( !ret ){ SetDefaultPADAnalogCfg(&GV_PadCfg) ; }	// 解析失敗時はデフォルト設定使用
	/*----------------------------------------------------------*/

	/*-- 設定解析(主観視点) ------------------------------------*/

	ret = PersePADButtonCfg(&GV_PadCfgS, PADBTNCFGS_FILENAME) ;
	if( !ret )
	{
		/*-- 解析失敗時はデフォルト設定使用 --------------------*/

		memcpy(GV_PadCfgS.button_cnv, default_PadButtonCnvTbl, sizeof(GV_PadCfgS.button_cnv)) ;
		/*------------------------------------------------------*/
	}

	ret = PersePADAnalogCfg(&GV_PadCfgS, PADANACFGS_FILENAME) ;
	if( !ret ){ SetDefaultPADAnalogCfg(&GV_PadCfgS) ; }	// 解析失敗時はデフォルト設定使用
	/*----------------------------------------------------------*/

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	ReleasePADCfg	PAD設定終了処理							*/
	/*----------------------------------------------------------*/
static	BOOL	ReleasePADCfg(void)
{
	ZeroMemory(&GV_PadCfg,  sizeof(GV_PadCfg)) ;
	ZeroMemory(&GV_PadCfgS, sizeof(GV_PadCfgS)) ;

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	PersePADButtonCfg	PAD Button設定解析					*/
	/*----------------------------------------------------------*/
#define	PERSE_PADBTNCFG_STR_SIZE		(128)

static	BOOL	PersePADButtonCfg(WPAD_PAD_CFG *cfg, char *file)
{
	char				str[PERSE_PADBTNCFG_STR_SIZE] ;
	int					set_id ;
	DWORD				button_id ;

	/*-- 解析 --------------------------------------------------*/

	if( !StartPerseFile(pcGetIniFilePath(), file) )
	{
		return(FALSE) ;	// 解析失敗
	}

	while( !PerseEOF() )
	{
		/*-- ボタン番号取得 ------------------------------------*/

		set_id = GetNextIntPerse() ;
		/*------------------------------------------------------*/

		/*-- ボタン文字列取得 ----------------------------------*/

		GetNextStrPerse(str, PERSE_PADBTNCFG_STR_SIZE) ;
		button_id = XBButtonStr2PADID(str) ;
#ifdef DEBUG_MODE
		if( !button_id ){ printf("[ WARNING ] PADBTN:%02X unknown X-button %s!!\n", set_id, str) ; }
#endif
		/*------------------------------------------------------*/

		/*-- 設定 ----------------------------------------------*/

		if( (set_id >= 0) && (set_id < WPAD_PAD_BUTTON_CNV_NUM) )
		{
			cfg->button_cnv[set_id].button_id |= button_id ;
		}
		/*------------------------------------------------------*/
	}

	EndPerseFile() ;
	/*----------------------------------------------------------*/

	/*-- 結果出力(Debug) ---------------------------------------*/
#ifdef DEBUG_MODE
	{
		int	i ;

		for(i=0; i<WPAD_PAD_BUTTON_CNV_NUM; i++)
		{
			if( cfg->button_cnv[i].button_id )
			printf("[ PAD ]%02X --> %08X\n", i,
					cfg->button_cnv[i].button_id) ;
		}
	}
#endif
	/*----------------------------------------------------------*/
	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	PersePADAnalogCfg	PAD Analog設定解析					*/
	/*----------------------------------------------------------*/
#define	PERSE_PADANACFG_STR_SIZE		(128)

static	BOOL	PersePADAnalogCfg(WPAD_PAD_CFG *cfg, char *file)
{
	int					i ;
	WPAD_PAD_ANALOG_CNV	*cnv ;
	char				str[PERSE_PADANACFG_STR_SIZE] ;
	int					set_id ;
	PADAnalogSetting	*setting ;
	BOOL				rev ;

	/*-- 状態初期化 --------------------------------------------*/

	cnv = cfg->analog_cnv ;
	for(i=WPAD_PAD_ANALOG_CNV_NUM; i>0; i--, cnv++)
	{
		cnv->ofs  = WPAD_PAD_ANALOG_CNV_OFS_UNUSE ;
		cnv->ofs2 = WPAD_PAD_ANALOG_CNV_OFS_UNUSE ;
	}
	/*----------------------------------------------------------*/

	/*-- 解析 --------------------------------------------------*/

	if( !StartPerseFile(pcGetIniFilePath(), file) )
	{
		return(FALSE) ;	// 解析失敗
	}

	while( !PerseEOF() )
	{
		/*-- アナログ番号取得 ----------------------------------*/

		set_id = GetNextIntPerse() ;
		/*------------------------------------------------------*/

		/*-- 設定取得 ------------------------------------------*/

		GetNextStrPerse(str, PERSE_PADBTNCFG_STR_SIZE) ;
		setting = GetPADAnalogSetting(str) ;
		/*------------------------------------------------------*/

		/*-- 入力反転設定取得 ----------------------------------*/
		GetNextStrPerse(str, PERSE_PADBTNCFG_STR_SIZE) ;

		switch( *str )
		{
		  case 'R' :
			rev = TRUE ;	// 反転設定
			break ;

		   default :
			rev = FALSE ;	// 通常設定
			break ;
		}
		/*------------------------------------------------------*/

		/*-- 設定 ----------------------------------------------*/

		if(  (set_id >= 0) && (set_id < WPAD_PAD_ANALOG_CNV_NUM)
		  && setting )
		{
			cnv = &cfg->analog_cnv[set_id] ;

			cnv->ofs     = setting->ofs ;
			cnv->ofs2    = setting->ofs2 ;
			cnv->reverse = rev ;
		}
		/*------------------------------------------------------*/
	}

	EndPerseFile() ;
	/*----------------------------------------------------------*/

	/*-- 結果出力(Debug) ---------------------------------------*/
#ifdef DEBUG_MODE

	cnv = cfg->analog_cnv ;
	for(i=WPAD_PAD_ANALOG_CNV_NUM; i>0; i--, cnv++)
	{
		printf("[ PAD ANALOG ] %2d:(%04X, %04X)",
				i, cnv->ofs, cnv->ofs2) ;
		if( cnv->reverse ){ printf("...Reverse") ; }
		printf("\n") ;
	}
#endif
	/*----------------------------------------------------------*/

	return(TRUE) ;
}

	/*----------------------------------------------------------*/
	/*	GetPADAnalogSetting		PAD Analog設定取得				*/
	/*----------------------------------------------------------*/
static	PADAnalogSetting	*GetPADAnalogSetting(char *name)
{
	int					i ;
	PADAnalogSetting	*setting ;

	setting = _pad_ana_setting_tbl ;
	for(i=PAD_ANA_SETTIN_TBL_NUM; i>0; i--, setting++)
	{
		if( !strcmp(name, setting->name) ){ break ; }	// 検索ヒット
	}
	if( !i ){ return(NULL) ; }	// 検索失敗

	return(setting) ;
}

	/*----------------------------------------------------------*/
	/*	SetDefaultPADAnalogCfg		PAD Analog Default設定		*/
	/*----------------------------------------------------------*/
static	BOOL	SetDefaultPADAnalogCfg(WPAD_PAD_CFG *cfg)
{
	int					i ;
	WPAD_PAD_ANALOG_CNV	*cnv ;

	/*-- 状態初期化 --------------------------------------------*/

	cnv = cfg->analog_cnv ;
	for(i=WPAD_PAD_ANALOG_CNV_NUM; i>0; i--, cnv++)
	{
		cnv->ofs  = WPAD_PAD_ANALOG_CNV_OFS_UNUSE ;
		cnv->ofs2 = WPAD_PAD_ANALOG_CNV_OFS_UNUSE ;

		cnv->reverse = FALSE ;
	}
	/*----------------------------------------------------------*/

	return(TRUE) ;
}

/*-- End Of File --*/
