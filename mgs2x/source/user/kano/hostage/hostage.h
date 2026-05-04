/*
	hostage.h
		人質初期化関数群

	2000/01/24 K.Kano
	$Id: hostage.h,v 1.1.1.3 2002/11/19 11:43:13 Yoshizawa1 Exp $
*/

#ifndef _hostage_h_
#define _hostage_h_


#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef PSX2
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <float.h>

#include "gameheader.h"
#include "libutl.h"


int InitObject_Hostage(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type);
int InitObject_MidHostage(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type);
int InitObject_LowHostage(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type);
void FreeObject_Hostage(OBJECT *object,OBJECT_CHG *work);


#endif
