/*
	db_tex.c
	オブジェクト設置

	1999/10/18 Y.Korekado
	$Id: db_spher.c,v 1.1.1.3 2002/11/19 11:44:26 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewSling( name, where )

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

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"

#include "bp_matrix.h"

/*----------------------------------------------------------------*/
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

#define PRIO	0x60	/* 敵兵より後に処理 */


#define N_PRIMS	(4)		/* ４枚 */
#define N_VERTS (4)		/* 四角ポリゴン */

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT		sling_back ;
	OBJECT		adjust ;
	DG_PRIM2		*prim ;
	FMATRIX		*w1, *w2 ;
	FMATRIX		*lights ;

	FVECTOR		pos[ N_PRIMS*N_VERTS ] ;
	FVECTOR		adj_pos ;
	SVECTOR		adj_rot ;

	FVECTOR		sft1[4] ;
	FVECTOR		sft2[4] ;

	OBJECT		*body ;
	
	int			count ;
} Work ;

static	FVECTOR	WakiTmp[2] ;


/*----------------------------------------------------------------*/
static void MakePos ( world, sft, pos )
FMATRIX	*world ;
FVECTOR	*sft ;
FVECTOR	*pos ;
{
	DG_SetPos( world ) ;
	DG_PutVector( sft, pos, 1 ) ;	/* 絶対座標へ変換 */
}

static void SetLights( DG_PRIM2 *prim, FMATRIX *lights )
{
#if 1
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

static	void	DivVector( vec1, vec2, n )
FVECTOR	*vec1, *vec2 ;
float	n ;
{
	vec1->vx = vec2->vx / n ;
	vec1->vy = vec2->vy / n ;
	vec1->vz = vec2->vz / n ;
}

//static	FVECTOR	SlingWidth = { 23.3, 26.11, 0 } ;
static	FVECTOR	SlingWidth = { 0, 35.0, 0 } ;

static	void	MakeAdjustPos( work )
Work	*work ;
{
	FVECTOR	addpos, vec ;
	FMATRIX	mat ;
	FVECTOR	pos1, pos2 ;

	/* 座標を銃後寄りにする */
	_sceVu0AddVector( &pos1, &work->pos[1], &work->pos[7] ) ; 	/* 銃後 ＋ 銃前 */
	DivVector( &pos1,  &pos1, 2.0 ) ;
	_sceVu0AddVector( &pos2, &work->pos[1], &work->pos[12] ) ; 	/* 銃後 ＋ 横腹 */
	DivVector( &pos2,  &pos2, 2.0 ) ;

	_sceVu0AddVector( &addpos, &work->pos[1], &pos1 ) ; 	/* 銃後 ＋ 銃前 */
	_sceVu0AddVector( &addpos, &addpos, &pos2 ) ;		/* ＋ 横腹 */

	/* アジャスト位置 */
	DivVector( &WakiTmp[0],  &addpos, 3.0 ) ;
	_sceVu0SubVector(  &vec, &WakiTmp[0], &work->pos[12] ) ;
	work->adj_rot.vx = _FVecDirX( &vec ) - 2048 ;
	work->adj_rot.vy = _FVecDir2( &vec ) - 1024 ;
	work->adj_rot.vz = 0 ;
	work->adj_pos = WakiTmp[0] ;

	DG_SetPos2( &work->adj_pos, &work->adj_rot ) ;
	DG_PutObjs( work->adjust.objs ) ;

	DG_MovePos( &SlingWidth ) ;
	DG_GetPos( &mat ) ;
	KR_FMatToFvec( &mat, &WakiTmp[1] ) ;
}

static	void	SetPosSring( work )
Work	*work ;
{
	/* 右肩から銃後ろ */
	MakePos( work->w1, &work->sft1[0], &work->pos[0] ) ;	/* 右肩 */
	MakePos( work->w1, &work->sft1[1], &work->pos[2] ) ;
	MakePos( work->w2, &work->sft2[0], &work->pos[1] ) ;	/* 銃後ろ */
	MakePos( work->w2, &work->sft2[1], &work->pos[3] ) ;

	/* 左胸から銃前 */
	MakePos( work->w2, &work->sft2[2], &work->pos[7] ) ;	/* 銃前 */
	MakePos( work->w2, &work->sft2[3], &work->pos[5] ) ;

	/* 左胸から銃後ろ */
	MakePos( work->w2, &work->sft2[0], &work->pos[9] ) ;	/* 銃後ろ */
	MakePos( work->w2, &work->sft2[1], &work->pos[11] ) ;

	/* 横腹から左胸 */
	MakePos( work->w1, &work->sft1[2], &work->pos[12] ) ;	/* 横腹 */
	MakePos( work->w1, &work->sft1[3], &work->pos[14] ) ;

	/* アジャスト部 */
	MakeAdjustPos( work ) ;
	/* 左胸から銃前 */
	work->pos[6]	=  WakiTmp[0] ;		/* 左胸 */
	work->pos[4]	=  WakiTmp[1] ;
	/* 左胸から銃後ろ */
	work->pos[10]	=  WakiTmp[0] ;		/* 左胸 */
	work->pos[8]	=  WakiTmp[1] ;
	/* 横腹から左胸 */
	work->pos[15]	=  WakiTmp[0] ;		/* 左胸 */
	work->pos[13]	=  WakiTmp[1] ;

	memcpy( work->prim->pos[work->prim->buffer_clock], &work->pos[0], sizeof(FVECTOR) * N_PRIMS*N_VERTS ) ;
}

static	void	SetInvisible( work )
Work	*work ;
{
	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ) {
		work->sling_back.objs->flag |= DG_FLAG_INVISIBLE ;
		work->adjust.objs->flag |= DG_FLAG_INVISIBLE ;
		work->prim->type |= DG_PRIM2_INVISIBLE ;
	} else {
		work->sling_back.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->adjust.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->prim->type &= ~DG_PRIM2_INVISIBLE ;
		GM_GroupPrim2( work->prim, work->body->map_name ) ;
	}
	
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
    GM_FreeObject( &(work->sling_back) ) ;
    GM_FreeObject( &(work->adjust) ) ;
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

static	int	GetResources( work, body, weapon, sft1, sft2, lights, data )
Work	*work ;
OBJECT		*body, *weapon ;
FVECTOR		*sft1, *sft2 ;
FMATRIX		*lights ;
{
	DG_TEX	*tex ;
	DG_PRIM2	*prim ;
	int		i ;

	/* 背中側スリングモデル */
	GM_InitObject( &(work->sling_back), data , DG_FLAG_SHADE|DG_FLAG_ONEPIECE );

	GM_ConfigObjectRoot( &(work->sling_back), body, HUMAN21_MUNE ) ;

	/* アジャスターモデル */
	GM_InitObject( &(work->adjust), GV_StrCode("kanagu") , DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	DG_SetLightMatrix( work->adjust.objs, body->objs->light );
	
	work->body = body ;
	work->w1 = &BODYWORLD( body, HUMAN21_MUNE ) ;
	work->w2 = &weapon->objs->world ;
	work->lights = lights ;
	for ( i = 0 ; i < 4 ; i++ ){
		work->sft1[i] = *sft1 ;
		work->sft2[i] = *sft2 ;
		sft1++ ;
		sft2++ ;
	}

	prim = work->prim = GM_MakePrim2( DG_PRIM2_FOG|DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_SHADE, N_PRIMS, N_VERTS );
	tex = DG_GetTexture( GV_StrCode("gbs_ak_sling2") );	/* ak_sling.tri */
	DG_ConfigPrim2Tex( prim, tex );

	InitPacket( prim, tex ) ;

	SetPosSring( work ) ;

	work->count = 0 ;

	return 0 ;
}

void		*NewSling( body, weapon, sft1, sft2, lights, data )
OBJECT		*body, *weapon ;
FVECTOR		*sft1, *sft2 ;
FMATRIX		*lights ;
int			data ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, weapon, sft1, sft2, lights, data ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

void	NewBodySling( body, weapon, lights )
OBJECT		*body ;
OBJECT		*weapon ;
FMATRIX		*lights ;
{
	FVECTOR	sft1[4], sft2[4] ;
	
	/* スリング */
/*	A：39.5, -55.5, 72.6
	B：39.5, -55.5, 37.6
	C：20.8, -452, 121.4
	D：51.1, -452, 103.9
	*/
	/* 右肩 */
	sft1[0].vx = -89.28 ; sft1[0].vy = 308.22 ; sft1[0].vz = 11.59 ;
	sft1[1].vx = -112.58 ; sft1[1].vy = 282.1 ; sft1[1].vz = 11.59 ;
	/* 左横腹 */
	sft1[2].vx = 205.86 ; sft1[2].vy = 44.79 ; sft1[2].vz = 59.88 ;
	sft1[3].vx = 182.56 ; sft1[3].vy = 18.67 ; sft1[3].vz = 59.88 ;

#if 1
	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 20.8 ; sft2[2].vy = -452.0 ; sft2[2].vz = 121.4 ;
	sft2[3].vx = 51.0 ; sft2[3].vy = -452.0 ; sft2[3].vz = 103.9 ;
#else
	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -34.3 ; sft2[0].vz = 77.3 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -42.5 ; sft2[1].vz = 43.3 ;
	/* 銃前 */
	sft2[2].vx = 20.8 ; sft2[2].vy = -408.2 ; sft2[2].vz = 218.0 ;
	sft2[3].vx = 51.0 ; sft2[3].vy = -412.3 ; sft2[3].vz = 207.0 ;
#endif

	NewSling( body, weapon, sft1, sft2, lights, GV_StrCode("sling") ) ;
}

