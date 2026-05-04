//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	rain_fix.c
	雨
	2000/02/04 S.Okajima
	$Id: rain_fix.c,v 1.1.1.3 2002/11/19 11:47:10 Yoshizawa1 Exp $

*/

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

#ifndef BP_PSX2_ASM //BP
//#ifdef KP_XBOX
//..VUレジスタの代わり
static FVECTOR _vf0, _vf1, _vf2, _vf3, _vf4, _vf5, _vf6, _vf7,
			_vf8, _vf9, _vf10,_vf11,_vf12,_vf13,_vf14,_vf15,_vf16;
#endif


extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern int	OK_CheckCrossBoundaryOuter( FVECTOR *from, FVECTOR *to, FVECTOR *bound_min, FVECTOR *bound_max, FVECTOR *hit );
extern int	OK_CheckCrossBoundaryInner( FVECTOR *from, FVECTOR *to, FVECTOR *bound_min, FVECTOR *bound_max, FVECTOR *hit );

extern	char *GCL_GetOption( char );
extern	int GCL_GetNextInt( void );

extern	FVECTOR G_wind;
extern	int G_wind_sw;
extern	int OK_rain_se_existance;

extern	FVECTOR	OK_rain_fall_vec;
extern	SVECTOR	OK_rain_fall_rot;




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
#define N_PRIMS2	(LINE_NUM/N_VERTS2 / 2 / 2)


/* シナリオバンダリ */
#define	BOUND_NUMMAX	(8)

/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE		(8000.0f)



#define		MAX_UNIT	(256)

/* ２倍が実際のランダム幅 */
#define	RAIN_RANDOM	(40.0f)

enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_PARAM,
	REQ_NO
};

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

	int		bound_num_max;
	FVECTOR bound_min[BOUND_NUMMAX];
	FVECTOR bound_max[BOUND_NUMMAX];

	FVECTOR *center;
	FVECTOR wind_max;
	float speed_max;
	u_char color;

	int	se_count1;
	int	se_count2;
	int	se_count3;
	int	se_count4;

	int		unit_num;

	FVECTOR before_cam_pos;

	int		rain_se_existance;

	int		invisible_flag;
	float	percentage;

	Unit	unit[0];	/* 可変ワーク */
} Work ;

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
		  case GV_MESSAGE_KILL:
			GV_DestroyActor( work ) ;
			break;
		  case REQ_PARAM:
			work->percentage = (float)(msg->message[1]) * 0.01f;
			if( work->percentage > 1.0f ){
				work->percentage = 1.0f;
			}else if( work->percentage < 0.0f ){
				work->percentage = 0.0f;
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
	FVECTOR	*pos0;
	FVECTOR	*pos1;
	int	clock;
	int	i,j;
	int	flag;
	int	unit;
	int	activate_polys;
	int	dis_act_polys;
	FVECTOR	wind_ratio;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	b_dis;
	FVECTOR	cam_pos;
	float	ftemp;
	FVECTOR	scale;
	static FVECTOR	fall       = { RAIN_RANDOM*3.0f, 250.0f+RAIN_RANDOM*3.0f, RAIN_RANDOM*3.0f, 0.0f };
	static FVECTOR	randam     = { RAIN_RANDOM*2.0f, RAIN_RANDOM*2.0f, RAIN_RANDOM*2.0f, 1.0f }; /* vw:flag */
	static FVECTOR	bound_box = { CENTER_DISTANCE, CENTER_DISTANCE*0.5f, CENTER_DISTANCE, 1.0f };
	//..追加
	int clip_f=0;
   PS2CTX ps2_ctx;
   PS2CTX *pPS2 = &ps2_ctx;

   ps2ctx_init( pPS2 );

	CheckMesgParam( work );

   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }

	if( work->invisible_flag ){
		p_unit = work->unit;
		for(unit=0; unit<work->unit_num; unit++){
			DG_InvisiblePrim2( p_unit->prim );
		}
		return;
	}else{
		p_unit = work->unit;
		for(unit=0; unit<work->unit_num; unit++){
			DG_VisiblePrim2( p_unit->prim );
		}
	}


	ftemp = (float)(N_PRIMS2*N_VERTS2) * work->percentage;
	activate_polys = (int)ftemp;
	dis_act_polys  = N_PRIMS2*N_VERTS2 - activate_polys;


	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );
	b_dis.vx=0.0f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );

	b_dis.vy += CENTER_DISTANCE*0.25f;
	_sceVu0AddVector( &bound_0, &b_dis, &bound_box );
	_sceVu0SubVector( &bound_1, &b_dis, &bound_box );

//----------------------------

	scale.vx = 0.0f;
	scale.vy = RAIN_RANDOM * 8.0f / (float)work->unit_num;
	scale.vz = 0.0f;
	scale.vw = 0.0f;

	clock = 1 - work->unit[0].prim->buffer_clock;

	/* fix data load */
#ifdef BP_PSX2_ASM
	asm volatile ( "
			lqc2 vf5,0(%3)
			lqc2 vf6,0(%4)
			lqc2 vf2,0(%0)
			lqc2 vf3,0(%1)
			lqc2 vf4,0(%2)
			vadd.xyz vf7xyz, vf5xyz, vf5xyz		# 2a
			vsub.xyz vf7xyz, vf7xyz, vf6xyz		# 2a-b
			"::
				"r"(&scale),			/* =vf2 */
				"r"(&fall),				/* =vf3 */
				"r"(&randam),			/* =vf4 */
				"r"(&bound_0),			/* =vf5 */     /* a */
				"r"(&bound_1)			/* =vf6 */     /* b */
										/*  vf7= 2a-b */
	);

#else

	{
		_vf5 = bound_0;
		_vf6 = bound_1;
		_vf2 = scale;
		_vf3 = fall;
		_vf4 = randam;
	}

#endif
	ftemp = 0.1f / (float)work->unit_num;
	p_unit = work->unit;
	for(unit=0; unit<work->unit_num; unit++){
		DG_SwitchBuffPrim2( p_unit->prim );

		UTL_StartMemToSpr( MEM_ADDR1, p_unit->prim->pos[ 1 - clock ], N_PRIMS2*N_VERTS2 );
		UTL_EndMemToSpr();
		UTL_StartMemToSpr( MEM_ADDR2, p_unit->prim->pos[     clock ], N_PRIMS2*N_VERTS2 );
		UTL_EndMemToSpr();

		wind_ratio.vx=(float)unit*ftemp+0.05f;
		wind_ratio.vz=wind_ratio.vy=wind_ratio.vx;

		/* ユニット毎に別のパラメータを与える */
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2		vf10, 0(%0)
			lqc2		vf1,  0(%1)
			lqc2		vf9,  0(%2)
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
		_vf10 = p_unit->wind;
		_vf1  = G_wind;
		_vf9  = wind_ratio;
		// vsub.xz		vf1,  vf1,  vf10
		_vf1.x = _vf1.x - _vf10.x;
		_vf1.z = _vf1.z - _vf10.z;
		// vmul.xz		vf1,  vf1,  vf9
		_vf1.x = _vf1.x * _vf9.x;
		_vf1.z = _vf1.z * _vf9.z;
		// vadd.xz		vf10, vf1,  vf10
		_vf10.x = _vf1.x + _vf10.x;
		_vf10.z = _vf1.z + _vf10.z;
		// vmove.y		vf10, vf0
		_vf10.y = 0;
		// sqc2		vf10, 0(%0)
		p_unit->wind = _vf10;
		// vsub.xyz	vf10, vf10, vf3
		//vf10 = vf10 - vf3;
		sceVu0SubVector(&_vf10, &_vf10, &_vf3);

		// vadd.y		vf3,  vf3,  vf2
		_vf3.y = _vf3.y - _vf2.y;

#endif

		/* 座標更新 */
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

		_vf11 = bound_0;
		_vf12 = bound_1;
		
		//vf13 = vf11 + vf12;
		sceVu0AddVector(&_vf13, &_vf11, &_vf12);
		//vf12 = vf11 - vf12;
		sceVu0SubVector(&_vf12, &_vf11, &_vf12);

		//vf16 = vf13 * 0.5f;
		sceVu0ScaleVector( &_vf16, &_vf13, 0.5);
		//vf12 = vf12 * 0.5f;
		sceVu0ScaleVector( &_vf12, &_vf12, 0.5);

#endif

		pos0=MEM_ADDR1;	/*   前フレーム */
		pos1=MEM_ADDR2;	/* 更新フレーム */
		for ( i = activate_polys ; i > 0  ; i-- ){
			/* クリッピングする */
#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf11,0x00(%0)
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
				"::"r"(pos1),"r"(&flag):"$8","memory"
			);
#else
			if(pos0->x < bound_1.x || bound_0.x < pos0->x) clip_f = 1;
			if(pos0->y < bound_1.y || bound_0.y < pos0->y) clip_f = 1;
			if(pos0->z < bound_1.z || bound_0.z < pos0->z) clip_f = 1;

#endif


#ifdef BP_PSX2_ASM
			/* vi18 は clipping flag */
			if( !(flag & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4))) ){
				/* 位置更新（通常落下） */

				asm volatile ("
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
#else
			if(!clip_f){
			
				_vf9 = *(pos0+1);
				*(pos1) = _vf9;
				
				_vf14.x  = rnd()+1.0f;
				_vf14.y  = rnd()+1.0f;
				_vf14.z  = rnd()+1.0f;
				
				//vf9 = vf9 +vf10;		//#現在の下点を計算
				sceVu0AddVector(&_vf9, &_vf9, &_vf10);
				
				// vmul.xyz	vf14, vf14, vf4
				_vf14.x = _vf14.x * _vf4.x;
				_vf14.y = _vf14.y * _vf4.y;
				_vf14.z = _vf14.z * _vf4.z;
				
				// vadd.xyz	vf9, vf9, vf14
				//vf9 = vf9 + vf14;
				sceVu0AddVector( &_vf9, &_vf9, &_vf14);
				
				*(pos1+1) = _vf9;

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
				{
					_vf9.x = bound_1.x + (bound_0.x - bound_1.x) * rnd() ;
					_vf9.y = bound_1.y + (bound_0.y - bound_1.y) * rnd() ;
					_vf9.z = bound_1.z + (bound_0.z - bound_1.z) * rnd() ;
					_vf9.w = 1.0f ;
					*pos1 = _vf9;
					*(pos1+1) = _vf9;
				}


#endif
			}
			pos0 += 2;
			pos1 += 2;
		}
		for ( i = dis_act_polys ; i > 0  ; i-- ){
#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf9, 16(%0)
				sqc2		vf9,  0(%1)			#1 frame 前の下点を現在の上点にコピー
				sqc2		vf9, 16(%1)			#1 frame 前の下点を現在の下点にコピー
				"::"r"(pos0),"r"(pos1):"memory"
			);
#else
			_vf9 = *(pos0+1);
			*pos1 = _vf9;
			*(pos1+1) = _vf9;

#endif
			pos0 += 2;
			pos1 += 2;
		}




		//----------------------------------

#if 1
		for( j=0; j<BOUND_NUMMAX; j++ ){
			/* ＮＯＴバンダリ */
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
				"::"r"(&work->bound_min[j]),"r"(&work->bound_max[j]),"r"(0.5f):"memory"
			);
#else


#endif

			pos0=MEM_ADDR1;	/*   前フレーム */
			pos1=MEM_ADDR2;	/* 更新フレーム */
//			for ( i = activate_polys ; i > 0  ; i-- ){
			for ( i = N_PRIMS2*N_VERTS2/2 ; i > 0  ; i-- ){
				/* クリッピングする */
#ifdef BP_PSX2_ASM
				asm volatile ("
					lqc2		vf11,0x00(%0)
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
					"::"r"(pos1),"r"(&flag):"$8","memory"
				);
#else

#if 1//BP_ASM - match above asm
            _vf11 = *pos1;                            //lqc2		   vf11,0x00(%0)
            _vf13.vw = _vf12.vx;                      //vmulx.w	   vf13,vf0,vf12
            _vf14.vw = _vf12.vy;                      //vmuly.w	   vf14,vf0,vf12
            BP_Vec3_SubVec( &_vf11, &_vf11, &_vf16 ); //vsub.xyz	   vf11,vf11,vf16
            _vf15.vw = _vf12.vz;                      //vmulz.w	   vf15,vf0,vf12
            vu_clipw_xyz( &_vf11, &_vf13 );           //vclipw.xyz	vf11,vf13
            vu_clipw_xyz( &_vf11, &_vf14 );           //vclipw.xyz	vf11,vf14
            vu_clipw_xyz( &_vf11, &_vf15 );           //vclipw.xyz	vf11,vf15
            flag = pPS2->vu0.flags[0].clipping;               //cfc2	$8,$vi18, sw		$8,0(%1)
#else
            // BP_ASM - this does not match above!
				if(pos0->x < work->bound_min[j].x || work->bound_max[j].x < pos0->x) clip_f = 1;
				if(pos0->y < work->bound_min[j].y || work->bound_max[j].y < pos0->y) clip_f = 1;
				if(pos0->z < work->bound_min[j].z || work->bound_max[j].z < pos0->z) clip_f = 1;
#endif

#endif
				/* vi18 は clipping flag */
				if( !(flag & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4))) ){
					/* 座標同一化で不可視にする */
#ifdef BP_PSX2_ASM
					asm volatile ("
						lqc2		vf9, 16(%0)
						vrnext.x	vf14, R
						vrnext.y	vf14, R
						vrnext.z	vf14, R
						vadd.xyz	vf9, vf9, vf10		#現在の下点を計算
						vmul.xyz	vf14, vf14, vf4
						vadd.xyz	vf9, vf9, vf14
						sqc2		vf9, 16(%1)
						sqc2		vf9,  0(%1)			#下点を上点にコピー
						"::"r"(pos0),"r"(pos1):"memory"
					);
#else
               //BP_ASM
               _vf9 = *pos0;
               _vf14.vx = vu_rnext();
               _vf14.vy = vu_rnext();
               _vf14.vz = vu_rnext();
               BP_Vec3_AddVec( &_vf9, &_vf9, &_vf10 );
               BP_Vec3_MulVec( &_vf14, &_vf14, &_vf4 );
               BP_Vec3_AddVec( &_vf9, &_vf9, &_vf14 );
               BP_Vec4_Copy( &pos1[1], &_vf9 );
               BP_Vec4_Copy( &pos1[0], &_vf9 );
#endif
				}
				pos0 += 2;
				pos1 += 2;
			}
		}
#endif


		UTL_StartSprToMem( p_unit->prim->pos[ clock ], MEM_ADDR2, N_PRIMS2*N_VERTS2 );
		UTL_EndSprToMem();
		p_unit++;
	}


#ifdef BP_PSX2_ASM
	asm volatile ("
		sqc2		vf10, 0(%0)
		"::"r"(&OK_rain_fall_vec)
		);

#endif

	OK_DirVecXY( &DG_ZeroVector, &OK_rain_fall_vec, &OK_rain_fall_rot );

}

static void Die( Work *work )
{
	int	i;

	for(i=0; i<MAX_UNIT; i++){
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
	int	i;
	FVECTOR	fvtemp1;
	FVECTOR	fvtemp2;

	/* バンダリ */
	work->bound_num_max=0;
	if ( GCL_GetOption( 'b' ) != NULL ){
		for( i = 0; GCL_NextStr() != NULL; i++ ){
			fvtemp1.vx = (float)GCL_GetNextInt() ;
			fvtemp1.vy = (float)GCL_GetNextInt() ;
			fvtemp1.vz = (float)GCL_GetNextInt() ;
			fvtemp2.vx = (float)GCL_GetNextInt() ;
			fvtemp2.vy = (float)GCL_GetNextInt() ;
			fvtemp2.vz = (float)GCL_GetNextInt() ;

			work->bound_min[i].vx = (fvtemp1.vx < fvtemp2.vx)?fvtemp1.vx:fvtemp2.vx ;
			work->bound_min[i].vy = (fvtemp1.vy < fvtemp2.vy)?fvtemp1.vy:fvtemp2.vy ;
			work->bound_min[i].vz = (fvtemp1.vz < fvtemp2.vz)?fvtemp1.vz:fvtemp2.vz ;
			work->bound_max[i].vx = (fvtemp1.vx > fvtemp2.vx)?fvtemp1.vx:fvtemp2.vx ;
			work->bound_max[i].vy = (fvtemp1.vy > fvtemp2.vy)?fvtemp1.vy:fvtemp2.vy ;
			work->bound_max[i].vz = (fvtemp1.vz > fvtemp2.vz)?fvtemp1.vz:fvtemp2.vz ;

//printf("a:%d %d %d\n",(int)work->bound_min[i].vx,(int)work->bound_min[i].vy,(int)work->bound_min[i].vz);
//printf("b:%d %d %d\n",(int)work->bound_max[i].vx,(int)work->bound_max[i].vy,(int)work->bound_max[i].vz);
			if(i>=BOUND_NUMMAX-1) break;
		}
		work->bound_num_max=i;
		printf("%d\n",i);
	}else{
		work->bound_num_max=0;
	}


	/* カラー */
	work->color=24;
	if ( GCL_GetOption( 'c' ) != NULL ) {
		work->color = (u_char)GCL_GetNextInt() ;
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

	work->percentage = 1.0f;
	work->invisible_flag = 0;

	if( OK_rain_se_existance == 0 ){
		OK_rain_se_existance = 1;
		work->rain_se_existance = 1;
		/* まき雨 */
		GM_SdSet( SD_A_RAIN_SE1 );
	}else{
		work->rain_se_existance = 0;
	}

//	for(i=0; i<BOUND_NUMMAX; i++){
//		work->bound_flag[i]=0;
//	}

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

	work->unit_num=unit_num;


	/* 表示範囲 */
	bound_0.vx =  CENTER_DISTANCE ;
	bound_0.vy =  CENTER_DISTANCE ;
	bound_0.vz =  CENTER_DISTANCE ;
	bound_1.vx = -CENTER_DISTANCE ;
//	bound_1.vy = -CENTER_DISTANCE ;
	bound_1.vy = 0.0f ;
	bound_1.vz = -CENTER_DISTANCE ;

	b_dis.vx=0.0f;
	b_dis.vy=+CENTER_DISTANCE*0.5f;
	b_dis.vz=CENTER_DISTANCE;

	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );
	_sceVu0AddVector( &bound_0, &b_dis, &bound_0 );
	_sceVu0AddVector( &bound_1, &b_dis, &bound_1 ) ;

	for( unit=0; unit < work->unit_num; unit++ ){
		prim = work->unit[unit].prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}

		prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

		DG_ConfigPrim2Tex( prim, tex );
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

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


	work->se_count1=0;
	work->se_count2=irnd()%30;
	work->se_count3=irnd()%60;
	work->se_count4=irnd()%90;

	return (0);
}


void *NewRainFix( int name, int where )
{
	Work		*work ;
	int			buf_size;
	int			unit_num;

	OPERATOR() ;

	unit_num=0;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		unit_num = GCL_GetNextInt() ;
		if(unit_num > MAX_UNIT){
			printf("MAX_UNIT=%d\n",MAX_UNIT);
			unit_num = MAX_UNIT;
		}
	}

	buf_size = sizeof( Work ) + sizeof( Unit ) * unit_num;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->where = where;

		if ( GetResources( work, unit_num ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
