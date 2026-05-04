//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   name_layout.c
   ネームエントリー
   
   2001/06/05	S.Kobayashi
   $Id: name_layout.c,v 1.2 2002/12/05 18:42:01 takaki Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>

#if 0 //BP_PS2 def PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include <libutl.h>
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"
#include    "../../mode/codec/cjimaku.h"
#include    "../../mode/codec/codecmem.h"

#include    "../../mode/node/node.h"
#include    "name_layout.h"

#include "BP_Misc.h"

#include "mode/menu/xtextscn.h"

enum {
	SK_NORMAL             = 0x00001 ,
	SK_END                = 0x00002 ,
	SK_STREAM_NORMAL      = 0x00004 ,
	SK_STREAM_END         = 0x00008 ,
	SK_CHANCEL            = 0x00010 ,
	SK_ACTION_START       = 0x00020 ,
	SK_ACTION_OK          = 0x00040 ,
	SK_NAME_ENTRY         = 0x00080 ,
	SK_SEX_ENTRY          = 0x00100 ,
	SK_BIRTH_ENTRY        = 0x00200 ,
	SK_BLOOD_ENTRY        = 0x00400 ,
	SK_REGION_ENTRY       = 0x00800 ,
	SK_OK_MODE            = 0x01000 ,
	SK_NORMAL_MODE        = 0x02000 ,
	SK_NAME_ENTRY_ESCAPE  = 0x04000 ,
	SK_SPRITE_INIT_OK     = 0x08000 ,
	SK_BIRTH_ENTRY_ESCAPE = 0x10000 ,
	SK_SEX_ENTRY_ESCAPE   = 0x20000 ,
	SK_BLOOD_ENTRY_ESCAPE = 0x40000 ,
	SK_REGION_ENTRY_ESCAPE = 0x80000 ,
	SK_OK_ENTRY_ESCAPE     = 0x100000 ,
	SK_MOVE_OK             = 0x200000 ,
	SK_ROOL_OK             = 0x400000 ,
	SK_FADE_OK             = 0x800000 ,
	SK_FIRST_ACTION        = 0x1000000 ,
	SK_SIGNAL_OK           = 0x2000000 ,
};

enum {
	SK_BIRTH_FIRST_INPUT = 0x1 ,
};

typedef struct {
	int x;
	int y;
	int add_x;
	int add_y;
} Soft;

typedef struct {
	void      *work;
	u_char        r;
	u_char        g;
	u_char        b;
	u_char        a;
} Code ;

typedef struct {
	float p;
	int   count;
	int   code1;
	int   code2;
	int   morf_parts;
	int   flag;
} Hokan;

typedef struct {
	int position;     // 通常時カーソルの場所
	int position_max; // 常に変動するため
	int old_position; // 前の場所
} NormalMode;

typedef struct {
	int position;
	int old_position;
	int input_flag;
} Name;

typedef struct {
	int position;     // 通常時カーソルの場所
	int old_position;
	int back_position;
	Hokan hokan;
} Sex;

typedef struct {
	int year;
	int month;
	int day;
	int old_year;
	int old_month;
	int old_day;
	int phase; // 0 : year , 1 : month , 2 : day
	char flag;
   SPR_OBJ *obj[ MAX_BIRTH ];
	SPR_OBJ *happy;
} Birth;

typedef struct {
	int position;     // 通常時カーソルの場所
	int old_position;
	int back_position;
	Hokan hokan;
} Blood;

typedef struct {
	int position;     // 通常時カーソルの場所
	int old_position;
	int back_position;
	SPR_OBJ *obj[ MAX_REGION_OBJ ];
	SPR_OBJ *kcej; // 特殊すぷらいと
} Region;

typedef struct {
	int font;
	int region;
	int birth;
} DupCount;

#define MAX_NAME (20)

typedef struct _name_work {
	struct _name_work *pPrev;
	struct _name_work *pNext;
	u_char first_name[ MAX_NAME ];
	u_char last_name[ MAX_NAME ];
	int  sex; 
	int  year; 
	int  month; 
	int  day; 
	int  blood; 
	int  nation;
	char flag; 
} NameWork;

typedef struct _ng_work {
	struct _ng_work *pPrev;
	struct _ng_work *pNext;
	u_char first_name[ MAX_NAME ];
	u_char last_name[ MAX_NAME ];
} NgWork;

typedef	struct _name_layout_work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int					handle_out;

	SPR_OBJ             *font[ ASCCI_MAX ];
	char                backup_name[ ASCCI_MAX ];
	SPR_OBJ             *hilight; // sprite
	NormalMode          normal_mode; // 通常状態各種
	Sex                 sex;         // 性別入力情報
	Name                name; // 名前
	Birth               birth; // 誕生日
	Blood               blood; // 血液型
	Region              region; // 国
	Code                font_work;
	SPR_COLOR           color;
	int                 action_num;
	int                 base_u;
	int                 base_v;
	int                 base_num_u;
	int                 base_num_v;
	int                 flag; // 
	int                 pad_status[ 2 ]; // pad info
	int                 pad_check[ 2 ]; // pad info
	int                 hold_time[ 3 ]; // auto mode 
	int                 proc_prev; // キャンセル時のproc
	int                 proc_next; // 決定時のproc
	int                 scn_name;
	int                 action; // action strcode
	Hokan               dot_hokan; // dot
	Hokan               disp_hokan[ MAX_KEY_ANIME ]; // disp
	Soft                soft;
	DupCount            dup_count;
	NameWork            *name_work;
	NgWork              *ng_work;
	u_long64              *name_work_top_add;
	u_long64              *ng_work_top_add;

   int                  bp_keyret;
   char                 bp_keypress;

	void  ( *act )( struct _name_layout_work * );

	int   child_mode:1;


} Work ;

// プロトタイプ
static void AnimetionAct( Work *pWork ); // アニメーション
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan * );
static void NemaEntryEscapeAct( Work *pWork );
static void NormalAct( Work *pWork );
static void CopyAlpha( Work *pWork );
static void EscapeAct( Work *pWork );
static void BirthEntryEscapeAct( Work *pWork );
static void SexEntryEscapeAct( Work *pWork );
static void BloodEntryEscapeAct( Work *pWork );
static void RegionEntryEscapeAct( Work *pWork );
static void OkEntryEscapeAct( Work *pWork );
static inline int MorfAct( Work *pWork , Hokan *pHokan );
static void StaffNameSwap( Work *pWork ); // スタッフデータに入れ替える
static int NgWordCheck( Work *pWork ); // 卑猥な言葉かどうか
static void PositionAct( Work *pWork );
static void HappyBirthDay( Work *pWork ); // 誕生日おめでとう
static void NoneName( Work *pWork ); // 名無の時？にする
static inline void SoftHideShowControl( Work *pWork , char mode ); // Sprite 表示 、非表示

// extern 
extern void SK_Printf( int ascci , SPR_OBJ *pObj , int u , int v , int , int );
extern void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode );
extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void * CDC_GetFileEntry(int strcode, int suffix);
extern void *GetLocalResource( int ref_id, int offset );


extern void SK_CODEC_OpenMesgPanel( CODEC_MESG_PANEL *panel, int width, int height );
extern void SK_CODEC_CloseMesgPanel( CODEC_MESG_PANEL *panel );
extern void SK_CODEC_ShowMesgPanel(CODEC_MESG_PANEL * panel);
extern void SK_CODEC_DrawMesgPanel( CODEC_MESG_PANEL *panel, int x, int y , CVECTOR *color );
extern void SK_CODEC_DrawMessage( CODEC_MESG_PANEL *panel, char *mesg );
extern int codecMemInit( void );  /* 無線用メモリ領域を初期化する */ 
extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfNormal2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern float SK_PrintfNormalWidth( u_char , SPR_OBJ * );
extern float SK_PrintfNormalWidth_NameSelect( u_char , SPR_OBJ * );
extern void SK_PrintfNum( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern float SK_PrintfNormalWidth3( u_char *ascci , int max_position ); // 一括 space通常
extern float SK_PrintfNormalWidth2( u_char *ascci ); // ?? space??


// #define NAME_ENTRY_END (0x10)   // node.h で定義のためいらない。

/*----------------------------------------------------------------*/
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )workp;

	switch ( signal ){
	case 0x2 :
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_CallParentSignalFunc( pWork , NAME_ENTRY_END , 0 );
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

static void NgWorkInitialize( Work *pWork )
{
	// 確保
	pWork->ng_work = ( NgWork * )SK_MALLOC( sizeof( NgWork ) );
	if ( pWork->ng_work == NULL ){
		SK_Err("staff name store miss\0");
		return;
	}
	pWork->ng_work->pPrev = NULL;	
	pWork->ng_work->pNext = NULL;

	pWork->ng_work->first_name[ 0 ] = 0x0;
	pWork->ng_work->last_name[ 0 ] = 0x0;
	// address
	pWork->ng_work_top_add = ( u_long64 * )pWork->ng_work;
}

static void NgWorkListAdd( Work *pWork , u_char *pFirst_name , u_char *pLast_name ) // リストに追加
{
	NgWork *pNgWork;
	NgWork *pTmp;
	int      len;

	pNgWork = ( NgWork * )pWork->ng_work_top_add;

	while ( pNgWork->pNext != NULL ){ // 先頭の１文字で判断
		if ( pNgWork->first_name[ 0 ] < pFirst_name[ 0 ] ){
			pNgWork = pNgWork->pNext;
			continue;
		}
		// 挿入
		pTmp = ( NgWork * )SK_MALLOC( sizeof( NgWork ) );
		if ( pTmp == NULL ){
			SK_Err("staff name store miss1\0");
			return;
		}
		len = strlen( pFirst_name );
		strncpy( pTmp->first_name , pFirst_name , len );
		pTmp->first_name[ len ] = '\0';
		len = strlen( pLast_name );
		strncpy( pTmp->last_name , pLast_name , len );
		pTmp->last_name[ len ] = '\0';
		// link
		pTmp->pPrev = pNgWork;
		pTmp->pNext = pNgWork->pNext;
		pNgWork->pNext->pPrev = pTmp;
		pNgWork->pNext = pTmp;
		return;
	}
	// 新品というかbottomに追加
	pTmp = ( NgWork * )SK_MALLOC( sizeof( NgWork ) );
	if ( pTmp == NULL ){
		SK_Err("staff name store miss2\0");
		return;
	}
	len = strlen( pFirst_name );
	strncpy( pTmp->first_name , pFirst_name , len );
	pTmp->first_name[ len ] = '\0';
	len = strlen( pLast_name );
	strncpy( pTmp->last_name , pLast_name , len );
	pTmp->last_name[ len ] = '\0';
	// link
	pTmp->pPrev = pNgWork;
	pTmp->pNext = NULL;
	pNgWork->pNext = pTmp;

	return;
}

static void NameWorkInitialize( Work *pWork )
{
	// 確保
	pWork->name_work = ( NameWork * )SK_MALLOC( sizeof( NameWork ) );
	if ( pWork->name_work == NULL ){
		SK_Err("staff name store miss\0");
		return;
	}
	pWork->name_work->pPrev = NULL;	
	pWork->name_work->pNext = NULL;

	pWork->name_work->first_name[ 0 ] = 0x0;
	pWork->name_work->last_name[ 0 ] = 0x0;
	// address
	pWork->name_work_top_add = ( u_long64 * )pWork->name_work;
}

static void NameWorkListAdd( Work *pWork , u_char *pFirst_name , u_char *pLast_name , 
							 int sex , int year , int month , int day , int blood , int nation ) // リストに追加
{
	NameWork *pNameWork;
	NameWork *pTmp;
	int      len;

	pNameWork = ( NameWork * )pWork->name_work_top_add;

	while ( pNameWork->pNext != NULL ){ // 先頭の１文字で判断
		if ( pNameWork->first_name[ 0 ] < pFirst_name[ 0 ] ){
			pNameWork = pNameWork->pNext;
			continue;
		}
		// 挿入
		pTmp = ( NameWork * )SK_MALLOC( sizeof( NameWork ) );
		if ( pTmp == NULL ){
			SK_Err("staff name store miss1\0");
			return;
		}
		len = strlen( pFirst_name );
		strncpy( pTmp->first_name , pFirst_name , len );
		pTmp->first_name[ len ] = '\0';
		len = strlen( pLast_name );
		strncpy( pTmp->last_name , pLast_name , len );
		pTmp->last_name[ len ] = '\0';
		pTmp->sex    = sex;
		pTmp->year   = year;
		pTmp->month  = month;
		pTmp->day    = day;
		pTmp->blood  = blood;
		pTmp->nation = nation;
		pTmp->flag = 0x0;
		// link
		pTmp->pPrev = pNameWork;
		pTmp->pNext = pNameWork->pNext;
		pNameWork->pNext->pPrev = pTmp;
		pNameWork->pNext = pTmp;
		return;
	}
	// 新品というかbottomに追加
	pTmp = ( NameWork * )SK_MALLOC( sizeof( NameWork ) );
	if ( pTmp == NULL ){
		SK_Err("staff name store miss2\0");
		return;
	}
	len = strlen( pFirst_name );
	strncpy( pTmp->first_name , pFirst_name , len );
	pTmp->first_name[ len ] = '\0';
	len = strlen( pLast_name );
	strncpy( pTmp->last_name , pLast_name , len );
	pTmp->last_name[ len ] = '\0';
	pTmp->sex    = sex;
	pTmp->year   = year;
	pTmp->month  = month;
	pTmp->day    = day;
	pTmp->blood  = blood;
	pTmp->nation = nation;
	pTmp->flag = 0x0;
	// link
	pTmp->pPrev = pNameWork;
	pTmp->pNext = NULL;
	pNameWork->pNext = pTmp;
	return;
}

#define STR_RESOURCE (8505736)
static void NameWorkListMng( Work *pWork )
{
	void *ptr;
	u_char *first_name , *last_name;
	int year , manth , day , blood , nation , sex;

	// set
	ptr = ( void * )GetLocalResource( STR_RESOURCE , 0 );
	if ( ptr == NULL ) {
		return;
	}
	GCL_SetArgTop( ptr );

	while ( 1 ){
		first_name = GCL_GetNextString();
		last_name = GCL_GetNextString();
		sex = GCL_GetNextInt();
		year = GCL_GetNextInt();
		manth = GCL_GetNextInt();
		day = GCL_GetNextInt();
		blood = GCL_GetNextInt();
		nation = GCL_GetNextInt();
		if ( sex == -1 ){
			break;
		}			
		NameWorkListAdd( pWork , first_name , last_name , sex , year , manth , day , blood , nation );
	}
}

#define STR_RESOURCE_NG (12978293)
static void NgWorkListMng( Work *pWork )
{
	void *ptr;
	u_char *first_name , *last_name;

	// set
	ptr = ( void * )GetLocalResource( STR_RESOURCE_NG , 0 );
	if ( ptr == NULL ) {
		return;
	}
	GCL_SetArgTop( ptr );

	while ( 1 ){
		first_name = GCL_GetNextString();
		last_name = GCL_GetNextString();
		if ( first_name[ 0 ] == '-' ){
			break;
		}			
		NgWorkListAdd( pWork , first_name , last_name );
	}
}

static void NameWorkListFree( Work *pWork )
{
	NameWork *pNameWork;
	NameWork *pNameWorktmp;

	pNameWork = ( NameWork * )pWork->name_work_top_add;
	while ( pNameWork != NULL ) {
		pNameWorktmp = pNameWork->pNext;
		SK_FREE( pNameWork , sizeof( NameWork ) );
		pNameWork = pNameWorktmp;
	}
}

static void NgWorkListFree( Work *pWork )
{
	NgWork *pNgWork;
	NgWork *pNgWorktmp;

	pNgWork = ( NgWork * )pWork->ng_work_top_add;
	while ( pNgWork != NULL ) {
		pNgWorktmp = pNgWork->pNext;
		SK_FREE( pNgWork , sizeof( NgWork ) );
		pNgWork = pNgWorktmp;
	}
}


static int SprInit( Work *pWork ) // default を a
{
	SPR_OBJ		*spr;
	int            i;

	// 文字の取得
	spr = L2D_GetObject( pWork->handle_2d, STR_NODE_FONT ) ;
	if ( spr == NULL ){
		SK_Err("font font\0");
		return -1 ;
	}
	SPR_SHOW( spr );
#if 0 //BP_PS2 def PSX2
	pWork->base_u = spr->sprite.head.tex.u;
	pWork->base_v = spr->sprite.head.tex.v;
#else
	pWork->base_u = spr->sprite.head.tex.u*16.0f;
	pWork->base_v = spr->sprite.head.tex.v*16.0f;
#endif
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_POS_X;
	spr->sprite.pos.y = DEFAULT_POS_Y;
	for ( i = pWork->dup_count.font ; i < ASCCI_MAX ; i++ ){
		pWork->font[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->font[ i ] == NULL ){
			return ( -1 );
		} else {
			pWork->dup_count.font++;
		}
	}
	SK_PrintfNormal2( NULL , pWork->font , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
	SK_PrintfChengColor2( pWork->font , SK_R , SK_G , SK_B , 0 , ASCCI_MAX ); // 一行いっきに書き換え
	SPR_HIDE( spr );
	SK_AllHide( pWork->font , ASCCI_MAX );
	// 国の取得
	spr = L2D_GetObject( pWork->handle_2d, STR_NODE_FONT ) ;
	if ( spr == NULL ){
		SK_Err("font region\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_REGION_POS_X;
	spr->sprite.pos.y = DEFAULT_REGION_POS_Y;
	spr->sprite.col.r = SK_R;
	spr->sprite.col.g = SK_G;
	spr->sprite.col.b = SK_B;
	for ( i = pWork->dup_count.region ; i < MAX_REGION_OBJ ; i++ ){
		pWork->region.obj[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->region.obj[ i ] == NULL ){
			return ( -1 );
		} else {
			pWork->dup_count.region++;
		}
	}
	SK_PrintfNormal( NULL , pWork->region.obj , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , MAX_REGION_OBJ );
	SK_PrintfChengColor2( pWork->region.obj , SK_R , SK_G , SK_B , 0 , MAX_REGION_OBJ ); // 一行いっきに書き換え
	SPR_HIDE( spr );
	// kcej
	spr = L2D_GetObject( pWork->handle_2d , STR_KCEJ );
	if ( spr == NULL ){
		SK_Err("kcej\0");
		return ( -1 );
	}
	SPR_HIDE( spr );
	spr->sprite.col.a = 0;
	pWork->region.kcej = spr;
	pWork->region.kcej->sprite.pos.x = DEFAULT_REGION_POS_X;
	pWork->region.kcej->sprite.pos.y = DEFAULT_REGION_POS_Y;
	// hilight
	spr = L2D_GetObject( pWork->handle_2d , STR_FONT_HILIGHT ) ;
	if ( spr == NULL ){
		SK_Err("hilight\0");
		return ( -1 );
	}
	spr->sprite.col.a = 0;
	spr->sprite.dw = 0.0f;
	pWork->hilight = spr;
	// birth
   spr = L2D_GetObject( pWork->handle_2d , STR_NODE_FONT );
   pWork->base_u = spr->sprite.head.tex.u*16.0f;
   pWork->base_v = spr->sprite.head.tex.v*16.0f;

   // birth position
	spr->sprite.pos.x = DEFAULT_BIRTH_POS_X;
	spr->sprite.pos.y = DEFAULT_BIRTH_POS_Y;
	spr->sprite.dw = FONT_NUM_WIDTH;
	spr->sprite.dh = FONT_NUM_HEIGHT;
	spr->sprite.col.r = SK_R;
	spr->sprite.col.g = SK_G;
	spr->sprite.col.b = SK_B;
	if ( spr == NULL ){
		SK_Err("font birth\0");
		return ( -1 );
	}
	for ( i = pWork->dup_count.birth ; i < MAX_BIRTH ; i ++ )
   {
		pWork->birth.obj[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->birth.obj[ i ] == NULL )
      {
			return ( -1 );
		}
      else 
      {
			pWork->dup_count.birth++;
		}
	}
	SPR_HIDE( spr );
	SK_PrintfChengColor2( pWork->birth.obj , SK_R , SK_G , SK_B , 0 , MAX_BIRTH ); // 一行いっきに書き換え
	SK_AllHide( pWork->birth.obj , MAX_BIRTH );
	// happy birth day
	spr = L2D_GetObject( pWork->handle_2d , STR_HAPPY );
	if ( spr == NULL ){
		SK_Err("happy node\0");
		return ( -1 );
	}
	SPR_HIDE( spr );
	pWork->birth.happy = spr;

	// soft key
	SoftHideShowControl( pWork , 1 );

	pWork->flag |= SK_SPRITE_INIT_OK;

	return( 0 );
}


/* 
   -3 : 上スクロール ＋ 左右スクロール
   -2 : 下スクロール ＋ 左右スクロール
   -1 : スクロール
	0 : ブランク(リピート)
	1 : 通常
	2 : 左下
	3 : 右下
	4 : 上スクロール（左右は普通に移動出来る）
	5 : 下スクロール（左右は普通に移動出来る）
*/
static int soft_key_move_table[ 3 ][ 16 ] = {
	{ -3 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , -3 } ,
	{ -1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,  6 } ,
	{ -2 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , -2 } ,
};

static int soft_key_table[ 3 ][ 16 ] = {
	{ 'A' , 'B' , 'C' , 'D' , 'E' , 'F' , 'G' , 'H' , 'I' , 'J' , 'K' , 'L' , 'M' , '&' , '!' , 0x60 } ,
	{ 'N' , 'O' , 'P' , 'Q' , 'R' , 'S' , 'T' , 'U' , 'V' , 'W' , 'X' , 'Y' , 'Z' , ':' , '?' ,    0 } ,
	{ ' ' , '0' , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '/' , '_' , '.' , ' ' , 0x61 } ,
};

static int soft_key_strcode[ 3 ][ 16 ] = {
  { STR_HI_A , STR_HI_B , STR_HI_C , STR_HI_D , STR_HI_E , STR_HI_F , STR_HI_G , STR_HI_H , STR_HI_I , STR_HI_J , STR_HI_K , STR_HI_L , STR_HI_M ,
	STR_HI_AND , STR_HI_DOKKIRI ,  STR_HI_BS } ,
  { STR_HI_N , STR_HI_O , STR_HI_P , STR_HI_Q , STR_HI_R , STR_HI_S , STR_HI_T , STR_HI_U , STR_HI_V , STR_HI_W , STR_HI_X , STR_HI_Y , STR_HI_Z , 
	STR_HI_CORON , STR_HI_QU , 0 } ,
  { STR_HI_SPACE , STR_HI_ZERO ,  STR_HI_ONE , STR_HI_TWO , STR_HI_THREE , STR_HI_FOUR , STR_HI_FIVE , STR_HI_SIX , STR_HI_SEVEN , STR_HI_EIGHT ,
	STR_HI_NINE , STR_HI_SU , STR_HI_ANDER , STR_HI_DOT , STR_HI_SPACE2 , STR_HI_CANCEL } ,
};

#define SOFT_WIDTH (16)
#define SOFT_HEIGHT (12)
#define SK_HOKAN_RAUTE (4.0f)
#define SK_HILIGHT_A (42)
enum {
	SK_PAD_X_PLUS = 0x1 ,
	SK_PAD_X_SUB  = 0x2 ,
	SK_PAD_Y_PLUS = 0x4 ,
	SK_PAD_Y_SUB  = 0x8 ,
};
#define SOFT_X_MAX (16)
#define SOFT_Y_MAX (3)

static void SoftKeySerch( Work *pWork ) // キーをtableから探して初期化
{
	u_char *ascci;
	int i , j;

	ascci = ( u_char * )GM_MyName;
	for ( i = 0 ; i < SOFT_Y_MAX ; i ++ ){
		for ( j = 0 ; j < SOFT_X_MAX ; j ++ ){
			if ( soft_key_table[ i ][ j ] == ascci[ pWork->name.position ] ){
				pWork->soft.x = j;
				pWork->soft.y = i;
				pWork->soft.add_x = 0;
				pWork->soft.add_y = 0;
			}
		}
	}
}

static inline void SoftHideShowControl( Work *pWork , char mode ) // Sprite 表示 、非表示
{
	SPR_OBJ *spr;

	spr = L2D_GetObject( pWork->handle_2d , soft_key_strcode[ pWork->soft.y ][ pWork->soft.x ] );
	if ( spr != NULL ){
		if ( mode == 0 ){
			SPR_HIDE( spr );
		} else {
			SPR_SHOW( spr );
		}
	}
}

static inline void SoftInitialize( Work *pWork ) // 初期化
{
  /*
	SoftHideShowControl( pWork , 0 );
	pWork->soft.x = 0;
	pWork->soft.y = 0;
	pWork->soft.add_x = 0;
	pWork->soft.add_y = 0;
	SoftHideShowControl( pWork , 1 );
  */
}

static inline void SoftMove( Work *pWork , u_char pad_mode ) // ソフトウェアキーボード移動管理
{
	SoftHideShowControl( pWork , 0 );
	switch( soft_key_move_table[ pWork->soft.y ][ pWork->soft.x ] ){
	case -3 : // 上スクロール ＋ 左右スクロール
		if ( pad_mode & SK_PAD_X_PLUS ){
			if ( pWork->soft.x > 0 ){
				pWork->soft.x = 0;
			} else {
				pWork->soft.x++;
			}
		} else if ( pad_mode & SK_PAD_X_SUB ){
			if ( pWork->soft.x > 0 ){
				pWork->soft.x--;
			} else {
				pWork->soft.x = SOFT_X_MAX - 1;
			}
		} else if ( pad_mode & SK_PAD_Y_PLUS ){
			pWork->soft.y++;
		} else if ( pad_mode & SK_PAD_Y_SUB ){
			pWork->soft.y = SOFT_Y_MAX - 1;
		}
		if ( soft_key_move_table[ pWork->soft.y ][ pWork->soft.x ] == 6 ){
			pWork->soft.x += pWork->soft.add_x;
			pWork->soft.y += pWork->soft.add_y;
		}
		break;
	case -2 : // 下スクロール ＋ 左右スクロール
		if ( pad_mode & SK_PAD_X_PLUS ){
			if ( pWork->soft.x > 0 ){
				pWork->soft.x = 0;
			} else {
				pWork->soft.x++;
			}
		} else if ( pad_mode & SK_PAD_X_SUB ){
			if ( pWork->soft.x > 0 ){
				pWork->soft.x--;
			} else {
				pWork->soft.x = SOFT_X_MAX - 1;
			}
		} else if ( pad_mode & SK_PAD_Y_PLUS ){
			pWork->soft.y = 0;
		} else if ( pad_mode & SK_PAD_Y_SUB ){
			pWork->soft.y--;
		}
		if ( soft_key_move_table[ pWork->soft.y ][ pWork->soft.x ] == 6 ){
			pWork->soft.x += pWork->soft.add_x;
			pWork->soft.y += pWork->soft.add_y;
		}
		break;
	case -1 : // スクロール
		if ( pad_mode & SK_PAD_X_PLUS ){
			if ( pWork->soft.x > 0 ){
				pWork->soft.x = 0;
			} else {
				pWork->soft.x++;
			}
		} else if ( pad_mode & SK_PAD_X_SUB ){
			if ( pWork->soft.x > 0 ){
				pWork->soft.x--;
			} else {
				pWork->soft.x = SOFT_X_MAX - 1;
			}
		} else if ( pad_mode & SK_PAD_Y_PLUS ){
			pWork->soft.y++;
		} else if ( pad_mode & SK_PAD_Y_SUB ){
			pWork->soft.y--;
		}
		if ( soft_key_move_table[ pWork->soft.y ][ pWork->soft.x ] == 6 ){
			pWork->soft.x += pWork->soft.add_x;
			pWork->soft.y += pWork->soft.add_y;
		}
		break;
	case 0 : // ブランク(リピート)
		pWork->soft.x += pWork->soft.add_x;
		pWork->soft.y += pWork->soft.add_y;
		break;
	case 6 : // ブランク(リピート)
		if ( pWork->soft.add_x > 0 ){
			pWork->soft.x = 0;
			pWork->soft.y += pWork->soft.add_y;
		}
		break;
	case 1 : // 通常
		pWork->soft.x += pWork->soft.add_x;
		pWork->soft.y += pWork->soft.add_y;
		if ( soft_key_move_table[ pWork->soft.y ][ pWork->soft.x ] == 6 ){
			if ( pad_mode & SK_PAD_X_PLUS ){
				pWork->soft.x = 0;
			} else if ( pad_mode & SK_PAD_X_SUB ){
				pWork->soft.x = SOFT_X_MAX - 1;
			}
		}
		break;
	case 2 : // 左下
		pWork->soft.x--;
		pWork->soft.y++;
		break;
	case 3 : // 右下
		pWork->soft.x++;
		pWork->soft.y++;
		break;
	case 4 : // 上スクロール（左右は普通に移動出来る）
		if ( pad_mode & SK_PAD_Y_PLUS ){
			pWork->soft.y++;
		} else if ( pad_mode & SK_PAD_Y_SUB ){
			pWork->soft.y = SOFT_Y_MAX - 1;
		} else if ( pad_mode & SK_PAD_X_PLUS ){
			pWork->soft.x++;
		} else if ( pad_mode & SK_PAD_X_SUB ){
			pWork->soft.x--;
		}
		break;
	case 5 : // 下スクロール（左右は普通に移動出来る）
		if ( pad_mode & SK_PAD_Y_PLUS ){
			pWork->soft.y = 0;
		} else if ( pad_mode & SK_PAD_Y_SUB ){
			pWork->soft.y--;
		} else if ( pad_mode & SK_PAD_X_PLUS ){
			pWork->soft.x++;
		} else if ( pad_mode & SK_PAD_X_SUB ){
			pWork->soft.x--;
		}
		break;
	}
	SoftHideShowControl( pWork , 1 );
	pWork->soft.add_x = 0;
	pWork->soft.add_y = 0;
	SE_RGB_TUNE();
}

static void SoftKey( Work *pWork ) // ソフトキーボード
{
	u_char *ascci;
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( status == L2D_STAT_BUSY ) ){
		return;
	}
	ascci = ( u_char * )GM_MyName;
   //BP added case for keystrokes to provide input.  Takes precedence over all controller input.
   //There is a good amount of redundant code here, but that's because it's copied almost verbatim
   //from the controller entry code below and there was unnecessary duplication in it as well.
   //Don't want to miss something subtle and introduce bugs with this addition...
   if( pWork->bp_keyret == 0 )
   {
      //No matter what, a keyboard press teleports the controller input cursor to the ENTER button.
      SoftHideShowControl( pWork , 0 );
      pWork->soft.x = SOFT_X_MAX - 1;
      pWork->soft.y = SOFT_Y_MAX - 1;
      pWork->soft.add_x = 0;
      pWork->soft.add_y = 0;
      SoftHideShowControl( pWork , 1 );
      if ( pWork->name.position >= ASCCI_MAX - 1 ) // last
      {
         if( pWork->bp_keypress == 0x08 )  //back space
         {
            ascci[ pWork->name.position ] = 0x80; // space 
            pWork->name.position--;
            ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
            SE_CANCEL();
         }
         else if( pWork->bp_keypress == 0x0a )   //enter
         {
            ascci[ pWork->name.position ] = 0x80;
            NoneName( pWork );
            if ( NgWordCheck( pWork ) ){ // 卑猥チェック
               ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
               return;
            }
            pWork->flag |= SK_NAME_ENTRY_ESCAPE;
            pWork->flag &= ~SK_NAME_ENTRY;
            pWork->act = ( void * )NemaEntryEscapeAct;
            StaffNameSwap( pWork );
            // 移動
            pWork->normal_mode.old_position = pWork->normal_mode.position;
            pWork->normal_mode.position++;
            pWork->normal_mode.position++;
            SE_OK();
            // layout
            pWork->action = STR_CLOSE_KB;
            pWork->flag |= SK_ACTION_START;
            pWork->flag &= ~SK_ACTION_OK;
            L2D_BreakAction( pWork->handle_2d );
            L2D_EvokeAction( pWork->handle_2d , pWork->action );
            pWork->flag |= SK_ACTION_OK;
            pWork->flag &= ~SK_ACTION_START;
            return;
         }
      }
      else
      {
         if( pWork->bp_keypress == 0x08 )  //back space
         {
            if ( pWork->name.position > 0 )
            {
               ascci[ pWork->name.position ] = 0x80; // space 
               pWork->name.position--;
               ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
               SE_CANCEL();
            }
         }
         else if( pWork->bp_keypress == 0x0a )   //end
         {
            ascci[ pWork->name.position + 1 ] = 0x80;
            NoneName( pWork );
            if ( NgWordCheck( pWork ) )
            { // 卑猥チェック
               pWork->name.position += 1;
               ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
               return;
            }
            pWork->name.position++;
            ascci[ pWork->name.position ] = 0x80;
            pWork->flag |= SK_NAME_ENTRY_ESCAPE;
            pWork->flag &= ~SK_NAME_ENTRY;
            pWork->act = ( void * )NemaEntryEscapeAct;
            StaffNameSwap( pWork );
            // 移動
            pWork->normal_mode.old_position = pWork->normal_mode.position;
            pWork->normal_mode.position++;
            pWork->normal_mode.position++;
            SE_OK();
            // layout
            pWork->action = STR_CLOSE_KB;
            pWork->flag |= SK_ACTION_START;
            pWork->flag &= ~SK_ACTION_OK;
            L2D_BreakAction( pWork->handle_2d );
            L2D_EvokeAction( pWork->handle_2d , pWork->action );
            pWork->flag |= SK_ACTION_OK;
            pWork->flag &= ~SK_ACTION_START;
            return;
         }
         else if ( pWork->name.position < ASCCI_MAX - 2 ) //general add a character case
         {
            ascci[ pWork->name.position + 1 ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ];
            ascci[ pWork->name.position ] = pWork->bp_keypress;
            pWork->name.position++;
            SE_TYPE();
         }
         else // (second to last)
         {
            ascci[ pWork->name.position ] = pWork->bp_keypress;
            pWork->name.position++;
            ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
            SoftHideShowControl( pWork , 0 );
            SoftHideShowControl( pWork , 1 );
            SE_TYPE();
         }
      }
   }
	else
   if ( pWork->name.position >= ASCCI_MAX - 1 ){ // last
		if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status[ 1 ] & PAD_U ) {
			pWork->pad_status[ 1 ] = 0;
			pWork->soft.add_y = -1;
			SoftMove( pWork , SK_PAD_Y_SUB );
			ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
		} else if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status[ 1 ] & PAD_D ) {
			pWork->pad_status[ 1 ] = 0;
			pWork->soft.add_y = 1;
			SoftMove( pWork , SK_PAD_Y_PLUS );
			ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
		} else if ( GV_PadDataDirect[ 0 ].press & SELECT ){
			if ( ascci[ pWork->name.position ] == 0x60 ){ // back space
				if ( pWork->name.position > 0 ){
					ascci[ pWork->name.position ] = 0x80; // space 
					pWork->name.position--;
					ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
					SE_CANCEL();
				}
			} else if ( ascci[ pWork->name.position ] == 0x61 ){ // end
				ascci[ pWork->name.position ] = 0x80;
				NoneName( pWork );
				if ( NgWordCheck( pWork ) ){ // 卑猥チェック
					ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
					return;
				}
				pWork->flag |= SK_NAME_ENTRY_ESCAPE;
				pWork->flag &= ~SK_NAME_ENTRY;
				pWork->act = ( void * )NemaEntryEscapeAct;
				StaffNameSwap( pWork );
				// 移動
				pWork->normal_mode.old_position = pWork->normal_mode.position;
				pWork->normal_mode.position++;         // BP JG - skip to sex
            pWork->normal_mode.position++;         // BP JG - skip to birthday
				SE_OK();
				// layout
				pWork->action = STR_CLOSE_KB;
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				L2D_BreakAction( pWork->handle_2d );
				L2D_EvokeAction( pWork->handle_2d , pWork->action );
				pWork->flag |= SK_ACTION_OK;
				pWork->flag &= ~SK_ACTION_START;
				return;
			}
		} else if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
			if ( pWork->name.position > 0 ){
				ascci[ pWork->name.position ] = 0x80; // space 
				pWork->name.position--;
				ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
				SE_CANCEL();
			}
		} else if ( GV_PadDataDirect[ 0 ].press & ESCAPE ){ // 一発ぬけ
			if ( ( pWork->soft.x == SOFT_X_MAX - 1 ) && ( pWork->soft.y == SOFT_Y_MAX - 1 ) ){
				ascci[ pWork->name.position ] = 0x80;
				NoneName( pWork );
				if ( NgWordCheck( pWork ) ){ // 卑猥チェック
					ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
					return;
				}
				pWork->flag |= SK_NAME_ENTRY_ESCAPE;
				pWork->flag &= ~SK_NAME_ENTRY;
				pWork->act = ( void * )NemaEntryEscapeAct;
				StaffNameSwap( pWork );
				// 移動
				pWork->normal_mode.old_position = pWork->normal_mode.position;
				pWork->normal_mode.position++;
				SE_OK();
				// layout
				pWork->action = STR_CLOSE_KB;
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				L2D_BreakAction( pWork->handle_2d );
				L2D_EvokeAction( pWork->handle_2d , pWork->action );
				pWork->flag |= SK_ACTION_OK;
				pWork->flag &= ~SK_ACTION_START;
				return;
			} else { // enter に いく
				SoftHideShowControl( pWork , 0 );
				pWork->soft.x = SOFT_X_MAX - 1;
				pWork->soft.y = SOFT_Y_MAX - 1;
				pWork->soft.add_x = 0;
				pWork->soft.add_y = 0;
				ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
				SoftHideShowControl( pWork , 1 );
				SE_RGB_TUNE();
			}
		}
	} else if ( ( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status[ 1 ] & PAD_L ) ){
		pWork->pad_status[ 1 ] = 0;
		pWork->soft.add_x = -1;
		SoftMove( pWork , SK_PAD_X_SUB );
		ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
	} else if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status[ 1 ] & PAD_U ) {
		pWork->pad_status[ 1 ] = 0;
		pWork->soft.add_y = -1;
		SoftMove( pWork , SK_PAD_Y_SUB );
		ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
	} else if ( ( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status[ 1 ] & PAD_R ) ){
		pWork->pad_status[ 1 ] = 0;
		pWork->soft.add_x = 1;
		SoftMove( pWork , SK_PAD_X_PLUS );
		ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
	} else if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status[ 1 ] & PAD_D ) {
		pWork->pad_status[ 1 ] = 0;
		pWork->soft.add_y = 1;
		SoftMove( pWork , SK_PAD_Y_PLUS );
		ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
	} else if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		if ( ascci[ pWork->name.position ] == 0x60 ){ // back space
			if ( pWork->name.position > 0 ){
				ascci[ pWork->name.position ] = 0x80; // space 
				pWork->name.position--;
				ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
				SE_CANCEL();
			}
		} else if ( ascci[ pWork->name.position ] == 0x61 ){ // end
			ascci[ pWork->name.position + 1 ] = 0x80;
			NoneName( pWork );
			if ( NgWordCheck( pWork ) ){ // 卑猥チェック
				pWork->name.position += 1;
				ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
				return;
			}
			pWork->name.position++;
			ascci[ pWork->name.position ] = 0x80;
			pWork->flag |= SK_NAME_ENTRY_ESCAPE;
			pWork->flag &= ~SK_NAME_ENTRY;
			pWork->act = ( void * )NemaEntryEscapeAct;
			StaffNameSwap( pWork );
			// 移動
			pWork->normal_mode.old_position = pWork->normal_mode.position;
			pWork->normal_mode.position++;               // BP JG - advance to sex.
         pWork->normal_mode.position++;               // BP JG - skip sex, next to birth
			SE_OK();
			// layout
			pWork->action = STR_CLOSE_KB;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			L2D_BreakAction( pWork->handle_2d );
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
			return;
		} else if ( pWork->name.position < ASCCI_MAX - 2 ){
			ascci[ pWork->name.position + 1 ] = 0x20;//ascci[ pWork->name.position ]; 
			pWork->name.position++;
			ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
			SE_TYPE();
		} else { // last
			ascci[ pWork->name.position + 1 ] = 0x20;
			pWork->name.position++;
			SoftHideShowControl( pWork , 0 );
			pWork->soft.x = SOFT_X_MAX - 1;
			pWork->soft.y = SOFT_Y_MAX - 1;
			pWork->soft.add_x = 0;
			pWork->soft.add_y = 0;
			ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
			SoftHideShowControl( pWork , 1 );
			SE_TYPE();
		}
	} else if ( GV_PadDataDirect[ 0 ].press & ESCAPE ){ // 一発ぬけ
		if ( ( pWork->soft.x == SOFT_X_MAX - 1 ) && ( pWork->soft.y == SOFT_Y_MAX - 1 ) ){
			ascci[ pWork->name.position + 1 ] = 0x80;
			NoneName( pWork );
			if ( NgWordCheck( pWork ) ){ // 卑猥チェック
				pWork->name.position += 1;
				ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
				return;
			}
			pWork->name.position++;
			ascci[ pWork->name.position ] = 0x80;
			pWork->flag |= SK_NAME_ENTRY_ESCAPE;
			pWork->flag &= ~SK_NAME_ENTRY;
			pWork->act = ( void * )NemaEntryEscapeAct;
			StaffNameSwap( pWork );
			// 移動
			pWork->normal_mode.old_position = pWork->normal_mode.position;
			pWork->normal_mode.position++;
         pWork->normal_mode.position++;
			SE_OK();
			// layout
			pWork->action = STR_CLOSE_KB;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			L2D_BreakAction( pWork->handle_2d );
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
			return;
		} else { // enter に いく
			SoftHideShowControl( pWork , 0 );
			pWork->soft.x = SOFT_X_MAX - 1;
			pWork->soft.y = SOFT_Y_MAX - 1;
			pWork->soft.add_x = 0;
			pWork->soft.add_y = 0;
			ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
			SoftHideShowControl( pWork , 1 );
			SE_RGB_TUNE();
		}
	} else if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		if ( pWork->name.position > 0 ){
			ascci[ pWork->name.position ] = 0x80; // space 
			pWork->name.position--;
			ascci[ pWork->name.position ] = soft_key_table[ pWork->soft.y ][ pWork->soft.x ] ;
			SE_CANCEL();
		}
	}
	// auto mode
	if ( pWork->hold_time[ 1 ] >= DIRECT_TICK( 20 ) ){
		pWork->pad_status[ 1 ] = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time[ 1 ] -= DIRECT_TICK( 4 );
	} else if ( pWork->pad_check[ 1 ] & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time[ 1 ]++;
	} else {
		pWork->pad_check[ 1 ] = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_L | PAD_R | PAD_U | PAD_D ) ) ){
		pWork->hold_time[ 1 ] = 0;
		pWork->pad_status[ 1 ] = 0;
	}
}

static void Update( Work *pWork ) // update
{
	u_char  *ascci;
	int alpha;
	int width;
	int tmp;
//	int i;

	ascci = ( u_char * )GM_MyName;
	
	SK_PrintfNormal2( ascci , pWork->font , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
	SK_AllShow( pWork->font , ASCCI_MAX );
	if ( pWork->flag & SK_NAME_ENTRY ){
		// hilight
		pWork->hilight->sprite.pos.x = pWork->font[ 0 ]->sprite.pos.x - 1;
		pWork->hilight->sprite.pos.x += SK_PrintfNormalWidth3( ascci , pWork->name.position );
#if 0
		for ( i = 0 ; i < pWork->name.position ; i ++ ){
			tmp = SK_PrintfNormalWidth( ascci[ i ] , pWork->font[ i ] );
			tmp = ( tmp == 0xff ? 14 : tmp );
			pWork->hilight->sprite.pos.x += tmp; 
		}
#endif
		pWork->hilight->sprite.pos.y =  pWork->font[ pWork->name.position ]->sprite.pos.y - 2;
		// 補間
		tmp = SK_PrintfNormalWidth( ascci[ pWork->name.position ] , pWork->font[ pWork->name.position ] );
		tmp = ( tmp == 0xff ? 14 : tmp );
		width = tmp + 1;
		pWork->hilight->sprite.dw += ( width - pWork->hilight->sprite.dw ) / SK_HOKAN_RAUTE;
		pWork->hilight->sprite.dh = pWork->font[ pWork->name.position ]->sprite.dh;
      pWork->hilight->sprite.dw +=0.5f;      // BP JG - make the cursor a little bigger, looks better.
		// alpha
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha >= SK_HILIGHT_A ){
			alpha = SK_HILIGHT_A;
		}
		pWork->hilight->sprite.col.a = ( u_char )alpha;
		SPR_SHOW( pWork->hilight );
	}
}

static inline void RegionHilight( Work *pWork , char *buf , char mode )
{
	int len;
	int width[ 2 ];
	int tmp , tmp2;
	int alpha;
	int i;

	if ( pWork->flag & SK_REGION_ENTRY ){
		switch ( mode ){
		case 0 : // normal
			// hilight
			pWork->hilight->sprite.pos.x = pWork->region.obj[ 0 ]->sprite.pos.x - 1;
			pWork->hilight->sprite.pos.y =  pWork->region.obj[ 0 ]->sprite.pos.y - 2;
			pWork->hilight->sprite.dh = pWork->region.obj[ 0 ]->sprite.dh;
			len = strlen( buf );
			width[ 0 ] = 0.0f;
			for ( i = 0 ; i < len ; i ++ ){
				tmp = SK_PrintfNormalWidth_NameSelect( buf[ i ] , pWork->region.obj[ i ] );
				tmp = ( tmp == 0xff ? 6 : tmp );
				width[ 0 ] += tmp;
			}
//			pWork->hilight->sprite.dw += ( width[ 0 ] - pWork->hilight->sprite.dw ) / SK_HOKAN_RAUTE;
         pWork->hilight->sprite.dw = (width[ 0 ]*TARGET_ASPECT_X)+1;
			// alpha
			alpha = ( int )pWork->hilight->sprite.col.a;
			alpha += IN_FADE_SPEED;
			if ( alpha >= SK_HILIGHT_A ){
				alpha = SK_HILIGHT_A;
			}
			pWork->hilight->sprite.col.a = ( u_char )alpha;
			SPR_SHOW( pWork->hilight );
			break;
		case 1 : // kcej
			// hilight
			pWork->hilight->sprite.pos.x = pWork->region.kcej->sprite.pos.x - 1;
			pWork->hilight->sprite.pos.y =  pWork->region.kcej->sprite.pos.y - 2;
			width[ 0 ] = pWork->region.kcej->sprite.dw + 1;
			pWork->hilight->sprite.dw += ( width[ 0 ] - pWork->hilight->sprite.dw ) / SK_HOKAN_RAUTE;
			pWork->hilight->sprite.dh = pWork->region.kcej->sprite.dh + 2;
			// alpha
			alpha = ( int )pWork->hilight->sprite.col.a;
			alpha += IN_FADE_SPEED;
			if ( alpha >= SK_HILIGHT_A ){
				alpha = SK_HILIGHT_A;
			}
			pWork->hilight->sprite.col.a = ( u_char )alpha;
			SPR_SHOW( pWork->hilight );
			break;
		case 3 : // 改行ふくみ
			// hilight
			pWork->hilight->sprite.pos.x = pWork->region.obj[ 0 ]->sprite.pos.x - 1;
			pWork->hilight->sprite.pos.y =  pWork->region.obj[ 0 ]->sprite.pos.y - 2;
			pWork->hilight->sprite.dh = pWork->region.obj[ 0 ]->sprite.dh * 2;
			tmp2 = 0;
			while ( buf[ tmp2 ] != 0x0a ){
				tmp2++;
			}
			width[ 0 ] = 0.0f;
			for ( i = 0 ; i < tmp2 ; i ++ ){
				tmp = SK_PrintfNormalWidth_NameSelect( buf[ i ] , pWork->region.obj[ i ] );
				tmp = ( tmp == 0xff ? 6 : tmp );
				width[ 0 ] += tmp;
			}
			i = tmp2 + 1;
			width[ 1 ] = 0.0f;
			len = strlen( buf );
			for ( ; i < len ; i ++ ){
				tmp = SK_PrintfNormalWidth_NameSelect( buf[ i ] , pWork->region.obj[ i ] );
				tmp = ( tmp == 0xff ? 6 : tmp );
				width[ 1 ] += tmp;
			}
			width[ 0 ] = width[ 0 ] < width[ 1 ] ? width[ 1 ] : width[ 0 ];
			//pWork->hilight->sprite.dw += ( width[ 0 ] - pWork->hilight->sprite.dw ) / SK_HOKAN_RAUTE;
         pWork->hilight->sprite.dw = (width[ 0 ]*TARGET_ASPECT_X)+2;
			// alpha
			alpha = ( int )pWork->hilight->sprite.col.a;
			alpha += IN_FADE_SPEED;
			if ( alpha >= SK_HILIGHT_A ){
				alpha = SK_HILIGHT_A;
			}
			pWork->hilight->sprite.col.a = ( u_char )alpha;
			SPR_SHOW( pWork->hilight );
		}
		
	}
}

static void NoneName( Work *pWork ) // 名無の時？にする
{
	u_char *ascci;
	int  i;

	ascci = ( char * )GM_MyName;

	/* スペース除去 */
	for ( i=pWork->name.position ; i>=0 ; --i ){
		if ( ascci[i] == 0x20 || ascci[i] == 0x60 || ascci[i] == 0x61 || ascci[i] == 0x80 ){
			ascci[i] = 0x80 ;
			pWork->name.position-- ;
		} else {
			break ;
		}
	}

	/* 名無しであれば？を挿入 */
	if ( pWork->name.position <= 0 && ascci[ 0 ] == 0x80 ) {
		pWork->name.position = 0 ;
		ascci[ 0 ] = '?';
		ascci[ 1 ] = 0x80;
	}
}

static void SelectModeRecav( Work *pWork )
{
	switch( pWork->normal_mode.old_position ){
	case 0 : // name entry
		pWork->flag |= SK_NAME_ENTRY_ESCAPE;
		pWork->flag &= ~SK_NAME_ENTRY;
		pWork->flag &= ~SK_FADE_OK;
		pWork->act = ( void * )NemaEntryEscapeAct;
		break;
	case 1 : // sex
		pWork->act = ( void * )SexEntryEscapeAct;
		pWork->flag |= SK_SEX_ENTRY_ESCAPE;
		pWork->flag &= ~SK_SEX_ENTRY;
		pWork->flag &= ~SK_FADE_OK;
		break;
	case 2 : // birth
		pWork->birth.phase = 0;
		pWork->act = ( void * )BirthEntryEscapeAct;
		pWork->flag |= SK_BIRTH_ENTRY_ESCAPE;
		pWork->flag &= ~SK_BIRTH_ENTRY;
		pWork->flag &= ~SK_FADE_OK;
		pWork->birth.flag |= SK_BIRTH_FIRST_INPUT;
		break;
	case 3 : // blood
		pWork->act = ( void * )BloodEntryEscapeAct;
		pWork->flag |= SK_BLOOD_ENTRY_ESCAPE;
		pWork->flag &= ~SK_BLOOD_ENTRY;
		pWork->flag &= ~SK_FADE_OK;
		break;
	case 4 :
		pWork->act = ( void * )RegionEntryEscapeAct;
		pWork->flag |= SK_REGION_ENTRY_ESCAPE;
		pWork->flag &= ~SK_REGION_ENTRY;
		pWork->flag &= ~SK_FADE_OK;
		pWork->hold_time[ 2 ] = 0;
		break;
	}
}

#define MAX_POSITION (5)
static void PadControlNormal( Work *pWork )
{
	if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status[ 0 ] & PAD_U ){
		pWork->pad_status[ 0 ] = 0;
		if ( pWork->normal_mode.position > 0 )
      {
			pWork->normal_mode.old_position = pWork->normal_mode.position;
			pWork->normal_mode.position--;

         if ( pWork->normal_mode.position == 1 )         //sex
            pWork->normal_mode.position--;

         if ( pWork->normal_mode.position == 3 )         //sex
            pWork->normal_mode.position--;


			pWork->flag &= ~SK_FADE_OK;
			SE_SEL();
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status[ 0 ] & PAD_D )
   {
		pWork->pad_status[ 0 ] = 0;
		if ( pWork->normal_mode.position < MAX_POSITION )
      {
			pWork->normal_mode.old_position = pWork->normal_mode.position;
			pWork->normal_mode.position++;
			
         if ( pWork->normal_mode.position == 1 )         //sex
			   pWork->normal_mode.position++;

         if ( pWork->normal_mode.position == 3 )         //sex
            pWork->normal_mode.position++;


         pWork->flag &= ~SK_FADE_OK;
			SE_SEL();
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		if ( pWork->normal_mode.position > 0 )
      {
			pWork->normal_mode.old_position = pWork->normal_mode.position;

         if ( pWork->normal_mode.position== 5 )
         {
            pWork->normal_mode.position--;         // BP JG - back to nationality
         }
         else
         {
            pWork->normal_mode.position--;         // BP JG - back to previous which was sex
            pWork->normal_mode.position--;         // BP JG - but keep going to name
         }

			pWork->flag &= ~SK_FADE_OK;
			SE_SEL();
		}
	}
	if ( ( pWork->normal_mode.position != MAX_POSITION ) || !( pWork->flag & SK_OK_MODE ) )
   {
		if ( ( GV_PadDataDirect[ 0 ].press & PAD_OK/* PAD_A でした yano */ ) || ( ( pWork->normal_mode.position == MAX_POSITION ) && !( pWork->flag & SK_OK_MODE ) ) ){
//		if ( pWork->normal_mode.old_position != pWork->normal_mode.position ){
//			pWork->flag &= ~SK_NORMAL_MODE;
#if 0
		switch( pWork->normal_mode.old_position ){
		case 0 : // name entry
			// 消す
			ascci = ( char * )GM_MyName;
			ascci[ pWork->name.position + 1 ] = 0x80;
				NoneName( pWork );
			if ( NgWordCheck( pWork ) ){
				pWork->normal_mode.position = pWork->normal_mode.old_position;
				return;
			}
			StaffNameSwap( pWork );
			pWork->act = ( void * )NemaEntryEscapeAct;
			pWork->flag |= SK_NAME_ENTRY_ESCAPE;
			break;
		case 1 : // sex
			pWork->act = ( void * )SexEntryEscapeAct;
			pWork->flag |= SK_SEX_ENTRY_ESCAPE;
			break;
		case 2 : // birth
			pWork->act = ( void * )BirthEntryEscapeAct;
			pWork->flag |= SK_BIRTH_ENTRY_ESCAPE;
			break;
		case 3 : // blood
			pWork->act = ( void * )BloodEntryEscapeAct;
			pWork->flag |= SK_BLOOD_ENTRY_ESCAPE;
			break;
		case 4 : // region
			pWork->act = ( void * )RegionEntryEscapeAct;
			pWork->flag |= SK_REGION_ENTRY_ESCAPE;
			break;
		case 5 : // ok
			pWork->act = ( void * )OkEntryEscapeAct;
			pWork->flag |= SK_OK_ENTRY_ESCAPE;
			break;
		}
#endif	
			// flag
			pWork->flag &= ~SK_NORMAL_MODE;
			pWork->flag &= ~SK_OK_MODE;
			pWork->act = ( void * )PositionAct;
			pWork->hold_time[ 2 ] = 0;
			// non
			pWork->flag &= ~SK_NAME_ENTRY;
			pWork->flag &= ~SK_SEX_ENTRY;
			pWork->flag &= ~SK_BIRTH_ENTRY;
			pWork->flag &= ~SK_BLOOD_ENTRY;
			pWork->flag &= ~SK_REGION_ENTRY;
			pWork->flag &= ~SK_FADE_OK;
			if ( pWork->normal_mode.position != 5 ){
				SE_OK();
			}
		}
	}
	if ( ( pWork->normal_mode.position == MAX_POSITION - 1 ) && ( pWork->flag & SK_OK_MODE ) ){ 
		pWork->act = ( void * )OkEntryEscapeAct;
		pWork->flag &= ~SK_OK_MODE;
		pWork->flag &= ~SK_FADE_OK;
		pWork->flag |= SK_OK_ENTRY_ESCAPE;
		return;
	}
	// auto mode
	if ( pWork->hold_time[ 0 ] >= DIRECT_TICK( 20 ) ){
		pWork->pad_status[ 0 ] = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time[ 0 ] -= DIRECT_TICK( 3 );
	} else if ( pWork->pad_check[ 0 ] & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time[ 0 ]++;
	} else {
		pWork->pad_check[ 0 ] = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_D | PAD_U ) ) ){
		pWork->hold_time[ 0 ] = 0;
		pWork->pad_status[ 0 ] = 0;
	}
}

#if 0
static void PadControlNameEntry( Work *pWork )
{
	static char ForWardOffset[]  = { 33 , 1 , 4 , 1 , 1 , 1 , 1 , 6 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 12 , 
									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 38 , 1 , 4 , 1 , 1 , 1 , 33 ,
 									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , -42 , 1 , 1 , 1 , 1 , 31 ,
									 1 , -65 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,  
									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 5 , 1 , 1 , 1 , 2 , 1 };

	static char BackWardOffset[] = { 31 , 2 , 1 , 1 , 1 , 1 , 4 , 1 , 1 , 1 , 1 , 1 , 1 , 6 , 1 , 1 , 
									 -42 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 12 , 1 , 1 , 1 , 4 ,  
									 1 , 33 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,  
									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 38 ,
									 33 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
 									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 31 , 1 , };
	u_char *ascci;
	int tmp;

	ascci = ( u_char * )GM_MyName;

	if ( pWork->name.position >= ASCCI_MAX - 1 ){ // last
		if ( GV_PadDataDirect[ 0 ].press & SELECT ){
			if ( ascci[ pWork->name.position ] == 0x61 ){ // end
				ascci[ pWork->name.position ] = 0x80;
				NoneName( pWork );
				if ( NgWordCheck( pWork ) ){ // 卑猥チェック
					return;
				}
				pWork->flag |= SK_NAME_ENTRY_ESCAPE;
				pWork->flag &= ~SK_NAME_ENTRY;
				pWork->act = ( void * )NemaEntryEscapeAct;
				StaffNameSwap( pWork );
				SE_OK();
				return;
			}
		} else if ( GV_PadDataDirect[ 0 ].press & PAD_STA ){ // 一発ぬけ
			ascci[ pWork->name.position ] = 0x80;
			NoneName( pWork );
			if ( NgWordCheck( pWork ) ){ // 卑猥チェック
				return;
			}
			pWork->flag |= SK_NAME_ENTRY_ESCAPE;
			pWork->flag &= ~SK_NAME_ENTRY;
			pWork->act = ( void * )NemaEntryEscapeAct;
			StaffNameSwap( pWork );
			SE_OK();
			return;
		} else if ( GV_PadDataDirect[ 0 ].press & CANCEL ){ // 削除
			if ( pWork->name.position > 0 ){
				ascci[ pWork->name.position ] = 0x80; // space 
				pWork->name.position--;
				SE_CANCEL();
			} else if ( ascci[ pWork->name.position ] != 0x20 ){
				ascci[ pWork->name.position ] = 0x20; // space 
				SE_CANCEL();
			}
		}
	} else if ( ( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status[ 1 ] & PAD_L ) ||  
		 ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status[ 1 ] & PAD_U ) ){
		pWork->pad_status[ 1 ] = 0;
		SE_SEL();
		tmp = ascci[ pWork->name.position ];
		ascci[ pWork->name.position ] -= BackWardOffset[ ascci[ pWork->name.position ] - 0x20 ];
		if ( ascci[ pWork->name.position ] < 0x20 ){
			ascci[ pWork->name.position ] = 0x80 + ( ascci[ pWork->name.position ] - tmp );
		}
	} else if ( ( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status[ 1 ] & PAD_R ) || 
		 ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status[ 1 ] & PAD_D ) ){
		pWork->pad_status[ 1 ] = 0;
		SE_SEL();
		ascci[ pWork->name.position ] += ForWardOffset[ ascci[ pWork->name.position ] - 0x20 ];
		if ( ascci[ pWork->name.position ] >= 0x7f ){ // loop
			ascci[ pWork->name.position ] = 0x20 + ( ascci[ pWork->name.position ] - 0x7f );
		}
	} else if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		if ( ascci[ pWork->name.position ] == 0x60 ){ // back space
			if ( pWork->name.position > 0 ){
				ascci[ pWork->name.position ] = 0x80; // space 
				pWork->name.position--;
				SE_CANCEL();
			}
		} else if ( ascci[ pWork->name.position ] == 0x61 ){ // end
			ascci[ pWork->name.position + 1 ] = 0x80;
			NoneName( pWork );
			if ( NgWordCheck( pWork ) ){ // 卑猥チェック
				return;
			}
			pWork->name.position++;
			ascci[ pWork->name.position ] = 0x80;
			pWork->flag |= SK_NAME_ENTRY_ESCAPE;
			pWork->flag &= ~SK_NAME_ENTRY;
			pWork->act = ( void * )NemaEntryEscapeAct;
			StaffNameSwap( pWork );
			// 移動
			pWork->normal_mode.old_position = pWork->normal_mode.position;
			pWork->normal_mode.position++;
			SE_OK();
			return;
		} else if ( pWork->name.position < ASCCI_MAX - 2 ){
			ascci[ pWork->name.position + 1 ] = 0x20;//ascci[ pWork->name.position ]; 
			pWork->name.position++;
			SE_TYPE();
		} else { // last
			ascci[ pWork->name.position + 1 ] = 0x61;
			pWork->name.position++;
			SE_TYPE();
		}
	} else if ( GV_PadDataDirect[ 0 ].press & PAD_STA ){ // 一発ぬけ
			ascci[ pWork->name.position + 1 ] = 0x80;
			NoneName( pWork );
			if ( NgWordCheck( pWork ) ){ // 卑猥チェック
				return;
			}
			pWork->name.position++;
			ascci[ pWork->name.position ] = 0x80;
			pWork->flag |= SK_NAME_ENTRY_ESCAPE;
			pWork->flag &= ~SK_NAME_ENTRY;
			pWork->act = ( void * )NemaEntryEscapeAct;
			StaffNameSwap( pWork );
			// 移動
			pWork->normal_mode.old_position = pWork->normal_mode.position;
			pWork->normal_mode.position++;
			SE_OK();
			return;
	} else if ( GV_PadDataDirect[ 0 ].press & CANCEL ){ // 削除
		if ( pWork->name.position > 0 ){
			ascci[ pWork->name.position ] = 0x80; // space 
			pWork->name.position--;
			SE_CANCEL();
		} else if ( ascci[ pWork->name.position ] != 0x20 ){
			ascci[ pWork->name.position ] = 0x20; // space 
			SE_CANCEL();
		}
	}

	// auto mode
	if ( pWork->hold_time[ 1 ] >= DIRECT_TICK( 4 ) ){
		pWork->pad_status[ 1 ] = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time[ 1 ] -= DIRECT_TICK( 3 );
	} else if ( pWork->pad_check[ 1 ] & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time[ 1 ]++;
	} else {
		pWork->pad_check[ 1 ] = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_L | PAD_R | PAD_U | PAD_D ) ) ){
		pWork->hold_time[ 1 ] = 0;
		pWork->pad_status[ 1 ] = 0;
	}
}
#endif

static void PadControlSexEntry( Work *pWork )
{
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_L ) || ( GV_PadDataDirect[ 0 ].press & PAD_U ) ){
		pWork->pad_status[ 1 ] = 0;
		if ( pWork->sex.position > 0 ){
			pWork->sex.position--;
		} else {
			pWork->sex.position = 1;
		}
		SE_SEL();
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_R ) || ( GV_PadDataDirect[ 0 ].press & PAD_D ) ){
		pWork->pad_status[ 1 ] = 0;
		if ( pWork->sex.position < 1 ){
			pWork->sex.position++;
		} else {
			pWork->sex.position = 0;
		}
		SE_SEL();
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		SelectModeRecav( pWork );
		// 移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position++;
		SE_OK();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
//		pWork->sex.position = pWork->sex.back_position;
		SelectModeRecav( pWork );
		// back移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position--;
		SE_OK();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_STA ){
//		pWork->sex.position = pWork->sex.back_position;
		SelectModeRecav( pWork );
		// 移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position++;
		SE_OK();
		return;
	}
}

static inline int MonthToDay( int year , int month ) // 年月から日の計算
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

#define MAX_YEAR (2100)
#define MIN_YEAR (1900)
#define MAX_MONTH (12)
#define MIN_MONTH (1)

static void PadControlBirthEntry( Work *pWork ) // 誕生日入力
{
	int *buf; // 操作対象のアドレスをおぼえましょう
	int max , min;
	int tmp;

	buf = NULL;
	max = min = 0;
	switch( pWork->birth.phase ){ // 何処を入力しているか
	case 0 : // year
		buf = &pWork->birth.year;
		max = MAX_YEAR;
		min = MIN_YEAR;
		tmp = MonthToDay( pWork->birth.year , pWork->birth.month );
		pWork->birth.day = pWork->birth.day > tmp ? tmp : pWork->birth.day;
		break;
	case 1 : // month
		buf = &pWork->birth.month;
		max = MAX_MONTH;
		min = MIN_MONTH;
		tmp = MonthToDay( pWork->birth.year , pWork->birth.month );
		pWork->birth.day = pWork->birth.day > tmp ? tmp : pWork->birth.day;
		break;
	case 2 : // day
		buf = &pWork->birth.day;
		// day は 月によって違うので動的
		min = 1;
		max = MonthToDay( pWork->birth.year , pWork->birth.month );
		break;
	}
	if ( buf == NULL ){
		return;
	}
	// main
	if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status[ 1 ] & PAD_U ){
		pWork->pad_status[ 1 ] = 0;
		if ( *buf > min ){
			*buf = ( *buf - 1 );
			SE_SEL();
		} else {
			*buf = max;
			SE_SEL();
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status[ 1 ] & PAD_D ){
		pWork->pad_status[ 1 ] = 0;
		if ( *buf < max ){
			*buf = ( *buf + 1 );
			SE_SEL();
		} else {
			*buf = min;
			SE_SEL();
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status[ 1 ] & PAD_R || GV_PadDataDirect[ 0 ].press & SELECT ){
		switch ( pWork->birth.phase ){
		case 0 : // year
		case 1 : // month
			pWork->birth.phase++;
			pWork->hilight->sprite.dw = 0.0f;
			SE_TYPE();
			break;
		case 2 : // day
			if ( !( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status[ 1 ] & PAD_R ) ){
				SelectModeRecav( pWork );
				HappyBirthDay( pWork );
				// 移動
				pWork->normal_mode.old_position = pWork->normal_mode.position;
				pWork->normal_mode.position++;      //BP JG - skip to blood type
            pWork->normal_mode.position++;      //BP JG - skip to nationality
				SE_OK();
			}
			break;
		}
		pWork->pad_status[ 1 ] = 0;
		pWork->hold_time[ 2 ] = 0;
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status[ 1 ] & PAD_L || GV_PadDataDirect[ 0 ].press & CANCEL ){
		switch ( pWork->birth.phase ){
		case 0 : // year
      case 1 : // month
			if ( !( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status[ 1 ] & PAD_L ) ){
				SelectModeRecav( pWork );
				HappyBirthDay( pWork );
				// back移動
				pWork->normal_mode.old_position = pWork->normal_mode.position;
				pWork->normal_mode.position--;            // BP JG - back to sex
            pWork->normal_mode.position--;            // BP JG - back to name
				SE_OK();
			}
			break;

		case 2 : // day
			pWork->birth.phase--;
			pWork->hilight->sprite.dw = 0.0f;
			SE_TYPE();
			break;
		}
		pWork->pad_status[ 1 ] = 0;
		pWork->hold_time[ 2 ] = 0;
		return;
	}
#if 0
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		pWork->birth.year = pWork->birth.old_year;
		pWork->birth.month = pWork->birth.old_month;
		pWork->birth.day = pWork->birth.old_day;
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		HappyBirthDay( pWork );
		SelectModeRecav( pWork );
		// 移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position++;
		SE_OK();
	}
#endif
	if ( GV_PadDataDirect[ 0 ].press & PAD_STA ){
		HappyBirthDay( pWork );
		SelectModeRecav( pWork );
		// 移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position++;
      pWork->normal_mode.position++;
		SE_OK();
		return;
	}
	// auto mode
	if ( pWork->hold_time[ 1 ] >= DIRECT_TICK( 20 ) ){
		pWork->pad_status[ 1 ] = GV_PadDataDirect[ 0 ].status;
		if ( pWork->hold_time[ 2 ] >= 6 ) {
			pWork->hold_time[ 1 ] -= DIRECT_TICK( 2 );
		} else {
			pWork->hold_time[ 1 ] -= DIRECT_TICK( 4 );
			pWork->hold_time[ 2 ]++;
		}
	} else if ( pWork->pad_check[ 1 ] == GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time[ 1 ]++;
	} else {
		pWork->pad_check[ 1 ] = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_L | PAD_R | PAD_U | PAD_D ) ) ){
		pWork->hold_time[ 1 ] = 0;
		pWork->hold_time[ 2 ] = 0;
		pWork->pad_status[ 1 ] = 0;
	}
}

#define MAX_BLOOD (5) // A , B , AB , O , ?
static void PadControlBloodEntry( Work *pWork )
{
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_L ) || ( GV_PadDataDirect[ 0 ].press & PAD_U ) ){
		pWork->pad_status[ 1 ] = 0;
		if ( pWork->blood.position > 0 ){
			pWork->blood.old_position = pWork->blood.position;
			pWork->blood.position--;
		    pWork->blood.hokan.flag &= ~SK_MOVE_OK;
			pWork->blood.hokan.flag &= ~SK_ROOL_OK;
			SE_SEL();
		} else {
			pWork->blood.old_position = pWork->blood.position;
			pWork->blood.position = 4;
		    pWork->blood.hokan.flag &= ~SK_MOVE_OK;
			pWork->blood.hokan.flag &= ~SK_ROOL_OK;
			SE_SEL();
		}
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_R ) || ( GV_PadDataDirect[ 0 ].press & PAD_D ) ){
		pWork->pad_status[ 1 ] = 0;
		if ( pWork->blood.position < MAX_BLOOD ){
			pWork->blood.old_position = pWork->blood.position;
			pWork->blood.position++;
		    pWork->blood.hokan.flag &= ~SK_MOVE_OK;
			pWork->blood.hokan.flag &= ~SK_ROOL_OK;
			SE_SEL();
		} else {
			pWork->blood.old_position = pWork->blood.position;
			pWork->blood.position = 1;
		    pWork->blood.hokan.flag &= ~SK_MOVE_OK;
			pWork->blood.hokan.flag &= ~SK_ROOL_OK;
			SE_SEL();
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		SelectModeRecav( pWork );
		// 移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position++;
		SE_OK();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
//		pWork->blood.position = pWork->blood.back_position;
		SelectModeRecav( pWork );
		// back移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position--;
		SE_OK();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_STA ){
		SelectModeRecav( pWork );
		// 移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position++;
		SE_OK();
		return;
	}
}

static void PadControlRegionEntry( Work *pWork ) // 国の入力
{
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_L ) || ( pWork->pad_status[ 1 ] & PAD_L ) || 
		 ( GV_PadDataDirect[ 0 ].press & PAD_U ) || ( pWork->pad_status[ 1 ] & PAD_U ) ){
		pWork->pad_status[ 1 ] = 0;
		if ( pWork->region.position > 0 ){
			pWork->region.position--;
		} else {
			pWork->region.position = MAX_REGION - 1;
		}
		SE_SEL();
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_R ) || ( pWork->pad_status[ 1 ] & PAD_R ) || 
		 ( GV_PadDataDirect[ 0 ].press & PAD_D ) || ( pWork->pad_status[ 1 ] & PAD_D ) ){
		pWork->pad_status[ 1 ] = 0;
		if ( pWork->region.position < MAX_REGION - 1 ){
			pWork->region.position++;
		} else {
			pWork->region.position = 0;
		}
		SE_SEL();
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		SelectModeRecav( pWork );
		// 移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position++;
		SE_OK();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		SelectModeRecav( pWork );
		// back移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position--;      // BP JG - back to previous entry which used to be blood type
      pWork->normal_mode.position--;      // BP JG - back up again.. to birth.
		SE_OK();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_STA ){
		SelectModeRecav( pWork );
		// 移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position++;
		SE_OK();
		return;
	}
	// auto mode
	if ( pWork->hold_time[ 1 ] >= DIRECT_TICK( 20 ) ){
		pWork->pad_status[ 1 ] = GV_PadDataDirect[ 0 ].status;
		if ( pWork->hold_time[ 2 ] >= 6 ) {
			pWork->hold_time[ 1 ] -= DIRECT_TICK( 2 );
		} else {
			pWork->hold_time[ 1 ] -= DIRECT_TICK( 4 );
			pWork->hold_time[ 2 ]++;
		}
	} else if ( pWork->pad_check[ 1 ] & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time[ 1 ]++;
	} else {
		pWork->pad_check[ 1 ] = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_L | PAD_R | PAD_U | PAD_D ) ) ){
		pWork->hold_time[ 1 ] = 0;
		pWork->hold_time[ 2 ] = 0;
		pWork->pad_status[ 1 ] = 0;
	}
}

static void PadControlOk( Work *pWork ) // 終了
{
	static int strcode_show[] = { STR_SHOW_NAME , STR_SHOW_SEX , STR_SHOW_BIRTH , STR_SHOW_BLD , STR_SHOW_REG , STR_SHOW_OK , STR_SHOW_UNDERLINE };
	static int strcode_hide[] = { STR_HIDE_NAME , STR_HIDE_SEX , STR_HIDE_BIRTH , STR_HIDE_BLD , STR_HIDE_REG , STR_HIDE_OK , STR_HIDE_UNDERLINE };
	static int strcode_parts[] = { STR_PARTS_NAME_NAME , STR_PARTS_NAME_SEX , STR_PARTS_NAME_BIRTH , STR_PARTS_NAME_BLD , 
								   STR_PARTS_NAME_REG , STR_PARTS_MENU_OK , STR_UNDERLINE };
	SPR_OBJ *spr;
	int width;
	int alpha;
	int i;


	width = ((18 * 2)*TARGET_ASPECT_X)+1;
	pWork->hilight->sprite.pos.x = 58.0f;
	pWork->hilight->sprite.pos.y = 309.0f;
	pWork->hilight->sprite.dw += ( width - pWork->hilight->sprite.dw ) / SK_HOKAN_RAUTE;
	pWork->hilight->sprite.dh = STR_HEIGHT;
	// alpha
	alpha = ( int )pWork->hilight->sprite.col.a;
	alpha += IN_FADE_SPEED;
	if ( alpha >= SK_HILIGHT_A ){
		alpha = SK_HILIGHT_A;
	}
	pWork->hilight->sprite.col.a = ( u_char )alpha;
	SPR_SHOW( pWork->hilight );

	if ( ( GV_PadDataDirect[ 0 ].press & SELECT ) || ( GV_PadDataDirect[ 0 ].press & PAD_STA ) ){
		spr = L2D_GetObject( pWork->handle_2d , STR_SELCURS );
		if ( spr == NULL ){
			return;
		}
		spr->sprite.col.a = 0;
		// value で menu 非表示
		for ( i = 0 ; i < MAX_KEY_ANIME ; i ++ ){
			pWork->disp_hokan[ i ].p = 0.0f;
			pWork->disp_hokan[ i ].count = 40;
			pWork->disp_hokan[ i ].code1 = strcode_hide[ i ];
			pWork->disp_hokan[ i ].code2 = strcode_show[ i ];
			pWork->disp_hokan[ i ].morf_parts = strcode_parts[ i ];
			pWork->disp_hokan[ i ].flag = 0;
		}
		pWork->act = ( void * )EscapeAct;
		pWork->flag |= SK_END;
		pWork->flag &= ~SK_NORMAL_MODE;
		SE_OK();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		// back移動
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position--;
		pWork->flag &= ~SK_FADE_OK;
		SE_OK();
	}
#if 0
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position--;
		pWork->act = ( void * )OkEntryEscapeAct;
		pWork->flag |= SK_OK_ENTRY_ESCAPE;
		pWork->flag &= ~SK_OK_MODE;
		pWork->flag &= ~SK_FADE_OK;
#if 0
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		pWork->normal_mode.position--;
#endif
		SE_CANCEL();
		return;
	}
#endif
}

static void PadAct( Work *pWork ) // パッドマネージャー
{
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) ){
		return;
	}
	if ( pWork->flag & SK_NORMAL_MODE ){
		PadControlNormal( pWork );
	}
	if ( pWork->flag & SK_NAME_ENTRY ){
		SoftKey( pWork );
//		PadControlNameEntry( pWork );
		return;
	}
	if ( pWork->flag & SK_SEX_ENTRY ){
		PadControlSexEntry( pWork );
		return;
	}
	if ( pWork->flag & SK_BIRTH_ENTRY ){
		PadControlBirthEntry( pWork );
		return;
	}
	if ( ( pWork->flag & SK_BLOOD_ENTRY ) && ( pWork->blood.hokan.flag & SK_MOVE_OK ) ){
		PadControlBloodEntry( pWork );
		return;
	}
	if ( pWork->flag & SK_REGION_ENTRY ){
		PadControlRegionEntry( pWork );
		return;
	}
	if ( pWork->flag & SK_OK_MODE ){
		PadControlOk( pWork );
		return;
	}
}

#define STR_PARTS_SELCURS (14909507)
static void DotUpdate( Work *pWork ) // normal_positionからドットの場所を割出して
{
	if ( ( pWork->normal_mode.position == pWork->normal_mode.old_position ) ){
		pWork->normal_mode.old_position = pWork->normal_mode.position;
		return;
	}
    pWork->dot_hokan.flag &= ~SK_MOVE_OK;
	switch( pWork->normal_mode.position ){
	case 0 : // name
		pWork->dot_hokan.code1 = STR_CUR_NAME;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 1 : // sex
		pWork->dot_hokan.code1 = STR_CUR_SEX;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 2 : // birthday
//		pWork->dot_hokan.code1 = STR_CUR_BIRTH;
      pWork->dot_hokan.code1 = STR_CUR_SEX;
      pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 3 : // blood type
		pWork->dot_hokan.code1 = STR_CUR_BLD;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 4 : // region
//		pWork->dot_hokan.code1 = STR_CUR_REG;
      pWork->dot_hokan.code1 = STR_CUR_BIRTH;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 10;
		break;
	case 5 : // ok
		pWork->dot_hokan.code1 = STR_CUR_OK;
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 20;
		break;
	}
	switch( pWork->normal_mode.old_position ){
	case 0 : // name
		pWork->dot_hokan.code2 = STR_CUR_NAME;
		break;
	case 1 : // sex
		pWork->dot_hokan.code2 = STR_CUR_SEX;
		break;
	case 2 : // birthday
		pWork->dot_hokan.code2 = STR_CUR_BIRTH;
		break;
	case 3 : // blood type
		pWork->dot_hokan.code2 = STR_CUR_BLD;
		break;
	case 4 : // region
		pWork->dot_hokan.code2 = STR_CUR_REG;
		break;
	case 5 : // ok
		pWork->dot_hokan.code2 = STR_CUR_OK;
		pWork->dot_hokan.count = 20;
		break;
	}
	pWork->normal_mode.old_position = pWork->normal_mode.position;
}

static void PositionAct( Work *pWork )
{
	u_char *ascci;

	switch( pWork->normal_mode.position ){
	case 0 : // name entry
		// layout
		pWork->action = STR_OPEN_KB;
		L2D_BreakAction( pWork->handle_2d );
		L2D_EvokeAction( pWork->handle_2d , pWork->action );
		pWork->flag &= ~SK_ACTION_OK;
		pWork->flag &= ~SK_ACTION_START;
		//
		memcpy( pWork->backup_name , GM_MyName , ASCCI_MAX ); // 現在の情報を退避
		ascci = ( char * )GM_MyName;
		if ( (pWork->name.position != 0 &&
			  ascci[pWork->name.position] != 0x61) ||
			 (pWork->name.position == 0 &&
			  ascci[pWork->name.position] != 0x80) ){ // 文字ありならenterへ
			SoftHideShowControl( pWork , 0 );
			pWork->soft.x = SOFT_X_MAX - 1;
			pWork->soft.y = SOFT_Y_MAX - 1;
			pWork->soft.add_x = 0;
			pWork->soft.add_y = 0;
			ascci[ ++pWork->name.position ] = 0x61 ;
			SoftHideShowControl( pWork , 1 );
			SE_RGB_TUNE();
		}

		pWork->name.old_position = pWork->name.position;
		if ( pWork->name.old_position == 0 && ascci[ 0 ] == 0x80 ){
			ascci[ 0 ] = 0x41;
		}

		// soft
		SoftHideShowControl( pWork , 0 ); // Sprite 表示 、非表示
		SoftKeySerch( pWork );
		SoftHideShowControl( pWork , 1 ); // Sprite 表示 、非表示
		pWork->flag |= SK_NAME_ENTRY;
		// non
		pWork->flag &= ~SK_SEX_ENTRY;
		pWork->flag &= ~SK_BIRTH_ENTRY;
		pWork->flag &= ~SK_BLOOD_ENTRY;
		pWork->flag &= ~SK_REGION_ENTRY;
		pWork->flag &= ~SK_OK_MODE;
		break;
	case 1 : // sex
		pWork->flag |= SK_SEX_ENTRY;
		// non
		pWork->flag &= ~SK_NAME_ENTRY;
		pWork->flag &= ~SK_BIRTH_ENTRY;
		pWork->flag &= ~SK_BLOOD_ENTRY;
		pWork->flag &= ~SK_REGION_ENTRY;
		pWork->flag &= ~SK_OK_MODE;
		// etc
		pWork->sex.back_position = pWork->sex.position;
		break;
	case 2 : // birth
		pWork->birth.old_year  = pWork->birth.year;
		pWork->birth.old_month = pWork->birth.month;
		pWork->birth.old_day   = pWork->birth.day;
		pWork->birth.phase = 1;          // BP JG - skip year, start on month.
		pWork->flag |= SK_BIRTH_ENTRY;
		// non
		pWork->flag &= ~SK_NAME_ENTRY;
		pWork->flag &= ~SK_SEX_ENTRY;
		pWork->flag &= ~SK_BLOOD_ENTRY;
		pWork->flag &= ~SK_REGION_ENTRY;
		pWork->flag &= ~SK_OK_MODE;
		break;
	case 3 : // blood
		pWork->flag |= SK_BLOOD_ENTRY;
		// non
		pWork->flag &= ~SK_NAME_ENTRY;
		pWork->flag &= ~SK_SEX_ENTRY;
		pWork->flag &= ~SK_BIRTH_ENTRY;
		pWork->flag &= ~SK_REGION_ENTRY;
		pWork->flag &= ~SK_OK_MODE;
		// etc
		pWork->blood.back_position = pWork->blood.position;
		break;
	case 4 : // region
		pWork->flag |= SK_REGION_ENTRY;
		// non
		pWork->flag &= ~SK_NAME_ENTRY;
		pWork->flag &= ~SK_SEX_ENTRY;
		pWork->flag &= ~SK_BIRTH_ENTRY;
		pWork->flag &= ~SK_BLOOD_ENTRY;
		pWork->flag &= ~SK_OK_MODE;
		// etc
		pWork->region.back_position = pWork->region.position;
		break;
	case 5 : // ok
		pWork->flag |= SK_NORMAL_MODE;
		pWork->flag |= SK_OK_MODE;
		// non
		pWork->flag &= ~SK_NAME_ENTRY;
		pWork->flag &= ~SK_SEX_ENTRY;
		pWork->flag &= ~SK_BIRTH_ENTRY;
		pWork->flag &= ~SK_BLOOD_ENTRY;
		pWork->flag &= ~SK_REGION_ENTRY;
		break;
	}
	pWork->act = ( void * )NormalAct;
}

static void StaffNameSwap( Work *pWork ) // スタッフデータに入れ替える
{	
	u_char *ascci;
	u_char first_name[ ASCCI_MAX + 1 ];
	u_char last_name[ ASCCI_MAX + 1 ];
	NameWork *pName_work;
	int i , j , k , tmp;

	pName_work = ( NameWork * )pWork->name_work_top_add;
	ascci = ( u_char * )GM_MyName;
#ifdef DEBUG_MODE
	for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
		//printf("%c" , ascci[ i ] );
	}
#endif
	// space 飛ばし
	for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
		if ( ascci[ i ] != 0x20 || ascci[ i ] == 0x80 ){
			break;
		}
	}
	for ( j = i ; j < ASCCI_MAX ; j ++ ){
		if ( !( ( ( ascci[ j ] >= 0x41 ) && ( ascci[ j ] <= 0x5a ) ) || 
				( ascci[ j ] == '_' ) || ( ascci[ j ] != 0x20 ) ) ){
			break;
		}
	}
	strncpy( first_name , &ascci[ i ] , ( j - i == 0 ? 1 : j - i ) );
	first_name[ ( j - i == 0 ? 1 : j - i ) ] = '\0';
	j++;
	for ( k = j ; k < ASCCI_MAX ; k ++ ){
		if ( !( ( ascci[ k ] >= 0x41 ) && ( ascci[ k ] <= 0x5a ) ) ){
			break;
		}
	}
	strncpy( last_name , &ascci[ j ] , ( k - j == 0 ? 1 : k - j ) );
	last_name[ ( k - j == 0 ? 1 : k - j ) ] = '\0';
	for ( i = k ; i < ASCCI_MAX ; i ++ ){
		if ( ascci[ i ] != 0x20 && ascci[ i ] != 0x80 ){
			return;
		}
	}
	tmp = 0;
	while ( pName_work != NULL ) {
		if ( ( SK_StringCmp( first_name , last_name , pName_work->first_name , pName_work->last_name ) == 1 ) && 
			 !( pName_work->flag & 0x1 ) ){
			tmp = 1;
			break;
		}
		pName_work = pName_work->pNext;
	}
	if ( tmp == 1 ){ // 入れ換え
		pWork->sex.position = pName_work->sex;
		pWork->birth.year = pName_work->year;
		pWork->birth.month = pName_work->month;
		pWork->birth.day = pName_work->day;
		pWork->blood.position = pName_work->blood;
		pWork->region.position = pName_work->nation;
		pWork->birth.flag |= SK_BIRTH_FIRST_INPUT;
		pName_work->flag = 0x1;
	}
}

static int NgWordCheck( Work *pWork ) // 卑猥な言葉かどうか
{	
	u_char *ascci;
	u_char first_name[ 32 ];
	NgWork *pNg_work;
	int    i , j;

	ascci = ( u_char * )GM_MyName;
	// space 飛ばし
	for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
		if ( ascci[ i ] != 0x20 || ascci[ i ] == 0x80 ){
			break;
		}
	}

	while ( 1 ){
		// 先頭に移動
		pNg_work = ( NgWork * )pWork->ng_work_top_add;
		// 単語の取得
		for ( j = i ; j < ASCCI_MAX ; j ++ ){
			if ( !( ( ( ascci[ j ] >= 0x41 ) && ( ascci[ j ] <= 0x5a ) ) || 
					( ascci[ j ] == 0x2d ) ) ){
				break;
			}
		}
		strncpy( first_name , &ascci[ i ] , ( j - i == 0 ? 1 : j - i ) );
		first_name[ ( j - i == 0 ? 1 : j - i ) ] = '\0';
#ifdef DEBUG_MODE
		//printf("j %d i %d %s\n" , j , i , first_name );
#endif
		i = j + 1;
		// リストと比較
		while ( pNg_work != NULL ) {
			if ( ( SK_StringCmpSingle( first_name , pNg_work->first_name ) == 1 ) ){
				GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_BUZZER01 );
				return ( 1 );
			}
			pNg_work = pNg_work->pNext;
		}
		if ( i >= ASCCI_MAX - 1 ){ // 終了処理
			break;
		}
	}
	return ( 0 );
}

static void NemaEntryEscapeAct( Work *pWork )
{
	u_char    *ascci;
	int      alpha;
	int          i;

	ascci = ( u_char * )GM_MyName;
	
	if ( pWork->flag & SK_NAME_ENTRY_ESCAPE ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha -= 4;
		if ( alpha < 0 ){
			// 元に戻す
			pWork->hilight->sprite.col.a = SK_HILIGHT_A;
#if 0
			if ( pWork->flag & SK_CHANCEL ){ // キャンセル時：情報を元に戻す
				memcpy( ascci , pWork->backup_name , ASCCI_MAX );
				pWork->flag &= ~SK_CHANCEL;
			}
#endif
			for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
				if ( ascci[ i ] == 0x80 ){
					break;
				}
			}
			if ( i == 0 ){
				pWork->name.position = 0;
			} else {
				pWork->name.position = i - 1;
			}

			Update( pWork );
			pWork->hilight->sprite.dw = 0.0f;
			SPR_HIDE( pWork->hilight );
			SoftInitialize( pWork ); // キーボード初期化
			HappyBirthDay( pWork ); // 誕生日おめでとう
			pWork->flag |= SK_NORMAL_MODE;
			pWork->flag &= ~SK_NAME_ENTRY_ESCAPE;
			pWork->flag &= ~SK_FADE_OK;
			pWork->act = ( void * )NormalAct;
			return;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	}
	Update( pWork );
}

static void BirthEntryEscapeAct( Work *pWork )
{
	int      alpha;

	if ( pWork->flag & SK_BIRTH_ENTRY_ESCAPE ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha -= 4;
		if ( alpha < 0 ){
			// 元に戻す
			pWork->hilight->sprite.dw = 0.0f;
			pWork->hilight->sprite.col.a = SK_HILIGHT_A;
			SPR_HIDE( pWork->hilight );
			pWork->flag |= SK_NORMAL_MODE;
			pWork->flag &= ~SK_BIRTH_ENTRY_ESCAPE;
			pWork->act = ( void * )NormalAct;
			pWork->flag &= ~SK_FADE_OK;
			return;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	}
}

static void SexEntryEscapeAct( Work *pWork )
{
	int      alpha;

	if ( pWork->flag & SK_SEX_ENTRY_ESCAPE ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha -= 4;
		if ( alpha < 0 ){
			// 元に戻す
			pWork->hilight->sprite.dw = 0.0f;
			pWork->hilight->sprite.col.a = SK_HILIGHT_A;
			SPR_HIDE( pWork->hilight );
			pWork->flag |= SK_NORMAL_MODE;
			pWork->flag &= ~SK_SEX_ENTRY_ESCAPE;
			pWork->act = ( void * )NormalAct;
			pWork->flag &= ~SK_FADE_OK;
			return;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	}
}

static void BloodEntryEscapeAct( Work *pWork )
{
	int      alpha;

	if ( pWork->flag & SK_BLOOD_ENTRY_ESCAPE ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha -= 4;
		if ( alpha < 0 ){
			// 元に戻す
			pWork->hilight->sprite.dw = 0.0f;
			pWork->hilight->sprite.col.a = SK_HILIGHT_A;
			SPR_HIDE( pWork->hilight );
			pWork->flag |= SK_NORMAL_MODE;
			pWork->flag &= ~SK_BLOOD_ENTRY_ESCAPE;
			pWork->act = ( void * )NormalAct;
			pWork->flag &= ~SK_FADE_OK;
			return;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	}
}

static void RegionEntryEscapeAct( Work *pWork )
{
	int      alpha;

	if ( pWork->flag & SK_REGION_ENTRY_ESCAPE ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha -= 4;
		if ( alpha < 0 ){
			// 元に戻す
			pWork->hilight->sprite.dw = 0.0f;
			pWork->hilight->sprite.col.a = SK_HILIGHT_A;
			SPR_HIDE( pWork->hilight );
			pWork->flag |= SK_NORMAL_MODE;
			pWork->flag &= ~SK_REGION_ENTRY_ESCAPE;
			pWork->act = ( void * )NormalAct;
			pWork->flag &= ~SK_FADE_OK;
			return;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	}
}

static void OkEntryEscapeAct( Work *pWork )
{
	int      alpha;

	if ( pWork->flag & SK_OK_ENTRY_ESCAPE ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha -= 4;
		if ( alpha < 0 ){
			// 元に戻す
			pWork->hilight->sprite.dw = 0.0f;
			pWork->hilight->sprite.col.a = SK_HILIGHT_A;
			SPR_HIDE( pWork->hilight );
//			pWork->flag |= SK_NORMAL_MODE;
			pWork->flag &= ~SK_OK_ENTRY_ESCAPE;
			pWork->act = ( void * )NormalAct;
			pWork->flag &= ~SK_FADE_OK;
			return;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	}
}

static inline void StoreData( Work *pWork )
{
	u_char *ascci;
	int       i;

	ascci = ( char * )GM_MyName;
	for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
		if ( ascci[ i ] == 0x80 ){
			ascci[ i ] = 0x0; // end code
			break;
		}
	}
	GM_MySexData = pWork->sex.position;
	GM_MyYearData = pWork->birth.year;
	GM_MyMonthData = pWork->birth.month;
	GM_MyDayData = pWork->birth.day;
	GM_MyBloodData = pWork->blood.position == 5 ? 0 : pWork->blood.position;
	GM_MyRegionData = pWork->region.position;

	// bood mode? on or off
	if ( GM_MyYearData >= 1990 ){
		GM_Configuration |= GM_CONFIG_BLOOD_OFF;
	}
}

static void EscapeAct( Work *pWork )
{
	SPR_OBJ *spr;
	int morf_count;
	int        tmp;
	int      alpha;
	int          i;

	tmp = 0;
	// key文字
	morf_count = 0;
	for( i = 0 ; i < MAX_KEY_ANIME ; i ++ ){
		morf_count = MorfAct( pWork , &pWork->disp_hokan[ i ] );
	}
	CopyAlpha( pWork );
	alpha = ( int )pWork->hilight->sprite.col.a;
	alpha -= 4;
	if ( alpha < 0 ){
		pWork->hilight->sprite.dw = 0.0f;
		SPR_HIDE( pWork->hilight );
		tmp++;
	}
	pWork->hilight->sprite.col.a = ( u_char )alpha;
	// happy
	spr = pWork->birth.happy->head.child;
	for ( i = 0 ; i < 4 ; i ++ ){
		alpha = ( int )spr->sprite.col.a;
		alpha -= 4;
		if ( alpha < 0 ){
			alpha = 0;
			tmp++;
		}
		spr->sprite.col.a = ( u_char )alpha;
		spr = spr->head.next;
	}
	// jude
	if ( morf_count == 0 && tmp >= 5 ) {
		if ( pWork->proc_next != 0 ){
			GM_ExecProc( pWork->proc_next , NULL );
		}
		pWork->action = STR_CLOSEENTRY;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->act = ( void * )AnimetionAct;
		StoreData( pWork );
	}
}

static void SexUpdate( Work *pWork )
{
	SPR_OBJ *spr;
	int     width;
	int     alpha;
	int     raute;

	raute = 0;
	if ( pWork->flag & SK_SEX_ENTRY ){
		// hilight
		spr = L2D_GetObject( pWork->handle_2d , STR_PARTS_SEX );
		if ( spr == NULL ){
			return;
		}
		pWork->hilight->sprite.pos.x = spr->sprite.pos.x;
		pWork->hilight->sprite.pos.y = spr->sprite.pos.y;
		switch ( pWork->sex.position ){
		case 0 : // male
			raute = 4;
			break;
		case 1 : // male
			raute = 6;
			break;
		}
		width = ((18 * raute)*TARGET_ASPECT_X)+1;
		pWork->hilight->sprite.dw += ( width - pWork->hilight->sprite.dw ) / SK_HOKAN_RAUTE;
		pWork->hilight->sprite.dh = spr->sprite.dh;
		// alpha
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha >= SK_HILIGHT_A ){
			alpha = SK_HILIGHT_A;
		}
		pWork->hilight->sprite.col.a = ( u_char )alpha;
		SPR_SHOW( pWork->hilight );
	}

	if ( pWork->sex.position == pWork->sex.old_position ){
		return;
	}
    pWork->sex.hokan.flag &= ~SK_MOVE_OK;
	switch( pWork->sex.position ){
	case 0 : // male
		pWork->sex.hokan.code1 = STR_SEX_MALE;
		pWork->sex.hokan.code2 = STR_SEX_FEMALE;
		pWork->sex.hokan.p = 0.0f;
		pWork->sex.hokan.count = 1;
		break;
	case 1 : // female
		pWork->sex.hokan.code1 = STR_SEX_FEMALE;
		pWork->sex.hokan.code2 = STR_SEX_MALE;
		pWork->sex.hokan.p = 0.0f;
		pWork->sex.hokan.count = 1;
		break;
	}
	pWork->sex.old_position = pWork->sex.position;

}

static void BirthDayUpdate( Work *pWork ) // 誕生日
{
   char tempString[ 3 ];

	char buf[ MAX_BIRTH + 1 ];
	int  size_count;
	int  position;
	int  width;
	int  alpha;
	int  i;
   int  monthWidth;
   int  dayWidth;

   // just to calc the width of the entries
   sprintf( tempString, "%02d" , pWork->birth.month);
   monthWidth = (int)SK_PrintfNormalWidth2( tempString  );
   sprintf( tempString, "%02d" , pWork->birth.day);
   dayWidth = (int)SK_PrintfNormalWidth2( tempString );

   // now create the string and print
   sprintf( buf , "????");
   sprintf( buf+4 , "/%02d/%02d" , pWork->birth.month , pWork->birth.day );
   SK_PrintfNormal(buf , pWork->birth.obj , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , MAX_BIRTH );



	if ( pWork->flag & SK_BIRTH_ENTRY )
   {
		// hilight
		position = 0;
		size_count = 0;
		switch( pWork->birth.phase ){

      case 0 : // year
			position = 0;
			size_count = 4;
         pWork->hilight->sprite.pos.x = pWork->birth.obj[ position ]->sprite.pos.x + 1;
			break;
		case 1 : // month
			position = 5;
			size_count = monthWidth;  //2;
         pWork->hilight->sprite.pos.x = pWork->birth.obj[ position ]->sprite.pos.x - 1;
			break;
		case 2 : // day
			position = 8;
			size_count = dayWidth; //2;
         pWork->hilight->sprite.pos.x = pWork->birth.obj[ position ]->sprite.pos.x - 1;
			break;
		}
		pWork->hilight->sprite.pos.y =  pWork->birth.obj[ position ]->sprite.pos.y - 1;
		width = 0;
		//for ( i = position ; i < position + size_count ; i ++ )
      //{
		//	width += FONT_NUM_WIDTH;
		//}
      width+=1;
		pWork->hilight->sprite.dw = size_count;  //((( width - pWork->hilight->sprite.dw ) / SK_HOKAN_RAUTE)*TARGET_ASPECT_X);
		pWork->hilight->sprite.dh = pWork->birth.obj[ position ]->sprite.dh;
		// alpha
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha >= SK_HILIGHT_A ){
			alpha = SK_HILIGHT_A;
		}
		pWork->hilight->sprite.col.a = ( u_char )alpha;
		SPR_SHOW( pWork->hilight );
	}
	SK_AllShow( pWork->birth.obj , MAX_BIRTH );
}

static void BloodUpdate( Work *pWork ) // 血液型
{
	SPR_OBJ *spr;
	int     width;
	int     raute;
	int     alpha;

	raute = 0;
	if ( pWork->flag & SK_BLOOD_ENTRY ){
		// hilight
		spr = L2D_GetObject( pWork->handle_2d , STR_PARTS_BLD );
		if ( spr == NULL ){
			return;
		}
		pWork->hilight->sprite.pos.x = spr->sprite.pos.x;
		pWork->hilight->sprite.pos.y = spr->sprite.pos.y;
		switch ( pWork->blood.position ){
		case 0 : // ?
		case 1 : // A
		case 2 : // B
		case 4 : // O
		case 5 : // ?
			raute = 1;
			break;
		default : // AB
			raute = 2;
			break;
		}
		width = ((18 * raute)*TARGET_ASPECT_X)+1;
		pWork->hilight->sprite.dw += ( width - pWork->hilight->sprite.dw ) / SK_HOKAN_RAUTE;
      // BP JG - set the hight manually as sometimes the 'spr' is not right for a frame.
      pWork->hilight->sprite.dh = 12;   //spr->sprite.dh;
		// alpha
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha >= SK_HILIGHT_A ){
			alpha = SK_HILIGHT_A;
		}
		pWork->hilight->sprite.col.a = ( u_char )alpha;
		SPR_SHOW( pWork->hilight );
	}

	if ( pWork->blood.position == pWork->blood.old_position ){
		pWork->blood.old_position = pWork->blood.position;
		return;
	}

	pWork->blood.hokan.flag = 0;
	switch( pWork->blood.position ){
	case 0 : // UNKONWN
		pWork->blood.hokan.code1 = STR_BLOOD_UNKNOWN1;
		pWork->blood.hokan.code2 = STR_BLOOD_UNKNOWN1;
		pWork->blood.hokan.p = 0.0f;
		pWork->blood.hokan.count = 1;
		break;
	case 1 : // A
		pWork->blood.hokan.code1 = STR_BLOOD_A;
		pWork->blood.hokan.code2 = STR_BLOOD_A;
		pWork->blood.hokan.p = 0.0f;
		pWork->blood.hokan.count = 1;
		break;
	case 2 : // B
		pWork->blood.hokan.code1 = STR_BLOOD_B;
		pWork->blood.hokan.code2 = STR_BLOOD_B;
		pWork->blood.hokan.p = 0.0f;
		pWork->blood.hokan.count = 1;
		break;
	case 3 : // AB
		pWork->blood.hokan.code1 = STR_BLOOD_AB;
		pWork->blood.hokan.code2 = STR_BLOOD_AB;
		pWork->blood.hokan.p = 0.0f;
		pWork->blood.hokan.count = 1;
		break;
	case 4 : // O
		pWork->blood.hokan.code1 = STR_BLOOD_O;
		pWork->blood.hokan.code2 = STR_BLOOD_O;
		pWork->blood.hokan.p = 0.0f;
		pWork->blood.hokan.count = 1;
		break;
	case 5 : // UNKONWN
		pWork->blood.hokan.code1 = STR_BLOOD_UNKNOWN2;
		pWork->blood.hokan.code2 = STR_BLOOD_UNKNOWN2;
		pWork->blood.hokan.p = 0.0f;
		pWork->blood.hokan.count = 1;
		break;
	}
	pWork->blood.old_position = pWork->blood.position;
}

static inline int MorfAct( Work *pWork , Hokan *pHokan ) // これのみ実行
{
	if ( !( pHokan->flag & SK_MOVE_OK ) ){
		MorfL2d( pWork->handle_2d , pHokan->morf_parts  , pHokan->code2  , pHokan->code1  , pHokan );
	} else {
		return ( 0 );
	}
	return( 1 );
}

static inline int TwoLineCheck( int nation ) // ２行だったら１
{
	//static int two_line[] = { 5 , 21 , 32 , 53 , 107 , 132 , 146 , 154 , 158 , 160 , 173 , -1 }; // ２行ある国
	static int two_line[] = { 5 , 21 , 32 , 53 , 107 , 132 , 146 , 154 , 158 , 160 , 172 , -1 }; // ２行ある国
	int i;

	i = 0;
	while ( two_line[ i ] != -1 ){
		if ( nation == two_line[ i ] ){
			return ( 1 );
		}
		i++;
	}
	return ( 0 );
}


static void RegionUpdate( Work *pWork )
{
	if ( pWork->region.position == KCEJ ){ // KCEJ
		SPR_SHOW( pWork->region.kcej );
		SK_AllHide( pWork->region.obj , MAX_REGION_OBJ );
		RegionHilight( pWork , WorldName[ pWork->region.position ] , 0x1 );
	} else {
		SK_PrintfNormal( NULL , pWork->region.obj , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , MAX_REGION_OBJ );
		SK_PrintfNormal( WorldName[ pWork->region.position ] , pWork->region.obj , 
						 pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , MAX_REGION_OBJ );
		SK_AllShow( pWork->region.obj , MAX_REGION_OBJ );
	    SPR_HIDE( pWork->region.kcej );
		if ( TwoLineCheck( pWork->region.position ) ) {
			RegionHilight( pWork , WorldName[ pWork->region.position ] , 0x3 );
		} else {
			RegionHilight( pWork , WorldName[ pWork->region.position ] , 0x0 );
		}
	}
}

static void HappyBirthDay( Work *pWork ) // 誕生日おめでとう
{
	int month;
	int day;

	if ( !( pWork->flag & SK_SPRITE_INIT_OK ) ){//|| ( pWork->normal_mode.position == 2 ) ){
		return;
	}

	/* G_Rtcを更新する */
	GM_GetRTCTime() ;/* gamed.c*/

	month = ( ( ( G_rtc.month & 0xf0 ) >> 4 ) * 10 ) + ( G_rtc.month & 0xf );
	day = ( ( ( G_rtc.day & 0xf0 ) >> 4 ) * 10 ) + ( G_rtc.day & 0xf );
	if ( ( pWork->birth.flag & SK_BIRTH_FIRST_INPUT ) && ( pWork->birth.month == month ) && ( pWork->birth.day == day ) ){
#ifdef DEBUG_MODE
		printf("happy show\n");
#endif
		SPR_SHOW( pWork->birth.happy );
	} else {
#ifdef DEBUG_MODE
		printf("happy hide\n");
#endif
		SPR_HIDE( pWork->birth.happy );
	}
}

static void NormalAct( Work *pWork )
{
	int status;

	status = L2D_ActionStatus( pWork->handle_out );
	if ( !( pWork->flag & SK_FIRST_ACTION ) && ( pWork->flag & SK_SIGNAL_OK ) && ( status != L2D_STAT_BUSY ) ){
		pWork->act = ( void * )PositionAct;
		pWork->flag |= SK_FIRST_ACTION;
	} else {
		PadAct( pWork );
	}
}

static void Act( Work *pWork )
{
	int i;

   //BP added USB keyboard support to fulfill TCR 043.
   //N.B. must poll keyboard every update cycle to keep keypresses from queueing up
   //when the name entry interface is not active.
   pWork->bp_keyret = BP_GetMGSNameKeyboardKeystroke( &pWork->bp_keypress );
   if( !pWork->bp_keyret )
   {
      printf("Keypress: %c\n", pWork->bp_keypress );
   }

 	pWork->act( pWork );
	if ( ( pWork->flag & SK_SPRITE_INIT_OK ) && !( pWork->flag & SK_END ) ){
		MorfAct( pWork , &pWork->dot_hokan );
		MorfAct( pWork , &pWork->sex.hokan );
		MorfAct( pWork , &pWork->blood.hokan );
		CopyAlpha( pWork );
		DotUpdate( pWork );
		Update( pWork );
		SexUpdate( pWork );
		BirthDayUpdate( pWork );
		BloodUpdate( pWork );
		RegionUpdate( pWork );
		// key文字
		for( i = 0 ; i < MAX_KEY_ANIME ; i ++ ){
			MorfAct( pWork , &pWork->disp_hokan[ i ] );
		}
	}
}

#define STR_HIDE_CUR       (8680992)

static void AnimetionAct( Work *pWork )
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( pWork->flag & SK_ACTION_OK ) && ( status != L2D_STAT_BUSY ) ){
		switch ( pWork->action ){
		case STR_OPENENTRY :
#if 0
			spr = L2D_GetObject( pWork->handle_2d , STR_PARTS_SELCURS );
			L2D_MorfObject( spr , STR_HIDE_CUR , STR_HIDE_CUR , 1 );
#endif
			if ( SprInit( pWork ) < 0 ){
				SK_Err("Non Memory\0");
				return;
			}
//			pWork->flag |= SK_NORMAL_MODE;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->act = ( void * )NormalAct;
			return;
		case STR_CLOSEENTRY :
			GV_CallParentSignalFunc( pWork , NAME_ENTRY_END , 0 );
			GV_DestroyActor( pWork );
			return;
		}
		pWork->flag &= ~SK_ACTION_OK;
	}
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
	}
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d );
	}
	NameWorkListFree( work );
	NgWorkListFree( work );
	/* 子 Actor として起動されたのでなければ、開放 */
	if(!work->child_mode){
		if ( work->handle_out >= 0 ){
			L2D_ReleaseLayout( work->handle_out );
		}
	}
}

static void CopyAlpha( Work *pWork ) // 1 は己制御
{
	static int CopyTable[][ 4 ] = { { STR_PARTS_SELCURS , STR_PARTS_NAME_NAME , 1 , 0 } ,
								    { STR_PARTS_SELCURS , STR_PARTS_NAME_SEX , STR_PARTS_SEX , 0 } ,
								    { STR_PARTS_SELCURS , STR_PARTS_NAME_BIRTH , 2 , 0 } ,
									{ STR_PARTS_SELCURS , STR_PARTS_NAME_BLD , STR_PARTS_BLD , 0 } ,
									{ STR_PARTS_SELCURS , STR_PARTS_NAME_REG , 3 , 0 } ,
									{ STR_PARTS_SELCURS , STR_PARTS_MENU_OK , 0 } , };
	SPR_OBJ *spr;
	SPR_OBJ	*spr_child;
	u_char  alpha;
	int     alpha_tmp;
	int     i , j;

	for ( i = 0 ; i < 6 ; i ++ ){
		spr = L2D_GetObject( pWork->handle_2d , CopyTable[ i ][ 0 ] );
		if ( ( i == pWork->normal_mode.position ) || ( pWork->flag & SK_END ) ){
			alpha = spr->sprite.col.a;
		} else {
			alpha = 52;
		}
		j = 1;
		while( 1 ){
			if ( CopyTable[ i ][ j ] == 0 ){
				break;
			}
			switch( CopyTable[ i ][ j ] ){
			case 1 : // name
				alpha_tmp = pWork->font[ 0 ]->sprite.col.a;
				if ( alpha_tmp < ( int )alpha ){
					alpha_tmp += IN_FADE_SPEED;
					if ( alpha_tmp >= alpha ){
						alpha_tmp = ( int )alpha;
					}
				} else {
					alpha_tmp -= OUT_FADE_SPEED;
					if ( alpha_tmp <= alpha ){
						alpha_tmp = ( int )alpha;
					}
				}
				SK_PrintfChengColor2( pWork->font , SK_R , SK_G , SK_B , alpha_tmp , ASCCI_MAX ); // 一行いっきに書き換え
				break;
			case 2 : // birth
				alpha_tmp = pWork->birth.obj[ 0 ]->sprite.col.a;
				if ( alpha_tmp < ( int )alpha ){
					alpha_tmp += IN_FADE_SPEED;
					if ( alpha_tmp >= alpha ){
						alpha_tmp = ( int )alpha;
					}
				} else {
					alpha_tmp -= OUT_FADE_SPEED;
					if ( alpha_tmp <= alpha ){
						alpha_tmp = ( int )alpha;
					}
				}
				SK_PrintfChengColor2( pWork->birth.obj , SK_R , SK_G , SK_B , alpha_tmp , 10 ); // 一行いっきに書き換え
				break;
			case 3 : // region
				alpha_tmp = pWork->region.obj[ 0 ]->sprite.col.a;
				if ( alpha_tmp < ( int )alpha ){
					alpha_tmp += IN_FADE_SPEED;
					if ( alpha_tmp >= alpha ){
						alpha_tmp = ( int )alpha;
					}
				} else {
					alpha_tmp -= OUT_FADE_SPEED;
					if ( alpha_tmp <= alpha ){
						alpha_tmp = ( int )alpha;
					}
				}
				SK_PrintfChengColor2( pWork->region.obj , SK_R , SK_G , SK_B , alpha_tmp , MAX_REGION_OBJ ); // 一行いっきに書き換え
				pWork->region.kcej->sprite.col.a = ( int )alpha_tmp;
				break;
			default :
				spr_child = L2D_GetObject( pWork->handle_2d , CopyTable[ i ][ j ] );
				alpha_tmp = spr_child->sprite.col.a;
				if ( ( pWork->flag & SK_FADE_OK ) && ( alpha == 128 ) ){ // ドットが動いている時はαを操作しない
					alpha_tmp = alpha;
				} else if ( alpha_tmp < ( int )alpha ){
					alpha_tmp += IN_FADE_SPEED;
					if ( alpha_tmp >= alpha ){
						alpha_tmp = ( int )alpha;
						if ( alpha == 128 ){
							pWork->flag |= SK_FADE_OK;
						}
					}
				} else {
					alpha_tmp -= OUT_FADE_SPEED;
					if ( alpha_tmp <= alpha ){
						alpha_tmp = ( int )alpha;
					}
				}
				spr_child->sprite.col.a = ( int )alpha_tmp;
				break;
			}
			j++;
		}
	}
}

// モーフィングを設定してくれる関数
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan *hokan )
{
	void *parts;

	if ( strcode == 0 ){
		return;
	}
	parts = L2D_GetParts( handle , strcode );    /* パーツポインタの取得 */

	if ( parts == NULL ){
#ifdef DEBUG_MODE
//		SK_Err("morf failed\0");
#endif
		return;
	}
	hokan->p += ( 1.0f - hokan->p ) / hokan->count;
	hokan->count--;

	L2D_MorfObject( parts , code1 , code2 , hokan->p );
	if ( ( hokan->p == 1.0f ) || ( hokan->count == 0 ) ){
		hokan->flag = SK_MOVE_OK;
	}
}

#define STR_NODE_FRAME_L2D (13421591)
#define SIG_OPENSLANT1     (4557548)
#define SIG_OPENSLANT4     (4557551)
#define SIG_SHOWMENU       (10212634)
#define SIG_CLOSETORIGHT   (15006264)
#define SIG_CLOSESLANT1    (15006263)
#define SIG_CODE_CLOSE1	   (0x00e4fa37)		/* closeSlant1 */
#define SIG_CODE_CLOSE2	   (0x00e4fa38)		/* closeSlant2 */

#define SIG_CODE_CLOSE_T   (0x004873ec)		/* closeToTop */
#define SIG_CODE_CLOSE_B   (0x00482c89)  	/* closeToBtm */
#define SIG_CODE_CLOSE_L   (0x000a54bd)		/* closeToLeft */

#define SIG_CODE_OPEN_D	   (0x0007601f)		/* openDown */
#define SIG_CODE_CLOSE_D   (0x00da47fa)		/* closeDown */

static void Signal( void *work , int sign , int value )
{
	Work *pWork;
	static int strcode_show[] = { STR_SHOW_NAME , STR_SHOW_SEX , STR_SHOW_BIRTH , STR_SHOW_BLD , STR_SHOW_REG , STR_SHOW_UNDERLINE , STR_SHOW_OK };
	static int strcode_hide[] = { STR_HIDE_NAME , STR_HIDE_SEX , STR_HIDE_BIRTH , STR_HIDE_BLD , STR_HIDE_REG , STR_HIDE_UNDERLINE , STR_HIDE_OK };
	// STR_SHOW_MARUBATSU , STR_HIDE_MARUBATSU ,
	static int strcode_parts[] = { STR_PARTS_NAME_NAME , STR_PARTS_NAME_SEX , STR_PARTS_NAME_BIRTH , STR_PARTS_NAME_BLD , 
								   STR_PARTS_NAME_REG , STR_UNDERLINE , STR_PARTS_MENU_OK }; // STR_PARTS_NEXT_CANSEL
	int i;

	pWork = ( Work * )work;

	switch ( sign ){
	case SIG_OPENSLANT1 : // node frame action 実行
	case SIG_OPENSLANT4 : // node frame action 実行
		// 外部L2D アクション実行
		L2D_EvokeAction( pWork->handle_out , SIG_OPENSLANT1 );
		SE_WINOPEN_R();
		pWork->flag |= SK_SIGNAL_OK;
		break;
	case SIG_SHOWMENU : // ひらく
		// value で menu 表示
	  for ( i = 0 ; i < MAX_KEY_ANIME - 1 ; i ++ ){ // OK はのぞく
			pWork->disp_hokan[ i ].p = 0.0f;
			pWork->disp_hokan[ i ].count = value;
			pWork->disp_hokan[ i ].code1 = strcode_show[ i ];
			pWork->disp_hokan[ i ].code2 = strcode_hide[ i ];
			pWork->disp_hokan[ i ].morf_parts = strcode_parts[ i ];
			pWork->disp_hokan[ i ].flag = 0;
//			pWork->flag |= SK_NAME_ENTRY;
		}
		// etc
		pWork->dot_hokan.p = 0.0f;
		pWork->dot_hokan.count = 20;
		pWork->dot_hokan.code1 = STR_CUR_NAME;
		pWork->dot_hokan.code2 = STR_HIDE_CUR;
		pWork->dot_hokan.morf_parts = STR_PARTS_SELCURS;
		pWork->dot_hokan.flag = 0;
		break;
	case SIG_CODE_CLOSE1 :
	case SIG_CODE_CLOSE2 :
	case SIG_CODE_CLOSE_T :
	case SIG_CODE_CLOSE_B :
	case SIG_CODE_CLOSE_L :
	case SIG_CODE_CLOSE_D :
		// 外部L2D アクション実行
		L2D_EvokeAction( pWork->handle_out , SIG_CLOSESLANT1 );
		SE_WINCLOSE_L();
		break;
	}
}

/*----------------------------------------------------------------*/
/*
static	int	GetResources( Work *work )
{
	u_char    *ascci;
	int		handle;
	int          i;

	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MENU , 0, 0 ) ;
	if ( handle < 0 ){
		SK_Err("name_l2d\0");
		return -1 ;
	}
	work->handle_2d = handle ;
	L2D_EvokeAction( work->handle_2d , STR_DEFAULT );

	// frame l2d
	work->handle_out = -1;
	handle = L2D_LoadLayout( STR_NODE_FRAME_L2D , DG_CHANL_MENU , 0, 0 ) ;
	if ( handle < 0 ){
		SK_Err("frame_l2d\0");
		return -1 ;
	}
	work->handle_out = handle ;
	L2D_EvokeAction( work->handle_out , STR_DEFAULT );
	work->child_mode = 0;

	// l2d
	L2D_SetSignalHandle( work->handle_2d , work , Signal );

	work->action = STR_OPENENTRY;
	work->flag = SK_ACTION_START | SK_NORMAL;
	work->act = (void *)AnimetionAct;
	work->name.position = 0;
	work->name.old_position = 0;
	for ( i = 0 ; i < 2 ; i ++ ){
		work->pad_status[ i ] = 0;
		work->pad_check[ i ] = 0;
	}
	// initialize
	ascci = ( char * )GM_MyName;
	ascci[ 0 ] = 0x41; // a
	for ( i = 1 ; i < ASCCI_MAX ; i ++ ){
		ascci[ i ] = 0x80; // space
	}
	// dot
	work->dot_hokan.p = 0.0f;
	work->dot_hokan.count = 1;
	work->dot_hokan.morf_parts = STR_PARTS_SELCURS;
	MorfL2d( work->handle_2d , STR_PARTS_SELCURS  , STR_HIDE_CUR , STR_HIDE_CUR , &work->dot_hokan );
	// normal_mode
	work->normal_mode.position = 0;
	work->normal_mode.position_max = 0;
	work->normal_mode.old_position = 0;
	// sex
	work->sex.position = 0;
	work->sex.old_position = 0;
	work->sex.hokan.p = 0.0f;
	work->sex.hokan.count = 1;
	work->sex.hokan.morf_parts = STR_PARTS_SEX;
	// blood
	work->blood.position = 0;
	work->blood.old_position = 0;
	work->blood.hokan.p = 0.0f;
	work->blood.hokan.count = 1;
	work->blood.hokan.morf_parts = STR_PARTS_BLD;
	// birth
	work->birth.year = 1970;
	work->birth.month = 1;
	work->birth.day = 1;
	work->birth.phase = 0;
	// region
#ifdef ENGLISH
	work->region.position = 46;
	work->region.old_position = 46;
#endif
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	work->region.position = 46;
	work->region.old_position = 46;
#endif
#ifdef JAPANESE_BP_IGNORE()
	work->region.position = 20;
	work->region.old_position = 20;
#endif
	// gcl 
	work->proc_prev = GCL_GetOptionValue( 'p' , 0 ); // procの読み込み
	work->proc_next = GCL_GetOptionValue( 'n' , 0 ); // procの読み込み
	// name
	memcpy( work->backup_name , ascci , ASCCI_MAX );
	// dup
	work->dup_count.font = 0;
	work->dup_count.region = 0;
	work->dup_count.birth = 0;

	return 0 ;
}
*/

/*
 * ノードから呼び出した場合の初期化
 */
static	int	GetResourcesByNode( Work *work, int frame_l2d )
{
   float safeZoneOffsetY=0;
	u_char  *ascci;
	int		handle;
	int        tmp;
	int          i;

	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;
	{
	  void * ptr;
	  if(NULL == (ptr = GV_GetCache(GV_CacheID(LAYOUT, 'o'))))
	    ptr = CDC_GetFileEntry( LAYOUT, 'o' );
	  ASSERT(NULL != ptr);
	  handle = L2D_SetupLayout2( ptr , DG_CHANL_MENU, 0, SPR_FLAG_PRIV, 0, safeZoneOffsetY);
	}
	if ( handle < 0 ){
	  SK_Err("name_l2d\0");
	  return -1 ;
	}
	work->handle_2d = handle ;
	L2D_EvokeAction( work->handle_2d , STR_DEFAULT );

	// frame l2d
	work->handle_out = frame_l2d;
	L2D_EvokeAction( work->handle_out , STR_DEFAULT );
	work->child_mode = 1;

	// l2d
	L2D_SetSignalHandle( work->handle_2d , work , Signal );

	work->action = STR_OPENENTRY;
	work->flag = SK_ACTION_START | SK_NORMAL;
	work->act = (void *)AnimetionAct;
	for ( i = 0 ; i < 2 ; i ++ ){
		work->pad_status[ i ] = 0;
		work->pad_check[ i ] = 0;
	}
	// initialize
	// soft
	work->soft.x = 0;
	work->soft.y = 0;
	work->soft.add_x = 0;
	work->soft.add_y = 0;
	// dot
	work->dot_hokan.p = 0.0f;
	work->dot_hokan.count = 1;
	work->dot_hokan.morf_parts = STR_PARTS_SELCURS;
	// normal_mode
	work->normal_mode.position = 0;
	work->normal_mode.position_max = 0;
	work->normal_mode.old_position = 0;
	// sex
	work->sex.position = 0;
	work->sex.old_position = 0;
	work->sex.back_position = 0;
	work->sex.hokan.p = 0.0f;
	work->sex.hokan.count = 1;
	work->sex.hokan.morf_parts = STR_PARTS_SEX;
	// blood
	work->blood.position = 0;
	work->blood.old_position = 0;
	work->blood.back_position = 0;
	work->blood.hokan.p = 0.0f;
	work->blood.hokan.count = 1;
	work->blood.hokan.morf_parts = STR_PARTS_BLD;
	work->blood.hokan.flag = SK_MOVE_OK;
	// birth
	work->birth.year = 1970;
	work->birth.month = 1;
	work->birth.day = 1;
	work->birth.phase = 0;
	work->birth.flag = 0;
	// disp_hokan
	for ( i = 0 ; i < MAX_KEY_ANIME ; i ++ ){
		work->disp_hokan[ i ].p = 0.0f;
		work->disp_hokan[ i ].count = 0;
		work->disp_hokan[ i ].code1 = 0;
		work->disp_hokan[ i ].code2 = 0;
		work->disp_hokan[ i ].morf_parts = 0;
		work->disp_hokan[ i ].flag = 0;
	}

	// region 2002.10.1 YANO 
    work->region.position 		= GM_MyRegionData;
	work->region.old_position 	= GM_MyRegionData;

	ascci = ( char * )GM_MyName;
#if 0  // ＭＹＮＡＭＥを反映させたいため変えます。2002.08.21 T.Morita
	tmp = GM_PlantClearCount < 0 ? 0 : GM_PlantClearCount;
#else
	tmp = *ascci ? 1 : 0 ;
#endif

#ifdef DEBUG_MODE 
	printf("clear_count %d\n" , tmp );
#endif
	switch ( tmp ){

	case 0 : // １周目
		//ascci[ 0 ] = 0x41; // a
		for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
			ascci[ i ] = 0x80; // space
		}
		work->name.position = 0;
		work->name.old_position = 0;
		break;
	default : // ２周目以降
		for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
#ifdef DEBUG_MODE
			//printf("%x\n" , ascci[ i ] );
#endif
			if ( ascci[ i ] == 0x0 ){ // null last code
				break;
			}
		}
		work->name.position = ( i <= 0 ? 0 : i );
		work->name.old_position = work->name.position;
		if ( i == 0 ){ // 一応初期化ありえない
			ascci[ 0 ] = 0x20;
			for ( i = 1 ; i < ASCCI_MAX ; i ++ ){
				ascci[ i ] = 0x80; // space
			}
		} else {
			ascci[ i ] = 0x61;
			for ( i = i + 1 ; i < ASCCI_MAX ; i ++ ){
				ascci[ i ] = 0x80; // space
			}
		}
#ifdef DEBUG_MODE
		for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
	  		//printf("%c" , ascci[ i ] );
		}
#endif
		// etc
		work->sex.position = GM_MySexData;
		work->birth.year = GM_MyYearData;
		work->birth.month = GM_MyMonthData;
		work->birth.day = GM_MyDayData;
		work->blood.position = GM_MyBloodData;
		work->region.position = work->region.old_position = GM_MyRegionData;
		work->birth.flag |= SK_BIRTH_FIRST_INPUT;
		break;
	}
	// gcl 
	work->proc_prev = 0;
	work->proc_next = 0;

	/*
	  work->proc_prev = GCL_GetOptionValue( 'p' , 0 ); // procの読み込み
	  work->proc_next = GCL_GetOptionValue( 'n' , 0 ); // procの読み込み
	*/
	// name
	memcpy( work->backup_name , ascci , ASCCI_MAX );
	// dup
	work->dup_count.font = 0;
	work->dup_count.region = 0;
	work->dup_count.birth = 0;

	// staff data initialize
	NameWorkInitialize( work );
	NameWorkListMng( work );
	// ng word
	NgWorkInitialize( work );
	NgWorkListMng( work );

	return 0 ;
}

/*----------------------------------------------------------------*/
/*
void *NewNameLayout( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->scn_name = name;
	if ( GetResources( work ) < 0 ) {
//		GV_CallParentSignalFunc( pWork , NAME_ENTRY_END , 0 );
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
*/
/*
 * ノードから立ち上げるための起動関数
 */
void *NewNameLayoutByNode( int name, int frame_l2d )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 32 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->scn_name = name;
	if ( GetResourcesByNode( work, frame_l2d ) < 0 ) {
//		GV_CallParentSignalFunc( pWork , NAME_ENTRY_END , 0 );
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

