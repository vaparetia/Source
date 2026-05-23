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
	edgezlib_inflate_task.cpp

SPU_LIBS		+= $(EDGE_TARGET_ROOT)/spu/lib/libedgezlib$(EDGE_BUILD_SUFFIX).a
SPU_LDLIBS		+= -ldma -lspurs -lsync -latomic
SPU_LDFLAGS		+= -Ttext 0x3000
SPU_TARGET		= $(OBJS_DIR)/edgezlib_inflate_task.spu.elf

include $(CELL_MK_DIR)/sdk.target.mk

# Local Variables:
# mode: Makefile
# End:
