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

ifeq ($(OS),Windows_NT)
 HOST       := win32
 HOSTDEFS	:= -DHOST_WIN32=1
else
 ifeq ($(OS),CYGWIN)
  HOST      := win32
  HOSTDEFS	:= -DHOST_WIN32=1
 else
  HOST      := linux
  HOSTDEFS 	:= -DHOST_LINUX=1
 endif
endif

MAKE_FSELF  = $(CELL_SDK)/host-$(HOST)/bin/make_fself
PPU_CC		= $(CELL_SDK)/host-$(HOST)/ppu/bin/ppu-lv2-gcc
PPU_AS		= $(CELL_SDK)/host-$(HOST)/ppu/bin/ppu-lv2-gcc
PPU_AR		= $(CELL_SDK)/host-$(HOST)/ppu/bin/ppu-lv2-ar
PPU_LD		= $(CELL_SDK)/host-$(HOST)/ppu/bin/ppu-lv2-gcc
PPU_OBJCOPY	= $(CELL_SDK)/host-$(HOST)/ppu/bin/ppu-lv2-objcopy
SPU_CC		= $(CELL_SDK)/host-$(HOST)/spu/bin/spu-lv2-gcc
SPU_AS		= $(CELL_SDK)/host-$(HOST)/spu/bin/spu-lv2-gcc
SPU_AR		= $(CELL_SDK)/host-$(HOST)/spu/bin/spu-lv2-ar
SPU_LD		= $(CELL_SDK)/host-$(HOST)/spu/bin/spu-lv2-gcc
SPU_OBJCOPY	= $(CELL_SDK)/host-$(HOST)/spu/bin/spu-lv2-objcopy

SPA_SRCS = # edgegeom_foo.spa

OBJS_DIR = .
SPA_SPU_OBJS	= $(patsubst %.spa, $(OBJS_DIR)/%.spu.o, $(SPA_SRCS))


SPU_C_OBJS = edgegeom.o edgegeom_c.o edgegeom_compress.o edgegeom_compress_c.o \
	edgegeom_cull.o edgegeom_cull_c.o edgegeom_decompress.o edgegeom_decompress_c.o \
	edgegeom_masks.o edgegeom_occlusion.o edgegeom_occlusion_main.o edgegeom_occlusion_c.o \
	edgegeom_skin.o edgegeom_skin_c.o edgegeom_transform.o edgegeom_transform_c.o

DEPS = $(SPU_C_OBJS:.o=.d)
DEPS_D = $(SPU_C_OBJS:.o=.dd)

ALL_OBJS = $(SPU_C_OBJS) $(SPA_SPU_OBJS)
ALL_OBJS_D = $(SPU_C_OBJS:.o=.od) $(SPA_SPU_OBJS)

CFLAGS = -fno-rtti -fno-exceptions -fpic \
	-ffunction-sections -fdata-sections \
	-I$(EDGE_TARGET_ROOT)/common/include -I$(EDGE_TARGET_ROOT)/spu/include \
	-Os -Wall -g
CFLAGS_D = $(CFLAGS) -fstack-check -DEDGE_GEOM_DEBUG -g

## This is a nasty GCC hack to force GCC to actually use all of the volatile 
## registers allowed by the ABI. 
## WARNING: Do not apply this to any file which contains functions which
## call other functions. IE only leaf functions allowed!!
LEAF_FLAGS = \
	-fcall-saved-4 -fcall-saved-5 -fcall-saved-6 \
	-fcall-saved-7 -fcall-saved-8 -fcall-saved-9 -fcall-saved-10 \
	-fcall-saved-11 -fcall-saved-12 -fcall-saved-13 -fcall-saved-14 \
	-fcall-saved-15 -fcall-saved-16 -fcall-saved-17 -fcall-saved-18 \
	-fcall-saved-19 -fcall-saved-20 -fcall-saved-21 -fcall-saved-22 \
	-fcall-saved-23 -fcall-saved-24 -fcall-saved-25 -fcall-saved-26 \
	-fcall-saved-27 -fcall-saved-28 -fcall-saved-29 -fcall-saved-30 \
	-fcall-saved-31 -fcall-saved-32 -fcall-saved-33 -fcall-saved-34 \
	-fcall-saved-35 -fcall-saved-36 -fcall-saved-37 -fcall-saved-38 \
	-fcall-saved-39 -fcall-saved-40 -fcall-saved-41 -fcall-saved-42 \
	-fcall-saved-43 -fcall-saved-44 -fcall-saved-45 -fcall-saved-46 \
	-fcall-saved-47 -fcall-saved-48 -fcall-saved-49 -fcall-saved-50 \
	-fcall-saved-51 -fcall-saved-52 -fcall-saved-53 -fcall-saved-54 \
	-fcall-saved-55 -fcall-saved-56 -fcall-saved-57 -fcall-saved-58 \
	-fcall-saved-59 -fcall-saved-60 -fcall-saved-61 -fcall-saved-62 \
	-fcall-saved-63 -fcall-saved-64 -fcall-saved-65 -fcall-saved-66 \
	-fcall-saved-67 -fcall-saved-68 -fcall-saved-69 -fcall-saved-70 \
	-fcall-saved-71 -fcall-saved-72 -fcall-saved-73 -fcall-saved-74 \
	-fcall-saved-75 -fcall-saved-76 -fcall-saved-77 -fcall-saved-78 \
	-fcall-saved-79

ifeq ($(CELL_BUILD_TOOLS), SNC)
SPU_LIB = $(EDGE_TARGET_ROOT)/spu/lib/libedgegeom_snc.a
SPU_LIB_D = $(EDGE_TARGET_ROOT)/spu/lib/libedgegeom_dbg_snc.a
else
SPU_LIB = $(EDGE_TARGET_ROOT)/spu/lib/libedgegeom.a
SPU_LIB_D = $(EDGE_TARGET_ROOT)/spu/lib/libedgegeom_dbg.a
endif


.SUFFIXES:

all: $(SPU_LIB) $(SPU_LIB_D)

release: $(SPU_LIB)

debug: $(SPU_LIB_D)

$(SPU_LIB): $(ALL_OBJS)
	@mkdir -p $(dir $@)
	@rm -f $@
	$(SPU_AR) rs $@ $(ALL_OBJS)

$(SPU_LIB_D) : $(ALL_OBJS_D)
	@mkdir -p $(dir $@)
	@rm -f $@
	$(SPU_AR) rs $@ $(ALL_OBJS_D)

edgegeom_occlusion.o : edgegeom_occlusion.cpp
	@echo " * Compiling $<"
	@mkdir -p $(dir $@)
	$(SPU_CC) $(CFLAGS) $(LEAF_FLAGS) -c $< -o $@

edgegeom_occlusion.od : edgegeom_occlusion.cpp
	@echo " * Compiling $<"
	@mkdir -p $(dir $@)
	$(SPU_CC) $(CFLAGS_D) $(LEAF_FLAGS) -c $< -o $@

$(SPA_SPU_OBJS): $(OBJS_DIR)/%.spu.o: %.spa $(SPA)
	@mkdir -p $(dir $(@))
	@$(EDGE_SPA) $< -o $@ --dwarf2 true

%.o : %.cpp
	@echo " * Compiling $<"
	@mkdir -p $(dir $@)
	$(SPU_CC) $(CFLAGS) -c $< -o $@
#	$(SPU_CC) $(CFLAGS) -S $< -o $@.S
#	$(SPU_CC) $(CFLAGS) -c $@.S -o $@

%.od : %.cpp
	@echo " * Compiling $<"
	@mkdir -p $(dir $@)
	$(SPU_CC) $(CFLAGS_D) -c $< -o $@
#	$(SPU_CC) $(CFLAGS) -S $< -o $@.S
#	$(SPU_CC) $(CFLAGS) -c $@.S -o $@

clean:
	rm -f $(ALL_OBJS) $(ALL_OBJS_D)
	-rm -f $(DEPS) $(DEPS_D) $(DEPS:.d=.d.tmp)
	rm -f $(SPU_LIB)
	rm -f $(SPU_LIB_D)

# Automatic dependency generation (.d files)
%.d : %.cpp
	@echo " * Updating dependencies for $<"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(SPU_CC) -Werror -MM -MQ "$*.o" -o $@.tmp $(CFLAGS) $<
	@cat $@.tmp | sed "s%\($*\)\.o[ :]*%\1.o $@ : %g" > $@
	@[ -s $@ ] || rm -f $@ $@.d.tmp
	@rm -f $@.tmp

# Include the per-source-file dependencies, unless we're building the
# "clean" target.
ifeq ($(filter %clean , $(MAKECMDGOALS)),)
-include $(DEPS)
endif 
