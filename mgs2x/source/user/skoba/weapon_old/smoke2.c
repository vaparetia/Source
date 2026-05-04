//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	smoke2.c
	ポインタ参照先に接続する力積付き煙
	2001/05/26 S.Kobayashi
	$Id: smoke2.c,v 1.1.1.3 2002/11/19 11:50:36 Yoshizawa1 Exp $

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

#include <libutl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../test/etc.h"

#include	"../../okajima/etc/ok_util.h"


#define	MULTIPLE	(4.0f)

#define	GRAVI_RAND_LEN	(3.0f)

#define N_PRIMS		(16)
#define N_VERTS		(16)

#define	RGB_SMOKE		(255)
#define	ALPHA_MIN		(6.0f)
#define	ALPHA_RND		(6.0f)
#define	ALPHA_SUB		( (float)(ALPHA_MIN+ALPHA_RND)/(float)(N_PRIMS*N_VERTS)*MULTIPLE )

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	SIZE_MIN		(12.0f)
#define	SIZE_RND		(16.0f)

#define	ZOOM_ADD		(6.0f)
#define	ANGLE_ADD		(rnd() * 0.4f * PI / 360.f)
#define	RAISE_SMOKE			(1000)
#define SMOKE_FLAG  ( DG_PRIM2_RSPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA )

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

#define WIND_MAX ( 5.f )

/*----------------------------------------------------------------*/

typedef	struct	{
	FVECTOR		vec;
	float		angle;
	float		size;
	float		alpha;
	float       alpha_sub;
	float       sign; // 回転の方向
} Unit ;

typedef struct {
	char flag;
} KILL_LIST;

typedef	struct _work {
	GV_ACT_EX		actor ;

	FMATRIX		*world;
	FVECTOR		shift;
	FVECTOR     power;
	FVECTOR		before_pos;
	FVECTOR     from;
	FVECTOR     to;

	DG_PRIM2	*prim ;
	DG_TEX      *tex;
	int         num;
	float		size_rnd;
	float       alpha;
	float       alpha_div;
	float       patern;
	FVECTOR     range;
	Unit		*unit;//[N_PRIMS*N_VERTS];
	KILL_LIST   *kill_list;
	float	   scale_u,scale_v,pic_scale_u,pic_scale_v ;
    char        *status;
	int         offset;
	FVECTOR     color;

	char         flag;
	void (*act)( struct _work * );
} Work ;

enum {
	SK_MODE_NORMAL = 0 , 
	SK_MODE_SUB = 2 ,
};
 
enum {
	SK_EFFECT_NORMAL ,
	SK_EFFECT_KILL = 0x1,
	SK_EFFECT_STAY = 0x2,	
};

enum {
	SK_EFFECT_KILL_OK = 0x0,
	SK_EFFECT_KILL_NO = 0x1,
};


extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;

extern void *NewTest_Sprite( FVECTOR *pPos , int size ); // 場所をたしかめるのに便利かも 

static int KillSearch( Work *pWork ) // 使用可能な新しい領域の確保？ err - 0xff
{
	int i;

	for ( i = 0 ; i < N_VERTS * pWork->num ; i ++ ){
		if ( pWork->kill_list[ i ].flag == SK_EFFECT_KILL_OK ){
			return ( i );
		}
	}
	return ( 0xffff );
}

static void StoreKillList( Work *pWork , int point )
{
	pWork->kill_list[ point ].flag = SK_EFFECT_KILL_OK;
}

#define SPRITE_PLUS ( 100 )

static void hokan( Work *pWork , FVECTOR *from , FVECTOR *to) // 補完?
{
 	FVECTOR pos;
	FVECTOR speed;
	FVECTOR *tmp_pos;
	DG_PRIM2_UVRGBWH *tmp_uvrgbwh;
	Unit *tmp_unit;
	float tmp2;
	float offset_u , offset_v;
	int	x , y , i;
	int point;
	int clock;

	if ( to->vx + to->vy + to->vz == 0.0f ){
		return;
	}
	clock = pWork->prim->buffer_clock;
	_sceVu0SubVector( &speed , to , from );
	_sceVu0DivVector( &speed , &speed , ( float )SPRITE_PLUS ); // 十枚 
	DG_COPY_VEC( &pos , from );
	for( i = 0 ; i < SPRITE_PLUS ; i ++ ){
		// 追加
		point = KillSearch( pWork );
		if ( point == 0xffff ){
			return;
		}
		tmp_pos = pWork->prim->pos[ clock ];
		tmp_pos += point;
		tmp_unit = &pWork->unit[ point ];
		tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )pWork->prim->uvrgb[ clock ];
		tmp_uvrgbwh += point;
		pWork->kill_list[ point ].flag = SK_EFFECT_KILL_NO;

		DG_COPY_VEC( tmp_pos , &pos );
		_sceVu0AddVector( &pos , &pos , &speed );

		tmp_pos->vx += frnd() * pWork->range.vx;
		tmp_pos->vy += frnd() * pWork->range.vy;
		tmp_pos->vz += frnd() * pWork->range.vz;

		tmp_unit->angle  = frnd() * 60.f * PI / 360.f;
		tmp_unit->sign   = frnd() * 1.5f;
		tmp_unit->size   = SIZE_MIN + pWork->size_rnd * rnd();
		tmp_unit->alpha  = pWork->alpha * rnd();
		tmp_unit->alpha_sub = 2.0f;
		// speed
		tmp2 = frnd();
		tmp2 = tmp2 < 0 ? -tmp2 + 1 : tmp2 + 1 ;
		DG_COPY_VEC( &tmp_unit->vec , &pWork->power );
		_sceVu0ScaleVector( &tmp_unit->vec , &tmp_unit->vec , tmp2 );
		tmp_uvrgbwh->pad0 = irnd() % 30;
		x = tmp_uvrgbwh->pad0 % 8 ;
		y = tmp_uvrgbwh->pad0 / 8 ;
		offset_u = pWork->tex->u_offset + (float)x * (32.0F * pWork->pic_scale_u) ;
		offset_v = pWork->tex->v_offset + (float)y * (64.0F * pWork->pic_scale_v) ;
		tmp_uvrgbwh->u0 = FTOI12( offset_u );
		tmp_uvrgbwh->v0 = FTOI12( offset_v );
		tmp_uvrgbwh->u1 = FTOI12( pWork->scale_u + offset_u );
		tmp_uvrgbwh->v1 = FTOI12( pWork->scale_v + offset_v );
		tmp_uvrgbwh->w = (int)(tmp_unit->size * cosf( tmp_unit->angle ));
		tmp_uvrgbwh->h = (int)(tmp_unit->size * sinf( tmp_unit->angle ));
		tmp_uvrgbwh->a = (int)tmp_unit->alpha;
		tmp_uvrgbwh->r = ( int )pWork->color.vx;
		tmp_uvrgbwh->g = ( int )pWork->color.vy;
		tmp_uvrgbwh->b = ( int )pWork->color.vz;
	}
}

static	void NormalAct( Work *work )
{
	FVECTOR	wind_tmp;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FMATRIX	fmtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	float offset_u , offset_v;
	float tmp2;
	int	x , y , i;
	int       tmp;
	int		clock;
	int		count;
	int     point;
	Unit	*unit;

	DG_VisiblePrim2( work->prim ) ;
// yano
//	DG_InvisiblePrim2Chanl( work->prim, 1 );

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	unit           = work->unit;
	pos            = work->prim->pos[ clock ];
	pos_before     = work->prim->pos[ 1 - clock ];
	uvrgbwh        = work->prim->uvrgb[ clock ];
	uvrgbwh_before = work->prim->uvrgb[ 1 - clock ];

	tmp = N_VERTS * work->num;

	count = 0;
	for( i = 0 ; i < tmp ; i++ ){
		if( unit->alpha > 0 ){
			_sceVu0AddVector( pos, pos_before, &unit->vec );
			// 風の影響
			_sceVu0Normalize( &wind_tmp , &G_wind );
			_sceVu0ScaleVector( &wind_tmp , &wind_tmp , WIND_MAX );
			_sceVu0AddVector( pos , pos , &wind_tmp );
			unit->alpha += ( 0 - unit->alpha ) / ( work->alpha_div );
			if ( unit->alpha < 1 ){
				uvrgbwh->a = 0;
				uvrgbwh->w = 0;
				uvrgbwh->h = 0;
				unit->alpha = 0.0f;
				// kill_listに登録
				StoreKillList( work , i );
				count++;
				unit++;
				pos++;
				pos_before++;
				uvrgbwh++;
				uvrgbwh_before++;
				continue;
			} else if ( unit->vec.vx + unit->vec.vy + unit->vec.vz > 1.0f ){
				// パターンの更新
				uvrgbwh->pad0 = uvrgbwh_before->pad0 + ( int )work->patern;
				if ( uvrgbwh->pad0 >= 30 ){
					uvrgbwh->pad0 = 0;
				}
				uvrgbwh->pad1 = uvrgbwh->pad0;
				unit->alpha_sub -= 0.2f;
				if ( unit->alpha_sub < 0.4f ){
					unit->alpha_sub = 0.4f;
				}
				/* パターン */
				x = uvrgbwh->pad0 % 8;
				y = uvrgbwh->pad0 / 8;
				offset_u = work->tex->u_offset + x * (32.0F * work->pic_scale_u);
				offset_v = work->tex->v_offset + y * (64.0F * work->pic_scale_v);
				uvrgbwh->u0 = FTOI12( offset_u );
				uvrgbwh->v0 = FTOI12( offset_v );
				uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
				uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );
			}
			unit->size += ZOOM_ADD;
			unit->angle += ( ANGLE_ADD * unit->sign );
			if ( unit->angle > 180.f * PI / 360.f ){
				unit->angle = -180.f * PI / 360.f;
			}
			if ( unit->angle < -180.f * PI / 360.f ){
				unit->angle = 180.f * PI / 360.f;
			}
			uvrgbwh->w = (int)(unit->size * cosf( unit->angle ));
			uvrgbwh->h = (int)(unit->size * sinf( unit->angle ));
			uvrgbwh->a = (int)unit->alpha;
			uvrgbwh->r = ( int )work->color.vx;
			uvrgbwh->g = ( int )work->color.vy;
			uvrgbwh->b = ( int )work->color.vz;

			_sceVu0DivVector( &unit->vec , &unit->vec , ( rnd() + 1.2f ) );
		} else {
			uvrgbwh->a = 0;
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			unit->alpha = 0.0f;
			// kill_listに登録
		  	StoreKillList( work , i );
			count++;
		}
		unit++;
		pos++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}
	// 追加
	if ( work->status != NULL && !( *work->status & SK_EFFECT_STAY ) ){ // 待機状態
		DG_SetPos( work->world );
		DG_MovePos( &work->shift );
		DG_GetPos( &fmtemp );

		DG_COPY_VEC( &work->to , &DG_ZeroVector );
		point = KillSearch( work );
		if( !( work->flag & SK_EFFECT_KILL ) && ( point != 0xffff ) ){ // 生成
			{
				FVECTOR *tmp_pos;
				DG_PRIM2_UVRGBWH *tmp_uvrgbwh;
				Unit *tmp_unit;
	
				tmp_pos = work->prim->pos[ clock ];
				tmp_pos += point;
				tmp_unit = &work->unit[ point ];
				tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )work->prim->uvrgb[ clock ];
				tmp_uvrgbwh += point;

				work->kill_list[ point ].flag = SK_EFFECT_KILL_NO;
				DG_COPY_VEC( tmp_pos , (FVECTOR *)fmtemp.m[ 3 ] );

				work->to = *tmp_pos;
				tmp_unit->angle  = frnd() * 60.f * PI / 360.f;
				tmp_unit->sign   = frnd() * 1.5f;
				tmp_unit->size   = SIZE_MIN + work->size_rnd * rnd();
				tmp_unit->alpha  = work->alpha * rnd();
				tmp_unit->alpha_sub = 2.0f;
				// speed
				tmp2 = frnd();
				tmp2 = tmp2 < 0 ? -tmp2 + 1 : tmp2 + 1 ;
				DG_COPY_VEC( &tmp_unit->vec , &work->power );
				_sceVu0ScaleVector( &tmp_unit->vec , &tmp_unit->vec , tmp2 );
				tmp_uvrgbwh->pad0 = tmp_uvrgbwh->pad1 = irnd() % 30;
				x = tmp_uvrgbwh->pad0 % 8 ;
				y = tmp_uvrgbwh->pad0 / 8 ;
				offset_u = work->tex->u_offset + (float)x * (32.0F * work->pic_scale_u) ;
				offset_v = work->tex->v_offset + (float)y * (64.0F * work->pic_scale_v) ;
				tmp_uvrgbwh->u0 = FTOI12( offset_u );
				tmp_uvrgbwh->v0 = FTOI12( offset_v );
				tmp_uvrgbwh->u1 = FTOI12( work->scale_u + offset_u );
				tmp_uvrgbwh->v1 = FTOI12( work->scale_v + offset_v );
				tmp_uvrgbwh->q0 = 4096 ;
				tmp_uvrgbwh->q1 = 4096 ;
				tmp_uvrgbwh->f0 = 0x0fff ;
				tmp_uvrgbwh->f1 = 0x0fff ;
				tmp_uvrgbwh->w = (int)(tmp_unit->size * cosf( tmp_unit->angle ));
				tmp_uvrgbwh->h = (int)(tmp_unit->size * sinf( tmp_unit->angle ));
				tmp_uvrgbwh->a = (int)tmp_unit->alpha;
				tmp_uvrgbwh->r = ( int )work->color.vx;
				tmp_uvrgbwh->g = ( int )work->color.vy;
				tmp_uvrgbwh->b = ( int )work->color.vz;
			}
		}
		hokan( work , &work->from , &work->to );
		DG_COPY_VEC( &work->from , (FVECTOR *)fmtemp.m[ 3 ] ); // バックアップ
	}

	if ( ( work->status == NULL ) || ( *work->status & SK_EFFECT_KILL ) ){
		work->flag = SK_EFFECT_KILL;
	}
	if( ( work->flag & SK_EFFECT_KILL ) && count >= tmp - 1 ){
		GV_DestroyActor( work );
	}
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
	if ( pWork->kill_list != NULL ){
		GV_DelayedFree( pWork->kill_list );
	}
	if ( pWork->prim != NULL ){
		pWork->prim = OK_FreePrim2( pWork->prim );
	}
}

static int InitPacket( Work *pWork, DG_PRIM2 *prim, DG_TEX *tex , int mode )
{
	FVECTOR *pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* rotスプライト用 */
	FMATRIX		fmtemp;
	Unit		*unit;
	int     tmp;
	int		x , y;
	float	offset_u,offset_v;
	int		i , j;
	float   tmp2;

	tmp = N_VERTS * pWork->num;
	DG_SetPos( pWork->world );
	DG_MovePos( &pWork->shift ); // 銃口にもっていく
	DG_GetPos( &fmtemp );        // マトリクスの取得
	DG_COPY_VEC( &pWork->before_pos, (FVECTOR *)fmtemp.m[3] ); // トランスの取得

	// flag 
	pWork->flag = 0;
	// kill_list
	pWork->kill_list = GV_Malloc( sizeof( KILL_LIST ) * tmp );
	if ( pWork->kill_list == NULL ){
		pWork->unit = NULL;
		return ( -1 );
	}
	// unit
	pWork->unit = GV_Malloc( sizeof( Unit ) * tmp );
	if ( pWork->unit == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( pWork->unit , sizeof( Unit ) * tmp );
	GV_ZeroMemory( pWork->kill_list , sizeof( KILL_LIST ) * tmp );
	// 加速度の計算
	unit       = pWork->unit;
	pos        = MEM_ADDR1 ;
	uvrgbwh   = MEM_ADDR1 ;
	DG_ConfigPrim2Tex( prim, tex );
	switch ( mode ){
	case SK_MODE_SUB :
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		prim->raise = RAISE_SMOKE / 2;
		break;
	default : 
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		prim->raise = RAISE_SMOKE;
		break;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * pWork->num * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * pWork->num * N_VERTS );

	GV_ZeroMemory( prim->uvrgb[0], sizeof(DG_PRIM2_UVRGBWH) * pWork->num * N_VERTS );
	GV_ZeroMemory( prim->uvrgb[1], sizeof(DG_PRIM2_UVRGBWH) * pWork->num * N_VERTS );

	DG_COPY_VEC( &pWork->from , ( FVECTOR * )fmtemp.m[ 3 ] );
	pWork->offset = 0;
	pWork->pic_scale_u = tex->u_scale / 255.0F;
	pWork->pic_scale_v = tex->v_scale / 255.0F;
	pWork->scale_u	 = pWork->pic_scale_u * 31.0F;
	pWork->scale_v	 = pWork->pic_scale_v * 63.0F;
	pWork->patern = 3.0f;
	{
		FVECTOR *tmp_pos;
		FVECTOR *tmp_pos_pre;
		int mem_count;
		int trans_count;
		int point;

		mem_count = ( tmp / SCR_FVECTOR_MAX );
		mem_count = mem_count == 0 ? 1 : mem_count;
		trans_count = 0;
		tmp_pos = prim->pos[ 0 ];
		tmp_pos_pre = prim->pos[ 1 ];
		for ( i = 0 ; i < mem_count ; i++ ){
			for ( j = 0 ; j < SCR_FVECTOR_MAX ; j++ ){
				// 加速度の計算
				tmp2 = frnd();
				tmp2 = tmp2 < 0 ? -tmp2 + 1 : tmp2 + 1 ;
				DG_COPY_VEC( &unit->vec , &pWork->power );
				_sceVu0ScaleVector( &unit->vec , &unit->vec , tmp2 );
				DG_COPY_VEC( pos , &pWork->before_pos );
				if ( trans_count < tmp - 1 ){
					trans_count++;
				} else {
					unit++;
					pos++;
					trans_count++;
					break;
				}
				unit++;
				pos++;
			}
			point = ( ( trans_count - 1 ) % SCR_FVECTOR_MAX ) + 1;
			_MemCopy( tmp_pos, MEM_ADDR1, sizeof( FVECTOR ), point  );
			_MemCopy( tmp_pos_pre, MEM_ADDR1, sizeof( FVECTOR ), point  );
			tmp_pos += point;
			tmp_pos_pre += point;
			pos = MEM_ADDR1;
			trans_count = 0;
		}		
	}
	{
		DG_PRIM2_UVRGBWH *tmp_uvrgbwh;
		DG_PRIM2_UVRGBWH *tmp_uvrgbwh_pre;
		int mem_count;
		int trans_count;
		int point;

		mem_count = ( tmp / SCR_UVRGBWH_MAX );
		mem_count = mem_count == 0 ? 1 : mem_count;
		trans_count = 0;
		tmp_uvrgbwh = ( DG_PRIM2_UVRGBWH * )prim->uvrgb[ 0 ];
		tmp_uvrgbwh_pre = ( DG_PRIM2_UVRGBWH * )prim->uvrgb[ 1 ];
		for ( i = 0 ; i < mem_count ; i++ ){
			for ( j = 0 ; j < SCR_UVRGBWH_MAX ; j ++ ){
				uvrgbwh->pad0 = irnd() % 30;
				x = uvrgbwh->pad0 % 8 ;
				y = uvrgbwh->pad0 / 8 ;
				offset_u = pWork->tex->u_offset + (float)x * (32.0F * pWork->pic_scale_u) ;
				offset_v = pWork->tex->v_offset + (float)y * (64.0F * pWork->pic_scale_v) ;
				uvrgbwh->u0 = FTOI12( offset_u );
				uvrgbwh->v0 = FTOI12( offset_v );
				uvrgbwh->u1 = FTOI12( pWork->scale_u + offset_u );
				uvrgbwh->v1 = FTOI12( pWork->scale_v + offset_v );
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->w  = 0;
				uvrgbwh->h  = 0;
				uvrgbwh->r = ( int )pWork->color.vx;
				uvrgbwh->g = ( int )pWork->color.vy;
				uvrgbwh->b = ( int )pWork->color.vz;
				uvrgbwh->a = ( int )pWork->color.vw;
				if ( trans_count < tmp - 1 ){
					trans_count++;
				} else {
					uvrgbwh++;
					trans_count++;
					break;
				}
				uvrgbwh++;
			}
			point = ( ( trans_count - 1 ) % SCR_UVRGBWH_MAX ) + 1;
			_MemCopy( tmp_uvrgbwh, MEM_ADDR1, sizeof(DG_PRIM2_UVRGBWH), point );
			_MemCopy( tmp_uvrgbwh_pre, MEM_ADDR1, sizeof(DG_PRIM2_UVRGBWH), point );
			tmp_uvrgbwh += point;
			tmp_uvrgbwh_pre += point;
			uvrgbwh = MEM_ADDR1;
			trans_count = 0;
		}
	}
	for ( i = 0 ; i < tmp ; i++ ){
		// KillList Initialize
		StoreKillList( pWork , i );
	}

	return 0;
}

#define STR_SMOKE (16238439) // smoke_msk
/*----------------------------------------------------------------*/
static int GetResources( Work *pWork , int mode )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	/* 煙 */
	tex = pWork->tex = DG_GetTexture( STR_SMOKE );

	prim = pWork->prim = GM_MakePrim2( SMOKE_FLAG , pWork->num , N_VERTS );
	if( prim == NULL ){
		pWork->unit = NULL;
		pWork->kill_list = NULL;
		return ( -1 );
	}
	if ( InitPacket( pWork, prim, tex , mode ) < 0 ){
		return ( -1 );
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

static void *New_SK_Smoke2( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , float size_rnd , FVECTOR *range , float alpha , float alpha_div , FVECTOR *pColor , char *status , int mode )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act , Die ) ;
		GV_ActorEX( &pWork->actor );
		DG_SetPos( world );
		DG_RotVector( pPower , &pWork->power , 1 );
		pWork->world = world;
		DG_COPY_VEC( &pWork->shift, shift );
		DG_COPY_VEC( &pWork->color, pColor );
		pWork->range = *range;
		pWork->size_rnd = size_rnd;
		pWork->num = num;
		pWork->alpha = alpha;
		pWork->alpha_div = alpha_div;
		pWork->status = status;
		pWork->act = ( void * )NormalAct;
		if ( GetResources( pWork , mode ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

#define SK_R (255.0f)
#define SK_G (255.0f)
#define SK_B (255.0f)
#define SK_A (0.0f)
void *NewSK_Smoke2Normal( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , float size_rnd , FVECTOR *range , float alpha , float alpha_div , char *status )
{
	FVECTOR default_color = { SK_R , SK_G , SK_B , SK_A };

	return ( New_SK_Smoke2( num , world , shift , pPower , size_rnd , range , alpha , alpha_div , &default_color , status , SK_MODE_NORMAL ) );
}

void *NewSK_Smoke2Sub( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , float size_rnd , FVECTOR *range , float alpha , float alpha_div , char *status )
{
	FVECTOR default_color = { SK_R , SK_G , SK_B , SK_A };

	return ( New_SK_Smoke2( num , world , shift , pPower , size_rnd , range , alpha , alpha_div , &default_color , status , SK_MODE_SUB ) );
}

void *NewSK_Smoke2rgbaSub( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , FVECTOR *pColor , float size_rnd , FVECTOR *range , float alpha , float alpha_div , char *status )
{
	return ( New_SK_Smoke2( num , world , shift , pPower , size_rnd , range , alpha , alpha_div , pColor , status , SK_MODE_SUB ) );
}

void *NewSK_Smoke2rgbaNormal( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , FVECTOR *pColor , float size_rnd , FVECTOR *range , float alpha , float alpha_div , char *status )
{
	return ( New_SK_Smoke2( num , world , shift , pPower , size_rnd , range , alpha , alpha_div , pColor , status , SK_MODE_NORMAL ) );
}
// easy
void NewSK_Easy_Smoke2Normal( FMATRIX *world, FVECTOR *shift, char *status )
{
	FVECTOR default_color = { SK_R , SK_G , SK_B , SK_A };
	FVECTOR  ftmp = { 50.f , 75.f , 50.f , 0.f };
	FVECTOR  power;

	power.vx = 0.0f;
	power.vy = rnd() * 10.0f;
	power.vz = rnd() * 10.0f;
	power.vw = 0.0f;
	
	if ( shift == NULL ){
		New_SK_Smoke2( 16 , world , &DG_ZeroVector , &power , 256 , &ftmp , 4.0f , 4.0f , &default_color , status , SK_MODE_NORMAL );
	} else {
		New_SK_Smoke2( 16 , world , shift , &power , 256 , &ftmp , 14.0f , 4.0f , &default_color , status , SK_MODE_NORMAL );
	}
}
