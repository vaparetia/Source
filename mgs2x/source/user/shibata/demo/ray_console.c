//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ray_consol.c
	レイのコンソール

	2000/01/25 T.Shibata

	$Id: ray_console.c,v 1.1.1.3 2002/11/19 11:48:33 Yoshizawa1 Exp $
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
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"


#define CLOCK_COUNT	(BP_BASE_TICK())

static void ActScrPrim( DG_PRIM2 *prim, int n_verts, int alpha );

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define		GET_COL_R(_rgba)	(((_rgba)>>24)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>> 8)&0xff)
#define		GET_COL_A(_rgba)	(((_rgba)>> 0)&0xff)
#define		SET_COL(_r,_g,_b,_a)	(((_r)<<24)|((_g)<<16)|((_b)<<8)|(_a))
//#define 	TS_DEBUG

#define MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define MEM_SCR_UV		((void*)(SCRPAD_ADDR+sizeof(FVECTOR)*256))

/*
top
max = 1882
min = 0
bottom
max = 1618
min = 0
*/

#define		FADE_LEN		(1536)

#define		BRIGHT_LEN		(256)
#define		MIN_POS_Z		(512)// - BRIGHT_LEN/2)
#define		MAX_POS_Z		(1882)
#define		DIFF_POS_Z		(MAX_POS_Z-MIN_POS_Z)
//#define		MAX_FADE_COUNT	(MAX_POS_Z + FADE_LEN + BRIGHT_LEN)
#define		MAX_FADE_COUNT	(2288)

#define		ROOT_BOUND_X			(300.0f)
#define		ROOT_BOUND_Y			(300.0f)
#define		ROOT_ROT				(-1.190289950f)

#define L_SIZE_X					(400.0f)
#define L_SIZE_Y					(32.0f/256.0f*L_SIZE_X*2.0f)

#define S_MAX_SIZE_X					(300.0f/4.0f)
#define S_MAX_SIZE_Y					(S_MAX_SIZE_X)

#define M_MAX_SIZE_X					(300.0f/2.0f)
#define M_MAX_SIZE_Y					(M_MAX_SIZE_X)
#define M_MIN_SIZE_X					(S_MAX_SIZE_X)
#define M_MIN_SIZE_Y					(S_MAX_SIZE_X)

#define M_DIFF_SIZE						(M_MAX_SIZE_X-M_MIN_SIZE_X)

#define MAX_TRI_LEN						(110.0f)
#define MIN_TRI_LEN						(0.0f)
#define TRI_WIDE						(20.0f)
#define DIFF_TRI_LEN					(MAX_TRI_LEN-MIN_TRI_LEN)

#define CANOPY_TOP		(1)
#define CANOPY_BOTTOM	(0)

#define RAYCON_FALGS_LINE			(0x0001)
#define RAYCON_FALGS_END_LINE		(0x0010)
#define RAYCON_FALGS_ALLACT			(0x0002)
#define RAYCON_FALGS_END_ALLACT		(0x0020)
#define RAYCON_FALGS_ALLON			(0x0040)
#define RAYCON_FALGS_ALLONEND		(0x0080)

#define RAYCON_FALGS_ACT1		(0x0400)
#define RAYCON_FALGS_ACT2		(0x0800)
#define RAYCON_FALGS_ACT3		(0x1000)
#define RAYCON_FALGS_ACT4		(0x2000)
#define RAYCON_FALGS_ACT5		(0x4000)
#define RAYCON_FALGS_ACT6		(0x8000)

/*
メタルコクピットディスプレー：
チェストマウントコンパネ
座標関節番より
(±223, 744, -704)
(±223, 594, -767)
*/


extern int DM_FrameSkip ;

typedef struct {
	GV_ACT_EX		actor;
	int				name;
	OBJECT			*object;
	int				flags;
	int				console_timer;

	//キャノピー関係
	DG_PRIM2		*canopy_alp[2];		//不透明
	DG_PRIM2		*canopy_add[2];		//透明　加算で六角形
	DG_PRIM2		*canopy_msk[2];		//ますく
	int				n_verts_in[2];
	int				n_verts_out[2];
	int				can_timer;
	int				can_step;

	//ぬらりエフェクト関係
	DG_OBJS			*objs;				//カバー
	DG_PRIM2		*line_eft;			//ラインエフェクト用

	float			r_line_eft;
	FVECTOR			line_eft_center;
/*
開始点(rootから)：0, 207, 1000
幅：1500
終了時の半径：1100程のシリンダ
フレーム数:40～50フレー
*/
	//サイト関係
	DG_PRIM2		*con_l_circle;
	DG_PRIM2		*con_m_circle;
	DG_PRIM2		*con_s_circle;
	DG_PRIM2		*con_tri;
	DG_PRIM2		*con_r_mem;
	DG_PRIM2		*con_l_mem;
	DG_PRIM2		*con_pos_mem;

	short			si_timer;
	short			si_step;
	float			si_m_x;
	float			si_m_y;
	float			si_m_size;
	float			si_s_x;
	float			si_s_y;

	//文字列
	DG_PRIM2		*con_str;
	DG_PRIM2		*con_str_bg;

	u_short			str_timer;
	short				str_step;
	DG_TEX			*str_tex;
	
	//テンション関係
	DG_PRIM2		*con_tension;
	int				tension;
	float			r_rot_x;
	float			l_rot_x;	
	int				ten_step;

	//リアヴュー関係
	DG_PRIM2		*rear_view;
	u_short			rear_timer;
	short			rear_step;

	//タッチパネル
	DG_PRIM2		*touch_panel;
	u_short			touch_timer;
	short			touch_step;
	
	//メインコンパネ
	DG_PRIM2		*main_con;
	u_short			main_timer;
	short			main_step;

	FMATRIX			root;
} Work;

typedef struct {
	short	vx;
	short	vy;
	short	vz;
	short	u;
	short	v;
	short	flags;
	int		count;
}VERTS_DATA;

//コンソールステップ
//起動
//移動テスト
//追従
//ズームテスト

enum {
	CON_STEP_SCR_WAKE = 0,	//起動123統合	メインコンパネ	//飽和
	CON_STEP_SCR_ACT1,		//								//落ち着く
	CON_STEP_SCR_ACT2,		//								//待ち
	
	CON_STEP_ACT1_WAKE = 0,	//起動１	メインコンパネ
	CON_STEP_ACT1_ACT,		//
	
	CON_STEP_ACT2_WAKE = 0,	//起動２	タッチパネル
	CON_STEP_ACT2_ACT,		//
	
	CON_STEP_ACT3_WAKE = 0,	//起動３	リアヴュー
	CON_STEP_ACT3_ACT,
	
	CON_STEP_ACT4_WAKE = 0,	//起動４	テンション
	CON_STEP_ACT4_ACT,

	CON_STEP_ACT5_WAKE = 0,	//起動５	文字列
	CON_STEP_ACT5_MOVE,		//			動く
	CON_STEP_ACT5_WAIT,		//			待つ
	CON_STEP_ACT5_WAIT2,	//			さらに待つ

	CON_STEP_ACT6_WAKE = 0,	//起動５サイト	起動
	CON_STEP_ACT6_MOVE1,	//起動５		移動１
	CON_STEP_ACT6_MOVE2,	//起動５		移動２
	CON_STEP_ACT6_MOVE3,	//起動５		移動３
	CON_STEP_ACT6_MOVE4,	//起動５		移動３
	CON_STEP_ACT6_ZOOM1,	//起動５		ズーム１
	CON_STEP_ACT6_ZOOM2,	//起動５		ズーム２
};

static FVECTOR RearView_Pos[] = {
#if 0
	{ 325.0f, 670.0f, -452.0f, 1.0f },
	{ 265.0f, 670.0f, -452.0f, 1.0f },
	{ 325.0f, 620.0f, -452.0f, 1.0f },
	{ 265.0f, 620.0f, -452.0f, 1.0f },

	{ -325.0f, 670.0f, -452.0f, 1.0f },
	{ -265.0f, 670.0f, -452.0f, 1.0f },
	{ -325.0f, 620.0f, -452.0f, 1.0f },
	{ -265.0f, 620.0f, -452.0f, 1.0f },
#else
	{ 322.0f, 666.0f, -452.0f, 1.0f },
	{ 262.0f, 666.0f, -452.0f, 1.0f },
	{ 322.0f, 616.0f, -452.0f, 1.0f },
	{ 262.0f, 616.0f, -452.0f, 1.0f },

	{ -322.0f, 666.0f, -452.0f, 1.0f },
	{ -262.0f, 666.0f, -452.0f, 1.0f },
	{ -322.0f, 616.0f, -452.0f, 1.0f },
	{ -262.0f, 616.0f, -452.0f, 1.0f },
#endif
};

static FVECTOR TouchPanel_Pos[] = {
#if 1
	{ 390.0f, 500.0f, -560.0f, 1.0f },
	{ 260.0f, 500.0f, -560.0f, 1.0f },
	{ 390.0f, 325.0f, -624.0f, 1.0f },
	{ 260.0f, 325.0f, -624.0f, 1.0f },
	
	{ -390.0f, 500.0f, -560.0f, 1.0f },
	{ -260.0f, 500.0f, -560.0f, 1.0f },
	{ -390.0f, 325.0f, -624.0f, 1.0f },
	{ -260.0f, 325.0f, -624.0f, 1.0f },
#else
	{ 390.0f, 500.0f, -564.0f, 1.0f },
	{ 260.0f, 500.0f, -564.0f, 1.0f },
	{ 390.0f, 325.0f, -628.0f, 1.0f },
	{ 260.0f, 325.0f, -628.0f, 1.0f },
	
	{ -390.0f, 500.0f, -564.0f, 1.0f },
	{ -260.0f, 500.0f, -564.0f, 1.0f },
	{ -390.0f, 325.0f, -628.0f, 1.0f },
	{ -260.0f, 325.0f, -628.0f, 1.0f },
#endif
};

static FVECTOR MainConPane_Pos[] = {
	{  223.0f, 744.0f, -704.0f, 1.0f },
	{ -223.0f, 744.0f, -704.0f, 1.0f },
	{  223.0f, 594.0f, -767.0f, 1.0f },
	{ -223.0f, 594.0f, -767.0f, 1.0f },
};
/*
(-370, 1807, -909)
(-296, 1662, -506)
(-240, 1683, -506)
(-314, 1828, -909)
*/
#define		CONRMEM_ROT_Z		(0.2491f)
#define		CONRMEM_CENTER_X	(-350.0f)
#define		CONRMEM_CENTER_Y	(0.0f)
static FVECTOR ConRMem_Pos[] = {
	{  20.0f,  300.0f, 0.0f, 1.0f },
	{ -20.0f,  300.0f, 0.0f, 1.0f },
	{  20.0f, -400.0f, 0.0f, 1.0f },
	{ -20.0f, -400.0f, 0.0f, 1.0f },
};

static FVECTOR ConMemMem_Pos[] = {
	{  0.0f,  300.0f, 0.0f, 1.0f },
	{  0.0f, -400.0f, 0.0f, 1.0f },
};

/*
(333, 1734, -708)
(277, 1756, -708)
(296, 1662, -506)
(240, 1683, -506)
*/
#define		CONLMEM_ROT_Z		(-0.2491f)
#define		CONLMEM_CENTER_X	(350.0f)
#define		CONLMEM_CENTER_Y	(-50.0f)
static FVECTOR ConLMem_Pos[] = {
	{  20.0f,  100.0f, 0.0f, 1.0f },
	{ -20.0f,  100.0f, 0.0f, 1.0f },
	{  20.0f, -100.0f, 0.0f, 1.0f },
	{ -20.0f, -100.0f, 0.0f, 1.0f },
};

#define		CONPOSMEM_ROT_Z		(0.2491f+PI/2.0f)
#define		CONPOSMEM_ALPHA		(128)
static FVECTOR ConPosMem_Pos[] = {
	{  4.0f,  0.0f, 0.0f, 1.0f },
	{ -4.0f,  0.0f, 0.0f, 1.0f },
	{  4.0f, -20.0f, 0.0f, 1.0f },
	{ -4.0f, -20.0f, 0.0f, 1.0f },
};

static VERTS_DATA CanopyVerts_Bottom[40] = {
	{    0,  207,  917, 4095, 3056, 1,    0 },
	{ -261,  379,  437, 2547, 2419, 1,  480 },
	{ -151,  134,  372, 3182, 3309, 0,  545 },
	{ -300,  362,  332, 2319, 2504, 0,  585 },
	{ -242,  112,   74, 2660, 3386, 0,  843 },
	{ -478,  378, -180, 1301, 2449, 0, 1097 },
	{ -576,  274, -661,  736, 2813, 0, 1578 },
	{  261,  379,  437, 2547, 2419, 1,  480 },
	{  151,  134,  372, 3182, 3309, 1,  545 },
	{  300,  362,  332, 2319, 2504, 0,  585 },
	{  242,  112,   74, 2660, 3386, 0,  843 },
	{  478,  378, -180, 1301, 2449, 0, 1097 },
	{  576,  274, -661,  736, 2813, 0, 1578 },
	{  704,  558, -701,    0, 1812, 0, 1618 },
	{    0,  207,  917, 4095, 3056, 1,    0 },
	{ -151,  134,  372, 3182, 3309, 1,  545 },
	{    0,    0,  502, 4095, 3796, 0,  415 },
	{ -242,  112,   74, 2660, 3386, 0,  843 },
	{    0,  -85,  239, 4095, 4095, 0,  678 },
	{  151,  134,  372, 3182, 3309, 1,  545 },
	{    0,    0,  502, 4095, 3796, 1,  415 },
	{  242,  112,   74, 2660, 3386, 0,  843 },
	{    0,  -85,  239, 4095, 4095, 0,  678 },
	{ -478,  378, -180, 1301, 2449, 1, 1097 },
	{ -536,  468, -244,  962, 2127, 1, 1161 },
	{ -576,  274, -661,  736, 2813, 0, 1578 },
	{ -704,  558, -701,    0, 1812, 0, 1618 },
	{  261,  379,  437, 2547, 2419, 1,  480 },
	{    0,  207,  917, 4095, 3056, 1,    0 },
	{  151,  134,  372, 3182, 3309, 0,  545 },
	{    0,    0,  502, 4095, 3796, 0,  415 },
	{ -261,  379,  437, 2547, 2419, 1,  480 },
	{ -361,  465,  225, 1947, 2129, 1,  692 },
	{ -300,  362,  332, 2319, 2504, 0,  585 },
	{  478,  378, -180, 1301, 2449, 1, 1097 },
	{  704,  558, -701,    0, 1812, 1, 1618 },
	{  536,  468, -244,  962, 2127, 0, 1161 },
	{  300,  362,  332, 2319, 2504, 1,  585 },
	{  361,  465,  225, 1947, 2129, 1,  692 },
	{  261,  379,  437, 2547, 2419, 0,  480 }, 
};

static VERTS_DATA CanopyVerts_Top[82] = {
	{  314, 1969, -1314, 2271,  320, 1, 1881 },
	{  153, 2030, -1314, 3199,  107, 1, 1881 },
	{  443, 1800, -942, 1529,  918, 0, 1509 },
	{  218, 1879, -922, 2828,  637, 0, 1489 },
	{  362, 1648, -527, 1995, 1453, 0, 1094 },
	{  177, 1721, -512, 3061, 1192, 0, 1079 },
	{  290, 1520, -185, 2406, 1904, 0,  752 },
	{  141, 1580, -172, 3268, 1691, 0,  739 },
	{  261, 1379,   87, 2547, 2419, 0,  480 },
	{  134, 1422,  119, 3307, 2249, 0,  448 },
	{    0, 1207,  567, 4095, 3056, 0,    0 },
	{    0, 1442,  147, 4095, 2176, 0,  420 },
	{ -134, 1422,  119, 3307, 2249, 0,  448 },
	{ -153, 2030, -1314, 3199,  107, 1, 1881 },
	{ -314, 1969, -1314, 2271,  320, 1, 1881 },
	{ -218, 1879, -922, 2828,  637, 0, 1489 },
	{ -443, 1800, -942, 1529,  918, 0, 1509 },
	{ -177, 1721, -512, 3065, 1192, 0, 1079 },
	{ -361, 1648, -527, 2002, 1453, 0, 1094 },
	{ -141, 1580, -172, 3268, 1691, 0,  739 },
	{ -290, 1520, -185, 2406, 1904, 0,  752 },
	{ -134, 1422,  119, 3307, 2249, 0,  448 },
	{ -261, 1379,   87, 2547, 2419, 0,  480 },
	{    0, 1207,  567, 4095, 3056, 0,    0 },
	{  153, 2030, -1314, 3199,  107, 1, 1881 },
	{    0, 2055, -1315, 4095,    0, 1, 1882 },
	{  218, 1879, -922, 2828,  637, 0, 1489 },
	{    0, 1916, -902, 4095,  505, 0, 1469 },
	{  177, 1721, -512, 3061, 1192, 0, 1079 },
	{    0, 1753, -497, 4095, 1080, 0, 1064 },
	{  141, 1580, -172, 3268, 1691, 0,  739 },
	{    0, 1598, -159, 4095, 1625, 0,  726 },
	{  134, 1422,  119, 3307, 2249, 0,  448 },
	{    0, 1442,  147, 4095, 2176, 0,  420 },	
	{  290, 1520, -185, 2406, 1904, 1,  752 },
	{  361, 1465, -125, 1947, 2129, 1,  692 },
	{  290, 1570, -185, 2406, 1904, 0,  752 },
	{  361, 1515, -125, 1947, 2129, 0,  692 },
	{  361, 1465, -125, 1947, 2129, 1,  692 },
	{  290, 1520, -185, 2406, 1904, 1,  752 },
	{  261, 1379,   87, 2547, 2419, 0,  480 },


	
	{    0, 1916, -902, 4095,  505, 1, 1469 },
	{ -218, 1879, -922, 2828,  637, 1, 1489 },
	{    0, 1753, -497, 4095, 1080, 0, 1064 },
	{ -177, 1721, -512, 3065, 1192, 0, 1079 },
	{    0, 1598, -159, 4095, 1625, 0,  726 },
	{ -141, 1580, -172, 3268, 1691, 0,  739 },
	{    0, 1442,  147, 4095, 2176, 0,  420 },
	{ -134, 1422,  119, 3307, 2249, 0,  448 },
	{  314, 1969, -1314, 2271,  320, 1, 1881 },
	{  153, 2080, -1314, 3199,  107, 1, 1881 },
	{  153, 2030, -1314, 3199,  107, 0, 1881 },
	{    0, 2105, -1315, 4095,    0, 0, 1882 },
	{    0, 2055, -1315, 4095,    0, 0, 1882 },
	{ -153, 2080, -1314, 3199,  107, 0, 1881 },
	{ -153, 2030, -1314, 3199,  107, 0, 1881 },
	{ -218, 1879, -922, 2828,  637, 1, 1489 },
	{    0, 1916, -902, 4095,  505, 1, 1469 },
	{ -153, 2030, -1314, 3199,  107, 0, 1881 },
	{    0, 2055, -1315, 4095,    0, 0, 1882 },
	{ -361, 1465, -125, 1947, 2129, 1,  692 },
	{ -290, 1520, -185, 2406, 1904, 1,  752 },
	{ -361, 1515, -125, 1947, 2129, 0,  692 },
	{ -290, 1570, -185, 2406, 1904, 0,  752 },
	{ -261, 1379,   87, 2547, 2419, 1,  480 },
	{ -290, 1520, -185, 2406, 1904, 1,  752 },
	{ -361, 1465, -125, 1947, 2129, 0,  692 },
	{ -314, 1969, -1314, 2271,  320, 1, 1881 },
	{ -153, 2030, -1314, 3199,  107, 1, 1881 },
	{ -314, 2019, -1314, 2271,  320, 0, 1881 },
	{ -153, 2080, -1314, 3199,  107, 0, 1881 },
	{ -443, 1800, -942, 1529,  918, 1, 1509 },
	{ -314, 1969, -1314, 2271,  320, 1, 1881 },
	{ -443, 1850, -942, 1529,  918, 0, 1509 },
	{ -314, 2019, -1314, 2271,  320, 0, 1881 },
	{  314, 1969, -1314, 2271,  320, 1, 1881 },
	{  443, 1800, -942, 1529,  918, 1, 1509 },
	{  314, 2019, -1314, 2271,  320, 0, 1881 },
	{  443, 1850, -942, 1529,  918, 0, 1509 },
	{  153, 2080, -1314, 3199,  107, 1, 1881 },
	{  314, 1969, -1314, 2271,  320, 1, 1881 },
	{  314, 2019, -1314, 2271,  320, 0, 1881 },

};

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
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVectorW(in, out, 1 ) ;
#endif
}

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

static float GetMemYaji_X( FVECTOR *from, FVECTOR *to, float mx, float my, float mrot, float y )
{
	FVECTOR		center = { mx, my, 0.0f, 1.0f };
	FMATRIX		world;
	float		x;
	FVECTOR		pos0,pos1;
	
	_sceVu0RotMatrixZ( &world, &DG_UnitMatrix, mrot );
	DG_COPY_VEC( world.m[3], &center );

	_RotTrans( &pos0, &world, from );
	_RotTrans( &pos1, &world, to );

	x = (y - pos0.vy)*(pos1.vx - pos0.vx)/(pos1.vy - pos0.vy) + pos0.vx;

	return x;
}

enum {
	MSG_RAYCON_LINE_EFT = 0,
	MSG_RAYCON_ACTIVE,
	MSG_RAYCON_ON,
	MSG_RAYCON_DIE,
};

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
		  case MSG_RAYCON_LINE_EFT:
			if(work->flags&0x00ff){
				printf("一番最初じゃなきゃ\n");
			}else{
				work->flags |= RAYCON_FALGS_LINE;
				//printf("line\n");
			}
			break;
		  case MSG_RAYCON_ACTIVE:
           // BP WARNING - The following line does not work as expected because ! has precedence
           // over &.  Not fixing.
			if(!work->flags&RAYCON_FALGS_END_LINE){
				printf("ラインエフェクト終了しなくちゃだめ\n");
			}else{
				work->flags |= RAYCON_FALGS_ALLACT;
				//printf("console\n");
			}
			break;
			
		  case MSG_RAYCON_ON:
			// 緊急起動
			work->flags |= RAYCON_FALGS_ALLON;
			break;
		  case MSG_RAYCON_DIE:
			printf("reboot\n");
#if 0
			{	
				static int GetResources( Work *work );
				static void Die(Work *work);
				work->flags = 0;

				Die(work);
				if(GetResources(work)) printf("aaaa\n");
			}
#endif
			//return 1;
			break;
		  default:
			printf("ray_console.c Msg Err!![%d]\n",num);
			break;
		}
		msg--;
	}

	return 0;
}

static void SetLineEftPrim( DG_PRIM2 *prim, int n_polys, int n_verts, FVECTOR *center, float r, float rot )
{
	FVECTOR			*pos = MEM_SCR_POS;
	FVECTOR			*fvtemp = MEM_SCR_UV;
	int				clock,i,j;
	float			temp = rot;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	//pos	= prim->pos[clock];
	
	for( i = 0; i < 6; i++ ){
		fvtemp->vx = r*cos(temp);
		fvtemp->vy = r*sin(temp);
		fvtemp->vz = 0.0f;
		temp += PI/3.0f;
		fvtemp++;
	}
	fvtemp->vx = r*cos(rot);
	fvtemp->vy = r*sin(rot);
	fvtemp->vz = 0.0f;
	fvtemp = MEM_SCR_UV;

	for( i = 0; i < n_polys; i++ ){
		for( j = 0; j < n_verts; j++ ){
			pos->vx = center->vx + fvtemp->vx;
			pos->vy = center->vy + fvtemp->vy;
			pos->vz = center->vz;
			if(j&1) pos->vz += 1500.0f;

			pos->vw = 1.0f;
			if(j&1)fvtemp++;
			pos++;
		}
		fvtemp--;
	}

	TS_Scr_Mem( prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_polys );
	
}

static void SetMemPos( DG_PRIM2 *prim, FVECTOR *ppoly, float x, float y, float rot, int alpha )
{	
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			*pos, *poly = ppoly;
	int				clock,i;
	FMATRIX			world;
	FVECTOR			center = { x, y, 0.0f, 1.0f };

	_sceVu0RotMatrixZ( &world, &DG_UnitMatrix, rot );
	DG_COPY_VEC( world.m[3], &center );

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];
	pos	= prim->pos[clock];

	for( i = 0; i < 4; i++ ){
		_RotTrans( pos, &world, poly );
		uvrgb->a = alpha;

		poly++;
		pos++;
		uvrgb++;
	}
	
}

static void SetSightPrim( DG_PRIM2 *prim, float x, float y,
						  float size_x, float size_y, int alpha, int flag )
{
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			*pos;
	int				clock,i;
	FMATRIX			world;
	FVECTOR			poly[] = {
		{ -size_x,  size_y, 0.0f, 1.0f },
		{  size_x,  size_y, 0.0f, 1.0f },
		{ -size_x, -size_y, 0.0f, 1.0f },
		{  size_x, -size_y, 0.0f, 1.0f },
	};
	FVECTOR			center = { x, y, 0.0f, 1.0f };
	int				a = alpha;
	if(a<0)a=0;
	if(a>128)a=128;
	if(flag){
		_sceVu0RotMatrixZ( &world, &DG_UnitMatrix, (size_x-M_MIN_SIZE_X)/M_DIFF_SIZE*PI - PI );
		DG_COPY_VEC( world.m[3], &center );
	}
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];
	pos	= prim->pos[clock];

	for( i = 0; i < 4; i++ ){
		//DG_COPY_VEC( pos, &poly[i] );
		if(flag) _RotTrans( pos, &world, &poly[i] );
		else _sceVu0AddVector( pos, &poly[i], &center );
		uvrgb->a = a;//lpha;
		pos++;
		uvrgb++;
	}
}

static void SetTriPrim( DG_PRIM2 *prim, float sx, float sy,
						float mx, float my, int alpha )
{
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			*pos;
	int				clock,i;
	FMATRIX			world;
	FVECTOR			poly[] = {
		{ -TRI_WIDE, 0.0f, 0.0f, 1.0f },
		{  TRI_WIDE, 0.0f, 0.0f, 1.0f },
		{ -TRI_WIDE, -MIN_TRI_LEN, 0.0f, 1.0f },
		{  TRI_WIDE, -MIN_TRI_LEN, 0.0f, 1.0f },
	};	
	FVECTOR			center = { sx, sy, 0.0f, 1.0f };
	float			diff_x,diff_y,len;

	diff_x = sx-mx;
	diff_y = sy-my;
	len = diff_x*diff_x + diff_y*diff_y;
	len = DG_SQRT( len );
	if( len > MAX_TRI_LEN ) len = MAX_TRI_LEN;
	poly[2].vy = -len;
	poly[3].vy = -len;
//printf("%f\n",len);
	len = atan2f( diff_y, diff_x );
//printf("%f\n",len);
	/*
	if(diff_y < 0.0f){
		if( len < 0.0f ) len -= PI;
		else len += PI;
	}
	*/
	_sceVu0RotMatrixZ( &world, &DG_UnitMatrix, -len );
	DG_COPY_VEC( world.m[3], &center );

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];
	pos	= prim->pos[clock];

	for( i = 0; i < 4; i++ ){
		_RotTrans( pos, &world, &poly[i] );
//PRINT_PFVEC(i,pos);
		uvrgb->a = alpha;

		pos++;
		uvrgb++;
	}
}

static int SetSightMove( float *pnow_x, float *pnow_y,
						 float to_x, float to_y, float pow )
{
	float	diff_x,diff_y,len;

	diff_x = to_x - *pnow_x;
	diff_y = to_y - *pnow_y;
	
	len = diff_x*diff_x + diff_y*diff_y;
	if( len <= pow*pow ){
		*pnow_x = to_x;
		*pnow_y = to_y;
		return 1;
	}
	len = DG_SQRT( len );
	
	*pnow_x += pow*diff_x/len;
	*pnow_y += pow*diff_y/len;

	return 0;
}

#define S_CIRCLE_SPEED		(10.0f)
#define M_CIRCLE_SPEED		(S_CIRCLE_SPEED/2.0f)

static void SightAct( Work *work )
{
	int ret;
	float	diff_x,diff_y,len,x;
	
	diff_x = work->si_s_x - work->si_m_x;
	diff_y = work->si_s_y - work->si_m_y;
	len = diff_x*diff_x + diff_y*diff_y;
	len = DG_SQRT( len )/8.0f;
	if( len > 150.0f ) len = 150.0f;
	
	switch( work->si_step ){
	  case CON_STEP_ACT6_WAKE:
		//2
		SetSightPrim( work->con_s_circle, 0.0f, 36.0f, S_MAX_SIZE_X, S_MAX_SIZE_Y, work->si_timer/CLOCK_COUNT - 8*2*4, 0 );
		//3
		SetSightPrim( work->con_m_circle, 0.0f, 36.0f, 150.0f, 150.0f, work->si_timer/CLOCK_COUNT - 8*3*4, 1 );
		//1
		SetSightPrim( work->con_l_circle, 0.0f, 36.0f, L_SIZE_X, L_SIZE_Y, work->si_timer/CLOCK_COUNT - 8*1*4, 0 );

		//SetMemPos( work->con_r_mem, ConRMem_Pos, CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, work->si_timer/CLOCK_COUNT );
		//SetMemPos( work->con_l_mem, ConLMem_Pos, CONLMEM_CENTER_X, CONLMEM_CENTER_Y, CONLMEM_ROT_Z, work->si_timer/CLOCK_COUNT );


		//0
		ActScrPrim( work->con_r_mem, 4, work->si_timer/CLOCK_COUNT );
		ActScrPrim( work->con_l_mem, 4, work->si_timer/CLOCK_COUNT );

		x = GetMemYaji_X( &ConMemMem_Pos[0], &ConMemMem_Pos[1],
		                  CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, 0.0f );
		//0
		//0
		if(work->si_timer/CLOCK_COUNT < 128){
			ActScrPrim( work->con_r_mem, 4, work->si_timer/CLOCK_COUNT );
			ActScrPrim( work->con_l_mem, 4, work->si_timer/CLOCK_COUNT );
			SetMemPos( work->con_pos_mem, ConPosMem_Pos, x, 36.0f, CONPOSMEM_ROT_Z, work->si_timer/CLOCK_COUNT );
		}else{
			ActScrPrim( work->con_r_mem, 4, 128 );
			ActScrPrim( work->con_l_mem, 4, 128 );
			SetMemPos( work->con_pos_mem, ConPosMem_Pos, x, 36.0f, CONPOSMEM_ROT_Z, 128 );
		}
		work->si_timer += 4*5 * (DM_FrameSkip+1);
		if( work->si_timer/CLOCK_COUNT > 128+(8*3*4) ){
			work->si_step++;
			work->si_timer = 0;
		}
		break;
	  case CON_STEP_ACT6_MOVE1:
		ret = SetSightMove( &work->si_s_x, &work->si_s_y, 300.0f-S_MAX_SIZE_X, 300.0f-S_MAX_SIZE_Y, S_CIRCLE_SPEED*(DM_FrameSkip+1) );
		
		SetSightMove( &work->si_m_x, &work->si_m_y, work->si_s_x, work->si_s_y, M_CIRCLE_SPEED*(DM_FrameSkip+1) );

		SetSightPrim( work->con_s_circle, work->si_s_x, work->si_s_y, S_MAX_SIZE_X, S_MAX_SIZE_Y, 128, 0 );
		SetSightPrim( work->con_m_circle, work->si_m_x, work->si_m_y, 150.0f+len, 150.0f+len, 128, 1 );

		SetTriPrim( work->con_tri, work->si_s_x, work->si_s_y, work->si_m_x, work->si_m_y, 128 );

		x = GetMemYaji_X( &ConMemMem_Pos[0], &ConMemMem_Pos[1],
		                  CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, work->si_s_y );
		SetMemPos( work->con_pos_mem, ConPosMem_Pos, x, work->si_s_y, CONPOSMEM_ROT_Z, CONPOSMEM_ALPHA );

		if( ret ){
			work->si_step++;
		}
		break;
	  case CON_STEP_ACT6_MOVE2:
		ret = SetSightMove( &work->si_s_x, &work->si_s_y, 0.0f, -300.0f+S_MAX_SIZE_Y, S_CIRCLE_SPEED*(DM_FrameSkip+1) );
		
		SetSightMove( &work->si_m_x, &work->si_m_y, work->si_s_x, work->si_s_y, M_CIRCLE_SPEED*(DM_FrameSkip+1) );

		SetSightPrim( work->con_s_circle, work->si_s_x, work->si_s_y, S_MAX_SIZE_X, S_MAX_SIZE_Y, 128, 0 );
		SetSightPrim( work->con_m_circle, work->si_m_x, work->si_m_y, 150.0f+len, 150.0f+len, 128, 1 );
		
		SetTriPrim( work->con_tri, work->si_s_x, work->si_s_y, work->si_m_x, work->si_m_y, 128 );

		x = GetMemYaji_X( &ConMemMem_Pos[0], &ConMemMem_Pos[1],
		                  CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, work->si_s_y );
		SetMemPos( work->con_pos_mem, ConPosMem_Pos, x, work->si_s_y, CONPOSMEM_ROT_Z, CONPOSMEM_ALPHA );
		if( ret ){
			work->si_step++;
		}
		break;
	  case CON_STEP_ACT6_MOVE3:
		ret = SetSightMove( &work->si_s_x, &work->si_s_y, -300.0f+S_MAX_SIZE_X, 300.0f-S_MAX_SIZE_Y, S_CIRCLE_SPEED*(DM_FrameSkip+1) );
		
		SetSightMove( &work->si_m_x, &work->si_m_y, work->si_s_x, work->si_s_y, M_CIRCLE_SPEED*(DM_FrameSkip+1));

		SetSightPrim( work->con_s_circle, work->si_s_x, work->si_s_y, S_MAX_SIZE_X, S_MAX_SIZE_Y, 128, 0 );
		SetSightPrim( work->con_m_circle, work->si_m_x, work->si_m_y, 150.0f+len, 150.0f+len, 128, 1 );

		SetTriPrim( work->con_tri, work->si_s_x, work->si_s_y, work->si_m_x, work->si_m_y, 128 );

		x = GetMemYaji_X( &ConMemMem_Pos[0], &ConMemMem_Pos[1],
		                  CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, work->si_s_y );
		SetMemPos( work->con_pos_mem, ConPosMem_Pos, x, work->si_s_y, CONPOSMEM_ROT_Z, CONPOSMEM_ALPHA );

		if( ret ){
			work->si_step++;
		}
		break;
	  case CON_STEP_ACT6_MOVE4:
		ret = SetSightMove( &work->si_s_x, &work->si_s_y, 0.0f, 36.0f, S_CIRCLE_SPEED*(DM_FrameSkip+1) );
		ret &= SetSightMove( &work->si_m_x, &work->si_m_y, work->si_s_x, work->si_s_y, M_CIRCLE_SPEED*(DM_FrameSkip+1) );

		SetSightPrim( work->con_s_circle, work->si_s_x, work->si_s_y, S_MAX_SIZE_X, S_MAX_SIZE_Y, 128, 0 );
		SetSightPrim( work->con_m_circle, work->si_m_x, work->si_m_y, 150.0f+len, 150.0f+len, 128, 1 );
		
		SetTriPrim( work->con_tri, work->si_s_x, work->si_s_y, work->si_m_x, work->si_m_y, 128 );

		x = GetMemYaji_X( &ConMemMem_Pos[0], &ConMemMem_Pos[1],
		                  CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, work->si_s_y );
		SetMemPos( work->con_pos_mem, ConPosMem_Pos, x, work->si_s_y, CONPOSMEM_ROT_Z, CONPOSMEM_ALPHA );

		if( ret ){
			work->si_step++;
			work->si_timer = 32*5;
		}
		break;
	  case CON_STEP_ACT6_ZOOM1:

		SetSightPrim( work->con_m_circle, work->si_m_x, work->si_m_y,
					  M_DIFF_SIZE*(float)work->si_timer/CLOCK_COUNT/32.0f+M_MIN_SIZE_X,
					  M_DIFF_SIZE*(float)work->si_timer/CLOCK_COUNT/32.0f+M_MIN_SIZE_Y,
					  128, 1 );
		work->si_timer -= CLOCK_COUNT * (DM_FrameSkip+1);
		if( work->si_timer < 0 ){
			work->si_timer = 0;
			work->si_step++;
		}

		break;
	  case CON_STEP_ACT6_ZOOM2:

		SetSightPrim( work->con_m_circle, work->si_m_x, work->si_m_y,
					  M_DIFF_SIZE*(float)(work->si_timer/CLOCK_COUNT)/128.0f+M_MIN_SIZE_X,
					  M_DIFF_SIZE*(float)(work->si_timer/CLOCK_COUNT)/128.0f+M_MIN_SIZE_Y,
					  128, 1 );
		work->si_timer+=4*5 * (DM_FrameSkip+1);
		if( work->si_timer/CLOCK_COUNT > 128 ){
			work->si_timer = 128*5;
			work->si_step++;
		}
		break;
	  default:
		break;
	}
}

static void SetStrUV( DG_PRIM2_UVRGB *uvrgb, DG_TEX *tex, int u, int v, int w, int h  )
{
	int			tw,th,off_u,off_v;
	int			u12[2],v12[2];
	
	DG_GetTexelInfo( NULL, NULL, &off_u, &off_v, tex );

	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	tw = 1 << tw ;
	th = 1 << th ;

	//printf("[%d,%d][%d,%d]\n",u,u+w,v,v+h);
	
	u12[0] = FTOI12( ((float)(off_u + u)+0.5f)/(float)tw );
	v12[0] = FTOI12( ((float)(off_v + v)+0.5f)/(float)th );
	u12[1] = FTOI12( ((float)(off_u + u + w)-0.5f)/(float)tw );
	v12[1] = FTOI12( ((float)(off_v + v + h)-0.5f)/(float)th );
#if 0
	printf("u[%f]<->[%f]\n", tex->u_offset, tex->u_offset+tex->u_scale );
	printf("v[%f]<->[%f]\n", tex->v_offset, tex->v_offset+tex->v_scale );
	printf("u0 %f: u1 %f:\n",
		   ((float)(off_u + u)+0.5f)/(float)tw,
		   ((float)(off_u + u + w)-0.5f)/(float)tw);
	printf("v0 %f: v1 %f:\n",
		   ((float)(off_v + v)+0.5f)/(float)th,
		   ((float)(off_v + v + h)-0.5f)/(float)th);
#endif
	uvrgb[0].u = u12[0];
	uvrgb[0].v = v12[0];
	uvrgb[1].u = u12[1];
	uvrgb[1].v = v12[0];
	uvrgb[2].u = u12[0];
	uvrgb[2].v = v12[1];
	uvrgb[3].u = u12[1];
	uvrgb[3].v = v12[1];
	
}

#define		STR_WIDTH	(96.0f)
#define		STR_HEIGHT	(12.0f)
#define		STR_START_X	(300.0f - STR_WIDTH/2.0f)
#define		STR_START_Y	(300.0f - STR_HEIGHT*6.0f)
#define		STR_ROT_Z	(CONLMEM_ROT_Z)

static void StringMove( Work *work, int alpha )
{
	DG_PRIM2		*prim = work->con_str;
	int				str_num = (irnd()>>16)&0x0f;
	DG_PRIM2_UVRGB	*uvrgb,*pre_uvrgb;
	FVECTOR			*pos,*pre_pos;
	int				clock,i,j;
	FMATRIX			world;
	static FVECTOR	str_pos[] = {
		{  STR_WIDTH/2.0f,  STR_HEIGHT/2.0f, 0.0f, 1.0f },
		{ -STR_WIDTH/2.0f,  STR_HEIGHT/2.0f, 0.0f, 1.0f },
		{  STR_WIDTH/2.0f, -STR_HEIGHT/2.0f, 0.0f, 1.0f },
		{ -STR_WIDTH/2.0f, -STR_HEIGHT/2.0f, 0.0f, 1.0f },
	};
	static FVECTOR center = {STR_START_X,STR_START_Y,0.0f,1.0f };
	FVECTOR add = {0.0f,STR_HEIGHT,0.0f,1.0f };

	_sceVu0RotMatrixZ( &world, &DG_UnitMatrix, STR_ROT_Z );
	DG_COPY_VEC( world.m[3], &center );

	_RotVector( &add, &world, &add );
	
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;
	
	uvrgb = prim->uvrgb[clock];
	pre_uvrgb = prim->uvrgb[1-clock];
	
	pos	= prim->pos[clock];
	pre_pos	= prim->pos[1-clock];

	for( i = 1; i < 6; i++ ){
		for( j = 0; j < 4; j++ ){
			uvrgb[i*4+j].u = pre_uvrgb[(i-1)*4+j].u;
			uvrgb[i*4+j].v = pre_uvrgb[(i-1)*4+j].v;
			if(pre_uvrgb[(i-1)*4+j].a) uvrgb[i*4+j].a = alpha;
//
			
//			pos[i*4+j].vx = pre_pos[(i-1)*4+j].vx;
//			pos[i*4+j].vy = pre_pos[(i-1)*4+j].vy + STR_HEIGHT;

			_sceVu0AddVector(&pos[i*4+j], &pre_pos[(i-1)*4+j],&add);
			//PRINT_PFVEC(i*4+j, &pos[i*4+j]);
		}
	}
//printf("%d\n",str_num);
	SetStrUV( uvrgb, work->str_tex, 0, str_num*4, 32, 4 );
	for( i = 0; i < 4; i++ ){
		//DG_COPY_VEC( pos, &str_pos[i] );
		_RotTrans( pos, &world, &str_pos[i] );
		uvrgb->a = alpha;
		//PRINT_PFVEC(i,pos);
		uvrgb++;
		pos++;
	}
	
}

static FVECTOR	StrBg_Pos[] = {
	{  STR_WIDTH/2.0f + STR_START_X + 16.0f,  STR_HEIGHT/2.0f + STR_START_Y + STR_HEIGHT*8.0f + 16.0f, 0.0f, 1.0f },
	{ -STR_WIDTH/2.0f + STR_START_X - 16.0f,  STR_HEIGHT/2.0f + STR_START_Y + STR_HEIGHT*8.0f + 16.0f, 0.0f, 1.0f },
	{  STR_WIDTH/2.0f + STR_START_X + 16.0f, -STR_HEIGHT/2.0f + STR_START_Y - 24.0f, 0.0f, 1.0f },
	{ -STR_WIDTH/2.0f + STR_START_X - 16.0f, -STR_HEIGHT/2.0f + STR_START_Y - 24.0f, 0.0f, 1.0f },
};

static void StringAct( Work *work )
{
	
	switch( work->str_step ){
	  case CON_STEP_ACT5_WAKE:
		work->str_timer += CLOCK_COUNT * (DM_FrameSkip+1);
		if(work->str_timer/CLOCK_COUNT < 32){
			SetMemPos( work->con_str_bg, StrBg_Pos, 0.0f, 0.0f, 0.0f, (work->str_timer/CLOCK_COUNT) * 8 + 7);
		}else if(work->str_timer/CLOCK_COUNT < 192+32){
			SetMemPos( work->con_str_bg, StrBg_Pos, 0.0f, 0.0f, 0.0f, 255 - (work->str_timer/CLOCK_COUNT - 32) );
		}else{
			work->str_step++;
			SetMemPos( work->con_str_bg, StrBg_Pos, 0.0f, 0.0f, 0.0f, 64 );
		}
		if((work->str_timer/CLOCK_COUNT)&2){
			int alpha = work->str_timer/CLOCK_COUNT;
			if(alpha > 128)alpha = 128;
			StringMove( work, alpha );
		}
		break;
	  case CON_STEP_ACT5_MOVE:
		StringMove( work, 128 );
		//printf("shibashiba\n");
		work->str_step++;
		if(!(irnd() & 0x00070000)){
			work->str_step = CON_STEP_ACT5_WAIT2;
			work->str_timer = (((irnd()>>16)&0x7)*2+2)*5;
		}

		break;
	  case CON_STEP_ACT5_WAIT:
		work->str_timer += CLOCK_COUNT * (DM_FrameSkip+1);
		if(work->str_timer/CLOCK_COUNT > 1){
			work->str_timer = 0;
			work->str_step = CON_STEP_ACT5_MOVE;
		}
		break;
	  case CON_STEP_ACT5_WAIT2:
		work->str_timer -= CLOCK_COUNT * (DM_FrameSkip+1);
		if(work->str_timer == 0){
			work->str_step = CON_STEP_ACT5_MOVE;
		}
		break;
	  default:
		break;
	}

#if 0
	if( 1 && GV_PadDataDirect[0].press&PAD_X){
		work->str_step = 0;
	}
#endif

}
#define		TEN_ALPHA		(64)
#define		TEN_MAX			(TEN_ALPHA*4)
#define		TEN_ADD_Y		(10.0f)
#define		TEN_ADD_Z		(10.0f)
#define		TEN_SIZE		(100.0f)
#define		TEN_POS_X		(300.0f)
#define		TEN_ROT_X		(PI/4.0f)
#define		TEN_ROT_Z		(PI/9.0f)

static FVECTOR TenBottom_Pos[] = {
	{ 300.0f, 700.0f, -400.0f, 1.0f },
	{ -300.0f, 700.0f, -400.0f, 1.0f },
};
static FVECTOR Tension_Verts[] = {
	{ -TEN_SIZE/2.0f, 0.0f,  TEN_SIZE/2.0f, 1.0f },
	{  TEN_SIZE/2.0f, 0.0f,  TEN_SIZE/2.0f, 1.0f },
	{ -TEN_SIZE/2.0f, 0.0f, -TEN_SIZE/2.0f, 1.0f },
	{  TEN_SIZE/2.0f, 0.0f, -TEN_SIZE/2.0f, 1.0f },
};

static void ActTensionPrim( DG_PRIM2 *prim, int ten, float rot_x, float rot_z )
{
	FVECTOR		*pos;
	int			i,j,k,clock,tension,alpha;
	FVECTOR		add;// = { 0.0f, 0.0f, 0.0f, 0.0f };
	FVECTOR		*center = TenBottom_Pos;
	FMATRIX		world;
	DG_PRIM2_UVRGB	*uvrgb;
	
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	uvrgb = prim->uvrgb[clock];
	pos = prim->pos[clock];

	for( i = 0; i < 2; i++ ){
		tension = ten;
		add.vx = 0.0f;
		add.vy = 0.0f;
		add.vz = 0.0f;
		add.vw = 0.0f;
		_sceVu0RotMatrixZ( &world, &DG_UnitMatrix, (i&1)?-rot_z:rot_z );
		_sceVu0RotMatrixX( &world, &DG_UnitMatrix, rot_x );

		for( j = 0; j < 4; j++ ){
			_sceVu0AddVector( (FVECTOR*)world.m[3], &center[i], &add );
#if 1
			alpha = (tension/64>=j)?128:0;
#else
			alpha = tension;
			if(alpha > 64) alpha = 64;
			if(alpha < 0) alpha = 0;
			tension -= 64;
#endif
			for( k = 0; k < 4; k++ ){
				_RotTrans( pos, &world, &Tension_Verts[k] );
				uvrgb->a = alpha;
				//uvrgb->a = (ten/64 < j)?64:ten%64;
				pos++;
				uvrgb++;
			}			
			add.vx += 0.0f;
			add.vy += TEN_ADD_Y;
			add.vz += TEN_ADD_Z;
		}
	}
}

static void TensionAct( Work *work )
{
	switch( work->ten_step ){
	  case CON_STEP_ACT4_WAKE:
		work->tension += 8 * (DM_FrameSkip+1);
		if( work->tension >= TEN_MAX){
			work->tension = TEN_MAX;
			work->ten_step++;
		}
		ActTensionPrim( work->con_tension, work->tension, TEN_ROT_X, TEN_ROT_Z);
		break;
	  case CON_STEP_ACT4_ACT:
		//work->ten_step++;
		break;
	  default:
		work->tension -= 4 * (DM_FrameSkip+1);
		if( work->tension <= 0){
			work->tension = 0;
			work->ten_step = 0;
		}
		ActTensionPrim( work->con_tension, work->tension, TEN_ROT_X, TEN_ROT_Z);
		break;
	}
}

static void ActScrPrim( DG_PRIM2 *prim, int n_verts, int alpha )
{
	int 			clock,i;
	DG_PRIM2_UVRGB	*uvrgb;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < n_verts; i++ ){
		uvrgb->a = alpha;
		uvrgb++;
	}
	
}

static void CommonScrAct( DG_PRIM2 *prim, int n_verts, short *step, u_short *timer )
{
	switch( *step ){
	  case CON_STEP_SCR_WAKE:
		*timer += 10*5 * (DM_FrameSkip+1);
		if(*timer/CLOCK_COUNT >= 240){
			*timer = 240*5;
			(*step)++;
		}
		ActScrPrim( prim, n_verts, (*timer/CLOCK_COUNT) );
		break;
	  case CON_STEP_SCR_ACT1:
		(*timer) -= 2*5 * (DM_FrameSkip+1);
		if(*timer/CLOCK_COUNT <= 128){
			*timer = 128*5;
			(*step)++;
		}
		ActScrPrim( prim, n_verts, *timer/CLOCK_COUNT );
		break;
	  case CON_STEP_SCR_ACT2:
#if 0
		(*timer)++;
		if(*timer/CLOCK_COUNT >= 512){
			*timer = 128;
			(*step)++;
		}
#endif
		break;
	  default:
#if 0
		*timer -= 4;
		if(*timer/CLOCK_COUNT >= 128){
			*timer = 0;
			(*step) = CON_STEP_SCR_WAKE;
		}
		ActScrPrim( prim, n_verts, *timer/CLOCK_COUNT );
#endif
		break;
	}
}

static void SetCanopyAlpha( DG_PRIM2 *prim, int n_verts, VERTS_DATA *pdata, int count, int mode )
{
	//mode	0:add, 1:alpha
	int 			clock,i;
	DG_PRIM2_UVRGB	*uvrgb;
//	float			ratio;
//	FVECTOR			*pos;
	VERTS_DATA		*data = pdata;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;
	uvrgb = prim->uvrgb[clock];
	
	if( mode ){
		for( i = 0; i < n_verts; i++ ){
			if( count < data->count ){
				uvrgb->a = 255;
			}else if( count-FADE_LEN > data->count ){
				uvrgb->a = 0;
			}else{
				uvrgb->a = 255-(int)(255.0f * (float)(count - data->count)/(float)FADE_LEN);
			}

			//if(i==7 && n_verts == 82)printf("count [%d/%d] alpha = %d\n",data->count,count,uvrgb->a);
			uvrgb++;
			data++;
		}
	}else{
		for( i = 0; i < n_verts; i++ ){
			if( count < data->count ){
				uvrgb->a = 0;
			}else if( count-FADE_LEN > data->count ){
				uvrgb->a = 128;
			}else{
				uvrgb->a = (int)(128.0f * (float)(count - data->count)/(float)FADE_LEN);
			}

			uvrgb++;
			data++;
		}
	}
}

static int LineEffectAct( Work *work )
{
	switch(work->can_step){
	  case 0:
		//最初にカバー、マスクキャノピー表示
		DG_VisiblePrim2(work->canopy_msk[0]);
		DG_VisiblePrim2(work->canopy_msk[1]);
		DG_VisibleObjs( work->objs );
		work->can_step++;
		break;
	  case 1:
		//透明、不透明キャノピー系非表示
		DG_InvisiblePrim2(work->canopy_add[0]);
		DG_InvisiblePrim2(work->canopy_add[1]);
		DG_InvisiblePrim2(work->canopy_alp[0]);
		DG_InvisiblePrim2(work->canopy_alp[1]);
		work->can_step++;
		break;
	  case 2:
		SetLineEftPrim( work->line_eft, 3, 6, &work->line_eft_center, work->r_line_eft, 0.0f );
		work->line_eft_center.vz -= 4000.0f/90.0f * (DM_FrameSkip+1);
		work->r_line_eft += 3500.0f/90.0f * (DM_FrameSkip+1);
		work->can_timer += 5 * (DM_FrameSkip+1);
		if(work->can_timer/CLOCK_COUNT > 90){
			work->can_step++;
			work->can_timer = 0;
			//
		}
		break;
	  case 3:
		//透明、不透明キャノピー系表示
		SetLineEftPrim( work->line_eft, 3, 6, &work->line_eft_center, 0.0f, 0.0f );
		DG_VisiblePrim2(work->canopy_add[0]);
		DG_VisiblePrim2(work->canopy_add[1]);
		DG_VisiblePrim2(work->canopy_alp[0]);
		DG_VisiblePrim2(work->canopy_alp[1]);
		work->can_step++;
		break;
	  case 4:
		//最後にカバー、マスクキャノピー非表示
		DG_InvisiblePrim2(work->canopy_msk[0]);
		DG_InvisiblePrim2(work->canopy_msk[1]);
		DG_InvisiblePrim2(work->line_eft);
		DG_InvisibleObjs( work->objs );
		work->can_step = 0;
		return 1;
		break;
	}
	return 0;
}

static int ConsoleEffectAct( Work *work )
{
	//メイン
	CommonScrAct( work->main_con, 4, &work->main_step, &work->main_timer );
	work->console_timer += 5 * (DM_FrameSkip+1);
	if(work->console_timer/CLOCK_COUNT > 8){
		//タッチパネル
		CommonScrAct( work->touch_panel, 8, &work->touch_step, &work->touch_timer );
	}
	if(work->console_timer/CLOCK_COUNT > 16){
		//リアヴュー
		CommonScrAct( work->rear_view, 8, &work->rear_step, &work->rear_timer );
	}
	if(work->console_timer/CLOCK_COUNT > 24){
		//テンション
		TensionAct( work );
	}
	if(work->console_timer/CLOCK_COUNT > 32){
		//文字列
		StringAct( work );
	}
	if(work->console_timer/CLOCK_COUNT > 40){
		//サイト関係
		SightAct( work );
	}
	
	switch(work->can_step){
	  case 0:
		//待ちかなんかで使う取り合えず初期化で
		//printf("r = %f\n",work->r_line_eft);

		work->can_step++;
		work->can_timer = MIN_POS_Z*5;
		break;
	  case 1:
		//外フェードイン
		//アルファ
		SetCanopyAlpha( work->canopy_alp[0], 82, CanopyVerts_Top, work->can_timer/CLOCK_COUNT - BRIGHT_LEN/2, 1 );
		SetCanopyAlpha( work->canopy_alp[1], 40, CanopyVerts_Bottom, work->can_timer/CLOCK_COUNT - BRIGHT_LEN/2, 1 );
		//加算
		SetCanopyAlpha( work->canopy_add[0], 82, CanopyVerts_Top, work->can_timer/CLOCK_COUNT, 0 );
		SetCanopyAlpha( work->canopy_add[1], 40, CanopyVerts_Bottom, work->can_timer/CLOCK_COUNT, 0 );
		work->can_timer += 16*5 * (DM_FrameSkip+1);
		//printf("count = %d\n",work->can_timer);
		if( work->can_timer/CLOCK_COUNT > MAX_FADE_COUNT ){
			work->can_timer = 0;
			work->can_step++;
		}
		break;
	  default:
		break;
	}
	return 0;
}

static void Act(Work *work)
{
	static FVECTOR	center = { 0.0f, 840.0f, -555.0f, 1.0f };
	FMATRIX			world;

	if(CheckMesgParam( work )){
		//自爆
		GV_DestroyActor( work );
		return;
	}
	
	_sceVu0RotMatrixX( &world, &DG_UnitMatrix , ROOT_ROT );
	DG_COPY_VEC( world.m[3], &center );
	_sceVu0MulMatrix( &world, &work->object->objs->objs[0].world, &world );
	DG_COPY_MAT( &work->root, &world );
	if( work->flags & RAYCON_FALGS_ALLON ){
		if( !(work->flags & RAYCON_FALGS_ALLONEND) ){
			work->flags |= RAYCON_FALGS_ALLONEND;
			//すべてオンじゃいー
			//キャノピー関係
			//アルファ
			DG_VisiblePrim2(work->canopy_add[0]);
			DG_VisiblePrim2(work->canopy_add[1]);
			DG_VisiblePrim2(work->canopy_alp[0]);
			DG_VisiblePrim2(work->canopy_alp[1]);
			//最後にカバー、マスクキャノピー非表示
			DG_InvisiblePrim2(work->canopy_msk[0]);
			DG_InvisiblePrim2(work->canopy_msk[1]);
			DG_InvisiblePrim2(work->line_eft);
			DG_InvisibleObjs( work->objs );

			SetCanopyAlpha( work->canopy_alp[0], 82, CanopyVerts_Top, MAX_FADE_COUNT - BRIGHT_LEN/2, 1 );
			SetCanopyAlpha( work->canopy_alp[1], 40, CanopyVerts_Bottom, MAX_FADE_COUNT - BRIGHT_LEN/2, 1 );
			//加算
			SetCanopyAlpha( work->canopy_add[0], 82, CanopyVerts_Top, MAX_FADE_COUNT, 0 );
			SetCanopyAlpha( work->canopy_add[1], 40, CanopyVerts_Bottom, MAX_FADE_COUNT, 0 );

			//コンソール系
			ActScrPrim( work->main_con, 4, 128 );
			ActScrPrim( work->touch_panel, 8, 128 );
			ActScrPrim( work->rear_view, 8, 128 );
			//テンション
			ActTensionPrim( work->con_tension, TEN_MAX, TEN_ROT_X, TEN_ROT_Z);

			ActScrPrim( work->con_r_mem, 4, 128 );
			ActScrPrim( work->con_l_mem, 4, 128 );
			SetMemPos( work->con_pos_mem,
					   ConPosMem_Pos,
					   GetMemYaji_X( &ConMemMem_Pos[0], &ConMemMem_Pos[1],
					                 CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, 0.0f ),
					   36.0f, CONPOSMEM_ROT_Z, 128 );
		
			SetSightPrim( work->con_s_circle, 0.0f, 36.0f, S_MAX_SIZE_X, S_MAX_SIZE_Y, 128, 0 );
			SetSightPrim( work->con_m_circle, 0.0f, 36.0f, 150.0f, 150.0f, 128, 1 );
			SetSightPrim( work->con_l_circle, 0.0f, 36.0f, L_SIZE_X, L_SIZE_Y, 128, 0 );

			work->str_step = CON_STEP_ACT5_MOVE;
			SetMemPos( work->con_str_bg, StrBg_Pos, 0.0f, 0.0f, 0.0f, 64 );
		}
		//文字列ちゃん
		StringAct( work );
		//サイトちゃん
		//SightAct( work );
		//printf("MOMONGA\n");
	}else if( work->flags & RAYCON_FALGS_LINE && LineEffectAct(work)){
		work->flags &= ~RAYCON_FALGS_LINE;
		work->flags |= RAYCON_FALGS_END_LINE;
	}else if(work->flags & RAYCON_FALGS_ALLACT){
		ConsoleEffectAct(work);
	}
	//PRINT_PFVEC(CANOPY_TOP,(FVECTOR*)work->object->objs->objs[CANOPY_TOP].world.m[3]);
	//PRINT_PFVEC(CANOPY_BOTTOM,(FVECTOR*)work->object->objs->objs[CANOPY_BOTTOM].world.m[3]);
}

static void Die(Work *work)
{
	if( work->canopy_add[0] ) GM_FreePrim2( work->canopy_add[0] );
	if( work->canopy_add[1] ) GM_FreePrim2( work->canopy_add[1] );
	if( work->canopy_alp[0] ) GM_FreePrim2( work->canopy_alp[0] );
	if( work->canopy_alp[1] ) GM_FreePrim2( work->canopy_alp[1] );
	if( work->canopy_msk[0] ) GM_FreePrim2( work->canopy_msk[0] );
	if( work->canopy_msk[1] ) GM_FreePrim2( work->canopy_msk[1] );

	if( work->con_l_circle ) GM_FreePrim2( work->con_l_circle );
	if( work->con_m_circle ) GM_FreePrim2( work->con_m_circle );
	if( work->con_s_circle ) GM_FreePrim2( work->con_s_circle );
	if( work->con_tri ) GM_FreePrim2( work->con_tri );
	
	if( work->con_pos_mem ) GM_FreePrim2( work->con_pos_mem );
	if( work->con_r_mem ) GM_FreePrim2( work->con_r_mem );
	if( work->con_l_mem ) GM_FreePrim2( work->con_l_mem );

	if( work->con_str ) GM_FreePrim2( work->con_str );
	if( work->con_str_bg ) GM_FreePrim2( work->con_str_bg );
	
	if( work->con_tension ) GM_FreePrim2( work->con_tension );

	
	if( work->rear_view ) GM_FreePrim2( work->rear_view );
	if( work->touch_panel ) GM_FreePrim2( work->touch_panel );
	if( work->main_con ) GM_FreePrim2( work->main_con );
	
	if( work->line_eft ) GM_FreePrim2( work->line_eft );
	if(work->objs) DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs );
}

#ifdef TS_DEBUG
static CONTROL* SearchControl( int name, int map )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		//if ( ( control->map & map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("ray_console.c: search faild control !!\n");
	return ( NULL );
}
#endif

static DG_PRIM2 *InitPolyAddPrim( int n_prims, int n_verts, int tex_code,
								  int n_polys, int n_pverts, FVECTOR *init_pos, int color )
{
	DG_PRIM2		*prim;	
	DG_TEX 			*tex;
	int				i,j;
	int				u[2],v[2];
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <ray_console.c>\n"); return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
						 n_prims, n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_console.c>\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset);
	u[1] = FTOI12(1.0f * tex->u_scale + tex->u_offset);
	v[1] = FTOI12(1.0f * tex->v_scale + tex->v_offset);
	
	for( i = 0; i < n_polys; i++ ){
		for( j = 0; j < n_pverts; j++ ){
			if( init_pos ){
				DG_COPY_VEC( pos, &init_pos[i*n_pverts+j] );
			}else{
				DG_COPY_VEC( pos, &DG_ZeroVector );
			}
			uvrgb->r = GET_COL_R(color);
			uvrgb->g = GET_COL_G(color);
			uvrgb->b = GET_COL_B(color);
			uvrgb->a = GET_COL_A(color);
			uvrgb->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb->q = 4096;
			uvrgb->u = u[j&1];
			uvrgb->v = v[j>>1];

			pos++;
			uvrgb++;
		}
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );

	return prim;
}

#ifndef BP_PS2
#define	__n_pverts	(6)
#endif

static DG_PRIM2 *InitLineEftPrim( int n_prims, int n_verts, int tex_code, int n_polys, int n_pverts, int color )
{
	DG_PRIM2		*prim;	
	DG_TEX 			*tex;
	int				i,j;
#ifdef BP_PS2
	int				u[n_pverts],v[n_pverts];
#else
	int				u[__n_pverts],v[__n_pverts];
#endif
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;

#ifndef PSX2
	ASSERT( __n_pverts == n_pverts ) ;
#endif


	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <ray_console.c>\n"); return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
						 n_prims, n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_console.c>\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	for(i = 0; i < n_pverts; i++ ){
		float	u_scl,v_scl;
		u_scl = (i&1)?0.0f:1.0f;
		v_scl = 2.0f*(float)i/(float)(n_pverts-1);
		if(v_scl > 1.0f) v_scl = 2.0f - v_scl;
		u[i] = FTOI12(u_scl * tex->u_scale + tex->u_offset);
		v[i] = FTOI12(v_scl * tex->v_scale + tex->v_offset);
	}
	
	for( i = 0; i < n_polys; i++ ){
		for( j = 0; j < n_pverts; j++ ){
			DG_COPY_VEC( pos, &DG_ZeroVector );

			uvrgb->r = GET_COL_R(color);
			uvrgb->g = GET_COL_G(color);
			uvrgb->b = GET_COL_B(color);
			uvrgb->a = GET_COL_A(color);
			uvrgb->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
			uvrgb->q = 4096;
			uvrgb->u = u[j];
			uvrgb->v = v[j];

			pos++;
			uvrgb++;
		}
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );

	return prim;
	
}

static DG_PRIM2 *InitCanopyPrim( int n_prims, int n_verts, int tex_code, VERTS_DATA *vdata, int color )
{
	DG_PRIM2		*prim;	
	DG_TEX 			*tex;
	int				i;
	VERTS_DATA 		*data = vdata;
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;

//	int				max = 0x80000000;
//	int				min = 0x7f000000;
	
	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <ray_console.c>\n"); return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
						 n_prims, n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_console.c>\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_ConfigPrim2Tex( prim, tex );

	for( i = 0; i < n_verts*n_prims; i++ ){
		pos->vx = (float)data->vx;
		pos->vy = (float)data->vy;
		pos->vz = (float)data->vz;
		pos->vw = 1.0f;

		//if( data->vz > max ){
		//	max = data->vz;
		//}
		//if( data->vz < min ){
		//	min = data->vz;
		//}
		uvrgb->r = GET_COL_R(color);
		uvrgb->g = GET_COL_G(color);
		uvrgb->b = GET_COL_B(color);
		uvrgb->a = GET_COL_A(color);
		uvrgb->f = (data->flags)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb->q = 4096;
		uvrgb->u = FTOI12((float)data->u/4096.0f * tex->u_scale + tex->u_offset);
		uvrgb->v = FTOI12((float)data->v/4096.0f * tex->v_scale + tex->v_offset);

		//PRINT_PFVEC(i,pos);
		uvrgb++; pos++; data++;
	}

	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_verts*n_prims );

	//printf("max %d: min %d\n",max,min);
	return prim;
}


static int InitCanopy( Work *work )
{
	DG_PRIM2	*prim;
#if 1
	//raycp_hexa.mdl
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("raycp_cover"), 'k' ) ) ;
	if(!def){
		printf("ERR!! NO MODEL!!\n");
		return -1;
	}
	objs = work->objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
	if(!objs) return -1;
	if(DG_QueueObjs( work->objs )<0) return -1;
	objs->root = &work->object->objs->world;
	DG_InvisibleObjs( objs );
	prim = work->line_eft = InitLineEftPrim( 1, 18, 15350186, 3, 6, 0x80808080 );
	if(!prim) return -1;
	prim->root = &work->object->objs->world;
	prim->raise = -5000;

	//SetLineEftPrim( prim, 3, 6, &(FVECTOR){0.0f,207.0f,1000.0f,1.0f}, 1100.0f, 0.0f );
#endif

	//半透明上
	prim = work->canopy_add[0] = InitCanopyPrim( 2,41,
												 GV_StrCode("raycp_display1_add_alp"),
												 CanopyVerts_Top,
												 SET_COL(180,196,180,0) );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->root = &work->object->objs->objs[CANOPY_TOP].world;
	//prim->raise = -500;
	//半透明下
	prim = work->canopy_add[1] = InitCanopyPrim( 1,40,
												 GV_StrCode("raycp_display1_add_alp"),
												 CanopyVerts_Bottom,
												 SET_COL(180,196,180,0) );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->root = &work->object->objs->objs[CANOPY_BOTTOM].world;
	//prim->raise = -500;


	//不透明上
	prim = work->canopy_alp[0] = InitCanopyPrim( 2,41,
												GV_StrCode("raycp_display1_dammy_alp"),
												CanopyVerts_Top,
												SET_COL(0,0,0,255) );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->root = &work->object->objs->objs[CANOPY_TOP].world;
	prim->raise = -1000;

	//不透明下
	prim = work->canopy_alp[1] = InitCanopyPrim( 1,40,
												GV_StrCode("raycp_display1_dammy_alp"),
												CanopyVerts_Bottom,
												SET_COL(0,0,0,255) );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->root = &work->object->objs->objs[CANOPY_BOTTOM].world;
	prim->raise = -1000;


	
	//マスク上
	prim = work->canopy_msk[0] = InitCanopyPrim( 2,41,
												 GV_StrCode("raycp_canopy_liner_alp"),
												 CanopyVerts_Top,
												 SET_COL(64,64,64,255) );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->root = &work->object->objs->objs[CANOPY_TOP].world;
	prim->raise = -2000;

	//マスク下
	prim = work->canopy_msk[1] = InitCanopyPrim( 1,40,
												 GV_StrCode("raycp_canopy_liner_alp"),
												 CanopyVerts_Bottom,
												 SET_COL(64,64,64,255) );
	if(!prim) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->root = &work->object->objs->objs[CANOPY_BOTTOM].world;
	prim->raise = -2000;


	//DG_InvisiblePrim2(work->canopy_add[0]);
	//DG_InvisiblePrim2(work->canopy_add[1]);
	//DG_InvisiblePrim2(work->canopy_alp[0]);
	//DG_InvisiblePrim2(work->canopy_alp[1]);
	DG_InvisiblePrim2(work->canopy_msk[0]);
	DG_InvisiblePrim2(work->canopy_msk[1]);

	
	return 0;
}

static int GetResources( Work *work )
{
#ifdef TS_DEBUG
	CONTROL	*control;
	
	control = SearchControl( GV_StrCode("メタルギア"), GM_CurrentStageMap );
	if(!control) return -1;

	work->object = control->object;
#endif
	if( !work->object ) return -1;
	if( InitCanopy( work ) ) return -1;

	//サイト関係
	work->con_l_circle = InitPolyAddPrim( 1, 4, 257022, 1, 4, NULL, 0x80808000 );
	if(!work->con_l_circle) return -1;
	work->con_m_circle = InitPolyAddPrim( 1, 4, 14937085, 1, 4, NULL, 0x80808000 );
	if(!work->con_m_circle) return -1;
	work->con_s_circle = InitPolyAddPrim( 1, 4, 15985661, 1, 4, NULL, 0x80808000 );
	if(!work->con_s_circle) return -1;
	work->con_tri = InitPolyAddPrim( 1, 4, 4559435, 1, 4, NULL, 0x80808000 );
	if(!work->con_tri) return -1;
	
	work->con_pos_mem = InitPolyAddPrim( 1, 4, 14493247, 1, 4, NULL, 0x80a86000 );
	if(!work->con_pos_mem) return -1;

	work->con_r_mem = InitPolyAddPrim( 1, 4, 9774895, 1, 4, NULL, 0x80808000 );
	if(!work->con_r_mem) return -1;
	work->con_l_mem = InitPolyAddPrim( 1, 4, 3483439, 1, 4, NULL, 0x80808000 );
	if(!work->con_l_mem) return -1;
	
	SetMemPos( work->con_r_mem, ConRMem_Pos, CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, 0 );
	SetMemPos( work->con_r_mem, ConRMem_Pos, CONRMEM_CENTER_X, CONRMEM_CENTER_Y, CONRMEM_ROT_Z, 0 );
	SetMemPos( work->con_l_mem, ConLMem_Pos, CONLMEM_CENTER_X, CONLMEM_CENTER_Y, CONLMEM_ROT_Z, 0 );
	SetMemPos( work->con_l_mem, ConLMem_Pos, CONLMEM_CENTER_X, CONLMEM_CENTER_Y, CONLMEM_ROT_Z, 0 );

	//文字列関係
	work->con_str = InitPolyAddPrim( 1, 24, 14818372, 6, 4, NULL, 0x80808000 );
	if(!work->con_str) return -1;
	work->con_str_bg = InitPolyAddPrim( 1, 4, 14493247, 1, 4, NULL, 0x80a86000 );
	if(!work->con_str_bg) return -1;
	work->str_tex = DG_GetTexture(14818372);

	DG_InvisiblePrim2(work->con_str_bg);
	
	//テンションバー
	work->con_tension = InitPolyAddPrim( 1, 32, 12641631, 8, 4, NULL, 0x80808000 );
	if(!work->con_tension) return -1;
	work->con_tension->root = &work->object->objs->world;

	//リアヴュー
	work->rear_view = InitPolyAddPrim( 1, 8, 15445123, 2, 4, RearView_Pos, 0x80808000 );
	if(!work->rear_view) return -1;
	work->rear_view->root = &work->object->objs->world;

	//タッチパネル
	work->touch_panel = InitPolyAddPrim( 1, 8, 6045883, 2, 4, TouchPanel_Pos, 0x80808000 );
	if(!work->touch_panel) return -1;
	work->touch_panel->root = &work->object->objs->world;

	//メインコンパネ
	work->main_con = InitPolyAddPrim( 1, 4, 2686569, 1, 4, MainConPane_Pos, 0xa0a0a000 );
	if(!work->main_con) return -1;
	work->main_con->root = &work->object->objs->objs[6].world;


	work->con_l_circle->root = &work->root;
	work->con_m_circle->root = &work->root;
	work->con_s_circle->root = &work->root;
	work->con_tri->root = &work->root;
	work->con_str->root = &work->root;
	work->con_str_bg->root = &work->root;
	work->con_pos_mem->root = &work->root;
	work->con_r_mem->root = &work->root;
	work->con_l_mem->root = &work->root;
	
	//キャノピー関節非表示
	//work->object->objs->objs[CANOPY_TOP].flag |= DG_FLAG_INVISIBLE;
	//work->object->objs->objs[CANOPY_BOTTOM].flag |= DG_FLAG_INVISIBLE;
	//work->object->objs->objs[9].flag |= DG_FLAG_INVISIBLE;

	work->si_m_size = 150.0f;
	work->si_m_x = 0.0f;
	work->si_m_y = 36.0f;
	work->si_s_x = 0.0f;
	work->si_s_y = 36.0f;

	work->can_step = 0;
	work->can_timer = 0;
	work->si_timer = 0;
	work->si_step = 0;
	work->str_timer = 0;
	work->str_step = 0;	
	work->ten_step = 0;
	work->tension = 0;

	work->rear_timer = 0;
	work->rear_step = 0;
	work->touch_timer = 0;
	work->touch_step = 0;
	work->main_timer = 0;
	work->main_step = 0;

	work->console_timer = 0;
	
	work->r_line_eft = 0.0f;
	work->line_eft_center.vx = 0.0f;
	work->line_eft_center.vy = 207.0f;
	work->line_eft_center.vz = 1000.0f;
	work->line_eft_center.vw = 1.0f;
	return 0;
}

void *NewRayConsol( int name, OBJECT *object )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
//	work = (Work*)GV_NewEffect(GV_ACTOR_AFTER2,sizeof(Work));
	if(work){
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );
		
		work->name = name;
		work->object = object;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL;
		}
		//printf("ray_consol起動ですので\n");
	}

	return (void *)work ;
}

void *NewRayConsol_debug( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
//	work = (Work*)GV_NewEffect(GV_ACTOR_AFTER2,sizeof(Work));
	if(work){
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );
		
		work->name = name;
		//work->object = object;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL;
		}
		//printf("ray_consol起動ですので\n");
	}

	return (void *)work ;
}

