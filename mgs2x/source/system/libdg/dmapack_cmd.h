/*
	dmapack.c
	ＤＭＡパケット接続型オブジェクトコマンド定義用ヘッダ

	2002/09/06 E.Takaki
*/

#ifdef	DMAPACK_CMD_EQU

	/*--		DG_DMAPACK_CMD_ ## cmd,		func --*/
	DMAPACK_CMD_EQU( POINT,			DrawPoint	)	/* ポイント（未サポート） */
	DMAPACK_CMD_EQU( LINE,			DrawLine	)	/* ライン */
	DMAPACK_CMD_EQU( TRIANGLE,		DrawTriangle)	/* トライアングル */
	DMAPACK_CMD_EQU( QUAD,			DrawQuad	)	/* 四角形 */
	DMAPACK_CMD_EQU( BOX,			DrawBox 	)	/* テクスチャ無しスプライト */
	DMAPACK_CMD_EQU( SPRT,			DrawSprt 	)	/* テクスチャ付きスプライト */
	DMAPACK_CMD_EQU( RSPRT,			DrawRSprt 	)
						/* 回転テクスチャ付きスプライト（四角形） */
	DMAPACK_CMD_EQU( LINESTRIP,		DrawLineStrip) 	/* ラインストリップ */
	DMAPACK_CMD_EQU( TRIANGLESTRIP,	DrawTriangleStrip)	/* トライアングルストリップ */
	DMAPACK_CMD_EQU( MODE,			DrawMode	)	/* 各種設定 *//* もう使ってはいけない */
	DMAPACK_CMD_EQU( TEX,			DrawTex		)	/* テクスチャ設定 */
	DMAPACK_CMD_EQU( TEXLIN,		DrawTexLin	)	/* 線形テクスチャ設定 */
	DMAPACK_CMD_EQU( USERTEX,		DrawUserTex	)	/* Userテクスチャ設定 */
	DMAPACK_CMD_EQU( ALPHA,			DrawAlpha	)	/* アルファ設定 */
	DMAPACK_CMD_EQU( WINDOW,		DrawWindow	)	/* 描画領域の設定 */
	DMAPACK_CMD_EQU( SETZ,			DrawSetZ	)	/* 描画時のＺ値を設定 */
	DMAPACK_CMD_EQU( ENABLE,		DrawModeEnable)	/* 各種設定 */
	DMAPACK_CMD_EQU( DISABLE,		DrawModeDisable)/* 各種設定 */
	DMAPACK_CMD_EQU( USEFRAMETEX,	DrawUseFrameTex)	
						/* フレームバッファをテクスチャとして設定 */
	DMAPACK_CMD_EQU( BACKUPFRAME,	DrawBackupFrame)
						/* 現在描画中のフレームバッファを退避 */
#if FALSE
	DMAPACK_CMD_EQU( BACKUPFRAME2BUFF,DrawBackupFrame2Buff)
						/* 現在描画中のフレームバッファをBufferに退避 */
#endif
	DMAPACK_CMD_EQU( CALLBACK,		DrawPacketCallback)
						/* コールバックを設定（移植した際に互換性がなくなるので注意！） */
	DMAPACK_CMD_EQU( NEXT,			DrawNext	)	
												/* 指定アドレスのパケットへジャンプ */
	DMAPACK_CMD_EQU( END,			DrawDummy ) /* パケットの終了 */

	DMAPACK_CMD_EQU( LINE_F,		DrawLineF	)	/* ライン(float精度) */
	DMAPACK_CMD_EQU( BOX_F,			DrawBoxF	)	/* テクスチャ無しスプライト(float精度) */
	DMAPACK_CMD_EQU( BACKUPFRAME_RECT,DrawBackupFrameRect)
						/* 現在描画中のフレームバッファを退避(領域指定) */
	DMAPACK_CMD_EQU( RENDBACKBUFFER2FRAME,DrawRendBackBuffer2Frame)
						/* DG_RendBackBuffer2Frame呼び出し */
#endif	/* DMAPACK_CMD_EQU */
