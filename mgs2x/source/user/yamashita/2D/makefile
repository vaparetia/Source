#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 2002/04/10 S.Yamashita
# $Id: makefile,v 1.1.1.3 2002/11/19 11:51:41 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名
LIBNAME = yamashita.a

# オブジェクトリスト
ifeq ($(MGS2_VRTRIAL),1)
OBJECTS =	layoutman.o \
			lom_get.o \
			lom_new.o \
			lom_load.o \
			lom_get_cur.o \
			lom_set_cur.o \
			lom_spr_get.o \
			lom_spr_set.o \
			vr_screen.o \
			vr_window.o \
			vr_trial_clear.o \
			vr_select.o \
			vr_select_act.o \
			vr_select_cb.o \
			vr_select_pla.o \
			vr_select_mis.o \
			vr_select_mod.o \
			vr_select_wea.o \
			vr_select_lev.o \
			vr_select_dbg.o \
			vr_trial_pause.o \
			vr_map_3d.o \
			msn.o \
			vr_trial_select.o \
			vr_trial_title.o \
			vr_world_map_layout.o \
			key_con_sel.o \
			st_select.o \
			vr_pause2.o \
			vr_clear_code.o 
else
OBJECTS =	layoutman.o \
			lom_get.o \
			lom_new.o \
			lom_load.o \
			lom_get_cur.o \
			lom_set_cur.o \
			lom_spr_get.o \
			lom_spr_set.o \
			vr_screen.o \
			vr_window.o \
			vr_clear.o \
			vr_select.o \
			vr_select_act.o \
			vr_select_cb.o \
			vr_select_pla.o \
			vr_select_mis.o \
			vr_select_mod.o \
			vr_select_wea.o \
			vr_select_lev.o \
			vr_select_dbg.o \
			vr_pause.o \
			vr_map_3d.o \
			msn.o \
			vr_world_map_layout.o \
			key_con_sel.o \
			st_select.o \
			vr_pause2.o \
			vr_clear_code.o 
endif

XOBJECTS =	layoutman.obj \
			lom_get.obj \
			lom_new.obj \
			lom_load.obj \
			lom_get_cur.obj \
			lom_set_cur.obj \
			lom_spr_get.obj \
			lom_spr_set.obj \
			vr_screen.obj \
			vr_window.obj \
			vr_clear.obj \
			vr_select.obj \
			vr_select_act.obj \
			vr_select_cb.obj \
			vr_select_pla.obj \
			vr_select_mis.obj \
			vr_select_mod.obj \
			vr_select_wea.obj \
			vr_select_lev.obj \
			vr_select_dbg.obj \
			vr_pause.obj \
			vr_map_3d.obj \
			msn.obj \
			vr_world_map_layout.obj \
			key_con_sel.obj \
			st_select.obj \
			vr_pause2.obj \
			vr_clear_code.obj 

# ここでのローカルフラグリスト
LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体
include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) : msn.h vr_def.h layoutman.h lom.h ../../../../scn/msn_def_hiscore.h ../../../../scn/msn_stage_id2.h vr_select.h
$(XOBJECTS) : msn.h vr_def.h layoutman.h lom.h ../../../../scn/msn_def_hiscore.h ../../../../scn/msn_stage_id2.h vr_select.h
