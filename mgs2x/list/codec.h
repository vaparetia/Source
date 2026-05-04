//
// 無線モード用パーツ
// codec.h
// 
// 2000/09/06  Y.Kira
//
// $Id: codec.h,v 1.12 2002/07/15 05:24:40 usr04761 Exp $
//

#ifdef MGS2_XBOX
/* XBOX */
/* ボタン名称が違うので、テクスチャ変更 */
l2d	   2D/codec_x/codec.l2d
l2d	   2D/call_sub_x/call.l2d
#else
/* PS2 */
l2d	   2D/codec/codec.l2d
l2d	   2D/call_icn/call.l2d
#endif
