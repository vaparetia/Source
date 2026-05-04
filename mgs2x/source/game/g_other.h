/*
	g_other.c
	各キャラ固有のグローバル共有定義用ヘッダー

	2000/05/22 K.Takabe
	$Id: g_other.h,v 1.1.1.3 2002/11/19 11:41:49 Yoshizawa1 Exp $

*/
/*
	このヘッダーには各キャラ固有の定義を、プレイヤーもしくは
	その他のキャラと共有しなければならないような物を記述するように
	してください。
	また、名前は
	変数名　：GC_???
	構造体名：GCT_???
	と統一することにします。
*/

#ifndef _g_other_h_
#define _g_other_h_

/* ---------------------------------------------------------------- */
/*
	高部管理分
*/
//<Takabe>

	/*
		投影型スポットライトのパラメータ構造体
	*/
typedef struct _gct_projectionspot_spotparam{
	FVECTOR			pos ;				/* スポットライト座標 */
	FVECTOR			dir ;				/* 光源方向 */
	int				name ;				/* スポットライトキャラ名 */
	int				flag ;				/* フラグ（0:スポット光源 1:平行光源） */
	float			angle ;				/* コーン角度／２ */
	float			range ;				/* 有効距離／２ */
} GCT_ProjectionSpot_SpotParam ;

	/*
		プレイヤー側グローバル変数
		（スネーク座標判定の通知関連）
	*/
extern int							GC_ProjectionSpot_HitLights ;
extern GCT_ProjectionSpot_SpotParam	*GC_ProjectionSpot_SnakeCheckList ;



//</Takabe>
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */


#endif
