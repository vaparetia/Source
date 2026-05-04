#########################################################
# SCE CONFIDENTIAL
# PlayStation(R)Edge 1.2.0
# Copyright (C) 2007 Sony Computer Entertainment Inc.
# All Rights Reserved.
#########################################################


PSGLDIR					= $(CELL_SDK)/target/ppu/lib/PSGL/RSX/ultra-opt
GCMCOMMONDIR			= $(CELL_COMMON_DIR)/gcmutil

EDGEANIMPPUDIR			= $(EDGE_TARGET_ROOT)/ppu/src/edge/anim
EDGEANIMSPUDIR			= $(EDGE_TARGET_ROOT)/spu/src/edge/anim
EDGEZLIBPPUDIR			= $(EDGE_TARGET_ROOT)/ppu/src/edge/zlib
EDGEZLIBSPUDIR			= $(EDGE_TARGET_ROOT)/spu/src/edge/zlib
EDGEGEOMSPUDIR			= $(EDGE_TARGET_ROOT)/spu/src/edge/geom
EDGEGEOMTOOLPPUDIR		= $(EDGE_TARGET_ROOT)/../host-common/src/edge/libedgegeomtool

EDGELZMAPPUDIR			= $(EDGE_TARGET_ROOT)/ppu/src/edge/lzma
EDGELZMASPUDIR			= $(EDGE_TARGET_ROOT)/spu/src/edge/lzma

EDGEDXTSPUDIR			= $(EDGE_TARGET_ROOT)/spu/src/edge/dxt
EDGEPOSTPPUDIR			= $(EDGE_TARGET_ROOT)/ppu/src/edge/post
EDGEPOSTSPUDIR			= $(EDGE_TARGET_ROOT)/spu/src/edge/post
EDGEPOSTMLAATASKDIR		= $(EDGE_TARGET_ROOT)/spu/src/edge/post-mlaa-task

EDGEZLIBDEFLATETASKDIR	= $(EDGE_TARGET_ROOT)/spu/src/edge/zlib-deflate-task
EDGEZLIBINFLATETASKDIR	= $(EDGE_TARGET_ROOT)/spu/src/edge/zlib-inflate-task

EDGELZOPPUDIR			= $(EDGE_TARGET_ROOT)/ppu/src/edge/lzo
EDGELZOSPUDIR			= $(EDGE_TARGET_ROOT)/spu/src/edge/lzo

EDGELZO1XDEFLATETASKDIR	= $(EDGE_TARGET_ROOT)/spu/src/edge/lzo1x-deflate-task
EDGELZO1XINFLATETASKDIR	= $(EDGE_TARGET_ROOT)/spu/src/edge/lzo1x-inflate-task

EDGE_FILTERGEN			= $(EDGE_TARGET_ROOT)/../host-win32/bin/edgepostfiltergen.exe
EDGE_SPA				= $(EDGE_TARGET_ROOT)/../host-win32/bin/spa.exe


## Settings for controlling build configurations 

# Possible configurations
EDGE_BUILD_TYPES	=  debug release

# We default to release build
ifeq ($(EDGE_BUILD),)
$(warning Defaulting EDGE_BUILD to release)
export EDGE_BUILD	=  release
endif

# The user may have set EDGE_BUILD from the command line. 
# Check that the value is valid
ifeq ($(findstring $(EDGE_BUILD),$(EDGE_BUILD_TYPES)),)
$(error EDGE_BUILD=$(EDGE_BUILD) is unknown, must be one of: $(EDGE_BUILD_TYPES))
endif

# BUILD_TAG governs the subdir that intermediates are output to
export BUILD_TAG	=  $(EDGE_BUILD)


## PPU common settings

GCC_PPU_CFLAGS	 	+= -g -D__CELL_ASSERT__ -W -Wall -Wundef -Wshadow -Werror
GCC_PPU_CXXFLAGS 	+= -g -D__CELL_ASSERT__ -W -Wall -Wundef -Wshadow -Werror -fno-rtti -fno-exceptions
SNC_PPU_CFLAGS		+= -g -D__CELL_ASSERT__
SNC_PPU_CXXFLAGS 	+= -g -D__CELL_ASSERT__
SNC_DIAG_SUPPRESS	+= 

ifeq ($(EDGE_BUILD), debug)
ifeq ($(CELL_BUILD_TOOLS), SNC)
EDGE_BUILD_SUFFIX	= _dbg_snc
else
EDGE_BUILD_SUFFIX	= _dbg
endif
PPU_OPTIMIZE_LV		= -O0
SPU_OPTIMIZE_LV		= -O0
GCC_PPU_CFLAGS	 	+= -D_DEBUG
GCC_PPU_CXXFLAGS 	+= -D_DEBUG
SNC_PPU_CFLAGS		+= -D_DEBUG
SNC_PPU_CXXFLAGS 	+= -D_DEBUG
endif
ifeq ($(EDGE_BUILD), release)
ifeq ($(CELL_BUILD_TOOLS), SNC)
EDGE_BUILD_SUFFIX	= _snc
else
EDGE_BUILD_SUFFIX	= 
endif
PPU_OPTIMIZE_LV		= -O2
SPU_OPTIMIZE_LV		= -O2
endif


PPU_INCDIRS			+= -I$(EDGE_TARGET_ROOT)/common/include
PPU_INCDIRS			+= -I$(EDGE_TARGET_ROOT)/ppu/include
PPU_INCDIRS			+= -I$(CELL_TARGET_PATH)/ppu/include/vectormath/cpp


## We copy embedded ELFs to the root dir so the debugger
## finds them automatically

# The ELF files which are embedded in the PPU obj
PPU_EMBEDDED_ELFS	= $(filter spu/$(OBJS_DIR)/%.spu.elf, $(PPU_SRCS))

# The local files we wish to copy those ELFs to
# (we rename the local files to include the build config)
LOCAL_EMBEDDED_ELFS	= $(patsubst %.spu.elf,%.$(EDGE_BUILD).spu.elf,$(notdir $(PPU_EMBEDDED_ELFS)))

EXTRA_TARGET		+= $(strip $(LOCAL_EMBEDDED_ELFS))
CLEANFILES			+= $(strip $(LOCAL_EMBEDDED_ELFS))

# Rules to copy SPU embedded elfs locally (also renaming the
# copied file to include the build config)
%.$(EDGE_BUILD).spu.elf: spu/$(OBJS_DIR)/%.spu.elf
	@echo Copying $< to root dir for debugger
	cp $< $@

%.$(EDGE_BUILD).spu.elf: $(EDGE_TARGET_ROOT)/ppu/src/edge/lzma/$(OBJS_DIR)/%.spu.elf
	@echo Copying $< to root dir for debugger
	cp $< $@

%.$(EDGE_BUILD).spu.elf: $(EDGE_TARGET_ROOT)/ppu/src/edge/zlib/$(OBJS_DIR)/%.spu.elf
	@echo Copying $< to root dir for debugger
	cp $< $@

%.$(EDGE_BUILD).spu.elf: $(EDGE_TARGET_ROOT)/ppu/src/edge/lzo/$(OBJS_DIR)/%.spu.elf
	@echo Copying $< to root dir for debugger
	cp $< $@

## SPU common settings

SPU_CFLAGS			+= -g -D__CELL_ASSERT__ -W -Wall -Wundef -Wshadow -Werror -ffunction-sections -fdata-sections -fstack-check
SPU_CXXFLAGS		+= -g -D__CELL_ASSERT__ -W -Wall -Wundef -Wshadow -Werror -ffunction-sections -fdata-sections -fstack-check -fno-rtti -fno-exceptions

SPU_LDFLAGS			+= -Wl,--gc-sections

SPU_INCDIRS			+= -I$(EDGE_TARGET_ROOT)/common/include
SPU_INCDIRS			+= -I$(EDGE_TARGET_ROOT)/spu/include

SPU_ELF_TO_PPU_OBJ_FLAGS =

# Local Variables:
# mode: Makefile
# End:
