/*******************************************************************************
 * mdloutline - mdloutline.h
 * モデルアウトラインヘッダー
 * 2002/03/12 S.Yamashita
 * $Id: mdloutline.h,v 1.2 2002/03/19 06:18:09 usr03692 Exp $
 */

#ifndef __INC_MDLOUTLINE__
#define __INC_MDLOUTLINE__

#ifdef __cplusplus
extern "C" {		/* Assume C declarations for C++ */
#endif				/* __cplusplus */

/******************************************************************************
 * include
 */

#include "outline.h"

/******************************************************************************
 * definitions and typedefs and structures
 */

#define DEFAULT_SCALE     (100.0)
#define DEFAULT_BLOCKSIZE (1000)
#define DEFAULT_AMARGIN   (0.0001)
#define DEFAULT_PMARGIN   (0.01)

/*
 * 動作フラグ
 */
#define MFLAG_LOGOUT      (0x00010000)
#define MFLAG_STDOUT      (0x00020000)
#define MFLAG_OLVINFO     (0x00040000)
#define MFLAG_DELCONVNAME (0x00100000)
#define MFLAG_NAMEPREFIX  (0x00200000)
#define MFLAG_CALCNORMAL  (0x00400000)
#define MFLAG_IGNOREFACES (0x00800000)
#define MFLAG_ONEOBJECT   (0x01000000)
#define MFLAG_JOINTEDGE   (0x02000000)

/*
 * アプリケーションデータ
 */
typedef struct tagAPPDATA
{
	char         szMdlFName[256];		/* 入力ファイル名 */
	char         szRolFName[256];		/* 出力ファイル名 */
	char         szLogFName[256];		/* ログファイル名 */

	float        fScale;				/* スケール */
	int          nFlag;					/* フラグ */
	FILE*        fpLog;					/* ログファイルポインタ */
	unsigned int unBlockSize;			/* メモリ確保ブロックサイズ */
	double       dAMargin;				/* 角度マージン */
	double       dPMargin;				/* 位置マージン */
	unsigned int unSkipNameCount;		/* 処理しない名前数 */
	char**       lppszSkipName;			/* 処理しない名前配列 */
	unsigned int unColorNameCount[3];	/* 色グループ名前数の配列 */
	char**       lppszColorName[3];		/* 色グループ名前配列の配列 */
	OUTLINE      outline;				/* アウトライン */
}
APPDATA, * PAPPDATA, * LPAPPDATA;

/*******************************************************************************
 */

#ifdef __cplusplus
}					/* End of extern "C" { */
#endif				/* __cplusplus */

#endif	/* __INC_MDLOUTLINE__ */
