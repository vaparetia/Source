//----------------------------------------------------------------------------
// BP_RendererDebug.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"

//----------------------------------------------------------------------------

#include "Engine/System/CSyncCriticalSection.h"

#include "Renderer/Base/Backend/CRenderBackend.h"

#include "MGS_Common.h"
#include "BP_DebugPad.h"

#include "BP_Renderer.h"
#include "BP_RenderFX.h"

//----------------------------------------------------------------------------

#include "BP_RendererDebug.h"

//----------------------------------------------------------------------------

extern "C" 
{
   extern int gBP_ShowDebugMenuBlackBackground;
}

int gBP_DebugMenuIsActive = 0;

#if BP_ENABLE_DEBUG_MENU

//----------------------------------------------------------------------------

struct SMenu
{
   int         index;
   int         parentIdx;
   const char* pName;
   int         currentRow;
};

enum EMenuEntryType
{
   kMET_Bool,
   kMET_Float,
   kMET_Int,
   kMET_Enum,
   kMET_Action,
   kMET_String,      
   kMET_Separator,      
};

struct SMenuEntry
{
   int            menu;
   const char *   pName;
   EMenuEntryType type;

   const int*     pEnabled;
   int            (*callback)(int);
   int            callbackData;

   SMenuEntry*    pNext;
};

struct SMenuEntryBool
{
   SMenuEntry  entry;
   int*        pValue;
};

struct SMenuEntryFloat
{
   SMenuEntry  entry;
   float* pValue;
   float min, max;
   float stepSmall, stepBig;
};

struct SMenuEntryInt
{
   SMenuEntry  entry;
   int* pValue;
   int min, max;
   int stepSmall, stepBig;
};

struct SMenuEntryEnum
{
   SMenuEntry  entry;
   const char** ppEnums;
   int* pValue;
   int min, max;
};

struct SMenuEntryAction
{
   SMenuEntry  entry;
};

struct SMenuEntryString
{
   SMenuEntry  entry;
   const char *pValue;
};

struct SMenuEntrySeparator
{
   SMenuEntry  entry;
};

//----------------------------------------------------------------------------

namespace DebugMenu
{
   int const kMaxMenuCount = 256*2;

   SMenu gMenus[kMaxMenuCount] = {0};
   int gMenuCount = 0;

   int const kMenuEntryBufferSize = 8192 * 2;
   char gMenuEntryBuffer[kMenuEntryBufferSize] = {0};
   char const * gMenuEntryBufferEnd = gMenuEntryBuffer + kMenuEntryBufferSize;

   SMenuEntry* gMenuEntryHead = NULL;
   SMenuEntry* gMenuEntryTail = NULL;
   SMenuEntry* gMenuEntryAllocPtr = (SMenuEntry*)gMenuEntryBuffer;
   
   SMenuEntry* AllocMenuEntry(int size)
   { 
      char* pAllocEnd = ((char*)gMenuEntryAllocPtr) + size;
      
      if( pAllocEnd > gMenuEntryBufferEnd )
         return NULL;

      SMenuEntry* pResult = gMenuEntryAllocPtr;
      memset( pResult, 0, size );
      
      gMenuEntryAllocPtr = (SMenuEntry*)pAllocEnd;

      pResult->pNext = NULL;

      if( !gMenuEntryHead )
         gMenuEntryHead = pResult;

      if( gMenuEntryTail )
         gMenuEntryTail->pNext = pResult;

      gMenuEntryTail = pResult;

      return pResult;
   }

   int gCurrentMenuStack[128] = { -1 };
   int gCurrentMenuStackDepth = 1;
   int gRootMenuCurrentRow = 0;

   int gPrintMeasure = 0;
   int gPrintMinX = 0;
   int gPrintMinY = 0;
   int gPrintMaxX = 0;
   int gPrintMaxY = 0;

   int gQuadAlpha = 90;

   CSyncCriticalSection sDebugMenuReadWriteMutex;
};

//----------------------------------------------------------------------------

struct SMenuRow
{
   SMenu const *        pMenu;
   SMenuEntry const *   pEntry;
};

int BP_DebugMenu_FlattenTree(int menuIdx, SMenuRow* pOutput, int const maxCount)
{
   using namespace DebugMenu;

   int count = 0;

   for( int i = 0; i < gMenuCount && (count < maxCount); ++i )
   {
      SMenu const & menu = gMenus[i];

      if( menu.parentIdx == menuIdx )
      {
         pOutput[count].pMenu = &menu;
         pOutput[count].pEntry = NULL;
         ++count;
      }
   }

   for( SMenuEntry* pEntry = gMenuEntryHead; pEntry && (count < maxCount); pEntry = pEntry->pNext )
   {
      if( pEntry->menu == menuIdx )
      {
         pOutput[count].pMenu = NULL;
         pOutput[count].pEntry = pEntry;
         ++count;
      }
   }

   return count;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_IsActive()
{
   return gBP_DebugMenuIsActive;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_GetActiveMenu()
{
   using namespace DebugMenu;

   if(gBP_DebugMenuIsActive)
   {
      return gCurrentMenuStack[gCurrentMenuStackDepth - 1];
   }
   else
   {
      return -1;
   }
}

//----------------------------------------------------------------------------

int& BP_DebugMenu_GetCurrentRow()
{
   using namespace DebugMenu;

   // Lookup selected menu index
   BPE_ASSERT_NO_MSG(gCurrentMenuStackDepth>0);
   const int menuIndex = gCurrentMenuStack[gCurrentMenuStackDepth - 1];

   // Lookup menu current row
   if( (menuIndex >= 0) && (menuIndex < gMenuCount) )
   {
      // Return menu row
      return gMenus[menuIndex].currentRow;
   }
   else
   {
      // Return root menu row
      return gRootMenuCurrentRow;
   }
}

//----------------------------------------------------------------------------

void BP_DebugMenu_SetActive(int active)
{
   using namespace DebugMenu;

   gBP_DebugMenuIsActive = active;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_ProcessInput(int controllerPort)
{
   using namespace DebugMenu;

   CSyncCriticalSectionLocker locker( &sDebugMenuReadWriteMutex );

#if JADEBUG
   if( BP_DebugPad_Press(controllerPort, BP_PAD_TRIANGLE) )
#else
   if( BP_DebugPad_Press(controllerPort, BP_PAD_SQUARE) )
#endif
   {
      BP_DebugMenu_SetActive(!gBP_DebugMenuIsActive);
   }
   else if( gBP_DebugMenuIsActive )
   {
      SMenuRow rows[128];
      int count = BP_DebugMenu_FlattenTree(gCurrentMenuStack[gCurrentMenuStackDepth - 1], rows, 128);

      if( count > 0 )
      {
         int& currentRow = BP_DebugMenu_GetCurrentRow();

         if( currentRow < 0 )
            currentRow = 0;

         if( currentRow >= count )
            currentRow = count - 1;

         if( rows[currentRow].pMenu )
         {
            if( BP_DebugPad_Press(controllerPort, BP_PAD_B) ) // Cross
            {
               gCurrentMenuStack[gCurrentMenuStackDepth] = rows[currentRow].pMenu->index;
               gCurrentMenuStackDepth++;
               return;
            }
         }
         else if( rows[currentRow].pEntry )
         {
            SMenuEntry * pEntry = (SMenuEntry *)rows[currentRow].pEntry;
            switch(pEntry->type)
            {
            case kMET_String:  // do nothing
               break;            
            case kMET_Separator:  // do nothing

               break;            
            case kMET_Bool:
               {
                  SMenuEntryBool* pBoolEntry = (SMenuEntryBool*)pEntry;

                  if( BP_DebugPad_Press(controllerPort, BP_PAD_B ) || //Cross
                      BP_DebugPad_Press(controllerPort, BP_PAD_L ) || 
                      BP_DebugPad_Press(controllerPort, BP_PAD_R ) )
                  {
                     *pBoolEntry->pValue = !(*pBoolEntry->pValue);

                     if( pEntry->callback )
                     {
                        pEntry->callback(pEntry->callbackData);
                     }
                  }
               }
               break;

            case kMET_Float:
               {
                  SMenuEntryFloat* pFloatEntry = (SMenuEntryFloat*)pEntry;

                  float direction = 0.0f;

                  if( BP_DebugPad_Repeat(controllerPort, BP_PAD_L))
                     direction = -1.0f;
                  else if( BP_DebugPad_Repeat(controllerPort, BP_PAD_R) )
                     direction = 1.0f;

                  if( direction != 0.0f )
                  {
                     float const scale = (BP_DebugPad_Status(controllerPort, BP_PAD_R1)) ? pFloatEntry->stepBig : pFloatEntry->stepSmall;
                  
                     float value = *pFloatEntry->pValue + scale * direction;
                     *pFloatEntry->pValue = bpe::min_val( bpe::max_val(value, pFloatEntry->min), pFloatEntry->max);

                     if( pEntry->callback )
                     {
                        pEntry->callback(pEntry->callbackData);
                     }
                  }
               }
               break;

            case kMET_Int:
               {
                  SMenuEntryInt* pIntEntry = (SMenuEntryInt*)pEntry;

                  int direction = 0;

                  if( BP_DebugPad_Repeat(controllerPort, BP_PAD_L))
                     direction = -1;
                  else if( BP_DebugPad_Repeat(controllerPort, BP_PAD_R) )
                     direction = 1;

                  if( direction != 0 )
                  {
                     int const scale = BP_DebugPad_Status(controllerPort, BP_PAD_R1) ? pIntEntry->stepBig : pIntEntry->stepSmall;

                     int value = *pIntEntry->pValue + scale * direction;
                     *pIntEntry->pValue = bpe::min_val( bpe::max_val(value, pIntEntry->min), pIntEntry->max);

                     if( pEntry->callback )
                     {
                        pEntry->callback(pEntry->callbackData);
                     }
                  }
               }
               break;

            case kMET_Enum:
               {
                  SMenuEntryEnum* pEnumEntry = (SMenuEntryEnum*)pEntry;

                  int direction = 0;

                  if( BP_DebugPad_Repeat(controllerPort, BP_PAD_L) )
                     direction = -1;
                  else if( BP_DebugPad_Repeat(controllerPort, BP_PAD_R) )
                     direction = 1;

                  if( direction != 0 )
                  {
                     int value = *pEnumEntry->pValue + direction;
                     if( value < pEnumEntry->min )
                        value = pEnumEntry->max;
                     else if( value > pEnumEntry->max )
                        value = pEnumEntry->min;
                     *pEnumEntry->pValue = value;

                     if( pEntry->callback )
                     {
                        pEntry->callback(pEntry->callbackData);
                     }
                  }
               }
               break;

            case kMET_Action:
               {
                  SMenuEntry* pActionEntry = (SMenuEntry*)pEntry;

                  if( BP_DebugPad_Press(controllerPort, BP_PAD_B) ) //Cross
                  {
                     if( pActionEntry->callback )
                     {
                        if( pActionEntry->callback(pActionEntry->callbackData) )
                        {
                           gCurrentMenuStackDepth = 1;
                           BP_DebugMenu_SetActive(0);
                        }

                        return;
                     }
                  }
               }
               break;
            }
         }

         // Move cursor up down?
         int dir = 0;
         
         if( BP_DebugPad_Repeat(controllerPort, BP_PAD_U) )
         {
            dir = -1;
         }
         else if( BP_DebugPad_Repeat(controllerPort, BP_PAD_D) )
         {
            dir = +1;
         }

         if( dir )
         {
            // Move cursor, skipping separators and disabled entries
            const SMenuEntry * pEntry;
            do 
            {
               currentRow += dir;
               while( currentRow < 0 )
                  currentRow += count;
               currentRow = currentRow % count;
               pEntry = rows[currentRow].pEntry;
            } while( (pEntry) && ( (pEntry->type == kMET_Separator) || ((pEntry->pEnabled) && (*pEntry->pEnabled == 0)) ) );
         }
      }

      if( BP_DebugPad_Press(controllerPort, BP_PAD_A) ) // Circle
      {
         if( gCurrentMenuStackDepth > 1 )
         {
            gCurrentMenuStackDepth--;
            return;
         }
      }
   }
}

//----------------------------------------------------------------------------

void BP_DebugMenu_Print(const char * pString, int x, int y, CColor const & color)
{
   using namespace DebugMenu;

   if(gPrintMeasure)
   {
      const CVector2 ext = gpRenderBackend->GetTextExtents(pString);
      gPrintMinX = bpe::min_val(gPrintMinX, x);
      gPrintMinY = bpe::min_val(gPrintMinY, y);
      gPrintMaxX = bpe::max_val(gPrintMaxX, x + (int)ext.GetX());
      gPrintMaxY = bpe::max_val(gPrintMaxY, y + (int)ext.GetY());
   }
   else
   {
      gpRenderBackend->RenderText(pString, (real32)x, (real32)y, kRenderFonts_Console, color);
   }
}

void BP_DebugMenu_PrintLine(const char * pString, int& x, int& y, CColor const & color)
{
   using namespace DebugMenu;

   if(gPrintMeasure)
   {
      const CVector2 ext = gpRenderBackend->GetTextExtents(pString);
      gPrintMinX = bpe::min_val(gPrintMinX, x);
      gPrintMinY = bpe::min_val(gPrintMinY, y);
      gPrintMaxX = bpe::max_val(gPrintMaxX, x + (int)ext.GetX());
      gPrintMaxY = bpe::max_val(gPrintMaxY, y + (int)ext.GetY());
   }
   else
   {
      gpRenderBackend->RenderText(pString, (real32)x, (real32)y, kRenderFonts_Console, color);
   }
   y += 16;
}

//----------------------------------------------------------------------------

void BP_DebugMenu_PrintMenu(bool isActive, SMenu const * pMenu, int & x, int & y)
{
   char buffer[256];
   sprintf(buffer, ">%s...", pMenu->pName);

   BP_DebugMenu_PrintLine(buffer, x, y, isActive ? CColor::Yellow() : CColor::Grey());
}

//----------------------------------------------------------------------------

void BP_DebugMenu_PrintMenuEntryBool(SMenuEntryBool const * pEntry, int & x, int & y, CColor const & color)
{
   char buffer[256];
   sprintf(buffer, "%-30s : %s", pEntry->entry.pName, *pEntry->pValue ? "True" : "False");

   BP_DebugMenu_PrintLine(buffer, x, y, color);
}

//----------------------------------------------------------------------------

void BP_DebugMenu_PrintMenuEntryFloat(SMenuEntryFloat const * pEntry, int & x, int & y, CColor const & color)
{
   char buffer[256];
   sprintf(buffer, "%-30s : %.3f", pEntry->entry.pName, *pEntry->pValue);

   BP_DebugMenu_PrintLine(buffer, x, y, color);
}

//----------------------------------------------------------------------------

void BP_DebugMenu_PrintMenuEntryInt(SMenuEntryInt const * pEntry, int & x, int & y, CColor const & color)
{
   char buffer[256];
   sprintf(buffer, "%-30s : %d", pEntry->entry.pName, *pEntry->pValue);

   BP_DebugMenu_PrintLine(buffer, x, y, color);
}

//----------------------------------------------------------------------------

void BP_DebugMenu_PrintMenuEntryEnum(SMenuEntryEnum const * pEntry, int & x, int & y, CColor const & color)
{
   char buffer[256];
   sprintf(buffer, "%-30s : (%d)%s", pEntry->entry.pName, *pEntry->pValue, pEntry->ppEnums[*pEntry->pValue - pEntry->min]);

   BP_DebugMenu_PrintLine(buffer, x, y, color);
}

//----------------------------------------------------------------------------

void BP_DebugMenu_PrintMenuEntryString(SMenuEntryString const * pEntry, int & x, int & y, CColor const & color)
{
   char buffer[256];
   sprintf(buffer, "%-30s : %s", pEntry->entry.pName, pEntry->pValue);

   BP_DebugMenu_PrintLine(buffer, x, y, color);
}

//----------------------------------------------------------------------------

void BP_DebugMenu_PrintMenuEntry(bool isActive, SMenuEntry const * pEntry, int & x, int & y)
{
   // Compute item color
   CColor color = isActive ? CColor::Yellow() : CColor::Grey();
   if( (pEntry->pEnabled) && (*pEntry->pEnabled == 0) )
   {
      color = isActive ? CColor::Beige() : CColor::Brown();
   }

   // Render text
   switch(pEntry->type)
   {
   case kMET_Bool:
      BP_DebugMenu_PrintMenuEntryBool((SMenuEntryBool const *)pEntry, x, y, color);
      break;

   case kMET_Float:
      BP_DebugMenu_PrintMenuEntryFloat((SMenuEntryFloat const *)pEntry, x, y, color);
      break;

   case kMET_Int:
      BP_DebugMenu_PrintMenuEntryInt((SMenuEntryInt const *)pEntry, x, y, color);
      break;

   case kMET_Enum:
      BP_DebugMenu_PrintMenuEntryEnum((SMenuEntryEnum const *)pEntry, x, y, color);
      break;

   case kMET_String:
      BP_DebugMenu_PrintMenuEntryString((SMenuEntryString const *)pEntry, x, y, color);
      break;   

   case kMET_Separator:
      y += 8;
      break;   

   default:
      BP_DebugMenu_PrintLine(pEntry->pName, x, y, color);
      break;
   }
}

//----------------------------------------------------------------------------

void BP_RenderMenu()
{
   using namespace DebugMenu;

   int currentX = 56;
   int currentY = 56;

   // Print menu heading
   {
      char buffer[1024];
      buffer[0] = 0;

      for( int i = 0; i < gCurrentMenuStackDepth; ++i )
      {
         if( i > 0 )
            strcat(buffer, "/");

         int curMenuIdx = gCurrentMenuStack[i];

         if( curMenuIdx == -1 )
            strcat(buffer, "Debug");
         else
            strcat(buffer, gMenus[curMenuIdx].pName);
      }

      strcat(buffer, ":");

      BP_DebugMenu_PrintLine(buffer, currentX, currentY, CColor::White());

      // Print separator with same length
      {
         for( char* ptr = buffer; *ptr; ++ptr )
            *ptr = '-';

         BP_DebugMenu_PrintLine(buffer, currentX, currentY, CColor::White());
      }

      currentX += 10;
   }

   // Render rows
   SMenuRow rows[128];
   int count = BP_DebugMenu_FlattenTree(gCurrentMenuStack[gCurrentMenuStackDepth - 1], rows, 128);
   if( count > 0 )
   {
      const int currentRow = BP_DebugMenu_GetCurrentRow();
      for( int i = 0; i < count; ++i )
      {
         SMenuRow const & row = rows[i];

         if( row.pMenu )
            BP_DebugMenu_PrintMenu(i == currentRow, row.pMenu, currentX, currentY);
         else
            BP_DebugMenu_PrintMenuEntry(i == currentRow, row.pEntry, currentX, currentY);
      }
   }
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_Render()
{
   using namespace DebugMenu;

   CSyncCriticalSectionLocker locker( &sDebugMenuReadWriteMutex );

   if( gBP_DebugMenuIsActive )
   {
      if( gBP_ShowDebugMenuBlackBackground )
      {
         // Measure menu display
         gPrintMeasure = true;
         gPrintMinX = 0x7fffffff;
         gPrintMinY = 0x7fffffff;
         gPrintMaxX = 0x80000000;
         gPrintMaxY = 0x80000000;
         BP_RenderMenu();
         gPrintMeasure = 0;

         // Compute normalized screen co-ords
         const int    border = 4;
         const real32 w  = gpRenderBackend->GetBackBufferWidth();
         const real32 h  = gpRenderBackend->GetBackBufferHeight();
         const real32 x0 = (2.0f * (real32)(gPrintMinX-border) / w) - 1.0f;
         const real32 x1 = (2.0f * (real32)(gPrintMaxX+border) / w) - 1.0f;
         const real32 y0 = 1.0f - (2.0f * (real32)(gPrintMinY-border) / h);
         const real32 y1 = 1.0f - (2.0f * (real32)(gPrintMaxY+border) / h);

         // Render quad
         gpRenderBackend->SetDepthCompareEnabled(false);
         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha);
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
         gpRenderBackend->SetAlphaTestEnable(false);
#endif
         BP_DrawRectTextureModulate((CBaseTexture*)&gpRenderBackend->GetWhiteMap(), x0, y0, x1-x0, y1-y0, CVector4(0, 0, 0, 0.75f), CVector4(0, 0, 0, 0), 0, 1);
         BP_EndShader();
      }

      // Render menu text
      BP_RenderMenu();
   }
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_AddMenu(const char* pName, int parentMenu)
{
   using namespace DebugMenu;

   if( gMenuCount < kMaxMenuCount )
   {
      gMenus[gMenuCount].index = gMenuCount;
      gMenus[gMenuCount].parentIdx = parentMenu;
      gMenus[gMenuCount].pName = pName;
      gMenuCount++;
      
      return gMenuCount - 1;
   }
   
   return -1;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_GetMenu(const char* pName)
{
   using namespace DebugMenu;

   for( int i = 0; i < DebugMenu::gMenuCount; ++i )
   {
      if( strcasecmp(gMenus[i].pName, pName) == 0 )
         return i;
   }
   
   return -1;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_AddFloat(int menu, const char* pName, float* pVar, float min, float max, float smallStep, float bigStep)
{
   using namespace DebugMenu;
   
   SMenuEntry* pEntry = AllocMenuEntry(sizeof(SMenuEntryFloat));
   pEntry->menu = menu;
   pEntry->type = kMET_Float;
   pEntry->pName = pName;

   SMenuEntryFloat* pFloatEntry = (SMenuEntryFloat*)pEntry;

   pFloatEntry->pValue = pVar;

   pFloatEntry->min = min;
   pFloatEntry->max = max;
   pFloatEntry->stepSmall = smallStep;
   pFloatEntry->stepBig = bigStep;

   return (int)pEntry;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_AddInt(int menu, const char* pName, int* pVar, int min, int max, int smallStep, int bigStep)
{
   using namespace DebugMenu;

   SMenuEntry* pEntry = AllocMenuEntry(sizeof(SMenuEntryInt));
   pEntry->menu = menu;
   pEntry->type = kMET_Int;
   pEntry->pName = pName;

   SMenuEntryInt* pIntEntry = (SMenuEntryInt*)pEntry;

   pIntEntry->pValue = pVar;

   pIntEntry->min = min;
   pIntEntry->max = max;
   pIntEntry->stepSmall = smallStep;
   pIntEntry->stepBig = bigStep;

   return (int)pEntry;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_AddEnum(int menu, const char* pName, const char** ppEnums, int* pVar, int min, int max)
{
   using namespace DebugMenu;

   SMenuEntry* pEntry = AllocMenuEntry(sizeof(SMenuEntryEnum));
   pEntry->menu = menu;
   pEntry->type = kMET_Enum;
   pEntry->pName = pName;

   SMenuEntryEnum* pEnumEntry = (SMenuEntryEnum*)pEntry;

   pEnumEntry->ppEnums = ppEnums;
   pEnumEntry->pValue = pVar;
   pEnumEntry->min = min;
   pEnumEntry->max = max;

   return (int)pEntry;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_AddBool(int menu, const char* pName, int* pVar)
{
   using namespace DebugMenu;
   
   SMenuEntry* pEntry = AllocMenuEntry(sizeof(SMenuEntryBool));
   pEntry->menu = menu;
   pEntry->type = kMET_Bool;
   pEntry->pName = pName;
   
   SMenuEntryBool* pBoolEntry = (SMenuEntryBool*)pEntry;
   pBoolEntry->pValue = pVar;

   return (int)pEntry;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_AddAction(int menu, const char* pName, int (*callback)(int), int callbackData)
{
   using namespace DebugMenu;

   SMenuEntry* pEntry = AllocMenuEntry(sizeof(SMenuEntryAction));
   pEntry->menu = menu;
   pEntry->type = kMET_Action;
   pEntry->pName = pName;
   pEntry->callback = callback;
   pEntry->callbackData = callbackData;

   return (int)pEntry;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_AddString(int menu, const char* pName, const char* pVar)
{
   using namespace DebugMenu;
   
   SMenuEntry* pEntry = AllocMenuEntry(sizeof(SMenuEntryString));
   pEntry->menu = menu;
   pEntry->type = kMET_String;
   pEntry->pName = pName;   
   
   SMenuEntryString * pStringEntry = (SMenuEntryString*)pEntry;
   pStringEntry->pValue = pVar;

   return (int)pEntry;
}

//----------------------------------------------------------------------------

extern "C" int BP_DebugMenu_AddSeparator(int menu)
{
   using namespace DebugMenu;

   SMenuEntry* pEntry = AllocMenuEntry(sizeof(SMenuEntrySeparator));
   pEntry->menu = menu;
   pEntry->type = kMET_Separator;

   return (int)pEntry;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_SetEnabled(int item, const int* pEnabled)
{
   SMenuEntry* pEntry = (SMenuEntry*)item;
   pEntry->pEnabled = pEnabled;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_SetCallback(int item, int (*callback)(int), int callbackData)
{
   SMenuEntry* pEntry = (SMenuEntry*)item;
   pEntry->callback = callback;
   pEntry->callbackData = callbackData;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_SetName(int item, const char* pName)
{
   SMenuEntry* pEntry = (SMenuEntry*)item;
   pEntry->pName = pName;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_SetFloatVar(int item, float* pVar)
{
   SMenuEntryFloat* pEntry = (SMenuEntryFloat*)item;
   pEntry->pValue = pVar;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_SetIntVar(int item, int* pVar)
{
   SMenuEntryInt* pEntry = (SMenuEntryInt*)item;
   pEntry->pValue = pVar;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_SetEnumVar(int item, int* pVar)
{
   SMenuEntryEnum* pEntry = (SMenuEntryEnum*)item;
   pEntry->pValue = pVar;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_SetBoolVar(int item, int* pVar)
{
   SMenuEntryBool* pEntry = (SMenuEntryBool*)item;
   pEntry->pValue = pVar;
}

//----------------------------------------------------------------------------

extern "C" void BP_DebugMenu_SetStringVar(int item, const char* pVar)
{
   SMenuEntryString* pEntry = (SMenuEntryString*)item;
   pEntry->pValue = pVar;
}

//----------------------------------------------------------------------------

void BP_Debug_DrawString(const char * string, int* currentX, int* currentY, int const flags, CColor const & color )
{
   int const lineSeparation = 14;
   gpRenderBackend->RenderText( string, (real32)*currentX, (real32)*currentY, kRenderFonts_Console, color, flags);
   *currentY += lineSeparation;
}

//----------------------------------------------------------------------------

namespace Debug
{
   CSyncCriticalSection debugStringCriticalSection;
   std::vector<std::string> debugStrings;
}

//----------------------------------------------------------------------------
extern "C" void BP_DebugText_Puts(char *str)
{
   CSyncCriticalSectionLocker lock(Debug::debugStringCriticalSection);
   Debug::debugStrings.push_back(str);
}

extern "C" void BP_DebugText_Print(const char * fmt, ...)
{
   CSyncCriticalSectionLocker lock(Debug::debugStringCriticalSection);

   static char tmp[1024];

   va_list args;
   va_start( args, fmt );

   vsprintf( tmp, fmt, args );

   va_end( args );

   Debug::debugStrings.push_back(tmp);
}

//----------------------------------------------------------------------------

void BP_DebugText_Clear()
{
   CSyncCriticalSectionLocker lock(Debug::debugStringCriticalSection);

   Debug::debugStrings.clear();
}

//----------------------------------------------------------------------------

int BP_DebugText_Render(int const x, int const y, int flags)
{
   CSyncCriticalSectionLocker lock(Debug::debugStringCriticalSection);

   int currentX = x;
   int currentY = y;

   for( uint32 i = 0; i < Debug::debugStrings.size(); ++i )
   {
      BP_Debug_DrawString(Debug::debugStrings[i].c_str(), &currentX, &currentY, flags);
   }

   return currentY;
}

//----------------------------------------------------------------------------

#endif
