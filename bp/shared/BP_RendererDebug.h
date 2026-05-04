//----------------------------------------------------------------------------
// BP_RendererDebug.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_DEBUG_MENU

#  if defined(GOLD_VERSION)
#     error BP_ENABLE_DEBUG_MENU can't be defined with GOLD_VERSION!
#  endif

int BP_DebugMenu_IsActive();
int BP_DebugMenu_GetActiveMenu();

void BP_DebugMenu_ProcessInput(int controllerPort);
void BP_DebugMenu_Render();

int BP_DebugMenu_AddMenu(const char* pName, int parentMenu);
int BP_DebugMenu_GetMenu(const char* pName);

int BP_DebugMenu_AddFloat(int menu, const char* pName, float* pVar, float min, float max, float smallStep, float bigStep);
int BP_DebugMenu_AddInt(int menu, const char* pName, int* pVar, int min, int max, int smallStep, int bigStep);
int BP_DebugMenu_AddEnum(int menu, const char* pName, const char** ppEnums, int* pVar, int min, int max);
int BP_DebugMenu_AddBool(int menu, const char* pName, int* pVar);

int BP_DebugMenu_AddAction(int menu, const char* pName, int (*callback)(int), int callbackData);
int BP_DebugMenu_AddString(int menu, const char* pName, const char* pVar);

int BP_DebugMenu_AddSeparator(int menu);

void BP_DebugMenu_SetEnabled(int item, const int* pEnabled);
void BP_DebugMenu_SetCallback(int item, int (*callback)(int), int callbackData);
void BP_DebugMenu_SetName(int item, const char* pName);

void BP_DebugMenu_SetFloatVar(int item, float* pVar);
void BP_DebugMenu_SetIntVar(int item, int* pVar);
void BP_DebugMenu_SetEnumVar(int item, int* pVar);
void BP_DebugMenu_SetBoolVar(int item, int* pVar);
void BP_DebugMenu_SetStringVar(int item, const char* pVar);

#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus

#ifndef GOLD_VERSION
void BP_Debug_DrawString(const char * string, int* currentX, int* currentY, int const flags = 0, CColor const & color = CColor::White() );
#else
#define BP_Debug_DrawString
#endif

#endif

//----------------------------------------------------------------------------

#if !defined(GOLD_VERSION)
void BP_DebugText_Print(const char * fmt, ...);
void BP_DebugText_Puts(char *str);

#ifdef __cplusplus
void BP_DebugText_Clear();
int BP_DebugText_Render(int const x, int const y, int flags);
#endif

#else

static inline void BP_DebugText_Print(const char * fmt, ...) {}
static inline void BP_DebugText_Puts(char *str) {}

#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
