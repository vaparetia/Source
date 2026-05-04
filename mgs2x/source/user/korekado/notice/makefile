#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.23 2002/02/01 08:50:22 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = 	notice.o thkendam.o thknoise.o thkcorps.o thkdamag.o \
			thkdiscv.o thkhold.o thkoboro.o thktouch.o thkfound.o \
			thktrace.o thkbox.o thkaccid.o thkunifo.o thkinves.o \
			thkacrepo.o thkradio.o

XOBJECTS = 	notice.obj thkendam.obj thknoise.obj thkcorps.obj thkdamag.obj \
			thkdiscv.obj thkhold.obj thkoboro.obj thktouch.obj thkfound.obj \
			thktrace.obj thkbox.obj thkaccid.obj thkunifo.obj thkinves.obj \
			thkacrepo.obj thkradio.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../watcher ../enemy ../conv ../action
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) : ../enemy/enemy.h ../action/action.h

thkendam.o : thkendam.c radio.c move.c
thknoise.o : thknoise.c radio.c move.c
thkcorps.o : thkcorps.c radio.c move.c
thkdamag.o : thkdamag.c radio.c move.c
thkdiscv.o : thkdiscv.c radio.c move.c
thkhold.o : thkhold.c radio.c move.c
thkoboro.o : thkoboro.c radio.c move.c
thktouch.o : thktouch.c radio.c move.c
thkfound.o : thkfound.c radio.c move.c
thktrace.o : thktrace.c radio.c move.c
thkbox.o : thkbox.c radio.c move.c
thkaccid.o : thkaccid.c radio.c move.c
thkunifo.o : thkunifo.c radio.c move.c
thkinves.o : thkinves.c radio.c move.c
thkacrepo.o : thkacrepo.c radio.c move.c
thkradio.o : thkradio.c radio.c move.c

thkendam.obj : thkendam.c radio.c move.c
thknoise.obj : thknoise.c radio.c move.c
thkcorps.obj : thkcorps.c radio.c move.c
thkdamag.obj : thkdamag.c radio.c move.c
thkdiscv.obj : thkdiscv.c radio.c move.c
thkhold.obj : thkhold.c radio.c move.c
thkoboro.obj : thkoboro.c radio.c move.c
thktouch.obj : thktouch.c radio.c move.c
thkfound.obj : thkfound.c radio.c move.c
thktrace.obj : thktrace.c radio.c move.c
thkbox.obj : thkbox.c radio.c move.c
thkaccid.obj : thkaccid.c radio.c move.c
thkunifo.obj : thkunifo.c radio.c move.c
thkinves.obj : thkinves.c radio.c move.c
thkacrepo.obj : thkacrepo.c radio.c move.c
thkradio.obj : thkradio.c radio.c move.c
