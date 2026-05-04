//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    flow_paper.c
	浮遊する紙
	2000/01/22 T.Shibata

	$Id: flow_paper.c,v 1.1.1.3 2002/11/19 11:48:36 Yoshizawa1 Exp $
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
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define	PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define	N_CHECK_MAX		(4)


typedef struct {
	DG_OBJS				*objs;
    //VERTEX_ANIME_WORK	*anime;
    //CV2_DEF  			*c_def;
	short				flags;
	short				pad;
	
	FVECTOR				pos;
	FVECTOR				vec;
	FVECTOR				acc;
	FVECTOR				rot;
	FVECTOR				center;				//
	FVECTOR				wide;				//
}PAPER;

typedef struct {
	GV_ACT_EX		actor;
	int				name;
	int				map;
	
	PAPER			*paper;
	int				n_objs;

	int				check_name[N_CHECK_MAX];
	float			check_len[N_CHECK_MAX];
	FVECTOR			check_diff[N_CHECK_MAX];
	CONTROL			*con[N_CHECK_MAX];
	FVECTOR			pre_pos[N_CHECK_MAX];
	int				n_check;
} Work;

#define		BOUND_K	(0.8f)
#define		FLAGS_OUTBOUND_X	(0x01)
#define		FLAGS_OUTBOUND_Y	(0x02)
#define		FLAGS_OUTBOUND_Z	(0x04)
#if 0
#define		CLIPCHECK_OVER_X	(0x01)
#define		CLIPCHECK_UNDER_X	(0x02)
#define		CLIPCHECK_OVER_Y	(0x04)
#define		CLIPCHECK_UNDER_Y	(0x08)
#define		CLIPCHECK_OVER_Z	(0x10)
#define		CLIPCHECK_UNDER_Z	(0x20)
#else
#define		CLIPCHECK_OVER	(0x01)
#define		CLIPCHECK_UNDER	(0x02)
#endif


static CONTROL* SearchControl( int name, int map )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		if ( ( control->map & map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("puddle.c: search faild control !!\n");
	return ( NULL );
}

static void CheckBound( Work *work, PAPER *paper )
{
	//BOUND_K*wideから出た時に加速度設定　xyz各		FLAG立て
	//外に出たら補正無理やり戻して法線方向に反転
	//中に入ったら等速運動　xyz各						FLAG降ろし
	//FVECTOR		*pos = &paper->pos;
	FVECTOR		l_bound_min,l_bound_max;
	FVECTOR		m_bound_min,m_bound_max;
	FVECTOR		inf,fvtemp;
	float		*l_max,*l_min,*m_max,*m_min;
	float		*vec,*acc,*pos,*inf_m;
	int			i;
	CONTROL		**control = work->con;
	float		inner;
	FVECTOR		*ppos = &paper->pos;
	FVECTOR		*cppos,*cpos,*cfor;
#if 1
	for( i = 0; i < work->n_check; i++ ){
		float		len_diff;
//		float		len_move;

		//PRINT_PFVEC(0,&(*control)->mov);
		cpos = &(*control)->mov;
		cppos = &work->pre_pos[i];
		cfor = &work->check_diff[i];
		//範囲チェック

		//キャラからの影響
		DG_COPY_VEC( &inf, &DG_ZeroVector );
		_sceVu0SubVector( &fvtemp, cpos, ppos );	//紙からキャラへの距離
		len_diff = VectorLength(&fvtemp);
		inner = _sceVu0InnerProduct( cfor, &fvtemp );
		if( inner < 0.0f ){
			if( 10.0f < len_diff && len_diff < 16.0f*work->check_len[i] ){
				//printf("inf_max\n");
				//inner = _sceVu0InnerProduct( cfor, &fvtemp );
				//_sceVu0MulVector( &fvtemp, &fvtemp, &fvtemp );
				//fvtemp.vx = 1.0f/fvtemp.vx;
				//fvtemp.vy = 1.0f/fvtemp.vy;
				//fvtemp.vz = 1.0f/fvtemp.vz;
				//_sceVu0ScaleVector( &fvtemp, &fvtemp, inner );
				//_sceVu0AddVector( &inf, &inf, &fvtemp );
				paper->rot.vx += PI*work->check_len[i]/len_diff/4.0f;
			}else{
				paper->rot.vx += PI/128.0f;
			}
		}else{
			if( 10.0f < len_diff && len_diff < 32.0f*work->check_len[i] ){
				//printf("inf_max\n");
				//inner = _sceVu0InnerProduct( cfor, &fvtemp );
				//_sceVu0MulVector( &fvtemp, &fvtemp, &fvtemp );
				//fvtemp.vx = 1.0f/fvtemp.vx;
				//fvtemp.vy = 1.0f/fvtemp.vy;
				//fvtemp.vz = 1.0f/fvtemp.vz;
				//_sceVu0ScaleVector( &fvtemp, &fvtemp, inner );
				//_sceVu0AddVector( &inf, &inf, &fvtemp );
				paper->rot.vx += PI*work->check_len[i]/len_diff/4.0f;
			}else{
				paper->rot.vx += PI/128.0f;	
			}
		}
		if(paper->rot.vx > PI) paper->rot.vx -= 2.0f*PI;
		control++;
	}
#endif
	//PRINT_PFVEC(0,&inf);	
	_sceVu0ScaleVector( &l_bound_min, &paper->wide, BOUND_K );
	_sceVu0AddVector( &l_bound_max, &paper->center, &l_bound_min );
	_sceVu0SubVector( &l_bound_min, &paper->center, &l_bound_min );
	
	_sceVu0AddVector( &m_bound_max, &paper->center, &paper->wide );
	_sceVu0SubVector( &m_bound_min, &paper->center, &paper->wide );

	pos = (float*)&paper->pos;
	vec = (float*)&paper->vec;
	acc = (float*)&paper->acc;
	l_max = (float*)&l_bound_max;
	l_min = (float*)&l_bound_min;
	m_max = (float*)&m_bound_max;
	m_min = (float*)&m_bound_min;
	inf_m = (float*)&inf;
	
	for( i = 0; i < 3; i++ ){
		if(paper->flags&(1<<i)){
			//等加速運動
			//外に出たか
			//中に入ったか
			if( *l_min < *pos && *pos < *l_max ){
				*acc = 0.0f;
				paper->flags &= ~(1<<i);
			}
		}else{
			//等速運動(チェックのみ)
			//*pos += *inf_m;
			//if(*vec < 0.0f) *vec = (*vec - 2.0f)/2.0f;
			//else *vec = (*vec + 2.0f)/2.0f;

			if( *pos > *l_max ){
				//加速度設定
				*acc = -*vec**vec/2.0f/(*m_max-*pos);
				paper->flags |= (1<<i);
			}else if( *pos < *l_min ){
				//加速度設定
				*acc = -*vec**vec/2.0f/(*m_min-*pos);
				paper->flags |= (1<<i);
			}
		}
		pos++; vec++; acc++; inf_m++;
		l_max++; l_min++;
		m_max++; m_min++;
	}
	if(0){
		FVECTOR bound[2];
		_sceVu0ScaleVector( &bound[1], &paper->wide, BOUND_K );
		_sceVu0AddVector( &bound[0], &paper->center, &bound[1] );
		_sceVu0SubVector( &bound[1], &paper->center, &bound[1] );
		NewBoundingBoxView( &bound[0], &bound[1], 32, 32, 128 );
	}

}
static void InvPaparObj( Work *work )
{
	int		i;
	PAPER	*paper = work->paper;
	
	for( i = 0; i < work->n_objs; i++ ){
		DG_InvisibleObjs( paper->objs );
		paper++;
	}
}

static void VisPaparObj( Work *work )
{
	int		i;
	PAPER	*paper = work->paper;
	
	for( i = 0; i < work->n_objs; i++ ){
		DG_VisibleObjs( paper->objs );
		paper++;
	}
}

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
		  case 0:
			VisPaparObj( work );
			break;
		  case 1:
			InvPaparObj( work );
			return 1;
			break;
		  default:
			break;
		}
		msg--;
	}

	return 0;
}

static void Act(Work *work)
{
	int		i;
	PAPER	*paper = work->paper;
	if( CheckMesgParam( work ) ){
		GV_WaitMessage( work, work->name );
		return;
	}
	
	for( i = 0; i < work->n_objs; i++ ){
		FMATRIX		*world = &paper->objs->world;
//		FVECTOR		pos,vec;
#if 0
		//重いねー
		//v_anime
		if(paper->anime->count){
			SimpleVertexAnimation( paper->anime );
		}else{
			paper->anime->p[0] = ((paper->anime->p[0]>0.5f)?0.0f:1.0f);
			paper->anime->p[1] = ((paper->anime->p[1]>0.5f)?0.0f:1.0f);
			paper->anime->count = 300;
		}
#endif
		//rot(rot)
		//paper->rot.vz += PI/360.0f;
		//if(paper->rot.vz > PI) paper->rot.vz -= 2.0f*PI;

		//move(位置)
//		DG_COPY_VEC( &vec, &paper->vec );
//		vec.vw = 0.0f;
//		_sceVu0ScaleVector( &vec, &vec, paper->vec.vw );
		_sceVu0AddVector( &paper->vec, &paper->vec, &paper->acc );
		_sceVu0AddVector( &paper->pos, &paper->pos, &paper->vec );

		//chara_check(速度と位置)
		//CheckChara( work, paper );//バウンドチェックに埋め込み
		
		//bound_check(速度と位置)
#if 0
		{
			FVECTOR	bound[2];
			_sceVu0AddVector( &bound[0], &paper->center, &paper->wide );
			_sceVu0SubVector( &bound[1], &paper->center, &paper->wide );
			NewBoundingBoxView( &bound[0], &bound[1], 128, 32, 32 );
		}
#endif
		CheckBound( work, paper );

		_sceVu0RotMatrix( world, &DG_UnitMatrix, &paper->rot );
		//DG_COPY_VEC( &paper->pos, &pos );
		DG_COPY_VEC( (FVECTOR*)world->m[3], &paper->pos );
		paper++;
	}
	{
		CONTROL		**control = work->con;
		for( i = 0; i < work->n_check; i++ ){
			//PRINT_PFVEC(i,&(*control)->mov);
			_sceVu0SubVector( &work->check_diff[i], &(*control)->mov, &work->pre_pos[i] );
			//printf("move"); PRINT_PFVEC(0,&work->check_diff[i]);
			work->check_len[i] = VectorLength(&work->check_diff[i]);
			DG_COPY_VEC( &work->pre_pos[i], &(*control)->mov );
			control++;
		}
	}
	
	
	//PRINT_PFVEC(0,&work->con[0]->mov);
	//PRINT_PFVEC(1,&work->con[0]->step);
}

static void Die(Work *work)
{
	int i;
	PAPER	*paper = work->paper;
	if(paper){
		for( i = 0; i < work->n_objs; i++ ){
			DG_DequeueObjs( paper->objs );
			DG_FreeObjs( paper->objs );
			//Vアニメ
			//ExitVertexAnimation( paper->anime ) ;
			paper++;
		}
		GV_Free( work->paper );
	}
}

static DG_OBJS *ObjsInit( int mdl_code, FVECTOR *pos )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( mdl_code, 'k' ) ) ;
	if(!def){
		printf("ERR!! NO MODEL!!\n");
		return NULL;
	}

	objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 );
	if(!objs) return NULL;

	if(DG_QueueObjs( objs )<0) return NULL;

	//DG_SetLightMatrix( objs, work->lights );
#ifdef BP_PS2
	DG_SetPos2( pos, &(SVECTOR){0,0,0,0});
#else
	DG_SetPos2( pos, &DG_ZeroSVector);
#endif
	DG_PutObjs( objs );

	return objs;
}
#if 0
static CV2_DEF *CV2Init( int mdl_code )
{
	CV2_DEF		*def;

	def = (CV2_DEF*)GV_GetCache( GV_CacheID( mdl_code, 'c' ) ) ;
	if(!def){
		printf("ERR!! NO MODEL!!\n");
		return NULL;
	}
	return def;
}
#endif

static int GetTrapInfo( FVECTOR *center, FVECTOR *wide, HZX_GROUP_ID group_id, int name_id )
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
		if( trap->name_id == name_id ){

			bound0.vx = (float)trap->b1.vx + (float)block->tx;
			bound0.vy = (float)trap->b1.vy + (float)block->ty;
			bound0.vz = (float)trap->b1.vz + (float)block->tz;
			bound0.vw = 1.0f;

			bound1.vx = (float)trap->b2.vx + (float)block->tx;
			bound1.vy = (float)trap->b2.vy + (float)block->ty;
			bound1.vz = (float)trap->b2.vz + (float)block->tz;
			bound1.vw = 1.0f;

			_sceVu0SubVector( &fvtemp0, &bound1, &bound0 );
			_sceVu0ScaleVector( &fvtemp0, &fvtemp0, 0.5f );
			_sceVu0AddVector( center, &fvtemp0, &bound0 );
			_sceVu0SubVector( wide, center, &bound0 );

			//PRINT_PFVEC(0,center);
			//PRINT_PFVEC(0,wide);
			
			return 0;
		}
	}
	block++;
	}
	return (-1);
}

static int GetOptions( Work *work )
{
	PAPER		*paper;
	int			n_objs,i=0,mdl_code,code,name;//,j
	FVECTOR		pos;
	CONTROL		*control;
	int			hzx_group;
	int			trap_code;
	work->n_check = GCL_GetOptionValue( 'c', 0 );
	
	if( work->n_check && GCL_GetOption( 'o' ) != NULL){
		
		while( i < N_CHECK_MAX && i < work->n_check ){

			name = GCL_GetNextInt();
			control = SearchControl( name, work->map );
			if(!control){
				printf("No Control code <%d>\n",name);
				work->n_check--;
			}else{
				work->check_name[i] = name;
				work->con[i] = control;
				work->check_len[i] = 0.0f;
				DG_COPY_VEC( &work->pre_pos[i], &control->mov );
				DG_COPY_VEC( &work->check_diff[i], &DG_ZeroVector );
				i++;
			}
		}
	}
	
	n_objs = work->n_objs = GCL_GetOptionValue( 'n', 0 );

	paper = work->paper = GV_Malloc( sizeof(PAPER) * n_objs );

	if(!paper){
		printf("ERR!! NO MEM!!<flow_paper.c>\n");
		return 1;
	}
	
	for( i = 0; i < n_objs; i++ ){
		code = GCL_GetNextOption();
		if(code == 'm') mdl_code = GCL_GetNextInt();
		else mdl_code = 0;

#if 0
		code = GCL_GetNextOption();
		if(code == 'p'){
			pos.vx = (float)GCL_GetNextInt();
			pos.vy = (float)GCL_GetNextInt();
			pos.vz = (float)GCL_GetNextInt();
		}else{
			pos.vx = 0.0f;
			pos.vy = 0.0f;
			pos.vz = 0.0f;
		}
		pos.vw = 1.0f;
		
		code = GCL_GetNextOption();
		if(code == 'b'){
			paper->wide.vx = (float)GCL_GetNextInt();
			paper->wide.vy = (float)GCL_GetNextInt();
			paper->wide.vz = (float)GCL_GetNextInt();
		}else{
			paper->wide.vx = 1000.0f;
			paper->wide.vy = 1000.0f;
			paper->wide.vz = 1000.0f;
		}
#else
		
		code = GCL_GetNextOption();
		if( code != 'g' ){ printf("not input funa hzx group\n"); }
		hzx_group = ( HZX_GROUP_ID )GV_GetBit( GCL_GetNextInt() );
		code = GCL_GetNextOption();
		if( code != 't' ){ printf("not input funa trap group\n"); }
		trap_code = GCL_GetNextInt();
		if(GetTrapInfo( &pos, &paper->wide, hzx_group, trap_code )){
			pos.vx = 0.0f;
			pos.vy = 0.0f;
			pos.vz = 0.0f;
			pos.vw = 1.0f;
			paper->wide.vx = 1000.0f;
			paper->wide.vy = 1000.0f;
			paper->wide.vz = 1000.0f;
			paper->wide.vz = 0.0f;

			printf("駄目ジャーん\n");
		}
#endif
		
		paper->wide.vx = fabs(paper->wide.vx);
		paper->wide.vy = fabs(paper->wide.vy);
		paper->wide.vz = fabs(paper->wide.vz);

		paper->wide.vw = 0.0f;

		paper->objs = ObjsInit( mdl_code, &pos );
		paper->objs->objs[1].flag |= DG_FLAG_INVISIBLE;
//		paper->c_def = CV2Init( mdl_code );
		//DG_COPY_VEC( &paper->pos, &pos );
		DG_COPY_VEC( &paper->center, &pos );
		paper->pos.vx = pos.vx;// + frnd()*paper->wide.vx*BOUND_K;
		paper->pos.vy = pos.vy;// + frnd()*paper->wide.vy*BOUND_K;
		paper->pos.vz = pos.vz;// + frnd()*paper->wide.vz*BOUND_K;
		paper->pos.vw = 1.0f;
		
		paper->rot.vx = frnd()*PI;
		paper->rot.vy = 0.0f;//frnd()*PI;
		paper->rot.vz = frnd()*PI;
		paper->rot.vw = 0.0f;

		paper->vec.vx = frnd();
		paper->vec.vy = frnd();
		paper->vec.vz = frnd();
		paper->vec.vw = 0.0f;
		_sceVu0Normalize( &paper->vec, &paper->vec );
		//_sceVu0ScaleVector( &paper->vec, &paper->vec, 1.0f );
		
//		paper->vec.vw = 2.0f*rnd()+0.5f;
		
		//PRINT_PFVEC(i,&pos);
		//PRINT_PFVEC(i,&paper->vec);
		//PRINT_PFVEC(i,&paper->bound);
		
		//Ｖアニメ初期化
#if 0
		paper->anime = InitVertexAnimation( paper->objs->objs,
											paper->c_def->models,
											DG_VANIME_VERTS|DG_VANIME_NORMS,
											paper->c_def->n_models );
		if(!paper->anime) return -1;

		for ( j = 0; j < paper->c_def->n_models; j++ ){
		    paper->anime->key[j] = &paper->c_def->models[j];
			paper->anime->p[j] = 0.0f;
	    }
		paper->anime->p[0] = 1.0f;
		paper->anime->count = 0;

		SimpleVertexAnimation( paper->anime ) ;
#endif
		paper->acc.vx = 0.0f;
		paper->acc.vy = 0.0f;
		paper->acc.vz = 0.0f;
		paper->acc.vw = 0.0f;
		paper->flags = 0;
		paper++;
	}
	return 0;
}

static int GetResources( Work *work )
{
	work->n_check = 0;
	
	if(GetOptions( work )) return -1;

	return 0;
}

void *NewFlowPaper( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		
		work->name = name;
		work->map = map;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		printf("new paper_flow.c\n");
	}

	return (void *)work ;
}
