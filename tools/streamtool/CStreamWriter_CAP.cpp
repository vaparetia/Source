//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Streams/CLabeledOffsetStream.h"
#include "Engine/Streams/CDiskOutputStream.h"

#include "gcroot.h"

//----------------------------------------------------------------------------

#include "CStreamWriter_CAP.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;

//----------------------------------------------------------------------------

namespace
{
   const char* GetCurrentCaptionName()
   {
      return CStringExtras::Stringize_s("%s/%s/%s.cap", gOutputFolder, gCurrStreamName, gCurrStreamName);
   }

   gcroot<TextToolData::Backend^> gTextBackend;
   gcroot<TextToolData::StringDB^> gTextStringDB;
}





CStreamWriter_CAP::CStreamWriter_CAP( const bool bDumpText, const int dumpLanguage )
:  didWriteData(false)
,  mbDumpText( bDumpText )
,  mDumpLanguage( dumpLanguage )
{
}

//----------------------------------------------------------------------------
bool CStreamWriter_CAP::ReplacementFileExistsForCurrStream(EPlatform curPlatform) const
{
   (void)curPlatform;
   return BP_FileExists( GetCurrentCaptionName() );
}

//----------------------------------------------------------------------------

struct SCaption
{
   int start_count;
   int end_count;
   int name;

   short len;
   short lang;
   char string[0];
};


struct SAction
{
   int start;
   int name;
   int val;
   int option_plus_lang;

   bool operator()(SAction const &a, SAction const &b)
   {
      return a.start < b.start;
   }
   void Sort(std::vector<SAction> &vec)
   {
      std::sort(vec.begin(), vec.end(), *this);
   }
};

void CStreamWriter_CAP::WriteOnePacket( FILE * const streamfp, EPlatform curPlatform, const int currTick )
{
   if( !didWriteData )
   {
      if( ReplacementFileExistsForCurrStream(curPlatform) )
      {
         XmlDocument^ doc = gcnew XmlDocument();
         String^ inputPath = gcnew String(GetCurrentCaptionName());
         String^ inputFilename = IO::Path::GetFileName( inputPath );
         doc->Load(inputPath);

         if( !gTextBackend )
         {
            gTextBackend = gcnew TextToolData::Backend();
         }

         if( !gTextStringDB )
         {
            gTextStringDB = gcnew TextToolData::StringDB(gTextBackend);
         }

         IO::StreamWriter^ textDumpStreamWriter = nullptr;

         if( mbDumpText )
         {
            String^ dumpPath = IO::Path::ChangeExtension(inputPath, "csv");
            textDumpStreamWriter = IO::File::CreateText( dumpPath );
         }

         CLabeledOffsetStream packetStream;
         packetStream.SetEndian(COutputStream::kOE_LittleEndian);
         
         // Write header for new BP caption packet
         packetStream.WriteUint32('BCAP');

         // store offset to font data
         packetStream.AddOffset32("FontData", "Start");

         packetStream.AddLabel("Start");

         int currentCaptionIdx  = 0;

         bool bFirstString = true;

         // Write captions
         for each( XmlElement^ caption in doc->SelectNodes("//Caption") )
         {
            std::string captionStartMarker = CStringExtras::Stringize("Caption%d", currentCaptionIdx);
            std::string captionEndMarker = CStringExtras::Stringize("CaptionEnd%d", currentCaptionIdx);
            
            currentCaptionIdx++;

            packetStream.AddLabel(captionStartMarker);

            int const start = Int32::Parse(caption->GetAttribute("start"));
            int const end = Int32::Parse(caption->GetAttribute("end"));
            int const name = Int32::Parse(caption->GetAttribute("name"));
            int const language = Int32::Parse(caption->GetAttribute("lang"));

            // Write Start Tick
            packetStream.WriteInt32(start);
            
            // Write End Tick
            packetStream.WriteInt32(end);

            // Write "name"
            packetStream.WriteInt32(name);

            // write length of caption
            packetStream.AddOffset16(captionEndMarker, captionStartMarker);

            // write language
            packetStream.WriteUint16((uint16)language);

            // Write string data
            {
               List<int>^ problemCodes = gcnew List<int>();

               XmlElement^ directTextElement = (XmlElement^)caption->SelectSingleNode( "./DirectReplacementText" );
               String ^ decodedString;
               if( directTextElement != nullptr )
               {
                  decodedString = directTextElement->InnerText;
               }
               else
               {
                  decodedString = gTextBackend->DecodeString(caption, nullptr, problemCodes, nullptr);
               }

               if( problemCodes->Count > 0 )
                  Console::WriteLine("ERROR: Unknown characters converting caption {0}", inputPath);

               String^ remappedString = gTextStringDB->ApplyOriginalCodeOverrides(decodedString);
               remappedString = gTextStringDB->ApplyKeywordReplacement(remappedString);

               array<Byte>^ utf8Data = gTextBackend->ConvertStringToUTF8(remappedString);

               for( int i = 0; i < utf8Data->Length; ++i )
                  packetStream.WriteUint8(utf8Data[i]);

               packetStream.WriteUint8(0);

               if( mbDumpText && language == mDumpLanguage )
               {
                  textDumpStreamWriter->WriteLine( "\"{0}\"\t\"{1}\"", bFirstString ? inputFilename : gcnew System::String(""), decodedString );
                  bFirstString = false;
               }
            }

            // Write end of caption marker
            packetStream.AddLabel(captionEndMarker);
         }

         // Sort the actions
         std::vector<SAction> actions;
         for each (XmlElement ^action in doc->SelectNodes("//Action"))
         {
            SAction act;
            act.start = Int32::Parse(action->GetAttribute("start"));
            act.name = Int32::Parse(action->GetAttribute("name"));
            act.val = Int32::Parse(action->GetAttribute("value"));
            act.option_plus_lang = Int32::Parse(action->GetAttribute("option")) & 0x00ffffff;
            act.option_plus_lang |= Int32::Parse(action->GetAttribute("lang")) << 24;

            actions.push_back(act);
         }
         SAction sorter;
         sorter.Sort(actions);

         // Write the action data.
         unsigned int actionOffset = 0;
         for (std::vector<SAction>::iterator it = actions.begin(); it != actions.end(); ++it)
         {
            // write out the start count.
            packetStream.WriteInt32(it->start);

            // write out the "name".
            packetStream.WriteInt32(it->name);

            // write out the value.
            packetStream.WriteInt32(it->val);

            // write out the option/lang.
            packetStream.WriteInt32(it->option_plus_lang);

            // calculate the size of the action data.
            actionOffset += 4 * sizeof( int );
         }

         // Write "font data"
         {
            packetStream.AddLabel("FontData");

            //NOTE: There is none!
            // We simply pad this file out to be 16 bytes aligned!
            packetStream.AddAlignmentPadding(16, 0x00);
         }

         STREAM_TAG streamTag;

         //Write a stream driver startup packet.
         streamTag._type = CHUNK_TYPE_SYSTEM;
         streamTag._size = sizeof( streamTag );
         streamTag._time = 0;
         streamTag._option = STREAM_TYPE_CAP;

         fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );

         //Write a header packet.
         packetStream.PatchOffsets();
         if( packetStream.GetDataSize() )
         {
            streamTag._type = STREAM_TYPE_CAP;
            streamTag._size = sizeof( streamTag ) + packetStream.GetDataSize();
            streamTag._time = 0;
            streamTag._option = actionOffset; //OFFSET for special case packet stuff!!!!

            fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );
            fwrite( packetStream.GetData(), packetStream.GetDataSize(), 1, streamfp );
         }

         if( mbDumpText )
         {
            textDumpStreamWriter->Close();
         }
      }

      didWriteData = true;
   }
}

//----------------------------------------------------------------------------

void CStreamWriter_CAP::EndStream(FILE * const /*streamfp*/, EPlatform /*curPlatform*/, STREAM_TAG*)
{
   didWriteData = false;
}

//----------------------------------------------------------------------------

bool CStreamWriter_CAP::ReplacesDataForCurrStream( const unsigned int type )
{
   if( ( type & 0xffff ) == STREAM_TYPE_CAP )
   {
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------

unsigned int CStreamWriter_CAP::GetRebuildPlatformMask() const
{
   return kPlatformMask_Base;
}
