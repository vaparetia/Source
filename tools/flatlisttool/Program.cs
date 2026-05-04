using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Security.Cryptography;

namespace flatlisttool
{
   class CommandLine
   {
      public enum EListType
      {
         TextureList,
         AssetList
      }

      public enum EOperationMode
      {
         None,
         UpgradeFlatlist,
         UpgradeMetadata,
         MoveFolders,
         AddTexturesAndAssets,
         Lookup,
         UpgradeLookup,
         PrintNewName,
         CombineAssets,
         RefreshOriginalWidthHeight
      }

      private void SetOperationModeOnce( EOperationMode mode )
      {
         if ( mOperationMode != EOperationMode.None )
         {
            throw new Exception( String.Format( "Trying to set mode {0} when already mode {1}", mode, mOperationMode ) );
         }

         mOperationMode = mode;
      }

      public CommandLine( String[] _args )
      {
         mOperationMode = EOperationMode.None;

         int currentArg = 0;
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

         while ( currentArg < args.Length )
         {
            int lastArg = currentArg;

            switch ( args[currentArg].ToLower() )
            {
            case "-assets":
               mListType = EListType.AssetList;
               ++currentArg;
               break;
            case "-i":
               mFilenameIn = args[currentArg + 1];
               currentArg += 2;
               break;
            case "-o":
               mFilenameOut = args[currentArg + 1];
               currentArg += 2;
               break;
            case "-m":
               mMetadataFolder = args[currentArg + 1];
               currentArg += 2;
               break;
            case "-g":
               mGamedataFolder = args[currentArg + 1];
               currentArg += 2;
               break;
            case "-uf":
               SetOperationModeOnce( EOperationMode.UpgradeFlatlist );
               currentArg += 1;
               break;
            case "-ulu":
               SetOperationModeOnce( EOperationMode.UpgradeLookup );
               currentArg++;
               break;
            case "-lu":
               SetOperationModeOnce( EOperationMode.Lookup );
               currentArg++;
               break;
            case "-um":
               SetOperationModeOnce( EOperationMode.UpgradeMetadata );
               currentArg += 1;
               break;
            case "-mf":
               SetOperationModeOnce( EOperationMode.MoveFolders );
               currentArg += 1;
               break;
            case "-at":
            case "-aa":
               SetOperationModeOnce( EOperationMode.AddTexturesAndAssets );
               currentArg += 1;
               break;
            case "-pnn":
               SetOperationModeOnce( EOperationMode.PrintNewName );
               ++currentArg;
               break;
            case "-ca":
               SetOperationModeOnce( EOperationMode.CombineAssets );
               ++currentArg;
               break;
            case "-rwh":
               SetOperationModeOnce( EOperationMode.RefreshOriginalWidthHeight );
               ++currentArg;
               break;
            default:
               mExtraArgs.Add( args[currentArg] );
               ++currentArg;
               break;
            }

            if ( currentArg == lastArg )
            {
               // If you get this exception, you forgot to increment the loop above
               throw new Exception( String.Format( "Command line parser code broken for '{0}'", args[currentArg] ) );
            }
         }

         if ( mFilenameIn == null && mMetadataFolder == null )
         {
            switch ( mOperationMode )
            {
            case EOperationMode.UpgradeLookup:
            case EOperationMode.PrintNewName:
               // ok to have no input files
               break;
            default:
               throw new Exception( "No input files or folders specified" );
            }
         }

         if ( mFilenameIn == null || mMetadataFolder == null )
         {
            if ( mOperationMode == EOperationMode.RefreshOriginalWidthHeight )
            {
               throw new Exception( "Needs both an input dir and a metadata folder" );
            }
         }

         if ( mGamedataFolder == null && mOperationMode == EOperationMode.CombineAssets )
         {
            throw new Exception( "No gamedata folder specified" );
         }

         if ( mFilenameOut == null && mOperationMode == EOperationMode.UpgradeFlatlist )
         {
            throw new Exception( "No output file specified" );
         }
      }

      public String GetLookupFile()
      {
         switch ( mListType )
         {
         case CommandLine.EListType.TextureList:
            return "EditorSupport/TextureLookup.txt";
         case CommandLine.EListType.AssetList:
            return "EditorSupport/AssetLookup.txt";
         }

         throw new Exception( "Couldn't get lookup file from list type" );
      }

      public readonly String mFilenameIn = null;
      public readonly String mFilenameOut = null;
      public readonly String mMetadataFolder = null;
      public readonly String mGamedataFolder = null;
      public EOperationMode mOperationMode { get; private set; }
      public readonly List<String> mExtraArgs = new List<String>();
      public readonly EListType mListType = EListType.TextureList;
   }

   class CtrlBreakException : System.Exception
   {
      public CtrlBreakException() : base()
      {
      }
   }

   class Program
   {
      static int mNoCtrlBreakRefCount = 0;
      static bool mPressedCtrlBreak = false;
      static bool mThrewCtrlBreak = false;

      public static void AddRefNoCtrlBreak()
      {
         System.Threading.Interlocked.Increment( ref mNoCtrlBreakRefCount );
      }

      public static void DelRefNoCtrlBreak()
      {
         if ( System.Threading.Interlocked.Decrement( ref mNoCtrlBreakRefCount ) == 0 )
         {
            ThrowCtrlBreakIfNeeded();
         }
      }

      public static void ThrowCtrlBreakIfNeeded()
      {
         if ( mPressedCtrlBreak && !mThrewCtrlBreak )
         {
            mThrewCtrlBreak = true;
            throw new CtrlBreakException();
         }
      }

      private static void HandleCtrlBreak( object sender, ConsoleCancelEventArgs e )
      {
         switch ( e.SpecialKey )
         {
         case ConsoleSpecialKey.ControlBreak:
         case ConsoleSpecialKey.ControlC:
            if ( mNoCtrlBreakRefCount > 0 )
            {
               mPressedCtrlBreak = true;
               e.Cancel = true;
            }
            break;
         }
      }

      public static String MD5HashToString( byte[] hash )
      {
         String ret = "";

         foreach ( byte b in hash )
         {
            ret += b.ToString( "x2" );
         }

         return ret;
      }

      static public String FileIdForListType( CommandLine.EListType listType )
      {
         switch ( listType )
         {
         case CommandLine.EListType.AssetList:
            return "assetmapv2";
         case CommandLine.EListType.TextureList:
            return "flatmapv2";
         default:
            throw new Exception( "Got unknown list type" );
         }
      }

      public static Dictionary<String, String> ReadMappingFile( String filenamein, CommandLine.EListType listType )
      {
         String[] lines = File.ReadAllLines( filenamein );


         if ( lines[0] != FileIdForListType( listType ) )
         {
            throw new Exception( String.Format( "Error, flat map for {0} is wrong format", filenamein ) );
         }

         Dictionary< String, String > ret = new Dictionary<String, String>();

         for ( int line = 1; line < lines.Length; ++line )
         {
            if ( lines[line] == "" )
               continue;

            String[] splitline = lines[line].Split( ' ' );

            if ( splitline.Length != 2 )
            {
               throw new Exception( String.Format( "Error, flat map is corrupt at line {0}", line ) );
            }

            ret.Add( splitline[0], splitline[1] );
         }

         return ret;
      }

      static void PrintNewStyleName( String filenamein, String optionalMetadataFolder )
      {
         String qualifiedFilenameIn;
         if ( optionalMetadataFolder != null )
         {
            qualifiedFilenameIn = Path.GetFullPath( Path.Combine( optionalMetadataFolder, filenamein ) );
         }
         else
         {
            qualifiedFilenameIn = Path.GetFullPath( filenamein );
         }

         String xmlMetaName = Path.ChangeExtension( qualifiedFilenameIn, "xmlmeta" );

         XmlDocument doc = new XmlDocument();
         doc.Load( xmlMetaName );

         ValidateXmlMeta( doc, xmlMetaName );
         ValidateXmlMetaType( doc, CommandLine.EListType.TextureList, xmlMetaName );

         // First, we need to divine the asset id.  The best way for this
         // is to take the first mapped path and grab its id
         String originalId;
         {
            String firstMappedPath = doc.DocumentElement.SelectSingleNode( "MappedPaths/Path" ).InnerText;

            int lastSlash = firstMappedPath.LastIndexOf( '/' );
            int firstPeriod = firstMappedPath.IndexOf( '.', lastSlash );

            // At this point, the string for lastSlash -> period should be
            // /########
            // Which is a slash, then 9 hex numbers

            if ( firstPeriod - lastSlash != 9 )
            {
               throw new Exception( String.Format( "{0} - Error couldn't get id from mapped path", filenamein ) );
            }

            originalId = firstMappedPath.Substring( lastSlash + 1, 8 ).ToLower();
         }

         // Now let's grab the md5 hash
         String md5 = doc.DocumentElement.SelectSingleNode( "OriginalFileHash" ).InnerText;

         // The "new" name is 
         // id-name[~hash]
         // ~hash is optional

         bool nameNeedsHash;
         String noExtNameNoHash;
         {
            // noExtNameBeforeHashTaken is taking off .tga...  There may be .bmp or .img, but they are
            // done as .bmp_HASH, so we need to remove the hash (later on) before removing THAT extension
            String noExtNameBeforeHashTaken = Path.GetFileNameWithoutExtension( qualifiedFilenameIn ).ToLower();

            // First, tear off the hash 
            if ( noExtNameBeforeHashTaken.EndsWith( "_" + md5 ) || noExtNameBeforeHashTaken.EndsWith( "~" + md5 ) )
            {
               // If we end with the hash, we need it
               nameNeedsHash = true;
               // Clip off the md5 and the ~ (or _)
               noExtNameNoHash = Path.GetFileNameWithoutExtension( noExtNameBeforeHashTaken.Substring( 0, noExtNameBeforeHashTaken.Length - md5.Length - 1 ) );
            }
            else
            {
               nameNeedsHash = false;
               noExtNameNoHash = Path.GetFileNameWithoutExtension( noExtNameBeforeHashTaken );
            }
         }

         // Now we have some strange logic.  If all we have left is the
         // id, then we change to id-unknownfilename
         // if we have id-something, we keep it the same
         // if we have something, then make it id-something
         String namePartOnly;

         if ( noExtNameNoHash == originalId )
         {
            namePartOnly = String.Format( "{0}-unknownfilename", originalId );
         }
         else if ( noExtNameNoHash.StartsWith( originalId + "-" ) )
         {
            namePartOnly = noExtNameNoHash;
         }
         else
         {
            namePartOnly = String.Format( "{0}-{1}", originalId, noExtNameNoHash );
         }

         if ( nameNeedsHash )
         {
            Console.WriteLine( "{0}~{1}.tga", namePartOnly, md5 );
         }
         else
         {
            Console.WriteLine( String.Format( "{0}.tga", namePartOnly ) );
         }
      }

      public static void WriteMappingFile( String filenameout, Dictionary<String, String> mappings, CommandLine.EListType listType )
      {
         try
         {
            AddRefNoCtrlBreak();

            List<String> linesOut = new List<String>();

            linesOut.Add( FileIdForListType( listType ) );

            foreach ( KeyValuePair<String,String> kv in mappings )
            {
               if ( kv.Key.Contains( ' ' ) || kv.Value.Contains( ' ' ) )
               {
                  throw new Exception( String.Format( "One of these files has a space: {0} or {1}", kv.Key, kv.Value ) );
               }

               String line = String.Format( "{0} {1}", kv.Key, kv.Value );

               linesOut.Add( line );
            }

            File.WriteAllLines( filenameout, linesOut.ToArray() );
         }
         finally
         {
            DelRefNoCtrlBreak();
         }
      }

      static public void SaveXmlFileSafe( XmlDocument doc, String name )
      {
         try
         {
            AddRefNoCtrlBreak();

            if ( File.Exists( name ) && (File.GetAttributes( name ) & FileAttributes.ReadOnly) == FileAttributes.ReadOnly )
            {
               PerforceCmd.CheckoutFileThrow( new List<String> { name }, false );
            }

            doc.Save( name );
         }
         finally
         {
            DelRefNoCtrlBreak();
         }
      }

      static Dictionary<String,String> ConvertToNewMappingFormat( String filenamein )
      {
         Console.WriteLine( "Converting old flatlist '{0}'", filenamein );
         
         String text = File.ReadAllText( filenamein, Encoding.ASCII );

         if ( text.StartsWith( "flatmapv2" ) )
         {
            throw new Exception( "This file doesn't need conversion" );
         }

         String[] fakeLines = text.Split( (char)0 );

         int fakeLineCount = int.Parse( fakeLines[0] );

         Dictionary<String,String> linesOut = new Dictionary<String, String>();

         for ( int line = 0; line < fakeLineCount; ++line )
         {
            String srcAsset = fakeLines[1 + line * 2];
            String dstAsset = fakeLines[2 + line * 2];

            // Fix up so it's .tga, not .ctxr
            if ( !srcAsset.EndsWith( ".ctxr" ) )
            {
               throw new Exception(String.Format( "Asset #{0} - Expected source asset to end in .ctxr: {1}", line, srcAsset ));
            }

            if ( srcAsset.Contains( ' ' ) || dstAsset.Contains( ' ' ) )
            {
               throw new Exception( String.Format( "Asset #{0} - Either '{1}' or '{2}' contains space", line, srcAsset, dstAsset ) );
            }

            srcAsset = srcAsset.Substring( 0, srcAsset.Length - 4 ) + "tga";

            linesOut.Add( srcAsset, dstAsset );
         }

         return linesOut;
      }

      static void PrintUsage()
      {
         Console.WriteLine( @"
Usage: flatlisttool [-assets] [-i filename] [-m folder] [-g folder ] [-c] 
       <-uf|-um|-mf|-at|-aa|-lu|-pnn|-ca> [args...]
  -assets       - Run in assets mode.  If not specified, in texture mode.
  -i filename   - Input mapping file is filename
  -m folder     - Metadata and asset folder
  -g folder     - Gamedata folder
  -ulu          - Upgrades lookup table from old format
  -uf           - Upgrades texture flatlist from old format
  -um           - Upgrades metadata from old format
  -mf           - Moves assets in folder mappings.  Args are src then dst
  -at|-aa       - Adds textures or assets passed as args.  Args can be 
                  wildcards
  -lu           - Looks up an entry in the mapping table
  -pnn          - Prints the new-style filename for a given tga file in the 
                  texture flatlist.  Does not work in assets mode.
  -ca           - Combine assets.  args are pairs of src/dst assets to combine.
  -rwh          - Refresh OriginalWidth and OriginalHeight from folder in -i 
                  to folder in -m
  @filename     - Pulls in arguments from an external file

Examples:
flatlisttool -i bp_flatlisttexturemapping.txt -m textures/flatlist -at @file.txt
 - pulls in TGA's from file.txt and adds them to the flat list
flatlisttool -i bp_flatlisttexturemapping.txt -m textures/flatlist -mf slot/ jp/slot/ stage/ jp/stage/
 - moves assets in slot to jp/slot and stage to jp/stage" );
      }

      static String[] MatchWildcards( String pattern, SearchOption searchOption )
      {
         String slashesRight = pattern.Replace( '/', '\\' );
         int lastSlash = slashesRight.LastIndexOf( '\\' );
         String directoryPart;
         String filePart;
         if ( lastSlash != -1 )
         {
            directoryPart = Path.GetFullPath( slashesRight.Substring( 0, lastSlash ) );
            filePart = slashesRight.Substring( lastSlash + 1 );
         }
         else
         {
            directoryPart = ".";
            filePart = slashesRight;
         }

         return Directory.GetFiles( directoryPart, filePart, searchOption );
      }

      static List<KeyValuePair<String, String>> GetFilesInOut( String patternin, String wantFileout, SearchOption so )
      {
         String [] filesin = MatchWildcards( patternin, so );
         String [] filesout;

         if ( filesin.Length == 0 )
         {
            throw new Exception( "Couldn't match any files to input" );
         }

         if ( wantFileout != null )
         {
            if ( filesin.Length == 1 )
            {
               filesout = new String[1];
               filesout[0] = wantFileout;
            }
            else
            {
               throw new Exception( "Cannot specify output files when input is a wildcard" );
            }
         }
         else
         {
            filesout = filesin;
         }

         List< KeyValuePair<String,String> > ret = new List<KeyValuePair<String,String>>();
         for ( int i = 0; i < filesin.Length; ++i )
         {
            ret.Add( new KeyValuePair<String, String>( filesin[i], filesout[i] ) );
         }

         return ret;
      }

      public static List<XmlElement> CopyElementsFromNodeList( XmlNodeList list )
      {
         List<XmlElement> el = new List<XmlElement>( list.Count );
         foreach ( XmlElement l in list )
         {
            el.Add( l );
         }

         return el;
      }

      static public int GetXmlMetaVersion( XmlDocument doc )
      {
         int versionNumber = int.Parse(doc.DocumentElement.GetAttribute("version"));
         return versionNumber;
      }

      static public void ValidateXmlMeta( XmlDocument doc, String name )
      {
         int versionNumber = GetXmlMetaVersion(doc);
         
         // NOTE! this is different for MGS2 - don't integrate a check for 3
         if ( versionNumber != 3 && versionNumber != 2 )
         {
            throw new Exception( String.Format( "{0} - invalid version {1} - expected 3", name, versionNumber ) );
         }
      }

      static void ValidateXmlMetaType( XmlDocument doc, CommandLine.EListType lt, String name )
      {
         switch ( lt )
         {
         case CommandLine.EListType.AssetList:
            if ( doc.DocumentElement.Name != "AssetMetadata" )
            {
               throw new Exception( String.Format( "ERROR {0} - Not asset metadata, but we're in assets mode!\n", name ) );
            }
            break;
         case CommandLine.EListType.TextureList:
            if ( doc.DocumentElement.Name != "TextureMetadata" )
            {
               throw new Exception( String.Format( "ERROR {0} - Not texture metadata, but we're in textures mode!\n", name ) );
            }
            break;

         default:
            throw new Exception( String.Format( "ERROR {0} - Running upgrade with unknown list type {1}", name, lt ) );
         }

      }

      static bool IsNewStyleFileName( String s )
      {
         // New style files start with "00######-" and # is a hex value

         if ( s.Length > 9 && s[8] == '-' && s.StartsWith( "00" ) )
         {
            return true;
         }
         else
         {
            return false;
         }
      }

      static void RefreshOriginalWidthHeight( String inputFolder, String metadataFolder )
      {
         Console.WriteLine( "Refreshing original width height" );
         Console.WriteLine( "* Input folder: {0}", inputFolder );
         Console.WriteLine( "* Output folder: {0}", metadataFolder );

         Console.WriteLine();
         Console.WriteLine( "Getting file list..." );
         String[] files = Directory.GetFiles( metadataFolder, "*.xmlmeta" );
         Console.WriteLine( "Checking files..." );

         int index = 0;
         foreach ( String path in Directory.GetFiles( metadataFolder, "*.xmlmeta" ) )
         {
            String filepart = Path.GetFileName( path );
            String destFullPath = Path.GetFullPath( path );
            String sourceFullPath = Path.ChangeExtension( Path.Combine( inputFolder, filepart ), "tga" );

            if ( ( ++index ) % 100 == 0 )
            {
               Console.WriteLine( "(file {0})", index );
            }

            if ( IsNewStyleFileName( filepart ) )
            {
               Console.WriteLine( "Ignoring {0}", filepart );
            }
            else if ( !File.Exists( sourceFullPath ) )
            {
               Console.WriteLine( "NO SOURCE FILE: {0}", sourceFullPath );
            }
            else
            {
               Paloma.TargaHeader header = Paloma.TargaImage.LoadTargaHeader( sourceFullPath );
               XmlDocument doc = new XmlDocument();
               doc.Load( destFullPath );

               XmlElement originalWidthElement = (XmlElement) doc.DocumentElement.SelectSingleNode( "OriginalWidth" );   
               XmlElement originalHeightElement = (XmlElement) doc.DocumentElement.SelectSingleNode( "OriginalHeight" );   

               int originalWidth = ( originalWidthElement == null ) ? -1 : int.Parse( originalWidthElement.InnerText );
               int originalHeight = ( originalHeightElement == null ) ? -1 : int.Parse( originalHeightElement.InnerText );

               if ( originalWidth != header.Width || originalHeight != header.Height )
               {
                  Console.WriteLine( "{0} - {1}x{2} -> {3}x{4}",
                     filepart,
                     originalWidth, originalHeight,
                     header.Width, header.Height );

                  if ( originalWidthElement == null )
                  {
                     originalWidthElement = doc.CreateElement( "OriginalWidth" );
                     doc.DocumentElement.AppendChild( originalWidthElement );
                  }

                  if ( originalHeightElement == null )
                  {
                     originalHeightElement = doc.CreateElement( "OriginalHeight" );
                     doc.DocumentElement.AppendChild( originalHeightElement );
                  }

                  originalWidthElement.InnerText = header.Width.ToString();
                  originalHeightElement.InnerText = header.Height.ToString();

                  PerforceCmd.CheckoutFileThrow( new List<String>( new String[] { destFullPath } ), false );

                  doc.Save( destFullPath );
               }
            }


         }
      }

      static void UpgradeMetadata( String metadataFolder, String wantFileout, CommandLine.EListType lt )
      {
         // Note - upgrade is disabled because version "3" was created by Andy by other means than this tool
         // and now we have some things that are two and some that are 3
         if ( lt == CommandLine.EListType.TextureList )
         {
            throw new Exception( "Upgrading of textures is not supported... yet." );
         }
         
         List<KeyValuePair<String,String> > inandout = GetFilesInOut( metadataFolder + "\\*.xmlmeta", wantFileout,
            ( lt == CommandLine.EListType.TextureList ) ? SearchOption.TopDirectoryOnly : SearchOption.AllDirectories );

         foreach ( KeyValuePair<String,String> kv in inandout )
         {
            String filein = kv.Key;
            String fileout = kv.Value;

            Console.Write( "Reading {0}...  ", filein );

            XmlDocument doc = new XmlDocument();
            doc.Load( filein );

            ValidateXmlMetaType( doc, lt, filein );

            if ( doc.DocumentElement.GetAttribute( "version" ) != "3" )
            {
               int oldVersion = int.Parse( doc.DocumentElement.GetAttribute( "version" ) );

               if ( oldVersion > 2 )
               {
                  throw new Exception( String.Format( "Can't upgrade version {0} - file {1}", oldVersion, filein ) );
               }

               Console.Write( "Upgrading...  " );
               doc.DocumentElement.SetAttribute( "version", "3" );

               if ( oldVersion == 1 )
               {
                  String currentMD5 = null;
                  MD5 md5 = MD5.Create();

                  // Fix mappings to tga instead of ctxr
                  foreach ( XmlElement path in CopyElementsFromNodeList( doc.DocumentElement.SelectNodes( "MappedPaths/Path" ) ) )
                  {
                     String oldValue = path.InnerText;
                     if ( !oldValue.EndsWith( "ctxr" ) )
                     {
                        throw new Exception( String.Format( "Mapped path {0} doesn't end in ctxr", oldValue ) );
                     }

                     path.InnerText = oldValue.Substring( 0, oldValue.Length - 4 ) + "tga";

                     byte[] fileBytes = File.ReadAllBytes( path.InnerText );
                     md5.Initialize();
                     String thisHash = MD5HashToString( md5.ComputeHash( fileBytes ) );

                     if ( currentMD5 != null && thisHash != currentMD5 )
                     {
                        throw new Exception( "Hashes don't match for mapped inputs" );
                     }

                     currentMD5 = thisHash;
                  }

                  if ( currentMD5 == null )
                  {
                     throw new Exception( "No mapped files, so no MD5 hash?!" );
                  }

                  XmlElement originalHash = doc.CreateElement( "OriginalFileHash" );
                  originalHash.InnerText = currentMD5;
                  doc.DocumentElement.AppendChild( originalHash );
               }

               if ( oldVersion == 2)
               {
                  // Rename Name to OriginalBPUnifiedFilename

                  XmlElement name = (XmlElement) doc.DocumentElement.SelectSingleNode( "Name" );
                  XmlElement originalBPUnifiedFilename = doc.CreateElement( "OriginalBPUnifiedFilename" );

                  originalBPUnifiedFilename.InnerXml = name.InnerXml;

                  doc.DocumentElement.RemoveChild( name );
                  doc.DocumentElement.AppendChild( originalBPUnifiedFilename );

                  // TODO: Upgrade width and height for textures here
               }
               SaveXmlFileSafe( doc, fileout );
               Console.WriteLine( "Done!" );
            }
            else
            {
               Console.WriteLine( "Warning: already at good version. Skipping." );
            }
            
         }

      }

      static String ModifyMapping( String inString, List<KeyValuePair<String,String>> moveFolders, ref bool wasChanged )
      {
         foreach ( KeyValuePair<String,String> moveKv in moveFolders )
         {
            if ( inString.StartsWith( moveKv.Key ) )
            {
               wasChanged = true;
               return moveKv.Value + inString.Substring( moveKv.Key.Length );
            }
         }

         return inString;
      }

      static void Lookup( String filenameIn, CommandLine.EListType listType, IEnumerable<String> lookups )
      {
         Dictionary<String,String> mappingFileIn = ReadMappingFile( filenameIn, listType );

         foreach ( String lookup in lookups )
         {
            if ( mappingFileIn.ContainsKey( lookup ) )
            {
               Console.WriteLine( mappingFileIn[lookup] );
            }
            else
            {
               throw new Exception( String.Format( "Couldn't find '{0}' in mapping table", lookup ) );
            }
         }
      }

      static String ExtractRegionFromGameDataRelativeFolder( String filename )
      {
         String[] split = filename.Split( '/' );

         if ( split[0] != "assets" )
         {
            throw new Exception( String.Format( "'{0}' is not a proper assets string", filename ) );
         }

         if ( split[2].Length != 2 )
         {
            throw new Exception( String.Format( "'{0}' region part is wrong or something", filename ) );
         }

         return split[2];
      }

      static String ReplaceRegionForFakeStagePath( String stagePath, String newRegion )
      {
         String[] split = stagePath.Split( '/' );
         split[0] = newRegion;

         return String.Join( "/", split );
      }

      static void CombineFiles( String filenameIn, String metadataFolder, CommandLine.EListType listType, String gamedataFolder, List<KeyValuePair<String, String>> combineFiles )
      {
         if ( listType != CommandLine.EListType.AssetList )
         {
            throw new Exception( "This operation only works on Asset Lists" );
         }

         // Let's check out all the manifest files
         Console.WriteLine( "Finding manifests..." );

         String[] manifests = Directory.GetFiles( gamedataFolder, "manifest.txt", SearchOption.AllDirectories );

         if ( manifests.Length == 0 )
         {
            throw new Exception( "No manifest files?!" );
         }

         Console.WriteLine( "Checking out mapping file..." );
         PerforceCmd.CheckoutFileThrow( new List<String>( new string[] { filenameIn } ), false );

         Dictionary< String, String > mappingFileIn = ReadMappingFile( filenameIn, listType );
         Dictionary<String,String> mappingFileOut = new Dictionary<String, String>( mappingFileIn );

         Console.WriteLine( "Fixing up the mapping file..." );

         Dictionary< String, bool > filesToDelete = new Dictionary<String, bool>();

         foreach ( KeyValuePair<String,String> combine in combineFiles )
         {
            String srcFile = combine.Key;
            String dstFile = combine.Value;

            if ( !File.Exists( srcFile ) )
            {
               throw new Exception( String.Format( "Can't find source file {0}", srcFile ) );
            }

            if ( !File.Exists( dstFile ) )
            {
               throw new Exception( String.Format( "Can't find dst file {0}", dstFile ) );
            }

            filesToDelete[srcFile] = true;

            // First, let's find/replace it in the mapping
            String mappingRelativeSrc = GetRelativeNormalizedPath( metadataFolder, srcFile );
            String mappingRelativeDst = GetRelativeNormalizedPath( metadataFolder, dstFile );

            bool anyEntriesReplaced = false;

            foreach ( String key in new List<String>( mappingFileOut.Keys ) )
            {
               if ( mappingFileOut[key] == mappingRelativeSrc )
               {
                  mappingFileOut[key] = mappingRelativeDst;
                  anyEntriesReplaced = true;
               }
            }

            if ( !anyEntriesReplaced )
            {
               throw new Exception( String.Format( "Couldn't find '{0}' in the mapping file", mappingRelativeSrc ) );
            }
         }

         WriteMappingFile( filenameIn, mappingFileOut, listType );

         Console.WriteLine( "Fixing up xml meta's..." );
         foreach ( KeyValuePair<String,String> combine in combineFiles )
         {
            String srcFile = String.Format( "{0}.xmlmeta", combine.Key );
            String dstFile = String.Format( "{0}.xmlmeta", combine.Value );

            if ( !File.Exists( srcFile ) )
            {
               Console.WriteLine( "Warning: No src xmlmeta '{0}'", srcFile );
            }
            else if ( !File.Exists( dstFile ) )
            {
               Console.WriteLine( "Warning: No dst xmlmeta '{0}'", dstFile );
               filesToDelete[srcFile] = true;
            }
            else
            {
               filesToDelete[srcFile] = true;

               // We are fixing up src xmlmeta
               XmlDocument srcDoc = new XmlDocument();
               XmlDocument dstDoc = new XmlDocument();

               srcDoc.Load( srcFile );
               dstDoc.Load( dstFile );

               // Pull in the mapped paths from src -> dst
               XmlElement dstMappedPaths = (XmlElement) dstDoc.DocumentElement.SelectSingleNode( "MappedPaths" );
               foreach ( XmlNode srcPath in srcDoc.DocumentElement.SelectNodes( "MappedPaths/Path" ) )
               {
                  dstMappedPaths.AppendChild( dstDoc.ImportNode( srcPath, true ) );
               }

               if ( (File.GetAttributes( dstFile ) & FileAttributes.ReadOnly) == FileAttributes.ReadOnly )
               {
                  PerforceCmd.CheckoutFileThrow( new List<String>( new string[] { dstFile } ), false );
               }

               dstDoc.Save( dstFile );
            }
         }

         // Remap all of the paths
         Dictionary<String,String> gameDataRelativePaths = new Dictionary<String, String>();

         foreach ( KeyValuePair<String,String> combine in combineFiles )
         {
            String newKey = GetRelativeNormalizedPath( gamedataFolder, combine.Key );
            String newValue = GetRelativeNormalizedPath( gamedataFolder, combine.Value );

            // Console.WriteLine( "{0} {1}", newKey, newValue );

            gameDataRelativePaths.Add(
               newKey, newValue
                );
         }

         Console.WriteLine( "Fixing up manifests..." );
         foreach ( String manifest in manifests )
         {
            List<String> newManifestLines = new List<String>();
            bool didAnyWork = false;

            foreach ( String oldLine in File.ReadAllLines( manifest ) )
            {
               // Split the manifest into its component parts
               String[] split = oldLine.Split( ',' );

               // Skip empty lines
               if ( split.Length != 0 )
               {
                  // Manifests must have 3 parts (at least).  They may have more.
                  if ( split.Length < 3 )
                  {
                     throw new Exception( String.Format( "Invalid line in manifest {0}", manifest ) );
                  }

                  // split[0] is the flat path
                  // split[1] is a weird stage generated path

                  String dst;
                  if ( gameDataRelativePaths.TryGetValue( split[0], out dst ) )
                  {
                     split[0] = dst;
                     split[1] = ReplaceRegionForFakeStagePath( split[1], ExtractRegionFromGameDataRelativeFolder( dst ) );
                     didAnyWork = true;
                  }

                  // Rebuild the string and add it if it's not there already
                  String reconstructed = String.Join( ",", split );
                  if ( !newManifestLines.Contains( reconstructed ) )
                  {
                     newManifestLines.Add( reconstructed );
                  }
               }
            }

            if ( didAnyWork )
            {
               if ( (File.GetAttributes( manifest ) & FileAttributes.ReadOnly) == FileAttributes.ReadOnly )
               {
                  PerforceCmd.CheckoutFileThrow( new List<String>( new string[] { manifest } ), false );
               }
               File.WriteAllLines( manifest, newManifestLines.ToArray() );
            }
         }

         Console.WriteLine( "Issuing perforce deletes..." );
         PerforceCmd.DeleteFileThrow( new List<String>( filesToDelete.Keys ) );
      }

      static void MoveFolders( String filenameIn, String metadataFolder, CommandLine.EListType listType, List<KeyValuePair<String, String>> moveFolders )
      {
         Console.WriteLine( "Moving folders in mapping file" );
         String metadataWildcard = String.Format( "{0}\\*.xmlmeta", metadataFolder );
         {
            Dictionary< String, String > mappingFileIn = ReadMappingFile( filenameIn, listType );
            Dictionary<String,String> mappingFileOut = new Dictionary<String, String>();
            bool anyMappingChange = false;
            foreach ( KeyValuePair<String,String> kv in mappingFileIn )
            {
               String outSrc = ModifyMapping( kv.Key, moveFolders, ref anyMappingChange );

               mappingFileOut.Add( outSrc, kv.Value );

            }

            if ( anyMappingChange )
            {
               Console.WriteLine( "Writing mapping changes to {0}", filenameIn );
               WriteMappingFile( filenameIn, mappingFileOut, listType );
            }
            else
            {
               Console.WriteLine( "No changes to mapping file!" );
            }
         }

         int processCount = 0;
         int changeCount = 0;
         String [] allMetafiles = MatchWildcards( 
            metadataWildcard, 
            ( listType == CommandLine.EListType.TextureList ) 
            ? SearchOption.TopDirectoryOnly 
            : SearchOption.AllDirectories );

         Console.WriteLine( "Processing {0} xmlmeta files...", allMetafiles.Length );
         foreach ( String metaFile in allMetafiles )
         {
            XmlDocument doc = new XmlDocument();
            bool anyChange = false;
            doc.Load( metaFile );
            foreach ( XmlElement path in CopyElementsFromNodeList( doc.DocumentElement.SelectNodes( "MappedPaths/Path" ) ) )
            {
               path.InnerText = ModifyMapping( path.InnerText, moveFolders, ref anyChange );
            }

            if ( anyChange )
            {
               Console.WriteLine( "Writing changed metafile {0}", metaFile );
               ++changeCount;

               SaveXmlFileSafe( doc, metaFile );
            }

            if ( 0 == (++processCount % 500) )
            {
               Console.WriteLine( "(processed {0}/{1})", processCount, allMetafiles.Length );
            }

         }

         Console.WriteLine( "Done. Changed {0}/{1} xmlmeta files", changeCount, allMetafiles.Length );
      }

      static String GetRelativeNormalizedPath( String fromDir, String filename )
      {
         String thisPath = Path.GetFullPath( fromDir );
         String thatPath = Path.GetFullPath( filename );

         if ( !thatPath.ToLower().StartsWith( thisPath.ToLower() ) )
         {
            throw new Exception( String.Format( "{0} does not live under {1}", filename, thisPath ) );
         }

         return thatPath.Substring( thisPath.Length ).Replace( '\\', '/' ).TrimStart( new char[] { '/' } );
      }

      static public UInt32 ParseUintOrHex( String s )
      {
         if ( s.StartsWith( "0x" ) )
         {
            return UInt32.Parse( s.Substring( 2 ), System.Globalization.NumberStyles.HexNumber );
         }
         else
         {
            return UInt32.Parse( s );
         }
      }

      static public String ToHex8( UInt32 i )
      {
         return i.ToString( "x8" );
      }

      static void UpgradeLookupFile( String lookupFile )
      {
         String[] lines = File.ReadAllLines( lookupFile, Encoding.ASCII );

         for ( int i = 0; i < lines.Length; ++i )
         {
            if ( lines[i] != "" )
            {
               String[] tokens = lines[i].Trim().Split( new char[] { ' ' } );

               if ( tokens.Length != 2 )
               {
                  throw new Exception( String.Format( "{0}: line {1}: Error - too many tokens", lookupFile, i+1 ) );
               }

               lines[i] = String.Format( "0x{0} {1}", ToHex8( ParseUintOrHex( tokens[0] ) ), tokens[1] );
            }
         }

         File.WriteAllLines( lookupFile, lines );
      }

      static int Main( string[] args )
      {
         try
         {
            // has to work out of GameData
            Directory.SetCurrentDirectory(System.Environment.GetEnvironmentVariable("BPE_REPOSITORY"));

            Console.CancelKeyPress += HandleCtrlBreak;
            
            if ( args.Length == 0 )
            {
               PrintUsage();
               return 0;
            }
            CommandLine cmd = new CommandLine( args );

            switch ( cmd.mOperationMode )
            {
            case CommandLine.EOperationMode.RefreshOriginalWidthHeight:
               RefreshOriginalWidthHeight( cmd.mFilenameIn, cmd.mMetadataFolder );
               break;
            case CommandLine.EOperationMode.UpgradeFlatlist:
               {
                  Dictionary<String,String> fileContents;
                  fileContents = ConvertToNewMappingFormat( cmd.mFilenameIn );

                  WriteMappingFile( cmd.mFilenameOut ?? cmd.mFilenameIn, fileContents, cmd.mListType );
               }
               break;
            case CommandLine.EOperationMode.UpgradeMetadata:
               UpgradeMetadata( cmd.mMetadataFolder, cmd.mFilenameOut, cmd.mListType );
               break;
            case CommandLine.EOperationMode.Lookup:
               Lookup( cmd.mFilenameIn, cmd.mListType, cmd.mExtraArgs );
               break;
            case CommandLine.EOperationMode.CombineAssets:
               {
                  List<KeyValuePair<String,String>> combineFiles = new List<KeyValuePair<String, String>>();

                  if ( cmd.mExtraArgs.Count == 0 )
                  {
                     Console.WriteLine( "ERROR: No extra arguments for combining assets" );
                     return 1;
                  }

                  if ( (cmd.mExtraArgs.Count & 1) != 0 )
                  {
                     Console.WriteLine( "ERROR: Got an odd amount of extra args for combining assets.  Should be pairs of src and dst assets" );
                     return 1;
                  }

                  for ( int argIndex = 0; argIndex < cmd.mExtraArgs.Count; argIndex += 2 )
                  {
                     String fullpath1 = Path.GetFullPath( cmd.mExtraArgs[argIndex] ).ToLower();
                     String fullpath2 = Path.GetFullPath( cmd.mExtraArgs[argIndex + 1] ).ToLower();

                     if ( fullpath1 != fullpath2 )
                     {
                        combineFiles.Add( new KeyValuePair<string, string>(
                           fullpath1,
                           fullpath2 ) );
                     }
                  }

                  CombineFiles( cmd.mFilenameIn, cmd.mMetadataFolder, cmd.mListType, cmd.mGamedataFolder, combineFiles );
               }
               break;
            case CommandLine.EOperationMode.MoveFolders:
               {
                  List< KeyValuePair<String,String> > moveFolders = new List<KeyValuePair<String, String>>();

                  if ( cmd.mExtraArgs.Count == 0 )
                  {
                     Console.WriteLine( "ERROR: No extra arguments for moving folders" );
                     return 1;
                  }

                  if ( (cmd.mExtraArgs.Count & 1) != 0 )
                  {
                     Console.WriteLine( "ERROR: Got an odd amount of extra args for moving.  Should be pairs of src and dst folders" );
                     return 1;
                  }

                  for ( int i = 0; i < cmd.mExtraArgs.Count; i += 2 )
                  {
                     moveFolders.Add( new KeyValuePair<string, string>( cmd.mExtraArgs[i], cmd.mExtraArgs[i + 1] ) );
                  }
                  MoveFolders( cmd.mFilenameIn, cmd.mMetadataFolder, cmd.mListType, moveFolders );
               }
               break;
            case CommandLine.EOperationMode.UpgradeLookup:
               {
                  UpgradeLookupFile( cmd.GetLookupFile() );
               }
               break;
            case CommandLine.EOperationMode.AddTexturesAndAssets:
               {
                  Console.WriteLine( "Adding assets - list type: {0}", cmd.mListType );
                  FlatListAdder adder = new FlatListAdder( cmd.mMetadataFolder, cmd.mFilenameIn, cmd.mListType );

                  adder.AddIDToAssetMap( cmd.GetLookupFile() );
                  
                  AddRefNoCtrlBreak();

                  try
                  {
                     foreach ( String tex in cmd.mExtraArgs )
                     {
                        foreach ( String matched in MatchWildcards( tex, SearchOption.TopDirectoryOnly ) )
                        {
                           String path = GetRelativeNormalizedPath( ".", matched );
                           //                        Console.WriteLine( path );
                           adder.TryAddFile( path );
                           ThrowCtrlBreakIfNeeded();
                        }
                     }
                     adder.WriteFlatMap();
                     adder.WriteIDToAssetMap(cmd.GetLookupFile());
                  }
                  finally
                  {
                     DelRefNoCtrlBreak();
                  }
                        
               }
               break;
            case CommandLine.EOperationMode.PrintNewName:
               foreach ( String arg in cmd.mExtraArgs )
               {
                  PrintNewStyleName( arg, cmd.mMetadataFolder ); 
               }
               break;
            default:
               throw new Exception( String.Format( "Don't know how to work with operation mode {0}", cmd.mOperationMode ) );
            }

            return 0;
         }
         catch ( CtrlBreakException )
         {
            Console.WriteLine( "^C" );

            return 3;
         }
         catch (System.Exception e)
         {
            Console.WriteLine();
            Console.WriteLine( "FATAL ERROR ({0}):", e.GetType().ToString() );
            Console.WriteLine( e.Message );
            Console.WriteLine( e.StackTrace );

            return 1;
         }

      }
   }
}
