//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_obj_clb.c 
   フォーチュン戦 ライデン隠れ専用プットオブジェ

   2000/12/14 T.Morita
   $Id: fort_obj_ini.c,v 1.1.1.3 2002/11/19 11:46:15 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

void FRT_OBJ_InitShadow( HIDE *h )
{
    char *shadow  ;

    shadow = FRT_OBJ_GetTypeShadow( h ) ;
    if ( shadow )
	h->shadow = NewFortDropShadow( h->objs, -45000.0f, shadow,
				       (h->type & FRT_TYP_FLAG_SHADOW)>>8 ) ;
}

void FRT_OBJ_InitTarget( HIDE *h, FVECTOR *t_size, FVECTOR *t_pos )
{
    GM_SetTarget( &h->target,
		  TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE,
		  h->work->where,
		  BOTH_SIDE, t_size, t_pos ) ;
    GM_MoveTarget2( &h->target, &h->objs->world ) ;
    GM_SetPowerTarget( &h->target, &h->power, POWER_DECREASE,
		       GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &h->target, FRT_OBJ_TargetCallBack, h ) ;
}

int FRT_OBJ_InitTargetHzxHzd( HIDE *h )
{
    int      where = h->work->where ;
    DG_OBJS *o = h->objs ;
    FVECTOR  t_size, t_pos ;
    FVECTOR *hzd_geo ;
    int      i ;

    /* 当たり用モデルをタイプから捜す */
    if ( (hzd_geo = FRT_OBJ_GetTypeHazard( h )) )
    {
	_sceVu0CopyVector( &t_size, hzd_geo++ ) ;
	_sceVu0CopyVector( &t_pos , hzd_geo++ ) ;
    }
    /* モデルのサイズと中心をとる（ローカル大きさと位置） */
    else
	FRT_OBJ_MakeHzxSizeAndCenter( o->def, &t_size, &t_pos ) ;

    /*ターゲットを貼る*/
    FRT_OBJ_InitTarget( h, &t_size, &t_pos ) ;

    /*HZXを貼る*/
    if ( FRT_OBJ_MakeHzxFromOBJS( h, o, where, &t_size, &t_pos ) )
	PERROR( "Cannot Make HZX Segment :NewFortHideObject\n" ) ;

    /*HZDを貼る*/
    if ( FRT_OBJ_GetTypeAimFlag(h) )
    {
	ASSERT( !h->hzd[0] ) ;
	if ( hzd_geo )
	{
	    for ( i=0 ; i<3 ; i++ )
	    {
		if ( hzd_geo->vw == -1.0f )
		    break ;
		_sceVu0ApplyMatrix( &t_pos, &o->world, hzd_geo+1 ) ;
		if ( !(h->hzd[i] = BRK_MakeHazard( BRK_HZD_OUTSIDE|
						   BRK_HZD_ROTATE, NULL,
						   &o->world,
						   hzd_geo, &t_pos,
						   NULL )) )
		    PERROR( "Too many OBJ!! :NewFortHideObject\n" ) ;
		hzd_geo += 2 ;
	    }
	}
	else
	{
	    _sceVu0ApplyMatrix( &t_pos, &o->world, &t_pos ) ;
	    if ( !(h->hzd[0] = BRK_MakeHazard( BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,
					       NULL, &o->world,
					       &t_size, &t_pos, NULL )) )
		PERROR( "Too many OBJ!! :NewFortHideObject\n" ) ;
	}

	/*狙いを貼る*/
	if ( FRT_AIM_AddAimSpot( &h->target,
				 (h->move ? &(h+h->move)->target : NULL),
				 h->phase,
				 (FVECTOR *)o->world.m[W],
				 FRT_OBJ_GetTypeFront(h),
				 FRT_OBJ_GetTypeRear(h),
				 FRT_OBJ_GetTypeAimFlag(h) ) < 0 )
	    PERROR( "Cannot Add aimming spot :NewFortHideObject\n" ) ;
    }

    return  0 ;
}

int FRT_OBJ_InitModel( HIDE *h, FMATRIX *mtx )
{
    int id ;
    DG_DEF  *def ;
    DG_OBJS *objs ;

    /* モデルをタイプから捜す */
    id = GV_StrCode( FRT_OBJ_GetTypeModel( h ) ) ;
    if ( !(def = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No KMS<%s> in data.cnf!! : NewFortHideObject\n",
		FRT_OBJ_GetTypeModel( h ) ) ;

    /* モデルを作る */
    if ( !(objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cant make DG_OBJS(Maybe no memory!!) :NewFortHideObject\n" ) ;
    DG_QueueObjs( objs ) ;
    if ( mtx )
    {
		if ( h->type & FRT_TYP_FLAG_SYMETRY ) {
			_sceVu0CopyMatrix( &objs->world, mtx ) ;
		} else {
			_sceVu0CopyMatrix( &objs->world, mtx ) ;
		}
    }
    DG_MakePreshade( objs, h->work->lit_def ) ;

    /* モデルがちゃんとできたら解放する */
    FRT_OBJ_FreeObject( h, 0 ) ;
    h->objs = objs ;

    /* 当たり関係を貼り直す */
    if ( !(h->work->flag & 6) )
	FRT_OBJ_InitTargetHzxHzd( h ) ;

    /* 影を貼る */
    FRT_OBJ_InitShadow( h ) ;

#if MAKING
    {
	extern void *NewDrawWireframe( DG_OBJ *packs ) ;
	h->wireframe = NewDrawWireframe( objs->objs ) ;
    }
#endif

    return 0 ;
}

int FRT_OBJ_InitHazard( Work *work )
{
    if ( !(work->hzd = BRK_InitHazard( BRK_HZD_W11B_FORTUNE )) )
	return -1 ;
    return 0 ;
}


int FRT_OBJ_GetOptions( Work *work, int name, int where )
{
    int       i, j, buf[3] ;
    int       n_hide ;
    FMATRIX   mtx ;
    HIDE     *h = work->hide ;

    if ( !(i = GCL_GetOptionValue( 'l', 0 )) )
	PERROR( "No -light option in GCL : NewFortHideObject\n" ) ;
    if ( !(work->lit_def = (LIT_DEF *)GV_GetCache( GV_CacheID( i, 'l' )) ) )
	PERROR( "No LT2-data in data.cnf!! : NewFortHideObject\n" ) ;

    work->flag = GCL_GetOptionValue( 'f', 0 ) ;

    n_hide = 0 ;
    while( (i = GCL_GetNextOption()) )
	switch( i )
	{
	case 't':
	    /* フラグに２が入った場合,とっておいた情報を使う */
	    h->type   = GCL_GetNextInt() ;
	    if ( work->flag & 2 )
		h->type = FRT_OBJ_TypePool[n_hide] ;

	    h->work   = work ;
	    h->shadow = NULL ;
	    h->phase  = 0    ;
	    h->move   = 0    ;
	    h->objs   = NULL ;
	    h->segs[0] = h->segs[1] = h->segs[2] = h->segs[3] = NULL ;
	    h->flrs = NULL ;
	    h->hzd[0]  = h->hzd[1]  = h->hzd[2]  = NULL ;

printf( "[%d] type%d ", n_hide, FRT_OBJ_GetTypeMasked(h) ) ;

	    if ( ++n_hide > FRT_MAX_SNAHIDE )
		PERROR( "Too many objects < %d : NewFortHideObject\n",
			FRT_MAX_SNAHIDE ) ;

	    break ;

	case 'C':
	    for ( j=0 ; j<FRT_OBJ_N_PROCS; j++ )
		if ( GCL_NextStr() )
		    h->procs[j] = GCL_GetNextInt() ;
	    break ;

	case 'A':
	    h->phase = GCL_GetNextInt() ;
	    break ;

	case 'm':
	    h->move  = GCL_GetNextInt() ;
	    break ;

	case 's':
	    if ( !h->type )
		PERROR( "'-type' must come before '-rot' : NewFortHideObject\n" ) ;
	    GCL_GetIV( GCL_NextStr(), buf ) ;
            vu0_IV0toFV( (IVECTOR *)buf, &h->slide ) ;
	    break ;

	case 'r':
	    if ( !h->type )
		PERROR( "'-type' must come before '-rot' : NewFortHideObject\n" ) ;
	    h->rot.vx = 0 ;
	    h->rot.vy = GCL_GetNextInt() ;
	    h->rot.vz = 0 ;
	    i = h->rot.vx & 0x0fff ;
	    _sceVu0RotMatrixX( &mtx, &DG_UnitMatrix, (i-4096*(i>>11))*(float)M_PI/2048.0F ) ;
	    i = h->rot.vy & 0x0fff ;
	    _sceVu0RotMatrixY( &mtx, &mtx, (i-4096*(i>>11))*(float)M_PI/2048.0F ) ;
	    i = h->rot.vz & 0x0fff ;
	    _sceVu0RotMatrixZ( &mtx, &mtx, (i-4096*(i>>11))*(float)M_PI/2048.0F ) ;
	    break ;

	case 'p':
	    if ( !h->type )
		PERROR( "'-type' must come before '-pos' : NewFortHideObject\n" ) ;

	    GCL_GetIV( GCL_NextStr(), buf ) ;
            vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)mtx.m[W] ) ;
	    mtx.m[W][W] = 1.0f ;

printf( "(%.0f %.0f %.0f)\n", mtx.m[W][X],mtx.m[W][Y],mtx.m[W][Z] ) ;

	    if ( strlen( FRT_OBJ_GetTypeModel( h ) ) > 1 )
	    {
		/* モデルの初期化 */
		if ( FRT_OBJ_InitModel( h, &mtx ) < 0 )
		    return -1 ;
		/* システムにターゲットを登録する */
		if ( !(h->work->flag & 6) )
		    if ( FRT_OBJ_GetTypeFlag( h ) >= 0 )
		    {
			GM_PutTarget( &h->target ) ;
			//NewTargetView( &h->target, 200, 50, 32 ) ;
		    }
	    }
	    if ( work->flag & 2 )
		if ( BRK_FRT_OBJ_InitBreak[FRT_OBJ_GetTypeMasked(h)] )
		    (*BRK_FRT_OBJ_InitBreak[FRT_OBJ_GetTypeMasked(h)])( h, (FVECTOR *)mtx.m[W] ) ;

	    h++ ;
	    break ;
	}

    /* グローバルの初期化 */
    FRT_BulletEnable = 0 ; 

    return 0 ;
}

