//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    cigarette.c
    デモ用タバコ
	2000/08/01 T.Shibata

	$Id: cigarette.c,v 1.1.1.3 2002/11/19 11:48:29 Yoshizawa1 Exp $
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
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"

/*--------------------------------------------------------------------------------
途中からコメントを充実させてみることにした。

int			name:		メッセージ受けとる名前
FMATRIX		*world:		タバコ投げるまでのワールドのポインタ
FMATRIX		*sna_head:	スネーク頭
FMATRIX		*sna_rhand:	スネーク右手
float		force:		投げる強さ
FVECTOR		*dire:		投げる方向

void *NewCigarette_Demo( int name,
                         FMATRIX *world, FMATRIX *sna_head, FMATRIX *sna_rhand,
						 float force, FVECTOR *dire );

//message[0]
enum {
	CGRT_MSG_SLOW = 0,		//すべての移動の関する値の係数書き換え
	                        //	message[1]10000 で 1.0f
	CGRT_MSG_SMOKE,			//吸う
	                        //	message[1]吸う時間（タバコが明るいままのframe数）
	CGRT_MSG_EMIT,			//吐く
	                        //	message[1]吐く時間（煙を口からだすframe数）
	CGRT_MSG_THROW,			//捨てる
	CGRT_MSG_KILL,			//こーろーすー
	CGRT_MSG_LINK,			//リンク先切替え(うそでーす)
};

---------------------------------------------------------------------------------*/
#define CGRT_DEBUG 0

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,_fv->vx,_fv->vy,_fv->vz,_fv->vw)
#define		CHECK_MAIN_STEP(_s)	(_s&0xff)

#define		TEST_TEX_CODE	(6715088)

#define		MODEL_CODE 			(3354945)//(GV_StrCode("ciga"))
//#define		MSMK_TEX_CODE		(GV_StrCode("rcm_l_msk"))

#define		MSMK_TEX_CODE		(9998494)
#define		NSMK_TEX_CODE		(MSMK_TEX_CODE)//(GV_StrCode("powder02_alp"))

#define		FIRETAIL_TEX_CODE	(0)//(GV_StrCode("drop01_msk"))

#define		CSMK_TEX_CODE0	(1084957)//(GV_StrCode("smoke_lp1_alp"))		//標準
#define		CSMK_TEX_CODE1	(2133533)//(GV_StrCode("smoke_lp2_alp"))		//細い
#define		CSMK_TEX_CODE2	(3182109)//(GV_StrCode("smoke_lp3_alp"))		//ガス

//タバコ煙	ストリップポリ
#define		N_CGRT_SMOKE		(2)
#define		N_CSMK_JOINT		(32)		//今のところ64まで
#define		N_CSMK_ALLVERTS		(2*N_CSMK_JOINT)
#define		N_CSMK_PRIMS		(1)
#define		N_CSMK_VERTS		(64)
#define		CSMK_SIZE			(9.0f)//(2.5f)

#define		MEM_SCR_CGRT_POS	((void*)(SCRPAD_ADDR))
#define		MEM_SCR_CGRT_UV		((void*)(MEM_SCR_CGRT_POS  + sizeof(FVECTOR)*N_CSMK_ALLVERTS))
#define		MEM_SCR_CGRT_JPOS	((void*)(MEM_SCR_CGRT_UV   + sizeof(DG_PRIM2_UVRGB)*N_CSMK_ALLVERTS))
#define		MEM_SCR_CGRT_JVEC	((void*)(MEM_SCR_CGRT_JPOS + sizeof(FVECTOR)*N_CSMK_JOINT))
#define		MEM_SCR_CGRT_BOTTOM	((void*)(MEM_SCR_CGRT_JVEC + sizeof(FVECTOR)*N_CSMK_JOINT))

//火種尻尾		ポリ
#define		N_FRTL_PRIMS	(1)
#define		N_FRTL_VERTS	(64)
#define		FRTL_SIZE		(2.5f)//(2.5f)
#define		FRTL_SHIFT_X	(2.5f)
#define		FRTL_SHIFT_Y	(18.0f)
#define		FRTL_SHIFT_Z	(0.0f)
#define		SCREEN_Z		(51.0f)
#define		FRTL_V_ADD		(1.0f/32.0f)

//口煙			回転スプライト
#define		N_MSMK_PRIMS	(8)
#define		N_MSMK_VERTS	(16)

#define		N_MSMK_ALLVERTS	(N_MSMK_PRIMS*N_MSMK_VERTS)
#define		N_MSMK_N_TIME	(64)
#define		N_MSMK_N_INIT	(2)

#define		MSMK_INIT_SIZE	(0)
#define		MSMK_ADD_SIZE	(6.0f)
#define		MSMK_ADD_ANGLE	(PI/60.0f)
#define		MSMK_MAX_ALPHA	(8.0f)
#define		MSMK_INIT_ADD_ALPHA (4.0f/64.0f)
#define		MSMK_COLOR 		(0x80808000)	//0xrrggbb00

#if 0 //BP_PS2
#define		MEM_SCR_MSMK_POS	((void*)(SCRPAD_ADDR))
#define		MEM_SCR_MSMK_BPS	((void*)(MEM_SCR_MSMK_POS + sizeof(FVECTOR)*N_MSMK_ALLVERTS))
#define		MEM_SCR_MSMK_VEC	((void*)(MEM_SCR_MSMK_BPS + sizeof(FVECTOR)*N_MSMK_ALLVERTS))
#define		MEM_SCR_MSMK_DATA	((void*)(MEM_SCR_MSMK_VEC + sizeof(FVECTOR)*N_MSMK_ALLVERTS))
#define		MEM_SCR_MSMK_UV		((void*)(MEM_SCR_MSMK_DATA + sizeof(FVECTOR)*N_MSMK_ALLVERTS))
#define		MEM_SCR_MSMK_BOTTOM	((void*)(MEM_SCR_MSMK_UV + sizeof(DG_PRIM2_UVRGBWH)*N_MSMK_ALLVERTS))
#else
#define		MEM_SCR_MSMK_POS	((void*)(SCRPAD_ADDR))
#define		MEM_SCR_MSMK_BPS	((void*)((char *)MEM_SCR_MSMK_POS + sizeof(FVECTOR)*N_MSMK_ALLVERTS))
#define		MEM_SCR_MSMK_VEC	((void*)((char *)MEM_SCR_MSMK_BPS + sizeof(FVECTOR)*N_MSMK_ALLVERTS))
#define		MEM_SCR_MSMK_DATA	((void*)((char *)MEM_SCR_MSMK_VEC + sizeof(FVECTOR)*N_MSMK_ALLVERTS))
#define		MEM_SCR_MSMK_UV		((void*)((char *)MEM_SCR_MSMK_DATA + sizeof(FVECTOR)*N_MSMK_ALLVERTS))
#define		MEM_SCR_MSMK_BOTTOM	((void*)((char *)MEM_SCR_MSMK_UV + sizeof(DG_PRIM2_UVRGBWH)*N_MSMK_ALLVERTS))
#endif

//鼻煙			回転スプライト
#define		N_NSMK_PRIMS	(5)
#define		N_NSMK_VERTS	(12)

#define		N_NSMK_ALLVERTS	(N_NSMK_PRIMS*N_NSMK_VERTS)
#define		N_NSMK_N_TIME	(60)
#define		N_NSMK_N_INIT	(1)

#define		NSMK_INIT_SIZE	(0)
#define		NSMK_ADD_SIZE	(4.0f)
#define		NSMK_ADD_ANGLE	(M_PI/60.0f)
#define		NSMK_MAX_ALPHA	(8.0f)
#define		NSMK_INIT_ADD_ALPHA (3.0f/60.0f)

#define		NSMK_COLOR 		(0x80808000)	//0xrrggbb00

//風関係
#define	WIND_INTNS_CHECK	(100)	//

#define	FLAG_CGRT_OBJS_THROW	(0x0001)
#define	FLAG_CGRT_SMK_VSBL 	(0x0002)

typedef struct _cigarette_Work
{
	GV_ACT_EX			actor;

	FMATRIX				fire[2] ;		//火種ライト
#if CGRT_DEBUG
	FVECTOR				crgt_shiff;		//デバッグ用
#endif
	FMATRIX				l_world ;		//投げた後使用
	FVECTOR				rot;			//投げた後使用
	FVECTOR				dire;			//投げる方向
	FVECTOR				center;			//タバコの端
	FVECTOR				pre_center;		//前の煙草の端
	FVECTOR				cfcnt;			//係数
	FVECTOR				wind;			//風

	FVECTOR 			csmk_joint_pos2[N_CSMK_JOINT];
	FVECTOR 			csmk_joint_dvec2[N_CSMK_JOINT];
	float				csmk_joint_alpha2[N_CSMK_JOINT];
	float				csmk_joint_size2[N_CSMK_JOINT];
	
	FVECTOR 			csmk_joint_pos[N_CGRT_SMOKE][N_CSMK_JOINT];
	FVECTOR 			csmk_joint_dvec[N_CGRT_SMOKE][N_CSMK_JOINT];
	float				csmk_joint_alpha[N_CGRT_SMOKE][N_CSMK_JOINT];
	float				csmk_joint_size[N_CGRT_SMOKE][N_CSMK_JOINT];
	
	
	FVECTOR				msmk_dvec[N_MSMK_ALLVERTS];		//口煙速度
	FVECTOR				msmk_data[N_MSMK_ALLVERTS];		//vx:アルファ補正 vy:アルファ補正の増減値 vz:サイズ vw:角度
	FVECTOR				nsmk_dvec[N_NSMK_ALLVERTS];		//口煙速度
	FVECTOR				nsmk_data[N_NSMK_ALLVERTS];		//vx:アルファ補正 vy:アルファ補正の増減値 vz:サイズ vw:角度

	int					m_timer;		//メインタイマー
	int					s_timer;		//サブタイマー
	int					l_timer;		//ローカルタイマー
	int					flags;			//フラグ
	
	int 				step;			//ステップ
	int					name;			//氏名
	float				force;			//投げる強さ
	FMATRIX				*world;			//タバコのワールド
	
	FMATRIX				*sna_head;		//スネーク頭のワールド
	FMATRIX				*sna_rhand;		//スネーク右手のワールド
	DG_PRIM2			*nsmk_prim;		//鼻煙
	DG_PRIM2			*msmk_prim;		//口煙
	
	DG_PRIM2			*ash_prim;		//灰
	DG_PRIM2			*frtl_prim;		//火種尻尾
	DG_OBJS				*cgrt_objs;		//タバコモデル
	float				color;			//輝度？みたいなー
	
	float				smoke_amnt;		//タバコ煙の量
	float				emit_amnt;		//吐く煙の量(アルファ補正)
	float				add_alpha;		//msmkのアルファ補正の増減値
	float				diff_v;
	
	DG_PRIM2			*csmk_prim[N_CGRT_SMOKE];		//タバコ煙	
	float				csmk_rot_x[N_CGRT_SMOKE];
	float				csmk_rot_y[N_CGRT_SMOKE];
	float				csmk_size[N_CGRT_SMOKE];
	
	DG_PRIM2			*csmk_prim2;		//タバコ煙2	
	float				csmk_rot_x2;
	float				csmk_rot_y2;
	float				csmk_size2;

	float				min_v,len_v;
	float				now_v,dvec_v;
	
} Work ;

//message[0]
enum {
	CGRT_MSG_SLOW = 0,		//すべての移動の関する値の係数書き換え		[1]10000 で 1.0f
	CGRT_MSG_SMOKE,			//吸う		[1]吸う時間（タバコが明るいままのframe数）
	CGRT_MSG_EMIT,			//吐く		[1]吐く時間（煙を口からだすframe数）
	CGRT_MSG_THROW,			//捨てる
	CGRT_MSG_KILL,			//こーろーすー
	CGRT_MSG_LINK,			//リンク先変更
};

enum {
	STEP_NORMAL = 0,		//通常処理
	STEP_SMOKING,			//吸う処理
	STEP_EMITTING,			//吐く処理
};

//////////////////////////////////////////////////////////////////
//		extern

extern int OK_GetLocalWind_SH( FVECTOR *pos, FVECTOR *output );
extern FVECTOR G_wind;
extern int G_wind_intense;
extern int G_wind_intense_max;

/////////////////////////////////////////////////////////////////
//		g_variable
static FVECTOR	Frtl_Shiff = { FRTL_SHIFT_X, FRTL_SHIFT_Y, FRTL_SHIFT_Z, 0.0f };
#if 0
static FVECTOR	Frtl_InitVerts[4] = {
	{  FRTL_SIZE,  FRTL_SIZE, 0.0f, 0.0f },
	{ -FRTL_SIZE,  FRTL_SIZE, 0.0f, 0.0f },
	{  FRTL_SIZE, -FRTL_SIZE, 0.0f, 0.0f },
	{ -FRTL_SIZE, -FRTL_SIZE, 0.0f, 0.0f },
};
#endif
#if 0
static FVECTOR	Csmk_InitVerts_Ex[2] = {
	{  CSMK_SIZE, 0.0f, 0.0f, 0.0f },
	{ -CSMK_SIZE, 0.0f, 0.0f, 0.0f },
};
#endif
static FVECTOR jvec_z = { 8.0f, 1.0f, 0.0f, 0.0f };
static FVECTOR jvec_out = { 0.0f, 0.0f, 1.0f, 0.0f };
static FVECTOR Wind_Vec = { -1.0f, 0.0f, 1.0f, 0.0f };
static FVECTOR Smoke_Rgst = { 0.985f,0.985f,0.985f,0.0f };
static SVECTOR Wind_Rot = { 0, 0, 0, 0 };

#if CGRT_DEBUG

#define CGRT_SHIFT_X	(-10.0f)
#define CGRT_SHIFT_Y	(-30.0f)
#define CGRT_SHIFT_Z	(124.0f)

#define CGRT_DIFF_X		(0.0f)
#define CGRT_DIFF_Y		(0.0f)
#define CGRT_DIFF_Z		(1.0f)

#endif

#define EMIT_FRAME		(180.0f)
#define EMIT_ADD_ALPHA	(3.0f/EMIT_FRAME)

#if 0
//鼻	口と統合してもいいかも
static void NoseSmoke_Act( Work *work )
{
	//鼻(vx -6.0f,6.0f vy -5.0f vz 110.0f)
	static FVECTOR		OffSet = { 0.0f, -5.0f, 110.0f, 0.0f };
	static FVECTOR		Diff   = { 0.0f, -2.0f, 1.0f, 0.0f };

	int					i, clock, init_index;
	DG_PRIM2			*prim = work->nsmk_prim;
	FVECTOR				*pos, *before_pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh0;
	FVECTOR				*dvec, *data,fvtemp;
	FVECTOR				rgst,center,diff,wind;
	FMATRIX				fmtemp;
	SVECTOR				rot = {0,0,0,0};
	
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	pos = prim->pos[clock];
	before_pos = prim->pos[1-clock];
	uvrgbwh0 = prim->uvrgb[clock];
	dvec = work->nsmk_dvec;
	data = work->nsmk_data;

	//風加工
	
	OK_GetLocalWind_SH( &work->center, &wind );
	_sceVu0ScaleVector( &wind, &wind, 0.001f * work->cfcnt.vx );
	//抵抗
	_sceVu0MulVector( &rgst, &Smoke_Rgst, &work->cfcnt );

	for( i = 0; i < N_NSMK_ALLVERTS; i++ ){
		//移動
		_sceVu0ScaleVector( &fvtemp, dvec, work->cfcnt.vx );
		_sceVu0AddVector( pos, before_pos, &fvtemp );

		//dvec->vy += 0.1f;
		//風
		_sceVu0AddVector( dvec, &wind, dvec );
		//抵抗
		_sceVu0MulVector( dvec, &rgst, dvec );

		//アルファ変更
		uvrgbwh0->a = (short)( work->emit_amnt * data->vx * NSMK_MAX_ALPHA );
//		uvrgbwh0->a = (short)( data->vx * NSMK_MAX_ALPHA );
		data->vx += data->vy;
		if( data->vx > 1.0f ){
			data->vx = 1.0f;
			data->vy = -NSMK_INIT_ADD_ALPHA/2.0f;
		}else if( data->vx < 0.0f ){
			data->vx = 0.0f;
			data->vy = 0.0f;
		}

		//サイズと回転変更
		uvrgbwh0->w = (short)data->vz * vu0_Cos(data->vw);
		uvrgbwh0->h = (short)data->vz * vu0_Sin(data->vw);
		data->vz += (work->cfcnt.vx * NSMK_ADD_SIZE);
		data->vw += ((i&1)?(work->cfcnt.vx * NSMK_ADD_ANGLE):-(work->cfcnt.vx * NSMK_ADD_ANGLE)) * rnd();
		if( data->vw < -M_PI ) data->vw += 2.0f * M_PI;
		if( data->vw >  M_PI ) data->vw -= 2.0f * M_PI;
		
		pos++; before_pos++;
		uvrgbwh0++;
		dvec++; data++;
	}
	
	//初期
	pos = prim->pos[clock];
	uvrgbwh0 = prim->uvrgb[clock];
	dvec = work->nsmk_dvec;
	data = work->nsmk_data;
	
	init_index = (work->l_timer%N_NSMK_N_TIME)*N_NSMK_N_INIT;
//	printf("init_index = %d\n",init_index);
	
	pos += init_index;
	uvrgbwh0 += init_index;
	dvec += init_index;
	data += init_index;

	//出現マトリックス生成
	DG_SetPos(work->sna_head);
	DG_PutVector( &OffSet, &center, 1 );
	//_sceVu0AddVector( &center, &center, (FVECTOR*)work->sna_head->m[3] );
	DG_RotVector( &Diff, &diff, 1 );
	TS_MakeMatrix( &fmtemp, &diff, &center );

	for( i = 0; i < N_NSMK_N_INIT; i++ ){
		static FVECTOR force = { 0.0f, 0.0f, 5.0f, 0.0f };
		//pos  初期化
		DG_COPY_VEC( pos, &center );
		//dvec 初期化
		rot.vx = (irnd() % 512) - 256;
		rot.vy = (irnd() % 512) - 256;
		DG_SetPos(&fmtemp);
		DG_RotatePos( &rot );
		DG_RotVector( &force, dvec, 1 );

		//data 初期化 vx:アルファ補正 vy:アルファ補正の増減値 vz:サイズ vw:角度
		//PRINT_PFVEC(i,data);
		data->vx = 0.0f;
		data->vy = NSMK_INIT_ADD_ALPHA;
		data->vz = 0.0f;
		data->vw = M_PI * frnd();
		
		//uvrgbwh0 初期化
		uvrgbwh0->a = 0;
		uvrgbwh0->w = 0;
		uvrgbwh0->h = 0;
		
		pos++; before_pos++;
		uvrgbwh0++;
		dvec++; data++;
	}

}
#endif
static void MouthSmoke_Act( Work *work )
{
	//口
	static FVECTOR		OffSet = { 0.0f, -29.0f, 103.0f, 0.0f };
	static FVECTOR		Diff   = { 0.0f,   0.0f, 1.0f, 0.0f };

	int					i, clock, init_index;
	DG_PRIM2			*prim = work->msmk_prim;
	FVECTOR				*pos, *before_pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh0;
	FVECTOR				*dvec, *data;//, fvtemp;
	FVECTOR				rgst,center,diff,wind;
	FMATRIX				fmtemp;
	SVECTOR				rot = {0,0,0,0};
	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;
	
	TS_Mem_Scr( MEM_SCR_MSMK_BPS, prim->pos[1-clock],sizeof(FVECTOR),N_MSMK_ALLVERTS);
	TS_Mem_Scr( MEM_SCR_MSMK_VEC, work->msmk_dvec,sizeof(FVECTOR),N_MSMK_ALLVERTS);
	TS_Mem_Scr( MEM_SCR_MSMK_DATA, work->msmk_data,sizeof(FVECTOR),N_MSMK_ALLVERTS);
	TS_Mem_Scr( MEM_SCR_MSMK_UV, prim->uvrgb[clock],sizeof(DG_PRIM2_UVRGBWH),N_MSMK_ALLVERTS);
	
	pos = MEM_SCR_MSMK_POS;
	before_pos = MEM_SCR_MSMK_BPS;
	uvrgbwh0 = MEM_SCR_MSMK_UV;
	dvec = MEM_SCR_MSMK_VEC;
	data = MEM_SCR_MSMK_DATA;

	//風加工
#if 0
	OK_GetLocalWind_SH( &work->center, &wind );
	_sceVu0ScaleVector( &wind, &wind, 0.001f );
#else
	_sceVu0ScaleVector( &wind, &G_wind, 0.001f );
#endif
	wind.vw = 0.0f;
	//抵抗
	_sceVu0MulVector( &rgst, &Smoke_Rgst, &work->cfcnt );

	for( i = 0; i < N_MSMK_ALLVERTS; i++ ){
		//移動
//		_sceVu0ScaleVector( &fvtemp, dvec, work->cfcnt.vx );
//		_sceVu0AddVector( pos, before_pos, &fvtemp );
		_sceVu0AddVector( pos, before_pos, dvec );
		//dvec->vy += 0.1f;
		//風
		_sceVu0AddVector( dvec, &wind, dvec );
		//抵抗
		_sceVu0MulVector( dvec, &rgst, dvec );

//		PRINT_PFVEC(i,dvec);
		//アルファ変更
		uvrgbwh0->a = DG_FTOI( work->emit_amnt * data->vx * MSMK_MAX_ALPHA );
//		uvrgbwh0->a = (short)( data->vx * MSMK_MAX_ALPHA );
//		uvrgbwh0->a = (short)( MSMK_MAX_ALPHA );
		data->vx += data->vy;
		if( data->vx > 1.0f ){
			data->vx = 1.0f;
			data->vy = -MSMK_INIT_ADD_ALPHA/3.0f;
		}else if( data->vx < 0.0f ){
			data->vx = 0.0f;
			data->vy = 0.0f;
		}

		//サイズと回転変更
		uvrgbwh0->w = DG_FTOI(data->vz * vu0_Cos(data->vw));
		uvrgbwh0->h = DG_FTOI(data->vz * vu0_Sin(data->vw));
		
		data->vz += MSMK_ADD_SIZE;
//		data->vz += (work->cfcnt.vx * MSMK_ADD_SIZE);
		data->vw += ((i&1)?(MSMK_ADD_ANGLE):-(MSMK_ADD_ANGLE)) * rnd();
		if( data->vw < -PI ) data->vw += 2.0f * PI;
		else if( data->vw > PI ) data->vw -= 2.0f * PI;

		pos++; before_pos++;
		uvrgbwh0++;
		dvec++; data++;
	}

	//初期
	init_index = (work->l_timer%N_MSMK_N_TIME)*N_MSMK_N_INIT;
//	printf("init_index = %d\n",init_index);

	pos = MEM_SCR_MSMK_POS;
	uvrgbwh0 = MEM_SCR_MSMK_UV;
	dvec = MEM_SCR_MSMK_VEC;
	data = MEM_SCR_MSMK_DATA;
	
	pos += init_index;
	uvrgbwh0 += init_index;
	dvec += init_index;
	data += init_index;

	//出現マトリックス生成
	DG_SetPos(work->sna_head);
	DG_PutVector( &OffSet, &center, 1 );
	DG_RotVector( &Diff, &diff, 1 );
	TS_MakeMatrix( &fmtemp, &diff, &center );

	for( i = 0; i < N_MSMK_N_INIT; i++ ){
		static FVECTOR force = { 0.0f, 0.0f, 20.0f, 0.0f };
		//pos  初期化
		DG_COPY_VEC( pos, &center );
		//dvec 初期化
		rot.vx = (irnd() % 512) - 256;
		rot.vy = (irnd() % 512) - 256;
		//rot.vx = (irnd() % 256) - 128;
		//rot.vy = (irnd() % 256) - 128;
		DG_SetPos(&fmtemp);
		DG_RotatePos( &rot );
		DG_RotVector( &force, dvec, 1 );
		//PRINT_PFVEC(i,dvec);

		//data 初期化 vx:アルファ補正 vy:アルファ補正の増減値 vz:サイズ vw:角度
		//PRINT_PFVEC(i,data);
		data->vx = 0.0f;
		data->vy = MSMK_INIT_ADD_ALPHA;
		data->vz = 0.0f;
		data->vw = PI * frnd();
		
		//uvrgbwh0 初期化
		uvrgbwh0->a = 0;
		uvrgbwh0->w = 0;
		uvrgbwh0->h = 0;
		
		pos++;
		uvrgbwh0++;
		dvec++; data++;
	}

	TS_Scr_Mem( prim->pos[clock], MEM_SCR_MSMK_POS, sizeof(FVECTOR), N_MSMK_ALLVERTS);
	TS_Scr_Mem( prim->uvrgb[clock], MEM_SCR_MSMK_UV, sizeof(DG_PRIM2_UVRGBWH), N_MSMK_ALLVERTS);
	TS_Scr_Mem( work->msmk_dvec, MEM_SCR_MSMK_VEC, sizeof(FVECTOR), N_MSMK_ALLVERTS);
	TS_Scr_Mem( work->msmk_data, MEM_SCR_MSMK_DATA, sizeof(FVECTOR), N_MSMK_ALLVERTS);
//MEM_SCR_MSMK_POS
//MEM_SCR_MSMK_BPS
//MEM_SCR_MSMK_VEC
//MEM_SCR_MSMK_DATA
//MEM_SCR_MSMK_UV
	
}

#if 0
static void FireTail_Act( Work *work )
{
	int			i,clock,check = 4;
	DG_PRIM2	*prim = work->frtl_prim;
	FVECTOR		*pos,*before_pos,fvtemp,init_pos[4];
	DG_PRIM2_UVRGB	*uvrgb0, *uvrgb1;
	float		screen_near_x,screen_near_y;

	screen_near_x = SCREEN_Z / (ASPECT_X()                                           ) / DG_Chanls[0].screen;
	screen_near_y = SCREEN_Z / (ASPECT_Y() * DG_Chanls[0].width / DG_Chanls[0].height) / DG_Chanls[0].screen;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	
	pos = prim->pos[clock];
	before_pos = prim->pos[1-clock];
	uvrgb0 = prim->uvrgb[clock];
	uvrgb1 = prim->uvrgb[1-clock];
	memcpy( &pos[4], &before_pos[0], sizeof(FVECTOR) * (N_FRTL_VERTS-4) );
	memcpy( &uvrgb0[4], &uvrgb1[0], sizeof(DG_PRIM2_UVRGB) * (N_FRTL_VERTS-4) );

	DG_SetPos( &work->cgrt_objs->world ) ;
	DG_PutVector( &Frtl_Shiff, &fvtemp, 1 ) ;	//先っ歩位置
//	printf("center pos vx%f:vy %f:vz %f:vw %f\n",fvtemp.vx,fvtemp.vy,fvtemp.vz,fvtemp.vw);

	DG_COPY_VEC( &work->center, &fvtemp );
	
	DG_SetPos( &DG_Chanls[0].eye );
	DG_RotVector( Frtl_InitVerts, init_pos, 4 );
	for( i = 0; i < 4; i++ ) _sceVu0AddVector( &init_pos[i], &init_pos[i], &fvtemp );

	DG_SetPos( &DG_Chanls[0].eye_pers );
	DG_PutVector( init_pos, init_pos, 4 );

	for( i = 0; i < 4; i++ ){
		if(init_pos[i].vz > init_pos[i].vw * 1.4f) check--;
		
		init_pos[i].vw = fpu_Abs(init_pos[i].vw);
		init_pos[i].vz = SCREEN_Z;
		init_pos[i].vx = screen_near_x * init_pos[i].vx / init_pos[i].vw;
		init_pos[i].vy = screen_near_y * init_pos[i].vy / init_pos[i].vw;
#if 0
uvrgb0->r = 70 + (short)fpu_Abs(work->color.vx);
uvrgb0->g = 35 + (short)fpu_Abs(work->color.vy);
uvrgb0->b = 35 + (short)fpu_Abs(work->color.vz);
#else
uvrgb0->r = (short)( 180.0f * fpu_Abs(work->color) / 150.0f );
uvrgb0->g = (short)( 128.0f * fpu_Abs(work->color) / 150.0f );
uvrgb0->b = (short)(  90.0f * fpu_Abs(work->color) / 150.0f );
#endif
		uvrgb0++; uvrgb1++;
	}
	
	if( !check ) memset( init_pos, 0, sizeof(FMATRIX) );

	DG_COPY_MAT( pos, init_pos );

	//DG_SetTmpLight2( &fvtemp, 350.0f, 700.0f, 0x802040ff, LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY );
	//DG_SetTmpLight2( &fvtemp, 250.0f, 1000.0f, 0x80a20b0, LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY );
}
#endif

static FVECTOR _InitFvector[2] = {
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
};
//タバコの煙 広がらない奴(筋)
static void CgrtSmoke_Act( Work *work, int num )
{
	int			i,clock;
	DG_PRIM2	*prim = work->csmk_prim[num];
	FVECTOR		*now_jpos, *pre_jpos;
	FVECTOR		*now_jvec, *pre_jvec;
	FVECTOR		*pos, wind, dpos, *eye_z = (FVECTOR*)DG_Chanls[0].eye.m[2];
	FMATRIX				fmtemp;
	DG_PRIM2_UVRGB		*uvrgb0;
	SVECTOR				rot;
	float				*size, *alpha;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	
	//MEM_SCR_CGRT_POS
	//MEM_SCR_CGRT_UV	
	//MEM_SCR_CGRT_JPOS
	//MEM_SCR_CGRT_JVEC
	//MEM_SCR_CGRT_BOTTOM
	
	pos      = prim->pos[clock];
	uvrgb0   = prim->uvrgb[clock];
	now_jpos = work->csmk_joint_pos[num];
	pre_jpos = now_jpos+1;
	now_jvec = work->csmk_joint_dvec[num];
	pre_jvec = now_jvec+1;
	size = work->csmk_joint_size[num];
	alpha = work->csmk_joint_alpha[num];
	
	//風加工
	//OK_GetLocalWind_SH( &work->center, &wind );
	_sceVu0ScaleVector( &wind, &work->wind, 0.02f );
	wind.vw = 0.0f;
#if 0
	{
		FVECTOR *pwind = &G_wind;
		PRINT_PFVEC(255,pwind);
	}
#endif
#define SSS (0.95f)
#define MMM (1.0f/SSS)
#define KKK (SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS*SSS)
	
	if(work->flags & FLAG_CGRT_OBJS_THROW){
		DG_COPY_VEC( &dpos, &DG_ZeroVector );
	}else{
		_sceVu0SubVector( &dpos, &work->center, &work->pre_center );
		_sceVu0ScaleVector( &dpos, &dpos, KKK );
	}
	
#if 0
	{
		FVECTOR *pdpos = &dpos;
		PRINT_PFVEC(1,pdpos);
	}
#endif

	for( i = 0; i < N_CSMK_JOINT-1; i++ ){
		_sceVu0AddVector( now_jpos, pre_jpos, pre_jvec );
		_sceVu0AddVector( now_jpos, now_jpos, &dpos );
		
		_sceVu0AddVector( now_jvec, &wind, pre_jvec );
		_sceVu0MulVector( now_jvec, &Smoke_Rgst, now_jvec );
		
		_sceVu0ScaleVector( &dpos, &dpos, MMM );
	
		//size[0] = size[1] + 3.0f;// + fpu_Abs(work->csmk_size2)/2.0f ;//2.0f;
		size[0] = size[1];//2.0f;
		
		//now_jvec->vy += 0.2f;

		//進んだ方向に回転 及び カメラ方向に回転 マトリクス生成
		TS_MakeMatrix2( &fmtemp, eye_z, now_jvec, now_jpos );
		_InitFvector[0].vx = *size;
		_InitFvector[1].vx = -*size;
		DG_SetPos( &fmtemp );
		DG_PutVector( _InitFvector, pos, 2 );

		uvrgb0[0].a = uvrgb0[1].a = work->smoke_amnt * (float)(i);
//		uvrgb0[0].a = uvrgb0[1].a = 0x80;
		//printf("a[0] %d\n",uvrgb0[0].a);
		//PRINT_PFVEC(i,now_jpos);

		now_jpos++; now_jvec++;
		pre_jpos++; pre_jvec++;
		size++; alpha++;
		
		pos+=2;
		uvrgb0+=2;
	}

	uvrgb0[0].a = uvrgb0[1].a = 0;

	DG_COPY_VEC( now_jpos, &work->center );

	//jdvec 初期化
		
	TS_MakeMatrix2( &fmtemp, &jvec_z, &jvec_out, now_jpos );
	rot.vx = 0;//-(short)fpu_Abs( work->csmk_rot_x[num] );
	rot.vy = (short)fpu_Abs( work->csmk_rot_y[num] );
	rot.vz = 0;
	now_jvec->vx = 0.0f;
	now_jvec->vy = 0.0f;
	now_jvec->vz = 15.0f;
	now_jvec->vw = 0.0f;
	
	DG_SetPos( &fmtemp );
	DG_RotatePos( &rot );
	DG_RotVector( now_jvec, now_jvec, 1 );
	
//	_sceVu0ScaleVector( &wind, &work->wind, 0.01f );
//	_sceVu0AddVector( now_jvec, &wind, now_jvec );
	
	//	PRINT_PFVEC( 666, next_jvec );
	
	*size = CSMK_SIZE - fpu_Abs(work->csmk_size[num]);
	
	//進んだ方向に回転 及び カメラ方向に回転 マトリクス生成
	TS_MakeMatrix2( &fmtemp, eye_z, now_jvec, now_jpos );
	//回転 と タバコの端に移動
	_InitFvector[0].vx = *size;
	_InitFvector[1].vx = -*size;
	DG_SetPos( &fmtemp );
	DG_PutVector( _InitFvector, pos, 2 );
	
#define MAX_YURAGI (192.0f)
	
	work->csmk_rot_x[num] += 10.0f * rnd();
	if( work->csmk_rot_x[num] > MAX_YURAGI ) work->csmk_rot_x[num] = -MAX_YURAGI;
	work->csmk_rot_y[num] += 10.0f * rnd();
	if( work->csmk_rot_y[num] > MAX_YURAGI ) work->csmk_rot_y[num] = -MAX_YURAGI;

	work->csmk_size[num] += 0.1f * rnd();
	if( work->csmk_size[num] > 6.0f ) work->csmk_size[num] = -6.0f;

}

//タバコの煙 広がる奴
static void CgrtSmoke2_Act( Work *work, int num )
{
	int			i,clock;
	DG_PRIM2	*prim = work->csmk_prim2;
	FVECTOR		*now_jpos = work->csmk_joint_pos2;
	FVECTOR		*pre_jpos = now_jpos+1;
	FVECTOR		*now_jvec = work->csmk_joint_dvec2;
	FVECTOR		*pre_jvec = now_jvec+1;
	FVECTOR		wind,dpos,*eye_z = (FVECTOR*)DG_Chanls[0].eye.m[2];
	FVECTOR		*pos;
	FMATRIX				fmtemp;
	DG_PRIM2_UVRGB		*uvrgb0, *uvrgb1;
	SVECTOR				rot;
	float				*size = work->csmk_joint_size2;
	float				*alpha = work->csmk_joint_alpha2;
	float				v;
	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;
	
	pos = prim->pos[clock];
	uvrgb0 = prim->uvrgb[clock];
	uvrgb1 = prim->uvrgb[1-clock];

	//風加工
//	OK_GetLocalWind_SH( &work->center, &wind );
	_sceVu0ScaleVector( &wind, &work->wind, 0.02f );
	wind.vw = 0.0f;
#if 0
	{
		FVECTOR *pwind = &work->wind;
		PRINT_PFVEC(255,pwind);
	}
#endif
	if(work->flags & FLAG_CGRT_OBJS_THROW){
		DG_COPY_VEC( &dpos, &DG_ZeroVector );
	}else{
		_sceVu0SubVector( &dpos, &work->center, &work->pre_center );
		_sceVu0ScaleVector( &dpos, &dpos, KKK );
	}
	
	for( i = 0; i < N_CSMK_JOINT-1; i++ ){
		_sceVu0AddVector( now_jpos, pre_jpos, pre_jvec );
		_sceVu0AddVector( now_jpos, now_jpos, &dpos );
		
		_sceVu0AddVector( now_jvec, &wind, pre_jvec );
		//抵抗
		_sceVu0MulVector( now_jvec, &Smoke_Rgst, now_jvec );
		
		_sceVu0ScaleVector( &dpos, &dpos, MMM );
		
		//size[0] = size[1] + 3.0f;// + fpu_Abs(work->csmk_size2)/2.0f ;//2.0f;
		size[0] = size[1] + 1.0f + fpu_Abs(work->csmk_size2)/2.0f ;//2.0f;
		
		now_jvec->vy += 0.2f;

		//進んだ方向に回転 及び カメラ方向に回転 マトリクス生成
		TS_MakeMatrix2( &fmtemp, eye_z, now_jvec, now_jpos );
		_InitFvector[0].vx = *size;
		_InitFvector[1].vx = -*size;
		DG_SetPos( &fmtemp );
		DG_PutVector( _InitFvector, pos, 2 );

		uvrgb0[0].a = uvrgb0[1].a = work->smoke_amnt * (float)(i);
		//uvrgb0[0].a = uvrgb0[1].a = 0x80;
		//printf("a[0] %d\n",uvrgb0[0].a);
		//PRINT_PFVEC(i,now_jpos);
		//テクスチャー流し
		v = fpu_Abs(work->now_v) + ((work->now_v < 0.0f)?-work->diff_v*(float)i:work->diff_v*(float)i);
		if( v < 0.0f ){
			v += 2.0f * work->len_v;
		}
		if( v > work->len_v ){
			v = 2.0f * work->len_v - v;
		}
	
		v += work->min_v;
		
		uvrgb0[0].v = uvrgb0[1].v = FTOI12(v);
		now_jpos++; now_jvec++;
		pre_jpos++; pre_jvec++;
		size++; alpha++;
		
		pos+=2;
		uvrgb0+=2;
		uvrgb1+=2;

//		printf("%d\n",i);
	}

	uvrgb0[0].a = uvrgb0[1].a = 0;

	DG_COPY_VEC( now_jpos, &work->center );

	//jdvec 初期化
	TS_MakeMatrix2( &fmtemp, &jvec_z, &jvec_out, now_jpos );
	rot.vx = 0;//-(short)fpu_Abs( work->csmk_rot_x2 ) ;
	rot.vy = (short)fpu_Abs( work->csmk_rot_y2 ) ;
	rot.vz = 0;
	now_jvec->vx = 0.0f;
	now_jvec->vy = 0.0f;
	now_jvec->vz = 15.0f;
	now_jvec->vw = 0.0f;
	
	DG_SetPos( &fmtemp );
	DG_RotatePos( &rot );
	DG_RotVector( now_jvec, now_jvec, 1 );

//	_sceVu0ScaleVector( &wind, &work->wind, 0.1f );
//	_sceVu0AddVector( now_jvec, &wind, now_jvec );
	
	//	PRINT_PFVEC( 666, next_jvec );
	
	*size = 0.0f;
	
	//進んだ方向に回転 及び カメラ方向に回転 マトリクス生成
	TS_MakeMatrix2( &fmtemp, eye_z, now_jvec, now_jpos );
	//回転 と タバコの端に移動
	_InitFvector[0].vx = *size;
	_InitFvector[1].vx = -*size;
	DG_SetPos( &fmtemp );
	DG_PutVector( _InitFvector, pos, 2 );

	//printf("%d\n",++i);

#define MAX_YURAGI2 (320.0f)
	//ゆらめき
	work->csmk_rot_x2 += 10.0f * rnd();
	if( work->csmk_rot_x2 > MAX_YURAGI2 ) work->csmk_rot_x2 = -MAX_YURAGI2;
	work->csmk_rot_y2 += 10.0f * rnd();
	if( work->csmk_rot_y2 > MAX_YURAGI2 ) work->csmk_rot_y2 = -MAX_YURAGI2;
	//サイズ
	work->csmk_size2 += 0.1f * rnd();
	if( work->csmk_size2 > 6.0f ) work->csmk_size2 = -6.0f;
	//テクスチャ流し
	work->now_v += work->dvec_v;
	if( work->now_v > work->len_v ) work->now_v = -work->len_v;

}
#if CGRT_DEBUG
static void Cgrt_Norm_Act( Work *work )
{
	DG_OBJS		*objs = work->cgrt_objs;
#if 1
	FVECTOR		pos,dire;
	FMATRIX		temp;
	SVECTOR		rot = { 1024, 0, 0, 0 };
	pos.vx = CGRT_SHIFT_X;
	pos.vy = CGRT_SHIFT_Y;
	pos.vz = CGRT_SHIFT_Z;
	pos.vw = 0.0f;
	
	dire.vx = CGRT_DIFF_X;
	dire.vy = CGRT_DIFF_Y;
	dire.vz = CGRT_DIFF_Z;
	dire.vw = 0.0f;

	DG_SetPos(work->sna_head);
	
	DG_RotVector( &pos, &pos, 1 );
	DG_RotVector( &dire, &dire, 1 );
	_sceVu0AddVector( &pos, &pos, (FVECTOR*)work->sna_head->m[3] );
	TS_MakeMatrix2( &temp, &dire, (FVECTOR*)work->sna_head->m[0],&pos );
	

	DG_SetPos(&temp);
	DG_RotatePos( &rot );
	DG_GetPos( &objs->world );
#else
	FVECTOR		z = { 0.0f, 1.0f, 0.0f, 0.0f };
	FVECTOR		outer = { 0.0f, 0.0f, 1.0f, 0.0f };
	FVECTOR		center = { 0.0f, 68000.0f, 16000.0f, 0.0f };
	
	TS_MakeMatrix2( &objs->world, &z, &outer, &center );
#endif
	
//	DG_GetLightMatrix( &pos, work->lights );
//	DG_SetLightMatrix( objs, work->lights );

}
#endif
static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case CGRT_MSG_SLOW:			//すべての移動の関する値の係数書き換え		[1]10000 で 1.0f
			work->cfcnt.vx = work->cfcnt.vy = work->cfcnt.vz = (float)msg->message[1]/10000.0f;
			printf("Change cfcnt[%f]\n",work->cfcnt.vz);
			break;
		  case CGRT_MSG_SMOKE:			//吸う		[1]吸う時間（タバコが明るいままのframe数）
			work->step = STEP_SMOKING;
			work->color = fpu_Abs( work->color );
			work->s_timer = msg->message[1]<<1;

			DG_InvisiblePrim2(work->msmk_prim) ;

			printf("Smoke Mode\n");
			break;
		  case CGRT_MSG_EMIT:			//吐く		[1]吐く時間（煙を口からだすframe数）
			work->step = STEP_EMITTING;
			work->s_timer = msg->message[1]<<1;
			work->add_alpha = 3.0f/(float)work->s_timer;
			//リセット emit_prim reset
			
			GV_ZeroMemory( work->msmk_dvec, sizeof(FVECTOR)*N_MSMK_ALLVERTS );
			GV_ZeroMemory( work->msmk_data, sizeof(FVECTOR)*N_MSMK_ALLVERTS );
	
			DG_VisiblePrim2(work->msmk_prim) ;
	
			work->l_timer = 0;
			printf("Emit Mode\n");
			break;
		  case CGRT_MSG_THROW:			//捨てる
			//work->step = STEP_THROWING;
			work->flags |= FLAG_CGRT_OBJS_THROW;
#if 0
			DG_COPY_MAT( &work->l_world, &work->cgrt_objs->world );
			work->cfcnt.vx = work->cfcnt.vy = work->cfcnt.vz = 0.5f;
#endif
			printf("Throw Mode\n");
			break;
		  case CGRT_MSG_KILL:			//こーろーすー
			if(work->nsmk_prim) DG_InvisiblePrim2(work->nsmk_prim);
			if(work->msmk_prim) DG_InvisiblePrim2(work->msmk_prim);
			if(work->ash_prim) DG_InvisiblePrim2(work->ash_prim);
			if(work->frtl_prim) DG_InvisiblePrim2(work->frtl_prim);

			{
				int i;
				for( i = 0; i < N_CGRT_SMOKE; i++ ){
					if(work->csmk_prim[i]) DG_InvisiblePrim2(work->csmk_prim[i]);
				}
			}
			GV_DestroyActor(work) ;
			printf("Cgrt Msg Kill\n");
			return 1;
			break;
		  case CGRT_MSG_LINK:
//			printf("Cgrt Msg Link Change\n");
#if 0
			{
				FMATRIX	*fmtemp;
				fmtemp = work->world;
				work->world = work->sna_rhand;
				work->sna_rhand = fmtemp;
			}
#else
			work->flags |= FLAG_CGRT_SMK_VSBL;
#endif
			break;
		  default:
			printf("Cgrt Msg Err!!\n");
			break;
		}
		msg--;
	}
	return 0;
}
#define		SMOKE_INTNS	(280.0f)
//もし吸ってる途中でメッセージでステップを変えられると可笑しくなるかも。
//いきなり暗くなるなど
static void Act(Work *work)
{
	FVECTOR *light = (FVECTOR*)work->fire[1].m[3];
	//FVECTOR	dvec,rot;
	FVECTOR fvtemp;
	float	ftemp;
	FMATRIX	wind_mat;
	int		wind_int;
	//printf("wind_intns = %d\n",G_wind_intense);
	
	//メッセージチェック
	if( CheckMesgParam( work ) ) return;
	wind_int = OK_GetLocalWind_SH( &work->center, &fvtemp );
	ftemp = (wind_int > 20)?(float)wind_int:(float)G_wind_intense + 20.0f;
	//ftemp = (float)OK_GetLocalWind_SH( &work->center, &fvtemp );
	//風生成
	Wind_Rot.vx = (short)fpu_Abs(work->csmk_rot_x[0]) - 96;
	Wind_Rot.vy = (short)fpu_Abs(work->csmk_rot_y[0]) - 96;
	
	TS_MakeMatrix2( &wind_mat, &Wind_Vec, NULL, &DG_ZeroVector );
	DG_SetPos( &wind_mat );
	DG_RotatePos( &Wind_Rot );
	work->wind.vx = 0.0f;
	work->wind.vy = 0.0f;
	work->wind.vz = ftemp;
	work->wind.vw = 0.0f;
	DG_RotVector( &work->wind, &work->wind, 1 );

//	fvtemp.vw = 0.0f;
//	ftemp = fpu_VectorLength2(&fvtemp);
//	ftemp = fpu_Sqrt( ftemp );
//	printf("l_wnd_intns = %f\t",ftemp);
//	printf("g_wnd_intns = %d\n",G_wind_intense);
	
#if 0
	{
		FVECTOR *pwind = &G_wind;
		
		PRINT_PFVEC(255,pwind);
		pwind = &fvtemp;
		PRINT_PFVEC(444,pwind);
	}
#endif
	work->color += 1.0f;
	
	switch(work->step){
	  case STEP_NORMAL:				//通常処理
		
		ftemp = 1.0f - ftemp/SMOKE_INTNS;
		if(ftemp<0.0f)ftemp = 0.0f;
		if( fpu_Abs(work->smoke_amnt - ftemp) > 0.025f ){
			work->smoke_amnt += ((work->smoke_amnt - ftemp > 0.0f)?-0.025f:0.025f);
		}else{
			work->smoke_amnt = ftemp;
		}
		//work->smoke_amnt = 1.0f - (float)G_wind_intense/180.0f;//(float)G_wind_intense_max;
		
		if(work->smoke_amnt<0.0f)work->smoke_amnt = 0.0f;
		if(work->color > 30.0f) work->color = -30.0f;
		break;
	  case STEP_SMOKING:			//吸う処理
		
		if(work->color > 150.0f) work->color = 150.0f;
		

		ftemp = 1.0f - ftemp/SMOKE_INTNS;
		if(ftemp<0.0f)ftemp = 0.0f;
		if( fpu_Abs(work->smoke_amnt - ftemp) > 0.025f ){
			work->smoke_amnt += ((work->smoke_amnt - ftemp > 0.0f)?-0.025f:0.025f);
		}else{
			work->smoke_amnt = ftemp;
		}
		//処理終了ノーマルへ
		if( work->s_timer-- < 0){
			if( work->color > 0.0f ) work->color *= -1.0f;
			work->s_timer = 0;
			work->step = STEP_NORMAL;
			printf("End Smoke\n");
		}
		break;
	  case STEP_EMITTING:			//吐く処理

		work->l_timer++;
		MouthSmoke_Act( work );
	
		if(work->color > 30.0f) work->color = -30.0f;
		
		if(work->flags & FLAG_CGRT_SMK_VSBL){
			ftemp = 1.0f - ftemp/SMOKE_INTNS;
			if(ftemp<0.0f)ftemp = 0.0f;
			if( fpu_Abs(work->smoke_amnt - ftemp) > 0.025f ){
				work->smoke_amnt += ((work->smoke_amnt - ftemp > 0.0f)?-0.025f:0.025f);
			}else{
				work->smoke_amnt = ftemp;
			}
		}else{
			work->smoke_amnt -= 0.025f;
			if(work->smoke_amnt<0.2f)work->smoke_amnt = 0.2f;
		}
		work->emit_amnt += work->add_alpha;
		if( work->emit_amnt < 0.0f ){
			work->emit_amnt = 0.0f;
		}else if( work->emit_amnt > 1.0f ){
			work->emit_amnt = 1.0f;
			work->add_alpha = -EMIT_ADD_ALPHA/2.0f;
		}
		
		//処理終了ノーマルへ
		if( work->s_timer-- < 0){
			work->s_timer = 0;
			work->step = STEP_NORMAL;
			DG_InvisiblePrim2(work->msmk_prim) ;
			printf("End Emit\n");
		}
		break;
	  default:
		printf("ERR!! Cigrette.c in act\n");
		break;
	}
#if CGRT_DEBUG
	
	if(work->flags & FLAG_CGRT_OBJS_THROW){
		_sceVu0MulVector( &dvec, &work->dire, &work->cfcnt );
		_sceVu0MulVector( &rot, &work->rot, &work->cfcnt );
		DG_COPY_VEC( &fvtemp, (FVECTOR*)work->l_world.m[3] );	//pos退避
		
		_sceVu0RotMatrix( &work->l_world, &work->l_world, &rot );	//回転
		_sceVu0AddVector( (FVECTOR*)work->l_world.m[3], &fvtemp, &dvec );	//pos復帰

		DG_COPY_MAT( &work->cgrt_objs->world, &work->l_world );
		work->dire.vy -= work->cfcnt.vx * 0.5f;//重力
	}else{
		Cgrt_Norm_Act( work );
	}
#else
#if 0
	if(work->flags & FLAG_CGRT_OBJS_THROW){
		_sceVu0MulVector( &dvec, &work->dire, &work->cfcnt );
		_sceVu0MulVector( &rot, &work->rot, &work->cfcnt );
		DG_COPY_VEC( &fvtemp, (FVECTOR*)work->l_world.m[3] );	//pos退避
		
		_sceVu0RotMatrix( &work->l_world, &work->l_world, &rot );	//回転
		_sceVu0AddVector( (FVECTOR*)work->l_world.m[3], &fvtemp, &dvec );	//pos復帰

		DG_COPY_MAT( &work->cgrt_objs->world, &work->l_world );
		work->dire.vy -= work->cfcnt.vx * 0.5f;//重力
	}else{
#endif
		DG_COPY_MAT( &work->cgrt_objs->world, work->world );			//Cgrt Obj Act
//	}
#endif
	DG_COPY_VEC( &work->pre_center, &work->center );
	DG_SetPos( &work->cgrt_objs->world ) ;
	DG_PutVector( &Frtl_Shiff, &work->center, 1 ) ;	//先っ歩位置
	{
		//タバコの煙

		int i;
		for( i = 0; i < N_CGRT_SMOKE; i++ ) CgrtSmoke_Act( work, i );
		CgrtSmoke2_Act( work, 0 );
	}
	light->vx = 70.0f + fpu_Abs( work->color );
	light->vy = 35.0f + fpu_Abs( work->color );
	light->vz = 35.0f + fpu_Abs( work->color );

}

//static Work	*test = NULL;

static void Die(Work *work)
{
	int i;
	for( i = 0; i < N_CGRT_SMOKE; i++ ){
		if(work->csmk_prim[i]) GM_FreePrim2(work->csmk_prim[i]);
	}
	
	if(work->csmk_prim2) GM_FreePrim2(work->csmk_prim2);
	if(work->nsmk_prim) GM_FreePrim2(work->nsmk_prim);
	if(work->msmk_prim) GM_FreePrim2(work->msmk_prim);
	if(work->ash_prim) GM_FreePrim2(work->ash_prim);
//	if(work->fir_prim) GM_FreePrim2(work->fir_prim);
	if(work->frtl_prim) GM_FreePrim2(work->frtl_prim);
	
	if( work->cgrt_objs ){
		DG_DequeueObjs( work->cgrt_objs );
		DG_FreeObjs( work->cgrt_objs ) ;
	}
//	test = NULL;
}

//吐く煙の初期化
static DG_PRIM2 *InitEmitSmokePrim( int tex_code, int alpha, int n_prims, int n_verts, int color )
{
	int					i,u0,v0,u1,v1;
	DG_PRIM2			*prim = NULL;
	DG_TEX				*tex = NULL;
	FVECTOR				*pos0, *pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0, *uvrgbwh1;
	short				r,g,b,a;
	
	tex = DG_GetTexture(tex_code);
	if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return(NULL); }
	
//	prim = GM_MakePrim2(DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, n_prims, n_verts );
	prim = GM_MakePrim2(DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, n_prims, n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM2!! cgrt.c\n"); return (NULL); }
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, alpha );

	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	r = color>>24;
	g = (color>>16)&0xff;
	b = (color>>8)&0xff;
	a = color&0xff;
	
	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgbwh0 = prim->uvrgb[0];
	uvrgbwh1 = prim->uvrgb[1];
	
	for( i = 0; i < n_prims*n_verts; i++ ){
		DG_COPY_VEC( pos0, &DG_ZeroVector );
		DG_COPY_VEC( pos1, &DG_ZeroVector );

		uvrgbwh0->r = r;
		uvrgbwh0->g = g;
		uvrgbwh0->b = b;
		uvrgbwh0->a = a;
		
		uvrgbwh0->u0 = u0;
		uvrgbwh0->v0 = v0;
		uvrgbwh0->u1 = u1;
		uvrgbwh0->v1 = v1;
		
		uvrgbwh0->f0 = 0;
		uvrgbwh0->f1 = 0;
		uvrgbwh0->q0 = 4096;
		uvrgbwh0->q1 = 4096;
		
		uvrgbwh0->w = 0;
		uvrgbwh0->h = 0;
		
		*uvrgbwh1 = *uvrgbwh0;
		
		pos0++; pos1++;
		uvrgbwh0++; uvrgbwh1++;
	}
	
	return prim;
}

#if 0
//火種の尻尾の初期化
//三次元 -> カメラ変換 四角ポリゴン
static int InitFireTailPrim( Work *work )
{
	int				i,j,u[2],v[2],check = 0;
	DG_PRIM2		*prim = NULL;
	DG_TEX			*tex = NULL;
	FVECTOR			*pos0, *pos1,fvtemp,init_pos[4];
	DG_PRIM2_UVRGB	*uvrgb0, *uvrgb1;
	float			screen_near_x,screen_near_y;

	screen_near_x = SCREEN_Z / (ASPECT_X()                                           ) / DG_Chanls[0].screen;
	screen_near_y = SCREEN_Z / (ASPECT_Y() * DG_Chanls[0].width / DG_Chanls[0].height) / DG_Chanls[0].screen;
	
	tex = DG_GetTexture(FIRETAIL_TEX_CODE);
	if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return(-1); }
	
	prim = work->frtl_prim = GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_ON_CAMERA,
										  N_FRTL_PRIMS,
										  N_FRTL_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM2!! cgrt.c\n"); return (-1); }
	
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	
	DG_SetPos( &work->cgrt_objs->world ) ;
	DG_PutVector( &Frtl_Shiff, &fvtemp, 1 ) ;	//先っ歩位置
	printf("center pos vx%f:vy %f:vz %f\n",fvtemp.vx,fvtemp.vy,fvtemp.vz);
	
	DG_COPY_VEC( &work->center, &fvtemp );
	
	DG_SetPos( &DG_Chanls[0].eye );
	DG_RotVector( Frtl_InitVerts, init_pos, 4 );
	for( i = 0; i < 4; i++ ) _sceVu0AddVector( &init_pos[i], &init_pos[i], &fvtemp );
//								 printf("init_pos vx%f:vy %f:vz %f\n",init_pos[i].vx,init_pos[i].vy,init_pos[i].vz);
	
	DG_SetPos( &DG_Chanls[0].eye_pers );
	DG_PutVector( init_pos, init_pos, 4 );

	for( i = 0; i < 4; i++ ){
		if(init_pos[i].vz > init_pos[i].vw * 1.4f) check++;
		
		init_pos[i].vw = fpu_Abs(init_pos[i].vw);
		init_pos[i].vz = SCREEN_Z;
		init_pos[i].vx = screen_near_x * init_pos[i].vx / init_pos[i].vw;
		init_pos[i].vy = screen_near_y * init_pos[i].vy / init_pos[i].vw;
	}
	
	if( check == 4 ) memset( init_pos, 0, sizeof(FMATRIX) );
	
	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];
	
	for( i = 0; i < N_FRTL_VERTS/4; i++ ){
		DG_COPY_MAT( pos0, &init_pos );
		DG_COPY_MAT( pos1, &init_pos );
		for( j = 0; j < 4; j++ ){
			uvrgb0->r = 0xf0;
			uvrgb0->g = 0x40;
			uvrgb0->b = 0x10;
			uvrgb0->a = 0x11 - i;
		
			uvrgb0->u = u[j&1];
			uvrgb0->v = v[j/2];
			uvrgb0->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb0->q = 4096;
		
			*uvrgb1 = *uvrgb0;
		
			uvrgb0++; uvrgb1++;
		}
		pos0+=4; pos1+=4;
	}
	
	return 0;
}
#endif

//タバコ煙
static DG_PRIM2 *InitCgrtSmokePrim( Work *work, int tex_code, int num )
{
	int					i,u0,v0,u1,v1;
	DG_PRIM2 			*prim;
	DG_TEX				*tex;
	FVECTOR				*pos0, *pos1;
	DG_PRIM2_UVRGB		*uvrgb0, *uvrgb1;
	
//	FVECTOR				*joint = work->csmk_joint_pos[num];
//	FVECTOR				*joint_dvec = work->csmk_joint_dvec[num];
//	float				*size = work->csmk_joint_size[num]; <-- BP COMMENTED BECAUSE NOT USED AND num can be 666
//	float				*alpha = work->csmk_joint_alpha[num]; <-- BP COMMENTED BECAUSE NOT USED AND num can be 666
	
	tex = DG_GetTexture(tex_code);
	if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return (NULL); }
	
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
						 N_CSMK_PRIMS,
						 N_CSMK_VERTS );
	if(!prim){ printf("ERR!! MAKE PRIM2!! cgrt.c\n"); return (NULL); }

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];

	for( i = 0; i < N_CSMK_ALLVERTS; i++ ){
		DG_COPY_VEC( pos0, &work->center );
		DG_COPY_VEC( pos1, &work->center );

		uvrgb0->r = 0x80;
		uvrgb0->g = 0x80;
		uvrgb0->b = 0x80;
		//uvrgb0->a = 0x20 - (i/2);
		uvrgb0->a = 0x40;
		
		uvrgb0->q = 4096;
		uvrgb0->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		
		uvrgb0->u = (i&1)?u1:u0;
		//uvrgb0->v = (i&4)?v1:v0;//FTOI12((float)(i/2) * 1.0f/32.0f * tex->v_scale + tex->v_offset );
		uvrgb0->v = FTOI12((float)(i/2) * 1.0f/32.0f * tex->v_scale + tex->v_offset );

		*uvrgb1 = *uvrgb0;
#if 0
		if( i < N_CSMK_JOINT ){

			DG_COPY_VEC( joint, &work->center );

			
			*size = 0.0f;
			*alpha = 0.0f;
			DG_COPY_VEC( joint_dvec, &DG_ZeroVector );
//			PRINT_PFVEC(i,joint);
			joint++;joint_dvec++;
			alpha++;
			size++;
		}
#endif
		//*size = 0.0f;
		//*alpha = 0.0f;
		
		pos0++; pos1++;
		uvrgb0++; uvrgb1++;
//		alpha++; <-- BP COMMENTED BECAUSE NOT USED AND num can be 666
//		size++; <-- BP COMMENTED BECAUSE NOT USED AND num can be 666
	}
	if(num != 666){
		work->csmk_rot_x[num] = 0.0f;
		work->csmk_rot_y[num] = 0.0f;
		work->csmk_size[num] = 0.0f;
	}
	return prim;
}

static int InitCgrtObj( Work *work )
{
	DG_DEF		*def = NULL;
	DG_OBJS		*objs = NULL;
//	FVECTOR		pos,dire;
	
	def = (DG_DEF*)GV_GetCache( GV_CacheID( MODEL_CODE, 'k' ) ) ;
	if(!def){ printf("ERR!! NO MODEL!! Cgrt.c\n"); return -1; }
	
	objs = work->cgrt_objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
	if(!objs){ printf("ERR!! MAKE OBJ!! Cgrt.c\n"); return -1; }
	
	if( DG_QueueObjs( objs ) < 0 ) return -1;
	
#if CGRT_DEBUG

	{
		FVECTOR		z = { 0.0f, 1.0f, 0.0f, 0.0f };
		FVECTOR		outer = { 0.0f, 0.0f, 1.0f, 0.0f };
		FVECTOR		center = { 0.0f, 68000.0f, 16000.0f, 0.0f };
	
		TS_MakeMatrix2( &objs->world, &z, &outer, &center );
	}

	//printf("pos vx%f:vy %f:vz %f\n",pos.vx,pos.vy,pos.vz);
#else
	DG_COPY_MAT( &objs->world, work->world );
#endif
	DG_COPY_MAT(&work->fire[0],&objs->objs[1].light[0]) ;
	DG_COPY_MAT(&work->fire[1],&objs->objs[1].light[1]) ;
	objs->objs[1].light = work->fire;
//	objs->objs[0].light = work->fire;
	
	return 0;

}

static int GetResources( Work *work, int name,
						 FMATRIX *world, FMATRIX *sna_head, FMATRIX *sna_rhand,
						 float force, FVECTOR *dire )
{
	work->name = name;
	work->m_timer = 0;
	work->s_timer = 0;
	work->l_timer = 0;
	work->flags = 0;
	work->step = 0;
	work->force = force;
	work->world = world;
	work->sna_head = sna_head;
	work->sna_rhand = sna_rhand;

	work->nsmk_prim = NULL;
	work->ash_prim = NULL;
	work->frtl_prim = NULL;
	
	work->cfcnt.vx = 1.0f;
	work->cfcnt.vy = 1.0f;
	work->cfcnt.vz = 1.0f;
	work->cfcnt.vw = 1.0f;
	
//	DG_COPY_VEC( &work->dire, dire );
	 _sceVu0Normalize(&work->dire,dire);
	 _sceVu0ScaleVector(&work->dire,&work->dire,force);
//	DG_COPY_VEC( &work->color, &DG_ZeroVector );
	work->rot.vx = M_PI / 30.0f;
	work->rot.vy = 0.0f;
	work->rot.vz = M_PI / 30.0f;
	work->rot.vw = 0.0f;
	work->color = 0.0f;

	work->emit_amnt = 0.0f;
	work->smoke_amnt = 0.0f;
	work->add_alpha = 0.0f;

	//タバコオブジェクト初期化
	if( InitCgrtObj( work ) ) return -1;
	
	DG_SetPos( &work->cgrt_objs->world ) ;
	//先っ歩位置
	DG_PutVector( &Frtl_Shiff, &work->center, 1 ) ;
	printf("vx %f: vy %f: vz %f:\n",work->center.vx,work->center.vy,work->center.vz);
	DG_COPY_VEC( &work->pre_center, &work->center );
	//火種初期化
//	if( InitFireTailPrim( work ) ) return -1;
#if 1
	//タバコ煙初期化
	{
		int i,j;
		int tex_code[] = {
		//	CSMK_TEX_CODE1,
			CSMK_TEX_CODE0,
			CSMK_TEX_CODE2,
			CSMK_TEX_CODE1,
			
		};
#if 1
		for( i = 0; i < N_CGRT_SMOKE; i++ ){
			if( (work->csmk_prim[i] = InitCgrtSmokePrim( work, tex_code[i%2], i )) == NULL ) return -1;
			//DG_InvisiblePrim2(work->csmk_prim[i]) ;

			for( j = 0; j < N_CSMK_JOINT; j++ ){
				DG_COPY_VEC( &work->csmk_joint_pos[i][j], &work->center );
				DG_COPY_VEC( &work->csmk_joint_dvec[i][j], &DG_ZeroVector );
				work->csmk_joint_alpha[i][j] = 0.0f;
				work->csmk_joint_size[i][j] = 0.0f;

			}
		}
#endif
		//例外君後々整理する
		if( (work->csmk_prim2 = InitCgrtSmokePrim( work, tex_code[2], 666 )) == NULL ) return -1;
		for( i = 0; i < N_CSMK_JOINT; i++ ){
			DG_COPY_VEC( &work->csmk_joint_pos2[i], &work->center );
			DG_COPY_VEC( &work->csmk_joint_dvec2[i], &DG_ZeroVector );
			work->csmk_joint_alpha2[i] = 0.0f;
			work->csmk_joint_size2[i] = 0.0f;
		}
		{
			DG_TEX *tex = DG_GetTexture(tex_code[2]);
			if(!tex){ printf("ERR!! NO TEX!! cgrt.c\n"); return (-1); }
		
			work->min_v = 0.0f * tex->v_scale + tex->v_offset ;
			work->len_v = 1.0f * tex->v_scale + tex->v_offset - work->min_v;
			work->now_v = 0.0f;
			work->diff_v = work->len_v / 32.0f;
			work->dvec_v = work->len_v / 128.0f;
		}

		//ここまで
		//DG_InvisiblePrim2(work->csmk_prim2) ;
	}
#endif
	//口煙初期化
	work->msmk_prim = InitEmitSmokePrim( MSMK_TEX_CODE, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ),
										 N_MSMK_PRIMS, N_MSMK_VERTS, MSMK_COLOR );
	if( !work->msmk_prim ) return -1;
	GV_ZeroMemory( work->msmk_dvec, sizeof(FVECTOR)*N_MSMK_ALLVERTS );
	GV_ZeroMemory( work->msmk_data, sizeof(FVECTOR)*N_MSMK_ALLVERTS );
/*
	//鼻煙初期化
	work->nsmk_prim = InitEmitSmokePrim( NSMK_TEX_CODE, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ),
										 N_NSMK_PRIMS, N_NSMK_VERTS, NSMK_COLOR );
	if( !work->nsmk_prim ) return -1;
	GV_ZeroMemory( work->nsmk_dvec, sizeof(FVECTOR)*N_NSMK_ALLVERTS );
	GV_ZeroMemory( work->nsmk_data, sizeof(FVECTOR)*N_NSMK_ALLVERTS );
*/
//	DG_InvisiblePrim2(work->frtl_prim) ;
	DG_InvisiblePrim2(work->msmk_prim) ;
	
	return 0;
}

/*
int			name:		メッセージ受けとる名前
FMATRIX		*world:		タバコ投げるまでのワールドのポインタ
FMATRIX		*sna_head:	スネーク頭
FMATRIX		*sna_rhand:	スネーク右手
float		force:		投げる強さ
FVECTOR		*dire:		投げる方向
*/

void *NewCigarette_Demo( int name,
						 FMATRIX *world, FMATRIX *sna_head, FMATRIX *sna_rhand,
						 float force, FVECTOR *dire )
{
	Work *work = NULL;
#if 0
//	return NULL;
	
	if(test){
//		GV_DestroyActor(test) ;
//		test = NULL;
		return NULL;
	}
#endif
	work = (Work*)GV_NewEffect( GV_ACTOR_EFFECT, sizeof(Work) );
	if(work){
		GV_SetActor( &(work->actor), Act, Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources( work, name, world, sna_head, sna_rhand, force, dire ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
//		test = work;
		printf("Make Cgrt\n");
	}
	
	printf("MEM_SCR_MSMK_BOTTOM = %p\n",MEM_SCR_MSMK_BOTTOM);
	
	return (void *)work ;
}

void *NewCigarette_Demo_Hand( int name,
							  FMATRIX *world, FMATRIX *sna_head, FMATRIX *sna_rhand,
							  float force, FVECTOR *dire )
{
	Work *work = NULL;
#if 0
//	return NULL;
	
	if(test){
//		GV_DestroyActor(test) ;
//		test = NULL;
		return NULL;
	}
#endif
	work = (Work*)GV_NewEffect( GV_ACTOR_EFFECT, sizeof(Work) );
	if(work){
		GV_SetActor( &(work->actor), Act, Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources( work, name, world, sna_head, sna_rhand, force, dire ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
//		test = work;
		printf("Make Cgrt\n");
	}
	
	printf("MEM_SCR_MSMK_BOTTOM = %p\n",MEM_SCR_MSMK_BOTTOM);
	
	return (void *)work ;
}
