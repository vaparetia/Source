using System;
using System.Collections.Generic;
using System.Text;

namespace TextureViewer
{
   /// <summary>
   /// Basically a copy of BP tools Assets.Manager, deals with asset paths.
   /// </summary>
   public class PathHelper
   {
      string mRepositoryRoot;

      public PathHelper()
      {
         mRepositoryRoot = GetNormalizedPath( System.Environment.GetEnvironmentVariable("BPE_REPOSITORY") );
      }

      public PathHelper(string repRoot)
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
               throw new Exception(string.Format("Path '{0}' is not inside the repository {1}.", absolutePath, mRepositoryRoot));
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
            throw new Exception("Invalid path, it doesn't reside inside the repository.");
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
   }
}
