//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
        yn_utl.c

        出力系、ベクトル計算系などの汎用プログラム
        
        2001/07/03 Y.YANO
        $Id: yn_utl.c,v 1.1.1.3 2002/11/19 11:52:00 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <stdarg.h>
#endif

#include <libutl.h>

#include "gameheader.h"

/*---------------------------------------------------------------*/
/*                    ダンプ各種                                 */

void YN_MatrixDump( FMATRIX * matrix ){
    printf("%8.8f %8.8f %8.8f %8.8f \n" , matrix->m[0][0] , matrix->m[0][1]
           , matrix->m[0][2] , matrix->m[0][3] );
    printf("%8.8f %8.8f %8.8f %8.8f \n" , matrix->m[1][0] , matrix->m[1][1]
           , matrix->m[1][2] , matrix->m[1][3] );
    printf("%8.8f %8.8f %8.8f %8.8f \n" , matrix->m[2][0] , matrix->m[2][1]
           , matrix->m[2][2] , matrix->m[2][3] );
    printf("%8.8f %8.8f %8.8f %8.8f \n" , matrix->m[3][0] , matrix->m[3][1]
           , matrix->m[3][2] , matrix->m[3][3] );
}

void YN_VectorDump( FVECTOR * vector ){
    printf("%8.8f %8.8f %8.8f \n" , vector->vx , vector->vy , vector->vz );
}




/*---------------------------------------------------------------*/
/*                    ベクトル計算系                             */

/*----------------------------------------------------------------*/
                      /* YN_VectorLenXZ */
//XZだけの距離を見る
float YN_VectorLenXZ( FVECTOR *in )
{
	float  	ftemp ;

	ftemp = in->vx * in->vx + in->vz * in->vz ;
	ftemp = DG_SQRT( ftemp );

	return ftemp ;
}

/*****************************************************************/
                     /*YN_VectorLength*/
/*
  ベクトル(FVECTOR)の長さを返す
*/
float YN_VectorLength( FVECTOR *in )
{
        float temp ;
        temp = in->vx * in->vx + in->vy * in->vy + in->vz * in->vz ;
        temp = DG_SQRT( temp );

        return temp ;
}
/*****************************************************************/
                     /*YN_RotVector*/
/*
  X-Z平面のベクトルの回転行列を求める
  ベクトル in を angle だけ Y 軸を中心に回転させたものを out に返す
  angle は ラジアンで指定
*/
void YN_RotVector( FVECTOR *out , FVECTOR *in  , float angle )
{
        FMATRIX nyaan , unit_matrix = DG_UnitMatrix ;
        
        _sceVu0RotMatrixY( &nyaan , &unit_matrix , angle );
        _sceVu0ApplyMatrix( out , &nyaan , in ); 

}
/*****************************************************************/
                     /*YN_GetHousen*/
/*
  三つの HZX_VEC から張られる平面の法線を求める
  三点から出てきた二つのベクトルは(x,z,y)の順番
*/
void YN_GetHousen( FVECTOR *housen , HZX_VEC p1 , HZX_VEC p2 , HZX_VEC p3 )
{
        FVECTOR v1 , v2 ;
        float ftemp ;
        _sceVu0SubVector( &v1 , (FVECTOR*)(&p2) , (FVECTOR*)(&p1) );
        _sceVu0SubVector( &v2 , (FVECTOR*)(&p3) , (FVECTOR*)(&p1) );
        ftemp = v1.vy ;
        v1.vy = v1.vz ;
        v1.vz = ftemp ;
        ftemp = v2.vy ;
        v2.vy = v2.vz ;
        v2.vz = ftemp ;
        _sceVu0OuterProduct( housen , &v1 , &v2 ) ;
}
        
/*****************************************************************/
                      /*YN_OuterOuter*/
/*
  外積を二回行なって、in と in2 の二つのベクトルに貼られる平面上の
  in に垂直で 左にある単位ベクトル out を返す。
*/
void YN_OuterOuter( FVECTOR *out , FVECTOR *in , FVECTOR *in2 )
{
        _sceVu0OuterProduct( out , in2 , in ) ;
        _sceVu0OuterProduct( out , in , out ) ;
        _sceVu0Normalize ( out , out ) ;
}

