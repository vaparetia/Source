/*
	mt3_util.c
	ｍｔ３ファイルモーションデータ再生補助ルーチン（デバッグ用のため現在未使用）

	1999/07/07 K.Takabe
	$Id: mt3_util.c,v 1.1.1.3 2002/11/19 11:42:52 Yoshizawa1 Exp $

*/
/*
	《ＭＥＴＡＬ ＧＥＡＲ ＳＯＬＩＤ》

*/

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

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat ;
	FMATRIX	mat ;
} ScrPadWork ;

typedef struct {
	FVECTOR	vec ;
	FVECTOR	joints[64] ;
	FMATRIX	root ;
	FMATRIX	mats[64] ;
} ScrPadWork2 ;

static FVECTOR	ZeroFQuat = {0.0F,0.0F,0.0F,1.0F} ;

#if 0
/* ＭＴ３モーション再生情報をCONTROL,OBJECT構造体に反映させる */
void MT_SetObjectEmulate( MT3_CONTROL *mt3_ctrl, CONTROL *ctrl, OBJECT *object )
{
	MATRIX	mat ;
	SVECTOR	vec ;
	if ( mt3_ctrl->move != NULL ){
		RotMatrix( &ctrl->rot, &mat );
		ApplyMatrixSV( &mat, &mt3_ctrl->move->step, &vec );
		object->height = mt3_ctrl->move->step.vy ;
		ctrl->step.vx = vec.vx ;
		ctrl->step.vy = 0 ;
		ctrl->step.vz = vec.vz ;
	}
	if ( mt3_ctrl->flag & MT3_PLAYEND ){
		object->is_end = 1 ;
	} else {
		object->is_end = 0 ;
	}
}
#endif

/* オブジェクトの関節に合わせて絶対回転クォータニオンに変換 */
void MT_LocalToAbsoluteQuat( FVECTOR *rots, FVECTOR *abs, DG_OBJS *objs )
{
	ScrPadWork		*work = (ScrPadWork*)SCRPAD_ADDR ;
	FVECTOR	*joints = work->joints ;
	DG_DEF	*def ;
	DG_MDL	*mdl ;
	DG_OBJ	*obj ;
	int		i ;

	def = objs->def ;
	obj = objs->objs ;
	work->root = ZeroFQuat ;
	/* モデル情報から親子関係を取得して求める */
	for ( i = def->n_models ; i > 0 ; i-- ){
		mdl = obj->model ;
		work->quat = *rots ;
		MT_QuatMul( joints, &work->joints[ mdl->parent ], &work->quat );
		*abs = *joints ;
		obj++ ;
		rots++ ;
		joints++ ;
		abs++ ;
	}
}

/* 絶対回転クォータニオンをスクラッチパッドワークにセット */
void MT_SetAbsoluteQuaternions( FVECTOR *abs, DG_OBJS *objs )
{
	ScrPadWork		*work = (ScrPadWork*)SCRPAD_ADDR ;
	FVECTOR	*joints = work->joints ;
	DG_DEF	*def ;
	int		i ;

	def = objs->def ;
	for ( i = def->n_models ; i > 0 ; i-- ){
		*(u_long128*)joints = *(u_long128*)abs ;
		joints++ ;
		abs++ ;
	}
}

/* 絶対回転クォータニオンをオブジェクトのマトリクスに設定する */
void MT_SetObjsWorld( DG_OBJS *objs )
{
	ScrPadWork2		*work = (ScrPadWork2*)SCRPAD_ADDR ;
	FVECTOR			*joints = work->joints ;
	FMATRIX			*mats = work->mats ;
	DG_DEF	*def ;
	DG_MDL	*mdl ;
	DG_OBJ	*obj = objs->objs ;
	int		i ;

	def = objs->def ;
	if ( objs->root != NULL ) objs->world = *( objs->root );
	work->root = objs->world ;
	work->vec.vw = 1.0F ;
	{/* モデル情報から親子関係を取得して求める */
		for ( i = def->n_models ; i > 0 ; i-- ){
			FMATRIX	*parent ;
			mdl = obj->model ;
			MT_QuatToMat( mats, joints );
			_sceVu0MulMatrix( mats, &work->root, mats ) ;
			work->vec.vx = mdl->tx ;
			work->vec.vy = mdl->ty ;
			work->vec.vz = mdl->tz ;
			parent = &work->mats[ mdl->parent ] ;
			_sceVu0ApplyMatrix( &mats->m[3][0], parent, &work->vec );
			obj->world = *mats ;
			obj++ ;
			mats++ ;
			joints++ ;
		}
	}
}

