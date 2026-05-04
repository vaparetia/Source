//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vmp_knife.c
	ヴァンプ投げナイフ（コモデル）
	
	2001/03/06 T.Shibata
	
	$Id: vmp_knife.c,v 1.1.1.3 2002/11/19 11:48:57 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"
#include "libfs.h"

#include "BP_Misc.h"

#define CLOCK_RATIO	((float)BP_BASE_TICK())
#define CLOCK_COUNT	(BP_AdjustTick2(5))

// -----------------------------------------------------------
//			extern
extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );
//extern void VmpShdwClear();

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );
//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define	PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define CHECK_FLAG(_data,_flag)	((_data)&(_flag))
#define SET_FLAG(_data,_flag)	((_data)|=(_flag))
#define CLEAR_FLAG(_data,_flag)	((_data)&=~(_flag))

#define	FLAGS_ACTIVE	(0x01000000)
#define FLAGS_DAM		(0x02000000)
#define FLAGS_DEF		(0x04000000)
#define FLAGS_HZX		(0x08000000)
#define FLAGS_FALL		(0x10000000)
#define FLAGS_DHZX		(0x20000000)

#define FLAGS_TIMER		(0x0000ffff)
//#define LIFE		(60*CLOCK_COUNT)
#define		MODEL_CODE	(GV_StrCode( "vkf_small_cm" ))
#define		N_COMDLS	(256)
#define		MEM_SCR_POS	((void*)(SCRPAD_ADDR))

#define TARGET_CLASS	(TARGET_OFFENSE|TARGET_SEEK|TARGET_ONLINE)

typedef	struct	{
	GV_ACT_EX	actor;
	int			map_id;
	DG_COMDL	*comdl;
	u_int		index;
	u_int		flags;
	int			map;
	int			game_diff;
	TARGET			trgt[N_COMDLS];
	TARGET			def_trgt[N_COMDLS];
	POWER_TARGET	pow_trgt[N_COMDLS];
	HOMING_TRG		homing[N_COMDLS];							// ホーミング部位ターゲット(現在 BODY21_ATAMA)
	int				se_index;
	FVECTOR		vec[N_COMDLS];
	void		*eft_work[N_COMDLS];
	void		*eft_work2[N_COMDLS];
	void		*eft_work3[N_COMDLS];
} Work;

static Work *VmpKnifeWork = NULL;

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

static inline void _ScaleMatrix( FMATRIX *out, FMATRIX *world, float r )
{
	FVECTOR temp = { r, r, r, 0.0f };
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	vmul.xyzw			vf4, vf4,vf8
	vmul.xyzw			vf5, vf5,vf8
	vmul.xyzw			vf6, vf6,vf8
	sqc2				vf4,0x00(%0)
	sqc2				vf5,0x10(%0)
	sqc2				vf6,0x20(%0)
	": : "r"(out), "r"(world), "r"(&temp) );//:"memory" );
#else
	out->m[0][0] = world->m[0][0] * r ;
	out->m[1][0] = world->m[1][0] * r ;
	out->m[2][0] = world->m[2][0] * r ;	

	out->m[0][1] = world->m[0][1] * r ;
	out->m[1][1] = world->m[1][1] * r ;
	out->m[2][1] = world->m[2][1] * r ;	

	out->m[0][2] = world->m[0][2] * r ;
	out->m[1][2] = world->m[1][2] * r ;
	out->m[2][2] = world->m[2][2] * r ;	

	out->m[0][3] = 0.0f ;
	out->m[1][3] = 0.0f ;
	out->m[2][3] = 0.0f ;	
#endif
}

static void TargCallBack( TARGET *off, TARGET *def, Work *work )
{
    if( def->damaged & TARGET_POWER ){
		int			*flags = (int*)&work->vec[off->name].vw;
		FMATRIX		*cmdl_pos = (FMATRIX*)&work->comdl->pos[off->name];

		SET_FLAG(*flags,FLAGS_DAM);
		DG_COPY_VEC( (FVECTOR*)cmdl_pos->m[3], &def->hit );
		//VmpShdwClear();
		//GM_ClearTargetDamage( off );
	}
}

static void DefTargCallBack( TARGET *off, TARGET *def, Work *work )
{
    if( off->damaged & TARGET_POWER && !(def->weapon_type &WP_COLDSPRAY)){
		extern void *NewTs_Spark( FVECTOR *center, FVECTOR *vec, float pow );
		FVECTOR		vec = { frnd(), frnd(), frnd(), 0.0f };
		int			*flags = (int*)&work->vec[def->name].vw;
		FMATRIX		*cmdl_pos = (FMATRIX*)&work->comdl->pos[def->name];

		SET_FLAG(*flags,FLAGS_DEF);
		DG_COPY_VEC( (FVECTOR*)cmdl_pos->m[3], &def->hit );
		GM_ClearTargetDamage( off );
		NewTs_Spark( &def->hit, &vec, 50.0f );
	}
}

static void SetTarget( TARGET *trg, POWER_TARGET *pow_trg, FVECTOR *pow, int num, int damege, int mode )
{
	
	GM_SetTarget( trg, TARGET_CLASS, VmpKnifeWork->map, PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector  );
	if( mode ){
		GM_SetTargetWeaponType( trg, WP_BULLET|WP_BLADEFAINT );
	}else{
		GM_SetTargetWeaponType( trg, WP_BULLET );
	}
    GM_SetPowerTarget( trg, pow_trg, POWER_CONST, 255, 0, damege, pow );
	GM_SetTargetName( trg, num );
    //GM_PutTarget( trg );
	GM_SetTargetCallBack( trg, (TARGET_CALLBACK)TargCallBack, VmpKnifeWork );

	//NewTargetView( trg, 34, 184, 200 );
}

static void SetDefTarget( TARGET *trg, int num, int damege )
{
	static FVECTOR KnifeSize = { 150.0f, 150.0f, 150.0f, 0.0f };
#define DEF_TARGET_CLASS (TARGET_LOCKON|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)
	GM_SetTarget( trg, DEF_TARGET_CLASS, VmpKnifeWork->map, ENEMY_SIDE, &KnifeSize, &DG_ZeroVector );
	GM_SetTargetWeaponType( trg, 0 );
	GM_SetTargetName( trg, num );
    //trg->class = DEF_TARGET_CLASS;
	GM_SetTargetCallBack( trg, (TARGET_CALLBACK)DefTargCallBack, VmpKnifeWork );
	//NewTargetView( trg, 34, 184, 200 );
	GM_PutTarget( trg );
}

static void Act( Work *work )
{
	FVECTOR			*vec = work->vec;
	int				i,num = 0;
	TARGET			*off = work->trgt;
	DG_COMDL_POS	*comdl_pos = work->comdl->pos;
	
	if( !CHECK_FLAG( work->flags, FLAGS_ACTIVE ) ){
		//work->comdl->flag |= DG_COMDL_INVISIBLE;
		return ;
	}
	//work->comdl->flag &= ~DG_COMDL_INVISIBLE;
	for( i = 0; i < N_COMDLS; i++ ){
		u_int *act_flag = (int*)&vec->vw;
		if( CHECK_FLAG( *act_flag, FLAGS_ACTIVE ) ){
			//あたったかフラグチェック
			//当たってたら開放
			if(CHECK_FLAG( *act_flag, FLAGS_FALL )){
				FVECTOR	to;
				FVECTOR *now = (FVECTOR*)comdl_pos->world.m[3];
				_sceVu0AddVector( &to, now, vec );
				to.vw = 1.0f;
				vec->vy -= 2.0f;
				_sceVu0RotMatrixZ( &comdl_pos->world, &comdl_pos->world, PI/6.0f );
				DG_COPY_VEC( now, &to );

				//AN_Test_Eye2( now, 2 );
				
				if( CHECK_FLAG( *act_flag, FLAGS_TIMER ) > 600 ){
					CLEAR_FLAG( *act_flag, FLAGS_ACTIVE );
					DG_COPY_MAT( comdl_pos, &DG_UnitMatrix );
#if 0
					if( work->eft_work[i] ){
						GV_DestroyOtherActor( work->eft_work[i] );
						work->eft_work[i] = NULL;
					}
					if( work->eft_work2[i] ){
						GV_DestroyOtherActor( work->eft_work2[i] );
						work->eft_work2[i] = NULL;
					}
#endif
				}
				(*act_flag)++;
			}else if(CHECK_FLAG( *act_flag, FLAGS_DAM|FLAGS_HZX|FLAGS_DEF|FLAGS_DHZX )){
				if(CHECK_FLAG( *act_flag, FLAGS_DAM )){
					CLEAR_FLAG( *act_flag, FLAGS_ACTIVE );
					DG_COPY_MAT( comdl_pos, &DG_UnitMatrix );
					//printf("ターゲットに");
					//SD_P_N_DMG003		ナイフ刺さりダメージ音
					GM_SeSetMode( SD_P_N_DMG003, (FVECTOR*)comdl_pos->world.m[3], GM_SEMODE_BOMB );
#if 0
					if( work->eft_work[i] ){
						GV_DestroyOtherActor( work->eft_work[i] );
						work->eft_work[i] = NULL;
					}
					if( work->eft_work2[i] ){
						GV_DestroyOtherActor( work->eft_work2[i] );
						work->eft_work2[i] = NULL;
					}
#endif
				}else if(CHECK_FLAG( *act_flag, FLAGS_DEF|FLAGS_DHZX )){
					int		rot0,rot1;
					FVECTOR	rot,temp;
					rot0 = irnd()%1025 - 512;
					rot1 = irnd()%4096;

					vec->vx = 50.0f * TS_SINs(rot0) * TS_COSs(rot1);
					vec->vy = 50.0f * TS_COSs(rot0);
					vec->vz = 50.0f * TS_SINs(rot0) * TS_COSs(rot1);

					rot.vx = frnd()*PI*0.5f;
					rot.vy = frnd()*PI*0.5f;
					rot.vz = 0.0f;
					rot.vw = 0.0f;
					DG_COPY_VEC( &temp, (FVECTOR*)comdl_pos->world.m[3] );
					_sceVu0RotMatrix( &comdl_pos->world, &DG_UnitMatrix, &rot );
					DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], &temp );
					CLEAR_FLAG( *act_flag, FLAGS_TIMER );
					SET_FLAG( *act_flag, FLAGS_FALL );
					if( CHECK_FLAG( *act_flag, FLAGS_DEF ) ){
						GM_SeSetMode( SD_P_N_SHOT02, (FVECTOR*)comdl_pos->world.m[3], GM_SEMODE_BOMB );
					}else{
						int se_list[] = {
							SD_E_N_KABE01, //ナイフ壁、壁刺さり1（rdm呼）
							SD_E_N_KABE02, //ナイフ壁、壁刺さり2（rdm呼）
						};
						GM_SeSetMode( se_list[work->se_index%2], (FVECTOR*)comdl_pos->world.m[3], GM_SEMODE_BOMB );
					}
				}else{
					int		se_list[] = {
						SD_E_N_YUKA01, //ナイフ壁、床刺さり1（rdm呼）
						SD_E_N_YUKA02, //ナイフ壁、床刺さり2（rdm呼）
						SD_E_N_KABE01, //ナイフ壁、壁刺さり1（rdm呼）
						SD_E_N_KABE02, //ナイフ壁、壁刺さり2（rdm呼）
					};
					GM_SeSetMode( se_list[work->se_index%4], (FVECTOR*)comdl_pos->world.m[3], GM_SEMODE_BOMB );
					work->se_index++;
					CLEAR_FLAG( *act_flag, FLAGS_ACTIVE );
					if( work->eft_work[i] ) DG_COPY_MAT( comdl_pos, &DG_UnitMatrix );
#if 0
					if( work->eft_work[i] ){
						GV_DestroyOtherActor( work->eft_work[i] );
						work->eft_work[i] = NULL;
					}
					if( work->eft_work2[i] ){
						GV_DestroyOtherActor( work->eft_work2[i] );
						work->eft_work2[i] = NULL;
					}
#endif
				}
				if( work->eft_work[i] ){
					GV_DestroyOtherActor( work->eft_work[i] );
					work->eft_work[i] = NULL;
				}
				if( work->eft_work2[i] ){
					GV_DestroyOtherActor( work->eft_work2[i] );
					work->eft_work2[i] = NULL;
				}
				if( work->eft_work3[i] ){
					GV_DestroyOtherActor( work->eft_work3[i] );
					work->eft_work3[i] = NULL;
				}
				GM_FreeTarget( &work->def_trgt[i] );
				GM_FreeHomingTrg( &work->homing[i] );
				//printf("当たった!!\n");
			}else{
				//タイマーチェック
				if( CHECK_FLAG( *act_flag, FLAGS_TIMER ) > 600 ){
					CLEAR_FLAG( *act_flag, FLAGS_ACTIVE );
					DG_COPY_MAT( comdl_pos, &DG_UnitMatrix );
					GM_FreeTarget( &work->def_trgt[i] );
					GM_FreeHomingTrg( &work->homing[i] );
					//printf("時間切れ\n");
					
					if( work->eft_work[i] ){
						GV_DestroyOtherActor( work->eft_work[i] );
						work->eft_work[i] = NULL;
					}
					if( work->eft_work2[i] ){
						GV_DestroyOtherActor( work->eft_work2[i] );
						work->eft_work2[i] = NULL;
					}
					if( work->eft_work3[i] ){
						GV_DestroyOtherActor( work->eft_work3[i] );
						work->eft_work3[i] = NULL;
					}
				}else{
					FVECTOR	from, to;
					FVECTOR *now = (FVECTOR*)comdl_pos->world.m[3];
					int		result;
					DG_COPY_VEC( &from, now );
					_sceVu0AddVector( &to, now, vec );
					to.vw = 1.0f;
					//ハザードチェック
					
					result = HZX_OnlineHazardCheck( GM_GetHzxGroupID( work->map_id ), &from, &to,
													(HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR | HZX_CHK_D_SEGMENT |
													HZX_CHK_D_FLOOR),
													HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
													HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE );
					if(result){
						HZX_HZD seg;
						HZX_GetOnlineHazard2( &seg );
						if( seg.attribute & HZX_SEG_DYNAMIC ){
							SET_FLAG( *act_flag, FLAGS_DHZX );
						}else{
							SET_FLAG( *act_flag, FLAGS_HZX );
						}
						HZX_GetOnlinePoint( &to );
					}
					//ターゲットのステップ値
					GM_MoveTarget( &work->def_trgt[i], &to );
					GM_MoveOnlineTarget( off, &from, &to );
					GM_PutTarget( off );
					DG_COPY_VEC( now, &to );
					
					//NewTargetView2( &work->def_trgt[i], 34, 184, 200 );
					//AN_Test_Eye2( now, 2 );
					//PRINT_PFVEC(i,now);
					(*act_flag)++;
					
				}
			}
			num++;
		}
		off++;
		vec++;
		comdl_pos++;
	}

	if(!num) CLEAR_FLAG( work->flags, FLAGS_ACTIVE );
}

static void Die( Work *work )
{
	int			i;
	FVECTOR		*vec = work->vec;

	for( i = 0; i < N_COMDLS; i++ ){
		u_int *act_flag = (int*)&vec->vw;
		if( CHECK_FLAG( *act_flag, FLAGS_ACTIVE ) ){
			GM_FreeTarget( &work->def_trgt[i] );
			GM_FreeHomingTrg( &work->homing[i] );
		}
	}
	
	if(work->comdl){
		DG_DequeueComdlObjs( work->comdl );
		DG_FreeComdl( work->comdl );
	}
	VmpKnifeWork = NULL;
}

static int ChangeGameLevel( int lvl )
{
	
   if ( BP_Area_JP() )
   {
//#ifdef JAPANESE_BP_IGNORE()
	   switch( lvl ){
	     case GM_LEVEL_VERYEASY:
		   return 0;
	     case GM_LEVEL_EASY:
		   return 1;
	     case GM_LEVEL_NORMAL:
		   return 2;
	     case GM_LEVEL_HARD:
		   return 3;
	     case GM_LEVEL_EXTREME:
	     case GM_LEVEL_E_EXTREME:
		   return 4;
	     default:
		   return 0;
	   }
   }
   else
   {
//#else
	   switch( lvl ){
	     case GM_LEVEL_VERYEASY:
		   return 0;
	     case GM_LEVEL_EASY:
		   return 1;
	     case GM_LEVEL_NORMAL:
		   return 2;
	     case GM_LEVEL_HARD:
		   return 3;
	     case GM_LEVEL_EXTREME:
	     case GM_LEVEL_E_EXTREME:
		   return 4;
	     default:
		   return 0;
	   }
   }
//#endif
}

static int GetResources( Work *work, int map )
{
	DG_DEF			*def;
	DG_COMDL		*comdl;
	
	work->map_id = map;
	work->game_diff = ChangeGameLevel( GM_GameLevel );
	
	def = (DG_DEF*)GV_GetCache( GV_CacheID( MODEL_CODE, 'k' ) );
	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_DEFAULT, N_COMDLS, 0 );
	if( !comdl ) return -1;
	DG_QueueComdlObjs( comdl );
	comdl->group_id = GM_GetDGGroupID( map );

	memset( comdl->pos, 0, sizeof(DG_COMDL_POS)*N_COMDLS );

	return (0);
}

void *NewVampKnife( int name, int map )
{
	Work		*work ;
	if( VmpKnifeWork ) return NULL;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->map = map;
		if ( GetResources( work, map ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		VmpKnifeWork = work;
	}
	return (void *)work ;
}

#define		KNIFE_SPEED		(128.0f*CLOCK_RATIO/5.0f)
#define		KNIFE_SPEED_S		(KNIFE_SPEED * 10.0f)


#define KNF_DAMEGE			(18)

static int GameDiffDam_KnfDamege[5] = {
	5,
	10,
	KNF_DAMEGE,		// 18
	22,
	24,
};

static float GameDiffDam_KnfSpeed[5];

void VmpThrowKnifeSet( FVECTOR *from, FVECTOR *to, int *map, int mode )
{
	static IVECTOR col = { 128,128,128,128 };
	FVECTOR			fvtemp,*vec;
	u_int 			*act_flag,index = VmpKnifeWork->index;
	DG_COMDL_POS	*comdl_pos = &VmpKnifeWork->comdl->pos[index];

	vec = &VmpKnifeWork->vec[index];
	act_flag = (u_int*)&vec->vw;

	if( CHECK_FLAG( *act_flag, FLAGS_ACTIVE ) ){
		printf("NULL Knife\n");
		return;
	}
	_sceVu0SubVector( &fvtemp, to, from );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	SetTarget( &VmpKnifeWork->trgt[index],&VmpKnifeWork->pow_trgt[index], &fvtemp,
				index, GameDiffDam_KnfDamege[VmpKnifeWork->game_diff], mode );
	SetDefTarget( &VmpKnifeWork->def_trgt[index], index, 0 );
	TS_MakeMatrix( (FMATRIX*)comdl_pos, &fvtemp, from );
	
	GM_SetHomingTrg( &VmpKnifeWork->homing[index], (FMATRIX*)comdl_pos, NULL, map, NULL, 0 );
	GM_PutHomingTrg( &VmpKnifeWork->homing[index] );

//	_sceVu0ScaleVector( ((FMATRIX*)comdl_pos)->m[0], ((FMATRIX*)comdl_pos)->m[0], 2.0f );
//	_sceVu0ScaleVector( ((FMATRIX*)comdl_pos)->m[1], ((FMATRIX*)comdl_pos)->m[1], 2.0f );
//	_sceVu0ScaleVector( ((FMATRIX*)comdl_pos)->m[2], ((FMATRIX*)comdl_pos)->m[2], 2.0f );
	if( mode ){
		extern void *NewVampKnifeAura( FVECTOR *pos, FVECTOR *vec );
		_sceVu0ScaleVector( vec, &fvtemp, KNIFE_SPEED_S );
		VmpKnifeWork->eft_work[index] = NewVampKnifeAura( (FVECTOR*)((FMATRIX*)comdl_pos)->m[3],
													   (FVECTOR*)((FMATRIX*)comdl_pos)->m[2] );
	}else
   {


      if ( BP_IsPAL()==TRUE )
      {
         GameDiffDam_KnfSpeed[0] = KNIFE_SPEED * 0.6f * 1.2f;
         GameDiffDam_KnfSpeed[1] = KNIFE_SPEED * 0.8f * 1.2f;
         GameDiffDam_KnfSpeed[2] = KNIFE_SPEED * 1.2f;
         GameDiffDam_KnfSpeed[3] = KNIFE_SPEED * 1.5f * 1.2f;
         GameDiffDam_KnfSpeed[4] = KNIFE_SPEED * 2.0f * 1.2f;
      }
      else
      {
         GameDiffDam_KnfSpeed[0] = KNIFE_SPEED * 0.6f;
         GameDiffDam_KnfSpeed[1] = KNIFE_SPEED * 0.8f;
         GameDiffDam_KnfSpeed[2] = KNIFE_SPEED;
         GameDiffDam_KnfSpeed[3] = KNIFE_SPEED * 1.5f;
         GameDiffDam_KnfSpeed[4] = KNIFE_SPEED * 2.0f;
      }

		_sceVu0ScaleVector( vec, &fvtemp, GameDiffDam_KnfSpeed[VmpKnifeWork->game_diff] );

      if ( BP_IsPAL()==TRUE )
   		_sceVu0ScaleVector( vec, vec, 5.0f/6.0f );

   }
	*act_flag = 0;
	SET_FLAG( *act_flag, FLAGS_ACTIVE );
	comdl_pos->color = col;

	{
		extern	void	*NewInterPoly_Gr( FVECTOR *, float , CVECTOR );
		//0x10283000
#ifdef BP_PS2 //yano
		VmpKnifeWork->eft_work2[index] =
			NewInterPoly_Gr( (FVECTOR*)((FMATRIX*)comdl_pos)->m[3], 15.0f, (CVECTOR){ 16, 30, 38, 96 } );
#else
		{
		CVECTOR dmdm = { 16, 30, 38, 96 };
		VmpKnifeWork->eft_work2[index] =
			NewInterPoly_Gr( (FVECTOR*)((FMATRIX*)comdl_pos)->m[3], 15.0f, dmdm );
	    }
#endif
	}

	{
		extern void *NewKirari( FMATRIX *world );
		VmpKnifeWork->eft_work3[index] =
			NewKirari( &comdl_pos->world );
	}

	if(++VmpKnifeWork->index >= N_COMDLS) VmpKnifeWork->index = 0;

	SET_FLAG( VmpKnifeWork->flags, FLAGS_ACTIVE );
	//printf("%x\n",VmpKnifeWork->flags );
}
