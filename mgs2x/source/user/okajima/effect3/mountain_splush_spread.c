//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	mountain_splush_spread.c
	今度こそ沸き上がる水面２
	2001/08/07 S.Okajima
	$Id: mountain_splush_spread.c,v 1.1.1.3 2002/11/19 11:47:33 Yoshizawa1 Exp $

*/


#ifdef PSX2 ///
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

#include <libutl.h>
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../etc/ok_util.h"
#include	"utl_dma.h"

#define RAISE		 (-1000)
#define RAISE_SUB	 (1000)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	COL_R	(80)
#define	COL_G	(80)
#define	COL_B	(80)
#define	COL_A	(64)

//	 4* 4	 3* 8	  24
//	 8* 8	 7*16	 112
//	16*16	15*32	 480
//	32*32	31*64	1984


#define	N_SIDES0			(8)	/*  */
#define	N_SIDES1			(32)	/* 円周分割数 */
#define	N_TOTAL_SIDES	(N_SIDES0 * N_SIDES1)

#define	N_LOOP0		(N_SIDES0 - 1)
#define	N_LOOP1		(2 * N_SIDES1)
#define	N_TOTAL		( N_LOOP0 * N_LOOP1 )

#define	N_VERTS		(64)
#define	N_PRIMS		(N_TOTAL/N_VERTS)

//#define	SCROLL_DIV		(256)
#define	SCROLL_DIV		(128)
#define	DIVISION		(4)
#define	SCROLL_INIT		(SCROLL_DIV/DIVISION)

#define	LIFE		(SCROLL_DIV - SCROLL_INIT)

//#define	MARGINE_RATIO	(0.1f)
#define	MARGINE_RATIO	(0.0f)

#define	MIN_RATIO		(0.3f)
#define	SPREAD_RATIO	(4.0f)

#define	TARGET_HEIGHT	(5000.0f)


#define	CHILD_SPLUSH_NUM	(1)
#define	CHILD_SPLUSH_MIN	(100.0f)
#define	CHILD_SPLUSH_RND	(200.0f)



/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int	OK_PutSplush( FVECTOR *center, FVECTOR *force );
/*----------------------------------------------------------------*/
extern int OK_ListSplushWaveFlag[];
extern FVECTOR OK_ListSplushWavePos[];
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	TARGET		target;
	TARGET		target_child[N_SIDES0] ;

	DG_PRIM2	*prim;
	DG_PRIM2	*prim_sub;

	FVECTOR		center;
	FMATRIX		world;
	FMATRIX		world_sub;

	float		intense;
	float		height;
	float		height_vec;
	float		radius_min;
	float		radius_add;
	float		radius_stp;
	float		offset;
	float		shift_max;
	float		target_width_limit;

	int			scroll_count;
	int			life;
	int			life_max;
	int			flag;
	int			fade_sw;
	int			my_num;
} Work ;

static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkSin[N_SIDES1];
static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkCos[N_SIDES1];

/* ---------------------------------------------------------------- */
static void ChildTargCallBack( TARGET *off, TARGET *def, void *ptr )
{
	Work        *work ;
	FVECTOR		diff;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	FVECTOR		fvtemp2;
	int			i, j;
	float		ftemp;

	work = (Work *)ptr ;
	if ( def->damaged & TARGET_POWER ){
		for( i=0; i<N_SIDES0; i++ ){
			if ( &work->target_child[i] == def  &&  (def->weapon_type & (WP_BULLET|WP_M92)) ){
				if( ( off->class & TARGET_POWER ) && ( off->power != NULL ) ) {

					_sceVu0Normalize( &fvtemp1, &off->power->force );
					_sceVu0SubVector( &diff, &def->hit, &work->center ) ;
					diff.vy = 0.0f;
					_sceVu0Normalize( &diff, &diff );

					for( j=0; j<CHILD_SPLUSH_NUM; j++ ){
						// フォースを適当な大きさにする
						_sceVu0ScaleVector( &fvtemp2, &fvtemp1, (CHILD_SPLUSH_MIN + CHILD_SPLUSH_RND*rnd())*(float)work->life/(float)work->life_max );

						// 半径方向のベクトル
						ftemp = _sceVu0InnerProduct( &diff, &fvtemp2 ) * (1.5f + 1.0f*rnd());
						_sceVu0ScaleVector( &fvtemp0, &diff, ftemp );

						// 半径方向ベクトルに対して反射させる
						_sceVu0AddVector( &fvtemp0, &fvtemp0, &fvtemp2 ) ;
						OK_PutSplush( &def->hit, &fvtemp0 );
					}
/*
AN_Test_Eye2( &def->hit, 2 );
printf("%x:a::::::::::::::%f %f %f\n",ptr,def->hit.vx,def->hit.vy,def->hit.vz);
printf("%x:b::::::::::::::%f %f %f\n",ptr,fvtemp.vx,fvtemp.vy,fvtemp.vz);
*/
				}
			}
		}
//		def->class |= TARGET_POWER;
		GM_ClearTargetDamage(def) ;
	}
}

/* ---------------------------------------------------------------- */
static void CalcBaseVerts( Work *work, int clock )
{
	FVECTOR		*base0;
	FVECTOR		*base1;
	FVECTOR		*latice_pos;
	int		i,j,k;
	float	height;
	float	height1;
	float	ratio;
	float	ratio1;
	float	radius;
	float	radius1;
	float	angle;
	float	*p_sin;
	float	*p_cos;
	TARGET	*target ;
	FVECTOR	size ;
	FVECTOR	mov ;

	work->radius_add+= work->radius_stp;
	work->radius_min+= work->radius_stp;

	target = work->target_child;
	base0   = SCR_TMP;
	base1   = base0;
	base1++;
	for ( i = 0 ; i < N_SIDES0 ; i++ ){
		ratio = (float)i     / (float)(N_SIDES0+1);
		ratio1= (float)(i+1) / (float)(N_SIDES0+1);
		radius = work->radius_add * (1.0f - ratio )
		       + work->radius_min;
		radius1= work->radius_add * (1.0f - ratio1)
		       + work->radius_min;
		latice_pos = SCR_POS;
		p_sin = OK_WS_WorkSin;
		p_cos = OK_WS_WorkCos;

		height =  work->height *ratio *ratio ;
		height1=  work->height *ratio1*ratio1;

		for ( k = 0 ; k < N_SIDES1 ; k++ ){
			angle = TPI * (float)k / (float)(N_SIDES1-1);
			latice_pos->vx = radius * (*(p_sin++));
			latice_pos->vy = height * (1.0f + 0.5f*sinf(angle));
			latice_pos->vz = radius * (*(p_cos++));
			latice_pos++;
		}

		latice_pos = SCR_POS;
		if( i==0 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base0, latice_pos );
				base0+=2;
				latice_pos++;
			}
		}else if( i==N_SIDES0-1 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base1, latice_pos );
				base1+=2;
				latice_pos++;
			}
		}else{
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base0, latice_pos );
				DG_COPY_VEC( base1, latice_pos );
				base0+=2;
				base1+=2;
				latice_pos++;
			}
		}

		radius = (radius + radius1)*0.5f;
		if( radius < work->target_width_limit ){
			size.vx = radius;
			size.vy = (height1 - height)*0.5f;
			size.vz = radius;
			mov.vx  = work->center.vx;
			mov.vy  = GM_WaterLevel + height + size.vy;
			mov.vz  = work->center.vz;
			GM_SetTargetSize( target, &size );
			GM_MoveTargetMap( target, &mov, GM_CurrentStageMap );
//			NewTargetView2( target, 128, 196, 255 );
			GM_TargetResetSkip( target );
		}else{
			GM_TargetSetSkip( target );
		}

		target++;
	}
	OK_Scr_Mem( work->prim->pos[clock], SCR_TMP, sizeof(FVECTOR), N_PRIMS*N_VERTS );




}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim;
	DG_PRIM2		*prim_sub;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		clock;
	int		i,j;
	int		alpha;
	int		flag;
	float	alpha_base;
	float	shift;
	float	ftemp1;

	flag = work->life_max - work->life;

	prim     = work->prim;
	prim_sub = work->prim_sub;

	if( work->flag & 0x01){
		DG_VisiblePrim2( prim );
	}else{
		DG_InvisiblePrim2( prim );
	}
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	if( work->flag & 0x02){
		DG_VisiblePrim2( prim_sub );
	}else{
		DG_InvisiblePrim2( prim_sub );
	}
	GM_GroupPrim2( prim_sub, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim_sub );

//printf("a:%f %f %f\n",prim->pos[clock][0].vx,prim->pos[clock][0].vy,prim->pos[clock][0].vz);
//printf("b:%f %f %f\n",prim_sub->pos[clock][0].vx,prim_sub->pos[clock][0].vy,prim_sub->pos[clock][0].vz);

	flag = 0;
	if( work->height > 0.0f ) flag = 1;
	work->height+= work->height_vec;
	if( work->height > 0.0f ){
		work->height_vec+= P_GRAVITY*2.0f;
		if(!flag ) work->height_vec*= 0.75f;
	}else{
		work->fade_sw = 1;
		work->height_vec-= P_GRAVITY*8.0f;
		if( flag ) work->height_vec*= 0.75f;
	}
//	work->height_vec*= 0.99f;

	CalcBaseVerts( work, clock );

	shift = work->shift_max * (float)(SCROLL_DIV - work->scroll_count) / (float)SCROLL_DIV
	      + work->offset;

	uvrgb0 = prim->uvrgb[clock];
	uvrgb1 = uvrgb0;
	uvrgb1++;

	alpha_base = (float)(COL_A * work->life) / (float)work->life_max;

	for ( i = 0 ; i < N_SIDES0 ; i++ ){

		ftemp1 = work->shift_max / (float)DIVISION * (float)(i  )/(float)N_LOOP0 + shift;
		if( ftemp1 > work->shift_max + work->offset ){
			ftemp1 = work->shift_max + work->offset;
		}else if( ftemp1 < work->offset ){
			ftemp1 = work->offset;
		}
		alpha = (int)(alpha_base * sinf( PI * 0.5f * (float)(i+1) / (float)(N_SIDES0+1) ) ) ;
		if( alpha < 0 ) alpha = 0;

		if( i==0 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0->a = 0;
				uvrgb0->u = FTOI12( ftemp1 );
				uvrgb0+= 2;
			}
		}else if( i==N_SIDES0-1 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb1->a = 0;
				uvrgb1->u = FTOI12( ftemp1 );
				uvrgb1+= 2;
			}
		}else{
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0->a = uvrgb1->a = alpha;
				uvrgb0->u = uvrgb1->u = FTOI12( ftemp1 );
				uvrgb0+= 2;
				uvrgb1+= 2;
			}
		}

	}

//	work->scroll_count++;
	work->scroll_count = SCROLL_DIV;
	if( work->scroll_count > SCROLL_DIV ) work->scroll_count = SCROLL_INIT;

	if( work->fade_sw ){
		work->life-=2;
	}else{
		work->life--;
	}
	if( work->life <= 0) GV_DestroyActor( work ) ;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int	i;

	if( work->my_num != -1 ){
		OK_ListSplushWaveFlag[work->my_num] = 0;
	}

	work->prim_sub = OK_FreePrim2( work->prim_sub );
	work->prim     = OK_FreePrim2( work->prim );
	GM_FreeTarget( &work->target );

	if(&work->target != NULL){
		GM_ClearTargetDamage( &work->target );
		GM_FreeTarget( &work->target ) ;
	}
	for( i=0; i<N_SIDES0; i++ ){
		if(&work->target_child[i] != NULL) GM_FreeTarget( &work->target_child[i] ) ;
	}
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		i, k ;
	float	margine_offset;
	float	margine_scale;
	float	ftemp0;
	float	ftemp1;
	float	ftemp2;
	float	angle;
	float	*p_sin;
	float	*p_cos;

	work->offset    = tex->u_offset;
	work->shift_max = tex->u_scale;


	p_sin = OK_WS_WorkSin;
	p_cos = OK_WS_WorkCos;
	for ( k = 0 ; k < N_SIDES1 ; k++ ){
		angle = TPI * (float)k / (float)(N_SIDES1-1);
		(*p_sin++) = sinf( angle );
		(*p_cos++) = cosf( angle );
	}

	margine_offset = tex->v_scale * MARGINE_RATIO + tex->v_offset;
	margine_scale  = tex->v_scale * (1.0f - MARGINE_RATIO*2.0f);

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	ftemp0 = margine_scale * 2.0f / (float)(N_LOOP1-2);
	for ( i = 0 ; i < N_LOOP0 ; i++ ){
		ftemp1 = tex->u_scale * (float)(i  )/(float)N_LOOP0;
		ftemp2 = tex->u_scale * (float)(i+1)/(float)N_LOOP0;
		for ( k = 0 ; k < N_LOOP1 ; k++ ){
			if( k&1 ){
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp2 + tex->u_offset );
			}else{
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp1 + tex->u_offset );
			}
			uvrgb1->v = uvrgb0->v = FTOI12((float)(k>>1) * ftemp0 + margine_offset );
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = (k==0)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = COL_R;
			uvrgb1->g = uvrgb0->g = COL_G;
			uvrgb1->b = uvrgb0->b = COL_B;
			uvrgb1->a = uvrgb0->a = 0;
			uvrgb0++;
			uvrgb1++;
		}
	}

}

static int GetResources( Work *work, FVECTOR *center, float radius, float intense )
{
	DG_PRIM2	*prim ;
	DG_PRIM2	*prim_sub ;
	DG_TEX		*tex ;
	SVECTOR 	svtemp;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	int	i;
	int	ftemp;
	float	flr_height[2];

	work->fade_sw = 0;

	fvtemp1.vx = center->vx;
	fvtemp1.vy = GM_WaterLevel;
	fvtemp1.vz = center->vz;
	if( HZX_LevelHazardCheck(
			GM_GetHzxGroupID( GM_CurrentStageMap ),
			&fvtemp1,
			HZX_CHK_ALL,
			0 )
		 & 2 ){
		HZX_GetLevelHeight( flr_height );
		ftemp = (flr_height[1]-GM_WaterLevel)*0.5f;
		if( intense > ftemp ) intense = ftemp;
	}




	DG_COPY_VEC( &work->center, center );
	work->center.vy = GM_WaterLevel;
	work->radius_min = radius*MIN_RATIO;
	work->radius_add = radius - work->radius_min;
	work->radius_stp = radius*SPREAD_RATIO*0.25f / (float)LIFE;
	work->intense    = intense;
	work->height     = intense;
	work->height_vec = intense*0.125f;


	work->scroll_count = SCROLL_INIT;
	work->life_max = work->life = LIFE;


	svtemp.vx = 0;
	svtemp.vy = (irnd()>>8)&4095;
	svtemp.vz = 0;
	DG_SetPos2( center, &svtemp );
	DG_GetPos( &work->world );
	work->world.m[3][1] = GM_WaterLevel;
	work->world.m[0][0]*= 0.95f;
	work->world.m[1][1]*= 0.95f;
	work->world.m[2][2]*= 0.95f;

	svtemp.vy+= (((irnd()>>8)&1)*2-1)*(4096/N_SIDES1);
	DG_SetPos2( center, &svtemp );
	DG_GetPos( &work->world_sub );
	work->world_sub.m[3][1] = GM_WaterLevel;

//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 8038630 /*"wave12_alp_ovl"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("mo:null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE;
	prim->root = &work->world;

	prim_sub = work->prim_sub = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_NOBUFFER, N_PRIMS, N_VERTS );
//	prim_sub = work->prim_sub = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim_sub==NULL){
		printf("mo:null prim\n");
		return -1;
	}
	DG_SetPrim2Buffer( prim_sub, prim->pos[0], prim->pos[1], prim->uvrgb[0], prim->uvrgb[1] );
//	InitPacket2( work, prim_sub, tex );

	DG_ConfigPrim2Tex( prim_sub, tex );
	DG_SetPrim2Alpha( prim_sub, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	prim_sub->raise = RAISE_SUB;
	prim_sub->root = &work->world_sub;


	DG_InvisiblePrim2( prim );
	DG_InvisiblePrim2( prim_sub );


	work->target_width_limit = radius*(SPREAD_RATIO+MIN_RATIO) * 0.5f;
	fvtemp0.vx = work->target_width_limit;
	fvtemp0.vy = TARGET_HEIGHT;
	fvtemp0.vz = work->target_width_limit;
	fvtemp1.vx = center->vx;
	fvtemp1.vy = center->vy + TARGET_HEIGHT*0.5f;
	fvtemp1.vz = center->vz;
	GM_SetTarget(
		&work->target,
		TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_CHILD|TARGET_CHILD_ALWAYS,
		GM_CurrentStageMap,
		BOTH_SIDE,
		&fvtemp0,
		&fvtemp1 ) ;
//	GM_SetTargetCallBack( &work->target, TargetCallBack, work ) ;
	GM_PutTarget( &work->target ) ;

	for( i=0; i<N_SIDES0; i++ ){
		GM_SetTarget( &work->target_child[i], TARGET_THROUGH|TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE, 0, BOTH_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( &work->target_child[i], ChildTargCallBack, work ) ;
	}
	GM_SetTargetParts( &work->target, work->target_child, N_SIDES0, 0 ) ;


	for( i=0; i<OK_SPLUSH_MOUNTAIN_MAX; i++ ){
		if( OK_ListSplushWaveFlag[i]==0 ){
			OK_ListSplushWaveFlag[i] = 1;
			DG_COPY_VEC( &OK_ListSplushWavePos[i], &work->center );
			work->my_num = i;
			break;
		}
	}

	return 0 ;
}

/*
flag:0x01 加算ＯＮ
flag:0x02 減算ＯＮ
*/
void *NewWaterSurfaceMountain2( FVECTOR *center, float radius, float intense, int flag )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->flag = flag;
		work->my_num = -1;

		if ( GetResources( work, center, radius, intense ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

FVECTOR *OK_GetSplushWavePos( FVECTOR *pos )
{
	int	i, num;
	int 	*pi;
	float	len_min;
	float	len;
	FVECTOR	*pfv;
	FVECTOR	center;
	FVECTOR	fvtemp;

	DG_COPY_VEC( &center, pos );
	center.vy = GM_WaterLevel;

	num = -1;
	len_min = FLOAT_MAX;
	pi  = OK_ListSplushWaveFlag;
	pfv = OK_ListSplushWavePos;
	for( i=0; i<OK_SPLUSH_MOUNTAIN_MAX; i++ ){
		if( (*pi)!=0 ){
			_sceVu0SubVector( &fvtemp, &center, pfv );
			len = GV_VecLen3F( &fvtemp );
			if( len_min > len ){
				len_min = len;
				num = i;
			}
		}
		pi++;
		pfv++;
	}

	if( num < 0 ) return NULL;

	return &OK_ListSplushWavePos[num] ;
}

