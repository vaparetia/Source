#pragma once

#ifndef __TRANSFARRING_MENU__H__
#define __TRANSFARRING_MENU__H__

#if MGS_VERSION == 3
#include "common.h"
GV_HANDLE NewTransfarringMenu();
#endif

#if MGS_VERSION == 2
void* NewTransfarringMenu();
#endif

#endif