/*
   data.h

   2002/12/11 Nobumitsu Tanaka
   $Id: data.h,v 1.2 2002/12/11 06:28:37 usr03635 Exp $
*/

#ifndef __DATA_H__
#define __DATA_H__

/**************************************************************************
* define
**************************************************************************/

#define MAX_TEXT 2048
#define MAX_NAME 256


#define FILE_TYPE "Layout Editor                  \0"
#define LEF_VERSION 20

/**************************************************************************
* typedef 
**************************************************************************/
/* 中間データ抽出時に使用するテーブル */
/* IDと名称の対応表 */
typedef struct tagSell{
	int   nID;
	void *lpData;
} Sell;

typedef struct tagIdDataTable{
	int   nCount;
	Sell *lpSell;
} IdDataTable;

/* 中間データ抽出時に使用するダミーデータ */
typedef struct tagAnimeObjectDummy{
	int   nID;
	int   nCount;
	void *lpKey;
} AnimeObjectDummy;

typedef struct tagObjectDummy{
	int               nCount;
	AnimeObjectDummy *lpAnmObj;
	void             *lpObject;
} ObjectDummy;

typedef struct tagAnimeObjectHeader{
	int          nCount;
	ObjectDummy *lpObjList;
} AnimeObjectHeader;

/**************************************************************************/
/* 中間データ */

/* 頂点データ */
typedef struct tagVertex{
	int x;
	int y;
} Vertex;

/* カラーデータ */
typedef struct tagColor{
	int r;
	int g;
	int b;
	int a;
} Color;

/* UVデータ */
typedef struct tagUV{
	float u;
	float v;
} UV;

/* 実データ */
typedef struct tagData{
	struct tagData *lpNext;	/* 次のDataへのポインタ */

	unsigned int unID;	/* 識別用ID */

	unsigned int unType;

	char szName[MAX_NAME];	/* キー名称など */

	unsigned int  unVertexCount;	/* 頂点数 */
	Vertex       *lpvtVertex;		/* 頂点への配列 */
	int           nVertexFlag;		/* 出力制御フラグ */
	int           nSizeFlag;

	Color *lpclrColor;		/* カラーデータへのポインタ。数は頂点数と同じ */
	int    nColorFlag;		/* 出力制御フラグ */

	int nView;		/* 表示・非表示 */
	int nViewFlag;	/* 出力制御フラグ */

	float fMagni;		/* 拡大・縮小値 */
	int   nMagniFlag;	/* 出力制御フラグ */

	int nBlend;			/* ブレンド属性 */
	int nBlendFlag;		/* 出力制御フラグ */

	char szPictureName[MAX_NAME];		/* 参照ピクチャー名 */
	int  nPictureFlag;	/* 出力制御フラグ */

	char szTRIName[MAX_NAME];	/* TRI名 */

	UV  *lpUV;			/* ＵＶ。数は頂点数と同じ */
	int  nUVFlag;		/* 出力制御フラグ */

	int nRotate;		/* 回転 */
	int nRotateFlag;	/* 出力制御フラグ */

	int nRotateMode;		/* 回転方向フラグ */
	int nRotateModeFlag;	/* 出力制御フラグ */

	Vertex vtSpinCenter;	/* 回転中心 */
	int    nSpinCenterFlag;	/* 出力制御フラグ */

	int   nOffset;		/* 実機側用 */
	void *lpRefData;	/* 参照用 */
} Data;

/* オブジェクトデータ */
typedef struct tagObject{
	struct tagObject *lpNext;		/* 次データへのリンク */
	struct tagObject *lpChild;	/* 子へのリンク */

	int nObjectID;	/* 実機側用 */

	unsigned int unType;			/* オブジェクトタイプ empty, sprite など */
	char         szName[MAX_NAME];	/* オブジェクト名 */

	unsigned int  unDataCount;	/* データ数 */
	void         *lpData;		/* データへのポインタ */
} Object;

/* キーデータ */
typedef struct tagKey{
	int           nFrame;	/* フレーム数 */
	unsigned int  unType;	/* キータイプ：morfかsetか */
	Data         *lpData;	/* Dataへのポインタ */
} Key;

/* アクションで使用する、ダミーオブジェクト */
typedef struct tagActObj{
	Object       *lpObject;	/* オブジェクトへのポインタ */
	unsigned int  unKeyCount;	/* キーの数 */
	Key          *lpKey;		/* キーへのポインタ */
} ActObj;

/* アクションデータ */
typedef struct tagAction{
	char szName[MAX_NAME];	/* 名称 */

	int nFrameMin;	/* フレーム最小値 */
	int nFrameMax;	/* フレーム最大値 */

	unsigned int  unActObjCount;	/* アクションオブジェクト数 */
	ActObj       *lpActObj;		/* アクションオブジェクトへのポインタ */
} Action;

/* ピクチャーデータ */
typedef struct tagPicture{
	char szName[MAX_NAME];	/* ファイル名 */
	char szTRIName[MAX_NAME];	/* TRI名 */
	char szPath[MAX_TEXT];		/* ピクチャーファイルへのフルパス */

	int nWidth;		/* ピクチャーのサイズ */
	int nHeight;

	unsigned int  unFileSize;	/* ピクチャーファイルのサイズ */
	void         *lpFile;		/* ピクチャーファイル */

	int nYear;		/* 取り込んだピクチャーファイルの作成日時 */
	int nMonth;		/* 不明・もしくは取得できなかった場合は All 0 */
	int nDay;
	int nHour;
	int nMin;
	int nSec;
} Picture;

/* LEFから抽出した2Dデータのヘッダー */
typedef struct tagSceneData{
	unsigned int unVersion;		/* LEFのバージョン情報 */

	int nFieldWidth;	/* LEF上で設定されていた画面サイズ */
	int nFieldHeight;

	unsigned int  unObjectCount;
	Object       *lpObject;

	unsigned int  unActionCount;
	Action       *lpAction;

	unsigned int  unPictureCount;
	Picture      *lpPicture;
} SceneData;


/* コンバート情報 */
typedef struct tagConvertInfo{
	char      szInputPath[MAX_TEXT];
	char      szOutputPath[MAX_TEXT];
	SceneData scnData;
} ConvertInfo;



#endif
