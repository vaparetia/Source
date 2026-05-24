//----------------------------------------------------------------------------
// DCMain.cpp
// KallistiOS entry point for the MGS2 Dreamcast port.
//
// KOS calls main() after hardware initialization (video, timers, IRQs).
// KOS_INIT_FLAGS must appear exactly once in the final binary; it lives here.
//----------------------------------------------------------------------------

#include <kos.h>

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char* argv[])
{
   (void)argc; (void)argv;

   // Phase 3 stub: verifies the engine links against KOS.
   // Engine + renderer initialization goes here once linking is clean.

   return 0;
}
