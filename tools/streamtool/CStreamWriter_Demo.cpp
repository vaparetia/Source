//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Streams/CLabeledOffsetStream.h"
#include "Engine/Streams/CDiskOutputStream.h"

#include "gcroot.h"

//----------------------------------------------------------------------------

#include "CStreamWriter_Demo.h"

//----------------------------------------------------------------------------

namespace
{
   static void get_50hz_source_filename( char * outFullPath )
   {
      const char * const regionSrcFolder =
#if MGS_VERSION==2
         "eu";
#else
         //N.B. all demo packets are the same across fr/gr/it/sp SKU in MGS3.
         //We've got the FR ones checked into perforce so will use them.
         "fr";
#endif
      if( const char * const bpe_repository = getenv( "BPE_REPOSITORY" ) )
      {
         //In both games, we just use US data for demo streams in both US and EU[/FR/GR/IT/SP].
         //Look for the corresponding EU demo stream from which we'll copy and insert 50hz demo packets.
         const char * usRegion = strstr( gOutputFolder, "us/" );
         if( usRegion != NULL && ( usRegion == gOutputFolder || (*(usRegion-1)) == '/' ) )
         {
            sprintf( outFullPath, "%s/%s/%s/%s.sdt", bpe_repository, regionSrcFolder, usRegion+3, gCurrStreamName );
         }
         else
         {
            //No 50hz source to be found. (presumably a JP demo stream, don't need any 50hz packets here.)
            *outFullPath = 0;
         }
      }
      else
      {
         printf("Error: I need BPE_REPOSITORY to find 50hz source stream file!\n");
      }
   }
}

//----------------------------------------------------------------------------

CStreamWriter_Demo::CStreamWriter_Demo()
: mpSourceFile( NULL )
, mWritingState( kWritingState_None )
{
}

//----------------------------------------------------------------------------

bool CStreamWriter_Demo::ReplacementFileExistsForCurrStream(EPlatform /*curPlatform*/) const
{
   char sourceFilename[FILENAME_MAX];
   get_50hz_source_filename( sourceFilename );
   if( FILE * fp = fopen( sourceFilename, "rb" ) )
   {
      //There is a region-specific file for this stream.  See if it's got any demo packets in it.
      STREAM_TAG streamTag;
      bool bDemoPacketsInStream = false;
      fread( &streamTag, sizeof( streamTag ), 1, fp );

      do 
      {
         if( ( streamTag._option & 0xffff ) == CHUNK_TYPE_DEMO )
         {
            bDemoPacketsInStream = true;
            break;
         }

         fseek( fp, streamTag._size - sizeof( streamTag ), SEEK_CUR );
         fread( &streamTag, sizeof( streamTag ), 1, fp );

      } while ( streamTag._type != CHUNK_TYPE_SYSTEM );
      fclose( fp );

      return bDemoPacketsInStream;
   }
   else
   {
      //If there's no stream in this path checked in to source control, we can assume
      //that it does not have any demo packets in it.
      return false;
   }
}

//----------------------------------------------------------------------------

void CStreamWriter_Demo::WriteOnePacket( FILE * const streamfp, EPlatform curPlatform, const int currTick )
{
   if( mWritingState != kWritingState_Finished )
   {
      //There are demo packets in this stream.
      if( !mpSourceFile )
      {
         //Haven't yet opened up my source file.
         char sourceFilename[FILENAME_MAX];
         get_50hz_source_filename( sourceFilename );
         mpSourceFile = fopen( sourceFilename, "rb" );
         if( !mpSourceFile )
         {
            //don't try to open the file again.
            mWritingState = kWritingState_Finished;
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

void CStreamWriter_Demo::ModifyCurrentSourcePacket( STREAM_TAG & streamTag, unsigned char * const /*packetBuf*/, EPlatform /*curPlatform*/ )
{
   switch( streamTag._type & 0xffff )
   {
   case CHUNK_TYPE_SYSTEM:
      {
         //Need to add the 60hz packet type identifier to the system packets for demo packets.
         if( (streamTag._option & 0xffff) == CHUNK_TYPE_DEMO )
         {
            int demoLayer = streamTag._option >> 16;
            if( demoLayer > 255 )
            {
               printf("TODO: demoLayer > 1 byte means we need to figure out another way to add 50/60hz identifiers!\n");
               throw false;
            }
            streamTag._option |= BP_STREAM_SUBTYPE_DEMO_60;
            //Try to write data to this stream.
            mWritingState = kWritingState_Pending;
         }
      }
      break;
   case CHUNK_TYPE_DEMO:
      {
         //Need to add the 60hz packet type identifier to the demo packets themselves.
         streamTag._type |= BP_STREAM_SUBTYPE_DEMO_60;
         if( (streamTag._time % 5) != 0 )
         {
//            printf("Warning: unexpected timing (%d) of 60hz demo packet! (all should be multiples of 5 ticks)\n", streamTag._time);
         }
      }
      break;
   }
}

//----------------------------------------------------------------------------

void CStreamWriter_Demo::EndStream(FILE * const streamfp)
{
   while( mpSourceFile )
   {
      //Write any remainder packets.
      DoWriteOnePacket( streamfp, INT_MAX );
   }
   mWritingState = kWritingState_Finished;
}

//----------------------------------------------------------------------------

bool CStreamWriter_Demo::ReplacesDataForCurrStream( const unsigned int type )
{
   //N.B. we let the original 60hz demo packets through, we just add information to their type in ModifyCurrentSourcePacket()
   return false;
}

//----------------------------------------------------------------------------

unsigned int CStreamWriter_Demo::GetRebuildPlatformMask() const
{
   return kPlatformMask_Base;
}

//----------------------------------------------------------------------------

void CStreamWriter_Demo::DoWriteOnePacket( FILE * const streamfp, const int currTick )
{
   while( mpSourceFile )
   {
      STREAM_TAG streamTag;
      if( 1 != fread( &streamTag, sizeof( streamTag ), 1, mpSourceFile ) )
      {
         printf("Error: CStreamWriter_Demo trying to read source packets\n");
         throw false;
      }

      if( streamTag._type == CHUNK_TYPE_END )
      {
         //End of stream!  We're done.
         fclose( mpSourceFile );
         mpSourceFile = NULL;
         mWritingState = kWritingState_Finished;
      }
      else if( streamTag._type == CHUNK_TYPE_SYSTEM )
      {
         //handle system packets for the demo type.
         if( ( streamTag._option & 0xffff ) == CHUNK_TYPE_DEMO )
         {
            //Update the subtype to indicate a 50hz packet and write it out.
            streamTag._option |= BP_STREAM_SUBTYPE_DEMO_50;
            fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );
         }
         if( streamTag._size != sizeof( streamTag ) )
         {
            //system packets are all size of 16.
            printf("TODO: non-16 size system packet???\n");
            throw false;
         }
      }
      else if( ( streamTag._type & 0xffff ) == CHUNK_TYPE_DEMO )
      {
         //If it's a demo packet, check the timestamp and write out any whose time has come.
         //Stop when we hit a demo packet that's too far in the future.
         if( ( streamTag._time % 6 ) != 0 )
         {
            //BP_INVESTIGATE: why do a couple of (seemingly actually used) PAL streams in MGS3 have 60hz demo packets??
//            printf("Warning: unexpected timing (%d) of PAL demo packet! (all should be multiples of 6 ticks)\n", streamTag._time);
         }

         if( streamTag._time > currTick )
         {
            //Stop at this packet, we don't want to insert it yet.
            fseek( mpSourceFile, -(int)(sizeof( streamTag )), SEEK_CUR );
            break;
         }
         else
         {
            //Add our 50hz tag and insert this packet.
            streamTag._type |= BP_STREAM_SUBTYPE_DEMO_50;
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
