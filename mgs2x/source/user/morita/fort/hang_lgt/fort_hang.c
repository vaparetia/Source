//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_hang.c
   フォーチュン戦専用 ライト揺れ

   1999/12/13 T. Morita
   $Id: fort_hang.c,v 1.1.1.3 2002/11/19 11:46:12 Yoshizawa1 Exp $
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



#define _MAIN_FILE_
#include "fort_hang.h"

extern int FRT_LGT_GetMagnitude( FVECTOR *pos, FVECTOR *vel, float *magnitude ) ;

static int   FRT_LGT_SeTime  = 0    ;
static float FRT_LGT_SeForce = 0.0f ;
static Work *FRT_LGT_SeWork  = NULL ;

static FVECTOR FRT_HNG_Offset[] = {
    { 0.0f, -150.0f,   495.0f, 1.0f },
    { 0.0f, -150.0f, -3005.0f, 1.0f },
    { 0.0f, -150.0f, -6505.0f, 1.0f },
    { 0.0f, -150.0f,-10005.0f, 1.0f }
} ;


#define MAX_N_LIST 32
static Work *FRT_HNG_List[MAX_N_LIST + 1 /* +1 due to AddWorkList below */ ] ;
static int   FRT_HNG_n_List ;


/* ワークリストを作る */
static void AddWorkList( Work *work )
{
   // BP_WARNING: Note that the below if check allows for an overflow of FRT_HNG_List by one element.
   // Rather than stopping that behavior, the list is just one element bigger.  This fixes a case where something may have
   // depended on the list being one element too large.

    if ( FRT_HNG_n_List > MAX_N_LIST )
	return ;
    FRT_HNG_List[FRT_HNG_n_List++] = work ;
}

/* ワークリストから外す */
static void DelWorkList( Work *work )
{
    int i ;

    for ( i=0 ; i<FRT_HNG_n_List ; i++ )
	if ( FRT_HNG_List[i] == work )
	    for ( FRT_HNG_n_List-- ; i<FRT_HNG_n_List ; i++ )
		FRT_HNG_List[i] = FRT_HNG_List[i+1] ;
}

/* 壊れフラグ  取得 */
int NewCom_FRT_HNG_GetBreakFlag()
{
    int   name ;
    int   i ;
    GCL_VAR_REF ref ; /* 配列への参照データ */

    name = GCL_GetOptionValue( 'n', 0 ) ;
    if ( name )
	if ( GCL_GetOption( 'f' ) )
	{
	    for ( i=0 ; i<FRT_HNG_n_List ; i++ )
		if ( FRT_HNG_List[i]->name == name )
		{
		    printf( "SetFlag <%x> : NewFortHang\n",
			    FRT_HNG_List[i]->flag ) ;
		    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
		    GCL_SetVarRef( &ref, 0, FRT_HNG_List[i]->flag ) ;
		    return 1 ;
		}
#if DEBUG_MODE
	    printf( "Work<%d> not found : NewFortHang\n", name ) ;
#endif
	}
    return 0 ;
}





/* ベクトルからマトリックスを計算する（ベクトルがZ軸を向くようなマトリックス） */
static void VectorToMatrix( FMATRIX *m, FVECTOR *v )
{
    float sht, phi ;

    _sceVu0Normalize( v, v ) ;
    phi = atan2f( v->vz, v->vx ) ;
    sht = atan2f( v->vy, sceVu0Sqrt( v->vx*v->vx + v->vz*v->vz ) ) ;
    _sceVu0RotMatrixY( m, &DG_UnitMatrix, phi ) ;
    _sceVu0RotMatrixZ( m, m, -sht ) ; 
}

/* 先祖と子孫（つまりライトの根元）を結ぶベクトルを得る */
static void GetTipOfPos( int i, DG_OBJ *o, FVECTOR *pos )
{
    *pos = DG_ZeroVector ;
    for ( ; i!=-1 ; i=o[i].parent )
    {
        _sceVu0ApplyMatrix( pos, &o[i].world, pos ) ;
        pos->vx += o[i].model->tx ;
        pos->vy += o[i].model->ty ;
        pos->vz += o[i].model->tz ;
    }
}

static void MakeObjWorld( Work *work )
{
    FMATRIX *mtx, base ;
    FVECTOR pos ;
    DG_OBJ  *o = work->objs->objs ;
    int      i ;

    /* まず全ての間接のマトリックスを計算する */
    for ( i=work->objs->n_models ; --i>=0 ; )
	RotateMatrixXYZ( &o[i].world, &DG_UnitMatrix, work->rot_xyz[i] ) ;

    /* 先祖と子孫（つまりライトの根元）を結ぶベクトルを得る */
    GetTipOfPos( work->objs->n_models-1, o, &pos ) ;

    /* そのベクトルから逆マトリックスを得る */
    VectorToMatrix( &base, &pos ) ;

    /* そのマトリックスを間接全てに掛けることで ライトの根元は動かないように見える */
    RotateTransMatrix( &work->objs->world, &base, &work->rot, &work->pos ) ;
    for ( i=0 ; i<work->objs->n_models ; i++ )
    {
	TransMatrix( mtx = &o[i].world, (FVECTOR *)&o[i].trans ) ;
	if ( o[i].parent == -1 )
	    _sceVu0MulMatrix( mtx, &work->objs->world   , mtx ) ;
	else
	    _sceVu0MulMatrix( mtx, &o[o[i].parent].world, mtx ) ;
    }
    GM_MoveTarget2( &work->target, &o[1].world ) ;
}




static void FRT_HNG_ShakeHanger( Work *work, FVECTOR *frc, float scale )
{
    work->rotv_xyz[0][Z] = (short)( frc->vx) ;
    work->rotv_xyz[1][Z] = (short)(-frc->vx) ;
    work->rotv_xyz[2][Z] = (short)( frc->vx/3) ;
    work->rotv_xyz[3][Z] = (short)( frc->vx*2/3) ;
    work->rotv_xyz[4][Z] = (short)( frc->vx) ;

    if ( !(int)(frc->vx/frc->vz*0.8f) )
    {
	_sceVu0ScaleVector( frc, frc, scale ) ;
	work->rotv_xyz[0][X] = (short)(-frc->vz) ;
	work->rotv_xyz[0][Y] = (short)(-frc->vx) ;
	work->rotv_xyz[1][X] = (short)( frc->vz) ;
	work->rotv_xyz[1][Y] = (short)( frc->vx) ;
	work->rotv_xyz[2][X] = (short)( frc->vz/3) ;
	work->rotv_xyz[2][Y] = (short)( frc->vx/3) ;
	work->rotv_xyz[3][X] = (short)( frc->vz/3) ;
	work->rotv_xyz[3][Y] = (short)(-frc->vx/3) ;
	work->rotv_xyz[4][X] = (short)( frc->vz) ;
	work->rotv_xyz[4][Y] = (short)(-frc->vx) ;
    }
    work->mode = FRT_MOD_MOVE ;
}

static void FRT_HNG_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    FMATRIX mat ;
    FVECTOR offs, frc ;
    Work *work = (Work *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( !(off->weapon_type & (WP_BULLET|WP_M92|WP_NIKITA)) )
	    return ;

	work->mode = FRT_MOD_MOVE ;

	/*ローカルの位置と力積のの向きを計算*/
        _sceVu0CopyVector( &offs, &def->hit ) ;
        offs.vw = 1.0f ;
	_sceVu0InversMatrix( &mat, &work->objs->world ) ;
        _sceVu0ApplyMatrix( &offs, &mat, &offs ) ;
        _sceVu0ApplyMatrix( &frc , &mat, &off->power->force ) ;
	/*力積の位置と方向から揺れる向きを計算*/
	_sceVu0ScaleVector( &frc, &frc, 0.01f*16.0f ) ;
	offs.vx -= 350.0f ;

	FRT_HNG_ShakeHanger( work, &frc, (offs.vx>0.0f ?-0.5f : 0.5f) ) ;
    }
}

static int FRT_HNG_InitTarget( Work *work, DG_OBJ *obj, int where )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;

    t_size.vx = (obj->model->ux - obj->model->lx) * 0.5f ;
    t_size.vy = (obj->model->uy - obj->model->ly) * 0.5f ;
    t_size.vz = (obj->model->uz - obj->model->lz) * 0.5f ;
    t_pos.vx  = (obj->model->ux + obj->model->lx) * 0.5f ;
    t_pos.vy  = (obj->model->uy + obj->model->ly) * 0.5f ;
    t_pos.vz  = (obj->model->uz + obj->model->lz) * 0.5f ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE|TARGET_SEEK, where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector) ;
    GM_SetTargetCallBack( t, FRT_HNG_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

static inline void FRT_HNG_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch( msg->message[0] )
        {
        case FRT_HNG_M_INVISBLE:
	    work->objs->flag &= ~DG_FLAG_INVISIBLE ;
	    work->objs->flag |=  msg->message[1] ;
            break ;
        case FRT_HNG_M_VISIBLE:
            break ;
        }
}


static void Die( Work *work )
{
#if MAKING
    int  i ;

    for ( i=work->objs->n_models ; --i>=0 ; )
	if ( work->wireframe[i] )
	    GV_DestroyOtherActor( work->wireframe[i] ) ;
#endif

    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
    DelWorkList( work )  ;
}


static void ActSetSound( int prv, int cur, int vol )
{
    int     se, pan ;
    float bp_angle;
    FVECTOR pos ;

    /* 回転速度の符号が反転したら音がなる */
    if ( (prv>0 && cur<=0) || (prv<=0 && cur>0) )
    {
	vol = vol>0 ? vol : -vol ;
	vol = (vol >> 3) & 0x1f ;
	if ( vol )
	{
	    DG_COPY_VEC( &pos, &GM_PlayerControl->mov ) ;
	    pos.vy = -41000.0f ;
	    pos.vx = pos.vx> 3500.0f ?  3500.0f :
		pos.vx<-3500.0f ? -3500.0f : pos.vx ;
	    pos.vz = pos.vz> 4000.0f ?  4000.0f :
		pos.vz<-7000.0f ? -7000.0f : pos.vz ;
	    se  = cur>0 ? SD_A_LANPSWI1 : SD_A_LANPSWI2 ;
	    pan = GM_SeGetPan( &pos, GM_SEMODE_NORMAL, &bp_angle ) ;
	    GM_SeSet3D( pan, vol, se, bp_angle ) ;
	}
    }
}

static void FRT_HNG_ActMagnitude( Work *work )
{
    int     r   ;
    FVECTOR frc ;
    float   mag ;

    /* 振動が起きて揺れる必要があるかどうかを調べる */
    mag = 0.0f ;
    r  = FRT_LGT_GetMagnitude( (FVECTOR *)work->objs->objs[1].world.m[W],
			       &frc, &mag )*1 ;
    r += FRT_LGT_GetMagnitude( (FVECTOR *)work->objs->objs[2].world.m[W],
			       &frc, &mag )*2 ;
    r += FRT_LGT_GetMagnitude( (FVECTOR *)work->objs->objs[3].world.m[W],
			       &frc, &mag )*4 ;
    r += FRT_LGT_GetMagnitude( (FVECTOR *)work->objs->objs[4].world.m[W],
			       &frc, &mag )*8 ;

    /* rが正ならば揺れるようにハンガーに設定する */
    if ( r )
    {
	_sceVu0ScaleVector( &frc, &frc, mag*mag*5.0f*16.0f ) ;
	FRT_HNG_ShakeHanger( work, &frc, -0.5f ) ;

	/* 一番強く揺れるところがサウンドのボリュームを決める */
	if ( FRT_LGT_SeForce < frc.vx )/* 一番強い所がサウンドのボリュームを決める */
	{
	    FRT_LGT_SeWork  = work    ;
	    FRT_LGT_SeForce = frc.vx ;
	    FRT_LGT_SeTime  = GV_Time ;
	}
    }
}

static void FRT_HNG_ActMove( Work *work )
{
    int prv, cur ;
    int i, j, k, r ;

    work->mode = FRT_MOD_FINISHED ;

    prv = work->rotv_xyz[0][Z] ;/* 前の回転を覚えておく */
    for ( i=FRT_MAX_MDL ; --i>=0 ; )
	for( k=XYZ ; --k>=0 ; )
	{
	    r = work->rotv_xyz[i][k] + work->rot_xyz[i][k] ;
	    for ( j=6 ; r/2 && --j>=0 ; )
		r /= 2 ;
	    work->rot_xyz[i][k] += work->rotv_xyz[i][k] -= r ;
	    work->mode |= work->rot_xyz[i][k] | work->rotv_xyz[i][k] ;
	}
    MakeObjWorld( work ) ;
    cur = work->rotv_xyz[0][Z] ;/* 今回の回転を代入して回転の符号が変わる所を検出する */

    /* ゆれが一番大きい奴が 音を鳴らす */
    if ( FRT_LGT_SeWork==work && FRT_LGT_SeTime!=GV_Time )
    {
	ActSetSound( prv, cur, work->rot_xyz[0][Z] ) ;
	if ( work->mode == FRT_MOD_FINISHED )
	    FRT_LGT_SeWork = NULL ;
	FRT_LGT_SeForce = 0.0f ;
    }
}

static void Act( Work *work )
{
    /* メッセージ受取 */
    FRT_HNG_ReceiveMessage( work ) ;

    /* 力の大きさ */
    FRT_HNG_ActMagnitude( work ) ;

    /* 動いてなければ処理を軽くする */
    if ( work->mode )
	FRT_HNG_ActMove( work ) ;
}


static void FRT_LGT_InitSound()
{
    FRT_LGT_SeTime  = 0    ;
    FRT_LGT_SeForce = 0.0f ;
    FRT_LGT_SeWork  = NULL ;
}

static int GetResources( Work *work, int name, int where )
{
    int      i, buf[3] ;
    DG_DEF  *def ;
    LIT_DEF *lit ;
    int md_id, bf_id, af_id ;
    int gl_id, vl_id ;

    work->name = name ;
    work->mode = FRT_MOD_FINISHED ;
    work->flag  = GCL_GetOptionValue( 'f', 0 ) ;
    work->flag |= GCL_GetOptionValue( 'b', 0 ) ;

    FRT_LGT_InitSound() ;

    if ( (i = GCL_GetOptionValue( 'l', 0 )) )
	lit = GV_GetCache( GV_CacheID( i, 'l' ) ) ;

    if ( GCL_GetOption( 'r' ) )
    {
	work->rot.vx = GCL_GetNextInt()*16 ;
	work->rot.vy = GCL_GetNextInt()*16 ;
	work->rot.vz = GCL_GetNextInt()*16 ;
    }
    if ( GCL_GetOption( 'p' ) )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &work->pos ) ;
    }

    if ( !GCL_GetOption( 'v' ) )
	PERROR( "No additional model ID(-volume option missing) :: NewPutHangLight\n" ) ;
    gl_id = GCL_GetNextInt() ;
    vl_id = GCL_GetNextInt() ;

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No model ID(-model option missing) :: NewPutHangLight\n" ) ;
    md_id = GCL_GetNextInt() ;
    bf_id = GCL_GetNextInt() ;
    af_id = GCL_GetNextInt() ;

    if ( !(def = GV_GetCache( GV_CacheID( md_id, 'k' ) )) )
	PERROR( "Cannot find KMS-Model( Not in data.cnf ) :: NewPutHangLight\n" ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_FINISHCALC, 0 )) )
	PERROR( "Cannot create DG_OBJS( Maybe no memory ) :: NewPutHangLight\n" ) ;
    DG_QueueObjs( work->objs ) ;
    GM_GroupObjs( work->objs, where ) ;
    MakeObjWorld( work ) ;
    DG_MakePreshade( work->objs, GM_GetMap( where )->light ) ;

    FRT_HNG_InitTarget( work, &work->objs->objs[1], where ) ;

    if ( !(i = GCL_GetOptionValue( 'h', 0 )) )
	PERROR( "No hazrad ID(-hazard option missing) :: NewPutHangLight\n" ) ;

    /* 電灯をつける処理 */
    {
	extern void *NewFortLight( int hzd_id, int where,
				   FMATRIX *parent, FVECTOR *pos,
				   int bf_id, int af_id, int gl_id, int vl_id,
				   int *flag, int off_mask,
				   int brk_mask, int flr_mask ) ;
	DG_OBJ  *obj = work->objs->objs ;

	GV_SetActorChild( work,
			  NewFortLight( i, where,
					&obj[1].world, &FRT_HNG_Offset[0],
					bf_id, af_id, gl_id, vl_id,
					&work->flag,
					0x0100<<0, 0x0010<<0, 0x0001<<0 ) ) ;
	GV_SetActorChild( work,
			  NewFortLight( i, where,
					&obj[1].world, &FRT_HNG_Offset[1],
					bf_id, af_id, gl_id, vl_id,
					&work->flag,
					0x0100<<1, 0x0010<<1, 0x0001<<1 ) ) ;
	GV_SetActorChild( work,
			  NewFortLight( i, where,
					&obj[1].world, &FRT_HNG_Offset[2],
					bf_id, af_id, gl_id, vl_id,
					&work->flag,
					0x0100<<2, 0x0010<<2, 0x0001<<2 ) ) ;
	GV_SetActorChild( work,
			  NewFortLight( i, where,
					&obj[1].world, &FRT_HNG_Offset[3],
					bf_id, af_id, gl_id, vl_id,
					&work->flag,
					0x0100<<3, 0x0010<<3, 0x0001<<3 ) ) ;
    }

    /* ワークに登録する */
    AddWorkList( work ) ;

#if MAKING
    for ( i=work->objs->n_models ; --i>=0 ; )
    {
	extern void *NewDrawWireframe( DG_OBJ *packs ) ;
	work->wireframe[i] = NewDrawWireframe( &work->objs->objs[i] ) ;
    }
#endif

    return 0 ;
}


void *NewFortHangLight( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

