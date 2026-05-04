//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cam_slater.c
	カメラに飛んでくるフナムシ

	2001/06/11 T.Shibata
	
	$Id: cam_slater.c,v 1.1.1.3 2002/11/19 11:48:48 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"../util/ts_util.h"

#define CLOCK_COUNT	(BP_BASE_TICK())

#define FABS(_x) ({float _a = _x;asm("abs.s %0,%1":"=f"(_a):"f"(_a));(_a);})
#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );
//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

typedef struct {
	u_char	pow ;
	u_char	len ;
} SCRIPT_PARAM ;


typedef	struct {
	GV_ACT_EX		actor;
	int				name;
	int				map;
	int				step;
	DG_OBJS			*objs;

	float			rot;
	float			speed;
	float			add_rot;
	int				timer;
	int				wait_time;
	FVECTOR			shift;
	FVECTOR			now;
	FVECTOR			from;
	FVECTOR			z_vec_ratio;
	FMATRIX			lights[2];
} Work ;

static int MaxCamSlater = 0;

static inline void _RotVector( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddz.xyzw			vf8, vf6,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVector(in, out, 1 ) ;
#endif
}

static inline void _RotTrans( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	lqc2				vf7,0x30(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddaz.xyzw		ACC, vf6,vf8
	vmaddw.xyzw			vf8, vf7,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) :"memory" );
#else
//	DG_SetPos( world ) ;
//	DG_PutVector(in, out, 1 ) ;
	_sceVu0ApplyMatrix( out, world, in );
#endif
}

static inline float _Vu0VecLen2XYZ( FVECTOR *vec1, FVECTOR *vec2 )
{
	float	ans;

#ifdef BP_PSX2_ASM
	asm volatile ("
 	 	  lqc2			vf12, 0x00(%1)
 	 	  lqc2			vf13, 0x00(%2)
		  
		  vsub.xyzw		vf14, vf12, vf13
		  
		  vmul.xyz		vf15, vf14, vf14
		  vmulax.w		ACC, vf0, vf15x
		  vmadday.w		ACC, vf0, vf15y
		  vmaddz.w		vf12, vf0, vf15z

		  vaddw.x		vf13, vf0, vf12

		  qmfc2.i		$8,vf13
		  sw			$8,0(%0)
	": : "r"(&ans), "r"(vec1), "r"(vec2) : "$8", "memory" );
#else
	FVECTOR tmp ;
	_sceVu0SubVector( &tmp, vec1, vec2 );
	_sceVu0MulVector( &tmp, &tmp, &tmp );
	ans = tmp.vx + tmp.vy + tmp.vz ;
#endif

	return (ans);
}


#define	SPEED	(100.0f)
#define	ROT_X	(PI/8.0f)
#define	ROT_Z	(PI/360.0f)

#define SCALE_SSLT	(0.6f)

extern float GM_WaterLevel;

#pragma optimize("",off)

static void Act( Work *work )
{
#if 1
	//FVECTOR shift = { 0.0f, -40.0f, 150.0f, 1.0f };
	FVECTOR to,fvtemp;
	float	len;

	if( GM_CheckPlayerStatus( PLAYER_WATCH ) ){
		_sceVu0SubVector( &fvtemp, &work->from, (FVECTOR*)DG_Chanls[0].eye.m[3] );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		len = _sceVu0InnerProduct( (FVECTOR*)DG_Chanls[0].eye.m[2], &fvtemp );
//printf("cam_slater.c ######### 番号 %d,work->step %d, len %f\n",MaxCamSlater,work->step,len);
		if( work->step < 2 && len < 0.707106f ){
			//動かした
			GV_DestroyActor( work );
			return;
		}else{
			_RotTrans( &to, &DG_Chanls[0].eye, &work->shift );
		}
	}else{
		GV_DestroyActor( work );
		return;
	}

	if( DG_Chanls[0].eye.m[3][1] < GM_WaterLevel ){
		GV_DestroyActor( work );
		return;
	}

	switch( work->step ){
	  case 0:
		_sceVu0SubVector( &fvtemp, &to, &work->now );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		_sceVu0ScaleVector( &fvtemp, &fvtemp, SPEED );
		_sceVu0AddVector( &work->now, &work->now, &fvtemp );
		work->rot += ROT_X;
		if( work->rot > PI ) work->rot -= 2.0f*PI;

		if( (len = _Vu0VecLen2XYZ( &work->now, &to )) < 1000.0f * 1000.0f ){
			len = bp_sqrtf( len );  //BP_MATH - emulate PS2 sqrtf
			work->speed = len/31.0f;
			work->timer = 0;
			//if( work->rot < 0.0f ) len = work->rot + PI*2.0f;
			work->add_rot = (work->rot+PI*2.0f)/31.0f;
			work->step++;
		}
		break;
	  case 1:
		_sceVu0SubVector( &fvtemp, &to, &work->now );
		_sceVu0Normalize( &fvtemp, &fvtemp );
		_sceVu0ScaleVector( &fvtemp, &fvtemp, work->speed );
		_sceVu0AddVector( &work->now, &work->now, &fvtemp );
		work->rot += work->add_rot;
		if( work->rot > PI ) work->rot -= 2.0f*PI;
		else if( work->rot < -PI ) work->rot += 2.0f*PI;
		if( ++work->timer > 30 ){
			work->now = to;
			work->rot = 0.0f;
			work->step++;
			work->timer = 0;
			{
			
			// 舟虫の死骸がカメラ付着
			extern void* NewWormCorpseCamera( FVECTOR* 	pvecCenter,	// 中心
											  int		nBodyTexCode,	// 本体
											  int		nSpreadTexCode,	// 飛抹
											  u_int	unBodyRGBA,	// 本体RGBA
											  u_int	unSpreadRGBA,	// 飛抹RGBA
											  float	fSizeBase,	// サイズベース
											  float	fSizeAdd,	// サイズ加算分
											  int 		nLife );		// 表示時間

			NewWormCorpseCamera( &work->now,
								GV_StrCode("bombgas6_alp"),
								GV_StrCode("blood_1bw_msk"),
								0xa0a0a040, 0x70707040, 4.0f, 0.3f, -1);

			
			
			}
			//SD_A_FNACAM01 //船虫カメラにベチッ
			GM_SeSetMode( SD_A_FNACAM01, &work->now, GM_SEMODE_BOMB );
			_RotTrans( &work->now, &DG_Chanls[0].eye_inv, &work->now );
			work->add_rot = (ROT_Z+ROT_Z*rnd()) * ((irnd()&0x0010000)?-1.0f:1.0f);
			work->wait_time = 40 + irnd()%40;
#if 1
			{
				extern void *NewPadVibration( char *script, int type );
#ifdef BP_PS2
				static SCRIPT_PARAM h_param[2] = {
					{
						.pow = 128,
						.len = 1,
					},
					{
						.pow = 0,
						.len = 0,
					},
				};
				static SCRIPT_PARAM l_param[2] = {
					{
						.pow = 128,
						.len = 6,
					},
					{
						.pow = 0,
						.len = 0,
					},
				};
#else
				static SCRIPT_PARAM h_param[2] = {{128,1},{0,0}} ;
				static SCRIPT_PARAM l_param[2] = {{128,6},{0,0}} ;
#endif

				NewPadVibration( (unsigned char*)h_param , 1 );
				NewPadVibration( (unsigned char*)l_param , 2 );
			}
#endif

		}
		break;
	  case 2:
		if( ++work->timer > work->wait_time ){
			work->step++;
			work->timer = 0;
			//GV_DestroyActor( work );
		}
		break;
	  case 3:
		work->rot += work->add_rot;
		if( work->rot > PI ) work->rot -= 2.0f*PI;
		else if( work->rot < -PI ) work->rot += 2.0f*PI;
		work->now.vy += rnd();
		if( ++work->timer > 12 ){
			work->step++;
			work->timer = 0;
			//GV_DestroyActor( work );
		}
		break;
	  case 4:
		
		break;
	}


	//マトリックスセット
	if( work->step < 2 ){
		FMATRIX		world;
		DG_COPY_VEC( world.m[1], DG_UnitMatrix.m[2] );//DG_Chanls[0].eye.m[2] );
		DG_COPY_VEC( world.m[2], &work->z_vec_ratio );
		_sceVu0OuterProduct( (FVECTOR*)world.m[0], (FVECTOR*)world.m[1], (FVECTOR*)world.m[2] );
//		world.m[0][3] = 0.0f;//yano add 2002.03.04
		_sceVu0RotMatrixX( &world, &world, work->rot );
		_sceVu0MulMatrix( &work->objs->world, &DG_Chanls[0].eye, &world );
		
		DG_COPY_VEC( (FVECTOR*)work->objs->world.m[3], &work->now );
	}else{
		FMATRIX		world;
		DG_COPY_VEC( world.m[1], DG_UnitMatrix.m[2] );
		DG_COPY_VEC( world.m[2], &work->z_vec_ratio );
		_sceVu0OuterProduct( (FVECTOR*)world.m[0], (FVECTOR*)world.m[1], (FVECTOR*)world.m[2] );
//		world.m[0][3] = 0.0f;//yano add 2002.03.04
		_sceVu0RotMatrixZ( &world, &world, work->rot );
		_sceVu0MulMatrix( &work->objs->world, &DG_Chanls[0].eye, &world );

		_RotTrans( (FVECTOR*)work->objs->world.m[3], &DG_Chanls[0].eye, &work->now );
	}
	_sceVu0ScaleVector( (FVECTOR*)work->objs->world.m[0], (FVECTOR*)work->objs->world.m[0], SCALE_SSLT );
	_sceVu0ScaleVector( (FVECTOR*)work->objs->world.m[1], (FVECTOR*)work->objs->world.m[1], SCALE_SSLT );
	_sceVu0ScaleVector( (FVECTOR*)work->objs->world.m[2], (FVECTOR*)work->objs->world.m[2], SCALE_SSLT );
	

//	printf("step %d\n",work->step);
//	PRINT_PFVEC(0,(FVECTOR*)work->objs->world.m[0]);
//	PRINT_PFVEC(1,(FVECTOR*)work->objs->world.m[1]);
//	PRINT_PFVEC(2,(FVECTOR*)work->objs->world.m[2]);
//	PRINT_PFVEC(3,(FVECTOR*)work->objs->world.m[3]);
	
#else
	FMATRIX		world;
	FVECTOR shift = { 0.0f, -40.0f, 150.0f, 1.0f };
	work->rot += ROT_X;

	if( work->rot > PI ) work->rot -= 2.0f*PI;

	//work->z_vec_ratio
	
	DG_COPY_VEC( world.m[1], DG_UnitMatrix.m[2] );//DG_Chanls[0].eye.m[2] );
	DG_COPY_VEC( world.m[2], &work->z_vec_ratio );
	_sceVu0OuterProduct( (FVECTOR*)world.m[0], (FVECTOR*)world.m[1], (FVECTOR*)world.m[2] );
	_sceVu0RotMatrixX( &world, &world, work->rot );
	_sceVu0MulMatrix( &work->objs->world, &DG_Chanls[0].eye, &world );
	_RotTrans( (FVECTOR*)work->objs->world.m[3], &DG_Chanls[0].eye, &shift );
#endif
}

#pragma optimize("",on)


static void Die( Work *work )
{
	if( work->objs ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
	MaxCamSlater--;
}

static DG_OBJS *InitItemObjs( int code, FMATRIX *root, FMATRIX *light )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( code, 'k' ) ) ;
	if(!def){ printf("ERR!! NO MODEL!!\n"); return NULL; }
	objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
	if(!objs) return NULL;
	if(DG_QueueObjs( objs ) < 0) return NULL;
	if(root) objs->root = root;	
	if(light) DG_SetLightMatrix( objs, light );

	return objs;
}

static int GetResources( Work *work )
{
	//int		step;

	work->objs = InitItemObjs( 79849, NULL, work->lights );
	if( !work->objs ) return -1;
	GM_GroupObjs( work->objs, work->map );

	work->z_vec_ratio.vx = frnd();
	work->z_vec_ratio.vy = frnd();
	work->z_vec_ratio.vz = 0.0f;
	work->z_vec_ratio.vw = 0.0f;

	work->shift.vx = 60.0f * frnd();
	work->shift.vy = 40.0f * frnd();
	work->shift.vz = 150.0f;
	work->shift.vw = 1.0f;
	
	_sceVu0Normalize( &work->z_vec_ratio, &work->z_vec_ratio );

//	DG_COPY_VEC( work->objs->world.m[0], &DG_Chanls[0].eye.m[1] );
//	DG_COPY_VEC( work->objs->world.m[1], &DG_Chanls[0].eye.m[2] );
//	DG_COPY_VEC( work->objs->world.m[2], &DG_Chanls[0].eye.m[0] );
//	DG_COPY_VEC( work->objs->world.m[3], &DG_ZeroVector );

	work->lights[1].m[3][0] = 128.0f;
	work->lights[1].m[3][1] = 128.0f;
	work->lights[1].m[3][2] = 128.0f;

	//NewDbugSprite( &work->now, 50.0f );
	return 0;
}

void *NewCameraSlater( int name, int map, FVECTOR *from )
{
	Work		*work ;

	if( MaxCamSlater > 64 ) return NULL;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		work->map = map;
		DG_COPY_VEC( &work->from, from );
		DG_COPY_VEC( &work->now, from );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		MaxCamSlater++;
		//printf("フナ虫発射\n");
	}
	
	return (void *)work ;
}
