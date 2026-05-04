//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ok_util.c
	おかじの便利集
	1999/09/01 S.Okajima
	$Id: ok_util.c,v 1.1.1.3 2002/11/19 11:47:41 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"

#ifndef PSX2
#include	"dmapack.h"
#endif


extern float	OK_slow_param;	//1.0f で デフォルト速度

//-------------------------------
float OK_RectLen( FVECTOR *pos0, FVECTOR *pos1 )
{
	float	ftemp;
	FVECTOR	fvtemp;

	_sceVu0SubVector( &fvtemp, pos0, pos1 );
	ftemp     = DG_FABS( fvtemp.vx );
	fvtemp.vy = DG_FABS( fvtemp.vy );
	fvtemp.vz = DG_FABS( fvtemp.vz );

	if( fvtemp.vy > ftemp ) ftemp = fvtemp.vy;
	if( fvtemp.vz > ftemp ) ftemp = fvtemp.vz;

	return ftemp;
}

//-------------------------------
void OK_Print3D_perse( FVECTOR *pos, char *strings, CVECTOR col )
{
	FVECTOR		ret ;   
	DG_CHANL	*cp ;
	int			flag;
	int			locate_x;
	int			locate_y;

	if( col.cd <= 0 ) return;

	if( GM_CheckGameStatus(STATE_PLAY_DEMO) ) return;

	flag = 0;

	/* 位置決定 */
	cp = DG_Chanl(0) ;

	DG_COPY_VEC( &ret, pos );
	ret.vw = 1.0f;

	_sceVu0ApplyMatrix( &ret, &cp->eye_pers, &ret ) ;

	/* カメラの範囲内に入っているかどうか */
	if(ret.vz > ret.vw) flag = 1 ;
	if(ret.vw < 0) ret.vw = - ret.vw ;

	if((ret.vx > ret.vw*2.0f) || (ret.vx < (-ret.vw)*2.0f)) flag = 1 ;
	if((ret.vy > ret.vw) || (ret.vy < (-ret.vw))) flag = 1 ;

	if( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ){
		if( ret.vy > ret.vw*0.5f ){
			ret.vy = ret.vw*0.5f;
		}else if( ret.vy < -ret.vw*0.5f ){
			ret.vy =-ret.vw*0.5f;
		}
		if( ret.vx > ret.vw*0.5f ){
			ret.vx = ret.vw*0.5f;
		}else if( ret.vx < -ret.vw*0.5f ){
			ret.vx =-ret.vw*0.5f;
		}
	}

	if( !flag ){
		locate_x = (int)((ret.vx / ret.vw) * DRAW_WIDTH /2) + DRAW_WIDTH /2 ;
		locate_y = (int)((ret.vy / ret.vw) * DRAW_HEIGHT/2) + DRAW_HEIGHT/2 - 32;
	}else{
		return;
	}

	MENU_SetAlphaMode( 0, 1, 0, 1, 0 );
	MENU_Locate( locate_x, locate_y, MENU_MODE_CENTER );
	MENU_Color( col.r, col.g, col.b, col.cd );
	MENU_Printf( "%s\n",strings ) ;

	return;
}

//-------------------------------
DG_PRIM2 *OK_FreePrim2( DG_PRIM2 *prim )
{
	if ( prim != NULL ) {
		DG_DequeuePrim2( prim ) ;
		DG_FreePrim2( prim ) ;
		prim = NULL;
	}
	return prim;
}



/* Ｚ軸方向が０方向、Ｚからの方向を求める */
void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot )
{
	SVECTOR	temp_rot ;
	FVECTOR	vec ;
	float tmp ;

	_sceVu0SubVector( &vec, to, from ) ;
//	_sceVu0Normalize( &vec, &vec );

	tmp = atan2f( vec.vx, vec.vz ) ;
	rot->vy = 4095 & ( short )( ( tmp * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */
	temp_rot.vx = 0.0f;
	temp_rot.vy = -rot->vy;
	temp_rot.vz = 0.0f;

	DG_SetPos2( &DG_ZeroVector, &temp_rot );
	DG_PutVector( &vec, &vec, 1 );

	tmp = -atan2f( vec.vy, vec.vz ) ;
	rot->vx = 4095 & ( short )( ( tmp * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */

	rot->vz = 0 ;
}

/* Ｙ軸方向が０方向、Ｙからの方向を求める */
void OK_DirVecXY_Y( FVECTOR *from, FVECTOR *to, SVECTOR *rot )
{
	OK_DirVecXY( from, to, rot );
	rot->vx -= 1024 ;
}

void OK_IntSmoother( int *itemp1, int *itemp2, float ratio )
{
	float		r1;

	r1=1.0f-ratio;

	*itemp1 = (int)( (float)(*itemp1) * ratio + (float)(*itemp2)*r1 );


}

void OK_FloatSmoother( float *ftemp1, float *ftemp2, float ratio )
{
	float		r1;

	r1 = 1.0f-ratio;
	(*ftemp1) += ( (*ftemp2) - (*ftemp1) ) * r1;
}


void OK_DirectionSmoother( SVECTOR *svect1, SVECTOR *svect2, float ratio )
{
	SVECTOR	svtemp;
	float	minus_ratio;

	minus_ratio=1.0f-ratio;

	svect1->vx &= 4095;
	svect1->vy &= 4095;
	svect1->vz &= 4095;
	svect2->vx &= 4095;
	svect2->vy &= 4095;
	svect2->vz &= 4095;

	svtemp.vx = svect2->vx - svect1->vx;
	svtemp.vy = svect2->vy - svect1->vy;
	svtemp.vz = svect2->vz - svect1->vz;

	if(svtemp.vx >= 2048) svect1->vx+=4096;
	if(svtemp.vy >= 2048) svect1->vy+=4096;
	if(svtemp.vz >= 2048) svect1->vz+=4096;
	if(svtemp.vx <=-2048) svect2->vx+=4096;
	if(svtemp.vy <=-2048) svect2->vy+=4096;
	if(svtemp.vz <=-2048) svect2->vz+=4096;

//TEST_Printf( "1  :%d %d %d ",svect1->vx,svect1->vy,svect1->vz );
//TEST_Printf( "2  :%d %d %d ",svect2->vx,svect2->vy,svect2->vz );
	svect1->vx=(short)( (float)svect1->vx * ratio  +  (float)svect2->vx * minus_ratio );
	svect1->vy=(short)( (float)svect1->vy * ratio  +  (float)svect2->vy * minus_ratio );
	svect1->vz=(short)( (float)svect1->vz * ratio  +  (float)svect2->vz * minus_ratio );

//TEST_Printf( "a  :%d %d %d ",svect1->vx,svect1->vy,svect1->vz );
}



/* 検索直線とバンダリの交差判定 */
/* from が バンダリの外から */
int	OK_CheckCrossBoundaryOuter( FVECTOR *from, FVECTOR *to, FVECTOR *bound_min, FVECTOR *bound_max, FVECTOR *hit )
{
	FVECTOR	fvtemp1;
	FVECTOR	fvtemp2;
	FVECTOR	fvtemp3;

	if( !vu0_CheckBoundingBox( to, bound_min, bound_max ) ) return 0;

	_sceVu0SubVector( &fvtemp1, to, from ) ;
	_sceVu0SubVector( &fvtemp2, bound_max, from ) ;
	_sceVu0SubVector( &fvtemp3, bound_min, from ) ;

	if( fvtemp1.vx == 0 ){
		goto cutx_failed;
	}else if( fvtemp1.vx < 0 ){
		hit->vx = bound_max->vx;
		hit->vy = from->vy + fvtemp2.vx * fvtemp1.vy / fvtemp1.vx ;
		hit->vz = from->vz + fvtemp2.vx * fvtemp1.vz / fvtemp1.vx ;
	}else{
		hit->vx = bound_min->vx;
		hit->vy = from->vy + fvtemp3.vx * fvtemp1.vy / fvtemp1.vx ;
		hit->vz = from->vz + fvtemp3.vx * fvtemp1.vz / fvtemp1.vx ;
	}
	if( hit->vy < bound_min->vy || hit->vy > bound_max->vy ) goto cutx_failed;
	if( hit->vz < bound_min->vz || hit->vz > bound_max->vz ) goto cutx_failed;
	return 1 ;

cutx_failed :
	if( fvtemp1.vy == 0 ){
		goto cuty_failed;
	}else if( fvtemp1.vy < 0 ){
		hit->vy = bound_max->vy;
		hit->vz = from->vz + fvtemp2.vy * fvtemp1.vz / fvtemp1.vy ;
		hit->vx = from->vx + fvtemp2.vy * fvtemp1.vx / fvtemp1.vy ;
	}else{
		hit->vy = bound_min->vy;
		hit->vz = from->vz + fvtemp3.vy * fvtemp1.vz / fvtemp1.vy ;
		hit->vx = from->vx + fvtemp3.vy * fvtemp1.vx / fvtemp1.vy ;
	}
	if( hit->vz < bound_min->vz || hit->vz > bound_max->vz ) goto cuty_failed;
	if( hit->vx < bound_min->vx || hit->vx > bound_max->vx ) goto cuty_failed;
	return 1 ;

cuty_failed :
	if( fvtemp1.vz == 0 ){
		return 1 ;
	}else if( fvtemp1.vz < 0 ){
		hit->vz = bound_max->vz;
		hit->vx = from->vx + fvtemp2.vz * fvtemp1.vx / fvtemp1.vz ;
		hit->vy = from->vy + fvtemp2.vz * fvtemp1.vy / fvtemp1.vz ;
	}else{
		hit->vz = bound_min->vz;
		hit->vx = from->vx + fvtemp3.vz * fvtemp1.vx / fvtemp1.vz ;
		hit->vy = from->vy + fvtemp3.vz * fvtemp1.vy / fvtemp1.vz ;
	}
	if( hit->vx < bound_min->vx || hit->vx > bound_max->vx ) return 0;
	if( hit->vy < bound_min->vy || hit->vy > bound_max->vy ) return 0;
	return 1 ;
}

/* 検索直線とバンダリの交差判定 */
/* from が バンダリの中から */
int	OK_CheckCrossBoundaryInner( FVECTOR *from, FVECTOR *to, FVECTOR *bound_min, FVECTOR *bound_max, FVECTOR *hit )
{
	FVECTOR	fvtemp1;
	FVECTOR	fvtemp2;
	FVECTOR	fvtemp3;

	_sceVu0SubVector( &fvtemp1, to, from ) ;
	_sceVu0SubVector( &fvtemp2, bound_max, from ) ;
	_sceVu0SubVector( &fvtemp3, bound_min, to ) ;

	if( fvtemp1.vx == 0 ){
		goto cutx_failed;
	}else if( fvtemp1.vx > 0 ){
		hit->vx = bound_max->vx;
		hit->vy = from->vy + fvtemp2.vx * fvtemp1.vy / fvtemp1.vx ;
		hit->vz = from->vz + fvtemp2.vx * fvtemp1.vz / fvtemp1.vx ;
	}else{
		hit->vx = bound_min->vx;
		hit->vy = from->vy + fvtemp3.vx * fvtemp1.vy / fvtemp1.vx ;
		hit->vz = from->vz + fvtemp3.vx * fvtemp1.vz / fvtemp1.vx ;
	}
	if( hit->vy < bound_min->vy || hit->vy > bound_max->vy ) goto cutx_failed;
	if( hit->vz < bound_min->vz || hit->vz > bound_max->vz ) goto cutx_failed;
	return 1 ;

cutx_failed :
	if( fvtemp1.vy == 0 ){
		goto cuty_failed;
	}else if( fvtemp1.vy > 0 ){
		hit->vy = bound_max->vy;
		hit->vz = from->vz + fvtemp2.vy * fvtemp1.vz / fvtemp1.vy ;
		hit->vx = from->vx + fvtemp2.vy * fvtemp1.vx / fvtemp1.vy ;
	}else{
		hit->vy = bound_min->vy;
		hit->vz = from->vz + fvtemp3.vy * fvtemp1.vz / fvtemp1.vy ;
		hit->vx = from->vx + fvtemp3.vy * fvtemp1.vx / fvtemp1.vy ;
	}
	if( hit->vz < bound_min->vz || hit->vz > bound_max->vz ) goto cuty_failed;
	if( hit->vx < bound_min->vx || hit->vx > bound_max->vx ) goto cuty_failed;
	return 1 ;

cuty_failed :
	if( fvtemp1.vz == 0 ){
		return 1 ;
	}else if( fvtemp1.vz > 0 ){
		hit->vz = bound_max->vz;
		hit->vx = from->vx + fvtemp2.vz * fvtemp1.vx / fvtemp1.vz ;
		hit->vy = from->vy + fvtemp2.vz * fvtemp1.vy / fvtemp1.vz ;
	}else{
		hit->vz = bound_min->vz;
		hit->vx = from->vx + fvtemp3.vz * fvtemp1.vx / fvtemp1.vz ;
		hit->vy = from->vy + fvtemp3.vz * fvtemp1.vy / fvtemp1.vz ;
	}
	if( hit->vx < bound_min->vx || hit->vx > bound_max->vx ) return 0;
	if( hit->vy < bound_min->vy || hit->vy > bound_max->vy ) return 0;
	return 1 ;
}

/* メインメモリからスクラッチパッドへ転送 */
void OK_Mem_Scr( void *dst, void *src, int size, int num )
{
	UTL_StartMemToSpr( dst, src, size * num / sizeof(u_long128) );
	UTL_EndMemToSpr() ;
}

/* スクラッチパッドからメインメモリへ転送 */
void OK_Scr_Mem( void *dst, void *src, int size, int num )
{
	UTL_StartSprToMem( dst, src, size * num / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}

/* 床から平面バンダリを抽出（高さは調べない） */
void OK_FloorToBoundary( HZX_FLR *flr, FVECTOR *fv )
{
	/* x:min */
	fv->vx = flr->p1.x;
	fv->vx = (fv->vx < flr->p2.x)?fv->vx:flr->p2.x;
	fv->vx = (fv->vx < flr->p3.x)?fv->vx:flr->p3.x;
	fv->vx = (fv->vx < flr->p4.x)?fv->vx:flr->p4.x;

	/* z:min */
	fv->vy = flr->p1.z;
	fv->vy = (fv->vy < flr->p2.z)?fv->vy:flr->p2.z;
	fv->vy = (fv->vy < flr->p3.z)?fv->vy:flr->p3.z;
	fv->vy = (fv->vy < flr->p4.z)?fv->vy:flr->p4.z;

	/* x:max */
	fv->vz = flr->p1.x;
	fv->vz = (fv->vz > flr->p2.x)?fv->vz:flr->p2.x;
	fv->vz = (fv->vz > flr->p3.x)?fv->vz:flr->p3.x;
	fv->vz = (fv->vz > flr->p4.x)?fv->vz:flr->p4.x;

	/* z:max */
	fv->vw = flr->p1.z;
	fv->vw = (fv->vw > flr->p2.z)?fv->vw:flr->p2.z;
	fv->vw = (fv->vw > flr->p3.z)?fv->vw:flr->p3.z;
	fv->vw = (fv->vw > flr->p4.z)?fv->vw:flr->p4.z;
}


/* 床が無かった時の初期化 */
void OK_NoFloorInit( FVECTOR *fv, float *fl )
{
	/* 常にバンダリに引っかかるようにする */
	/* （座標移動して下に床があった時用） */
	fv->vx =  2100000000.0f;
	fv->vy =  2100000000.0f;
	fv->vz = -2100000000.0f;
	fv->vw = -2100000000.0f;

	/* 跳ね返りはしないように最低床はとことん下げる */
	fl[0]  = -2100000000.0f;
	fl[1]  =  2100000000.0f;
}


/* あたりを見ながら進む点（反射） */
int OK_StepCheckHzd( FVECTOR *now, FVECTOR *before, FVECTOR *vec, float decay, float margin, HZX_GROUP_ID map_id )
{
	int	flag;
	float		len;
	float		ratio;
	FVECTOR		local_vec;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	FVECTOR		next;
	HZX_SEG		*seg ;
	HZX_FLR		flr[2] ;
	int			atr[2] ;

	len = GV_VecLen3F( vec );
	ratio = margin * 1.41421f / len + 1.0f;
	local_vec.vx = vec->vx  * ratio;
	local_vec.vy = vec->vy  * ratio;
	local_vec.vz = vec->vz  * ratio;


	_sceVu0AddVector( &next, before, &local_vec );

//	if( vec->vy < 0.0f ){
		flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
			map_id,
			before,
			&next,
			HZX_CHK_FIX,
	        HZX_SEG_NO_PLAYER,
	        HZX_FLOOR_NO_PLAYER );
//	}else{
//		flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
//			map_id,
//			before,
//			&next,
//			HZX_CHK_F_SEGMENT,
//	        HZX_SEG_NO_PLAYER,
//	        HZX_FLOOR_NO_PLAYER );
//	}


//printf("flag:%d\n",flag);


	if( flag==1 ){	/* 壁 */
		HZX_GetOnlinePoint( &fvtemp1 );
		HZX_GetOnlineHazard( flr, atr ) ;
		seg=(HZX_SEG *)flr;
		fvtemp0.vx = seg->p2.z - seg->p1.z ;
		fvtemp0.vy = 0.0f ;
		fvtemp0.vz = seg->p1.x - seg->p2.x ;
		DG_ReflectVector( &fvtemp0, vec, vec );
		vec->vx *= 0.5f;
		vec->vy *= decay;
		vec->vz *= 0.5f;
		DG_COPY_VEC( now, before );
//		now->vx = fvtemp1.vx + vec->vx * ratio;
//		now->vy = fvtemp1.vy + vec->vy * ratio;
//		now->vz = fvtemp1.vz + vec->vz * ratio;
		return 1;
	}else if( flag==2 ){	/* 床 */
		HZX_GetOnlinePoint( &fvtemp1 );
#if 0
		vec->vx = rnd()*vec->vy - vec->vy*0.5f;
		vec->vz = rnd()*vec->vy - vec->vy*0.5f;
		vec->vy*= -decay;
#else
//		vec->vx*= -decay;
//		vec->vz*= -decay;
		vec->vy*= -decay;
#endif
		DG_COPY_VEC( now, before );
		now->vy = fvtemp1.vy + margin;

//		now->vx = fvtemp1.vx + vec->vx * ratio;
//		now->vy = fvtemp1.vy + vec->vy * ratio;
//		now->vz = fvtemp1.vz + vec->vz * ratio;
		return 1;
	}else{
//printf("%f\n",OK_slow_param);

#if 0
		_sceVu0ScaleVector( &fvtemp1, vec, OK_slow_param );
 		_sceVu0AddVector( now, before, &fvtemp1 );
#else
 		_sceVu0AddVector( now, before, vec );
#endif
//		DG_COPY_VEC( before, &next );
	}
	return 0;
}

/* 点が床面の上にあるか？ */
int OK_CheckPos_Floor( HZX_FLR *flr, FVECTOR *pos )
{
	float temp_c ;
	float temp_s ;
	float tmp0 ;
	float tmp1 ;
	float tmp2 ;

	/* 四角を三角にして検査 */
	//---------------------
	/* p1 p2 p3 */
	/* p1を原点とし、
	   Ｘ－Ｚ平面への投影を、
	   p1-p2線をＸ軸に合わせるよう全点を回転させ、
	   Ｚの符号比較を行なう */
	tmp0 = atan2f( flr->p2.x - flr->p1.x, flr->p2.z - flr->p1.z );
	temp_c = cosf( tmp0 );
	temp_s = sinf( tmp0 );
	tmp1 = (flr->p3.z - flr->p1.z) * temp_c  -  (flr->p3.x - flr->p1.x) * temp_s;
	tmp2 = (pos->vz   - flr->p1.z) * temp_c  -  (pos->vx   - flr->p1.x) * temp_s;
	if( (tmp1 < 0.0f && tmp2 > 0.0f)
	 || (tmp1 > 0.0f && tmp2 < 0.0f) ){
		return 0;
	}

	/* p2 p3 p1 */
	/* p2を原点とし、
	   Ｘ－Ｚ平面への投影を、
	   p2-p3線をＸ軸に合わせるよう全点を回転させ、
	   Ｚの符号比較を行なう */
	tmp0 = atan2f( flr->p3.x - flr->p2.x, flr->p3.z - flr->p2.z );
	temp_c = cosf( tmp0 );
	temp_s = sinf( tmp0 );
	tmp1 = (flr->p1.z - flr->p2.z) * temp_c  -  (flr->p1.x - flr->p2.x) * temp_s;
	tmp2 = (pos->vz   - flr->p2.z) * temp_c  -  (pos->vx   - flr->p2.x) * temp_s;
	if( (tmp1 < 0.0f && tmp2 > 0.0f)
	 || (tmp1 > 0.0f && tmp2 < 0.0f) ){
		return 0;
	}


	//---------------------
	/* p3 p4 p1 */
	/* p3を原点とし、
	   Ｘ－Ｚ平面への投影を、
	   p3-p4線をＸ軸に合わせるよう全点を回転させ、
	   Ｚの符号比較を行なう */
	tmp0 = atan2f( flr->p4.x - flr->p3.x, flr->p4.z - flr->p3.z );
	temp_c = cosf( tmp0 );
	temp_s = sinf( tmp0 );
	tmp1 = (flr->p1.z - flr->p3.z) * temp_c  -  (flr->p1.x - flr->p3.x) * temp_s;
	tmp2 = (pos->vz   - flr->p3.z) * temp_c  -  (pos->vx   - flr->p3.x) * temp_s;
	if( (tmp1 < 0.0f && tmp2 > 0.0f)
	 || (tmp1 > 0.0f && tmp2 < 0.0f) ){
		return 0;
	}

	/* p4 p1 p3 */
	/* p4を原点とし、
	   Ｘ－Ｚ平面への投影を、
	   p4-p1線をＸ軸に合わせるよう全点を回転させ、
	   Ｚの符号比較を行なう */
	tmp0 = atan2f( flr->p1.x - flr->p4.x, flr->p1.z - flr->p4.z );
	temp_c = cosf( tmp0 );
	temp_s = sinf( tmp0 );
	tmp1 = (flr->p3.z - flr->p4.z) * temp_c  -  (flr->p3.x - flr->p4.x) * temp_s;
	tmp2 = (pos->vz   - flr->p4.z) * temp_c  -  (pos->vx   - flr->p4.x) * temp_s;
	if( (tmp1 < 0.0f && tmp2 > 0.0f)
	 || (tmp1 > 0.0f && tmp2 < 0.0f) ){
		return 0;
	}

	return 1;

}


/*
	床面の端チェック
	水平面に投影されたものでチェック
	ans_vec に エッジ方向
*/
int OK_CheckFloorEdge( FVECTOR *ans_vec, FVECTOR *pos, float check_length, HZX_FLR *flr )
{
	float	length;
	FVECTOR	fvtemp1;

	/* 頂点１ */
	/* 直線の正規化法線 */
	ans_vec->vx =  (flr->p2.z - flr->p1.z);
	ans_vec->vy =  0.0f;
	ans_vec->vz = -(flr->p2.x - flr->p1.x);
	_sceVu0Normalize( ans_vec, ans_vec);

	/* チェックする点から線分の端点までのベクトル */
	fvtemp1.vx =  (flr->p1.x - pos->vx);
	fvtemp1.vy =  0.0f;
	fvtemp1.vz =  (flr->p1.z - pos->vz);
//printf("%f %f %f\n",fvtemp1.vx,fvtemp1.vy,fvtemp1.vz);

	/* 点から線分までの距離 */
	length = _sceVu0InnerProduct( ans_vec, &fvtemp1 );
	length = (length<0)?-length:length;
//printf("length1:%f\n",length);
	if( length < check_length ) return 1;	/* エッジだった */


	/* 頂点２ */
	/* 直線の正規化法線 */
	ans_vec->vx =  (flr->p3.z - flr->p2.z);
	ans_vec->vy =  0.0f;
	ans_vec->vz = -(flr->p3.x - flr->p2.x);
	_sceVu0Normalize( ans_vec, ans_vec);

	/* チェックする点から線分の端点までのベクトル */
	fvtemp1.vx =  (flr->p2.x - pos->vx);
	fvtemp1.vy =  0.0f;
	fvtemp1.vz =  (flr->p2.z - pos->vz);

	/* 点から線分までの距離 */
	length = _sceVu0InnerProduct( ans_vec, &fvtemp1 );
//printf("length2:%f\n",length);
	if( length < check_length ) return 1;	/* エッジだった */


	/* 頂点３ */
	/* 直線の正規化法線 */
	ans_vec->vx =  (flr->p4.z - flr->p3.z);
	ans_vec->vy =  0.0f;
	ans_vec->vz = -(flr->p4.x - flr->p3.x);
	_sceVu0Normalize( ans_vec, ans_vec);

	/* チェックする点から線分の端点までのベクトル */
	fvtemp1.vx =  (flr->p3.x - pos->vx);
	fvtemp1.vy =  0.0f;
	fvtemp1.vz =  (flr->p3.z - pos->vz);

	/* 点から線分までの距離 */
	length = _sceVu0InnerProduct( ans_vec, &fvtemp1 );
//printf("length3:%f\n",length);
	if( length < check_length ) return 1;	/* エッジだった */


	/* 頂点４ */
	/* 直線の正規化法線 */
	ans_vec->vx =  (flr->p1.z - flr->p4.z);
	ans_vec->vy =  0.0f;
	ans_vec->vz = -(flr->p1.x - flr->p4.x);
	_sceVu0Normalize( ans_vec, ans_vec);

	/* チェックする点から線分の端点までのベクトル */
	fvtemp1.vx =  (flr->p4.x - pos->vx);
	fvtemp1.vy =  0.0f;
	fvtemp1.vz =  (flr->p4.z - pos->vz);

	/* 点から線分までの距離 */
	length = _sceVu0InnerProduct( ans_vec, &fvtemp1 );
//printf("length4:%f\n",length);
	if( length < check_length ) return 1;	/* エッジだった */

	return 0;	/* フロア内だった */

}

/* トラップ進入チェック */
int OK_CheckTrap( CONTROL *ctrl, int trap_id )
{
	u_int	*inside ;
	int		n_inside ;

	n_inside = ctrl->evt.n_inside ;
	inside = ctrl->evt.inside ;
	while ( -- n_inside >= 0 ) {
		if ( ( *inside & 0x00ffffff ) == ( trap_id & 0x00ffffff ) ){
			return ( *inside ) ;
		}
		inside ++ ;
	}
	return 0 ;
}

/* バウンドが画面内に入っているかどうか */
int	OK_CheckBoundOnCamera( FVECTOR *fvec0, FVECTOR *fvec1, int cam_num )
{
	int	i;
	int	count;
	float	fmax_x,fmax_y;
	float	fmin_x,fmin_y;
	FVECTOR	*pfv;
	FVECTOR	fv[8];

#ifndef BP_PSX2_ASM
	fv[0].vx = fvec0->vx;
	fv[0].vy = fvec0->vy;
	fv[0].vz = fvec0->vz;

	fv[1].vx = fvec0->vx;
	fv[1].vy = fvec0->vy;
	fv[1].vz = fvec1->vz;

	fv[2].vx = fvec0->vx;
	fv[2].vy = fvec1->vy;
	fv[2].vz = fvec0->vz;

	fv[3].vx = fvec0->vx;
	fv[3].vy = fvec1->vy;
	fv[3].vz = fvec1->vz;

	fv[4].vx = fvec1->vx;
	fv[4].vy = fvec0->vy;
	fv[4].vz = fvec0->vz;

	fv[5].vx = fvec1->vx;
	fv[5].vy = fvec0->vy;
	fv[5].vz = fvec1->vz;

	fv[6].vx = fvec1->vx;
	fv[6].vy = fvec1->vy;
	fv[6].vz = fvec0->vz;

	fv[7].vx = fvec1->vx;
	fv[7].vy = fvec1->vy;
	fv[7].vz = fvec1->vz;
#else
	asm volatile ("
		lqc2 vf8,0(%1)
		lqc2 vf9,0(%2)
		vmove.xyz	vf10, vf8
		vmove.xy	vf11, vf8
		vmove.z		vf11, vf9
		vmove.xz	vf12, vf8
		vmove.y		vf12, vf9
		vmove.x		vf13, vf8
		vmove.yz	vf13, vf9
		vmove.yz	vf14, vf8
		vmove.x		vf14, vf9
		vmove.y		vf15, vf8
		vmove.xz	vf15, vf9
		vmove.z		vf16, vf8
		vmove.xy	vf16, vf9
		vmove.xyz	vf17, vf9
		sqc2 vf10,	0x00(%0)
		sqc2 vf11,	0x10(%0)
		sqc2 vf12,	0x20(%0)
		sqc2 vf13,	0x30(%0)
		sqc2 vf14,	0x40(%0)
		sqc2 vf15,	0x50(%0)
		sqc2 vf16,	0x60(%0)
		sqc2 vf17,	0x70(%0)
	" :: "r"(fv), "r"(fvec0), "r"(fvec1) );
#endif

	/* 透視変換する */
	DG_SetPos( &DG_Chanls[cam_num].eye_pers );
	DG_PutVector( fv, fv, 8 );

	// ２Ｄのバンダリに変換
	pfv = fv;
	count = 0;
	for( i=0; i<8; i++ ){
		if( pfv->vz > pfv->vw ) count++;
		pfv++;
	}
	if( count == 8 ) return 0;	// 全部カメラの後ろ

	pfv = fv;
	for( i=0; i<8; i++ ){
		pfv->vw = (pfv->vw > 0.0f)? pfv->vw: -pfv->vw; 
		pfv->vx /= pfv->vw;
		pfv->vy /= pfv->vw;
		pfv++;
	}

	pfv = fv;
	fmax_x = fmin_x = pfv->vx;
	fmax_y = fmin_y = pfv->vy;
	pfv++;
	for( i=0; i<7; i++ ){
		fmax_x = ( pfv->vx > fmax_x )? pfv->vx: fmax_x;
		fmin_x = ( pfv->vx < fmin_x )? pfv->vx: fmin_x;
		fmax_y = ( pfv->vy > fmax_y )? pfv->vy: fmax_y;
		fmin_y = ( pfv->vy < fmin_y )? pfv->vy: fmin_y;
		pfv++;
	}

	// ２Ｄバンダリの画面内チェック
	if( fmax_x < -1.0f ) return 0;
	if( fmin_x >  1.0f ) return 0;
	if( fmax_y < -1.0f ) return 0;
	if( fmin_y >  1.0f ) return 0;

	return 1 ;
}

void OK_ScaleVector( FVECTOR *output_v, FVECTOR *input_v, float scale, int num )
{
	while( --num >= 0 ){
		_sceVu0ScaleVector( output_v++, input_v++, scale );
	}
}





