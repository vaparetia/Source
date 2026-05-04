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

PPU_SRCS	= \
	edgelzo1x_deflate_queue.cpp \
	edgelzo1x_inflate_queue.cpp \
	$(OBJS_DIR)/edgelzo1x_deflate_task.spu.elf \
	$(OBJS_DIR)/edgelzo1x_inflate_task.spu.elf

PPU_LIB_TARGET	= $(EDGE_TARGET_ROOT)/ppu/lib/libedgelzo$(EDGE_BUILD_SUFFIX).a

GCC_PPU_CXXFLAGS 	+= --param large-function-growth=800 
GCC_PPU_CFLAGS	 	+= --param large-function-growth=800 
SNC_PPU_CXXFLAGS 	+= 
SNC_PPU_CFLAGS		+= 

$(OBJS_DIR)/edgelzo1x_deflate_task.spu.elf : $(EDGELZO1XDEFLATETASKDIR)/$(OBJS_DIR)/edgelzo1x_deflate_task.spu.elf
	cp $< $@

$(OBJS_DIR)/edgelzo1x_inflate_task.spu.elf : $(EDGELZO1XINFLATETASKDIR)/$(OBJS_DIR)/edgelzo1x_inflate_task.spu.elf
	cp $< $@

CLEANFILES =	$(OBJS_DIR)/edgelzo1x_deflate_task.spu.elf \
				$(OBJS_DIR)/edgelzo1x_inflate_task.spu.elf

include $(CELL_MK_DIR)/sdk.target.mk

# Local Variables:
# mode: Makefile
# End:
