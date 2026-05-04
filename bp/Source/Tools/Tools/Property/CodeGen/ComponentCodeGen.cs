using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using Tools.Property;
using Tools.Property.Build;
using Tools.Property.Misc;

namespace Tools.Property.CodeGen
{
   /// <summary>
   /// Generates C++ code for loading property objects
   /// </summary>
   public class ComponentCodeGen
   {
      public StringBuilder mDeclarationForwardsAndIncludesCode = new StringBuilder();
      public StringBuilder mDeclarationCode = new StringBuilder();
      public StringBuilder mBodyIncludesCode = new StringBuilder();
      public StringBuilder mBodyCode = new StringBuilder();
      XmlDocument          mDocument;
      XPathNavigator       mNav;
      int                  mTabCount;
      string               mComponentName;
      public string        mComponentGroupName;
      public string        mComponentPropertiesClassName;
      string               mClassExportDeclaration;
      StringBuilder        mCB = null;       // Current builder

      public ComponentCodeGen(XPathNavigator srcComponent, int startingTabCount)
      {
         mDocument = Helpers.NavigatorToDocument(srcComponent);
         mTabCount = startingTabCount;
         mNav = mDocument.CreateNavigator();
         XPathNavigator nav = mNav.SelectSingleNode(@"/Component[@id]");
         if (nav == null)
         {
            // This isn't a valid property object because it doesn't have a top level name
            return;
         }
         mComponentName = nav.GetAttribute(Helpers.kComponent_Id, "");
         mComponentGroupName = nav.GetAttribute(Helpers.kComponent_Group, ""); ;
         mComponentPropertiesClassName = CodeGenHelpers.GetComponentPropertiesClassName(mComponentGroupName, mComponentName);
         
         // Find kComponent_classPropertiesExportDeclaration
         {
            XPathNavigator componentNav =  mNav.SelectSingleNode("/Component[@classPropertiesExportDeclaration]");
            if (componentNav != null)
            {
               mClassExportDeclaration = componentNav.GetAttribute(Helpers.kComponent_classPropertiesExportDeclaration, String.Empty);
               mClassExportDeclaration += " ";  // Pad for neatness!
            }
         }

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
         mCB.AppendLine(String.Format("class {0}{1} : public {2}", 
                                             mClassExportDeclaration,
                                             mComponentPropertiesClassName, 
                                             CodeGenHelpers.GetPropertiesClassName(mComponentGroupName, MergeProperties.EBuildTreeSrcType.kComponentGroup)));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("public:");
         mTabCount++;
         AddTabs();
         mCB.AppendLine(String.Format("{0}();", mComponentPropertiesClassName));
         AddTabs();
         mCB.AppendLine(String.Format("virtual ~{0}();", mComponentPropertiesClassName));
         mCB.AppendLine();

         mCB.AppendLine( "#pragma region \"Component Property Internal Glue Code\"" );

         // Loader function declaration
         AddTabs();
         mCB.AppendLine(String.Format("{0, -45}   ApplyProperties{1};", "virtual void", CodeGenHelpers.kApplyPropertiesArguments));
         // Post loader function declaration
         AddTabs();
         mCB.AppendLine(String.Format("{0, -45}   PostLoadUpdate();", "virtual void"));
         
         // Compile time name hash
         AddTabs();
         mCB.AppendFormat("{0, -45}   GetComponentType() const;\n", "virtual uint32");
         
         // Compile time name
         AddTabs();
         mCB.AppendFormat("{0, -45}   GetComponentTypeName() const;\n", "virtual char const *"); 
         
         // Virtual copy constructor
         AddTabs();
         mCB.AppendFormat("{0, -45}   clone() const;\n", "virtual CGameObjectComponentProperties * ");
         mCB.AppendLine();

         // Initialize evaluators
         AddTabs();
         mCB.AppendLine(String.Format("{0, -45}   InitializeEvaluators(CEvaluatorUpdateData &updateData) const;", "void"));
         mCB.AppendLine();

         // Compile time id
         AddTabs();
         mCB.AppendFormat("{0, -45}   kComponentPropertiesType = 0x{1:X};\n", "static const uint32", CodeGenHelpers.GetPropertyNameHash(mComponentName) );
         mCB.AppendLine();
         
         CodeGenHelpers.GeneratePropertiesEnumCode(new XmlNodeReader(mDocument), mCB, mTabCount);
         mCB.AppendLine( "#pragma endregion" );
         mCB.AppendLine();
         CodeGenHelpers.GenerateBitFieldsEnum(mDocument.CreateNavigator(), mCB, mTabCount);
         CodeGenHelpers.GenerateEnumCodeForEnumProperties(mDocument.CreateNavigator(), mCB, mTabCount);

         AddTabs();
         mCB.AppendLine( "// Regular component properties" );
         // Write var declarations
         {
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ) );
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            while (bSkipped || nodeReader.Read())
            {
               bSkipped = false;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               bSkipped = CodeGenHelpers.GeneratePropertyDeclaration(nodeReader, mCB, mTabCount);
            }
         }

         // Add code to assign default components if no components were found in the property list
         {
            // Write vars which need loading
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ) );
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            while (bSkipped || nodeReader.Read())
            {
               bSkipped = false;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               if (nodeReader.Name == Helpers.kElement_ComponentGroup)
               {
                  bSkipped = true;
                  // Generate statement to check for null
                  string name = nodeReader.GetAttribute(Helpers.kComponentGroup_Name, "");
                  AddTabs();
                  mCB.AppendLine(String.Format("{0,-45}   mb{1}Initialized : 1;", "bool", name));
                  nodeReader.Skip();
               }
            }
         }

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
         mCB.AppendFormat("#include \"{0}.h\"\n", CodeGenHelpers.GetPropertiesClassName(mComponentGroupName, MergeProperties.EBuildTreeSrcType.kComponentGroup));
         AddTabs();
         mCB.AppendLine("#include \"boost/optional.hpp\"");
         AddTabs();
         mCB.AppendLine("#include \"Engine/Resource/CResourceManager.h\"");
         mCB.AppendLine();
         AddTabs();
         mCB.AppendLine("class CInputStream;");
         AddTabs();
         mCB.AppendLine("class CEvaluatorUpdateData;");
         AddTabs();
         mCB.AppendLine("class IEvaluator;");
         AddTabs();
         mCB.AppendLine("class IBoolEvaluator;");
         AddTabs();
         mCB.AppendLine("class IIntEvaluator;");
         AddTabs();
         mCB.AppendLine("class IFloatEvaluator;");
         AddTabs();
         mCB.AppendLine("class IColorEvaluator;");
         AddTabs();
         mCB.AppendLine("class IVectorEvaluator;");
         AddTabs();
         mCB.AppendLine("class IModifierPVEvaluator;");
         AddTabs();
         mCB.AppendLine("class ICustomEvaluator;");
         AddTabs();
         mCB.AppendLine("class IUserDataEvaluator;");
         
         mCB.AppendLine();
         // Forward declare all asset classes
         foreach(AssetSystem.AssetType assetType in AssetSystem.AssetTypes.mAssetTypes)
         {
            if (assetType.ClassName.Length != 0)
            {
               AddTabs();
               mCB.AppendFormat("class {0};\n", assetType.ClassName);
            }
         }

         mCB.AppendLine();
      }

      public void BuildBody()
      {
         mCB = mBodyIncludesCode;
         AddTabs();
         mCB.AppendLine("#include \"StdAfx.h\"");
         mCB.AppendLine();
         AddTabs();
         mCB.AppendLine("#include \"../ComponentPropertiesCommonCppIncludes.h\"");
         AddTabs();
         mCB.AppendFormat("#include \"{0}.h\"\n", mComponentPropertiesClassName);

         {
            // Write asset includes
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ) );
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            while (bSkipped || nodeReader.Read())
            {
               bSkipped = false;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               bSkipped = CodeGenHelpers.GeneratePropertyIncludes(nodeReader, mCB, mTabCount);
            }
         }
         mCB.AppendLine();

         // Set active builder to body code
         mCB = mBodyCode;
         BuildClassBody();
         BuildNameHashBody();
         BuildLoaderBody();
         BuildPostLoaderBody();
         BuildInitializeEvaluatorsBody();
      }

      public void BuildClassBody()
      {
         // Class declaration
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("{0}::{0}()", mComponentPropertiesClassName));
         {
            // Write vars which need default initialization
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ));
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            bool bFirstInitialization = true;
            while (bSkipped || nodeReader.Read())
            {
               bSkipped = false;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               bSkipped = CodeGenHelpers.GeneratePropertyDefaultInitialization(nodeReader, ref bFirstInitialization, mCB, mTabCount);
            }
         }

         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;

         // Set default values
         {
            // Write vars which need default initialization
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ));
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            while (bSkipped || nodeReader.Read())
            {
               bSkipped = false;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               bSkipped = CodeGenHelpers.GeneratePropertyDefaults(nodeReader, mCB, mTabCount);
            }
         }

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("{0}::~{0}()", mComponentPropertiesClassName));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildNameHashBody()
      {
         // Compile time group name
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("char const * {0}::GetComponentTypeName() const\n", mComponentPropertiesClassName); 

         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         AddTabs();
         mCB.AppendFormat("return \"{0}\";\n", mComponentName);

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         // Compile time group name
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("uint32 {0}::GetComponentType() const", mComponentPropertiesClassName)); 

         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         AddTabs();
         mCB.AppendLine(String.Format("return kComponentPropertiesType;  // 0x{0:X};", CodeGenHelpers.GetPropertyNameHash(mComponentName)));

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("CGameObjectComponentProperties * {0}::clone() const\n", mComponentPropertiesClassName);
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         AddTabs();
         mCB.AppendLine(String.Format("return new {0}(*this);", mComponentPropertiesClassName));

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildLoaderBody()
      {
         // Loader function declaration
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("void {0}::ApplyProperties{1}", mComponentPropertiesClassName, CodeGenHelpers.kApplyPropertiesArguments));
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         AddTabs();
         mCB.AppendLine(CodeGenHelpers.kUnrefApplyPropertiesArguments);
         
         AddTabs();
         mCB.AppendLine("if (applyProperties.mStream.ReadBool())");
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         
         AddTabs();
         mCB.AppendLine("mEditorId = TComponentEditorId(applyProperties.mStream);");
         
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");

         AddTabs();
         mCB.AppendLine("int32 const numProperties = applyProperties.mStream.ReadUint16();");

         AddTabs();
         mCB.AppendLine("for (int loop = 0; loop < numProperties; loop++)");

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
            // Write vars which need loading
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ));
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            while (bSkipped || nodeReader.Read())
            {                                 
               bSkipped = false;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               bSkipped = CodeGenHelpers.GeneratePropertyLoadingCode(nodeReader, mCB, mTabCount);
            }
         }

         // Default case
         AddTabs();
         mCB.AppendLine("default:");
         mTabCount++;

         AddTabs();
         mCB.AppendLine("property_load_error_printf( \"Unknown property 0x%08x in component properties loader " + mComponentPropertiesClassName + ".\\n\", fourCC );");

         AddTabs();
         mCB.AppendLine("applyProperties.mStream.Get( NULL, size );");

         AddTabs();
         mCB.AppendLine("break;");

         mTabCount--;
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildPostLoaderBody()
      {
         // Post loader function - used for checking defaults and locking asset tokens
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("void {0}::PostLoadUpdate()", mComponentPropertiesClassName));
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         AddTabs();
         mCB.AppendLine("// Add code to assign default components if no components were found in the loaded property list.");
         AddTabs();
         mCB.AppendLine("// Also lock resources if present and call PostLoadUpdate on child properties.");
         {
            // Write vars which need loading
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ));
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            while (bSkipped || nodeReader.Read())
            {
               bSkipped = false;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               bSkipped = CodeGenHelpers.GeneratePostLoaderCode(nodeReader, mCB, mTabCount);
            }
         }

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }
      
      void BuildInitializeEvaluatorsBody()
      {
         // Post loader function - used for checking defaults and locking asset tokens
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("void {0}::InitializeEvaluators(CEvaluatorUpdateData &updateData) const", mComponentPropertiesClassName));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("(updateData);");
         mCB.AppendLine();

         mTabCount++;
         {
            // Write vars which need loading
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ));
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            while (bSkipped || nodeReader.Read())
            {
               bSkipped = false;
               if (nodeReader.NodeType != XmlNodeType.Element)
               {
                  // Only interested in elements
                  continue;
               }
               bSkipped = CodeGenHelpers.GenerateInitializeEvaluatorCode(nodeReader, mCB, mTabCount);
            }
         }

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
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

