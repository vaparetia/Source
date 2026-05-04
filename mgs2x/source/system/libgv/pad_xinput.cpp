// NOTE - NO PRECOMPILED HEADER
#if BP_360
#include <xtl.h>
#else
#include <windows.h>
#include <xinput.h>
#endif

#include <stdio.h>
#include <libpad.h>

#if !BP_360
#pragma comment( lib, "xinput.lib" )
#endif

namespace
{
   XINPUT_STATE sXInputState = { 0 };

   inline unsigned char float_to_byte( int s, float f )
   {
      int i = 128;
      if( s < 0 )
      {
         i -= (int)((f*128.0f)+0.5f);
      }
      else
      {
         i += (int)((f*127.0f)+0.5f);
      }
      if( i < 0 ) i = 0;
      if( i > 255 ) i = 255;
      return (unsigned char)i;
   }

   inline void short_int_to_byte( int x, int y, uint16_t& outX, uint16_t& outY )
   {
      // Compute abs axis 0->1
      float fx = (x < 0) ? (x / -32768.0f) : (x/32767.0f);
      float fy = (y < 0) ? (y / -32768.0f) : (y/32767.0f);
     
      // Map to byte
      unsigned char cx = float_to_byte( +x, fx );
      unsigned char cy = float_to_byte( -y, fy );

      // Store
      outX = cx;
      outY = cy;
   }

   inline unsigned short move_bit( unsigned short data, unsigned short mask_src, int bit_dst )
   {
      return ( ( data & mask_src ) == mask_src ) ? ( 1 << bit_dst ) : 0;
   }

   inline unsigned char pressure_from_bit( unsigned short data, unsigned short mask_src )
   {
      return ( ( data & mask_src ) == mask_src ) ? 0xFF : 0;
   }
}

#if BP_360
extern int gRemappedControllerIndex[4];
extern int gMainControllerDisconnected;
#endif

extern "C"
{
   extern int pad_xinput_get_data( int controllerIndex_, CellPadData *pPadData )
   {
#if BP_360
      int controllerIndex = gRemappedControllerIndex[controllerIndex_];
#else
      int controllerIndex = controllerIndex_;
#endif

      DWORD xiError = XInputGetState( controllerIndex, &sXInputState );

      if ( xiError != ERROR_SUCCESS && xiError != ERROR_DEVICE_NOT_CONNECTED )
         printf( "pad_xinput: Error %8.8x on controller %d\n", xiError, controllerIndex );
#if BP_360
      if( xiError == ERROR_DEVICE_NOT_CONNECTED )
      {
         if( controllerIndex_ == 0 )
         {
            gMainControllerDisconnected = 1;
         }
      }
      else if( xiError == ERROR_SUCCESS )
      {
         if( controllerIndex_ == 0 )
         {
            gMainControllerDisconnected = 0;
         }
      }
#endif

      if ( xiError != ERROR_SUCCESS )
      {
         return 0;
      }

      pPadData->len = CELL_PAD_BTN_OFFSET_PRESS_R2 + 1;
      memset( pPadData->button, 0, sizeof( pPadData->button ) );

      XINPUT_GAMEPAD const &xpad = sXInputState.Gamepad;

      //Fill analog info
      short_int_to_byte( xpad.sThumbLX, xpad.sThumbLY,  pPadData->button[ CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X  ], pPadData->button[ CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y  ]);
      short_int_to_byte( xpad.sThumbRX, xpad.sThumbRY,  pPadData->button[ CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X ], pPadData->button[ CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y ]);

      //Fill digital info
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] = 0;
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] = 0;

      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_BACK, 0 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB, 1 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB, 2 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_START, 3 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_DPAD_UP, 4 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT, 5 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN, 6 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL1 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT, 7 );

      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] |= move_bit( xpad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 1, 0 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] |= move_bit( xpad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 1, 1 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, 2 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, 3 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_Y, 4 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_B, 5 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_A, 6 );
      pPadData->button[ CELL_PAD_BTN_OFFSET_DIGITAL2 ] |= move_bit( xpad.wButtons, XINPUT_GAMEPAD_X, 7 );

      //Fill pressure info
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_LEFT ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT );
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_RIGHT ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT );
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_UP ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_DPAD_UP );
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_DOWN ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN );

      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_TRIANGLE ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_Y );
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_CIRCLE ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_B );
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_CROSS ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_A );
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_SQUARE ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_X );

      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_L1 ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER );
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_R1 ] = pressure_from_bit( xpad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER );
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_L2 ] = xpad.bLeftTrigger;
      pPadData->button[ CELL_PAD_BTN_OFFSET_PRESS_R2 ] = xpad.bRightTrigger;

      return 1;
   }

   extern float gBP_VibrationLgMin;
   extern float gBP_VibrationLgMax;
   extern float gBP_VibrationLgThreshold;
   extern float gBP_VibrationSmScale;

   unsigned int pad_xinput_set_state( int controllerIndex_, unsigned char sm, unsigned char lg, int sync )
   {
#if BP_360
      int controllerIndex = gRemappedControllerIndex[controllerIndex_];
#else
      int controllerIndex = controllerIndex_;
#endif
      //small is 0 or 1
      //large is 0...255
      XINPUT_VIBRATION vib = { 0 };
      const float vibrationLgRange = gBP_VibrationLgMax - gBP_VibrationLgMin;
      float fVibrationLg = (float)lg / 255.0f;
      fVibrationLg = (fVibrationLg > gBP_VibrationLgThreshold) ? (gBP_VibrationLgMin + (vibrationLgRange * fVibrationLg)) : 0.0f;         // be sure to clear the actparams

      //low-frequency
      vib.wLeftMotorSpeed = (WORD)(fVibrationLg*65535);

      //high-frequency
      vib.wRightMotorSpeed = (WORD)( gBP_VibrationSmScale * sm * 65535 );

      //printf("VIB low %5d, high %5d\n", (int)vib.wLeftMotorSpeed, (int)vib.wRightMotorSpeed );
      DWORD res = XInputSetState( controllerIndex, &vib );
      if( sync )
      {
         while( res == ERROR_BUSY )
         {
            ::Sleep(1);
            res = XInputSetState( controllerIndex, &vib );
         }
      }

      return res;
   }
}