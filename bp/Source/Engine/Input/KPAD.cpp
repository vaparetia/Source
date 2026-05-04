#define RVL_ASSERT(x)   (x)
BOOL OSDisableInterrupts() { return FALSE; }
void OSEnableInterrupts() {};
void OSRestoreInterrupts(BOOL) {};


// device type
#define WPAD_DEV_GUITAR               17
// data format
#define WPAD_FMT_GUITAR               11

#define WPAD_DEV_TRAIN               16
// data format of Core + Train
#define WPAD_FMT_TRAIN               10    // buttons, train

#define WPAD_PRESS_UNITS            4
#define WPAD_DEV_BALANCE_CHECKER    3
#define WPAD_FMT_BALANCE_CHECKER    12      // buttons + balance checker


/*---------------------------------------------------------------------------*
  Project:     KPAD library version 2
  File:        KPAD.c
  Programmers: Keizo Ohta
               HIRATSU Daisuke
               Tojo Haruki
               Tetsuya Sasaki

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Project:     WPAD library
  File:        wpad.h
  Programmers: TOKUNAGA, Yasuhiro
               HIRATSU Daisuke
               FUKUDA Takahiro
               TOJO Haruki

  Copyright (C) 2005-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: wpad.h,v $
  Revision 1.72  2008/04/15 09:59:17  tojo
  Privated WPADSetDpdSensitivity.
  Added APIs to check using special extensions.

  Revision 1.71  2008/03/25 02:19:20  tojo
  Merged from SDK3.1 branch.

  Revision 1.70.2.3  2008/03/17 05:57:52  tojo
  Made the following functions public again.
   - WPADSaveConfig
   - WPADEnableMotor
   - WPADSetSpeakerVolume

  Revision 1.70.2.2  2008/03/17 05:18:16  tojo
  Added WPADResetAutoSleepTimeCount().

  Revision 1.70.2.1  2007/12/10 07:25:08  tojo
  Made the following functions privated.
   - WPADSaveConfig
   - WPADEnableMotor
   - WPADSetSpeakerVolume

  Revision 1.70  2007/07/10 08:10:28  tojo
  Removed WPAD_SYNC_EVT_BUSY.

  Revision 1.69  2007/05/03 08:22:24  tojo
  (none)

  Revision 1.68  2007/04/12 07:59:42  tojo
  (none)

  Revision 1.67  2007/04/11 00:59:29  tojo
  (none)

  Revision 1.66  2007/04/10 10:14:08  tojo
  (none)

  Revision 1.65  2007/04/02 09:54:56  tojo
  Added attaching/detaching dummy extension only for debug use.

  Revision 1.64  2007/03/19 12:48:10  tojo
  (none)

  Revision 1.63  2006/11/29 10:27:29  tojo
  Implemented WPADControlExtGimmick.

  Revision 1.62  2006/11/29 10:14:14  tojo
  (none)

  Revision 1.60  2006/10/20 01:56:44  tojo
  Added WPADGetDpdFormat().

  Revision 1.59  2006/10/18 09:55:07  tojo
  (none)

  Revision 1.58  2006/10/11 17:08:04  tojo
  Added busy state.

  Revision 1.57  2006/09/27 01:09:15  tojo
  Changed api.

  Revision 1.56  2006/09/23 06:01:23  tojo
  Changed API names.

  Revision 1.55  2006/09/22 02:49:22  tojo
  Added WPAD_ERR_CORRUPTED.

  Revision 1.54  2006/09/21 05:47:50  tojo
  Added WPAD_ERR_NOPERM and WPAD_ERR_BROKEN

  Revision 1.53  2006/09/15 13:52:03  tojo
  Added fast sync apis and check audio streaming can

  Revision 1.52  2006/09/06 03:12:34  tojo
  Added some macros.

  Revision 1.51  2006/09/06 02:40:45  tojo
  Implemented to control connectability.

  Revision 1.50  2006/09/04 01:54:20  tojo
  Added WPADSetDpdSensitivity.

  Revision 1.49  2006/09/02 02:06:50  tojo
  (none)

  Revision 1.48  2006/08/29 01:42:05  tojo
  Added WPAD_DEV_FUTURE

  Revision 1.47  2006/08/28 14:32:18  tojo
  Added WPAD_DEV_NOT_SUPPORTED

  Revision 1.46  2006/08/28 13:07:25  tojo
  Made WPADGetDpdSensitivity public.

  Revision 1.45  2006/08/16 10:55:28  mitu
  deleted WPAD_MEM_FACEDATA_LEN.

  Revision 1.44  2006/08/16 09:37:36  tojo
  (none)

  Revision 1.43  2006/08/15 01:00:08  tojo
  (none)

  Revision 1.42  2006/08/14 17:45:02  tojo
  (none)

  Revision 1.41  2006/08/14 16:58:05  tojo
  Modified WPADMEM API.

  Revision 1.40  2006/08/14 07:49:11  tojo
  (none)

  Revision 1.39  2006/08/11 10:19:23  yasumoto_yoshitaka
  surpress padding warning

  Revision 1.38  2006/08/11 07:53:55  tojo
  Added embedded memory apis.

  Revision 1.37  2006/08/10 01:18:42  tojo
  Added dpd full mode.
  Changed system config apis.

  Revision 1.36  2006/08/07 00:18:16  tojo
  (none)

  Revision 1.35  2006/08/03 13:09:34  tojo
  Moved some config api to privete apis.

  Revision 1.34  2006/08/03 11:54:45  tojo
  Changed clamp apis.

  Revision 1.33  2006/08/01 06:03:18  tojo
  (none)

  Revision 1.32  2006/07/20 01:18:12  tojo
  Added WPADIsMotorEnabled.

  Revision 1.31  2006/07/19 08:14:43  tojo
  Changed argument type uint8 to uint16 at WPADSetDisableChannel

  Revision 1.30  2006/07/18 10:04:49  tojo
  Added WPAD_STRM_INTERVAL
  Changed WPAD_DISABLED_WIFI_*

  Revision 1.29  2006/07/04 08:16:52  tojo
  Added speaker/config/motor functions

  Revision 1.28  2006/06/30 08:59:19  tojo
  Added WPADGetWorkMemorySize().

  Revision 1.27  2006/06/17 07:00:55  tojo
  (none)

  Revision 1.26  2006/06/17 05:19:13  tojo
  Added WPAD_ADD_LEN
  Removed WPAD_ACC_1G_VALUE, WPAD_FS_ACC_1G_VALUE

  Revision 1.25  2006/06/16 04:30:50  tojo
  Added WPAD_CLEAR_EVT_BUSY.
  Changed WPAD_CL_STICK_RESO.

  Revision 1.24  2006/06/15 07:04:03  tojo
  Changed API of WPADGetAccGravityUnit.

  Revision 1.22  2006/06/15 05:46:05  tojo
  (none)

  Revision 1.21  2006/06/15 05:20:33  tojo
  Added WPADGetAccGravityUnit.
  Removed WPADGetOrigin.
  Resumed WPADRecalibrate for backward compatibility.

  Revision 1.20  2006/06/14 11:31:14  tojo
  Added WPADGetRegisteredDevNum().

  Revision 1.19  2006/06/14 10:49:05  tojo
  Fixed typo : Extention -> Extension.
  Removed WPADIsEmulator, WPADRecalibrate.
  Added WPADIsSpeakerEnabled.

  Revision 1.18  2006/06/13 14:21:54  tojo
  Added some macros.

  Revision 1.17  2006/06/13 11:45:14  tojo
  Added delete all link keys handler.

  Revision 1.16  2006/06/13 06:29:55  tojo
  (none)

  Revision 1.15  2006/06/13 06:05:34  tojo
  Added
   WPADSetExtentionCallback
   WPADSetConnectCallback
   WPADIsEmulator

  Revision 1.14  2006/06/12 11:42:18  ekwon
  New API to query enable/disable state of DPD.

  Revision 1.13  2006/06/12 06:05:44  tojo
  (none)

  Revision 1.12  2006/06/12 00:07:39  tojo
  Changed some API
  Defined classic controller button mappings

  Revision 1.11  2006/06/05 11:47:43  tojo
  Defined WPADOrigin.
  Added WPADGetOrigin.

  Revision 1.10  2006/06/03 10:00:50  tojo
  Switched WPAD_BUTTON_C and WPAD_BUTTON_Z

  Revision 1.9  2006/06/01 08:53:36  tojo
  Changed the declaration of WPADSyncCallback

  Revision 1.8  2006/05/25 13:00:33  tojo
  wrapped wud api

  Revision 1.7  2006/05/25 12:21:04  tojo
  added WPADRegisterAllocator(),

  Revision 1.6  2006/05/23 05:48:26  tojo
  added WPAD_DPD_* and WPAD_SPEAKER_*

  Revision 1.5  2006/05/17 12:37:30  ekwon
  Added NearEmpty field to WPADInfo.

  Revision 1.4  2006/05/17 04:59:33  kawaset
  Added definitions for production version controllers. Use preprocessor macro WPADEMU for backwards compatibility.

  Revision 1.3  2006/02/07 11:44:09  yasuh-to
  Deleted structure for Classic Contoroller. CL is not release at SDK1.0.

 *---------------------------------------------------------------------------*/

#ifndef __WPAD_H__
#define __WPAD_H__

//#include <revolution/types.h>
//#include <revolution/os.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WPAD_CHAN0                    0
#define WPAD_CHAN1                    1
#define WPAD_CHAN2                    2
#define WPAD_CHAN3                    3

#define WPAD_MAX_CONTROLLERS          4

#define WPAD_MOTOR_STOP               0
#define WPAD_MOTOR_RUMBLE             1

#ifndef WPADEMU
// Under construction. These definitions are subject to change.

#define WPAD_BUTTON_LEFT              0x0001
#define WPAD_BUTTON_RIGHT             0x0002
#define WPAD_BUTTON_DOWN              0x0004
#define WPAD_BUTTON_UP                0x0008
#define WPAD_BUTTON_PLUS              0x0010
#define WPAD_BUTTON_2                 0x0100
#define WPAD_BUTTON_1                 0x0200
#define WPAD_BUTTON_B                 0x0400
#define WPAD_BUTTON_A                 0x0800
#define WPAD_BUTTON_MINUS             0x1000
#define WPAD_BUTTON_HOME              0x8000

#define WPAD_BUTTON_Z                 0x2000
#define WPAD_BUTTON_C                 0x4000

// backwards compatibility
#define WPAD_BUTTON_START             WPAD_BUTTON_PLUS
#define WPAD_BUTTON_SELECT            WPAD_BUTTON_MINUS
#define WPAD_BUTTON_SMALL_A           WPAD_BUTTON_1
#define WPAD_BUTTON_SMALL_B           WPAD_BUTTON_2

// for classic controller extension
#define WPAD_CL_BUTTON_UP             0x0001
#define WPAD_CL_BUTTON_LEFT           0x0002
#define WPAD_CL_TRIGGER_ZR            0x0004
#define WPAD_CL_BUTTON_X              0x0008
#define WPAD_CL_BUTTON_A              0x0010
#define WPAD_CL_BUTTON_Y              0x0020
#define WPAD_CL_BUTTON_B              0x0040
#define WPAD_CL_TRIGGER_ZL            0x0080
#define WPAD_CL_RESERVED              0x0100
#define WPAD_CL_TRIGGER_R             0x0200
#define WPAD_CL_BUTTON_PLUS           0x0400
#define WPAD_CL_BUTTON_HOME           0x0800
#define WPAD_CL_BUTTON_MINUS          0x1000
#define WPAD_CL_TRIGGER_L             0x2000
#define WPAD_CL_BUTTON_DOWN           0x4000
#define WPAD_CL_BUTTON_RIGHT          0x8000


#define WPAD_DPD_OFF                  0
#define WPAD_DPD_STD                  1
#define WPAD_DPD_EXP                  3
#define WPAD_DPD_FULL                 5

#define WPAD_DPD_MAX_OBJECTS          4
#define WPAD_DPD_ANGLE               42.0f  // viewing angle
#define WPAD_DPD_IMG_RESO_WX       1024     // DPD image resolution 
#define WPAD_DPD_IMG_RESO_WY        768

#define WPAD_SPEAKER_OFF              0
#define WPAD_SPEAKER_ON               1
#define WPAD_SPEAKER_MUTE             2
#define WPAD_SPEAKER_MUTE_OFF         3
#define WPAD_SPEAKER_PLAY             4
#define WPAD_SPEAKER_RESET            5

#define WPAD_EXTGMK_OFF               0
#define WPAD_EXTGMK_ON                1

#define WPAD_ACC_RESO              1024     // accelerometer resolution
#define WPAD_ACC_MAX                  3.4f  // valid gravity = 3.4G

#define WPAD_FS_STICK_RESO          256     // stick resolution of FreeStyle unit
#define WPAD_FS_ACC_RESO           1024     // accelerometer resolution of FreeStyle unit
#define WPAD_FS_ACC_MAX               2.1f  // valid gravity = 2.1G

#define WPAD_CL_STICK_RESO         1024
#define WPAD_CL_TRIGGER_RESO        256

// device type
#define WPAD_DEV_CORE                 0
#define WPAD_DEV_FREESTYLE            1
#define WPAD_DEV_CLASSIC              2
#define WPAD_DEV_FUTURE             251
#define WPAD_DEV_NOT_SUPPORTED      252
#define WPAD_DEV_NOT_FOUND          253
#define WPAD_DEV_UNKNOWN            255

// data format of Core
#define WPAD_FMT_CORE                 0     // only buttons
#define WPAD_FMT_CORE_ACC             1     // buttons, motion sensor
#define WPAD_FMT_CORE_ACC_DPD         2     // buttons, motion sensor, pointing
// data format of Core + Nunchaku
#define WPAD_FMT_FREESTYLE            3     // buttons, nunchaku
#define WPAD_FMT_FREESTYLE_ACC        4     // buttons, motion sensor, nunchaku
#define WPAD_FMT_FREESTYLE_ACC_DPD    5     // buttons, motion sensor, pointing, nunchaku
// data format of Core + Classic Controller
#define WPAD_FMT_CLASSIC              6     // buttons, classic
#define WPAD_FMT_CLASSIC_ACC          7     // buttons, motion sensor, classic
#define WPAD_FMT_CLASSIC_ACC_DPD      8     // buttons, motion sensor, pointing, classic
// data format of Core + DPD full mode
#define WPAD_FMT_CORE_ACC_DPD_FULL    9     // buttons, motion sensor, pointing

#define WPAD_STATE_DISABLED           0
#define WPAD_STATE_ENABLING           1
#define WPAD_STATE_ENABLED            2
#define WPAD_STATE_SETUP              3
#define WPAD_STATE_DISABLING          4

// disable channel to avoid WiFi interference.
#define WPAD_DISABLE_CLEAR            0x0000    // enable all range
#define WPAD_DISABLE_WIFI_CHAN_1      0x0001
#define WPAD_DISABLE_WIFI_CHAN_2      0x0002
#define WPAD_DISABLE_WIFI_CHAN_3      0x0004
#define WPAD_DISABLE_WIFI_CHAN_4      0x0008
#define WPAD_DISABLE_WIFI_CHAN_5      0x0010
#define WPAD_DISABLE_WIFI_CHAN_6      0x0020
#define WPAD_DISABLE_WIFI_CHAN_7      0x0040
#define WPAD_DISABLE_WIFI_CHAN_8      0x0080
#define WPAD_DISABLE_WIFI_CHAN_9      0x0100
#define WPAD_DISABLE_WIFI_CHAN_10     0x0200
#define WPAD_DISABLE_WIFI_CHAN_11     0x0400
#define WPAD_DISABLE_WIFI_CHAN_12     0x0800
#define WPAD_DISABLE_WIFI_CHAN_13     0x1000
#define WPAD_DISABLE_WIFI_CHAN_14     0x2000

// battery level
#define WPAD_BATTERY_LEVEL_CRITICAL   0
#define WPAD_BATTERY_LEVEL_LOW        1
#define WPAD_BATTERY_LEVEL_MEDIUM     2
#define WPAD_BATTERY_LEVEL_HIGH       3
#define WPAD_BATTERY_LEVEL_MAX        4

#define WPAD_LED_CHAN_1             0x1
#define WPAD_LED_CHAN_2             0x2
#define WPAD_LED_CHAN_3             0x4
#define WPAD_LED_CHAN_4             0x8

#define WPAD_SYNC_EVT_START           0
#define WPAD_SYNC_EVT_DONE            1

#define WPAD_CLEAR_EVT_BUSY          -1
#define WPAD_CLEAR_EVT_START          0
#define WPAD_CLEAR_EVT_DONE           1

#define WPAD_ADDR_LEN                 6

#define WPAD_STRM_INTERVAL            OSNanosecondsToTicks(6666667)

// clamp algorithm for stick
#define WPAD_STICK_CLAMP_OCTA_WITH_PLAY         0
#define WPAD_STICK_CLAMP_OCTA_WITHOUT_PLAY      1
#define WPAD_STICK_CLAMP_CIRCLE_WITH_PLAY       2
#define WPAD_STICK_CLAMP_CIRCLE_WITHOUT_PLAY    3
// clamp algorithm for trigger
#define WPAD_TRIGGER_FIXED_BASE       0
#define WPAD_TRIGGER_INDIVIDUAL_BASE  1
// clamp algorithm for accelerometer
#define WPAD_ACC_CLAMP_CUBE           0
#define WPAD_ACC_CLAMP_SPHERE         1

#define WPAD_SENSOR_BAR_POS_BOTTOM    0
#define WPAD_SENSOR_BAR_POS_TOP       1

#define WPAD_MEM_GAMEDATA_LEN         3888

#define WPAD_SYNC_TYPE_STD            0
#define WPAD_SYNC_TYPE_SMP            1

#else
// SI-based WPAD

#define WPAD_BUTTON_HOME              0x0001
#define WPAD_BUTTON_SELECT            0x0002
#define WPAD_BUTTON_A                 0x0004
#define WPAD_BUTTON_B                 0x0008
#define WPAD_BUTTON_SMALL_A           0x0010
#define WPAD_BUTTON_SMALL_B           0x0020
#define WPAD_BUTTON_START             0x0100
#define WPAD_BUTTON_UP                0x0200
#define WPAD_BUTTON_DOWN              0x0400
#define WPAD_BUTTON_RIGHT             0x0800
#define WPAD_BUTTON_LEFT              0x1000
#define WPAD_BUTTON_Z1                0x2000
#define WPAD_BUTTON_Z2                0x4000
#define WPAD_BUTTON_X                 WPAD_BUTTON_SMALL_A
#define WPAD_BUTTON_Y                 WPAD_BUTTON_SMALL_B
#define WPAD_TRIGGER_Z                WPAD_BUTTON_Z1
#define WPAD_TRIGGER_R                WPAD_BUTTON_Z2
#define WPAD_TRIGGER_L                0x8000

#define WPAD_DPD_MAX_OBJECTS          4
#define WPAD_DPD_ANGLE               42.0f  // viewing angle
#define WPAD_DPD_IMG_RESO_WX       1024     // DPD image resolution 
#define WPAD_DPD_IMG_RESO_WY        768

#define WPAD_ACC_RESO              1024     // accelerometer resolution
#define WPAD_ACC_1G_VALUE           205     // gravity value

#define WPAD_FS_STICK_RESO          256     // stick resolution of FreeStyle unit
#define WPAD_FS_ACC_RESO           1024     // accelerometer resolution of FreeStyle unit
#define WPAD_FS_ACC_1G_VALUE        205     // gravity value of FreeStyle unit

#define WPAD_DEV_CORE                 0
#define WPAD_DEV_FREESTYLE            1
#define WPAD_DEV_DOLPHIN            254
#define WPAD_DEV_UNKNOWN            255
#define WPAD_DEV_NOT_FOUND          253     // For backwards compatibility. Don't use.

#define WPAD_FMT_CORE                 0     // data format of Core controller 
#define WPAD_FMT_FREESTYLE            1     // data format of FreeStyle controller 
#define WPAD_FMT_DOLPHIN            255     // data format of Dolphin controller(for debug) 

#endif

#define WPAD_ERR_NONE                 0
#define WPAD_ERR_NO_CONTROLLER       -1
#define WPAD_ERR_BUSY                -2
#define WPAD_ERR_TRANSFER            -3
#define WPAD_ERR_INVALID             -4
#define WPAD_ERR_NOPERM              -5
#define WPAD_ERR_BROKEN              -6
#define WPAD_ERR_CORRUPTED           -7


#define WPAD_PRESS_UNITS            4


#ifdef  __MWERKS__
#pragma warn_padding off
#endif
typedef struct DPDObject
{
    int16       x;                    // 0 <=  x   < WPAD_DPD_IMG_RESO_WX
    int16       y;                    // 0 <=  y   < WPAD_DPD_IMG_RESO_WY
    uint16       size;
    uint8        traceId;
    // 1 byte padding
} DPDObject;
#ifdef  __MWERKS__
#pragma warn_padding reset
#endif


typedef struct WPADStatus
{
    uint16       button;               // Or-ed WPAD_BUTTON_* bits
    int16       accX;                 // -WPAD_ACC_RESO/2 <= accX < WPAD_ACC_RESO/2
    int16       accY;                 // -WPAD_ACC_RESO/2 <= accY < WPAD_ACC_RESO/2
    int16       accZ;                 // -WPAD_ACC_RESO/2 <= accZ < WPAD_ACC_RESO/2
    DPDObject obj[WPAD_DPD_MAX_OBJECTS];

    uint8        dev;                  // one of WPAD_DEV_* number
    int8        err;                  // one of WPAD_ERR_* number
} WPADStatus;

typedef struct WPADFSStatus
{
    uint16       button;               // Or-ed WPAD_BUTTON_* bits
    int16       accX;                 // -WPAD_ACC_RESO/2 <= accX < WPAD_ACC_RESO/2
    int16       accY;                 // -WPAD_ACC_RESO/2 <= accY < WPAD_ACC_RESO/2
    int16       accZ;                 // -WPAD_ACC_RESO/2 <= accZ < WPAD_ACC_RESO/2
    DPDObject obj[WPAD_DPD_MAX_OBJECTS];

    uint8        dev;                  // one of WPAD_DEV_* number
    int8        err;                  // one of WPAD_ERR_* number

    int16       fsAccX;               // -WPAD_FS_ACC_RESO/2 <= fsAccX < WPAD_ACC_RESO/2
    int16       fsAccY;               // -WPAD_FS_ACC_RESO/2 <= fsAccY < WPAD_ACC_RESO/2
    int16       fsAccZ;               // -WPAD_FS_ACC_RESO/2 <= fsAccZ < WPAD_ACC_RESO/2
    int8        fsStickX;             // -WPAD_FS_STICK_RESO/2 <= fsStickX < WPAD_FS_STICK_RESO/2
    int8        fsStickY;             // -WPAD_FS_STICK_RESO/2 <= fsStickY < WPAD_FS_STICK_RESO/2
} WPADFSStatus;



typedef struct WPADInfo
{
    BOOL dpd;                       // the active status of pointer unit
    BOOL speaker;                   // the active status of speaker unit
    BOOL attach;                    // whether an attachment is installed (=1) or not (=0)
    BOOL lowBat;                    // whether the current battery level is lower than a threshold (0x20)
    BOOL nearempty;                 // sound buffer is nearly empty
    uint8 battery;                     // the current battery level (WPAD_BATTERY_LEVEL_*)
    uint8 led;                         // the current pattern of 4 LEDs
    uint8 protocol;                    // not used
    uint8 firmware;                    // not used
} WPADInfo;

typedef struct WPADAcc
{
    int16 x;
    int16 y;
    int16 z;
} WPADAcc;


typedef void   ( *WPADExtensionCallback         )( int32  chan, int32 result );
typedef void   ( *WPADSamplingCallback          )( int32  chan );
typedef void   ( *WPADConnectCallback           )( int32  chan, int32 reason );
typedef void   ( *WPADCallback                  )( int32  chan, int32 result );
typedef void * ( *WPADAlloc                     )( uint32  size );
typedef uint8     ( *WPADFree                      )( void *ptr );
typedef void   ( *WPADSyncDeviceCallback        )( int32  result, int32 num );
typedef void   ( *WPADClearDeviceCallback       )( int32  result );
typedef void   ( *WPADFlushCallback             )( uint32  result );

#define WPADButtonDown( buttonLast, button )   \
    ( (uint16) (((buttonLast) ^ (button)) & (button)) )

#define WPADButtonUp( buttonLast, button )   \
    ( (uint16) (((buttonLast) ^ (button)) & (buttonLast)) )

#define WPADStartMotor( chan )  WPADControlMotor( ( chan ), WPAD_MOTOR_RUMBLE )
#define WPADStopMotor( chan )   WPADControlMotor( ( chan ), WPAD_MOTOR_STOP   )


#ifdef WPADEMU
// only for WPADEmu
BOOL WPADAttach                 ( int32 chan );
BOOL WPADRecalibrate            ( int32 chan );

#endif


void WPADInit                   ( void )                                            { return ; };
void WPADShutdown               ( void );
void WPADReconnect              ( void );
void WPADRegisterAllocator      ( WPADAlloc alloc, WPADFree free );
uint32  WPADGetWorkMemorySize      ( void );

int32  WPADProbe                  ( int32 chan, uint32 *type )                      
{ 
   if (chan == 0)
   {
      if (type)
      {
         *type = WPAD_DEV_FREESTYLE;
      }
      return 1;
   }

   return WPAD_ERR_NO_CONTROLLER;
}

int32  WPADGetStatus              ( void )                                          { return WPAD_STATE_SETUP; };
uint32  WPADGetDataFormat          ( int32 chan )                                   { return WPAD_FMT_FREESTYLE_ACC_DPD; };
int32  WPADSetDataFormat          ( int32 chan, uint32 fmt )                        { return -1; };

class CInputGeneratorWiimote;
void WPADRead( int32 chan, CInputGeneratorWiimote * pInput, void *status );

#if 0
void WPADRead                   ( int32 chan, void *status )                        
{  
   // TODO: Fill in the read data!
   if (chan == 0)
   {
      WPADFSStatus nunchuck;
      memset(&nunchuck, 0, sizeof(nunchuck));
      memcpy(status, &nunchuck, sizeof(nunchuck));
   }
}
#endif

void WPADSetAutoSamplingBuf     ( int32 chan, void *buf, uint32 length);
uint32  WPADGetLatestIndexInBuf    ( int32 chan );

void WPADGetAccGravityUnit      ( int32 chan, uint32 type, WPADAcc *acc )
{
   // Setting values to zero to force estimation.
   acc->x = 0;
   acc->y = 0;
   acc->z = 0;
}

void WPADGetCLTriggerThreshold  ( int32 chan, uint8 *left, uint8 *right );

#ifdef WPADEMU
void WPADClampStick             ( uint32 fmt, void *status );
#else
void WPADClampStick             ( int32 chan, void *status, uint32 type );
void WPADClampTrigger           ( int32 chan, void *status, uint32 type );
void WPADClampAcc               ( int32 chan, void *status, uint32 type );
#endif

BOOL WPADIsDpdEnabled           ( int32 chan )                                      { return TRUE; };
uint8   WPADGetDpdFormat           ( int32 chan )                                   { return WPAD_DPD_STD; };
int32  WPADControlDpd             ( int32 chan, uint32 command, WPADCallback callback )     { return -1; };
uint8   WPADGetDpdSensitivity      ( void )                                         { return 128; };     // TODO: Check this value!

int32  WPADControlExtGimmick      ( int32 chan, uint32 command, WPADCallback callback );

void WPADDisconnect             ( int32 chan );
int32  WPADGetInfo                ( int32 chan, WPADInfo *info );
int32  WPADGetInfoAsync           ( int32 chan, WPADInfo *info, WPADCallback callback );
void WPADGetAddress             ( int32 chan, uint8 *addr );
uint8   WPADGetRegisteredDevNum    ( void );
uint8   WPADGetTemporaryDevNum     ( void );
uint8   WPADGetRadioSensitivity    ( int32 chan );

BOOL WPADIsSpeakerEnabled       ( int32 chan );
int32  WPADControlSpeaker         ( int32 chan, uint32 command, WPADCallback callback );
uint8   WPADGetSpeakerVolume       ( void );
int32  WPADSendStreamData         ( int32 chan, void *buf, uint16 len );
BOOL WPADCanSendStreamData      ( int32 chan );

void WPADControlMotor           ( int32 chan, uint32 command )                      {};
BOOL WPADIsMotorEnabled         ( void );

void WPADSetAutoSleepTime       ( uint8 minute );
void WPADResetAutoSleepTimeCount( int32 chan );
uint8   WPADGetSensorBarPosition   ( void )                                         { return WPAD_SENSOR_BAR_POS_TOP; };
void WPADClearPortMapTable      ( void );

int32  WPADReadGameData           ( int32 chan, void *p_buf, uint16 len, uint16 offset, WPADCallback callback );
int32  WPADWriteGameData          ( int32 chan, void *p_buf, uint16 len, uint16 offset, WPADCallback callback );
void WPADSetGameTitleUtf16      ( const uint16  *title );
int32  WPADGetGameTitleUtf16      ( int32 chan, const uint16 **title );
//int32  WPADGetGameDataTimeStamp   ( int32 chan, OSTime *time );

BOOL WPADSetAcceptConnection    ( BOOL accept );
BOOL WPADGetAcceptConnection    ( void );

BOOL WPADSetDisableChannel      ( uint16 disable );
BOOL WPADStartSyncDevice        ( void );
BOOL WPADStartClearDevice       ( void );
BOOL WPADStartSimpleSync        ( void );
BOOL WPADStopSimpleSync         ( void );
BOOL WPADStartFastSyncDevice    ( void );
BOOL WPADStartFastSimpleSync    ( void );

BOOL WPADSaveConfig             ( WPADFlushCallback callback );
void WPADEnableMotor            ( BOOL enable );
void WPADSetSpeakerVolume       ( uint8 volume );

int32  WPADGetSyncType            ( int32 chan, uint8 *type );

WPADSamplingCallback            WPADSetSamplingCallback     ( int32 chan, WPADSamplingCallback  callback )     { return NULL; };
WPADExtensionCallback           WPADSetExtensionCallback    ( int32 chan, WPADExtensionCallback callback );
WPADConnectCallback             WPADSetConnectCallback      ( int32 chan, WPADConnectCallback   callback )     { return NULL; };
WPADSyncDeviceCallback          WPADSetSyncDeviceCallback   ( WPADSyncDeviceCallback  callback );
WPADSyncDeviceCallback          WPADSetSimpleSyncCallback   ( WPADSyncDeviceCallback  callback );
WPADClearDeviceCallback         WPADSetClearDeviceCallback  ( WPADClearDeviceCallback callback );

// These can be used in only debug build so that application can debug extension handling
BOOL WPADAttachDummyExtension( int32 chan, uint32 type );
BOOL WPADDetachDummyExtension( int32 chan );

BOOL WPADIsEnabledVSM( void );
BOOL WPADIsEnabledTRN( void );
BOOL WPADIsEnabledGTR( void );
BOOL WPADIsEnabledDRM( void );
BOOL WPADIsEnabledWBC( void );


#ifdef __cplusplus
}
#endif


#endif // __WPAD_H__


/*---------------------------------------------------------------------------*
  Project:     KPAD library version 2
  File:        kpad.h
  Programmers: Keizo Ohta
               HIRATSU Daisuke
               Tojo Haruki
               Tetsuya Sasaki

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifdef __KPADOLD_H__
#error KPAD version 2 (kpad.h) cannot be used with KPADOld (kpadOld.h)
#endif

#ifndef __KPAD_H__
#define __KPAD_H__
/*
#include <revolution/wpad.h>
#include <revolution/wpadTrain.h>
#include <revolution/wpadGuitar.h>
#include <revolution/wpadBalance.h>
#include <revolution/wbc.h>
#include <revolution/types.h>
#include <revolution/mtx/GeoTypes.h>
*/
#ifdef __cplusplus
extern "C" {
#endif


#define KPAD_BUTTON_LEFT        WPAD_BUTTON_LEFT
#define KPAD_BUTTON_RIGHT       WPAD_BUTTON_RIGHT
#define KPAD_BUTTON_DOWN        WPAD_BUTTON_DOWN
#define KPAD_BUTTON_UP          WPAD_BUTTON_UP
#define KPAD_BUTTON_A           WPAD_BUTTON_A
#define KPAD_BUTTON_B           WPAD_BUTTON_B
#define KPAD_BUTTON_HOME        WPAD_BUTTON_HOME

#define KPAD_BUTTON_PLUS        WPAD_BUTTON_PLUS
#define KPAD_BUTTON_MINUS       WPAD_BUTTON_MINUS
#define KPAD_BUTTON_1           WPAD_BUTTON_1
#define KPAD_BUTTON_2           WPAD_BUTTON_2
#define KPAD_BUTTON_Z           WPAD_BUTTON_Z
#define KPAD_BUTTON_C           WPAD_BUTTON_C

#define KPAD_CL_BUTTON_UP       WPAD_CL_BUTTON_UP   
#define KPAD_CL_BUTTON_LEFT     WPAD_CL_BUTTON_LEFT 
#define KPAD_CL_TRIGGER_ZR      WPAD_CL_TRIGGER_ZR  
#define KPAD_CL_BUTTON_X        WPAD_CL_BUTTON_X    
#define KPAD_CL_BUTTON_A        WPAD_CL_BUTTON_A    
#define KPAD_CL_BUTTON_Y        WPAD_CL_BUTTON_Y    
#define KPAD_CL_BUTTON_B        WPAD_CL_BUTTON_B    
#define KPAD_CL_TRIGGER_ZL      WPAD_CL_TRIGGER_ZL  
#define KPAD_CL_RESERVED        WPAD_CL_RESERVED    
#define KPAD_CL_TRIGGER_R       WPAD_CL_TRIGGER_R   
#define KPAD_CL_BUTTON_PLUS     WPAD_CL_BUTTON_PLUS 
#define KPAD_CL_BUTTON_HOME     WPAD_CL_BUTTON_HOME 
#define KPAD_CL_BUTTON_MINUS    WPAD_CL_BUTTON_MINUS
#define KPAD_CL_TRIGGER_L       WPAD_CL_TRIGGER_L   
#define KPAD_CL_BUTTON_DOWN     WPAD_CL_BUTTON_DOWN 
#define KPAD_CL_BUTTON_RIGHT    WPAD_CL_BUTTON_RIGHT

#define KPAD_BUTTON_MASK        0x0000ffff
#define KPAD_BUTTON_RPT         0x80000000

#define KPAD_RING_BUFS          16
#define KPAD_MAX_READ_BUFS      KPAD_RING_BUFS

#define KPAD_READ_ERR_NONE           0
#define KPAD_READ_ERR_NO_DATA       -1
#define KPAD_READ_ERR_NO_CONTROLLER -2
#define KPAD_READ_ERR_SETUP         -3
#define KPAD_READ_ERR_LOCKED        -4
#define KPAD_READ_ERR_INIT          -5

#define KPAD_WBC_ERR_EXIST           1
#define KPAD_WBC_ERR_NONE            0
#define KPAD_WBC_ERR_NO_BATTERY     -1
#define KPAD_WBC_ERR_SETUP          -2
#define KPAD_WBC_ERR_WRONG_TEMP     -3
#define KPAD_WBC_ERR_WRONG_ZERO     -4

typedef enum KPADPlayMode {
    KPAD_PLAY_MODE_LOOSE = 0,
    KPAD_PLAY_MODE_TIGHT
} KPADPlayMode ;

typedef struct Vec2{
    real32 x;
    real32 y;
} Vec2;

// 3D vector, point
typedef struct
{

	real32 x, y, z;

} Vec, *VecPtr, Point3d, *Point3dPtr;


typedef struct Rect {
    real32 left;
    real32 top;
    real32 right;
    real32 bottom;
} Rect;

typedef union KPADEXStatus
{
    struct {
        Vec2    stick ;

        Vec     acc ;
        real32     acc_value ;
        real32     acc_speed ;
    } fs ;

} KPADEXStatus ;

typedef struct KPADStatus
{
    uint32  hold ;
    uint32  trig ;
    uint32  release ;

    Vec  acc ;
    real32  acc_value ;
    real32  acc_speed ;

    Vec2 pos ;
    Vec2 vec ;
    real32  speed ;

    Vec2 horizon ;
    Vec2 hori_vec ;
    real32  hori_speed ;

    real32  dist ;
    real32  dist_vec ;
    real32  dist_speed ;

    Vec2 acc_vertical ;

    uint8   dev_type ;
    int8   wpad_err ;
    int8   dpd_valid_fg ;
    uint8   data_format ;

    KPADEXStatus    ex_status ;
} KPADStatus ;

typedef struct KPADUnifiedWpadStatus {
    union {
        WPADStatus      core ;
        WPADFSStatus    fs ;
    } u ;
    uint8     fmt ;
    uint8     padding ;
} KPADUnifiedWpadStatus ;

void KPADInit    ( void ) ;
void KPADInitEx  ( KPADUnifiedWpadStatus uniRingBufs[], uint32 length ) ;
void KPADShutdown( void ) ;
void KPADReset   ( void ) ;
int32  KPADRead    ( int32 chan, KPADStatus samplingBufs[], uint32 length ) ;
int32  KPADReadEx  ( int32 chan, KPADStatus samplingBufs[], uint32 length, int32 *err ) ;

void KPADSetPosParam ( int32 chan, real32 play_radius, real32 sensitivity ) ;
void KPADSetHoriParam( int32 chan, real32 play_radius, real32 sensitivity ) ;
void KPADSetDistParam( int32 chan, real32 play_radius, real32 sensitivity ) ;
void KPADSetAccParam ( int32 chan, real32 play_radius, real32 sensitivity ) ;

void KPADGetPosParam ( int32 chan, real32 *play_radius, real32 *sensitivity ) ;
void KPADGetHoriParam( int32 chan, real32 *play_radius, real32 *sensitivity ) ;
void KPADGetDistParam( int32 chan, real32 *play_radius, real32 *sensitivity ) ;
void KPADGetAccParam ( int32 chan, real32 *play_radius, real32 *sensitivity ) ;

void KPADSetBtnRepeat( int32 chan, real32 delay_sec, real32 pulse_sec ) ;

void KPADSetObjInterval  ( real32 interval               ) ;
int32  KPADCalibrateDPD    ( int32 chan                   ) ;
void KPADSetSensorHeight ( int32 chan, real32 level        ) ;

// These APIs are not recommended.
// Please use KPADGetUnifiedWpadStatus() instead.
// These are provided for compatibility with KPADOld. But not 100% compatible.

// These APIs are obsolete in SDK3.2 or later
/*
WPADStatus   *KPADGetWPADRingBuffer  ( int32 chan ) ;
WPADFSStatus *KPADGetWPADFSRingBuffer( int32 chan ) ;
WPADCLStatus *KPADGetWPADCLRingBuffer( int32 chan ) ;
*/

void KPADSetFSStickClamp   ( int8 min, int8 max ) ;

void KPADDisableDPD( int32 chan ) ;
void KPADEnableDPD ( int32 chan ) ;

#define KPAD_STATE_CTRL_DPD_START       0
#define KPAD_STATE_CTRL_DPD_FINISHED    1

typedef void ( *KPADControlDpdCallback )( int32 chan, int32 reason ) ;

void KPADSetControlDpdCallback( int32 chan, KPADControlDpdCallback callback ) ;


void KPADEnableAimingMode ( int32 chan ) ;
void KPADDisableAimingMode( int32 chan ) ;

void KPADGetProjectionPos( Vec2 *dst, const Vec2 *src, const Rect *projRect, real32 viRatio ) ;

void KPADGetUnifiedWpadStatus( int32 chan, KPADUnifiedWpadStatus *dst, uint32 count ) ;

void KPADEnableStickCrossClamp ( void ) ;
void KPADDisableStickCrossClamp( void ) ;

WPADSamplingCallback KPADSetSamplingCallback( int32 chan, WPADSamplingCallback callback ) ;
WPADConnectCallback  KPADSetConnectCallback ( int32 chan, WPADConnectCallback  callback ) ;

void KPADSetReviseMode ( int32 chan, BOOL sw ) ;
real32  KPADReviseAcc     ( Vec *acc ) ;
real32  KPADGetReviseAngle( void ) ;

void KPADSetPosPlayMode ( int32 chan, KPADPlayMode mode ) ;
void KPADSetHoriPlayMode( int32 chan, KPADPlayMode mode ) ;
void KPADSetDistPlayMode( int32 chan, KPADPlayMode mode ) ;
void KPADSetAccPlayMode ( int32 chan, KPADPlayMode mode ) ;

KPADPlayMode KPADGetPosPlayMode ( int32 chan ) ;
KPADPlayMode KPADGetHoriPlayMode( int32 chan ) ;
KPADPlayMode KPADGetDistPlayMode( int32 chan ) ;
KPADPlayMode KPADGetAccPlayMode ( int32 chan ) ;

#define KPAD_BUTTON_PROC_MODE_LOOSE     0
#define KPAD_BUTTON_PROC_MODE_TIGHT     1

void KPADSetButtonProcMode( int32 chan, uint8 mode ) ;
uint8   KPADGetButtonProcMode( int32 chan ) ;

#ifdef __cplusplus
}
#endif

#endif // __KPAD_H__



/*---------------------------------------------------------------------------*
  Project:     KPAD library version 2
  File:        KPADinside.h
  Programmers: Keizo Ohta
               HIRATSU Daisuke

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifdef KPADOLD_INSIDE_H__
#error KPADinside.h of KPAD version 2 cannot be used with that of KPADOld
#endif

#ifndef KPAD_INSIDE_H__
#define KPAD_INSIDE_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include <revolution/kpad.h>


/*******************************************************************************
    DEFINE
*******************************************************************************/
//----- Definitions for the different calculations
#define KPAD_CMOS_FOV       WPAD_DPD_ANGLE  // CMOS lens angle of view (Field Of View, in degrees)
#define KPAD_CMOS_HFOV_TAN  0.383864f   // Tangent of half angle of CMOS_FOV

#define KPAD_CMOS_RESO_WX   128     // CMOS resolution
#define KPAD_CMOS_RESO_WY   96
#define KPAD_CMOS_RESO_CX   (KPAD_CMOS_RESO_WX / 2)
#define KPAD_CMOS_RESO_CY   (KPAD_CMOS_RESO_WY / 2)

#define KPAD_DPD_RESO_WX    WPAD_DPD_IMG_RESO_WX    // DPD resolution
#define KPAD_DPD_RESO_WY    WPAD_DPD_IMG_RESO_WY
#define KPAD_DPD_RESO_CX    (KPAD_DPD_RESO_WX / 2)
#define KPAD_DPD_RESO_CY    (KPAD_DPD_RESO_WY / 2)

#define KPAD_USE_OBJECTS    2   // Number of marks used for determining position

#define KPAD_BTN_RPT_TIME_MAX   20000   // btn_repeat_time range (will not be this value)
#define KPAD_BTN_NO_RPT_DELAY   40000   // Delay setting when not repeating

/*******************************************************************************
    TYPEDEF
*******************************************************************************/
typedef struct {
    Vec2    center ;    // Center of mass coordinate (coordinate system where the horizontal range of CMOS will be Å}1)

    int8      error_fg ;  // -1: not displayed, 0: OK, 1: too far out, 2: same coordinate
    int8      state_fg ;  // -1: expected value, 0: normal
    int8      _fg_1 ;     // Extra
    int8      _fg_2 ;     // Extra
} KPADObject ;

typedef union {
    KPADStatus              k;
    KPADUnifiedWpadStatus   w;
} KPADTmpStatus;

typedef struct {
    KPADStatus  status ;

    //----- Variables that adjust how the controller responds
    real32     pos_play_radius ;   // Tolerance radius (size at pos coordinate system)
    real32     pos_sensitivity ;   // Tracking sensitivity (0: slow -> 1: sensitive)

    real32     hori_play_radius ;  // Adjust response of 'horizon'
    real32     hori_sensitivity ;  // 

    real32     dist_play_radius ;  // Adjust response of 'dist'
    real32     dist_sensitivity ;  // 

    real32     acc_play_radius ;   // Adjust the response of 'acc'
    real32     acc_sensitivity ;   // 

    //----- DPD calibration value
    real32     dist_org ;          // Distance during calibration
    Vec2    accXY_nrm_hori ;    // Direction of XY acceleration when the controller is placed horizontally
    Vec2    sec_nrm_hori ;      // Direction from left mark to right mark when the controller is placed horizontally
    Vec2    center_org ;        // Coordinate during calibration
    real32     dpd2pos_scale ;     // Scale value where the left and right pointing edge in the shooting range will be Å}1

    //----- Variables used for internal control
    KPADObject  kobj_sample[ WPAD_DPD_MAX_OBJECTS ] ;   // All captured marks will be covered
    KPADObject  kobj_regular[ KPAD_USE_OBJECTS ] ;      // Left to right order (top to bottom order for straight vertical)

    int16     valid_objs ;        // Valid object count
    uint8      bufIdx ;
    uint8      bufCount ;

    KPADUnifiedWpadStatus        uniRingBuf[ KPAD_RING_BUFS ] ;
    KPADUnifiedWpadStatus       *uniRingBufEx ;         // Pointer to additional ring buffer
    uint32                          uniRingBufExLen ;      // Length of additional ring buffer

    real32     sec_length ;        // Distance between left and right marks
    Vec2    sec_nrm ;           // Direction from the left to right mark
    real32     sec_dist ;          // Raw distance from the TV (meter)
    real32     trust_sec_length ;  // Trusted interval distance stored here

    Vec     hard_acc ;          // Raw acceleration

    Vec2    obj_horizon ;       // Tilt calculated with the object
    Vec2    acc_horizon ;       // Tilt calculated with the acceleration (either side)

    Vec2    ah_circle_pos ;     // Point chasing the acc_horizon
    uint16     ah_circle_ct ;      // Countdown if in a certain range (stopped if zero)

    uint8      dpd_valid2_ct ;     // Number of successful consecutive two point recognitions
    uint8      readLocked ;        // The KPADRead function is in progress

    //----- Button control
    uint16     btn_repeat_time ;   // Time to be counted
    uint16     btn_repeat_next ;   // Time of setting next flag
    uint16     btn_repeat_delay ;  // Repeat start time
    uint16     btn_repeat_pulse ;  // Repeat interval time
    //----- For Classic Controller unit
    uint16     btn_cl_repeat_time ;   // Time to be counted
    uint16     btn_cl_repeat_next ;   // Time of setting next flag

    KPADControlDpdCallback  dpd_ctrl_callback ; // User callback when DPD setting change becomes necessary

    real32        acc_scale_x ;
    real32        acc_scale_y ;
    real32        acc_scale_z ;
    real32     fs_acc_scale_x ;
    real32     fs_acc_scale_y ;
    real32     fs_acc_scale_z ;

    Vec2    kobj_frame_min;     // Range where the center of mass coordinate is valid
    Vec2    kobj_frame_max;     //
    real32     err_dist_speed_1 ;  // Reciprocal
    real32     err_dist_speedM_1 ; // Reciprocal of negative value
    real32     ah_circle_radius2 ; // Second power
    real32     dist_vv1 ;          // Constants
    real32     err_dist_min ;      // Automatic calculation of minimum operational distance (in meters)

    WPADSamplingCallback    appSamplingCallback ;    // For application
    WPADConnectCallback     appConnectCallback ;     // For application

    uint8      resetReq ;
    uint8      exResetReq ;
    uint8      dpdNextState ;
    uint8      dpdCurrState ;
    
    uint8      dpdIssued ;
    uint8      dpdCmd ;
    uint8      dpdPreCallbackDone ;
    uint8      dpdPostCallbackDone ;

    uint8      aimReq ;
    uint8      aimEnabled ;
    
    uint8      fsAccRevise ;
    uint8      btnProcMode ;

    KPADPlayMode    pos_play_mode ;
    KPADPlayMode    hori_play_mode ;
    KPADPlayMode    dist_play_mode ;
    KPADPlayMode    acc_play_mode ;

} KPADInsideStatus;


/*******************************************************************************
    EXTERNAL WORK
*******************************************************************************/
//----- Various adjustments
extern real32  kp_obj_interval ;       // Separation between marks at each extreme (in meters)
extern real32  kp_acc_horizon_pw ;     // Calculating twist from acceleration
extern real32  kp_ah_circle_radius ;   // Static determination radius
extern real32  kp_ah_circle_pw ;       // Static determination tracking level
extern uint16  kp_ah_circle_ct ;       // Static determination count
extern BOOL kp_stick_clamp_cross ;  // Perform cross-clamping


//----- Numerical values regarded as errors
extern real32  kp_err_outside_frame ;  // Width of surrounding area where center of mass coordinate is invalid (not all surrounding lights are necessarily shown)
extern real32  kp_err_dist_min ;       // Minimum operational distance (in meters)
extern real32  kp_err_dist_max ;       // Maximum operational distance (in meters)
extern real32  kp_err_dist_speed ;     // Acceptable range of change in distance (in meters)
extern real32  kp_err_first_inpr ;     // Dot product of acceleration tilt and object tilt when selecting two points for the first time
extern real32  kp_err_next_inpr ;      // Acceptable range of change in tilt (internal product value)
extern real32  kp_err_acc_inpr ;       // Acceptable range for internal product with static acceleration tilt
extern real32  kp_err_up_inpr ;        // Acceptable range for internal product with controller pointed upwards
extern real32  kp_err_near_pos ;       // Distance from the previous point when selecting one point as a continuation

//----- KPAD
extern KPADInsideStatus inside_kpads[] ;


/*******************************************************************************
    EXTERNAL FUNCTIONS
*******************************************************************************/


#ifdef __cplusplus
}
#endif

#endif // KPAD_INSIDE_H__


//#include <revolution.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

//#include <revolution/revodefs.h>
//REVOLUTION_LIB_VERSION(KPAD);



/*******************************************************
        VARIABLES
 *******************************************************/
//----- DPD calibration initial values
static Vec2     icenter_org = { 0.000f, 0.000f } ;      // Center coordinate of two marks in CMOS
static real32      idist_org = 1.000f ;                    // Distance during calibration (in meters)
static Vec2     iaccXY_nrm_hori = { 0.000f,-1.000f } ;  // Direction of XY acceleration when the controller is placed horizontally
static Vec2     isec_nrm_hori   = { 1.000f, 0.000f } ;  // Direction from left mark to right mark when the controller is placed horizontally
real32             kp_obj_interval = 0.200f ;              // Separation between marks at each extreme (in meters)

//----- Various adjustments
real32             kp_acc_horizon_pw   = 0.050f ;  // Calculating twist from acceleration
real32             kp_ah_circle_radius = 0.070f ;  // Static determination radius
real32             kp_ah_circle_pw     = 0.060f ;  // Static determination tracking level
uint16             kp_ah_circle_ct     = 100 ;     // Static determination count
BOOL            kp_stick_clamp_cross = FALSE ;  // Perform circular clamping

//----- Numerical values regarded as errors
real32             kp_err_outside_frame = 0.050f ; // Width of surrounding area where center of mass coordinate is invalid (not all surrounding lights are necessarily shown)
real32             kp_err_dist_min ;               // Automatic calculation of minimum operational distance (in meters)
real32             kp_err_dist_max      = 3.000f ; // Maximum operational distance (in meters)
real32             kp_err_dist_speed    = 0.040f ; // Acceptable range of change in distance (in meters)
real32             kp_err_first_inpr    = 0.900f ; // Dot product of acceleration tilt and object tilt when selecting two points for the first time
real32             kp_err_next_inpr     = 0.900f ; // Acceptable range of change in tilt (internal product value)
real32             kp_err_acc_inpr      = 0.900f ; // Acceptable range for internal product with static acceleration tilt
real32             kp_err_up_inpr       = 0.700f ; // Acceptable range for internal product with controller pointed upwards
real32             kp_err_near_pos      = 0.100f ; // Distance from the previous point when selecting one point as a continuation

//----- For internal processing
//static Vec2     kobj_frame_min, kobj_frame_max ;        // Range over which center of gravity coordinates are enabled
//static real32      kp_err_dist_speed_1 ;                   // Reciprocal
//static real32      kp_err_dist_speedM_1 ;                  // Negative reciprocal
//static real32      kp_ah_circle_radius2 ;                  // Square
static real32      kp_dist_vv1 ;                           // Constants

static int32      kp_fs_fstick_min  =  15 ;   // Nunchuk unit stick clamp settings
static int32      kp_fs_fstick_max  =  71 ;
static int32      kp_cl_stick_min   =  60 ;   // Classic Controller unit stick clamp settings
static int32      kp_cl_stick_max   = 308 ;
static int32      kp_cl_trigger_min =  30 ;   // Classic Controller unit analog trigger clamp settings
static int32      kp_cl_trigger_max = 180 ;
//static int32      kp_gc_mstick_min  =  15 ;   // Old GC3D stick clamp setting
//static int32      kp_gc_mstick_max  =  77 ;
//static int32      kp_gc_cstick_min  =  15 ;   // Old GCC stick clamp setting
//static int32      kp_gc_cstick_max  =  64 ;
//static int32      kp_gc_trigger_min =  30 ;   // Old GC analog trigger clamp setting
//static int32      kp_gc_trigger_max = 180 ;
static real32      kp_rm_acc_max     = 3.4f ;  // Wii Remote acceleration clamp settings
static real32      kp_fs_acc_max     = 2.1f ;  // Nunchuk acceleration clamp settings
static int32      kp_ex_trigger_min =    0 ;  // Clamp settings for analog trigger buttons on an extension controller unit
static int32      kp_ex_trigger_max =  256 ;
static int32      kp_ex_analog_min  =    0 ;  // Clamp settings for analog input from an extension controller unit
static int32      kp_ex_analog_max  = 1024 ;


static uint8       kp_initialized = 0 ; // Has the KPADInit function been called?

//----- KPAD
KPADInsideStatus        inside_kpads[ WPAD_MAX_CONTROLLERS ] ;

//----- Zero vector
static Vec2 Vec2_0 = { 0.0f, 0.0f } ;

//----- Used for Nunchuk acceleration correction
static CMatrix34      kp_fs_rot(CMatrix34::Identity()) ;                 // Rotation matrix
static real32      kp_fs_revise_deg = 24.0f ;  // Correction angle (in degrees)

static void     KPADiSamplingCallback     ( int32 chan, CInputGeneratorWiimote * pInput ) ;
static void     KPADiConnectCallback      ( int32 chan, int32 reason ) ;
static void     KPADiControlDpdCallback   ( int32 chan, int32 result ) ;

static int32      KPADiRead( int32 chan, KPADStatus samplingBufs[], uint32 length, int32 *err, BOOL keep ) ;

/*******************************************************
        EXTERN
 *******************************************************/
//----- Function to emit a warning when a callback used by the KPAD library is overwritten (this function is only used by the KPAD library)
void WPADSetCallbackByKPAD( BOOL use )       { };

// These APIs are not recommended.
// Please use the KPADGetUnifiedWpadStatus function instead.
// These are provided for compatibility with KPAD1. But not 100% compatible.

// These functions are not supported in SDK 3.2 or later.

#if 0

static void    *get_ring_buffer_by_kpad1_style( int32 chan, void *buf, uint32 size ) ;

/*******************************************************************************
        Get the ring buffer of WPADStatus
 *******************************************************************************/
WPADStatus *KPADGetWPADRingBuffer( int32 chan )
{
    static WPADStatus status[ KPAD_RING_BUFS ] ;

    return (WPADStatus *)get_ring_buffer_by_kpad1_style( chan, &status, WPAD_DEV_CORE ) ;
}

WPADFSStatus *KPADGetWPADFSRingBuffer( int32 chan )
{
    static WPADFSStatus status[ KPAD_RING_BUFS ] ;

    return (WPADFSStatus *)get_ring_buffer_by_kpad1_style( chan, &status, WPAD_DEV_FREESTYLE ) ;
}

WPADCLStatus *KPADGetWPADCLRingBuffer( int32 chan )
{
    static WPADCLStatus status[ KPAD_RING_BUFS ] ;

    return (WPADCLStatus *)get_ring_buffer_by_kpad1_style( chan, &status, WPAD_DEV_CLASSIC ) ;
}

static void *get_ring_buffer_by_kpad1_style( int32 chan, void *buf, uint32 dev )
{
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;
    BOOL    enabled ;
    int32     idx1 ;
    int32     idx2 ;
    uint32     count ;
    uint32     size ;
    uint32     type ;

    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;

    switch ( dev ) {
      case WPAD_DEV_CORE :
        size = sizeof( WPADStatus ) ;
        break ;

      case WPAD_DEV_FREESTYLE :
        size = sizeof( WPADFSStatus ) ;
        break ;

      case WPAD_DEV_CLASSIC :
        size = sizeof( WPADCLStatus ) ;
        break ;

      default :
        return buf ;
    }

    enabled = OSDisableInterrupts() ;
    idx1 = kp->bufIdx - 1 ;
    idx2 = (int32)WPADGetLatestIndexInBuf( chan ) ;
    for ( count = 0; count < KPAD_RING_BUFS; count++ ) {
        if ( idx1 < 0 ) {
            idx1 = KPAD_RING_BUFS - 1 ;
        }
        if ( idx2 < 0 ) {
            idx2 = KPAD_RING_BUFS - 1 ;
        }
        
        switch( kp->uniRingBuf[idx1].u.core.dev ) {
            case WPAD_DEV_CORE:
            case WPAD_DEV_FUTURE:
            case WPAD_DEV_NOT_SUPPORTED:
            case WPAD_DEV_UNKNOWN:
                type = WPAD_DEV_CORE;
                break;
                
            case WPAD_DEV_FREESTYLE:
                type = WPAD_DEV_FREESTYLE;
                break;
                
            case WPAD_DEV_CLASSIC:
                type = WPAD_DEV_CLASSIC;
                break;
            
            default:
                // Unreachable here.
                type = WPAD_DEV_UNKNOWN;
                break;
        }
        
        if ( type == dev ) {
            if ( WPADGetStatus() != WPAD_STATE_SETUP ) {
                kp->uniRingBuf[idx1].u.core.err = WPAD_ERR_INVALID ;
            }
            memcpy((uint8 *)buf + idx2 * size,
                   &kp->uniRingBuf[ idx1 ].u,
                   size) ;
        }
        idx1-- ;
        idx2-- ;
    }
    (void)OSRestoreInterrupts( enabled ) ;

    return buf ;
}
#endif

/*******************************************************************************
        Analog Data Clamp Settings
*******************************************************************************/
void KPADSetFSStickClamp( int8 min, int8 max )
{
    kp_fs_fstick_min = (int32)min ;
    kp_fs_fstick_max = (int32)max ;
}


/*******************************************************************************
        Configure the Button Repeat Rate
 *******************************************************************************/
void KPADSetBtnRepeat( int32 chan, real32 delay_sec, real32 pulse_sec )
{
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;

    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    if ( pulse_sec ) {
        //----- Set repeat flag setting
        kp->btn_repeat_delay = (uint16)(int32)( delay_sec * 200.0f + 0.5f ) ;
        kp->btn_repeat_pulse = (uint16)(int32)( pulse_sec * 200.0f + 0.5f ) ;
    } else {
        //----- No repeat flag setting
        kp->btn_repeat_delay = KPAD_BTN_NO_RPT_DELAY ;
        kp->btn_repeat_pulse = 0 ;
    }

    //----- Reset
    kp->btn_repeat_time = 0 ;
    kp->btn_repeat_next = kp->btn_repeat_delay ;
    kp->btn_cl_repeat_time = 0 ;
    kp->btn_cl_repeat_next = kp->btn_repeat_delay ;
}


/*******************************************************************************
        Set Marker Placement Interval (in meters)
 *******************************************************************************/
void KPADSetObjInterval( real32 interval )
{
//#pragma unused( interval )

    // KPADSetObjInterval is obsolete.
}

static void set_obj_interval( real32 interval )
{
    //BOOL enabled = OSDisableInterrupts() ;

    kp_obj_interval = interval ;

    //----- DPD operational minimum distance (so that the length between marks will be half that of lens diameter)
    kp_err_dist_min = interval / KPAD_CMOS_HFOV_TAN ;

    //----- Constants for calculating distance
    kp_dist_vv1 = interval / KPAD_CMOS_HFOV_TAN ;

    //(void)OSRestoreInterrupts( enabled ) ;
    
}


/*******************************************************************************
        Parameter set
 *******************************************************************************/
void KPADSetPosParam( int32 chan, real32 play_radius, real32 sensitivity )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].pos_play_radius = play_radius ;
    inside_kpads[ chan ].pos_sensitivity = sensitivity ;
}

void KPADSetHoriParam( int32 chan, real32 play_radius, real32 sensitivity )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) );
    inside_kpads[ chan ].hori_play_radius = play_radius ;
    inside_kpads[ chan ].hori_sensitivity = sensitivity ;
}

void KPADSetDistParam( int32 chan, real32 play_radius, real32 sensitivity )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].dist_play_radius = play_radius ;
    inside_kpads[ chan ].dist_sensitivity = sensitivity ;
}

void KPADSetAccParam( int32 chan, real32 play_radius, real32 sensitivity )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].acc_play_radius = play_radius ;
    inside_kpads[ chan ].acc_sensitivity = sensitivity ;
}

/*******************************************************************************
        Get Parameters
 *******************************************************************************/
void KPADGetPosParam( int32 chan, real32 *play_radius, real32 *sensitivity )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    *play_radius = inside_kpads[ chan ].pos_play_radius ;
    *sensitivity = inside_kpads[ chan ].pos_sensitivity ;
}

void KPADGetHoriParam( int32 chan, real32 *play_radius, real32 *sensitivity )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    *play_radius = inside_kpads[ chan ].hori_play_radius ;
    *sensitivity = inside_kpads[ chan ].hori_sensitivity ;
}

void KPADGetDistParam( int32 chan, real32 *play_radius, real32 *sensitivity )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    *play_radius = inside_kpads[ chan ].dist_play_radius ;
    *sensitivity = inside_kpads[ chan ].dist_sensitivity ;
}

void KPADGetAccParam( int32 chan, real32 *play_radius, real32 *sensitivity )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    *play_radius = inside_kpads[ chan ].acc_play_radius ;
    *sensitivity = inside_kpads[ chan ].acc_sensitivity ;
}


/*******************************************************************************
        Seek an easy-to-use scale value from the calibrated center position
 *******************************************************************************/
static void calc_dpd2pos_scale( KPADInsideStatus *kp )
{
    real32  scale ;
    real32  sx,sy ;

    //----- Movable distance of the controller in the vertical and horizontal directions
    sx = 1.0f ;                                             // Horizontal
    sy = (real32)KPAD_DPD_RESO_WY / (real32)KPAD_DPD_RESO_WX ;    // Vertical

    //----- Longest movable distance of the controller
    scale = sqrtf( sx * sx + sy * sy ) ;// Diagonal

    //----- Correct the horizontal movable distance
    if ( kp->center_org.x < 0.0f ) {
        sx += kp->center_org.x ;
    } else {
        sx -= kp->center_org.x ;
    }

    //----- Correct the vertical movable distance
    if ( kp->center_org.y < 0.0f ) {
        sy += kp->center_org.y ;
    } else {
        sy -= kp->center_org.y ;
    }

    //----- Scale that will cover the longest distance of the smaller of the range of movable distance
    kp->dpd2pos_scale = scale / ( ( sx < sy ) ? sx : sy ) ;
}


/*******************************************************************************
        Initialize KPAD Values
 *******************************************************************************/
static void reset_kpad( KPADInsideStatus *kp )
{
    KPADObject      *op ;
    KPADStatus      *sp = &kp->status ;
    KPADEXStatus    *ep = &kp->status.ex_status ;

    kp->resetReq = FALSE ;

    //----- Recalculate constants
    kp->kobj_frame_min.x = -1.0f + kp_err_outside_frame ;
    kp->kobj_frame_max.x =  1.0f - kp_err_outside_frame ;
    kp->kobj_frame_min.y = -((real32)KPAD_DPD_RESO_WY / KPAD_DPD_RESO_WX) + kp_err_outside_frame ;
    kp->kobj_frame_max.y =  ((real32)KPAD_DPD_RESO_WY / KPAD_DPD_RESO_WX) - kp_err_outside_frame ;

    kp->err_dist_speed_1  =  1.0f /  kp_err_dist_speed ;
    kp->err_dist_speedM_1 = -1.0f /  kp_err_dist_speed ;
    kp->ah_circle_radius2 = kp_ah_circle_radius * kp_ah_circle_radius ;

    kp->err_dist_min = kp_err_dist_min ;
    kp->dist_vv1     = kp_dist_vv1 ;

    //----- Clear button information
    sp->hold = sp->trig = sp->release = 0x00000000 ;
    kp->btn_repeat_time = 0 ;
    kp->btn_repeat_next = kp->btn_repeat_delay ;

    //----- Clear DPD information
    sp->dpd_valid_fg  = 0 ;          // Disabled
    kp->dpd_valid2_ct = 0 ;

    sp->pos = sp->vec = Vec2_0 ;
    sp->speed = 0.0f ;

    sp->horizon.x = kp->acc_horizon.x = kp->obj_horizon.x = 1.0f ;
    sp->horizon.y = kp->acc_horizon.y = kp->obj_horizon.y = 0.0f ;
    sp->hori_vec   = Vec2_0 ;
    sp->hori_speed = 0.0f ;

    sp->acc_vertical.x = 1.0f ;
    sp->acc_vertical.y = 0.0f ;

    sp->dist = kp->dist_org ;
    sp->dist_vec = sp->dist_speed = 0.0f ;

    kp->sec_dist = sp->dist ;
    kp->sec_length = kp->trust_sec_length = kp->dist_vv1 / kp->sec_dist ;
    kp->sec_nrm = kp->sec_nrm_hori ;

    //----- Clear acceleration information
    sp->acc.x = sp->acc.z = 0.0f ;
    sp->acc.y = -1.0f ;
    sp->acc_value = 1.0f ;
    sp->acc_speed = 0.0f ;
    kp->hard_acc = sp->acc ;

    kp->ah_circle_pos = kp->acc_horizon ;
    kp->ah_circle_ct = kp_ah_circle_ct ;

    //----- Clear individual object information
    kp->valid_objs = 0 ;

    op = &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS - 1 ] ;
    do {
        op->error_fg = -1 ;     // Not applied
    } while ( --op >= kp->kobj_sample ) ;

    op = &kp->kobj_regular[ KPAD_USE_OBJECTS - 1 ] ;
    do {
        op->error_fg = -1 ;     // Not applied
    } while ( --op >= kp->kobj_regular ) ;

    //----- Other
    kp->bufCount = 0 ;          // Ring buffer unprocessed

    //----- Clear extension controller information
    kp->exResetReq = TRUE ;
}


/*******************************************************************************
        Convert DPD Coordinates to Projection Coordinate System
 *******************************************************************************/
void KPADGetProjectionPos( Vec2 *dst, const Vec2 *src, const Rect *projRect, real32 viRatio )
{
    real32 projection_height = projRect->bottom - projRect->top ;

    // Convert the normalized values into projection coordinates.
    (*dst).x = src->x * (projection_height / 2.0f) * 1.2f ;
    (*dst).y = src->y * (projection_height / 2.0f) * 1.2f ;
    // Horizontal direction pixel ratio correction
    (*dst).x *= viRatio * 0.908 ;
}


/*******************************************************************************
        Calibration process
 *******************************************************************************/
int32 KPADCalibrateDPD( int32 chan )
{
    KPADInsideStatus        *kp = &inside_kpads[ chan ] ;
    KPADStatus              *sp = &kp->status ;
    KPADObject              *op1, *op2 ;
    real32                     f1, vx,vy ;

    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;

    if ( kp->valid_objs != KPAD_USE_OBJECTS ) return ( kp->valid_objs ) ;

    /***********************************************************************
                Acceleration during calibration
     ***********************************************************************/
    vx = kp->hard_acc.x ;
    vy = kp->hard_acc.y ;
    f1 = sqrtf( vx * vx + vy * vy ) ;
    if ( f1 <= 0.5f ) return (-1) ;         // Abnormal acceleration
    kp->accXY_nrm_hori.x = vx / f1 ;
    kp->accXY_nrm_hori.y = vy / f1 ;

    /***********************************************************************
                Object location during calibration
     ***********************************************************************/
    //----- Determine the mark order by location
    op1 = kp->kobj_sample ;
    while ( op1->error_fg != 0 ) ++op1 ;
    op2 = op1 + 1 ;
    while ( op2->error_fg != 0 ) ++op2 ;

    if ( op1->center.x < op2->center.x )      goto LABEL_cp12 ;
    else if ( op2->center.x < op1->center.x ) goto LABEL_cp21 ;
    else if ( op2->center.y < op1->center.y ) goto LABEL_cp21 ;

LABEL_cp12:
    kp->kobj_regular[ 0 ] = *op1 ;
    kp->kobj_regular[ 1 ] = *op2 ;
    goto LABEL_cpend ;

LABEL_cp21:
    kp->kobj_regular[ 0 ] = *op2 ;
    kp->kobj_regular[ 1 ] = *op1 ;

LABEL_cpend:

    //kp->center_org.x = ( kp->kobj_regular[0].center.x + kp->kobj_regular[1].center.x ) * 0.5f ;
    //kp->center_org.y = ( kp->kobj_regular[0].center.y + kp->kobj_regular[1].center.y ) * 0.5f ;
    //kp->center_org.x = kp->center_org.y = 0.0f ;
    calc_dpd2pos_scale( kp ) ;

    //----- Section direction when the controller is in horizontal position
    vx = kp->kobj_regular[ KPAD_USE_OBJECTS - 1 ].center.x - kp->kobj_regular[ 0 ].center.x ;
    vy = kp->kobj_regular[ KPAD_USE_OBJECTS - 1 ].center.y - kp->kobj_regular[ 0 ].center.y ;
    f1 = 1.0f / sqrtf( vx * vx + vy * vy ) ;        // Should not be zero
    kp->sec_nrm_hori.x = vx * f1 ;
    kp->sec_nrm_hori.y = vy * f1 ;

    /***********************************************************************
                Distance during calibration
     ***********************************************************************/
    kp->dist_org = kp->dist_vv1 * f1 ;

    /***********************************************************************
                Other
     ***********************************************************************/
    sp->dpd_valid_fg = 0 ;  // Invalid for now

    return ( kp->valid_objs ) ;
}


/*******************************************************************************
        Enable Wii Remote Orientation Correction
 *******************************************************************************/
void KPADEnableAimingMode( int32 chan )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].aimReq     = TRUE ;
    inside_kpads[ chan ].aimEnabled = TRUE ;
}


/*******************************************************************************
        Disable Wii Remote Orientation Correction
 *******************************************************************************/
void KPADDisableAimingMode( int32 chan )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) );
    inside_kpads[ chan ].aimReq     = TRUE ;
    inside_kpads[ chan ].aimEnabled = FALSE ;
}


/*******************************************************************************
        Calibration process
*******************************************************************************/
void KPADSetSensorHeight( int32 chan, real32 level )
{
    KPADInsideStatus        *kp = &inside_kpads[ chan ] ;

    kp->center_org.x = 0.0f ;
    kp->center_org.y = -level ;
    calc_dpd2pos_scale( kp ) ;
}


/*******************************************************************************
        Digital Button Repeat Process
*******************************************************************************/
static void calc_button_repeat( KPADInsideStatus *kp, uint32 dev_type, uint32 count )
{
    KPADStatus      *sp = &kp->status ;
    KPADEXStatus    *ep = &kp->status.ex_status ;

    if ( sp->trig != 0 || sp->release != 0 ) {
        //----- Reset because the button state changed
        kp->btn_repeat_time = 0 ;
        kp->btn_repeat_next = kp->btn_repeat_delay ;

        //----- Set flags also at the beginning of button press (but only when repeat is set)
        if ( sp->trig && kp->btn_repeat_pulse ) {
            sp->hold |= KPAD_BUTTON_RPT ;
        }
    } else if ( sp->hold != 0 ) {
        //----- Advance time because the button is pushed and its state is not changed
        kp->btn_repeat_time += count ;
        if ( kp->btn_repeat_time >= KPAD_BTN_NO_RPT_DELAY ) {
            kp->btn_repeat_time -= KPAD_BTN_NO_RPT_DELAY ;
        }

        //----- Set flag when the repeat time is reached
        if ( kp->btn_repeat_time >= kp->btn_repeat_next ) {
            sp->hold |= KPAD_BUTTON_RPT ;

            //----- Set the next repeat time
            kp->btn_repeat_next += kp->btn_repeat_pulse ;

            //----- Loop if the time has exceeded its range here
            if ( kp->btn_repeat_time >= KPAD_BTN_RPT_TIME_MAX ) {
                kp->btn_repeat_time -= KPAD_BTN_RPT_TIME_MAX ;
                kp->btn_repeat_next -= KPAD_BTN_RPT_TIME_MAX ;
            }
        }
    }
}


/*******************************************************************************
        KPAD Button Information Loading
 *******************************************************************************/
static void read_kpad_button( KPADInsideStatus *kp, uint32 dev_type, uint32 count, uint32 core, uint32 fs, uint32 cl)
{
    KPADStatus      *sp = &kp->status ;
    KPADEXStatus    *ep = &kp->status.ex_status ;
    uint32             old_fg, change_fg ;
    uint32             ex ;

    //----- Store the previous value
    old_fg = sp->hold & KPAD_BUTTON_MASK ;

    //----- Load new value
    sp->hold = ( core & (KPAD_BUTTON_MASK & ~(WPAD_BUTTON_Z | WPAD_BUTTON_C)) ) ;

    //----- Load Nunchuk buttons
    if ( dev_type == WPAD_DEV_FREESTYLE ) {
        if ( kp->exResetReq ) {
            ex = 0 ;
        } else {
            ex = fs ;
        }
        sp->hold |= ( ex & (WPAD_BUTTON_Z | WPAD_BUTTON_C) ) ;
    }

    //----- Button state processing
    change_fg = sp->hold ^ old_fg ;    // Changed button
    sp->trig = change_fg & sp->hold ;  // Pressed button
    sp->release = change_fg & old_fg ; // Released button


    //----- Repeat processing
    calc_button_repeat( kp, dev_type, count ) ;
}


/*******************************************************************************
        Acceleration Tracking
 *******************************************************************************/
static void calc_acc( KPADInsideStatus *kp, real32 *acc, real32 acc2 )
{
    real32             f1,f2 ;


    //----- Difference to the target value
    f2 = acc2 - *acc ;

    if ( kp->acc_play_mode == KPAD_PLAY_MODE_LOOSE ) {
        if ( f2 < 0.0f ) {
            f1 = -f2 ;
        } else {
            f1 = f2 ;
        }

        //----- Calculation of tracking rate inside/outside the play tolerance
        if ( f1 >= kp->acc_play_radius ) {
            //----- Apply 100% tracking sensitivity if outside play tolerance
            f1 = 1.0f ;
        } else {
            //----- If inside play tolerance, weaken tracking sensitivity as target gets closer
            f1 /= kp->acc_play_radius ;
            f1 *= f1 ;      // Second power
            f1 *= f1 ;      // Fourth power
        }
        f1 *= kp->acc_sensitivity ;

        //----- Tracking
        *acc += f1 * f2 ;
    } else {
        if ( f2 < -kp->acc_play_radius ) {
            *acc += ( f2 + kp->acc_play_radius ) * kp->acc_sensitivity ;
        } else if ( f2 > kp->acc_play_radius ) {
            *acc += ( f2 - kp->acc_play_radius ) * kp->acc_sensitivity ;
        }
    }
}


/*******************************************************************************
        Calculate Controller Tilt from Acceleration
 *******************************************************************************/
static void calc_acc_horizon( KPADInsideStatus *kp )
{
    real32             f1, vx,vy, ax,ay ;


    //----- XY acceleration normalization
    f1 = sqrtf( kp->hard_acc.x * kp->hard_acc.x + kp->hard_acc.y * kp->hard_acc.y ) ;
    if ( f1 == 0.0f || f1 >= 2.0f ) return ;
    ax = kp->hard_acc.x / f1 ;
    ay = kp->hard_acc.y / f1 ;

    //----- There will be more power the closer the XY acceleration length is to one (1).
    if ( f1 > 1.0f ) {
        f1 = 2.0f - f1 ;
    }
    f1 *= f1 * kp_acc_horizon_pw ;

    //----- Target tilt
    vx = kp->accXY_nrm_hori.x * ax + kp->accXY_nrm_hori.y * ay ;
    vy = kp->accXY_nrm_hori.y * ax - kp->accXY_nrm_hori.x * ay ;

    //----- Set closer
    ax = ( vx - kp->acc_horizon.x ) * f1 + kp->acc_horizon.x ;
    ay = ( vy - kp->acc_horizon.y ) * f1 + kp->acc_horizon.y ;

    //----- Normalization
    f1 = sqrtf( ax * ax + ay * ay ) ;
    if ( f1 == 0.0f ) return ;
    kp->acc_horizon.x = ax / f1 ;
    kp->acc_horizon.y = ay / f1 ;


    //----- Update static determination coordinate
    kp->ah_circle_pos.x += ( kp->acc_horizon.x - kp->ah_circle_pos.x ) * kp_ah_circle_pw ;
    kp->ah_circle_pos.y += ( kp->acc_horizon.y - kp->ah_circle_pos.y ) * kp_ah_circle_pw ;

    vx = kp->acc_horizon.x - kp->ah_circle_pos.x ;
    vy = kp->acc_horizon.y - kp->ah_circle_pos.y ;
    if ( vx*vx + vy*vy <= kp->ah_circle_radius2 ) {
        if ( kp->ah_circle_ct ) -- kp->ah_circle_ct ;
    } else {
        kp->ah_circle_ct = kp_ah_circle_ct ;
    }
}

static void calc_acc_vertical( KPADInsideStatus *kp )
{
    KPADStatus      *sp = &kp->status ;
    real32             f1,f2, ax,ay ;


    //----- Target tilt
    ax = sqrtf( f2 = kp->hard_acc.x * kp->hard_acc.x + kp->hard_acc.y * kp->hard_acc.y ) ;
    ay = - kp->hard_acc.z ;
    f1 = sqrtf( f2 + ay * ay ) ;
    if ( f1 == 0.0f || f1 >= 2.0f ) return ;
    ax /= f1 ;
    ay /= f1 ;

    //----- There will be more power the closer the acceleration length is to one (1).
    if ( f1 > 1.0f ) {
        f1 = 2.0f - f1 ;
    }
    f1 *= f1 * kp_acc_horizon_pw ;

    //----- Set closer
    ax = ( ax - sp->acc_vertical.x ) * f1 + sp->acc_vertical.x ;
    ay = ( ay - sp->acc_vertical.y ) * f1 + sp->acc_vertical.y ;

    //----- Normalization
    f1 = sqrtf( ax * ax + ay * ay ) ;
    if ( f1 == 0.0f ) return ;
    sp->acc_vertical.x = ax / f1 ;
    sp->acc_vertical.y = ay / f1 ;
}


/*******************************************************************************
        KPAD Acceleration Information Loading
 *******************************************************************************/
static real32 clamp_acc( real32 acc, real32 clamp )
{
    if ( acc < 0.0f ) {
        if ( acc < -clamp ) return ( -clamp ) ;
    } else {
        if ( acc > clamp ) return ( clamp ) ;
    }
    return ( acc ) ;
}

static void read_kpad_acc( KPADInsideStatus *kp, KPADUnifiedWpadStatus *uwp )
{
    KPADStatus      *sp = &kp->status ;
    Vec             fsrc ;
    Vec             vec ;

    switch ( uwp->fmt ) {
      case WPAD_FMT_CORE_ACC :
      case WPAD_FMT_CORE_ACC_DPD :
      case WPAD_FMT_FREESTYLE_ACC :
      case WPAD_FMT_FREESTYLE_ACC_DPD :
      case WPAD_FMT_CLASSIC_ACC :
      case WPAD_FMT_CLASSIC_ACC_DPD :
      case WPAD_FMT_GUITAR :
        // Core ACC is OK
        break ;

      default :
        return ;
    }

    //----- Update raw values
    kp->hard_acc.x = clamp_acc( (real32)(int32)-uwp->u.core.accX * kp->acc_scale_x, kp_rm_acc_max ) ;
    kp->hard_acc.y = clamp_acc( (real32)(int32)-uwp->u.core.accZ * kp->acc_scale_z, kp_rm_acc_max ) ;
    kp->hard_acc.z = clamp_acc( (real32)(int32) uwp->u.core.accY * kp->acc_scale_y, kp_rm_acc_max ) ;

    //----- Temporary save
    vec = sp->acc ;

    //----- Acceleration tracking process for application use
    calc_acc( kp, &sp->acc.x, kp->hard_acc.x ) ;
    calc_acc( kp, &sp->acc.y, kp->hard_acc.y ) ;
    calc_acc( kp, &sp->acc.z, kp->hard_acc.z ) ;
    sp->acc_value = sqrtf( sp->acc.x * sp->acc.x + sp->acc.y * sp->acc.y + sp->acc.z * sp->acc.z ) ;

    //----- Amount of change in acceleration, for application use
    vec.x -= sp->acc.x ;
    vec.y -= sp->acc.y ;
    vec.z -= sp->acc.z ;
    sp->acc_speed = sqrtf( vec.x * vec.x + vec.y * vec.y + vec.z * vec.z ) ;

    //----- Calculate controller tilt from the raw acceleration
    calc_acc_horizon( kp ) ;
    calc_acc_vertical( kp ) ;


    /***********************************************************************
            Load Nunchuk unit acceleration
    ***********************************************************************/
    if ( uwp->u.fs.err != WPAD_ERR_NONE ||
         uwp->u.fs.dev != WPAD_DEV_FREESTYLE ||
        (uwp->fmt != WPAD_FMT_FREESTYLE_ACC &&
         uwp->fmt != WPAD_FMT_FREESTYLE_ACC_DPD) ) {
        return ;
    }

    fsrc.x = clamp_acc( (real32)(int32)-uwp->u.fs.fsAccX * kp->fs_acc_scale_x, kp_fs_acc_max ) ;
    fsrc.y = clamp_acc( (real32)(int32)-uwp->u.fs.fsAccZ * kp->fs_acc_scale_z, kp_fs_acc_max ) ;
    fsrc.z = clamp_acc( (real32)(int32) uwp->u.fs.fsAccY * kp->fs_acc_scale_y, kp_fs_acc_max ) ;

    if ( kp->fsAccRevise ) {
        //MTXMultVec( kp_fs_rot, &fsrc, &fsrc ) ;
       CVector3 &_fsrc = reinterpret_cast<CVector3 &>(fsrc);
       _fsrc = kp_fs_rot * _fsrc;
    }

    //----- Temporary save
    vec = sp->ex_status.fs.acc ;

    //----- Acceleration tracking process for application use
    calc_acc( kp, &sp->ex_status.fs.acc.x, fsrc.x ) ;
    calc_acc( kp, &sp->ex_status.fs.acc.y, fsrc.y ) ;
    calc_acc( kp, &sp->ex_status.fs.acc.z, fsrc.z ) ;
    sp->ex_status.fs.acc_value = sqrtf( sp->ex_status.fs.acc.x * sp->ex_status.fs.acc.x + sp->ex_status.fs.acc.y * sp->ex_status.fs.acc.y + sp->ex_status.fs.acc.z * sp->ex_status.fs.acc.z ) ;

    //----- Amount of change in acceleration, for application use
    vec.x -= sp->ex_status.fs.acc.x ;
    vec.y -= sp->ex_status.fs.acc.y ;
    vec.z -= sp->ex_status.fs.acc.z ;
    sp->ex_status.fs.acc_speed = sqrtf( vec.x * vec.x + vec.y * vec.y + vec.z * vec.z ) ;
}


/*******************************************************************************
        Change WPAD object to KPAD
 *******************************************************************************/
static void get_kobj( KPADInsideStatus *kp, DPDObject *wobj_p )
{
    const real32       dpd_scale = 2.0f / (real32)KPAD_DPD_RESO_WX ;
    const real32       dpd_cx = (real32)( KPAD_DPD_RESO_WX - 1 ) / (real32)KPAD_DPD_RESO_WX ;
    const real32       dpd_cy = (real32)( KPAD_DPD_RESO_WY - 1 ) / (real32)KPAD_DPD_RESO_WX ;

    KPADObject      *kobj_p ;

    //----- Store
    kobj_p = &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS - 1 ] ;
    do {
        if ( wobj_p->size ) {
            //----- Valid object
            kobj_p->center.x = (real32)(int32)wobj_p->x * dpd_scale - dpd_cx ;
            kobj_p->center.y = (real32)(int32)wobj_p->y * dpd_scale - dpd_cy ;

            kobj_p->error_fg = 0 ;  // Applied
            kobj_p->state_fg = 0 ;  // Normal
        } else {
            //----- Invalid object
            kobj_p->error_fg = -1 ; // Not applied
        }

        -- wobj_p ;
    } while ( --kobj_p >= kp->kobj_sample ) ;
}


/*******************************************************************************
        Set Surrounding Objects to Invalid
 *******************************************************************************/
static void check_kobj_outside_frame( KPADInsideStatus *kp, KPADObject *kobj_t )
{
    KPADObject      *kobj_p = &kobj_t[ WPAD_DPD_MAX_OBJECTS - 1 ] ;

    do {
        if ( kobj_p->error_fg < 0 ) continue ;

        if ( kobj_p->center.x <= kp->kobj_frame_min.x || kobj_p->center.x >= kp->kobj_frame_max.x ||
             kobj_p->center.y <= kp->kobj_frame_min.y || kobj_p->center.y >= kp->kobj_frame_max.y ) {
            kobj_p->error_fg |= 1 ;
        }
    } while ( --kobj_p >= kobj_t ) ;
}


/*******************************************************************************
        Set Objects at Same Coordinates to Invalid
 *******************************************************************************/
static void check_kobj_same_position( KPADObject *kobj_t )
{
    KPADObject      *op1, *op2 ;


    op1 = kobj_t ;
    do {
        if ( op1->error_fg != 0 ) continue ;

        op2 = op1 + 1 ;
        do {
            if ( op2->error_fg != 0 ) continue ;

            if ( op1->center.x == op2->center.x && op1->center.y == op2->center.y ) {
                op2->error_fg |= 2 ;    // Set just one as error
            }
        } while ( ++op2 <= &kobj_t[ WPAD_DPD_MAX_OBJECTS - 1 ] ) ;
    } while ( ++op1 < &kobj_t[ WPAD_DPD_MAX_OBJECTS - 1 ] ) ;
}


/*******************************************************************************
        Calculate Controller Tilt from Two Points (return the distance from TV)
 *******************************************************************************/
static real32 calc_horizon( KPADInsideStatus *kp, Vec2 *p1, Vec2 *p2, Vec2 *hori )
{
    real32             f1, vx,vy ;


    vx = p2->x - p1->x ;
    vy = p2->y - p1->y ;
    f1 = 1.0f / sqrtf( vx * vx + vy * vy ) ;        // Should not be zero
    vx *= f1 ;
    vy *= f1 ;

    hori->x = kp->sec_nrm_hori.x * vx + kp->sec_nrm_hori.y * vy ;
    hori->y = kp->sec_nrm_hori.y * vx - kp->sec_nrm_hori.x * vy ;

    return ( kp->dist_vv1 * f1 ) ;
}


/*******************************************************************************
        Select Two Marks for the First Time
 *******************************************************************************/
static int8 select_2obj_first( KPADInsideStatus *kp )
{
    KPADObject      *op1,*op2, *rp1,*rp2 ;
    Vec2            hori ;
    real32             f1, max = kp_err_first_inpr ;

    op1 = kp->kobj_sample ;
    do {
        if ( op1->error_fg != 0 ) continue ;

        op2 = op1 + 1 ;
        do {
            if ( op2->error_fg != 0 ) continue ;

            f1 = calc_horizon( kp, &op1->center, &op2->center, &hori ) ;

            //----- Operational distance range check
            if ( f1 <= kp->err_dist_min || f1 >= kp_err_dist_max ) continue ;

            f1 = kp->acc_horizon.x * hori.x + kp->acc_horizon.y * hori.y ;
            if ( f1 < 0.0f ) {
                if ( -f1 > max ) {
                    max = -f1 ;
                    rp1 = op2 ;
                    rp2 = op1 ;
                }
            } else {
                if ( f1 > max ) {
                    max = f1 ;
                    rp1 = op1 ;
                    rp2 = op2 ;
                }
            }

        } while ( ++op2 <= &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS - 1 ] ) ;
    } while ( ++op1 < &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS - 1 ] ) ;

    //----- Confirmed regular mark?
    if ( max == kp_err_first_inpr ) return ( 0 ) ;

    kp->kobj_regular[ 0 ] = *rp1 ;
    kp->kobj_regular[ 1 ] = *rp2 ;

    return ( 2 ) ;            // Two points recognition
}


/*******************************************************************************
        Continuing, select two marks using only the interval information
 *******************************************************************************/
static int8 select_2obj_continue( KPADInsideStatus *kp )
{
    KPADObject      *op1,*op2, *rp1,*rp2 ;
    Vec2            nrm ;
    int32             rev_fg ;
    real32             f1,f2, vx,vy, min = 2.0f ;


    //----- Find two points closest to the previous tilt and distance
    op1 = kp->kobj_sample ;
    do {
        if ( op1->error_fg != 0 ) continue ;

        op2 = op1 + 1 ;
        do {
            if ( op2->error_fg != 0 ) continue ;

            //----- Direction calculation
            vx = op2->center.x - op1->center.x ;
            vy = op2->center.y - op1->center.y ;
            f1 = 1.0f / sqrtf( vx*vx + vy*vy ) ;    // Should not be zero.
            nrm.x = vx * f1 ;
            nrm.y = vy * f1 ;

            //----- Operational distance range check
            f1 *= kp->dist_vv1 ;             // Distance
            if ( f1 <= kp->err_dist_min || f1 >= kp_err_dist_max ) continue ;

            //----- Check amount of change in distance
            f1 -= kp->sec_dist ;
            if ( f1 < 0.0f ) {
                f1 *= kp->err_dist_speedM_1 ;
            } else {
                f1 *= kp->err_dist_speed_1 ;
            }
            if ( f1 >= 1.0f ) continue ;    // Distance error rate

            //----- Check amount of change in tilt
            f2 = kp->sec_nrm.x * nrm.x + kp->sec_nrm.y * nrm.y ;
            if ( f2 < 0.0f ) {
                f2 = -f2 ;
                rev_fg = 1 ;    // Handle with orientation inverted (op2 -> op1)
            } else {
                rev_fg = 0 ;    // Handle as is (op1 -> op2)
            }
            if ( f2 <= kp_err_next_inpr ) continue ;
            f2 = ( 1.0f - f2 ) / ( 1.0f - kp_err_next_inpr ) ;      // Tilt error rate

            //----- Record the object with the smallest error
            f1 += f2 ;      // Determine through the sum of distance error rate and tilt error rate
            if ( f1 < min ) {
                min = f1 ;
                if ( rev_fg ) {
                    rp1 = op2 ;
                    rp2 = op1 ;
                } else {
                    rp1 = op1 ;
                    rp2 = op2 ;
                }
            }

        } while ( ++op2 <= &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS - 1 ] ) ;
    } while ( ++op1 < &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS - 1 ] ) ;

    //----- Confirmed regular mark?
    if ( min == 2.0f ) return ( 0 ) ;

    kp->kobj_regular[ 0 ] = *rp1 ;
    kp->kobj_regular[ 1 ] = *rp2 ;

    return ( 2 ) ;            // Two points recognition
}


/*******************************************************************************
        Select One Mark for First Time
 *******************************************************************************/
static int8 select_1obj_first( KPADInsideStatus *kp )
{
    KPADObject      *op1 ;
    real32             vx,vy ;
    Vec2            p1,p2 ;


    //----- Determine the section direction
    vx = kp->sec_nrm_hori.x * kp->acc_horizon.x + kp->sec_nrm_hori.y * kp->acc_horizon.y ;
    vy = kp->sec_nrm_hori.y * kp->acc_horizon.x - kp->sec_nrm_hori.x * kp->acc_horizon.y ;

    //----- Determine the section vector
    vx *= kp->trust_sec_length ;
    vy *= kp->trust_sec_length ;

    //----- Search for a point where the expected point can be outside
    op1 = kp->kobj_sample ;
    do {
        if ( op1->error_fg != 0 ) continue ;

        p1.x = op1->center.x - vx ;     // Expected point is to the left
        p1.y = op1->center.y - vy ;
        p2.x = op1->center.x + vx ;     // Expected point is to the right
        p2.y = op1->center.y + vy ;

        if ( p1.x <= kp->kobj_frame_min.x || p1.x >= kp->kobj_frame_max.x ||
             p1.y <= kp->kobj_frame_min.y || p1.y >= kp->kobj_frame_max.y ) {
            //----- If the left expected point is outside, the right expected point needs to be inside
            if ( p2.x > kp->kobj_frame_min.x && p2.x < kp->kobj_frame_max.x &&
                 p2.y > kp->kobj_frame_min.y && p2.y < kp->kobj_frame_max.y ) {
                //----- op1 may be the right mark
                kp->kobj_regular[ 1 ] = *op1 ;

                kp->kobj_regular[ 0 ].center = p1 ;
                kp->kobj_regular[ 0 ].error_fg = 0 ;
                kp->kobj_regular[ 0 ].state_fg = -1 ;

                return ( -1 ) ;   // Recognize one unstable point
            }
        } else {
            //----- If the left expected point is inside, the right expected point needs to be outside.
            if ( p2.x <= kp->kobj_frame_min.x || p2.x >= kp->kobj_frame_max.x ||
                 p2.y <= kp->kobj_frame_min.y || p2.y >= kp->kobj_frame_max.y ) {
                //----- op1 may be the left mark
                kp->kobj_regular[ 0 ] = *op1 ;

                kp->kobj_regular[ 1 ].center = p2 ;
                kp->kobj_regular[ 1 ].error_fg = 0 ;
                kp->kobj_regular[ 1 ].state_fg = -1 ;

                return ( -1 ) ;   // Recognize one unstable point
            }
        }

    } while ( ++op1 < &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS ] ) ;

    return ( 0 ) ;
}


/*******************************************************************************
        Select One Mark Subsequently
 *******************************************************************************/
static int8 select_1obj_continue( KPADInsideStatus *kp )
{
    KPADObject      *op1,*op2, *rp1,*rp2 ;
    real32             f1, vx,vy ;
    real32             min = kp_err_near_pos * kp_err_near_pos ;


    //----- Select the mark closest to the past regular mark
    op1 = kp->kobj_regular ;
    do {
        if ( op1->error_fg != 0 ) continue ;
        if ( op1->state_fg != 0 ) continue ;    // No expected points

        op2 = kp->kobj_sample ;
        do {
            if ( op2->error_fg != 0 ) continue ;

            vx = op1->center.x - op2->center.x ;
            vy = op1->center.y - op2->center.y ;
            f1 = vx * vx + vy * vy ;
            if ( f1 < min ) {
                min = f1 ;
                rp1 = op1 ;
                rp2 = op2 ;
            }
        } while ( ++op2 < &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS ] ) ;
    } while ( ++op1 < &kp->kobj_regular[ KPAD_USE_OBJECTS ] ) ;

    //----- Confirmed regular mark?
    if ( min == kp_err_near_pos * kp_err_near_pos ) return ( 0 ) ;

    *rp1 = *rp2 ;

    //----- Calculate tilt from the acceleration
    kp->sec_nrm.x = kp->sec_nrm_hori.x * kp->acc_horizon.x + kp->sec_nrm_hori.y * kp->acc_horizon.y ;
    kp->sec_nrm.y = kp->sec_nrm_hori.y * kp->acc_horizon.x - kp->sec_nrm_hori.x * kp->acc_horizon.y ;

    //----- Calculate expected point coordinates
    vx = kp->sec_length * kp->sec_nrm.x ;
    vy = kp->sec_length * kp->sec_nrm.y ;
    if ( rp1 == &kp->kobj_regular[ 0 ] ) {
        kp->kobj_regular[ 1 ].center.x = rp1->center.x + vx ;
        kp->kobj_regular[ 1 ].center.y = rp1->center.y + vy ;
        kp->kobj_regular[ 1 ].error_fg = 0 ;
        kp->kobj_regular[ 1 ].state_fg = -1 ;
    } else {
        kp->kobj_regular[ 0 ].center.x = rp1->center.x - vx ;
        kp->kobj_regular[ 0 ].center.y = rp1->center.y - vy ;
        kp->kobj_regular[ 0 ].error_fg = 0 ;
        kp->kobj_regular[ 0 ].state_fg = -1 ;
    }

    if ( kp->status.dpd_valid_fg < 0 ) {
        return ( -1 ) ;   // Recognize one unstable point
    } else {
        return ( 1 ) ;    // Recognize one point
    }
}


/*******************************************************************************
        Calculate Controller Tilt from Object
 *******************************************************************************/
static void calc_obj_horizon( KPADInsideStatus *kp )
{
    real32             f1, vx,vy ;


    vx = kp->kobj_regular[ 1 ].center.x - kp->kobj_regular[ 0 ].center.x ;
    vy = kp->kobj_regular[ 1 ].center.y - kp->kobj_regular[ 0 ].center.y ;
    kp->sec_length = sqrtf( vx * vx + vy * vy ) ;   // Should not be zero

    f1 = 1.0f / kp->sec_length ;
    kp->sec_dist = kp->dist_vv1 * f1 ;

    kp->sec_nrm.x = ( vx *= f1 ) ;
    kp->sec_nrm.y = ( vy *= f1 ) ;

    kp->obj_horizon.x = kp->sec_nrm_hori.x * vx + kp->sec_nrm_hori.y * vy ;
    kp->obj_horizon.y = kp->sec_nrm_hori.y * vx - kp->sec_nrm_hori.x * vy ;
}


/*******************************************************************************
        Update Application Variables
 *******************************************************************************/
static void calc_dpd_variable( KPADInsideStatus *kp, int8 valid_fg_next )
{
    KPADStatus      *sp = &kp->status ;
    real32             f1,f2, dist ;
    Vec2            pos, vec ;


    if ( valid_fg_next == 0 ) {
        sp->dpd_valid_fg = 0 ;
        return ;
    }

    /***********************************************************************
                Calculate Controller Tilt
     ***********************************************************************/
    //----- Calculate the target value
    pos.x = kp->sec_nrm_hori.x * kp->sec_nrm.x + kp->sec_nrm_hori.y * kp->sec_nrm.y ;
    pos.y = kp->sec_nrm_hori.y * kp->sec_nrm.x - kp->sec_nrm_hori.x * kp->sec_nrm.y ;

    //----- Consider the tracking sensitivity and play tolerance for the target value
    if ( sp->dpd_valid_fg == 0 ) {
        //----- Because this is the first pointing, initialize with the given values
        sp->horizon = pos ;
        sp->hori_vec = Vec2_0 ;
        sp->hori_speed = 0.0f ;
    } else {
        //----- Difference to the target value
        vec.x = pos.x - sp->horizon.x ;
        vec.y = pos.y - sp->horizon.y ;
        f1 = sqrtf( vec.x * vec.x + vec.y * vec.y ) ;

        if ( kp->hori_play_mode == KPAD_PLAY_MODE_LOOSE ) {
            //----- Calculation of tracking rate inside/outside the play tolerance
            if ( f1 >= kp->hori_play_radius ) {
                //----- Apply 100% tracking sensitivity if outside play tolerance
                f1 = 1.0f ;
            } else {
                //----- If inside play tolerance, weaken tracking sensitivity as target gets closer
                f1 /= kp->hori_play_radius ;
                f1 *= f1 ;      // Second power
                f1 *= f1 ;      // Fourth power
            }
            f1 *= kp->hori_sensitivity ;

            //----- Tracking
            vec.x = f1 * vec.x + sp->horizon.x ;
            vec.y = f1 * vec.y + sp->horizon.y ;
            f1 = sqrtf( vec.x * vec.x + vec.y * vec.y ) ;   // Normalize because this is tilt
            vec.x /= f1 ;
            vec.y /= f1 ;

            sp->hori_vec.x = vec.x - sp->horizon.x ;
            sp->hori_vec.y = vec.y - sp->horizon.y ;
            sp->hori_speed = sqrtf( sp->hori_vec.x * sp->hori_vec.x + sp->hori_vec.y * sp->hori_vec.y ) ;

            sp->horizon = vec ;
        } else {
            if ( f1 > kp->hori_play_radius ) {
                //----- Track because this is outside the play tolerance
                f1 = ( f1 - kp->hori_play_radius ) / f1 * kp->hori_sensitivity ;
                vec.x = vec.x * f1 + sp->horizon.x ;
                vec.y = vec.y * f1 + sp->horizon.y ;
                f1 = sqrtf( vec.x * vec.x + vec.y * vec.y ) ;
                vec.x /= f1 ;
                vec.y /= f1 ;

                sp->hori_vec.x = vec.x - sp->horizon.x ;
                sp->hori_vec.y = vec.y - sp->horizon.y ;
                sp->hori_speed = sqrtf( sp->hori_vec.x * sp->hori_vec.x + sp->hori_vec.y * sp->hori_vec.y ) ;

                sp->horizon = vec ;
            } else {
                //----- Do not move this because it is inside the play tolerance
                sp->hori_vec = Vec2_0 ;
                sp->hori_speed = 0.0f ;
            }
        }
    }

    /***********************************************************************
                Calculate Distance from TV
     ***********************************************************************/
    //----- Calculate the target value
    dist = kp->dist_vv1 / kp->sec_length ;

    //----- Consider the tracking sensitivity and play tolerance for the target value
    if ( sp->dpd_valid_fg == 0 ) {
        //----- Because this is the first pointing, initialize with the given values
        sp->dist = dist ;
        sp->dist_vec = 0.0f ;
        sp->dist_speed = 0.0f ;
    } else {
        //----- Difference to the target value
        f2 = dist - sp->dist ;
        if ( f2 < 0.0f ) {
            f1 = -f2 ;
        } else {
            f1 = f2 ;
        }

        if ( kp->dist_play_mode == KPAD_PLAY_MODE_LOOSE ) {
            //----- Calculation of tracking rate inside/outside the play tolerance
            if ( f1 >= kp->dist_play_radius ) {
                //----- Apply 100% tracking sensitivity if outside play tolerance
                f1 = 1.0f ;
            } else {
                //----- If inside play tolerance, weaken tracking sensitivity as target gets closer
                f1 /= kp->dist_play_radius ;
                f1 *= f1 ;      // Second power
                f1 *= f1 ;      // Fourth power
            }
            f1 *= kp->dist_sensitivity ;

            //----- Tracking
            sp->dist_vec = f1 * f2 ;
            if ( sp->dist_vec < 0.0f ) {
                sp->dist_speed = -sp->dist_vec ;
            } else {
                sp->dist_speed = sp->dist_vec ;
            }

            sp->dist += sp->dist_vec ;
        } else {
            if ( f1 > kp->dist_play_radius ) {
                //----- Track because this is outside the play tolerance
                f1 = ( f1 - kp->dist_play_radius ) / f1 * kp->dist_sensitivity ;
                sp->dist_vec = f1 * f2 ;
                if ( sp->dist_vec < 0.0f ) {
                    sp->dist_speed = -sp->dist_vec ;
                } else {
                    sp->dist_speed = sp->dist_vec ;
                }

                sp->dist += sp->dist_vec ;
            } else {
                //----- Do not move this because it is inside the play tolerance
                sp->dist_vec = 0.0f ;
                sp->dist_speed = 0.0f ;
            }
        }
    }

    /***********************************************************************
                Calculate Pointing Location
     ***********************************************************************/
    //----- Center coordinates of two marks
    pos.x = ( kp->kobj_regular[ 0 ].center.x + kp->kobj_regular[ 1 ].center.x ) * 0.5f ;
    pos.y = ( kp->kobj_regular[ 0 ].center.y + kp->kobj_regular[ 1 ].center.y ) * 0.5f ;

    //----- Rotate by the amount of twist
    f1 =  kp->sec_nrm.x * kp->sec_nrm_hori.x + kp->sec_nrm.y * kp->sec_nrm_hori.y ;
    f2 = -kp->sec_nrm.y * kp->sec_nrm_hori.x + kp->sec_nrm.x * kp->sec_nrm_hori.y ;
    vec.x = f1 * pos.x - f2 * pos.y ;
    vec.y = f2 * pos.x + f1 * pos.y ;

    //----- Apply scaling after correcting the center position
    vec.x = ( kp->center_org.x - vec.x ) * kp->dpd2pos_scale ;
    vec.y = ( kp->center_org.y - vec.y ) * kp->dpd2pos_scale ;

    //----- Convert to the gravitational direction coordinate system during calibration
    pos.x = -kp->accXY_nrm_hori.y * vec.x + kp->accXY_nrm_hori.x * vec.y ;
    pos.y = -kp->accXY_nrm_hori.x * vec.x - kp->accXY_nrm_hori.y * vec.y ;

    //----- Consider the tracking sensitivity and play tolerance for the target value
    if ( sp->dpd_valid_fg == 0 ) {
        //----- Because this is the first pointing, initialize with the given values
        sp->pos = pos ;
        sp->vec = Vec2_0 ;
        sp->speed = 0.0f ;
    } else {
        //----- Difference to the target value
        vec.x = pos.x - sp->pos.x ;
        vec.y = pos.y - sp->pos.y ;
        f1 = sqrtf( vec.x * vec.x + vec.y * vec.y ) ;

        if ( kp->pos_play_mode == KPAD_PLAY_MODE_LOOSE ) {
            //----- Calculation of tracking rate inside/outside the play tolerance
            if ( f1 >= kp->pos_play_radius ) {
                //----- Apply 100% tracking sensitivity if outside play tolerance
                f1 = 1.0f ;
            } else {
                //----- If inside play tolerance, weaken tracking sensitivity as target gets closer
                f1 /= kp->pos_play_radius ;
                f1 *= f1 ;      // Second power
                f1 *= f1 ;      // Fourth power
            }
            f1 *= kp->pos_sensitivity ;

            //----- Tracking
            sp->vec.x = f1 * vec.x ;
            sp->vec.y = f1 * vec.y ;
            sp->speed = sqrtf( sp->vec.x * sp->vec.x + sp->vec.y * sp->vec.y ) ;

            sp->pos.x += sp->vec.x ;
            sp->pos.y += sp->vec.y ;
        } else {
            if ( f1 > kp->pos_play_radius ) {
                //----- Track because this is outside the play tolerance
                f1 = ( f1 - kp->pos_play_radius ) / f1 * kp->pos_sensitivity ;
                sp->vec.x = f1 * vec.x ;
                sp->vec.y = f1 * vec.y ;
                sp->speed = sqrtf( sp->vec.x * sp->vec.x + sp->vec.y * sp->vec.y ) ;

                sp->pos.x += sp->vec.x ;
                sp->pos.y += sp->vec.y ;
            } else {
                //----- Do not move this because it is inside the play tolerance
                sp->vec = Vec2_0 ;
                sp->speed = 0.0f ;
            }
        }
    }

    /***********************************************************************
                Update Flags
     ***********************************************************************/
    sp->dpd_valid_fg = valid_fg_next ;
}


/*******************************************************************************
        KPAD DPD Information Loading
 *******************************************************************************/
static void read_kpad_dpd( KPADInsideStatus *kp, KPADUnifiedWpadStatus *uwp )
{
    KPADStatus      *sp = &kp->status ;
    KPADObject      *op1 ;
    int8              valid_fg_next ;

    /***********************************************************************
                Change WPAD object to KPAD
     ***********************************************************************/
    if ( uwp->fmt == WPAD_FMT_CORE_ACC_DPD 
    ||   uwp->fmt == WPAD_FMT_FREESTYLE_ACC_DPD 
    ||   uwp->fmt == WPAD_FMT_CLASSIC_ACC_DPD 
    ||   ( uwp->fmt == WPAD_FMT_GUITAR && kp->dpdCurrState ) ) {
        //----- Change the WPAD object to KPAD
        get_kobj( kp, &uwp->u.core.obj[ WPAD_DPD_MAX_OBJECTS - 1 ] ) ;
    } else {
        // dpd data is not prepared
        op1 = &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS - 1 ] ;
        do {
            op1->error_fg = -1 ;
        } while ( --op1 >= kp->kobj_sample ) ;
    }

    /***********************************************************************
                Select the normal object
     ***********************************************************************/
    //----- Remove untrustworthy objects
    check_kobj_outside_frame( kp, kp->kobj_sample ) ;   // Set Surrounding Objects to Invalid
    check_kobj_same_position( kp->kobj_sample ) ;   // Set the objects at the same coordinates to invalid

    //----- Determine how many are being applied
    kp->valid_objs = 0 ;
    op1 = &kp->kobj_sample[ WPAD_DPD_MAX_OBJECTS - 1 ] ;
    do {
        if ( op1->error_fg == 0 ) ++ kp->valid_objs ;
    } while ( --op1 >= kp->kobj_sample ) ;

    //----- Recognition processing
    if ( sp->acc_vertical.x <= kp_err_up_inpr ) goto LABEL_select_NG ;

    if ( sp->dpd_valid_fg == 2 || sp->dpd_valid_fg == -2 ) {
        //----- Recognized using two objects from the previous time
        if ( kp->valid_objs >= 2 ) 
        {
            valid_fg_next = select_2obj_continue( kp ) ;
            if ( valid_fg_next ) goto LABEL_select_OK ;
        }
        if ( kp->valid_objs >= 1 ) 
        {
            valid_fg_next = select_1obj_continue( kp ) ;
            if ( valid_fg_next ) goto LABEL_select_OK ;
        }
    } else if ( sp->dpd_valid_fg == 1 || sp->dpd_valid_fg == -1 ) {
        //----- Recognized using one object from the previous time
        if ( kp->valid_objs >= 2 ) 
        {
            valid_fg_next = select_2obj_first( kp ) ;
            if ( valid_fg_next ) goto LABEL_select_OK ;
        }
        if ( kp->valid_objs >= 1 ) 
        {
            valid_fg_next = select_1obj_continue( kp ) ;
            if ( valid_fg_next ) goto LABEL_select_OK ;
        }
    } else {
        //----- Not recognized the previous time
        if ( kp->valid_objs >= 2 ) 
        {
            valid_fg_next = select_2obj_first( kp ) ;

            if ( valid_fg_next ) goto LABEL_select_OK ;
        }
        if ( kp->valid_objs == 1 ) 
        {
            valid_fg_next = select_1obj_first( kp ) ;
            if ( valid_fg_next ) goto LABEL_select_OK ;
        }
    }

LABEL_select_NG :

    valid_fg_next = 0 ;     // Was not able to select

LABEL_select_OK :           // Maybe was able to select

    //----- Update section information if selected
    if ( valid_fg_next ) {
        //----- Calculate the information of two points and object tilt
        calc_obj_horizon( kp ) ;

        //----- Error if obviously different from the acceleration tilt
        if ( kp->ah_circle_ct == 0 ) {
            if ( kp->obj_horizon.x * kp->acc_horizon.x + kp->obj_horizon.y * kp->acc_horizon.y <= kp_err_acc_inpr ) {
                valid_fg_next = 0 ;     // Invalid after all

                kp->kobj_regular[ 0 ].error_fg =
                  kp->kobj_regular[ 1 ].error_fg = 1 ;
            }
        }

        //----- Consecutive two point recognition count
        if ( sp->dpd_valid_fg == 2 && valid_fg_next == 2 ) {
            if ( kp->dpd_valid2_ct == 200 ) {
                kp->trust_sec_length = kp->sec_length ;
            } else {
                ++ kp->dpd_valid2_ct ;
            }
        } else {
            kp->dpd_valid2_ct = 0 ;
        }
    } else {
        kp->dpd_valid2_ct = 0 ;
    }

    //----- Update application variables
    calc_dpd_variable( kp, valid_fg_next ) ;
}


/*******************************************************************************
        Clamp Processing of Analog Triggers
*******************************************************************************/
static void clamp_trigger( real32 *trigger, int32 tr, int32 min, int32 max )
{
    if ( tr <= min ) {
        *trigger = 0.0f ;
    } else if ( tr >= max ) {
        *trigger = 1.0f ;
    } else {
        *trigger = (real32)( tr - min ) / (real32)( max - min ) ;
    }
}


/*******************************************************************************
        Clamp Processing of Sticks
*******************************************************************************/
static void clamp_stick_circle( Vec2 *stick, int32 sx, int32 sy, int32 min, int32 max )
{
    real32     length ;
    real32     fx = (real32)sx ;
    real32     fy = (real32)sy ;
    real32     fmin = (real32)min ;
    real32     fmax = (real32)max ;


    length = sqrtf( fx * fx + fy * fy ) ;

    if ( length <= fmin ) {
        stick->x = stick->y = 0.0f ;

    } else if ( length >= fmax ) {
        stick->x = fx / length ;
        stick->y = fy / length ;

    } else {
        length = ( length - fmin ) / ( fmax - fmin ) / length ;
        stick->x = fx * length ;
        stick->y = fy * length ;
    }
}

static void clamp_stick_cross( Vec2 *stick, int32 sx, int32 sy, int32 min, int32 max )
{
    real32     length ;


    //----- Process clamp for each axis separately
    if ( sx < 0 ) {
        clamp_trigger( &stick->x, -sx, min, max ) ;
        stick->x = -stick->x ;
    } else {
        clamp_trigger( &stick->x, sx, min, max ) ;
    }
    if ( sy < 0 ) {
        clamp_trigger( &stick->y, -sy, min, max ) ;
        stick->y = -stick->y ;
    } else {
        clamp_trigger( &stick->y, sy, min, max ) ;
    }

    //----- Set maximum length to 1
    length = stick->x * stick->x + stick->y * stick->y ;
    if ( length > 1.0f ) {
        length = sqrtf( length ) ;
        stick->x /= length ;
        stick->y /= length ;
    }
}

/*******************************************************************************
        Load stick information
*******************************************************************************/
static void read_kpad_ext( KPADInsideStatus *kp, KPADUnifiedWpadStatus *uwp )
{
    KPADEXStatus *ep = &kp->status.ex_status ;
    void (*clampStickFuncp)( Vec2 *stick, int32 sx, int32 sy, int32 min, int32 max ) ;
    int idx ;

    clampStickFuncp = kp_stick_clamp_cross ? clamp_stick_cross : clamp_stick_circle ;

    if ( uwp->u.fs.dev == WPAD_DEV_FREESTYLE 
    &&  (uwp->fmt == WPAD_FMT_FREESTYLE 
    ||   uwp->fmt == WPAD_FMT_FREESTYLE_ACC 
    ||   uwp->fmt == WPAD_FMT_FREESTYLE_ACC_DPD) ) {

        if ( kp->exResetReq ) {
            kp->exResetReq = FALSE ;

            //----- Nunchuk unit
            ep->fs.stick = Vec2_0 ;
            ep->fs.acc.x = ep->fs.acc.z = 0.0f ;
            ep->fs.acc.y = -1.0f ;
            ep->fs.acc_value = 1.0f ;
            ep->fs.acc_speed = 0.0f ;
        }

        //----- Stick data processing
        clampStickFuncp( &ep->fs.stick, uwp->u.fs.fsStickX, uwp->u.fs.fsStickY, kp_fs_fstick_min, kp_fs_fstick_max ) ;
    }

}

/*******************************************************************************
        READ KPAD (return the number of loaded buffers)
 *******************************************************************************/
int32 KPADRead( int32 chan, KPADStatus samplingBufs[], uint32 length )
{
    return KPADiRead( chan, samplingBufs, length, NULL, FALSE ) ;
}

int32 KPADReadEx( int32 chan, KPADStatus samplingBufs[], uint32 length, int32 *err )
{
    return KPADiRead( chan, samplingBufs, length, err, TRUE ) ;
}

static int32 KPADiRead( int32 chan, KPADStatus samplingBufs[], uint32 length, int32 *err, BOOL keep )
{
    // This assumption is confirmed in the KPADInit function
    // (sizeof(KPADTmpStatus) == sizeof(KPADStatus))
    KPADTmpStatus    *tp = (KPADTmpStatus *)samplingBufs ;
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;
    KPADStatus             tmp ;
    KPADUnifiedWpadStatus *uwp ;
    int32     wpad_err ;
    int32     kpad_err = KPAD_READ_ERR_NONE ;
    int32     idx ;
    int32     copy_ct ;
    int32     return_ct = 0 ;
    uint32     bufCount ;
    BOOL    enabled ;
    uint32     lastCoreButton ;
    uint32     lastFsButton ;
    uint32     lastClButton ;
    uint32     lastDev ;
    BOOL    corrupted ;
    BOOL    changed ;

    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;

    //----- Do not process if the KPADInit function has not been called
    if ( !kp_initialized ) {
        kpad_err = KPAD_READ_ERR_INIT ;
        goto read_finish ;
    }
    //----- Do not process until WPAD library initialization is complete
    if ( WPADGetStatus() != WPAD_STATE_SETUP ) {
        kpad_err = KPAD_READ_ERR_SETUP ;
        goto read_finish ;
    }
    //----- Do not process this if this was called from another thread mid-process
    enabled = OSDisableInterrupts() ;
    if ( kp->readLocked ) {
        (void)OSRestoreInterrupts( enabled ) ;
        kpad_err = KPAD_READ_ERR_LOCKED ;
        goto read_finish ;
    }
    kp->readLocked = TRUE ;

    wpad_err = WPADProbe( chan, NULL ) ;
    if ( wpad_err == WPAD_ERR_NO_CONTROLLER ) {
        //----- Reset internal parameters
        reset_kpad( kp ) ;
        
        if ( kp->dpd_ctrl_callback &&
             kp->dpdPreCallbackDone &&
             !kp->dpdPostCallbackDone ) {
            KPADiControlDpdCallback( chan, WPAD_ERR_NONE ) ;
        }
        //----- Remove locks
        kp->readLocked = FALSE ;
        (void)OSRestoreInterrupts( enabled ) ;
        kpad_err = KPAD_READ_ERR_NO_CONTROLLER ;
        goto read_finish ;
    }

    (void)OSRestoreInterrupts( enabled ) ;

    if ( kp->resetReq ) {
        // Data is invalid when a reset occurs
        kp->status.wpad_err = (int8)WPAD_ERR_INVALID ;
        reset_kpad( kp ) ;
    }

    //----- Copy the first element of the buffer that was passed in
    tmp = samplingBufs[ 0 ] ;

    if ( kp->bufCount > 1 &&
         samplingBufs &&
         length ) {
        enabled = OSDisableInterrupts() ;

        //----- Check the number of times that data has been saved to the buffer
        bufCount = kp->bufCount ;
        if ( bufCount > length ) {
            bufCount = length ;
        }
        copy_ct = (int32)bufCount ;

        //----- Flag initialization
        corrupted = FALSE ;
        changed   = FALSE ;
        //----- Make the index match the most recent data
        idx = (int32) kp->bufIdx ;
        idx-- ;
        if ( idx < 0 ) {
            idx += kp->uniRingBufExLen + KPAD_RING_BUFS ;
        }
        //----- Get the most recent device
        if ( idx >= KPAD_RING_BUFS ) {
            uwp = &kp->uniRingBufEx[ idx - KPAD_RING_BUFS ] ;
        } else {
            uwp = &kp->uniRingBuf[ idx ] ;
        }
        lastDev = uwp->u.core.dev ;
        
        do {
            // The data located immediately before the corrupted data may be also corrupt, even if the error code indicates there is no error, so we treat it as though it is corrupted.
            // 
            if ( corrupted ) {
                if ( uwp->u.core.err == WPAD_ERR_NONE ) {
                    uwp->u.core.err = WPAD_ERR_CORRUPTED ;
                    corrupted = FALSE ;
                } else if ( uwp->u.core.err == WPAD_ERR_BUSY ) {
                    corrupted = FALSE ;
                }
            } else if ( uwp->u.core.err == WPAD_ERR_CORRUPTED ) {
                corrupted = TRUE ;
            }
            if ( kp->status.dev_type != uwp->u.core.dev ) {
                changed = TRUE ;
            }
            if ( changed ) {
                uwp->u.core.err = WPAD_ERR_INVALID ;
            }
            //----- Advance the index by one
            idx-- ;
            if ( idx < 0 ) {
                idx += kp->uniRingBufExLen + KPAD_RING_BUFS ;
            }
            if ( idx >= KPAD_RING_BUFS ) {
                uwp = &kp->uniRingBufEx[ idx - KPAD_RING_BUFS ] ;
            } else {
                uwp = &kp->uniRingBuf[ idx ] ;
            }
        } while ( --copy_ct > 0 ) ;

        //----- Leave the most recent data because the error check was not accurate
        kp->bufCount = 1 ;
        //----- Get the number of pieces of data in the buffer, excluding the most recent data
        if ( bufCount > 1 ) {
            copy_ct = (int32)( bufCount - 1 ) ;
        } else {
            copy_ct = 1 ;
        }
        //----- Number of pieces of data to return to the application
        return_ct = copy_ct ;
        //----- Save data to the sampling buffer, starting with the oldest
        tp += copy_ct ;
        //----- Index calculation
        idx = (int32)( kp->bufIdx - copy_ct - 1 ) ;
        if ( idx < 0 ) {
            idx += kp->uniRingBufExLen + KPAD_RING_BUFS ;
        }
        //----- Copy data to the buffer, starting with the oldest
        do {
            if ( idx >= KPAD_RING_BUFS ) {
                uwp = &kp->uniRingBufEx[ idx - KPAD_RING_BUFS ] ;
            } else {
                uwp = &kp->uniRingBuf[ idx ] ;
            }
        
            --tp ;
            tp->w = *uwp ;
            idx++ ;
            if ( idx >= kp->uniRingBufExLen + KPAD_RING_BUFS ) {
                idx = 0 ;
            }
        } while ( --copy_ct > 0 ) ;

        (void)OSRestoreInterrupts( enabled ) ;

        // Obtain 1G value from the controller to absorb the individual differences between controllers.

        // Although we'd like to get the value in one try, the value cannot be retrieved immediately after connecting or attaching.
        //  For the time being, it is obtained here each time through the loop.
        {
            WPADAcc core1G = {1, 1, 1} ;
            WPADAcc fs1G = {1, 1, 1} ;

            WPADGetAccGravityUnit( chan, WPAD_DEV_CORE, &core1G ) ;
            if(core1G.x * core1G.y * core1G.z != 0){    // Bug fix
                kp->acc_scale_x    = 1.0f / core1G.x ;
                kp->acc_scale_y    = 1.0f / core1G.y ;
                kp->acc_scale_z    = 1.0f / core1G.z ;
            } else {
        // The values listed here are obtained with WPADGetAccGravityUnit().
        // Accurate values should not be all that different.
                kp->acc_scale_x    = 1.0f / 100 ;
                kp->acc_scale_y    = 1.0f / 100 ;
                kp->acc_scale_z    = 1.0f / 100 ;
            }

            WPADGetAccGravityUnit( chan, WPAD_DEV_FREESTYLE, &fs1G ) ;
            if( fs1G.x * fs1G.y * fs1G.z != 0 ){     // Bug fix
                kp->fs_acc_scale_x = 1.0f / fs1G.x ;
                kp->fs_acc_scale_y = 1.0f / fs1G.y ;
                kp->fs_acc_scale_z = 1.0f / fs1G.z ;
            } else {
        // The values listed here are obtained with WPADGetAccGravityUnit().
        // Accurate values should not be all that different.
                kp->fs_acc_scale_x = 1.0f / 200 ;
                kp->fs_acc_scale_y = 1.0f / 200 ;
                kp->fs_acc_scale_z = 1.0f / 200 ;
            }
        }

        //----- Process obtained data, starting with the oldest
        copy_ct = return_ct ;
        tp = (KPADTmpStatus *)samplingBufs + copy_ct ;

        //----- Process one at a time
        if ( kp->btnProcMode == KPAD_BUTTON_PROC_MODE_TIGHT )
        {
            do {
                --tp ;
                uwp = &tp->w ;

                //----- Check if the device type has changed
                if ( kp->status.dev_type != uwp->u.core.dev ) {
                    kp->status.dev_type = uwp->u.core.dev ;     // Update the device type
                    kp->exResetReq = TRUE ;                     // Reset the extension controller information
                }

                //----- Copy the error code
                kp->status.wpad_err = uwp->u.core.err ;
                //----- Copy the data format
                kp->status.data_format = uwp->fmt ;
                //----- Get the device type when data was sent
                lastDev = kp->status.dev_type ;
                //----- Error processing for button input
                lastCoreButton = lastFsButton = lastClButton = KPAD_BUTTON_MASK ;
                switch ( uwp->u.core.err ) {
                  case WPAD_ERR_NONE :
                    if ( lastDev == WPAD_DEV_FREESTYLE ) {
                        lastFsButton = uwp->u.fs.button ;
                        lastClButton = 0 ;
                    } else {
                        lastFsButton = lastClButton = 0 ;
                    }
                    lastCoreButton = (uint32)( uwp->u.core.button & 0x00009F1F ) ;  // Enable Wii Remote buttons only
                    break;

                  case WPAD_ERR_CORRUPTED :
                    lastFsButton = lastClButton = 0 ;
                    // thru
                  case WPAD_ERR_BUSY :
                    lastCoreButton = (uint32)( uwp->u.core.button & 0x00009F1F ) ;  // Enable Wii Remote buttons only
                    break ;

                  default :
                    // Do nothing and keep the previous state
                    break ;
                }
                //----- If ACK is received from the Wii Remote, use the previous button information
                if ( lastCoreButton == KPAD_BUTTON_MASK ) {
                    lastCoreButton = (uint32)( kp->status.hold & 0x00009F1F ) ;
                }
                if ( lastFsButton == KPAD_BUTTON_MASK ) {
                    lastFsButton = kp->status.hold ;
                }
                //----- Load button information
                read_kpad_button( kp, lastDev, 1, lastCoreButton, lastFsButton, lastClButton ) ;
                //----- Miscellaneous input processing
                switch ( uwp->u.core.err ) {
                  case WPAD_ERR_NONE :
                    read_kpad_ext( kp, uwp ) ;
                    // thru
                  case WPAD_ERR_CORRUPTED :
                    read_kpad_acc( kp, uwp ) ;
                    read_kpad_dpd( kp, uwp ) ;
                    break ;

                  default :
                    if ( !keep ) {
                        kp->status.dpd_valid_fg = 0 ;
                    }
                    break ;
                }
                tp[ 0 ].k = kp->status ;

            } while ( --copy_ct > 0 ) ;
        //----- As before, the most recent button presses are applied to all data in the buffer
        } else {
            //----- Find and get the most recent button and device type from the buffer
            lastDev = WPAD_DEV_NOT_FOUND ;
            lastCoreButton = lastFsButton = lastClButton = KPAD_BUTTON_MASK ;
            do {
                --tp ;
                uwp = &tp->w ;

                //----- Check if the device type has changed
                if ( kp->status.dev_type != uwp->u.core.dev ) {
                    kp->status.dev_type = uwp->u.core.dev ;
                    kp->exResetReq = TRUE ;
                }
                //----- Get the device type when data was sent
                lastDev = uwp->u.core.dev ;

                switch ( uwp->u.core.err ) {
                  case WPAD_ERR_NONE :
                    if ( lastDev == WPAD_DEV_FREESTYLE ) {
                        lastFsButton = uwp->u.fs.button ;
                        lastClButton = 0 ;
                    } else {
                        lastFsButton = lastClButton = 0 ;
                    }
                    // thru
                  case WPAD_ERR_CORRUPTED :
                  case WPAD_ERR_BUSY :
                    lastCoreButton = (uint32)( uwp->u.core.button & 0x00009F1F ) ;  // Enable Wii Remote buttons only
                    break ;

                  default :
                    break ;
                }
            } while ( --copy_ct > 0 ) ;

            //----- If there is no valid button data, inherit the previous data
            if ( lastCoreButton == KPAD_BUTTON_MASK ) {
                lastCoreButton = (uint32)( kp->status.hold & 0x00009F1F ) ;
            }
            if ( lastFsButton == KPAD_BUTTON_MASK ) {
                lastFsButton = kp->status.hold ;
            }
            //----- Process button information
            read_kpad_button( kp, lastDev, (uint32)return_ct, lastCoreButton, lastFsButton, lastClButton ) ;
            //----- Start other input processing once again from the start of the buffer
            copy_ct = return_ct ;
            tp = (KPADTmpStatus *)samplingBufs + copy_ct ;
            do {
                --tp ;
                uwp = &tp->w ;
                //----- Error code copy
                kp->status.wpad_err = uwp->u.core.err ;
                //----- Copy the data format
                kp->status.data_format = uwp->fmt ;
                //----- Miscellaneous input processing
                switch ( uwp->u.core.err ) {
                  case WPAD_ERR_NONE :
                    read_kpad_ext( kp, uwp ) ;
                    // thru
                  case WPAD_ERR_CORRUPTED :
                    read_kpad_acc( kp, uwp ) ;
                    read_kpad_dpd( kp, uwp ) ;
                    break ;

                  default :
                    if ( !keep ) {
                        kp->status.dpd_valid_fg = 0 ;
                    }
                    break ;
                }
                tp[ 0 ].k = kp->status ;
            } while ( --copy_ct > 0 ) ;
        }
    }
    //----- Remove locks
    kp->readLocked = FALSE ;

read_finish:

    if ( return_ct == 0 ) {
        if ( kpad_err == KPAD_READ_ERR_NONE ) {
            //----- Even if there is no valid data, we will guarantee at least the final data
            if ( keep ) {
                samplingBufs[ 0 ] = tmp ;
            }
            kpad_err = KPAD_READ_ERR_NO_DATA ;
        } else if ( kpad_err == KPAD_READ_ERR_NO_CONTROLLER ) {
            if ( keep ) {
                samplingBufs[ 0 ].dev_type = WPAD_DEV_NOT_FOUND ;
                samplingBufs[ 0 ].data_format = WPAD_FMT_CORE ;
                samplingBufs[ 0 ].wpad_err = WPAD_ERR_NO_CONTROLLER ;
            }
        }
    }
    if ( err ) {
        *err = kpad_err ;
    }

    return return_ct ;

}


/*******************************************************************************
        INIT KPAD
 *******************************************************************************/
void KPADInit( void )
{
    KPADInitEx( NULL, 0 );
}

void KPADInitEx( KPADUnifiedWpadStatus uniRingBufs[], uint32 length )
{
    int32             i ;
    KPADInsideStatus *kp ;
    uint32             idx ;

    // SDK 2.0 or later is required (see SDK release note 41)
    // Also KPADRead() assumes: (sizeof(KPADTmpStatus) == sizeof(KPADStatus))
    if (offsetof( WPADFSStatus, err ) == offsetof( WPADStatus, err ) &&
        sizeof(KPADTmpStatus) == sizeof(KPADStatus) ) {
        // OK
    } else {
#if 0
      GXColor black = {0, 0, 0, 0} ;
      GXColor white = {255, 255, 255, 255} ;
        OSFatal( black, white, "KPADInit error" ) ;
        // Never reach here.
#endif
    }

    //----- Exit immediately if this is already initialized
    if ( kp_initialized ) {
        return ;
    }

    //----- WPAD
    WPADInit() ;

    //----- KPAD
    memset( &inside_kpads, 0, sizeof(inside_kpads) ) ;
    kp_err_dist_max = (real32)( 1.0f + (real32)WPADGetDpdSensitivity() ) ;

    i = 0 ;
    do {
        kp = &inside_kpads[ i ] ;

        //----- Configure the connection event callback
        kp->appConnectCallback = WPADSetConnectCallback( i, KPADiConnectCallback ) ;
        //----- Configure the sampling callback
        //kp->appSamplingCallback = WPADSetSamplingCallback( i, KPADiSamplingCallback ) ;

        //----- The DPD is enabled by default
        kp->dpdCurrState = FALSE ;
        kp->dpdNextState = TRUE ;
        kp->dpdCmd = WPADGetDpdFormat( i ) ;

        //----- Check device
        kp->status.dev_type = WPAD_DEV_NOT_FOUND ;
        kp->status.data_format = WPAD_FMT_CORE ;

        kp->dist_org = idist_org ;
        kp->accXY_nrm_hori = iaccXY_nrm_hori ;
        kp->sec_nrm_hori = isec_nrm_hori ;
        kp->center_org = icenter_org ;
        calc_dpd2pos_scale( kp ) ;

        //----- Initialize KPAD pointing behavior
        kp->pos_play_radius  =
          kp->hori_play_radius =
            kp->dist_play_radius =
              kp->acc_play_radius = 0.0f ;

        kp->pos_sensitivity  =
          kp->hori_sensitivity =
            kp->dist_sensitivity =
              kp->acc_sensitivity = 1.0f ;

        kp->pos_play_mode = 
          kp->hori_play_mode = 
            kp->dist_play_mode = 
              kp->acc_play_mode = KPAD_PLAY_MODE_LOOSE ;

        //----- Initialize to no button repeat
        KPADSetBtnRepeat( i, 0.0f, 0.0f ) ;
        //----- Buttons are processed as before
        kp->btnProcMode = KPAD_BUTTON_PROC_MODE_LOOSE ;

        //----- Enabling error correction for the Sensor Bar placement position
        KPADEnableAimingMode( i ) ;

        //----- Nunchuk acceleration correction is disabled by default
        kp->fsAccRevise = 0 ;

        if ( length > 0 && uniRingBufs ) {
            RVL_ASSERT( ( length % 4 ) == 0 ) ;
            kp->uniRingBufExLen = length / 4 ;
            kp->uniRingBufEx = &uniRingBufs[ i * kp->uniRingBufExLen ] ;
        } else {
            kp->uniRingBufExLen = 0 ;
            kp->uniRingBufEx = NULL ;
        }

        for ( idx = 0 ; idx < KPAD_RING_BUFS ; idx++ ) {
            kp->uniRingBuf[idx].u.core.err = WPAD_ERR_NO_CONTROLLER ;
        }
        for ( idx = 0 ; idx < kp->uniRingBufExLen ; idx++ ) {
            kp->uniRingBufEx[idx].u.core.err = WPAD_ERR_NO_CONTROLLER ;
        }

    } while ( ++i < WPAD_MAX_CONTROLLERS ) ;

    //----- Cause a warning to be given when the WPAD library's callback registration function is called
    WPADSetCallbackByKPAD( TRUE ) ;

    //---- Initialize values
    KPADReset() ;

    //----- Rotation matrix initialization
#if 0
    MTXRowCol( kp_fs_rot, 0, 0 ) = 1 ;
    MTXRowCol( kp_fs_rot, 0, 1 ) = 0 ;
    MTXRowCol( kp_fs_rot, 0, 2 ) = 0 ;
    MTXRowCol( kp_fs_rot, 0, 3 ) = 0 ;
    MTXRowCol( kp_fs_rot, 1, 0 ) = 0 ;
    MTXRowCol( kp_fs_rot, 1, 1 ) = (real32) cos( MTXDegToRad(kp_fs_revise_deg) ) ;
    MTXRowCol( kp_fs_rot, 1, 2 ) = (real32)-sin( MTXDegToRad(kp_fs_revise_deg) ) ;
    MTXRowCol( kp_fs_rot, 1, 3 ) = 0 ;
    MTXRowCol( kp_fs_rot, 2, 0 ) = 0 ;
    MTXRowCol( kp_fs_rot, 2, 1 ) = (real32) sin( MTXDegToRad(kp_fs_revise_deg) ) ;
    MTXRowCol( kp_fs_rot, 2, 2 ) = (real32) cos( MTXDegToRad(kp_fs_revise_deg) ) ;
    MTXRowCol( kp_fs_rot, 2, 3 ) = 0 ;
#endif
    kp_fs_rot = CMatrix34::RotateX(CAngle::FromDegrees(kp_fs_revise_deg));

    kp_initialized = 1 ;

    //OSRegisterVersion( __KPADVersion ) ;
}

/*******************************************************************************
        SHUTDOWN KPAD
 *******************************************************************************/
void KPADShutdown( void )
{
    KPADInsideStatus *kp ;
    int32     chan ;

    //----- Prevents a warning from being given, even if the WPAD library's callback registration function is called
    WPADSetCallbackByKPAD( FALSE ) ;

    for ( chan = 0 ; chan < WPAD_MAX_CONTROLLERS ; chan++ ) {
        kp = &inside_kpads[ chan ] ;

        if ( kp->appSamplingCallback ) {
            WPADSetSamplingCallback( chan, kp->appSamplingCallback ) ;
        } else {
            WPADSetSamplingCallback( chan, NULL ) ;
        }
        if ( kp->appConnectCallback ) {
            WPADSetConnectCallback( chan, kp->appConnectCallback ) ;
        } else {
            WPADSetConnectCallback( chan, NULL ) ;
        }
    }
    
    //----- The KPADInit function must be called to reuse KPAD
    kp_initialized = 0 ;
}

/*******************************************************************************
        RESET KPAD
 *******************************************************************************/
void KPADReset( void )
{
    int32     chan ;

    //----- Recalculate constants
    set_obj_interval( kp_obj_interval ) ;

    //---- Reset all KPADs
    chan = WPAD_MAX_CONTROLLERS - 1 ;
    do {
        if ( WPADGetStatus() == WPAD_STATE_SETUP ) {
            WPADStopMotor( chan ) ;
        }
        inside_kpads[ chan ].resetReq = TRUE ;
    } while ( --chan >= 0 ) ;
}

/*******************************************************************************
        Processing Immediately Following a Connection
 *******************************************************************************/
static void KPADiConnectCallback( int32 chan, int32 reason )
{
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;
    uint32 idx ;

    if ( reason == WPAD_ERR_NONE ) {
        //----- We are going to set the sampling callback, so temporarily prevent warnings from being given
        WPADSetCallbackByKPAD( FALSE ) ;
        //----- Configure the sampling callback
        //WPADSetSamplingCallback( chan, KPADiSamplingCallback ) ;
        //----- Sampling callback is set, so re-enable warnings
        WPADSetCallbackByKPAD( TRUE ) ;
        //----- Reset the DPD state
        kp->dpdCurrState = FALSE ;
        kp->dpdCmd = WPAD_DPD_OFF ;
    } else {
        //----- Throw away any remaining data in the ring buffer
        for ( idx = 0 ; idx < KPAD_RING_BUFS ; idx++ ) {
            kp->uniRingBuf[idx].u.core.err = WPAD_ERR_NO_CONTROLLER ;
        }
        for ( idx = 0 ; idx < kp->uniRingBufExLen ; idx++ ) {
            kp->uniRingBufEx[idx].u.core.err = WPAD_ERR_NO_CONTROLLER ;
        }
    }
    
    if ( kp->appConnectCallback ) {
        kp->appConnectCallback( chan, reason ) ;
    }
}

WPADConnectCallback KPADSetConnectCallback( int32 chan, WPADConnectCallback callback )
{
    WPADConnectCallback prevCallback ;
    BOOL enable ;
    
    enable = OSDisableInterrupts() ;
    prevCallback = inside_kpads[ chan ].appConnectCallback ;
    inside_kpads[ chan ].appConnectCallback = callback ;
    OSRestoreInterrupts( enable ) ;
    
    return prevCallback ;
}


/*******************************************************************************
        DPD Control Functions
 *******************************************************************************/
void KPADDisableDPD( const int32 chan )
{
    inside_kpads[ chan ].dpdNextState = FALSE ;
}

void KPADEnableDPD ( const int32 chan )
{
    inside_kpads[ chan ].dpdNextState = TRUE ;
}

void KPADSetControlDpdCallback( int32 chan, KPADControlDpdCallback callback )
{
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;
    BOOL enabled ;

    enabled = OSDisableInterrupts() ;
    kp->dpd_ctrl_callback = callback ;
    (void)OSRestoreInterrupts( enabled ) ;
}

static void KPADiControlDpdCallback( int32 chan, int32 result )
{
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;

    if ( result == WPAD_ERR_NONE ) {
        if ( kp->dpd_ctrl_callback &&
            !kp->dpdPostCallbackDone ) {
            kp->dpdPostCallbackDone = TRUE ;
            kp->dpd_ctrl_callback( chan, KPAD_STATE_CTRL_DPD_FINISHED ) ;
            kp->dpdPreCallbackDone = FALSE ;
        }
    }
    kp->dpdCurrState = (uint8)WPADIsDpdEnabled( chan ) ;
    kp->dpdIssued = FALSE ;
}

/*******************************************************************************
        Processing when data is obtained
 *******************************************************************************/

static void KPADiSamplingCallback(int32 chan, CInputGeneratorWiimote * pInput)
{
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;
    KPADUnifiedWpadStatus *uwp ;
    uint32 idx ;
    uint32 type ;
    uint32 curDpd ;
    real32 aimClbr ;
    static struct {
        uint8 dpd ;
        uint8 fmt ;
    } table[] = {
        { WPAD_DPD_OFF, WPAD_FMT_CORE_ACC },
        { WPAD_DPD_EXP, WPAD_FMT_CORE_ACC_DPD },
        { WPAD_DPD_OFF, WPAD_FMT_FREESTYLE_ACC },
        { WPAD_DPD_STD, WPAD_FMT_FREESTYLE_ACC_DPD },
        { WPAD_DPD_OFF, WPAD_FMT_CLASSIC_ACC },
        { WPAD_DPD_STD, WPAD_FMT_CLASSIC_ACC_DPD },
        { WPAD_DPD_OFF, WPAD_FMT_GUITAR },
        { WPAD_DPD_STD, WPAD_FMT_GUITAR },
        { WPAD_DPD_OFF, WPAD_FMT_BALANCE_CHECKER },
        { WPAD_DPD_OFF, WPAD_FMT_BALANCE_CHECKER },
        { WPAD_DPD_OFF, WPAD_FMT_TRAIN },
        { WPAD_DPD_OFF, WPAD_FMT_TRAIN }
    } ;

    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) );

    if ( WPADProbe( chan, &type ) == WPAD_ERR_NO_CONTROLLER ) {
        goto finish ;
    }

    idx = kp->bufIdx ;
    if ( idx >= KPAD_RING_BUFS + kp->uniRingBufExLen ) {
        idx = 0 ;
    }

    if ( idx >= KPAD_RING_BUFS ) {
        uwp = &kp->uniRingBufEx[ idx - KPAD_RING_BUFS ] ;
    } else {
        uwp = &kp->uniRingBuf[ idx ] ;
    }
    WPADRead( chan, pInput, &uwp->u ) ;
    uwp->fmt = (uint8)WPADGetDataFormat( chan ) ;

    kp->bufIdx = (uint8)( idx + 1 ) ;
    if ( kp->bufCount < kp->uniRingBufExLen + KPAD_RING_BUFS ) {
        kp->bufCount++ ;
    }

    if ( kp->aimReq ) {
        if ( kp->aimEnabled ) {
            if ( WPAD_SENSOR_BAR_POS_TOP == WPADGetSensorBarPosition() ) {
                aimClbr = 0.2f ;
            } else {
                aimClbr = -0.2f ;
            }
        } else {
            aimClbr = 0.0f ;
        }
        KPADSetSensorHeight( chan,  aimClbr ) ;
        
        kp->aimReq = FALSE ;
    }

    //----- Check DPD settings
    switch ( type ) {
      case WPAD_DEV_CORE :
      case WPAD_DEV_FUTURE :
      case WPAD_DEV_NOT_SUPPORTED :
      case WPAD_DEV_UNKNOWN :           idx = 0 ;       break ;
      case WPAD_DEV_FREESTYLE :         idx = 2 ;       break ;
      case WPAD_DEV_CLASSIC :           idx = 4 ;       break ;
      case WPAD_DEV_GUITAR :            idx = 6 ;       break ;
      case WPAD_DEV_BALANCE_CHECKER :   idx = 8 ;       break ;
      case WPAD_DEV_TRAIN :             idx = 10 ;      break ;
      default :                         goto finish ;   break ;
    }

    if ( kp->dpdNextState ) {
        idx += 1 ;
    }

    curDpd = (uint32)( WPADIsDpdEnabled( chan ) ? kp->dpdCmd : WPAD_DPD_OFF );

    if ( curDpd != table[idx].dpd ) {
        if ( kp->dpd_ctrl_callback &&
             !kp->dpdPreCallbackDone ) {
            kp->dpdPreCallbackDone = TRUE ;
            kp->dpd_ctrl_callback( chan, KPAD_STATE_CTRL_DPD_START ) ;
            kp->dpdPostCallbackDone = FALSE ;
        }
        if ( !kp->dpdIssued ) {
            kp->dpdIssued = TRUE ;
            if (WPADControlDpd( chan, table[ idx ].dpd, KPADiControlDpdCallback )
                == WPAD_ERR_NONE ) {
                kp->dpdCmd = table[ idx ].dpd ;
            }
        }
    } else {
        if ( uwp->fmt != table[ idx ].fmt ) {
            WPADSetDataFormat( chan, table[ idx ].fmt );
        }
    }

finish :
    if ( kp->appSamplingCallback ) {
        kp->appSamplingCallback( chan ) ;
    }
}

WPADSamplingCallback KPADSetSamplingCallback( int32 chan, WPADSamplingCallback callback )
{
    WPADSamplingCallback prevCallback ;
    BOOL enable ;

    enable = OSDisableInterrupts() ;
    prevCallback = inside_kpads[ chan ].appSamplingCallback ;
    inside_kpads[ chan ].appSamplingCallback = callback ;
    OSRestoreInterrupts( enable ) ;
    
    return prevCallback ;
}

/*******************************************************************************
        Get WPAD-Format Data
 *******************************************************************************/
void KPADGetUnifiedWpadStatus( int32 chan, KPADUnifiedWpadStatus *dst, uint32 count )
{
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;
    KPADUnifiedWpadStatus *uwp ;
    BOOL    enabled ;
    uint32     idx ;

    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;

    if ( count > kp->uniRingBufExLen + KPAD_RING_BUFS ) {
        count = kp->uniRingBufExLen + KPAD_RING_BUFS ;
    }

    enabled = OSDisableInterrupts() ;

    idx = kp->bufIdx ;
    while ( count-- ) {
        if ( idx == 0 ) {
            idx = kp->uniRingBufExLen + KPAD_RING_BUFS - 1 ;
        } else {
            idx-- ;
        }
        //    <== latest ... oldest ==>
        // dst[0] dst[1] ... dst[KPAD_RING_BUFS - 1]
        if ( idx >= KPAD_RING_BUFS ) {
            uwp = &kp->uniRingBufEx[ idx - KPAD_RING_BUFS ] ;
        } else {
            uwp = &kp->uniRingBuf[ idx ] ;
        }
        if ( WPADGetStatus() != WPAD_STATE_SETUP ) {
            uwp->u.core.err = WPAD_ERR_INVALID ;
        }
        memcpy( dst, uwp, sizeof(KPADUnifiedWpadStatus) ) ;
        dst++ ;
    }

    (void)OSRestoreInterrupts( enabled ) ;
}

/*******************************************************************************
        Clamping Method for Control Stick
 *******************************************************************************/
void KPADEnableStickCrossClamp( void )
{
    kp_stick_clamp_cross = TRUE ;
}

void KPADDisableStickCrossClamp( void )
{
    kp_stick_clamp_cross = FALSE ;
}

/*******************************************************************************
        Angle Correction for Nunchuk Acceleration
 *******************************************************************************/
void KPADSetReviseMode( int32 chan, BOOL sw )
{
    KPADInsideStatus *kp = &inside_kpads[ chan ] ;
    
    kp->fsAccRevise = (uint8)sw ;
}

real32 KPADReviseAcc( Vec *acc )
{
    //MTXMultVec( kp_fs_rot, acc, acc ) ;
      CVector3 &_acc = reinterpret_cast<CVector3 &>(acc);
       _acc = kp_fs_rot * _acc;

    
    return kp_fs_revise_deg ;   // Return the correction angle (in degrees).
}

real32 KPADGetReviseAngle( void )
{
    return kp_fs_revise_deg ;   // Return the correction angle (in degrees).
}

/*******************************************************************************
        Controls for All Calculation Methods
 *******************************************************************************/
void KPADSetPosPlayMode( int32 chan, KPADPlayMode mode )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].pos_play_mode = mode ;
}

void KPADSetHoriPlayMode( int32 chan, KPADPlayMode mode )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].hori_play_mode = mode ;
}

void KPADSetDistPlayMode( int32 chan, KPADPlayMode mode )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].dist_play_mode = mode ;
}

void KPADSetAccPlayMode( int32 chan, KPADPlayMode mode )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].acc_play_mode = mode ;
}

KPADPlayMode KPADGetPosPlayMode( int32 chan )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    return ( inside_kpads[ chan ].pos_play_mode ) ;
}

KPADPlayMode KPADGetHoriPlayMode( int32 chan )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    return ( inside_kpads[ chan ].hori_play_mode ) ;
}

KPADPlayMode KPADGetDistPlayMode( int32 chan )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    return ( inside_kpads[ chan ].dist_play_mode ) ;
}

KPADPlayMode KPADGetAccPlayMode( int32 chan )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    return ( inside_kpads[ chan ].acc_play_mode ) ;
}

/*******************************************************************************
        Button Processing Method Controls
 *******************************************************************************/
void KPADSetButtonProcMode( int32 chan, uint8 mode )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    inside_kpads[ chan ].btnProcMode = mode ;
}

uint8 KPADGetButtonProcMode( int32 chan )
{
    RVL_ASSERT( (0 <= chan) && (chan < WPAD_MAX_CONTROLLERS) ) ;
    return ( inside_kpads[ chan ].btnProcMode ) ;
}
