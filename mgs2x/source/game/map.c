//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   map.c
   マップシステム（仮々々々々バージョン）
   
   1999/07/16 M.Sonoyama
   $Id: map.c,v 1.1.1.3 2002/11/19 11:41:53 Yoshizawa1 Exp $
   */

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#include "bp_vector.h"//BP_ASM


#define	MAX_MAPS	GM_MAX_MAPS	/* g_struct.h */
#define	MAX_MAP_OBJS	(32)

/* マップリスト */
static	int	N_Maps ;
static	MAP	Maps[ MAX_MAPS ] ;

/* マップオブジェクトリスト（影投影用含む） */
static	int	N_MapObjs ;
static	DG_OBJS	*MapObjs[ MAX_MAP_OBJS ] ;

/**/
/* ステージマップ全部 */
int		GM_StageMapAll ;

/* 現在処理中のマップ名 */
int		GM_CurrentMap ;
/* カレントステージマップ */
int		GM_CurrentStageMap ;
int		GM_CurrentChanlMap[ GM_MAP_CHANL_MAX ];
int		GM_ChanlTargetMap[ GM_MAP_CHANL_MAX ];

/* マップチェンジトラップ */
typedef	struct	{
#if 0
	FVECTOR			b1, b2 ;
#endif
	int				name_id ;
	int				now_map ;
	int				next_map ;
	int				chara_name ;
} MCT ;

static	int			N_MapChangeTraps ;
static	MCT			MapChangeTrap[ 8 ] ;

/*----------------------------------------------------------------*/

typedef	struct	{
    GV_ACT	actor ;
} Work ;

#if 0
static	void	Act( work )
Work		*work ;
{
    int	i ;
    DG_OBJS	*objs ;
    DG_OBJ	*obj ;

    objs = MapObjs[ 0 ] ;
    for ( i = 0; i < N_MapObjs; i ++ ) {
		printf( "map[ %d ]\n", i ) ;
		DumpMatrix( objs->world ) ;
		DumpVec( ( FVECTOR * )&objs->def->models[ 0 ].tx ) ;
		obj = &objs->objs[ 4 ] ;
		printf( "\n" ) ;
		DumpMatrix( obj->world ) ;	
		printf( "\n" ) ;
		objs ++ ;
    }
}
#endif

/*ステージモデルを取得*/
DG_OBJS *GM_GetMapObjs(int map_no){
	ASSERT( map_no < N_MapObjs  ) ;
	return MapObjs[ map_no ] ;
}

/* ＩＤからビットを得る */
int GM_GetBit( int id )
{
#ifdef PSX2	
    return 0x40000000 >> id ;
#else
	return GV_GetBit( id );
#endif	
}

/* ビットからＩＤを得る */
int GM_GetID( int bit )
{
#if 0 //BP_ASM def PSX2
    long64	bit64, id64 ;

    bit64 = id64 = 0 ;
    bit64 = bit ;
    asm volatile ( "plzcw %0,%1" : "=r"(id64) : "r"(bit64) ) ;
    /* 上からの連続したビット数 - 1 */
    return ( int )( id64 & 0xffffffff ) ;
#else
	return GV_GetNo( bit );
#endif	
}

/* 終了関数 */
static	void	Die( work )
Work		*work ;
{
    int		i ;
    DG_OBJS	**objs ;
    extern void	DG_FreePreshade( DG_OBJS * ) ;

    objs = MapObjs ;
    for ( i = 0; i < N_MapObjs; i ++, objs ++ ) {
		DG_FreePreshade( *objs ) ;
		DG_DequeueObjs( *objs ) ;
		DG_FreeObjs( *objs ) ;
    }
}

/* マップを検索 ０の時は空きマップを返す */
static	int	GetMap( name )
u_int		name ;
{
    int		i ;

    for ( i = 0; i < MAX_MAPS; i ++ ) {
		if ( Maps[ i ].name == name ) return i ;
    }
    return -1 ;
}


/* 指定ハザードグループＩＤを持つマップＩＤを返す */
int		GM_GetMapIDfromHzxGroupID( HZX_GROUP_ID hzx_id )
{
    int		i ;
	int		res = 0 ;

    for ( i = 0; i < N_Maps; i ++ ) {
		if ( Maps[ i ].hzx_group & ( int )hzx_id ) res |= Maps[ i ].id ;
    }
    return res ;
}

/* 指定チャンネルに表示中のマップＩＤを返す */
int		GM_GetMapIDfromChanlDisp( int chanl )
{
	int		dg_group ;
	int		i, res ;

	dg_group = DG_Chanl( chanl )->group_id ;
	res = 0 ;
	for ( i = 0; i < N_Maps; i ++ ) {
		if ( Maps[ i ].dg_group & dg_group ) res |= Maps[ i ].id ;
	}
	return res ;
}

/* マップＩＤからハザードグループ番号を返す */
int		GM_GetHzxGroupID( bit )
int		bit ;
{
#if 0
    int		id ;

    id = GM_GetID( bit ) ;
    return Maps[ id ].hzx_group ;
#else
	int res = 0;

	while( bit != 0 ){
		int id;
		id = GV_GetNo( bit );
		bit &= ~GV_GetBit( id );

		res |= Maps[ id ].hzx_group ;
	}
	return res;
#endif
}

/* マップＩＤからＤＧ＿ＧＲＯＵＰを返す */
int		GM_GetDGGroupID( bit )
int		bit ;
{
#if 0
    int		id ;

    id = GM_GetID( bit ) ;
    return Maps[ id ].dg_group ;
#else
	int res = 0;

	while( bit != 0 ){
		int id;
		id = GV_GetNo( bit );
		bit &= ~GV_GetBit( id );

		res |= Maps[ id ].dg_group;
	}
	return res;
#endif
}

/* ハザード初期化 */
static	void	InitHazard( map, name, group )
MAP		*map ;
int		name, group ;
{
    map->hzx_group = GV_GetBit( group ) ;
	HZX_AllMapID |= GV_GetBit( group ) ;
}

/* ライトを初期化 */
static	void	InitLight( map, name )
MAP		*map ;
int		name ;
{
    map->light = ( LIT_DEF * )GV_GetCache( GV_CacheID( name, 'l' ) ) ;
    DG_SetFixedLightMap( map->light, map->id ) ;
}

/* モデルをセット */
static	void	InitStage( map, name, pos )
MAP		*map ;
int		name ;
IVECTOR		*pos ;
{
    extern int 	DG_ActiveShadowFlag ;
    DG_DEF	*def ;
    DG_OBJS	*objs ;

    ASSERT( N_MapObjs < MAX_MAP_OBJS ) ;
    def = ( DG_DEF * )GV_GetCache( GV_CacheID( name, 'k' ) ) ;
    ASSERT( def != NULL ) ;
    objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE | DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT, 0 ) ;
    objs->world.m[ 3 ][ 0 ] = ( float )pos->vx ;
    objs->world.m[ 3 ][ 1 ] = ( float )pos->vy ;
    objs->world.m[ 3 ][ 2 ] = ( float )pos->vz ;
    objs->group_id = map->dg_group ;
    if ( map->light != NULL ) DG_MakePreshade( objs, map->light ) ;
    DG_QueueObjs( objs ) ;
    DG_ActiveShadowFlag = 0 ;
    MapObjs[ N_MapObjs ] = objs ;
    N_MapObjs ++ ;
}

#if 0
/* 影投影モデルをセット */
static	void	InitStageShadow( map, name )
MAP		*map ;
int		name ;
{
    extern int 	DG_ActiveShadowFlag ;
    DG_DEF	*def ;
    DG_OBJS	*objs ;

    ASSERT( N_MapObjs < MAX_MAP_OBJS ) ;
    def = ( DG_DEF * )GV_GetCache( GV_CacheID( name, 'k' ) ) ;
    objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE | DG_FLAG_SHADOWWRITE, 0 ) ;
    DG_QueueObjs( objs );
    DG_ActiveShadowFlag = 1 ;
    MapObjs[ N_MapObjs ] = objs ;
    N_MapObjs ++ ;    
}
#endif

/*-----------------------------------------------------------------*/

/* マップをセット */
void	*NewSetMap( name, mapid )
 int	name, mapid ;
{	
    char	opt, *top ;
    MAP		*map ;
    int		i, strid, group ;
    IVECTOR	pos ;

    i = GetMap( name ) ;
    if ( i == -1 ) {
		map = &Maps[ N_Maps ] ; 
		map->id = GM_GetBit( N_Maps ) ;
		map->name = name ;
		map->dg_group = map->id ;
		map->hzx_group = 0 ;
		map->light = NULL ;
		
		GM_StageMapAll |= map->id ;

		N_Maps ++ ;
    } else {
		map = &Maps[ i ] ;
    }
    top = GCL_NextStrPtr ;
    /* オプションにしたがってマップ設定 */
    while( ( opt = GCL_GetNextOption() ) != 0 ) {
		if ( opt == 'h' ) {
			/* ハザードグループ設定 */
			strid = GCL_GetNextInt() ;
			if ( GCL_NextStr() != NULL ) {
				group = GCL_GetNextInt() ;
			} else {
				group = 0 ;
			}
			InitHazard( map, strid, group ) ;
		} else if ( opt == 'l' ) {
			/* ライト設定 */
			InitLight( map, GCL_GetNextInt() ) ;
		} else if ( opt == 's' ) {
			/* 影モデル設定 */
			//	    InitStageShadow( map, GCL_GetNextInt() ) ;
		} 
    }
    /* モデル設定用にもう一回まわす */
    GCL_SetArgTop( top ) ;
    while( ( opt = GCL_GetNextOption() ) != 0 ) {
		if ( opt == 'k' ) {
			/* モデル設定 */
			strid = GCL_GetNextInt() ;
			if ( GCL_NextStr() != NULL ) {
				GCL_GetNextIV( ( int * )( &pos ) ) ;
			} else {
				pos = DG_ZeroIVector ;
			}
			printf( "stage set %x\n", strid ) ;
			InitStage( map, strid, &pos ) ;
		}
    }
    /* 当たりグループ設定（一応有効にしておくがそのうち消える） */
    if ( GCL_GetOption( 'g' ) != NULL ) {
		map->hzx_group = GV_GetBit( GCL_GetNextInt() );
    }
    return map ;
}

/*-------------------------------------------------------*/

/* マップシステム初期化 */
void	*GM_InitMapSystem( void )
{
    int		i ;
    Work	*work ;
    extern void	DG_ResetFixedLight( void ) ;

    /* 固定ライト初期化 */
    DG_ResetFixedLight();

    N_Maps = 0 ; N_MapObjs = 0 ;
    GM_CurrentMap = 0x7fffffff ;
	GM_StageMapAll = 0 ;
	GM_CurrentStageMap = 0;
    GM_ScriptCurrentMap = 0x7fffffff ;
    for ( i = 0; i < MAX_MAPS; i ++ ){
		Maps[ i ].name = 0 ;
		Maps[ i ].id = 0 ;
		Maps[ i ].dg_group = 0 ;
		Maps[ i ].hzx_group = 0 ;
		Maps[ i ].light = NULL ;
    }

	N_MapChangeTraps = 0 ;

    work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), NULL, Die ) ;
    }

	for( i = 0; i < GM_MAP_CHANL_MAX; i++ ){
		GM_CurrentChanlMap[ i ] = 0;
		GM_ChanlTargetMap[ i ] = 0;
	}
    return work ;
}

/* マップ取得 */
MAP	*GM_GetMap( name )
int	name ;
{
    int		i ;

    for ( i = 0; i < MAX_MAPS; i ++ ) {
#if 0
		if ( Maps[ i ].id == name ) return &Maps[ i ] ;
#else
		// nameが2つ以上指定されていても大丈夫なように変更
		if ( Maps[ i ].id & name ) return &Maps[ i ] ;
#endif
    }
    return NULL ;
}

MAP	*GM_GetMap2( name )
int	name ;
{
    int		i ;

    for ( i = 0; i < MAX_MAPS; i ++ ) {
		if ( Maps[ i ].name == name ) return &Maps[ i ] ;
    }
    return NULL ;
}

int	GM_GetMapID( name )
int	name ;
{
    int		i ;

    for ( i = 0; i < MAX_MAPS; i ++ ) {
		if ( Maps[ i ].name == name ) return Maps[ i ].id ;
    }
    return 0 ;
}

int	GM_GetMapName( int map_id )
{
	int		i ;

	for ( i = 0; i < N_Maps; i ++ ) {
		if ( Maps[ i ].id & map_id ) return Maps[ i ].name ;
	}
	return 0 ;
}

/*-----------------------------------------------------------------*/

/* マップを設定してキャラ起動 */
int	NewMapAbout( void )
{
    MAP		*map ;
    char	*top, *p ;
    int		type, value ;
    int		block ;
	int		curmapbuf ;

    top = p = GCL_NextStr() ;

    /* 実行ブロックを取得 */
    GCL_GetOption( 's' ) ;
    ASSERT( GCL_NextStr() != NULL ) ;
    GCL_GetNextValue( GCL_NextStr(), &type, &block ) ;
    ASSERT( type == GCL_BLOCK ) ;

    GCL_SetArgTop( top ) ;
    /* ScriptCurrentMapを設定してブロック実行 */
	curmapbuf = GM_CurrentMap ;
    GM_ScriptCurrentMap = 0 ;
//    HZX_ResetCurrentGroup() ;
    while( 1 ) {
		p = GCL_GetNextValue( p, &type, &value ) ;
		if ( !( type & GCL_STRID ) && !( type & GCL_INT ) ) break ;
		map = GM_GetMap2( value ) ;
		if ( map != NULL ) {
			GM_ScriptCurrentMap |= map->id ;
//			HZX_AddCurrentGroup( map->hzx_group ) ;
		}
    }
	printf( "map about %x\n", GM_ScriptCurrentMap ) ;
    GM_CurrentMap = GM_ScriptCurrentMap ;
    GCL_ExecBlock( ( char * )block, NULL ) ;
    
    GM_ScriptCurrentMap = 0x7fffffff ;
	GM_CurrentMap = curmapbuf ;
//    HZX_ResetCurrentGroup() ;

    return 1 ;
}

/* ---------------------------------------------------------------------- */
/*
	マップ表示系
*/
#if 0
static void MapOff( MAP *map )
{
	DG_DeleteCurrentGroup( map->dg_group ) ;
	HZX_DeleteCurrentGroup( map->hzx_group ) ;
	GM_CurrentStageMap &= ~map->id ;
}

static void MapOn( MAP *map )
{
	DG_AddCurrentGroup( map->dg_group ) ;
	HZX_AddCurrentGroup( map->hzx_group ) ;
	GM_CurrentStageMap |= map->id ;
	//	DG_SetFixedLight( map->light ) ;
}
#endif

/* ---------------------------------------------------------------------- */
/*
	外部呼び出し関数
*/

static void wakeup_map_change( void *this )
{
	/*
		マップ切替え待ちキャラクタにマップ切替えシグナルを送る
	*/
	if( GV_CHECK_WAIT( this, GM_ACTOR_WAIT_MAP_CHANGE ) ){
		GV_WakeupActor( this, GM_ACTOR_WAIT_MAP_CHANGE );
	}
}

static inline void SendMapChangeMessage()
{
	GV_CheckActorAll( wakeup_map_change );
}

#if 0
void GM_ShowMap( int map )
{
	int onmap, offmap;
	int id;

	onmap = ~GM_CurrentStageMap & map;
	offmap = GM_CurrentStageMap & ~map;
	while( onmap != 0 ){
		id = GM_GetID( onmap );
		MapOn( &( Maps[ id ] ) );
		onmap &= ~GM_GetBit( id );
	}
	while( offmap != 0 ){
		id = GM_GetID( offmap );
		MapOff( &( Maps[ id ] ) );
		offmap &= ~GM_GetBit( id );
	}
	GV_CheckActorAll( wakeup_map_change );
	printf( "SHOWMAP %08X HZD %08X\n", GM_CurrentStageMap, HZX_CurrentGroupID );
}
#endif

/* マップ表示コマンド(シナリオ起動) */
int	NewShowMap( void )
{
#if 0
    char	opt ;
    int		name ;
    MAP		*map ;

    while( ( opt = GCL_GetNextOption() ) != 0 ) {
		if ( opt == 'a' || opt == 's' || opt == 'd' ) {
			while( GCL_NextStr() != NULL ) {
				name = GCL_GetNextInt() ;
				map = GM_GetMap2( name ) ;
				if ( map != NULL ) {
					if ( opt == 'd' ) {
						MapOff( map );
					} else {
						MapOn( map );
					}
				}
			}
		}
    }
#else
	/* シナリオでのmap show */
	/* シナリオ指定は０チャンネルだけ */
    char	opt ;
    int		name ;
	int		map;

	map = GM_CurrentStageMap;
    while( ( opt = GCL_GetNextOption() ) != 0 ) {
		if ( opt == 'a' || opt == 's' || opt == 'd' ) {
			while( GCL_NextStr() != NULL ) {
				int mp;
				name = GCL_GetNextInt() ;
				mp = GM_GetMapID( name ) ;
				if ( opt == 'd' ) {
					map &= ~mp;
				} else {
					map |= mp;
				}
			}
		}
    }
	GM_SetChanlTargetMap( 0, map );
	GM_CalcChanlMap();
#endif
    return 1 ;
}

void GM_ChangeControlMapFromHzxId( CONTROL *ctrl, HZX_GROUP_ID hzx_id )
{
    /* hzx_id はかならず1ビット */
    int		i ;

    for ( i = 0; i < N_Maps; i ++ ) {
		if ( Maps[ i ].hzx_group == hzx_id ){
			ctrl->hzx_id = hzx_id;
			ctrl->map = Maps[ i ].id;
			ctrl->dg_group_id = Maps[ i ].dg_group;
			return;
		}
    }
    ASSERT( FALSE );
    return;
}

void GM_ChangeControlMapFromMapId( CONTROL *ctrl, int map )
{
	/* mapは複数のビットのこともある */

	int hzx_group;
	int dg_group;

	hzx_group = 0;
	dg_group = 0;

	ctrl->map = map ;
	while( map != 0 ){
	    int no;
	    no = GV_GetNo( map );
	    map &= ~GV_GetBit( no );
	    
	    hzx_group |= Maps[ no ].hzx_group;
	    dg_group |= Maps[ no ].dg_group;
	}
	ASSERT( hzx_group != 0 && dg_group != 0 );
	ctrl->hzx_id = hzx_group;
	ctrl->dg_group_id = dg_group;
}

/* 位置からマップＩＤ（単一）を返す */
int		GM_GetMapIDfromPos( int map, FVECTOR *pos )
{
	int					z ;
	HZX_GROUP_ID		hzx_id, hid ;

	if ( map == 0 ) map = GM_StageMapAll ;
	if ( GM_IsOneID( map ) ) return map ;
	hzx_id = GM_GetHzxGroupID( map ) ;
	hid = HZX_GetHzxIDbyZoneEx( hzx_id, pos, &z, 1 ) ;
	return GM_GetMapIDfromHzxGroupID( hid ) ;
}

/* ---------------------------------------------------------------------- */
/*
	複数チャンネルのカメラ対応
*/

void GM_CalcChanlMap( void )
{
	/* 表示中の各チャンネルのマップを検索してアクティブなマップを決める */
	int i;
	int active_map;
	int	prev_map ;

	prev_map = GM_CurrentChanlMap[ 0 ] ;
	active_map = 0;
	for( i = 0; i < GM_MAP_CHANL_MAX; i++ ){
		if( DG_Chanl( i )->flag != 0 ){
			int map, dg_map;
			/* 表示中の時だけ */
			map = GM_GetConnectMapID( GM_ChanlTargetMap[ i ] );
#if 0
			ASSERT( map != 0 );
#else
			if( map == 0 ){
				printf( "CalcChanlMap:NOMAP\n" );
				continue;
			}
#endif

			GM_CurrentChanlMap[ i ] = map;
			dg_map = GM_GetDGGroupID( map );
			DG_Chanl( i )->group_id = dg_map;
			active_map |= map;
		} else {
			GM_CurrentChanlMap[ i ] = 0;
		}
	}

	/* アクティブなチャンネルがない時は現状維持 */
	if( active_map == 0 ) return;

	HZX_SetCurrentGroup( GM_GetHzxGroupID( active_map ) );
	GM_CurrentStageMap = active_map;

	/* コールバック */
	if ( prev_map != GM_CurrentChanlMap[ 0 ] ) {
		GCL_ARGS		args ;
		int				buf[ 1 ] ;

		args.argc = 2 ;
		args.argv = buf ;
		buf[ 0 ] = GM_CurrentChanlMap[ 0 ] ;	/* チャンネル０のみ */
		GM_CallCallbackProc2( GM_CALLBACK_ACTIVE_MAP_CHANGE, &args ) ;
		SendMapChangeMessage();
	}
}

void GM_ChanlControl( int chanl, int flag )
{
	DG_Chanl( chanl )->flag = flag;

	if( chanl < GM_MAP_CHANL_MAX ){
		GM_CalcChanlMap();
	}
}

/* ---------------------------------------------------------------------- */

/* マップチェンジトラップ登録 */
int	GM_ComSetMapChangeTrap( void )
{
#if 0
	HZX_GRP			*grp ;
	HZX_BLOCK		*blk ;
	HZX_TRP			*trp ;
	HZX_HDL			*hdl ;
	int				n_blocks, n_traps ;
	int				bit, no ;
	int				find ;
#ifdef HZX_DTRP
	HZX_D_TRP		*dtrp ;
#endif
#endif
	HZX_GROUP_ID	hzx_id ;
	MCT				*mct ;
	int				trap_name ;
	int				chara_name ;
	int				now_map, next_map ;

	ASSERT( N_MapChangeTraps < 8 ) ;
	mct = &MapChangeTrap[ N_MapChangeTraps ] ;
	
	GCL_GetOption( 't' ) ;
	trap_name = GCL_GetNextInt() ;
	chara_name = GCL_GetOptionValue( 'n', -1 ) ;

	GCL_GetOption( 'm' ) ;
	now_map = GCL_GetNextInt() ;
	now_map = GM_GetMapID( now_map ) ;
	ASSERT( now_map != 0 ) ;
	next_map = GCL_GetNextInt() ;
	next_map = GM_GetMapID( next_map ) ;
	ASSERT( next_map != 0 ) ;

	hzx_id = GM_GetHzxGroupID( now_map ) ;
#if 0
	hdl = HZX_GetCurrentHzx() ;
	find = 0 ;
	while( hzx_id != 0 ) {
		no = GV_GetNo( hzx_id ) ;
		bit = GV_GetBit( no ) ;
		hzx_id &= ~bit ;
		
		grp = hdl->def->groups + no ;
		n_blocks = grp->n_blocks ;
		blk = grp->blocks ;
		while( -- n_blocks >= 0 ) {
			n_traps = blk->n_traps ;
			trp = blk->traps ;
			while( -- n_traps >= 0 ) {
				if ( trap_name == trp->name_id ) {
					mct->b1.vx = ( float )( blk->tx + trp->b1.vx ) ;
					mct->b1.vy = ( float )( blk->ty + trp->b1.vy ) ;
					mct->b1.vz = ( float )( blk->tz + trp->b1.vz ) ;
					mct->b2.vx = ( float )( blk->tx + trp->b2.vx ) ;
					mct->b2.vy = ( float )( blk->ty + trp->b2.vy ) ;
					mct->b2.vz = ( float )( blk->tz + trp->b2.vz ) ;
					find = 1 ;
					goto find_trap ;
				}
				trp ++ ;
			}
			blk ++ ;
		}
#ifdef HZX_DTRP
		dtrp = grp->dynamics->traps ;
		while( dtrp != NULL ) {
			if ( trap_name == dtrp->name_id ) {
				DG_COPY_VEC( &mct->b1, &dtrp->b1 ) ;
				DG_COPY_VEC( &mct->b2, &dtrp->b2 ) ;
				find = 1 ;
				goto find_trap ;
			}
			dtrp = dtrp->next ;
		}
#endif
	}	
find_trap :
	ASSERT( find == 1 ) ;
#endif
	mct->name_id = trap_name ;	
	mct->chara_name = chara_name ;
	mct->now_map = now_map ;
	mct->next_map = next_map ;

	N_MapChangeTraps ++ ;

	return 1 ;
}

static	inline	void	MakeBound( FVECTOR *b1, FVECTOR *b2,
								  FVECTOR *v1, FVECTOR *v2 ) 
{
#if 1 //BP_ASM
   BP_Vec4_MinVec( b1, v1, v2 );
   BP_Vec4_MaxVec( b2, v1, v2 );
#else

#ifdef PSX2	
    asm __volatile__("
	lqc2    	vf4,0x0(%2)
	lqc2    	vf5,0x0(%3)
    vmini.xyzw	vf6,vf4,vf5
    vmax.xyzw	vf7,vf4,vf5
	sqc2    	vf6,0x0(%0)
	sqc2    	vf7,0x0(%1)
	": : "r" (b1), "r" (b2), "r" (v1), "r" (v2) : "memory" ) ;
#endif

#ifdef KP_XBOX
	D3DXVec3Minimize((D3DXVECTOR3*)b1, (D3DXVECTOR3*)v1, (D3DXVECTOR3*)v2);
	D3DXVec3Maximize((D3DXVECTOR3*)b2, (D3DXVECTOR3*)v1, (D3DXVECTOR3*)v2);
#endif	

#endif
}

/* マップチェンジトラップチェック */
/* 都合上、D_TRPには非対応 */
int	  	GM_CheckMapChangeTrap( CONTROL *ctrl, FVECTOR *p )
{
	int					i, n, c, n_traps ;
	int					find ;
	HZX_TRP				*trp ;
	HZX_BLOCK			*blk ;
	MCT					*mct ;
	FVECTOR				b1, b2, mctb1, mctb2 ;

	n = N_MapChangeTraps ;
	if ( n == 0 ) return 0 ;
	mct = MapChangeTrap ;
	c = 0 ;
	for ( ; n > 0; n --, mct ++ ) {
		if ( ctrl->map != mct->now_map || ctrl->map == mct->next_map ) {
			continue ;
		}
		if ( mct->chara_name != -1 && mct->chara_name != ctrl->name ) {
			continue ;
		}
		if ( !( ctrl->skip_flag & CTRL_SKIP_TRAP ) ) {
			for ( i = 0; i < ctrl->evt.n_inside; i ++ ) {
				if ( ctrl->evt.inside[ i ] == mct->name_id ) {
					goto check_map_change_trap__find ;
				}
			}
			continue ;
		}
		find = 0 ;
		blk = HZX_GetInsideBlock( ctrl->hzx_id, &ctrl->mov ) ;
		if ( blk == NULL ) continue ;
		n_traps = blk->n_traps ;
		trp = blk->traps ;
		while( -- n_traps >= 0 ) {
			if ( mct->name_id == trp->name_id ) {
				mctb1.vx = ( float )( blk->tx + trp->b1.vx ) ;
				mctb1.vy = ( float )( blk->ty + trp->b1.vy ) ;
				mctb1.vz = ( float )( blk->tz + trp->b1.vz ) ;
				mctb2.vx = ( float )( blk->tx + trp->b2.vx ) ;
				mctb2.vy = ( float )( blk->ty + trp->b2.vy ) ;
				mctb2.vz = ( float )( blk->tz + trp->b2.vz ) ;

				if ( c == 0 ) {
					MakeBound( &b1, &b2, p, &ctrl->mov ) ;
					c = 1 ;
				}		

				if ( b1.vx > mctb2.vx || b2.vx < mctb1.vx ||
					 b1.vy > mctb2.vy || b2.vy < mctb1.vy ||
					 b1.vz > mctb2.vz || b2.vz < mctb1.vz ) {
					goto check_map_change_trap__find_trap_loop_end ;
				}
				find = 1 ;
				break ;
			}
check_map_change_trap__find_trap_loop_end :
	        trp ++ ;
		}
		if ( find == 0 ) continue ;
check_map_change_trap__find :
#ifdef DEBUG_MODE
		printf( "%x : map change %x by map change trap %x\n", 
			     ctrl->name, mct->next_map, mct->name_id ) ;
#endif
		GM_ChangeControlMapFromMapId( ctrl, mct->next_map ) ;
		ctrl->addr = HZX_GetAddress( ctrl->hzx_id, &ctrl->mov, -1 ) ;
		return 1 ; /* 同時に二つのマップチェンジは無し */
	}
	return 0 ;
}

/* マップ表示中チェック（command） */
int		NewCheckMapIsDisplay( void )
{
	int		which, map ;

	which = GCL_GetOptionValue( 'w', 0 ) ;
	map = GM_GetMapID( GCL_GetOptionValue( 'm', 0 ) ) ;
	if ( GM_CurrentChanlMap[ which ] & map ) return 1 ;
	return 0 ;
}
