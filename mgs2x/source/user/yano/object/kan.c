//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	kan.c

	w31bステージの水に浮かぶ空き缶
	
	2001/05/08 YUKO YANO
	$Id: kan.c,v 1.1.1.3 2002/11/19 11:52:01 Yoshizawa1 Exp $

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
#include <libutl.h>

#include "gameheader.h"

#define CTRL_FLAG		(CTRL_SKIP_CHANGE_MAP|CTRL_SKIP_GET_ADDRESS)/* source/game/g_struct.hを参照 */
#define BODY_FLAG		(DG_FLAG_ONEPIECE)	/* source/system/libdg/libdg.hを参照 */

#define WATER_EDGE					(-255022.0F)

/*-------kan_flag(long)---------*/
#define BIT_ON_STAIRS				(0x0800)
//#define BIT_SPLASH_MUST_CALL 		(0x1000)
#define BIT_SPLASH3_KILLCALLED1     (0x2000)
#define BIT_SPLASH3_KILLCALLED2     (0x4000)
#define BIT_WATEREXPLOSION 	 		( 0x00008000 )
#define BIT_SOUNDED_CANWALL1 		( 0x00010000 )
#define BIT_STAIRS_WATERPOOL		( 0x00020000 )

#define PLUS  			1
#define MINUS 			0
#define POSI_Y 			(-48.0f)
#define ROT_Z 			(4000)
#define EFFECT_SCALE 	(900.0f)
#define DEFENSE_SCALE 	(400.0f)
#define UPDOWN_RATE 	(0.2f)
#define _1024_TO_PI 	( PI / 2048.0F )
#define CIRCLE_VALUE 	(2.0F * PI / 4096.0F)
#define FLY_A			(1.0F/15.0F)
#define FLY_V0			(4.0F)
#define HANKEI_RATE		(200.0F)
#define HANKEI_RATE2	(5.0F)
#define GURU_LEVEL		(10.0F)
#define GURU_SCALE		(500.0F)
//#define YUREHABA		(1.0F / 50.0F * 100.0F)
#define YUREHABA		(1.0F / 50.0F * 500.0F)
#define MIGI		(0)
#define HIDARI		(1)
#define KSF_WATERPUSH 	(-0.9F)
#define KSF_RANDNUM   	(1000)
#define WATER_POWER   	(0.01F)
#define WATER_POWER2   	(0.2F)
#define	DECAY_RATIO		( 0.4f )

/*-------------------------*/
#define	BIT_SPLASH_STOP		( 0x0002 )
#define	BIT_SPLASH2_STOP	( 0x0004 )
#define	BIT_SPLASH3_STOP_A	( 0x0008 )
#define	BIT_SPLASH3_STOP_B	( 0x0010 )

/*-----滝 関係--------*/
#define WATERFALL_X 	( 9300.0F )
//#define WATERFALL_Y 	( -6450.0F )
#define WATERFALL_Z 	( -256474.0F )
#define WATERFALL_LEN 	( 450.0F )
#define WATERFALL_POWER ( -50.0F )
#define WATERFALL_AMPLITUDE ( 28.0F )

/*-------水爆----------*/
#define WATEREXPLO_LIMIT	( 1500.0F )
#define RETURN_EXIT_VALUE   ( -1000.0F )


#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) ) /*nまでの乱数を出す*/

/*任意の軸で右回りに回転させるマトリクス取得*/
extern void GTE_MakeRotateAxis( FMATRIX *res, FVECTOR *axis, float angle );
extern void GTE_MakeRotate( FMATRIX *res, float x, float y, float z, float angle );

extern float YN_VectorLength( FVECTOR *in );

/*水飛抹*/
extern void *NewKanSplash( int name, int where , FMATRIX *kan_world );
extern void *NewKanSplash2( int name, int where , FMATRIX *kan_world );
extern void *NewKanSplash3( int name, int where , FMATRIX *kan_world , int *kan_flag );

/*水爆*/
extern FVECTOR *OK_GetSplushWavePos( FVECTOR *pos );


enum {/*KanYuraのstep文で使用*/
	KY_EXIT = 0 ,
	KY_INIT ,
	KY_YURA ,
	KY_INIT2 ,
	KY_YURA2
};
enum {/*KanGururiのstep文で使用*/
	KG_EXIT = 0 ,
	KG_INIT ,
	KG_GURU 
};
enum {/*KanShotFlyのstep文で使用*/
	KSF_EXIT = 0 ,
	KSF_INIT ,
	KSF_DOWN ,
	KSF_TAME ,
	KSF_FLYING ,
	KSF_LANDING_INIT ,
	KSF_LANDING 
};
enum {/*KanShotSpinのstep文で使用*/
	KSS_EXIT = 0 ,
	KSS_INIT ,
	KSS_INWATER ,
	KSS_OUTOFWATER ,
	KSS_OUTOFWATER2 ,
	KSS_LANDING_INIT ,
	KSS_LANDING ,
};

enum {/*GetAngleWaterExplosionのstep文で使用*/
	GAW_EXIT = 0 ,
	GAW_EXIT2 ,
	GAW_INIT ,
	GAW_YURA ,
};

typedef	struct	{
    float vx , vy ;
    float x ,y ;
} Round ;

/* プログラムワーク */
typedef	struct	{
	GV_ACT_EX	actor ;
	CONTROL		control ;		/* コントロール */
	OBJECT		object ;		/* オブジェクト */
	FMATRIX		light[2] ;		/* ライトマトリクス設定用 */

    TARGET		target ;/*ターゲット*/
    POWER_TARGET	power ;

	FMATRIX world ,		/*回転用記憶マトリクス*/
		unit_matrix ;	/*汎用単位マトリクス*/

	int 		random[4];/*汎用ランダム変数*///0,1->固定 2,3->毎Act更新
	
	int 		condition ;/*浮いているか、水中か判別用*/

	long 		kan_flag ; 	//1ビット目から、|上下運動|,|水没|,,,
	float 		sabun ;		/*ctrl->mov.vyの値を変えずにこっちを使って、直接DG_SetPos2を使う*/
	FVECTOR 	updown ;	/*直接DG_SetPos2を使う用*/
	FVECTOR 	kan;		/*壁抜け阻止用*/
    int cont , cont_limit ;	/*速度の伝達を鈍らせるための時間カウンタ*/
    float down_a ;
    
	/*KanRound*/
	float		circle_vx , circle_vy ;/*点の移動量*/
	
	/*KanUpdown*/
	Round		ud ;			/*上下運動用 記憶変数*/
	float		sinpuku ;		/* 段々振幅が小さくなるように */
	int 		updown_cnter ;	/* 缶が揺れるのが続く */

	/*KanPlayer*/
	Round		move ;			/*移動用 記憶変数*/
	float 		p_vector_len ;	/*プレイヤーの移動距離*/
    FVECTOR 	b_posi ,		/*前フレームの位置*/
				p_vector ,		/*プレーヤーの動きベクトル*/
				diff_vector ;	/*プレイヤーとの差分ベクトル*/
    float		diff_len ;		/*プレイヤーとの差分ベクトルの距離*/
	float 		temp_vx , temp_vz;/*空き缶の移動量*/
	float		noroo ;			/*大きな円を描きながらゆっくり進むため*/
	float		hankei ;		/*初期半径を記憶する用*/
	float		back_cnt ;		/*後ろからついてくるのが続く用時間カウンター*/
	FVECTOR 	velocity ;

	
	/** 沈んでる **/
	float		y_level ;/*沈んでいる時の水面からの距離*/
	float		shosoku ;/*yの初速*/

	/*KanFly*/
	FVECTOR		haizuru ;
	float		haizuru_len ;

	/*KanGururiY*/
	float 		degreeDAMY ;
	short 		degree  ;	/*角度*/
	int 		Guru_v ;	/*switch文で使用*/

	/*KanRandom*/
	FVECTOR		randv ;		/*ランダム移動用ベクトル*/
	int			rand_cnt ;	/*時間制御用*/
    float 	rand_length ;
    float	rand_theta ;
    float	rand_sin , rand_cos ;

	/*KanYura*/
	int 		KY_step ;
	short       vz_limit ;
	Round 		KY_round ;
	float       KY_angle , KY_vz_dmy ;
	Round		yura_round ;
	FVECTOR   	KY_axis ;
	float 		last_y_rate ;

    /*KanShot*/
    FVECTOR hit_cen ;
    FVECTOR shot_vel ;
	FVECTOR g_cos , g_sin , flyfor , slidefor , spinfor ;
	float   g_cos_len , flylen , slidelen , spinlen ;
 
    short   KSY_haba ;
    float   KSY_add ;

	float angle ;

    FVECTOR force ;
	FMATRIX rotate ;

	/*KanShotFly*/
	int KSF_step ;
	int KSF_rand , KSF_rand2 ;
	int KSF_flag ;

	/*KanShotRebound*/
	//FVECTOR rb_vec ;

	/*KanShotSpin*/
	int KSS_step ;
	float revs ;
    FVECTOR axis ;

	/*KanWaterExplosion*/
	Round GAW_round ;
	short angle_limit ;
	int   GAW_step ;
	float GAW_last_angle ;
	FVECTOR KWE_diff ;
	float GAW_hertz ;
	float last_explosion_x ;

	/*KanWaterFall*/
	int 	waterfall_flag ;

	/* In Water */
	FVECTOR default_pos ;
	float 	inwater_diff_limit ;
} Work ;


/*****************************************************************/
                     /*YanoGetHousen*/
/*
 * 三つの HZX_VEC から張られる平面の法線を求める
 * 三点から出てきた二つのベクトルは(x,z,y)の順番
 */
static void YanoGetHousen( FVECTOR *housen , HZX_VEC p1 , HZX_VEC p2 , HZX_VEC p3 )
{
	FVECTOR v1 , v2 ;
	float ftemp ;
	_sceVu0SubVector( &v1 , (FVECTOR*)(&p2) , (FVECTOR*)(&p1) );
	_sceVu0SubVector( &v2 , (FVECTOR*)(&p3) , (FVECTOR*)(&p1) );
	ftemp = v1.vy ;
	v1.vy = v1.vz ;
	v1.vz = ftemp ;
	ftemp = v2.vy ;
	v2.vy = v2.vz ;
	v2.vz = ftemp ;
	_sceVu0OuterProduct( housen , &v1 , &v2 ) ;
}

/*----------------------------------------------------------------*/
                      /* YN_VectorLenXZ */
/*
 *XZだけの距離を見る
 */
static float YN_VectorLenXZ( FVECTOR *in )
{
	float  	ftemp ;

	ftemp = in->vx * in->vx + in->vz * in->vz ;
	ftemp = DG_SQRT( ftemp );

	return ftemp ;
}
	
	
/*****************************************************************/
                      /*YanoOuterOuter*/
/*
 *外積を二回行なって、in と in2 の二つのベクトルに貼られる平面上の
 *in に垂直で 左にある単位ベクトル out を返す。
 */
static void YanoOuterOuter( FVECTOR *out , FVECTOR *in , FVECTOR *in2 )
{
	_sceVu0OuterProduct( out , in2 , in ) ;
	_sceVu0OuterProduct( out , in , out ) ;
	_sceVu0Normalize ( out , out ) ;
}
/***************************************************************/
/* 円運動  4096で一周 、 半径 1 */
/***************************************************************/
/*
 * rは半径の長さ
 */
static float KanRound( Work *work , Round *in , float step , float r , int l_r)
{	
	Round	*round ;		/*4096で一周 、半径は1 とする。KanRound用*/
	float	ax , ay ;
	float 	v_temp , t_temp ;
	float 	new_vx , new_vy ;
	float   tr , nv_temp , tnv_temp ,a_temp ;

	round = in ;
	
	/* 安全機構 */
	if( round->vx < 0.00001f ){
		round->vx = 0.00001f;
	}
	if( round->vy < 0.00001f ){
		round->vy = 0.00001f;
	}

	v_temp = round->vx*round->vx + round->vy*round->vy ;
	v_temp = bp_sqrtf(v_temp) * step ;  //BP_MATH - emulate PS2 sqrtf
	tr = 1 / r ;
	new_vx = round->vx * tr * step ;
	new_vy = round->vy * tr * step ;
	nv_temp = v_temp * tr ;
	tnv_temp = 1 / nv_temp ;
	a_temp = bp_sqrtf(1 - nv_temp*nv_temp) ;  //BP_MATH - emulate PS2 sqrtf
	a_temp = 1 - a_temp ;
	if(l_r == MIGI){/*右回り*/
	    ax = -new_vy * tnv_temp * a_temp ;
	    ay =  new_vx * tnv_temp * a_temp ;
	}else{          /*左回り*/
	    ax =  new_vy * tnv_temp * a_temp ;
	    ay =  -new_vx * tnv_temp * a_temp ;
	}
	round->x += ( ax + new_vx ) ;
	round->y += ( ay + new_vy ) ;
	work->circle_vx = ( ax + new_vx ) ;
	work->circle_vy = ( ay + new_vy ) ;

	t_temp = round->x*round->x + round->y*round->y ;
	t_temp = bp_sqrtf( t_temp ) ; //BP_MATH - emulate PS2 sqrtf
	round->x /= t_temp ;
	round->y /= t_temp ;

	if(l_r == MIGI){/*右回り*/
	    round->vx = -round->y * v_temp / step ;
	    round->vy =  round->x * v_temp / step ;
	}else{          /*左回り*/
	    round->vx =  round->y * v_temp / step ;
	    round->vy = -round->x * v_temp / step ;
	}	    
	return round->y ;
}


/*************************************************************************/
/* 揺れる */
/*************************************************************************/

/*
 * すべてマトリクスで計算。shot系の関数と統一性持たせる。
 */
static int KanYura( Work * work , float add_deg )
{
	/*任意軸で揺らす*/
    FMATRIX buff2 , buff ;
    FVECTOR temp ;
	float b_angle = 0.0F ;
	float temptemp ;
	int step = work->KY_step ;
	float y_rate ;

	/*ランダム要素追加*/
	//ran = (float)(RND(2000)-1000)/1000.0F ;/*-1 〜 1*/
	
	DG_GetPos( &buff ) ;/*transfer のみ必要*/

    switch( step ){
      case KY_INIT:/*揺れ初期化*/
		work->KY_round.vx = CIRCLE_VALUE ;/*KanRound関数使用変数初期化*/
		work->KY_round.vy = 0.0F ;
		work->KY_round.x = 0.0F ;
		work->KY_round.y = 1.0F ;
		work->vz_limit  = (short)(work->p_vector_len * YUREHABA) ;
		if( work->vz_limit > 500 ){
			work->vz_limit = 500 ;
		}
		if( work->vz_limit < 100 ){
			work->vz_limit = 100 ;
		}
		work->KY_vz_dmy = (float)work->vz_limit ;

		b_angle = 0.0F ;
		step = KY_YURA ;   /*次へ*/

	  case KY_YURA :
		//		b_roundy = work->KY_round.y ;
		y_rate = KanRound( work , &(work->KY_round) , 40.0F , 1.0F , MIGI );
		if( y_rate * work->last_y_rate <= 0.0F ){/*正負逆転*/
			work->KY_vz_dmy += add_deg ;
			work->vz_limit  = (short)work->KY_vz_dmy ;
		}
		work->last_y_rate = y_rate ;/*次回のために記憶*/
		b_angle = work->KY_angle ;
		work->KY_angle = y_rate * (float)work->vz_limit / 2048.0F * PI ;

		if(work->vz_limit < 0 ){    /*幅がなくなり次第*/
			step = KY_EXIT ;		/*    次へ        */
			work->vz_limit = 0 ;
			work->kan_flag &= (~0x80);
			return step ;
		}
		break ;

	  case KY_INIT2:
		work->vz_limit  = (short)(work->p_vector_len * YUREHABA) ;
		if( work->vz_limit > 500 ){
			work->vz_limit = 500 ;
		}
		if( work->vz_limit < 100 ){
			work->vz_limit = 100 ;
		}
		work->KY_vz_dmy = (float)work->vz_limit ;

		b_angle = 0.0F ;
		step = KY_YURA2 ;   /*次へ*/

	  case KY_YURA2:
		y_rate = KanRound( work , &(work->KY_round) , 40.0F , 1.0F , MIGI );
		if( y_rate * work->last_y_rate <= 0.0F ){/*正負逆転*/
			work->KY_vz_dmy += add_deg ;
			work->vz_limit  = (short)work->KY_vz_dmy ;
		}
		work->last_y_rate = y_rate ;/*次回のために記憶*/
		b_angle = work->KY_angle ;
		work->KY_angle = y_rate * (float)work->vz_limit / 2048.0F * PI ;

		if(work->vz_limit < 0 ){    /*幅がなくなり次第*/
			step = KY_EXIT ;		/*    次へ        */
			work->vz_limit = 0 ;
			work->kan_flag &= (~0x80);
			return step ;
		}
		step = KY_YURA ;
		break ;

	  default :
		b_angle = 0.0F ;
		break ;
    }

	temptemp =  b_angle - work->KY_angle ;
    _sceVu0Normalize( &work->KY_axis , &work->KY_axis ) ;    
    GTE_MakeRotateAxis( &buff2 , &work->KY_axis , temptemp ) ;

    temp.vx = buff.m[3][0] ;
    temp.vy = buff.m[3][1] ;
    temp.vz = buff.m[3][2] ;
    work->world.m[3][0] = 0.0F ;
    work->world.m[3][1] = 0.0F ;
    work->world.m[3][2] = 0.0F ;

    _sceVu0MulMatrix( &work->world , &buff2 , &work->world ) ;
    work->world.m[3][0] = temp.vx ;
    work->world.m[3][1] = temp.vy ;
    work->world.m[3][2] = temp.vz ;

    DG_SetPos( &work->world ) ;

	return step ;
}	

/*************************************************************************/
/* Y軸に対して回転する  */
/*************************************************************************/

static void KanGururiY( Work *work , CONTROL *ctrl , float decay )
{
	int plus ;
	FMATRIX buff , buff2 ;
	float angle ;
	FVECTOR temp ;

	DG_GetPos( &buff );/*transfer のみ使用*/

	if( work->kan_flag & 0x40 ){/*PLAYERの進行方向に対して左に缶がある時*/
		plus = 1 ;
	}else{/*右に缶がある時*/
		plus = -1 ;
	}

	switch( work->Guru_v ){
	  case KG_INIT:
	    work->degree = (short)(work->p_vector_len * 1.0F) ;
		if(work->degree > 50){
			work->degree = 50 ;
		}
		work->degreeDAMY = (float)work->degree ;
		work->Guru_v = KG_GURU ;

	  case KG_GURU:
		work->degreeDAMY = work->degreeDAMY + decay ;
		work->degree = (short)work->degreeDAMY ;
		if( work->degree <= 0 ){
			work->Guru_v = KG_EXIT ;
			work->kan_flag &= (~0x20) ;/*回転フラグOFF*/			
		}
		break;

	}

	angle = (float)work->degree*(float)plus / 2048.0F * PI ;
	_sceVu0RotMatrixY( &buff2 , &work->unit_matrix , angle );
	_sceVu0ApplyMatrix( &work->KY_axis , &buff2 , &work->KY_axis );/*軸も回転*/

    temp.vx = buff.m[3][0] ;
    temp.vy = buff.m[3][1] ;
    temp.vz = buff.m[3][2] ;
    work->world.m[3][0] = 0.0F ;
    work->world.m[3][1] = 0.0F ;
    work->world.m[3][2] = 0.0F ;
    _sceVu0MulMatrix( &work->world , &buff2 , &work->world ) ;
    work->world.m[3][0] = temp.vx ;
    work->world.m[3][1] = temp.vy ;
    work->world.m[3][2] = temp.vz ;

    DG_SetPos( &work->world );

}

/********************************************************************/
/* プレイヤーの動きに連動 */
/********************************************************************/
static void KanPlayer( Work *work , CONTROL *ctrl )
{

	float temp , naiseki , len;
	float buff_vx , buff_vz ;
	
	/*playerから空き缶へのベクトル*/
	if( &GM_PlayerPosition == NULL ){
		printf("PLAYER POSITION nothing!!! at kan.c\n");
	    return ;
	}
	work->diff_vector.vx = ctrl->mov.vx - GM_PlayerPosition.vx ;
	work->diff_vector.vz = ctrl->mov.vz - GM_PlayerPosition.vz ;
	temp = (work->diff_vector.vx)*(work->diff_vector.vx)
	    +(work->diff_vector.vz)*(work->diff_vector.vz);
	work->diff_len = bp_sqrtf(temp);/*缶との距離*/  //BP_MATH - emulate PS2 sqrtf
	/*playerの移動ベクトル*/
	work->p_vector.vx = work->b_posi.vx - GM_PlayerPosition.vx ;
	work->p_vector.vz = work->b_posi.vz - GM_PlayerPosition.vz ;
	temp = (work->p_vector.vx)*(work->p_vector.vx)+(work->p_vector.vz)*(work->p_vector.vz);
	work->p_vector_len = bp_sqrtf(temp);/*PLAYERの移動距離*/ //BP_MATH - emulate PS2 sqrtf

	/*影響範囲内に缶が入ってきて、かつ、PLAYERが速く動いている時に
	  避けたり、回ったり、揺れたりする。*/
	if (work->diff_len < EFFECT_SCALE && work->p_vector_len > 10.0F ){
		/* Limit処理 */
		if( work->p_vector_len > 40.0F ){
			work->p_vector_len = 40.0F ;
		}
		/*PLAYERの進行方向に対して缶が右左どっちにいるか?*/
		buff_vx =  work->p_vector.vz ;
		buff_vz = -work->p_vector.vx ;
		naiseki = buff_vx * work->diff_vector.vx +
			buff_vz * work->diff_vector.vz ;
		if( naiseki < 0.0F ){
			work->kan_flag |= 0x40 ;/*左フラグがON*/
		}else{
			work->kan_flag &= (~0x40) ;/*右フラグがON*/
		}
		/*PLAYERの前にいるか、後ろにいるか*/
		naiseki = work->p_vector.vx * work->diff_vector.vx + work->p_vector.vz * work->diff_vector.vz ;
		if( naiseki > 0.0F ){
			work->kan_flag |= 0x100 ;/*後ろフラグがON*/
		}else{
			work->kan_flag &= (~0x100) ;/*前フラグがON*/
		}

		if( !(work->kan_flag & 0x100) ){/*缶が前にある時*/
			/*円を描いて避けていく*/
			/*KanRoundへ渡すための変数初期化*/
		    len = ( EFFECT_SCALE - work->diff_len ) * work->p_vector_len * 0.01F ;
			if( len > 15.0F ){
				len = 15.0F ;
			}
				
			if(work->kan_flag & 0x40){/*左回り*/
				work->move.x =  -work->diff_vector.vz / work->diff_len ;
				work->move.y =   work->diff_vector.vx / work->diff_len ;
				work->move.vx =  work->move.y * len ;
				work->move.vy = -work->move.x * len ;
				work->hankei = work->p_vector_len*HANKEI_RATE2 + HANKEI_RATE;
				KanRound( work , &(work->move) , 0.5F , work->hankei , HIDARI ) ;
			}else {                   /*右回り*/
				work->move.x =   work->diff_vector.vz / work->diff_len ;
				work->move.y =  -work->diff_vector.vx / work->diff_len ;
				work->move.vx = -work->move.y * len ;
				work->move.vy =  work->move.x * len ;
				work->hankei = work->p_vector_len*HANKEI_RATE2 + HANKEI_RATE;
				KanRound( work , &(work->move) , 0.5F , work->hankei , MIGI ) ;
			}

			ctrl->step.vx = work->circle_vx * work->hankei ;
			ctrl->step.vz = work->circle_vy * work->hankei ;
		
		}else {/*缶が後ろにいるとPLAYERについてくる。*/
		    buff_vx = work->diff_vector.vx/work->diff_len ;
		    buff_vz = work->diff_vector.vz/work->diff_len ;
			work->velocity.vx = -buff_vx * work->p_vector_len * 0.25F ;
			work->velocity.vz = -buff_vz * work->p_vector_len * 0.25F ;
			ctrl->step.vx = work->velocity.vx + buff_vx ;
			ctrl->step.vz = work->velocity.vz + buff_vz ;
			work->kan_flag |= 0x0200 ;/*追随フラグON*/
			//work->back_cnt = 60 ;
		}

		/*回転&揺らし*/
		if(work->p_vector_len > GURU_LEVEL){

		    /*回転*/
		    if(work->diff_len < GURU_SCALE){
				work->Guru_v = 1 ;
				work->kan_flag |= 0x20 ;/*回転フラグON*/
				KanGururiY( work , ctrl , -0.1F ) ;			
		    }
		    
		    /*揺らす*/
			//-->DG_SetPosで動かす CONTROLでは動かさない
			if(work->kan_flag & 0x80){
			    work->KY_step = KY_INIT2 ;
			}else{
			    work->kan_flag |= 0x80 ;/*揺らしフラグON*/
			    work->KY_step = KY_INIT ;
			}
		}
		
	}else{/*範囲外または、PLAYER動いていない*/
		if( !(work->kan_flag & 0x100) ){/*缶が前にある時*/
			/*円を描いて避ける。*/
			if(work->kan_flag & 0x40){/*左回り*/
				KanRound( work , &(work->move) , 0.2F , work->hankei , HIDARI ) ;
			}else {                   /*右回り*/
				KanRound( work , &(work->move) , 0.2F , work->hankei , MIGI ) ;
			}

			work->hankei += 1.0F ;
			if(work->hankei > 3000.0F){
				work->hankei = 3000.0F ;
			}
			work->move.vx *= 0.99F ;
			work->move.vy *= 0.99F ;
		
			ctrl->step.vx = work->circle_vx * work->hankei ;
			ctrl->step.vz = work->circle_vy * work->hankei ;
			
		}else {/*後ろからついていくよー 追随*/
			float vel_len , vel_unit_vx , vel_unit_vz ;
			FVECTOR new_vel;
			if(work->kan_flag & 0x0200){
				temp = work->velocity.vx * work->velocity.vx + work->velocity.vz * work->velocity.vz ;
				vel_len = bp_sqrtf(temp) ; //BP_MATH - emulate PS2 sqrtf
				vel_unit_vx = work->velocity.vx / vel_len ;
				vel_unit_vz = work->velocity.vz / vel_len ;
				new_vel.vx = work->velocity.vx - vel_unit_vx * 0.05F ;/*下の数字も変えること↓*/
				new_vel.vz = work->velocity.vz - vel_unit_vz * 0.05F ;
				if(new_vel.vx * work->velocity.vx <= 0.0F || new_vel.vz * work->velocity.vz <= 0.0F){
					work->velocity.vx = 0.0F ;
					work->velocity.vz = 0.0F ;
					work->kan_flag &= (~0x0200) ;
				}
				work->velocity.vx -= vel_unit_vx * 0.05F ;/*ここ↑*/
				work->velocity.vz -= vel_unit_vz * 0.05F ;

				ctrl->step.vx = work->velocity.vx ;
				ctrl->step.vz = work->velocity.vz ;

			}else {
				GV_ZeroMemory( &ctrl->step , sizeof(FVECTOR)) ;
			}
		}
		
		work->p_vector_len = 0.0F ;/*範囲外の時は 0 にしておく*/
		
		/*回転*/
		if(work->kan_flag & 0x20){/*回転フラグチェック*/
		    KanGururiY( work , ctrl , -0.1F ) ;
		}
		
	}

	/*playerの場所を記憶*/
	work->b_posi.vx = GM_PlayerPosition.vx ;
	work->b_posi.vz = GM_PlayerPosition.vz ;
}

/********************************************************************/
/* 缶がプレイヤーにめりこむのを防ぐ関数                             */
/********************************************************************/
static void KanDefense( Work *work , CONTROL *ctrl )
{

	float temp , def_vx , def_vz , sin_th , cos_th /*, kaiten_x , kaiten_z*/ ;
	
	/*playerから空き缶へのベクトル*/
	work->diff_vector.vx = ctrl->mov.vx - GM_PlayerPosition.vx ;
	work->diff_vector.vz = ctrl->mov.vz - GM_PlayerPosition.vz ;
	temp = (work->diff_vector.vx)*(work->diff_vector.vx) + (work->diff_vector.vz)*(work->diff_vector.vz);
	work->diff_len = bp_sqrtf(temp);/*距離*/  //BP_MATH - emulate PS2 sqrtf

	if (work->diff_len <= DEFENSE_SCALE){
	    sin_th = work->diff_vector.vz / work->diff_len;
	    cos_th = work->diff_vector.vx / work->diff_len;
	    def_vx = (DEFENSE_SCALE - work->diff_len)*cos_th ;
	    def_vz = (DEFENSE_SCALE - work->diff_len)*sin_th ;
	}else{
	    def_vx = 0.0F ; 
	    def_vz = 0.0F ;
	    //work->p_vector_len = 0.0F ;/*範囲外の時は 0 にしておく*/
	}
	ctrl->step.vx += def_vx;
	ctrl->step.vz += def_vz;
	
	/*playerの場所を記憶*/
	work->b_posi.vx = GM_PlayerPosition.vx ;
	work->b_posi.vz = GM_PlayerPosition.vz ;
}

/***************************************************************/
/*  上下運動                                                   */
/***************************************************************/
static void KanUpdown( Work *work , CONTROL *ctrl)
{
	FVECTOR fvtemp ;
	FVECTOR waterfall_pos ;
	float   waterfall_len ;
	
	/* 円運動を使って振幅 */
	/*player の動きによって振幅が変わる*/
	work->sinpuku += work->p_vector_len * UPDOWN_RATE ;
	work->sinpuku -= 1.0F ;
	if( work->sinpuku > 18.0F){
		work->sinpuku = 18.0F ;
	}
	if( work->sinpuku <= 2.0F){
		work->sinpuku = 2.0F ;
		work->updown_cnter ++ ;
		if( work->updown_cnter >= 600 ){
			work->sinpuku = 0.0F ;
			work->updown_cnter = 0 ;
		}
	}

	/* 滝 */
	waterfall_pos.vx = WATERFALL_X ;
	//waterfall_pos.vy = WATERFALL_Y ;
	waterfall_pos.vz = WATERFALL_Z ;
	_sceVu0SubVector( &fvtemp , &ctrl->mov , &waterfall_pos );
	fvtemp.vy = 0.0F ;
	waterfall_len = YN_VectorLength( &fvtemp );
	if( waterfall_len < WATERFALL_LEN && work->waterfall_flag == 1 ){
		float hertz , amplitude ; 
		/* IN WATERFALL *///振幅が早く、大きい
		hertz     = (float)work->random[0]*0.001*40.0F + 180.0F ;
		amplitude = (float)work->random[2]*0.001* 4.0F + WATERFALL_AMPLITUDE ;
		work->sabun = KanRound( work , &(work->ud) , hertz , 1.0F , MIGI) * ( amplitude );
	}else{
		/* NORMAL */
		work->sabun = KanRound( work , &(work->ud) , 50.0F , 1.0F , MIGI) * ( 3.0F + work->sinpuku ) ;
	}
}	
/*********************************************************************/
/*  沈んでる缶の動き */
/*********************************************************************/
static void KanPlayer2( Work *work , CONTROL *ctrl )
{
    float temp , naiseki ;
    float buff_vx , buff_vz ;
	
    /*playerから空き缶へのベクトル*/
    work->diff_vector.vx = ctrl->mov.vx - GM_PlayerPosition.vx ;
    work->diff_vector.vz = ctrl->mov.vz - GM_PlayerPosition.vz ;
    temp = (work->diff_vector.vx)*(work->diff_vector.vx)+(work->diff_vector.vz)*(work->diff_vector.vz);
    work->diff_len = bp_sqrtf(temp);/*距離*/ //BP_MATH - emulate PS2 sqrtf

    if( work->diff_len <= 300.0F ){
		/*playerの移動ベクトル*/
		work->p_vector.vx = work->b_posi.vx - GM_PlayerPosition.vx ;
		work->p_vector.vz = work->b_posi.vz - GM_PlayerPosition.vz ;
		temp = (work->p_vector.vx)*(work->p_vector.vx)+(work->p_vector.vz)*(work->p_vector.vz);
		work->p_vector_len = bp_sqrtf(temp);/*距離*/ //BP_MATH - emulate PS2 sqrtf

		/*PLAYERの進行方向に対して缶が右左どっちにいるか?*/
		buff_vx =  work->p_vector.vz ;
		buff_vz = -work->p_vector.vx ;
		naiseki = buff_vx * work->diff_vector.vx +
			buff_vz * work->diff_vector.vz ;
		if( naiseki < 0.0F ){
			work->kan_flag |= 0x40 ;/*左フラグがON*/
		}else{
			work->kan_flag &= (~0x40) ;/*右フラグがON*/
		}

	
#if 0
		naiseki = work->p_vector.vx * work->diff_vector.vx + work->p_vector.vz * work->diff_vector.vz ;
		if( naiseki >= 0.0F ){/*PLAYERの進行方向に対して左に缶がある時*/
			ctrl->step.vx = -work->p_vector.vz ;
			ctrl->step.vz =  work->p_vector.vx ;
		}else{/*右に缶がある時*/
			ctrl->step.vx =  work->p_vector.vz ;
			ctrl->step.vz = -work->p_vector.vx ;
		}
#else
		work->kan_flag |= 0x04 ;/*ちょっと浮かぶモーション始まる*/
		work->haizuru.vx = work->diff_vector.vx ;/*最初の方向を続けるための処理*/
		work->haizuru.vz = work->diff_vector.vz ;
		work->haizuru_len = work->diff_len ;
	
		ctrl->step.vx = work->diff_vector.vx * ( 300.0F-work->diff_len ) / work->diff_len ;
		ctrl->step.vz = work->diff_vector.vz * ( 300.0F-work->diff_len ) / work->diff_len ;

		/*大きく動いた時、回して、揺らす*/
		if(work->p_vector_len > GURU_LEVEL){
			/*回す*/
			work->Guru_v = 1 ;
			work->kan_flag |= 0x20 ;/*回転フラグON*/
			KanGururiY( work , ctrl , -0.1F ) ;			
			
			/*揺らしのフラグ立て*///-->DG_SetPosで動かす CONTROLでは動かさない
			if(work->kan_flag & 0x80){
			    work->KY_step = KY_INIT2 ;
			}else{
			    work->kan_flag |= 0x80 ;/*揺らしフラグON*/
			    work->KY_step = KY_INIT ;
			}
		} else{
		    if(work->kan_flag & 0x20){/*回転フラグチェック*/
				//work->Guru_v = 1 ;
				KanGururiY( work , ctrl , -0.1F ) ;
		    }
		}
#endif
		    
    }else {
		if(work->kan_flag & 0x20){/*回転フラグチェック*/
			//work->Guru_v = 1 ;
			KanGururiY( work , ctrl , -0.1F ) ;
		}
	
		ctrl->step.vx = 0.0F ;/*stepをクリア*/
		ctrl->step.vz = 0.0F ;
    }

    /*playerの場所を記憶*/
    work->b_posi.vx = GM_PlayerPosition.vx ;
    work->b_posi.vz = GM_PlayerPosition.vz ;
    
    return ;
}

/***********************************************************************/
/* ちょびっと浮くよ */
/***********************************************************************/
static void KanFly( Work *work , CONTROL *ctrl )
{
    
    /*二秒で戻ってきて、毎フレーム -1 される動き*/
    if( (work->shosoku - FLY_A) < -FLY_V0 ){/*地面に戻ってきた*/
		work->shosoku = FLY_V0 ;
		work->kan_flag &= (~0x04) ;/*フラグ消しておく*/
		//work->kan_flag |= 0x10 ;/*ずるずるフラグON*/
		ctrl->step.vx = 0.0F ;/*stepをクリア*/
		ctrl->step.vy = 0.0F ;
		ctrl->step.vz = 0.0F ;
		return ;
    }
    ctrl->step.vy = work->shosoku - FLY_A ;
    work->shosoku = work->shosoku - FLY_A ;

    ctrl->step.vx += work->haizuru.vx / work->haizuru_len ;/*1づづ進んでいる*/
    ctrl->step.vz += work->haizuru.vz / work->haizuru_len ;

    return ;
}

/*************************************************************************/
/* ふらふら */
/*************************************************************************/
/*加速度の長さ、角度ともにランダム*/
static void KanRandom( Work *work , CONTROL *ctrl )
{
    float theta ;

    if( work->rand_cnt >= 600 ){
		work->rand_length = (float)RND(10000)/ 10000.0F ;
		theta  = RND(4095);
		work->rand_sin = sinf(theta / 4096.0F * TPI) ;
		work->rand_cos = cosf(theta / 4096.0F * TPI) ;
		work->rand_cnt = 0 ;
    }else {
		work->rand_cnt ++ ;
    }
    ctrl->step.vx += work->rand_length * 0.5F * work->rand_sin ;
    ctrl->step.vz += work->rand_length * 0.5F * work->rand_cos ;
}

/*********************************************************************/
                          /*撃たれて飛ぶ*/
/*
 hori --> 水平方向(X-Z)   vert --> 垂直方向(Y)
 addvel_len ---> 水平方向の減速度
*/
static int KanShotFly( Work *work , CONTROL *ctrl , int step ,
					   FVECTOR *hori , FVECTOR *vert , float addvel_len )
{
    FVECTOR add_hori , hori_unit ;
    float   temp ;
	FVECTOR fvtemp ;
	FVECTOR waterfall_pos ;
	float   waterfall_len = 0.0F ;

	//GV_FLOAT_OPE_ON();
	switch( step ){

	  case KSF_INIT :
		work->KSF_rand = RND(KSF_RANDNUM) ; /*汎用乱数*/
		work->KSF_rand2 = RND(KSF_RANDNUM) ;/*汎用乱数2*/
		work->KSF_flag = 0 ;
		GV_SetActorChild( work , NewKanSplash2( ctrl->name , ctrl->map , &work->world ) );/*水飛抹水面版呼び出し*/
		GV_SetActorChild( work , NewKanSplash2( ctrl->name , ctrl->map , &work->world ) );/*水飛抹水面版呼び出し*/

		step = KSF_DOWN ;

	  case KSF_DOWN :
		/*水平方向*/
		temp = hori->vx ;
		_sceVu0Normalize( &hori_unit , hori ) ;
		_sceVu0ScaleVectorXYZ( &add_hori , &hori_unit , KSF_WATERPUSH ) ;
		_sceVu0AddVector( hori , hori , &add_hori ) ;
		ctrl->step.vx = hori->vx ;
		ctrl->step.vz = hori->vz ;
		
		/*垂直方向*/
		//vert->vy += KSF_UPFORCE ;
		vert->vy += WATER_POWER2 * ( GM_WaterLevel - ctrl->mov.vy );
		if( ctrl->mov.vy-POSI_Y + vert->vy > GM_WaterLevel ){
			step = KSF_TAME ;/*次へ*/
		}
		ctrl->step.vy = vert->vy ;

		break ;

	  case KSF_TAME :
		if(vert->vy > 80.0F){
			vert->vy = 80.0F ;
		}
		if(vert->vy < 20.0F){
			vert->vy = 20.0F ;
		}
#if 0
		_sceVu0Normalize( &hori_unit , hori ) ;
		_sceVu0ScaleVectorXYZ( &add_hori , &hori_unit , 0.2F ) ;
		if(work->KSF_rand % 2){
			hori->vx += (float)(work->KSF_rand - KSF_RANDNUM/2)/250.0F ;
			hori->vz += (float)(work->KSF_rand2 - KSF_RANDNUM/2)/250.0F ;
		}else{
			hori->vx += (float)(work->KSF_rand2 - KSF_RANDNUM/2)/250.0F ;
			hori->vz += (float)(work->KSF_rand - KSF_RANDNUM/2)/250.0F ;
		}
		step = KSF_FLYING ;/*次へ*/

		{/*水飛抹へ「もう出さんでええ」*/
		    int signal = 0  , value = 0 ;
		    
		    signal |= BIT_SPLASH2_STOP ;
		    
		    GV_CallChildSignalFunc( work, signal, value );
		}
		
		GV_SetActorChild( work , NewKanSplash( ctrl->name , ctrl->map , &work->world ) );/*水飛抹呼び出し*/

		break ;
#else
		step = KSF_FLYING ;/*次へ*/

		{/*水飛抹へ「もう出さんでええ」*/
		    int signal = 0  , value = 0 ;
		    
		    signal |= BIT_SPLASH2_STOP ;
		    
		    GV_CallChildSignalFunc( work, signal, value );
		}
		
		GV_SetActorChild( work , NewKanSplash( ctrl->name , ctrl->map , &work->world ) );/*水飛抹呼び出し*/

#endif
		
	  case KSF_FLYING :
		if( work->waterfall_flag == 1 ){
			/* 滝 */
			waterfall_pos.vx = WATERFALL_X ;
			//waterfall_pos.vy = WATERFALL_Y ;
			waterfall_pos.vz = WATERFALL_Z ;
			_sceVu0SubVector( &fvtemp , &ctrl->mov , &waterfall_pos );
			fvtemp.vy = 0.0F ;
			waterfall_len = YN_VectorLength( &fvtemp );
		}else{
			/* NORMAL */
			waterfall_len = 0.0F ;
		}

		/*水平方向*/
		if( !(work->KSF_flag & 0x01) ){
			fvtemp = (*hori) ;
			_sceVu0Normalize( &hori_unit , hori ) ;
			if( waterfall_len < WATERFALL_LEN && work->waterfall_flag == 1 ){
				//printf("滝！！！！！！！\n");
				/* IN WATERFALL *///減速
				_sceVu0ScaleVectorXYZ( &add_hori , &hori_unit , addvel_len - 10.0F ) ;
			}else{
				/* NORMAL */
				_sceVu0ScaleVectorXYZ( &add_hori , &hori_unit , addvel_len ) ;
			}
			_sceVu0AddVector( hori , hori , &add_hori ) ;
			temp = _sceVu0InnerProduct( hori , &fvtemp );/*内積*/  
			if( temp <= 0.0F ){/*ベクトルが逆向きになる*/
				ctrl->step.vx = 0.0F ;
				ctrl->step.vz = 0.0F ;
				work->KSF_flag |= 0x01 ; 
			}else {
				ctrl->step.vx = hori->vx ;
				ctrl->step.vz = hori->vz ;
			}
		}
		
		/*垂直方向*/
		if( waterfall_len < WATERFALL_LEN && work->waterfall_flag == 1 ){
			//printf("滝！！！！！！！\n");
			/* IN WATERFALL *///落ちる
			vert->vy += ( P_GRAVITY + WATERFALL_POWER );
		}else{
			/* NORMAL */
			vert->vy += P_GRAVITY ;
		}
		
		if( ctrl->mov.vy-POSI_Y + vert->vy < GM_WaterLevel ){
			ctrl->step.vy = GM_WaterLevel - ( ctrl->mov.vy-POSI_Y ) ;/*Y座標は初期位置に戻す*/
			step = KSF_LANDING_INIT ;
		}else{
			ctrl->step.vy = vert->vy ;
		}

		break ;

	  case KSF_LANDING_INIT :
		{/*水飛抹へ「もう出さんでええ」*/
			int signal = 0  , value = 0 ;

			signal |= BIT_SPLASH_STOP ;
			
			GV_CallChildSignalFunc( work, signal, value );
		}

		/* 「着水した音」 SE呼びだし */
		//printf("「着水した音」\n");
		GM_SeSetMode( SD_A_CANWIN01 , &ctrl->mov , GM_SEMODE_NORMAL );
		
		step = KSF_LANDING ;

	  case KSF_LANDING :
	    /*水平方向*/
		if( !(work->KSF_flag & 0x01) ){
			fvtemp = (*hori);
			_sceVu0Normalize( &hori_unit , hori ) ;

			/* 滝 *///減速早い
			if( work->waterfall_flag == 1 ){
				waterfall_pos.vx = WATERFALL_X ;
				//waterfall_pos.vy = WATERFALL_Y ;
				waterfall_pos.vz = WATERFALL_Z ;
				_sceVu0SubVector( &fvtemp , &ctrl->mov , &waterfall_pos );
				fvtemp.vy = 0.0F ;
				waterfall_len = YN_VectorLength( &fvtemp );
			}
			if( waterfall_len < WATERFALL_LEN && work->waterfall_flag == 1 ){
				//printf("滝！！！！！！！\n");
				/* IN WATERFALL */
				_sceVu0ScaleVectorXYZ( &add_hori , &hori_unit , -2.0F ) ;
			}else{
				/* NORMAL */
				_sceVu0ScaleVectorXYZ( &add_hori , &hori_unit , -0.7F ) ;
			}
			
			_sceVu0AddVector( hori , hori , &add_hori ) ;
			temp = _sceVu0InnerProduct( hori , &fvtemp );/*内積*/  
			if( temp <= 0.0F ){/*ベクトル逆向き*/
				ctrl->step.vx = 0.0F ;
				ctrl->step.vz = 0.0F ;
				work->KSF_flag |= 0x01 ; 
			}else {
				ctrl->step.vx = hori->vx ;
				ctrl->step.vz = hori->vz ;
			}
		}

		/*垂直方向*/
		/*
		 * 水中、水外それぞれに逆方向の加速度がかかる
		 * 水中では、水面からの距離に応じて加速度が変化する
		 */
		if( !(work->KSF_flag & 0x02)){
			if( ctrl->mov.vy-POSI_Y < GM_WaterLevel ){/*水中*/
				vert->vy += WATER_POWER * ( GM_WaterLevel - ctrl->mov.vy );
				if(ctrl->mov.vy-POSI_Y + vert->vy < GM_WaterLevel){
					/*水から出た瞬間*/
					vert->vy *= 0.8F ;
				}
			}else{/*水外*/
				vert->vy += P_GRAVITY ;
			}

			if(  ctrl->mov.vy-POSI_Y < GM_WaterLevel+10.0F &&
			     ctrl->mov.vy-POSI_Y > GM_WaterLevel-10.0F &&
			     vert->vy < 10.0F && vert->vy > -10.0F  ){/*垂直方向終了*/
				work->KSF_flag |= 0x02 ;
				vert->vy = 0.0F ;
				//vert->vy = ( ctrl->mov.vy - POSI_Y ) - GM_WaterLevel ; 
			}
		}
		ctrl->step.vy = vert->vy ;
		
		if( work->KSF_flag == 0x03 ){/*水平、垂直両方終了したら*/
			step = KSF_EXIT ;/*終り*/
			{/*splash3へ*/
				/*水飛抹へ「もう出さんでええ」*/
				int signal = 0  , value = 0 ;
				signal |= BIT_SPLASH3_STOP_A ;
				GV_CallChildSignalFunc( work, signal, value );
			}
		}
	}
	//GV_FLOAT_OPE_OFF();
	return step ;
}

/**********************************************************************/
                    /*壁、天井跳ね返り*/
static void KanShotRebound( Work *work , CONTROL *ctrl , FVECTOR *step)
{
	HZX_SEG *seg0  = ctrl->segs[0] ;   /*壁*/
	//HZX_SEG *seg1  = ctrl->segs[1] ;   /*  */
	HZX_FLR *floor = ctrl->level[0] ; /*床  */
	HZX_FLR *ceil  = ctrl->level[1] ; /*天井*/
	FVECTOR *vec = step ;
	FVECTOR fvtemp1 ;
	FVECTOR vec_temp ;
	
	/*   ↓ これはコントロールで行なわれている          */
	//HZX_GetOnlinePoint( &fvtemp0 );/*交点を求める     */
	//ctrl->mov = fvtemp ;/*交点までとりあえず進めておく*/
	
	/*壁*/
	if( ctrl->n_touches >= 1 ){		
		fvtemp1.vx = seg0->p2.z - seg0->p1.z ;
		fvtemp1.vy = 0.0f ;
		fvtemp1.vz = seg0->p1.x - seg0->p2.x ;
		DG_ReflectVector( &fvtemp1, vec, vec );
		vec->vx *= DECAY_RATIO;
		vec->vy = 0.0F ;/* Y は床天井に当たるかもしれないので、胡麻化し*/
		vec->vz *= DECAY_RATIO;
		vec_temp = *(vec) ;

		work->shot_vel.vx = vec->vx ;
		work->shot_vel.vz = vec->vz ;
	}
	if( ctrl->n_touches == 2 ){
	    //printf("げっ、壁ふたつにあたってるんですけど\n");
	}

	/*床*/
	if( ctrl->grounded == 1 ){
		if( work->KSF_step == KSF_LANDING ){
			/*階段のところへ滑ってきた時
			 、、、とは限らない、、、
			戻ってきたときもあるじゃん*/
#if 0
			fvtemp1.vx = 1.0F ;
			fvtemp1.vy = 0.0F ;
			fvtemp1.vz = 0.0F ;
			DG_ReflectVector( &fvtemp1, vec, vec );
			vec->vy = 0.0F ;/*念のため*/
#else
			vec->vx = 0.0F ;
			vec->vy = 0.0F ;
			vec->vz = -10.0F ;
#endif
			work->shot_vel = ( *vec );
			work->flyfor.vy = vec->vy ;
		}else{
			/*普段の床 -->実際はあんまりないかも〜*/
			YanoGetHousen( &fvtemp1 , floor->p1 , floor->p2 , floor->p3 ) ;
			DG_ReflectVector( &fvtemp1, vec, vec );
			vec->vx = 0.0F ;        /*壁を抜けるかもしれないから、Y だけ*/
			vec->vy *= DECAY_RATIO ;/*                                  */
			vec->vz = 0.0F ;        /*                                  */
			
			work->flyfor.vy = vec->vy ;
		}
	}
	
	/*天井*/
	if( ctrl->grounded == 2 ){		
		YanoGetHousen( &fvtemp1 , ceil->p1 , ceil->p2 , ceil->p3 ) ;
		DG_ReflectVector( &fvtemp1, vec, vec );
		vec->vx = 0.0F ;        /*壁を抜けるかもしれないから、Y だけ*/
		vec->vy *= DECAY_RATIO ;/*                                  */
		vec->vz = 0.0F ;        /*                                  */

		work->flyfor.vy = vec->vy ;
	}
}

/******************************************************************/
           /*階段よりこっちに来たらダメよ〜ん*/
static void KanShotRebound2( Work *work , CONTROL *ctrl , FVECTOR *step )
{
	step->vx =   0.0F ;/*強制送還*/
	step->vz = -50.0F ;
	work->shot_vel.vx =   0.0F ;
	work->shot_vel.vz = -50.0F ;
	work->KSF_flag &= (~0x01) ;/*無理矢理。。。*/
	if( ctrl->grounded & 0x01 ){/*地面*/
		
		if( work->KSF_step != KSF_LANDING ){
			work->flyfor.vy   =  3.0F ;
			step->vy =  3.0F ;
		}else{
			work->flyfor.vy   =  0.0F ;
			step->vy =  0.0F ;
			//work->KSF_step = KSF_FLYING ;
		}			
	}else{/*天井*/
		work->flyfor.vy =  0.0F ;
		step->vy =  0.0F ;
	}		
}
	 

/***********************************************************************/
                          /*スピン回転*/
/*
  KanShotYura と KanShotGuru を合わせた関数
 */
static int KanShotSpin( Work *work , float decay )
{
	int step = work->KSS_step ;

	FMATRIX buff , buff2 , buff3 ;
	float temp ;
	FMATRIX  fmat , fmat1 ;
	float    angle ;
	FVECTOR  fvtemp ;

	DG_GetPos( &buff );/*transfer のみ必要*/

	switch( step ){
	  case KSS_EXIT:
	    buff2 = DG_UnitMatrix ;
	    break;
	    
	  case KSS_INIT:
	    /*軸決め*/
	    _sceVu0OuterProduct( &work->axis , &work->hit_cen , &work->g_cos );
	    _sceVu0Normalize( &work->axis , &work->axis );
	    /*回転量決め*/
		if( work->g_cos_len > PI*1000.0F ){
			work->g_cos_len = 1000.0F ;
		}
		if( work->g_cos_len < PI*100.0F ){
			work->g_cos_len = 100.0F ;
		}
		work->revs = work->g_cos_len * 0.0005 ;
				
		step = KSS_INWATER ;

	  case KSS_INWATER:
		temp = work->revs ;
		work->revs += decay*0.5F ;
		if( temp*work->revs < 0.0F ){/*正負反転*/
			step = KSS_EXIT ;
		}
		GTE_MakeRotateAxis( &buff2 , &work->axis , work->revs );

		if(work->KSF_step == KSF_FLYING || work->KSF_step == KSF_TAME ){/*水から出たら*/
			step = KSS_OUTOFWATER ;/*次へ        */
			work->revs *= 2.0F ;
		}
		
		break ;

	  case KSS_OUTOFWATER:
		temp = work->revs ;
		work->revs += decay;
		if( temp*work->revs < 0.0F ){/*回転角度なくなったら*/
#if 0
			step = KSS_EXIT ;
#else
			step = KSS_OUTOFWATER2 ;
#endif
			buff2 = DG_UnitMatrix ;
			break ;
		}
		GTE_MakeRotateAxis( &buff2 , &work->axis , work->revs - decay );

		if( work->KSF_step == KSF_LANDING_INIT ){/*水面に戻ってきたら*/
			step = KSS_LANDING_INIT ;/*次へ        */
		}

		break ;

	  case KSS_OUTOFWATER2:
		/*水にもどってくるのを待つ*/
		buff2 = DG_UnitMatrix ;
		if( work->KSF_step == KSF_LANDING_INIT ){/*水面に戻ってきたら*/
			step = KSS_LANDING_INIT ;/*次へ        */
		}

		break ;
		
	  case KSS_LANDING_INIT:
		/*横倒しマトリクス 代入*/
		/*Z軸で倒して、X軸で回転 ランダム*/
		fmat1.m[0][0]= 0.0F ; fmat1.m[0][1]=1.0F ; fmat1.m[0][2]=0.0F ; fmat1.m[0][3]=0.0F ;/*Z軸*/
		fmat1.m[1][0]=-1.0F ; fmat1.m[1][1]=0.0F ; fmat1.m[1][2]=0.0F ; fmat1.m[1][3]=0.0F ;
		fmat1.m[2][0]= 0.0F ; fmat1.m[2][1]=0.0F ; fmat1.m[2][2]=1.0F ; fmat1.m[2][3]=0.0F ;
		fmat1.m[3][0]= 0.0F ; fmat1.m[3][1]=0.0F ; fmat1.m[3][2]=0.0F ; fmat1.m[3][3]=1.0F ;
		angle = (float)(RND( 2048 ));
		angle = angle * _1024_TO_PI ;
		fvtemp.vx = 1.0F ; fvtemp.vy = 0.0F ; fvtemp.vz = 0.0F ;/*X軸*/
		GTE_MakeRotateAxis( &fmat , &fvtemp , angle );
		//YN_MatrixDump( &fmat );
		_sceVu0MulMatrix( &fmat , &fmat , &fmat1 );
#if 0
		work->world.m[0][0] = fmat.m[0][0] ;
		work->world.m[0][1] = fmat.m[0][1] ;
		work->world.m[0][2] = fmat.m[0][2] ;
		work->world.m[1][0] = fmat.m[1][0] ;
		work->world.m[1][1] = fmat.m[1][1] ;
		work->world.m[1][2] = fmat.m[1][2] ;
		work->world.m[2][0] = fmat.m[2][0] ;
		work->world.m[2][1] = fmat.m[2][1] ;
		work->world.m[2][2] = fmat.m[2][2] ;
#else
		*(FVECTOR*)work->world.m[0] = *(FVECTOR*)fmat.m[0] ;
		*(FVECTOR*)work->world.m[1] = *(FVECTOR*)fmat.m[1] ;
		*(FVECTOR*)work->world.m[2] = *(FVECTOR*)fmat.m[2] ;
#endif
		
		/*落ち水飛抹呼び出し*/
		GV_SetActorChild( work ,
						  NewKanSplash3( work->control.name , work->control.map
										 , &work->world , &work->KSS_step ) );
		work->kan_flag &= (~BIT_SPLASH3_KILLCALLED1);
		work->kan_flag &= (~BIT_SPLASH3_KILLCALLED2);
				
		step = KSS_LANDING ;

	  case KSS_LANDING:
		work->revs += decay*DG_FABS(work->revs)*20.0F ;

		if(work->revs > -0.3F && work->revs < 0.3F ){
		    /*水飛抹へ「もう出さんでええ」*/
		    int signal = 0  , value = 0 ;
			
			signal |= BIT_SPLASH3_STOP_B ;
		    GV_CallChildSignalFunc( work, signal, value );
		}
		
		if( work->revs > -0.01F && work->revs < 0.01 ){/*回転角度なくなったら*/
			step = KSS_EXIT ;
			work->revs = 0.0F ;

		}
		/*Y軸で回す*/
		buff3 = DG_UnitMatrix ;
		_sceVu0RotMatrixY( &buff2 , &buff3 , work->revs );

		break ;
	}

	/*回転マトリクスと移動量を合成*/
	work->world.m[3][0] = 0.0F ;
	work->world.m[3][1] = 0.0F ;
	work->world.m[3][2] = 0.0F ;
	_sceVu0MulMatrix( &work->world , &buff2 , &work->world ) ;
	work->world.m[3][0] = buff.m[3][0] ;
	work->world.m[3][1] = buff.m[3][1] ;
	work->world.m[3][2] = buff.m[3][2] ;
	
	DG_SetPos( &work->world ) ;
	
	return step ;
}
/***********************************************************************/
                     /* 着弾(穴が空かない、、、) */
static void KanShot( Work *work , CONTROL *ctrl )
{
	FVECTOR step_save ;
	float ftemp;

	//GV_FLOAT_OPE_ON();
	/*撃たれ初期化*/
    if( work->kan_flag & 0x0400 ){
		float for_sq , hit_sq , inner , coscos , for_len ;
		float sin_th , cos_th , damy;
		short haba ;
		FVECTOR hit_cen_unit ;

		/* 「弾があたった音」 SE 呼びだし */
		//printf("「弾があたった音」\n");
		GM_SeSetMode( SD_A_CANWTR01 , &ctrl->mov , GM_SEMODE_NORMAL );

		/*壁際で SE がなりつづけないように細工*/
		work->kan_flag &= ( ~BIT_SOUNDED_CANWALL1 );

		/*重心を低めに設定*/
		work->target.center.vy -= 30.0F ;
		_sceVu0SubVector(&work->hit_cen , &work->target.hit , &work->target.center ) ;
		GV_ZeroMemory( &ctrl->step , sizeof(FVECTOR)) ;/*撃たれた時は
														 今までの動き
														 をすべて
														 無視して動く*/

		/*力の分解*/
		inner = work->hit_cen.vx*work->force.vx
			+ work->hit_cen.vy*work->force.vy
			+ work->hit_cen.vz*work->force.vz ;
		inner = inner *inner ;/*内積の2乗*/
		for_sq = work->force.vx*work->force.vx
			+ work->force.vy*work->force.vy
			+ work->force.vz*work->force.vz ;
		for_len = bp_sqrtf( for_sq ) ;/*力積のスカラ*/  //BP_MATH - emulate PS2 sqrtf
		hit_sq = work->hit_cen.vx*work->hit_cen.vx
			+work->hit_cen.vy*work->hit_cen.vy
			+work->hit_cen.vz*work->hit_cen.vz ;
		ftemp = for_sq * hit_sq;
		if( ftemp == 0.0f ){
			ftemp = 0.0000001f;
		}
		coscos = inner / ftemp;/* cosの2乗 */
		//printf("coscos %f\n", coscos );
		if( coscos < 0.0f ){
			ASSERT( 0 );
			coscos = 0.0f;
		}
		if( coscos > 1.0f ){
			ASSERT( 0 );
			coscos = 1.0f;
		}
		sin_th = bp_sqrtf( 1 - coscos ) ;   //BP_MATH - emulate PS2 sqrtf
		cos_th = bp_sqrtf( coscos ) ; //BP_MATH - emulate PS2 sqrtf
		/*力の分解 sin-->並行移動 cos-->揺れ*/
		/*分解後のベクトルは g_sin g_cos */
		damy = sin_th ;    /* θ-90°*/
		sin_th = cos_th ; /*        */
		cos_th = damy ;    /*        */
		_sceVu0Normalize( &hit_cen_unit , &work->hit_cen );
		work->g_sin.vx = -hit_cen_unit.vx * sin_th * for_len ;
		work->g_sin.vy = -hit_cen_unit.vy * sin_th * for_len ;
		work->g_sin.vz = -hit_cen_unit.vz * sin_th * for_len ;
		YanoOuterOuter( &work->g_cos , &work->g_sin , &work->force ) ;
		work->g_cos.vx = work->g_cos.vx * cos_th * for_len ;		
		work->g_cos.vy = work->g_cos.vy * cos_th * for_len ;		
		work->g_cos.vz = work->g_cos.vz * cos_th * for_len ;
		work->g_cos_len = cos_th * for_len ;
		/*跳躍力  fly */
		work->flyfor.vx = 0.0F ;
		work->flyfor.vy = work->g_sin.vy ;
		work->flyfor.vz = 0.0F ;
		if( work->flyfor.vy < 0.0F ){/*長さは正*/
			work->flylen = -work->flyfor.vy ;
		}
		/*平行移動と回転  slide , spin */
		work->slidefor.vx = work->g_sin.vx ;
		work->slidefor.vy = 0.0F ;
		work->slidefor.vz = work->g_sin.vz ;

		work->spinfor = work->g_cos ;
                                   		damy = work->spinfor.vx*work->spinfor.vx +
			work->spinfor.vy*work->spinfor.vy +
			work->spinfor.vz*work->spinfor.vz ;
		work->spinlen = DG_SQRT( damy );
		
		/*揺れ初期化*/
		haba = (short)work->g_cos_len ;
		work->KSY_haba = haba ;
		work->KSY_add  = -0.1F ; /* 仮 */

		/*飛翔初期化*/
		work->KSF_step = KSF_INIT ;
		/*slide に比例して移動。*/
		_sceVu0ScaleVectorXYZ( &work->shot_vel , &work->slidefor , 0.4F ) ;
		
		/*fly に比例してはねる*/
		_sceVu0ScaleVectorXYZ( &work->flyfor , &work->flyfor , 0.2F ) ;
		/*回転初期化*/
		work->KSS_step = KSS_INIT ;
		
    }else {/*初期化終了時*/
		GV_ZeroMemory( &ctrl->step , sizeof(FVECTOR)) ;/*初期化しておく*/
    }

    /*飛翔*/
    work->KSF_step = KanShotFly( work , ctrl , work->KSF_step ,
				 &work->shot_vel , &work->flyfor , -0.2F ) ;

	/*階段の上に来ないように細工*/
	if( work->kan_flag & BIT_STAIRS_WATERPOOL ){
		if( ctrl->mov.vz + ctrl->step.vz  > WATER_EDGE ){
			if( work->KSF_step != KSF_FLYING ){
				work->kan_flag |= BIT_ON_STAIRS ;
			}else{/*跳んでる*/
				work->kan_flag |= BIT_ON_STAIRS ;
			}
		}else{
			work->kan_flag &= (~BIT_ON_STAIRS) ;
		}
	}
	//GV_FLOAT_OPE_OFF();

	/*移動のみコントロールで制御*/
	step_save = ctrl->step ;

	if( ctrl->step.vy > -1.0 && ctrl->step.vy < 1.0f ){
		/* Controlの壁抜けを防止(step.vyが1.0f以下だと抜けてしまう可能性がある) */
		ctrl->step.vy = 0.0f;
	}
	GM_ActControl( ctrl );

	/*壁当たり処理*/
	if((ctrl->n_touches != 0) || (ctrl->grounded & 0x03)){
		/* 「壁に跳ね返る音」 SE呼びだし */
		//printf("「壁に跳ね返る音」\n");
		if( !(work->kan_flag & BIT_SOUNDED_CANWALL1) ){
			GM_SeSetMode( SD_A_CANWALL1 , &ctrl->mov , GM_SEMODE_NORMAL );
			work->kan_flag |= BIT_SOUNDED_CANWALL1 ;
		}
		if( work->kan_flag & BIT_ON_STAIRS ){
			if( work->kan_flag & BIT_STAIRS_WATERPOOL ){
				/*階段よりこっちに跳んできた時は跳ね返す*/
				KanShotRebound2( work , ctrl , &step_save );
			}
		}else{
			/*壁、天地に接触していた場合*/	
			KanShotRebound( work , ctrl , &step_save ) ;
			work->axis.vx = -work->axis.vx ;
			work->axis.vy = -work->axis.vy ;
			work->axis.vz = -work->axis.vz ;
		}
	}else{
		work->kan_flag &= ( ~BIT_SOUNDED_CANWALL1 );
	}	
	
	/*回転*/
	work->KSS_step = KanShotSpin( work , -0.001F ) ;
}

/*-------------------------------------------------------------------*/
/*                           水爆                                    */
#if 0
static float GetAngleWaterExplosion( Work *work , int *step , short *angle_limit , float length )
{
	float ftemp , ftemp1;
	short R_angle_limit, angle_limit_decay;
	
	switch( *step ){
	  case GAW_INIT:
		work->GAW_round.vx = CIRCLE_VALUE ;/*KanRound関数使用変数初期化*/
		work->GAW_round.vy = 0.0F ;
		work->GAW_round.x = 1.0F ;
		work->GAW_round.y = 0.0F ;
		length = 1.5F - length / WATEREXPLO_LIMIT ;/*WATEREXPLO_LIMITを1とした時の居場所の割合*/
		R_angle_limit = (short)(((float)work->random[1]*0.2F/*(0.001F*200)*/ + 600.0F) * length );
		*angle_limit = R_angle_limit ;
		*step = GAW_YURA ;
		work->GAW_hertz = work->random[0]*0.02F/*(0.001F*20.0F)*/ + 40.0F ;

	  case GAW_YURA:
		ftemp = KanRound( work , &(work->GAW_round) , work->GAW_hertz , 1.0F , MIGI );
		if( ftemp * work->GAW_last_angle <= 0.0F ){/*正負逆になったらlimit減る*/
			angle_limit_decay = (short)( (float)work->random[3]*0.01F/*(0.001F*10)*/) + 45 ;
			*angle_limit -= angle_limit_decay ;
			if( *angle_limit < 50 ){
				*step = GAW_EXIT2 ;
				return RETURN_EXIT_VALUE ;
			}
		}
		work->GAW_last_angle = ftemp ;/*次回のために記憶*/

		ftemp1 = ftemp * (float)(*angle_limit) * _1024_TO_PI ;
		if( ftemp1 > PI || ftemp1 < -PI ){
			return RETURN_EXIT_VALUE ;
		}else{
			return ftemp1 ;
		}

	  case GAW_EXIT2:
		/*なんもしない*/
		break ;

	  default :
		printf("GAW 'step ERR!!'\n");
		break ;
	}
	
	return RETURN_EXIT_VALUE ;
}
#else
static float GetAngleWaterExplosion( Work *work, float length )
{
	float ftemp , ftemp1;
	short R_angle_limit = 0, angle_limit_decay;
	
	switch( work->GAW_step ){
	  case GAW_INIT:
		/* KanRound関数使用変数初期化 */
		work->GAW_round.vx = CIRCLE_VALUE ;
		work->GAW_round.vy = 0.0F ;
		work->GAW_round.x = 1.0F ;
		work->GAW_round.y = 0.0F ;

		length = 1.5F - length / WATEREXPLO_LIMIT;/* WATEREXPLO_LIMITを1とした時の居場所の割合 */
		R_angle_limit = (short)(((float)work->random[1]*0.2F/*(0.001F*200)*/ + 600.0F) * length );
		work->GAW_step = GAW_YURA ;
		work->GAW_hertz = work->random[0]*0.02F/*(0.001F*20.0F)*/ + 40.0F ;

	  case GAW_YURA:
		ftemp = KanRound( work , &(work->GAW_round) , work->GAW_hertz , 1.0F , MIGI );
		if( ftemp * work->GAW_last_angle <= 0.0F ){
			/*正負逆になったらlimit減る*/
			angle_limit_decay = (short)( (float)work->random[3]*0.01F/*(0.001F*10)*/) + 45 ;
			R_angle_limit -= angle_limit_decay ;
			if( R_angle_limit < 50 ){
				work->GAW_step = GAW_EXIT2 ;
				return RETURN_EXIT_VALUE ;
			}
		}
		work->GAW_last_angle = ftemp ;/* 次フレームのために記憶 */

		ftemp1 = ftemp * (float)(R_angle_limit) * _1024_TO_PI ;
		if( ftemp1 > PI || ftemp1 < -PI ){
			return RETURN_EXIT_VALUE ;
		}else{
			return ftemp1 ;
		}

	  case GAW_EXIT2:
		/*なんもしない*/
		break ;

	  default :
		printf("GAW 'step ERR!!'\n");
		break ;
	}
	
	return RETURN_EXIT_VALUE ;
}
#endif

static void KanWaterExplosion( Work *work , FVECTOR *explosion_pos )
{
	float 	length = WATEREXPLO_LIMIT , movement , angle ;
	FVECTOR fvtemp , diff_unit , axis ;
	FMATRIX fmat , fmat1 , res ;
	int 	flag = 0 ;

	/*爆発の中心から移動*/
	//新しい水爆座標かどうか判定のため、X座標のみ記憶
	if( explosion_pos != NULL ){
		if( explosion_pos->vx != work->last_explosion_x ){
			/* 新しい水爆がきた */
			flag = 1 ;
		}else{
			/* 前の水爆の影響で動いている状態 */
			flag = 0 ;
		}
		work->last_explosion_x = explosion_pos->vx ;

		_sceVu0SubVector( &work->KWE_diff , &work->control.mov , explosion_pos );
		work->KWE_diff.vy = 0.0F;/* vyは無視して、x-z平面で考える */
		fvtemp = work->KWE_diff;

		length = YN_VectorLength( &fvtemp );
		if( length <= WATEREXPLO_LIMIT ){
			movement = (WATEREXPLO_LIMIT - length) * 0.1F  ;
			_sceVu0Normalize( &diff_unit , &work->KWE_diff );
			_sceVu0ScaleVector( &fvtemp , &diff_unit , movement );
			work->control.step.vx = fvtemp.vx ;
			work->control.step.vy = 0.0F ;
			work->control.step.vz = fvtemp.vz ;
		}
	}else{
		//爆発ないので、移動なし
		length = WATEREXPLO_LIMIT ;
		flag = 0 ;
	}

	DG_GetPos( &fmat );

	if( work->control.step.vy > -1.0 && work->control.step.vy < 1.0f ){
		/* Controlの壁抜けを防止(step.vyが1.0f以下だと抜けてしまう可能性がある) */
		work->control.step.vy = 0.0f;
	}
	GM_ActControl( &work->control );
	/* 水爆用階段処理 */
	if( work->kan_flag & BIT_STAIRS_WATERPOOL ){
		if( (work->control.grounded & 0x01) ||
			(work->control.mov.vz > WATER_EDGE ) ){
			FMATRIX fmat ;
			DG_GetPos( &fmat );
			fmat.m[3][2] = WATER_EDGE ;/*強制的にここへ*/
			DG_SetPos( &fmat );
			work->control.mov.vz = WATER_EDGE ;
		}
	}
	
	/* 揺れ */
	if( work->GAW_step != GAW_EXIT2 ){
		if( work->GAW_step == GAW_EXIT ){
			work->GAW_step = GAW_INIT ;
		}
		if( flag == 1 ){/* 新しい水爆が来ている時 */
			work->GAW_step = GAW_INIT ;
		}
		angle = GetAngleWaterExplosion( work, length );
		//printf("angle = %f\n" , angle );
		if( angle > (RETURN_EXIT_VALUE + 1.0F) ){
			fvtemp.vx = 0.7F ; fvtemp.vy = 0.0F ; fvtemp.vz = 0.7F ;/* XZ平面 */
			YanoOuterOuter( &axis , &work->KWE_diff , &fvtemp );
			GTE_MakeRotateAxis( &res , &axis , angle );
			fmat.m[3][0] = 0.0F ;
			fmat.m[3][1] = 0.0F ;
			fmat.m[3][2] = 0.0F ;
			fmat.m[3][3] = 1.0F ;
			_sceVu0MulMatrix( &fmat , &fmat  , &res );
			DG_GetPos( &fmat1 );
			fmat.m[3][0] = fmat1.m[3][0] ;/*移動量のみ影響させる*/
			fmat.m[3][1] = fmat1.m[3][1] ;
			fmat.m[3][2] = fmat1.m[3][2] ;
			fmat.m[3][3] = 1.0F ;
		}else{
			/* angleが不正値を返した時 */
			DG_GetPos( &fmat1 );
			fmat.m[3][0] = fmat1.m[3][0];
			fmat.m[3][1] = fmat1.m[3][1];
			fmat.m[3][2] = fmat1.m[3][2];
			fmat.m[3][3] = 1.0F ;
		}
		//printf("famt\n");
		//YN_MatrixDump( &fmat );
		DG_SetPos( &fmat );

	}else{
		/* 揺れてない時 */
		DG_GetPos( &fmat1 );
		fmat.m[3][0] = fmat1.m[3][0];
		fmat.m[3][1] = fmat1.m[3][1];
		fmat.m[3][2] = fmat1.m[3][2];
		fmat.m[3][3] = 1.0F ;
		DG_SetPos( &fmat );
	}
} 


static void InvKanObj( Work *work )
{
	OBJECT	*kan = &work->object ;
	
	DG_InvisibleObjs( kan->objs );
}

static void VisKanObj( Work *work )
{
	OBJECT	*kan = &work->object ;
	
	DG_VisibleObjs( kan->objs );
}

static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num = GV_ReceiveMessage( work->control.name, &msg );

	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			VisKanObj( work );
			//printf("kan visible");
			break;
		  case 1:
			InvKanObj( work );
			//printf("kan invisible");
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
	FVECTOR *explosion_pos = NULL ;

    /*Message*/
    if( CheckMesgParam( work ) ){
		GV_WaitMessage( work, work->control.name );
		return;
    }

	/*汎用乱数 毎Act違うもの*/
	work->random[2] = RND( 1000 );
	work->random[3] = RND( 1000 );
	
    ctrl = &work->control ;
    body = &work->object ;

	/*前回の座標記憶*/
    work->kan      = ctrl->mov ;

	/*水爆座標取得*/
	explosion_pos = OK_GetSplushWavePos( &ctrl->mov );
	if( explosion_pos != NULL ){
		work->kan_flag |= BIT_WATEREXPLOSION ;
	}else{
		work->kan_flag &= ( ~BIT_WATEREXPLOSION ) ;
		if( work->GAW_step == GAW_EXIT2 ){
			work->GAW_step = GAW_EXIT ;
		}
	}

	//printf("kan_flag= %d , KSF_step= %d , KSS_step = %d\n", work->kan_flag , work->KSF_step , work->KSS_step );
	
    if( !(work->kan_flag & 0x02) ){
		/********浮いてるよ〜*******/
		if( ctrl->mov.vy - GM_PlayerPosition.vy > 800.0F ){
			/*ある一定距離より下だと、缶への影響がない*/
			GV_ZeroMemory( &ctrl->step , sizeof(FVECTOR)) ;/*初期化しておく*/
			KanRandom( work , ctrl ) ;/*ふらふら*/
			KanUpdown( work , ctrl );/*上下運動*/
		}else{
			KanPlayer( work , ctrl ) ;/*プレイヤーの動きに連動*/
			KanRandom( work , ctrl ) ;/*ふらふら*/
			KanDefense( work , ctrl );/*プレイヤーにめりこんだからあかん*/
			KanUpdown( work , ctrl );/*上下運動*/
		}

		if( ctrl->step.vy > -1.0 && ctrl->step.vy < 1.0f ){
			/* Controlの壁抜けを防止(step.vyが1.0f以下だと抜けてしまう可能性がある) */
			ctrl->step.vy = 0.0f;
		}
		GM_ActControl( ctrl );
	    
		/* 書き込む方に缶の座標を入力 */
		work->updown.vx = ctrl->mov.vx ;
		work->updown.vy = ctrl->mov.vy + work->sabun ;
		work->updown.vz = ctrl->mov.vz ;
		work->updown.vw = ctrl->mov.vw ;
	    
		/*階段をすり抜けてしまうのをなんとかしようぜ*/
		if( work->kan_flag & BIT_STAIRS_WATERPOOL ){
			if( ctrl->mov.vz > WATER_EDGE ){
				if( work->KSF_step == KSF_EXIT ||
					work->KSF_step == KSF_LANDING ){
					work->kan_flag &= (~0x01);//上下運動フラグOFF
#if 0
					work->updown.vx = work->kan.vx;//もどれっ
					work->updown.vy = work->kan.vy + work->sabun ;
					work->updown.vz = work->kan.vz - 1.0F ;
					ctrl->mov.vx = work->kan.vx ;//もどれっ
					ctrl->mov.vy = work->kan.vy ;
					ctrl->mov.vz = work->kan.vz - 1.0F ;
#else
					work->updown.vz = WATER_EDGE ;
					work->updown.vy = GM_WaterLevel + POSI_Y ;
					ctrl->mov.vz = WATER_EDGE ;
#endif
				}
			}else{
				work->kan_flag |= 0x01 ;//上下運動フラグON
			}
		}
		DG_SetPos2( &(work->updown) , &(ctrl->rot)) ;/*上下と階段はコントロールと別処理*/

		{
			FMATRIX aa ;
			DG_GetPos( &aa );
			*(FVECTOR*)aa.m[0] = *(FVECTOR*)work->world.m[0];/*回転部分だけ抽出*/
			*(FVECTOR*)aa.m[1] = *(FVECTOR*)work->world.m[1];/*                */
			*(FVECTOR*)aa.m[2] = *(FVECTOR*)work->world.m[2];/*                */
			DG_SetPos( &aa );
			work->world = aa ;
		}
		
		if(work->kan_flag & 0x0400 ||
		   work->KSS_step != KSS_EXIT ||
		   work->KSF_step != KSF_EXIT ){
			/* 着弾 */
			KanShot( work , ctrl ) ;
			if( work->kan_flag & 0x0400 ){
				work->kan_flag &= (~0x0400) ;
				work->kan_flag &= (~0x20) ;/*回転、揺らしフラグをOFF*/
				work->kan_flag &= (~0x80) ;/*                       */
			}
			
			if( work->KSF_step == KSF_LANDING
				|| work->KSF_step == KSF_LANDING_INIT
				|| work->KSS_step == KSS_LANDING ){/*水上に落ちてきた*/
				if( work->kan_flag & 0x20 ){/*回転フラグcheck*/
					KanGururiY( work , ctrl , -0.1F );
				}
				if( work->kan_flag & 0x80 ){/*揺らしフラグcheck*/
					work->KY_step = KanYura( work , -20.0F ) ;/*COLTROLの制御から抜けて揺らす*/
				}
			}

		}else{
			if( ( work->kan_flag & BIT_WATEREXPLOSION ) ||
				( work->GAW_step == GAW_YURA )            ){
				/* 水爆 */
				work->kan_flag &= ( ~BIT_WATEREXPLOSION );
				KanWaterExplosion( work , explosion_pos );
			}
				
			if( work->kan_flag & 0x20 ){/*回転フラグcheck*/
				KanGururiY( work , ctrl , -0.1F );
			}
			if( work->kan_flag & 0x80 ){/*揺らしフラグcheck*/
				work->KY_step = KanYura( work , -20.0F );/*COLTROLの制御から抜けて揺らす*/
			}
		}

		/*カメラ当たりの処理*/
		{
			FVECTOR came , fvtemp ;
			float   ftemp ;
			if( &DG_Chanls[0] != NULL ){
				came.vx = DG_Chanls[0].eye.m[3][0];
				came.vy = DG_Chanls[0].eye.m[3][1];
				came.vz = DG_Chanls[0].eye.m[3][2];
				_sceVu0SubVector( &fvtemp , &came , &ctrl->mov );
				ftemp = YN_VectorLength( &fvtemp );
				if( ftemp <= 200.0F ){/*バウンダリーに近付いたら消す*/
					DG_InvisibleObjs( work->object.objs );
				}else{
					DG_VisibleObjs( work->object.objs );
				}
			}
		}
#if 0
		{
			FMATRIX fmat ;
			DG_GetPos( &fmat );
			YN_MatrixDump( &fmat );
		}
#endif
		/*最終階段チェック
		 *もしも、階段を抜けていたら
		 *Actを殺す
		 */
		if( work->kan_flag & BIT_STAIRS_WATERPOOL ){
			FMATRIX fmat ;
			DG_GetPos( &fmat );
			if( fmat.m[3][2] > WATER_EDGE ){
				FVECTOR *pos ;
				int 	flag ;
				pos = (FVECTOR*)fmat.m[3] ;
				flag = HZX_OnlineHazardCheck( ctrl->hzx_id , &work->kan , pos ,
											  HZX_CHK_F_FLOOR ,	/*ハザード検出タイプ*/
											  0xFFFF , 			/* 壁 */
											  0x0000 );			/* 床 */
				if( flag == 1 ){/*床(階段)ぬけ*/
					printf("Kan goes though the stairs ,so Destroy KAN \n");
					/* 最悪の事態 */
					GV_DestroyActor( work );
				}
			}
		}
		
		GM_ActObject( body );
	    
    }else {
		/* 水中 */
		FMATRIX world2 , world3 ;
		float   ftemp ;
		FVECTOR fvtemp , diff_vec ;

		if( work->kan_flag & 0x08 ){/*沈み初期化したか？*/
			ctrl->height = 75.0F ;
			ctrl->mov.vy = GM_WaterLevel + work->y_level ;/*yを初期化*/
			work->default_pos = ctrl->mov ;
			work->kan_flag &= (~0x08) ;/*沈み初期化フラグOFF*/
		}

		DG_SetPos( &work->world );
	    
		GV_ZeroMemory( &ctrl->step , sizeof(FVECTOR) ) ;
	    
		if( GM_PlayerPosition.vy -  ctrl->mov.vy < 1000.0F ){/*立って歩いているとき*/
			KanPlayer2( work , ctrl ) ;/*プレイヤーから逃げる缶*/
			if( work->kan_flag & 0x04 ){
				KanFly( work , ctrl) ;/*ちょびっと浮いて逃げるよー*/
			}
		}
		KanRandom( work , ctrl ) ;/*ふらふら*/

		if( ctrl->step.vy > -1.0 && ctrl->step.vy < 1.0f ){
			/* Controlの壁抜けを防止(step.vyが1.0f以下だと抜けてしまう可能性がある) */
			ctrl->step.vy = 0.0f;
		}
		GM_ActControl( ctrl );

		/*フラフラどこまでも行かないようにするための処理*/
		//ただし、壁をワープする可能性があるので、設置には気をつける
		_sceVu0SubVector( &diff_vec , &ctrl->mov , &work->default_pos );
		ftemp = YN_VectorLenXZ( &diff_vec );
		if( ftemp > work->inwater_diff_limit ){
			FMATRIX fmat ;
			_sceVu0Normalize( &fvtemp , &diff_vec );
			_sceVu0ScaleVector( &fvtemp , &fvtemp , work->inwater_diff_limit );
			_sceVu0AddVector( &ctrl->mov , &work->default_pos , &fvtemp );
			ctrl->mov.vw = 1.0F ;
			DG_GetPos( &fmat );
			(*(FVECTOR*)fmat.m[3]) = ctrl->mov ;
			DG_SetPos( &fmat );
		}
			
		DG_GetPos( &world3 );
		world2 = work->unit_matrix ;
		(*(FVECTOR*)world2.m[3]) = (*(FVECTOR*)world3.m[3]) ;
		DG_SetPos( &world2 );

		if( work->kan_flag & 0x20 ){/*回転フラグcheck*/
			KanGururiY( work , ctrl , -0.1F );
	    }
		if( work->kan_flag & 0x80 ){/*揺らしフラグcheck*/
			work->KY_step = KanYura( work , -20.0F ) ;/*COLTROLの制御から抜けて揺らす*/
		}
	    
		GM_ActObject( body );

		DG_GetPos( &world2 );
		work->world = world2 ; 

    }

    {/*ターゲット処理*/
		FMATRIX 	pos ;

		DG_GetPos( &pos ) ;
		GM_MoveTarget2( &work->target , &pos ) ;
		//NewTargetView2( &work->target, 32, 232, 32 ) ;/*ターゲットに線をつける*/
    }

	
    /* ライトマトリクスの取得 */
    DG_GetLightMatrix( &ctrl->mov, work->light );

//printf("KAN pos[%f][%f][%f][%f]\n",ctrl->mov.vx,ctrl->mov.vy,ctrl->mov.vz,ctrl->mov.vw);
}

/* ---------------------------------------------------------------- */
/* アクター終了処理 */
static void Die( Work *work )
{
	/* 起動時に確保したものを開放する */
	GM_FreeObject( &work->object );
	GM_FreeControl( &work->control );
	GM_FreeTarget( &work->target ) ;
}

/********************************************************************/
/* ターゲット コールバック関数 */
/********************************************************************/
static void KanCallback( TARGET *offense , TARGET *defense , Work *work )
{
    if ( offense->weapon_type & (WP_LASERSIGHT | WP_STUNGRENADE | WP_STUNFAR) ){
		return ;
    }
    
    work->kan_flag |= 0x0400 ;/*弾命中フラグON*/
    work->force = offense->power->force ;/*力積を格納*/
	/* リミッタをかける */
	if( work->force.vx > 1000000.0f || work->force.vx < -1000000.0f ){
		printf("force x:%f\n",work->force.vx);
		work->force.vx = 0.0f;
		//ASSERT( 0 );
	}
	if( work->force.vy > 1000000.0f || work->force.vy < -1000000.0f ){
		printf("force y:%f\n",work->force.vy);
		work->force.vy = 0.0f;
		//ASSERT( 0 );
	}
	if( work->force.vz > 1000000.0f || work->force.vz < -1000000.0f ){
		printf("force z:%f\n",work->force.vz);
		work->force.vz = 0.0f;
		//ASSERT( 0 );
	}
	if( work->force.vx < 0.00001f && work->force.vx > -0.00001f ){
		printf("force x:%f\n",work->force.vx);
		work->force.vx = 0.0f;
		//ASSERT( 0 );
	}
	if( work->force.vy < 0.00001f && work->force.vy > -0.00001f ){
		printf("force y:%f\n",work->force.vy);
		work->force.vy = 0.0f;
		//ASSERT( 0 );
	}
	if( work->force.vz < 0.00001f && work->force.vz > -0.00001f ){
		printf("force z:%f\n",work->force.vz);
		work->force.vz = 0.0f;
		//ASSERT( 0 );
	}
#if 0
	{
		float x;
		GV_FLOAT_OPE_ON();
		x = work->force.vx + work->force.vy + work->force.vz;
		printf("x = %f\n", x );
		GV_FLOAT_OPE_OFF();	
	}
#endif
}


/********************************************************************/
/* 缶の初期化                                                       */
/********************************************************************/
static void KanInit( Work *work )
{
    FMATRIX initworld ;
    FMATRIX fmat1 , fmat2 ;
    float   angle ;
    
	//player初期位置取得 
	work->b_posi.vx = GM_PlayerPosition.vx ;
	work->b_posi.vz = GM_PlayerPosition.vz ;

	/*プログラムで使用するランダム値*/
	        /* 0.0F 〜 1.0F */
	//簡単な乱数を使用したいとき用。複雑なものには使用しないこと。
	work->random[0] = RND( 1000 );
	work->random[1] = RND( 1000 );

	work->unit_matrix = DG_UnitMatrix ;

	/*初期マトリクス取得*/
	DG_GetPos( &initworld ) ;
	work->world = initworld ;

	fmat1 = work->unit_matrix ;
	if( (work->kan_flag & BIT_STAIRS_WATERPOOL) &&
	    (work->random[1] % 3 == 0)                 ){
	    /*横倒し*/
	    fmat1.m[0][0]= 0.0F ; fmat1.m[0][1]=1.0F ; fmat1.m[0][2]=0.0F ; fmat1.m[0][3]=0.0F ;/*Z軸*/
	    fmat1.m[1][0]=-1.0F ; fmat1.m[1][1]=0.0F ; fmat1.m[1][2]=0.0F ; fmat1.m[1][3]=0.0F ;
	    fmat1.m[2][0]= 0.0F ; fmat1.m[2][1]=0.0F ; fmat1.m[2][2]=1.0F ; fmat1.m[2][3]=0.0F ;
	    fmat1.m[3][0]= 0.0F ; fmat1.m[3][1]=0.0F ; fmat1.m[3][2]=0.0F ; fmat1.m[3][3]=1.0F ;
	}
	angle = work->random[0] * 0.001 * PI ;
	_sceVu0RotMatrixY( &fmat2 , &fmat1 , angle );
	_sceVu0MulMatrix( &work->world , &fmat2 , &work->world );
	DG_SetPos( &work->world );

	work->KY_axis.vx = 1.0F ;
	work->KY_axis.vy = 0.0F ;
	work->KY_axis.vz = 0.0F ;
	
	if(work->kan_flag & 0x02){/*沈みフラグON の時*/
		work->kan_flag |= 0x08 ;/*沈み初期化してませんフラグON*/
	}else {
		work->kan_flag |= 0x01 ;/*上下運動する*/
	}

	work->ud.x  = 1.0F ;/*4096で一周 、半径は1 とする。 円運動用*/
	work->ud.y  = 0.0F ;
	work->ud.vx = 0.0F ;
	work->ud.vy = CIRCLE_VALUE ;
	work->move.vx   = 0.0F ;/*4096で一周 、半径は1 とする。 円運動用*/
	work->move.vy   = CIRCLE_VALUE ;
	work->move.x   = 1.0F ;
	work->move.y   = 0.0F ;
	work->hankei   = 500.0F ;

	work->y_level	= -3950.0F ;/*沈んでる位置*/
	work->shosoku 	= FLY_V0 ;/*ちょっと浮くやつのyの初速*/

	/*KanRandom*/
	work->randv.vx = 0.5F ;
	work->randv.vz = -0.5F ;
	work->rand_cnt = 0 ;

}

/* ---------------------------------------------------------------- */
/* リソース確保関数 */
static int GetResources( Work *work, int name, int where )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	int			model_id ;

	/* シナリオ起動時のオプションを解析 */
	/* 初期位置取得 */
	if ( GCL_GetOption( 'p' ) != NULL ){
		pos.vx = GCL_GetNextInt();
		pos.vy = GM_WaterLevel + POSI_Y ;
		GCL_GetNextInt();/*Yを無視*/
		pos.vz = GCL_GetNextInt();
		//w31bの階段のところにいたら、特別なことをするので、チェック
		if( pos.vz > -259500 && pos.vz < -255000 && pos.vx > 7500 && pos.vx < 13000 ){
			work->kan_flag |= BIT_STAIRS_WATERPOOL ;
		}
	}else{
	    printf("not KAN Positin \n");
	    return (-1);
	}
	
	/* 状態取得 *///横か縦かもいづれここで取得する
	if ( GCL_GetOption( 'c' ) != NULL ){
	    work->condition = GCL_GetNextInt();/* 0--浮かんでいる 1--沈んでる */
	    if( work->condition  & 0x01 ){/*沈んでいるとき、フラグON*/
			work->kan_flag |= 0x02 ;
		}
	}else{
	    printf("not KAN Condition\n");
	    return (-1);
	}
	
	if( GCL_GetOption( 'd' ) != NULL ){
		work->inwater_diff_limit = (float)GCL_GetNextInt();//リミッター設定
	}else{
		if( work->condition  & 0x01 ){
			work->inwater_diff_limit = 1000.0F ;
		}else{
			printf("inwater diff OK!\n");
		}
	}
	
	/* 滝があるかないか */
	if ( GCL_GetOption( 'w' ) != NULL ){
	    work->waterfall_flag = GCL_GetNextInt();/* 0--なし 1--あり */
	    if( !(work->waterfall_flag == 0 || work->waterfall_flag == 1) ){
			work->waterfall_flag = 0 ;
		}
	}else{
	    printf("not KAN waterfall_flag\n");
	    work->waterfall_flag = 0 ;
	}
	/* モデルＩＤ取得 */
	if ( GCL_GetOption( 'm' ) != NULL ){
		model_id = GCL_GetNextInt();
	}else{
	    printf("KAN model isn't !!");
	    return (-1);
	}

	ctrl = &work->control ;
	body = &work->object ;
	/* コントロール初期化 */
	GM_InitControlEx( ctrl, name, where, CTRL_FLAG );	/* コントロール初期化 */
	GM_ConfigControlPosition( ctrl, &pos, &rot );		/* 位置初期化 */
	GM_ConfigControlObject( ctrl, body );				/* オブジェクトとの関連付け */
	GM_ConfigControlHazard( ctrl, 200 , 80, 80 );	/* 当たり関連初期化 */
	//GM_ConfigControlHzxHeight( ctrl, 201, pos.vy );

	GM_ConfigControlHzxCheckFlag( ctrl, HZX_SEG_NO_ENEMY , 0 );

	/* オブジェクト初期化 */
	GM_InitObject( body, model_id, BODY_FLAG );			/* オブジェクト初期化 */
	GM_ConfigObjectLight( body, work->light );			/* ライトマトリクス設定 */

	GM_ActMotion( body );
	GM_ActControl( ctrl );

	/*缶を傾ける処理*/
	rot = DG_ZeroSVector ;
	if( work->kan_flag & BIT_STAIRS_WATERPOOL ){
	    rot.vz = ROT_Z ;
	}
	DG_SetPos2( &pos , &rot ) ;
	
	GM_ActObject2( body );

	KanInit( work );/*缶の初期化*/

	if( !(work->kan_flag & 0x02) ){ // 缶自体の波紋//浮いている時のみ
		extern void* NewConcRipple( int, int, int, int, float,
									float, float, float);
		
		GV_SetActorChild( work, NewConcRipple( GV_StrCode("あきかんの波紋"), name, 3, 6 ,
											   100.f, 150.f, 200.f, 3000.f));
	}

	{    /*ターゲット初期設定*/
	    FVECTOR		size , offset = { 0.0F , 0.0F , 0.0F } ;
	    int map , side ;

	    /*モデルの大きさゲット-->ターゲットの大きさに仕込む*/
	    size.vx = 55.0F ;size.vy = 88.0F ; size.vz = 55.0F ;
	    map = where ;
	    side = ENEMY_SIDE ;
	    GM_SetTarget( &work->target , TARGET_DEFENSE | TARGET_SEEK | TARGET_ROTATE,
					  map , side , &size , &offset ) ;
	    GM_SetPowerTarget( &(work->target) , &(work->power) , POWER_ONCE , 255 , 0 , 0 , &DG_ZeroVector ) ;
	    GM_SetTargetCallBack( &(work->target) , (TARGET_CALLBACK)KanCallback , work  ) ;
	    GM_PutTarget( &(work->target) ) ;
	}
	return (0);
}

/* シナリオ起動関数 */
void *NewKan( int name, int where )
{
	Work		*work ;
	
	OPERATOR() ;
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
