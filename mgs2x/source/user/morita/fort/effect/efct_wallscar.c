//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_wallscar.c
  フォーチュン戦 専用弾痕

  2001/02/08 T.Morita
  $Id: efct_wallscar.c,v 1.1.1.3 2002/11/19 11:46:11 Yoshizawa1 Exp $
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

#define WLLSCR_MAX_UNIT (10)
#define WLLSCR_MAX_DIFF (5)
#define WLLSCR_SIZE 50

#define PUSH_UP 5.0f
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"


typedef struct work_t Work ;
struct work_t
{
    GV_ACT    actor  ;

    int       name   ;
    int       flag   ; /* 制御フラグ */

    short     n_scar ;
    DG_PRIM2 *scar   ;
    short     n_pos  ;
    short     front  ;
} ;


extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */
extern void *NewFortWallScarBase( int name, int where ) ;

static Work *FortWallScarBase_Work = NULL ;

static FVECTOR for_flr[4] = {
    { WLLSCR_SIZE, 0.0f, WLLSCR_SIZE, 1.0f },
    {-WLLSCR_SIZE, 0.0f, WLLSCR_SIZE, 1.0f },
    { WLLSCR_SIZE, 0.0f,-WLLSCR_SIZE, 1.0f },
    {-WLLSCR_SIZE, 0.0f,-WLLSCR_SIZE, 1.0f },
} ;
static FVECTOR inv_flr[4] = {
    { WLLSCR_SIZE, 0.0f, WLLSCR_SIZE, 1.0f },
    { WLLSCR_SIZE, 0.0f,-WLLSCR_SIZE, 1.0f },
    {-WLLSCR_SIZE, 0.0f, WLLSCR_SIZE, 1.0f },
    {-WLLSCR_SIZE, 0.0f,-WLLSCR_SIZE, 1.0f },
} ;

static FVECTOR for_seg[4] = {
    { WLLSCR_SIZE,  WLLSCR_SIZE, 0.0f, 1.0f },
    {-WLLSCR_SIZE,  WLLSCR_SIZE, 0.0f, 1.0f },
    { WLLSCR_SIZE, -WLLSCR_SIZE, 0.0f, 1.0f },
    {-WLLSCR_SIZE, -WLLSCR_SIZE, 0.0f, 1.0f },
} ;
static FVECTOR inv_seg[4] = {
    { WLLSCR_SIZE,  WLLSCR_SIZE, 0.0f, 1.0f },
    { WLLSCR_SIZE, -WLLSCR_SIZE, 0.0f, 1.0f },
    {-WLLSCR_SIZE,  WLLSCR_SIZE, 0.0f, 1.0f },
    {-WLLSCR_SIZE, -WLLSCR_SIZE, 0.0f, 1.0f },
} ;

static void CalcFloorMatrix( HZX_FLR *flr, FMATRIX *dir, FVECTOR *v )
{
    FMATRIX mx ;
    FVECTOR nrm ;
    float d  ;
    FVECTOR  min, max ;
    float dx, dz ;

    min.vx = MIN( MIN( flr->p1.x, flr->p2.x), MIN( flr->p3.x, flr->p4.x) ) ;/*as not ALIGN16*/
    min.vy = MIN( MIN( flr->p1.y, flr->p2.y), MIN( flr->p3.y, flr->p4.y) ) ;
    min.vz = MIN( MIN( flr->p1.z, flr->p2.z), MIN( flr->p3.z, flr->p4.z) ) ;
    max.vx = MAX( MAX( flr->p1.x, flr->p2.x), MAX( flr->p3.x, flr->p4.x) ) ;
    max.vy = MAX( MAX( flr->p1.y, flr->p2.y), MAX( flr->p3.y, flr->p4.y) ) ;
    max.vz = MAX( MAX( flr->p1.z, flr->p2.z), MAX( flr->p3.z, flr->p4.z) ) ;

    _sceVu0Normalize( &nrm, &nrm ) ;
    mx = DG_UnitMatrix ;
    mx.m[X][Y] = nrm.vx = flr->p1.h/32000.0F ;
    mx.m[Y][Y] = nrm.vy = flr->p3.h/32000.0F ;
    mx.m[Z][Y] = nrm.vz = flr->p2.h/32000.0F ;
    nrm.vw = 0.0f ;
    if ( (dx = sceVu0Sqrt(nrm.vx*nrm.vx + nrm.vy*nrm.vy)) > 0.001f )
	mx.m[Y][X] = nrm.vx / dx, mx.m[X][X] = -nrm.vy / dx ;
    if ( (dz = sceVu0Sqrt(nrm.vz*nrm.vz + nrm.vy*nrm.vy)) > 0.001f )
	mx.m[Y][Z] = nrm.vz / dz, mx.m[Z][Z] = -nrm.vy / dz ;

    d = _sceVu0InnerProduct( (FVECTOR*)dir->m[Y], &nrm ) ;
    _sceVu0ScaleVector( &nrm, &nrm, d<0 ? -PUSH_UP : PUSH_UP ) ;
    _sceVu0AddVector( (FVECTOR*)mx.m[W], (FVECTOR*)dir->m[W], &nrm ) ;

    DG_SetPos( &mx ) ;
    DG_PutVector( d<0 ? for_flr : inv_flr, v, 4 ) ;
    MinMaxVector4( &min, &max, v ) ;
}

static void CalcSegMatrix( HZX_SEG *seg, FMATRIX *dir, FVECTOR *v )
{
    FMATRIX mx ;
    FVECTOR nrm = { seg->p1.z-seg->p2.z, 0.0f, -seg->p1.x+seg->p2.x, 0.0f } ;
    float d ;
    FVECTOR  min, max ;

    min.vx = MIN( seg->p1.x, seg->p2.x ) ;/*as not ALIGN16*/
    min.vy = MIN( seg->p1.y, seg->p2.y ) ;
    min.vz = MIN( seg->p1.z, seg->p2.z ) ;
    max.vx = MAX( seg->p1.x, seg->p2.x ) ;
    max.vy = MAX( seg->p1.y+seg->p1.h, seg->p2.y+seg->p2.h ) ;
    max.vz = MAX( seg->p1.z, seg->p2.z ) ;

    _sceVu0Normalize( &nrm, &nrm ) ;
    mx = DG_UnitMatrix ;
    mx.m[X][X] = nrm.vz ; mx.m[X][Z] = -nrm.vx ;
    mx.m[Z][X] = nrm.vx ; mx.m[Z][Z] =  nrm.vz ;
    d = dir->m[Y][Z]*nrm.vz + dir->m[Y][X]*nrm.vx ;

    _sceVu0ScaleVector( &nrm, &nrm, d<0 ?-PUSH_UP :  PUSH_UP ) ;
    _sceVu0AddVector( (FVECTOR*)mx.m[W], (FVECTOR*)dir->m[W], &nrm ) ;
    //NewWallScarDust( (FVECTOR *)dir->m[W], nrm.vx, nrm.vz ) ;

    DG_SetPos( &mx ) ;
    DG_PutVector( d>0 ? for_seg : inv_seg, v, 4 ) ;
    MinMaxVector4( &min, &max, v ) ;
}

void NewFortWallScar( FMATRIX *dir, HZX_SEG *seg, HZX_FLR *flr )
{
    Work *work = FortWallScarBase_Work ;
    FMATRIX  lights[2] ;
    float l ;
    DG_PRIM2_UVRGB *rgb ;
    FVECTOR        *pos ;

    if ( !work )
	work = NewFortWallScarBase( GV_StrCode( "フォーチュン弾痕" ), 0 ) ;
    if ( !work->flag )
	return ;
    rgb = ((DG_PRIM2_UVRGB *)work->scar->uvrgb[0]) + work->n_pos*4 ;
    pos = (       (FVECTOR *)work->scar->pos  [0]) + work->n_pos*4 ;

    DG_GetLightMatrixFix( (FVECTOR *)dir->m[3], lights ) ;
    l = MAX( lights[1].m[0][2], MAX( lights[1].m[0][0],  lights[1].m[0][1] )) ;
    (rgb+3)->r = (rgb+2)->r = (rgb+1)->r = rgb->r =
    (rgb+3)->b = (rgb+2)->b = (rgb+1)->b = rgb->b =
    (rgb+3)->g = (rgb+2)->g = (rgb+1)->g = rgb->g = l ;
    (rgb+3)->a = (rgb+2)->a = (rgb+1)->a = rgb->a = 64 ;

    if ( seg )
	CalcSegMatrix( seg, dir, pos ) ;
    else
	CalcFloorMatrix( flr, dir, pos ) ;

    if ( (work->n_pos = (work->n_pos+1) % work->n_scar) == work->front )
	work->front = work->n_pos ;
}

static void Die( Work *work )
{
    FortWallScarBase_Work = NULL ;

    if ( work->scar )
	GM_FreePrim2( work->scar ) ;
}

static void Act( Work *work )
{
    int i, j ;
    DG_PRIM2_UVRGB *rgb ;
    GV_MSG         *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	if ( (work->flag = msg->message[0]) )
	    DG_InvisiblePrim2( work->scar ) ;
	else
	    DG_VisiblePrim2( work->scar ) ;

    i = work->front - work->n_pos ;
    i = WLLSCR_MAX_DIFF - (i>0 ? i : work->n_scar+i) ;
    for ( j=work->front ; --i>=0 ; j = (j+1)%work->n_scar )
    {
	rgb = (DG_PRIM2_UVRGB *)work->scar->uvrgb[0] + j*4 ;
	if ( rgb->a < 2 )
	    (rgb+3)->a = (rgb+2)->a = (rgb+1)->a = (rgb->a  = 0), work->front++ ;
	else
	    (rgb+3)->a = (rgb+2)->a = (rgb+1)->a = (rgb->a -= 2) ;
    }
    work->front %= work->n_scar ;
}

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
static inline void init_t4pkt( DG_PRIM2_UVRGB *uvrgb, DG_TEX *tex, int i )
{
    for ( ; --i>=0 ; uvrgb += 4 )
    {
	uvrgb[0].u = uvrgb[2].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb[1].u = uvrgb[3].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb[0].v = uvrgb[1].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb[2].v = uvrgb[3].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096 ;
	uvrgb[0].f = uvrgb[1].f = 0x8fff ;
	uvrgb[2].f = 0x0000 ;/* 左回り表示 */
	uvrgb[3].f = 0x0020 ;/* 右回り表示 */
	uvrgb[0].a = uvrgb[1].a = uvrgb[2].a = uvrgb[3].a = 0 ;
    }
}

#define PRIM_FLAG (DG_PRIM2_POLY    | DG_PRIM2_SHADE     |\
				   DG_PRIM2_TEX     | DG_PRIM2_SINGLEBUFF|\
				   DG_PRIM2_CULLPOLY| DG_PRIM2_ALPHA | DG_PRIM2_CCW)

static int GetResources( Work *work, int name, int where )
{
    DG_TEX    *tex ;

    work->name   = name ;
    work->n_scar = where ? GCL_GetOptionValue( 'n', WLLSCR_MAX_UNIT ) : WLLSCR_MAX_UNIT ;
    work->flag   = 1 ;

    if ( !(work->scar = GM_MakePrim2( PRIM_FLAG, work->n_scar, 4 ) ) )
	return -1 ;

    tex = DG_GetTexture( GV_StrCode("lnr_dankon_hibi1_alp") ) ;

    work->scar->group_id = 0x7fffffff ;
    DG_ConfigPrim2Tex( work->scar, tex );
    init_t4pkt( work->scar->uvrgb[0], tex, work->n_scar ) ;	
    DG_SetPrim2Alpha( work->scar, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ) ;
    work->front = 0 ;
    work->n_pos = 0 ;

    FortWallScarBase_Work = work ;

    return 0 ;
}

void *NewFortWallScarBase( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
	    printf( "Dankon Failed\n" ) ;
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
