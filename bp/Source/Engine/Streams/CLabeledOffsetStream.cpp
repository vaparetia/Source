//------------------------------------------------------------------------------------------
// CLabeledOffsetStream.cpp
// Bluepoint
// Copyright 2008
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Streams/CLabeledOffsetStream.h"

//----------------------------------------------------------------------------

CLabeledOffsetStream::CLabeledOffsetStream( int const initialSize )
: CGrowableMemoryOutStream( initialSize )
{
}

//----------------------------------------------------------------------------

CLabeledOffsetStream::~CLabeledOffsetStream()
{
}

//----------------------------------------------------------------------------

static int GetAlignmentPosition(uint32 const pos, uint32 const alignment)
{
   int const remainder = (pos + (alignment - 1)) & (~(alignment - 1));
   return remainder;
}

//----------------------------------------------------------------------------

void CLabeledOffsetStream::AddAlignmentPadding(uint32 const alignment, uint8 const alignmentValue)
{
   int const aligmentPosition = GetAlignmentPosition(GetDataSize(), alignment);
   while (GetDataSize() < aligmentPosition)
   {
      WriteUint8(alignmentValue);
   }
}

//----------------------------------------------------------------------------

void CLabeledOffsetStream::AddLabel(std::string const &label)
{
   mLabels[label] = GetDataSize();
}

//----------------------------------------------------------------------------

void CLabeledOffsetStream::AddAlignedLabel(std::string const &label, uint32 const alignment)
{
   int const alignedPosition = GetAlignmentPosition(GetDataSize(), alignment);
   mLabels[label] = alignedPosition;
}

//----------------------------------------------------------------------------

void CLabeledOffsetStream::AddOffset(SLabledOffset::EOffsetType const type, std::string const &label, std::string const &relativeLabel)
{
   SLabledOffset offset;
   offset.mType = type;
   offset.mOffsetPosition = GetDataSize();
   offset.mLabel = label;
   offset.mRelativeLabel = relativeLabel;
   if (relativeLabel.empty())
   {
      offset.mRelativeOffsetPosition = GetDataSize();
   }
   else
   {
      offset.mRelativeOffsetPosition = -1;
   }

   mOffsets.push_back(offset);
}

//----------------------------------------------------------------------------

void CLabeledOffsetStream::AddOffset16(std::string const &label, std::string const &relativeLabel)
{
   AddOffset(SLabledOffset::kOT_16, label, relativeLabel);
   // Write placeholder value so we can catch unpatched offsets
   WriteUint16(0xFEED);
}

//----------------------------------------------------------------------------

void CLabeledOffsetStream::AddOffset32(std::string const &label, std::string const &relativeLabel)
{
   AddOffset(SLabledOffset::kOT_32, label, relativeLabel);
   // Write placeholder value so we can catch unpatched offsets
   WriteUint32(0xFEEDBABE);
}

//----------------------------------------------------------------------------

void CLabeledOffsetStream::PatchOffsets(void)
{
   foreach(SLabledOffset &offset, mOffsets)
   {
      TLabelMap::const_iterator foundLabelIt = mLabels.find(offset.mLabel);
      if (foundLabelIt == mLabels.end())
      {
         // Can't find label, overwrite patch as zero offset.
         uint8 * pOffsetPtr = ((uint8*)Data()) + offset.mOffsetPosition;
         switch (offset.mType)
         {
         case SLabledOffset::kOT_16:
            {
               *((uint16*) pOffsetPtr) = 0;
            }
            break;
         case SLabledOffset::kOT_32:
            *((int*) pOffsetPtr) = 0;
            break;
         }
         continue;
      }
      int relativeOffsetPosition = offset.mRelativeOffsetPosition;
      TLabelMap::const_iterator foundLabelRelativeIt = mLabels.find(offset.mRelativeLabel);
      if (foundLabelRelativeIt != mLabels.end())
      {
         relativeOffsetPosition = foundLabelRelativeIt->second;
      }
      if (relativeOffsetPosition >= 0)
      {
         int const positionOffset = foundLabelIt->second - relativeOffsetPosition;
         // Set offset
         uint8 * pOffsetPtr = ((uint8*)Data()) + offset.mOffsetPosition;

         switch (offset.mType)
         {
         case SLabledOffset::kOT_16:
            {
               uint16 const positionOffset16 = (uint16) positionOffset;
               *((uint16*) pOffsetPtr) = positionOffset16;
               if (GetEndian() == kOE_BigEndian)
               {
                  // Only valid under Win32, no effect on other platforms
                  NEndian::Swap2Bytes(pOffsetPtr);
               }
            }
            break;
         case SLabledOffset::kOT_32:
            *((int*) pOffsetPtr) = positionOffset;
            if (GetEndian() == kOE_BigEndian)
            {
               // Only valid under Win32, no effect on other platforms
               NEndian::Swap4Bytes(pOffsetPtr);
            }
            break;
         }
      }
   }
   mOffsets.clear();
}

//----------------------------------------------------------------------------

int CLabeledOffsetStream::GetLabelOffset(std::string const &label)
{
   TLabelMap::const_iterator it = mLabels.find(label);
   if (it != mLabels.end())
   {
      return it->second;
   }

   // Couldn't find label
   return -1;
}

//----------------------------------------------------------------------------


