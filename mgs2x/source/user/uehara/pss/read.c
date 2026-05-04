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
 *                            read.c
 *               callback functions for elemetary streams
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           12.17.1999      umemura     the first version
 */

#if defined(BP_WIN32) || defined(BP_360)
typedef __int64 int64;
#elif defined(BP_PS3) || defined(BP_VITA)
typedef long long int64;
#else
#error todo
#endif

#include <stdio.h>
#include <string.h>
#include <libmpeg.h>
#include "readbuf.h"
#include "videodec.h"
#include "defs.h"

static int copy2area(u_char *pd0, int d0, u_char *pd1, int d1,
    u_char *ps0, int s0, u_char *ps1, int s1);

// ////////////////////////////////////////////////////////////////
//
// Stream callback function for MPEG2 video stream
//
int videoCallback(sceMpeg *mp, sceMpegCbDataStr *str, void *data)
{
   BP_TODO_BREAK;
#if 0 //BP_TODO
	VideoDec *vd = data;

    u_char *pd0;
    u_char *pd1;
    u_char *pd0Unc;
    u_char *pd1Unc;
    int d0, d1;
    int len;

	if( ( str->len > 0 && str->len < 100 ) || sceMpegIsEnd( mp ) ){
		printf( "len=%d %d\n", str->len, sceMpegIsEnd( mp ) );
	}

	videoDecBeginPut(vd, &pd0, &d0, &pd1, &d1);

    pd0Unc = (u_char*)UncAddr(pd0);
    pd1Unc = (u_char*)UncAddr(pd1);

    len = copy2area( pd0Unc, d0, pd1Unc, d1, str->data, str->len, NULL, 0 );

    // set PTS
    if (len > 0) {
		if (!videoDecPutTs(vd, str->pts, str->dts, pd0, len)) {
			ErrMessage("pts buffer overflow\n");
		}
    }

    videoDecEndPut(vd, len);

    // ////////////////////////////////////////////
    //
    // Return 0 if no data is put
    //
    return (len > 0)? 1: 0;
#else
   return 0;
#endif
}

// ////////////////////////////////////////////////////////////////
//
// Copy two areas
//
static int copy2area(u_char *pd0, int d0, u_char *pd1, int d1,
    u_char *ps0, int s0, u_char *ps1, int s1)
{
    if (d0 + d1 < s0 + s1) {
        return 0;
    }

    if (s0 >= d0) {
    	memcpy(pd0,		ps0,		d0);
    	memcpy(pd1,		ps0 + d0,	s0 - d0);
    	memcpy(pd1 + s0 - d0,	ps1,		s1);
    } else { // s0 < d0
    	if (s1 >= d0 - s0) {
			memcpy(pd0,		ps0,		s0);
			memcpy(pd0 + s0,	ps1,		d0 - s0);
			memcpy(pd1,		ps1 + d0 - s0,	s1 - (d0 - s0));
		} else { // s1 < d0 - s0
			memcpy(pd0,		ps0,		s0);
			memcpy(pd0 + s0,	ps1,		s1);
		}
    }
    return s0 + s1;
}
