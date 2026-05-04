//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Streams/CLabeledOffsetStream.h"
#include "Engine/Streams/CDiskOutputStream.h"

#include "gcroot.h"

//----------------------------------------------------------------------------
/*

#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <vector>
#include <map>
#include <set>
*/
//----------------------------------------------------------------------------

#include "libgcl.h"

GCL_WORK_T gcl_work;

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;
using namespace TextToolData;

//----------------------------------------------------------------------------

enum EUsageMode
{
   kUsageMode_ExtractStringTable,
   kUsageMode_Rebuild,
   kUsageMode_DumpText,

   kUsageMode_Invalid = -1
};

enum ERegion
{
   kRegion_US,
   kRegion_JP,
   kRegion_EU
};

enum EPlatform
{
   kPlatform_PS3,
   kPlatform_X360
};

enum { kAllLanguages = -1 };

const char * const skPlatformExtension[] =
{
   "_ps3/",
   "_360/"
};

struct SCommandLineArgs
{
   char           mMainInputFilename[FILENAME_MAX];
   EUsageMode     mUsageMode;
   ERegion        mRegion;
   EPlatform      mPlatform;
   bool           mbInputFileIsList;
   int            mDumpLanguage;
   int            mExtractLanguage;
};

//----------------------------------------------------------------------------

SCommandLineArgs  gCommandLineArgs = { 0 };
gcroot<Backend^>  gTextBackend;
gcroot<StringDB^> gTextStringDB;
bool              gbOriginalReleaseData = false;
gcroot<IO::StreamWriter^> gTextDumpStreamWriter;

char gCurrInputFilename[FILENAME_MAX] = "";

//----------------------------------------------------------------------------

void BP_UnifyPath( char * path )
{
   int totalLen = 0;
   while( path[totalLen] != NULL )
   {
      if( path[totalLen] == '\\' )
      {
         path[totalLen] = '/';
      }
      ++totalLen;
   }
}

//----------------------------------------------------------------------------

static std::string ConvertString( System::String^ value )
{
   System::IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(value);

   char const* str = (char const*)(void*) ptr;

   std::string ret( str );

   System::Runtime::InteropServices::Marshal::FreeHGlobal( ptr );

   return ret;
}

//----------------------------------------------------------------------------

static void usage()
{
   printf("stringtableextract.exe: string table extraction tool for MGS%d.\n", MGS_VERSION );
   printf("usage: stringtableextract.exe [-x|b|d<n>] [/0] <gcx filename>\n");
   printf("-x[n]: extract string table from script. [0...7] extracts only specified language into subfolder\n");
   printf("-b   : rebuild new bp format script file..\n");
   printf("-d<n>: dump converted text of specified language [0...7] into a csv file..\n");
   printf("/0   : indicate original release data (e.g. MGS2: SOL rather than Substance)\n");
}

bool parse_command_line( int argc, char const *argv[] )
{
   if( argc < 2 )
      return false;

   gCommandLineArgs.mUsageMode = kUsageMode_Invalid;
   gCommandLineArgs.mRegion = kRegion_US;
   gCommandLineArgs.mPlatform = kPlatform_PS3;
   gCommandLineArgs.mbInputFileIsList = false;
   gCommandLineArgs.mDumpLanguage = 0;
   gCommandLineArgs.mExtractLanguage = kAllLanguages;

   for( int i=1; i < argc; ++i )
   {
      const char * arg = argv[i];
      if( !strncmp( arg, "-x", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_ExtractStringTable;
         if( (*(arg+2)!=0) && ( 1!=sscanf( arg+2,"%d", &gCommandLineArgs.mExtractLanguage ) ) )
         {
            printf("Error: invalid language for -x option: %s\n", arg+2 );
            throw false;
         }
      }
      else if( !strncmp( arg, "-b", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_Rebuild;
      }
      else if( !strncmp( arg, "-d", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_DumpText;
         if( 1!=sscanf( arg+2,"%d", &gCommandLineArgs.mDumpLanguage ) )
         {
            printf("Error: invalid language for -d option: %s\n", arg+2 );
            throw false;
         }
      }
      else if( !strncmp( arg, "/r", 2 ) )
      {
         if( !strcmp( arg+2, "jp" ) )
         {
            gCommandLineArgs.mRegion = kRegion_JP;
         }
         else if( !strcmp( arg+2, "eu" ) || !strcmp( arg+2, "fr" ) || !strcmp( arg+2, "gr" ) || !strcmp( arg+2, "it" ) || !strcmp( arg+2, "sp" ) )
         {
            gCommandLineArgs.mRegion = kRegion_EU;
         }
         else if( !strcmp( arg+2, "us" ) )
         {
            gCommandLineArgs.mRegion = kRegion_US;
         }
         else
         {
            printf("Error: unknown region specified: '%s'.\n", arg+2 );
            return false;
         }
      }
      else if( !strncmp( arg, "/p", 2 ) )
      {
         if( !strcmp( arg+2, "_ps3" ) )
         {
            gCommandLineArgs.mPlatform = kPlatform_PS3;
         }
         else if( !strcmp( arg+2, "_x360" ) )
         {
            gCommandLineArgs.mPlatform = kPlatform_X360;
         }
         else
         {
            printf("Error: unknown platform specified: '%s'.\n", arg+2 );
            return false;
         }
      }
      else if( !strcmp( arg, "/0" ) )
      {
         gbOriginalReleaseData = true;
      }
      else
      {
         strcpy( gCommandLineArgs.mMainInputFilename, arg );
         _strlwr( gCommandLineArgs.mMainInputFilename );
         BP_UnifyPath( gCommandLineArgs.mMainInputFilename );
         if( const char * const at = strchr( gCommandLineArgs.mMainInputFilename, '@' ) )
         {
            if( ( at == gCommandLineArgs.mMainInputFilename ) || ( at == strrchr( gCommandLineArgs.mMainInputFilename, '/' ) ) )
            {
               gCommandLineArgs.mbInputFileIsList = true;
               strcpy( gCommandLineArgs.mMainInputFilename, arg+1 );
               _strlwr( gCommandLineArgs.mMainInputFilename );
            }
         }
      }
   }

   return gCommandLineArgs.mMainInputFilename[0] != NULL && (gCommandLineArgs.mUsageMode != kUsageMode_Invalid);
}

#if MGS_VERSION == 2
static int find_lang_for_string( const GCL_ENVIRONMENT & env, const int index )
{
   if( gbOriginalReleaseData )
   {
      return 0;
   }
   //Really the only way to know is to look for a language-specific proc that references this string
   //and take the language of the proc.
   //A reference to a string is one byte, GCL_CONST|GCL_STTRES, followed by two bytes for the index.
   //Let's hope that this quick and dirty search doesn't produce false matches!
   uint32 * pProcs = (uint32*)env.script.proc_table;
   //Extra help against false positives-- detect when this pattern is encountered multiple times
   //in one proc but with different languages, and prefer to take the language that matches the
   //last language encountered since they seem to be somewhat grouped by language in the string table.
   int foundLangs[8] = { 0 };
   static int lastFoundLang = 0;

   for( int i = 0; i < env.script.proc_num; ++i )
   {
      int procId = GCL_GetLong( (char*)(pProcs+i*2+0));
      int procOffset = GCL_GetLong( (char*)(pProcs+i*2+1));
      char * procBegin = env.script.proc_body + procOffset;
      int procSize;
      /*unsigned char * next = */
         GCL_GetBlockSize( (unsigned char*)procBegin, &procSize );
      char * procEnd = procBegin + procSize;
      for( char * p = procBegin; p <= procEnd - 3; ++p )
      {
         if( *p == ( GCL_CONST | GCL_STRRES ) )
         {
            const int refIndex = GCL_GetShort(p+1);
            if( refIndex == index )
            {
               int procLang = procId >> 24;
               if( procLang > 0 && procLang < 8 )
               {
                  foundLangs[procLang] = 1;
               }
               else
               {
                  //no Substance data contains language 0; assume this pattern is a false match.
               }
            }
         }
      }
   }
   //Did we find a match with the last language encountered?
   if( foundLangs[lastFoundLang] )
   {
      return lastFoundLang;
   }
   //Okay, must be a new language.  If there are multiple matches, at least warn, then take the highest
   //number since we're pretty much just using this for Japanese (language 7) processes anyway.
   //Easy to visually sort out false positives in that case (an english string showing up in a dump)
   for( int i=7; i > 0; --i )
   {
      if( foundLangs[i] )
      {
         lastFoundLang = i;
         return i;
      }
   }
   printf( "Warning: %s: could not find language for string index %d\n", gCurrInputFilename, index );
   return 0;
}
#endif

void process_input_file()
{
   printf("Processing: %s...\n", gCurrInputFilename);
   
   // Skip over files in _bp folder, as these are already processed!
   if( strstr(gCurrInputFilename, "_bp") )
      return;

   FILE* file = fopen(gCurrInputFilename, "rb");

   if( file )
   {
      fseek(file, 0, SEEK_END);
      int const fileSize = ftell(file);
      fseek(file, 0, SEEK_SET);

      if( fileSize > 0 )
      {
         std::auto_ptr<char> scriptData(new char[fileSize]);
         char* pScriptData = scriptData.get();

         if( fread(pScriptData, fileSize, 1, file) == 1 )
         {
            GCL_LoadScript(pScriptData);
            
            GCL_ENVIRONMENT env;
            GCL_SaveCurrentEnvironment(&env);

            if( gCommandLineArgs.mUsageMode == kUsageMode_ExtractStringTable )
            {
               XmlDocument^ doc = gcnew XmlDocument();
               doc->AppendChild( doc->CreateXmlDeclaration( "1.0", nullptr, nullptr ) );

               XmlElement^ documentRoot = doc->CreateElement("StringTable");
               doc->AppendChild(documentRoot);

               char* pFontBegin = GCL_GetFontDataTop();
               char* pFontEnd = GCL_GetFontDataEnd();

               // Write out font data for this caption packet
               {
                  XmlElement^ element = doc->CreateElement("FontData-Base64");
                  element->SetAttribute("Id", "0");

                  int const destSize = pFontEnd - pFontBegin;
                  if( destSize > 0 )
                  {
                     array<Byte>^ destData = gcnew array<Byte>( destSize );
                     System::Runtime::InteropServices::Marshal::Copy( (IntPtr)pFontBegin, destData, 0, destSize );

                     element->InnerText = System::Convert::ToBase64String( destData, System::Base64FormattingOptions::None );
                  }
                  documentRoot->AppendChild(element);
               }

               int* pStringOffsets = env.res.resource_table;
               int const numStrings = (int*)env.res.string_table - env.res.resource_table;

               for( int i = 0; i < numStrings; ++i )
               {
                  // only process "string" type resources from the resource table.
                  if( pStringOffsets[i] & 0x80000000 )
                  {
                     int const offset = pStringOffsets[i] & 0x7FFFFFFF;
#if MGS_VERSION==2
                     int const lang = find_lang_for_string( env, i );
#else
                     int const lang = 0;
#endif

                     if( ( gCommandLineArgs.mExtractLanguage != kAllLanguages ) && ( lang != gCommandLineArgs.mExtractLanguage ) )
                     {
                        continue;
                     }

                     char* pString = env.res.string_table + offset;

                     XmlElement^ stringElement = doc->CreateElement("String");
                     stringElement->SetAttribute("fontId", "0");
                     stringElement->SetAttribute("offset", offset.ToString());
                     stringElement->SetAttribute("Language", lang.ToString());

                     // Text
                     {
                        XmlElement^ element = doc->CreateElement("Text");
                        element->InnerText = gcnew System::String(pString);
                        stringElement->AppendChild(element);
                     }

                     // TextData
                     {
                        XmlElement^ element = doc->CreateElement("Text-Base64");

                        int const destSize = strlen(pString) + 1;

                        array<Byte>^ destData = gcnew array<Byte>( destSize );
                        System::Runtime::InteropServices::Marshal::Copy( (IntPtr)pString, destData, 0, destSize );

                        element->InnerText = System::Convert::ToBase64String( destData, System::Base64FormattingOptions::None );
                        stringElement->AppendChild(element);
                     }

                     documentRoot->AppendChild(stringElement);
                  }
               }

               char outputFilename[256];
               strcpy(outputFilename, gCurrInputFilename);
               *strstr(outputFilename, ".") = 0;

               if( gCommandLineArgs.mExtractLanguage != kAllLanguages )
               {
                  String^ langOutputFilename = IO::Path::GetDirectoryName(gcnew String(outputFilename));
                  langOutputFilename = IO::Path::Combine(langOutputFilename, gCommandLineArgs.mExtractLanguage.ToString());
                  IO::Directory::CreateDirectory( langOutputFilename );
                  langOutputFilename = IO::Path::Combine(langOutputFilename, IO::Path::GetFileName(gcnew String(outputFilename)));
                  doc->Save(gcnew String(langOutputFilename) + ".strings");
               }
               else
               {
                  doc->Save(gcnew String(outputFilename) + ".strings");
               }
            }
            else if( gCommandLineArgs.mUsageMode == kUsageMode_Rebuild )
            {
               Backend^ backend = gTextBackend;
               
               if( backend == nullptr )
               {
                  backend = gTextBackend = gcnew Backend();
               }

               StringDB^ stringDB = gTextStringDB;
               if( stringDB == nullptr )
               {
                  stringDB = gTextStringDB = gcnew StringDB(backend);
               }

               String^ inputPath = gcnew String(gCurrInputFilename);

               String^ outputFilename = IO::Path::GetDirectoryName(inputPath);
               outputFilename = IO::Path::Combine(outputFilename, "_bp");
               outputFilename = IO::Path::Combine(outputFilename, IO::Path::GetFileName(inputPath));
               
               IO::Directory::CreateDirectory(IO::Path::GetDirectoryName(outputFilename));

               CLabeledOffsetStream gcl;
               gcl.SetEndian(COutputStream::kOE_LittleEndian);

               // Begin writing header
               gcl.AddLabel("Header");

               // Write unique BP identifier
               gcl.WriteUint32('BGCL');
               // Write GCL variable version
               gcl.WriteUint32(GCL_VariableVersion);

               // Write all procs
               uint32 * pProcs = (uint32*)env.script.proc_table;
               for( int i = 0; i < env.script.proc_num; ++i )
               {
#if MGS_VERSION == 3
                  gcl.WriteUint32(pProcs[i]);
#else
                  gcl.WriteUint32(pProcs[i*2+0]);
                  gcl.WriteUint32(pProcs[i*2+1]);
#endif
               }

               // Write last proc terminator
#if MGS_VERSION == 3
               gcl.WriteUint32(0xFFFFFFFF);
#else
               gcl.WriteUint32(0);
               gcl.WriteUint32(0);
#endif

               // Begin writing string resource
               gcl.AddLabel("StringResource");

               // Write size of string resource block
               gcl.AddOffset32("ProcBodySection", "StringResource");

               // Write offset to resource table
               gcl.AddOffset32("ResourceTable", "StringResource");

               // Write offset to string table
               gcl.AddOffset32("StringTable", "StringResource");

               // Write offset to font data
               gcl.AddOffset32("FontData", "StringResource");

#if MGS_VERSION == 3
               // Write "seed" for decoding, 0 means data is already decoded!
               gcl.WriteUint32(0);
#endif

               // Write offset to BPStringData
               gcl.AddOffset32("BPStringData", "StringResource");

               // Write BPStringData
               {
                  gcl.AddLabel("BPStringData");

                  String^ stringsPath = IO::Path::ChangeExtension(inputPath, "strings");
                  
                  XmlDocument^ stringsDoc = gcnew XmlDocument();
                  stringsDoc->Load(stringsPath);
                  
                  uint32* pStringOffsets = (uint32*)env.res.resource_table;
                  int const numStrings = (int*)env.res.string_table - env.res.resource_table;

                  // Write string table count
                  gcl.WriteUint32(numStrings);

                  // Write offsets to strings
                  for( int i = 0; i < numStrings; ++i )
                  {
                     // only process "string" type resources from the resource table.
                     if( pStringOffsets[i] & 0x80000000 )
                     {
                        int const offset = pStringOffsets[i] & 0x7FFFFFFF;
                        gcl.AddOffset32(CStringExtras::Stringize("String%d", offset), "BPStringData");
                     }
                     else
                     {
                        gcl.WriteUint32(0xFFFFFFFF);
                     }
                  }

                  // Write strings
                  for( int i = 0; i < numStrings; ++i )
                  {
                     // only process "string" type resources from the resource table.
                     if( pStringOffsets[i] & 0x80000000 )
                     {
                        int const offset = pStringOffsets[i] & 0x7FFFFFFF;

                        gcl.AddLabel(CStringExtras::Stringize("String%d", offset));

                        XmlElement^ stringElement = (XmlElement^)stringsDoc->SelectSingleNode(String::Format("//String[@offset='{0}']", offset));

                        List<int>^ problemCodes = gcnew List<int>();

                        String^ decodedString = backend->DecodeString(stringElement, nullptr, problemCodes, nullptr);

                        if( problemCodes->Count > 0 )
                        {
                           System::Console::WriteLine("ERROR: {0}: Problems while converting string: {1}", stringsPath, decodedString);
                        }

                        String^ remappedString = gTextStringDB->ApplyOriginalCodeOverrides(decodedString);
                        remappedString = gTextStringDB->ApplyKeywordReplacement(remappedString);

                        array<Byte>^ utf8Data = backend->ConvertStringToUTF8(remappedString);
                      
                        for( int i = 0; i < utf8Data->Length; ++i )
                           gcl.WriteUint8(utf8Data[i]);

                        // terminating 0.
                        gcl.WriteUint8(0);
                     }
                  }
               }

               // Write resource table
               {
                  gcl.AddLabel("ResourceTable");
               
                  uint32* pStringOffsets = (uint32*)env.res.resource_table;
                  int const numStrings = (int*)env.res.string_table - env.res.resource_table;

                  for( int i = 0; i < numStrings; ++i )
                  {
                     gcl.WriteUint32(pStringOffsets[i]);
                  }
               }

               // Write string table
               {
                  gcl.AddLabel("StringTable");
                  int const stringTableSize = env.res.font_data - env.res.string_table;
                  gcl.Put(env.res.string_table, stringTableSize);
               }
               
               // Write font data
               {
                  gcl.AddLabel("FontData");

                  char* pFontBegin = GCL_GetFontDataTop() - 4;
                  char* pFontEnd = GCL_GetFontDataEnd();

                  int const fontDataSize = pFontEnd - pFontBegin;

                  if( fontDataSize > 0 )
                  {
                     gcl.Put(pFontBegin, fontDataSize);
                  }
               }

               // Write proc body
               {
                  gcl.AddLabel("ProcBodySection");

                  // Write size of proc body
                  gcl.AddOffset32("ProcBodyEnd", "ProcBody");

                  // Begin proc body
                  gcl.AddLabel("ProcBody");

                  int const procDataSize = GCL_GetLong(env.script.proc_body - 4);
                  gcl.Put(env.script.proc_body, procDataSize);
                  gcl.AddLabel("ProcBodyEnd");
               }
               
               // Write script body
               {
                  // Write size of script body
                  gcl.AddOffset32("ScriptBodyEnd", "ScriptBody");

                  // Begin script body
                  gcl.AddLabel("ScriptBody");

                  int const scriptDataSize = GCL_GetLong(env.script.script_body - 4);
                  gcl.Put(env.script.script_body, scriptDataSize);
                  gcl.AddLabel("ScriptBodyEnd");
               }

               // Align to 16 bytes
               gcl.AddAlignmentPadding(16, 0);

               gcl.PatchOffsets();

               std::string outFilename = ConvertString(outputFilename);
               CDiskOutputStream outStream(outFilename);

               if( gcl.GetDataSize() > 0 )
                  outStream.Put(gcl.GetData(), gcl.GetDataSize());
            }
            else if( gCommandLineArgs.mUsageMode == kUsageMode_DumpText )
            {
               Backend^ backend = gTextBackend;

               if( backend == nullptr )
               {
                  backend = gTextBackend = gcnew Backend();
               }

               // Write BPStringData
               {
                  String^ inputPath = gcnew String(gCurrInputFilename);
                  String^ stringsPath = IO::Path::ChangeExtension(inputPath, "strings");
                  String^ inputFilename = IO::Path::GetFileName( inputPath );

                  if( !gCommandLineArgs.mbInputFileIsList )
                  {
                     String^ dumpPath = IO::Path::ChangeExtension(inputPath, "csv");
                     gTextDumpStreamWriter = IO::File::CreateText( dumpPath );
                  }

                  XmlDocument^ stringsDoc = gcnew XmlDocument();
                  stringsDoc->Load(stringsPath);

                  uint32* pStringOffsets = (uint32*)env.res.resource_table;
                  int const numStrings = (int*)env.res.string_table - env.res.resource_table;

                  System::String^ langString = gCommandLineArgs.mDumpLanguage.ToString();
                  bool bFirstString = true;
                  // Dump strings
                  for( int i = 0; i < numStrings; ++i )
                  {
                     // only process "string" type resources from the resource table.
                     if( pStringOffsets[i] & 0x80000000 )
                     {
                        int const offset = pStringOffsets[i] & 0x7FFFFFFF;

                        XmlElement^ stringElement = (XmlElement^)stringsDoc->SelectSingleNode(String::Format("//String[@offset='{0}']", offset));
                        int const elementLanguage = Int32::Parse(stringElement->GetAttribute("Language"));
                        if( elementLanguage == gCommandLineArgs.mDumpLanguage )
                        {
                           List<int>^ problemCodes = gcnew List<int>();

                           String^ decodedString = backend->DecodeString(stringElement, nullptr, problemCodes, nullptr);

                           //Strings have linefeeds in them, so they are quote-separated.  This means we must replace any actual " characters in the
                           //strings themselves with another one.
                           decodedString->Replace( "\"", "“" );

                           if( problemCodes->Count > 0 )
                           {
                              System::Console::WriteLine("ERROR: {0}: Problems while converting string: {1}", stringsPath, decodedString);
                           }

                           gTextDumpStreamWriter->WriteLine( "\"{0}\"\t\"{1}\"", bFirstString ? inputFilename : gcnew System::String(""), decodedString );
                           bFirstString = false;
                        }
                     }
                  }

                  gTextDumpStreamWriter->WriteLine( "" );
                  if( !gCommandLineArgs.mbInputFileIsList )
                  {
                     gTextDumpStreamWriter->Close();
                     gTextDumpStreamWriter = nullptr;
                  }
               }
            }
         }
      }

      fclose(file);
   }
}

int main( int argc, char const *argv[] )
{
   if( !parse_command_line( argc, argv ) )
   {
      usage();
      return -1;
   }

   try
   {
      if( gCommandLineArgs.mUsageMode == kUsageMode_DumpText && gCommandLineArgs.mbInputFileIsList )
      {
         String^ dumpFolder = IO::Path::GetDirectoryName( gcnew System::String(gCommandLineArgs.mMainInputFilename) );
         String^ dumpPath = IO::Path::Combine(dumpFolder, "strings.csv");
         gTextDumpStreamWriter = IO::File::CreateText( dumpPath );
      }

      if( !gCommandLineArgs.mbInputFileIsList )
      {
         strcpy( gCurrInputFilename, gCommandLineArgs.mMainInputFilename );
         process_input_file();
      }
      else
      {
         FILE * textfp = fopen( gCommandLineArgs.mMainInputFilename, "rt" );
         if( !textfp )
         {
            printf("Error: could not open %s for reading.\n", gCommandLineArgs.mMainInputFilename );
            throw false;
         }
         char processFilename[FILENAME_MAX];
         while( 1 == fscanf( textfp, "%s", processFilename ) )
         {
            strcpy( gCurrInputFilename, processFilename );
            process_input_file();
         }
         fclose( textfp );
      }
   }
   catch(System::Exception^ e)
   {
      System::Console::WriteLine(e->ToString());
      return -1;
   }
   catch(...)
   {
      printf("stringtableextract failed.\n");
      return -1;
   }

   return 0;
}
