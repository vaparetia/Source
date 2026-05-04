/*
	raincoat.h
		レインコート

	2000/2/15 K.Kano
	$Id: raincoat.h,v 1.1.1.3 2002/11/19 11:43:33 Yoshizawa1 Exp $
*/

#ifndef _raincoat_h_
#define _raincoat_h_


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
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

#include "mm_coat.h"
#include "va_coat.h"
#include "dispcoat.h"


#endif
