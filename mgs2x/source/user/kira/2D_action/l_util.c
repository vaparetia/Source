//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  l_util.c
  汎用関数群

  2001/01/11   Y.Kira
  $Id: l_util.c,v 1.1.1.3 2002/11/19 11:43:50 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

#include "font.h"
#include "libfs.h"

#define _l_util_c_
#include "l_util.h"

#define E_BIAS (8)


/*
 * 16bit 浮動小数点型を float 型に変換する
 */
float lutil_short_to_float(unsigned short mf)
{
  unsigned int    s_dat, e_dat, f_dat ;
  float           f ;
  
  /* 符合取りだし */
  s_dat = ( mf & 0x8000 ) << 16 ;
  /* 指数取りだし */
  e_dat = ( mf & 0x7c00 ) ;
  if ( e_dat != 0 ){
    e_dat += ( 127 - E_BIAS ) << 10 ;
    e_dat <<= 13 ;
  }
  /* 仮数部取りだし */
  f_dat = ( mf & 0x03ff ) << 13 ;
  /* ビット合成 */
  *(unsigned int*)&f = s_dat | e_dat | f_dat ;
  
  return ( f );
}

