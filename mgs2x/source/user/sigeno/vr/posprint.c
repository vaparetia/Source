//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	posprint.c
	ＶＲステージ空中文字
	2002/03/25 K.Sigeno
	$Id: posprint.c,v 1.1.1.3 2002/11/19 11:49:56 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include	<string.h>

#include "libutl.h"
#include "gameheader.h"
#include "vr.h"


#define TEX_SCALE (0.70f)
#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))


//extern void PosBox(FVECTOR * ,float ,SVECTOR * );


#define Y_DEF (9)
typedef	struct _Work {
	GV_ACT_EX	actor ;
	char	str1[8];
	char	str2[8];
	char	str3[8];
	int		time ;
	int		mode ;
	int		limit ;
	int		*flag ;
	FVECTOR	pos;
	FVECTOR	shift;
	FMATRIX	*posbuf;
	DG_PRIM2		*prim_sprt ;

} Work ;


#if 0
{
	float     tmp ;
	FVECTOR   ret ;   
	int       x,y ;   /* 表示座標 */
	DG_CHANL  *cp ;
	u_char	r,g,b,alpha	;
	r = 127;g=127;b=127;
	
	/* 位置決定 */
	cp = DG_Chanl(0) ;
	tmp = work->pos.vw ;
	work->pos.vw = 1.0F ;
	_sceVu0ApplyMatrix(&ret, &cp->eye_pers, &work->pos) ;
	work->pos.vw = tmp ;
}
#endif
static void Act(Work *work)
{
	FVECTOR *pos ,tmp;

	if(*work->flag & DG_FLAG_INVISIBLE){
		DG_InvisiblePrim2(work->prim_sprt) ;
		return ;
	}else {
		DG_VisiblePrim2(work->prim_sprt) ;
	}

	GV_MatToVec( work->posbuf, &work->pos ) ;

//
#if 1
//	FMATRIX		eye_pers ;			/* カメラ透視変換マトリクス（=pers*eye_inv） */
//	FMATRIX		eye_inv ;			/* カメラ逆行列				*/
//	FMATRIX		eye ;				/* カメラ行列				*/
//	FMATRIX		pers ;				/* 透視変換マトリクス */

	{
		DG_CHANL  *cp ;
		cp = DG_Chanl(0) ;
//		DG_SetPos(&cp->eye_pers) ;
//		DG_SetPos(&cp->eye_inv) ;
		DG_SetPos(&cp->eye) ;
//		DG_SetPos(&cp->pers) ;
		DG_RotVector( &work->shift, &tmp, 1 ) ;
	}
#else
	tmp = work->shift ;
#endif
#if 0
printf("shift.vx[%f]\n",work->shift.vx);
printf("shift.vy[%f]\n",work->shift.vy);
printf("shift.vz[%f]\n",work->shift.vz);

printf("tmp.vx[%f]\n",tmp.vx);
printf("tmp.vy[%f]\n",tmp.vy);
printf("tmp.vz[%f]\n",tmp.vz);

ASSERT(0) ;
#endif

	work->pos.vx += tmp.vx ;

	work->pos.vy += tmp.vy ;
//	work->pos.vy -= tmp.vy ;
//	work->pos.vy += work->shift.vy ;

	work->pos.vz += tmp.vz ;
//	work->pos.vz -= tmp.vz ;

	pos = work->prim_sprt->pos[work->prim_sprt->buffer_clock] ;
	pos->vx = work->pos.vx ;
	pos->vy = work->pos.vy ;
	pos->vz = work->pos.vz ;

#if 0
	GV_DestroyActor(work) ;
#endif
}
static void Die(Work *work)
{
	DG_DequeuePrim2( work->prim_sprt );
	DG_FreePrim2( work->prim_sprt );
}
static int GetResources(Work *work,FMATRIX *pos ,int mode,int *flag ,float shift)
{
	DG_TEX		*name_tex ;
	DG_PRIM2		*prim ;
	int i,j,k,tex_code;
	short tex_w,tex_h ;
	FVECTOR *prim_pos ;

	work->time = 0 ;
	work->posbuf = pos ;
	GV_MatToVec( work->posbuf, &work->pos ) ;

	switch(mode){
		case VR_TRG_TYPE_TRI :
			tex_code = NAME_TAG_TRI_DEF ;
			break;
		case VR_TRG_TYPE_PUNCH :
			tex_code = NAME_TAG_OCT_DEF ;
			break;
		case VR_TRG_TYPE_OCT :
			tex_code = NAME_TAG_OCT_DEF ;
			break;
		case VR_TRG_TYPE_KATANA :
			tex_code = NAME_TAG_STR_DEF ;
			break;
		case VR_TRG_TYPE_CROSS :
			tex_code = NAME_TAG_SQU_DEF ;
			break;
		case VR_TRG_TYPE_HEX :
			tex_code = NAME_TAG_HEX_DEF ;
			break;
		case VR_TRG_TYPE_MOVE_CUBE :
			tex_code = NAME_TAG_CUB_DEF ;
			break;
		case VR_TRG_TYPE_ONE_TRI :
			tex_code = NAME_TAG_TRI_DEF ;
			break;
		case VR_TRG_TYPE_ENEMY :
			tex_code = NAME_TAG_GBS_DEF ;
			break;
		default :
			tex_code = NAME_TAG_TRI_DEF ;
	}

	name_tex = DG_GetTexture( tex_code );

	prim = work->prim_sprt = 
	  GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, 1, 1 );
	DG_ConfigPrim2Tex( work->prim_sprt, name_tex );
//	DG_InvisiblePrim2(work->prim_sprt) ;
	DG_VisiblePrim2(work->prim_sprt) ;

	DG_SetPrim2Alpha( work->prim_sprt, KASAN );

	for(i= 0 ;i<2;i++){
		DG_PRIM2_UVRGBWH *uvrgbwh ;
		for(k= 0 ; k<1;k++){
			uvrgbwh = (DG_PRIM2_UVRGBWH *)work->prim_sprt->uvrgb[i] ;
			uvrgbwh += k*1 ;
			tex_w = FTOI12( name_tex->u_scale ) ;
			tex_h = FTOI12( name_tex->v_scale ) ;


			prim_pos = work->prim_sprt->pos[i] ;
			prim_pos->vx = work->pos.vx ;
			prim_pos->vy = work->pos.vy ;
			prim_pos->vz = work->pos.vz ;

			for (j= 0 ; j<1 ;j++){
				uvrgbwh->u0 = FTOI12( 0.0F * name_tex->u_scale + name_tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * name_tex->v_scale + name_tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * name_tex->u_scale + name_tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * name_tex->v_scale + name_tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
			/* ＷＨ値は整数なので注意！ */
				uvrgbwh->w = (tex_w/2) *TEX_SCALE;/* 幅／２ */
				uvrgbwh->h = (tex_h/2) *TEX_SCALE;/* 高さ／２ */
//printf("uvrgbwh->w[%d]uvrgbwh->h[%d]\n",uvrgbwh->w,uvrgbwh->h);
				uvrgbwh->r  = 127;
				uvrgbwh->g  = 127;
				uvrgbwh->b  = 127;
//				uvrgbwh->a  = 64;
				uvrgbwh->a  = 100;
				uvrgbwh++ ;
			}
		}
	}


//	strcpy(work->str1,str);
//	strcpy(work->str1,"TEST");
//	work->limit = time ;
	work->mode = mode ;
	work->shift.vx = (float)(tex_w/2) * TEX_SCALE;
	work->shift.vy = -shift;
	work->shift.vz = 0.0f;
	work->flag = flag ;

printf("POSPRINT shift[%f]\n",work->shift.vy) ;

	return 1 ;
}
void *NewSIG_3DPOSPrint(FMATRIX *pos ,int mode,int *flag ,float shift)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,pos ,mode,flag ,shift)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return work ;
}

