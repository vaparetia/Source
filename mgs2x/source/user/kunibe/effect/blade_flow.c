//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    blade_flow.c
    デモ用剣軌跡
    2001/04/04 Yuuta Kunibe
    $Id: blade_flow.c,v 1.1.1.3 2002/11/19 11:44:33 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
#include	"../../shibata/util/ts_util.h"


#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define		GET_COL_R(_rgba)	(((_rgba)>>24)&0xff)
#define		GET_COL_G(_rgba)	(((_rgba)>>16)&0xff)
#define		GET_COL_B(_rgba)	(((_rgba)>>8)&0xff)
#define		GET_COL_A(_rgba)	((_rgba)&0xff)

#define CLOCK_COUNT	(BP_BASE_TICK())

#define N_INTER		(16)
#define N_VERTS		(2*N_INTER + 2)
#define MAX_DISP	(512/N_VERTS)

#define MEM_SCR_POS		((void*)(SCRPAD_ADDR))
#define MEM_SCR_UV		((void*)((char*)MEM_SCR_POS + sizeof(FVECTOR)*512))

#define TEST_ALPHA		(128.0f)

// yano
#ifdef BP_PSX2_ASM
#define USE_VU0
#endif

/*{
	ライデン愛国ブレード
	天狗兵刀
	共和刀
	民主刀
	オルガナイフ
	ニンジャ刀	
}*/
	 

/* ライデン刀オフセット値 */
static	FVECTOR	HFBlade_Shift[] = {
	{ 10.0F, -118.0F, 75.0F },
	{ 13.454841F, -501.865540F, 834.200134F },
};

/* 天狗兵刀オフセット値 */
static	FVECTOR	TNGBlade_Shift[] = {
	{ 12.0F, -100.0F, 80.0F },
	{ -38.0F, -278.0F, 690.0F },
};

/* ソリダス共和刀オフセット値 */
static	FVECTOR	KWT_Shift[] = {
	{ 18.0F, -135.0F, 87.0F },
	{ 18.0F, -320.0F, 800.0F },
};

/* ソリダス民主刀オフセット値 */
static	FVECTOR	MST_Shift[] = {
	{ -18.0F, -135.0F, 87.0F },
	{ -18.0F, -285.0F, 607.0F },
};

/* オルガナイフオフセット値 */
static	FVECTOR ORGKnife_Shift[] = {
	{ 18.0F, -102.0F, 60.0F },
	{ 18.0F, -102.0F, 253.0F },
};

/* オルガ忍者刀オフセット値 */
static	FVECTOR NJBlade_Shift[] = {
	{ 10.0F, -137.0F, 66.0F },
	{ 10.0F, -474.0F, 852.0F },
};
     
static	FVECTOR	*Shifts[] = {
    HFBlade_Shift,
    TNGBlade_Shift,

    KWT_Shift, 
    MST_Shift,

	ORGKnife_Shift,
	NJBlade_Shift,
};     

static int BladeTexCode[] = {
    8819182,
    8819182,

    7338993,
    7338993,

    7338993,
    8819182,
};

static int BladeColor[] = {
	0x040a0e00,
	0x08141a00,

	0x2060c400,
	0x2060c400,

	0xadb6dd00,
	0x040a0e00,
};


typedef struct {

	GV_ACT_EX		actor;
	int				name;
	int				code[3];

    FMATRIX			*root;
	FVECTOR			offset[2];
	
	FVECTOR			pos[2];
	FVECTOR			pre_pos[2];
	FVECTOR			vec[2];

	int				n_disp;		//残すプリミティブ数
	float			alpha;
	float			add_alpha;
	int				fade_count;
	DG_PRIM2		*prim;
	float			verts_alp[0];

} Work;




static void RecieveMessage( Work *work )
{

    GV_MSG*	msg;
    int 	n_msg;


    if ( work->name == 0 ) {
		return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

		switch ( msg->message[ 0 ] ) {
		case 0:			/* fade_in, fade_out */
			//printf("receive_code %d, %d,%d\n", msg->message[ 0 ], msg->message[ 1 ], msg->message[ 2 ] );
			work->fade_count = DIRECT_TICK( msg->message[2] );
			if ( work->fade_count != 0 ) {
				work->add_alpha = ( (float)msg->message[1] - work->alpha ) / (float)work->fade_count;
			}
			//printf("get_message : add_alpha %f, fade_count %d\n", work->add_alpha, work->fade_count);
			break;
		}
		msg++;
    }

	return;
}



static void Act(Work *work)
{

	DG_PRIM2		*prim = work->prim;
	int				i,j,clock,n_disp = work->n_disp;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb,*pre_uvrgb;
	float			alpha;
	float			*verts_alp = work->verts_alp, diff_alpha;
   int buffSwitch;

	//printf("%f\n",alpha);
#ifndef USE_VU0
	FVECTOR		vec_a[2],vec_b[2];
#endif



	/* debug */
#if 0	
    if ( GV_PadData[1].press & PAD_L1 ) {	/* fade_in */
	    work->code[0] = 0;	
	    work->code[1] = 16;
		work->code[2] = 60;

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 3;
	    GV_SendMessage( &msg );

		printf("send_message : fade_in\n");
    }
    if ( GV_PadData[1].press & PAD_L2 ) {	/* fade_out */
	    work->code[0] = 0;	
	    work->code[1] = 0;
		work->code[2] = 60;

	    msg.address = work->name;
	    msg.message = work->code;
	    msg.message_len = 3;
	    GV_SendMessage( &msg );

		printf("send_message : fade_out\n");
    }
#endif	

	// message受信
	RecieveMessage( work );

	// alpha操作
	if ( work->fade_count > 0 ) {
		work->alpha += work->add_alpha;
		work->fade_count--;
	}	
	alpha = work->alpha;
		
   //AR_PARTICLE_FULL
	buffSwitch = DG_SwitchBuffPrim2( prim );
	clock = work->prim->buffer_clock;

	pos = MEM_SCR_POS;
	uvrgb = MEM_SCR_UV;
	pre_uvrgb = prim->uvrgb[buffSwitch ^ clock];

	TS_Mem_Scr( &pos[N_VERTS], prim->pos[buffSwitch ^ clock], sizeof(FVECTOR), (n_disp-1)*N_VERTS );
	TS_Mem_Scr( &uvrgb[N_VERTS], pre_uvrgb, sizeof(DG_PRIM2_UVRGB), (n_disp-1)*N_VERTS );
	TS_Mem_Scr( uvrgb, &pre_uvrgb[(n_disp-1)*N_VERTS], sizeof(DG_PRIM2_UVRGB), N_VERTS );

	//memmove( &verts_alp[1], &verts_alp[0], sizeof(float)*(n_disp-1) );
	//printf( "0: %f\n", alpha );

	for( i = 1; i < n_disp; i++ ){
		//verts_alp[i] = verts_alp[i-1] - TEST_ALPHA/(float)n_disp;
		verts_alp[i] = verts_alp[i-1] - alpha/(float)n_disp;
		if( verts_alp[i] < 0.0f ) verts_alp[i] = 0.0f;
		//printf( "%d: %f\n", i, verts_alp[i] );
	}
	//printf( "\n" );
	verts_alp[0] = alpha;



	/* 剣の両ポジション算出 */
	DG_SetPos( work->root );
	DG_PutVector( work->offset, work->pos, 2 );		

	//HZX_ViewMatrix( work->root, 1000.0f );
	
#ifndef USE_VU0
	_sceVu0SubVector( &vec_b[0], &work->pos[0], &work->pre_pos[0] );
	_sceVu0SubVector( &vec_b[1], &work->pos[1], &work->pre_pos[1] );
	_sceVu0ScaleVector( &vec_a[0], &work->vec[0], 0.75f );
	_sceVu0ScaleVector( &vec_a[1], &work->vec[1], 0.75f );
#else
	//下がＶＵの時使用（ｖｆにデータロード）
    asm volatile ("
		qmtc2.ni		%3,vf16		# 0.75f
		lqc2	vf14, 0x00(%1)		#vec[0]
		lqc2	vf15, 0x10(%1)		#vec[1]
		lqc2	vf12, 0x00(%0)		#pos[0]
		lqc2	vf13, 0x10(%0)		#pos[1]
		lqc2	vf08, 0x00(%2)		#pre_pos[0]
		lqc2	vf09, 0x10(%2)		#pre_pos[1]
		vmulx.xyzw		vf04,  vf14, vf16x		# vf04 = vec[0] * (0.75f)
		vmulx.xyzw		vf05,  vf15, vf16x		# vf05 = vec[1] * (0.75f)
		vsub.xyzw		vf06,  vf12, vf08		# vf06 = vec[0] - pre_pos[0]
		vsub.xyzw		vf07,  vf13, vf09		# vf07 = vec[1] - pre_pos[1]
	": : "r"(work->pos), "r"(work->vec), "r"(work->pre_pos), "r"(0.75f) : "memory" );

#endif


	pos += N_VERTS-2;
	//一個目
	diff_alpha = (verts_alp[1] - verts_alp[0])/(float)N_INTER;
	DG_COPY_VEC( &pos[0], &work->pre_pos[0] );
	DG_COPY_VEC( &pos[1], &work->pre_pos[1] );
	uvrgb[0].a = (int)alpha;
	uvrgb[1].a = (int)alpha;
	alpha -= diff_alpha;
	if( alpha < 0.0f ) alpha = 0.0f;
	uvrgb+=2;
	pos-=2;

	
	for( i = 1; i < N_INTER; i++ ){
#ifndef USE_VU0
		//元
		float	ratio = (float)i/(float)N_INTER;
		float	ratio_b = (float)i/(float)N_INTER;
		float	ratio_a = 1.0f - ratio_b;
		FVECTOR	fvtemp_a, fvtemp_b;

		_sceVu0ScaleVector( &fvtemp_b, &vec_b[0], ratio*ratio_b );
		_sceVu0ScaleVector( &fvtemp_a, &vec_a[0], ratio*ratio_a );
		_sceVu0AddVector( &pos[0], &fvtemp_a, &fvtemp_b );
		_sceVu0AddVector( &pos[0], &pos[0], &work->pre_pos[0] );

		_sceVu0ScaleVector( &fvtemp_b, &vec_b[1], ratio*ratio_b );
		_sceVu0ScaleVector( &fvtemp_a, &vec_a[1], ratio*ratio_a );
		_sceVu0AddVector( &pos[1], &fvtemp_a, &fvtemp_b );
		_sceVu0AddVector( &pos[1], &pos[1], &work->pre_pos[1] );
#else
		//ＶＵ化
		float	ratio = (float)i/(float)N_INTER;
		float	ratio_b = ratio;//(float)i/(float)N_INTER;
		float	ratio_a = 1.0f - ratio_b;

        asm volatile ("
			qmtc2.ni		%1,vf10			#ratio*ratio_a
			qmtc2.ni		%2,vf11			#ratio*ratio_b

			vmulax.xyzw		ACC,  vf04, vf10x		# acc = vec_a[0] * (ratio*ratio_a)
			vmaddax.xyzw	ACC,  vf06, vf11x		# acc = acc + vec_b[0] * (ratio*ratio_b)
			vmaddw.xyzw		vf12, vf08, vf00w		# vf12 = acc + pos[0]

			vmulax.xyzw		ACC,  vf05, vf10x		# acc = vec_a[1] * (ratio*ratio_a)
			vmaddax.xyzw	ACC,  vf07, vf11x		# acc = acc + vec_b[1] * (ratio*ratio_b)
			vmaddw.xyzw		vf13, vf09, vf00w		# vf12 = acc + pos[1]

			sqc2	        vf12, 0x00(%0)
			sqc2	        vf13, 0x10(%0)
		": : "r"(pos), "r"(ratio*ratio_a), "r"(ratio*ratio_b) : "memory" );
		
#endif
		uvrgb[0].a = (int)alpha;
		uvrgb[1].a = (int)alpha;
		alpha -= diff_alpha;
		if( alpha < 0.0f ) alpha = 0.0f;
		uvrgb+=2;
		pos-=2;
	}


	//最後
	DG_COPY_VEC( &pos[0], &work->pos[0] );
	DG_COPY_VEC( &pos[1], &work->pos[1] );
	uvrgb[0].a = (int)verts_alp[1];
	uvrgb[1].a = (int)verts_alp[1];
	uvrgb+=2;
	for( i = 1; i < n_disp; i++ ){
		alpha = verts_alp[i];
		diff_alpha = ( verts_alp[i-1] - verts_alp[i] )/N_INTER;
		for( j = 0; j < N_VERTS; j++ ){
			uvrgb->a = (int)alpha;
			//printf("%2d alpha %f\n", j+ i*N_VERTS, alpha );
			if( j & 1 )alpha -= diff_alpha;
			if( alpha < 0.0f ) alpha = 0.0f;
			uvrgb++;
		}
	}
	//printf("\n");


	/* 次フレームのためにパラメータ確保 */
	_sceVu0SubVector( &work->vec[0], &work->pos[0], &work->pre_pos[0] );
	_sceVu0SubVector( &work->vec[1], &work->pos[1], &work->pre_pos[1] );
	DG_COPY_VEC( &work->pre_pos[0], &work->pos[0] );
	DG_COPY_VEC( &work->pre_pos[1], &work->pos[1] );


	TS_Scr_Mem( prim->pos[clock], MEM_SCR_POS, sizeof(FVECTOR), n_disp*N_VERTS );
	TS_Scr_Mem( prim->uvrgb[clock], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), n_disp*N_VERTS );
}




static void Die(Work *work)
{
	if ( work->prim ) GM_FreePrim2( work->prim );
}




static int GetResources( Work *work, int n_disp, int init_col, int tex_code )
{

	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB	*uvrgb = MEM_SCR_UV;
	int				i, j;
	int				th, tw, off_u, off_v, h, w;
	float			alpha;
#if 0
	int				u[2];
	float			fv;
#else
	int				v[2];
	float			fu;
#endif

	tex = DG_GetTexture( tex_code );
	if ( !tex ) {
	    printf("ERR!! NO TEX!! blade_flow.c\n");
	    return (-1);
	}

	work->prim = prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
									  n_disp,
									  N_VERTS );
	if ( !prim ) {
	    printf("ERR!! MAKE PRIM2!! blade_flow.c\n");
	    return (-1);
	}

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	// まだチャンネル１～３は無いが取りあえず フラグ立て
	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	DG_GetTexelInfo( &w, &h, &off_u, &off_v, tex );
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	tw = 1 << tw ;
	th = 1 << th ;
#if 0
	u[0] = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	u[1] = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
#else
	v[0] = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	v[1] = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
#endif

	for( i = 0; i < n_disp; i++ ){
#if 0
		fv = 1.0f*(float)i/(float)n_disp;
#else
		fu = 1.0f*(float)i/(float)n_disp;
#endif
		alpha = TEST_ALPHA - TEST_ALPHA*(float)i/(float)n_disp;
		for( j = 0; j < N_VERTS; j++ ){

			//DG_COPY_VEC( pos, &DG_ZeroVector );
			DG_COPY_VEC( pos, &work->pos[j&1] );
//180 96 240
			uvrgb->r = GET_COL_R(init_col);//4;//255 - 16;
			uvrgb->g = GET_COL_G(init_col);//10;//255 - 16;
			uvrgb->b = GET_COL_B(init_col);//14;//255 - 16;
			uvrgb->a = 0;//(int)alpha;

			uvrgb->q = 4096;
			uvrgb->f = (j<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
#if 0
			uvrgb->u = u[j&1];
			uvrgb->v = FTOI12( fv * tex->v_scale + tex->v_offset );
			if(j&1){
				fv += 1.0f/(float)(n_disp*N_INTER);
				alpha -= TEST_ALPHA/(float)(n_disp*N_INTER);
				if( alpha < 0.0f ) alpha = 0.0f;
			}
#else
			uvrgb->u = FTOI12( fu * tex->u_scale + tex->u_offset );
			uvrgb->v = v[j&1];
			if (j&1) {
				fu += 1.0f/(float)(n_disp*N_INTER);
				alpha -= TEST_ALPHA/(float)(n_disp*N_INTER);
				if( alpha < 0.0f ) alpha = 0.0f;
			}
#endif
			pos++;
			uvrgb++;
		}
	}
	
	TS_Scr_Mem( prim->pos[0],   MEM_SCR_POS, sizeof(FVECTOR),        n_disp*N_VERTS );
	TS_Scr_Mem( prim->pos[1],   MEM_SCR_POS, sizeof(FVECTOR),        n_disp*N_VERTS );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV,  sizeof(DG_PRIM2_UVRGB), n_disp*N_VERTS );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV,  sizeof(DG_PRIM2_UVRGB), n_disp*N_VERTS );

	return (0);
}



// デモ用剣軌跡
void *NewDemoBladeFlow( int name, FMATRIX *root, int bld_mode, int n_disp )
{

	Work 	*work = NULL;
	FVECTOR	*shift;
	int		tex_code;
	int		init_col;

	if( MAX_DISP < n_disp ){
		printf("多すぎるよ<blade_eft.c>\n");
		return NULL;
	}

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof(Work) + sizeof(FVECTOR)*n_disp );

	if ( work ) {

		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );

		work->name = name;

		/* ルートマトリクスと両オフセットを取得 */
		work->root = root;
		shift = Shifts[bld_mode];
		DG_COPY_VEC( &work->offset[0], &shift[0] );
		DG_COPY_VEC( &work->offset[1], &shift[1] );

		/* 前位置を算出しておく */
		DG_SetPos( work->root );
		DG_PutVector( work->offset, work->pos, 2 );
		DG_COPY_VEC( &work->pre_pos[0], &work->pos[0] );
		DG_COPY_VEC( &work->pre_pos[1], &work->pos[1] );

		/* テクスチャ,色取得 */
		tex_code = BladeTexCode[bld_mode];
		init_col = BladeColor[bld_mode];
		//printf("tex %d, color %x\n", tex_code, init_col);

		/* 速度ベクトル初期化 */
		DG_COPY_VEC( &work->vec[0], &DG_ZeroVector );
		DG_COPY_VEC( &work->vec[1], &DG_ZeroVector );

		work->n_disp = n_disp;
		work->alpha  = 0.0f;

		if ( GetResources( work, n_disp, init_col, tex_code ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}

	}

	return (void *)work ;

}

