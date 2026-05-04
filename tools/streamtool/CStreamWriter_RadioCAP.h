//----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <map>

#include "CStreamWriter.h"

//----------------------------------------------------------------------------

class CStreamWriter_RadioCAP : public CStreamWriter
{
public:
   CStreamWriter_RadioCAP();

   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick);

   virtual void ModifyCurrentSourcePacket( STREAM_TAG & streamTag, unsigned char * const packetBuf, EPlatform curPlatform );

   virtual void EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag);
   virtual bool ReplacesDataForCurrStream( const unsigned int type );
   virtual unsigned int GetRebuildPlatformMask() const;

private:

   void BeginCppOutputFile( const EPlatform platform );
   void EndCppOutputFile();

private:

   struct SRadioCaptionOverride
   {
      int   mStart;
      int   mEnd;
   };

   //One for each platform in this vector.
   typedef std::vector< SRadioCaptionOverride > TRadioCaptionOverrideVector;
   //Unique key by combining start count and language.
   typedef std::pair< int, int >                TRadioCaptionStartCountLanguagePair;

   typedef std::map< TRadioCaptionStartCountLanguagePair, TRadioCaptionOverrideVector >  TRadioCaptionMap;

private:
   bool mbInStream;

   TRadioCaptionMap mCaptions;
   FILE *            mpCppOutputFile;
};

//----------------------------------------------------------------------------
