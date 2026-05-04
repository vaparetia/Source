//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vmp_shdwtrg.c
	ヴァンプ影縛りターゲット
	
	2001/05/11 T.Shibata
	
	$Id: vmp_shdwtrg.c,v 1.1.1.3 2002/11/19 11:48:57 Yoshizawa1 Exp $

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
#include	"g_other.h"

#include	"../util/ts_util.h"
#include "libfs.h"

#define CLOCK_RATIO	(BP_BASE_TICK())
#define CLOCK_COUNT	(BP_AdjustTick2(5))

// -----------------------------------------------------------
//			extern
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
#define	BODYWORLD(_body,_index) (((OBJECT*)_body)->objs->objs[(_index)].world)

#define	FLAGS_ACTIVE	(0x01000000)
#define FLAGS_DAM		(0x02000000)
#define FLAGS_DEF		(0x04000000)
#define FLAGS_HZX		(0x08000000)
#define FLAGS_FALL		(0x10000000)
#define FLAGS_TIMER		(0x0000ffff)
//#define LIFE		(60*CLOCK_COUNT)
#define		MODEL_CODE	(GV_StrCode( "vkf_small_cm" ))
#define		N_COMDLS	(256)
#define		MEM_SCR_POS	((void*)(SCRPAD_ADDR))

#define DEF_TARGET_CLASS (TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_THROUGH)

#define N_MAX_SPOT		(4)
#define N_TRGT_PARTS	(3)



/* 影縛り開始 */
extern void PL_StartKageshibari( int time ) ;
/* 影縛りチェック */
extern int PL_KageshibariNow( void ) ;
/* 影縛り解除 */
extern void PL_FreeKageshibari( void ) ;

typedef	struct	{
	GV_ACT_EX	actor;
	int			map_id;
	int			map;
	short		flags;
	short		pad_count;
	int			hzx_id;
	int			hzx_group;
	char		status_count[16];
	int			proc;
	TARGET		def_trgt[N_MAX_SPOT][N_TRGT_PARTS];//[GM_MAX_PLAYER_SHADOW_POS];
	FVECTOR		trgt_pos[N_MAX_SPOT][N_TRGT_PARTS];
	short		trgt_flags[N_MAX_SPOT][N_TRGT_PARTS];
	int			game_diff;
	DG_COMDL	*comdl;
	TARGET		def_knf_trgt;
	FVECTOR		hit;
	FVECTOR		hit_light;
	FVECTOR		vec;
	//int			hit_num;
	int			timer;
	int			hit_name;
	int			dbg_timer;
} Work;

static Work *VmpShdwWork = NULL;

#if 0
static void CallOffenceWhenThrough( TARGET *off, TARGET *def )
{
    off->hit = def->hit ;
    off->damaged |= off->class & TARGET_CHECK_CLASS ;
    if ( off->callback != NULL )
	( *off->callback )( off, def, off->work ) ;
}
#endif

static void DefTargCallBack( TARGET *off, TARGET *def, Work *work )
{
//	static int aaa = 600*5;
    if( def->damaged & TARGET_POWER ){
		if( off->weapon_type & WP_BLADEFAINT ){
			if(!(work->flags & 1)){
				DG_COMDL_POS	*comdl_pos = work->comdl->pos;
				PL_StartKageshibari( 600*5 );
				/*
				if( !PL_KageshibariNow() ){
					work->flags &= ~3;
					//printf("なんか影縛り失敗らしいっすよ\n");
					//return;
				}
				*/
				printf("vamp:  HIT!! shadow[%d]\n", def->name);
				
				//CallOffenceWhenThrough( off, def );
				work->flags |= 1;
				work->pad_count = 0;

				{
					FVECTOR dmdm = { 0.0f, 1.0f, 0.0f, 0.0f };
					TS_MakeMatrix2( &comdl_pos->world, &off->power->force, &dmdm, &def->hit );
					comdl_pos->world.m[0][3] = 0.0f ;
					comdl_pos->world.m[1][3] = 0.0f ;
					comdl_pos->world.m[2][3] = 0.0f ;
					comdl_pos->world.m[3][3] = 1.0f ;
				}

				comdl_pos->color.vx = 128;
				comdl_pos->color.vy = 128;
				comdl_pos->color.vz = 128;
				comdl_pos->color.vw = 128;
				DG_COPY_VEC( &work->hit, &def->hit );
				DG_COPY_VEC( &work->hit_light, &GC_ProjectionSpot_SnakeCheckList[def->name].pos );
				work->hit_name = GC_ProjectionSpot_SnakeCheckList[def->name].name;
				//PRINT_PFVEC( 0, (FVECTOR*)(comdl_pos->world.m[0]) );
				//PRINT_PFVEC( 1, (FVECTOR*)(comdl_pos->world.m[1]) );
				//PRINT_PFVEC( 2, (FVECTOR*)(comdl_pos->world.m[2]) );
				//PRINT_PFVEC( 3, (FVECTOR*)(comdl_pos->world.m[3]) );
				//work->hit_num = def->name;

				work->dbg_timer = 0;
				//GC_ProjectionSpot_SnakeCheckList[def->name].pos;
				//GV_PadReleaseOn( 0 );
			}
		}
	}
	GM_ClearTargetDamage( def );
}

static void DefTrgtAllSkip( Work *work )
{
	int			i;
	TARGET *def = work->def_trgt[0];
	short			*flags = work->trgt_flags[0];
	for( i = 0; i < N_MAX_SPOT*N_TRGT_PARTS; i++ ){
		def->class |= TARGET_SKIP;
		*flags = 0;
		def++;
		flags++;
	}
}


#define GACHA_COUNT			(160)

static int GameDiff_GachaCount[5] = {
	120,
	120,
	GACHA_COUNT,		// 160
	200,
	400,
};

static int CheckSpot( int name )
{
	int	i,n_lights = GC_ProjectionSpot_HitLights ;
	GCT_ProjectionSpot_SpotParam	*spot;
	spot = GC_ProjectionSpot_SnakeCheckList;
	
	for( i = 0; i < n_lights; i++ ){
		if( spot->name == name ) return 0;
		spot++;
	}
	printf("vamp: spot light nothing!!\n");
	return 1;
}

static void Act( Work *work )
{
#if 1
	FVECTOR							from[N_TRGT_PARTS];
	int								n_lights, i, j, n_trgt_parts ;
	HZX_GROUP_ID					hzx_id, tmp ;
	GCT_ProjectionSpot_SpotParam	*spot ;

	if( work->flags & 1 ){
		GV_PAD *pad = GV_PadDataDirect ;
		int		i,press = pad->press,status = pad->status;
		//ライトが未だ有るかチェック
		if( !PL_KageshibariNow() || CheckSpot( work->hit_name ) ){
			work->flags &= ~7;
			work->def_knf_trgt.class |= TARGET_SKIP;
			work->def_knf_trgt.class &= ~TARGET_LOCKON;
			GV_CallChildSignalFunc( work, 0, 0 );
			PL_FreeKageshibari();
			printf("vamp: esc shadow bind\n");
			return;
		}

		if( !(work->flags & 0x0004) ){
			//エフェクト生成
			extern void *NewVampShadowTrapEffect( FVECTOR *t_pos, FVECTOR *l_pos );
			GM_SeSetMode( SD_E_V_KAGE02, &work->hit, GM_SEMODE_BOMB );
			//これが
			GV_SetActorChild( work,
							  NewVampShadowTrapEffect( &work->hit, &work->hit_light ) );

			work->def_knf_trgt.class &= ~TARGET_SKIP;
			work->def_knf_trgt.class |= TARGET_LOCKON;
			GM_MoveTarget( &work->def_knf_trgt, &work->hit );

			if( /*!(work->flags&0x0080) && ++work->dbg_timer > 4 && */work->proc ){
				GCL_ARGS	gcl_args;
				int			data[3];

				data[0] = work->hit.vx;
				data[1] = work->hit.vy;
				data[2] = work->hit.vz;

				gcl_args.argc = 3;
				gcl_args.argv = data;
			
				work->flags |= 0x0080;
				GCL_ExecProc( work->proc, &gcl_args );
			}
			printf("vamp: shadow effect run!\n");
			work->flags |= 0x0004;
		}
		//printf("pad press  [%08x]\n", pad->press );
		//printf("pad status [%08x]\n", pad->status );
		for( i = 0; i < 16; i++ ){
			if( press & (1<<i) ) work->pad_count++;
			if( status & (1<<i) ) work->status_count[i]++;
			if( work->status_count[i] > 8 ){
				work->status_count[i] = 0;
				work->pad_count++;
			}
		}
		
		//DEBUG_Locate( 20, 330, 0 );
		//DEBUG_Printf( "PAD COUNT [%3d]\n", work->pad_count );

		DefTrgtAllSkip( work );
		
		if( !(work->flags & 2) && work->pad_count > GameDiff_GachaCount[work->game_diff]/4 ){
			int se_table[] = {
				SD_V_RAISIB01, //ライデン影縛られ１「動けない」
				SD_V_RAISIB02, //ライデン影縛られ２「体が！？」
			};
			GM_JimakuSeSetMode( se_table[irnd()%2], &GM_PlayerPosition, GM_SEMODE_BOMB );
			work->flags |= 2;
		}else if( work->pad_count >= GameDiff_GachaCount[work->game_diff] ){
			//GV_PadReleaseOff( 0 );
			work->flags &= ~7;
			work->def_knf_trgt.class |= TARGET_SKIP;
			work->def_knf_trgt.class &= ~TARGET_LOCKON;
			PL_FreeKageshibari();
			printf("vamp: esc shadow bind(counter)\n");
			GV_CallChildSignalFunc( work, 0, 0 );
		}
		return;
	}
	if( work->flags & 0x0010 ){
		FVECTOR	to;
		FVECTOR *now = (FVECTOR*)work->comdl->pos->world.m[3];
		_sceVu0AddVector( &to, now, &work->vec );
		to.vw = 1.0f;
		work->vec.vy -= 2.0f;
		_sceVu0RotMatrixZ( &work->comdl->pos->world, &work->comdl->pos->world, PI/6.0f );
		DG_COPY_VEC( now, &to );
		if( --work->timer < 0 ){
			work->flags &= ~0x0010;
			DG_COPY_MAT( &work->comdl->pos->world, &DG_UnitMatrix );
		}
	}

	
	n_lights = GC_ProjectionSpot_HitLights ;
	if ( !n_lights ){ DefTrgtAllSkip( work ); return; }

	hzx_id = ( HZX_GROUP_ID )work->hzx_group;
	if ( hzx_id == -1 ){ DefTrgtAllSkip( work ); return; }

	tmp = HZX_CurrentGroupID ;
	HZX_CurrentGroupID = 0;

	switch( GM_Item ){
	  case IT_CBBox:
	  case IT_CBBoxB:
	  case IT_CBBoxC:
	  case IT_CBBoxWet:
	  case IT_CBBoxD:
	  case IT_CBBoxE:
		if( GM_PlayerBoxBody ){
			FVECTOR box_shift = { 0.0f, 300.0f, 0.0f, 0.0f };
			n_trgt_parts = 1;
			_sceVu0AddVector( &from[0], (FVECTOR*)BODYWORLD( GM_PlayerBoxBody, 0 ).m[3], &box_shift );
			DG_COPY_VEC( &from[1], &from[0] );
			DG_COPY_VEC( &from[2], &from[0] );
		}else{
			n_trgt_parts = N_TRGT_PARTS;
			DG_COPY_VEC( &from[0], (FVECTOR*)BODYWORLD( GM_PlayerBody, HUMAN21_KUBI ).m[3] );
			DG_COPY_VEC( &from[1], (FVECTOR*)BODYWORLD( GM_PlayerBody, HUMAN21_MUNE ).m[3] );
			DG_COPY_VEC( &from[2], (FVECTOR*)BODYWORLD( GM_PlayerBody, HUMAN21_KOSHI ).m[3] );
		}
		break;
	  default:
		n_trgt_parts = N_TRGT_PARTS;
		DG_COPY_VEC( &from[0], (FVECTOR*)BODYWORLD( GM_PlayerBody, HUMAN21_KUBI ).m[3] );
		DG_COPY_VEC( &from[1], (FVECTOR*)BODYWORLD( GM_PlayerBody, HUMAN21_MUNE ).m[3] );
		DG_COPY_VEC( &from[2], (FVECTOR*)BODYWORLD( GM_PlayerBody, HUMAN21_KOSHI ).m[3] );
		break;
	}
	spot = GC_ProjectionSpot_SnakeCheckList ;

	
//	printf("%d\n",n_lights);
	
	//DEBUG_Locate( 20, 330, 0 );
	

	for ( i = 0; i < N_MAX_SPOT; i++ ) {
		TARGET *def = work->def_trgt[i];
		FVECTOR	*pos = work->trgt_pos[i];
		short	*flags = work->trgt_flags[i];
		//DEBUG_Printf( "%d:", i );
		if( i < n_lights ){
			//printf("name %d\n",spot->name);
			for( j = 0; j < N_TRGT_PARTS; j++ ){
				int			c;
				FVECTOR		to, dir;

				_sceVu0SubVector( &dir, &from[j], &spot->pos );
				_sceVu0Normalize( &dir, &dir );
				// 発見される影は、スポットから影響距離の半分までの間にある
				_sceVu0ScaleVector( &dir, &dir, spot->range * 1.4f );
				_sceVu0AddVector( &to, &spot->pos, &dir );
#if 0
				{
					FVECTOR		v[ 2 ] ;
					DG_COPY_VEC( &v[ 0 ], &from[j] );
					DG_COPY_VEC( &v[ 1 ], &to ) ;
					NewLineView( v, 1, 23, 232, 23 );
				}
#endif
				c = HZX_OnlineHazardCheck( hzx_id, &from[j], &to, HZX_CHK_ALL,
										   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ;
				if ( j < n_trgt_parts && c ) {
					FVECTOR		shdw;
				
					HZX_GetOnlinePoint( &shdw ) ;
					shdw.vy += 1.0F ;
					def->class &= ~TARGET_SKIP;
					DG_COPY_VEC( pos, &shdw );
					GM_MoveTarget( def, &shdw );
					//AN_Test_Eye2( &shdw, 2 );
					*flags = 1;
					GM_SetTargetName( def, i );
					//DEBUG_Printf( "%d", j );
					//printf("ありまーす\n");
				}else{
					*flags = 0;
					//DEBUG_Printf( "-" );
					def->class |= TARGET_SKIP;
				}
				def++;
				pos++;
				flags++;
			}
		}else{
			for( j = 0; j < N_TRGT_PARTS; j++ ){
				*flags = 0;
				//DEBUG_Printf( "-" );
				def->class |= TARGET_SKIP;
			}
		}
		//DEBUG_Printf( "\n");

		spot ++ ;
	}
	HZX_CurrentGroupID = tmp ;

#else
	int				i;
	TARGET			*def = work->def_trgt;
	for( i = 0; i < GM_MAX_PLAYER_SHADOW_POS; i++ ){
		if( i < GM_N_PlayerShadowPos ){
			def->class &= ~TARGET_SKIP;
			GM_MoveTarget( def, &GM_PlayerShadowPos[i] );
			//AN_Test_Eye2( &GM_PlayerShadowPos[i], 2 );
			//printf("shdw trgt num[%d]\n",i);
		}else{
			def->class |= TARGET_SKIP;
			//printf("tetete\n");
		}
		def++;
	}
	//printf("shdw %d\n",GM_N_PlayerShadowPos);
#endif
}

static void Die( Work *work )
{
	int			i,j;
	for( i = 0; i < N_MAX_SPOT; i++ ){
		for( j = 0; j < N_TRGT_PARTS; j++ ){
			GM_FreeTarget( &work->def_trgt[i][j] );
		}
	}
	
	if(work->comdl){
		DG_DequeueComdlObjs( work->comdl );
		DG_FreeComdl( work->comdl );
	}
	GM_FreeTarget( &work->def_knf_trgt );
	VmpShdwWork = NULL;
}

static void SetDefTarget( Work *work, TARGET *trg )
{
	static FVECTOR ShdwSize = { 400.0f, 1.0f, 400.0f, 0.0f };

	GM_SetTarget( trg, TARGET_DEFENSE|TARGET_SEEK, work->map, PLAYER_SIDE, &ShdwSize, &DG_ZeroVector );
//	GM_SetTarget( trg, TARGET_DEFENSE|TARGET_SEEK, work->map, ENEMY_SIDE, &ShdwSize, &DG_ZeroVector );
	GM_SetTargetWeaponType( trg, 0 );
	//GM_SetTargetName( trg, num );
    trg->class = DEF_TARGET_CLASS;
	GM_SetTargetCallBack( trg, (TARGET_CALLBACK)DefTargCallBack, work );

	GM_PutTarget( trg );
	trg->class |= TARGET_SKIP;
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

static void DefTargCallBack_Knf( TARGET *off, TARGET *def, Work *work )
{
    if( off->damaged & TARGET_POWER ){
		extern void *NewTs_Spark( FVECTOR *center, FVECTOR *vec, float pow );
		FVECTOR		vec = { frnd(), frnd(), frnd(), 0.0f };
		off->class |= TARGET_SKIP;
		NewTs_Spark( &def->hit, &vec, 50.0f );
		//GM_SeSetMode( SD_P_N_DMG003, &def->hit, GM_SEMODE_BOMB );
		GM_SeSetMode( SD_P_N_SHOT02, &def->hit, GM_SEMODE_BOMB );
		work->flags &= ~7;
		work->flags |= 0x0010;
		PL_FreeKageshibari();
		GV_CallChildSignalFunc( work, 0, 0 );
		def->class |= TARGET_SKIP;
		def->class &= ~TARGET_LOCKON;
		{
			int		rot0,rot1;
			FVECTOR	rot,temp;
			rot0 = irnd()%1025 - 512;
			rot1 = irnd()%4096;
			
			work->vec.vx = 50.0f * TS_SINs(rot0) * TS_COSs(rot1);
			work->vec.vy = 100.0f * TS_COSs(rot0);
			work->vec.vz = 50.0f * TS_SINs(rot0) * TS_COSs(rot1);

			rot.vx = frnd()*PI*0.5f;
			rot.vy = frnd()*PI*0.5f;
			rot.vz = 0.0f;
			rot.vw = 0.0f;
			DG_COPY_VEC( &temp, (FVECTOR*)work->comdl->pos->world.m[3] );
			_sceVu0RotMatrix( &work->comdl->pos->world, &DG_UnitMatrix, &rot );
			DG_COPY_VEC( (FVECTOR*)work->comdl->pos->world.m[3], &temp );
			work->timer = 600;
		}
		printf("vamp: att knf\n");
	}
	GM_ClearTargetDamage( def );
}

static void SetDefTarget_Knf( Work *work, TARGET *trg, int map )
{
	static FVECTOR KnifeSize = { 100.0f, 100.0f, 100.0f, 0.0f };
#define DEF_TARGET_CLASS_KNF (TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)
	GM_SetTarget( trg, DEF_TARGET_CLASS_KNF, map, ENEMY_SIDE, &KnifeSize, &DG_ZeroVector );
	GM_SetTargetWeaponType( trg, 0 );
	GM_SetTargetCallBack( trg, (TARGET_CALLBACK)DefTargCallBack_Knf, work );
	//NewTargetView( trg, 34, 184, 200 );
	GM_PutTarget( trg );

	trg->class |= TARGET_SKIP;
}

static int GetResources( Work *work, int map )
{
	int				i,j;
	DG_DEF			*def;
	DG_COMDL		*comdl;

	work->game_diff = ChangeGameLevel( GM_GameLevel );
	
	work->map_id = map;
	work->flags = 0;
	for( i = 0; i < N_MAX_SPOT; i++ ){
		for( j = 0; j < N_TRGT_PARTS; j++ ){
			SetDefTarget( work, &work->def_trgt[i][j] );
			//NewTargetView( &work->def_trgt[i][j], ((i&1)?128:0), ((i&2)?128:0), 128 );
		}
	}

	GCL_GetOption( 'c' );
	work->hzx_id = GM_GetHzxGroupID( GM_GetMapID( GCL_GetNextInt() ) );
	ASSERT( work->hzx_id != 0 );

	GCL_GetOption( 'g' );
	work->hzx_group = ( HZX_GROUP_ID )GV_GetBit( GCL_GetNextInt() ) ;
	work->proc = GCL_GetOptionValue( 'p', 0 );

	def = (DG_DEF*)GV_GetCache( GV_CacheID( MODEL_CODE, 'k' ) );
	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_DEFAULT, 1, 0 );
	if( !comdl ) return -1;
	DG_QueueComdlObjs( comdl );
	comdl->group_id = GM_GetDGGroupID( map );

	memset( comdl->pos, 0, sizeof(DG_COMDL_POS) );

	SetDefTarget_Knf( work, &work->def_knf_trgt, map );
	return (0);
}

void *NewVampShdwTrgt( int name, int map )
{
	Work		*work ;
	if( VmpShdwWork ) return NULL;
	//work = (Work *)GV_NewActor( GV_ACTOR_PLAYER, sizeof( Work ) );

    work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, 
									 sizeof( Work ), PLAYER_BODY_ACTOR_PRIO+1 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		work->map = map;
		if ( GetResources( work, map ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		VmpShdwWork = work;
	}
	return (void *)work ;
}

void VmpShdwClear()
{
	if( VmpShdwWork->flags & 4 ){
		VmpShdwWork->flags &= ~7;
		VmpShdwWork->def_knf_trgt.class |= TARGET_SKIP;
		VmpShdwWork->def_knf_trgt.class &= ~TARGET_LOCKON;
		PL_FreeKageshibari();
		GV_CallChildSignalFunc( VmpShdwWork, 0, 0 );
		printf("vamp: esc shadow bind(out file)\n");
	}
}

int	VMP_CheckShdwBind()
{
	return (VmpShdwWork->flags & 4);
}

int	VMP_CheckShdw()
{
	int			i;
	short		*flags = VmpShdwWork->trgt_flags[0];
	for( i = 0; i < N_MAX_SPOT*N_TRGT_PARTS; i++ ){
		if( *flags ){
			return 1;
		}
		flags++;
	}
	return 0;
}

void VMP_GetShdwPos( FVECTOR *shdw )
{
	int			i;
	short		*flags = VmpShdwWork->trgt_flags[0];
	FVECTOR		*pos = VmpShdwWork->trgt_pos[0];

	for( i = 0; i < N_MAX_SPOT*N_TRGT_PARTS; i++ ){
		if( *flags ){
			DG_COPY_VEC( shdw, pos );
			return;
		}
		flags++;
		pos++;
	}
}
