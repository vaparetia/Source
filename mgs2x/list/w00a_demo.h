//
//	w00a_demo.h
//
//	written by S.Hirano 2000.Sep
//
//	$Id: w00a_demo.h,v 1.4 2001/08/08 13:57:35 usr01717 Exp $

// d00aで、タンカーのモデルのみ減ポリゴンモデルを使用。
// それ以外はw00aのデータを使用するので、w00a.kms を削除
// し、world/d00a/d00a_tanker.kms を含めたヘッダを作成した。
// dpackで「このモデルを含まない」オプションが出来れば…

// テクスチャ
pack_all w00a_demo.tri \
//		world/w00a/w00_chn_c1.dar \
		world/d00a/d00a_tanker.dar \
//		world/w00a/w00a.dar \
//		world/w00a/w00a_chn_c1.dar \
		world/w00a/w00a_crps_saku_l1.dar \
//		world/w00a/w00a_crps_saku_l2.dar \
		world/w00a/w00a_crps_saku_r1.dar \
//		world/w00a/w00a_crps_saku_r2.dar \
		world/w00a/w00a_p1.dar \
		world/w00a/w00a_p2.dar \
		world/w00a/w00a_wa1.dar \
		world/w00a/w00a_wa2.dar \
		world/w00a/w00a_wa3.dar \
		world/w00a/w00a_wa4.dar \
		world/w00a/w00a_winch.dar \
		world/w00a/w00ak1.kms



//フォグ用
effect	fog/w00_fog_fader_alp.bmp

// ＫＭＳ
//world	w00a/w00_chn_c1.kms
world	d00a/d00a_tanker.kms
//world	w00a/w00a.kms
//world	w00a/w00a_chn_c1.kms
world	w00a/w00a_crps_saku_l1.kms
//world	w00a/w00a_crps_saku_l2.kms
world	w00a/w00a_crps_saku_r1.kms
//world	w00a/w00a_crps_saku_r2.kms
world	w00a/w00a_p1.kms
world	w00a/w00a_p2.kms
world	w00a/w00a_wa1.kms
world	w00a/w00a_wa2.kms
world	w00a/w00a_wa3.kms
world	w00a/w00a_wa4.kms
world	w00a/w00a_winch.kms
world	w00a/w00ak1.kms


// ＣＶ２ は不要
