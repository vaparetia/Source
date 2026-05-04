using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using Tools.Property;
using Tools.Property.Misc;

namespace Tools.Property.CodeGen
{
   /// <summary>
   /// Generates C++ code for evaluators
   /// </summary>
   public class EvaluatorCodeGen
   {
      public StringBuilder mDeclarationForwardsAndIncludesCode = new StringBuilder();
      public StringBuilder mMarkedDeclarationCode = new StringBuilder();
      public StringBuilder mPostDeclarationCode = new StringBuilder();
      
      public StringBuilder mPreConstructorBodyCode = new StringBuilder();
      public StringBuilder mMarkedConstructorCode = new StringBuilder();
      public StringBuilder mBodyCode = new StringBuilder();
      public StringBuilder mMarkedBodyCode = new StringBuilder();
      public StringBuilder mPostMarkedBodyCode = new StringBuilder();
      XmlDocument          mDocument;
      XPathNavigator       mNav;
      int                  mTabCount;
      string               mEvaluatorName;
      string               mClassExportDeclaration;
      public string        mEvaluatorType;
      public string        mEvaluatorScope;
      public string        mEvaluatorClassName;

      StringBuilder        mCB = null;       // Current builder

      public EvaluatorCodeGen(XPathNavigator srcEvaluator, int startingTabCount)
      {
         mDocument = Helpers.NavigatorToDocument(srcEvaluator);
         mTabCount = startingTabCount;
         mNav = mDocument.CreateNavigator();
         XPathNavigator nav = mNav.SelectSingleNode(@"/Evaluator[@id]");
         if (nav == null)
         {
            // This isn't a valid property object because it doesn't have a top level name
            return;
         }
         mEvaluatorName = nav.GetAttribute(Helpers.kEvaluator_Id, "");
         mEvaluatorType = nav.GetAttribute(Helpers.kEvaluator_Type, ""); ;
         mEvaluatorScope = nav.GetAttribute(Helpers.kEvaluator_Scope, ""); ;
         mEvaluatorClassName = CodeGenHelpers.GetEvaluatorClassName(mEvaluatorName, mEvaluatorType);
         mClassExportDeclaration = nav.GetAttribute(Helpers.kEvaluator_ExportDeclaration, String.Empty);
         if (mClassExportDeclaration.Length != 0)
         {
            mClassExportDeclaration += " ";  // Pad for neatness
         }

         BuildDeclaration();
         BuildBody();
      }

      public void BuildDeclaration()
      {
         BuildDeclarationForwardsAndIncludesCode();
         // Set active builder to declaration code
         mCB = mMarkedDeclarationCode;

         // Class declaration
         AddTabs();
         mCB.AppendLine(String.Format("class {0}{1} : public {2}", mClassExportDeclaration, mEvaluatorClassName, CodeGenHelpers.GetEvaluatorInterfaceClassFromType(mEvaluatorType)));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("public:");
         mTabCount++;
         AddTabs();

         mCB.AppendLine(String.Format("{0}(CEvaluatorAllocator &allocator);", mEvaluatorClassName));
         AddTabs();
         mCB.AppendLine(String.Format("virtual ~{0}();", mEvaluatorClassName));
         mCB.AppendLine();

         AddTabs();
         mCB.AppendLine("virtual void         Initialize(CEvaluatorUpdateData &updateData) const;");
         // GetValue (or other)
         AddTabs();

         string valueFunction = Get_ValueFunction_FromType(mEvaluatorType, String.Empty);
         if (valueFunction.Length != 0)
         {
            mCB.AppendLine(String.Format("virtual {0};", valueFunction));
            AddTabs();
         }

         mCB.AppendLine("virtual uint32       GetType() const;");
         AddTabs();
         mCB.AppendFormat("static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);\n");
         AddTabs();
         mCB.AppendFormat("virtual void         ApplyProperties{0};\n", CodeGenHelpers.kApplyPropertiesArguments);
         AddTabs();
         mCB.AppendLine("virtual void         PostLoadUpdate();");
         AddTabs();
         mCB.AppendLine("virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;\n");

         // Compile time id
         AddTabs();
         mCB.AppendFormat("{0, -45}   kType_0x{1:X08};\n", "static const uint32", CodeGenHelpers.GetPropertyNameHash(mEvaluatorName) );
         mCB.AppendLine();
         
         CodeGenHelpers.GeneratePropertiesEnumCode(new XmlNodeReader(mDocument), mCB, mTabCount);
         mCB.AppendLine();
         CodeGenHelpers.GenerateBitFieldsEnum(mDocument.CreateNavigator(), mCB, mTabCount);
         CodeGenHelpers.GenerateEnumCodeForEnumProperties(mDocument.CreateNavigator(), mCB, mTabCount);
         
         AddTabs();
         mCB.AppendLine("// Regular component properties");
         // Write var declarations
         {
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
         }
         // End of CRC marked block         
         mCB = mPostDeclarationCode;

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
      }

      public void BuildBody()
      {
         // Set active builder to body code
         mCB = mPreConstructorBodyCode;
         
         BeginHashDefineBlock("NO_EVALUATOR_REGISTRATION", false);
         BuildRegistrationCode();
         EndHashDefineBlock("NO_EVALUATOR_REGISTRATION");
         
         BeginHashDefineBlock("NO_EVALUATOR_CONSTRUCTOR", false);
         BuildClassBody();
         EndHashDefineBlock("NO_EVALUATOR_CONSTRUCTOR");
         
         BeginHashDefineBlock("NO_EVALUATOR_GET_VALUE", false);
         BuildGetValueBody();
         EndHashDefineBlock("NO_EVALUATOR_GET_VALUE");
         
         mCB = mMarkedBodyCode;
         BuildCompileTimeTypeBody();
         BeginHashDefineBlock("NO_EVALUATOR_TYPE", false);
         BuildMiscBody();
         EndHashDefineBlock("NO_EVALUATOR_TYPE");
         
         BeginHashDefineBlock("NO_EVALUATOR_LOADER", false);
         BuildFactoryBody();
         BuildLoaderBody();
         BuildCopyToLinearStreamBody();
         BuildPostLoaderBody();
         EndHashDefineBlock("NO_EVALUATOR_LOADER");
         
         BeginHashDefineBlock("NO_EVALUATOR_INITIALIZER", false);
         BuildInitializerBody();
         
         mCB = mPostMarkedBodyCode;
         AddTabs();
         mCB.AppendLine("}");
         EndHashDefineBlock("NO_EVALUATOR_INITIALIZER");
         
         mCB.AppendLine();
      }
      
      public void BuildRegistrationCode()
      {
         string factoryEvaluatorType = mEvaluatorType;
         switch (mEvaluatorType)
         {
            case Helpers.kEvaluatorProperty_Type_float:
               break;
            case Helpers.kEvaluatorProperty_Type_int:
               break;
            case Helpers.kEvaluatorProperty_Type_bool:
               break;
            case Helpers.kEvaluatorProperty_Type_color:
               break;
            case Helpers.kEvaluatorProperty_Type_vector3:
               break;
            case Helpers.kEvaluatorProperty_Type_modifierPV:
               break;
            case Helpers.kEvaluatorProperty_Type_emitterPV:
               break;
            case Helpers.kEvaluatorProperty_Type_userData:
               break;
            default:
               // Custom type
               factoryEvaluatorType = "custom";
               break;
         }

         string factoryName = String.Format("g{0}EvaluatorFactories{1}", factoryEvaluatorType, mEvaluatorScope);

         AddTabs();
         mCB.AppendLine(String.Format("extern CRegisteredEvaluatorFactories {0};", factoryName));
         
         mCB.AppendLine();
         AddTabs();
         mCB.AppendLine("// Register factory");
         AddTabs();
         mCB.AppendLine(String.Format("static CRegisterEvaluatorFactory sRegisterFactory{1}({0}, {1}::kType_0x{2:X08}, {1}::Factory);", factoryName, mEvaluatorClassName, CodeGenHelpers.GetPropertyNameHash(mEvaluatorName)));
         mCB.AppendLine();
      }

      public void BuildClassBody()
      {
         // Class declaration
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("{0}::{0}(CEvaluatorAllocator &allocator)", mEvaluatorClassName));
         // Added so we don't have to worry about first initializer, doesn't do anything
         mCB.AppendLine(String.Format(": {0}()", CodeGenHelpers.GetEvaluatorInterfaceClassFromType(mEvaluatorType)));
         
         // Marked block for internal initialization
         mCB = mMarkedConstructorCode;
         {
            // Write vars which need default initialization
            XmlNodeReader nodeReader = new XmlNodeReader(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( mDocument ));
            nodeReader.Read();   // Skip top level element
            bool bSkipped = false;
            bool bFirstInitialization = false;
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
         // Unmarked block for user initialization
         mCB = mBodyCode;

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("{0}::~{0}()", mEvaluatorClassName));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildGetValueBody()
      {
         // GetValue (or other)
         string valueFunction = Get_ValueFunction_FromType(mEvaluatorType, String.Empty);
         if (valueFunction.Length == 0)
         {
            // Custom type, no function
            return;
         }

         AddTabs();
         mCB.AppendLine(Get_ValueFunction_FromType(mEvaluatorType, mEvaluatorClassName)); 

         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }
      
      public void BuildFactoryBody()
      {
         // Compile time group name
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("IEvaluator * {0}::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)", mEvaluatorClassName)); 
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendLine(String.Format("{0} * pEval = new {0}(allocator);", mEvaluatorClassName)); 
         AddTabs();
         mCB.AppendLine(String.Format("CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);"));
         AddTabs();
         mCB.AppendLine("pEval->ApplyProperties(applyProperties, allocator);");
         AddTabs();
         mCB.AppendLine("return pEval;");
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }
      
      public void BuildCompileTimeTypeBody()
      {
         // Compile time type (in body so it appears in symbols)
         AddTabs();
         mCB.AppendFormat("uint32 const {0}::kType_0x{1:X08} = 0x{1:X08};\n", mEvaluatorClassName, CodeGenHelpers.GetPropertyNameHash(mEvaluatorName) );
         mCB.AppendLine();
      }
      
      public void BuildMiscBody()
      {
         // Type virtual
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("uint32 {0}::GetType() const", mEvaluatorClassName)); 
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("return kType_0x{0:X08};\n", CodeGenHelpers.GetPropertyNameHash(mEvaluatorName)); 
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
         mCB.AppendLine(String.Format("void {0}::ApplyProperties{1}", mEvaluatorClassName, CodeGenHelpers.kApplyPropertiesArguments));
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
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
         mCB.AppendLine("property_load_error_printf( \"Unknown property 0x%08x in evaluator loader " + mEvaluatorClassName + ".\\n\", fourCC );");

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

      public void BuildInitializerBody()
      {
         // Loader function declaration
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("void {0}::Initialize(CEvaluatorUpdateData &updateData) const", mEvaluatorClassName));
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
         // Find top level evaluators and initialize them
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
               
               if (nodeReader.Name == Helpers.kElement_EvaluatorProperty)
               {
                  string name = nodeReader.GetAttribute(Helpers.kEvaluatorProperty_Name, "");
                  AddTabs();
                  mCB.AppendLine(String.Format("if (m{0}.get()) m{0}->Initialize(updateData);", name));
                  nodeReader.Skip(); // Ignore all elements below this
                  bSkipped = true;
               }
            }
         }
         mTabCount--;
         // Close brace comes after marked block so we can add code afterwards without screwing the CRC
      }
      
      public void BuildPostLoaderBody()
      {
         // Post loader function - used for checking defaults and locking asset tokens
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("void {0}::PostLoadUpdate()", mEvaluatorClassName));
         AddTabs();
         mCB.AppendLine("{");

         mTabCount++;
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
      
      public void BuildCopyToLinearStreamBody()
      {
         // Compile time group name
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("uint32 {0}::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const", mEvaluatorClassName)); 
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendLine("uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);"); 
         AddTabs();
         mCB.AppendLine("int const classPtrBufferPos = stream.Put(this, sizeof(*this));"); 
         AddTabs();
         mCB.AppendLine("stream.SetValue(pVTable, classPtrBufferPos);"); 
         AddTabs();
         mCB.AppendLine();
         
         // Copy evaluator members
         {
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
               
               if (nodeReader.Name == Helpers.kElement_EvaluatorProperty)
               {
                  string name = nodeReader.GetAttribute(Helpers.kEvaluatorProperty_Name, "");
                  AddTabs();
                  mCB.AppendFormat("if (m{0})\n", name);
                  AddTabs();
                  mCB.AppendLine("{");
                  mTabCount++;
                  AddTabs();
                  mCB.AppendFormat("uint32 const evalPtr = m{0}->CopyToLinearStream(stream, evalToVTable);\n", name);
                  AddTabs();
                  mCB.AppendFormat("int const evalPtrOffset = (int) (((uint8*) &m{0}) - ((uint8*) this));\n", name);
                  AddTabs();
                  mCB.AppendFormat("stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy\n");
                  mTabCount--;
                  AddTabs();
                  mCB.AppendLine("}");
                  
                  nodeReader.Skip(); // Ignore all elements below this
                  bSkipped = true;
               }
               else if (nodeReader.Name == Helpers.kElement_Property)
               {
                  // Check for custom spline type
                  string type = CodeGenHelpers.GetCookType(nodeReader);
                  if (type == Helpers.kProperty_Type_custom)
                  {
                     string subType = nodeReader.GetAttribute(Helpers.kProperty_SubType, "");
                     if (subType == Helpers.kProperty_SubType_spline)
                     {
                        string name = nodeReader.GetAttribute(Helpers.kProperty_Name, "");
                        
                        AddTabs();
                        mCB.AppendLine("{");
                        mTabCount++;
                        AddTabs();
                        mCB.AppendFormat("int const memberPtrOffset = (int) (((uint8*) &m{0}) - ((uint8*) this));\n", name);
                        
                        AddTabs();
                        mCB.AppendFormat("// Pointers fixed up in CopyToLinear_Inplace\n");
                        
                        AddTabs();
                        mCB.AppendFormat("m{0}.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);\n", name);
                        mTabCount--;
                        AddTabs();
                        mCB.AppendLine("}");
                        
                        nodeReader.Skip(); // Ignore all elements below this
                        bSkipped = true;
                     }
                  }
               }
            }
         }
         
         AddTabs();
         mCB.AppendLine("return stream.BufferPosToBufferPointer(classPtrBufferPos);");
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }
      
      
      /// <summary>
      /// Returns the basic function signature for the given type.
      /// </summary>
      /// <returns></returns>
      static string Get_ValueFunction_FromType(string type, string classPrefix)
      {
         string returnType = "";
         switch (type)
         {
            case Helpers.kEvaluatorProperty_Type_float:
               returnType =  "real32";
               break;
            case Helpers.kEvaluatorProperty_Type_int:
               returnType =  "int32";
               break;
            case Helpers.kEvaluatorProperty_Type_bool:
               returnType =  "bool";
               break;
            case Helpers.kEvaluatorProperty_Type_color:
               returnType =  "CColorf";
               break;
            case Helpers.kEvaluatorProperty_Type_vector3:
               returnType =  "CVector3";
               break;
            case Helpers.kEvaluatorProperty_Type_modifierPV:
               returnType =  "uint32";
               break;
            case Helpers.kEvaluatorProperty_Type_emitterPV:
               returnType =  "void";
               break;
            case Helpers.kEvaluatorProperty_Type_userData:
               returnType = "void";               
               break;
            default:
               // Must be custom type, no value function
               return "";
         }
         
         string functionSignature = "UnknownFunction()";
         switch (type)
         {
            case Helpers.kEvaluatorProperty_Type_int:
            case Helpers.kEvaluatorProperty_Type_float:
            case Helpers.kEvaluatorProperty_Type_bool:
            case Helpers.kEvaluatorProperty_Type_color:
            case Helpers.kEvaluatorProperty_Type_vector3:
               functionSignature = "GetValue(CEvaluatorUpdateData const &updateData) const";
               break;
            case Helpers.kEvaluatorProperty_Type_modifierPV:
               functionSignature = "ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const";
               break;
            case Helpers.kEvaluatorProperty_Type_emitterPV:
               functionSignature = "GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const";
               break;
            case Helpers.kEvaluatorProperty_Type_userData:
               functionSignature = "GetUserData(CEvaluatorUpdateData const &updateData, void * pBufferOut) const";
               break;
         }
         string function = String.Format("{0} {1}{2}{3}", returnType, classPrefix, ((classPrefix.Length != 0) ? "::" : String.Empty), functionSignature);
      
         return function;
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

      void BeginHashDefineBlock(string defineTag, bool mustBeDefined)
      {
         // No tabs on these
         mCB.AppendFormat("#if {0}defined({1})\n\n", mustBeDefined ? "" : "!", defineTag);
      }
      
      void EndHashDefineBlock(string defineTag)
      {
         // No tabs on these
         mCB.AppendFormat("#endif //{0}\n\n", defineTag);
      }
   }
}


