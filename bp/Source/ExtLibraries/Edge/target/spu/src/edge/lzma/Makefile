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
	edgelzma_addqueue.cpp \
	edgelzma_inflate_raw_data.cpp \
	edgelzma_decodeproperties.c \
	edgelzma_LzmaDecode.spu.s \
	edgelzma_LzmaDecode.c \
	edgelzma_unaligned_large_dma_put.cpp

SPU_LIB_TARGET	= $(EDGE_TARGET_ROOT)/spu/lib/libedgelzma$(EDGE_BUILD_SUFFIX).a


SPU_CFLAGS		+= -fpic
SPU_CXXFLAGS	+= -fpic

SPU_ASFLAGS		+= -Wa,--gdwarf2

FRONTEND = $(EDGE_TARGET_ROOT)/../../internal/host-win32/bin/frontend.exe

ifeq ($(shell test -f $(FRONTEND) && echo 1),1)
%.spu.s: $(EDGE_TARGET_ROOT)/../../internal/target/spu/src/edge/lzma/%.spu.fe $(FRONTEND)
	$(FRONTEND) $< -o $@ -useequ
endif

include $(CELL_MK_DIR)/sdk.target.mk

# Local Variables:
# mode: Makefile
# End:
