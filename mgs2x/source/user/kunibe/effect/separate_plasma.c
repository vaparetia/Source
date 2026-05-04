//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  separate_plasma.c
  ３点指定ポリゴンプラズマ(位置固定型)
  2001/05/21 Yuuta Kunibe	
  $Id: separate_plasma.c,v 1.1.1.3 2002/11/19 11:44:48 Yoshizawa1 Exp $
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
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"


#define COLOR			(255)
#define ALPHA			(128)

#define N_PRIMS			(24)//(16)//(32)
#define N_VERTS 		(32)
#define SPRITE_NUM 		( N_PRIMS * N_VERTS )
#define HALF_NUM		( SPRITE_NUM / 2 )
#define PART_NUM		( SPRITE_NUM / 3 )

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	WAVE_HEIGHT		(50.0f)//(80.0f)//(100.0f)
#define SPEED			(50.0f)//(100.0f)//(100.0f)//(50.0f)


typedef struct {
    FVECTOR	height;
    FVECTOR	wave;
    FVECTOR	noise;
    FVECTOR	speed;
} PointParam;


typedef	struct {

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	*from;
    FVECTOR	middle;
    FVECTOR	*to1;
    FVECTOR	*to2;

    PointParam	param[SPRITE_NUM];

    int		alpha;
    int		sub_alpha;
    int		cnt;
    int		life;

    float	width;

} Work;



static inline void AddVector4( FVECTOR *r, FVECTOR *a, FVECTOR *b, FVECTOR *c, FVECTOR *d  )
{
// yano /*未検証*/
#ifdef BP_PSX2_ASM
	asm volatile ("
	lqc2		vf1, 0(%0)
	lqc2		vf2, 0(%1)
	vadd.xyz	vf5, vf2, vf1
	lqc2		vf3, 0(%2)
	vadd.xyz	vf6, vf3, vf5
	lqc2		vf4, 0(%3)
	vadd.xyz	vf7, vf4, vf6
	sqc2		vf7, 0(%4)
	" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(r) ) ;
#else
	FVECTOR fvtemp0, fvtemp1;
	_sceVu0AddVector( &fvtemp0, a, b );
	_sceVu0AddVector( &fvtemp1, c, d );
	_sceVu0AddVector( r, &fvtemp0, &fvtemp1 );
#endif
}



/* アクト関数 */
static void Act( Work *work )
{

    int 		i,j,k;
    int	  		clock;
    FVECTOR		*pos;
    DG_PRIM2_UVRGBWH	*uvrgbwh;
    FVECTOR		vectmp;



    /* 追従用 */
    FVECTOR		center;

    FVECTOR		from_middle;
    FVECTOR		middle_to;

    FVECTOR		m_bottom;

    FVECTOR		vec_bottom;
    FVECTOR		vec_bottom1;
    FVECTOR		vec_bottom2;
    float		length;
    FVECTOR		height;

    FVECTOR		center_tmp;

    PointParam		*p_param;

    int			noise_flag;
 

    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;
    pos	        = work->prim->pos[clock];
    uvrgbwh     = work->prim->uvrgb[clock];

    p_param = work->param;



    /* 追従用 */
    /* from -> middle */	
    _sceVu0SubVector( &from_middle, &work->middle, work->from );
    _sceVu0SubVector( &vectmp, work->to1, work->from );
    _sceVu0Normalize( &vectmp, &vectmp );
    length = _sceVu0InnerProduct( &from_middle, &vectmp );
    _sceVu0ScaleVector( &m_bottom, &vectmp, length );

    _sceVu0SubVector( &height, &from_middle, &m_bottom );
    _sceVu0ScaleVector( &vec_bottom, &m_bottom, 1.0f / (float)PART_NUM );

    _sceVu0AddVector( &m_bottom, &m_bottom, work->from );


    /* middle -> to1 */
    _sceVu0SubVector( &middle_to, work->to1, &work->middle );
    length = _sceVu0InnerProduct( &middle_to, &vectmp );
    _sceVu0ScaleVector( &vec_bottom1, &vectmp, length );
    _sceVu0ScaleVector( &vec_bottom1, &vec_bottom1, 1.0f / (float)PART_NUM );


    /* middle -> to2 */
    _sceVu0SubVector( &middle_to, work->to2, &work->middle );
    _sceVu0SubVector( &vectmp, work->to2, &m_bottom );
    _sceVu0Normalize( &vectmp, &vectmp );
    length = _sceVu0InnerProduct( &middle_to, &vectmp );
    _sceVu0ScaleVector( &vec_bottom2, &vectmp, length );
    _sceVu0ScaleVector( &vec_bottom2, &vec_bottom2, 1.0f / (float)PART_NUM );


    //AN_Test_Eye2( work->from, 500.f );
    //AN_Test_Eye2( work->to1, 500.f );
    //AN_Test_Eye2( work->to2, 500.f );


    if (!(irnd()%10)) {
	noise_flag = 1;
    }
    else {
	noise_flag = 0;
    }

    for ( i = 0 ; i < 3 ; i++ ) {

	pos = SCR_POS;
	
	for ( j = 0 ; j < N_PRIMS/3 ; j++ ) {

	    for ( k = 0 ; k < N_VERTS ; k++ ) {

		if ( i == 0 && j == 0 && k == 0 ) {

		    DG_COPY_VEC( &center, work->from );
		    _sceVu0AddVector( pos, work->from, &p_param->height );
		    _sceVu0AddVector( pos, pos, &p_param->wave );

		}
		else {

		    if ( j== 0 && k == 0 ) {
			if ( i==1 ) {
			    DG_COPY_VEC( &center_tmp, &center );
			}
			else if ( i==2 ) {
			    DG_COPY_VEC( &center, &center_tmp );
			}
		    }

		    /* 追従用 */
		    switch ( i%3 ) {
		    case 0:
			_sceVu0AddVector( &center, &center, &vec_bottom );
			break;
		    case 1:
			_sceVu0AddVector( &center, &center, &vec_bottom1 );
			break;
		    case 2:
			_sceVu0AddVector( &center, &center, &vec_bottom2 );
			break;
		    }



		    /* プラズマ点を算出 */
#if 1			
		    AddVector4( pos, &center, &p_param->height, &p_param->noise, &p_param->wave );
		    //AddVector4( pos, &center, &p_param->height, &p_param->noise, &DG_ZeroVector );
		    //AddVector4( pos, &center, &p_param->height, &DG_ZeroVector, &DG_ZeroVector );
#else			
		    _sceVu0AddVector( pos, &center, &p_param->height );
		    _sceVu0AddVector( pos, pos, &p_param->noise );
		    _sceVu0AddVector( pos, pos, &p_param->wave );
#endif

		    /* 次点パラメータ更新 */	    
		    _sceVu0SubVector( &p_param->noise, &p_param->noise, &p_param->speed );
			
		    if ( work->cnt < work->life / 2 ) {
			_sceVu0ScaleVector( &p_param->height, &p_param->height, 1.03f );
			_sceVu0ScaleVector( &p_param->wave, &p_param->wave, 1.02f );
			if ( noise_flag ) {
			    _sceVu0SubVector( &p_param->noise, &DG_ZeroVector, &p_param->noise );
			    _sceVu0SubVector( &p_param->speed, &DG_ZeroVector, &p_param->speed );
			}
		    }
		    else {
			_sceVu0ScaleVector( &p_param->height, &p_param->height, 0.99f );
			_sceVu0ScaleVector( &p_param->wave, &p_param->wave, 0.98f );
		    _sceVu0ScaleVector( &p_param->speed, &p_param->speed, 0.97f );
		    }
		    //_sceVu0ScaleVector( &p_param->speed, &p_param->speed, 0.97f );

		}

		uvrgbwh->a = work->alpha;

		pos++;
		uvrgbwh++;

		p_param++;

	    }

	}

	OK_Scr_Mem( &work->prim->pos[clock][i*N_PRIMS/3*N_VERTS], SCR_POS, sizeof(FVECTOR), N_PRIMS/3*N_VERTS );

    }


    /* プラズマアルファ更新 */
    work->alpha -= work->sub_alpha;


    if ( ++work->cnt >= work->life ) {
	GV_DestroyActor( work );
    }

}


static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *color )
{

	int		i,j,k,l,n;
	FVECTOR		*pos;	
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	SVECTOR		rot;
	FMATRIX		mat;
	int		rot_flag;

	FVECTOR		center;

	FVECTOR		from_middle;
	FVECTOR		middle_to;

	FVECTOR		vectmp;
	FVECTOR		m_bottom;

	FVECTOR		vec_bottom;
	FVECTOR		vec_bottom1;
	FVECTOR		vec_bottom2;
	float		length;
	FVECTOR		height;

	float		param[3];
	float		add_param[3];
	int		param_flag[3];

	float		param_tmp[3];
	float		add_param_tmp[3];
	int		param_flag_tmp[3];
	FMATRIX		mat_tmp;

	PointParam	*p_param;

	FVECTOR		center_tmp;




	prim->raise = 0;
	
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



	/*
	  パラメータ算出
	  vec_bottom  : from->middle の bottom_vector
	  vec_bottom1 : middle->to1  の bottom_vector
	  vec_bottom2 : middle->to2  の bottom_vector
	  height      : middleの高さベクトル
	*/

	/* from -> middle */	
	_sceVu0SubVector( &from_middle, &work->middle, work->from );
	_sceVu0SubVector( &vectmp, work->to1, work->from );
	_sceVu0Normalize( &vectmp, &vectmp );
	length = _sceVu0InnerProduct( &from_middle, &vectmp );
	_sceVu0ScaleVector( &m_bottom, &vectmp, length );

	_sceVu0SubVector( &height, &from_middle, &m_bottom );
	_sceVu0ScaleVector( &vec_bottom, &m_bottom, 1.0f / (float)PART_NUM );

	_sceVu0AddVector( &m_bottom, &m_bottom, work->from );


	/* middle -> to1 */
	_sceVu0SubVector( &middle_to, work->to1, &work->middle );
	length = _sceVu0InnerProduct( &middle_to, &vectmp );
	_sceVu0ScaleVector( &vec_bottom1, &vectmp, length );
	_sceVu0ScaleVector( &vec_bottom1, &vec_bottom1, 1.0f / (float)PART_NUM );


	/* middle -> to2 */
	_sceVu0SubVector( &middle_to, work->to2, &work->middle );
	_sceVu0SubVector( &vectmp, work->to2, &m_bottom );
	_sceVu0Normalize( &vectmp, &vectmp );
	length = _sceVu0InnerProduct( &middle_to, &vectmp );
	_sceVu0ScaleVector( &vec_bottom2, &vectmp, length );
	_sceVu0ScaleVector( &vec_bottom2, &vec_bottom2, 1.0f / (float)PART_NUM );




	OK_DirVecXY( &vectmp, &DG_ZeroVector, &rot );
	DG_SetPos( &DG_UnitMatrix );
	DG_RotatePos( &rot );

	rot.vx = 0;
	rot.vy = 0;
	rot.vz = irnd()%4096;

	DG_RotatePos( &rot );
	DG_GetPos( &mat );	

	param[0] = 0.f;
	param[1] = 0.f;
	param[2] = 0.f;
	add_param[0] = 0.f;
	add_param[1] = 0.f;
	add_param[2] = 0.f;
	param_flag[0] = 0;
	param_flag[1] = 0;
	param_flag[2] = 0;

	if ( irnd()%2 ) {
	    rot_flag = 1;
	}
	else {
	    rot_flag = 0;
	}





	pos   	= SCR_POS;
	uvrgbwh	= SCR_UVS;	
	p_param = work->param;


	for ( l = 0 ; l < 4 ; l++ ) {

	    pos   	= SCR_POS;
	    uvrgbwh	= SCR_UVS;	
	
	    for ( i = 0 ; i < N_PRIMS/4; i++ ) {

		for ( j = 0 ; j < N_VERTS ; j++ ) {

		    DG_COPY_VEC( pos, &DG_ZeroVector );

		    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->q0 = 4096;
		    uvrgbwh->f0 = 0x0fff;

		    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->q1 = 4096;
		    uvrgbwh->f1 = 0x0fff;

		    uvrgbwh->w = work->width;
		    uvrgbwh->h = work->width;

		    uvrgbwh->r = color->vx;
		    uvrgbwh->g = color->vy;
		    uvrgbwh->b = color->vz;
		    uvrgbwh->a = color->vw;

		    pos++;
		    uvrgbwh++;


		    n = l * N_PRIMS * N_VERTS / 4 + i * N_VERTS + j;

		    if ( n%PART_NUM == 0 ) {
			switch ( n/PART_NUM ) {
			case 1:
			    DG_COPY_VEC( &center_tmp, &center );
			    DG_COPY_MAT( &mat_tmp, &mat );
			    for ( k = 0 ; k < 3 ; k++ ) {
				param_tmp[k] = param[k];
				add_param_tmp[k] = add_param[k];
				param_flag_tmp[k] = param_flag[k];
			    }
			    break;
			case 2:
			    DG_COPY_VEC( &center, &center_tmp );
			    DG_COPY_MAT( &mat, &mat_tmp );
			    for ( k = 0 ; k < 3 ; k++ ) {
				param[k] = param_tmp[k];
				add_param[k] = add_param_tmp[k];
				param_flag[k] = param_flag_tmp[k];
			    }
			    break;
			}
		    }




		    if ( !(j%16) ) {

			if ( rot_flag ) {
			    rot.vz = irnd()%80 + 2048/32;
			}
			else {
			    rot.vz = -irnd()%80 - 2048/32;
			}
		
			rot.vz *= 1.f / 32.f;

			//n = l*N_PRIMS/4 + i;

			if ( n < ( N_PRIMS - 4 ) * N_VERTS ) {
			    for ( k = 0 ; k < 3 ; k++ ) {
		    
				if ( param_flag[k] == 1 ) {
				    add_param[k] =-0.036f - frnd()*0.004f;
				    if ( param[k] + add_param[k] < 0.25f ) {
					param_flag[k] = 0;
				    }
				}
				else if ( param_flag[k] == -1 ) {
				    add_param[k] = 0.036f + frnd()*0.004f;
				    if ( param[k] + add_param[k] > -0.25f ) {
					param_flag[k] = 0;

				    }
				}
				else {			   			    
				    add_param[k] = frnd()*0.04f;
				    if ( param[k] + add_param[k] > 0.30f ) {
					param_flag[k] = 1;
				    }
				    else if ( param[k] + add_param[k] < -0.30f ) {
					param_flag[k] = -1;
				    }
				}

				add_param[k] *= 1.f/16.f;
		    
			    }
			}
			else {
			    for ( k = 0 ; k < 3 ; k++ ) {
				if ( param[k] > 0.0f ) {
				    add_param[k] =-0.036f - frnd()*0.004f;
				}
				else {
				    add_param[k] = 0.036f + frnd()*0.004f;
				}

				add_param[k] *= 1.f/16.f;
		    
			    }
			}

		    }



		

		

		    DG_SetPos( &mat );
		    DG_RotatePos( &rot );
		    DG_GetPos( &mat );

		    //n = l * N_PRIMS * N_VERTS / 4 + i * N_VERTS + j;

		    /* 中心点 */
		    /* 開始点 */
		    if ( n == 0 ) {					
			DG_COPY_VEC( &center, work->from );
			DG_COPY_VEC( &p_param->height, &DG_ZeroVector );
			_sceVu0ScaleVector( &p_param->wave, (FVECTOR *)mat.m[0], WAVE_HEIGHT );
			DG_COPY_VEC( &p_param->noise, &DG_ZeroVector );
			DG_COPY_VEC( &p_param->speed, &DG_ZeroVector );
		    }
		    else {
			switch ( n / PART_NUM ) {
			case 0:		/* from->middle */
			    _sceVu0AddVector( &center, &center, &vec_bottom );
			    _sceVu0ScaleVector( &p_param->height,
						&height,
						(float)( PART_NUM*PART_NUM - (PART_NUM-n)*(PART_NUM-n) ) / (float)(PART_NUM*PART_NUM) );
			    _sceVu0ScaleVector( &p_param->wave, (FVECTOR *)mat.m[0], WAVE_HEIGHT );
			    break;
			case 1:		/* middle->to1 */
			    _sceVu0AddVector( &center, &center, &vec_bottom1 );
			    _sceVu0ScaleVector( &p_param->height,
						&height,
						(float)( PART_NUM*PART_NUM - (n-PART_NUM)*(n-PART_NUM) ) / (float)(PART_NUM*PART_NUM) );
			    _sceVu0ScaleVector( &p_param->wave, (FVECTOR *)mat.m[0], WAVE_HEIGHT );
			    break;
			case 2:		/* middle->to2 */
			    n -= PART_NUM;
			    _sceVu0AddVector( &center, &center, &vec_bottom2 );
			    _sceVu0ScaleVector( &p_param->height,
						&height,
						(float)( PART_NUM*PART_NUM - (n-PART_NUM)*(n-PART_NUM) ) / (float)(PART_NUM*PART_NUM) );
			    _sceVu0ScaleVector( &p_param->wave, (FVECTOR *)mat.m[0], WAVE_HEIGHT );
			    break;
			    
			}
		    }

		    p_param->speed.vx = SPEED * param[0];
		    p_param->speed.vy = SPEED * param[1];
		    p_param->speed.vz = SPEED * param[2];

		    param[0] += add_param[0];
		    param[1] += add_param[1];
		    param[2] += add_param[2];

		    _sceVu0ScaleVector( &p_param->noise, &p_param->speed, 25.0f );

		    p_param++;

		}
	    
	
		OK_Scr_Mem( &prim->pos[ 0 ][l*N_PRIMS/4*N_VERTS], SCR_POS, sizeof(FVECTOR), N_PRIMS/4*N_VERTS );
		OK_Scr_Mem( &prim->pos[ 1 ][l*N_PRIMS/4*N_VERTS], SCR_POS, sizeof(FVECTOR), N_PRIMS/4*N_VERTS );
		OK_Scr_Mem( &((DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 0 ]))[l*N_PRIMS/4*N_VERTS], SCR_UVS,
			    sizeof(DG_PRIM2_UVRGBWH), N_PRIMS/4*N_VERTS );
		OK_Scr_Mem( &((DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 1 ]))[l*N_PRIMS/4*N_VERTS], SCR_UVS,
			    sizeof(DG_PRIM2_UVRGBWH), N_PRIMS/4*N_VERTS );

	    }


	}

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources(
			Work *work,
			FVECTOR *from,
			FVECTOR *middle,
			FVECTOR *to1,
			FVECTOR *to2,
			float width,
			FVECTOR *color,
			int life )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;


    work->from   = from;
    work->middle = *middle;
    work->to1    = to1;
    work->to2    = to2;

    work->life      = life;
    work->cnt       = 0;
    work->alpha     = color->vw;
    work->sub_alpha = (int)color->vw / work->life;

    work->width = width;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT | DG_PRIM2_ALPHA | DG_PRIM2_SHADE | DG_PRIM2_TEX,
				      N_PRIMS, N_VERTS );

    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, tex, color );
    
    return 0;

}



/*-------- NewSpritePlasma : スプライトプラズマ呼び出し関数 --------
     FVECTOR	*from	: 始点
     FVECTOR	*middle : 中継点
     FVECTOR	*to	: 終点
     float	width	: スプライト幅
     FVECTOR	*color	: 色
------------------------------------------------------------------*/  
void *NewSpritePlasmaSeparate(
			      FVECTOR *from,
			      FVECTOR *middle,
			      FVECTOR *to1,
			      FVECTOR *to2,
			      float width,
			      FVECTOR *color,
			      int life )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, from, middle, to1, to2, width, color, life ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



