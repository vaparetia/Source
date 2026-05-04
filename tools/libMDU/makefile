# 
# makfile for libMDU
# 
# libMDU : library for MGS2 Data Utility
#	LIBDIR2 = /usr/local/develop/tools/libMDU

# by M.Sonoyama 1999 Aug. ¡Á
# Modified by K.Kano , 11/13/1999
# 
# Konami Computer Entertainment Japan West CS1
#

SHELL = /bin/sh

TARGET = libMDU.a
CSRC   = MDU_texInit.c MDU_texLoad.c MDU_texSave.c MDU_texCm2.c \
	 MDU_util.c MDU_darFile.c \
	 MDU_mdlMdl2Km3.c MDU_mdlLoad.c \
	 MDU_mdlSave.c MDU_mdlUtil.c MDU_mdlKm32Kms.c \
	 MDU_texExtMdl.c MDU_mdlDivide.c \
	 MDU_mdlMdl2Km4.c MDU_mdlKm42Kms.c MDU_mdlDivideKm4.c \
	 Mdl2Kmx.c Kmx2Kms2.c ToStrip.c DivideKmx.c block.c DivPrims.c \
	 Mdl2Evf.c Evf2Evm.c CheckPrims.c Overlap.c Kmx2Kmsx.c Picture.c

OBJS   = $(CSRC:.c=.o)

CPU    = -m486

ifeq ($(MGS2_WIN32),1)
MDLLIB = /u/develop/mj001b/exports/tools/Developer/lib
else
MDLLIB = /u/develop/mj001b/tools/vrs/lib
endif

#MDLLIB = /usr/local/develop/tools/vrs
#LIBDIR = /usr/local/develop/tools/libMDU
INCPATH = $(MDLLIB)/include
SUPPORTPATH = $(MDLLIB)/support/include
LIBPATH = $(MDLLIB)/lib
LIBS = -lP3DCL$(CPU) -lSFDCL$(CPU) -lm -lc

LIBDIR = /u/develop/mj001b/tools
DEFINES = -D_LINUX -DIS_LITTLE_ENDIAN

CC     = gcc
AR     = ar
CFLAGS = -c -g -O2 $(CPU) -Wall $(DEFINES) -I. -I$(INCPATH) \
	 -I$(SUPPORTPATH) -malign-double

all: $(TARGET)

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	-rm -f $@
	$(AR) q $@ $(OBJS)
	ranlib $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

install:
	cp $(TARGET) $(LIBDIR)/lib
	cp *.h $(LIBDIR)/include
#	cp makefile *.c *.h $(LIBDIR)/src

#---------------------------------------------------------------

MDU_texInit.o : MDU_texInit.c MDU_tex.h
MDU_texLoad.o : MDU_texLoad.c MDU_tex.h
MDU_texSave.o : MDU_texSave.c MDU_tex.h
MDU_darFile.o : MDU_darFile.c MDU_darFile.h
MDU_mdlLoad.o : fmt_kms.h MDU_mdl.h MDU_mdlLoad.c
MDU_mdlSave.o : fmt_kms.h MDU_mdl.h MDU_mdlSave.c
MDU_mdlUtil.o : fmt_kms.h MDU_mdl.h MDU_mdlUtil.c
MDU_mdlMdl2km3.o : fmt_kms.h MDU_mdl.h MDU_mdlMdl2km3.c
MDU_mdlKm32Kms.o : MDU_mdlKm32Kms.c MDU_mdlKm32Kms.h MDU_mdlKm32KmsUtil.c

MDU_mdlMdl2km4.o : fmt_kms.h fmt_km4.h MDU_mdl.h MDU_mdlMdl2km4.c
MDU_mdlKm42Kms.o : MDU_mdlKm42Kms.c MDU_mdlKm42Kms.h MDU_mdlKm32KmsUtil.c
MDU_mdlDivideKm4.o : MDU_mdlDivideKm4.c MDU_util.h MDU_mdl.h MDU_tex.h
