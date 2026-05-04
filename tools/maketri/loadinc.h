/*
	$Id: loadinc.h,v 1.2 2000/09/18 00:33:26 usr02774 Exp $
 */
/*                                        */
/* Linux-gcc用構造体定義読み込みヘッダー [[Read the header structure definition for]]  */
/*                                        */

#define long64 long long
#define long32 int
#define u_long64 unsigned long long
#define u_long32 unsigned int

/* added by SteveM */
#ifndef u_int
#define u_int unsigned int
#endif

/* 各種ヘッダーの読み込み [[Reading various headers]] */

#include "ps2.h"
#include "mgs2.h"

#include "fmt_cm2.h"
#include "loadcm2.h"


