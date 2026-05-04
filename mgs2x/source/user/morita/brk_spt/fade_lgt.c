//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	set_t_fc.c
	カメラに平行にテクスチャセット（クロスフェード）
	1999/11/09 S.Okajima
	$Id: fade_lgt.c,v 1.1.1.3 2002/11/19 11:45:46 Yoshizawa1 Exp $

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

extern int ok_flush_status;
extern float ok_focus_z_far;

#define	DUMMY_Z			(5000.0f)
#define	RATIO_MAX		(64.0f)
#define	RATIO_MAX_CORE	(128.0f)

#define	RGB_0			(40)
#define	RGB_1			(100)
#define	RGB_CORE		(120)

/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR		(8000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR		(-700.0f)

#define	R_POS		(1)
#define	R_UVS		(2)
#define	R_DAT		( R_POS +   R_UVS + R_UVS + R_UVS )

#define N_SET		(3)
#define N_PRIMS2	(1)
#define N_VERTS2	(16)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVS0		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS1		(SCRPAD_ADDR + 0x2000)
#define	SCR_UVS_CORE	(SCRPAD_ADDR + 0x3000)

//#define	RAISE_SHIFT		(120.0f)
#define	RAISE_SHIFT		(110.0f)

/*----------------------------------------------------------------*/
typedef	struct	{
    GV_ACT	actor ;
    int		name;
    int		where;

    FVECTOR	center ;

    DG_PRIM2	*prim0 ;
    DG_PRIM2	*prim1 ;
    DG_PRIM2	*prim_core ;
    float	zoom_max;

    float	radius0 ;
    float	radius1 ;
    float	radius_core ;

    int		flag ;     /*  11/26 /1999  Revised by S.Okajima */
} Work ;
/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, int alpha )
{
    FVECTOR          *pos0, *pos1 ;
    DG_PRIM2_UVRGBWH *urw0, *urw1 ;	/* スプライト用 */
    int i ;

    DG_ConfigPrim2Tex( prim, tex ) ;
    DG_SetPrim2Alpha( prim, alpha ) ;

    pos0 = prim->pos[0] ;
    pos1 = prim->pos[1] ;
    urw0 = prim->uvrgb[0] ;
    urw1 = prim->uvrgb[1] ;
    for ( i=N_PRIMS2 ; --i>=0 ; pos0++, pos1++, urw0++, urw1++ )
    {
	_sceVu0CopyVector( pos0, center ) ;
	_sceVu0CopyVector( pos1, center ) ;

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
	urw0->u0 = urw1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	urw0->v0 = urw1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	urw0->u1 = urw1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	urw0->v1 = urw1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	urw0->q0 = urw1->q0 = 4096 ;
	urw0->q1 = urw1->q1 = 4096 ;
	urw0->f0 = urw1->f0 = 0x0fff ;
	urw0->f1 = urw1->f1 = 0x0fff ;

	//urw0->w = 1000*cosf( (float)M_PI*(i*13+k*20)/180.0f ) ;/* L/2*cos(ang) */
	//urw0->h = 1000*sinf( (float)M_PI*(i*13+k*20)/180.0f ) ;/* L/2*sin(ang) */
	urw0->w = urw1->w = 0 ;
	urw0->h = urw1->h = 0 ;
		          
	urw0->r = urw1->r = 0 ;
	urw0->g = urw1->g = 0 ;
	urw0->b = urw1->b = 0 ;
	urw0->a = urw1->a = 0 ;
    }

    return 1;
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
    FVECTOR	raise_shift;
    FVECTOR	fvcal;
    float	change_start;
    float	change_end;
    float	len;
    float	zoom;
    float	radius0_x;
    float	radius0_y;
    float	radius1_x;
    float	radius1_y;
    float	cf,sf;
    int	 clock ;
    float	ratio;
    DG_PRIM2_UVRGBWH	*uvrgbwh0		;	/* スプライト用 */
    DG_PRIM2_UVRGBWH	*uvrgbwh1		;	/* スプライト用 */
    DG_PRIM2_UVRGBWH	*uvrgbwh_core	;	/* スプライト用 */

    if( ok_flush_status!=0 ){
	DG_InvisiblePrim2( work->prim0 ) ;
	DG_InvisiblePrim2( work->prim1 ) ;
	DG_InvisiblePrim2( work->prim_core ) ;
	return;
    }else{
	DG_VisiblePrim2( work->prim0 ) ;
	DG_VisiblePrim2( work->prim1 ) ;
	DG_VisiblePrim2( work->prim_core ) ;
    }

    if( ok_focus_z_far!=0.0f )
    {
	change_start = ok_focus_z_far - FADE_RANGE_NEAR ;
	if(change_start < 1.0f) change_start = 1.0f ;
	change_end   = ok_focus_z_far + FADE_RANGE_FAR ;
    }else{
	/* キャラ：Ｚ制御が起動していないとき */
	change_start = DUMMY_Z - FADE_RANGE_NEAR ;
	if(change_start < 1.0f) change_start = 1.0f ;
	change_end   = DUMMY_Z + FADE_RANGE_FAR ;
    }

    DG_SetPos( &DG_Chanls->eye ) ;
    raise_shift.vx = 0.0f ;
    raise_shift.vy = 0.0f ;
    raise_shift.vz = -RAISE_SHIFT ;
    DG_RotVector( &raise_shift, &raise_shift, 1 ) ;

    fvcal.vx=0.0f ;
    fvcal.vy=0.0f ;
    fvcal.vz=1.0f ;
    DG_SetPos( &DG_Chanls[0].eye_inv ) ;
    DG_RotVector( &fvcal, &fvcal, 1 ) ;

    len=GV_VecLen3F( (FVECTOR *)DG_Chanls[0].eye.m[3] );	/* 対象はなんでもいい？ */
    cf = cosf( len/2048.0f + fvcal.vz ) ;
    sf = sinf( len/2048.0f + fvcal.vz ) ;
    radius0_x = work->radius0 * cf ;
    radius0_y = work->radius0 * sf ;
    radius1_x = work->radius1 * cf ;
    radius1_y = work->radius1 * sf ;

    DG_SwitchBuffPrim2( work->prim0 ) ;
    DG_SwitchBuffPrim2( work->prim1 ) ;
    DG_SwitchBuffPrim2( work->prim_core ) ;
    clock = work->prim0->buffer_clock ;
    uvrgbwh0     = work->prim0->uvrgb[clock]     ;
    uvrgbwh1     = work->prim1->uvrgb[clock]     ;
    uvrgbwh_core = work->prim_core->uvrgb[clock] ;

    _sceVu0SubVector( &fvcal, &work->center, DG_Chanls[0].eye.m[3] ) ;
    _sceVu0AddVector( work->prim0->pos[clock], &work->center, &raise_shift ) ;
    _sceVu0CopyVector( work->prim1->pos[clock], work->prim0->pos[clock] ) ;

    len = GV_VecLen3F( &fvcal ) ;
    if ( len > change_end )
	ratio = RATIO_MAX ;
    else if (len > change_start )
	ratio = (len - change_start)*(float)RATIO_MAX/(change_end - change_start) ;
    else
	ratio = 0.0f ;		/* フラグとしても使用 */

    zoom = work->zoom_max + (1.0f-work->zoom_max) * (RATIO_MAX - ratio) / RATIO_MAX ;
    uvrgbwh0->w = (int)(radius0_x * zoom) ;
    uvrgbwh0->h = (int)(radius0_y * zoom) ;
    uvrgbwh0->r = uvrgbwh0->g = uvrgbwh0->b = RGB_0 ;

    uvrgbwh1->w = (int)(radius1_x * zoom) ;
    uvrgbwh1->h = (int)(radius1_y * zoom) ;
    uvrgbwh1->r = uvrgbwh1->g = uvrgbwh1->b = RGB_1 ;

    uvrgbwh_core->w = uvrgbwh_core->h = (int)(work->radius_core * zoom);
    uvrgbwh_core->r = uvrgbwh_core->g = uvrgbwh_core->b = RGB_CORE;

    if ( work->flag )
    {
	int a ;

	a = (int)(ratio*0.8f) ;
	uvrgbwh0->a     = a<0 ? 0 : a>128 ? 128 : a ;
	a = (int)(RATIO_MAX-ratio) ;
	uvrgbwh1->a     = a<0 ? 0 : a>128 ? 128 : a ;
	a = (int)(RATIO_MAX_CORE*(RATIO_MAX-ratio)/RATIO_MAX) ;
	uvrgbwh_core->a = a<0 ? 0 : a>128 ? 128 : a ;
    }
    else
    {
	uvrgbwh0->a     = 0 ;
	uvrgbwh1->a     = 0 ;
	uvrgbwh_core->a = 0 ;
    }


	if( len < 10000.0f ){
		FVECTOR fvtemp;
	    _sceVu0SubVector( &fvcal, DG_Chanls[0].eye.m[3], &work->center ) ;
		_sceVu0Normalize( &fvcal, &fvcal );
		_sceVu0ScaleVector( &fvtemp, &fvcal, 150.0f );
		_sceVu0AddVector( &fvtemp, &work->center, &fvtemp );
		_sceVu0ScaleVector( &fvcal, &fvcal, 1000.0f );
		_sceVu0AddVector( &fvcal, &work->center, &fvcal );

		if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID,
				&fvtemp,
				&fvcal,
			    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
			    HZX_SEG_RECOIL_TYPE , HZX_FLOOR_RECOIL_TYPE ) ) {
			uvrgbwh0->a     = 0 ;
			uvrgbwh1->a     = 0 ;
			uvrgbwh_core->a = 0 ;
		}
	}


}

static void Die( Work *work )
{
    if ( work->prim0 )
	GM_FreePrim2( work->prim0 ) ;
    if ( work->prim1 )
	GM_FreePrim2( work->prim1 ) ;
    if ( work->prim_core )
	GM_FreePrim2( work->prim_core ) ;
}


static int GetResources( Work *work, int name, int where,
			 int id_t, int id_u, int id_v, int alpha,
			 float radius0, float radius1, float radius_core,
			 FVECTOR *center, float zoom_max, int prim_flag )
{
   // JM(AS) - Force a certain light off in w03a, as it causes an odd flicker
   // when trying to open a door. MGSTWO-3027
   if ( !strcmp( "w03a", GM_GetArea() ) )
   {
      switch ( name )
      {
      case 0x0070ab56:
         return -1;

   #if 0
      // AS(JM) - These are the other lights in the area.
      // Leaving them in here in case we have to disable more.  I 
      // just use a binary search (manually) to find the right light
      case 0x000c0523:
      case 0x0027d608:
      case 0x003e09f0:
      case 0x003e09f2:
      case 0x003e09f3:
      case 0x003e09f4:
      case 0x003e09f6:
      case 0x003e09f7:
      case 0x003e09f8:
      case 0x0070ab56:
      case 0x0070ab57:
      case 0x0070ab58:
      case 0x00906464:
      case 0x00906465:
      case 0x00906466:
      case 0x00906467:
      case 0x00a06464:
      case 0x00a06465:
      case 0x00a06466:
      case 0x00a06467:
      case 0x00a06468:
      case 0x00b06464:
      case 0x00b06465:
      case 0x00b06466:
   #endif
      default:
         break;
      }
   }

   work->name  = name  ;
    work->where = where ;

    work->radius0     = radius0 * 2.0f ;
    work->radius1     = radius1 * 2.0f ;
    work->radius_core = radius_core    ;

    work->center = *center ;
    work->flag = 1 ;
    work->zoom_max = zoom_max<=0.0f ? 1.0f : zoom_max/4096.0f ;

    /* 回転スプライト */
    if ( !(work->prim0 = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|prim_flag, N_PRIMS2, 1 )) )
    {
	printf("null prim\n") ;
	return -1 ;
    }
    InitPacket2( work, work->prim0, DG_GetTexture( id_t ), &work->center, alpha ) ;

    /* 回転スプライト */
    if ( !(work->prim1 = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|prim_flag, N_PRIMS2, 1 )) )
    {
	printf("null prim\n") ;
	return -1 ;
    }
    InitPacket2( work, work->prim1, DG_GetTexture( id_u ), &work->center, alpha ) ;

    /* 固定スプライト */
    if ( !(work->prim_core = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|prim_flag, N_PRIMS2, 1 )) )
    {
	printf("null prim\n") ;
	return -1 ;
    }
    InitPacket2( work, work->prim_core, DG_GetTexture( id_v ), &work->center, alpha ) ;

    return 0 ;
}

void *NewPutTexFadeOnLight( int name, int where,
			    int id_t, int id_u, int id_v, int alpha,
			    float radius0, float radius1, float radius_core,
			    FVECTOR *center, float zoom_max, int prim_flag )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL )
    {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	if ( GetResources( work, name, where, id_t, id_u, id_v, alpha,
			   radius0, radius1, radius_core,  center, zoom_max, prim_flag  ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}

