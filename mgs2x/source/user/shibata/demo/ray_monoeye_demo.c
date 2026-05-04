//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ray_consol.c
	レイのコンソール

	2000/01/25 T.Shibata

	$Id: ray_monoeye_demo.c,v 1.1.1.3 2002/11/19 11:48:33 Yoshizawa1 Exp $
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
#include "../../mode/demo/eft_con.h"


#define CLOCK_COUNT	(BP_BASE_TICK())

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

//----------ここからコピーして使ってみたりして。

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )


#define GSPRT_MODE_FIXCOR	(0x00100000)	// カラー固定起動時のみ
#define GSPRT_MODE_FOG		(0x00200000)	// フォグが掛かる
#define GSPRT_MODE_TEX		(0x00000000)	// 描画モード　テクスチャーそのまま　起動時のみ
#define GSPRT_MODE_ADD		(0x00010000)	// 描画モード　加算　起動時のみ
#define GSPRT_MODE_SUB		(0x00020000)	// 描画モード　減算　起動時のみ
#define GSPRT_MODE_ALPHA	(0x00030000)	// 描画モード　アルファ　起動時のみ

#define GSPRT_MODE_VSBL		(0x80000001)	// 表示にする
#define GSPRT_MODE_INVSBL	(0x80000002)	// 非表示にする
#define GSPRT_MODE_KILL		(0x80000010)	// 非表示にして殺す


#define		GET_COL_R(_rgba)	(((int)(_rgba)>> 0)&0xff)
#define		GET_COL_G(_rgba)	(((int)(_rgba)>> 8)&0xff)
#define		GET_COL_B(_rgba)	(((int)(_rgba)>>16)&0xff)
#define		GET_COL_A(_rgba)	(((int)(_rgba)>>24)&0xff)

#define		SET_COL(_r,_g,_b,_a) \
				((((int)(_r)&0xff)<<0)|(((int)(_g)&0xff)<<8)|(((int)(_b)&0xff)<<16)|(((int)(_a)&0xff)<<24) )
//#define 	TS_DEBUG

#define MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define MEM_SCR_UV		((void*)(SCRPAD_ADDR+sizeof(FVECTOR)*256))

extern int DM_FrameSkip ;

typedef struct {
	GV_ACT_EX		actor;
	int				name;
	int				flags;
	int				col;
	int				col_timer;


	//EFTCONTROL      control;
	FVECTOR			diff_col;
	FVECTOR			to_col;
	FVECTOR			now_col;

	DG_TEX			*tex;
	FVECTOR			debug_pos[6];

	FVECTOR			center;
	FVECTOR			verts[6];
	float			now_ratio;
	float			to_ratio;
	float			off_set;
	int				noiz_time;
	int				add_noiz;

	//キャノピー関係
	DG_PRIM2		*prim_eye;			//
	DG_PRIM2		*prim_base;			//
	DG_PRIM2		*prim_noiz;		//
	FMATRIX			*root;
	FMATRIX			eye_root;
	FVECTOR			*look;

	//ぼんぼり
	FVECTOR			bon_pos;
	int				bon_mode;
	
} Work;

static FVECTOR RayMonoEyeVerts[] = {
	{ -460.0f, 1041.0f, 2640.0f, 1.0f },
	{ -454.0f,  975.0f, 2564.0f, 1.0f },
	{    0.0f,  746.0f, 3508.0f, 1.0f },
	{    0.0f,  687.0f, 3407.0f, 1.0f },
	{  460.0f, 1041.0f, 2640.0f, 1.0f },
	{  454.0f,  975.0f, 2564.0f, 1.0f },
};

static FVECTOR RayMonoEyeLineVerts[] = {
	{ (-460.0f+-454.0f)*0.5f, (1041.0f+975.0f)*0.5f, (2640.0f+2564.0f)*0.5f, 1.0f },
	{ ( 0.0f + 0.0f )*0.5f,    (746.0f+687.0f)*0.5f, (3508.0f+3407.0f)*0.5f, 1.0f },
	{ ( 460.0f+ 454.0f)*0.5f, (1041.0f+975.0f)*0.5f, (2640.0f+2564.0f)*0.5f, 1.0f },
};


static FVECTOR RayMonoEyesNorms[] = {
	{ -0.841846f, -0.45116f,  0.296226f, 0.0f },
	{  0.00043f,  -0.834603f, 0.550852f, 0.0f },
	{  0.842309f, -0.449133f, 0.297983f, 0.0f },
};

static inline void _Vu0GetCenterVec2( FVECTOR *center, FVECTOR *vec0, FVECTOR *vec1 )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    lqc2		vf5, 0x00(%2)
	qmtc2.ni	%3,  vf3

    vsub.xyzw   vf6, vf5, vf4
	vmulx.xyzw	vf7, vf6, vf3
	vadd.xyzw	vf8, vf4, vf7

    sqc2		vf8, 0x00(%0)
	
	": : "r"(center), "r"(vec0), "r"(vec1), "r"(0.5f) : "$8", "memory" );
#else
	FVECTOR tmp ;
	
	_sceVu0SubVector( &tmp, vec1, vec0 );
	_sceVu0ScaleVector( &tmp, &tmp, 0.5f );
	_sceVu0AddVector( center, vec0, &tmp );
#endif
}

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


static inline float _Vu0VecLenXYZ( FVECTOR *vec )
{
	float	ans;
	
#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    vmul.xyz    vf5, vf4, vf4
    vmulax.w	ACC, vf0, vf5x
    vmadday.w	ACC, vf0, vf5y
    vmaddz.w	vf5, vf0, vf5z
	vsqrt		Q, vf5w
	vwaitq
	vaddq.x		vf6, vf0, Q
    qmfc2.i		$8,vf6
    sw			$8,0(%0)
	": : "r"(&ans), "r"(vec) : "$8", "memory" );
#else
	ans = GV_VecLen3F( vec ) ;
#endif

	return (ans);
}


enum {
	RAYEYE_MESG_COLOR = 0,
};
static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num = GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	//for( i = 0; i < mes_num; i++ ){
	while( --mes_num >= 0 ){
		num = msg->message[0];
		switch( num ){
		  case RAYEYE_MESG_COLOR:
			/*
			printf(" *******************************************\n %d:%d:%d:%d:%d\n",msg->message[1],
				   msg->message[2],
				   msg->message[3],
				   msg->message[4],
				   msg->message[5] );
			*/
			work->col_timer = msg->message[1];
			if( work->col_timer <= 0 ){
				work->col = SET_COL( (msg->message[2] < 0)?GET_COL_R(work->col):msg->message[2],
									 (msg->message[3] < 0)?GET_COL_G(work->col):msg->message[3],
									 (msg->message[4] < 0)?GET_COL_B(work->col):msg->message[4],
									 (msg->message[5] < 0)?GET_COL_A(work->col):msg->message[5] );
				printf("now col change\n");
				return 0;
			}
			work->now_col.vx = (float)GET_COL_R(work->col);
			work->now_col.vy = (float)GET_COL_G(work->col);
			work->now_col.vz = (float)GET_COL_B(work->col);
			work->now_col.vw = (float)GET_COL_A(work->col);

			work->to_col.vx = (msg->message[2] < 0)?(work->now_col.vx):(float)(msg->message[2]);
			work->to_col.vy = (msg->message[3] < 0)?(work->now_col.vy):(float)(msg->message[3]);
			work->to_col.vz = (msg->message[4] < 0)?(work->now_col.vz):(float)(msg->message[4]);
			work->to_col.vw = (msg->message[5] < 0)?(work->now_col.vw):(float)(msg->message[5]);

			work->diff_col.vx = ( work->to_col.vx - work->now_col.vx )/(float)work->col_timer;
			work->diff_col.vy = ( work->to_col.vy - work->now_col.vy )/(float)work->col_timer;
			work->diff_col.vz = ( work->to_col.vz - work->now_col.vz )/(float)work->col_timer;
			work->diff_col.vw = ( work->to_col.vw - work->now_col.vw )/(float)work->col_timer;
			
			//printf("now"); PRINT_PFVEC(0,&work->now_col);
			//printf("to"); PRINT_PFVEC(0,&work->to_col);
			//printf("diff"); PRINT_PFVEC(0,&work->diff_col);
			work->flags |= 1;
			
			break;
		  default:
			break;
		}
		msg--;
	}

	return 0;
}

static void SetEyePrimData( DG_PRIM2* prim,
							float a, float b, float c, float wide,
							FVECTOR *verts, float inner, DG_TEX *tex, int color )
{
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int				clock,i;
	FVECTOR			fvtemp;
	float			fabc[3] = { a, b, c };
	
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	pos = prim->pos[clock];
	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < 3; i++ ){
		float	scale = fabc[i];
		int		index = 0;
		if( scale > 1.0f ){
			scale -= 1.0f;
			index = 2;
		}

		_sceVu0SubVector( &fvtemp, &verts[index+2], &verts[index] );
		_sceVu0ScaleVector( &fvtemp, &fvtemp, scale );
		_sceVu0AddVector( &pos[0], &verts[index], &fvtemp );
		
		_sceVu0SubVector( &fvtemp, &verts[index+2+1], &verts[index+1] );
		_sceVu0ScaleVector( &fvtemp, &fvtemp, scale );
		_sceVu0AddVector( &pos[1], &verts[index+1], &fvtemp );

		uvrgb[0].u = uvrgb[1].u = FTOI12( (0.5f - (fabc[i]-inner)/wide) * tex->u_scale + tex->u_offset);
		//uvrgb[1].u = FTOI12( (0.5f - (fabc[i]-inner)/wide) * tex->u_scale + tex->u_offset);
		uvrgb[0].r = uvrgb[1].r = GET_COL_R(color);
		uvrgb[0].g = uvrgb[1].g = GET_COL_G(color);
		uvrgb[0].b = uvrgb[1].b = GET_COL_B(color);
		uvrgb[0].a = uvrgb[1].a = GET_COL_A(color);

		uvrgb+=2;
		pos+=2;
	}

}
#if 0
static void SetEyePrimData2( DG_PRIM2* prim, int alpha )
{
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int				clock,i;
	
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

//	pos = prim->pos[clock];
	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < 3; i++ ){
		
		uvrgb[0].a = uvrgb[1].a = alpha;
//		uvrgb[0].r = uvrgb[1].r = 128;
//		uvrgb[0].g = uvrgb[1].g = 128;
//		uvrgb[0].b = uvrgb[1].b = 128;

		uvrgb+=2;
		pos+=2;
	}

}
#endif
/*
static float InterpRatio( float now, float to )
{
	return ((to-now)*0.3f+now);
}
*/
static void Act( Work *work )
{
	int i;
	FMATRIX		eye_root,eye_root_inv;//,root_inv;
	FVECTOR		local_look,center;
//	FVECTOR		ans;
//	FVECTOR		p;		//シフト位置
	FVECTOR		d;		//シフト位置から目標への
	float		max_len = 0.0f,ratio;//,inner;
	FVECTOR		diff;
	SVECTOR		rot;


	CheckMesgParam( work );

	if( work->flags & 1 ){
		FVECTOR		diff;

		_sceVu0ScaleVector( &diff, &work->diff_col, (float)(DM_FrameSkip+1) );
		_sceVu0AddVector( &work->now_col, &work->now_col, &diff );
		work->col_timer -= (DM_FrameSkip+1);

		if( work->col_timer <= 0 ){
			work->flags &= ~1;
			DG_COPY_VEC( &work->now_col, &work->to_col );
		}
		
		work->col = SET_COL( work->now_col.vx, work->now_col.vy, work->now_col.vz, work->now_col.vw);
		//printf("col[%08x]\n",work->col);
	}

	_sceVu0SubVector( &diff, &RayMonoEyeLineVerts[1], &RayMonoEyeLineVerts[0] );
	max_len = _Vu0VecLenXYZ( &diff );
#if 0
	for( i = 0; i < 6; i++ ){
		_RotTrans( &work->debug_pos[i], work->root, &RayMonoEyeVerts[i] );
	}
#endif
	_Vu0GetCenterVec2( &center, &RayMonoEyeLineVerts[0], &RayMonoEyeLineVerts[2] );
	//DG_COPY_VEC( (FVECTOR*)work->eye_root.m[3], (FVECTOR*)work->root->m[3] );
	DG_COPY_VEC( (FVECTOR*)work->eye_root.m[3], &center );
	_sceVu0MulMatrix( &eye_root, work->root, &work->eye_root );
	//HZX_ViewMatrix( &eye_root, 10000.0f );

	_sceVu0InversMatrix( &eye_root_inv, &eye_root );
	_RotTrans( &local_look, &eye_root_inv, work->look );
	//_sceVu0SubVector( &d, &local_look, &center );	
	_sceVu0Normalize( &d, &local_look );

#if 1
	if(0){
		//デバッグ用max min 計算
		FMATRIX		eye_root_inv_root;
		SVECTOR		rot;
		FVECTOR		temp;

		_sceVu0MulMatrix( &eye_root_inv_root, &eye_root_inv, work->root );
		PRINT_PFVEC(0, (FVECTOR*)eye_root_inv_root.m[0]);
		PRINT_PFVEC(1, (FVECTOR*)eye_root_inv_root.m[1]);
		PRINT_PFVEC(2, (FVECTOR*)eye_root_inv_root.m[2]);
		PRINT_PFVEC(3, (FVECTOR*)eye_root_inv_root.m[3]);
		for( i = 0; i < 3; i++ ){
			
			_RotTrans( &temp, &eye_root_inv_root, &RayMonoEyeLineVerts[i] );
			_sceVu0Normalize( &temp, &temp );
			TS_VecToRot( &rot, &temp );
			printf( "rot [%d][%d][%f][%f]\n",i,rot.vy,temp.vx,temp.vz);
		}
		//max 1024	min -1023
	}
#endif
	
	TS_VecToRot( &rot, &d );
	//printf( "rot [%d][%d]\n",-1,rot.vy);

	//PRINT_PFVEC(0,&d);
#if 0
	//今回の目の位置を決める
	{
		FVECTOR		a;		//目の単位べく
		FVECTOR		fvtemp;
		float		check_max,check_min;
		
		if( local_look.vx < 0.0f ){
			i = 0;
			check_min = max_len/4.0f;
			check_max = max_len;
		}else{
			i = 1;
			check_min = 0.0f;
			check_max = max_len-max_len/4.0f;
		}

		//必要なものを用意
		DG_COPY_VEC( &p, &RayMonoEyeLineVerts[i] );
		_sceVu0SubVector( &a, &RayMonoEyeLineVerts[i+1], &RayMonoEyeLineVerts[i] );
		_sceVu0Normalize( &a, &a );

		//計算開始
		inner = _sceVu0InnerProduct( &a, &d );
		
		if( inner > max_len ) inner = max_len;
		else if( inner < 0.0f ) inner = 0.0f;
		
		_sceVu0ScaleVector( &fvtemp, &a, inner );
		_sceVu0AddVector( &ans, &fvtemp, &p );

		//デバッグ用
		//PRINT_PFVEC(i,&ans);
		//_RotTrans( &ans, work->root, &ans );
		//AN_Test_Eye( &ans, 2 );
		//PRINT_PFVEC(i,&ans);
	}
	ratio = inner/max_len;
	if( i ){
		ratio = ratio + 1.0f;
	}
	work->to_ratio = ratio;
	work->now_ratio = InterpRatio( work->now_ratio, work->to_ratio );
#else
	if( rot.vy > 1024 ) rot.vy = 1024;
	else if( rot.vy < -1024 ) rot.vy = -1024;
	ratio = (float)rot.vy/1024.0f+1.0f;
	
	//printf( "eye_ratio: %f\n", ratio );
	work->to_ratio = ratio;
	work->now_ratio = ratio;
	//work->now_ratio = InterpRatio( work->now_ratio, work->to_ratio );
	{
		FVECTOR		a;
		FVECTOR		p;
		
		if( ratio > 1.0f ){
			DG_COPY_VEC( &p, &RayMonoEyeLineVerts[1] );
			_sceVu0SubVector( &a, &RayMonoEyeLineVerts[2], &RayMonoEyeLineVerts[1] );
			_sceVu0ScaleVector( &a, &a, ratio-1.0f );
		}else{
			DG_COPY_VEC( &p, &RayMonoEyeLineVerts[0] );
			_sceVu0SubVector( &a, &RayMonoEyeLineVerts[1], &RayMonoEyeLineVerts[0] );
			_sceVu0ScaleVector( &a, &a, ratio );
		}
		_sceVu0AddVector( &a, &a, &p );
		_RotTrans( &work->bon_pos, work->root, &a );
	}
#endif
	
#define		EYE_LEN		(1600.0f)		//半分
	
	{
		float	eye_len_ratio = EYE_LEN/max_len;
		float	temp0,temp1,temp2;

		temp0 = work->now_ratio - eye_len_ratio;
		temp2 = work->now_ratio + eye_len_ratio;
		if( 1.0f-eye_len_ratio < work->now_ratio && work->now_ratio < 1.0f+eye_len_ratio ){
			temp1 = 1.0f;
		}else{
			temp1 = work->now_ratio;
		}
		if( temp0 < 0.0f ) temp0 = 0.0f;
		if( temp2 > 2.0f ) temp2 = 2.0f;
		
//		if( work->name == GV_StrCode("ray1") ){
//			printf( "%d: %f:%f:%f [%d]\n", *work->mode, temp0, temp1, temp2, work->noiz_time );
//		}
		SetEyePrimData( work->prim_base, temp0, temp1, temp2, eye_len_ratio, work->verts, work->now_ratio, work->tex, work->col );
	}
	if(0){
		FVECTOR line[2];
		DG_COPY_VEC( &line[0], work->look );
		DG_COPY_VEC( &line[1], (FVECTOR*)eye_root.m[3] );
		NewLineView( line, 1, 160, 32, 140 );
	}
	//HZX_ViewMatrix( work->root, 10000.0f );
}

static void Die( Work *work )
{
//	if( work->prim_eye ) GM_FreePrim2( work->prim_eye );
	if( work->prim_base ) GM_FreePrim2( work->prim_base );
	if( work->prim_noiz ) GM_FreePrim2( work->prim_noiz );
//    DM_EftControlDelList( &work->control );
}


static DG_PRIM2 *InitPolyAddPrim( int n_prims, int n_verts, int tex_code,
								  int n_polys, int n_pverts, FVECTOR *init_pos, int color )
{
	DG_PRIM2		*prim;	
	DG_TEX 			*tex;
	int				i,j;
	int				u[3],v[2];
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;

	tex = DG_GetTexture(tex_code);
	if(!tex) { printf("ERR!! NO TEX!! <ray_console.c>\n"); return(NULL); }
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
						 n_prims, n_verts );
	if(!prim){ printf("ERR!! MAKE PRIM!! <ray_console.c>\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	u[0] = FTOI12(0.0f * tex->u_scale + tex->u_offset);
	u[1] = FTOI12(0.5f * tex->u_scale + tex->u_offset);
	u[2] = FTOI12(1.0f * tex->u_scale + tex->u_offset);
	v[0] = FTOI12(0.0f * tex->v_scale + tex->v_offset);
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
			uvrgb->u = u[j/2];
			uvrgb->v = v[j&1];

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

static int GetResources( Work *work )
{
	int	i;

#ifdef TS_DEBUG
	CONTROL	*control;
	
	control = SearchControl( GV_StrCode("メタルギア"), GM_CurrentStageMap );
	if(!control) return -1;

	work->root = &control->object->objs->objs[14].world;
	work->look = &GM_PlayerBody->objs->objs[12].world;
#endif
	
	for( i = 0; i < 6; i++ ){
		_sceVu0ScaleVector( &work->verts[i], &RayMonoEyesNorms[i/2], 10.0f );
		_sceVu0AddVector( &work->verts[i], &work->verts[i], &RayMonoEyeVerts[i] );
	}
	
	work->now_ratio = 1.0f;
	//pdray_eye_hl_alp
	work->prim_base = InitPolyAddPrim( 1, 6, 16229417,//6715088,
									   1, 6, NULL, 0x80808080 );
	work->prim_base->root = work->root;
	work->prim_base->raise = 1000;
	DG_SetPrim2Alpha( work->prim_base, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	
	//pdray_eye_noise_alp
	work->prim_noiz = InitPolyAddPrim( 1, 6, 10758245,//6715088,
									   1, 6, work->verts, 0x00808080 );
	work->prim_noiz->root = work->root;
	work->prim_noiz->raise = 0;
	
	work->tex = DG_GetTexture(16229417);//6715088);
	{
		//NewDbugSprite( &work->center, 100 );
	}
	work->add_noiz = irnd()%8+4;
	{
		FVECTOR		fvtemp0,fvtemp1;
		_sceVu0SubVector( &fvtemp0, &RayMonoEyeLineVerts[0], &RayMonoEyeLineVerts[1] );
		_sceVu0SubVector( &fvtemp1, &RayMonoEyeLineVerts[2], &RayMonoEyeLineVerts[1] );

		_sceVu0OuterProduct( (FVECTOR*)work->eye_root.m[1], &fvtemp1, &fvtemp0 );
		DG_COPY_VEC( (FVECTOR*)work->eye_root.m[2], &RayMonoEyesNorms[1] );

		_sceVu0Normalize( (FVECTOR*)work->eye_root.m[1], (FVECTOR*)work->eye_root.m[1] );
		_sceVu0OuterProduct( (FVECTOR*)work->eye_root.m[0], (FVECTOR*)work->eye_root.m[1], (FVECTOR*)work->eye_root.m[2] );
		_sceVu0Normalize( (FVECTOR*)work->eye_root.m[0], (FVECTOR*)work->eye_root.m[0] );
		_sceVu0OuterProduct( (FVECTOR*)work->eye_root.m[2], (FVECTOR*)work->eye_root.m[0], (FVECTOR*)work->eye_root.m[1] );
		_sceVu0Normalize( (FVECTOR*)work->eye_root.m[2], (FVECTOR*)work->eye_root.m[2] );
		//DG_COPY_VEC( (FVECTOR*)work->eye_root.m[3], &RayMonoEyeLineVerts[1] );
		DG_COPY_VEC( (FVECTOR*)work->eye_root.m[3], &DG_ZeroVector );
		//PRINT_PFVEC(0, (FVECTOR*)work->eye_root.m[0]);
		//PRINT_PFVEC(1, (FVECTOR*)work->eye_root.m[1]);
		//PRINT_PFVEC(2, (FVECTOR*)work->eye_root.m[2]);
		//PRINT_PFVEC(3, (FVECTOR*)work->eye_root.m[3]);

	}
	{
		work->bon_mode = GSPRT_MODE_ADD;//
		GV_SetActorChild(work, NewGeneralSprite( 3594043, &work->bon_pos, 100.0f,
												 &work->col, 100, 100, 0, &work->bon_mode ) );
	}
	return 0;
}

void *NewRayMonoEye_Demo( int name, FMATRIX *root, int eftcon, int init_col )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_AFTER,sizeof(Work));
//	work = (Work*)GV_NewEffect(GV_ACTOR_AFTER2,sizeof(Work));
	if(work){
		EFTCONTROL *eft = DM_GetEftControl( eftcon );
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );
		
		work->name = name;
		work->root = root;
		if( !eft ){
			printf("NULL eftcontrol<ray_mono_eye_demo>\n");
			return NULL;
		}
		//DM_EftControlAddList( name, &work->control ) ;
		work->col = init_col;
		work->look = &eft->mov;
		//NewDbugSprite( work->look, 100.0f );
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL;
		}
		//printf("ray_consol起動ですので\n");
	}

	return (void *)work ;
}
