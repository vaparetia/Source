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

EDGELZMATASKDIR		= $(EDGE_TARGET_ROOT)/spu/src/edge/lzma-inflate-task

PPU_SRCS	= \
	edgelzma_inflate_queue.cpp \
	$(OBJS_DIR)/edgelzma_inflate_task.spu.elf

PPU_LIB_TARGET		= $(EDGE_TARGET_ROOT)/ppu/lib/libedgelzma$(EDGE_BUILD_SUFFIX).a

GCC_PPU_CXXFLAGS 	+= --param large-function-growth=800
GCC_PPU_CFLAGS	 	+= --param large-function-growth=800
SNC_PPU_CXXFLAGS 	+=
SNC_PPU_CFLAGS		+=

$(OBJS_DIR)/edgelzma_inflate_task.spu.elf : $(EDGELZMATASKDIR)/$(OBJS_DIR)/edgelzma_inflate_task.spu.elf
	cp $< $@

CLEANFILES			= $(OBJS_DIR)/edgelzma_inflate_task.spu.elf

include $(CELL_MK_DIR)/sdk.target.mk

# Local Variables:
# mode: Makefile
# End:
