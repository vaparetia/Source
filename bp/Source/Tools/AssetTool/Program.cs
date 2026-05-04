using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using System.Diagnostics;
using System.IO;
using System.Threading;
using Tools.Property;
using Tools.Property.UI;
using Tools.Property.Build;
using Tools.Property.Cook;
using Tools.Property.Misc;
using Tools.Property.Source;
using Tools.AssetSystem;
using Tools.Common;
using Helper;
using Tools.WorkQueue;

namespace AssetTool
{
   class Program
   {
      static RecursivePatternMatcher   sInputFiles = new RecursivePatternMatcher();
      static Manager                   sAssetManager = new Manager();

      static IPropertyDataSource       sPropertySource;
      static bool                      sEditMetaData = false;
      static bool                      sDisplayMatches = false;
      static String                    sHardLinkPath = null;
      
      /// <summary>
      /// Cook related (i.e. non-global) options.
      /// </summary>
      static CookingOptions            sCookingOptions = new CookingOptions();
      static public CookingOptions     CookingOptions
      {
         get { return sCookingOptions; }
      }     

      /// <summary>
      /// sSupportedAssetTypes uses ';' as a delimiter for multiple types.
      /// </summary>
      static readonly string sSupportedAssetTypes = "MODL;RMDL;GMDL;SKIN;GPRP;ANIM;ADNI;COLL;TXTR;SAMP;XML_;FONT;EFCT;FMDP;FMSB;EMDL;EBSM";

      [STAThread]
      static int Main(string[] args)
      {
         System.Windows.Forms.Application.EnableVisualStyles();

         if (args.Length == 0)
         {
            ShowUsage();
            return 1;
         }

         if (!ProcessArgs(args))
         {
            return 1;
         }

         bool bResult = true;

         if ( sDisplayMatches )
         {
            DisplayMatchingAssets();
         }
         else
         {

            if ( sEditMetaData )
            {
               bResult = EditMetaData();
               // Fall through to allow changed assets to get recooked.            
            }

            if ( bResult )
            {
               CookStatusWindow statusWindow = null;

               if ( sCookingOptions.mShowGUI )
               {
                  System.Windows.Forms.Application.EnableVisualStyles();
                  statusWindow = new CookStatusWindow();
                  // DONT CALL Show() - The window will show itself after a second

                  // Allow gui window to draw correctly.
                  System.Windows.Forms.Application.DoEvents();
               }

               using ( AssetBuildSystem buildSystem = new AssetBuildSystem( sCookingOptions.mForceRecookTime, sCookingOptions.mEnableMultiThreadedCook ? 2*System.Environment.ProcessorCount : 1 ) )
               {
                  buildSystem.StatusWindow = statusWindow;
                  bResult = ProcessInputAssetsForCook( buildSystem );
               }

               if ( bResult && sHardLinkPath != null )
               {
                  bResult = CreateHardLinksToAssets( statusWindow );
               }

               if ( statusWindow != null )
               {
                  statusWindow.Dispose();
               }
            }

            if ( !bResult )
            {
               Console.WriteLine( "Error: AssetTool cook failed!" );
               return 1;
            }
         }

         return 0;
      }

      private static bool ProcessInputAssetsForCook( AssetBuildSystem buildSystem )
      {
         List<string> matches;
         try
         {
            sInputFiles.Match( out matches );
         }
         catch ( AssetSystemException e )
         {
            Console.WriteLine( "Error parsing assets: {0}", e.Message );

            Console.WriteLine( "Input assets:" );
            foreach ( String s in sInputFiles.Inputs )
            {
               Console.WriteLine( "* {0}", s );
            }
            return false;
         }

         CookAssetList(matches, sCookingOptions, buildSystem );

         return buildSystem.Build( sCookingOptions );
      }

      private static bool CreateHardLinksToAssets( CookStatusWindow statusWindow )
      {
         List<String> matches;
         sInputFiles.Match( out matches );
         StringBuilder errorOutput = ( statusWindow == null ) ? null : new StringBuilder();

         if ( !PackageGather.CreateCookedHardLinks( matches, sAssetManager, sCookingOptions, sHardLinkPath, errorOutput ) )
         {
            if ( statusWindow != null )
            {
               statusWindow.TurnIntoErrorDisplayMode();
               statusWindow.AddTextToDetailsOutput( "\n" + errorOutput.ToString() );
               statusWindow.WaitForFormClose();
            }

            return false;
         }

         return true;
      }

      private static void CookAssetList(List<string> matches, CookingOptions cookingOptions, AssetBuildSystem buildSystem )
      {
         AnnotatedDependencyCloud cloud = new AnnotatedDependencyCloud();

         bool displayVerboseStatus = matches.Count > 500;

         if (displayVerboseStatus)
            System.Console.WriteLine("Checking which assets out of {0} need cooking...", matches.Count);
         
         int matchIdx = 0;

         Stopwatch watch = new Stopwatch();
         watch.Start();

         foreach ( string match in matches )
         {
            // Display percentage every so often (currently 5s)
            if( displayVerboseStatus && watch.ElapsedMilliseconds >= 5000 )
            {
               Console.Write("\r{0}%  ", matchIdx * 100 / matches.Count);
               watch.Reset();
               watch.Start();
            }

            ++matchIdx;
            string sourcePath = match;

            if (cookingOptions.mInputFileIsCookedAsset)
            {
               // Try to convert to src filename
               sourcePath = sAssetManager.GetCookedFilenameToSourceRepositoryRelativePath(match);
               if (sourcePath.Length == 0)
               {
                  Console.WriteLine("Error: No src filename for {0}", match);
                  continue;
               }
            }

            try
            {
               buildSystem.TryQueueAssetForBuild( sourcePath, cookingOptions.mPlatforms, !cookingOptions.mCookOnlyTopLevelAsset, cloud );
            }
            catch ( Tools.AssetSystem.Helper.InvalidAssetTypeException )
            {
               // This is actually OK, since if we use a wildcard we'll get some invalid asset types.
            }

         }

         if (displayVerboseStatus)
            Console.WriteLine("\r100%  \n");

         if ( cloud.HasAnyAnnotations() )
         {
            Console.WriteLine( "Problems queuing assets to build:" );
            cloud.PrintAnnotations();
         }
      }

      /// <summary>
      /// This function does the actual cooking of the asset based on platform.
      /// </summary>
      public static bool CookAsset(string assetPath, PlatformType.PlatformList inPlatforms, int verboseOutputLevel)
      {      
         string systemPathInputFilename;
         string[] systemPathOutputFilenames = null;
         PlatformType.EPlatform[] platforms = null;

         if ( !ValidateAssetAndGetSystemPaths( assetPath, inPlatforms, out systemPathInputFilename, out systemPathOutputFilenames, out platforms ) )
         {
            return false;
         }

         foreach ( String outputPath in systemPathOutputFilenames )
         {
            try
            {
               String directory = Path.GetDirectoryName(outputPath);
               if (!System.IO.Directory.Exists(directory))
               {
                  System.IO.Directory.CreateDirectory(directory);
               }
            }
            catch (System.IO.IOException)
            {
            }
         }

         bool bResult = true;

         // Single threaded cooking.
         Console.Write("Cooking {0}", sAssetManager.GetRepositoryRelativePath(systemPathOutputFilenames[0] ) );
         for ( int i = 1; i < systemPathOutputFilenames.Length; ++i )
         {
            Console.Write( ", {0}", 
               Misc.FindCommonStringBeginningAndReplaceWithEllipsis( 
                  sAssetManager.GetRepositoryRelativePath(systemPathOutputFilenames[0]),
                  sAssetManager.GetRepositoryRelativePath( systemPathOutputFilenames[i] ) ) );
         }
         Console.Write( "..." );

         AssetType type = AssetTypes.GetAssetTypeForSourceAsset(systemPathInputFilename);

         try
         {

            if ( !type.IsCooked )
            {
               bResult = false;

               Debug.Assert( type != null );

               switch ( type.TypeString )
               {
               case "GPRP":
                  bResult = Cookers.CookGPRP( systemPathInputFilename, systemPathOutputFilenames, platforms, sAssetManager );
                  break;
               case "SKIN":
                  for ( int platformIndex = 0; platformIndex < platforms.Length; ++platformIndex )
                  {
                     string systemPathOutputFilename = systemPathOutputFilenames[ platformIndex ];
                     PlatformType.EPlatform platform = platforms[ platformIndex ];

                     bResult = AssetToolMPP.Cookers.CookSKIN( systemPathInputFilename, systemPathOutputFilename, sAssetManager, platform, verboseOutputLevel );

                     if ( !bResult )
                     {
                        break;
                     }
                  }
                  break;
               case "MODL":
                  bResult = AssetToolMPP.Cookers.CookMODL( systemPathInputFilename, systemPathOutputFilenames, platforms, sAssetManager, verboseOutputLevel );
                  break;
               case "COLL":
                  bResult = AssetToolMPP.Cookers.CookCOLL( systemPathInputFilename, systemPathOutputFilenames[0], sAssetManager );
                  break;
               case "EMDL":
                  bResult = AssetToolMPP.Cookers.CookEMDL( systemPathInputFilename, systemPathOutputFilenames[0], sAssetManager );
                  break;
               case "TXTR":
                  bool debugInformation = false;
                  if (Program.CookingOptions.mCookerArguments.Contains("texture:debuginfo"))
                  {
                     debugInformation = true;
                  }
                  for ( int platformIndex = 0; platformIndex < platforms.Length; ++platformIndex )
                  {
                     string systemPathOutputFilename = systemPathOutputFilenames[platformIndex];
                     PlatformType.EPlatform platform = platforms[platformIndex];

                     bResult = AssetToolMPP.Cookers.CookTXTR(systemPathInputFilename, systemPathOutputFilename, sAssetManager, platform, debugInformation);
                     if ( !bResult )
                     {
                        break;
                     }
                  }
                  break;
               case "ANIM":
               case "ADNI":
                  for ( int platformIndex = 0; platformIndex < platforms.Length; ++platformIndex )
                  {
                     string systemPathOutputFilename = systemPathOutputFilenames[platformIndex];
                     PlatformType.EPlatform platform = platforms[platformIndex];

                     bResult = AssetToolMPP.Cookers.CookANIM( systemPathInputFilename, systemPathOutputFilename, sAssetManager, platform );
                     if ( !bResult )
                     {
                        break;
                     }
                  }
                  break;
               case "XML_":
                  bResult = Cookers.CookXML_( systemPathInputFilename, systemPathOutputFilenames[0], sAssetManager );
                  break;
               case "FONT":
                  bResult = AssetToolMPP.Cookers.CookFONT( systemPathInputFilename, systemPathOutputFilenames[0], sAssetManager );
                  break;
               case "EFCT":
                  {
                     bool compileDebugShaders = false;
                     if (Program.CookingOptions.mCookerArguments.Contains("shader:debug"))
                     {
                        compileDebugShaders = true;
                     }
                     for (int platformIndex = 0; platformIndex < platforms.Length; ++platformIndex)
                     {
                        bResult = AssetToolMPP.Cookers.CookEFCT(systemPathInputFilename, systemPathOutputFilenames[platformIndex], sAssetManager, platforms[platformIndex], compileDebugShaders);

                        if (!bResult)
                        {
                           break;
                        }
                     }
                  }
                  break;
               case "RMDL":
                  bResult = Cookers.CookRMDL( systemPathInputFilename );
                  break;
               case "FMDP":
                  for ( int platformIndex = 0; platformIndex < platforms.Length; ++platformIndex )
                  {
                     string systemPathOutputFilename = systemPathOutputFilenames[platformIndex];
                     PlatformType.EPlatform platform = platforms[platformIndex];

                     bResult = Cookers.CookFMDP( systemPathInputFilename, systemPathOutputFilename, sAssetManager, platform );
                     if ( !bResult )
                     {
                        break;
                     }
                  }
                  break;
               case "FMSB":
                  bResult = true;   // We don't currently cook these. Just here to stop cooker crashing on unknown type.
                  break;
               case "EBSM":
                  for ( int platformIndex = 0; platformIndex < platforms.Length; ++platformIndex )
                  {
                     string systemPathOutputFilename = systemPathOutputFilenames[platformIndex];
                     PlatformType.EPlatform platform = platforms[platformIndex];

                     bResult = Cookers.CookEBSM( systemPathInputFilename, systemPathOutputFilename, sAssetManager, platform );
                  }

                  break;
               default:
                  Debug.Assert( false );
                  bResult = false;
                  break;
               }
            }
         }
         finally
         {
            // This is in a finally so that if a cook fails by throwing an exception, the file gets deleted

            if ( !bResult )
            {
               Console.WriteLine( "\nAssetCook failed for {0}", String.Join( ", ", systemPathOutputFilenames ) );

               foreach ( String file in systemPathOutputFilenames )
               {
                  try
                  {
                     File.Delete( file );
                  }
                  catch ( System.IO.IOException )
                  {
                  }
               }
            }
            else
            {
               Console.WriteLine( "Done!" );
            }
         }

         return bResult;
      }

      /// <summary>
      /// Helper function that verifies that source asset filename is good, and can be cooked.
      /// If OK, returns src and cooked asset filenames.
      /// </summary>
      /// <returns>Returns false if not possible to be cooked.</returns>
      static bool ValidateAssetAndGetSystemPaths(string assetInputFilename, PlatformType.PlatformList platforms, out string systemPathInputFilename, out string[] systemPathCookedFilenames, out PlatformType.EPlatform[] cookedPlatforms)
      {
         systemPathInputFilename = "";
         systemPathCookedFilenames = null;
         cookedPlatforms = null;

         if (!sAssetManager.IsValidRepositoryRelativePath(assetInputFilename, sSupportedAssetTypes))
         {
            // Not valid filename
            Console.WriteLine("{0} is not a valid asset path.", assetInputFilename);
            return false;
         }

         systemPathInputFilename = sAssetManager.GetSystemPath(assetInputFilename);
         if (!System.IO.File.Exists(systemPathInputFilename))
         {
            Console.WriteLine("{0} doesn't exist.", systemPathInputFilename);
            return false;
         }
         AssetType type = AssetTypes.GetAssetTypeForSourceAsset(systemPathInputFilename);
         if (type.IsCooked)
         {
            // Don't need to check if the output file is readonly.
            return true;
         }

         if ( !Manager.GetCookedAssetPaths( systemPathInputFilename, platforms, out systemPathCookedFilenames, out cookedPlatforms ) )
         {
            Console.WriteLine( "Couldn't get cooked asset paths for {0}", systemPathInputFilename );
            return false;
         }

         foreach ( string systemPathCookedFilename in systemPathCookedFilenames )
         {
            if ( System.IO.File.Exists( systemPathCookedFilename ) && ( new System.IO.FileInfo( systemPathCookedFilename ) ).IsReadOnly )
            {
               Console.WriteLine( "{0} is read only.", systemPathCookedFilename );
               return false;
            }
         }

         return true;
      }

      /// <summary>
      /// Shows the matching assets of the incoming wildcard and command line options.
      /// </summary>
      private static void DisplayMatchingAssets()
      {
         List<string> matches;
         sInputFiles.Match( out matches );

         foreach ( string file in matches )
         {
            AssetType type = AssetTypes.GetAssetTypeForSourceAsset( file );

            if ( type != null && ( type.IsCooked == sCookingOptions.mInputFileIsCookedAsset ) )
            {
               Console.WriteLine( file );
            }
         }
      }

      /// <summary>
      /// Edits meta data for assets.
      /// </summary>
      private static bool EditMetaData()
      {
         bool bResult = true;

         FileBasedPropertyObjectEnumerator propertySource = new FileBasedPropertyObjectEnumerator();
         propertySource.AddFiles(FileBasedPropertyObjectEnumerator.FileSearchParams.DefaultFileSearchParams(sAssetManager));
         sPropertySource = propertySource;

         List<string> metaDataPaths = new List<string>();
         List<string> metaDataAssetPaths = new List<string>();
         List<XmlDocument> metaDataProperties = new List<XmlDocument>();

         List<string> matches;
         sInputFiles.Match(out matches);

         Console.WriteLine( "Editing combined meta data for..." );

         foreach (string assetPath in matches)
         {
            Console.WriteLine( " * {0}", assetPath );
            CollectMetaData(assetPath, ref metaDataPaths, ref metaDataAssetPaths, ref metaDataProperties);
         }

         if (metaDataProperties.Count > 0)
         {
            List<XmlElement> objects = new List<XmlElement>();

            foreach (XmlDocument doc in metaDataProperties)
            {
               XmlElement objectNav = doc.SelectSingleNode("//PropertyContainer") as XmlElement;
               if (objectNav != null)
                  objects.Add(objectNav);
            }

            ModalPropertyDlg dlg = new ModalPropertyDlg(objects, propertySource);
            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
               for (int i = 0; i < metaDataProperties.Count; ++i)
               {
                  XmlDocument fullDocument = metaDataProperties[i];
                  string metaPath = metaDataPaths[i];

                  if ((File.Exists(metaPath) == false) || (File.GetAttributes(metaPath) & FileAttributes.ReadOnly) == 0)
                  {
                     // File is writeable
                     XmlDocument unmergedDocument = MergePropertiesV2.Unmerge(fullDocument);
                     unmergedDocument.Save(metaPath);
                  }
                  else
                  {
                     Console.WriteLine("Error: {0} is readonly.", metaPath);
                     bResult = false;
                  }
               }
            }
         }
         else
         {
            Console.WriteLine("Error: No files found that match the pattern to edit meta data.");
         }
         return bResult;
      }

      /// <summary>
      /// Loads/creates meta data for the given path
      /// </summary>
      static void CollectMetaData(string assetPathInput, ref List<string> metaDataPaths, ref List<string> metaDataAssetPaths, ref List<XmlDocument> metaDataProperties)
      {
         string assetPath = assetPathInput;
         // Check for source cooked asset
         if (sCookingOptions.mInputFileIsCookedAsset)
         {
            // Try to convert to src filename
            assetPath = sAssetManager.GetCookedFilenameToSourceRepositoryRelativePath(assetPath);
            if (assetPath.Length == 0)
            {
               Console.WriteLine("Error: No src filename for {0}", assetPath);
               return;
            }
         }

         AssetType type = AssetTypes.GetAssetTypeForSourceAsset(assetPath);
         if (type != null)
         {
            if (sPropertySource.FindPropertyContainerById(type.TypeString) != null)
            {
               string metaDataPath = sAssetManager.GetMetaDataSystemPath(assetPath);

               XmlDocument metaData = new XmlDocument();

               if (File.Exists(metaDataPath))
               {
                  metaData.Load(metaDataPath);
               }
               else
               {
                  XmlElement usingElement = metaData.CreateElement("UsingPropertyContainer");

                  usingElement.SetAttribute(MergePropertiesV2.kPropertyContainer_version, MergePropertiesV2.kPropertyContainer_versionValue);
                  usingElement.SetAttribute("id", type.TypeString);
                  usingElement.SetAttribute("name", "MetaData");

                  metaData.AppendChild(usingElement);
               }

               MergePropertiesV2.Merge_Inplace(metaData, sPropertySource);
               metaDataProperties.Add(metaData);

               metaDataPaths.Add(metaDataPath);
               metaDataAssetPaths.Add(assetPath); // Add asset so we can cook it after we've applied the meta data changes.
            }
         }

         return;
      }

      static void ShowUsage()
      {
         Console.WriteLine("AssetTool [/esmftcdgpvh] <FILENAME>");
         Console.WriteLine();
         Console.WriteLine("Builds raw assets into compiled assets.");
         Console.WriteLine("FILENAME can be asset path relative or a system path (wild cards allowed)");
         Console.WriteLine("Output filename based off input filename.");
         Console.WriteLine("/e   Edit meta data for given path.");
         Console.WriteLine("/s   Scans recursively for the given path.");
         Console.WriteLine("/m   Display assets that match the command line passed in" );
         Console.WriteLine("/f   Forces cook of assets." );
         Console.WriteLine("/t   Only checks first asset for cooking. Use with /f to force command line asset only to cook.");
         Console.WriteLine("/c   Input data is cooked filename, attempts convert to source filename.");
         Console.WriteLine("/d   Disable multi-threaded cooking.");
         Console.WriteLine("/g   Display progress GUI while cooking.");
         Console.WriteLine("/p[] Cook only files for specified platform.");
         Console.WriteLine("/v   Verbose cooking output.");
         Console.WriteLine("/h[] Clone cooked asset tree to path [] with hard links" );
         Console.WriteLine("/aShader:debug");
         Console.WriteLine("     Compiles debug shaders (X360 only)");
         Console.WriteLine("/aTexture:debuginfo");
         Console.WriteLine("     Compiles debug info for textures (Min/Max/Avg RGB PCT.tga) (X360 only)");
         Console.WriteLine();
         Console.WriteLine("E.g. AssetTool.exe $/Test/SimpleArea/SimpleArea.sgpr");
         Console.WriteLine(@"E.g. AssetTool.exe C:\dev\Next\GameData\Main\Test\SimpleArea\SimpleArea.sgpr");
         Console.WriteLine("E.g. AssetTool.exe /s *.mdl");
         Console.WriteLine("E.g. AssetTool.exe /s *.mdl *.sgpr");
         Console.WriteLine("E.g. AssetTool.exe /p_ps3 /s *.fdp");
         Console.WriteLine();
         Console.WriteLine("Supported asset types:");
         foreach (string supportedType in sSupportedAssetTypes.Split(';'))
         {
            AssetType type = AssetTypes.GetAssetTypeForTypeId(Misc.GetFourCCFromString(supportedType));
            Debug.Assert(type != null);
            Console.WriteLine("{0}/{1,-20}   SrcExtension: {2}", type.TypeString, type.Description, String.Join(";", type.SrcExtensions));
         }
         Console.WriteLine();
         Console.WriteLine("Supported platforms:");
         foreach (PlatformType platform in PlatformType.sAssetPlatformTypes)
         {
            Console.WriteLine("{0,-25}   TypeId: {1}", platform.Description, platform.TypeString);
         }              
         // Only display default cookable platforms if different count from supported platforms
         if (PlatformType.CookPlatforms.Count != PlatformType.sAssetPlatformTypes.Length)
         {
            Console.WriteLine();
            Console.WriteLine("Default cook platforms:");
            foreach (PlatformType.EPlatform platformEnum in PlatformType.CookPlatforms)
            {
               PlatformType platform = PlatformType.GetPlatformTypeForEnum(platformEnum);
               Console.WriteLine("{0,-25}   TypeId: {1}", platform.Description, platform.TypeString);
            }
         }
         Console.WriteLine();
         Console.WriteLine("Set %BPE_PLATFORM_COOK_EXCLUSIONS% to exclude cooked platforms.");
         Console.WriteLine(" e.g. BPE_PLATFORM_COOK_EXCLUSIONS=_ps3;_360 would disable PS3 and X360 assets from being cooked by default.");
      }

      static bool ProcessArgs(string[] args)
      {
         foreach (string arg in args)
         {
            if ((arg[0] == '/') && (arg.Length > 1))
            {
               switch ( arg.Substring( 1,1 ).ToUpper() )
               {
                  case "S":   // Source filename
                     sInputFiles.mRecursive = true;
                     break;

                  case "F":
                     // Force a recook if asset is older than now
                     sCookingOptions.mForceRecookTime = DateTime.Now;
                     break;

                  case "T":
                     sCookingOptions.mCookOnlyTopLevelAsset = true;
                     break;

                  case "E":
                     sEditMetaData = true;
                     break;
                  
                  case "C":
                     sCookingOptions.mInputFileIsCookedAsset = true;
                     break;

                  case "D":
                     sCookingOptions.mEnableMultiThreadedCook = false;
                     break;

                  case "G":
                     sCookingOptions.mShowGUI = true;
                     break;

                  case "V":
                     sCookingOptions.mVerboseOutputLevel = 1;
                     break;

                  case "P":
                     {
                        string typeString = arg.Substring(2);
                        PlatformType platformType = PlatformType.GetPlatformTypeForString(typeString);
                        if (platformType == null)
                        {
                           Console.WriteLine("Unknown platform '{0}'.", typeString);
                           return false;
                        }

                        sCookingOptions.mPlatforms.Add(platformType.Platform);
                     }
                     break;

                  case "H":
                     sHardLinkPath = arg.Substring( 2 );
                     if ( !System.IO.Directory.Exists( sHardLinkPath ) )
                     {
                        Console.WriteLine( "Folder '{0}' does not exist or is not a folder" );
                     }
                     break;

                  case "M":
                     sDisplayMatches = true;
                     break;

                  case "A":
                     {
                        string cookerArgument = arg.Substring(2);
                        sCookingOptions.mCookerArguments.Add(cookerArgument.ToLower());
                     }
                     break;

                  case "?":
                     ShowUsage();
                     return false;

                  default:
                     Console.WriteLine(String.Format("Unknown argument '{0}'.", arg));
                     return false;
               }
            }
            else
            {
               if (arg.StartsWith("@"))
               {
                  using( StreamReader reader = new StreamReader(arg.Substring(1)) )
                  {
                     while(!reader.EndOfStream)
                     {
                        string path = reader.ReadLine();
                        sInputFiles.AddSpecification(path);
                     }
                  }
               }
               else
               {
                  sInputFiles.AddSpecification(arg);
               }
            }
         }

         // If no platforms specified, all platforms work
         if ( sCookingOptions.mPlatforms.Empty )
         {
            sCookingOptions.mPlatforms = PlatformType.CookPlatforms;
         }

         return true;
      }
   }
}
