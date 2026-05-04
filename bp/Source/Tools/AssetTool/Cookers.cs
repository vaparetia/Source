using System;
using System.Collections.Generic;
using System.Text;
using Tools.AssetSystem;
using System.Xml;
using Tools.Property.Cook;
using Tools.Property.Source;
using System.Xml.XPath;
using Tools.Common;
using System.IO;
using Tools.Property.Misc;
using Tools.GameEngine;

namespace AssetTool
{
   partial class Cookers
   {
      public static bool CookRMDL( string inputFilename )
      {
         System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo( "mdx2obj.exe", inputFilename );
         startInfo.UseShellExecute = false;
         using ( System.Diagnostics.Process proc = System.Diagnostics.Process.Start( startInfo ) )
         {
            proc.WaitForExit();

            return (proc.ExitCode == 0);
         }
      }

      public static bool CookFMDP(string inputFilename, string outputFilename, Manager assetManager, PlatformType.EPlatform platform)
      {
         // Create platform specific cooked directory
         string directory = Path.GetDirectoryName(outputFilename);
         if (!System.IO.Directory.Exists(directory))
         {
            // Create directory
            System.IO.Directory.CreateDirectory(directory);
         }

         string commandLineOptions = String.Empty;

         switch ( platform )
         {
         case PlatformType.EPlatform.kWin32:
            commandLineOptions += "-pc -b " + directory;
            break;
         case PlatformType.EPlatform.kPS3:
            commandLineOptions += "-ps3 -b " + directory;
            break;
         default:
            Console.WriteLine( "ERROR: CookFMDP doesn't know how to cook {0}", platform );
            return false;
         }

         commandLineOptions += " " + inputFilename;

         System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo("fmod_designercl.exe", commandLineOptions);
         startInfo.UseShellExecute = false;
         System.Diagnostics.Process proc = System.Diagnostics.Process.Start(startInfo);
         proc.WaitForExit();
         bool bResult = (proc.ExitCode == 0);

         // Add .fsb package dependency
         List<PackageAsset> outPackageAssets = new List<PackageAsset>();
         // We assume that the .FSB is the same path/name as the .FEV for now.
         // We can revisit this later to support multiple FSBs.
         string fsbFilename = Path.ChangeExtension(outputFilename, ".fsb");
         if (!System.IO.File.Exists(fsbFilename))
         {
            // Couldn't find FSB
            Console.WriteLine("Missing {0}!", fsbFilename);
         }
         // FMOD take care of loading these ingame, so we use package only.
         Tools.AssetSystem.Helper.MarkSourceAssetAsGenerated(fsbFilename, assetManager);
         outPackageAssets.Add(new PackageAsset(assetManager.GetRepositoryRelativePath(fsbFilename), PackageAsset.EPackageType.kPackageOnly, PackageAsset.EAssetSource.kGenerated));
         PackageAssets.WriteAssets(inputFilename, outPackageAssets, assetManager, platform);

         return bResult;
      }

      public static void CookGPRPErrorDelegate(string errorMessage)
      {
         System.Console.WriteLine("");
         System.Console.WriteLine(errorMessage);
      }

      public static bool CookGPRP(string inputFilename, string[] outputFilenames, PlatformType.EPlatform[] platforms, Manager assetManager)
      {
         FileBasedPropertyObjectEnumerator propertySource = new FileBasedPropertyObjectEnumerator(new FileBasedPropertyObjectEnumerator.ReportErrorDelegate(CookGPRPErrorDelegate));
         propertySource.AddFiles(FileBasedPropertyObjectEnumerator.FileSearchParams.DefaultFileSearchParams(assetManager));

         CoreTools.Prefab.PrefabManager prefabManager = new CoreTools.Prefab.PrefabManager(assetManager, propertySource);
         prefabManager.AddPrefabRepositoryForScriptFile(inputFilename);

         XmlDocument sceneDoc = new XmlDocument();
         sceneDoc.Load(inputFilename);

         CookProperties.CookInfo cookInfo = new CookProperties.CookInfo();
         CookProperties.CookOutput initialCookOutput = new CookProperties.CookOutput();

         using (new Tools.Common.Timer(string.Format("Merge Properties: {0}", inputFilename)))
         {
            foreach (XmlElement scriptObject in sceneDoc.SelectNodes(".//Node/Properties"))
            {
               XmlElement propertyContainer = (XmlElement) scriptObject["UsingPropertyContainer"];
               if (propertyContainer != null)
               {
                  Tools.Property.Build.MergePropertiesV2.Merge_Inplace(ref propertyContainer, propertySource);
               }
            }

            foreach (XmlElement prefabElement in sceneDoc.SelectNodes(".//Node[PrefabData]"))
            {
               // Add dependencies for this prefab
               {
                  Guid prefabId = new Guid(prefabElement.SelectSingleNode("./PrefabData/AddPrefab/@id").Value);
                  CoreTools.Prefab.Prefab prefab = prefabManager.GetPrefabById(prefabId);

                  if (prefab != null)
                  {
                     foreach (string prefabDependency in prefab.ScriptDependencies)
                     {
                        Tools.AssetSystem.BuildAsset buildDependency = Tools.AssetSystem.BuildAsset.StandardCookingAsset("Prefab", assetManager.GetRepositoryRelativePath(prefabDependency));
                        initialCookOutput.AddBuildAsset( buildDependency );
                     }
                  }
                  else
                  {
                      Console.WriteLine("WARNING: Prefab '{0}' not found in prefab repository for script file '{1}'. It will not be cooked.", prefabId, inputFilename);
                  }
               }

               // Merge and inject prefab into the Open3d scene.
               if( prefabManager.MergePrefab_Inplace(prefabElement) )
               {
                   prefabManager.InjectPrefab(prefabElement);
               }
            }
         }

         using (new Tools.Common.Timer(string.Format("Cook Properties: {0}", inputFilename)))
         {
            // Add custom cooker for splines
            cookInfo.mCustomPropertyCooker += new CookProperties.CookInfo.CustomPropertyCooker(Helper.Spline.ProcessCustomProperty);
            // Custom properties modifier for updating to lit models if available
            CUpdateModelProperties updateModelProperties = new CUpdateModelProperties(inputFilename);
            cookInfo.mScriptObjectPropertiesModifier += new CookProperties.CookInfo.ScriptObjectPropertiesModifier(updateModelProperties.ReplaceStaticModelsWithLitVersions);
            cookInfo.mScriptObjectPropertiesModifier += new CookProperties.CookInfo.ScriptObjectPropertiesModifier(
                delegate(XmlElement scriptObject, Tools.AssetSystem.Manager _manager, CookProperties.CookInfo _cookinfo, CookProperties.CookOutput cookOutput )
                {
                   SceneUtils.CopyOpen3dTransformToObject(scriptObject);
                }
                );

            for ( int outputTargetIndex = 0; outputTargetIndex < platforms.Length; ++outputTargetIndex )
            {
               CookProperties.CookOutput cookOutput = initialCookOutput.Clone();

               String outputFilename = outputFilenames[outputTargetIndex];
               PlatformType.EPlatform platform = platforms[outputTargetIndex];

               cookInfo.mPlatform = platform;

               System.IO.FileStream output = new System.IO.FileStream( outputFilename, System.IO.FileMode.Create );
               CookProperties.WriteCookedPropertyContainerPropertiesForScene( 
                  assetManager, 
                  sceneDoc, 
                  propertySource, 
                  cookInfo, 
                  cookOutput, 
                  output );
               output.Close();

               // write out assets file
               PackageAssets.WriteAssets( inputFilename, cookOutput.PackageAssets, assetManager, platform );
               // write out build file
               BuildAssets.WriteAssets( inputFilename, cookOutput.BuildAssets, assetManager, platform );
            }

         }
         return true;
      }

      /// <summary>
      /// Converts xml doc into a binary string.
      /// Any attributes with 'pxAsset' are assumed to be src assets, and will be converted to cooked assets in output and added to .assets.
      /// </summary>
      public static bool CookXML_(string inputFilename, string outputFilename, Manager assetManager)
      {
         XmlDocument doc = new XmlDocument();
         doc.Load(inputFilename);

         List<PackageAsset> outPackageAssets = new List<PackageAsset>();

         // Find all src assets
         XPathNodeIterator iter = doc.CreateNavigator().Select("//*[@pxAsset]");
         while (iter.MoveNext())
         {
            XPathNavigator assetNav = iter.Current;
            assetNav.MoveToAttribute("pxAsset", String.Empty);
            string srcAsset = assetNav.Value;
            AssetType type = AssetTypes.GetAssetTypeForSourceAsset(srcAsset);
            if (type == null)
            {
               Console.WriteLine(String.Format("Warning: {0} isn't a valid asset type in {1}.\n", srcAsset, inputFilename));
               continue;
            }

            string destAsset = Path.ChangeExtension(srcAsset, type.CookedExtension);

            // Replace srcAsset with dest type.
            assetNav.SetValue(destAsset);

            // Check to see if it's OK.
            bool bValid = assetManager.IsValidRepositoryRelativePath(srcAsset, type.TypeString);
            if (bValid)
            {
               bValid = File.Exists(assetManager.GetSystemPath(srcAsset));
            }
            if (!bValid)
            {
               Console.WriteLine(String.Format("Warning: {0} isn't a valid asset in {1}.\n", srcAsset, inputFilename));
               continue;
            }
            outPackageAssets.Add(PackageAsset.StandardRepositoryAsset(srcAsset));
         }

         // Write out a binary string that represents the xml.
         string xmlString = doc.OuterXml;

         System.IO.FileStream output = new System.IO.FileStream(outputFilename, System.IO.FileMode.Create);
         using (BinaryWriterBigEndian writer = new BinaryWriterBigEndian(output))
         {
            writer.Write(xmlString);
         }
         output.Close();

         // write out assets file
         PackageAssets.WriteAssets(inputFilename, outPackageAssets, assetManager);

         return true;
      }

      public class SceneUtils
      {

         /// <summary>
         /// Copies the Open3d transform from the node to the properties of the script object.
         /// </summary>
         /// <param name="scriptObject">A navigator pointing at the merged data for the script object.</param>
         public static void CopyOpen3dTransformToObject(XmlElement scriptObject)
         {
            // Now walk up to the node for this property container
            XmlElement sceneNodeNav = (XmlElement) scriptObject;
            for ( sceneNodeNav = sceneNodeNav.ParentNode as XmlElement; sceneNodeNav != null; sceneNodeNav = sceneNodeNav.ParentNode as XmlElement )
            {
               if ( sceneNodeNav.LocalName == "Node" )
               {
                  break;
               }
            }

            // Couldn't find a base node, punt
            if (sceneNodeNav == null )
            {
               return;
            }

            // Grab the space-delimited translation and orientation from the Open3d scene node
            String translation = sceneNodeNav.SelectSingleNode("Transform/Translation").InnerText;
            String orientation = sceneNodeNav.SelectSingleNode( "Transform/Orientation" ).InnerText;
            String scale = sceneNodeNav.SelectSingleNode( "Transform/Scale" ).InnerText;

            XmlElement rgc = RGCHelpers.GetRGCForScriptObject(scriptObject);
            XmlElement transformNode = RGCHelpers.GetTransformForRGC( rgc );
            XmlElement scaleNode = RGCHelpers.GetScaleForRGC( rgc );

            // .. and split them up into separate entries
            string[] translations = translation.Trim().Split(' ');
            string[] orientations = orientation.Trim().Split(' ');
            string[] scales = scale.Trim().Split(' ');

            // Make sure we have a 3x3 matrix and 3 component translation
            if (translations.Length == 3 && orientations.Length == 9 && scales.Length == 3)
            {
               // Jam the translations and orientations into the node
               for (int row = 0; row < 3; ++row)
               {
                  for (int column = 0; column < 3; ++column)
                  {
                     string src = orientations[row * 3 + column];
                     XmlElement dstNav = Helpers.FindDirectChildProperty( transformNode, String.Format( "m{0}{1}", row, column ) );
                     Helpers.SetPropertyValueElement(dstNav, src);
                  }
               }

               for (int column = 0; column < 3; ++column)
               {
                  string src = translations[column];
                  XmlElement dstNav = Helpers.FindDirectChildProperty( transformNode, String.Format( "m3{0}", column ) );
                  Helpers.SetPropertyValueElement(dstNav, src);
               }

               Helpers.SetPropertyValueElement(Helpers.FindDirectChildProperty(scaleNode, "X"), scales[0]);
               Helpers.SetPropertyValueElement(Helpers.FindDirectChildProperty(scaleNode, "Y"), scales[1]);
               Helpers.SetPropertyValueElement(Helpers.FindDirectChildProperty(scaleNode, "Z"), scales[2]);
            }
            else
            {
               throw new System.InvalidOperationException("Got scene node with bad translations or orientations");
            }
         }

      }

      public class CUpdateModelProperties
      {
         string   mScriptingFilename;

         public CUpdateModelProperties(string scriptingFilename)
         {
            mScriptingFilename = scriptingFilename;
         }

         /// <summary>
         /// Finds static lit models and replace with lit versions if present.
         /// </summary>
         /// <param name="sceneDoc">Area scripting.</param>
         public void ReplaceStaticModelsWithLitVersions(XmlElement scriptObjectElement, Manager assetManager, Tools.Property.Cook.CookProperties.CookInfo cookInfo, CookProperties.CookOutput cookOutput )
         {
            // Build list of static model components using static lighting.
            // This handles dynamic components correctly.
            XmlNodeList modelComponentGroups = scriptObjectElement.SelectNodes( ".//ComponentGroup[@id='Model' and Component[@group='Model' and child::Property[@label='model_lighting_type']/Value='kLT_PrecalculatedLightmaps']]" );

            // Find all static models
            foreach(XmlElement modelComponentGroup in modelComponentGroups)
            {
               XmlElement modelComponent = modelComponentGroup["Component"];
               XmlElement modelAssetPropertyNode = (XmlElement) modelComponent.SelectSingleNode( "./Property[@label='model_static_resource']" );
               XmlElement modelAssetValueNode = (XmlElement) modelComponent.SelectSingleNode( "./Property[@label='model_static_resource']/Value" );
               string unlitModelAsset = modelAssetValueNode.InnerText;

               if ( !assetManager.IsValidRepositoryRelativePath( unlitModelAsset ) )
               {
                  continue;
               }

               string componentId = modelComponentGroup.GetAttribute("editorId");

               string unlitModelAssetSystemPath = assetManager.GetSystemPath(unlitModelAsset);

               string baseOutputPath = Helper.Lighting.GetBaseOutputPath(mScriptingFilename);
               string modelOutputPath = Helper.Lighting.GetLitModelOutputPath(baseOutputPath, unlitModelAssetSystemPath, new Guid(componentId));
               string litModelAssetSystemPath = Helper.Lighting.GetLitModelPath(modelOutputPath, unlitModelAssetSystemPath);

               // Recook scripting options based on lit model.
               // If we're not using the lit model then scripting will only get recooked if a lit model is created.
               Tools.AssetSystem.BuildAsset.ERecookOptions recookScripting_LitModelOptions = Tools.AssetSystem.BuildAsset.ERecookOptions.kRO_Newer;

               // See if the lit asset exists
               if (File.Exists(litModelAssetSystemPath))
               {
                  // See if it is newer than the unlit model
                  if (File.GetLastWriteTime(litModelAssetSystemPath) > File.GetLastWriteTime(unlitModelAssetSystemPath))
                  {
                     // Change unlit value to lit model asset
                     string litModelAsset = assetManager.GetRepositoryRelativePath(litModelAssetSystemPath);
                     modelAssetPropertyNode.SetAttribute(Helpers.kProperty_IsGeneratedAsset, true.ToString());

                     modelAssetValueNode.InnerText = litModelAsset;
                     // Force scripting to recook if lit model is newer or if it doesn't exist (i.e. use unlit model instead)
                     // This will cause the scripting to use the unlit model if the lit model is deleted.
                     recookScripting_LitModelOptions = Tools.AssetSystem.BuildAsset.ERecookOptions.kRO_NewerOrDoesntExist;
                  }
               }

               // Add build dependencies on lit and unlit models to force scripting to recook.
               cookOutput.AddBuildAsset( Tools.AssetSystem.BuildAsset.StandardCookingAsset( "UnlitModel", unlitModelAsset ) );
               cookOutput.AddBuildAsset( new Tools.AssetSystem.BuildAsset( "LitModel", assetManager.GetRepositoryRelativePath( litModelAssetSystemPath ),
                  recookScripting_LitModelOptions, Tools.AssetSystem.BuildAsset.EAssetSource.kGenerated));
            }
         }
      }
   }
}
