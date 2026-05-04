//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vrobj.c
	オブジェクト設置

	2002/07/01 Y.Korekado
	$Id: vrobj.c,v 1.1.1.3 2002/11/19 11:44:15 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewVrObject( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-r	向き
		-s  ステータス

----------------------------------------------------------------*/

#ifdef PSX2
#include <eekernel.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"
#include	"korekado/conv/korekado.x"
/*----------------------------------------------------------------*/
#ifdef DEBUG_MODE
//#define BRAKE_DEBUG	(1)
#endif

#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)
#define	BODY_FLAG_SHADOW	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE|DG_FLAG_SHADOWWRITE)

#define	PO_STATUS_SHADE			0x00000001
#define	PO_STATUS_SPOTLIGHT		0x00000002
#define	PO_STATUS_LOD			0x00000004
#define	PO_STATUS_D_HAZARD		0x00000008	/* モデルのバウンディングに動的壁セット */
#define	PO_STATUS_TRANS_SCALE	0x00000010	/* トランスにスケールをかける */
#define	PO_STATUS_NO_PRESHADE	0x00000020	/* プレイシェードしない */
#define	PO_STATUS_AUTOREPAINT	0x00000040	/* オートリペイント */
#define	PO_STATUS_NOFOG			0x00000080	/* フォグ効かない */
#define	PO_STATUS_IR			0x00000100	/* 赤外線ゴーグル反応 */
#define	PO_STATUS_STAGE_ROTATE	0x00000200	/* ステージモデル（トランス付き）を回転 */
#define	PO_STATUS_NV			0x00000400	/* 暗視ゴーグルに反応 */

#define	PO_STATUS_TARGET		0x00001000	/* ボム系で消える */
#define	PO_STATUS_OBJ_OFF		0x00002000
#define	PO_STATUS_SPOTDRAW		0x00004000	/* スポットライトがうつる */

#define SPOTMODELNAME	(11668919)	/*投影モデル箱*/
/*----------------------------------------------------------------*/
typedef	struct	{
    GV_ACT_EX	actor;
	OBJECT		body ;

	FMATRIX		lights[2] ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	FVECTOR		scale ;
	int			status ;
	int			name ;
	int			model ;
	int			lit ;
    TARGET		deftrg ;	/* 防御ターゲット */
	int			flag  ;
	void		*wall ;
	void		*spotlight ;
	int			break_proc  ;
	FMATRIX		spotworld ;
} Work ;

enum {
	OBJ_OFF=0,
	OBJ_ON,
	OBJ_TURN,
	OBJ_MOVE,
	OBJ_PRESHADE,
	OBJ_KILL,
	OBJ_CRUMBLE
};

/*----------------------------------------------------------------*/
#define FLAG_DAMAGED	0x00000001
#define FLAG_BRAKED		0x00000002
#define FLAG_CRUMBLE	0x00000004
#define FLAG_DISAPPEAR	0x00000008
/*----------------------------------------------------------------*/
extern void *NewMakeObjectBoundHazard( HZX_GROUP_ID	hzx_id, DG_OBJS *objs, u_int seg_atr, u_int flr_atr ) ;
void		*NewPutObject( int name, int where ) ;
/*----------------------------------------------------------------*/

static void SetObject( Work *work )
{
	FMATRIX	mat ;

	_RotMatrixZYX( &mat, &work->rot ) ;
	_ScaleMatrix( &mat, &work->scale ) ;
	KR_FvecToMat( &work->pos, &mat ) ;
	DG_SetPos( &mat ) ;

	DG_PutObjs( work->body.objs ) ;
}

static void CheckMessage( Work *work )
{
	GV_MSG *msg ;
	int mes_num ;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		switch( msg->message[0] ){
		  case OBJ_OFF:
			work->status |= PO_STATUS_OBJ_OFF ;
			work->body.objs->flag |= DG_FLAG_INVISIBLE ;
//printf("putobj: model[%d] OFF \n",work->name ) ;
			break;
		  case OBJ_ON:
			work->status &= ~PO_STATUS_OBJ_OFF ;
			work->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
			break;
		  case OBJ_TURN:
			if ( msg->message_len >= 4 ) {
				work->rot.vx = (short)msg->message[ 1 ] ;
				work->rot.vy = (short)msg->message[ 2 ] ;
				work->rot.vz = (short)msg->message[ 3 ] ;
			}
			SetObject( work ) ;
			break;
		  case OBJ_PRESHADE:
			ENE_PreShade( work->body.objs, work->lit ) ;
			break;
		  case OBJ_MOVE:	/* 同マップ内限定 */
			if ( msg->message_len >= 4 ) {
				work->pos.vx = (float)msg->message[ 1 ] ;
				work->pos.vy = (float)msg->message[ 2 ] ;
				work->pos.vz = (float)msg->message[ 3 ] ;
			}
			SetObject( work ) ;
			break;
		  case OBJ_KILL:
			if ( work->spotlight != NULL ) {
				GV_DestroyActor( work->spotlight ) ;
			}
			if ( work->wall != NULL ) {
				GV_DestroyActor( work->wall ) ;
			}
			GV_DestroyActor( work ) ;
			break;
		  case OBJ_CRUMBLE:
			work->flag |= FLAG_CRUMBLE ;
			{
				extern void *NewSigBreakObj7(DG_OBJS *,float ,int ,int ) ;
				int	time_a = 60 ;	/*ひとつのパーツが消えるまでの時間*/
				int	time_b = 30 ;	/*上面が変形してから底面が変形するまでの遅れ時間*/
				/*全体の完全消滅にかかる時間はtime_a+time_bとなります*/
				GV_SetActorChild( work, NewSigBreakObj7((work->body.objs),5000.0f ,time_a ,time_b) ) ; 

printf("vrobjjjjjjjjjjjjj model[%d]\n",work->model ) ;

				{	/* エフェクト消去 */
					void UTL_EFT_ExecCallback(void) ;
					int UTL_EFT_AddBound( FVECTOR *p0, FVECTOR *p1, int flag ) ;
					DG_DEF			*def ;
					FVECTOR		b1, b2 ;

					def = work->body.objs->def ;
					b1.vx = def->lx - 100.0 + def->tx ;
					b1.vy = def->ly - 100.0 + def->ty ;
					b1.vz = def->lz - 100.0 + def->tz ;
					b2.vx = def->ux + 100.0 + def->tx ;
					b2.vy = def->uy + 100.0 + def->ty ;
					b2.vz = def->uz + 100.0 + def->tz ;
printf("vrobj.c: b1 [%f][%f][%f] b2[%f][%f][%f] \n",b1.vx,b1.vy,b1.vz,b2.vx,b2.vy,b2.vz);

					UTL_EFT_AddBound( &b1, &b2, 1 ) ;
					UTL_EFT_ExecCallback() ;
				}

			}
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	CheckMessage( work ) ;

//AN_Test_Eye2( &work->pos, 1 );

	if ( work->status & PO_STATUS_SHADE ) {
		DG_GetLightMatrix( &work->pos, (FMATRIX *)work->lights );
	}

	if ( work->status & PO_STATUS_NV ) {
		if ( Ply_GetPlayerItem() == IT_NightVision ) {
			if ( !(work->status & PO_STATUS_SHADE) ) {
				DG_GetLightMatrix( &work->pos, (FMATRIX *)work->lights );
			}

			work->lights[1].m[3][0] += 42 ;
			work->lights[1].m[3][1] += 42 ;
			work->lights[1].m[3][2] += 42 ;
		}
	}

	if ( work->flag & FLAG_DAMAGED ) {
		if ( !(work->flag & FLAG_BRAKED) ) {
			extern void *NewSigBreakObj7(DG_OBJS *,float ,int ,int ) ;
			int	time_a = 60 ;	/*ひとつのパーツが消えるまでの時間*/
			int	time_b = 30 ;	/*上面が変形してから底面が変形するまでの遅れ時間*/
			/*全体の完全消滅にかかる時間はtime_a+time_bとなります*/
			GV_SetActorChild( work, NewSigBreakObj7((work->body.objs),5000.0f ,time_a ,time_b) ) ; 
			if( work->wall != NULL ) {
				GV_DestroyActor( work->wall ) ;
			}
			if ( work->spotlight != NULL ) {
				GV_DestroyActor( work->spotlight ) ;
			}
			work->deftrg.class |= TARGET_SKIP ;
			work->flag |= FLAG_BRAKED ;
			GM_SeSetMode( SD_A_BRVRBOX1, &work->deftrg.center, GM_SEMODE_BOMB ) ;
			if ( work->break_proc != 0 ) {
				GCL_ARGS arg ;
				int	argv[ 4 ] ;

				argv[0] = work->name ;
				arg.argc = 1 ;
				arg.argv = &argv[0] ;
				GCL_ExecProc( work->break_proc, &arg ) ;
			}
			{	/* エフェクト消去 */
				void UTL_EFT_ExecCallback(void) ;
				int UTL_EFT_AddBound( FVECTOR *p0, FVECTOR *p1, int flag ) ;
				FVECTOR		b1, b2 ;
				TARGET		*trg  ;

				trg = &work->deftrg ;
				b1.vx = trg->center.vx - trg->size.vx - 100.0 ;
				b1.vy = trg->center.vy - trg->size.vy - 100.0 ;
				b1.vz = trg->center.vz - trg->size.vz - 100.0 ;
				b2.vx = trg->center.vx + trg->size.vx + 100.0 ;
				b2.vy = trg->center.vy + trg->size.vy + 100.0 ;
				b2.vz = trg->center.vz + trg->size.vz + 100.0 ;

				UTL_EFT_AddBound( &b1, &b2, 1 ) ;
				UTL_EFT_ExecCallback() ;
			}
		}
	}
	if ( work->flag & FLAG_CRUMBLE ) {
	}
}

static	void	Die( Work *work )
{
	extern	void	DG_FreePreshade( DG_OBJS * ) ;

	if ( work->status & PO_STATUS_TARGET ) {
	    GM_FreeTarget( &work->deftrg ) ;
	}
	if ( work->spotlight != NULL ) {
//		GV_DestroyActor( work->spotworld ) ;
	}
    DG_FreePreshade( work->body.objs ) ;
    GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/
static	void	TargetCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	
	work = (Work *)ptr ;

//printf("1koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee [%lx] \n",off->weapon_type ) ;
	if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
//printf("2koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee [%lx] \n",off->weapon_type ) ;
			if ( off->weapon_type  & (WP_C4BOMB) ) {
				if ( off->weapon_type  & (WP_NOPLAYER) ) {
					work->flag |= FLAG_DAMAGED ;
				}
			}
#ifdef BRAKE_DEBUG
			if ( off->weapon_type  & (WP_GRENADE) ) {
				if ( !(off->weapon_type  & (WP_NOPLAYER)) ) {
					work->flag |= FLAG_DAMAGED ;
				}
			}
#endif
		}
	}
}

static void	InitTarget( Work *work )
{
	TARGET		*trg  ;
	FVECTOR 	size, shift ;
	DG_DEF		*def ;
	IVECTOR		v[ 4 ] ;

	trg = &work->deftrg ;

	def = work->body.objs->def ;

	GV_InitVec3( &v[ 0 ], def->lx, def->ly, def->lz ) ;
	GV_InitVec3( &v[ 1 ], def->ux, def->ly, def->lz ) ;	
	GV_InitVec3( &v[ 2 ], def->lx, def->ly, def->uz ) ;	
	GV_InitVec3( &v[ 3 ], def->ux, def->uy, def->uz ) ;

	shift.vx = (float)(v[1].vx - v[0].vx) / 2 ;
	shift.vy = ((float)(v[3].vy - v[0].vy) / 2) + 10.0 ;
	shift.vz = (float)(v[2].vz - v[0].vz) / 2 ;

	size.vx = DG_FABS(shift.vx) + 50.0 ;
	size.vy = DG_FABS(shift.vy) ;
	size.vz = DG_FABS(shift.vz) + 50.0  ;

#if 1
	shift.vx = 0.0 ;
	shift.vy = 10.0 ;
	shift.vz = 0.0 ;
#endif

#ifdef BRAKE_DEBUG
	GM_SetTarget( trg, (TARGET_POWER|TARGET_DEFENSE), 1, BOTH_SIDE,  &size, &shift ) ;
#else
	GM_SetTarget( trg, (TARGET_POWER|TARGET_DEFENSE), 1, ENEMY_SIDE,  &size, &shift ) ;
#endif
	GM_SetTargetWeaponType( trg, 0 ) ;
	GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, TargetCallBack, work ) ;

	{
		FVECTOR	trans_pos, *trans ;

		DG_SetPos2( &work->pos, &work->rot ) ;
		trans = &(work->body.objs->trans) ;	/* ０のモデルから計算 */
		DG_PutVector( trans, &trans_pos, 1 ) ;

	    GM_MoveTarget( trg, &trans_pos ) ;
	}

//NewTargetView( trg, 200, 34, 184 ) ;
}

static	int	GetResources( Work *work, int name, int where )
{
	char		*opt ;
	int			model ;

	work->name = name ;
	work->flag = 0 ;

	work->status = GCL_GetOptionValue( 'f', 0 ) ;
	GM_CurrentMap = where ;	

	work->model = model = GCL_GetOptionValue( 'm', 0 ) ;
	if ( model == 0 ) return -1 ;
	GM_InitObject( &work->body, model, BODY_FLAG );

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &work->pos ) ;
	} else {
		work->pos = DG_ZeroVector ;
	}

	if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){
		GCL_GetSV( opt, (short *)&work->rot ) ;
	} else {
		work->rot = DG_ZeroSVector ;
	}

	if ( ( opt = GCL_GetOption( 's' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &work->scale ) ;
		work->scale.vx /= 100.0F ;
		work->scale.vy /= 100.0F ;
		work->scale.vz /= 100.0F ;
	} else {
		work->scale.vx = work->scale.vy = work->scale.vz = 1.0F ;
	}

	work->body.objs->flag |= DG_FLAG_SHADOWMAKE ;

	if ( work->status & (PO_STATUS_SHADE|PO_STATUS_SPOTLIGHT) ) {
		work->body.objs->flag |= DG_FLAG_SHADOWMAKE ;
		if ( work->status & PO_STATUS_NOFOG ) {
			work->body.objs->flag |= DG_FLAG_NOFOG ;
		}
		if ( work->status & PO_STATUS_IR ) {
			work->body.objs->flag |= DG_FLAG_IRREACTION ;
		}
	}

	/* スケーリング前に座標移動 */
	if( work->status & PO_STATUS_STAGE_ROTATE ) {
		/* ステージ変換されたモデルはトランス値を回転させる */
		FVECTOR	*trans ;

		DG_SetPos2( &DG_ZeroVector, &work->rot ) ;
		trans = &(work->body.objs->trans) ;	/* ０のモデルから計算 */
		DG_PutVector( trans, trans, 1 ) ;
	}

	DG_SetPos2( &work->pos, &work->rot ) ;
	DG_PutObjs( work->body.objs ) ;

	/* プリシェードはスケーリング前に */
	if( !(work->status & (PO_STATUS_SHADE|PO_STATUS_NO_PRESHADE)) ) {
		if ( (work->lit = GCL_GetOptionValue( 'l', 0 )) != 0 ) {
			ENE_PreShade( work->body.objs, work->lit ) ;
			if ( work->status & PO_STATUS_AUTOREPAINT ) {
				work->body.objs->flag |= DG_FLAG_AUTOREPAINT ;
			}
		}
	}

	if( work->status & PO_STATUS_NO_PRESHADE ) {
		work->lights[1].m[3][0] = 128 ;
		work->lights[1].m[3][1] = 128 ;
		work->lights[1].m[3][2] = 128 ;
	}

	if( work->status & PO_STATUS_TRANS_SCALE ) {
		FVECTOR	*trans ;

		trans = &(work->body.objs->trans) ;	/* ０のモデルから計算 */
		trans->vx *= (work->scale.vx) ;
		trans->vy *= (work->scale.vy) ;
		trans->vz *= (work->scale.vz) ;
	}

	if ( work->status & PO_STATUS_TARGET ) {
		InitTarget( work ) ;
		work->break_proc = GCL_GetOptionValue( 'e', 0 ) ;
	}

	SetObject( work ) ;

	work->wall = NULL ;
	if( work->status & PO_STATUS_D_HAZARD ) {
		GV_SetActorChild( work,(work->wall = NewMakeObjectBoundHazard( GM_GetHzxGroupID( where ), 
								work->body.objs, HZX_SEG_NO_BULLETHOLE|HZX_SEG_NO_C4, HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_NO_C4 )) ) ;
	}

	work->spotlight = NULL ;
	if( work->status & PO_STATUS_SPOTDRAW ) {
		extern void *NewSpotDrawObject( int name, FMATRIX *world, int model ) ;
		
		work->spotworld = work->body.objs->world ;
		work->spotworld.m[3][0] += 10.0 ;
		work->spotworld.m[3][1] += 10.0 ;
		work->spotworld.m[3][2] -= 10.0 ;
		work->spotlight = NewSpotDrawObject( SPOTMODELNAME,&work->spotworld, model ) ;
	}

//printf( " status[%x] work->body[i].objs->flag=[%x] \n",work->status, work->body[0].objs->flag ) ;
	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewVrObject( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX(&work->actor) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
