#
# makefile for MGS2 scenario file
#
#	2000/02/01 S.Hirano
#	$Id: makefile,v 1.416 2002/10/08 01:06:53 usr01475 Exp $
#

# -------------------------------------------- リミット定義

# シナリオ変数のサイズの最大値(これを変更する時は、植原まで連絡して下さい)
VARIABLE_SIZE = 7168
LOCAL_VARIABLE_SIZE = 512

# -------------------------------------------- ソースファイル定義

# 共通ソース

# GCLソースファイルを追加するときはこれを編集。
include ../list/stage.inc

BOSS_SCENARIOS = r_sna_b.boss.gcl r_rai_b.boss.gcl w00b.boss.gcl w03b.boss.gcl \
				 w20c.boss.gcl w25a.boss.gcl w31c.boss.gcl w44a.boss.gcl w46a.boss.gcl \
				 w61a.boss.gcl boss.gcl

ifeq ($(MGS2_DISC),2)
SCENARIOS = $(addsuffix .gcl, $(STAGES))
else
# disc1にのみ必要。
SCENARIOS = $(addsuffix .gcl, $(STAGES)) $(BOSS_SCENARIOS)
endif


# 無線ソース

COMMON_CODECS =  t_otacon.cdc t_demo.cdc t_save.cdc photo_term.cdc p_phone.cdc \
				p_demo.cdc p_colonel.cdc p_emma.cdc p_otacon.cdc p_peter.cdc \
				p_rose.cdc p_snake.cdc

DEMO_CODECS   = $(COMMON_CODECS)

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

# 仕向け値オプション

AREAOPT = -D AREA_US

ifeq ($(MGS2_AREA),JAPAN)
AREAOPT = -D AREA_JAPAN
endif
ifeq ($(MGS2_AREA),EU)
AREAOPT = -D AREA_EU
endif

LANGOPT := $(LANGOPT) $(AREAOPT)

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

# メイキング用オプション
ifeq ($(MGS2_VRTRIAL),1)
MAKINGOPT = -D MGS2_VRTRIAL=1
else
MAKINGOPT = -D MGS2_VRTRIAL=0
endif

ifeq ($(MGS2_ENG_VRTRIAL),1)
MAKINGOPT := $(MAKINGOPT) -D MGS2_ENG_VRTRIAL=1
else
MAKINGOPT := $(MAKINGOPT) -D MGS2_ENG_VRTRIAL=0
endif

CODECS = $(COMMON_CODECS) $(CONCEPT_CODECS)

ifeq ($(MGS2_DESIGN),1)

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

ifeq ($(FIX_MODE),1)
LOCAL_OPTIONS = -D MGS2_SCN=0 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=1 -D FIX_MODE=1
else
LOCAL_OPTIONS = -D MGS2_SCN=0 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=1

ifeq ($(MGS2_SCN),1)
# シナリオ班作成環境
LOCAL_OPTIONS = -D MGS2_SCN=1 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=1 $(E3OPT)
CODECS = $(COMMON_CODECS) $(CONCEPT_CODECS)
endif

ifeq ($(MGS2_SCN),2)
# デモ班作成環境
LOCAL_OPTIONS = -D MGS2_SCN=0 -D MGS2_DEMO=1 -D MGS2_TANKER=1 -D MGS2_PLANT=1 $(E3OPT)
CODECS = $(DEMO_CODECS)
endif

ifeq ($(MGS2_SCN),3)
# モーション班作成環境
LOCAL_OPTIONS = -D MGS2_SCN=1 -D MGS2_DEMO=0 -D MGS2_TANKER=1 -D MGS2_PLANT=1 $(E3OPT)
CODECS = $(COMMON_CODECS) $(CONCEPT_CODECS)
endif

endif

endif


# -------------------------------------------------------------------------------

LKOPT = -l $(VARIABLE_SIZE) $(LOCAL_VARIABLE_SIZE)

VARIABLE_SYM = variable.sym

LKOPT := $(LKOPT) -fv $(VARIABLE_SYM)

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

ifeq ($(MGS2_AREA),EU)
LOCAL_OPTIONS := $(LOCAL_OPTIONS) -n 1:2:3:4:5
else
LOCAL_OPTIONS := $(LOCAL_OPTIONS) -n 1:7
endif

# --------------------------------------------------- マクロ定義

.SUFFIXES: .gco .gcl .cdc .cdo

#CDLISTS     = vox.lst demo.lst movie.lst

VOXLIST		= vox.lst
VOX2LIST	= vox2.lst
DEMOLIST	= demo.lst
DEMO2LIST	= demo2.lst
MOVIELIST	= movie.lst movievr.lst
MOVIEVRLIST = movievr.lst
MISCLIST	= tsymbol.lst
FACELIST    = face.lst

CODECLIST	= codec.lst
DEPENDS     = make.depend

CODEC_DEPEND_LIST = $(VOXLIST) $(MOVIELIST) $(FACELIST) $(MISCLIST)
GCL_DEPEND_LIST = $(VOXLIST) $(DEMOLIST) $(MOVIELIST) $(FACELIST) $(MISCLIST) $(CODECLIST) $(DEMO2LIST) $(MOVIVRLIST)

ifeq ($(MGS2_DISC),2)
GCL_DEPEND_LIST = $(VOX2LIST) $(DEMO2LIST) $(MOVIEVRLIST) $(FACELIST) $(MISCLIST) $(CODECLIST)
endif
ifeq ($(MGS2_DISC),)
GCL_DEPEND_LIST := $(GCL_DEPEND_LIST) $(VOX2LIST) $(DEMO2LIST) $(MOVIEVRLIST)
endif

ifeq (1,1)
#サブスタンスではフォントはＰＡＬも共通で使う。
CODECFONTCHECK = -fe /u/develop/mj001data/fix.xbox/binary/keepfontsub.row 512 120 0 4
else

ifeq ($(MGS2_LANG),ENGLISH)
ifeq ($(MGS2_VMODE),PAL)
CODECFONTCHECK = -fe /u/develop/mj001data/fix/binary/keepfontec.row 520 120 0 4
else
CODECFONTCHECK = -fe /u/develop/mj001data/fix/binary/keepfont.row 520 120 0 12
endif
else
CODECFONTCHECK = -fc /u/develop/mj001data/fix/binary/keepfont.row 472 120 0 12
endif

endif

ifneq ($(MGS2_XBOX),1)
OPTIONS = -f /u/develop/mj001b/tools/bin/linux $(MAKINGOPT) $(LOCAL_OPTIONS) $(LANGOPT) $(VMODEOPT) -DPSX2
else
# xbox:とりあえず
OPTIONS =  -f /u/develop/mj001b/exports/tools/bin/mgs2xbox $(MAKINGOPT) $(LOCAL_OPTIONS) $(LANGOPT) $(VMODEOPT) -D_XBOX -DXBOX
endif


GCLOPTIONS   = $(OPTIONS) -d command.def $(addprefix -t ,$(GCL_DEPEND_LIST))
CODECOPTIONS = $(OPTIONS) -b -d codec.def $(addprefix -t ,$(CODEC_DEPEND_LIST)) \
				$(CODECFONTCHECK)

#ifeq ($(MGS2_DISC),2)
ifeq ($(MGS2_SUBSTANCE),1)
GCLOPTIONS := $(GCLOPTIONS) -wt
endif

OBJECTS = $(SCENARIOS:.gcl=.gco)
CODECOBJS = $(CODECS:.cdc=.cdo)

# -------------------------------------------------- ターゲット定義

all : compiled $(VARIABLE_SYM)

.gcl.gco :
	gclconv $(GCLOPTIONS) $<

.cdc.cdo :
	gclconv $(CODECOPTIONS) $< -o $@

CODECOPTS = -bl

# 無線関係
ifneq ($(MGS2_VRTRIAL),1)
$(CODECOBJS) : $(CODEC_DEPEND_LIST)
$(CODECLIST) : $(CODECOBJS)
	gclk $(CODECOPTS) $(LKOPT) $(CODECOBJS) -o $(CODECPATH)/codec.dat -t $(CODECLIST)
endif

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

$(VARIABLE_SYM):
	make clean all

select.gcl : vox_test.h
vox_test.h : vox.lst
	cat $< | sort +1 | awk -f conv_voxtest.awk > $@

ifeq ($(MGS2_XBOX),1)
dogtag_data.h : dogtag_data.txt conv_dogtag.awk
	cat $< | awk -f conv_dogtag.awk > $@
dogtag_view.h : dogtag_data.txt conv_dogtagview.awk
	cat $< | awk -f conv_dogtagview.awk > $@
else
dogtag_data.h : dogtag_data_2002.txt conv_w_dogtag.awk
	cat $< | awk -f conv_w_dogtag.awk > $@
dogtag_view.h : dogtag_data_2002.txt conv_w_dogtagview.awk
	cat $< | awk -f conv_w_dogtagview.awk > $@
endif

# 自動生成ヘッダ
AUTO_HEADERS = dogtag_data.h dogtag_view.h vox_test.h

# ------------------------------------------------------ コマンド定義

.PHONY : clean stage depend allstage source list strcheck tags

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

tags :
	gcltags.pl $(SCENARIOS) $(CODECS) > TAGS
htags :
	gcltags.pl -H $(SCENARIOS) $(CODECS) > HTAGS

.DEFAULT:
	mergescene -i $(STAGEPATH)/$@/$@.scn -o $(STAGEPATH)/$@/_$@.scn
	scn2hzx	-i $(STAGEPATH)/$@/_$@.scn -o $(STAGEPATH)/$@/$@.hzx

ifneq ($(wildcard $(DEPENDS)),)
include $(DEPENDS)
endif

ifeq ($(MGS2_VRTRIAL),1)
tr01a.gco : tr01a.gcl vs03a.gcl
tr02a.gco : tr02a.gcl vs08a.gcl
tr03a.gco : tr03a.gcl wp01a.gcl
tr04a.gco : tr04a.gcl wp73a.gcl
tr05a.gco : tr05a.gcl sp06a.gcl
endif
