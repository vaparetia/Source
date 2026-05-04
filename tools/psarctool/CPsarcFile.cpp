//----------------------------------------------------------------------------
// CPsarcFile.cpp
// encapsulates operations on a psarc file.
//----------------------------------------------------------------------------

#include "CPsarcFile.h"

#include "Engine/Mechanics/zlib/zlib.h"
#include "Engine/Basics/NEndian.h"

//----------------------------------------------------------------------------

enum
{
   kCompressionMethod_zlib = 'zlib',
   kCompressionMethod_lzma = 'lzma'
};

namespace
{
   const unsigned int   skMagicNumber = 'PSAR';

   void byteswap_psarc_header( SPsarcHeader & header )
   {
      NEndian::Swap4Bytes( &header.mMagicNumber );
      NEndian::Swap4Bytes( &header.mVersionNumber );
      NEndian::Swap4Bytes( &header.mCompressionMethod );
      NEndian::Swap4Bytes( &header.mTOCSize );
      NEndian::Swap4Bytes( &header.mTOCEntrySize );
      NEndian::Swap4Bytes( &header.mNumFiles );
      NEndian::Swap4Bytes( &header.mBlockSize );
      NEndian::Swap4Bytes( &header.mArchiveFlags );
   }

   void read_toc_entry( STOCEntry & entry, const unsigned char * const pInput, const int inputSize )
   {
      memset( &entry, 0, sizeof( entry ) );
      if( inputSize == 30 )
      {
         // 0-15: nameDigest
         memcpy( &entry.mNameDigest, pInput + 0, 16 );
         //16-19: blockListStart
         memcpy( (unsigned char*)&entry.mBlockListStart, pInput + 16, 4 );
         NEndian::Swap4Bytes( &entry.mBlockListStart );
         //20-24: originalSize
         memcpy( (unsigned char*)&entry.mOriginalSize + 3, pInput + 20, 5 );
         NEndian::Swap8Bytes( &entry.mOriginalSize );
         //25-29: startOffset
         memcpy( (unsigned char*)&entry.mStartOffset + 3, pInput + 25, 5 );
         NEndian::Swap8Bytes( &entry.mStartOffset );
      }
      else
      {
         printf("TODO: unsupported TOC entry size %d!\n", inputSize );
         throw false;
      }
   }

   int get_block_size_entry_size( const int blockSize )
   {
      if( blockSize <= 64*1024 )
      {
         return 2;
      }
      if( blockSize <= 16*1024*1024 )
      {
         return 3;
      }

      return 4;
   }

   unsigned int read_block_size_entry( const unsigned char * const pInput, const int inputSize )
   {
      if( inputSize > 4 || inputSize <= 0 )
      {
         printf("TODO: unsupported block entry size %d!\n", inputSize );
         throw false;
      }

      unsigned int blockSizeEntry = 0;
      memcpy( (unsigned char*)&blockSizeEntry + ( 4 - inputSize ), pInput, inputSize );
      NEndian::Swap4Bytes( &blockSizeEntry );
      return blockSizeEntry;
   }

   static int decompress_block( unsigned char * const dst, unsigned char * const src, const int decompressed_size, const int compressed_size )
   {
      int ret;
      z_stream mZStream = { 0 };

      ret = inflateInit(&mZStream);
      if( ret != Z_OK )
         return -3;

      mZStream.avail_out = decompressed_size;
      mZStream.next_out = (Bytef *)dst;
      mZStream.avail_in = compressed_size;
      mZStream.next_in = src;

      ret = inflate(&mZStream, Z_NO_FLUSH);
      if(ret == Z_STREAM_ERROR)
      {
         return -1;
      }
      return 0;
   }
}

//----------------------------------------------------------------------------

int CPsarcFile::ReadPsarcFile( const char * filename )
{
   mFilename = filename;
   FILE * psarcFp = NULL;
   try
   {
      psarcFp = fopen( filename, "rb" );
      if( !psarcFp )
      {
         printf("Failed to open %s for reading.\n", filename );
         throw false;
      }
      fread( &mHeader, sizeof( mHeader ), 1, psarcFp );
      byteswap_psarc_header( mHeader );
      if( mHeader.mMagicNumber != skMagicNumber )
      {
         printf("Bad psarc header: %08x\n", mHeader.mMagicNumber );
         throw false;
      }
      std::vector< unsigned char > tocEntryBuf( mHeader.mTOCEntrySize, 0 );
      mTOC.reserve( mHeader.mNumFiles );
      for( int i=0; i < mHeader.mNumFiles; ++i )
      {
         STOCEntry tocEntry;
         fread( &tocEntryBuf[0], 1, mHeader.mTOCEntrySize, psarcFp );
         read_toc_entry( tocEntry, &tocEntryBuf[0], mHeader.mTOCEntrySize );
         mTOC.push_back( tocEntry );
      }
      int tocSize = ftell( psarcFp );
      int blockSizeListSize = mHeader.mTOCSize - tocSize;
      int blockEntrySize = get_block_size_entry_size( mHeader.mBlockSize );
      if( blockSizeListSize % blockEntrySize )
      {
         printf( "Error: block size list size not a multiple of block entry size (%d/%d)\n", blockSizeListSize, blockEntrySize );
         throw false;
      }
      int blockCount = blockSizeListSize / blockEntrySize;
      printf("Block count: %d\n", blockSizeListSize );
      mBlockSizes.reserve( blockCount );
      unsigned char blockEntrySizeBuf[4] = { 0 };
      for( int i=0; i < blockCount; ++i )
      {
         fread( blockEntrySizeBuf, 1, blockEntrySize, psarcFp );
         unsigned int blockSizeEntry = read_block_size_entry( blockEntrySizeBuf, blockEntrySize );
         mBlockSizes.push_back( blockSizeEntry );
      }

      fclose( psarcFp );
      psarcFp = NULL;

      int ret = LoadEntireFile( NULL, mManifestData );
      if( ret )
      {
         printf("Failed to load manifest file!\n");
         return -1;
      }

#if 0
      {
         FILE * fp = fopen( ( mFilename + ".manifest.txt" ).c_str(), "wb" );
         fwrite( &mManifestData[0], mManifestData.size(), 1, fp );
         fclose( fp );
      }
#endif
      return 0;
   }
   catch(...)
   {
      printf("psarctool failed.\n");
      if( psarcFp )
      {
         fclose( psarcFp );
      }
      return -1;
   }
}

int CPsarcFile::LoadEntireFile( const unsigned char * nameManifest, TDataBuffer & outDataBuffer ) const
{
   TDataBuffer decompressionBuffer( mHeader.mBlockSize, 0 );
   TDataBuffer compressedBlockBuffer( mHeader.mBlockSize, 0 );

   if( !nameManifest )
   {
      //Load manifest file for the psarc.
      const STOCEntry & tocEntry = mTOC[0];
      outDataBuffer.assign( (int)tocEntry.mOriginalSize, 0 );
      int blockCount = (int)( ( tocEntry.mOriginalSize + mHeader.mBlockSize - 1 ) / mHeader.mBlockSize );
      int remain = (int)tocEntry.mOriginalSize;
      unsigned char * dst = &outDataBuffer[0];
      FILE * fp = fopen( mFilename.c_str(), "rb" );
      if( !fp )
      {
         return -1;
      }
      _fseeki64( fp, tocEntry.mStartOffset, SEEK_SET );

      for( int i=0; i < blockCount; ++i )
      {
         unsigned int compressedBlockSize = mBlockSizes[tocEntry.mBlockListStart+i];
         //Read compressed block from file.
         fread( &compressedBlockBuffer[0], 1, compressedBlockSize, fp );
         //Decompress this block.
         int decompress_ret = decompress_block( &decompressionBuffer[0], &compressedBlockBuffer[0], mHeader.mBlockSize, compressedBlockSize );
         if( decompress_ret != 0 )
         {
            printf("Decompression error: block %d\n", tocEntry.mBlockListStart+i );
            fclose( fp );
            return -1;
         }

         //Write the decompressed data to the output buffer.
         int writeSize = std::min( remain, (int)mHeader.mBlockSize );
         memcpy( dst, &decompressionBuffer[0], writeSize );

         //Next compressed block offset.
         remain -= mHeader.mBlockSize;
         dst += mHeader.mBlockSize;
      }
      fclose( fp );
      return 0;
   }
   //TODO
   throw false;
}
