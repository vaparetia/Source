#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima.a

# オブジェクトリスト

ifneq ($(MGS2_XBOX),1)  
# ----------------- PS2
OBJECTS = flashlight.o fl_light_ini.o fl_light_target.o fl_light_act.o

else
# ----------------- Xbox
XOBJECTS = flashlight.obj fl_light_ini.obj fl_light_target.obj fl_light_act.obj

endif



# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

flashlight.o:	./flashlight.h
fl_light_ini.o:	./flashlight.h
fl_light_act.o:	./flashlight.h
fl_light_target.o:	./flashlight.h

