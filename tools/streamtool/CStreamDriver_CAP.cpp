//----------------------------------------------------------------------------
// CStreamDriver_Cap.cpp
//----------------------------------------------------------------------------

#include "CStreamDriver_CAP.h"

//----------------------------------------------------------------------------

#include "string.h"
#include "gcroot.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Xml;

namespace
{
   bool isInitialized = false;
   gcroot<XmlDocument^> captionDoc;
   gcroot<XmlNode^> documentRoot;
   int currentFontId;
}

//----------------------------------------------------------------------------

CStreamDriver_CAP::CStreamDriver_CAP(EMode const mode) 
: mMode(mode)
{
}

//----------------------------------------------------------------------------

CStreamDriver_CAP::~CStreamDriver_CAP()
{
   EndStream();
}

//----------------------------------------------------------------------------

struct SCaption
{
   int start_count;
   int end_count;
   int name;
   
#if MGS_VERSION==2
   int len;
#else
   short len;
   short lang;
#endif
   char string[0];
};

struct SAction
{
   int start_count;
   int name;
   int value;
   int option;
};

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

void CStreamDriver_CAP::ProcessPacket(const STREAM_TAG& packet, const void * const pBody)
{
   BeginStream();

   switch( mMode )
   {
   case kMode_Caption:
   case kMode_StrippedDownJapaneseCaption:
      ProcessPacket_Caption( packet, pBody );
      break;
   case kMode_RadioCaption:
      ProcessPacket_RadioCaption( packet, pBody );
      break;
   default:
      printf("CStreamDriver_CAP::ProcessPacket: TODO! mode %d\n", mMode);
      throw false;
   }
}

void CStreamDriver_CAP::ProcessPacket_Caption(const STREAM_TAG& packet, const void * const pBody)
{
   char* pCurrentData = (char*)pBody;
   
   int const offset = packet._option;

   int const captionSize = *((int*)pCurrentData);
   pCurrentData += 4;

   char* font_top = pCurrentData + captionSize;
   char* font_end = ((char*)pBody) + packet._size - sizeof(packet);

   int const packetFontId = currentFontId;

   if( mMode != kMode_StrippedDownJapaneseCaption )
   {
      // Write out font data for this caption packet
      {
         XmlElement^ element = captionDoc->CreateElement("FontData-Base64");

         element->SetAttribute("Id", currentFontId.ToString());
         ++currentFontId;

         int const destSize = font_end - font_top - sizeof(int);

         array<Byte>^ destData = gcnew array<Byte>( destSize );
         System::Runtime::InteropServices::Marshal::Copy( (IntPtr)(font_top + sizeof(int)), destData, 0, destSize );

         element->InnerText = System::Convert::ToBase64String( destData, System::Base64FormattingOptions::None );

         documentRoot->AppendChild(element);
      }
   }

   SAction* pCurrentAction = NULL;
   int actionCount = 0;
   
   char* pLevel = NULL;
   int levelSize = 0;

   int const ofs_base = offset & 0xFFFF;
   
   if( ofs_base > 0 )
   {
      pCurrentAction = (SAction*)((char*)font_top - ofs_base);
      actionCount = ofs_base / sizeof(SAction);
   }

   int const ofs_level = offset >> 16;

   if( ofs_level > 0 )
   {
      pLevel = (char*)font_top - ofs_base - ofs_level;
      levelSize = ofs_level;
   }

   char* caption_end = (char*)font_top - ofs_base - ofs_level;

   // Store captions
   while( pCurrentData != caption_end )
   {
      SCaption* pCurrentCaption = (SCaption*)pCurrentData;
      
      XmlElement^ caption = captionDoc->CreateElement("Caption");

#if MGS_VERSION==2
      short lang = packet._type >> 16;
#elif MGS_VERSION==3
      short lang = pCurrentCaption->lang;
#endif

      if( mMode != kMode_StrippedDownJapaneseCaption )
      {
         caption->SetAttribute("fontId", packetFontId.ToString());
         caption->SetAttribute("start", pCurrentCaption->start_count.ToString());
         caption->SetAttribute("end", pCurrentCaption->end_count.ToString());
         caption->SetAttribute("name", pCurrentCaption->name.ToString());
         caption->SetAttribute("lang", lang.ToString());
      }

      if( ( mMode != kMode_StrippedDownJapaneseCaption ) || (lang==GM_LANG_DEFAULT || lang == GM_LANG_JAPANESE) )
      {
         documentRoot->AppendChild(caption);

         // Text
         {
            XmlElement^ element = captionDoc->CreateElement("Text");
            element->InnerText = gcnew System::String(pCurrentCaption->string);
            caption->AppendChild(element);
         }

         // TextData
         {
            XmlElement^ element = captionDoc->CreateElement("Text-Base64");

            int const destSize = strlen(pCurrentCaption->string) + 1;

            array<Byte>^ destData = gcnew array<Byte>( destSize );
            System::Runtime::InteropServices::Marshal::Copy( (IntPtr)pCurrentCaption->string, destData, 0, destSize );

            element->InnerText = System::Convert::ToBase64String( destData, System::Base64FormattingOptions::None );
            
            caption->AppendChild(element);
         }
      }

      pCurrentData = pCurrentData + pCurrentCaption->len;
   }

   if( mMode != kMode_StrippedDownJapaneseCaption )
   {
      // actions are from MGS2 only
      short lang = packet._type >> 16;

      // Store actions
      for( int i = 0; i < actionCount; ++i )
      {
         XmlElement^ element = captionDoc->CreateElement("Action");

         element->SetAttribute("start", pCurrentAction[i].start_count.ToString());
         element->SetAttribute("name", pCurrentAction[i].name.ToString());
         element->SetAttribute("value", pCurrentAction[i].value.ToString());
         element->SetAttribute("option", pCurrentAction[i].option.ToString());
         element->SetAttribute("lang", lang.ToString());

         documentRoot->AppendChild(element);
      }

      // Level Data
      if(levelSize)
      {
         XmlElement^ element = captionDoc->CreateElement("LevelData-Base64");

         array<Byte>^ destData = gcnew array<Byte>( levelSize );
         System::Runtime::InteropServices::Marshal::Copy( (IntPtr)pLevel, destData, 0, levelSize );

         element->InnerText = System::Convert::ToBase64String( destData, System::Base64FormattingOptions::None );

         documentRoot->AppendChild(element);
      }
   }
}

void CStreamDriver_CAP::ProcessPacket_RadioCaption(const STREAM_TAG& packet, const void * const pBody)
{
   char* pCurrentData = (char*)pBody;

//   int const offset = packet._option;

   int const captionSize = *((int*)pCurrentData);
   pCurrentData += 4;

   char* caption_end = pCurrentData + captionSize;

   // Store captions
   while( pCurrentData != caption_end )
   {
      const SRadioCaption * pCurrentCaption = (SRadioCaption*)(pCurrentData);

      XmlElement^ caption = captionDoc->CreateElement("Caption");

#if MGS_VERSION==2
      short lang = packet._type >> 16;
#elif MGS_VERSION==3
      short lang = pCurrentCaption->lang;
#endif

      {
         caption->SetAttribute("start", pCurrentCaption->start_count.ToString());
         caption->SetAttribute("end", pCurrentCaption->end_count.ToString());
         caption->SetAttribute("lang", lang.ToString());
      }

      {
         //Throw in placeholders for default replacement (we'll be hand-editing these XML files in place
         //to fix timing bugs at the last moment and these same files will be loaded in by the corresponding
         //streamwriter...:(
         //If a platform-specific override is needed, just copy-paste the default override and name the new element
         //according to the platform.
         {
            XmlElement^ element = captionDoc->CreateElement("_bp");
            element->SetAttribute("start", pCurrentCaption->start_count.ToString());
            element->SetAttribute("end", pCurrentCaption->end_count.ToString());
            caption->AppendChild(element);
         }

      }
      documentRoot->AppendChild(caption);

      pCurrentData = pCurrentData + pCurrentCaption->len;
   }
}

//----------------------------------------------------------------------------

void CStreamDriver_CAP::BeginStream()
{
   if(!isInitialized)
   {
      isInitialized = true;
      captionDoc = gcnew XmlDocument();
      captionDoc->AppendChild( captionDoc->CreateXmlDeclaration( "1.0", nullptr, nullptr ) );
      documentRoot = captionDoc->AppendChild(captionDoc->CreateElement("CaptionDoc"));
      currentFontId = 0;
   }
}

//----------------------------------------------------------------------------

void CStreamDriver_CAP::EndStream()
{
   if(isInitialized)
   {
      isInitialized = false;

      char filename[256];
      const char * extension;
      switch( mMode )
      {
      case kMode_StrippedDownJapaneseCaption:
         extension = ".filtered.cap";
         break;
      case kMode_RadioCaption:
         extension = ".radio_cap";
         break;
      default:
         extension = ".cap";
         break;
      }

      sprintf(filename, "%s/%s/%s%s", gOutputFolder, gCurrStreamName, gCurrStreamName, extension );

      try
      {
         captionDoc->Save(gcnew System::String(filename));
      }
      catch(System::Xml::XmlException^ e)
      {
         System::Console::WriteLine(e->ToString());
      }

      printf("Extracting %s\n", filename);
   }
}