/*
   パッド入出力モジュール
   
   1999/03/25 K.Uehara
   $Id: pad.cpp,v 1.14 2002/11/23 11:47:45 Yoshizawa1 Exp $
*/

#if 1//def KP_XBOX //BP

#include <stdio.h>

#if 1//def KP_XBOX
#ifdef KP_WINDOWS

//#define	INITGUID
#include <xtl.h>
#include <dinput.h>
#else

#include <xtl.h>

#endif
#else

#include <dinput.h>

#endif

#include "libgv.h"
#include "util.h"


#ifdef __cplusplus
extern "C" {
#endif



#ifndef KP_WINDOWS
const int PAD_NUM = 4;			// パッド処理数
#else
const int PAD_NUM = 1;			// パッド処理数
#endif
const int MU_NUM = 8;			// メモリユニット処理数

/* 1999/12/15 lib ver1.2の導入でlibpadを使用するように変更 */

#define HANGUP_KEY	( PAD_A  | PAD_L2 | PAD_R1 | PAD_R2 | PAD_SEL | PAD_STA )

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

#define PRESSURE_SIZE 12

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

#if 0 //BP

#ifndef KP_WINDOWS	// (#ifdef KP_XBOX)

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

typedef struct {
	LPDIRECTINPUTDEVICE8	pJoystick;	// デバイスポインタ
	int pad_status;				// 状態
	DIDEVCAPS	caps;			// パッドの能力変数
	int		s32LMotor;			// 振動
	int		s32RMotor;			// 振動
	PADBUF_DATA	pad_data;
} XPAD;

static LPDIRECTINPUT8	pDI	= NULL;
extern HWND DG_hWnd;

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
											VOID* pContext );
static BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
									   VOID* pContext );
#endif

static XPAD xpad[ PAD_NUM ];

#endif //BP

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
	u_short	u16ButtonFlag;	// PS2のボタンフラグ
	short	s16PressOrder;	// PS2の感圧番号
} EMTABLE;

#ifndef KP_WINDOWS	// (#ifdef KP_XBOX)
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
	{PAD_Y, PAD_PRESS_Y},
	{PAD_X, PAD_PRESS_X},
	{PAD_AL, -1},
	{PAD_B, PAD_PRESS_B},
	{PAD_A, PAD_PRESS_A},
	{PAD_AR, -1},
	{PAD_L1,PAD_PRESS_L1},
	{PAD_R1,PAD_PRESS_R1},
	{PAD_L2,PAD_PRESS_L2},
	{PAD_R2,PAD_PRESS_R2},
	{PAD_STA, -1},
	{PAD_SEL, -1},
};

static u_short tabDir[] = {
	PAD_U, PAD_U|PAD_R, PAD_R, PAD_R|PAD_D,
	PAD_D, PAD_D|PAD_L, PAD_L, PAD_L|PAD_U,
};

#endif


#define IS_CONNECT( _state ) ( (_state) >= PAD_STATUS_CONNECT_OTHER )

#define IS_AVAILABLE(_state) ( (_state) > PAD_STATUS_CONNECT_OTHER )
#define PAD_TYPE( _state )	 ( (_state) - PAD_STATUS_CONNECT_PAD )

#define PAD_DATA_NORMAL			0x00	// データ取得可能
#define PAD_DATA_ERROR			0x01	// エラーなので,前回のデータをそのままつかう
#define PAD_DATA_RELEASE		0x02	// 通信中または抜けている
#define PAD_DATA_NOT_SUPPORT	0x03	// サポートされていないデバイス

#define PARAM_MAX		6

static int pad_read_act( void )
{	// パッドデータを読み込んでエミュレーション

   BP_TED_BREAK;
#if 0 //BP_TODO

#ifndef KP_WINDOWS	//(#ifdef KP_XBOX)
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
			u_short u16State = 0xffff;
			WORD	wButtons = xiState.Gamepad.wButtons;
			for(j = 0; j < sizeof(tabButton)/sizeof(EMTABLE) ; j++){
				if(wButtons & 1){
					// 該当ボタンが押されている
					u16State &= ~tabButton[j].u16ButtonFlag;
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
					u16State &= ~tabPress[j].u16ButtonFlag;
				}
			}
			pXpad->pad_data.button[0] = (u16State >> 8) & 0xff;
			pXpad->pad_data.button[1] = (u16State >> 0) & 0xff;
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
	XPAD* pXpad;
	int i,j;
	for( i = 0, pXpad = &xpad[0]; i < PAD_NUM; i++, pXpad++ ){

		if(pXpad->pJoystick == NULL) continue;

		HRESULT     hr;
		DIJOYSTATE	js;
		hr = pXpad->pJoystick->Poll(); 
		if( FAILED(hr) ){
			// DInput is telling us that the input stream has been
			// interrupted. We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done. We
			// just re-acquire and try again.
			hr = pXpad->pJoystick->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = pXpad->pJoystick->Acquire();
			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later 
			continue;
		}

		// Get the input's device state
		if( FAILED( hr = pXpad->pJoystick->GetDeviceState( sizeof(DIJOYSTATE), &js ) ) )
			continue ; // The device should have been acquired during the Poll()

		// スティックのアナログ情報を移動
		pXpad->pad_data.analog.data[0] = js.lZ + 128;
		pXpad->pad_data.analog.data[1] = js.lRz + 128;
		pXpad->pad_data.analog.data[2] = js.lX + 128;
		pXpad->pad_data.analog.data[3] = js.lY + 128;

		// 感圧情報を移動
		u_short u16State = 0;
		for( j = 0; j < sizeof(tabButton)/sizeof(EMTABLE); j++ ){
#if FALSE
			if(js.rgbButtons[j] & 0x80){
				u16State |= tabButton[j].u16ButtonFlag;
			}
			if(tabButton[j].s16PressOrder >= 0){
				pXpad->pad_data.pressure.data[tabButton[j].s16PressOrder] = js.rgbButtons[j];
			}
#else
			if(js.rgbButtons[j]){
				u16State |= tabButton[j].u16ButtonFlag;
				if(tabButton[j].s16PressOrder >= 0){
					pXpad->pad_data.pressure.data[tabButton[j].s16PressOrder] = 0xff;
				}
			}
#endif
		}

		// ボタンエミュレーション
		// 方向をボタンに
		if(js.rgdwPOV[0] >= 0){
			int dir = js.rgdwPOV[0] / 4500;
			if(dir < sizeof(tabDir) / sizeof(u_short))
				u16State |= tabDir[dir];
		}
		pXpad->pad_data.pressure.data[PAD_PRESS_U] = (u16State & PAD_U) ? 0xff : 0;
		pXpad->pad_data.pressure.data[PAD_PRESS_D] = (u16State & PAD_D) ? 0xff : 0;
		pXpad->pad_data.pressure.data[PAD_PRESS_L] = (u16State & PAD_L) ? 0xff : 0;
		pXpad->pad_data.pressure.data[PAD_PRESS_R] = (u16State & PAD_R) ? 0xff : 0;

		pXpad->pad_data.button[0] = (~u16State >> 8) & 0xff;
		pXpad->pad_data.button[1] = (~u16State >> 0) & 0xff;
		
		// その他雑多な情報を設定
		pXpad->pad_data.flag = 0;	// 通信成功
		pXpad->pad_data.id = 0x79;	// DS2 を示す
	}
	
#endif	

#endif //BP

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

   BP_TED_BREAK;
#if 0 //BP_TODO

#ifndef KP_WINDOWS
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
#else
	for(i = 0; i < PAD_NUM; i++){
		// パッドデータの初期化
		GV_ZeroMemory(&xpad[i], sizeof(XPAD));
		xpad[i].pJoystick = NULL;
		xpad[i].pad_data.button[0] = 0xff;	// 全ボタンリリース
		xpad[i].pad_data.button[1] = 0xff;	// 全ボタンリリース
		xpad[i].s32LMotor = 0;
		xpad[i].s32RMotor = 0;
		xpad[i].pad_status = PAD_STATUS_NO_CONNECT;
		portmap[ i ] = i;	// ポートマップ初期化
	}
	// DirectInput8 オブジェクト作成
	HRESULT hr;
	if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION,
										 IID_IDirectInput8, (VOID**)&pDI, NULL))){
		ASSERT(0);
	}
	// Joystick を探す
    if( FAILED( hr = pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, 
									   EnumJoysticksCallback,
									   NULL, DIEDFL_ATTACHEDONLY ))){
		ASSERT(0);
	}

	for(i = 0; i < PAD_NUM; i++){
		if(xpad[i].pJoystick != NULL){
			printf("pad %d found\n", i);
			// フォーマット設定
			if( FAILED( hr = xpad[i].pJoystick->SetDataFormat( &c_dfDIJoystick ))){
				ASSERT(0);
			}
			if( FAILED( hr = xpad[i].pJoystick->SetCooperativeLevel( DG_hWnd, DISCL_EXCLUSIVE | 
															   DISCL_FOREGROUND ))){
				ASSERT(0);
			}
			// パッドの能力を得る
			xpad[i].caps.dwSize = sizeof(DIDEVCAPS);
			if ( FAILED( hr = xpad[i].pJoystick->GetCapabilities(&xpad[i].caps))){
				ASSERT(0);
			}
			// アナログスティックの設定
			if ( FAILED( hr = xpad[i].pJoystick->EnumObjects( EnumAxesCallback, 
															  (VOID*)&xpad[i], DIDFT_AXIS ))){
				ASSERT(0);
			}
			xpad[i].pad_status = PAD_STATUS_CONNECT_DS2;	// Dualshock2 相当
		}
	}
#endif	// KP_WINDOWS
	GV_PadSetDefaultKeyConf() ;

#endif //BP
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
	PAD_AL, PAD_AR, PAD_STA,PAD_SEL
} ;
/* キーコンフィグデフォルト値 */
static char key_config_table_default[] = {
	PAD_PRESS_R	,	PAD_PRESS_L	,	PAD_PRESS_U	,	PAD_PRESS_D	,
	PAD_PRESS_X	,	PAD_PRESS_A	,	PAD_PRESS_B	,	PAD_PRESS_Y	,
	PAD_PRESS_L1,	PAD_PRESS_R1,	PAD_PRESS_L2,	PAD_PRESS_R2,

	12,13,	14,15
} ;

/* 変えられているかどうかのフラグ PAD_??に準拠 */
u_int  key_config_flag ;

void GV_PadSetDefaultKeyConf( void )
{
	key_config_flag = 0 ;
	memcpy( key_config_table, key_config_table_default, 16*sizeof(char) ) ;
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
	int status ;
	u_char pressure[12] ;
	int  i ;

#if 0 /* なんか危険な予感なので取っておく */
	if ( !(key_config_flag & pad->status ) ) {
		return ;
	}
#endif

	/*  変える前の情報を取っておく */
	status = pad->status ;
	memcpy( pressure, pad->pressure, sizeof(pressure) ) ;

	/* キーコンフィグ変更処理 */
	pad->status &= 0xf000 ;
	for( i=0 ; i<12 ; i++ ) {
		if ( status & botton2status_table[i] ) {
			pad->status |= botton2status_table[key_config_table[i]] ;
			pad->pressure[key_config_table[i]] = pressure[i] ;
		} else {
			pad->pressure[key_config_table[i]] = 0 ;
		}
	}
	for(     ; i<16 ; i++ ) {
		if ( status & botton2status_table[i] ) {
			pad->status |= botton2status_table[key_config_table[i]] ;
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
	pad->analog_input &= ~GV_PAD_ANALOG_INPUT ;
	pad->analog_input |= up->analog_flag & GV_PAD_ANALOG_INPUT ;
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
		status = ( ( p->button[ 0 ] << 8 ) | ( p->button[ 1 ] ) ) ^ 0xffff ;
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





/* パッドシステム */
/*-----------------------------------------------------------------*/
/* gamed.cから呼ばれるシステム関数 */

void GV_UpdatePadSystem( void )
{
   BP_TED_BREAK;
#if 0 //BP_TODO

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
#ifdef KP_WINDOWS
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
				up->anaval = (128U<<24)|(128U<<16)|(128U<<8)|(128U<<0) ;
				/* シナリオプレス */
				/* リリース状態でも効くようにしてみた。(2001/08/06) */
				if ( pad->flag & GV_PAD_PRESS_SCN ) {
					up->status |= GV_PadPress[ mapid ] ;
					//up->type = PAD_STATUS_CONNECT_DS;
				} else {
					up->type = PAD_STATUS_CONNECT_DS;
				}
			} else {
				/* パッドマスク */
				if ( pad->flag & GV_PAD_MASK ) {
					if ( pad->flag & GV_PAD_MASK_PRG ){
						up->status &= GV_PadMask[ mapid * 2 ] ;
					}
					if ( pad->flag & GV_PAD_MASK_SCN ){
						up->status &= GV_PadMask[ mapid * 2 + 1 ] ;
					}
					//up->type = PAD_STATUS_CONNECT_DS;
				}
				/* シナリオプレス */
				if ( pad->flag & GV_PAD_PRESS_SCN ) {
					up->status |= GV_PadPress[ mapid ] ;
					//up->type = PAD_STATUS_CONNECT_DS;
				}
			}
			/* ＸＢＯＸゲーム用に感圧値を変更 */
			ChangePressure( pad, up );

			/* キーコンフィグ処理 */
			UpdatePad( pad, up ) ;
		}
		pad->flag &= ~GV_PAD_PRESS_SCN ;
	}

#ifdef USBKBD
	update_key();
#endif
	
#if 1	// AREA_EU_BP_IGNORE()
	if( GV_PadDataDirect[ 0 ].status == 0 && GV_PadDataDirect[ 0 ].release == 0) {
		// ポート２−４のパッドが押されていれば
		// それをパッド１に交換
#ifdef DEBUG_MODE	// デバッグモード時はPAD2は無視
#define START_PAD	2		
#else
#define START_PAD	1		
#endif		
		for( i = START_PAD; i < PAD_NUM; ++i ) {
			if( !(GV_PadDataDirect[ i ].flag & GV_PAD_DISCONNECT)
				&& GV_PadDataDirect[ i ].status != 0 ) {
				GV_ChangePadPortMapping( i, 0 );
				break;
			}
		}
	}
#endif

#endif //BP
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
	ASSERT(0 <= port && port < PAD_NUM);
	// ポートマップによって設定
	// （ここは逆引きになる）

   BP_TED_BREAK;

#if 0 //BP_TODO
#ifndef KP_WINDOWS
	xpad[ invportmap( port ) ].s32RMotor = (value << 14) & 0xffff;
#endif

#endif
}

void GV_SetPadVibration2( int port, int value )
{	// もともと１バイト
	ASSERT(0 <= port && port < PAD_NUM);
	// ポートマップによって設定
	// （ここは逆引きになる）
   BP_TED_BREAK;

#if 0 //BP_TODO
#ifndef KP_WINDOWS
	xpad[ invportmap( port ) ].s32LMotor = (value << 8) & 0xffff;
#endif

#endif
}

// XBOXTCR対策用 ポートマップ
void GV_ChangePadPortMapping( int prevport, int newport )
{	// ゲーム側から見て prevport だったポートを
	// newport と入れ替える

	GV_PAD padtmp;
	int i;
	int flag;

	if( newport == prevport ) return;
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
	memcpy( &demo_pad_data, data, sizeof( PADBUF_DATA ) ) ;
	demo_pad_data.flag = PAD_DATA_NORMAL ;
}



/*--------------------------------------------------------*/

int	GV_GetXPadType( int port )
{	// （ゲーム側から見た）port 番号のパッドの種類を得る
   BP_TED_BREAK;
#if 0//BP_TODO
#ifdef KP_WINDOWS
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

#else
   return 0;
#endif
}

int GV_GetXPadPort( int port ) {
	// ゲーム側から見た port を実際のポートに変換
	return invportmap( port );
}

//#ifndef KP_XBOX		// 元はここにありましたが、Windows版の為、ちょっと下に下げました

/*--------------------------------------------------------*/
// 必要コールバック

#if 0 //BP

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
    HRESULT hr;

	int i;
	for(i = 0; i < PAD_NUM; i++){
		if(xpad[i].pJoystick == NULL){	// パッドを作成してよい
			// インターフェース取得
			hr = pDI->CreateDevice( pdidInstance->guidInstance, &xpad[i].pJoystick, NULL );
			// If it failed, then we can't use this joystick. (Maybe the user unplugged
			// it while we were in the middle of enumerating it.)
			if( FAILED(hr) ) {
				xpad[i].pJoystick = NULL;
				dbgErrMessPuts("", hr) ;
			}
			return DIENUM_CONTINUE;
		}
	}
	// パッドが作成できない
	return DIENUM_STOP;
}

static BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                VOID* pContext )
{
	XPAD* pXpad = (XPAD*)pContext;

    DIPROPRANGE diprg; 
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYID; 
    diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
    diprg.lMin              = -128; 
    diprg.lMax              = +127; 
    
	// Set the range for the axis
	if( FAILED( pXpad->pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
		return DIENUM_STOP;
	
    return DIENUM_CONTINUE;
}

#endif //BP

void GV_ExitPad(void)
{
   BP_TED_BREAK;
#if 0//BP_TODO
	int i;
	for(i = 0; i < PAD_NUM; i++){
		if(xpad[i].pJoystick != NULL) {
			xpad[i].pJoystick->Unacquire();
			xpad[i].pJoystick->Release();
			xpad[i].pJoystick = NULL;
		}
	}
	if(pDI) pDI->Release();
#endif
}

#ifdef __cplusplus
}
#endif

#endif
