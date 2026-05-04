/*
	attachactf.h
		装備品落下

	1999/11/05 K.Kano
	$Id: attachmentf.h,v 1.1.1.3 2002/11/19 11:43:05 Yoshizawa1 Exp $
*/


#ifndef _attachmentf_h_
#define _attachmentf_h_

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


typedef struct {
    int count;
    float floor_height;
    FVECTOR v;
} MODEL_PARAMETERF;

void InitAttachmentFall(DG_OBJS *objs,MODEL_PARAMETERF *mpf,FVECTOR *base);
int MoveAttachmentFall(DG_OBJS *objs,MODEL_PARAMETERF *mpf);


#endif
