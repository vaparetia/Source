#
#	makefile for stage data download
#		1999/10/28 K.Uehara
#
#	$Id: makefile,v 1.433 2002/10/08 01:06:40 usr01475 Exp $
#

.SUFFIXES: .src .stm

DEPENDDIR = depend

# ---- stage definitions

include stage.inc	# この中で各環境用にSTAGESを定義

# ---- motion list

COMMON_TANKER_MOTION_LIST = \
		sna_w00a sna_w00b sna_w00c sna_w01a sna_w01b sna_w01c sna_w01d sna_w01e sna_w01f \
		sna_w02a sna_w03a sna_w04 sna_w04c \
		sna_a00a sna_a00c sna_a01a sna_a01b sna_a01f sna_a02a sna_a03a

COMMON_PLANT_MOTION_LIST = \
		rai_w11a rai_w11c rai_w12a rai_w12b rai_w13a rai_w14a rai_w15a \
		rai_w16a rah_w16a iro_w16a ptr_w16a c4_w16a sprayer_w16a sensorA_w16a rai_w17a rai_w18a \
		rai_w19a rai_w20a rai_w20b rai_w21a rai_w22a rai_w23a rai_w24a rai_w24b rai_w24c rai_w24d \
		rai_w25a rai_w25b rai_w25c rai_w25d rai_w28a rai_w31a rai_w31b rai_w31c rai_w32a \
		rai_w41a rai_w42a rai_w43a rai_w44a sna_w44a rai_w45a rai_w46a npc_snake rai_w61a \
		rai_a12a rai_a12b rai_a13a rai_a14a rai_a15a rai_a16a rai_a17a rai_a19a rai_a20a rai_a20b rai_a21a rai_a22a rai_a23a \
		rai_a24a rai_a24b rai_a24c rai_a24d sna_a25a rai_a25d rai_a28a rai_a31a rai_a41a rai_a42a rai_a43a rai_a44a rai_a45a \
		rai_a46a

COMMON_VR_MOTION_LIST = \
		rai_v01a \
		rai_vs01a rai_vs05a rai_vs06a rai_vs07a rai_vs08a rai_vs09a rai_vs10a \
		rai_wp01a rai_wp02a rai_wp03a rai_wp04a rai_wp05a \
		rai_wp11a rai_wp12a rai_wp13a rai_wp14a rai_wp15a \
		rai_wp21a rai_wp22a rai_wp23a rai_wp24a rai_wp25a \
		rai_wp31a rai_wp32a rai_wp33a rai_wp34a rai_wp35a \
		rai_wp41a rai_wp42a rai_wp43a rai_wp44a rai_wp45a \
		rai_wp51a rai_wp52a rai_wp53a rai_wp54a rai_wp55a \
		rai_wp61a rai_wp62a rai_wp63a rai_wp64a rai_wp65a \
		rai_wp71a rai_wp72a rai_wp73a rai_wp74a rai_wp75a \
		rai_sp01a rai_sp02a rai_sp03a rai_sp06a rai_sp07a rai_sp08a \
		rai_sp21a rai_sp22a rai_sp25a \
		rai_st01a \
		rai_vr_test

COMMON_MOTION_LIST = $(COMMON_TANKER_MOTION_LIST) $(COMMON_PLANT_MOTION_LIST) $(COMMON_VR_MOTION_LIST)
MOTION_LIST = $(addsuffix .mls, $(COMMON_MOTION_LIST))

# ------ face 

FACES = node photo capture mobile \
	f00a f01a f01b f01c f01d f01e f01f f02a f03a f03b f04a f04b f04c f04d f04e f05a f06a

#音声リスト
ifneq ($(MGS2_DISC),"")
VOXLSS = vox1.lss
CODECLSS = codec.lss
else
VOXLSS = vox.lss
CODECLSS = codec.lss
endif

ifeq ($(MGS2_TRIAL),1)

# 体験版用
SOUNDLIST = soundex.lst
STAGEPATH = ../cdrom.img/stage
FACEPATH = ../cdrom.img/face
FACEDAT = ../cdrom.img/face.dat

else

ifeq ($(MGS2_DESIGN),1)

# プレビュー環境は、CDイメージ自体が違うので、別口の定義ファイルを用いる。
SOUNDLIST = soundpre.lst
STAGEPATH = $(PREVIEW_DATA)/stage
FACEPATH = ../cdrom.img/face
FACEDAT = ../cdrom.img/face.dat

else

ifeq ($(MGS2_E3),1)

# E3環境
SOUNDLIST = soundet.lst
STAGEPATH = ../cdrom.img/stage
FACEPATH = ../cdrom.img/face
FACEDAT = ../cdrom.img/face.dat

else

ifeq ($(MGS2_LANG),ENGLISH)
# 英語版環境用サウンドリスト
SOUNDLIST = soundsde.lst
else
# 日本語版環境用サウンドリスト
SOUNDLIST = soundsd.lst
endif

STAGEPATH = ../cdrom.img/stage
FACEPATH = ../cdrom.img/face
FACEDAT = ../cdrom.img/face.dat

endif

endif

endif

ifeq ($(MGS2_VRTRIAL),1)
SOUNDLIST = soundtr.lst
VOXLSS = vrtrial.lss
CODECLSS =
endif

# ---------- その他のファイルパス

ifeq ($(MGS2_XBOX),1)
FIXPATH=/u/develop/mj001data/fix.xbox
else
ifeq ($(MGS2_SUBSTANCE),1)
FIXPATH=/u/develop/mj001data/fix.xbox
else
FIXPATH=/u/develop/mj001data/fix
endif
endif


ifeq ($(MGS2_VMODE),PAL)
# PAL版
SOUNDPATH = $(FIXPATH)/sound.eng
DEMOKEY = demo.pal
else
ifeq ($(MGS2_LANG),ENGLISH)
# 英語版
SOUNDPATH = $(FIXPATH)/sound.eng
DEMOKEY = demo.eng
else
# 日本語版
SOUNDPATH = $(FIXPATH)/sound
DEMOKEY = demo
endif
endif

############### VR 体験版の時は音声だけ日本語にするため上書き！！！
ifeq ($(MGS2_VRTRIAL),1)
SOUNDPATH = $(FIXPATH)/sound
endif


VOXPATH = $(SOUNDPATH)/vox1/pcm1
VOXCPATH = $(SOUNDPATH)/vox1/pcm1c
VOXSRC = vox.scr

DEMOPATH = $(FIXPATH)/demo
DEMOSRC = demo.scr



ifeq ($(MGS2_EU_TRIAL),ENGLISH)
MOVIESRC = movie_et.scr
else

ifeq ($(MGS2_EU_TRIAL),GERMAN)
MOVIESRC = movie_et.scr
else

ifeq ($(MGS2_SUBSTANCE),1)
# PS2 disc 2層バージョン用
MOVIESRC = movie1.scr
MOVIEVRSRC = movie2.scr
DISC1OPT = -c 0
DISC2OPT = -c 1
DEMO1LSS = demo1.lss
DEMO2LSS = demo2.lss
else

MOVIESRC = movie.scr
MOVIEVRSRC = movievr.scr
DISC1OPT =
DISC2OPT =
DEMO1LSS = demo.lss
DEMO2LSS = 
endif

endif
endif

SCNPATH = ../scn
IMAGEPATH = $(STAGEPATH)/..

STREAMDEF = ./stream.def
VOXFILE = $(IMAGEPATH)/vox.dat
VOXLIST = $(SCNPATH)/vox.lst
VOX2FILE = $(IMAGEPATH)/vox2.dat
VOX2LIST = $(SCNPATH)/vox2.lst

DEMOFILE = $(IMAGEPATH)/demo.dat
DEMOLIST = $(SCNPATH)/demo.lst
DEMO2FILE = $(IMAGEPATH)/demo2.dat
DEMO2LIST = $(SCNPATH)/demo2.lst

DEMOLISTDIR = ./demolist

MOVIEFILE = $(IMAGEPATH)/movie.dat
MOVIELIST = $(SCNPATH)/movie.lst
MOVIEVRFILE = $(IMAGEPATH)/movievr.dat
MOVIEVRLIST = $(SCNPATH)/movievr.lst

CACHEFILE = ./.cache

USRLIST = /u/home/user/usrlist
MAILMSG = mailmsg -d -u$(USRLIST)/mgs2user.lst -a$(USRLIST)/mgs2user.ali

ifeq ("$(DOWNLOADPATH)","")
DOWNLOADPATH = /u/develop/mj001data/image
endif

# ----------

TMPSTAGES =$(addsuffix .t, $(STAGES))
SRCFILES = $(addsuffix .src, $(STAGES))

STAMPS = $(addprefix $(DEPENDDIR)/, $(SRCFILES:src=stm))

# ---- command definitions

DOWNLOAD = dpack

# ---- xbox dashboard image
ifeq ($(MGS2_XBOX),1)
TITLEIMAGE = titleimage.row 
#SAVEIMAGES = 
endif

# ---- targets

.PHONY : all data depend $(STAGES) $(FACES) vox demo sound errlog mh stageonly \
		downloadstage downloadstage_c getdashboardimage

depend : $(STAMPS) mh

$(DEPENDDIR)/%.stm : %.src
	$(DOWNLOAD) ../cdrom.img/stage/$(<:.src=) $<
ifeq ($(MGS2_XBOX),1)
	-sed -e 's/wvx1/xwvx1/g' -e 's/efx1/xefx1/g' -e 's/mdx1/xmdx1/g' $(SOUNDLIST) > _sound.lst
	-soundpak _sound.lst -o $(STAGEPATH) -s $(SOUNDPATH) -d $(<:.src=)
else
	-soundpak $(SOUNDLIST) -o $(STAGEPATH) -s $(SOUNDPATH) -d $(<:.src=)
endif
	touch $@

stageonly : clean clearstage $(STAGES) errlog

stagelist :
	@echo $(DUALLAYER_STAGE) $(STAGES)

ifeq ($(MGS2_SCN),1)
all : clean clearstage clearface mh $(STAGES) $(FACEDAT) errlog getdashboardimage
download : clearface $(FACEDAT) mh downloadstage getdashboardimage
else
all : clean clearstage clearface mh $(STAGES) $(FACEDAT) vox demo movie errlog getdashboardimage
download : clearface downloadface mh downloadstage_c getdashboardimage
endif

new : $(STAGES)

resident : $(INIT_STAGES) $(RESIDENT_STAGES)
resident_vr : $(INIT_STAGES) $(RESIDENT_STAGES_VR)

face : $(FACEDAT)
$(FACEDAT) : $(FACES)

$(STAGES) :
	$(DOWNLOAD) ../cdrom.img/stage/$@ $@.src
ifeq ($(MGS2_XBOX),1)
	-sed -e 's/wvx1/xwvx1/g' -e 's/efx1/xefx1/g' -e 's/mdx1/xmdx1/g' $(SOUNDLIST) > _sound.lst
	-soundpak _sound.lst   -o $(STAGEPATH) -s $(SOUNDPATH) -d $@
else
	-soundpak $(SOUNDLIST) -o $(STAGEPATH) -s $(SOUNDPATH) -d $@
endif
	touch $(DEPENDDIR)/$@.stm

$(FACEDAT):
	mkcdimg -o $(FACEDAT) -s $(FACEPATH) -t ../scn/face.lst

$(FACES) :
	$(DOWNLOAD) $(FACEPATH)/$@ $@.src

downloadface:
	cp -r $(DOWNLOADPATH)/mgs2x/cdrom.img/face/ ../cdrom.img/face/
	cp $(DOWNLOADPATH)/mgs2x/cdrom.img/face.dat ../cdrom.img
	cp $(DOWNLOADPATH)/mgs2x/scn/face.lst ../scn

$(TMPSTAGES) :
	$(DOWNLOAD) -t ../cdrom.img/stage/$(@:.t=) $(@:.t=).src

mh :
	(for i in $(MOTION_LIST) ; do makemar -m -n -e $$i ; done)
	mv *.mh ../scn

clean :
	-rm err.log
	-rm -vf $(DEPENDDIR)/*.stm .#* .cache

clearstage :
	-rm -rf ../cdrom.img/stage/*

clearface :
	-rm -rf ../cdrom.img/face/*

downloadstage downloadstage_c:
	@if [ ! -e $(DOWNLOADPATH)/done ]; then \
		echo "DOWNLOAD ERROR !! / アップデート中か、自動アップデートでエラーが発生しています。"; \
		echo "$(DOWNLOADPATH)/lasterrを確認して下さい。"; \
		false; \
	fi
	@if [ "$@" = "downloadstage_c" ]; then rm -rf ../cdrom.img/stage/*; fi
	@cd ../cdrom.img; tar zxvf $(DOWNLOADPATH)/stage.tar.gz
	@grep Err $(DOWNLOADPATH)/err.log; \
	if [ $$? -eq 0 ]; then \
		echo "DOWNLOAD FINISH. BUT .. ダウンロードは完了しましたが、エラーがあるようです。"; \
		echo "$(DOWNLOADPATH)/err.logを参照してsrcを修正して下さい。"; \
	else \
		echo "download complete"; \
	fi

downloaddat :
	cp -v $(DOWNLOADPATH)/mgs2x/cdrom.img/vox.dat ../cdrom.img
	cp -v $(DOWNLOADPATH)/mgs2x/cdrom.img/demo.dat ../cdrom.img
	cp -v $(DOWNLOADPATH)/mgs2x/cdrom.img/movie.dat ../cdrom.img
	cp -v $(DOWNLOADPATH)/mgs2x/cdrom.img/movievr.dat ../cdrom.img
	cp -v $(DOWNLOADPATH)/mgs2x/scn/vox.lst ../scn
	cp -v $(DOWNLOADPATH)/mgs2x/scn/demo.lst ../scn
	cp -v $(DOWNLOADPATH)/mgs2x/scn/movie.lst ../scn
	cp -v $(DOWNLOADPATH)/mgs2x/scn/movievr.lst ../scn
	cp $(DEMOPATH)/data_tanker.int/*.lst $(DEMOLISTDIR)
	cp $(DEMOPATH)/data_plant.int/*.lst $(DEMOLISTDIR)
	cp $(DEMOPATH)/data_scn/*.lst $(DEMOLISTDIR)

errlog :
	grep Err err.log | sort | uniq

sound :
ifeq ($(MGS2_XBOX),1)
	sed -e 's/wvx1/xwvx1/g' -e 's/efx1/xefx1/g' -e 's/mdx1/xmdx1/g' $(SOUNDLIST) > _sound.lst
	soundpak _sound.lst   -o $(STAGEPATH) -s $(SOUNDPATH)
else
	soundpak $(SOUNDLIST) -o $(STAGEPATH) -s $(SOUNDPATH)
endif

# vox  --------------------------------------------------------------
vox : 
	makevox $(VOXLSS) vox $(VOXPATH) > $(VOXSRC)
ifneq ($(CODECLSS),)
	makevox $(CODECLSS) codec $(VOXCPATH) >> $(VOXSRC)
endif
	scr2xbox $(VOXSRC) > _xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
	sed -e 's#/sound.pal/#/sound.eng/#g' -e 's#/sound.eng/vox1/mst#/sound.pal/vox1/mst#g' _xbox.scr > $(VOXSRC)
	stream $(VOXSRC) $(DISC1OPT) -d $(STREAMDEF) -o $(VOXFILE) -t $(VOXLIST)
	rm -f _xbox.scr
ifeq ($(MGS2_SUBSTANCE),1)	# ------------ PS2 2層ディスク バージョン
	makevox vox2.lss vox $(VOXPATH) > $(VOXSRC)
	scr2xbox $(VOXSRC) > _xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
	sed -e 's#/sound.pal/#/sound.eng/#g' -e 's#/sound.eng/vox1/mst#/sound.pal/vox1/mst#g' _xbox.scr > $(VOXSRC)
	stream $(VOXSRC) $(DISC2OPT) -d $(STREAMDEF) -o $(VOX2FILE) -t $(VOX2LIST)
	rm -f _xbox.scr
endif

# demo --------------------------------------------------------------

demo :
ifeq ($(MGS2_VMODE), PAL)
# PAL版
	@if [ -e ./demo_pal.lss ]; then \
		rm ./demo_pal.lss; \
	fi
	#DEMO_KEYは、demo.engに固定です。
	makevox $(DEMO1LSS) demo.eng $(DEMOPATH) > $(DEMOSRC)
	scr2pal _demo.scr >> $(DEMOSRC)
	scr2xbox $(DEMOSRC) > _xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
	sed -e 's#/demo/data_tanker.*/#/demo/data_tanker.pal/#g' -e 's#/demo/data_plant.*/#/demo/data_plant.pal/#g' -e 's#/sound.pal/#/sound.eng/#g' _xbox.scr > $(DEMOSRC)
	cp $(DEMOSRC) _xbox.scr

	#こう書かないとWIN32で通らない
	cp $(DEMOPATH)/data_tanker.pal/*.lst $(DEMOLISTDIR)
	cp $(DEMOPATH)/data_plant.pal/*.lst $(DEMOLISTDIR)
	cp $(DEMOPATH)/data_scn/*.lst $(DEMOLISTDIR)
	#cp `ls -rt $(DEMOPATH)/{data_tanker.pal,data_plant.pal,data_scn}/*.lst` $(DEMOLISTDIR)
else
# 英語版
	makevox $(DEMO1LSS) $(DEMOKEY) $(DEMOPATH) > $(DEMOSRC)
	scr2sub _demo.scr >> $(DEMOSRC)
	scr2xbox $(DEMOSRC) > _xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する

	cp $(DEMOPATH)/data_tanker.int/*.lst $(DEMOLISTDIR)
	cp $(DEMOPATH)/data_plant.int/*.lst $(DEMOLISTDIR)
	cp $(DEMOPATH)/data_scn/*.lst $(DEMOLISTDIR)
endif
	stream _xbox.scr $(DISC1OPT) -d $(STREAMDEF) -o $(DEMOFILE) -t $(DEMOLIST)
	rm -f _xbox.scr
ifeq ($(MGS2_SUBSTANCE),1)	# ------------ PS2 2層ディスク バージョン

ifeq ($(MGS2_VMODE), PAL)
# PAL版
	makevox $(DEMO2LSS) demo.eng $(DEMOPATH) > $(DEMOSRC)
	scr2xbox $(DEMOSRC) > _xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
	sed -e 's#/demo/data_tanker.*/#/demo/data_tanker.pal/#g' -e 's#/demo/data_plant.*/#/demo/data_plant.pal/#g' -e 's#/sound.pal/#/sound.eng/#g' _xbox.scr > $(DEMOSRC)
	cp $(DEMOSRC) _xbox.scr
else
# 英語版
	makevox $(DEMO2LSS) $(DEMOKEY) $(DEMOPATH) > $(DEMOSRC)
	scr2xbox $(DEMOSRC) > _xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
endif
	stream _xbox.scr $(DISC2OPT) -d $(STREAMDEF) -o $(DEMO2FILE) -t $(DEMO2LIST)
	cat $(DEMO2LIST) | sed -e 's/p048_01_p01/vr_p048_01_p01/g' > _xbox.scr
	cp _xbox.scr $(DEMO2LIST)
	rm -f _xbox.scr
endif # MGS2_SUBSTANCE

# demo ここまで

# movie --------------------------------------------------------------

movie :
#ifeq ($(MGS2_SCN),1)
#	@echo `date` `whoami` ": MOVIEを作り直します。" | nkf -s | $(MAILMSG) scn
#endif

ifeq ($(MGS2_VMODE), PAL)
# PAL版
	scr2xbox $(MOVIESRC) > _movie_xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
	scr2pal  _movie_xbox.scr > _movie_pal.scr
	sed -e 's#/sound.pal/#/sound.eng/#g' _movie_pal.scr > _movie_xbox.scr
	stream _movie_xbox.scr $(DISC1OPT) -d $(STREAMDEF) -o $(MOVIEFILE) -t $(MOVIELIST)
	rm -f _movie_pal.scr _movie_xbox.scr

	scr2xbox $(MOVIEVRSRC) > _movievr_xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
	scr2pal _movievr_xbox.scr > _movievr_pal.scr
	stream _movievr_pal.scr $(DISC2OPT) -d $(STREAMDEF) -o $(MOVIEVRFILE) -t $(MOVIEVRLIST)
	rm -f _movievr_pal.scr _movievr_xbox.scr
else
# 英語版
	scr2sub $(MOVIESRC) > _movie_eng.scr
	scr2xbox _movie_eng.scr > _movie_xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
	stream _movie_xbox.scr $(DISC1OPT) -d $(STREAMDEF) -o $(MOVIEFILE) -t $(MOVIELIST)
	rm -f _movie_pal.scr _movie_xbox.scr
	scr2sub $(MOVIEVRSRC) > _movie_eng.scr
	scr2xbox _movie_eng.scr > _movie_xbox.scr  #Xboxじゃない場合はちゃんとスルーする MGS2_XBOXで判断する
	stream _movie_xbox.scr $(DISC2OPT) -d $(STREAMDEF) -o $(MOVIEVRFILE) -t $(MOVIEVRLIST)
	rm -f _movie_pal.scr _movie_xbox.scr
endif


getdashboardimage :
ifeq ($(MGS2_XBOX),1)
	mkdir -p ../cdrom.img/image
	cp -fu  $(FIXPATH)/binary/$(TITLEIMAGE) ../cdrom.img/image
#	(for i in $(SAVEIMAGES) ; do cp -fu $(FIXPATH)/binary/$$i ../cdrom.img/image ; done)
endif

# ---- depend
