//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	glass_mist2.c

	ガラス、鏡がくもるエフェクト(ポリゴン貼りつけ部分)
	
	2001/07/07 Y.YANO
	$Id: glass_mist2.c,v 1.1.1.3 2002/11/19 11:51:57 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include	"def_dma.h"
#include	"utl_dma.h"

#include 	"../etc/yn_utl.h"
#include 	"glass_mist.h"


#define N_PRIMS	(1)                /*パケット数*/
#define N_VERTS	(64)               /*頂点数*/
#define N_UNIT  (4)                /*四角形を作るため*/
#define N_POLY	(N_VERTS / N_UNIT) /*四角形の数*/

#define WIDTH   (600.0F)
#define HEIGHT  (600.0F)

#define ALPHA_MAX (80)

typedef	struct	{
    GV_ACT_EX	actor ;
    DG_PRIM2	*prim ;

	MainWork	*main_work ;
	DG_TEX      *tex ;
	int called_cnt ;
	int called_num ;
	int called_flag ;
	
	int alpha_mode[N_POLY] ;
	float f_alpha[N_POLY] ;
	int alpha_cnt[N_POLY] ;
	int act_cnt ;

} Work ;

static void InitUV( Work *work , DG_PRIM2_UVRGB *uvrgb , DG_TEX	*tex , FVECTOR *cut )
{
	short stemp ;
	float ftemp , tab ;
	SVECTOR  rot ;
	FMATRIX  fmatrix ;
	FVECTOR  fake , fvtemp ;
	MainWork *mwork = work->main_work ;
	float right , left , upper , bottom ;

	/* 鏡の法線から、rot を求める */
	rot.vx = 0 ;
	rot.vz = 0 ;
	ftemp = atan2f( mwork->perpen.vx , mwork->perpen.vz );
	rot.vy = 0x0fff & (short)(( ftemp * 2048.0F / PI ) + 0.5F );/*四捨五入*/

	/* 逆行列を求めて、画面に正対した鏡で考える */
	fvtemp = DG_ZeroVector ;
	DG_SetPos2( &fvtemp , &rot );
	DG_GetPos( &fmatrix );
	_sceVu0InversMatrix( &fmatrix , &fmatrix );

	DG_SetPos( &fmatrix );
	_sceVu0SubVector( &fvtemp , &mwork->glass_point , &mwork->center );
	DG_PutVector( &fvtemp , &fake , 1 );
	_sceVu0AddVector( &fake , &fake , &mwork->center );
	fake.vw = 1.0F ;
	
	/*右*/
	tab = (mwork->center.vx + mwork->width) - fake.vx ;
	if( tab < WIDTH / 2 ){
		right = ( WIDTH / 2 + tab ) / WIDTH ;
	}else{
		right = 1.0F ;
	}
	/*左*/
	tab = fake.vx - (mwork->center.vx - mwork->width) ;
	if( tab < WIDTH / 2 ){
		left = ( WIDTH / 2 - tab ) / WIDTH ;
	}else{
		left = 0.0F ;
	}
	/*上*/
	tab = mwork->center.vy + mwork->height - fake.vy ;
	if( tab < HEIGHT / 2 ){
		upper = ( HEIGHT / 2 - tab ) / HEIGHT ;
	}else{
		upper = 0.0F ;
	}
	/*下*/
	tab = fake.vy - (mwork->center.vy - mwork->height) ;
	if( tab < HEIGHT / 2 ){
		bottom = ( HEIGHT / 2 + tab ) / HEIGHT ;
	}else{
		bottom = 1.0F ;
	}

	/* uv回転 */
	stemp = RND(4);
	switch( stemp ){
	  case 0:
		stemp = 0;
		break;
	  case 1:
		stemp = 1024;
		break;
	  case 2:
		stemp = 2048;
		break;
	  case 3:
		stemp = 3072;
		break;
	  default:
		stemp = 0;
		break;
	}
	rot.vx = 0 ;
    rot.vy = stemp & 0x0fff ;
    rot.vz = 0 ;
	fvtemp.vx = left - 0.5F ;
	fvtemp.vy = 0.0F ;
	fvtemp.vz = upper - 0.5F ;
	fvtemp.vw = 1.0F ;
    DG_SetPos2( &DG_ZeroVector , &rot );
    DG_RotVector( &fvtemp , &fvtemp , 1 );
	fvtemp.vx += 0.5F ;
	fvtemp.vz += 0.5F ;
	uvrgb[0].u = FTOI12( fvtemp.vx * tex->u_scale + tex->u_offset );
	uvrgb[0].v = FTOI12( fvtemp.vz * tex->v_scale + tex->v_offset );

	fvtemp.vx = right - 0.5F ;
	fvtemp.vy = 0.0F ;
	fvtemp.vz = upper - 0.5F ;
	///fvtemp.vw = 1.0F - 0.5F ;
	fvtemp.vw = 1.0F;
    DG_SetPos2( &DG_ZeroVector , &rot );
    DG_RotVector( &fvtemp , &fvtemp , 1 );
	fvtemp.vx += 0.5F ;
	fvtemp.vz += 0.5F ;
	uvrgb[1].u = FTOI12( fvtemp.vx * tex->u_scale + tex->u_offset );
	uvrgb[1].v = FTOI12( fvtemp.vz * tex->v_scale + tex->v_offset );

	fvtemp.vx = left - 0.5F ;
	fvtemp.vy = 0.0F ;
	fvtemp.vz = bottom - 0.5F ;
	fvtemp.vw = 1.0F ;
    DG_SetPos2( &DG_ZeroVector , &rot );
    DG_RotVector( &fvtemp , &fvtemp , 1 );
	fvtemp.vx += 0.5F ;
	fvtemp.vz += 0.5F ;
	uvrgb[2].u = FTOI12( fvtemp.vx  * tex->u_scale + tex->u_offset );
	uvrgb[2].v = FTOI12( fvtemp.vz * tex->v_scale + tex->v_offset );

	fvtemp.vx = right - 0.5F ;
	fvtemp.vy = 0.0F ;
	fvtemp.vz = bottom - 0.5F ;
	///fvtemp.vw = 1.0F - 0.5F ;
	fvtemp.vw = 1.0F;
    DG_SetPos2( &DG_ZeroVector , &rot );
    DG_RotVector( &fvtemp , &fvtemp , 1 );
	fvtemp.vx += 0.5F ;
	fvtemp.vz += 0.5F ;
	uvrgb[3].u = FTOI12( fvtemp.vx * tex->u_scale + tex->u_offset );
	uvrgb[3].v = FTOI12( fvtemp.vz * tex->v_scale + tex->v_offset );

	cut->vx = left ;
	cut->vy = right ;
	cut->vz = upper ;
	cut->vw = bottom ;
}

static void InitPolygon( Work *work )
{
	int k ;
	int clock ;
	DG_TEX          *tex ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB 	*uvrgb /*, *last_uvrgb */;
	FVECTOR cut ;
	
	work->alpha_mode[work->called_num] = 0 ;
	
	clock = work->prim->buffer_clock ;
	tex   = work->tex ;
	pos   = (FVECTOR*)work->prim->pos[ clock ] + work->called_num * N_UNIT ;
	uvrgb = (DG_PRIM2_UVRGB*)work->prim->uvrgb[ clock ] + work->called_num * N_UNIT ;
	//last_uvrgb = (DG_PRIM2_UVRGB*)work->prim->uvrgb[ 1-clock ] + work->called_num * N_UNIT ;
	/* uv */
	InitUV( work , uvrgb , tex , &cut );
	for( k = 0 ; k < 4 ; k++ ){
		uvrgb[k].a = 0 ;/*最初は透明*/
		//last_uvrgb[k].a = 0 ;/*念のため*/
	}
	/* pos */
	pos[0].vx = work->main_work->glass_point.vx
		- (WIDTH*(1-cut.vx)-WIDTH/2) * work->main_work->perpen.vz ;
	pos[0].vy = work->main_work->glass_point.vy + (HEIGHT*(1-cut.vz)-HEIGHT/2) ;
	pos[0].vz = work->main_work->glass_point.vz
		+ (WIDTH*(1-cut.vx)-WIDTH/2) * work->main_work->perpen.vx ;
	pos[0].vw = 1.0F ;
	pos[1].vx = work->main_work->glass_point.vx
		+ (WIDTH*cut.vy-WIDTH/2) * work->main_work->perpen.vz ;
	pos[1].vy = work->main_work->glass_point.vy + (HEIGHT*(1-cut.vz)-HEIGHT/2) ;
	pos[1].vz = work->main_work->glass_point.vz
		- (WIDTH*cut.vy-WIDTH/2) * work->main_work->perpen.vx ;
	pos[1].vw = 1.0F ;
	pos[2].vx = work->main_work->glass_point.vx
		- (WIDTH*(1-cut.vx)-WIDTH/2) * work->main_work->perpen.vz ;
	pos[2].vy = work->main_work->glass_point.vy - (HEIGHT*cut.vw-HEIGHT/2) ;
	pos[2].vz = work->main_work->glass_point.vz
		+ (WIDTH*(1-cut.vx)-WIDTH/2) * work->main_work->perpen.vx ;
	pos[2].vw = 1.0F ;
	pos[3].vx = work->main_work->glass_point.vx
		+ (WIDTH*cut.vy-WIDTH/2) * work->main_work->perpen.vz ;
	pos[3].vy = work->main_work->glass_point.vy - (HEIGHT*cut.vw-HEIGHT/2) ;
	pos[3].vz = work->main_work->glass_point.vz
		- (WIDTH*cut.vy-WIDTH/2) * work->main_work->perpen.vx ;
	pos[3].vw = 1.0F ;

	memcpy( (FVECTOR*)work->prim->pos[ 1-clock ] + work->called_num * N_UNIT ,
			(FVECTOR*)work->prim->pos[ clock ] + work->called_num * N_UNIT ,
			sizeof(FVECTOR) * N_UNIT ) ;
	memcpy( (DG_PRIM2_UVRGB*)work->prim->uvrgb[ 1-clock ] + work->called_num * N_UNIT ,
			(DG_PRIM2_UVRGB*)work->prim->uvrgb[ clock ] + work->called_num * N_UNIT ,
			sizeof(DG_PRIM2_UVRGB) * N_UNIT ) ;
}

static void Act( Work *work )
{
	int temp = 0 ;
	int i , j , k ;
	int            clock ;
	//MainWork       *mwork = work->main_work ;
	DG_PRIM2_UVRGB *uvrgb , *last_uvrgb ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

	/*変数初期化*/
	clock = work->prim->buffer_clock ;
	uvrgb = work->prim->uvrgb[ clock ];
	last_uvrgb = work->prim->uvrgb[ 1 - clock ];
	
	for( i = 0 ; i < N_PRIMS ; i++ ){
		for( j = 0 ; j < N_POLY ; j++ ){
			if( work->called_flag & (1 << j) ){
				if( work->alpha_mode[j] == 0 ){/* a 増加 */
					for( k = 0 ; k < N_UNIT ; k++ ){
						//uvrgb[k].a = last_uvrgb[k].a + 1 ;
						if(k == 0){
							work->f_alpha[j] += 1.0F ;
						}
						uvrgb[k].a = (u_char)work->f_alpha[j] ;
						if( uvrgb[3].a > ALPHA_MAX ){
								//work->alpha_mode[j] = 1 ;
							work->alpha_mode[j] = 2 ;
							uvrgb[0].a = ALPHA_MAX ;
							uvrgb[1].a = ALPHA_MAX ;
							uvrgb[2].a = ALPHA_MAX ;
							uvrgb[3].a = ALPHA_MAX ;
							last_uvrgb[0].a = ALPHA_MAX ;
							last_uvrgb[1].a = ALPHA_MAX ;
							last_uvrgb[2].a = ALPHA_MAX ;
							last_uvrgb[3].a = ALPHA_MAX ;
						}
					}
				}else if( work->alpha_mode[j] == 1 ){
					if( (work->act_cnt % N_POLY) == j ){/*順繰りにalpha減らす*/
						for( k = 0 ; k < N_UNIT ; k++ ){
							if(k == 0){
								//work->f_alpha[j] -= 2.0F ;
								work->f_alpha[j] -= 0.5F ;
								temp = (int)work->f_alpha[j] ;
								if( temp < 0 ){
									temp = 0 ;
								}
								work->alpha_cnt[j]++ ;
							}
							uvrgb[k].a = (u_char)temp ;
							if( k == 3 ){
								if( work->alpha_cnt[j] >= 30 ){
									work->alpha_mode[j] = 2 ;
									work->alpha_cnt[j] = 0 ;
								}
							}
						}
					}
				}else if( work->alpha_mode[j] == 2 ){/* a 減少 */
					if( work->act_cnt % N_POLY == j ){/*順繰りにalpha減らす*/
						for( k = 0 ; k < N_UNIT ; k++ ){
							if(k == 0){
								work->f_alpha[j] -= 1.7F ;
								temp = (int)work->f_alpha[j] ;
								if( temp < 0 ){
									temp = 0 ;
								}
							}
							uvrgb[k].a = (u_char)temp ;
							if( uvrgb[3].a <= 0 ){
								work->alpha_mode[j] = 0 ;
								uvrgb[0].a = 0 ;/*透明化*/
								uvrgb[1].a = 0 ;/*      */
								uvrgb[2].a = 0 ;/*      */
								uvrgb[3].a = 0 ;/*      */
								last_uvrgb[0].a = 0 ;
								last_uvrgb[1].a = 0 ;
								last_uvrgb[2].a = 0 ;
								last_uvrgb[3].a = 0 ;
								work->called_flag &= ( ~(1 << j) );/*OFF*/
								break ;
							}
						}
					}else{
						for( k = 0 ; k < N_UNIT ; k++ ){
							uvrgb[k].a = last_uvrgb[k].a ;
						}
					}
				}else{
					printf("glass_mist2.c mode ERR\n");
				}
			}
			uvrgb += N_UNIT ;
			last_uvrgb += N_UNIT ;
		}
	}
#ifdef KP_XBOX
	/* サーマルゴーグルでみている時だけ減算に変更 */
	if( GM_Item == IT_Thermal ){
		work->prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 2 , 0 , 0 , 1 , 0x00 ) ;
	} else {
		work->prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0 , 1 , 0 , 1 , 0x00 ) ;
	}
#endif

	/* 親から呼ばれてポリゴンの初期化 */
	if( work->called_cnt % 60 == 0 ){/* 何回かに1回 */
		work->called_cnt ++ ;/*このif文が永遠に呼び出され続けないように防止*/
		work->called_num ++ ;
		if( work->called_num >= N_POLY ){
			work->called_num = 0 ;
		}
		work->called_flag |= ( 1 << work->called_num );
		InitPolygon( work );
	}
	/* 終了処理 */
	if( work->called_flag == 0 ){/*すべてOFF*/
		GV_DestroyActor( work );
	}

	work->act_cnt ++ ;/* Actのカウンター */
}

static void Die( Work *work )
{
	work->main_work->child_call &= ( ~0x01 );
	GM_FreePrim2( work->prim );
}


static void InitPacket( Work *work )
{
	int i , j , k ;
	DG_PRIM2_UVRGB 	*uvrgb ;

	uvrgb = work->prim->uvrgb[ 0 ];
	/* qf , rgba */
	for( i = 0 ; i < N_PRIMS ; i++ ){
		for( j = 0 ; j < N_POLY ; j++ ){
			uvrgb[0].q = 4096 ;
			uvrgb[0].f = 0x8fff ;
			uvrgb[1].q = 4096 ;
			uvrgb[1].f = 0x8fff ;
			uvrgb[2].q = 4096 ;
			uvrgb[2].f = 0x0fff ;
			uvrgb[3].q = 4096 ;
			uvrgb[3].f = 0x0fff ;
		    for( k = 0 ; k < N_UNIT ; k++ ){
				uvrgb[k].r = 128 ;
				uvrgb[k].g = 128 ;
				uvrgb[k].b = 128 ;
				uvrgb[k].a = 0 ;/*最初は透明*/
			}

			uvrgb += N_UNIT ;
		}
	}
}

static int ReceiveSignal( void *workp , int signal, int value )
{
	Work *work = workp ;
	
	/* KILL フラグ処理 */
	GV_DefaultSignalFunc( work , signal, value );

	/* 呼び出され処理 */
	switch( signal ){
	  case 0x02 :
		work->called_cnt += 1 ;
		break ;
		
	  default:
		break ;
	}
	return 0 ;
	
}
static int GetResources( Work *work )
{
	DG_TEX		*tex ;

	/* 親子関係 */
	GV_SetActorSignalFunc( work, ReceiveSignal );
	work->called_cnt = 1 ;
	work->called_num = 0 ;
	work->called_flag |= 0x01 ;

	/* テクスチャ取得 */
	work->tex = tex = DG_GetTexture( 248058  /*frost02_alp*/ );
	if( work->tex == NULL ){
		return -1 ;
	}
	
	/* プリミティブ初期化 */
	work->prim = GM_MakePrim2(  DG_PRIM2_POLY
							  | DG_PRIM2_SHADE
							  | DG_PRIM2_TEX
							  | DG_PRIM2_ALPHA
							  , N_PRIMS , N_VERTS );
	if( work->prim == NULL ){
		return -1 ;
	}
	DG_ConfigPrim2Tex( work->prim, tex );
	work->prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0 , 1 , 0 , 1 , 0x00 ) ;

	/* パケットの初期化 */
	InitPacket( work );
	/* 一枚目のポリゴンの初期化 */
	InitPolygon( work );
	/*バッファーの[1]にも同じものを入れておく*/	
	memcpy( work->prim->uvrgb[ 1 ] , work->prim->uvrgb[ 0 ] ,
			sizeof(DG_PRIM2_UVRGB) * N_VERTS * N_PRIMS ) ;
	
    return (0) ;
}


void *NewGlassMistPaste( MainWork *main_work )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->main_work = main_work ;/*呼び出し側のワークへのポインタ*/
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

