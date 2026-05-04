//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fadeobj.c
	chara フェードオブジェ
	明滅モデル表示
	2002/03/11 K.Sigeno
	$Id: fadeobj.c,v 1.1.1.3 2002/11/19 11:49:55 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

extern void *NewSigBreakObj(int ,int ,FMATRIX *,float ,CVECTOR *) ;
extern void *NewSigRgbRandObj(int ,FMATRIX *,float ,int ,CVECTOR *) ;

	
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	FVECTOR		pos;	/*表示座標*/
	int			name ;
	int			flag ;
	short		cnt ;
	short		limit_time ;
	u_char		r ;
	u_char		g ;
	u_char		b ;
	u_char		pad ;
	/*******************/
} Work ;

/*message label*/
enum{
	STOP_REQUEST = 1,
};
enum{
	FLAG_STOP_REQUEST = 0x0001,
};


static void SetAmb(Work *work,u_char r,u_char g,u_char b){
#if 0
	work->lights[1].m[ 0 ][ 0 ] = (float) r ;
	work->lights[1].m[ 0 ][ 1 ] = (float) g ;
	work->lights[1].m[ 0 ][ 2 ] = (float) b ;
#endif
	work->lights[1].m[ 3 ][ 0 ] = (float) r ;
	work->lights[1].m[ 3 ][ 1 ] = (float) g ;
	work->lights[1].m[ 3 ][ 2 ] = (float) b ;
}

static void CheckMesg(Work *work){
	GV_MSG *msg;
	int n;
	if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for( ; n > 0; n-- ){
			switch( msg->message[ 0 ] ) {
				case STOP_REQUEST :
					work->flag |= FLAG_STOP_REQUEST ;
					break ;
			}
			msg++;
		}
	}
}


static void Act(Work *work)
{
	int col ;
	float col_rate ;
	int r,g,b;

	if(!(work->flag & FLAG_STOP_REQUEST)){
		CheckMesg(work) ;
	}
	col = work->cnt ;
	if(col > work->limit_time) {
		col = work->limit_time*2 - work->cnt ;
	}
	col_rate = (float)col/(float)work->limit_time ;
	r= (float)work->r*col_rate ;
	g= (float)work->g*col_rate ;
	b= (float)work->b*col_rate ;
#if 0
printf("FADEOBJ CNT [%d] LMT [%d] r[%d] g[%d] b [%d]\n",work->cnt,work->limit_time,r,g,b);
#endif

#if 1
	SetAmb(work,r,g,b) ;
#else
	DG_GetLightMatrix( &work->pos, work->lights );
#endif
	if(work->flag & FLAG_STOP_REQUEST){
		if(work->cnt > work->limit_time) {
			work->cnt++ ;
		}else if(work->cnt > 0){
			work->cnt-- ;
		}
	}else {
		work->cnt++ ;
	}
	work->cnt %= (work->limit_time*2) ;
#if 0
	if(work->cnt == 0){
		CVECTOR col ;
		col.r = 127 ;col.g = 127; col.b = 127 ;
		NewSigBreakObj(7298698, 7298698 ,&work->objs->world,30.0f, &col) ;
		col.r = 127 ;col.g = 0; col.b = 0 ;
		NewSigRgbRandObj(7298698,&work->objs->world,30.0f,180 ,&col) ;
		col.r = 0 ;col.g = 127; col.b = 0 ;
		NewSigRgbRandObj(7298698,&work->objs->world,30.0f,180 ,&col) ;
		col.r = 0 ;col.g = 0; col.b = 127 ;
		NewSigRgbRandObj(7298698,&work->objs->world,30.0f,180 ,&col) ;
	}
#endif
}
static void Die(Work *work)
{
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs );
}
static int GetResources(Work *work, int name, int where)
{
	DG_DEF *def ;
    int buf[3],model;
	work->cnt = 0 ;
	work->name = name ;
	work->flag = 0 ;
	/* 座標 */
	if ( GCL_GetOption( 'p' ) ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->pos ) ;
	}
	/* モデル */
	if ( GCL_GetOption( 'k' ) != NULL ) {
		model = GCL_GetNextInt() ;
		def = (DG_DEF*) GV_GetCache( GV_CacheID( model, 'k' ) ) ;
		work->objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_NOFOG, DG_CHANL_MAIN ) ;
		DG_QueueObjs( work->objs ) ;
		DG_SetLightMatrix( work->objs, work->lights );

		DG_SetPos2( &work->pos , &DG_ZeroSVector ) ;
		DG_PutObjs( work->objs ) ;
	}
	if ( GCL_GetOption( 'r' ) ){
		work->r = GCL_GetNextInt() ;
		work->g = GCL_GetNextInt() ;
		work->b = GCL_GetNextInt() ;
	}else {
		work->r = work->g = work->b = 127 ;
	}
	if ( GCL_GetOption( 't' ) ){
		work->limit_time = GCL_GetNextInt() ;
	}else {
		work->limit_time = 60;
	}
	return 1;
}



/* 初期化部メイン */
void *NewFadeObj( int name,int where)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

