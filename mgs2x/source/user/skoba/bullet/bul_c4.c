//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_c4.c
   セットされたＣ４
   
   1999/12/02 M.Sonoyama
   $Id: bul_c4.c,v 1.1.1.3 2002/11/19 11:49:59 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../../kano/attachment/attachment_called.h"
#include	"bul_c4.h"

extern FMATRIX	*C4MAN_GetWorld( int name );
extern int	*C4MAN_GetMap( int name );
extern DG_OBJS	*C4MAN_GetObjs( int name );
extern void *NewUSPLight( FMATRIX *mat, float umbra, float penumbra, float range, int color );


/*------------------------------------------------------------*/

extern	void	*NewBlast2( FMATRIX *, int, int, int, int, int, int ) ;
extern  void    *NewC4_Lamp( FMATRIX * , int , char * , char * , int );
extern  void    *NewC4_LampEX( FMATRIX * , int , char * , char * , int, FVECTOR * );
extern  void    *NewIceComodel( FMATRIX * , int );
extern	void	*NewIceEffect( FMATRIX *pWorld , FMATRIX *light, int str_code, int flag, DG_OBJS *objs ) ;

#define	BODY_NAME	(104754)	/* cfr */
#define BODY_NAME_SCN_1		(9723355)	/* c4_kaitai_a1 */
#define BODY_NAME_SCN_2		(9723387)	/* c4_kaitai_b1 */

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE | DG_FLAG_IRREACTION)
#define	DELAY_BLAST	(8)
#define	COLD_TIME	( DIRECT_TICK( 1800 ) )	// ３秒（６０フレーム時) <-- 小数を表記するため１０倍の値が入っている 

#define		TYPE_NONE		C4_TYPE_NONE
#define		TYPE_SEGMENT	C4_TYPE_SEGMENT
#define		TYPE_FLOOR		C4_TYPE_FLOOR
#define		TYPE_CHARA		C4_TYPE_CHARA
#define		TYPE_CHARA2		C4_TYPE_CHARA2

#define FLUSH_COLD (0x10)

static	FVECTOR	Shift = { 47.5F, -131.5F, 7.5F } ;
int				PL_ProgKaitaiC4BlastProc ;

/*----------------------------------------------------------------*/

/* Ｃ４リストを検索 */
static	Work		*PL_FindC4( int id )
{
	C4_MNG_WORK		*mwork ;
	Work			*list ;

	mwork = C4MngWork ;
	if ( mwork == NULL ) return NULL ;
	list = mwork->list.next ;
	while( list != NULL ) {
		if ( list->n_incremental == id ) {
			return list ;
		}
		list = list->next ;	
	}
	return NULL ;
}

/* Ｃ４リストに登録 */
/* 一番後ろに追加 */
static	void		PL_AddC4List( Work *work )
{
	C4_MNG_WORK		*mwork ;
	Work			*list ;

	mwork = C4MngWork ;
	if ( mwork == NULL ) {
		printf( "warning : C4 Mng not exist!\n" ) ;
		return ;
	}
	list = &mwork->list ;
	while( list->next != NULL ) list = list->next ;
	list->next = work ;
	work->next = NULL ;
	work->n_incremental = mwork->n_incremental ;
	mwork->n_bombs ++ ;
	mwork->n_incremental ++ ;
    if ( !( work->flag & FLAG_KAITAI ) ) GM_N_C4Bombs ++ ;
}

/* Ｃ４リストから削除 */
static	void	   	PL_RemoveC4List( Work *work )
{
	C4_MNG_WORK		*mwork ;
	Work			*list, *prev ;
	int				remove ;

	mwork = C4MngWork ;
	if ( mwork != NULL ) {
		prev = &mwork->list ;
		list = mwork->list.next ;
		remove = 0 ;
		while( list != NULL ) {
			if ( list->n_incremental == work->n_incremental ) {
				prev->next = list->next ;
				remove = 1 ;
				break ;
			}
			prev = list ;
			list = list->next ;
		}
		if ( remove == 0 ) return ;
		mwork->n_bombs -- ;
	}
    if ( !( work->flag & FLAG_KAITAI ) ) GM_N_C4Bombs -- ;
}

/*----------------------------------------------------------------*/

/* プロック実行 */
static	void	ExecProc( Work *work )
{
	int			buf[ 5 ] ;
	GCL_ARGS	args ;

	if ( ( work->flag & FLAG_FATMAN ) &&
		 ( work->blast_count <= 0 || ( work->flag & FLAG_DESTROY ) ) ) {
		work->proc = PL_ProgKaitaiC4BlastProc ;
	}
	if ( work->proc == 0 ) {
		return ;
	}

	args.argc = 5 ;
	args.argv = buf ;

	buf[ 0 ] = work->name ;
	if ( work->flag & FLAG_DESTROY ) {
		buf[ 1 ] = 0 ;
	} else if ( (GM_GameStatus & STATE_VR_ANOTHER) && (work->flag & FLAG_FALL) ) {
		buf[ 1 ] = 3 ;
	} else if ( work->flag & FLAG_FREEZE ) {
		buf[ 1 ] = 1 ;
	} else {
		buf[ 1 ] = 2 ;
	}

	if ( ( work->flag & FLAG_FATMAN ) && ( work->blast_count <= 0 ) ) buf[ 1 ] = 0 ;

	buf[ 2 ] = work->world.m[ 3 ][ 0 ] ;
	buf[ 3 ] = work->world.m[ 3 ][ 1 ] ;
	buf[ 4 ] = work->world.m[ 3 ][ 2 ] ;

	if ( !GM_IsGameOver() ) GM_ForceExecProc( work->proc, &args ) ;
//	work->proc = 0 ;
}

/*------------------------------------------------------------*/

/* ターゲットコールバック */
static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work;
//	FMATRIX  fmat;
//	FVECTOR   tmp;
//	SVECTOR   rot;

    work = ( Work * )ptr ;
	/* 凍っている奴は爆発しない */
    if ( work->flag & ( FLAG_DESTROY | FLAG_FREEZE ) ) return ;
    if ( def->damaged & TARGET_POWER ) {
		if ( def->weapon_type & WP_BLAST ) {
#if 0
			if ( work->flag & FLAG_FATMAN ) {
				def->damaged &= ~TARGET_POWER ;
				def->weapon_type = I64(0) ;				
				return ;
			}
#endif
			work->flag |= FLAG_DESTROY | FLAG_FREE ;
			work->delay = DELAY_BLAST ;
			GM_FreeTarget( def ) ;
		} else if ( def->weapon_type & ( WP_BULLET | WP_M92 ) ) {
#if 0
			if ( work->flag & FLAG_FATMAN ) {
				def->damaged &= ~TARGET_POWER ;
				def->weapon_type = I64(0) ;				
				return ;
			}
#endif
			work->flag |= FLAG_DESTROY | FLAG_FREE ;
			work->delay = 1 ;
			GM_FreeTarget( def ) ;
		} else {
			if ( ( def->weapon_type & WP_COLDSPRAY ) &&
				( work->flag & FLAG_KAITAI ) &&
				!( work->flag & FLAG_NO_SPRAY ) &&
				!( work->flag & FLAG_FREEZE ) ) {
				work->lamp_status |= LAMP_STATE_FREEZING ;
				work->cold_count += off->power->damage ;
//				printf("cold_count = %d , damege %d\n" ,work->cold_count , off->power->damage );
				GV_CallChildSignalFunc( work , 0x02 , work->cold_count );
				if ( work->cold_count >= COLD_TIME ) {
					work->flag |= FLAG_FREEZE ;
					if ( work->type == TYPE_CHARA ) {
						if ( work->id != -1 ) TurnOnOff_IR_called( work->id, 0 ) ;
					} else {
						work->body.objs->flag &= ~DG_FLAG_IRREACTION ;
#if 1
						{/* 赤外線反応フラグは各オブジェクトユニット単位でフラグを見ているため */
							int		i ;
							for ( i = 0 ; i < work->body.objs->n_models ; i++ ){
								work->body.objs->objs[ i ].flag &= ~DG_FLAG_IRREACTION ;
							}
						}
#endif
					}
					work->freeze_disp = 0 ;
				    GV_CallChildSignalFunc( work , 0x04 , work->cold_count );
					ExecProc( work ) ;
					//printf( "freeze!!\n" ) ;
				}
//			    GV_CallChildSignalFunc( work , 0x08 , work->cold_count );
			}
			def->damaged &= ~TARGET_POWER ;
			def->weapon_type = I64(0) ;
		}
    }
}

/* 動的ハザードコールバック */
static	void	DynamicHzdCallback( void *hzd, void *ptr, void *ptr2, int flag )
{
	Work			*work ;
	FMATRIX			old, new ;

	work = ( Work * )ptr ;
	if ( flag & ( HZX_CALLBACK_FLAG_DESTROY | HZX_CALLBACK_FLAG_PITFALL ) ) {
        work->flag |= FLAG_FALL ;
		HZX_RemoveDynamicCallback( work->stick, &work->dhcb ) ;
		work->stick = NULL ;
//		GV_DestroyActor( work ) ;
		return ;
	}
	DG_COPY_MAT( &old, &work->world ) ;
	HZX_CallbackUpdateMatrix( hzd, &old, &new ) ;
	DG_SetPos( &new ) ;
	DG_GetPos( &work->world ) ;
	DG_GetPos( &work->body.objs->world ) ;	

	DG_MovePos( &work->shift ) ;
	DG_GetPos( &new ) ;
	if ( work->flag & FLAG_KAITAI ) GM_MoveTarget2Map( &work->target, &new, work->map ) ;
	else							GM_MoveTarget3Map( &work->target, &new, work->map ) ;
}

/*------------------------------------------------------------*/

#if 0
static	void	DispFreeze( Work *work, int count )
{
	FVECTOR		ret, pos ;
	DG_CHANL	*cp ;
	int			alpha, flag ;

	if ( work->freeze_disp >= 256 ) return ;

	flag = 0 ;
	cp = DG_Chanl( 0 ) ;
	GV_MatToVec( &work->world, &pos ) ;
	pos.vw = 1.0F ;
	_sceVu0ApplyMatrix( &ret, &cp->eye_pers, &pos ) ;
	if ( ret.vz > ret.vw ) flag = 1 ;
	if ( ret.vw < 0.0F ) ret.vw = -ret.vw ;
	
	if ( ( ret.vx > ret.vw * 2.0F ) || ( ret.vx < ( -ret.vw * 2.0F ) ) ) flag = 1 ;
	if ( ( ret.vy > ret.vw ) || ( ret.vy < -ret.vw ) ) flag = 1 ;

	if ( GM_CheckPlayerStatus( PLAYER_BEHIND | PLAYER_INTRUDE | PLAYER_WATCH ) ) {
		if ( ret.vy > ret.vw * 0.5F ) {
			ret.vy = ret.vw * 0.5F ;
		} else if( ret.vy < -ret.vw * 0.5F ) {
			ret.vy = -ret.vw * 0.5F ;
		}
		if ( ret.vx > ret.vw * 0.5F ) {
			ret.vx = ret.vw * 0.5F ;
		} else if( ret.vx < -ret.vw*0.5f ) {
			ret.vx = -ret.vw * 0.5F ;
		}
	}

	if ( !flag && work->freeze_disp == 0 ) {
		work->freeze_disp = 1 ;	// 表示すべき位置計算初期化された
		work->disp_x = (int)((ret.vx / ret.vw) * DRAW_WIDTH/2) + DRAW_WIDTH/2 ;
		work->disp_y = (int)((ret.vy / ret.vw) * DRAW_HEIGHT/2) + DRAW_HEIGHT/2 - 32;
	} else if ( work->freeze_disp == 0 ) {
		return;
	}

	if ( count < 0 ) work->freeze_disp += 2 ;
	alpha = 256 - work->freeze_disp ;
	if ( alpha < 1 ) return ;

	MENU_SetAlphaMode( 0, 1, 0, 1, 0 ) ;
	MENU_Locate( work->disp_x, work->disp_y, MENU_MODE_CENTER ) ;
	MENU_Color( 96, 112, 232, alpha ) ;
	if ( count < 0 ) MENU_Printf( "FREEZED\n" ) ;
	else			 MENU_Printf( "%03d\n", count ) ;
}
#endif


/* 場所から床傾き計算 */
static void	SetGRotFromPos( SVECTOR *rot, FVECTOR *pos, HZX_GROUP_ID hzx_id, int seg_type, int flr_type )
{
    FVECTOR	head, vec ;
    float	hh, diff ;
    int		turn, flag ;	
	float		levels[ 2 ] ;
	HZX_HZD		flr[ 2 ] ;
	FVECTOR		norm ;

    flag = HZX_LevelHazardCheck( hzx_id, pos, seg_type, flr_type ) ;
	if ( flag != 0 ) {
		HZX_GetLevelHeight( levels ) ; 
		HZX_GetLevelHazard( flr, NULL ) ;
	}
	if ( !( 1 & flag ) ) return ;

    norm.vx = flr->p1.h ;
    norm.vz = flr->p2.h ;
    norm.vy = 0 ;

	rot->vx = 0 ;
	rot->vy = GV_VecDir2( &norm ) ;

printf(" level slope [%d] \n",rot->vy);

    DG_SetPos2( pos, rot ) ;
    head.vx = head.vy = 0.0F ;
    head.vz = 1000.0f ; 
    DG_PutVector( &head, &head, 1 ) ;
    HZX_SlopeFloorLevel( &hh, &head, flr ) ;
//printf(" hh=%f head=%f floor.y=%f!!\n",hh, head.vz, levels[0]);
    diff = hh - levels[ 0 ] ;
    vec.vx = diff ; 
    vec.vz = 1000.0f ;
    turn = - GV_VecDir2( &vec ) ;
	if ( turn < -2048 ) turn += 4096 ;
	
	rot->vx = turn ;
}

/* 落下処理 */
static	void	Fall( Work *work )
{
	CONTROL		control ;
	FVECTOR		mov ;
	FMATRIX		world ;

	GV_MatToVec( &work->body.objs->world, &mov ) ;
	GV_ZeroMemory( &control, sizeof( CONTROL ) ) ;
	GM_InitControl( &control, 0, work->map ) ;
	GM_ConfigControlHazard( &control, 40, 400, 300 ) ;
	GM_ConfigControlHzxHeight( &control, 40.0F, mov.vy - 40.0F ) ;
	GM_ConfigControlHzxCheckFlag( &control, HZX_SEG_NO_C4, 
								  HZX_FLOOR_PITFALL ) ;
	control.hzx_check_type |= HZX_CHK_RECOIL_TYPE_ONLY ;
	control.map = work->map ;
	control.hzx_id = GM_GetHzxGroupID( work->map ) ;
	DG_COPY_VEC( &control.mov, &mov ) ;

	if ( !(GM_GameStatus & STATE_VR_ANOTHER) || !(work->flag & FLAG_KAITAI) ) {
		GV_SetVec3( &control.rot, 0, 0, 1024 - 256 ) ; 
	} else {
		GV_SetVec3( &control.rot, -1024, 0, 0) ; 
		SetGRotFromPos( &control.rot, &mov, control.hzx_id, HZX_SEG_NO_C4, HZX_FLOOR_PITFALL ) ;
	    DG_SetPos2( &control.mov, &control.rot ) ;
	}

	control.turn = control.rot ;
	control.step.vy = -64.0F ;
	GM_ActControl( &control ) ;
	if ( control.grounded & 1 ) {
		work->flag &= ~FLAG_FALL ;
		if ( !(GM_GameStatus & STATE_VR_ANOTHER) ) {
			/* ついた床がＣ４つけられないとき消す */
			if ( control.flr_atrs[ 0 ] & HZX_FLOOR_NO_C4 ) {
				work->delay = 1 ;
				work->flag |= FLAG_NO_BLAST ;
			}
		}
	}
	/* 1000000以下で消す */
	if ( control.mov.vy < GM_WORLD_LIMIT_BOTTOM ||
		 control.mov.vy < GM_WaterLevel ) {
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			work->flag &= ~FLAG_FALL ;
		} else {
			work->delay = 1 ;
			work->flag |= FLAG_NO_BLAST ;
		}
	}

	DG_PutObjs( work->body.objs ) ;
	DG_GetPos( &work->world ) ;

	DG_MovePos( &work->shift ) ;
	DG_GetPos( &world ) ;
	GM_MoveTarget2( &work->target, &world ) ;	

	//NewTargetView2( &work->target, 32, 232, 32 ) ;

	GM_FreeControl( &control ) ;
}

/* メッセージチェック */
static	void	CheckMessage( Work *work )
{
	int			n_msg ;
	GV_MSG		*msg ;

	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		if ( msg->message[ 0 ] == 0 ) {		/* スプレー効く／効かない */
			if ( msg->message[ 1 ] == 0 ) {
				work->flag |= FLAG_NO_SPRAY ;
			} else {
				work->flag &= ~FLAG_NO_SPRAY ;
			}
		} else if ( msg->message[ 0 ] == 1 ) {
			/* 爆破 */
			if ( !( work->flag & ( FLAG_DESTROY | FLAG_FREEZE | FLAG_COUNTDOWN ) ) ) {
				work->flag |= FLAG_DESTROY | FLAG_FREE ;
				work->delay = 1 ;
				GM_FreeTarget( &work->target ) ;
			}			
		}
		msg ++ ;
	}
}

/*------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    int		time ;

	/* 解体用はメッセージチェック */
	if ( work->flag & FLAG_KAITAI ) {
		CheckMessage( work ) ;
	}

	/* ターゲット移動 */

	if ( (work->type != TYPE_CHARA) && (work->type != TYPE_CHARA2) ) {
		GM_SetCurrentMap( work->map ) ;
		//NewTargetView2( &work->target, 32, 232, 32 ) ;
		
		if ( work->flag & FLAG_LIKE_CEILING ) {
			extern int	KR_CeilingCheck( float height ) ;
			work->body.objs->flag = KR_CeilingCheck( work->body.objs->world.m[3][1] ) ;
			work->body.objs->flag |= BODY_FLAG ;
		}
	} else if ( work->type == TYPE_CHARA2 ) {
		FMATRIX			world ;
		SVECTOR			rot ;

		GM_SetCurrentMap( *work->effect_map ) ;
		DG_SetPos( work->effect_world ) ;
		DG_RotatePos( &work->shift_rot ) ;
		DG_MovePos( &work->shift ) ;
		DG_PutObjs( work->body.objs ) ;
		GM_GroupObjs( work->body.objs, *work->effect_map ) ;
		if( work->effect_objs->flag & DG_FLAG_INVISIBLE ) {
			 work->body.objs->flag |= DG_FLAG_INVISIBLE ;
		} else {
			 work->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
		}

		if ( !( work->flag & FLAG_KAITAI ) ) {
			GV_SetVec3( &rot, -1024, 990, 0 ) ;
			DG_RotatePos( &rot ) ;
			DG_GetPos( &work->world ) ;
			DG_MovePos( &Shift ) ;
			DG_GetPos( &world ) ;
		} else {
			DG_GetPos( &work->world ) ;
			DG_COPY_MAT( &world, &work->world ) ;
		}
		GM_MoveTarget2( &work->target, &world ) ;

		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			extern int	C4MAN_GetStatus( int name ) ;
			int		c4_status ;
			
			c4_status = C4MAN_GetStatus( work->name ) ;
			if ( c4_status & C4MAN_STATUS_DROP ) {
				printf( "c4 toreta!!\n" ) ;

				work->flag |= FLAG_FALL ;
				work->type = TYPE_NONE ;
				ExecProc( work ) ;
				GM_FreeObject( &work->body ) ;
				GM_InitObject( &( work->body ), work->model, BODY_FLAG ) ;
				GM_ConfigObjectLight( &work->body, work->lights ) ;
				DG_GetLightMatrix( ( FVECTOR * )&work->body.objs->world.m[ 3 ], work->lights ) ;
				DG_COPY_MAT( &work->body.objs->world, &work->world ) ;
				work->body.map_name = work->map ;
				GM_GroupObjs( work->body.objs, work->map ) ;
				work->this.mov = ( FVECTOR * )work->body.objs->world.m[ 3 ] ;
				DG_COPY_VEC( &work->shift, &Shift ) ;
				work->stick = NULL ;

				/* エフェクト張りなおし */
				if ( work->flag & FLAG_KAITAI ) {
					if ( work->flag & FLAG_FREEZE ) {
						GV_DestroyOtherActor( work->effect_ice ) ;
						GV_SetActorChild( work , (work->effect_ice = 
							NewIceEffect( &work->body.objs->world , work->lights, work->effect_name, work->flag, work->body.objs) ) ) ;
					    GV_CallChildSignalFunc( work , 0x04 , work->cold_count );
					} else {
						NewBlast2( &work->world, work->side, 1500, 3000, 0, 0, WP_C4Bomb ) ;
					}
				}
			}
		}

//		NewTargetView2( &work->target, 32, 232, 32 ) ;
	} else {
		OBJECT			*body ;
		CAPTURE_TARGET	*capture ;
		SVECTOR			rot ;
		FMATRIX			world ;
		
		capture = ( CAPTURE_TARGET * )work->stick ;
		body = capture->body ;
		GM_SetCurrentMap( body->map_name ) ;
		work->map = body->map_name ;
		DG_SetPos( &body->objs->objs[ work->enemy_joint ].world ) ;

		if ( !( work->flag & FLAG_KAITAI ) ) {
			DG_MovePos( &work->shift ) ;			
			GV_SetVec3( &rot, -1024, 990, 0 ) ;
			DG_RotatePos( &rot ) ;
			DG_GetPos( &work->world ) ;
			DG_MovePos( &Shift ) ;
			DG_GetPos( &world ) ;
		} else {
			DG_MovePos( &work->shift ) ;
			DG_RotatePos( &work->shift_rot ) ;
			DG_GetPos( &work->world ) ;
			DG_COPY_MAT( &world, &work->world ) ;
		}
		GM_MoveTarget2( &work->target, &world ) ;
		//NewTargetView2( &work->target, 32, 232, 32 ) ;

		/* 取れた */
		if ( !( capture->flag & CAPTURE_C4EXIST ) ) {
			printf( "toreta!!\n" ) ;
            if ( work->flag & FLAG_BELTOBJ ) {
				/* ベルトコンベアの場合、壊れモデルにつけかえる */
				KillAttachment_called( work->id ) ;
				/* 付け直し */
				work->id = MakeAttachment3A_called( work->objinfo->model, 
												    &work->objinfo->rot, 
												    capture->body,
												    work->objinfo->joint, 
												    &work->objinfo->mov, 0 ) ;
				capture->flag |= CAPTURE_C4EXIST ;
			} else {
				work->flag |= FLAG_FALL ;
				work->type = TYPE_NONE ;
				ExecProc( work ) ;
				if ( work->id != -1 ) KillAttachment_called( work->id ) ;
				work->id = -1 ;
				GM_InitObject( &( work->body ), work->model, BODY_FLAG ) ;
				GM_ConfigObjectLight( &work->body, work->lights ) ;
				DG_GetLightMatrix( ( FVECTOR * )&work->body.objs->world.m[ 3 ], work->lights ) ;
				DG_COPY_MAT( &work->body.objs->world, &work->world ) ;
				work->body.map_name = work->map ;
				GM_GroupObjs( work->body.objs, work->map ) ;
				work->this.mov = ( FVECTOR * )work->body.objs->world.m[ 3 ] ;
				DG_COPY_VEC( &work->shift, &Shift ) ;
				work->stick = NULL ;

				/* エフェクト張りなおし */
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					if ( work->flag & FLAG_KAITAI ) {
						if ( work->flag & FLAG_FREEZE ) {
							GV_DestroyOtherActor( work->effect_ice ) ;
							GV_SetActorChild( work , (work->effect_ice = 
								NewIceEffect( &work->body.objs->world , work->lights, work->effect_name, work->flag, work->body.objs) ) ) ;
						    GV_CallChildSignalFunc( work , 0x04 , work->cold_count );
						} else {
							NewBlast2( &work->world, work->side, 1500, 3000, 0, 0, WP_C4Bomb ) ;
						}
					}
				}

			}
		}
	}

	/* 落下処理 */
	if ( work->flag & FLAG_FALL ) {
		Fall( work ) ;
	}

	/* 解体Ｃ４カウントダウン */
	if ( ( work->flag & FLAG_KAITAI ) &&
		 ( work->flag & FLAG_COUNTDOWN ) &&
		 ( work->blast_count > 0 ) && 
		 !( work->flag & ( FLAG_DESTROY | FLAG_FREEZE ) ) ) {
		if ( -- work->blast_count <= 0 ) {
			work->flag |= FLAG_DESTROY | FLAG_FREE ;
			work->delay = 1 ;
			GM_FreeTarget( &work->target ) ;
		}
	}
	
	/* ボムリストメッセージから消す */
	if ( work->this.flag & GM_BMB_FLAG_DESTROY ) {
		work->flag |= FLAG_NO_BLAST | FLAG_DESTROY ;
		work->delay = 1 ;
	}

    if ( work->delay > 0 ) {
		if ( -- work->delay == 0 ) {
			if ( work->flag & FLAG_KAITAI ) {
				/* 解体用はダメージ無し */
				if ( !GM_IsGameOver() ) {
					NewBlast2( &work->world, work->side, 1500, 3000, 0, 0, WP_C4Bomb ) ;
				}
			} else if ( !( work->flag & FLAG_NO_BLAST ) ) {
				NewBlast2( &work->world, work->side, 1500, 3000, DMG_BLAST, FNT_BLAST, WP_C4Bomb ) ;
			}
			work->flag |= FLAG_DESTROY ;
			ExecProc( work ) ;
			GV_DestroyActor( work ) ;
		}
    }

	if ( work->flag & FLAG_KAITAI ) {
		goto blast_check_skip ;
	}

    time = GV_Time ;
	
	if ( work->flag & FLAG_SWITCH ) {
		work->flag |= FLAG_DESTROY | FLAG_FREE ;
		NewBlast2( &work->world, work->side, 1500, 3000, DMG_BLAST, FNT_BLAST, WP_C4Bomb ) ;
		GM_FreeTarget( &work->target ) ;
		ExecProc( work ) ;
		GV_DestroyActor( work ) ;
		return ;
	} 
blast_check_skip :
	if ( work->flag & FLAG_FREEZE ) {
#if 0
		DispFreeze( work, -1 ) ;
#endif
	} else {
#if 0
		if ( work->cold_count > 0 ) {
			DispFreeze( work, work->cold_count  ) ;
		}
#endif
#if 0
		if ( work->flag & FLAG_SMELL ) {
			if ( PL_GetPlayerItem() == IT_BombSenserA ) {
				GM_RadarSetFlag( &work->radar, RADAR_VISIBLE | RADAR_SIGHT ) ;
			} else {
				GM_RadarResetFlag( &work->radar, RADAR_VISIBLE | RADAR_SIGHT ) ;
			}
		}
#endif
	}
}

static	void	Die( work )
Work		*work ;
{
	if ( work->type != TYPE_CHARA ) {
		if ( work->type == TYPE_CHARA2 ) {
			GM_FreeObject( &work->body ) ;
		} else {
printf("kokkokanaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
			HZX_RemoveDynamicCallback( work->stick, &work->dhcb ) ;
			GM_FreeObject( &work->body ) ;
		}
	} else {
		( ( CAPTURE_TARGET * )work->stick )->flag &= ~CAPTURE_C4EXIST ;
		if ( work->id != -1 ) KillAttachment_called( work->id ) ;
	}
	if ( !( work->flag & FLAG_KAITAI ) ) {
		GM_RemoveBombList( &work->this ) ;
#if 0
		GM_FreeEneFind( &work->ef ) ;
#endif
	} 
    if ( !( work->flag & FLAG_FREE ) ) GM_FreeTarget( &work->target ) ;
#if 0
	if ( work->flag & FLAG_SMELL ) {
		GM_FreeRadarControl( &work->radar ) ;
	}
#endif
	if ( work->objinfo != NULL ) GV_Free( work->objinfo ) ;
	PL_RemoveC4List( work ) ;
}

/*------------------------------------------------------------*/

static	void	SetTarget( work, world, scn )
Work			*work ;
FMATRIX			*world ;
int				scn ;
{
    TARGET	*t ;
    FVECTOR	size, shift, ofs ;
	FMATRIX	tworld ;

    t = &( work->target ) ;
	DG_SetPos( world ) ;
	DG_PutVector( &work->shift, &shift, 1 ) ;
	if ( work->flag & FLAG_KAITAI ) {
		if ( scn == 1 && GCL_GetOption( 'Z' ) != NULL ) {
			size.vx = ( float )GCL_GetNextInt() ;
			size.vy = ( float )GCL_GetNextInt() ;
			size.vz = ( float )GCL_GetNextInt() / 2.0F ;
			GV_SetVec3( &ofs, 0.0F, 0.0F, size.vz ) ;
		} else {
			GV_SetVec3( &size, 125.0F, 150.0F, 75.0F ) ;
			size.vx *= work->scale ;
			size.vy *= work->scale ;
			size.vz *= work->scale ;
			GV_SetVec3( &ofs, 0.0F, 0.0F, 75.0F * work->scale ) ;
		}
		GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER | TARGET_SEEK | 
					  TARGET_ROTATE | TARGET_NO_CLAYMORE, 0,
					  ENEMY_SIDE, &size, &ofs ) ;
		DG_COPY_MAT( &tworld, world ) ;
		GV_VecToMat( &shift, &tworld ) ;
		GM_MoveTarget2( t, &tworld ) ;
	} else {
		size.vx = size.vz = 50.0F ; size.vy = 50.0F ;
		GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER | TARGET_SEEK | TARGET_NO_CLAYMORE, 0,
					  BOTH_SIDE, &size, &DG_ZeroVector ) ;
		GM_MoveTarget( t, &shift ) ;
	}
    GM_SetTargetCallBack( t, Hit, work ) ;
    GM_PutTarget( t ) ;
}

static	int	GetResources( work, side, world, seg, type ) 
Work		*work ;
int			side ;
FMATRIX		*world ;
HZX_HZD		*seg ;
u_int		type ;
{
	extern	void	VertexSearch( FVECTOR *, FVECTOR *, DG_OBJS *, int, FVECTOR * ) ;
	FVECTOR		mov, vans ;
	SVECTOR		rot ;
	OBJECT		*body ;

    work->world = *world ;
    work->side = side ;
    work->type = type ;
    work->flag = FLAG_PLAYER ;
	work->proc = 0 ;
	work->name = WP_C4Bomb ;
	work->model = BODY_NAME ;
	work->scale = 1.0F ;

	if ( type != TYPE_CHARA && type != TYPE_CHARA2 ) {
		/* 壁・床 */
		GM_InitObject( &( work->body ), BODY_NAME, BODY_FLAG ) ;
		if ( work->body.objs == NULL ) return -1 ;
		work->body.objs->world = *world ;
		GM_ConfigObjectLight( &work->body, work->lights ) ;

		{
			HZX_ZONE_ADD	zone[ 2 ] ;
			int				hzx_id, map ;

			HZX_GetInterruptZone( ( FVECTOR * )world->m[ 3 ], 0, 150, &zone[ 0 ] ) ;
			hzx_id = GV_GetBit( HZX_ZoneMapNo( zone[ 0 ] ) ) ;
			hzx_id |= GV_GetBit( HZX_ZoneMapNo( zone[ 1 ] ) ) ;
			map = GM_GetMapIDfromHzxGroupID( hzx_id ) ;
			GM_GroupObjs( work->body.objs, map ) ;
		}

		DG_GetLightMatrix( ( FVECTOR * )&work->body.objs->world.m[ 3 ], work->lights ) ;
		work->map = GM_CurrentMap ;
		work->id = -1 ;
#if 1
		if ( HZX_IsDynamic( seg ) ) {
			work->stick = seg->ptr ;
			HZX_SetDynamicCallback( seg->ptr, &work->dhcb, DynamicHzdCallback, 
								    ( void * )work, NULL ) ;
		} else {
			work->stick = NULL ;
		}
#else
		/* 中心座標でチェックし直す */
		{
			FMATRIX		chkWorld ;
			FVECTOR		chkPos ;
			HZX_HZD		level[ 2 ] ;
			float		levels[ 2 ] ;

			DG_SetPos( world ) ;
			DG_MovePos( &Shift ) ;
			DG_GetPos( &chkWorld ) ;
			GV_MatToVec( &chkWorld, &chkPos ) ;
			chkPos.vy += 100.0F ;
			work->stick = NULL ;
			if ( HZX_LevelHazardCheck( HZX_CurrentGroupID, &chkPos, HZX_CHK_ALL,
									   HZX_FLOOR_NO_C4 | HZX_FLOOR_RECOIL_TYPE ) & 1 ) {
				HZX_GetLevelHazard( level, NULL ) ;
				HZX_GetLevelHeight( levels ) ;
				if ( HZX_IsDynamic( &level[ 0 ] ) ) {
					if ( DG_FABS( chkPos.vy - levels[ 0 ] ) < 300.0F ) {
						work->stick = level[ 0 ].ptr ;
						HZX_SetDynamicCallback( level[ 0 ].ptr, &work->dhcb, DynamicHzdCallback, 
											    ( void * )work, NULL ) ;
					} 
				} 
			}
		}
#endif
		DG_COPY_VEC( &work->shift, &Shift ) ;
		body = &work->body ;
	} else {
		/* キャラクタ */
		work->stick = seg ;
		body = ( ( CAPTURE_TARGET * )seg )->body ;
		GV_MatToVec( world, &mov ) ;
		//VertexSearch( &vans, &nans, body->objs, HUMAN21_KOSHI, &mov ) ;
		//rot = DG_ZeroSVector ;
		GV_SetVec3( &vans, -150.0F, 50.0F, -75.0F ) ; /* 位置を決めうち */
		GV_SetVec3( &rot, -1024, 990, 0 ) ;
		work->id = MakeAttachment3A_called( BODY_NAME, &rot, body,
										    HUMAN21_KOSHI, &vans, 4 ) ;
		DG_COPY_MAT( &work->world, &body->objs->objs[ HUMAN21_KOSHI ].world ) ;
		DG_COPY_VEC( &work->shift, &vans ) ;
	}

	SetTarget( work, world, 0 ) ;

#if 0
	/* 敵発見設定 */
	GV_MatToVec( &work->world, &mov ) ;
	GM_SetEneFind( &work->ef, &mov, GM_PlayerAddress, EF_TYPE_LV3 | EF_TYPE_C4 ) ;
	GM_PutEneFind( &work->ef ) ;
#endif

//	GM_InitRadarControl( &work->radar, &mov, 0, GM_CurrentMap ) ;
//	GM_RadarSetSight( &work->radar, 0, 4096, 200.0F, RADAR_COLOR_YELOW ) ;

	work->this.weapon = WP_C4Bomb ;
	work->this.mov = ( FVECTOR * )body->objs->world.m[ 3 ] ;

	GM_AddBombList( &work->this ) ;

	PL_AddC4List( work ) ;
	work->blast_count = -1 ;

    return 0 ;
}

/* Ｃ４起動 */
void	*NewBulletC4( world, side, seg, type )
FMATRIX		*world ;
int			side ;
HZX_HZD		*seg ;
int			type ;
{
    Work	*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, side, world, seg, type ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		if ( GM_ShootCount < 30000 ) ++ GM_ShootCount ;
		GM_SetWeaponFire( WP_C4Bomb ) ;
    }
    return work ;
}

/*------------------------------------------------------------*/

static	int	GetResourcesKaitai( Work *work, int name, int where, 
							    FMATRIX *world, int count, int scn )
{
	FVECTOR		mov, shift_mov ;
	SVECTOR		rot, shift_rot ;
	int			model, bodyflag, status ;
	FMATRIX		*effect_world ;
	
	status = 0 ;
	work->name = name ;
	work->map = where ;

	effect_world = NULL ;

	if ( scn == 1 ) {
		PL_GetOptionFV( 'p', &mov ) ;
		PL_GetOptionSV( 'r', &rot ) ;
		work->shift_rot = rot ;
		shift_mov = mov ;
		shift_rot = rot ;
		DG_SetPos2( &mov, &rot ) ;
		if ( GCL_GetOption( 'l' ) != NULL ) {
			work->lamp_shift.vx = ( float )GCL_GetNextInt() ;
			work->lamp_shift.vy = ( float )GCL_GetNextInt() ;
			work->lamp_shift.vz = ( float )GCL_GetNextInt() ;
			//GV_SetVec3( &work->lamp_shift, 91.0F, 16.0F, 229.0F ) ;
		} else {
			GV_SetVec3( &work->lamp_shift, -38.0F, 93.0F, 133.0F ) ;
		}
	} else {
		work->shift_rot = DG_ZeroSVector ;
		DG_SetPos( world ) ;
		GV_SetVec3( &work->lamp_shift, -38.0F, 93.0F, 133.0F ) ;
	}

	DG_GetPos( &work->world ) ;
    work->side = BOTH_SIDE ;
    work->type = TYPE_NONE ;
    work->flag = FLAG_KAITAI ;
	work->stick = NULL ;

	if ( scn == 1 ) {
		/* ＶＲ解体Ｃ４用に追加 */
		work->flag |= GCL_GetOptionValue( 'f' , 0 );
	}

	// ランプの追加
	if ( scn ) {
		if ( work->flag & FLAG_LIKE_CEILING ) {
			extern void *NewC4_LampCeiling( FMATRIX *pWorld , int where , char *pStatus ,char *pLevel , int name, FVECTOR *shift) ;
			GV_SetActorChild( work , NewC4_LampCeiling( &work->world, where, NULL, NULL, 0, 
												   &work->lamp_shift ) ) ;
		} else {
			GV_SetActorChild( work , NewC4_LampEX( &work->world, where, NULL, NULL, 0, 
												   &work->lamp_shift ) ) ;
		}
	} else {
		void		*child ;

		child = NewC4_Lamp( &work->world, where, 
						    &work->lamp_status, &work->lamp_count , 0 ) ;
	    if ( child != NULL ) {
			extern	void	PL_AddC4LampParam( void *ptr, int *Count, int *No ) ;

			GV_SetActorChild( work, child ) ;
			PL_AddC4LampParam( child, &work->blast_count, &work->n_incremental ) ;
		}
		work->lamp_status = 0 ;
		work->lamp_count = LAMP_COLOR_MAX ;
		work->flag |= FLAG_FATMAN ;
	}

	if ( scn == 1 ) {
		if ( GCL_GetOption( 't' ) != NULL ) {
			if ( GCL_GetNextInt() != 0 ) {
				work->flag |= FLAG_NO_SMELL ;
			} else {
				work->flag |= FLAG_SMELL ;
			}
		} else {
			work->flag |= FLAG_SMELL ;
		}
	} else {
		work->flag |= FLAG_SMELL ;
	}
	// big?
	work->scale = 1.0F ;
	{
		FVECTOR fvtmp;
		int scale;

		if ( scn == 1 ) {
			scale =	GCL_GetOptionValue( 'b' , 10 );
	
			fvtmp.vx = ( float )scale / 10.0f;
			fvtmp.vy = ( float )scale / 10.0f;
			fvtmp.vz = ( float )scale / 10.0f;
			fvtmp.vw = 1.0f;

			DG_SetPos( &work->world );
			DG_ScalePos( &fvtmp );
			DG_GetPos( &work->world );
			work->scale = ( float )scale / 10.0F ;
		}
	}

	if ( work->flag & FLAG_SMELL ) {
		model = BODY_NAME_SCN_1 ;
	} else {
		model = BODY_NAME_SCN_2 ;
	}

	if ( !(GM_GameStatus & STATE_VR_ANOTHER) ) {
		if ( work->flag & FLAG_SMELL ) {
			/* においつき */
			if ( scn == 1 && GCL_GetOption( 'i' ) != NULL ) {
				GV_SetActorChild( work , NewIceComodel( &work->world , GCL_GetNextInt() ) ) ;
			} else {
				GV_SetActorChild( work , NewIceComodel( &work->world , GV_StrCode("c4_kaitai_a1_frost_cm") ) );
			}
		} else {
			/* においなし */
			if ( scn == 1 && GCL_GetOption( 'i' ) != NULL ) {
				GV_SetActorChild( work , NewIceComodel( &work->world , GCL_GetNextInt() ) ) ;
			} else {
				GV_SetActorChild( work , NewIceComodel( &work->world , GV_StrCode("c4_kaitai_b1_frost_cm") ) );
			}
		}
	}

	/* モデル指定あり */
	if ( scn == 1 && GCL_GetOption( 'm' ) != NULL ) model = GCL_GetNextInt() ;
	
	work->model = model ;
	work->map = where ;
	work->id = -1 ;

	GV_SetVec3( &work->shift, 0.0F, 0.0F, 0.0F ) ;
	GM_SetCurrentMap( where ) ;
	SetTarget( work, &work->world, scn ) ;
#if 0
	if ( work->flag & FLAG_SMELL ) {
		GM_InitRadarControl( &work->radar, &mov, 0, where ) ;
		GM_RadarSetSight( &work->radar, 0, 4096, 200.0F, RADAR_COLOR_YELOW ) ;
	}
#endif
	bodyflag = BODY_FLAG ;
	if ( scn == 1 ) {
		if ( GCL_GetOption( 's' ) != NULL ) {
			if ( (status = GCL_GetNextInt()) != 0 ) {
				work->flag |= FLAG_FREEZE ;
				GV_CallChildSignalFunc( work , FLUSH_COLD , 0 );
				bodyflag &= ~DG_FLAG_IRREACTION ;
				work->freeze_disp = 256 ;
			}
		}
		work->proc = GCL_GetOptionValue( 'O', 0 ) ;
	} else {
		work->proc = 0 ;
	}

	work->type = TYPE_NONE ;

	if ( scn == 0 || ( scn == 1 && GCL_GetOption( 'e' ) == NULL ) ) {	/* 壁床タイプ */
		FVECTOR		from = { 0.0F, 0.0F, 50.0F } ;
		FVECTOR		to =   { 0.0F, 0.0F, -150.0F } ;
		HZX_HZD		seg ;
		int			atr ;

		GM_InitObject( &( work->body ), model, bodyflag ) ;
		if ( work->body.objs == NULL ) return -1 ;
		work->effect_objs = work->body.objs ;
		effect_world = &work->body.objs->world ;
		DG_COPY_MAT( &work->body.objs->world, &work->world ) ;
		GM_ConfigObjectLight( &work->body, work->lights ) ;
		DG_GetLightMatrix( ( FVECTOR * )&work->body.objs->world.m[ 3 ], work->lights ) ;
		printf( "bul_c4 %f %f %f\n", work->body.objs->world.m[ 3 ][ 0 ],
			     work->body.objs->world.m[ 3 ][ 1 ], work->body.objs->world.m[ 3 ][ 2 ] ) ;
		DG_SetPos( &work->world ) ;
		DG_PutVector( &from, &from, 1 ) ;
		DG_PutVector( &to, &to, 1 ) ;		
		work->stick = NULL ;
		work->id = -1 ;
		if ( HZX_OnlineHazardCheck( HZX_AllMapID, &from, &to, HZX_CHK_ALL, 0, 0 ) ) {
			HZX_GetOnlineHazard( &seg, &atr ) ;
			if ( HZX_IsDynamic( &seg ) ) {
				work->stick = seg.ptr ;
				HZX_SetDynamicCallback( seg.ptr, &work->dhcb, DynamicHzdCallback, 
									   ( void * )work, NULL ) ;			
			}
			if ( seg.type == HZX_TYPE_SEGMENT ) work->type = TYPE_SEGMENT ;
			else								 work->type = TYPE_FLOOR ;
		} 
	} else {								/* 敵タイプ */
		HOMING_TRG			*hom ;
		CAPTURE_TARGET		*capture ;
		OBJECT				*body ;
		int					name, joint ;
		
		name = GCL_GetNextInt() ;
		work->enemy_joint = joint = GCL_GetNextInt() ;
		hom = GM_GetHoming() ;
		work->id = -1 ;

		if ( (effect_world = C4MAN_GetWorld( name )) != NULL ) {
			work->type = TYPE_CHARA2 ;
			work->effect_world = effect_world ;
			work->effect_objs = C4MAN_GetObjs( name ) ;
			work->effect_map = C4MAN_GetMap( name ) ;
//			C4MAN_GetParam( name, &work->shift, &work->shift_rot ) ;
			work->shift = shift_mov ;
			work->shift_rot = shift_rot ;
			GM_InitObject( &( work->body ), model, bodyflag ) ;
			if ( work->body.objs == NULL ) return -1 ;
			GM_ConfigObjectLight( &work->body, work->lights ) ;
			DG_GetLightMatrix( ( FVECTOR * )&work->body.objs->world.m[ 3 ], work->lights ) ;
			/* エフェクトワールドは座標変換後のワールド*/
			effect_world = &work->body.objs->world ;

printf( "---------------------- chara2 search success!!\n" ) ;

		} else {//test
effect_world = &work->body.objs->world ;
		while( hom != NULL ) {
//			if ( hom->status & HOMING_ENEMY ) {		/* 敵に限らない */
				if ( hom->ctrl->name == name && hom->trg != NULL ) {
					capture = hom->trg->capture ;
					if ( capture != NULL ) {
						capture->flag |= CAPTURE_C4EXIST ;
						work->stick = capture ;
						body = capture->body ;

						//rot = DG_ZeroSVector ;
						if ( hom->status & HOMING_SKIP ) {
							/* SKIPフラグが立ってるのは動かないものだと思われる */
							work->objinfo = GV_Malloc( sizeof( OBJINFO ) ) ;
							ASSERT( work->objinfo != NULL ) ;
							work->id = MakeAttachment3A_called( model, &rot, body,
															    joint, &mov, 0 ) ;
							work->flag |= FLAG_BELTOBJ ;
							DG_COPY_VEC( &work->objinfo->mov, &mov ) ;
							work->objinfo->rot = rot ;
							work->objinfo->model = model ;
							work->objinfo->joint = joint ;
						} else {
							work->id = MakeAttachment3A_called( model, &rot, body,
															    joint, &mov, 4 ) ;
						}

						effect_world = &(GetAttachmentDGobjs(work->id)->world) ;
						work->effect_objs = GetAttachmentDGobjs(work->id) ;

						if ( bodyflag & DG_FLAG_IRREACTION ) {
							TurnOnOff_IR_called( work->id, 1 ) ;
						}
						DG_COPY_MAT( &work->world, &body->objs->objs[ joint ].world ) ;
						_sceVu0AddVector( &work->shift, &work->shift, &mov ) ;
printf( "---------------------- enemy search success!!\n" ) ;
						break ;
					}
				}
//			}
			hom = hom->next ;
		}
}//test

		if ( work->id != -1 ) {
			work->type = TYPE_CHARA ;
		} else {
printf( "---------------------- enemy search failed\n" ) ;
		}
	}

	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		work->effect_name = GV_StrCode("c4_kaitai_a1_frost") ;
		if ( work->flag & FLAG_SMELL ) {
			/* においつき */
			if ( scn == 1 && GCL_GetOption( 'i' ) != NULL ) {
				work->effect_name = GCL_GetNextInt() ;
			}
		} else {
			/* においなし */
			if ( scn == 1 && GCL_GetOption( 'i' ) != NULL ) {
				work->effect_name = GCL_GetNextInt() ;
			} else {
				work->effect_name = GV_StrCode("c4_kaitai_b1_frost") ;
			}
		}
		GV_SetActorChild( work , (work->effect_ice = NewIceEffect( effect_world , work->lights, work->effect_name, work->flag, work->effect_objs) ) ) ;

		if ( status != 0 ) {
			GV_CallChildSignalFunc( work , FLUSH_COLD , 0 );
		}
	}

	work->blast_count = count ;
	PL_AddC4List( work ) ;

    return 0 ;
}

/* シナリオ起動 */
void	*NewSetKaitaiC4( int name, int where )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResourcesKaitai( work, name, where, NULL, -1, 1 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/* 解体Ｃ４プログラム起動（ファットマン） */
/* 起動成功 → ＩＤ（正の整数）が返る。
   起動失敗 → -1 が返る。*/
int				NewSetKaitaiC4Prog( FMATRIX *world, int where, int count )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResourcesKaitai( work, 0, where, world, count, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return -1 ;
		}
	} else return -1 ;
	return work->n_incremental ;	
}

/* プログラム起動したＣ４の状態を調べる */
/* 正整数 : 設置されていて未爆発。残りカウントが返る */
/* -1	  : 凍らされている */
/* -2	  : 既に存在しない */
int				GM_GetKaitaiC4Status( int id )
{
	Work		*work ;

	work = PL_FindC4( id ) ;
	if ( work == NULL ) return -2 ;
	if ( work->flag & FLAG_DESTROY ) return -2 ;
	if ( work->flag & FLAG_FREEZE ) return -1 ;
	return work->blast_count ;
}

/* プログラム起動したＣ４のカウントダウンを開始する */
void		 	GM_StartCountDownKaitaiC4( int id )
{
	Work		*work ;

	work = PL_FindC4( id ) ;
	if ( work == NULL ) return ;
	work->flag |= FLAG_COUNTDOWN ;
}

/* プログラム起動したＣ４を消去 */
void			GM_DestroyKaitaiC4( int id )
{
	Work		*work ;

	work = PL_FindC4( id ) ;
	if ( work == NULL ) return ;
	GV_DestroyOtherActor( work ) ;
}

/*------------------------------------------------------------*/

/* 一番近いＣ４への距離 */
float			GM_GetNearC4Length( mov, pos )
FVECTOR			*mov, *pos ;
{
    Work		*n ;
	float		len, minlen ;

	minlen = 1000000.0F ;
    n = C4MngWork->list.next ;
    while( n != NULL ) {
		if ( ( n->flag & FLAG_KAITAI ) &&
			!( n->flag & ( FLAG_SMELL | FLAG_FREEZE | FLAG_DESTROY ) ) ) {
			GV_MatToVec( &n->world, pos ) ;
			len = GV_VecLen3F2( mov, pos ) ;
			if ( minlen > len ) minlen = len ;
		}
		n = n->next ;
    }
	return minlen ;
}

/*------------------------------------------------------------*/

/* Ｃ４を付け替える（敵兵→死体 など） */
void		GM_ChangeC4Body( TARGET *old, TARGET *new )
{
	CAPTURE_TARGET	*oldc, *newc ;
	Work			*this ;
	int				found ;

	oldc = old->capture ;
	newc = new->capture ;

	if ( !( oldc->flag & CAPTURE_C4EXIST ) ) return ;
	
    this = C4MngWork->list.next ;
	found = 0 ;
    while( this != NULL ) {
		if ( this->type == TYPE_CHARA && 
			 ( CAPTURE_TARGET * )this->stick == oldc ) {
			found = 1 ; 
			break ;
		}
		this = this->next ;
    }
	if ( !found ) return ;

	/* 古いほうを消す */
	KillAttachment_called( this->id ) ;
	oldc->flag &= ~CAPTURE_C4EXIST ;

	/* 新しいほうに付ける */
	{
		FVECTOR		shift ;
		SVECTOR		rot ;

		GV_SetVec3( &shift, -150.0F, 50.0F, -75.0F ) ; 
		GV_SetVec3( &rot, -1024, 990, 0 ) ;
		this->id = MakeAttachment3A_called( BODY_NAME, &rot, newc->body,
										    HUMAN21_KOSHI, &shift, 4 ) ;		
		this->stick = ( void * )newc ;
		newc->flag |= CAPTURE_C4EXIST ;
	}
}

/*------------------------------------------------------------*/



