/*
   パッド入出力モジュール
   
   1999/03/25 K.Uehara
   $Id: wpad.cpp,v 1.17 2003/01/07 05:04:56 takaki Exp $
*/

#include <stdio.h>

#ifdef _XBOX
#ifdef _WINDOWS

//#define	INITGUID
#include <xtl.h>
#define DIRECTINPUT_VERSION         0x0800
#include <dinput.h>
#else

#include <xtl.h>

#endif
#else

#define DIRECTINPUT_VERSION         0x0800
#include <dinput.h>

#endif

#include "libgv.h"
#include "util.h"
#include "wpad_config.h"
#include "x2w_app_config.h"


#ifdef __cplusplus
extern "C" {
#endif

const int PAD_NUM = 4;			// パッド処理数
const int MU_NUM = 8;			// メモリユニット処理数

static	BOOL	InputModeIsSubject = FALSE ;
static	WPAD_KEYBOARD_CFG	*CurKeyboardCfg = &GV_KeyboardCfg ;
static	WPAD_PAD_CFG		*CurGV_PadCfg   = &GV_PadCfg ;
static	BOOL				CurMouseEnable = FALSE ;


/* 1999/12/15 lib ver1.2の導入でlibpadを使用するように変更 */

#define HANGUP_KEY	( PAD_A  | PAD_L2 | PAD_R1 | PAD_R2 | PAD_SEL | PAD_STA )

/* ---------------------------------------------------------------------- */
/*
   入力デバイス設定値
*/

#define	__KEYBOARD_SUBJECTMOBE_USE_ONLY_DIRBTN__	(TRUE)
#define	__KEYBOARD_USE_BUFFERING_DATA__				(TRUE)

#define	KEYBOARD_POW_PRESS			(0x30)	// 弱ボタン押し時の入力値(Old: 0xA0)
#define	KEYBOARD_SLOW_PRESS_NFRIC	(1.0f)	// ゆっくりボタン解放時の入力値変更率(Old: 0.4f)
#define	KEYBOARD_SLOW_PRESS_SFRIC	(0.1f)	// ゆっくりボタン使用時の入力値変更率(Old: 0.05f)
#define	KEYBOARD_BTNEMU_TH			(0x10)	// 仮想感圧をボタン入力と見なす閾値(Old: 0x40)

#define	JOYSTICK_POW_PRESS			(0x30)	// 弱ボタン押し時の入力値(Old: 0xA0)
#define	JOYSTICK_SLOW_PRESS_NFRIC	(1.0f)	// ゆっくりボタン解放時の入力値変更率(Old: 0.4f)
#define	JOYSTICK_SLOW_PRESS_SFRIC	(0.1f)	// ゆっくりボタン使用時の入力値変更率(Old: 0.05f)
#define	JOYSTICK_BTNEMU_TH			(0x10)	// 仮想感圧をボタン入力と見なす閾値(Old: 0x40)

#define	MOUSE_POW_PRESS			(0x30)	// 弱ボタン押し時の入力値(Old: 0xA0)
#define	MOUSE_SLOW_PRESS_NFRIC	(1.0f)	// ゆっくりボタン解放時の入力値変更率(Old: 0.4f)
#define	MOUSE_SLOW_PRESS_SFRIC	(0.1f)	// ゆっくりボタン使用時の入力値変更率(Old: 0.05f)
#define	MOUSE_MOVE_ABS_COEF		(8.0f)	// 絶対値移動入力時係数
#define	MOUSE_MOVE_OFS_COEF		(64.0f)	// 絶対値移動入力時係数


/* ---------------------------------------------------------------------- */
/*
   USB keyboard関連(デバッグ用)
*/

#ifdef USBKBD
#if 0

GV_KEY GV_KeyData;

static inline int is_exist( unsigned char *key, int code )
{
	int i;
	for( i = 0; i < 3; i++ ){
		if( key[ i ] == code ){
			return 1;
		}
	}
	return 0;
}

static void update_key( void )
{
	unsigned char *p;
	int i;
	USBKBD_DATA new;
	usbkbd_read( &new );

	// PRESS check
		*( int * )GV_KeyData.press = 0;
	p = GV_KeyData.press;
	for( i = 0; i < 3; i++ ){
		int c;
		if( ( c = new.key[ i ] ) == 0 ) break;
		if( ! is_exist( GV_KeyData.status, c ) ){
			*( p++ ) = c;
		}
	}

	// release check
		*( int * )GV_KeyData.release = 0;
	p = GV_KeyData.release;
	for( i = 0; i < 3; i++ ){
		int c;
		if( ( c = GV_KeyData.status[ i ] ) == 0 ) break;
		if( ! is_exist( new.key, c ) ){
			*( p++ ) = c;
		}
	}

	*( USBKBD_DATA * )&GV_KeyData = new;
}
#endif
#endif

/* ------------------------------------------- */
/*
   libpad system driver
*/

// SCE のパッドバッファを読むための構造体

#ifndef _WINDOWS
#define PRESSURE_SIZE 12
#else
#define PRESSURE_SIZE	(MAX_PAD_PRESS)
#endif

#ifndef _WINDOWS
typedef struct {
	unsigned char flag;
	unsigned char id;
	unsigned char button[2];
	struct {
		unsigned char data[ 4 ];
	} analog;
	struct {
		unsigned char data[ PRESSURE_SIZE ];
	} pressure;
} PADBUF_DATA;
#else

typedef struct {
	unsigned char flag;
	unsigned char id;
	unsigned char button[2];
	struct {
		unsigned char data[ 4 ];
	} analog;
	struct {
		unsigned char data[ PRESSURE_SIZE ];
	} pressure;
} DEMO_PADBUF_DATA;

typedef struct {
	unsigned char	flag;
	unsigned char	id;
	DWORD			button;
	struct {
		unsigned char data[ 4 ];
	} analog;
	struct {
		unsigned char data[ PRESSURE_SIZE ];
	} pressure;
} PADBUF_DATA;
#endif

#ifndef _WINDOWS	// (#ifdef _XBOX)

// Xbox 用変数
typedef struct {
	HANDLE	hDevice;	// ハンドル
	int pad_status;		// 状態
	XINPUT_CAPABILITIES	caps;	// パッドの能力変数
	XINPUT_FEEDBACK	feedback;	// 振動(通信用)
	int		s32LMotor;			// 振動
	int		s32RMotor;			// 振動
	PADBUF_DATA	pad_data;
} XPAD;

#else

#if FALSE
typedef struct {
	LPDIRECTINPUTDEVICE8	pJoystick;	// デバイスポインタ
	int pad_status;				// 状態
	DIDEVCAPS	caps;			// パッドの能力変数
	int		s32LMotor;			// 振動
	int		s32RMotor;			// 振動
	PADBUF_DATA	pad_data;
} XPAD;
#endif

typedef struct	WPAD_DEVICE_
{
	LPDIRECTINPUTDEVICE8	pDev ;	// デバイスポインタ
	DIDEVCAPS				caps ;	// パッドの能力変数
	int pad_status ;				// 状態

	BOOL					use ;	// 実際に使用されているか？フラグ

	LPDIRECTINPUTEFFECT		pEffect ;	// エフェクトデバイスポインタ
	DIPERIODIC				periodic ;	// 振動制御パラメータ
	BOOL					vib_start	;
	PADBUF_DATA				pad_data;

#if	__KEYBOARD_USE_BUFFERING_DATA__
	BYTE					keystate[256] ;
#endif

} WPAD_DEVICE;

#define	MAX_WPAD_DEVICE_NUM	(64)

typedef struct {
	int		dev_num ;					// 全Device数
	int		dev_gamectrl_num ;			// Game Controller Device数
	int		dev_keyboard_num ;			// Key Board Device数
	int		dev_pointer_num ;			// Pointer Device数
	WPAD_DEVICE	dev[MAX_WPAD_DEVICE_NUM] ;	// Device

	int pad_status;				// 状態
	int		s32LMotor;			// 振動
	int		s32RMotor;			// 振動
	PADBUF_DATA	pad_data;
} WPAD;


static LPDIRECTINPUT8	pDI	= NULL;
extern HWND DG_hWnd;

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
											VOID* pContext );
static BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
									   VOID* pContext );
static BOOL CALLBACK DIEnumEffectsCallback( LPCDIEFFECTINFO pdei, LPVOID pvRef) ;

static void	PadBuffInit(PADBUF_DATA *pad_data) ;

#endif


static void	PadBuffInit(PADBUF_DATA *pad_data)
{
	ZeroMemory(pad_data, sizeof(PADBUF_DATA)) ;

	pad_data->analog.data[0] = 0x80 ;
	pad_data->analog.data[1] = 0x80 ;
	pad_data->analog.data[2] = 0x80 ;
	pad_data->analog.data[3] = 0x80 ;
	pad_data->button = 0xffffffff;
}

static	WPAD	wpad ;

#ifndef _WINDOWS
static XPAD xpad[ PAD_NUM ];
#endif
static int	portmap[ PAD_NUM ];	// ポートマッピング
// ポート番号iのパッド情報はゲーム側から portmap[i] のポートに見える
static int	invportmap( int port )
{	// portmap の逆関数
	int i;
	for( i = 0 ; i < PAD_NUM; i++ )	{
		if( portmap[ i ] == port ) return i;
	}
	return 0;
}

// パッドデモ用
static int 			demo_pad_status ;
static PADBUF_DATA	demo_pad_data ;

// パッドマスク等指定用
int		GV_PadMask[ PAD_NUM * 2 ] ; 	/* マスク指定用 */
int		GV_PadPress[ PAD_NUM ] ;	/* シナリオパッド押し用 */


enum {
	PAD_STATUS_NO_CONNECT,
	PAD_STATUS_CONNECT_OTHER,		// OTHER PAD ( not support )
	PAD_STATUS_CONNECT_PAD,			// NORMAL PAD
	PAD_STATUS_CONNECT_AC,			// ANALOG CONTROLER
	PAD_STATUS_CONNECT_DS,			// DUAL SHOCK
	PAD_STATUS_CONNECT_DS2,			// DUAL SHOCK 2
};

// Xbox pad->Ps2 pad Emulation table
typedef struct {
	DWORD	u32ButtonFlag;	// PS2のボタンフラグ
	short	s16PressOrder;	// PS2の感圧番号
} EMTABLE;

#ifndef _WINDOWS	// (#ifdef _XBOX)
static EMTABLE tabButton[] = {
	{PAD_U,    2},	// Up
	{PAD_D,    3},	// Down
	{PAD_L,    1},	// Left
	{PAD_R,    0},	// Right
	{PAD_STA, -1},	// Start
	{PAD_SEL, -1},	// Select
	{PAD_AL,  -1},	// Analog L
	{PAD_AR,  -1},	// Analog R
};

static EMTABLE tabPress[] = {
	{PAD_B,   6},	// A(×)
	{PAD_A,   5},	// B(○)
	{PAD_Y,   7},	// X(□)
	{PAD_X,   4},	// Y(△)
	{PAD_R2, 11},	// Black(R2)
	{PAD_L2, 10},	// White(L2)
	{PAD_L1,  8},	// L1
	{PAD_R1,  9},	// R1
};
#else
static EMTABLE tabButton[] = {
	{PAD_Y,		PAD_PRESS_Y	},
	{PAD_X,		PAD_PRESS_X	},
	{PAD_AL,	-1			},
	{PAD_B,		PAD_PRESS_B	},
	{PAD_A,		PAD_PRESS_A	},
	{PAD_AR,	-1			},
	{PAD_L1,	PAD_PRESS_L1},
	{PAD_R1,	PAD_PRESS_R1},
	{PAD_L2,	PAD_PRESS_L2},
	{PAD_R2,	PAD_PRESS_R2},
	{PAD_STA,	-1			},
	{PAD_SEL,	-1			},
};
#endif
static DWORD tabDir[] = {
	PAD_U, PAD_U|PAD_R, PAD_R, PAD_R|PAD_D,
	PAD_D, PAD_D|PAD_L, PAD_L, PAD_L|PAD_U,
};

static	DWORD	wpad_press_to_button_tbl[PRESSURE_SIZE] =
{
	PAD_R,	// PAD_PRESS_R		0
	PAD_L,	// PAD_PRESS_L		1
	PAD_U,	// PAD_PRESS_U		2
	PAD_D,	// PAD_PRESS_D		3
	PAD_X,	// PAD_PRESS_X		4
	PAD_A,	// PAD_PRESS_A		5
	PAD_B,	// PAD_PRESS_B		6
	PAD_Y,	// PAD_PRESS_Y		7
	PAD_L1,	// PAD_PRESS_L1		8
	PAD_R1,	// PAD_PRESS_R1		9
	PAD_L2,	// PAD_PRESS_L2		10
	PAD_R2,	// PAD_PRESS_R2		11
} ;


#define IS_CONNECT( _state ) ( (_state) >= PAD_STATUS_CONNECT_OTHER )

#define IS_AVAILABLE(_state) ( (_state) > PAD_STATUS_CONNECT_OTHER )
#define PAD_TYPE( _state )	 ( (_state) - PAD_STATUS_CONNECT_PAD )

#define PAD_DATA_NORMAL			0x00	// データ取得可能
#define PAD_DATA_ERROR			0x01	// エラーなので,前回のデータをそのままつかう
#define PAD_DATA_RELEASE		0x02	// 通信中または抜けている
#define PAD_DATA_NOT_SUPPORT	0x03	// サポートされていないデバイス

#define PARAM_MAX		6

static void	pad_read_act_joystick(WPAD_DEVICE *dev)
{
	int				i ;
	HRESULT    		hr ;
	DIJOYSTATE		js ;
	DWORD			u32State ;
	float			press_tgt[PRESSURE_SIZE] ;
	float			coef ;
	float			fric ;
	BOOL			slow_press ;
	float			pow_press ;
	PADBUF_DATA		pad_data;

	/*-- 入力取得 ------------------------------------------------------*/

	if( FAILED(hr = dev->pDev->GetDeviceState(sizeof(DIJOYSTATE), &js)) )
	{
		/*-- 入力取得失敗 --*/
		PadBuffInit(&dev->pad_data) ;
		return ;
	}
	/*------------------------------------------------------------------*/

	pad_data = dev->pad_data ;
	u32State = 0 ;

	/*-- アナログスティック --------------------------------------------*/

#if FALSE
	pad_data.analog.data[0] = js.lZ  + 128;
	pad_data.analog.data[1] = js.lRz + 128;
	pad_data.analog.data[2] = js.lX  + 128;
	pad_data.analog.data[3] = js.lY  + 128;
#else
	{
		WPAD_PAD_ANALOG_CNV	*cnv ;
		unsigned char		*data ;
		LONG				val ;

		cnv  = CurGV_PadCfg->analog_cnv ;
		data = pad_data.analog.data ;
		for(i=WPAD_PAD_ANALOG_CNV_NUM; i>0; i--, cnv++, data++)
		{
			if( cnv->ofs == WPAD_PAD_ANALOG_CNV_OFS_UNUSE )
			{
				*data = 128 ;
				continue ;
			}

			val = *(LONG *)((BYTE *)&js + cnv->ofs) + 128 ;
			if( cnv->reverse ){ val = 0xff - val ; }

			*data = (unsigned char)val ;
		}
	}
#endif
	/*------------------------------------------------------------------*/

	/*-- 入力変換 ------------------------------------------------------*/

	{
		WPAD_PAD_BUTTON_CNV	*cnv ;
		BYTE				*rgbButton ;

		cnv       = CurGV_PadCfg->button_cnv ;
		rgbButton = js.rgbButtons ;
		for(i=WPAD_PAD_BUTTON_CNV_NUM; i>0; i--, cnv++, rgbButton++)
		{
			if( !cnv->button_id ){ continue ; }
			if( *rgbButton ){ u32State |= cnv->button_id ; }
		}
	}
	/*------------------------------------------------------------------*/

	/*-- 特殊ボタン判定 ------------------------------------------------*/

	if( u32State & PAD_EX2 ){ slow_press = TRUE ; }
	else{ slow_press = FALSE ; }

	if( u32State & PAD_EX3 ){ pow_press = (float)JOYSTICK_POW_PRESS ; }
	else{ pow_press = (float)0xff ; }
	/*------------------------------------------------------------------*/

	/*-- 方向入力ボタン変換 --------------------------------------------*/

	if(js.rgdwPOV[0] >= 0){
		int dir = js.rgdwPOV[0] / 4500;
		if(dir < sizeof(tabDir) / sizeof(tabDir[0]))
			u32State |= tabDir[dir];
	}
	/*------------------------------------------------------------------*/

	/*-- 感圧エミュレーション ------------------------------------------*/

	ZeroMemory(press_tgt, sizeof(press_tgt)) ;
	{
		DWORD	*p2b ;
		float	*tgt_ptr ;

		p2b     = wpad_press_to_button_tbl ;
		tgt_ptr = press_tgt ;
		for(i=PRESSURE_SIZE; i>0; i--, p2b++, tgt_ptr++)
		{
			if( u32State & *p2b ){ *tgt_ptr = pow_press ; }
		}
	}

	if( slow_press ){ fric = JOYSTICK_SLOW_PRESS_SFRIC ; }	// ゆっくり
	else{ fric = JOYSTICK_SLOW_PRESS_NFRIC ; }				// 早め

	for(i=0; i<PRESSURE_SIZE; i++)
	{
		coef = (float)pad_data.pressure.data[i] ;
		coef += (press_tgt[i] - coef) * fric ;
		pad_data.pressure.data[i]  = (unsigned char)coef ;
	}
	/*------------------------------------------------------------------*/

	/*-- スティックエミュレーション ------------------------------------*/

	{
		float	an_tgt ;

		/*-- 右スティック --------------------------------------------------*/

		if( u32State & (PAD_RR|PAD_RL) )
		{
			an_tgt = 128.0f ;
			if( u32State & PAD_RR ){ an_tgt += 127.0f ; }
			if( u32State & PAD_RL ){ an_tgt -= 128.0f ; }

			coef = (float)pad_data.analog.data[0] ;
			coef += (an_tgt - coef) * fric ;
			pad_data.analog.data[0] = (unsigned char)coef ;
		}

		if( u32State & (PAD_RD|PAD_RU) )
		{
			an_tgt = 128.0f ;
			if( u32State & PAD_RD ){ an_tgt += 127.0f ; }
			if( u32State & PAD_RU ){ an_tgt -= 128.0f ; }

			coef = (float)pad_data.analog.data[1] ;
			coef += (an_tgt - coef) * fric ;
			pad_data.analog.data[1] = (unsigned char)coef ;
		}
		/*--------------------------------------------------------------*/
	}
	/*------------------------------------------------------------------*/

	/*-- ボタンエミュレーション ----------------------------------------*/

	u32State &= ~(PAD_U|PAD_D|PAD_L|PAD_R|PAD_X|PAD_A|PAD_B|PAD_Y
				|PAD_L1|PAD_R1|PAD_L2|PAD_R2) ;

	if( pad_data.pressure.data[PAD_PRESS_U] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_U ; }
	if( pad_data.pressure.data[PAD_PRESS_D] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_D ; }
	if( pad_data.pressure.data[PAD_PRESS_L] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_L ; }
	if( pad_data.pressure.data[PAD_PRESS_R] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_R ; }

	if( pad_data.pressure.data[PAD_PRESS_X] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_X ; }
	if( pad_data.pressure.data[PAD_PRESS_A] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_A ; }
	if( pad_data.pressure.data[PAD_PRESS_B] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_B ; }
	if( pad_data.pressure.data[PAD_PRESS_Y] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_Y ; }

	if( pad_data.pressure.data[PAD_PRESS_L1] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_L1 ; }
	if( pad_data.pressure.data[PAD_PRESS_R1] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_R1 ; }
	if( pad_data.pressure.data[PAD_PRESS_L2] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_L2 ; }
	if( pad_data.pressure.data[PAD_PRESS_R2] > JOYSTICK_BTNEMU_TH ){ u32State |= PAD_R2 ; }
	/*------------------------------------------------------------------*/

	pad_data.button = ~u32State & 0xffffffff;

	// その他雑多な情報を設定
	pad_data.flag = 0;		// 通信成功
	pad_data.id = 0x79;		// DS2 を示す

	dev->pad_data = pad_data ;
}

static void	pad_read_act_keyboard(WPAD_DEVICE *dev)
{
	int				i ;
	HRESULT    		hr ;
	DWORD			u32State ;
#if	!__KEYBOARD_USE_BUFFERING_DATA__
	BYTE			keystate[256];
#else
	BYTE			*keystate;
#endif
	float			press_tgt[PRESSURE_SIZE] ;
	float			coef ;
	float			fric ;
	BOOL			slow_press ;
	float			pow_press ;
	PADBUF_DATA		pad_data;

	/*-- 入力取得 ------------------------------------------------------*/

#if	!__KEYBOARD_USE_BUFFERING_DATA__
	hr = dev->pDev->GetDeviceState(sizeof(keystate), keystate) ;
	if( FAILED(hr) )
	{
		/*-- 入力取得失敗 --*/
		PadBuffInit(&dev->pad_data) ;
		return ;
	}
#else
	keystate = dev->keystate ;
	{
		DIDEVICEOBJECTDATA	data[8] ;
		DIDEVICEOBJECTDATA	*data_ptr ;
		DWORD				item_num ;
		
		while( TRUE )
		{
			/*-- Bufferingされているデータの取得 -----------------------*/

			item_num = sizeof(data)/sizeof(data[0]) ;
			hr = dev->pDev->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
										data,
										&item_num,
										0) ;
			if( FAILED(hr) )
			{
				/*-- 入力取得失敗 --*/
				PadBuffInit(&dev->pad_data) ;
				ZeroMemory(dev->keystate, sizeof(dev->keystate)) ;
				return ;
			}

			if( !item_num ){ break ; }		// 入力が無ければ失敗
			/*----------------------------------------------------------*/

			/*-- データ反映 --------------------------------------------*/

			data_ptr = data ;
			for(i=item_num; i>0; i--, data_ptr++)
			{
				keystate[data_ptr->dwOfs] ^= 0x80 ;
			}
			/*----------------------------------------------------------*/
		}
	}
#endif
	/*------------------------------------------------------------------*/

	pad_data = dev->pad_data ;

	/*-- 入力変換 ------------------------------------------------------*/

	{
		WPAD_KEYBOARD_CNV	*cnv ;

		u32State = 0 ;
		cnv = CurKeyboardCfg->cnv ;
		for(i=CurKeyboardCfg->cnv_num; i>0; i--, cnv++)
		{
			if( keystate[cnv->key_id] ){ u32State |= cnv->button_id ; }
		}
	}
	/*------------------------------------------------------------------*/

	/*-- 特殊ボタン判定 ------------------------------------------------*/

	if( u32State & PAD_EX2 ){ slow_press = TRUE ; }
	else{ slow_press = FALSE ; }

	if( u32State & PAD_EX3 ){ pow_press = (float)KEYBOARD_POW_PRESS ; }
	else{ pow_press = (float)0xff ; }
	/*------------------------------------------------------------------*/

	/*-- 感圧エミュレーション ------------------------------------------*/

	ZeroMemory(press_tgt, sizeof(press_tgt)) ;
	{
		DWORD	*p2b ;
		float	*tgt_ptr ;

		p2b     = wpad_press_to_button_tbl ;
		tgt_ptr = press_tgt ;
		for(i=PRESSURE_SIZE; i>0; i--, p2b++, tgt_ptr++)
		{
			if( u32State & *p2b ){ *tgt_ptr = pow_press ; }
		}
	}

	if( slow_press ){ fric = KEYBOARD_SLOW_PRESS_SFRIC ; }	// ゆっくり
	else{ fric = KEYBOARD_SLOW_PRESS_NFRIC ; }				// 通常(早め)

	for(i=0; i<PRESSURE_SIZE; i++)
	{
		coef = (float)pad_data.pressure.data[i] ;
		coef += (press_tgt[i] - coef) * fric ;
		pad_data.pressure.data[i]  = (unsigned char)coef ;
	}
	/*------------------------------------------------------------------*/

	/*-- スティックアナログ情報 ----------------------------------------*/

	/* 左スティック */
#if __KEYBOARD_SUBJECTMOBE_USE_ONLY_DIRBTN__
	if( InputModeIsSubject )
	{
		pad_data.analog.data[2] = 128 ;
		pad_data.analog.data[3] = 128 ;
	}
	else
	{
		pad_data.analog.data[2] = (((int)pad_data.pressure.data[PAD_PRESS_R]
								- (int)pad_data.pressure.data[PAD_PRESS_L]) >> 1) + 128 ;
		pad_data.analog.data[3] = (((int)pad_data.pressure.data[PAD_PRESS_U]
								- (int)pad_data.pressure.data[PAD_PRESS_D]) >> 1) + 128 ;
	}
#else
	pad_data.analog.data[2] = (((int)pad_data.pressure.data[PAD_PRESS_R]
							- (int)pad_data.pressure.data[PAD_PRESS_L]) >> 1) + 128 ;
	pad_data.analog.data[3] = (((int)pad_data.pressure.data[PAD_PRESS_U]
							- (int)pad_data.pressure.data[PAD_PRESS_D]) >> 1) + 128 ;
#endif
	{
		float	an_tgt ;

		/*-- 右スティック --------------------------------------------------*/

		an_tgt = 128.0f ;
		if( u32State & PAD_RR ){ an_tgt += 127.0f ; }
		if( u32State & PAD_RL ){ an_tgt -= 128.0f ; }

		coef = (float)pad_data.analog.data[0] ;
		coef += (an_tgt - coef) * fric ;
		pad_data.analog.data[0] = (unsigned char)coef ;

		an_tgt = 128.0f ;
		if( u32State & PAD_RD ){ an_tgt += 127.0f ; }
		if( u32State & PAD_RU ){ an_tgt -= 128.0f ; }

		coef = (float)pad_data.analog.data[1] ;
		coef += (an_tgt - coef) * fric ;
		pad_data.analog.data[1] = (unsigned char)coef ;

		/*--------------------------------------------------------------*/
	}
	/*------------------------------------------------------------------*/

	/*-- ボタンエミュレーション ----------------------------------------*/

	/* 通常移動時 */
	u32State &= ~(PAD_U|PAD_D|PAD_L|PAD_R|PAD_X|PAD_A|PAD_B|PAD_Y
				|PAD_L1|PAD_R1|PAD_L2|PAD_R2) ;

	if( pad_data.pressure.data[PAD_PRESS_U] > KEYBOARD_BTNEMU_TH ){ u32State |= PAD_U ; }
	if( pad_data.pressure.data[PAD_PRESS_D] > KEYBOARD_BTNEMU_TH ){ u32State |= PAD_D ; }
	if( pad_data.pressure.data[PAD_PRESS_L] > KEYBOARD_BTNEMU_TH ){ u32State |= PAD_L ; }
	if( pad_data.pressure.data[PAD_PRESS_R] > KEYBOARD_BTNEMU_TH ){ u32State |= PAD_R ; }

	if( pad_data.pressure.data[PAD_PRESS_X] > KEYBOARD_BTNEMU_TH ){ u32State |= PAD_X ; }
	if( pad_data.pressure.data[PAD_PRESS_A] > KEYBOARD_BTNEMU_TH ){ u32State |= PAD_A ; }
	if( pad_data.pressure.data[PAD_PRESS_B] > KEYBOARD_BTNEMU_TH ){ u32State |= PAD_B ; }
	if( pad_data.pressure.data[PAD_PRESS_Y] > KEYBOARD_BTNEMU_TH ){ u32State |= PAD_Y ; }	

	if( pad_data.pressure.data[PAD_PRESS_L1]>KEYBOARD_BTNEMU_TH ){ u32State |= PAD_L1 ; }
	if( pad_data.pressure.data[PAD_PRESS_R1]>KEYBOARD_BTNEMU_TH ){ u32State |= PAD_R1 ; }
	if( pad_data.pressure.data[PAD_PRESS_L2]>KEYBOARD_BTNEMU_TH ){ u32State |= PAD_L2 ; }
	if( pad_data.pressure.data[PAD_PRESS_R2]>KEYBOARD_BTNEMU_TH ){ u32State |= PAD_R2 ; }

	/*------------------------------------------------------------------*/

	pad_data.button = ~u32State & 0xffffffff;

	// その他雑多な情報を設定
	pad_data.flag = 0;		// 通信成功
	pad_data.id = 0x79;		// DS2 を示す

	dev->pad_data = pad_data ;
}

static void	pad_read_act_mouse(WPAD_DEVICE *dev)
{
	int				i ;
	DIMOUSESTATE	ms ;
	HRESULT    		hr ;
	DWORD			u32State ;
	PADBUF_DATA		pad_data;
	float			press_tgt[PRESSURE_SIZE] ;
	float			coef ;
	float			fric ;
	BOOL			slow_press ;
	float			pow_press ;
	int				an_tgt[2] ;
	BOOL			abs_mode ;	// 絶対値移動モード

	/*-- 入力取得 ------------------------------------------------------*/

	hr = dev->pDev->GetDeviceState(sizeof(DIMOUSESTATE), &ms) ;
	if( FAILED(hr) )
	{
		/*-- 入力取得失敗 --*/
		PadBuffInit(&dev->pad_data) ;
		return ;
	}
	/*------------------------------------------------------------------*/

	pad_data = dev->pad_data ;

	/*-- 入力変換 ------------------------------------------------------*/

	u32State = 0 ;
	abs_mode = FALSE ;

	if( ms.rgbButtons[1] ){ u32State |= PAD_Y ; }
	if( ms.rgbButtons[2] ){ u32State |= PAD_AR ; }

	if( ms.rgbButtons[0] ){ abs_mode = TRUE ; }

	slow_press = FALSE ;
	pow_press  = (float)0xff ;
	/*------------------------------------------------------------------*/

	/*-- 感圧エミュレーション ------------------------------------------*/

	ZeroMemory(press_tgt, sizeof(press_tgt)) ;

	{
		DWORD	*p2b ;
		float	*tgt_ptr ;

		p2b     = wpad_press_to_button_tbl ;
		tgt_ptr = press_tgt ;
		for(i=PRESSURE_SIZE; i>0; i--, p2b++, tgt_ptr++)
		{
			if( u32State & *p2b ){ *tgt_ptr = pow_press ; }
		}
	}

	if( slow_press ){ fric = MOUSE_SLOW_PRESS_SFRIC ; }	// ゆっくり
	else{ fric = MOUSE_SLOW_PRESS_NFRIC ; }				// 早め

	for(i=0; i<PRESSURE_SIZE; i++)
	{
		coef = (float)pad_data.pressure.data[i] ;
		coef += (press_tgt[i] - coef) * fric ;
		pad_data.pressure.data[i]  = (unsigned char)coef ;
	}
	/*------------------------------------------------------------------*/

	/*-- スティックアナログ情報 ----------------------------------------*/

	if( abs_mode )
	{
		an_tgt[0] = (int)pad_data.analog.data[0] ;
		an_tgt[1] = (int)pad_data.analog.data[1] ;

		an_tgt[0] += (int)((float)ms.lX * MOUSE_MOVE_ABS_COEF) ;
		an_tgt[1] += (int)((float)ms.lY * MOUSE_MOVE_ABS_COEF) ;
	}
	else
	{
		an_tgt[0] = (int)((float)ms.lX * MOUSE_MOVE_OFS_COEF) + 128  ;
		an_tgt[1] = (int)((float)ms.lY * MOUSE_MOVE_OFS_COEF) + 128  ;
	}

	if(      an_tgt[0] < 0   ){ an_tgt[0] = 0 ; }
	else if( an_tgt[0] > 255 ){ an_tgt[0] = 255 ; }
	if(      an_tgt[1] < 0   ){ an_tgt[1] = 0 ; }
	else if( an_tgt[1] > 255 ){ an_tgt[1] = 255 ; }

	coef = (float)pad_data.analog.data[0] ;
	coef += ((float)an_tgt[0] - coef) * 0.4f ;
	pad_data.analog.data[0] = (unsigned char)coef ;

	coef = (float)pad_data.analog.data[1] ;
	coef += ((float)an_tgt[1] - coef) * 0.4f ;
	pad_data.analog.data[1] = (unsigned char)coef ;

	/*------------------------------------------------------------------*/

	/*-- ボタンエミュレーション ----------------------------------------*/

	u32State &= ~(PAD_U|PAD_D|PAD_L|PAD_R|PAD_X|PAD_A|PAD_B|PAD_Y
				|PAD_L1|PAD_R1|PAD_L2|PAD_R2) ;

	if( pad_data.pressure.data[PAD_PRESS_X] > 0x40 ){ u32State |= PAD_X ; }
	if( pad_data.pressure.data[PAD_PRESS_Y] > 0x40 ){ u32State |= PAD_Y ; }
	/*------------------------------------------------------------------*/

	pad_data.button = ~u32State & 0xffffffff;

	// その他雑多な情報を設定
	pad_data.flag = 0;		// 通信成功
	pad_data.id = 0x79;		// DS2 を示す

	dev->pad_data = pad_data ;
}

static void check_dev_used( WPAD_DEVICE *dev )
{
	PADBUF_DATA		pad_data ;

	pad_data = dev->pad_data ;

	/*-- 入力があったら使用中(感圧は敢えて判定しません(ボタン入力で大体OK)) --*/
	if(  (pad_data.button ^ 0xffffffff)
	  || (pad_data.analog.data[0] != 128)
	  || (pad_data.analog.data[1] != 128)
	  || (pad_data.analog.data[2] != 128)
	  || (pad_data.analog.data[3] != 128) )
	{
		dev->use = TRUE ;	// 使用中
	}
}

static int pad_read_act( void )
{	// パッドデータを読み込んでエミュレーション

#ifndef _WINDOWS	//(#ifdef _XBOX)
	// 動的な抜き差しを見る
	DWORD dwInsertions, dwRemovals;
	XGetDeviceChanges(XDEVICE_TYPE_GAMEPAD, &dwInsertions, &dwRemovals);

	XPAD* pXpad;
	int i;
	int j;
	for( i = 0, pXpad = &xpad[0]; i < PAD_NUM; i++, pXpad++ ){
		// XGetDeviceChanges は 抜かれて挿された場合通知する
		// 挿されて抜かれたのは通知しない
		if( dwRemovals & (1<<i) ){	//抜かれたのを処理
			XInputClose( pXpad->hDevice );
			pXpad->pad_status = PAD_STATUS_NO_CONNECT;
			pXpad->hDevice = NULL;
		}
		if( dwInsertions & (1<<i) ){	// 挿されたのを処理
			pXpad->hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i,
										 XDEVICE_NO_SLOT, NULL);
			XInputGetCapabilities(pXpad->hDevice, &pXpad->caps);
			pXpad->pad_status = PAD_STATUS_CONNECT_DS2;
		}
		if(pXpad->hDevice){
			// 挿されているものに関してパッドデータを読み込み
			XINPUT_STATE xiState;
			XInputGetState(pXpad->hDevice, &xiState);
			
			// ボタンエミュレーション
			DWORD	u32State = 0xffffffff;
			WORD	wButtons = xiState.Gamepad.wButtons;
			for(j = 0; j < sizeof(tabButton)/sizeof(EMTABLE) ; j++){
				if(wButtons & 1){
					// 該当ボタンが押されている
					u32State &= ~tabButton[j].u32ButtonFlag;
					if(tabButton[j].s16PressOrder >= 0){
						pXpad->pad_data.pressure.data[tabButton[j].s16PressOrder] = 0xff;
					}
				}else{
					if(tabButton[j].s16PressOrder >= 0){
						pXpad->pad_data.pressure.data[tabButton[j].s16PressOrder] = 0;
					}
				}
				wButtons >>= 1;
			}
			for( j = 0 ; j < sizeof(tabPress)/sizeof(EMTABLE) ; j++){
				if(tabPress[j].s16PressOrder >=0){
					// 感圧情報を移動
					pXpad->pad_data.pressure.data[tabPress[j].s16PressOrder]
							= xiState.Gamepad.bAnalogButtons[j];
				}
				if(xiState.Gamepad.bAnalogButtons[j] > 0){
					// デジタル情報に含める
					u32State &= ~tabPress[j].u32ButtonFlag;
				}
			}
			pXpad->pad_data.button[0] = (u32State >> 8) & 0xff;
			pXpad->pad_data.button[1] = (u32State >> 0) & 0xff;
			// スティックのアナログ情報を移動
			pXpad->pad_data.analog.data[0] = xiState.Gamepad.sThumbRX / 256 + 128;
			pXpad->pad_data.analog.data[1] = -(xiState.Gamepad.sThumbRY + 1)/ 256 + 128;
			pXpad->pad_data.analog.data[2] = xiState.Gamepad.sThumbLX / 256 + 128;
			pXpad->pad_data.analog.data[3] = -(xiState.Gamepad.sThumbLY + 1)/ 256 + 128;

			// その他雑多な情報を設定
			pXpad->pad_data.flag = 0;	// 通信成功
			pXpad->pad_data.id = 0x79;	// DS2 を示す

			// 振動処理もやる
			
			pXpad->feedback.Rumble.wLeftMotorSpeed = (WORD)pXpad->s32LMotor;
			pXpad->feedback.Rumble.wRightMotorSpeed = (WORD)pXpad->s32RMotor;
			XInputSetState(pXpad->hDevice, &pXpad->feedback);

			pXpad->s32LMotor = 0;	// 振動リセット
			pXpad->s32RMotor = 0;	// 振動リセット
			
			if(pXpad->s32LMotor < 0) pXpad->s32LMotor = 0;
			if(pXpad->s32RMotor < 0) pXpad->s32RMotor = 0;
		}
	}
#else
	WPAD_DEVICE		*dev ;
	int				i, j ;
	int				vib ;
	HRESULT    		 hr;

	vib = wpad.s32RMotor + wpad.s32LMotor ;

	dev = wpad.dev ;
	for(i=wpad.dev_num; i>0; i--,  dev++ )
	{
		if( dev->pDev == NULL){ continue ; }

		hr = dev->pDev->Poll() ;
		if( FAILED(hr) ){
			// 接続が切れているっぽいのでAcquireを試みます
			dev->pDev->Acquire() ;

			// 内容初期化
			PadBuffInit(&dev->pad_data) ;
#if	__KEYBOARD_USE_BUFFERING_DATA__
			switch( dev->caps.dwDevType & 0xff )
			{
			  case DI8DEVTYPE_KEYBOARD :
				ZeroMemory(dev->keystate, sizeof(dev->keystate)) ;
				break ;
			}
#endif

			// 次回に処理は持ち越し
			continue ;
		}

		// DEVICE TYPEで処理分岐
		DWORD u32State = 0;
		switch( dev->caps.dwDevType & 0xff )
		{
		  case DI8DEVTYPE_GAMEPAD :
		  case DI8DEVTYPE_JOYSTICK :
			pad_read_act_joystick(dev) ;
			break ;

		  case DI8DEVTYPE_KEYBOARD :
			pad_read_act_keyboard(dev) ;
			break ;

		  case DI8DEVTYPE_MOUSE :
		  case DI8DEVTYPE_SCREENPOINTER :
			if( CurMouseEnable )
			{
				pad_read_act_mouse(dev) ;
			}
			else
			{
				/* 使用できないので内容初期化 */
				ZeroMemory(&dev->pad_data, sizeof(PADBUF_DATA)) ;

				dev->pad_data.analog.data[0] = 0x80 ;
				dev->pad_data.analog.data[1] = 0x80 ;
				dev->pad_data.analog.data[2] = 0x80 ;
				dev->pad_data.analog.data[3] = 0x80 ;
#ifndef _WINDOWS
				dev->pad_data.button[0] = 0xff;
				dev->pad_data.button[1] = 0xff;
#else
				dev->pad_data.button = 0xffffffff;
#endif
			}
			break ;
		}

		// プレイヤ使用判定
		if( !dev->use ){ check_dev_used(dev) ; }

		// 振動
		if( dev->use && dev->pEffect )
		{
			if( (vib > 0) )
			{
				float	coef ;

				if( !dev->vib_start )
				{
					dev->pEffect->Start(-1, 0) ;
					dev->vib_start = TRUE ;
				}

				// 振動力設定
				if( vib > 128 )
				{
					dev->periodic.dwMagnitude = DI_FFNOMINALMAX ;
				}
				else
				{
					dev->periodic.dwMagnitude = vib * (DI_FFNOMINALMAX/128) ;
				}

				// 振動数
				coef = 0.15f - ((float)vib * (0.15f/128.0f)) ;
				if( coef < 0.05f ){ coef = 0.05f ; }
				dev->periodic.dwPeriod = (DWORD)(coef * (float)DI_SECONDS) ;

			}
			else
			{
				if( dev->vib_start )
				{
					dev->pEffect->Stop() ;
					dev->vib_start = FALSE ;
				}
			}
		}
	}
	wpad.s32LMotor = 0 ;	// 振動リセット
	wpad.s32RMotor = 0 ;	// 振動リセット

	/*-- 入力情報の統合 ------------------------------------------------*/

	{
		PADBUF_DATA		pad_data ;
		int				analog_data[4] ;
		DWORD			pressure_data[PRESSURE_SIZE] ;
		unsigned char	*src_pd_ptr ;
		DWORD			*pd_ptr ;
		DWORD			button;

		ZeroMemory(analog_data, sizeof(analog_data)) ;
		ZeroMemory(pressure_data, sizeof(pressure_data)) ;
		button = 0 ;
		dev = wpad.dev ;
		for(i=wpad.dev_num; i>0; i--, dev++)
		{
			if( dev->pDev == NULL){ continue ; }
			if( !dev->use ){ continue ; }

			pad_data = dev->pad_data ;

			/*-- Analog入力 --------------------------------------------*/

			analog_data[0] += ((int)pad_data.analog.data[0] - 128) ;
			analog_data[1] += ((int)pad_data.analog.data[1] - 128) ;
			analog_data[2] += ((int)pad_data.analog.data[2] - 128) ;
			analog_data[3] += ((int)pad_data.analog.data[3] - 128) ;
			/*----------------------------------------------------------*/

			/*-- 感圧情報 ----------------------------------------------*/

			src_pd_ptr = pad_data.pressure.data ;
			pd_ptr     = pressure_data ;
			for(j=PRESSURE_SIZE; j>0; j--, src_pd_ptr++, pd_ptr++)
			{
#if FALSE
				if( *pd_ptr < (DWORD)*src_pd_ptr ){ *pd_ptr = (DWORD)*src_pd_ptr ; }
#else
				*pd_ptr += (DWORD)*src_pd_ptr ;
#endif
			}
			/*----------------------------------------------------------*/

			/*-- ボタン入力 --------------------------------------------*/

			button |= ~pad_data.button ;
			/*----------------------------------------------------------*/
		}

		/*-- 数値補正 --------------------------------------------------*/

		analog_data[0] += 128 ;
		analog_data[1] += 128 ;
		analog_data[2] += 128 ;
		analog_data[3] += 128 ;
		if(      analog_data[0] < 0    ){ analog_data[0] = 0 ; }
		else if( analog_data[0] > 0xff ){ analog_data[0] = 0xff ; }
		if(      analog_data[1] < 0    ){ analog_data[1] = 0 ; }
		else if( analog_data[1] > 0xff ){ analog_data[1] = 0xff ; }
		if(      analog_data[2] < 0    ){ analog_data[2] = 0 ; }
		else if( analog_data[2] > 0xff ){ analog_data[2] = 0xff ; }
		if(      analog_data[3] < 0    ){ analog_data[3] = 0 ; }
		else if( analog_data[3] > 0xff ){ analog_data[3] = 0xff ; }

#if FALSE
		pd_ptr = pressure_data ;
		for(j=PRESSURE_SIZE; j>0; j--, pd_ptr++)
		{
			if( *pd_ptr > 0xff ){ *pd_ptr = 0xff ; }
		}
#endif
		/*--------------------------------------------------------------*/

		/*-- 設定 ------------------------------------------------------*/

		wpad.pad_data.analog.data[0] = (unsigned char)analog_data[0] ;
		wpad.pad_data.analog.data[1] = (unsigned char)analog_data[1] ;
		wpad.pad_data.analog.data[2] = (unsigned char)analog_data[2] ;
		wpad.pad_data.analog.data[3] = (unsigned char)analog_data[3] ;

		src_pd_ptr = wpad.pad_data.pressure.data ;
		pd_ptr     = pressure_data ;
		for(j=PRESSURE_SIZE; j>0; j--, pd_ptr++, src_pd_ptr++)
		{
			*src_pd_ptr = (unsigned char)*pd_ptr ;
		}

		wpad.pad_data.button = ~button ;

		// その他雑多な情報を設定
		wpad.pad_data.flag = 0;		// 通信成功
		wpad.pad_data.id = 0x79;	// DS2 を示す
		/*--------------------------------------------------------------*/
	}
	/*------------------------------------------------------------------*/

#endif	
	return 1 ;
}

/* ------------------------------------------- */

GV_PAD GV_PadData[ GV_PAD_MAX ] = { { 0 } };
GV_PAD GV_PadDataDirect[ GV_PAD_MAX ] = { { 0 } };

static	int	PadOrg = 0 ;
//static	int	GV_PadStatus = 0 ;
//static	int	GV_UnablePadUpdateCount = 0 ;


/*-----------------------------------------------------------------*/
/* 左アナログスティックの入力を十字キー状態に反映させる */
static	inline	int	AnalogStat( u_char dx, u_char dy, int stat )
{
    stat &= ~PAD_UDLR ;
    if ( dx < 128 - ANALOG_MARGIN ) stat |= PAD_L ;
    else if ( dx > 128 + ANALOG_MARGIN ) stat |= PAD_R ;
    if ( dy < 128 - ANALOG_MARGIN ) stat |= PAD_U ;
    else if ( dy > 128 + ANALOG_MARGIN ) stat |= PAD_D ;
    return stat ;
}

/* アナログスティックの入力から方向を計算 */
static	inline	int	GetPadDirAnalog( unsigned char* pad )
{
    FVECTOR		vec ;
    
    vec.vx = ( float )( *pad - 128 ) ;
    vec.vz = ( float )( *( pad + 1 ) - 128 ) ;
    if ( vec.vx > -ANALOG_MARGIN_F && vec.vx < ANALOG_MARGIN_F &&
		vec.vz > -ANALOG_MARGIN_F && vec.vz < ANALOG_MARGIN_F ) {
		return -1 ;
    }
    return ( ( GV_VecDir2( &vec ) + PadOrg ) & 4095 ) ;
}

/* 十字キーのビットから入力方向を計算 */
static	inline	int GetPadDir( int pad )
{
	static	short	Dirs[] = {
		0x000, 0x800, 0x400, 0x600,
		0x000, 0x000, 0x200, 0x000,
		0xc00, 0xa00, 0x000, 0x000,
		0xe00, 0x000, 0x000, 0x000
	} ;
	pad &= PAD_UDLR ;
	if ( pad == 0 ) return -1 ;
	return 4095 & ( Dirs[ pad >> 12 ] + PadOrg ) ;
}

#define BUTTON_NUM	16

static signed char conv_table[ 1 ][ BUTTON_NUM ] = {
	{
		PAD_PRESS_L,
		PAD_PRESS_D,
		PAD_PRESS_R,
		PAD_PRESS_U,
		-1, -1, -1, -1,
		PAD_PRESS_Y,
		PAD_PRESS_B,
		PAD_PRESS_A,
		PAD_PRESS_X,
		PAD_PRESS_R1,
		PAD_PRESS_L1,
		PAD_PRESS_R2,
		PAD_PRESS_L2,
	}
};

static inline void setup_pressure( unsigned char *pressure, int button )
{
	/* ボタンが押されていたら255に感圧情報をセット */

	signed char *table;
	int i, mask;

	table = conv_table[ 0 ];		// Keyconfigが入ったら変える
		mask = 0x8000;

	for( i = 0; i < BUTTON_NUM; i++ ){
		if( *table >= 0 ){
			if( button & mask ){
				pressure[ *table ] = 0xFF;
			} else {
				pressure[ *table ] = 0x00;
			}
		}
		table++;
		mask >>= 1;
	}
}

#ifdef DEBUG_MODE
int GV_PadHangupCheck( void )
{
	
#if 0
	/* pad 1のみを見て、パッドリセット */
	PADBUF_DATA *pd;
	int status;

	pd = ( PADBUF_DATA * )pad_data[ 0 ];

	if( scePadRead( 0, 0, pad_data[ 0 ] ) == 0 ) return 0;
	if( pd->flag != 0 ) return 0;

	status = ( ( pd->button[ 0 ] << 8 ) | ( pd->button[ 1 ] ) ) ^ 0xffff ;

	if( ( status & HANGUP_KEY ) == HANGUP_KEY ){
		return 1;
	}
#endif
	return 0;
}
#endif

/*-----------------------------------------------------------------*/

void GV_InitPadSystem( void )
{
	/* システム初期化 */
	int i;

#ifndef _WINDOWS
	// XInputライブラリのイニシャライズ
	static XDEVICE_PREALLOC_TYPE deviceTypes[] = {
		{XDEVICE_TYPE_GAMEPAD, PAD_NUM},
//		{XDEVICE_TYPE_MEMORY_UNIT, MU_NUM},	// MUには対応しない
//		{XDEVICE_TYPE_DEBUG_KEYBOARD, 1},		
	};
	XInitDevices( sizeof(deviceTypes) / sizeof(XDEVICE_PREALLOC_TYPE), deviceTypes );

#if 0	// デバッグ用キーボードキューの設定
    XINPUT_DEBUG_KEYQUEUE_PARAMETERS xdkp = {
        XINPUT_DEBUG_KEYQUEUE_FLAG_KEYDOWN     |
         XINPUT_DEBUG_KEYQUEUE_FLAG_KEYREPEAT  |
         XINPUT_DEBUG_KEYQUEUE_FLAG_ASCII_ONLY,
        50,
        500,
        70 };
    XInputDebugInitKeyboardQueue( &xdkp );
#endif	

#if 0
	//	使用可能なパッドを得る
	DWORD	dwDeviceMask = XGetDevices( XDEVICE_TYPE_GAMEPAD );
#endif	

	for(i = 0 ; i < PAD_NUM; i++){
#if 0		
		if( dwDeviceMask & (1<<i) ){
			// パッドがつながってる
			xpad[i].hDevice = XInputOpen(XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL);
			XInputGetCapabilities(xpad[i].hDevice, &xpad[i].caps);
			xpad[i].pad_status = PAD_STATUS_CONNECT_DS2;	// Dualshock2 相当
		}else{
#endif
			xpad[i].pad_status = PAD_STATUS_NO_CONNECT;
			xpad[i].hDevice = NULL;
#if 0			
		}
#endif			
		xpad[i].pad_data.button[0] = 0xff;	// 全ボタンリリース
		xpad[i].pad_data.button[1] = 0xff;	// 全ボタンリリース

		xpad[i].s32LMotor = 0;
		xpad[i].s32RMotor = 0;
		xpad[i].feedback.Header.hEvent = NULL;

		portmap[ i ] = i;	// ポートマップ初期化
	}

#else	// 以降Windows

	// コンフィグ設定読み込み
	GV_InitWPadConfig() ;

	// 通常操作モード設定
	GV_SetWindowsInputConfigulationMode(FALSE) ;

	// パッドデータの初期化
	GV_ZeroMemory(&wpad, sizeof(WPAD));
	for(i = 0; i < PAD_NUM; i++){
		portmap[ i ] = i;	// ポートマップ初期化
	}
	// DirectInput8 オブジェクト作成
	HRESULT hr;
	if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION,
										 IID_IDirectInput8, (VOID**)&pDI, NULL))){
		ASSERT(0);
	}

	// 入力Deviceを探す
    if( FAILED( hr = pDI->EnumDevices( DI8DEVCLASS_KEYBOARD, 
									   EnumJoysticksCallback,
									   NULL, DIEDFL_ATTACHEDONLY ))){
		ASSERT(0);
	}

	if(  X2W_GetAppConfig(X2WAPPCFG_MOUSE_ENABLE)
	  || X2W_GetAppConfig(X2WAPPCFG_MOUSE_ENABLE2) )	// MOUSE系入力許可
	{
	    if( FAILED( hr = pDI->EnumDevices( DI8DEVCLASS_POINTER, 
										   EnumJoysticksCallback,
										   NULL, DIEDFL_ATTACHEDONLY ))){
			ASSERT(0);
		}
	}
	if( FAILED( hr = pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, 
									   EnumJoysticksCallback,
									   NULL, DIEDFL_ATTACHEDONLY ))){
		ASSERT(0);
	}
	WPAD_DEVICE				*dev ;
	LPDIRECTINPUTDEVICE8	pDev ;

	dev = wpad.dev ;
	for(i=wpad.dev_num; i>0; i--, dev++)
	{
		pDev = dev->pDev ;
		if( !pDev ){ continue ; }
		printf("pad %d found\n", i);

		/*-- その他設定 --*/
		dev->pad_status = PAD_STATUS_CONNECT_DS2;	// Dualshock2 相当
	}
#endif	// _WINDOWS
	GV_PadSetDefaultKeyConf() ;
}

void GV_ResetPadSystem( void )
{
	GV_PAD *pad;
	int i;

	pad = GV_PadData;

	for( i = GV_PAD_MAX; i > 0; i-- ){
		pad->status = pad->press = pad->release = 0;
		pad->dir = -1;
		
		pad->flag = 0 ;			/* 状態フラグの初期化 M.Sonoyama */

		pad++;
	}
}

typedef	struct	{
	int			type ;
	int			status ;
	int			dir ;
	int			anaval ;
	int			analog_flag ;
	unsigned char* pressure ;
} UPDATEPAD ;

static	inline	void	SET_UPDATEPAD( UPDATEPAD *up, int type, int status, int dir, 
									   int anaval, int analog_flag, u_char *pressure )
{
	up->type = type ;
	up->status = status ;
	up->dir = dir ;
	up->anaval = anaval ;
	up->analog_flag = analog_flag ;
	up->pressure = (BYTE *)pressure ;
}



/* キーコンフィグ関連 */
/*-----------------------------------------------------------------*/
/* キーコンフィグ本体 */
static char key_config_table[16] ;
/* キープレスの順序→ステータス値 */
static int botton2status_table[] = {
	PAD_R ,	PAD_L ,	PAD_U ,	PAD_D ,
	PAD_X ,	PAD_A ,	PAD_B ,	PAD_Y ,
	PAD_L1,	PAD_R1,	PAD_L2,	PAD_R2,
	PAD_AL, PAD_AR, PAD_STA,PAD_SEL,
} ;
/* キーコンフィグデフォルト値 */
static char key_config_table_default[] = {
	PAD_PRESS_R	,	PAD_PRESS_L	,	PAD_PRESS_U	,	PAD_PRESS_D	,
	PAD_PRESS_X	,	PAD_PRESS_A	,	PAD_PRESS_B	,	PAD_PRESS_Y	,
	PAD_PRESS_L1,	PAD_PRESS_R1,	PAD_PRESS_L2,	PAD_PRESS_R2,
	12,13,	14,15,
} ;
/* 変えられているかどうかのフラグ PAD_??に準拠 */
u_int  key_config_flag ;

void GV_PadSetDefaultKeyConf( void )
{
	key_config_flag = 0 ;
	memcpy( key_config_table, key_config_table_default, sizeof(key_config_table) ) ;
}

/* aをbに変える PAD_PRESS_???を使うこと */
void GV_PadSetKeyConf( int a, int b )
{
	key_config_flag |= botton2status_table[a] ;
	key_config_table[a] = b ;
}

/* ステータスをキーコンフィグに沿って変える */
int GV_PadGetStatus( int input )
{
	if ( key_config_flag & input ) {
		int i, status = 0 ;

		for( i=0 ; i<16 ; i++ ) {
			if ( key_config_table[i] >= 0 ) {
				if ( input & 1 )
				  status |= 1<<key_config_table[i] ;
			}
			input >>= 1 ;
		}
		return status ;
	}
	return input ;
}

/* キーコンフィグを反映させる */
static inline void	SetKeyConfiguration( UPDATEPAD *pad )
{
	UPDATEPAD prev ;
	int  i ;

	if ( !(key_config_flag & pad->status ) ) {
		return ;
	}
	prev = *pad ;
#ifndef _WINDOWS
	pad->status &= 0xf000 ;
#else
	pad->status &= (PAD_AL | PAD_AR | PAD_STA | PAD_SEL) ;
#endif
	for( i=0 ; i<12 ; i++ ) {
		if ( prev.status & botton2status_table[i] ) {
			pad->status |= botton2status_table[key_config_table[i]] ;
			pad->pressure[key_config_table[i]] = prev.pressure[i] ;
		}
	}
}
/*-----------------------------------------------------------------*/


/* 整合性チェック */
static	void	CheckPad( GV_PAD *pad )
{
	if ( ( pad->status & PAD_A ) && ( pad->pressure[ PAD_PRESS_A ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_A ] = 255 ;
	} else if ( !( pad->status & PAD_A ) && ( pad->pressure[ PAD_PRESS_A ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_A ] = 0 ;		
	}
	if ( ( pad->status & PAD_B ) && ( pad->pressure[ PAD_PRESS_B ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_B ] = 255 ;
	} else if ( !( pad->status & PAD_B ) && ( pad->pressure[ PAD_PRESS_B ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_B ] = 0 ;		
	}
	if ( ( pad->status & PAD_X ) && ( pad->pressure[ PAD_PRESS_X ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_X ] = 255 ;
	} else if ( !( pad->status & PAD_X ) && ( pad->pressure[ PAD_PRESS_X ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_X ] = 0 ;		
	}
	if ( ( pad->status & PAD_Y ) && ( pad->pressure[ PAD_PRESS_Y ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_Y ] = 255 ;
	} else if ( !( pad->status & PAD_Y ) && ( pad->pressure[ PAD_PRESS_Y ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_Y ] = 0 ;		
	}
	if ( ( pad->status & PAD_U ) && ( pad->pressure[ PAD_PRESS_U ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_U ] = 255 ;
	} else if ( !( pad->status & PAD_U ) && ( pad->pressure[ PAD_PRESS_U ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_U ] = 0 ;		
	}
	if ( ( pad->status & PAD_D ) && ( pad->pressure[ PAD_PRESS_D ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_D ] = 255 ;
	} else if ( !( pad->status & PAD_D ) && ( pad->pressure[ PAD_PRESS_D ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_D ] = 0 ;		
	}
	if ( ( pad->status & PAD_L ) && ( pad->pressure[ PAD_PRESS_L ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_L ] = 255 ;
	} else if ( !( pad->status & PAD_L ) && ( pad->pressure[ PAD_PRESS_L ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_L ] = 0 ;		
	}
	if ( ( pad->status & PAD_R ) && ( pad->pressure[ PAD_PRESS_R ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_R ] = 255 ;
	} else if ( !( pad->status & PAD_R ) && ( pad->pressure[ PAD_PRESS_R ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_R ] = 0 ;		
	}
	if ( ( pad->status & PAD_L1 ) && ( pad->pressure[ PAD_PRESS_L1 ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_L1 ] = 255 ;
	} else if ( !( pad->status & PAD_L1 ) && ( pad->pressure[ PAD_PRESS_L1 ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_L1 ] = 0 ;		
	}
	if ( ( pad->status & PAD_L2 ) && ( pad->pressure[ PAD_PRESS_L2 ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_L2 ] = 255 ;
	} else if ( !( pad->status & PAD_L2 ) && ( pad->pressure[ PAD_PRESS_L2 ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_L2 ] = 0 ;		
	}
	if ( ( pad->status & PAD_R1 ) && ( pad->pressure[ PAD_PRESS_R1 ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_R1 ] = 255 ;
	} else if ( !( pad->status & PAD_R1 ) && ( pad->pressure[ PAD_PRESS_R1 ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_R1 ] = 0 ;		
	}
	if ( ( pad->status & PAD_R2 ) && ( pad->pressure[ PAD_PRESS_R2 ] == 0 ) ) {
		pad->pressure[ PAD_PRESS_R2 ] = 255 ;
	} else if ( !( pad->status & PAD_R2 ) && ( pad->pressure[ PAD_PRESS_R2 ] > 0 ) ) {
		pad->pressure[ PAD_PRESS_R2 ] = 0 ;		
	}
}

static void UpdatePad( GV_PAD *pad, UPDATEPAD *up )
{
	int prev;

	prev = pad->status;

	pad->type = PAD_TYPE( up->type ) ;
	pad->status = up->status ;
	pad->press = ( up->status & ~prev ) ;
	pad->release = ( ~up->status & prev ) ;

	pad->dir = up->dir ;
	pad->analog_input = up->analog_flag ;
	*( int * )&( pad->right_dx ) = up->anaval ;

	/* DUALSHOCK2の時は感圧情報も読み込む */
	if( up->type >= PAD_STATUS_CONNECT_DS2 ){
		memcpy( pad->pressure, up->pressure, PRESSURE_SIZE ) ;
	} else {
		setup_pressure( pad->pressure, up->status ) ;
	}
	/* 整合性チェック */
	/* もっとうまい方法がないだろうか */
	if ( !( pad->flag & ( GV_PAD_RELEASE | GV_PAD_MASK | GV_PAD_PRESS_SCN ) ) ) return ;
	CheckPad( pad ) ;
}

static	void	SetPadState( UPDATEPAD *up, GV_PAD *pad, PADBUF_DATA *p, int type )
{
	int dir, status, ana_flag, ana_value, stat ;

	if ( IS_AVAILABLE( type ) ) {
#ifndef _WINDOWS
		status = ( ( p->button[ 0 ] << 8 ) | ( p->button[ 1 ] ) ) ^ 0xffff ;
#else
		status = p->button ^ 0xffffffff ;
#endif
	} else {
		status = 0 ;
	}
	/* analog or normal */
	/* GV_PAD_DIGITAL or GV_PAD_ANALOG に合わせてある */
	ana_flag = 0;
	/* アナログデータのコピー */
	if( type >= PAD_STATUS_CONNECT_AC ) {
		ana_value = *( int * )( &p->analog ) ;
		/* アナログ右スティック使用中？ */
		if ( ( p->analog.data[ 0 ] > 128 + ANALOG_MARGIN ) ||
			( p->analog.data[ 0 ] < 128 - ANALOG_MARGIN ) ||
			( p->analog.data[ 1 ] > 128 + ANALOG_MARGIN ) ||
			( p->analog.data[ 1 ] < 128 - ANALOG_MARGIN ) ) {	// デジタル情報に含める
			ana_flag |= GV_PAD_ANALOG_R_USE ;
		}
	} else {
		ana_value = 0x80808080 ;
	}
	
	/* 十字キーと左アナログスティック同時のときは十字キー優先 */
	if ( !( status & PAD_UDLR ) && ( type >= PAD_STATUS_CONNECT_AC ) ) {
		/* デジタルボタンが使用されていない */
		if ( pad->flag & GV_PAD_SEPARATE_STICK ) {
			stat = status ;
		} else {
			stat = AnalogStat( p->analog.data[ 2 ], p->analog.data[ 3 ], status ) ;
		}
		dir = GetPadDirAnalog( &p->analog.data[ 2 ] ) ;
		if ( dir != -1 ){
			ana_flag |= GV_PAD_ANALOG_L_USE ;
		} else {
			ana_flag &= ~GV_PAD_ANALOG_L_USE ;
		}
	} else {
		ana_flag &= ~GV_PAD_ANALOG_L_USE ;
		dir = GetPadDir( status ) ;
		stat = status ;
	}
	SET_UPDATEPAD( up, type, stat, dir, ana_value, ana_flag, (unsigned __int8 *)p->pressure.data ) ;
}

/* ボタン圧力曲線 */
/*-----------------------------------------------------------------*/
/* ボタン圧力曲線の選択 */

#if TRUE
static char key_pressure_config[16] ;

static inline void SetChangePressure( int press_key, int type )
{
	ASSERT( type>=0 && type<=2 ) ;
	key_pressure_config[press_key] = type ;
}

static void InitChangePressure( void )
{
	int i ;

	for( i=16 ; --i>= 0 ; ) {
		key_pressure_config[i] = 0 ;
	}
}

static void ChangePressure( GV_PAD *pad, UPDATEPAD *up )
{
	int i, press ;

	if ( (pad->analog_input & GV_PAD_ANALOG_CHANGE) == 0 ) {
		return ;
	}

	/*XBOXとPS2の感圧の感触をあわせるための処理*/
	for( i=PRESSURE_SIZE ; --i>=0 ; ) {
		if ( pad->analog_input & (0x0010<<i) ) {
			switch( key_pressure_config[i] ) {
			  case 2: /* L1 R1 ハンドガン用 */
				press = up->pressure[i]<120 ? 0 : up->pressure[i]-120 ;
				press = (int)(press * press * 0.0093664f) - 60 ;
				if ( press<0 ) {
					up->status &= ~botton2status_table[i] ;
				}
				up->pressure[i] = press>255 ? 255 : press<0 ? 0 : press ;
				break ;
			  case 1:
				press = up->pressure[i]<60 ? 0 : up->pressure[i]-60 ;
				press = (int)(press * press * 0.0093664f) ;
				if ( (up->status & botton2status_table[i]) && (press==0) ) {
					press = 1 ;
				}
				up->pressure[i] = press>255 ? 255 : press ;
				break ;
			  default:
				break ;
			}
		}
	}
}
#else
static inline void SetChangePressure( int press_key, int type )
{
	printf("SetChangePressure(%d, %d)\n", press_key, type) ;
}

static void InitChangePressure( void ){}

static void ChangePressure( GV_PAD *pad, UPDATEPAD *up )
{
	int i, press ;

	/*XBOXとPS2の感圧の感触をあわせるための処理*/
	for( i=PRESSURE_SIZE ; --i>=0 ; ) {
		press = pad->pressure[i]<90 ? 0 : pad->pressure[i]-90 ;
		press = (int)(press * press * 0.0093664f) ;
		pad->pressure[i] = press>255 ? 255 : press ;
	}
}
#endif

void GV_UpdatePadSystem( void )
{
#if TRUE
	int 			i ;
	UPDATEPAD		updatepad, *up ;
	int 			type ;
	unsigned char	pressure[ PRESSURE_SIZE ] ;
	PADBUF_DATA 	*p ;
	GV_PAD			*pad ;
	int				mapid ;

	up = &updatepad ;
	pad_read_act();

	/* とりあえずパッド情報を更新 */

	p    = &wpad.pad_data ;
	type = PAD_STATUS_CONNECT_DS2 ;	// とりあえず

	i = 0 ;
	mapid = portmap[i] ;

	GV_PadDataDirect[mapid].flag &= ~GV_PAD_DISCONNECT;
	GV_PadData[mapid].flag &= ~GV_PAD_DISCONNECT;

	/* DIRECT のアップデート */
	pad = &GV_PadDataDirect[ mapid ] ;
	SetPadState( up, pad, p, type ) ;
	UpdatePad( pad, up ) ;

	/* NORMAL データのアップデート */
	pad = &GV_PadData[ mapid ] ;
	if ( pad->flag & GV_PAD_PAD_DEMO ) {
		/* パッドデモ */
		p = &demo_pad_data ;
		type = demo_pad_status ;
		SetPadState( up, pad, p, type ) ;
		pad->flag &= ~GV_PAD_PAD_DEMO ;
		/* キーコンフィグ処理 */
		UpdatePad( pad, up ) ;
	} else {
		memcpy( pressure, p->pressure.data, PRESSURE_SIZE ) ;  //仮
		up->pressure = pressure ;

		/* こっちが本当のキーコンフィグ処理 */
		SetKeyConfiguration( up ) ;

		/* リリース、マスク等の処理 */
		if ( pad->flag & GV_PAD_RELEASE ) {
			up->status = 0 ;
			/* シナリオプレス */
			/* リリース状態でも効くようにしてみた。(2001/08/06) */
			if ( pad->flag & GV_PAD_PRESS_SCN ) {
				up->status |= GV_PadPress[ i ] ;
				//up->type = PAD_STATUS_CONNECT_DS;
			} else {
				up->type = PAD_STATUS_CONNECT_DS;
			}
		} else {
			/* パッドマスク */
			if ( pad->flag & GV_PAD_MASK ) {
				if ( pad->flag & GV_PAD_MASK_PRG ){
					up->status &= GV_PadMask[ i * 2 ] ;
				}
				if ( pad->flag & GV_PAD_MASK_SCN ){
					up->status &= GV_PadMask[ i * 2 + 1 ] ;
				}
				//up->type = PAD_STATUS_CONNECT_DS;
			}
			/* シナリオプレス */
			if ( pad->flag & GV_PAD_PRESS_SCN ) {
				up->status |= GV_PadPress[ i ] ;
				//up->type = PAD_STATUS_CONNECT_DS;
			}
		}
		/* キーコンフィグ処理 */
		UpdatePad( pad, up ) ;
		/* ＸＢＯＸゲーム用に感圧値を変更 */
		ChangePressure( pad, up );
	}
	pad->flag &= ~GV_PAD_PRESS_SCN ;
#else
	int 			i ;
	UPDATEPAD		updatepad, *up ;

	up = &updatepad ;
	pad_read_act();

	/* とりあえずパッド情報を更新 */

	for( i = 0; i < PAD_NUM; i++ ){
		int 			type ;
		unsigned char	pressure[ PRESSURE_SIZE ] ;
		PADBUF_DATA 	*p ;
		GV_PAD			*pad ;
		int				mapid = portmap[ i ];

		p = &xpad[i].pad_data;
		type = xpad[i].pad_status;
#ifdef _WINDOWS
		if( xpad[i].pJoystick == NULL ) {
			GV_PadDataDirect[mapid].flag |= GV_PAD_DISCONNECT;
			GV_PadData[mapid].flag |= GV_PAD_DISCONNECT;
		} else {
			GV_PadDataDirect[mapid].flag &= ~GV_PAD_DISCONNECT;
			GV_PadData[mapid].flag &= ~GV_PAD_DISCONNECT;
		}
#else
		if( xpad[i].hDevice == NULL ) {
			GV_PadDataDirect[mapid].flag |= GV_PAD_DISCONNECT;
			GV_PadData[mapid].flag |= GV_PAD_DISCONNECT;
		} else {
			GV_PadDataDirect[mapid].flag &= ~GV_PAD_DISCONNECT;
			GV_PadData[mapid].flag &= ~GV_PAD_DISCONNECT;
		}
#endif		
		
		/* DIRECT のアップデート */
		pad = &GV_PadDataDirect[ mapid ] ;
		SetPadState( up, pad, p, type ) ;
		UpdatePad( pad, up ) ;

		/* NORMAL データのアップデート */
		pad = &GV_PadData[ mapid ] ;
		if ( pad->flag & GV_PAD_PAD_DEMO ) {
			/* パッドデモ */
			p = &demo_pad_data ;
			type = demo_pad_status ;
			SetPadState( up, pad, p, type ) ;
			pad->flag &= ~GV_PAD_PAD_DEMO ;
			/* キーコンフィグ処理 */
			UpdatePad( pad, up ) ;
		} else {
			memcpy( pressure, p->pressure.data, PRESSURE_SIZE ) ;  //仮
			up->pressure = pressure ;

			/* こっちが本当のキーコンフィグ処理 */
			SetKeyConfiguration( up ) ;

			/* リリース、マスク等の処理 */
			if ( pad->flag & GV_PAD_RELEASE ) {
				up->status = 0 ;
				/* シナリオプレス */
				/* リリース状態でも効くようにしてみた。(2001/08/06) */
				if ( pad->flag & GV_PAD_PRESS_SCN ) {
					up->status |= GV_PadPress[ i ] ;
					//up->type = PAD_STATUS_CONNECT_DS;
				} else {
					up->type = PAD_STATUS_CONNECT_DS;
				}
			} else {
				/* パッドマスク */
				if ( pad->flag & GV_PAD_MASK ) {
					if ( pad->flag & GV_PAD_MASK_PRG ){
						up->status &= GV_PadMask[ i * 2 ] ;
					}
					if ( pad->flag & GV_PAD_MASK_SCN ){
						up->status &= GV_PadMask[ i * 2 + 1 ] ;
					}
					//up->type = PAD_STATUS_CONNECT_DS;
				}
				/* シナリオプレス */
				if ( pad->flag & GV_PAD_PRESS_SCN ) {
					up->status |= GV_PadPress[ i ] ;
					//up->type = PAD_STATUS_CONNECT_DS;
				}
			}
			/* キーコンフィグ処理 */
			UpdatePad( pad, up ) ;
			/* ＸＢＯＸゲーム用に感圧値を変更 */
			ChangePressure( pad, up );
		}
		pad->flag &= ~GV_PAD_PRESS_SCN ;
	}
#endif

#ifdef USBKBD
	update_key();
#endif
}

void	GV_OriginPadSystem( int org )
{
    PadOrg = org ;
}

int	GV_GetPadOrigin( void )
{
    return PadOrg ;
}

void GV_SetPadVibration1( int port, int value )
{	// もともと１ビット
#ifndef _WINDOWS
	ASSERT(0 <= port && port < PAD_NUM);
	// ポートマップによって設定
	// （ここは逆引きになる）
	xpad[ invportmap( port ) ].s32RMotor = (value << 14) & 0xffff;
#else
	wpad.s32RMotor = (value << 14) & 0xffff;
#endif
}

void GV_SetPadVibration2( int port, int value )
{	// もともと１バイト
#ifndef _WINDOWS
	ASSERT(0 <= port && port < PAD_NUM);
	// ポートマップによって設定
	// （ここは逆引きになる）
	xpad[ invportmap( port ) ].s32LMotor = (value << 8) & 0xffff;
#else
	wpad.s32LMotor = (value << 8) & 0xffff;
#endif
}

// XBOXTCR対策用 ポートマップ
void GV_ChangePadPortMapping( int prevport, int newport )
{	// ゲーム側から見て prevport だったポートを
	// newport と入れ替える

	GV_PAD padtmp;
	int i;

	if( newport == prevport ) return;
#if TRUE	// >>>>>>>>>>>>>>>>
	// GV_PAD の入れ替え
	padtmp = GV_PadDataDirect[ newport ];
	GV_PadDataDirect[ newport ] = GV_PadDataDirect[ prevport ];
	GV_PadDataDirect[ prevport ] = padtmp;
	padtmp = GV_PadData[ newport ];
	GV_PadData[ newport ] = GV_PadData[ prevport ];
	GV_PadData[ prevport ] = padtmp;
#else //==================
	// GV_PAD の入れ替え
	padtmp = GV_PadDataDirect[ newport ];
	flag = GV_PadDataDirect[ newport ].flag;
	GV_PadDataDirect[ newport ] = GV_PadDataDirect[ prevport ];
	GV_PadDataDirect[ newport ].flag = flag;	// フラグは継承
	flag = GV_PadDataDirect[ prevport ].flag;
	GV_PadDataDirect[ prevport ] = padtmp;
	GV_PadDataDirect[ prevport ].flag = flag;	// フラグは継承
	
	padtmp = GV_PadData[ newport ];
	flag = GV_PadData[ newport ].flag;
	GV_PadData[ newport ] = GV_PadData[ prevport ];
	GV_PadData[ newport ].flag = flag;	// フラグは継承
	flag = GV_PadData[ prevport ].flag;
	GV_PadData[ prevport ] = padtmp;
	GV_PadData[ prevport ].flag = flag;	// フラグは継承
#endif // <<<<<<<<<<<<<<<<<<<<<<<<<<
	// マップの入れ替え
	for( i = 0 ; i < PAD_NUM ; i ++ ) {
		if( portmap[ i ] == prevport ) {
			portmap[ i ] = newport;
		} else if( portmap[ i ] == newport ) {
			portmap[ i ] = prevport;
		}
	}
}


/*--------------------------------------------------------*/

/* パッドリリース操作関係 */

/* パッドリリース */
static	inline	void	PadReleaseOn( int which, int flag )
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadData[ which ].flag |= flag ;
}

static	inline	void	PadReleaseOff( int which, int flag )
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadData[ which ].flag &= ~flag ;
}

void	GV_PadReleaseOnSystem( int which ) 
{
	PadReleaseOn( which, GV_PAD_RELEASE_SYSTEM ) ;
}

void	GV_PadReleaseOffSystem( int which ) 
{
	PadReleaseOff( which, GV_PAD_RELEASE_SYSTEM ) ;
}

void	GV_PadReleaseOn( int which )
{
	PadReleaseOn( which, GV_PAD_RELEASE_PRG ) ;
}

void	GV_PadReleaseOff( int which ) 
{
	PadReleaseOff( which, GV_PAD_RELEASE_PRG ) ;
}

void	GV_PadReleaseOnScn( int which )
{
	PadReleaseOn( which, GV_PAD_RELEASE_SCN ) ;
	/* 今だけ */
}

/* パッドマスク */
void	GV_PadMaskOn( int which, int mask )
{
    ASSERT( which >= 0 && which < 4 ) ;

    GV_PadMask[ which * 2 ] = ~mask ;
    GV_PadData[ which ].flag |= GV_PAD_MASK_PRG ;
}

void	GV_PadMaskOff( int which )
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadMask[ which * 2 ] = 0xffffffff ;
    GV_PadData[ which ].flag &= ~GV_PAD_MASK_PRG ;    
}

void	GV_PadMaskOnScn( int which, int mask )
{
    ASSERT( which >= 0 && which < 4 ) ;
    ASSERT( mask != 0 ) ;

    GV_PadMask[ which * 2 + 1 ] = ~mask ;
    GV_PadData[ which ].flag |= GV_PAD_MASK_SCN ;
}


/* アナログとデジタルスティックの分離*/
void	GV_PadSeparateOnScn( int which )
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadData[ which ].flag |=  GV_PAD_SEPARATE_STICK ;
}
void	GV_PadSeparateOffScn( int which )
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadData[ which ].flag &= ~GV_PAD_SEPARATE_STICK ;
}

/* 感圧を曲線を使うパッドの制御フラグ */
void GV_SetPressureChangeFlag( short flag )
{
	int i ;

	flag |= GV_PAD_ANALOG_CHANGE ;
	for( i = 0; i < GV_PAD_MAX; i++ ){
		GV_PadData[i].analog_input |= flag ;
	}
}
void GV_ResetPressureChangeFlag( short flag )
{
	int i ;

	for( i = 0; i < GV_PAD_MAX; i++ ){
		GV_PadData[i].analog_input &= ~flag ;
	}
}
void GV_InitPressureChangeFlag()
{
	int i ;

	for( i = 0; i < GV_PAD_MAX; i++ ){
		GV_PadData[i].analog_input &= ~GV_PAD_ANALOG_ALLCHANGE ;
	}
}


void GV_SetChangePressure( int press_key, int type )
{
	GV_SetPressureChangeFlag( 1<<press_key ) ;
	SetChangePressure( press_key, type ) ;
}


/* シナリオパッド押し */
void	GV_PadPressScn( int which, int press )
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadData[ which ].flag |= GV_PAD_PRESS_SCN ;
    GV_PadPress[ which ] = press ;
}

/* マスク等解除 */
void	GV_PadMaskCancelScn( int which, int mode )
{
    ASSERT( which >= 0 && which < 4 ) ;
    if ( mode == 0 ) mode = 3 ;
    if ( mode & 1 ) {
		/* リリース解除 */
		GV_PadData[ which ].flag &= ~GV_PAD_RELEASE_SCN ;
		/* 今だけ */
    }
    if ( mode & 2 ) {
		/* マスク解除 */
		GV_PadData[ which ].flag &= ~GV_PAD_MASK_SCN ;
		GV_PadMask[ which * 2 + 1 ] = 0xffffffff ;
    }
}

/*--------------------------------------------------------*/

/* パッドデモ */

/* パッドデモデータセット */
void	GV_PadSetDemoData( int which, int type, void *data )
{
	GV_PadData[ which ].flag |= GV_PAD_PAD_DEMO ;
	demo_pad_status = type + PAD_STATUS_CONNECT_PAD ;
#ifndef _WINDOWS
	memcpy( &demo_pad_data, data, sizeof( PADBUF_DATA ) ) ;
#else
	{
		DEMO_PADBUF_DATA	*demo_data ;

		demo_data = (DEMO_PADBUF_DATA *)data ;

		demo_pad_data.flag   = demo_data->flag ;
		demo_pad_data.id     = demo_data->id ;
		demo_pad_data.button = 0xffff0000
							| ((DWORD)demo_data->button[0] << 8)
							| (DWORD)demo_data->button[1] ;
		memcpy(&demo_pad_data.analog, &demo_data->analog,
				sizeof(demo_pad_data.analog)) ;
		memcpy(&demo_pad_data.pressure, &demo_data->pressure,
				sizeof(demo_pad_data.pressure)) ;
	}

#endif
	demo_pad_data.flag = PAD_DATA_NORMAL ;
}



/*--------------------------------------------------------*/

int	GV_GetXPadType( int port )
{	// （ゲーム側から見た）port 番号のパッドの種類を得る
#ifdef _WINDOWS
	return(0) ;
#else
	port = invportmap( port );
	if( xpad[ port ].hDevice == NULL ) {
		// ささってない
		return 0;
	} else {
		return xpad[ port ].caps.SubType;
	}
#endif
}

int GV_GetXPadPort( int port ) {
#ifdef _WINDOWS
	return(0) ;
#else
	// ゲーム側から見た port を実際のポートに変換
	return invportmap( port );
#endif
}

//#ifndef _XBOX		// 元はここにありましたが、Windows版の為、ちょっと下に下げました

/*--------------------------------------------------------*/
// 必要コールバック
static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
    HRESULT hr;
	WPAD_DEVICE				*dev ;
	LPDIRECTINPUTDEVICE8	pDev ;
	DWORD					coop_flag ;

	if( wpad.dev_num >= MAX_WPAD_DEVICE_NUM ){ return(DIENUM_STOP) ; }
	dev = &wpad.dev[wpad.dev_num] ;

	/*-- インターフェース取得 --*/
	hr = pDI->CreateDevice( pdidInstance->guidInstance, &dev->pDev, NULL) ;
	if( FAILED(hr) ){ return(DIENUM_CONTINUE) ; }	// 作成失敗

	pDev = dev->pDev ;

	/*-- パッド能力取得 --*/
	dev->caps.dwSize = sizeof(DIDEVCAPS) ;
	hr = pDev->GetCapabilities(&dev->caps) ;
	ASSERT( !FAILED(hr) ) ;


	/*-- 協調モード設定 --*/
	switch( dev->caps.dwDevType & 0xff )
	{
	  case DI8DEVTYPE_GAMEPAD :
	  case DI8DEVTYPE_JOYSTICK :
		coop_flag = DISCL_EXCLUSIVE | DISCL_FOREGROUND ;
		break ;

	  default :
		coop_flag = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND ;
		break ;
	}

	hr = pDev->SetCooperativeLevel(DG_hWnd, coop_flag) ;
	if( FAILED(hr) )
	{
		if( coop_flag & DISCL_EXCLUSIVE )
		{
			coop_flag &= ~DISCL_EXCLUSIVE ;
			coop_flag |= DISCL_NONEXCLUSIVE ;

			hr = pDev->SetCooperativeLevel(DG_hWnd, coop_flag) ;
		}

		ASSERT( !FAILED(hr) ) ;
	}

	/*-- 初期化 --*/
	switch( dev->caps.dwDevType & 0xff )
	{
	  case DI8DEVTYPE_GAMEPAD :
	  case DI8DEVTYPE_JOYSTICK :
		wpad.dev_gamectrl_num++ ;

		/*-- フォーマット設定 --*/
		hr = pDev->SetDataFormat(&c_dfDIJoystick) ;
		ASSERT( !FAILED(hr) ) ;

		/*-- アナログスティックの設定 --*/
		hr = pDev->EnumObjects(EnumAxesCallback, (VOID*)dev, DIDFT_AXIS) ;
		ASSERT( !FAILED(hr) ) ;

		/*-- Force Feedback --*/
		if( coop_flag & DISCL_EXCLUSIVE )
		{
			hr = pDev->EnumEffects(DIEnumEffectsCallback, (VOID*)dev, DIEFT_ALL) ;
			ASSERT( !FAILED(hr) ) ;
		}
		break ;

	  case DI8DEVTYPE_KEYBOARD :
		wpad.dev_keyboard_num++ ;

		/*-- フォーマット設定 --*/
		hr = pDev->SetDataFormat(&c_dfDIKeyboard) ;

#if	__KEYBOARD_USE_BUFFERING_DATA__
		/*-- バッファ設定 --*/
		{
			DIPROPDWORD	diprop ;

			ZeroMemory(&diprop, sizeof(diprop)) ;
			diprop.diph.dwSize       = sizeof(diprop) ;
			diprop.diph.dwHeaderSize = sizeof(diprop.diph) ;
			diprop.diph.dwObj        = 0 ;
			diprop.diph.dwHow        = DIPH_DEVICE ;
			diprop.dwData            = 1024 ;	// バッファサイズ

			hr = pDev->SetProperty(DIPROP_BUFFERSIZE, &diprop.diph) ;
		}
#endif
		break ;

	  case DI8DEVTYPE_MOUSE :
	  case DI8DEVTYPE_SCREENPOINTER :
		wpad.dev_pointer_num++ ;

		/*-- フォーマット設定 --*/
		hr = pDev->SetDataFormat(&c_dfDIMouse) ;
		break ;
	}

	/*-- Callback終了 --*/
	wpad.dev_num++ ;
	return(DIENUM_CONTINUE) ;	// 継続
}

static BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                VOID* pContext )
{
	WPAD_DEVICE *dev ;
    DIPROPRANGE diprg ;
	HRESULT	 	hr ;

	dev = (WPAD_DEVICE *)pContext;

	/*-- 軸入力の場合、入力レンジを設定する --*/
    if( pdidoi->dwType & DIDFT_AXIS )
    {
		diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		diprg.diph.dwHow        = DIPH_BYID; 
		diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
		diprg.lMin              = -128; 
		diprg.lMax              = +127; 
    
		// Set the range for the axis
		hr = dev->pDev->SetProperty(DIPROP_RANGE, &diprg.diph) ;
		if( FAILED(hr) ){ return DIENUM_STOP; }
	}

    /*-- 入力機能チェック --*/
#ifdef DEBUG_MODE
	{
		GUID		guidType ;
		guidType = pdidoi->guidType ;

		if(      guidType == GUID_Button  ){ printf("\tFind Button\n") ; }
		else if( guidType == GUID_Key     ){ printf("\tFind Key\n") ; }
		else if( guidType == GUID_POV     ){ printf("\tFind POV\n") ; }
		else if( guidType == GUID_XAxis   ){ printf("\tFind XAxis\n") ; }
		else if( guidType == GUID_YAxis   ){ printf("\tFind YAxis\n") ; }
		else if( guidType == GUID_ZAxis   ){ printf("\tFind ZAxis\n") ; }
		else if( guidType == GUID_RxAxis  ){ printf("\tFind RxAxis\n") ; }
		else if( guidType == GUID_RyAxis  ){ printf("\tFind RyAxis\n") ; }
		else if( guidType == GUID_RzAxis  ){ printf("\tFind RzAxis\n") ; }
		else if( guidType == GUID_Slider  ){ printf("\tFind Slider\n") ; }
		else if( guidType == GUID_POV     ){ printf("\tFind POVr\n") ; }
		else if( guidType == GUID_Unknown ){ printf("\tFind Unknow\n") ; }
		else { printf("\tFind 0x%d\n", pdidoi->guidType) ; }
	}
#endif
	/*-- 次 --*/
    return(DIENUM_CONTINUE) ;
}

static BOOL CALLBACK DIEnumEffectsCallback( LPCDIEFFECTINFO pdei, LPVOID pvRef)
{
	BOOL		ret ;
	WPAD_DEVICE	*dev ;
	DIEFFECT	diEffect ;
	DIENVELOPE	diEnvelope ;
	HRESULT		hr ;

	dev = (WPAD_DEVICE *)pvRef ;
	ret = DIENUM_CONTINUE ;

	/*-- Type分岐 ------------------------------------------------------*/

	switch( pdei->dwEffType & DIEFT_HARDWARE )
	{
	  case DIEFT_PERIODIC :			// 周期的フォース エフェクト
		{
			LONG	direction[2] = {0,0} ;
			DWORD	dwAxes[2] = { DIJOFS_X, DIJOFS_Y, } ;

			ZeroMemory(&dev->periodic, sizeof(DIPERIODIC)) ;
			dev->periodic.dwMagnitude	= DI_FFNOMINALMAX ;
			dev->periodic.lOffset		= 0 ;
			dev->periodic.dwPhase		= 0 ;
			dev->periodic.dwPeriod		= (DWORD)(0.05f * DI_SECONDS) ;

			ZeroMemory(&diEnvelope, sizeof(DIENVELOPE)) ;
			diEnvelope.dwSize         	= sizeof(DIENVELOPE) ;
			diEnvelope.dwAttackLevel	= 0 ;
			diEnvelope.dwAttackTime		= (DWORD)(0.1f * DI_SECONDS) ;
			diEnvelope.dwFadeLevel		= 0 ;
			diEnvelope.dwFadeTime		= (DWORD)(0.5f * DI_SECONDS) ;

			ZeroMemory(&diEffect, sizeof(DIEFFECT)) ;
			diEffect.dwSize				= sizeof(diEffect); 
			diEffect.dwFlags			= DIEFF_POLAR 
										| DIEFF_OBJECTOFFSETS ;	// 極座標
			diEffect.dwDuration			= INFINITE ; 			// 寿命は無限
			diEffect.dwSamplePeriod		= 0;					// サンプリング間隔デフォルト
			diEffect.dwGain				= DI_FFNOMINALMAX ;		// スケーリング無し 
			diEffect.dwTriggerButton	= DIEB_NOTRIGGER ;		// 関連トリガ無し
			diEffect.dwTriggerRepeatInterval = 1 ;
			diEffect.cAxes				= 2 ;
			diEffect.rgdwAxes			= dwAxes ; 
			diEffect.rglDirection		= direction ; 
			diEffect.lpEnvelope			= &diEnvelope ;
			diEffect.cbTypeSpecificParams  = sizeof(DIPERIODIC);
			diEffect.lpvTypeSpecificParams = &dev->periodic ; 
			diEffect.dwStartDelay		= 0 ;

			hr = dev->pDev->CreateEffect(pdei->guid, &diEffect, &dev->pEffect, NULL) ;
			if( FAILED(hr) ){ dbgErrMessPuts("", hr) ; }
			ASSERT( !FAILED(hr) ) ;
		}

		ret = DIENUM_STOP ;	// 終了
		break ;

	  case DIEFT_CONDITION :		// 条件
	  case DIEFT_RAMPFORCE :		// 傾斜フォース エフェクト
	  case DIEFT_CONSTANTFORCE  :	// コンスタント フォース エフェクト
	  case DIEFT_CUSTOMFORCE  :		// カスタム フォース エフェクト
		break ;
	}
	/*------------------------------------------------------------------*/

	return(ret) ;
}

void GV_ExitPad(void)
{
	int i;

	WPAD_DEVICE	*dev ;

	dev = wpad.dev ;
	for(i=MAX_WPAD_DEVICE_NUM; i>0; i--, dev++)
	{
		if( dev->pDev)
		{
			if( dev->pEffect )
			{
				dev->pEffect->Release() ;
				dev->pEffect = NULL ;
			}

			dev->pDev->Unacquire();
			dev->pDev->Release();
			dev->pDev = NULL;
		}
	}
	ZeroMemory(&wpad, sizeof(WPAD)) ;

	GV_ReleaseWPadConfig() ;

	if(pDI) pDI->Release();	
}


void	GV_SetWindowsInputConfigulationMode(BOOL subject)
{
	// 入力モードを設定(通常/主観)
	InputModeIsSubject = subject ;
	if( subject )
	{
		CurKeyboardCfg = &GV_KeyboardCfgS ;
		CurGV_PadCfg   = &GV_PadCfgS ;
		CurMouseEnable = X2W_GetAppConfig(X2WAPPCFG_MOUSE_ENABLE2) ;
	}
	else
	{
		CurKeyboardCfg = &GV_KeyboardCfg ;
		CurGV_PadCfg   = &GV_PadCfg ;
		CurMouseEnable = X2W_GetAppConfig(X2WAPPCFG_MOUSE_ENABLE) ;
	}
}

int GV_GetWindowsSubjectMove(void)
{
	return(0) ;
}

#ifdef __cplusplus
}
#endif
