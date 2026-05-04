# Makefile for MGS2 Sub Module
#
# 1999/03/12 K.Uehara
# $Id: makefile,v 1.44 2002/02/14 10:58:27 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ..
MODULEDIR = ../lib

# ステージディレクトリ
ifeq ($(MGS2_DESIGN),1)
	STAGEPATH = $(PREVIEW_DATA)/stage
	# 常駐部elfファイル
	BASEFILE = $(PREVIEW_DATA)/mgs2.elf
	# オブジェクトリスト
	STAGES = $(PREVIEW_DATA)/init.c $(PREVIEW_DATA)/r_tnk0.c $(PREVIEW_DATA)/preview.c
else

ifeq ($(MGS2_DEMO),1)
# デモ実行環境

ifdef DEMO_DATA
# Hunter上の変形ディレクトリ構成用
	STAGEPATH = $(DEMO_DATA)/stage
	# 常駐部elfファイル
	BASEFILE = $(DEMO_DATA)/mgs2.elf
	# オブジェクトリスト
	STAGES = $(wildcard *.c)
else
# 通常のディレクトリ構成
	STAGEPATH = ../../cdrom.img/stage
	# 常駐部elfファイル
	BASEFILE = ../mgs2.elf
	# オブジェクトリスト
	STAGES = $(wildcard *.c)
endif

else
	STAGEPATH = ../../cdrom.img/stage
	# 常駐部elfファイル
	BASEFILE = ../mgs2.elf
	# オブジェクトリスト
	STAGES = $(wildcard *.c)
endif
endif

# コマンドファイル
CMDFILE = stage.cmd
BINHEAD = binhead.o

MODULELIB   = $(addprefix $(MODULEDIR)/, $(addsuffix .a, $(MODULES)))

# TARGET

TARGETS = $(STAGES:.c=.elf)

# COMMAND

.PHONY : all lib clean

all :
	cd $(BASEDIR); make

ifeq ($(TARGETS),)
lib :
	@echo !!!! no stage file !!! try Again !!!!
	$(MAKE) stage -C ../../scn
	$(MAKE) link
else
ifeq ($(MGS2STAGE),)
lib : $(TARGETS)
else
ifeq ($(MGS2STAGE),all)
lib : $(TARGETS)
else
#lib : $(addsuffix .bin, $(basename init $(MGS2STAGE)))
lib : $(addsuffix .elf, $(filter-out %.boss, init $(MGS2RESIDENT:.bin=) $(MGS2STAGE:.bin=)))
endif
endif
endif


ifeq ($(MGS2_DESIGN),1)

clean :
	-rm -f $(PREVIEW_DATA)/*.c $(PREVIEW_DATA)/*.o $(PREVIEW_DATA)/*.bin $(PREVIEW_DATA)/*.elf

else

clean :
	-rm -f *.c *.o *.obj *.bin *.elf

endif


.DEFAULT:
	cd $(BASEDIR); make $@

STAGELINK = 1
include $(BASEDIR)/makefile.def

$(BINHEAD) : $(BINHEAD:.o=.s)

$(TARGETS) : %.elf : %.o $(CMDFILE) $(BASEFILE) $(BINHEAD)
ifeq ($(MGS2_DESIGN),1)
	echo Link $(STAGEPATH)/${*F}/${@F}
	$(LD) -o $@ -Wl,-R,$(BASEFILE) -T $(CMDFILE) \
		 -Wl,-defsym,HEAPSIZE=$(SYSTEM_HEAP_SIZE) $(BINHEAD) $< \
		 -Wl,-\( $(MODULELIB) -Wl,-\) $(LIBS) -nostartfiles -L$(LIBDIR) -L$(MODULEDIR)
	-$(OBJCOPY) -S -O binary $@ $(@:.elf=.bin)
	cp -f $(@:.elf=.bin) $(STAGEPATH)/${*F}/
else
	@echo Link $(STAGEPATH)/$*/$(@:.elf=.bin)
	@$(LD) -o $@ -Wl,-R,$(BASEFILE) -T $(CMDFILE) \
		 -Wl,-defsym,HEAPSIZE=$(SYSTEM_HEAP_SIZE),-G0 $(BINHEAD) $< \
		 -Wl,-\( $(MODULELIB) -Wl,-\) $(LIBS) -nostartfiles -L$(LIBDIR) -L$(MODULEDIR)
	-@$(OBJCOPY) -S -O binary $@ $(STAGEPATH)/$*/$(@:.elf=.bin)
endif
