//
//	w11bc_common	
//
//	written by H.Yoshiike 2000.Aug
//
//	$Id: w11bc_common.h,v 1.8 2002/09/11 11:58:13 usr03682 Exp $


//天井落下モーション
mtn		w11c_ceil.mls		// ステージ固有

//影用の重なり用( 描画順制御のため TRI 名前注意 )
pack_all zzza.tri \
	world/w11c/w11c2_forklift_bc_b.dar
pack_all zzzb.tri \
	world/w11c/w11c2_forklift_bc_a.dar 
pack_all zzzc.tri \
	world/w11c/w11c2_box_backclip4.dar
pack_all zzzd.tri \
	world/w11c/w11c2_box_backclip3.dar \
	world/w11c/w11c2_contena_brk5_bc_b.dar \
	world/w11c/w11c2_drum_bc.dar \
	world/w11c/w11c2_drum_brk2_bc_b.dar
pack_all zzze.tri \
	world/w11c/w11c2_box_backclip2.dar \
	world/w11c/w11c2_contena_brk4_bc_a.dar \
	world/w11c/w11c2_contena_brk5_bc_a.dar \
	world/w11c/w11c2_contena_brk6_bc_b.dar \
	world/w11c/w11c2_contena_brk7_bc_a.dar \
	world/w11c/w11c2_drum_brk2_bc_a.dar
pack_all zzzf.tri \
	world/w11c/w11c2_box_backclip.dar \
	world/w11c/w11c2_contena_bc.dar \
	world/w11c/w11c2_contena_brk4_bc_b.dar \
	world/w11c/w11c2_contena_brk6_bc_a.dar


//壊れ物 ( パックの関係上先 )
world/w11c \
	w11c2_ironbox.kms  w11c2_ironbox_brk1.kms  w11c2_ironbox_brk2.kms \
	w11c2_ironbox_frg1_cm.kms  w11c2_ironbox_frg2_cm.kms \
	w11c2_contena.kms \
	w11c2_contena_brk1.kms  w11c2_contena_brk2.kms  w11c2_contena_brk3.kms \
	w11c2_contena_brk4.kms  w11c2_contena_brk5.kms  w11c2_contena_brk6.kms \
	w11c2_contena_brk7.kms  w11c2_contena_brk8.kms \
	w11c2_woodbox.kms \
	w11c2_woodbox_frg1_cm.kms w11c2_woodbox_frg2_cm.kms  w11c2_woodbox_frg3_cm.kms \
	w11c2_oilspot1.kms \
	w11c2_explode_drum.kms  w11c2_explode_drum_brk1.kms \
	w11c2_drum.kms  w11c2_drum_brk1.kms  w11c2_drum_brk2.kms \
        w11c2_drum_futa.kms w11c2_drum_frg1_cm.kms \
	w11c2_forklift.kms  w11c2_forklift_brk1.kms  w11c2_forklift_brk2.kms \
	w11c2_forklift_fwheel.kms  w11c2_forklift_bwheel.kms \
	w11c2_forklift_frame.kms  w11c2_forklift_front.kms \
	w11c2_hari1.kms  w11c2_hari2.kms  w11c2_hari3.kms

cvd world/w11c/w11c2_forklift_fwheel.cv2      vn
cvd world/w11c/w11c2_oilspot1.cv2 vn
cvd world/w11c/w11c2_ceiling1_brk1.cv2 vn

// ワールド ( 残りをまとめてしまう )
option makexti6 -l 1024 -f PAL8
pack_all	w11c2.tri world/w11c world/w11_common world/door/dr_free.dar world/elevator/elv_1/elv_1.dar
option makexti6

world		w11c
world		w11_common
world		door/dr_free.kms

// オブジェ関係

// デバッグ用
// 小さいサイズのテクスチャはまとめておく

//昇降機モデル達
human vmp_coat/vmp_coat_mh_mt.evm
