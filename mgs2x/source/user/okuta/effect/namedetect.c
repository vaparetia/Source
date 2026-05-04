//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    namedetect.c
    ドッグタグ名表示
    2001/07/11 Masafumi Okuta
    $Id: namedetect.c,v 1.1.1.3 2002/11/19 11:47:52 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libcdvd.h>
#include <libscf.h>
#endif

#include <libutl.h>
#include	"gameheader.h"


#include	"def_dma.h"
#include	"../../korekado/conv/define.h"
#include	"camera.h"

#include "BP_Misc.h"

#define DETECT_DIST (15000)	// 認識できる距離

extern int BP_FRAMES_PER_SEC();
#define FRAME_TIME (BP_FRAMES_PER_SEC())

enum{ // 表示チェック
VISIBLE_NONE,	// 不可視
VISIBLE_NORMAL, // 通常モデル
VISIBLE_LOD_1,  // LOD１
VISIBLE_LOD_2,  // LOD２
};

extern int   DG_TickCount;

#ifdef DEBUG_MODE
#include	"debugmenu.h"	
static int nDbgNum   = 0;

#ifdef PSX2
static GM_DEBUG_MENU dbg = {
    class:	"NAME",
    menu:	"ALWAY-DRAW",
    max:	2,
    items:( char *[] ){ "ON", "OFF" },
    values: ( int [] ){ 1, 0 },
    target: 	&nDbgNum,
    mask:	0x00000001,	// 必須
};
#else
static char *debug_items[] = { "ON", "OFF" };
static int debug_values[] = { 1, 0 };
static GM_DEBUG_MENU dbg = { 
	NULL,
	"NAME",
	"ALWAY_DRAW",
	debug_items,
	debug_values,
	&nDbgNum,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};
#endif

#endif
typedef struct _NAME_DETECT{
    OBJECT*		body;
    OBJECT*		bodyLod1;
    OBJECT*		bodyLod2;
    FVECTOR		ret;
    int			nX;
    int			nY;
    int			nAddY;
    int			nWidth;		// 文字幅
    int			nHeight;	// 文字高さ
    int			nDelayCntr;	// 非表示ディレイカウンタ
    int			nAlpha;		// α値
    int			nAlphaMax;	// α値最大
    int			nDrawFlag;	// 描画フラグ
    char*		str;
    int			nDogtagID;	// ドッグタグID

    int			nYear;		// 年
    int			nMonth;		// 月
    int			nDay;		// 日

    int			( *ActFunc)( struct _NAME_DETECT* );	// 動作関数

    struct _NAME_DETECT* pworkPrev;	// リスト前
    struct _NAME_DETECT* pworkNext;	// リスト後
}Work;

typedef	int ( *NDETECT_ACTFUNC )( Work* );

// 動作関数設定
static inline void SetActFunc( Work* work, NDETECT_ACTFUNC actfunc )
{
    work->ActFunc = actfunc;
}


// 名前表示マネージャ
typedef struct _NDETECT_MAN{
    GV_ACT_EX		actor;
    int			nDist;		// 認識距離
    u_char		ucR;		// Ｒ
    u_char		ucG;		// Ｇ
    u_char		ucB;		// Ｂ
    u_char		ucA;		// Alpha

    int			nLastClock;	// 取得したDG時間
    int			nChgTime;	// 日付が変わるまでの時間時間
    int 		nPastTime;	// 経過時間
    sceCdCLOCK		nLastRTC;	// 最後に取得したRTC時間

    int			nRTCYear;
    int			nRTCMonth;
    int			nRTCDay;

    int			nRTCHour;
    int			nRTCMinute;
    int			nRTCSecond;
    int			nRTCLastFrm;

    Work* 		pworkTop;	// リスト先頭
    Work* 		pworkEnd;	// リスト終端
}NDetectManager;

static NDetectManager* _ndetectManeger = NULL;

// その月が何日まであるか: from skoba
static inline int MonthToDay( int year, int month ) // 年月から日の計算
{
    switch ( month ){
    case 2 :
	if ( ( ( ( year % 4 ) == 0 ) && ( ( year % 100 ) != 0 ) ) || ( year % 400 ) == 0 ){ // 閏年
	    return( 29 );
	} else {
	    return( 28 );
	}
    case 4 :
    case 6 :
    case 9 :
    case 11 :
	return ( 30 );
    default :
	return ( 31 );
    }
}

// ワークをリストに加える
static void NDETECT_InsertList( Work* pwork)	// リストに加えるワークへのポインタ
{
    if ( _ndetectManeger == NULL ) return;

    if ( _ndetectManeger->pworkTop == NULL){ 	// 先頭に追加
	_ndetectManeger->pworkTop = _ndetectManeger->pworkEnd = pwork;
	pwork->pworkPrev 			= NULL;
	pwork->pworkNext 			= NULL;
    }else { 			// 最後尾に追加
	_ndetectManeger->pworkEnd->pworkNext 	= pwork;
	pwork->pworkPrev 			= _ndetectManeger->pworkEnd;
	pwork->pworkNext 			= NULL;
	_ndetectManeger->pworkEnd 	  	= pwork;
    }
}
// ワークをリストから外す
static void NDETECT_RemoveList( Work* pwork)	// リストから外すワークへのポインタ
{
    if ( _ndetectManeger == NULL ) return;

    if ( pwork->pworkPrev == NULL ){ // 先頭
	_ndetectManeger->pworkTop = pwork->pworkNext;
	if ( pwork->pworkNext != NULL){
	    pwork->pworkNext->pworkPrev = NULL;
	}
    }else{
	pwork->pworkPrev->pworkNext = pwork->pworkNext;
    }

    if ( pwork->pworkNext == NULL ){ // 最後
	_ndetectManeger->pworkEnd = pwork->pworkPrev;
	if ( pwork->pworkPrev != NULL){
	    pwork->pworkPrev->pworkNext = NULL;
	}
    }else{
	pwork->pworkNext->pworkPrev = pwork->pworkPrev;
    }
}

// ２つのリストをマージソートする
static Work* MergeList( Work* a, Work* b)
{
    Work	Head;
    Work*	pwork;

    pwork = &Head;
    
    // どちらか空になるまで
    while( a != NULL && b != NULL){
	// 値比較
	if ( a->nY <= b->nY ){
	    pwork->pworkNext = a;
	    pwork = a;
	    a = a->pworkNext;
	}else{
	    pwork->pworkNext = b;
	    pwork = b;
	    b = b->pworkNext;
	}
    }

    // 最後の一つを追加
    if ( a == NULL)	pwork->pworkNext = b;
    else		pwork->pworkNext = a;

    return Head.pworkNext; // 先頭を返す
}
// リストをマージソートする
static Work* MergeSortList( Work* x)
{
    Work*	a;
    Work*	b;
    Work*	p;

    // リストにある数が１以下の時はそのまま返す
    if ( x == NULL || x->pworkNext == NULL ){
	return x;
    }

    a = x;

    b = x->pworkNext;
    if ( b != NULL )
	b = b->pworkNext;

    // 要素の２分点をサーチ
    while( b != NULL){
	a = a->pworkNext;
	b = b->pworkNext;
	if ( b != NULL )
	    b = b->pworkNext;
    }

    p = a->pworkNext;
    a->pworkNext = NULL;

    // 再帰整列
    return MergeList( MergeSortList(x), MergeSortList(p) );
}

// yyyymmdd形式からyyyy/mm/dd形式に変換
static void ConvBCD2Day( int* pnYear, int* pnMonth, int* pnDay, int nBCD )
{
    (*pnYear)  = ((nBCD & 0xf0000000) >> 28) * 1000 + 
	         ((nBCD & 0x0f000000) >> 24) * 100 + 
	         ((nBCD & 0x00f00000) >> 20) * 10 + 
	         ((nBCD & 0x000f0000) >> 16);
    (*pnMonth) = ((nBCD & 0x0000f000) >> 12) * 10 + 
	         ((nBCD & 0x00000f00) >> 8);
    (*pnDay)   = ((nBCD & 0x000000f0) >> 4) * 10 + 
	          (nBCD & 0x0000000f);
}
#if 0
// yyyy/mm/dd形式からyyyymmdd形式に変換
static int ConvDay2BCD( int nYear, int nMonth, int nDay )
{
    int nBCD;
    int y1, y2, y3, y4, m1, m2, d1, d2;

    y1 = nYear % 10;
    y2 = (int)((nYear % 100) - y1) / 10;
    y3 = (int)((nYear % 1000) - (y1+y2)) / 100;
    y4 = (int)((nYear % 10000) - (y1+y2+y3)) / 1000;
    m1 = nMonth % 10;
    m2 = (int)((nMonth % 100) - m1) / 10;
    d1 = nDay % 10;
    d2 = (int)((nDay % 100) - d1) / 10;

    nBCD = (y4 << 28) | (y3 << 24) | (y2 << 20) | (y1 << 16) | (m2 << 12) | (m1 << 8) | (d2 << 4) | d1;

    return (nBCD);
}
#endif
// byte単位でBCD時間を通常に直す
static u_char ConvByteBCD2Time( u_char ucByte)
{
    return ( ((ucByte & 0xf0) >> 4) * 10 + (ucByte & 0x0f) );
}

// 時間更新
static void UpdateDate( NDetectManager* ndmng )
{
    sceCdCLOCK* pRTCTime = &G_rtc;

    // 経過時間更新
    if ( DG_TickCount >= ndmng->nLastClock ){
	ndmng->nPastTime += (DG_TickCount - ndmng->nLastClock);
    }else{
	ndmng->nPastTime += DG_TickCount;
    }
    ndmng->nLastClock = DG_TickCount;

    if ( ndmng->nPastTime >= ndmng->nChgTime){ // 日付がかわった
	ndmng->nPastTime = 0;
	ndmng->nChgTime  = 24 * 3600 * FRAME_TIME;

	ndmng->nRTCDay++; // 日の更新
	if ( ndmng->nRTCDay > MonthToDay( ndmng->nRTCYear, ndmng->nRTCMonth) ) { // 月の更新
	    ndmng->nRTCMonth++;
	    ndmng->nRTCDay = 1;
	}
	if ( ndmng->nRTCMonth > 12) { // 年の更新
	    ndmng->nRTCDay   = 1;
	    ndmng->nRTCMonth = 1;
	    ndmng->nRTCYear++;
	}
	if ( ndmng->nRTCYear > 2100 ) { // リミット
	    ndmng->nRTCYear  = 2100;
	    ndmng->nRTCDay   = 1;
	    ndmng->nRTCMonth = 1;
	}
    }

    // RTC更新チェック
    {
	sceCdCLOCK* pRTCLast = &ndmng->nLastRTC;
	if ( (pRTCLast->second != pRTCTime->second) ||
	     (pRTCLast->minute != pRTCTime->minute) ||
	     (pRTCLast->hour   != pRTCTime->hour) ||
	     (pRTCLast->day    != pRTCTime->day) ||
	     (pRTCLast->month  != pRTCTime->month) ||
	     (pRTCLast->year   != pRTCTime->year)){
	    memcpy ( &ndmng->nLastRTC, pRTCTime, sizeof( sceCdCLOCK ));

	    ndmng->nChgTime = ( ( (23 - ConvByteBCD2Time( pRTCTime->hour)) * 3600) + 
				( (59 - ConvByteBCD2Time( pRTCTime->minute)) * 60) + 
				( (60 - ConvByteBCD2Time( pRTCTime->second)))) * FRAME_TIME;
	    ndmng->nPastTime = 0;

	    ndmng->nRTCYear  = ConvByteBCD2Time( pRTCTime->year) + 2000;
	    ndmng->nRTCMonth = ConvByteBCD2Time( pRTCTime->month);
	    ndmng->nRTCDay   = ConvByteBCD2Time( pRTCTime->day);

	    ndmng->nRTCHour   = ConvByteBCD2Time( pRTCTime->hour);
	    ndmng->nRTCMinute = ConvByteBCD2Time( pRTCTime->minute);
	    ndmng->nRTCSecond = ConvByteBCD2Time( pRTCTime->second);
#ifdef DEBUG_MODE
printf("ＲＴＣ更新 %4d年%2d月%2d日 %2d時%2d分%2d秒！！\n", ndmng->nRTCYear, ndmng->nRTCMonth, ndmng->nRTCDay,
       ndmng->nRTCHour, ndmng->nRTCMinute, ndmng->nRTCSecond);
#endif
	}
    }

    {
	if ( abs(DG_TickCount - ndmng->nRTCLastFrm) >= FRAME_TIME ){
	    ndmng->nRTCSecond += (int)(abs(DG_TickCount - ndmng->nRTCLastFrm) / FRAME_TIME);	
	    ndmng->nRTCLastFrm = DG_TickCount;
	}

	if ( ndmng->nRTCSecond >= 60){
	    ndmng->nRTCMinute++;
	    ndmng->nRTCSecond -= 60;
	}
	if ( ndmng->nRTCMinute >= 60){
	    ndmng->nRTCHour++;
	    ndmng->nRTCMinute = 0;
	}

	if ( ndmng->nRTCHour >= 24){
	    ndmng->nRTCHour = 0;
	}

#ifdef DEBUG_MODE
#if 0
	MENU_Locate( 30, 50, 0);
	MENU_Color( 58, 238, 122, 128);
	MENU_Printf("%4d/%2d/%2d %2d:%2d:%2d\n", ndmng->nRTCYear, ndmng->nRTCMonth, ndmng->nRTCDay,
		    ndmng->nRTCHour, ndmng->nRTCMinute, ndmng->nRTCSecond);
#endif
#endif
    }
}

// 透視変換
static  void  PosToRet(Work *work, OBJECT* pbody)
{
    DG_CHANL  *cp ;
    FVECTOR   pos ;   
    GM_CameraSet	*cam ;
    extern void	KR_FMatToFvec( FMATRIX	*mat, FVECTOR *vec );

    cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

    /* 位置決定 */
    KR_FMatToFvec( &BODYWORLD( pbody, HUMAN21_ATAMA), &pos ) ;
    cp = DG_Chanl(0) ;
    pos.vw = 1.f;
    _sceVu0ApplyMatrix( &work->ret, &cp->eye_pers, &pos) ;
}

// 表示内チェック
static  int  InScreen( FVECTOR *ret )
{
    /* カメラの範囲内に入っているかどうか */
    if( ret->vz > ret->vw )	return 0;

    if( ret->vw < 0 )	ret->vw = -ret->vw;

    if( (ret->vx > ret->vw) || (ret->vx < (-ret->vw)) )	return 0 ;
    if( (ret->vy > ret->vw) || (ret->vy < (-ret->vw)) )	return 0 ;
    
    return 1 ;
}

// α値取得
static int GetAlpha( Work* work )
{
    int 	value;
    static int  power = 0;

#ifdef DEBUG_MODE
    if( (((PL_GetPlayerItem( ) == IT_Scope) ||
	  (PL_GetPlayerItem( ) == IT_Camera) ||
	  (PL_GetPlayerItem( ) == IT_TnkCamera) ) &&
	 (GV_PadDataDirect[ 0 ].pressure[ PAD_PRESS_X ] > 10)) || (*dbg.target) ){
	power += 8;
    } else {
	power -= 16;
    }
#else
    if( ((PL_GetPlayerItem( ) == IT_Scope) ||
	 (PL_GetPlayerItem( ) == IT_Camera) ||
	 (PL_GetPlayerItem( ) == IT_TnkCamera) ) &&
	(GV_PadDataDirect[ 0 ].pressure[ PAD_PRESS_X ] > 3) ){
	power += 8;
    } else {
	power -= 16;
    }
#endif

    value = 1 ;
    if ( power > 30 ) {
	if ( power > work->nAlphaMax*2 + 30  ) power = work->nAlphaMax*2 + 30;
	value = ((power-30) / 2)  + 1;
    } else {
	if ( power < 0 ) power = 0 ;
	value = 1 ;
    }

    if ( value < 0 ) 		value = 0;
    else if ( value > 255)	value = 255;
    
    return value;
}

// 表示位置設定
static void UpdatePoint( Work* work)
{
    FVECTOR*	ret;

    ret = &work->ret ;

    work->nX = (int)((ret->vx / ret->vw) * (float)(DRAW_WIDTH>>1)) + (DRAW_WIDTH>>1) ;
    work->nY = (int)((ret->vy / ret->vw) * (float)(DRAW_HEIGHT>>1)) + (DRAW_HEIGHT>>1) - 50 ;
    
    { // 画面はみ出しチェック
	int nHalfWidth, nHalfHeight;
	nHalfWidth  = work->nWidth >> 1;
	nHalfHeight = work->nHeight >> 1;
	
	if ( work->nX < nHalfWidth + 16 ) 				 
	    work->nX = nHalfWidth + 16;
	else if ( work->nX > DG_Chanls[0].width - (nHalfWidth + 16) )	 
	    work->nX = DG_Chanls[0].width - (nHalfWidth + 16);

	if ( work->nY < nHalfHeight + 40 ) 			 
	    work->nY = nHalfHeight + 40;
	else if ( work->nY > DG_Chanls[0].height - (nHalfHeight + 40) ) 
	    work->nY = DG_Chanls[0].height - (nHalfHeight + 40);
    }
}
// 誕生日チェック
static int ChickBirthDay(Work *work)
{
    NDetectManager* ndmng = _ndetectManeger;

    if ( ndmng == NULL ) return 0;

    if ( work->nMonth != 0 && work->nDay != 0 &&
	 work->nMonth == ndmng->nRTCMonth &&
	 work->nDay   == ndmng->nRTCDay ){
	return 1;
    }

    return 0;
}

// 名前表示
static  void  PrintScreen(Work *work)
{
    MENU_Locate( work->nX, work->nY + work->nAddY, 2);
    MENU_Color( 58, 238, 122, work->nAlpha);
    if ( ChickBirthDay( work ) ){ // お誕生日おめでとー
	MENU_Printf("Happy birthday!!\n");
    }
    MENU_Color( _ndetectManeger->ucR, _ndetectManeger->ucG, _ndetectManeger->ucB, work->nAlpha);
    MENU_Printf("%s\n",work->str);
    MENU_ResetColor();
}

// 動作関数
static int NDT_ActFunc( Work *work )
{
    OBJECT*	pbody;
    int nChkVisible = VISIBLE_NONE;

    // 不可視状態
    if ( (work->body->objs->flag & DG_FLAG_INVISIBLE0) &&
	 (work->body->objs->flag & DG_FLAG_INVISIBLE1) ){
	if ( work->body->evmobj != NULL){
	    if ( (work->body->evmobj->flag & DG_EVMOBJ_INVISIBLE0) && 
		 (work->body->evmobj->flag & DG_EVMOBJ_INVISIBLE1) ){
		nChkVisible = VISIBLE_NONE;
	    }else{
		nChkVisible = VISIBLE_NORMAL;
	    }
	}else{
	    nChkVisible = VISIBLE_NONE;
	}
    }else{
	nChkVisible = VISIBLE_NORMAL;
    }

    // LOD1チェック
    if ( !nChkVisible && work->bodyLod1 != NULL ) {
	if ( !(work->bodyLod1->objs->flag & DG_FLAG_INVISIBLE0) ||
	     !(work->bodyLod1->objs->flag & DG_FLAG_INVISIBLE1) ){
	    nChkVisible = VISIBLE_LOD_1;
	}
    }

    // LOD2チェック
    if ( !nChkVisible && work->bodyLod2 != NULL ) {
	if ( !(work->bodyLod2->objs->flag & DG_FLAG_INVISIBLE0) ||
	     !(work->bodyLod2->objs->flag & DG_FLAG_INVISIBLE1) ){
	    nChkVisible = VISIBLE_LOD_2;
	}
    }

    switch ( nChkVisible ){ // 不可視
    case VISIBLE_NORMAL:
	pbody = work->body;
	break;
    case VISIBLE_LOD_1:
	pbody = work->bodyLod1;
	break;
    case VISIBLE_LOD_2:
	pbody = work->bodyLod2;
	break;
    default:
	return 0; // 不可視
	break;
    }

    {
	int nAim;
	extern int MAO_SphereCheck(FVECTOR* pvecPos, FVECTOR* pvecCenter, int nRad);
	// 距離チェック
	if ( !MAO_SphereCheck( (FVECTOR*)&DG_Chanls[0].eye.m[3][0], 
			       (FVECTOR*)&BODYWORLD( pbody, HUMAN21_ATAMA).m[3][0], DETECT_DIST ) ){
	    nAim = 32;
   
      if ( BP_IsPAL()==TRUE )
         GV_NearExp8PVPAL( &work->nAlphaMax, &nAim, 1 );
      else
         GV_NearExp8PV( &work->nAlphaMax, &nAim, 1 );
	}else{
	    nAim = 96;


      if ( BP_IsPAL()==TRUE )
	      GV_NearExp8PVPAL( &work->nAlphaMax, &nAim, 1 );
      else
         GV_NearExp8PV( &work->nAlphaMax, &nAim, 1 );
	}

	// 透視変換
	PosToRet( work, pbody ) ;

	// 画面チェック
	if ( InScreen( &work->ret ) ) {
	    work->nDelayCntr = COUNT_VMODE(120);
	    work->nAlpha = GetAlpha( work );
	    UpdatePoint( work );
	}else{
	    if ( work->nDelayCntr < work->nAlpha ) work->nAlpha = work->nDelayCntr;
	    if ( work->nDelayCntr > 0 ){
		if ( work->ret.vw > 0.f ){
		    UpdatePoint( work);
		}
		work->nDelayCntr -= ( (work->nDelayCntr < 4) ? work->nDelayCntr : 4 );
	    }
	}

	if ( work->nAlpha == 0 ) return 0;	
    }
    return  1;
}

// 破棄関数
static int NDT_DieFunc( Work *work )
{
    NDETECT_RemoveList( work);			// リストから外す
    if ( work != NULL ) GV_Free( work );	// メモリ解放	

    return 0;
}

// マネージャ動作関数
static void Act( NDetectManager* ndmng )
{
    int 	nList, nDraw, nCurrY, nDiff;
    Work* 	pwork;

    UpdateDate( ndmng);

    nList = 0;
    for ( pwork = ndmng->pworkTop; pwork != NULL; pwork = pwork->pworkNext ){	
	extern int SIG_CheckDogTagFlag( int);

	if ( !SIG_CheckDogTagFlag( pwork->nDogtagID ) ){ // 取得チェック
	    pwork->nDrawFlag = 0;
	}else{
	    pwork->nDrawFlag = 1;
	}

#ifdef DEBUG_MODE
	if ((*dbg.target)){
	    pwork->nDrawFlag = 1; // 強制表示
	}
#endif
	if ( pwork->nDrawFlag ){
	    // 表示位置計算
	    pwork->nDrawFlag = NDT_ActFunc( pwork );
	}
	nList++;
    }

    // 高さをキーにしてマージソート
    ndmng->pworkTop = MergeSortList( ndmng->pworkTop );
    nDraw = 0;
    nCurrY = -50;
    for ( pwork = ndmng->pworkTop; pwork != NULL; pwork = pwork->pworkNext ){
	// 描画チェック
	if ( pwork->nDrawFlag){
	    nDiff = abs( pwork->nY - nCurrY);

	    if ( ChickBirthDay( pwork) ){ // 
		if ( nDiff < 30 ){
		    pwork->nAddY = 30;
		}else{
		    if ( pwork->nAddY > 0 ) pwork->nAddY--;
		}
	    }else{
		if ( nDiff < 15 ){
		    pwork->nAddY = 15;
		}else{
		    if ( pwork->nAddY > 0 ) pwork->nAddY--;
		}
	    }

	    // 表示
	    PrintScreen( pwork );

	    // 判定用Y値更新
	    nCurrY = pwork->nY + pwork->nAddY;	

	    nDraw++;
	}
	ndmng->pworkEnd = pwork;

	if ( pwork->pworkNext != NULL){
	    pwork->pworkNext->pworkPrev = pwork;
	}
    }
    if ( ndmng->pworkTop != NULL) ndmng->pworkTop->pworkPrev = NULL;
    if ( ndmng->pworkEnd != NULL) ndmng->pworkEnd->pworkNext = NULL;
}

// マネージャ破棄関数
static void Die( NDetectManager* ndmng )
{
    Work* 	pwork;
    Work* 	pworkCurrent;

    for ( pwork = ndmng->pworkTop; pwork != NULL; pwork = pworkCurrent ){
	pworkCurrent = pwork->pworkNext;
	NDT_DieFunc( pwork );
    }

    _ndetectManeger = NULL;
}


// 初期化関数
static int InitNDetect( Work* 	work,        
			OBJECT* body,        // ドッグタグ保持者のモデル
			OBJECT* bodyLod1,    // ドッグタグ保持者のモデル[LOD 1]
			OBJECT* bodyLod2,    // ドッグタグ保持者のモデル[LOD 2]
			char* 	str,         // 名前
			int 	tagID,       // ドッグタグ番号
			int 	nBirthday )  // 誕生日
{
    work->body 	   = body;
    work->bodyLod1 = bodyLod1;
    work->bodyLod2 = bodyLod2;
    work->str      = str;

    {
	char* pstrCurr;
	extern char font_pitch1[];
	
	work->nWidth  = 0;
	work->nHeight = 14;
	for ( pstrCurr = work->str; (*pstrCurr) != '\0'; pstrCurr++){
	    int code;
	    code = (*pstrCurr) - ' ';
	    if ( code >= ' ' && code < ( ' ' + 96 ) ) 	work->nWidth += font_pitch1[ code ];
	    else					work->nWidth += font_pitch1[ 0 ];
	}
    }

    work->nAddY		= 0;
    work->nDrawFlag 	= 0;

    work->nDogtagID = tagID;

    // 誕生日取得
    ConvBCD2Day( &work->nYear, &work->nMonth, &work->nDay, nBirthday);

    // リストに追加
    NDETECT_InsertList( work);

    return  0 ;
}

// ドッグタグ名前表示登録
void* NewNamePrint( OBJECT *body, char *str, int tagID, int nBCD )
{
    Work* work;

    if ( _ndetectManeger == NULL){
#ifdef DEBUG_MODE
	printf("DogtagNameDetect Manager Not Found!!\n");
#endif
	return NULL;
    }

    // メモリ確保
    work = (Work *)GV_Malloc( sizeof(Work) );

    if( work != NULL){
	GV_ZeroMemory( work, sizeof(Work) );
	if( InitNDetect( work, body, NULL, NULL, str, tagID, nBCD ) < 0 ){
	    GV_Free( work ); // 解放
	    return NULL;
	}
    }				

    return (void *)work;
}

// ドッグタグ名前表示登録:LOD対応版
void* NewNamePrint4Lod( OBJECT* body, OBJECT* bodyLod1, OBJECT* bodyLod2, char *str, int tagID, int nBCD )
{
    Work* work;

    if ( _ndetectManeger == NULL){
#ifdef DEBUG_MODE
	printf("DogtagNameDetect Manager Not Found!!\n");
#endif
	return NULL;
    }

    // メモリ確保
    work = (Work *)GV_Malloc( sizeof(Work) );

    if( work != NULL){
	GV_ZeroMemory( work, sizeof(Work) );
	if( InitNDetect( work, body, bodyLod1, bodyLod2, str, tagID, nBCD ) < 0 ){
	    GV_Free( work ); // 解放
	    return NULL;
	}
    }				

    return (void *)work;
}

// マネージャ初期化
static int GetResources( NDetectManager* ndmng )
{
    char* 	optColor = NULL;		// 色データ

    // RTC時間更新
    GM_GetRTCTime() ;/* gamed.c*/

    ndmng->pworkTop = NULL;	// リスト先頭
    ndmng->pworkEnd = NULL;	// リスト終端

    // 認識距離
    ndmng->nDist = GCL_GetOptionValue( 'd', 15000 );

    // カラー情報
    if ( ( optColor = GCL_GetOption( 'c' ) ) != NULL ){	
	GCL_SetArgTop( optColor );
	ndmng->ucR = (u_char)GCL_GetNextInt();
	optColor = GCL_NextStr();
	GCL_SetArgTop( optColor );
	ndmng->ucG = (u_char)GCL_GetNextInt();
	optColor = GCL_NextStr();
	GCL_SetArgTop( optColor );
	ndmng->ucB = (u_char)GCL_GetNextInt();
	optColor = GCL_NextStr();
	GCL_SetArgTop( optColor );
	ndmng->ucA = (u_char)GCL_GetNextInt();
	optColor = GCL_NextStr();
    }else{
	ndmng->ucR = 58;
	ndmng->ucG = 122;
	ndmng->ucB = 255;
	ndmng->ucA = 128;
    }

    // 最新のRTCを取得
    {
	sceCdCLOCK* pRTCTime = &G_rtc;

	memcpy ( &ndmng->nLastRTC, pRTCTime, sizeof( sceCdCLOCK ));

	ndmng->nChgTime = ( ( (23 - ConvByteBCD2Time( pRTCTime->hour)) * 3600) + 
			   ( (59 - ConvByteBCD2Time( pRTCTime->minute)) * 60) + 
			   ( (60 - ConvByteBCD2Time( pRTCTime->second)))) * FRAME_TIME;
	ndmng->nLastClock = DG_TickCount;
	ndmng->nPastTime = 0;

	ndmng->nRTCYear  = ConvByteBCD2Time( pRTCTime->year) + 2000;
	ndmng->nRTCMonth = ConvByteBCD2Time( pRTCTime->month);
	ndmng->nRTCDay   = ConvByteBCD2Time( pRTCTime->day);

	ndmng->nRTCHour   = ConvByteBCD2Time( pRTCTime->hour);
	ndmng->nRTCMinute = ConvByteBCD2Time( pRTCTime->minute);
	ndmng->nRTCSecond = ConvByteBCD2Time( pRTCTime->second);
	ndmng->nRTCLastFrm = DG_TickCount;
    }
#ifdef DEBUG_MODE
    GM_AddDebugMenu( &dbg );	// 番号表示デバック登録
#endif

    return 0;
}

// 外部からの破棄関数
int NDETECT_Remove( void* pvNDetect)
{
    Work* work = (Work*)pvNDetect;
    
    if ( _ndetectManeger == NULL || work == NULL) return -1;

//    NDETECT_RemoveList( work);
    NDT_DieFunc( work );
#ifdef DEBUG_MODE
printf(">>>>>>>>>>>> NDETECT_Remove <<<<<<<<<<<<\n");
#endif
    return 0;
}

// ドッグタグ名前表示マネージャ
void* NewNamePrintManager( name, where )
int	name;
int	where;
{
    NDetectManager*	ndmng;

    ndmng = (NDetectManager *)GV_NewActor(  GV_ACTOR_AFTER, sizeof( NDetectManager ) ) ;

    if( ndmng != NULL){
	GV_SetActor( &(ndmng->actor), Act,Die) ;
	GV_ActorEX( &ndmng->actor );
	if( GetResources( ndmng ) < 0){
	    GV_DestroyActor( ndmng ) ;
	    return NULL ;
	}
    }				

    // ポインタ取得
    _ndetectManeger = ndmng;

    return (void *)ndmng;
}
