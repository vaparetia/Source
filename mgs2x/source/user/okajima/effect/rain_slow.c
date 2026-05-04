//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	rain.c
	雨
	1999/07/07 S.Okajima
	$Id: rain_slow.c,v 1.1.1.3 2002/11/19 11:47:11 Yoshizawa1 Exp $

*/

/* asm->C by yano  未検証!! */

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"../etc/ok_util.h"

extern float	OK_slow_param;

extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern int	OK_CheckCrossBoundaryOuter( FVECTOR *from, FVECTOR *to, FVECTOR *bound_min, FVECTOR *bound_max, FVECTOR *hit );
extern int	OK_CheckCrossBoundaryInner( FVECTOR *from, FVECTOR *to, FVECTOR *bound_min, FVECTOR *bound_max, FVECTOR *hit );

extern	char *GCL_GetOption( char );
extern	int GCL_GetNextInt( void );

extern	FVECTOR G_wind;

extern	FVECTOR	OK_rain_fall_vec;
extern	SVECTOR	OK_rain_fall_rot;
extern	int		OK_rain_on_off_flag;

extern	int		OK_rain_se_existance;
extern	float	OK_rain_percentage;

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))


// スクラッチパッド割り当て
//    << Scratch Pad >>
//    V 0x70000000 - 0x70003fff
#define	SCR_LENGTH	( 0x4000 )

// FVECTOR のバイト数で割ってスクラッチパットに入る最大のFVECTOR の数を出し
// speed と wind 用のFVECTOR を引いてやる
#define		LINE_NUM	( SCR_LENGTH/16 )

/* ポリゴン＆ラインの最大頂点数は６４まで！！ */
#define N_VERTS2	(64)
/* 半分づつ使用で、ライン数はさらに半分 */
#define N_PRIMS2	(1)


/* シナリオバンダリ */
#define	BOUND_NUMMAX	(8)

/* バウンドの中心と、視点との距離 */
#define	BOUND_WIDTH			(3000.0f)
#define	CENTER_DISTANCE		(5000.0f)

#define	QUICK_DISTANCE		(2000.0f)

#define		MAX_UNIT	(256)

/* ２倍が実際のランダム幅 */
#define	RAIN_RANDOM	(40.0f)



typedef	struct	{
	FVECTOR		wind;	/* １フレーム前の風 */
	DG_PRIM2	*prim ;
} Unit;

typedef	struct	{
	GV_ACT_EX	actor;
	int		name;
	int		where;

	int		life_max;
	int		life;

	int		cam_in_out_flag;
	int		cam_in_out_flag_old;

	FVECTOR *center;
	FVECTOR wind_max;
	float speed_max;
	int		color;
	int		unit_num;
	int		unit_num_mul;
	FVECTOR before_cam_pos;
	int		rain_se_existance;
	int		stable_count;
	int		invisible_flag;
	int		retry;

	float	before_slow_param;

	Unit	unit[0];	/* 可変ワーク */
} Work ;

enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_PARAM,
	REQ_NO
};

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
			work->invisible_flag = 1;
			break;
		  case REQ_ON:
			work->invisible_flag = 0;
			break;
		  case REQ_KILL:
			GV_DestroyActor( work ) ;
			break;
		  case REQ_PARAM:
			OK_rain_percentage = (float)(msg->message[1]) * 0.01f;
			if( OK_rain_percentage > 1.0f ){
				OK_rain_percentage = 1.0f;
			}else if( OK_rain_percentage < 0.0f ){
				OK_rain_percentage = 0.0f;
			}
			break;
			default:
			  break;
		}
		msg--;
	}
}

static void Act( Work *work )
{
	Unit	*p_unit;
	Unit	*p_unit2;
	FVECTOR	*pos0;
	FVECTOR	*pos1;
	int	clock;
	int	i,j;
	int	unit;
	int	activate_polys;
	int	dis_act_polys;
	FVECTOR	wind_ratio;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	clip_0;
	FVECTOR	clip_1;
	FVECTOR	b_dis;
	FVECTOR	cam_pos;
	FVECTOR	speed;
	float	ftemp;
	FVECTOR	scale;
	static FVECTOR	fall;
	static FVECTOR	randam;
	DG_PRIM2_UVRGB		*uvrgb ;
#ifndef BP_PSX2_ASM
	FVECTOR c_fvtemp7, c_fvtemp10, c_fvtemp3, c_fvtemp13, c_fvtemp12, \
	  c_fvtemp16, c_fvtemp14, c_fvtemp11, c_fvtemp15, c_fvtempA, c_fvtemp19, \
		c_fvtemp9, c_fvtemp18, c_fvtemp17, c_fvtemp8;
#endif

	CheckMesgParam( work );

//printf("OK_slow_param:%f\n",OK_slow_param);

	ftemp = RAIN_RANDOM*OK_slow_param;
	fall.vx   = ftemp*3.0f;
	fall.vy   = ftemp*3.0f + 250.0f;
	fall.vz   = ftemp*3.0f;
	fall.vw   = 0.0f;

	randam.vx = ftemp*2.0f;
	randam.vy = ftemp*2.0f;
	randam.vz = ftemp*2.0f;
	randam.vw = 1.0f;

	ftemp = 1.0f - OK_slow_param;
	speed.vx = ftemp;
	speed.vy = ftemp;
	speed.vz = ftemp;
	speed.vw = 1.0f;

	speed.vw = 1.0f;

	ftemp = (float)(N_PRIMS2*N_VERTS2/2) * OK_rain_percentage;
	activate_polys = (int)ftemp;
	dis_act_polys  = N_PRIMS2*N_VERTS2/2 - activate_polys;

	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );

	_sceVu0SubVector( &scale, &work->before_cam_pos, &cam_pos ) ;
	DG_COPY_VEC( &work->before_cam_pos, &cam_pos );


	ftemp = DG_Chanls->screen / 2.0f;
	if( ftemp < 0.5f ) ftemp = 0.5f;

	clip_0.vx = clip_0.vy = clip_0.vz = BOUND_WIDTH / ftemp;
	clip_1.vx = clip_1.vz =-clip_0.vx;
	clip_1.vy = clip_1.vx*0.5f;

	b_dis.vx=0.0f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE / ftemp;

	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );


	if( GV_VecLen3F( &scale ) > QUICK_DISTANCE ){	/* 急激にカメラ位置が変化したとき */
		work->cam_in_out_flag = work->cam_in_out_flag_old + 1;
		clip_0.vx = clip_0.vy = clip_0.vz = CENTER_DISTANCE;
		clip_1.vx = clip_1.vz =-clip_0.vx;
		clip_1.vy = clip_1.vx*0.5f;


//		return;


	}else{
		clip_0.vx = clip_0.vy = clip_0.vz = CENTER_DISTANCE;
		clip_1.vx = clip_1.vz =-clip_0.vx;
		clip_1.vy = clip_1.vx*0.5f;
		work->cam_in_out_flag=0;
		b_dis.vx=0.0f;
		b_dis.vy=0.0f;
		b_dis.vz=CENTER_DISTANCE;
		DG_PutVector( &b_dis, &b_dis, 1 );
	}

	_sceVu0AddVector( &bound_0, &b_dis, &clip_0 );
	_sceVu0AddVector( &bound_1, &b_dis, &clip_1 );



//----------------------------

	scale.vx = 0.0f;
	scale.vy = RAIN_RANDOM * 8.0f / (float)work->unit_num;
	scale.vz = 0.0f;
	scale.vw = 0.0f;

	clock = AS_WillPrimBuffSwitch() ^ work->unit[0].prim->buffer_clock;

	/* fix data load */
#ifdef BP_PSX2_ASM
	asm volatile ( "
			lqc2 vf5,0(%3)
			lqc2 vf6,0(%4)
			lqc2 vf2,0(%0)
			lqc2 vf3,0(%1)
			lqc2 vf4,0(%2)
			lqc2 vf17,0(%5)
			vadd.xyz vf7xyz, vf5xyz, vf5xyz		# 2a
			vsub.xyz vf7xyz, vf7xyz, vf6xyz		# 2a-b
			"::
				"r"(&scale),			/* =vf2 */
				"r"(&fall),				/* =vf3 */
				"r"(&randam),			/* =vf4 */
				"r"(&bound_0),			/* =vf5 */     /* a */
				"r"(&bound_1),			/* =vf6 */     /* b */
				"r"(&speed)				/* =vf17 */
	);
#else
	_sceVu0AddVector( &c_fvtemp7, &bound_0 , &bound_0 );
	_sceVu0SubVector( &c_fvtemp7, &c_fvtemp7 , &bound_1 );
	_sceVu0CopyVector( &c_fvtemp3, &fall );
#endif

//	ftemp = 0.1f / (float)work->unit_num;
//	ftemp = 2.0f / (float)work->unit_num;
	if( work->stable_count-- < 0 ){
		work->stable_count = irnd()%32 + 16;
	}
	ftemp = 0.1f * (float)work->stable_count / (float)work->unit_num / 48.0f;
	p_unit  = p_unit2  = work->unit;
	p_unit2 += work->unit_num;


	for( unit=0; unit<work->unit_num; unit++ )
   {
      //AR_PARTICLE_FULL
      int const buffSwitch = DG_SwitchBuffPrim2( p_unit->prim );
		DG_SwitchBuffPrim2( p_unit2->prim );

		OK_Mem_Scr( MEM_ADDR1, p_unit->prim->pos[ buffSwitch ^ clock ], sizeof(FVECTOR),  N_PRIMS2*N_VERTS2 );
		OK_Mem_Scr( MEM_ADDR2, p_unit->prim->pos[     clock ], sizeof(FVECTOR),  N_PRIMS2*N_VERTS2 );
		wind_ratio.vx=( (float)(unit)*ftemp+0.15f ) * OK_slow_param;
		wind_ratio.vz=wind_ratio.vy=wind_ratio.vx;

#if 0

   		/* ユニット毎に別のパラメータを与える */
/*
  		asm volatile ("
			lqc2		vf10, 0(%0)
			lqc2		vf1,  0(%1)
			lqc2		vf9,  0(%2)

			vmul.xyz	vf1, vf1, vf17
			vmul.xyz	vf10, vf10, vf17

			vsub.xz		vf1,  vf1,  vf10
			vmul.xz		vf1,  vf1,  vf9
			vadd.xz		vf10, vf1,  vf10
			vmove.y		vf10, vf0
			sqc2		vf10, 0(%0)
			vsub.xyz	vf10, vf10, vf3
			vadd.y		vf3,  vf3,  vf2
			"::"r"(&p_unit->wind),"r"(&G_wind),"r"(&wind_ratio):"memory"
		);
*/
#else
		/* ユニット毎に別のパラメータを与える */
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2		vf10, 0(%0)
			lqc2		vf1,  0(%1)
			lqc2		vf9,  0(%2)

			vmul.xyz	vf1, vf1, vf17

			vsub.xz		vf1,  vf1,  vf10
			vmul.xz		vf1,  vf1,  vf9
			vadd.xz		vf10, vf1,  vf10
			vmove.y		vf10, vf0

			sqc2		vf10, 0(%0)
			vsub.xyz	vf10, vf10, vf3
			vadd.y		vf3,  vf3,  vf2

			"::"r"(&p_unit->wind),"r"(&G_wind),"r"(&wind_ratio):"memory"
		);
#else
		p_unit->wind.vx = ( G_wind.vx*speed.vx - p_unit->wind.vx ) * wind_ratio.vx + p_unit->wind.vx;
		p_unit->wind.vy = 0.0f;
		p_unit->wind.vz = ( G_wind.vz*speed.vz - p_unit->wind.vz ) * wind_ratio.vz + p_unit->wind.vz;
		c_fvtemp10.vx = p_unit->wind.vx - c_fvtemp3.vx;
		c_fvtemp10.vy = p_unit->wind.vy - c_fvtemp3.vy;
		c_fvtemp10.vz = p_unit->wind.vz - c_fvtemp3.vz;
		c_fvtemp3.vy = c_fvtemp3.vy + scale.vy;
#endif //PSX2

#endif

		/* 座標更新 */
		pos0=MEM_ADDR1;	/*   前フレーム */
		pos1=MEM_ADDR2;	/* 更新フレーム */
		if( work->cam_in_out_flag == work->cam_in_out_flag_old ){
			/* クリッピング準備 */
#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf11,0x00(%0)
				lqc2		vf12,0x00(%1)
				vadd.xyz	vf13,vf11,vf12
				vsub.xyz	vf12,vf11,vf12

				qmtc2.ni	%2,vf14
				vmulx.xyz	vf16,vf13,vf14
				vmulx.xyz	vf12,vf12,vf14
				"::"r"(&bound_0),"r"(&bound_1),"r"(0.5f):"memory"
			);
#else
			_sceVu0AddVector( &c_fvtemp13, &bound_0, &bound_1 );
			_sceVu0SubVector( &c_fvtemp12, &bound_0, &bound_1 );
			_sceVu0ScaleVectorXYZ( &c_fvtemp16, &c_fvtemp13, 0.5f );
			_sceVu0ScaleVectorXYZ( &c_fvtemp12, &c_fvtemp12, 0.5f );
#endif

			for ( i = activate_polys ; i > 0  ; i-- ){
				/* クリッピングする */
#ifdef BP_PSX2_ASM
				asm volatile ("
					lqc2	vf11,0x00(%0)
					vmulx.w	vf13,vf0,vf12
					vmuly.w	vf14,vf0,vf12
					vsub.xyz	vf11,vf11,vf16
					vmulz.w	vf15,vf0,vf12
					vclipw.xyz	vf11,vf13
					vclipw.xyz	vf11,vf14
					vclipw.xyz	vf11,vf15
					vnop
					vnop
					vnop
					vnop
					cfc2	$8,$vi18
					sw		$8,0(%1)
					"::"r"(pos0),"r"(&j):"$8","memory"
				);

				/* vi18 は clipping flag */
				if( !(j & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4))) ){

#else
				c_fvtemp13.vw = c_fvtemp12.vx;
				c_fvtemp14.vw = c_fvtemp12.vy;
				_sceVu0SubVector( &c_fvtemp11, pos0, &c_fvtemp16 );
				c_fvtemp15.vw = c_fvtemp12.vz;
				/* clipping */
				if( !(c_fvtemp11.vx >  DG_FABS( c_fvtemp12.vx ) ||
					  c_fvtemp11.vx < -DG_FABS( c_fvtemp12.vx ) ||
					  c_fvtemp11.vy >  DG_FABS( c_fvtemp12.vy ) ||
					  c_fvtemp11.vy < -DG_FABS( c_fvtemp12.vy ) ||
					  c_fvtemp11.vz >  DG_FABS( c_fvtemp12.vz ) ||
					  c_fvtemp11.vz < -DG_FABS( c_fvtemp12.vz )   ) ){
#endif

#if 0
					/* 位置更新（通常落下） */
/*					asm volatile ("
						lqc2		vf9, 16(%0)
						sqc2		vf9,  0(%1)			#1 frame 前の下点を現在の上点にコピー
						vrnext.x	vf14, R
						vrnext.y	vf14, R
						vrnext.z	vf14, R
						vadd.xyz	vf9, vf9, vf10		#現在の下点を計算
						vmul.xyz	vf14, vf14, vf4
						vadd.xyz	vf9, vf9, vf14
						sqc2		vf9, 16(%1)
						"::"r"(pos0),"r"(pos1):"memory"
					);
*/
#else
					/* 位置更新（通常落下） */
#ifdef BP_PSX2_ASM
					asm volatile ("
						lqc2		vf18, 0(%0)
						lqc2		vf9, 16(%0)
						vmove.xyzw	vf19, vf9
						vrnext.x	vf14, R
						vrnext.y	vf14, R
						vrnext.z	vf14, R
						vadd.xyz	vf9, vf9, vf10		#現在の下点を計算
						vmul.xyz	vf14, vf14, vf4
						vadd.xyz	vf9, vf9, vf14

						vsub.xyz	vf18, vf19, vf18
						vmul.xyz	vf18, vf18, vf17

						vsub.xyz	vf19, vf19, vf18
						vsub.xyz	vf9,  vf9,  vf18
						sqc2		vf19,  0(%1)
						sqc2		vf9,  16(%1)
						"::"r"(pos0),"r"(pos1):"memory"
					);
#else
					c_fvtemp14.vx = rnd() + 1.0f;
					c_fvtemp14.vy = rnd() + 1.0f;
					c_fvtemp14.vz = rnd() + 1.0f;
					_sceVu0AddVector( &c_fvtemp9, &pos0[1], &c_fvtemp10 );
					_sceVu0MulVector( &c_fvtemp14, &c_fvtemp14, &randam );
					_sceVu0AddVector( &c_fvtemp9, &c_fvtemp9, &c_fvtemp14 );
					_sceVu0SubVector( &c_fvtemp18, &pos0[1], &pos0[0] );
					_sceVu0MulVector( &c_fvtemp18, &c_fvtemp18, &speed );
					_sceVu0SubVector( &pos1[0], &pos0[1], &c_fvtemp18 );
					_sceVu0SubVector( &pos1[1], &c_fvtemp9 , &c_fvtemp18 );
#endif //PSX2

#endif
				}else{
					/* 初期位置計算 */
#ifdef BP_PSX2_ASM
					asm volatile ("
						vadda.xyz	ACC,  vf0, vf7	#2a-b
						vsub.xyz	vf8,  vf6, vf5	#b-a
						vrnext.x	vf9,  R
						vrnext.y	vf9,  R
						vrnext.y	vf14, R
						vrnext.y	vf9,  R
						vrnext.y	vf14, R
						vrnext.y	vf9,  R
						vrnext.z	vf9,  R
						vrnext.z	vf14, R
						vrnext.z	vf9,  R
						vrnext.z	vf14, R
						vrnext.z	vf9,  R
						vmadd.xyz	vf9,  vf9, vf8		#(2a-b)+(b-a)R
						sqc2		vf9,  0(%0)
						sqc2		vf9,  16(%0)
						"::"r"(pos1):"memory"
					);
#else
					_sceVu0SubVector( &c_fvtemp8, &bound_1, &bound_0 );
					c_fvtemp9.vx = rnd() + 1.0f;
					c_fvtemp9.vy = rnd() + 1.0f;
					c_fvtemp9.vz = rnd() + 1.0f;
					c_fvtemp17.vw = 1.0f;
					_sceVu0MulVector( &c_fvtempA, &c_fvtemp9, &c_fvtemp8 );
					_sceVu0AddVector( &pos1[0], &c_fvtempA, &c_fvtemp7 );
					_sceVu0CopyVector( &pos1[1], &pos1[0] );
#endif
				}

				pos0 += 2;
				pos1 += 2;
			}
		}else{
			for ( i = activate_polys ; i > 0  ; i-- ){
				/* 初期位置計算 */
#ifdef BP_PSX2_ASM
				asm volatile ("
					vadda.xyz	ACC,  vf0, vf7	#2a-b
					vsub.xyz	vf8,  vf6, vf5	#b-a
					vrnext.x	vf9,  R
					vrnext.y	vf9,  R
					vrnext.y	vf14, R
					vrnext.y	vf9,  R
					vrnext.y	vf14, R
					vrnext.y	vf9,  R
					vrnext.z	vf9,  R
					vrnext.z	vf14, R
					vrnext.z	vf9,  R
					vrnext.z	vf14, R
					vrnext.z	vf9,  R
					vmadd.xyz	vf9,  vf9, vf8		#(2a-b)+(b-a)R
					sqc2		vf9,  0(%0)
					sqc2		vf9,  16(%0)
					"::"r"(pos1):"memory"
				);
#else
					_sceVu0SubVector( &c_fvtemp8, &bound_1, &bound_0 );
					c_fvtemp9.vx = rnd() + 1.0f;
					c_fvtemp9.vy = rnd() + 1.0f;
					c_fvtemp9.vz = rnd() + 1.0f;
					c_fvtemp17.vw = 1.0f;
					_sceVu0MulVector( &c_fvtempA, &c_fvtemp9, &c_fvtemp8 );
					_sceVu0AddVector( &pos1[0], &c_fvtempA, &c_fvtemp7 );
					_sceVu0CopyVector( &pos1[1], &pos1[0] );

#endif
				pos0 += 2;
				pos1 += 2;
			}
		}
		/* 残り掃除 */
		for ( i = dis_act_polys ; i > 0  ; i-- ){
#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf9, 16(%0)
				sqc2		vf9,  0(%1)			#1 frame 前の下点を現在の上点にコピー
				sqc2		vf9, 16(%1)			#1 frame 前の下点を現在の下点にコピー
				"::"r"(pos0),"r"(pos1):"memory"
			);
#else
			_sceVu0CopyVector( &pos1[0], &pos0[1] );
			_sceVu0CopyVector( &pos1[1], &pos0[1] );
#endif
			pos0 += 2;
			pos1 += 2;
		}

		OK_Scr_Mem( p_unit->prim->pos[ clock ], MEM_ADDR2, sizeof(FVECTOR), N_PRIMS2*N_VERTS2 );


		pos1=MEM_ADDR2;
		for ( i = activate_polys ; i > 0  ; i-- ){
			pos1->vx += 300.0f;
			pos1->vy += 300.0f;
			pos1++;
			pos1->vx += 300.0f;
			pos1->vy += 400.0f;
			pos1++;
		}

		OK_Scr_Mem( p_unit2->prim->pos[ clock ], MEM_ADDR2, sizeof(FVECTOR), N_PRIMS2*N_VERTS2 );

		p_unit++;
		p_unit2++;
	}


	work->cam_in_out_flag_old = work->cam_in_out_flag;
	OK_rain_on_off_flag = work->cam_in_out_flag;

#ifdef BP_PSX2_ASM
	asm volatile ("
		sqc2		vf10, 0(%0)
		"::"r"(&OK_rain_fall_vec)
		);
#else
		_sceVu0CopyVector( &OK_rain_fall_vec, &c_fvtemp10 );
#endif
	OK_DirVecXY( &DG_ZeroVector, &OK_rain_fall_vec, &OK_rain_fall_rot );




	if( work->before_slow_param != OK_slow_param  ||  work->retry ){
		work->before_slow_param = OK_slow_param;
		if( work->retry==0 ){
			work->retry = 1;
		}else{
			work->retry = 0;
		}

		i = (int)( (162.0f - (float)work->color) * (1.0f - OK_slow_param) ) + work->color;
		if( i>255 ){
			i=255;
		}else if( i<0 ){
			i=0;
		}


		for( unit=0; unit < work->unit_num_mul; unit++ ){
			OK_Mem_Scr( MEM_ADDR2, work->unit[unit].prim->uvrgb[ clock ], sizeof(DG_PRIM2_UVRGB), N_VERTS2 * N_PRIMS2 ) ;
			uvrgb = MEM_ADDR2 ;
			uvrgb++;
			for ( j = 0 ; j < N_PRIMS2 * N_VERTS2 / 2 ; j++ ){
				uvrgb->b = uvrgb->g = uvrgb->r = i;
				uvrgb+=2;
			}
			OK_Scr_Mem( work->unit[unit].prim->uvrgb[ clock ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS2 * N_PRIMS2 ) ;
		}
	}


}

static void Die( Work *work )
{
	int	i;

	for(i=0; i<work->unit_num_mul; i++){
		work->unit[i].prim = OK_FreePrim2( work->unit[i].prim );
	}

	if( work->rain_se_existance == 1 ){
		OK_rain_se_existance    = 0;
		work->rain_se_existance = 0;
	}
}

/* 初期設定値を取得 */
static	void	GetOptionValue( Work *work )
{
	/* カラー */
	work->color=24;
	if ( GCL_GetOption( 'c' ) != NULL ) {
		work->color = GCL_GetNextInt() ;
	}
}

static int GetResources( Work *work, int unit_num )
{
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;
	FVECTOR				*pos;
	DG_PRIM2_UVRGB		*uvrgb ;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	b_dis;
	int		k ;
	int		unit ;

	work->retry = 0;
	work->before_slow_param = OK_slow_param;
	work->invisible_flag = 0;
	work->stable_count   = 0;

	work->unit_num     = unit_num;
	work->unit_num_mul = unit_num * 2;

	if( OK_rain_se_existance == 0 ){
		OK_rain_se_existance = 1;
		work->rain_se_existance = 1;
		/* まき雨 */
		GM_SdSet( SD_A_RAIN_SE1 );
	}else{
		work->rain_se_existance = 0;
	}

	/* テクスチャ取得 */
	tex = DG_GetTexture( 15050428 /*"rain01_msk"*/ );

#if 1
	{
		FVECTOR	ftemp={0.197384f, 0.0f, 0.0f, 0.0f};
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2 vf1,0(%0)
			vrinit	R, vf1x
		"::"r"(&ftemp) );

#endif
	}
#endif

	GetOptionValue( work ) ;



	/* 表示範囲 */
	bound_0.vx =  BOUND_WIDTH ;
	bound_0.vy =  BOUND_WIDTH ;
	bound_0.vz =  BOUND_WIDTH ;
	bound_1.vx = -BOUND_WIDTH ;
	bound_1.vy = -BOUND_WIDTH ;
	bound_1.vz = -BOUND_WIDTH ;

	b_dis.vx=0.0f;
	b_dis.vy=+CENTER_DISTANCE*0.5f;
	b_dis.vz=CENTER_DISTANCE;

	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );
	_sceVu0AddVector( &bound_0, &b_dis, &bound_0 );
	_sceVu0AddVector( &bound_1, &b_dis, &bound_1 ) ;

	for( unit=0; unit < work->unit_num_mul; unit++ ){
		prim = work->unit[unit].prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}

		prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

		DG_ConfigPrim2Tex( prim, tex );
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
//		static inline void DG_SetPrim2Alpha( DG_PRIM2 *prim, u_long64 alpha )

		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

		pos   = MEM_ADDR1 ;
		uvrgb = MEM_ADDR2 ;
		for ( k = 0 ; k < N_PRIMS2 * N_VERTS2 ; k++ ){
			pos[k].vx = bound_0.vx + (bound_1.vx - bound_0.vx) * rnd() ;
			pos[k].vy = bound_0.vy + (bound_1.vy - bound_0.vy) * rnd() ;
			pos[k].vz = bound_0.vz + (bound_1.vz - bound_0.vz) * rnd() ;
			pos[k].vw = 1.0f ;
			/* ＵＶ値は1.3.12の固定小数点なので注意！ */
			if ( k & 1 ){
				uvrgb[k].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb[k].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb[k].f = 0x0fff ;
				uvrgb[k].q = 4096 ;

				uvrgb[k].r = work->color ;
				uvrgb[k].g = work->color ;
				uvrgb[k].b = work->color ;
				uvrgb[k].a = 128 ;

			} else {
				uvrgb[k].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb[k].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb[k].f = 0x8fff ;
				uvrgb[k].q = 4096 ;

				uvrgb[k].r = 0 ;
				uvrgb[k].g = 0 ;
				uvrgb[k].b = 0 ;
				uvrgb[k].a = 128 ;
			}
		}

		OK_Scr_Mem( prim->pos[ 0 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( prim->pos[ 1 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS2 * N_PRIMS2 ) ;
	}

	work->cam_in_out_flag     = -1;
	work->cam_in_out_flag_old = -2;

	return (0);
}


void *NewRainSlow( int name, int where )
{
	Work		*work ;
	int			buf_size;
	int			unit_num;

	OPERATOR() ;

	unit_num=0;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		unit_num = GCL_GetNextInt();
		if(unit_num > MAX_UNIT){
//			printf("MAX_UNIT=%d\n",MAX_UNIT);
			unit_num = MAX_UNIT;
		}
	}

	buf_size = sizeof( Work ) + sizeof( Unit ) * unit_num        * 2;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name;
		work->where = where;

		if ( GetResources( work, unit_num ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
