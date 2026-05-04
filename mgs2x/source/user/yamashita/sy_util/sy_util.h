/*******************************************************************************
 * sy_util - sy_util.h
 * 山下便利
 * 2002/03/12 S.Yamashita
 * $Id: sy_util.h,v 1.1.1.3 2002/11/19 11:51:54 Yoshizawa1 Exp $
 */

#ifndef __INC_SY_UTIL__
#define __INC_SY_UTIL__

/*******************************************************************************
 * include
 */

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

#define _SY_UTIL_DEBUG
#define _SY_PRINTF1_DEBUG
#define _SY_PRINTF2_DEBUG
#define _SY_PRINTF3_DEBUG

#define _SY_STRING_MAX (80)

/* SY_DIR */
enum
{
	SY_DIR_NORTH = 0,
	SY_DIR_SOUTH,
	SY_DIR_EAST,
	SY_DIR_WEST,
	SY_DIR_UP,
	SY_DIR_DOWN,
};

/*******************************************************************************
 * macros
 */

#ifdef PSX2

	/* 通常のデバッグプリント（作業中はＯＮ。作業を終えたらＯＦＦ） */
	#if defined(_SY_UTIL_DEBUG) && defined(_SY_PRINTF1_DEBUG) && defined(SY_PRINTF1_DEBUG)
		#ifdef __CHARA_NAME__
			#define SY_PRINTF(...)  sy_printnf(__CHARA_NAME__, __VA_ARGS__)
			#define SY_PRINTF1(...) sy_printnf(__CHARA_NAME__, __VA_ARGS__)
		#else
			#define SY_PRINTF(...)  sy_printf(__VA_ARGS__)
			#define SY_PRINTF1(...) sy_printf(__VA_ARGS__)
		#endif
	#else
		#define SY_PRINTF(...)
		#define SY_PRINTF1(...)
	#endif

	/* 重要なメッセージ（基本的にはＯＮ） */
	#if defined(_SY_UTIL_DEBUG) && defined(_SY_PRINTF2_DEBUG) && defined(SY_PRINTF2_DEBUG)
		#ifdef __CHARA_NAME__
			#define SY_PRINTF2(...) sy_printnf(__CHARA_NAME__, __VA_ARGS__)
		#else
			#define SY_PRINTF2(...) sy_printf(__VA_ARGS__)
		#endif
	#else
		#define SY_PRINTF2(...)
	#endif

	/* 関数の入り口などで使用する（基本的にはＯＦＦ） */
	#if defined(_SY_UTIL_DEBUG) && defined(_SY_PRINTF3_DEBUG) && defined(SY_PRINTF3_DEBUG)
		#define SY_PRINTF3(...) sy_printnf(__FILE__, __VA_ARGS__)
	#else
		#define SY_PRINTF3(...)
	#endif

#else

	#include <stdarg.h>

	/* 通常のデバッグプリント（作業中はＯＮ。作業を終えたらＯＦＦ） */
	#if defined(_SY_UTIL_DEBUG) && defined(_SY_PRINTF1_DEBUG) && defined(SY_PRINTF1_DEBUG)
		#ifdef __CHARA_NAME__
			static void inline SY_PRINTF(char *f, ...)
			{
				va_list argptr;
				va_start(argptr, f);
				//vsprintf(buff, f, argptr);
				va_end(argptr);
			}
			static void inline SY_PRINTF1(char *f, ...)
			{
				va_list argptr;
				va_start(argptr, f);
				//vsprintf(buff, f, argptr);
				va_end(argptr);
			}
		#else
			static void inline SY_PRINTF(char *f, ...)
			{
				va_list argptr;
				va_start(argptr, f);
				//vsprintf(buff, f, argptr);
				va_end(argptr);
			}
			static void inline SY_PRINTF1(char *f, ...)
			{
				va_list argptr;
				va_start(argptr, f);
				//vsprintf(buff, f, argptr);
				va_end(argptr);
			}
		#endif
	#else
		static void inline SY_PRINTF(char *f, ...)
		{
			va_list argptr;
			va_start(argptr, f);
			//vsprintf(buff, f, argptr);
			va_end(argptr);
		}
		static void inline SY_PRINTF1(char *f, ...)
		{
			va_list argptr;
			va_start(argptr, f);
			//vsprintf(buff, f, argptr);
			va_end(argptr);
		}
	#endif

	/* 重要なメッセージ（基本的にはＯＮ） */
	#if defined(_SY_UTIL_DEBUG) && defined(_SY_PRINTF2_DEBUG) && defined(SY_PRINTF2_DEBUG)
		#ifdef __CHARA_NAME__
			static void inline SY_PRINTF2(char *f, ...)
			{
				va_list argptr;
				va_start(argptr, f);
				//vsprintf(buff, f, argptr);
				va_end(argptr);
			}
		#else
			static void inline SY_PRINTF2(char *f, ...)
			{
				va_list argptr;
				va_start(argptr, f);
				//vsprintf(buff, f, argptr);
				va_end(argptr);
			}
		#endif
	#else
		static void inline SY_PRINTF2(char *f, ...)
		{
			va_list argptr;
			va_start(argptr, f);
			//vsprintf(buff, f, argptr);
			va_end(argptr);
		}
	#endif

	/* 関数の入り口などで使用する（基本的にはＯＦＦ） */
	#if defined(_SY_UTIL_DEBUG) && defined(_SY_PRINTF3_DEBUG) && defined(SY_PRINTF3_DEBUG)
		static void inline SY_PRINTF3(char *f, ...)
		{
			va_list argptr;
			va_start(argptr, f);
			//vsprintf(buff, f, argptr);
			va_end(argptr);
		}
	#else
		static void inline SY_PRINTF3(char *f, ...)
		{
			va_list argptr;
			va_start(argptr, f);
			//vsprintf(buff, f, argptr);
			va_end(argptr);
		}
	#endif

#endif	/* PSX2 */

/*******************************************************************************
 * functions
 */

void sy_printf(char *format, ...);
void sy_printnf(char *name, char *format, ...);

void sy_memcpy(void *dst, void *src, int size, int num);
void sy_memcpy2(void *dst, void *src, int size, int num);

void  sy_setpos(FVECTOR *pos, FVECTOR *center, float width, float height, int dir);
void  sy_setuv(DG_PRIM2_UVRGB *uvrgb, DG_TEX *tex, int div_w, int div_h, int index);
void  sy_setuv2(DG_PRIM2_UVRGB *uvrgb, DG_TEX *tex, int div_w, int div_h, int index);
void  sy_setrgba(DG_PRIM2_UVRGB *uvrgb, u_short r, u_short g, u_short b, u_short a, int num);
float sy_grid(float a, float grid);

/*******************************************************************************
 */

#endif	/* __INC_SY_UTIL__ */
