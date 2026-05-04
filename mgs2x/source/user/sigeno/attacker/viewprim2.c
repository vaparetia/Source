/*
	viewprim2.c

	1999/10/21 K.Sigeno
	$Id: viewprim2.c,v 1.1.1.3 2002/11/19 11:49:05 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

#include	"at_enum.h"
#include "at_thk.h"

//extern HZX_ZON *ENE_HZX_GetZone(int addr);

static HZX_ZON *SIG_HZX_GetZone(int addr){

	HZX_HDL	*hzx_hdl ;		
	HZX_GRP *grp;
	HZX_ZON		*zone;
	int	mapno ;

	hzx_hdl = HZX_GetCurrentHzx();
	grp = hzx_hdl->grp;
	mapno = HZX_ZoneMapNo( addr ) ;
	grp += mapno ;

	zone = grp->zones+HZX_Zone1(addr);

	return zone;
}

#define PRIM_NUM 4
typedef	struct	{
	GV_ACT		actor ;
	int addr;
	int time;
	int type;
	HZX_HDL *hzx;
	HZX_GROUP_ID hzx_id;
	DG_PRIM		*prim;
	FVECTOR		primpos[PRIM_NUM][4]; 
} Work ;

#if 0
	HZX_GRP *grp;
	int mapno, add_mapno;
	grp = HZX_GetGroup( hzx_id );
	num_zones = grp->n_zones;
#endif

static inline void Prim_RGBA_GRAD( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly,   0,    0, 127, alpha );
	DG_SET_RGBA2( poly,   0,  127,   0, alpha );
	DG_SET_RGBA3( poly, 127,    0,   0, alpha );
	DG_SET_RGBA4( poly,   0,  127, 127, alpha );
}
static inline void Prim_RGBA_RED( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly, 127,    0,   0, alpha );
	DG_SET_RGBA2( poly, 127,    0,   0, alpha );
	DG_SET_RGBA3( poly, 127,    0,   0, alpha );
	DG_SET_RGBA4( poly, 127,    0,   0, alpha );
}
static inline void Prim_RGBA_BLUE( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly, 0,    0,  127, alpha );
	DG_SET_RGBA2( poly, 0,    0,  127, alpha );
	DG_SET_RGBA3( poly, 0,    0,  127, alpha );
	DG_SET_RGBA4( poly, 0,    0,  127, alpha );
}
static inline void Prim_RGBA_WHITE( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly, 127,  127,  127, alpha );
	DG_SET_RGBA2( poly, 127,  127,  127, alpha );
	DG_SET_RGBA3( poly, 127,  127,  127, alpha );
	DG_SET_RGBA4( poly, 127,  127,  127, alpha );
}
static inline void Prim_RGBA_YELLOW( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly, 127,  127,   0, alpha );
	DG_SET_RGBA2( poly, 127,  127,   0, alpha );
	DG_SET_RGBA3( poly, 127,  127,   0, alpha );
	DG_SET_RGBA4( poly, 127,  127,   0, alpha );
}
static inline void Prim_RGBA_BLACK( DG_POLY_G4 *poly ,int alpha){
	DG_SET_RGBA1( poly,  0,    0,   0, alpha );
	DG_SET_RGBA2( poly,  0,    0,   0, alpha );
	DG_SET_RGBA3( poly,  0,    0,   0, alpha );
	DG_SET_RGBA4( poly,  0,    0,   0, alpha );
}
#define ZONE_H  1500.0F
#define ZONE_L  500.0F

static void SetZonePrim(Work *work,int prim_num,int address,int type)
{
	HZX_ZON		*zone,tmpzone ; 
	FVECTOR		trgpos;
	int i ;
	float high = ZONE_H;

	DG_PRIM_PACKET *packet ;
	DG_POLY_G4 *poly ;

	packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[DG_Clock] );
	poly = (DG_POLY_G4*)packet->prim_top ;

	prim_num *= 4;
	for(i=0;i<prim_num;i++,poly++);
//	switch(type){
	switch(type){
		case 0:
			high = ZONE_L;
			for(i=0;i<4;i++){
//				Prim_RGBA_RED( poly ,127);			
				Prim_RGBA_GRAD( poly ,127);			
				poly++;
			}
			break;
		case 1:
			high = ZONE_H;
			for(i=0;i<4;i++){
				Prim_RGBA_BLUE( poly ,127);			
				poly++;
			}
			break;
	}
	DG_VisiblePrim( work->prim );
	if(HZX_Zone1(address) == 255){

		tmpzone.x = 0;
		tmpzone.y = 0;
		tmpzone.z = 0;

		tmpzone.w = 0;
		tmpzone.h = 0;

		zone = &tmpzone;

	}else {
//		zone = HZX_GetZone(hzx_id, address ) ;
		zone = SIG_HZX_GetZone(address) ;
	}
	trgpos.vx = zone->x;
	trgpos.vy = zone->y;
	trgpos.vz = zone->z;
/*DIST TEST*/
#if 0
for(i=0;i<4;i++){
	printf("NEAR NUM %d DIST %d\n",zone->nears[i],zone->dists[ i ]);
}
#endif
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		if(i&1)	work->primpos[prim_num][i].vx += (float)(zone->w);
		else	work->primpos[prim_num][i].vx -= (float)(zone->w);
		if(i<2)	work->primpos[prim_num][i].vz -= (float)(zone->h);
		else	work->primpos[prim_num][i].vz += (float)(zone->h);
		work->primpos[prim_num][i].vy  += high;
	}
	prim_num++;
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		work->primpos[prim_num][i].vx -= (float)(zone->w);
		if(i&1)	work->primpos[prim_num][i].vz += (float)(zone->h);
		else	work->primpos[prim_num][i].vz -= (float)(zone->h);
		if(i<2)	work->primpos[prim_num][i].vy += high;
	}
	prim_num++;
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		if(i&1)	work->primpos[prim_num][i].vx += (float)(zone->w);
		else	work->primpos[prim_num][i].vx -= (float)(zone->w);
		if(i<2)	work->primpos[prim_num][i].vy += high;
		work->primpos[prim_num][i].vz += (float)(zone->h);
	}
	prim_num++;
	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		work->primpos[prim_num][i].vx += (float)(zone->w);

		if(i&1)	work->primpos[prim_num][i].vz -= (float)(zone->h);
		else	work->primpos[prim_num][i].vz += (float)(zone->h);
		if(i<2)	work->primpos[prim_num][i].vy += high;
	}
}
static void FlatZonePrim(Work *work,int prim_num,int address)
	{
	HZX_ZON		*zone,tmpzone ; 
	FVECTOR		trgpos;
	int i;
	

	DG_VisiblePrim( work->prim );

	if(address == 255){

		tmpzone.x = 0;
		tmpzone.y = 0;
		tmpzone.z = 0;

		tmpzone.w = 0;
		tmpzone.h = 0;

		zone = &tmpzone;

	}else {
//		zone = HZX_GetZone(hzx_id, address ) ;
		zone = ENE_HZX_GetZone(address) ;
	}

		trgpos.vx = zone->x;
		trgpos.vy = zone->y;
		trgpos.vz = zone->z;

	for(i=0;i<4;i++){
		work->primpos[prim_num][i] = trgpos;
		if(i&1)	work->primpos[prim_num][i].vx += (float)(zone->w);
		else	work->primpos[prim_num][i].vx -= (float)(zone->w);
		if(i<2)	work->primpos[prim_num][i].vz -= (float)(zone->h);
		else	work->primpos[prim_num][i].vz += (float)(zone->h);
		work->primpos[prim_num][i].vy  += ZONE_H;
	}
}







static void Act( Work *work)
{
//GM_PlayerControl
//	DG_PRIM.group_id ; 表示グループ

//	work->prim->group_id = GM_PlayerBody->objs->group_id;
//	GM_PlayerBody->objs->group_id = work->prim->group_id ;

	SetZonePrim(work,0,work->addr,work->type);
	if(work->time > 0 ) work->time--;
	if(work->time <= 0) GV_DestroyActor( work ) ;
}


static void InitPacket( DG_PRIM_PACKET *packet, int n, int which,int mode ,int color)
{
	int	i ;
	DG_POLY_G4 *poly ;


	poly = (DG_POLY_G4*)packet->prim_top ;
	switch(0){
		case 0: 
			*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,2,0,1,0 ) ;
			break;
		case 1: 
			*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,1,0,1,0 ) ;
			break;
		case 2: 
			*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,1,0,2,0 ) ;
			break;
	}

	packet->gif_tag.PRIM |= SCE_GS_PRIM_ABE ;
	packet->gif_tag.PRE = 1 ;
	packet->gif_tag.REGS0 = GS_REGS_PRIM ;


	for ( i = n ; i > 0 ; i-- ){
		DG_SET_POLYG4( poly, 1 );
		switch(mode){
			case 0:
				Prim_RGBA_GRAD(poly , 127);
				break;
			case 1:
				Prim_RGBA_RED(poly , 127);
				break;
			case 2:
				Prim_RGBA_BLUE(poly , 127);
				break;
			case 3:
				Prim_RGBA_WHITE(poly , 127);
				break;
			case 4:
				Prim_RGBA_BLACK( poly ,0);
				break;
		}
		poly++ ;
	}
}
//#define POLY_PRIM (DG_PRIM_POLY_G4|DG_PRIM_VISIBLE|DG_PRIM_ON_WORLD|DG_PRIM_VERTICES)
#define POLY_PRIM (DG_PRIM_POLY_G4|DG_PRIM_INVISIBLE|DG_PRIM_ON_WORLD|DG_PRIM_VERTICES)

static void InitPrim( Work *work,int mode,int color)
{
	int i,j;
	DG_PRIM_PACKET *packet ;

//	work->prim = GM_MakePrim( POLY_PRIM, 1,PRIM_NUM, work->primpos, NULL ) ;
	work->prim = GM_MakePrim2( POLY_PRIM, 1,PRIM_NUM, work->primpos, NULL ) ;//	work->prim = DG_MakePrim( POLY_PRIM, 1,PRIM_NUM,0, work->primpos, NULL ) ;

	for ( j = 0 ; j < 2 ; j++ ){
		for ( i = 0 ; i < 1 ; i++ ){
			packet = (DG_PRIM_PACKET*)( (int)work->prim->packs[j] + work->prim->packet_size * i );
			InitPacket( packet, PRIM_NUM, j ,mode ,color);
		}
	}

}
static void Die( Work *work )
{
	GM_FreePrim( work->prim );
}

static int GetResources( Work *work)
{
//    MAP			*map ;

	InitPrim( work ,work->type,0);
//	map = GM_GetMap( GM_CurrentMap ) ;
	/*要修正 2000.01.13*/
}

/*---------------------------------------------------------------*/
void *NewZoneViewer( addr, time ,type)
int	addr ;
int	time ;
int	type ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), SUBCOMM_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->addr =addr;
		work->time =time;
		work->type =type;
	}
	return (void *)work ;
}


