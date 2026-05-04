#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#	エフェクト関連
# 2001/07/03 Masafumi Okuta
# $Id: makefile,v 1.31 2002/02/14 05:07:52 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名
LIBNAME = okuta.a

# オブジェクトリスト
OBJECTS = kmdanna.o kmdnnfloor.o kmdnncam.o kmfeather.o parfeather.o powspplit.o wdustmng.o waterdust.o maoparticle.o waterfall.o waterbubble.o waterspread.o waterdropmng.o waterdrop.o waterlinefall.o waterripple.o bloodweep.o bloodfloor.o forttear.o ventilatorlit.o hexagonal.o concspread.o charaspread.o plasmaevade.o electrofield.o concripple.o oneripple.o onespread.o drowbubble.o wormcorpse.o namedetect.o 

XOBJECTS = kmdanna.obj kmdnnfloor.obj kmdnncam.obj kmfeather.obj parfeather.obj powspplit.obj wdustmng.obj waterdust.obj maoparticle.obj waterfall.obj waterbubble.obj waterspread.obj waterdropmng.obj waterdrop.obj waterlinefall.obj waterripple.obj bloodweep.obj bloodfloor.obj forttear.obj ventilatorlit.obj hexagonal.obj concspread.obj charaspread.obj plasmaevade.obj electrofield.obj concripple.obj oneripple.obj onespread.obj drowbubble.obj wormcorpse.obj namedetect.obj


# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
kmdanna.o : kmdanna.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

kmdnnfloor.o : kmdnnfloor.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

kmdnncam.o : kmdnncam.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

kmfeather.o : kmfeather.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

parfeather.o : parfeather.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

powspplit.o : powspplit.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

namedetect.o : namedetect.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

wormcorpse.o : wormcorpse.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

wdustmng.o : wdustmng.c waterdust.c wdust.h ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

waterdust.o : waterdust.c wdustmng.c wdust.h ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

maoparticle.o : maoparticle.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

waterfall.o : waterfall.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

waterbubble.o : waterbubble.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

waterspread.o : waterspread.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

waterdropmng.o : waterdropmng.c waterdrop.h ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h 

waterdrop.o : waterdrop.c waterdrop.h ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

waterlinefall.o : waterlinefall.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h

waterripple.o : waterripple.c ../conv/maoutil.h ../conv/algfunc.h ../../okajima/effect2/bubble.h ../../okajima/etc/ok_util.h ../../mode/demo/eft_con.h

bloodweep.o : bloodweep.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h ../../okajima/effect/blood.h

bloodfloor.o : bloodfloor.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h ../../okajima/effect/blood.h

forttear.o : forttear.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h 

ventilatorlit.o : ventilatorlit.c ../conv/maoutil.h ../conv/algfunc.h ../../korekado/conv/define.h 

hexagonal.o : hexagonal.c ../conv/maoutil.h ../conv/algfunc.h

concspread.o : concspread.c ../conv/maoutil.h ../conv/algfunc.h

charaspread.o : charaspread.c ../conv/maoutil.h ../conv/algfunc.h

plasmaevade.o : plasmaevade.c ../conv/maoutil.h ../conv/algfunc.h

electrofield.o : electrofield.c ../conv/maoutil.h ../conv/algfunc.h

concripple.o : concripple.c ../conv/maoutil.h ../conv/algfunc.h

oneripple.o : oneripple.c ../conv/maoutil.h ../conv/algfunc.h

onespread.o : onespread.c ../conv/maoutil.h ../conv/algfunc.h

drowbubble.o : drowbubble.c ../conv/maoutil.h ../conv/algfunc.h 
