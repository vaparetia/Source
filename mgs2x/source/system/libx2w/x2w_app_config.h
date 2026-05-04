/*--------------------------------------------------------------*/
/*	x2w_app_config.h											*/
/*					アプリケーションコンフィグ					*/
/*--------------------------------------------------------------*/
#ifndef	__X2W_APP_CONFIG_H__
#define	__X2W_APP_CONFIG_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __X2W_APP_CONFIG_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

/*--------------------------------------------------------------*/
/*	設定項目													*/
/*--------------------------------------------------------------*/
enum
{
	X2WAPPCFG_NULL			= 0x000,		// DUMMY

	/* 起動設定 */
	X2WAPPCFG_WINDOWED		= 0x001,		// 0:FullScreenで起動, 1:Windowsモードで起動
	X2WAPPCFG_DISP_FORMAT	= 0x002,		// 画面フォーマット(0:16bit, 1:32bit)
	X2WAPPCFG_DISP_WIDTH	= 0x003,		// 表示幅
	X2WAPPCFG_DISP_HEIGHT	= 0x004,		// 表示高
	X2WAPPCFG_DEPTH_FORMAT	= 0x005,		// Z-Bufferフォーマット(0:16bit, 1:32bit)
	X2WAPPCFG_REND_WIDTH	= 0x006,		// 描画用バッファ幅(2^n)
	X2WAPPCFG_REND_HEIGHT	= 0x007,		// 描画用バッファ高(2^n)

	/* PixelShader系設定 */
	X2WAPPCFG_USE_PIXEL_SHADER	= 0x010,		// PixelShader使用フラグ

	/* VertexShader系設定 */
	X2WAPPCFG_OBJ_USE_OBJ_MULTITEX	= 0x020,	// モデル系のマルチテクスチャ実装フラグ
	X2WAPPCFG_EVM_USE_OBJ_MULTITEX	= 0x021,	// EVMモデル系のマルチテクスチャ実装フラグ
	X2WAPPCFG_PRIM_USE_VERTEXSHADER	= 0x022,	// プリミティブ系モデルVertexShader使用

	/* 表示系設定 */
	X2WAPPCFG_DYNAMICLIGHT_MAX		= 0x030,	// [未実装]動的光源最大数(0～4)
	X2WAPPCFG_SHADOWTEX_WIDTH		= 0x031,	// 影Texture幅(2^n)
	X2WAPPCFG_SHADOWTEX_HEIGHT		= 0x032,	// 影Texture高(2^n)
	X2WAPPCFG_CROSSFADE_TEXLEVEL	= 0x033,	// クロスフェード解像度レベル(最高:0～)
	X2WAPPCFG_REND_OFS_W			= 0x034,	// BackBuffer描画オフセット(0～0x10000)
	X2WAPPCFG_REND_OFS_H			= 0x035,	//	 	(＊)画像のクリアさが変わります

	/* 表示エフェクト系設定 */
	X2WAPPCFG_PROJECTION_SHADOW		= 0x040,	// 投影Shadow使用フラグ
	X2WAPPCFG_OPTCMF_EFFECT			= 0x041,	// [未実装]光学迷彩系エフェクト(0:通常, 1;簡易)
	X2WAPPCFG_FOCUS_EFFECT			= 0x042,	// FOCUS系エフェクト使用フラグ
	X2WAPPCFG_CODEC_FOCUS_EFFECT	= 0x043,	// 無線FOCUS系エフェクト使用フラグ
	X2WAPPCFG_BLUR_EFFECT			= 0x044,	// BLUR系エフェクト使用フラグ
	X2WAPPCFG_SMKBLUR_EFFECT		= 0x045,	// [実装?]煙型BLUR系エフェクト使用フラグ
	X2WAPPCFG_MIRROR_OBJ_EFFECT		= 0x046,	// 鏡面反射オブジェクト使用フラグ
	X2WAPPCFG_CROSSFADE_ENABLE		= 0x047,	// クロスフェード使用許可フラグ
	X2WAPPCFG_INWATER_EFFECT_LEVEL	= 0x048,	// 水中エフェクト量調整(NONE:0～256:MAX)
	X2WAPPCFG_RAIN_EFFECT_LEVEL		= 0x049,	// 雨エフェクト量調整(NONE:0～256:MAX)
	X2WAPPCFG_VR_BREAKBODY_ENABLE	= 0x04A,	// VR敵兵体破壊エフェクト許可(TEST)
	X2WAPPCFG_DISP_VIBRATION_ENABLE	= 0x04B,	// 画面振動(PAD代替機能)許可(TEST)

	/* 入力設定 */
	X2WAPPCFG_MOUSE_ENABLE			= 0x060,	// マウス入力許可
	X2WAPPCFG_MOUSE_ENABLE2			= 0x061,	// マウス入力許可

	/* デバイスネーム */
	X2WAPPCFG_DEV_NAME00			= 0x070,
	X2WAPPCFG_DEV_NAME01			= 0x071,
	X2WAPPCFG_DEV_NAME02			= 0x072,
	X2WAPPCFG_DEV_NAME03			= 0x073,
	X2WAPPCFG_DEV_NAME04			= 0x074,
	X2WAPPCFG_DEV_NAME05			= 0x075,
	X2WAPPCFG_DEV_NAME06			= 0x076,
	X2WAPPCFG_DEV_NAME07			= 0x077,
	X2WAPPCFG_DEV_NAME08			= 0x078,
	X2WAPPCFG_DEV_NAME09			= 0x079,
	X2WAPPCFG_DEV_NAME10			= 0x07A,
	X2WAPPCFG_DEV_NAME11			= 0x07B,
	X2WAPPCFG_DEV_NAME12			= 0x07C,

	/* サウンド設定 */
	X2WAPPCFG_SND_NAME00			= 0x090,
	X2WAPPCFG_SND_NAME01			= 0x091,
	X2WAPPCFG_SND_NAME02			= 0x092,
	X2WAPPCFG_SND_NAME03			= 0x093,
	X2WAPPCFG_SND_NAME04			= 0x094,
	X2WAPPCFG_SND_NAME05			= 0x095,
	X2WAPPCFG_SND_NAME06			= 0x096,
	X2WAPPCFG_SND_NAME07			= 0x097,
	X2WAPPCFG_SND_NAME08			= 0x098,
	X2WAPPCFG_SND_NAME09			= 0x099,
	X2WAPPCFG_SND_NAME10			= 0x09A,
	X2WAPPCFG_SND_NAME11			= 0x09B,
	X2WAPPCFG_SND_NAME12			= 0x09C,

	X2WAPPCFG_SND_QUALITY			= 0x09D,	// 再生レート
	X2WAPPCFG_SND_SE				= 0x09E,	// SE発音数
	X2WAPPCFG_SND_3DQUALITY			= 0x09F,	// 3Dサウンドのクォリティ

	/* 最大設定数 */
	MAX_X2WAPPCFG_NUM,


} ;

/*--------------------------------------------------------------*/
/*	広域変数													*/
/*--------------------------------------------------------------*/
EXTERN	DWORD	_X2W_AppConfig[MAX_X2WAPPCFG_NUM] ;

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/
extern	BOOL	X2W_InitAppConfig(void) ;
extern	void	X2W_DestAppConfig(void) ;

#define	X2W_GetAppConfig(stat_)	(_X2W_AppConfig[(stat_)])

extern	char	*X2W_GetGraphicDeviceName(char *name, int size) ;
extern	char	*X2W_GetSoundDeviceName(char *name, int size) ;

#undef	EXTERN
#ifdef __cplusplus
}
#endif
#endif	/* !defined(__X2W_APP_CONFIG_H__) */
