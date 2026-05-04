using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Tools;

namespace Tools.AssetSystem
{
   /// <summary>
   /// Contains all information for a single asset type
   /// It stores a description which should be something simple like "Model", "Cooked Model", ...
   /// Also a fourCC identifier which mirrors what you see in the game and the class name for code gen resource locking
   /// </summary>
   public class AssetType
   {
      /// <summary>
      /// When cooked, does this asset type have other cooked assets it could require to be loaded for use.
      /// Used to determine if it needs a CResource or a CResourceWithDependencies for property code gen.
      /// </summary>
      public enum ELoadNeedsDependencies
      {
         kNoDependencies,
         kComplexDependencies,
      }

      /// <summary>
      /// When platform specific, assets are cooked into _win32/_ps3 folders and a dummy asset
      /// is used for dependency checking.      
      /// </summary>
      public enum ECookedAssetDirectory
      {
         kNotPlatformSpecific,     // One cooker, one file for all platforms
         kPlatformSpecific,        // Platform-specific and seperate cooker instances
         kPlatformSpecificGrouped, // Platform-specific, but cooked by one cooker
      }

      private string    mDescription;
      private uint      mTypeId;
      private string[]  mSrcExtensions;
      private string    mCookedExtension;
      private string    mClassName;
      private string    mClassInclude;
      private ELoadNeedsDependencies mCookedDependencies;
      private ECookedAssetDirectory  mCookedAssetDirectory;
      /// <summary>
      /// Assets get cooked in decreasing priority order to prevent build dependency issues.
      /// </summary>
      private int       mCookPriority;
      private int       mCookBatchSize;

      public string                 Description                      { get { return mDescription; } }
      public uint                   TypeId                           { get { return mTypeId; } }
      public string[]               SrcExtensions                    { get { return mSrcExtensions; } }
      public string                 CookedExtension                  { get { return mCookedExtension; } }
      public bool                   IsCooked                         { get { return mCookedExtension == string.Empty; } }
      public string                 ClassName                        { get { return mClassName; } }
      public string                 ClassInclude                     { get { return mClassInclude; } }
      public string                 TypeString                       { get { return Tools.Common.Misc.GetStringFromFourCC(mTypeId); } }
      public ELoadNeedsDependencies CookedDependencies               { get { return mCookedDependencies; } }
      public ECookedAssetDirectory  CookedAssetDirectory             { get { return mCookedAssetDirectory; } }
      public bool                   CooksToPlatformSpecificDirectory { get { return mCookedAssetDirectory != ECookedAssetDirectory.kNotPlatformSpecific; } }
      public bool                   ShouldDistributePlatformCooks    { get { return mCookedAssetDirectory == ECookedAssetDirectory.kPlatformSpecific; } }
      public int                    CookPriority                     { get { return mCookPriority; } }
      public int                    CookBatchSize                    { get { return mCookBatchSize;  } }
      public readonly int           Version;

      /// <summary>
      ///  Constructs an asset type that has multiple extensions
      /// </summary>
      public AssetType( string description, 
                        string typeFourCC, 
                        string[] srcExtensions,
                        int version,
                        string cookedExtension, 
                        string className,
                        string classInclude,
                        ELoadNeedsDependencies cookedDependencies,
                        ECookedAssetDirectory cookedAssetDirectory,
                        int cookPriority,
                        int cookBatchSize)
      {
         mDescription = description;
         mTypeId = Tools.Common.Misc.GetFourCCFromString(typeFourCC);
         mSrcExtensions = srcExtensions;
         mCookedExtension = cookedExtension;
         mClassName = className;
         mClassInclude = classInclude;
         mCookedDependencies = cookedDependencies;
         mCookedAssetDirectory = cookedAssetDirectory;
         mCookPriority = cookPriority;
         mCookBatchSize = cookBatchSize;
         Version = version;
      }

      /// <summary>
      /// Constructs an asset type that has only one extension
      /// </summary>
      public AssetType( string description, 
                        string typeFourCC, 
                        string srcExtension,
                        int version,
                        string cookedExtension,
                        string className,
                        string classInclude, 
                        ELoadNeedsDependencies cookedDependencies,
                        ECookedAssetDirectory cookedAssetDirectory,
                        int cookPriority,
                        int cookBatchSize)
         : this(description, typeFourCC, new string[] { srcExtension }, version, cookedExtension, className, classInclude, cookedDependencies, cookedAssetDirectory, cookPriority, cookBatchSize)
      {
      }
   }

   /// <summary>
   /// This is where all valid asset types are specified
   /// </summary>
   public class AssetTypes
   {
      enum ECookPriorties
      {
         // Lower = cooks later

         Default,
         RawGameModels, // Top-level game models
         ThingsThatUseTextures, // Models, Fonts, Skinned Models
         StateMachines,         // Because they contain things like models and animations
         Scripting,
         XML
      }

      static public AssetType[] mAssetTypes = new AssetType[]
      {
         new AssetType( "Text",                 "TEXT", "txt", 0, 
            "", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 1 ),
         
         new AssetType( "Texture",              "TXTR", new string[] { "tga", "tif", "jpg", "bmp", "hdr", "vtex", "png" }, 9, 
            "ctxr", "CBaseTexture", "Renderer/Base/Backend/CTexture.h", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecific, 0, 5 ),
         
         new AssetType( "GroupedModel",         "GMDL", "gmd", 0, 
            "cgmd", "CMesh", "Renderer/Base/Primitive/CMesh.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecific, (int) ECookPriorties.ThingsThatUseTextures, 4 ),

         new AssetType( "RawModel",             "RMDL", "mdl", 0,
            "bmdl", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, (int) ECookPriorties.RawGameModels, 1 ),
            
         new AssetType( "Model",                "MODL", "bmdl", 2, 
            "cmdl", "CMesh", "Renderer/Base/Primitive/CMesh.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecificGrouped, (int) ECookPriorties.ThingsThatUseTextures, 4 ),
         
         new AssetType( "Skin",                 "SKIN", "skn", 2, 
            "cskn", "CSkinnedMesh", "Renderer/Base/Primitive/CSkinnedMesh.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecific, (int) ECookPriorties.ThingsThatUseTextures, 4 ),

         new AssetType( "Collision",            "COLL", "col", 0, 
            "ccol", "CCollisionMesh", "Engine/Collision/CCollisionMesh.h", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 4 ),

         new AssetType( "Editor Model",         "EMDL", "editorModel", 0, 
            "cemd", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 1 ),
         
         new AssetType( "Animation",            "ANIM", "anim", 6, 
            "cani", "CAnimation", "Engine/Animation/CAnimation.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecific, 0, 4 ),
         
         new AssetType( "Additive Animation",   "ADNI", "adni", 6, 
            "cadi", "CAnimation", "Engine/Animation/CAnimation.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecific, 0, 4 ),
         
         new AssetType( "GameProperties",       "GPRP", "sgpr", 1, 
            "cgpr", "CGameObjectPropertiesPackage", "Engine/GameObjectSystem/CGameObjectPropertiesPackage.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecificGrouped, (int)ECookPriorties.Scripting, 3 ),
         
         new AssetType( "Sample",               "SAMP", new string[] { "wav", "ogg", "mp3", "cmp3" }, 0, 
            "", "CSoundSystemSample", "Engine/Sound/CSoundSystemSample.h", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 4 ),
         
         new AssetType( "FMODProject",          "FMDP", "fdp", 0, 
            "fev", "CSoundSystemFEV", "Engine/Sound/CSoundSystemFEV.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecific, 0, 1 ),
         
         new AssetType( "FMODSoundBank",        "FMSB", "fsb", 0, 
            "", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 1 ),

         new AssetType( "XmlMeta",              "XMLM", new string[] { "xmlmeta" }, 0, 
            "", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 1 ),

         new AssetType( "Xml",                  "XML_", new string[] { "xml"}, 0, 
            "cxml", "TiXmlDocument", "Engine/Mechanics/TinyXml/TinyXml.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, (int) ECookPriorties.XML, 4 ),
         
         new AssetType( "Prefab",               "PFAB", "prefab", 0, 
            "", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 1),
         
         new AssetType( "Font",                 "FONT", new string[] { "font" }, 0, 
            "cfon", "CMTXFont", "Renderer/Base/CMTXFont.h", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, (int) ECookPriorties.ThingsThatUseTextures, 4 ),
         
         new AssetType( "FreeType",             "FTYP", new string[] { "ttf", "ttc", "pfb", "cff", "otf", "pcf", "fon", "fnt", "bdf", "pfr" }, 0, 
            "", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, (int) ECookPriorties.Default, 1 ),

         new AssetType( "LUA Source File",      "LUAS", "lua", 0, 
            "", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 1 ),
         
         // DON'T FORGET TO UPDATE ShaderCooker[PLATFORM].cpp's version!
         new AssetType( "Effect",               "EFCT", "fx", 5, 
            "cfx", "CCGEffect", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecific, 0, 1 ),

         new AssetType( "Effect Sub-Assets",      "EFCS", new string[] { "fx-settings", "fxh" }, 0, 
            "", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 1 ),

         new AssetType( "StateMachine",         "EBSM", "sesm", 5, 
            "cesm", "CStateMachine", "", AssetType.ELoadNeedsDependencies.kComplexDependencies, AssetType.ECookedAssetDirectory.kPlatformSpecific, (int) ECookPriorties.StateMachines, 4 ),

         new AssetType( "Animation Events",     "AEVT", "events_anim", 0, 
            "", "", "", AssetType.ELoadNeedsDependencies.kNoDependencies, AssetType.ECookedAssetDirectory.kNotPlatformSpecific, 0, 1 )
         
      };

      static public string GetFileFilters(string[] assetTypes)
      {
         string filter = "";

         // If we have multiple asset types add a "All Support Files" filter first.
         if (assetTypes.Length > 1)
         {
            string allExtensionsString = "";

            foreach (string assetType in assetTypes)
            {
               foreach (AssetType type in mAssetTypes)
               {
                  if (string.Compare(type.TypeString, assetType) == 0)
                  {
                     foreach (string extension in type.SrcExtensions)
                     {
                        if (allExtensionsString.Length > 0)
                           allExtensionsString += "; ";

                        allExtensionsString += string.Format("*.{0}", extension);
                     }
                  }
               }
            }

            if (!string.IsNullOrEmpty(allExtensionsString))
            {
               filter = string.Format("All Supported Files ({0})|{0}", allExtensionsString);
            }
         }

         foreach( string assetType in assetTypes )
         {
            foreach( AssetType type in mAssetTypes )
            {
               if( string.Compare(type.TypeString, assetType) == 0 )
               {
                  if( filter.Length > 0 )
                     filter += "|";

                  string extensionsString = "";
                  foreach( string extension in type.SrcExtensions )
                  {
                     if( extensionsString.Length > 0 )
                        extensionsString += "; ";

                     extensionsString += string.Format("*.{0}", extension);
                  }
                  
                  filter += string.Format( "{0} Files ({1})|{1}", type.Description, extensionsString );
               }
            }
         }

         return filter;
      }

      static public AssetType GetAssetTypeForSourceAsset(string systemOrAssetPath)
      {
         string extensionWithDot = System.IO.Path.GetExtension(systemOrAssetPath);

         return GetAssetTypeForExtension(extensionWithDot);
      }

      static public AssetType GetAssetTypeForExtension(string extensionOfType)
      {
         string extensionNoDot = ( extensionOfType.Length > 0 && extensionOfType[0] == '.' ) ? extensionOfType.Substring(1) : extensionOfType;

         foreach( AssetType type in mAssetTypes )
         {
            foreach( string extension in type.SrcExtensions )
            {
               if( String.Compare(extensionNoDot, extension, true) == 0 )
                  return type;
            }
         }

         return null;
      }

      static public AssetType GetAssetTypeForTypeId(uint typeId)
      {
         foreach( AssetType type in mAssetTypes )
         {
            if( type.TypeId == typeId )
                  return type;
         }

         return null;
      }

      /// <summary>
      /// Returns merged ELoadNeedsDependencies for an array of assetTypes.
      /// </summary>
      public static AssetSystem.AssetType.ELoadNeedsDependencies GetMergedLoadDependenciesForAssetTypes(string[] assetTypes)
      {
         AssetSystem.AssetType.ELoadNeedsDependencies cookedDependencies = AssetSystem.AssetType.ELoadNeedsDependencies.kNoDependencies;
         // Need to write token declaration
         foreach(string assetTypeExtension in assetTypes)
         {
            AssetSystem.AssetType assetType = AssetSystem.AssetTypes.GetAssetTypeForTypeId(Tools.Common.Misc.GetFourCCFromString(assetTypes[0].ToUpper()));
            if (assetType.CookedDependencies == AssetSystem.AssetType.ELoadNeedsDependencies.kComplexDependencies)
            {
               // One of these assets has complex dependencies, need to use CResourceWithDependencies.
               cookedDependencies = AssetSystem.AssetType.ELoadNeedsDependencies.kComplexDependencies;
               break;
            }
         }

         return cookedDependencies;
      }
   }
}
