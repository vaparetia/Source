/*
	handcuff.h
		手錠の鎖を計算

	2001/06/09 K.Kano
	$Id: handcuff.h,v 1.1.1.3 2002/11/19 11:43:12 Yoshizawa1 Exp $
*/


#ifndef _handcuff_h_
#define _handcuff_h_


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>

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


int Rope_GetModelName(int *name);


#endif
