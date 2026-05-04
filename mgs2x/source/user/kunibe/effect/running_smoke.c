//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    runnning_smoke.c
    走る煙
    2001/03/23 Yuuta Kunibe	
    $Id: running_smoke.c,v 1.4 2002/11/23 12:28:39 Yoshizawa1 Exp $
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


#define N_PRIMS	(160)

#define COLOR	(128)
#define ALPHA	(48)

//#define E3_VERSION

#ifdef E3_VERSION
#define SIZE	(500.0f)
#define MOVE_Z	(0.10f)
#else
#define SIZE	(250.0f)
#define MOVE_Z	(0.01f)
#endif

#define	LIFE	(180)//(360)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define SCR_PRE_POS ((void *)SCR_POS + sizeof(FVECTOR)*N_PRIMS)
#define SCR_PRE_UVS ((void *)SCR_PRE_POS + sizeof(FVECTOR)*N_PRIMS)
#define SCR_NOW_UVS ((void *)SCR_PRE_UVS + sizeof(DG_PRIM2_UVRGBWH)*N_PRIMS)

#define WIND_MAX (20.0f)


/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	vecadd;
    FVECTOR	vecBullet;

    FVECTOR	pos[5];
    FVECTOR	vec[5];
    float	alpha[5];
    float	size[5];

    FVECTOR	wind;

    int  	id;
    int 	life;
    int		count;

} Work;



/* アクト関数 */
static void Act( Work *work )
{

    int			i,j;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    float  		mov_z;
    float		intense;
    FVECTOR		vectmp;
    FVECTOR		local_wind;
    int			invisible_num;


    invisible_num = 0;
    
    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];

    /* 風計算 */
    intense = WIND_MAX / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;
	

    /*----------------*/
    /* スプライト更新 */
    /*----------------*/
    for ( i = 0 ; i < work->id ; i++ ) {

	mov_z = MOVE_Z * (float)( i / 5 ) * (float)( i / 5 );
	DG_COPY_VEC( &vectmp, &work->vecadd );
	_sceVu0ScaleVector( &vectmp, &work->vecadd, mov_z );
		
	_sceVu0AddVector( &pos[i], &pos_pre[i], &vectmp );
	
	mov_z = (float)i / 5.f / 40.f;// * ( 1.0f + 0.15f * frnd() );	/*check!*/
	_sceVu0ScaleVector( &vectmp, &work->wind, mov_z );	
	_sceVu0AddVector( &pos[i], &pos[i], &vectmp );

	/* サイズ更新 */
	if ( work->id >= N_PRIMS ) {
	    uvrgbwh[i].w = (short)(uvrgbwh_pre[i].w * ( 1.005f + 0.005f * (float)(i/10) / 20.f ));/*1.004f*/
	    uvrgbwh[i].h = (short)(uvrgbwh_pre[i].h * ( 1.005f + 0.005f * (float)(i/10) / 20.f ));
	}
	else {
	    uvrgbwh[i].w = (short)(uvrgbwh_pre[i].w * ( 1.f + 0.15f * (float)( 40 - (i/5) ) / 40.f ));
	    uvrgbwh[i].h = (short)(uvrgbwh_pre[i].h * ( 1.f + 0.15f * (float)( 40 - (i/5) ) / 40.f ));
	}

	if ( work->id >= N_PRIMS ) {
	    if ( ( i % 5 ) == 2 || ( i % 5 ) == 4 ) {	/* 本線 */
		if ( uvrgbwh_pre[i].a > 0 ) {
		    if ( !work->count ) {
			uvrgbwh[i].a = uvrgbwh_pre[i].a - 1;
		    }
		    else {
			uvrgbwh[i].a = uvrgbwh_pre[i].a;
		    }
		}
		else {
		    uvrgbwh[i].w = 0;
		    uvrgbwh[i].h = 0;
		    uvrgbwh[i].a = 0;
		    invisible_num++;
		}
	    }
	    else {					/* サブ線 */
		if ( uvrgbwh_pre[i].a > 0 ) {
		    //if ( !(work->count%2) )
			uvrgbwh[i].a = uvrgbwh_pre[i].a - 1;
		    //else uvrgbwh[i].a = uvrgbwh_pre[i].a;
		}
		else {
		    uvrgbwh[i].w = 0;
		    uvrgbwh[i].h = 0;
		    uvrgbwh[i].a = 0;
		    invisible_num++;
		}
	    }
	}
	else {
	    uvrgbwh[i].a = uvrgbwh_pre[i].a;
	}
    }

    if ( ++work->count >= 3 ) work->count = 0;
    
    if ( work->id < N_PRIMS ) {

	for ( j = 0 ; j < 4 ; j++ ) {
	    for ( i = 0 ; i < 5 ; i++ ) {

		_sceVu0AddVector( &work->pos[i], &work->pos[i], &work->vec[i] );

		/* 位置にランダム性持たせる */				/*check!*/
		/*vectmp.vx = 0.f;//(float)( irnd()%50 ) - 25.0f;
		vectmp.vy = 0.f;//(float)( irnd()%50 ) - 25.0f;
		vectmp.vz = 0.f;//(float)( irnd()%50 ) - 25.0f;
		vectmp.vw = 0.f;
		_sceVu0AddVector( &work->pos[i], &work->pos[i], &vectmp );*/
		
		_sceVu0AddVector( &work->vec[i], &work->vec[i], &work->vecBullet );
		work->vec[i].vy -= 4.f;

		DG_COPY_VEC( &pos[work->id], &work->pos[i] );
		
		uvrgbwh[work->id].a = work->alpha[i];// + irnd()%10;	/*check!*/
		
		/* ID更新 */
		work->id++;
	    }

	    //if ( !(irnd()%2) ) {
		if ( work->alpha[i] > 0 ) {
		    work->alpha[i]--;
		}
	    //}

	}

    }


    //if ( --work->life < 0 || invisible_num >= N_PRIMS ) {
    if ( invisible_num >= N_PRIMS ) {
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	float 			ftmp;
	float			angle;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    if ( i >= 5 && i < 10 ) {	/* 爆心反映 */
		uvrgbwh->r  = 128;
		uvrgbwh->g  = 58 + irnd()%16;//64;
		uvrgbwh->b  = 0;
		uvrgbwh->a  = 0;

	    ftmp  = work->size[i%5] * 4.2f * ( 1.0f + frnd() * 0.2f );
	    angle = TPI * frnd();
	    uvrgbwh->w  = ftmp * vu0_Cos( angle );
	    uvrgbwh->h  = ftmp * vu0_Sin( angle );

	    }
	    else {
		ftmp = COLOR * ( 0.5f * frnd() + 1.f );/**/
		uvrgbwh->r  = ftmp;
		uvrgbwh->g  = ftmp;
		uvrgbwh->b  = ftmp;
		uvrgbwh->a  = 0;

	    ftmp  = work->size[i%5] * ( 1.20f + frnd() * 0.70f ) * ( 0.5f + ( 52.f - (float)(i/5) ) / 52.f * 0.5f );/**/
	    angle = TPI * frnd();
	    uvrgbwh->w  = ftmp * vu0_Cos( angle );
	    uvrgbwh->h  = ftmp * vu0_Sin( angle );

	    }

	    pos++;
	    uvrgbwh++;
	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}

static int GetTargetData( Work *work, FVECTOR* hit_pos, FVECTOR* n_vector, FVECTOR* bullet_vec )
{

    int 	i;
    FVECTOR	vectmp;
    FVECTOR	n_vec;
    FVECTOR	vecline;
    FVECTOR	unit_vec;
    SVECTOR	rot;
    FMATRIX	mat;
    FMATRIX	matInv;
    float	inner;
    float	inner2;
    float 	fcos,fsin;
    float	ftmp;
    float	angle;


    /* 弾ベクトル(vecBullet)取得 debug */
    DG_COPY_VEC( &work->vecBullet, bullet_vec );

    /* 当たり面法線(n_vec)取得 */
    DG_COPY_VEC( &n_vec, n_vector );    
    _sceVu0Normalize( &n_vec, &n_vec );    		/* 念のため正規化 */

    /* 当たり面マトリクス(mat)生成 */
    rot.vx = (short)( 2048.0f * asinf( -(n_vec.vy) ) / PI );
    rot.vy = (short)( 2048.0f * atan2f( n_vec.vx, n_vec.vz ) / PI );
    rot.vz = 0;
    DG_SetPos( &DG_UnitMatrix );
    DG_SetPos2( &DG_ZeroVector, &rot );
    DG_GetPos( &mat );
#if 0    
    printf("%f %f %f\n", mat.m[0][0], mat.m[0][1], mat.m[0][2]);
    printf("%f %f %f\n", mat.m[1][0], mat.m[1][1], mat.m[1][2]);
    printf("%f %f %f\n", mat.m[2][0], mat.m[2][1], mat.m[2][2]);
#endif    

    /* 当たり平面に沿ったベクトルを飛ばす基本方向として抽出(vecline) */
    inner = _sceVu0InnerProduct( &work->vecBullet, &n_vec );
    DG_COPY_VEC( &vectmp, &work->vecBullet );
    _sceVu0ScaleVector( &vectmp, &n_vec, inner );
    _sceVu0SubVector( &vecline, &work->vecBullet, &vectmp );
    _sceVu0Normalize( &vecline, &vecline );
#if 0    
    printf( "vecline : %f, %f, %f\n", vecline.vx, vecline.vy, vecline.vz );
#endif
    
    /* 本体マトリクスに対して相対化 -> vectmp */
    FastInverseMatrix( &matInv, &mat );
    matInv.m[3][0] = 0.f;
    matInv.m[3][1] = 0.f;
    matInv.m[3][2] = 0.f;
    _sceVu0ApplyMatrix( &vecline, &matInv, &vecline );
#if 0    
    printf( "vectmp : %f, %f, %f\n", vectmp.vx, vectmp.vy, vectmp.vz );
#endif    
    

    /* 当たり面法線との内積 */
    _sceVu0Normalize( &work->vecBullet, &work->vecBullet );
    inner2 = _sceVu0InnerProduct( &work->vecBullet, &n_vec );
    inner2 = inner2*inner2;


    if ( inner2 > 0.8f ) {	/* 正面からの当たり : 内積値0.8以上は正面と判定 */
	inner2 = 1.0f;	
	unit_vec.vx = 1.f;
	unit_vec.vy = 0.f;
	unit_vec.vz = 0.f;
	unit_vec.vw = 1.f;
    }
    else {			/* それ以外の当たり */
	DG_COPY_VEC( &unit_vec, &vecline );
    }    

    for ( i = 0 ; i < 5 ; i++ ) {

	/* 各ラインの初速度算出 */
	//ftmp  = 60.f + frnd() * 40.f;
	ftmp  = 180.f + frnd() * 60.f;
	angle = TPI * inner2 * ( frnd() * 0.5f );
	if ( angle < 0.f ) {
	    angle += TPI;
	}
	fcos  = vu0_Cos( angle );
	fsin  = vu0_Sin( angle );

	work->vec[i].vx = unit_vec.vx * fcos - unit_vec.vy * fsin;
	work->vec[i].vy = unit_vec.vx * fsin + unit_vec.vy * fcos;
	work->vec[i].vz = 0.f;
	work->vec[i].vw = 1.f;

	_sceVu0ScaleVector( &work->vec[i], &work->vec[i], ftmp );
	_sceVu0ApplyMatrix( &work->vec[i], &mat, &work->vec[i] );

	/* 各ラインの出発点を当たり位置に初期化 */
	DG_COPY_VEC( &work->pos[i], hit_pos );

	/* 各ラインのスプライト情報設定 */
	if ( i == 2 || i == 4 ) {	/* 本線 */
	    work->alpha[i] = ALPHA;
	    work->size[i]  = SIZE;
	}
	else {				/* サブ線 */
	    work->alpha[i] = (int)( (float)ALPHA * ( 0.8f + frnd() * 0.2f ) );
	    work->size[i]  = SIZE * ( 0.7f + frnd() * 0.3f );
	}

    }


    /* 流れる方向ベクトル取得 */
    DG_COPY_VEC( &work->vecadd, &work->vecBullet );
    _sceVu0ScaleVector( &work->vecBullet, &work->vecBullet, 4.f );
#if 0    
    printf("inner2 %f\n",inner2);
#endif
    
    return 0;

}


static int GetResources( Work *work, FVECTOR* hit_pos, FVECTOR* n_vector, FVECTOR* bullet_vec )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->life  = LIFE;
    work->count = 0;

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );

    work->id = 0;
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 10, 16 );
    if ( prim == NULL ) {
	return -1;
    }

    GetTargetData( work, hit_pos, n_vector, bullet_vec );
    
    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}

/*
  NewRunningSmoke
      FVECTOR* hit_pos    : 当たった座標
      FVECTOR* n_vector   : 当たった面の法線ベクトル
      FVECTOR* bullet_vec : 弾ベクトル
*/
void *NewRunningSmoke( FVECTOR* hit_pos, FVECTOR* n_vector, FVECTOR* bullet_vec )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, hit_pos, n_vector, bullet_vec ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


