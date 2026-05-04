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

#include  "gameheader.h"
#include  "libutl.h"

#include "../../brk_utl/brk_utl.x"
#include "../../include/util.h"


#define FRT_LSR_ALPHA    80


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    TARGET    seek  ;
    FVECTOR   hit   ;

    DG_PRIM2 *laser ;
    DG_PRIM2 *point ;
    FMATRIX  *world ;
    FVECTOR  *offst ;
    FVECTOR  *aim   ;

    DG_TEX   *tex      ;
    float     v_scroll ;
} Work ;


extern int FRT_SmokeClouded ;


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_SEEK | TARGET_ONLINE | TARGET_ONLINE_MIN | \
			 TARGET_CHILD)

/* ターゲットに当たった */
static void HitTarget( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work = ( Work * )ptr ;
    DG_PRIM2    *p ;

    _sceVu0CopyVector( &work->hit, &off->hit ) ;
    work->hit.vw = 1.0f ;

    p = work->laser ;
    _sceVu0CopyVector( p->pos[p->buffer_clock]+1, &work->hit ) ;
    p = work->point ;
    _sceVu0CopyVector( p->pos[p->buffer_clock]  , &work->hit ) ;

    DG_VisiblePrim2( work->point ) ;
}

static void Act( Work *work )
{
    FVECTOR        *nxt_p ;
    DG_PRIM2_UVRGB *nxt_u ;
    DG_PRIM2       *p ;
    FVECTOR        *from = (void *)SCRPAD_ADDR ;
    extern int FRT_OBJ_ActCheckHideFromToByID( u_int id, FVECTOR *from, FVECTOR *to ) ;

    _sceVu0ApplyMatrix( from, work->world, work->offst ) ;

    /*レーザーポインタを更新*/
    DG_SwitchBuffPrim2( p=work->laser ) ;
    nxt_p = p->pos  [p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;
    _sceVu0CopyVector( nxt_p  , from       ) ;
    _sceVu0CopyVector( nxt_p+1, &work->hit ) ;

    /* レーザーサイトのチラチラ表現 */
    work->v_scroll += rnd()*0.2f ;
    if ( work->v_scroll > 1.0f )
	work->v_scroll -= 1.0f ;
    (nxt_u+0)->v = (nxt_u+1)->v = 
	FTOI12( work->v_scroll * work->tex->v_scale + work->tex->v_offset ) ;

    /* レーザーサイトの濃さ */
    (nxt_u+0)->a = (nxt_u+1)->a = FRT_LSR_ALPHA + FRT_SmokeClouded*3 ;


    /*ポイントの輝点位置を更新*/
    DG_SwitchBuffPrim2( p=work->point ) ;
    _sceVu0CopyVector( p->pos[p->buffer_clock], &work->hit ) ;

    /* ターゲットに当たるか毎フレーム検知 */
    _sceVu0ScaleVector( &work->hit, work->world->m[Y], -12000.0f ) ;
    _sceVu0AddVector( &work->hit, &work->hit, from ) ;
    work->hit.vw = 1.0f ;
    GM_MoveOnlineTarget( &work->seek, from, &work->hit ) ;
    GM_PutTarget( &work->seek ) ;

    /* とりあえずポインタは消す */
    DG_InvisiblePrim2( work->point ) ;
}


static void Die( Work *work )
{
    if ( work->laser )
	GM_FreePrim2( work->laser ) ;
    if ( work->point )
	GM_FreePrim2( work->point ) ;
}

static int GetResources( Work *work, FMATRIX *world, FVECTOR *aim, FVECTOR *offset )
{
    if ( !(work->laser = BRK_UTL_MakeLINE( 1, TXT_LSIGHT_LINE,
					   SCE_GS_SET_ALPHA(0,2,0,1,0),
					   (FRT_LSR_ALPHA<<24)|0x001f1f1f,
					   (FRT_LSR_ALPHA<<24)|0x001f1f1f )) )
	PERROR( "No Laser Prim(no memory) : NewFortLaserSight\n" ) ;
    if ( !(work->point = BRK_UTL_MakeSPRTWH( 1, TXT_LSIGHT_POINT,
					     SCE_GS_SET_ALPHA(0,2,0,1,0),
					     20.0f,
					     0x7f3f3f7f )) )
	PERROR( "No Point Prim(no memory) : NewFortLaserSight\n" ) ;
    work->aim   = aim    ;
    work->offst = offset ;
    work->world = world  ;

    work->tex = DG_GetTexture( TXT_LSIGHT_LINE ) ;

    GM_SetTarget( &work->seek, TARGET_CLASS, 0, PLAYER_SIDE,
		  &DG_ZeroVector, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->seek, WP_LASERSIGHT ) ;
    GM_SetTargetCallBack( &work->seek, HitTarget, work ) ;

    return 0 ;
}

void *NewFortLaserSight( FMATRIX *world, FVECTOR *aim, FVECTOR *offset )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, world, aim, offset ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
