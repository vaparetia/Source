//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_strip_demo.c
	落下血：デモバージョン：ストリップ

	2001/06/15 S.Okajima
	$Id: blood_strip_demo.c,v 1.1.1.3 2002/11/19 11:47:28 Yoshizawa1 Exp $
*/
//asm

#ifdef PSX2	///
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"
#include	"../effect/blood.h"

#define	COL_R	(12)
#define	COL_G	(0)
#define	COL_B	(0)

#ifdef ENGLISH
#define	COL_A	(128)
#else
#define	COL_A	(BLOOD_ALPHA_1)
#endif

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_VEC		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(16)
#define N_PRIMS		(0x1000 / 16 / N_VERTS)


#define	TOTAL_VERTS	(N_PRIMS*N_VERTS)
#define N_MUL		(8)
#define CYCLE_MAX	(TOTAL_VERTS/N_MUL)


#define	N_VERTS_STRIP	(64)
#define	N_PRIMS_STRIP	(N_PRIMS/2)
#define	N_STRIP			(N_PRIMS_STRIP*N_VERTS_STRIP/2-1)


#define SIZE_MIN	(10)
#define SIZE_RND	(10)

/*----------------------------------------------------------------*/
extern int DM_FrameSkip ;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;
	int			put_flag;
	int			cnt_count;
	int			cnt_count_max;

	int			cycle;
	float		intense;

	SVECTOR		rot;
	SVECTOR		stp;

	FMATRIX		*world;
	FMATRIX		before_world;

	FVECTOR		vec[TOTAL_VERTS];
	FVECTOR		pos[TOTAL_VERTS];

	DG_PRIM2	*prim_strip;
} Work ;

static ALIGN16_PRE float ALIGN16_POST OK_s_sin[N_MUL];

/* ---------------------------------------------------------------- */
enum {
	REQ_PUT=0,
	REQ_CONT,
	REQ_KILL,
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
		  case REQ_PUT:
			work->put_flag = 1;
			work->cycle = 0;
			break;
		  case REQ_CONT:
			work->cnt_count     = 0;
//			work->cnt_count_max = msg->message[1];
			work->cnt_count_max = DIRECT_TICK( msg->message[1] );
			break;
		  case REQ_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			  break;
		}
		msg--;
	}
}


static inline void SetConst( void )
{
#ifdef BP_PSX2_ASM	///
	FVECTOR	fvtemp = {0.5f, 0.5f, 0.5f, 1.0f};
	asm volatile("
		lqc2		vf8,0x00(%0)
		": : "r"(&fvtemp) :"memory" );
#endif
}

#ifdef BP_PSX2_ASM	///
static inline void InterPos( FVECTOR *pos0, FVECTOR *pos1, FVECTOR *pos2 )
{
	asm volatile("
		lqc2		vf10,0x00(%0)
		lqc2		vf11,0x00(%1)
		lqc2		vf12,0x00(%2)
		vadd.xyz	vf11, vf11, vf12	
		vmul.xyz	vf11, vf11, vf8
		vadd.xyz	vf10, vf10, vf11
		vmul.xyz	vf10, vf10, vf8
		sqc2		vf10,0x00(%0)
		": : "r"(pos0),"r"(pos1),"r"(pos2) :"memory" );
}
#else

static inline void InterPos( FVECTOR *pos0, FVECTOR *pos1, FVECTOR *pos2 )
{
	FVECTOR	pos10,pos11,pos12;
	FVECTOR	fvtemp = {0.5f, 0.5f, 0.5f, 1.0f};
	pos10 = *pos0 ;						//lqc2		vf10,0x00(%0)
	pos11 = *pos1 ;						//lqc2		vf11,0x00(%1)
	pos12 = *pos2 ;						//lqc2		vf12,0x00(%2)
	_sceVu0AddVector(&pos11,&pos11,&pos12) ;	//vadd.xyz	vf11, vf11, vf12
	_sceVu0MulVector(&pos11,&pos11,&fvtemp) ;	//vmul.xyz	vf11, vf11, vf8
	_sceVu0AddVector(&pos10,&pos10,&pos11) ;	//vadd.xyz	vf10, vf10, vf11
	_sceVu0MulVector(&pos10,&pos10,&fvtemp) ;	//vmul.xyz	vf10, vf10, vf8
	*pos0 = pos11 ;						//sqc2		vf10,0x00(%0)
}
#endif

/*----------------------------------------------------------------*/
static void Act( Work *work )
{
	FMATRIX	fmat;
	FMATRIX	fmat1;
	FVECTOR	*sc_pos;
	FVECTOR	*sc_pos1;
	FVECTOR	*sc_pos2;
	FVECTOR	*sc_vec;
	FVECTOR	center;
	FVECTOR	n_center;
	FVECTOR	diff;
	FVECTOR	force;
	int	clock;
	int	num;
	int	i,j;
	float	*s_sin;
	DG_PRIM2	*prim ;
	FVECTOR		fvtemp;
	FVECTOR		cam;
	SVECTOR		rot;


	CheckMesgParam( work );


	force.vx = 0.0f;
	force.vy = 0.0f;
	force.vz = work->intense / (float)N_MUL;
	DG_SetPos( work->world );
	DG_RotVector( &force, &force, 1 );



	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );

	if( work->cnt_count_max > 0 ){
		work->cnt_count -= DM_FrameSkip ;
		work->cnt_count--;
		if( work->cnt_count < 0){
			work->cnt_count = work->cnt_count_max;
			work->put_flag = 1;
			work->cycle = 0;
		}
	}


	DG_COPY_VEC( &center, (FVECTOR *)work->before_world.m[3] );
	DG_COPY_VEC( &n_center, (FVECTOR *)work->world->m[3] );
	_sceVu0SubVector( &diff, &n_center, &center );
	_sceVu0ScaleVector( &diff, &diff, 1.0f / (float)N_MUL );

	OK_Mem_Scr( SCR_POS, work->pos, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
	OK_Mem_Scr( SCR_VEC, work->vec, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;


	num = work->cycle*N_MUL;

//	if( work->put_flag ){
//	if( GV_Time%300<150 ){
	if(1){
		sc_pos  = SCR_POS;
		sc_vec  = SCR_VEC;
		sc_pos += num;
		sc_vec += num;
		s_sin = OK_s_sin;
		rot.vz = 0;
		for ( i = 0 ; i < N_MUL  ; i++ ){
			DG_COPY_VEC( sc_pos, &center );
			_sceVu0AddVector( &center, &center, &diff );

			rot.vx = ((irnd()>>8)&2047)-1024;
			rot.vy = (irnd()>>8)&4095;

			DG_SetPos2( &DG_ZeroVector, &rot );
			DG_RotVector( &diff, sc_vec, 1 );
			_sceVu0ScaleVector( sc_vec, sc_vec, (*s_sin++) );
			_sceVu0AddVector( sc_vec, sc_vec, &diff );
			_sceVu0AddVector( sc_vec, sc_vec, &force );

			sc_pos++;
			sc_vec++;
		}
	}else{
		sc_pos  = SCR_POS;
		sc_vec  = SCR_VEC;
		sc_pos += num;
		sc_vec += num;
		if(num!=0){
			sc_pos1  = SCR_POS;
			sc_pos2  = SCR_VEC;
			sc_pos1 += num-1;
			sc_pos2 += num-1;
		}else{
			sc_pos1 = SCR_POS;
			sc_pos2 = SCR_VEC;
			sc_pos1+= CYCLE_MAX*N_MUL-1;
			sc_pos2+= CYCLE_MAX*N_MUL-1;
		}
		s_sin = OK_s_sin;
		rot.vz = 0;
		for ( i = 0 ; i < N_MUL  ; i++ ){
			DG_COPY_VEC( sc_pos, sc_pos1 );
			DG_COPY_VEC( sc_vec, sc_pos2 );
			sc_pos++;
			sc_vec++;
		}
	}


#if 0
	sc_vec  = SCR_VEC;
	sc_pos  = SCR_POS;
	sc_pos1 = SCR_POS;
	sc_pos1++;
	sc_pos2 = SCR_UVR;
	for ( i = 0 ; i < N_VERTS*N_PRIMS; i++ ){
		sc_vec->vy+= P_GRAVITY;
		sc_pos->vx+= sc_vec->vx;
		sc_pos->vy+= sc_vec->vy;
		sc_pos->vz+= sc_vec->vz;

		_sceVu0SubVector( sc_pos2, sc_pos1, sc_pos );	// 後で透視変換する

		sc_pos++;
		sc_pos1++;
		sc_pos2++;
		sc_vec++;
	}
#else
	sc_vec  = SCR_VEC;
	sc_pos  = SCR_POS;
	for ( i = 0 ; i < N_VERTS*N_PRIMS; i++ ){
		sc_vec->vy+= P_GRAVITY;
		sc_pos->vx+= sc_vec->vx;
		sc_pos->vy+= sc_vec->vy;
		sc_pos->vz+= sc_vec->vz;
		sc_pos++;
		sc_vec++;
	}
#endif



	SetConst();
	if( work->cycle < CYCLE_MAX-1 ){
		sc_pos  = SCR_POS;
		sc_pos += num+N_MUL;
		sc_pos1 = sc_pos;
		sc_pos2 = sc_pos;
		sc_pos += 1;
		sc_pos2+= 2;
		for ( i=num+N_MUL; i<N_VERTS*N_PRIMS-2; i++ ){
			InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
		}
		sc_pos2 = SCR_POS;
		InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
		sc_pos  = SCR_POS;
		InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
		sc_pos1 = SCR_POS;
		InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
	}else{
		sc_pos  = SCR_POS;
		sc_pos1 = sc_pos;
		sc_pos2 = sc_pos;
		sc_pos += 1;
		sc_pos2+= 2;
	}
	for ( i=0; i<num+N_MUL-3; i++ ){
		InterPos( sc_pos++, sc_pos1++, sc_pos2++ );
	}

	OK_Scr_Mem( work->pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( work->vec, SCR_VEC, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;


	work->cycle++;
	if( work->cycle >= CYCLE_MAX ) work->cycle = 0;

	work->put_flag = 0;
	DG_COPY_MAT( &work->before_world, work->world );


	//-----------------------------------------------------STRIP
	prim = work->prim_strip;
	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	DG_COPY_MAT( &fmat, &DG_Chanls->eye_inv );
	DG_COPY_VEC( (FVECTOR *)fmat.m[3], &DG_ZeroVector );	// カメラ前に変換

	rot.vx = 0;
	rot.vy = 0;
	rot.vz = 1024;
	DG_SetPos2( &DG_ZeroVector, &rot );
	DG_GetPos( &fmat1 );
	_sceVu0MulMatrix( &fmat, &fmat1, &fmat );	// ９０度回転

	DG_COPY_MAT( &fmat1, &DG_Chanls->eye );
	DG_COPY_VEC( (FVECTOR *)fmat1.m[3], &DG_ZeroVector );
	_sceVu0MulMatrix( &fmat, &fmat1, &fmat );	// ワールドに戻す


	sc_pos  = SCR_POS;
	sc_pos1 = SCR_POS;
	sc_pos1+= 1;
	sc_pos2 = SCR_UVR;
	for ( i = 0 ; i < N_PRIMS*N_VERTS/(N_VERTS_STRIP/2); i++ ){
		for ( j = 0 ; j < N_VERTS_STRIP/2-1 ; j++ ){
			_sceVu0SubVector( sc_pos2++, sc_pos1++, sc_pos++ );	// 差のリストを生成
		}
		// ストリップが切れてしまうので引っ付ける
		sc_pos ++;
		sc_pos1++;
		_sceVu0SubVector( sc_pos2, sc_pos1, sc_pos );
		sc_pos --;
		sc_pos1--;
		DG_COPY_VEC( sc_pos, sc_pos1 );
		sc_pos ++;
		sc_pos1++;
		sc_pos2++;
	}

	sc_pos = SCR_UVR;
	DG_SetPos( &fmat );
	DG_RotVector( sc_pos, sc_pos, N_VERTS*N_PRIMS );	// 点の間の距離を「カメラから見て幅を持つように」変換



	sc_pos  = SCR_UVR;
	sc_pos1 = SCR_UVR;
	sc_pos1+=1;
	DG_COPY_VEC( &fvtemp, sc_pos1 );
	for ( i = 1 ; i < N_VERTS*N_PRIMS-1; i++ ){
		if( i!=num+N_MUL-1 && i!=num+N_MUL){
			_sceVu0AddVector( sc_pos, sc_pos, sc_pos1 ) ;
			_sceVu0ScaleVector( sc_pos, sc_pos, 0.5f );
		}
		sc_pos ++;
		sc_pos1++;
	}


	// 境界処理
	if( work->cycle < CYCLE_MAX-1 ){
		sc_pos  = SCR_POS;
		sc_pos1 = SCR_POS;
		sc_pos2 = SCR_UVR;
		sc_pos += num+N_MUL;
		sc_pos1+= num+N_MUL+1;
		sc_pos2+= num+N_MUL;
	}
	DG_COPY_VEC( sc_pos, sc_pos1 );
	DG_COPY_VEC( sc_pos2, &DG_ZeroVector );
	sc_pos  = SCR_POS;
	sc_pos1 = SCR_POS;
	sc_pos2 = SCR_UVR;
	sc_pos += num+N_MUL-1;
	sc_pos1+= num+N_MUL-2;
	sc_pos2+= num+N_MUL-1;
	DG_COPY_VEC( sc_pos, sc_pos1 );
	DG_COPY_VEC( sc_pos2, &DG_ZeroVector );

	if( GV_Time%300<150 ){
		sc_pos2 = SCR_UVR;
		sc_pos2+= num;
		for ( i = 0 ; i < N_MUL  ; i++ ){
			DG_COPY_VEC( sc_pos2++, &DG_ZeroVector );
		}
	}


	sc_pos = SCR_POS;
	sc_pos1= SCR_UVR;
	sc_pos2= prim->pos[clock];
	for ( i = 0 ; i < N_VERTS*N_PRIMS-1; i++ ){
//		DG_COPY_VEC( sc_pos2, sc_pos );		sc_pos2++;
//		_sceVu0Normalize( sc_pos1, sc_pos1 );
		_sceVu0ScaleVector( sc_pos1, sc_pos1, 0.5f );
		_sceVu0SubVector( sc_pos2, sc_pos, sc_pos1 );
//if(GV_Time%300==0) printf("%3d:%f %f %f\n",i,sc_pos1->vx,sc_pos1->vy,sc_pos1->vz);
		sc_pos2++;
		_sceVu0AddVector( sc_pos2, sc_pos, sc_pos1 );
//if(GV_Time%300==0) printf("%3d:%f %f %f\n",i,sc_pos2->vx,sc_pos2->vy,sc_pos2->vz);
		sc_pos2++;
		sc_pos++;
		sc_pos1++;
	}
	sc_pos = prim->pos[clock];
	DG_COPY_VEC( sc_pos2++, sc_pos++ );
	DG_COPY_VEC( sc_pos2++, sc_pos++ );


}

/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	work->prim_strip = OK_FreePrim2( work->prim_strip );
}

/*----------------------------------------------------------------*/
static void InitPacketStrip( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			fvtemp;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		j, k, l ;
	int		count ;


	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	DG_COPY_VEC( &fvtemp, (FVECTOR *)work->world->m[3] );

	count = 0;
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	for ( k = 0 ; k < N_PRIMS_STRIP ; k++ ){
		for ( j = 0 ; j < N_VERTS_STRIP ; j++ ){
			if( j < N_VERTS_STRIP-2 ){
				uvrgb1->u = uvrgb0->u = FTOI12((float)(count/2) * 2.0f/(float)(N_PRIMS_STRIP*N_VERTS_STRIP-2) * tex->u_scale + tex->u_offset );
			}else{
				l = count + 2; 	// ６４頂点で切れてしまうのでケツをのばす
				uvrgb1->u = uvrgb0->u = FTOI12((float)(l/2) * 2.0f/(float)(N_PRIMS_STRIP*N_VERTS_STRIP-2) * tex->u_scale + tex->u_offset );
			}
			uvrgb1->v = uvrgb0->v = (count&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = (count<2)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = COL_R ;
			uvrgb1->g = uvrgb0->g = COL_G ;
			uvrgb1->b = uvrgb0->b = COL_B ;
			uvrgb1->a = uvrgb0->a = COL_A;
			uvrgb0++;
			uvrgb1++;
			count++;
		}
	}




}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			i;

	for( i=0; i<N_MUL; i++ ){
		OK_s_sin[i] = sinf( PI*(float)(i)/(float)(N_MUL) );
	}

	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
	prim = work->prim_strip = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_STRIP, N_VERTS_STRIP );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacketStrip( work, prim, tex );
//	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	work->put_flag = 0;
	work->cnt_count_max = 0;
	return 0 ;
}


void *NewBloodStripDemo( int name, FMATRIX *world, float intense )
{
	Work		*work ;

	OPERATOR() ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->world = world;
		DG_COPY_MAT( &work->before_world, world );
		work->intense = intense;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
