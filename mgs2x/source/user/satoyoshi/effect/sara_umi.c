//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	set_tex_f.c
	拡張海セット
	1999/11/09 S.Okajima
	$Id: sara_umi.c,v 1.1.1.3 2002/11/19 11:48:17 Yoshizawa1 Exp $

*/

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
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../../okajima/etc/ok_util.h"

extern int ok_flush_status;
extern float ok_focus_z_far;

#define M_PI 3.14159265358979323846264338327950288419716939937510f

#define	DUMMY_Z			(5000.0f)
#define	RATIO_MAX		(48)
#define	RGB_0		(200)
#define	RGB_1		(210)

/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR		(8000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR		(-4000.0f)


#define ITA_NUM		(work->devide)
#define TATE_DIV	(5)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

/*----------------------------------------------------------------*/
typedef	struct	{
    GV_ACT_EX	actor ;
    DG_PRIM2	*prim[300];
    FVECTOR	center;

    int		lines;
    int		devide;
    int		small_r;
    int		large_r;
    float	hight;
} Work ;


/*----------------------------------------------------------------*/
// verts は FVECTOR ４つ分作成
static void MakeVerticsbyRot( Work *work, FVECTOR *center, float small_r, float large_r,
			      float rot_center, float rot_width, int hight, FVECTOR *verts )
{
    verts[0].vx = center->vx + small_r*cosf(rot_center+rot_width);
    verts[0].vz = center->vz + small_r*sinf(rot_center+rot_width);
    verts[0].vy = center->vy;

    verts[1].vx = center->vx + small_r*cosf(rot_center-rot_width);
    verts[1].vz = center->vz + small_r*sinf(rot_center-rot_width);;
    verts[1].vy = center->vy;

    verts[2].vx = center->vx + large_r*cosf(rot_center+rot_width);
    verts[2].vz = center->vz + large_r*sinf(rot_center+rot_width);
    verts[2].vy = center->vy + hight;

    verts[3].vx = center->vx + large_r*cosf(rot_center-rot_width);
    verts[3].vz = center->vz + large_r*sinf(rot_center-rot_width);
    verts[3].vy = center->vy + hight;

    if (center->vy == work->center.vy){
	float templen = 180000.0f+(work->lines*60000.0f);
	
	if (verts[0].vx > center->vx + templen){
	    verts[0].vx = center->vx + templen;
	    verts[0].vy = center->vy-100.0f;
	}
	else if (verts[0].vx < center->vx - templen){
	    verts[0].vx = center->vx - templen;
	    verts[0].vy = center->vy-100.0f;
	}
	if (verts[1].vx > center->vx + templen){
	    verts[1].vx = center->vx + templen;
	    verts[1].vy = center->vy-100.0f;
	}
	else if (verts[1].vx < center->vx - templen){
	    verts[1].vx = center->vx - templen;
	    verts[1].vy = center->vy-100.0f;
	}
	if (verts[0].vz > center->vz + templen){
	    verts[0].vz = center->vz + templen;
	    verts[0].vy = center->vy-100.0f;
	}
	else if (verts[0].vz < center->vz - templen){
	    verts[0].vz = center->vz - templen;
	    verts[0].vy = center->vy-100.0f;
	}
	if (verts[1].vz > center->vz + templen){
	    verts[1].vz = center->vz + templen;
	    verts[1].vy = center->vy-100.0f;
	}
	else if (verts[1].vz < center->vz - templen){
	    verts[1].vz = center->vz - templen;
	    verts[1].vy = center->vy-100.0f;
	}
    }
    
}
/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center,
			float small_r, float large_r, int hight, 
			float rot_center, float rot_width, 
			int colr, int colg, int colb )
{
    FVECTOR			*pos ;
    DG_PRIM2_UVRGB		*uvrgb ;
    
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
    
    pos   = SCR_POS ;
    uvrgb = SCR_UVS ;
    
    
    MakeVerticsbyRot(work, center, small_r, large_r,
		      rot_center, rot_width, hight, pos );
    pos += 4;
    
    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
    uvrgb->q = 4096 ;
    uvrgb->f = 0x8fff ;
    uvrgb->r = colr ;
    uvrgb->g = colg ;
    uvrgb->b = colb ;
    uvrgb->a = 0 ;
    uvrgb++;
    
    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
    uvrgb->q = 4096 ;
    uvrgb->f = 0x8fff ;
    uvrgb->r = colr ;
    uvrgb->g = colg ;
    uvrgb->b = colb ;
    uvrgb->a = 0 ;
    uvrgb++;
    
    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
    uvrgb->q = 4096 ;
    uvrgb->f = 0x0fff ;
    uvrgb->r = colr ;
    uvrgb->g = colg ;
    uvrgb->b = colb ;
    uvrgb->a = 0 ;
    uvrgb++;
    
    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
    uvrgb->q = 4096 ;
    uvrgb->f = 0x0fff ;
    uvrgb->r = colr ;
    uvrgb->g = colg ;
    uvrgb->b = colb ;
    uvrgb->a = 0 ;
    uvrgb++;
    //	}
    
    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        4);
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        4);
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 4);
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 4);
    
    return 1;
    
}
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
#ifdef PSX2

#if 1 //BP_GCC
   DG_PRIM2_UVRGB	**uvrgb = STACK_ALLOC(ITA_NUM*TATE_DIV*sizeof(DG_PRIM2_UVRGB)) ;
#else
    DG_PRIM2_UVRGB	*uvrgb[ITA_NUM*TATE_DIV] ;
#endif

#endif

#ifdef KP_XBOX
	DG_PRIM2_UVRGB	**uvrgb, **p;
#endif

    FVECTOR		*fvtemp;
    FVECTOR		fvcal;
    float	change_start;
    float	change_end;
    float	len;
    int		ratio;
    int		clock;
    int		alpha,data_alpha;
    int		loop0;

#ifdef KP_XBOX
	p = malloc(sizeof(DG_PRIM2_UVRGB*) * ITA_NUM * TATE_DIV);
	uvrgb = p;
#endif

    
    
    clock = work->prim[0]->buffer_clock;
    for (loop0=0; loop0<ITA_NUM*TATE_DIV; loop0++){
	if( ok_flush_status!=0 ){			//！！！！！！要チェック ！！！！！
	    DG_InvisiblePrim2( work->prim[loop0] ) ;
	    
	}else{
	    DG_VisiblePrim2( work->prim[loop0] ) ;
	}
	DG_SwitchBuffPrim2( work->prim[loop0] );
    }
    
    
    if( ok_focus_z_far!=0.0f ){
	change_start = ok_focus_z_far - FADE_RANGE_NEAR;
	if(change_start < DRAW_Z_MIN) change_start = DRAW_Z_MIN;
	change_end   = ok_focus_z_far + FADE_RANGE_FAR;
	if(change_end   > DRAW_Z_MAX) change_end   = DRAW_Z_MAX;
    }else{
	/* キャラ：Ｚ制御が起動していないとき */
	change_start = DUMMY_Z - FADE_RANGE_NEAR;
	if(change_start < DRAW_Z_MIN) change_start = DRAW_Z_MIN;
	change_end   = DUMMY_Z + FADE_RANGE_FAR;
	if(change_end   > DRAW_Z_MAX) change_end   = DRAW_Z_MAX;
    }
    
    fvtemp = &work->center;
    
    
    for (loop0=0; loop0<ITA_NUM*TATE_DIV; loop0++){
	uvrgb[loop0]  = work->prim[loop0]->uvrgb[clock];
	
	_sceVu0SubVector( &fvcal, fvtemp, (FVECTOR *)DG_Chanls[0].eye.m[3] );
	len=GV_VecLen3F( &fvcal );
	
	if(len > change_end){
	    ratio=RATIO_MAX;
	}
	else if(len > change_start ){
	    ratio=(int)((len - change_start)*(float)RATIO_MAX/(float)(change_end - change_start));
	}
	else{
	    ratio=0;		/* フラグとしても使用 */
	}
	
	data_alpha = ratio/2;
	alpha=(data_alpha>0)?data_alpha:0;
	alpha=(data_alpha<255)?data_alpha:255;
	uvrgb[loop0]->a = alpha; uvrgb[loop0]++;
	uvrgb[loop0]->a = alpha; uvrgb[loop0]++;
	uvrgb[loop0]->a = alpha; uvrgb[loop0]++;
	uvrgb[loop0]->a = alpha; uvrgb[loop0]++;
    }
#ifdef KP_XBOX
	free(p);
#endif

}

static void Die( Work *work )
{
    int	loop0;
    for (loop0=0; loop0<ITA_NUM*TATE_DIV; loop0++){
	work->prim[loop0] = OK_FreePrim2( work->prim[loop0] );
    }
}

static int GetResources( Work *work )
{
    int			tex_name;
    FVECTOR		*center;
    DG_PRIM2		*prim ;
    DG_TEX		*tex=NULL ;
    FVECTOR		tmppos;
    int			loop0, loop1;
    int	colr, colg, colb;

    
    if ( GCL_GetOption( 't' ) != NULL ) {
	if( GCL_NextStr() != NULL ){
	    tex_name       = GCL_GetNextInt();
	    tex = DG_GetTexture( tex_name );
	}
    }

    if ( GCL_GetOption( 'r' ) != NULL ) {
	work->small_r = (float)GCL_GetNextInt() ;
	work->large_r = (float)GCL_GetNextInt() ;
    }

    if ( GCL_GetOption( 'h' ) != NULL ) {
	work->hight = (float)GCL_GetNextInt() ;
    }

    if ( GCL_GetOption( 'd' ) != NULL ) {
	work->devide = GCL_GetNextInt() ;
    }

    if ( GCL_GetOption( 'l' ) != NULL ) {
	work->lines = GCL_GetNextInt() ;
    }
    
    if ( GCL_GetOption( 'p' ) != NULL ) {
	center=&work->center;
	if( GCL_NextStr() != NULL ){
	    center->vx   = (float)GCL_GetNextInt() ;
	    center->vy   = (float)GCL_GetNextInt() ;
	    center->vz   = (float)GCL_GetNextInt() ;
	}
	
    }
    
    if ( GCL_GetOption( 'c' ) != NULL ) {
	colr   = GCL_GetNextInt() ;
	colg   = GCL_GetNextInt() ;
	colb   = GCL_GetNextInt() ;
    }
    else {
	colr   = 128;
	colg   = 128;
	colb   = 128;
    }
    
    tmppos.vx = work->center.vx;
    tmppos.vy = work->center.vy;
    tmppos.vz = work->center.vz;
    
    
    printf ("Begin Umi-Ex Setting\n");
    
    for (loop0=0; loop0<ITA_NUM; loop0++){
	FVECTOR tempvec = work->center;
	for (loop1=0; loop1<TATE_DIV; loop1++){
	    
	    
	    prim = work->prim[loop0*TATE_DIV+loop1] = 
		GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_FOG, 1, 4 );
	    if(prim==NULL){
		printf("						*********null prim\n");
		return -1;
	    }
	    InitPacket2( work, prim, tex, &tempvec,
			 work->small_r + ((work->large_r-work->small_r)*loop1/TATE_DIV),
			 work->small_r + ((work->large_r-work->small_r)*(loop1+1)/TATE_DIV),
			 work->hight/TATE_DIV,
			 loop0*M_PI*2/ITA_NUM, M_PI/ITA_NUM,
			 colr,colg,colb );
	    tempvec.vy += work->hight/TATE_DIV;	
	}
    }
    return 0 ;
}

void *NewExtraUmiSaraDraw( void )
{
    Work		*work ;
    
    OPERATOR() ;
    
    work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor )
	    if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	    }
    }
    return (void *)work ;
}





