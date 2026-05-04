//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	debris_cm.c
	破片パーティクル（コモデル）
	
	2000/12/25 T.Shibata
	
	$Id: debris_cm.c,v 1.1.1.3 2002/11/19 11:48:30 Yoshizawa1 Exp $

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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"
#include	"../../mode/demo/eft_con.h"

#define CLOCK_COUNT	(BP_BASE_TICK())


// -----------------------------------------------------------
//			extern
extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

extern void OK_frnd_to_scr( void *, int );
//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define	PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)


#define GRAVITATION	(P_GRAVITY)
#define LIFE		(60*CLOCK_COUNT)

#define		MEM_SCR_POS	((void*)(SCRPAD_ADDR))

typedef	struct	{
	GV_ACT_EX	actor;
	DG_COMDL	*comdl;
	int			n_comdl;
	int			flags;
	
	int			timer;
	int			map_id;
	float		velocity;

	FVECTOR		dvec[0];
} Work;

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

static void NoLevelCheck_Act(Work *work)
{
	int				i;
	//float			*p_randam;
	DG_COMDL_POS 	*comdl_pos;
	FVECTOR			*dvec;
		
	comdl_pos = work->comdl->pos;
	dvec = MEM_SCR_POS;//work->dvec;

//	TS_Scr_Mem( MEM_SCR_POS, work->dvec, sizeof(FVECTOR), work->n_comdl );
//	OK_frnd_to_scr( MEM_SCR_POS+sizeof(FVECTOR)*work->n_comdl, 0x1000/4 );
//	p_randam = (float *)MEM_SCR_POS+sizeof(FVECTOR)*work->n_comdl;
	TS_Scr_Mem( MEM_SCR_POS, work->dvec, sizeof(FVECTOR), work->n_comdl );
	
	for( i = 0; i < work->n_comdl; i++ ){
		FVECTOR to_pos;
		//PRINT_PFVEC(i,dvec);
		dvec->vy += GRAVITATION;
		DG_COPY_VEC(&to_pos,dvec);
		to_pos.vw = 0.0f;
		_sceVu0AddVector( &to_pos, (FVECTOR*)comdl_pos->world.m[3], &to_pos );
		//PRINT_PFVEC(i,dvec);
		if( dvec->vw > to_pos.vy ){
#if 1
			float len = VectorLength(dvec) * 0.7f;
			float rot_x,rot_y,temp = dvec->vw;
			FVECTOR force = {0.0f,len,0.0f,0.0f};
			FMATRIX world;
			
			rot_x = PI * 0.166f * frnd();
			rot_y = PI * frnd();
			_sceVu0RotMatrixX( &world, &DG_UnitMatrix, rot_x );
			_sceVu0RotMatrixY( &world, &world, rot_y );		
			_RotVector( dvec, &world, &force );

			dvec->vw = temp;
			to_pos.vy = temp;
#else
			//バッグってハニー
			//sin(x) = 1.0f*x + s1*x^3 + s2*x^5 + s3*x^7 + s4*x^9
			float	temp = dvec->vw;
			FVECTOR rot;
			static FVECTOR S4 = {
				(-1.0f/(3.0f*2.0f)),
				( 1.0f/(5.0f*4.0f*3.0f*2.0f)),
				(-1.0f/(7.0f*6.0f*5.0f*4.0f*3.0f*2.0f)),
				( 1.0f/(9.0f*8.0f*7.0f*6.0f*5.0f*4.0f*3.0f*2.0f))
			};
			
			rot.vx = PI*0.25f * frnd();//*p_randam; p_randam++;//frnd();
			rot.vw = PI/2.0f * frnd();//*p_randam; p_randam++;//frnd();
			rot.vy = rot.vy + PI/2.0f;
			rot.vz = rot.vw + PI/2.0f;

			rot.vx *= -1.0f;
			rot.vw *= -1.0f;

			if(rot.vz > PI/2.0f) rot.vz -= PI;

            asm ("
			lqc2			vf7,0x00(%0)
			lqc2			vf8,0x00(%2)
			lqc2			vf9,0x00(%1)
			qmtc2.ni		%3,vf6

			vmul.xyzw	    vf10,vf7,vf7
			vmul.xyz		vf16,vf8,vf8
			vmulaw.xyzw		ACC,vf7,vf00w
			vmul.xyzw		vf11,vf7,vf10
			vmaddax.xyzw	ACC,vf11,vf9x
			vmul.xyzw		vf12,vf10,vf11
			vmadday.xyzw	ACC,vf12,vf9y
			vmul.xyzw		vf11,vf10,vf12
			vmaddaz.xyzw	ACC,vf11,vf9z
			vmul.xyzw		vf12,vf10,vf11
			vmaddw.xyzw		vf7,vf12,vf9w

#			vmul.xyz		vf16,vf8,vf8
			vmulax.w		ACC,vf0,vf16x
			vmadday.w		ACC,vf0,vf16y
			vmaddz.w		vf16,vf0,vf16z
			vsqrt	        Q,vf16w

			sqc2	        vf7,0x00(%0)
			vwaitq
			vmulq.x			vf16,vf6,Q
			sqc2	        vf16,0x00(%2)
			": : "r"(&rot), "r"(&S4), "r"(dvec), "r"(0.7f) );

			dvec->vw = dvec->vx;
			dvec->vx = dvec->vw * rot.vx * rot.vw;
			dvec->vy = dvec->vw * rot.vy;
			dvec->vz = dvec->vw * rot.vz * rot.vx;
			//PRINT_PFVEC(i,dvec);
			dvec->vw = temp;
			to_pos.vy = temp;
#endif
	    }else{
			_sceVu0RotMatrixZ( &comdl_pos->world, &comdl_pos->world, (i&1)?(PI*0.0625f):(-PI*0.0625f) );
		}

		DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], &to_pos );
		if(work->timer < 64){
			comdl_pos->color.vw = work->timer * 2;
			//if(i==0)printf("alpha %d\n",comdl_pos->color.vw);
		}
		comdl_pos++;
		dvec++;
	}
	TS_Scr_Mem( work->dvec, MEM_SCR_POS, sizeof(FVECTOR), work->n_comdl );
	
	if(--work->timer < 0){
		//printf("DIE\n");
		GV_DestroyActor( work );
	}
}

static void LevelCheck_Act(Work *work)
{
	int		i;
	DG_COMDL_POS 	*comdl_pos;
	FVECTOR			*dvec;
		
	comdl_pos = work->comdl->pos;
	dvec = work->dvec;

	for( i = 0; i < work->n_comdl; i++ ){
		FVECTOR to_pos;
		//PRINT_PFVEC(i,dvec);
		dvec->vy += GRAVITATION;
		DG_COPY_VEC(&to_pos,dvec);
		to_pos.vw = 0.0f;
		_sceVu0AddVector( &to_pos, (FVECTOR*)comdl_pos->world.m[3], &to_pos );
		//PRINT_PFVEC(i,dvec);
		if(dvec->vw > to_pos.vy){
			float len = VectorLength(dvec) * 0.7f;
			float rot_x,rot_y,temp = dvec->vw;
			FVECTOR force = {0.0f,len,0.0f,0.0f};
			FMATRIX world;
			
			rot_x = PI * 0.166f * frnd();
			rot_y = PI * frnd();
			_sceVu0RotMatrixX( &world, &DG_UnitMatrix, rot_x );
			_sceVu0RotMatrixY( &world, &world, rot_y );		
			_RotVector( dvec, &world, &force );

			dvec->vw = temp;
			to_pos.vy = temp;
		}else{
			_sceVu0RotMatrixZ( &comdl_pos->world, &comdl_pos->world, PI*0.0625f );
		}
		//床
		if( dvec->vy < 0.0f ){	/* ハザードチェック */
			unsigned int 		init_flr_flag;
			HZX_FLR				flr[2];
			int					flr_atrs[2];
			float				flr_height[2];

			init_flr_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( work->map_id ),
												 &to_pos,
												 HZX_CHK_F_FLOOR,
												 HZX_FLOOR_NO_PLAYER );
			//printf(" flr_Flag = %x\n",init_flr_flag);
			//床有り
			if( init_flr_flag & 1){
				
				HZX_GetLevelHazard( flr, flr_atrs );
				/* flr_height[2] 0:flr 1:cel*/
				HZX_GetLevelHeight( flr_height );

				dvec->vw = flr_height[0];
			}
		}
		DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], &to_pos );
		//PRINT_PFVEC(i,&to_pos);

		if(work->timer < 64){
			comdl_pos->color.vw = work->timer * 2;
			//if(i==0)printf("alpha %d\n",comdl_pos->color.vw);
		}
		comdl_pos++;
		dvec++;
	}
	
	if(--work->timer < 0){
		//printf("DIE\n");
		GV_DestroyActor( work );
	}
}

static void Act( Work *work )
{
	if((work->flags&3) == 3){
		LevelCheck_Act(work);
	}else{
		NoLevelCheck_Act(work);
	}
}

static void Die( Work *work )
{
	if(work->comdl){
		DG_DequeueComdlObjs( work->comdl );
		DG_FreeComdl( work->comdl );
	}
}


static int GetResources( Work *work, FVECTOR *bound, FVECTOR *force,
						 int num, int obj_code,
						 int color, float scale, int flags )
{
	int				i;
	DG_DEF			*def;
	DG_COMDL		*comdl;
	DG_COMDL_POS 	*comdl_pos;
	FVECTOR			add,*dvec,pow,pos;
	FMATRIX			world;
	int				mode = 0;
	float			inner;
	
	work->n_comdl = num;
	work->velocity = force->vw;	
	work->flags = flags;
	work->timer = LIFE;
	work->map_id = GM_CurrentStageMap;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( obj_code, 'k' ) );
	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, num, 0 );
	if( !comdl ) return -1;
	DG_QueueComdlObjs( comdl );

	work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );
	
	DG_COPY_VEC(&pow,force);
	pow.vw = 0.0f;

	_sceVu0Normalize( &pow, &pow );
	inner = _sceVu0InnerProduct(&pow,(FVECTOR*)DG_UnitMatrix.m[0]);
	inner = fabs(inner);
	if( inner >= 0.999f ){
		mode = 1;
	}

#ifdef BP_PS2
	TS_MakeMatrix2( &world, &pow, &(FVECTOR){0.0f,-1.0f,0.0f,0.0f}, &DG_ZeroVector );
#else
	{
		FVECTOR tmp = {0.0f,-1.0f,0.0f,0.0f} ;
		TS_MakeMatrix2( &world, &pow, &tmp, &DG_ZeroVector );
	}
#endif
	//PRINT_PFVEC(0,(FVECTOR*)world.m[0]);
	//PRINT_PFVEC(1,(FVECTOR*)world.m[1]);
	//PRINT_PFVEC(2,(FVECTOR*)world.m[2]);
	
	DG_COPY_VEC(&pos,bound);
	_sceVu0SubVector( &add, &bound[1], &bound[0] );

	comdl_pos = comdl->pos;
	dvec = work->dvec;

	for( i = 0; i < num; i++ ){
		FMATRIX		l_world;
		FVECTOR		fvtemp = {0.0f,0.0f,work->velocity*0.5f*(rnd() + 1.0f),0.0f};
		float		rot_x,rot_y,scl;
		
		//DVEC初期化
		rot_x = PI * 0.166f * frnd();
		rot_y = PI * 0.5f * frnd();
		if(mode){
			_sceVu0RotMatrixY( &l_world, &world, rot_y );
			_sceVu0RotMatrixX( &l_world, &l_world, rot_x );
		}else{
			_sceVu0RotMatrixX( &l_world, &world, rot_x );
			_sceVu0RotMatrixY( &l_world, &l_world, rot_y );
		}
		_RotVector( dvec, &l_world, &fvtemp );

		dvec->vw = -FLOAT_MAX;

		
		//コモデルMATRIX初期化
		fvtemp.vx = pos.vx + add.vx * rnd();
		fvtemp.vy = pos.vy + add.vy * rnd();
		fvtemp.vz = pos.vz + add.vz * rnd();
		fvtemp.vw = 1.0f;
#if 1
		rot_x = PI * frnd();
		rot_y = PI * frnd();
		scl = scale*(0.5f+rnd());
		_ScaleMatrix( &comdl_pos->world, &DG_UnitMatrix, scl );
		//PRINT_PFVEC(0,(FVECTOR*)comdl_pos->world.m[0]);
		//PRINT_PFVEC(1,(FVECTOR*)comdl_pos->world.m[1]);
		//PRINT_PFVEC(2,(FVECTOR*)comdl_pos->world.m[2]);
		//PRINT_PFVEC(3,(FVECTOR*)comdl_pos->world.m[3]);
		_sceVu0RotMatrixX( &comdl_pos->world, &comdl_pos->world, rot_x );
		_sceVu0RotMatrixY( &comdl_pos->world, &comdl_pos->world, rot_y );
#else
		DG_COPY_MAT( &comdl_pos->world, &DG_UnitMatrix );
#endif

		DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], &fvtemp );
		//PRINT_PFVEC(i,(FVECTOR*)comdl_pos->world.m[3]);
		
		if(flags&2){	/* 床ハザードチェック */
			unsigned int 		init_flr_flag;
			HZX_FLR				flr[2];
			int					flr_atrs[2];
			float				flr_height[2];

			init_flr_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( work->map_id ),
												 &fvtemp,
												 HZX_CHK_F_FLOOR,
												 HZX_FLOOR_NO_PLAYER );
			//printf(" flr_Flag = %x\n",init_flr_flag);
			//床有り
			if( init_flr_flag & 1){
				
				HZX_GetLevelHazard( flr, flr_atrs );
				/* flr_height[2] 0:flr 1:cel*/
				HZX_GetLevelHeight( flr_height );

				dvec->vw = flr_height[0];
				//printf("aaa\n");
			}
		}

#if 0
		{
			void *temp = NewDbugSprite( (FVECTOR*)comdl_pos->world.m[3], 64 );
			if(temp) GV_SetActorChild(work,temp);
		}
#endif
		//PRINT_PFVEC(i,dvec);
		// 整数型なので注意！
		comdl_pos->color.vx = (color>>24)&0xff;
		comdl_pos->color.vy = (color>>16)&0xff;
		comdl_pos->color.vz = (color>>8)&0xff;
		comdl_pos->color.vw = 128;

		comdl_pos++;
		dvec++;
	}
	return (0);
}
/*
FVECTOR		*bound:		生成平面もしくはポイント（１～２個）[0]MINBOUND [1]MAXBOUND
FVECTOR		*force:		方向（ＶＷは速度）
int			num:		数
int			objcode:	モデル
int			color:		色
float		scale:		スケール
int			flags:		下記

flags
			[0] 0:最初のみ 1:毎回			//
			[1] 0:床当たり無し 1:有り		//
			[2] 0:壁当たり無し 1:有り		//未対応
*/

void *NewDebris_Cm( FVECTOR *bound, FVECTOR *force,
					int num, int objcode,
					int color, float scale, int flags)
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) + sizeof(FVECTOR)*num );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, bound, force, num, objcode, color, scale, flags ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;
}

void *NewDebris_Cm_Demo( FVECTOR *bound0, FVECTOR *bound1, FVECTOR *force,
						 float pow, int num, int objcode,
						 int color, float scale, int flags)
{
	FVECTOR bound[2];
	FVECTOR vec;
	
	DG_COPY_VEC(&bound[0],bound0);
	DG_COPY_VEC(&bound[1],bound1);
	DG_COPY_VEC(&vec,force);
	vec.vw = pow;

	return NewDebris_Cm( bound, &vec, num, objcode, color, scale, flags);
}

void *NewDebris_Cm_Demo2( int name0, int name1,
						  float pow, int num, int objcode,
						  int color, float scale, int flags)
{
	FVECTOR 	bound[2];
	FVECTOR 	vec;
	FMATRIX		world;
	EFTCONTROL	*cntrl0;
	EFTCONTROL	*cntrl1;

	cntrl0 = DM_GetEftControl( name0 );
	DG_COPY_VEC( &bound[0], &cntrl0->mov );
	if( name1 != 1 ){
		cntrl1 = DM_GetEftControl( name1 );
		DG_COPY_VEC( &bound[1], &cntrl1->mov );
	}else{
		DG_COPY_VEC( &bound[1], &cntrl0->mov );
	}
	DM_EftControlMatrix( cntrl0, &world );
	DG_COPY_VEC( &vec, (FVECTOR*)world.m[2] );
	vec.vw = pow;

	return NewDebris_Cm( bound, &vec, num, objcode, color, scale, flags);
}

void *NewDebris_Cm_DemoBlood( int name0, int name1,
							  float pow, int num, int objcode,
							  int color, float scale, int flags)
{
	FVECTOR 	bound[2];
	FVECTOR 	vec;
	FMATRIX		world;
	EFTCONTROL	*cntrl0;
	EFTCONTROL	*cntrl1;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ){
		return NULL;
	}

	cntrl0 = DM_GetEftControl( name0 );
	DG_COPY_VEC( &bound[0], &cntrl0->mov );
	if( name1 != 1 ){
		cntrl1 = DM_GetEftControl( name1 );
		DG_COPY_VEC( &bound[1], &cntrl1->mov );
	}else{
		DG_COPY_VEC( &bound[1], &cntrl0->mov );
	}
	DM_EftControlMatrix( cntrl0, &world );
	DG_COPY_VEC( &vec, (FVECTOR*)world.m[2] );
	vec.vw = pow;

	return NewDebris_Cm( bound, &vec, num, objcode, color, scale, flags);
}


