#
# makefile for MGS2 scenario file
#
#	2000/02/01 S.Hirano
#	$Id: makefile,v 1.1 2002/02/01 06:08:31 usr01475 Exp $
#

# -------------------------------------------- リミット定義

# シナリオ変数のサイズの最大値(これを変更する時は、植原まで連絡して下さい)
VARIABLE_SIZE = 6144

# -------------------------------------------- ソースファイル定義

# 共通ソース
# ※体験版用は別のところで設定する
INIT_SCENARIOS = init.gcl select.gcl r_tnk0.gcl r_plt0.gcl r_plt1.gcl r_plt2.gcl r_plt3.gcl \
				 r_plt4.gcl r_plt5.gcl r_plt6.gcl r_plt7.gcl r_plt8.gcl r_plt9.gcl r_sna_b.gcl r_rai_b.gcl \
				 r_sna_b.boss.gcl r_rai_b.boss.gcl r_title.gcl

TITLE_SCENARIOS = n_title.gcl museum.gcl ending.gcl


COMMON_TANKER_SCENARIOS =	w00a.gcl w00b.gcl w00c.gcl \
							w01a.gcl w01f.gcl w01b.gcl w01c.gcl w01d.gcl w01e.gcl \
							w02a.gcl \
							w03a.gcl w03b.gcl \
							w04a.gcl w04b.gcl w04c.gcl \
							w00b.boss.gcl

COMMON_PLANT_SCENARIOS =	w11a.gcl w11b.gcl w11c.gcl w12a.gcl w12c.gcl w12b.gcl w13a.gcl w13b.gcl w14a.gcl w15a.gcl w15b.gcl \
							w16a.gcl w16b.gcl w17a.gcl w18a.gcl w19a.gcl w20a.gcl w20b.gcl w20c.gcl w20d.gcl \
							w21a.gcl w21b.gcl w22a.gcl w23a.gcl w23b.gcl \
							w24a.gcl w24b.gcl w24c.gcl w24d.gcl w24e.gcl w25a.gcl w25b.gcl w25c.gcl w25d.gcl w28a.gcl \
							w31a.gcl w31b.gcl w31c.gcl w31d.gcl w31f.gcl w32a.gcl w32b.gcl \
							w41a.gcl w42a.gcl w43a.gcl w44a.gcl w45a.gcl w46a.gcl \
							w51a.gcl w61a.gcl wmovie.gcl webdemo.gcl \
							w20c.boss.gcl w25a.boss.gcl w31c.boss.gcl w46a.boss.gcl w61a.boss.gcl \
							boss.gcl

DEBUG_SCENARIOS = debug.gcl

E3_SCENARIOS = etitle.gcl e01a.gcl e04b.gcl e11c.gcl e14a.gcl e43a.gcl e45a.gcl

# ゲーム用の基本ステージをパック(デバッグ用、Ｅ３用、通常)
ifeq ($(MGS2_DEBUG),1)
# デバッグ用・プレゼン用のシナリオ構成（タンカーやプラントはなし）
COMMON_SCENARIOS = $(TITLE_SCENARIOS) $(DEBUG_SCENARIOS)
else
ifeq ($(MGS2_E3),1)
# E3用ステージを追加したもの
COMMON_SCENARIOS = $(TITLE_SCENARIOS) $(COMMON_TANKER_SCENARIOS) $(COMMON_PLANT_SCENARIOS) $(E3_SCENARIOS)
else
COMMON_SCENARIOS = $(TITLE_SCENARIOS) $(COMMON_TANKER_SCENARIOS) $(COMMON_PLANT_SCENARIOS)
endif
endif


# デモステージ
COMMON_DEMO_TANKER_SCENARIOS =	d00t.gcl d01t.gcl d04t.gcl d05t.gcl \
								d10t.gcl d11t.gcl d12t.gcl d12t3.gcl d12t4.gcl d13t.gcl d14t.gcl

COMMON_DEMO_PLANT_SCENARIOS =	d001p01.gcl d001p02.gcl d005p01.gcl d005p03.gcl \
								d010p01.gcl d012p01.gcl d014p01.gcl \
								d021p01.gcl \
								d036p03.gcl d036p05.gcl \
								d045p01.gcl d046p01.gcl \
								d053p01.gcl d055p01.gcl \
								d063p01.gcl d065p02.gcl \
								d070p01.gcl d070p09.gcl d070px9.gcl d078p01.gcl \
								d080p01.gcl d080p06.gcl d080p07.gcl d080p08.gcl d082p01.gcl

COMMON_DEMO_SCENARIOS = $(COMMON_DEMO_TANKER_SCENARIOS) $(COMMON_DEMO_PLANT_SCENARIOS)


COMMON_CODECS    =  t_otacon.cdc t_demo.cdc t_save.cdc photo_term.cdc p_phone.cdc \
										p_demo.cdc p_colonel.cdc p_emma.cdc p_otacon.cdc p_peter.cdc p_rose.cdc p_snake.cdc


#################################################  for XBOX Custumize ##########################################
# /* 2002.01.31  Temporarily Deleted by T.Morita */
INIT_SCENARIOS   = init.gcl 
TITLE_SCENARIOS = 
COMMON_TANKER_SCENARIOS = t00a.gcl 
COMMON_PLANT_SCENARIOS = 
COMMON_DEMO_SCENARIOS =
COMMON_CODECS    =  
##################################################　ここまで          ##########################################


# シナリオ班ローカルのソース
CONCEPT_SCENARIOS =

CONCEPT_CODECS   =

# デモ班ローカルのソース
ifndef DEMO_SCENARIOS
# コンパイルするgclを直接指定するため。mgdemo対応。2000/09/30 田中(信) 追加
# DEMO_SCENARIOS =  d00a.gcl d00b.gcl
DEMO_SCENARIOS =  t00a1D.d00a.gcl t01a1D.w00a.gcl t04a1D.w00a.gcl t05a1D.w00b.gcl \
				  w03b.gcl \
				  w04a.gcl w04c.gcl \
				  t10a1D.w04a.gcl t11a1D.w04c.gcl t12a1D.w04c.gcl t12a3D.w04c.gcl t12a4D.w04c.gcl t13a1D.w04c.gcl \
				  P001_01_p01.w10a.gcl P001_01_p02.w11a.gcl P005_01_p01.w12a.gcl P005_04_p03.w10a.gcl\
				  P010_01_p01.w14a.gcl P012_01_p01.w15a.gcl P014_01_p01.w16a.gcl \
				  P021_01_p01.w20c.gcl \
				  P036_04_p03.w24c.gcl P036_06_p05.w20c.gcl \
				  P045_01_p01.w25a.gcl P046_01_p01.w25b.gcl \
				  P053_01_p01.w31b.gcl P055_01_p01.w31c.gcl \
				  P063_01_p01.w25d.gcl P065_03_p02.w32a.gcl \
				  P070_01_p01.w24d.gcl P070_09_p09.w24d.gcl P070_13_p09.w41a.gcl P078_01_p01.w45a.gcl \
				  P080_01_p01.w46a.gcl P080_13_p06.w51a.gcl P080_13_p07b.w51a.gcl P080_14_p08.w61a.gcl \
				  P082_01_p01.w61a.gcl \
				  w11a.gcl w11c.gcl w18a.gcl \
				  w24b.gcl \
				  w31a.gcl w31b.gcl w31c.gcl w32a.gcl w32b.gcl \
				  w41a.gcl w42a.gcl w43a.gcl \
				  e04b.gcl \
				  tmp_P001_01_p01.w10a.gcl

endif

DEMO_CODECS   = $(COMMON_CODECS)


# 体験版用のステージ
# 体験版には余分なデータを入れないために独自に設定する
TRIAL_SCENARIOS = init.gcl select.gcl title.gcl special.gcl \
                  w00a.gcl w00b.gcl \
                  w01a.gcl w01f.gcl w01b.gcl w01c.gcl w01d.gcl w01e.gcl

# TRIAL_DEMO_SCENARIOS =	d00a0.gcl d00a1.gcl d00a4.gcl d00a5.gcl
TRIAL_DEMO_SCENARIOS =	d00t.gcl d01t.gcl d04t.gcl d05t.gcl

# 無線ファイル
TRIAL_CODECS    =  t_otacon_t.cdc t_demo_t.cdc


EU_TRIAL_INIT_SCENARIOS = r_title.gcl r_tnk0.gcl init.gcl select.gcl

EU_TRIAL_SCENARIOS = n_title.gcl \
		w00a.gcl w00b.gcl w00c.gcl w01a.gcl w01f.gcl \
		w01b.gcl w01c.gcl w01d.gcl w01e.gcl w02a.gcl

EU_TRIAL_DEMO_SCENARIOS = d00t.gcl d01t.gcl d04t.gcl d05t.gcl

EU_TRIAL_CODEC = t_otacon.cdc t_demo.cdc t_save.cdc photo_term.cdc

# ------------------------------------------ 各環境ごとの定義

# 言語オプション
ifeq ($(MGS2_LANG),ENGLISH)

ifeq ($(MGS2_PAL),SPANISH)

LANGOPT =  -D ENGLISH -D SPANISH

else

ifeq ($(MGS2_PAL),ITALIAN)

LANGOPT =  -D ENGLISH -D ITALIAN

else

ifeq ($(MGS2_PAL),EU)

LANGOPT = -D ENGLISH -DEU

else

LANGOPT = -D ENGLISH

endif

endif

endif

else
LANGOPT = -D JAPANESE
endif

# TVモードオプション
ifeq ($(MGS2_VMODE),PAL)
VMODEOPT = -D PAL
else
VMODEOPT = -D NTSC
endif

# E3用オプション
ifeq ($(MGS2_E3),1)
E3OPT = -D MGS2_E3=1
else
E3OPT = -D MGS2_E3=0
endif

ifeq ($(MGS2_DESIGN),1)

# --------------------

# デザイナープレビュー環境

SOURCEPATH = $(PREVIEW_DATA)
STAGEPATH = $(PREVIEW_DATA)/stage
CODECPATH = .

LOCAL_OPTIONS = -I $(PREVIEW_DATA) -D DESIGN_PREVIEW -D MGS2_TRIAL=0 -D MGS2_SCN=0 -D MGS2_DEMO=0 -D MGS2_TANKER=0 -D MGS2_PLANT=0
SCENARIOS = init.gcl r_tnk0.gcl $(PREVIEW_DATA)/preview.gcl
CODECS = t_otacon.cdc
DEMOLISTPATH = ../list/demolist

else # デザイナープレビュー環境以外

SOURCEPATH = ../source/stage
STAGEPATH = ../cdrom.img/stage
CODECPATH = ../cdrom.img
DEMOLISTPATH = ../list/demolist

ifeq ($(MGS2_TRIAL),1)

# 体験版環境
ifeq ($(FIX_MODE),1)
LOCAL_OPTIONS = -D FIX_MODE=1 -D MGS2_TRIAL=1 -D MGS2_SCN=0 -D MGS2_DEMO=0
else
LOCAL_OPTIONS = -D MGS2_SCN=0 -D MGS2_DEMO=0
endif

SCENARIOS = $(TRIAL_SCENARIOS) $(TRIAL_DEMO_SCENARIOS)
CODECS = $(TRIAL_CODECS)

else
ifeq ($(MGS2_TANKER),1)
#タンカー編のみ
LOCAL_OPTIONS = -D MGS2_SCN=1 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=0 $(E3OPT)

SCENARIOS = $(INIT_SCENARIOS) $(TITLE_SCENARIOS) $(COMMON_TANKER_SCENARIOS) $(COMMON_DEMO_TANKER_SCENARIOS)

CODECS = $(COMMON_CODECS) $(CONCEPT_CODECS)

else
#プラント編のみ
ifeq ($(MGS2_PLANT),1)
LOCAL_OPTIONS = -D MGS2_SCN=1 -D MGS2_DEMO=0 -D MGS2_TANKER=0 -D MGS2_PLANT=1 $(E3OPT)

SCENARIOS = $(INIT_SCENARIOS) $(TITLE_SCENARIOS) $(COMMON_PLANT_SCENARIOS) $(COMMON_DEMO_PLANT_SCENARIOS)

CODECS = $(COMMON_CODECS) $(CONCEPT_CODECS)

else
ifeq ($(MGS2_SCN),1)
# シナリオ班作成環境
LOCAL_OPTIONS = -D MGS2_SCN=1 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=1 $(E3OPT)

SCENARIOS = $(INIT_SCENARIOS) $(COMMON_SCENARIOS) $(CONCEPT_SCENARIOS) $(COMMON_DEMO_SCENARIOS)

CODECS = $(COMMON_CODECS) $(CONCEPT_CODECS)

else
ifeq ($(MGS2_SCN),2)
# デモ班作成環境
LOCAL_OPTIONS = -D MGS2_SCN=0 -D MGS2_DEMO=1 -D MGS2_TANKER=1 -D MGS2_PLANT=1 $(E3OPT)

SCENARIOS = $(INIT_SCENARIOS) $(DEMO_SCENARIOS)

CODECS = $(DEMO_CODECS)

else
ifeq ($(MGS2_SCN),3)
# モーション班作成環境
LOCAL_OPTIONS = -D MGS2_SCN=1 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=1 $(E3OPT)

SCENARIOS = $(INIT_SCENARIOS) $(COMMON_SCENARIOS) $(CONCEPT_SCENARIOS) $(COMMON_DEMO_SCENARIOS)

CODECS = $(COMMON_CODECS) $(CONCEPT_CODECS)

else
# リリース環境
ifeq ($(FIX_MODE),1)
LOCAL_OPTIONS = -D MGS2_SCN=0 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=1 -D FIX_MODE=1
else
LOCAL_OPTIONS = -D MGS2_SCN=0 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=1
endif

SCENARIOS = $(INIT_SCENARIOS) $(COMMON_SCENARIOS) $(COMMON_DEMO_SCENARIOS)

CODECS = $(COMMON_CODECS)

endif
endif
endif
endif
endif
endif
endif

LKOPT = -l $(VARIABLE_SIZE)

# MGS2STAGEで特定のステージだけ
ifneq ($(MGS2STAGE),)
ifneq ($(MGS2STAGE),all)
SCENARIOS = $(addsuffix .gcl, init $(MGS2RESIDENT:.bin=) $(MGS2STAGE:.bin=))
LKOPT := $(LKOPT) -w
endif
endif

ifneq ($(MGS2_SCN),2)
LKOPT := $(LKOPT) -e
endif

#LOCAL_OPTIONS := $(LOCAL_OPTIONS) -D MGS2_MEMCARD=1
#SCENARIOS := $(SCENARIOS) memcard.gcl

LOCAL_OPTIONS := $(LOCAL_OPTIONS) -D MGS2_MEMCARD=0
SCENARIOS_ := $(filter-out %.boss.gcl, $(SCENARIOS) )


# TGS2001秋用の変更を有効にする
ifeq ($(TGS2001F),1)
LOCAL_OPTIONS := $(LOCAL_OPTIONS) -D TGS2001F=1
endif


# ヨーロッパ向け最終体験版用
ifeq ($(MGS2_EU_TRIAL),ENGLISH)
LOCAL_OPTIONS := $(LOCAL_OPTIONS) -D EU_TRIAL=1
SCENARIOS = $(EU_TRIAL_INIT_SCENARIOS) $(EU_TRIAL_SCENARIOS) $(EU_TRIAL_DEMO_SCENARIOS)
CODECS = $(EU_TRIAL_CODEC)
endif

ifeq ($(MGS2_EU_TRIAL),GERMAN)
LOCAL_OPTIONS := $(LOCAL_OPTIONS) -D EU_TRIAL=1 -D GERMAN_TRIAL=1
SCENARIOS = $(EU_TRIAL_INIT_SCENARIOS) $(EU_TRIAL_SCENARIOS) $(EU_TRIAL_DEMO_SCENARIOS)
CODECS = $(EU_TRIAL_CODEC)
endif


# --------------------------------------------------- マクロ定義

.SUFFIXES: .gco .gcl .cdc .cdo

#CDLISTS     = vox.lst demo.lst movie.lst

VOXLIST		= vox.lst
DEMOLIST	= demo.lst
MOVIELIST	= movie.lst
MISCLIST	= tsymbol.lst
FACELIST    = face.lst

CODECLIST	= codec.lst
DEPENDS     = make.depend

CODEC_DEPEND_LIST = $(VOXLIST) $(MOVIELIST) $(FACELIST) $(MISCLIST)
GCL_DEPEND_LIST = $(VOXLIST) $(DEMOLIST) $(MOVIELIST) $(FACELIST) $(MISCLIST) $(CODECLIST)

OPTIONS = -f /u/develop/mj001b/exports/tools/bin/mgs2xbox $(LOCAL_OPTIONS) $(LANGOPT) $(VMODEOPT)

ifeq ($(MGS2_LANG),ENGLISH)
ifeq ($(MGS2_VMODE),PAL)
CODECFONTCHECK = -fe /u/develop/mj001data/fix.xbox/binary/keepfontec.row 520 120 0 4
else
CODECFONTCHECK = -fe /u/develop/mj001data/fix.xbox/binary/keepfont.row 520 120 0 12
endif
else
CODECFONTCHECK = -fc /u/develop/mj001data/fix.xbox/binary/keepfont.row 472 120 0 12
endif

GCLOPTIONS   = $(OPTIONS) -d command.def $(addprefix -t ,$(GCL_DEPEND_LIST))
CODECOPTIONS = $(OPTIONS) -b -d codec.def $(addprefix -t ,$(CODEC_DEPEND_LIST)) \
				$(CODECFONTCHECK)

OBJECTS = $(SCENARIOS:.gcl=.gco)
CODECOBJS = $(CODECS:.cdc=.cdo)

# -------------------------------------------------- ターゲット定義

all : compiled variable.sym

.gcl.gco :
	gclconv $(GCLOPTIONS) $<

.cdc.cdo :
	gclconv $(CODECOPTIONS) $< -o $@

ifeq ($(MGS2_VMODE),PAL)
CODECOPTS = -bl
else
CODECOPTS = -b
endif

# 無線関係
$(CODECOBJS) : $(CODEC_DEPEND_LIST)
$(CODECLIST) : $(CODECOBJS)
	gclk $(CODECOPTS) $(LKOPT) $(CODECOBJS) -o $(CODECPATH)/codec.dat -t $(CODECLIST)

# シナリオ関係
$(OBJECTS)   : %.gco : %.gcl $(GCL_DEPEND_LIST) dogtag_data.h dogtag_view.h

compiled : $(OBJECTS) linkvar.h
	gclk $(LKOPT) -p $(STAGEPATH) $(OBJECTS)
	touch compiled

$(CDLISTS) :
	echo '' > $@				# 暫定

#	@echo "$@ is not exist"
#	@echo "please 'make newcd'"
#	@false

linkvar.h : linkvar.def
	gclk -i

variable.sym :
	make clean all

select.gcl : vox_test.h
vox_test.h : vox.lst
	cat $< | sort +1 | awk -f conv_voxtest.awk > $@

dogtag_data.h : dogtag_data.txt conv_dogtag.awk
	cat $< | awk -f conv_dogtag.awk > $@
dogtag_view.h : dogtag_data.txt conv_dogtagview.awk
	cat $< | awk -f conv_dogtagview.awk > $@

# 自動生成ヘッダ
AUTO_HEADERS = dogtag_data.h dogtag_view.h vox_test.h

# ------------------------------------------------------ コマンド定義

.PHONY : clean stage depend allstage source list strcheck

clean : depend
#	-rm -f *.gco compiled tsymbol.def linkvar.h .#*
	-rm -f $(OBJECTS) $(CODECOBJS) $(AUTO_HEADERS) compiled linkvar.h _ChangeLog .#*

ifeq ($(MGS2_DEMO),1)

stage : $(AUTO_HEADERS)
	@echo "Demo Compile."
	-(for i in $(SCENARIOS_:.gcl=); do gclconv $(GCLOPTIONS) $$i.gcl -m $(SOURCEPATH) -l $(DEMOLISTPATH) ; done)

else

stage : $(AUTO_HEADERS)
	-(for i in $(SCENARIOS_); do gclconv $(GCLOPTIONS) $$i -m $(SOURCEPATH)  -l $(DEMOLISTPATH) ; done)

endif

onestage:
	-(for i in $(STAGE); do gclconv $(GCLOPTIONS) $$i.gcl -m $(SOURCEPATH)  -l $(DEMOLISTPATH) ; done)

depend :
	-rm $(DEPENDS)
	(for i in $(SCENARIOS); do if gclconv $(GCLOPTIONS) $$i -M$(DEPENDS); then true; else rm $(DEPENDS); break; fi ; done)

allstage :
	gclconv $(GCLOPTIONS) init.gcl -A $(SOURCEPATH)

source :
	cd ../module; $(MAKE) all
	cd ../source; $(MAKE) all

list :
	cd ../list; cvs update; $(MAKE) all

strcheck :
	gclconv $(GCLOPTIONS) $(PARAM)

go : run

run download module:
	cd $(SOURCEPATH); make $@

debug :
	gclconv $(GCLOPTIONS) -p $(STAGE).gcl
	cd ../source; $(MAKE) debug

log :
	-cvs2cl.pl -f _ChangeLog -d -l "-d '>2 months ago'"

newcd :
	ps2file $(addprefix /mgs2/,$(VOXLIST) $(DEMOLIST) $(MOVIELIST))

.DEFAULT:
	mergescene -i $(STAGEPATH)/$@/$@.scn -o $(STAGEPATH)/$@/_$@.scn
	scn2hzx	-i $(STAGEPATH)/$@/_$@.scn -o $(STAGEPATH)/$@/$@.hzx

ifneq ($(wildcard $(DEPENDS)),)
include $(DEPENDS)
endif
