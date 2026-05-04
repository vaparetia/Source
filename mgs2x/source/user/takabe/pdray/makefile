#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/10/22 K.Takabe
# $Id: makefile,v 1.1.1.3 2002/11/19 11:51:23 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = takabe.a

# オブジェクトリスト
.CFILES = pdray.c pdr_move.c pdr_act.c pdr_coll.c r_server.c \
	pdr_thk.c pdr_thk1.c pdr_thk2.c pdr_thk3.c pdr_efct.c pdr_ik.c \
	pdr_stage.c smkreact.c pdr_demo.c pdr_fenc.c \
	ef_laser.c wp_misle.c disparc.c ef_smoke.c ef_shock.c wp_bul.c ef_spark.c ef_cloud.c \
	ef_muzle.c ef_splsh.c ef_spl2.c ef_spk2.c ef_brmsl.c ef_smk2.c ef_smk3.c ef_shck2.c \
	ef_spl3.c ef_clud2.c
NO_USEOBJ = 
# オブジェクトリスト
OBJECTS = $(.CFILES:.c=.o)
XOBJECTS = $(.CFILES:.c=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
pdray.o: pdray.c pdray.h r_common.h

pdr_move.o: pdr_move.c pdray.h r_common.h

pdr_act.o: pdr_act.c pdray.h r_common.h

pdr_coll.o: pdr_coll.c pdray.h r_common.h

r_server.o: r_server.c r_common.h

pdr_thk.o: pdr_thk.c pdray.h r_common.h

pdr_thk1.o: pdr_thk1.c pdray.h r_common.h

pdr_thk2.o: pdr_thk2.c pdray.h r_common.h

pdr_thk3.o: pdr_thk3.c pdray.h r_common.h

pdr_efct.o: pdr_efct.c pdray.h r_common.h

pdr_ik.o: pdr_efct.c pdray.h r_common.h
