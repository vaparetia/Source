//	vrstage.h
//	ＶＲステージ
//	$Id$

#ifdef VR_TGT_CUBE
	//キューブ
	meca		vr2_target/vr2_tgt_cube_tab.kms
	meca		vr2_target/vr2_tgt_cube.kms
	meca		vr2_target/vr2_tgt_cube_exp.kms
	meca		vr2_target/vr2_tgt_cube_hlt.kms

	//cvd
	meca		vr2_target/vr2_tgt_cube_rgb.kms
	rol			meca/vr2_target/vr2_tgt_cube_rgb.rol



#endif

#ifdef VR_TGT_SQUARE
	//十字
	meca		vr2_target/vr2_tgt_square.kms
	meca		vr2_target/vr2_tgt_square_exp.kms
	meca		vr2_target/vr2_tgt_square_tab.kms
	meca		vr2_target/vr2_tgt_square_hlt.kms

	meca		vr2_target/vr2_tgt_square_rgb.kms
	rol			meca/vr2_target/vr2_tgt_square_rgb.rol


#endif

#ifdef VR_TGT_TRIANGLE
	//三角
	meca		vr2_target/vr2_tgt_triangle.kms
	meca		vr2_target/vr2_tgt_triangle_exp.kms
	meca		vr2_target/vr2_tgt_triangle_tab.kms
	meca		vr2_target/vr2_tgt_triangle_hlt.kms

	meca		vr2_target/vr2_tgt_triangle_rgb.kms
	rol			meca/vr2_target/vr2_tgt_triangle_rgb.rol
	//部位なし三角
	meca	vr2_target/vr2_tgt_triangle_nov.kms
	meca	vr2_target/vr2_tgt_triangle_nov_tab.kms

#endif

#ifdef VR_TGT_OCTA
	//オクタ
	meca		vr2_target/vr2_tgt_octa.kms

	meca		vr2_target/vr2_tgt_octa_exp.kms
	meca		vr2_target/vr2_tgt_octa_tab.kms
	meca		vr2_target/vr2_tgt_octa_hlt.kms

	meca		vr2_target/vr2_tgt_octa_rgb.kms
	rol			meca/vr2_target/vr2_tgt_octa_rgb.rol


#endif

#ifdef VR_TGT_STRAW
	//刀
	meca	vr2_target/vr2_tgt_straw.kms
	meca	vr2_target/vr2_tgt_straw_exp.kms
	meca	vr2_target/vr2_tgt_straw_tab.kms
	meca	vr2_target/vr2_tgt_straw_hlt.kms

	meca		vr2_target/vr2_tgt_straw_rgb.kms
	rol			meca/vr2_target/vr2_tgt_straw_rgb.rol


#endif

#ifdef VR_TGT_HEXA
	//スティンガー用UFO的
	meca	vr2_target/vr2_tgt_hexa.kms
	meca	vr2_target/vr2_tgt_hexa_exp.kms
	meca	vr2_target/vr2_tgt_hexa_tab.kms

	meca		vr2_target/vr2_tgt_hexa_rgb.kms
	rol			meca/vr2_target/vr2_tgt_hexa_rgb.rol


#endif

#ifdef VR_TGT_WALL
//壁
	meca	vr2_target/vr2_tgt_wall.kms
	meca	vr2_target/vr2_tgt_wall_rgb.kms
	rol		meca/vr2_target/vr2_tgt_wall_rgb.rol
#endif

#ifdef VR_TGT_MINE
	meca	vr2_target/vr2_tgt_watermine.kms
	pack_trnall	effect.tri \
			effect/light/xlit04a_alp.bmp \
			effect/light/xlit04b_alp.bmp
#endif

#ifdef VR_TGT_DARK_OCTA
	/*ダークステージ専用 縮み的*/
	meca	vr2_target/vr2_tgt_octa_nov.kms
	meca		vr2_target/vr2_tgt_octa_rgb.kms
	rol		meca/vr2_target/vr2_tgt_octa_rgb.rol
#endif
