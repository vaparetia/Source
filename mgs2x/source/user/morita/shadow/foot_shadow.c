//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include "gameheader.h"
#include "libdg.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h" 


#ifdef KP_XBOX   /* XBOXでは、足影はプリム */
#define USEPRIM
#endif


#define SDW_FOOT_HGT_DIFF 88.0f
#define SDW_N_SHADOW      4

typedef struct work_t
{
    GV_ACT_EX actor   ;

#ifdef USEPRIM
	DG_PRIM2 *shadow ;
#else
    DG_COMDL *shadow  ;
#endif
    FVECTOR   lgt_dir ;
    int       fade ;

    int       l_alph,  r_alph ;
    FMATRIX  *l_foot, *r_foot ;
    CONTROL  *control ;
    FMATRIX  *lights ;
    int      *flag ;
} Work ;

#ifdef KP_XBOX
#define SHDW_ALPH_MAX   90
#else
#define SHDW_ALPH_MAX   120 
#endif
#define SHDW_ALPH_SPEED 4


#ifdef USEPRIM
static void MakeShape( FVECTOR *p, FMATRIX *m )
{
	static FVECTOR shape[] = {
		{-100.0f, 0.0f, 200.0f, 1.0f },
		{ 100.0f, 0.0f, 200.0f, 1.0f },
		{-100.0f, 0.0f,   0.0f, 1.0f },
		{ 100.0f, 0.0f,   0.0f, 1.0f },
	} ;
	int i ;

	for( i=4 ; --i>=0 ; ) {
		_sceVu0ApplyMatrix( p++, m, &shape[i] ) ;
	}
}
#endif


/* Calcurate the Shadow-Matrix from Floor-Hazard */
static void CalcShadowMatrix( FMATRIX *mx, HZX_FLR *f )
{
    float dx, dz ;
    float nx, ny, nz ;

    *mx = DG_UnitMatrix ;
    /* rotate shadow for xz-axis as slope does */
    mx->m[0][1] = nx = f->p1.h/32000.0F ;
    mx->m[1][1] = ny = f->p3.h/32000.0F ;
    mx->m[2][1] = nz = f->p2.h/32000.0F ;
    dx = ( nx<0.001F && ny<0.001F ) ? 100.0F : sceVu0Sqrt(nx*nx + ny*ny) ;
    dz = ( nz<0.001F && ny<0.001F ) ? 100.0F : sceVu0Sqrt(nz*nz + ny*ny) ;
    mx->m[1][0] = nx / dx ; mx->m[0][0] =-ny / dx ;
    mx->m[1][2] = nz / dz ; mx->m[2][2] =-ny / dz ;
}

static void CalcFootLongShadow( Work *work,
							    FMATRIX *o, FMATRIX *world, float y, int *si )
{
    float d, cs, sn ;
    FVECTOR v ;
    FMATRIX mx ;
    //int *si = &(p-0)->color.vw ;
    //int *so = &(p-2)->color.vw ;

    v.vx = -work->lgt_dir.vx ;
    v.vy =  work->lgt_dir.vy ;
    v.vz = -work->lgt_dir.vz ;

    /* 足の上下により影を表示する */
#if 1
    if ( *si >= 0 )
	if ( (d=sceVu0Sqrt( v.vx*v.vx + v.vz*v.vz )) > 0.0001F || v.vy < 0.0F )
	{
#else
	    d = sceVu0Sqrt( v.vx*v.vx + v.vz*v.vz ) ;
		if( d < 0.0001F || DG_FABS(v.vy) < 0.0001F ) return;	// add 2002/02/21 K.Uehara 0.0check
#endif
	    if ( d > 0.0f ){
		  cs = v.vz/d ; sn = v.vx/d ; /* calc cos & sin for XZ-plane */
		} else {
		  cs = 0.0f   ; sn = 1.0f   ; /* calc cos & sin for XZ-plane */
	    }

	    //d =  d/v.vy>-3.0F ? -3.0F : (d/v.vy<3.0F ? 3.0F : d/v.vy) ;
		if ( v.vy==0.0f )
		  d = 2.0f ;
		else {
			d /= v.vy ;
			d = d <-2.0F ? -2.0F : 
			    d > 2.0F ?  2.0F : d ;
		}

	    /* rotate shadow (scale d to Z-axis and rotate for Y-axis)*/
	    _sceVu0CopyMatrix( world, &DG_UnitMatrix ) ;
	    world->m[0][0] =  cs ; world->m[2][2] = cs*d ; 
	    world->m[0][2] = -sn ; world->m[2][0] = sn*d ;
	    if ( work->control->level[0] )
	    {
		CalcShadowMatrix( &mx, work->control->level[0] ) ;
		_sceVu0MulMatrix( &world->m, &mx, &world->m ) ;
	    }

	    /* check disappear or not */
	    if ( (d = o->m[3][1]-y) < SDW_FOOT_HGT_DIFF )
		*si = (*si<SHDW_ALPH_MAX  ) ? (*si+SHDW_ALPH_SPEED) : SHDW_ALPH_MAX ;
	    else
		*si = (*si>SHDW_ALPH_SPEED) ? (*si-SHDW_ALPH_SPEED) : 0 ;
#if 1
	}
	else
	    *si *= -1 ;
    else
	*si += 1 ;
#endif

	/* 影の明るさは大元を越えない */
    if ( *si > work->fade )
	*si = work->fade ;
    // *so = *si ;
	 
	world->m[3][0] = o->m[3][0] ;
	world->m[3][1] = y ;
	world->m[3][2] = o->m[3][2] ;
}


static void CalcFootShadow( Work *work, FMATRIX *right, FMATRIX *left, float y )
{
    float dx, dz ;
    FMATRIX  mx ;
    FMATRIX *l = work->l_foot, *r = work->r_foot ;

    /* length between both feet in XZ-plane */
    dx = r->m[3][0] - l->m[3][0] ;
    dz = r->m[3][2] - l->m[3][2] ;

    /* rotate shadow for y-axis */
    _sceVu0CopyMatrix( right, &DG_UnitMatrix ) ;
    _sceVu0CopyMatrix( left , &DG_UnitMatrix ) ;
    right->m[0][0] = -(left->m[0][0] =  dz/400.0F) ;
    right->m[0][2] = -(left->m[0][2] = -dx/400.0F) ;
    right->m[2][2] = -(left->m[2][2] =  dz/300.0F) ; /* because Z size is 200.F */
    right->m[2][0] = -(left->m[2][0] =  dx/300.0F) ;

    if ( work->control->level[0] )
    {
	CalcShadowMatrix( &mx, work->control->level[0] ) ;
	_sceVu0MulMatrix( right, &mx, right ) ;
	_sceVu0MulMatrix( left , &mx, left  ) ;
    }

    /* trans to both foot */
    right->m[3][0] = l->m[3][0] ;
    right->m[3][2] = l->m[3][2] ;
	right->m[3][1] = y ;
    left->m[3][1] = y + 1.0f ;
    left->m[3][0] = r->m[3][0] ;
    left->m[3][2] = r->m[3][2] ;
}

static void Act( Work *work )
{
    float    y ;
    int      det ;

    if ( GM_CheckGameStatus( STATE_DEMO ) )
	det = 1 ;
    else
	det = !(work->control->skip_flag & CTRL_SKIP_FLR_CHECK) ;

	if ( GM_VRStatus & GM_VR_IDLE )
	  det = 0 ;

    if ( (work->flag ? *work->flag : 1) && det )
	work->fade = (work->fade<SHDW_ALPH_MAX  )?(work->fade+SHDW_ALPH_SPEED):SHDW_ALPH_MAX ;
    else
	work->fade = (work->fade>SHDW_ALPH_SPEED)?(work->fade-SHDW_ALPH_SPEED):0 ;

    if ( !work->fade )
#ifdef USEPRIM
	work->shadow->flag |=  DG_PRIM2_INVISIBLE ;
#else
	work->shadow->flag |=  DG_COMDL_INVISIBLE ;
#endif
    else
    {
#ifdef USEPRIM
	work->shadow->flag &= ~DG_PRIM2_INVISIBLE ;
#else
	work->shadow->flag &= ~DG_COMDL_INVISIBLE ;
#endif

	GM_GroupObject( work->shadow, work->control->map ) ;
	y = work->control->levels[ 0 ] + 5.0f ;

	/* smooth direction change */
	work->lgt_dir.vx += ( work->lights[0].m[0][X] - work->lgt_dir.vx ) / 16.0f ;
	work->lgt_dir.vy += ( work->lights[0].m[0][Y] - work->lgt_dir.vy ) / 16.0f ;
	work->lgt_dir.vz += ( work->lights[0].m[0][Z] - work->lgt_dir.vz ) / 16.0f ;

	{
#ifdef USEPRIM
		FMATRIX right, left ;
		FVECTOR        *pos ;
		DG_PRIM2_UVRGB *uvs ;
		int i ;

		DG_SwitchBuffPrim2( work->shadow ) ;
		pos = work->shadow->pos  [work->shadow->buffer_clock] ;
		uvs = work->shadow->uvrgb[work->shadow->buffer_clock] ;
#endif


		/*for body shadow */
		CalcFootShadow( work,
#ifdef USEPRIM
					   &right,
					   &left,
#else
					   &work->shadow->pos[0].world,
					   &work->shadow->pos[1].world,
#endif
					   y+1.0f ) ;

#ifdef USEPRIM
		MakeShape( pos+0, &right ) ;
		MakeShape( pos+8, &left  ) ;
#endif


		/*for foot shadow */
		CalcFootLongShadow( work, work->l_foot,
#ifdef USEPRIM
						   &left,
#else
						   &work->shadow->pos[2].world,
#endif
						   y+3.0f,
						   &work->l_alph ) ;

		CalcFootLongShadow( work, work->r_foot,
#ifdef USEPRIM
						   &right,
#else
						   &work->shadow->pos[3].world,
#endif
						   y+4.0f,
						   &work->r_alph ) ;

#ifdef USEPRIM
		MakeShape( pos+4 , &right ) ;
		MakeShape( pos+12, &left  ) ;

		for( i=4*2 ; --i>=0 ;  ){
			uvs->a = work->r_alph>0 ? work->r_alph : -work->r_alph ;
			uvs++ ;
		}
		for( i=4*2 ; --i>=0 ;  ){
			uvs->a = work->l_alph>0 ? work->l_alph : -work->l_alph ;
			uvs++ ;
		}
#else
		work->shadow->pos[0].color.vw = work->shadow->pos[2].color.vw =
		  work->l_alph>0 ? work->l_alph : -work->l_alph ;
		work->shadow->pos[1].color.vw = work->shadow->pos[3].color.vw = 
		  work->r_alph>0 ? work->r_alph : -work->r_alph ;
#endif
	}
    }
}


static void Die( Work *work )
{
#ifdef USEPRIM
    if ( work->shadow ) {
		GM_FreePrim2( work->shadow ) ;
	}
#else
    if ( work->shadow ) {
		DG_DequeueComdlObjs( work->shadow ) ;
		DG_FreeComdl( work->shadow ) ;
	}
#endif
}


static int GetResources( Work *work, 
			 DG_OBJ *l_foot, DG_OBJ *r_foot,
			 CONTROL *control,
			 FMATRIX *lights,
			 int *flag )
{
    int i ;

    work->l_foot  = &l_foot->world ;
    work->r_foot  = &r_foot->world ;
    work->control = control ;
    work->lights  = lights  ;
    work->flag    = flag    ;
    work->fade    = 0 ;
    work->l_alph = work->fade ;
    work->r_alph = work->fade ;

    work->lgt_dir = *(FVECTOR*)&lights[0].m[0] ;

	{
#ifdef USEPRIM
		int j ;
		FVECTOR        *p0, *p1 ;
		DG_PRIM2_UVRGB *u0, *u1 ;	/* スプライト用 */
		DG_TEX         *tex = DG_GetTexture( 13721466 ) ; /*foot_shadow_alp*/

		if ( !(work->shadow = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, SDW_N_SHADOW, 4 )) )
		  PERROR( "Can't make Comdl(May be no memory) : NewShadow\n" ) ;
		DG_ConfigPrim2Tex( work->shadow, tex ) ;
		DG_SetPrim2Alpha( work->shadow, SCE_GS_SET_ALPHA(0,1,0,1,0x00) ) ;

		p0 = work->shadow->pos[0]  , p1 = work->shadow->pos[1]   ;
		u0 = work->shadow->uvrgb[0], u1 = work->shadow->uvrgb[1] ;
		for ( i=SDW_N_SHADOW ; --i>=0 ; ) {
			_sceVu0CopyVector( p0, &DG_ZeroVector ) ;
			_sceVu0CopyVector( p1, &DG_ZeroVector ) ;
			p0 += 4 ;
			p1 += 4 ;

			for ( j=4 ; --j>=0 ; ) {
#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
				u0->u = u1->u = FTOI12( (j&1 ? 0.0f : 1.0f)*tex->u_scale + tex->u_offset ) ;
				u0->v = u1->v = FTOI12( (j&2 ? 0.0f : 1.0f)*tex->v_scale + tex->v_offset ) ;
				u0->q = u1->q = 4096 ;
				u0->f = u1->f = j>=2 ? 0x8fff : 0x0fff ;
				
				u0->r = u1->r =
				u0->g = u1->g =
				u0->b = u1->b = 128 ;
				u0->a = u1->a = 0 ;
				u0++, u1++ ;
			}
		}
#else
		DG_DEF  *def ;
		DG_COMDL_POS *p ;

		if ( !(def = (DG_DEF *)GV_GetCache( GV_CacheID( GV_StrCode( "foot_shadow_cm" ), 'k' ) )) )
		  PERROR( "Can't find 'foot_shadow_cm.kms' (not in data.cnf) : NewShadow\n" ) ;
		if ( !(work->shadow = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, SDW_N_SHADOW, 0 )) )
		  PERROR( "Can't make Comdl(May be no memory) : NewShadow\n" ) ;
		DG_QueueComdlObjs( work->shadow ) ;

		p = work->shadow->pos ;
		for ( i=SDW_N_SHADOW ; --i>=0 ; ) {
			p->world = DG_UnitMatrix ;
			p->color.vx = p->color.vy = p->color.vz = 120 ;
			p->color.vw = 0 ;
			p++ ;
		}
#endif
	}

    return 0 ;
}

void *NewShadow( DG_OBJ *l_foot, DG_OBJ *r_foot,
				 CONTROL *control, FMATRIX *lights, int *flag )
{
    Work *work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, l_foot, r_foot, control, lights, flag ) < 0 ){
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
