//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Streams/CLabeledOffsetStream.h"
#include "Engine/Streams/CDiskOutputStream.h"

#include "gcroot.h"

//----------------------------------------------------------------------------

#include "CStreamWriter_MergeSOL.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;

//----------------------------------------------------------------------------

namespace
{
   static void get_jp_sol_source_filename( char * outFullPath )
   {
      const char * const regionSrcFolder = "jp_sol";

      if( const char * const bpe_repository = getenv( "BPE_REPOSITORY" ) )
      {
         //This is hardcodec to merge us/ and jp_sol/ streams into jp/ streams.
         //Look for the corresponding SOL/JP stream from which we'll copy and insert packets as appropriate.
         const char * usRegion = strstr( gCurrInputFilename, "us/" );
         if( usRegion != NULL && ( usRegion == gCurrInputFilename || (*(usRegion-1)) == '/' ) )
         {
            const char * subfolder = usRegion+3;
            //Only looking for these particular streams.
            //Also work around single layer SOL problem.  These streams were distributed into layer 1 and 2 in Substance.
            if( !strncmp( subfolder, "vox",3 ) || !strncmp( subfolder, "vox2",4 ) )
            {
               subfolder = "vox";
            }
            else if( !strncmp( subfolder, "demo",4 ) || !strncmp( subfolder, "demo2",5 ) )
            {
               subfolder = "demo";
            }
            else if( !strncmp( subfolder, "movie",5 ) || !strncmp( subfolder, "movievr", 7 ) )
            {
               subfolder = "movie";
            }
            else
            {
               printf("Error: unhandled merge stream .dat file %s!\n", subfolder );
               throw false;
            }

            sprintf( outFullPath, "%s/%s/%s/%s.sdt", bpe_repository, regionSrcFolder, subfolder, gCurrStreamName );
         }
         else
         {
            printf("Error: can only perform a SOL merge with US source data!\n");
            throw false;
         }
      }
      else
      {
         printf("Error: I need BPE_REPOSITORY to find jp_sol source stream file!\n");
      }
   }
}

//----------------------------------------------------------------------------

CStreamWriter_MergeSOL::CStreamWriter_MergeSOL()
:  mpSourceFile( NULL )
,  mWritingState( kWritingState_None )
{
   memset( &mCurrentSourceStreamTag, 0, sizeof(mCurrentSourceStreamTag) );
}

//----------------------------------------------------------------------------

bool CStreamWriter_MergeSOL::ReplacementFileExistsForCurrStream(EPlatform /*curPlatform*/) const
{
   char sourceFilename[FILENAME_MAX];
   get_jp_sol_source_filename( sourceFilename );
   //If file does not exist, then there is no jp_sol source stream.
   //VR / Snake Tales do not exist in jp_sol data, for example.
   //We wind up with unmerged US data in this case.
   return BP_FileExists( sourceFilename );
}

//----------------------------------------------------------------------------

void CStreamWriter_MergeSOL::ModifyCurrentSourcePacket( STREAM_TAG & streamTag, unsigned char * const packetBuf, EPlatform /*curPlatform*/ )
{
   mCurrentSourceStreamTag = streamTag;
}

//----------------------------------------------------------------------------

void CStreamWriter_MergeSOL::WriteOnePacket( FILE * const streamfp, EPlatform /*curPlatform*/, const int currTick )
{
   if( mWritingState != kWritingState_Finished )
   {
      //There are demo packets in this stream.
      if( !mpSourceFile )
      {
         //Haven't yet opened up my source file.
         char sourceFilename[FILENAME_MAX];
         get_jp_sol_source_filename( sourceFilename );
         mpSourceFile = fopen( sourceFilename, "rb" );
         if( !mpSourceFile )
         {
            printf("Error: should have checked existence earlier!\n");
            throw false;
         }

         //Initialize list of packets we need to merge in from JP_SOL data.
         STREAM_TAG streamTag;
         fread( &streamTag, sizeof( streamTag ), 1, mpSourceFile );

         //We're going to assume that the corresponding streams have the same types
         //of packets.  So we'll leave the US system packets in place and only replace
         //the typed packets after this section.
         bool bCodecStream = false;
         do 
         {
            if( ( streamTag._option & 0xffff ) == CHUNK_TYPE_CODEC_CAP )
            {
               bCodecStream = true;
            }

            fseek( mpSourceFile, streamTag._size - sizeof( streamTag ), SEEK_CUR );
            fread( &streamTag, sizeof( streamTag ), 1, mpSourceFile );

         } while ( streamTag._type == CHUNK_TYPE_SYSTEM );

         fseek( mpSourceFile, -(int)(sizeof( streamTag )), SEEK_CUR );

         //We are now at the first packet after the initial system packets.
         if( bCodecStream )
         {
            //For Codecs, we take audio / Japanese codec_cap / motion.
            mMergePacketTypes.push_back( CHUNK_TYPE_PCM );
            mMergePacketTypes.push_back( CHUNK_TYPE_CODEC_CAP | (GM_LANG_DEFAULT<<16) );
            mMergePacketTypes.push_back( CHUNK_TYPE_CODEC_CAP | (GM_LANG_JAPANESE<<16) );
            mMergePacketTypes.push_back( CHUNK_TYPE_MOTION );
         }
         else
         {
            //For all others, we take audio / Japanese caption.
            mMergePacketTypes.push_back( CHUNK_TYPE_PCM );
            mMergePacketTypes.push_back( CHUNK_TYPE_CAPTION | (GM_LANG_DEFAULT<<16) );
            mMergePacketTypes.push_back( CHUNK_TYPE_CAPTION | (GM_LANG_JAPANESE<<16) );
         }
      }

      DoWriteOnePacket( streamfp, currTick );
   }
   else
   {
      //nothing for me to write to this stream.
   }
}

//----------------------------------------------------------------------------

void CStreamWriter_MergeSOL::EndStream(FILE * const streamfp, EPlatform /*curPlatform*/, STREAM_TAG*)
{
   while( mpSourceFile )
   {
      //Write any remainder packets.
      DoWriteOnePacket( streamfp, INT_MAX );
   }
   mWritingState = kWritingState_None;
   mMergePacketTypes.clear();
}

//----------------------------------------------------------------------------

bool CStreamWriter_MergeSOL::ReplacesDataForCurrStream( const unsigned int type )
{
   //Mask off the subtype unless we're considering a caption or codec_cap packet.
   unsigned int checkType = type;
   const unsigned int baseType = (type & 0xffff); 
   if( !(baseType==CHUNK_TYPE_CAPTION || baseType==CHUNK_TYPE_CODEC_CAP) )
   {
      checkType &= 0xffff; 
   }

   if( std::find( mMergePacketTypes.begin(), mMergePacketTypes.end(), checkType ) != mMergePacketTypes.end() )
   {
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------

unsigned int CStreamWriter_MergeSOL::GetRebuildPlatformMask() const
{
   //unused.
   return kPlatformMask_Base;
}

//----------------------------------------------------------------------------

void CStreamWriter_MergeSOL::DoWriteOnePacket( FILE * const streamfp, const int currTick )
{
   while( mpSourceFile )
   {
      STREAM_TAG streamTag;
      if( 1 != fread( &streamTag, sizeof( streamTag ), 1, mpSourceFile ) )
      {
         printf("Error: CStreamWriter_MergeSOL trying to read source packets\n");
         throw false;
      }

      if( streamTag._type == CHUNK_TYPE_END )
      {
         //End of stream!  We're done.
         fclose( mpSourceFile );
         mpSourceFile = NULL;
         mWritingState = kWritingState_Finished;
      }
      else if( ReplacesDataForCurrStream( streamTag._type ) )
      {
         if( streamTag._time > currTick || mCurrentSourceStreamTag._type == CHUNK_TYPE_SYSTEM )
         {
            //Stop at this packet, we don't want to insert it yet.
            //Wait until all the system packets have been written as well.
            fseek( mpSourceFile, -(int)(sizeof( streamTag )), SEEK_CUR );
            break;
         }
         else
         {
            //Insert this packet.

            if( streamTag._type == CHUNK_TYPE_CAPTION || streamTag._type == CHUNK_TYPE_CODEC_CAP )
            {
               //N.B. if it's a caption packet, then we must insert a subtype of 7 for Substance's benefit
               //as the JP_SOL data did not have a subtype.
               streamTag._type |= (GM_LANG_JAPANESE << 16);
            }

            std::vector< unsigned char > packetBuf( streamTag._size, 0 );
            memcpy( &packetBuf[0], &streamTag, sizeof( streamTag ) );
            if( int bodySize = streamTag._size - sizeof( streamTag ) )
            {
               fread( &packetBuf[sizeof(streamTag)], sizeof(unsigned char), bodySize, mpSourceFile );
            }
            fwrite( &packetBuf[0], packetBuf.size(), 1, streamfp );
         }
      }
      else
      {
         //seek over all other source packets.
         fseek( mpSourceFile, streamTag._size - sizeof( streamTag ), SEEK_CUR );
      }
   }
}
