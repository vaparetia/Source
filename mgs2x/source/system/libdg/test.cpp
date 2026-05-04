//-----------------------------------------------------------------------------
// テストルーチン
//                                                               F.Miyauchi
//-----------------------------------------------------------------------------

#include "dxgame.h"

// 仮クラス
#include "dxcamera.h" // カメラ制御クラス
#include "dxdraw.h"   // プリミティブ描画クラス
//#include "dxinput.h"  // 入力取得クラス
#include "libfs.h"
#include "assert.h"

//#include "fmt_kms.h"
#include "libdg.h"

//-----------------------------------------------------------------------------

CDXCamera *g_pCamera = NULL;        // カメラ制御クラス
CDXDraw   *g_pDraw = NULL;          // プリミティブ描画クラス
//CDXInput  *g_pInput = NULL;         // 入力取得クラス

#define N_TEXTURES 256
LPDIRECT3DTEXTURE8 g_pTexture[N_TEXTURES];    // テクスチャ(仮)

static char mdl_buf[1024 * 1024];  // モデル読み込みバッファ(仮)

typedef struct tagBMPTABLE {
	char         *bmp_name;
	unsigned int id;	
} BMPTABLE;

#define MDL_PATH   "m:\\develop\\usr\\miya\\x\\data\\mdl\\"
//#define MDL_NAME "btl_01.kmx"
//#define MDL_NAME MDL_PATH"bottle\\btl_01.kmx"
//#define MDL_NAME "w00a.kmx"
//#define MDL_NAME "hri_def.kmx"
//#define MDL_NAME    MDL_PATH"sna_def\\sna_def.kmx"
#define MDL_NAME    MDL_PATH"sna_def\\sna_def.kmx"


#if 0
BMPTABLE bmp_lst[] = {
	{"btl_01.bmp", 7832316},
};
#endif

// 適当テーブル
BMPTABLE bmp_lst[] = {
	{"sna_p1dt.bmp", 2917692},
	{"sna_body_ss01dt.bmp", 3338965},
	{"sna_p2dt.bmp", 2918716},
	{"sna_face_ss01dt.bmp", 7426787},
	{"sna_hair2_alp_ovl.bmp", 11240710},
	{"sna_hair1_alp_ovl.bmp", 11175174},
	{"sna_hair3.bmp", 2669432},
	{"sna_hand1dt.bmp", 3597719},
	{"sna_hand2.bmp", 2674103},
	{"sna_arm_ss.bmp", 2698091},
	{"sna_leg_ss.bmp", 5647232},
	{"sna_foot01dt.bmp", 14375133},
	{"sna_toe01dt.bmp", 9234610},
	{"sna_toe02dt.bmp", 9235634},
	{"sna_belt_leg.bmp", 15966826},
	{"sna_shoul01dt.bmp", 3577397},
	{"sna_leg_r01dt.bmp", 9403401},
	{"rai_gun_fro.bmp", 10304308},
	{"rai_gun_back.bmp", 10822014},
	{"rai_gun_side2.bmp", 12071751},
	{"rai_gun_top.bmp", 10318549},
	{"rai_gun_side.bmp", 11387288},
	{"v_scm_r.bmp", 6512279},
};
#if 0
BMPTABLE bmp_lst[] = {
	{"sna_p1dt.bmp", 2917692},
	{"sna_hair2dt_alp_ovl.bmp", 13322804},
	{"sna_hair1_alp_ovl.bmp", 11175174},
	{"sna_hair3.bmp", 2669432},
	{"sna_hi_bdn01dt.bmp", 5370957},
	{"sna_hi_eye01dt.bmp", 12711222},
	{"sna_hi_face01dt.bmp", 9263947},
	{"sna_hi_meziri.bmp", 14777305},
	{"sna_hi_ela01_alp_ovl.bmp", 11429483},
	{"sna_body_ss01dt.bmp", 3338965},
	{"sna_p2dt.bmp", 2918716},
	{"sna_hand1dt.bmp", 3597719},
	{"sna_hand2.bmp", 2674103},
	{"sna_arm_ss.bmp", 2698091},
	{"sna_leg_ss.bmp", 5647232},
	{"sna_foot01dt.bmp", 14375133},
	{"sna_toe01dt.bmp", 9234610},
	{"sna_toe02dt.bmp", 9235634},
	{"sna_belt_leg.bmp", 15966826},
	{"sna_shoul01dt.bmp", 3577397},
	{"sna_leg_r01dt.bmp", 9403401},
	{"rai_gun_fro.bmp", 10304308},
	{"rai_gun_back.bmp", 10822014},
	{"rai_gun_side2.bmp", 12071751},
	{"rai_gun_top.bmp", 10318549},
	{"rai_gun_side.bmp", 11387288},
	{"v_scm_r.bmp", 6512279},
};
BMPTABLE bmp_lst[] = {
	{"kck_twing_s.bmp", 9879552},
	{"kck_body1_s.bmp", 4940656},
	{"kck_body2.bmp", 13062870},
	{"kck_body1_t.bmp", 4940657},
	{"kck_mark504.bmp", 2387958},
	{"kck_glass.bmp", 1427032},
	{"kck_iner_wall.bmp", 8421198},
	{"kck_iner_floor.bmp", 358049},
	{"kck_ropebox.bmp", 142710},
	{"kck_gray1.bmp", 1623766},
	{"kck_twing2_s.bmp", 14109189},
	{"kck_red.bmp", 14774416},
	{"kck_twing2_t.bmp", 14109190},
	{"kck_propera.bmp", 16234820},
	{"kck_roter.bmp", 13078679},
};
BMPTABLE bmp_lst[] = {
	{"w00ak_01a0_fl02.bmp", 5867822},
	{"w00ak_01b_wb00_fade.bmp", 14809732},
	{"w00ak_01b_wb00.bmp", 14595276},
	{"w00ak_01b_flb00.bmp", 13394378},
	{"w00ak_01b_wb00b.bmp", 14064125},
	{"col_blk.bmp", 15806351},
	{"w00b_tower_saku1.bmp", 2282809},
	{"w00a_frt1.bmp", 10314617},
	{"w00ak_shd_flor1_sub_alp_ovl.bmp", 10440379},
	{"w00ak_00b_wingtower_fs.bmp", 1189056},
	{"w00b_wingtower_b.bmp", 10223445},
	{"w00b_step2wing_t2.bmp", 15818036},
	{"w00b_raderdome.bmp", 10093931},
	{"w00b_smllrader_jiku.bmp", 5908200},
	{"w00b_wingwall1.bmp", 15580059},
	{"w00b_wingwall2.bmp", 15580060},
	{"w00b_col_black.bmp", 9387556},
	{"w00b_tsr1.bmp", 10774331},
	{"w00b_tsr2.bmp", 10774332},
	{"w00a_pl3.bmp", 4526617},
	{"w00a_rb2a.bmp", 10689385},
	{"w00b_step_s.bmp", 10974946},
	{"w00_stp_t1.bmp", 9118573},
	{"w00c_stp_f1msk.bmp", 12720424},
	{"w00_pll_pt5msk.bmp", 12375293},
	{"w00_pll_pt4.bmp", 2385269},
	{"w00b_tower_saku2.bmp", 2282810},
	{"w00b_tower_deck_s1PSD.bmp", 11743991},
	{"w00b_tower_deck_u1.bmp", 13674403},
	{"w00b_tower_deck_t1.bmp", 13674371},
	{"w00_pll_pt3msk.bmp", 12309757},
	{"w00b_tower_deck_t2.bmp", 13674372},
	{"w00b_rader1.bmp", 7785725},
	{"w00c_tower_wing_msk.bmp", 13989442},
	{"w00ak_ue1.bmp", 5893530},
	{"w00m_ikari_kabe.bmp", 14183972},
	{"w00m_ikari_sotowaku.bmp", 2808924},
	{"w00m_ikari_side.bmp", 14454372},
	{"w00m_ikari_face.bmp", 14020164},
	{"w00ak_discovery_msk.bmp", 16430708},
	{"w00b_tower_s1.bmp", 3461138},
	{"w00b_tower_s2.bmp", 3461139},
	{"lifeboat_number_01_alp_ovl.bmp", 1462688},
	{"lifeboat_rescue_alp_ovl.bmp", 1442995},
	{"w00m_wiremsk.bmp", 12883370},
	{"w00m_bandmsk.bmp", 8654938},
	{"w00m_pick.bmp", 10632619},
	{"w00m_block_side.bmp", 16644775},
	{"w00m_block.bmp", 6907102},
	{"w00m_blacket.bmp", 11765432},
	{"w00m_blacket_face.bmp", 9150012},
	{"w00m_blacket_top.bmp", 12359355},
	{"w00m_post_side.bmp", 7331924},
	{"w00m_post_face.bmp", 6897716},
	{"w00m_post_top.bmp", 8094667},
	{"w00m_hashira_sasae2.bmp", 5812270},
	{"w00m_hashira_sasae.bmp", 14861695},
	{"w00_was_t2msk.bmp", 12349724},
	{"w00_was_f1msk.bmp", 14414107},
	{"w00_was_f2msk.bmp", 14446875},
	{"w00a_dc1.bmp", 4514039},
	{"w00b_fannel_text_msk.bmp", 14908146},
	{"w00a_pl1.bmp", 4526615},
	{"w00a_w2v.bmp", 4531996},
	{"w00a_pl2_msk.bmp", 11899884},
	{"w00a_w1.bmp", 3287348},
	{"w00a_pol1.bmp", 10638969},
	{"w00a_os1.bmp", 4525815},
	{"w00ak_2fdeck_antisna.bmp", 11887505},
	{"w00b_huge_lit_jiku.bmp", 8190661},
	{"w00_box1.bmp", 2527127},
	{"w00a_rl5.bmp", 4528667},
	{"w00a_w3.bmp", 3287350},
	{"w00a_rl3.bmp", 4528665},
	{"w00a_ltb3.bmp", 10512699},
	{"w00a_ltb4.bmp", 10512700},
	{"w00a_ltb1v.bmp", 862122},
	{"w00a_ltb2.bmp", 10512698},
	{"w00a_cod_alp.bmp", 13983706},
	{"w00ak_survial_rope_alp.bmp", 2230392},
	{"w00ak_blc_t4.bmp", 5748613},
	{"w00ak_dck_flr1.bmp", 14160287},
	{"w00ak_sf2.bmp", 5891515},
	{"w00ak_blc_s2.bmp", 5748579},
	{"w00ak_sf3.bmp", 5891516},
	{"w00ak_blc_s1.bmp", 5748578},
	{"w00ak_sf4.bmp", 5891517},
	{"w00_ldk_u1.bmp", 8954750},
	{"w00_ldk_u2.bmp", 8954751},
	{"w00ak_sf1.bmp", 5891514},
	{"w00_blc_u1.bmp", 303979},
	{"w00a_blc_t1.bmp", 2501918},
	{"w00_blc_u2.bmp", 303980},
	{"w00a_blc_t2.bmp", 2501919},
	{"w00a_blc_t3.bmp", 2501920},
	{"w00_blc_u3.bmp", 303981},
	{"w00a_dck_at10.bmp", 11278520},
	{"w00_blc_u4.bmp", 303982},
	{"w00a_dck_at5.bmp", 4546760},
	{"w00a_dck_at11.bmp", 11278521},
	{"w00a_dck_at7.bmp", 4546762},
	{"w00a_dck_at6.bmp", 4546761},
	{"w00a_dck_at13.bmp", 11278523},
	{"w00a_dck_at14.bmp", 11278524},
	{"w00a_dck_at15.bmp", 11278525},
	{"w00a_dck_at8.bmp", 4546763},
	{"w00ak_flr1_alp_ovl.bmp", 13669026},
	{"w00ak_gateflor1_alp_ovl.bmp", 13394285},
	{"w00ak_flrshad2_sub_alp_ovl.bmp", 10063669},
	{"w00ak_flrshad5_sub_alp_ovl.bmp", 10075957},
	{"w00ak_flrshad4_sub_alp_ovl.bmp", 10071861},
	{"w00ak_flrshad1_sub_alp_ovl.bmp", 10059573},
	{"w00ak_flrshad3_sub_alp_ovl.bmp", 10067765},
	{"w00ak_whiteline_alp_ovl.bmp", 14573962},
	{"w00ak_door_lit.bmp", 299980},
	{"lifeboat_body_center_01.bmp", 9308124},
	{"lifeboat_body_back_01.bmp", 11944649},
	{"lifeboat_body_front_01.bmp", 244128},
	{"lifeboat_window_back_01.bmp", 11008356},
	{"lifeboat_window_front.bmp", 1415794},
	{"lifeboat_fin_01.bmp", 15060725},
	{"lifeboat_lift_winch_01.bmp", 11459684},
	{"lifeboat_lift_winch_side_01.bmp", 13749296},
	{"lifeboat_lift_winch_under_01.bmp", 4445035},
	{"lifeboat_lift_side_01.bmp", 16665601},
	{"lifeboat_lift_under_01.bmp", 13880720},
	{"w00m_winch_face4.bmp", 9064646},
	{"lifeboat_lift_top_01.bmp", 15586433},
	{"lifeboat_lift_arm_01.bmp", 1856604},
	{"lifeboat_lift_09.bmp", 4529237},
	{"w00a_at2.bmp", 4511512},
	{"w00m_mush_huta_back.bmp", 15865153},
	{"w00m_mush_huta_face.bmp", 15996219},
	{"w00m_mushroom_ura.bmp", 11382837},
	{"w00m_mushroom_back.bmp", 11289440},
	{"w00m_mushroom_face.bmp", 11420506},
	{"w00m_mush_huta_top3.bmp", 16469673},
	{"w00a_rtn1_msk.bmp", 13334902},
	{"w00ak_crane_floor.bmp", 372222},
	{"w00a_rtn0.bmp", 10709688},
	{"w00a_rw1.bmp", 4529015},
	{"w00_roll4.bmp", 13748857},
	{"w00m_mashroom_panel_side.bmp", 16375919},
	{"w00m_mashroom_panel_face.bmp", 15941711},
	{"w00m_mush_handl_side.bmp", 495773},
	{"w00m_mush_handl_topmsk.bmp", 11051755},
	{"w00ak_survial_ashi3.bmp", 14610579},
	{"w00ak_survial_s1.bmp", 8815544},
	{"w00ak_survial_f1.bmp", 8815128},
	{"w00a_rb3.bmp", 4528345},
	{"w00a_sn1_msk.bmp", 10851504},
	{"w00a_at1_msk.bmp", 10850364},
	{"w00a_sn2.bmp", 4529752},
	{"w00a_sn3.bmp", 4529753},
	{"w00ak_craneside_box1.bmp", 844787},
	{"w00m_pole_hikari.bmp", 8214610},
	{"w00m_sakumsk.bmp", 6067354},
	{"w00m_top_yukamsk.bmp", 10247966},
	{"w00m_top_yuka_uramsk.bmp", 9176385},
	{"w00b_crane_post_s.bmp", 8861915},
	{"w00m_winch_side1111.bmp", 6486395},
	{"w00m_winch_back1.bmp", 4870531},
	{"w00m_winch_top2.bmp", 10193905},
	{"w00m_winch_top1.bmp", 10193904},
	{"w00m_winch_side4rn.bmp", 5450791},
	{"w00m_winch_back2.bmp", 4870532},
	{"w00m_winch_side6.bmp", 6182089},
	{"w00m_winch_side5.bmp", 6182088},
	{"w00a_ac1.bmp", 4510967},
	{"w00a_fin1.bmp", 10305209},
	{"w00ak_dbx_tank_f1.bmp", 10188899},
	{"w00ak_dbx_tank_s1.bmp", 10189315},
	{"dbx_tnk3_msk.bmp", 12858043},
	{"w00ak_ldk_p0.bmp", 14399252},
	{"w00_rib01.bmp", 13540086},
	{"w00ak_ldk_p01.bmp", 7791308},
	{"w00a_rl4_msk.bmp", 13997164},
	{"w00_pll_f2.bmp", 598440},
	{"w00d_wpol_s1.bmp", 9362814},
	{"w00d_wpol_s2.bmp", 9362815},
	{"w00ak_ref1.bmp", 3947036},
	{"w00ak_ref_ldk_f1.bmp", 8756318},
	{"w00ak_ref_door_panel.bmp", 1426311},
	{"w00ak_ref_door_lit.bmp", 10401395},
	{"w00ak_ref_dbx_tnk2.bmp", 7945026},
	{"w00ak_ref_dbx_tnk1.bmp", 7945025},
	{"w00ak_ref_dbx_tnk3_msk.bmp", 6870655},
	{"w00ak_ref_wnc.bmp", 2558016},
	{"w00ak_ref_roll1.bmp", 13742668},
	{"w00ak_refcover.bmp", 1910967},
	{"w00ak_ref3.bmp", 3947038},
	{"w00ak_ref_rabx_f2.bmp", 3143708},
	{"w00ak_ref_rail2.bmp", 13280845},
	{"w00ak_ref_rabx_sb.bmp", 3144172},
	{"w00ak_endwall_f1.bmp", 7279175},
	{"w00ak_ldk_s1.bmp", 14399349},
	{"w00ak_ldk_f1.bmp", 14398933},
	{"w00b_fannel1.bmp", 8253504},
	{"w00b_fannel2.bmp", 8253505},
	{"w00b_fannel3s.bmp", 12454082},
	{"w00ak_wi1.bmp", 5895706},
	{"w00ak_rail1_t.bmp", 6057577},
	{"w00ak_rail1_b_msk.bmp", 10946992},
	{"w00ak_rail1msk.bmp", 9307510},
	{"w00ak_rail2.bmp", 8738585},
	{"w00ak_rail2_b.bmp", 6058583},
	{"w00ak_rabx_b1.bmp", 15935109},
	{"w00ak_rabx_f1.bmp", 15935237},
	{"w00ak_rabx_s1.bmp", 15935653},
	{"w00_rail8.bmp", 13287037},
	{"w00ak_ldk_p2.bmp", 14399254},
	{"w00a_fl1.bmp", 4516375},
	{"w00a_abs.bmp", 4511001},
	{"w00ak_00b_wall1.bmp", 14092620},
	{"w00a_w01e_wind.bmp", 7788741},
	{"w00_wng_u1.bmp", 2532245},
	{"w00_wng_u2.bmp", 2532246},
	{"w00_wng_u3.bmp", 2532247},
	{"w00c_drwl_f.bmp", 9465527},
	{"w00ak_01b_wb03a.bmp", 14064220},
	{"w00c_drwl_s.bmp", 9465540},
	{"w00c_drwl_st.bmp", 907526},
	{"w00ak_door_panel.bmp", 9119006},
	{"w00ak_wnc_s3.bmp", 7845774},
	{"w00ak_wnc_s2_parts.bmp", 2112278},
	{"w00ak_ref_rail1.bmp", 13280844},
	{"w00ak_ref_rabx_f1.bmp", 3143707},
};
BMPTABLE bmp_lst[] = {
	{"hri2_nozzlef_s1.bmp", 10626196},
	{"hri2_twing_t1.bmp", 12485620},
	{"hri2_nozzleb_s1.bmp", 10495124},
	{"hri2_mwing_t1.bmp", 12471284},
	{"hri2_twing_s1.bmp", 12485588},
	{"hri2_fan.bmp", 5538275},
	{"hri2_body_s1.bmp", 13849034},
	{"hri2_face_s1.bmp", 12145326},
	{"hri2_nozzle_cov1.bmp", 14060388},
	{"hri2_face_flenz.bmp", 10911458},
	{"hri2_body_duct_s1.bmp", 1190645},
	{"hri2_tail_s1.bmp", 1889839},
	{"hri2_seat.bmp", 9882430},
	{"hri2_cockpit_sideplate.bmp", 8407289},
	{"hri2_wheel.bmp", 1767007},
	{"hri2_cockpit_console_t1.bmp", 1122493},
	{"hri2_cockpit_console_f1.bmp", 1122045},
	{"hri2_gunpod_cntr.bmp", 9058312},
	{"hri2_gunpod_s1.bmp", 4564045},
	{"hri2_kyuyu_pipe.bmp", 15803449},
	{"hri2_piron1_s.bmp", 13706792},
	{"hri_col1.bmp", 2533129},
	{"hri2_missilepod_s1.bmp", 4358248},
};
#endif

// テクスチャ検索
static LPDIRECT3DTEXTURE8 FindTexture(unsigned int id)
{
	int i;
	for (i = 0; i < N_TEXTURES; i ++) {
		if (bmp_lst[i].id == id) {
			printf("----%s found\n", bmp_lst[i].bmp_name);
			return g_pTexture[i];
		}
	}
	//printf("id %d not found\n", id);	
	return NULL;
}

CUSTOMVERTEX2 g_Vertexbuf[65536];

#define COUNTOF(n)  (sizeof(n) / sizeof((n)[0]))

// 再帰的に親オブジェクトにさかのぼってオフセット取得
// とりあえず表示だけだからこれでいいや。(^^;;
// 本当は行列演算しなきゃ。
static void get_offset(DG_DEF *def, DG_MDL *mdl, float &tx, float &ty, float &tz)
{
	int parent_id;
	DG_MDL *parent_mdl;

	// 親オブジェクトに付随するフラグ
	if (!(mdl->type & DG_TYPE_EXTEND)) {
		tx += mdl->tx;
		ty += mdl->ty;
		tz += mdl->tz;
	}
	parent_id = mdl->parent;
	
	if (parent_id < 0) { // 親オブジェクト無し
		return;
	} else {
		parent_mdl = &def->models[parent_id];
		get_offset(def, parent_mdl, tx, ty, tz);
	}
}

// KMS_PKT解析→座標セット
static void set_kms_pkt(DG_DEF *def, DG_MDL *mdl, DG_MDLPACK *pkt)
{
	int i;
	float tx, ty, tz; // 親からのオフセット

	// 絶対アドレスに変換
	pkt->verts = (SVECTOR *)((char *)pkt->verts + (unsigned int)def);
	pkt->norms = (SVECTOR *)((char *)pkt->norms + (unsigned int)def);
	pkt->uvs[0] = (TVECTOR_S *)((char *)pkt->uvs[0] + (unsigned int)def);
	pkt->uvs[1] = (TVECTOR_S *)((char *)pkt->uvs[1] + (unsigned int)def);
	pkt->uvs[2] = (TVECTOR_S *)((char *)pkt->uvs[2] + (unsigned int)def);	
	pkt->rgbs = (CVECTOR *)((char *)pkt->rgbs + (unsigned int)def);

    printf( "\t\t flag		%lx\n", pkt->flag ) ;
    printf( "\t\t n_verts	%ld\n", pkt->n_verts ) ;
    printf( "\t\t tid		%ld\n",
			pkt->tex_id[0], pkt->tex_id[1], pkt->tex_id[2] ) ;
	
	CUSTOMVERTEX2 l_Vertices[128];
	if (pkt->n_verts > 128) {
		Xerror("Vertex 128 Over....(%d)\n", pkt->n_verts);
	}
	// 親オブジェクトに遡ってオフセットを取得する
	tx = ty = tz = 0.0f;
	get_offset(def, mdl, tx, ty, tz);
	
	for (i = 0; i < pkt->n_verts; i ++) {
#if 0
		printf( "\t\t[%d %d %d %d]", pkt->verts[ i ].vx, pkt->verts[ i ].vy,
				pkt->verts[ i ].vz, pkt->verts[ i ].vw ) ;
		printf( "[%d %d %d %x]", pkt->norms[ i ].vx, pkt->norms[ i ].vy,
				pkt->norms[ i ].vz, pkt->norms[ i ].vw ) ;
		printf( "[%d %d]", pkt->uvs[ i ].u, pkt->uvs[ i ].v ) ;
		printf( "[%d %d %d]\n",
				pkt->rgbs[i].r, pkt->rgbs[i].g, pkt->rgbs[i].b);
#endif
		l_Vertices[i].x = (float)pkt->verts[i].vx + tx;
		l_Vertices[i].y = (float)pkt->verts[i].vy + ty;
		l_Vertices[i].z = (float)pkt->verts[i].vz + tz;
#if 0
		l_Vertices[i].color =
				pkt->rgbs[i].r
				| (pkt->rgbs[i].g << 8)
				| (pkt->rgbs[i].b << 16)
				| (0xff << 24);
#else
		l_Vertices[i].color = 0xffffffff;
#endif
		l_Vertices[i].tu = (float)pkt->uvs[0][i].u / 4096.0f;
		l_Vertices[i].tv = (float)pkt->uvs[0][i].v / 4096.0f;
	}
	g_pDraw->SetVertex(D3DPT_TRIANGLESTRIP, l_Vertices,
					   pkt->n_verts, FindTexture(pkt->tex_id[0])); // とりあえず
}

// KMS_OBJ 解析
static void set_kms_obj(DG_DEF *def, DG_MDL *mdl)
{
	int i;
    DG_MDLPACK	*pkt;
	
	printf("DumpKmsObj\n");
	printf("objs = %p\n", mdl);
	
    printf("\t type 		%lx\n", mdl->type);
    printf("\t n_packs		%ld\n", mdl->n_packs);
    printf("\t lx,ly,lz		%.3f, %.3f, %.3f\n", mdl->lx,mdl->ly,mdl->lz);
    printf("\t ux,uy,uz		%.3f, %.3f, %.3f\n", mdl->ux,mdl->uy,mdl->uz);
    printf("\t tx,ty,tz		%.3f, %.3f, %.3f\n", mdl->tx,mdl->ty,mdl->tz);
    printf("\t parent 		%ld\n", mdl->parent);

	pkt = (DG_MDLPACK *)((unsigned char *)mdl->packs + (unsigned int)def);
    for ( i = 0; i < mdl->n_packs; i ++ ) {
		set_kms_pkt(def, mdl, pkt);
		pkt ++ ;
    }
}

static void set_kms(DG_DEF *def)
{ // モデルの頂点設定
	int i;
	DG_MDL *mdl;

	printf("data_format = %d\n\n", def->data_format);
	
	mdl = def->models;
	for (i = 0; i < def->n_x_models; i ++, mdl ++) {
		printf("object No.%d\n", i);
		set_kms_obj(def, mdl);
	}
}


VOID TEST_Init()
{
	HRESULT hr;

	// 各クラスの初期化。

	// 適当版描画クラスの初期化
	g_pDraw = new CDXDraw(g_pd3dDevice);
	g_pCamera = new CDXCamera();
//	g_pCamera->SetViewLookAtLH(D3DXVECTOR3(0.0f, 0.0f, - 80000.0f),  // カメラ位置
	g_pCamera->SetViewLookAtLH(D3DXVECTOR3(0.0f, 200.0f, - 2400.0f),  // カメラ位置
							   D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // 注視点
							   D3DXVECTOR3(0.0f, 1.0f, 0.0f));   // 上ベクトル
	g_pCamera->SetPerspectiveFovLH(D3DX_PI / 4, // 視野
								   //1.0f,        // アスペクト比
								   1.1f,        // アスペクト比
								   100.0f,        // 近くのビュープレーン
								   100000.0f);     // 遠くのビュープレーン
	// テクスチャを全部読み込み
	int i;
	for (i = 0; i < COUNTOF(bmp_lst); i ++) {
		// 高レベルテクスチャ生成関数。
		// 速度はかってないけど、重いのかなぁ。(^^;
		hr = D3DXCreateTextureFromFileEx(
				g_pd3dDevice,
				bmp_lst[i].bmp_name,//g_szTexName[i],
				D3DX_DEFAULT,       // 幅
				D3DX_DEFAULT,       // 高さ
				D3DX_DEFAULT,       // ミップマップ数
				0,                  // レンダリングターゲットにするかどうか
				D3DFMT_UNKNOWN,     // テクスチャフォーマット(D3DFMT)
				D3DPOOL_DEFAULT,    // リソース配置位置
				D3DX_FILTER_LINEAR, // フィルタ
				D3DX_FILTER_LINEAR, // ミップマップフィルタ
				0xff000000,         // 透明色
				NULL, // (返値)D3DXIMAGE_INFO
				NULL, // (返値)パレット
				&g_pTexture[i]);
		if (FAILED(hr)) {
			Xerror("Texture (%d) Load Failed\n", i);
		}
	}

	FS_StartDaemon();
	{
		int size;
		size = FS_LoadRequest(MDL_NAME);
		if (size <= 0) {
			assert(0);
		}
		FS_LoadSet(mdl_buf, size);
		while (FS_LoadSync() >= 0);
	}
	
	// モデル読み込み＆セット
	set_kms((DG_DEF *)mdl_buf);

//	g_pInput = new CDXInput();

#if 0
	printf("G_VERTICES = %d\n", COUNTOF(g_Vertex00));
	printf("G_VERTICES = %d\n", COUNTOF(g_Vertex01));
	g_pDraw->SetVertex(D3DPT_TRIANGLESTRIP, g_Vertex00,
					   COUNTOF(g_Vertex00), FindTexture(7832316));
	g_pDraw->SetVertex(D3DPT_TRIANGLESTRIP, g_Vertex01,
					   COUNTOF(g_Vertex01), FindTexture(7832316));
#endif
}


#define TRIANGLE_NUM 100
#define RECTANGLE_NUM 10000

VOID TEST_MainLoop()
{
	static D3DXMATRIX matWorld;
	static D3DXVECTOR3 vecMove(0.0f, 0.0f, 0.0f);
	static FLOAT speed = 100.0f;
	static const LONG lCenter = 16;
	static FLOAT rotx = 0.0f;
	static FLOAT roty = 0.0f;
	static FLOAT rotz = 0.0f;	

#if 0
	DIJOYSTATE2 *p_Js;
	g_pInput->UpdateInputState();
	p_Js = g_pInput->GetJoyState();

	g_pGame->m_pFont->DrawText(100, 100,
							   "X : %d\n"
							   "Y : %d\n"
							   "Z      : %d\n"
							   "Z(ROT) : %d\n",
							   p_Js->lX,
							   p_Js->lY,
							   p_Js->lZ,
							   p_Js->lRz);
	if (p_Js->lX > lCenter) {
		vecMove.x += speed;
		rotx += speed;		
	} else if (p_Js->lX < - lCenter) {
		vecMove.x -= speed;
		rotx -= speed;		
	}
	if (p_Js->lY > lCenter) {
		vecMove.y += speed;
		roty += speed;
	} else if (p_Js->lY < - lCenter) {
		vecMove.y -= speed;
		roty -= speed;		
	}
	if (p_Js->lRz > lCenter) {
		vecMove.z += speed;
		rotz += speed;		
	} else if (p_Js->lRz < - lCenter) {
		vecMove.z -= speed;
		rotz -= speed;
	}
	if (p_Js->lZ > lCenter) {
		roty += speed;
	} else if (p_Js->lZ < - lCenter) {
		roty -= speed;		
	}
#else
	//roty += speed / 3.0f;
	//rotx += speed / 100.0f;	
#endif

	rotx += 0.01f;
	
	// 適当に動かしてみる
	D3DXMatrixTranslation(&matWorld, vecMove.x, vecMove.y, vecMove.z);
	D3DXMatrixTranslation(&matWorld, 0.0f, 0.0f, 0.0f);
	// 適当に動かしてみる
	D3DXMATRIX matTmp;
	D3DXMatrixRotationYawPitchRoll(&matTmp, rotx, roty, rotz);
	//D3DXMatrixRotationY(&matWorld, roty);
	//D3DXMatrixRotationZ(&matWorld, roty);	
	matWorld = matWorld * matTmp;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	
	g_pCamera->SetTransform(g_pd3dDevice);

	// 各VertexBufferをレンダリング
	g_pDraw->Render();
}

VOID TEST_Exit()
{
	delete g_pCamera;
	delete g_pDraw;

//	delete g_pInput;
}

