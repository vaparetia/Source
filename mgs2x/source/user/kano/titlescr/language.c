//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	language.c
		言語選択

	2001/10/22 K.Kano
	$Id: language.c,v 1.1.1.3 2002/11/19 11:43:38 Yoshizawa1 Exp $
*/


#include "titlescr.h"

#ifdef PSX2
#include <libscf.h>
#endif


int ComGetLanguage(void)
{

   BP_TODO_BREAK;
#if 0 //BP_TODO

#ifdef PSX2	
	switch(sceScfGetLanguage()){

#if 0
	case SCE_SPANISH_LANGUAGE:
	default:
		GM_Language=GM_LANG_SPANISH;
		break;

#elif 0
	case SCE_ITALIAN_LANGUAGE:
	default:
		GM_Language=GM_LANG_ITALY;
		break;

#else

	case SCE_ENGLISH_LANGUAGE:
		GM_Language=GM_LANG_ENGLISH;
		break;
	case SCE_FRENCH_LANGUAGE:
		GM_Language=GM_LANG_FRENCH;
		break;
	case SCE_GERMAN_LANGUAGE:
		GM_Language=GM_LANG_GERMANY;
		break;

	default:
		GM_Language=GM_LANG_ENGLISH;
		break;

#endif
	}
#else
	switch( XGetLanguage() ) {
#if 0		
	case XC_LANGUAGE_JAPANESE:
		GM_Language = GM_LANG_JAPANESE;
		break;
#endif		
	case XC_LANGUAGE_GERMAN:
		GM_Language = GM_LANG_GERMANY;
		break;
	case XC_LANGUAGE_FRENCH:
		GM_Language = GM_LANG_FRENCH;
		break;
	case XC_LANGUAGE_SPANISH:
		GM_Language = GM_LANG_SPANISH;
		break;
	case XC_LANGUAGE_ITALIAN:
		GM_Language = GM_LANG_ITALY;
		break;
	default:
		GM_Language = GM_LANG_ENGLISH;
		break;
	}
#endif

#endif //BP

	return 0;
}
