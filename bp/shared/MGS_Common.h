#pragma once

#if MGS_VERSION==3

#ifdef __cplusplus
extern "C"
{
#pragma warning ( disable : 4200 )
#define class class_type
#endif

#include "common.h"

#ifdef __cplusplus
#undef class
};
#endif

#endif

#if MGS_VERSION==2

#include	<math.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "gameheader.h"
#include "mgs_type.h"

#ifdef __cplusplus
};
#endif

#endif
