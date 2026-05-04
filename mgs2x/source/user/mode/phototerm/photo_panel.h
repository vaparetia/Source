#ifndef _photo_panel_h_
#define _photo_panel_h_

#include "BP_BuildDefines.h"
#include "BP_Font.h"

/*
 * 転送端末用テキストパネル
 */
typedef struct PHOTO_MESG_PANEL {
#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_VRAMINFO  bp_vinfo;
   struct _SBP_BufferedTexture* pbp_BufferedTexture;
#else
   FONT_VRAMINFO vinfo;
   void        * vram;
   int           vramsize;
   DG_TEX_LIN *tex;
#endif

   int           width, height;
   int				tex_width_xbox;

   DG_DMAPACK  * dmapack;

   int           panel_dx;
   int           panel_dy;
   int           panel_w;
   int           panel_h;

   void				*packet_mem;
   void                *prim ;

} PHOTO_MESG_PANEL;




#ifndef _photo_panel_c_
#define EXT  extern
#else
#define EXT
#endif  /* _photo_panel_c_ */



#undef EXT



void PHOTO_OpenMesgPanel( PHOTO_MESG_PANEL *panel, int width, int height );
void PHOTO_CloseMesgPanel( PHOTO_MESG_PANEL *panel );

void PHOTO_DrawMessage( PHOTO_MESG_PANEL *panel, char *mesg );
void PHOTO_DrawMesgPanel(PHOTO_MESG_PANEL *panel, int x, int y, int w, int h);


#endif /* _photo_panel_h_ */
