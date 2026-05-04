//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   wall_scar.c
   弾痕

   1999/11/09 T. Morita
   $Id: wall_scar.c,v 1.1.1.3 2002/11/19 11:46:37 Yoshizawa1 Exp $
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

#include "../include/util.h"

#define _MAIN_FILE_
#include "wall_scar.h"


extern void AN_Test_Eye2( FVECTOR *mov, int size ) ; /* for test */

static Work *wk ;

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
    _sceVu0ScaleVector( &nrm, &nrm, (d<0 ? -PUSH_UP : PUSH_UP) ) ;
    _sceVu0AddVector( (FVECTOR*)mx.m[W], (FVECTOR*)dir->m[W], &nrm ) ;

    _sceVu0AddVector( &min, &min, &nrm ) ;
    _sceVu0AddVector( &max, &max, &nrm ) ;

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

    _sceVu0AddVector( &min, &min, &nrm ) ;
    _sceVu0AddVector( &max, &max, &nrm ) ;

    DG_SetPos( &mx ) ;
    DG_PutVector( d>0 ? for_seg : inv_seg, v, 4 ) ;
    MinMaxVector4( &min, &max, v ) ;
}

void NewWallScar( FMATRIX *dir, HZX_SEG *seg, HZX_FLR *flr )
{
    FMATRIX  lights[2] ;
    float l ;
    SCAR *s = &wk->scar[0] ;
    DG_PRIM2_UVRGB *rgb = ((DG_PRIM2_UVRGB *)s->prim->uvrgb[0]) + s->n_pos*4 ;
    FVECTOR        *pos = (       (FVECTOR *)s->prim->pos  [0]) + s->n_pos*4 ;

    if ( (!seg && !flr) || !wk->flag )
	return ;
    if ( UTL_EFT_CheckBound( (FVECTOR *)dir->m[3] ) < 0 )
    {
	DG_GetLightMatrixFix( (FVECTOR *)dir->m[3], lights ) ;
	l = MAX( lights[1].m[0][2], MAX( lights[1].m[0][0],  lights[1].m[0][1] )) ;
	(rgb+3)->r = (rgb+2)->r = (rgb+1)->r = rgb->r =
	    (rgb+3)->b = (rgb+2)->b = (rgb+1)->b = rgb->b =
	    (rgb+3)->g = (rgb+2)->g = (rgb+1)->g = rgb->g = (int)l ;
	(rgb+3)->a = (rgb+2)->a = (rgb+1)->a = rgb->a = 64 ;

	if ( seg )
	    CalcSegMatrix( seg, dir, pos ) ;
	else
	    CalcFloorMatrix( flr, dir, pos ) ;

	if ( (s->n_pos = (s->n_pos+1) % wk->n_unit) == s->front )
	    s->front = s->n_pos ;
    }
}


static void Die( Work *work )
{
    int i ;

    for ( i=work->n_scar ; --i>=0 ; )
	if ( work->scar[i].prim )
	    GM_FreePrim2(  work->scar[i].prim ) ;
    GV_Free( work->scar ) ;
    UTL_EFT_DelCallback( work ) ;
}

static void ActMessage( Work *work )
{
    GV_MSG *msg ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( (work->flag = msg->message[0]) )
	{
	case 0:
	    for ( i=work->n_scar ; --i>=0 ; )
		if ( work->scar[i].prim )
		    DG_InvisiblePrim2( work->scar[i].prim ) ;
	    break ;
	case 1:
	    for ( i=work->n_scar ; --i>=0 ; )
		if ( work->scar[i].prim )
		    DG_VisiblePrim2( work->scar[i].prim ) ;
	    break ;
	case 2:
	    break ;
	}

}


static void BoundCallBack( Work *work )
{
    DG_PRIM2 *p    ;
    int       i, j ;
    FVECTOR        *pos ;
    DG_PRIM2_UVRGB *uvs ;

    for ( i=work->n_scar ; --i>=0 ; )
	if ( (p = work->scar[i].prim) )
	{
	    pos = p->pos[0]   ;
	    uvs = p->uvrgb[0] ;
	    for( j=0 ; j<work->n_unit ; j++ )
		if ( UTL_EFT_CheckBound( &pos[j*4]) >= 0 )
		    uvs[j*4+0].a = uvs[j*4+1].a =
			uvs[j*4+2].a = uvs[j*4+3].a = 0 ;
	}
}

static void Act( Work *work )
{
    int i, j, k ;
    SCAR *s ;
    DG_PRIM2_UVRGB *rgb ;

    ActMessage( work ) ;
    for ( k=work->n_scar, s=work->scar ; --k>=0 ; s++ )
    {
	i = s->front - s->n_pos ;
	i = WLLSCR_MAX_DIFF - (i>0 ? i : work->n_unit+i) ;
	for ( j=s->front ; --i>=0 ; j = (j+1)%work->n_unit )
	{
	    rgb = (DG_PRIM2_UVRGB *)s->prim->uvrgb[0] + j*4 ;
	    if ( rgb->a < 2 ) {
		(rgb+3)->a = (rgb+2)->a = (rgb+1)->a = (rgb->a  = 0) ;
		s->front++ ;
		s->front %= work->n_unit ;
	    } else {
		(rgb+3)->a = (rgb+2)->a = (rgb+1)->a = (rgb->a -= 2) ;
	    }
	}
    }
}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
static inline void init_t4pkt( DG_PRIM2 *prim, DG_TEX *tex, int i )
{
    DG_PRIM2_UVRGB *uvs ;
    FVECTOR        *pos ;

    uvs = prim->uvrgb[0] ;
    pos = prim->pos[0]   ;

    for ( ; --i>=0 ; uvs+=4, pos+=4 )
    {
	uvs[0].u = uvs[2].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvs[1].u = uvs[3].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvs[0].v = uvs[1].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvs[2].v = uvs[3].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvs[0].q = uvs[1].q = uvs[2].q = uvs[3].q = 4096 ;
	uvs[0].f = uvs[1].f = 0x8fff ;
	//uvs[2].f = uvs[3].f = 0x0fff ;
	uvs[2].f = 0x0000 ;/* 左回り表示 */
	uvs[3].f = 0x0020 ;/* 右回り表示 */
	uvs[0].a = uvs[1].a = uvs[2].a = uvs[3].a = 0 ;

	_sceVu0CopyVector( &pos[0], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[1], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[2], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[3], &DG_ZeroVector ) ;
    }
}


static int GetResources( Work *work, int name )
{
    int        i  ;
    DG_TEX    *tex[32] ;
    SCAR      *s  ;
#if 0
    char      *c  ;

    if ( GCL_GetOption( 'm' ) )
        for( i=0 ; (c=GCL_NextStr()) ; i++ )
            tex[i] = DG_GetTexture( GCL_GetInt( c ) ) ;
    else
        return -1 ;
#else
    i=1 ; tex[0] = DG_GetTexture( GV_StrCode("dankon2_alp") ) ;
#endif

    UTL_EFT_AddCallback( BoundCallBack, work ) ;

    work->name   = name ;
    work->n_unit = GCL_GetOption( 'u' ) ? GCL_GetNextInt() : WLLSCR_MAX_UNIT ;
    work->flag   = 1 ;

    work->scar = GV_Malloc( sizeof(SCAR) * (work->n_scar = i) ) ;
    for ( s = work->scar ; --i>=0 ; s++ )
    {
	if ( !(s->prim = GM_MakePrim2( WALLSCR_PRIM_FLAG, work->n_unit, 4 ) ) )
	    return -1 ;
	s->prim->group_id = 0x7fffffff ;
	DG_ConfigPrim2Tex( s->prim, tex[i] );
	init_t4pkt( s->prim, tex[i], work->n_unit ) ;	
	DG_SetPrim2Alpha( s->prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ) ;
	s->prim->raise = 0 ;
	s->front = 0 ;
	s->n_pos = 0 ;
    }
    wk = work ;

    return 0 ;
}

void *NewWallScarBase( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, name ) < 0 )
        {
	    printf( "Dankon Failed\n" ) ;
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

/*------------------------------------------------------------------*/

/* 弾痕リスト登録コマンド */
/* added by M.Sonoyama 2000/03/29 */
void	NewEntryWallScar( void )
{
    GM_EntryScarFunction( (void *)NewWallScar, GCL_GetOptionValue( 'n', 32 ) ) ;
}
