/*
   brk_box_ini.c
   じゃがいも壊れ

   1999/12/13 T. Morita
   $Id: brk_box_ini.c,v 1.1.1.3 2002/11/19 11:45:23 Yoshizawa1 Exp $
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

#include "brk_box.h"



int BRK_BOX_InitHazard( Work *work, int map )
{
    work->hzx = GM_GetHzxGroupID( map ) ;

    return 0 ;
}




#if 0
int BRK_BOX_GetOptions( Work *work )
{
    int      i ;
    DG_DEF  *m_def, *p_def ;
    BOX  *p ;

    if ( (i = GCL_GetOptionValue( 'f', 0 )) )
        work->n_foot = i ;
    else
        work->n_foot = 0 ;
    if ( (i = GCL_GetOptionValue( 'n', 0 )) )
        work->n_box = i ;

    p = work->box = GV_Malloc( sizeof(POTATO) * (work->n_box + work->n_foot * 2) ) ;
    for (  ; --i>=0 ; p++ )
    {
	MakePositionFromBox( work, &work->box, p ) ;

	p->box = DG_MakeObjs( m_def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
	DG_QueueObjs( (DG_OBJS*)p->box ) ;
	DG_InvisibleObjs( (DG_OBJS*)p->box ) ;
	p->parts.def = p_def ;

	p->seg[0].p1.y = p->seg[0].p2.y = p->seg[0].p1.h = p->seg[0].p2.h = 0 ;
	p->flr[0].p1.h = p->flr[0].p2.h = p->flr[0].p3.h = 0 ;

	/*強引にPARTの初期化を使って初期化しちゃう*/
	BRK_BOX_InitPart( (PART*)p, m_def->models, (DG_OBJ*)p->box, &p->pos,
			  BRK_BOX_Act,
			  BRK_BOX_TargetCallBack ) ;
    }
    work->foot = p ;

    return 0 ;
}
#endif

void BRK_BOX_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    BOX *p = (BOX *)ptr ;
    int i ;

    if ( def->damaged )
    {
        p->rot_vx = -off->power->force.vz*0.3f ;
        p->rot_vy = -off->power->force.vx*0.3f ;

	p->act = BRK_BOX_Act ;

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
    _sceVu0AddVector  ( &t_pos , &t_pos, pos ) ;

    GM_SetTarget( t, flag, map, BOTH_SIDE, &t_size, &DG_ZeroVector ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_BOX_TargetCallBack, b ) ;
    GM_PutTarget( t ) ;
    if ( b )
	GM_MoveTarget2( t, &b->objs->world ) ;
    if ( n_box )
	GM_SetTargetParts( t, t+1, 0, n_box ) ;
    GM_MoveTarget( t, &t_pos ) ;
    //NewTargetView( t,  200, 50, 32 ) ;
}

#if 0
int BRK_BOX_InitBox( BOX *b, DG_MDL *mdl, DG_OBJ *obj, FVECTOR *pos,
		      void *act,
		      void (*callback)( TARGET *off, TARGET *def, void *ptr ) )
{
    b->obj    = obj ;
    b->mdl    = mdl ;
    b->act    = act  ;

    b->pos.vw = b->pos_v.vx = b->pos_v.vy = b->pos_v.vz = 0 ;
    b->rot_vx = b->rot_vy   = 0 ;

    b->pos   = *pos ;
    b->rot_x = irnd()&4095 ;
    b->rot_y = irnd()&4095 ;
    RotateMatrixXY( &b->obj->world, &DG_UnitMatrix, b->rot_x, b->rot_y ) ;
    TransMatrix( &b->obj->world, &b->pos ) ;
}
#endif

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

    work->n_foot = GCL_GetOptionValue( 'f', 0 ) ;
    work->n_box  = GCL_GetOptionValue( 'n', 0 ) ;

    if ( !GCL_GetOption( 'p' ) )
	return -1 ;
    b = work->box = GV_Malloc( sizeof(BOX) * (work->n_box + work->n_foot * 2) ) ;
    t = work->target + 1 ;
    p = work->power  + 1 ;
    for( ; --i>=0  ; b++, t++, p++ )
    {
	b->rot_vx = b->rot_vy = 0 ;
	b->rot_x = GCL_GetNextInt() & 0x0fff ;
	b->rot_y = GCL_GetNextInt() & 0x0fff ;
	RotateMatrixXY( &b->objs->world, &DG_UnitMatrix, b->rot_x, b->rot_y ) ;
	b->pos.vx = b->objs->world.m[W][X] = GCL_GetNextInt() ;
	b->pos.vy = b->objs->world.m[W][Y] = GCL_GetNextInt() ;
	b->pos.vz = b->objs->world.m[W][Z] = GCL_GetNextInt() ;

	/* それぞれの箱のターゲットの設定 */
	ORG_BOX_InitTarget( b, where, 0,
			    TARGET_DEFENSE|TARGET_ROTATE|TARGET_SEEK|TARGET_POWER,
			    &b->objs->def->lx, &b->objs->def->ux, &b->pos,  t, p ) ;
	_sceVu0AddVector( &t_pos, &t_pos, &b->pos ) ;
	MinVector( &t_min, &t_min, &b->pos ) ;
	MaxVector( &t_max, &t_max, &b->pos ) ;
    }
    /* 親ターゲットの設定 */
    _sceVu0ScaleVector( &t_pos, &t_pos, 1.0f/work->n_box ) ;
    _sceVu0SubVector( &t_min, &t_min, &t_ofs ) ;
    _sceVu0AddVector( &t_max, &t_max, &t_ofs ) ;
    _sceVu0SubVector( &t_min, &t_min, &t_pos ) ;
    _sceVu0SubVector( &t_max, &t_max, &t_pos ) ;
    ORG_BOX_InitTarget( NULL, where, work->n_box, 
			TARGET_DEFENSE|TARGET_POWER|TARGET_CHILD,
			(float*)&t_min, (float*)&t_max, &t_pos,	work->target, work->power ) ;

    return 0 ;
}
