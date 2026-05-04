//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	putmodel.c
	間接モデル設置

	2000/03/22 Y.Korekado
	$Id: putmodel.c,v 1.1.1.3 2002/11/19 11:44:04 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewPutModel( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-r	向き
		-s  ステータス

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

#include	"gameheader.h"
#include	"camera.h"

#include	"korekado/conv/korekado.x"
#include	"../../kano/us_soldier/us_soldier.h"
/*----------------------------------------------------------------*/

//#define USE_CTRL
//#define USE_MOTION
//#define DEB_VIEW
//#define LOD_2

#define	BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define MAX_OBJECT	3	/* ＬＯＤ */
#define MAX_LOD	3	/* ＬＯＤ */

#define	PO_STATUS_SHADE		0x00000001
#define	PO_STATUS_SPOTLIGHT	0x00000002
#define	PO_STATUS_LOD		0x00000004
#define	PO_STATUS_OBJ_OFF	0x10000000

#define JOINT_NUM 21

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT		actor ;
#ifdef LOD_2
	OBJECT		body ;
	DG_DEF 		*lodbuff[MAX_LOD] ;
#else
	OBJECT		body[MAX_LOD] ;
#endif
	FMATRIX		lights[2] ;
	OBJECT_CHG	object_chg ;

	FVECTOR		pos ;
	SVECTOR		rot ;
	FVECTOR		scale ;
	int			c_obj ;
	int			status ;
	int			name ;
	int			objs_num ;
	int			lod_th[ MAX_OBJECT-1 ] ;

	u_int		count ;
	int			id ;

#ifdef DEB_VIEW
	int 		deb_num ;
#endif
} Work ;

static int	PUTMDL_ID = 0 ;
static OBJECT	*MASTER_MDL ;

enum {
	OBJ_OFF=0,
	OBJ_ON,
	OBJ_TURN,
	OBJ_MOVE,
	OBJ_PRESHADE,
	OBJ_KILL
};

/*----------------------------------------------------------------*/
/*	表示管理 */
#if 0//no use
static	void	ChangeDef( objs, def )
DG_DEF	*def ;
DG_OBJS	*objs ;
{
	DG_MDL		*mdl ;
	DG_OBJ	*obj ;
	DG_OBJ_PACKET	*pack ;
	int		i, j ;

	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; -- i ) {
		if ( obj->packets != NULL ){
			GV_Free( obj->packets );
		}
		obj ++ ;
	}

	mdl = def->models ;
	objs->def = def ;
	obj = objs->objs ;

	for ( i=0; i<objs->n_models; i++ ) {
		obj->model = mdl ;
		obj->n_packs = mdl->n_packs ;
		/* 頂点情報のポインタ設定 */
		obj->verts = (SVECTOR *)mdl->packs->verts ;
		obj->norms = (SVECTOR *)mdl->packs->norms ;
		obj->uvs[0] = mdl->packs->uvs[0] ;
		obj->uvs[1] = mdl->packs->uvs[1] ;
		obj->uvs[2] = mdl->packs->uvs[2] ;
		obj->packets = pack = GV_Malloc( sizeof(DG_OBJ_PACKET) * obj->n_packs );

		for ( j = 0 ; j < mdl->n_packs ; j++ ){
			if ( mdl->packs[j].n_verts > 64 ) printf("too many n_verts(%d)\n",mdl->packs[j].n_verts);
			pack->flag = mdl->packs[j].flag ;
			pack->n_verts = mdl->packs[j].n_verts ;
			pack->norms_offset = pack->verts_offset =
			  ( mdl->packs[j].n_verts + 1 ) / 2 ;
			pack->uvs_offset[2] = pack->uvs_offset[1] = pack->uvs_offset[0] =
			  ( mdl->packs[j].n_verts + 3 ) / 4 ;
			if ( !( pack->flag & DG_PACKFLAG_UV0 ) ) pack->uvs_offset[0] = 0 ;
			if ( !( pack->flag & DG_PACKFLAG_UV1 ) ) pack->uvs_offset[1] = 0 ;
			if ( !( pack->flag & DG_PACKFLAG_UV2 ) ) pack->uvs_offset[2] = 0 ;
			pack->tex_ptr[0] = &( (DG_TEX*)mdl->packs[j].tex_id[0] )->tex_trans ;
			pack->tex_ptr[1] = &( (DG_TEX*)mdl->packs[j].tex_id[1] )->tex_trans ;
			pack->tex_ptr[2] = &( (DG_TEX*)mdl->packs[j].tex_id[2] )->tex_trans ;
			pack++ ;
		}
		obj ++ ;
		mdl ++ ;
	}
}
#endif

static	int	CameraDis( pos )
FVECTOR		*pos ;
{
	GM_CameraSet	*cam ;
	FVECTOR		vec ;
	float dis ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/
//    _sceVu0SubVector( &vec, pos, &cam->target ) ;
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
    dis = GV_VecLen3F( &vec ) ;
	dis /= cam->angle/2 ;
	
	return (int)dis ;
}

static	void	SetLod( work )
Work		*work ;
{
	int cam_dis, i, lod ;

	if ( work->id == 0 ) return ;

	cam_dis = CameraDis( &work->pos ) ;
	lod = 0 ;
	for ( i=0; i<work->objs_num-1; i++ ) {
		if ( cam_dis > work->lod_th[ i ] ) lod = i+1 ;
	}

	if ( work->c_obj != lod ) {
		work->c_obj = lod ;
#ifdef LOD_2
		ChangeDef( work->body.objs, work->lodbuff[lod] ) ;
#else
		for ( i=0; i<work->objs_num; i++ ) {
			if ( lod == i ) {
				work->body[i].objs->flag &= ~DG_FLAG_INVISIBLE ;
			} else {
				work->body[i].objs->flag |= DG_FLAG_INVISIBLE ;
			}
		}
#endif

#ifdef DEB_VIEW
		work->deb_num |= work->c_obj << 3 ;
#endif
	}
}


/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{

	SetLod( work ) ;

	DG_SetPos2( &work->pos, &work->rot ) ;

#ifdef LOD_2
	if ( work->id == 0 ) {
		GM_ActObject( &work->body ) ;
	} else {
		DG_PutObjs( work->body.objs ) ;
		MT_ActMotion2( MASTER_MDL->m_ctrl, work->body.objs ) ;
	}
#else
	if ( work->id == 0 ) {
		GM_ActObject( &work->body[0] ) ;
	} else {
		DG_PutObjs( work->body[work->c_obj].objs ) ;
		MT_ActMotion2( MASTER_MDL->m_ctrl, work->body[work->c_obj].objs ) ;
	}
#endif

//	DG_GetLightMatrix( &work->pos, work->lights );
	work->count ++ ;
//printf("put model %f %f %f\n",work->pos.vx,work->pos.vy,work->pos.vz ) ;
}

static	void	Die( work )
Work		*work ;
{
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, name, where )
Work	*work ;
int		name ;
int		where ;
{
	char		*opt ;
	int			model[MAX_OBJECT], i, num ;

	GM_CurrentMap = where ;	
	work->id = PUTMDL_ID++ ;

	if ( ( opt = GCL_GetOption( 'm' ) ) != NULL ){
		if ( (work->objs_num = ENE_GclGetInt( opt, &model[0] )) == 0 ) return -1 ;
#ifdef LOD_2
		GM_InitObject( &work->body, model[0], BODY_FLAG );
		work->lodbuff[0] = work->body.objs->def ;
		work->c_obj = 0 ;
		for( i=1; i<work->objs_num; i++ ) {
			work->lodbuff[i] = (DG_DEF*) GV_GetCache( GV_CacheID( model[i], 'k' ) );
		}
#else
		GM_InitObject( &work->body[0], model[0], BODY_FLAG );
		work->c_obj = 0 ;
		for( i=1; i<work->objs_num; i++ ) {
			GM_InitObject( &work->body[i], model[i], BODY_FLAG );
			work->body[i].objs->flag |= DG_FLAG_INVISIBLE ;
		}
#endif
		printf("LOD NUM [%d]\n",work->objs_num ) ;
	}
	num = 0 ;
	if ( ( opt = GCL_GetOption( 't' ) ) != NULL ){
		num = ENE_GclGetInt( opt, &work->lod_th[0] ) ;
	}
	if ( num != work->objs_num-1 ) return -1 ;

#ifdef LOD_2
	GM_GroupObjs( work->body.objs, GM_CurrentMap ) ;
#else
	GM_GroupObjs( work->body[0].objs, GM_CurrentMap ) ;
#endif

	if ( work->id == 0 ) {
#ifdef LOD_2
		GM_ConfigObjectMotion( &work->body, 1, GV_StrCode("ushold"), MT_FLAG_HUMAN2 );
		GM_ConfigObjectAction( &work->body, 0, 84, 0, 0xfffff, 0 );
#else
		GM_ConfigObjectMotion( &work->body[0], 1, GV_StrCode("ushold"), MT_FLAG_HUMAN2 );
		GM_ConfigObjectAction( &work->body[0], 0, 84, 0, 0xfffff, 0 );
#endif
	}

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &work->pos ) ;
		work->pos.vy += 1000.0F ;
	} else {
		work->pos = DG_ZeroVector ;
	}

	if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){
		GCL_GetSV( opt, (short *)&work->rot ) ;
	} else {
		work->rot = DG_ZeroSVector ;
	}

	DG_SetPos2( &work->pos, &work->rot ) ;
#ifdef LOD_2
	DG_PutObjs( work->body.objs ) ;
#else
	DG_PutObjs( work->body[0].objs ) ;
#endif

#ifdef LOD_2
	GM_ConfigObjectLight( &work->body, work->lights );
#else
	for( i=0; i<work->objs_num; i++ ) {
		GM_ConfigObjectLight( &work->body[i], work->lights );
	}
#endif
	DG_GetLightMatrix( &work->pos, work->lights );

	if ( work->id == 0 ) {
#ifdef LOD_2
		MASTER_MDL = &work->body ;
#else
		MASTER_MDL = &work->body[0] ;
#endif
	}

	work->count = 0 ;

#ifdef DEB_VIEW
{
	extern void *NewBig_Add_Objnum( FVECTOR *, int * ) ;
	NewBig_Add_Objnum( &(work->pos), &(work->deb_num) ) ;
	work->deb_num = 0 ;/* 黄色 */
	work->deb_num |= work->c_obj << 3 ;
}
#endif

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewPutModel( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

