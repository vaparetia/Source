//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	atanes.c
	攻撃兵 本体

	2000/09/30 K.Sigeno
	$Id: atanes.c,v 1.1.1.3 2002/11/19 11:49:00 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"
#include "at_thk.h"
#include "at_enum.h"

void	AT_AnesThink(ENETHINK *entk){
	AT_THK *at_thk ;
	at_thk = (AT_THK *) entk->character ;
	if(at_thk->at_status & AT_ST_WAKE ){
		/*救助員派遣済み*/
		if(entk->buddy != NULL ){
			if(entk->buddy->think2 != TH2_RESCUE){
				/*相棒が救助中断されたので再要請*/
				at_thk->at_status &= ~AT_ST_WAKE ;
			}
			return ;
		}
		at_thk->at_status &= ~AT_ST_WAKE ;
	}
}
