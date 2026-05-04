/******************************************************************************
 * System	: Virtual Reality Studio KONAMI metalgear DE2 file struct
 * Computer : DOS/V
 * OS		: Microsoft Windows
 * Compiler : Microsoft Visual C++
 * Module	: kernel
 */

#ifndef _INC_DE2STRUCT
#define _INC_DE2STRUCT

/******************************************************************************
 * definitions and typedefs and structures
 */

#define DE2_EFFECTINFORMATION "KONAMI DE2 Effect information"

/* エフェクト番号 */
enum { DE2_EFFECTC4EXPLOSION = 1,			/* 爆発 */
	   DE2_EFFECTBLOOD2D,					/* 血煙 */
	   DE2_EFFECTBREATH,					/* 息 */
	   DE2_EFFECTSHIELDSMOKE,				/* 盾煙 */
	   DE2_EFFECTSPARK1,					/* 火花 */
	   DE2_EFFECTPIYORI,					/* ぴよぴよ（ノーマル） */
	   DE2_EFFECTPIYORIANES,				/* ぴよぴよ（麻酔） */
	   DE2_EFFECTZZZ,						/* 居眠り */
	   DE2_EFFECTCAMERABLOOD,				/* カメラ血飛沫 */
	   DE2_EFFECTBLOODSPREAD,				/* 床に広がる血 */
	   DE2_EFFECTWALLBLOOD,					/* 壁に付着する血 */
	   DE2_EFFECTCIRCLELIGHT,				/* 敵兵の銃のライト */
	   DE2_EFFECTFADEINOUT,					/* フェードイン・アウト */
	   DE2_EFFECTFADEINOUTDEMO,				/* フェードイン・アウトデモ */
	   DE2_EFFECTBLOODDEMO,					/* 任意血 */
	   DE2_EFFECTBLOODDROPS,				/* 落ちる血 */
	   DE2_EFFECTPLASMALINEDEMO,			/* プラズマライン */
	   DE2_EFFECTBODYPLASMADOTDEMO,			/* 光学迷彩壊れスパーク */
	   DE2_EFFECTENEEQUIP,					/* 敵兵装備品 */
	   DE2_EFFECTBODYSHADOW,				/* 遮り影 */
	   DE2_EFFECTFLUSH,						/* ＣＬＵＴフラッシュ */
	   DE2_EFFECTRAINCAMERADEMO,			/* カメラ前水滴 */
	   DE2_EFFECTMESSAGE,					/* メッセージ */
	   DE2_EFFECTCARTRIDGECONTROL,			/* 銃のカートリッジ */
	   DE2_EFFECTNEARFOCUS,					/* 近景ぼかし */
	   DE2_EFFECTFARFOCUS,					/* 遠景ぼかし */
	   DE2_EFFECTSPRITEFOGWORLDDEMO,		/* テクセル流しワールド */
	   DE2_EFFECTSPRITEFOGPERSDEMO,			/* テクセル流しワールド透視変換 */
	   DE2_EFFECTSPRITEFOGONCAMERADEMO,		/* テクセル流しカメラ */
	   DE2_EFFECTSPLASHDEMO,				/* 風紋水飛沫 */
	   DE2_EFFECTSPLASHPARTSDEMO,			/* 単発水飛沫 */
	   DE2_EFFECTSPLASHRIPPLEDEMO,			/* 水飛沫複数呼び */
	   DE2_EFFECTOPTICALCAMOBREAKDEMO,		/* 光学迷彩壊れ */
	   DE2_EFFECTBODYSHADOWVOLUMEDEMO,		/* ボリューミックシャドーライン */
	   DE2_EFFECTGBSHANDDEMO,				/* ゴルルコ フィンガーサイン */
	   DE2_EFFECTPUTSTANIMEOBJECTCALL,		/* 頂点ストリームアニメーション */
	   DE2_EFFECTE3FACEANIMATION,			/* Ｅ３限定フェイスアニメ制御 */
	   DE2_EFFECTCONTRASTDEMO,				/* コントラスト＆ネガポジ */
	   DE2_EFFECTPLASMAPOLYDEMO,			/* プラズマポリゴン */
	   DE2_EFFECTTHUNDERDEMO,				/* 稲光 */
	   DE2_EFFECTFOGSETDEMO,				/* フォグセット */
	   DE2_EFFECTSPLASHMOTIONDEMO,			/* モーション連動水飛沫 */
	   DE2_EFFECTROPEMODEL3,				/* ロープモデル３ */
	   DE2_EFFECTOOZEBLOODDEMO,				/* 敵兵滲み血 */
	   DE2_EFFECTWAVINGCLOTHMODELW,			/* オブジェに付く布モデル */
	   DE2_EFFECTROPEMODEL2,				/* ロープモデル２ */
	   DE2_EFFECTEVMMMORGA,					/* オルガの揺れ物 */
	   DE2_EFFECTSMOKEBLUR,					/* 煙型ブラー */
	   DE2_EFFECTBODYSPLASH,				/* 体からの水はね */
	   DE2_EFFECTRAINCOAT,					/* レインコートの揺れ */
	   DE2_EFFECTSHADOW,					/* 足影 */
	   DE2_EFFECTDROPSHADOW,				/* キャラ影 */
	   DE2_EFFECTATTACHMENT3,				/* 装備品Ｃ */
	   DE2_EFFECTINTERPOLYDEMO,				/* ポリゴン頂点補完残像 */
	   DE2_EFFECTPUTATTACHMENTS,			/* ゆれる付属品 */
	   DE2_EFFECTFOOTSPLASH,				/* 足元水飛沫 */
	   DE2_EFFECTSAASMOKE,					/* ＳＡＡ銃口煙 */
	   DE2_EFFECTVANIMEBULLER,				/* 頂点アニメブラー */
	   DE2_EFFECTDEMOBULLET,				/* 弾丸 */
	   DE2_EFFECTSPARKDEMO,					/* 跳弾 */
	   DE2_EFFECTGBSFACEDEMO,				/* ゴルルゴ兵目パチ */
	   };










typedef struct tagDE2EFFECTMESSAGINFO	/* メッセージ情報 */
{
	char szName[64];		/* エフェクト名 */
	int  nLength;			/* パラメータ数 */
	char szParam1[32];		/* パラメータ1 文字列 */
	char szParam2[32];		/* パラメータ2 文字列 */
	char szParam3[32];		/* パラメータ3 文字列 */
	char szParam4[32];		/* パラメータ4 文字列 */
	char szParam5[32];		/* パラメータ5 文字列 */
	char szParam6[32];		/* パラメータ6 文字列 */
	char szParam7[32];		/* パラメータ7 文字列 */
	char szParam8[32];		/* パラメータ8 文字列 */
	char szParam9[32];		/* パラメータ9 文字列 */
	char szParam10[32];		/* パラメータ10 文字列 */
	char szParam11[32];		/* パラメータ11 文字列 */
	char szParam12[32];		/* パラメータ12 文字列 */
	char szParam13[32];		/* パラメータ13 文字列 */
	char szParam14[32];		/* パラメータ14 文字列 */
	char szParam15[32];		/* パラメータ15 文字列 */
	char szParam16[32];		/* パラメータ16 文字列 */
} DE2EFFECTMESSAGEINFO, *PDE2EFFECTMESSAGEINFO, *LPDE2EFFECTMESSAGEINFO;

typedef struct tagDE2EFFECTCARTRIDGECONTROLINFO		/* 銃のカートリッジ情報 */
{
	char szName[64];	/* エフェクト名 */
	int nScene1;		/* リンク人体シーン  0 ～ */
	int nScene2;		/* リンク武器シーン  0 ～ */
	int nPat;			/* 人体と武器の種類 */
	int nControlNum;	/* カートリッジの挙動制御 */
	int nCount;			/* 表示フレーム数 */
} DE2EFFECTCARTRIDGECONTROLINFO, *PDE2EFFECTCARTRIDGECONTROLINFO, *LPDE2EFFECTCARTRIDGECONTROLINFO;

typedef struct tagDE2EFFECTNEARFOCUSINFO	/* 近景ぼかし情報 */
{
	char szName[64];	/* エフェクト名 */
	int nMaxPlane;		/* 最大描画枚数 */
	int nNear;			/* ぼかし最大距離 */
	int nFar;			/* ぼかし最小距離 */
	int nCount;			/* 表示フレーム数 */
} DE2EFFECTNEARFOCUSINFO, *PDE2EFFECTNEARFOCUSINFO, *LPDE2EFFECTNEARFOCUSINFO;

typedef struct tagDE2EFFECTFARFOCUSINFO	/* 遠景ぼかし情報 */
{
	char szName[64];	/* エフェクト名 */
	int nMaxPlane;		/* 最大描画枚数 */
	int nNear;			/* ぼかし最小距離 */
	int nFar;			/* ぼかし最大距離 */
	int nCount;			/* 表示フレーム数 */
} DE2EFFECTFARFOCUSINFO, *PDE2EFFECTFARFOCUSINFO, *LPDE2EFFECTFARFOCUSINFO;

typedef struct tagDE2EFFECTSPRITEFOGWORLDDEMOINFO	/* テクセル流しワールド情報 */
{
	char    szTexName[64];	/* テクスチュア名 */
	int     nSize;			/* スプライトサイズ */
	int     nScene1;		/* 立方体の範囲1 リンクシーン  0 ～ */
	int     nNode1;			/* 立方体の範囲1 リンクノード  0 ～ */
	int     nScene2;		/* 立方体の範囲2 リンクシーン  0 ～ */
	int     nNode2;			/* 立方体の範囲2 リンクノード  0 ～ */
	FVECTOR fvectBound1;	/* 立方体の範囲1 */
	FVECTOR fvectBound2;	/* 立方体の範囲2 */
	FVECTOR fvectDir;		/* 進行方向ベクトル */
	FVECTOR fvectColor;		/* 最大到達RGBα */
	int     nLife;			/* 消滅までのフレーム数 */
	int     nMode;			/* 0: 加算半透明  1: 減算半透明  2:平均半透明 */
} DE2EFFECTSPRITEFOGWORLDDEMOINFO, *PDE2EFFECTSPRITEFOGWORLDDEMOINFO, *LPDE2EFFECTSPRITEFOGWORLDDEMOINFO;

typedef struct tagDE2EFFECTSPRITEFOGPERSDEMOINFO	/* テクセル流しワールド透視変換情報 */
{
	char    szTexName[64];	/* テクスチュア名 */
	int     nSize;			/* スプライトサイズ */
	int     nScene1;		/* 立方体の範囲1 リンクシーン  0 ～ */
	int     nNode1;			/* 立方体の範囲1 リンクノード  0 ～ */
	int     nScene2;		/* 立方体の範囲2 リンクシーン  0 ～ */
	int     nNode2;			/* 立方体の範囲2 リンクノード  0 ～ */
	FVECTOR fvectBound1;	/* 立方体の範囲1 */
	FVECTOR fvectBound2;	/* 立方体の範囲2 */
	FVECTOR fvectDir;		/* 進行方向ベクトル */
	FVECTOR fvectColor;		/* 最大到達RGBα */
	int     nLife;			/* 消滅までのフレーム数 */
	int     nMode;			/* 0: 加算半透明  1: 減算半透明  2:平均半透明 */
} DE2EFFECTSPRITEFOGPERSDEMOINFO, *PDE2EFFECTSPRITEFOGPERSDEMOINFO, *LPDE2EFFECTSPRITEFOGPERSDEMOINFO;

typedef struct tagDE2EFFECTSPRITEFOGONCAMERADEMOINFO	/* テクセル流しカメラ情報 */
{
	char    szTexName[64];	/* テクスチュア名 */
	int     nSize;			/* スプライトサイズ */
	FVECTOR fvectDir;		/* 進行方向ベクトル */
	FVECTOR fvectColor;		/* 最大到達RGBα */
	int     nLife;			/* 消滅までのフレーム数 */
	int     nMode;			/* 0: 加算半透明  1: 減算半透明  2:平均半透明 */
} DE2EFFECTSPRITEFOGONCAMERADEMOINFO, *PDE2EFFECTSPRITEFOGONCAMERADEMOINFO, *LPDE2EFFECTSPRITEFOGONCAMERADEMOINFO;

typedef struct tagDE2EFFECTGBSHANDDEMOINFO	/* ゴルルコ フィンガーサイン情報 */
{
	char szName[64];	/* エフェクト名 */
	int  nScene;		/* リンクシーン  0 ～ */
	int  nCount;		/* 表示時間 */
} DE2EFFECTGBSHANDDEMOINFO, *PDE2EFFECTGBSHANDDEMOINFO, *LPDE2EFFECTGBSHANDDEMOINFO;

typedef struct tagDE2EFFECTPUTSTANIMEOBJECTCALLINFO		/* 頂点ストリームアニメーション情報 */
{
	char    szName[64];		/* エフェクト名 */
	char    szModel[32];	/* モデル名 */
	FVECTOR fvectPos;		/* 位置 */
	FVECTOR fvectScale;		/* サイズ */
	FVECTOR fvectBound_U;	/* モデルのバウンド最大点 */
	FVECTOR fvectBound_L;	/* モデルのバウンド最小点 */
	FVECTOR fvectRot;		/* 回転 */
	int     nMode;			/* 再生モード */
	int     nCount;			/* 表示フレーム数 */
	int     nScene;			/* リンクシーン  -1: 無効  0 ～ */
	int     nNode;			/* リンクノード  0 ～ */
} DE2EFFECTPUTSTANIMEOBJECTCALLINFO, *PDE2EFFECTPUTSTANIMEOBJECTCALLINFO, *LPDE2EFFECTPUTSTANIMEOBJECTCALLINFO;

typedef struct tagDE2EFFECTE3FACEANIMATIONINFO	/* Ｅ３限定フェイスアニメ制御情報 */
{
	char szName[64];	/* エフェクト名 */
	int  nScene;		/* リンクシーン  0 ～ */
	int  nCount;		/* 表示時間 */
} DE2EFFECTE3FACEANIMATIONINFO, *PDE2EFFECTE3FACEANIMATIONINFO, *LPDE2EFFECTE3FACEANIMATIONINFO;

typedef struct tagDE2EFFECTROPEMODEL3INFO	/* ロープモデル３情報 */
{
	char    szModelName[64];	/* ロープモデル名 */
	int     nSampleNum;			/* サンプルデータ番号 */
	FVECTOR fvectMove;			/* 取り付ける場所のローカル座標値 */
	FVECTOR fvectRot;			/* 取り付けるモデルのローカルの回転値 */
	int     nScene;				/* リンクシーン  0 ～ */
	int     nNode;				/* リンクノード  0 ～ */
	float   fOvalParam;			/* 当たり判定パラメータ */
	int     nCollisionFlag;		/* 当たり判定フラグ */
	int     nVisibleFlag;		/* 強制表示フラグ */
	int     nMode;				/* 0: バンダナ  1: 髪の毛 */
	int     nCount;				/* 表示時間 */
} DE2EFFECTROPEMODEL3INFO, *PDE2EFFECTROPEMODEL3INFO, *LPDE2EFFECTROPEMODEL3INFO;

typedef struct tagDE2EFFECTOOZEBLOODDEMOINFO	/* 敵兵滲み血情報 */
{
	char szName[64];	/* エフェクト名 */
	int  nScene;		/* リンクシーン  0 ～ */
	int  nCount;		/* 表示時間 */
} DE2EFFECTOOZEBLOODDEMOINFO, *PDE2EFFECTOOZEBLOODDEMOINFO, *LPDE2EFFECTOOZEBLOODDEMOINFO;

typedef struct tagDE2EFFECTWAVINGCLOTHMODELWINFO	/* オブジェに付く布モデル情報 */
{
	char    szName[64];				/* エフェクト名 */
	char    szModelName[8][32];		/* 布モデル名 */
	int     nModelNameSize;			/* 布モデル数 */
	int     nSampleNum;				/* サンプルデータ番号 */
	int     nScene;					/* リンクシーン  0 ～ */
	int     nNode;					/* リンクノード  0 ～ */
	FVECTOR fvectMove;				/* 取り付ける場所のローカル座標値 */
	FVECTOR fvectRot;				/* 取り付けるモデルのローカルの回転値 */
	char    szBoundModelName[32];	/* 当たり判定用モデル名 */
	float   fOvalParam;				/* 当たり判定パラメータ */
	int     nForceDispFlag;			/* 強制表示フラグ */
	int     nCount;					/* 表示時間 */
} DE2EFFECTWAVINGCLOTHMODELWINFO, *PDE2EFFECTWAVINGCLOTHMODELWINFO, *LPDE2EFFECTWAVINGCLOTHMODELWINFO;

typedef struct tagDE2EFFECTROPEMODEL2INFO	/* ロープモデル２情報 */
{
	char    szModelName[64];	/* モデル名 */
	int     nSampleNum;			/* サンプルデータ番号 */
	int     nCollisionFlag;		/* 当たり判定フラグ */
	int     nCount;				/* 表示時間 */
} DE2EFFECTROPEMODEL2INFO, *PDE2EFFECTROPEMODEL2INFO, *LPDE2EFFECTROPEMODEL2INFO;

typedef struct tagDE2EFFECTRAINCOATINFO		/* レインコートの揺れ情報 */
{
	int   nScene;					/* リンクシーン  0 ～ */
	char  szSingleNames[8][64];		/* モデル名の配列 */
	int   nSingleNamesSize;			/* 配列のサイズ */
	char  szMultiName[64];			/* マルチウェイト側(レインコートの裾)のモデル名 */
	float fWindMin;					/* 風の強さの最小値 */
	float fWindMax;					/* 風の強さの最大値 */
	float fFrameMin;				/* アニメ間隔の最小値 */
	float fFrameMax;				/* アニメ間隔の最大値 */
	int   nCount;					/* 表示時間 */
} DE2EFFECTRAINCOATINFO, *PDE2EFFECTRAINCOATINFO, *LPDE2EFFECTRAINCOATINFO;

typedef struct tagDE2EFFECTSHADOWINFO	/* 足影情報 */
{
	int nScene;			/* リンクシーン  0 ～ */
	int nLFootNode;		/* 左足リンクノード  0 ～ */
	int nRFootNode;		/* 右足リンクノード  0 ～ */
	int nCount;			/* 表示時間 */
	int nLFootDefFlag;	/* 1: 左足デフォルト */
	int nRFootDefFlag;	/* 1: 右足デフォルト */
} DE2EFFECTSHADOWINFO, *PDE2EFFECTSHADOWINFO, *LPDE2EFFECTSHADOWINFO;

typedef struct tagDE2EFFECTATTACHMENT3INFO	/* 装備品Ｃ情報 */
{
	char    szModelNum[64];		/* 取り付けるモデル名 */
	int     nFrames;			/* 遅れフレーム数 */
	FVECTOR fvectMove;			/* 取り付ける場所のローカル座標値 */
	FVECTOR fvectRot;			/* 取り付けるモデルのローカルの回転値 */
	int     nScene;				/* リンクシーン  0 ～ */
	int     nNode;				/* リンクノード  0 ～ */
	int     nCount;				/* 表示時間 */
} DE2EFFECTATTACHMENT3INFO, *PDE2EFFECTATTACHMENT3INFO, *LPDE2EFFECTATTACHMENT3INFO;

typedef struct tagDE2EFFECTVANIMEBULLERINFO	/* 頂点アニメブラー情報 */
{
	char    szModelID[64];	/* モデル名 */
	float   fRotVy;			/* Y 軸回転速度 */
	FVECTOR fvectTo;		/* 目標点 */
	float   fSpeed;			/* 方向に対する移動速度 */
	int     nBase;			/* ベースキーフレーム */
	int     nRand;			/* キーフレームの乱数幅 */
	int     nCount;			/* 表示フレーム数  未使用 */
	int     nFlag;			/* デバッグフラグ  0: OFF  1: ON */
} DE2EFFECTVANIMEBULLERINFO, *PDE2EFFECTVANIMEBULLERINFO, *LPDE2EFFECTVANIMEBULLERINFO;

typedef struct tagDE2EFFECTDEMOBULLETINFO	/* 弾丸情報 */
{
	char    szModelID[64];	/* モデル名 */
	float   fRotVy;			/* Y 軸回転速度 */
	FVECTOR fvectTo;		/* 目標点 */
	float   fSpeed;			/* 方向に対する移動速度 */
	int     nCount;			/* 表示フレーム数 */
	int     nFlag;			/* 弾丸の表示  0: 非表示  1: 表示 */
} DE2EFFECTDEMOBULLETINFO, *PDE2EFFECTDEMOBULLETINFO, *LPDE2EFFECTDEMOBULLETINFO;

typedef struct tagDE2EFFECTGBSFACEDEMOINFO	/* ゴルルゴ兵目パチ情報 */
{
	char szName[64];	/* エフェクト名 */
	int  nScene;		/* リンクシーン  0 ～ */
	int  nCount;		/* 表示時間 */
} DE2EFFECTGBSFACEDEMOINFO, *PDE2EFFECTGBSFACEDEMOINFO, *LPDE2EFFECTGBSFACEDEMOINFO;










typedef struct tagDE2EFFECTC4EXPLOSION	/* 爆発 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTC4EXPLOSION + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	BYTE    byPadding1[8];	/* ダミー */
	FVECTOR fvectCenter;	/* 位置情報 */
} DE2EFFECTC4EXPLOSION, *PDE2EFFECTC4EXPLOSION, *LPDE2EFFECTC4EXPLOSION;

typedef struct tagDE2EFFECTBLOOD2D	/* 血煙 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTBLOOD2D + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	BYTE    byPadding1[8];	/* ダミー */
	FVECTOR fvectCenter;	/* 出現位置 */
} DE2EFFECTBLOOD2D, *PDE2EFFECTBLOOD2D, *LPDE2EFFECTBLOOD2D;

typedef struct tagDE2EFFECTBREATH	/* 息 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTBREATH + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	BYTE    byPadding1[8];	/* ダミー */
	FMATRIX fmatWorld;		/* 頭部のマトリックス */
	FVECTOR fvectMove;		/* 頭部の中心からの位置 */
} DE2EFFECTBREATH, *PDE2EFFECTBREATH, *LPDE2EFFECTBREATH;

typedef struct tagDE2EFFECTSHIELDSMOKE	/* 盾煙 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSHIELDSMOKE + 0x01000000) */
	int     nID;			/* 動作監視 ID */
	BYTE    byPadding1[8];	/* ダミー */
	FMATRIX fmatWorld;		/* マトリックス情報 */
} DE2EFFECTSHIELDSMOKE, *PDE2EFFECTSHIELDSMOKE, *LPDE2EFFECTSHIELDSMOKE;

typedef struct tagDE2EFFECTSPARK1	/* 火花 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSPARK1 + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nCount;			/* 表示フレーム数 */
	int     nPrims;			/* 火花の数 */
	float   fMinSpeed;		/* 初期最小スピード */
	float   fSpeedWide;		/* スピード幅 */
	float   fGravity;		/* 重力 */
	float   fLength;		/* スピードに対する火の長さの割合 */
	FVECTOR fvectCenter;	/* 位置 */
	FVECTOR fvectColor;		/* 色 */
	SVECTOR svectRot;		/* 向き */
	SVECTOR svectRotWide;	/* ワイド */
} DE2EFFECTSPARK1, *PDE2EFFECTSPARK1, *LPDE2EFFECTSPARK1;

typedef struct tagDE2EFFECTPIYORI	/* ぴよぴよ（ノーマル） */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTPIYORI + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nNode;		/* リンクノード  0 ～ */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTPIYORI, *PDE2EFFECTPIYORI, *LPDE2EFFECTPIYORI;

typedef struct tagDE2EFFECTPIYORIANES	/* ぴよぴよ（麻酔） */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTPIYORIANES + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nNode;		/* リンクノード  0 ～ */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTPIYORIANES, *PDE2EFFECTPIYORIANES, *LPDE2EFFECTPIYORIANES;

typedef struct tagDE2EFFECTZZZ	/* 居眠り */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTZZZ + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nNode;		/* リンクノード  0 ～ */
} DE2EFFECTZZZ, *PDE2EFFECTZZZ, *LPDE2EFFECTZZZ;

typedef struct tagDE2EFFECTCAMERABLOOD	/* カメラ血飛沫 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTCAMERABLOOD + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nWhite;			/* 色  0: 赤  1: 白 */
	BYTE    byPadding1[4];	/* ダミー */
	FVECTOR fvectPosPers;	/* カメラ座標系の座標 X,Y 方向  範囲: -1 ～ 1 */
} DE2EFFECTCAMERABLOOD, *PDE2EFFECTCAMERABLOOD, *LPDE2EFFECTCAMERABLOOD;

typedef struct tagDE2EFFECTBLOODSPREAD	/* 床に広がる血 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTBLOODSPREAD + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	SVECTOR svectRot;		/* 法線方向 */
	FVECTOR fvectPos;		/* 位置 */
	int     nDecay;			/* 消滅するまでのフレーム数 */
	float   fSize;			/* サイズ */
} DE2EFFECTBLOODSPREAD, *PDE2EFFECTBLOODSPREAD, *LPDE2EFFECTBLOODSPREAD;

typedef struct tagDE2EFFECTWALLBLOOD	/* 壁に付着する血 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTWALLBLOOD + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	SVECTOR svectRot;		/* 方向 */
	FVECTOR fvectPos;		/* 位置 */
	int     nWhite;			/* 色  0: 赤  1: 白 */
} DE2EFFECTWALLBLOOD, *PDE2EFFECTWALLBLOOD, *LPDE2EFFECTWALLBLOOD;

typedef struct tagDE2EFFECTCIRCLELIGHT	/* 敵兵の銃のライト */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTCIRCLELIGHT + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nNode;		/* リンクノード  0 ～ */
	int nMode;		/* 武器の種類  0: E_WP_AKS  1: E_WP_AKS_SP  2: アクリル盾  3: プレイヤーＵＳＰ（GUN_LIGHT2 付きの時） */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTCIRCLELIGHT, *PDE2EFFECTCIRCLELIGHT, *LPDE2EFFECTCIRCLELIGHT;

typedef struct tagDE2EFFECTFADEINOUT	/* フェードイン・アウト */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTFADEINOUT + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nAddSub;		/* 0: 加算  1: 減算 */
	int     nColorR;		/* 0 ～ 255 */
	int     nColorG;		/* 0 ～ 255 */
	int     nColorB;		/* 0 ～ 255 */
	int     nFadeInOut;		/* 0: フェードイン  1: フェードアウト */
	int     nVariation;		/* 0: フル  1: センター  2: まだら  3: パース */
	FVECTOR fvectOffset;	/* (nVariation == 3) の時: 位置
							                 それ以外: ＮＵＬＬを指定すること */
	int     nCount;			/* 変化フレーム数 */
	int     nStable;		/*       -1: 変化フレーム後デストロイする（通常）
							   0 ～ 128: 途中で止めるα（且つ自分でデストロイしない） */
	int     nKillCount;		/* 表示フレーム数 */
	int     nScene;			/* リンクシーン  0 ～ */
	int     nNode;			/* リンクノード  0 ～ */
} DE2EFFECTFADEINOUT, *PDE2EFFECTFADEINOUT, *LPDE2EFFECTFADEINOUT;

typedef struct tagDE2EFFECTFADEINOUTDEMO	/* フェードイン・アウトデモ */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTFADEINOUTDEMO + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nColorR;	/* 0 ～ 255 */
	int nColorG;	/* 0 ～ 255 */
	int nColorB;	/* 0 ～ 255 */
	int nColorA;	/* 0 ～ 128 */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTFADEINOUTDEMO, *PDE2EFFECTFADEINOUTDEMO, *LPDE2EFFECTFADEINOUTDEMO;

typedef struct tagDE2EFFECTBLOODDEMO	/* 任意血 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTBLOODDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nScene;			/* リンクシーン  0 ～ */
	int     nNode;			/* リンクノード  0 ～ */
	FVECTOR fvectForce;		/* 吹き出し方向 */
	int     nSize;			/* 大きさ */
	int     nCount;			/* 表示フレーム数 */
	int     nPat;			/* 現在未使用  常に0 */
	BYTE    byPadding1[4];	/* ダミー */
	FVECTOR fvectOffset;	/* オフセット座標 */
} DE2EFFECTBLOODDEMO, *PDE2EFFECTBLOODDEMO, *LPDE2EFFECTBLOODDEMO;

typedef struct tagDE2EFFECTBLOODDROPS	/* 落ちる血 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTBLOODDROPS + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nDecay;			/* 消滅するまでのフレーム数 */
	int     nWhite;			/* 色  0: 赤  1: 白 */
	FVECTOR fvectPos;		/* 位置 */
	int     nSpreadSize;	/* 床に到達した後に広がる血の大きさ */
} DE2EFFECTBLOODDROPS, *PDE2EFFECTBLOODDROPS, *LPDE2EFFECTBLOODDROPS;

typedef struct tagDE2EFFECTPLASMALINEDEMO	/* プラズマライン */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTPLASMALINEDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nScene1;		/* リンクシーン1  0 ～ */
	int     nNode1;			/* リンクノード1  0 ～ */
	int     nScene2;		/* リンクシーン2  0 ～ */
	int     nNode2;			/* リンクノード2  0 ～ */
	int     nRadius;		/* 半径 */
	int     nCount;			/* 表示フレーム数 */
	FVECTOR fvectOffset1;	/* 1 のオフセット座標 */
	FVECTOR fvectOffset2;	/* 2 のオフセット座標 */
} DE2EFFECTPLASMALINEDEMO, *PDE2EFFECTPLASMALINEDEMO, *LPDE2EFFECTPLASMALINEDEMO;

typedef struct tagDE2EFFECTBODYPLASMADOTDEMO	/* 光学迷彩壊れスパーク */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTBODYPLASMADOTDEMO + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nModel;		/* CV2 モデル (strcode) */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTBODYPLASMADOTDEMO, *PDE2EFFECTBODYPLASMADOTDEMO, *LPDE2EFFECTBODYPLASMADOTDEMO;

typedef struct tagDE2EFFECTENEEQUIP	/* 敵兵装備品 */
{
	int nEffectID;		/* エフェクト番号( DE2_EFFECTENEEQUIP + 0x01000000 ) */
	int nID;			/* 動作監視 ID */
	int nScene;			/* リンクシーン  0 ～ */
	int nFlag;			/* フラグ */
	int nWeaponName;	/* 武器モデル (strcode) */
	int nCount;			/* 表示フレーム数 */
} DE2EFFECTENEEQUIP, *PDE2EFFECTENEEQUIP, *LPDE2EFFECTENEEQUIP;

typedef struct tagDE2EFFECTBODYSHADOW	/* 遮り影 */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTBODYSHADOW + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTBODYSHADOW, *PDE2EFFECTBODYSHADOW, *LPDE2EFFECTBODYSHADOW;

typedef struct tagDE2EFFECTFLUSH	/* ＣＬＵＴフラッシュ */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTFLUSH + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nDecay;		/* フラッシュ継続時間 */
	int nMode;		/* 0: ブラーなし  1: ブラーあり */
} DE2EFFECTFLUSH, *PDE2EFFECTFLUSH, *LPDE2EFFECTFLUSH;

typedef struct tagDE2EFFECTRAINCAMERADEMO	/* カメラ前水滴 */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTRAINCAMERADEMO + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nLife;		/* 効果継続時間 */
} DE2EFFECTRAINCAMERADEMO, *PDE2EFFECTRAINCAMERADEMO, *LPDE2EFFECTRAINCAMERADEMO;

typedef struct tagDE2EFFECTMESSAGE	/* メッセージ */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTMESSAGE + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nName;		/* エフェクト名のStrCode値 */
	int nLength;	/* パラメータ数 */
	int nParam[1];	/* パラメータ配列 */
} DE2EFFECTMESSAGE, *PDE2EFFECTMESSAGE, *LPDE2EFFECTMESSAGE;

typedef struct tagDE2EFFECTCARTRIDGECONTROL		/* 銃のカートリッジ */
{
	int nEffectID;		/* エフェクト番号( DE2_EFFECTCARTRIDGECONTROL + 0x01000000 ) */
	int nID;			/* 動作監視 ID */
	int nName;			/* エフェクト名のStrCode値 */
	int nScene1;		/* リンク人体シーン  0 ～ */
	int nScene2;		/* リンク武器シーン  0 ～ */
	int nPat;			/* 人体と武器の種類 */
	int nControlNum;	/* カートリッジの挙動制御 */
	int nCount;			/* 表示フレーム数 */
} DE2EFFECTCARTRIDGECONTROL, *PDE2EFFECTCARTRIDGECONTROL, *LPDE2EFFECTCARTRIDGECONTROL;

typedef struct tagDE2EFFECTNEARFOCUS	/* 近景ぼかし */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTNEARFOCUS + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nName;		/* エフェクト名のStrCode値 */
	int nMaxPlane;	/* 最大描画枚数 */
	int nNear;		/* ぼかし最大距離 */
	int nFar;		/* ぼかし最小距離 */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTNEARFOCUS, *PDE2EFFECTNEARFOCUS, *LPDE2EFFECTNEARFOCUS;

typedef struct tagDE2EFFECTFARFOCUS	/* 遠景ぼかし */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTFARFOCUS + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nName;		/* エフェクト名のStrCode値 */
	int nMaxPlane;	/* 最大描画枚数 */
	int nNear;		/* ぼかし最小距離 */
	int nFar;		/* ぼかし最大距離 */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTFARFOCUS, *PDE2EFFECTFARFOCUS, *LPDE2EFFECTFARFOCUS;

typedef struct tagDE2EFFECTSPRITEFOGWORLDDEMO	/* テクセル流しワールド */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSPRITEFOGWORLDDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nTexName;		/* テクスチュア名のStrCode値 */
	int     nSize;			/* スプライトサイズ */
	int     nScene1;		/* 立方体の範囲1 リンクシーン  -1: 無効  0 ～ */
	int     nNode1;			/* 立方体の範囲1 リンクノード  0 ～ */
	int     nScene2;		/* 立方体の範囲2 リンクシーン  -1: 無効  0 ～ */
	int     nNode2;			/* 立方体の範囲2 リンクノード  0 ～ */
	FVECTOR fvectBound1;	/* 立方体の範囲1 */
	FVECTOR fvectBound2;	/* 立方体の範囲2 */
	FVECTOR fvectDir;		/* 進行方向ベクトル */
	FVECTOR fvectColor;		/* 最大到達RGBα */
	int     nLife;			/* 消滅までのフレーム数 */
	int     nMode;			/* 0: 加算半透明  1: 減算半透明  2:平均半透明 */
} DE2EFFECTSPRITEFOGWORLDDEMO, *PDE2EFFECTSPRITEFOGWORLDDEMO, *LPDE2EFFECTSPRITEFOGWORLDDEMO;

typedef struct tagDE2EFFECTSPRITEFOGPERSDEMO	/* テクセル流しワールド透視変換 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSPRITEFOGPERSDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nTexName;		/* テクスチュア名のStrCode値 */
	int     nSize;			/* スプライトサイズ */
	int     nScene1;		/* 立方体の範囲1 リンクシーン  -1: 無効  0 ～ */
	int     nNode1;			/* 立方体の範囲1 リンクノード  0 ～ */
	int     nScene2;		/* 立方体の範囲2 リンクシーン  -1: 無効  0 ～ */
	int     nNode2;			/* 立方体の範囲2 リンクノード  0 ～ */
	FVECTOR fvectBound1;	/* 立方体の範囲1 */
	FVECTOR fvectBound2;	/* 立方体の範囲2 */
	FVECTOR fvectDir;		/* 進行方向ベクトル */
	FVECTOR fvectColor;		/* 最大到達RGBα */
	int     nLife;			/* 消滅までのフレーム数 */
	int     nMode;			/* 0: 加算半透明  1: 減算半透明  2:平均半透明 */
} DE2EFFECTSPRITEFOGPERSDEMO, *PDE2EFFECTSPRITEFOGPERSDEMO, *LPDE2EFFECTSPRITEFOGPERSDEMO;

typedef struct tagDE2EFFECTSPRITEFOGONCAMERADEMO	/* テクセル流しカメラ */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSPRITEFOGONCAMERADEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nTexName;		/* テクスチュア名のStrCode値 */
	int     nSize;			/* スプライトサイズ */
	FVECTOR fvectDir;		/* 進行方向ベクトル */
	FVECTOR fvectColor;		/* 最大到達RGBα */
	int     nLife;			/* 消滅までのフレーム数 */
	int     nMode;			/* 0: 加算半透明  1: 減算半透明  2:平均半透明 */
} DE2EFFECTSPRITEFOGONCAMERADEMO, *PDE2EFFECTSPRITEFOGONCAMERADEMO, *LPDE2EFFECTSPRITEFOGONCAMERADEMO;

typedef struct tagDE2EFFECTSPLASHDEMO	/* 風紋水飛沫 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSPLASHDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	float   fRadius;		/* 発生円の半径 */
	float   fSize;			/* 水飛沫自体の大きさ */
	int     nScene1;		/* 起源点 リンクシーン  -1: 無効  0 ～ */
	int     nNode1;			/* 起源点 リンクノード  0 ～ */
	int     nScene2;		/* 発生円の中心 リンクシーン  -1: 無効  0 ～ */
	int     nNode2;			/* 発生円の中心 リンクノード  0 ～ */
	FVECTOR fvectOrigin;	/* 起源点 */
	FVECTOR fvectCenter;	/* 発生円の中心 */
	int     nMultiple;		/* １フレームに発生させる水飛沫のパーツの個数 */
	int     nLife;			/* 発生させ続けるフレーム数 */
} DE2EFFECTSPLASHDEMO, *PDE2EFFECTSPLASHDEMO, *LPDE2EFFECTSPLASHDEMO;

typedef struct tagDE2EFFECTSPLASHPARTSDEMO	/* 単発水飛沫 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSPLASHPARTSDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	SVECTOR svectRot;		/* 発生方向 */
	FVECTOR fvectCenter;	/* 発生場所 */
	float   fIntense;		/* 強さ */
} DE2EFFECTSPLASHPARTSDEMO, *PDE2EFFECTSPLASHPARTSDEMO, *LPDE2EFFECTSPLASHPARTSDEMO;

typedef struct tagDE2EFFECTSPLASHRIPPLEDEMO	/* 水飛沫複数呼び */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSPLASHRIPPLEDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	float   fRadius;		/* 発生円の最大半径 */
	float   fDirection;		/* Ｙ軸回転角度（ラジアン単位） */
	FVECTOR fvectCenter;	/* 発生円の中心 */
	float   fAngle;			/* direction からの＋－ */
	float   fSize;			/* 水飛沫自体の大きさ */
	int     nMultiple;		/* 発生させる円弧内の水飛沫の数 */
	int     nLife;			/* 発生させ続けるフレーム数 */
} DE2EFFECTSPLASHRIPPLEDEMO, *PDE2EFFECTSPLASHRIPPLEDEMO, *LPDE2EFFECTSPLASHRIPPLEDEMO;

typedef struct tagDE2EFFECTOPTICALCAMOBREAKDEMO		/* 光学迷彩壊れ */
{
	int nEffectID;		/* エフェクト番号( DE2_EFFECTOPTICALCAMOBREAKDEMO + 0x01000000 ) */
	int nID;			/* 動作監視 ID */
	int nScene;			/* リンクシーン  0 ～ */
	int nType;			/* エフェクトタイプ  0: 横歪み  1: 縦縮小 */
	int nStartCount;	/* 開始時間 */
	int nEndCount;		/* 終了時間 */
	int nColor;			/* 光学迷彩カラー */
} DE2EFFECTOPTICALCAMOBREAKDEMO, *PDE2EFFECTOPTICALCAMOBREAKDEMO, *LPDE2EFFECTOPTICALCAMOBREAKDEMO;

typedef struct tagDE2EFFECTBODYSHADOWVOLUMEDEMO		/* ボリューミックシャドーライン */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTBODYSHADOWVOLUMEDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nScene;			/* リンクシーン  0 ～ */
	float   fReach;			/* 光源から光が（影が）到達する距離 */
	float   fBackShift;		/* 角度調節のために光源位置を仮想的に後ろへシフトさせる距離 */
	int     nModelID;		/* CV2モデル */
	int     nScene2;		/* リンクシーン2  0 ～ */
	int     nNode2;			/* リンクノード2  0 ～ */
	FVECTOR fvectOffset2;	/* 2 のオフセット座標 */
	int     nAddSub;		/* 加算減算フラグ  0: 減算  1: 加算 */
	int     nLife;			/* 寿命。フレーム指定 */
} DE2EFFECTBODYSHADOWVOLUMEDEMO, *PDE2EFFECTBODYSHADOWVOLUMEDEMO, *LPDE2EFFECTBODYSHADOWVOLUMEDEMO;

typedef struct tagDE2EFFECTGBSHANDDEMO	/* ゴルルコ フィンガーサイン */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTGBSHANDDEMO + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nName;		/* エフェクト名のStrCode値 */
	int nScene;		/* リンクシーン  0 ～ */
	int nCount;		/* 表示時間 */
} DE2EFFECTGBSHANDDEMO, *PDE2EFFECTGBSHANDDEMO, *LPDE2EFFECTGBSHANDDEMO;

typedef struct tagDE2EFFECTPUTSTANIMEOBJECTCALL		/* 頂点ストリームアニメーション */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTPUTSTANIMEOBJECTCALL + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nName;			/* エフェクト名のStrCode値 */
	int     nModel;			/* モデル名のStrCode値 */
	FVECTOR fvectPos;		/* 位置 */
	FVECTOR fvectScale;		/* サイズ */
	FVECTOR fvectBound_U;	/* モデルのバウンド最大点 */
	FVECTOR fvectBound_L;	/* モデルのバウンド最小点 */
	SVECTOR svectRot;		/* 回転 */
	int     nMode;			/* 再生モード */
	int     nCount;			/* 表示フレーム数 */
	int     nScene;			/* リンクシーン  -1: 無効  0 ～ */
	int     nNode;			/* リンクノード  0 ～ */
} DE2EFFECTPUTSTANIMEOBJECTCALL, *PDE2EFFECTPUTSTANIMEOBJECTCALL, *LPDE2EFFECTPUTSTANIMEOBJECTCALL;

typedef struct tagDE2EFFECTE3FACEANIMATION	/* Ｅ３限定フェイスアニメ制御 */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTE3FACEANIMATION + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nName;		/* エフェクト名のStrCode値 */
	int nScene;		/* リンクシーン  0 ～ */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTE3FACEANIMATION, *PDE2EFFECTE3FACEANIMATION, *LPDE2EFFECTE3FACEANIMATION;

typedef struct tagDE2EFFECTCONTRASTDEMO		/* コントラスト＆ネガポジ */
{
	int nEffectID;		/* エフェクト番号( DE2_EFFECTCONTRASTDEMO + 0x01000000 ) */
	int nID;			/* 動作監視 ID */
	int nColU_R;		/* 飽和させる下限値R（0 ～ 255）*/
	int nColU_G;		/* 飽和させる下限値G（0 ～ 255）*/
	int nColU_B;		/* 飽和させる下限値B（0 ～ 255）*/
	int nColD_R;		/* 飽和させる上限値R（0 ～ (255 - nColU_R)）*/
	int nColD_G;		/* 飽和させる上限値G（0 ～ (255 - nColU_G)）*/
	int nColD_B;		/* 飽和させる上限値B（0 ～ (255 - nColU_B)）*/
	int nNegaPosiFlag;	/* 0: 通常  1: ネガ（nColD_R, nColD_G, nColD_B が引かれる） */
	int nTime;			/* 変化終了までのフレーム数 */
} DE2EFFECTCONTRASTDEMO, *PDE2EFFECTCONTRASTDEMO, *LPDE2EFFECTCONTRASTDEMO;

typedef struct tagDE2EFFECTPLASMAPOLYDEMO	/* プラズマポリゴン */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTPLASMAPOLYDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nRadius;		/* 半径 */
	int     nBranch;		/* 枝分かれフラグ */
	int     nScene1;		/* リンクシーン1  0 ～ */
	int     nNode1;			/* リンクノード1  0 ～ */
	int     nScene2;		/* リンクシーン2  0 ～ */
	int     nNode2;			/* リンクノード2  0 ～ */
	FVECTOR fvectOffset1;	/* 1 のオフセット座標 */
	FVECTOR fvectOffset2;	/* 2 のオフセット座標 */
	int     nLife;			/* 表示フレーム数 */
	int     nWidth;			/* 幅  -1 以下: デフォルト幅  0: 長さに比例  1 以上: 幅の値 */
} DE2EFFECTPLASMAPOLYDEMO, *PDE2EFFECTPLASMAPOLYDEMO, *LPDE2EFFECTPLASMAPOLYDEMO;

typedef struct tagDE2EFFECTTHUNDERDEMO	/* 稲光 */
{
	int   nEffectID;	/* エフェクト番号( DE2_EFFECTTHUNDERDEMO + 0x01000000 ) */
	int   nID;			/* 動作監視 ID */
	float fX;			/* X 座標 -1.0 ～ 1.0 */
	float fY;			/* Y 座標 -1.0 ～ 1.0 */
} DE2EFFECTTHUNDERDEMO, *PDE2EFFECTTHUNDERDEMO, *LPDE2EFFECTTHUNDERDEMO;

typedef struct tagDE2EFFECTFOGSETDEMO	/* フォグセット */
{
	int   nEffectID;	/* エフェクト番号( DE2_EFFECTFOGSETDEMO + 0x01000000 ) */
	int   nID;			/* 動作監視 ID */
	int   nCol_R;		/* 目標カラーＲ成分 */
	int   nCol_G;		/* 目標カラーＧ成分 */
	int   nCol_B;		/* 目標カラーＢ成分 */
	float fNear;		/* 目標ニア値 */
	float fFar;			/* 目標ファー値 */
	int   nTime;		/* 変化フレーム数 */
} DE2EFFECTFOGSETDEMO, *PDE2EFFECTFOGSETDEMO, *LPDE2EFFECTFOGSETDEMO;

typedef struct tagDE2EFFECTSPLASHMOTIONDEMO		/* モーション連動水飛沫 */
{
	int   nEffectID;	/* エフェクト番号( DE2_EFFECTSPLASHMOTIONDEMO + 0x01000000 ) */
	int   nID;			/* 動作監視 ID */
	int   nScene;		/* リンクシーン  0 ～ */
	float fStepLimit;	/* 前フレームとどれだけ離れていれば発生させるか */
	int   nLife;		/* 表示フレーム数 */
} DE2EFFECTSPLASHMOTIONDEMO, *PDE2EFFECTSPLASHMOTIONDEMO, *LPDE2EFFECTSPLASHMOTIONDEMO;

typedef struct tagDE2EFFECTROPEMODEL3	/* ロープモデル３ */
{
	int     nEffectID;			/* エフェクト番号( DE2_EFFECTROPEMODEL3 + 0x01000000 ) */
	int     nID;				/* 動作監視 ID */
	int     nModelName;			/* ロープモデル名のStrCode値 */
	int     nSampleNum;			/* サンプルデータ番号 */
	FVECTOR fvectMove;			/* 取り付ける場所のローカル座標値 */
	SVECTOR svectRot;			/* 取り付けるモデルのローカルの回転値 */
	int     nScene;				/* リンクシーン  0 ～ */
	int     nNode;				/* リンクノード  0 ～ */
	float   fOvalParam;			/* 当たり判定パラメータ */
	int     nCollisionFlag;		/* 当たり判定フラグ */
	int     nVisibleFlag;		/* 強制表示フラグ */
	int     nMode;				/* 0: バンダナ  1: 髪の毛 */
	int     nCount;				/* 表示フレーム数 */
} DE2EFFECTROPEMODEL3, *PDE2EFFECTROPEMODEL3, *LPDE2EFFECTROPEMODEL3;

typedef struct tagDE2EFFECTOOZEBLOODDEMO	/* 敵兵滲み血 */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTOOZEBLOODDEMO + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nName;		/* エフェクト名のStrCode値 */
	int nScene;		/* リンクシーン  0 ～ */
	int nModelID;	/* モデルファイル名のStrCode値 */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTOOZEBLOODDEMO, *PDE2EFFECTOOZEBLOODDEMO, *LPDE2EFFECTOOZEBLOODDEMO;

typedef struct tagDE2EFFECTWAVINGCLOTHMODELW	/* オブジェに付く布モデル */
{
	int     nEffectID;			/* エフェクト番号( DE2_EFFECTWAVINGCLOTHMODELW + 0x01000000 ) */
	int     nID;				/* 動作監視 ID */
	int     nName;				/* エフェクト名のStrCode値 */
	int     nModelName[8];		/* 布モデル名のStrCode値 */
	int     nModelNameSize;		/* 布モデル数 */
	int     nSampleNum;			/* サンプルデータ番号 */
	int     nScene;				/* リンクシーン  0 ～ */
	int     nNode;				/* リンクノード  0 ～ */
	int     nBoundModelName;	/* 当たり判定用モデル名のStrCode値 */
	FVECTOR fvectMove;			/* 取り付ける場所のローカル座標値 */
	SVECTOR svectRot;			/* 取り付けるモデルのローカルの回転値 */
	float   fOvalParam;			/* 当たり判定パラメータ */
	int     nForceDispFlag;		/* 強制表示フラグ */
	int     nCount;				/* 表示フレーム数 */
} DE2EFFECTWAVINGCLOTHMODELW, *PDE2EFFECTWAVINGCLOTHMODELW, *LPDE2EFFECTWAVINGCLOTHMODELW;

typedef struct tagDE2EFFECTROPEMODEL2	/* ロープモデル２ */
{
	int     nEffectID;			/* エフェクト番号( DE2_EFFECTROPEMODEL2 + 0x01000000 ) */
	int     nID;				/* 動作監視 ID */
	int     nModelName;			/* モデル名のStrCode値 */
	int     nSampleNum;			/* サンプルデータ番号 */
	FVECTOR fvectMove;			/* 取り付ける場所のローカル座標値 */
	SVECTOR svectRot;			/* 取り付けるモデルのローカルの回転値 */
	int     nCollisionFlag;		/* 当たり判定フラグ */
	int     nCount;				/* 表示フレーム数 */
} DE2EFFECTROPEMODEL2, *PDE2EFFECTROPEMODEL2, *LPDE2EFFECTROPEMODEL2;

typedef struct tagDE2EFFECTEVMMMORGA	/* オルガの揺れ物 */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTEVMMMORGA + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTEVMMMORGA, *PDE2EFFECTEVMMMORGA, *LPDE2EFFECTEVMMMORGA;

typedef struct tagDE2EFFECTSMOKEBLUR	/* 煙型ブラー */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSMOKEBLUR + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	int     nScene;			/* リンクシーン  0 ～ */
	int     nNode;			/* リンクノード  0 ～ */
	FVECTOR fvectMove;		/* オフセット座標 */
	FVECTOR fvectRot;		/* 回転 */
	int     nStartSpeed;	/* ブラーパーティクル初期スピード */
	int     nEndSpeed;		/* ブラーパーティクル最終スピード */
	int     nStartSize;		/* ブラーパーティクル初期サイズ */
	int     nEndSize;		/* ブラーパーティクル最終サイズ */
	int     nSpotSize;		/* ブラーパーティクル出現位置半径 */
	int     nSpotAngle;		/* 最大放射角度（ＰＳ角度0~4095） */
	int     nInterval;		/* 噴射間隔（フレーム単位） */
	int     nPrims;			/* 最大パーティクル数 */
	int     nColor;			/* 色指定  0x00BBGGRR */
	int     nFlag;			/* フラグ */
	int     nCount;			/* 表示フレーム数 */
} DE2EFFECTSMOKEBLUR, *PDE2EFFECTSMOKEBLUR, *LPDE2EFFECTSMOKEBLUR;

typedef struct tagDE2EFFECTBODYSPLASH	/* 体からの水はね */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTBODYSPLASH + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nModelID;	/* CV2 モデル (strcode) */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTBODYSPLASH, *PDE2EFFECTBODYSPLASH, *LPDE2EFFECTBODYSPLASH;

typedef struct tagDE2EFFECTRAINCOAT		/* レインコートの揺れ */
{
	int   nEffectID;			/* エフェクト番号( DE2_EFFECTRAINCOAT + 0x01000000 ) */
	int   nID;					/* 動作監視 ID */
	int   nScene;				/* リンクシーン  0 ～ */
	int   nSingleNames[8];		/* モデル名のStrCode値配列 */
	int   nSingleNamesSize;		/* 配列のサイズ */
	int   nMultiName;			/* マルチウェイト側(レインコートの裾)のモデル名のStrCode値 */
	float fWindMin;				/* 風の強さの最小値 */
	float fWindMax;				/* 風の強さの最大値 */
	float fFrameMin;			/* アニメ間隔の最小値 */
	float fFrameMax;			/* アニメ間隔の最大値 */
	int   nCount;				/* 表示フレーム数 */
} DE2EFFECTRAINCOAT, *PDE2EFFECTRAINCOAT, *LPDE2EFFECTRAINCOAT;

typedef struct tagDE2EFFECTSHADOW	/* 足影 */
{
	int nEffectID;		/* エフェクト番号( DE2_EFFECTSHADOW + 0x01000000 ) */
	int nID;			/* 動作監視 ID */
	int nScene;			/* リンクシーン  0 ～ */
	int nLFootNode;		/* リンクノード  0 ～ */
	int nRFootNode;		/* リンクノード  0 ～ */
	int nCount;			/* 表示フレーム数 */
} DE2EFFECTSHADOW, *PDE2EFFECTSHADOW, *LPDE2EFFECTSHADOW;

typedef struct tagDE2EFFECTDROPSHADOW	/* キャラ影 */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTDROPSHADOW + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTDROPSHADOW, *PDE2EFFECTDROPSHADOW, *LPDE2EFFECTDROPSHADOW;

typedef struct tagDE2EFFECTATTACHMENT3	/* 装備品Ｃ */
{
	int     nEffectID;	/* エフェクト番号( DE2_EFFECTATTACHMENT3 + 0x01000000 ) */
	int     nID;		/* 動作監視 ID */
	int     nModelNum;	/* 取り付けるモデル名のStrCode値 */
	int     nFrames;	/* 遅れフレーム数 */
	FVECTOR fvectMove;	/* 取り付ける場所のローカル座標値 */
	SVECTOR svectRot;	/* 取り付けるモデルのローカルの回転値 */
	int     nScene;		/* リンクシーン  0 ～ */
	int     nNode;		/* リンクノード  0 ～ */
	int     nCount;		/* 表示フレーム数 */
} DE2EFFECTATTACHMENT3, *PDE2EFFECTATTACHMENT3, *LPDE2EFFECTATTACHMENT3;

typedef struct tagDE2EFFECTINTERPOLYDEMO	/* ポリゴン頂点補完残像 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTINTERPOLYDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	SVECTOR svectCol;		/* ＲＧＢ */
	FVECTOR fvectOffset1;	/* 1 のオフセット座標 */
	FVECTOR fvectOffset2;	/* 2 のオフセット座標 */
	int     nScene1;		/* リンクシーン1  0 ～ */
	int     nNode1;			/* リンクノード1  0 ～ */
	int     nScene2;		/* リンクシーン2  0 ～ */
	int     nNode2;			/* リンクノード2  0 ～ */
	int     nDispF;			/* 保存フレーム数  最大１６ */
	int     nCount;			/* 表示フレーム数 */
} DE2EFFECTINTERPOLYDEMO, *PDE2EFFECTINTERPOLYDEMO, *LPDE2EFFECTINTERPOLYDEMO;

typedef struct tagDE2EFFECTPUTATTACHMENTS	/* ゆれる付属品 */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTPUTATTACHMENTS + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nAtID;		/* 付属品ID */
	int nFlag;		/* フラグ */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTPUTATTACHMENTS, *PDE2EFFECTPUTATTACHMENTS, *LPDE2EFFECTPUTATTACHMENTS;

typedef struct tagDE2EFFECTFOOTSPLASH	/* 足元水飛沫 */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTFOOTSPLASH + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nScene;		/* リンクシーン  0 ～ */
	int nCount;		/* 表示フレーム数 */
} DE2EFFECTFOOTSPLASH, *PDE2EFFECTFOOTSPLASH, *LPDE2EFFECTFOOTSPLASH;

typedef struct tagDE2EFFECTSAASMOKE	/* ＳＡＡ銃口煙 */
{
	int     nEffectID;	/* エフェクト番号( DE2_EFFECTSAASMOKE + 0x01000000 ) */
	int     nID;		/* 動作監視 ID */
	int     nScene;		/* リンクシーン  0 ～ */
	int     nNode;		/* リンクノード  0 ～ */
	FVECTOR fvectMove;	/* オフセット座標 */
	FVECTOR fvectRot;	/* 回転 */
	int     nLife;		/* 表示フレーム数 */
} DE2EFFECTSAASMOKE, *PDE2EFFECTSAASMOKE, *LPDE2EFFECTSAASMOKE;

typedef struct tagDE2EFFECTVANIMEBULLER	/* 頂点アニメブラー */
{
	int     nEffectID;	/* エフェクト番号( DE2_EFFECTVANIMEBULLER + 0x01000000 ) */
	int     nID;		/* 動作監視 ID */
	int     nModelID;	/* モデルID */
	float   fRotVy;		/* Y 軸回転速度 */
	FVECTOR fvectFrom;	/* 初期位置 */
	FVECTOR fvectTo;	/* 目標点 */
	float   fSpeed;		/* 方向に対する移動速度 */
	int     nBase;		/* ベースキーフレーム */
	int     nRand;		/* キーフレームの乱数幅 */
	int     nCount;		/* 表示フレーム数  未使用 */
	int     nFlag;		/* デバッグフラグ  0: OFF  1: ON */
} DE2EFFECTVANIMEBULLER, *PDE2EFFECTVANIMEBULLER, *LPDE2EFFECTVANIMEBULLER;

typedef struct tagDE2EFFECTDEMOBULLET	/* 弾丸 */
{
	int     nEffectID;	/* エフェクト番号( DE2_EFFECTDEMOBULLET + 0x01000000 ) */
	int     nID;		/* 動作監視 ID */
	int     nModelID;	/* モデルID */
	float   fRotVy;		/* Y 軸回転速度 */
	FVECTOR fvectFrom;	/* 初期位置 */
	FVECTOR fvectTo;	/* 目標点 */
	float   fSpeed;		/* 方向に対する移動速度 */
	int     nCount;		/* 表示フレーム数 */
	int     nFlag;		/* 弾丸の表示  0: 非表示  1: 表示 */
} DE2EFFECTDEMOBULLET, *PDE2EFFECTDEMOBULLET, *LPDE2EFFECTDEMOBULLET;

typedef struct tagDE2EFFECTSPARKDEMO	/* 跳弾 */
{
	int     nEffectID;		/* エフェクト番号( DE2_EFFECTSPARKDEMO + 0x01000000 ) */
	int     nID;			/* 動作監視 ID */
	BYTE    byPadding1[8];	/* ダミー */
	FMATRIX fmatDWorld;		/* 位置・方向 */
	FVECTOR fvectDColor;	/* 色 */
} DE2EFFECTSPARKDEMO, *PDE2EFFECTSPARKDEMO, *LPDE2EFFECTSPARKDEMO;

typedef struct tagDE2EFFECTGBSFACEDEMO	/* ゴルルゴ兵目パチ */
{
	int nEffectID;	/* エフェクト番号( DE2_EFFECTGBSFACEDEMO + 0x01000000 ) */
	int nID;		/* 動作監視 ID */
	int nName;		/* エフェクト名のStrCode値 */
	int nScene;		/* リンクシーン  0 ～ */
	int nCount;		/* 表示時間 */
} DE2EFFECTGBSFACEDEMO, *PDE2EFFECTGBSFACEDEMO, *LPDE2EFFECTGBSFACEDEMO;

/******************************************************************************
 */

#endif // _INC_DE2STRUCT
