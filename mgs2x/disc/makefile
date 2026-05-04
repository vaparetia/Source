#
# CD/DVD イメージ作成
#

# ----------------- パラメータ設定

ifeq ($(MGS2_AREA),JAPAN)
BOOTFILE = SLPM_670.02
endif

ifndef MGS2_VERSION
VERSION = 1.00
else
VERSION = $(MGS2_VERSION)
endif
SYSTEMCNF = $(DISC_ROOT)/system.cnf

ifeq ($(MGS2_VMODE),PAL)
VMODE = PAL
else
VMODE = NTSC
endif

NEEDMODULES = *.img
USERMODULES = pld.irx

DATAFILES = codec.dat face.dat demo.dat movie.dat vox.dat

# ----------------- パス設定

export DISC_ROOT = image
export MGS2DIR   = $(DISC_ROOT)/mgs2

MODULES = $(DISC_ROOT)/modules

SCNDIR     = ../scn
SOURCEDIR  = ../source
MODULEDIR  = ../module
MODULESDIR = ../module/irx

SYSTEM_MODULES_PATH = /usr/local/sce/iop/modules

SYSTEM_MODULES = $(addprefix $(SYSTEM_MODULES_PATH)/, $(NEEDMODULES) )

# ----------------- ターゲット設定

.PHONY : all image vox demo bin master movie

ifeq ($(MGS2_MASTER),1)
MASTER = master
else
MASTER = 
endif

all : clean vox demo bin $(MASTER)
download : clean binhdu $(MASTER)

image:
	if [ ! -d $(DISC_ROOT) ]; then \
	 mkdir $(DISC_ROOT); mkdir $(MGS2DIR); mkdir $(MODULES); \
	fi

vox : image
	cp $(SCNDIR)/vox.lst $(MGS2DIR)

demo : image
	cp $(SCNDIR)/demo.lst $(MGS2DIR)

bin : image
	-cp $(MGS2DIR)/*.lst $(SCNDIR)
#	cd $(SCNDIR); make clean all
#	cd $(MODULEDIR); make clean all
#	cd $(SOURCEDIR); make stage cd iop;
	echo "BOOT2 = cdrom0:\\$(strip $(BOOTFILE));1" > $(SYSTEMCNF)
	echo "VER = $(VERSION)" >> $(SYSTEMCNF)
	echo "VMODE = $(VMODE)" >> $(SYSTEMCNF)
#	echo "HDDUNITPOWER = NICHDD" >> $(SYSTEMCNF)
	cp $(SYSTEM_MODULES) $(MODULES)
	cp -vpu ../module/irx/pld.irx $(MODULES)
	cp -vpu ../cdrom.img/modules/module.ipk $(MODULES)
#	cp -vpu $(addprefix ../cdrom.img/, $(DATAFILES)) $(MGS2DIR)
	(cd $(MGS2DIR); for i in $(DATAFILES); do ln -s ../../../cdrom.img/$$i $$i; done)
#	mkcdimg -s ../cdrom.img/stage -o $(MGS2DIR)/stage.dat -z
	ee-strip ../cdrom.img/mgs2.elf -o $(DISC_ROOT)/$(BOOTFILE)
	cd $(DISC_ROOT); mkdummy 1048576

binhdu :
	if [ ! -d $(DISC_ROOT) ]; then \
	 mkdir $(DISC_ROOT); mkdir $(MODULES); \
	fi
	cd $(SCNDIR); make clean all
	cd $(MODULEDIR); make clean all
	cd $(SOURCEDIR); make stage hdu iop;
	cp -v ../cdrom.img/hdboot.cnf $(DISC_ROOT)
	cp -v ../cdrom.img/hdinst.cnf $(DISC_ROOT)
	cp $(SYSTEM_MODULES) $(MODULES)
	cp -v ../module/irx/pld.irx $(MODULES)
	cp -v ../cdrom.img/modules/*.ipk $(MODULES)
	cp -v $(addprefix ../cdrom.img/, $(DATAFILES)) $(DISC_ROOT)
	mkcdimg -s ../cdrom.img/stage -o $(DISC_ROOT)/stage.dat -z
	ee-strip ../cdrom.img/mgs2.elf -o $(DISC_ROOT)/hdboot.elf

clean : 
	rm -rf $(DISC_ROOT)

master :
	rm $(MGS2DIR)/*.lst

movie :	# 体験版用ムービー
	cd movie; make data
