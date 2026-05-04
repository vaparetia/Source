#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.16 2002/07/23 12:26:53 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# ターゲット

DEPENDSTAMP = .effectdemo.stm
CNFFILE = demoeffect.cnf

# コマンド

.PHONY : all lib clean

all : $(DEPENDSTAMP)
	@make -f makefile.compile all

lib : $(DEPENDSTAMP)
	@make -f makefile.compile lib

$(DEPENDSTAMP) : $(CNFFILE)
	@test -e pool || mkdir pool
	ef_create $(CNFFILE) pool
	@cd pool ; (for i in *.c ; do diff $$i ../$$i > /dev/null || cp $$i .. ; done)
	@touch $(DEPENDSTAMP)

clean :
	make -f makefile.compile clean 
	-rm -rf *.o *.c core *~ $(DEPENDSTAMP)
	-rm -rf pool

.DEFAULT:
	cd $(BASEDIR); make $@

# 依存関係のリスト

