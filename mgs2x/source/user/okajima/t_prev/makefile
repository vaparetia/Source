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

# ----------------- PS2版設定

OBJECTS =	manhat3d.o brooklyn.o bonbori_yw.o bonbori_red.o \
			sky_prev.o thund_flash.o fog_wall.o \
			sky_column.o sky_prev_alpha.o

else

XOBJECTS = 	manhat3d.obj brooklyn.obj bonbori_yw.obj bonbori_red.obj \
			sky_prev.obj thund_flash.obj fog_wall.obj \
			sky_column.obj sky_prev_alpha.obj

endif



# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

