//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* SCEI CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library  Release 2.0
 */
/*
 *              Emotion Engine Library Sample Program
 *
 *                       - mpeg streaming -
 *
 *                         Version 0.10
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                            vobuf.c
 *             functions for video output buffer
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 */
#include <eekernel.h>
#include "defs.h"
#include "vobuf.h"

//BP
static void DI() {}
static void EI() {}

// ////////////////////////////////////////////////////////////////
//
// Functions called from decoding thread
//
void voBufCreate(
    VoBuf *f,
    VoData *data,
    int size
)
{
    f->data = data;
    f->size = size;
    f->count = 0;
    f->write = 0;
}

void voBufDelete(VoBuf *f)
{
}

void voBufReset(VoBuf *f)
{
    f->count = 0;
    f->write = 0;
}

int voBufIsFull(VoBuf *f)
{
    return f->count == f->size;
}

void voBufIncCount(VoBuf *f)
{
    // disable interrupt
    DI();

    f->count++;
    f->write = (f->write + 1) % f->size;

    // enable interrupt
    EI();
}

VoData *voBufGetData(VoBuf *f)
{
    return voBufIsFull(f)? (VoData*)NULL: f->data + f->write;
}

// ////////////////////////////////////////////////////////////////
//
// Functions called from interrupt handler
//
int voBufIsEmpty(VoBuf *f)
{
    return f->count == 0;
}

void voBufDecCount(VoBuf *f)
{
    if (f->count > 0) {
	f->count--;
    }
}

