#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.10 2002/05/10 02:15:18 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = mode.a

# オブジェクトリスト
OBJECTS =	photo_term.o \
		photo_pic.o \
		photo_itp.o \
		photo_panel.o \
		photo_puppet.o \
		photo_disp.o \
		console.o

XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


photo_term.o:	photo_term.c photo_term.h photo_pic.h photo_itp.h \
		photo_panel.h photo_config.h photo_codes.h
photo_pic.o:	photo_pic.c photo_pic.h photo_config.h
photo_itp.o:	photo_itp.c photo_itp.h photo_config.h photo_syntax.h
photo_panel.o:	photo_panel.c photo_panel.h photo_config.h
photo_puppet.o:	photo_puppet.c photo_puppet.h photo_config.h photo_term.h \
		photo_pic.h photo_itp.h photo_panel.h photo_codes.h

photo_disp.o:	photo_disp.c photo_disp.h photo_pic.h photo_codes.h

console.o:	console.c console.h photo_codes.h con_act.h

# for XBOX
photo_term.obj:	photo_term.c photo_term.h photo_pic.h photo_itp.h \
		photo_panel.h photo_config.h photo_codes.h
photo_pic.obj:	photo_pic.c photo_pic.h photo_config.h
photo_itp.obj:	photo_itp.c photo_itp.h photo_config.h photo_syntax.h
photo_panel.obj:	photo_panel.c photo_panel.h photo_config.h
photo_puppet.obj:	photo_puppet.c photo_puppet.h photo_config.h photo_term.h \
		photo_pic.h photo_itp.h photo_panel.h photo_codes.h

photo_disp.obj:	photo_disp.c photo_disp.h photo_pic.h photo_codes.h

console.obj:	console.c console.h photo_codes.h con_act.h


photo_codes.h:	photo_codes.lst
	macrobld.pl < $< > $@

con_act.h:	con_act.lst
	macrobld.pl < $< > $@

photo_syntax.h:	photo_syntax.lst
	macrobld.pl < $< > $@
