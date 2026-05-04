//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	timer.c
	タイマー
	2000/12/15 K.Sigeno
	$Id: timer.c,v 1.2 2002/12/05 18:41:58 takaki Exp $
*/
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//#include	"def_dma.h"		/* 2002.02.09 エラーが出るので消しました */
#include	"gameheader.h"

#include "libfs.h"


#include "BP_Misc.h"

#define TIME_FONT_MINI (824712)		//timefont_mini_alp_ovl
#define TIME_FONT	(8971472)	//timefont_alp_ovl
#define TIME_BG		(13247596)	//timebg_alp_ovl
#define TIME_COMMA	(3917197)	//comma_alp_ovl
#define BOMB_LEFT	(13460485)			//bomleft_alp_ovl

//void	GM_SdSet( code )
//SD_S_COUNTDW1
//SD_S_COUNTRV1
//#define DEBUG_TIMER 1

#if 0
/*保留 レーダーの上部にWARNINGだすつもりだったらしい。*/
#define TIME_WAR	(13667457)	//alt_warn_alp_ovl
#define TIME_WAR2	(15491870)	//alt_warn2_alp_ovl
#endif

#define RADAR_TRI (7429504)		//2D_tex.tri

#define SC_RATE_H		(0.8571F)

#if 0
#  ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
#    define BASE_OFF_Y2 163
#  else
#    define BASE_OFF_Y2 165
#  endif
#else
#  define BASE_OFF_Y2 ( 173+9 )
#endif

#ifdef PSX2
#define TIME_OFF_X 7 /* 時間部の左上余白*/
#else
#define TIME_OFF_X 122 /* 時間部の左上余白*/
#endif
#define TIME_OFF_Y (-2)
#define TIME_OFF_Y_L 18

//DIRECT_SCREEN_Y
#define BG_SIZE_W	(121.0f*TARGET_ASPECT_X)
#define BG_SIZE_H	(30.5f)
#define BG_SIZE_W_SEC	(100.0f) /*秒モード時のベース横幅*/

#define BG_SIZE_W_BOMB	(106.0f*TARGET_ASPECT_X)
#define BG_SIZE_H_BOMB	(28.0f)

#define BASE_OFF_X (382.0F)
#define BASE_OFF_Y (155.0F)
//#define BASE_OFF_BOMB_Y (160.0F)

#define MIN_OFF_X (5.0F)	/*現在未使用*/

#define LINE_X	(6.0F) /*BASEからの相対*/
#define LINE_Y	(15.0F)
#define LINE_W	(105.0F*TARGET_ASPECT_X)
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


#define COUNT_MAX (MIN_FRAME*99)


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
#define INT_ST_INC	(0x01)
#define INT_ST_BOM	(0x02)

#define INC_SPEED (19)
#define WAR_LEFT	DIRECT_TICK(60*10)

/*残り時間参照用*/
static	int	SigTimerLeft ;
static	int Dsip_bomb ;
static	int TimerStatus ;

typedef struct {
	GV_ACT_EX	actor ;
	int			name ;
	int		tri_id ;
	int			proc_id ;
	SPR_OBJ		* empty;
	SPR_OBJ		* min[2] ;
	SPR_OBJ		* sec[2] ;
	SPR_OBJ		* perc[2] ;
	SPR_OBJ		* comma[2] ;
	SPR_OBJ		* bg ;
	SPR_OBJ		* line ;
	SPR_POS		sp_pos;
//	int time ;
//	int status ;
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
	MODE_SEC ,
	MODE_BOMBSEC
};


#ifdef DEBUG_MODE
#include	"debugmenu.h"

#define DEBUG_TMR_NOP (0x0000)
#define DEBUG_TMR_5SEC (0x0001)
static int timer_debug_flag;
#ifdef PSX2
static GM_DEBUG_MENU debug_menu = {
	class:	"TIMER",
	menu:	"LEFT TIME",
	max:    2,
	items:  (char * []){ "NORMAL", "5 SECS" },
	values: (int []){ DEBUG_TMR_NOP, DEBUG_TMR_5SEC },
	target: &timer_debug_flag,	// intの変数へのポインタ。代入される。
	mask:   0x00000001,
	type:   0,
} ;
#endif

static void TimerDebugSet(void){
#ifdef PSX2
	GM_AddDebugMenu(&debug_menu);
#endif

	timer_debug_flag = 0 ;
}
static void CheckDebugMenu(void){
	if(timer_debug_flag & DEBUG_TMR_5SEC ){
		SigTimerLeft = DIRECT_TICK(60*5) ;
		timer_debug_flag &= ~DEBUG_TMR_5SEC ;
	}
}
#endif

extern void SIG_PadPos(int pad,int mode,float *x,float *y,float def_sp,float rate) ;

#if 0
static int RAD_Fix2Int(SPR_FIX fix){
	int res ;
	res = (int)(fix>>4) ;
	return res ;
}
#endif

#define MENU_MODE_OFF	(MENU_RADIO_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN)
//#define MENU_MODE_OFF	(MENU_RADIO_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN|MENU_WEAPON_OFF|MENU_ITEM_OFF|MENU_RADAR_OFF|MENU_GAGE_OFF|MENU_CAPTION_OFF|MENU_SUBWIN_OFF)

extern int gBP_HUD_DisableTimer;

static int CheckVisible(Work *work)
{
	if( (GM_CheckMenuStatus(MENU_MODE_OFF) || (GV_PauseLevel & GV_LEVEL_NORMAL) || (TimerStatus & ST_INVISIBLE)) || gBP_HUD_DisableTimer )
	{
//printf("GM_MenuStatus[%x]\n",GM_MenuStatus) ;
		return 1;
	}
	return 0;
}

/*テクスチャWHにスプライトサイズを合わせる*/
#if 0
static void SetTexSizeSpr(SPR_OBJ * sprite){
	float w,h;
	w = (float) (RAD_Fix2Int(sprite->sprite.head.tex.w)+1) ;
	h = (float) (RAD_Fix2Int(sprite->sprite.head.tex.h)+1) ;
printf("tex w %f\n",w);
printf("tex h %f\n",h);
	SPR_SetSizeSprite(sprite, w, h);
}
#endif
#if 0
static void SetWHSpr(SPR_OBJ * sprite,float w,float h){
	SPR_FIX dw, dh;      /* 表示上のサイズ     */
	dw = SPR_FIXED(w) ;
	dh = SPR_FIXED(h) ;
	sprite->sprite.head.tex.w = dw ;
	sprite->sprite.head.tex.h = dh ;
}
#endif
#if 0
static void SetUVSpr(SPR_OBJ * sprite,float u){
	SPR_FIX du ;      /* 表示上のサイズ     */
	du = SPR_FIXED(u) ;
	sprite->sprite.head.tex.u = du ;
//	sprite->sprite.head.tex.v = dv ;
}
#endif
#if 0
static void TimeUVSet(SPR_OBJ **spr,float width,int time){
	SPR_OBJ **sprite ;
	float w ;
	sprite = spr;
	w = (float) (time/10) ;
	SetUVSpr(*sprite,w*width);
	sprite++;
	w = (float) (time%10) ;
	SetUVSpr(*sprite,w*width);
//	w = (float) (time/10) ;
//	SetUVSpr(work->min[1],w*FONT_WIDTH,0.0F);
}
#endif
#define ALPHA_HALF (SCE_GS_SET_ALPHA(0,1,0,1,0x80))
#if 0
static void TimerSprInit(Work *work,SPR_OBJ **spr ,int tex_code ,float w ,float h){
	*spr = SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, work->empty);
		/* テクスチャを指定する */
	SPR_ObjSetTexture(*spr, tex_code, work->tri_id);
	(*spr)->head.alpha =	ALPHA_HALF ;
	(*spr)->head.flags |= SPR_FLAG_ALPHA;

	SET_COLOR_DEF(*spr);
	SPR_SetSizeSprite(*spr, w,h);
	SetWHSpr(*spr,w, h);
	SPR_SHOW(*spr);
}
#endif
#if 0
static void SetAllNumWar(Work *work){
	int i ;
	for(i=0;i<2;i++){
		SET_COLOR_WAR(work->min[i]);
		SET_COLOR_WAR(work->sec[i]) ;
		SET_COLOR_WAR(work->perc[i]) ;
		SET_COLOR_WAR(work->comma[i]) ;
	}
}
static void SetAllNumDef(Work *work){
	int i ;
	for(i=0;i<2;i++){
		SET_COLOR_DEF(work->min[i]) ;
		SET_COLOR_DEF(work->sec[i]) ;
		SET_COLOR_DEF(work->perc[i]) ;
		SET_COLOR_DEF(work->comma[i]) ;
	}
}
static int SprInit(Work *work ){
	int i;
//	SPR_POS		b_pos;

	/* 操作座標となる Empty オブジェクトを作成 */
	work->empty 
		= SPR_Create_2D_Object(SP_EMPTY, DG_CHANL_MENU, NULL) ;
	if(work->empty == NULL ) return -1 ;

	/*以下、Emptyオブジェクトを最上位の親として、
	その下に各種オブジェクトを作成する。*/
	for(i=0;i<2;i++){
		TimerSprInit(work,&work->min[i],TIME_FONT ,FONT_WIDTH, FONT_HEIGHT);
		TimerSprInit(work,&work->sec[i],TIME_FONT ,FONT_WIDTH, FONT_HEIGHT);
		TimerSprInit(work,&work->perc[i],TIME_FONT_MINI ,PERC_FONT_WIDTH,PERC_FONT_HEIGHT);
		TimerSprInit(work,&work->comma[i],TIME_COMMA ,COMMA_WIDTH, COMMA_HEIGHT);
	}
	work->bg = 
		SPR_Create_2D_Object(SP_SPRITE, DG_CHANL_MENU, work->empty);
	SPR_ObjSetTexture(work->bg, TIME_BG, work->tri_id);
	SetTexSizeSpr(work->bg) ;
	work->bg->head.alpha =	ALPHA_HALF ;
	work->bg->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_DEF(work->bg);
	SPR_SHOW(work->bg);
	SPR_SHOW(work->empty);
	SetUVSpr(work->comma[1],COMMA_WIDTH);

	work->sp_pos.x = BASE_OFF_X ;
	work->sp_pos.y = BASE_OFF_Y ;
	SPR_SetPosEmpty(work->empty, &work->sp_pos);
	work->sp_pos.x = 0.0F ;
	work->sp_pos.y = 0.0F ;
	SPR_SetPosSprite(work->bg,&work->sp_pos);
	/* スプライト表示位置の指定 */
	work->sp_pos.x = MIN_OFF_X ;
	work->sp_pos.y = MIN_OFF_Y ;
	SPR_SetPosSprite(work->min[0],&work->sp_pos);
	work->sp_pos.x += FONT_WIDTH+SPACE_WIDTH ;
	SPR_SetPosSprite(work->min[1],&work->sp_pos);
	work->sp_pos.x += FONT_WIDTH+SPACE_WIDTH ;
	SPR_SetPosSprite(work->comma[0],&work->sp_pos);
	work->sp_pos.x += COMMA_SPACE_0 ;
	SPR_SetPosSprite(work->sec[0],&work->sp_pos);
	work->sp_pos.x += FONT_WIDTH+SPACE_WIDTH ;
	SPR_SetPosSprite(work->sec[1],&work->sp_pos);
	work->sp_pos.x += FONT_WIDTH+SPACE_WIDTH ;
	SPR_SetPosSprite(work->comma[1],&work->sp_pos);
	work->sp_pos.x += COMMA_SPACE_1 ;
	work->sp_pos.y = PERC_OFF_Y ;
	SPR_SetPosSprite(work->perc[0],&work->sp_pos);
	work->sp_pos.x += PERC_FONT_WIDTH+SPACE_WIDTH ;
	SPR_SetPosSprite(work->perc[1],&work->sp_pos);
	return 0;
}
#endif

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

#if 1
static void CheckMesg(Work *work){
	GV_MSG *msg;
	int n;
	if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for( ; n > 0; n-- ){
			switch( msg->message[ 0 ] ) {
				case MSG_STOP :
					TimerStatus |= ST_STOP ;
					break ;
				case MSG_START :
					TimerStatus &= ~ST_STOP ;
					break ;
				case MSG_INC :
//					work->time += DIRECT_TICK(msg->message[ 1 ]) ;
					work->inter_s |= INT_ST_INC ;
					SigTimerLeft += DIRECT_TICK(msg->message[ 1 ]) ;
					break ;
				case MSG_VISIBLE :
					if(msg->message[ 1 ] == 0){
						/*消す*/
						TimerStatus |= ST_INVISIBLE ;
					}else if(msg->message[ 1 ] == 1){
						/*出す*/
						TimerStatus &= ~ST_INVISIBLE ;
					}
					break ;
			}
			msg++;
		}
	}
}
#endif
static void Act( Work *work )
{
	int disp ;
	int min[2],sec[2],m_sec[2] ;

	if((GM_GameStatus & STATE_GAMEOVER)||(TimerStatus &ST_DESTOROY)){
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

#ifdef DEBUG_MODE
	CheckDebugMenu() ;
#endif
	CheckMesg(work) ;
	if(CheckVisible(work)){
		SPR_HIDE(work->empty);
		return ;
	}else {
		SPR_SHOW(work->empty);
	}
	if(~TimerStatus & ST_STOP){
		if(work->inter_s & INT_ST_INC){
			if((work->disp_time + INC_SPEED)< SigTimerLeft){
				work->disp_time += INC_SPEED;
				if((DG_TickCount%3)==0){
					GM_SdSet( SD_S_COUNTRV1 ) ;
				}
			}else if(work->disp_time < SigTimerLeft){
				work->disp_time = SigTimerLeft ;
			}else {
				/*加算モード終了*/
				work->inter_s &= ~INT_ST_INC ;
			}
		}else if(SigTimerLeft > 0){
			SigTimerLeft--;
		}
	}

	if(SigTimerLeft == 0){
		if((!(GM_GameStatus & STATE_GAMEOVER))&&( work->proc_id != NULL )){
			GM_ExecProc( work->proc_id, NULL );
		}
		work->proc_id = NULL ;
//		SigTimerLeft--;
	}

	if(!(work->inter_s & INT_ST_INC)){
		work->disp_time = SigTimerLeft ;
	}
	if(!(TimerStatus & ST_DISP_ZERO)){
		if(SigTimerLeft == 0) {
			TimerStatus |= ST_INVISIBLE ;
			return ;
		}
	}
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
		case MODE_BOMBSEC :
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
	m_sec[1] = disp%10 ;


	if(
	((WAR_LEFT > SigTimerLeft)||(work->mode ==MODE_BOMBSEC))
	&&((disp > 50)||(disp ==0))
	&&(~work->inter_s & INT_ST_INC)
	&&(~TimerStatus & ST_STOP)
	){
		/*赤*/
		if(WAR_LEFT > SigTimerLeft){
			MENU_Color( 140,12,18,0x80 );
			if((disp ==0)&&(SigTimerLeft!= 0) ){
				GM_SdSet( SD_S_COUNTDW1 ) ;
			}
		}else {
			MENU_Color( 180,180,180,0x80 );
			/*FATMAN SE*/
			if((disp ==0)&&(SigTimerLeft!= 0) ){
				GM_SdSet( SD_A_C4COUNTS ) ;
			}
		}
	}else {
		/*標準*/
		MENU_Color( 180,180,180,0x80 );
	}



	switch (work->mode) {
		case MODE_MIN :
		default :
#ifdef DEBUG_TIMER
			MENU_Locate( (int) (work->sp_pos.x+TIME_OFF_X), (int)(work->sp_pos.y/SC_RATE_H)+TIME_OFF_Y, MENU_MODE_LEFT );
#endif

	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X, DIRECT_SCREEN_Y( BASE_OFF_Y2+TIME_OFF_Y), MENU_MODE_LEFT );
//	MENU_Printf( "TIME LIMIT");
	MENU_Printf( "TIME LEFT");
	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X, DIRECT_SCREEN_Y(BASE_OFF_Y2+TIME_OFF_Y+TIME_OFF_Y_L), MENU_MODE_LEFT );
	MENU_Printf( "%d%d '%d%d ''%d%d",min[0],min[1],sec[0],sec[1],m_sec[0],m_sec[1] );

			break;
		case MODE_SEC :
	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X, DIRECT_SCREEN_Y(BASE_OFF_Y2+TIME_OFF_Y), MENU_MODE_LEFT );
//	MENU_Printf( "TIME LIMIT");
	MENU_Printf( "TIME LEFT");

	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X, DIRECT_SCREEN_Y(BASE_OFF_Y2+TIME_OFF_Y+TIME_OFF_Y_L), MENU_MODE_LEFT );
	MENU_Printf( "%d%d%d%d''%d%d",min[0],min[1],sec[0],sec[1],m_sec[0],m_sec[1] );
			break;
		case MODE_BOMBSEC :
	MENU_Locate( (int) BASE_OFF_X+TIME_OFF_X,DIRECT_SCREEN_Y(BASE_OFF_Y2+TIME_OFF_Y), MENU_MODE_LEFT );
   if ( !BP_Area_EU() )
   {
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
   	MENU_Printf( "%d%d BOMBS",Dsip_bomb/10 ,Dsip_bomb%10 );
   }
   else
   {
//#else 
	   if(Dsip_bomb == 1){
		   MENU_Printf( "%d%d BOMB",Dsip_bomb/10 ,Dsip_bomb%10 );
	   }else {
		   MENU_Printf( "%d%d BOMBS",Dsip_bomb/10 ,Dsip_bomb%10 );
	   }
   }
//#endif
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
	Dsip_bomb = 0 ;
	TimerStatus = 0 ;
	work->tri_id = SPR_LoadTexture(RADAR_TRI);
	work->name = name;
	cnt = GCL_GetOptionValue( 't', 0 ) ;

	SigTimerLeft = DIRECT_TICK(cnt) ;
	work->disp_time = SigTimerLeft ;

	work->proc_id = NULL;
	if ( GCL_GetOption( 'p' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->proc_id = GCL_GetNextInt();
		}
	}
	if ( GCL_GetOption( 's' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			TimerStatus = GCL_GetNextInt();
		}
	}else {
		TimerStatus = 0 ;
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
#ifdef DEBUG_MODE
	TimerDebugSet() ;
#endif

	return 0;
}
static int ProgGetResources( Work *work ,int time )
{
	TimerStatus = 0 ;
	Dsip_bomb = 0 ;
	work->tri_id = SPR_LoadTexture(RADAR_TRI);
	SigTimerLeft = DIRECT_TICK(time) ;
	work->disp_time = SigTimerLeft ;

	work->proc_id = NULL;
	TimerStatus = 0 ;
	work->inter_s = 0 ;
	work->mode = MODE_BOMBSEC ;
	if(SprInit2( work ) < 0) return -1 ;


#ifdef DEBUG_MODE
	TimerDebugSet() ;
#endif
	return 0;
}

void *NewTimer( int name, int where )
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
void *NewProgTimer( int time )
{
	Work		*work;
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(ProgGetResources( work,time )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

int NewGetLeftTime( void )
{
   if ( BP_IsPAL()==TRUE )
      return (SigTimerLeft*6)/5 ;
   else
      return SigTimerLeft ;
}
int NewAddLeftTime( void )
{

	int time = 0;
    if ( GCL_GetOption( 't' ) != NULL ) {
		time = GCL_GetNextInt() ;
	}
	SigTimerLeft += DIRECT_TICK(time) ;
	return ( 0 );

}


void NewSetTimerStatus( void )
{
    if ( GCL_GetOption( 's' ) != NULL ) {
		TimerStatus |= GCL_GetNextInt() ;
	}
}
void NewUnsetTimerStatus( void )
{
    if ( GCL_GetOption( 's' ) != NULL ) {
		TimerStatus &= ~(GCL_GetNextInt()) ;
	}
}
int NewGetTimerStatus( void )
{
	return TimerStatus ;
}

void NewTimerEnd( void )
{
	TimerStatus |= ST_DESTOROY ;
}
void NewProgAddLeftTime( int time )
{
	SigTimerLeft += DIRECT_TICK(time) ;

}
void NewProgSetLeftTime( int time )
{
//	SigTimerLeft = DIRECT_TICK(time) ;
	SigTimerLeft = time ;
}
void NewProgSetLeftBomb( int bomb )
{
	Dsip_bomb = bomb ;
}
