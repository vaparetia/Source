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
#include <dc/pvr.h>

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char* argv[])
{
   (void)argc; (void)argv;

   // Phase 4a: direct framebuffer — confirms ELF loads and KOS runs.
   vid_set_mode(DM_640x480, PM_RGB565);
   uint16 *fb = vram_s;
   for (int i = 0; i < 640 * 480; i++)
      fb[i] = 0x001F;  /* blue in RGB565 */
   bfont_set_encoding(BFONT_CODE_ISO8859_1);
   bfont_draw_str(fb + 40 + 180 * 640, 640, 1, "MGS2 Dreamcast Port");
   bfont_draw_str(fb + 40 + 216 * 640, 640, 1, "Initializing PVR...");

   thd_sleep(1000);

   // Phase 4b: PVR tile accelerator — confirms pvr_init and scene loop work.
   // Green background = PVR is rendering frames; different from the direct-VRAM
   // blue above so the transition is visually unambiguous.
   pvr_init_defaults();
   pvr_set_bg_color(0.0f, 0.5f, 0.0f);

   for (;;) {
      pvr_wait_ready();
      pvr_scene_begin();
      pvr_list_begin(PVR_LIST_OP_POLY);
      pvr_list_finish();
      pvr_list_begin(PVR_LIST_TR_POLY);
      pvr_list_finish();
      pvr_scene_finish();
   }

   return 0;
}
