//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_spotlgt.c 
   プットライトオブジェ

   1999/12/26 T.Morita
   $Id: brk_spotlgt.c,v 1.1.1.3 2002/11/19 11:45:46 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "brk_spotlgt.h"


#if 0
static void BRK_SPLGT_PreshadeMessage( int name )
{
    GV_MSG msg ;
    int buffer = 4 ;/*プットオブジェのプリシェードさせるメッセージID*/

    if ( name )
    {
	msg.address = name    ;
	msg.message = &buffer ;
	msg.message_len = 1   ;
	GV_SendMessage( &msg ) ;
    }
}
#endif


/* ワークリストを作る */
static void BRK_SPLGT_AddWorkList( Work *work )
{
    // BP_WARNING: Note that in this function, the list can get one element too big.  Rather than fix this, I made the
    // list one element larger internally but kept the MAX macro the same.  This should replicated the results of the
    // original without a memory overwrite.
    if ( BRK_SPTLGT_n_List> MAX_N_LIST )
	   return ;
    BRK_SPTLGT_List[BRK_SPTLGT_n_List++] = work ;
}

/* ワークリストから外す */
static void BRK_SPLGT_DelWorkList( Work *work )
{
    int i ;

    for ( i=0 ; i<BRK_SPTLGT_n_List ; i++ )
	if ( BRK_SPTLGT_List[i] == work )
	    for ( BRK_SPTLGT_n_List-- ; i<BRK_SPTLGT_n_List ; i++ )
		BRK_SPTLGT_List[i] = BRK_SPTLGT_List[i+1] ;
}



/* メッセージ関係 */
static void BRK_SPLGT_OffSpotLightMessage( int name )
{
    GV_MSG msg ;
    int buffer = 0 ;/*静止スポットライトを消すメッセージID*/

   if ( name )
    {
	msg.address = name    ;
	msg.message = &buffer ;
	msg.message_len = 1   ;
	GV_SendMessage( &msg ) ;
    }
}

static void BRK_SPLGT_OnSpotLightMessage( int name )
{
    GV_MSG msg ;
    int buffer = 1 ;/*静止スポットライトを付けるメッセージID*/

    if ( name )
    {
	msg.address = name    ;
	msg.message = &buffer ;
	msg.message_len = 1   ;
	GV_SendMessage( &msg ) ;
    }
}





/* モデル生成 */
static int ChangeModel( Work *work, DG_DEF *model, FMATRIX *mtx )
{
    DG_OBJS *objs ;

    if ( (objs = DG_MakeObjs( model, 
			      DG_FLAG_ONEPIECE| DG_FLAG_PAINT| DG_FLAG_AUTOREPAINT, 0 )) )
    {
	if ( mtx ) {
	    _sceVu0CopyMatrix( &objs->world, mtx ) ;
	}
	else if ( work->objs )
	{
	    _sceVu0CopyMatrix( &objs->world, &work->objs->world ) ;
	    objs->flag  = work->objs->flag  ;/* 天井君によって変られるかも知れないから */
	    DG_FreePreshade( work->objs ) ;
	    DG_DequeueObjs( work->objs ) ;
	    DG_FreeObjs( work->objs ) ;
	}
	work->objs = objs ;
	DG_QueueObjs( work->objs ) ;
	DG_MakePreshade( work->objs, work->lit ) ;
	GM_GroupObjs( work->objs, work->where ) ;
	return 0 ;
    }
    return -1 ;
}


static int TurnOffLight( Work *work, DG_DEF *def, FMATRIX *mtx )
{
    if ( work->flag & BRK_SPT_F_ALREADYBRK )
	return -1 ;
    if ( work->flag & BRK_SPT_F_ALREADYOFF )
	return -1 ;
    if ( !ChangeModel( work, def, mtx ) )
    {
	BRK_SPLGT_OffSpotLightMessage( work->lt_name ) ;

	/* ボリューミックシャドウからの削除 */
	if ( work->flag & BRK_SPT_F_VOL_SHADOW )
	    OK_RemoveDynamicLight( &work->trgt )  ;

	/* 光源を消す */
	DG_SwitchLightSphere( &work->trgt, work->radius, 0 ) ;

	work->flag |= BRK_SPT_F_ALREADYOFF ;

	if ( work->fade_tex )
	    GV_DestroyOtherActor( work->fade_tex ), work->fade_tex = NULL ;
	return 0 ;
    }
    return -1 ;
}


static int TurnOnLight( Work *work, DG_DEF *def, FMATRIX *mtx )
{
    SVECTOR rot = { 0, 0, 0, 0 } ;

    if ( work->flag & BRK_SPT_F_ALREADYBRK )
	return -1 ;
    if ( !(work->flag & BRK_SPT_F_ALREADYOFF) )
	return -1 ;

    if ( !ChangeModel( work, def, mtx ) )
    {
	BRK_SPLGT_OnSpotLightMessage( work->lt_name ) ;

	if ( work->flag & BRK_SPT_F_VOL_SHADOW ) /* ボリューミックシャドウへの登録 */
	    OK_SetDynamicLight( (FVECTOR*)&work->trgt, &rot ) ;

	/* 光源をつける */
	DG_SwitchLightSphere( &work->trgt, work->radius, 1 ) ;

	if ( work->flag & BRK_SPT_PUTW00B ) /*ボンボリキャラ*/
	{
	    OK_LightPos_PutCameraDrop( &work->trgt ) ;
	    work->fade_tex = NewPutTexFadeOnLight( work->name, work->where,
						   GV_StrCode( "xlit04b_alp" ),
						   GV_StrCode( "xlit04a_alp" ),
						   GV_StrCode( "xlit05b_alp" ),
						   SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ),
						   400, 400, 100,
						   &work->trgt, 8000, 0 ) ;
	}
	else if ( work->flag & BRK_SPT_PUTW03A ) /*ボンボリキャラ*/
	    work->fade_tex = NewPutTexFadeOnLight( work->name, work->where,
						   GV_StrCode( "xlit04b_alp" ),
						   GV_StrCode( "xlit04a_alp" ),
						   GV_StrCode( "xlit05b_alp" ),
						   SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ),
						   400, 400, 100,
						   &work->trgt, 8000, DG_PRIM2_FOG ) ;
	else
	    work->fade_tex = NULL ;

	work->flag &= ~BRK_SPT_F_ALREADYOFF ;

	return 0 ;
    }
    return -1 ;
}

static void BRK_SPLGT_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work      *work = (Work*)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;
	if( (&work->trg_bllt == def && (off->weapon_type & (WP_BULLET|WP_M92)) ) ||
	    (&work->trg_bomb == def && (off->weapon_type & WP_BLAST))  )
	{
	    /* スルーなのでこっちで止める */
	    BRK_UTL_CallOffenceWhenThrough( off, def ) ;

	    work->flag &= ~BRK_SPT_F_ALREADYOFF ;
	    if ( !TurnOffLight( work, work->broken, NULL ) )
	    {
		if ( !(work->flag & BRK_SPT_F_NO_EFFECT) )
		{
		    GM_SeSetMode( SD_A_LAMP02, &work->trgt, GM_SEMODE_BOMB ) ;
		    NewBreakLight( &work->trgt, &work->trgt, work->where ) ;
		}
		if ( work->pr_name )
		    GCL_ExecProc( work->pr_name, &work->pr_arg ) ;

		GM_SetNoise( NOISE_S, &def->hit, work->where ) ;

		work->flag |= BRK_SPT_F_ALREADYBRK ;
		GM_FreeTarget( &work->trg_bllt ) ;
		if ( work->flag & BRK_SPT_F_PUT_C4 )
		    GM_FreeTarget( &work->trg_bomb ) ;
	    }
	}
    }
}

static inline int BRK_SPLGT_InitTarget( Work *work,
					TARGET *t, POWER_TARGET *p,
					int flag, int where,
					FVECTOR *size,
					FMATRIX *m ) 
{
    FVECTOR t_size = { size->vx, size->vy, size->vz, 0.0f } ;
    FVECTOR t_pos ;
    FVECTOR tv = { work->on->tx, work->on->ty, work->on->tz, 0.0f } ;
    FVECTOR uv = { work->on->ux, work->on->uy, work->on->uz, 0.0f } ;
    FVECTOR lv = { work->on->lx, work->on->ly, work->on->lz, 0.0f } ;

    if ( work->flag & BRK_SPT_F_OBJS_FITALL )
    {
	_sceVu0AddVector( &t_pos, &uv, &lv ) ;
	_sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
	_sceVu0ApplyMatrix( &t_pos, m, &t_pos ) ;
	_sceVu0AddVector( &t_pos, &t_pos, &tv ) ;
	_sceVu0AddVector( &work->trgt, &work->trgt, &t_pos ) ;
    }

    if ( !flag &&  !(work->flag & BRK_SPT_F_SET_TARGET) )
    {
	if ( work->flag & BRK_SPT_F_OBJS_TARGET )
	{
	    _sceVu0SubVector( &t_size, &uv, &lv ) ;
	    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
	}
	if ( work->flag & BRK_SPT_F_OBJS_OFFSET )
	{
	    _sceVu0AddVector( &work->trgt, &uv, &lv ) ;
	    _sceVu0ScaleVector( &work->trgt, &work->trgt, 0.5f ) ;
	    _sceVu0ApplyMatrix( &work->trgt, m, &work->trgt ) ;
	}
    }

    _sceVu0ApplyMatrix( &t_size, m, &t_size ) ;
    t_size.vx = fpu_Abs( t_size.vx ) ;
    t_size.vy = fpu_Abs( t_size.vy ) ;
    t_size.vz = fpu_Abs( t_size.vz ) ;
    
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|flag,
		  where, BOTH_SIDE,
		  &t_size, &work->trgt ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_SPLGT_TargetCallBack, work ) ;

#ifdef DEBUG_MODE
    if ( work->flag & BRK_SPT_F_TARGETSHOW )
	NewTargetView( t,  200, 50, 32 ) ;
#endif

    return 0 ;
}







/* メッセージ処理 */
static void BRK_SPTLGT_Message( Work *work, int message[] )
{
    switch ( message[0] )
    {
    case BRK_SPT_M_BREAKDOWN:
	TurnOffLight( work, work->broken, NULL ) ;

	work->flag |= BRK_SPT_F_ALREADYBRK ;
	GM_FreeTarget( &work->trg_bllt ) ;
	if ( work->flag & BRK_SPT_F_PUT_C4 )
	    GM_FreeTarget( &work->trg_bomb ) ;
	break ;
    case BRK_SPT_M_TURNON:
    case BRK_SPT_M_FIXUP:
	TurnOnLight( work, work->on, NULL ) ;
	break ;
    case BRK_SPT_M_TURNOFF:
	TurnOffLight( work, work->off ? work->off : work->broken, NULL ) ;
	break ;

    case BRK_SPT_M_INVISBLE:
	DG_InvisibleObjs( work->objs ) ;
	break ;
    case BRK_SPT_M_VISIBLE:
	DG_VisibleObjs( work->objs ) ;
	break ;

    case BRK_SPT_M_CHANGFLAG:
	work->objs->flag &= ~DG_FLAG_INVISIBLE ;
	work->objs->flag |=  message[1] ;
	break ;
    }
}








static void Die( Work *work )
{
    if ( work->objs )
    {
	DG_FreePreshade( work->objs ) ;
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }
    if ( work->fade_tex )
	GV_DestroyOtherActor( work->fade_tex ) ;
    if ( !(work->flag & BRK_SPT_F_ALREADYOFF) )
    {
	if ( work->flag & BRK_SPT_F_VOL_SHADOW ) /* ボリューミックシャドウからの削除 */
	    OK_RemoveDynamicLight( &work->trgt )  ;
	GM_FreeTarget( &work->trg_bllt ) ;
	if ( work->flag & BRK_SPT_F_PUT_C4 )
	    GM_FreeTarget( &work->trg_bomb ) ;
    }

    /* ワークリストから削除 */
    BRK_SPLGT_DelWorkList( work ) ;
}

static void Act( Work *work )
{
    GV_MSG  *msg  ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	BRK_SPTLGT_Message( work, msg->message ) ;
}

static int GetResources( Work *work, int name, int where )
{
    int     i, buf[3] ;
    FMATRIX mtx = DG_UnitMatrix ;
    float   value ;
    FVECTOR size ;

    work->name    = name  ;
    work->where   = where ;
    work->flag    = GCL_GetOptionValue( 'f', 0 ) | BRK_SPT_F_ALREADYOFF ;
    work->lt_name = GCL_GetOptionValue( 'n', 0 ) ;
    work->pr_name = GCL_GetOptionValue( 'b', 0 ) ;
    work->pr_arg.argc = 1 ;
    work->pr_arg.argv = &work->name ;

    if ( (i=GCL_GetOptionValue( 'l', 0 )) )
	work->lit = GV_GetCache( GV_CacheID( i, 'l') ) ;
    else
	PERROR( "No world light name(-l option missing) :: NewPutSpotLightObject\n" ) ;

    if ( GCL_GetOption( 'm' ) )
    {
	work->on     = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k') ) ;
	work->broken = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k') ) ;
	if ( GCL_NextStr() )
	    work->off = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k') ) ;
    }
    else
	PERROR( "No model name(-m option missing) :: NewPutSpotLightObject\n" ) ;

    if ( GCL_GetOption( 'r' ) != NULL )
    {
	int r ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixX( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixY( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixZ( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    }

    if ( GCL_GetOption( 'p' ) )
    {
	FVECTOR tv = { work->on->tx, work->on->ty, work->on->tz, 0.0f } ;
	
	if ( work->flag & BRK_SPT_F_OBJS_FITALL )
	    _sceVu0ApplyMatrix( &tv, &mtx, &tv ) ;

	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)mtx.m[3] ) ;

	if ( work->flag & BRK_SPT_F_OBJS_FITALL )
	    _sceVu0AddVector( (FVECTOR*)mtx.m[3], (FVECTOR*)mtx.m[3], &tv ) ;

	mtx.m[3][3] = 1.0f ;
    }    

    if ( GCL_GetOption( 's' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)&size ) ;
	work->flag |= BRK_SPT_F_SET_TARGET ;
    }

    if ( GCL_GetOption( 't' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &work->trgt ) ;
	if ( GCL_NextStr() )
	{
	    value = (float)GCL_GetNextInt() ;
	    if ( !(work->flag & BRK_SPT_F_SET_TARGET) )
	        size.vx = size.vy = size.vz = value ;
	    BRK_SPLGT_InitTarget( work, &work->trg_bllt, &work->pow_bllt,
				  TARGET_THROUGH, where, &size, &mtx ) ;
	}

	if ( GCL_NextStr() )
	  work->radius = (float)GCL_GetNextInt() ;
	else
	  work->radius = 1000.0f ;

	if ( GCL_NextStr() )
	{
	    value = (float)GCL_GetNextInt() ;
	    size.vx = size.vy = size.vz = value ;
	    BRK_SPLGT_InitTarget( work, &work->trg_bomb, &work->pow_bomb,
				  TARGET_THROUGH, where, &size, &mtx ) ;
	    work->flag |= BRK_SPT_F_PUT_C4 ;
	}
    }
    else
	PERROR( "No target size(-t option missing) :: NewPutSpotLightObject\n" ) ;

    TurnOnLight( work, work->on, &mtx ) ;
    GM_PutTarget( &work->trg_bllt ) ;
    if ( work->flag & BRK_SPT_F_PUT_C4 )
	GM_PutTarget( &work->trg_bomb ) ;

    /* ワークリストに追加 */
    BRK_SPLGT_AddWorkList( work ) ;

    return 0 ;
}


void *NewPutSpotLightObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}


static int BRK_SPLGT_CommandMessage( int name, int message[] )
{
    int   i ;
    int   flag ;

    if ( name )
    {
	flag = 0 ;
	if ( name & 0xff000000 )
	{
	    for ( i=0 ; i<BRK_SPTLGT_n_List ; i++ )
		if ( BRK_SPTLGT_List[i]->name == name )
		    BRK_SPTLGT_Message( BRK_SPTLGT_List[i], message ), flag=1 ;
	}
	else
	{
	    for ( i=0 ; i<BRK_SPTLGT_n_List ; i++ )
		if ( (BRK_SPTLGT_List[i]->name & 0x00ffffff) == name )
		    BRK_SPTLGT_Message( BRK_SPTLGT_List[i], message ),flag=1 ;
	}
#if DEBUG_MODE
	if ( !flag )
	    printf( "Work<%d> not found : NewPutSpotLightObject\n", name ) ;
#endif
	return 1 ;
    }
    return 0 ;
}

int NewCom_BRK_SPLGT_LightMessage()
{
    int   name ;
    int   message[2] ;

    name = GCL_GetOptionValue( 'n', 0 ) ;
    message[0] = BRK_SPT_M_CHANGFLAG ;
    message[1] = GCL_GetOptionValue( 'f', 0 ) ;
    return BRK_SPLGT_CommandMessage( name, message ) ;
}

int NewCom_BRK_SPLGT_BreakMessage()
{
    int   name ;
    int   message[2] ;

    name = GCL_GetOptionValue( 'n', 0 ) ;
    message[0] = BRK_SPT_M_BREAKDOWN ;
    message[1] = GCL_GetOptionValue( 'f', 0 ) ;
    return BRK_SPLGT_CommandMessage( name, message ) ;
}



