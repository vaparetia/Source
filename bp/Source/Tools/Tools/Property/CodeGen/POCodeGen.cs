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
   public class POCodeGen
   {
      public StringBuilder    mDeclarationCode = new StringBuilder();
      public StringBuilder    mBodyCode = new StringBuilder();
      XmlDocument             mDocument;
      int                     mTabCount;
      string                  mPropertyName;
      public string           mClassName;
      string                  mExportDeclaration;
      StringBuilder           mCB = null;       // Current builder

      public POCodeGen(XPathNavigator srcPropertyObject, int startingTabCount)
      {
         mDocument = Helpers.NavigatorToDocument(srcPropertyObject);
         mTabCount = startingTabCount;
         XPathNavigator nav = mDocument.CreateNavigator();
         nav = nav.SelectSingleNode(@"/PropertyObject[@id]");
         if (nav == null)
         {
            // This isn't a valid property object because it doesn't have a top level name
            return;
         }
         // Find export declaration
         {
            XPathNavigator exportNav =  nav.SelectSingleNode("/PropertyObject[@exportDeclaration]");
            if (exportNav != null)
            {
               mExportDeclaration = exportNav.GetAttribute(Helpers.kPropertyObject_exportDeclaration, String.Empty);
               mExportDeclaration += " ";  // Pad for neatness!
            }
         }
         
         
         mPropertyName = nav.GetAttribute(Helpers.kProperty_Id, "");
         mClassName = CodeGenHelpers.GetPropertiesClassName(mPropertyName, MergeProperties.EBuildTreeSrcType.kPropertyObject);
         BuildDeclaration();
         BuildBody();
      }

      public void BuildDeclaration()
      {
         // Set active builder to declaration code
         mCB = mDeclarationCode;

         // Forward declarations
         AddTabs();
         mCB.AppendLine("class CEvaluatorAllocator;");
         AddTabs();
         mCB.AppendLine("class CEvaluatorUpdateData;");
         AddTabs();
         mCB.AppendLine("class IEvaluatorFactory;");
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
         mCB.AppendLine("class CGameObjectApplyPropertiesData;");
         AddTabs();
         mCB.AppendLine("class ICustomEvaluator;");
         AddTabs();
         mCB.AppendLine("class IUserDataEvaluator;");
         mCB.AppendLine();
         
         // Class declaration
         AddTabs();
         mCB.AppendLine(String.Format("class {0}{1}", mExportDeclaration, mClassName));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("public:");
         mTabCount++;
         AddTabs();
         mCB.AppendLine(String.Format("{0}();", mClassName));
         AddTabs();
         mCB.AppendLine(String.Format("~{0}();", mClassName));
         mCB.AppendLine();
         
         // Property enums
         CodeGenHelpers.GeneratePropertiesEnumCode(new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument )), mCB, mTabCount);
         mCB.AppendLine();
         CodeGenHelpers.GenerateBitFieldsEnum(mDocument.CreateNavigator(), mCB, mTabCount);
         CodeGenHelpers.GenerateEnumCodeForEnumProperties(mDocument.CreateNavigator(), mCB, mTabCount);
         
         // Loader function declaration
         AddTabs();
         mCB.AppendLine(String.Format("void\tApplyProperties{0};",CodeGenHelpers.kApplyPropertiesArguments));
         AddTabs();
         mCB.AppendLine(String.Format("void\tPostLoadUpdate();"));
         // Initialize evaluators
         AddTabs();
         mCB.AppendLine(String.Format("void\tInitializeEvaluators(CEvaluatorUpdateData &updateData) const;"));
         mCB.AppendLine();

         // Write var declarations
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
            bSkipped = CodeGenHelpers.GeneratePropertyDeclaration(nodeReader, mCB, mTabCount);
         }

         mTabCount--;
         AddTabs();
         mCB.AppendLine("};");
         mCB.AppendLine();
      }

      public void BuildBody()
      {
         // Set active builder to declaration code
         mCB = mBodyCode;
         BuildClassBody();
         BuildLoaderBody();
         BuildPostLoaderBody();
         BuildInitializeEvaluatorsBody();
      }

      public void BuildClassBody()
      {
         // Class declaration
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("{0}::{0}()", mClassName));
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
         mCB.AppendLine(String.Format("{0}::~{0}()", mClassName));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildLoaderBody()
      {
         // Loader function declaration
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("void {0}::ApplyProperties{1}", mClassName, CodeGenHelpers.kApplyPropertiesArguments));
         AddTabs();
         mCB.AppendLine("{");
         
         mTabCount++;
         AddTabs();
         mCB.AppendLine(CodeGenHelpers.kUnrefApplyPropertiesArguments);
         
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
         mCB.AppendLine("property_load_error_printf( \"Unknown property 0x%08x in property object loader " + mClassName + ".\\n\", fourCC );");

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
         mCB.AppendLine(String.Format("void {0}::PostLoadUpdate()", mClassName));
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         // Add code to lock resources if present and to call PostLoadUpdate on child properties
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
         mCB.AppendLine(String.Format("void {0}::InitializeEvaluators(CEvaluatorUpdateData &updateData) const", mClassName));
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         AddTabs();
         mCB.AppendLine("(updateData);");
         mCB.AppendLine();
         
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
