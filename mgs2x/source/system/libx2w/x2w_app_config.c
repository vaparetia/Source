/*--------------------------------------------------------------*/
/*	x2w_app_config.c											*/
/*					アプリケーションコンフィグ管理				*/
/*--------------------------------------------------------------*/
#define	__X2W_APP_CONFIG_C__

#include <stdio.h>
#include <stdlib.h>

#include <xtl.h>

#include "libfs.h"
#include "libgv.h"
#include "x2w_app_config.h"

/*--------------------------------------------------------------*/
/*	define														*/
/*--------------------------------------------------------------*/
//#define	CONFIG_FILE_ROOT	"../"
#define	CONFIG_FILE_ROOT	"../../bin/"
#define	CONFIG_FILENAME		"mgs2.ini"

/*--------------------------------------------------------------*/
/*	macro														*/
/*--------------------------------------------------------------*/
#if FALSE
#define	isSpaceChara(ch_)	(((ch_) == ' ') || ((ch_) == '\t') || ((ch_) == '\n') || ((ch_) == '\r'))
#else
#define	isSpaceChara(ch_)	(((ch_) > 0x00) && ((ch_) < 0x20))
#endif
#define	isTermChara(ch_)	((ch_) == 0)

/*--------------------------------------------------------------*/
/*	ロカール関数宣言											*/
/*--------------------------------------------------------------*/
static	void	SkipSpaceChara(char **buff) ;
static	DWORD	GetValue(char **buff) ;
static	void	PerseConfig(void *buff) ;
static	char	*GetGraphicDeviceName(int stat, char *name, int size)  ;

/*--------------------------------------------------------------*/
/*	GetValue		値取得										*/
/*--------------------------------------------------------------*/
static	DWORD	GetValue(char **buff)
{
	char	*ptr ;
	char	*root ;
	char	*btm ;
	char	ch ;
	DWORD	val ;
	BOOL	chk ;

	/*-- 値文字列終端検索 --------------------------------------*/

	ptr = *buff ;
	ch = *ptr ;
	while( !isSpaceChara(ch) && !isTermChara(ch) )
	{
		ptr++ ;
		ch = *ptr ;
	}
	root = *buff ;	// 先頭保存
	btm  = ptr ;	// 終端保存

	*buff = ptr ;
	/*----------------------------------------------------------*/

	/*-- 値計算 ------------------------------------------------*/

	val = 0 ;
	chk = TRUE ;
	for(ptr=root; ptr<btm; ptr++)
	{
		val <<= 4 ;
		ch = *ptr ;
		if(  !((ch >= '0') && (ch <= '9'))
		  && !((ch >= 'A') && (ch <= 'F')) )
		{
			chk= FALSE ;	// 設定値として正しくない
			break ;
		}

		if( (ch >= '0') && (ch <= '9') )
		{
			val |= (ch - '0') ;
		}
		else 
		{
			val |= (ch - 'A') + 10 ;
		}
	}
	if( !chk ){ return(0) ; }
	/*----------------------------------------------------------*/

	return(val) ;
}

/*--------------------------------------------------------------*/
/*	SkipSpaceChara		空文字Skip								*/
/*--------------------------------------------------------------*/
static	void	SkipSpaceChara(char **buff)
{
	char	*ptr ;
	char	ch ;

	ptr = *buff ;
	ch = *ptr ;
	while( isSpaceChara(ch) )
	{
		ptr++ ;
		ch = *ptr ;
	}
	*buff = ptr ;
}

/*--------------------------------------------------------------*/
/*	PerseConfig		定義内容の解析								*/
/*--------------------------------------------------------------*/
static	void	PerseConfig(void *buff)
{
	char	*ptr ;
	DWORD	stat ;
	DWORD	value ;

	ZeroMemory(_X2W_AppConfig, sizeof(_X2W_AppConfig)) ;

	ptr = buff ;
	while( *ptr )
	{
		/*-- 設定項目 ------------------------------------------*/

		SkipSpaceChara(&ptr) ;
		if( !*ptr ){ break ; }
		
		stat = GetValue(&ptr) ;
		if( !*ptr ){ break ; }
		/*------------------------------------------------------*/

		/*-- 設定値 --------------------------------------------*/

		SkipSpaceChara(&ptr) ;
		if( !*ptr ){ break ; }

		value = GetValue(&ptr) ;
		/*------------------------------------------------------*/

		if( (stat > 0) && (stat < MAX_X2WAPPCFG_NUM) )
		{
#ifdef DEBUG_MODE
			printf("CONFIG[%03X][%03X(%d)]\n", stat, value, value) ;
#endif
			_X2W_AppConfig[stat] = value ;
		}
	}
}

/*--------------------------------------------------------------*/
/*	X2W_InitAppConfig											*/
/*			コンフィグ初期化＆読み込み							*/
/*--------------------------------------------------------------*/
//#include "OptionIO.h"
//#include "app_util.h"

BOOL	X2W_InitAppConfig(void)
{
	ZeroMemory(_X2W_AppConfig, sizeof(_X2W_AppConfig)) ;

	/*-- 設定ファイル読込 --------------------------------------*/

	{
		int		size ;
		void	*buff ;
		char	filename[512] ;	// 多めに

#if FALSE
		FS_ChangeDirectory(CONFIG_FILE_ROOT) ;
#endif
		/*-- 読込要求 ------------------------------------------*/

#if FALSE
		size = FS_LoadRequest(CONFIG_FILENAME) ;	// 読込要求
#else
		sprintf(filename, "%s/" CONFIG_FILENAME, pcGetIniFilePath()) ;
		size = FS_LoadRequestDirectPath(filename) ;
#endif

		if( size <= 0 )
		{
			ASSERT(0) ;		// 読み込み失敗
			return(FALSE) ;
		}
		/*------------------------------------------------------*/

		/*-- 領域確保 ------------------------------------------*/

		buff = malloc(((size+3) & (~3))+1) ;	// +1は終端記号追加の為
		if( !buff )
		{
			ASSERT(0) ;		// 読み込み失敗
			return(FALSE) ;
		}
		/*------------------------------------------------------*/

		/*-- 読込発効 ------------------------------------------*/

		FS_LoadSet(buff, size) ;
		while( FS_LoadSync() > 0 ) ;	// 終了待ち

		((char *)buff)[size] = 0 ;	// 終端(安全策)
		/*------------------------------------------------------*/

		/*-- Perse ---------------------------------------------*/

		PerseConfig(buff) ;
		/*------------------------------------------------------*/

		/*-- 終了処理 ------------------------------------------*/

		free(buff) ;
		buff = NULL ;
		/*------------------------------------------------------*/
	}
	/*----------------------------------------------------------*/

#if FALSE
{
	DWORD	*buff ;
	SOptionIO	opt ;

	ZeroMemory(&opt, sizeof(opt)) ;
	opt.m_setting.m_wWidth  = 640 ;
	opt.m_setting.m_wHeight = 480 ;

	buff = malloc(GetDetailConfigSize()) ;

	MakeDetailConfig(&opt, buff) ;	// 設定データ作成
	OutputDetailConfig("", buff) ;	// 設定データ出力

	free(buff) ;
}
#endif
	return(TRUE) ;
}

/*--------------------------------------------------------------*/
/*	X2W_DestAppConfig											*/
/*			コンフィグ終了処理									*/
/*--------------------------------------------------------------*/
void	X2W_DestAppConfig(void)
{
}

/*--------------------------------------------------------------*/
/*	GetConfigString												*/
/*			設定文字列取得										*/
/*--------------------------------------------------------------*/
static char	*GetGraphicDeviceName(int stat, char *name, int size) 
{
	char	*src ;
	char	*dst ;

	src = (char *)&_X2W_AppConfig[stat] ;
	dst = name ;
	for(; size>3; size-=4, src+=4, dst+=4)
	{
		dst[3] = src[0] ;
		dst[2] = src[1] ;
		dst[1] = src[2] ;
		dst[0] = src[3] ;
	}

	dst += 4 ;
	for(;size>0; size--, dst--, src++)
	{
		*src = *dst ;
	}

	return(name) ;
}

/*--------------------------------------------------------------*/
/*	X2W_GetGraphicDeviceName									*/
/*			グラフィックデバイス名取得							*/
/*--------------------------------------------------------------*/
char	*X2W_GetGraphicDeviceName(char *name, int size) 
{
	return( GetGraphicDeviceName(X2WAPPCFG_DEV_NAME00, name, size) ) ;
}

/*--------------------------------------------------------------*/
/*	X2W_GetSoundDeviceName										*/
/*			サウンドデバイス名取得								*/
/*--------------------------------------------------------------*/
char	*X2W_GetSoundDeviceName(char *name, int size)
{
	return( GetGraphicDeviceName(X2WAPPCFG_SND_NAME00, name, size) ) ;
}

/*-- End Of File --*/
