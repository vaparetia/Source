using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.AssetSystem
{
   /// <summary>
   /// Asset manager, deals with everything that is asset system related.
   /// </summary>
   public class Manager
   {
      string mRepositoryRoot;

      public Manager()
      {
         mRepositoryRoot = GetNormalizedPath( System.Environment.GetEnvironmentVariable("BPE_REPOSITORY") );
      }

      public Manager(string repRoot)
      {
         mRepositoryRoot = repRoot;
      }

      /// <summary>
      /// Filepath for where the repository is located
      /// </summary>
      public string RepositoryRoot
      {
         get { return mRepositoryRoot; }
      }

      /// <summary>
      /// Converts asset path to file system path, also allows for system relative paths to be converted.
      /// </summary>
      /// <param name="assetPath">Asset path</param>
      /// <returns>File system path for the given asset path</returns>
      public string GetSystemPath(string systemOrAssetPath)
      {
         // empty path stays empty
         if (systemOrAssetPath == "")
            return "";

         // turns slashes into forward slashes
         string path = GetNormalizedPath(systemOrAssetPath);
         
         // check if it's a repository relative path
         if( path.StartsWith("$") )
         {
            return mRepositoryRoot + path.Substring(1);
         }
         else
         {
            string absolutePath = GetNormalizedPath(System.IO.Path.GetFullPath(path));

            // Do this check rather than IsSystemPathInsideRepository to save on expensive Path.GetFullPath.
            if( absolutePath.StartsWith(mRepositoryRoot, StringComparison.CurrentCultureIgnoreCase) )
            {
               return absolutePath;
            }
            else
            {
               throw new AssetSystemException(string.Format("Path '{0}' is not inside the repository {1}.", absolutePath, mRepositoryRoot));
            }
         }
      }

      /// <summary>
      /// Converts asset path to file system path just like GetSystemPath, but all slashes are backslashes
      /// </summary>
      /// <param name="assetPath">Asset path</param>
      /// <returns>File system path for the given asset path</returns>
      public string GetSystemPathWithBackslashes( string systemOrAssetPath )
      {
         String systemPath = GetSystemPath( systemOrAssetPath );

         return systemPath.Replace( '/', '\\' );
      }

      /// <summary>
      /// Helper function that returns system path to asset meta data.
      /// </summary>
      /// <param name="assetPath">Asset or system path</param>
      /// <returns>System path of meta data. Meta data may not exist.</returns>
      public string GetMetaDataSystemPath(string assetPath)
      {
         return GetSystemPath(assetPath) + ".meta";
      }

      /// <summary>
      /// Converts file system path to asset path. If already an asset path, then no conversion occurs.
      /// </summary>
      /// <param name="filePath">File system path of asset, must be inside repository, otherwise exception is thrown.</param>
      /// <returns>Asset path of given asset</returns>
      public string GetRepositoryRelativePath(string filePath)
      {
         if( filePath == "" )
            return "";

         if( IsRepositoryRelativePath(filePath) )
            return filePath;

         string absolutePath = GetNormalizedPath(System.IO.Path.GetFullPath(filePath));

         // Do this check rather than IsSystemPathInsideRepository to save on expensive Path.GetFullPath.
         if( absolutePath.StartsWith(mRepositoryRoot, StringComparison.CurrentCultureIgnoreCase) )
         {
            // Check for absolute path being the repository root
            if (absolutePath.Length == mRepositoryRoot.Length)
            {
               return "$/";
            }
            return "$/" + absolutePath.Substring(mRepositoryRoot.Length + 1);
         }
         else
         {
            throw new AssetSystemException( String.Format( "'{0}' (abs '{1}') not inside of repository '{2}'", filePath, absolutePath, mRepositoryRoot ) );
         }
      }

      /// <summary>
      /// This function checks whether the given path is inside the repository.
      /// </summary>
      /// <param name="path">File system path of asset</param>
      /// <returns>True if asset is inside repository, false otherwise.</returns>
      public bool IsSystemPathInsideRepository(string path)
      {
         if (!string.IsNullOrEmpty(path))
         {
            string normalized = GetNormalizedPath(System.IO.Path.GetFullPath(path));
            if (normalized.StartsWith(mRepositoryRoot, StringComparison.CurrentCultureIgnoreCase))
            {
               return true;
            }
         }
         return false;
      }          

      /// <summary>
      /// Checks to see if path is an asset path.
      /// </summary>
      /// <param name="path">path to asset (either asset path or file system path)</param>
      /// <returns>True if path is an asset path, false if it is any other (for example file system path).</returns>
      public static bool IsRepositoryRelativePath(string path)
      {
         return NormalizeRepositoryRelativePath(path) != null;
      }

      /// <summary>
      /// Normalizes a repository relative asset path if it is one.
      /// </summary>
      /// <param name="path">path to asset (either asset path or file system path)</param>
      /// <returns>Normalized asset path if filePath is an asset path, null if it is any other (for example file system path).</returns>
      public static string NormalizeRepositoryRelativePath(string filePath)
      {
         string normalizedPath = GetNormalizedPath(filePath);
         if (normalizedPath.StartsWith("$/"))
         {
            return normalizedPath;
         }
         else
         {
            return null;
         }
      }

      /// <summary>
      /// Converts all backward slashes to forward slashes.
      /// Also fixes multiple slashes, converts to lower case and strips trailing '/'.
      /// Works with system paths and asset paths.
      /// Does not fix '/../' cases.
      /// </summary>
      private static string GetNormalizedPath(string inPath)
      {
         // Fix backslashes
         inPath = inPath.Replace('\\', '/');

         // Make path lower case
         inPath = inPath.ToLowerInvariant();

         string fixedPath = String.Empty;

         // Remove all '//'
         while (fixedPath != inPath)
         {
            if (fixedPath.Length != 0)
            {
               inPath = fixedPath;
            }
            fixedPath = inPath.Replace("//", "/");
         }

         // Strip trailing "/" if it exists
         if (fixedPath.EndsWith("/"))
         {
            fixedPath = fixedPath.Substring(0, fixedPath.Length - 1);
         }

         return fixedPath;
      }

      /// <summary>
      /// returns the cooked output filename for the src input path.
      /// Keeps current path type(system or asset).
      /// Performs no validation on srcPath.
      /// Returns empty path if asset type couldn't be found.
      /// </summary>
      public static string GetCookedAssetPath(string srcPath)
      {
         return GetCookedAssetPath(srcPath, null);
      }

      /// <summary>
      /// Returns the asset paths and the platforms that correspond to that asset.
      /// 
      /// If the asset type does not support multi-platform cooking, the platforms array will be null.
      /// 
      /// On success, assetCookPaths will contain at least one entry.
      /// </summary>
      /// <param name="srcAssetPath">Source Asset whose cooked assets we want</param>
      /// <param name="platform">Platform to scan.  If null, scans all platforms.</param>
      /// <param name="assetCookPaths">OUT - Array of cooked asset paths</param>
      /// <param name="platforms">
      ///   OUT - If asset type cooks to different platforms, an array that corresponds to the 
      ///   entries in the assetCookPaths array to specify the platform for that path.
      /// 
      ///   If asset type cooks to one asset for all platforms, this member is set to null
      /// </param>
      /// <returns>
      ///   Returns true on success and sets assetCookPaths and optionally platforms
      ///   Returns false on failure, and assetCookPaths and platforms is null
      /// </returns>
      public static bool GetCookedAssetPaths( string srcAssetPath, PlatformType.PlatformList platformsIn, out String[] assetCookPaths, out PlatformType.EPlatform[] platforms )
      {
         assetCookPaths = null;
         platforms = null;

         string srcPathNormalized = GetNormalizedPath( srcAssetPath );
         AssetType type = AssetTypes.GetAssetTypeForSourceAsset( srcPathNormalized );
         if ( type != null )
         {
            Dictionary<PlatformType.EPlatform, String> assetPaths = new Dictionary<PlatformType.EPlatform, string>();

            if ( type.CooksToPlatformSpecificDirectory )
            {
               platforms = platformsIn.ToArray();

               assetCookPaths = new String[platforms.Length];
               for ( int i = 0; i < platforms.Length; ++i )
               {
                  assetCookPaths[i] = GetCookedAssetPath( srcAssetPath, platforms[i] );
               }

               return true;
            }
            else
            {
               // If we don't care about the platform, then leave the platforms[] array null
               assetCookPaths = new String[1];
               assetCookPaths[0] = GetCookedAssetPath( srcAssetPath );
               return true;
            }
         }

         return false;
      }

      public static string GetCookedAssetPath(string srcAssetPath, PlatformType.EPlatform? platform)
      {
         string srcPathNormalized = GetNormalizedPath( srcAssetPath );
         AssetType type = AssetTypes.GetAssetTypeForSourceAsset(srcPathNormalized);
         if( type != null )
         {
            if (type.IsCooked)
            {
               return srcPathNormalized;
            }
            else
            {
               string path = System.IO.Path.ChangeExtension( srcPathNormalized, type.CookedExtension);

               if ( type.CooksToPlatformSpecificDirectory )
               {
                  string platformDirectory = "****";
                  if ( platform.HasValue )
                  {
                     platformDirectory = PlatformType.GetPlatformTypeForEnum( platform.Value ).TypeString;
                  }
                  // Insert platform specific directory, directory will be "****" if no platform specified.
                  int lastSeperatorIndex = path.LastIndexOf( '/' );
                  path = path.Insert( lastSeperatorIndex, "/" + platformDirectory );
                  return path;
               }
               else
               {
                  return path;
               }
            }
         }
         
         return string.Empty;
      }

      /// <summary>
      /// Performs a robust check on a src asset path to confirm that it is valid.
      /// Doesn't check for the file existing.
      /// </summary>
      public bool IsValidRepositoryRelativePath(string srcAssetPath)
      {
         string[] assetTypesFilter = null;
         return IsValidRepositoryRelativePath(srcAssetPath, assetTypesFilter);
      }

      /// <summary>
      /// Performs a robust check on a src asset path to confirm that it is valid for asset types filter specified.
      /// Doesn't check for the file existing.
      /// </summary>
      /// <param name="srcAssetPropertyValue">'$/' asset path of src type specified in the asset types filter.</param>
      /// <param name="propertyAssetTypesFilter">';' delimited list of asset types, normally specified by attribute Helpers.kProperty_AssetTypes.</param>
      /// <returns>'true' if valid.</returns>
      public bool IsValidRepositoryRelativePath(string srcAssetPath, string propertyAssetTypesFilter)
      {
         string[] assetTypesFilter = propertyAssetTypesFilter.Split(new char[] {';'}, StringSplitOptions.RemoveEmptyEntries);
         return IsValidRepositoryRelativePath(srcAssetPath, assetTypesFilter);
      }

      /// <summary>
      /// Performs a robust check on a src asset path to confirm that it is valid for asset types filter specified.
      /// Doesn't check for the file existing.
      /// </summary>
      /// <param name="srcAssetPath">'$/' asset path of src type specified in the asset types filter.</param>
      /// <param name="assetTypesFilter">List of four CC asset types.</param>
      /// <returns>'true' if valid.</returns>
      public bool IsValidRepositoryRelativePath(string srcAssetPath, string[] assetTypesFilter)
      {
         if (!AssetSystem.Manager.IsRepositoryRelativePath(srcAssetPath))
         {
            // Not repository relative path
            return false;
         }
         return IsValidPath(srcAssetPath, assetTypesFilter);
      }

      /// <summary>
      /// Performs a robust check on a system or asset path to confirm that it is valid.
      /// Doesn't check for the file existing.
      /// </summary>
      public bool IsValidPath(string systemOrAssetPath)
      {
         string[] assetTypesFilter = null;
         return IsValidPath(systemOrAssetPath, assetTypesFilter);
      }

      /// <summary>
      /// Performs a robust check on a system or asset path to confirm that it is valid for asset types filter specified.
      /// Doesn't check for the file existing.
      /// </summary>
      /// <param name="propertyAssetTypesFilter">';' delimited list of asset types, normally specified by attribute Helpers.kProperty_AssetTypes.</param>
      /// <returns>'true' if valid.</returns>
      public bool IsValidPath(string systemOrAssetPath, string propertyAssetTypesFilter)
      {
         string[] assetTypesFilter = propertyAssetTypesFilter.Split(new char[] {';'}, StringSplitOptions.RemoveEmptyEntries);
         return IsValidPath(systemOrAssetPath, assetTypesFilter);
      }

      /// <summary>
      /// Performs a robust check on a system or asset path to confirm that it is valid for asset types filter specified.
      /// Doesn't check for the file existing.
      /// </summary>
      /// <param name="assetTypesFilter">List of four CC asset types.</param>
      /// <returns>'true' if valid.</returns>
      public bool IsValidPath(string systemOrAssetPath, string[] assetTypesFilter)
      {
         // We'll convert it to a system path as it could either be a repository path or relative path
         string systemPath = String.Empty;
         try 
         {
            systemPath = GetSystemPath(systemOrAssetPath);
         }
         catch (Exception)
         {
            // Not an asset path
            return false;
         }

         // This will deal with any '$/../foo.bar' cases
         if (!IsSystemPathInsideRepository(systemPath))
         {
            // Invalid path to asset
            return false;
         }

         // Asset type for extension
         AssetSystem.AssetType assetType = AssetSystem.AssetTypes.GetAssetTypeForSourceAsset(systemPath);
         if (assetType == null)
         {
            // Unknown asset type
            return false;
         }

         // Let's see if it matches asset types specified by filters 
         // if we got asset types, validate the asset path against them
         if( (assetTypesFilter != null) && (assetTypesFilter.Length > 0) )
         {
            foreach( string typeFourCC in assetTypesFilter )
            {
               if( assetType.TypeId == Tools.Common.Misc.GetFourCCFromString(typeFourCC) )
               {
                  return true;
               }
            }

            // Wrong asset type
            return false;
         }
         // if there is no asset types specified, any asset type is valid
         else
         {
            return true;
         }
      }

      /// <summary>
      /// Converts a cooked asset/system path to a source repository relative path.
      /// This may return the wrong filename if multiple source matches exist.
      /// </summary>
      public string GetCookedFilenameToSourceRepositoryRelativePath(string cookedFilename)
      {
         string systemCookedFilename = GetSystemPath(cookedFilename);
         string cookedExtension = String.Empty;
         string srcFilename = String.Empty;
         if (System.IO.Path.HasExtension(systemCookedFilename))
         {
            // Extension, remove '.', lowercase.
            cookedExtension = System.IO.Path.GetExtension(systemCookedFilename).Substring(1).ToLower();
         }

         foreach (AssetType type in AssetTypes.mAssetTypes)
         {
            if (type.CookedExtension == cookedExtension)
            {
               string tempCookedFilename = systemCookedFilename;
               if (type.CooksToPlatformSpecificDirectory)
               {
                  // Strip out platform specific directory
                  int lastSeparator = tempCookedFilename.LastIndexOfAny("\\/".ToCharArray());
                  // Remove platform specific directory (5 chars including separator)
                  tempCookedFilename = tempCookedFilename.Remove(lastSeparator - 5, 5);
               }

               foreach (string srcExtension in type.SrcExtensions)
               {
                  // Add extension to processed filename
                  string tempSrcFilename = System.IO.Path.ChangeExtension(tempCookedFilename, srcExtension);
                  if (System.IO.File.Exists(tempSrcFilename))
                  {
                     // OK, we'll go with the first match
                     srcFilename = tempSrcFilename;
                     break;
                  }
               }
            }
         }
         // Convert to asset path
         srcFilename = GetRepositoryRelativePath(srcFilename);

         return srcFilename;
      }
   }
}
