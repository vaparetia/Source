//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  dogtag_flash.c
  ドッグタグのキラキラ
  2001/04/20 Yuuta Kunibe	
  $Id: dogtag_flash.c,v 1.1.1.3 2002/11/19 11:44:37 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"

#define COL_R	(128)
#define COL_G	(128)
#define COL_B	(128)
#define ALPHA	(48)

#define N_PRIMS	(1)

#define	SIZE	(70)

#define	SHIFT_Y (150.0f)
#define	SHIFT_Z (150.f)//(150.0f)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define DOGTAG_INVISIBLE	(0)
#define DOGTAG_VISIBLE		(1)
#define DOGTAG_DESTROY		(-1)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    OBJECT	*object;
    DG_TEX	*tex;
    
    int 	count;
    int		life;

    int		*flag;
    
} Work;


static void Act( Work *work )
{

    
    int	  		clock;
    FVECTOR		*pos;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    FVECTOR		vectmp;


    GM_GroupPrim2( work->prim, work->object->map_name );

    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;


    switch ( *work->flag ) {

    case DOGTAG_INVISIBLE:
		DG_InvisiblePrim2( work->prim );
		work->count = work->life-1;
		break;

    case DOGTAG_VISIBLE:	    
		pos	= work->prim->pos[clock];		
		uvrgbwh = work->prim->uvrgb[clock];    
    
		/* 位置更新 (胸からのオフセット)*/
		DG_COPY_VEC( pos, (FVECTOR *)(&work->object->objs->objs[2].world.m[3]) );
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)(&work->object->objs->objs[2].world.m[1]), SHIFT_Y );
		_sceVu0AddVector( pos, pos, &vectmp );
		_sceVu0ScaleVector( &vectmp, (FVECTOR *)(&work->object->objs->objs[2].world.m[2]), SHIFT_Z );
		_sceVu0AddVector( pos, pos, &vectmp );

		/* ３フレームテクスチャアニメーション */
		if ( work->count < 3 ) {
			DG_VisiblePrim2( work->prim );
		}
		else {
			DG_InvisiblePrim2( work->prim );
		}

		switch ( work->count ) {
		case 0:
			uvrgbwh->u0 = FTOI12( 0.0F * work->tex->u_scale + work->tex->u_offset );
			uvrgbwh->v0 = FTOI12( 0.0F * work->tex->v_scale + work->tex->v_offset );
			uvrgbwh->u1 = FTOI12( 0.5F * work->tex->u_scale + work->tex->u_offset );
			uvrgbwh->v1 = FTOI12( 0.5F * work->tex->v_scale + work->tex->v_offset );
			break;
		case 1:
			uvrgbwh->u0 = FTOI12( 0.5F * work->tex->u_scale + work->tex->u_offset );
			uvrgbwh->v0 = FTOI12( 0.0F * work->tex->v_scale + work->tex->v_offset );
			uvrgbwh->u1 = FTOI12( 1.0F * work->tex->u_scale + work->tex->u_offset );
			uvrgbwh->v1 = FTOI12( 0.5F * work->tex->v_scale + work->tex->v_offset );
			break;
		case 2:
			uvrgbwh->u0 = FTOI12( 0.0F * work->tex->u_scale + work->tex->u_offset );
			uvrgbwh->v0 = FTOI12( 0.5F * work->tex->v_scale + work->tex->v_offset );
			uvrgbwh->u1 = FTOI12( 0.5F * work->tex->u_scale + work->tex->u_offset );
			uvrgbwh->v1 = FTOI12( 1.0F * work->tex->v_scale + work->tex->v_offset );
			break;
		}
    
		if ( ++work->count == work->life ) {
			work->life = DIRECT_TICK( 20 + irnd()%70 );
			work->count = 0;
		}
		break;

    case DOGTAG_DESTROY:
		GV_DestroyActor( work );
		break;

    }
    
}


static void Die(Work *work )
{
    /* メモリ解放 */
    if (work->prim) work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
    
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	GM_GroupPrim2( prim, work->object->map_name );
	
	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;


	DG_COPY_VEC( pos, &DG_ZeroVector );

	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	uvrgbwh->q0 = 4096;
	uvrgbwh->f0 = 0x0fff;
	
	uvrgbwh->u1 = FTOI12( 0.5F * tex->u_scale + tex->u_offset );
	uvrgbwh->v1 = FTOI12( 0.5F * tex->v_scale + tex->v_offset );
	uvrgbwh->q1 = 4096;
	uvrgbwh->f1 = 0x0fff;
	
	uvrgbwh->r  = COL_R;
	uvrgbwh->g  = COL_G;
	uvrgbwh->b  = COL_B;
	uvrgbwh->a  = ALPHA;
	
	uvrgbwh->w  = SIZE;
	uvrgbwh->h  = SIZE;
	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, OBJECT *object, int *flag )
{
    
    DG_PRIM2		*prim;

    work->object = object;
    work->flag = flag;
    work->count  = 0;    
    work->life   = DIRECT_TICK( 20 + irnd()%70 );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
	if ( prim == NULL ) {
		return -1;
	}

    work->tex = DG_GetTexture( GV_StrCode( "kirari_alp_ovl" ) );
    //work->tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, work->tex );    
    
    return 0;
}


/*
  void *NewDogtagFlash( OBJECT *object ) : 呼び出し関数
        OBJECT	*object : ドッグタグをしている対象オブジェクト       
	int	*flag	: 解放フラグ
*/
void *NewDogtagFlash( OBJECT *object, int *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, object, flag ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





