//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "Engine/Basics/CStringExtras.h"

#include "gcroot.h"

//----------------------------------------------------------------------------

#include "CStreamWriter_RadioCAP.h"

//----------------------------------------------------------------------------
//For code patching solution:
//This struct is written out to a .cpp file built into the game.

/*
typedef struct _SBPRadioCapOverride
{
const char * const   mStreamName;
int                  mLanguage;
int                  mOrgBegin;
int                  mOrgEnd;
int                  mNewBegin;
int                  mNewEnd;
} SBPRadioCapOverride;
*/

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;

//----------------------------------------------------------------------------

namespace
{
   const char * const skPlatformXPathLabels[] =
   {
      "./_bp",
      "./_ps3",
      "./_360",
      "./_vta"
   };

   const char* GetCurrentCaptionName()
   {
      return CStringExtras::Stringize_s("%s/%s/%s.radio_cap", gOutputFolder, gCurrStreamName, gCurrStreamName);
   }
}

BPE_CTASSERT( BPE_ARRAY_SIZE( skPlatformXPathLabels ) == kPlatform_Count );

CStreamWriter_RadioCAP::CStreamWriter_RadioCAP()
: mbInStream(false)
, mCaptions()
, mpCppOutputFile( NULL )
{
}

//----------------------------------------------------------------------------
bool CStreamWriter_RadioCAP::ReplacementFileExistsForCurrStream(EPlatform curPlatform) const
{
   (void)curPlatform;
   return BP_FileExists( GetCurrentCaptionName() );
}

//----------------------------------------------------------------------------

struct SRadioCaption
{
   int start_count;
   int end_count;
   int unused_name;
#if MGS_VERSION==2
   int len;
#else
   short len;
   short lang;
#endif
};

//----------------------------------------------------------------------------

void CStreamWriter_RadioCAP::ModifyCurrentSourcePacket( STREAM_TAG & streamTag, unsigned char * const packetBuf, EPlatform curPlatform )
{
   if( !mbInStream )
   {
      //First source packet of the stream.
      //Load captions if they exist.
      if( ReplacementFileExistsForCurrStream(kPlatform_Base) )
      {
         XmlDocument^ doc = gcnew XmlDocument();
         String^ inputPath = gcnew String(GetCurrentCaptionName());
         doc->Load(inputPath);

         // Write captions
         for each( XmlElement^ caption in doc->SelectNodes("//Caption") )
         {
            int const start = Int32::Parse(caption->GetAttribute("start"));
            int const end = Int32::Parse(caption->GetAttribute("end"));
            int const language = Int32::Parse(caption->GetAttribute("lang"));

            //Default initialize all platform overrides with source values.
            SRadioCaptionOverride defaultOverride = { start, end };
            TRadioCaptionOverrideVector captionOverrides;
            captionOverrides.assign( kPlatform_Count, defaultOverride );

            //Read any explicit overrides from the xml elements.
            for( int i=0; i < kPlatform_Count; ++i )
            {
               const char * const platformLabel = skPlatformXPathLabels[i];
               XmlElement^ captionOverrideElement = (XmlElement^)caption->SelectSingleNode( gcnew System::String( platformLabel ) );
               if( captionOverrideElement != nullptr )
               {
                  captionOverrides[i].mStart = Int32::Parse(captionOverrideElement->GetAttribute("start"));
                  captionOverrides[i].mEnd = Int32::Parse(captionOverrideElement->GetAttribute("end"));
                  if( i==kPlatform_Base )
                  {
                     //Make sure that base override is applied to all platforms by default.
                     for( int j=0; j < kPlatform_Count; ++j )
                     {
                        captionOverrides[j].mStart = captionOverrides[i].mStart;
                        captionOverrides[j].mEnd = captionOverrides[i].mEnd;
                     }
                  }
               }
            }

            //Add this override to the map.  Keyed by pair of language and start tick (together, unique).
            TRadioCaptionStartCountLanguagePair pr;
            pr.first = start;
            pr.second = language;

            mCaptions[pr] = captionOverrides;
         }

         BeginCppOutputFile( curPlatform );
      }

      mbInStream = true;
   }

   if( mbInStream && ( ( streamTag._type & 0xffff ) == STREAM_TYPE_CODEC_CAP ) && mCaptions.size() )
   {
      //Need to override the start and end timing for all captions in this packet.
      char* pCurrentData = (char*)packetBuf + sizeof(streamTag);

      int const captionSize = *((int*)pCurrentData);
      pCurrentData += 4;

      char* caption_end = pCurrentData + captionSize;

      while( pCurrentData != caption_end )
      {
         SRadioCaption * pCurrentCaption = (SRadioCaption*)(pCurrentData);
#if MGS_VERSION==2
         short lang = streamTag._type >> 16;
#elif MGS_VERSION==3
         short lang = pCurrentCaption->lang;
#endif
         TRadioCaptionStartCountLanguagePair pr;
         pr.first = pCurrentCaption->start_count;
         pr.second = lang;

         TRadioCaptionMap::const_iterator found = mCaptions.find( pr );
         if( found == mCaptions.end() )
         {
            printf("Error: caption override not found for (%d,%d).  corrupted radio_cap file?\n", pr.first, pr.second );
            pCurrentData = pCurrentData + pCurrentCaption->len;
            continue;
            //throw false;
         }

         //Do the override.
         const TRadioCaptionOverrideVector & captionOverrides = found->second;

         if( pCurrentCaption->start_count != captionOverrides[curPlatform].mStart || pCurrentCaption->end_count != captionOverrides[curPlatform].mEnd )
         {
            fprintf( mpCppOutputFile, "   { \"%s\", %d, %6d, %6d, %6d, %6d },\n", gCurrStreamName, lang, pCurrentCaption->start_count, pCurrentCaption->end_count, captionOverrides[curPlatform].mStart, captionOverrides[curPlatform].mEnd );
         }

         pCurrentCaption->start_count = captionOverrides[curPlatform].mStart;
         pCurrentCaption->end_count = captionOverrides[curPlatform].mEnd;

         pCurrentData = pCurrentData + pCurrentCaption->len;
      }
   }
}

void CStreamWriter_RadioCAP::WriteOnePacket( FILE * const /*streamfp*/, EPlatform /*curPlatform*/, const int /*currTick */)
{
}

//----------------------------------------------------------------------------

void CStreamWriter_RadioCAP::EndStream(FILE * const /*streamfp*/, EPlatform /*curPlatform*/, STREAM_TAG*)
{
   mbInStream = false;
   mCaptions.clear();
   EndCppOutputFile();
}

//----------------------------------------------------------------------------

bool CStreamWriter_RadioCAP::ReplacesDataForCurrStream( const unsigned int type )
{
   return false;
}

//----------------------------------------------------------------------------

unsigned int CStreamWriter_RadioCAP::GetRebuildPlatformMask() const
{
   return kPlatformMask_Base;
}


//----------------------------------------------------------------------------

void CStreamWriter_RadioCAP::BeginCppOutputFile( const EPlatform platform )
{
   char fullPath[FILENAME_MAX];
   const char * const platformLabel = skPlatformXPathLabels[platform] + 2;
   sprintf(fullPath,"%s/%s/%s.radio_cap%s.cpp", gOutputFolder, gCurrStreamName, gCurrStreamName, platformLabel);
   mpCppOutputFile = fopen( fullPath, "wt" );
   if( !mpCppOutputFile )
   {
      printf("Error: could not open %s for writing.\n", fullPath );
      throw false;
   }

   //header text
   fprintf( mpCppOutputFile, "static const SBPRadioCapOverride skRadioCapOverride_%s_%s[] =\n{\n", gCurrStreamName, gRegionString ); 
}

//----------------------------------------------------------------------------

void CStreamWriter_RadioCAP::EndCppOutputFile()
{
   if( mpCppOutputFile )
   {
      fprintf( mpCppOutputFile, "};\n\n" );
      fclose( mpCppOutputFile );
      mpCppOutputFile = NULL;
   }
}
