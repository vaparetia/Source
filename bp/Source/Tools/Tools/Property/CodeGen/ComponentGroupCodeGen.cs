using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using Tools.Property;
using Tools.Property.UI;
using Tools.Property.Build;
using Tools.Property.Misc;

namespace Tools.Property.CodeGen
{
   /// <summary>
   /// Generates C++ code for loading property objects
   /// </summary>
   public class ComponentGroupCodeGen
   {
      public enum ECodeGenerationOptions
      {
         kNormal = 0,
         kGeneratePropertyCodeOnly,
      }

      public StringBuilder    mDeclarationForwardsAndIncludesCode = new StringBuilder();
      public StringBuilder    mDeclarationCode = new StringBuilder();
      public StringBuilder    mBodyIncludesCode = new StringBuilder();
      public StringBuilder    mBodyCode = new StringBuilder();
      ECodeGenerationOptions  mOptions;
      XmlDocument             mDocument;
      XPathNavigator          mNav;
      int                     mTabCount;
      public string           mComponentGroupName;
      public string           mClassName;
      string                  mGameObjectType;
      string                  mClassExportDeclaration;
      bool                    mbAutoRegisterFactory = true;
      StringBuilder           mCB = null;       // Current builder

      public ComponentGroupCodeGen(XPathNavigator srcComponentGroup, int startingTabCount, ECodeGenerationOptions options)
      {
         mOptions = options;
         mDocument = Helpers.NavigatorToDocument(srcComponentGroup);
         mTabCount = startingTabCount;
         mNav = mDocument.CreateNavigator();
         XPathNavigator nav = mNav.SelectSingleNode(@"/ComponentGroup[@id]");
         if (nav == null)
         {
            // This isn't a valid property object because it doesn't have a top level name
            return;
         }
         // Find gameObjectType of component (stored with component)
         {
            XPathNavigator componentNav =  mNav.SelectSingleNode("/ComponentGroup/Component[@gameObjectType]");
            if (componentNav == null)
            {
               // No good, need a game object type attribute for propery derivation
               return;
            }
            mGameObjectType = componentNav.GetAttribute(Helpers.kComponentGroup_GameObjectType, "");
            if (mNav.GetAttribute(Helpers.kComponent_registerFactory, "").Length != 0)
            {
               mbAutoRegisterFactory = Tools.Common.Misc.ParseBool((mNav.GetAttribute(Helpers.kComponent_registerFactory, ""))); 
            }
         }

         // Find kComponent_classPropertiesExportDeclaration
         {
            XPathNavigator componentNav =  mNav.SelectSingleNode("/ComponentGroup/Component[@classPropertiesExportDeclaration]");
            if (componentNav != null)
            {
               mClassExportDeclaration = componentNav.GetAttribute(Helpers.kComponent_classPropertiesExportDeclaration, String.Empty);
               mClassExportDeclaration += " ";  // Pad for neatness!
            }
         }

         mComponentGroupName = nav.GetAttribute(Helpers.kComponent_Id, "");
         mClassName = CodeGenHelpers.GetPropertiesClassName(mComponentGroupName, MergeProperties.EBuildTreeSrcType.kComponentGroup);
         BuildDeclaration();
         BuildBody();
      }

      public void BuildDeclaration()
      {
         BuildDeclarationForwardsAndIncludesCode();

         // Set active builder to declaration code
         mCB = mDeclarationCode;

         // Class declaration
         AddTabs();
         mCB.AppendLine(String.Format("class {0}{1} : public CGameObjectComponentProperties{2}", mClassExportDeclaration, mClassName, mGameObjectType));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("public:");
         mTabCount++;
         // Leave constructor/desructor for future expansion
         AddTabs();
         mCB.AppendLine(String.Format("{0}();", mClassName));
         AddTabs();
         mCB.AppendLine(String.Format("virtual ~{0}();", mClassName));
         mCB.AppendLine();

         // Loader function declaration
         AddTabs();
         mCB.AppendLine(String.Format("{0, -45}   BuildComponentProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator * pAllocator = NULL);", "static CGameObjectComponentProperties *"));

         // Post loader function declaration
         AddTabs();
         mCB.AppendLine(String.Format("//virtual void\tPostLoadUpdate();"));

         if (mOptions == ECodeGenerationOptions.kNormal)
         {
            // Builder declaration
            AddTabs();
            mCB.AppendFormat( "{0, -45}   BuildComponent(boost::shared_ptr<CGameObjectComponentProperties const> const &pComponentProperties);\n", 
                              "static CGameObjectComponent * const");
         }

         // Compile time group name
         AddTabs();
         mCB.AppendFormat("{0, -45}   GetComponentGroupName(void) const;\n", "virtual char const *"); 

         // Compile time group name hash
         AddTabs();
         mCB.AppendFormat("{0, -45}   GetComponentGroupType(void) const;\n", "virtual uint32"); 
         mCB.AppendLine();

         // Compile time id
         AddTabs();
         mCB.AppendFormat("{0, -45}   kComponentGroupType = 0x{1:X};\n", "static const uint32", CodeGenHelpers.GetPropertyNameHash(mComponentGroupName) );
         mCB.AppendLine();

         mTabCount--;
         AddTabs();
         mCB.AppendLine("};");
         mCB.AppendLine();
      }

      public void BuildDeclarationForwardsAndIncludesCode()
      {
         mCB = mDeclarationForwardsAndIncludesCode;

         AddTabs();
         mCB.AppendLine("#pragma once");
         mCB.AppendLine();
         AddTabs();
         mCB.AppendFormat("#include \"../CGameObjectComponentProperties{0}.h\"", mGameObjectType);
         AddTabs();
         mCB.AppendLine();
         AddTabs();
         mCB.AppendLine("class CGameObjectComponent;");
         mCB.AppendLine();
      }

      public void BuildBody()
      {
         // Add includes
         mCB = mBodyIncludesCode;
         AddTabs();
         mCB.AppendLine("#include \"StdAfx.h\"");
         mCB.AppendLine();
         AddTabs();
         mCB.AppendFormat("#include \"{0}.h\"\n", mClassName);
         mCB.AppendLine("#include \"../ComponentGroupCommonCppIncludes.h\"");

         // Set active builder to body code
         mCB = mBodyCode;
         mCB.AppendLine();
         if (mbAutoRegisterFactory)
         {
            BuildAutoRegisterFactoryCode();
         }
         BuildClassBody();
         BuildMiscFunctionBody();
         BuildGroupNameBody();
         BuildPropertiesLoaderBody();
         if (mOptions == ECodeGenerationOptions.kNormal)
         {
            BuildComponentBuilderBody();
         }
      }

      public void BuildClassBody()
      {
         // Class declaration, just empty constructor, destructor
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("{0}::{0}()", mClassName));

         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("{0}::~{0}()", mClassName));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildMiscFunctionBody()
      {
         // Other required functions
         AddTabs();
         mCB.AppendFormat("//{0}::PostLoadUpdate()\n", mClassName);
         AddTabs();
         mCB.AppendLine("//{");
         AddTabs();
         mCB.AppendLine("//}");
         mCB.AppendLine();
      }

      public void BuildGroupNameBody()
      {
         // Compile time group name
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("char const * {0}::GetComponentGroupName(void) const\n", mClassName); 

         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         AddTabs();
         mCB.AppendFormat("return \"{0}\";\n", mComponentGroupName);

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         // Compile time group name hash
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("uint32 {0}::GetComponentGroupType(void) const\n", mClassName); 

         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         AddTabs();
         mCB.AppendLine("return kComponentGroupType;");

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      private void AppendPlatformIfArgs( StringBuilder b, string[] platforms )
      {
         for ( int i = 0; i < platforms.Length; ++i )
         {
            if ( i != 0 )
            {
               b.Append( " || " );
            }

            b.Append( "BPE_TARGET==BPE_TARGET_" );
            b.Append( platforms[i] );
         }
      }

      private void AddPlatformIncludeBegin( StringBuilder b, string[] platforms )
      {
         if ( platforms != null )
         {
            b.Append( "#if ( " );

            AppendPlatformIfArgs( b, platforms );

            b.AppendLine( " )" );
         }
      }

      private void AddPlatformIncludeEnd( StringBuilder b, string[] platforms )
      {
         if ( platforms != null )
         {
            b.AppendLine( "#endif" );
            b.AppendLine();
         }
      }

      public void BuildPropertiesLoaderBody()
      {
         // Loader function declaration
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("CGameObjectComponentProperties * {0}::BuildComponentProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator * pAllocator)", mClassName));
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         
         AddTabs();
         mCB.AppendLine("uint32 const fourCC = applyProperties.mStream.ReadUint32();");

         AddTabs();
         mCB.AppendLine("uint16 size = applyProperties.mStream.ReadUint16();");

         AddTabs();
         mCB.AppendLine("switch (fourCC)");

         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         // Main body loading code
         {
            // Use xpath to identify components
            XPathNodeIterator componentsIter =  mNav.Select("/ComponentGroup/Component");
            while (componentsIter.MoveNext())
            {
               XPathNavigator componentNav = componentsIter.Current;

               string id = componentNav.GetAttribute(Helpers.kComponent_Id, "");
               string name = componentNav.GetAttribute(Helpers.kComponent_Name, ""); // For information in comment only
               string componentClassName = CodeGenHelpers.GetComponentClassName(mComponentGroupName, id);
               string ldrComponentClassName = CodeGenHelpers.GetComponentPropertiesClassName(mComponentGroupName, id);
               uint idHash = CodeGenHelpers.GetPropertyNameHash(id);

               string[] componentPlatforms = Helpers.GetComponentPlatforms( (XmlElement) componentNav.UnderlyingObject );

               AddPlatformIncludeBegin( mCB, componentPlatforms );

               AddTabs();
               mCB.AppendLine(String.Format(@"case {0:}::kComponentPropertiesType:   // 0x{1:X} / {2} - ({3})", ldrComponentClassName, CodeGenHelpers.GetStringHash(id), id, name));

               AddTabs();
               mCB.AppendLine("{");

               mTabCount++;
               AddTabs();
               mCB.AppendLine(String.Format("{0} * pComponentProperties = new {0}();", ldrComponentClassName));

               AddTabs();
               mCB.AppendLine(String.Format("pComponentProperties->ApplyProperties(applyProperties, pAllocator ? *pAllocator : pComponentProperties->mEvaluatorAllocator);"));

               AddTabs();
               mCB.AppendLine(String.Format("pComponentProperties->PostLoadUpdate();"));

               AddTabs();
               mCB.AppendLine(String.Format("return pComponentProperties;"));

               mTabCount--;
               AddTabs();
               mCB.AppendLine("}");

               AddPlatformIncludeEnd( mCB, componentPlatforms );

               // Write includes for this type (add to front of code)
               StringBuilder b = new StringBuilder();

               AddPlatformIncludeBegin( b, componentPlatforms );

               if ( mOptions != ECodeGenerationOptions.kGeneratePropertyCodeOnly )
               {
                  b.AppendLine( String.Format( "#include \"{0}.h\"", componentClassName ) );
               }

               b.AppendLine( String.Format( "#include \"{0}.h\"", ldrComponentClassName ) );

               AddPlatformIncludeEnd( b, componentPlatforms );

               mCB.Insert( 0, b.ToString() );
            }
         }
         // Null component case
         AddTabs();
         mCB.AppendLine("case 0:   // 'None' / Empty component.");
         mTabCount++;

         AddTabs();
         mCB.AppendLine("applyProperties.mStream.Get( NULL, size );");

         AddTabs();
         mCB.AppendLine("return NULL;");

         mTabCount--;

         // Default case
         AddTabs();
         mCB.AppendLine("default:");
         mTabCount++;
         AddTabs();
         mCB.AppendLine("property_load_error_printf( \"Unknown component 0x%08x in component loader " + mClassName + ".\\n\", fourCC );");

         AddTabs();
         mCB.AppendLine("applyProperties.mStream.Get( NULL, size );");

         AddTabs();
         mCB.AppendLine("return NULL;");

         mTabCount--;

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         mBodyIncludesCode.AppendLine();
      }

      public void BuildComponentBuilderBody()
      {
         // Builder declaration
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("CGameObjectComponent * const {0}::BuildComponent(boost::shared_ptr<CGameObjectComponentProperties const> const &pComponentProperties)\n", mClassName);

         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;

         AddTabs();
         mCB.AppendLine("// Check for NULL properties, this will occur when 'None' component is selected.");

         AddTabs();
         mCB.AppendLine("if (!pComponentProperties)");

         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;

         AddTabs();
         mCB.AppendLine("return NULL;");

         mTabCount--;

         AddTabs();
         mCB.AppendLine("}");

         mCB.AppendLine("");

         AddTabs();
         mCB.AppendLine("uint32 const fourCC = pComponentProperties->GetComponentType();");

         AddTabs();
         mCB.AppendLine("switch (fourCC)");

         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         // Main body loading code
         {
            // Use xpath to identify components
            XPathNodeIterator componentsIter =  mNav.Select("/ComponentGroup/Component");
            while (componentsIter.MoveNext())
            {
               XPathNavigator componentNav = componentsIter.Current;
               string id = componentNav.GetAttribute(Helpers.kComponent_Id, "");
               string name = componentNav.GetAttribute(Helpers.kComponent_Name, ""); // For information in comment only
               string componentClassName = CodeGenHelpers.GetComponentClassName(mComponentGroupName, id);
               string ldrComponentClassName = CodeGenHelpers.GetComponentPropertiesClassName(mComponentGroupName, id);
               uint idHash = CodeGenHelpers.GetPropertyNameHash(id);

               string[] componentPlatforms = Helpers.GetComponentPlatforms( (XmlElement) componentNav.UnderlyingObject );

               AddPlatformIncludeBegin( mCB, componentPlatforms );

               AddTabs();
               mCB.AppendLine(String.Format(@"case {0:}::kComponentPropertiesType:   // 0x{1:X} / {2} - ({3})", ldrComponentClassName, CodeGenHelpers.GetStringHash(id), id, name));

               AddTabs();
               mCB.AppendLine("{");

               mTabCount++;
               AddTabs();
               mCB.AppendLine(String.Format("{0} * pComponent = new {0}(pComponentProperties);", componentClassName));

               AddTabs();
               mCB.AppendLine(String.Format("return pComponent;"));

               mTabCount--;
               AddTabs();
               mCB.AppendLine("}");

               AddPlatformIncludeEnd( mCB, componentPlatforms );
            }
         }

         // Default case
         AddTabs();
         mCB.AppendLine("default:");
         mTabCount++;

         AddTabs();
         mCB.AppendLine("property_load_error_printf( \"Unknown component 0x%08x in component builder " + mClassName + ".\\n\", fourCC );");

         AddTabs();
         mCB.AppendLine("return NULL;");

         mTabCount--;

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

      }

      void BuildAutoRegisterFactoryCode()
      {
         AddTabs();
         mCB.AppendLine("// Register factory");
         AddTabs();
         switch( mOptions )
         {
            case ECodeGenerationOptions.kNormal:
               mCB.AppendFormat("BPE_FORCE_REFERENCE CGameObjectRegisterComponentGroupObjectFactory sRegisterFactory{0}( gsComponentGroupObjectFactories{1}, {0}::kComponentGroupType, {0}::BuildComponentProperties, {0}::BuildComponent);\n", mClassName, mGameObjectType);
               break;
            case ECodeGenerationOptions.kGeneratePropertyCodeOnly:
               mCB.AppendFormat("BPE_FORCE_REFERENCE CGameObjectRegisterComponentGroupObjectFactory sRegisterFactory{0}( gsComponentGroupObjectFactories{1}, {0}::kComponentGroupType, {0}::BuildComponentProperties, NULL);\n", mClassName, mGameObjectType);
               break;
         }
         mCB.AppendLine();
      }

      // Hacky way to add tabs quickly
      void AddTabs()
      {
         CodeGenHelpers.AddTabs(mCB, mTabCount);
      }

      void AddFunctionSeparator()
      {
         CodeGenHelpers.AddFunctionSeparator( mCB );
      }
   }
}
