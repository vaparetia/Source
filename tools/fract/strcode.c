/*
	strcode.c
		文字列を数字に変換する
		MGS2は24BitCode を使用
		作成される値は正で０ではない

	1999/05/31 K.Uehara
	$Id: strcode.c,v 1.2 2000/04/24 06:52:22 usr04098 Exp $
*/

#include <stdio.h>

#define BIT_LEN		24

int GV_StrCode( char *string )
{
    unsigned char *p ;
    unsigned int id, mask ;

    p = ( unsigned char * )string;
    id = 0 ;
    mask = ( 1 << BIT_LEN ) - 1 ;

    for ( ; *p != '\0' && *p != '.' ; p++ )
    {
	id = ( id << 5 ) | ( id >> (BIT_LEN-5) ) ;
	id += *p ;
	id &= mask ;
    }
    if ( id == 0 )
	id = 1 ;

    return id ;
}
