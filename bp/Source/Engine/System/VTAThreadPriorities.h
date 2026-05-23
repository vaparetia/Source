#pragma once
#ifndef __VTATHREADPRIORITIES__H
#define __VTATHREADPRIORITIES__H

#include <kernel/threadmgr.h>

#ifdef __cplusplus
namespace NVtaThreadPriorities
{
#endif
   int const kPrioritySoundHWUpdate    = SCE_KERNEL_DEFAULT_PRIORITY_USER - 3;
   int const kPrioritySoundLogicUpdate = SCE_KERNEL_DEFAULT_PRIORITY_USER - 2;
   int const kPriorityMovieUpdate      = SCE_KERNEL_DEFAULT_PRIORITY_USER - 2;
   int const kPrioritySaveLoad         = SCE_KERNEL_DEFAULT_PRIORITY_USER - 1;
   int const kPriorityTUS              = SCE_KERNEL_DEFAULT_PRIORITY_USER - 1;
   int const kPriorityRenderer         = SCE_KERNEL_DEFAULT_PRIORITY_USER;
   int const kPriorityEndFrame         = SCE_KERNEL_DEFAULT_PRIORITY_USER + 1;
   int const kPriorityUltWorkers       = SCE_KERNEL_DEFAULT_PRIORITY_USER + 2;
   int const kPriorityUltDrainers      = SCE_KERNEL_DEFAULT_PRIORITY_USER + 3;
#ifdef __cplusplus
}
#endif

#endif
