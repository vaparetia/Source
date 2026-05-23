#pragma once

#define RVL_FILEIO_DVD 0
#define RVL_FILEIO_MCS 1

#define RVL_FILEIO RVL_FILEIO_MCS

#include "RVLFileIO_Mcs.h"
//#include "RVLFileIO_Dvd.h"

#if RVL_FILEIO == RVL_FILEIO_MCS
#define RVLFileIO RVLFileIO_Mcs
#else
#define RVLFileIO RVLFileIO_Dvd
#endif
