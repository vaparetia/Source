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
   public class ComponentGroupClassCodeGen
   {
      public StringBuilder    mDeclarationForwardsAndIncludesCode = new StringBuilder();
      public StringBuilder    mDeclarationCode = new StringBuilder();
      public StringBuilder    mPostDeclarationCode = new StringBuilder();
      public StringBuilder    mBodyIncludesCode = new StringBuilder();
      public StringBuilder    mBodyCode = new StringBuilder();
      XmlDocument             mDocument;
      XPathNavigator          mNav;
      int                     mTabCount;
      public string           mComponentGroupName;
      public string           mClassName;
      string                  mPropertiesClassName;
      string                  mGameObjectType;
      string                  mClassExportDeclaration;
      StringBuilder           mCB = null;       // Current builder

      public ComponentGroupClassCodeGen(XPathNavigator srcComponentGroup, int startingTabCount)
      {
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
         }
         
         // Find kComponent_classExportDeclaration
         {
            XPathNavigator componentNav =  mNav.SelectSingleNode("/ComponentGroup/Component[@classExportDeclaration]");
            if (componentNav != null)
            {
               mClassExportDeclaration = componentNav.GetAttribute(Helpers.kComponent_classExportDeclaration, String.Empty);
               mClassExportDeclaration += " ";  // Pad for neatness!
            }
         }

         mComponentGroupName = nav.GetAttribute(Helpers.kComponent_Id, "");
         mClassName = CodeGenHelpers.GetComponentGroupClassName(mComponentGroupName);
         mPropertiesClassName = CodeGenHelpers.GetPropertiesClassName(mComponentGroupName, MergeProperties.EBuildTreeSrcType.kComponentGroup);
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
         mCB.AppendLine(String.Format("class {0}{1} : public CGameObjectComponent{2}", mClassExportDeclaration, mClassName, mGameObjectType));
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("typedef CGameObjectComponent{0} inherited;\n", mGameObjectType);
         mTabCount--;
         AddTabs();
         mCB.AppendLine("public:");
         mTabCount++;
         // Leave constructor/desructor for future expansion
         AddTabs();
         mCB.AppendLine(String.Format("{0}(boost::shared_ptr<CGameObjectComponentProperties const > const &pProperties);", mClassName));
         AddTabs();
         mCB.AppendLine(String.Format("virtual ~{0}();", mClassName));
         mCB.AppendLine();

         // Find component declaration
         AddTabs();
         mCB.AppendFormat("{0, -54}   FindBaseComponent(CGameObject const &object);\n", String.Format("static {0} const *", mClassName));
         AddTabs();
         mCB.AppendFormat("{0, -54}   FindBaseComponent(CGameObject &object);\n", String.Format("static {0} *", mClassName));

         // Cast component declaration
         AddTabs();
         mCB.AppendFormat("{0, -54}   Cast(CGameObjectComponent const * pComponent);\n", String.Format("static {0} const *", mClassName));
         AddTabs();
         mCB.AppendFormat("{0, -54}   Cast(CGameObjectComponent * pComponent);\n", String.Format("static {0} *", mClassName));
         
         mTabCount--;

         mCB = mPostDeclarationCode;
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
         mCB.AppendFormat("#include \"../CGameObjectComponent{0}.h\"\n", mGameObjectType);
         AddTabs();
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
         mCB.AppendLine("#include \"Engine/GameObjectSystem/CGameObject.h\"");
         mCB.AppendLine();
         AddTabs();
         mCB.AppendFormat("#include \"{0}.h\"\n", mClassName);
         AddTabs();
         mCB.AppendFormat("#include \"{0}.h\"\n", mPropertiesClassName);
         mCB.AppendLine();

         // Set active builder to body code
         BuildClassBody();
         mCB = mBodyCode;
         BuildMiscFunctionBody();
      }

      public void BuildClassBody()
      {
         // Class constructor, destructor
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendLine(String.Format("{0}::{0}(boost::shared_ptr<CGameObjectComponentProperties const > const &pProperties)", mClassName));
         AddTabs();
         mCB.AppendFormat(": inherited(pProperties)\n");

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
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("{0} * {0}::FindBaseComponent(CGameObject &object)\n", mClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("{0} * pBaseComponent = static_cast<{0} *>( object.BaseComponentByType({1}::kComponentGroupType));\n", mClassName, mPropertiesClassName);

         AddTabs();
         mCB.AppendLine("return pBaseComponent;");
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();


         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("{0} const * {0}::FindBaseComponent(CGameObject const &object)\n", mClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("{0} const * pBaseComponent = static_cast<{0} const *>( object.GetBaseComponentByType({1}::kComponentGroupType));\n", mClassName, mPropertiesClassName);

         AddTabs();
         mCB.AppendLine("return pBaseComponent;");
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("{0} const * {0}::Cast(CGameObjectComponent const * pComponent)\n", mClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("if (!pComponent) return NULL;\n");
         AddTabs();
         mCB.AppendFormat("if (pComponent->GetProperties()->GetComponentGroupType() != {0}::kComponentGroupType) return NULL;\n", mPropertiesClassName);
         AddTabs();
         mCB.AppendFormat("return static_cast<{0} const *>(pComponent);\n", mClassName);
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("{0} * {0}::Cast(CGameObjectComponent * pComponent)\n", mClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("if (!pComponent) return NULL;\n");
         AddTabs();
         mCB.AppendFormat("if (pComponent->GetProperties()->GetComponentGroupType() != {0}::kComponentGroupType) return NULL;\n", mPropertiesClassName);
         AddTabs();
         mCB.AppendFormat("return static_cast<{0} *>(pComponent);\n", mClassName);
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
