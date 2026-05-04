//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   パッド入出力モジュール
   
   1999/03/25 K.Uehara
   $Id: pad.c,v 1.1.1.3 2002/11/19 11:42:45 Yoshizawa1 Exp $
*/


#if defined(BP_360)
#include <xtl.h>
#endif

#if defined(BP_WIN32)
#include <windows.h>
#include <xinput.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include <eekernel.h>
#include <eeregs.h>
//BP #include <libpad.h>

#if defined(BP_PS3)
#  include <cell/pad/libpad.h>      // USB Gamepad Library
#  include <cell/pad.h>
#else
#  if defined( BP_VITA )
#    define __int32_t_defined
#    define __uint32_t_defined
#    define __int16_t_defined
#    define __uint16_t_defined
#    define __int8_t_defined
#    define __uint8_t_defined
#  endif
#  include "ExtLibraries/libPad/include/libpad.h"
#  define LIBPAD_SUPPORTED 0
#endif

#include "mgs_type.h"
#include "libgv.h"
#include "util.h"

// Include TGS #define
#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

#include "BP_Debug.h"   //BP_INPUT - update X360 remap



#if BP_WIN32 || BP_360
extern int pad_xinput_get_data( int, CellPadData * );
extern DWORD pad_xinput_set_state( int, unsigned char, unsigned char );
#elif BP_VITA
extern void pad_vta_init();
extern int pad_vta_get_data(int, CellPadData*);
#endif


//BP
#define SCE_PADLup     (1<<12)
#define SCE_PADLdown   (1<<14)
#define SCE_PADLleft   (1<<15)
#define SCE_PADLright  (1<<13)
#define SCE_PADRup     (1<< 4)
#define SCE_PADRdown   (1<< 6)
#define SCE_PADRleft   (1<< 7)
#define SCE_PADRright  (1<< 5)
#define SCE_PADi       (1<< 9)
#define SCE_PADj       (1<<10)
#define SCE_PADk       (1<< 8)
#define SCE_PADl       (1<< 3)
#define SCE_PADm       (1<< 1)
#define SCE_PADn       (1<< 2)
#define SCE_PADo       (1<< 0)
#define SCE_PADh       (1<<11)
#define SCE_PADL1      SCE_PADn
#define SCE_PADL2      SCE_PADo
#define SCE_PADL3      SCE_PADi
#define SCE_PADR1      SCE_PADl
#define SCE_PADR2      SCE_PADm
#define SCE_PADR3      SCE_PADj
#define SCE_PADstart   SCE_PADh
#define SCE_PADselect  SCE_PADk

/* 1999/12/15 lib ver1.2の導入でlibpadを使用するように変更 */

#define HANGUP_KEY	( PAD_A  | PAD_L2 | PAD_R1 | PAD_R2 | PAD_SEL | PAD_STA )

#if defined(BP_VITA)
//Center coordinate for the analog stick.
#define NEUTRAL_ANALOG_STICK_POS 127

int GV_direction_pressed_with_threshold( GV_PAD *pad, int padDirection, unsigned char desiredAnalogDelta )
{
   ASSERT( padDirection == PAD_PRESS_U 
      || padDirection == PAD_PRESS_D
      || padDirection == PAD_PRESS_L
      || padDirection == PAD_PRESS_R );

   if( pad->pressure[padDirection] )
   {
      return 1;
   }

   if( padDirection == PAD_PRESS_D || padDirection == PAD_PRESS_R )
   {
      unsigned char delta = padDirection == PAD_PRESS_D ? pad->left_dy : pad->left_dx;
      return delta > NEUTRAL_ANALOG_STICK_POS + desiredAnalogDelta;
   }
   else
   {
      unsigned char delta = padDirection == PAD_PRESS_U ? pad->left_dy : pad->left_dx;
      return delta < NEUTRAL_ANALOG_STICK_POS - desiredAnalogDelta;;
   }
}
#endif

/* ---------------------------------------------------------------------- */
/*
   USB keyboard関連(デバッグ用)
*/

#ifdef USBKBD

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

/* ------------------------------------------- */
/*
   libpad system driver
*/

// SCE のパッドバッファを読むための構造体

#define PRESSURE_SIZE 12

typedef struct {
	unsigned char flag;
	unsigned char id;
	unsigned char button[ 2 ];
	struct {
		unsigned char data[ 4 ];
	} analog;
	struct {
		unsigned char data[ PRESSURE_SIZE ];
	} pressure;
} PADBUF_DATA;

// システムバッファ
// BP_PS2 u_long128 pad_dma_buf[ 2 ][ scePadDmaBufferMax ] __attribute__((aligned(64)));
unsigned char pad_data[ 2 ][ 32 ];

static char pad_prev_id[ 2 ] = { 0, 0 };
static int pad_status[ 2 ];
static int pad_status_next[ 2 ];

// パッドデモ用
static int 			demo_pad_status ;
static PADBUF_DATA	demo_pad_data ;

// パッドマスク等指定用
int		GV_PadMask[ 4 * 2 ] ; 	/* マスク指定用 */
int		GV_PadPress[ 4 ] ;	/* シナリオパッド押し用 */

enum {
	PAD_STATUS_NO_CONNECT,
	PAD_STATUS_SETUP_ANALOG,
	PAD_STATUS_SETUP_DS,		// scePadSetActAlign
	PAD_STATUS_SETUP_DS2,		// scePadSetPressMode
	PAD_STATUS_WAIT_COMMAND,

	PAD_STATUS_CONNECT_OTHER,		// OTHER PAD ( not support )
	PAD_STATUS_CONNECT_PAD,			// NORMAL PAD
	PAD_STATUS_CONNECT_AC,			// ANALOG CONTROLER
	PAD_STATUS_CONNECT_DS,			// DUAL SHOCK
	PAD_STATUS_CONNECT_DS2,			// DUAL SHOCK 2
};

#define IS_CONNECT( _state ) ( (_state) >= PAD_STATUS_CONNECT_OTHER )

#define IS_AVAILABLE(_state) ( (_state) > PAD_STATUS_CONNECT_OTHER )
#define PAD_TYPE( _state )	 ( (_state) - PAD_STATUS_CONNECT_PAD )

#define PAD_DATA_NORMAL			0x00	// データ取得可能
#define PAD_DATA_ERROR			0x01	// エラーなので,前回のデータをそのままつかう
#define PAD_DATA_RELEASE		0x02	// 通信中または抜けている
#define PAD_DATA_NOT_SUPPORT	0x03	// サポートされていないデバイス

#define PARAM_MAX		6

#if 0 //BP_PS2
static int init_expand_pad( int port )
{
	int actno;
	int i;
	static char param[ 2 ][ PARAM_MAX ];

	// アクチュエータ情報を読み込み 正しい値を設定する

		actno = scePadInfoAct( port, 0, -1, 0 );
	for( i = 0; i < PARAM_MAX; i++ ){
		param[ port ][ i ] = 0xff;
	}
	if( actno > PARAM_MAX ){
		actno = PARAM_MAX;
	}

	for( i = actno - 1; i >= 0; --i ){
		int funcno, subno, len;

		funcno = scePadInfoAct( port, 0, i, InfoActFunc );
		subno = scePadInfoAct( port, 0, i, InfoActSub );
		len = scePadInfoAct( port, 0, i, InfoActSize );
		if( funcno == 1 ){
			if( subno == 1 && len == 1 ){
				/* 低速回転、1バイトの振動子 */
				param[ port ][ 1 ] = i;
			} else if( subno == 2 && len == 0 ){
				/* 高速回転、1ビットの振動子 */
				param[ port ][ 0 ] = i;
			}
		}
	}
	return scePadSetActAlign( port, 0, param[ port ] );
}
#endif

static void pad_init( void )
{
	PADBUF_DATA *p;

#if 1 //BP_PS2
   int i;

#if LIBPAD_SUPPORTED
   cellPadInit(2);

   for( i = 0; i < 2; i++ )
   {
      cellPadSetPortSetting( i, CELL_PAD_SETTING_PRESS_ON );

      // BP - Setup the read buffer to be empty and analog sticks in the middle
      // This is for the case where we get a connection attempt, but then no
      // data until there is stick movement on PS3.

      memset(pad_data[ i ], 0, sizeof(pad_data[ i ]));
      // Center analog controls
      pad_data[ i ][4] = 128;
      pad_data[ i ][5] = 128;
      pad_data[ i ][6] = 128;
      pad_data[ i ][7] = 128;
   }
#elif defined(BP_VITA)
   pad_vta_init();
#endif

#else
	scePadInit( 0 );
	scePadPortOpen( 0, 0, pad_dma_buf[ 0 ] );
	scePadPortOpen( 1, 0, pad_dma_buf[ 1 ] );
#endif

	pad_status[ 0 ] = pad_status[ 1 ] = PAD_STATUS_NO_CONNECT;

	p = ( PADBUF_DATA * )( &pad_data[ 0 ][ 0 ] );
	p->button[ 0 ] = p->button[ 1 ] = 0xFF;			// 全ボタンリリース
		p = ( PADBUF_DATA * )( &pad_data[ 1 ][ 0 ] );
	p->button[ 0 ] = p->button[ 1 ] = 0xFF;			// 全ボタンリリース
}

static unsigned char pad_vibration1[ 2 ];
static unsigned char pad_vibration2[ 2 ];

extern float gBP_VibrationLgMin;
extern float gBP_VibrationLgMax;
extern float gBP_VibrationLgThreshold;

static void pad_send_vibration( int port )
{
	static unsigned char prev_vib[ 2 ][ 2 ] = { { 0 } };
	static unsigned char vib_buf[ 6 ] = { 0 };

   int success = 1;//BP_INPUT - handle fail

	if( pad_vibration1[ port ] > 0 ){
		vib_buf[ 0 ] = 1;
		pad_vibration1[ port ]--;
	} else {
		vib_buf[ 0 ] = 0;
	}
	vib_buf[ 1 ] = pad_vibration2[ port ];
	pad_vibration2[ port ] = 0;
	if( vib_buf[ 0 ] != 0 || vib_buf[ 1 ] != 0
		|| prev_vib[ port ][ 0 ] != 0 || prev_vib[ port ][ 1 ] != 0 ){
#ifdef BP_PS3
         CellPadActParam actParam;

         const float vibrationLgRange = gBP_VibrationLgMax - gBP_VibrationLgMin;
         float fVibrationLg = (float)vib_buf[1] / 255;
         fVibrationLg = fVibrationLg > gBP_VibrationLgThreshold ? (gBP_VibrationLgMin + vibrationLgRange * fVibrationLg) : 0.0f;         // be sure to clear the actparams

         memset(&actParam, 0x00, sizeof(CellPadActParam));

         // motor[0] is the small (On or Off ) motor.
         actParam.motor[0] = vib_buf[0];
         // motor[1] is the larger (0=off, 1-255 = rotation speed ) motor.
         actParam.motor[1] = (unsigned char)(fVibrationLg*255.f);

         // handle the actual vibration
         //printf("VIB low %5d, high %5d\n", (int)actParam.motor[1], (int)actParam.motor[0] );
         success = (cellPadSetActDirect (port, &actParam) == CELL_PAD_OK);
#elif defined(BP_360)
         success = (pad_xinput_set_state( port, vib_buf[0], vib_buf[1] ) == ERROR_SUCCESS);
#elif 0 //BP_PS2
         scePadSetActDirect( port, 0, vib_buf );
#endif
	}
   if( success )//BP_INPUT - handle fail case so we send it again next update
   {
	prev_vib[ port ][ 0 ] = vib_buf[ 0 ];
	prev_vib[ port ][ 1 ] = vib_buf[ 1 ];
   }
}

static inline void pad_set_vibration1( int port, int value )
{
	pad_vibration1[ port ] = value;
}

static inline void pad_set_vibration2( int port, int value )
{
	if( value > 255 ) value = 255;
	pad_vibration2[ port ] = value;
}

#if !BP_360

#define DEBUG_CHECK_PAD_CAPS 0

static int pad_check_cell_pad_info( int const port_num, CellPadInfo2 const *pPadInfo )
{
   if ( pPadInfo->port_status[port_num] & CELL_PAD_STATUS_CONNECTED )
   {
      static const unsigned skCapsChecks = 
         CELL_PAD_CAPABILITY_PS3_CONFORMITY |
         CELL_PAD_CAPABILITY_PRESS_MODE;

      if ( ( pPadInfo->device_capability[port_num] & skCapsChecks ) == skCapsChecks )
      {
         if ( pPadInfo->device_type[port_num] == CELL_PAD_DEV_TYPE_STANDARD )
         {
            return 1;
         }
#if DEBUG_CHECK_PAD_CAPS
         else
         {
            printf( "Port %d: Failed type check: %8.8x\n", port_num, pPadInfo->device_type[port_num] );
         }
#endif
      }
#if DEBUG_CHECK_PAD_CAPS
      else
      {
         printf( "Port %d: Failed caps checks: %8.8x\n", port_num, pPadInfo->device_capability[port_num] );
      }
#endif
   }

   return 0;
}

#endif

static void pad_read_act( void )
{
   int i;

#if 1 //BP_PS2

#if LIBPAD_SUPPORTED
   CellPadInfo2 padInfo;
   int ret = cellPadGetInfo2(&padInfo);
#else
   int ret = 0;
#endif

   if (ret)
   {
      printf ("ERROR: cellPadGetInfo () error %d\n", ret);
      return;
   }

	for( i = 0; i < 2; i++ )
   {
      CellPadData padData;

      int stat = PAD_STATUS_NO_CONNECT;
      int cell_stat = PAD_STATUS_NO_CONNECT;

#if LIBPAD_SUPPORTED
   if( pad_check_cell_pad_info( i, &padInfo ) )
   {
      // Support pressure sensitive buttons?
      if( padInfo.device_capability[i] & CELL_PAD_CAPABILITY_PRESS_MODE )
      {
         cell_stat = stat = PAD_STATUS_CONNECT_DS2;
      }
      else
      {
         cell_stat = stat = PAD_STATUS_CONNECT_DS;
      }
   }
#elif defined(BP_360) || defined(BP_WIN32)
      if( stat == PAD_STATUS_NO_CONNECT )
      {
         stat = pad_xinput_get_data(i, &padData) ? PAD_STATUS_CONNECT_DS2 : PAD_STATUS_NO_CONNECT;
      }
#elif defined (BP_VITA)
      if (stat == PAD_STATUS_NO_CONNECT)
         stat = pad_vta_get_data(i, &padData) ? PAD_STATUS_CONNECT_DS2 : PAD_STATUS_NO_CONNECT;
#else
#error Unsupported platform!
#endif

      pad_status[ i ] = stat;

      if( stat != PAD_STATUS_NO_CONNECT )
      {
         int n;
         int profile = 0;

#if LIBPAD_SUPPORTED
         if ( cell_stat != PAD_STATUS_NO_CONNECT )
         {
            ret = cellPadGetData(i, &padData);
         }
#else
         ret = 0;
#endif

#if BP_TGS_DEMO()
         // We want to disable the debug controller for the TGS demo, but still be able to use it.
         // We will disable all controller 2 input unless both L1+L2 are held.
         if ((i == 1) && 
            (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & (CELL_PAD_CTRL_L2 | CELL_PAD_CTRL_L1)) != (CELL_PAD_CTRL_L2 | CELL_PAD_CTRL_L1))
         {
            padData.len = 0;
         }
#endif

         if (ret)
         {
            printf ("ERROR: cellPadGetData (%d) error %d\n", i, ret);
            pad_status[ i ] = PAD_DATA_ERROR;
         }
         else
         {
            if (padData.len > CELL_PAD_BTN_OFFSET_PRESS_R2)
            {
               //Fill pad data with newest.
               short digitalState = 0;

               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_SELECT) ? SCE_PADselect : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3) ? SCE_PADL3 : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_R3) ? SCE_PADR3 : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_START) ? SCE_PADstart : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_UP) ? SCE_PADLup : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_RIGHT) ? SCE_PADLright : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_DOWN) ? SCE_PADLdown : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_LEFT) ? SCE_PADLleft : 0;

               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2) ? SCE_PADL2 : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) ? SCE_PADR2 : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L1) ? SCE_PADL1 : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1) ? SCE_PADR1 : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_TRIANGLE) ? SCE_PADRup : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE) ? SCE_PADRright : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CROSS) ? SCE_PADRdown : 0;
               digitalState |= (padData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_SQUARE) ? SCE_PADRleft : 0;

               digitalState = ~digitalState;

               memset( pad_data[ i ], 0, sizeof( pad_data[i] ) );

               //Fill digital info
               pad_data[ i ][3] = digitalState & 0xFF;
               pad_data[ i ][2] = digitalState >> 8;

               //Fill analog info
               pad_data[ i ][4] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X];
               pad_data[ i ][5] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y];
               pad_data[ i ][6] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X];
               pad_data[ i ][7] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y];

               //Fill pressure info
               pad_data[ i ][8] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_RIGHT];
               pad_data[ i ][9] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_LEFT];
               pad_data[ i ][10] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_UP];
               pad_data[ i ][11] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_DOWN];
               pad_data[ i ][12] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_TRIANGLE];
               pad_data[ i ][13] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_CIRCLE];
               pad_data[ i ][14] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_CROSS];
               pad_data[ i ][15] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_SQUARE];
               pad_data[ i ][16] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_L1];
               pad_data[ i ][17] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_R1];
               pad_data[ i ][18] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_L2];
               pad_data[ i ][19] = (unsigned char)padData.button[CELL_PAD_BTN_OFFSET_PRESS_R2];
            }
            //else no change since last.

            pad_send_vibration( i );
         }
      }
      else
      {
         //not connected.
      }
   }

#else
	for( i = 0; i < 2; i++ ){
		int state;

		state = scePadGetState( i, 0 );

		pad_data[ i ][ 0 ] = PAD_DATA_RELEASE;

		switch( state ){
		case scePadStateFindPad:
		case scePadStateDiscon:
			// ささっていない
				pad_status[ i ] = PAD_STATUS_NO_CONNECT;
		case scePadStateExecCmd:
			// 通信中
				continue;
		case scePadStateError:
			// 通信エラー
				pad_data[ i ][ 0 ] = PAD_DATA_ERROR;
			//printf( "Pad Error %d %d\n", i, GV_Time );
			continue;
		case scePadStateFindCTP1:
			// 通常パッド
				if( ! IS_CONNECT( pad_status[ i ] ) ){
					int id;
					id = scePadInfoMode( i, 0, InfoModeCurID, 0 );

					if( id == 0x04 ){
						printf( "Normal PAD\n" );
						pad_status[ i ] = PAD_STATUS_CONNECT_PAD;
					} else if( id == 0x07 ){
						printf( "ANALOG PAD\n" );
						pad_status[ i ] = PAD_STATUS_CONNECT_AC;
					} else if( id != 0 ){
						printf( "OTHER PAD %X\n", id );
						pad_status[ i ] = PAD_STATUS_CONNECT_OTHER;
					}
					pad_prev_id[ i ] = id;
				}
			break;
		case scePadStateStable:
			// 拡張パッド
				if( ! IS_CONNECT( pad_status[ i ] ) ){
					// 拡張パッドの初期化を行なう
						switch( pad_status[ i ] ){
						case PAD_STATUS_NO_CONNECT:
							// ANALOG切替えボタンのロック要求
								if( scePadSetMainMode( i, 0, 1, 3 ) == 1 ){
									pad_status[ i ] = PAD_STATUS_WAIT_COMMAND;
									pad_status_next[ i ] = PAD_STATUS_SETUP_ANALOG;
								}
							break;
						case PAD_STATUS_SETUP_ANALOG:
							{
								int id;

								id = scePadInfoMode( i, 0, InfoModeCurID, 0 );
								// ロックしたのにANALOGじゃなかったら,DUALSHOCKじゃない
									pad_prev_id[ i ] = id;
								if( id != 0x07 ){
									printf( "Other ExPad %X\n", id );
									pad_status[ i ] = PAD_STATUS_CONNECT_OTHER;
									break;
								}
								pad_status[ i ] = PAD_STATUS_SETUP_DS;
							}
							break;
						case PAD_STATUS_SETUP_DS:
							{
								int pressmode;
								pressmode = scePadInfoPressMode( i, 0 );
								if( pressmode == 1 ){
									/* 感圧対応 */
									if( scePadEnterPressMode( i, 0 ) == 1 ){
										printf( "DualShock2 Detected\n" );
										pad_status_next[ i ] = PAD_STATUS_SETUP_DS2;
										pad_status[ i ] = PAD_STATUS_WAIT_COMMAND;
									}
								} else {
									if( init_expand_pad( i ) == 1 ){
										pad_status[ i ] = PAD_STATUS_WAIT_COMMAND;
										pad_status_next[ i ] = PAD_STATUS_CONNECT_DS;
									}
								}
							}
							break;
						case PAD_STATUS_SETUP_DS2:
							{
								if( init_expand_pad( i ) == 1 ){
									pad_status[ i ] = PAD_STATUS_WAIT_COMMAND;
									pad_status_next[ i ] = PAD_STATUS_CONNECT_DS2;
								}
							}
							break;
						case PAD_STATUS_WAIT_COMMAND:
							{
								int result;
								result = scePadGetReqState( i, 0 );
								if( result == scePadReqStateFailed ){
									printf( "pad.c : ERROR\n" );
									pad_status[ i ] = PAD_STATUS_NO_CONNECT;
								} else if( result == scePadReqStateComplete ){
									pad_status[ i ] = pad_status_next[ i ];
									if( pad_status[ i ] == PAD_STATUS_CONNECT_DS ){
										printf( "DualShock Detected\n" );
									} else if( pad_status[ i ] == PAD_STATUS_CONNECT_DS2 ){
										printf( "DualShock2 Pressmode Detected\n" );
									}
								}
							}
							break;
						}
					continue;
				} else {
					if( pad_status[ i ] >= PAD_STATUS_CONNECT_DS ){
						pad_send_vibration( i );
					}
				}
			break;
		}
		if( scePadRead( i, 0, pad_data[ i ] ) == 0 ){
			pad_data[ i ][ 0 ] = PAD_DATA_ERROR;
		} else {
			if( pad_prev_id[ i ] != ( pad_data[ i ][ 1 ] >> 4 ) ){
				pad_status[ i ] = PAD_STATUS_NO_CONNECT;
			}
		}
	}
#endif
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
static	inline	int	GetPadDirAnalog( pad )
u_char			*pad ;
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
static	inline	int	GetPadDir( pad )
int			pad ;
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

static inline void setup_pressure( char *pressure, int button )
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
	return 0;
}
#endif

/*-----------------------------------------------------------------*/

void GV_InitPadSystem( void )
{
	/* システム初期化 */

	pad_init();
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
	u_char		*pressure ;
} UPDATEPAD ;

static	inline	void	SET_UPDATEPAD( UPDATEPAD *up, int type, int status, int dir, 
									   int anaval, int analog_flag, u_char *pressure )
{
	up->type = type ;
	up->status = status ;
	up->dir = dir ;
	up->anaval = anaval ;
	up->analog_flag = analog_flag ;
	up->pressure = pressure ;
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
	PAD_PRESS_R	,
	PAD_PRESS_L	,
	PAD_PRESS_U	,
	PAD_PRESS_D	,

	PAD_PRESS_X	,
	PAD_PRESS_A	,
	PAD_PRESS_B	,
	PAD_PRESS_Y	,

	PAD_PRESS_L1,
	PAD_PRESS_R1,
	PAD_PRESS_L2,
	PAD_PRESS_R2,

	12,13,
	14,15
} ;

/* 変えられているかどうかのフラグ PAD_??に準拠 */
u_int  key_config_flag ;

/* キーコンフィグをデフォルトにする */
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

#if 0
	if ( !(key_config_flag & pad->status ) ) {
		return ;
	}
#endif

	/*  変える前の情報を取っておく */
	status = pad->status ;
	memcpy( pressure, pad->pressure, sizeof(pressure) ) ;

	pad->status &= 0xf000 ;
	for( i=0 ; i<12 ; i++ ) {
		if ( status & botton2status_table[i] ) {
			pad->status |= botton2status_table[(int)key_config_table[i]] ;
			pad->pressure[(int)key_config_table[i]] = pressure[i] ;
		} else {
			pad->pressure[(int)key_config_table[i]] = 0 ;
		}
	}
	for(     ; i<16 ; i++ ) {
		if ( status & botton2status_table[i] ) {
			pad->status |= botton2status_table[(int)key_config_table[i]] ;
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
			( p->analog.data[ 1 ] < 128 - ANALOG_MARGIN ) ) {
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
	SET_UPDATEPAD( up, type, stat, dir, ana_value, ana_flag, p->pressure.data ) ;
}

void GV_UpdatePadSystem( void )
{
	int 			i ;
	UPDATEPAD		updatepad, *up ;

	up = &updatepad ;
	pad_read_act();

	/* とりあえずパッド情報を更新 */

	/* 今は2こだけ */
	for( i = 0; i < 2; i++ ){
		int 			type ;
		u_char			pressure[ PRESSURE_SIZE ] ;
		PADBUF_DATA 	*p ;
		GV_PAD			*pad ;

		p = ( PADBUF_DATA * )( &pad_data[ i ][ 0 ] );
		type = pad_status[ i ];

		/* DIRECT のアップデート */
		pad = &GV_PadDataDirect[ i ] ;
		SetPadState( up, pad, p, type ) ;
		UpdatePad( pad, up ) ;

		/* NORMAL データのアップデート */
		pad = &GV_PadData[ i ] ;
		if ( pad->flag & GV_PAD_PAD_DEMO ) {
			/* パッドデモ */
			p = &demo_pad_data ;
			type = demo_pad_status ;
			SetPadState( up, pad, p, type ) ;
			pad->flag &= ~GV_PAD_PAD_DEMO ;
		} else {
			memcpy( pressure, p->pressure.data, PRESSURE_SIZE ) ;  //仮
			up->pressure = pressure ;
			/* リリース、マスク等の処理 */
			if ( pad->flag & GV_PAD_RELEASE ) {
				up->status = 0 ;
				up->anaval = (128U<<24)|(128U<<16)|(128U<<8)|(128U<<0) ;
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
		}
		/* キーコンフィグ処理 */
		UpdatePad( pad, up ) ;
		pad->flag &= ~GV_PAD_PRESS_SCN ;
	}

#ifdef USBKBD
	update_key();
#endif

   BP_UpdatePlayerPadRemap(); //BP_INPUT - update X360 remap

   // Process debug controller input
   BP_Debug_PushCPUMarker( "BP_ProcessDebugInput" );
   {
      extern void BP_ProcessDebugInput();
      BP_ProcessDebugInput();
   }
   BP_Debug_PopCPUMarker();
}

void	GV_OriginPadSystem( org )
int	org ;
{
    PadOrg = org ;
}

int	GV_GetPadOrigin( void )
{
    return PadOrg ;
}

void GV_SetPadVibration1( int which, int value )
{
	pad_set_vibration1( which, value );
}

void GV_SetPadVibration2( int which, int value )
{
	pad_set_vibration2( which, value );
}

/*--------------------------------------------------------*/

/* パッドリリース操作関係 */

/* パッドリリース */
static	inline	void	PadReleaseOn( which, flag )
int		which, flag ;
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadData[ which ].flag |= flag ;
}

static	inline	void	PadReleaseOff( which, flag )
int		which, flag ;
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadData[ which ].flag &= ~flag ;
}

void	GV_PadReleaseOnSystem( which ) 
int		which ;
{
	PadReleaseOn( which, GV_PAD_RELEASE_SYSTEM ) ;
}

void	GV_PadReleaseOffSystem( which ) 
int		which ;
{
	PadReleaseOff( which, GV_PAD_RELEASE_SYSTEM ) ;
}

void	GV_PadReleaseOn( which )
int	which ;
{
	PadReleaseOn( which, GV_PAD_RELEASE_PRG ) ;
}

void	GV_PadReleaseOff( which ) 
int	which ;
{
	PadReleaseOff( which, GV_PAD_RELEASE_PRG ) ;
}

void	GV_PadReleaseOnScn( which )
int	which ;
{
	PadReleaseOn( which, GV_PAD_RELEASE_SCN ) ;
}

/* パッドマスク */
void	GV_PadMaskOn( which, mask )
int	which ;
int	mask ;
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadMask[ which * 2 ] = ~GV_PadGetStatus( mask ) ;
    GV_PadData[ which ].flag |= GV_PAD_MASK_PRG ;
}

void	GV_PadMaskOff( which )
int	which ;
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadMask[ which * 2 ] = 0xffffffff ;
    GV_PadData[ which ].flag &= ~GV_PAD_MASK_PRG ;    
}

void	GV_PadMaskOnScn( int which, int mask )
{
    ASSERT( which >= 0 && which < 4 ) ;
    ASSERT( mask != 0 ) ;
    GV_PadMask[ which * 2 + 1 ] = ~GV_PadGetStatus( mask ) ;
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
	for( i = 0; i < GV_PAD_MAX ; i++ ){
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


/* シナリオパッド押し */
void	GV_PadPressScn( which, press )
int	which ;
int	press ;
{
    ASSERT( which >= 0 && which < 4 ) ;
    GV_PadData[ which ].flag |= GV_PAD_PRESS_SCN ;
    GV_PadPress[ which ] = GV_PadGetStatus( press ) ;
}

/* マスク等解除 */
void	GV_PadMaskCancelScn( which, mode )
int	which ;
int	mode ;
{
    ASSERT( which >= 0 && which < 4 ) ;
    if ( mode == 0 ) mode = 3 ;
    if ( mode & 1 ) {
		/* リリース解除 */
		GV_PadData[ which ].flag &= ~GV_PAD_RELEASE_SCN ;
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

void GV_ExitPad()
{
   BP_TODO_BREAK;
}


int GV_PadControlDisabled(int control)
{
   int controlDisabledProgrammatically = 0;
   int controlDisabledThroughScripting = 0;
   int allControlsDisabled = GV_PadData[0].flag & (GV_PAD_RELEASE_PRG|GV_PAD_RELEASE_SCN|GV_PAD_RELEASE_SYSTEM);
   if (!allControlsDisabled)
   {
      int controlMask = GV_PadGetStatus(control);
      controlDisabledProgrammatically = (GV_PadData[0].flag & GV_PAD_MASK_PRG) && !(GV_PadMask[0] & controlMask);
      controlDisabledThroughScripting = (GV_PadData[0].flag & GV_PAD_MASK_SCN) && !(GV_PadMask[1] & controlMask);
   }

   return allControlsDisabled + controlDisabledProgrammatically + controlDisabledThroughScripting;
}
