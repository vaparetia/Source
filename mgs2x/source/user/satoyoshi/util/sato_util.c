//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*										*/
/*	里吉汎用計算プログラム		<sato_util.h>				*/
/*					Hiroki Satoyoshi 2001/2/3		*/
/*	$Id: sato_util.c,v 1.1.1.3 2002/11/19 11:48:27 Yoshizawa1 Exp $		*/
/*										*/
/********************************************************************************/
/********************************************************************************/
/*	program      								*/
/********************************************************************************/
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <float.h>
#include "gameheader.h"
#include	"camera.h"


#define GET_IN	0
#define GET_OUT	1

/* yano add 2002.03.12 */
#define inline

/********************************************************************************/
/*	program      								*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void get_setuzoku_num						*/
/*	引数:	ACRO_POINT*	acro_pos					*/
/*		int		flag	0:開始					*/
/*					1:終了					*/
/*	返値:	接続番号							*/
/*	説明:	接続場所を得る							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int setuzokuget(FVECTOR* getpos, FVECTOR* getvec, int flag){
    FVECTOR	tmppos;
    FVECTOR	tmpvec;
    
    tmppos = *getpos;
    tmpvec = *getvec;

    
    _sceVu0Normalize(&tmpvec, &tmpvec);
    if (flag == GET_IN){
	_sceVu0ScaleVector(&tmpvec, &tmpvec, -30000.0f);
    }
    else if (flag == GET_OUT){
	_sceVu0ScaleVector(&tmpvec, &tmpvec, 30000.0f);
    }
    _sceVu0AddVector(&tmppos, &tmppos, &tmpvec);
    // グループ

    if(tmppos.vx <= -15000.0f){
	if (flag == GET_IN){
	    return 1;
	}
	else if (flag == GET_OUT){
	    return 2;
	}
    }
    else if(tmppos.vx >  15000.0f){
	if (flag == GET_IN){
	    return 3;
	}
	else if (flag == GET_OUT){
	    return 0;
	}
    }
    else if(tmppos.vz <=  -150000.0f){
	if (flag == GET_IN){
	    return 0;
	}
	else if (flag == GET_OUT){
	    return 1;
	}
    }
    else if(tmppos.vz > - 150000.0f){
	if (flag == GET_IN){
	    return 2;
	}
	else if (flag == GET_OUT){
	    return 3;
	}
    }
    return -1;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	View_HermiteRout( *res, *q0, *q1, *d0, *d1, t, scale )		*/
/*	引数:	FVECTOR	*q0		補間元座標				*/
/*		FVECTOR	*q1		補間先座標				*/
/*		FVECTOR	*d0		補間元変化量				*/
/*		FVECTOR	*d1		補間先変化量				*/
/*		float	scale		変化量のスケーリング値			*/
/*		int	devide		解像度					*/
/*	説明:	ルート表示							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void View_HermiteRout( FVECTOR* q0, FVECTOR* q1, FVECTOR* d0, FVECTOR* d1, float scale, int devide){
#ifdef BP_PS2
    FVECTOR	verts[devide*2];
#else
   FVECTOR	*verts;
#endif

    short	loop;

#ifndef BP_PS2	
	verts= malloc(sizeof(FVECTOR) * devide * 2);
#endif    

    MT_HermiteLerpVecScale( &verts[0], q0, q1, d0, d1, 0.0f, scale);
    MT_HermiteLerpVecScale( &verts[1], q0, q1, d0, d1, 1.0f/devide, scale);

    //printf ("   %f\n", scale);

    NewLineView(&verts[0], 1, 250,250,250);

    for (loop=1; loop<devide; loop++){
	verts[loop*2].vx = verts[loop*2-1].vx;
	verts[loop*2].vy = verts[loop*2-1].vy;
	verts[loop*2].vz = verts[loop*2-1].vz;

	MT_HermiteLerpVecScale( &verts[loop*2+1], q0, q1, d0, d1,
				(loop+1)*1.0f/devide, scale);
	NewLineView(&verts[loop*2], 1, 250,250,250);
    }
#ifndef BP_PS2	
	free(verts);
#endif    


}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void viewp							*/
/*	引数:	FVECTOR*							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void sat_viewp(FVECTOR* point, char r, char g, char b){
    FVECTOR verts[2];
    
    verts[0] = verts[1] = *point;
    verts[0].vy -= 300.0f;
    verts[1].vy += 300.0f;

    NewLineView(verts, 1, r, g, b);
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void viewp							*/
/*	引数:	FVECTOR*							*/
/*	説明:									*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void sat_viewp2(FVECTOR* point, float length, char r, char g, char b){
    FVECTOR verts[2];
    
    verts[0] = verts[1] = *point;
    verts[0].vy -= length;
    verts[1].vy += length;

    NewLineView(verts, 1, r, g, b);
}


extern int HAR_RoutView;
/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void acrobat							*/
/*	引数:	Work	*work							*/
/*	説明:	アクロバット飛行						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void get_hermove(FVECTOR* answer, FVECTOR* FROM_POS, FVECTOR* TO_POS,
			FVECTOR* FROM_SP, FVECTOR* TO_SP, float time, float max_time)
{
    FVECTOR FROM_VEC, TO_VEC;

    _sceVu0ScaleVector(&FROM_VEC, FROM_SP, 1.0f/TIME_BASE);
    _sceVu0ScaleVector(&TO_VEC, TO_SP, 1.0f/TIME_BASE);

    MT_HermiteLerpVecScale( answer, 
			    FROM_POS,
			    TO_POS,
			    &FROM_VEC,
			    &TO_VEC,
			    1.0f*time/max_time,
			    (float)max_time);
#ifdef DEBUG_MODE
    if (HAR_RoutView==1){
	View_HermiteRout( FROM_POS,
			  TO_POS,
			  &FROM_VEC,
			  &TO_VEC,
			  max_time,
			  30);
    }
#endif

}


/*::::::::::::::::::::::::::::::<inline function>:::::::::::::::::::::::::::::::*/
/*	名前:	void SetRGBA							*/
/*	引数:	int r,g,b	RGBの輝度				       	*/
/*	    :	int a   	αチャンネル輝度			       	*/
/*	説明:	RGBAをセットする			       			*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
inline void SetRGBA(int r ,int g ,int b ,int a,int *rgba){
    *rgba = ((a<<24)|(b<<16)|(g<<8)|(r)) ;
} 


/*::::::::::::::::::::::::::::::<inline function>:::::::::::::::::::::::::::::::*/
/*	名前:	void Fvec_from_Matrix						*/
/*	返値:	なし								*/
/*	引数:	FVECTOR	*vec							*/
/*	    :	MATRIX	*mat							*/
/*	説明:	マトリクスからベクトルを得る					*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
inline void Fvec_from_Matrix(FVECTOR *vec, FMATRIX *mat){
	vec->vx = mat->m[ 3 ][ 0 ];
	vec->vy = mat->m[ 3 ][ 1 ];
	vec->vz = mat->m[ 3 ][ 2 ];
	vec->vw = 1.0f;
}

/*::::::::::::::::::::::::::::::<inline function>:::::::::::::::::::::::::::::::*/
/*	名前:	void Dir_from_2Vec						*/
/*	返値:	なし								*/
/*	引数:	FVECTOR	*pos1							*/
/*	    :	FVECTOR	*pos2							*/
/*	    :	SVECTOR	*dir							*/
/*	説明:	２つの位置ベクトルから方向ベクトルを算出する			*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
inline void Dir_from_2Vec(FVECTOR *pos1,FVECTOR *pos2,SVECTOR *dir){
	FVECTOR	vec ;
	float	dis_y,dis_x ;

	/*Ｙ角度*/	
	_sceVu0SubVector(  &vec, pos2, pos1 ) ;
	dir->vy = GV_VecDir2( &vec ) ;

	/*Ｘ角度*/
	dis_y = -vec.vy ;
	vec.vy = 0.0F;
	dis_x = GV_VecLen3F( &vec ) ;
	vec.vx = dis_y ;
	vec.vz = dis_x ;
	dir->vx = GV_VecDir2( &vec ) ;
	dir->vz = 0 ;
}

/*::::::::::::::::::::::::::::::<inline function>:::::::::::::::::::::::::::::::*/
/*	名前:	void Dir_from_2Vec						*/
/*	返値:	なし								*/
/*	引数:	FVECTOR	*pos1							*/
/*	    :	FVECTOR	*pos2							*/
/*	    :	SVECTOR	*dir							*/
/*	説明:	２つの位置ベクトルから方向ベクトルを算出する			*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
inline void Dir_from_2VecF(FVECTOR *pos1,FVECTOR *pos2, FVECTOR *dir){
	FVECTOR	vec ;
	float	dis_y,dis_x ;

	/*Ｙ角度*/	
	_sceVu0SubVector(  &vec, pos2, pos1 ) ;
	dir->vy = atan2f( vec.vx, vec.vz );

	/*Ｘ角度*/
	dis_y = -vec.vy ;
	vec.vy = 0.0F;
	dis_x = GV_VecLen3F( &vec ) ;
	vec.vx = dis_y ;
	vec.vz = dis_x ;

	dir->vx = atan2f( vec.vx, vec.vz );
	dir->vz = 0.0f ;
}


/*::::::::::::::::::::::::::::::<inline function>:::::::::::::::::::::::::::::::*/
/*	名前:	void SAT_Minus_FVECTOR						*/
/*	引数:	FVECTOR	*answer	  					       	*/
/*		FVECTOR	*pos1	  					       	*/
/*		FVECTOR	*pos2	  					       	*/
/*	説明:	pos1からpos2を減算する						*/
/*		つまりpos1 > pos2 という方向ベクトルを計算する			*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
inline void SAT_Minus_FVECTOR(FVECTOR *answer, FVECTOR *pos1, FVECTOR *pos2){
    answer->vx = pos1->vx - pos2->vx;
    answer->vy = pos1->vy - pos2->vy;
    answer->vz = pos1->vz - pos2->vz;
    answer->vw = 0.0f ;
}

/*::::::::::::::::::::::::::::::<inline function>:::::::::::::::::::::::::::::::*/
/*	名前:	float AbsLimit							*/
/*	引数:	float	base							*/
/*		float	limit							*/	
/*	説明:	絶対値の限界の設定						*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
inline float AbsfLimit(float base,float limit){

    //	    printf ("%f\n", limit);

    if(fabs(base) > limit ) {
	
	if(base >= 0.0f ){

	    return fabs(limit) ;
	}
	else {
	    return (-1.0f * fabs(limit)) ;
	}
    }else {
	return base ;
    }
}

