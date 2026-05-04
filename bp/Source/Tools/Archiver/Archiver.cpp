// Archiver.cpp : main project file.

#include "stdafx.h"

//----------------------------------------------------------------------------

static System::String^ BP_GetEnvironmentVariable( System::String^ name )
{
   System::String^ var = System::Environment::GetEnvironmentVariable(name);
   return var;
}

//----------------------------------------------------------------------------

#include <windows.h>

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/Mechanics/zlib/zlib.h"
#include "Engine/Basics/NEndian.h"

//----------------------------------------------------------------------------

#define XCOMPRESS_FILE_IDENTIFIER_LZXNATIVE         0x0FF512EE

typedef struct _XCOMPRESS_FILE_HEADER {
   DWORD Identifier;
   WORD Version;
   WORD Reserved;
} XCOMPRESS_FILE_HEADER;

typedef struct _XMEMCODEC_PARAMETERS_LZX {
   DWORD Flags;
   DWORD WindowSize;
   DWORD CompressionPartitionSize;
} XMEMCODEC_PARAMETERS_LZX;

typedef struct _XCOMPRESS_FILE_HEADER_LZXNATIVE {
   XCOMPRESS_FILE_HEADER Common;
   DWORD ContextFlags;
   XMEMCODEC_PARAMETERS_LZX CodecParams;
   DWORD UncompressedSizeHigh;
   DWORD UncompressedSizeLow;
   DWORD CompressedSizeHigh;
   DWORD CompressedSizeLow;
   DWORD UncompressedBlockSize;
   DWORD CompressedBlockSizeMax;
} XCOMPRESS_FILE_HEADER_LZXNATIVE;

//----------------------------------------------------------------------------

ref class FileEntry
{
public:
   System::String^ mSourceFilePath;
   System::String^ mArchivePath;

   array<unsigned char>^ mArchivePathHash;
   System::String^ mArchivePathHashString;

   // This information is currently only used when writing out archive stats
   int64 mSourceSize;
   int64 mCompressedSize;
   int   mTotalBlockCount;
   int   mCompressedBlockCount;
   int   mFileCount;
   int   mUniqueFileCount;
};

//----------------------------------------------------------------------------

ref class Archive
{
public:
   System::String^ mArchiveFilename;
   System::Collections::Generic::List<FileEntry^>^ mFileEntries;
   System::Collections::Generic::SortedDictionary<System::String^,FileEntry^>^ mHashEntries;
   System::Text::ASCIIEncoding^ mAsciiEncoder;
   int   mBlockSize;

   Archive()
   {
      mFileEntries = gcnew System::Collections::Generic::List<FileEntry^>();
      mHashEntries = gcnew System::Collections::Generic::SortedDictionary<System::String^,FileEntry^>();
      mAsciiEncoder = gcnew System::Text::ASCIIEncoding();
   }

   void AddEntry( FileEntry ^ entry )
   {
      //sanity check against dupes.
      array<unsigned char>^ stringBytes = mAsciiEncoder->GetBytes( entry->mArchivePath );

      entry->mArchivePathHash = (gcnew System::Security::Cryptography::SHA1CryptoServiceProvider())->ComputeHash( stringBytes );
      entry->mArchivePathHashString = "";
      for( int i=0; i < entry->mArchivePathHash->Length; ++i )
      {
         entry->mArchivePathHashString += ( gcnew System::Byte( entry->mArchivePathHash[i] ) )->ToString( "X2" );
      }

      FileEntry^ existingValue;
      if( mHashEntries->TryGetValue( entry->mArchivePathHashString, existingValue ) )
      {
         if( existingValue->mArchivePath == entry->mArchivePath )
         {
            System::Console::WriteLine("Warning: ignoring duplicate entry for " + entry->mArchivePath );
            return;
         }
         else
         {
            //This is an error because it is a hash collision!
            System::Console::WriteLine("Error: hash collision between {0} and {1}" + existingValue->mArchivePath, entry->mArchivePath );
            throw false;
         }
      }
      mHashEntries->Add( entry->mArchivePathHashString, entry );
      mFileEntries->Add( entry );
   }
};

//----------------------------------------------------------------------------

ref class Main
{
public:
   static System::String^           sInputXMLFilename;
   static System::Xml::XmlDocument^ sXMLDocument;
   static Archive^                  sArchive;
   static System::Collections::Generic::Dictionary<System::String^, int>^ sCommonSourceFilenames;
   static array<System::String^>^ sCommonSourceDirectories_ToExclude;   

   static bool                      sbCleanBuild;
   static bool                      sbWriteStats;
   static bool                      sbWriteCommonFileXML;
   static int64                     sCommonFilesMaxArchiveSize;
   static System::String^           sCommonFilesXMLFilename;
   static System::String^           sCommonFilesXMLFilename_ToExclude;
};

//----------------------------------------------------------------------------

struct SArchiveHeader
{
   unsigned int   mMagicNumber;
   unsigned int   mVersionNumber;
   unsigned int   mFileCount;
   unsigned int   mBlockSize;
   unsigned int   mBlockCount;
};

static const unsigned int skMagicNumber = 'XBAR';
static const unsigned int skVersionNumber = 0;

//----------------------------------------------------------------------------

struct SArchiveTOCEntry
{
   unsigned char mArchivePathHash[20];
   unsigned int mBlockListStartIndex;  //Index of first block in compressed block size list for this file
   unsigned int mOriginalSize;         //uncompressed size
   unsigned __int64 mStartOffset;      //Start offset of compressed blocks for this file in the archive

   bool operator < ( SArchiveTOCEntry const & rhs ) const
   {
      return memcmp( mArchivePathHash, rhs.mArchivePathHash, sizeof( mArchivePathHash ) ) < 0;
   }
};

//----------------------------------------------------------------------------

unsigned int BP_GetFileSizeAttr( const char * const path )
{
   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesExA( path, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      printf("File not found: %s\n", path );
      throw false;
   }
   _ULARGE_INTEGER byteSize;
   byteSize.HighPart = fileAttr.nFileSizeHigh;
   byteSize.LowPart = fileAttr.nFileSizeLow;
   if( fileAttr.nFileSizeHigh )
   {
      printf("TODO: file sizes greater than 32-bit! (%s)\n", path );
      throw false;
   }
   return (unsigned int)byteSize.QuadPart;
}

//----------------------------------------------------------------------------

unsigned long long BP_GetFileModificationDate( const char * const path )
{
   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesExA( path, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      printf("GetFileModificationDate failed: %s\n", path );
      throw false;
   }
   _ULARGE_INTEGER lastWriteTime;
   lastWriteTime.HighPart = fileAttr.ftLastWriteTime.dwHighDateTime;
   lastWriteTime.LowPart = fileAttr.ftLastWriteTime.dwLowDateTime;
   return lastWriteTime.QuadPart;
}

//----------------------------------------------------------------------------

static void ByteSwap_XCOMPRESS_FILE_HEADER_LZXNATIVE( XCOMPRESS_FILE_HEADER_LZXNATIVE & fileHeader )
{
   NEndian::Swap4Bytes( &fileHeader.Common.Identifier );
   NEndian::Swap2Bytes( &fileHeader.Common.Version );
   NEndian::Swap2Bytes( &fileHeader.Common.Reserved );
   NEndian::Swap4Bytes( &fileHeader.ContextFlags );
   NEndian::Swap4Bytes( &fileHeader.CodecParams.Flags );
   NEndian::Swap4Bytes( &fileHeader.CodecParams.WindowSize );
   NEndian::Swap4Bytes( &fileHeader.CodecParams.CompressionPartitionSize );
   NEndian::Swap4Bytes( &fileHeader.UncompressedSizeHigh );
   NEndian::Swap4Bytes( &fileHeader.UncompressedSizeLow );
   NEndian::Swap4Bytes( &fileHeader.CompressedSizeHigh );
   NEndian::Swap4Bytes( &fileHeader.CompressedSizeLow );
   NEndian::Swap4Bytes( &fileHeader.UncompressedBlockSize );
   NEndian::Swap4Bytes( &fileHeader.CompressedBlockSizeMax );
}

//----------------------------------------------------------------------------

static void ByteSwap_ArchiveHeader( SArchiveHeader & header )
{
   NEndian::Swap4Bytes( &header.mMagicNumber );
   NEndian::Swap4Bytes( &header.mVersionNumber );
   NEndian::Swap4Bytes( &header.mFileCount );
   NEndian::Swap4Bytes( &header.mBlockSize );
   NEndian::Swap4Bytes( &header.mBlockCount );
}

//----------------------------------------------------------------------------

static void ByteSwap_TOCEntry( SArchiveTOCEntry & entry )
{
   NEndian::Swap4Bytes( &entry.mBlockListStartIndex );
   NEndian::Swap4Bytes( &entry.mOriginalSize );
   NEndian::Swap8Bytes( &entry.mStartOffset );
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

static System::String^ GetCompressedFilename( System::String^ filename )
{
   return "xbarc_temp/" + filename + ".xbc";
}

//----------------------------------------------------------------------------

static void CreateFullSubdirectories( System::String^ filename )
{
   int slashPos = 0;
   while( (slashPos = filename->IndexOf( "/", slashPos )) != -1 )
   {
      System::String^ folder = filename->Substring( 0, slashPos );
      System::IO::Directory::CreateDirectory( folder );
      ++slashPos;
   }
}

//----------------------------------------------------------------------------

static void ShowUsage()
{
   System::Console::WriteLine("Builds X360 compressed XBARC archives.");
   System::Console::WriteLine("Usage:");
   System::Console::WriteLine(" Archiver [/c] [/s] [/bMaxSize] <FILENAME> [CommonFilesXML] [MainArchiveExcludeXML]");
   System::Console::WriteLine("  /c           = clean build (recompress intermediate files)");
   System::Console::WriteLine("  /s           = write out CSV with stats");
   System::Console::WriteLine("  /bMaxSize    = write out XML of common files to MaxSize KB");
}

//----------------------------------------------------------------------------

static bool ProcessArgs(int argc, char *argv[])
{
   Main::sbCleanBuild = false;
   Main::sbWriteStats = false;
   bool bArgsError = false;

   for( int ii=1; ii < argc; ++ii )
   {
      char* pArg = argv[ii];
      if (pArg[0] == '/')
      {
         if (strlen(pArg) < 2)
         {
            printf("Unknown argument: %s\n", pArg);
            bArgsError = true;
            break;
         }

         switch (toupper(pArg[1]))
         {
            case 'C':
               Main::sbCleanBuild = true;
               break;
            case 'S':
               Main::sbWriteStats = true;
               break;
            case 'B':
               if (strlen(pArg) > 2)
               {
                  Main::sCommonFilesMaxArchiveSize = atoi(pArg + 2) * 1024;
                  Main::sbWriteCommonFileXML = true;
               }
               break;
            default:
               printf("Unknown argument: %s\n", pArg);
               bArgsError = true;
               break;
         }
      }
      else
      {
         if (Main::sInputXMLFilename == nullptr)
         {
            // Source XML filename
            System::String^ clrString = gcnew System::String(pArg);
            Main::sInputXMLFilename = System::IO::Path::GetFullPath(clrString);
         }
         else if (Main::sCommonFilesXMLFilename == nullptr)
         {
            // Common files XML filename
            System::String^ clrString = gcnew System::String(pArg);
            Main::sCommonFilesXMLFilename = System::IO::Path::GetFullPath(clrString);            
         }
         else if (Main::sCommonFilesXMLFilename_ToExclude == nullptr)
         {
            // Common files to exclude XML filename
            System::String^ clrString = gcnew System::String(pArg);
            Main::sCommonFilesXMLFilename_ToExclude = System::IO::Path::GetFullPath(clrString); 
         }
         else
         {
            // Already got input filename and common files filename
            printf("Too many arguments: %s\n", pArg);
            bArgsError = true;
         }
      }
   }

   if (Main::sInputXMLFilename == nullptr)
   {
      System::Console::WriteLine("Missing source XML file");
      bArgsError = true;
   }

   return bArgsError;
}

//----------------------------------------------------------------------------
// Writes out a CSV file with stats for archive files
static void WriteArchiveFileStats()
{
   System::Collections::Generic::Dictionary<System::String^, int>^ sourceFileEntryCounts = gcnew System::Collections::Generic::Dictionary<System::String^, int>();
   System::Collections::Generic::List<FileEntry^>^ sourceFileEntries = gcnew System::Collections::Generic::List<FileEntry^>();

   // Determine unique source files
   // Also add to list so XML ordering is preserved
   for (int i = 0; i < Main::sArchive->mFileEntries->Count; ++i)
   {
      FileEntry^ fileEntry = Main::sArchive->mFileEntries[i];
      if (sourceFileEntryCounts->ContainsKey(fileEntry->mSourceFilePath) )
      {
         // Already exists, increase usage count
         sourceFileEntryCounts[fileEntry->mSourceFilePath]++;
      }
      else
      {
         // Add this entry
         sourceFileEntryCounts->Add(fileEntry->mSourceFilePath, 1);
         sourceFileEntries->Add(fileEntry);
      }
   }

   // Write CSV for general stats
   {
      System::IO::StreamWriter^ sw = System::IO::File::CreateText(System::String::Format("{0}_stats_files.csv", Main::sInputXMLFilename));
      sw->WriteLine("Type, Filename, UsedCount, SourceSize, CompressedSize, TotalCompressedSize, CompressionRatio, TotalBlockCount, CompressedBlockCount");
      for (int i = 0; i < sourceFileEntries->Count; ++i)
      {
         FileEntry^ fileEntry = sourceFileEntries[i];
         int entryCount = sourceFileEntryCounts[fileEntry->mSourceFilePath];
         sw->WriteLine(System::String::Format("{0}, {1}, {2}, {3}, {4}, {5}, {6:0.00}, {7}, {8}",
            System::IO::Path::GetExtension(fileEntry->mSourceFilePath),
            fileEntry->mSourceFilePath,
            entryCount,
            fileEntry->mSourceSize,
            fileEntry->mCompressedSize,
            entryCount * fileEntry->mCompressedSize,
            fileEntry->mCompressedSize / (float) fileEntry->mSourceSize,
            fileEntry->mTotalBlockCount,
            fileEntry->mCompressedBlockCount
            ));
      }
      delete (System::IDisposable^) sw;
   }

   // Get stats by type
   System::Collections::Generic::SortedDictionary<System::String^, FileEntry^>^ sourceFileEntryByType = gcnew System::Collections::Generic::SortedDictionary<System::String^, FileEntry^>();

   for (int i = 0; i < sourceFileEntries->Count; ++i)
   {
      FileEntry^ fileEntry = sourceFileEntries[i];
      int entryCount = sourceFileEntryCounts[fileEntry->mSourceFilePath];

      System::String^ fileType = System::IO::Path::GetExtension(fileEntry->mSourceFilePath);
      FileEntry^ existingTypeEntry = nullptr;
      if (sourceFileEntryByType->TryGetValue(fileType, existingTypeEntry) )
      {
         // Type already exists, sum
         existingTypeEntry->mSourceSize += entryCount * fileEntry->mSourceSize;
         existingTypeEntry->mCompressedSize += entryCount * fileEntry->mCompressedSize;
         existingTypeEntry->mTotalBlockCount += entryCount * fileEntry->mTotalBlockCount;
         existingTypeEntry->mCompressedBlockCount += entryCount * fileEntry->mCompressedBlockCount;
         existingTypeEntry->mFileCount += entryCount;
         existingTypeEntry->mUniqueFileCount++;

      }
      else
      {
         // New type, create new entry
         FileEntry^ typeEntry = gcnew FileEntry;

         typeEntry->mSourceSize += entryCount * fileEntry->mSourceSize;
         typeEntry->mCompressedSize += entryCount * fileEntry->mCompressedSize;
         typeEntry->mTotalBlockCount += entryCount * fileEntry->mTotalBlockCount;
         typeEntry->mCompressedBlockCount += entryCount * fileEntry->mCompressedBlockCount;
         typeEntry->mSourceFilePath = fileType;
         typeEntry->mFileCount = entryCount;
         typeEntry->mUniqueFileCount = 1;
         sourceFileEntryByType->Add(fileType, typeEntry);
      }
   }

   // Write CSV for general stats
   {
      System::IO::StreamWriter^ sw = System::IO::File::CreateText(System::String::Format("{0}_stats_types.csv", Main::sInputXMLFilename));
      sw->WriteLine("Type, FileCount, UniqueFileCount, TotalSourceSizeKB, TotalCompressedSizeKB, CompressionRatio, TotalBlockCount, CompressedBlockCount");

      for each(System::Collections::Generic::KeyValuePair<System::String^, FileEntry^> kvp in sourceFileEntryByType)
      {
         FileEntry^ typeEntry = kvp.Value;
         sw->WriteLine(System::String::Format("{0}, {1}, {2}, {3}, {4}, {5:0.00}, {6}, {7}",
            typeEntry->mSourceFilePath,
            typeEntry->mFileCount,
            typeEntry->mUniqueFileCount,
            typeEntry->mSourceSize / 1024,
            typeEntry->mCompressedSize / 1024,
            typeEntry->mCompressedSize / (float) typeEntry->mSourceSize,
            typeEntry->mTotalBlockCount,
            typeEntry->mCompressedBlockCount
            ));
      }
      delete (System::IDisposable^) sw;
   }

}

//----------------------------------------------------------------------------

ref class CCommonFileEntry : public System::IComparable
{
public:
   FileEntry^  mFileEntry;
   int64       mDuplicatedSizeOverhead;

   virtual System::Int32 CompareTo(Object^ obj)
   {
      if (obj->GetType() == CCommonFileEntry::typeid)
      {
         CCommonFileEntry^ temp = dynamic_cast<CCommonFileEntry^>(obj);

         return temp->mDuplicatedSizeOverhead.CompareTo(mDuplicatedSizeOverhead);
      }
      throw gcnew System::ArgumentException("object is not a CCommonFileEntry");
   }
};

//----------------------------------------------------------------------------
// Writes out an XML file with largest space saving for common files
static void WriteCommonFilesXML(int64 maxCommonArchiveSize)
{
   System::Collections::Generic::Dictionary<System::String^, int>^ sourceFileEntryCounts = gcnew System::Collections::Generic::Dictionary<System::String^, int>();
   System::Collections::Generic::List<FileEntry^>^ sourceFileEntries = gcnew System::Collections::Generic::List<FileEntry^>();

   // Determine unique source files
   // Also add to list so XML ordering is preserved
   for (int i = 0; i < Main::sArchive->mFileEntries->Count; ++i)
   {
      FileEntry^ fileEntry = Main::sArchive->mFileEntries[i];
      if (sourceFileEntryCounts->ContainsKey(fileEntry->mSourceFilePath) )
      {
         // Already exists, increase usage count
         sourceFileEntryCounts[fileEntry->mSourceFilePath]++;
      }
      else
      {
         // Add this entry
         sourceFileEntryCounts->Add(fileEntry->mSourceFilePath, 1);
         sourceFileEntries->Add(fileEntry);
      }
   }

   // Use greedy algo to determine 'best' set of common files.
   // Warning, the optimal set of files is a variation of the 0-1 knapsack problem (can be NP-hard)
   // http://en.wikipedia.org/wiki/Knapsack_problem#0-1_knapsack_problem
   System::Collections::Generic::List<CCommonFileEntry^>^ commonFileEntries = gcnew System::Collections::Generic::List<CCommonFileEntry^>();

   {
      for (int i = 0; i < sourceFileEntries->Count; ++i)
      {
         FileEntry^ fileEntry = sourceFileEntries[i];
         int entryCount = sourceFileEntryCounts[fileEntry->mSourceFilePath];
         if (entryCount > 1)
         {
            CCommonFileEntry^ commonFileEntry = gcnew CCommonFileEntry;
            commonFileEntry->mFileEntry = fileEntry;
            commonFileEntry->mDuplicatedSizeOverhead = entryCount;
            commonFileEntries->Add(commonFileEntry);
         }
      }
   }
   commonFileEntries->Sort();

   // Write 'XML' for common files
   // Entries are sorted in order of 'most savings to main archive' without consideration of space used in common archive, not optimal solution.
   {
      // Remove double extension
      System::String^ inputXmlFilenameNoExtensions = System::IO::Path::GetFileNameWithoutExtension(System::IO::Path::GetFileNameWithoutExtension(Main::sInputXMLFilename));
      System::String^ commonXmlFilename = System::String::Format("{0}\\{1}_common.xbarc.xml", System::IO::Path::GetDirectoryName(System::IO::Path::GetFullPath(Main::sInputXMLFilename)), inputXmlFilenameNoExtensions);
      System::IO::StreamWriter^ sw = System::IO::File::CreateText(commonXmlFilename);
      sw->WriteLine("<psarc>");
      sw->WriteLine(System::String::Format("\t<create archive=\"{0}_common.xbarc\" absolute=\"true\" overwrite=\"true\">", inputXmlFilenameNoExtensions));   
      int64 currentCompressedSize = 0;
      for (int i = 0; i < commonFileEntries->Count; ++i)
      {
         CCommonFileEntry^ commonFileEntry = commonFileEntries[i];
         currentCompressedSize += commonFileEntry->mFileEntry->mCompressedSize;
         if (currentCompressedSize > maxCommonArchiveSize)
         {
            // Adding this file will exceed our max common archive size, done!
            break;
         }
         // Use source filename for both entries as otherwise archive hash can't be used for looking common source files in game
         
         // Additions: Strip off /_bp/ folders, we use these for custom versions of .gcx and .tri files
         System::String^ bp_specific_stripped_SourceFilePath = commonFileEntry->mFileEntry->mSourceFilePath->Replace("/_bp/", "/");

         // Also strip off override paths: ovr_ps3, ovr_360, ovr_jp, ovr_us, ovr_eu.
         {
            bp_specific_stripped_SourceFilePath = bp_specific_stripped_SourceFilePath->Replace("/ovr_360/", "/");
            bp_specific_stripped_SourceFilePath = bp_specific_stripped_SourceFilePath->Replace("/ovr_ps3/", "/");
            bp_specific_stripped_SourceFilePath = bp_specific_stripped_SourceFilePath->Replace("/ovr_us/", "/");
            bp_specific_stripped_SourceFilePath = bp_specific_stripped_SourceFilePath->Replace("/ovr_eu/", "/");
            bp_specific_stripped_SourceFilePath = bp_specific_stripped_SourceFilePath->Replace("/ovr_jp/", "/");
         }

         sw->WriteLine(System::String::Format("\t\t<file path=\"{0}\" archivepath=\"{1}\" />",
            commonFileEntry->mFileEntry->mSourceFilePath,
            bp_specific_stripped_SourceFilePath
            ));
      }
      sw->WriteLine("\t</create>");
      sw->WriteLine("</psarc>");
      delete (System::IDisposable^) sw;

      System::Console::WriteLine("Info: Wrote common xml file: " + commonXmlFilename);
   }

}

//----------------------------------------------------------------------------
// Loads in 'XML' of common files to be excluded from main archive
static void LoadCommonFilesXML(System::String^ commonFilesXMLFilename)
{
   Main::sCommonSourceFilenames = gcnew System::Collections::Generic::Dictionary<System::String^, int>();
   System::Xml::XmlDocument^ xmlDocument = gcnew System::Xml::XmlDocument();
   xmlDocument->Load(commonFilesXMLFilename);

   System::Xml::XmlNodeList^ xmlCreateNodes = xmlDocument->SelectNodes("./psarc/create");
   for( int ii=0; ii < xmlCreateNodes->Count; ++ii )
   {
      System::Xml::XmlElement^ createNode = (System::Xml::XmlElement^)xmlCreateNodes[ii];
      System::Xml::XmlNodeList^ xmlFileNodes = createNode->SelectNodes("file");
      //foreach (XmlElement fileElement in xmlFileNodes)
      for( int jj=0; jj < xmlFileNodes->Count; ++jj )
      {
         System::Xml::XmlElement^ fileElement = (System::Xml::XmlElement^)xmlFileNodes[jj];
         System::String^ sourceFilename = fileElement->GetAttribute("path");
         Main::sCommonSourceFilenames->Add(sourceFilename, 0);  // Don't care about value (.NET needs a set)
      }
   }
}

//----------------------------------------------------------------------------
// Loads in 'XML' of common files to be excluded from main archive
static void LoadCommonFilesXML_ToExclude(System::String^ commonFilesXMLFilename)
{
   System::Collections::Generic::List<System::String^>^ listCommonSourceDirectories_ToExclude = gcnew System::Collections::Generic::List<System::String^>();

   System::Xml::XmlDocument^ xmlDocument = gcnew System::Xml::XmlDocument();
   xmlDocument->Load(commonFilesXMLFilename);

   System::Xml::XmlNodeList^ xmlCreateNodes = xmlDocument->SelectNodes("./psarc/create");
   for( int ii=0; ii < xmlCreateNodes->Count; ++ii )
   {
      System::Xml::XmlElement^ createNode = (System::Xml::XmlElement^)xmlCreateNodes[ii];
      System::Xml::XmlNodeList^ xmlFileNodes = createNode->SelectNodes("file");
      for( int jj=0; jj < xmlFileNodes->Count; ++jj )
      {
         System::Xml::XmlElement^ fileElement = (System::Xml::XmlElement^)xmlFileNodes[jj];
         System::String^ sourceFilename = fileElement->GetAttribute("path");
         listCommonSourceDirectories_ToExclude->Add(sourceFilename);
      }
   }

   Main::sCommonSourceDirectories_ToExclude = listCommonSourceDirectories_ToExclude->ToArray();
}

//----------------------------------------------------------------------------

static void CompressFile( System::String^ filePath )
{
   System::String^ compressedFilePath = GetCompressedFilename( filePath );

   if( !Main::sbCleanBuild && System::IO::File::Exists( compressedFilePath ) )
   {
      //skip files that are already compressed and newer.
      unsigned long long orgFileDate = BP_GetFileModificationDate( ConvertString( filePath ).c_str() );
      unsigned long long compressedFileDate = BP_GetFileModificationDate( ConvertString( compressedFilePath ).c_str() );
      if( compressedFileDate > orgFileDate )
      {
         return;
      }
   }

   CreateFullSubdirectories( compressedFilePath );

   const int blockSize = Main::sArchive->mBlockSize / 1024;

   System::String^ xedk = BP_GetEnvironmentVariable("XEDK");
   if( xedk == nullptr )
   {
      System::Console::WriteLine("Could not find environment variable XEDK (needed for xbcompress.exe)");
      throw false;
   }

   System::String^ processFileName = xedk + "\\bin\\win32\\xbcompress.exe";

   System::String^ processArguments = "";
   processArguments += " /q"; //Suppresses console output when files are compressed.
   processArguments += " /y"; //Does not prompt when it overwrites an existing file.

   processArguments += " /n:";
   processArguments += blockSize;

   System::Diagnostics::Process^ compressProcess = gcnew System::Diagnostics::Process();
   compressProcess->StartInfo->UseShellExecute = false;

   compressProcess->StartInfo->FileName = processFileName;
   compressProcess->StartInfo->Arguments = processArguments;
   compressProcess->StartInfo->Arguments += " " + filePath + " " + compressedFilePath;

   compressProcess->Start();
   compressProcess->WaitForExit();
   if (compressProcess->ExitCode != 0)
   {
      System::Console::WriteLine("xbcompress.exe failed: exit code " + compressProcess->ExitCode);
      throw false;
   }
}

//----------------------------------------------------------------------------

static void AddBlockSizes(FileEntry ^ fileEntry, std::vector< unsigned int > & refBlockSizeList )
{
   System::String^ sourceFilePath = fileEntry->mSourceFilePath;
   CompressFile( sourceFilePath );
   System::String^ compressedFilePath = GetCompressedFilename( sourceFilePath );
   //Open up the xbox-compressed file and read out the block sizes.
   XCOMPRESS_FILE_HEADER_LZXNATIVE fileHeader;
   FILE * fp = fopen( ConvertString( compressedFilePath ).c_str(), "rb" );
   if( !fp )
   {
      System::Console::WriteLine("Error opening file " + sourceFilePath);
      throw false;
   }
   fread( &fileHeader, sizeof( fileHeader ), 1, fp );
   ByteSwap_XCOMPRESS_FILE_HEADER_LZXNATIVE( fileHeader );
   if( fileHeader.Common.Identifier != XCOMPRESS_FILE_IDENTIFIER_LZXNATIVE )
   {
      System::Console::WriteLine("Error: bad xbox-compressed file! (" + sourceFilePath + ")" );
      throw false;
   }
   if( fileHeader.CompressedSizeHigh )
   {
      System::Console::WriteLine("TODO: file sizes greater than 32-bit! (" + sourceFilePath + ")" );
      throw false;
   }

   {
      // Initialize information about compression for this entry
      fileEntry->mSourceSize = fileHeader.UncompressedSizeLow;
      fileEntry->mCompressedSize = 0;
      fileEntry->mTotalBlockCount = 0;
      fileEntry->mCompressedBlockCount = 0;

      int remainingFileSize = fileHeader.CompressedSizeLow;
      int uncompressedFileOffset = 0;

      while( remainingFileSize )
      {
         //Format of the compressed blocks is a DWORD for the compressed block size
         //followed by the compressed data.
         unsigned int compressedBlockSize;
         fread( &compressedBlockSize, sizeof( compressedBlockSize ), 1, fp );
         remainingFileSize -= sizeof( unsigned int );
         NEndian::Swap4Bytes( &compressedBlockSize );

         fileEntry->mTotalBlockCount++;

         int uncompressedBlockSize = fileHeader.UncompressedSizeLow - uncompressedFileOffset;
         if( uncompressedBlockSize > Main::sArchive->mBlockSize )
         {
            uncompressedBlockSize = Main::sArchive->mBlockSize;
         }

         if( compressedBlockSize >= uncompressedBlockSize )
         {
            //We will be taking the uncompressed data for this block because compression did not reduce the size.
            //0 indicates uncompressed data, either an entire uncompressed block size or the remainder at the end
            //of a file.
            refBlockSizeList.push_back( 0 );

            fileEntry->mCompressedSize += uncompressedBlockSize;
         }
         else
         {
            //Take the compressed block.
            refBlockSizeList.push_back( compressedBlockSize );

            fileEntry->mCompressedSize += compressedBlockSize;
            fileEntry->mCompressedBlockCount++;
         }
         //Seek to the next block size / compressed block data.
         fseek( fp, compressedBlockSize, SEEK_CUR );
         remainingFileSize -= compressedBlockSize;
         if( remainingFileSize < 0 )
         {
            System::Console::WriteLine("Error: block size inconsistency in compressed file! (" + sourceFilePath + ")" );
            throw false;
         }

         uncompressedFileOffset += Main::sArchive->mBlockSize;
      }
   }
   fclose( fp );
}

//----------------------------------------------------------------------------

static void AddArchiveFile( FILE * outFp, System::String^ filePath )
{
   System::String^ compressedFilePath = GetCompressedFilename( filePath );
   //Open up the xbox-compressed file and read out the compressed data block.
   XCOMPRESS_FILE_HEADER_LZXNATIVE fileHeader;
   FILE * compressedFp = fopen( ConvertString( compressedFilePath ).c_str(), "rb" );
   if( !compressedFp )
   {
      System::Console::WriteLine("Error opening file " + compressedFilePath);
      throw false;
   }
   FILE * uncompressedFp = fopen( ConvertString( filePath ).c_str(), "rb" );
   if( !uncompressedFp )
   {
      System::Console::WriteLine("Error opening file " + filePath);
      throw false;
   }

   fread( &fileHeader, sizeof( fileHeader ), 1, compressedFp );
   ByteSwap_XCOMPRESS_FILE_HEADER_LZXNATIVE( fileHeader );
   if( fileHeader.Common.Identifier != XCOMPRESS_FILE_IDENTIFIER_LZXNATIVE )
   {
      System::Console::WriteLine("Error: bad xbox-compressed file! (" + compressedFilePath + ")" );
      throw false;
   }
   if( fileHeader.CompressedSizeHigh )
   {
      System::Console::WriteLine("TODO: file sizes greater than 32-bit! (" + compressedFilePath + ")" );
      throw false;
   }

   unsigned char * buf = (unsigned char*)_alloca( Main::sArchive->mBlockSize );

   {
      int remain = fileHeader.CompressedSizeLow;
      int uncompressedFileOffset = 0; //read / write offset wrt original uncompressed file.
      while( remain )
      {
         //Format of the compressed blocks is a DWORD for the compressed block size
         //followed by the compressed data.
         unsigned int compressedBlockSize;
         fread( &compressedBlockSize, sizeof( compressedBlockSize ), 1, compressedFp );
         remain -= sizeof( unsigned int );
         NEndian::Swap4Bytes( &compressedBlockSize );

         int uncompressedBlockSize = fileHeader.UncompressedSizeLow - uncompressedFileOffset;
         if( uncompressedBlockSize > Main::sArchive->mBlockSize )
         {
            uncompressedBlockSize = Main::sArchive->mBlockSize;
         }

         if( compressedBlockSize >= uncompressedBlockSize )
         {
            //Insert the uncompressed data for this block because compression did not reduce the size.
            fseek( uncompressedFp, uncompressedFileOffset, SEEK_SET );
            int n = fread( buf, 1, uncompressedBlockSize, uncompressedFp );
            if( n != uncompressedBlockSize )
            {
               System::Console::WriteLine("Error reading file " + filePath);
               throw false;
            }
            n = fwrite( buf, 1, uncompressedBlockSize, outFp );
            if( n != uncompressedBlockSize )
            {
               System::Console::WriteLine("Error writing archive file!" );
               throw false;
            }
            //Skip over the compressed block data we didn't use.
            fseek( compressedFp, compressedBlockSize, SEEK_CUR );
         }
         else
         {
            //Insert the compressed data.
            int n = fread( buf, 1, compressedBlockSize, compressedFp );
            if( n != compressedBlockSize )
            {
               System::Console::WriteLine("Error reading file " + compressedFilePath);
               throw false;
            }
            n = fwrite( buf, 1, compressedBlockSize, outFp );
            if( n != compressedBlockSize )
            {
               System::Console::WriteLine("Error writing archive file!" );
               throw false;
            }
         }

         remain -= compressedBlockSize;
         if( remain < 0 )
         {
            System::Console::WriteLine("Error: block size inconsistency in compressed file! (" + compressedFilePath + ")" );
            throw false;
         }
         uncompressedFileOffset += Main::sArchive->mBlockSize;
      }
   }
   fclose( uncompressedFp );
   fclose( compressedFp );
}

//----------------------------------------------------------------------------

static void WriteOutFileEntries()
{
   SArchiveHeader header =
   {
      skMagicNumber,
      skVersionNumber,
      Main::sArchive->mFileEntries->Count,
      Main::sArchive->mBlockSize,
      0
   };

   std::vector< SArchiveTOCEntry >   tocEntries;
   tocEntries.resize( Main::sArchive->mFileEntries->Count );

   //File format similar to psarc:
   // Header
   // TOC
   // Block size list
   // Compressed blocks (file data)

   //Build block size list and fill in TOC entry (except for file start offset) for each file.

   //N.B. psarc is clever and uses fewer bytes for block size entries depending on uncompressed
   //block size: 2 bytes for <= 64KB and 3 bytes for <=16MB.  Would save a few hundred K I guess.
   std::vector< unsigned int >   blockSizeList;
   blockSizeList.reserve( 64 * 1024 );
   for( int i=0;i<Main::sArchive->mFileEntries->Count;++i )
   {
      FileEntry^ fileEntry = Main::sArchive->mFileEntries[i];
      SArchiveTOCEntry & tocEntry = tocEntries[i];

      std::string filePath( ConvertString( fileEntry->mSourceFilePath ) );
      std::string archivePath( ConvertString( fileEntry->mArchivePath ) );

      for( int b=0; b < sizeof( tocEntry.mArchivePathHash ); ++b )
      {
         tocEntry.mArchivePathHash[b] = fileEntry->mArchivePathHash[b];
      }

      tocEntry.mBlockListStartIndex = blockSizeList.size();
      tocEntry.mOriginalSize = BP_GetFileSizeAttr( filePath.c_str() );
      tocEntry.mStartOffset = 0;

      printf("Indexing [%6d/%6d] %s\n", i+1, Main::sArchive->mFileEntries->Count, archivePath.c_str() );
      AddBlockSizes( fileEntry, blockSizeList );
   }

   if (Main::sbWriteStats)
   {
      WriteArchiveFileStats();
   }

   if (Main::sbWriteCommonFileXML)
   {
      // Need to indexing step above to determine file counts and compressed sizes
      WriteCommonFilesXML(Main::sCommonFilesMaxArchiveSize);
      // Don't create archive as the user want to run this again, skipping those common files
      return;
   }

   header.mBlockCount = blockSizeList.size();

   std::string arcFilename( ConvertString( Main::sArchive->mArchiveFilename ) );
   FILE * outFp = fopen( arcFilename.c_str(), "wb" );
   if( !outFp )
   {
      printf("Failed to open file %s\n", arcFilename.c_str());
      throw false;
   }

   //Write out header then placeholder TOC.
   fwrite( &header, sizeof( header ), 1, outFp );
   fwrite( &tocEntries[0], tocEntries.size() * sizeof( tocEntries[0] ), 1, outFp );

   //Write out completed block size list.
   for( int i=0; i < blockSizeList.size(); ++i )
   {
      NEndian::Swap4Bytes( &blockSizeList[i] );
   }
   fwrite( &blockSizeList[0], blockSizeList.size() * sizeof( blockSizeList[0] ), 1, outFp );

   //Write out compressed file data and fill in file start offsets in the TOC entries.
   for( int i=0;i<Main::sArchive->mFileEntries->Count;++i )
   {
      FileEntry^ fileEntry = Main::sArchive->mFileEntries[i];
      SArchiveTOCEntry & tocEntry = tocEntries[i];

      std::string filePath( ConvertString( fileEntry->mSourceFilePath ) );
      std::string archivePath( ConvertString( fileEntry->mArchivePath ) );
      printf("Archiving [%6d/%6d] %s\n", i+1, Main::sArchive->mFileEntries->Count, archivePath.c_str() );

      tocEntry.mStartOffset = _ftelli64( outFp );

      ByteSwap_TOCEntry( tocEntry );

      AddArchiveFile( outFp, fileEntry->mSourceFilePath );
   }

   //Sort by hash then rewrite final header and TOC at the head of the file.
   std::sort( tocEntries.begin(), tocEntries.end() );
   fseek( outFp, 0, SEEK_SET );
   ByteSwap_ArchiveHeader( header );
   fwrite( &header, sizeof( header ), 1, outFp );
   fwrite( &tocEntries[0], tocEntries.size() * sizeof( tocEntries[0] ), 1, outFp );

   //Done!
   fclose( outFp );
}

//----------------------------------------------------------------------------

static void LoadXMLFile()
{
   Main::sXMLDocument = gcnew System::Xml::XmlDocument();
   Main::sXMLDocument->Load(Main::sInputXMLFilename);
   
   System::Xml::XmlNodeList^ xmlCreateNodes = Main::sXMLDocument->SelectNodes("./psarc/create");
   //foreach (XmlElement createNode in xmlCreateNodes)
   for( int ii=0; ii < xmlCreateNodes->Count; ++ii )
   {
      System::Xml::XmlElement^ createNode = (System::Xml::XmlElement^)xmlCreateNodes[ii];
      System::String^ archiveName = createNode->GetAttribute("archive");
      Main::sArchive = gcnew Archive();
      Main::sArchive->mArchiveFilename = archiveName;

      System::Xml::XmlNodeList^ xmlFileNodes = createNode->SelectNodes("file");
      //foreach (XmlElement fileElement in xmlFileNodes)
      for( int jj=0; jj < xmlFileNodes->Count; ++jj )
      {
         System::Xml::XmlElement^ fileElement = (System::Xml::XmlElement^)xmlFileNodes[jj];
         System::String^ path = fileElement->GetAttribute("path");
         System::String^ archivepath = fileElement->GetAttribute("archivepath");
         // Set if the source filename is part of our common set
         if( (Main::sCommonSourceFilenames != nullptr) && (Main::sCommonSourceFilenames->ContainsKey(path)) )
         {
            bool directoryExcluded = false;
            if( Main::sCommonSourceDirectories_ToExclude )
            {
               for( int kk=0; kk<Main::sCommonSourceDirectories_ToExclude->Length; ++kk )
               {
                  System::String^ dir = Main::sCommonSourceDirectories_ToExclude[kk];
                  if( archivepath->Contains(dir) )
                  {
                     directoryExcluded = true;
                     break;
                  }
               }
            }
            if( !directoryExcluded )
            {
               // This file is in the common archive, skip
               System::Console::WriteLine("Info: ignoring common file for " + archivepath );
               continue;
            }
         }

         //process fileElement, in the future we might expand options here for wildcards, exclusions etc
         FileEntry^ temp = gcnew FileEntry();
         temp->mSourceFilePath = path;
         temp->mArchivePath = archivepath;
         temp->mSourceSize = 0;
         temp->mCompressedSize = 0;
         temp->mTotalBlockCount = 0;
         temp->mCompressedBlockCount = 0;
         temp->mFileCount = 0;
         Main::sArchive->AddEntry( temp );
      }
      Main::sArchive->mBlockSize = 128 * 1024;   // Use 128KB blocks for better compression

      WriteOutFileEntries();
   }
}

//----------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   array<System::String ^> ^args = gcnew array<System::String ^>(0);
   if( argc <  2)
   {
      ShowUsage();
      return 1;
   }

   if (ProcessArgs(argc, argv))
   {
      return 1;
   }

   try
   {
      if (Main::sCommonFilesXMLFilename)
      {
         LoadCommonFilesXML(Main::sCommonFilesXMLFilename);
         if( Main::sCommonFilesXMLFilename_ToExclude)
         {
            LoadCommonFilesXML_ToExclude(Main::sCommonFilesXMLFilename_ToExclude);
         }
      }
      LoadXMLFile();
   }
   catch( System::Exception^ e)
   {
      System::Console::WriteLine("Archive build failed: " + e->ToString() );
      return -1;
   }
   catch(...)
   {
      System::Console::WriteLine("Archive build failed" );
      return -1;
   }
   return 0;
}

//----------------------------------------------------------------------------
