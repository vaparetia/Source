//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	c_light.c
	懐中電灯の光c
	1999/09/28 S.Okajima
	$Id: c_light_spot.c,v 1.1.1.3 2002/11/19 11:47:03 Yoshizawa1 Exp $
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

extern float ok_focus_z_far;
extern float ok_focus_z_near;

extern void OK_RemoveDynamicLight( FVECTOR *pos );
extern void OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot );

/*----------------------------------------------------------------*/
#define	SCREEN_NEAR	( 51.0f )

#define	RAISE					(0.0f)
#define	RAISE_SHIFT_WORLD		(100.0f)

#define	DUMMY_SIZE		(0)
#define	DUMMY_COLOR		(32)
#define	DUMMY_ALPHA		(0)

#define	RATIO_STEP		(0.05f)

#define	CLIP_LENGTH		(10000.0f)

/* コーン */
#define	N_PRIMS			(4)
#define	N_VERTS			(32)

/* 中心から円錐ライトの最遠中心まで */
//#define	LIGHT_LEN		(1200)
#define	LIGHT_LEN		(15000)

#define	DUMMY_Z			(5000.0f)


#define	FADE_RANGE_FAR		(15000.0f)
#define	FADE_RANGE_NEAR		(5000.0f)


#define	RGB_BASE			(32)

#define	ALPHA_BASE			(64.0f)

#define	SHIFT0_X			(0.0f)
#define	SHIFT0_Y			(1000.0f)
#define	SHIFT0_Z			(0.0f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			count;

	float		ratio;
	int			flag;
	int			flag_scn_msg;

	FVECTOR		shift ;
	FVECTOR		pos ;	/* 円錐の位置（外部計算用） */
	SVECTOR		rot ;	/* 円錐の方向（外部計算用） */

	/* 光円錐 */
	DG_PRIM2	*prim ;

	float		total_length;
	int			poly_num;
	int			max_width;

} Work ;


static FVECTOR s_pos[N_VERTS*N_PRIMS];

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR shift0 = { SHIFT0_X, SHIFT0_Y, SHIFT0_Z, 1.0f };

	int	i;
	int	itemp;
	float	ratio;
	FVECTOR	fvtemp;
	FVECTOR	cam_lit;
	SVECTOR	svtemp;
	FVECTOR	cam_pos;
	float	len;
	float	inner;
	float	ftemp;
	float	screen_near_x;
	float	screen_near_y;
	FMATRIX		tempmat;
	int	clock;
	int	temp_n_verts;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	{/* メッセージチェック */
		GV_MSG *msg;
		int n;
		if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
			for ( ; n > 0 ; n--, msg++ ){
				itemp = msg->message[0];
				if( itemp < 10 ){
					work->flag = msg->message[0] ;
				}else{
					work->flag_scn_msg = msg->message[0] ;
				}
			}
		}
	}

	if( work->flag == 0          ||   work->flag_scn_msg == 10 ){
		DG_InvisiblePrim2( work->prim ) ;
		OK_RemoveDynamicLight( &work->pos );
		return;
	}else if( work->flag == -1 ){
		DG_InvisiblePrim2( work->prim ) ;
		OK_RemoveDynamicLight( &work->pos );
		GV_DestroyActor( work ) ;
		return;
	}else if( work->flag == 1 ){
		DG_VisiblePrim2( work->prim ) ;
		GM_GroupPrim2( work->prim,       work->map ) ;
//		GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
		OK_SetDynamicLight( &work->pos, &work->rot );
	}

   //BP_MATH - fix divide by zero when screen fades down
   if( DG_Chanls->screen == 0.0f )
   {
      DG_Chanls->screen = 0.000001f;   //BP - value taken from blood_cm.c
   }
   //BP_MATH - fix divide by zero when screen fades down

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );

	/* カメラからの距離を求める */
	_sceVu0SubVector( &fvtemp, &work->pos, &cam_pos );
	len=GV_VecLen3F( &fvtemp );

	if(len > FADE_RANGE_FAR){
		ratio=1.0f;
	}else if(len > FADE_RANGE_NEAR ){
		ratio = (len - FADE_RANGE_NEAR) / (FADE_RANGE_FAR - FADE_RANGE_NEAR);
	}else{
		ratio=0.0f;		/* フラグとしても使用 */
	}
	if( ratio - work->ratio > RATIO_STEP ){
		work->ratio+=RATIO_STEP;
	}else if( ratio - work->ratio < -RATIO_STEP ){
		work->ratio-=RATIO_STEP;
	}

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	/* カメラ方向にシフトさせて優先問題を回避 */
	OK_DirVecXY( &work->pos, &cam_pos, &svtemp );
	DG_SetPos2( &work->pos, &svtemp );
	fvtemp.vx=0.0f;
	fvtemp.vy=0.0f;
	fvtemp.vz=RAISE_SHIFT_WORLD;
	DG_PutVector( &fvtemp, &fvtemp, 1 );
//if(GV_Time%360==0)printf("%f %f %f\n",fvtemp.vx,fvtemp.vy,fvtemp.vz);
	DG_SetPos2( &fvtemp, &work->rot );
	DG_MovePos( &shift0 );
	DG_GetPos( &tempmat );
	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz =-1.0f;
	DG_RotVector( &fvtemp, &fvtemp, 1 );	/* 光源の方向ベクトル */

	/* カメラからライトへのベクトルと、ライトのベクトルの内積 */
	_sceVu0SubVector( &cam_lit, (FVECTOR *)tempmat.m[3], &cam_pos ) ;
	_sceVu0Normalize( &cam_lit, &cam_lit );
	inner = _sceVu0InnerProduct( &fvtemp, &cam_lit );
	inner = (inner > 0.0f)? inner: -inner;
	ftemp = 1.0f - inner*0.5f;
	temp_n_verts = (int)( (float)(work->poly_num) * ftemp );


	//---------------
	DG_PutVector( s_pos, work->prim->pos[clock], work->poly_num );
	uvrgbwh = work->prim->uvrgb[clock];


	for( i=0; i<temp_n_verts; i++ ){
		ftemp = ALPHA_BASE * (float)(temp_n_verts - i) / (float)temp_n_verts;
		uvrgbwh->a = (int)(ftemp);
		uvrgbwh->w = uvrgbwh->h = (int)((float)(i*work->max_width) * work->ratio  + 700.0f);
		uvrgbwh++;
	}
	for( i=temp_n_verts; i<work->poly_num; i++ ){
		uvrgbwh->w = uvrgbwh->h = 0;
		uvrgbwh->a = 0;
		uvrgbwh++;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_RemoveDynamicLight( &work->pos );
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int prim_num, int vert_num, int rgb )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	*fvtemp;
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	for ( i=0; i<2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		fvtemp  = prim->pos[i];
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

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
}

static int GetResourcesCalled( Work *work, int name, int where,
			       FMATRIX *root, FVECTOR *pos, FVECTOR *aim,
			       int length, int width, int n_poly )
{
	int	i;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->name = name;
	work->map  = where;

	/* 設置座標 */
	_sceVu0CopyVector( &work->pos, pos ) ;

	/* スポットライト方向 */
	if ( root )/* added by T.Morita */
	    work->rot = DG_ZeroSVector ;
	else
	    OK_DirVecXY( pos, aim, &work->rot );

	/* シナリオ調整用 */
	work->total_length = length ;

	work->poly_num = n_poly*N_PRIMS ;
	if( work->poly_num > N_VERTS*N_PRIMS ) work->poly_num = N_VERTS*N_PRIMS;

	work->max_width = 150;
	if ( width )
	    work->max_width = width / work->poly_num + 1 ;

	work->ratio=0.0f;

	work->flag=1;	/* 表示 */
	work->flag_scn_msg=1;

	//-----------------------------
	tex = DG_GetTexture( 12296685 /*"light10_msk"*/ );
//	tex = DG_GetTexture( 4046924 /*"xlit04b_alp"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );

	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	prim->root =root;/* added by T.Morita */
	prim->raise=RAISE;
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS, RGB_BASE );

	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		s_pos[i].vx=0.0f;
		s_pos[i].vy=0.0f;
		s_pos[i].vz=(float)( i * work->total_length / (N_PRIMS*N_VERTS-1) );
	}


	//-----------------------------
	OK_SetDynamicLight( &work->pos, &work->rot );

	return 0 ;
}


static int GetResources( Work *work, int name, int where )
{
	FVECTOR pos ;
	FVECTOR aim ;

	/* 設置座標 */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		pos.vx = (float)GCL_GetNextInt() ;
		pos.vy = (float)GCL_GetNextInt() ;
		pos.vz = (float)GCL_GetNextInt() ;
		pos.vw = 1.0f ;
	}else{
		printf("GCL:ERR!::c_light_spot::set option P\n");
		return -1;
	}

	/* スポットライト方向 */
	if ( GCL_GetOption( 'd' ) != NULL ) {
		aim.vx = (float)GCL_GetNextInt() ;
		aim.vy = (float)GCL_GetNextInt() ;
		aim.vz = (float)GCL_GetNextInt() ;
		aim.vw = 0.0f ;
	}else{
		printf("GCL:ERR!::c_light_spot::set option D\n");
		return -1;
	}

	if ( GetResourcesCalled( work, name, where,
				 NULL, &pos, &aim, 
				 GCL_GetOptionValue( 'l', LIGHT_LEN ),
				 GCL_GetOptionValue( 'w', 0 ),
				 GCL_GetOptionValue( 'n', N_VERTS ) ) < 0 )
	    return -1 ;
	return 0 ;
}


void *NewCircleLightSpot( int name, int where )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
//printf("NewCircleLightSpot\n");
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


void *NewCircleLightSpotCalled( int name, int where,
				FMATRIX *root, FVECTOR *pos, FVECTOR *aim,
				int length, int width, int n_poly )
{
	Work	*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
//printf("NewCircleLightSpot\n");
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;

		if ( GetResourcesCalled( work, name, where,
					 root, pos, aim,
					 length, width, n_poly ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
