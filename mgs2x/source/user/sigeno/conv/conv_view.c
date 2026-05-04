//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"

#define BOX_WIDE (50.0f)

static void Zone2FVec( zone , vec )
HZX_ZON		*zone;
FVECTOR		*vec;
{
	vec->vx = (float) zone->x ;
	vec->vy = (float) zone->y ;
	vec->vz = (float) zone->z ;
}
static HZX_ZON *getzonefromzonenum(int addr){

	HZX_ZON		*zone;
	zone = HZX_GetGroupFromNo( HZX_ZoneMapNo( addr ) )->zones
		+HZX_Zone1(addr);
	return zone;
}

void SIG_NumPrintformat(FVECTOR *pos ,int disp,int mode)
{
#ifdef DEBUG_MODE
    float     tmp ;
    FVECTOR   ret ;   
    int       x,y ;   /* 表示座標 */
    DG_CHANL  *cp ;

    /* 位置決定 */
    cp = DG_Chanl(0) ;
    tmp = pos->vw ;
    pos->vw = 1.0F ;
    _sceVu0ApplyMatrix(&ret, &cp->eye_pers, pos) ;
    pos->vw = tmp ;

    /* カメラの範囲内に入っているかどうか */
    if(ret.vz > ret.vw)
    {
	return ;
    }
    if(ret.vw < 0) ret.vw = - ret.vw ;
    
    if((ret.vx > ret.vw) || (ret.vx < (-ret.vw)))
    {
	return ;
    }
    
    if((ret.vy > ret.vw) || (ret.vy < (-ret.vw)))
    {
	return ;
    }
    
    x = (int)((ret.vx / ret.vw) * (float)(DRAW_WIDTH>>1)) + (DRAW_WIDTH>>1) ;
    y = (int)((ret.vy / ret.vw) * (float)(DRAW_HEIGHT>>1)) + (DRAW_HEIGHT>>1) ;
    DEBUG_Locate(x,y,0) ;
    DEBUG_Color(200,200,200,0x80);
	switch( mode ){
		case 0:
		    DEBUG_Printf("%3d\n",disp) ;
		break ;
		case 1:
		    DEBUG_Printf("%3x\n",disp) ;
		break;
	}
#endif
}
void SIG_NumPrint(FVECTOR *pos ,int disp){
	SIG_NumPrintformat(pos ,disp,0) ;
}
void SIG_NumPrint2(FVECTOR *pos ,int disp){
	SIG_NumPrintformat(pos ,disp,1) ;
}

void	SigZoneView( int     addr,
					SVECTOR *rgb,
					float	 height )
{
#ifdef DEBUG_MODE
	HZX_ZON		*zone;
	FVECTOR		pos[4];
	u_char	r,g,b;
	int i,disp;
	
	if(rgb != NULL) {
		r = (u_char)rgb->vx;
		g = (u_char)rgb->vy;
		b = (u_char)rgb->vz;
	}else {
		r = 255 ;
		g = 255 ;
		b = 0 ;
	}
	if(HZX_Zone1(addr) == 255 ) return ;
	
	zone  = getzonefromzonenum(addr);


	for(i=0;i<4;i++){
		Zone2FVec( zone , &pos[i] );
	}
	pos[0].vy += (float)(height ) ;
	disp = zone->flag ;
//	disp = HZX_Zone1(addr) ;

	SIG_NumPrint2(&pos[0] ,disp) ;

	pos[0].vx -= (float)(zone->w) ;
//	pos[0].vy += (float)(height ) ;
	pos[0].vz -= (float)(zone->h) ;

	pos[1].vx += (float)(zone->w) ;
	pos[1].vy += (float)(height ) ;
	pos[1].vz -= (float)(zone->h) ;

	pos[2].vx += (float)(zone->w) ;
	pos[2].vy += (float)(height ) ;
	pos[2].vz += (float)(zone->h) ;

	pos[3].vx -= (float)(zone->w) ;
	pos[3].vy += (float)(height ) ;
	pos[3].vz += (float)(zone->h) ;

	NewSquareView( pos,1,r,g,b);
#endif
}
void PosBox(FVECTOR	*pos,float wide,SVECTOR	*rgb)
{
#ifdef DEBUG_MODE
	FVECTOR	v_pos[2];
	int i;
	u_char	r,g,b ;



	for(i=0;i<2;i++){
		v_pos[i] = *pos;

	}

	v_pos[0].vx -= wide ;
	v_pos[0].vy -= wide ;
	v_pos[0].vz -= wide ;

	v_pos[1].vx += wide ;
	v_pos[1].vy += wide ;
	v_pos[1].vz += wide ;

	if(rgb != NULL){
		r = (u_char) (rgb->vx&255);
		g = (u_char) (rgb->vy&255);
		b = (u_char) (rgb->vz&255);
	}else {
		r = 255 ;
		g = 0 ;
		b = 0 ;
	}	
	NewBoundingBoxView(&v_pos[0],&v_pos[1],r,g,b);
#endif
}
void PosBox2(FVECTOR	*pos,float wide,int sr,int sg,int sb)
{
#ifdef DEBUG_MODE
	FVECTOR	v_pos[2];
	int i;
	u_char	r,g,b ;



	for(i=0;i<2;i++){
		v_pos[i] = *pos;

	}

	v_pos[0].vx -= wide ;
	v_pos[0].vy -= wide ;
	v_pos[0].vz -= wide ;

	v_pos[1].vx += wide ;
	v_pos[1].vy += wide ;
	v_pos[1].vz += wide ;

	r = sr&255 ;
	g = sg&255 ;
	b = sb&255 ;

	NewBoundingBoxView(&v_pos[0],&v_pos[1],r,g,b);
#endif
}

/**/
void SIG_PosLine(FVECTOR *pos){
#ifdef DEBUG_MODE
	FVECTOR line[2] ;
	line[0] = line[1] =*pos ;
	line[1].vy += 250.0F ;
	NewLineView( line ,1,255,0,0) ;
#endif
}

void SIG_PosLine2(FVECTOR *pos,FVECTOR *pos2){
#ifdef DEBUG_MODE
	FVECTOR line[2] ;
	line[0] = *pos ;
	line[1] = *pos2 ;
	NewLineView( line ,1,255,0,0) ;
#endif
}

