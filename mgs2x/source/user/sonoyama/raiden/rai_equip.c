//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   rai_equip.c
   ライデンの装備品管理（マガジン、ホルスター、グリップ）など

   2001/04/19	M.Sonoyama
   $Id: rai_equip.c,v 1.1.1.3 2002/11/19 11:51:01 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../../kano/attachment/attachments.h"

extern	void *NewAttachment2_called(int model_name,FVECTOR *v,SVECTOR *r,
									OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p);

typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT			*pbody ;

	void			*attach_mag_work ;
	DG_OBJS			*holstor ;
	DG_OBJS			*grip ;
	DG_OBJS			*hfb_sht ;
	DG_OBJS			*hfb_grip ;

	void			*attach_mag_work_gol ;
	void			*attach_knif_work_gol ;

	DG_EVMOBJ		*bodyarmor ;
	DG_EVMOBJ		*gbs_body ;

	void			*attach_dogtag_work ;

	int				name ;
	int				demo_invisible ;
	int				demo_invisible_blade_grip ;

	int				demo_invisible_blade_saya ;
	int				reserved ;
		
	SVECTOR			blade_rot ;
	SVECTOR			blade_turn ;
} Work ;

/*----------------------------------------------------------------*/

/* ライデンマガジンケース */
static	void	RaiMagCaseAct( Work *work, int item, int weapon )
{
	if ( !GM_CheckPlayerStatusEX( PLAYER_STEALTH, PLAYER2_NUDE ) &&
		 item != IT_Uniform &&
		 ( GM_WeaponNum( WP_m92 ) >= 0 ||
		   GM_WeaponNum( WP_Socom ) >= 0 ||
		   GM_WeaponNum( WP_Aks ) >= 0 ||
		   GM_WeaponNum( WP_m4 ) >= 0 ) ) {
		if ( work->attach_mag_work == NULL ) {
			FVECTOR			shift = { 44.0F, -36.0F, 32.0F } ;
			FVECTOR			shift2 = { 111.0F, 60.0F, 56.0F } ;
			work->attach_mag_work = NewAttachment2_called( GV_StrCode( "rai_mag" ), 
														  NULL, &DG_ZeroSVector,
														  work->pbody, 
														  HUMAN21_KOSHI, &shift2,
														  HUMAN21_HIDARI_ASHI1, &shift ) ;
		}
	} else if ( work->attach_mag_work != NULL ) {
		GV_DestroyOtherActor( work->attach_mag_work ) ;		
		work->attach_mag_work = NULL ;
	}
}

/* はだかドッグタグ */
static	void	RaiNakedDogtagAct( Work *work, int item, int weapon ) 
{
    extern void *NewEvmHairModel_called2( int name, int model_name, int sample_num,
										 OBJECT *target,
										 unsigned char *objnum,
										 int n_objnum,
										 FVECTOR *x,
										 SVECTOR *r,
										 float oval_param,
										 int collision_flag,
										 unsigned char *collision_objs,
										 int visible_flag,int light_flag,
										 int boundmodel_name ) ;

	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
		if ( work->attach_dogtag_work == NULL ) {
			if ( GV_GetCache( GV_CacheID( GV_StrCode( "rai_dogtag_naked_mh" ), 'e' ) ) != NULL ) { 
				unsigned char id_objnum[] = { 2 } ;
				unsigned char id_colobjs[] = { 2, 3, 7 } ;

				work->attach_dogtag_work 
					= NewEvmHairModel_called2( GV_StrCode( "ライデンドッグタグ" ),
											   GV_StrCode( "rai_dogtag_naked_mh" ), 25,
											   work->pbody,
											   id_objnum,
											   sizeof( id_objnum ) / sizeof( id_objnum[ 0 ] ),
											   NULL, NULL,
											   1.0F,
											   sizeof( id_colobjs ) / sizeof( id_colobjs[ 0 ] ),
											   id_colobjs,
											   0, 0, GV_StrCode( "rai_dogtag_bounding" ) ) ;

			}
		}
	} else if ( work->attach_dogtag_work != NULL ) {
		GV_DestroyOtherActor( work->attach_dogtag_work ) ;		
		work->attach_dogtag_work = NULL ;
	}
}

/* ホルスター＆グリップ */
static	void	RaiHolsterAct( Work *work, int item, int weapon ) 
{
	DG_OBJS		*objs ;

	if ( !GM_CheckPlayerStatusEX( PLAYER_STEALTH, PLAYER2_NUDE ) &&
		 ( GM_WeaponNum( WP_m92 ) >= 0 ||
		   GM_WeaponNum( WP_Socom ) >= 0 ) ) {
		objs = work->holstor ;
		if ( ( work->pbody->objs->flag & DG_FLAG_INVISIBLE0 ) && 
			 ( ( work->pbody->evmobj ? ( work->pbody->evmobj->flag & DG_EVMOBJ_INVISIBLE0 ) : 1 ) ) ) {
			DG_InvisibleObjsChanl( objs, 0 ) ;
			DG_VisibleObjsChanl( objs, 1 ) ;
		} else if ( item == IT_Uniform ) {
			DG_InvisibleObjs( objs ) ;
		} else {
			DG_VisibleObjsChanl( objs, 0 ) ;
			DG_VisibleObjsChanl( objs, 1 ) ;
		}
		objs->group_id = work->pbody->objs->group_id ;
		objs->root = &work->pbody->objs->objs[ HUMAN21_MIGI_ASHI1 ].world ;

		/* グリップはゴルと共通 */
		objs = work->grip ;
		if ( ( work->pbody->objs->flag & DG_FLAG_INVISIBLE0 ) && 
			 ( ( work->pbody->evmobj ? ( work->pbody->evmobj->flag & DG_EVMOBJ_INVISIBLE0 ) : 1 ) ) ) {
			DG_InvisibleObjsChanl( objs, 0 ) ;
			DG_VisibleObjsChanl( objs, 1 ) ;
		} else if ( !GM_CheckGameStatus( STATE_DEMO ) && 
				    !GM_CheckPlayerStatus( PLAYER_WEAPON_INVISIBLE ) &&
				    weapon == WP_Socom ) {
			DG_InvisibleObjs( objs ) ;
		} else if ( GM_CheckGameStatus( STATE_DEMO ) &&
				    work->demo_invisible == 1 ) {
			DG_InvisibleObjs( objs ) ;	
		} else {
			DG_VisibleObjsChanl( objs, 0 ) ;
			DG_VisibleObjsChanl( objs, 1 ) ;
		}
		objs->group_id = work->pbody->objs->group_id ;
		objs->root = &work->pbody->objs->objs[ HUMAN21_MIGI_ASHI1 ].world ;
	} else {
		DG_InvisibleObjs( work->holstor ) ;
		DG_InvisibleObjs( work->grip ) ;
	}
}

/* 刀さや */
static	void	RaiBladeCaseAct( Work *work, int item, int weapon ) 
{
	DG_OBJS		*objs ;
	FMATRIX		mat, mat2 ;
	FVECTOR		vec ;

	if ( GM_WeaponNum( WP_Blade ) > 0 &&
		!GM_CheckPlayerStatus( PLAYER_STEALTH | PLAYER_CB_BOX ) ) {
		if ( work->pbody->height < 350.0F &&
			 work->pbody->height > 0.0F ) {
			/* ほふく、ダウン中 */
			work->blade_turn.vz = 10 * 1024 / 90 ;
		} else {
			work->blade_turn.vz = 0 ;
		}
		work->blade_rot.vz = GV_NearExp4P( work->blade_rot.vz, work->blade_turn.vz ) ;

		DG_SetPos2( &DG_ZeroVector, &work->blade_rot ) ;
		DG_GetPos( &mat ) ;
		GV_MatToVec( &work->pbody->objs->objs[ HUMAN21_MUNE ].world, &vec ) ;
		DG_COPY_MAT( &mat2, &work->pbody->objs->objs[ HUMAN21_MUNE ].world ) ;
		GV_VecToMat( &DG_ZeroVector, &mat2 ) ;
		_sceVu0MulMatrix( &mat, &mat2, &mat ) ;
		GV_VecToMat( &vec, &mat ) ;

		objs = work->hfb_sht ;
		if ( ( work->pbody->objs->flag & DG_FLAG_INVISIBLE0 ) && 
			( ( work->pbody->evmobj ? ( work->pbody->evmobj->flag & DG_EVMOBJ_INVISIBLE0 ) : 1 ) ) ) {
			DG_InvisibleObjsChanl( objs, 0 ) ;
			DG_VisibleObjsChanl( objs, 1 ) ;
		} else if ( work->demo_invisible_blade_saya == 1 ) {
			DG_InvisibleObjsChanl( objs, 0 ) ;			
			DG_InvisibleObjsChanl( objs, 1 ) ;
		} else {
			DG_VisibleObjsChanl( objs, 0 ) ;
			DG_VisibleObjsChanl( objs, 1 ) ;
		}		
		objs->group_id = work->pbody->objs->group_id ;
		//objs->root = &work->pbody->objs->objs[ HUMAN21_MUNE ].world ;
		DG_COPY_MAT( &objs->world, &mat ) ;

		objs = work->hfb_grip ;
		if ( ( work->pbody->objs->flag & DG_FLAG_INVISIBLE0 ) && 
			( ( work->pbody->evmobj ? ( work->pbody->evmobj->flag & DG_EVMOBJ_INVISIBLE0 ) : 1 ) ) ) {
			DG_InvisibleObjsChanl( objs, 0 ) ;
			DG_VisibleObjsChanl( objs, 1 ) ;
		} else if ( work->demo_invisible_blade_grip == 1 ) {
			DG_InvisibleObjs( objs ) ;
		} else {
			DG_VisibleObjsChanl( objs, 0 ) ;
			DG_VisibleObjsChanl( objs, 1 ) ;
		}		
		objs->group_id = work->pbody->objs->group_id ;
		//objs->root = &work->pbody->objs->objs[ HUMAN21_MUNE ].world ;
		DG_COPY_MAT( &objs->world, &mat ) ;
	} else {
		DG_InvisibleObjs( work->hfb_sht ) ;		
		DG_InvisibleObjs( work->hfb_grip ) ;
	}
}

/* ボディーアーマー */
static	void	RaiBodyArmorAct( Work *work, int item, int weapon )
{	
	DG_EVMOBJ	*evm ;
	FMATRIX		*dst, *src ;
	FVECTOR		trans ;
	EVM_SKEL	*skel ;
	int			visible, i ;

	if ( work->bodyarmor == NULL ) return ;

	visible = 0 ;
	if ( !GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) &&
		 item == IT_Jacket ) {	
		evm = work->bodyarmor ;
		if ( ( work->pbody->objs->flag & DG_FLAG_INVISIBLE0 ) && 
			( ( work->pbody->evmobj ? ( work->pbody->evmobj->flag & DG_EVMOBJ_INVISIBLE0 ) : 1 ) ) ) {
			evm->flag |= DG_EVMOBJ_INVISIBLE0 ;
			evm->flag &= ~DG_EVMOBJ_INVISIBLE1 ;
			visible = 1 ;
		} else {
			evm->flag &= ~( DG_EVMOBJ_INVISIBLE0 | DG_EVMOBJ_INVISIBLE1 ) ;
			visible = 1 ;
		}				
		if ( visible ) {
			evm->group_id = work->pbody->objs->group_id ;
         DG_Arm_SwitchEvmBuffer(evm);
			//if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
			//	MT_EvmActMotion( work->pbody->m_ctrl, evm ) ;
			//} else {
				if ( work->pbody->evmobj != NULL ) {
					src = work->pbody->evmobj->matrix[ work->pbody->evmobj->use_buffer ] ;
					dst = evm->matrix[ evm->use_buffer ] ;
					for ( i = 0; i < evm->n_skeleton; i ++, src ++, dst ++ ) {
						DG_COPY_MAT( dst, src ) ;
					}
				} else {
					dst = evm->matrix[ evm->use_buffer ] ;
					skel = evm->def->skeleton ;
					for ( i = 0; i < evm->n_skeleton; i ++, dst ++, skel ++ ) {
						DG_COPY_MAT( dst, &work->pbody->objs->objs[ i ].world ) ;
						trans.vx = -skel->rt_tx ;
						trans.vy = -skel->rt_ty ;
						trans.vz = -skel->rt_tz ;
						trans.vw = 1.0F ;
						_sceVu0ApplyMatrix( ( FVECTOR * )dst->m[ 3 ], dst, &trans ) ;
					}					
				}
			//}
		}
	} else {
		work->bodyarmor->flag |= DG_EVMOBJ_INVISIBLE ;
	}
}

/* ゴルマガジン */
static	void	GolMagCaseAct( Work *work, int item, int weapon )
{
	if ( !GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) &&
		 item == IT_Uniform ) {
		if ( work->attach_mag_work_gol == NULL ) {
			FVECTOR			shift = { 97.0F, 45.0F, -79.0F } ;
			FVECTOR			shift2 = { -1.0F, -48.0F, -132.0F } ;
			work->attach_mag_work_gol = NewAttachment2_called( GV_StrCode( "gbs_mag" ), 
														  NULL, &DG_ZeroSVector,
														  work->pbody, 
														  HUMAN21_KOSHI, &shift,
														  HUMAN21_HIDARI_ASHI1, &shift2 ) ;
		}
	} else if ( work->attach_mag_work_gol != NULL ) {
		GV_DestroyOtherActor( work->attach_mag_work_gol ) ;		
		work->attach_mag_work_gol = NULL ;
	}
}

/* ゴルナイフ */
static	void	GolKnifeAct( Work *work, int item, int weapon )
{
	if ( !GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) &&
		 item == IT_Uniform ) {
		if ( work->attach_knif_work_gol == NULL ) {
			FVECTOR			shift = { -67.0F, 19.0F, 123.0F } ;
			FVECTOR			shift2 = { -3.0F, -144.0F, 94.0F } ;
			work->attach_knif_work_gol = NewAttachment2_called( GV_StrCode( "gbs_knif" ), 
														  NULL, &DG_ZeroSVector,
														  work->pbody, 
														  HUMAN21_KOSHI, &shift,
														  HUMAN21_MIGI_ASHI1, &shift2 ) ;
		}
	} else if ( work->attach_knif_work_gol != NULL ) {
		GV_DestroyOtherActor( work->attach_knif_work_gol ) ;
		work->attach_knif_work_gol = NULL ;
	}
}

/* ゴルからだ */
static	void	GolBodyAct( Work *work )
{
	DG_EVMOBJ	*evm ;
	FMATRIX		*dst, *src ;
	int			i, visible ;

	if ( !GM_CheckGameStatus( STATE_DEMO ) || work->gbs_body == NULL ) return ;
	evm = work->gbs_body ;
	if ( ( work->pbody->objs->flag & DG_FLAG_INVISIBLE0 ) && 
		 ( ( work->pbody->evmobj ? ( work->pbody->evmobj->flag & DG_EVMOBJ_INVISIBLE0 ) : 1 ) ) ) {
		evm->flag |= DG_EVMOBJ_INVISIBLE ;
		visible = 0 ;
	} else {
		evm->flag &= ~DG_EVMOBJ_INVISIBLE ;
		visible = 1 ;
	}				
	if ( visible ) {
		evm->group_id = work->pbody->objs->group_id ;
      DG_Arm_SwitchEvmBuffer(evm);
		src = work->pbody->evmobj->matrix[ work->pbody->evmobj->use_buffer ] ;
		dst = evm->matrix[ evm->use_buffer ] ;
		for ( i = 0; i < evm->n_skeleton; i ++, src ++, dst ++ ) {
			DG_COPY_MAT( dst, src ) ;
		}	
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	int			item, weapon ;

	/* メッセージチェック */
	{
		GV_MSG		*msg ;
		int			n_msg, code ;

		n_msg = GV_ReceiveMessage( work->name, &msg ) ;
		if ( n_msg > 0 ) {
			code = msg->message[ 0 ] ;
			if ( code >= 0 && code <= 1 && GM_CheckGameStatus( STATE_DEMO ) ) {
				work->demo_invisible = code ;
			} else if ( code >= 2 && code <= 3 ) {
				work->demo_invisible_blade_grip = code - 2 ;
			} else if ( code >= 4 && code <= 5 ) {
				work->demo_invisible_blade_saya = code - 4 ;
			}
		}
	}

	item = ( GM_CheckGameStatus( STATE_DEMO ) ) ? GM_Item : PL_GetPlayerItem() ;
	weapon = ( GM_CheckGameStatus( STATE_DEMO ) ) ? GM_Weapon : PL_GetPlayerWeapon() ;

	/*---- ライデン ----*/

	/* マガジンケース */
	RaiMagCaseAct( work, item, weapon ) ;
	/* ホルスター＆グリップ */
	RaiHolsterAct( work, item, weapon ) ;
	/* 刀さや */
	RaiBladeCaseAct( work, item, weapon ) ;
	/* ボディアーマー */
	RaiBodyArmorAct( work, item, weapon ) ;
	/* はだかドッグタグ */
	RaiNakedDogtagAct( work, item, weapon ) ;

	/*---- ゴルルコ兵装 ----*/
	GolMagCaseAct( work, item, weapon ) ;
	GolKnifeAct( work, item, weapon ) ;
	GolBodyAct( work ) ;
}

static	void	Die( Work *work )
{
	if ( work->attach_mag_work != NULL ) {
		GV_DestroyOtherActor( work->attach_mag_work ) ;
	}
	if ( work->holstor != NULL ) {
		DG_DequeueObjs( work->holstor ) ;
		DG_FreeObjs( work->holstor ) ;
	}
	if ( work->grip != NULL ) {
		DG_DequeueObjs( work->grip ) ;
		DG_FreeObjs( work->grip ) ;
	}
	if ( work->hfb_sht != NULL ) {
		DG_DequeueObjs( work->hfb_sht ) ;
		DG_FreeObjs( work->hfb_sht ) ;
	}
	if ( work->hfb_grip != NULL ) {
		DG_DequeueObjs( work->hfb_grip ) ;
		DG_FreeObjs( work->hfb_grip ) ;
	}
	if ( work->bodyarmor != NULL ) {
		DG_DequeueEvmObj( work->bodyarmor ) ;
		DG_FreeEvmObj( work->bodyarmor ) ;
	}

	if ( work->attach_dogtag_work != NULL ) {
		GV_DestroyOtherActor( work->attach_dogtag_work ) ;
	}

	if ( work->attach_mag_work_gol != NULL ) {
		GV_DestroyOtherActor( work->attach_mag_work_gol ) ;
	}
	if ( work->attach_knif_work_gol != NULL ) {
		GV_DestroyOtherActor( work->attach_knif_work_gol ) ;
	}
	if ( work->gbs_body != NULL ) {
		DG_DequeueEvmObj( work->gbs_body ) ;
		DG_FreeEvmObj( work->gbs_body ) ;
	}
}

/*----------------------------------------------------------------*/

static	int		GetResources( Work *work, OBJECT *body )
{
	DG_DEF		*def ;
	EVM_DEF		*evmdef ;
	DG_OBJS		*objs ;

	work->pbody = body ;

	/* マガジン */
	work->attach_mag_work = NULL ;
	/* ホルスター */
	work->holstor = NULL ;
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "rai_hlst" ), 'k' ) ) ;
	if ( def != NULL ) {
		objs = work->holstor = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
		DG_InvisibleObjs( objs ) ;
	}
	/* グリップ */
	work->grip = NULL ;
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "rai_grip" ), 'k' ) ) ;
	if ( def != NULL ) {
		objs = work->grip = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
		DG_InvisibleObjs( objs ) ;
	}
	/* 刀さや */
	work->hfb_sht = NULL ;
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "hfb_sht" ), 'k' ) ) ;
	if ( def != NULL ) {
		objs = work->hfb_sht = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
		DG_InvisibleObjs( objs ) ;
		work->demo_invisible_blade_saya = 0 ;
	} else {
		work->demo_invisible_blade_saya = 1 ;
	}

	/* 刀柄 */
	work->hfb_grip = NULL ;
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "hfb_grip_mount" ), 'k' ) ) ;
	if ( def != NULL ) {
		objs = work->hfb_grip = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADE, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
		DG_InvisibleObjs( objs ) ;
		work->demo_invisible_blade_grip = 0 ;	/* 表示 */
	} else {
		work->demo_invisible_blade_grip = 1 ;	/* 非表示 */
	}

	/* ボディアーマー */
	work->bodyarmor = NULL ;
	evmdef = ( EVM_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "rba_def_mh_mt" ), 'e' ) ) ;
	if ( evmdef != NULL ) {
		work->bodyarmor = DG_MakeEvmObj( evmdef, 0, 0 ) ;
		DG_QueueEvmObj( work->bodyarmor ) ;
		work->bodyarmor->light = body->objs->light ;
		work->bodyarmor->flag |= DG_EVMOBJ_INVISIBLE ;
		work->bodyarmor->root = &body->objs->world ;
	}

	/* ポリデモ時ゴルルコボディ */
	work->gbs_body = NULL ;
	/* モデルがゴル兵にすげかえられているときのみ */
	if ( GM_CheckGameStatus( STATE_DEMO ) &&
		 GM_Item == IT_Uniform &&
		 body->evmobj != NULL &&
		 body->evmobj->n_skeleton == 53 &&	/* addhandはここではじく */
		 body->evmobj->def != GV_GetCache( GV_CacheID( GV_StrCode( "rai_def_mh_mt" ), 'e' ) ) ) {
		evmdef = ( EVM_DEF * )GV_GetCache( GV_CacheID( GV_StrCode( "rai_gbs_body_mh_mt" ), 'e' ) ) ;
		if ( evmdef != NULL ) {
			work->gbs_body = DG_MakeEvmObj( evmdef, 0, 0 ) ;
			DG_QueueEvmObj( work->gbs_body ) ;
			work->gbs_body->light = body->objs->light ;
			work->gbs_body->flag |= DG_EVMOBJ_INVISIBLE ;
			work->gbs_body->root = &body->objs->world ;
		}	
	}

	/* はだか時ドッグタグ */
	work->attach_dogtag_work = NULL ;

	/* ゴル兵マガジン */
	work->attach_mag_work_gol = NULL ;
	/* ゴル兵ナイフ */
	work->attach_knif_work_gol = NULL ;

	work->demo_invisible = 0 ;
	return 0 ;
}

void			*PL_RaidenEquipmentManager( OBJECT *body, int name )
{
	Work		*work ;

	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, sizeof( Work ),
										 PLAYER_CHECK_ACTOR_PRIO ) ;
	} else {
		work = ( Work * )GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof( Work ),
										 255 ) ;
	}
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	GetResources( work, body ) ;
	work->name = name ;
	return work ;
}








typedef	struct	{
    GV_ACT_EX	actor ;
	OBJECT     *body  ;
    int         open  ;
    int         close ;
} WorkNin ;

static void ActNinja( WorkNin *work )
{
	int inv ;

	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ) {
		inv = 2 ;
	} else {
		inv = 1 ;
	}
	if ( GM_Item == IT_Thermal || GM_Item == IT_NightVision ) {
		work->close = inv ;
		work->open  = 0 ;
    } else {
		work->close = 0 ;
		work->open  = inv ;
    }
}

static int GetResourcesNinja( WorkNin* work, OBJECT *body ) 
{
    void *NewConnectEquip( DG_OBJS *objs, int joint, int kms, int *flag, int disp_flag ) ;
    void *child ;

	work->body = body ;
    if ( !(child = NewConnectEquip( body->objs,
				    HUMAN21_ATAMA,
				    7862789/*rai_nin_face_open */,
				    &work->open,
				    DG_FLAG_IRREACTION )) ) {
	return -1 ;
    }
    GV_SetActorChild( work, child ) ;

    if ( !(child = NewConnectEquip( body->objs,
				    HUMAN21_ATAMA,
				    4024755/*rai_nin_face_close */,
				    &work->close,
				    DG_FLAG_IRREACTION )) ) {
	return -1 ;
    }
    GV_SetActorChild( work, child ) ;

    return 0 ;
}


void	*PL_RaidenNinjaManager( OBJECT *body, int name )
{
    WorkNin *work ;

    work = ( WorkNin * )GV_CreateActor( GV_ACTOR_PLAYER,
					GV_CLASS_OBJECT,
					sizeof( WorkNin ),
					PLAYER_CHECK_ACTOR_PRIO ) ;
    if ( work ) {
	GV_SetActor( &work->actor, ActNinja, NULL ) ;
	GV_ActorEX( &work->actor ) ;

	if ( GetResourcesNinja( work, body ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
