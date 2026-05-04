using System;
using System.Collections.Generic;
using System.Text;
using Tools.AssetSystem;
using System.IO;

namespace AssetTool
{
   class PackageGather
   {
      public static List<String> GatherCookedAssets( IEnumerable<String> sourceAssets, Manager assetManager, CookingOptions options )
      {
         Dictionary<String,bool> outCookedAssets = new Dictionary<String,bool>();
         StringBuilder errorOutput = new StringBuilder();

         foreach ( PlatformType.EPlatform platform in options.mPlatforms )
         {
            Dictionary<String, bool> processedAssets = new Dictionary<String, bool>();
            Queue<String> pendingAssets = new Queue<string>( sourceAssets );

            while ( pendingAssets.Count > 0 )
            {
               String processing = pendingAssets.Dequeue();
               String cookedPath = Manager.GetCookedAssetPath( processing, platform );

               if ( cookedPath == String.Empty )
               {
                  continue;
               }

               outCookedAssets[cookedPath] = true;
               processedAssets[processing] = true;

               foreach ( PackageAsset asset in PackageAssets.GetAssets( processing, assetManager, platform ) )
               {
                  if ( !processedAssets.ContainsKey( asset.mAssetPath ) )
                  {
                     pendingAssets.Enqueue( asset.mAssetPath );
                  }
               }
            }
         }

         return new List<String>( outCookedAssets.Keys );
      }

      public static List<String> NewListIncludingAssetsFiles( IEnumerable<String> cookedAssets, Manager assetManager )
      {
         List<String> ret = new List<string>();

         foreach ( String asset in cookedAssets )
         {
            ret.Add( asset );

            String assetsFile = asset + ".assets";
            if ( File.Exists( assetManager.GetSystemPathWithBackslashes( assetsFile ) ) )
            {
               ret.Add( assetsFile );
            }
         }

         return ret;
      }

      public static bool CreateCookedHardLinks( IEnumerable<String> sourceAssets, Manager assetManager, CookingOptions options, String outputFolder, StringBuilder passedErrorOutput )
      {
         List<String> cookedAssets = NewListIncludingAssetsFiles( GatherCookedAssets( sourceAssets, assetManager, options ), assetManager );
         bool anyFailures = false;

         StringBuilder errorOutput = passedErrorOutput ?? new StringBuilder();

         foreach ( String cookedAsset in cookedAssets )
         {
            String systemSourcePath = assetManager.GetSystemPathWithBackslashes( cookedAsset );
            String systemDestPath = Path.Combine( outputFolder, cookedAsset.Substring( 2 ) );
            bool isNew = true;
            
            if ( File.Exists( systemDestPath ) )
            {
               try
               {
                  File.Delete( systemDestPath );
                  isNew = false;
               }
               catch (System.Exception)
               {
                  anyFailures = true;
                  errorOutput.AppendFormat( "ERROR: Could not delete {0}", systemDestPath );
                  errorOutput.AppendLine();
               }
            }

            if ( !File.Exists( systemDestPath ) )
            {
               System.IO.Directory.CreateDirectory( System.IO.Path.GetDirectoryName( systemDestPath ) );
               bool worked = Tools.Common.WindowsNative.CreateHardLink( systemDestPath, systemSourcePath, IntPtr.Zero );

               if ( worked )
               {
                  if ( isNew )
                  {
                     Console.WriteLine( "{0} - Added", systemDestPath );
                  }
               }
               else
               {
                  errorOutput.AppendFormat( "ERROR: {0} -> {1}", cookedAsset, systemDestPath );
                  errorOutput.AppendLine();
                  anyFailures = true;
               }
            }
         }

         if ( anyFailures )
         {
            errorOutput.AppendLine();
            errorOutput.AppendLine( "Errors are listed above." );

            // If we didn't pass in an error output, output errors to the console
            if ( passedErrorOutput == null )
            {
               Console.Write( errorOutput.ToString() );
            }

            return false;
         }
         else
         {
            return true;
         }
      }
   }
}
