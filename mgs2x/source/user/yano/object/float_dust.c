//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	float_dust.c

	w31bステージの水を浮遊するゴミ
	
	2001/05/16 YUKO YANO
	$Id: float_dust.c,v 1.1.1.3 2002/11/19 11:52:01 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <stdarg.h>

#include "gameheader.h"

#define CTRL_FLAG		(CTRL_SKIP_CHANGE_MAP|CTRL_SKIP_GET_ADDRESS)/* source/game/g_struct.hを参照 */
#define BODY_FLAG		(DG_FLAG_ONEPIECE)	/* source/system/libdg/libdg.hを参照 */

#define DUST_FLAG_KAITEN	(0x01)
#define DUST_FLAG_YURERU	(0x02)

#define GURU_LEVEL		(10.0F)
#define CIRCLE_VALUE 		(2.0F * PI / 4096.0F)
#define MIGI 			1
#define HIDARI 			0
#define RAGI_1024		(1/PI * 1024.0F)
#define YUREHABA		(1.0F / 50.0F * 350.0F)
#define DEFENSE_SCALE    	( 500.0F )
#define MOVE_LEVEL    		( 10.0F )
#define YOYUU    		( 200.0F )
//#define YOYUU    ( 300.0F )

 /*nまでの乱数を出す*/
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/*-2047～2048の値に直す*/
#define REGULAR_ANGLE( _a )	( (signed short)( ((unsigned short)(_a)) << 4 ) >> 4 )
/*パチってきました*/
#define PRINT_PFVEC(_i,_fv) printf("[%d] vx %8.8f:vy %8.8f:vz %8.8f:vw %8.8f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

/*任意の軸で右回りに回転させるマトリクス取得*/
extern void GTE_MakeRotateAxis( FMATRIX *res, FVECTOR *axis, float angle );
extern void GTE_MakeRotate( FMATRIX *res, float x, float y, float z, float angle );

extern float YN_VectorLength( FVECTOR *in );
extern float YN_VectorLenXZ( FVECTOR *in );
extern void *NewBloodWater_Test( FMATRIX*, FVECTOR*, FVECTOR*, float, int );

/* プログラムワーク */
typedef	struct	{
    GV_ACT_EX	actor ;
    CONTROL	control ;		/* コントロール */
    OBJECT	object ;		/* オブジェクト */
    FMATRIX	light[2] ;		/* ライトマトリクス設定用 */
    int name ;/*マップ*/

    int mode ;/*中身が出る用のモデル識別*/

    FVECTOR     wide ;/*centerから、trapのはじっこまでのベクトル*/
    FVECTOR     center ;/*trapのセンター*/
    FVECTOR	updown ;
    FVECTOR	b_PlayerPosi ;
    int 	kan_flag ;
    int 	Guru_step , Yura_step ;
    FVECTOR	diff_vector , p_vector;
    float       diff_vector_len ;
    float	p_vector_len ;
    FVECTOR 	out ;
    FMATRIX 	rotate ;
    FVECTOR    	step_save ;

    /*DustGuruY*/
    short 	degree ;
    int 	Guru_cnt ;
    float 	temp_degree ;
    FVECTOR   	jiku ;
    int 	norm_cnt ;

    /*DustRandom*/
    int		rand_cnt ;
    float	rand_length ;
    float 	rand_sin ,rand_cos ;
    float       rand_y;

    /*DustDefense*/
    float  	diff_vecXZ_len ;
    float 	def_vx_save , def_vz_save ;
    
} Work ;

enum {/*DustGuruのstep文で使用*/
	DG_EXIT = 0 ,
	DG_INIT ,
	DG_GURURI 
};

//static void matrix_dump( FMATRIX * matrix ){
//    printf("%f %f %f %f \n" , matrix->m[0][0] , matrix->m[0][1] , matrix->m[0][2] , matrix->m[0][3] );
//    printf("%f %f %f %f \n" ,matrix->m[1][0] ,matrix->m[1][1] ,matrix->m[1][2] ,matrix->m[1][3] );
//    printf("%f %f %f %f \n" ,matrix->m[2][0] ,matrix->m[2][1] ,matrix->m[2][2] ,matrix->m[2][3] );
//    printf("%f %f %f %f \n" ,matrix->m[3][0] ,matrix->m[3][1] ,matrix->m[3][2] ,matrix->m[3][3] );
//}
//static void FvectorDump( FVECTOR * vector ){
//    printf("%f %f %f \n" , vector->vx , vector->vy , vector->vz );
//}

/***********************************************************************/
                         /*3D回転*/
/***********************************************************************/
static int DustGuruXYZ( Work *work , int step , float add_degree )
{
    short damy ;
    FVECTOR t1 ,t2 , b_jiku ;
    float jiku_len , fdamy;
    FMATRIX b_rotate ;

    b_rotate = work->rotate ;
    
    switch( step ){
      case DG_INIT:
		b_jiku = work->jiku ;
		/*回転軸を決める*/
		_sceVu0AddVector( &t1 , &work->p_vector , &work->diff_vector ) ;
		_sceVu0AddVector( &t2 , &work->b_PlayerPosi , &work->diff_vector ) ;
		_sceVu0OuterProduct( &t2 , &t1 , &t2) ;
		_sceVu0OuterProduct( &work->jiku , &t1 , &t2) ;
		
		_sceVu0AddVector( &work->jiku , &work->jiku , &b_jiku );/*前回のと軸を足したもの*/
		jiku_len = work->jiku.vx*work->jiku.vx + work->jiku.vy*work->jiku.vy
			+ work->jiku.vz*work->jiku.vz ;
		jiku_len = bp_sqrtf( jiku_len ) ;   //BP_MATH - emulate PS2 sqrtf

		/*回転量決定*/
		work->degree = (short)(work->p_vector_len / work->diff_vector_len * 10000.0F) ;
		if( work->degree > 10 ){
			work->degree = 10 ;
			work->temp_degree = 10.0F ;
		}
		step = DG_GURURI ;
		break;
      case DG_GURURI:
		damy = work->degree ;
		work->temp_degree += add_degree ;
		work->degree = (short)work->temp_degree ;
		if(work->degree * damy <= 0 ){
			step = DG_EXIT ;
			work->kan_flag &= (~DUST_FLAG_KAITEN) ;/*回転フラグOFF*/
			work->degree = 0 ;
		}
		break;
    }
    fdamy = REGULAR_ANGLE( work->degree ) / 2048.0F * PI ;

    /*回転マトリクス取得*/
    _sceVu0Normalize( &work->jiku , &work->jiku ) ;/*正規化*/
    GTE_MakeRotateAxis( &work->rotate , &work->jiku , fdamy ) ;
    _sceVu0MulMatrix( &work->rotate , &work->rotate , &b_rotate) ;

    /*マトリクスも正規化すること～↓*/
    if( work->norm_cnt >= 600 ){
		_sceVu0Normalize( (FVECTOR*)&(work->rotate.m[0][0]) , (FVECTOR*)&(work->rotate.m[0][0]) );
		_sceVu0Normalize( (FVECTOR*)&(work->rotate.m[1][0]) , (FVECTOR*)&(work->rotate.m[1][0]) );
		_sceVu0Normalize( (FVECTOR*)&(work->rotate.m[2][0]) , (FVECTOR*)&(work->rotate.m[2][0]) );
		work->norm_cnt = 0 ;
    }
    work->norm_cnt ++ ;
    
    return step ;
}


/*********************************************************************/
                 /*  PLAYERに対する人形の動き */
/*********************************************************************/
static void DustPlayer( Work *work , CONTROL *ctrl )
{
    float temp ;
    
    if( work->diff_vector_len <= 2000.0F ){
		/*playerの移動ベクトル*/
#if 0
		work->p_vector.vx = work->b_PlayerPosi.vx - GM_PlayerPosition.vx ;
		work->p_vector.vy = work->b_PlayerPosi.vy - GM_PlayerPosition.vy ;
		work->p_vector.vz = work->b_PlayerPosi.vz - GM_PlayerPosition.vz ;
		temp = (work->p_vector.vx)*(work->p_vector.vx)
			+ (work->p_vector.vy)*(work->p_vector.vy)
			+ (work->p_vector.vz)*(work->p_vector.vz);
		work->p_vector_len = bp_sqrtf(temp);/*距離*/ //BP_MATH - emulate PS2 sqrtf
#else
		_sceVu0SubVector( &work->p_vector , &work->b_PlayerPosi , &GM_PlayerPosition );
		work->p_vector_len = YN_VectorLength( &work->p_vector );
#endif
		/*外積を求めて、進行方向と垂直なベクトルをモデルに与える*/
		if( work->p_vector_len > MOVE_LEVEL ){
			FVECTOR t1 ,t2 ;
			float out_len , force ;
			
			_sceVu0AddVector( &t1 , &GM_PlayerPosition , &work->b_PlayerPosi );
			_sceVu0AddVector( &t2 , &ctrl->mov , &work->b_PlayerPosi );
			_sceVu0OuterProduct( &t2 , &t1 , &t2 );
			_sceVu0OuterProduct( &work->out , &t1 , &t2 );
#if 0
			out_len = work->out.vx*work->out.vx + work->out.vy*work->out.vy + work->out.vz*work->out.vz ;
			out_len = bp_sqrtf( out_len );   //BP_MATH - emulate PS2 sqrtf
#else
			out_len = YN_VectorLength( &work->out );
#endif
			force = 1 / work->diff_vector_len * 10000.0F ;
			if( force > 20.0F ){
				force = 20.0F ;
			}
			ctrl->step.vx = - work->out.vx / out_len * force ;
			ctrl->step.vy = - work->out.vy / out_len * force ;
			ctrl->step.vz = - work->out.vz / out_len * force ;
			work->step_save = ctrl->step ;
		}else {/*止まっている時*/
			float step_len , step_unit ;

			step_len = work->step_save.vx*work->step_save.vx + work->step_save.vy*work->step_save.vy
				+ work->step_save.vz*work->step_save.vz ;
			step_len = bp_sqrtf( step_len ) ;   //BP_MATH - emulate PS2 sqrtf
			step_unit = 1 / step_len ;
			work->step_save.vx -= work->step_save.vx * step_unit * 0.1F ;
			work->step_save.vy -= work->step_save.vy * step_unit * 0.1F ;
			work->step_save.vz -= work->step_save.vz * step_unit * 0.1F ;
			if(work->step_save.vx * ctrl->step.vx < 0.0F){/*速度が逆になった時*/
				ctrl->step.vx = 0.0F ;/*stepをクリア*/
				ctrl->step.vy = 0.0F ;
				ctrl->step.vz = 0.0F ;
			}else {
				ctrl->step = work->step_save ;
			}
		}
	    
		/*大きく動いた時、回して、揺らす*/
		if(work->p_vector_len > GURU_LEVEL){
			/*回転*/
			work->Guru_step = DG_INIT ;
			work->kan_flag |= DUST_FLAG_KAITEN ;/*回転フラグON*/
			DustGuruXYZ( work ,DG_INIT , -0.1F ) ;
	    
			if( work->mode == 1 || work->mode == 2 ){/*ペットボトルの大小*/
				FVECTOR p = { 0.0F , 0.0F , 0.0F , 1.0F } , v = { 0.0F , 1.0F , 0.0F , 0.0F } ;
				float pow = 0.0F ;

				if( work->mode == 1 ){
					pow = 50.0F ;
				}
				if( work->mode == 2 ){
					pow = 10.0F ;
				}
		
				/*中身が出る*/
				NewBloodWater_Test( &work->object.objs->world , &p , &v , pow , work->mode ) ;
			}

		} else{
			if(work->kan_flag & DUST_FLAG_KAITEN){/*回転フラグチェック*/
				DustGuruXYZ( work , DG_GURURI , -0.1F ) ;
			}
		}
	
    }else {/*スケール外*/
		float step_len , step_unit ;

		step_len = work->step_save.vx*work->step_save.vx + work->step_save.vy*work->step_save.vy
			+ work->step_save.vz*work->step_save.vz ;
		step_len = bp_sqrtf( step_len ) ;   //BP_MATH - emulate PS2 sqrtf
		if ( step_len == 0.0f ) step_len = 0.00001f ; // T.Morita Added 2002.03.01
		step_unit = 1 / step_len ;
		work->step_save.vx -= work->step_save.vx * step_unit * 0.1F ;
		work->step_save.vy -= work->step_save.vy * step_unit * 0.1F ;
		work->step_save.vz -= work->step_save.vz * step_unit * 0.1F ;
		if(work->step_save.vx * ctrl->step.vx < 0.0F){/*速度が逆になった時*/
			ctrl->step.vx = 0.0F ;/*stepをクリア*/
			ctrl->step.vy = 0.0F ;
			ctrl->step.vz = 0.0F ;
		}else {
			ctrl->step = work->step_save ;
		}
    
		if(work->kan_flag & DUST_FLAG_KAITEN){/*回転フラグチェック*/
			DustGuruXYZ( work , DG_GURURI , -0.1F ) ;
		}
    }

    /*playerの場所を記憶*/
#if 0 
    work->b_PlayerPosi.vx = GM_PlayerPosition.vx ;
    work->b_PlayerPosi.vy = GM_PlayerPosition.vy ;
    work->b_PlayerPosi.vz = GM_PlayerPosition.vz ;
#else
	work->b_PlayerPosi = GM_PlayerPosition ;
#endif
	
    return ;
}

/*************************************************************************/
                          /* ふらふら */
/*************************************************************************/
/*加速度の長さ、角度ともにランダム*/
static void DustRandom( Work *work , CONTROL *ctrl )
{
    float theta ;

    if( work->rand_cnt >= 600 ){
		work->rand_length = (float)RND(10000)/ 5000.0F ;
		theta = RND(4095);
		work->rand_sin = sinf(theta / 4096.0F * TPI) ;
		work->rand_cos = cosf(theta / 4096.0F * TPI) ;
		work->rand_y = (float)RND(10000)/10000.0F - 0.5F ;/*-0.5～0.5Fまで*/
		work->rand_cnt = 0 ;
    }else {
		work->rand_cnt ++ ;
    }
    ctrl->step.vx += work->rand_length * 0.5F * work->rand_sin ;
    ctrl->step.vz += work->rand_length * 0.5F * work->rand_cos ;
    ctrl->step.vy += work->rand_y ;
}

/********************************************************************/
          /* モデルがプレイヤーにめりこむのを防ぐ関数 */
/********************************************************************/
static void DustDefense( Work *work , CONTROL *ctrl )
{

	float def_vx , def_vz , sin_th , cos_th ;
	float len ;
	
	if (work->diff_vecXZ_len <= DEFENSE_SCALE){
	    sin_th = work->diff_vector.vz / work->diff_vecXZ_len;
	    cos_th = work->diff_vector.vx / work->diff_vecXZ_len;
	    if( DEFENSE_SCALE - work->diff_vecXZ_len  > 5.0F ){
			len = 5.0F ;
	    }else {
			len = DEFENSE_SCALE - work->diff_vecXZ_len ;
	    }
	    def_vx = len * cos_th ;
	    def_vz = len * sin_th ;
	    
	}else{
	    def_vx = 0.0F ; 
	    def_vz = 0.0F ;
	}
#if 0
	work->def_vx_save = work->def_vx_save - work->def_vx_save*0.5F ;/*慣性を持たせる。*/
	work->def_vz_save = work->def_vz_save - work->def_vz_save*0.5F ;
	ctrl->step.vx += ( def_vx + work->def_vx_save );
	ctrl->step.vz += ( def_vz + work->def_vz_save );
	work->def_vx_save = ( def_vx + work->def_vx_save ) ;
	work->def_vz_save = ( def_vz + work->def_vz_save ) ;
#else
	ctrl->step.vx += def_vx ;
	ctrl->step.vz += def_vz ;
#endif
}

/********************************************************************/
                /* トラップを自分で見る*/
static int DustCheckBoundary( Work * work , CONTROL *ctrl )
{
    int trap_flag = 0 ;
    
    if( ctrl->mov.vx > work->wide.vx + work->center.vx - YOYUU
		|| ctrl->mov.vx < -(work->wide.vx) + work->center.vx + YOYUU ){
		trap_flag |= 0x01 ;
    }
    if( ctrl->mov.vy > work->wide.vy + work->center.vy - YOYUU
		|| ctrl->mov.vy < -(work->wide.vy) + work->center.vy + YOYUU){
		trap_flag |= 0x02 ;
    }
    if( ctrl->mov.vz > work->wide.vz + work->center.vz - YOYUU
		|| ctrl->mov.vz < -(work->wide.vz) + work->center.vz + YOYUU){
		trap_flag |= 0x04 ;
    }
	
    return trap_flag ;
}

static void InvPaparObj( Work *work )
{
	OBJECT	*dust = &work->object ;
	
	DG_InvisibleObjs( dust->objs );
}

static void VisPaparObj( Work *work )
{
	OBJECT	*dust = &work->object ;
	
	DG_VisibleObjs( dust->objs );
}

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num = GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			VisPaparObj( work );
			break;
		  case 1:
			InvPaparObj( work );
			return 1;
			break;
		  default:
			break;
		}
		msg--;
	}

	return 0;
}


/* ---------------------------------------------------------------- */
/* アクターメイン処理 */
static void Act( Work *work )
{
    CONTROL		*ctrl ;
    OBJECT		*body ;
    float		diff ;
    FVECTOR    	save_mov ;
    float 		temp ;
    int 		trap_flag ;

    /*Message*/
    if( CheckMesgParam( work ) ){
		GV_WaitMessage( work, work->name );
		return;
    }

    ctrl = &work->control;
    body = &work->object ;

    save_mov = ctrl->mov ;/*前回の位置記憶*/

    /*playerから人形へのベクトル*/
#if 0 
    work->diff_vector.vx = ctrl->mov.vx - GM_PlayerPosition.vx ;
    work->diff_vector.vy = ctrl->mov.vy - GM_PlayerPosition.vy ;
    work->diff_vector.vz = ctrl->mov.vz - GM_PlayerPosition.vz ;
	temp = (work->diff_vector.vx)*(work->diff_vector.vx)
		+(work->diff_vector.vy)*(work->diff_vector.vy)
		+ (work->diff_vector.vz)*(work->diff_vector.vz) ;
    work->diff_vector_len = bp_sqrtf(temp);/*距離*/   //BP_MATH - emulate PS2 sqrtf
    temp = (work->diff_vector.vx)*(work->diff_vector.vx)
		+ (work->diff_vector.vz)*(work->diff_vector.vz) ;
    work->diff_vecXZ_len = bp_sqrtf(temp) ;  //BP_MATH - emulate PS2 sqrtf
#else
	_sceVu0SubVector( &work->diff_vector , &ctrl->mov , &GM_PlayerPosition );
	work->diff_vector_len = YN_VectorLength( &work->diff_vector );
	work->diff_vecXZ_len  = YN_VectorLenXZ( &work->diff_vector );
#endif 
    diff =  GM_PlayerPosition.vy -  ctrl->mov.vy ;
    if( diff < 1000.0F || diff > -1000.0F ){/*Y軸に対して範囲内にいる時*/
		DustPlayer( work , ctrl ) ;/*プレイヤーから逃げる缶*/
    }
    DustRandom( work , ctrl ) ;/*ふらふら*/
    DustDefense( work , ctrl ) ;/*めりこんじゃダメ*/

    GM_ActControl( ctrl ) ;

    trap_flag = DustCheckBoundary( work , ctrl ) ;/*トラップ監視*/
    if( trap_flag & 0x07 ){
		ctrl->mov = save_mov ;
		trap_flag &= (~trap_flag) ;
		ctrl->step.vx = 0.0F ;
		ctrl->step.vy = 0.0F ;
		ctrl->step.vz = 0.0F ;
    }
    DG_SetPos2( &ctrl->mov , &ctrl->rot ) ;
   
    {/*回転処理*/
    	FMATRIX before ;
    	
    	DG_GetPos( &before ) ;
    	_sceVu0MulMatrix( &before , &before , &work->rotate ); 
    	DG_SetPos( &before ) ;/*回転させてる*/
    }
    
    GM_ActObject( body );
	
	/*カメラ当たりの処理*/
	{
		FVECTOR came , fvtemp ;
		float   ftemp ;
		if( &DG_Chanls[0] != NULL ){
#if 0
			came.vx = DG_Chanls[0].eye.m[3][0];
			came.vy = DG_Chanls[0].eye.m[3][1];
			came.vz = DG_Chanls[0].eye.m[3][2];
#else
			came = *(FVECTOR *)DG_Chanls[0].eye.m[3] ;
#endif
			_sceVu0SubVector( &fvtemp , &came , &ctrl->mov );
			ftemp = YN_VectorLength( &fvtemp );
			if( ftemp <= 200.0F ){/*バウンダリーに近付いたら消す*/
				DG_InvisibleObjs( work->object.objs );
			}else{
				DG_VisibleObjs( work->object.objs );
			}
		}
	}
	
    /* ライトマトリクスの取得 */
    DG_GetLightMatrix( &ctrl->mov, work->light );
    
}

/* ---------------------------------------------------------------- */
/* アクター終了処理 */
static void Die( Work *work )
{
	/* 起動時に確保したものを開放する */
	GM_FreeObject( &work->object );
	GM_FreeControl( &work->control );
}

/**********************************************************************/
                     /* トラップ情報取得 */
static int GetTrapInfo( FVECTOR *center, FVECTOR *wide,
			HZX_GROUP_ID group_id, int name_id )
{
	int				i , j ;
	HZX_GRP			*group;
	HZX_BLOCK		*block;
	HZX_TRP			*trap;
	FVECTOR			bound0,bound1,fvtemp0;

	group = HZX_GetGroup( group_id );
	ASSERT( group );
	block = group->blocks;
	ASSERT( block );
	//printf( "segs %d[%08x]\n", block->n_segs, block->segs );
	//printf( "flrs %d[%08x]\n", block->n_flrs, block->flrs );
	//printf( "trap %d[%08x]\n", block->n_traps, block->traps );
	//ASSERT( trap );

	for ( j = 0 ; j < group->n_blocks ; j++ , block++ ){
		trap = block->traps;
	    for ( i = 0; i < block->n_traps; i++, trap++ ) {
			if( trap->name_id == name_id ){
				
				bound0.vx = (float)trap->b1.vx + (float)block->tx;
				bound0.vy = (float)trap->b1.vy + (float)block->ty;
				bound0.vz = (float)trap->b1.vz + (float)block->tz;
				bound0.vw = 1.0f;
				
				bound1.vx = (float)trap->b2.vx + (float)block->tx;
				bound1.vy = (float)trap->b2.vy + (float)block->ty;
				bound1.vz = (float)trap->b2.vz + (float)block->tz;
				bound1.vw = 1.0f;
				
				_sceVu0SubVector( &fvtemp0, &bound1, &bound0 );
				_sceVu0ScaleVector( &fvtemp0, &fvtemp0, 0.5f );
				_sceVu0AddVector( center, &fvtemp0, &bound0 );
				_sceVu0SubVector( wide, center , &bound0 );/*bound0->centerへのベクトル*/
				
				//PRINT_PFVEC(0,center);
				//PRINT_PFVEC(0,wide);
				return 0;
			}
	    }
	}
	return (-1);
}


/* ---------------------------------------------------------------- */
/* リソース確保関数 */
static int GetResources( Work *work, int name, int where )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	int		model_id ;
	int  		code ;
	int		hzx_group;
	int		trap_code;

	work->name = name ;

	/*回転マトリクスの初期化*/
	work->rotate = DG_UnitMatrix ;/*←単位マトリクス*/

	/* シナリオ起動時のオプションを解析 */
	/* トラップ関係取得 */
	code = GCL_GetNextOption();
	if( code != 'g' ){
	    printf("not input floatdust hzx group\n");
	    return (-1);
	}
	hzx_group = ( HZX_GROUP_ID )GV_GetBit( GCL_GetNextInt() );

	code = GCL_GetNextOption();
	if( code != 't' ){
	    printf("not input floatdust trap group\n");
	    return (-1);
	}
	trap_code = GCL_GetNextInt();

	/*トラップの真中にモデル配置*/
	GetTrapInfo( &pos, &work->wide , hzx_group, trap_code ) ;
	work->center = pos ;

	/*初期rotをランダムで取得*/
	rot.vx = (short)RND(4096) ;
	rot.vy = (short)RND(4096) ;
	rot.vz = (short)RND(4096) ;
	
	/* モデルＩＤ取得 */
	if( GCL_GetOption( 'm' ) != NULL ){
		model_id = GCL_GetNextInt();
	}else{
	    printf("ERR not float_dust.c model !!\n");
	    return (-1);
	}
	/* モデルタイプ 取得*/
	//	if( GCL_GetOption( 'y' ) != NULL ){
	//		work->mode = GCL_GetNextInt();
	//	}
	work->mode = GCL_GetOptionValue( 'y', 0 );
	
	ctrl = &work->control ;
	body = &work->object ;

	/* コントロール初期化 */
	GM_InitControlEx( ctrl, name, where, CTRL_FLAG );	/* コントロール初期化 */
	ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK ;	//	： 壁当たりチェックを行わない。
	ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;	//	： 床当たりチェックを行わない。
	GM_ConfigControlPosition( ctrl, &pos, &rot );		/* 位置初期化 */
	GM_ConfigControlObject( ctrl, body );			/* オブジェクトとの関連付け */
	//GM_ConfigControlHazard( ctrl, 150, 100, 500 );	/* 当たり関連初期化(仮) */
	//GM_ConfigControlHzxHeight( ctrl, 100, 100 );		/* 当たり判定逆さ初期化（今は仮！！） */
	//GM_ConfigControlHzxCheckFlag( ctrl, 0, 0 );			/* （今は仮！！） */
	/* オブジェクト初期化 */
	GM_InitObject( body, model_id, BODY_FLAG );			/* オブジェクト初期化 */
	GM_ConfigObjectLight( body, work->light );			/* ライトマトリクス設定 */
	//GM_ConfigObjectStep( body, &ctrl->step );			/* モーション反映用にステップ変数設定 */
	//GM_ConfigObjectMotion( body, 2, motion_id, MT_FLAG_PDRAY );		/* モーション割り当て */
	//GM_ConfigObjectAction( body, 0, 0, 0, ~0, MOTION_INTERP_TIME );

	GM_ActMotion( body );
	GM_ActControl( ctrl );
	GM_ActObject2( body );

	return (0);
}

/* シナリオ起動関数 */
void *NewDust( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	//work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;	/* 旧形式 */
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 0 );	/* 新形式 */
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
