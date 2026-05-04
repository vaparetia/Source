/*
  CRI Middleware 用 ストリームドライバ

  2002/05/23 M.Kobayashi
  $Id: cristream.h,v 1.1.1.2 2002/11/19 11:41:47 Yoshizawa1 Exp $

 */

#ifndef __CRISTREAM_H__
#define __CRISTREAM_H__
extern int GM_StreamGetCriInfo( int *width, int *height, int *frame );
extern void* GM_StreamGetCri( void );
extern void GM_StreamCopyCriImage( void *dst, int dst_width, int dst_height, int dst_x, int dst_y, int flag );
extern void GM_StreamFreeCri( void );
extern BOOL	GM_StreamIsEndCri( void );

extern void GM_StreamCriDriverInit( int dummy );
extern void GM_StreamCriDriverEnd( void );

extern void *NewMemStreamIpic( void *ipu_data, int mode );


#define NewMemStreamIpu	NewMemStreamIpic
#define GM_StreamFreeIpu GM_StreamFreeCri
#define GM_StreamCopyIpuImage GM_StreamCopyCriImage
#define GM_StreamGetIpuInfo GM_StreamGetCriInfo
#define GM_StreamGetIpu GM_StreamGetCri
#define GM_StreamIpuDriverInit GM_StreamCriDriverInit
#define GM_StreamIpuDriverEnd GM_StreamCriDriverEnd

#endif //__CRISTREAM_H__
