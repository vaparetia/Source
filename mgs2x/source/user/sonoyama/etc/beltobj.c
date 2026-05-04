//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   beltobj.c
   ベルトコンベア上荷物
   
   2001/01/12 M.Sonoyama
   $Id: beltobj.c,v 1.1.1.3 2002/11/19 11:50:41 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libutl.h>
#endif

#include	"gameheader.h"

#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

/*------------------------------------------------------------*/

typedef	struct _BELTOBJ_LIST {
	struct _BELTOBJ_LIST	*next ;
	OBJECT					*body ;
} BELTOBJ_LIST ;

static	BELTOBJ_LIST	BeltObjList ;
static	int				AlertMode, AlertCheckTime ;
static	int				AttributeChange ;
static	int				N_Incremental ;

static	FVECTOR			Wide = { 1.50F, 2.0F, 1.50F } ;

/*------------------------------------------------------------*/

/* 荷物リスト関連 */

/* リスト登録 */
static	void	AddList( BELTOBJ_LIST *this )
{
	this->next = BeltObjList.next ;
	BeltObjList.next = this ;
}

/* リストから削除 */
static	int		RemoveList( BELTOBJ_LIST *this )
{
	BELTOBJ_LIST	*cur, *prev ;

	prev = &BeltObjList ;
	cur = BeltObjList.next ;
	while( cur != NULL ) {
		if ( cur == this ) {
			prev->next = cur->next ;
			return 1 ;
		}
		cur = cur->next ;
	}
	return 0 ;
}

/* 指定トラップ内に荷物があるかチェック */

static	int	CheckOneObjInsideTrap( BELTOBJ_LIST *this, FVECTOR *tb1, FVECTOR *tb2 ) 
{
	int			i ;
	FVECTOR		p[ 8 ], b1, b2 ;
	DG_MDL		*mdl ;

	mdl = this->body->objs->def->models + 0 ;
	for ( i = 0; i < 8; i ++ ) {
		p[ i ].vx = ( i & 1 ) ? mdl->lx : mdl->ux ;
		p[ i ].vy = ( i & 2 ) ? mdl->ly : mdl->uy ;
		p[ i ].vz = ( i & 4 ) ? mdl->lz : mdl->uz ;
		p[ i ].vw = 1.0F ;
	}
	DG_SetPos( &this->body->objs->objs[ 0 ].world ) ;
	DG_PutVector( p, p, 8 ) ;

	DG_COPY_VEC( &b1, &p[ 0 ] ) ;
	DG_COPY_VEC( &b2, &p[ 0 ] ) ;
	for ( i = 0; i < 8; i ++ ) {
		if ( p[ i ].vx < b1.vx ) b1.vx = p[ i ].vx ;
		if ( p[ i ].vy < b1.vy ) b1.vy = p[ i ].vy ;
		if ( p[ i ].vz < b1.vz ) b1.vz = p[ i ].vz ;
		if ( p[ i ].vx > b2.vx ) b2.vx = p[ i ].vx ;
		if ( p[ i ].vy > b2.vy ) b2.vy = p[ i ].vy ;
		if ( p[ i ].vz > b2.vz ) b2.vz = p[ i ].vz ;
	}	
	if ( ( b2.vx < tb1->vx || b1.vx > tb2->vx ) ||
		 ( b2.vy < tb1->vy || b1.vy > tb2->vy ) ||
		 ( b2.vz < tb1->vz || b1.vz > tb2->vz ) ) return 0 ;

//	NewBoundingBoxView( tb1, tb2, 32, 232, 32 ) ;
//	NewBoundingBoxView( &b1, &b2, 32, 32, 232 ) ;
	return 1 ;
}

int		NewBeltObjCheckInsideTrap( void )
{
	int				map, trp_name ;
	HZX_GROUP_ID	hzx_id ;
	HZX_BLOCK		*blk ;
	HZX_TRP			*trp ;
	FVECTOR			trp_b1, trp_b2 ;
	BELTOBJ_LIST	*this ;

	map = GCL_GetOptionValue( 'm', 0 ) ;
	trp_name = GCL_GetOptionValue( 't', 0 ) ;
	hzx_id = GM_GetHzxGroupID( GM_GetMapID( map ) ) ;
	ASSERT( hzx_id != 0 ) ;
	/* マップは単一であること */
	HZX_FindTrap( hzx_id, trp_name, &blk, &trp ) ;
	ASSERT( trp != NULL && blk != NULL ) ;
	trp_b1.vx = blk->tx + trp->b1.vx ;
	trp_b1.vy = blk->ty + trp->b1.vy ;
	trp_b1.vz = blk->tz + trp->b1.vz ;
	trp_b2.vx = blk->tx + trp->b2.vx ;
	trp_b2.vy = blk->ty + trp->b2.vy ;
	trp_b2.vz = blk->tz + trp->b2.vz ;

	this = BeltObjList.next ;
	while( this != NULL ) {
		if ( CheckOneObjInsideTrap( this, &trp_b1, &trp_b2 ) ) {
			return 1 ;
		}
		this = this->next ;
	}
	return 0 ;
}

/*------------------------------------------------------------*/

/* 解体Ｃ４くっつき用ワーク */
typedef	struct	{
	CONTROL_NOEVT		control ;
	HOMING_TRG			homing_trg ;
	CAPTURE_TARGET		capture ;
} WorkC4 ;

/* 個々の荷物 */

typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT			body ;
	FMATRIX			lights[ 2 ] ;
	FVECTOR			pos ;
	TARGET			target ;
	POWER_TARGET	power ;
	HZX_EVT			evt ;
	int				map ;
	int				name ;
	int				hzx_id ;
	void			*objhzx ;
	void			*objhzx_wide ;
	BELTOBJ_LIST	list ;
	WorkC4			*workc4 ;
	int				break_model ;
	int				type ;
	int				n_incremental ;
} Work ;

extern	void 	*NewMakeObjectBoundHazard2( HZX_GROUP_ID hzx_id, DG_OBJS *objs, 
										   int seg_atr, int flr_atr, int disp ) ;
extern	void 	*NewMakeObjectBoundHazard3( HZX_GROUP_ID hzx_id, DG_OBJS *objs, 
										    int seg_atr, int flr_atr, int disp, FVECTOR *scale ) ;
extern	void	PL_UpdateObjectBoundHazard( void *work, FMATRIX *world ) ;
extern	void	PL_ObjectBoundHazardSetAttribute( void *ptr, int seg_atr, int flr_atr ) ;
extern	void	PL_ObjectBoundHazardResetAttribute( void *ptr, int seg_atr, int flr_atr ) ;

enum {
	MSG_POSITION = 0,
	MSG_MAP,
} ;

/*------------------------------------------------------------*/

static	void	Hitted( TARGET *off, TARGET *def, void *ptr )
{
	long64		wt ;

	if ( def->damaged & TARGET_POWER ) {
		Work	*work;
		FVECTOR	norm;
		FVECTOR	pos;
		FMATRIX	mat;
		
		work = ( Work * )ptr ;

		wt = off->weapon_type ;
		
		if ( ( off->class & TARGET_ONLINE ) && 
			 ( wt & (WP_M92|WP_USP|WP_SOCOM|WP_PSG1|WP_AKS|WP_M4|
					 WP_PSG1T|WP_CLAYMORE) ) ) {
			/* 弾 */
			_sceVu0Normalize( &norm, &off->size );
			//printf("norm:%f %f %f %f\n",off->size.vx,off->size.vy,off->size.vz,off->size.vw);
			if( _sceVu0InnerProduct( &off->power->force/*攻撃方向*/, &off->size/*法線*/ ) > 0.0f ){
				_sceVu0ScaleVector( &norm, &norm,-1.0f );
			}else{
				_sceVu0ScaleVector( &norm, &norm, 1.0f );
			}
			_sceVu0InversMatrix( &mat, &work->body.objs->world );

			DG_SetPos( &mat );
			DG_PutVector( &def->hit, &pos, 1 );
			DG_RotVector( &norm, &norm, 1 );

			if ( work->type == 0 ) {	/* 紙タイプ 弾痕 */
				extern void *DynamicDanKon( FMATRIX *world, FVECTOR *norm, FVECTOR *pos );
				GV_SetActorChild( work, DynamicDanKon( &work->body.objs->world, &norm, &pos ) );
			}
			if ( work->type == 0 ) {	/* 紙タイプ 破片 */
				extern void *NewBoxBreakDust( FVECTOR *pos, FVECTOR *force );
				GV_SetActorChild( work, NewBoxBreakDust( &def->hit, &off->power->force ) );
			} 
			/* 跳弾 */
			{
				extern void	NewSpark( FMATRIX * ) ;
				FMATRIX		m ;
				FVECTOR		v ;
				SVECTOR		r ;

				_sceVu0SubVector( &v, &off->offset, &off->center ) ;
				GV_VecToRot( &v, &r ) ; r.vx -= 1024 ;
				DG_SetPos2( &def->hit, &r ) ;
				DG_GetPos( &m ) ;
				GV_MatToVec( &m, &v ) ;
				DG_ReflectMatrix( &off->size, &m, &m ) ;
				GV_VecToMat( &v, &m ) ;
				if ( work->type == 1 ) {
					NewSpark( &m ) ;
					//printf( "[%d] tetsu se!!\n", GV_Time ) ;
					GM_SeSetMode( SD_A_RICOCH03, &v, GM_SEMODE_BOMB ) ;
				} else {
					//printf( "[%d] danbo-ru se!!\n", GV_Time ) ;
					GM_SeSetMode( SD_A_RICOCH05, &v, GM_SEMODE_BOMB ) ;					
				}
			}
		} else if ( wt & (WP_RGB6|WP_NIKITA|WP_STINGER|WP_C4BOMB|WP_GRENADE) ) {
			/* 爆発 */
			if ( work->break_model != 0 ) {
				OBJECT		new ;
				FMATRIX		world ;
				int			map ;
				/* モデル入れ替え */
				DG_COPY_MAT( &world, &work->body.objs->world ) ;
				map = work->body.map_name ;
				GM_InitObject( &new, work->break_model, BODY_FLAG ) ;
				/* モデルがだせなかったらそのまま */
				if ( new.objs != NULL ) {
					GM_FreeObject( &work->body ) ;
					work->body.objs = new.objs ;
					/* Ｃ４がついてたら通知 */
					if ( work->workc4 != NULL ) {
						work->workc4->capture.flag &= ~CAPTURE_C4EXIST ;
					}
				}
				GM_ConfigObjectLight( &work->body, work->lights ) ;
				DG_SetPos( &world ) ;
				work->body.map_name = map ;
				GM_ActObject( &work->body ) ;
				work->break_model = 0 ;
			} else if ( work->type == 0 ) {
				extern void *NewBoxBreakDust( FVECTOR *pos, FVECTOR *force );
				/* 紙タイプは爆発したらおしまい */
				GM_MakeTargetForce( off, def ) ;
				NewBoxBreakDust( &def->center, &def->power->force ) ;
				NewBoxBreakDust( &def->center, &def->power->force ) ;
				NewBoxBreakDust( &def->center, &def->power->force ) ;
				NewBoxBreakDust( &def->center, &def->power->force ) ;
				GV_DestroyActor( work ) ;
			}
		}
		GM_ClearTargetDamage( def ) ;
	}
}

static	void	CheckMessage( Work *work )
{
	GV_MSG		*msg ;
	FVECTOR		pos ;
	int			n_msg, code, where ;
	
	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
		case MSG_POSITION :		/* 位置変更 */
			pos.vx = ( float )msg->message[ 1 ] ;
			pos.vy = ( float )msg->message[ 2 ] ;
			pos.vz = ( float )msg->message[ 3 ] ;
			GV_VecToMat( &pos, &work->body.objs->world ) ;
			DG_COPY_VEC( &work->pos, &pos ) ;
			break ;
		case MSG_MAP :			/* マップ変更 */
			where = GM_GetMapID( msg->message[ 1 ] ) ;
			work->hzx_id = GM_GetHzxGroupID( where ) ;
			work->map = where ;
//			GV_DestroyOtherActor( work->objhzx ) ;
			GV_DestroyOtherActor( work->objhzx_wide ) ;
//			work->objhzx = NewMakeObjectBoundHazard2( work->hzx_id, work->body.objs,
//													 0, 0, 1 ) ;			
			work->objhzx_wide = NewMakeObjectBoundHazard3( work->hzx_id, work->body.objs,
														   HZX_SEG_ATR_ALL & ~HZX_SEG_NO_ENEMY_EYES,
														   HZX_FLOOR_ATR_ALL & ~HZX_FLOOR_NO_ENEMY_EYES,
														   0, &Wide ) ;			
			if ( work->workc4 != NULL ) {
				work->workc4->control.map = where ;
			}
			break ;
		default :
		  ;
		}
		msg ++ ;
	}
}

/*------------------------------------------------------------*/

static	void	Act( Work *work )
{
	FVECTOR		npos ;
	FMATRIX		world ;

	CheckMessage( work ) ;
	GM_SetCurrentMap( work->map ) ;
#if 0
	if ( GV_Time % 2 == work->n_incremental ) {
		if ( work->evt.n_inside > 0 ) {
			int			i ;
		
			for ( i = 0; i < work->evt.n_inside; i ++ ) {
				if ( !HZX_CheckInsideTrap( work->hzx_id, &work->pos, work->evt.inside[ i ] ) ) {
					HZX_EnterTrap( work->hzx_id, &work->evt ) ;			
					break ;
				}
			}
		} else {
			HZX_EnterTrap( work->hzx_id, &work->evt ) ;			
		}
	}
#else
	if ( GV_Time % 2 == work->n_incremental ) {
		HZX_EnterTrap( work->hzx_id, &work->evt ) ;
	}
#endif

	if ( PL_ExecMoveFloorFunc( &npos, work->hzx_id, ( FVECTOR * )work->body.objs->world.m[ 3 ],
							   &work->evt, work->name ) ) {
		DG_COPY_MAT( &world, &work->body.objs->world ) ;
		GV_VecToMat( &npos, &world ) ;
		DG_COPY_VEC( &work->pos, &npos ) ;
		DG_SetPos( &world ) ;
		GM_ActObject( &work->body ) ;
//		PL_UpdateObjectBoundHazard( work->objhzx, &work->body.objs->world ) ;
		PL_UpdateObjectBoundHazard( work->objhzx_wide, &work->body.objs->world ) ;
		DG_GetLightMatrix( &npos, work->lights ) ;
		GM_MoveTargetMap( &work->target, &work->pos, work->map ) ;
	} else {
		//printf( "beltobj outside beltconveyer!! : %x : ", work->map ) ;
	}
	//NewTargetView2( &work->target, 32, 232, 32 ) ;
#if 0
	if ( GV_PadData->status & PAD_L1 ) {
		printf( "beltobj %x : ", work->name ) ;
		DumpVec( ( FVECTOR * )work->body.objs->world.m[ 3 ] ) ;
	}
#endif
	/* 危険モードで視界ＯＦＦ解除 */
	if ( AlertCheckTime != GV_Time ) {
		AttributeChange = 0 ;
		AlertCheckTime = GV_Time ;
		if ( GM_AlertMode != AlertMode &&
			 ( GM_AlertMode == ALERT_MODE_ALERT ||
			   AlertMode == ALERT_MODE_ALERT ) ) {
			AttributeChange = 1 ;
		}
		AlertMode = GM_AlertMode ;
	}
	if ( AttributeChange ) {
		if ( GM_AlertMode == ALERT_MODE_ALERT ) {
			PL_ObjectBoundHazardResetAttribute( work->objhzx_wide, 
											    HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES ) ;
		} else {
			PL_ObjectBoundHazardSetAttribute( work->objhzx_wide, 
											  HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES ) ;
		}
	}
}

static	void	Die( Work *work )
{
	GM_FreeObject( &work->body ) ;
	GM_FreeTarget( &work->target ) ;
//	GV_DestroyOtherActor( work->objhzx ) ;
	GV_DestroyOtherActor( work->objhzx_wide ) ;
	RemoveList( &work->list ) ;
	if ( work->workc4 != NULL ) {
		WorkC4	*wc4 ;

		wc4 = work->workc4 ;
		GM_FreeHomingTrg( &wc4->homing_trg ) ;
		wc4->capture.flag &= ~CAPTURE_C4EXIST ;
		GV_DelayedFree( wc4 ) ;
		work->workc4 = NULL ;
	}
}

/*------------------------------------------------------------*/

static	void	SetTarget( Work *work, int where )
{
	TARGET		*t ;
	FVECTOR		bound, size, ofs ;

	DG_SetPos( &work->body.objs->world ) ;

	bound.vx = work->body.objs->def->ux - work->body.objs->def->lx ;
	bound.vy = work->body.objs->def->uy - work->body.objs->def->ly ;
	bound.vz = work->body.objs->def->uz - work->body.objs->def->lz ;
	DG_RotVector( &bound, &bound, 1 ) ;

	size.vx = DG_FABS( bound.vx / 2.0F ) ;
	size.vy = DG_FABS( bound.vy / 2.0F ) ;
	size.vz = DG_FABS( bound.vz / 2.0F ) ;

	bound.vx = work->body.objs->def->ux + work->body.objs->def->lx ;
	bound.vy = work->body.objs->def->uy + work->body.objs->def->ly ;
	bound.vz = work->body.objs->def->uz + work->body.objs->def->lz ;
	DG_RotVector( &bound, &bound, 1 ) ;

	ofs.vx = bound.vx / 2.0F ;
	ofs.vy = bound.vy / 2.0F ;
	ofs.vz = bound.vz / 2.0F ;
	
	t = &work->target ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER | TARGET_SEEK, where,
				  BOTH_SIDE, &size, &ofs ) ;
	GM_SetPowerTarget( t, &work->power, POWER_CONST, 255, 0, 0, &DG_ZeroVector ) ;
	GM_MoveTargetMap( t, &work->pos, work->map ) ;
	GM_SetTargetCallBack( t, Hitted, work ) ;
	GM_PutTarget( t ) ;
}

static	int	GetResources( Work *work, int name, int where )
{
	int			model ;
	FVECTOR		pos ;
	SVECTOR		rot ;

	model = GCL_GetOptionValue( 'm', 0 ) ;
	GM_InitObject( &work->body, model, BODY_FLAG ) ;
	GM_ConfigObjectLight( &work->body, work->lights ) ;
	GCL_GetOption( 'p' ) ;
	pos.vx = ( float )GCL_GetNextInt() ;
	pos.vy = ( float )GCL_GetNextInt() ;
	pos.vz = ( float )GCL_GetNextInt() ;
	GCL_GetOption( 'r' ) ;
	rot.vx = ( float )GCL_GetNextInt() ;
	rot.vy = ( float )GCL_GetNextInt() ;
	rot.vz = ( float )GCL_GetNextInt() ;
	DG_SetPos2( &pos, &rot ) ;
	GM_ActObject( &work->body ) ;
	DG_GetLightMatrix( &pos, work->lights ) ;
	if ( !GM_IsOneID( where ) ) where = GM_GetMapIDfromPos( where, &pos ) ;
	ASSERT( where != 0 ) ;
	work->name = name ;
	work->hzx_id = GM_GetHzxGroupID( where ) ;
	work->map = where ;
//	work->objhzx = NewMakeObjectBoundHazard2( work->hzx_id, work->body.objs,
//											 0, 0, 1 ) ;
	work->objhzx_wide = NewMakeObjectBoundHazard3( work->hzx_id, work->body.objs,
												   HZX_SEG_ATR_ALL & ~HZX_SEG_NO_ENEMY_EYES,
												   HZX_FLOOR_ATR_ALL & ~HZX_FLOOR_NO_ENEMY_EYES,
												   0, &Wide ) ;			
	DG_COPY_VEC( &work->pos, &pos ) ;
	HZX_SetEvent( &work->evt, name, &work->pos, &DG_ZeroSVector ) ;
	
	work->list.body = &work->body ;
	AddList( &work->list ) ;

	SetTarget( work, where ) ;

	/* 解体Ｃ４つけ用 */
	{
		int			flag ;
		WorkC4		*wc4 ;

		flag = GCL_GetOptionValue( 'f', 0 ) ;
		if ( flag & 1 ) {
			wc4 = work->workc4 = GV_Malloc( sizeof( WorkC4 ) ) ;
			if ( wc4 == NULL ) return -1 ;
			GV_ZeroMemory( wc4, sizeof( WorkC4 ) ) ;
			wc4->control.name = name ;
			wc4->control.map = where ;
			GM_SetCaptureTarget( &work->target, &wc4->capture, 
								 ( CONTROL * )&wc4->control, &work->body ) ;
			/* 解体Ｃ４が荷物を検索するためだけのものなので、
			   CAPTURE_CLASSはりセットする */
			work->target.class &= ~TARGET_CAPTURE ;
			/* これも解体Ｃ４が荷物を検索するためだけのものなので、
			   SKIPフラグを立てておく */
			GM_SetHomingTrg( &wc4->homing_trg, &work->body.objs->world, &work->body,
							 &wc4->control.map, ( CONTROL * )&wc4->control, HOMING_SKIP ) ;
			wc4->homing_trg.trg = &work->target ;
			GM_PutHomingTrg( &wc4->homing_trg ) ;
		} else {
			work->workc4 = NULL ;
		}
	}

	work->break_model = GCL_GetOptionValue( 'b', 0 ) ;
	work->type = GCL_GetOptionValue( 't', 0 ) ;
	
	work->n_incremental = N_Incremental % 2 ;
	N_Incremental ++ ;

	return 0 ;
}

void	*NewBeltObject( int name, int where )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/*----------------------------------------------------------------*/

int		NewBeltObjectManager( void )
{
	BeltObjList.next = NULL ;
	AlertMode = ALERT_MODE_SNEAK ;
	AlertCheckTime = 0 ;
	AttributeChange = 0 ;
	N_Incremental = 0 ;
	return 0 ;
}
