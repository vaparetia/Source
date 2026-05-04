#
# ステージ定義ファイル
#

# -------------------------- 通常ゲームディスク用

INIT_STAGES = init select

RESIDENT_STAGES = \
		r_title r_tnk0 r_plt0 r_plt1 r_plt2 r_plt3 r_plt4 r_plt5 \
		r_plt6 r_plt7 r_plt8 r_plt9 r_plt10 r_plt11 r_plt12 r_plt13 \
		r_sna_b r_rai_b  r_plt_s r_tnk_r

TITLE_STAGES = n_title museum ending

COMMON_TANKER_STAGES = \
		w00a w00b w00c w01a w01f w01b w01c w01d w01e w02a w03a w03b w04a w04b w04c

COMMON_PLANT_STAGES = \
		w11a w11b w11c w12a w12c w12b w13a w13b w14a w15a w15b w16a \
		w16b w17a w18a w19a w20a w20b w20c w20d w21a w21b w22a w23a w23b \
		w24a w24b w24c w24d w24e w25a w25b w25c w25d w28a \
		w31a w31b w31c w31d w31f w32a w32b \
		w41a w42a w43a w44a w45a w46a \
		w51a w61a wmovie webdemo boss

# デモステージ

COMMON_DEMO_TANKER_STAGES =	d00t d01t d04t d05t \
							d10t d11t d12t d12t3 d12t4 d13t d14t

COMMON_DEMO_PLANT_STAGES =	d001p01 d001p02 d005p01 d005p03 \
							d010p01 d012p01 d014p01 \
							d021p01 \
							d036p03 d036p05 \
							d045p01 d046p01 \
							d053p01 d055p01 \
							d063p01 d065p02 \
							d070p01 d070p09 d070px9 d078p01 \
							d080p01 d080p06 d080p07 d080p08 d082p01

COMMON_DEMO_STAGES = $(COMMON_DEMO_TANKER_STAGES) $(COMMON_DEMO_PLANT_STAGES)

# ------------------------ VR ディスク用

RESIDENT_STAGES_VR = r_title r_vr_s r_vr_r r_vr_p r_vr_t r_vr_b r_vr_1 r_vr_rp r_vr_sp r_vr_x

COMMON_ANOTER_STAGES = \
		a00a a00b a00c a01a a01f a01b a01c a01d a01e a02a a02b a03a a03b \
		a12a a12b a13a a13b a13c a14a a14b a15a a15b a16a a17a a18a a19a a20a a20b a20c a20e \
		a21a a21b a22a a22b a23a a23b a24a a24b a24c a24f a24d a24g a25a a25d a28a a31a a31b a31c \
		a41a a41b a42a a43a a45a a46a a61a \
		tales ta02a ta24a tsp03a tvs03a twp03a tvs05a tvs08a ta31a ta12a twp43a ta20a tvs06a ta01f ta00a ta01a ta01b ta01c ta01d ta01e ta22a twp34a ta42a

COMMON_VR_STAGES = \
		vs01a vs02a vs03a vs04a vs05a vs06a vs07a vs08a vs09a vs10a \
		wp01a wp02a wp03a wp04a wp05a \
		wp11a wp12a wp13a wp14a wp15a \
		wp21a wp22a wp23a wp24a wp25a \
		wp31a wp32a wp33a wp34a wp35a \
		wp41a wp42a wp43a wp44a wp45a \
		wp51a wp52a wp53a wp54a wp55a \
		wp61a wp62a wp63a wp64a wp65a \
		wp71a wp72a wp73a wp74a wp75a \
		sp01a sp02a sp03a sp06a sp07a sp08a \
		sp21a sp22a sp24a sp25a \
		st01a st02a st03a st04a st05a \
		mselect sselect

COMMON_VR_TRIAL_STAGES = r_title r_vr_s r_vr_r r_vr_b init mkselct \
							tr01a tr02a tr03a tr04a tr05a trmenu0 trmenu1
# ------------------------ プレビュー用

DEMO_STAGES =  	t00a1D.d00a t01a1D.w00a t04a1D.w00a t05a1D.w00b \
				w03b w04a w04c \
				t10a1D.w04a t11a1D.w04c t12a1D.w04c t12a3D.w04c t12a4D.w04c t13a1D.w04c t14a1D.w04c \
				P001_01_p01.w10a P001_01_p02.w11a \
				P005_01_p01.w12a P005_04_p03.w10a \
				P010_01_p01.w14a P012_01_p01.w15a P014_01_p01.w16a \
				P021_01_p01.w20c \
				P036_04_p03.w24c \
				P036_06_p05.w20c \
				P045_01_p01.w25a P046_01_p01.w25b \
				P053_01_p01.w31b P055_01_p01.w31c \
				P063_01_P01.w25d \
				P065_03_p02.w32a \
				P070_01_p01.w24d P070_09_p09.w24d P070_13_p09.w41a P078_01_p01.w45a \
				P080_01_p01.w46a P080_11_p06.w51a P080_13_p07b.w51a P080_14_p08.w61a P082_01_p01.w61a \
				w11a w11c w18a \
				w24b w28a\
				w31a w31b w31c w32a w32b \
				w41a w42a w43a w45a w46a w51a w61a \
				e04b \
				tmp_P001_01_p01.w10a


COMMON_STAGES = $(TITLE_STAGES) $(COMMON_TANKER_STAGES) $(COMMON_PLANT_STAGES)

# memory card
MEMCARD_STAGE = memcard

# concept maps
CONCEPT_STAGES = 

# STAGESの設定。下に行くほど上書きする。

# DISC 1の設定

# 標準環境
STAGES1 = $(RESIDENT_STAGES) $(COMMON_STAGES) $(COMMON_DEMO_STAGES)

ifeq ($(MGS2_TANKER),1)	# タンカー編のみ
STAGES1 = $(RESIDENT_STAGES) $(TITLE_STAGES) $(COMMON_TANKER_STAGES) $(COMMON_DEMO_TANKER_STAGES)
endif
ifeq ($(MGS2_PLANT),1)	# プラント編のみ
STAGES1 = $(RESIDENT_STAGES) $(TITLE_STAGES) $(COMMON_PLANT_STAGES) $(COMMON_DEMO_PLANT_STAGES)
endif

ifeq ($(MGS2_SCN),1)	# シナリオ班作成環境
STAGES1 = $(RESIDENT_STAGES) $(COMMON_STAGES) $(CONCEPT_STAGES) $(COMMON_DEMO_STAGES) $(MEMCARD_STAGE)
endif
ifeq ($(MGS2_SCN),2)	# デモ班作成環境
STAGES1 = $(RESIDENT_STAGES) $(DEMO_STAGES)
endif
ifeq ($(MGS2_SCN),3)	# モーション班作成環境
STAGES1 = $(RESIDENT_STAGES) $(COMMON_STAGES) $(CONCEPT_STAGES) $(COMMON_DEMO_STAGES)
endif


# DISC 2の設定

# 標準環境
STAGES2 =  $(RESIDENT_STAGES_VR) $(COMMON_ANOTER_STAGES) $(COMMON_VR_STAGES)

ifeq ($(MGS2_VRTRIAL),1)
STAGES2 =  $(COMMON_VR_TRIAL_STAGES)
endif
#

# 環境変数による構築ステージの切り替え

# 重複を取り除く
STAGES_ALL = $(STAGES1) $(filter-out $(STAGES1), $(STAGES2))

# -------------- 最終設定

# PS2 2層DVDで2重持ちするステージ
ifeq ($(MGS2_DISC),1)
#STAGES1 := $(filter-out n_title, $(STAGES1))
endif
ifeq ($(MGS2_DISC),2)
STAGES2 := $(STAGES2) n_title
DUALLAYER_STAGE = w51a d080p06 wmovie d080p07 d080p08 d082p01 n_title
endif


# デフォルト
STAGES = $(STAGES_ALL)

ifeq ($(MGS2_DISC),1)
STAGES = $(STAGES1)
endif
ifeq ($(MGS2_DISC),2)
STAGES = $(STAGES2)
endif

STAGES := $(STAGES) $(INIT_STAGES)

