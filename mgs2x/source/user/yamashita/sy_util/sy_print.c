//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * sy_util - sy_print.c
 * テキスト出力
 * 2002/03/12 S.Yamashita
 * $Id: sy_print.c,v 1.1.1.3 2002/11/19 11:51:53 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "sy_util.h"

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * テキスト出力
 */
void sy_printf(
	char *format,	/* テキストの書式 */
	...)			/* 可変引数 */
{
#if defined(_SY_UTIL_DEBUG)
	char    text[_SY_STRING_MAX * 5];
	va_list valist;

	memset(text, 0x00, sizeof(text));
	va_start(valist, format);
	vsprintf(text, format, valist);
	va_end(valist);

	text[_SY_STRING_MAX - 1] = '\0';
	printf(text);
#endif
}

/*******************************************************************************
 * 名前付テキスト出力
 */
void sy_printnf(
	char *name,		/* 名前 */
	char *format,	/* テキストの書式 */
	...)			/* 可変引数 */
{
#if defined(_SY_UTIL_DEBUG)
	char    text[_SY_STRING_MAX * 5];
	va_list valist;

	memset(text, 0x00, sizeof(text));
	va_start(valist, format);
	vsprintf(text, format, valist);
	va_end(valist);

	text[_SY_STRING_MAX - 1] = '\0';
	printf("[%s] %s", name, text);
#endif
}
