/*
	bgscr.h
		タイトル等のバックグランド画面のアニメーション

	2001/06/15 K.Kano
	$Id: bgscr.h,v 1.1.1.3 2002/11/19 11:43:36 Yoshizawa1 Exp $
*/


#ifndef _bgscr_h_
#define _bgscr_h_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"


enum {
	BG_MSG_UP=0,
	BG_MSG_DOWN,
	BG_MSG_LEFT,
	BG_MSG_RIGHT,
	BG_MSG_RIGHTUP,
	BG_MSG_LEFTDOWN,
	BG_MSG_STOP,

	BG_MSG_DISAPPEAR,
	BG_MSG_FADE,

	BG_MSG_OPENGRD,
	BG_MSG_CLOSEGRD,

	BG_MSG_OPENPLMAP,
	BG_MSG_CLOSEPLMAP,

	BG_MSG_CHGFACE,

	BG_MSG_GENZI,
};

enum {
	GENZI_MSG_SHOW_MAIN=0,
	GENZI_MSG_HIDE_MAIN,
	GENZI_MSG_SHOW_NEWGAME,
	GENZI_MSG_HIDE_NEWGAME,
	GENZI_MSG_SHOW_QUESTION,
	GENZI_MSG_HIDE_QUESTION,
	GENZI_MSG_SHOW_DIFFICULTY,
	GENZI_MSG_HIDE_DIFFICULTY,
	GENZI_MSG_SHOW_OPTION,
	GENZI_MSG_HIDE_OPTION,
};


void ComNodeFrameAction(int action_strcode);
int ComNodeFrameBusy(void);

/* source/user/mode/node/node_bg.c */
void *NewNodeBG(void);


#endif
