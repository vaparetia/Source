//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	projector_light.c
	プロジェクタの光
	2000/05/17 S.Okajima
	$Id: projector_light.c,v 1.1.1.3 2002/11/19 11:47:09 Yoshizawa1 Exp $
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

/*----------------------------------------------------------------*/
#define	RAISE					(4095.0f)
#define	RAISE_SHIFT_WORLD		(100.0f)

#define	ANGLE_WIDTH_LIMIT	(64)
#define	ANGLE_ADD		(1)

#define	DUMMY_SIZE		(0)
#define	DUMMY_COLOR		(32)
#define	DUMMY_ALPHA		(0)

/* コーン */
#define	N_PRIMS			(1)
#define	N_VERTS			(32)
#define	N_VP			(N_PRIMS * N_VERTS)


/* 中心から円錐ライトの最遠中心まで */
#define	LIGHT_LEN		(3000)

#define	LIMIT_ANGLE			(0.4f)
#define	RGB_BASE			(32)
#define	ALPHA_BASE			(96)
#define	ALPHA_MIN			(16)

#define	BAR_NUM			(3)

#define	FADE_TIME		(16)

typedef	struct	{
	int			r;
	int			g;
	int			b;
	int			count ;
	SVECTOR		rot ;
	SVECTOR		rot_add ;
	DG_PRIM2	*prim ;
} BAR ;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			prog_flag;
	int			flag;
	int			*on_off;

	int			scaling_alpha;
	int			scaling_alpha_mini;
	int			scaling_alpha_destination;
	int			scaling_count;

	FVECTOR		pos ;	/* 円錐の位置（外部計算用） */
	SVECTOR		rot ;	/* 円錐の方向（外部計算用） */
	DG_PRIM2	*prim ;	/* 光円錐 */

	BAR			bar[BAR_NUM];

} Work ;


static FVECTOR s_pos[N_VP];
static FVECTOR s_bar_pos[N_VP];

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;
	SVECTOR	svtemp;
	FVECTOR	cam_pos;
	float	inner;
	int	itemp;
	int	alpha;
	int	alpha_mini;
	int	clock;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh_before ;	/* スプライト用 */

	if( work->scaling_count >= 0 ){
		if( work->scaling_alpha_destination==0 ){
			work->scaling_alpha      = ALPHA_BASE * work->scaling_count / FADE_TIME;
			work->scaling_alpha_mini = ALPHA_MIN  * work->scaling_count / FADE_TIME;
			if( work->scaling_count==0 ) work->flag = 0;
		}else{
			work->scaling_alpha      = ALPHA_BASE * (FADE_TIME - work->scaling_count) / FADE_TIME;
			work->scaling_alpha_mini = ALPHA_MIN  * (FADE_TIME - work->scaling_count) / FADE_TIME;
		}
		work->scaling_count--;
	}

	if( work->prog_flag ){ // プログラム起動
		if( work->on_off != NULL ) work->flag = *work->on_off;
	}else{ // シナリオ起動
		{/* メッセージチェック */
			GV_MSG *msg;
			int mes_num;
			mes_num=GV_ReceiveMessage( work->name, &msg );
			msg += mes_num-1;
			while( --mes_num >= 0 ){
				itemp = msg->message[0];
				if( itemp==2  &&  work->flag!=0 ){	//ゆっくりＯＦＦ
					work->scaling_alpha_destination = 0;
					work->scaling_count = FADE_TIME;
					GM_SeSetMode( SD_A_PROJOFF1, &work->pos, GM_SEMODE_BOMB ) ;//プロジェクタ消灯//
				}else if( itemp==3  &&  work->flag!=1 ){	//ゆっくりＯＮ
					work->scaling_alpha_destination = 1;
					work->scaling_count = FADE_TIME;
					work->flag = 1;
					GM_SeSetMode( SD_A_PROJCHUG, &work->pos, GM_SEMODE_BOMB ) ;//プロジェクタ点灯//projchng 587
				}else{
//printf("ok:%d:%d\n",work->flag,itemp);
					if( itemp==0 ){
//printf("ok:a\n");
						GM_SeSetMode( SD_A_PROJOFF1, &work->pos, GM_SEMODE_BOMB ) ;//プロジェクタ消灯//
					}else{
//printf("ok:b\n");
						GM_SeSetMode( SD_A_PROJCHUG, &work->pos, GM_SEMODE_BOMB ) ;//プロジェクタ点灯//projchng 587
					}
					work->flag = itemp ;
				}
				msg--;
			}
		}
	}

//	GV_CallChildSignalFunc( work, 0, work->scaling_alpha * 4096 / ALPHA_BASE );

//printf("%d:%d:%d:\n",work->flag,work->scaling_count,work->scaling_alpha);

	if( work->flag == 0 || (work->scaling_alpha==0 && work->scaling_alpha_mini==0)){
		DG_InvisiblePrim2( work->prim ) ;
		for( i=0; i<BAR_NUM; i++ ){
			DG_InvisiblePrim2( work->bar[i].prim ) ;
		}
		return;
	}else if( work->flag == -1 ){
		DG_InvisiblePrim2( work->prim ) ;
		for( i=0; i<BAR_NUM; i++ ){
			DG_InvisiblePrim2( work->bar[i].prim ) ;
		}
		GV_DestroyActor( work ) ;
		return;
	}else if( work->flag == 1 ){
		DG_VisiblePrim2( work->prim ) ;
		for( i=0; i<BAR_NUM; i++ ){
			DG_VisiblePrim2( work->bar[i].prim ) ;
		}
		GM_GroupPrim2( work->prim,       work->map ) ;

		GM_SeSetMode( SD_A_PROJMOTO, &work->pos, GM_SEMODE_NORMAL ) ;//プロジェクタ駆動音(18fps周期で連)//projmoto 585
	}


	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );

	/* 光線の方向ベクトルを生成 */
	fvtemp0.vx = 0.0f;
	fvtemp0.vy = 0.0f;
	fvtemp0.vz =-1.0f;
	DG_SetPos2( &DG_ZeroVector, &work->rot );
	DG_RotVector( &fvtemp0, &fvtemp0, 1 );

	/* カメラからライトへのベクトルと、ライトのベクトルの内積 */
	_sceVu0SubVector( &fvtemp1, &work->pos, &cam_pos ) ;
	_sceVu0Normalize( &fvtemp1, &fvtemp1 );
	inner = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );
//	if( inner < LIMIT_ANGLE ) inner = LIMIT_ANGLE;
	if( inner < 0.0f ) inner = 0.0f;

	/* 懐中電灯の光線の方向ベクトルを */
	/* カメラ行列と乗算（乗算結果のＺ成分がライト方向のカメラ法線への成分となる） */
	DG_SetPos( &DG_Chanls->eye_inv );
	DG_RotVector( &fvtemp0, &fvtemp0, 1 );
//	if( fvtemp0.vz < LIMIT_ANGLE ) fvtemp0.vz = LIMIT_ANGLE;
	if( fvtemp0.vz < 0.0f ) fvtemp0.vz = 0.0f;

//printf("%f %f\n",inner,fvtemp0.vz);
	inner *= fvtemp0.vz;
//	inner = (inner - LIMIT_ANGLE) / (1.0f - LIMIT_ANGLE);	/* 比に変換 */



//	if( inner < LIMIT_ANGLE ) inner = LIMIT_ANGLE;


	alpha      = (int)((float)work->scaling_alpha      * inner);
	alpha_mini = (int)((float)work->scaling_alpha_mini);

	if( alpha < 0 ) alpha = 0;

   //AR_PARTICLE_HALF
	if(!DG_SwitchBuffPrim2( work->prim ))
   {
      return;
   }
	clock = work->prim->buffer_clock;

	/* カメラ方向にシフトさせて優先問題を回避 */
	fvtemp0.vx=0.0f;
	fvtemp0.vy=0.0f;
	fvtemp0.vz=RAISE_SHIFT_WORLD;
	OK_DirVecXY( &work->pos, &cam_pos, &svtemp );
	DG_SetPos2( &work->pos, &svtemp );
	DG_PutVector( &fvtemp0, &fvtemp0, 1 );


	//---------------
	DG_SetPos2( &fvtemp0, &work->rot );
	DG_PutVector( s_pos, work->prim->pos[clock], N_VP );
	uvrgbwh = work->prim->uvrgb[clock];
	for( j=0; j<N_VP; j++ ){
		uvrgbwh->w = uvrgbwh->h = j*30 + 100;
		uvrgbwh->a = alpha * (N_VP-j) / N_VP + alpha_mini;
		uvrgbwh++;
	}

	//---------------
	for( i=0; i<BAR_NUM; i++ ){
		work->bar[i].rot.vx += work->bar[i].rot_add.vx;
		work->bar[i].rot.vy += work->bar[i].rot_add.vy;
		work->bar[i].rot.vz += work->bar[i].rot_add.vz;
		if((irnd()>>8)%256==0){
			CVECTOR	col;
//			work->bar[i].rot.vx = work->rot.vx + (irnd()>>8)%(ANGLE_WIDTH_LIMIT*2)-ANGLE_WIDTH_LIMIT;
//			work->bar[i].rot.vy = work->rot.vy + (irnd()>>8)%(ANGLE_WIDTH_LIMIT*2)-ANGLE_WIDTH_LIMIT;
			uvrgbwh = work->bar[i].prim->uvrgb[clock];
			col.r = RGB_BASE + (irnd()>>8)%16-8;
			col.g = RGB_BASE + (irnd()>>8)%16-8;
			col.b = RGB_BASE + (irnd()>>8)%16-8;
			for( j=0; j<N_VP; j++ ){
				uvrgbwh->r = col.r;
				uvrgbwh->g = col.g;
				uvrgbwh->b = col.b;
				uvrgbwh++;
			}
		}else{
			uvrgbwh        = work->bar[i].prim->uvrgb[clock];
			uvrgbwh_before = work->bar[i].prim->uvrgb[1-clock];
			for( j=0; j<N_VP; j++ ){
				uvrgbwh->r = uvrgbwh_before->r;
				uvrgbwh->g = uvrgbwh_before->g;
				uvrgbwh->b = uvrgbwh_before->b;
				uvrgbwh++;
				uvrgbwh_before++;
			}
		}
		itemp = work->bar[i].rot.vx - work->rot.vx;
		if( (itemp > ANGLE_WIDTH_LIMIT)
		 || (itemp <-ANGLE_WIDTH_LIMIT)
		 || (irnd()%32==0) ){
			work->bar[i].rot_add.vx *= -1;
		}
		itemp = work->bar[i].rot.vy - work->rot.vy;
		if( (itemp > ANGLE_WIDTH_LIMIT)
		 || (itemp <-ANGLE_WIDTH_LIMIT)
		 || (irnd()%32==0) ){
			work->bar[i].rot_add.vy *= -1;
		}
		DG_SetPos2( &fvtemp0, &work->bar[i].rot );
		DG_PutVector( s_bar_pos, work->bar[i].prim->pos[clock], N_VP );
		uvrgbwh = work->bar[i].prim->uvrgb[clock];
		for( j=0; j<N_VP; j++ ){
			uvrgbwh->w = uvrgbwh->h = j*25 + 100;
			uvrgbwh->a = alpha * (N_VP-j) / N_VP + alpha_mini;
			uvrgbwh++;
		}
	}

}

static void Die( Work *work )
{
	int	i;

	work->prim = OK_FreePrim2( work->prim );
	for( i=0; i<BAR_NUM; i++ ){
		work->bar[i].prim = OK_FreePrim2( work->bar[i].prim );
	}
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise=RAISE;

	//-------------------------------
	for ( i=0; i<2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		for ( j=0; j<N_PRIMS; j++ ){
			for ( k=0; k<N_VERTS; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h = uvrgbwh->w = DUMMY_SIZE ;
				uvrgbwh->r = uvrgbwh->g = uvrgbwh->b = RGB_BASE ;
				uvrgbwh->a = DUMMY_ALPHA ;
				uvrgbwh ++ ;
			}
		}
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

}

/* ---------------------------------------------------------------- */
static void InitPacketBar( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;
	int		r;
	int		g;
	int		b;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE;

	r = RGB_BASE + (irnd()>>8)%16;
	g = RGB_BASE + (irnd()>>8)%16;
	b = RGB_BASE + (irnd()>>8)%16;
	//-------------------------------
	for ( i=0; i<2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		for ( j=0; j<N_PRIMS; j++ ){
			for ( k=0; k<N_VERTS; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h = uvrgbwh->w = DUMMY_SIZE ;
				uvrgbwh->r = r;
				uvrgbwh->g = g;
				uvrgbwh->b = b;
				uvrgbwh->a = DUMMY_ALPHA ;
				uvrgbwh ++ ;
			}
		}
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
}

static int GetResources( Work *work )
{
	int	i;
	FVECTOR	fvtemp;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->scaling_alpha      = ALPHA_BASE;
	work->scaling_alpha_mini = ALPHA_MIN;
	work->scaling_count = -1;

	/* 設置座標 */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		work->pos.vx = (float)GCL_GetNextInt() ;
		work->pos.vy = (float)GCL_GetNextInt() ;
		work->pos.vz = (float)GCL_GetNextInt() ;
		work->pos.vw = 1.0f ;
	}else{
		printf("GCL:ERR!::c_light_spot::set option P\n");
		return -1;
	}
	/* スポットライト方向 */
	if ( GCL_GetOption( 'd' ) != NULL ) {
		fvtemp.vx = (float)GCL_GetNextInt() ;
		fvtemp.vy = (float)GCL_GetNextInt() ;
		fvtemp.vz = (float)GCL_GetNextInt() ;
		fvtemp.vw = 0.0f ;
		OK_DirVecXY( &work->pos, &fvtemp, &work->rot );
	}else{
		printf("GCL:ERR!::c_light_spot::set option D\n");
		return -1;
	}

	work->flag=1;	/* 表示 */

	//-----------------------------
//	tex = DG_GetTexture( 12296685 /*"light10_msk"*/ );
	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex );



	//-----------------------------
	for( i=0; i<BAR_NUM; i++ ){
		prim = work->bar[i].prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacketBar( work, prim, tex );

		work->bar[i].rot.vx = work->rot.vx + irnd()%ANGLE_WIDTH_LIMIT - ANGLE_WIDTH_LIMIT/2;
		work->bar[i].rot.vy = work->rot.vy + irnd()%ANGLE_WIDTH_LIMIT - ANGLE_WIDTH_LIMIT/2;
		work->bar[i].rot.vz = irnd()%4096;

		work->bar[i].rot_add.vx = irnd()%ANGLE_ADD+1;
		work->bar[i].rot_add.vy = irnd()%ANGLE_ADD+1;
		if( irnd()%2==0 ){
			work->bar[i].rot_add.vz = (irnd()%ANGLE_ADD+1);
		}else{
			work->bar[i].rot_add.vz =-(irnd()%ANGLE_ADD+1);
		}
	}



	for ( i = 0 ; i < N_VP ; i++ ){
		s_pos[i].vx=0.0f;
		s_pos[i].vy=0.0f;
		s_pos[i].vz=(float)i * (float)LIGHT_LEN / (float)(N_VP-1);

		s_bar_pos[i].vx=0.0f;
		s_bar_pos[i].vy=0.0f;
		s_bar_pos[i].vz=(float)i * (float)LIGHT_LEN * 0.95f / (float)(N_VP-1);
	}

#if 0
{
	extern void *NewDustCone( FVECTOR *center, FVECTOR *direction, float min_len, float max_len, float max_radius, int col );
	FVECTOR	fvtemp;
	void	*p0;
	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = 1.0f;
	DG_SetPos2( &DG_ZeroVector, &work->rot );
	DG_RotVector( &fvtemp, &fvtemp, 1 );

	p0 = NewDustCone( &work->pos, &fvtemp, 1000.0f, 10000.0f, 2000.0f, 0x80808030 );
	if(p0)GV_SetActorChild( work, p0 );
}
#endif

	return 0 ;
}

void *NewProjectorLight( int name, int where )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->prog_flag = 0;	// シナリオ起動
		work->on_off    = NULL;
		work->name      = name;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/*
on_off : 0 : 非表示
on_off : 1 :   表示
on_off :-1 : デストロイ
*/
void *NewProjectorLight_P( int *on_off )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->prog_flag = 1;	// プログラム起動
		work->on_off    = on_off;
		work->name      = 0;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
