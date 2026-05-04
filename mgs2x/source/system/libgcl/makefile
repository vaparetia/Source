#
# Makefile for MGS2 Sub Module
#	≥∆•Ω°º•π§¨§™§´§ÅEø•«•£•ÅEØ•»•ÅE— makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.4 2002/11/23 11:43:43 Yoshizawa1 Exp $
#

# •Ÿ°º•π•«•£•ÅEØ•»•ÅE
BASEDIR = ../..

# 16 Bit Strcode§Œª˛§œ§≥§ÅE¨$BASEDIR/makefile§«ƒÅE¡§µ§ÅE∆§§§ÅE
#STRCODE16 =

ifdef STRCODE16
 STRID_BIT = 16
else
 STRID_BIT = 24
endif

# •È•§•÷•È•ÅEæ

LIBNAME = libgcl.a

# •™•÷•∏•ß•Ø•»•ÅEπ•»
OBJECTS =	basic_g.o expr.o parse.o command.o gcl_init.o variable.o anti_par.o

XOBJECTS = $(OBJECTS:.o=.obj)

WOBJECTS = $(OBJECTS:.o=.obj)


# §≥§≥§«§Œ•˙Ωº•´•ÅE’•È•∞•ÅEπ•»

LOCAL_INCLUDES =
LOCAL_DEFINES =
LOCAL_OPTIONS = 

# ¿ﬂƒÍ§Œº¬¬Œ

include $(BASEDIR)/makefile.sub

# ∞Õ¬∏¥ÿ∑∏§Œ•ÅEπ•»

%_g.c : %.c
	stridconv.pl $< $(STRID_BIT) > $@






