//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	glass_mist.c

	ガラス、鏡がくもるエフェクト
	
	2001/07/05 Y.YANO
	$Id: glass_mist.c,v 1.1.1.3 2002/11/19 11:51:57 Yoshizawa1 Exp $

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

#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"utl_dma.h"

#include 	"../etc/yn_utl.h"
#include 	"glass_mist.h"

#define 	BIT_HAVE_COOLSPRAY	(0x0001)

#define 	EFFECT_SCALE		( 1800.0F ) 
#define 	INNER_LIMIT			( -0.7F )/*45度くらい*/
#define		JET_POWER			( 3000.0F )/*噴射距離*/
#define		SCALE_OFFSET		( 500.0F )

extern void *NewGlassMistPaste( MainWork *main_work );
//extern void AN_Test_Eye2( FVECTOR *mov, int size );

static void Act( MainWork *work )
{
	FVECTOR nozzle_pos , dist_vec ;
	float 	distance ;
	FVECTOR unit_vec = { 0.0F , 0.0F , 1.0F , 1.0F };
	float 	inner ;
	float 	length1 , length2 ;
	FVECTOR fvtemp ;
	float 	x1,x2,y1,y2,z1,z2 ;

	GM_SetCurrentMap( work->map ) ;
	if( GM_WeaponFire == WP_ColdSpray ){
		/*噴射ベクトルと鏡の法線の内積で鏡に向いているか検出*/
		DG_SetPos( &DG_Chanls[ 0 ].eye );/*カメラのマトリクス*/
		DG_RotVector( &unit_vec , &work->jet_vec , 1 );/*噴射ベクトル(単位ベクトル)*/
		inner = _sceVu0InnerProduct( &work->perpen , &work->jet_vec );
		inner = - DG_FABS( inner );
		if( inner < INNER_LIMIT ){
			/*噴射ベクトルと鏡との平面の交点を求める*/
			/*ノズルの座標わりだし*/
			if( !(GM_PlayerSubWeaponBody == NULL) ){
			    _sceVu0ApplyMatrix( &nozzle_pos ,
									GM_PlayerSubWeaponBody->objs->root ,
									&work->NozzlePoint );
				/*平面上の点とノズルの座標ベクトルを求める*/
				_sceVu0SubVector( &fvtemp , &work->center , &nozzle_pos );
				/*法線との内積をとって距離を出す*/
				length1 = _sceVu0InnerProduct( &fvtemp , &work->perpen );
				/*噴射ベクトルと法線の内積を出す*/
				length2 = _sceVu0InnerProduct( &work->jet_vec , &work->perpen );
				/*噴射ベクトルを伸ばして平面へ*/
				length1 = length1 / length2 ;
				if( length1 > 0.0F ){/*逆にベクトルを伸ばすのは方向が逆になっているよ*/
					_sceVu0ScaleVector( &fvtemp , &work->jet_vec , length1 );
					_sceVu0AddVector( &work->glass_point , &nozzle_pos , &fvtemp );/*glass_pointが当たった座標*/
					/* 優先間違いを避けるため、2 mm ほど手前に貼り付ける */
					_sceVu0ScaleVector( &fvtemp , &work->perpen , 2.0F );
					_sceVu0AddVector( &work->glass_point , &work->glass_point , &fvtemp );
					work->glass_point.vw = 1.0F ;
					
					
					/*範囲以内にいるかチェック*/
					/*必ずしも範囲に入っていないからといって
					  曇らないわけでないので、少し広めの範囲で考えた方がいいかも*/
					y1 = work->center.vy + work->height + SCALE_OFFSET ;
					y2 = work->center.vy - work->height - SCALE_OFFSET ;
					if( work->glass_point.vy < y1 && work->glass_point.vy > y2 ){
						if( work->perpen.vz >= 0.0F ){
							x1 = work->center.vx + work->width*work->perpen.vz + SCALE_OFFSET ;
							x2 = work->center.vx - work->width*work->perpen.vz - SCALE_OFFSET ; 
						}else{
							x1 = work->center.vx - work->width*work->perpen.vz + SCALE_OFFSET ;
							x2 = work->center.vx + work->width*work->perpen.vz - SCALE_OFFSET ;
						}				
						if( work->glass_point.vx < x1 && work->glass_point.vx > x2 ){
							if( work->perpen.vx >= 0.0F ){
								z1 = work->center.vz + work->width*work->perpen.vx + SCALE_OFFSET ;
								z2 = work->center.vz - work->width*work->perpen.vx - SCALE_OFFSET ;
							}else{
								z1 = work->center.vz - work->width*work->perpen.vx + SCALE_OFFSET ;
								z2 = work->center.vz + work->width*work->perpen.vx - SCALE_OFFSET ;
							}				
							if( work->glass_point.vz < z1 && work->glass_point.vz > z2 ){
								/*距離を求める*/
								_sceVu0SubVector( &dist_vec , &work->glass_point , &nozzle_pos );
								distance = YN_VectorLength( &dist_vec );
								//printf("dist = %f \n", distance );
								if( distance < EFFECT_SCALE ){
									if( !(work->child_call & 0x01) ){/*子の呼び出しは一回だけ
																	   ただし、子が死んだときに
																	   child_call が OFF になる
																	   つまり親に対して子は
																	   いつも一人になるようにする*/
										GV_SetActorChild( work, NewGlassMistPaste( work ) );
										work->child_call |= 0x01 ;
									}else{/*後は、シグナルを送るだけ*/
										int signal = 0x02 ;
										int value = 0 ;
										GV_CallChildSignalFunc( work, signal, value );
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

static void Die( MainWork *work )
{
}


static int GetResources( MainWork *work, int name, int where )
{
    /* シナリオ起動時のオプションを解析 */
    if( GCL_GetOption( 'p' ) == NULL ){
		printf("not misty glass center position \n");
		return (-1) ;
    }else{/*鏡の中心*/
		work->center.vx = GCL_GetNextInt();
		work->center.vy = GCL_GetNextInt();
		work->center.vz = GCL_GetNextInt();
		work->center.vw = 1.0F ;
    }
    if( GCL_GetOption( 'h' ) == NULL ){
		printf("not misty glass height \n");
		return (-1) ;
    }else{/*鏡の高さ*/
		work->height = GCL_GetNextInt();
    }
    if( GCL_GetOption( 'w' ) == NULL ){
		printf("not misty glass width \n");
		return (-1) ;
    }else{/*鏡の幅*/
		work->width = GCL_GetNextInt();
    }
    if( GCL_GetOption( 'x' ) == NULL ){
		printf("not misty glass perpendicular(housen) \n");
		return (-1) ;
    }else{/*鏡の法線*/
		work->perpen.vx = GCL_GetNextInt();
		work->perpen.vy = GCL_GetNextInt();
		work->perpen.vy = 0.0F ;/* Y は無視 (地面に対して垂直な鏡だけを想定) */
		work->perpen.vz = GCL_GetNextInt();
		_sceVu0Normalize( &work->perpen , &work->perpen );/*正規化*/
		work->perpen.vw = 1.0F ;
    }

	/*ワーク変数初期化*/
	work->NozzlePoint.vx = 0.0f ;/*主観腕からノズル迄のベクトル*/
	work->NozzlePoint.vy = 0.0f ;
	work->NozzlePoint.vz = 0.0f ;
	work->NozzlePoint.vw = 1.0f ;

    return 0 ;
}

static int GetResources2(MainWork *work,FVECTOR *center,FVECTOR *housen,
						 float width,float height )
{
    /*鏡の中心*/
	work->center.vx = center->vx ;
	work->center.vy = center->vy ;
	work->center.vz = center->vz ;
	work->center.vw = 1.0F ;
    /*鏡の高さ*/
	work->height = height ;
    /*鏡の幅*/
	work->width = width ;
    /*鏡の法線*/
	work->perpen.vx = housen->vx ;
	work->perpen.vy = housen->vy ;
	work->perpen.vy = 0.0F ;/* Y は無視 (地面に対して垂直な鏡だけを想定) */
	work->perpen.vz = housen->vz ;
	_sceVu0Normalize( &work->perpen , &work->perpen );/*正規化*/
	work->perpen.vw = 1.0F ;
	
	/*ワーク変数初期化*/
	work->NozzlePoint.vx = 0.0f ;/*主観腕からノズル迄のベクトル*/
	work->NozzlePoint.vy = 0.0f ;
	work->NozzlePoint.vz = 0.0f ;
	work->NozzlePoint.vw = 1.0f ;

    return 0 ;
}

void *NewGlassMistMain2( FVECTOR *center,FVECTOR *housen,float width,float height )
{/*割れガラスからの呼びだし用*/
	MainWork		*work ;

	OPERATOR() ;
	work = (MainWork *)GV_NewActor( GV_ACTOR_USER, sizeof( MainWork ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources2(work,center,housen,width,height ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	work->map = GM_CurrentMap ;
	return (void *)work ;
}

void *NewGlassMistMain( int name, int where )
{/*シナリオ呼びだし用*/
	MainWork		*work ;

	OPERATOR() ;
	work = (MainWork *)GV_NewActor( GV_ACTOR_USER, sizeof( MainWork ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	work->map = where ;
	return (void *)work ;
}
