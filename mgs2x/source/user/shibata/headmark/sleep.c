//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sleep.c
	ＺＺＺエフェクト
	2000/10/14 T.Shibata

	$Id: sleep.c,v 1.1.1.3 2002/11/19 11:48:47 Yoshizawa1 Exp $
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include "BP_Misc.h"

/* ------------------------------------------------------ */
           /* 
               補助マクロ
           */

#define   FTOI12(_f)        ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define  N_PRIMS	 1
#define  N_VERTS 	 4
#define  PRIM_TYPE       (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
			     
#define    MAX_COUNT    1800

typedef struct {
    GV_ACT_EX		actor ;

    DG_PRIM2	    *prim ;
    FMATRIX	    	*world ;
	int				*map;
    short           count[N_VERTS] ;         /* カウント */

    short			timer;
	short			interval;
}
SSleepZZZWork;


#if 1
static  int  ReceiveSignal( void *workp, int signal, int value)
{
    SSleepZZZWork *work = workp ;

    switch(signal){
	case 0:
      // this determines how many little Zs show up :)
		work->interval = DIRECT_TICK(160/(value+1));//DIRECT_TICK(160 - value * 40);
		break;
	default:
		return GV_DefaultSignalFunc( work, signal, value );// Must do this!!!
	}

	printf("sleep value %d\n",work->interval);
	return 0;
}
#endif

/* -------------------------------------------------------------------- */ 
static  void  DecidePos(FMATRIX *world, FVECTOR *pos, int count)
{
   FVECTOR mem_fvector[3];
    FVECTOR  *shift ;
    FVECTOR  *center ;
    FVECTOR  *base ;
    SVECTOR  ang ;

    shift  = &mem_fvector[0];
    center = &mem_fvector[1];
    base   = &mem_fvector[2];

   if ( BP_IsPAL()==TRUE )
      shift->vx = 1.2F*6.0f/5.0f * count ;
   else
      shift->vx = 1.2F * count ;

    shift->vy = 0.0F ;
    shift->vz = 0.0F ;
    shift->vw = 1.0F ;
    ang.vx = 0 ;

    if ( BP_IsPAL()==TRUE )
      ang.vy = ((64*6/5) * count) % 4096 ;
    else
      ang.vy = (64 * count) % 4096 ;

    ang.vz = 0 ;

    DG_SetPos2(&DG_ZeroVector, &ang) ;
    DG_PutVector(shift, base, 1) ;

    /* 移動距離 */
    shift->vx = 0.0F ;
    shift->vy = 120.0F ;
    shift->vz = 300.0F ;
    shift->vw = 1.0F ;
    DG_SetPos(world) ;
    DG_RotVector(shift,shift,1) ;
    
    center->vx = shift->vx + world->m[3][0] ;

    if ( BP_IsPAL()==TRUE )
      center->vy = 200.0F + (2.5F*6.0f/5.0f) * count + world->m[3][1] ;
    else
      center->vy = 200.0F + 2.5F * count + world->m[3][1] ;

    center->vz = shift->vz + world->m[3][2] ;

    ang.vx = 0 ;
    ang.vy = 0 ;
    ang.vz = 0 ;
    DG_SetPos2(center,&ang) ;
    DG_PutVector(base,pos,1) ;
}

static unsigned char CheckColor(short tmp)
{
   if(tmp > 255)
      tmp = 255;
   else if(tmp < 0)
      tmp = 0;
    
   return (unsigned char)tmp;
}

static void DecideUVRGBWH(DG_PRIM2_UVRGBWH *uvrgbwh, int count)
{
   int stopFadingInCount = DIRECT_TICK(50);
   int fadeCount = count < stopFadingInCount ? count : stopFadingInCount;

   uvrgbwh->r = 100*fadeCount/stopFadingInCount;
   uvrgbwh->g = 100*fadeCount/stopFadingInCount;
   uvrgbwh->b = 150*fadeCount/stopFadingInCount;
   uvrgbwh->a = 128;
   uvrgbwh->w = count/2;
   uvrgbwh->h = count/2;
}


 	
static void Act(SSleepZZZWork *work)
{
   int  				i, count;
   DG_PRIM2			*prim;
   FVECTOR				*pos;
   DG_PRIM2_UVRGBWH	*uvrgbwh;

	prim = work->prim;
	GM_GroupPrim2( prim, *work->map );
   DG_SwitchBuffPrim2( prim ) ;

   pos = prim->pos[prim->buffer_clock];
	uvrgbwh = prim->uvrgb[prim->buffer_clock];
	
   for (i = 0; i < N_VERTS; ++i, ++pos, ++uvrgbwh)
   {
		count = work->count[i];

      if (count > DIRECT_TICK(163))
      {
         // free
         work->count[i] = -1;
      }
      else if (count > DIRECT_TICK(160))
      {
         // hide the ZZZs
         uvrgbwh->a = 0;
         ++work->count[i];
		}
      else if (count >= 0)
      {
			DecidePos(work->world, pos, count);
			DecideUVRGBWH(uvrgbwh, count);
         ++work->count[i];
		}
   }

   if (work->timer++ >= work->interval)
   {
      // find a free one
      for (i = 0; i < N_VERTS; ++i)
      {
         if (work->count[i] < 0)
         {
            // and start it going again
            work->count[i] = 0;
            work->timer = 0;
            break;
         }
      }
   }
}

static  void  Die(SSleepZZZWork *work)
{
   if (work->prim != NULL)
   {
      GM_FreePrim2(work->prim);
   }
}
     
/* -------------------------------------------------------------------- */
static int Init_Scr(SSleepZZZWork *work, DG_TEX *tex)
{
    FVECTOR           *pos, *pos1;
    DG_PRIM2_UVRGBWH  *uvrgbwh, *uvrgbwh1;

    int               i ;
    
    pos     = work->prim->pos[0];
    pos1     = work->prim->pos[1];
    uvrgbwh = work->prim->uvrgb[0];
    uvrgbwh1 = work->prim->uvrgb[1];
    
    for(i = 0; i < N_VERTS; i++) {
		work->count[i] = -1;

		*pos = DG_ZeroVector ;
		*pos1 = DG_ZeroVector ;
      // clear the padding between members
      memset(uvrgbwh, 0, sizeof(DG_PRIM2_UVRGBWH));
		uvrgbwh->u0 = FTOI12( tex->u_offset ) ;
		uvrgbwh->v0 = FTOI12( tex->v_offset ) ;
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
	
		*uvrgbwh1 = *uvrgbwh;

		pos++;
		uvrgbwh++;
		pos1++;
		uvrgbwh1++ ;
	}

	return 0;
}

static int GetResources( SSleepZZZWork *work, int *map )
{
	DG_TEX	       *tex ;
	DG_PRIM2       *prim ;

	if(work->world == NULL)
	    return -1 ;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE, N_PRIMS, N_VERTS);

	if(prim == NULL) return -1 ;
	//tex = DG_GetTexture(GV_StrCode("zzz_msk"));
	tex = DG_GetTexture(13722182);
	if(tex == NULL) return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 )) ;


	/* スクラッチパット上で初期化したデータを実際のワークにコピーする */
	Init_Scr(work,tex) ;

	/* シグナル設定 */
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

	work->timer = work->interval = DIRECT_TICK(40);
	
	work->map = map;
	return 0;
}

/* -------------------------------------------------------------------- */
void *NewSleep(FMATRIX *world, int *map )
{
	SSleepZZZWork		*work ;

	work = (SSleepZZZWork *)GV_NewEffect( GV_ACTOR_USER, sizeof( SSleepZZZWork ) ) ;
	if ( work != NULL ) {
	    work->world = world ;
	    GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX(&work->actor) ;
	    if ( GetResources( work, map ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	    }
	}
	return (void *)work ;
}
