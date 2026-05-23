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

SPU_SRCS	=

SPA_SRCS	= \
    edgedxt_compress1.spa \
    edgedxt_compress1a.spa \
    edgedxt_compress3.spa \
    edgedxt_compress5.spa \
	edgedxt_decompress1.spa \
	edgedxt_decompress3.spa \
	edgedxt_decompress5.spa

#Recompile the .spa files any time any of these includes change.
SPA_INCLUDE_DEPENDENCIES	+= edgedxt_compress_generic.inc
SPA_INCLUDE_DEPENDENCIES	+= edgedxt_decompress_generic.inc

SPU_LIB_TARGET	= $(EDGE_TARGET_ROOT)/spu/lib/libedgedxt$(EDGE_BUILD_SUFFIX).a

SPU_CFLAGS		+= -fpic
SPU_CXXFLAGS	+= -fpic

SPU_ASFLAGS		+= -Wa,--gdwarf2

include $(CELL_MK_DIR)/sdk.target.mk

SPA_SPU_OBJS	= $(patsubst %.spa, $(OBJS_DIR)/%.spu.o, $(SPA_SRCS))
SPU_OBJS		+= $(SPA_SPU_OBJS)
$(SPU_LIB_TARGET): $(SPA_SPU_OBJS)

$(SPA_SPU_OBJS): $(OBJS_DIR)/%.spu.o: %.spa $(SPA) $(SPA_INCLUDE_DEPENDENCIES)
	@mkdir -p $(dir $(@))
	@$(EDGE_SPA) $< -o $@ --dwarf2 true

# Local Variables:
# mode: Makefile
# End:
