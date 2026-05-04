//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	set_tex_f.c
	拡張海セット
	1999/11/09 S.Okajima
	$Id: umi_ex.c,v 1.1.1.3 2002/11/19 11:48:18 Yoshizawa1 Exp $

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

#define	DUMMY_Z			(5000.0f)
#define	RATIO_MAX		(48)
#define	RGB_0		(200)
#define	RGB_1		(210)

/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR		(8000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR		(-4000.0f)

#define	N_POLYS		(16)
#define	N_VERTS		(N_POLYS * 4)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#if 0
#define	LINES			(2)
#define LENGS			(6+LINES*2)
#define	PRIM_NUM		((LENGS+6)*LINES*2)
#else
#define	LINES			(work->Lines)
#define LENGS			(6+LINES*2)
#define	PRIM_NUM		((LENGS+6)*LINES*2)
#endif

/*----------------------------------------------------------------*/
typedef	struct	{
    GV_ACT_EX	actor ;
    
    //DG_PRIM2	*prim[PRIM_NUM] ;
    DG_PRIM2	*prim[220];	// LINES 5まで
    FVECTOR	center;
    
    int		Lines;        
    float	width ;
    float	height ;
    
} Work ;


/*----------------------------------------------------------------*/
// verts は FVECTOR ４つ分作成
static void MakeRotationVertics( FVECTOR *center, float w, float h, FVECTOR *verts )
{
    SVECTOR tmprot = {1024, 0, 0, 0};
    verts[0].vx = -w;
    verts[0].vy = -h;
    verts[0].vz = 0.0f;
    verts[1].vx =  w;
    verts[1].vy = -h;
    verts[1].vz = 0.0f;
    verts[2].vx = -w;
    verts[2].vy =  h;
    verts[2].vz = 0.0f;
    verts[3].vx =  w;
    verts[3].vy =  h;
    verts[3].vz = 0.0f;
    
    DG_SetPos2( center, &tmprot);
    DG_PutVector( verts, verts, 4 );
}
/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, float width, float height,
			int colr, int colg, int colb )
{
    FVECTOR			*pos ;
    DG_PRIM2_UVRGB		*uvrgb ;
    
    DG_ConfigPrim2Tex( prim, tex );
    //	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
    
    pos   = SCR_POS ;
    uvrgb = SCR_UVS ;
    
    
    MakeRotationVertics( center, width, height, pos );
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
   DG_PRIM2_UVRGB	** uvrgb = STACK_ALLOC(PRIM_NUM*sizeof(DG_PRIM2_UVRGB)) ;
#else
    DG_PRIM2_UVRGB	*uvrgb[PRIM_NUM] ;
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
	p = malloc(sizeof(DG_PRIM2_UVRGB*) * PRIM_NUM);
	uvrgb = p;
#endif
    
    
    clock = work->prim[0]->buffer_clock;
    for (loop0=0; loop0<PRIM_NUM; loop0++){
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
    
    
    for (loop0=0; loop0<PRIM_NUM; loop0++){
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
    for (loop0=0; loop0<PRIM_NUM; loop0++){
	work->prim[loop0] = OK_FreePrim2( work->prim[loop0] );
    }
}

static int GetResources( Work *work )
{
    int		tex_name;
    FVECTOR		*center;
    DG_PRIM2	*prim ;
    DG_TEX		*tex=NULL ;
    FVECTOR		tmppos;
    int	loop0, loop1, loop2, loop3;
    int	colr, colg, colb;
    
    
    if ( GCL_GetOption( 't' ) != NULL ) {
	if( GCL_NextStr() != NULL ){
	    tex_name       = GCL_GetNextInt();
	    tex = DG_GetTexture( tex_name );
	}
    }
    
    work->width  = (float)30000;
    work->height = (float)30000;
    
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
    
    
    if ( GCL_GetOption( 'l' ) != NULL ) {
	work->Lines = GCL_GetNextInt(); 
    }
    else {
	work->Lines = 2;
    }
    
    
    
    
    tmppos.vx = work->center.vx + 210000.0f;
    work->center.vy -= 100.0f;
    tmppos.vz = work->center.vz - 60000.0f*(2.5f+LINES);
    
    
    printf ("Begin Umi-Ex Setting\n");
    
    for (loop3=0; loop3<LINES; loop3++){
	for (loop2=1; loop2<=6; loop2++){
	    work->center.vx = tmppos.vx - (60000.0f*loop2);
	    prim = work->prim[loop2-1+loop3*(6)] = 
		GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_FOG, 1, 4 );
	    if(prim==NULL){
		printf("						*********null prim\n");
		return -1;
	    }
	    work->center.vz = tmppos.vz + 60000.0f*loop3;
	    InitPacket2( work, prim, tex, &work->center, work->width, work->height, colr,colg,colb );
	    
	    printf ("1:%3d   %7.0f %7.0f %7.0f\n",
		    loop2-1+loop3*(6), 
		    work->center.vx, work->center.vy, work->center.vz); 
	}
    }
    
    printf("\n");	
    
    for (loop3=0; loop3<LINES; loop3++){
	for (loop2=1; loop2<=6; loop2++){
	    work->center.vx = tmppos.vx - (60000.0f*loop2);
	    prim = work->prim[loop2-1+(6)*(LINES+loop3)] = 
		GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_FOG, 1, 4 );
	    if(prim==NULL){
		printf("						*********:null prim\n");
		return -1;
	    }
	    work->center.vz = tmppos.vz + 60000.0f*(6+LINES+loop3);
	    InitPacket2( work, prim, tex, &work->center, work->width, work->height, colr,colg,colb );
	    
	    printf ("2:%3d   %7.0f %7.0f %7.0f\n",
		    loop2-1+(6)*(LINES+loop3), 
		    work->center.vx, work->center.vy, work->center.vz); 
	    
	}
    }
    
    
    for (loop1=0; loop1<LINES; loop1++){
	
	printf("\n");
	
	work->center.vx = tmppos.vx + (60000.0f*(loop1));
	for (loop0=0; loop0<LENGS; loop0++){
	    prim = work->prim[(6)*LINES*2 + loop0+LENGS*loop1*2] = 
		GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_FOG, 1, 4 );
	    if(prim==NULL){
		printf("						*********null prim\n");
		return -1;
	    }
	    work->center.vz = tmppos.vz + 60000.0f*loop0;
	    InitPacket2( work, prim, tex, &work->center, work->width, work->height, colr,colg,colb );
	    
	    printf ("3:%3d   %7.0f %7.0f %7.0f\n",
		    (6)*LINES*2 + loop0+LENGS*loop1*2,
		    work->center.vx, work->center.vy, work->center.vz); 
	}
	
	printf("\n");
	
	work->center.vx = tmppos.vx - 210000.0f*2 - (60000.0f*(loop1));
	for (loop0=0; loop0<LENGS; loop0++){
	    prim = work->prim[(6)*LINES*2 + loop0+LENGS*loop1*2 + LENGS] = 
		GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_FOG, 1, 4 );
	    if(prim==NULL){
		printf("						*********null prim\n");
		return -1;
	    }
	    work->center.vz = tmppos.vz + 60000.0f*loop0;
	    InitPacket2( work, prim, tex, &work->center, work->width, work->height, colr,colg,colb );
	    
	    printf ("4:%3d   %7.0f %7.0f %7.0f\n",
		    (6)*LINES*2 + loop0+LENGS*loop1*2 + LENGS,
		    work->center.vx, work->center.vy, work->center.vz); 
	}
	
	
    }
    
    return 0 ;
}

void *NewExtraUmiDraw( void )
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







