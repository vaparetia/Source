/*
   gameheader.h
   ゲーム用ヘッダをまとめたもの

   1999/07/07 M.Sonoyama
   $Id: gameheader.h,v 1.1.1.3 2002/11/19 11:41:50 Yoshizawa1 Exp $			

   *** PROJECT OF METAL GEAR SOLID2 ***
*/

#ifndef _gameheader_h_
#define _gameheader_h_

#include	<stdio.h>

#ifdef GOLD_VERSION
#define printf( ... )
#endif

#ifdef PSX2
#include	<math.h>

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libhzx.h"
#include	"libgcl.h"
#include	"libmt.h"
#include	"game.h"
#include	"g_define.h"
#include	"g_struct.h"
#include	"g_extern.h"
#include	"g_macro.h"
#include	"util.h"
#include	"game.x"
#include	"menuprim.h"
#include	"g_sound.h"
#include	"se_defin.h"
#include	"player.h"
#include	"commdef.h"
#endif

#ifdef KP_XBOX
#include	<xtl.h>
#include	"libgv.h"
#include	"libgv.cnf"
#include	"libdg.h"
#include	"dgconf.h"
#include	"libhzx.h"
#include	"libgcl.h"
#include	"libmt.h"
#include	"game.h"
#include	"g_define.h"
#include	"g_struct.h"
#include	"g_extern.h"
#include	"g_macro.h"
#include	"util.h"
#include	"game.x"
///#include	"menuprim.h"
#include	"g_sound.h"
#include	"se_defin.h"
#include	"player.h"
#include	"commdef.h"
#endif

#include "bp_stdlib_ps2.h"


// XBOX TCR 対策選択文字色
// (一応PS2でもdefineされるようにしておく)
// add 2002/08/27 M.Kobayashi
#define GM_FOCUS_COLOR_R 	112
#define GM_FOCUS_COLOR_G 	64
#define GM_FOCUS_COLOR_B 	16
#define GM_FOCUS_COLOR		(GM_FOCUS_COLOR_R | (GM_FOCUS_COLOR_G << 8) | (GM_FOCUS_COLOR_B << 16))

/*-----------------------------------------------*/

extern	void	TEST_Color( int, int, int, int ) ;
extern	void	TEST_Locate( int, int, int ) ;
extern	void	TEST_Printf( char *, ... ) ;

//extern	void	_MENU_Init( void ) ;
//extern	void	_MENU_Locate( long64, long64, long64 ) ;
//extern	long64	_MENU_Printf( char *, int, int, int, int ) ;

#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265358979323846264338327950288419716939937510f

// BP FIX for KJPFB-145 and MGSTWO-845
// dog tag names should match between in-game and menu
#define DOGTAG_DOUBLE 1

#endif
