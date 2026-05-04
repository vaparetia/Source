#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.12 2002/05/22 06:12:09 usr02774 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kira.a

# オブジェクトリスト
#OBJECTS = radar1.o vuradar.o radsprite.o
OBJECTS = radar.o vuradar.o radsprite.o \
		bomb_sensor.o bomb.o bomb_child.o outrange.o \
		rdr_movie.o


.LOCAL_VSHADER = vs_radar.vsh vs_radar2.vsh
.LOCAL_PSHADER = 
XOBJECTS = radar.obj radsprite.obj \
		bomb_sensor.obj bomb.obj bomb_child.obj outrange.obj \
		rdr_movie.obj xradar.obj \
		$(.LOCAL_VSHADER:.vsh=_vsh.obj) \
		$(.LOCAL_PSHADER:.psh=_psh.obj)
#vuradar.o 



# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

vuradar.o:	vuradar.dsm vuradar.vu1
radar.o:	radar.c radsprite.h radsize.h radar_config.h radar_macro.h
radsprite.o:	radsprite.c radsprite.h 
bomb_sensor.o:	bomb_sensor.c bomb_sensor.h radar_config.h radar_macro.h
bomb.o:		bomb.c bomb_sensor.h bomb.h
bomb_child.o:	bomb_child.c bomb_sensor.h bomb_child.h
outrange.o:	outrange.c outrange.h bomb.h

rdr_movie.o:	rdr_movie.c rdr_movie.h


%_vsh.c : %_vsh.v
	echo unsigned char VERTEX_SHADER_$*\[\] = { > $@
	od -An -tx1 -v $< | sed 's/\([a-z0-9][a-z0-9]\)/0x\1,/g'>> $@
	echo }\; >> $@

%_psh.c : %_psh.u
	echo unsigned char PIXEL_SHADER_$*\[\] = { > $@
	od -An -tx1 -v $< | sed 's/\([a-z0-9][a-z0-9]\)/0x\1,/g' >> $@
	echo }\; >> $@


%_vsh.v : %.vsh
	xsasm -nologo -D KP_XBOX $<  $@

%_psh.u : %.psh
	xsasm -nologo -D KP_XBOX $<  $@
