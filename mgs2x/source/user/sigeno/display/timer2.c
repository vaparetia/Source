//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	timer2.c
	タイマー
	2001/09/17 K.Sigeno
	$Id: timer2.c,v 1.2 2002/12/05 18:41:57 takaki Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif


//#include	"def_dma.h"		/* 2002.02.09 エラーが出るので消しました */
#include	"gameheader.h"

#include "libfs.h"

#define TIME_FONT_MINI (824712)		//timefont_mini_alp_ovl
#define TIME_FONT	(8971472)	//timefont_alp_ovl
#define TIME_BG		(13247596)	//timebg_alp_ovl
#define TIME_COMMA	(3917197)	//comma_alp_ovl

//void	GM_SdSet( code )
//SD_S_COUNTDW1
//SD_S_COUNTRV1
//#define DEBUG_TIMER 1

#define RADAR_TRI (7429504)		//2D_tex.tri

#define SC_RATE_H		(0.8571F)

#define TIME_OFF_X 7 /* 時間部の左上余白*/
#define TIME_OFF_Y -1
#define TIME_OFF_Y_L 17


//#define BG_SIZE_W	(121.0f)
#define BG_SIZE_W	(110.0f*TARGET_ASPECT_X)

#define BG_SIZE_H	(30.5f)
#define BG_SIZE_W_SEC	(100.0f*TARGET_ASPECT_X) /*秒モード時のベース横幅*/


//#define BASE_OFF_X (375.0F)
#define BASE_OFF_X (15.0F)

#define BASE_OFF_Y (140.0F)
#if 0
#  ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
#    define BASE_OFF_Y2 163
#  else
#    define BASE_OFF_Y2 165
#  endif
#else
#  define BASE_OFF_Y2 ( BP_Area_EU() ? 165 : 163 )
#endif


#define MIN_OFF_X (5.0F)	/*現在未使用*/

#define LINE_X	(6.0F) /*BASEからの相対*/
#define LINE_Y	(15.0F)
//#define LINE_W	(105.0F)
//#define LINE_W	(94.0F)
#define LINE_W	(98.0F*TARGET_ASPECT_X)

#define LINE_H	(1.0F)


#define MIN_OFF_Y (3.0F)
#define PERC_OFF_Y (7.0F)

#define	FONT_WIDTH (18.0F)
#define	FONT_HEIGHT (18.0F)
#define	SPACE_WIDTH (1.0F)

#define COMMA_WIDTH (8.0F)
#define COMMA_HEIGHT (6.0F)

#define COMMA_SPACE_0 (5.0F)
#define COMMA_SPACE_1 (8.0F)

#define	PERC_FONT_WIDTH (14.0F)
#define	PERC_FONT_HEIGHT (14.0F)

extern int BP_FRAMES_PER_SEC();
#define SEC_FRAME (BP_FRAMES_PER_SEC())

#define MIN_FRAME (SEC_FRAME*60)
//356400
//3540
//59
//359999
#define COUNT_MAX (MIN_FRAME*99)
//3600*99+60*59+59

#define CNT_MAX_LIMIT (MIN_FRAME*99 + SEC_FRAME*59 + (SEC_FRAME-1) )


#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))

#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))

#define SET_COLOR_DEF(_prim) SET_COLOR_2DPRIM2(_prim,0x80,0x80,0x80,0x80);
#define SET_COLOR_WAR(_prim) SET_COLOR_2DPRIM2(_prim,0x80,0x10,0x10,0x80);
#define SET_COLOR_BG(_prim) SET_COLOR_2DPRIM2(_prim,0x00,0x00,0x00,0x80);
#define SET_COLOR_LINE(_prim) SET_COLOR_2DPRIM2(_prim,96,96,96,0x80);

#define SET_COLOR_DEF_ALP(_prim,_alp) SET_COLOR_2DPRIM2(_prim,0x80,0x80,0x80,_alp);

#define _ABSf(x) ((x>=0.0f)?(x):(-(x)))

#define ST_STOP			(0x01)
#define ST_INVISIBLE	(0x02)
#define ST_DESTOROY		(0x04)
#define ST_DISP_ZERO	(0x08)

/*内部ステータス*/
#define INT_SELECT_INV	(0x01)

#define INC_SPEED (19)
#define WAR_LEFT	DIRECT_TICK(60*10)

/*残り時間参照用*/
static	int	SigTimerLeft2 ;
static	int	SigTimerLeft3 ;
static	int TimerStatus2 ;

typedef struct {
	GV_ACT_EX	actor ;
	int			name ;
	int		tri_id ;
	SPR_OBJ		* empty;
	SPR_OBJ		* min[2] ;
	SPR_OBJ		* sec[2] ;
	SPR_OBJ		* perc[2] ;
	SPR_OBJ		* comma[2] ;
	SPR_OBJ		* bg ;
	SPR_OBJ		* line ;
	SPR_POS		sp_pos;
	int inter_s ; /*内部ステータス*/
	int disp_time ; /*表示上の数値*/
	int	mode;
	/*debug*/
	float	bg_size_w ; 
	float	bg_size_h ;
} Work;
enum {
	MSG_STOP = 0 ,
	MSG_START,
	MSG_INC,
	MSG_VISIBLE
};
enum {
	MODE_MIN = 0,
	MODE_SEC 
};



extern void SIG_PadPos(int pad,int mode,float *x,float *y,float def_sp,float rate) ;


#define MENU_MODE_OFF	(MENU_RADIO_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN)
//#define MENU_MODE_OFF	(MENU_RADIO_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN|MENU_WEAPON_OFF|MENU_ITEM_OFF|MENU_RADAR_OFF|MENU_GAGE_OFF|MENU_CAPTION_OFF|MENU_SUBWIN_OFF)



extern int gBP_HUD_DisableTimer;

static int CheckVisible(Work *work)
{
	if( (GM_CheckMenuStatus(MENU_MODE_OFF)) || (GV_PauseLevel & GV_LEVEL_NORMAL) || (TimerStatus2 & ST_INVISIBLE) || (work->inter_s & INT_SELECT_INV) || gBP_HUD_DisableTimer )
   {
//printf("GM_MenuStatus[%x]\n",GM_MenuStatus) ;
		return 1 ;
	}
	return 0;
}
static void TimerCountAdd(void){
	if((!(GV_PadData[0].flag & GV_PAD_RELEASE))&&(~TimerStatus2 & ST_STOP)
	){
		if(SigTimerLeft2 < CNT_MAX_LIMIT) {
			SigTimerLeft2++;
		}
		if(SigTimerLeft3 < CNT_MAX_LIMIT) {
			SigTimerLeft3++;
		}
	}
}

static int SprInit2(Work *work ){
//	int i;
//	SPR_POS		b_pos;

	/* 操作座標となる Empty オブジェクトを作成 */
	work->empty 
		= SPR_Create_2D_Object(SP_EMPTY, DG_CHANL_MENU, NULL) ;
	if(work->empty == NULL ) return -1 ;
	work->bg = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, work->empty);
	work->line = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, work->empty);
	SET_COLOR_LINE(work->line) ;
	SPR_SHOW(work->line);
	SPR_SetSizeSprite(work->line, LINE_W ,  LINE_H );
	SPR_SetSizeSprite(work->bg, BG_SIZE_W , BG_SIZE_H );
	work->sp_pos.x = LINE_X ;
	work->sp_pos.y = LINE_Y ;
	SPR_SetPosSprite(work->line,&work->sp_pos);
	work->bg_size_w = BG_SIZE_W ;
	work->bg_size_h = BG_SIZE_H ;

	work->bg->head.alpha =	SCE_GS_SET_ALPHA(0, 1, 2, 1, 64) ;
	work->bg->head.flags |= SPR_FLAG_ALPHA;

	SET_COLOR_BG(work->bg);
	SPR_SHOW(work->bg);
	SPR_SHOW(work->empty);

	work->sp_pos.x = BASE_OFF_X ;
	work->sp_pos.y = BASE_OFF_Y ;
	SPR_SetPosEmpty(work->empty, &work->sp_pos);
	work->sp_pos.x = 0.0F ;
	work->sp_pos.y = 0.0F ;
	SPR_SetPosSprite(work->bg,&work->sp_pos);
	/* スプライト表示位置の指定 */
	return 0;
}
static void PadCheck( Work *work ){
	if(GV_PauseLevel & GV_LEVEL_NORMAL) return ;
	if(GV_PadData[ 0].press & PAD_SEL){
//	if(GV_PadData[ 0].press & PAD_A){
		if(work->inter_s & INT_SELECT_INV) work->inter_s &= ~INT_SELECT_INV ;
		else work->inter_s |= INT_SELECT_INV ;
	}
}
static void Act( Work *work )
{
	int disp ;
	int min[2],sec[2],m_sec[2] ;

	if((GM_GameStatus & STATE_GAMEOVER)||(TimerStatus2 & ST_DESTOROY)){
		GV_DestroyActor(work) ;
		return ;
	}

#ifdef DEBUG_TIMER
	SIG_PadPos(1,0,&work->sp_pos.x,&work->sp_pos.y,1.0f,0.5f) ;
	SIG_PadPos(1,1,&work->bg_size_w,&work->bg_size_h,1.0f,0.5f) ;


//	work->sp_pos.x = BASE_OFF_X ;
//	work->sp_pos.y = BASE_OFF_Y ;
	SPR_SetPosEmpty(work->empty, &work->sp_pos);
	SPR_SetSizeSprite(work->bg, work->bg_size_w , work->bg_size_h );

	MENU_ResetColor();
	MENU_Locate( 100, 300, MENU_MODE_LEFT );
//	MENU_Locate( (int) work->sp_pos.x, (int)(work->sp_pos.y/(SC_RATE_H)), MENU_MODE_LEFT );
	MENU_Printf( " SPR_X [%f] SPR_Y [%f] ",work->sp_pos.x ,work->sp_pos.y);

	MENU_Locate( 100, 320, MENU_MODE_LEFT );
	MENU_Printf( " SPR_W [%f] SPR_H [%f] ",work->bg_size_w ,work->bg_size_h);
#endif

	PadCheck( work ) ;

	if(CheckVisible(work)){
		SPR_HIDE(work->empty);
#if 1
		if(GV_PauseLevel & (GV_LEVEL_STOP|GV_PAUSE_PAUSE)){
			/*	カウントも行わない条件をここに並べよう*/
		}else {
			/*メニューで消えている場合 カウントだけは行う*/
			TimerCountAdd() ;
		}
#endif
		return ;
	}else {
		SPR_SHOW(work->empty);
	}
	TimerCountAdd() ;
	work->disp_time = SigTimerLeft2 ;

	switch (work->mode) {
		case MODE_MIN :
		default :
			disp = (work->disp_time/MIN_FRAME) ;
			min[0] = disp/10 ;
			min[1] = disp%10 ;
			disp = (work->disp_time/SEC_FRAME) ;
			disp %= 60 ;
			sec[0] = disp/10 ;
			sec[1] = disp%10 ;
		break;
		case MODE_SEC :
			disp = (work->disp_time/SEC_FRAME) ;
			min[0] = disp/1000 ;
			min[1] = (disp%1000)/100 ;
			sec[0] = (disp%100)/10 ;
			sec[1] = disp%10 ;
		break;
	}

	disp = (work->disp_time%SEC_FRAME) ;
	disp = (disp*100)/SEC_FRAME ;
	m_sec[0] = disp/10 ;
	if(work->disp_time == CNT_MAX_LIMIT){
		m_sec[1] = 9 ;
	}else {
		m_sec[1] = disp%10 ;
	}

	MENU_Color( 180,180,180,0x80 );


	switch (work->mode) {
		case MODE_MIN :
		default :
#ifdef DEBUG_TIMER
			MENU_Locate( (int) (work->sp_pos.x+TIME_OFF_X), (int)(work->sp_pos.y/SC_RATE_H)+TIME_OFF_Y, MENU_MODE_LEFT );
#endif

	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X, DIRECT_SCREEN_Y(BASE_OFF_Y2+TIME_OFF_Y), MENU_MODE_LEFT );
//	MENU_Printf( "TIME LEFT");
	MENU_Printf( "TIME");
	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X, DIRECT_SCREEN_Y(BASE_OFF_Y2+TIME_OFF_Y+TIME_OFF_Y_L), MENU_MODE_LEFT );
	MENU_Printf( "%d%d '%d%d ''%d%d",min[0],min[1],sec[0],sec[1],m_sec[0],m_sec[1] );

			break;
		case MODE_SEC :
	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X, DIRECT_SCREEN_Y(BASE_OFF_Y2+TIME_OFF_Y), MENU_MODE_LEFT );
//	MENU_Printf( "TIME LIMIT");
	MENU_Printf( "TIME");

	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X, DIRECT_SCREEN_Y(BASE_OFF_Y2+TIME_OFF_Y+TIME_OFF_Y_L), MENU_MODE_LEFT );
	MENU_Printf( "%d%d%d%d''%d%d",min[0],min[1],sec[0],sec[1],m_sec[0],m_sec[1] );
			break;
	}
}

static void Die( Work *work )
{
	/*親を最後に*/
	SPR_Destroy_2D_Object( work->empty ) ;
	SPR_KillTexture(work->tri_id) ;
}
static int GetResources( Work *work, int name )
{
	int	cnt ;
	TimerStatus2 = 0 ;
	work->tri_id = SPR_LoadTexture(RADAR_TRI);
	work->name = name;
	cnt = GCL_GetOptionValue( 't', 0 ) ;

//	SigTimerLeft2 = DIRECT_TICK(cnt) ;
	SigTimerLeft2 = cnt ;
	work->disp_time = SigTimerLeft2 ;
	SigTimerLeft3 = 0 ;

	if ( GCL_GetOption( 's' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			TimerStatus2 = GCL_GetNextInt();
		}
	}else {
		TimerStatus2 = 0 ;
	}

	if ( GCL_GetOption( 'm' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->mode = GCL_GetNextInt();
		}
	}else {
		work->mode = MODE_MIN ;
	}
	if(SprInit2( work ) < 0) return -1 ;

	work->inter_s = 0 ;
	return 0;
}
void *NewTimer2( int name, int where )
{
	Work		*work;
//	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) );
//	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources( work,name )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
/*プログラム呼び出し版*/
/*

*/

int NewGetLeftTime2( void )
{
#if 0
    return (SigTimerLeft2*6)/5 ;
#else
    return SigTimerLeft2 ;
#endif

}
int NewGetLapTime( void )
{
#if 0
    return (SigTimerLeft3*6)/5 ;
#else
printf("NewGetLapTime [%d]\n",SigTimerLeft3);
//ASSERT(0) ;
    return SigTimerLeft3 ;
#endif

}

int NewAddLeftTime2( void )
{
	int time = 0;
    if ( GCL_GetOption( 't' ) != NULL ) {
		time = GCL_GetNextInt() ;
	}
	SigTimerLeft2 += DIRECT_TICK(time) ;
	return ( 0 );

}

void NewSetTimerStatus2( void )
{
    if ( GCL_GetOption( 's' ) != NULL ) {
		TimerStatus2 |= GCL_GetNextInt() ;
	}
}
void NewUnsetTimerStatus2( void )
{
    if ( GCL_GetOption( 's' ) != NULL ) {
		TimerStatus2 &= ~(GCL_GetNextInt()) ;
	}
}
int NewGetTimerStatus2( void )
{
	return TimerStatus2 ;
}

void NewTimerEnd2( void )
{
	TimerStatus2 |= ST_DESTOROY ;
}
void NewProgAddLeftTime2( int time )
{
	SigTimerLeft2 += DIRECT_TICK(time) ;

}
void NewProgSetLeftTime2( int time )
{
	SigTimerLeft2 = DIRECT_TICK(time) ;
}
