using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml;
using System.Xml.XPath;
using Tools.Property.Build;
using Microsoft.Win32.SafeHandles;
using System.Runtime.InteropServices;
using Tools.Common;

namespace Tools.AssetSystem
{
	public class Helper
	{
		public enum ENeedsCooking
		{
			No,
			Yes,
         NotACookableAsset,
		};

      public enum ECookAssetsMode
      {
         Default = 0,
         ForceCook = 1,
         DisplayGUI = 2,
         WaitForFinish = 4
      };

      public class InvalidAssetTypeException : AssetSystemException
      {
         public InvalidAssetTypeException( string message, string assetPath )
            : base( message )
         {
            AssetPath = assetPath;
         }

         public readonly String AssetPath;
      }

      public static PlatformType.PlatformList GetPlatformsNeedingCooking( string assetPath, Manager manager, DateTime? forceRecookTime, PlatformType.PlatformList checkPlatforms )
      {
         AssetType type = AssetTypes.GetAssetTypeForSourceAsset( assetPath );
         if ( type == null )
         {
            throw new InvalidAssetTypeException( String.Format( "Invalid asset type cooking {0}", assetPath ), assetPath );
         }

         if ( type.CooksToPlatformSpecificDirectory )
         {
            // Multiple output files, one for each platform, so check them all
            PlatformType.PlatformList ret = new PlatformType.PlatformList();

            foreach ( PlatformType.EPlatform platformType in checkPlatforms )
            {
               if ( NeedsCookingForPlatform( assetPath, manager, platformType, forceRecookTime ) == ENeedsCooking.Yes )
               {
                  ret.Add( platformType );
               }
            }

            return ret;
         }
         else
         {
            if ( NeedsCookingForPlatform( assetPath, manager, null, forceRecookTime ) == ENeedsCooking.Yes )
            {
               return checkPlatforms.Clone();
            }
            else
            {
               return new PlatformType.PlatformList();
            }
         }
      }

      /// <summary>
      /// This function determines if the given asset needs to be cooked
      /// </summary>
      /// <param name="assetPath"></param>
      /// <param name="manager"></param>
      /// <returns></returns>
      public static ENeedsCooking NeedsCookingOnAnyPlatform(string assetPath, Manager manager, DateTime? forceRecookTime)
		{ 			
         AssetType type = AssetTypes.GetAssetTypeForSourceAsset(assetPath);
         if (type == null)
         {
            // Not a valid asset type
            return ENeedsCooking.NotACookableAsset;
         }

         if ( type.CooksToPlatformSpecificDirectory )
         {
            // Multiple output files, one for each platform, so check them all
            foreach ( PlatformType.EPlatform platformType in PlatformType.CookPlatforms )
            {
               if ( NeedsCookingForPlatform( assetPath, manager, platformType, forceRecookTime ) == ENeedsCooking.Yes )
               {
                  return ENeedsCooking.Yes;
               }
            }
         }
         else
         {
            // One output file for all platforms, so just check that.
            return NeedsCookingForPlatform( assetPath, manager, null, forceRecookTime );
         }

         return ENeedsCooking.No;
		}

      static private ENeedsCooking NeedsCookingForPlatform(string assetPath, Manager manager, PlatformType.EPlatform? platformType, DateTime? forceRecookTime)
      {
         string systemPath = manager.GetSystemPath(assetPath);
         string cookedPath = Manager.GetCookedAssetPath(systemPath, platformType);

         // if output file doesn't exist, this file needs cooking
         if (!File.Exists(cookedPath))
         {
            return ENeedsCooking.Yes;
         }

         // If output file is 0 bytes, the file needs cooking
         if ( FileUtils.GetFileSize( cookedPath ) <= 0 )
         {
            return ENeedsCooking.Yes;
         }

         DateTime cookedAssetWriteTime = File.GetLastWriteTime(cookedPath);

         // if the output file exists but is older than the input file, it needs cooking
         if (File.GetLastWriteTime(systemPath) > cookedAssetWriteTime)
         {
            return ENeedsCooking.Yes;
         }

         // Check if we need to force a cook because the asset is too old
         if (forceRecookTime.HasValue && (forceRecookTime > cookedAssetWriteTime))
         {
            return ENeedsCooking.Yes;
         }

         BuildAssetList buildAssets = BuildAssets.GetAssets(assetPath, manager, platformType);

         // Make sure that cooked version matches what we can cook
         AssetType assetType = AssetTypes.GetAssetTypeForSourceAsset(assetPath);
         if (assetType != null)
         {
            if (assetType.Version != buildAssets.mVersion)
            {
               return ENeedsCooking.Yes;
            }
         }

         // Check for input src assets used to cook.
         foreach (BuildAsset buildAsset in buildAssets.mAssets)
         {
            string buildAssetSystemPath = manager.GetSystemPath(buildAsset.mAssetPath);
            if (File.GetLastWriteTime(buildAssetSystemPath) > cookedAssetWriteTime)
            {
               // An asset it depends on is newer, needs cooking
               return ENeedsCooking.Yes;
            }

            if (buildAsset.mRecookOptions == BuildAsset.ERecookOptions.kRO_NewerOrDoesntExist)
            {
               if (!File.Exists(buildAssetSystemPath))
               {
                  // Asset doesn't exist, recook.
                  return ENeedsCooking.Yes;

               }
            }
         }

         // Check for updated meta data.
         string systemMetaDataPath = manager.GetMetaDataSystemPath(assetPath);
         if (File.Exists(systemMetaDataPath))
         {
            // Has meta data
            if (File.GetLastWriteTime(systemMetaDataPath) > cookedAssetWriteTime)
            {
               // Meta data is newer than cooked asset, needs cooking
               return ENeedsCooking.Yes;
            }
         }

         return ENeedsCooking.No;         
      }
   
      public static void MarkSourceAssetAsGenerated(string assetPath, Manager manager)
      {
         FileStream stream = System.IO.File.Create(manager.GetSystemPath(assetPath) + ".generated");
         stream.Dispose();
      }

      public static bool IsSourceAssetGenerated(string assetPath, Manager manager)
      {
         return System.IO.File.Exists(manager.GetSystemPath(assetPath) + ".generated");
      }

      public static bool CookAssets( List<string> assets, PlatformType.PlatformList platforms, ECookAssetsMode mode )
      {
         return CookAssetsInternal( assets, platforms, String.Empty, mode );
      }

      public static bool CookAssetsInternal(List<string> assets, PlatformType.PlatformList platforms, String hardlinkPath, ECookAssetsMode mode)
      {
         if (assets.Count == 0)
            return true;

         System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
         startInfo.UseShellExecute = false;
         startInfo.CreateNoWindow = true;
         startInfo.FileName = "AssetTool.exe";

         startInfo.Arguments = "";

         if( (mode & ECookAssetsMode.DisplayGUI) != 0)
         {
            startInfo.Arguments = "/g";
         }

         if( (mode & ECookAssetsMode.ForceCook) != 0)
         {
            startInfo.Arguments += " /f";
         }

         if ( hardlinkPath != "" )
         {
            startInfo.Arguments += String.Format( " \"/h{0}\"", hardlinkPath );
         }

         foreach (PlatformType.EPlatform platform in platforms)
         {
            string typeId = PlatformType.GetPlatformTypeForEnum(platform).TypeString;
            startInfo.Arguments += " /p" + typeId;
         }

         foreach (string asset in assets)
         {
            startInfo.Arguments += " \"" + asset + "\"";
         }

         using (System.Diagnostics.Process p = new System.Diagnostics.Process())
         {
            p.StartInfo = startInfo;
            if ( p.Start() )
            {
               if ( ( mode & ECookAssetsMode.WaitForFinish ) != 0 )
               {
                  p.WaitForExit();
                  return p.ExitCode == 0;
               }
               else
               {
                  return true;
               }
            }
            else
            {
               return false;
            }
         }
      }

      public static bool CookAssetsWin32(List<string> assets, ECookAssetsMode mode)
      {
         return CookAssets(assets, PlatformType.PlatformList.FromPlatform(PlatformType.EPlatform.kWin32), mode);
      }

      public static bool CookAssetsAllPlatforms(List<string> assets, ECookAssetsMode mode)
      {
         return CookAssets(assets, PlatformType.CookPlatforms, mode);
      }

      public static bool CookAssets(List<string> assets, PlatformType.EPlatform platform, ECookAssetsMode mode)
      {
         return CookAssets(assets, PlatformType.PlatformList.FromPlatform(platform), mode);
      }

      public static bool CookAssetsAndHardLink( List<string> assets, PlatformType.EPlatform platform, String hardlinkPath, ECookAssetsMode mode )
      {
         return CookAssetsInternal( assets, PlatformType.PlatformList.FromPlatform( platform ), hardlinkPath, mode );
      }

      public static bool EditMetaData(List<string> assets)
      {
         if (assets.Count == 0)
            return true;

         System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
         startInfo.UseShellExecute = false;
         startInfo.CreateNoWindow = true;
         startInfo.FileName = "AssetTool.exe";

         startInfo.Arguments = "/e";

         foreach (string asset in assets)
         {
            startInfo.Arguments += " \"" + asset + "\"";
         }

         using (System.Diagnostics.Process p = new System.Diagnostics.Process())
         {
            p.StartInfo = startInfo;
            if ( p.Start() )
            {
               p.WaitForExit();

               return p.ExitCode == 0;
            }
            else
            {
               return false;
            }
         }
      }
   }

   public class PackageAsset
   {
      public enum EPackageType
      {
         kPackageAndLoad,     // Gets loaded as part of the asset dependency tree at runtime
         kPackageOnly,
      }

      public enum EAssetSource
      {
         kRepository,
         kGenerated
      }

      public PackageAsset(PackageAsset packageAsset)
         : this(packageAsset.mAssetPath, packageAsset.mPackageType, packageAsset.mAssetSource )
      {
      }

      public PackageAsset(string assetPath, EPackageType packageType, EAssetSource assetSource )
      {
         mAssetPath = assetPath.ToLowerInvariant();
         mPackageType = packageType;
         mAssetSource = assetSource;
      }

      public static PackageAsset StandardRepositoryAsset( string assetPath )
      {
         return new PackageAsset(assetPath, EPackageType.kPackageAndLoad, EAssetSource.kRepository);
      }

      public static PackageAsset RepositoryAsset(string assetPath, EPackageType packageType)
      {
         return new PackageAsset(assetPath, packageType, EAssetSource.kRepository);
      }

      public override bool Equals(object obj)
      {
         if ( obj is PackageAsset )
         {
            PackageAsset other = obj as PackageAsset;
            bool bEqual = ( 
               this.mAssetPath.Equals( other.mAssetPath ) && 
               ( this.mPackageType == other.mPackageType ) && 
               ( mAssetSource == other.mAssetSource ) );
            return bEqual;
         }
         else
         {
            return base.Equals( obj );
         }
      }

      public override int GetHashCode()
      {
         return this.mAssetPath.GetHashCode() + mPackageType.GetHashCode() + mAssetSource.GetHashCode();
      }

      public string        mAssetPath;
      public EPackageType  mPackageType;
      public EAssetSource  mAssetSource;
   }

   public class PackageAssets
   {
      public const string kExtension = ".assets";
      public const string kCookedExtension = ".cassets"; // Not technically cooked, but platform ready.

      /// <summary>
      /// This function returns a list of asset paths of the package assets for the given input asset path.
      /// Note that only package assets for the specified platform will be returned. 
      /// </summary>
      /// <param name="platform">If the asset path references a platform specific asset this must be provided.</param>
      /// <returns></returns>
      public static List<PackageAsset> GetAssets(string assetPath, Manager manager, PlatformType.EPlatform? platform)
      {
         List<PackageAsset> packAssets = new List<PackageAsset>();

         string assetsPath = GetPath(assetPath, manager, platform);
         
         XmlDocument doc = XmlNodeHelpers.LoadDocumentOrNull( assetsPath );
         if ( doc != null )
         {
            XmlNodeList nodes = doc.SelectNodes("//PackageAssets/Asset");

            foreach (XmlNode node in nodes)
            {
               XmlElement element = (XmlElement)node;

               PackageAsset.EPackageType packageType = (node.Attributes.GetNamedItem("packageOnly") != null) ? PackageAsset.EPackageType.kPackageOnly : PackageAsset.EPackageType.kPackageAndLoad;
               PackageAsset.EAssetSource assetSource = Tools.Common.Misc.ParseBool( element.GetAttribute( "isGenerated" ) ) ? PackageAsset.EAssetSource.kGenerated : PackageAsset.EAssetSource.kRepository;

               packAssets.Add(new PackageAsset(node.InnerText, packageType, assetSource ));
            }
         }

         return packAssets;
      }

      /// <summary>
      /// This function returns a list of asset paths of the package assets for the given input asset path. This is for ALL platforms.
      /// </summary>
      /// <param name="assetPath"></param>
      /// <param name="manager"></param>
      /// <returns></returns>
      public static List<PackageAsset> GetAssetsAllPlatforms(string assetPath, Manager manager)
      {
         List<PackageAsset> packAssets = new List<PackageAsset>();
         foreach ( PlatformType.EPlatform platform in PlatformType.CookPlatforms )
         {
            packAssets.AddRange(GetAssets(assetPath, manager, platform));
         }

         packAssets = RemoveDuplicateAssetEntries(packAssets);
         
         return packAssets;
      }

      /// <summary>
      /// Write out assets paths given in package assets to .assets file for the cooked version of "assetPath"
      /// </summary>
      public static void WriteAssets(string assetPath, IEnumerable<PackageAsset> packageAssets, Manager manager)
      {
         WriteAssets(assetPath, packageAssets, manager, null);
      }

      /// <summary>
      /// Write out assets paths given in package assets to .assets file for the cooked version of "assetPath".
      /// Platform specific version.
      /// </summary>
      public static void WriteAssets(string assetPath, IEnumerable<PackageAsset> packageAssets, Manager manager, PlatformType.EPlatform? platform)
      {
         string assetsPath = GetPath(assetPath, manager, platform);

         XmlDocument doc = new XmlDocument();
         XmlElement packageAssetsElement = doc.CreateElement("PackageAssets");
         doc.AppendChild(packageAssetsElement);

         foreach (PackageAsset packageAsset in RemoveDuplicateAssetEntries(packageAssets))
         {
            XmlElement packageAssetElement = doc.CreateElement("Asset");
            packageAssetElement.InnerText = packageAsset.mAssetPath;
            
            // Add cooked asset attribute to make it easy to parse for game resource loading.
            XmlAttribute cookedAsset = doc.CreateAttribute("cookedAsset");
            cookedAsset.InnerText = Manager.GetCookedAssetPath(packageAsset.mAssetPath);
            packageAssetElement.SetAttributeNode(cookedAsset);

            // Add attribute for packageOnly if required
            if (packageAsset.mPackageType == PackageAsset.EPackageType.kPackageOnly)
            {
               XmlAttribute packageOnly = doc.CreateAttribute("packageOnly");
               packageOnly.InnerText = "1";
               packageAssetElement.SetAttributeNode(packageOnly);
            }

            if (packageAsset.mAssetSource == PackageAsset.EAssetSource.kGenerated)
            {
               packageAssetElement.SetAttribute("isGenerated", true.ToString() );
            }

            packageAssetsElement.AppendChild(packageAssetElement);
         }


         // Unhide the file, save the document, and hide it again
         FileUtils.ChangeFileAttributes(assetsPath, FileAttributes.Hidden, 0);
         doc.Save( assetsPath );
         FileUtils.ChangeFileAttributes(assetsPath, FileAttributes.Hidden, FileAttributes.Hidden);
      }

      /// <summary>
      /// Returns path to ".assets" file for given asset path, this will return the cooked system path with the .assets extension added
      /// </summary>
      public static string GetPath(string assetPath, Manager manager, PlatformType.EPlatform? platform)
      {
         string systemPath = manager.GetSystemPath(assetPath);

         string cookedPath = Manager.GetCookedAssetPath(systemPath, platform);
         string assetsPath = cookedPath + kExtension;
         return assetsPath;
      }

      /// <summary>
      /// Creates a new list of assets with duplicates removed.
      /// </summary>
      private static List<PackageAsset> RemoveDuplicateAssetEntries(IEnumerable<PackageAsset> packageAssets)
      {
         // Remove duplicate entries
         Dictionary<string, PackageAsset> sortedPackageAssets = new Dictionary<string, PackageAsset>();

         foreach (PackageAsset packageAsset in packageAssets)
         {
            if (!sortedPackageAssets.ContainsKey(packageAsset.mAssetPath))
            {
               sortedPackageAssets.Add(packageAsset.mAssetPath, packageAsset);
            }
         }

         // Rebuild list
         List<PackageAsset> outPackageAssets = new List<PackageAsset>(sortedPackageAssets.Values);
         return outPackageAssets;
      }
   }

   public class BuildAsset
   {
      public enum ERecookOptions
      {
         kRO_Newer,                 // Recook if build file is newer
         kRO_NewerOrDoesntExist,    // Recook if build file is newer or doesn't exist. Useful for generated assets such as lit models.
      }

      public enum EAssetSource
      {
         kRepository,
         kGenerated
      }

      public string mId;
      public string mAssetPath;
      public ERecookOptions mRecookOptions;
      public EAssetSource mAssetSource;

      public BuildAsset(string id, string assetPath, ERecookOptions recookOptions, EAssetSource assetSource )
      {
         mId = id;
         mAssetPath = assetPath;
         mRecookOptions = recookOptions   ;
         mAssetSource = assetSource;
      }

      public static BuildAsset StandardCookingAsset( string id, string assetPath )
      {
         return new BuildAsset( id, assetPath, ERecookOptions.kRO_NewerOrDoesntExist, EAssetSource.kRepository );
      }

      public static BuildAsset RepositoryAsset(string id, string assetPath, ERecookOptions recookOptions)
      {
         return new BuildAsset(id, assetPath, recookOptions, EAssetSource.kRepository);
      }

      public override bool Equals(object obj)
      {
         if ( obj is BuildAsset )
         {
            BuildAsset rhs = (BuildAsset) obj;

            return mId == rhs.mId && mAssetPath == rhs.mAssetPath && mRecookOptions == rhs.mRecookOptions && mAssetSource == rhs.mAssetSource;
         }
         else
         {
            return base.Equals( obj );
         }
      }

      public override int GetHashCode()
      {
         return mId.GetHashCode() + mAssetPath.GetHashCode() + mRecookOptions.GetHashCode()  + mAssetSource.GetHashCode();
      }
   }

   public class BuildAssetList
   {
      public List<BuildAsset> mAssets = new List<BuildAsset>();
      public int mVersion = 0;
   }

   public class BuildAssets
   {
      public const string kExtension = ".build"; 

      /// <summary>
      /// This function returns a list of assets paths of the build assets for the given input asset path
      /// </summary>
      public static BuildAssetList GetAssets(string assetPath, Manager manager, PlatformType.EPlatform? platform)
      {
         BuildAssetList buildAssetList = new BuildAssetList();
         List<BuildAsset> buildAssets = buildAssetList.mAssets;

         string buildFilePath = GetPath(assetPath, manager, platform);
         XmlDocument doc = XmlNodeHelpers.LoadDocumentOrNull( buildFilePath );
         if ( doc != null )
         {
            XmlElement buildAssetsElement = (XmlElement) doc.SelectSingleNode( "BuildAssets" );
            if ( buildAssetsElement != null && buildAssetsElement.HasAttribute( "version" ) )
            {
               buildAssetList.mVersion = int.Parse( buildAssetsElement.GetAttribute( "version" ) );
            }
            XmlNodeList nodes = doc.SelectNodes( "BuildAssets/Asset" );

            foreach ( XmlElement node in nodes )
            {
               BuildAsset.ERecookOptions recookOptions = BuildAsset.ERecookOptions.kRO_Newer;

               if ( node.HasAttribute( "recookOptions" ) )
               {
                  recookOptions = (BuildAsset.ERecookOptions) System.Enum.Parse( typeof( BuildAsset.ERecookOptions ), node.GetAttribute( "recookOptions" ), false );
               }

               BuildAsset.EAssetSource assetSource = Tools.Common.Misc.ParseBool( node.GetAttribute( "isGenerated" ) )
                  ? BuildAsset.EAssetSource.kGenerated : BuildAsset.EAssetSource.kRepository;
               buildAssets.Add( new BuildAsset( node.GetAttribute( "id" ), node.InnerText, recookOptions, assetSource ) );
            }
         }

         return buildAssetList;
      }

      public static List<BuildAsset> GetAssetsAllPlatforms(string assetPath, Manager manager)
      {
         List<BuildAsset> buildAssets = new List<BuildAsset>();
         foreach ( PlatformType.EPlatform platform in PlatformType.CookPlatforms )
         {
            buildAssets.AddRange(GetAssets(assetPath, manager, platform).mAssets);
         }

         buildAssets = RemoveDuplicateAssetEntries(buildAssets);

         return buildAssets;
      }

      /// <summary>
      /// Creates a new list of assets with duplicates removed.
      /// </summary>
      private static List<BuildAsset> RemoveDuplicateAssetEntries(List<BuildAsset> buildAssets)
      {
         // Remove duplicate entries
         Dictionary<string, BuildAsset> sortedBuildAssets = new Dictionary<string, BuildAsset>();

         foreach (BuildAsset buildAsset in buildAssets)
         {
            if (!sortedBuildAssets.ContainsKey(buildAsset.mAssetPath))
            {
               sortedBuildAssets.Add(buildAsset.mAssetPath, buildAsset);
            }
         }

         // Rebuild list
         return new List<BuildAsset>(sortedBuildAssets.Values);
      }

      /// <summary>
      /// Write out assets paths given in build assets to .build file for the cooked version of "assetPath"
      /// </summary>
      /// <param name="assetPath"></param>
      /// <param name="packageAssets"></param>
      /// <param name="manager"></param>
      public static void WriteAssets(string assetPath, IEnumerable<BuildAsset> buildAssets, Manager manager, PlatformType.EPlatform? platform)
      {
         string buildFilePath = GetPath(assetPath, manager, platform);
         AssetType assetType = AssetTypes.GetAssetTypeForSourceAsset(assetPath);
         int version = 0;

         if (assetType != null)
         {
            version = assetType.Version;
         }

         // Delete .build file if no dependencies to improve cooking performance.
         if ( buildAssets.GetEnumerator().MoveNext() == false && version == 0 )
         {
            File.Delete(buildFilePath);
            return;
         }

         XmlDocument doc = new XmlDocument();
         XmlElement buildAssetsElement = doc.CreateElement("BuildAssets");

         if (version != 0)
         {
            buildAssetsElement.SetAttribute("version", version.ToString());
         }

         doc.AppendChild(buildAssetsElement);

         foreach (BuildAsset buildAsset in buildAssets)
         {
            if ( !manager.IsValidRepositoryRelativePath( buildAsset.mAssetPath ) )
            {
               throw new AssetSystemException( String.Format( "Asset '{0}' trying to write an build asset path '{1}' for build id {2}",
                  assetPath, buildAsset.mAssetPath, buildAsset.mId ) );
            }

            XmlElement buildAssetElement = doc.CreateElement("Asset");
            buildAssetElement.SetAttribute("id", buildAsset.mId);
            buildAssetElement.InnerText = buildAsset.mAssetPath;
            buildAssetsElement.AppendChild(buildAssetElement);
            buildAssetElement.SetAttribute("recookOptions", System.Enum.GetName(typeof(BuildAsset.ERecookOptions), buildAsset.mRecookOptions));

            if (buildAsset.mAssetSource == BuildAsset.EAssetSource.kGenerated)
            {
               buildAssetElement.SetAttribute("isGenerated", true.ToString());
            }
         }

         // Unhide the file, save the document, and hide it again
         FileUtils.ChangeFileAttributes(buildFilePath, FileAttributes.Hidden, 0);
         doc.Save(buildFilePath);
         FileUtils.ChangeFileAttributes(buildFilePath, FileAttributes.Hidden, FileAttributes.Hidden);
      }
      
      /// <summary>
      /// This function returns the build asset matching the id from the list of passed in assets
      /// </summary>
      public static BuildAsset GetById(string id, List<BuildAsset> assets)
      {
         foreach (BuildAsset asset in assets)
         {
            if (asset.mId == id)
               return asset;
         }

         return null;
      }

      /// <summary>
      /// Returns path to ".build" file for given asset path, this will return the cooked system path with the .build extension added.
      /// Will return an empty string if not a valid source asset type.
      /// </summary>
      private static string GetPath(string assetPath, Manager manager, PlatformType.EPlatform? platform)
      {
         string systemPath = manager.GetSystemPath(assetPath);

         string cookedPath = Manager.GetCookedAssetPath(systemPath, platform);
         if( cookedPath == "" )
         {
            // Return empty string, easier to handle than an exception.
            return String.Empty;
         }
         string assetsPath = cookedPath + kExtension;
         return assetsPath;
      }
   }

   public class MetaData
   {
      /// <summary>
      /// Returns full meta data if it exists.
      /// </summary>
      public static XmlDocument GetMetaData(string assetPath, Manager manager, Tools.Property.Source.IPropertyDataSource propertySource)
      {
         string metaDataFilename = manager.GetMetaDataSystemPath(assetPath);
         if (!File.Exists(metaDataFilename))
         {
            // No meta data
            return null;
         }

         XmlDocument mergedDoc = new XmlDocument();
         mergedDoc.Load(metaDataFilename);

         Tools.Property.Build.MergePropertiesV2.Merge_Inplace(mergedDoc, propertySource);

         return mergedDoc;
      }

      /// <summary>
      /// Returns full meta data if it exists, otherwise creates new meta data.
      /// </summary>
      public static XmlDocument CreateMetaData(string assetPath, Manager manager, Tools.Property.Source.IPropertyDataSource propertySource)
      {
         AssetType type = AssetTypes.GetAssetTypeForSourceAsset(assetPath);
         if (type == null)
         {
            return null;
         }

         if (propertySource.FindPropertyContainerById(type.TypeString) == null)
         {
            return null;
         }

         string metaDataPath = manager.GetMetaDataSystemPath(assetPath);

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

         MergePropertiesV2.Merge_Inplace(metaData, propertySource);
         return metaData;
      }

      /// <summary>
      /// Saves meta data to a file.
      /// </summary>
      public static void SaveMetaData(string assetPath, Manager manager, XmlDocument metaData)
      {
         XmlDocument unmerged = MergePropertiesV2.Unmerge(metaData);
         string metaDataPath = manager.GetMetaDataSystemPath(assetPath);
         unmerged.Save(metaDataPath);
      }
   }

   public class CookedAssetInfo
   {
      public static readonly String kRootXmlElementName = "CookedAssetInfo";
      private static readonly String kNTFSStreamName = "bpe_info.xml";

      private enum EOpenType
      {
         kRead,
         kWrite
      };

      [Flags]
      private enum EFileAccess : uint
      {
         /// <summary>
         ///
         /// </summary>
         GenericRead = 0x80000000,
         /// <summary>
         ///
         /// </summary>
         GenericWrite = 0x40000000,
         /// <summary>
         ///
         /// </summary>
         GenericExecute = 0x20000000,
         /// <summary>
         ///
         /// </summary>
         GenericAll = 0x10000000
      }

      [Flags]
      private enum EFileShare : uint
      {
         /// <summary>
         ///
         /// </summary>
         None = 0x00000000,
         /// <summary>
         /// Enables subsequent open operations on an object to request read access.
         /// Otherwise, other processes cannot open the object if they request read access.
         /// If this flag is not specified, but the object has been opened for read access, the function fails.
         /// </summary>
         Read = 0x00000001,
         /// <summary>
         /// Enables subsequent open operations on an object to request write access.
         /// Otherwise, other processes cannot open the object if they request write access.
         /// If this flag is not specified, but the object has been opened for write access, the function fails.
         /// </summary>
         Write = 0x00000002,
         /// <summary>
         /// Enables subsequent open operations on an object to request delete access.
         /// Otherwise, other processes cannot open the object if they request delete access.
         /// If this flag is not specified, but the object has been opened for delete access, the function fails.
         /// </summary>
         Delete = 0x00000004
      }

      private enum ECreationDisposition : uint
      {
         /// <summary>
         /// Creates a new file. The function fails if a specified file exists.
         /// </summary>
         New = 1,
         /// <summary>
         /// Creates a new file, always.
         /// If a file exists, the function overwrites the file, clears the existing attributes, combines the specified file attributes,
         /// and flags with FILE_ATTRIBUTE_ARCHIVE, but does not set the security descriptor that the SECURITY_ATTRIBUTES structure specifies.
         /// </summary>
         CreateAlways = 2,
         /// <summary>
         /// Opens a file. The function fails if the file does not exist.
         /// </summary>
         OpenExisting = 3,
         /// <summary>
         /// Opens a file, always.
         /// If a file does not exist, the function creates a file as if dwCreationDisposition is CREATE_NEW.
         /// </summary>
         OpenAlways = 4,
         /// <summary>
         /// Opens a file and truncates it so that its size is 0 (zero) bytes. The function fails if the file does not exist.
         /// The calling process must open the file with the GENERIC_WRITE access right.
         /// </summary>
         TruncateExisting = 5
      }

      [Flags]
      private enum EFileAttributes : uint
      {
         Readonly = 0x00000001,
         Hidden = 0x00000002,
         System = 0x00000004,
         Directory = 0x00000010,
         Archive = 0x00000020,
         Device = 0x00000040,
         Normal = 0x00000080,
         Temporary = 0x00000100,
         SparseFile = 0x00000200,
         ReparsePoint = 0x00000400,
         Compressed = 0x00000800,
         Offline = 0x00001000,
         NotContentIndexed = 0x00002000,
         Encrypted = 0x00004000,
         Write_Through = 0x80000000,
         Overlapped = 0x40000000,
         NoBuffering = 0x20000000,
         RandomAccess = 0x10000000,
         SequentialScan = 0x08000000,
         DeleteOnClose = 0x04000000,
         BackupSemantics = 0x02000000,
         PosixSemantics = 0x01000000,
         OpenReparsePoint = 0x00200000,
         OpenNoRecall = 0x00100000,
         FirstPipeInstance = 0x00080000
      }

      [DllImport("kernel32.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
      private static extern SafeFileHandle CreateFile(
            string lpFileName,
            uint dwDesiredAccess,
            uint dwShareMode,
            IntPtr SecurityAttributes,
            uint dwCreationDisposition,
            uint dwFlagsAndAttributes,
            IntPtr hTemplateFile
            );

      private static SafeFileHandle OpenFileStreamWin32(string cookedPath, EOpenType ot)
      {
         // First, let's generate the full on name normalized, then add the stream
         // Windows requires the full path to open streams
         string fullPath = System.IO.Path.GetFullPath(cookedPath);
         string pathWithStream = fullPath + ":" + kNTFSStreamName;

         uint desiredAccess = (uint)((ot == EOpenType.kRead) ? EFileAccess.GenericRead : EFileAccess.GenericWrite);
         uint sharedAccess = (uint)EFileShare.Delete | (uint)EFileShare.Read;
         uint creationDisposition = (uint)((ot == EOpenType.kRead) ? ECreationDisposition.OpenExisting : ECreationDisposition.CreateAlways);

         SafeFileHandle handle = CreateFile(pathWithStream, desiredAccess, sharedAccess, IntPtr.Zero, creationDisposition, (uint)EFileAttributes.Normal, IntPtr.Zero);

         return handle;
      }

      public class Exception : AssetSystemException
      {
         public Exception( String s ) : base( s )
         {
         }
      };

      public class InvalidDocumentException : Exception
      {
         public InvalidDocumentException(String s)
            : base(s)
         {
         }
      }

      public class InvalidVersionException : Exception
      {
         public InvalidVersionException(int version, int versionMin, int versionMax)
            : base(String.Format("Bad cooked asset version {0}, min {1} max {2}", version, versionMin, versionMax))
         {
            mVersion = version;
            mVersionMin = versionMin;
            mVersionMax = versionMax;
         }

         public int mVersion, mVersionMin, mVersionMax;
      }

      /// <summary>
      /// Returns full meta data if it exists, or null on error
      /// </summary>
      public static XmlDocument Load(string cookedPath)
      {
         using (SafeFileHandle fileHandle = OpenFileStreamWin32(cookedPath, EOpenType.kRead))
         {
            if (!fileHandle.IsInvalid)
            {
               using (System.IO.FileStream stream = new FileStream(fileHandle, FileAccess.Read))
               {
                  try
                  {
                     XmlDocument document = new XmlDocument();
                     document.Load(stream);

                     return document;
                  }
                  catch (XmlException e)
                  {
                     Console.WriteLine("Error loading cooked metadata for {0}:\n{1}", cookedPath, e.Message);
                  }
               }
            }
         }

         return null;
      }

      /// <summary>
      /// Saves meta data to a file.
      /// </summary>
      public static bool Save(string cookedPath, XmlDocument document)
      {
         // First let's validate the document by "getting" the root element if it fails,
         // it will throw exceptions about the document object.  This is preferred to returning
         // false, since passing in an invalid document is stupid to do and deserves swift punishment.
         GetRootElement(document, -1, -1);

         using (SafeFileHandle fileHandle = OpenFileStreamWin32(cookedPath, EOpenType.kWrite))
         {
            if (fileHandle.IsInvalid)
            {
               Console.WriteLine("WARNING: Cannot open cooked metadata for {0}\n", cookedPath);
            }
            else
            {
               using (System.IO.FileStream stream = new FileStream(fileHandle, FileAccess.Write))
               {
                  document.Save(stream);
                  return true;
               }
            }
         }

         return false;
      }

      /// <summary>
      /// Creates the root node element.
      /// </summary>
      public static XmlElement CreateRootElement(XmlDocument document, int version)
      {
         XmlElement root = document.CreateElement(kRootXmlElementName);
         root.SetAttribute("version", version.ToString() );

         return root;
      }

      /// <summary>
      /// Gets the root element in a version range, or -1 for dont care in each
      /// Throws:
      ///   InvalidVersionException - If the version is out of range
      ///   InvalidDocumentException - If the document is out of whack
      /// </summary>
      public static XmlElement GetRootElement(XmlDocument document, int versionMin, int versionMax)
      {
         // Allow for a null document and return a null node
         if (document != null && document.ChildNodes.Count == 1)
         {
            try
            {
               XmlElement element = document.ChildNodes[0] as XmlElement;
               int version = element.HasAttribute("version") ? int.Parse(element.GetAttribute("version")) : -1;

               if ((versionMin == -1 || version >= versionMin) && (versionMax == -1 || version <= versionMax))
               {
                  return element;
               }
               else
               {
                  throw new InvalidVersionException(version, versionMin, versionMax);
               }
            }
            catch (System.InvalidCastException)
            {
            }
         }

         throw new InvalidDocumentException("An invalid XML Document was passed in");
      }

      private static String GetKeyInnerText( XmlElement root, String nodeName )
      {
         XmlElement node = root.SelectSingleNode(nodeName) as XmlElement;

         if (node == null)
         {
            return "";
         }
         else
         {
            return node.InnerText;
         }
      }

      private static void SetKeyInnerText(XmlElement root, String nodeName, String text)
      {
         XmlElement node = (XmlElement) root.SelectSingleNode(nodeName);

         if (node == null)
         {
            if (text != "")
            {
               XmlElement newElement = root.OwnerDocument.CreateElement(nodeName);
               newElement.InnerText = text;
               root.AppendChild(newElement);
            }
         }
         else
         {
            if (text == "")
            {
               // Delete the description if it's empty
               root.RemoveChild(node);
            }
            else
            {
               node.InnerText = text;
            }
         }
      }

      public static String GetShortDescription( XmlElement root )
      {
         return GetKeyInnerText(root, "ShortDescription");
      }

      public static String GetLongDescription( XmlElement root )
      {
         return GetKeyInnerText(root, "LongDescription");
      }

      public static void SetShortDescription(XmlElement root, String text)
      {
         SetKeyInnerText(root, "ShortDescription", text);
      }

      public static void SetLongDescription(XmlElement root, String text)
      {
         SetKeyInnerText(root, "LongDescription", text);
      }
   }
}

