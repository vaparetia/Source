//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_box.c 
   オルガ オルガ戦専用プットオブジェ

   2000/01/30 T.Morita
   $Id: orga_box.c,v 1.1.1.3 2002/11/19 11:46:25 Yoshizawa1 Exp $
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

#include "../include/orga_se.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


#define ORGA_N_BOXIES 20
#define ORGA_BOX_GRAVITY 6.0f
#define ORGA_BOX_BOUNCE  0.30f
#define ORGA_BOX_ROT_R   16
#define ORGA_BOX_VEL_R   8

#define ORGA_BOX_HZX_SPHERE 400

typedef struct box_t BOX ;
typedef struct boxwork_t Work ;

struct boxwork_t
{
    GV_ACT       actor  ;

    TARGET       target[ORGA_N_BOXIES+1] ;    /*防御 部位ターゲット*/
    POWER_TARGET power [ORGA_N_BOXIES+1] ;

    int          name   ;
    int          flag   ;
    int          hzx_id ;
    int          n_box  ;
    struct box_t
    {
	FVECTOR  pos    ;
	FVECTOR  pos_v  ;
	SVECTOR  rot    ;
	SVECTOR  rot_v  ;

	HZX_SEG  seg[2] ;
	HZX_FLR  flr[2] ;

	TARGET  *target ;
	int    (*act)( Work*, BOX* ) ;
	DG_OBJS *objs   ;
    } box[ORGA_N_BOXIES] ;
} ;

/*

  Inline Functions

*/
static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot )
{
    int r ;
    r = rot->vx & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vz & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vy & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][X] += pos->vx ;
    mtx->m[3][Y] += pos->vy ;
    mtx->m[3][Z] += pos->vz ;
}

static inline float CheckDestanceLineAndPoint( FVECTOR *from, FVECTOR *to, FVECTOR *p )
{
    FVECTOR t1, t2, a ;

    _sceVu0SubVector( &t1, to, from ) ;
    _sceVu0SubVector( &t2,  p, from ) ;
    _sceVu0OuterProduct( &a, &t2, &t1 ) ;
    _sceVu0OuterProduct( &a, &a , &t1 ) ;
    _sceVu0Normalize( &a, &a ) ;

    return -_sceVu0InnerProduct( &a, &t2 ) ;
}




static void Die( Work *work )
{
    BOX *b ;

    for ( b=work->box ; b->objs ; b++ )
    {
	DG_DequeueObjs( b->objs ) ;
	DG_FreeObjs( b->objs );
    }
    GM_FreeTarget( work->target ) ;
}



static void ORG_BOX_HzdCheck( Work *work, HZX_SEG *seg, HZX_FLR *flr, FVECTOR *pos )
{
    int atr[2] ;

    if ( ((pos->vx >= seg[0].p2.x || pos->vx <= seg[0].p1.x)  &&
          (pos->vz >= seg[0].p2.z || pos->vz <= seg[0].p1.z)) ||
         (seg[0].p1.y == seg[0].p1.h && seg[0].p2.y == seg[0].p2.h ) )
        if ( HZX_NearHazardCheck( work->hzx_id, pos,
				  ORGA_BOX_HZX_SPHERE,
				  HZX_CHK_ALL,
				  HZX_SEG_NO_PLAYER,
				  ORGA_BOX_HZX_SPHERE ) )
            HZX_GetNearHazard( seg, atr ) ;
#if 0
    if ( ((pos->vx >= flr[0].b2.x || pos->vx <= flr[0].b1.x)  &&
          (pos->vz >= flr[0].b2.z || pos->vz <= flr[0].b1.z)) ||
         (!flr[0].p1.h && !flr[0].p2.h && !flr[0].p3.h ) )
        if ( HZX_LevelHazardCheck( work->hzx_id, pos, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
            HZX_GetLevelHazard( flr, atr ) ;
#else
	/* HZX_FLR のバウンディングは全然正しくないです（園山）*/
	{
		FVECTOR		max, min ;

		HZX_GetBounding( &flr[ 0 ], &max, &min ) ;
		if ( ((pos->vx >= max.vx || pos->vx <= min.vx)  &&
			  (pos->vz >= max.vz || pos->vz <= min.vz)) ||
			(!flr[0].p1.h && !flr[0].p2.h && !flr[0].p3.h ) )
			if ( HZX_LevelHazardCheck( work->hzx_id, pos, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
				HZX_GetLevelHazard( flr, atr ) ;
	}
#endif
}

static int ORG_BOX_HzdSegLineCheck( Work *work, HZX_SEG *seg, FVECTOR *p1, FVECTOR *p2 )
{
    float px, pz, sx, sz, tx, tz ;
    float r, s, rd ;

    if ( seg->p1.y == seg->p1.h && seg->p2.y == seg->p2.h )
        return 0 ;

    px = p2->vx    - p1->vx    ; pz = p2->vz    - p1->vz    ;
    sx = seg->p2.x - seg->p1.x ; sz = seg->p2.z - seg->p1.z ;
    if ( (rd = sx*pz - sz*px) == 0.0f )                          
        return 0 ;

    tz = seg->p1.z - p1->vz ;
    tx = seg->p1.x - p1->vx ;
    r = (tz*px - tx*pz)/rd ;
    if (  r > 1.0f || r < 0.0f )                 
        return 0 ;
    s = (tz*sx - tx*sz)/rd ;
    if ( s > 1.0f || s < 0.0f )
        return 0 ;
    return 1 ;
}

static void ORG_BOX_HzdSegReactVector( HZX_SEG *seg, FVECTOR *p  )
{
    float dx, dz, f ;

    dx =  seg->p2.x - seg->p1.x ;
    dz =  seg->p2.z - seg->p1.z ;
    f = 2*( dz*p->vx - dx*p->vz )/(dx*dx + dz*dz) ;
    p->vx -= dz * f ;
    p->vz += dx * f ;
}

static void StickPartsOnFloor( BOX *b, FVECTOR *min )
{
    int      i, j ;
    DG_MDLPACK *o = b->objs->objs->model->packs ;
    FVECTOR  f ;

    min->vy = 60000000.0f ;
    for ( j=b->objs->objs->model->n_packs ; --j>=0 ; o++ ) {
#ifdef PSX2
		SVECTOR *s = (SVECTOR *)o->verts ;
		i = o->n_verts ;
#else
		DG_VERTEX_KMSS *s = b->objs->objs->model[j].vbuff ;
		i = b->objs->objs->model[j].n_verts ;
#endif
		while( --i>=0 ) {
#ifdef PSX2
			vu0_SV0toFV( s, &f ) ;
#else
			f.vx = s->vx ;
			f.vy = s->vy ;
			f.vz = s->vz ;
			f.vw = 1.0f ;
#endif
			_sceVu0ApplyMatrix( &f, &b->objs->world, &f ) ;
			if ( f.vy < min->vy )
			  *min = f ;
			s++ ;
		}
	}
}

int ORG_BOX_ActNone( Work *work, BOX *b )
{
    return 0 ;
}


/*
  flag の条件は,以下の表に基づいて作られた

  Z/E = Zero(0 or 180Deg)/Erect(90 or 270Deg)
    _   _ _ _ _ _ _
  x Z Z Z Z E E E E
     _  _ _ _ _ _ _
  y Z Z E E Z Z E E
    _ _ _          
  z Z E Z E Z E Z E
      _
  頭に がついているのが真でついていないのが偽
*/
int ORG_BOX_Act( Work *work, BOX *b )
{
    FVECTOR v ;
    SVECTOR r = { 0, 0, 0, 0 } ;
    int i,flag ;

    if ( !b->objs )
	return 0 ;

    b->pos_v.vy -= ORGA_BOX_GRAVITY ;
    _sceVu0AddVector( &v, &b->pos, &b->pos_v ) ;
    ORG_BOX_HzdCheck( work, b->seg, b->flr, &b->pos ) ;
    if ( ORG_BOX_HzdSegLineCheck( work, b->seg, &b->pos, &v ) )
	ORG_BOX_HzdSegReactVector( b->seg, &b->pos_v ) ;
    else
	_sceVu0CopyVector( &b->pos, &v ) ;

    flag  = (b->rot.vx + 512)&0x400 ? 1 : 0 ;
    flag |= (b->rot.vy + 512)&0x400 ? 2 : 0 ;
    flag |= (b->rot.vz + 512)&0x400 ? 4 : 0 ;
    if ( !(b->rot_v.vx/256) && flag != 4              )
	r.vx = (b->rot.vx + (b->rot.vx>0?512:-512))/1024*1024 - b->rot.vx ;
    if ( !(b->rot_v.vy/256) && flag != 0 && flag != 2 )
	r.vy = (b->rot.vy + (b->rot.vy>0?512:-512))/1024*1024 - b->rot.vy ;
    if ( !(b->rot_v.vz/256) && flag != 1 && flag != 4 )
	r.vz = (b->rot.vz + (b->rot.vz>0?512:-512))/1024*1024 - b->rot.vz ;
//printf( "flag %x S%d %d %d  R%d %d %d\n", flag, r.vx,r.vy,r.vz,	b->rot.vx,b->rot.vy,b->rot.vz ) ;
    for ( i=3 ; --i>=0 ; )
	r.vx /= (r.vx>=2 ? 2 : 1), r.vy /= (r.vy>=2 ? 2 : 1), r.vz /= (r.vz>=2 ? 2 : 1) ;
    AddSVector( &b->rot_v, &b->rot_v, &r ) ;
    AddSVector( &b->rot, &b->rot, &b->rot_v ) ;

    RotateMatrix( &b->objs->world, &DG_UnitMatrix, &b->rot ) ;
    StickPartsOnFloor( b, &v ) ;
    v.vy += b->pos.vy ;
    if ( v.vy < b->flr[0].p1.y )
    {
	b->pos.vy -= v.vy - b->flr[0].p1.y ;
	_sceVu0ScaleVector( &b->pos_v, &b->pos_v, 0.8f ) ;
	if ( (b->pos_v.vy *= -ORGA_BOX_BOUNCE) < 2.0f )
	{
	    if ( ( !(int)b->pos_v.vx && !(int)b->pos_v.vz       ) && /*速度が０  */
		 ( !b->rot_v.vx && !b->rot_v.vz && !b->rot_v.vz ) && /*角速度が０*/
		 ( ( flag != 4              ? !(b->rot.vx&0x3ff) : 1) &&/*面が接している*/
		   ( flag != 0 && flag != 2 ? !(b->rot.vy&0x3ff) : 1) &&
		   ( flag != 1 && flag != 4 ? !(b->rot.vz&0x3ff) : 1) ) )
		b->act = ORG_BOX_ActNone ;
	    b->pos_v.vy = 0.0f ;
//printf( "Rv(%d %d %d)->",b->rot_v.vx,b->rot_v.vy,b->rot_v.vz ) ;
//	    ScaleSVector( &b->rot_v, &b->rot_v, (short)(0.8f*256) ) ;
	    b->rot_v.vx = (short)(b->rot_v.vx * 0.8f) ;
	    b->rot_v.vy = (short)(b->rot_v.vy * 0.8f) ;
	    b->rot_v.vz = (short)(b->rot_v.vz * 0.8f) ;
//printf( "Rv(%d %d %d)\n",b->rot_v.vx,b->rot_v.vy,b->rot_v.vz ) ;
	}
	else
	    ScaleSVector( &b->rot_v, &b->rot_v, (short)(-ORGA_BOX_BOUNCE*256) ) ;
    }
    TransMatrix( &b->objs->world, &b->pos ) ;
    GM_MoveTarget2( b->target, &b->objs->world ) ;

    return 1 ;
}



static void ORG_BOX_Message( Work *work )
{
    GV_MSG *msg ;
    int i, j ;
    BOX *b ;
    FVECTOR min ;
    float d ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch ( msg->message[0] )
        {
        case 0:
	    min.vw = 1000.0f ;

printf( "FROM %.2f %.2f %.2f  TO %.2f %.2f %.2f\n",
((FVECTOR *)msg->message[1])->vx,((FVECTOR *)msg->message[1])->vy,((FVECTOR *)msg->message[1])->vz,
((FVECTOR *)msg->message[2])->vx,((FVECTOR *)msg->message[2])->vy,((FVECTOR *)msg->message[2])->vz
    ) ;

	    for ( j=work->n_box, b=work->box ; --j>=0 ; b++ )
	    {
		d = CheckDestanceLineAndPoint( ((FVECTOR *)msg->message[1]),
					       ((FVECTOR *)msg->message[2]),
					       &b->pos ) ;
		printf( "d=%.2f ", d ) ;
		if ( (d = CheckDestanceLineAndPoint( ((FVECTOR *)msg->message[1]),
						     ((FVECTOR *)msg->message[2]),
						     &b->pos )) < min.vw )
		    min = b->pos, min.vw = d ;
	    }
printf("BOX %.2f  %.2f  %.2f  vw%.2f\n", min.vx,min.vy,min.vz,min.vw ) ;

	    *((FVECTOR *)msg->message[3]) = min ;
            break ;
        }
}



static void Act( Work *work )
{
    int  i ;
    BOX *b = work->box ;

    ORG_BOX_Message( work ) ;    
    for ( i=work->n_box ; --i>=0 ; b++ )
	(*b->act)( work, b ) ;
}



static void ORG_BOX_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    BOX *b = (BOX *)ptr ;
    FMATRIX     mat ;
    extern  void    NewSpark( FMATRIX * ) ; /* 火花を散らす tanaka/tyoudan/tyoudan.c */

    if ( def->damaged & TARGET_POWER )
    {
        b->rot_v.vx = (short)(-off->power->force.vx*0.2f) ;
        b->rot_v.vy = (short)(-off->power->force.vy*0.2f) ;
        b->rot_v.vz = (short)(-off->power->force.vz*0.2f) ;

	b->act = ORG_BOX_Act ;

	mat = DG_UnitMatrix ;
	_sceVu0CopyVector( (FVECTOR*)&mat.m[Y], &off->power->force ) ;
	_sceVu0Normalize( (FVECTOR*)&mat.m[Y], (FVECTOR*)&mat.m[Y] ) ;
	_sceVu0CopyVector( (FVECTOR*)&mat.m[W], &off->hit ) ;
	mat.m[W][W] = 1.0f ;
	DG_ReflectMatrix( (FVECTOR*)&b->objs->world.m[1], &mat, &mat ) ;
	NewSpark( &mat ) ;

	GM_SeSetMode( irnd()&0x40 ? ORGA_SE_WLL_SCAR1 : ORGA_SE_WLL_SCAR2,
		      &def->hit, GM_SEMODE_BOMB ) ;

	/* clear damage */
	GM_ClearTargetDamage( def ) ;
    }
}

static void ORG_BOX_InitTarget( BOX *b, int map, int n_box, int flag,
				float *lp, float *up, FVECTOR *pos, 
				TARGET *t, POWER_TARGET *p )
{
    FVECTOR t_size, t_pos ;
    FVECTOR uv = { up[X], up[Y], up[Z], 0 } ;
    FVECTOR lv = { lp[X], lp[Y], lp[Z], 0 } ;

    _sceVu0SubVector  ( &t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector  ( &t_pos , &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos, 0.5f ) ;
    GM_SetTarget( t, flag, map, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, 0 ) ;
    if ( b )
	GM_SetTargetCallBack( t, ORG_BOX_TargetCallBack, b ),
	GM_MoveTarget2( b->target = t, &b->objs->world ) ;
    if ( n_box )
	GM_SetTargetParts( t, t+1, n_box, 0 ),
	GM_PutTarget( t ),
	GM_MoveTarget( t, pos ) ;
    //NewTargetView( t,  200, 50, 32 ) ;
}


static int GetResources( Work *work, int name, int where )
{
    LIT_DEF *lit_def = NULL ;
    int      i ;
    char    *c ;
    BOX     *b ;
    TARGET  *t ;
    POWER_TARGET *p ;
    FVECTOR t_min={ 600000.0f, 600000.0f, 600000.0f } ;
    FVECTOR t_max={-600000.0f,-600000.0f,-600000.0f }, t_pos ;
    static FVECTOR t_ofs = { 5000.0f, 2500.0f, 5000.0f, 0 } ;

    work->hzx_id = GM_GetHzxGroupID( where ) ;
    work->name   = name ;

    work->flag = GCL_GetOptionValue( 'f', 0 ) ;

    if ( (i = GCL_GetOptionValue( 'l', 0 )) )
        lit_def = (LIT_DEF *)GV_GetCache( GV_CacheID( i, 'l' ) ) ;
    else
	return -1 ;

    if ( GCL_GetOption( 'm' ) )
	for( b=work->box, work->n_box=0 ; (c=GCL_NextStr()) ; b++, work->n_box++ )
	{
	    DG_DEF  *def = GV_GetCache( GV_CacheID( i=GCL_GetInt( c ), 'k' ) ) ;

	    b->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 ) ;
	    DG_QueueObjs( b->objs ) ;
	    DG_MakePreshade( b->objs, lit_def ) ;
	}
    else
	return -1 ;

    t = work->target + 1 ;
    p = work->power  + 1 ;
    if ( GCL_GetOption( 'p' ) )
	for( i=work->n_box, b=work->box ; --i>=0 ; b++, t++, p++ )
	{
	    b->rot_v.vz = b->rot_v.vx = b->rot_v.vy = 0 ;
	    b->rot.vx = GCL_GetNextInt() & 0x0fff ;
	    b->rot.vy = GCL_GetNextInt() & 0x0fff ;
	    b->rot.vz = GCL_GetNextInt() & 0x0fff ;
	    RotateMatrix( &b->objs->world, &DG_UnitMatrix, &b->rot ) ;
	    b->pos.vx = b->objs->world.m[W][X] = (float)GCL_GetNextInt() ;
	    b->pos.vy = b->objs->world.m[W][Y] = (float)GCL_GetNextInt() ;
	    b->pos.vz = b->objs->world.m[W][Z] = (float)GCL_GetNextInt() ;

	    b->act = ORG_BOX_Act ;

	    /* それぞれの箱のターゲットの設定 */
	    ORG_BOX_InitTarget( b, where, 0,
				TARGET_DEFENSE| TARGET_ROTATE| TARGET_POWER| TARGET_SEEK,
				&b->objs->def->lx, &b->objs->def->ux,
				(FVECTOR *)&b->objs->world.m[W],  t, p ) ;
	    _sceVu0AddVector( &t_pos, &t_pos, &b->pos ) ;
	    MinVector( &t_min, &t_min, &b->pos ) ;
	    MaxVector( &t_max, &t_max, &b->pos ) ;
	}
    else
	return -1 ;

    /* 親ターゲットの設定 */
    _sceVu0ScaleVector( &t_pos, &t_pos, 1.0f/work->n_box ) ;
    _sceVu0SubVector( &t_min, &t_min, &t_ofs ) ;
    _sceVu0AddVector( &t_max, &t_max, &t_ofs ) ;
    _sceVu0SubVector( &t_min, &t_min, &t_pos ) ;
    _sceVu0SubVector( &t_max, &t_max, &t_pos ) ;
    ORG_BOX_InitTarget( NULL, where, work->n_box, 
			TARGET_DEFENSE| TARGET_POWER| TARGET_CHILD,
			(float*)&t_min, (float*)&t_max, &t_pos,	work->target, work->power ) ;

    return 0 ;
}


void *NewPutBoxObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
