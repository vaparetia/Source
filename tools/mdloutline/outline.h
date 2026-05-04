/*******************************************************************************
 * outline - outline.h
 * アウトラインヘッダー
 * 2002/03/12 S.Yamashita
 * $Id: outline.h,v 1.2 2002/03/19 06:18:09 usr03692 Exp $
 */

#ifndef __INC_OUTLINE__
#define __INC_OUTLINE__

#ifdef __cplusplus
extern "C" {		/* Assume C declarations for C++ */
#endif				/* __cplusplus */

/*******************************************************************************
 * include
 */

#ifdef _WINDOWS
	#include <SFDWIN.h>
#else
	#include <SFDLINUX.h>
#endif

/*******************************************************************************
 * definitions and typedefs and structures
 */

#define OL_STRING_MAX   (256)	/* 最大文字列長さ */
#define OL_POLYNODE_MAX  (50)	/* 最大ポリゴンノード数 */

/*
 * フラグ
 */
#define OL_MFLAG_MASK        (0xffff0000)	/* 動作フラグマスク */
#define OL_MFLAG_LOGOUT      (0x00010000)	/* ログファイルへ出力する */
#define OL_MFLAG_STDOUT      (0x00020000)	/* 標準出力へ出力する */
#define OL_MFLAG_OLVINFO     (0x00040000)	/* アウトライン頂点情報もテキスト出力する */
#define OL_MFLAG_DELCONVNAME (0x00100000)	/* 変換定義の削除を行う */
#define OL_MFLAG_NAMEPREFIX  (0x00200000)	/* 名前比較 プリフィックスモード */
#define OL_MFLAG_CALCNORMAL  (0x00400000)	/* ポリゴンの法線を計算する */
#define OL_MFLAG_IGNOREFACES (0x00800000)	/* 表裏を無視する */
#define OL_MFLAG_ONEOBJECT   (0x01000000)	/* １オブジェクトモード */
#define OL_MFLAG_JOINTEDGE   (0x02000000)	/* エッジの連結処理を行う */

#define OLO_FLAG_COLORMASK  (0x00000003)	/* カラーマスク */
#define OLO_FLAG_COLOR1     (0x00000000)	/* カラー１ */
#define OLO_FLAG_COLOR2     (0x00000001)	/* カラー２ */
#define OLO_FLAG_COLOR3     (0x00000002)	/* カラー３ */
#define OLO_FLAG_COLOR4     (0x00000003)	/* カラー４ */

#define OLV_MFLAG_DONE      (0x00010000)	/* 処理済 */
#define OLV_FLAG_DRAWEDGE   (0x00000001)	/* エッジを描画 */

/*
 * アウトライン頂点
 */
typedef struct tagOL_VERTEX
{
	int    nFlag;	/* フラグ */
	float  fx;		/* Ｘ座標 */
	float  fy;		/* Ｙ座標 */
	float  fz;		/* Ｚ座標 */
}
OL_VERTEX, * POL_VERTEX, * LPOL_VERTEX;

/*
 * アウトラインオブジェクト
 */
typedef struct tagOL_OBJECT
{
	int          nID;			/* オブジェクトＩＤ */
	int          nFlag;			/* フラグ */
	unsigned int unMemSize;		/* 頂点メモリサイズ */
	unsigned int unVertCount;	/* 頂点数 */
	LPOL_VERTEX  lpol_vertex;	/* 頂点配列 */
}
OL_OBJECT, * POL_OBJECT, * LPOL_OBJECT;

/*
 * アウトライン
 */
typedef struct tagOUTLINE
{
	int          nFlagA;				/* フラグ */
	FILE*        fpLog;					/* ログファイルポインタ */
	unsigned int unBlockSize;			/* 頂点メモリ確保ブロックサイズ(1～) */
	double       dAMargin;				/* ２つの面の法線の角度判定マージン(0～1)  0:０度の時のみ描画しない  1:全て描画しない */
	double       dPMargin;				/* ２つのエッジの重なり誤差判定マージン(0～) */
	unsigned int unSkipNameCount;		/* 処理しない名前数 */
	char**       lppszSkipName;			/* 処理しない名前配列 */
	unsigned int unColorNameCount[3];	/* 色グループ名前数の配列 */
	char**       lppszColorName[3];		/* 色グループ名前配列の配列 */
	unsigned int unObjCount;			/* オブジェクト数 */
	LPOL_OBJECT lpol_object;			/* オブジェクト配列 */
}
OUTLINE, * POUTLINE, * LPOUTLINE;

/*******************************************************************************
 * functions
 */

int CreateOutline(LPOUTLINE lpoutline, HP3DMODEL hP3DModel, int nFlagA, FILE* fpLog, unsigned int unBlockSize, double dAMargin, double dPMargin, unsigned int unSkipNameCount, char** lppszSkipName, unsigned int* lpunColorNameCount, char*** lpppszColorName, int nFlag);
int DestroyOutline(LPOUTLINE lpoutline, int nFlag);
int ScaleOutline(LPOUTLINE lpoutline, float fScale, int nFlag);
int OutputOutline(LPOUTLINE lpoutline, char* lpszFName, int nFlag);

/*******************************************************************************
 */

#ifdef __cplusplus
}					/* End of extern "C" { */
#endif				/* __cplusplus */

#endif	/* __INC_OUTLINE__ */
