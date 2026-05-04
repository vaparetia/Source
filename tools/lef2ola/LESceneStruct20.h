/*
	LESceneStruct20.h

   2002/12/11 Nobumitsu Tanaka
   $Id: LESceneStruct20.h,v 1.2 2002/12/11 06:28:36 usr03635 Exp $
*/

////////////////////////////////////////////////////////////////////////////
// 2D ツール [ Layout Editor ] Scene データファイル構造体定義ファイル
//
// このファイルには "Layout Editor" の専用形式 "lef" のデータ構造が定義されています。


#ifndef  __LESCENESTRUCT20
#define __LESCENESTRUCT20

////////////////////////////////////////////////////////////////////////////
// 構造
// LEF_LE_SCENE_HEADER
//   LEF_LE_SCENE_INFO
//   LEF_LE_ANIME_TREE_HEADER
//     LEF_LE_ANIME_ACTOR		

//   LEF_LE_OBJECT_TREE_HEADER
//     LEF_LE_GROUP_HEADER
//       LEF_LE_OBJECT
//         LEF_LE_ANIME_OBJECT
//           LEF_LE_ANIME_KEY    

// LEF_LE_PICTURE_TREE_HEADER
//   PICTURE_FILE_DATA

enum {
	LEPRIM_NULL = 0,		/* NULL オブジェクト */
	LEPRIM_LINE,			/* ラインオブジェクト */
	LEPRIM_RECT,			/* 長方形オブジェクト */
	LEPRIM_FILLRECT,		/* 塗り潰し長方形オブジェクト */
	LEPRIM_TEXRECT,			/* テクスチャ張り付けオブジェクト */
	LEPRIM_SPINRECT,		/* 回転オブジェクト */
	LEPRIM_FILLPOLYGON,		/* 任意四角形オブジェクト */
	LEPRIM_FILLTRIANGLE,	/* 三角形オブジェクト */
	LEPRIM_TEXPOLYGON,		/* 任意四角形テクスチャ張り付けオブジェクト */
	LEPRIM_STRIP,			/* ストリップオブジェクト */
	LEPRIM_FILLSTRIP,		/* 塗り潰しストリップオブジェクト */
};

enum {
	LA_EMPTY = 0,		/* empty */
	LA_BOX,				/* box */
	LA_SPRT,			/* sprt */
	LA_RSPRT,			/* rsprt */
	LA_TRIANGLE,		/* triangle */
	LA_QUAD,			/* quad */
	LA_LINE,			/* line */
	LA_LINESTRIP,		/* linestrip */
	LA_TRIANGLESTRIP,	/* trianglestrip */
	LA_TRIANGLEFAN,		/* trianglefan */
};

////////////////////////////////////////////////////////////////////////////
// Layout Editor セーブファイルのヘッダー
typedef struct tagLEFLESCENEHEADER {
	char         szFileType[32];
							// ファイル識別用テキスト
	unsigned int uiVersion;	// バージョン情報
	unsigned int uiSize;	// ファイルサイズ

	long lLEScene;		// シーン情報ヘッダーへのファイルポインタ
	long lLEPictures;   // ピクチャー情報ヘッダーへのファイルポインタ 
	long lLEAnime;	// アニメ情報ヘッダーへのファイルポインタ
	long lLEObject;		// オブジェクトデータへのファイルポインタ
} LEFLESCENEHEADER;

////////////////////////////////////////////////////////////////////////////
// 保存されているLayout Editorのシーン情報
typedef struct tagLEFLESCENEINFO {
// メインフィールド設定
	int   nOffsetX;
	int   nOffsetY;
	int   nFieldWidth;
	int   nFieldHeight;

// カラー設定
	unsigned int uiFieldRGBA;
	unsigned int uiBGroundRGBA;

// ユーザー設定カラー
	unsigned int aryuiCustomColor[16];
} LEFLESCENEINFO;

////////////////////////////////////////////////////////////////////////////
// 保存されているLEPictureの総合ヘッダー
typedef struct tagLEFLEPICTURETREEHEADER {
	int nPictureCount;	// 全オブジェクト数
} LEFLEPICTURETREEHEADER;

////////////////////////////////////////////////////////////////////////////
// 保存されているLEObjectの総合ヘッダー
typedef struct tagLEFLEOBJECTTREEHEADER {
	int nTotalObjectCount;	// 全オブジェクト数
} LEFLEOBJECTTREEHEADER;

////////////////////////////////////////////////////////////////////////////
// 保存されているLEObjectの１グループ分のデータヘッダー
typedef struct tagLEFLEOGROUPHEADER {
	int nGroupObjectCount;	// このグループに含まれるオブジェクト数。
	int nParentObjectNumber;	// このグループの親の識別番号
								// このグループはすべてこの親にぶら下がっている
} LEFLEOGROUPHEADER;




////////////////////////////////////////////////////////////////////////////
// オブジェクト共通データ
typedef struct tagLEFOBJECTCOMMON {
	int          nObjectType;
	int          nOriginalNumber;
	int          nMoveX;
	int          nMoveY;
	int          nAnimeObjectCount;		// アニメ関連 0 でないときはアニメが続いている
	int          nBlend;
	int          nPriority;
	int          nVisible;
	int          nNameLength;
	float        fMagni;
	unsigned int uiColorRGBA[4];
} LEFOBJECTCOMMON;
// この後に名称の文字列が来る


////////////////////////////////////////////////////////////////////////////
// オブジェクト頂点データ
typedef struct tagLEFOBJECTVERTEX {
	int nVertexCount;
} LEFOBJECTVERTEX;
// この後に、ＸＹの順で、nVertexCount の数だけ頂点が並ぶ

////////////////////////////////////////////////////////////////////////////
// オブジェクトテクスチャデータ
typedef struct tagLEFOBJECTTEX {
	int arynUV[2];
	int arynUVSize[2];
	int nTexNameLength;
} LEFOBJECTTEX;
// この後に名称の文字列が来る

////////////////////////////////////////////////////////////////////////////
// オブジェクトスピンデータ
typedef struct tagLEFOBJECTSPIN {
	int   nSpinCenterX;
	int   nSpinCenterY;
	int   nRotate;
	int   nRotateMode;
} LEFOBJECTSPIN;

////////////////////////////////////////////////////////////////////////////
// オブジェクトストリップデータ
typedef struct tagLEFOBJECTSTRIP {
	int   nCloseFlag;
} LEFOBJECTSTRIP;

////////////////////////////////////////////////////////////////////////////
// オブジェクト塗り潰しストリップデータ
typedef struct tagLEFOBJECTFILLSTRIP {
	int   nFillFlag;
} LEFOBJECTFILLSTRIP;

////////////////////////////////////////////////////////////////////////////
// 保存されているLEAimeの総合ヘッダー
typedef struct tagLEFLEANIMETREEHEADER {
	int nAnimeActorCount;	// 全AnimeActorの数
} LEFLEANIMETREEHEADER;

////////////////////////////////////////////////////////////////////////////
// 保存されているLEAnimeActorのデータ
typedef struct tagLEFLEANIMEACTOR {
	int  nAnimeID;
	int  nRangeStart;
	int  nRangeEnd;
	int  nActorNameLength;
} LEFLEANIMEACTOR;
// この後に名称の文字列が来る

////////////////////////////////////////////////////////////////////////////
// Anime オブジェクト共通データ
typedef struct tagLEFANIMEOBJECTCOMMON {
	int          nAnimeID;
	int          nKeyCount;
	int          nDefineKeyCount;
	int          nNameLength;
	unsigned int uiDefColorRGBA[4];
} LEFANIMEOBJECTCOMMON;
// この後に名称の文字列が来る

////////////////////////////////////////////////////////////////////////////
// Anime オブジェクト頂点データ
typedef struct tagLEFANIMEOBJECTVERTEX {
	int nVertexCount;
} LEFANIMEOBJECTVERTEX;
// この後に、ＸＹの順で、nDefineVertexCountの数だけ頂点データが並ぶ。
// この後に、ＸＹの順で、nMoveVertexCountの数だけ頂点データが並ぶ。

////////////////////////////////////////////////////////////////////////////
// Anime オブジェクトテクスチャデータ
typedef struct tagLEFANIMEOBJECTTEX {
	int  arynDefUV[2];
	int  arynDefUVSize[2];
	int  nTexNameLength;
} LEFANIMEOBJECTTEX;
// この後に名称の文字列が来る

////////////////////////////////////////////////////////////////////////////
// Anime オブジェクト回転データ
typedef struct tagLEFANIMEOBJECTSPIN {
	int  nDefSpinCenterX;
	int  nDefSpinCenterY;
	int  nDefRotate;
	int  nDefRotateMode;
} LEFANIMEOBJECTSPIN;

////////////////////////////////////////////////////////////////////////////
// AnimeKey 共通データ
typedef struct tagLEFANIMEKEYCOMMON {
	short        sType;
	short        sVisible;
	int          nFrame;
	int          nKeyNameLength;
	float        fMagni;
	unsigned int uiColorRGBA[4];
	char         cAnimeCommonOutFlag;
	char         cVisibleOutFlag;
	char         cMagniOutFlag;
	char         cVertexOutFlag;
	char         cSizeOutFlag;
	char         cRGBAOutFlag;
	char         cTmp[3];
} LEFANIMEKEYCOMMON;
// この後に名称の文字列が来る

////////////////////////////////////////////////////////////////////////////
// AnimeKey 頂点データ
typedef struct tagLEFANIMEKEYVERTEX {
	int nVertexCount;
} LEFANIMEKEYVERTEX;
// この後にＸＹの順でnMoveVertexCountの頂点が来る

////////////////////////////////////////////////////////////////////////////
// AnimeKey テクスチャデータ
typedef struct tagLEFANIMEKEYTEX {
	int  arynUV[2];
	int  arynUVSize[2];
	int  nTexNameLength;
	char cAnimeTextureOutFlag;
	char cTextureNameOutFlag;
	char cUVOutFlag;
	char cTmp;
} LEFANIMEKEYTEX;
// この後に名称の文字列が来る

////////////////////////////////////////////////////////////////////////////
// AnimeKey 回転データ
typedef struct tagLEFANIMEKEYSPIN {
	int  nSpinCenterX;
	int  nSpinCenterY;
	int  nRotate;
	int  nRotateMode;
	char cAnimeSpinOutFlag;
	char cSpinCenterOutFlag;
	char cRotateOutFlag;
	char cTmp;
} LEFANIMEKEYSPIN;

////////////////////////////////////////////////////////////////////////////
// AnimeKey イベントデータ
typedef struct tagLEFANIMEKEYEVENT {
	int nFrame;
	int nKeyNameLength;
	int nMessageLength;
} LEFANIMEKEYEVENT;
// キーの名称が入る
// この後にメッセージ文字列が来る

////////////////////////////////////////////////////////////////////////////
// Picture
typedef struct tagLEFLEPICTURE {
	int nYear;		/* 最終更新日時・年 */
	int nMonth;		/* 最終更新日時・月 */
	int nDay;		/* 最終更新日時・日 */
	int nHour;		/* 最終更新日時・時 */
	int nMin;		/* 最終更新日時・分 */
	int nSec;		/* 最終更新日時・秒 */
	int nWidth;		/* テクスチャの幅 */
	int nHeight;	/* テクスチャの高さ */
	int nNameLength;			/* ファイル名 */
	int nTexPathLength1;	/* フルパス */
	int nTexPathLength2;	/* 相対パス */
	int nTRINameLength;			/* TRIファイル名 */
	unsigned int unFileSize;	/* ピクチャーファイル自体のサイズ */
} LEFLEPICTURE;
// この後にファイル名が来る
// この後にフルパス文字列が来る
// この後に相対テクスチャパス文字列が来る
// この後にTRIファイル名が来る
// この後にテクスチャデータが来る

#endif __LESCENESTRUCT18