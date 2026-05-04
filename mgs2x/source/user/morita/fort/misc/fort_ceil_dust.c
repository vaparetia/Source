//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_anydust.c
   フォーチュン戦  屑飛び

   2000/12/15 T.Morita
   $Id: fort_ceil_dust.c,v 1.1.1.3 2002/11/19 11:46:17 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"
#include "../../brk_hzd/brk_hazard.h"

#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_BOUNCE       1.18f
#define BRK_BOUNCE_WALL  1.18f
#define BRK_GRAVITY      2.0f


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct move_t
{
    FVECTOR pos   ;
    FVECTOR pos_v ;

    short   rot_x,  rot_y  ;
    short   rot_vx, rot_vy ;

    int          flag   ;
    float        push_up ;
    DG_MDL       *model ;
    DG_COMDL_POS *comdl ;
} MOVE ;

typedef struct work_brk_t
{
    GV_ACT_EX   actor    ;

    FVECTOR     hit_pos  ;
    DG_COMDL   *piece[2] ;
    HZD_BOX    *hzd      ;
    int         where    ;

    int         sound_tic ;
    int         flag     ;

    MOVE       *moves    ;
    int         n_moves  ;
} BrkWork ;


extern void *NewFortFallDust( FVECTOR *pos, int n_dust, float gravity, int latency ) ;

static void RotateMatrixXY( MOVE *p )
{
    int r ;
    r = p->rot_x & 0x0fff ;
    _sceVu0RotMatrixX( &p->comdl->world, &DG_UnitMatrix  , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = p->rot_y & 0x0fff ;
    _sceVu0RotMatrixY( &p->comdl->world, &p->comdl->world, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static void TransMatrix( MOVE *p, int where )
{
    _sceVu0CopyVectorXYZ( (FVECTOR *)p->comdl->world.m[W], &p->pos ) ;
    if ( where )
	BRK_UTL_ComdlColor( p->comdl, 1.0f, where ) ;
    p->comdl->world.m[W][Y] += p->push_up ;
}

/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( HZD_BOX *hzd, MOVE *p, int where )
{
    int flag = 0 ;

    p->pos_v.vy -= BRK_GRAVITY ;
    p->rot_x += p->rot_vx ;
    p->rot_y += p->rot_vy ;
    RotateMatrixXY( p ) ;
    _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    TransMatrix( p, where ) ;

    return flag ;
}

static void Die( BrkWork *work )
{
    if ( work->piece[0] )
	DG_DequeueComdlObjs( work->piece[0] ), DG_FreeComdl( work->piece[0] ) ;
    if ( work->piece[1] )
	DG_DequeueComdlObjs( work->piece[1] ), DG_FreeComdl( work->piece[1] ) ;
    if ( work->moves )
	GV_Free( work->moves ) ;
}

static void Act( BrkWork *work )
{
    int     i ;
    MOVE   *m ;

    if ( work->flag )
    {
	work->flag = 0 ;
	for( i=work->n_moves, m=work->moves ; --i>=0 ; m++ )
	{
	    switch( m->flag )
	    {
	    case 2:
		if ( MoveObject( work->hzd, m, work->where ) == 1 )
		    m->flag = 0 ;
		break ;
	    case 0:
		break ;
	    case 3:
		NewFortFallDust( &m->pos, 2, 1.0f, 0 ) ;
	    default:
		m->flag-- ;
		break ;
	    }
	    work->flag |= m->flag ;
	}
    }


    if ( work->sound_tic++ == 10*5/TIME_BASE )
    {
	static int FRT_CEL_Sound = 0 ;

	/* 埃の音 */
	//printf( "Ciel Down Sound %d\n", (FRT_CEL_Sound&1 ? SD_A_TENPARA2 : SD_A_TENPARA1)  ) ;
	GM_SeSetMode( (FRT_CEL_Sound&1 ? SD_A_TENPARA2 : SD_A_TENPARA1),
		      &work->hit_pos,
		      GM_SEMODE_BOMB ) ;
	FRT_CEL_Sound ^= 1 ;
    }
}

static int InitMovesParam( BrkWork *work, CV2_DEF *cdef, DG_OBJS *objs, DG_MDL *models,
			   DG_COMDL_POS *comdl,  FVECTOR *hit, int n_moves )
{
    FVECTOR v ;
    FVECTOR *verts ;
    int     j, i ;
    int     n_verts ;

    verts = cdef->models->verts ;
    n_verts = irnd() & 7 ;
    for( j=work->n_moves/2 ; --j>=0 ; )
    {
	MOVE *m = &work->moves[n_moves++] ;
	i = irnd() & 1 ;

	/*パラメータの初期値*/
	m->comdl   = &comdl[j] ;
	m->model   = models + i ;
	m->push_up = 3.0f*frnd() ;

	/*位置,速度,回転,回転速度などの設定*/
	m->pos_v.vx = frnd()*0.5f ;
	m->pos_v.vy =        0.0f ;
	m->pos_v.vz = frnd()*0.5f ;

	n_verts += (irnd() & 7) + 1 ;

	_sceVu0ApplyMatrix( &v, &objs->world, &verts[n_verts] ) ;
	_sceVu0AddVector( &m->pos, &v, &objs->trans ) ;
	if ( n_verts >= cdef->models->n_verts )
	    n_verts = 0 ;

	m->rot_vx = (irnd()&0x70)-64 ;
	m->rot_vy = (irnd()&0x38)-32 ;
	m->rot_x =  (irnd()&0x70)-64 ;
	m->rot_y =  (irnd()&0x70)-64 ;
	RotateMatrixXY( m ) ;
	TransMatrix( m, work->where ) ;

	/*着弾から遠いものは遅れて動き出す。*/
	_sceVu0SubVector( &v, hit, &m->pos ) ;
	m->flag = 2 + (int)(sceVu0Sqrt( _sceVu0InnerProduct( &v, &v ) ) / 1000.0f) ;
	m->comdl->color.vw = 128 ;
    }

    return n_moves ;
}

static int GetResources( BrkWork *work, int where, HZD_BOX *hzd,
			 DG_OBJS *objs, int cv2_id, int kms_id,
			 int n_dust, FVECTOR *hit )
{
    int      i ;
    DG_DEF  *def ;
    CV2_DEF *cdef ;
    DG_MDL  *mdl ;
    int      n_moves = 0 ;

    work->where   = where ;
    work->hzd     = hzd   ;
    work->n_moves = n_dust & 0xfffffffe ;
    work->flag    = 1 ;
    work->sound_tic = 0 ;

    _sceVu0CopyVector( &work->hit_pos, hit ) ;

    if ( !(work->moves = GV_Malloc( work->n_moves * sizeof(MOVE) )) )
	PERROR( "Can't allocate memory :: NewIronBoxDust\n" ) ;

    if ( !(cdef = GV_GetCache( GV_CacheID( cv2_id, 'c' ) )) )
	PERROR( "No CV2(%d) found in data.cnf!! :: NewIronBoxDust\n", cv2_id ) ;
    if ( !(def = (DG_DEF *)GV_GetCache( GV_CacheID( kms_id, 'k' ) )) )
	PERROR( "Can't find COMDEL<%d>(Not in data.cnf) :: NewIronBoxDust\n", kms_id ) ;
    for( i=2 ; --i>=0 ; )
    {
	mdl = &def->models[i] ;
	if ( !(work->piece[i] = DG_MakeComdl( mdl->packs, DG_COMDL_SEMITRANS, work->n_moves/2,0 )) )
	    PERROR( "Can't make DG_COMDL(Maybe no memory) :: NewIronBoxDust\n" ) ;
	DG_QueueComdlObjs( work->piece[i] ) ;
	GM_GroupObject( work->piece[i], where ) ;
	n_moves = InitMovesParam( work, cdef, objs, def->models, work->piece[i]->pos, hit, n_moves ) ;
    }

    return 0 ;
}

int NewCeilingDust( DG_OBJS *objs, int where, HZD_BOX *hzd,
		     int cv2_id, int kms_id, int n_dust,
		     FVECTOR *hit )
{
    BrkWork *work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;

    if ( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if( GetResources( work, where, hzd, objs, cv2_id, kms_id, n_dust, hit ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return 0 ;
	}
	return 1 ;
    }
    return 0 ;
}
