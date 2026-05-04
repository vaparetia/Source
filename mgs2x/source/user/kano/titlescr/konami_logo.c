//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	konami_logo.c
		コナミロゴの表示

	2001/07/01 K.Kano
	$Id: konami_logo.c,v 1.2 2002/12/09 18:38:17 takaki Exp $
*/


#include "titlescr.h"
#include "BP_SplashScreen.h"
#include "BP_FileSupport.h"

typedef struct _konami_logo_Work {
	GV_ACT_EX actor;
	int name;
	int timer;
	int step;

	int fadein_time;
	int disp_time;
	int fadeout_time;

	int proc;
	int ans;

	int key_status;
	int key_press;

	SPR_OBJ *title_konami;
	int tri_handle;
	int tri_strcode;
	int bmp_strcode;

#ifdef KP_XBOX
	u_int	prevconnectstate;
#endif	

   int BP_Skip;
   int BP_ShowStartSplashScreens;
   int BP_DidStartSplashScreens;

} Work;

#ifdef KP_XBOX
#define TITLE_PAD_CANCEL	( PAD_ABXY | PAD_LR | PAD_STA | PAD_SEL | PAD_AL | PAD_AR )
#endif

#define TEX_CODE			3656015			/* logo.tri */
#define TEX_CODE_KONAMI		(7545956)		/* logo_konami.bmp */
#define TEX_CODE_KONAMI_ENG	(9543198)		/* logo_konami_r.bmp */


#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) \
	SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))


#define DISP_CHANL			4


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[N_PROC_ARGS];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=work->ans;

		GCL_ExecProc(work->proc,&arg);
	}
}

static void Key(Work *work)
{
	work->key_status=GV_PadDataDirect[0].status;
#ifdef KP_XBOX	
	work->key_press=GV_PadDataDirect[0].press |
			GV_PadDataDirect[1].press |
			GV_PadDataDirect[2].press |
			GV_PadDataDirect[3].press ;
#else
	work->key_press=GV_PadDataDirect[0].press;
#endif
}

static int KonamiAct(Work *work)
{
#ifdef KP_XBOX
#ifndef KP_WINDOWS
	{	// Xbox の場合のキャンセル処理
		int i;
		u_int state = 0;
		for( i = 0 ; i < GV_PAD_MAX ; i++ ) {
			state |= (( GV_PadDataDirect[ i ].flag & GV_PAD_DISCONNECT ) ? 0 : 1 ) << i;
		}
		if( ( ( state ^ work->prevconnectstate ) & state ) ||
			(work->key_press & TITLE_PAD_CANCEL) ) {
			work->ans=PROC_COM_CANCEL;
		}
		work->prevconnectstate = state;
	}
#else
	{	// Windows の場合のキャンセル処理
		if( (work->key_press & TITLE_PAD_CANCEL) ) {
			work->ans=PROC_COM_CANCEL;
		}
	}
#endif
#else	
	if(work->key_press & PAD_STA){
		work->ans=PROC_COM_CANCEL;
	}
#endif	

   if( work->BP_Skip )
      return 1;

	if(work->timer==0){
		/* 開始 */
		SPR_SHOW(work->title_konami);
	}
	else if(work->timer>work->fadeout_time){
		/* 終了 */
		SPR_HIDE(work->title_konami);
		return 1;
	}

	if(work->timer<work->fadein_time){
		int alpha=work->timer*128/work->fadein_time;
		SET_COLOR_2DPRIM2(work->title_konami,128,128,128,alpha);
	}
	else if(work->timer<work->disp_time){
		SET_COLOR_2DPRIM2(work->title_konami,128,128,128,128);
		if(work->ans==PROC_COM_CANCEL) work->timer=work->disp_time;
	}
	else if(work->timer<work->fadeout_time){
		int alpha=(work->fadeout_time-work->timer)*128/(work->fadeout_time-work->disp_time);
		SET_COLOR_2DPRIM2(work->title_konami,128,128,128,alpha);
	}
	else{
		SET_COLOR_2DPRIM2(work->title_konami,128,128,128,0);
	}

	return 0;
}

static void Act(Work *work)
{
   int isFinished = 0;

   if( work->BP_ShowStartSplashScreens )
   {
      if( !work->BP_DidStartSplashScreens )
      {
#if BP_360
         BP_StartCommonArchiveReadThread();
#endif

         work->BP_DidStartSplashScreens = 1;
         BP_Splash_Start();
      }

      isFinished = BP_Splash_IsDone();
   }
   else
   {
      Key(work);
      isFinished = KonamiAct(work);
   }

   if(isFinished)
   {
      if(work->title_konami) SPR_Destroy_2D_Object(work->title_konami);
      if(work->tri_handle>=0) SPR_KillTexture(work->tri_handle);
      CallLocalProc(work);
      GV_DestroyActor(work);
   }

   work->timer++;
}

static void Die(Work *work)
{
   if( work->BP_DidStartSplashScreens )
   {
      BP_Splash_Finish();
   }
}

/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('t')!=NULL){
		work->fadein_time=DIRECT_TICK(GCL_GetNextInt());
		work->disp_time=work->fadein_time+DIRECT_TICK(GCL_GetNextInt());
		work->fadeout_time=work->disp_time+DIRECT_TICK(GCL_GetNextInt());
    }
    if(GCL_GetOption('b')!=NULL){
		work->tri_strcode=GCL_GetNextInt();
		work->bmp_strcode=GCL_GetNextInt();
	}
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
   SPR_POS newSprPos;
   float newSprWidth;

	GetOptionValue(work);

   work->BP_ShowStartSplashScreens = 0;
   work->BP_DidStartSplashScreens = 0;
   work->BP_Skip = 0;

   switch(work->bmp_strcode)
   {
   case 16499055: //GV_StrCode("sce_cesa_attention")
      // Instantly skip violence warning screen.
      work->BP_Skip = 1;
      break;
   
   case 7545956: //GV_StrCode("logo_konami")
   case 9543198: //GV_StrCode("logo_konami_r")
      work->BP_ShowStartSplashScreens = 1;
      break;

   case 10351174: //GV_StrCode("kcej_logo_alp_ovl") -> kojima productions logo
      work->BP_Skip = 1;
      break;

   default:
      break;
   }

	work->tri_handle=SPR_LoadTexture(work->tri_strcode);
	if(work->tri_handle<0) return 0;

	work->title_konami = SPR_Create_2D_Object(SP_SPRITE, DISP_CHANL, NULL);
	if(work->title_konami == NULL){
		printf("ERR!! Make2DObj[work->title_konami]\n");
		return 0;
	}

	SPR_ObjSetTexture(work->title_konami, work->bmp_strcode, work->tri_handle);
#if 0 //BP_PS2
//#ifdef PSX2	
	SPR_SetPosSprite(work->title_konami, &(SPR_POS){ 0.0f, 0.0f});
#else

   if ( work->bmp_strcode==TEX_CODE_KONAMI || work->bmp_strcode == TEX_CODE_KONAMI_ENG || work->bmp_strcode == 10351174 )
   {
      newSprPos.x = 0.0f; newSprPos.y = 0.0f;
      newSprWidth = 512.0f;
   }
   else
   {
      newSprPos.x = ((1.0f-TARGET_ASPECT_X)*512.0f)/2.0f; newSprPos.y = 0.0f;
      newSprWidth = 512.0f*TARGET_ASPECT_X;
   }

	SPR_SetPosSprite(work->title_konami, &newSprPos );
#endif	

   SPR_SetSizeSprite(work->title_konami, newSprWidth, 384.0f );

	work->title_konami->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_konami->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(work->title_konami,0x00000000);
	
#ifdef KP_XBOX
	work->prevconnectstate = ~0;
#endif

    return 1;
}

/* 初期化部メイン */
void *NewTitleKonami(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;
		work->title_konami=NULL;
		work->timer=0;
		work->proc=0;
		work->ans=PROC_COM_OK;
		work->step=0;

		work->fadein_time=64;
		work->disp_time=128;
		work->fadeout_time=192;

		work->tri_handle=-1;
		work->tri_strcode=TEX_CODE;
		work->bmp_strcode=TEX_CODE_KONAMI;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}
