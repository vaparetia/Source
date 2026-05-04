/*
	us_soldier.h
		アメリカ兵初期化関数群

	2000/01/24 K.Kano
	$Id: citizen.h,v 1.1.1.3 2002/11/19 11:45:54 Yoshizawa1 Exp $
*/

#ifndef _us_soldier_h_
#define _us_soldier_h_


#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"


int InitObject_USSoldier(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type);
void FreeObject_USSoldier(OBJECT *object,OBJECT_CHG *work);


#endif
