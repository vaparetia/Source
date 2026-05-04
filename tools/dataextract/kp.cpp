#include <stdio.h>
#include <stdlib.h>

#include "kp.h"

unsigned int KPCrypt::seed = 0;
unsigned int KPCrypt::_seed = 0;

int GV_StrCode( char const *string )
{
#define BIT_LEN		24

   unsigned char c;
   unsigned char *p;
   unsigned int id, mask;

   p = ( unsigned char * )string;
   id = 0;
   mask = ( 1 << BIT_LEN ) - 1;

   while( ( c = *( p++ ) ) != '\0' ){
      id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
      id += c;
      id &= mask;
   }
   if( id == 0 ) id = 1;

   return id;
#undef BIT_LEN
}

void KPCrypt::setup_decrypt( void *buffer )
{
   unsigned short *p = (unsigned short *) buffer;
   unsigned int s;

   s = *p ^ 0x9385;
   seed = ( s ) | ( ( s ^ 0x6576 ) << 16 );
   _seed = s * 278;
}

void KPCrypt::decrypt_buffer( void *buffer, int size )
{
   unsigned int *p = (unsigned int *) buffer;
   unsigned int s, _s;

   s = seed;
   _s = _seed;

   for( ; size > 0; size -= sizeof( int ) ){
      *p = *p ^ s;
      s = s * 48828125 + _s;
      p++;
   }
   seed = s;
}

void KPCrypt::set_encode( void )
{
   int s;

   //   encode_flag = 1;
   s = ( rand() >> 15 ) & 0xFFFF;
   seed = ( s ) | ( ( s ^ 0x6576 ) << 16 );
   _seed = s * 278;
   //printf( "encode seed %X %X %X\n", s, seed, _seed );
}

void KPCrypt::encode_buffer( void *buffer, int size )
{
   unsigned int *p = (unsigned int *) buffer;

#if 0
   if( compress_mode == 0 ){
      // 圧縮データでない場合は暗号化しない
      return;
   }
#endif

   for( ; size > 0; size -= sizeof( int ) ){
      *p = *p ^ seed;
      seed = seed * 48828125 + _seed;
      p++;
   }
}

