//------------------------------------------------------------------------------------------
// CLabeledOffsetStream.h
// Bluepoint
// Copyright 2008
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"

//----------------------------------------------------------------------------

class ENGINE_API CLabeledOffsetStream : public CGrowableMemoryOutStream
{
public:
   explicit CLabeledOffsetStream(int const initialSize = 4096);
   virtual ~CLabeledOffsetStream();

   void  AddAlignmentPadding(uint32 const alignment, uint8 const alignmentValue = 0xff);

   // Sets label marker at current position, offset will be fixed up to point to this.
   void  AddLabel(std::string const &label);
   // Sets a label marker padded to the alignment value
   void  AddAlignedLabel(std::string const &label, uint32 const alignment);

   // Offsets to label. Without a relativeLabel, the offset will be relative to current position.
   void  AddOffset16(std::string const &label, std::string const &relativeLabel = std::string(""));
   void  AddOffset32(std::string const &label, std::string const &relativeLabel = std::string(""));

   // Fixes up offsets to match labels specified by 'AddLabel'.
   void  PatchOffsets(void);
   int   GetLabelOffset(std::string const &label);

private:
   struct SLabledOffset
   {
      enum EOffsetType
      {
         kOT_16,
         kOT_32,
         kOT_Count
      };
        
      EOffsetType    mType;
      int            mOffsetPosition;
      int            mRelativeOffsetPosition;
      std::string    mLabel;
      std::string    mRelativeLabel;
   };

   void  AddOffset(SLabledOffset::EOffsetType const type, std::string const &label, std::string const &relativeLabel);

   typedef std::map<std::string, int>  TLabelMap;
   
   std::list<SLabledOffset>   mOffsets;
   TLabelMap                  mLabels;
};

//----------------------------------------------------------------------------

