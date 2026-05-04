#include <message_dialog.h>
#include <ctrl.h>
#include <stdio.h>
#include <string.h>

typedef int LONG;
typedef unsigned int ULONG;
typedef short SHORT;
typedef unsigned short USHORT;
typedef char CHAR;
typedef unsigned char UCHAR;

#include "ExtLibraries/libPad/include/libpad.h"

#define FALSE (0)
#define TRUE (1)

static int gPressingControllerSwitchChord = FALSE;
static int gCurrentControllerIndex = 0;

inline unsigned short set_button_bit(unsigned short input, unsigned short button_mask, int shift)
{
   return ((input & button_mask) != 0) ? (1 << shift) : 0;
}

inline unsigned char get_button_pressure(unsigned short input, unsigned short button_mask)
{
   return ((input & button_mask) != 0) ? 0xFF : 0;
}

extern void pad_vta_init()
{
   sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALANALOG);
}

extern int pad_vta_get_data(int controllerIndex, CellPadData* pPadData)
{
   SceCtrlData vtaPad;
   int nBufs;

   pPadData->len = sizeof(pPadData->button);
   memset(pPadData->button, 0, sizeof( pPadData->button));

   if ( sceMsgDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_RUNNING )
   {
      // If we have a message dialog up, don't allow for any controls

      return 0;
   
   }
   // Call a non-blocking read
   nBufs = sceCtrlPeekBufferPositive( 0, &vtaPad, 1 );
   if ( nBufs < 0 )
   {
      return 0;
   }

   if (controllerIndex != gCurrentControllerIndex)
   {
      return 0;
   }

#ifndef GOLD_VERSION
   if ((vtaPad.buttons & SCE_CTRL_L) && (vtaPad.buttons & SCE_CTRL_R) && (vtaPad.buttons & SCE_CTRL_TRIANGLE))
   {
      if (!gPressingControllerSwitchChord)
      {
         gCurrentControllerIndex = 1 - gCurrentControllerIndex;
         gPressingControllerSwitchChord = TRUE;
      }
      // while pressing the switch chord, don't do anything in the game
      return 0;
   }
   else
   {
      gPressingControllerSwitchChord = FALSE;
   }
#endif

   pPadData->button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] = vtaPad.lx;
   pPadData->button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] = vtaPad.ly;
   pPadData->button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] = vtaPad.rx;
   pPadData->button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] = vtaPad.ry;

   // Digital info...
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL1] = 0;
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL2] = 0;

   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= set_button_bit(vtaPad.buttons, SCE_CTRL_SELECT, 0);

   // Bits position 1 and 2 are thumb stick presses which are not supported on vita...
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= set_button_bit(vtaPad.buttons, SCE_CTRL_START, 3);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= set_button_bit(vtaPad.buttons, SCE_CTRL_UP, 4);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= set_button_bit(vtaPad.buttons, SCE_CTRL_RIGHT, 5);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= set_button_bit(vtaPad.buttons, SCE_CTRL_DOWN, 6);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= set_button_bit(vtaPad.buttons, SCE_CTRL_LEFT, 7);

   // Bits position 0 and 1 are the trigger buttons which are not present on vita...
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= set_button_bit(vtaPad.buttons, SCE_CTRL_L, 2);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= set_button_bit(vtaPad.buttons, SCE_CTRL_R, 3);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= set_button_bit(vtaPad.buttons, SCE_CTRL_TRIANGLE, 4);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= set_button_bit(vtaPad.buttons, SCE_CTRL_CIRCLE, 5);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= set_button_bit(vtaPad.buttons, SCE_CTRL_CROSS, 6);
   pPadData->button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= set_button_bit(vtaPad.buttons, SCE_CTRL_SQUARE, 7);


   //Fill pressure info
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_LEFT] = get_button_pressure(vtaPad.buttons, SCE_CTRL_LEFT);
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_RIGHT] = get_button_pressure(vtaPad.buttons, SCE_CTRL_RIGHT);
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_UP] = get_button_pressure(vtaPad.buttons, SCE_CTRL_UP);
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_DOWN] = get_button_pressure(vtaPad.buttons, SCE_CTRL_DOWN);

   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_TRIANGLE] = get_button_pressure(vtaPad.buttons, SCE_CTRL_TRIANGLE);
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_CIRCLE] = get_button_pressure(vtaPad.buttons, SCE_CTRL_CIRCLE);
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_CROSS] = get_button_pressure(vtaPad.buttons, SCE_CTRL_CROSS);
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_SQUARE] = get_button_pressure(vtaPad.buttons, SCE_CTRL_SQUARE);

   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_L1] = get_button_pressure(vtaPad.buttons, SCE_CTRL_L);
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_R1] = get_button_pressure(vtaPad.buttons, SCE_CTRL_R);
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_L2] = 0; // Buttons does not exist on vita
   pPadData->button[CELL_PAD_BTN_OFFSET_PRESS_R2] = 0; // Button does not exist on vita

   return 1;
}
