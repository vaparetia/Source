#
# Makefile for MGS2
#	Main module
#
# 1999/03/12 K.Uehara
# $Id: makefile,v 1.7 2002/11/04 11:43:40 takaki Exp $
#

BASEDIR = .

ifeq ($(MGS2_DESIGN),1)
CDROMIMG = $(PREVIEW_DATA)
STAGEBIN = .
else

ifeq ($(MGS2_DEMO),1)
# ¥Ç¥â¼Â¹Ô´Ä¶­

ifdef DEMO_DATA
# HunterÍÑÊÑ·Á¥Ç¥£¥E¯¥È¥E½À®ÍÑ
	CDROMIMG = $(DEMO_DATA)
	STAGEBIN = ../source/stage
else
# ÄÌ¾EÇ¥£¥E¯¥È¥E½À®ÍÑ
	CDROMIMG = ../cdrom.img
	STAGEBIN = ../source/stage
endif

else
	CDROMIMG = ../cdrom.img
	STAGEBIN = ../source/stage
endif
endif

ifdef MGS2LOCAL
STAGEMAKEOPT = -j 4
else
STAGEMAKEOPT =
endif

# TARGET

ifneq ($(MGS2_XBOX),1)	# ------------------------- PS2 VERSION

TARGET      = mgs2
OBJS        = main/main.o
COMMONHEAD  = include/commdef.h
STARTUP		= crt0.o
STARTUPSRC	= $(LIBDIR)/crt0.s

else					# -------------------------- XBOX VERSION

TARGET      = mgs2
ifeq ($(MGS2_WIN),1)	# ------------------------- Windows VERSION
TARGET_SSE  = $(TARGET)_sse
OBJS        = main/mainw.obj
else
OBJS        = main/mainx.obj
endif
COMMONHEAD  = include/commdef.h
STARTUP		= 
STARTUPSRC	= 

XBOX_DIR = xe:\\mgs2x
XCDROMIMG = $(subst /,\\, $(CDROMIMG))

endif

SELECTOBJ_PATH = system/libfs

MODULESELECT   = hd hdc cd usb cdd usbd hdu hdud

DIRECTORYS  = system game user

 # ½çÈÖ¤Ï½ÅÍ×¤Ê¤Î¤Ç¡¢ÃúÌÕ¡£
ifeq ($(MGS2_WIN),1)
MODULES =  mode \
		nobu korekado sonoyama sigeno yamashita uehara \
		yano shibata satoyoshi okuta kunibe \
		okajima3 okajima2 \
		morita kano okajima skoba kira takabe \
		game libutl libmt libfs libdg libgv libhzx libgcl libx2w libwbc
MODULES_SSE := $(addsuffix _sse, $(MODULES))
else
MODULES =  mode \
		nobu korekado sonoyama sigeno yamashita uehara \
		yano shibata satoyoshi okuta kunibe \
		okajima3 okajima2 \
		morita kano okajima skoba kira takabe \
		game libutl libmt libfs libdg libgv libhzx libgcl 
endif

ifeq ($(MGS2_XBOX),1)
MODULES     := $(filter-out nobu,$(MODULES))
MODULES_SSE := $(filter-out nobu_sse,$(MODULES_SSE))
endif

MODULEDIR     = ./lib
MODULELIB     = $(addprefix $(MODULEDIR)/, $(addsuffix .a, $(MODULES)))
MODULELIB_SSE = $(addprefix $(MODULEDIR)/, $(addsuffix .a, $(MODULES_SSE)))
FSLIB		  = $(addprefix $(MODULEDIR)/, libfs.a )
FSLIB_SSE	  = $(addprefix $(MODULEDIR)/, libfs_sse.a )

export MODULES

# IOP MODULES --------------------------

ifneq ($(MGS2_XBOX),1)
SYSMODULEDIR = ../module
else
ifneq ($(MGS2_WIN),1)
SYSMODULEDIR = ../xmodule
else
SYSMODULEDIR = ../wmodule
endif
endif

SYSIRX_PATH = /usr/local/sce/iop/modules
USRIRX_PATH = $(SYSMODULEDIR)/irx

ifeq ($(MGS2_DESIGN),1)
IOPPAKPATH = ../cdrom.img/modules
else
IOPPAKPATH = $(CDROMIMG)/modules
endif

IOPPAKS = $(addprefix $(IOPPAKPATH)/, module.ipk modhd.ipk modusb.ipk modhdu.ipk syshd.ipk )
IOPPAKDEFS = -D SYSPATH=$(SYSIRX_PATH) -D USRPATH=$(USRIRX_PATH)
IOPPAKLIST = main/ioppak.ils

# COMMANDS -----------------------------

.PHONY : default check all run debug go clean _clean stage needfull

default: check all

 # ¥Õ¥E³¥ó¥Ñ¥¤¥E½Ìó¤¬¤«¤«¤Ã¤¿¾Eç¡¢¥á¥Ã¥»¡¼¥¸¤òÉ½¼¨¤·¤ÆÄä»ß
check:
	@cmp --q .need_full .done_full; \
	if [ $$? != 0 ]; then \
		 echo "IT NEEDS TO BE FULL COMPILED. Do \"make clean\" first.";\
		 false;\
	fi
ifeq ($(MGS2_XBOX),1)
	@cmp --q .need_full.xbox .done_full.xbox; \
	if [ $$? != 0 ]; then \
		 echo "IT NEEDS TO BE FULL COMPILED. Do \"make clean\" first.";\
		 false;\
	fi
endif
	@cmp --q .need_module .done_module; \
	if [ $$? != 0 ]; then \
		cd $(dir $(SYSMODULEDIR)); cvs update -dP $(notdir $(SYSMODULEDIR)); \
		cd $(notdir $(SYSMODULEDIR)); $(MAKE) all; \
		cd ../source; cp .need_module .done_module; \
	fi
	@cmp --q .need_libclr .done_libclr; \
	if [ $$? != 0 ]; then \
		rm -f lib/*.a; \
		cp .need_libclr .done_libclr; \
	fi

 # ¥³¥ó¥Ñ¥¤¥EèÍý¤Î¼ÂÂÎ
ifeq ($(MGS2_XBOX),1)
all : downloadcri hd
else
all : hd
endif

 # other default setting
include makefile.def

 # ¼Â¹Ô

 # PS2HOST¤òÀßÄê¤¹¤EÈ¡¢¤½¤Î¥Þ¥·¥ó¤Çµ¯Æ°
ifeq ($(PS2HOST),)
HOSTOPTION = 
else
HOSTOPTION = -d $(PS2HOST)
endif

ifeq ($(MGS2SELPRINT),)
	MGS2SELPRINT = .*
endif

go : run


ifneq ($(MGS2_XBOX),1)	# ---------------------------- PS2 VERSION

run: $(IOPPAKS)
	cd $(CDROMIMG); $(DSEDB) $(HOSTOPTION) -r "run $(TARGET).elf;\
		printf \"MARK value is %s\n\" *GV_DebugMes;\
		record __log; printf \"sload $(STAGEBIN)/%s.elf\n\" area_directory; record;\
		source __log; bt; printf \">>>>>\"; list; " | egrep '$(MGS2SELPRINT)' ; rm -f __log


 # ¥Ç¥Ð¥Ã¥¬µ¯Æ°¡¢¥Ð¥Ã¥¯¥È¥E¼¥¹É½¼¨
debug:
	cd $(CDROMIMG); $(DSEDB) $(HOSTOPTION) -nr "sload mgs2.elf;\
		sload $(STAGEBIN)/$(STAGE).elf;dr;bt"

else					# ---------------------------- XBOX VERSION

run:
	xbcp -x $(XBOX_TARGET) -r -d -t -y $(TARGET).xbe $(XBOX_DIR)\\$(TARGET).xbe
	xbreboot -x $(XBOX_TARGET) $(XBOX_DIR)\\$(TARGET).xbe

copy:
	xbcp -x $(XBOX_TARGET) -r -d -t -y $(XCDROMIMG)\\stage $(XBOX_DIR)
	xbcp -x $(XBOX_TARGET) -r -d -t -y $(XCDROMIMG)\\*.dat $(XBOX_DIR)

debug:
	c:/Program\ Files/Microsoft\ Visual\ Studio/Common/MSDev98/Bin/msdev ../mgs2xdbg.dsw &

endif

 # clean
clean:
	-rm $(TARGET).* $(OBJS) $(STARTUP)
	-rm $(CDROMIMG)/$(TARGET).elf
	-(for i in $(DIRECTORYS); do $(MAKE) clean -C $$i; done)
	-make clean -C stage
	-rm $(MODULELIB)
	-rm $(MODULELIB_SSE)
	-find . -name ".#*" -type f -exec rm -f {} \;
	-find . -name "_ChangeLog" -type f -exec rm -f {} \;
	-cp .need_full .done_full
ifeq ($(MGS2_XBOX),1)
	-cp .need_full.xbox .done_full.xbox
endif

_clean :
	-rm $(TARGET).* $(OBJS) $(STARTUP)
	-rm $(CDROMIMG)/$(TARGET).elf
	-(for i in $(DIRECTORYS); do $(MAKE) clean -C $$i; done)
	-rm $(MODULELIB)
	-rm $(MODULELIB_SSE)
	-cp .need_full .done_full
ifeq ($(MGS2_XBOX),1)
	-cp .need_full.xbox .done_full.xbox
endif


 # ¥Õ¥E³¥ó¥Ñ¥¤¥E½ÌE

ifeq ($(MGS2_XBOX),1)
USRLIST = /u/develop/mj001b/exports/tools/bin/mgs2xbox
else
USRLIST = /u/home/user/usrlist
endif
MAILMSG = mailmsg -d -u$(USRLIST)/mgs2user.lst -a$(USRLIST)/mgs2user.ali

needfull:
	date +%s > .need_full
	-cp .need_full .done_full
	cvs commit -m "" .need_full
	echo `date` "mgs2x/source NEEDFULL from" `whoami` $(MESSAGE) | nkf -s | $(MAILMSG) prog scn

ifeq ($(MGS2_XBOX),1)
needfullxbox:
	date +%s > .need_full.xbox
	-cp .need_full.xbox .done_full.xbox
	cvs commit -m "" .need_full.xbox
	echo `date` "mgs2x/source XBOX NEEDFULL from" `whoami` $(MESSAGE) | nkf -s | $(MAILMSG) prog

 # CRI lib, include ¤Î¥À¥¦¥ó¥ú½¼¥É
downloadcri:
	@if [ ! -d $(BASEDIR)/$(CRIDIR) ]; then \
		mkdir -p $(BASEDIR)/$(CRIDIR); \
		cp -ur /u/develop/mj001b/exports/tools/cri/xb/mwlib/include $(BASEDIR)/$(CRIDIR); \
		cp -ur /u/develop/mj001b/exports/tools/cri/xb/mwlib/xb $(BASEDIR)/$(CRIDIR); \
	fi
endif

 # ¥â¥¸¥å¡¼¥E³¥ó¥Ñ¥¤¥E½ÌE
needmodule:
	date +%s > .need_module
	-cp .need_module .done_module
	cvs commit -m "" .need_module

needlibclear:
	rm -f lib/*.a
	date +%s > .need_libclr
	-cp .need_libclr .done_libclr
	cvs commit -m "" .need_libclr

tags:
	-rm TAGS
	find . -name "*.[ch]" -exec etags -a -oTAGS {} \;

# MODULES -----------------------------

.PHONY : $(DIRECTORYS) link $(MODULESELECT) compile

$(MODULESELECT) : % : $(COMMONHEAD) $(OBJS) $(STARTUP) $(DIRECTORYS) link_%

nolink : $(COMMONHEAD) $(OBJS) $(STARTUP) $(DIRECTORYS) 

link : link_hd		# default

$(DIRECTORYS) :
	@$(MAKE) lib -C $@


ifneq ($(MGS2_XBOX),1)	# ------------------ PS2 VERSION

ifeq ($(MGS2_DESIGN),1)

# ¥½¡¼¥¹¡¢¥ª¥Ö¥¸¥§¥¯¥È¡¢¥é¥¤¥Ö¥é¥ê¤ò¶¦Í­¤·¤Æ¤¤¤EÎ¤Ç¡¢
# ¸Ä¡¹¤Î¥Ç¥£¥E¯¥È¥ê¤Ëlink·EÌ¤òÄ¾ÀÜÊÝÂ¸¤¹¤Eè¤¦¤ËÊÑ¹¹¤¹¤E£

$(addprefix link_, $(MODULESELECT) ):
	@echo Link with $@.o $(TARGET).elf
	@$(LD) -o $(CDROMIMG)/$(TARGET).elf -T $(LCFILE) $(OBJS) $(STARTUP) $(SELECTOBJ_PATH)/$@.o \
		$(LDOPTIONS) $(FSLIB) -Wl,-\( $(MODULELIB) -Wl,-\) $(LIBS) $(LDFLAGS)
	@$(MAKE) -C stage lib $(STAGEMAKEOPT)

else

$(addprefix link_, $(MODULESELECT) ):
	@echo Link with $@.o $(TARGET).elf
	@$(LD) -o $(TARGET).elf -T $(LCFILE) $(OBJS) $(STARTUP) $(SELECTOBJ_PATH)/$@.o \
		$(LDOPTIONS) $(FSLIB) -Wl,-\( $(MODULELIB) -Wl,-\) $(LIBS) $(LDFLAGS)
	cp $(TARGET).elf $(CDROMIMG)
	@$(MAKE) -C stage lib $(STAGEMAKEOPT)

endif

else					# ------------------ XBOX VERSION

STAGEOBJ = stage/_stage.obj

IMAGEBLDOPTIONS =	\
	/stack:0x10000 \
	/limitmem \
	/titleimage:"../cdrom.img/image/titleimage.row" \
	/insertfile:"../cdrom.img/image/Rev24b.bin",Rev24b,R \
	/testname:"METALGEAR SOLID 2 SUBSTANCE" \
	/testid:0x4B4E0007

stage/_stage.c : $(wildcard stage/[^_]*.c)
	@if [ -z "$^" ]; then echo "Error:NO STAGE FILE"; false; fi;
	@echo "make $@ in $(notdir $(basename $^))"
	@perl stage/mkxstage.pl $^ > $@

$(addprefix link_, $(MODULESELECT) ): $(STAGEOBJ)
	@echo Link with $@.o $(TARGET).elf
ifneq ($(MGS2_USESSE),1)
	@echo [ $(TARGET).exe ]
	$(LD) -out:$(TARGET).exe $(OBJS) $(STARTUP) $(SELECTOBJ_PATH)/$@.obj $(STAGEOBJ) \
		$(LDOPTIONS) $(FSLIB) $(MODULELIB) $(LIBS) $(LDFLAGS)
else
	@echo [ $(TARGET_SSE).exe ]
	$(LD) -out:$(TARGET_SSE).exe $(OBJS) $(STARTUP) $(SELECTOBJ_PATH)/$@.obj $(STAGEOBJ) \
		$(LDOPTIONS) $(FSLIB_SSE) $(MODULELIB_SSE) $(LIBS) $(LDFLAGS)
endif
ifneq ($(MGS2_WIN),1)
	imagebld /nologo /in:$(TARGET).exe /out:$(TARGET).xbe $(IMAGEBLDOPTIONS)
endif


#	imagebld /DUMP $(TARGET).xbe > mgs2.dump
#	stage/exports.pl mgs2.dump mgs2.map stage/resident.asm
#	ml /nologo /c /coff stage/resident.asm /Fostage/resident.obj

endif

# COMMON HEADER -------------------------

# infÌ¾¥Ø¥Ã¥Àscn¤«¤é¥³¥Ô¡¼
$(COMMONHEAD) : ../scn/commdef.h
	cp -f $< $@

# IOP MODULE PACKS -------------------------

$(IOPPAKPATH)/module.ipk : $(USRIRX_PATH)/*.irx $(IOPPAKLIST)
	irxar a $@ $(IOPPAKLIST) $(IOPPAKDEFS) -D$(MEDIA_TYPE) -DNOUSB -DNOHDSYS

$(IOPPAKPATH)/modhd.ipk : $(USRIRX_PATH)/*.irx $(IOPPAKLIST)
	irxar a $@ $(IOPPAKLIST) $(IOPPAKDEFS) -D$(MEDIA_TYPE) -DNOUSBFS

$(IOPPAKPATH)/modusb.ipk : $(USRIRX_PATH)/*.irx $(IOPPAKLIST)
	irxar a $@ $(IOPPAKLIST) $(IOPPAKDEFS) -DMEDIA_CD -DUSBFS

$(IOPPAKPATH)/modhdu.ipk : $(USRIRX_PATH)/*.irx $(IOPPAKLIST)		# CDµ¯Æ°
	irxar a $@ $(IOPPAKLIST) $(IOPPAKDEFS) -DMEDIA_CD -DUSBFS -DNOHDSYS

$(IOPPAKPATH)/syshd.ipk : $(USRIRX_PATH)/*.irx $(IOPPAKLIST)
	irxar a $@ $(IOPPAKLIST) $(IOPPAKDEFS) -DMEDIA_CD -DHDSYSONLY

# SUB COMMANDS -----------------------------

stagelink:
	$(MAKE) -C stage lib

scn : 
	$(MAKE) -C ../scn

stage :
	$(MAKE) -C ../scn stage

download :
	$(MAKE) -C ../list all

module :
	cd $(SYSMODULEDIR); cvs update; $(MAKE) all

list :
	cd ../list; cvs update;	$(MAKE) all

stagelist:
	@echo `ee-nm $(TARGET).elf | grep _mgs2_keep_end | cut -f 1 -d " "` $(TARGET).elf
	@(for i in stage/*.elf; do echo `ee-nm $$i | grep _stage_bin_end \
		 | cut -f 1 -d " "` $$i; done) | sort

log:
	-cvs2cl.pl --no-wrap -w -S -f _ChangeLog -U doc/userlist.txt -l "-d '>1 month ago'"

iop: $(IOPPAKS)

HDBUILD_TMPFILE = .\#_stagetmp.lst
DEFAULTSTAGE = init

hdbuild :
	$(MAKE) hdu
	ee-strip $(CDROMIMG)/$(TARGET).elf -o $(CDROMIMG)/hdboot.elf
ifeq ($(subst all,,$(strip $(MGS2STAGE))),)
	mkcdimg -o $(CDROMIMG)/stage.dat -s$(CDROMIMG)/stage -z
else
	echo $(DEFAULTSTAGE) $(MGS2STAGE) > $(HDBUILD_TMPFILE)
	mkcdimg -l $(HDBUILD_TMPFILE) -o $(CDROMIMG)/stage.dat -s$(CDROMIMG)/stage -z
	-rm -f $(HDBUILD_TMPFILE)
endif

.DEFAULT:
	@if [ -d .$@ ]; then \
		echo .$@; \
		make lib -C .$@ ; \
	else \
		echo "make: $@: no such target."; \
		false; \
	fi
