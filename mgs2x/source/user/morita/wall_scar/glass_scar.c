//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   wall_scar.c
   弾痕

   1999/11/09 T. Morita
   $Id: glass_scar.c,v 1.1.1.3 2002/11/19 11:46:37 Yoshizawa1 Exp $
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


#define WLLSCR_MAX_UNIT (2*10)
#define WLLSCR_MAX_DIFF (4)
#define WLLSCR_SIZE 350


#define PUSH_UP 5.0f
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

typedef struct scar_t SCAR ;
typedef struct work_t Work ;
struct work_t
{
    GV_ACT    actor  ;

    int       name   ;
    int       flag   ; /* 制御フラグ */

    DG_PRIM2 *prim   ;
    short     n_scar ;

    struct scar_t
    {
	TARGET          target ;
	POWER_TARGET    power  ;

	FVECTOR        *pos   ;
	DG_PRIM2_UVRGB *uvrgb ;
	short           n_pos ;
	short           front ;
	float           thick ;
	float           size  ;
	float           size_r ;
	int             flag  ;

	void           *fog_mist ;
    } *scar ;

} ;

#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

static void WallGlassTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    SCAR    *s = (SCAR *)ptr ;
    float    l ;
    DG_PRIM2_UVRGB *rgb ;
    FVECTOR        *pos ;
    FVECTOR        v, *p ;
    int  i ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    rgb = s->uvrgb + s->n_pos*5 ;
	    pos = s->pos   + s->n_pos*5 ;

#if 1 /* 残る方 */
	    for ( i=s->n_pos ; --i>=0 ; )
	    {
		p = s->pos + i*5 ;
		_sceVu0AddVector( &v, &p[0], &p[1] ) ;
		_sceVu0AddVector( &v, &v, &p[2] ) ;
		_sceVu0AddVector( &v, &v, &p[3] ) ;
		_sceVu0ScaleVector( &v, &v, 0.25f ) ;
		_sceVu0SubVector( &v, &v, &def->hit ) ;

		if ( v.vx > -s->size_r && v.vx < s->size_r &&
		     v.vy > -s->size_r && v.vy < s->size_r &&
		     v.vz > -s->size_r && v.vz < s->size_r )
		    return ;
	    }
	    if ( s->n_pos >= WLLSCR_MAX_UNIT )
		return ;
	    s->n_pos += 2 ;
#else
	    if ( (s->n_pos = (s->n_pos+2) % WLLSCR_MAX_UNIT) == s->front )
		s->front = s->n_pos ;
#endif

	    GM_SeSetMode( SD_A_HIBI01, &def->hit, GM_SEMODE_BOMB ) ;

#if 0
	    {
		FMATRIX  lights[2] ;
    
		DG_GetLightMatrixFix( &off->power->force, lights ) ;
		l = MAX( lights[1].m[0][2], MAX( lights[1].m[0][0],  lights[1].m[0][1] )) ;
	    }
#else
	    l = 100 ;
#endif

	    (rgb+8)->r = (rgb+7)->r = (rgb+6)->r = (rgb+5)->r =
		(rgb+8)->b = (rgb+7)->b = (rgb+6)->b = (rgb+5)->b =
		(rgb+8)->g = (rgb+7)->g = (rgb+6)->g = (rgb+5)->g =
		(rgb+3)->r = (rgb+2)->r = (rgb+1)->r = (rgb+0)->r =
		(rgb+3)->b = (rgb+2)->b = (rgb+1)->b = (rgb+0)->b =
		(rgb+3)->g = (rgb+2)->g = (rgb+1)->g = (rgb+0)->g = (short)l ;
	    (rgb+3)->a = (rgb+2)->a = (rgb+1)->a = rgb->a = 64 ;

	    BRK_UTL_PutGlassWeb( def,
				 DG_GetTexture( GV_StrCode( "g_dankon_alp" ) ),
				 NULL, s->thick, s->size,
				 pos, pos+5, rgb, rgb+5 ) ;

	    //CalcPoints( &def->world, &def->hit, &off->power->force, pos ) ;
	}
    }
}

static void Die( Work *work )
{
    int  i ;
    SCAR *s ;

    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
    if ( (s = work->scar) ) 
    {
	for( i=work->n_scar ; --i>=0 ; s++ )
	    GV_DestroyOtherActor( s->fog_mist ) ;
	GV_Free( work->scar ) ;
    }
}

static void Act( Work *work )
{
    int i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	if ( (work->flag = msg->message[0]) )
	    DG_VisiblePrim2( work->prim ) ;
	else
	    DG_InvisiblePrim2( work->prim ) ;

#if 0
    DG_PRIM2_UVRGB *rgb ;
    SCAR *s ;

    for ( k=work->n_scar, s=work->scar ; --k>=0 ; s++ )
    {
	i = s->front - s->n_pos ;
	i = WLLSCR_MAX_DIFF - (i>0 ? i : WLLSCR_MAX_UNIT+i) ;
	for ( j=s->front ; --i>=0 ; j = (j+1)%WLLSCR_MAX_UNIT )
	{
	    rgb = s->uvrgb + j*5 ;
	    if ( rgb->a < 2 )
		(rgb+3)->a = (rgb+2)->a = (rgb+1)->a = (rgb->a  = 0), s->front++ ;
	    else
		(rgb+3)->a = (rgb+2)->a = (rgb+1)->a = (rgb->a -= 2) ;
	}
	s->front %= WLLSCR_MAX_UNIT ;
    }
#endif
}

#define PRIM_FLAG (DG_PRIM2_POLY    | DG_PRIM2_SHADE     |\
				   DG_PRIM2_TEX     | DG_PRIM2_SINGLEBUFF|\
				   DG_PRIM2_CULLPOLY| DG_PRIM2_ALPHA | DG_PRIM2_CCW)

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
static inline DG_PRIM2 *InitPrim( DG_TEX *tex, int i )
{
    DG_PRIM2       *prim  ;
    DG_PRIM2_UVRGB *uvs ;
    FVECTOR        *pos   ;

    if ( !(prim = GM_MakePrim2( PRIM_FLAG, i*WLLSCR_MAX_UNIT, 5 ) ) )
	return NULL ;

    prim->group_id = 0x7fffffff ;
    prim->raise    = 0 ;
    DG_ConfigPrim2Tex( prim, tex ) ;
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ) ;

    uvs = prim->uvrgb[0] ;
    pos = prim->pos  [0] ;
    for ( i*=WLLSCR_MAX_UNIT ; --i>=0 ; uvs+=5, pos+=5 )
    {
	uvs[0].u = uvs[2].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvs[1].u = uvs[3].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvs[0].v = uvs[1].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvs[2].v = uvs[3].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvs[0].q = uvs[1].q = uvs[2].q =
	    uvs[3].q = uvs[4].q = 4096 ;
	uvs[0].f = uvs[1].f = uvs[4].f = 0x8fff ;
	//uvs[2].f = uvs[3].f = 0x0fff ;
	uvs[2].f = 0x0000 ;/* 左回り表示 */
	uvs[3].f = 0x0020 ;/* 右回り表示 */
	uvs[0].a = uvs[1].a = uvs[2].a =
	    uvs[3].a = uvs[4].a = 0 ;

	_sceVu0CopyVector( &pos[0], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[1], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[2], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[3], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[4], &DG_ZeroVector ) ;
    }

    return prim ;
}

static int InitTarget( SCAR *s, FVECTOR *p0, FVECTOR *p1,
		       int where, float thick )
{
    TARGET       *t = &s->target ;
    POWER_TARGET *p = &s->power  ;
    FVECTOR       t_size, t_pos ;
    FMATRIX       world ;

    _sceVu0SubVector( &t_size, p0, p1 ) ;
    _sceVu0AddVector( &t_pos , p0, p1 ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;
    _sceVu0RotMatrixY( &world, &DG_UnitMatrix, -atan2f( t_size.vz, t_size.vx ) ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)world.m[W], &t_pos ) ;
    t_size.vx = sceVu0Sqrt( t_size.vx*t_size.vx + t_size.vz*t_size.vz ) ;
    t_size.vz = thick ;

    GM_SetTarget( t, TARGET_DEFENSE| TARGET_POWER| TARGET_ROTATE,
		  where, BOTH_SIDE,
		  &t_size, &DG_ZeroVector ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, WallGlassTargetCallBack, s ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget2Map( t, &world, where ) ;

#if DEBUG_MODE
    if ( s->flag & 1 )
	NewTargetView( t,  200, 50, 32 ) ;
#endif

    {
	FVECTOR center ;
	extern void *NewGlassMistMain2( FVECTOR *center, FVECTOR *housen,
					float width, float height ) ;
	t->offset.vw= 1.0f ;
	_sceVu0ApplyMatrix( &center, &world, &t->offset ) ;
	s->fog_mist = NewGlassMistMain2( &center,
					 (FVECTOR *)world.m[Z],
					 t->size.vx, t->size.vy ) ;
    }

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    int   i     ;
    SCAR *s     ;
    int   count ;
    float thick ;
    float size  ;
    float size_r;

    work->name = name ;
    work->flag = GCL_GetOptionValue( 'f', 0 ) ;


    if ( !(work->n_scar = GCL_GetOptionValue( 'n', 0 )) )
	PERROR( "Please set number more than 1(-number option may missing) : NewGlassScarBase\n" ) ;
    if ( !(s = work->scar = GV_Malloc( sizeof(SCAR) * work->n_scar )) )
	PERROR( "No Memory(work->scar) : NewGlassScarBase\n" ) ;
    if ( !(work->prim = InitPrim( DG_GetTexture( GV_StrCode( "g_dankon_alp" ) ), work->n_scar )) )
	PERROR( "No Memory for PRIM2(work->prim) : NewGlassScarBase\n" ) ;
    thick = (float)(GCL_GetOptionValue( 't', 10 )) ;
    size  = (float)(WLLSCR_SIZE * GCL_GetOptionValue( 's', 100 ) / 100) ;
    size_r= (float)(size        * GCL_GetOptionValue( 'r',  25 ) / 100) ;
    count = 0 ;
    while( (i = GCL_GetNextOption()) )
	switch( i )
	{
	    IVECTOR buf ;
	    FVECTOR p0, p1 ;

	case 'p':
	    GCL_GetIV( GCL_NextStr(), (int*)&buf ) ;
	    vu0_IV0toFV( &buf, &p0 ) ;
	    GCL_GetIV( GCL_NextStr(), (int*)&buf ) ;
	    vu0_IV0toFV( &buf, &p1 ) ;
	    s->front = 0 ;
	    s->n_pos = 0 ;
	    s->flag  = work->flag ;
	    s->thick = thick ;
	    s->size  = size  ;
	    s->size_r= size_r;
	    s->uvrgb  = work->prim->uvrgb[0] ;
	    s->uvrgb += WLLSCR_MAX_UNIT*5 * count ;
	    s->pos    = work->prim->pos[0]   ;
	    s->pos   += WLLSCR_MAX_UNIT*5 * count ;
	    InitTarget( s, &p0, &p1, where, thick ) ;
	    s++ ;
	    if ( ++count > work->n_scar )
		PERROR( "Options exeeds the number required : NewGlassScarBase\n" ) ;
	    break ;
	}

    return 0 ;
}

void *NewGlassScarBase( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;

        if ( GetResources( work, name, where ) < 0 )
        {
	    printf( "Oh, NOOOOooooooo!!!!!! Dankon Failed : NewGlassScarBase\n" ) ;
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
