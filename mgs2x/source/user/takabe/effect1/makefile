#
# Makefile for MGS2 Sub Module
#	ｳﾆ･ｽ｡ｼ･ｹ､ｬ､ｪ､ｫ､・ｿ･ﾇ･｣･・ｯ･ﾈ･・ﾑ makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.5 2002/11/23 12:09:25 Yoshizawa1 Exp $
#

# ･ﾙ｡ｼ･ｹ･ﾇ･｣･・ｯ･ﾈ･・
BASEDIR = ../../..

# ･鬣､･ﾖ･鬣・ｾ

LIBNAME = takabe.a

ifeq ($(MGS2_XBOX),1)

# ｣ﾘ｣ﾂ｣ﾏ｣ﾘﾍﾑ･ﾕ･｡･､･・
.CFILES =  actblur.c anmtex.c crosfade.c e3_fanim.c eye_ctrl.c f_focus.c fanim.c fat_ikef.c frm_skip.c \
			grayclut.c ir_mode.c n_focus.c mini_scn.c \
			oozeblod.c optcmfbr.c optcmflg.c raster.c smk_blur.c spot.c xbox_dmy.c
.LOCAL_SHADER = ps_irmode.psh
# ･ｪ･ﾖ･ｸ･ｧ･ｯ･ﾈ･・ｹ･ﾈ
XOBJECTS = $(.CFILES:.c=.obj) $(.LOCAL_SHADER:.psh=_psh.obj)

# Windowsﾍﾑ･ﾕ･｡･､･・
.WCFILES =  actblur.c anmtex.c wcrosfade.c e3_fanim.c eye_ctrl.c f_focus.c fanim.c fat_ikef.c frm_skip.c \
			grayclut.c ir_mode.c n_focus.c mini_scn.c \
			oozeblod.c optcmfbr.c optcmflg.c raster.c smk_blur.c spot.c xbox_dmy.c
.LOCAL_SHADER = ps_irmode.psh
# ･ｪ･ﾖ･ｸ･ｧ･ｯ･ﾈ･・ｹ･ﾈ
WOBJECTS = $(.WCFILES:.c=.obj) $(.LOCAL_SHADER:.psh=_psh.obj)

%_vsh.c : %_vsh.v
	echo unsigned char VERTEX_SHADER_$*\[\] = { > $@
	od -An -tx1 -v $< | sed 's/\([a-z0-9][a-z0-9]\)/0x\1,/g'>> $@
	echo }\; >> $@

%_psh.c : %_psh.u
	echo unsigned char PIXEL_SHADER_$*\[\] = { > $@
	od -An -tx1 -v $< | sed 's/\([a-z0-9][a-z0-9]\)/0x\1,/g' >> $@
	echo }\; >> $@


%_vsh.v : %.vsh
ifeq ($(MGS2_ECHO_SHORT),1)
	@echo [ xsasm ] $<
	@xsasm -nologo -D KP_XBOX $<  $@
else
	xsasm -nologo -D KP_XBOX $<  $@
endif

%_psh.u : %.psh
ifeq ($(MGS2_ECHO_SHORT),1)
	@echo [ xsasm ] $<
	@xsasm -nologo -D KP_XBOX $<  $@
else
	xsasm -nologo -D KP_XBOX $<  $@
endif

else

# ｣ﾐ｣ﾓ｣ｲﾍﾑ･ﾕ･｡･､･・
.CFILES =  oozeblod.c envmap.c spot.c fit_doll.c scrolmap.c camofmap.c grayclut.c \
	f_focus.c n_focus.c optcmflg.c smk_blur.c eye_ctrl.c ir_mode.c crosfade.c raster.c \
	frm_skip.c anmtex.c mini_scn.c fat_ikef.c actblur.c \
	optcmfbr.c mono_scn.c \
	e3_fanim.c fanim.c
.VUFILES = b2rgb_vu.dsm
# ･ｪ･ﾖ･ｸ･ｧ･ｯ･ﾈ･�･ｹ･ﾈ
OBJECTS = $(.CFILES:.c=.o) $(.VUFILES:.dsm=.o)

endif


# ､ｳ､ｳ､ﾇ､ﾎ･悅ｼ･ｫ･・ﾕ･鬣ｰ･・ｹ･ﾈ

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# ﾀﾟﾄ熙ﾎｼﾂﾂﾎ

include $(BASEDIR)/makefile.sub

# ｰﾍﾂｸｴﾘｷｸ､ﾎ･・ｹ･ﾈ

b2rgb_vu.o: b2rgb_vu.dsm
	$(DVPASM) $(DVPASMFLAGS) -I$(INCDIR) -o $@ $< $(LISTOPTION)

