//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_obj_msg.c
   オブジェクト壊れ

   2000/01/15 T. Morita
   $Id: brk_obj_msg.c,v 1.1.1.3 2002/11/19 11:45:38 Yoshizawa1 Exp $
*/
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

#include "brk_object.h"
#include "../brk_utl/brk_utl.x"



void BRK_OBJ_RegistFoot( int name, OBJECT *body )
{
    GV_MSG msg ;
    u_int buffer[2] ;

    /* 壊れポテトに足のポインタを登録する。*/
    buffer[0] = BRK_OBJ_M_REGIST_FOOT ;
    buffer[1] = (u_int)body ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}

void BRK_OBJ_GetBoxVitality( int name, int **v )
{
    GV_MSG msg ;
    u_int buffer[2] ;


    /* 壊れポテトの箱の寿命を取得する。*/
    buffer[0] = BRK_OBJ_M_GET_BOX_VITALITY ;
    buffer[1] = (u_int)v ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}

void BRK_OBJ_GetPotatoNum( int name, int **v )
{
    GV_MSG msg ;
    u_int buffer[2] ;

    /* 壊れポテトの数を取得する。*/
    buffer[0] = BRK_OBJ_M_GET_OBJ_NUM ;
    buffer[1] = (u_int)v ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}

void BRK_OBJ_GetPotatoPos( int name, FVECTOR **v, int n )
{
    GV_MSG msg ;
    u_int buffer[3] ;

    /* 壊れポテトの位置を取得する。*/
    buffer[0] = BRK_OBJ_M_GET_OBJ_POS ;
    buffer[1] = (u_int)v ;
    buffer[2] = (u_int)n ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 3 ;
    GV_SendMessage( &msg ) ;
}

void BRK_OBJ_GetBoxPos( int name, FVECTOR **v )
{
    GV_MSG msg ;
    u_int buffer[2] ;

    /* 壊れポテトの箱の位置を取得する。*/
    buffer[0] = BRK_OBJ_M_GET_OBJ_POS ;
    buffer[1] = (u_int)v ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}

static inline void BRK_OBJ_SetFoot( Work *work, OBJECT *body )
{
    work->n_foot-- ;
    work->foot->parts.foot = (FVECTOR *)&body->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[W] ;
    work->foot->target.size.vx = 200 ;
    work->foot->act = BRK_OBJ_ActOnFoot ;
    work->foot++ ;
    work->foot->parts.foot = (FVECTOR *)&body->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[W] ;
    work->foot->target.size.vx = 200 ;
    work->foot->act = BRK_OBJ_ActOnFoot ;
    work->foot++ ;
    work->n_obj += 2 ;
}



/* ハザードにそわして置く */
int BRK_OBJ_PutOnHazard( Work *work, BRK_OBJ *obj, int i )
{
    FVECTOR  pos, pos_v ;
    FMATRIX *mat = &obj->objs->objs[i].world ;
    float    width = (i&1 ? 400.0f : 1200.0f) ;
    FVECTOR  size = { 100.0f, 80.0f, 100.0f } ;
    FVECTOR  cen ;
	int      flag ;
	
    RotateMatrixXY( mat, &DG_UnitMatrix,
				   irnd() & (work->flag & BRK_F_STABLE_MSK ? 0 : 4095),
				   irnd() & 4095 ) ;
	
    if ( work->flag & BRK_F_STRICT_FLR ) {
		BRK_OBJ_StickPartsOnFloor( &obj->objs->objs[i], &size, &cen ) ;
		_sceVu0AddVector( &pos, &obj->pos, &cen ) ;
	} else {
		_sceVu0CopyVector( &pos, &obj->pos ) ;
	}
    pos.vx += frnd() * width ;
    pos.vz += frnd() * width ;
    pos_v.vz = pos_v.vx = 0.0f ;
    pos_v.vy = -2000.0f ;

    flag = BRK_CheckHazard( (HZD_BOX *)work->hzd,
						    &pos, &pos_v,
						    &BRK_HZD_NoBounce, &size ) ;
    if ( !(BRK_HZD_Flag & BRK_HZD_THROUGH) && (flag & 1) ) {
		if ( !BRK_UTL_PutCenterHazard( (HZD_BOX *)work->hzd, &pos, NULL, 3000.0f, 0.0f ) ) {
			if ( work->flag & BRK_F_STRICT_FLR )
			  _sceVu0SubVector( &pos, &pos, &cen ) ;
			TransMatrix( mat, &pos ) ;
			return 1 ;
		}
	}
    return 0 ;
}

void BRK_OBJ_AlreadyMessy( Work *work )
{
    DG_DEF  *def = work->part_def ;
    int      i, j ;
    BRK_OBJ *obj ;
    static FVECTOR DustSize = { 5.0f, 5.0f, 5.0f } ;

    work->n_obj   &= ~BRK_OBJ_INACTIVE ;
    work->n_piece &= ~BRK_OBJ_INACTIVE ;
    for ( j=work->n_obj, obj=work->obj ; --j>=0 ; obj++ )
    {
	/* 取り敢えずメモリを解放する */
	BRK_OBJ_FreeObj( obj ) ;

	if ( work->n_obj > 8 )
	    if ( j&1 )
		continue ;

	/* 破片のメモリーの確保とモデルの初期化 */
	if ( BRK_OBJ_InitObject( obj, def, (DG_FLAG_SHADE| DG_FLAG_FINISHCALC), NULL ) >= 0 )
	{
	    obj->n_parts = 0 ;/* これでメモリを確保していないことになる(実際していないのでOK) */
	    for ( i=def->n_models ; --i>=0 ; )
		if ( !BRK_OBJ_PutOnHazard( work, obj, i ) )
		    obj->objs->objs[i].flag |= DG_FLAG_INVISIBLE ;

	    if ( work->dust_mode == 3 )/* ケチャップ*/
		NewKetchapSpread( work->hzd, &obj->pos, NULL, 0x70000a3a,
				  2.0f + 0.5f*(int)(obj - work->obj) ) ;
	    else if ( work->dust_mode == 4 )/* ソース */
		NewKetchapSpread( work->hzd, &obj->pos, NULL, 0x70000810,
				  1.0f + 0.5f*(int)(obj - work->obj) ) ;

	    /* ゴミの配置 */
	    if ( work->comdl[0] )
		for ( i=BRK_N_PIECE ; --i>=0 ; )
		{
		    DG_COMDL_POS *p ;

		    p = work->piece[work->n_piece].comdl ;
		    if ( BRK_UTL_PutOnHazard( &p->world,
					      &obj->pos,
					      800.0f,
					      (work->flag & BRK_F_STABLE_MSK ? 0 : 4095),
					       4095,
					      work->hzd, &DustSize ) )
		    {
			p->color.vw = 128 ;
			if ( ++work->n_piece >= BRK_N_PIECE )
			    work->n_piece = 0 ;
		    }
	    }
	}

	/* ライトの反映 */
	BRK_UTL_GetLightMatrix( &obj->pos, work->light.mtx, 0.9f, work->where ) ;
    }
    work->n_obj   |= BRK_OBJ_INACTIVE ;
    work->n_piece |= BRK_OBJ_INACTIVE ;
}

void BRK_OBJ_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;
    extern OBJECT *GM_PlayerBody ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case BRK_OBJ_M_REGIST_FOOT:
	    work->n_obj &= ~BRK_OBJ_INACTIVE ;
	    if ( work->n_foot )
		BRK_OBJ_SetFoot( work, msg->message[1] ? (OBJECT*)msg->message[1] : GM_PlayerBody ) ;
	    break ;

	case BRK_OBJ_M_GET_OBJ_POS:
	    *((FVECTOR **)msg->message[1]) = &work->obj[msg->message[2]].pos ;
	    break ;

	case BRK_OBJ_M_GET_OBJ_NUM:
	    *((int **)msg->message[1]) = &work->n_obj ;
	    break ;

	case BRK_OBJ_M_ALREADY_MESSY:
	    BRK_OBJ_AlreadyMessy( work ) ;
	    break ;
	}
}

