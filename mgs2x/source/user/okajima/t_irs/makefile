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
OBJECTS = irs.o irs_break.o trap_c4.o

else
# ----------------- Xbox
XOBJECTS = irs.obj irs_break.obj trap_c4.obj

endif



# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


