//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	trample_slater.c
	潰れたふなむし
	
	2001/06/06 T.Shibata
	
	$Id: all_slater.c,v 1.1.1.3 2002/11/19 11:48:48 Yoshizawa1 Exp $

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
extern int NewEMA_CommandIsLink() ;
//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )


#include	"sea_slater.h"

AllWork *AllSlater_CurrentWork = NULL;
#if 0
static int CheckMesgParam( AllWork *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num = GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		default:
			break;
		}
		msg--;
	}

	return 0;
}
#endif

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
	_sceVu0ApplyMatrix( out, world, in );
#endif
}

static inline float _Vu0VecLenXYZ( FVECTOR *vec1, FVECTOR *vec2 )
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
	ans = _sceVu0InnerProduct(&tmp,&tmp);
#endif

	return (ans);
}

static inline float _Vu0VecLenXZ( FVECTOR *vec1, FVECTOR *vec2 )
{
	float	ans;

#ifdef BP_PSX2_ASM
	asm volatile ("
 	 	  lqc2			vf12, 0x00(%1)
 	 	  lqc2			vf13, 0x00(%2)
		  
		  vsub.xyzw		vf14, vf12, vf13
		  
		  vmul.xyz		vf15, vf14, vf14
		  vmulax.w		ACC, vf0, vf15x
#		  vmadday.w		ACC, vf0, vf15y
		  vmaddz.w		vf12, vf0, vf15z

		  vaddw.x		vf13, vf0, vf12

		  qmfc2.i		$8,vf13
		  sw			$8,0(%0)
	": : "r"(&ans), "r"(vec1), "r"(vec2) : "$8", "memory" );
#else
	FVECTOR tmp ;

	_sceVu0SubVector( &tmp, vec1, vec2 );
	_sceVu0MulVector( &tmp, &tmp, &tmp );
	ans = tmp.vx + tmp.vz ;
#endif

	return (ans);
}

float GetAllCheckNearPosLen( FVECTOR *out_pos, FVECTOR *out_vec, FVECTOR *in, AllWork *work )
{
	int		i,j;

	float len = FLOAT_MAX, ret = 0.0f;
	for( i = 0; i < work->n_check; i++ ){
		if( work->flags[i] ){
			for( j = 0; j < 4; j++ ){
				if( DG_FABS(work->joint_pos[i][j].vy ) < 1000.0f ){
					float tmp = _Vu0VecLenXYZ( &work->joint_pos[i][j], &DG_ZeroVector );
					if( len > tmp ){
						len = tmp;
						ret = _Vu0VecLenXZ( &work->joint_pos[i][j], &DG_ZeroVector );
						out_pos->vx = work->joint_pos[i][j].vx+work->joint_diff[i][j].vx*8.0f;
						out_pos->vy = work->joint_pos[i][j].vy+work->joint_diff[i][j].vy*8.0f;
						out_pos->vz = work->joint_pos[i][j].vz+work->joint_diff[i][j].vz*8.0f;
						out_pos->vw = 1.0f;

						//DG_COPY_VEC( out_pos, &work->check_pos[i] );
						DG_COPY_VEC( out_vec, &work->joint_diff[i][j] );
					}
				}
			}
		}
	}
	//len = ret;
	for( i = 0; i < N_MAX_BLAST_POS; i++ ){
		if( work->blast_timer[i] >= 0 ){
			float tmp = _Vu0VecLenXYZ( &work->blast_list[i], in );
			if( len > tmp ){
				len = tmp;

				DG_COPY_VEC( out_pos, &work->blast_list[i] );
				DG_COPY_VEC( out_vec, &DG_ZeroVector );
			}
		}
	}
	
	return len;
}
#if 0
//これらはいらないかも
FVECTOR *GetAllCheckSltrTramplePos( AllWork *work )
{
	return work->foot_pos;
}

int GetAllCheckSltrTrampleNum( AllWork *work )
{
	return work->n_foot;
}

FVECTOR *GetAllCheckSltrBulletPos( AllWork *work )
{
	return work->bullet_list;
}

int GetAllCheckSltrBulletNum( AllWork *work )
{
	return work->pre_n_bullet;
}
#endif

void AllCheckSltrBlast( FVECTOR *sltr, FVECTOR *wide, AllWork *work, int n_ssltr )
{
	int		i;
	int		cm_sslt_flag = 0;
	if( !(work->act_flags & 0x0001) ) return;
	
	for( i = 0; i < N_MAX_BLAST_POS; i++ ){
		if( work->blast_timer[i] == BLAST_WAIT_TIME-1 &&
			_Vu0VecLenXYZ( &work->blast_list[i], sltr ) < 500.0f * 500.0f ){
			extern void *NewDebris_Cm_Snd( FVECTOR *bound, FVECTOR *force,
										   int num, int objcode,
										   int color, float scale, int flags, int*, int);
			static int SSltBoundSeTbl[] = {
					SD_A_FNABOU01, //船虫弾む１
					SD_A_FNABOU02, //船虫弾む２
					SD_A_FNABOU03, //船虫弾む３
					SD_A_FNABOU04, //船虫弾む４(４種交互に使う)
			};
			
			//このＮｅｗ関数は_sceを使っているのでVu0のレジスタ破壊あり
			FVECTOR		bound[2],force;

			DG_COPY_VEC( &force, (FVECTOR*)work->root.m[1] );
			force.vw = 50.0f+rnd()*50.0f;
			
			_sceVu0AddVector( &bound[0], sltr, wide );
			_sceVu0SubVector( &bound[1], sltr, wide );
			DG_SetPos( &work->root );
			DG_PutVector( bound, bound, 2 );
			NewDebris_Cm_Snd( bound, &force, n_ssltr, 10389881, 0x60606060, 0.8f, 2, SSltBoundSeTbl, 4 );

#if 1 //yano add 2002.03.04 /* NewDebris_Cm_Snd() にレジスタを壊されていたみたいでした。*/
			DG_SetPos( &work->root );
#endif
			if( !cm_sslt_flag ){//&& CamTimer != GV_Time ){
				extern void *NewCameraSlater( int name, int map, FVECTOR *from );
				FVECTOR		from;
				
				DG_PutVector( sltr, &from, 1 );
				NewCameraSlater( work->name, work->map, &from );
				//CamTimer = GV_Time;
				cm_sslt_flag = 1;
			}
			printf("hit blast\n");
		}
	}
	
}

static void DefTargCallBack( TARGET *off, TARGET *def, AllWork *work )
{
	long64	weapon;
	weapon = def->weapon_type;

    if( weapon & (WP_BULLET|WP_M92) ){
		if( work->n_bullet < N_MAX_BULLET_POS ){
			//FVECTOR		temp;
			//DG_COPY_VEC( &temp, &def->hit );
			_RotTrans( &work->bullet_list[work->n_bullet], &work->inv_root, &def->hit );
			work->bullet_list[work->n_bullet].vy = 0.0f;
			//DG_COPY_VEC( &work->bullet_list[work->n_bullet], &def->hit );
			work->n_bullet++;
		}
		printf("hit bullet\n");
	}else if( weapon & WP_BLAST && !off->power->faint ){
		int i;
		for( i = 0; i < N_MAX_BLAST_POS; i++ ){
			if( work->blast_timer[i] < 0 ){
				FVECTOR		fvtemp;
				_sceVu0AddVector( &fvtemp, &off->center, &off->offset );
				_RotTrans( &work->blast_list[i], &work->inv_root, &fvtemp );
				
				work->blast_timer[i] = BLAST_WAIT_TIME;
				printf("set blast pos[%2d]\n",i);
				break;
			}
		}
		//printf("hit blast\n");
	}
	
	GM_ClearTargetDamage( def );
}

static CONTROL* SearchControl( AllWork *work, int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		//if ( ( control->map & work->map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("check_slater.c: search faild control !!<%x>\n",name);
	return ( NULL );
}

static void Act( AllWork *work )
{
	int		check_joint[4] = { 0, 2, 16, 20, };
	int		i,j;
	work->act_flags = 0;
	work->n_foot = 0;
	work->pre_n_bullet = work->n_bullet;
	work->n_bullet = 0;

	for( i = 0; i < work->n_check; i++ ){
		FVECTOR		*right,*left;
		//AN_Test_Eye2( (FVECTOR*)work->check_body[i]->objs->objs[0].world.m[3], 2 );

		if( !SearchControl( work, work->check_name[i] ) ) continue;
		work->flags[i] = 0;

		for( j = 0; j < 4; j++ ){
			FVECTOR		fvtemp;
			_RotTrans( &fvtemp, &work->inv_root,
						(FVECTOR*)work->check_body[i]->objs->objs[check_joint[j]].world.m[3] );

			_sceVu0SubVector( &work->joint_diff[i][j], &fvtemp, &work->joint_pos[i][j] );
			if( fabs( work->joint_diff[i][j].vx ) < 100.0f ) work->joint_diff[i][j].vx = 0;
			if( fabs( work->joint_diff[i][j].vy ) < 100.0f ) work->joint_diff[i][j].vy = 0;
			if( fabs( work->joint_diff[i][j].vz ) < 100.0f ) work->joint_diff[i][j].vz = 0;
			DG_COPY_VEC( &work->joint_pos[i][j], &fvtemp );
		}

		//printf( "obj_h %f: atama-koshi %f\n",work->check_body[i]->height,(atama->vy-koshi->vy));
		//_RotTrans( &right, &work->inv_root, (FVECTOR*)work->check_body[i]->objs->objs[16].world.m[3] );
		//_RotTrans( &left, &work->inv_root, (FVECTOR*)work->check_body[i]->objs->objs[20].world.m[3] );

		//ふみふみチェック用
		right = (FVECTOR*)work->check_body[i]->objs->objs[16].world.m[3];
		left = (FVECTOR*)work->check_body[i]->objs->objs[20].world.m[3];

		//if( work->check_body[i]->height > 700.0f && (atama->vy-koshi->vy) > 300.0f ){
		//バンダリチェックに変える( +2000 )
		if( 1 ){
			if( !(work->main_flags & 0x0001) ){
				if( right->vy < *work->level[i]+50.0f && *work->level[i]+50.0f < work->pre_height[i][0] &&
					DG_FABS(work->center.vy-*work->level[i]) < 100.0f &&
					right->vy < work->pre_height[i][0] ){

					if( !(work->main_flags & 0x0010) ){
						FVECTOR		temp;
						DG_COPY_VEC( &temp, right );
						temp.vy = *work->level[i];
						_RotTrans( &work->foot_pos[work->n_foot], &work->inv_root, &temp );
						work->n_foot++;
						work->main_flags |= 0x0010;
					}
				}else{
					work->main_flags &= ~0x0010;
				}
				if( left->vy < *work->level[i]+50.0f && *work->level[i]+50.0f < work->pre_height[i][1] &&
					DG_FABS(work->center.vy-*work->level[i]) < 100.0f &&
					left->vy < work->pre_height[i][1] ){

					if( !(work->main_flags & 0x0020) ){
						FVECTOR		temp;
						DG_COPY_VEC( &temp, left );
						temp.vy = *work->level[i];
						_RotTrans( &work->foot_pos[work->n_foot], &work->inv_root, &temp );
						work->n_foot++;
						work->main_flags |= 0x0020;
					}
				}else{
					work->main_flags &= ~0x0020;
				}
			}
#if 0
			//エマ掴みよう
			if( !NewEMA_CommandIsLink() ){
				work->flags[i] = 1;
			}
#endif
			work->flags[i] = 1;

		}
		work->pre_height[i][0] = right->vy;
		work->pre_height[i][1] = left->vy;
	}

	for( i = 0; i < N_MAX_BLAST_POS; i++ ){
		if( work->blast_timer[i] >= 0 ){
			work->blast_timer[i]--;
			work->act_flags |= 0x0001;
		}
	}

	if( work->main_flags&0x8000 ){
		int i;
		FVECTOR	bound[2];
		for( i = 0; i < work->n_refuge; i++ ){
			_sceVu0AddVector( &bound[0], &work->refuge_center[i][0], &work->refuge_wide[i][0] );
			_sceVu0SubVector( &bound[1], &work->refuge_center[i][0], &work->refuge_wide[i][0] );
			DG_SetPos( &work->root );
			DG_PutVector( bound, bound, 2 );
			NewBoundingBoxView( &bound[0], &bound[1], 128, 128, 16 );

			_sceVu0AddVector( &bound[0], &work->refuge_center[i][1], &work->refuge_wide[i][1] );
			_sceVu0SubVector( &bound[1], &work->refuge_center[i][1], &work->refuge_wide[i][1] );
			DG_SetPos( &work->root );
			DG_PutVector( bound, bound, 2 );
			NewBoundingBoxView( &bound[0], &bound[1], 128, 16, 16 );

			//_RotTrans( FVECTOR *out, , &bound[0] );
		}
		HZX_ViewMatrix( &work->root, 1000.0f );
	}
}

static void Die( AllWork *work )
{
	
	GM_FreeTarget( &work->def_trgt );
}

static int GetOptions( AllWork *work )
{
	int n_check,i;
//	int hzx_group,atrap_code;
	FVECTOR		center,wide;
	TARGET		*trg = &work->def_trgt;
	SVECTOR		rot;
	FMATRIX		world;

	

	if( GCL_GetOption( 'p' ) ){
		center.vx = (float)GCL_GetNextInt();
		center.vy = (float)GCL_GetNextInt();
		center.vz = (float)GCL_GetNextInt();
		center.vw = 1.0f;
	}else{
		DG_COPY_VEC( &center, &DG_ZeroVector );
	}
	
	if( GCL_GetOption( 'w' ) ){
		wide.vx = (float)GCL_GetNextInt();
		wide.vy = 1.0f;
		wide.vz = (float)GCL_GetNextInt();
		wide.vw = 0.0f;
	}else{
		DG_COPY_VEC( &center, &DG_ZeroVector );
		wide.vw = 0.0f;
	}

	if( GCL_GetOption( 'r' ) ){
		rot.vx = GCL_GetNextInt();
		rot.vy = GCL_GetNextInt();
		rot.vz = GCL_GetNextInt();
		rot.pad = 0;
	}else{
		rot.vx = 0;
		rot.vy = 0;
		rot.vz = 0;
		rot.pad = 0;
	}

	work->n_refuge = GCL_GetOptionValue( 'R', 0 );

	for( i = 0; i < work->n_refuge && i < N_MAX_REFUGE; i++ ){
		if( GCL_GetNextOption() != 'E' ){ printf("Err Wrong GCL Param<-rEfuge>\n"); return -1; }
		work->refuge_center[i][0].vx = (float)GCL_GetNextInt();
		work->refuge_center[i][0].vy = 0.0f;
		work->refuge_center[i][0].vz = (float)GCL_GetNextInt();
		work->refuge_center[i][0].vw = 1.0f;
		work->refuge_wide[i][0].vx = (float)GCL_GetNextInt();
		work->refuge_wide[i][0].vy = 1.0f;
		work->refuge_wide[i][0].vz = (float)GCL_GetNextInt();
		work->refuge_wide[i][0].vw = 0.0f;
		if( GCL_GetNextOption() != 'e' ){ printf("Err Wrong GCL Param<-escape>\n"); return -1; }
		work->refuge_center[i][1].vx = (float)GCL_GetNextInt();
		work->refuge_center[i][1].vy = 0.0f;
		work->refuge_center[i][1].vz = (float)GCL_GetNextInt();
		work->refuge_center[i][1].vw = 1.0f;
		work->refuge_wide[i][1].vx = (float)GCL_GetNextInt();
		work->refuge_wide[i][1].vy = 1.0f;
		work->refuge_wide[i][1].vz = (float)GCL_GetNextInt();
		work->refuge_wide[i][1].vw = 0.0f;
	}

	DG_COPY_VEC( &work->center, &center );
	DG_COPY_VEC( &work->wide, &wide );
	work->rot.vx = rot.vx;
	work->rot.vy = rot.vy;
	work->rot.vz = rot.vz;
	work->rot.pad = 0;
	DG_SetPos2( &center, &rot );
	DG_GetPos( &world );
	DG_GetPos( &work->root );

	_sceVu0InversMatrix( &work->inv_root, &work->root );

	//チェックオブジェクト
	if( GCL_GetOption( 'n' ) == NULL ){ printf("not input check_num\n"); return -1; }
	n_check = GCL_GetNextInt();
	if( n_check >= N_MAX_CHECK ) n_check = N_MAX_CHECK;
	if( n_check < 0 ) n_check = 0;
	work->n_check = n_check;

	if( n_check && GCL_GetOption( 'o' ) ){
		for( i = 0; i < n_check; i++ ){
			int		name = GCL_GetNextInt();
			CONTROL	*control;
			
			control = SearchControl( work, name );
			if( !control ){ printf("input wrong name\n"); return -1; };
			work->check_body[i] = control->object;
			work->level[i] = control->levels;

			_RotTrans( &work->joint_pos[i][0], &work->inv_root, (FVECTOR*)work->check_body[i]->objs->objs[0].world.m[3] );
			_RotTrans( &work->joint_pos[i][1], &work->inv_root, (FVECTOR*)work->check_body[i]->objs->objs[12].world.m[3] );
			_RotTrans( &work->joint_pos[i][2], &work->inv_root, (FVECTOR*)work->check_body[i]->objs->objs[16].world.m[3] );
			_RotTrans( &work->joint_pos[i][3], &work->inv_root, (FVECTOR*)work->check_body[i]->objs->objs[20].world.m[3] );
	
			DG_COPY_VEC( &work->joint_diff[i][0], &DG_ZeroVector );
			DG_COPY_VEC( &work->joint_diff[i][1], &DG_ZeroVector );
			DG_COPY_VEC( &work->joint_diff[i][2], &DG_ZeroVector );
			DG_COPY_VEC( &work->joint_diff[i][3], &DG_ZeroVector );
			work->joint_diff[i][0].vw = 0.0f;
			work->joint_diff[i][1].vw = 0.0f;
			work->joint_diff[i][2].vw = 0.0f;
			work->joint_diff[i][3].vw = 0.0f;
			//work->check_diff[i].vw = 0.0f;
						
			//work->koshi[i] = (FVECTOR*)work->check_body[i]->objs->objs[0].world.m[3];
			work->check_name[i] = name;
		}
	}
	
	work->main_flags = GCL_GetOptionValue( 'f', 0 );

	work->main_flags |= (GCL_GetOption( 'd' ))?0x8000:0;

	
	//ターゲット初期化
#define DEF_TARGET_CLASS (TARGET_ROTATE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_THROUGH)
	
	GM_SetTarget( trg, DEF_TARGET_CLASS, 0, BOTH_SIDE, &wide, &DG_ZeroVector );
	GM_SetTargetWeaponType( trg, 0 );
//    trg->class = DEF_TARGET_CLASS;
	GM_SetTargetCallBack( trg, (TARGET_CALLBACK)DefTargCallBack, work );
#ifdef DEBUG_MODE
	if( work->main_flags&0x8000 ) NewTargetView( trg, 34, 184, 200 );
#endif
	GM_MoveTarget2( trg, &world );
	GM_PutTarget( trg );
	//GM_MoveTarget( def, &center );
	
	//for( i = 0; i < N_MAX_BLAST_POS; i++ ){
	//	NewDbugSprite(&work->bullet_list[i],100 );
	//}


	return 0;
}

static int GetResources( AllWork *work )
{
	if( GetOptions( work ) ){ printf("ERR!! Wrong Option<ふなむし当り管理>\n"); return -1; };
	return 0;
}

void *NewAllSlater( int name, int map )
{
	AllWork		*work ;
		
	work = (AllWork *)GV_NewActor( GV_ACTOR_AFTER, sizeof( AllWork ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		AllSlater_CurrentWork = work;
	}
	
	return (void *)work ;
}
