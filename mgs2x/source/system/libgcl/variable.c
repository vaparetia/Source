//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   変数処理
	1999/07/08 K.Uehara
	$Id: variable.c,v 1.1.1.3 2002/11/19 11:42:42 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>

#include	"libgcl.h"

//#define __VARIABLE_C__

#include	"linkvar.h"
//#include	"../menu/memory.h"

#include "BP_TrophySystem.h"
#include "BP_TrophyLogicMGS2.h"
#include "BP_Debug.h"//BP_GAMEPLAY
#include "../../user/kano/titlescr/titlescr.h"//BP_TROPHY
#include "BP_EndianSupport.h"

#include "BP_Transfarring.h"

/* この中で short linkvarbuf[ MAX_LINKVARBUF ]; を宣言。ゲームシステムとリンクする変数群 */


#ifdef NO_VARIABLE_SHIFT

/* 通常変数領域 */
static char linkvarbuf_body[ MAX_LINKVARBUF ];
static char var_buf[ MAX_VAR_BUF ];

/* セーブ用 */

static char		sv_linkvarbuf[ MAX_LINKVARBUF ];
static char		sv_var_buf[ MAX_VAR_BUF ];

#else

// アクションリプレイ対策で変数領域を動かす

char gcl_variable_buf[ ( MAX_LINKVARBUF + MAX_VAR_BUF ) * 2 + 1024 ];

char *linkvarbuf = gcl_variable_buf;
char *sv_linkvarbuf = gcl_variable_buf + MAX_LINKVARBUF;
char *var_buf = gcl_variable_buf + ( MAX_LINKVARBUF * 2 );
char *sv_var_buf = gcl_variable_buf + ( MAX_LINKVARBUF * 2 + MAX_VAR_BUF );

#endif

SBP_LinkVars bp_linkvars = { 0 };
SBP_LinkVars sv_bp_linkvars = { 0 };
static char local_var_buf[ MAX_LOCAL_VAR_BUF ];

// Save the currently loaded transfarring id when calling GCL_SaveVar() and restore it when calling GCL_RestoreVar().
// Not doing this ends up disabling trophies when a game is continued after dying.
static unsigned char sLoadedTransfarringID_savedWithLinkvars[16] = { 0 };

static void bp_trophy_set_gcl_var( const char * const top, int value )
{
   unsigned int code = GCL_GetVarCode( (char*)top );

   //code contains type, offset, and bit offset.
   //check against variable.sym to get the code for the variable you want.
   //The code here is based on the low 16 bits of the first column
   //and the entire third column of entries in that file.
   //[ 4 bits] GCL_CONST | GCL_VAR | GCL_ARRAY
   //[ 4 bits] data type, e.g. GCL_BOOL
   //[ 4 bits] GCL_VAR_TYPE_LINK | GCL_VAR_TYPE_LOCAL | 0
   //[ 4 bits] bit offset
   //[16 bits] offset within variable buffer

   const char * area = GM_GetArea();
   switch( code )
   {
   case 0x11000ADC:  //写真属性 (verified same code all three SKU)
      {
         //photo attribute
         if( value == 170 )   //デジカメ心霊写真
         {
            //ghost image
            if( !strcmp( area, "w04b"	) )
            {
               //"Photograph ghost image in Hold No. 2"
               BP_TrophySystem_UnlockTrophy( kTRP_PhotographGhostImages );
            }
         }
      }
      break;
   case 0x12000056:  //デジカメパンツ兵評価回数
      {
         if( value == 1 )
         {
            //Showing Otacon the no-pants marine photo for the first time
            if( !strcmp( area, "w04c"	) )
            {
               BP_TrophySystem_UnlockTrophy( kTRP_SendNoPantsMarinePhoto );
            }
         }
      }
      break;
   case 0x140501DD:  //テイルズＡクリアしたフラグ
   case 0x140601DD:  //テイルズＢクリアしたフラグ
   case 0x140701DD:  //テイルズＣクリアしたフラグ
   case 0x14000262:  //テイルズＤクリアしたフラグ
   case 0x14010262:  //テイルズＥクリアしたフラグ
      {
         if( value )
         {
            //Snake tales complete
            bp_trophy_snake_tales_clear( code );
         }
      }
      break;
   }
}

/* ----------------------------- GCL 変数アクセス --------------------------- */

/* 変数領域の初期化 */

void GCL_InitVar( void )
{
	memset( var_buf, 0, MAX_VAR_BUF );
	memset( linkvarbuf, 0, MAX_LINKVARBUF );
}

//BP_TROPHY - track game type
extern int gBP_PlayingMissions;
//BP_TROPHY - track game type

void GCL_InitClearVar( void )
{
	/* コンフィグレーションとゲームレベルは保存 */
	short gm_titlemenustatus=GM_TitleMenuStatus;

   //BP - check whether we're starting a new game from a "game clear" save file
   int game_clear = GM_GameClearCount;
   int tanker_clear = GM_TankerClearCount;
   int plant_clear = GM_PlantClearCount;

   //game_clear should be redundant, but can't hurt to be sure!
   int start_from_clear = (game_clear!=0) || (tanker_clear!=0) || (plant_clear!=0);

	memset( var_buf, 0, MAX_VAR_BUF );

   //BP_TROPHY - skip reset when playing VR missions since 
   //            this function gets called during stage loads.
   //            Also, skip reset when starting a new main game off a game clear save file.
   if( gBP_PlayingMissions == 0 && !start_from_clear )
   {
      memset( &bp_linkvars, 0, sizeof( bp_linkvars ) );
   }
   else
   {
      //Just clear "skipped a cinematic" flag which the player would *not* want to persist
      //across playthroughs so he gets another shot at the "watch all cinematics" trophy
      //the next time around.  This flag is irrelevant for VR missions save files.
      bp_linkvars.mSkippedACinematic = 0;
   }

	memset( ( char * )&GM_SaveArea, 0
			, ( char * )&GM_MyName[0] - ( char * )&GM_SaveArea );

	memset( ( char * )&GM_StageBreakPoint, 0
			, MAX_LINKVARBUF - ( ( char * )&GM_StageBreakPoint - ( char * )linkvarbuf ) );

	GM_TitleMenuStatus=gm_titlemenustatus;

	GCL_SaveVar();
}

/* --------- 変数の待機、復帰 ------ */

void BackupLoadedTransfarringID()
{
   //Backup the current TID
   Transfarring_GetLoadedTransfarringID( sLoadedTransfarringID_savedWithLinkvars );
}

void GCL_SaveVar( void )
{
	memcpy( sv_linkvarbuf, linkvarbuf, MAX_LINKVARBUF );
	memcpy( sv_var_buf, var_buf, MAX_VAR_BUF );

   //Backup the current TID
   BackupLoadedTransfarringID();

   sv_bp_linkvars = bp_linkvars;
}

void GCL_RestoreVar( void )
{
	memcpy( linkvarbuf, sv_linkvarbuf, MAX_LINKVARBUF ) ;
	memcpy( var_buf, sv_var_buf, MAX_VAR_BUF );	
   
   //Restore the TID
   Transfarring_SetLoadedTransfarringIDByString( sLoadedTransfarringID_savedWithLinkvars );
   
   bp_linkvars = sv_bp_linkvars;
}

/* ある変数のみをセーブする。(linkvar) */
void	GCL_SaveLinkVar( void *ptr, int size )
{
	int			diff ;

	diff = ( int )ptr - ( int )linkvarbuf ;
	memcpy( sv_linkvarbuf + diff, ptr, size ) ;
}

/* セーブ領域のポインタを取得 */
void 	*GCL_GetSavedLinkVar( void *ptr )
{
	int			diff ;

	diff = ( int )ptr - ( int )linkvarbuf ;
	return ( void * )( sv_linkvarbuf + diff ) ;
}

/* ローカル変数の消去 */
void GCL_ResetLocalVar( void )
{
	memset( local_var_buf, 0, MAX_LOCAL_VAR_BUF );
}

/* --------- 変数の取得、変更 --------- */

static void get_var( char *bufp, int code, int offset, int *value_p, char swap )
{
	switch( ( code >> 24 ) & 0x0f ){
#ifndef STRCODE16
	  case GCL_STRID:
#endif
	  case GCL_INT:
		{
			bufp += offset * sizeof( int );
			*value_p = *((int *) bufp );
         if (swap != 0)
         {
            BP_BE_Swap4Bytes(value_p, value_p);
         }
		}
		break;
	  case GCL_PROCID:
		{
			bufp += offset * sizeof( int );
			*value_p = GCL_GetStrCode( bufp );
         // Don't think we need to endian swap here because they were stored as bytes
		}
		break;
#ifdef STRCODE16
	  case GCL_STRID:
#endif
	  case GCL_SHORT:
		{
			unsigned short *sp = ( unsigned short * )bufp;
			sp += offset;
//printf( "get value = %X\n", sp );
         if (swap != 0)
         {
            unsigned short temp;
            BP_BE_Swap2Bytes(&temp, sp);
            *value_p = ( short ) temp;
         }
         else
         {
            *value_p = ( short )*sp;
         }
		}
		break;
	  case GCL_BYTE:
	  case GCL_CHAR:
		{
			bufp += offset;
			*value_p = ( unsigned char )*bufp;
		}
		break;
	  case GCL_BOOL:
		{
			unsigned char *sp;
			int m;
			int bit;

			bit = ( ( code >> 16 ) & 0x0f );
			offset = offset + bit;
			sp = bufp + ( offset / 8 );
			bit = offset % 8;

			m = ( 1 << bit );
			*value_p = ( *sp & m ) ? 1 : 0;
		}
		break;
	}
}


char *GCL_GetVar( char *top, int *type_p, int *value_p )
{
	char *next;
	unsigned int code, type;
	unsigned char *bufp;
	int offset;
   char swap;

	code = GCL_GetVarCode( top );

	type = ( ( code >> 24 ) & 0x0f );
	*type_p = type;
	
   swap = 0;
	if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LINK ){
		bufp = ( unsigned char * )linkvarbuf;
	} else if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LOCAL ){
		bufp = ( unsigned char * )local_var_buf;
	} else {
		bufp = ( unsigned char * )var_buf;
      swap = 1;
	}
	bufp += ( code & 0xffff );

	if( ( ( code >> 24 ) & 0xf0 ) == GCL_ARRAY ){
		int max, t;
		next = GCL_GetNextValue( top + 4, &t, &max );
		next = GCL_GetNextValue( next, &t, &offset );
		if( offset >= max ){
			printf( "get ARRAY OFFSET OVER!! code %X offset %d max %d\n", code, offset, max );
			HANGUP();
		}
		D( printf( "MAX = %d offset = %d\n", max, offset ) );
	} else {
		offset = 0;
		next = top + 4;
	}

	get_var( bufp, code, offset, value_p, swap );

	return next;
}

static void set_var( char *bufp, int code, int offset, int value, char swap )
{
//printf( "set value = %X\n", bufp );
	switch( ( code >> 24 ) & 0x0f ){
#ifndef STRCODE16
	  case GCL_STRID:
#endif
	  case GCL_INT:
		{
			int *sp = ( int * )bufp;
			sp = sp + offset;

         if (swap != 0)
         {
            BP_BE_Swap4Bytes(&value, &value);
         }
			*sp = value;
		}
		break;
	  case GCL_PROCID:
		{
			unsigned char *sp = ( unsigned char * )bufp;

			sp = sp + offset * sizeof( int );
			sp[ 2 ] = ( value >> 16 );
			sp[ 1 ] = ( value >> 8 );
			sp[ 0 ] = ( value >> 0 );

         // Don't think we need to endian swap here since they're stored as single bytes
		}
		break;
#ifdef STRCODE16
	  case GCL_STRID:
#endif
	  case GCL_SHORT:
		{
			unsigned short *sp = ( unsigned short * )bufp;

			sp = sp + offset;

         *sp = value;
         if ( swap )
         {
            BP_BE_Swap2Bytes_Inp( sp );
         }
		}
		break;
	  case GCL_BYTE:
	  case GCL_CHAR:
		{
			unsigned char *sp = ( unsigned char * )bufp;

			sp = sp + offset;
			*sp = value;
		}
		break;
	  case GCL_BOOL:
		{
			unsigned char *sp = ( unsigned char * )bufp;
			int m, bit;

			bit = ( ( code >> 16 ) & 0x0f );
			offset = offset + bit;

			sp = bufp + ( offset / 8 );
			bit = offset % 8;

			m = ( 1 << bit );
			if( value ){
				*sp |= m;
			} else {
				*sp &= ~m;
			}
		}
		break;
	}
}

char *GCL_SetVar( char *top, int value )
{
	unsigned int code;
	unsigned char *bufp;
	int offset;
	char *next;
   char swap;

	code = GCL_GetVarCode( top );

   //BP_GAMEPLAY - override variable?
   value = BP_GCL_SetVar_Override( code, value );

   swap = 0;
	if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LINK ){
		bufp = ( unsigned char * )linkvarbuf;
	} else if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LOCAL ){
		bufp = ( unsigned char * )local_var_buf;
	} else {
		bufp = ( unsigned char * )var_buf;
      swap = 1;
	}
	bufp += ( code & 0xffff );

	if( ( ( code >> 24 ) & 0xf0 ) == GCL_ARRAY ){
		int max, t;
		next = GCL_GetNextValue( top + 4, &t, &max );
		next = GCL_GetNextValue( next, &t, &offset );
		if( offset >= max ){
			printf( "set ARRAY OFFSET OVER!! code %X offset %d max %d\n", code, offset, max );
			HANGUP();
		}
		D( printf( "SET MAX = %d offset = %d\n", max, offset ) );
	} else {
		offset = 0;
		next = top + 4;
	}

	set_var( bufp, code, offset, value, swap );

   bp_trophy_set_gcl_var( top, value );

	return next;
}

/* ------------------------------------------------- */
/*
	GCL_VAR_REF interface
*/

char *GCL_GetVarRef( char *top, GCL_VAR_REF *ref )
{
	char *next;

	memcpy( ref->code, top, 4 );

	if( ( ref->code[ 0 ] & 0xf0 ) == GCL_ARRAY ){
		int max, t, offset;
		next = GCL_GetNextValue( top + 4, &t, &max );
		next = GCL_GetNextValue( next, &t, &offset );

		ref->max = max;
		ref->offset = offset;
	} else {
		next = top + 4;
		ref->max = 1;
		ref->offset = 0;
	}
	return next;
}

void GCL_SetVarRef( GCL_VAR_REF *ref, int offset, int value )
{
	unsigned int code;
	unsigned char *bufp;
   char swap;

	code = GCL_GetVarCode( ref->code );

   swap = 0;
	if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LINK ){
		bufp = ( unsigned char * )linkvarbuf;
	} else if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LOCAL ){
		bufp = ( unsigned char * )local_var_buf;
	} else {
		bufp = ( unsigned char * )var_buf;
      swap = 1;
	}
	bufp += ( code & 0xffff );

	if( ( ( code >> 24 ) & 0xf0 ) == GCL_ARRAY ){
		int max;
		max = ref->max;
		offset += ref->offset;
		if( offset >= max ){
			printf( "ref ARRAY OFFSET OVER!! code %X offset %d max %d\n", code, offset, max );
			HANGUP();
		}
		D( printf( "SET MAX = %d offset = %d\n", max, offset ) );
	} else {
		if( offset > 0 ){
			printf( "SetVarRef var\n" );
			HANGUP();
		}
	}
	set_var( bufp, code, offset, value, swap );
}

int GCL_ReadVarRef( GCL_VAR_REF *ref, int offset )
{
	unsigned int code;
	unsigned char *bufp;
	int value;
   char swap;

	code = GCL_GetVarCode( ref->code );

   swap = 0;
	if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LINK ){
		bufp = ( unsigned char * )linkvarbuf;
	} else if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LOCAL ){
		bufp = ( unsigned char * )local_var_buf;
	} else {
		bufp = ( unsigned char * )var_buf;
      swap = 1;
	}
	bufp += ( code & 0xffff );

	if( ( ( code >> 24 ) & 0xf0 ) == GCL_ARRAY ){
		int max;
		max = ref->max;
		offset += ref->offset;
		if( offset >= max ){
			printf( "ARRAY OFFSET OVER!!\n" );
			HANGUP();
		}
		D( printf( "MAX = %d offset = %d\n", max, offset ) );
	} else {
		if( offset > 0 ){
			printf( "ReadVarRef var\n" );
			HANGUP();
		}
	}

	get_var( bufp, code, offset, &value, swap );

	return value;
}

/* 変数の値をセーブ用バッファに反映させる */
char *GCL_VarSaveBuffer( char *top )
{
	unsigned int code;
	unsigned char *bufp;
	GCL_VAR_REF refbuf, *ref;
	char *next;
	int offset;
	int value;
   char swap;

	ref = &refbuf;
	offset = 0;

	next = GCL_GetVarRef( top, ref );
	value = GCL_ReadVarRef( ref, 0 );
	
	code = GCL_GetVarCode( ref->code );

   swap = 0;
	ASSERT( ( code & 0xf00000 ) != GCL_VAR_TYPE_LOCAL );
	if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LINK ){
		bufp = ( unsigned char * )sv_linkvarbuf;
	} else {
		bufp = ( unsigned char * )sv_var_buf;
      swap = 1;
	}
	bufp += ( code & 0xffff );

	if( ( ( code >> 24 ) & 0xf0 ) == GCL_ARRAY ){
		int max;
		max = ref->max;
		offset += ref->offset;
		if( offset >= max ){
			printf( "ARRAY OFFSET OVER!!\n" );
			HANGUP();
		}
		D( printf( "SET MAX = %d offset = %d\n", max, offset ) );
	}
	set_var( bufp, code, offset, value, swap );

	return next;
}

/* セーブ用バッファから値を読みだす */
int GCL_ReadSavedVar( char *top )
{
	GCL_VAR_REF refbuf, *ref;
	int offset;
	int value;
	unsigned int code;
	unsigned char *bufp;
   char swap;

	ref = &refbuf;
	offset = 0;
	
	GCL_GetVarRef( top, ref );
	code = GCL_GetVarCode( ref->code );
	ASSERT( ( code & 0xf00000 ) != GCL_VAR_TYPE_LOCAL );
   swap = 0;
	if( ( code & 0xf00000 ) == GCL_VAR_TYPE_LINK ){
		bufp = ( unsigned char * )sv_linkvarbuf;
	} else {
		bufp = ( unsigned char * )sv_var_buf;
      swap = 1;
	}
	bufp += ( code & 0xffff );

	if( ( ( code >> 24 ) & 0xf0 ) == GCL_ARRAY ){
		int max;
		max = ref->max;
		offset += ref->offset;
		if( offset >= max ){
			printf( "ARRAY OFFSET OVER!!\n" );
			HANGUP();
		}
		D( printf( "SET MAX = %d offset = %d\n", max, offset ) );
	}

	get_var( bufp, code, offset, &value, swap );

	return value;
}

/* ---------------------------------------------------------------------- */
/*
	セーブロード用領域取得
*/

void *GCL_GetLinkvarSaveAreaTop( void )
{
	return sv_linkvarbuf;
}

void *GCL_GetVarSaveAreaTop( void )
{
	return sv_var_buf;
}
