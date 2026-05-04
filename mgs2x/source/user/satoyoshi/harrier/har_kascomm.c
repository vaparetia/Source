//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	har_kasacka.c								*/
/*	カサッカ								*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: */
/********************************************************************************/
#include "harrier.h"

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewKac_GetCommandIsSnakeAngry()					*/
/*	引数:	Work	*work							*/
/*	説明:	スネーク怒っている？						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewKac_GetCommandIsSnakeAngry()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    Kas_Work *work;
    if ( GET_KAK_WORK == NULL ){
        return -1;
    }
    work = GET_KAK_WORK;

    if (work->is_bossrush == ON){
        return -1;
    }

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */

    if(work->snake_ikari!=0){
	GCL_SetVarRef( &ref, 0, 1 ) ;
    }
    else {
	GCL_SetVarRef( &ref, 0, 0 );
    }
    return 1;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewKac_CommandGetKasLife()					*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカのライフを返す						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewKac_CommandGetKasLife()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    Kas_Work *work;
    if ( GET_KAK_WORK == NULL ){
        return 0;
    }
    work = GET_KAK_WORK;

    if (work->is_bossrush == ON){
        return -1;
    }

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)100*work->gageset.value/work->gageset.max ) ;

    return 1;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewKac_CommandGetKasStatus()					*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカの状態を返す						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewKac_CommandGetKasStat()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    Kas_Work *work;
    if ( GET_KAK_WORK == NULL ){
        return 0;
    }
    work = GET_KAK_WORK;

    if (work->is_bossrush == ON){
        return -1;
    }

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */

    if (work->damage_act_time > 0){
	GCL_SetVarRef( &ref, 0, 1 );
    }
    else if (0){
	GCL_SetVarRef( &ref, 0, 2 );
    }
    else {
	GCL_SetVarRef( &ref, 0, 0 );
    }

    return 1;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandGetPosition()					*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカの座標を返す						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewKac_CommandGetPosition()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    Kas_Work *work;

    if ( GET_KAK_WORK == NULL ){
        return 0;
    }
    work = GET_KAK_WORK;

    if (work->is_bossrush == ON){
	FMATRIX	tmpmat;
	FVECTOR	shift = {600.0f, -1570.0f, 300.0f, 1.0f};
	SVECTOR	rot = {0, 1024, 0, 0};	

	DG_SetPos( &work->body.objs->objs[0].world  ) ;
	DG_MovePos( &shift );
	DG_RotatePos ( &rot );
	DG_GetPos( &tmpmat ) ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)tmpmat.m[3][0] ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)tmpmat.m[3][1] ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)tmpmat.m[3][2] ) ;

	return 1;
    }

    // *************  中心
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snake.objs->world.m[3][0] ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snake.objs->world.m[3][1] ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snake.objs->world.m[3][2] ) ;

    // *************  後
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snk_back.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snk_back.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snk_back.vz ) ;

    // *************  真上
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snk_ue.vx ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snk_ue.vy ) ;
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->snk_ue.vz ) ;

    return 1 ;
}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandGetRoutNum()					*/
/*	引数:	Work	*work							*/
/*	説明:	現在のルートの番号を返す					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewKac_CommandGetRoutNum()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    Kas_Work *work;

    if ( GET_KAK_WORK == NULL ){
        return 0;
    }
    work = GET_KAK_WORK;

    if (work->is_bossrush == ON){
        return -1;
    }

    // *************  ルート番号
    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)work->rout_time ) ;

    return 1;
}



