using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Security.Cryptography;
using System.Diagnostics;

namespace flatlisttool
{
   public class PerforceCmd
   {
      #region Private Fields
      private static Boolean mHasShownMessage = false;
      private static List<String> mLastFilePathListUsed = null;
      #endregion

      public static Boolean SyncToHead( List<String> inFilePaths, Boolean showMessageOnFail )
      {
         Int32 exitCode = 0;
         String output = "";
         if ( !ExecuteP4Command( "sync", inFilePaths, showMessageOnFail, out output, out exitCode ) )
            return false;
         return true;
      }
      public static Boolean CheckoutFile( List<String> inFilePaths, Boolean inLockFile, Boolean showMessageOnFail )
      {
         Int32 exitCode = 0;
         String output = "";
         if ( !ExecuteP4Command( "edit", inFilePaths, showMessageOnFail, out output, out exitCode ) )
            return false;
         if ( inLockFile )
            if ( !ExecuteP4Command( "lock", inFilePaths, showMessageOnFail, out output, out exitCode ) )
               return false;
         return true;
      }

      public static void CheckoutFileThrow( List<String> inFilePaths, Boolean inLockFile )
      {
         if ( !CheckoutFile( inFilePaths, inLockFile, true ) )
         {
            throw new Exception( "Perforce Error" );
         }
      }

      public static Boolean DeleteFile( List<String> inFilePaths, Boolean showMessageOnFail )
      {
         Int32 exitCode = 0;
         String output = "";
         if ( !ExecuteP4Command( "delete", inFilePaths, showMessageOnFail, out output, out exitCode ) )
            return false;

         return true;
      }

      public static void DeleteFileThrow( List<String> inFilePaths )
      {
         if ( !DeleteFile( inFilePaths, true ) )
         {
            throw new Exception( "Perforce Error" );
         }
      }

      public static Boolean AddFile( List<String> inFilePaths, Boolean showMessageOnFail )
      {
         Int32 exitCode = 0;
         String output = "";
         if ( !ExecuteP4Command( "add", inFilePaths, showMessageOnFail, out output, out exitCode ) )
            return false;

         return true;
      }

      public static void AddFileThrow( List<String> inFilePaths)
      {
         if ( !AddFile( inFilePaths, true ) )
         {
            throw new Exception( "Perforce Error" );
         }
      }

      public static Boolean GetFilesOpenForEdit( out List<String> outFilePaths, Boolean showMessageOnFail )
      {
         Int32 exitCode = 0;
         String output = "";
         outFilePaths = null;
         if ( !ExecuteP4Command( "opened", null, showMessageOnFail, out output, out exitCode ) )
            return false;
         outFilePaths = new List<String>();
         return true;
      }
      public static Boolean IsOpenForAddOrEdit( String inFilePath, Boolean showMessageOnFail, Boolean throwOnFail )
      {
         Int32 exitCode = 0;
         String output = "";
         List<String> filePaths = new List<String>() { inFilePath };

         if ( !ExecuteP4Command( "opened", filePaths, showMessageOnFail, out output, out exitCode ) )
         {
            if ( throwOnFail )
            {
               throw new Exception( "Perforce error calling ExecuteP4Command" );
            }
            return false;
         }

         return !output.Contains( "file(s) not opened" );
      }

      public static Boolean GetFileDepotLocations( List<String> inFilePaths, out List<String> outDepotPaths, Boolean showMessageOnFail )
      {
         Int32 exitCode = 0;
         String output = "";
         outDepotPaths = null;
         if ( !ExecuteP4Command( "files", inFilePaths, showMessageOnFail, out output, out exitCode ) )
            return false;

         String[] outLines = output.Split( '\n' );
         outDepotPaths = new List<String>();
         foreach ( String line in outLines )
         {
            if ( String.IsNullOrEmpty( line.Trim() ) )
               continue;

            String depotFile = "";
            Int32 hashIndex = line.IndexOf( '#' );
            if ( hashIndex != -1 )
               depotFile = line.Substring( 0, hashIndex );
            outDepotPaths.Add( depotFile );
         }
         return true;
      }

      #region Private Methods
      private static Boolean StartProcessAndWait( String exeName, String arguments, Boolean showMessageOnFail, ref String output, ref Int32 exitCode )
      {
         String finalOutput = "";
         try
         {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.UseShellExecute = false;
            startInfo.CreateNoWindow = true;
            startInfo.FileName = exeName;
            startInfo.Arguments = arguments;
            startInfo.RedirectStandardOutput = output != null;
            startInfo.RedirectStandardError = true;

            System.Diagnostics.Process p = System.Diagnostics.Process.Start( startInfo );
            p.OutputDataReceived += delegate( Object sender, DataReceivedEventArgs e )
            {
               if ( !String.IsNullOrEmpty( e.Data ) )
                  finalOutput += e.Data + "\n";
            };

            if ( p != null )
            {
               p.BeginOutputReadLine();
               String error = p.StandardError.ReadToEnd();

               p.WaitForExit();
               if ( output != null )
                  output = finalOutput + error;
               exitCode = p.ExitCode;
               p.Dispose();
               if ( exitCode == 0 )
               {
                  mHasShownMessage = false;
                  return true;
               }
            }
         }
         catch ( Exception e )
         {
            if ( e.Message == "The system cannot find the file specified" )
            {
               Console.WriteLine( exeName + " is not installed on this machine.\nPlease see your Administrator.");
            }
         }
         if ( showMessageOnFail && exitCode == 1 && !mHasShownMessage )
         {
            Console.WriteLine("There was an error connecting with Perforce. More information may be available from your administrator.\n\n1. Make sure the default settings for Perforce are set up properly.\n2. Make sure the perforce server is running and accessible from this machine.\n3. Make sure that the perforce client is installed on this machine.\n\nChanges were not saved.\n\nReported Errors:\n" + output + "\n" );
            mHasShownMessage = true;
         }
         return false;
      }
      private static Boolean StartProcessAndForget( String exeName, String arguments, Boolean showMessageOnFail )
      {
         try
         {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.UseShellExecute = false;
            startInfo.CreateNoWindow = true;
            startInfo.FileName = exeName;
            startInfo.Arguments = arguments;

            System.Diagnostics.Process p = System.Diagnostics.Process.Start( startInfo );
            return true;
         }
         catch ( Exception e )
         {
            if ( e.Message == "The system cannot find the file specified" )
            {
               Console.WriteLine( exeName + " is not installed on this machine.\nPlease see your Administrator." );
            }
         }
         if ( showMessageOnFail && !mHasShownMessage )
         {
            Console.WriteLine( "There was an error connecting with Perforce. More information may be available from your administrator.\n\n1. Make sure the default settings for Perforce are set up properly.\n2. Make sure the perforce server is running and accessible from this machine.\n3. Make sure that the perforce client is installed on this machine.\n\nChanges were not saved." );
            mHasShownMessage = true;
         }
         return false;
      }
      private static Boolean ExecuteP4Command( String cmd, List<String> inFilePaths, Boolean showMessageOnFail, out String output, out Int32 exitCode )
      {
         output = "";
         exitCode = 0;
         if ( inFilePaths == null )
            return StartProcessAndWait( "p4.exe", cmd, showMessageOnFail, ref output, ref exitCode );
         else if ( inFilePaths.Count == 1 )
            return StartProcessAndWait( "p4.exe", cmd + " " + inFilePaths[0], showMessageOnFail, ref output, ref exitCode );
         else
            return StartProcessAndWait( "p4.exe", "-x " + MakeFileList( inFilePaths ) + " -d \"\" " + cmd, showMessageOnFail, ref output, ref exitCode );
      }
      private static String MakeFileList( List<String> inFiles )
      {
         String p4FilePath = Environment.GetEnvironmentVariable( "TEMP" ) + "\\p4files.txt";
         if ( inFiles == mLastFilePathListUsed )
            return p4FilePath;
         mLastFilePathListUsed = inFiles;

         StringBuilder filePaths = new StringBuilder();
         foreach ( String filePath in inFiles )
         {
            filePaths.AppendFormat( "{0}\n", filePath );
         }
         filePaths = filePaths.Replace( "\\", "/" );
         File.WriteAllText( p4FilePath, filePaths.ToString() );

         return p4FilePath;
      }
      #endregion
   }

   class FlatListAdder
   {
      public void AddIDToAssetMap( String file )
      {
         int lineNum = 0;

         string[] lines = File.ReadAllLines( file, Encoding.ASCII );

         string explanation = "Creating asset ID map: ";
         System.Console.Write(explanation);

         foreach ( String line in lines )
         {
            ++lineNum;

            if ((lineNum % 50) == 0)
            {
               System.Console.CursorLeft = explanation.Length;
               System.Console.Write(100 * lineNum / lines.Length);
               System.Console.Write("%");
            }

            if ( line == "" )
               continue;

            String[] torn = line.Split( ' ' );

            if (torn.Length == 2)
            {
               try
               {
                  uint key = Program.ParseUintOrHex(torn[0]);
                  if (!mIDToAssetMap.ContainsKey(key))
                  {
                     mIDToAssetMap.Add(key, torn[1]);
                  }
               }
               catch (ArgumentException)
               {
                  // Ignore the situation where a key already exists
               }
               catch (FormatException)
               {
                  Console.WriteLine("Error: Line {0}: '{1}' could not be parsed as an integer", lineNum, torn[0]);

                  throw;
               }
            }
         }
         System.Console.CursorLeft = 0;
         System.Console.WriteLine(explanation + "100%");
      }

      public void WriteIDToAssetMap( String file )
      {
         PerforceCmd.CheckoutFileThrow(new List<string>() { file }, false);

         string text = "\n";
         foreach (String name in mNewlyAddedAssets)
         {
            uint id = MakeStrCode(name.ToLower());
            text += String.Format("0x{0:x8} {1}\n", id, name);
         }

         File.AppendAllText(file, text);
      }

      void ParseMetasForMD5()
      {
         SearchOption so;

         switch ( mListType )
         {
         case CommandLine.EListType.TextureList:
            so = SearchOption.TopDirectoryOnly;
            break;
         case CommandLine.EListType.AssetList:
            so = SearchOption.AllDirectories;
            break;
         default:
            throw new Exception( "Unknown list type" );
         }

         String[] xmlMetaFiles = Directory.GetFiles( mXmlMetaFolder, "*.xmlmeta", so );
         int metaNum = 0;
         Console.Write( "Parsing metadata..." );
         Dictionary< String, String > hashToXmlMeta = new Dictionary<String, String>();

         foreach ( String xmlMetaFile in xmlMetaFiles )
         {
            XmlDocument doc = new XmlDocument();
            doc.Load( xmlMetaFile );

            string nameKey = "Name";
            if (Program.GetXmlMetaVersion(doc) > 2)
            {
               nameKey = "OriginalBPUnifiedFilename";
            }
            Program.ValidateXmlMeta( doc, xmlMetaFile );

            String hash = doc.DocumentElement.SelectSingleNode( "OriginalFileHash" ).InnerText;
            String name = doc.DocumentElement.SelectSingleNode( nameKey ).InnerText;

            if ( hashToXmlMeta.ContainsKey( hash ) )
            {
               Console.WriteLine();
               Console.WriteLine( "Conflicting hashes!" );
               Console.WriteLine( "  {0}", hashToXmlMeta[hash] );
               Console.WriteLine( "  {0}", xmlMetaFile );

               throw new Exception( "Conflicting hashes!" );
            }

            hashToXmlMeta.Add( hash, xmlMetaFile );
            mHashToNameMap.Add( hash, name );
            if ( (++metaNum % 500) == 0 )
            {
               Console.Write( "." );
            }
         }

         Console.WriteLine();
      }

      XmlDocument CreateNewXmlMeta( String name, String hash )
      {
         XmlDocument doc = new XmlDocument();

         String metadataRootTag;

         switch ( mListType )
         {
         case CommandLine.EListType.TextureList:
            metadataRootTag = "TextureMetadata";
            break;
         case CommandLine.EListType.AssetList:
            metadataRootTag = "AssetMetadata";
            break;
         default:
            throw new Exception( String.Format( "Unknown list type {0}", mListType ) );
         }

         ((XmlElement)doc.AppendChild( doc.CreateElement( metadataRootTag ) )).SetAttribute( "version", "3" );
         XmlElement nameElement = (XmlElement)doc.DocumentElement.AppendChild( doc.CreateElement( "OriginalBPUnifiedFilename" ) );
         nameElement.InnerText = name;

         doc.DocumentElement.AppendChild( doc.CreateElement( "TagList" ) );
         doc.DocumentElement.AppendChild( doc.CreateElement( "MappedPaths" ) );

         XmlElement addlText = (XmlElement)doc.DocumentElement.AppendChild( doc.CreateElement( "AdditionalText" ) );
         addlText.AppendChild( doc.CreateElement( "Text" ) );

         XmlElement originalFileHash = (XmlElement)doc.DocumentElement.AppendChild( doc.CreateElement( "OriginalFileHash" ) );
         originalFileHash.InnerText = hash;

         return doc;
      }

      void AddXmlMetaWidthAndHeight( XmlDocument doc, int width, int height )
      {
         XmlElement widthElement = (XmlElement) doc.DocumentElement.AppendChild( doc.CreateElement( "OriginalWidth" ) );
         XmlElement heightElement = (XmlElement)doc.DocumentElement.AppendChild( doc.CreateElement( "OriginalHeight" ) );

         widthElement.InnerText = width.ToString();
         heightElement.InnerText = height.ToString();
      }

      public FlatListAdder( String xmlMetaFolder, String flatMapFile, CommandLine.EListType listType )
      {
         mListType = listType;
         mXmlMetaFolder = xmlMetaFolder;
         mFlatMapFile = flatMapFile;
         ParseMetasForMD5();
         mFlatMap = Program.ReadMappingFile( mFlatMapFile, listType );
      }

      public void WriteFlatMap()
      {
         PerforceCmd.CheckoutFileThrow(new List<string>() { mFlatMapFile }, false);
         Program.WriteMappingFile( mFlatMapFile, mFlatMap, mListType );
      }

      public static uint MakeStrCode(string str)
      {
         uint id = 0;
         foreach (char c in str.ToLower())
         {
            id = (id << 5) | (id >> (24 - 5));
            id += c;
            id &= 0x00ffffff;
         }
         return (id == 0) ? 1 : id;
      }

      private String GetNamePart( String relativePath )
      {
         int lastSlash = relativePath.LastIndexOf('/');
         if (lastSlash == -1)
         {
            throw new Exception(String.Format("Relative path {0} doesn't have a slash?!"));
         }

         String namePart = relativePath.Substring(lastSlash + 1);
         int period = namePart.IndexOf('.');

         if (period != -1)
         {
            namePart = namePart.Substring(0, period);
         }

         return namePart;
      }

      UInt32 FilenameToId( String relativePath )
      {
         String namePart = GetNamePart( relativePath );

         uint id = 0;
         if (namePart.Length > 8 || !UInt32.TryParse( namePart, System.Globalization.NumberStyles.HexNumber, null, out id ))
         {
            // if it's not hex, return the strcode
            id = MakeStrCode(namePart);
         }
         return id;
      }

      String IdToNamePart( UInt32 id, ref bool isId )
      {
         String asset;
         if ( mIDToAssetMap.TryGetValue( id, out asset ) )
         {
            isId = false;
            return asset;
         }
         else
         {
            isId = true;
            return id.ToString( "x8" );
         }
      }

      String IdToNamePart_NoExt( UInt32 id, ref bool isId )
      {
         String namePart = IdToNamePart( id, ref isId );

         int period = namePart.IndexOf( '.' );
         if ( period == -1 )
         {
            return namePart;
         }
         else
         {
            return namePart.Substring( 0, period );
         }
      }

      String IdToName_TextureStyle( UInt32 id )
      {
         bool isId = false;
         String name = IdToNamePart_NoExt( id, ref isId );

         if ( isId )
         {
            return FormatTextureNameIdOnly( id );
         }
         else
         {
            return FormatTextureNameIdName( id, name );
         }
      }

      static public String FormatTextureNameIdOnly( UInt32 id )
      {
         return String.Format( "{0}-unknownfilename", id.ToString( "x8" ) );
      }

      static public String FormatTextureNameIdName( UInt32 id, String name )
      {
         return String.Format( "{0}-{1}", id.ToString( "x8" ), name );
      }

      static public String FormatTextureNameAddHash( String formattedName, String hash )
      {
         return String.Format( "{0}~{1}", formattedName, hash );
      }

      String GetUniqueName( String relativePath, String hash )
      {
         UInt32 id = FilenameToId( relativePath );

         if ( mListType == CommandLine.EListType.TextureList )
         {
            // If this is a texture, we need to check if both the old-style name or new-name exist
            // this makes us forward-compatible against texture collisions

            String name = IdToName_TextureStyle( id );
            bool isId_Ignore = false;
            String oldStyleName = IdToNamePart( id, ref isId_Ignore );

            if ( File.Exists( String.Format( "{0}/{1}.xmlmeta", mXmlMetaFolder, name ) )
               || File.Exists( String.Format( "{0}/{1}.xmlmeta", mXmlMetaFolder, oldStyleName ) ) )
            {
               return FormatTextureNameAddHash( name, hash );
            }
            else
            {
               return name;
            }
         }
         else
         {
            bool isId = false;
            String namePart = IdToNamePart_NoExt( id, ref isId );

            String[] pathParts = relativePath.ToLower().Split( new char[] { '/', '\\' } );

            // Should be region / type / foldername / type2 / assetname

            if ( pathParts.Length != 5 )
            {
               throw new Exception( String.Format( "Relative path '{0}' doesn't have 5 parts" ) );
            }

            String region = pathParts[0];
            String stageSlotFace = pathParts[1];
            String folderName = pathParts[2];
            String cacheNoCacheResident = pathParts[3];
            String rawAsset = pathParts[4];

            String assetType = rawAsset.Substring( rawAsset.LastIndexOf( '.' ) + 1 );
            if ( assetType == "" )
            {
               throw new Exception( String.Format( "Couldn't find file type of {0}", rawAsset ) );
            }

            String asset = String.Format( "{0}.{1}", namePart, assetType ).ToLower();

            String rootDir = String.Format( "{0}/{1}", assetType, region );

            if ( File.Exists( String.Format( "{0}/{1}/{2}.xmlmeta", mXmlMetaFolder, rootDir, asset ) ) )
            {
               String assetMinusExt = asset.Substring( 0, asset.Length - assetType.Length - 1 );

               String stagePostpend;
               switch ( cacheNoCacheResident )
               {
               case "cache":
                  stagePostpend = "";
                  break;
               case "nocache":
                  stagePostpend = "_nc";
                  break;
               case "resident":
                  stagePostpend = "_r";
                  break;
               default:
                  throw new Exception( String.Format( "Unknown stage subfolder {0} for {1}", cacheNoCacheResident, namePart ) );
               }

               String biggerFilename = String.Format( "{0}/{1}~{2}_{3}{4}.{5}",
                  rootDir,
                  assetMinusExt,
                  stageSlotFace,
                  folderName,
                  stagePostpend,
                  assetType
                  );

               return biggerFilename;
            }
            else
            {
               return String.Format( "{0}/{1}", rootDir, asset );
            }
         }
      }

      private bool CheckHashAndName( String inputName, String hash, out String name )
      {
         if ( mHashToNameMap.TryGetValue( hash, out name ) )
         {
            if ( mListType == CommandLine.EListType.AssetList )
            {
               // Check the extensions match
               String inputExt = inputName.Substring( inputName.LastIndexOf( '.' ) ).ToLower();
               String outputExt = name.Substring( name.LastIndexOf( '.' ) ).ToLower();

               if ( inputExt != outputExt )
               {
                  throw new Exception( String.Format( "Tried to match {0} and {1}", inputName, name ) );
               }
            }

            return true;
         }
         else
         {
            return false;
         }
      }

      public bool TryAddFile( String relativePath )
      {
         try
         {
            Program.AddRefNoCtrlBreak();

            mMD5.Initialize();

            if ( mFlatMap.ContainsKey( relativePath ) )
            {
               Console.WriteLine( "{0} is already in the DB", relativePath );

               return false;
            }

            String fileToHash = relativePath;
            if ( relativePath.ToLower().EndsWith( ".tri" ) )
            {
               fileToHash += ".hashfile";
            }

            String hash = Program.MD5HashToString( mMD5.ComputeHash( File.ReadAllBytes( fileToHash ) ) );
            String name;
            XmlDocument metaData;
            String xmlMetaName;

            if ( CheckHashAndName( relativePath, hash, out name ) )
            {
               // Do we already have this hash in the database?

               Console.WriteLine( "{0} is linked to {1}", relativePath, name );

               xmlMetaName = String.Format( "{0}/{1}.xmlmeta", mXmlMetaFolder, name );
               metaData = new XmlDocument();
               metaData.Load( xmlMetaName );
            }
            else
            {
               Console.WriteLine( relativePath );

               // Get the unique name
               String namePart = GetNamePart( relativePath );
               uint id = 0;
               if (!UInt32.TryParse(namePart, System.Globalization.NumberStyles.HexNumber, null, out id))
               {
                  id = MakeStrCode(namePart);
                  if (!mIDToAssetMap.ContainsKey(id))
                  {
                     mIDToAssetMap[id] = namePart.ToLower();
                  }
               }
               name = GetUniqueName( relativePath, hash );

               Console.WriteLine( "{0} is added as {1}", relativePath, name );

               xmlMetaName = String.Format( "{0}/{1}.xmlmeta", mXmlMetaFolder, name );

               {
                  String dirPart = xmlMetaName.Substring( 0, xmlMetaName.LastIndexOfAny( new char[] { '/', '\\' } ) );
                  if ( !Directory.Exists( dirPart ) )
                  {
                     Directory.CreateDirectory( dirPart );
                  }
               }

               metaData = CreateNewXmlMeta( name, hash );

               if ( mListType == CommandLine.EListType.TextureList )
               {
                  Paloma.TargaHeader header = Paloma.TargaImage.LoadTargaHeader( relativePath );

                  AddXmlMetaWidthAndHeight( metaData, header.Width, header.Height );
               }
               
               mHashToNameMap.Add( hash, name );

               mNewlyAddedAssets.Add(namePart);

               String extraExt = (mListType == CommandLine.EListType.TextureList) ? ".tga" : "";

               String assetName = String.Format( "{0}/{1}{2}", mXmlMetaFolder, name, extraExt );
               Console.Write( "Copying {0} to {1}...", relativePath, assetName );
               File.Copy( relativePath, assetName, true );
               if ( !PerforceCmd.IsOpenForAddOrEdit( assetName, true, true /* throw on fail */  ) )
               {
                  Console.Write( "  p4 adding..." );
                  List<String> paths = new List<String>();
                  paths.Add( assetName );
                  PerforceCmd.AddFileThrow( paths );
               }

               Console.WriteLine();
            }

            {
               bool foundPathAlready = false;
               foreach ( XmlElement el in metaData.DocumentElement.SelectNodes( "MappedPaths/Path" ) )
               {
                  if ( el.InnerText == relativePath )
                  {
                     foundPathAlready = true;
                     break;
                  }
               }

               if ( !foundPathAlready )
               {
                  XmlElement path = metaData.CreateElement( "Path" );
                  path.InnerText = relativePath;

                  metaData.DocumentElement.SelectSingleNode( "MappedPaths" ).AppendChild( path );
               }
            }

            if ( File.Exists( xmlMetaName ) )
            {
               if ( (File.GetAttributes( xmlMetaName ) & FileAttributes.ReadOnly) == FileAttributes.ReadOnly )
               {
                  List<String> file = new List<String>();
                  file.Add( xmlMetaName );
                  PerforceCmd.CheckoutFileThrow( file, false );
               }

               Program.SaveXmlFileSafe( metaData, xmlMetaName );
            }
            else
            {
               List<String> file = new List<String>();
               file.Add( xmlMetaName );

               Program.SaveXmlFileSafe( metaData, xmlMetaName );

               try
               {
                  PerforceCmd.AddFileThrow( file );
               }
               catch
               {
                  try
                  {
                     File.Delete( xmlMetaName );
                  }
                  catch ( IOException )
                  {
                  }

                  throw;
               }
            }

            mFlatMap.Add( relativePath, name );

//            Program.WriteMappingFile( mFlatMapFile, mFlatMap );

            return true;
         }
         finally
         {
            Program.DelRefNoCtrlBreak();
         }
      }

      String mXmlMetaFolder;
      String mFlatMapFile;
      CommandLine.EListType mListType;
      Dictionary< UInt32, String > mIDToAssetMap = new Dictionary<UInt32, String>();
      Dictionary< String, String > mHashToNameMap = new Dictionary<String, String>();
      Dictionary< String, String > mFlatMap;
      List<String> mNewlyAddedAssets = new List<String>();
      MD5 mMD5 = MD5.Create();
   }
}
