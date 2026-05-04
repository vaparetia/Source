//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   sk_printf.c 
   ????????
   
   2001/06/04	S.Kobayashi
   $Id: sk_printf.c,v 1.1.1.3 2002/11/19 11:50:25 Yoshizawa1 Exp $
*/

#if 0 //BP_PS2 def PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"

static short node_font_width[] = {  14 ,  10 ,  9 , 18 , 17 , 17 , 17 ,  5 ,  7 ,  7 , 17 , 15 ,  6 , 16 ,  6 , 15 ,
									18 ,  7 , 18 , 18 , 18 , 18 , 18 , 18 , 18 , 18 ,  6 ,  5 , 10 , 13 , 10 , 19 ,
									19 , 18 , 18 , 18 , 18 , 18 , 18 , 18 , 18 ,  5 , 18 , 18 , 18 , 19 , 18 , 18 ,
									18 , 18 , 18 , 18 , 18 , 18 , 19 , 19 , 18 , 18 , 18 ,  8 , 17 ,  8 , 18 , 14 ,
									16 , 18 , 16 , 16 , 16 , 16 , 16 , 16 , 16 ,  5 ,  9 , 16 ,  6 , 17 , 16 , 16 ,
									16 , 13 , 15 , 16 , 16 , 16 , 17 , 17 , 16 , 16 , 16 , 10 ,  5 , 10 ,  6 , 19 };

#define ONE_AFTER_SPACE (4) 
// ????????? by koba4 2001/08/01 AM 11:11
static void SK_PrintfNormal_main( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num , char mode )
{
	int tmp_u , tmp_v , ascci_tmp;
	int back_height;
   float back_width;
	int default_x , default_y;
	char ascci_buf;
	float raute_x;
	char  flag;
	int i;

	back_width = 0;
	back_height = 0;
	flag = 0;
	ascci_buf = 0;
	ascci_tmp = 0;
	default_x = pObj[ 0 ]->sprite.pos.x;
	default_y = pObj[ 0 ]->sprite.pos.y;
	raute_x = 1.0f;   //pObj[ 0 ]->sprite.dw / str_width;  //18.0f;
	for ( i = 0 ; i < num ; i ++ )
   {
      pObj[ i ]->sprite.dw = str_width*TARGET_ASPECT_X;
		pObj[ i ]->sprite.pos.x = default_x + back_width;
		pObj[ i ]->sprite.pos.y = default_y + back_height;
		if ( ascci != NULL && flag != 1 ){
			if ( ascci[ i ] >= 0x20 && ascci[ i ] <= 0x7f ){
				ascci_tmp = ascci[ i ] - 0x20;
			} else if ( ascci[ i ] == 0x0a ){
				back_height += pObj[ i ]->sprite.dh;
				back_width = 0;
				num--;
				continue;
			} else {
				flag = 1;
				ascci_tmp = 0;
			}
		} else {
			ascci_tmp = 0;
		}
		if ( ascci_tmp == 0 ){
			switch ( mode ){
			case 0 :
				back_width += ( (((float)node_font_width[ ascci_tmp ])*TARGET_ASPECT_X) * raute_x );
				break;
			case 1 :
				back_width += ( 6 * raute_x );
				break;
			}
		} else {
			if ( mode == 2 ){
				back_width += ( pObj[ 0 ]->sprite.dw * raute_x );
			} else {
            back_width += ( (((float)node_font_width[ ascci_tmp ])*TARGET_ASPECT_X) * raute_x );
			}
		}
		tmp_u = ( ascci_tmp & 0x1f ) * str_width * 16;
		tmp_v = ( ascci_tmp >> 5 ) * str_height * 16;
		// move
		{/* XBOX??? 2002.04.04yano */
			int du,dv,dw,dh;
			du = u + tmp_u;
			dv = v + tmp_v;
			dw = str_width * 16;
			dh = str_height * 16;
#if 0 //BP_PS2 def PSX2
			pObj[ i ]->sprite.head.tex.u = du;
			pObj[ i ]->sprite.head.tex.v = dv;
			pObj[ i ]->sprite.head.tex.w = dw;
			pObj[ i ]->sprite.head.tex.h = dh;
#else
         /*
			pObj[ i ]->sprite.head.tex.u = du / 16.0f;
			pObj[ i ]->sprite.head.tex.v = dv / 16.0f;
			pObj[ i ]->sprite.head.tex.w = node_font_width[ ascci_tmp ]-1;   //((float)(dw / 16.0f)) - 0.5f;
			pObj[ i ]->sprite.head.tex.h = dh / 16.0f;
         pObj[ i ]->sprite.dw = node_font_width[ ascci_tmp ]*TARGET_ASPECT_X;
*/
         pObj[ i ]->sprite.head.tex.u = du / 16.0f;
         pObj[ i ]->sprite.head.tex.v = dv / 16.0f;
         pObj[ i ]->sprite.head.tex.w = dw / 16.0f;
         pObj[ i ]->sprite.head.tex.h = dh / 16.0f;

#endif
		}
		// special
		if ( ascci != NULL ){
			if ( ( ascci_buf == '1' ) && ascci_tmp == 0x11 ){
				pObj[ i ]->sprite.pos.x += ( ONE_AFTER_SPACE * raute_x );
				back_width += ( ONE_AFTER_SPACE * raute_x );
			}
			ascci_buf = ascci[ i ];
		}
	}
}

void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num )
{
	SK_PrintfNormal_main( NULL , pObj , u , v , str_width , str_height , num , 1 ); // ???
	SK_PrintfNormal_main( ascci , pObj , u , v , str_width , str_height , num , 1 );
}

void SK_PrintfNormal2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num )
{
	SK_PrintfNormal_main( NULL , pObj , u , v , str_width , str_height , num , 0 ); // ???
	SK_PrintfNormal_main( ascci , pObj , u , v , str_width , str_height , num , 0 );
}

void SK_PrintfNormal3( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num )
{
	SK_PrintfNormal_main( NULL , pObj , u , v , str_width , str_height , num , 2 ); // ???
	SK_PrintfNormal_main( ascci , pObj , u , v , str_width , str_height , num , 2 );
}

float SK_PrintfNormalWidth( u_char ascci , SPR_OBJ *pObj )
{
	int ascci_tmp;

	if ( ( int )ascci >= 0x20 && ( int )ascci <= 0x7f ){
		ascci_tmp = ascci - 0x20;
	} else {
		ascci_tmp = 0;
	}
	// move
	if ( ascci_tmp == 0 ){
		return ( 0xff );
	} else {
		return ( ((float)node_font_width[ ascci_tmp ])*TARGET_ASPECT_X );
	}
}

float SK_PrintfNormalWidth_NameSelect( u_char ascci , SPR_OBJ *pObj )
{
   int ascci_tmp;

   if ( ( int )ascci >= 0x20 && ( int )ascci <= 0x7f ){
      ascci_tmp = ascci - 0x20;
   } else {
      ascci_tmp = 0;
   }
   // move
   if ( ascci_tmp == 0 ){
      return ( 0xff );
   } else {
      return ( ((float)node_font_width[ ascci_tmp ]) );
   }
}


float SK_PrintfNormalWidth2( u_char *ascci ) // ?? space??
{
	float size;
	int ascci_buf;
	int ascci_tmp;
	int i;

	size = 0.0f;
	ascci_buf = 0;
	i = 0;
	while ( ascci[ i ] != '\0' ){
		if ( ( int )ascci[ i ] >= 0x20 && ( int )ascci[ i ] <= 0x7f ){
			ascci_tmp = ascci[ i ] - 0x20;
		} else {
			ascci_tmp = 0;
		}
		size += ((float)node_font_width[ ascci_tmp ])*TARGET_ASPECT_X;
		if ( ( ascci_buf == '1' ) && ascci_tmp == 0x11 ){
			size += ONE_AFTER_SPACE;
		}
		ascci_buf = ascci[ i ];
		i++;
	}
	return ( size );
}

float SK_PrintfNormalWidth3( u_char *ascci , int max_position ) // ?? space??
{
	float size;
	int ascci_buf;
	int ascci_tmp;
	int i;

	size = 0.0f;
	ascci_buf = 0;
	i = 0;
	while ( ascci[ i ] != '\0' && i < max_position ){
		if ( ( int )ascci[ i ] >= 0x20 && ( int )ascci[ i ] <= 0x7f ){
			ascci_tmp = ascci[ i ] - 0x20;
		} else {
			ascci_tmp = 0;
		}
		size += ((float)node_font_width[ ascci_tmp ]*TARGET_ASPECT_X);
		if ( ( ascci_buf == '1' ) && ( ascci[ i ] == '1' ) ){
			size += ONE_AFTER_SPACE;
		}
		ascci_buf = ascci[ i ];
		i++;
	}
	if ( ( ascci_buf == '1' ) && ( ascci[ max_position ] == '1' ) ){
		size += ONE_AFTER_SPACE;
	}

	return ( size );
}

void SK_PrintfNum( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num ) // '/' ??
{
	int tmp_u , tmp_v , ascci_tmp;
	int back_width;
	int default_x;
	char ascci_buf;
	float raute_x;
	char  flag;
	int i;

	back_width = 0;
	flag = 0;
	ascci_buf = 0;
	default_x = pObj[ 0 ]->sprite.pos.x;
	raute_x = pObj[ 0 ]->sprite.dw / 18.0f;
   raute_x = 1.0f;
	for ( i = 0 ; i < num ; i ++ ){
		pObj[ i ]->sprite.pos.x = default_x + back_width;
		if ( ascci != NULL && ascci[ i ] >= 0x30 && ascci[ i ] <= 0x39 && flag != 1 ){
			ascci_tmp = ascci[ i ] - 0x30;
		} else if ( ascci != NULL && ascci[ i ] == 0x2f && flag != 1 ){
			if ( ascci[ i ] == 0x2f ){
				ascci_tmp = 0x3a - 0x30;
			}
		} else {
			flag = 1;
			ascci_tmp = 0;
		}			
		tmp_u = ( ascci_tmp & 0x1f ) * str_width * 16;
		tmp_v = ( ascci_tmp >> 5 ) * str_height * 16;
		// move
		pObj[ i ]->sprite.head.tex.u = (SPR_FIX)( u + tmp_u );
		pObj[ i ]->sprite.head.tex.v = (SPR_FIX)( v + tmp_v );
		pObj[ i ]->sprite.head.tex.w = (SPR_FIX)( str_width * 16 );
		pObj[ i ]->sprite.head.tex.h = (SPR_FIX)( str_height * 16 );
#if 1 //BP_XBOX def KP_XBOX
		pObj[ i ]->sprite.head.tex.u = pObj[ i ]->sprite.head.tex.u / 16.0f;
		pObj[ i ]->sprite.head.tex.v = pObj[ i ]->sprite.head.tex.v / 16.0f;
		pObj[ i ]->sprite.head.tex.w = pObj[ i ]->sprite.head.tex.w / 16.0f;
		pObj[ i ]->sprite.head.tex.h = pObj[ i ]->sprite.head.tex.h / 16.0f; 
#endif
		back_width += pObj[ i ]->sprite.dw;
	}
}

// ????(UV chenge)
void SK_Printf( u_char ascci , SPR_OBJ *pObj , int u , int v , int str_width , int str_height )
{
	int tmp_u , tmp_v;

	ascci -= 0x20;
	tmp_u = ( ascci & 0x1f ) * str_width * 16;
	tmp_v = ( ( ascci >> 5 ) & 0x1 ) * str_height * 16;
	// move
	pObj->sprite.head.tex.u = u + tmp_u;
	pObj->sprite.head.tex.v = v + tmp_v;
	pObj->sprite.head.tex.w = str_width * 16;
	pObj->sprite.head.tex.h = str_height * 16;
#if 1 //BP_XBOX def KP_XBOX
	pObj->sprite.head.tex.u = pObj->sprite.head.tex.u / 16.0f;
	pObj->sprite.head.tex.v = pObj->sprite.head.tex.v / 16.0f;
	pObj->sprite.head.tex.w = pObj->sprite.head.tex.w / 16.0f;
	pObj->sprite.head.tex.h = pObj->sprite.head.tex.h / 16.0f; 
#endif
}

// ????(UV chenge)???????
// ???????????
static short font_width[] = {    9 ,  7 , 10 , 13 , 12 , 17 , 15 ,  5 ,  8 ,  8 ,  9 , 13 , 10 , 14 ,  6 ,  9 ,
							    11 , 10 , 12 , 12 , 12 , 11 , 12 , 12 , 12 , 11 ,  5 ,  5 , 13 , 13 , 13 , 12 ,
								17 , 14 , 14 , 14 , 14 , 13 , 13 , 16 , 13 ,  6 , 12 , 14 , 12 , 16 , 14 , 15 ,
								13 , 16 , 14 , 13 , 13 , 14 , 14 , 17 , 14 , 14 , 13 ,  7 , 14 ,  7 , 11 , 13 ,
								 0 , 11 , 12 , 11 , 12 , 12 ,  8 , 12 , 12 ,  7 ,  7 , 12 ,  6 , 17 , 11 , 12 ,
								12 , 12 ,  8 , 11 ,  9 , 12 , 12 , 17 , 11 , 12 , 11 ,  9 ,  4 ,  9 ,  8 ,  0 }; 
#define I_AFTER_SPACE (0)
#define L_AFTER_SPACE (0)
void SK_Printf2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num )
{
	int tmp_u , tmp_v , ascci_tmp;
	int back_width;
	int default_x;
	char ascci_buf;
	float raute_x;
	char  flag;
	int i;

	back_width = 0;
	flag = 0;
	ascci_buf = 0;
	default_x = pObj[ 0 ]->sprite.pos.x;
	raute_x = pObj[ 0 ]->sprite.dw / 16.0f;
	for ( i = 0 ; i < num ; i ++ ){
		pObj[ i ]->sprite.pos.x = default_x + back_width;
		if ( ascci != NULL && ascci[ i ] >= 0x20 && ascci[ i ] <= 0x7f && flag != 1 ){
			ascci_tmp = ascci[ i ] - 0x20;
		} else {
			flag = 1;
			ascci_tmp = 0;
		}
		tmp_u = ( ascci_tmp & 0x1f ) * str_width * 16;
		tmp_v = ( ascci_tmp >> 5 ) * str_height * 16;
		// move
#if 0 //BP_PS2 def PSX2
		pObj[ i ]->sprite.head.tex.u = u + tmp_u;
		pObj[ i ]->sprite.head.tex.v = v + tmp_v;
#else
		pObj[ i ]->sprite.head.tex.u = (u + tmp_u) / 16.0f;
		pObj[ i ]->sprite.head.tex.v = (v + tmp_v) / 16.0f;
//		pObj[ i ]->sprite.head.tex.u = pObj[ i ]->sprite.head.tex.u / 16.0f; 
//		pObj[ i ]->sprite.head.tex.v = pObj[ i ]->sprite.head.tex.v / 16.0f;
#endif
		back_width += ( font_width[ ascci_tmp ] * raute_x );
		// special
		if ( ascci != NULL ){
			if ( ( ascci_buf == 'i' || ascci_buf == 'I' ) && ascci_tmp == 0 ){
				back_width += ( I_AFTER_SPACE * raute_x );
			}
			if ( ( ascci_buf == 'l' || ascci_buf == 'L' ) && ascci_tmp == 0 ){
				back_width += ( L_AFTER_SPACE * raute_x );
			}
			ascci_buf = ascci[ i ];
		}
		// w h
#if 0 //BP_PS2 def PSX2
 		pObj[ i ]->sprite.head.tex.w = str_width * 16;
		pObj[ i ]->sprite.head.tex.h = str_height * 16;
#else
 		pObj[ i ]->sprite.head.tex.w = str_width ;
		pObj[ i ]->sprite.head.tex.h = str_height ;
#endif
	}
}

void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ) // ??????????
{
	int i;

	for ( i = 0 ; i < num ; i ++ ){
		pObj[ i ]->sprite.col.r = r;
		pObj[ i ]->sprite.col.g = g;
		pObj[ i ]->sprite.col.b = b;
		pObj[ i ]->sprite.col.a = a;
	}
}

void SK_AllHide( SPR_OBJ **pObj , int num ) // ??????????
{
	int i;

	for ( i = 0 ; i < num ; i ++ ){
		SPR_HIDE( pObj[ i ] );
	}
}

void SK_AllShow( SPR_OBJ **pObj , int num ) // ??????????
{
	int i;

	for ( i = 0 ; i < num ; i ++ ){
		SPR_SHOW( pObj[ i ] );
	}
}

void SK_PrintfChengColor( SPR_OBJ *pObj , u_char r , u_char g , u_char b , u_char a )
{
	pObj->sprite.col.r = r;
	pObj->sprite.col.g = g;
	pObj->sprite.col.b = b;
	pObj->sprite.col.a = a;
}

void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode )
{
	MENU_S_Locate( x , y , mode );
	MENU_S_Color( color->r , color->g , color->b , color->cd );
	MENU_S_Printf( str );
}
