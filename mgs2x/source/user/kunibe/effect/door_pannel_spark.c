//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  door_pannel_spark.c
  電撃床エフェクト呼び出し
  2001/06/01 Yuuta Kunibe	
  $Id: door_pannel_spark.c,v 1.1.1.3 2002/11/19 11:44:38 Yoshizawa1 Exp $
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




#define N_PRIMS			(16)
#define N_VERTS 		(32)
#define	N_SPRTS			( N_PRIMS*N_VERTS )
#define N_HALF 			( N_SPRTS / 2 )

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	SIZE			(10)

#define	COL_R			(32)
#define	COL_G			(64)
#define	COL_B			(255)
#define ALPHA			(128)


#define LIFE			(4)//(30)//(10)//(20)




#define	WAVE_HEIGHT		(70.0f)//(50.0f)//(100.0f)
#define SPEED			(20.0f)//(50.0f)


#define	PANNEL_X		(750.0f)
#define	PANNEL_Z		(350.0f)

#define	PLASMA_NUM		(1)

#define	PLASMA_TEX		( GV_StrCode( "flare0_msk" ) )







typedef struct {
    FVECTOR	height;
    FVECTOR	wave;
    FVECTOR	noise;
    FVECTOR	speed;
} PointParam;


typedef	struct {

    GV_ACT_EX		actor;
    int			name;
    int			map;

    DG_PRIM2		*prim;

    /* ドアパネルパラメータ */
    FMATRIX		mat;
    float		width;
    float		height;

    /* プラズマパラメータ */
    FVECTOR		point[3];
    int			life;    
    int			cnt;
    PointParam		param[N_SPRTS];

    FVECTOR		bound_max;
    FVECTOR		bound_min;
    
    FVECTOR		light_pos;
    int			light_cnt;

    int			alpha;
    int 		sub_alpha;

    FVECTOR		color;

    int			plasma_flag;

    int			se_mode;
    float		se_length;

    int			seed;
    int			total_count;
    
} Work;


// シード固定ランダム関数
static inline int DOOR_IRand( Work *work, int a ) {  
    return ( GM_IRnd( &work->seed ) % a ); 
}
static inline int DOOR_GetRandom( Work *work, int min, int max ) {  
    return ( GM_IRnd( &work->seed ) % ((max - min) + 1) + min ); 
}
static inline float DOOR_FRand( Work *work ) {  
    return ( GM_FRnd( &work->seed ) ); 
}
static inline float DOOR_Rand( Work *work ) {  
    return ( GM_Rnd( &work->seed ) ); 
}


extern void *NewSpritePlasma2( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color, int life );

extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count );



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



static void InitPlasma( Work *work )
{

    //printf("----------------------init plasma\n");

	int		i,j,k,n;
	FVECTOR		*pos;	
	SVECTOR		rot;
	FMATRIX		mat;
	int		rot_flag;

	FVECTOR		center;

	FVECTOR		from_middle;
	FVECTOR		middle_to;

	FVECTOR		vectmp;
	FVECTOR		m_bottom;

	FVECTOR		vec_x1;
	FVECTOR		vec_x2;
	float		length;
	FVECTOR		height;

	float		param[3];
	float		add_param[3];
	int		param_flag[3];


	PointParam	*p_param;
	

	SVECTOR		rot_vec,rot_wide;


	
	work->alpha = ALPHA;
	work->sub_alpha = ALPHA / work->life;

	work->light_cnt = 4;
	



	/* プラズマ生成基準点設定 */
	/* 開始点 */
	work->point[0].vx = work->width;// * (0.90f + frnd()*0.10f);
	work->point[0].vy = frnd()*work->height;
	work->point[0].vz = -10.0f;
	work->point[0].vw = 1.0f;
	
	/* 終了点 */
	work->point[2].vx =-work->width;// * (0.90f + frnd()*0.10f);
	work->point[2].vy = frnd()*work->height;
	work->point[2].vz = -10.0f;
	work->point[2].vw = 1.0f;
	
	/* 中継点 */
	_sceVu0AddVector( &work->point[1], &work->point[0], &work->point[2] );
	_sceVu0ScaleVector( &work->point[1], &work->point[1], 0.50f );
	work->point[1].vz += 120.0f + frnd()*20.0f;
	work->point[1].vw = 1.0f;
	
	DG_SetPos( &work->mat );
	DG_PutVector( work->point, work->point, 3 );
	


	

	/* プラズマ生成パラメータ算出 */
	_sceVu0SubVector( &from_middle, &work->point[1], &work->point[0] );
	_sceVu0SubVector( &vectmp, &work->point[2], &work->point[0] );
	_sceVu0Normalize( &vectmp, &vectmp );
	length = _sceVu0InnerProduct( &from_middle, &vectmp );
	_sceVu0ScaleVector( &m_bottom, &vectmp, length );

	_sceVu0SubVector( &height, &from_middle, &m_bottom );

	_sceVu0ScaleVector( &vec_x1, &m_bottom, 1.0f / (float)N_HALF );

	_sceVu0AddVector( &m_bottom, &m_bottom, &work->point[0] );

	_sceVu0SubVector( &middle_to, &work->point[2], &work->point[1] );
	length = _sceVu0InnerProduct( &middle_to, &vectmp );
	_sceVu0ScaleVector( &vec_x2, &vectmp, length );
	_sceVu0ScaleVector( &vec_x2, &vec_x2, 1.0f / (float)N_HALF );




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



	pos = work->prim->pos[work->prim->buffer_clock];
	p_param = work->param;


	for ( i = 0 ; i < 4 ; i++ ) {

	    for ( j = 0 ; j < N_SPRTS / 4 ; j++ ) {

		n = i * N_SPRTS/4 + j;

		DG_COPY_VEC( pos, &DG_ZeroVector );

		if ( !(j%16) ) {

		    if ( rot_flag ) {
			rot.vz = irnd()%80 + 2048/32;
		    }
		    else {
			rot.vz = -irnd()%80 - 2048/32;
		    }
		    
		    rot.vz *= 1.f / 32.f;
			

		    if ( n < N_SPRTS*3/4 ) {

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


		/* 中心点 */
		/* 開始点 */
		if ( n == 0 ) {					
		    DG_COPY_VEC( &center, &work->point[0] );
		    DG_COPY_VEC( &p_param->height, &DG_ZeroVector );
		    _sceVu0ScaleVector( &p_param->wave, (FVECTOR *)mat.m[0], WAVE_HEIGHT );
		    DG_COPY_VEC( &p_param->noise, &DG_ZeroVector );
		    DG_COPY_VEC( &p_param->speed, &DG_ZeroVector );
		}
		else {
		    if ( n < N_HALF ) {
			_sceVu0AddVector( &center, &center, &vec_x1 );
			_sceVu0ScaleVector( &p_param->height,
					    &height, (float)( N_HALF*N_HALF - (N_HALF-n)*(N_HALF-n) ) / (float)(N_HALF*N_HALF) );
			_sceVu0ScaleVector( &p_param->wave, (FVECTOR *)mat.m[0], WAVE_HEIGHT );

		    }
		    else {
			_sceVu0AddVector( &center, &center, &vec_x2 );
			_sceVu0ScaleVector( &p_param->height,
					    &height, (float)( N_HALF*N_HALF - (n-N_HALF)*(n-N_HALF) ) / (float)(N_HALF*N_HALF) );
			_sceVu0ScaleVector( &p_param->wave, (FVECTOR *)mat.m[0], WAVE_HEIGHT );
			    
		    }
		}

		p_param->speed.vx = SPEED * param[0];
		p_param->speed.vy = SPEED * param[1];
		p_param->speed.vz = SPEED * param[2];
		
		param[0] += add_param[0];
		param[1] += add_param[1];
		param[2] += add_param[2];
		
		_sceVu0ScaleVector( &p_param->noise, &p_param->speed, 17.5f );
		
		p_param++;
		pos++;

	    }
	    
	}	


	/* 不定期に火花飛ぶ */
	switch ( DOOR_IRand( work, 5 ) ) {
	case 0:
	    DG_COPY_MAT( &mat, &work->mat );
	    DG_COPY_VEC( (FVECTOR *)mat.m[3], &work->point[2] );
	    rot_vec.vx =-512;
	    rot_vec.vy =-512;
	    rot_vec.vz = 1;
	    rot_wide.vx = 1024;
	    rot_wide.vy = 1024;
	    rot_wide.vz = 1;
	    NewSpark2( 20,
		       &work->mat,
		       1.0F,
		       100.0F + 25.0F * frnd(),
		       10.0F,
		       &rot_vec, &rot_wide,
		       &work->color,
		       1.0F,
		       15 );

	    if ( work->se_mode == 0 ) {
		GM_SeSetMode( SD_E_HIBANA02, &work->point[2], GM_SEMODE_NORMAL );
	    }
	    else {
		_sceVu0SubVector( &vectmp, &work->point[2], &GM_PlayerPosition );
		if ( _sceVu0InnerProduct( &vectmp, &vectmp ) < work->se_length * work->se_length ) {
		    GM_SeSetMode( SD_E_HIBANA02, &work->point[2], GM_SEMODE_NORMAL );
		}
	    }
	    break;
	case 1:
	    DG_COPY_MAT( &mat, &work->mat );
	    DG_COPY_VEC( (FVECTOR *)mat.m[3], &work->point[0] );
	    rot_vec.vx =-512;
	    rot_vec.vy =-512;
	    rot_vec.vz = 1;
	    rot_wide.vx = 1024;
	    rot_wide.vy = 1024;
	    rot_wide.vz = 1;
	    NewSpark2( 20,
		       &mat,
		       1.0F,
		       125.0F + 25.0F * frnd(),
		       10.0F,
		       &rot_vec, &rot_wide,
		       &work->color,
		       1.0F,
		       15 );
	    if ( work->se_mode == 0 ) {
		GM_SeSetMode( SD_E_HIBANA02, &work->point[0], GM_SEMODE_NORMAL );
	    }
	    else {
		_sceVu0SubVector( &vectmp, &work->point[0], &GM_PlayerPosition );
		if ( _sceVu0InnerProduct( &vectmp, &vectmp ) < work->se_length * work->se_length ) {
		    GM_SeSetMode( SD_E_HIBANA02, &work->point[0], GM_SEMODE_NORMAL );
		}
	    }
	    break;
	default:
	    break;
	}

}



static void ActPlasma( Work *work )
{

    //printf("act plasma\n");


    int 		i,j;
    int	  		clock;
    FVECTOR		*pos;
    DG_PRIM2_UVRGBWH	*uvrgbwh;
    FVECTOR		vectmp;



    /* 追従用 */
    FVECTOR		center;

    FVECTOR		from_middle;
    FVECTOR		middle_to;

    FVECTOR		m_bottom;

    FVECTOR		vec_x1;
    FVECTOR		vec_x2;
    float		length;
    FVECTOR		height;

    PointParam		*p_param;



    /* 操作する頂点バッファ取得 */
    clock = work->prim->buffer_clock;
    pos	    = work->prim->pos[clock];
    uvrgbwh = work->prim->uvrgb[clock];

    p_param = work->param;



    /* 追従用 */
    _sceVu0SubVector( &from_middle, &work->point[1], &work->point[0] );
    _sceVu0SubVector( &vectmp, &work->point[2], &work->point[0] );
    _sceVu0Normalize( &vectmp, &vectmp );
    length = _sceVu0InnerProduct( &from_middle, &vectmp );
    _sceVu0ScaleVector( &m_bottom, &vectmp, length );

    _sceVu0SubVector( &height, &from_middle, &m_bottom );
    _sceVu0ScaleVector( &vec_x1, &m_bottom, 1.0f / (float)N_HALF );

    _sceVu0AddVector( &m_bottom, &m_bottom, &work->point[0] );

    _sceVu0SubVector( &middle_to, &work->point[2], &work->point[1] );
    length = _sceVu0InnerProduct( &middle_to, &vectmp );
    _sceVu0ScaleVector( &vec_x2, &vectmp, length );
    _sceVu0ScaleVector( &vec_x2, &vec_x2, 1.0f / (float)N_HALF );

	

    for ( i = 0 ; i < 2 ; i++ ) {

	pos = SCR_POS;
	
	for ( j = 0 ; j < N_SPRTS/2 ; j++ ) {

	    if ( i == 0 && j == 0 ) {

		DG_COPY_VEC( &center, &work->point[0] );
		_sceVu0AddVector( pos, &work->point[0], &p_param->height );
		_sceVu0AddVector( pos, pos, &p_param->wave );

	    }
	    else {

		/* 追従用 */
		if ( i < 2 ) {
		    _sceVu0AddVector( &center, &center, &vec_x1 );
		}
		else {
		    _sceVu0AddVector( &center, &center, &vec_x2 );
		}


		/* プラズマ点を算出 */
#if 1			
		AddVector4( pos, &center, &p_param->height, &p_param->noise, &p_param->wave );
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
		}
		else {
		    _sceVu0ScaleVector( &p_param->height, &p_param->height, 0.99f );
		    _sceVu0ScaleVector( &p_param->wave, &p_param->wave, 0.98f );
		}
		_sceVu0ScaleVector( &p_param->speed, &p_param->speed, 0.97f );

	    }

	    uvrgbwh->a = work->alpha;

	    pos++;
	    uvrgbwh++;

	    p_param++;

	}

	OK_Scr_Mem( &work->prim->pos[clock][i*N_SPRTS/2], SCR_POS, sizeof(FVECTOR), N_SPRTS/2 );

    }


    /* アルファ更新 */
    work->alpha -= work->sub_alpha;



}


/* アクト関数 */
static void Act( Work *work )
{


    int 	flags;
    FVECTOR	vectmp;
    

    DG_SwitchBuffPrim2( work->prim );    

#if 0    
    NewBoundingBoxView( &work->bound_max, &work->bound_min, 128, 32, 32 );
#endif

    
    if ( --work->cnt == 0 ) {

	/* 寿命設定 */	
	work->life = DIRECT_TICK( 4+DOOR_IRand( work, 10 ) );
	work->cnt  = work->life + DIRECT_TICK( DOOR_IRand( work, 90 ) );

	flags = DG_BoundCheck( &DG_UnitMatrix, &work->bound_max, &work->bound_min );
	flags = flags & DG_PRIM2_INVISIBLE0;

	if ( !flags ) {
	    /* プラズマ初期化 */
	    work->plasma_flag = 1;
	    DG_VisiblePrim2( work->prim );
	    InitPlasma( work );
	    if ( work->se_mode == 0 ) {
		GM_SeSetMode( SD_A_SPARK01, &work->point[1], GM_SEMODE_NORMAL );
	    }
	    else {
		_sceVu0SubVector( &vectmp, &work->point[1], &GM_PlayerPosition );
		if ( _sceVu0InnerProduct( &vectmp, &vectmp ) < work->se_length * work->se_length ) {
		    GM_SeSetMode( SD_A_SPARK01, &work->point[1], GM_SEMODE_NORMAL );
		}
	    }
	}
	else {
	    /* 音のみ呼び出し */
	    if ( work->se_mode == 0 ) {
		GM_SeSetMode( SD_A_SPARK01, (FVECTOR *)work->mat.m[3], GM_SEMODE_NORMAL );
		if (irnd()%3) {
		    GM_SeSetMode( SD_E_HIBANA02, (FVECTOR *)work->mat.m[3], GM_SEMODE_NORMAL );
		}		
	    }
	    else {
		_sceVu0SubVector( &vectmp, (FVECTOR *)work->mat.m[3], &GM_PlayerPosition );
		if ( _sceVu0InnerProduct( &vectmp, &vectmp ) < work->se_length * work->se_length ) {
		    GM_SeSetMode( SD_A_SPARK01, (FVECTOR *)work->mat.m[3], GM_SEMODE_NORMAL );
		    if (irnd()%3) {
			GM_SeSetMode( SD_E_HIBANA02, (FVECTOR *)work->mat.m[3], GM_SEMODE_NORMAL );
		    }		
		}
	    }
	}
	    
    }
    else {
	if ( work->life > 0 && work->plasma_flag == 1 ) {
	    //DG_VisiblePrim2( work->prim );
	    ActPlasma( work );	
	    work->life--;
	}
	else {
	    work->plasma_flag = 0;
	    DG_InvisiblePrim2(work->prim);
	}
    }

    if ( work->light_cnt > 0 ) {
	DG_SetTmpLight2 (
			 &work->point[1],
			 500.0f  + 100.f * frnd(),
			 1000.0f + 200.f * frnd(),
			 (int)(work->color.vx) | (int)(work->color.vy)<<8 | (int)(work->color.vz)<<16,
			 LIT_FLAG_BGONLY );			
	work->light_cnt--;
    }


    work->total_count++;
    
}


static void Die(Work *work )
{
    work->prim = OK_FreePrim2( work->prim );
}











/* プリミティブ初期化関数 */
static int InitPacket( Work *work )
{

	int			i,j;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;	
	DG_PRIM2_UVRGBWH	*uvrgbwh;




	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT | DG_PRIM2_ALPHA | DG_PRIM2_SHADE | DG_PRIM2_TEX,
					  N_PRIMS, N_VERTS );

	tex = DG_GetTexture( PLASMA_TEX );	

	prim->raise = 0;
	
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	
	for ( i = 0 ; i < 4 ; i++ ) {

	    pos   	= SCR_POS;
	    uvrgbwh	= SCR_UVS;	
	
	    for ( j = 0 ; j < N_SPRTS/4 ; j++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );

		uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->q0 = 4096;
		uvrgbwh->f0 = 0x0fff;

		uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		uvrgbwh->q1 = 4096;
		uvrgbwh->f1 = 0x0fff;

		uvrgbwh->w = SIZE;
		uvrgbwh->h = SIZE;

		uvrgbwh->r = COL_R;
		uvrgbwh->g = COL_G;
		uvrgbwh->b = COL_B;
		
		uvrgbwh->a = 0;

		pos++;
		uvrgbwh++;

	    }

	
	    OK_Scr_Mem( &prim->pos[ 0 ][ N_SPRTS/4 * i ], SCR_POS, sizeof(FVECTOR), N_SPRTS/4 );
	    OK_Scr_Mem( &prim->pos[ 1 ][ N_SPRTS/4 * i ], SCR_POS, sizeof(FVECTOR), N_SPRTS/4 );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH *)( prim->uvrgb[ 0 ] ) )[ N_SPRTS/4 * i ], SCR_UVS,
			sizeof(DG_PRIM2_UVRGBWH), N_SPRTS/4 );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH *)( prim->uvrgb[ 1 ] ) )[ N_SPRTS/4 * i ], SCR_UVS,
			sizeof(DG_PRIM2_UVRGBWH), N_SPRTS/4 );


	}


	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;


}

















static void InitWork( Work *work )
{

    FVECTOR		pos;
    SVECTOR		rot;


    /* 中心位置取得 */
    if( GCL_GetOption( 'p' ) != NULL ) {
	pos.vx = (float)GCL_GetNextInt();
	pos.vy = (float)GCL_GetNextInt();
	pos.vz = (float)GCL_GetNextInt();
	pos.vw = 1.0f;
    }
    else {
	ASSERT( 0 );
    }

    /* 回転角度取得 */
    if( GCL_GetOption( 'r' ) != NULL ) {
	rot.vx = (short)GCL_GetNextInt();
	rot.vy = (short)GCL_GetNextInt();
	rot.vz = (short)GCL_GetNextInt();
    }
    else {
	ASSERT( 0 );
    }

    /* 幅取得 */
    if( GCL_GetOption( 'w' ) != NULL ) {
	work->width = (float)GCL_GetNextInt();
	work->width *= 0.50f;
    }
    else {
	ASSERT( 0 );
    }

    /* 高さ取得 */
    if( GCL_GetOption( 'h' ) != NULL ) {
	work->height = (float)GCL_GetNextInt();
	work->height *= 0.50f;
    }
    else {
	ASSERT( 0 );
    }

    if ( GCL_GetOption( 'm' ) != NULL ) {
	work->se_mode = GCL_GetNextInt();

	if ( work->se_mode == 1 ) {
	    if ( GCL_GetOption( 'l' ) != NULL ) {
		work->se_length = (float)GCL_GetNextInt();
	    }
	    else {
		ASSERT(0);		
	    }
	}
    }
    else {
	work->se_mode = 0;
    }
    
    /* パネルマトリクス生成 */
    DG_SetPos2( &pos, &rot );
    DG_GetPos( &work->mat );



    /* バウンディングボックス生成 */
    work->bound_max.vx = 500.0f;
    work->bound_max.vy = 1000.0f;
    work->bound_max.vz = 1000.0f;
    work->bound_max.vw = 1.0f;
    DG_SetPos( &work->mat );
    DG_PutVector( &work->bound_max, &work->bound_max, 1 );

    work->bound_min.vx =-500.0f;
    work->bound_min.vy =-1000.0f;
    work->bound_min.vz =-500.0f;
    work->bound_min.vw = 1.0f;
    DG_SetPos( &work->mat );
    DG_PutVector( &work->bound_min, &work->bound_min, 1 );


    /* 火花用色ベクトル */
    work->color.vx = 32.0f;
    work->color.vy = 100.0f;
    work->color.vz = 190.0f;
    work->color.vw = 64.0f;

    /* カウンタ初期化 */
    work->cnt = 1;
    work->light_cnt = 0;

    /* プラズマ発生フラグ初期化 */
    work->plasma_flag = 0;

    work->seed = 12432431; 

    work->total_count = 0;
    InitPacket( work );
    
    
}





/*-------- void *NewDoorPannelSpark( int name, int map ) ----------
                   ドアパネル壊れ電撃エフェクト
-----------------------------------------------------------------*/		   
void *NewDoorPannelSpark( int name, int map )
{
    
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
	    GV_SetActor( &( work->actor ), Act, Die );
	    GV_ActorEX( &work->actor );
	    work->name = name;
	    work->map  = map;
	    InitWork( work );
	}

	return (void *)work ;

}




