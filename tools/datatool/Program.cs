using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace datatool
{
   class CachedXmlParent
   {
      public CachedXmlParent( XmlElement element, bool enableCache )
      {
         mElement = element;
         mEnableCache = enableCache;
      }

      public void AppendChild( XmlNode child )
      {
         if ( mEnableCache )
         {
            String childXml = child.OuterXml;

            if ( !mCache.ContainsKey( childXml ) )
            {
               mCache.Add( childXml, true );
               mElement.AppendChild( child );
            }
         }
         else
         {
            mElement.AppendChild( child );
         }
      }

      public XmlDocument OwnerDocument { get { return mElement.OwnerDocument; } }

      XmlElement mElement;
      bool mEnableCache;
      Dictionary<String, bool> mCache = new Dictionary<String, bool>();
   };

   static class ListMerger
   {
      static public bool DebugMode = false;

      static bool FindFirstLikeSeq( List<String> lhs, List<string> rhs, int leftFirst, int rightFirst, out int lhsStart, out int rhsStart, out int seqSize )
      {
         if ( DebugMode )
         {
            Console.WriteLine( "lf {0} rf {1}", leftFirst, rightFirst );
         }

         for ( int left = leftFirst; left < lhs.Count; ++left )
         {
            for ( int right = rightFirst; right < rhs.Count; ++right )
            {
               if ( lhs[left] == rhs[right] )
               {
                  // Ok, we have a place to start.
                  lhsStart = left;
                  rhsStart = right;

                  int max = Math.Min( lhs.Count - left, rhs.Count - right );

                  for ( seqSize = 1; seqSize < max; ++seqSize )
                  {
                     if ( lhs[left + seqSize] != rhs[right + seqSize] )
                     {
                        // Stop when the sequence fails
                        break;
                     }
                  }

                  if ( DebugMode )
                  {
                     Console.WriteLine( "ls {0} rs {1} ss {2}", lhsStart, rhsStart, seqSize );
                  }

                  return true;
               }
            }
         }

         if ( DebugMode )
         {
            Console.WriteLine( "DONE" );
         }

         lhsStart = rhsStart = seqSize = -1;
         return false;
      }

      private class DiffListStringList : DifferenceEngine.IDiffList
      {
         public DiffListStringList( List<String> list )
         {
            mList = list;
         }

         List<String> mList;

         #region IDiffList Members

         public int Count()
         {
            return mList.Count;
         }

         public IComparable GetByIndex( int index )
         {
            return mList[index];
         }

         #endregion

      }

      static public List<String> MergeLists_Smart( List<String> lhs, List<String> rhs )
      {
         DifferenceEngine.DiffEngine engine = new DifferenceEngine.DiffEngine();

         engine.ProcessDiff( new DiffListStringList( lhs ), new DiffListStringList( rhs ), DifferenceEngine.DiffEngineLevel.SlowPerfect );

         List<String> result = new List<String>();

         foreach ( DifferenceEngine.DiffResultSpan span in engine.DiffReport() )
         {
            switch ( span.Status )
            {
            case DifferenceEngine.DiffResultSpanStatus.DeleteSource:
            case DifferenceEngine.DiffResultSpanStatus.NoChange:
               result.AddRange( lhs.GetRange( span.SourceIndex, span.Length ) );
               break;
            case DifferenceEngine.DiffResultSpanStatus.AddDestination:
               result.AddRange( rhs.GetRange( span.DestIndex, span.Length ) );
               break;
            case DifferenceEngine.DiffResultSpanStatus.Replace:
               result.AddRange( lhs.GetRange( span.SourceIndex, span.Length ) );
               result.AddRange( rhs.GetRange( span.DestIndex, span.Length ) );
               break;
            }
         }

         return result;
      }

      static public List<String> MergeLists( List<String> lhs, List<String> rhs )
      {
         return MergeLists_Dumb( lhs, rhs );
      }

      static public List<String> MergeLists_Dumb( List<String> lhs, List<String> rhs )
      {
         List<String> ret = new List<String>();

         int leftAt = 0;
         int rightAt = 0;

         if ( DebugMode )
         {
            Console.WriteLine( "lhs:" );
            for ( int i = 0; i < lhs.Count; ++i )
            {
               Console.WriteLine( " {0}: {1}", i, lhs[i] );
            }

            Console.WriteLine( "rhs:" );
            for ( int i = 0; i < rhs.Count; ++i )
            {
               Console.WriteLine( " {0}: {1}", i, rhs[i] );
            }
         }

         while ( true )
         {
            int lhsStart, rhsStart, seqSize;

            if ( FindFirstLikeSeq( lhs, rhs, leftAt, rightAt, out lhsStart, out rhsStart, out seqSize ) )
            {
               // We have a sequence.  Let's flush everything up to that sequence left then right
               ret.AddRange( lhs.GetRange( leftAt, lhsStart - leftAt ) );
               ret.AddRange( rhs.GetRange( rightAt, rhsStart - rightAt ) );

               // Now let's flush the sequence
               ret.AddRange( lhs.GetRange( lhsStart, seqSize ) );

               // Now we update our left and right pointers
               leftAt = lhsStart + seqSize;
               rightAt = rhsStart + seqSize;
            }
            else
            {
               // No sequences left.  Let's flush what's left and go away
               if ( leftAt < lhs.Count )
               {
                  ret.AddRange( lhs.GetRange( leftAt, lhs.Count - leftAt ) );
               }

               if ( rightAt < rhs.Count )
               {
                  ret.AddRange( rhs.GetRange( rightAt, rhs.Count - rightAt ) );
               }

               break;
            }
         }

         if ( DebugMode )
         {
            Console.WriteLine( "Result:" );
            for ( int i = 0; i < ret.Count; ++i )
            {
               Console.WriteLine( " {0}: {1}", i, ret[i] );
            }
         
         }

         return ret;
      }
   }

   class AbortException : Exception
   {
      public AbortException( String s, params object[] o )
         : base( String.Format( s, o ) )
      {
      }

      public AbortException( String s )
         : base( s )
      {
      }
   }

   class UsageException : Exception
   {
      public UsageException( String s, params object[] o )
         : base( String.Format( s, o ) )
      {
      }

      public UsageException( String s )
         : base( s )
      {
      }
   }

   struct SCommand 
   {
      public enum EType
      {
         // DO NOT RENAME THESE ENUMS
         // These are matching the enum names that are in text
         // files.  So if you rename these, the layout files 
         // won't work!

         slot,
         stage,
         dat,
         dat_lst
      }

      public string mFolder;
      public EType mType;
   }

   class CommandLine
   {
      public CommandLine( string[] _args )
      {
         List<String> realArgs = new List<String>();
         foreach ( String a in _args )
         {
            if ( a.StartsWith( "@" ) )
            {
               foreach ( String line in File.ReadAllLines( a.Substring( 1 ) ) )
               {
                  foreach ( String part in line.Split( new char[] { ' ', '\t' } ) )
                  {
                     if ( part != "" )
                     {
                        realArgs.Add( part );
                     }
                  }
               }
            }
            else
            {
               realArgs.Add( a );
            }
         }

         String[] args = realArgs.ToArray();
         for ( int argIndex = 0; argIndex != args.Length; /* increment in loop */ )
         {
            switch ( args[argIndex].ToLower() )
            {
            case "-p":
               mPlatform = args[argIndex + 1];
               argIndex += 2;
               break;
            case "-r":
               mRegions.Add( args[ argIndex + 1 ] );
               argIndex += 2;
               break;
            case "-s":
               mSKU = args[argIndex + 1];
               argIndex += 2;
               break;
            case "-o":
               mOutput = args[argIndex + 1];
               argIndex += 2;
               break;
            case "-f":
               mFilterFile = args[argIndex + 1];
               mFilterStartString = args[argIndex + 2];
               argIndex += 3;
               break;
            case "-2":
               mMGSVersion = 2;
               ++argIndex;
               break;
            case "-3":
               mMGSVersion = 3;
               ++argIndex;
               break;
            case "-u":
               mOutputUnifiedPaths = true;
               ++argIndex;
               break;
            case "--keepDupes":
               mKeepDupes = true;
               ++argIndex;
               break;
            default:
               if ( !args[argIndex].Contains( ',' ) )
               {
                  throw new UsageException( "Unknown arg {0}", args[argIndex] );
               }
               else
               {
                  String[] commands = args[argIndex].Split( ',' );
                  SCommand command = new SCommand();

                  command.mFolder = commands[0];
                  command.mType = (SCommand.EType) Enum.Parse( typeof( SCommand.EType ), commands[1] );

                  mFolders.Add( command );
               }

               ++argIndex;
               break;
            }
         }

         if ( mPlatform == null )
         {
            throw new UsageException( "No platform specified" );
         }

         if( mSKU == null )
         {
            throw new UsageException("No sku specified");
         }

         if (mRegions.Count == 0)
         {
            throw new UsageException( "No input region specified" );
         }

         if ( mOutput == null )
         {
            throw new UsageException( "No output specified" );
         }

         if ( mMGSVersion != 2 && mMGSVersion != 3 )
         {
            throw new UsageException( "MGS version {0} is invalid", mMGSVersion );
         }

         if ( mFolders.Count == 0 )
         {
            throw new UsageException( "No input folders specified" );
         }

         if ( mFilterFile != null )
         {
            if ( !File.Exists( mFilterFile ) )
            {
               throw new UsageException( "Filter file '{0}' not found", mFilterFile );
            }
         }

      }

      public readonly String mPlatform = null;
      public readonly String mSKU = null;
      public readonly List<String> mRegions = new List<String>();
      public readonly List< SCommand > mFolders = new List<SCommand>();
      public readonly String mOutput = null;
      public readonly int mMGSVersion = -1;
      public readonly bool mOutputUnifiedPaths = false;
      public readonly bool mKeepDupes = false;

      public readonly String mFilterFile = null;
      public readonly String mFilterStartString = null;
   }

   class Program
   {
      static void ParsePSP2Arc( String filename )
      {
         try
         {
            foreach ( String line in System.IO.File.ReadAllLines( filename ) )
            {
               int space = line.IndexOf( ' ' );
               int openparen = line.IndexOf( '(' );

               if ( space == -1 || openparen == -1 || openparen < space )
                  continue;

               int slashafterparen = line.IndexOf( '/', openparen );
               int spaceafterparen = line.IndexOf( ' ', openparen );

               if ( slashafterparen == -1 || spaceafterparen == -1 || slashafterparen > spaceafterparen )
                  continue;

               // Chop up the string
               String pathpart = line.Substring( 0, space );
               String compressedPart = line.Substring( openparen + 1, slashafterparen - openparen - 1 );
               String uncompressedPart = line.Substring( slashafterparen + 1, spaceafterparen - slashafterparen - 1 );

               int lastPathSlash = pathpart.LastIndexOf( '/' );
               if ( lastPathSlash == -1 )
                  continue;

               String dirpart = pathpart.Substring( 0, lastPathSlash );
               String filenamepart = pathpart.Substring( lastPathSlash + 1 );

               Console.WriteLine( "{0} {1} {2} {3}",
                  dirpart,
                  filenamepart,
                  compressedPart,
                  uncompressedPart );
            }
         }
         catch ( Exception e )
         {
            Console.WriteLine( e.Message );
         }
      }

      static int ParseDepends( String archiveXmlFilename )
      {
         XmlDocument doc = new XmlDocument();

         doc.Load( archiveXmlFilename );

         XmlElement mainContainer = (XmlElement)doc.SelectSingleNode( "psarc/create" );

         String archiveName = mainContainer.GetAttribute( "archive" );
         if ( !File.Exists( archiveName ) )
         {
            // File not there. Needs cooking.
            Console.WriteLine( "Archive {0} - doesn't exist.", archiveName );
            return 1;
         }

         DateTime archiveDate = File.GetLastWriteTime( archiveName );
         DateTime archiveXmlDate = File.GetLastWriteTime( archiveXmlFilename );

         if ( archiveXmlDate > archiveDate )
         {
            // Archive XML is newer
            Console.WriteLine( "Archive {0} - {1} is newer.", archiveName, archiveXmlFilename );

            return 1;
         }

         foreach ( XmlElement fileElement in mainContainer.SelectNodes( "file" ) )
         {
            String filepath = fileElement.GetAttribute( "path" );

            if ( !File.Exists( filepath ) )
            {
               Console.WriteLine( "Archive {0} - {1} doesn't exist.", archiveName, filepath );
               
               return 1;
            }

            DateTime fileDate = File.GetLastWriteTime( filepath );

            if ( fileDate > archiveDate )
            {
               Console.WriteLine( "Archive {0} - {1} is newer.", archiveName, filepath );

               return 1;
            }
         }

         Console.WriteLine( "Archive {0} - up-to-date.", archiveName );
         return 0;
      }

      static void PrintUsage()
      {
         Console.WriteLine(
            @"
Creates psarc/xbarc XML files used for generating archives.

Usage:
datatool <-2|-3> -r <region> [-r <region> ...] -o <outputfile> 
         -p <platform> -s <sku> <folder,type> [folder,type...]
         [--keepDupes] [-u]
         [ -f <filterfile> <filterstring> ]

-2 | -3       - -2 for MGS2, -3 for MGS3
<region>      - Specifies one or more regions to pack into an archive
<outputfile>  - Where to write the archive XML
<platform>    - Platform for the xml, currently _ps3, _360, or _vta
<sku>         - SKU for the xml, currently us/eu/jp
<folder,type> - Folder is a subfolder in a region to parse
                type is one of slot,stage,dat, or dat_lst
-f            - Lets you filter the archive output by only including
                assets in a text file (filterfile).  Only reads lines
                starting with filterstring and chops off filterstring
                when comparing with archive files to write
--keepDupes   - Keeps duplicate entries in the generated archive xml
-u            - Generate archive paths using unified paths.

Or...
datatool -psp2parse <filename>

Parses the output of a psp2archive list into something readable by excel

Or...
datatool -depends <archive xml>

Opens 'archive xml' and returns ERRORLEVEL 1 if the archive needs rebuilding.

" );
      }

      static Dictionary<String, bool> GetFileFilters( CommandLine cmd )
      {
         // If there is nothing to filter, this returns no dictionary
	  
         if ( cmd.mFilterFile == null )
         {
            return null;
         }

         // If there is a file, then each entry in the dictionary
		 // corresponds to a line relative to GameData (while should be what
		 // cmd.mFilterStartString represents

         String match = cmd.mFilterStartString.ToLower();
         Dictionary<String,bool> ret = new Dictionary<String, bool>();

         foreach ( String line in File.ReadAllLines( cmd.mFilterFile ) )
         {
            String lineLower = line.ToLower();

            if ( lineLower.StartsWith( match ) )
            {
               String clippedLine = lineLower.Substring( match.Length );

               if (!ret.ContainsKey(clippedLine))
               {
                  ret.Add(clippedLine, true);
               }
            }
         }

         return ret;
      }

      static bool PassesFilter( Dictionary<String, bool> filters, String fname, String platform )
      {
         // No filters always pass
	  
         if ( filters == null )
         {
            return true;
         }

         bool isTexture = fname.EndsWith( ".ctxr" );
         bool isModel = fname.EndsWith( ".cmdl" );

         if ( !isTexture && !isModel )
         {
            // If it's not a bluepoint asset, then there's no translation
			// what's it p4 is what's packaged
            return filters.ContainsKey( fname );
         }
         else
         {
            // If it's a bluepoint asset, we need to translate the cooked asset path
			// back to a file in perforce to check the mapping

            // Split the path into an array
            List<String> split = new List<String>( fname.Split( new char[] { '/' } ) );

            // Make sure that the file is in a platform folder (and the one we expect)
            if ( split[split.Count - 2] != platform )
            {
               throw new AbortException( "Error splitting '{0}' for platform {1}", fname, platform );
            }

            // Remove the platform folder and reassemble the string
            split.RemoveAt( split.Count - 2 );
            String platformRemoved = String.Join( "/", split.ToArray() );
			
			// Remove the extension (5 chars for .ctxr or .cmdl)
            String noExt = platformRemoved.Substring( 0, platformRemoved.Length - 5 );

            if ( isTexture )
            {
			   // If we're a texture, we match if either the tga or tga.meta is updated
               return
                  filters.ContainsKey( String.Format( "{0}.tga", noExt ) ) ||
                  filters.ContainsKey( String.Format( "{0}.tga.meta", noExt ) );
            }
            else
            {
			   // If we're a model, we match if the bmdl is updated
               return filters.ContainsKey( String.Format( "{0}.bmdl", noExt ) );
            }
         }
      }
      
      static bool GenerateXmlForRegions( CommandLine commandLine )
      {
         XmlDocument doc = new XmlDocument();

         Console.WriteLine( "Generating archive XML: {0}", commandLine.mOutput );
         
         Dictionary<String,bool> filters = GetFileFilters( commandLine );

         doc.AppendChild( doc.CreateElement( "psarc" ) );

         if ( !commandLine.mOutput.ToLower().EndsWith( ".xml" ) )
         {
            throw new AbortException( "Output file must end with .?barc.xml: {0}", commandLine.mOutput );
         }
         String archiveFile = commandLine.mOutput.Substring( 0, commandLine.mOutput.Length - ".xml".Length );

         XmlElement _createElement = (XmlElement)doc.DocumentElement.AppendChild( doc.CreateElement( "create" ) );
         _createElement.SetAttribute( "archive", System.IO.Path.GetFileName(archiveFile) );
         _createElement.SetAttribute( "absolute", "true" );
         _createElement.SetAttribute( "overwrite", "true" );

         if (commandLine.mOutputUnifiedPaths)
            _createElement.SetAttribute("mergedups", "true");

         Dictionary< String, List<String> > remainingFolders = new Dictionary<String, List<String>>();
         List<String> bpStreamsFiles = new List<String>();
         CachedXmlParent createElement = new CachedXmlParent( _createElement, !commandLine.mKeepDupes );

         foreach ( String region in commandLine.mRegions )
         {
            List<String> regionFolders = new List<String>();

            foreach ( SCommand command in commandLine.mFolders )
            {
               switch ( command.mType )
               {
               case SCommand.EType.slot:
                  {
                     // Slot file, write pages file
                     String pagesFileForRegion = String.Format( "{0}/{1}/pages.txt", region, command.mFolder );
                     AddFile_One( filters, commandLine.mPlatform, commandLine.mSKU, createElement, pagesFileForRegion );

                     foreach ( String line in File.ReadAllLines( pagesFileForRegion ) )
                     {
                        String folderName = line.Trim().Split( new char[] { ' ' } )[0];
                        if ( folderName != "" )
                        {
                           regionFolders.Add( String.Format( "{0}/{1}", command.mFolder, folderName ) );
                        }
                     }
                  }
                  break;
               case SCommand.EType.stage:
                  {
                     String stagesFileForRegion = String.Format( "{0}/{1}/stages.txt", region, command.mFolder );

                     foreach ( String line in File.ReadAllLines( stagesFileForRegion ) )
                     {
                        String folderName = line.Trim();
                        if ( folderName != "" )
                        {
                           String toAdd = String.Format( "{0}/{1}", command.mFolder, folderName );

                           if ( !regionFolders.Contains( toAdd ) )
                           {
                              regionFolders.Add( toAdd );
                           }
                        }
                     }
                  }
                  break;
               case SCommand.EType.dat:
               case SCommand.EType.dat_lst:
                  {
                     String bpStreams = String.Format( "{0}/{1}/_bp/{2}/bp_streams.txt",
                        region, command.mFolder, commandLine.mPlatform );

                     // If it's dat, then we also run through these streams
                     // If it's dat_lst, then we just include the list file, not the streams 
                     // themselves
                     if ( command.mType == SCommand.EType.dat )
                     {
                        bpStreamsFiles.Add( bpStreams );
                     }
                     else
                     {
                        Console.WriteLine( "Adding stream lst {0}", bpStreams );
                     }

                     AddFile_One( filters, commandLine.mPlatform, commandLine.mSKU, createElement, bpStreams );
                  }
                  break;
               }
            }

            if ( regionFolders.Count > 0 )
            {
               remainingFolders.Add( region, regionFolders );
            }
         }

         // Now we add the DAT assets
         {
            Dictionary< String,bool > alreadyInsertedSdt = new Dictionary<String,bool>();

            foreach ( String bpStream in bpStreamsFiles )
            {
               Console.WriteLine( "Adding stream {0}", bpStream );

               foreach ( String line in File.ReadAllLines( bpStream ) )
               {
                  if ( line != "" )
                  {
                     String[] parsed = line.Trim().Split( ' ' );

                     if ( parsed[0].Length != 10 )
                     {
                        throw new AbortException( "Line parsed from {0} doesn't start right:\n{1}\n", bpStream, line );
                     }

                     if ( !alreadyInsertedSdt.ContainsKey( parsed[1] ) )
                     {
                        AddFile_One( filters, commandLine.mPlatform, commandLine.mSKU, createElement, parsed[1] );
                        alreadyInsertedSdt.Add( parsed[1], true );
                     }
                  }
               }
            }
         }

         // Now that we have a dictionary of substages by region, we need to combine them.
         List< List<String> > stages = new List<List<String>>();

         while ( remainingFolders.Count > 0 )
         {
            String region = remainingFolders.Keys.ElementAt( 0 );
            List<String> regionFolders = remainingFolders[region];
            String folder = regionFolders[0];

            if ( regionFolders.Count == 1 )
            {
               remainingFolders.Remove( region );
            }
            else
            {
               regionFolders.RemoveAt( 0 );
            }

            // Ok, so "region" the region we're looking at, and "folder" is 
            // the one in that region we're looking at
            List<String> stageFolders = new List<String>();
            stageFolders.Add( String.Format( "{0}/{1}", region, folder ) );

            foreach ( String otherRegion in new List<String>( remainingFolders.Keys ) )
            {
               if ( otherRegion == region )
                  continue;

               List<String> otherRegionFolders = remainingFolders[otherRegion];
               int otherFolderIndex = otherRegionFolders.IndexOf( folder );

               if ( otherFolderIndex != -1 )
               {
                  stageFolders.Add( String.Format( "{0}/{1}", otherRegion, folder ) );

                  if ( otherRegionFolders.Count == 1 )
                  {
                     remainingFolders.Remove( otherRegion );
                  }
                  else
                  {
                     otherRegionFolders.RemoveAt( otherFolderIndex );
                  }
               }
            }

            stages.Add( stageFolders );
         }

         // Ok, now "stages" contains a list where each items is a stage that has assets that should be grouped together.
         ProcessStages( commandLine, filters, stages, createElement );

         doc.Save( commandLine.mOutput );

         return true;
      }

      static String[] ProtectedReadAllLines( String file )
      {
         if ( !File.Exists( file ) )
         {
            return new String[] {};
         }
         else
         {
            return File.ReadAllLines( file );
         }
      }

      static List<String> GatherPathsForStage( String stage )
      {
         List<String> manifestLines = new List<String>();
         List<String> ret = new List<String>();

         manifestLines.AddRange( ProtectedReadAllLines( String.Format( "{0}/bp_assets.txt", stage ) ) );
         manifestLines.AddRange( ProtectedReadAllLines( String.Format( "{0}/manifest.txt", stage ) ) );

         foreach ( String line in manifestLines )
         {
            if ( line != "" )
            {
               String[] fixedLine = line.Trim().Split( new char[] { ',' } );

               if ( fixedLine.Length <= 2 )
               {
                  throw new AbortException( "Line: {0} - Expected at least length 2, not {1}", line, fixedLine.Length );
               }

               ret.Add( String.Format( "{0},{1}", fixedLine[0], fixedLine[1] ) );
            }
         }

         return ret;
      }

      static String GetExt( String s )
      {
         int lastDot = s.LastIndexOf( '.' );
         if ( lastDot == -1 )
         {
            return "";
         }
         else
         {
            return s.Substring( lastDot + 1 ).ToLower();
         }
      }

      static String AddPlatform( String path, String platform )
      {
         int lastSlash = path.LastIndexOf( '/' );

         return String.Format( "{0}/{1}{2}", path.Substring( 0, lastSlash ), platform, path.Substring( lastSlash ) );
      }

      static String GetFilenameWithPlatformFolder(string fullPath, String platform)
      {
         String result;

         int lastSlash = fullPath.LastIndexOf('/');
         
         if( lastSlash > 0 )
         {
            result = fullPath.Substring(lastSlash + 1);

            int firstFolderSlash = fullPath.LastIndexOf('/', lastSlash - 1);

            if( firstFolderSlash > 0 )
            {
               String fileNameWithFolder = fullPath.Substring(firstFolderSlash + 1);
               if( fileNameWithFolder.StartsWith(platform) )
               {
                  result = fileNameWithFolder;
               }
            }
         }
         else
         {
            result = fullPath;
         }
         
         return result;
      }

      static String GetPathPlatformSKUOverride_Internal(string filePath, String platform, String skuOverride, String platformOverride )
      {
         String filenameWithPlatformFolder = GetFilenameWithPlatformFolder(filePath, platform);
         String folder = filePath.Substring(0, filePath.Length - filenameWithPlatformFolder.Length);

         String finalPath = folder;

         if (platformOverride != null)
         {
            switch(platformOverride.ToLower())
            {
               case "_360":
                  finalPath += "ovr_360/";
                  break;
               case "_ps3":
                  finalPath += "ovr_ps3/";
                  break;
               case "_vta":
                  finalPath += "ovr_vta/";
                  break;
               default:
                  throw new UsageException(String.Format("Unknown platform override {0} in GetPathPlatformSKUOverride_Internal", platformOverride));
                  //break; // Unreachable
            }
         }

         if (skuOverride != null)
         {
            finalPath += "ovr_" + skuOverride + "/";
         }

         finalPath += filenameWithPlatformFolder;

         return finalPath;
      }

      //IMPORTANT: An identical implementation of this functionality exists in BP_FileSupport.cpp
      static String GetPathPlatformSKUOverride(string filePath, String platform, String sku )
      {
         String overridePath;

         //i.e.: ovr_ps3/ovr_jp/temp.txt
         overridePath = GetPathPlatformSKUOverride_Internal(filePath, platform, sku, platform);
         if (File.Exists(overridePath))
            return overridePath;

         //i.e.: ovr_ps3/temp.txt
         overridePath = GetPathPlatformSKUOverride_Internal(filePath, platform, null, platform);
         if (File.Exists(overridePath))
            return overridePath;

         //i.e.: ovr_jp/temp.txt
         overridePath = GetPathPlatformSKUOverride_Internal(filePath, platform, sku, null);
         if (File.Exists(overridePath))
            return overridePath;

         return filePath;
      }

      static XmlElement AddFile_Split( Dictionary<String,bool> filters, String platform, String sku, CachedXmlParent parent, String _filePath, String _archivePath )
      {
         String filePath;
         String archivePath;

         // First, let's "normalize" the file path
         switch ( GetExt( _filePath ) )
         {
         case "ctxr":
         case "cmdl":
            filePath = AddPlatform( _filePath, platform );
            archivePath = AddPlatform( _archivePath, platform );
            break;
         case "gcx":
         case "tri":
            // All GCX files are in a _bp subfolder (rebuilt)
            // All TRI files are in a _bp subfolder (tex/clut data zeroed out)
            filePath = AddPlatform(_filePath, "_bp");
            archivePath = _archivePath;
            break;
         case "row":
            // Two row files are in a _bp subfolder (eu/us celgei_snapshot_bigsize.row)
			   {
               String possibleNewFilePath = AddPlatform(_filePath, "_bp");
               if(File.Exists(possibleNewFilePath))
               {
                  archivePath = _archivePath;
                  filePath = possibleNewFilePath;
               }
               else
               {
                  archivePath = _archivePath;
                  filePath = _filePath;
               }
			   }
            break;
         default:
            archivePath = _archivePath;
            filePath = _filePath;
            break;
         }

         filePath = GetPathPlatformSKUOverride(filePath, platform, sku);

         filePath = filePath.ToLower();
         archivePath = archivePath.ToLower();

         if ( !File.Exists( filePath ) )
         {
            throw new AbortException( "Error: Adding {0} but it doesn't exist!", filePath );
         }

         if ( PassesFilter( filters, filePath, platform ) )
         {
            XmlElement el = parent.OwnerDocument.CreateElement( "file" );
            el.SetAttribute( "path", filePath );
            el.SetAttribute( "archivepath", String.Format( "/{0}", archivePath ) );

            parent.AppendChild( el );

            return el;
         }
         else
         {
            return null;
         }
      }

      static XmlElement AddFile_One( Dictionary<String, bool> filters, String platform, String sku, CachedXmlParent parent, String filePath )
      {
         return AddFile_Split( filters, platform, sku, parent, filePath, filePath );
      }

      static void ProcessStages( CommandLine cmd, Dictionary<String,bool> filters, List<List<String>> stages, CachedXmlParent createElement )
      {
         foreach ( List<String> stage in stages )
         {
            String desc = String.Join( ", ", stage.ToArray() );
            Console.WriteLine( "Stage(s): {0}", ( desc.Length > 60 ) ? String.Format( "{0}...", desc.Substring( 0, 60 ) ) : desc  );
            createElement.AppendChild( createElement.OwnerDocument.CreateComment( desc ) );

            List<String> combined = new List<String>();
            List<String> sdx = new List<String>();

            for ( int i = 0; i < stage.Count; ++i )
            {
               AddFile_One( filters, cmd.mPlatform, cmd.mSKU, createElement, String.Format( "{0}/manifest.txt", stage[i] ) );
               AddFile_One( filters, cmd.mPlatform, cmd.mSKU, createElement, String.Format( "{0}/bp_assets.txt", stage[i] ) );

//               if ( stage[i] == "fr/stage/ending" )
//                  ListMerger.DebugMode = true;

               combined = ListMerger.MergeLists( combined, GatherPathsForStage( stage[i] ) );

//               ListMerger.DebugMode = false;
            }

            List<String> sdxStages = new List<String>();
            if ( cmd.mMGSVersion == 2)
            {
               // HACK HACK HACK - For MGS2, sdx files and manifests live only in the US folder!

               // Armature - For Vita we use a unified sdx archive so ignore sdx files.
               if (cmd.mPlatform != "_vta")
               {
                  if ((stage.Count != 1) || (stage[0].Substring(2, 1) != "/"))
                  {
                     throw new AbortException("Invalid stage spec for sdx's - {0}", stage[0]);
                  }

                  sdxStages = new List<String>();
                  sdxStages.Add(String.Format("us/{0}", stage[0].Substring(3)));
               }
            }
            else if ( (cmd.mMGSVersion == 3) && (cmd.mSKU != "jp") )            
            {
               // MGS3 - EU/NA
               //
               // NOTE: NA is now using EU regions to allow for EFS languages.
               
               // HACK HACK HACK
               // AndyO: For MGS3, only JP and US have unique SDX files.
               // EU SDX files are same as US but padded with garbage data in last 2k of file.
               // Instead of adding 4 EU SDX files, we're going to add 1 'us' version.

               if ( stage[0].Substring( 2, 1 ) != "/" )
               {
                  throw new AbortException( "Invalid stage spec for sdx's - {0}", stage[0] );
               }

               // Force 'us' region for SDX path
               sdxStages = new List<String>();
               sdxStages.Add( String.Format( "us/{0}", stage[0].Substring( 3 ) ) );
            }
            else
            {
               sdxStages = stage;
            }

            for ( int i = 0; i < sdxStages.Count; ++i )
            {
               foreach ( String sdxLine in ProtectedReadAllLines( String.Format( "{0}/sdxmanifest.txt", sdxStages[i] ) ) )
               {
                  if ( sdxLine != "" )
                  {
                     sdx.Add( String.Format( "{0}/{1}", sdxStages[i], sdxLine ) );
                  }
               }
            }


            foreach ( String paths in combined )
            {
               String[] pathParts = paths.Split( new char[] { ',' } );

               if ( cmd.mOutputUnifiedPaths )
               {
                  AddFile_One( filters, cmd.mPlatform, cmd.mSKU, createElement, pathParts[0] );
               }
               else
               {
                  AddFile_Split( filters, cmd.mPlatform, cmd.mSKU, createElement, pathParts[0], pathParts[1] );
               }
            }

            foreach ( String sdxLine in sdx )
            {
               AddFile_One( filters, cmd.mPlatform, cmd.mSKU, createElement, sdxLine );
            }
         }
      }

      static int RealMain( CommandLine cmd )
      {
         return GenerateXmlForRegions( cmd ) ? 0 : 1;
      }

      static int Main( string[] args )
      {
         try
         {
            if ( args.Length == 2 && args[0] == "-psp2parse" )
            {
               ParsePSP2Arc( args[1] );

               return 0;
            }
            else if ( args.Length == 2 && args[0] == "-depends" )
            {
               int ret = ParseDepends( args[1] );

               if ( ret != 0 )
               {
                  Console.WriteLine( "... archive needs to be rebuilt" );
               }

               return ret;
            }
            else
            {

               CommandLine cmd = new CommandLine( args );

               return RealMain( cmd );
            }
         }
         catch ( UsageException e )
         {
            Console.WriteLine( e.Message );
            Console.WriteLine();
            PrintUsage();

            return 1;
         }
         catch ( AbortException e )
         {
            Console.WriteLine( "Fatal error - aborting:" );
            Console.WriteLine( e.Message );
            return 3;
         }
         catch ( Exception e )
         {
            Console.WriteLine( "Fatal error:" );
            Console.WriteLine( e.Message );
            Console.WriteLine();
            Console.WriteLine( e.StackTrace );
            return 1;
         }
      }
   }
}
