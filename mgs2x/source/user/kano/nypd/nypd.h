/*
	nypd.h
		ニューヨーク市警初期化関数群

	2000/01/24 K.Kano
	$Id: nypd.h,v 1.1.1.3 2002/11/19 11:43:25 Yoshizawa1 Exp $
*/

#ifndef _nypd_h_
#define _nypd_h_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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


int InitObject_NYPD(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type);
void FreeObject_NYPD(OBJECT *object,OBJECT_CHG *work);


#endif
