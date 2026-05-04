/******************************************************************************
 * System	: KONAMI METALGEAR
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX
 * Compiler : Microsoft Visual C++ or cc
 * Module	: motion access2
 */

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WINDOWS
#include <io.h>
#include <windows.h>
#include <mmsystem.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <unistd.h>
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include "METALGEAR.h"

/* セーブロード用ヘッダ */
typedef struct {
	int			flags ;			/* 各種フラグ */
	int			motion_length ;	/* モーションの有効フレーム数 */
	int			motion_joints ;	/* モーションの関節数 */
	int			motion_tick ;	/* ベースクロック値（１／３００単位） */
	int			move_size ;		/* 移動量のサイズ((motion_length+1)*16) */
	int			rots_size ;		/* 関節回転データのサイズ(((motion_length+1)*motion_joints)*16) */
	int			trans_size ;	/* 関節移動量データへのサイズ(((motion_length+1)*motion_joints)*16) */
	int			minfo_size ;	/* 移動キー情報のサイズ((motion_length+1)*4) */
	int			jinfo_size ;	/* 関節キー情報のサイズ((motion_length+1)*motion_joints)*16) */
	int			sound_size ;	/* サウンド情報のサイズ（将来の拡張用） */
	int			anime_size ;	/* アニメーションのサイズ（将来の拡張用） */
	int			pad[4] ;
	int			error_angle ;	/* 圧縮時のエラー許容角度（ＰＳ固定小数点角度） */
} MTN_FILE_HEADER ;

typedef struct {
	float		vx, vy, vz, vw ;
} FVECTOR ;

typedef struct {
	unsigned int	key:1 ;
	unsigned int	fix:1 ;
	unsigned int	pad:30 ;
} KEY_INFO ;

enum {
	MTN_FLAG_LOOP = 0x0001,		/* ループモーションフラグ */
	MTN_FLAG_TURN = 0x0002,		/* ターンモーションフラグ */
	MTN_FLAG_ROOT = 0x0004,		/* 移動軌跡有効フラグ */
	MTN_FLAG_FIX  = 0x0008,		/* 関節固定有効フラグ */
	MTN_FLAG_TYPE_MASK = 0x00ff0000,		/* タイプ識別マスク */
	MTN_FLAG_TYPE_01   = 0x00010000,		/* 上半身下半身分離モーションフラグ */
};

/******************************************************************************
 * functions
 */

static BOOL WriteMotionMove(int fFile, HP3DMODEL hP3DSrcModel, HP3DMODEL hP3DModel, HP3DMOTION hP3DMotion, LPP3DXYZ lpp3dXYZScale);
static BOOL WriteMotionRotate(int fFile, HP3DMODEL hP3DSrcModel, HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, HP3DMOTION hP3DMotion);
static BOOL WriteMotionTrans( int fFile, HP3DMODEL hP3DSrcModel, HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, HP3DMOTION hP3DMotion, LPP3DXYZ lpp3dXYZScale );

static BOOL ProgressInterrupt(long*  lplBuffer, long lOffset, long lSize, LPARAM (*fnInterrupt)(WPARAM, LPARAM), LPARAM lParam);

static void MT_EulerToQuatXYZ( FVECTOR *quat, FVECTOR *rot);
static void MT_QuatNormalize( FVECTOR *res, FVECTOR *quat );
static int GetModelInfo_ObjectCount( HP3DMODEL hP3DModel );
static void GetAbsRotateMatrix( P3DMATRIX AbsMatrix, HP3DMODEL hP3DSrcModel, HP3DMODEL hP3DModel,
								HP3DOBJECT hP3DObject, HP3DMOTION hP3DMotion, int nFrame );

/******************************************************************************
 * publics
 */

/******************************************************************************
 * メタルギア 2 モーションの書き込み
 * fnInterrupt の第一引数は WPARAM で現在の書き込み率を渡します(％)
 * 第二引数は LPARAM で lParam 引数がそのまま渡されます。
 * 戻り値は LRESULT で値が 0 のときは書き込みを中止します。
 */
BOOL P3DWriteFileMETALGEAR2Motion(	/* 0以外	正常 */
									/* 0 */
	LPCSTR    lpcszFileName,					/* 書き込むファイルパス名 */
	LPP3DXYZ  lpp3dXYZScale,					/* スケール(標準:10.0F,10.0F,10.0F) */
	HP3DACTOR hP3DActor,						/* アクターハンドル */
	LPCSTR    lpcszMotionName,					/* 書き込む対象のモーション名 */
	LRESULT   (*fnInterrupt)(WPARAM, LPARAM),	/* 割り込み関数のポインタ(NULL で無視) */
	LPARAM    lParam)							/* fnInterrupt への引数 */
{
	long          lBuffer = 0;
	int           fFile;
	HP3DMODEL     hP3DModel;
	HP3DMOTION    hP3DMotion;
	HP3DMODEL     hP3DModelNew = NULL;
	/*MT1INFOHEADER mt1IH;*/
	MTN_FILE_HEADER	header, write_header ;

	/* 開始 0 ％を通知 */
	ProgressInterrupt(&lBuffer, 0, 100, fnInterrupt, lParam);

	if(((hP3DModel = P3DGetActorModel(hP3DActor)) == NULL)
		|| ((hP3DMotion = P3DFindMotionName(
			hP3DActor, lpcszMotionName)) == NULL)
		|| ((hP3DModelNew = P3DCreateConvertModel(hP3DModel,
			P3D_CCMMESH, P3D_CCMSKELETON, P3D_CCMNOCONVERT, NULL, 0 )) == NULL))
	{
		printf("convert start error!!\n");
		if(hP3DModelNew != NULL) P3DDestroyModel(hP3DModelNew);
		return 0;
	}

	/* MTN へ保存 */
	if((fFile = _openEx(lpcszFileName, _O_RDWR
		| _O_BINARY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE)) == -1)
	{
		P3DDestroyModel(hP3DModelNew);
		return 0;
	}

	/* MTNファイルヘッダーの初期化 */
	memset( &header, 0, sizeof(MTN_FILE_HEADER) );
	header.flags = 0 ;
	header.motion_length = P3DMotion(hP3DMotion)->nEndFrame
		- P3DMotion(hP3DMotion)->nStartFrame ;
	header.motion_joints = GetModelInfo_ObjectCount( hP3DModelNew ) ;
	header.motion_tick = 5 ;
	header.error_angle = 16 ;

	header.move_size = sizeof(FVECTOR) * ( header.motion_length + 1 ) ;
	header.rots_size = sizeof(FVECTOR) * ( header.motion_length + 1 ) * header.motion_joints ;
	header.minfo_size = ( sizeof(KEY_INFO) * ( header.motion_length + 1 ) + 15 ) & 0xffffff0 ;
	header.jinfo_size = ( sizeof(KEY_INFO) * ( header.motion_length + 1 ) * header.motion_joints + 15 ) & 0xffffff0 ;
	header.rots_size = sizeof(FVECTOR) * ( header.motion_length + 1 ) * header.motion_joints ;
	header.trans_size = sizeof(FVECTOR) * ( header.motion_length + 1 ) * header.motion_joints ;

	write_header = header ;
	LittleEndianStore(&write_header.flags, sizeof(write_header.flags));
	LittleEndianStore(&write_header.motion_length, sizeof(write_header.motion_length));
	LittleEndianStore(&write_header.motion_joints, sizeof(write_header.motion_joints));
	LittleEndianStore(&write_header.motion_tick, sizeof(write_header.motion_tick));
	LittleEndianStore(&write_header.error_angle, sizeof(write_header.error_angle));

	LittleEndianStore(&write_header.move_size, sizeof(write_header.move_size));
	LittleEndianStore(&write_header.rots_size, sizeof(write_header.rots_size));
	LittleEndianStore(&write_header.trans_size, sizeof(write_header.trans_size));
	LittleEndianStore(&write_header.minfo_size, sizeof(write_header.minfo_size));
	LittleEndianStore(&write_header.jinfo_size, sizeof(write_header.jinfo_size));

	/* MTN ファイルヘッダ書き込み */
	if((_writeEx(fFile,
			&header, sizeof(MTN_FILE_HEADER)) != sizeof(MTN_FILE_HEADER))
		|| (WriteMotionMove(
			fFile, hP3DModel, hP3DModelNew, hP3DMotion, lpp3dXYZScale) == 0)
		|| (WriteMotionRotate(
			fFile, hP3DModel, hP3DModelNew, NULL, hP3DMotion) == 0)
		|| (WriteMotionTrans(
			fFile, hP3DModel, hP3DModelNew, NULL, hP3DMotion, lpp3dXYZScale) == 0))
	{
		P3DDestroyModel(hP3DModelNew);
		_closeEx(fFile);
		remove(lpcszFileName);
		return 0;
	}

	/* MTNの残りのデータを書き込む */
	{/* キーフレーム情報の初期化＆書き込み */
		int i, size ;
		KEY_INFO *key ;

		size = sizeof(KEY_INFO) * ( header.motion_length + 1 ) ;
		size = ( size + 15 ) & 0xfffff0 ;
		key = malloc( size );
		memset( key, 0, size );
		/* 最低限必要なキーを設定しておく */
		key[ 0 + 1 ].key = 1 ;
		key[ header.motion_length ].key = 1 ;
		for ( i = 0 ; i < ( size / sizeof(KEY_INFO) ) ; i++ ){
			LittleEndianStore(&key[i], sizeof(KEY_INFO));
		}
		if ( _writeEx(fFile, key, size ) != size ){
			free( key );
			P3DDestroyModel(hP3DModelNew);
			_closeEx(fFile);
			remove(lpcszFileName);
			return 0;
		}
		free( key );
	}
	{/* 関節のキーフレーム情報の初期化＆書き込み */
		int i, size ;
		KEY_INFO *data, *key ;

		size = sizeof(KEY_INFO) * ( header.motion_length + 1 ) ;
		size = size * header.motion_joints ;
		size = ( size + 15 ) & 0xfffff0 ;
		data = malloc( size );
		memset( data, 0, size );
		/* 最低限必要なキーを設定しておく */
		for ( i = 0 ; i < header.motion_joints ; i++ ){
			key = &data[ i * ( header.motion_length + 1 ) ] ;
			key[ 0 + 1 ].key = 1 ;
			key[ header.motion_length ].key = 1 ;
		}
		for ( i = 0 ; i < ( size / sizeof(KEY_INFO) ) ; i++ ){
			LittleEndianStore(&data[i], sizeof(KEY_INFO));
		}
		if ( _writeEx(fFile, data, size ) != size ){
			free( data );
			P3DDestroyModel(hP3DModelNew);
			_closeEx(fFile);
			remove(lpcszFileName);
			return 0;
		}
		free( data );
	}

	_closeEx(fFile);
	P3DDestroyModel(hP3DModelNew);

	/* 開始 100 ％を通知 */
	ProgressInterrupt(&lBuffer, 100, 100, fnInterrupt, lParam);

	return 1;
}

/******************************************************************************
 * statics
 */

/******************************************************************************
 * モーションの移動量を書き込み
 */
static BOOL WriteMotionMove(	/* 0以外	正常 */
								/* 0 */
	int        fFile,			/* ファイルハンドル */
	HP3DMODEL  hP3DSrcModel,	/* 元々のモデルハンドル */
	HP3DMODEL  hP3DModel,		/* モデルハンドル */
	HP3DMOTION hP3DMotion,		/* モーションハンドル */
	LPP3DXYZ   lpp3dXYZScale)	/* スケール(標準:10.0F,10.0F,10.0F) */
{
	int              i;
	HP3DOBJECT       hP3DSrcObject;
	HP3DOBJECT       hP3DObject;
	HP3DMOTIONOBJECT hP3DMotionObject;
	LPP3DMOTIONFRAME lpP3DMotionFrame;
	MT1VECTOR        mt1Vector;
	FVECTOR          fvec ;

	if(((hP3DObject = P3DGetFirstObject(hP3DModel, NULL)) == NULL)
		|| ((hP3DSrcObject = P3DFindObjectName(
			hP3DSrcModel, P3DObject(hP3DObject)->szName)) == NULL)
		|| ((hP3DMotionObject = P3DFindMotionObjectObject(
			hP3DMotion, hP3DSrcObject)) == NULL))
		return 0;

#ifdef _DEBUG
	printfConsole(NULL, "OBJECT MOVE = %s\n", P3DObject(hP3DObject)->szName);
#endif

	for(i = 0, lpP3DMotionFrame
		= P3DMotionObject(hP3DMotionObject)->lpP3DMotionFrame;
		i < P3DMotionObject(hP3DMotionObject)->nMotionFrameCount;
		i++, lpP3DMotionFrame++)
	{
#ifdef _DEBUG
	printfConsole(NULL, " %d : %6.2f %6.2f %6.2f\n", i,
		lpP3DMotionFrame->p3dXYZMove.x, lpP3DMotionFrame->p3dXYZMove.y, lpP3DMotionFrame->p3dXYZMove.z);
#endif

		fvec.vx = lpP3DMotionFrame->p3dXYZMove.x * lpp3dXYZScale->x ;
		fvec.vy = lpP3DMotionFrame->p3dXYZMove.y * lpp3dXYZScale->y ;
		fvec.vz = lpP3DMotionFrame->p3dXYZMove.z * lpp3dXYZScale->z ;
		fvec.vw = 0.0 ;

		LittleEndianStore(&fvec.vx, sizeof(fvec.vx));
		LittleEndianStore(&fvec.vy, sizeof(fvec.vy));
		LittleEndianStore(&fvec.vz, sizeof(fvec.vz));
		LittleEndianStore(&fvec.vw, sizeof(fvec.vw));

		if(_writeEx(fFile, &fvec, sizeof(FVECTOR)) != sizeof(FVECTOR))
			return 0;
	}

	return 1;
}

/******************************************************************************
 * モーションのオブジェクト回転を書き込み
 */
static BOOL WriteMotionRotate(	/* 0以外	正常 */
								/* 0 */
	int        fFile,			/* ファイルハンドル */
	HP3DMODEL  hP3DSrcModel,	/* 元々のモデルハンドル */
	HP3DMODEL  hP3DModel,		/* モデルハンドル */
	HP3DOBJECT hP3DObject,		/* 親になるオブジェクトハンドル(NULL でトップレベル */
	HP3DMOTION hP3DMotion)		/* モーションハンドル */
{
	int              i;
	HP3DOBJECT       hP3DSrcObject;
	HP3DOBJECT       hP3DObjectL;
	HP3DMOTIONOBJECT hP3DMotionObject;
	LPP3DMOTIONFRAME lpP3DMotionFrame;
	/* 拡張 */
	FVECTOR          fvec, quat ;

	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
		return 1;

	do
	{
#ifdef _DEBUG
	printfConsole(NULL, "OBJECT ROTATE = %s\n", P3DObject(hP3DObjectL)->szName);
#endif
	printf("OBJECT ROTATE = %s\n", P3DObject(hP3DObjectL)->szName);

		if(((hP3DSrcObject = P3DFindObjectName(
				hP3DSrcModel, P3DObject(hP3DObjectL)->szName)) == NULL)
			|| ((hP3DMotionObject = P3DFindMotionObjectObject(
				hP3DMotion, hP3DSrcObject)) == NULL))
//			return 0;
		{
			printf("error !! %08x %08x\n", hP3DSrcObject, hP3DMotionObject );
			return 0 ;
		} else {
			printf("motion frame: %d\n", P3DMotionObject(hP3DMotionObject)->nMotionFrameCount);
		}

		for(i = 0, lpP3DMotionFrame
			= P3DMotionObject(hP3DMotionObject)->lpP3DMotionFrame;
			i < P3DMotionObject(hP3DMotionObject)->nMotionFrameCount;
			i++, lpP3DMotionFrame++)
		{
#ifdef _DEBUG
	printfConsole(NULL, " %d : %6.2f %6.2f %6.2f\n", i,
		lpP3DMotionFrame->p3dXYZRotate.x, lpP3DMotionFrame->p3dXYZRotate.y, lpP3DMotionFrame->p3dXYZRotate.z);
#endif
			fvec.vx = M_PI * lpP3DMotionFrame->p3dXYZRotate.x / 180.0 ;
			fvec.vy = M_PI * lpP3DMotionFrame->p3dXYZRotate.y / 180.0 ;
			fvec.vz = M_PI * lpP3DMotionFrame->p3dXYZRotate.z / 180.0 ;
			MT_EulerToQuatXYZ( &quat, &fvec );
			MT_QuatNormalize( &quat, &quat );

			LittleEndianStore(&quat.vx, sizeof(quat.vx));
			LittleEndianStore(&quat.vy, sizeof(quat.vy));
			LittleEndianStore(&quat.vz, sizeof(quat.vz));
			LittleEndianStore(&quat.vw, sizeof(quat.vw));

			if(_writeEx(fFile,
				&quat, sizeof(FVECTOR)) != sizeof(FVECTOR))
				return 0;
		}

		if(WriteMotionRotate(
			fFile, hP3DSrcModel, hP3DModel, hP3DObjectL, hP3DMotion) == 0)
			return 0;

	}
	while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);

	return 1;
}

/******************************************************************************
 * モーションのオブジェクト移動オフセット書き込み
 */
static BOOL WriteMotionTrans(	/* 0以外	正常 */
								/* 0 */
	int        fFile,			/* ファイルハンドル */
	HP3DMODEL  hP3DSrcModel,	/* 元々のモデルハンドル */
	HP3DMODEL  hP3DModel,		/* モデルハンドル */
	HP3DOBJECT hP3DObject,		/* 親になるオブジェクトハンドル(NULL でトップレベル */
	HP3DMOTION hP3DMotion,		/* モーションハンドル */
	LPP3DXYZ     lpp3dXYZScale)	/* スケール値 */
{
	int              i;
	HP3DOBJECT       hP3DSrcObject;
	HP3DOBJECT       hP3DObjectL;
	HP3DMOTIONOBJECT hP3DMotionObject;
	LPP3DMOTIONFRAME lpP3DMotionFrame;
	/* 拡張 */
	FVECTOR          fvec, quat ;
	LPP3DOBJECT      lpP3DSrcObject ;
	HP3DOBJECT       hP3DParentSrcObject;
	HP3DMOTIONOBJECT hP3DParentMotionObject;
	LPP3DMOTIONFRAME lpP3DParentMotionFrame;
	P3DMATRIX        P3DMatrix1, P3DMatrix2 ;
	P3DXYZ           P3DXyz ;

	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
		return 1;

	do
	{
#ifdef _DEBUG
	printfConsole(NULL, "OBJECT ROTATE = %s\n", P3DObject(hP3DObjectL)->szName);
#endif

		if(((hP3DSrcObject = P3DFindObjectName(
				hP3DSrcModel, P3DObject(hP3DObjectL)->szName)) == NULL)
			|| ((hP3DMotionObject = P3DFindMotionObjectObject(
				hP3DMotion, hP3DSrcObject)) == NULL))
			return 0;

		/* モーション取得中のオブジェクトの実態を取得 */
		lpP3DSrcObject = P3DObject( hP3DSrcObject ) ;

		/* 親オブジェクトのハンドル取得 */
		hP3DParentSrcObject = P3DGetParentObject( hP3DSrcObject );
		if ( hP3DParentSrcObject != NULL ){
			/* 親オブジェクトのモーションオブジェクト取得 */
			hP3DParentMotionObject = P3DFindMotionObjectObject( hP3DMotion, hP3DParentSrcObject ) ;
			if ( hP3DParentMotionObject != NULL ){
				/* 親オブジェクトのモーションフレーム取得 */
				lpP3DParentMotionFrame = P3DMotionObject(hP3DParentMotionObject)->lpP3DMotionFrame ;
			}
		}

		for(i = 0, lpP3DMotionFrame
			= P3DMotionObject(hP3DMotionObject)->lpP3DMotionFrame;
			i < P3DMotionObject(hP3DMotionObject)->nMotionFrameCount;
			i++, lpP3DMotionFrame++)
		{
#ifdef _DEBUG
	printfConsole(NULL, " %d : %6.2f %6.2f %6.2f\n", i,
		lpP3DMotionFrame->p3dXYZRotate.x, lpP3DMotionFrame->p3dXYZRotate.y, lpP3DMotionFrame->p3dXYZRotate.z);
#endif

			/* オブジェクトの角度＆デフォルトオフセットからの変位を求める */
			if ( hP3DParentSrcObject != NULL ){
				/* ルートからの総計回転マトリクス取得（＝絶対回転） */
				GetAbsRotateMatrix( P3DMatrix1, hP3DSrcModel, hP3DModel, hP3DParentSrcObject, hP3DMotion, i );
				/* 絶対回転マトリクスを親オブジェクトの位置に移動 */
				P3DMoveMatrix( P3DMatrix1,
							   lpP3DParentMotionFrame[i].p3dXYZMove.x,
							   lpP3DParentMotionFrame[i].p3dXYZMove.y,
							   lpP3DParentMotionFrame[i].p3dXYZMove.z );
				/* 逆変換マトリクス作成 */
				P3DInverseMatrix( P3DMatrix1 );
				/* スケルトン座標を表すマトリクス生成 */
				P3DInitMatrix( P3DMatrix2 );
				P3DMoveMatrix( P3DMatrix2,
							   lpP3DMotionFrame->p3dXYZMove.x,
							   lpP3DMotionFrame->p3dXYZMove.y,
							   lpP3DMotionFrame->p3dXYZMove.z );
				/* 親オブジェクトから見た相対オフセットを求める */
				P3DMultiplyMatrix( P3DMatrix2, P3DMatrix1 );
				P3DXyz.x = P3DMatrix2[3][0] - lpP3DSrcObject->p3dXYZMove.x ;
				P3DXyz.y = P3DMatrix2[3][1] - lpP3DSrcObject->p3dXYZMove.y ;
				P3DXyz.z = P3DMatrix2[3][2] - lpP3DSrcObject->p3dXYZMove.z ;
#if 0
				if ( ( i % 20 ) == 0 ){
					printf("%6.2f %6.2f %6.2f\n", P3DMatrix2[3][0], P3DMatrix2[3][1], P3DMatrix2[3][2] );
					printf("%6.2f %6.2f %6.2f\n",
						   lpP3DSrcObject->p3dXYZMove.x,
						   lpP3DSrcObject->p3dXYZMove.y,
						   lpP3DSrcObject->p3dXYZMove.z );
					printf("%6.2f %6.2f %6.2f %6.2f %6.2f %6.2f\n",
						   lpP3DParentMotionFrame[i].p3dXYZMove.x,
						   lpP3DParentMotionFrame[i].p3dXYZMove.y,
						   lpP3DParentMotionFrame[i].p3dXYZMove.z,
						   lpP3DMotionFrame->p3dXYZMove.x,
						   lpP3DMotionFrame->p3dXYZMove.y,
						   lpP3DMotionFrame->p3dXYZMove.z );
				}
#endif
			} else {
				/* ルートオブジェクトでは常に０ */
				P3DXyz.x = 0 ;
				P3DXyz.y = 0 ;
				P3DXyz.z = 0 ;
			}
			/* 誤差を無視するために四捨五入する */
			P3DXyz.x = (float)( (int)( ( P3DXyz.x * 10000 ) + 0.5 ) ) / 10000.0f ;
			P3DXyz.y = (float)( (int)( ( P3DXyz.y * 10000 ) + 0.5 ) ) / 10000.0f ;
			P3DXyz.z = (float)( (int)( ( P3DXyz.z * 10000 ) + 0.5 ) ) / 10000.0f ;

#if 0
			if ( i % 20 == 0 ){
				printf("%4d)%8.4f %8.4f %8.4f, %8.6f %8.6f %8.6f\n", i,
					   ( P3DXyz.x ) * lpp3dXYZScale->x,
					   ( P3DXyz.y ) * lpp3dXYZScale->y,
					   ( P3DXyz.z ) * lpp3dXYZScale->z,
					   lpP3DMotionFrame->p3dXYZRotate.x,
					   lpP3DMotionFrame->p3dXYZRotate.y,
					   lpP3DMotionFrame->p3dXYZRotate.z
					   );
			}
#endif

			fvec.vx = P3DXyz.x * lpp3dXYZScale->x ;
			fvec.vy = P3DXyz.y * lpp3dXYZScale->y ;
			fvec.vz = P3DXyz.z * lpp3dXYZScale->z ;
			fvec.vw = 1.0 ;

			LittleEndianStore(&fvec.vx, sizeof(fvec.vx));
			LittleEndianStore(&fvec.vy, sizeof(fvec.vy));
			LittleEndianStore(&fvec.vz, sizeof(fvec.vz));
			LittleEndianStore(&fvec.vw, sizeof(fvec.vw));

			if(_writeEx(fFile,
				&fvec, sizeof(FVECTOR)) != sizeof(FVECTOR))
				return 0;
		}

		if(WriteMotionTrans(
			fFile, hP3DSrcModel, hP3DModel, hP3DObjectL, hP3DMotion, lpp3dXYZScale) == 0)
			return 0;
	}
	while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);

	return 1;
}

/******************************************************************************
 * 割り込み処理
 * fnInterrupt 関数を第1引数に処理率(%)、第2引数に lParam 引数をそのまま渡して
 * 呼び出します。
 */
static BOOL ProgressInterrupt(	/* fnInterrupt 関数の戻り値(fnInterrupt が NULL のときは0以外) */
	long*  lplBuffer,						/* この関数の作業用領域(初回は0にすること) */
	long   lOffset,							/* 現在の位置 */
	long   lSize,							/* 最終位置 */
	LPARAM (*fnInterrupt)(WPARAM, LPARAM),	/* 割り込み関数のポインタ(NULL で無視) */
	LPARAM lParam)							/* fnInterrupt への引数 */
{
	if((fnInterrupt == NULL)
		|| ((lOffset != 0) && (lOffset != lSize)
		&& (((double)lOffset / (double)lSize * 100.0) < (*lplBuffer + 5))))
		return 1;

	*lplBuffer = (int)((double)lOffset / (double)lSize * 100.0);
	return (BOOL)fnInterrupt((WPARAM)*lplBuffer, lParam);
}


/******************************************************************************
 * 回転ベクトルからクォータニオンへ
 */
static void MT_EulerToQuatXYZ( FVECTOR *quat, FVECTOR *rot)
{
	float cr, cp, cy, sr, sp, sy, cpcy, spsy;

	cr = cos( rot->vx / 2 );
	cp = cos( rot->vy / 2 );
	cy = cos( rot->vz / 2 );

	sr = sin( rot->vx / 2 );
	sp = sin( rot->vy / 2 );
	sy = sin( rot->vz / 2 );

	cpcy = cp * cy;
	spsy = sp * sy;

	quat->vw = cr * cpcy + sr * spsy;
	quat->vx = sr * cpcy - cr * spsy;
	quat->vy = cr * sp * cy + sr * cp * sy;
	quat->vz = cr * cp * sy - sr * sp * cy;

}

/******************************************************************************
 * クォータニオンの正規化
 */
static void MT_QuatNormalize( FVECTOR *res, FVECTOR *quat )
{
    float	dist, square;

	square = quat->vx * quat->vx + quat->vy * quat->vy + quat->vz * quat->vz + quat->vw * quat->vw;
	if ( square > 0.0F ){
		dist = (float)( 1.0F / sqrt( square ) );
		res->vx = quat->vx * dist ;
		res->vy = quat->vy * dist ;
		res->vz = quat->vz * dist ;
		res->vw = quat->vw * dist ;
	} else {
		*res = *quat ;
	}

}

/******************************************************************************
 * その他の補助関数
 */
static int GetModelInfo_ObjectCount( HP3DMODEL hP3DModel )
{
	int		object_count, vertex_count, vertexweight_count, normal_count, polygon_count ;
	int		polygon_material_count ;
	int		polygon_texture_count, polygon_node_count, polygon_node_uv_count, material_count, texture_count ;
	int		picture_count ;
	P3DInformationModel( hP3DModel, &object_count, &vertex_count, &vertexweight_count,
						&normal_count, &polygon_count, &polygon_material_count, &polygon_texture_count,
						&polygon_node_count, &polygon_node_uv_count, &material_count, &texture_count,
						&picture_count );
	return ( object_count );
}

/* オブジェクトの指定したフレームでの絶対回転マトリクスの取得 */
static void GetAbsRotateMatrix(
	P3DMATRIX  AbsMatrix,
	HP3DMODEL  hP3DSrcModel,	/* 元々のモデルハンドル */
	HP3DMODEL  hP3DModel,		/* モデルハンドル */
	HP3DOBJECT hP3DSrcObject,	/* 親になるオブジェクトハンドル(NULL でトップレベル */
	HP3DMOTION hP3DMotion,		/* モーションハンドル */
	int        nFrame)
{
	HP3DOBJECT       hP3DParentSrcObject;
	HP3DMOTIONOBJECT hP3DMotionObject;
	LPP3DMOTIONFRAME lpP3DMotionFrame;
	P3DMATRIX        p3dMatrixLocal ;

	/* 親オブジェクトのハンドル取得 */
	if ( ( hP3DParentSrcObject = P3DGetParentObject( hP3DSrcObject ) ) != NULL ){
		/* 親の絶対回転マトリクスを取得 */
		GetAbsRotateMatrix( AbsMatrix, hP3DSrcModel, hP3DModel, hP3DParentSrcObject, hP3DMotion, nFrame);
	} else {
		P3DInitMatrix( AbsMatrix );
	}

	/* モーションオブジェクト取得 */
	hP3DMotionObject = P3DFindMotionObjectObject( hP3DMotion, hP3DSrcObject ) ;
	/* モーションフレーム取得 */
	lpP3DMotionFrame = P3DMotionObject(hP3DMotionObject)->lpP3DMotionFrame ;
	lpP3DMotionFrame += nFrame ;

	P3DInitMatrix( p3dMatrixLocal );
	P3DRotateXMatrix( p3dMatrixLocal, lpP3DMotionFrame->p3dXYZRotate.x );
	P3DRotateYMatrix( p3dMatrixLocal, lpP3DMotionFrame->p3dXYZRotate.y );
	P3DRotateZMatrix( p3dMatrixLocal, lpP3DMotionFrame->p3dXYZRotate.z );
	P3DMultiplyMatrix( p3dMatrixLocal, AbsMatrix );
	P3DCopyMatrix( AbsMatrix, p3dMatrixLocal );
	return ;
}
