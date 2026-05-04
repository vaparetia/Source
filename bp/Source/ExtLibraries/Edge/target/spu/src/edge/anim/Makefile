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

SPA_SRCS	= \
    edgeanim_blend_joints_linear.spa \
    edgeanim_blend_joints_relative.spa \
    edgeanim_blend_user_linear.spa \
    edgeanim_blend_user_relative.spa \
    edgeanim_matrices4x4_to_joints.spa \
    edgeanim_joints_to_matrices4x4.spa \
    edgeanim_matrices3x4_to_joints.spa \
    edgeanim_joints_to_matrices3x4.spa \
    edgeanim_local_joints_to_world_joints.spa \
    edgeanim_local_joints_to_world_matrices3x4.spa \
    edgeanim_local_joints_to_world_matrices4x4.spa \
    edgeanim_world_joints_to_local_joints.spa \
    edgeanim_multiply_matrices3x4.spa \
    edgeanim_evaluate_r.spa \
    edgeanim_evaluate_r_const.spa \
    edgeanim_evaluate_st.spa \
    edgeanim_evaluate_user.spa \
    edgeanim_evaluate_bitpacked.spa \
    edgeanim_evaluate_bitpacked_const.spa \
    edgeanim_evaluate_bitpacked_user.spa \
    edgeanim_evaluate_bitpacked_user_const.spa \
    edgeanim_mirror_joints.spa

SPU_SRCS	= \
    edgeanim_evaluate.cpp \
    edgeanim_spu.cpp

SPU_LIB_TARGET	= $(EDGE_TARGET_ROOT)/spu/lib/libedgeanim$(EDGE_BUILD_SUFFIX).a

SPU_CFLAGS		+= -fpic
SPU_CXXFLAGS	+= -fpic

SPU_ASFLAGS		+= -Wa,--gdwarf2

ifeq ($(EDGE_BUILD), debug)
#Over-ride optimization level or else we won't fit in to LS in debug mode
SPU_OPTIMIZE_LV		= -O1
endif

include $(CELL_MK_DIR)/sdk.target.mk

SPA_SPU_OBJS	= $(patsubst %.spa, $(OBJS_DIR)/%.spu.o, $(SPA_SRCS))
SPU_OBJS		+= $(SPA_SPU_OBJS)
$(SPU_LIB_TARGET): $(SPA_SPU_OBJS)

$(SPA_SPU_OBJS): $(OBJS_DIR)/%.spu.o: %.spa $(SPA)
	@mkdir -p $(dir $(@))
	@$(EDGE_SPA) $< -o $@ --dwarf2 true

# Local Variables:
# mode: Makefile
# End:
