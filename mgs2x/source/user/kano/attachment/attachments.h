/*
	attachments.c
		装備品揺らし

	1999/11/22 K.Kano
	$Id: attachments.h,v 1.1.1.3 2002/11/19 11:43:05 Yoshizawa1 Exp $
*/


#ifndef _attachments_h_
#define _attachments_h_

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


typedef struct {
    int model_name;
    FVECTOR *v;
    SVECTOR *r;

    int objnum;
    FVECTOR *x;

    int *a;
    int angle_limit;
    float oval_param;
} ATTACHMENT_ARGUMENT;

typedef struct {
    int model_name;
    FVECTOR *v;
    SVECTOR *r;

    int objnum;
    FVECTOR *x;
    int objnum2;
    FVECTOR *p;
} ATTACHMENT_ARGUMENT2;

typedef struct {
    int model_name;
    SVECTOR *r;

    int objnum;
    FVECTOR *x;
    int frames;
} ATTACHMENT_ARGUMENT3;


/* attachments.c */
void *NewAttachments_called(OBJECT *target,const ATTACHMENT_ARGUMENT *arg,int size,
			    const ATTACHMENT_ARGUMENT2 *arg2,int size2,
			    const ATTACHMENT_ARGUMENT3 *arg3,int size3);

FVECTOR *GetNowQFromAttachment2(void *param,int index);

#endif
