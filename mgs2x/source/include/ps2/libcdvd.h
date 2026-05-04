//BP - Stub file
#ifndef __LIBCDVD_H__
#define __LIBCDVD_H__

#define SyncDCache(_addr, _size)
#define iSyncDCache(_addr, _size)

typedef struct {
   unsigned char stat;
   unsigned char second;
   unsigned char minute;
   unsigned char hour;
   unsigned char pad;
   unsigned char day;
   unsigned char month;
   unsigned char year;
} sceCdCLOCK;


#endif//#ifndef __LIBCDVD_H__
