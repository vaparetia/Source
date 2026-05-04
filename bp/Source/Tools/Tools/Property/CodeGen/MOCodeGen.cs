using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using Tools.Property;
using Tools.Property.UI;
using Tools.Property.Misc;
using Tools.Property.Build;

namespace Tools.Property.CodeGen
{
   /// <summary>
   /// Generates C++ code for loading message objects
   /// </summary>
   public class MOCodeGen
   {
      public StringBuilder    mDeclarationCode = new StringBuilder();
      public StringBuilder    mBodyCode = new StringBuilder();
      XmlDocument             mDocument;
      XPathNavigator          mNav;
      int                     mTabCount;
      string                  mMessageName;
      string                  mTypeName;
      bool                    mbAutoRegisterFactory = true;
      string                  mExportDeclaration;
      public string           mClassName;
      StringBuilder           mCB = null;       // Current builder

      public MOCodeGen(XPathNavigator srcMessageObject, int startingTabCount)
      {
         mDocument = Helpers.NavigatorToDocument(srcMessageObject);
         mTabCount = startingTabCount;
         mNav = mDocument.CreateNavigator();
         mNav = mNav.SelectSingleNode(@"/MessageObject[@id]");
         if (mNav == null)
         {
            // This isn't a valid message object because it doesn't have a top level name
            return;
         }
         mMessageName = mNav.GetAttribute(Helpers.kMessage_id, "");
         mTypeName = mNav.GetAttribute(Helpers.kMessage_type, "");
         mClassName = CodeGenHelpers.GetPropertiesClassName(mMessageName, MergeProperties.EBuildTreeSrcType.kMessageObject);
         if (mNav.GetAttribute(Helpers.kMessage_registerFactory, "").Length != 0)
         {
            mbAutoRegisterFactory = Tools.Common.Misc.ParseBool((mNav.GetAttribute(Helpers.kMessage_registerFactory, ""))); 
         }
         // Find export declaration
         {
            XPathNavigator exportNav =  mNav.SelectSingleNode("/MessageObject[@exportDeclaration]");
            if (exportNav != null)
            {
               mExportDeclaration = exportNav.GetAttribute(Helpers.kMessage_exportDeclaration, String.Empty);
               mExportDeclaration += " ";  // Pad for neatness!
            }
         }
         
         BuildDeclaration();
         BuildBody();
      }

      public void BuildDeclaration()
      {
         // Set active builder to declaration code
         mCB = mDeclarationCode;

         // Class declaration
         AddTabs();
         mCB.AppendLine(String.Format("class {0}{1} : public CGameObjectMessageProperties", mExportDeclaration, mClassName));
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

         // Loader function declaration
         AddTabs();
         mCB.AppendFormat("{0, -30}   ApplyProperties{1};\n", "void", CodeGenHelpers.kApplyPropertiesArguments);
         AddTabs();
         mCB.AppendFormat("{0, -30}   PostLoadUpdate();\n", "void");
         AddTabs();
         mCB.AppendFormat("{0, -30}   GetMessageType() const;\n", "virtual uint32");
         AddTabs();
         mCB.AppendFormat("{0, -30}   InitializeEvaluators(CEvaluatorUpdateData &updateData) const;\n", "virtual void");
         mCB.AppendLine();

         AddTabs();
         {
            string cookAs = mNav.GetAttribute(Helpers.kMessage_cookAs, "");
            string fourCC = null;
            if (cookAs.Length == 0)
            {
               fourCC = String.Format("0x{0:X}", Helpers.GetMessageNameHash(mMessageName));
            }
            else
            {
               fourCC = String.Format("'{0}'", cookAs);
            }
            
            mCB.AppendFormat("{0, -45}   kMessageType = {1};\n", "static const uint32", fourCC );
         }
         mCB.AppendLine();
         
         // Property enums
         CodeGenHelpers.GeneratePropertiesEnumCode(new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument )), mCB, mTabCount);
         mCB.AppendLine();
         CodeGenHelpers.GenerateBitFieldsEnum(mDocument.CreateNavigator(), mCB, mTabCount);
         CodeGenHelpers.GenerateEnumCodeForEnumProperties(mDocument.CreateNavigator(), mCB, mTabCount);
         
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
         if (mbAutoRegisterFactory)
         {
            BuildAutoRegisterFactoryCode();
         }
         BuildClassBody();
         BuildLoaderBody();
         BuildPostLoaderBody();
         BuildMiscFunctionsBody();
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
         mCB.AppendLine("property_load_error_printf( \"Unknown property 0x%08x in message object loader " + mClassName + ".\\n\", fourCC );");

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

      public void BuildMiscFunctionsBody()
      {
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("uint32 {0}::GetMessageType() const\n", mClassName);
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;

         AddTabs();
         mCB.AppendLine("return kMessageType;");

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      void BuildAutoRegisterFactoryCode()
      {
         AddTabs();
         mCB.AppendFormat("static CGameObjectMessageProperties * {0}Factory(void)\n", mClassName);
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;

         AddTabs();
         mCB.AppendFormat("return new {0}();\n", mClassName);

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         mCB.AppendLine("// Register factory");
         mCB.AppendFormat("static CGameObjectRegisterMessageObjectFactory sRegisterFactory{0}( gsMessageObjectFactories{1}, {0}::kMessageType, {0}Factory);\n", mClassName, mTypeName);
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
