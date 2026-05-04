#########################################################
# SCE CONFIDENTIAL
# PlayStation(R)Edge 1.2.0
# Copyright (C) 2010 Sony Computer Entertainment Inc.
# All Rights Reserved.
#########################################################

EDGE_TARGET_ROOT = ../../../..

CELL_SDK ?= /usr/local/cell
CELL_MK_DIR ?= $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

include $(EDGE_TARGET_ROOT)/common/include/edge/edge_common.mk

SPU_SRCS	= edgepost_mlaa_task.cpp

SPU_CFLAGS		+= -ffunction-sections -fdata-sections -mspurs-task -fpic
SPU_CXXFLAGS		+= -ffunction-sections -fdata-sections -mspurs-task -fpic
SPU_LIBS		+= $(EDGE_TARGET_ROOT)/spu/lib/libedgepost$(EDGE_BUILD_SUFFIX).a
SPU_LDLIBS		+= -ldma -lspurs -lsync -latomic
SPU_LDFLAGS		+= -mspurs-task -Wl,--gc-sections
SPU_TARGET		= $(OBJS_DIR)/edgepost-mlaa-task.spu.elf

SUBDIRS		= $(EDGEPOSTSPUDIR)


include $(CELL_MK_DIR)/sdk.target.mk

# Local Variables:
# mode: Makefile
# End:
