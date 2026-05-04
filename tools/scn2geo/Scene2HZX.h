/******************************************************************************
 * System	: Scene To HZX File Convert exe exe
 * Computer : DOS/V
 * OS		: Microsoft Windows
 * Compiler : Microsoft Visual C++
 * Module	: Win32 Console Application
 */

#ifndef __INC_SCENE2HZX__
#define __INC_SCENE2HZX__

#include "HZXStruct2.h"

/******************************************************************************
 * defined
 */
#define GROUP_CLEARING_NUMBER_CONVERT

/******************************************************************************
 * typedef and struct
 */

typedef struct tagSCENE2HZXINFO			/* 入力ファイル名と出力ファイル名、フラグを持つ */
{
	char szInputFileName[_MAX_TEXT];	/* 入力ファイル名 */
	char szOutputFileName[_MAX_TEXT];	/* 出力ファイル名 */
	char szOutputDirName[ _MAX_TEXT ] ;	/* 出力ディレクトリ */
	char nOldHZXSceneFlag;			/* 旧式のフラグを指定 0 : 指定なし 1 : 指定有り */
	char DebugMode ;
} SCENE2HZXINFO, *PSCENE2HZXINFO, *LPSCENE2HZXINFO;

/******************************************************************************
 * HZX オブジェクトを抽出するための構造体
 */
typedef struct tagDOUBLEXYZ
{
	double x;
	double y;
	double z;
} DOUBLEXYZ, *PDOUBLEXYZ, *LPDOUBLEXYZ;

typedef struct tagINTXYZ
{
	int x;
	int y;
	int z;
} INTXYZ, *PINTXYZ, *LPINTXYZ;

typedef struct tagSEGMENTOBJECT		/* Hazard, Floor オブジェクト */
{
	int       nVertexCount;		/* 頂点数 */
	DOUBLEXYZ dxyzVertex[4];	/* 各頂点 */
	int       nAttribute;		/* アトリビュート */
} SEGMENTOBJECT, *PSEGMENTOBJECT, *LPSEGMENTOBJECT;

typedef struct tagTRAPOBJECT	/* Trap オブジェクト */
{
	DOUBLEXYZ dxyzBound[2];			/* トラップバウンディング 0 : 最小 1 : 最大 */
	char      szName[_MAX_TEXT];	/* トラップ名 */
	short     sAttribute;			/* アトリビュート */
} TRAPOBJECT, *PTRAPOBJECT, *LPTRAPOBJECT;

typedef struct tagCAMERAOBJECT		/* Camera オブジェクト */
{
	DOUBLEXYZ dxyzBound[2];		/* カメラバウンディング 0 : 最小 1 : 最大 */
	DOUBLEXYZ dxyzTarget;		/* 注視点 */
	DOUBLEXYZ dxyzCamera;		/* 視点 */
	DOUBLEXYZ dxyzRightTarget;	/* 右のぞき込み注視点 */
	DOUBLEXYZ dxyzRightCamera;	/* 右のぞき込み視点 */
	DOUBLEXYZ dxyzLeftTarget;	/* 左のぞき込み注視点 */
	DOUBLEXYZ dxyzLeftCamera;	/* 左のぞき込み視点 */
	int       nPadData[2];		/* ０：オプション発生方向 */
								/* １：オプションフラグ */
} CAMERAOBJECT, *PCAMERAOBJECT, *LPCAMERAOBJECT;

typedef struct tagZONEOBJECT	/* Zone オブジェクト */
{
	int       nVertexCount;		/* ポリゴンの頂点数 */
	DOUBLEXYZ dxyzVertex[4];	/* ポリゴンの各頂点 */
	int       nIncremental;		/* 逐次番号 */
	int       nFlag;			/* フラグ */
} ZONEOBJECT, *PZONEOBJECT, *LPZONEOBJECT;

typedef struct tagZONETOZONEOBJECT	/* ゾーン間フラグ設定用オブジェクト */
{
	int       nRouteIndex;	/* ゾーン間フラグが設定されているルートのインデックス */
	int       nFlag;
} ZONETOZONEOBJECT, *PZONETOZONEOBJECT, *LPZONETOZONEOBJECT;

typedef struct tagROUTECOORDOBJECT	/* Route 頂点オブジェクト */
{
	DOUBLEXYZ dxyzVertex;		/* ルート頂点 */
	int       nIncremental;		/* 逐次番号 */
	int       nFlag;			/* フラグ */
} ROUTECOORDOBJECT, *PROUTECOORDOBJECT, *LPROUTECOORDOBJECT;

typedef struct tagROUTEPOLYGONOBJECT	/* Route ポリゴンオブジェクト */
										/* ポリゴンはゾーンのつながりを示す */
{
	LPROUTECOORDOBJECT lpRouteCoord[2];		/* ルート頂点へのポインタ */
	int                nZoneToZoneFlag;		/* ゾーン間属性フラグ */
} ROUTEPOLYGONOBJECT, *PROUTEPOLYGONOBJECT, *LPROUTEPOLYGONOBJECT;

typedef struct tagROUTEOBJECT	/* Route オブジェクト */
{
	int                  nVertexCount;		/* ルート頂点数 */
	LPROUTECOORDOBJECT   lpRouteCoord;		/* ルート頂点配列 */
	int                  nPolygonCount;		/* ルートポリゴン数 */
	LPROUTEPOLYGONOBJECT lpRoutePolygon;	/* ルートポリゴン配列 */
} ROUTEOBJECT, *PROUTEOBJECT, *LPROUTEOBJECT;

typedef struct tagCHARACTERCOORDOBJECT		/* Character ポイントオブジェクト */
{
	DOUBLEXYZ dxyzVertex;		/* 座標 */
	DOUBLEXYZ dxyzTarget;		/* 視点 */
	int       nAction;			/* アクション番号 */
	int       nTime;			/* 継続番号 */
	int       nDirection;		/* 方向 */
	int       nPad;				/* リザーブ */
	int       nFlag;			/* フラグ */
} CHARACTERCOORDOBJECT, *PCHARACTERCOORDOBJECT, *LPCHARACTERCOORDOBJECT;

typedef struct tagCHARACTERROUTEOBJECT		/* Character ルートオブジェクト */
{
	int                    nCoordCount;			/* ポイント数 */
	int                    nPad;				/* 開始位置？ */
	LPCHARACTERCOORDOBJECT lpCharacterCoord;	/* ポイント */

	int                    nFlag;				/* フラグ */
} CHARACTERROUTEOBJECT, *PCHARACTERROUTEOBJECT, *LPCHARACTERROUTEOBJECT;

typedef struct tagCLEARINGCOORDOBJECT		/* Clearing ポイントオブジェクト */
{
	DOUBLEXYZ dxyzVertex;		/* 座標 */
	DOUBLEXYZ dxyzTarget;		/* 注視点 */
	int       nAction;			/* アクション */
	int       nTime;			/* 時間 */
	int       nDirection;		/* 方向 */
	int       nCondition;		/* 条件 */
} CLEARINGCOORDOBJECT, *PCLEARINGCOORDOBJECT, *LPCLEARINGCOORDOBJECT;

typedef struct tagCLEARINGROUTEOBJECT		/* Clearing ルートデータ */
{
	int                   nPoints;		/* ポイント数 */
	int                   nPad;			/* リザーブ */
	LPCLEARINGCOORDOBJECT lpClearingCoord;	/* ポイント */
} CLEARINGROUTEOBJECT, *PCLEARINGROUTEOBJECT, *LPCLEARINGROUTEOBJECT;

typedef struct tagCLEARINGAREAOBJECT		/* Clearing エリアデータ */
{
	DOUBLEXYZ             dxyzBound[2];			/* Clearing バウンディング 0 : 最小 1 : 最大 */
	char                  szName[_MAX_TEXT];	/* 名前 */
	int                   nTime;				/* 突入までの時間 */
	int                   nRouteCount;			/* クリアリングルート数 */
	LPCLEARINGROUTEOBJECT lpClearingRoute;		/* ルート */
} CLEARINGAREAOBJECT, *PCLEARINGAREAOBJECT, *LPCLEARINGAREAOBJECT;

typedef struct tagBLOCKOBJECT	/* Block オブジェクト */
{
	INTXYZ nxyzCenter;	/* ブロック中心座標 */
	int    nExtension;

	int nHazardCount;		/* 壁数 */
	int nFloorCount;		/* 床数 */
	int nTrapCount;			/* トラップ数 */
	int nBlockNumber;		/* ブロック番号 */

	LPSEGMENTOBJECT   lpHazard;		/* 壁 */
	LPSEGMENTOBJECT   lpFloor;		/* 床 */
	LPTRAPOBJECT      lpTrap;		/* トラップ */
} BLOCKOBJECT, *PBLOCKOBJECT, *LPBLOCKOBJECT;

typedef struct tagGROUPOBJECT		/* Group オブジェクト */
{
	INTXYZ nxyzBoundMin;		/* バウンディング */
	INTXYZ nxyzSize;			/* 1 ブロックのサイズ */

	INTXYZ nxyzDivide;			/* 分割数 シーンから得た値 */
	INTXYZ nxyzDivideReCalc;	/* 分割数 バウンディングから再計算したもの */

	int nCameraCount;		/* カメラ数 */
	int nZoneCount;			/* ゾーン数 */
	int nBlockCount;		/* ブロック数 */

	LPCAMERAOBJECT    lpCamera;		/* カメラ */
	LPZONEOBJECT      lpZone;		/* ゾーン */
	LPBLOCKOBJECT     lpBlock;		/* ブロック */
	

	/* 以下のデータは一時的なデータで、ブロック生成後分けられるため移動する */
	int nHazardCount;		/* 壁数 */
	int nFloorCount;		/* 床数 */
	int nTrapCount;			/* トラップ数 */

	LPSEGMENTOBJECT   lpHazard;		/* 壁 */
	LPSEGMENTOBJECT   lpFloor;		/* 床 */
	LPTRAPOBJECT      lpTrap;		/* トラップ */
} GROUPOBJECT, *PGROUPOBJECT, *LPGROUPOBJECT;

typedef struct tagSCENE2HZXHEADER		/* HZX 用オブジェクトヘッダー */
{
	int nVersion;		/* バージョン３ */

	int           nGroupCount;		/* グループ数 */
	LPGROUPOBJECT lpGroup;	/* グループ */

	int nCharacterRouteCount;	/* キャラクタルート数 */
	int arynVRCharacterOffset[HZX_VR_CHARACTER_GROUP_MAX];
	int nClearingAreaCount;		/* クリアリングエリア数 */
	int arynVRClearingOffset[HZX_VR_CLEARING_GROUP_MAX];
//	int nZoneToZoneCount;		/* ゾーン間フラグが設定されているルート数 */

	LPROUTEOBJECT          lpRoute;				/* ルート */
//	LPZONETOZONEOBJECT     lpZoneToZone;		/* ゾーン間フラグ */
	LPCHARACTERROUTEOBJECT lpCharacterRoute;	/* キャラクタルート */
	LPCLEARINGAREAOBJECT   lpClearingArea;		/* クリアリング */
} SCENE2HZXHEADER, *PSCENE2HZXHEADER, *LPSCENE2HZXHEADER;


/******************************************************************************
ゾーンとルートの対応について

ゾーンとルート頂点は互いに識別用のメンバ変数 nIncremental を持っています。
ゾーンと、それに対応するルート頂点は同じ値の nIncremental を持つことになります。

＜ それぞれのデータについて ＞
ZONEOBJECT
	ゾーンデータを保持しています。
	ゾーンポリゴンを構成する頂点配列と、識別用の nIncremental とフラグを持ちます。

ROUTEOBJECT
	ルートデータを保持しています。これは２つのデータで構成されています。

	ROUTECOORDOBJECT
		ルート頂点のデータ配列です。
		ゾーンに対するルート頂点はこの中にあります。

	ROUTEPOLYGONOBJECT
		ルートの接続に関するデータ配列です。
		２つのルート頂点を保持しており、これがルートのつながりを示します。
		ポインタは ROUTECOORDOBJECT へのポインタです。
 */


/******************************************************************************
 * globals
 */

/* AnalyzeCommand.c */
extern int AnalyzeCommand(int argc, char **argv, LPSCENE2HZXINFO lpScene2HZXInfo);

/* GetHZXObject.c */
extern int GetHZXObject(LPSCENE2HZXINFO lpscene2HZXInfo, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetGroupObject.c */
extern int GetGroupObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetHazardObject.c */
extern int GetHazardObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetFloorObject.c */
extern int GetFloorObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetTrapObject.c */
extern int GetTrapObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetCameraObject.c */
extern int GetCameraObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetZoneObject.c */
extern int GetZoneObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);
extern int GetOldZoneObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetRouteObject.c */	
extern int GetRouteObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);
extern int GetOldRouteObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetCharacterObject.c */	
extern int GetCharacterObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* GetCharacterObject.c */	
extern int GetClearingObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader);

/* MakeHZXBlockObject.c */	
extern int MakeHZXBlockObject(LPSCENE2HZXHEADER lpScene2HZXHeader);

/* DestroyScene2HZXObject.c */
extern void DestroyScene2HZXObject(LPSCENE2HZXHEADER lpScene2HZXHeader);

/* OESMathPublic.c */
/* OESP3DModelAnalysis.cpp */
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

Dllexport int    WINAPI OESDeleteOverVertexOnLine(LPP3DXYZ lpP3DXYZVertex, int nVertexCount, double dErrorRenge);
Dllexport BOOL   WINAPI OESP3DPolygonLineVertex(int** lppnIndex, int* lpnIndexCount, HP3DOBJECT hP3DObject);

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

/*****************************************************************************/

#endif // __INC_SCENE2HZX__
