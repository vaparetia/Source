#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = skoba.a
# オブジェクトリスト
OBJECTS = encute.o albam.o albam_name.o gameover.o dogtag2.o \
		  name_layout.o demo_c4.o black_rect.o mc_mng.o album_mng.o \
		  mc_dogtag_mng.o dogtag_mng.o world_map_bug_layout.o \
		  world_map_layout.o world_map_sub_layout.o photo_save.o \
		  web_site.o previous_story.o demo_nkt_smoke.o \
		  clear_result.o boss_result.o boss_result2.o boss_etc.o boss_pause.o \
		  zako_title.o photo_view.o boss_etc2.o

XOBJECTS =   demo_c4.obj demo_nkt_smoke.obj \
		  black_rect.obj \
		  encute.obj name_layout.obj \
		  world_map_bug_layout.obj \
		  world_map_layout.obj world_map_sub_layout.obj \
		  web_site.obj previous_story.obj gameover.obj \
		albam.obj albam_name.obj album_mng.obj boss_etc.obj boss_pause.obj clear_result.obj mc_dogtag_mng.obj \
		dogtag2.obj  mc_mng.obj dogtag_mng.obj photo_save.obj  boss_result.obj boss_result2.obj \
		zako_title.obj  photo_view.obj  boss_etc2.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
$(OBJECTS): ../test/etc.h


