#########################################################
# SCE CONFIDENTIAL
# PlayStation(R)Edge 1.2.0
# Copyright (C) 2009 Sony Computer Entertainment Inc.
# All Rights Reserved.
#########################################################

EDGE_TARGET_ROOT = ../../../..
CELL_MK_DIR ?= $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

include $(EDGE_TARGET_ROOT)/common/include/edge/edge_common.mk

SPA_SRCS = \
	edgepost_FP16.spa \
	edgepost_argb8ToFloats.spa \
	edgepost_horizontalgauss.spa \
	edgepost_verticalgauss.spa \
	edgepost_gauss7x1F.spa \
	edgepost_gauss1x7F.spa \
	edgepost_bloomcapture.spa\
	edgepost_ropmodulate.spa \
	edgepost_ropaddsat.spa \
	edgepost_ropblend.spa \
	edgepost_ropcombine.spa \
	edgepost_extractdepth.spa \
	edgepost_dof_fuzziness.spa \
	edgepost_dof_initialize.spa \
	edgepost_dof.spa \
	edgepost_dof_vsc.spa \
	edgepost_motionblur.spa \
	edgepost_downsample.spa \
	edgepost_upsample.spa \
	edgepost_downsample_minmax.spa \
	edgepost_copymemory.spa \
	edgepost_luv.spa \
	edgepost_LogLuvToFloats.spa \
	edgepost_FloatsToLogLuv.spa \
	edgepost_FP16LuvToFloats.spa \
	edgepost_FloatsToFP16Luv.spa \
	edgepost_mask.spa \
	edgepost_avgluminance.spa \
	edgepost_ilr.spa \
	edgepost_vignette.spa \
	edgepost_floatToGrayscale.spa \
	edgepost_square_transpose.spa \
	mlaa/edgepost_mlaa_blend.spa \
	mlaa/edgepost_mlaa_mark_discontinuities.spa \
	mlaa/edgepost_mlaa_write_threshold.spa \
	mlaa/edgepost_mlaa_blend_job_filter.spa \
	mlaa/edgepost_mlaa_find_separation_lines.spa \
	mlaa/edgepost_mlaa_compress_seplines.spa \
	mlaa/edgepost_mlaa_compute_bounds.spa \
	mlaa/edgepost_mlaa_clear_alpha.spa 
	

SPU_SRCS	= \
	edgepost_dmalist.cpp \
	edgepost_workload.cpp \
	edgepost_borders.cpp \
	edgepost_execute.cpp \
	edgepost_context.cpp \
	edgepost_dma.cpp \
	edgepost_dof_c.cpp \
	edgepost_motionblur_c.cpp \
	edgepost_inplace_transpose.cpp \
	edgepost_direction_lock.c \
	mlaa/edgepost_mlaa.cpp

SPU_LIB_TARGET	= $(EDGE_TARGET_ROOT)/spu/lib/libedgepost$(EDGE_BUILD_SUFFIX).a

# -fmodulo-sched is not yet working properly
SPU_CFLAGS		+= -fpic
SPU_CXXFLAGS	+= -fpic -Wstrict-aliasing=3

#Over-ride optimization level to be -O2 or else we won't fit in to LS in debug mode
SPU_OPTIMIZE_LV	= -O2

SPU_ASFLAGS		+= -Wa,--gdwarf2

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
