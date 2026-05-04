//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	trample_slater.c
	潰れたふなむし
	
	2001/06/06 T.Shibata
	
	$Id: check_slater.c,v 1.1.1.3 2002/11/19 11:48:49 Yoshizawa1 Exp $

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


#define		N_MAX_CHECK		(4)
#define		N_MAX_BULLET_POS	(16)
#define		N_MAX_BLAST_POS		(16)
#define		BLAST_WAIT_TIME		(120)

typedef	struct {
	GV_ACT_EX		actor;
	int				name;
	int				map;
	int				check_name[N_MAX_CHECK];
	short			n_check;
	short			n_foot;
	short			n_bullet;
	short			n_blast;
	
	short			flags[N_MAX_CHECK];
	int				pre_n_bullet;
	int				act_flags;
	float			pre_height[N_MAX_CHECK][2];
	float			*level[N_MAX_CHECK];
	OBJECT			*check_body[N_MAX_CHECK];
	FVECTOR			*koshi[N_MAX_CHECK];
	FVECTOR			diff_koshi[N_MAX_CHECK];

	FVECTOR			foot_pos[N_MAX_CHECK*2];
	FVECTOR			check_pos[N_MAX_CHECK];
	
	FVECTOR			bullet_list[N_MAX_BULLET_POS];
	FVECTOR			blast_list[N_MAX_BLAST_POS];
	short			blast_timer[N_MAX_BLAST_POS];
	
	TARGET			def_trgt;
	int				foot_flags;
} Work ;

static Work *CheckWork = NULL;
#if 0
static int CheckMesgParam( Work *work )
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
	_sceVu0MulVector( &tmp, &tmp, &tmp );
	ans = tmp.vx + tmp.vy + tmp.vz ;
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
		  
		  vmul.xyzw		vf15, vf14, vf14
		  vmulax.w		ACC, vf0, vf15x
		  #vmadday.w		ACC, vf0, vf15y
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

float GetCheckNearPosLen( FVECTOR *out_pos, FVECTOR *out_vec, FVECTOR *in )
{
	Work *work = CheckWork;
	int	i;

	float len = FLOAT_MAX;
	if( !work ) return len;

	for( i = 0; i < work->n_check; i++ ){
		if( work->flags[i] ){
			float tmp = _Vu0VecLenXZ( work->koshi[i], in );
			if( len > tmp ){
				len = tmp;
				out_pos->vx = work->check_pos[i].vx+work->diff_koshi[i].vx*8.0f;
				out_pos->vy = work->check_pos[i].vy+work->diff_koshi[i].vy*8.0f;
				out_pos->vz = work->check_pos[i].vz+work->diff_koshi[i].vz*8.0f;
				out_pos->vw = 1.0f;
				
				//DG_COPY_VEC( out_pos, &work->check_pos[i] );
				DG_COPY_VEC( out_vec, &work->diff_koshi[i] );
			}
		}
	}

	for( i = 0; i < N_MAX_BLAST_POS; i++ ){
		if( work->blast_timer[i] >= 0 ){
			float tmp = _Vu0VecLenXZ( &work->blast_list[i], in );
			if( len > tmp ){
				len = tmp;
				
				DG_COPY_VEC( out_pos, &work->blast_list[i] );
				DG_COPY_VEC( out_vec, &DG_ZeroVector );
			}
		}
	}
	
	return len;
}

FVECTOR *GetCheckSltrTramplePos()
{
	if( !CheckWork ) return &DG_ZeroVector;
	
	return CheckWork->foot_pos;
}

int GetCheckSltrTrampleNum()
{
	if( !CheckWork ) return 0;
	return CheckWork->n_foot;
}

FVECTOR *GetCheckSltrBulletPos()
{
	if( !CheckWork ) return &DG_ZeroVector;
	return CheckWork->bullet_list;
}

int GetCheckSltrBulletNum()
{
	if( !CheckWork ) return 0;
	return CheckWork->pre_n_bullet;
}
//static	int CamTimer = 0;
void CheckSltrBlast( FVECTOR *sltr, FVECTOR *wide )
{

	Work *work = CheckWork;
	int		i;
	int		cm_sslt_flag = 0;
	if( !(work->act_flags & 0x0001) ) return;
	
	if( !CheckWork ) return;
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
			FVECTOR		bound[2],force = { 0.0f, 1.0f, 0.0f, 50.0f+rnd()*50.0f };

			_sceVu0AddVector( &bound[0], sltr, wide );
			_sceVu0SubVector( &bound[1], sltr, wide );

			NewDebris_Cm_Snd( bound, &force, 16, 10389881, 0x60606060, 0.8f, 2, SSltBoundSeTbl, 4 );
			if( !cm_sslt_flag ){//&& CamTimer != GV_Time ){
				extern void *NewCameraSlater( int name, int map, FVECTOR *from );

				NewCameraSlater( work->name, work->map, sltr );
				//CamTimer = GV_Time;
				cm_sslt_flag = 1;
			}
		}
	}
	
}

//extern void SetTrmplSlater( FVECTOR *center, FVECTOR *norm, float size );


static void DefTargCallBack( TARGET *off, TARGET *def, Work *work )
{
	long64	weapon;
	weapon = def->weapon_type;

    if( weapon & (WP_BULLET|WP_M92) ){
		if( work->n_bullet < N_MAX_BULLET_POS ){
			DG_COPY_VEC( &work->bullet_list[work->n_bullet], &def->hit );
			work->n_bullet++;
		}
		//printf("hit bullet\n");
	}else if( weapon & WP_BLAST && !off->power->faint ){
		int i;
		for( i = 0; i < N_MAX_BLAST_POS; i++ ){
			if( work->blast_timer[i] < 0 ){
				_sceVu0AddVector( &work->blast_list[i], &off->center, &off->offset );
				work->blast_timer[i] = BLAST_WAIT_TIME;
				break;
			}
		}
		//printf("hit blast\n");
	}
	
	GM_ClearTargetDamage( def );
}

static CONTROL* SearchControl( Work *work, int name )
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

static void Act( Work *work )
{
	int		i;
	work->act_flags = 0;
	work->n_foot = 0;
	work->pre_n_bullet = work->n_bullet;
	work->n_bullet = 0;
	for( i = 0; i < work->n_check; i++ ){
//		AN_Test_Eye2( (FVECTOR*)work->check_body[i]->objs->objs[0].world.m[3], 2 );
		FVECTOR		*atama,*koshi,*right,*left;

		if( !SearchControl( work, work->check_name[i] ) ) continue;
		work->flags[i] = 0;
		atama = (FVECTOR*)work->check_body[i]->objs->objs[12].world.m[3];
		koshi = work->koshi[i];
		right = (FVECTOR*)work->check_body[i]->objs->objs[16].world.m[3];
		left = (FVECTOR*)work->check_body[i]->objs->objs[20].world.m[3];

		_sceVu0SubVector( &work->diff_koshi[i], koshi, &work->check_pos[i] );
		DG_COPY_VEC( &work->check_pos[i], koshi );
		//printf( "obj_h %f: atama-koshi %f\n",work->check_body[i]->height,(atama->vy-koshi->vy));

		//if( work->check_body[i]->height > 700.0f && (atama->vy-koshi->vy) > 300.0f ){
		if( 1 ){
			//AN_Test_Eye2( (FVECTOR*)work->check_body[i]->objs->objs[6].world.m[3], 2 );			
			//AN_Test_Eye2( (FVECTOR*)work->check_body[i]->objs->objs[6].world.m[3], 2 );
			if( right->vy < *work->level[i]+50.0f && *work->level[i]+50.0f < work->pre_height[i][0] &&
				right->vy < work->pre_height[i][0] ){

				if( !(work->foot_flags & 0x0001) ){
					DG_COPY_VEC( &work->foot_pos[work->n_foot], right );
				
					work->foot_pos[work->n_foot].vy = *work->level[i]+1.0f;
					//printf("aaaaaaaaaaaaaaa\n");
					work->foot_flags |= 0x0001;
					work->n_foot++;
				}
			}else{
				work->foot_flags &= ~0x0001;
			}
			if( left->vy < *work->level[i]+50.0f && *work->level[i]+50.0f < work->pre_height[i][1] &&
				left->vy < work->pre_height[i][1] ){
				
				if( !(work->foot_flags & 0x0002) ){
					//AN_Test_Eye2( left, 2 );
					DG_COPY_VEC( &work->foot_pos[work->n_foot], left );
					work->foot_pos[work->n_foot].vy = *work->level[i]+1.0f;
					//printf("aaaaaaaaaaaaaaa\n");
					work->n_foot++;
					work->foot_flags |= 0x0002;
				}
			}else{
				work->foot_flags &= ~0x0002;
			}

			if( !NewEMA_CommandIsLink() ){
				work->flags[i] = 1;
			}

		}
		work->pre_height[i][0] = right->vy;
		work->pre_height[i][1] = left->vy;
	}
	
	for( i = 0; i < N_MAX_BLAST_POS; i++ ){
		if( work->blast_timer[i] >= 0 ){
			work->blast_timer[i]--;
			work->act_flags |= 0x0001;
		}
		//else{
		//	DG_COPY_VEC( &work->blast_list[i], &DG_ZeroVector );
		//}
	}
}

static void Die( Work *work )
{
	CheckWork = NULL;
	
	GM_FreeTarget( &work->def_trgt );
}

static int GetOneTrapInfo( FVECTOR *center, FVECTOR *wide, HZX_GROUP_ID group_id, int name_id )
{
	int				i,j;
	HZX_GRP			*group;
	HZX_BLOCK		*block;
	HZX_TRP			*trap;
	FVECTOR			bound0,bound1,fvtemp0;

	group = HZX_GetGroup( group_id );
	ASSERT( group );
	block = group->blocks;
	ASSERT( block );
	//printf( "segs %d[%08x]\n", block->n_segs, block->segs );
	//printf( "flrs %d[%08x]\n", block->n_flrs, block->flrs );
	//printf( "trap %d[%08x]\n", block->n_traps, block->traps );

	for( j = 0; j < group->n_blocks; j++ ){
	trap = block->traps;
	//ASSERT( trap );
	for ( i = 0; i < block->n_traps; i++, trap++ ) {
		if( trap->name_id != name_id ) continue;

		bound0.vx = (float)trap->b1.vx + (float)block->tx;
		bound0.vy = (float)trap->b1.vy + (float)block->ty;
		bound0.vz = (float)trap->b1.vz + (float)block->tz;
		bound0.vw = 1.0f;

		bound1.vx = (float)trap->b2.vx + (float)block->tx;
		bound1.vy = (float)trap->b2.vy + (float)block->ty;
		bound1.vz = (float)trap->b2.vz + (float)block->tz;
		bound1.vw = 1.0f;

		//PRINT_PFVEC(0,&bound0);
		//PRINT_PFVEC(1,&bound1);
		
		_sceVu0SubVector( &fvtemp0, &bound1, &bound0 );
		_sceVu0ScaleVector( &fvtemp0, &fvtemp0, 0.5f );
		_sceVu0AddVector( center, &fvtemp0, &bound0 );
		_sceVu0SubVector( wide, center, &bound0 );

		//PRINT_PFVEC(0,center);
		//PRINT_PFVEC(1,wide);
		return 0;
	}
	block++;
	}

	return (-1);
}

static int GetOptions( Work *work )
{
	int n_check,i;
	int hzx_group,atrap_code;
	FVECTOR		center,wide;
	TARGET		*trg = &work->def_trgt;


	
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
			work->koshi[i] = (FVECTOR*)work->check_body[i]->objs->objs[0].world.m[3];
			work->check_name[i] = name;
		}
	}
	
	if( GCL_GetOption( 'g' ) == NULL ){ printf("not input funa hzx group\n"); return -1; }
	hzx_group = ( HZX_GROUP_ID )GV_GetBit( GCL_GetNextInt() );
	
	if( GCL_GetOption( 'a' ) == NULL ){ printf("not input all funa trap group\n"); return -1; }
	atrap_code = GCL_GetNextInt();

	GetOneTrapInfo( &center, &wide, hzx_group, atrap_code );

	//ターゲット初期化
	
#define DEF_TARGET_CLASS (TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_THROUGH)
	
	GM_SetTarget( trg, DEF_TARGET_CLASS, 0, BOTH_SIDE, &wide, &center );
	GM_SetTargetWeaponType( trg, 0 );
//    trg->class = DEF_TARGET_CLASS;
	GM_SetTargetCallBack( trg, (TARGET_CALLBACK)DefTargCallBack, work );
	//NewTargetView( trg, 34, 184, 200 );
	GM_PutTarget( trg );
	//GM_MoveTarget( def, &center );
	
	//for( i = 0; i < N_MAX_BLAST_POS; i++ ){
	//	NewDbugSprite(&work->bullet_list[i],100 );
	//}
	work->foot_flags = 0;
	return 0;
}

static int GetResources( Work *work )
{
	if( GetOptions( work ) ){ printf("ERR!! Wrong Option<ふなむし当り管理>\n"); return -1; };
	return 0;
}

void *NewCheckSlater( int name, int map )
{
	Work		*work ;
	if(CheckWork) return NULL;
		
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return (void*)1;//NULL;
		}
		CheckWork = work;
	}
	
	return (void *)work ;
}
