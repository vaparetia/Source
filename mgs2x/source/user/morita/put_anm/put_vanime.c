//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_vanime.c
   プット頂点アニメ

   1999/12/13 T. Morita
   $Id: put_vanime.c,v 1.1.1.3 2002/11/19 11:46:29 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct holowork_t
{
    GV_ACT_EX actor ;

    FMATRIX   lights[2] ;
    DG_OBJS  *objs  ;
    VERTEX_ANIME_WORK *anime ;

    int       key   ; /* キーフレーム */
    int       n_key ;
    int       name  ;

    int       base  ; /*ベースカウンタ*/
    int       rand  ; /*乱数幅        */
    int       mode  ;
} Work ;


static void Die( Work *work )
{
    if ( work->anime )
	ExitVertexAnimation( work->anime ) ;
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs );
}

static void Act( Work *work )
{
    GV_MSG *msg ;
    int     i   ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 0:
	    DG_InvisibleObjs( work->objs ) ;
	    break ;
	case 1:
	    DG_VisibleObjs( work->objs ) ;
	    break ;
	}

    if ( work->mode >= 0 )
    {
	SimpleVertexAnimation( work->anime ) ;

	if ( work->anime->count == 0 )
	{
	    work->anime->p[work->key] = 0.0f ;
	    if ( ++work->key >= work->n_key )
		work->key = 0, work->mode *= -1 ;
	    work->anime->p[work->key] = 1.0f ;
	    work->anime->count = work->base + ( irnd() & ( (1 << work->rand) - 1) ) ;
	    work->anime->count = (work->anime->count+1)*5/6 ;
	}
    }
}

static int GetResourcesCalled( Work *work, int id, LIT_DEF *lit,
			       FVECTOR *scale, SVECTOR *rot, FVECTOR *pos,
			       int base, int rand, int mode )
{
    int      i ;
    FMATRIX *mtx ;
    CV2_DEF *cdef ;
    DG_DEF  *def  ;
    int n_models, n_x_models ;

    if ( !id )
	PERROR( "No model ID!!(-m option missing) :: NewPutVanimeObject\n" ) ;
    if ( !(cdef = GV_GetCache( GV_CacheID( id, 'c' ) )) )
	PERROR( "No CV2<%d> found in data.cnf!! :: NewPutVanimeObject\n", id ) ;
    if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No KMS<%d> found in data.cnf!! :: NewPutVanimeObject\n", id ) ;
    n_models   = def->n_models   ;
    n_x_models = def->n_x_models ;
    def->n_models = def->n_x_models = 1 ;
    if ( !(work->objs = DG_MakeObjs( def,
				     (lit ? DG_FLAG_PAINT : DG_FLAG_SHADE)|DG_FLAG_ONEPIECE,
				     0 )) )
	PERROR( "Can't Create DG_OBJS(Maybe no memory)!! :: NewPutVanimeObject\n" ) ;
    DG_QueueObjs( work->objs ) ;
    def->n_models   = n_models   ;
    def->n_x_models = n_x_models ;

    /*オブジェクトのバウンド変更*/
    work->objs->objs->bound_min.vx = def->lx ;
    work->objs->objs->bound_min.vy = def->ly ;
    work->objs->objs->bound_min.vz = def->lz ;
    work->objs->objs->bound_max.vx = def->ux ;
    work->objs->objs->bound_max.vy = def->uy ;
    work->objs->objs->bound_max.vz = def->uz ;

    mtx = &work->objs->world ;
    if ( scale )
    {
	mtx->m[X][X] *= scale->vx ;
	mtx->m[Y][Y] *= scale->vy ;
	mtx->m[Z][Z] *= scale->vz ;
    }
    if ( rot )
    {
	int r ;
	r = rot->vx & 0x0fff ;
	_sceVu0RotMatrixX( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = rot->vy & 0x0fff ;
	_sceVu0RotMatrixY( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = rot->vz & 0x0fff ;
	_sceVu0RotMatrixZ( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    }
    if ( pos )
	_sceVu0CopyVectorXYZ( (FVECTOR *)mtx->m[W], pos ) ;
    else
	PERROR( "No position!! :: NewPutVanimeObject\n" ) ;

    /* ライトの設定 */
    if ( lit )
	DG_MakePreshade( work->objs, lit ) ; /* プリシェード */
    else
    {
	DG_SetLightMatrix( work->objs, work->lights ) ; /* シェード */
	DG_GetLightMatrixFix( (FVECTOR *)&mtx->m[W], work->lights ) ;
    }

    /* フレームコントロール初期化 */
    work->base = base ;
    work->rand = rand ;
    work->mode = mode ;

    /* アニメーション初期化 */
    if ( !(work->anime = InitVertexAnimation( work->objs->objs,
					      cdef->models,
					      DG_VANIME_VERTS|DG_VANIME_NORMS,
					      cdef->n_models ) ) )
	PERROR( "InitVertexAnimation() returns NULL!! :: NewPutVanimeObject\n" ) ;

    for ( i=0 ; i<cdef->n_models ; i++ )
	if ( cdef->models[i].n_verts == cdef->models[0].n_verts )
	    work->anime->key[i] = &cdef->models[i], work->anime->p[i] = 0.0f ;
	else
	    PERROR( "Animation Vertexies are not same!! :: NewPutVanimeObject\n" ) ;
    work->anime->p[0] = 1.0f ;
    work->key   = 0 ;
    work->n_key = cdef->n_models ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    int      id, buf[3] ;
    LIT_DEF *lit = NULL ;
    FVECTOR scale = { 1.0f, 1.0f, 1.0f, 1.0f } ;
    SVECTOR rot   = { 0,0,0,0 } ;
    FVECTOR pos ;

    work->name = name ;
    if ( (id = GCL_GetOptionValue( 'l', 0 )) )
	lit = GV_GetCache( GV_CacheID( id, 'l' ) ) ;

    if ( GCL_GetOption( 's' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &scale ) ;
    }
    if ( GCL_GetOption( 'r' ) != NULL )
        GCL_GetSV( GCL_NextStr(), (short *)&rot ) ;
    if ( GCL_GetOption( 'p' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &pos ) ;
    }
    else
	PERROR( "No position!! :: NewPutVanimeObject\n" ) ;

    if ( GetResourcesCalled( work,
			     GCL_GetOptionValue( 'm', 0 ), lit,
			     &scale, &rot, &pos,
			     GCL_GetOptionValue( 'b', 0 ),
			     GCL_GetOptionValue( 'w', 0 ), 0 ) < 0 )
	return -1 ;
    return 0 ;
}


void *NewPutVanimeObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}


/*

  基本的に DG_OBJS にあるモデルをキーフレームとして頂点アニメさせる。0 
  番目のモデルから表示が開始される。次のフレームまでは,base のフレーム
  数分だけ線形補間が掛かる。

  アニメーションのランダム性を出す場合は randに0以上の数を入れていくと
  フレーム数がそのビット幅分だけベースフレームがランダムに変化する。
	    base + ( irnd() & ( (1 << rand) - 1) )
  したがって,実際の補間時間は,上の式の通りである。

  現在ある mode の種類
  mode==0 :: 0からnフレームまで繰返し再生(nになったら次のフレームを0フレームとして補間して再生)
  mode==1 :: 0からnフレームまで再生しそのまま停止する（キャラが死ぬわけ出なく表示が止まる）

*/
void *NewPutVanimeObjectCalled( int id, int where,
				FVECTOR *scale, SVECTOR *rot, FVECTOR *pos,
				int base, int rand, int mode )
{
    Work    *work ;
    LIT_DEF *lit  ;
    MAP     *map  ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;

	lit = NULL ;
	if ( where )
	    if ( (map = GM_GetMap( where )) )
		lit = map->light ;

        if( GetResourcesCalled( work, id, lit, scale, rot, pos, base, rand, mode ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
