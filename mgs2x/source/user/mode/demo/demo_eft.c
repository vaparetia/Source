//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_eft.c
	エフェクトパケット処理

	2000/07/26	K.Takabe
	$Id: demo_eft.c,v 1.2 2002/12/04 10:29:00 takaki Exp $

*/
/*

	void DM_DestroyEffectAll( void )

	指定ＩＤのエフェクトを破棄


	void DM_DestroyEffect( int id )
	int		id ;	デモ認識用ＩＤ

	指定ＩＤのエフェクトを破棄


	void DM_SendEffectMessage()
	キャラにメッセージを送る

	--------------------------------
	＜パケット別処理ルーチン用関数＞

	void DM_Packet_Effect( DM_WORK *work, DEMO_EFFECT *packet )
	エフェクトパケット処理
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <mgs_type.h>
#include <libgv.h>
#include <libdg.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>
#include <camera.h>

#include "libdemo.h"

#include "bp_matrix.h"

#ifdef DEBUG_MODE
#include <string.h>
#endif

int DM_EffectInitName ;
int DM_EffectInitFlag ;

/* ---------------------------------------------------------------- */
/* エフェクトパケット処理 */
void DM_Packet_Effect( DM_WORK *work, DEMO_EFFECT *packet )
{
	DM_EFFECT	*effect ;

#ifdef PSX2
#ifdef DEBUG_MODE
	int  i, j;
	char id[1024];
#endif
#endif

   if ( !DEMO_PACKET_IS_ENDIAN_SWAP( packet->header ) )
   {
      int size = DEMO_PACKET_SIZE( packet->header ) - sizeof(DEMO_EFFECT) ;

      DEMO_PACKET_SET_ENDIAN_SWAP( packet->header );

      BP_LE_SwapSInt_Inp( &packet->effect_id );
      BP_LE_SwapSInt_Inp( &packet->exec_type );
      BP_LE_SwapSInt_Inp( &( packet->pad[0] ));
      BP_LE_SwapSInt_Inp( &( packet->pad[1] ));

      if ( size )
      {
         BP_LE_SwapSIntArray_Inp( packet->data, size / sizeof( int ) );
      }
   }

#ifdef PSX2
#ifdef DEBUG_MODE

	/* スロー再生時など多重呼びを回避する */
	if( (dm_debug.mode1 != 0) && (dm_debug.play_status != 1) )
		return;

	/* e フラグが立ち、リストにエフェクトがあったのならここは必ず通る */
	if( dm_debug.n_effect != 0 ) {
		sprintf(id, "%8x\n", packet->effect_id);
		j = -1 ;
		for ( i = 0; i < VIEW_LIST_COUNT; i++ ) {
			/* ビューリストを検索して登録 */
			if ( dm_debug.effect_view[i].view == 0 ) {
				/* フレーム数と表示フラグを設定 */
				dm_debug.effect_view[i].start = dm_debug.frame;
				dm_debug.effect_view[i].view  = 1;
				/* エフェクトリストから名称を検索 */
				for ( j = 0; j < dm_debug.n_effect; j++ ) {
					if ( strncmp(dm_debug.effect_list[j].id, id, 8) == 0 ) {
						dm_debug.effect_view[i].list_index = j;
						break;
					}
				}
				if ( j == dm_debug.n_effect ) {
					/* 未登録エフェクトであった */
					strcpy(dm_debug.effect_list[dm_debug.n_effect].id, id);
					strcpy(dm_debug.effect_list[dm_debug.n_effect].name, id);
					dm_debug.effect_view[i].list_index = dm_debug.n_effect;
					dm_debug.n_effect++;
					ASSERT( dm_debug.n_effect < DM_MAX_EFFECT );
				}
				break;
			}
		}
		/* ターミナル出力 */
		if ( j >= 0 )
		    printf("run effect:%s(%d)\n", 
			   dm_debug.effect_list[j].name, dm_debug.frame);
	}
#endif
#endif

	/* ローポリ劇場用に髪の毛エフェクトなどは,全て取る */
	if ( work->low_poly_demo )
	    if ( packet->effect_id == 0x0100a002 || /*常駐 テロップ    */
		 packet->effect_id == 0x01001020 || /*装備品 ライデン  */
		 packet->effect_id == 0x01001006 || /*装備品 スネーク  */
		 packet->effect_id == 0x01001007 || /*装備品 オルガ    */
		 packet->effect_id == 0x01001014 || /*装備品 フォーチュン   */
		 packet->effect_id == 0x0100100c || /*装備品 セルゲイ  */
		 packet->effect_id == 0x01001015 || /*装備品 エマ      */
		 packet->effect_id == 0x0100101f || /*装備品 プラントオルガ */
		 packet->effect_id == 0x01003011 || /*物シム・オルガ    */
		 packet->effect_id == 0x01003000 || /*物シム・バンダナ  */
		 packet->effect_id == 0x01003012 || /*物シム・関節揺らし*/
		 packet->effect_id == 0x01003010 || /*物シム マント     */
		 packet->effect_id == 0x01003004 || /*物シム 髪の毛     */
		 packet->effect_id == 0x01003008 || /*物シム 新髪の毛   */
		 packet->effect_id == 0x01003007 || /*物シム 新髪の毛ＥＶＭ  */
		 packet->effect_id == 0x01003009 || /*物シム 新髪の毛ＥＶＭ２*/
		 packet->effect_id == 0x01004004 || /*汎用ＣＶ２頂点アニメ   */
		 packet->effect_id == 0x01006011 || /*効果 リンク汎用点滅    */
		 packet->effect_id == 0x01001016  ) /*シールズ_目パチ        */
	    {
		printf( "SKIP effect[0x%08x] for LOW-POLYGON-CINEMA\n" , packet->effect_id ) ;
		return ;
	    }

      /* 対応エフェクトの起動 */
      effect           = &work->eft_list[ work->n_effect ] ;
      effect->id       = DEMO_PACKET_ID( packet->header );
      effect->work_ptr = NewDemoEffectLaunch( packet, work->actor ) ;
      if ( effect->work_ptr ){
#ifdef DEBUG_MODE
	  printf( "%dth effect create[0x%08x] header%d type%d\n" , work->n_effect, packet->effect_id, packet->header.id, packet->exec_type ) ;
#endif
	  work->n_effect++ ;/*ちゃんと起動したら足す*/
      }
}
/* ---------------------------------------------------------------- */
	/*
		指定ＩＤのエフェクトを破棄
	*/
void DM_DestroyEffectAll( void )
{
	DM_WORK		*work ;
	DM_EFFECT	*effect ;
	int			i ;

	/* デモ管理構造体へのポインタを取得 */
	if ( ( work = DM_GetDemoWork() ) == NULL ) return ;

	effect = work->eft_list ;
	for ( i = work->n_effect ; i > 0 ; effect++, i-- ){
		GV_CallSignalFunc( effect->work_ptr, GV_SIGNAL_KILL, 0 );
	}
}
/* ---------------------------------------------------------------- */
	/*
		指定ＩＤのエフェクトを破棄
	*/
void DM_DestroyEffect( int id )
{
	DM_WORK		*work ;
	DM_EFFECT	*effect ;
	int			i ;

	/* デモ管理構造体へのポインタを取得 */
	if ( ( work = DM_GetDemoWork() ) == NULL ) return ;

	/* 検索 */
	effect = work->eft_list ;
	for ( i = work->n_effect ; i > 0 ; effect++, i-- ){
		if ( effect->id == id ) break ;
	}
	if ( i == 0 ) return ;

	/* オブジェクト表示アクターに終了シグナルを送る */
	GV_CallSignalFunc( effect->work_ptr, GV_SIGNAL_KILL, 0 );

	/* 削除するオブジェクト以降を前に詰める */
	for (  ; i > 0 ; effect++, i-- ){
		effect[0] = effect[1] ;
	}
}

/* ---------------------------------------------------------------- */

/*引数の順序変更は 厳禁*/
void *DM_SendEffectMessage( int name,
			    int data0, int data1, int data2, int data3,
			    int data4, int data5, int data6, int data7,
			    int command, int num )
{
    GV_MSG msg ;
    int buffer[9] = { command, data0,data1,data2,data3, data4,data5,data6,data7 } ;

    if ( name == DM_EffectInitName )
    {
	extern void DM_EffectInitCommand( int *buffer, int num ) ;
	DM_EffectInitCommand( buffer, num ) ; /* source/user/morita/demo_init/demo_init.c */
    }
    else
    {
	//printf( "Mesg %x dat%d %d %d %d %d %d %d %d cmd %d num %d\n",
	//	name,	data0,data1,data2,data3, data4,data5,data6,data7, command, num ) ;

	if ( name )
	{
	    msg.address = name ;
	    msg.message = buffer ;
	    msg.message_len = num ;
	    GV_SendMessage( &msg ) ;
	}
    }
    return  NULL ;
}

void* DM_NULL()
{
    return NULL ;
}

void* DM_ChangeAmbient( IVECTOR *rgb )
{
#ifdef DEBUG_MODE
#ifdef PSX2
	if( !(dm_debug.sAmbientFlag & FLAG_AMBIENT_COLOR))
#endif
	    DG_SetAmbient( rgb->vx, rgb->vy ,rgb->vz ) ;
#else
    DG_SetAmbient( rgb->vx, rgb->vy ,rgb->vz ) ;
#endif
    
    return NULL ;
}

void* DM_ChangeParallel( IVECTOR *rgb, FVECTOR *dir )
{
    FVECTOR l_dir ;

    _sceVu0Normalize( &l_dir, dir ) ;
    _sceVu0ScaleVector( &l_dir, &l_dir, 4096 );

#ifdef DEBUG_MODE
#ifdef PSX2
	if( !(dm_debug.sParallelFlag & FLAG_PARALLEL_DIR))
#endif
	    DG_SetMainLightDir( (int)l_dir.vx, (int)l_dir.vy, (int)l_dir.vz );

#ifdef PSX2
	if( !(dm_debug.sParallelFlag & FLAG_PARALLEL_COLOR))
#endif
	    DG_SetMainLightCol( rgb->vx, rgb->vy ,rgb->vz ) ;
#else
    DG_SetMainLightDir( (int)l_dir.vx, (int)l_dir.vy, (int)l_dir.vz );
    DG_SetMainLightCol( rgb->vx, rgb->vy ,rgb->vz ) ;
#endif
    return NULL ;
}

void *DM_DebugPrint( char *message )
{
#if DEBUG_MODE
#ifdef PSX2
    int last ;

    if ( message ? (last = strlen( message )) : 0 )
    {
	int   i = 0 ;
	char *c = message ;

printf( "LAST %d ", last ) ;
	/* ￥マークがきたら改行に変える */
	while( (c = strstr(c, "\\n")) )
	    *c++=' ',*c='\n' ;

	c = message ;
	/*足し分入れた全ての行数を数える*/
	while( (c = strchr(c+1, '\n')) )
	    i++ ;

	c = dm_debug.print_buffer - 1 ;
	while( (c = strchr(c+1, '\n')) )
	    i++ ;
	if ( message[last] != '\n' )
	    i++ ;
	if ( (i -= 11) > 0 ) /*10行分だけ表示させるから,結果 i は,削る行数になる。*/
	{

	    c = dm_debug.print_buffer - 1 ;
	    while( --i>=0 )
		c = strchr( c+1, '\n' ) ;
	    memmove( dm_debug.print_buffer, c+1, strlen(c+1)+1) ;
	}
	strcat( dm_debug.print_buffer, message ) ;
	if ( message[last] != '\n' )
	    strcat( dm_debug.print_buffer, "\n" ) ;
    }
#endif
#endif

    return NULL ;
}

void *DM_ExecProc( int proc )
{
    DM_WORK *work ;
    int i ;

    /* デモ管理構造体へのポインタを取得 */
    if ( !( work = DM_GetDemoWork() ) )
	return NULL ;
    for ( i=0 ; i<DM_MAX_PROC ; i++ )
	if ( proc==work->proc_id[i] )
	{
	    GCL_ExecProc( work->proc_id[i], NULL ) ;
	    break ;
	}
    return NULL ;
}

void *DM_SetClipParam( float near_para, float far_para )
{
    extern void DG_SetClipParam( float near_para, float far_para ) ;

    DG_SetClipParam( near_para, far_para ) ;

    return NULL ;
}

int DM_GetCurrentMap()
{
    return GM_CurrentMap ;
}

void DM_IVecToSVector( IVECTOR *i_vec, SVECTOR *s_vec )
{
	s_vec->vx = i_vec->vx;
	s_vec->vy = i_vec->vy;
	s_vec->vz = i_vec->vz;
	s_vec->pad = i_vec->vw;
}

void DM_GetRotToRadVec(FVECTOR *r,float x,float y,float z,float w)
{
	r->vx = (x * 3.141593653589793F)/180.0f;
	r->vy = (y * 3.141593653589793F)/180.0f;
	r->vz = (z * 3.141593653589793F)/180.0f;
	r->vw = (w * 3.141593653589793F)/180.0f;
}

void DM_InFVector(FVECTOR *r,float x,float y,float z,float w)
{
	r->vx = x;
	r->vy = y;
	r->vz = z;
	r->vw = w;
}

void DM_GetFMatrixM_XYZ( FVECTOR *out, FVECTOR *rot, int num )
{
    FMATRIX mat;

    _sceVu0RotMatrixX( &mat, &DG_UnitMatrix,rot->vx ) ;
    _sceVu0RotMatrixY( &mat, &mat,          rot->vy ) ;
    _sceVu0RotMatrixZ( &mat, &mat,          rot->vz ) ;

    DG_COPY_VEC(out,(FVECTOR*)mat.m[num]);
}

void DM_GetFMatrixM( FVECTOR *out, FVECTOR *rot, int num )
{
	FMATRIX mat;

	_sceVu0RotMatrix( &mat, &DG_UnitMatrix, rot );

	printf("%f:%f:%f\n",mat.m[0][0],mat.m[0][1],mat.m[0][2]);
	printf("%f:%f:%f\n",mat.m[1][0],mat.m[1][1],mat.m[1][2]);
	printf("%f:%f:%f\n",mat.m[2][0],mat.m[2][1],mat.m[2][2]);
	
	DG_COPY_VEC(out,(FVECTOR*)mat.m[num]);
}


void DM_GetFMatrixM_ZXY( FVECTOR *out, FVECTOR *rot, int num )
{
	FMATRIX mat;

	_sceVu0RotMatrixZ( &mat, &DG_UnitMatrix, rot->vz );
	_sceVu0RotMatrixX( &mat, &mat, rot->vx );
	_sceVu0RotMatrixY( &mat, &mat, rot->vy );

//	printf("%f:%f:%f\n",mat.m[0][0],mat.m[0][1],mat.m[0][2]);
//	printf("%f:%f:%f\n",mat.m[1][0],mat.m[1][1],mat.m[1][2]);
//	printf("%f:%f:%f\n",mat.m[2][0],mat.m[2][1],mat.m[2][2]);
	
	DG_COPY_VEC(out,(FVECTOR*)mat.m[num]);
}

void DM_ControlChange( int   name ,
		       float rot_x,float rot_y,float rot_z,
		       float pos_x,float pos_y,float pos_z )
{
#if 0
    CONTROL *control ;

    control = GM_SearchWhere( name ) ;
    if ( !control )
	printf( "No Control[%x]!!! Call someone-else!!!!\nPlease.  ... Oh, no !!!\n", name ) ;
    else
    {
	control->rot.vx = (rot_x * 2048.0f)/M_PI ;
	control->rot.vy = (rot_y * 2048.0f)/M_PI ;
	control->rot.vz = (rot_z * 2048.0f)/M_PI ;
	control->mov.vx = pos_x ;
	control->mov.vy = pos_y ;
	control->mov.vz = pos_z ;
    }
#else
	EFTCONTROL *data;

	data = DM_GetEftControl( name );

    if ( !data ) {
		//printf( "No EftControl[%x] in DM_ControlChange\n", name ) ;
    } else {
		data->rot.vx = (short)((rot_x * 2048.0f)/M_PI) ;
		data->rot.vy = (short)((rot_y * 2048.0f)/M_PI) ;
		data->rot.vz = (short)((rot_z * 2048.0f)/M_PI) ;
		data->mov.vx = pos_x ;
		data->mov.vy = pos_y ;
		data->mov.vz = pos_z ;
    }
#endif
}

void  DM_EftControlMatrix( EFTCONTROL *eft_con, FMATRIX *world )
{
    FVECTOR rot ;

    ASSERT( world && eft_con ) ;

//printf( "%d %d %d\n",eft_con->rot.vx,eft_con->rot.vy,eft_con->rot.vz ) ;

    rot.vx = (eft_con->rot.vx * M_PI)/2048.0f ;
    rot.vy = (eft_con->rot.vy * M_PI)/2048.0f ;
    rot.vz = (eft_con->rot.vz * M_PI)/2048.0f ;
#if 1
    _sceVu0RotMatrixX( world, &DG_UnitMatrix,rot.vx ) ;
    _sceVu0RotMatrixY( world, world,         rot.vy ) ;
    _sceVu0RotMatrixZ( world, world,         rot.vz ) ;
#else
    _sceVu0RotMatrixZ( world, &DG_UnitMatrix,rot.vz ) ;
    _sceVu0RotMatrixY( world, world,         rot.vy ) ;
    _sceVu0RotMatrixX( world, world,         rot.vx ) ;
#endif
    _sceVu0CopyVectorXYZ( (FVECTOR *)world->m[3], &eft_con->mov ) ;

    //HZX_ViewMatrix( world, 100.0f ) ;
}


/* ---------------------------------------------------------------- */

