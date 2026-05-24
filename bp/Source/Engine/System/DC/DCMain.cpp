//----------------------------------------------------------------------------
// DCMain.cpp
// KallistiOS entry point for the MGS2 Dreamcast port.
//
// KOS calls main() after hardware initialization (video, timers, IRQs).
// KOS_INIT_FLAGS must appear exactly once in the final binary; it lives here.
//----------------------------------------------------------------------------

#include <kos.h>
#include <dc/video.h>
#include <dc/biosfont.h>

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char* argv[])
{
   (void)argc; (void)argv;

   vid_set_mode(DM_640x480, PM_RGB565);

   // Fill framebuffer blue so boot is visually confirmed before any engine init.
   uint16 *fb = vram_s;
   for (int i = 0; i < 640 * 480; i++)
      fb[i] = 0x001F;  /* blue in RGB565 */

   bfont_set_encoding(BFONT_CODE_ISO8859_1);
   bfont_draw_str(fb + 40 + 180 * 640, 640, 1, "MGS2 Dreamcast Port");
   bfont_draw_str(fb + 40 + 216 * 640, 640, 1, "Phase 4 Boot Test OK");

   for (;;) {}
   return 0;
}
