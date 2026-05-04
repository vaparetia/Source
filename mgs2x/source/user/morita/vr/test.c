#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#define _MAIN_FILE_
#include "vr.h"


static void Act( Work *work )
{
#ifdef SCRPAD
    volatile sceDmaChan      *sprDma ;

    sprDma = sceDmaGetChan( 9 /* toSPR */ ) ;
    sprDma->sadr = SCRPAD_ADDR ;
    sceDmaSendN( sprDma, &src, sizeof(ScrPad)/0x10 ) ;
    sceDmaSync( sprDma, 0, 0 ) ;

    ActWave( work, (ScrPad *)SCRPAD_ADDR ) ;

    sprDma = sceDmaGetChan( 8 /* fromSPR */ ) ;
    sprDma->sadr = SCRPAD_ADDR ;
    sceDmaSendN( sprDma, &src, sizeof(ScrPad)/0x10 ) ;
    sceDmaSync( sprDma, 0, 0 ) ;
#else
    static int flag = 1 ;

    if ( GV_PadData[0].press & PAD_STA ) flag ^= 1 ;
    if ( flag || GV_PadData[0].press & PAD_L1 )
    {
    }
#endif
}

static void Die( Work *work )
{
}

//DG_PRIM_SORTONLY|DG_PRIM_SPRT3D2
static int GetResources( Work *work )
{
    DG_PRIM_PACKET *packet0, *packet1 ;
    DG_SPRT *p0, *p1 ;
    DG_PRIM **p ;
    DG_TEX *t ;
    int i, j ;

    p  = work->prim ;
    t  = DG_GetTexture( WAVE_BRK_TEX ) ;
    for ( i=1 ; --i>=0 ; p++ )
    {
        if ( !(*p = GM_MakePrim( DG_PRIM_ON_WORLD|DG_PRIM_SPRT3D2, 1, 1, work->pos, (FVECTOR*)work->size )) )
            return -1 ;

        (*p)->world.m[3][X] = 0     ;
        (*p)->world.m[3][Y] = 400   ;
        (*p)->world.m[3][Z] = 15000 ;

	packet0 = (DG_PRIM_PACKET*)( (*p)->packs[0] ) ;
	packet1 = (DG_PRIM_PACKET*)( (*p)->packs[1] ) ;

	*(u_long*)&packet0->prim_init.tex0  = t->tex0_base ;
	*(u_long*)&packet1->prim_init.tex0  = t->tex0_base ;
	*(u_long*)&packet0->prim_init.clamp = t->clamp ;
	*(u_long*)&packet1->prim_init.clamp = t->clamp ;
	
	    /* プリミティブデータ部の初期化処理 */
	p0 = (DG_SPRT*)packet0->prim_top ;
	p1 = (DG_SPRT*)packet1->prim_top ;
	for ( j=1 ; --j>=0 ; p0++, p1++ )
	{
	    work->pos[j].vx = work->pos[j].vy = work->pos[j].vz = 0 ;
	    work->size[j][X] = work->size[j][Y] = 400 ;
	    work->vel[j] = 0 ;
	    work->alph[j] = 84 ;

	    DG_SET_RGBA1( p0, 128, 128, 128,  84 ) ;
	    DG_SET_RGBA1( p1, 128, 128, 128,  84 ) ;
	    DG_SET_STQ1( p0, t->u_offset           , t->v_offset           , 1.0F ) ;
	    DG_SET_STQ1( p1, t->u_offset           , t->v_offset           , 1.0F ) ;
	    DG_SET_STQ2( p0, t->u_offset+t->u_scale, t->v_offset+t->v_scale, 1.0F ) ;
	    DG_SET_STQ2( p1, t->u_offset+t->u_scale, t->v_offset+t->v_scale, 1.0F ) ;
	}
    }
    return 0 ;
}

void *NewOceanWave( int name, int where )
{
	Work *work ;

	if ( (work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) )) != NULL )
	{
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 )
		{
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}




