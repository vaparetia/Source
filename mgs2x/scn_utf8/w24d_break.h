//=============================================================================
// w24c_break.h
//
// 2001/07/03 T.Morita
// $Id: w24d_break.h,v 1.6 2001/07/18 15:00:05 usr04249 Exp $
//

//=============================================================================
// 壊れ物
//=============================================================================

proc 壊れ物設置
{
	@壊れコンピューター
	//@割れガラス
	//@壊れ書類
}

proc 壊れ書類
{
	chara プット書類オブジェ ファイル \
	        -model  w24d2_ppepar paper_a paper_b paperBro_S_cm \
		-hazard (d:BRK_HZD_W24D_COMP_ROOM1) \
		-number 1 \
	        -pos    700 -15100 -53624 \  // ここから複数指定（必ず最初）
	        -rot    0 500 0              // ここから複数指定（必ず最後）
}

proc 壊れコンピューター
{
	chara 壊れコンピュータW24D エロコンピューター
}

proc 割れガラス
{
#if 0
	// 外側右周り
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass3_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-pos     -3425,-14000,-47125 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass4_bro \
		-texture w24d2_glass_bld020 w24d2_glass2_bro_alp \
		-pos     -1800,-14000,-47125 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass1_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-rot     0 1024 0 \
		-pos     -5875,-14000,-50087 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass1_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-rot     0 1024 0 \
		-pos     -5875,-14000,-51962 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass1_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-rot     0 1024 0 \
		-pos     -5875,-14000,-58062 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass1_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-rot     0 1024 0 \
		-pos     -5875,-14000,-56187 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
#endif
	//内側右
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass2_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-pos     -4200,-14000,-54050 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
#if 0
	// 外側左周り
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass3_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-pos     3425,-14000,-47125 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass4_bro \
		-texture w24d2_glass_bld020 w24d2_glass2_bro_alp \
		-pos     1800,-14000,-47125 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass1_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-rot     0 1024 0 \
		-pos     5875,-14000,-50087 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass1_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-rot     0 1024 0 \
		-pos     5875,-14000,-51962 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass1_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-rot     0 1024 0 \
		-pos     5875,-14000,-58062 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass1_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-rot     0 1024 0 \
		-pos     5875,-14000,-56187 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
#endif

	//内側左
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass2_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-pos     4200,-14000,-54050 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass5_bro \
		-texture w24d2_glass_bld020 w24d2_glass3_bro_alp \
		-rot     0 1024 0 \
		-pos     3075,-14637,-54737 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2

#if 0
	//正面奥
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass6_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-pos     1500,-14000,-60500 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
	chara プットガラスオブジェ ガラス \
		-model   w24d2_glass6_bro \
		-texture w24d2_glass_bld020 w24d2_glass1_bro_alp \
		-pos     -1500,-14000,-60500 \
		-hazard  d:BRK_HZD_W24D_COMP_ROOM2
#endif

}
