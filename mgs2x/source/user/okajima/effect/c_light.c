//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	c_light.c
	懐中電灯の光
	1999/09/28 S.Okajima
	$Id: c_light.c,v 1.1.1.3 2002/11/19 11:47:02 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
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
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

extern int ok_flush_status;
extern float ok_focus_z_far;
extern float ok_focus_z_near;

extern void OK_RemoveDynamicLight( FVECTOR *pos );
extern void OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot );

/*----------------------------------------------------------------*/
#define	SCREEN_NEAR	( 51.0f )

#define	RAISE					(4000.0f)
#define	RAISE_SHIFT_WORLD		(100.0f)

#define	DUMMY_SIZE		(0)
#define	DUMMY_COLOR		(32)
#define	DUMMY_ALPHA		(0)

#define	RATIO_STEP		(0.05f)

#define	CLIP_LENGTH		(10000.0f)

/* コーン */
#define	N_PRIMS			(1)
//#define	N_VERTS			(32)
#define	N_VERTS			(24)

/* 中心から円錐ライトの最遠中心まで */
//#define	LIGHT_LEN		(1200)
#define	LIGHT_LEN		(800)

#define	DUMMY_Z			(5000.0f)

/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR		(3000.0f)
//#define	FADE_RANGE_FAR		(4000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR		(500.0f)
//#define	FADE_RANGE_NEAR		(2000.0f)

#define	NEAR_FADE_P_SIZE		(120.0f)
#define	NEAR_FADE_P_SIZE_MIN	(100.0f)
#define	FAR_FADE_P_SIZE			(400.0f)
#define	FAR_FADE_P_SIZE_MIN		(300.0f)

#define	RGB_BASE			(192)
#define	RGB_BASE_FADE_NEAR	(128)
#define	RGB_BASE_FADE_FAR	(255)
#define	RGB_BASE_HOLA		(255)

//#define	ALPHA_BASE				(16.0f)
#define	ALPHA_BASE				(24.0f)
#define	ALPHA_BASE_FADE_NEAR	(28.0f)
#define	ALPHA_BASE_FADE_FAR		(48.0f)
//#define	ALPHA_BASE_HOLA			(96.0f)
#define	ALPHA_BASE_HOLA			(64.0f)

/* カメラ前ハレーションサイズ */
#define	SC_P_SIZE		(50.0f)

/* AKS */
#define	SHIFT0_X			(19.5f)
#define	SHIFT0_Y			(-735.0f)
#define	SHIFT0_Z			(27.0f)

/* AKS_SP */
#define	SHIFT1_X			(-12.5f)
#define	SHIFT1_Y			(-464.3f)
#define	SHIFT1_Z			(93.0f)

/* SHIELD */
#define	SHIFT2_X			(54.0f)
#define	SHIFT2_Y			(-100.0f)
#define	SHIFT2_Z			(203.0f)

/* プレイヤーＵＳＰ（GUN_LIGHT2 付きの時）*/
#define	SHIFT3_X			(17.5f)
#define	SHIFT3_Y			(-297.0f)
#define	SHIFT3_Z			(20.5f)

/* 懐中電灯*/
#define	SHIFT4_X			(0.0f)
#define	SHIFT4_Y			(-180.0f)
#define	SHIFT4_Z			(100.0f)

/* あばかむ */
#define	SHIFT5_X			(20.0f)
#define	SHIFT5_Y			(-627.0f)
#define	SHIFT5_Z			(27.0f)

/* はいてくｍ４ */
#define	SHIFT6_X			(-18.0f)
#define	SHIFT6_Y			(-589.0f)
#define	SHIFT6_Z			(80.0f)

/* すぱす１２ショットガン */
#define	SHIFT7_X			(49.5f)
#define	SHIFT7_Y			(-800.0f)
#define	SHIFT7_Z			(77.5f)


typedef	struct	{
	GV_ACT_EX		actor ;
	int			*map ;

	float		ratio;

	FMATRIX		*world;

	int			mode;
	int			*flag;

	FVECTOR		l_pos ;	/* 円錐の位置（外部計算用） */
	SVECTOR		l_rot ;	/* 円錐の方向（外部計算用） */

	/* 光円錐 */
	DG_PRIM2	*prim ;

	/* 光源 */
	DG_PRIM2	*prim_fade0 ;
	DG_PRIM2	*prim_fade1 ;

	/* スクリーンハレーション */
	DG_PRIM2	*prim_sc ;

} Work ;


static FVECTOR s_pos[N_VERTS];

/*----------------------------------------------------------------*/
static	void	ALL_Invisible_1( Work *work )
{
	DG_InvisiblePrim2Chanl( work->prim, 0 );
	DG_InvisiblePrim2Chanl( work->prim_fade0, 0 );
	DG_InvisiblePrim2Chanl( work->prim_fade1, 0 );
	DG_InvisiblePrim2Chanl( work->prim_sc, 0 );
}

/*----------------------------------------------------------------*/
static	void	ALL_Invisible( Work *work )
{
#if 1
	DG_InvisiblePrim2( work->prim ) ;
	DG_InvisiblePrim2( work->prim_fade0 ) ;
	DG_InvisiblePrim2( work->prim_fade1 ) ;
	DG_InvisiblePrim2( work->prim_sc ) ;
#else
	DG_InvisiblePrim2Chanl( work->prim, 0 );
	DG_InvisiblePrim2Chanl( work->prim_fade0, 0 );
	DG_InvisiblePrim2Chanl( work->prim_fade1, 0 );
	DG_InvisiblePrim2Chanl( work->prim_sc, 0 );
#endif
}

/*----------------------------------------------------------------*/
static	void	ALL_Visible( Work *work )
{
	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim,       (*work->map) ) ;
	DG_VisiblePrim2( work->prim_fade0 ) ;
	GM_GroupPrim2( work->prim_fade0, (*work->map) ) ;
	DG_VisiblePrim2( work->prim_fade1 ) ;
	GM_GroupPrim2( work->prim_fade1, (*work->map) ) ;
	DG_InvisiblePrim2( work->prim_sc ) ;
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR shift0 = { SHIFT0_X, SHIFT0_Y, SHIFT0_Z, 1.0f };
	FVECTOR shift1 = { SHIFT1_X, SHIFT1_Y, SHIFT1_Z, 1.0f };
	FVECTOR shift2 = { SHIFT2_X, SHIFT2_Y, SHIFT2_Z, 1.0f };
	FVECTOR shift3 = { SHIFT3_X, SHIFT3_Y, SHIFT3_Z, 1.0f };
	FVECTOR shift4 = { SHIFT4_X, SHIFT4_Y, SHIFT4_Z, 1.0f };
	FVECTOR shift5 = { SHIFT5_X, SHIFT5_Y, SHIFT5_Z, 1.0f };
	FVECTOR shift6 = { SHIFT6_X, SHIFT6_Y, SHIFT6_Z, 1.0f };
	FVECTOR shift7 = { SHIFT7_X, SHIFT7_Y, SHIFT7_Z, 1.0f };

	int	i;
	int	itemp;
	int	clock;
	int	temp_n_verts;
	int	chanl_flag;
	float	ratio;
	float	angle_ratio;
	float	ftemp;
	float	inner;
	float	angle;
	float	len;
	float	change_start;
	float	change_end;
	float	screen_near_x;
	float	screen_near_y;
	SVECTOR	svtemp;
	FVECTOR	shift_vec;
	FVECTOR	fvtemp;
	FVECTOR	cam_lit;
	FVECTOR	cam_pos;
	FVECTOR	*pfv;
	FVECTOR local_s_pos[N_VERTS];
	FMATRIX		tempmat;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	DG_PRIM2_UVRGB		*uvrgb ;


	if( work->map==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}

	DG_COPY_VEC( &work->l_pos, (FVECTOR *)work->world->m[3] );

	DG_SetPos( &DG_Chanls->eye_inv );
	_sceVu0SubVector( &fvtemp, &work->l_pos, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	DG_RotVector( &fvtemp, &fvtemp, 1 );	/* 光源の透視変換点 */
//printf("fvtemp.vz:%f\n",fvtemp.vz);


//	chanl_flag = DG_Chanl( 1 )->flag;//GM_CheckMenuStatus(MENU_SUBWIN_ON);	// 子画面フラグ
//printf("chanl_flag:%d\n",chanl_flag);

	// 子画面もね
	chanl_flag = GM_GetMapIDfromChanlDisp(0)|GM_GetMapIDfromChanlDisp(1);

	if( *work->flag == -1 ){
		ALL_Invisible( work );
		OK_RemoveDynamicLight( &work->l_pos );
		GV_DestroyActor( work ) ;
		return;
	}else if( *work->flag == 0
//	 || ( (((*work->map) & GM_CurrentStageMap)==0) && !GM_CheckGameStatus( STATE_CLEARING ) ) ){
	 || !((*work->map) & chanl_flag) ){
		ALL_Invisible( work );
		OK_RemoveDynamicLight( &work->l_pos );

		return;
	}else if( (*work->flag == 1)
	       || (*work->flag == 2 && ( GM_CheckPlayerStatus(PLAYER_INTRUDE) ) )
	        ){
		ALL_Visible( work );
		if( !((*work->map) & chanl_flag) ){
			OK_RemoveDynamicLight( &work->l_pos );
		}else{
			OK_SetDynamicLight( &work->l_pos, &work->l_rot );
		}
	}
	//printf("flag %d:map %x\n",*work->flag, ((*work->map) & chanl_flag) );
//	if( (fvtemp.vz < 0.0f - LIGHT_LEN) && !GM_CheckGameStatus( STATE_CLEARING ) ){	// カメラの後ろ
//	if( (fvtemp.vz < 0.0f - LIGHT_LEN) ){	// カメラの後ろ
	if( fvtemp.vz < 0.0f ){	// カメラの後ろ
		ALL_Invisible_1( work );
//		return;
	}

	angle = DG_Chanls->screen;

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / angle;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / angle;

	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );

	/* カメラからの距離を求める */
	_sceVu0SubVector( &fvtemp, &work->l_pos, &cam_pos );
	len=GV_VecLen3F( &fvtemp );

	if( ok_focus_z_far!=0.0f ){
		change_start = ok_focus_z_far - FADE_RANGE_NEAR;
		if(change_start < DRAW_Z_MIN) change_start = DRAW_Z_MIN;
		change_end   = ok_focus_z_far + FADE_RANGE_FAR;
		if(change_end   > DRAW_Z_MAX) change_end   = DRAW_Z_MAX;
	}else{
		/* キャラ：Ｚ制御が起動していないとき */
		change_start = DUMMY_Z - FADE_RANGE_NEAR;
		if(change_start < DRAW_Z_MIN) change_start = DRAW_Z_MIN;
		change_end   = DUMMY_Z + FADE_RANGE_FAR;
		if(change_end   > DRAW_Z_MAX) change_end   = DRAW_Z_MAX;
	}

//画角とぼやけ具合をリンク
	change_start *= (angle*0.5f);
	change_end   *= (angle*0.5f);

	if(len > change_end){
		ratio=1.0f;
	}else if(len > change_start ){
		ratio = (len - change_start) / (change_end - change_start);
	}else{
		ratio=0.0f;		/* フラグとしても使用 */
	}
	if( ratio - work->ratio > RATIO_STEP ){
		work->ratio+=RATIO_STEP;
	}else if( ratio - work->ratio < -RATIO_STEP ){
		work->ratio-=RATIO_STEP;
	}

//if(GV_Time%120==0)printf("%f %f\n",ratio,work->ratio);


	DG_SwitchBuffPrim2( work->prim );
	DG_SwitchBuffPrim2( work->prim_fade0 );
	DG_SwitchBuffPrim2( work->prim_fade1 );
	DG_SwitchBuffPrim2( work->prim_sc );
	clock = work->prim->buffer_clock;

//AN_Test_Eye2( (FVECTOR *)work->world->m[3], 2 );

	DG_SetPos( work->world );

	switch( work->mode ){
	  case 0:
		DG_MovePos( &shift0 );
		break;
	  case 1:
		DG_MovePos( &shift1 );
		break;
	  case 2:
		DG_MovePos( &shift2 );
		svtemp.vx=0;
		svtemp.vy=0;
		svtemp.vz=1024;
		DG_RotatePos( &svtemp );
		break;
	  case 3:
		DG_MovePos( &shift3 );
		break;
	  case 4:
		DG_MovePos( &shift4 );
		svtemp.vx=-1024;
		svtemp.vy=0;
		svtemp.vz=0;
		DG_RotatePos( &svtemp );
		break;
	  case 5:
		DG_MovePos( &shift5 );
		break;
	  case 6:
		DG_MovePos( &shift6 );
		break;
	  case 7:
		DG_MovePos( &shift7 );
		break;
	}

	DG_GetPos( &tempmat );
	DG_COPY_VEC( &shift_vec, (FVECTOR *)tempmat.m[3] );


	_sceVu0SubVector( &fvtemp, &cam_pos, &shift_vec ) ;
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, RAISE_SHIFT_WORLD );
	_sceVu0AddVector( &shift_vec, &shift_vec, &fvtemp ) ;

	fvtemp.vx=0.0f;
	fvtemp.vy=1.0f;
	fvtemp.vz=0.0f;
	DG_RotVector( &fvtemp, &fvtemp, 1 );	/* 光源の方向ベクトル */

	/* カメラからライトへのベクトルと、ライトのベクトルの内積 */
	_sceVu0SubVector( &cam_lit, (FVECTOR *)tempmat.m[3], &cam_pos ) ;
	_sceVu0Normalize( &cam_lit, &cam_lit );
	inner = _sceVu0InnerProduct( &fvtemp, &cam_lit );
	inner = (inner > 0.0f)? inner: -inner;

	ftemp = 1.0f - inner*0.5f;

	temp_n_verts = (int)( (float)(N_VERTS) * ftemp );

//printf("temp_n_verts:%d\n",temp_n_verts);
	if( temp_n_verts < 0 ){
printf("a:temp_n_verts:%d\n",temp_n_verts);
		temp_n_verts = 0;
	}else if( temp_n_verts > N_VERTS ){
printf("b:temp_n_verts:%d\n",temp_n_verts);
		temp_n_verts = N_VERTS;
	}

	//---------------
	if(  work->mode==3		/* プレイヤー */
	  || work->mode==4){	/* 懐中電灯 */
		DG_SetTmpLight2(
			&work->l_pos,
			1250.0f,
			2500.0f,
			32 | 32<<8 | 32 <<16,
			LIT_FLAG_CHARAONLY ) ;
		OK_ScaleVector( local_s_pos, s_pos, ftemp, N_VERTS );
		DG_PutVector( local_s_pos, work->prim->pos[clock], N_VERTS );
		OK_DirVecXY( &work->l_pos, &work->prim->pos[clock][N_VERTS-1], &work->l_rot );
		uvrgbwh = work->prim->uvrgb[clock];
		for( i=0; i<temp_n_verts; i++ ){
			uvrgbwh->w = uvrgbwh->h = (i*10 + (int)(320.0f*work->ratio) + 30) * 3;
			uvrgbwh->a = ((int)( ALPHA_BASE * (1.0f - work->ratio) * (float)(temp_n_verts-i) / (float)temp_n_verts ) + 8) / 4;
			uvrgbwh++;
		}
		for( i=temp_n_verts; i<N_VERTS; i++ ){
			uvrgbwh->w = uvrgbwh->h = 0;
			uvrgbwh->a = 0;
			uvrgbwh++;
		}
	}else{
		OK_ScaleVector( local_s_pos, s_pos, ftemp, N_VERTS );
		DG_PutVector( local_s_pos, work->prim->pos[clock], N_VERTS );
		OK_DirVecXY( &work->l_pos, &work->prim->pos[clock][N_VERTS-1], &work->l_rot );
		uvrgbwh = work->prim->uvrgb[clock];
		for( i=0; i<temp_n_verts; i++ ){
			ftemp = ALPHA_BASE * (float)(N_VERTS - temp_n_verts + i) * (1.0f - work->ratio) / (float)N_VERTS + 2;
			uvrgbwh->a = (int)(ftemp);

			uvrgbwh->w = uvrgbwh->h = (N_VERTS - temp_n_verts + i)*12 + (int)(320.0f*work->ratio) + 30;
//			uvrgbwh->a = (int)( ALPHA_BASE * (1.0f - work->ratio) * (float)(N_VERTS-i) / (float)N_VERTS ) + 8;
//			uvrgbwh->a = (int)( ALPHA_BASE * (1.0f - work->ratio) * (float)(N_VERTS-itemp) / (float)N_VERTS ) + 2;
			uvrgbwh++;
		}
		for( i=temp_n_verts; i<N_VERTS; i++ ){
			uvrgbwh->w = uvrgbwh->h = 0;
			uvrgbwh->a = 0;
			uvrgbwh++;
		}
	}

	//---------------
	DG_COPY_VEC( &work->prim_fade0->pos[clock][0], &shift_vec );
	uvrgbwh = work->prim_fade0->uvrgb[clock];
	uvrgbwh->w = uvrgbwh->h = (int)(FAR_FADE_P_SIZE*work->ratio + FAR_FADE_P_SIZE_MIN);

	uvrgbwh->a = (int)(ALPHA_BASE_FADE_FAR * work->ratio);
	DG_COPY_VEC( &work->prim_fade1->pos[clock][0], &shift_vec );
	uvrgbwh = work->prim_fade1->uvrgb[clock];
	uvrgbwh->w = uvrgbwh->h = (int)(NEAR_FADE_P_SIZE*(1.0f-work->ratio) + NEAR_FADE_P_SIZE_MIN);
	uvrgbwh->a = (int)(ALPHA_BASE_FADE_NEAR * (1.0f-work->ratio));

	//--------------- SPECIAL MODE
	if( *work->flag == 2 && GM_CheckPlayerStatus(PLAYER_INTRUDE) ){
		uvrgb = work->prim_sc->uvrgb[clock];
		pfv = work->prim_sc->pos[clock];
		DG_SetPos( &DG_Chanls->eye_pers );
		DG_PutVector( (FVECTOR *)tempmat.m[3], &fvtemp, 1 );
		if( HZX_OnlineHazardCheck( /* ハザードチェック */
		      GM_GetHzxGroupID(*work->map),
	          &work->l_pos,
	          &cam_pos,
	          HZX_CHK_FIX,
	          HZX_SEG_NO_PLAYER,
	          HZX_FLOOR_NO_PLAYER ) ){

			itemp = 0;
			ftemp = 0.0f;
		}else{
			DG_SetPos( &DG_Chanls->eye_inv );
			shift_vec.vx=0.0f;
			shift_vec.vy=1.0f;
			shift_vec.vz=0.0f;
			DG_RotVector( &shift_vec, &shift_vec, 1 );	/* 光源の透視変換点 */
			angle_ratio = (shift_vec.vz > 0.0f)?shift_vec.vz*inner: 0.0f;
			itemp = (int)(ALPHA_BASE_HOLA * angle_ratio * (1.0f - work->ratio) );	/* 遠いと暗い */
			itemp = itemp*itemp*itemp/ALPHA_BASE_HOLA/ALPHA_BASE_HOLA;
			if( itemp < 0 ){
				itemp = 0;
			}else if( itemp > ALPHA_BASE_HOLA ){
				itemp = ALPHA_BASE_HOLA;
			}
			ftemp = SC_P_SIZE * angle_ratio * (1.0f - work->ratio) + SC_P_SIZE*0.25f;	/* 遠いと小さい */
			if( ftemp < 0.0f ){
				ftemp = 0.0f;
			}else if( ftemp > SC_P_SIZE ){
				ftemp = SC_P_SIZE;
			}
		}
		if( fvtemp.vz < fvtemp.vw ){
			fvtemp.vw = (fvtemp.vw  > 0.0f)? fvtemp.vw: -fvtemp.vw;
			fvtemp.vx = screen_near_x * fvtemp.vx / fvtemp.vw;
			fvtemp.vy = screen_near_y * fvtemp.vy / fvtemp.vw;
			uvrgb->a = itemp;	uvrgb++;
			uvrgb->a = itemp;	uvrgb++;
			uvrgb->a = itemp;	uvrgb++;
			uvrgb->a = itemp;	uvrgb++;
			pfv->vz = SCREEN_NEAR;
			pfv->vx = fvtemp.vx - ftemp;
			pfv->vy = fvtemp.vy - ftemp;
			pfv++;
			pfv->vz = SCREEN_NEAR;
			pfv->vx = fvtemp.vx + ftemp;
			pfv->vy = fvtemp.vy - ftemp;
			pfv++;
			pfv->vz = SCREEN_NEAR;
			pfv->vx = fvtemp.vx - ftemp;
			pfv->vy = fvtemp.vy + ftemp;
			pfv++;
			pfv->vz = SCREEN_NEAR;
			pfv->vx = fvtemp.vx + ftemp;
			pfv->vy = fvtemp.vy + ftemp;
			pfv++;
		}else{
			uvrgb->a = 0;	uvrgb++;
			uvrgb->a = 0;	uvrgb++;
			uvrgb->a = 0;	uvrgb++;
			uvrgb->a = 0;	uvrgb++;
			DG_COPY_VEC( pfv, &DG_ZeroVector );	pfv++;
			DG_COPY_VEC( pfv, &DG_ZeroVector );	pfv++;
			DG_COPY_VEC( pfv, &DG_ZeroVector );	pfv++;
			DG_COPY_VEC( pfv, &DG_ZeroVector );	pfv++;
		}
	}

/*
	if( ok_flush_status!=0 ){
		DG_InvisiblePrim2( work->prim ) ;
		DG_InvisiblePrim2( work->prim_fade0 ) ;
		DG_InvisiblePrim2( work->prim_fade1 ) ;
		DG_InvisiblePrim2( work->prim_sc ) ;
	}else{
		DG_VisiblePrim2( work->prim ) ;
		DG_VisiblePrim2( work->prim_fade0 ) ;
		DG_VisiblePrim2( work->prim_fade1 ) ;
		DG_VisiblePrim2( work->prim_sc ) ;
	}
*/

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	work->prim_fade0 = OK_FreePrim2( work->prim_fade0 );
	work->prim_fade1 = OK_FreePrim2( work->prim_fade1 );
	work->prim_sc = OK_FreePrim2( work->prim_sc );
	OK_RemoveDynamicLight( &work->l_pos );
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int prim_num, int vert_num, int rgb )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	for ( i=0; i<2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		for ( j=0; j<prim_num; j++ ){
			for ( k=0; k<vert_num; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h = uvrgbwh->w = DUMMY_SIZE ;
				uvrgbwh->r = uvrgbwh->g = uvrgbwh->b = rgb ;
				uvrgbwh->a = DUMMY_ALPHA ;
				uvrgbwh ++ ;
			}
		}
	}

}

/*----------------------------------------------------------------*/
static void InitPacketPoly( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int rgb )
{
	DG_PRIM2_UVRGB		*uvrgb ;
	int		i ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	for ( i=0; i<2; i++ ){
		uvrgb = prim->uvrgb[i] ;
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = uvrgb->g = uvrgb->b = rgb ;
		uvrgb->a = DUMMY_ALPHA ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = uvrgb->g = uvrgb->b = rgb ;
		uvrgb->a = DUMMY_ALPHA ;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = uvrgb->g = uvrgb->b = rgb ;
		uvrgb->a = DUMMY_ALPHA ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = uvrgb->g = uvrgb->b = rgb ;
		uvrgb->a = DUMMY_ALPHA ;
		uvrgb++;
	}

}

static int GetResources( Work *work, FMATRIX *world, int *flag, int mode, int *map )
{
	int	i;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->map = map;
	work->ratio=0.0f;

	work->mode = mode;
	work->world = world;
	work->flag=flag;

	//-----------------------------
	tex = DG_GetTexture( 12296685 /*"light10_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	prim->raise=RAISE;
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS, RGB_BASE );

	for ( i = 0 ; i < N_VERTS ; i++ ){
		s_pos[i].vx=0.0f;
		s_pos[i].vy=-(float)( i * LIGHT_LEN / (N_VERTS-1) );
		s_pos[i].vz=0.0f;
	}

	//-----------------------------
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim_fade0 = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 1 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * 1 * 1 );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * 1 * 1 );
	prim->raise=RAISE;
	InitPacket( work, prim, tex, 1, 1, RGB_BASE_FADE_FAR );

	tex = DG_GetTexture( 762348 /*"light05_msk"*/ );
	prim = work->prim_fade1 = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 1 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * 1 * 1 );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * 1 * 1 );
	prim->raise=RAISE;
	InitPacket( work, prim, tex, 1, 1, RGB_BASE_FADE_NEAR );

	//-----------------------------
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim_sc = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, 1, 4 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * 1 * 4 );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * 1 * 4 );
	prim->raise=RAISE;
	InitPacketPoly( work, prim, tex, RGB_BASE_HOLA );
//	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	ALL_Visible( work );

	//-----------------------------
	OK_SetDynamicLight( &work->l_pos, &work->l_rot );

	return 0 ;
}

void *NewCircleLight(
	FMATRIX *world,	/* 接続マトリックス */
	int *flag,		/* -1:DESTROY  0:OFF 1:ON 2:SPECIAL */
	int mode,	
		/*
		   0:E_WP_AKS
		   1:E_WP_AKS_SP
		   2:アクリル盾
		   3:プレイヤーＵＳＰ（GUN_LIGHT2 付きの時）
		   4:懐中電灯
		   5:あばかむ
		   6:ハイテクｍ４
		   7:すぱす１２ショットガン
		*/
	int *map		/* 接続元マップ */
){
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
//printf("NewCircleLight:%d\n",sizeof( Work ));
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world, flag, mode, map ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
