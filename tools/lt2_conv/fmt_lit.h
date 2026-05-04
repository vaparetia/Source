/*
	fmt_lit.h
	光源データ .lit フォーマット

	1999/09/27 K.Takabe
	$Id: fmt_lit.h,v 1.2 1999/09/27 12:38:11 usr02774 Exp $

*/

/*
	光源データ .lit フォーマット
*/

#ifndef	__FMT_LIT__H__
#define	__FMT_LIT__H__

typedef struct {
	short	vx, vy, vz, vw ;
} SVECTOR ;
typedef struct {
	float	vx, vy, vz, vw ;
} FVECTOR ;
typedef struct {
	unsigned char	r, g, b, pad ;
} CVECTOR ;
/*----------------------------------------------------------------*/

	/*
		光源データ
	*/
typedef	struct	{
	SVECTOR		point ;		/* 光源中心座標		*/
	u_short		r_range ;	/* 基準明度キョリ	*/
	u_short		e_range ;	/* 最大到達キョリ	*/
	CVECTOR		color ;		/* 光源色		*/
} DG_LIT ;

/*----------------------------------------------------------------*/

	/*
		.lit ヘッダ
	*/
typedef	struct	{
	u_int		n_lights ;	/* 光源数		*/
	DG_LIT		lights[ 0 ] ;	/* 光源配列		*/
} DG_LITS ;

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

	/*
		新光源データファイル（.lt2）フォーマット
	*/

	/*
		点光源データ
	*/
typedef struct {
	FVECTOR		point ;		/* 光源中心座標（vwに光源強度） */
	CVECTOR		color ;		/* 光源色 */
	float		r_range ;	/* 基準明度距離（明るさが１／２になる半径、r_range*2で明るさ０） */
	float		e_range ;	/* 最大到達距離（r_range*2が一番理想） */
	int			flag ;		/* フラグ */
} LIT_POINT ;

	/*
		スポット光源データ
	*/
typedef struct {
	FVECTOR		point ;		/* 光源中心座標（vwに光源強度） */
	FVECTOR		dir ;		/* 光源方向（vwに最大到達距離） */
	CVECTOR		color ;		/* 光源色 */
	float		umbra ;		/* スポット円錐の明部角度（ラジアン指定） */
	float		penumbra ;	/* スポット円錐の暗影部角度（penumbra >= umbra） */
	int			flag ;		/* フラグ */
} LIT_SPOT ;

	/*
		黒点光源データ
	*/
typedef struct {
	FVECTOR		bound_max ;	/* バウンディング（最大） */
	FVECTOR		bound_min ;	/* バウンディング（最小） */
	FVECTOR		point ;		/* 減衰中心座標（完全に光がなくなる座標） */
	float		r_range ;	/* 基準明度距離（明るさが１／２になる半径、r_range*2で明るさ変化なし） */
	int			flag ;		/* フラグ */
	int			pad[2] ;
} LIT_BLACKPOINT ;

	/*
		グループライトデータ
	*/
typedef struct {
	FVECTOR		bound_max ;			/* バウンディング（最大） */
	FVECTOR		bound_min ;			/* バウンディング（最小） */
	int			n_lights ;			/* グループに含まれるライト数 */
	int			type ;				/* グループ属性 */
	void		*lit ;				/* ライトデータへのポインタ */
	int			pad ;				/* リザーブ（現在は０でなければならない） */
} LIT_GRP ;

	/*
		新光源データヘッダ
	*/
typedef struct {
	FVECTOR		dir ;				/* グローバル平行光源の方向（vwに光源強度） */
	CVECTOR		color ;				/* グローバル平行光源の光源色 */
	CVECTOR		ambient ;			/* グローバル環境光 */
	int			n_lit_group ;		/* ライトグループ数 */
	int			pad ;				/* リザーブ（現在は０でなければならない） */
} LIT_DEF ;



enum {
	/* ライト属性 */
	LIT_FLAG_CHARAONLY	= 0x0100,		/* キャラクタに反映 */
	LIT_FLAG_BGONLY		= 0x0200,		/* 背景に反映 */
	LIT_FLAG_SHADOW		= 0x0400,		/* 影に反映 */
	LIT_FLAG_DISABLE	= 0x8000,		/* 光源の無効フラグ（LIT_TYPE_DYNAMIC指定時のみ有効） */
};
enum {
	/* グループの光源タイプ */
	LIT_TYPE_POINT			= 0x0001,		/* 点光源 */
	LIT_TYPE_SPOT			= 0x0002,		/* スポット光源 */
	LIT_TYPE_BLACKPOINT		= 0x0010,		/* 黒点光源 */
	LIT_TYPE_DYNAMIC		= 0x0100,		/* ダイナミックライト（個々のＯＮ、ＯＦＦに対応） */
	/*
		LIT_TYPE_POINT、LIT_TYPE_SPOT、及びLIT_TYPE_BLACKPOINTは共に排他である。
		但し、現在システムで対応している光源タイプはLIT_TYPE_POINT、LIT_TYPE_BLACKPOINTだけである。
	*/
};




/*
	光源データ（.lt2）フォーマットについて


	＜旧フォーマットとの違い＞
	処理時間を短縮するためにステージ全体のライトを複数の領域に
	分けることができるフォーマットになっている。また、キャラ、背景
	別々にライトを指定できるように属性で対象を選択できるようになった。


	＜光源強度について＞
	キャラクタには最高３つまでの光源しか反映させることができないため、
	有効な光源を検索するために光源強度というパラメータを用意した。
	この値は白色のときに最高値の1.0になる。また、点光源ではは距離が遠くなる
	ほどこの光源強度も低下する。

	光源強度 = ( red * 0.30 + green * 0.59 + blue * 0.11 ) / 255.0


	＜点光源について＞
	点光源はその点光源から(e_range,e_range,e_range)-(-e_range,-e_range,-e_range)を
	バウンディングボックスとした領域内において有効で、r_range*2の距離において光の
	影響が０になるように線形で減衰する光源である。（ＭＧＳ１と同じもの）

	・点光源の計算方法
	１） ２点から方向ベクトルを算出
	FV_LightVector = FV_VertexPos - FV_PointLightPos ;
	２） ２点の距離が( r_range * 2.0 ) の時に１となるような減衰値を算出
	Scale = ( r_range * 2.0 - FUNC_Length( FV_LightVector ) ) / ( r_range * 2.0 );
	Scale = ( Scale < 0 ) ? 0 : Scale ;
	３） 減衰値によって光源強度も補正
	Force = FV_PointLightPos.vw * Scale ;
	４） 正規化した方向ベクトルに減衰値を乗算
	FV_LightVector = FUNC_Normalize( FV_LightVector ) * Scale ;
	この FV_LightVector を用いて平行光源計算を行う


	＜スポット光源について＞
	スポット光源は光源位置から光源方向で指定した方向に対して円錐形で指定され、
	明るさが変化しない明部領域と徐々に光が減衰する暗影部領域を角度で指定する。
	現在未サポート。


	＜黒点光源について＞
	黒点光源とはゲーム用に特化した特殊ライトであり、通常のライトとは異なる。
	黒点光源の有効領域はバウンディングボックスで指定され、頂点に対して有効な３光源が
	決定した後に黒点光源が有効であれば減衰中心座標からの距離で光源の明るさが補正される。
	r_range*2以上で無変化、r_rangeで本来の１／２、距離０で完全に黒くなる。
	また、黒光源同士の領域は重なってはいけない。重なった場合は検索時も先に見つかった
	ものが有効になる。


	＜データ格納順番について＞
	[LIT_DEF]
	[LIT_GRP] × LIT_DEF.n_lit_group
	[LIT_POINT]or[LIT_SPOT]or[LIT_BLACKPOINT] × LIT_GRP[0].n_lights
	[LIT_POINT]or[LIT_SPOT]or[LIT_BLACKPOINT] × LIT_GRP[1].n_lights
	.
	.
	.
	[LIT_POINT]or[LIT_SPOT]or[LIT_BLACKPOINT] × LIT_GRP[LIT_DEF.n_lit_group-1].n_lights


	又、現在システムでは点光源しか対応していないので注意


*/





#endif

