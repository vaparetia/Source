/*
	lowpoly.h
	    ローポリ劇場用ヘッダファイル

	1999/09/03 Y.Matsuhana
	$Id: lowpoly.h,v 1.49 2002/10/04 02:12:35 usr03379 Exp $


*/

// ファイルが二重呼びされたときの対処
#ifndef d:LOWPOLY_H
#define LOWPOLY_H	1

#define DEMO1	1		//	タンカー編オープニング
#define DEMO2	2		//	タンカー乗っ取りデモ
#define DEMO3	3		//	オルガＶＳスネーク
#define DEMO4	4		//	ＲＡＹ強奪
#define DEMO5	5		//	ヴァンプ遭遇
#define DEMO6	6		//	フォーチュン遭遇
#define DEMO7	7		//	量産ＲＡＹvsソリダス
#define DEMO8	8		//	ソリダスの最後

#define ORI		0		//	もとのまま
#define RAI		1		//	ライデン
#define SOL		2		//	ソリダス
#define SNA		3		//	スネーク
#define REV		4		//	オセロット
#define VMP		5		//	ヴァンプ
#define FOR		6		//	フォーチュン
#define ROS		7		//	ローズ
#define MGS1S	8		//	ＭＧＳ１スネーク
#define MGS1R	9		//	ＭＧＳ１オセロット
#define BUS		10		//	ビジネスマン
#define OL		11		//	ＯＬ
#define OBA		12		//	おばちゃん
#define HTC		13		//	ハイテク兵
#define NIN		14		//	忍者

//	追加キャラ
#define OTA		15		//	オタコン
#define MRL		16		//	メリル
#define TXD		17		//	タキシードスネーク
#define SNAO	18		//	ハイクオリティＭＧＳ１スネーク
#define NRAI	19		//	忍者ライデン



//#########################################################################################
//	キャストセレクト部分関数

	//	共通
proc キャストセレクト共通 {
	//	ビジネスマン
	if ($1 == d:BUS) {
		eval ($s:新キャスト[ $2 ] = cit_maley_ctg_sh)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	ＯＬ
	} else if ($1 == d:OL) {
		eval ($s:新キャスト[ $2 ] = cit_femaled_def)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	おばちゃん
	} else if ($1 == d:OBA) {
		eval ($s:新キャスト[ $2 ] = cit_femalee_def)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	ハイテク兵
	} else if ($1 == d:HTC) {
		eval ($s:新キャスト[ $2 ] = htc_def_mt)
		eval ($s:新モデル種類[ $2 ] = kms)
	}

}

	//	ＫＭＳ用
proc キャストセレクトＫＭＳ {
	//	そのまま
	if ($1 == d:ORI) {
		eval ($s:新キャスト[ $2 ] = ori_mdl)

	//	ライデン
	} else if ($1 == d:RAI) {
		eval ($s:新キャスト[ $2 ] = rai_def_sh_mt)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	スネーク
	} else if ($1 == d:SNA) {
		eval ($s:新キャスト[ $2 ] = sna_def_sh)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	フォーチュン
	} else if ($1 == d:FOR) {
		eval ($s:新キャスト[ $2 ] = for_def_sh_mt)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	ＭＧＳ１スネーク
	} else if ($1 == d:MGS1S) {
		eval ($s:新キャスト[ $2 ] = sna_mgs1)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	ＭＧＳ１オセロット
	} else if ($1 == d:MGS1R) {
		eval ($s:新キャスト[ $2 ] = rev_mgs1)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	ＭＧＳ１オセロット
	} else if ($1 == d:MGS1R) {
		eval ($s:新キャスト[ $2 ] = rev_mgs1)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	メリル
	} else if ($1 == d:MRL) {
		eval ($s:新キャスト[ $2 ] = mrl_def_sh_mt)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	タキシードスネーク
	} else if ($1 == d:TXD) {
		eval ($s:新キャスト[ $2 ] = sna_txd_sh_mt)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	ハイクオリティＭＧＳ１スネーク
	} else if ($1 == d:SNAO) {
		eval ($s:新キャスト[ $2 ] = sna_oss_sh_mt)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	忍者ライデン
	} else if ($1 == d:NRAI) {
		eval ($s:新キャスト[ $2 ] = rai_nin_sh_mt)
		eval ($s:新モデル種類[ $2 ] = kms)

	//	上記以外の場合
	} else {
		//	共通部分
		@キャストセレクト共通 $1 $2
	}
}

	//	ＥＶＭ用
proc キャストセレクトＥＶＭ {
	//	そのまま
	if ($1 == d:ORI) {
		eval ($s:新キャスト[ $2 ] = ori_mdl)

	//	ライデン
	} else if ($1 == d:RAI) {
		eval ($s:新キャスト[ $2 ] = rai_def_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	ソリダス
	} else if ($1 == d:SOL) {
		eval ($s:新キャスト[ $2 ] = sol_def_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	スネーク
	} else if ($1 == d:SNA) {
		eval ($s:新キャスト[ $2 ] = sna_def_mh)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	オセロット
	} else if ($1 == d:REV) {
		eval ($s:新キャスト[ $2 ] = rev_def_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	ヴァンプ
	} else if ($1 == d:VMP) {
		eval ($s:新キャスト[ $2 ] = vmp_coat_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	フォーチュン
	} else if ($1 == d:FOR) {
		eval ($s:新キャスト[ $2 ] = for_def_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	ローズ
	} else if ($1 == d:ROS) {
		eval ($s:新キャスト[ $2 ] = ros_def_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	ＭＧＳ１スネーク
	} else if ($1 == d:MGS1S) {
		eval ($s:新キャスト[ $2 ] = sna_mgs1_mh)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	ＭＧＳ１オセロット
	} else if ($1 == d:MGS1R) {
		eval ($s:新キャスト[ $2 ] = rev_mgs1_mh)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	忍者
	} else if ($1 == d:NIN) {
		eval ($s:新キャスト[ $2 ] = org_tng_pal_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	オタコン
	} else if ($1 == d:OTA) {
		eval ($s:新キャスト[ $2 ] = otc_def_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	メリル
	} else if ($1 == d:MRL) {
		eval ($s:新キャスト[ $2 ] = mrl_def_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	タキシードスネーク
	} else if ($1 == d:TXD) {
		eval ($s:新キャスト[ $2 ] = sna_txd_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	ハイクオリティＭＧＳ１スネーク
	} else if ($1 == d:SNAO) {
		eval ($s:新キャスト[ $2 ] = sna_oss_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	忍者ライデン
	} else if ($1 == d:NRAI) {
		eval ($s:新キャスト[ $2 ] = rai_nin_mh_mt)
		eval ($s:新モデル種類[ $2 ] = evm)

	//	上記以外の場合
	} else {
		//	共通部分
		@キャストセレクト共通 $1 $2
	}
}





//#########################################################################################
//	キャスト代入部分関数

	//	無理矢理体型を当てはめる場合
proc キャスト代入 {
	//	変更しない場合以外は
	if ($s:新キャスト[ $1 ] != ori_mdl) {
		command デモローポリ劇場 \
			-change	$s:元キャスト[ $1 ]	$s:元モデル種類[ $1 ]	\
					$s:新キャスト[ $1 ]	$s:新モデル種類[ $1 ]
	}

}

	//	そのまま出す場合	ＥＶＭ用
proc キャスト代入ＥＶＭ（リネーム） {
	//	変更しない場合以外は
	if ($s:新キャスト[ $1 ] != ori_mdl) {

		//	描画モード調整用
		@描画モード調整 $1

	//	ビジネスマン／ＯＬ／おばちゃん／ハイテク兵はＫＭＳのみ

		//	ビジネスマンに
		if ($s:新キャスト[ $1 ] == cit_maley_ctg_sh) {
			command デモローポリ劇場 \
				-change	$s:元キャスト[ $1 ]	$s:元モデル種類[ $1 ]	\
						$s:新キャスト[ $1 ]	$s:新モデル種類[ $1 ]

		//	ＯＬに
		} else if ($s:新キャスト[ $1 ] == cit_femaled_def) {
			command デモローポリ劇場 \
				-change	$s:元キャスト[ $1 ]	$s:元モデル種類[ $1 ]	\
						$s:新キャスト[ $1 ]	$s:新モデル種類[ $1 ]	\
				-f $$i:描画モードフラグ									//	ＯＬは描画を後にするので必要

		//	おばちゃんに
		} else if ($s:新キャスト[ $1 ] == cit_femalee_def) {
			command デモローポリ劇場 \
				-change	$s:元キャスト[ $1 ]	$s:元モデル種類[ $1 ]	\
						$s:新キャスト[ $1 ]	$s:新モデル種類[ $1 ]

		//	ハイテク兵に
		} else if ($s:新キャスト[ $1 ] == htc_def_mt) {
			command デモローポリ劇場 \
				-change	$s:元キャスト[ $1 ]	$s:元モデル種類[ $1 ]	\
						$s:新キャスト[ $1 ]	$s:新モデル種類[ $1 ]

		//	それ以外は
		} else {
			command デモモデルリネーム追加 \
				-rename_evm	$s:元キャスト[ $1 ]	\
							$s:新キャスト[ $1 ] \
				-f $$i:描画モードフラグ

		}

	}

}

	//	そのまま出す場合ＫＭＳ用
proc キャスト代入ＫＭＳ（リネーム） {
	//	変更しない場合以外は
	if ($s:新キャスト[ $1 ] != ori_mdl) {

		//	描画モード調整用
		@描画モード調整 $1

		command デモモデルリネーム追加 \
			-rename_kms	$s:元キャスト[ $1 ]	\
						$s:新キャスト[ $1 ]	\
			-f $$i:描画モードフラグ
	}

}





//#########################################################################################
//	描画モード調整関数
proc 描画モード調整 {
	//	フォーチュン遭遇、またはソリダスの最後のときは
	if ( ($s:エリア == d012p01) || ($s:エリア == d082p01) ) {

		//	キャストが以下のモデルに変わったときは
		if (	($s:新キャスト[ $1 ] == sna_def_sh) || \			//	KMSスネーク
				($s:新キャスト[ $1 ] == sna_def_mh) || \			//	EVMスネーク
				($s:新キャスト[ $1 ] == sna_def_addhand_mh_mt) || \	//	EVM指付きスネーク
				($s:新キャスト[ $1 ] == cit_femaled_def) || \		//	KMSＯＬ
				($s:新キャスト[ $1 ] == otc_def_mh_mt) || \			//	オタコン
				($s:新キャスト[ $1 ] == sna_txd_sh_mt) || \			//	KMSタキシードスネーク
				($s:新キャスト[ $1 ] == sna_txd_mh_mt) || \			//	EVMタキシードスネーク
				($s:新キャスト[ $1 ] == sna_oss_sh_mt) || \			//	EVMハイクオリティＭＧＳ１スネーク
				($s:新キャスト[ $1 ] == sna_oss_mh_mt)	) {			//	KMSハイクオリティＭＧＳ１スネーク

			//	髪の毛がフォグの影響を受けるのであとから描画する
			eval ($$i:描画モードフラグ = 1)

		//	その他のモデルは何もしない
		} else {
			//	フラグを戻す
			eval ($$i:描画モードフラグ = 0)
		}

	//	それ以外のデモは何もしない
	} else {
			//	フラグを戻す
		eval ($$i:描画モードフラグ = 0)
	}

}





//#########################################################################################
//	メインセレクト部分関数（これがプログラム側から呼ばれる）

proc メインセレクト $:デモ $:新キャスト１ $:新キャスト２ $:新キャスト３ $:新キャスト４ {
	//	ローポリ劇場フラグを立てる
	eval ($f:ローポリ劇場フラグ = d:TRUE)

	//	ストーリーフラグを初期化しておく
	eval ($w:t_story = 0);
	eval ($w:p_story = 0);
	eval ($f:モノローグチェック = 0);

	//	髭剃りフラグも初期化しておく
	eval ($w:プレイヤーフラグ = $w:プレイヤーフラグ & ~d:PL_GAVE_SHAVER_TO_SNAKE);

	// サングラスフラグを寝かしておく
	eval( $w:クリア後フラグ = $w:クリア後フラグ & ~d:CLEARED_SUNGLASSES_PLAYING );

	//	描画モードフラグを寝かしておく
	eval ($$i:描画モードフラグ = 0)

//#########################################################################################
	//	タンカー編オープニング
	if ($:デモ == d:DEMO1) {
		//	スネークを変更する
			//	タキシードスネークはここでは裾つきsna_txd_demo_sh_mt.kmsを使用する
		if ($:新キャスト１ == d:TXD) {
			eval ($s:新キャスト[ 1 ] = sna_txd_demo_sh_mt)
			eval ($s:新モデル種類[ 1 ] = kms)

			//	忍者ライデンはここではフェイスガードつきrai_nin_sh_mt_fg.kmsを使用する
		} else if ($:新キャスト１ == d:NRAI) {
			eval ($s:新キャスト[ 1 ] = rai_nin_sh_mt_fg)
			eval ($s:新モデル種類[ 1 ] = kms)

			//	それ以外は
		} else {
			@キャストセレクトＫＭＳ $2 1
		}

		//	オセロットを変更する
			//	スネークはここではsna_def_mh.evmが無いので、sna_dive_sh_mt.kmsを使用する
		if ($:新キャスト２ == d:SNA) {
			eval ($s:新キャスト[ 2 ] = sna_dive_sh_mt)
			eval ($s:新モデル種類[ 2 ] = kms)

			//	それ以外は
		} else {
			@キャストセレクトＥＶＭ $3 2
		}

		//	ロード関数
		@mv_lowpoly_t00a1D

//#########################################################################################
//	タンカー乗っ取りデモ
	} else if ($:デモ == d:DEMO2) {
		//	スネークを変更する
		@キャストセレクトＥＶＭ $2 1

		//	ゴル兵を変更する
		@キャストセレクトＫＭＳ $3 2

		//	ロード関数
		@mv_lowpoly_t04a1D

//#########################################################################################
//	オルガＶＳスネーク
	} else if ($:デモ == d:DEMO3) {
		//	スネークを変更する
		@キャストセレクトＥＶＭ $2 1

		//	オルガを変更する
		@キャストセレクトＥＶＭ $3 2

		//	ゴル大佐を変更する
		@キャストセレクトＥＶＭ $4 3

		//	ロード関数
		@mv_lowpoly_t05a1D

//#########################################################################################
//	ＲＡＹ強奪
	} else if ($:デモ == d:DEMO4) {
		//	スネークを変更する
		@キャストセレクトＥＶＭ $2 1

		//	オセロットを変更する
		@キャストセレクトＥＶＭ $3 2

		//	スコットを変更する
			//	オセロットは指付きを使用する
		if ($:新キャスト３ == d:REV) {
			eval ($s:新キャスト[ 3 ] = rev_def_addhand_mh_mt)
			eval ($s:新モデル種類[ 3 ] = evm)

			//	フォーチュンは指付きを使用する
		} else if ($:新キャスト３ == d:FOR) {
			eval ($s:新キャスト[ 3 ] = for_last_addhand_mh_mt)
			eval ($s:新モデル種類[ 3 ] = evm)

			//	それ以外は
		} else {
			@キャストセレクトＥＶＭ $4 3
		}

		//	ゴル大佐を変更する
		@キャストセレクトＥＶＭ $5 4

		//	ロード関数
		@mv_lowpoly_t12a1D

//#########################################################################################
	//	ヴァンプ遭遇
	} else if ($:デモ == d:DEMO5) {
		//	ライデンを変更する
		@キャストセレクトＥＶＭ $2 1

		//	ヴァンプを変更する
		@キャストセレクトＥＶＭ $3 2

		//	シールズを変更する
		@キャストセレクトＫＭＳ $4 3

		//	プリスキンを変更する
		@キャストセレクトＥＶＭ $5 4

		//	ロード関数
		@mv_lowpoly_p010_01_p01

//#########################################################################################
	//	フォーチュン遭遇
	} else if ($:デモ == d:DEMO6) {
		//	ライデンを変更する
			//	フォーチュンはここではfor_def_mh_mt.evmがないので、for_coat_mh_mt.evmを使用する
		if ($:新キャスト１ == d:FOR) {
			eval ($s:新キャスト[ 1 ] = for_coat_mh_mt)
			eval ($s:新モデル種類[ 1 ] = evm)

		//	それ以外は
		} else {
			@キャストセレクトＥＶＭ $2 1
		}

		//	ヴァンプを変更する
			//	フォーチュンはここではfor_def_mh_mt.evmがないので、for_coat_mh_mt.evmを使用する
		if ($:新キャスト２ == d:FOR) {
			eval ($s:新キャスト[ 2 ] = for_coat_mh_mt)
			eval ($s:新モデル種類[ 2 ] = evm)

		//	それ以外は
		} else {
			@キャストセレクトＥＶＭ $3 2
		}

		//	フォーチュンを変更する
			//	オセロットは指付きを使用する
		if ($:新キャスト３ == d:REV) {
			eval ($s:新キャスト[ 3 ] = rev_def_addhand_mh_mt)
			eval ($s:新モデル種類[ 3 ] = evm)

		//	それ以外は
		} else {
			@キャストセレクトＥＶＭ $4 3
		}

		//	シールズを変更する
		@キャストセレクトＫＭＳ $5 4

		//	ロード関数
		@mv_lowpoly_p012_01_p01

//#########################################################################################
	//	量産ＲＡＹvsソリダス
	} else if ($:デモ == d:DEMO7) {
		//	ソリダスを変更する
			//	ヴァンプはここではvmp_coat_mh_mt.evmがないので、vmp_naked_mh_mt.evmを使用する
		if ($:新キャスト１ == d:VMP) {
			eval ($s:新キャスト[ 1 ] = vmp_naked_mh_mt)
			eval ($s:新モデル種類[ 1 ] = evm)

			//	それ以外は
		} else {
			@キャストセレクトＥＶＭ $2 1
		}

		//	ロード関数
		@mv_lowpoly_p080_03_p02

//#########################################################################################
	//	ソリダスの最後
	} else if ($:デモ == d:DEMO8) {
		//	ライデンを変更する
			//	ソリダスは指付きを使用する
		if ($:新キャスト１ == d:SOL) {
			eval ($s:新キャスト[ 1 ] = sol_def_addhand_mh_mt)
			eval ($s:新モデル種類[ 1 ] = evm)

			//	スネークは指付きを使用する
		} else if ($:新キャスト１ == d:SNA) {
			eval ($s:新キャスト[ 1 ] = sna_def_addhand_mh_mt)
			eval ($s:新モデル種類[ 1 ] = evm)

			//	ヴァンプはここではvmp_coat_mh_mt.evmがないので、vmp_naked_mh_mt.evmを使用する
		} else if ($:新キャスト１ == d:VMP) {
			eval ($s:新キャスト[ 1 ] = vmp_naked_mh_mt)
			eval ($s:新モデル種類[ 1 ] = evm)

			//	フォーチュンは指付きを使用する
		} else if ($:新キャスト１ == d:FOR) {
			eval ($s:新キャスト[ 1 ] = for_last_addhand_mh_mt)
			eval ($s:新モデル種類[ 1 ] = evm)

			//	それ以外は
		} else {
			@キャストセレクトＥＶＭ $2 1
		}

		//	ソリダスを変更する
			//	ライデンは指付きを使用する
		if ($:新キャスト２ == d:RAI) {
			eval ($s:新キャスト[ 2 ] = rai_def_addhand_mh_mt)
			eval ($s:新モデル種類[ 2 ] = evm)

			//	スネークは指付きを使用する
		} else if ($:新キャスト２ == d:SNA) {
			eval ($s:新キャスト[ 2 ] = sna_def_addhand_mh_mt)
			eval ($s:新モデル種類[ 2 ] = evm)

			//	ヴァンプはここではvmp_coat_mh_mt.evmがないので、vmp_naked_mh_mt.evmを使用する
		} else if ($:新キャスト２ == d:VMP) {
			eval ($s:新キャスト[ 2 ] = vmp_naked_mh_mt)
			eval ($s:新モデル種類[ 2 ] = evm)

			//	フォーチュンは指付きを使用する
		} else if ($:新キャスト２ == d:FOR) {
			eval ($s:新キャスト[ 2 ] = for_last_addhand_mh_mt)
			eval ($s:新モデル種類[ 2 ] = evm)

			//	それ以外は
		} else {
			@キャストセレクトＥＶＭ $3 2
		}

		//	ロード関数
		@mv_lowpoly_p082_01_p01

	}

}





//#########################################################################################
//各.gcl内で呼ばれる関数	ここで実際にモデルが置き換わる

proc ローポリ設定 {

	if ($f:ローポリ劇場フラグ == d:TRUE) {

	/*
		ローポリ劇場と認識させるには"command デモローポリ劇場"を起動しなければならない
		ゆえにとりあえず存在しないモデルで起動しておく
		(defaultのままでは、"command デモローポリ劇場"を通らないためである)
	*/
		command デモローポリ劇場 \
			-change AAA kms BBB kms

		//	描画モードフラグを寝かしておく
		eval ($$i:描画モードフラグ = 0)

//#########################################################################################
	//	タンカー編オープニング
		if ($s:エリア == d00t) {
		///////////////////////////////////////////////////////////////////////////スネーク
		//	雨合羽スネーク		sna_coat_mh_mt.evm
		//	光学迷彩スネーク	sna_dive_sh_mt.kms		光学迷彩はKMSのみ
			eval ($s:元キャスト[ 1 ] = sna_dive_sh_mt)
			eval ($s:元モデル種類[ 1 ] = kms)

				@キャスト代入ＫＭＳ（リネーム） 1

		///////////////////////////////////////////////////////////////////////////オセロット
		//	オセロット				rev_def_mh_mt.evm
			eval ($s:元キャスト[ 2 ] = rev_def_mh_mt)
			eval ($s:元モデル種類[ 2 ] = evm)

				//	スネークはsna_dive_sh_mt.kmsを使用する
			if ($s:新キャスト[ 2 ] == sna_dive_sh_mt) {
					command デモローポリ劇場 \
						-change	rev_def_mh_mt	evm	\
								sna_dive_sh_mt	kms

				//	ライデンに
			} else if ($s:新キャスト[ 2 ] == rai_def_mh_mt) {
				@キャスト代入 2

				//	フォーチュンに
			} else if ($s:新キャスト[ 2 ] == for_def_mh_mt) {
				@キャスト代入 2

				//	ローズに
			} else if ($s:新キャスト[ 2 ] == ros_def_mh_mt) {
				@キャスト代入 2

				//	忍者に
			} else if ($s:新キャスト[ 2 ] == org_tng_pal_mh_mt) {
				@キャスト代入 2

				//	オタコンに
			} else if ($s:新キャスト[ 2 ] == otc_def_mh_mt) {
				@キャスト代入 2

				//	忍者ライデンに
			} else if ($s:新キャスト[ 2 ] == rai_nin_mh_mt) {
				@キャスト代入 2

				//	それ以外は
			} else {
				@キャスト代入ＥＶＭ（リネーム） 2
			}

//#########################################################################################
	//	タンカー乗っ取りデモ
		} else if ($s:エリア == d04t) {
		///////////////////////////////////////////////////////////////////////////スネーク
		//	スネーク				sna_def_mh.evm
			eval ($s:元キャスト[ 1 ] = sna_def_mh)
			eval ($s:元モデル種類[ 1 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 1

		///////////////////////////////////////////////////////////////////////////ゴル兵
		//	ゴル兵					gbs_def_mt.kms
			eval ($s:元キャスト[ 2 ] = gbs_def_mt)
			eval ($s:元モデル種類[ 2 ] = kms)

			@キャスト代入ＫＭＳ（リネーム） 2

//#########################################################################################
	//	オルガＶＳスネーク
		} else if ($s:エリア == d05t) {
		///////////////////////////////////////////////////////////////////////////スネーク
		//	スネーク				sna_def_mh.evm
			eval ($s:元キャスト[ 1 ] = sna_def_mh)
			eval ($s:元モデル種類[ 1 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 1

		///////////////////////////////////////////////////////////////////////////オルガ
		//	オルガ					org_def.evm
		//	オルガ（指付き）	org_def_addhand_mh_mt.evm
		//	オルガ（映り込み）	org_sgl.kms
			//	オリジナル以外は帽子をとる
			if ($s:新キャスト[ 2 ] != ori_mdl) {
				command デモローポリ劇場 \
					-change	demo_org_hut	kms	\
							null			kms
			} 

/*	装備一覧
ホルスタｰ
org_gbhlst.mdl
ナイフのカバー
org_sk_cover.mdl
ナイフ
org_sk_knife.mdl
オルガ無線機
org_radio_forhand.mdl
USP
demo_usp
*/

//	とりあえずＵＳＰ出しておく
#if 0
			if ($s:新キャスト[ 2 ] == mrl_def_mh_mt)	{
				//	USP
				command デモローポリ劇場 \
					-change	demo_usp	kms	\
							null			kms

			}
#endif

			eval ($s:元キャスト[ 2 ] = org_def)
			eval ($s:元モデル種類[ 2 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 2

		//	追加処理		オルガ（指付き）も変更する
			//	変更しない場合以外は
			if ($s:新キャスト[ 2 ] != ori_mdl) {

				//	追加処理中の例外処理
					//	スネークは指付きも使用する
				if ($s:新キャスト[ 2 ] == sna_def_mh) {
					command デモモデルリネーム追加 \
						-rename_evm	org_def_addhand_mh_mt	\
									sna_def_addhand_mh_mt

//	フォーチュンは無くなった
#if 0
					//	フォーチュンは指付きも使用する
				} else if ($s:新キャスト[ 2 ] == for_def_mh_mt) {
					command デモモデルリネーム追加 \
						-rename_evm	org_def_addhand_mh_mt	\
									for_last_addhand_mh_mt
#endif

				//	ビジネスマンに
				} else if ($s:新キャスト[ 2 ] == cit_maley_ctg_sh) {
					command デモローポリ劇場 \
						-change	org_def_addhand_mh_mt	evm	\
								$s:新キャスト[ 2 ]	$s:新モデル種類[ 2 ]

				//	ＯＬに
				} else if ($s:新キャスト[ 2 ] == cit_femaled_def) {
					command デモローポリ劇場 \
						-change	org_def_addhand_mh_mt	evm	\
								$s:新キャスト[ 2 ]	$s:新モデル種類[ 2 ]

				//	おばちゃんに
				} else if ($s:新キャスト[ 2 ] == cit_femalee_def) {
					command デモローポリ劇場 \
						-change	org_def_addhand_mh_mt	evm	\
								$s:新キャスト[ 2 ]	$s:新モデル種類[ 2 ]

				//	ハイテク兵に
				} else if ($s:新キャスト[ 2 ] == htc_def_mt) {
					command デモローポリ劇場 \
						-change	org_def_addhand_mh_mt	evm	\
								$s:新キャスト[ 2 ]	$s:新モデル種類[ 2 ]

				//	それ以外は
				} else {
					command デモモデルリネーム追加 \
						-rename_evm	org_def_addhand_mh_mt	\
									$s:新キャスト[ 2 ]
				}
			}

		///////////////////////////////////////////////////////////////////////////ゴル大佐
		//	ゴル大佐				crg_def_mh_mt.evm
		//	ゴル大佐（帽子）	crg_hut_mt.kms
			//	デフォルトでこれを立てておかないと落ちる！
			command デモローポリ劇場 \
				-equip	crg_def_mh_mt	\
						d:DM_EQUIP_NOTHING
		
			//	オリジナル以外は帽子を切る
			if ($s:新キャスト[ 3 ] != ori_mdl) {
				command デモローポリ劇場 \
					-change	crg_hut_mt	kms	\
							null		kms
			} 

			eval ($s:元キャスト[ 3 ] = crg_def_mh_mt)
			eval ($s:元モデル種類[ 3 ] = evm)


				//	タキシードスネークは、sna_txd_sh_mt.kmsを使用する
			if ($s:新キャスト[ 3 ] == sna_txd_sh_mt) {
					command デモローポリ劇場 \
						-change	crg_def_mh_mt	evm	\
								sna_txd_sh_mt	kms

				//	忍者ライデンは、rai_nin_sh_mt.kmsを使用する
			} else if ($s:新キャスト[ 3 ] == rai_nin_sh_mt) {
					command デモローポリ劇場 \
						-change	crg_def_mh_mt	evm	\
								rai_nin_sh_mt	kms

				//	それ以外は
			} else {
				@キャスト代入ＥＶＭ（リネーム） 3
			}

//#########################################################################################
	//	ＲＡＹ強奪
		} else if ($s:エリア == d12t) {
		///////////////////////////////////////////////////////////////////////////スネーク
		//	スネーク				sna_def_mh.evm
			eval ($s:元キャスト[ 1 ] = sna_def_mh)
			eval ($s:元モデル種類[ 1 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 1

		///////////////////////////////////////////////////////////////////////////オセロット
		//	オセロット				rev_coat.evm
			//	オリジナル以外は銃をとる
			if ($s:新キャスト[ 2 ] != ori_mdl) {
				command デモローポリ劇場 \
					-change	saa		kms	\
							null	kms

//	とりあえず出しておく
#if 0
				//	フォーチュンは
				if ($s:新キャスト[ 2 ] == for_def_mh_mt) {
					//	起爆装置をとる
					command デモローポリ劇場 \
						-change	rev_dtdv	kms	\
								null		kms
				}
#endif
			}

			eval ($s:元キャスト[ 2 ] = rev_coat)
			eval ($s:元モデル種類[ 2 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 2

		///////////////////////////////////////////////////////////////////////////スコット
		//	スコット				sco_def_addhand_mh_mt.evm
			//	オリジナル以外は帽子をとる
			if ($s:新キャスト[ 3 ] != ori_mdl) {
				command デモローポリ劇場 \
					-change	sco_hat	kms	\
							null	kms
			}

			eval ($s:元キャスト[ 3 ] = sco_def_addhand_mh_mt)
			eval ($s:元モデル種類[ 3 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 3

		///////////////////////////////////////////////////////////////////////////ゴル大佐
		//	ゴル大佐				crg_def_addhand_mh_mt.evm
			eval ($s:元キャスト[ 4 ] = crg_def_addhand_mh_mt)
			eval ($s:元モデル種類[ 4 ] = evm)

			//	オタコン
			if ($s:新キャスト[ 4 ] == otc_def_mh_mt) {
				@キャスト代入ＥＶＭ（リネーム） 4

			//	それ以外は
			} else {
				@キャスト代入 4
			}

//#########################################################################################
	//	ヴァンプ遭遇
		} else if ($s:エリア == d010p01) {
		///////////////////////////////////////////////////////////////////////////ライデン
		//	ライデン				rai_def_mh_mt.evm
		//	ライデン（頬傷）		rai_kizu_face_mh.evm
			//	オリジナル以外はライデン（頬傷）をとる
			if ($s:新キャスト[ 1 ] != ori_mdl) {
				command デモローポリ劇場 \
					-change	rai_kizu_face_mh	evm	\
							null				kms
			}

			eval ($s:元キャスト[ 1 ] = rai_def_mh_mt)
			eval ($s:元モデル種類[ 1 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 1

		///////////////////////////////////////////////////////////////////////////ヴァンプ
		//	ヴァンプ				vmp_coat_mh_mt.evm
		//	ヴァンプ(胸傷)		vmp_kizu_mh.evm
		//	ヴァンプ（影）		vmp_coat_shadow.kms
			//	オリジナル以外はヴァンプ(胸傷)をとる
			if ($s:新キャスト[ 2 ] != ori_mdl) {
				command デモローポリ劇場 \
					-change	vmp_kizu_mh	evm	\
							null		kms
			}

			eval ($s:元キャスト[ 2 ] = vmp_coat_mh_mt)
			eval ($s:元モデル種類[ 2 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 2

		///////////////////////////////////////////////////////////////////////////シールズ
		//	シールズ				sel_def.kms
		//	シールズ（血染め）		sel_blood_sh.kms
		//	シールズ（驚き顔）		sel_def_sh.kms
			eval ($s:元キャスト[ 3 ] = sel_def)
			eval ($s:元モデル種類[ 3 ] = kms)

			@キャスト代入ＫＭＳ（リネーム） 3

		//	追加処理		シールズ（血染め）／シールズ（驚き顔）も変更する
			//	変更しない場合以外は
			if ($s:新キャスト[ 3 ] != ori_mdl) {
				command デモモデルリネーム追加 \
					-rename_kms	sel_blood_sh	\
								$s:新キャスト[ 3 ]
				command デモモデルリネーム追加 \
					-rename_kms	sel_def_sh	\
								$s:新キャスト[ 3 ]
			}

		///////////////////////////////////////////////////////////////////////////プリスキン
		//	プリスキン				iro_skull_mh_mt.evm
		//	プリスキン（腕に血）	iro_blood_mh_mt.evm
			//	以下のキャラはヘッドホンとる
			if ($s:新キャスト[ 4 ] == rai_def_mh_mt) {
				command デモローポリ劇場 \
					-change	iro_skull_phones	kms	\
							null				kms
			} else if ($s:新キャスト[ 4 ] == for_def_mh_mt) {
				command デモローポリ劇場 \
					-change	iro_skull_phones	kms	\
							null				kms
			} else if ($s:新キャスト[ 4 ] == org_tng_pal_mh_mt) {
				command デモローポリ劇場 \
					-change	iro_skull_phones	kms	\
							null				kms
			} else if ($s:新キャスト[ 4 ] == otc_def_mh_mt) {
				command デモローポリ劇場 \
					-change	iro_skull_phones	kms	\
							null				kms
			} else if ($s:新キャスト[ 4 ] == rai_nin_mh_mt) {
				command デモローポリ劇場 \
					-change	iro_skull_phones	kms	\
							null				kms
			}

			eval ($s:元キャスト[ 4 ] = iro_skull_mh_mt)
			eval ($s:元モデル種類[ 4 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 4

		//	追加処理		プリスキン（腕に血）も変更する
			//	変更しない場合以外は
			if ($s:新キャスト[ 4 ] != ori_mdl) {
				//	ビジネスマンに
				if ($s:新キャスト[ 4 ] == cit_maley_ctg_sh) {
					command デモローポリ劇場 \
						-change	iro_blood_mh_mt	evm	\
								$s:新キャスト[ 4 ]	$s:新モデル種類[ 4 ]

				//	ＯＬに
				} else if ($s:新キャスト[ 4 ] == cit_femaled_def) {
					command デモローポリ劇場 \
						-change	iro_blood_mh_mt	evm	\
								$s:新キャスト[ 4 ]	$s:新モデル種類[ 4 ]

				//	おばちゃんに
				} else if ($s:新キャスト[ 4 ] == cit_femalee_def) {
					command デモローポリ劇場 \
						-change	iro_blood_mh_mt	evm	\
								$s:新キャスト[ 4 ]	$s:新モデル種類[ 4 ]

				//	ハイテク兵に
				} else if ($s:新キャスト[ 4 ] == htc_def_mt) {
					command デモローポリ劇場 \
						-change	iro_blood_mh_mt	evm	\
								$s:新キャスト[ 4 ]	$s:新モデル種類[ 4 ]

				//	それ以外は
				} else {
					command デモモデルリネーム追加 \
						-rename_evm	iro_blood_mh_mt	\
									$s:新キャスト[ 4 ]
				}
			}

//#########################################################################################
	//	フォーチュン遭遇
		} else if ($s:エリア == d012p01) {
		///////////////////////////////////////////////////////////////////////////ライデン
		//	ライデン				rai_def_mh_mt.evm
			//	デフォルトで銃をつける
			chara delay ディレイ \
					-time 1 \ 
					-exec {
						chara 装備品セット ローポリ用raiden装備 -demo
					}

			if ($s:新キャスト[ 1 ] == ori_mdl) {
				command デモローポリ劇場 \
					-equip	rai_def_mh_mt \
							(d:DM_EQUIP_DEFAULT|d:DM_EQUIP_WEAPON)
			} else {
				command デモローポリ劇場 \
					-equip	$s:新キャスト[ 1 ] \
							(d:DM_EQUIP_DEFAULT|d:DM_EQUIP_WEAPON)
			}

			eval ($s:元キャスト[ 1 ] = rai_def_mh_mt)
			eval ($s:元モデル種類[ 1 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 1

		///////////////////////////////////////////////////////////////////////////ヴァンプ
		//	ヴァンプ				vmp_coat_mh_mt.evm
		//	ヴァンプ(胸傷)			vmp_kizu_mh.evm
		//	ヴァンプ(影)			vmp_coat_shadow.kms
			//	オリジナル以外はヴァンプ(胸傷)をとる
			if ($s:新キャスト[ 2 ] != ori_mdl) {
				command デモローポリ劇場 \
					-change	vmp_kizu_mh	evm	\
							null		kms
			}

			eval ($s:元キャスト[ 2 ] = vmp_coat_mh_mt)
			eval ($s:元モデル種類[ 2 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 2

		///////////////////////////////////////////////////////////////////////////フォーチュン
		//	フォーチュン			for_coat_addhand_mh_mt.evm
		//	フォーチュン(影)		for_cort_shadow.kms
			eval ($s:元キャスト[ 3 ] = for_coat_addhand_mh_mt)
			eval ($s:元モデル種類[ 3 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 3

		///////////////////////////////////////////////////////////////////////////シールズ
		//	シールズ				sel_def.kms
			eval ($s:元キャスト[ 4 ] = sel_def)
			eval ($s:元モデル種類[ 4 ] = kms)

			@キャスト代入ＫＭＳ（リネーム） 4

//#########################################################################################
	//	量産ＲＡＹvsソリダス
		} else if ($s:エリア == d080p01) {
		///////////////////////////////////////////////////////////////////////////ソリダス
		//	ソリダス（顔のみ）		sol_def_face_mh_mt.evm
		//	ソリダス（体のみ)		sol_vanim_0_sh_mt.kms
		//	ソリダス（眼帯）		sol_gantai_mh_mt.evm
		//	ソリダス（鉄襟）		sol_faceguard.kms
		//	ソリダス（蛇手）		sol_snakearm_mh_mt.evm
		//	ソリダス（蛇手２）		sol_snakearm_2_mh_mt.evm
		//	鞘						demo_kwt_uni_mount.kms
		//	鞘２					demo_kwt_uni_mount.kms


			//	オリジナル以外は以下をとる
			if ($s:新キャスト[ 1 ] != ori_mdl) {
				//	ソリダス（体のみ)		sol_vanim_0_sh_mt.kms
				command デモローポリ劇場 \
					-change	sol_vanim_0_sh_mt	kms	\
							null				kms
				//	ソリダス（眼帯）		sol_gantai_mh_mt.evm
				command デモローポリ劇場 \
					-change	sol_gantai_mh_mt	evm	\
							null				kms
				//	ソリダス（鉄襟）		sol_faceguard.kms
				command デモローポリ劇場 \
					-change	sol_faceguard	kms	\
							null			kms
				//	ソリダス（蛇手）		sol_snakearm_mh_mt.evm
				command デモローポリ劇場 \
					-change	sol_snakearm_mh_mt	evm	\
							null				kms
				//	ソリダス（蛇手２）		sol_snakearm_2_mh_mt.evm
				command デモローポリ劇場 \
					-change	sol_snakearm_2_mh_mt	evm	\
							null					kms
				//	鞘						demo_kwt_uni_mount.kms
				command デモローポリ劇場 \
					-change	demo_kwt_uni_mount	kms	\
							null				kms
				//	鞘２					demo_kwt_uni_mount.kms
				command デモローポリ劇場 \
					-change	demo_mst_uni_mount	kms	\
							null				kms
			}

			eval ($s:元キャスト[ 1 ] = sol_def_face_mh_mt)
			eval ($s:元モデル種類[ 1 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 1

//#########################################################################################
	//	ソリダスの最後
		} else if ($s:エリア == d082p01) {
		///////////////////////////////////////////////////////////////////////////ライデン
		//	ライデン				rai_def_addhand_mh_mt.evm
			eval ($s:元キャスト[ 1 ] = rai_def_addhand_mh_mt)
			eval ($s:元モデル種類[ 1 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 1

		///////////////////////////////////////////////////////////////////////////ソリダス
		//	ソリダス				sol_def_addhand_mh_mt.evm
		//	ソリダス（眼帯）		sol_gantai_mh_mt.evm
			//	オリジナル以外はソリダス（眼帯）をとる
			if ($s:新キャスト[ 2 ] != ori_mdl) {
				command デモローポリ劇場 \
					-change	sol_gantai_mh_mt	evm	\
							null				kms

				//	ローズは鞘をとる
				if ($s:新キャスト[ 2 ] == ros_de_mh_mt) {
					//	鞘						demo_kwt_uni_mount.kms
					command デモローポリ劇場 \
						-change	demo_kwt_uni_mount	kms	\
								null				kms
					//	鞘２					demo_kwt_uni_mount.kms
					command デモローポリ劇場 \
						-change	demo_mst_uni_mount	kms	\
								null				kms
				}
			}

			eval ($s:元キャスト[ 2 ] = sol_def_addhand_mh_mt)
			eval ($s:元モデル種類[ 2 ] = evm)

			@キャスト代入ＥＶＭ（リネーム） 2

		}

	}

}





//#########################################################################################
//各.gclからローポリ劇場セレクトに戻ってくる関数
proc mv_lowpoly {
	//	フェードアウトしてからロード
/*
	機構的に無理らしい
	chara フェードインアウト フェードアウト \
		-c 0,0,0 128 \
		-t d:FADEOUT_TIME \
		-p mv_lowpoly_end
*/
	@mv_lowpoly_end
}

proc mv_lowpoly_end {
	//	ロード関数
	@mv_lowpoly_select
}





//#########################################################################################
/*
	レジデント内にいるキャラについて

		全レジデント共通

			KMSライデン
			rai_def.kms
			rai_def_sh_mt.kms

			KMSスネーク
			sna_def.kms
			sna_def_sh.kms

			ＭＧＳ１スネーク
			sna_mgs1.kms
			sna_mgs1_mh.evm

			ＭＧＳ１オセロット
			rev_mgs1.kms
			rev_mgs1_mh.evm

			ビジネスマン
			cit_maley_ctg_sh.kms

			ＯＬ
			cit_femaled_def.kms

			おばちゃん
			cit_femalee_def.kms

			ハイテク兵
			htc_def_mt.kms

			忍者
			org_tng_pal_mh_mt.evm

//#########################################################################################

		r_plt6

			KMSスネーク
			sna_def_sh.kmsは無し


			EVMライデン
			rai_def_mh_mt.evm

			フォーチュン
			for_def_sh_mt.kms
			for_def_mh_mt.evm

			オタコン
			otc_def_mh_mt.evm

			タキシードスネーク
			sna_txd_demo_sh_mt.kms（裾つき）
			sna_txd_mh_mt.evm

			ハイクオリティMGS1スネーク
			sna_oss_sh_mt.kms
			sna_oss_mh_mt.evm

			忍者ライデン
			rai_nin_sh_mt_fg.kms（フェイスガードつき）
			rai_nin_mh_mt.evm


	//	タンカー編オープニング	t00a1D
				スネーク
				sna_dive_sh_mt.kms
				オセロット
				rev_def_mh_mt.evm

//#########################################################################################

		r_plt10

			ビジネスマン
			cit_maley_ctg_sh.kmsは無し


			EVMライデン
			rai_def_mh_mt.evm

			フォーチュン
			for_def_sh_mt.kms
			for_def_mh_mt.evm

			オタコン
			otc_def_mh_mt.evm

			タキシードスネーク
			sna_txd_sh_mt.kms
			sna_txd_mh_mt.evm

			ハイクオリティMGS1スネーク
			sna_oss_sh_mt.kms
			sna_oss_mh_mt.evm

			忍者ライデン
			rai_nin_sh_mt.kms
			rai_nin_mh_mt.evm


	//	タンカー乗っ取りデモ	t04a1D
				スネーク
				sna_def_mh.evm

//#########################################################################################

		r_plt11

			KMSライデン
			rai_def_sh_mt.kmsは無し

			KMSスネーク
			sna_def_sh.kmsは無し


			EVMライデン
			rai_def_mh_mt.evm

			ソリダス
			sol_def_mh_mt.evm

			オタコン
			otc_def_mh_mt.evm

			メリル
			mrl_def_mh_mt.evm

			タキシードスネーク
			sna_txd_mh_mt.evm

			ハイクオリティMGS1スネーク
			sna_oss_mh_mt.evm

			忍者ライデン
			rai_nin_mh_mt.evm


	//	オルガＶＳスネーク	t05a1D
				スネーク
				sna_def_mh.evm
				sna_def_addhand_mh_mt.evm

//#########################################################################################

		r_plt12

			KMSライデン
			rai_def_sh_mt.kmsは無し

			KMSスネーク
			sna_def_sh.kmsは無し

			ビジネスマン
			cit_maley_ctg_sh.kmsは無し


			EVMライデン
			rai_def_mh_mt.evm

			ソリダス
			sol_def_mh_mt.evm

			フォーチュン
			for_def_mh_mt.evm
			for_last_addhand_mh_mt.evm

			オタコン
			otc_def_mh_mt.evm

			タキシードスネーク
			sna_txd_mh_mt.evm

			ハイクオリティMGS1スネーク
			sna_oss_mh_mt.evm

			忍者ライデン
			rai_nin_mh_mt.evm


	//	ＲＡＹ強奪	t12a1D
				スネーク
				sna_def_mh.evm

				オセロット
				rev_coat.evm
				rev_def_mh_mt.evm
				rev_def_addhand_mh_mt.evm

//#########################################################################################

		r_plt7

			KMSスネーク
			sna_def.kmsはなし

			ビジネスマン
			cit_maley_ctg_sh.kmsは無し


			EVMスネーク
			sna_def_mh.evm

			オセロット
			rev_def_mh_mt.evm

			フォーチュン
			for_def_sh_mt.kms
			for_def_mh_mt.evm

			オタコン
			otc_def_mh_mt.evm

			タキシードスネーク
			sna_txd_sh_mt.kms
			sna_txd_mh_mt.evm

			ハイクオリティMGS1スネーク
			sna_oss_sh_mt.kms
			sna_oss_mh_mt.evm

			忍者ライデン
			rai_nin_sh_mt.kms
			rai_nin_mh_mt.evm


	//	ヴァンプ遭遇	P010_01_p01
				ライデン
				rai_def_mh_mt.evm

				スネーク
				sna_def.kms

				ヴァンプ
				vmp_coat_mh_mt.evm

//#########################################################################################

		r_plt8

			EVMスネーク
			sna_def_mh.evm

			オセロット
			rev_def_mh_mt.evm
			rev_def_addhand_mh_mt.evm

			オタコン
			otc_def_mh_mt.evm

			タキシードスネーク
			sna_txd_sh_mt.kms
			sna_txd_mh_mt.evm

			ハイクオリティMGS1スネーク
			sna_oss_sh_mt.kms
			sna_oss_mh_mt.evm

			忍者ライデン
			rai_nin_sh_mt.kms
			rai_nin_mh_mt.evm


	//	フォーチュン遭遇	P012_01_p01
				ライデン
				rai_def_mh_mt.evm

				ヴァンプ
				vmp_coat_mh_mt.evm

				フォーチュン
				for_def_sh_mt.kms
				for_coat_mh_mt.evm
				for_coat_addhand_mh_mt.evm

//#########################################################################################

		r_plt9
			KMSスネーク
			sna_def.kmsは無し
			sna_def_sh.kmsは無し

			ＯＬ
			cit_femaled_def.kmsは無し


			EVMスネーク
			sna_def_mh.evm

			オセロット
			rev_def_mh_mt.evm

			ヴァンプ(裸)
			vmp_naked_mh_mt.evm

			ローズ
			ros_def_mh_mt.evm

			オタコン
			otc_def_mh_mt.evm

			タキシードスネーク
			sna_txd_mh_mt.evm

			ハイクオリティMGS1スネーク
			sna_oss_mh_mt.evm

			忍者ライデン
			rai_nin_mh_mt.evm


	//	量産ＲＡＹvsソリダス	P080_03_p02
				スネーク
				sna_def.kms
				sna_def_addhand_mh_mt.evm

				ソリダス
				sol_def_face_mh_mt.evm 
				sol_def_addhand_mh_mt.evm

				フォーチュン
				for_def_mh_mt.evm

//#########################################################################################

		r_plt13

			KMSライデン
			rai_def_sh_mt.kmsは無し

			KMSスネーク
			sna_def.kmsは無し
			sna_def_sh.kmsは無し

			ＯＬ
			cit_femaled_def.kmsは無し

			オセロット
			rev_def_mh_mt.evm

			ヴァンプ(裸)
			vmp_naked_mh_mt.evm

			フォーチュン
			for_last_addhand_mh_mt.evm

			オタコン
			otc_def_mh_mt.evm

			メリル
			mrl_def_mh_mt.evm

			タキシードスネーク
			sna_txd_mh_mt.evm

			ハイクオリティMGS1スネーク
			sna_oss_mh_mt.evm

			忍者ライデン
			rai_nin_mh_mt.evm


	//	ソリダスの最後	P082_01_p01
				ライデン
				rai_def_addhand_mh_mt.evm

				ソリダス
				sol_def_addhand_mh_mt.evm

				スネーク
				sna_def.kms
				sna_def_addhand_mh_mt.evm

				ローズ
				ros_def_mh_mt.evm

*/



//#########################################################################################





#else

print 'lowpoly.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif

