#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"

#include "hit.h"

extern int HZX_DetCollide( CONTROL *c, float wdth, float leng  ) ;

/*-----------------------------------------------------------------*/

static void Act( work )
Work	*work ;
{
    static short pad_dat[] = { 0,
			       4096*180/360/*u */,       4096* 90/360/*r */,    4096* 90/360/*ru*/,
			       4096*  0/360/*d */, 0,    4096* 90/360/*dr*/, 0, 4096*270/360/*l */,
			       4096*270/360/*ul*/, 0, 0, 4096*270/360/*dl*/, 0, 0, 0, 0 } ;
    int     i ;
    TARGET *t ;

    GM_ActControl( &work->control ) ;
    GM_ActObject ( &work->body    ) ;

    work->control.height = work->body.height ;

    DG_GetLightMatrix( (FVECTOR *)(&work->control.mov), work->lights ) ;

    if ( work->control.grounded & 1 )
	work->control.step.vy = 0.0F ;
    work->control.step.vy -= GRAVITY ;

    for ( i=0, t=work->hzx ; i<2 ; i++ )
    {
	HZX_SEG *s ;
	HZX_FLR *f ;
	FVECTOR	size, center ;

	s = work->control.segs[i] ;
	if ( s && (u_int)s < 0x4000000  )
	{
	    size.vx = abs( s->p1.x - s->p2.x)/2.0f ; size.vz = abs( s->p1.z - s->p2.z )/2.0f ;
	    size.vy = abs( s->p1.h)/2.0f ;
	    center.vx = (s->p1.x + s->p2.x)/2.0f ;
	    center.vy = (s->p1.h          )/2.0f ;
	    center.vz = (s->p1.z + s->p2.z)/2.0f ;
	}
	else
	    size.vx = size.vz = size.vy = 0.0f ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_SEEK, 1, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	GM_MoveTarget( t++, &center ) ;

#define MAX(_a,_b) (((_a)>(_b))?(_a):(_b))
	if ( (f = work->control.level[i]) )
	{
	    size.vx = MAX(abs( f->p1.x - f->p3.x),abs( f->p2.x - f->p4.x))/2.0f ;
	    size.vy = MAX(abs( f->p1.y - f->p3.y),abs( f->p2.y - f->p4.y))/2.0f ;
	    size.vz = MAX(abs( f->p1.z - f->p3.z),abs( f->p2.z - f->p4.z))/2.0f ;

	    center.vx = (f->p1.x+f->p2.x+f->p3.x+f->p4.x)/4.0f ;
	    center.vy = (f->p1.y+f->p2.y+f->p3.y+f->p4.y)/4.0f ;
	    center.vz = (f->p1.z+f->p2.z+f->p3.z+f->p4.z)/4.0f ;
	}
	else
	    size.vx = size.vz = size.vy = 0.0f ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_SEEK, 1, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	GM_MoveTarget( t++, &center ) ;
    }

    {
	FMATRIX m = DG_UnitMatrix ;
	FVECTOR r ;
	vu0_PSDegV2RadV( &work->control.rot, &r );
	_sceVu0RotMatrixZ( &m, &m, r.vz ) ;
	_sceVu0RotMatrixX( &m, &m, r.vx ) ;
	_sceVu0RotMatrixY( &m, &m, r.vy ) ;
	m.m[3][0] = work->control.mov.vx ;
	m.m[3][1] = work->control.mov.vy ;
	m.m[3][2] = work->control.mov.vz ;
	GM_MoveTarget2( &work->def, &m ) ;
    }

    if ( GV_PadData[0].status&PAD_SEL )
	work->control.mov =  GM_PlayerPosition ;
    else if ( GV_PadData[0].status&PAD_L2 && GV_PadData[0].press&PAD_R2 )
	work->control.turn.vy += 5 ;
    else if ( GV_PadData[0].status&PAD_L1 && GV_PadData[0].press&PAD_R1 )
	work->control.turn.vy += 5 ;
    else
    {
	int x = GV_PadData[0].left_dx-128, y = GV_PadData[0].left_dy-128 ;

	if ( GV_PadData[0].status >> 12 )
	{
	    work->control.turn.vy = pad_dat[GV_PadData[0].status>>12] +
		(GV_PadData[0].status&(PAD_L|PAD_R) ? (x<0?y:-y) : (y>0?x:-x))*8 ;

	    if ( !(x/96) && !(y/96) )
	    {
		if ( work->motion != test_walk )
		    GM_ConfigObjectAction( &work->body, 0, work->motion = test_walk, 0, 0xfffff, 30 ) ;
	    }
	    else if ( !(x/120) && !(y/120) )
	    {
		if ( work->motion != test_run )
		    GM_ConfigObjectAction( &work->body, 0, work->motion = test_run, 0, 0xfffff, 30 ) ;
	    }
	    else
	    {
		if ( work->motion != test_dash )
		    GM_ConfigObjectAction( &work->body, 0, work->motion = test_dash, 0, 0xfffff, 30 ) ;
	    }
	}
	else if ( work->motion != naked_pose)
	    GM_ConfigObjectAction( &work->body, 0, work->motion = naked_pose, 0, 0xfffff, 30 ) ;
    }
    HZX_DetCollide( &work->control, WIDTH,  LENGTH ) ;
}

static void Die( work )
Work	*work ;
{
    GM_FreeControl( &work->control ) ;
    GM_FreeObject( &work->body ) ;
    GM_FreeTarget( &work->def ) ;
}

/*-----------------------------------------------------------------*/

/* コントロール初期化 */
static void InitControl( CONTROL *ctrl )
{
    GM_InitControl( ctrl, DEKU_NAME, 0 ) ;
    ctrl->levels[ 0 ] = 0.0F ;
    ctrl->levels[ 1 ] = 32000.0F ;
    ctrl->mov = DG_ZeroVector ; ctrl->mov.vy = 0.0F ;ctrl->mov.vz = 18500.0F ;
    ctrl->step = DG_ZeroVector ;
    ctrl->rot = DG_ZeroSVector ;
    ctrl->turn = DG_ZeroSVector ;

    GM_ConfigControlHazard( ctrl, 1000, 300, 1400 ) ;
    //GM_ConfigControlHzdHeight( ctrl, 750.0F, 4000.0F ) ;
    
    ctrl->seg_flag |= HZX_TYPE_ENEMY ;
    ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
}

/* オブジェクトを初期化 */
static void InitObject( Work *work )
{
    OBJECT	*body ;

    body = &work->body ;
    GM_InitObject( body, RAIDEN_NAME, OBJECT_FLAG ) ;
    GM_ConfigObjectLight( body, work->lights ) ;
    GM_ConfigObjectMotion( body, 2, MOTION_NAME, MT_FLAG_HUMAN2 ) ;
    GM_ConfigObjectStep( body, &work->control.step ) ;
    GM_ConfigObjectAction( body, 0, work->motion =naked_pose , 0, 0xfffff, 0 ) ;
}

/* ターゲット初期化 */
static void InitTarget( Work *work )
{
    TARGET	*t ;
    FVECTOR	size ;
    int i ;
    
    t = &work->def ;
    size.vx = 600.0f ; size.vz = 1200.0F ; size.vy = 600.0F ;
    GM_SetTarget( t, TARGET_DEFENSE | TARGET_SEEK, 1, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
    NewTargetView( t, 34, 184, 200 ) ;

    size.vx = size.vz =  size.vy = 0.0F ;
    for ( i=0, t=work->hzx ; i<2  ; i++ )
    {
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_SEEK, 1, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	NewTargetView( t++, 200, 32, 151 ) ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_SEEK, 1, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	NewTargetView( t++,  50,132, 181 ) ;
    }
    for ( i=0, t=work->hit ; i<2  ; i++ )
    {
	size.vx = size.vz =  size.vy = 200.0f ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_SEEK, 1, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	NewTargetView( t++,  200, 50, 32 ) ;
    }
}

static int GetResources( Work *work )
{
    work->flag = 0 ;
    InitControl( &work->control ) ;
    InitObject( work ) ;
    InitTarget( work ) ;

    return 0 ;
}

/*---------------------------------------------------------------------------*/

void *NewHitMan( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	if ( GetResources( work ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}
