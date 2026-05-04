/*
	XBOX CRI Stream Driver

	I Picture Only Stream,
	I Picture Only Memory Stream,
	MPEG Stream

	2002/05/20 M.Kobayashi
	$Id: winstream_mpeg.c,v 1.7 2002/12/26 15:02:40 takaki Exp $
*/

//#include	<cri_mw.h>
//#include	<sfvsjd.h>
// 上記２つは後にinclude してはいけない( #define long 問題 )

#include	"mgs_type.h"
#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"libdg.h"
#include	"stream.h"
#include	"g_define.h"

#include	"strctrl.h"
#include	"sd_ee.h"

#include	"filtergraph.h"

//#define DECODE_OTHER_THREAD

typedef struct MPEG_HEADER {
	short	width;
	short	height;
	int		nFrames;
	float	fFps;
	int		pad;
} MPEG_HEADER;

// MPEG 時のリングバッファサイズ
#define RING_BUF_SIZE	(1024 * 128)
#define RING_BUF_EXTEND_SIZE	(1024 * 32)

#define	OPT_FLAG_PRECIOUS		0x80000000	// スキップ不可データ
#define	OPT_FLAG_NOT_MPEG		0x40000000	// MPEG ストリームではないデータ

#define GET_MPEG_STREAM_SIZE( p ) ( FS_STREAM_GET_OPTION( p ) & ~(OPT_FLAG_PRECIOUS | OPT_FLAG_NOT_MPEG ) )


/* ---------------------------------------------------------------- */

/* ストリームドライバ登録情報 */
void *NewStreamCriMpegDriver( GM_STREAM_CONTROL *ctrl, int type );

static GM_STREAM_DRIVER drivermpg = {
	NULL,
	0,
	NewStreamCriMpegDriver,
};

enum {
	STATE_READY,	// ヘッダ読み込み中
	STATE_DEC,		// デコード中
	STATE_END,		// 終了待ち
};


/* ---------------------------------------------------------------- */
typedef struct {
	/* 通常ストリーム用ワーク */

	GV_ACT_EX	actor;

	int					type ;		/* ストリームタイプ */
	GM_STREAM_CONTROL	*pctrl ;	/* ストリームコントロール */
	u_int				flag ;
	int					state ;

	int					start_tick ;
	int					pre_tick ;
	int					cur_tick ;
} CRI_STR_WORK;

enum {
	FLAG_ON_MEMORY	= 0x01,
	FLAG_MPEG		= 0x02,
	FLAG_NO_SKIP	= 0x04,
};

static int	DriverRefCtr = 0 ;
static CRI_STR_WORK	*pWork = NULL ;


/* ---------------------------------------------------------------- */


/* ---------------------------------------------------------------- */
static BOOL CodeIsEnd( u_char* code ) {
	return code[ 0 ] == 0x00 &&
			code[ 1 ] == 0x00 &&
			code[ 2 ] == 0x01 &&
			code[ 3 ] == 0xb7;
}

static void *StreamGetData( CRI_STR_WORK *pw )
{
	if( pw->flag & FLAG_ON_MEMORY ) {
		return NULL;
	} else {
		void	*data_ptr ;

		data_ptr = FS_StreamGetData(pw->pctrl->stream_h, pw->type) ;
		return( data_ptr ) ;
	}		
}

static void StreamFreeData( CRI_STR_WORK* pw, void* pData )
{
	if ( pw->flag & FLAG_ON_MEMORY ){
		return ;
	} else {
		FS_StreamFreeData( pw->pctrl->stream_h, pData );
	}
}
static int StreamIsEnd( CRI_STR_WORK* pw )
{
	if ( pw->flag & FLAG_ON_MEMORY ){
		return 0;
	} else {
		return ( GM_IS_STREAM_END( pw->pctrl ) );
	}
}

static void ReleaseRes( CRI_STR_WORK* pw )
{	// Act 中に確保したリソースの解放
	if( pw->flag & FLAG_MPEG ) {
	} else {
	}
}

static void ActMpeg( CRI_STR_WORK* pw )
{
	void	*pData;
	int		tick ;

	switch( pw->state )
	{
	  case STATE_READY:	// ヘッダ(ファイル情報取得)
		if( (pData=(STREAM_TAG*)StreamGetData(pw))!= NULL )
		{
			if( *(char *)pData != 0 )
			{
				char	str[1024];
				sprintf(str,"%s/pac/%s",pcGetStreamFilePath(),pData);

				pw->start_tick = FS_STREAM_GET_TIME(pData) ;
				pw->pre_tick   = pw->pctrl->tick ;
				pw->cur_tick   = 0 ;

				GM_InitFilterGraphMan() ;	// フィルタグラフマネージャ初期化
				GM_CreateFilterGraph(str);

				pw->state = STATE_DEC ;
#ifdef DEBUG_MODE
				printf("start tick :%d\n", pw->start_tick) ;
				printf("pre   tick :%d\n", pw->pre_tick) ;
#endif
			}
#ifdef DEBUG_MODE
			else
			{
				printf("Null File Selected!!\n\tActMpeg():%s(%d)\n", __FILE__, __LINE__) ;
			}
#endif

			StreamFreeData(pw, pData) ;
		} else {
			if ( !(pw->flag & FLAG_ON_MEMORY) && StreamIsEnd( pw ) ) {
				GV_DestroyActor( pw );
			}
			break;
		}

	  case STATE_DEC:	// デコード中(同期合わせ)
		switch( FS_StreamGetStatus(pw->pctrl) )
		{
		  case FS_STR_STOP :
			if( !GM_FilterGraphGetPauseState() ){ GM_FilterGraphPauseOn() ; }
			break ;

		  default :
			/* 時間同期 */
			tick          = pw->pctrl->tick ;
			pw->cur_tick += tick - pw->pre_tick ;
			pw->pre_tick  = tick ;
			if( pw->start_tick > tick  )
			{
				/* Start待ち */
				if( !GM_FilterGraphGetPauseState() ){ GM_FilterGraphPauseOn() ; }
			}
			else
			{
				/* 同期再生 */
				if( GM_FilterGraphGetPauseState() ){ GM_FilterGraphPauseOff() ; }
				GM_FilterGraphSyncTime(pw->cur_tick) ;
			}
			break ;
		}

		/* 終了判定 */
		if( GM_FilterGraphIsEnd() ){ pw->state = STATE_END ; }
		break;

	  case STATE_END:	// 終了
		pw->state = STATE_READY ;

		ReleaseRes( pw );
		GM_ReleaseFilterGraph() ;		// フィルタグラフ解放処理
		GM_ReleaseFilterGraphMan() ;	// フィルタグラフマネージャ終了処理
		break ;
	}
}


static void Die( CRI_STR_WORK* pw )
{
	GM_ReleaseFilterGraphMan() ;	// フィルタグラフマネージャ終了処理

	if ( pWork == pw ){
		pWork = NULL ;
	}
}


static void GetResources( CRI_STR_WORK* pw, GM_STREAM_CONTROL* pctrl, int type )
{
	GV_ActorEX( &pw->actor );
	pWork = pw ;
	pw->type = type;
	pw->pctrl = pctrl;

	pw->state = STATE_READY;
}


void *NewStreamCriMpegDriver( GM_STREAM_CONTROL *pctrl, int type )
{
	// 通常ＭＰＥＧの入り口
	CRI_STR_WORK* pw = NULL ;

	if( ( pw = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM, sizeof( CRI_STR_WORK ), 0xf2 ) ) != NULL ){
		
		GV_SetActor( &pw->actor, ActMpeg, Die );
		GetResources( pw, pctrl, type );
		
		pw->flag = FLAG_MPEG;
	}

	return pw;
}

/* ---------------------------------------------------------------------- */
/*
	ドライバ登録 
*/

/* IPIC,MPEGストリームドライバ登録 */

void GM_StreamCriDriverInit_Mpeg( int dummy )
{
	GM_StreamAddDriver( &drivermpg, CHANK_TYPE_CRI_MPEG );
}

/* IPIC,MPEGストリームドライバ停止 */
void GM_StreamCriDriverEnd_Mpeg( void )
{	// Init を呼ばずに End だけ呼ばれることがある
	GM_StreamRemoveDriver( &drivermpg );
}

/* ---------------------------------------------------------------------- */
/*
	外部呼び出しインターフェイス
*/

/* ストリームの映像幅・高さ・最大フレーム数を取得 */
int GM_StreamGetCriInfo_Mpeg( int *width, int *height, int *frame )
{
	if( pWork == NULL ) return -1;
	if( pWork->state == STATE_READY ) return -1;

	{
		GM_FG_BUFFER_INFO	info ;

		if( !GM_FilterGraphGetCurrentBufferInfo(&info) ) return -1 ;

		if ( width != NULL )  *width  = info.width;
		if ( height != NULL ) *height = info.height;
		if ( frame != NULL )  *frame  = 0;	// 返せない
	}

	/* 終了なら真を返す */
	return FALSE;
}

void* GM_StreamGetCri_Mpeg( void )
{
	if ( pWork == NULL ) return ( NULL );

	return( GM_FilterGraphGetCurrentBuffer() ) ;
}

/* 展開後Ycc420をラスタイメージに変換 */
void GM_StreamCopyCriImage_Mpeg( void *dst, int dst_width, int dst_height, int dst_x, int dst_y, int flag )
{
	DWORD	*src_buff ;

	if ( pWork == NULL ) return ;

	src_buff = GM_FilterGraphGetCurrentBuffer() ;

	if( pWork->flag & FLAG_MPEG ) {
		if ( src_buff != NULL ) {
			int	x,y;
 			BYTE	*pd;
 			BYTE	*ps;
 			int		sw, sh;
 			int		s_width_tmp;
 			int		d_width_tmp;

			{
				GM_FG_BUFFER_INFO	info ;

				if( GM_FilterGraphGetCurrentBufferInfo(&info) )
				{
					sw = info.width ;
					sh = info.height ;
				}
				else
				{
					sw = 512 ;	// 安全策
					sh = 320 ;
				}
			}

			s_width_tmp = sw * 3;
			d_width_tmp = dst_width * 4;
			
			ps = (BYTE*)src_buff;
			for(y = 0 ; y <  sh ; y ++){
				pd = (BYTE*)dst + dst_x * 4 + (dst_y + ((sh - y) - 1)) * d_width_tmp;
				for(x=0; x<sw; x++)
				{
					pd[0] = ps[0] ;
					pd[1] = ps[1] ;
					pd[2] = ps[2] ;
					pd[3] = 0xff ;
					ps += 3;
					pd += 4;
				}
			}
		}
	}
}

/* 展開後バッファの開放 */
void GM_StreamFreeCri_Mpeg( void )
{
	if ( pWork == NULL ) return ;

	GM_FilterGraphUnLockBuffer() ;
}

BOOL	GM_StreamIsEndCri_Mpeg( void )
{
	if( pWork == NULL ) return TRUE;

	return( GM_FilterGraphIsEnd() ) ;
}
