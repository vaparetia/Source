//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	npc.c
	NPCメイン

	2001/02/08 Y.Korekado
	$Id: npc.c,v 1.1.1.3 2002/11/19 11:44:22 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------
----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include <libgcl.h>

#include	"gameheader.h"
#include	"npc.h"

#include "BP_Misc.h"

/*----------------------------------------------------------------*/
#define HEAD_MARK_3	(1)


#ifdef DEBUG_MODE
//#define NPC_TARGET_VIEW	(1)
#endif

/*----------------------------------------------------------------*/
#define OBJECT_FLAG (DG_FLAG_IRREACTION|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

/*----------------------------------------------------------------*/
extern void GM_MouthAnimation( int talk_name, DG_EVMOBJ *evmobj  ) ;
/*----------------------------------------------------------------*/
void	NPC_DamageFlagClear( NPCWORK	*npc )
{
	NPCTARGET	*npctrg ;
	TARGET		*def_child ;
	int			i ;
	
	npc->target.deftrg->weapon_type = 0 ;
	npc->target.deftrg->damaged = 0 ;
	npctrg = &npc->target ;
	def_child = npctrg->def_child ;
	for( i=0; i<npctrg->child_trg_num ; i++ ) {
		def_child->weapon_type = 0 ;
		def_child->damaged = 0 ;
		def_child ++ ;
	}
}

void	NPC_CaptureFlagClear( NPCWORK	*npc )
{
	CAPTURE_TARGET	*cap ;
	
	cap = npc->target.capture ;
	if ( cap->capture != NULL ) {
		cap->capture->flag |= CAPTURE_FREE ;
		cap->capture = NULL ;	
	}
	/* 仰向けかうつぶせかは残す */
	cap->flag &= (CAPTURE_BACK|CAPTURE_FRONT) ;
}

void	NPC_DamageCaptureFlagClear( NPCWORK	*npc )
{
	NPC_DamageFlagClear( npc ) ;
	NPC_CaptureFlagClear( npc ) ;
}

void	NPC_SetFaintCount( NPCWORK *npc, int count, int mode  )
{
	npc->action.faint_count = count ;
	npc->action.faint_mode = mode ;
}

/* ダメージを受けたチャイルドターゲット番号を返す */
int	NPC_ChildTargetCheck( NPCWORK	*npc )
{
	NPCTARGET	*npctrg ;
	TARGET		*def_child ;
	int			i ;
	
	npctrg = &npc->target ;
	def_child = npctrg->def_child ;
	for( i=0; i<npctrg->child_trg_num ; i++ ) {
		if ( TARGET_POWER & def_child->damaged ) {
			return i ;
		}
		def_child ++ ;
	}

	return -1 ;
}

/* ヘッドマーク */
/*
form user/shibata/t_headmark/headmark.h
HMK2_TYPE_RED_AT	//赤！
HMK2_TYPE_RED_QE	//赤？
HMK2_TYPE_YLW_AT	//黄！
HMK2_TYPE_YLW_QE	//黄？
HMK2_TYPE_WHT_AT	//白！
HMK2_TYPE_WHT_QE	//白？
HMK2_TYPE_PNK_AT	//ピンク！
HMK2_TYPE_ZZZ		//立ち居眠り
HMK2_TYPE_PIYO		//気絶
HMK2_TYPE_PIYO_A	//麻酔眠り
HMK2_TYPE_POWA		//放心 

HMK2_TYPE_KILL	//クリア
HMK2_TYPE_TRGT	//壊せる
*/
void	NPC_CallHeadMark( NPCWORK *npc, int mark  )
{
#ifdef HEAD_MARK_3
	if ( npc->action.headmark == NULL ) return ;

	*npc->action.headmark = mark ;
	if ( mark == HMK2_TYPE_PIYO )	npc->action.headmark_num = 3+1 ;	/* 初期個数は３個 */
	if ( mark == HMK2_TYPE_PIYO_A )	npc->action.headmark_num = 4+1 ;	/* 初期個数は４個 */

	CallActHeadMarks( npc->headmark, mark );
#else
	if ( npc->action.headmark == NULL ) return ;

	*npc->action.headmark = mark ;
	if ( mark == HMK2_TYPE_PIYO )	npc->action.headmark_num = 3+1 ;	/* 初期個数は３個 */
	if ( mark == HMK2_TYPE_PIYO_A )	npc->action.headmark_num = 4+1 ;	/* 初期個数は４個 */
	/* ヘッドマークコントロールが起動するまで1フレームかかる
		それまでに個数の変化メッセージを送るとバグるので
		変則処理をする。後で柴田君が直すかも*/
#endif
}

/* 麻酔が刺さる 注！モデルのcv2は必要なし */
void NPC_SetNeedl( OBJECT *body, int n_obj, FVECTOR *pos, int model )
{
	extern int MakeAttachment4_called(int ,FVECTOR *,SVECTOR *,OBJECT *,int,FVECTOR *,int,int );
	extern void VertexSearch(FVECTOR *vans,FVECTOR *nans,DG_OBJS *objs,int objnum,FVECTOR *target) ;
	SVECTOR	rot ;
	FVECTOR	vans, nans ;

	VertexSearch( &vans, &nans, body->objs, n_obj, pos );
	_FVecToRotXY( &nans, &rot ) ;
	rot.vx -= 1024 ; /* "m92_bul2"モデル固有補正 */

	MakeAttachment4_called( model, NULL, &rot, body, n_obj, &vans, 512, 4 );
}

/* 麻酔が刺さる LOD用にモデルが消えても表示する */
void NPC_SetNeedlV( OBJECT *body, int n_obj, FVECTOR *pos, int model )
{
	extern int MakeAttachment4V_called(int ,FVECTOR *,SVECTOR *,OBJECT *,int,FVECTOR *,int,int );
	extern void VertexSearch(FVECTOR *vans,FVECTOR *nans,DG_OBJS *objs,int objnum,FVECTOR *target) ;
	SVECTOR	rot ;
	FVECTOR	vans, nans ;

	VertexSearch( &vans, &nans, body->objs, n_obj, pos );
	_FVecToRotXY( &nans, &rot ) ;
	rot.vx -= 1024 ; /* "m92_bul2"モデル固有補正 */

	MakeAttachment4V_called( model, NULL, &rot, body, n_obj, &vans, 512, 4 );
}

void NPC_ClearNeedl( OBJECT *body )
{
	SearchAndFallAttachment_called( body, 0, COUNT_VMODE(180), 16 ) ;
}

void NPC_IK_Control( NPCWORK *npc )
{
	NPCACT	*act ;
	int status ;

	act = &npc->action ;
	status = npc->action.status ;

	if ( status & NPC_ACT_STATUS_IK_DOWN ) {
		TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->ikwork, 0 );
	} else if ( status & NPC_ACT_STATUS_IK_FOOT ) {
		TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->ikwork, 1 );
	} else if ( status & NPC_ACT_STATUS_IK_HAND ) {
		TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->ikwork, 2 );
	} else {
		TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->ikwork, -1 );
	}

	if ( status & NPC_ACT_STATUS_IK_PIKU ) {
		switch ( npc->target.dam_obj ) {
			case HUMAN21_MUNE :
			case HUMAN21_ATAMA :
			case HUMAN21_KOSHI :
			case HUMAN21_ONAKA :
				TAKABE_UtilPuppetIK_Piku( act->ikwork, -1 );
				break ;
			default :
				TAKABE_UtilPuppetIK_Piku( act->ikwork, npc->target.dam_obj );
				break ;
		}
	}

	TAKABE_ActPuppetIK( act->ikwork );
}

/*----------------------------------------------------------------*/
void	NPC_PreProcess( NPCWORK *npc ) 
{
	NPCACT 		*act ;

	act = &npc->action ;
	act->dir = -1 ;
	act->pad = 0 ;
	
	if ( npc->nadj != NULL ) {
		npc->nadj->adj_status = 0 ;
	}
}

void	NPC_AfterProcess( NPCWORK *npc ) 
{
	NPCTARGET	*npctrg ;
	NPCACT		*act ;
	TARGET		*def_child ;
	int i ;

	act = &npc->action ;
	if ( npc->target.deftrg != NULL ) {
		npctrg = &npc->target ;

		GM_MoveTargetMap( npctrg->deftrg, &(npc->ctrl->mov), npc->ctrl->map ) ;
//	    GM_MoveTarget( npctrg->deftrg, &(npc->ctrl->mov) ) ;

	    def_child = npctrg->def_child ;
		for( i=0; i<npctrg->child_trg_num ; i++ ) {
//		    GM_MoveTarget2( def_child, &(BODYWORLD( npc->body, npctrg->connect_obj[i] )) ) ;
		    GM_MoveTarget2Map( def_child, &(BODYWORLD( npc->body, npctrg->connect_obj[i])),
		    		npc->ctrl->map ) ;
		    def_child++ ;
		}

	}

	/* レーダー */
	if ( npc->rctrl != NULL ) {
		int	range ;
		float sight ;

//		GM_RadarSetFlag( npc->rctrl, RADAR_VISIBLE ) ;
		
		range = act->eye_range ;
		sight = (float)act->eye_sight ;
		if ( act->status & NPC_ACT_STATUS_EYE_CLOSE ) {
			range = 0 ;
			sight = 0.0f ;
		}
		GM_RadarSetSight( npc->rctrl, act->face_dir, range, sight, act->radar_color );
	}

	/* マルチウェイトモデル切替え */
	if ( npc->body->evmobj != NULL ) {
		DG_EVMOBJ *evm ;
		int dis, lod ;

		evm = npc->body->evmobj ;
		dis = KR_CameraDis( &npc->ctrl->mov ) ;

		lod = ( GM_CheckPlayerStatus(PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) 
			? npc->lod * 2 : npc->lod ;

		if ( dis > lod ) {
			UNSET_FLAG( npc->body->objs->flag, DG_FLAG_INVISIBLE ) ;
			SET_FLAG( evm->flag, DG_EVMOBJ_INVISIBLE ) ;
		} else {
			SET_FLAG( npc->body->objs->flag, DG_FLAG_INVISIBLE ) ;
			UNSET_FLAG( evm->flag, DG_EVMOBJ_INVISIBLE ) ;
		}
		if ( npc->inf_name ) {
			GM_MouthAnimation( npc->inf_name, evm ) ;
		}
	}
}

void	NPC_ActControl( NPCWORK *npc )
{
	OBJECT		*body ;
	CONTROL		*ctrl ;

	body = npc->body ;
	ctrl = npc->ctrl ;
	
	npc->old_body_height = body->height ;
    GM_ActMotion( body ) ;
	ctrl->height = body->height ;
	GM_ActControl( ctrl ) ;
	GM_ActObject2( body );
	NPC_IK_Control( npc ) ;

	DG_GetLightMatrix( &ctrl->mov, npc->lights );

    /* ＳＥ変換用 */
	if ( npc->se_id >= 0 ) {
	    MT_SetMotionSeTable( body->m_ctrl, GM_CurrentMap, npc->se_id, 
				 ( npc->ctrl->flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 ) ;
	}

}

void	NPC_ActControlExe( NPCWORK *npc, int scale )
{
	OBJECT		*body ;
	CONTROL		*ctrl ;

	body = npc->body ;
	ctrl = npc->ctrl ;

	npc->old_body_height = body->height*scale ;
    GM_ActMotion( body ) ;
	ctrl->height = body->height*scale ;
	if ( !(body->flag & OBJECT_MOTIONSTEP_THROUGH) ) {
		ctrl->step.vx *= (float)scale ;
		ctrl->step.vz *= (float)scale ;
//printf(" step vx[%f] vz[%f]\n",ctrl->step.vx, ctrl->step.vz ) ;
	}

	GM_ActControl( ctrl ) ;
	GM_ActObject2( body );
	NPC_IK_Control( npc ) ;
	DG_GetLightMatrix( &ctrl->mov, npc->lights );

    /* ＳＥ変換用 */
	if ( npc->se_id >= 0 ) {
	    MT_SetMotionSeTable( body->m_ctrl, GM_CurrentMap, npc->se_id, 
				 ( npc->ctrl->flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 ) ;
	}

}

void	NPC_Gravitation( NPCWORK *npc )
{
	CONTROL		*ctrl ;

	ctrl = npc->ctrl ;

	if ( !(ctrl->skip_flag & CTRL_SKIP_FLR_CHECK) ) {
		if ( ctrl->grounded & 1 ) ctrl->step.vy = 0.0F ;

      if ( BP_IsPAL()==TRUE )
		   ctrl->step.vy -= (16.0F * 1.2f) ;
      else
		   ctrl->step.vy -= 16.0F ;

   } else {
		/* Y移動量はモーションに任せる */
		ctrl->step.vy = npc->body->height - npc->old_body_height ;
	}
}

void	NPC_SetCheckPad( NPCWORK *npc, NPCCHECK checkpad )
{
	npc->CheckPad = checkpad ;
}
void	NPC_SetCheckDamage( NPCWORK *npc, NPCCHECK checkdamage )
{
	npc->CheckDamage = checkdamage ;
}
void	NPC_ChangeTargetSize( NPCWORK *npc, int n )
{
	GM_SetTargetSize( npc->target.deftrg, &npc->target.target_size[ n ] ) ;
}

/*----------------------------------------------------------------*/
void	NPC_InitNPC( NPCWORK *npc, void *work, OBJECT *body, CONTROL *ctrl, FMATRIX *lights )
{
	npc->body = body ;
	npc->ctrl = ctrl ;
	npc->lights = lights ;
	npc->character = work ;
	npc->rctrl = NULL ;
	npc->fcanim = NULL ;
	npc->inf_name = 0 ;
	npc->nadj = NULL ;
	npc->hom = NULL ;
	npc->se_id = -1 ;
	npc->rnavi = NULL ;
	npc->status = 0 ;

	npc->target.deftrg = NULL ;

	NPC_InitAction( npc, &npc->action ) ;
}

void	NPC_InitControl( NPCWORK *npc, int name )
{
	extern void GM_InitRadarControl( RADAR_CTRL *, FVECTOR *, int , int  );
	extern void GM_RadarSetVRange( RADAR_CTRL *, float , float );
	CONTROL	*ctrl ;

	ctrl = npc->ctrl ;

	GM_InitControl( ctrl, name, 0 ) ;
	ctrl->hzx_height = 750 ;
	ctrl->height = 1049.0F ;
	GM_ConfigControlHazard( ctrl, 1200, 450, 500 ) ;
	GM_ConfigControlTrapCheck( ctrl ) ;
	GM_ConfigControlMessageCheck( ctrl ) ;

	GM_ConfigControlAddressCheck( ctrl ) ;
	GM_ConfigControlMapCheck( ctrl ) ;

    ctrl->seg_flag |= HZX_TYPE_ENEMY ;
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
	ctrl->attribute |= CTRL_ATR_NPC ;

#if 0
	GM_SetZoneIntrpt( &entk->z_intrpt, ctrl, 0, 0, ZONE_INTRPT_ENEMY ) ;
	GM_PutZoneIntrpt( &entk->z_intrpt ) ;
#endif
}

void	NPC_InitObject( NPCWORK *npc, int modelname )
{
	OBJECT		*body ;
	CONTROL		*ctrl ;

	body = npc->body ;
	ctrl = npc->ctrl ;

	/* 関節型モデルの初期化 */
	GM_InitObject( body, modelname, OBJECT_FLAG  );
	GM_ConfigObjectLight( body, npc->lights );

	/* object control の関連付け */
	GM_ConfigControlObject( ctrl, body ) ;
}

void	NPC_InitMWObject( NPCWORK *npc, int modelname, int lod )
{
	npc->lod = lod ;
	GM_ConfigObjectEvm( npc->body, modelname, DG_EVMOBJ_IRREACTION ) ;

	/* EVMへライトを設定 */
	GM_ConfigObjectLight( npc->body, npc->lights );
}

void	NPC_InitMotion( NPCWORK *npc, int base, int damage, int capture, int drag )
{
	OBJECT		*body ;
	NPCACT *act ;

	act = &npc->action ;
	body = npc->body ;

	npc->base_mar = base ;
	npc->damage_mar = damage ;
	npc->capture_mar = capture ;
	npc->drag_mar = drag ;

	/* モーション初期化 オーバーライドは３つまで */
	act->current_mar = npc->base_mar ;
	GM_ConfigObjectMotion( body, 3, act->current_mar, MT_FLAG_HUMAN2 ) ;
	GM_ConfigObjectStep( body, &npc->ctrl->step ) ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;
	GM_ConfigObjectAction( body, 1, 0, 0, 0x0, 0 ) ;
	GM_ConfigObjectAction( body, 2, 0, 0, 0x0, 0 ) ;

	act->ikwork = TAKABE_MakePuppetIK( npc->ctrl, body );
}

void	NPC_InitNavi( NPCWORK *npc, CONTROL *ctrl, NAVIGATE *navi, NAVITARGET *nvtrg )
{
	npc->navi = navi ;
	npc->nvtrg = nvtrg ;

	GM_SetNavi( navi, ctrl ) ;
}

static FVECTOR 	Shift = { 34.0F, -465.0F, 263.0F } ;
void	NPC_InitDefenceTarget( NPCWORK *npc, TARGET *deftrg,
			int side, FVECTOR *size, FVECTOR *drag_shift, FVECTOR *shift )
{
	npc->target.deftrg = deftrg ;
	npc->target.child_trg_num = 0 ;
	npc->target.target_size = size ;
	npc->target.drag_shift = drag_shift ;
	npc->target.hang_shift = &Shift ;

	GM_SetTarget( deftrg, TARGET_DEFENSE|TARGET_SEEK, 1, side,  size, shift ) ;
	GM_SetTargetWeaponType( deftrg, 0 ) ;
	GM_PutTarget( deftrg ) ;
    deftrg->class = DEF_TARGET_CLASS ;

#ifdef NPC_TARGET_VIEW
NewTargetView( deftrg, 34, 184, 200 ) ;
#endif
}

void	NPC_SetHangShift( NPCWORK *npc, FVECTOR *shift )
{
	npc->target.hang_shift = shift ;
}

#define CHILD_TRG_FLAG	(TARGET_ROTATE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)
void	NPC_InitDefenceChildTarget( NPCWORK *npc, TARGET *child_trg,
			int side, FVECTOR *size, FVECTOR *shift, int *level_num, int *connect )
{
	NPCTARGET *npctrg ;
	TARGET	*deftrg, *child, *level_child ;
	int i,j, sum ;

	npctrg = &npc->target ;
	deftrg = npctrg->deftrg ;
	npctrg->def_child = child = child_trg ;

	sum = 0 ;
	for( j=0; j<4; j++ ) {
		level_child = child ;
		for( i=0; i<*level_num; i++ ) {
			GM_SetTarget( child, CHILD_TRG_FLAG, 1, side, size, shift ) ;
			GM_SetTargetWeaponType( child, 0 ) ;
		    child->class |= TARGET_POWER ;
			npctrg->connect_obj[sum++] = *connect ;
#ifdef NPC_TARGET_VIEW
NewTargetView( child, 200, 34, 184 ) ;
#endif
			child ++ ;
			size ++ ;
			shift ++ ;
			connect ++ ;
		}
		if ( i!=0 ) GM_SetTargetParts( deftrg, level_child, i, j ) ;
		npctrg->trg_level_num[j] = *level_num ;
		level_num ++ ;
	}
	ASSERT( sum <= NPC_MAX_CHILD_TARGET );
	npctrg->child_trg_num = sum ;
	npc->target.capture = NULL ;
}

static void TargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	NPCWORK 	*npc ;
	NPCTARGET	*npctrg ;

	npc = ( NPCWORK * )ptr ;

	npctrg = &npc->target ;

    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			npctrg->force = DG_ZeroVector ;
			if ( off->power != NULL ) {
				npctrg->force = off->power->force ;
			}
//			entk->act->bodyp.off_center = off->center ;
		}
	}
}

void	NPC_SetTargetCallBack( NPCWORK *npc )
{
	NPCTARGET	*npctrg ;
	TARGET		*def_child ;
	int			i ;
	
	npctrg = &npc->target ;
    GM_SetTargetCallBack( npctrg->deftrg, TargCallBack, npc ) ;

	def_child = npctrg->def_child ;
	for( i=0; i<npctrg->child_trg_num ; i++ ) {
	    GM_SetTargetCallBack( def_child, TargCallBack, npc ) ;
		def_child ++ ;
	}
}

void	NPC_InitDefenceCapture( NPCWORK *npc,CAPTURE_TARGET *capture, CONTROL *ctrl, OBJECT *body )
{
    GM_SetCaptureTarget( npc->target.deftrg, capture, ctrl, body ) ;
    npc->target.capture = capture ;
}

void	NPC_InitPose( NPCWORK *npc, FVECTOR *pos, int dir, int mar, int mot )
{
	OBJECT		*body ;
	CONTROL		*ctrl ;
	NPCACT *act ;

	act = &npc->action ;
	body = npc->body ;
	ctrl = npc->ctrl ;

	ctrl->mov = *pos ;
	ctrl->turn.vy = ctrl->rot.vy = dir ;

	GM_ConfigControlHzxHeight( ctrl, 750.0F, ctrl->mov.vy + 100.0F ) ;
	GM_ConfigControlMapID( ctrl ) ;

	if ( act->current_mar != mar ) {
		act->current_mar = mar ;
		npc->body->m_ctrl->motion_arc = MT_GetMotionArchives( mar ) ;
		MT_FreeSequence( npc->body->m_ctrl->sar_ctrl ) ;
		npc->body->m_ctrl->sar_ctrl = MT_InitSequence( npc->body->m_ctrl->n_layer, mar, 0 ) ;
	}
	act->current_mot = mot ;
	GM_ConfigObjectAction( body, 0, mot, 0, 0xfffff, 0 ) ;
}

void	NPC_FreeResources( NPCWORK *npc )
{
	SearchAndKillAttachment_called( npc->body ) ;
	TAKABE_FreePuppetIK( npc->action.ikwork ) ;
    GM_FreeTarget( npc->target.deftrg ) ;	/* 親だけで良い */
    GM_FreeControl( npc->ctrl ) ;
    GM_FreeObject( npc->body ) ;

    if( npc->rctrl != NULL ) 	GM_FreeRadarControl( npc->rctrl ) ;
	if( npc->fcanim != NULL ) 	FC_ReleaseFaceControl( npc->fcanim ) ;
	if( npc->hom != NULL ) 		GM_FreeHomingTrg( npc->hom ) ;
}

void	NPC_InitRader( NPCWORK *npc, RADAR_CTRL	*rctrl, FVECTOR *mov,
			float upper, float lower, int range, int sight, int color )
{
	NPCACT *act ;

	act = &npc->action ;
	npc->rctrl = rctrl ;

	act->face_dir = 0 ;
	act->eye_range = range ;
	act->eye_sight = sight ;
	act->radar_color = color ;

	GM_InitRadarControl( rctrl, mov, RADAR_VISIBLE|RADAR_SIGHT, -1 );
	GM_RadarSetVRange( rctrl, upper , lower );
//	GM_RadarSetVRange( rctrl, 2000.0f , -2000.0f );
}

void NPC_InitHeadMark(void *w,NPCWORK *npc,int *flag, OBJECT *body, CONTROL *ctrl, TARGET *trg )
{
#ifdef HEAD_MARK_3
	npc->action.headmark = flag ;
	GV_SetActorChild( w, 
		(npc->headmark=NewControl_Headmark3( &(BODYWORLD( body, HUMAN21_ATAMA )), trg, ctrl )) ) ;
#else
	extern	void  *NewControl_Headmark2( FMATRIX *world, int *type, TARGET *, CONTROL * ) ;

	npc->action.headmark = flag ;
	GV_SetActorChild( w, 
		NewControl_Headmark2( &(BODYWORLD( body, HUMAN21_ATAMA )), flag, trg, ctrl ) ) ;
#endif
}

/* 口パク目パチ */
int NPC_InitFaceAnime( NPCWORK *npc, DG_EVMOBJ * evm, int mot_name, int inf_name )
{
	NewFaceManager_forProg( ) ;
	if ( (npc->fcanim = FC_NewFaceControl( evm, mot_name, 0 )) == NULL ) {
		return -1 ;
	}
    FC_ExecFaceAnim( npc->fcanim, 1, 300, 300 ) ;

	npc->inf_name = inf_name ;
	
	return 1 ;
}

void NPC_InitMotionAdjust ( NPCWORK *npc, NPCADJUST *nadj )
{
	npc->nadj = nadj ;
	nadj->adj_status = 0 ;
}

void NPC_InitHomingTarget( NPCWORK *npc, HOMING_TRG *hom, OBJECT *body, CONTROL *ctrl,TARGET *trg )
{
	npc->hom = hom ;

	GM_SetHomingTrg( hom, &(BODYWORLD( body, HUMAN21_MUNE )), body, &(ctrl->hzx_id), ctrl, 0 ) ;
	GM_SetHomingTrgTarget( hom, trg ) ;
	GM_PutHomingTrg( hom ) ;
	SET_FLAG( hom->status, HOMING_NPC ) ;
}

void NPC_SetSeID ( NPCWORK *npc, int se_id )
{
	npc->se_id = se_id ;
}

void NPC_SetStatus ( NPCWORK *npc, int status )
{
	npc->status |= status ;
}

/*
ルートナビ関連関数

void ENE_SetNextnode( rnavi )
next_node に 次のポイント番号をセット

int	ENE_SearchNearRootPoint( rnavi, pos )
一番近いポイントを返す

*/
void NPC_InitRouteNavi ( NPCWORK *npc, ROUTENAVI *rnavi, int route )
{
	extern int ENE_ReadNodes( ROUTENAVI *, int ) ;

	npc->rnavi = rnavi ;
	npc->rnavi->next_route = route ;
	ENE_ReadNodes( npc->rnavi, route ) ;
}

void NPC_SetDogtag( NPCWORK *npc, int model, char c )
{
	extern int SIG_CheckDogTagFlag(int num) ;
	extern void *NewCreateDogtag (OBJECT* obj, int model_name, FVECTOR* shift, int* flag);
	NPCACT	*act ;
	OBJECT	*body ;
	int i ;

	act = &npc->action ;
	body = npc->body ;
#if 1
	if ( (GCL_GetOption( c ) != NULL) &&
		 (GM_GameLevel != GM_LEVEL_E_EXTREME) ){
#else
	if ( GCL_GetOption( c ) != NULL ){
#endif
		char *ptr, *tmp, *country ;
		int	level, birth ;
		
		level = 0 ;
		switch ( GM_GameLevel ) {
			case GM_LEVEL_VERYEASY :
			break ;
			case GM_LEVEL_EASY :
				level = 1 ;
			break ;
			case GM_LEVEL_NORMAL :
				level = 2 ;
			break ;
			case GM_LEVEL_HARD :
				level = 3 ;
			break ;
			case GM_LEVEL_EXTREME :
				level = 4 ;
			break ;
		}

		tmp = GCL_NextStr();	/* NextStr 先取り */
		act->dogtag_item.proc[ 0 ] = GCL_GetNextInt();

		ptr = GCL_GetNextString();
		for ( i=0; i<level; i++ ) {
			ptr = GCL_GetNextString();
		}

		GCL_SetArgTop( ptr );
		act->dogtag_id = GCL_GetNextInt();
ASSERT( act->dogtag_id >= 0 ) ;
#ifdef DOGTAG_DOUBLE
		if ( GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 ) {
			for ( i=0; i<DOGTAG_RES_NUM; i++ ) {
				GCL_GetNextInt();
			}
		}
#endif
		act->dogtag_item.str = GCL_GetNextString();
		country = GCL_GetNextString();
		birth = GCL_GetNextInt();

		act->dogtag_item.probability[ 0 ] = 100 ;
		act->dogtag_item.n_proc = 1 ;
		act->dogtag_item.c_proc = 0 ;
printf("dogtag_id [ %d ] \n",act->dogtag_id ) ;
		if ( SIG_CheckDogTagFlag( act->dogtag_id ) ) {
			act->dogtag_item.c_proc = 1 ;	/* 取得済み */
		} else {
			act->sw_dogtag = 1 ;
			GV_SetActorChild( npc->character, 
				NewCreateDogtag( body, model, &DG_ZeroVector, &act->sw_dogtag) ) ;
		}
		{
			extern	void *NewNamePrint( OBJECT *body, char *str, int, int ) ;
			NewNamePrint( body, act->dogtag_item.str, act->dogtag_id, birth ) ;
		}
printf("dogtagname[%s]\n",act->dogtag_item.str);
	} else {
		act->dogtag_id = -1 ;
		act->dogtag_item.n_proc = 0 ;
		act->dogtag_item.c_proc = 0 ;
	}
}
