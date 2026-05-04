#########################################################
# SCE CONFIDENTIAL
# PlayStation(R)Edge 1.2.0
# Copyright (C) 2007 Sony Computer Entertainment Inc.
# All Rights Reserved.
#########################################################

EDGE_TARGET_ROOT = ../../../..
CELL_MK_DIR ?= $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

include $(EDGE_TARGET_ROOT)/common/include/edge/edge_common.mk

PPU_SRCS	= \
	edgeanim_ppu.cpp \
	edgeanim_evaluate_ppu.cpp  

PPU_LIB_TARGET	= $(EDGE_TARGET_ROOT)/ppu/lib/libedgeanim$(EDGE_BUILD_SUFFIX).a

GCC_PPU_CXXFLAGS 	+= --param large-function-growth=800 
GCC_PPU_CFLAGS	 	+= --param large-function-growth=800 
SNC_PPU_CXXFLAGS 	+= 
SNC_PPU_CFLAGS		+= 

include $(CELL_MK_DIR)/sdk.target.mk

# Local Variables:
# mode: Makefile
# End:
