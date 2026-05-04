//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sub_mazzle.c
	主観用まずる -> Sthinger Back Fire
	2001/05/10 S.Kobayashi
	$Id: sub_mazzle.c,v 1.1.1.3 2002/11/19 11:50:38 Yoshizawa1 Exp $

*/
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

#include	"gameheader.h"
#include	"libmt.h"
#include	"../test/etc.h"


#define	MULTIPLE	(4.0f)

#define	GRAVI_RAND_LEN	(3.0f)

#define N_PRIMS		(16)
#define N_VERTS		(1)

#define	RGB_SMOKE		(72)
#define	ALPHA_MIN		(6.0f)
#define	ALPHA_RND		(6.0f)
#define	ALPHA_SUB		( (float)(ALPHA_MIN+ALPHA_RND)/(float)(N_PRIMS*N_VERTS)*MULTIPLE )

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	SIZE_MIN		(4.0f)
#define	SIZE_RND		(16.0f)

#define	ZOOM_ADD		(4.0f)
#define	ANGLE_ADD		(rnd() * 6.f * PI / 360.f)
#define	RAISE_FLUSH	    (1100)
#define SMOKE_FLAG  ( DG_PRIM2_RSPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA )

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

#define WIND_MAX ( 5.f )
/*----------------------------------------------------------------*/
typedef	struct	{
	int			count;
	FVECTOR		speed;
	float		angle;
	float		size;
	float		alpha;
	float       alpha_sub;
	float       sign; // 回転の方向 
	int          deg;
} Unit ;

typedef	struct _work {
	GV_ACT_EX		actor ;

	FMATRIX		*world;
	FVECTOR		shift;
	FVECTOR     power;

	DG_PRIM2	*prim ;
	DG_TEX      *tex;
	int			life;
	int         num;
	int         deg;
    int     angle_patern;
	int         mode;
	float		size_rnd;
	float       patern;
	float       alpha;
	FVECTOR     range;
	Unit		*unit;
	float	   scale_u,scale_v,pic_scale_u,pic_scale_v ;
	void (*act)( struct _work * );
} Work ;

enum {
	SK_MODE_NORMAL = 0 , 
	SK_MODE_RED , 
	SK_MODE_GREEN , 
};
 
extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;

/*----------------------------------------------------------------*/
static void NormalAct( Work *work )
{
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR wind_tmp;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	float offset_u , offset_v;
	int	x , y , i;
	int       tmp;
	int		clock;
	int		count;
	Unit	*unit;

	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	unit = work->unit;
	pos            = work->prim->pos[ clock ];
	pos_before     = work->prim->pos[ 1 - clock ];
	uvrgbwh        = work->prim->uvrgb[ clock ];
	uvrgbwh_before = work->prim->uvrgb[ 1 - clock ];
	count = 0;
	tmp = N_VERTS * work->angle_patern;
	for( i = 0 ; i < tmp ; i++ ){
		if( unit->alpha > 0 ){
			_sceVu0AddVector( pos, pos_before, &unit->speed );
			// 風の影響  
			_sceVu0Normalize( &wind_tmp , &G_wind );
			_sceVu0ScaleVector( &wind_tmp , &wind_tmp , WIND_MAX );
			_sceVu0AddVector( pos , pos , &wind_tmp );
			unit->alpha += (-unit->alpha / ( work->alpha ) );
			unit->size += ( i * 10 ); 
			if ( unit->alpha < 1 ){
				uvrgbwh->a = 0;
				uvrgbwh->w = 0;
				uvrgbwh->h = 0;
				count++;
				unit++;
				pos++;
				pos_before++;
				uvrgbwh++;
				uvrgbwh_before++;
				continue;
			} else {
				// パターンの更新 
				uvrgbwh->pad0 = uvrgbwh_before->pad0 + ( int )work->patern;
				if ( uvrgbwh->pad0 >= 6 ){
					uvrgbwh->pad0 = 0;
				}
				uvrgbwh->pad1 = uvrgbwh->pad0;
				unit->alpha_sub -= 0.2f;
				if ( unit->alpha_sub < 0.4f ){
					unit->alpha_sub = 0.4f;
				}
				{
//					SVECTOR rot;
					float rad;
					int stmp;
					int vz;

					vz = i * 11 * unit->deg;
					stmp = ( vz & 0x800 ? -1 : 1 ) * ( vz & 0x7ff );
					rad = ( float )stmp * PI / 4096.0f;
					uvrgbwh->w = (int)(unit->size * cosf( rad ) );
					uvrgbwh->h = (int)(unit->size * sinf( rad ) );
					unit->deg += i;
					unit->deg %= 4096;

#if 0
					rot.vx = rot.vy = 0;
					rot.vz = i * 11 * unit->deg;
					DG_SetPos2( &DG_ZeroVector , &rot );
					// 次の場所へ 
					DG_RotVector( &unit->speed , &unit->speed , 1 );
#endif
				}
				/* パターン */
				x = uvrgbwh->pad0 % 3;
				y = uvrgbwh->pad0 / 3;
				offset_u = work->tex->u_offset + x * (32.0F * work->pic_scale_u);
				offset_v = work->tex->v_offset + y * (64.0F * work->pic_scale_v);
				uvrgbwh->u0 = FTOI12( offset_u );
				uvrgbwh->v0 = FTOI12( offset_v );
				uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
				uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );
			}
			uvrgbwh->a = (int)unit->alpha;
			uvrgbwh_before->a = (int)unit->alpha;
			_sceVu0DivVector( &unit->speed , &unit->speed , ( rnd() + 1.2f ) );
		}else{
			count++;
		}
		unit++;
		pos++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}
	if( count >= tmp ){
		GV_DestroyActor( work );
	}
	//	work->life--;
}

static void Act( Work *pWork )
{
	pWork->act( pWork );
}

static void Die( Work *pWork )
{
	if ( pWork->unit != NULL ){
		GV_DelayedFree( pWork->unit );
	}
	if ( pWork->prim != NULL ){
		pWork->prim = OK_FreePrim2( pWork->prim );
	}
}

static int InitPacket( Work *pWork, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR *pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FMATRIX		fmtemp;
	Unit		*unit;
	int     tmp;
	int		x , y;
	float	offset_u,offset_v;
	int		i;

	tmp = N_VERTS * pWork->num;
	DG_SetPos( pWork->world );
	DG_MovePos( &pWork->shift ); // 銃口にもっていく 
	DG_GetPos( &fmtemp );        // マトリクスの取得 

	// メモリの確保 
	pWork->unit = GV_Malloc( sizeof( Unit ) * pWork->angle_patern );
	if ( pWork->unit == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( pWork->unit , sizeof( Unit ) * pWork->angle_patern );

	// main
	pos       = MEM_ADDR1 ;
	uvrgbwh   = MEM_ADDR2 ;
	unit      = pWork->unit;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 64 ) );
	prim->raise = RAISE_FLUSH;

	pWork->pic_scale_u = tex->u_scale / 96.0F;
	pWork->pic_scale_v = tex->v_scale / 128.0F;
	pWork->scale_u	 = pWork->pic_scale_u * 32.0F;
	pWork->scale_v	 = pWork->pic_scale_v * 64.0F;
	pWork->patern = 1.0f;
	{
		FVECTOR speed;
		SVECTOR rot;
		short stmp;
		float rad;

		for ( i = 0 ; i < pWork->angle_patern ; i++ ){
			unit->deg = pWork->deg;
			rot.vx = rot.vz = 0;
			rot.vy = i * 11 * unit->deg;
			DG_SetPos2( &DG_ZeroVector , &rot );
			// 次の場所へ 
			_sceVu0ScaleVector( &speed , &pWork->power , frnd() ); // 変化させる  
			DG_RotVector( &speed , &unit->speed , 1 );
			DG_SetPos( pWork->world );
			DG_RotVector( &unit->speed , &unit->speed , 1 );
			_sceVu0AddVector( pos , ( FVECTOR *)fmtemp.m[ 3 ] , &unit->speed );
			unit->size = rnd() * pWork->size_rnd + 200.f;
			uvrgbwh->pad0 = irnd() % 6;  // patern を pad に埋め込む  
			x = uvrgbwh->pad0 % 3 ;
			y = uvrgbwh->pad0 / 3 ;
			offset_u = pWork->tex->u_offset + (float)x * (32.0F * pWork->pic_scale_u) ;
			offset_v = pWork->tex->v_offset + (float)y * (64.0F * pWork->pic_scale_v) ;
			uvrgbwh->u0 = offset_u;
			uvrgbwh->v0 = offset_v;
			uvrgbwh->u1 = pWork->scale_u + offset_u;
			uvrgbwh->v1 = pWork->scale_v + offset_v;
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;
			// 角度算出  
			stmp = ( rot.vy & 0x800 ? -1 : 1 ) * ( rot.vy & 0x7ff );
			rad = ( float )stmp * PI / 4096.0f;
			uvrgbwh->w = (int)( unit->size * cosf( rad ) );
			uvrgbwh->h = (int)( unit->size * sinf( rad ) );
			// 色、etc 
			switch( pWork->mode ){
			case SK_MODE_NORMAL :
				uvrgbwh->r = 16;
				uvrgbwh->g = 12;
				uvrgbwh->b = 0;
				uvrgbwh->a  = 0;
				unit->alpha = 255;
				break;
			case SK_MODE_RED :
				uvrgbwh->a  = 0;
				uvrgbwh->b = uvrgbwh->g = RGB_SMOKE;
				uvrgbwh->r = 255;
				unit->alpha = 64;
				break;
			case SK_MODE_GREEN :
				uvrgbwh->a  = 0;
				uvrgbwh->r = uvrgbwh->b = RGB_SMOKE;
				uvrgbwh->g = 255;
				unit->alpha = 64;
				break;
			}
			pos++;
			unit++;
			uvrgbwh++;
		}
	}
	_MemCopy( prim->pos[ 0 ], MEM_ADDR1, sizeof(FVECTOR), N_VERTS * pWork->angle_patern );
	_MemCopy( prim->pos[ 1 ], MEM_ADDR1, sizeof(FVECTOR), N_VERTS * pWork->angle_patern );
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof( DG_PRIM2_UVRGBWH ), N_VERTS * pWork->angle_patern );
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof( DG_PRIM2_UVRGBWH ), N_VERTS * pWork->angle_patern );

	return 1;
}

#define STR_FRASH (4852090) // muzzlefrash_sev_01_alp
/*----------------------------------------------------------------*/
static int GetResources( Work *pWork )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	/* 煙 */
	tex = pWork->tex = DG_GetTexture( STR_FRASH );

	// 角度の算出 
	pWork->angle_patern = ( 360 / pWork->deg ) + 1;
	if ( pWork->angle_patern <= 0 ){
		pWork->prim = NULL;
		pWork->unit = NULL;
		return ( -1 );
	}
	prim = pWork->prim = GM_MakePrim2( SMOKE_FLAG , pWork->angle_patern , N_VERTS );
	if( prim == NULL ){
	    pWork->unit = NULL;
		return ( -1 );
	}
	if ( InitPacket( pWork, prim, tex ) < 0 ){
		return ( -1 );
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewSubMazzleFlush( FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life , float size_rnd , float alpha , int deg , int mode )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act , Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->world = world;
		DG_COPY_VEC( &pWork->shift, shift );
		DG_COPY_VEC( &pWork->power , pPower );
		pWork->life = life;
		pWork->size_rnd = size_rnd;
		pWork->alpha = alpha;
		pWork->deg = deg;
		pWork->mode = mode;
		pWork->act = ( void * )NormalAct;
		if ( GetResources( pWork ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

void *NewSubMazzleFlushNormal( FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , float alpha , int deg )
{
	return (NewSubMazzleFlush( world , shift , pPower , life , size_rnd , alpha , deg , SK_MODE_NORMAL ) );
}

void *NewSubMazzleFlushRed( FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , float alpha , int deg )
{
	return (NewSubMazzleFlush( world , shift , pPower , life , size_rnd , alpha , deg , SK_MODE_RED ) );
}

void *NewSubMazzleFlushGreen( FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life, float size_rnd , float alpha , int deg )
{
	return (NewSubMazzleFlush( world , shift , pPower , life , size_rnd , alpha , deg , SK_MODE_GREEN ) );
}
