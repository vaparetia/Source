//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  electric_floor.c
  電撃床エフェクト呼び出し
  2001/06/01 Yuuta Kunibe	
  $Id: electric_floor.c,v 1.1.1.3 2002/11/19 11:44:38 Yoshizawa1 Exp $
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"


#define COLOR			(255)
#define ALPHA			(128)

#define N_PRIMS			(2)
#define N_VERTS 		(64)
#define CENTER_NUM 		( N_PRIMS * N_VERTS / 2 )

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	WAVE_HEIGHT		(50.0f)
#define SPEED			(20.0f)

/* 電撃床情報 */
#define	FLOOR_MAX_X		(9375.0f)//(9000.0f)
#define	FLOOR_MIN_X		(4000.0f)
#define	FLOOR_MAX_Z		(-239500.0f)
#define	FLOOR_MIN_Z		(-242000.0f)

#define PANNEL_NUM		(12)

#define	PANNEL_WX		(750.0f)
#define	PANNEL_WZ		(350.0f)

#define	BOMB_DETECT_SPEED2	(5.0f*5.0f)


/* パネル位置 */
static float pannel_PX[3] = { 8400.0f, 6600.0f, 4900.0f };
static float pannel_PZ[4] = {-239800.f, -240400.0f, -241050.0f, -241700.0f };


extern void *NewPolyPlasma( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color );
extern void *NewPolyPlasma3( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color );

extern void *NewSpritePlasma( FVECTOR *from, FVECTOR *middle, FVECTOR *to,
			      float width, float noise_plasma, FVECTOR *color, int flag, int life );

extern void *NewSpritePlasma2( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color, int life );

extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count );

extern void *NewBookFire( FMATRIX *mat );


enum {
    FLOOR_KILL = 0,
};


typedef	struct	{

    GV_ACT_EX	actor;    
    int		name;
    int		code[2];

    int		flag;
    
    int		cnt;
    int		cnt1;
    int		cnt2;
    int		body_plsm_cnt;

	FVECTOR	se_pos;
	int		se_count;
	
    FVECTOR	color;

    int		light_life;
    FVECTOR	light_pos;
    
    GV_MSG	*msg;

    int		kill_flag;
    
} Work;


/* ダンボール燃え用フラグ -> okajima/effect/c_box_body.cでダンボール炎呼び出し */
int	FireBoxFlag = 0;


static void RecieveMessage( Work* work )
{
    GV_MSG*	msg;
    int 	n_msg;

    if ( work->name == 0 ) {
		return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &work->msg ); 

    msg = work->msg;

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case FLOOR_KILL:
	    //printf("recieve_message : kill\n");
	    work->kill_flag = 1;
	    break;
	}

	msg++;

    }

}



/* 爆弾系にプラズマ */
static void PlasmaToBomb( Work *work, FVECTOR *bomb_pos, float width, int life )
{

    int		pannel;
    FVECTOR	middle,to;

    /* 終端パネル位置取得 */
    pannel = irnd()%12;

    to.vx = pannel_PX[pannel/4] + frnd()*PANNEL_WX;
    to.vy =-100.0f;
    to.vz = pannel_PZ[pannel%4] + frnd()*PANNEL_WZ;
    to.vw = 1.0f;


    /* 中継点取得 */
    _sceVu0AddVector( &middle, &to, bomb_pos );
    _sceVu0ScaleVector( &middle, &middle, 0.50f );

    middle.vx += frnd()*250.0f;
    middle.vy += 500.0f + frnd()*250.0f;
    middle.vz += frnd()*250.0f;

		
    /* プラズマ呼び出し */
    NewSpritePlasma( bomb_pos, &middle, &to,
					 width, 1.0f, &work->color, 0, life );

	GM_SeSetMode( SD_A_ELFSPRK1,
				  bomb_pos,
				  GM_SEMODE_NORMAL );

}



/* BOMB系検知関数 */
static void SearchFloorBomb( Work *work )
{

    GM_BOMB	*this;
    GM_BOMB	*list;
    FVECTOR	*mov;
    FVECTOR	*step;
    FMATRIX	mat;

    list = GM_BombList.next;

    while( list != NULL ) {

	this = list;
	list = list->next;
	mov = this->mov;

	if ( mov->vx > FLOOR_MIN_X && mov->vx < FLOOR_MAX_X && mov->vz > FLOOR_MIN_Z && mov->vz < FLOOR_MAX_Z &&
	     mov->vy < 500.0f ) {

	    switch ( this->weapon ) {
	    case WP_Book:			// 本
		if ( !( this->flag & GM_BMB_FLAG_DESTROY ) ) {
		    this->flag |= GM_BMB_FLAG_DESTROY;	
		    PlasmaToBomb( work, this->mov, 15.0f + frnd()*5.0f, 5 );
		    DG_COPY_MAT( &mat, &DG_UnitMatrix );
		    DG_COPY_VEC( (FVECTOR *)mat.m[3], this->mov );
		    NewBookFire( &mat );
		}
		break;
	    case WP_C4Bomb:			// Ｃ４
	    case WP_Claymore:			// クレイモア
		/* 処理なし */				       
		break;
	    case WP_Magazine:			// マガジン
		/* 床で跳ねた時に電撃 */
		step = this->step;
		if ( _sceVu0InnerProduct( step, step ) > BOMB_DETECT_SPEED2 ) {	
		    if ( this->mov->vy <= 50.0f ) {
			PlasmaToBomb( work, this->mov, 15.0f + frnd()*5.0f, 5 );
		    }
		}
		break;
	    default:				// グレネード,チャフ,スタン
		step = this->step;
		/* 動いていたら電撃 */
		if ( _sceVu0InnerProduct( step, step ) > BOMB_DETECT_SPEED2 ) {	
		    if ( this->mov->vy <= 50.0f ) {
				PlasmaToBomb( work, this->mov, 15.0f + frnd()*5.0f, 5 );
		    }
		}
		/* 止まったら爆破 */
		else {								
		    PlasmaToBomb( work, this->mov, 30.0f + frnd()*10.0f, 15 );
		    if ( this->weapon == WP_Magazine ) {
			this->flag |= GM_BMB_FLAG_DESTROY;
		    }
		    else {
			this->flag |= GM_BMB_FLAG_BLAST;
		    }
		}
		break;
	    }

	}

    }

}




/* アクト関数 */
static void Act( Work *work )
{

    FVECTOR	from;
    FVECTOR	middle;
    FVECTOR	to;
    FVECTOR	vectmp;
    float	width;
    int		node;
    int		pannel;
    FMATRIX	mat;
    SVECTOR	rot_vec,rot_wide;
    FVECTOR	pl_pos;



	/* 駆動ＳＥ呼び出し */
	if ( !( ++work->se_count%6 ) ) {
		GM_SeSetMode( SD_A_ELFNOZ01, &work->se_pos, GM_SEMODE_NORMAL );		
	}


    /* トラップ用プレイヤー位置取得 */
    if ( GM_PlayerControl == NULL ) {
		return;
    }
    else {
	
		if ( GM_PlayerControl->root_offset != NULL && GM_PlayerControl->link_zone == NULL ) {
			_sceVu0AddVector( &pl_pos, &GM_PlayerControl->mov, GM_PlayerControl->root_offset );
		} else {
			DG_COPY_VEC( &pl_pos, &GM_PlayerControl->mov );
		}

    }


#if 0	// ダンボール燃えフラグデバッグ用	
	printf("box_fire_flag : %d\n", FireBoxFlag );
#endif	

    
    /* メッセージ受信 */
    RecieveMessage( work );


    if ( work->kill_flag == 1 ) {	/* キルメッセージ受信なら処理せずに速攻で殺す */
		FireBoxFlag = 0;
		GV_DestroyActor( work );
    }
	else {

		if ( GM_NikitaAlive[0] == NKT_NORMAL ) {
			/* ニキータ発射中の処理スキップ */
			if ( GM_NikitaPosition[0].vx < -9000.0f || GM_NikitaPosition[0].vz > -231000.0f ) {
				return;
			}
		}
		else {
			/* 通常状態の処理スキップ */
			if ( GM_PlayerPosition.vx < -9000.0f || GM_PlayerPosition.vz > -231000.0f ) {
				return;
			}
		}


    /* ボムリスト検索 */
    SearchFloorBomb( work );
    
    /* プレイヤーがパネル上にいるか判定 */
    if ( pl_pos.vx > FLOOR_MIN_X && pl_pos.vx < FLOOR_MAX_X && pl_pos.vz > FLOOR_MIN_Z && pl_pos.vz < FLOOR_MAX_Z ) {

		if ( work->flag == 0 ) {	/* 今電撃床に入った */
			work->cnt = 30;
		}
		work->flag = 1;

		/* ダンボール燃えフラグ ->> okajima/effect/c_box_body.cでダンボール燃え呼び出し */
		if ( GM_PlayerStatus & PLAYER_CB_BOX ) {
			FireBoxFlag = 1;
		}

    }
    else {

		/* 電撃床から飛ばされた後の余韻の体プラズマ */
		if ( work->flag == 1 ) {
			work->body_plsm_cnt = 120;
		}
		work->flag = 0;
   
	}




    /* カウンターに従って呼び出し処理 */
    if ( --work->cnt < 30 ) {

		/* パネル上 */
		if ( work->flag ) {

			if ( --work->cnt1 == 0 ) {
		
				/* 開始ノード取得 */
				switch ( irnd()%6 ) {
				case 0:
					node = 0;
					break;
				case 1:
					node = 12;
					break;
				case 2:
					node = 6;
					break;
				case 3:
					node = 10;
					break;
				case 4:
					node = 4;
					break;
				case 5:
					node = 8;
					break;
				default:
					node = 0;
					ASSERT(0);
					break;
				}

				/* 終端パネル位置取得 */
				pannel = irnd()%12;

				to.vx = pannel_PX[pannel/4] + frnd()*PANNEL_WX;
				to.vy =-100.0f;
				to.vz = pannel_PZ[pannel%4] + frnd()*PANNEL_WZ;
				to.vw = 1.0f;

				/* 中継点取得 */
				_sceVu0AddVector( &middle, &to, (FVECTOR *)GM_PlayerBody->objs->objs[node].world.m[3] );
				_sceVu0ScaleVector( &middle, &middle, 0.50f );

				middle.vx += frnd()*250.0f;
				middle.vy += 500.0f + frnd()*250.0f;
				middle.vz += frnd()*250.0f;
		
				width = 30.0f + frnd()*10.0f;

				/* プラズマ呼び出し */
				NewSpritePlasma( (FVECTOR *)GM_PlayerBody->objs->objs[node].world.m[3], &middle, &to,
								 width, 1.0f, &work->color, 1, 15 );

				GM_SeSetMode( SD_A_ELFSPRK1,
							  &GM_PlayerPosition,
							  GM_SEMODE_NORMAL );

				/* 火花呼び出し */
				DG_COPY_MAT( &mat, &DG_UnitMatrix );
				DG_COPY_VEC( (FVECTOR *)mat.m[3], &to );
				rot_vec.vx = 3072-512;
				rot_vec.vy = 0;
				rot_vec.vz = 0;
				rot_wide.vx = 1024;
				rot_wide.vy = 1024;
				rot_wide.vz = 1;
				NewSpark2( 30,					/* 発生火花数 */
						   &mat,				/* マトリクス */
						   1.0F,				/* 最小スピード */
						   200.0F + 50.0F * frnd(),		/* 幅スピード */
						   10.0F,				/* 重力 */
						   &rot_vec, &rot_wide,			/* 回転 回転幅 */
						   &work->color,				/* 色 */
						   1.0F,				/* スピードに対する火の長さの割合 */
						   20 );				/* 生存フレーム数 */

				work->cnt1 = 3 + irnd()%8;

				work->body_plsm_cnt = 39;	    
	    
			}

		}
		else {
	    
			if ( --work->cnt2 == 0 ) {

				// 開始パネル
				pannel = irnd()%PANNEL_NUM;
		
				from.vx = pannel_PX[pannel/4] + frnd() * PANNEL_WX;
				from.vy =-100.0f;
				from.vz = pannel_PZ[pannel%4] + frnd() * PANNEL_WZ;
				from.vw = 1.0f;
			
				
				// 終端パネル
				/* pannelの値で隣のパネルの値を決めで算出 */
				switch ( pannel ) {
				case 0:
					switch( irnd()%2 ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel += 4;
						break;
					}
					break;
				case 1:
				case 2:
					switch ( irnd()%3 ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel -= 1;
						break;
					case 2:
						pannel += 4;
						break;
					}			
					break;
				case 3:
					switch( irnd()%2 ) {
					case 0:
						pannel -= 1;
						break;
					case 1:
						pannel += 4;
						break;
					}
					break;
				case 4:
					switch( irnd()%2 ){
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel += 4;
						break;
					case 2:
						pannel -= 4;
						break;
					}
					break;
				case 5:
				case 6:
					switch ( irnd()%4 ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel -= 1;
						break;
					case 2:
						pannel += 4;
						break;
					case 3:
						pannel -= 4;
						break;
					}			
					break;
				case 7:
					switch ( irnd()%3 ) {
					case 0:
						pannel -= 1;
						break;
					case 1:
						pannel += 4;
						break;
					case 2:
						pannel -= 4;
						break;
					}
					break;
				case 8:
					switch ( irnd()%2 ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel -= 4;
						break;
					}
					break;
				case 9:
				case 10:
					switch ( irnd()%2 ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel -= 1;
						break;
					case 2:
						pannel -= 4;
						break;
					}			
					break;
				case 11:
					switch ( irnd()%2 ) {
					case 0:
						pannel -= 1;
						break;
					case 1:
						pannel -= 4;
						break;
					}
					break;

				}		    				

			
				to.vx = pannel_PX[pannel/4] + frnd() * PANNEL_WX;
				to.vy =-100.0f;
				to.vz = pannel_PZ[pannel%4] + frnd() * PANNEL_WZ;
				to.vw = 1.0f;

				// 中継点
				_sceVu0AddVector( &middle, &from, &to );
				_sceVu0ScaleVector( &middle, &middle, 0.50f );
				middle.vx += frnd() * 200.0f;
				middle.vy += 300.f + frnd() * 150.0f;
				middle.vz += frnd() * 200.0f;

				// プラズマ太さ
				width = 15.0f + frnd() * 5.0f;


				// プラズマ呼び出し
				NewSpritePlasma( &from, &middle, &to, width, 1.0f, &work->color, 0, 4 );

				GM_SeSetMode( SD_E_HIBANA02,
							  &middle,
							  GM_SEMODE_NORMAL );

				/* 火花呼び出し */
				DG_COPY_MAT( &mat, &DG_UnitMatrix );
				DG_COPY_VEC( (FVECTOR *)mat.m[3], &from );
				rot_vec.vx = 3072-512;
				rot_vec.vy = 0;
				rot_vec.vz = 0;
				rot_wide.vx = 1024;
				rot_wide.vy = 1024;
				rot_wide.vz = 1;
				NewSpark2( 16,					/* 発生火花数 */
						   &mat,				/* マトリクス */
						   1.0F,				/* 最小スピード */
						   100.0F + 25.0F * frnd(),		/* 幅スピード */
						   10.0F,				/* 重力 */
						   &rot_vec, &rot_wide,			/* 回転 回転幅 */
						   &work->color,				/* 色 */
						   1.0F,				/* スピードに対する火の長さの割合 */
						   10 );				/* 生存フレーム数 */


				DG_COPY_VEC( (FVECTOR *)mat.m[3], &to );
				NewSpark2( 16,					/* 発生火花数 */
						   &mat,				/* マトリクス */
						   1.0F,				/* 最小スピード */
						   100.0F + 25.0F * frnd(),		/* 幅スピード */
						   10.0F,				/* 重力 */
						   &rot_vec, &rot_wide,			/* 回転 回転幅 */
						   &work->color,				/* 色 */
						   1.0F,				/* スピードに対する火の長さの割合 */
						   10 );				/* 生存フレーム数 */


				DG_COPY_VEC( &work->light_pos, &middle );
				work->light_life = 4;

				work->cnt2 = 4+irnd()%10;

			}

		}

	
    }


	/* テンプライト設置 */
	if ( work->light_life > 0 ) {
		DG_SetTmpLight2 (
						 &work->light_pos,
						 500.0f + 50.f * (float)work->light_life,
						 1000.0f + 100.f * (float)work->light_life,
						 (int)work->color.vx | (int)work->color.vy<<8 | (int)work->color.vz<<16,
						 LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY );
		work->light_life--;
	}



    /* 体の周りにバチバチプラズマ */
    if ( work->body_plsm_cnt > 0 ) {

		if ( !(work->body_plsm_cnt%3) ) {	    

			width = 20.0f + frnd()*10.0f;

			GM_SeSetMode( SD_E_HIBANA02,
						  &GM_PlayerPosition,
						  GM_SEMODE_NORMAL );

			switch ( irnd()%6 ) {
			case 0:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[14].world.m[2], 10.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[14].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[0].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[15].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 1:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[18].world.m[2], 10.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[18].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[0].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[19].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 2:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[4].world.m[0], 10.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[4].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[11].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[6].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 3:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[8].world.m[0], 80.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[8].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[11].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[10].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 4:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[2].world.m[2], 80.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[2].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[0].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[3].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 5:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[2].world.m[2], -10.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[2].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[0].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[7].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			}

	    
			switch ( irnd()%6 ) {
			case 0:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[14].world.m[2], -10.0f );//-200.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[14].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[0].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[15].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 1:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[18].world.m[2], 10.0f );//-200.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[18].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[0].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[19].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 2:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[4].world.m[0], 10.0f );//150.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[4].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[12].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[6].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 3:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[8].world.m[0], -10.0f );//-150.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[8].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[12].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[10].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 4:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[2].world.m[2], -50.0f );//-250.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[2].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[12].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[13].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			case 5:
				_sceVu0ScaleVector( &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[2].world.m[2], -50.0f );//-250.0f );
				_sceVu0AddVector( &vectmp, &vectmp, (FVECTOR *)GM_PlayerBody->objs->objs[2].world.m[3] );
				NewSpritePlasma2( (FVECTOR *)GM_PlayerBody->objs->objs[12].world.m[3],
								  &vectmp,
								  (FVECTOR *)GM_PlayerBody->objs->objs[17].world.m[3],
								  width,
								  &work->color,
								  4 );
				break;
			}

		}



		/* テンプライト設置 */
		_sceVu0ScaleVector( &vectmp, (FVECTOR*)DG_Chanls[0].eye.m[2], -2000.0f );
		_sceVu0AddVector( &vectmp, &vectmp, &GM_PlayerPosition );
		
		DG_SetTmpLight2 (
						 &vectmp,
						 ( 1500.0f + 250.f * frnd() ) * (float)work->body_plsm_cnt / 40.0f,
						 ( 3000.0f + 500.f * frnd() ) * (float)work->body_plsm_cnt / 40.0f,
						 (int)(work->color.vx) | (int)(work->color.vy)<<8 | (int)(work->color.vz)<<16,
						 LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY );

		work->body_plsm_cnt--;

    }	    
    

    
    if ( work->cnt == 0 ) {
		work->cnt = 30 + irnd()%90;
    }


	}

}


static void Die(Work *work )
{
}


static void InitWork( Work *work )
{
    work->cnt  = 30;
    work->cnt1 = 15 + irnd()%10;
    work->cnt2 = 15 + irnd()%10;
    work->body_plsm_cnt = 0;

	work->se_pos.vx = ( FLOOR_MAX_X + FLOOR_MIN_X ) * 0.5f;
	//work->se_pos.vx = FLOOR_MAX_X;
	work->se_pos.vy = 0.0f;
	work->se_pos.vz = ( FLOOR_MAX_Z + FLOOR_MIN_Z ) * 0.5f;
	work->se_pos.vw = 1.0f;
	work->se_count = 0;

    work->color.vx = 32.0f;
    work->color.vy = 100.0f;
    work->color.vz = 190.0f;
    work->color.vw = 64.0f;

    FireBoxFlag = 0;

}



void *NewElectricFloor( int name, int map )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
	    GV_SetActor( &( work->actor ), Act, Die );
	    GV_ActorEX( &work->actor );
	    work->name = name;
	    InitWork( work );
	}

	return (void *)work ;

}



