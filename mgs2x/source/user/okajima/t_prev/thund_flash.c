//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thund_flash.c
	雲のフラッシュ 
	1999/11/16  H.TANAKA
	2000/10/18 S.Okajima
	$Id: thund_flash.c,v 1.1.1.3 2002/11/19 11:47:47 Yoshizawa1 Exp $
*/


#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <sys/types.h>
#endif

#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include "libutl.h"

#include   "gameheader.h"
#include   "libmt.h"
#ifdef PSX2
#include	"def_dma.h"
#endif
#include	"../etc/ok_util.h"

extern int ok_flush_status;

/* system/libutl/rand.h内の乱数関数を使いやすくするマクロ(added by K.Takabe 2001.7.29) */
#define RND(_n)		( (int)( ( (u_long64)irnd()*(_n) ) / ((u_long64)ULONG_MAX+1) ) )

#define	N_OBJS	 (5)
#define	MAX_FAR	(800000.0F)
#define	MAX_RANGE  (400000.0F)
#define	BORDER	 (300000.0F)
//#define	MAX_LEN	(35000.0F)
#define	MAX_LEN	(200000.0F)
#define	HEIGHT	 (200000.0F)
#define	BODY_FLAG	  (DG_FLAG_TEXT|DG_FLAG_NOFOG|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

#define	FLASH_LIFE	(20)
#define	FLASH_RAND	(20)

int big_thund_flash_flag = 0 ;   /* 光ったことを通知する */


/* vw が 0.0f 以外のとき強制的に光らせる */
FVECTOR big_thund_flash_force_vector = { 0.0f, 0.0f, 0.0f, 0.0f } ;

typedef struct
{
	GV_ACT_EX   actor ;
	int	  map ;

#ifdef PSX2
	DG_OBJS   *objs[N_OBJS] ;
#else
	DG_PRIM2   *prim ;
#endif
	FMATRIX   light[2] ;

	int	   count ;
	FVECTOR   base ;
	int	   interval ;	   /* 間隔 */
	
	int	   strong_flag ;

	int	   sound_count ;
	int	   sound_type ;

	int		flush_probability ;		/* 発生確率 */
	int		disable_thunder ;		/* 稲妻発生抑制 */

} Work ;


#ifndef PSX2
/*プリムに変更したので形をKMSと同じにしておく。w00_sky.kmxを参考のこと*/
#define N_VERTS 4
static FVECTOR thunder_shape[N_VERTS] = {
	{ 32000.0f, 0.0f, -32000.0f, 1.0f },
	{ 32000.0f, 0.0f,  32000.0f, 1.0f },
	{-32000.0f, 0.0f, -32000.0f, 1.0f },
	{-32000.0f, 0.0f,  32000.0f, 1.0f },
} ;
#endif

static void  Init_Work(Work *work)
{
	FVECTOR  pos ;
	FVECTOR  *base ;
	SVECTOR  rot ;
	FMATRIX  world ;
	int	  i ;
	float	scale ;
	int	  bright_wide ;
#ifndef PSX2
	FVECTOR        *prim_pos = work->prim->pos[work->prim->buffer_clock] ;
	DG_PRIM2_UVRGB *prim_uvs = work->prim->uvrgb[work->prim->buffer_clock] ;
#endif


	base = &work->base ;

	for(i = 0 ; i < N_OBJS; i++){
		/* 位置 */
		if(i == 0){
			DG_COPY_VEC( &pos, base );
		}else{
			pos.vx += frnd() * MAX_LEN ;
			pos.vz += frnd() * MAX_LEN ;
			pos.vy = base->vy ;
		}

		rot.vx = 0 ;
		rot.vy = irnd() % 4096 ;
		rot.vz = 0 ;
		/* 大きさ */
		scale = 4.0F + frnd() ;
		DG_SetPos2( &pos, &rot ) ;
		DG_GetPos( &world ) ;
		world.m[3][0] /= scale ;
		world.m[3][1] /= scale ;
		world.m[3][2] /= scale ;
		world.m[3][3] /= scale ;
		/* ライト数 */
		bright_wide = rnd() * 10.0F - 5.0F ;
		if(rnd() < 30.0F){
			work->light[1].m[3][0] = 25.0F + bright_wide;
			work->light[1].m[3][1] = 50.0F + bright_wide;
			work->light[1].m[3][2] = 50.0F + bright_wide;
			work->strong_flag = 1 ;
		}else{
			work->light[1].m[3][0] = 18.0F + bright_wide ;
			work->light[1].m[3][1] = 30.0F + bright_wide ;
			work->light[1].m[3][2] = 30.0F + bright_wide ;
			work->strong_flag = 0; 
		}
		work->interval = 120 + irnd() % 128 ;
		DG_SetPos(&world) ;
#ifdef PSX2
		DG_PutObjs(work->objs[i]) ;
#else
		DG_PutVector( thunder_shape, prim_pos, N_VERTS ) ;
		prim_pos += N_VERTS ;
		(prim_uvs+0)->r = (prim_uvs+1)->r = 
		  (prim_uvs+2)->r = (prim_uvs+3)->r = work->light[1].m[3][0] ;
		(prim_uvs+0)->g = (prim_uvs+1)->g = 
		  (prim_uvs+2)->g = (prim_uvs+3)->g = work->light[1].m[3][1] ;
		(prim_uvs+0)->b = (prim_uvs+1)->b = 
		  (prim_uvs+2)->b = (prim_uvs+3)->b = work->light[1].m[3][2] ;
		prim_uvs += N_VERTS ;
#endif
	}
}

/* サウンドをとってくる */
static void Init_GetSoundCount( Work *work )
{
	if((irnd()>>8)%2){
		work->sound_count =  30 + irnd() % 32;
		if((irnd()>>8)%2){
			work->sound_type  = SD_A_THUNDR01 ;
		}else{
			work->sound_type  = SD_A_THUNDR02 ;
		}
	}else{
		work->sound_count = 60 + irnd() % 64 ;
		if((irnd()>>8)%2){
			work->sound_type  = SD_A_CLOUD01 ;
		}else{
			work->sound_type  = SD_A_CLOUD02 ;
		}
	}
}

/* サウンドを鳴らす */
static void Init_CheckSound(Work *work)
{
	if(work->sound_count > 0){
		work->sound_count -- ;
		if(work->sound_count == 0){
			GM_SeSetMode(work->sound_type , &GM_PlayerPosition, GM_SEMODE_NORMAL ) ;
		}
	}
}


/*******************************************************************/
static void  Act(Work *work)
{
	FVECTOR  *base ;
	int	  i ;
	float	ftemp;
#ifndef PSX2
    int clock ;
	DG_PRIM2_UVRGB *prim_uvs ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;
#endif

	Init_CheckSound(work) ;


	base = &work->base ;
	if( !GM_CheckGameStatus( STATE_PLAY_DEMO ) ){
//		if(1){
//		if( GV_PadData[0].press & PAD_L1 ){
		if( RND(work->flush_probability) == 0 ){	/* changed by K.Takabe(2001.07.29) */
			extern void *NewThunder_Demo( float sx, float sy );
			if ( work->disable_thunder == 0 ){		/* added by K.Takabe(2001.07.29) */
				NewThunder_Demo( frnd(), -rnd()*2.0f );
			}

			work->count = (FLASH_LIFE>>1) + (irnd()>>8)%FLASH_RAND;

			DG_COPY_VEC( &work->base, &big_thund_flash_force_vector );
			if ( work->base.vy != 0.0f )
			  ftemp = HEIGHT / work->base.vy ;
			else
			  ftemp = 1.0e+24f ;
			work->base.vx *= ftemp;
			work->base.vy = HEIGHT;
			work->base.vz *= ftemp;

			Init_Work(work) ;

			big_thund_flash_flag = 1 ;   
			Init_GetSoundCount( work ) ;
			return ;
		}
	}else if( big_thund_flash_flag ){
		work->count = FLASH_LIFE + (irnd()>>8)%FLASH_RAND;
		DG_COPY_VEC( &work->base, &big_thund_flash_force_vector );
		if ( work->base.vy != 0.0f )
		  ftemp = HEIGHT / work->base.vy ;
		else
		  ftemp = 1.0e+24f ;
		work->base.vx *= ftemp;
		work->base.vy = HEIGHT;
		work->base.vz *= ftemp;

		Init_Work(work) ;

	}

	for(i = 0 ; i < N_OBJS; i++){
#ifdef PSX2	
		DG_InvisibleObjs(work->objs[i]) ;
#else
		work->prim->packet[clock][i].flag |= DG_PRIM2_INVISIBLE ;
#endif
	}

	if( work->count < 0 ) return;
	work->count -- ; 


	if( (irnd()>>8)%2 ){
		for(i = 0 ; i < N_OBJS; i++){
#ifdef PSX2	
			DG_VisibleObjs(work->objs[i]) ;
#else
			work->prim->packet[clock][i].flag &= ~DG_PRIM2_INVISIBLE ;
#endif
		}
	}

	if(work->count < FLASH_LIFE/2){
		work->light[1].m[3][0] -= 10.0F ;
		if(work->light[1].m[3][0] < 0.0F){
			work->light[1].m[3][0] = 0.0F ;
		}
		work->light[1].m[3][1] -= 10.0F ;
		if(work->light[1].m[3][1] < 0.0F){
			work->light[1].m[3][1] = 0.0F ;
		}
		work->light[1].m[3][2] -= 10.0F ;
		if(work->light[1].m[3][2] < 0.0F){
			work->light[1].m[3][2] = 0.0F ;
		}
#ifndef PSX2
		prim_uvs = work->prim->uvrgb[work->prim->buffer_clock] ;
		for( i=N_OBJS ; --i>=0 ; ){
			(prim_uvs+0)->r = (prim_uvs+1)->r = (prim_uvs+2)->r = (prim_uvs+3)->r = work->light[1].m[3][0] ;
			(prim_uvs+0)->g = (prim_uvs+1)->g = (prim_uvs+2)->g = (prim_uvs+3)->g = work->light[1].m[3][1] ;
			(prim_uvs+0)->b = (prim_uvs+1)->b = (prim_uvs+2)->b = (prim_uvs+3)->b = work->light[1].m[3][2] ;
			prim_uvs += N_VERTS ;
		}
#endif

		return ;
	}
	big_thund_flash_flag = 0 ;   
}

 static void   Die(Work *work)
{
#ifdef PSX2	
	int i ;

	for(i = 0 ; i < N_OBJS ; i++)
	{
	if(work->objs[i] != NULL)
	{
		DG_DequeueObjs(work->objs[i]) ;
		DG_FreeObjs(work->objs[i]) ;
		}
	}
#else
	if(work->prim != NULL)
	  GM_FreePrim2( work->prim ) ;
#endif
}


static  int  GetResources(Work  *work)
{
	int	  i ;

	work->count = -1 ;

	/* added by K.Takabe(2001.07.29) */
	work->flush_probability = 4096 ;
	if ( GCL_GetOption( 'p' ) ){
		work->flush_probability = GCL_GetNextInt();
	}
	if ( GCL_GetOption( 't' ) ){
		work->disable_thunder = 1 ;
	}

	/*ライトの初期化*/
	work->light[1].m[3][0] = 30.0F ;
	work->light[1].m[3][1] = 50.0F ;
	work->light[1].m[3][2] = 50.0F ;

#ifdef PSX2
	{
		DG_DEF   *def[2] ; 

		def[0] = (DG_DEF *)GV_GetCache(GV_CacheID(3241980 /*"w00_sky"*/,'k')) ;
		if(def[0] == NULL){
			printf("not sky_test\n") ;
			return -1 ;
		}

		for(i = 0; i < N_OBJS ; i++){
			work->objs[i] = DG_MakeObjs(def[0],BODY_FLAG,0) ;
			if(work->objs[i] == NULL) return -1 ;
			/* ライトの位置を取得 */
			DG_SetLightMatrix(work->objs[i],work->light) ;	
			DG_QueueObjs(work->objs[i]) ;
			DG_InvisibleObjs(work->objs[i]) ;
		}
	}
#else
	/* XBOXだけPRIMを通る */
#undef FTOI12
#define FTOI12(_f)      DG_FTOI( (float)(_f)*4096.0f )
	{
		FVECTOR        *p0, *p1 ;
		DG_PRIM2_UVRGB *u0, *u1 ;
		DG_TEX *tex ;
		int j ;

		work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
								   N_OBJS,
								   N_VERTS ) ;
		if ( work->prim == NULL ) {
			printf( "No Memory for DG_PRIM2 thund_flash.c\n" ) ;
			return -1 ;
		}
		tex = DG_GetTexture( 16001123 ) ; /* w00_sky_add.bmp */
		DG_ConfigPrim2Tex( work->prim, tex );
		DG_SetPrim2Alpha( work->prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		p0 = work->prim->pos[0]   ; p1 = work->prim->pos[1]   ;
		u0 = work->prim->uvrgb[0] ; u1 = work->prim->uvrgb[1] ;
		for( i=N_OBJS ; --i>=0 ; ) {
			work->prim->packet[0][i].flag |= DG_PRIM2_INVISIBLE ;
			work->prim->packet[1][i].flag |= DG_PRIM2_INVISIBLE ;

			for( j=N_VERTS ; --j>=0 ; ) {
				_sceVu0CopyVector( p0, &DG_ZeroVector ) ;
				_sceVu0CopyVector( p1, &DG_ZeroVector ) ;
				p0++, p1++ ;
				u0->u = u1->u = FTOI12( (j&1?0:1) * tex->u_scale + tex->u_offset ) ;
				u0->v = u1->v = FTOI12( (j>1?0:1) * tex->v_scale + tex->v_offset ) ;
				u0->q = u1->q = 4096 ;
				u0->f = u1->f = j>1 ? 0x8fff : 0x0fff ;
				u0->r = u1->r = work->light[1].m[3][0] ;
				u0->g = u1->g = work->light[1].m[3][1] ;
				u0->b = u1->b = work->light[1].m[3][2] ;
				u0->a = u1->a = 128 ;
				u0++, u1++ ;
			}
		}
	}
#endif

	/* 初期化を加えました T.Morita　2002.02.22　 */
	work->base.vx = 0.0f ;
	work->base.vy = HEIGHT;
	work->base.vz = 0.0f ;
	
	work->strong_flag = 0;
	return 0 ;
}

void   *NewThund_Flash(int name,int map )
{
	Work  *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER,sizeof(Work)) ;
	if(work != NULL){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->map = map ;
		if(GetResources( work ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
