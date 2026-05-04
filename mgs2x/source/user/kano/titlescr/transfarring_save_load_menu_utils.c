#include "stdafx.h"

#include "BP_BuildDefines.h"

#if BP_USE_NEW_FONT_SYSTEM()
#include "BP_Font.h"
#else
#include	"font.h"
#endif

#include "../mode/menu/menu.h"
#include "../mode/menu/text_scn.h"

#include "transfarring_save_load_menu_utils.h"

#include "../mcman/mcman.h"

#include "Transfarring_UI_Strings.h"

#define TRANSFARRING_ICON_WIDTH ( 12.0f * ( (float) DRAW_WIDTH / VR_WIDTH ) )
#define TRANSFARRING_ICON_HEIGHT ( 16.0f * ( (float) DRAW_HEIGHT / VR_HEIGHT ) )

#define TRANSFARRING_PSN_SPRITE_WIDTH ( 250.0f * ( (float) DRAW_WIDTH / VR_WIDTH ) )
#define TRANSFARRING_PSN_SPRITE_HEIGHT ( 8.0f * ( (float) DRAW_HEIGHT / VR_HEIGHT ) )

#define TRANSFARRING_TRI_ID_SAVE_LOAD_NORMAL (0x009deed4) /*GV_StrCode("save_load")*/
#define TRANSFARRING_TRI_ID_SAVE_LOAD_PHOTOS (0x0096a917) /*GV_StrCode("photo_save")*/

#define TRANSFARRING_LOCK_TEX_ID (0x00267e8d) /*GV_StrCode("transfarring_icon_lock")*/
#define TRANSFARRING_CLOUD_TEX_ID (0x003e8348) /*GV_StrCode("transfarring_icon_cloud")*/
#define TRANSFARRING_LINK_TEX_ID (0x002667ed) /*GV_StrCode("transfarring_icon_link")*/
#define TRANSFARRING_SQ_BUTTON_TEX_ID (0x003e1fc3) /*GV_StrCode("transfarring_button_square")*/

#define PSN_LOGO_COLOR 0x8054594f

void Transfarring_RenderIconSprite( struct xtextscn_Work *work, int x, int y, unsigned int color,
                                   ETransfarringIconType iconType, ESaveLoadScreenType screenType )
{
   if ( work )
   {
      float const fx0 = x * ( (float)DRAW_WIDTH / VR_WIDTH );
      float const fy0 = y * ( (float)DRAW_HEIGHT / VR_HEIGHT );

      TextScn_PRIM_CONTROL * const prim_ctrl = &work->text_ctrl;
      int const triId = screenType == kSaveLoadScreen_Normal
         ? TRANSFARRING_TRI_ID_SAVE_LOAD_NORMAL : TRANSFARRING_TRI_ID_SAVE_LOAD_PHOTOS;
      
      DG_TEX *iconTexture = 0;

      switch(iconType)
      {
      case kTransfarring_LockIcon:
         iconTexture = DG_GetTexture2(triId, TRANSFARRING_LOCK_TEX_ID);
         break;
      case kTransfarring_CloudIcon:
         iconTexture = DG_GetTexture2(triId, TRANSFARRING_CLOUD_TEX_ID);
         break;
      case kTransfarring_LinkIcon:
         iconTexture = DG_GetTexture2(triId, TRANSFARRING_LINK_TEX_ID);
         break;
      case kTransfarring_SqButtonIcon:
         iconTexture = DG_GetTexture2(triId, TRANSFARRING_SQ_BUTTON_TEX_ID);
         break;
      }
      
      if(iconTexture)
      {
         prim_ctrl->current_buffer = DG_SetDmapackTex(prim_ctrl->current_buffer, iconTexture);
         prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

         prim_ctrl->current_buffer = DG_SetDmapackSprt(prim_ctrl->current_buffer, fx0, fy0, 0, 0, 
            fx0 + TRANSFARRING_ICON_WIDTH, fy0 + TRANSFARRING_ICON_HEIGHT, 1.0f, 1.0f, DG_MakeDmaPackColorFromInt(color));

         DG_SetDmapackEnd( prim_ctrl->current_buffer );
      }
   }
}

void Transfarring_RenderPsnConnectionSprite( struct xtextscn_Work *work, int x, int y,
                                            ETransfarringPsnConnectionType connected )
{
   if ( work )
   {
      static unsigned int sConnectedTexId = 0;
      static unsigned int sDisconnectedTexId = 0;

      float const fx0 = x * ( (float)DRAW_WIDTH / VR_WIDTH );
      float const fy0 = y * ( (float)DRAW_HEIGHT / VR_HEIGHT );

      TextScn_PRIM_CONTROL * const prim_ctrl = &work->text_ctrl;

      DG_TEX *lockTexture;

      if(sConnectedTexId == 0 && sDisconnectedTexId == 0)
      {
         char textureName[256] = "transfarring_psn_connected";
         
         strcat(textureName, GetLanguageDependentImageStringPostfix());
         sConnectedTexId = GV_StrCode(textureName);

         sprintf(textureName, "%s%s", "transfarring_psn_notconnected", GetLanguageDependentImageStringPostfix());
         sDisconnectedTexId = GV_StrCode(textureName);
      }
      
      lockTexture = DG_GetTexture2(TRANSFARRING_TRI_ID_SAVE_LOAD_NORMAL, connected == kTransfarring_PSN_Connected ?
         sConnectedTexId : sDisconnectedTexId);

      prim_ctrl->current_buffer = DG_SetDmapackTex(prim_ctrl->current_buffer, lockTexture);
      prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

      prim_ctrl->current_buffer = DG_SetDmapackSprt(prim_ctrl->current_buffer, fx0, fy0, 0, 0, 
         fx0 + TRANSFARRING_PSN_SPRITE_WIDTH, fy0 + TRANSFARRING_PSN_SPRITE_HEIGHT, 1.0f, 1.0f, DG_MakeDmaPackColorFromInt(PSN_LOGO_COLOR));

      DG_SetDmapackEnd( prim_ctrl->current_buffer );
   }
}

ESaveType Transfarring_GetTransfarringType( int fileKind )
{
   switch(fileKind)
   {
   case MCMAN_FILE_KIND_GAME:
      return kST_Game;
   case MCMAN_FILE_KIND_VR:
      return kST_VR;
   case MCMAN_FILE_KIND_SNAKE_TALES:
      return kST_SnakeTales;
   default:
      return kST_Game;
   }
}
