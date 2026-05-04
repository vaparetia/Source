#ifndef __BP_ENDIANSUPPORT_H__
#define __BP_ENDIANSUPPORT_H__
#pragma once

#include "MGS_SysCommon.h"
#include "BP_Math.h"

//-----------------------------------------------------------------------------
// INPLACE FUNCTIONS
//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_Swap2Bytes( void *dst, void const *src )
{
   // Safe to use when src and dst overlap
   unsigned short const src_short= *( (unsigned short const *)src );
   unsigned short *pDst_short = (unsigned short *) dst;

#if BPE_IS_ENDIAN_BIG()
   *pDst_short = src_short >> 8 | ( ( src_short & 0xFF ) << 8 );
#else
   *pDst_short = src_short;
#endif
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_Swap4Bytes( void *dst, void const *src )
{
   unsigned int const src_int = *( (unsigned int const *)src );
   unsigned int *pDst_int = (unsigned int *) dst;

#if BPE_IS_ENDIAN_BIG()
   *pDst_int = 
      ( ( src_int & 0x000000FF ) << 24 ) |
      ( ( src_int & 0x0000FF00 ) << 8 ) |
      ( ( src_int & 0x00FF0000 ) >> 8 ) |
      ( ( src_int & 0xFF000000 ) >> 24 );
#else
   *pDst_int = src_int;
#endif
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_Swap8Bytes( void *dst, void const *src )
{
   unsigned long long const src_ll = *( (unsigned long long const *)src );
   unsigned long long *pDst_ll = (unsigned long long *) dst;

#if BPE_IS_ENDIAN_BIG()
   *pDst_ll = 
      ( ( src_ll & 0x00000000000000FFULL ) << 56ULL ) |
      ( ( src_ll & 0x000000000000FF00ULL ) << 40ULL ) |
      ( ( src_ll & 0x0000000000FF0000ULL ) << 24ULL ) |
      ( ( src_ll & 0x00000000FF000000ULL ) << 8ULL ) |
      ( ( src_ll & 0x000000FF00000000ULL ) >> 8ULL ) |
      ( ( src_ll & 0x0000FF0000000000ULL ) >> 24ULL ) |
      ( ( src_ll & 0x00FF000000000000ULL ) >> 40ULL ) |
      ( ( src_ll & 0xFF00000000000000ULL ) >> 56ULL );
#else
   *pDst_ll = src_ll;
#endif
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_Swap2Bytes_Inp( void *dst )
{ 
   BP_LE_Swap2Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_Swap4Bytes_Inp( void *dst )
{ 
   BP_LE_Swap4Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_Swap8Bytes_Inp( void *dst )                    
{ 
   BP_LE_Swap8Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapSShort_Inp( short *dst )                   
{ 
   BP_LE_Swap2Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapUShort_Inp( unsigned short *dst )          
{ 
   BP_LE_Swap2Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapSInt_Inp( int *dst )                       
{ 
   BP_LE_Swap4Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapSIntArray_Inp( int *dst, int count )       
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   {
      BP_LE_Swap4Bytes( dst + i, dst + i ); 
   }
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapUIntArray_Inp( unsigned int *dst, int count )
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   {
      BP_LE_Swap4Bytes( dst + i, dst + i );
   }
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapUInt_Inp( unsigned int *dst )              
{ 
   BP_LE_Swap4Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_SwapCheckFloat( float f )
{
   XASSERT( BP_Float_Check( f ), "bad float endian swap?" ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapFloat_Inp( float *dst )                    
{ 
   BP_LE_Swap4Bytes( dst, dst ); 
   BP_SwapCheckFloat( *dst );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapFloatArray_Inp( float *dst, int count )    
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   { 
      BP_LE_Swap4Bytes( dst + i, dst + i ); 
      BP_SwapCheckFloat( dst[i] );
   } 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapFVector_Inp( FVECTOR *dst )                
{ 
   BP_LE_Swap4Bytes( &dst->vx, &dst->vx ); 
   BP_LE_Swap4Bytes( &dst->vy, &dst->vy ); 
   BP_LE_Swap4Bytes( &dst->vz, &dst->vz ); 
   BP_LE_Swap4Bytes( &dst->vw, &dst->vw ); 

   // Just check XYZ components since W is often 0xffffffff
   BP_SwapCheckFloat( dst->vx );
   BP_SwapCheckFloat( dst->vy );
   BP_SwapCheckFloat( dst->vz );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapSVector_Inp( SVECTOR *dst )                
{ 
   BP_LE_Swap2Bytes( &dst->vx, &dst->vx ); 
   BP_LE_Swap2Bytes( &dst->vy, &dst->vy ); 
   BP_LE_Swap2Bytes( &dst->vz, &dst->vz ); 
   BP_LE_Swap2Bytes( &dst->pad, &dst->pad ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapFMatrix_Inp( FMATRIX *dst )                
{ 
   BP_LE_SwapFloatArray_Inp( &dst->m[0][0], 16 );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapSShortArray_Inp( short *dst, int count )   
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   {
      BP_LE_Swap2Bytes( dst + i, dst + i ); 
   }
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapUShortArray_Inp( unsigned short *dst, int count )   
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   {
      BP_LE_Swap2Bytes( dst + i, dst + i ); 
   }
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapPtr_Inp( void **dst )                      
{ 
   BP_LE_Swap4Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapSLongLong_Inp( long long *dst )            
{ 
   BP_LE_Swap8Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_LE_SwapULongLong_Inp( unsigned long long *dst )   
{ 
   BP_LE_Swap8Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------
// RETURNING FUNCTIONS
//-----------------------------------------------------------------------------

EXTERN_INLINE short BP_LE_SwapSShort( short src )                               
{ 
   short dst;               
   BP_LE_Swap2Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE unsigned short BP_LE_SwapUShort( unsigned short src )             
{ 
   unsigned short dst;      
   BP_LE_Swap2Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE int BP_LE_SwapSInt( int src )                                     
{ 
   int dst;                 
   BP_LE_Swap4Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE unsigned int BP_LE_SwapUInt( unsigned int src )                   
{ 
   unsigned int dst;        
   BP_LE_Swap4Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE float BP_LE_SwapFloat( float src )                                
{ 
   float dst;               
   BP_LE_Swap4Bytes( &dst, &src ); 
   XASSERT( BP_Float_Check( dst ), "bad float endian swap?" ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE long long BP_LE_SwapSLongLong( long long src )                    
{ 
   long long dst;           
   BP_LE_Swap8Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE unsigned long long BP_LE_SwapULongLong( unsigned long long src )  
{ 
   unsigned long long dst;  
   BP_LE_Swap8Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------
// Big Endian
//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_Swap2Bytes( void *dst, void const *src )
{
   // Safe to use when src and dst overlap
   unsigned short const src_short= *( (unsigned short const *)src );
   unsigned short *pDst_short = (unsigned short *) dst;

#if !BPE_IS_ENDIAN_BIG()
   *pDst_short = src_short >> 8 | ( ( src_short & 0xFF ) << 8 );
#else
   *pDst_short = src_short;
#endif
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_Swap4Bytes( void *dst, void const *src )
{
   unsigned int const src_int = *( (unsigned int const *)src );
   unsigned int *pDst_int = (unsigned int *) dst;

#if !BPE_IS_ENDIAN_BIG()
   *pDst_int = 
      ( ( src_int & 0x000000FF ) << 24 ) |
      ( ( src_int & 0x0000FF00 ) << 8 ) |
      ( ( src_int & 0x00FF0000 ) >> 8 ) |
      ( ( src_int & 0xFF000000 ) >> 24 );
#else
   *pDst_int = src_int;
#endif
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_Swap8Bytes( void *dst, void const *src )
{
   unsigned long long const src_ll = *( (unsigned long long const *)src );
   unsigned long long *pDst_ll = (unsigned long long *) dst;

#if !BPE_IS_ENDIAN_BIG()
   *pDst_ll = 
      ( ( src_ll & 0x00000000000000FFULL ) << 56ULL ) |
      ( ( src_ll & 0x000000000000FF00ULL ) << 40ULL ) |
      ( ( src_ll & 0x0000000000FF0000ULL ) << 24ULL ) |
      ( ( src_ll & 0x00000000FF000000ULL ) << 8ULL ) |
      ( ( src_ll & 0x000000FF00000000ULL ) >> 8ULL ) |
      ( ( src_ll & 0x0000FF0000000000ULL ) >> 24ULL ) |
      ( ( src_ll & 0x00FF000000000000ULL ) >> 40ULL ) |
      ( ( src_ll & 0xFF00000000000000ULL ) >> 56ULL );
#else
   *pDst_ll = src_ll;
#endif
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_Swap2Bytes_Inp( void *dst )
{ 
   BP_BE_Swap2Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_Swap4Bytes_Inp( void *dst )
{ 
   BP_BE_Swap4Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_Swap8Bytes_Inp( void *dst )                    
{ 
   BP_BE_Swap8Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapSShort_Inp( short *dst )                   
{ 
   BP_BE_Swap2Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapUShort_Inp( unsigned short *dst )          
{ 
   BP_BE_Swap2Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapSInt_Inp( int *dst )                       
{ 
   BP_BE_Swap4Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapSIntArray_Inp( int *dst, int count )       
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   {
      BP_BE_Swap4Bytes( dst + i, dst + i ); 
   }
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapUIntArray_Inp( unsigned int *dst, int count )
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   {
      BP_BE_Swap4Bytes( dst + i, dst + i );
   }
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapUInt_Inp( unsigned int *dst )              
{ 
   BP_BE_Swap4Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapFloat_Inp( float *dst )                    
{ 
   BP_BE_Swap4Bytes( dst, dst ); 
   BP_SwapCheckFloat( *dst );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapFloatArray_Inp( float *dst, int count )    
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   { 
      BP_BE_Swap4Bytes( dst + i, dst + i ); 
      BP_SwapCheckFloat( dst[i] );
   } 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapFVector_Inp( FVECTOR *dst )                
{ 
   BP_BE_Swap4Bytes( &dst->vx, &dst->vx ); 
   BP_BE_Swap4Bytes( &dst->vy, &dst->vy ); 
   BP_BE_Swap4Bytes( &dst->vz, &dst->vz ); 
   BP_BE_Swap4Bytes( &dst->vw, &dst->vw ); 

   // Just check XYZ components since W is often 0xffffffff
   BP_SwapCheckFloat( dst->vx );
   BP_SwapCheckFloat( dst->vy );
   BP_SwapCheckFloat( dst->vz );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapSVector_Inp( SVECTOR *dst )                
{ 
   BP_BE_Swap2Bytes( &dst->vx, &dst->vx ); 
   BP_BE_Swap2Bytes( &dst->vy, &dst->vy ); 
   BP_BE_Swap2Bytes( &dst->vz, &dst->vz ); 
   BP_BE_Swap2Bytes( &dst->pad, &dst->pad ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapFMatrix_Inp( FMATRIX *dst )                
{ 
   BP_BE_SwapFloatArray_Inp( &dst->m[0][0], 16 );
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapSShortArray_Inp( short *dst, int count )   
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   {
      BP_BE_Swap2Bytes( dst + i, dst + i ); 
   }
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapUShortArray_Inp( unsigned short *dst, int count )   
{ 
   int i; 
   for ( i = 0; i < count; ++i )
   {
      BP_BE_Swap2Bytes( dst + i, dst + i ); 
   }
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapPtr_Inp( void **dst )                      
{ 
   BP_BE_Swap4Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapSLongLong_Inp( long long *dst )            
{ 
   BP_BE_Swap8Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE void BP_BE_SwapULongLong_Inp( unsigned long long *dst )   
{ 
   BP_BE_Swap8Bytes( dst, dst ); 
}

//-----------------------------------------------------------------------------
// RETURNING FUNCTIONS
//-----------------------------------------------------------------------------

EXTERN_INLINE short BP_BE_SwapSShort( short src )                               
{ 
   short dst;               
   BP_BE_Swap2Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE unsigned short BP_BE_SwapUShort( unsigned short src )             
{ 
   unsigned short dst;      
   BP_BE_Swap2Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE int BP_BE_SwapSInt( int src )                                     
{ 
   int dst;                 
   BP_BE_Swap4Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE unsigned int BP_BE_SwapUInt( unsigned int src )                   
{ 
   unsigned int dst;        
   BP_BE_Swap4Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE float BP_BE_SwapFloat( float src )                                
{ 
   float dst;               
   BP_BE_Swap4Bytes( &dst, &src ); 
   XASSERT( BP_Float_Check( dst ), "bad float endian swap?" ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE long long BP_BE_SwapSLongLong( long long src )                    
{ 
   long long dst;           
   BP_BE_Swap8Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------

EXTERN_INLINE unsigned long long BP_BE_SwapULongLong( unsigned long long src )  
{ 
   unsigned long long dst;  
   BP_BE_Swap8Bytes( &dst, &src ); 
   return dst; 
}

//-----------------------------------------------------------------------------


#endif