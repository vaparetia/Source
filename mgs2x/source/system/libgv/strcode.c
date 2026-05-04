//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	strcode.c
		文字列を数字に変換する
		MGS2は24BitCode を使用
		作成される値は正で０ではない

	1999/05/31 K.Uehara
	$Id: strcode.c,v 1.1.1.3 2002/11/19 11:42:45 Yoshizawa1 Exp $
*/

#include <stdio.h>

#define BIT_LEN		24

int GV_StrCode( char const *string )
{
	unsigned char c;
	unsigned char *p;
	unsigned int id, mask;

	p = ( unsigned char * )string;
	id = 0;
	mask = ( 1 << BIT_LEN ) - 1;

	while( ( c = *( p++ ) ) != '\0' ){
		id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
		id += c;
		id &= mask;
	}
	if( id == 0 ) id = 1;

	return id;
}
