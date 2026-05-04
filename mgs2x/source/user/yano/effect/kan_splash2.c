//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	kan_splash2.c
	空き缶から出る水飛抹 水面版
	2001/06/27 Y.Yano
	$Id: kan_splash2.c,v 1.1.1.3 2002/11/19 11:51:57 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../../okajima/etc/ok_util.h"

#define SCR_TEST    1 /*1->スクラッチパッド使用*/
#define SCR_POS     1 /*1->スクラッチパッド不使用*/

#define N_PRIMS		(8)
#define N_VERTS		(32)

#define BIT_SPLASH_STOP  (0x0002)
#define BIT_SPLASH_STOP2 (0x0004)

#define RIPPLE_MASK 	(0x0041)
#define RIPPLE_SPACE 	(300.0F)

/*nまでの乱数を出す*/
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )

extern void UTL_StartSprToMem( void *dst, void *src, int size );
extern void UTL_EndSprToMem( void );
extern void UTL_StartMemToSpr( void *dst, void *src, int size );
extern void UTL_EndMemToSpr( void );
extern void GTE_MakeRotateAxis( FMATRIX *res, FVECTOR *axis, float angle );

typedef struct {
	FVECTOR 		position[N_PRIMS*N_VERTS] ;
	DG_PRIM2_UVRGBWH	_uvrgbwh[N_PRIMS*N_VERTS] ;
} ScrWork;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	int			name;
	int			where;
    FMATRIX		*kan_world ;/*参照用*/
	FMATRIX		kan_world2 ;/*記憶用*/

	FVECTOR 	position[N_PRIMS*N_VERTS] ;
	DG_PRIM2_UVRGBWH	_uvrgbwh[N_PRIMS*N_VERTS] ;

	DG_PRIM2	*prim ;
	FVECTOR		center ;
	FVECTOR		offset ;
	FVECTOR		vec[ N_PRIMS * N_VERTS ];

    int now_num ;
	int flag ;
    int verts_flag[ N_PRIMS ];	
	int stop_msg ;
	int time_cnt ;

    FVECTOR last_ripple ;

} Work ;
/*----------------------------------------------------------------*/

static void YN_ScrMem( void *dst , void *scr , int size , int num )
{
	UTL_StartSprToMem( dst, scr, size * num / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}
static void YN_MemScr( void *dst , void *scr , int size , int num )
{
	UTL_StartMemToSpr( dst, scr, size * num / sizeof(u_long128) );
	UTL_EndMemToSpr() ;
}

static int CheckLastRipple( Work *work , FVECTOR *pos )
{
	FVECTOR *last = &work->last_ripple ;
	int flag = 0 ;
	
	if( pos->vx < last->vx + RIPPLE_SPACE && pos->vx > last->vx - RIPPLE_SPACE ){
		flag |= 0x01 ;
	}
	if( pos->vz < last->vz + RIPPLE_SPACE && pos->vz > last->vz - RIPPLE_SPACE ){
		flag |= 0x02 ;
	}
	
	if( flag == 0x03 ){/*範囲内*/
		return 0 ;
	}else{             /*  外  */
		return 1 ;
	}
}

static void InitPrim( Work *work , FMATRIX *kan_world )
{
	int i ;
	int now_num	= work->now_num ;
	int clock   = work->prim->buffer_clock ;
	ScrWork *scrpad = ( ScrWork * )SCRPAD_ADDR;
	FVECTOR          *pos = scrpad->position ;
#if SCR_TEST
	DG_PRIM2_UVRGBWH *uvrgb = scrpad->_uvrgbwh ;
#else
	DG_PRIM2_UVRGBWH *uvrgb = (DG_PRIM2_UVRGBWH*)work->prim->uvrgb[clock]
	    + now_num*N_VERTS ;
#endif
	FVECTOR          *vec = &work->vec[ now_num * N_VERTS ];
	float ftemp , ftemp2 , ftemp3 , ftemp4 ;
	float angle ;
	FVECTOR fvtemp , fvtemp2 ;
	FVECTOR kan_pos , kan_pos2 , kankan ;
	SVECTOR svtemp ;
	int 	*v_flag = work->verts_flag ;
	FMATRIX fmatrix ;

	/*多重配列は重くなるのでFVECTORに*/
	DG_COPY_VEC( &kan_pos , (FVECTOR*)kan_world->m[3] );
	DG_COPY_VEC( &kan_pos2 , (FVECTOR*)work->kan_world2.m[3] );
	_sceVu0SubVector( &kankan , &kan_pos , &kan_pos2 );

	*(v_flag + now_num) = 0 ;/*頂点フラグ初期化*/
#if SCR_TEST
	/*スクラッチパッドにuvrgb情報をコピー*/	
	YN_MemScr( scrpad->_uvrgbwh ,
		   (DG_PRIM2_UVRGBWH*)work->prim->uvrgb[ clock ]
		   + now_num * N_VERTS ,
		   sizeof(DG_PRIM2_UVRGBWH) , N_VERTS );
#endif

	for( i = 0 ; i < N_VERTS ; i++ ){
		/* uvrgb */
	    if( i < 0  ){
			uvrgb->a = 0 ;
	    }else{
			uvrgb->a = 128 ;
			
		/* pos */
		DG_SetPos( work->kan_world );
		DG_PutVector( &work->offset , pos , 1 );
		pos->vy = GM_WaterLevel ;
	    }

	    /*初速ベクトル*/
		/* -1～1 の正規分布っぽい数字を出す */
		ftemp  = (float)RND( 2000 )*0.001F - 1.0F ;/* -1 ... 1 */
		ftemp2 = (float)RND( 2000 )*0.001F - 1.0F ;
		ftemp3 = (float)RND( 2000 )*0.001F - 1.0F ;
		ftemp = (ftemp + ftemp2)*0.5F ;
		
		kankan.vy = 0.0F ;
		_sceVu0Normalize( &fvtemp2 , &kankan );

	    angle       = (ftemp2+1.0F)*PI*0.25F ;
		ftemp4 = fvtemp2.vx ;
		fvtemp.vx = -fvtemp2.vz ;
		fvtemp.vz = ftemp4 ;
		fvtemp.vy = 0.0F ;
		_sceVu0Normalize( &fvtemp , &fvtemp );
		GTE_MakeRotateAxis( &fmatrix , &fvtemp , angle );
		DG_SetPos( &fmatrix );
		DG_RotVector( &fvtemp2 , &fvtemp2 , 1 );

		svtemp = DG_ZeroSVector ;
		svtemp.vy = (short)(ftemp * 2048.0F);
		DG_SetPos2( &DG_ZeroVector , &svtemp );
		DG_RotVector( &fvtemp2 , &fvtemp2 , 1 );
		ftemp3 = 40.0F * (ftemp3 + 1.0F);
		_sceVu0ScaleVectorXYZ( vec , &fvtemp2 , ftemp3 );

		/* etc. */
	    *(v_flag + now_num) |= (1 << i) ;

	    vec++ ;
	    uvrgb++ ;
	    pos++ ;
	}
	    
	YN_ScrMem( work->prim->pos[clock] + now_num * N_VERTS ,
		   scrpad->position ,
		   sizeof(FVECTOR) , N_VERTS );
#if SCR_TEST
	YN_ScrMem( (DG_PRIM2_UVRGBWH*)work->prim->uvrgb[clock]
		   + now_num * N_VERTS,
		   scrpad->_uvrgbwh ,
		   sizeof(DG_PRIM2_UVRGBWH) , N_VERTS ) ;
#endif

	work->kan_world2 = (*kan_world) ;
}

static	void	Act( Work *work )
{
    int		i,j;
    int 	checker , checker2 ;
    int		clock;
    DG_PRIM2_UVRGBWH	*uvrgbwh , *uvrgbwh_before ;
    FVECTOR				*pos , *pos_before;
    FVECTOR				*vec ;
    int 	*v_flag = work->verts_flag ;
    
    work->time_cnt++ ;
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }/* 0 <--> 1 バッファ切替え */

    clock          = work->prim->buffer_clock;
    vec            = work->vec;
    uvrgbwh        = work->prim->uvrgb[clock];
    uvrgbwh_before = work->prim->uvrgb[1-clock];
    pos            = work->prim->pos[clock];
    pos_before     = work->prim->pos[1-clock];
    
    checker2 = 0 ;
    for( i=0; i<N_PRIMS; i++ ){
		checker = 0 ;
		for( j=0; j<N_VERTS; j++ ){
			/*重力*/
			vec->vy += P_GRAVITY ;
			pos->vx  = pos_before->vx + vec->vx;
			pos->vy  = pos_before->vy + vec->vy;
			pos->vz  = pos_before->vz + vec->vz;
			//AN_Test_Eye2( pos, 2 );/*ポジションに色をつける*/

			(*uvrgbwh) = (*uvrgbwh_before) ;

			if( (*(v_flag + i)) & (1<<j) && pos->vy <= GM_WaterLevel - 5.0F ){
				uvrgbwh->a = 0 ;       /*不可視*/
				uvrgbwh_before->a = 0 ;/*      */
				(*(v_flag + i)) &= ~(1<<j) ;
				checker2++ ;
				if( checker2 & RIPPLE_MASK ){/*波紋呼び出し*/
					extern int OK_PutRipple( FVECTOR *center );
					if( CheckLastRipple( work , pos ) ){
						OK_PutRipple( pos );
						work->last_ripple = ( *pos );
					}
				}
			}
			
			pos++;
			pos_before++;
			uvrgbwh++;
			uvrgbwh_before++;
			vec++;
		}

		if( *(v_flag + i) == 0 ){
			/*全部見えなくなったら*/
			work->flag &= ~(1 << i) ;/*OFF*/
		}
    }

    if( work->stop_msg != 1 ){/*止まれといわれない限り出し続ける*/
		if( work->flag & (1 << (work->now_num)) ){
			/*現在の番号にフラグがたっている*/
			work->flag |= (1 << (work->now_num)) ;
		}else{
			work->flag |= (1 << (work->now_num)) ;
		}
		if( work->kan_world == NULL ){
		    GV_DestroyActor( work );
		}
		InitPrim( work , work->kan_world );
		work->now_num ++ ;
		if( work->now_num >= N_PRIMS ){
			work->now_num =0 ;
		}
    }

    if( work->flag == 0 ){/*すべてのフラグがOFFのとき*/
		GV_DestroyActor( work ) ;
    }
    
}

static void Die( Work *work )
{
    GM_FreePrim2( work->prim );
}


/*****************************************************************/
                   /*親からのシグナル受信*/
static int ReceiveSignal( void *workp, int signal, int value )
{
    Work *work = workp;
    switch( signal ){
      case GV_SIGNAL_KILL:
	//printf("缶飛抹死にます\n");
		GV_DefaultSignalFunc( work , signal , value );
		break;

      case BIT_SPLASH_STOP2:
	//printf("水面しぶきやめま～す\n");
		work->stop_msg = 1 ;
		break ;

      case BIT_SPLASH_STOP:
		/*kan_splash への信号なので無視*/
	//printf("「おれ関係ないし、、」by splash2\n");
		break ;
    }
    
    return 0;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, int name, int where , FMATRIX *kan_world )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	int i , k ;
	FVECTOR *pos ;
	FVECTOR *vec = work->vec;
	float angle , ftemp , ftemp2 , ftemp3 ,ftemp4 ;
	FMATRIX fmatrix ;
	FVECTOR fvtemp , fvtemp2 ;
	SVECTOR svtemp ;
	FVECTOR kan_pos , kan_pos2 , kankan ;

	if( kan_world == NULL ){
	    return (-1);
	}

	work->name  = name;
	work->where = where;
	work->kan_world  = kan_world ;
	work->kan_world2 = (*kan_world) ;

	/*親からのシグナル受信*/
	GV_SetActorSignalFunc( work, ReceiveSignal );

	/* 水飛抹の出る場所 */
	work->offset = DG_ZeroVector ;/*今は中心*/
	
	/* スプライト設定 */
	prim = work->prim = GM_MakePrim2(
			DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
			N_PRIMS , N_VERTS );
	if(prim==NULL){
		printf("null 'kan_splash' prim\n");
		return -1;
	}
	DG_VisiblePrim2( work->prim );
	tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );
	DG_ConfigPrim2Tex( prim, tex );
	prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;

	/*パケットの初期化*/
	uvrgbwh = work->prim->uvrgb[0] ;
	pos = work->prim->pos[0] ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
		    /* ＵＶ値は1.3.12の固定小数点なので注意！ */
		    uvrgbwh[k].u0 = FTOI12( 0.0F * tex->u_scale
					    + tex->u_offset ) ;/* 左上 */
		    uvrgbwh[k].v0 = FTOI12( 0.0F * tex->v_scale
					    + tex->v_offset ) ;/* 左上 */
		    uvrgbwh[k].u1 = FTOI12( 1.0F * tex->u_scale
					    + tex->u_offset ) ;/* 右下 */
		    uvrgbwh[k].v1 = FTOI12( 1.0F * tex->v_scale
					    + tex->v_offset ) ;/* 右下 */
		    uvrgbwh[k].q0 = 4096 ;
		    uvrgbwh[k].q1 = 4096 ;
		    uvrgbwh[k].f0 = 0x0fff ;
		    uvrgbwh[k].f1 = 0x0fff ;
		    /* ＷＨ値は整数なので注意！ */
		    uvrgbwh[k].w = RND(5)+4 ;/* 幅／２ */
		    uvrgbwh[k].h = RND(5)+4 ;/* 高さ／２ */
		    /* 色 */
		    uvrgbwh[k].r = 128 ;
		    uvrgbwh[k].g = 128 ;
		    uvrgbwh[k].b = 128 ;
		    uvrgbwh[k].a = 0 ;

		}
		uvrgbwh += N_VERTS ;
	}

	/*一回めのInitPrimを行なう*/
	/*多重配列は重くなるのでFVECTORに*/
	DG_COPY_VEC( &kan_pos , (FVECTOR*)kan_world->m[3] );
	DG_COPY_VEC( &kan_pos2 , (FVECTOR*)work->kan_world2.m[3] );
	_sceVu0SubVector( &kankan , &kan_pos , &kan_pos2 );


	for( i = 0 ; i < N_VERTS ; i++ ){
		/* uvrgb */
		uvrgbwh[i].a = 128 ;/*1パケット分だけ可視化*/
	
		/* pos */
		DG_SetPos( work->kan_world );
		DG_PutVector( &work->offset , pos , 1 );
		pos->vy = GM_WaterLevel ;
		angle = TPI / (float)N_VERTS *(float)i ;
		pos->vx += 70.0F * vu0_Cos( angle );
		pos->vz += 70.0F * vu0_Sin( angle );

	    /*初速ベクトル*/
		/* -1～1 の正規分布っぽい数字を出す */
		ftemp  = (float)RND( 2000 )*0.001F - 1.0F ;/* -1 ... 1 */
		ftemp2 = (float)RND( 2000 )*0.001F - 1.0F ;
		ftemp3 = (float)RND( 2000 )*0.001F - 1.0F ;
		ftemp = (ftemp + ftemp2)*0.5F ;
		
		kankan.vy = 0.0F ;
		_sceVu0Normalize( &fvtemp2 , &kankan );

		angle       = (ftemp2+1.0F)*PI*0.25F ;
		ftemp4 = fvtemp2.vx ;
		fvtemp.vx = -fvtemp2.vz ;
		fvtemp.vz = ftemp4 ;
		fvtemp.vy = 0.0F ;
		_sceVu0Normalize( &fvtemp , &fvtemp );
		GTE_MakeRotateAxis( &fmatrix , &fvtemp , angle );
		DG_SetPos( &fmatrix );
		DG_RotVector( &fvtemp2 , &fvtemp2 , 1 );

		svtemp = DG_ZeroSVector ;
		svtemp.vy = (short)(ftemp * 2048.0F);
		DG_SetPos2( &DG_ZeroVector , &svtemp );
		DG_RotVector( &fvtemp2 , &fvtemp2 , 1 );
		ftemp3 = 60.0F * (ftemp3 + 1.0F);
		_sceVu0ScaleVectorXYZ( vec , &fvtemp2 , ftemp3 );
				
		pos++ ;
		vec++ ;
	}
	/*バッファーの[1]にも同じものを一応入れておく*/	
	memcpy( prim->pos[1] , prim->pos[0] , sizeof(FVECTOR) * N_VERTS );
	memcpy( prim->uvrgb[1], prim->uvrgb[0] ,
			sizeof(DG_PRIM2_UVRGBWH) * N_VERTS * N_PRIMS ) ;
	
	return 0 ;
}


void *NewKanSplash2( int name, int where , FMATRIX *kan_world )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where , kan_world ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
