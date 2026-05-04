#ifndef _MGS_SYSCOMMON
#define _MGS_SYSCOMMON
#pragma once

#ifndef MGS_VERSION
#error please define MGS_VERSION=2 or MGS_VERSION=3
#endif

#if MGS_VERSION==3

#ifdef __cplusplus
extern "C"
{
#endif

#include "sys_common.h"

#ifdef __cplusplus
}
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#endif

#if MGS_VERSION==2

#ifdef __cplusplus

#include <math.h>
#include <string.h>

extern "C"
{
#endif

#include "gameheader.h"
#include "mgs_type.h"

#ifdef __cplusplus
}
#endif

#endif

#endif