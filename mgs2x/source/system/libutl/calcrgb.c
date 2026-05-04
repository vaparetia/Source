//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    lights[2] 情報から RGB値を計算する

    1999/11/15/             H.TANAKA
	$Id: calcrgb.c,v 1.1.1.3 2002/11/19 11:42:54 Yoshizawa1 Exp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libutl.h>
#endif

#include	"gameheader.h"
#include	"libmt.h"

#define   MAX_C  255.0F
#define   MIN_C  0.0F

extern void Big_CalcPlaneRGB(UCVECTOR *rgb,FMATRIX *lights);

static u_char Check_u_char(float e)
{
    if(e > MAX_C)
    {
	return (u_char)MAX_C ;
    }
    if(e < MIN_C)
    {
	return (u_char)MIN_C ;
    }

    return (u_char)e ;
}

#if 1
/* 内積と値から色を求める */
void Big_CalcRGB(UCVECTOR *rgb, FMATRIX *lights,FVECTOR  *norm)
{
    float    e[3] ;
    float    f;
    FVECTOR  tmp_vec,e_norm ;
    int      i ;

    if(norm == NULL)
    {
	Big_CalcPlaneRGB(rgb,lights) ;
	return ;
    }
    
    sceVu0Normalize(&e_norm,norm) ;
    /* 後で長さも掛けるので、光のベクトルを正規化する必要なし*/
    for(i = 0; i < 3; i++)
    { 
	tmp_vec.vx = lights[0].m[0][i] ;
	tmp_vec.vy = lights[0].m[1][i] ;
	tmp_vec.vz = lights[0].m[2][i] ;
	e[i] = _sceVu0InnerProduct(&tmp_vec,&e_norm) ;
    }
    
    tmp_vec.vx = e[0] * lights[1].m[0][0] + e[1] * lights[1].m[1][0] + e[2] * lights[1].m[2][0] ;
    tmp_vec.vy = e[0] * lights[1].m[0][1] + e[1] * lights[1].m[1][1] + e[2] * lights[1].m[2][1] ;
    tmp_vec.vz = e[0] * lights[1].m[0][2] + e[1] * lights[1].m[1][2] + e[2] * lights[1].m[2][2] ;

    f = lights[1].m[3][0] + tmp_vec.vx ;
    rgb->r = Check_u_char(f) ;
    f = lights[1].m[3][1] + tmp_vec.vy ;
    rgb->g = Check_u_char(f) ;
    f = lights[1].m[3][2] + tmp_vec.vz ;
    rgb->b = Check_u_char(f) ;

}
#endif

void Big_CalcPlaneRGB(UCVECTOR *rgb,FMATRIX *lights)
{
    float    e ;
    float    leng[3] ;
    FVECTOR  tmp_vec ;
    int      i ;

    e = 0.5F ;
    for(i = 0; i < 3; i++)
    { 
	tmp_vec.vx = lights[0].m[0][i] ;
	tmp_vec.vy = lights[0].m[1][i] ;
	tmp_vec.vz = lights[0].m[2][i] ;
	leng[i] = GV_VecLen3F(&tmp_vec) ;
    }

    tmp_vec.vx = e *(lights[1].m[0][0] * leng[0] + lights[1].m[1][0] * leng[1] + lights[1].m[2][0] * leng[2]) ;
    tmp_vec.vy = e *(lights[1].m[0][1] * leng[0] + lights[1].m[1][1] * leng[1] + lights[1].m[2][1] * leng[2]) ;
    tmp_vec.vz = e *(lights[1].m[0][2] * leng[0] + lights[1].m[1][2] * leng[1] + lights[1].m[2][2] * leng[2]) ;

    e  = lights[1].m[3][0] + tmp_vec.vx ;
    rgb->r = Check_u_char(e) ;
    
    e = lights[1].m[3][1] + tmp_vec.vy ;
    rgb->g = Check_u_char(e) ;

    e = lights[1].m[3][2] + tmp_vec.vz ;
    rgb->b = Check_u_char(e) ;
}
