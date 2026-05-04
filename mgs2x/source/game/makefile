#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.56 2002/08/27 01:43:33 usr03700 Exp $
#

# ベースディレクトリ
BASEDIR = ..

# ライブラリ名

LIBNAME = game.a

# オブジェクトリスト
OBJECTS = extern.o gamed.o chara.o loader.o control.o object.o target.o script.o \
	  camerad.o camera.o cam_util.o cam_set.o hmd_util.o debugcam.o map.o trg_view.o \
	  menupri.o homing.o viewer.o area.o gameinit.o radarctrl.o g_sound.o font.o \
	  mapcnct.o padctrl.o sparkd.o navi.o over.o syscllbk.o enefind.o getitem.o \
	  strctrl.o sdstream.o dumstream.o interupt.o mtstream.o caption.o varsys.o resource.o \
	  vibrate.o hazard.o ipustream.o debugmenu.o debugconf.o jimaku.o no_use.o unused.o \
	  sejimaku.o vrfunc.o vrsetfnc.o

XOBJECTS = area.obj cam_set.obj cam_util.obj camera.obj camerad.obj caption.obj \
	  chara.obj control.obj debugcam.obj debugmenu.obj dumstream.obj extern.obj \
	  font.obj gamed.obj gameinit.obj getitem.obj hazard.obj loader.obj map.obj \
	  mapcnct.obj xmenupri.obj no_use.obj object.obj padctrl.obj radarctrl.obj \
	  resource.obj script.obj sparkd.obj syscllbk.obj target.obj temp.obj viewer_cpp.obj \
	  debugconf.obj enefind.obj g_soundx.obj homing.obj \
	  interupt.obj mtstream.obj navi.obj over.obj sejimaku.obj strctrl.obj \
	  trg_view.obj unused.obj varsys.obj vibrate.obj sdstream.obj jimaku.obj \
	  vrfunc.obj vrsetfnc.obj cristream.obj discon.obj

#まだやっていないもの T.Morita
#	dumpdma.obj ipustream.obj
#	


# ここでのローカルフラグリスト

### CRI Middleware (ipicstream.c で使用 )

LOCAL_INCLUDES = $(addprefix $(BASEDIR)/, $(CRI_INCLUDE))
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体


include $(BASEDIR)/makefile.sub

# 依存関係のリスト

stage.obj : stage.c $(wildcard ../stage/*.c)

getitem.o : getitem.c player.h
sejimaku.o : sejimaku.c sejimaku.h
map.o :		map.c ../system/libhzx/libhzx.h



