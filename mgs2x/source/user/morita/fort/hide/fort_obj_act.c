//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_obj_act.c 
   フォーチュン戦 ライデン隠れ専用プットオブジェ

   2000/12/14 T.Morita
   $Id: fort_obj_act.c,v 1.1.1.3 2002/11/19 11:46:15 Yoshizawa1 Exp $
*/
#include "fort_obj.h"


void FRT_OBJ_SendFortuneHideMessage( Work *work, int status )
{
    GV_MSG msg ;/*ターゲットのポインタを得るメッセージ*/
    int buffer[2] ;

    msg.address = GV_StrCode( "フォーチュン" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    if ( !work->status && status )
    {
	buffer[0] = 1 ;
	buffer[1] = 0 ;
	printf( "Send IN stat%x new%x\n", work->status, status ) ;
	GV_SendMessage( &msg ) ;
    }
    else if ( work->status && !status )
    {
	buffer[0] = 0 ;
	buffer[1] = 0 ;
	printf( "Send OUT stat%x new%x\n", work->status, status ) ;
	GV_SendMessage( &msg ) ;
    }
}

static int FRT_OBJ_CalcOutsideOfRay( DG_OBJS *o, FVECTOR *from, FVECTOR *to, int i, int axis )
{
    FVECTOR s,t,u, v ;

    v.vx = i&1 ? o->def->ux : o->def->lx ;
    v.vy = i&2 ? o->def->uy : o->def->ly ;
    v.vz = i&4 ? o->def->uz : o->def->lz ;
    v.vw = 1.0f ;
    _sceVu0ApplyMatrix( &v, &o->world, &v ) ;
    _sceVu0SubVector( &s, from, to ) ;
    _sceVu0SubVector( &t, to  , &v ) ;
    _sceVu0SubVector( &u, from, &v ) ;

    /* XYZ何れかの軸が オルガとスネークの間にあれば,交差計算になる */
    if ( (s.vx < 0.0f ? u.vx<0.0f && t.vx>=0.0f : t.vx<0.0f && u.vx>=0.0f) ||
	 (s.vy < 0.0f ? u.vy<0.0f && t.vy>=0.0f : t.vy<0.0f && u.vy>=0.0f) ||
	 (s.vz < 0.0f ? u.vz<0.0f && t.vz>=0.0f : t.vz<0.0f && u.vz>=0.0f) )
	return axis ? (( s.vz * t.vy > s.vy * t.vz ) ? 1<<i : 0) :
		      (( s.vx * t.vz > s.vz * t.vx ) ? 1<<i : 0) ;
    else
	return 0xff ;
}

static inline int FRT_OBJ_CanISeeYou( DG_OBJS *o, FVECTOR *from, FVECTOR *to )
{
    int i, flag ;

    for ( flag=0, i=8 ; --i>=0 ; )
	flag |= FRT_OBJ_CalcOutsideOfRay( o, from, to, i, X ) ;
    if ( flag != 0 && flag != 0x00ff )
    {
	for ( flag=0, i=8 ; --i>=0 ; )
	    flag |= FRT_OBJ_CalcOutsideOfRay( o, from, to, i, Y ) ;
	if ( flag != 0 && flag != 0x00ff )
	    return 1 ;
    }

    return 0 ;
}

int FRT_OBJ_ActCheckHideFromTo( Work *work, FVECTOR *from, FVECTOR *to, HIDE *ignore )
{
    int       i ;
    int       status = 0  ;
    HIDE     *h = work->hide ;

    for ( i=FRT_MAX_SNAHIDE ; --i>=0 ; h++ )
    {
	if ( h != ignore && h->objs && h->hzd[0] )
	    if ( FRT_OBJ_CanISeeYou( h->objs, from, to ) )
		status |= 1 << (i & 0x1f) ;
    }
    return status ;
}

int FRT_OBJ_ActCheckHide( Work *work )
{
    extern FVECTOR *FRT_MiscPlayerPosNoHide( int joint ) ;
    extern FVECTOR *FRT_MiscFortuneJointPos( int joint ) ;
    FVECTOR  *raiden, *fortune ;

    raiden  = FRT_MiscPlayerPosNoHide( HUMAN21_KUBI  ) ;
    fortune = FRT_MiscFortuneJointPos( HUMAN21_ATAMA ) ;

    return FRT_OBJ_ActCheckHideFromTo( work, raiden, fortune, NULL ) ;
}


int FRT_OBJ_ActCheckHideFromToByID( u_int id, FVECTOR *from, FVECTOR *to )
{
    ASSERT( FRT_OBJ_Work ) ;

    return FRT_OBJ_ActCheckHideFromTo( FRT_OBJ_Work, from, to, (HIDE *)id ) ;
}
