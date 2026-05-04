#pragma once

#include "TransfarringDefines.h"

#ifndef __TRANSFARING_SAVE_LOAD_MENU_UTILS
#define __TRANSFARING_SAVE_LOAD_MENU_UTILS

#define TRANSFARRING_X_OFFSET 13

#define TRANSFARRING_LOCK_X 48
#define TRANSFARRING_LOCK_Y_OFFSET 2

#define TRANSFARRING_LOCK_COLOR (0x00aab4a0)
#define TRANSFARRING_LINK_COLOR (0x00aab4a0)
#define TRANSFARRING_CLOUD_COLOR (0x00ffffff)
#define TRANSFARRING_SQ_BUTTON_COLOR (0x8054594f)
#define TRANSFARRING_TEXT_COLOR (0x002a2d28)

#define TRANSFARRING_SELECTED_TEXT_COLOR (0x342a2d28)

#define TRANSFARRING_PSN_X 215
#define TRANSFARRING_PSN_Y 40

struct xtextscn_Work;

typedef enum
{
   kTransfarring_LockIcon,
   kTransfarring_CloudIcon,
   kTransfarring_LinkIcon,
   kTransfarring_SqButtonIcon,
}
ETransfarringIconType;

typedef enum
{
   kSaveLoadScreen_Normal,
   kSaveLoadScreen_Photos,
}
ESaveLoadScreenType;

typedef enum
{
   kTransfarring_PSN_Connected,
   kTransfarring_PSN_Disconnected,
}
ETransfarringPsnConnectionType;


void Transfarring_RenderIconSprite( struct xtextscn_Work *work_ptr, int x, int y, unsigned int color,
                                   ETransfarringIconType iconType, ESaveLoadScreenType screenType );

void Transfarring_RenderPsnConnectionSprite( struct xtextscn_Work *work_ptr, int x, int y,
                                            ETransfarringPsnConnectionType connected );

ESaveType Transfarring_GetTransfarringType( int fileKind );

#endif