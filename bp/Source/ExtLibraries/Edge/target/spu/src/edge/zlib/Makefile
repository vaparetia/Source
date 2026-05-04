#########################################################
# SCE CONFIDENTIAL
# PlayStation(R)Edge 1.2.0
# Copyright (C) 2007 Sony Computer Entertainment Inc.
# All Rights Reserved.
#########################################################

EDGE_TARGET_ROOT = ../../../../../target

CELL_SDK ?= /usr/local/cell
CELL_MK_DIR ?= $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

include $(EDGE_TARGET_ROOT)/common/include/edge/edge_common.mk

SPU_SRCS	= \
	edgezlib_addqueue.cpp \
	edgezlib_adler32.c \
	edgezlib_crc32.c \
	edgezlib_deflate.c \
	edgezlib_deflate_raw_data.cpp \
	edgezlib_fetch_and_deflate_raw_data.cpp \
	edgezlib_fetch_and_inflate_raw_data.cpp \
	edgezlib_inffast.c \
	edgezlib_inffast.spu.s \
	edgezlib_inflate.c \
	edgezlib_inflate_raw_data.cpp \
	edgezlib_inftrees.c \
	edgezlib_longest_match.spu.s \
	edgezlib_trees.c \
	edgezlib_unaligned_large_dma_put.cpp \
	edgezlib_uncompr.c \
	edgezlib_zutil.c

SPU_LIB_TARGET	= $(EDGE_TARGET_ROOT)/spu/lib/libedgezlib$(EDGE_BUILD_SUFFIX).a

SPU_CFLAGS		+= -fpic -DMAX_WBITS=12 -DMAX_MEM_LEVEL=6 -DDEF_WBITS=15
SPU_CXXFLAGS	+= -fpic -DMAX_WBITS=12 -DMAX_MEM_LEVEL=6 -DDEF_WBITS=15

#Over-ride optimization level to be -O2 or else we won't fit in to LS in debug mode
SPU_OPTIMIZE_LV	= -O2

SPU_ASFLAGS		+= -Wa,--gdwarf2

FRONTEND = $(EDGE_TARGET_ROOT)/../../internal/host-win32/bin/frontend.exe

ifeq ($(shell test -f $(FRONTEND) && echo 1),1)
%.spu.s: $(EDGE_TARGET_ROOT)/../../internal/target/spu/src/edge/zlib/%.spu.fe $(FRONTEND)
	$(FRONTEND) $< -o $@ -useequ
endif

include $(CELL_MK_DIR)/sdk.target.mk

# Local Variables:
# mode: Makefile
# End:
