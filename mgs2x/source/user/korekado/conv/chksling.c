//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shksling.c
	チョーカースリング

	2001/3/19 Y.Korekado
	$Id: chksling.c,v 1.1.1.3 2002/11/19 11:44:02 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewChalkerSling( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-r	向き

----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif
#include <string.h>

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"

#include "bp_matrix.h"

/*----------------------------------------------------------------*/
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

#define PRIO	0x60	/* 敵兵より後に処理 */

#define N_PRIMS	(1)		/* 1枚 */
#define N_VERTS (4)		/* 四角ポリゴン */

#define SLING_OBJ_A	(12500428)	/* sel_sling1 */
#define SLING_OBJ_B	(12500429)	/* sel_sling2 */
#define SLING_TEX	(12500430)	/* sel_sling3 */
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT		sling_a ;
	OBJECT		sling_b ;
	DG_PRIM2		*prim ;
	OBJECT		*body ;
	FMATRIX		*mune, *wp ;
	FMATRIX		*lights ;

	FVECTOR		tex_pos[ N_PRIMS*N_VERTS ] ;
	FVECTOR		tex_sft[4] ;

	FVECTOR		obj_a_sft ;
	SVECTOR		obj_a_rot ;
	FVECTOR		obj_b_sft ;
	SVECTOR		obj_b_rot ;

	int			count ;
} Work ;

/*----------------------------------------------------------------*/
static void MakePos ( world, sft, pos )
FMATRIX	*world ;
FVECTOR	*sft ;
FVECTOR	*pos ;
{
	DG_SetPos( world ) ;
	DG_PutVector( sft, pos, 1 ) ;	/* 絶対座標へ変換 */
}

static void MoveObj ( obj, world, sft, rot )
OBJECT	*obj ;
FMATRIX	*world ;
FVECTOR	*sft ;
SVECTOR	*rot ;
{
	DG_SetPos( world ) ;
	DG_MovePos( sft ) ;
	DG_RotatePos( rot ) ;
	DG_PutObjs( obj->objs ) ;
}

static void SetLights( DG_PRIM2 *prim, FMATRIX *lights )
{
#if 0
#else
	extern	void	Big_CalcRGB( UCVECTOR *rgb, FMATRIX *lights, FVECTOR *norm) ;
	DG_PRIM2_UVRGB	*uvrgb ;
	UCVECTOR rgb ;
	int i, j ;

	Big_CalcRGB( &rgb, lights, NULL ) ;

	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;
	for ( i=0; i<N_PRIMS; i++ ) {
		for ( j=0; j<N_VERTS; j++ ) {
			uvrgb->r = rgb.r ;
			uvrgb->g = rgb.g ;
			uvrgb->b = rgb.b ;
			uvrgb ++ ;
		}
	}
#endif
}

static	void	SetPosSring( work )
Work	*work ;
{
	FMATRIX	mat ;
	FVECTOR	sling_route_pos, vec ;

	/* スリングの根元位置 */
	DG_SetPos( work->mune ) ;
	DG_MovePos( &work->obj_a_sft ) ;
	DG_GetPos( &mat ) ;
	KR_FMatToFvec( &mat, &sling_route_pos ) ;

	/* 銃に向っての角度 */
	_sceVu0SubVector( &vec, &sling_route_pos, &work->tex_pos[2] ) ;
	_FVecToRotXY( &vec, &work->obj_a_rot ) ;
//	printf("vx=%d\n",work->obj_a_rot.vx ) ;

//	work->obj_a_rot.vx = dir_x ;

	/*A スリング1 胴体 obj2 からのシフト */
//	MoveObj( &work->sling_a, work->mune, &work->obj_a_sft, &work->obj_a_rot ) ;
	DG_SetPos2( &sling_route_pos, &work->obj_a_rot ) ;
	DG_PutObjs( work->sling_a.objs ) ;

	/*B スリング2 スリング1からのシフト */
	MoveObj( &work->sling_b, &(work->sling_a.objs->world), &work->obj_b_sft, &work->obj_b_rot ) ;

	/*C ベルト スリング2からのシフト */
	MakePos( &(work->sling_b.objs->world), &work->tex_sft[0], &work->tex_pos[0] ) ;
	MakePos( &(work->sling_b.objs->world), &work->tex_sft[1], &work->tex_pos[1] ) ;

	/*D 銃前 銃モデルからのシフト */
	MakePos( work->wp, &work->tex_sft[2], &work->tex_pos[2] ) ;
	MakePos( work->wp, &work->tex_sft[3], &work->tex_pos[3] ) ;

	memcpy( work->prim->pos[work->prim->buffer_clock], &work->tex_pos[0], sizeof(FVECTOR) * N_PRIMS*N_VERTS ) ;
}

static	void	SetInvisible( work )
Work	*work ;
{
	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ) {
		work->sling_a.objs->flag |= DG_FLAG_INVISIBLE ;
		work->sling_b.objs->flag |= DG_FLAG_INVISIBLE ;
		work->prim->flag |= DG_PRIM2_INVISIBLE ;
	} else {
		work->sling_a.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->sling_b.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->prim->flag &= ~DG_PRIM2_INVISIBLE ;
	}
	KR_GroupObject( &work->sling_a, work->body->map_name ) ;
	KR_GroupObject( &work->sling_b, work->body->map_name ) ;
	GM_GroupPrim2( work->prim, work->body->map_name ) ;
}


/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	SetInvisible( work ) ;

	DG_SwitchBuffPrim2( work->prim );
	SetPosSring( work ) ;
	SetLights( work->prim, work->lights ) ;

#if 0
if ( !(GV_Time & 0x80) ) {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[0], 3 );
	} else {
		AN_Test_Eye2( &work->pos[1], 3 );
	}
} else {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[2], 3 );
	} else {
		AN_Test_Eye2( &work->pos[3], 3 );
	}
}

if ( GV_Time & 0x80 ) {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[4], 3 );
	} else {
		AN_Test_Eye2( &work->pos[5], 3 );
	}
} else {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[6], 3 );
	} else {
		AN_Test_Eye2( &work->pos[7], 3 );
	}
}
#endif

}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &(work->sling_a) ) ;
    GM_FreeObject( &(work->sling_b) ) ;
	GM_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
/* パケットの初期化 */
static void InitPacket( DG_PRIM2 *prim, DG_TEX *tex )
{
	int	i ;
	DG_PRIM2_UVRGB	*uvrgb ;

	uvrgb = SCRPAD_ADDR ;	/* スクラッチパッド利用 */
	for( i=0; i<N_PRIMS; i++ ) {
		/* ＵＶ値は1.3.12の固定小数点なので注意！ */
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;
	}
	KR_MemCopy( prim->uvrgb[ 0 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS ) ;
	KR_MemCopy( prim->uvrgb[ 1 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS ) ;
}

static	int	GetResources( work, body, weapon, obj_a_sft, obj_b_sft, tex_sft, lights )
Work	*work ;
OBJECT		*body, *weapon ;
FVECTOR		*obj_a_sft, *obj_b_sft, *tex_sft ;
FMATRIX		*lights ;
{
	DG_TEX	*tex ;
	DG_PRIM2	*prim ;
	int		i ;

	/* スリングA */
	GM_InitObject( &(work->sling_a), SLING_OBJ_A, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	DG_SetLightMatrix( work->sling_a.objs, body->objs->light );

	/* スリングB */
	GM_InitObject( &(work->sling_b), SLING_OBJ_B, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	DG_SetLightMatrix( work->sling_b.objs, body->objs->light );

	work->body = body ;
	work->mune = &BODYWORLD( body, HUMAN21_MUNE ) ;
	work->wp = &weapon->objs->world ;
	work->lights = lights ;

	work->obj_a_sft = *obj_a_sft ;
	work->obj_a_rot = DG_ZeroSVector ;
	work->obj_b_sft = *obj_b_sft ;
	work->obj_b_rot = DG_ZeroSVector ;

	for ( i = 0 ; i < 4 ; i++ ){
		work->tex_sft[i] = *tex_sft ;
		tex_sft++ ;
	}

	prim = work->prim = GM_MakePrim2( DG_PRIM2_FOG|DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_SHADE, N_PRIMS, N_VERTS );
	tex = DG_GetTexture( SLING_TEX );
	DG_ConfigPrim2Tex( prim, tex );

	InitPacket( prim, tex ) ;

	SetPosSring( work ) ;

	work->count = 0 ;

	return 0 ;
}

void		*NewChalkerSling( body, weapon, obj_a_sft, obj_b_sft, tex_sft, lights )
OBJECT		*body, *weapon ;
FVECTOR		*obj_a_sft, *obj_b_sft, *tex_sft ;
FMATRIX		*lights ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, weapon, obj_a_sft, obj_b_sft, tex_sft, lights ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/*

[更新:Mar  7 22:14:16 2001][Humanタイプ:付属品]
[位置Y:7 ][位置Z:17.6 ][OFS詳細:SKL_CHESTより(単位はcm) ]
シールズM4用スリング
体に接しているパーツです。


[更新:Mar  7 22:31:54 2001][Humanタイプ:付属品]
[位置Y:-10.5 ][OFS詳細:sel_sling1のセンター位置からの距離(単位はcm) ]
シールズM4用スリング
sel_sling1に接しているパーツです。
このsel_sling1のセンターから

X:0,Y:-5.5,zZ:0の距離へと変わります。



[更新:Mar  7 22:42:50 2001][OFS詳細:コメントニ書きます。 ]
のびるテクスチャーのBMPです。
その頂点位置は、

前方A(M4にとって上)が、
このsel_sling2のセンター位置から
X0,Y-4.6,Z1.3

後方B(M4にとって下)が、
X0,Y-4.6,Z-1.3です。

BMPのM4側の頂点位置は
前方A'(M4にとって上)が、
M4のセンター位置から
X4.5,Y-6,Z10.2

後方B'(M4にとって下)が、
X4.5.Y-6,Z7.6です。

--
[更新:Mar  7 22:39:47 2001]
[更新:Mar  7 22:37:09 2001][Humanタイプ:付属品][OFS詳細:コメントに書きます。 ]
sling用のびるテクスチャーのBMPです。
その頂点位置は、
前方A(M4にとって上)が、
このsel_sling2のセンター位置から
X
*/

void	NewBodyChalkerSling( body, weapon, lights )
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	FVECTOR	obj_a_sft, obj_b_sft, tex_sft[4] ;
	
	/* スリング */
/*	A：0.0, 70.0, 176.0
	B：0.0, -105.0, 0.0
	C：0.0, -46.0, 13.0
	 ：0.0, -46.0, -13.0
	D：45.0, -60, 102.0
	 ：45.0, -60, 76.0
*/

	/*A スリング1 胴体 obj2 からのシフト */
	obj_a_sft.vx = 0.0f ; obj_a_sft.vy = 70.0 ; obj_a_sft.vz = 176.0 ;
	/*B スリング2 スリング1からのシフト */
	obj_b_sft.vx = 0.0f ; obj_b_sft.vy = -55.0 ; obj_b_sft.vz = 0.0 ;

	/*C ベルト スリング2からのシフト */
	tex_sft[0].vx = 0.0f ; tex_sft[0].vy = -46.0f ; tex_sft[0].vz = 13.0f ;
	tex_sft[1].vx = 0.0f ; tex_sft[1].vy = -46.0f ; tex_sft[1].vz = -13.0f ;

	/*D 銃前 銃モデルからのシフト */
	tex_sft[2].vx = 45.0 ; tex_sft[2].vy = -60.0 ; tex_sft[2].vz = 102.0 ;
	tex_sft[3].vx = 45.0 ; tex_sft[3].vy = -60.0 ; tex_sft[3].vz = 76.0 ;

	NewChalkerSling( body, weapon, &obj_a_sft, &obj_b_sft, tex_sft, lights ) ;
}

void	NewBodyHitecChalkerSling( ptr, body, weapon, lights )
void		*ptr ;
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	FVECTOR	obj_a_sft, obj_b_sft, tex_sft[4] ;
	
	/* スリング */
/*	A：0.0, 70.0, 176.0
	B：0.0, -105.0, 0.0
	C：0.0, -46.0, 13.0
	 ：0.0, -46.0, -13.0
	D：45.0, -60, 102.0
	 ：45.0, -60, 76.0
*/

	/*A スリング1 胴体 obj2 からのシフト */
	obj_a_sft.vx = 0.0f ; obj_a_sft.vy = 120.0 ; obj_a_sft.vz = 195.0 ;
	/*B スリング2 スリング1からのシフト */
	obj_b_sft.vx = 0.0f ; obj_b_sft.vy = -55.0 ; obj_b_sft.vz = 0.0 ;

	/*C ベルト スリング2からのシフト */
	tex_sft[0].vx = 0.0f ; tex_sft[0].vy = -46.0f ; tex_sft[0].vz = 13.0f ;
	tex_sft[1].vx = 0.0f ; tex_sft[1].vy = -46.0f ; tex_sft[1].vz = -13.0f ;

	/*D 銃前 銃モデルからのシフト */
	tex_sft[2].vx = 45.0 ; tex_sft[2].vy = -60.0 ; tex_sft[2].vz = 102.0 ;
	tex_sft[3].vx = 45.0 ; tex_sft[3].vy = -60.0 ; tex_sft[3].vz = 76.0 ;

	GV_SetActorChild( ptr, NewChalkerSling( body, weapon, &obj_a_sft, &obj_b_sft, tex_sft, lights ) ) ;
}
