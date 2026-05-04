//----------------------------------------------------------------------------
// BP_LocalizedTextByEnum.cpp
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"
#include "BP_LocalizedTextByEnum.h"
#include "Engine/System/COsContext.h"
 
#include "BP_LocalizedTextByEnumGeneric.h"

#if BPE_TARGET == BPE_TARGET_X360
#include "BP_LocalizedTextByEnumX360.h"
#elif BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_WIN32
#include "BP_LocalizedTextByEnumPS3.h"
#elif  BPE_TARGET == BPE_TARGET_VITA
#include "BP_LocalizedTextByEnumVita.h"
#else
#error unknown platform
#endif

BPE_CTASSERT( sizeof(le_remapped_text) == sizeof(lj_remapped_text) );
BPE_CTASSERT( sizeof(le_remapped_text) == sizeof(lf_remapped_text) );
BPE_CTASSERT( sizeof(le_remapped_text) == sizeof(lg_remapped_text) );
BPE_CTASSERT( sizeof(le_remapped_text) == sizeof(li_remapped_text) );
BPE_CTASSERT( sizeof(le_remapped_text) == sizeof(ls_remapped_text) );
BPE_CTASSERT( sizeof(le_remapped_text)/sizeof(le_remapped_text[0]) == kBP_GLS_Count - BP_LOCALIZEDTEXT_ENUM_START );

BPE_CTASSERT( sizeof(le_remapped_text_platform) == sizeof(lj_remapped_text_platform) );
BPE_CTASSERT( sizeof(le_remapped_text_platform) == sizeof(lf_remapped_text_platform) );
BPE_CTASSERT( sizeof(le_remapped_text_platform) == sizeof(lg_remapped_text_platform) );
BPE_CTASSERT( sizeof(le_remapped_text_platform) == sizeof(li_remapped_text_platform) );
BPE_CTASSERT( sizeof(le_remapped_text_platform) == sizeof(ls_remapped_text_platform) );
BPE_CTASSERT( sizeof(le_remapped_text_platform)/sizeof(le_remapped_text_platform[0]) == kBP_PLS_Count - BP_LOCALIZEDTEXT_ENUM_START_PLATFORM );

char** a_ls_ramapped_text[] =
{
   le_remapped_text, //kL_English
   lj_remapped_text, //kL_Japanese
   lf_remapped_text, //kL_French
   lg_remapped_text, //kL_German
   li_remapped_text, //kL_Italian
   NULL,             //kL_Portuguese
   ls_remapped_text  //kL_Spanish
};

char** a_ls_ramapped_text_platform[] =
{
   le_remapped_text_platform, //kL_English
   lj_remapped_text_platform, //kL_Japanese
   lf_remapped_text_platform, //kL_French
   lg_remapped_text_platform, //kL_German
   li_remapped_text_platform, //kL_Italian
   NULL,                      //kL_Portuguese
   ls_remapped_text_platform  //kL_Spanish
};                            

char* BP_GetStringForEnum( unsigned int id_ )
{
   BPE_VERIFY( id_ >= BP_LOCALIZEDTEXT_ENUM_START, false, "BP_GetStringForEnum:Invalid id")

   unsigned int id = id_;
   char** pArray = NULL;
   CBaseOsContext::ELanguage const language = gpOsContext->mLanguage;

   if( id_ >= BP_LOCALIZEDTEXT_ENUM_START_PLATFORM )
   {
      id -= BP_LOCALIZEDTEXT_ENUM_START_PLATFORM;
      pArray = a_ls_ramapped_text_platform[language];
      BPE_VERIFY( id <= kBP_PLS_Count - BP_LOCALIZEDTEXT_ENUM_START_PLATFORM, false, "BP_GetStringForEnum:Invalid id" );
   }
   else
   {
      id -= BP_LOCALIZEDTEXT_ENUM_START;
      pArray = a_ls_ramapped_text[language];
      BPE_VERIFY( id < kBP_GLS_Count - BP_LOCALIZEDTEXT_ENUM_START, false, "BP_GetStringForEnum:Invalid id" );
   }
   return pArray[id];
}
