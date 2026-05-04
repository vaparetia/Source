using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using Tools.Property;
using Tools.Property.Source;
using Tools.Property.Build;
using Tools.Property.Misc;

namespace Tools.Property.CodeGen
{
   /// <summary>
   /// Generates C++ code for loading property objects
   /// </summary>
   public class ComponentClassCodeGen
   {
      public class Method
      {
         public Method(string methodType, string methodFunction, string returnType, string arguments, string boostBindArguments, string forwardDeclarations)
         {
            mMethodType =           methodType;
            mMethodFunction =       methodFunction;
            mReturnType =           returnType;  
            mArguments =            arguments;  
            mBoostBindArguments =   boostBindArguments;
            mForwardDeclarations =  forwardDeclarations;
         }
         
         public string     mMethodType;
         public string     mMethodFunction;
         public string     mReturnType;
         public string     mArguments;
         public string     mBoostBindArguments;
         public string     mForwardDeclarations;
      };

      class Methods
      {
         public class GameObjectTypeMethods
         {
            public GameObjectTypeMethods(string gameObjectType, Method[] methods)
            {
               mGameObjectType = gameObjectType;
               mMethods = methods;
            }
            public string     mGameObjectType;
            public Method[]   mMethods;
         };

         static public List<Method> GetMethodsByType(string gameObjectType, string methodType, string[] functionFilters)
         {
            List<Method> matchingMethods = new List<Method>();
            foreach(GameObjectTypeMethods methods in mGameObjectTypeMethods)
            {
               if (methods.mGameObjectType == gameObjectType)
               {
                  foreach(Method method in methods.mMethods)
                  {
                     if (method.mMethodType == methodType)
                     {
                        bool bMatched = false;
                        foreach(string functionFilter in functionFilters)
                        {
                           if (functionFilter == method.mMethodFunction)
                           {
                              bMatched = true;
                              break;
                           }
                        }
                        if (!bMatched)
                        {
                           matchingMethods.Add(method);
                        }
                     }
                  }
               }
            }
            return matchingMethods;
         }

         static readonly private GameObjectTypeMethods[] mGameObjectTypeMethods = new GameObjectTypeMethods[]
         {
            new GameObjectTypeMethods( "Game",  new Method[] {
                  new Method( "Message",        "ProcessMessage",       "void", "(CGameManager const &gameManager, CGameObjectMessage const &message)", ", _1, _2", "class CGameManager;\n"),
                  new Method( "ScriptTouch",    "GetScriptBounds",      "ICollisionPrimitive", "(CGameManager const &gameManager)", ", _1", "class CGameManager;\n"),
                  new Method( "ScriptTouch",    "ProcessScriptTouch",   "void", "(CGameManager const &gameManager, CGameCObject &gameObject)", ", _1, _2", "class CGameManager;\nclass CGameObject;\n"),
                  new Method( "Move",           "Move",                 "void", "(CGameManager const &gameManager, real32 const deltaTime)", ", _1, _2", "class CGameManager;\n"),
                  new Method( "Think",          "Think",                "void", "(CGameManager &gameManager, real32 const deltaTime)", ", _1, _2", "class CGameManager;\n"),
                  new Method( "Render",         "GlobalPrerender",      "void", "(CGameManager const &gameManager)", ", _1", "class CGameManager;\n"),
                  new Method( "Render",         "ViewportPrerender",    "void", "(CGameManager const &gameManager)", ", _1", "class CGameManager;\n"),
                  new Method( "Render",         "Render",               "void", "(CGameManager const &gameManager, CGameRenderObjects &renderObjects) const", ", _1, _2", "class CGameManager;\nclass CGameRenderObjects;\n"),
                  new Method( "DebugRender",    "DebugRender",          "void", "(CGameManager const &gameManager) const", ", _1", "class CGameManager;\n"),
               }
            )
         };
      }

      //void                 ProcessMessage(CGameManager const &gameManager, CGameObjectMessage const &message);
      //ICollisionPrimitive  GetScriptBounds(CGameManager &gameManager, real32 const deltaTime);
      //void                 ProcessScriptTouch(CGameManager &gameManager, CGameObject &gameObject);
      //void                 Move(CGameManager &gameManager, real32 const deltaTime);
      //void                 Think(CGameManager &gameManager, real32 const deltaTime);
      //void                 GlobalPrerender(CGameManager const &gameManager, CRenderManager &renderManager);
      //void                 ViewportPrerender(CGameManager const &gameManager, CRenderManager &renderManager, CViewportState const &viewportState);
      //void                 Render(CGameManager const &gameManager, CRenderManager const &renderManager, CViewportState const &viewportState) const;
	   //void                 DebugRender(CGameManager const &gameManager, CRenderManager const &renderManager, CViewportState const &viewportState) const;

      public StringBuilder mDeclarationForwardsAndIncludesCode = new StringBuilder();
      public StringBuilder mDeclarationStartCode = new StringBuilder();
      public StringBuilder mDeclarationMiddleCode = new StringBuilder();
      public StringBuilder mDeclarationEndCode = new StringBuilder();
      public StringBuilder mBodyCode = new StringBuilder();
      public StringBuilder mEventsEnumCode = new StringBuilder();
      public StringBuilder mMessagesEnumCode = new StringBuilder();
      public StringBuilder mLinksEnumCode = new StringBuilder();
      public StringBuilder mMessagesForwardDeclarationCode = new StringBuilder();
      public StringBuilder mMessagesDeclarationCode = new StringBuilder();
      public StringBuilder mInternalProcessMessageBodyCode = new StringBuilder();
      public StringBuilder mMessagesBodyCode = new StringBuilder();
      public StringBuilder mMethodsDeclarationCode = new StringBuilder();
      public StringBuilder mMethodsBodyCode = new StringBuilder();
      public StringBuilder mMethodsBindBodyCode = new StringBuilder();
      public StringBuilder mFindComponentBodyCode = new StringBuilder();
      public StringBuilder mMiscBodyCode = new StringBuilder();
      XmlDocument          mDocument;
      XPathNavigator       mNav;
      IPropertyDataSource  mPropertySource;
      int                  mTabCount;
      string               mComponentName;
      public string        mComponentGroupName;
      string               mComponentGroupClassName;
      string               mComponentGameObjectType;
      string               mComponentEvaluatorUpdateType;
      string               mComponentMethodsType;
      string               mClassExportDeclaration;
      public string        mComponentClassName;
      string               mComponentPropertiesClassName;
      string               mComponentGroupPropertiesClassName;
      StringBuilder        mCB = null;       // Current builder
      bool                 mHasDynamicMessages = false;

      public ComponentClassCodeGen(XPathNavigator srcComponent, IPropertyDataSource propertySource, int startingTabCount)
      {
         mDocument = Helpers.NavigatorToDocument(srcComponent);
         mTabCount = startingTabCount;
         mPropertySource = propertySource;
         mNav = mDocument.CreateNavigator();
         XPathNavigator nav = mNav.SelectSingleNode(@"/Component[@id]");
         if (nav == null)
         {
            // This isn't a valid property object because it doesn't have a top level name
            return;
         }
         mComponentName = nav.GetAttribute(Helpers.kComponent_Id, "");
         mComponentGroupName = nav.GetAttribute(Helpers.kComponent_Group, ""); ;
         mComponentGroupClassName = CodeGenHelpers.GetComponentGroupClassName(mComponentGroupName);
         mComponentGameObjectType = nav.GetAttribute(Helpers.kComponentGroup_GameObjectType, ""); ;
         mComponentEvaluatorUpdateType = nav.GetAttribute(Helpers.kComponentGroup_EvaluatorUpdateType, ""); ;
         if (mComponentEvaluatorUpdateType.Length == 0)
         {
            // Use game object type for evaluator update data code-gen
            mComponentEvaluatorUpdateType = mComponentGameObjectType;
         }                                                     
         mComponentMethodsType = nav.GetAttribute(Helpers.kComponent_UseMethods, String.Empty);
         if (mComponentMethodsType.Length == 0) mComponentMethodsType = mComponentGameObjectType;
         mComponentClassName = CodeGenHelpers.GetComponentClassName(mComponentGroupName, mComponentName);
         mComponentPropertiesClassName = CodeGenHelpers.GetComponentPropertiesClassName(mComponentGroupName, mComponentName);
         mComponentGroupPropertiesClassName = CodeGenHelpers.GetPropertiesClassName(mComponentGroupName, MergeProperties.EBuildTreeSrcType.kComponentGroup);
         
         // Find kComponent_classExportDeclaration
         {
            XPathNavigator componentNav =  mNav.SelectSingleNode("/Component[@classExportDeclaration]");
            if (componentNav != null)
            {
               mClassExportDeclaration = componentNav.GetAttribute(Helpers.kComponent_classExportDeclaration, String.Empty);
               mClassExportDeclaration += " ";  // Pad for neatness!
            }
         }

         mHasDynamicMessages = mNav.SelectSingleNode( String.Format( "/Component/Messages/{0}", Helpers.kElement_DynamicItems ) ) != null;         
         
         BuildDeclaration();
         BuildBody();
      }

      public void BuildDeclaration()
      {
         BuildDeclarationForwardsAndIncludesCode();
         BuildEventsEnum();
         BuildLinksEnum();
         BuildMessagesEnum();
         mCB.AppendLine();
         BuildMessagesForwardDeclaration();
         BuildMessagesDeclaration();
         BuildMethodsDeclaration();

         // Set active builder to declaration code
         mCB = mDeclarationStartCode;

         // Class declaration
         AddTabs();
         mCB.AppendFormat("class {0}{1} : public {2}\n", 
                              mClassExportDeclaration,
                              mComponentClassName, 
                              mComponentGroupClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("typedef {0} inherited;\n", mComponentGroupClassName);
         mTabCount--;
         AddTabs();
         mCB.AppendLine("public:");
         mTabCount++;
         AddTabs();
         mCB.AppendLine(String.Format("{0}(boost::shared_ptr<CGameObjectComponentProperties const> const &pProperties);", mComponentClassName));
         AddTabs();
         mCB.AppendLine(String.Format("virtual ~{0}();", mComponentClassName));
         mCB.AppendLine();

         mCB.AppendLine( "#pragma region \"Common Component Functions\"" );
         // Initialize
         AddTabs();
         mCB.AppendFormat("{0, -21}   Initialize(CBaseGameManager &baseGameManager, CGameObject &gameObjectOwner, TComponentUniqueId const &uniqueId, CGameObjectConstructionParams const &params);\n", "virtual void");
         AddTabs();
         mCB.AppendFormat("{0, -21}   InitializeEvaluators(CBaseGameManager &baseGameManager);\n", "void");
         AddTabs();
         mCB.AppendFormat("{0, -21}   BindMethods(CGameObjectComponentMethods &methods) const;\n", "virtual void");         
         
         // Duplicate properties
         AddTabs();
         mCB.AppendFormat("{0, -21}   DuplicateProperties();\n", "virtual void");
         mCB.AppendLine();
         
         // Helper so we can get message names from int32 types when debug logging
         AddTabs();
         mCB.AppendFormat("{0, -21}   GetMessageTypeName(int32 const messageType) const;\n", "virtual char const *");
         AddTabs();
         mCB.AppendFormat("{0, -21}   GetEventTypeName(int32 const eventType) const;\n", "virtual char const *");
         mCB.AppendLine();
         
         // Casting
         AddTabs();
         mCB.AppendFormat("{0, -54}   FindComponent(CGameObject const &object);\n", String.Format("static {0} const *", mComponentClassName));
         AddTabs();
         mCB.AppendFormat("{0, -54}   FindComponent(CGameObject &object);\n", String.Format("static {0} *", mComponentClassName));
         AddTabs();
         mCB.AppendFormat("{0, -54}   Cast(CGameObjectComponent const * pComponent);\n", String.Format("static {0} const *", mComponentClassName));
         AddTabs();
         mCB.AppendFormat("{0, -54}   Cast(CGameObjectComponent * pComponent);\n", String.Format("static {0} *", mComponentClassName));
         mCB.AppendLine( "#pragma endregion" );
         mCB.AppendLine();

         mCB = mDeclarationMiddleCode;

         // Required member vars, maybe some these could be in CComponent?
         //boost::shared_ptr<CCPLdrmodelStatic const>   mpProperties;
         //std::vector<SScriptEventMessage>             mEventMessages;    // Copied from properties to allow modification. If empty, uses messages in properties.
         //std::vector<SScriptEventMessage>             mLinks;            // Copied from properties to allow modification. If empty, uses links in properties.
         //CGuid                                        mGuidUniqueId;
         //CGameObject *                                mParentGameObject;

         AddTabs();
         mCB.AppendFormat("{0, -45}   mpProperties;\n", String.Format("{0} const *", mComponentPropertiesClassName));
         mTabCount--;

         // Add "public:" declaration under properties so people know what the current access in the 
         // class is.
         mCB.AppendLine();
         AddTabs();
         mCB.AppendLine( "public:" );
         
         mCB = mDeclarationEndCode;
         AddTabs();
         mCB.AppendLine("};");
         mCB.AppendLine();
      }

      public void BuildBody()
      {
         // Set active builder to body code
         mCB = mBodyCode;
         BuildBodyIncludesCode();
         BuildClassBody();

         // Stub functions
         BuildMethodsBody();
         BuildMessagesBody();
         BuildMiscBody();

         // Code gen update goes here
         BuildInternalProcessMessageBody();
         BuildMethodsBindBody();
         BuildFindComponentBody();
         BuildDuplicatePropertiesBody();
         BuildGetEventTypeName();
         BuildGetMessageTypeName();
      }

      public void BuildClassBody()
      {
         // Class declaration
         AddTabs();
         mCB.AppendLine(String.Format("{0}::{0}(boost::shared_ptr<CGameObjectComponentProperties const> const &pProperties)", mComponentClassName));
         AddTabs();
         mCB.AppendFormat(" : {0}(pProperties)\n", mComponentGroupClassName);
         AddTabs();
         mCB.AppendFormat(" , mpProperties(static_cast<{0} const *>(pProperties.get()))\n", mComponentPropertiesClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddTabs();
         mCB.AppendLine(String.Format("{0}::~{0}()", mComponentClassName));
         AddTabs();
         mCB.AppendLine("{");
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildDeclarationForwardsAndIncludesCode()
      {
         mCB = mDeclarationForwardsAndIncludesCode;

         AddTabs();
         mCB.AppendLine("#pragma once");
         mCB.AppendLine();
         AddTabs();
         mCB.AppendFormat("#include \"{0}.h\"\n", mComponentGroupClassName);
         mCB.AppendLine();
         AddTabs();
         mCB.AppendLine("class CBaseGameManager;");
         AddTabs();
         mCB.AppendLine("class CGameObjectMessage;");
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
         mCB.AppendLine("class ICustomEvaluator;");
         AddTabs();
         mCB.AppendLine("class IUserDataEvaluator;");
         AddTabs();
         mCB.AppendFormat("class {0};\n", mComponentPropertiesClassName);
         AddTabs();
         mCB.AppendFormat("class C{0}Manager;\n", mComponentMethodsType);
         mCB.AppendLine();

      }

      public void BuildBodyIncludesCode()
      {
         AddTabs();
         mCB.AppendLine("#include \"StdAfx.h\"");
         mCB.AppendLine();
         AddTabs();
         mCB.AppendLine("#include \"Engine/GameObjectSystem/CGameObjectMessageProperties.h\"");
         AddTabs();
         mCB.AppendLine("#include \"Engine/GameObjectSystem/CGameObject.h\"");
         AddTabs();
         mCB.AppendFormat("#include \"../CGameObjectComponentMethods{0}.h\"\n", mComponentGameObjectType);
         AddTabs();
         mCB.AppendFormat("#include \"../CEvaluatorUpdateData{0}.h\"\n", mComponentEvaluatorUpdateType);
         mCB.AppendLine();
         mCB.AppendFormat("#include \"Game/CGameManager.h\"\n");
         mCB.AppendLine();
         AddTabs();
         mCB.AppendFormat("#include \"{0}.h\"\n", mComponentPropertiesClassName);
         AddTabs();
         mCB.AppendFormat("#include \"{0}.h\"\n", mComponentClassName);
         mCB.AppendLine();

         // Message includes
         XPathNodeIterator messagesNav = mNav.Select("/Component/Messages/Message");
         while (messagesNav.MoveNext())
         {
            XPathNavigator messageNav = messagesNav.Current;
            string type = messageNav.GetAttribute(Helpers.kMessage_type, "");
            AddTabs();
            if (Helpers.MessageInComponentHasProperties(messageNav, mPropertySource))
            {
               mCB.AppendFormat("#include \"../Messages/CMOLdr{0}.h\"\n", type);               
            }
         }
         if (messagesNav.Count != 0)
         {
            // Add a padding line
            mCB.AppendLine();
         }
      }

      public void BuildEventsEnum()
      {
         mCB = mEventsEnumCode;
         mTabCount++;
         AddTabs();
         mCB.AppendLine("enum EEvents");
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         XPathNodeIterator eventsNav = mNav.Select("/Component/Events/Event");
         while (eventsNav.MoveNext())
         {
            XPathNavigator eventNav = eventsNav.Current;
            string type = eventNav.GetAttribute(Helpers.kEvent_type, "");
            string cookAs = eventNav.GetAttribute(Helpers.kEvent_cookAs, "");
            string fourCC = null;
            if (cookAs.Length == 0)
            {
               fourCC = String.Format("0x{0:X08}", CodeGenHelpers.GetPropertyNameHash(type));
            }
            else
            {
               fourCC = String.Format("'{0}'", cookAs);
            }
            AddTabs();
            mCB.AppendFormat("kEvent_{0,-29}   = {1}{2}\n", type, fourCC, (eventsNav.CurrentPosition != eventsNav.Count) ? "," : "");
         }
         mTabCount--;
         AddTabs();
         mCB.AppendLine("};");
         mCB.AppendLine();
         mTabCount--;
      }

      public void BuildGetEventTypeName()      
      {
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("char const * {0}::GetEventTypeName(int32 const eventType) const\n", mComponentClassName);
         
         mCB.AppendLine("{");
         mTabCount++;
         XPathNodeIterator eventsNav = mNav.Select("/Component/Events/Event");
         if (eventsNav.Count != 0)
         {
            AddTabs();
            mCB.AppendLine("switch (eventType)");
            AddTabs();
            mCB.AppendLine("{");
            mTabCount++;

            while (eventsNav.MoveNext())
            {
               XPathNavigator eventNav = eventsNav.Current;
               string type = eventNav.GetAttribute(Helpers.kEvent_type, "");

               AddTabs();
               mCB.AppendFormat("case kEvent_{0}:\n", type);
               mTabCount++;
               AddTabs();
               mCB.AppendFormat("return \"{0}\";\n", type);
               mTabCount--;
            }
            
            mTabCount--;
            AddTabs();
            mCB.AppendLine("}");

            mCB.AppendLine();
         }
         AddTabs();
         mCB.AppendLine("return inherited::GetEventTypeName(eventType);");
         
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }
      
      public void BuildLinksEnum()
      {
         mCB = mLinksEnumCode;
         mTabCount++;
         AddTabs();
         mCB.AppendLine("enum ELinks");
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         XPathNodeIterator linksNav = mNav.Select("/Component/Links/Link");
         while (linksNav.MoveNext())
         {
            XPathNavigator linkNav = linksNav.Current;
            string type = linkNav.GetAttribute(Helpers.kLink_type, "");
            string cookAs = linkNav.GetAttribute(Helpers.kLink_cookAs, "");
            string fourCC = null;
            if (cookAs.Length == 0)
            {
               fourCC = String.Format("0x{0:X08}", CodeGenHelpers.GetPropertyNameHash(type));
            }
            else
            {
               fourCC = String.Format("'{0}'", cookAs);
            }
            AddTabs();
            mCB.AppendFormat("kLink_{0,-30}   = {1}{2}\n", type, fourCC, (linksNav.CurrentPosition != linksNav.Count) ? "," : "");
         }
         mTabCount--;
         AddTabs();
         mCB.AppendLine("};");
         mCB.AppendLine();
         mTabCount--;
      }

      public void BuildMessagesEnum()
      {
         mCB = mMessagesEnumCode;
         mTabCount++;
         AddTabs();
         mCB.AppendLine("// Messages without properties");
         AddTabs();
         mCB.AppendLine("enum EMessages");
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         XPathNodeIterator messagesNav = mNav.Select("/Component/Messages/Message");
         while (messagesNav.MoveNext())
         {
            XPathNavigator messageNav = messagesNav.Current;
            string type = messageNav.GetAttribute(Helpers.kMessage_type, "");
            AddTabs();
            if (!Helpers.MessageInComponentHasProperties(messageNav, mPropertySource))
            {
               mCB.AppendFormat("kMessage_{0,-29}   = 0x{1:X08}{2}\n", type, Helpers.GetMessageNameHash(type), (messagesNav.CurrentPosition != messagesNav.Count) ? "," : "");
            }
         }
         mCB.AppendLine();
         mTabCount--;
         AddTabs();
         mCB.AppendLine("};");
         mCB.AppendLine();
         mTabCount--;
      }
      
      public void BuildGetMessageTypeName()      
      {
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("char const * {0}::GetMessageTypeName(int32 const messageType) const\n", mComponentClassName);
         
         mCB.AppendLine("{");
         mTabCount++;
         
         XPathNodeIterator messagesNav = mNav.Select("/Component/Messages/Message");
         if (messagesNav.Count != 0)
         {
            AddTabs();
            mCB.AppendLine("switch (messageType)");
            AddTabs();
            mCB.AppendLine("{");
            mTabCount++;

            while (messagesNav.MoveNext())
            {
               XPathNavigator messageNav = messagesNav.Current;
               string type = messageNav.GetAttribute(Helpers.kMessage_type, "");

               if (Helpers.MessageInComponentHasProperties(messageNav, mPropertySource))
               {
                  AddTabs();
                  mCB.AppendFormat("case CMOLdr{0}::kMessageType:\n", type);
                  mTabCount++;
                  AddTabs();
                  mCB.AppendFormat("return \"{0}\";\n", type);
                  mTabCount--;
               }
               else
               {
                  AddTabs();
                  mCB.AppendFormat("case kMessage_{0}:\n", type);
                  mTabCount++;
                  AddTabs();
                  mCB.AppendFormat("return \"{0}\";\n", type);
                  mTabCount--;
               }
            }
            mTabCount--;
            AddTabs();
            mCB.AppendLine("}");
            mCB.AppendLine();
         }

         AddTabs();
         mCB.AppendLine("return inherited::GetMessageTypeName(messageType);");
         
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildMessagesForwardDeclaration()
      {
         mCB = mMessagesForwardDeclarationCode;
         AddTabs();
         mCB.AppendLine("// Message classes forward declaration");
         AddTabs();
         XPathNodeIterator messagesNav = mNav.Select("/Component/Messages/Message");
         while (messagesNav.MoveNext())
         {
            XPathNavigator messageNav = messagesNav.Current;
            string type = messageNav.GetAttribute(Helpers.kMessage_type, "");
            AddTabs();
            if (Helpers.MessageInComponentHasProperties(messageNav, mPropertySource))
            {
               mCB.AppendFormat("class CMOLdr{0};\n", type);
            }
         }
         mCB.AppendLine();
      }

      public void BuildMessagesDeclaration()
      {
         mCB = mMessagesDeclarationCode;
         mTabCount++;
         AddTabs();
         mCB.AppendLine("// Messages");
         AddTabs();
         mCB.AppendFormat("{0,-18}   InternalProcessMessage(CBaseGameManager &baseGameManager, CGameObjectMessage const &message, CGameObjectMessageInfo const &messageInfo);\n", "void");
         XPathNodeIterator messagesNav = mNav.Select("/Component/Messages/Message");
         while (messagesNav.MoveNext())
         {
            XPathNavigator messageNav = messagesNav.Current;
            string type = messageNav.GetAttribute(Helpers.kMessage_type, "");
            AddTabs();
            if (Helpers.MessageInComponentHasProperties(messageNav, mPropertySource))
            {
               mCB.AppendFormat("{0,-18}   ProcessMessage_{1}(C{2}Manager &manager, CGameObjectMessage const &message, CGameObjectMessageInfo const &messageInfo, CMOLdr{1} const &properties);\n", "void", type, mComponentMethodsType);
            }
            else
            {
               mCB.AppendFormat("{0,-18}   ProcessMessage_{1}(C{2}Manager &manager, CGameObjectMessage const &message, CGameObjectMessageInfo const &messageInfo);\n", "void", type, mComponentMethodsType);
            }
         }
         if (mHasDynamicMessages)
         {
            AddTabs();
            mCB.AppendFormat("{0, -18}   ProcessMessage_Unknown(C{1}Manager &manager, CGameObjectMessage const &message, CGameObjectMessageInfo const &messageInfo);\n", "void", mComponentMethodsType);
         }
         mTabCount--;
         mCB.AppendLine();
      }

      public void BuildInternalProcessMessageBody()
      {
         mCB = mInternalProcessMessageBodyCode;
         // Count number of messages
         XPathNodeIterator messagesNav = mNav.Select("/Component/Messages/Message");
         if (messagesNav.Count == 0 && !mHasDynamicMessages)
         {
            // No messages, just stub function
            AddFunctionSeparator();
            AddTabs();
            mCB.AppendFormat("void {0}::InternalProcessMessage(CBaseGameManager &, CGameObjectMessage const &, CGameObjectMessageInfo const &)\n", mComponentClassName);
            AddTabs();
            mCB.AppendLine("{");
            AddTabs();
            mCB.AppendLine("}");
            mCB.AppendLine();
            return;
         }

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("void {0}::InternalProcessMessage(CBaseGameManager &baseGameManager, CGameObjectMessage const &message, CGameObjectMessageInfo const &messageInfo)\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendLine("(baseGameManager);");
         AddTabs();
         mCB.AppendLine("(messageInfo);");
         AddTabs();
         mCB.AppendLine("CGameObjectMessageProperties const * const pProperties = message.mpProperties.get();");
         AddTabs();
         mCB.AppendLine("switch (pProperties->GetMessageType())");
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;

         while (messagesNav.MoveNext())
         {
            XPathNavigator messageNav = messagesNav.Current;
            string type = messageNav.GetAttribute(Helpers.kMessage_type, "");

            if (Helpers.MessageInComponentHasProperties(messageNav, mPropertySource))
            {
               AddTabs();
               mCB.AppendFormat("case CMOLdr{0}::kMessageType:\n", type);
               AddTabs();
               mCB.AppendLine("{");
               mTabCount++;
               AddTabs();
               mCB.AppendFormat("CMOLdr{0} const &properties = *static_cast<CMOLdr{0} const * const>(pProperties);\n", type);
               AddTabs();
               mCB.AppendFormat("ProcessMessage_{0}(*static_cast<C{1}Manager*>(&baseGameManager), message, messageInfo, properties);\n", type, mComponentMethodsType);
               mTabCount--;
               AddTabs();
               mCB.AppendLine("}");
               AddTabs();
               mCB.AppendLine("break;");
            }
            else
            {
               AddTabs();
               mCB.AppendFormat("case kMessage_{0}:\n", type);
               mTabCount++;
               AddTabs();
               mCB.AppendFormat("ProcessMessage_{0}(*static_cast<C{1}Manager*>(&baseGameManager), message, messageInfo);\n", type, mComponentMethodsType);
               AddTabs();
               mCB.AppendLine("break;");
               mTabCount--;
            }
         }
         AddTabs();
         mCB.AppendLine("default:");
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         if (mHasDynamicMessages)
         {
            AddTabs();
            mCB.AppendLine("// This component has imported messages, so pass to the \"Unknown\" message handler");
            AddTabs();
            mCB.AppendFormat("ProcessMessage_Unknown(*static_cast<C{0}Manager*>(&baseGameManager), message, messageInfo);\n", mComponentMethodsType);
         }
         else
         {
            AddTabs();
            mCB.AppendLine("// Maybe something so we can breakpoint here?");
         }
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         AddTabs();
         mCB.AppendLine("break;");

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");

         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildMessagesBody()
      {
         mCB = mMessagesBodyCode;
         AddTabs();
         XPathNodeIterator messagesNav = mNav.Select("/Component/Messages/Message");
         while (messagesNav.MoveNext())
         {
            XPathNavigator messageNav = messagesNav.Current;
            string type = messageNav.GetAttribute(Helpers.kMessage_type, "");
            AddFunctionSeparator();
            AddTabs();
            if (Helpers.MessageInComponentHasProperties(messageNav, mPropertySource))
            {
               mCB.AppendFormat("void {0}::ProcessMessage_{1}(C{2}Manager &manager, CGameObjectMessage const &message, CGameObjectMessageInfo const &messageInfo, CMOLdr{1} const &properties)\n", mComponentClassName, type, mComponentMethodsType);
               AddTabs();
               mCB.AppendLine("{");
               mTabCount++;
               AddTabs();
               mCB.AppendLine("(properties);");
            }
            else
            {
               mCB.AppendFormat("void {0}::ProcessMessage_{1}(C{2}Manager &manager, CGameObjectMessage const &message, CGameObjectMessageInfo const &messageInfo)\n", mComponentClassName, type, mComponentMethodsType);
               AddTabs();
               mCB.AppendLine("{");
               mTabCount++;
            }
            AddTabs();
            mCB.AppendLine("(manager);");
            AddTabs();
            mCB.AppendLine("(message);");
            AddTabs();
            mCB.AppendLine("(messageInfo);");
            mTabCount--;
            mCB.AppendLine("}");
            mCB.AppendLine();
         }
         if (mHasDynamicMessages)
         {
            AddFunctionSeparator();
            AddTabs();
            mCB.AppendFormat("void {0}::ProcessMessage_Unknown(C{1}Manager &manager, CGameObjectMessage const &message, CGameObjectMessageInfo const &messageInfo)\n", mComponentClassName, mComponentMethodsType);
            AddTabs();
            mCB.AppendLine("{");
            mTabCount++;
            AddTabs();
            mCB.AppendLine("(manager);");
            AddTabs();
            mCB.AppendLine("(message);");
            AddTabs();
            mCB.AppendLine("(messageInfo);");
            mTabCount--;
            mCB.AppendLine("}");
            mCB.AppendLine();
         }
         mCB.AppendLine();
      }

      public void BuildMethodsDeclaration()
      {
         mCB = mMethodsDeclarationCode;
         mTabCount++;
         AddTabs();
         mCB.AppendLine("// Methods");
         XPathNodeIterator methodsNav = mNav.Select("/Component/Methods/Method");
         while (methodsNav.MoveNext())
         {
            XPathNavigator methodNav = methodsNav.Current;
            string type = methodNav.GetAttribute(Helpers.kMethod_type, "");
            string ignoreFunctions = methodNav.GetAttribute(Helpers.kMethod_ignoreFunctions, "");
            string[] functionFilters = ignoreFunctions.Split(new char[] {';'});
            List<Method> methods = Methods.GetMethodsByType(mComponentMethodsType, type, functionFilters);
            foreach(Method method in methods)
            {
               AddTabs();
               mCB.AppendFormat("{0,-24}   {1}{2};\n", method.mReturnType, method.mMethodFunction, method.mArguments);

               // Write forward declarations for classes
               mDeclarationForwardsAndIncludesCode.Append(method.mForwardDeclarations);
            }
         }
         mTabCount--;
         mCB.AppendLine();

         mDeclarationForwardsAndIncludesCode.AppendLine();
      }

      public void BuildMethodsBody()
      {
         mCB = mMethodsBodyCode;
         XPathNodeIterator methodsNav = mNav.Select("/Component/Methods/Method");
         while (methodsNav.MoveNext())
         {
            XPathNavigator methodNav = methodsNav.Current;
            string type = methodNav.GetAttribute(Helpers.kMethod_type, "");
            string ignoreFunctions = methodNav.GetAttribute(Helpers.kMethod_ignoreFunctions, "");
            string[] functionFilters = ignoreFunctions.Split(new char[] {';'});
            List<Method> methods = Methods.GetMethodsByType(mComponentMethodsType, type, functionFilters);
            foreach(Method method in methods)
            {
               AddFunctionSeparator();
               AddTabs();
               mCB.AppendFormat("{0} {1}::{2}{3}\n", method.mReturnType, mComponentClassName, method.mMethodFunction, method.mArguments);
               AddTabs();
               mCB.AppendLine("{");
               AddTabs();
               mCB.AppendLine("}");
               mCB.AppendLine();
            }
         }
      }

      public void BuildMethodsBindBody()
      {
         mCB = mMethodsBindBodyCode;
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("void {0}::BindMethods(CGameObjectComponentMethods &methods) const\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         // Disabled assertion as we don't support RTTI ingame
         //AddTabs();
         //mCB.AppendFormat("BPE_ASSERT(dynamic_cast<CGameObjectComponentMethods{0}*>(&methods) != NULL, \"Incorrect type of CGameObjectComponentMethod in {1}::BindMethods, s/be method CGameObjectComponentMethods{0}.\");\n", mComponentMethodsType, mComponentClassName);
         AddTabs();
         mCB.AppendFormat("CGameObjectComponentMethods{0} &methods{0} = *static_cast<CGameObjectComponentMethods{0}*>(&methods);\n", mComponentMethodsType);
         AddTabs();
         mCB.AppendFormat("(methods{0});\n", mComponentMethodsType);
         mCB.AppendLine();

         XPathNodeIterator methodsNav = mNav.Select("/Component/Methods/Method");
         while (methodsNav.MoveNext())
         {
            XPathNavigator methodNav = methodsNav.Current;
            string type = methodNav.GetAttribute(Helpers.kMethod_type, "");
            string ignoreFunctions = methodNav.GetAttribute(Helpers.kMethod_ignoreFunctions, "");
            string[] functionFilters = ignoreFunctions.Split(new char[] {';'});
            List<Method> methods = Methods.GetMethodsByType(mComponentMethodsType, type, functionFilters);
            foreach(Method method in methods)
            {
               AddTabs();
               mCB.AppendFormat("methods{0}.m{1}.push_back(fastdelegate::MakeDelegate(const_cast<{2}*>(this), &{2}::{1}));\n",  mComponentMethodsType,
                                                                                                                  method.mMethodFunction, 
                                                                                                                  mComponentClassName);
            }
         }
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      public void BuildMiscBody()
      {
         mCB = mMiscBodyCode;

         // Initialize
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("void {0}::Initialize(CBaseGameManager &baseGameManager, CGameObject &gameObjectOwner, TComponentUniqueId const &uniqueId, CGameObjectConstructionParams const &params)\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("inherited::Initialize(baseGameManager, gameObjectOwner, uniqueId, params);\n");
         AddTabs();
         mCB.AppendFormat("InitializeEvaluators(baseGameManager);\n");
         AddTabs();
         mCB.AppendFormat("C{0}Manager &manager = *static_cast<C{0}Manager*>(&baseGameManager);\n", mComponentMethodsType);
         AddTabs();
         mCB.AppendFormat("(manager);\n");
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
         
         // Initialize evaluators
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("void {0}::InitializeEvaluators(CBaseGameManager &baseGameManager)\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("// You can replace the code below with special case functionality if required\n");
         AddTabs();
         mCB.AppendFormat("CEvaluatorUpdateData{0} updateData(GetInitializationEvaluatorUpdateData(baseGameManager));\n", mComponentEvaluatorUpdateType);
         AddTabs();
         mCB.AppendFormat("mpProperties->InitializeEvaluators(updateData);\n");
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }

      void BuildFindComponentBody()
      {
         // Casting
         mCB = mFindComponentBodyCode;
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("{0} * {0}::FindComponent(CGameObject &object)\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("{0} * pComponent = static_cast<{0} *>( object.ComponentByType({1}::kComponentGroupType, {2}::kComponentPropertiesType));\n", mComponentClassName, mComponentGroupPropertiesClassName, mComponentPropertiesClassName);

         AddTabs();
         mCB.AppendLine("return pComponent;");
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();


         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("{0} const * {0}::FindComponent(CGameObject const &object)\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("{0} const * pComponent = static_cast<{0} const *>( object.GetComponentByType({1}::kComponentGroupType, {2}::kComponentPropertiesType));\n", mComponentClassName, mComponentGroupPropertiesClassName, mComponentPropertiesClassName);

         AddTabs();
         mCB.AppendLine("return pComponent;");
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("{0} const * {0}::Cast(CGameObjectComponent const * pComponent)\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("if (!pComponent) return NULL;\n");
         AddTabs();
         mCB.AppendFormat("if (pComponent->GetProperties()->GetComponentGroupType() != {0}::kComponentGroupType) return NULL;\n", mComponentGroupPropertiesClassName);
         AddTabs();
         mCB.AppendFormat("if (pComponent->GetProperties()->GetComponentType() != {0}::kComponentPropertiesType) return NULL;\n", mComponentPropertiesClassName);
         AddTabs();
         mCB.AppendFormat("return static_cast<{0} const *>(pComponent);\n", mComponentClassName);
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();

         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("{0} * {0}::Cast(CGameObjectComponent * pComponent)\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("if (!pComponent) return NULL;\n");
         AddTabs();
         mCB.AppendFormat("if (pComponent->GetProperties()->GetComponentGroupType() != {0}::kComponentGroupType) return NULL;\n", mComponentGroupPropertiesClassName);
         AddTabs();
         mCB.AppendFormat("if (pComponent->GetProperties()->GetComponentType() != {0}::kComponentPropertiesType) return NULL;\n", mComponentPropertiesClassName);
         AddTabs();
         mCB.AppendFormat("return static_cast<{0} *>(pComponent);\n", mComponentClassName);
         mTabCount--;
         AddTabs();
         mCB.AppendLine("}");
         mCB.AppendLine();
      }
      
      void BuildDuplicatePropertiesBody()
      {
         AddFunctionSeparator();
         AddTabs();
         mCB.AppendFormat("void {0}::DuplicateProperties()\n", mComponentClassName);
         AddTabs();
         mCB.AppendLine("{");
         mTabCount++;
         AddTabs();
         mCB.AppendFormat("inherited::DuplicateProperties();\n");
         AddTabs();
         mCB.AppendFormat("mpProperties = static_cast<{0} const *>(GetProperties().get());\n", mComponentPropertiesClassName);
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

