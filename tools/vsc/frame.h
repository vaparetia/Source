#ifndef _frame_h_
#define _frame_h_

#include "env.h"
#include "pcmdata.h"

/*
 * Global variables
 */
#ifndef _frame_c_
#define EXT extern
#else
#define EXT
#endif /* _frame_c_ */

EXT   short  frmWindowBuf[ FRAME_WINDOW ];    /* 現在の25ms分         */
EXT   int    frmWindowSize;                   /* 現在有効なフレーム数 */

#undef EXT  /* end of global variables section */


/* =========================================================================
 * Function prototypes
 * ========================================================================= */

int frmInit(PcmFILE *pcm);
int frmSlideWindow(PcmFILE *pcm);

#endif /* _frame_h_ */
