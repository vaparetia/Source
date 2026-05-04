//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
   brk_mgz_clb.c
   雑誌壊れ コールバック関数

   2000/06/20 T. Morita
   $Id: brk_mgz_clb.c,v 1.2 2002/12/11 14:02:42 takaki Exp $
*/
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_magazine.h"


/*

   uvいじり

結局、CV2のUV値が変であるため、時間が無駄なのでこっちで頑張っちゃいました。

 */
void BRK_MGZ_ChangeBook( MAGAZINE *p, int offset )
{
    int            j, l ;
    DG_DEF	  *def;
    DG_MDL	  *mdl;
    DG_MDLPACK	  *mdlpack;
    DG_TEX	  *tex;
    DG_OBJ	  *obj;
    DG_OBJ_PACKET *obj_packet;
    int	   add_v, add_u ;
#ifdef PSX2
    short	  *uv_read,*uv_write;
	int        k ;
#else
	DG_VERTEX_KMSS	*verts ;
#endif


    obj = p->objs->objs ;
    def = p->objs->def  ;
    mdl = def->models   ;

    for( j = 0; j < (int)def->n_models; j++ )
    {
	DG_MakeAnimVertsBuffer( &p->v_anime[j+offset], obj,
				DG_VANIME_UVS|DG_VANIME_SINGLE ) ;
	mdlpack    = mdl->packs   ;
	obj_packet = obj->packets ;
#ifdef PSX2
	uv_write   = (short *)obj->uvs[0]  ;
#else
	verts = p->v_anime[j+offset].verts_top[0] ;/*シングルなので、バッファはどっちでもいいのだ*/
#endif

	/* この分だけUV値のVをずらす */
	tex = (DG_TEX*)mdlpack->tex_id[0] ;
	if ( p->work->hzd_id == BRK_HZD_W24B_REFRESH ) {
	    add_v = DG_FTOI( ((tex->v_scale * (float)p->mgz_type ) / 8.97f) * 4096.0f ) ;
		add_u = -42 ;
	} else {
	    add_v = DG_FTOI( ((tex->v_scale * (float)p->mgz_type ) / 9.0f) * 4096.0f ) ;
		add_u = -20 ;
	}

#ifdef PSX2
	for( k=0 ; k<(int)mdl->n_packs ; k++ )
	{
	    uv_read = (short*)mdlpack->uvs[0] ;
	    /* UVデータ生成 */
	    for( l=mdlpack->n_verts ; --l>=0 ; uv_read+=2, uv_write+=2 )
	    {
		    uv_write[0] = uv_read[0] + add_u ;
		    uv_write[1] = uv_read[1] + add_v ;
	    }
	    uv_write = (short*)((long128*)obj->uvs[0] + obj_packet->uvs_offset[0]) ;

	    mdlpack++ ;
	    obj_packet++ ;
	}
#else
	for( l=mdl->n_verts ; --l>=0 ; ) {
		int u, v ;

		u = (int)verts->u0 + add_u*8*1.5f ;
		v = (int)verts->v0 + add_v*8*1.5f ;
		verts->u0 = u>32767 ? 32767 : u<0 ? 0 : u ;
		verts->v0 = v>32767 ? 32767 : v<0 ? 0 : v ;
		verts = (DG_VERTEX_KMSS *)( (char *)verts + mdl->stride ) ;
	}
#endif
#ifdef KP_WINDOWS
	/* Windows版では、固有頂点バッファが必要 */
	DG_CreateDGObjPrivateVertexBuffer(obj, obj->n_indices, obj->n_verts) ;
	DG_AssignDGObjVertexBuffer(obj) ;	// 頂点バッファ変更
	DG_AssignDGObjIndexBuffer(obj) ;	// Indexバッファ変更
	//DG_AssignDGObjCVertexBuffer(obj) ;
#endif
	mdl++ ;
	obj++ ;

    }
}

int BRK_MGZ_ChangeOpenBook( MAGAZINE *p )
{
    DG_OBJS *objs ;

    /* モデルを初期化 */
    if ( !(objs = DG_MakeObjs( p->def, DG_FLAG_SHADE| DG_FLAG_FINISHCALC, 0 )) )
	PERROR( "Can't initialize DG_OBJS(Maybe no memory) :: NewPutMagazineObject\n" ) ;
    DG_QueueObjs( objs ) ;
    GM_GroupObjs( objs, p->work->where ) ;
    _sceVu0CopyMatrix( &objs->world, &p->objs->world ) ;


    /* 全ての初期化を確認してから モデルを解放 */
    BRK_MGZ_FreeMagazine( p ) ;
    DG_SetLightMatrix( p->objs=objs, p->lights ) ;
    DG_GetLightMatrix( &p->mov.pos, p->lights ) ;

    BRK_MGZ_ChangeBook( p, 1 ) ;
    p->non_damg = 0 ;

    /* 当たりを付ける */
    BRK_MGZ_InitTarget( p, &p->target[0], &p->power[0], p->work->where, TARGET_ROTATE,
			&p->def->models[0].lx, &p->def->models[0].ux, &p->mov.pos ) ;
    BRK_MGZ_InitTarget( p, &p->target[1], &p->power[1], p->work->where, TARGET_ROTATE,
			&p->def->models[1].lx, &p->def->models[1].ux, &p->mov.pos ) ;
#if MAKING
    GV_DestroyOtherActor( p->view[0] ) ;
    p->view[0] = NULL ;
    p->view[1] = NewTargetView( &p->target[0],  200, 50, 32 ) ;
    p->view[2] = NewTargetView( &p->target[1],  200, 50, 32 ) ;
#endif

    return  0 ;
}

void BRK_MGZ_BreakMagazine( MAGAZINE *p )
{
    Work *work = p->work ;

    /* エフェクトは連続呼び できないようにする */
    if ( p->non_damg )
	return  ;
    {
	p->non_damg = 5 ;
	NewTs_Min_Fog( &p->mov.pos ) ;

	/* 敵兵見つかり用 */
	if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
	    GM_SetNoise( NOISE_S, &p->mov.pos, work->where ) ;
	/* SEを鳴らす */
	GM_SeSetMode( SD_A_BOOKBR01, &p->mov.pos, GM_SEMODE_BOMB ) ;
    }

    /*棚にならんでいる状態か？*/
    if ( p->flag == -1 )
    {
	p->flag = -2 ;/* 飛び出す */

	/* 壊れプロックを呼び出す */
	if ( work->proc_id )
	    GCL_ExecProc( work->proc_id, NULL ) ;
    }

    /*棚にならんでいない状態*/
    else if ( p->flag != -2 )
    {
	/* 挙動の初期化 */
	p->mov.pos_v.vy = 90 ;
	p->mov.rot_vx = (irnd()&0x00e0)-128 ;
	p->mov.rot_vy = (irnd()&0x00e0)-128 ;
	if ( p->mov.rot_x < 0 )
	    p->open_v = p->open>0 ? DEG2RAD(20.0f) : -DEG2RAD(20.0f) ;

	/* 当たり判定ありのアクト */
	p->flag = 4 ; /* collision immeadiately */
    }

    /* 雑誌のアクト全体をアクティブにする */
    p->work->n_magazine &= ~BRK_MGZ_INACTIVE ;
}

void BRK_MGZ_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    MAGAZINE *p = (MAGAZINE *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BLOW|WP_BLAST|WP_STUNGRENADE|WP_PUNCHALL) )
	{
	    if ( !BRK_UTL_BlowBlastCheck( &def->center, off ) )
		return ;

	    /* 振動 */
	    BRK_UTL_PK_Vibrate( &def->center, off ) ;
	}
	else if ( !(off->weapon_type & (WP_BULLET|WP_M92)) )
	    return ;

	BRK_MGZ_BreakMagazine( p ) ;
    }
}
