#ifndef _layout_conv_h_
#define _layout_conv_h_

#define POS_VTX_EMPTY      1
#define POS_VTX_POINT      1
#define POS_VTX_LINE       2
#define POS_VTX_LINESTRIP -1
#define POS_VTX_BOX        2
#define POS_VTX_SPRITE     1
#define POS_VTX_ZOOM       2
#define POS_VTX_SPIN       1
#define POS_VTX_POLY       4
#define POS_VTX_TRIANGLE   3

#define RGB_VTX_EMPTY       0
#define RGB_VTX_POINT       1
#define RGB_VTX_LINE        2
#define RGB_VTX_LINESTRIP  -1
#define RGB_VTX_BOX         1
#define RGB_VTX_SPRITE      1
#define RGB_VTX_ZOOM        1
#define RGB_VTX_SPIN        1
#define RGB_VTX_POLY        4
#define RGB_VTX_TRIANGLE    3






#if 0 //BP_PS2

#ifdef PSX2
#ifndef _layout_conv_c_
#define EXT  extern
#define INIT(_n...)
#else
#define EXT
#define INIT(_n...) = _n
#endif /* _layout_conv_c_ */
#endif

#endif


#undef EXT
#undef INIT

int lconvEmpty(l2dSprite * spr, int form1, int form2, float rate);
int lconvPoint(l2dSprite * spr, int form1, int form2, float rate);
int lconvLine(l2dSprite * spr, int form1, int form2, float rate);
int lconvLineStrip(l2dSprite * spr, int form1, int form2, float rate);
int lconvBox(l2dSprite * spr, int form1, int form2, float rate);
int lconvSprite(l2dSprite * spr, int form1, int form2, float rate);
int lconvZoom(l2dSprite * spr, int form1, int form2, float rate);
int lconvSpin(l2dSprite * spr, int form1, int form2, float rate);
int lconvPoly(l2dSprite * spr, int form1, int form2, float rate);
int lconvTriangle(l2dSprite * spr, int form1, int form2, float rate);

#endif /* _layout_conv_h_ */
