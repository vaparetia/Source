/*
   libhzd.cnf
   ＨＺＤライブラリ内部コンフィギュレーション

   1999/07/07 M.Sonoyama
   $Id: private.h,v 1.1.1.3 2002/11/19 11:42:49 Yoshizawa1 Exp $			

  （外部定数については libhzd.h を参照）
*/

/*----------------------------------------------------------------*/

	/*
		バッファサイズ
	*/
#define	MAX_HAZARDS	(128)
#define	MAX_BINDS	(64)

	/*
		ゾーン直方体の高さ
	*/
#define	ZONE_HEIGHT			(2000.0F)
#define	ZONE_HEIGHT_UNDER	(-1000.0F)

#define	ZONE_LOW_HEIGHT			(500.0F)
#define	ZONE_LOW_HEIGHT_UNDER	(-10.0F)

	/*
		道順ナビゲート処理／最大ノード数
	*/
#define	MAX_ROUTE	(255)

/*----------------------------------------------------------------*/
