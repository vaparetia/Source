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
	edgelzo1x_addqueue.cpp \
	edgelzo1x_inflate_raw_data.cpp \
	edgelzo1x_deflate_raw_data.cpp \
	edgelzo_unaligned_large_dma_put.cpp \
	lzopro/src/edgelzo_lzo1x_d4s.c \
	lzopro/src/edgelzo_plzo1x_1n.c \
	lzopro/src/edgelzo_lzo1x_deflate.spu.s \
	minilzo/edgelzo_minilzo.c \
	minilzo/edgelzo_minilzo_lzo1x_decode.spu.s

SPU_LIB_TARGET	= $(EDGE_TARGET_ROOT)/spu/lib/libedgelzo$(EDGE_BUILD_SUFFIX).a

SPU_INCDIRS		+= -I./lzopro/include

SPU_CFLAGS		+= -fpic -Wno-undef
SPU_CXXFLAGS	+= -fpic -Wno-undef

SPU_ASFLAGS		+= -Wa,--gdwarf2

FRONTEND = $(EDGE_TARGET_ROOT)/../../internal/host-win32/bin/frontend.exe

ifeq ($(shell test -f $(FRONTEND) && echo 1),1)
lzopro/src/%.spu.s: $(EDGE_TARGET_ROOT)/../../internal/target/spu/src/edge/lzo/lzopro/src/%.spu.fe $(FRONTEND)
	$(FRONTEND) $< -o $@ -useequ
endif

ifeq ($(shell test -f $(FRONTEND) && echo 1),1)
minilzo/%.spu.s: $(EDGE_TARGET_ROOT)/../../internal/target/spu/src/edge/lzo/minilzo/%.spu.fe $(FRONTEND)
	$(FRONTEND) $< -o $@ -useequ
endif

include $(CELL_MK_DIR)/sdk.target.mk

# Local Variables:
# mode: Makefile
# End:
