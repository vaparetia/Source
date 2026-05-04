//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_computer.c
   壊れコンピュータ

   2001/06/24 T. Morita
   $Id: brk_computer.c,v 1.1.1.3 2002/11/19 11:45:25 Yoshizawa1 Exp $
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

#include "brk_computer.h"

#define WLLSCR_MAX_UNIT (10)
#define WLLSCR_MAX_DIFF (4)
#define WLLSCR_SIZE 300


#define PUSH_UP 5.0f

#define MIN(_a,_b) ((_a)< (_b)?(_a):(_b))
#define MAX(_a,_b) ((_a)>=(_b)?(_a):(_b))

extern void *NewBombEffect( FVECTOR *, int  ) ;
extern void *NewBreakComputerSquareButton( FVECTOR *pos, int rgba, int life ) ;
extern void *NewBreakComputerBlink( FVECTOR *pos, int rgba, int life, float size ) ;
extern void *NewBreakComputerPlasma( FVECTOR *pos, FVECTOR *conf, int life ) ;
extern void *NewBombGasEffect( FVECTOR *pos, SVECTOR *pole_rot ) ;
extern void *NewCrushDust( FVECTOR *pos, FVECTOR *force, int mode ) ;
extern void *NewBreakComputerBombKasu( FVECTOR *pos, SVECTOR *pole_rot ) ;
extern void *NewBreakComputerMonitor( int name, FVECTOR *pos, int start ) ;


static int BombEffect( FVECTOR *pos )
{
	HZX_GROUP_ID	map_id;
	float		flr_height[2];
	float		rad;
	float		ftemp;
	FVECTOR		fvtemp;
	int			seg_num;
	int			atr[2] ;
	HZX_SEG		seg[2] ;
	SVECTOR		pole_rot;

#define	LIFE_TIME		(64)
#define	SIZE_MIN		(500.0f)
#define	SIZE_RAND		(500.0f)
#define	SEARCH_HZX			(2000)

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );
	seg_num = HZX_NearHazardCheck( map_id,
				       pos,
				       SEARCH_HZX,
				       HZX_CHK_ALL,0,
				       SEARCH_HZX);

	/* 反射ベクトル */
	pole_rot.vy = 0;
	pole_rot.vz = 0;
	rad = 0.0f;	/* Ｘ回転の合計を計算する */
	if( seg_num ){
		HZX_GetNearHazard( seg, atr ) ;
		HZX_GetReactVector( &fvtemp );

		ftemp = atan2f( fvtemp.vx, fvtemp.vz ) ;
		pole_rot.vy = 4095 & ( short )( ( ftemp * 2048.0f / PI ) + 0.5f );

		rad = PI * 0.5f;
		if( HZX_LevelHazardCheck( map_id, pos, HZX_CHK_ALL, 0 ) & 1 ){
			HZX_GetLevelHeight( flr_height );
			ftemp = pos->vy - flr_height[0];
			if( ftemp > 0 ){
				rad += asinf( ftemp / ((SIZE_RAND + SIZE_MIN) * (float)LIFE_TIME) );
			}
		}
	}
	pole_rot.vx = 4095 & ( short )( ( rad * 1024.0f / PI ) + 0.5f );

	/* 中規模の煙 */
	NewBreakComputerBombKasu( pos, &pole_rot );
	//NewBombGasEffect( pos, &pole_rot );
	//NewBombKasu( pos, &pole_rot );

	return 0 ;
}

static void KillSameName( Work *work, int name )
{
    int i ;

    for ( i=work->n_comp ; --i>=0 ; )
	if ( work->comp[i].conf->name  == name )
	    if ( work->comp[i].ext )
	    {
		GV_DestroyOtherActor( work->comp[i].ext ) ;
		work->comp[i].ext = NULL ;
	    }	
}

static void KillComp( COMP *comp, int bomb )
{
    TARGET *def = &comp->target ;

    GM_FreeTarget( def ) ;

    /* 焦げ目を付ける */
    comp->uvrgb[0].a = comp->uvrgb[1].a =
	comp->uvrgb[2].a = comp->uvrgb[3].a = 32 ;

    /* ちょっとした爆発エフェクトを乗せる */
    if ( bomb )
    {
	GM_SeSetMode( SD_A_RICCOMP2, &def->hit, GM_SEMODE_BOMB ) ;
	//printf( "%f %f %f\n", def->hit.vx, def->hit.vy, def->hit.vz ) ; 
	//AN_Test_Eye2( &def->hit, 2 ) ;

	BombEffect( &def->hit ) ;
    }

    /* 同じ名前のものを消す */
    KillSameName( comp->work, comp->conf->name ) ;
}

static void BRK_COM_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    COMP *comp = (COMP *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( comp->work->non_damage > 0 )
	    return ; 

	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    FVECTOR ldir, lpos ;

	    BRK_UTL_CallOffenceWhenThrough( off, def ) ;

	    _sceVu0CopyVector( &lpos, &def->hit ) ;
	    _sceVu0Normalize( &ldir, &off->power->force ) ;
	    if ( !BRK_UTL_CheckLineInFloatPolygon( &lpos, &ldir,
						   comp->conf->v+0 ) )
		if ( !BRK_UTL_CheckLineInFloatPolygon( &lpos, &ldir,
						       comp->conf->v+1 ) )
		    return ;
	    if ( comp->life > 0 )
		if ( !--comp->life )
		    KillComp( comp, 1 ) ;

	    comp->work->flag |= 1 << comp->conf->group ;
	    if ( comp->work->proc )
	    {
		GCL_ARGS arg = { 1, &comp->work->flag } ;
		GCL_ExecProc( comp->work->proc, &arg ) ;
	    }

	    /* 6フレームの間,弾は当たらない */
	    comp->work->non_damage = 6 ;

	    /* 着弾音 */
	    GM_SeSetMode( SD_A_RICCOMP1, &def->hit, GM_SEMODE_BOMB ) ;

	    /* プラズマを出す */
	    NewBreakComputerPlasma( &lpos, comp->conf->v, 3 ) ;
	}
    }
}


static void Act( Work *work )
{
    if ( work->non_damage > 0 )
	work->non_damage-- ;
}

static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
    if ( work->comp ) 
    {
	int i ;

	for ( i=work->n_comp ; --i>=0 ; )
	{
	    GM_FreeTarget( &work->comp[i].target ) ;
	    if ( work->comp[i].ext )
		GV_DestroyOtherActor( work->comp[i].ext ) ;
	}
	GV_Free( work->comp ) ;
    }
}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
static inline DG_PRIM2 *InitPrim( int tex_id, int i )
{
    DG_PRIM2       *prim ;
    DG_PRIM2_UVRGB *u    ;
    DG_TEX         *tex = DG_GetTexture( tex_id ) ;
    FVECTOR        *p    ;

    if ( !(prim = GM_MakePrim2( (DG_PRIM2_POLY| DG_PRIM2_SHADE     |
				 DG_PRIM2_TEX | DG_PRIM2_SINGLEBUFF|
				 DG_PRIM2_ALPHA), i, 4 ) ) )
	return NULL ;
    DG_ConfigPrim2Tex( prim, tex ) ;
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 1, 0, 1, 0x00 ) ) ;

    u = prim->uvrgb[0] ;
    p = prim->pos  [0] ;
    for ( ; --i>=0 ; u+=4, p+=4 )
    {
	u[0].u = u[2].u = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
	u[1].u = u[3].u = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
	u[0].v = u[1].v = FTOI12( 0.0f * tex->v_scale + tex->v_offset ) ;
	u[2].v = u[3].v = FTOI12( 1.0f * tex->v_scale + tex->v_offset ) ;
	u[0].q = u[1].q = u[2].q = u[3].q = 4096 ;
	u[0].f = u[1].f = 0x8fff ;
	u[2].f = u[3].f = 0x0fff ;
	u[0].r = u[1].r = u[2].r = u[3].r = 
	u[0].g = u[1].g = u[2].g = u[3].g = 
	u[0].b = u[1].b = u[2].b = u[3].b = 128 ;
	u[0].a = u[1].a = u[2].a = u[3].a = 0 ;

	_sceVu0CopyVector( &p[0], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p[1], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p[2], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p[3], &DG_ZeroVector ) ;
    }

    return prim ;
}

static int InitTarget( COMP *s, FVECTOR *pos, int where, int flag )
{
    TARGET       *t = &s->target ;
    POWER_TARGET *p = &s->power  ;
    FVECTOR       size0, size1,  t_pos ;

    /* 頂点たちを含むようなターゲットにする */

    /* 大きさは３つの点の最大を大きさとする*/
    _sceVu0SubVector( &size0, &pos[0], &pos[1] ) ;
    size0.vx = fpu_Abs( size0.vx ) ;
    size0.vy = fpu_Abs( size0.vy ) ;
    size0.vz = fpu_Abs( size0.vz ) ;
    _sceVu0SubVector( &size1, &pos[0], &pos[2] ) ;
    size1.vx = fpu_Abs( size1.vx ) ;
    size1.vy = fpu_Abs( size1.vy ) ;
    size1.vz = fpu_Abs( size1.vz ) ;
    size0.vx = size0.vx>size1.vx ? size0.vx : size1.vx ;
    size0.vy = size0.vy>size1.vy ? size0.vy : size1.vy ;
    size0.vz = size0.vz>size1.vz ? size0.vz : size1.vz ;
    _sceVu0ScaleVector( &size0, &size0, 0.5f ) ;
    size0.vx = size0.vx<1.0f ? 1.0f : size0.vx ;
    size0.vy = size0.vy<1.0f ? 1.0f : size0.vy ;
    size0.vz = size0.vz<1.0f ? 1.0f : size0.vz ;

    /* 位置は４つの点の平均にする*/
    _sceVu0AddVector( &t_pos, &pos[0], &pos[1] ) ;
    _sceVu0AddVector( &t_pos, &t_pos, &pos[2] ) ;
    _sceVu0AddVector( &t_pos, &t_pos, &pos[3] ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.25f ) ;

    GM_SetTarget( t, TARGET_DEFENSE| TARGET_POWER| TARGET_THROUGH,
		  where, BOTH_SIDE, &size0, &DG_ZeroVector ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality,
		       0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_COM_TargetCallBack, s ) ;
    GM_PutTarget( t ) ;
    GM_SetTargetCallBack( t, BRK_COM_TargetCallBack, s ) ;
    GM_MoveTargetMap( t, &t_pos, where ) ;

#ifdef DEBUG_MODE
    if ( flag & 0x100 )
	NewTargetView( t,  200, 50, 32 ) ;
#endif

    return 0 ;
}

static int GetResources( Work *work,
			 int name, int where,
			 BRK_CONF *comp, int n_comp,
			 int flag, int proc )
{
    int   i ;
    COMP *s ;

    work->name   = name   ;
    work->flag   = flag   ;
    work->proc   = proc   ;
    work->n_comp = n_comp ;
    if ( !(work->comp = GV_Malloc( sizeof(COMP) * work->n_comp )) )
	PERROR( "No Memory(work->comp) : NewBreakComputer\n" ) ;
    if ( !(work->prim = InitPrim( 1081306/*g_dankon_alp*/, work->n_comp )) )
	PERROR( "No Memory for PRIM2(work->prim) : NewGlassScarBase\n" ) ;

    for( i=0, s=work->comp ; i<n_comp ; i++, s++ )
    {
	s->conf = &comp[i] ;
	s->ext  = NULL ;
	s->work = work ;
	switch( comp[i].flag & 0xff )
	{
	    int rgba ;
	    FVECTOR *pos ;

	case 3:
	    s->ext = NewBreakComputerMonitor( comp[i].name,
					      comp[i].v, comp[i].life ) ;
	    break ;

	case 2:
	    s->ext = NewBreakComputerSquareButton( comp[i].v,
						   comp[i].life,
						   (int)comp[i].v[0].vw ) ;
	    break ;

	case 1:
	    rgba = (int)comp[i].v[1].vw ;
	    rgba = (int)comp[i].v[1].vz | (rgba << 8) ;
	    rgba = (int)comp[i].v[1].vy | (rgba << 8) ;
	    rgba = (int)comp[i].v[1].vx | (rgba << 8) ;
	    s->ext = NewBreakComputerBlink( &comp[i].v[0],
					    rgba,
					    (int)comp[i].v[2].vx,
					    comp[i].v[2].vy ) ;
	    s->uvrgb = NULL ;
	    s->life  = comp[i].life ;
	    break ;

	case 0:
	    s->uvrgb  = work->prim->uvrgb[0] ;
	    s->uvrgb += i * 4 ;
	    pos = work->prim->pos[0] ;
	    pos += i * 4 ;
	    _sceVu0CopyVector( &pos[0], &comp[i].v[0] ) ;
	    _sceVu0CopyVector( &pos[1], &comp[i].v[1] ) ;
	    _sceVu0CopyVector( &pos[2], &comp[i].v[2] ) ;
	    _sceVu0CopyVector( &pos[3], &comp[i].v[3] ) ;
	    s->life  = comp[i].life ;
	    InitTarget( s, comp[i].v, where, comp[i].flag ) ;
	    break ;
	}
    }

    if ( flag )
	for( i=0, s=work->comp ; i<n_comp ; i++, s++ )
	    if ( (s->conf->flag & 0xff) == 0 )
		if ( flag & (1 << s->conf->group) )
		    KillComp( s, 0 ) ;
    return 0 ;
}

void *NewBreakComputer( int name, int where,
			BRK_CONF *comp, int n_comp,
			int flag, int proc )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;

        if ( GetResources( work, name, where, comp,
			   n_comp, flag, proc ) < 0 )
        {
	    GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
