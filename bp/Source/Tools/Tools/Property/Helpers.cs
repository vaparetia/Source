#region Using directives

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml;
using System.Xml.XPath;
using System.Diagnostics;
using System.Windows.Forms;
using System.Drawing;
using Tools.Property.UI;
using Tools.Property.Source;
using Tools.Property.Build;
using Tools.Common;

#endregion

namespace Tools.Property.Misc
{     
   /// <summary>
   /// Catch-all class for functions that are used across multiple property classes.
   /// </summary>
   public class Helpers
   {
      public const string kElement_Using =                                             "Using";
      public const string kElement_UsingEvaluator =                                    "UsingEvaluator";
      public const string kElement_PropertyContainer =                                 "PropertyContainer";
      public const string kElement_ComponentGroup =                                    "ComponentGroup";
      public const string kElement_Component =                                         "Component";
      public const string kElement_PropertyObject =                                    "PropertyObject";
      public const string kElement_MessageObject  =                                    "MessageObject";
      public const string kElement_Layout =                                            "Layout";
      public const string kElement_Property =                                          "Property";
      public const string kElement_EvaluatorProperty =                                 "EvaluatorProperty";
      public const string kElement_EvaluatorContainerRoot =                            "EvaluatorContainerRoot";
      public const string kElement_EvaluatorContainerEntry =                           "EvaluatorContainerEntry";
      
      /// <summary>
      /// Used by 'EvaluatorsContainerEntry' to signify target component. 
      /// Can be used multiple times in an EvaluatorContainerEntry
      /// </summary>
      public const string kElement_TargetGuid =                                        "TargetGuid";
      public const string kElement_Value =                                             "Value";
      public const string kElement_Evaluator =                                         "Evaluator";
      
      /// <summary>
      /// Used for containing gui related settings. 
      ///Assumes list of elements below, with none of the elements having sub elements.
      /// </summary>
      public const string kElement_UISettings =                                        "UISettings";
      
      public const string kElement_Enum =                                              "Enum";
      public const string kElement_EnumEntry =                                         "EnumEntry";
      public const string kElement_ActiveComponent =                                   "Value";

      public const string kElement_DynamicItems =                                      "DynamicItems";
      public const string kDynamicItems_Type =                                         "type";
      public const string kDynamicItems_Type_Placeholder =                             "placeholder";
      public const string kDynamicItems_Type_FromPropertyValues =                      "fromPropertyValues";
      public const string kDynamicItems_Type_FromPropertyContainer =                   "fromUnmergedPropertyContainers";
      public const string kDynamicItems_Type_FromXML =                                 "fromXmlElements";
      public const string kDynamicItems_Source =                                       "src";
      public const string kDynamicItems_Query =                                        "query";
      public const string kDynamicItems_RecursiveSourceQuery =                         "recursiveSourceQuery";

      public const string kLayout_Type =                                               "type";
      public const string kLayout_Type_group =                                         "group";
      public const string kLayout_Type_tabgroup =                                      "tabGroup";
      public const string kLayout_Type_tab =                                           "tab";
      public const string kLayout_Type_groupBar =                                      "groupBar";

      public const string kProperty_Name =                                             "name";
      public const string kProperty_Id =                                               "id";
      
      /// <summary>
      /// Use template='other_object_id' to derive from other object.
      /// e.g. PropertyContainer id="NewTypeOfFoo" name="NewFoo" template="Foo"
      /// Override values only, no additional values allowed.
      /// </summary>
      public const string kProperty_Template =                                         "template";
      public const string kProperty_Type =                                             "type";
      public const string kProperty_Type_vector3 =                                     "vector3";
      public const string kProperty_Type_matrix34 =                                    "matrix34";
      public const string kProperty_Type_float =                                       "float";
      public const string kProperty_Type_enum =                                        "enum";
      public const string kProperty_Type_bool =                                        "bool";
      
      /// <summary>
      /// The bit field entries are used to to pack bools into a uint32.
      /// Use 'bitFieldName="MyCommonVarName" attrib for the common variable name, 
      /// followed by 'bitPosition="15"', max 31.
      /// Only use this on bool properties.
      /// </summary>
      public const string kProperty_Type_bool_bitfield_name =                          "bitFieldName";
      public const string kProperty_Type_bool_bitfield_position =                      "bitFieldPosition";
      public const string kProperty_Type_string =                                      "string";
      public const string kProperty_Type_int =                                         "int";
      public const string kProperty_Type_color =                                       "color";
      public const string kProperty_Type_guid =                                        "guid";
      public const string kProperty_Type_evaluatorContainer =                          "EvaluatorContainer";
      public const string kProperty_Type_custom =                                      "custom";
      public const string kProperty_Min =                                              "min";
      public const string kProperty_Max =                                              "max";
      public const string kProperty_Tooltip =                                          "tooltip";
      
      /// <summary> When present and true, disables export in generated code and cooker. 
      /// Works properties, property objects, and components. 
      /// </summary>
      public const string kProperty_NoExport =                                         "noExport";
      
      /// <summary>
      /// When true, property editor indicates to the user that they
      /// need to re-export/restart to apply game side changes to this property.
      /// Helps prevent user confusion when using live property update.
      /// </summary>
      public const string kProperty_NeedsExportForUpdate =                             "needsExportForUpdate";
      
      /// <summary>
      /// When used by a property or property object, the cook and code gen will 
      /// treat the cookas value as the type or id.
      /// Currently used to apply different labels for translation, rotation and scale
      /// in editor properties but still allowing the different typedefs to be
      /// cooked as vectors.
      /// </summary>
      public const string kProperty_CookAs =                                           "cookAs";
      
      /// <summary>
      /// displayName is an override for "name" and will be used to 
      /// label the properties if it exists
      /// </summary>
      public const string kProperty_DisplayName =                                      "displayName";
      
      /// <summary>
      /// Terse display name used in message editor.
      /// </summary>
      public const string kProperty_ShortDisplayName =                                 "shortDisplayName";
      
      /// <summary>
      /// When true, property isn't visible in property editor.
      /// Used to hide internal properties.
      /// </summary>
      public const string kProperty_IsVisible =                                        "isVisible";
      
      /// <summary>
      /// "subType" controls how certain properties are represented in the ui as 
      /// well as controls how they get cooked. 
      /// This allows for having float attributes show up as a text box or as a slider, 
      /// or some other kind of control. 
      /// Other examples are "rgb" or "rgba" for the color property.
      /// </summary>
      public const string kProperty_SubType =                                          "subType";
      /// string subtypes
      public const string kProperty_SubType_asset =                                    "asset";
      public const string kProperty_SubType_hashedString =                             "hashedString";
      public const string kProperty_SubType_messageStringHash =                        "messageStringHash";
      public const string kProperty_SubType_animationEventStringHash =                 "animationEventStringHash";
      /// custom subtypes
      public const string kProperty_SubType_spline =                                   "CSpline";
      /// Color subtypes (a - alpha, i - intensity)
      public const string kProperty_SubType_color_rgb =                                "rgb";
      public const string kProperty_SubType_color_rgba =                               "rgba";
      public const string kProperty_SubType_color_rgbi =                               "rgbi";
      public const string kProperty_SubType_color_rgbai =                              "rgbai";

      /// <summary>
      /// This attribute can exist on a property of type "string" with a subType of 
      /// type "asset"
      /// It's a semicolon separated list of fourCC's for the asset types, 
      /// or just a single fourCC in case you only have one case.
      /// </summary>
      public const string kProperty_AssetTypes =                                       "assetTypes";

      /// <summary>
      /// This attribute can exist on a property of type "string" with a subType of 
      /// type "asset"
      /// It's a hint for cookers to describe the way this asset is used.
      /// </summary>
      public const string kProperty_AssetSubType = "assetSubType";

      /// <summary>
      /// This attribute can exist on a property of type "string" with a subType of 
      /// type "asset"
      /// It's a bool that is true is the asset was a generated asset, rather than an 
      /// asset from the repository
      /// </summary>
      public const string kProperty_IsGeneratedAsset = "isGeneratedAsset";

      /// <summary>
      /// This attribute can exist on a property of type "string" with a subType of 
      /// type "asset"
      /// Used to tell code gen if the asset should be locked on property load.
      /// If it doesn't exist, it is assumed to be true.
      /// </summary>
      public const string kProperty_LockAssetOnLoad =                                  "lockAssetOnLoad";

      /// <summary>
      /// This attribute specifies a link to a label. 
      /// This will enforce all other properties that have the same label be set to 
      /// the same value.
      /// This can be used to have common properties between components that 
      /// all share the same value.
      /// </summary>
      public const string kProperty_Link =                                             "link";
      
      /// <summary>
      /// When true, property uses the name of it's parent,
      /// normally used for enums in property objects.
      /// </summary>
      public const string kProperty_UseParentName =                                    "useParentName";
      
      /// <summary>
      /// Prepends the contents to the properties class declaration.
      /// e.g. class FOOBAR_API CPOLdrFoo.
      /// Use this for dll exports/imports.
      /// </summary>
      public const string kPropertyObject_exportDeclaration =                          "exportDeclaration";
      
      public const string kEvaluator_Id =                                              "id";
      public const string kEvaluator_Type =                                            "type";
      public const string kEvaluator_Id_Default =                                      "default";
      
      /// <summary>
      /// Property that is dynamically bound to an evaluator at run time.
      /// </summary>
      public const string kEvaluatorProperty =                                         "EvaluatorProperty";
      public const string kEvaluatorProperty_Name =                                    "name";
      public const string kEvaluatorProperty_AllowEmptyEvaluator =                     "allowEmptyEvaluator";
      public const string kEvaluatorProperty_ActiveEvaluator =                         "activeEvaluator";
      
      /// <summary>
      /// This prevents the code generation from adding initialization code for this 
      /// evaluator property inside 'InitializeEvaluators' function. 
      /// This does not affect the evaluator's use of CEvaluatorAllocator.
      /// This is currently used in particle code so we can explicitly order the 
      /// initialization of certain complex evaluators and 
      /// not have them initialized twice.
      /// </summary>
      public const string kEvaluatorProperty_DisableCodeGenEvaluatorInitialization =   "disableCodeGenEvaluatorInitialization";
           
      /// <summary>
      /// Type of evaluator.
      /// </summary>
      public const string kEvaluatorProperty_Type =                                    "type";
      public const string kEvaluatorProperty_Type_float =                              "float";
      public const string kEvaluatorProperty_Type_int =                                "int";
      public const string kEvaluatorProperty_Type_bool =                               "bool";
      public const string kEvaluatorProperty_Type_vector3 =                            "vector3";
      public const string kEvaluatorProperty_Type_color =                              "color";
      public const string kEvaluatorProperty_Type_userData =                           "userData";
      /// <summary>
      /// Used to modify incoming position and velocity, generally from particles.
      /// </summary>
      public const string kEvaluatorProperty_Type_modifierPV =                         "modifierPV";
      public const string kEvaluatorProperty_Type_emitterPV =                          "emitterPV";
      
      /// <summary>
      /// Attribute in the form PX_*_* ... where each * is a property name. 
      /// Names are hashed and used to send evaluated values to properties.
      /// </summary>
      public const string kEvaluatorContainerEntry_targetProperty =                    "targetProperty";
      
      /// <summary>
      /// Attached to an evaluator declaration. 
      /// It shows which in library it is defined (Engine, Particle, Shader).
      /// Engine scoped objects can be used by anything. 
      /// </summary>
      public const string kEvaluator_Scope =                                           "scope";

      /// <summary>
      /// Prepends the contents to the evaluator class declaration.
      /// e.g. class FOOBAR_API CCPLdrFoo.
      /// Use this for dll exports/imports.
      /// </summary>
      public const string kEvaluator_ExportDeclaration = "exportDeclaration";

      public const string kComponent_Name =                                            "name";
      public const string kComponent_Id =                                              "id";
      public const string kComponent_Group =                                           "group";
      public const string kComponent_Tooltip =                                         "tooltip";

      /// <summary>
      /// A list of platforms supported by this component for CodeGen purposes, delimited by ';'
      /// If there is no "platforms" list provided, it is assumed to be all platforms
      /// </summary>
      public const string kComponent_Platforms = "platforms";

      /// <summary>
      /// If this is attribute is false, code for auto registering factories won't 
      /// be added.
      /// </summary>
      public const string kComponent_registerFactory =                                 "registerFactory";
      
      /// <summary>
      /// Used for creating editor id attributes. 
      /// Attribute is bool type, assumed 'true' if not present on component element.
      /// </summary>
      public const string kComponent_CreateEditorId =                                  "createEditorId";
      
      /// <summary>
      /// Prepends the contents to the component class declaration.
      /// e.g. class FOOBAR_API CComponentFoo.
      /// Use this for dll exports/imports.
      /// It will also be assigned to the component group class.
      /// </summary>
      public const string kComponent_classExportDeclaration =                          "classExportDeclaration";
      
      /// <summary>
      /// Prepends the contents to the component properties class declaration.
      /// e.g. class FOOBAR_API CCPLdrFoo.
      /// Use this for dll exports/imports.
      /// It will also be assigned to the component group properties class.
      /// </summary>
      public const string kComponent_classPropertiesExportDeclaration =                "classPropertiesExportDeclaration";

      /// <summary>
      /// Used to override methods to be used from kComponentGroup_GameObjectType.
      /// e.g. If gameObjectType="GamePlayer" then useMethods="Game" will
      /// allow the use of "Game" methods instead of "GamePlayer" methods.
      /// </summary>
      public const string kComponent_UseMethods =                                      "useMethods";
      
      /// <summary>
      /// If this attribute is true on UsingComponentGroup inside components, 
      /// then sub components will use the parent component's evaluator allocator.
      /// </summary>
      public const string kComponentGroup_UseParentEvaluatorAllocator =                "useParentEvaluatorAllocator";
      public const string kComponentGroup_Id =                                         "id";
      public const string kComponentGroup_Name =                                       "name";
      public const string kComponentGroup_AllowEmptyComponent =                        "allowEmptyComponent";
      public const string kComponentGroup_DefaultComponent =                           "default";
      public const string kComponentGroup_GameObjectType =                             "gameObjectType";
      /// <summary>
      /// Overrides the component code-gen evaluator type, 
      /// specified by default from kComponentGroup_GameObjectType.
      /// </summary>
      public const string kComponentGroup_EvaluatorUpdateType =                        "evaluatorUpdateType";
      
      /// <summary>
      /// Has this component group been added dynamically,
      /// e.g. dynamicComponentGroup="true".
      /// Dynamic component groups also unmerge differently to regular component groups.
      ///</summary>
      public const string kComponentGroup_Dynamic =                                    "dynamicComponentGroup";
      
      /// <summary>
      /// Used to prevent components that include other components recursing infinitely 
      /// when building xml properties in BuildDerivedPropertyTree.
      /// If not present then a default recursion amount is specified.
      /// </summary>
      public const string kComponentGroup_MaxSubComponents =                           "maxSubComponents";

      /// <summary>
      /// Used for creating editor id attribute. 
      /// Attribute is bool type, assumed 'true' if not present on script object element.
      /// </summary>
      public const string kPropertyContainer_CreateEditorId =                          "createEditorId";
      public const string kPropertyContainer_Name =                                    "name";

      public const string kProperty_EnumName =                                         "enumName";
      public const string kEnumEntry_Name =                                            "name";
      public const string kEnumEntry_Description =                                     "desc";
      public const string kEnumEntry_Namespace =                                       "namespace";
      
      /// <summary>
      /// When true, automatically generate the enums for you. 
      /// Doesn't make sense to use this with 'namespace'.
      /// </summary>
      public const string kEnumEntry_GenerateCode =                                    "generateCode";

      ///<summary>Used a tag to find and modify property values. </summary>
      public const string kProperty_Label =                                            "label";  
      
      /// <summary>
      /// Used by properties cooker to determine if an object should be created on load. 
      /// Attached to a bool property.
      /// </summary>
      public const string kLabel_create_on_properties_load =                           "create_on_properties_load";
      public const string kProperty_Value_Default =                                    "default";
      public const string kProperty_Value_Grouped =                                    "grouped";
      public const string kProperty_Value_Modified =                                   "modified";
      public const string kProperty_Value_KeepInitialValue =                           "keepInitialValue";  // Prevents this value getting unmerged, useful for locking in component default values

      ///<summary>
      /// Local id. Used within a property object to reference local properties.
      /// </summary>
      public const string kUsing_Attribute_Id =                                        "id";
      
      ///<summary>Finds all Value or Minimized elements</summary>
      public const string kXPath_ValueSearch =                                         ".//*[self::" + kElement_UISettings + " or self::" + kElement_Value +"]";
      ///<summary>
      /// Finds all Value or Minimized below Using or UsingComponentGroup (special case!).
      /// Only used for merging in overriden values.
      ///</summary>
      public const string kXPath_UsingValueSearch =                                    "//Using/*[self::" + kElement_UISettings + " or self::" + kElement_Value +"] | //UsingComponentGroup/*[self::" + kElement_UISettings + " or self::" + kElement_Value +"]";

      public const string kEmptyComponentName =                                        "None";
      public const string kEmptyEvaluatorName =                                        "None";

      public const string kElement_Events =                                            "Events";
      public const string kElement_Event =                                             "Event";
      public const string kEvent_type =                                                "type";

      public const string kEvent_cookAs =                                              "cookAs";
      public const string kEventEntry =                                                "EventEntry";
      public const string kEventEntry_id =                                             "id";
      public const string kEventEntry_active =                                         "active";
      public const string kEventEntry_time =                                           "time";
      public const string kEventEntry_target =                                         "target";
      public const string kEventEntry_priority =                                       "priority";
      public const string kEventEntry_identifier =                                     "identifier";
      public const string kEventEntry_senderQuery =                                    "senderQuery";
      public const string kEventEntry_destQuery =                                      "query";

      public const string kElement_Links =                                             "Links";
      public const string kElement_Link =                                              "Link";
      /// <summary>
      /// 'Allow''/'Deny' syntax based off component names.
      /// If no 'Allow'/'Deny' present the all components allowed.
      /// If 'Allow' present then all objects denied by default.
      /// If 'Deny' present then all objects allowed by default.
      /// 'Deny' overrides 'Allow'.
      /// Use 'ComponentGroup':'ComponentName' for syntax.
      /// We can also filter on just the componentgroup.
      /// </summary>

      // Used like this:-
      // <Link name="foo">
      //    <Allow>Camera:Debug</Allow>
      //    <Allow>BaseGameProperties</Allow>
      // </Link>
      //
      // Allow all RBP except NoCollision.
      // <Link name="bar">
      //    <Allow>RigidBodyPhysics</Allow>
      //    <Deny>RigidBodyPhysics:NoCollision</Deny> 
      // </Link>
      //
      
      public const string kElement_Link_Allow =                                        "Allow";
      public const string kElement_Link_Deny =                                         "Deny";
      public const string kLink_type =                                                 "type";
      
      /// <summary>
      /// Links have a message object to specify properties associated with the link.
      /// The difference between messages and links is that the properties 
      /// on links are specfied by the source object not the destination.
      /// An example of useful link properties would be probability weights 
      /// for random object generation.
      /// </summary>
    public const string kLink_message =                                              "message";
      
      /// <summary>
      /// No message type associated with link.
      /// Used internally to prevent messageType="" empty attribute.
      ///</summary>
      public const string kLink_message_emptyMessage =                                 "emptyMessage";
      public const string kLink_cookAs =                                               "cookAs";
      public const string kLinkEntry =                                                 "LinkEntry";
      public const string kLinkEntry_active =                                          "active";
      public const string kLinkEntry_id =                                              "id";
      public const string kLinkEntry_target =                                          "target";
      public const string kLinkEntry_time =                                            "time";
      public const string kLinkEntry_priority =                                        "priority";
      public const string kLinkEntry_identifier =                                      "identifier";
      public const string kLinkEntry_senderQuery =                                     "senderQuery";
      public const string kLinkEntry_destQuery =                                       "query";

      public const string kElement_Messages =                                          "Messages";
      public const string kElement_Message =                                           "Message";
      public const string kMessage_id =                                                "id";
      public const string kMessage_type =                                              "type";
      
      /// <summary>
      /// If this is attribute is false, code for auto registering factories 
      /// won't be added.
      /// </summary>
      public const string kMessage_registerFactory =                                   "registerFactory";

      /// <summary>
      /// Used by message declaration to explicitly declare the type instead 
      /// of using a name hash.
      /// </summary>
      public const string kMessage_cookAs =                                            "cookAs";
      
      /// <summary>
      /// Prepends the contents to the message properties class declaration.
      /// e.g. class FOOBAR_API CMOLdrFoo.
      /// Use this for dll exports/imports.
      /// </summary>
      public const string kMessage_exportDeclaration =                                 "exportDeclaration";

      public const string kElement_Methods =                                           "Methods";
      public const string kElement_Method =                                            "Method";
      public const string kMethod_type =                                               "type";
      
      /// <summary>
      /// A named method can be associated with multiple function calls.
      /// Use a ';' delimited list of the Method.mName to exclude those functions.
      /// e.g. Method type="Render" ignoreFunctions="GlobalPrerender;ViewportPrerender"
      /// All functions are included by default.
      /// </summary>
      public const string kMethod_ignoreFunctions =                                    "ignoreFunctions";

      /// <summary>
      /// The helpCategory tag can be attached to a Component or Evaluator.
      /// This classifies the object to the help generator.
      /// In the AutoGenerate section of the help file you can specify which category you want to generate help for to filter them.
      /// </summary>
      public const string kAny_helpCategory =                                          "helpCategory";
      public const string kAny_helpCategory_Default =                                  "Scripting";

//----------------------------------------------------------------------------
      /// <summary>
      /// This class is used for selections that cannot achieved via xpath queries alone.
      /// This is mainly for the 'Layout' issue where we can have multiple layouts between properties.
      /// </summary>
      public class XPathMultiSelect : IEnumerable<XPathNavigator>
      {
         public XPathMultiSelect(XPathNavigator xmlNav, string xpathQuery)
         {
            foreach(XPathNavigator nav in xmlNav.Select(xpathQuery))
            {
               mSelect.Add(nav.Clone());
            }
         }
         
         public XPathMultiSelect(XPathMultiSelect multiSelect, string xpathQuery)
         {
            foreach(XPathNavigator msNav in multiSelect)
            {
               foreach(XPathNavigator nav in msNav.Select(xpathQuery))
               {
                  mSelect.Add(nav.Clone());
               }
            }
         }
         
         static public XPathMultiSelect LayoutSelect(XPathNavigator xmlNav, string xpathQuery)
         {
            XPathMultiSelect firstSet =  new XPathMultiSelect(xmlNav, "./Layout | ./Layout/Layout");
            // Also add self
            firstSet.mSelect.Add(xmlNav);
            return new XPathMultiSelect(firstSet, xpathQuery);
         }
         
         /// <summary>
         /// Skips first parent when checking for layouts.
         /// </summary>
         /// <param name="xmlNav"></param>
         /// <param name="xpathQuery"></param>
         /// <returns></returns>
         static public XPathMultiSelect LayoutSelectSkipParent(XPathNavigator xmlNav, string xpathQuery)
         {
            XPathNavigator xmlNavCopy = xmlNav.Clone();
            if (!xmlNavCopy.MoveToFirstChild())
            {
               return null;
            }
            return LayoutSelect(xmlNavCopy, xpathQuery);
         }
         
         static public XPathMultiSelect MultiSelect(XPathNavigator xmlNav, string firstXPathQuery, string secondXPathQuery)
         {
            XPathMultiSelect firstSet =  new XPathMultiSelect(xmlNav, firstXPathQuery);
            return new XPathMultiSelect(firstSet, secondXPathQuery);
         }
         
         public IEnumerator<XPathNavigator> GetEnumerator()
         {
            foreach(XPathNavigator nav in mSelect)
            {
               yield return nav;
            }
         }
         
         System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() { return GetEnumerator(); }
         
         public List<XPathNavigator> mSelect = new List<XPathNavigator>();
      }
//----------------------------------------------------------------------------

      /// <summary>
      /// Builds a new document from a navigator.
      /// </summary>
      static public XmlDocument NavigatorToDocument(XPathNavigator xmlNav)
      {
         XmlDocument newDoc = new XmlDocument();
         newDoc.LoadXml(xmlNav.OuterXml);
         return newDoc;
      }

      /// <summary>
      /// Checks to see if a property has a <value/>, then sets it to the new value.
      /// Returns the previous value.
      /// You can use the xmlNav from <see cref="Properties.Helpers.GetXmlNavForLabel"/>.
      /// If the property specified by xmlNav contains a "link" attribute, it will automatically update 
      /// all properties associated with this link.
      /// </summary>
      static public string SetPropertyValueElement(XPathNavigator xmlNav, string value)
      {
         string previousValue = SetPropertyValueElementInternal(xmlNav, value);

         string linkName = xmlNav.GetAttribute(Helpers.kProperty_Link, "");
         if( linkName != string.Empty )
         {
            // start searching for other properties with that label at the root of the current node
            XPathNavigator root = xmlNav.Clone();
            root.MoveToRoot();

            XPathNodeIterator foundLabels = root.Select(string.Format(".//Property[@label='{0}']", linkName));
            foreach( XPathNavigator currentNav in foundLabels )
            {
               if( currentNav != xmlNav )
               {
                  SetPropertyValueElementInternal(currentNav, value);
               }
            }
         }

         return previousValue;
      }
      static public string SetPropertyValueElement( XmlElement xml, string value )
      {
         return SetPropertyValueElement( xml.CreateNavigator(), value );
      }
      /// <summary>
      /// this function does the actual work of either setting the existing value child element to the value or creating it with the given value
      /// </summary>
      /// <param name="xmlNav"></param>
      /// <param name="value"></param>
      /// <returns></returns>
      static private string SetPropertyValueElementInternal(XPathNavigator xmlNav, string value)
      {
         string previousValue = string.Empty;

         if( xmlNav.MoveToChild(kElement_Value, "") )
         {
            previousValue = xmlNav.Value;
            xmlNav.SetValue(value);
            xmlNav.MoveToParent();
         }
         else
         {
            xmlNav.AppendChildElement("", Helpers.kElement_Value, "", value);
         }
         
         return previousValue;
      }

      /// <summary>
      /// Checks to see if a property has a 'Value' element and returns it.
      /// You can use the xmlNav from <see cref="Properties.Helpers.GetXmlNavForLabel"/>.
      /// </summary>
      static public string GetPropertyValueElement(XPathNavigator xmlNav)
      {
         string value = null;
         if (xmlNav.MoveToChild(kElement_Value, ""))
         {
            XmlDocument temp = new XmlDocument();
            temp.LoadXml("<decode/>");
            temp.DocumentElement.InnerXml = xmlNav.InnerXml;
            value = temp.DocumentElement.InnerText;
            xmlNav.MoveToParent();
         }
         return value;
      }
      static public string GetPropertyValueElement( XmlElement element )
      {
         XmlElement valueElement = (XmlElement) element.SelectSingleNode( kElement_Value );
         if ( valueElement != null )
         {
            return valueElement.InnerText;
         }
         else
         {
            return null;
         }
      }
      /// <summary>
      /// Checks to see if a property has a 'Value' element and returns it.
      /// Modifies the position of the reader to skip over children of current
      /// property element.
      /// </summary>
      static public void GetPropertyValueElement(XmlReader xmlReader, out string xmlValue, out string textValue)
      {
         xmlValue = null;
         textValue = null;

         int currentDepth = xmlReader.Depth;
         if (xmlReader.ReadToDescendant(kElement_Value))
         {
            xmlValue = xmlReader.ReadInnerXml();
            
            XmlDocument temp = new XmlDocument();
            temp.LoadXml("<decode/>");
            temp.DocumentElement.InnerXml = xmlValue;
            textValue = temp.DocumentElement.InnerText;

            // Skip over remaining children
            while (xmlReader.Depth > currentDepth)
            {
               xmlReader.Read();
            }
         }
         else
         {
            xmlReader.Skip();
         }
      }

      /// <summary>
      /// Reads the values of the property object into one string, space delimited.
      /// Assumes xmlReader is positioned at the property object above the sub properties.
      /// This will have skipped over the property object when completed.
      /// </summary>
      /// <param name="xmlReader"></param>
      /// <returns></returns>
      static public string GetPropertyValueElementsSpaceDelimited(XmlReader xmlReader, int maxElements )
      {
         int currentDepth = xmlReader.Depth;
         string value = "";

         if (maxElements < 1)
         {
            throw new System.InvalidOperationException("GetPropertyValueElementsSpaceDelimited needs >= 1 maxElements");
         }

         if (xmlReader.ReadToDescendant(kElement_Value))
         {
            value = xmlReader.ReadElementString();

            for (int i = 1; i < maxElements; ++i)
            {
               if (!xmlReader.ReadToFollowing(kElement_Property))
               {
                  break;
               }

               if (xmlReader.ReadToDescendant(kElement_Value))
               {
                  value += " " + xmlReader.ReadElementString();
               }
               else
               {
                  break;
               }
            }
         }

         // Skip everything else
         while (xmlReader.Depth > currentDepth)
         {
            xmlReader.Read();
         }

         return value;
      }

      /// <summary>
      /// Returns a navigator on 'Property/EvaluatorProperty' element for a name path.
      /// Name paths are of the form Name.Name.PropertyName, last entry must be a property/evaluator property/property object.
      /// Each match is based off the element 'name' attribute.
      /// E.g. EditorProperties.Translation.X is a valid name path. 
      /// Named elements above the first match are considered wildcards, use more specific paths for tighter matches.
      /// E.g. RenderActor_C.EditorProperties.Translation.X will match the script object name also.
      /// </summary>
      /// <param name="xmlNav">Xml to search for match.</param>
      /// <param name="namePath">'.' delimited name path.</param>
      /// <returns>
      /// It returns the first match if there are multiple matching entries, null if no matching entries.
      ///</returns>
      static public XPathNavigator GetPropertyNavForPath(XPathNavigator xmlNav, string namePath)
      {
         string[] names = namePath.Split('.');
         string xpath = "";
         foreach(string name in names)
         {
            xpath += String.Format("//*[@name=\"{0}\"]", name);
         }
         
         string comparisonXPath = xpath.Replace("//", "/");
         
         // Find all entries
         XPathNodeIterator navIter = xmlNav.Select(xpath);
         foreach(XPathNavigator nav in navIter)
         {
            string uniquePath = BuildUniquePathForValue(nav, xmlNav);
            if (uniquePath.EndsWith(comparisonXPath))
            {
               // Paths match, we're done
               return nav;
            }
         }
         return null;   // No match
      }
      
      /// <summary>
      /// Creates an string that uniquely identifies a value by name.
      /// Note that as we ignore 'Layout' elements.
      /// Also note that this looks like an xpath, and will work as an xpath, it will not match paths where the layout tags exist.
      /// Generates an string that looks like this '/*[@name="MyPropertyObject"]/*[@name="Property"]/Value'.
      /// Can include script objects, components, property objects etc.
      /// </summary>
      /// <param name="valueNav">
      /// Id string path is built from this.
      /// </param> 
      /// <param name="topLevelTerminationNav">
      /// Points to element above value. XPath generation is terminated when positions match.
      /// Must be generated from same document.
      /// Can be null, causing xpath to reach top of document.
      /// </param> 
      /// <returns>
      /// A string id that uniquely identifies a value based off the name attributes.
      /// </returns>
      public static string BuildUniquePathForValue(XPathNavigator valueNav, XPathNavigator topLevelTerminationNav)
      {
         // Copy so we don't modify incoming
         XPathNavigator srcNav = valueNav.CreateNavigator();
         StringBuilder path = new StringBuilder();
         bool bNameFirstEntry = false;
            
         switch (valueNav.Name)
         {
            case Helpers.kElement_TargetGuid:
               path.AppendFormat("/{0}/{1}", valueNav.Name, valueNav.Value);  // Add element and value
               break;
               
            case Helpers.kElement_EvaluatorContainerEntry:
               // Insert element name so we know to ignore it later in merge
              path.AppendFormat("/EvaluatorContainerEntry[@targetProperty=\"{0}\"]", srcNav.GetAttribute(Helpers.kEvaluatorContainerEntry_targetProperty, String.Empty));
              break;
              
            case Helpers.kElement_EvaluatorProperty:
            case Helpers.kElement_Property:
            case Helpers.kElement_PropertyObject:
               // We want to use the name of this object, so we'll disable the first 'MoveToParent'
               bNameFirstEntry = true;
               break;
              
            default:
               path.AppendFormat("/{0}", valueNav.Name);  // Add /Value (or whatever, Minimized) element
               break;
         }
            
         while (bNameFirstEntry || srcNav.MoveToParent())
         {
            bNameFirstEntry = false;
            // Ignore if no name
            if( !ShouldIgnoreElementForUniquePath( srcNav ) )
            {
               // Setup query for name attribute. Don't forget, we're building this string backwards.
               path.Insert(0, String.Format("/*[@name=\"{0}\"]", srcNav.GetAttribute(Helpers.kProperty_Name, String.Empty)));
            }
            else
            {
               if (srcNav.Name == Helpers.kElement_EvaluatorContainerEntry)
               {
                  // Insert element name so we know to ignore it later in merge
                  path.Insert(0, String.Format("/EvaluatorContainerEntry[@targetProperty=\"{0}\"]", srcNav.GetAttribute(Helpers.kEvaluatorContainerEntry_targetProperty, String.Empty)));
               }
               else if ((srcNav.Name == Helpers.kElement_Evaluator) || (srcNav.Name == Helpers.kElement_UsingEvaluator))
               {
                  // Add evaluator id for match in merge
                  path.Insert(0, String.Format("/*[@id=\"{0}\"]", srcNav.GetAttribute(Helpers.kEvaluator_Id, String.Empty)));
               }
            }
            
            // Quit if we're at the top level of the src navigator.
            // We don't want to go all the way up to the root as the dest docs xpath may be different.
            if ((topLevelTerminationNav != null) && topLevelTerminationNav.IsSamePosition(srcNav))
            {                 
               break;
            }
         }
                 
         return path.ToString();
      }

      /// <summary>
      /// This function will return true if the element pointed to by nav should be skipped while building a unique path, 
      /// this allows skipping nodes without names and layout nodes
      /// </summary>
      /// <param name="nav"></param>
      /// <returns>true if element should be skipped</returns>
      private static bool ShouldIgnoreElementForUniquePath(XPathNavigator nav)
      {
         // ignore layout elements
         if( nav.Name == Helpers.kElement_Layout )
            return true;

         // ignore if name is empty
         return nav.GetAttribute(Helpers.kProperty_Name, String.Empty).Length == 0;
      }

      /// <summary>
      /// Checks to see if a (evaluator) property has a 'grouped' attribute and returns it.
      /// You can use the xmlNav from <see cref="Properties.Helpers.GetXmlNavForLabel"/>.
      /// </summary>
      static public bool GetPropertyGrouped(XPathNavigator xmlNav)
      {
         string childElement = String.Empty;
         switch (xmlNav.Name)
         {
            case Helpers.kElement_EvaluatorProperty:
               childElement = Helpers.kElement_Evaluator;
               break;
            default:
               childElement = Helpers.kElement_Value;
               break;
         }
         
         if (xmlNav.MoveToChild(childElement, String.Empty))
         {
            string grouped = xmlNav.GetAttribute(kProperty_Value_Grouped, String.Empty);
            xmlNav.MoveToParent();
            if (grouped.Length == 0) return false;
            return Tools.Common.Misc.ParseBool(grouped);
         }
         return false;
      }
      static public bool GetPropertyGrouped( XmlElement xmlElement )
      {
         return GetPropertyGrouped( xmlElement.CreateNavigator() );
      }
      /// <summary>
      /// Sets the 'grouped' attribute for a (evaluator) property.
      /// You can use the xmlNav from <see cref="Properties.Helpers.GetXmlNavForLabel"/>.
      /// </summary>
      static public void SetPropertyGrouped(XPathNavigator xmlNav, bool bGrouped)
      {
         string childElement = String.Empty;
         switch (xmlNav.Name)
         {
            case Helpers.kElement_EvaluatorProperty:
               childElement = Helpers.kElement_Evaluator;
               break;
            default:
               childElement = Helpers.kElement_Value;
               break;
         }
         
         if (xmlNav.MoveToChild(childElement, String.Empty))
         {
            string grouped = xmlNav.GetAttribute(kProperty_Value_Grouped, String.Empty);
            if (grouped.Length == 0)
            {
               if (bGrouped)
               {
                  xmlNav.CreateAttribute(String.Empty, Helpers.kProperty_Value_Grouped, String.Empty, bGrouped.ToString());
               }
               xmlNav.MoveToParent();            
               return;
            }
            xmlNav.MoveToAttribute(Helpers.kProperty_Value_Grouped, String.Empty);
            xmlNav.SetValue(bGrouped.ToString());
            xmlNav.MoveToParent();            
            xmlNav.MoveToParent();            
         }
         return;
      }
      static public void SetPropertyGrouped( XmlElement xmlNav, bool bGrouped )
      {
         SetPropertyGrouped( xmlNav.CreateNavigator(), bGrouped );
      }
      /// <summary>
      /// Checks to see if a (evaluator) property has a 'modified' attribute and returns it.
      /// You can use the xmlNav from <see cref="Properties.Helpers.GetXmlNavForLabel"/>.
      /// </summary>
      static public bool GetPropertyModified(XPathNavigator xmlNav)
      {
         string childElement = String.Empty;
         switch (xmlNav.Name)
         {
            case Helpers.kElement_EvaluatorProperty:
               childElement = Helpers.kElement_Evaluator;
               break;
            default:
               childElement = Helpers.kElement_Value;
               break;
         }
         
         if (xmlNav.MoveToChild(childElement, String.Empty))
         {
            string modified = xmlNav.GetAttribute(kProperty_Value_Modified, String.Empty);
            xmlNav.MoveToParent();
            if (modified.Length == 0) return false;
            return Tools.Common.Misc.ParseBool(modified);
         }
         return false;
      }
      static public bool GetPropertyModified( XmlElement xmlElement )
      {
         return GetPropertyModified( xmlElement.CreateNavigator() );
      }

      /// <summary>
      /// Sets the 'modified' attribute for a (evaluator) property.
      /// You can use the xmlNav from <see cref="Properties.Helpers.GetXmlNavForLabel"/>.
      /// </summary>
      static public void SetPropertyModified(XPathNavigator xmlNav, bool bModified)
      {
         XPathNavigator valueNav = xmlNav.Clone();
         string childElement = String.Empty;
         switch (xmlNav.Name)
         {
            case Helpers.kElement_EvaluatorContainerEntry:
               // Evaluator container entries contain lots of evaluators below a Property.
               // We'll just mark the Property/Value as changed and then everything below it will be marked as changed.
               valueNav.MoveToParent();   // EvaluatorContainer
               valueNav.MoveToParent();   // Value
               break;
            case Helpers.kElement_EvaluatorProperty:
               childElement = Helpers.kElement_Evaluator;
               break;
            default:
               childElement = Helpers.kElement_Value;
               break;
         }
         
         // if empty string, already set valueNav
         if ((childElement == String.Empty) || valueNav.MoveToChild(childElement, String.Empty))
         {
            string modified = valueNav.GetAttribute(kProperty_Value_Modified, String.Empty);
            if (modified.Length == 0)
            {
               if (bModified)
               {
                  valueNav.CreateAttribute(String.Empty, Helpers.kProperty_Value_Modified, String.Empty, bModified.ToString());
               }
               return;
            }
            valueNav.MoveToAttribute(Helpers.kProperty_Value_Modified, String.Empty);
            valueNav.SetValue(bModified.ToString());
         }
         else
         {
            Debug.Fail("Can't set modified attribute for " + xmlNav.Name);
         }
         return;
      }
      static public void SetPropertyModified( XmlElement xml, bool bModified )
      {
         SetPropertyModified( xml.CreateNavigator(), bModified );
      }
      /// <summary>
      /// Returns the vector value for a string.
      /// </summary>
      /// <returns>
      /// Returns false if it couldn't parse the string.
      /// </returns>

      static public bool GetFloatValues(string valueString, int numComponents, out float[] outValues)
      {
         outValues = new float[numComponents];

         try
         {
            string[] numbers = valueString.Split(' ');
            if (numbers.Length != numComponents)
            {
               return false;
            }

            for (int i = 0; i < numComponents; ++i)
            {
               outValues[i] = float.Parse(numbers[i]);
            }

            return true;
         }
         catch (System.Exception)
         {
            return false;
         }
      }

      static public bool GetValue_vector3f(string value, out float x, out float y, out float z)
      {
         x = 0;
         y = 0;
         z = 0;
         try
         {
            string[] numbers = value.Split(new char[1]{' '});
            if (numbers.Length != 3)
            {
               return false;
            }
            x = float.Parse(numbers[0]);            
            y = float.Parse(numbers[1]);            
            z = float.Parse(numbers[2]);            
         }
         catch (Exception)
         {
            return false;
         }

         return true;
      }

      /// <summary>
      /// Gets tooltip string for a property.      
      /// You can use the xmlNav from <see cref="Properties.Helpers.GetXmlNavForLabel"/>.
      /// </summary>
      /// <returns>
      /// Nulled string if no tooltip.
      /// </returns>
      static public string GetToolTipText(XPathNavigator xmlNav)
      {
         string toolTipText = null;
         string minText = "";
         string maxText = "";
         if (xmlNav.HasAttributes)
         {
            xmlNav.MoveToFirstAttribute();
            do
            {
               switch (xmlNav.Name)
               {
                  case kProperty_Min:
                     minText = "Min value:" + xmlNav.Value;
                     break;
                     
                  case kProperty_Max:
                     maxText = "Max value:" + xmlNav.Value;
                     break;
                     
                  case kProperty_Tooltip:
                     toolTipText = xmlNav.Value;
                     break;
               }
            } while (xmlNav.MoveToNextAttribute());
            // Back to element
            xmlNav.MoveToParent();
         }

         if ((toolTipText != null) && (minText.Length != 0))
         {
            toolTipText += " ";
         }
         toolTipText += minText;
         if ((toolTipText != null) && (maxText.Length != 0))
         {
            toolTipText += " ";
         }
         toolTipText += maxText;

         return toolTipText;
      }
      static public string GetToolTipText( XmlElement xml )
      {
         return GetToolTipText( xml.CreateNavigator() );
      }
      /// <summary>
      /// Gets label string for a property.      
      /// You can use the xmlNav from <see cref="Properties.Helpers.GetXmlNavForLabel"/>.
      /// </summary>
      /// <returns>
      /// Nulled string if no label.
      /// </returns>
      static public string GetLabel(XPathNavigator xmlNav)
      {
         return xmlNav.GetAttribute( kProperty_Label, "" );
      }
      static public string GetLabel( XmlElement xmlNav )
      {
         return xmlNav.GetAttribute( kProperty_Label );
      }

      /// <summary>
      /// Sets the value of a property given a label.     
      /// The xmlRoot should point to the root of the property object.
      /// If <paramref name=" xmlRoot"/> is part of a <paramref name=" PropertyEditorGUI"/>
      /// then use <paramref name=" PropertyEditorGUI.SetValueAtLabel"/>, 
      /// otherwise the control will not be updated.
      /// </summary>
      /// <param name="xmlRoot">Root of label search.</param>
      /// <returns>
      /// Nulled string if no label. Otherwise returns the previous value.
      /// </returns>
      /// <remarks>
      /// No type checking is performed on 'value'.
      /// Callee needs to be sure that the format of the string passed matches that of the property type.
      /// <para>
      /// e.g. type="vector3f", value="0.1 0.2 0.3".
      /// </para>
      /// <para>
      /// e.g. type="enum", value="kNone".
      /// </para>
      /// </remarks>
      static public string SetValueAtLabel(XPathNavigator xmlRoot, string label, string value)
      {
         string oldValue = null; // nullable type
         XPathNavigator xmlNav = GetXmlNavForLabel(xmlRoot, label);
         if( xmlNav != null )
         {
            // Only deal with the first label
            oldValue = SetPropertyValueElement(xmlNav, value);
         }

         // string value will be null if not found
         return oldValue;
      }

      static public string SetValueAtLabel( XmlNode xmlRoot, string label, string value )
      {
         return SetValueAtLabel( xmlRoot.CreateNavigator(), label, value );
      }
      /// <summary>
      /// Gets value string for a label.      
      /// <paramref name="xmlRoot"/> is root of label search.
      /// The xmlRoot should point to the root of the property object.
      /// </summary>
      /// <returns>
      /// Nulled string if no value.
      /// </returns>
      static private string GetValueAtLabel(XPathNavigator xmlRoot, string label)
      {
         string value = null; // nullable type

         XPathNavigator xmlNav = GetXmlNavForLabel(xmlRoot, label);
         if( xmlNav != null )
         {
            value = GetPropertyValueElement(xmlNav);
         }

         // string value will be null if not found
         return value;
      }

      static public string GetValueAtLabel( XmlNode xmlRoot, string label )
      {
         return GetValueAtLabel( xmlRoot.CreateNavigator(), label );
      }

      /// <summary>
      /// Finds the xpath for property label.
      /// </summary>
      /// <paramref name="xmlRoot"/> is root of label search.
      /// The xmlRoot should point to the root of the property object.
      /// <returns>
      /// Returns the path to the property that has the label <c>label</c>.
      /// Will return <c>null</c> if label cannot be found.
      /// </returns>
      static public XPathNavigator GetXmlNavForLabel(XPathNavigator xmlRoot, string label)
      {
         string selectLabels = string.Format("descendant-or-self::Property[@label='{0}']", label);
         return xmlRoot.SelectSingleNode(selectLabels);
      }

      static public XmlElement GetXmlElementForLabel( XmlNode xmlRoot, string label )
      {
         string selectLabels = string.Format( "descendant-or-self::Property[@label='{0}']", label );
         return (XmlElement) xmlRoot.SelectSingleNode( selectLabels );
      }

      /// <summary>
      /// Gets list of component names that exist in the given component group
      /// </summary>
      /// <param name="xmlNav">Points to component group</param>
      /// <param name="components">Array to fill with list of components</param>
      public static void GetComponentsFromComponentGroup( XPathNavigator xmlNav, IPropertyDataSource propertySource, out List<string> components )
      {
         components = new List<string>();

         string groupId = xmlNav.GetAttribute(Helpers.kComponentGroup_Id, "");
         List<XPathNavigator> componentsInGroup = propertySource.FindComponentsByGroup(groupId);

         foreach (XPathNavigator component in componentsInGroup)
         {
            string componentName = component.GetAttribute(Helpers.kComponent_Id, "");
            components.Add(componentName);
         }
         
         // make sure they are sorted
         components.Sort();

         bool bAllowEmptyComponent = Helpers.GetAllowEmptyComponentFromComponentGroup(xmlNav);
         if( bAllowEmptyComponent )
            components.Insert(0, kEmptyComponentName);
      }
      public static List<string> GetComponentsFromComponentGroup( XmlElement xml, IPropertyDataSource propertySource )
      {
         List<string> components;

         GetComponentsFromComponentGroup( xml.CreateNavigator(), propertySource, out components );
         return components;
      }
      /// <summary>
      /// Gets name of active component for the given component group.
      /// </summary>
      /// <param name="xmlNav">points to component group</param>
      /// <returns>current value of active component, if there is no active value tag it will take the first available component</returns>
      public static string GetActiveComponentNameFromComponentGroup( XmlElement element )
      {
         string activeComponent = element.GetAttribute(MergePropertiesV2.kComponentGroup_activeComponent, "");
         
         if (activeComponent != "")
            return activeComponent;

         XmlNode nav = element.SelectSingleNode( "./" + Helpers.kElement_ActiveComponent );
         if ( nav != null )
         {
            return nav.InnerText;
         }
         else
         {
            return "None";
         }
      }

      /// <summary>
      /// Gets active component for the given component group.
      /// </summary>
      /// <param name="componentGroupNav">points to component group</param>
      /// <returns>Navigator to active component, will return 'null' if no component active</returns>
      public static XmlElement GetActiveComponentFromComponentGroup(XmlElement componentGroupNav)
      {
         string activeComponentName = GetActiveComponentNameFromComponentGroup(componentGroupNav);
         if (activeComponentName == Helpers.kEmptyComponentName)
         {
            // No component active
            return null;
         }
         XmlElement componentNav = GetComponentFromComponentGroup(componentGroupNav, activeComponentName);
         return componentNav;
      }

      /// <summary>
      /// Sets the name of the active component for the given component group.
      /// </summary>
      /// <param name="activeComponent">name of the new component group</param>
      /// <param name="xmlNav">points to component group</param>
      public static void SetActiveComponentInComponentGroup(string activeComponent, XPathNavigator xmlNav)
      {
         XPathNavigator xmlNavClone = xmlNav.Clone();

         if( xmlNavClone.MoveToAttribute(MergePropertiesV2.kComponentGroup_activeComponent, "") )
         {
            xmlNavClone.SetValue(activeComponent);
            return;
         }

         XPathNavigator nav = xmlNav.SelectSingleNode( "./" + Helpers.kElement_ActiveComponent );
         if( nav != null )
         {
            nav.SetValue(activeComponent);
         }
         else
         {
            xmlNav.AppendChildElement("", Helpers.kElement_ActiveComponent, "", activeComponent);
         }
      }

      public static void SetActiveComponentInComponentGroup( string activeComponent, XmlElement xml )
      {
         SetActiveComponentInComponentGroup( activeComponent, xml.CreateNavigator() );
      }

      /// <summary>
      /// Gets navigator for a component given a component group and the desired name
      /// </summary>
      /// <param name="componentGroup">Points to the component group for which to retrieve component from</param>
      /// <param name="componentName">Name of component you want to retrieve</param>
      /// <returns>Navigator for component or null if component is not found.</returns>
      public static XPathNavigator GetComponentFromComponentGroup( XPathNavigator componentGroup, string componentName )
      {
         if( componentGroup.GetAttribute(MergePropertiesV2.kComponentGroup_activeComponent, "") == componentName)
         {
            XPathNavigator componentNav = componentGroup.Clone();
            if( componentNav.MoveToChild(Helpers.kElement_Component, "") )
            {
               return componentNav;
            }
            else
            {
               return null;
            }
         }

         XPathNavigator foundNavigator = componentGroup.SelectSingleNode( "./" + Helpers.kElement_Component + "[@" + Helpers.kComponent_Name + "=\"" + componentName + "\"]" );
         
         if (foundNavigator != null)
            return foundNavigator.Clone();
         else
            return null;
      }
      public static XmlElement GetComponentFromComponentGroup( XmlElement componentGroup, string componentName )
      {
         XPathNavigator nav = GetComponentFromComponentGroup( componentGroup.CreateNavigator(), componentName );
         if ( nav == null )
         {
            return null;
         }
         else
         {
            return (XmlElement) nav.UnderlyingObject;
         }
      }
      public static XPathNavigator GetComponentFromEditorId(XPathNavigator objectXml, Guid componentEditorId)
      {
         XPathNavigator componentNav = objectXml.SelectSingleNode(String.Format("//ComponentGroup[@editorId=\"{0}\"]/Component", componentEditorId));
         return componentNav;
      }
      public static XmlElement GetComponentFromEditorId( XmlElement objectXml, Guid componentEditorId )
      {
         return (XmlElement) objectXml.SelectSingleNode( String.Format( "//ComponentGroup[@editorId=\"{0}\"]/Component", componentEditorId ) );
      }

      public static XPathNavigator GetComponentGroupFromEditorId(XPathNavigator objectXml, Guid componentEditorId)
      {
         XPathNavigator groupNav = objectXml.SelectSingleNode(String.Format("//ComponentGroup[@editorId=\"{0}\"]", componentEditorId));
         return groupNav;
      }

      public static XPathNavigator GetComponentGroupFromName(XPathNavigator propertyContainerXml, String name)
      {
         return propertyContainerXml.SelectSingleNode( String.Format( "ComponentGroup[@name={0}]", XmlNodeHelpers.AsQuotedXml( name ) ) );
      }

      /// <summary>
      /// Returns the name for display given the navigator, it will simply test whether it has a displayName attribute or name attribute.
      /// </summary>
      /// <param name="xmlNav">Property to retrieve display name for.</param>
      /// <returns>Display name</returns>
      public static string GetDisplayName(XPathNavigator xmlNav)
      {
         string displayName = xmlNav.GetAttribute(kProperty_DisplayName, String.Empty);
         if( displayName != String.Empty )
            return displayName;
            
         if (Tools.Common.Misc.ParseBool(xmlNav.GetAttribute(kProperty_UseParentName, String.Empty)))
         {
            XPathNavigator parentNav = xmlNav.Clone();
            if (parentNav.MoveToParent())
            {
               return parentNav.GetAttribute(kProperty_Name, String.Empty);
            }
         }

         string name = xmlNav.GetAttribute(kProperty_Name, String.Empty);
         
         if( string.IsNullOrEmpty(name) )
         {
            name = xmlNav.GetAttribute(kProperty_Id, String.Empty);
         }

         return name;
      }
      public static string GetDisplayName( XmlElement xmlNav )
      {
         return GetDisplayName( xmlNav.CreateNavigator() );
      }

      /// <summary>
      /// Returns the short name for display given the navigator. If no short name exists then will use the regular display name.
      /// </summary>
      /// <param name="xmlNav">Property to retrieve display name for.</param>
      /// <returns>Display name</returns>
      public static string GetShortDisplayName(XPathNavigator xmlNav)
      {
         string displayName = xmlNav.GetAttribute(kProperty_ShortDisplayName, String.Empty);
         if( displayName != String.Empty )
         {
            return displayName;
         }
         
         return GetDisplayName(xmlNav);
      }
      public static string GetShortDisplayName( XmlElement element )
      {
         return GetShortDisplayName( element.CreateNavigator() );
      }

      /// <summary>
      /// Can this property been seen in property editor?
      /// </summary>
      public static bool IsVisible(XPathNavigator xmlNav)
      {
         string isVisible = xmlNav.GetAttribute(kProperty_IsVisible, String.Empty);
         if( isVisible != String.Empty)
         {
            return Tools.Common.Misc.ParseBool(isVisible);
         }

         return true;
      }
      public static bool IsVisible( XmlElement xml )
      {
         return IsVisible( xml.CreateNavigator() );
      }

      /// <summary>
      /// Returns wheter the component group indicated by xmlNav allows a null component
      /// </summary>
      /// <param name="xmlNav">Points to component group</param>
      /// <returns>AllowEmptyComponent value</returns>
      public static bool GetAllowEmptyComponentFromComponentGroup(XPathNavigator xmlNav)
      {
         string value = xmlNav.GetAttribute(kComponentGroup_AllowEmptyComponent, "");
         if( value.Length > 0 )
            return bool.Parse(value);

         return false;
      }

      /// <summary>
      /// Retrieves list of all valid asset types (FourCCs) for the given asset property.
      /// </summary>
      /// <param name="xmlNav"></param>
      /// <param name="assetTypes"></param>
      public static void GetAssetTypesForAssetProperty(XPathNavigator xmlNav, out string[] assetTypes)
      {
         string value = xmlNav.GetAttribute(Helpers.kProperty_AssetTypes, "");
         if( value != "" )
            assetTypes = value.Split(new char[] { ';' });
         else
            assetTypes = new string[0];
      }
      public static string[] GetAssetTypesForAssetProperty( XmlElement xml )
      {
         string[] ret;
         GetAssetTypesForAssetProperty( xml.CreateNavigator(), out ret );
         return ret;
      }

      public static string[] GetComponentPlatforms( XmlElement xml )
      {
         string value = xml.GetAttribute( Helpers.kComponent_Platforms, String.Empty );

         if ( !String.IsNullOrEmpty( value ) )
         {
            string[] ret = value.Split( new char[] { ';' } );

            if ( ret.Length > 0 )
            {
               return ret;
            }
         }

         return null;
      }

      /// <summary>
      /// Gets 'noExport' attribute from node. 
      /// </summary>
      /// <param name="propertyNode"></param>
      /// <returns>
      /// True if the property/propertyobject/Component should be exported.
      /// </returns>
      public static bool ExportProperty(XmlNodeReader propertyNode)
      {
         string noExport = propertyNode.GetAttribute(Helpers.kProperty_NoExport, "");
         if (noExport == null)
         {
            // Attribute not present, export it.
            return true;
         }
         // Attribute present, return opposite.
         bool bShouldExport = !Tools.Common.Misc.ParseBool(noExport);
         return bShouldExport;
      }

      /// <summary>
      /// If a navigator is passed it at the script object level, 
      /// it will return the script object guid id.
      /// If it's a component nav, it will return the component id.
      /// Returns first property editor id guid encountered.
      /// </summary>
      /// <param name="objectOrComponentNav"></param>
      /// <returns></returns>
      public static Guid GetInternalEditorIdGuid(XPathNavigator objectOrComponentNav)
      {
         XmlNode node = objectOrComponentNav.UnderlyingObject as XmlNode;
         
         if (node.NodeType != XmlNodeType.Element)
            node = node.FirstChild;

         XmlElement element = node as XmlElement;
         if( element != null )
         {
            switch(element.Name)
            {
               case Helpers.kElement_PropertyContainer:
                  return new Guid(element.GetAttribute(MergePropertiesV2.kPropertyContainer_editorId));
               
               case Helpers.kElement_ComponentGroup:
                  return new Guid(element.GetAttribute(MergePropertiesV2.kComponentGroup_editorId));

               case Helpers.kElement_Component:
                  {
                     XmlElement componentGroup = element.ParentNode as XmlElement;
                     return new Guid(componentGroup.GetAttribute(MergePropertiesV2.kComponentGroup_editorId));
                  }
               
               default:
                  Debug.Assert(false, "Invalid node passed into GetInternalEditorIdGuid");
                  break;
            }
         }

         return Guid.Empty;
      }
      public static Guid GetInternalEditorIdGuid( XmlNode objectOrComponent )
      {
         return GetInternalEditorIdGuid( objectOrComponent.CreateNavigator() );
      }
      /// <summary>
      /// Returns if a message specified has an associated message object.
      /// </summary>
      public static bool MessageInComponentHasProperties(XPathNavigator messageNav, IPropertyDataSource propertySource)
      {
         if (propertySource.FindMessageObjectById(messageNav.GetAttribute(kMessage_type, "")) != null)
         {
            return true;
         }
         return false;
      }
      /// <summary>
      /// Returns if a message object specified in a expanded component has properties.
      /// The build BuildFullPropertyTree code will replaces unknown message objects with empty ones.
      /// If the node is an empty element, then it's got no properties.
      /// </summary>
      public static bool MessageObjectHasProperties(XmlNodeReader messageNode)
      {
         bool hasProperties = !messageNode.IsEmptyElement;
         return hasProperties;
      }

      /// <summary>
      /// Creates event with default settings. Use returned navigator to modify attributes.
      /// </summary>
      /// <param name="componentNav"></param>
      /// <param name="eventTypeName"></param>
      /// <param name="messageType"></param>
      /// <param name="targetComponentGuid"></param>
      /// <param name="messageSource"></param>
      /// <returns></returns>
      public static XPathNavigator CreateEvent(XPathNavigator componentNav, string eventTypeName, string messageType, Guid targetComponentGuid, IPropertyDataSource propertySource)
      {
         // Find event entry
         XPathNodeIterator eventIter = componentNav.Select(String.Format("Events/Event[@type=\"{0}\"]", eventTypeName));
         if (eventIter.Count == 0)
         {
            // Can't find this event
            return null;
         }
         Debug.Assert(eventIter.Count == 1, "Too many event matches");
         eventIter.MoveNext();

         XPathNavigator eventNav = eventIter.Current;
         // Create element wrapper
         string newEntry = String.Format("<EventEntry active=\"true\" time=\"0.0\" id=\"{0}\" target=\"{1}\" priority=\"0\" identifier=\"\" query=\"\">", Guid.NewGuid(), targetComponentGuid);
         // Add message xml
         newEntry += String.Format("<UsingMessageObject id=\"{0}\" name=\"{1}\"/>", messageType, eventTypeName);
         // Close event entry element
         newEntry += "</EventEntry>";

         XmlDocument mergedEntry = new XmlDocument();
         mergedEntry.LoadXml(newEntry);

         MergePropertiesV2.Merge_Inplace(mergedEntry, propertySource);

         eventNav.AppendChild(XmlNodeHelpers.GetDocumentRootElement(mergedEntry).CreateNavigator());

         // Move to the newly created element
         eventNav.MoveToChild(XPathNodeType.Element);

         // return the newly created event
         return eventNav;
      }
      public static XmlElement CreateEvent( XmlElement component, string eventTypeName, string messageType, Guid targetComponentGuid, IPropertyDataSource propertySource )
      {
         XPathNavigator nav = CreateEvent( component.CreateNavigator(), eventTypeName, messageType, targetComponentGuid, propertySource );
         return (XmlElement) nav.UnderlyingObject;
      }
      /// <summary>
      /// Creates link with default settings. Use returned navigator to modify attributes.
      /// </summary>
      /// <param name="componentNav"></param>
      /// <param name="linkTypeName"></param>
      /// <param name="messageType"></param>
      /// <param name="targetComponentGuid"></param>
      /// <param name="messageSource"></param>
      /// <returns></returns>
      public static XPathNavigator CreateLink(XPathNavigator componentNav, string linkTypeName, Guid targetComponentGuid, IPropertyDataSource propertySource)
      {
         // Note that links don't really need identifiers as they don't send messages.
         // However, I believe they will be useful for filtering objects in game code.
         
         // Find Link entry
         XPathNodeIterator linkIter = componentNav.Select(String.Format("Links/Link[@type=\"{0}\"]", linkTypeName));
         if (linkIter.Count == 0)
         {
            // Can't find this Link
            return null;
         }
         Debug.Assert(linkIter.Count == 1, "Too many Link matches");
         linkIter.MoveNext();


         XPathNavigator linkNav = linkIter.Current;
         string messageType = linkNav.GetAttribute(kLink_message, "");
         if (messageType.Length == 0) messageType = Helpers.kLink_message_emptyMessage;  // Avoid empty message type if not specified by <Link messageType="foo"/>
         // Create element wrapper
         string newEntry = String.Format("<LinkEntry active=\"true\" time=\"0\" id=\"{0}\" target=\"{1}\" priority=\"0\" identifier=\"\" query=\"\">", Guid.NewGuid(), targetComponentGuid);
         // Add message xml
         newEntry += String.Format("<UsingMessageObject id=\"{0}\" name=\"{1}\"/>", messageType, linkTypeName);
         // Close Link entry element
         newEntry += "</LinkEntry>";

         XmlDocument mergedEntry = new XmlDocument();
         mergedEntry.LoadXml(newEntry);

         MergePropertiesV2.Merge_Inplace(mergedEntry, propertySource);

         linkNav.AppendChild( XmlNodeHelpers.GetDocumentRootElement( mergedEntry ).CreateNavigator() );

         // Move to the newly created element
         linkNav.MoveToChild(XPathNodeType.Element);

         // return the newly created Link
         return linkNav;
      }
      public static XmlElement CreateLink( XmlElement component, string linkTypeName, Guid targetComponentGuid, IPropertyDataSource propertySource )
      {
         XPathNavigator nav = CreateLink( component.CreateNavigator(), linkTypeName, targetComponentGuid, propertySource );
         return (XmlElement) nav.UnderlyingObject;
      }
      /// <summary>
      /// Determines if the element pointed to is a vector property object
      /// </summary>
      /// <param name="xmlNav"></param>
      /// <returns></returns>
      public static bool IsVectorProperty(XPathNavigator xmlNav)
      {
         string id = xmlNav.GetAttribute(Helpers.kProperty_Id, "");
         string cookas = xmlNav.GetAttribute(Helpers.kProperty_CookAs, "");
         if (!string.IsNullOrEmpty(cookas))
         {
            id = cookas;
         }

         switch (xmlNav.Name)
         {
            case Helpers.kElement_PropertyObject:
               return id == Helpers.kProperty_Type_vector3;
            case Helpers.kElement_EvaluatorProperty:
               return id == Helpers.kEvaluatorProperty_Type_vector3;
         }

         return false;
      }
      public static bool IsVectorProperty( XmlElement xml )
      {
         return IsVectorProperty( xml.CreateNavigator() );
      }
      public static bool CanLinkToComponent(XPathNavigator linkNav, XPathNavigator componentNav)
      {
         string matchName = String.Format("{0}:{1}", componentNav.GetAttribute(kComponent_Group, String.Empty), componentNav.GetAttribute(kComponent_Id, String.Empty));
         XPathNodeIterator linkDenyIter = linkNav.Select("./Deny");
         while (linkDenyIter.MoveNext())
         {
            string deny = linkDenyIter.Current.Value;
            if (!deny.Contains(":"))
            {
               // Add ':' delimiter to fix prevent incorrect matches with just the start of the component group name
               deny = deny + ":";
            }
            if (matchName.StartsWith(deny))
            {
               // Nope, denied!
               return false;
            }
         }
         
         bool bDenied = false;
         XPathNodeIterator linkAllowIter = linkNav.Select("./Allow");
         while (linkAllowIter.MoveNext())
         {
            // If 'Allow' is present then all types denied by default
            bDenied = true;
            string allow = linkAllowIter.Current.Value;
            if (!allow.Contains(":"))
            {
               // Add ':' delimiter to fix prevent incorrect matches with just the start of the component group name
               allow = allow + ":";
            }
            if (matchName.StartsWith(allow))
            {
               // OK, we can link to this
               return true;
            }
         }
         
         return !bDenied;
      }
      public static bool CanLinkToComponent( XmlElement link, XmlElement component )
      {
         return CanLinkToComponent( link.CreateNavigator(), component.CreateNavigator() );
      }
      /// <summary>
      /// Returns wheter the evaluator property indicated by xmlNav allows a null evaluator
      /// </summary>
      /// <param name="xmlNav">Points to evaluator property</param>
      /// <returns>AllowEmptyEvaluator value</returns>
      public static bool GetAllowEmptyEvaluatorFromEvaluatorProperty(XPathNavigator xmlNav)
      {
         string value = xmlNav.GetAttribute(kEvaluatorProperty_AllowEmptyEvaluator, "");
         if (value.Length > 0)
            return bool.Parse(value);

         return false;
      }

      /// <summary>
      /// Retrieves list of evaluator names that are available for the given evaluator property
      /// </summary>
      /// <param name="propertyDB"></param>
      /// <param name="xmlNav"></param>
      /// <param name="evaluators"></param>
      public static void GetEvaluatorsForEvaluatorProperty(IPropertyDataSource propertyDB, XPathNavigator xmlNav, out List<string> evaluators)
      {
         evaluators = new List<string>();
         if (propertyDB != null)
         {
            List<XPathNavigator> evaluatorsXml = propertyDB.FindEvaluatorsByType(xmlNav.GetAttribute(Helpers.kEvaluatorProperty_Type, ""));
            foreach (XPathNavigator evaluatorXml in evaluatorsXml)
            {
               string evaluatorName = evaluatorXml.GetAttribute(Helpers.kEvaluator_Id, "");
               evaluators.Add(evaluatorName);
            }
         }

         // make sure they are sorted
         evaluators.Sort();

         bool bAllowEmptyEvaluator = Helpers.GetAllowEmptyEvaluatorFromEvaluatorProperty(xmlNav);
         if (bAllowEmptyEvaluator)
            evaluators.Insert(0, kEmptyEvaluatorName);

      }
      public static List<string> GetEvaluatorsForEvaluatorProperty( IPropertyDataSource propertyDB, XmlElement xml )
      {
         List<String> ret;
         GetEvaluatorsForEvaluatorProperty( propertyDB, xml.CreateNavigator(), out ret );
         return ret;
      }
      /// <summary>
      /// Retrieves the current active evaluator for the given evaluator property
      /// </summary>
      /// <param name="Navigator"></param>
      /// <returns></returns>
      public static string GetActiveEvaluatorFromEvaluatorProperty(XPathNavigator nav)
      {
         XPathNavigator evaluator = nav.SelectSingleNode( Helpers.kElement_Evaluator );
         if (evaluator != null)
         {
            return evaluator.GetAttribute(Helpers.kEvaluator_Id, "");
         }

         return kEmptyEvaluatorName;
      }
      public static string GetActiveEvaluatorFromEvaluatorProperty( XmlElement element )
      {
         return GetActiveEvaluatorFromEvaluatorProperty( element.CreateNavigator() );
      }

      /// <summary>
      /// Sets the current Evaluator for the EvaluatorProperty to be the one specified by evaluatorName
      /// </summary>
      /// <param name="propertyDB"></param>
      /// <param name="Navigator"></param>
      /// <param name="evaluatorName"></param>
      public static void SetActiveEvaluatorForEvaluatorProperty(IPropertyDataSource propertyDB, XPathNavigator xmlNav, string evaluatorName)
      {
         if (xmlNav.Name == Helpers.kEvaluatorProperty)
         {
            XPathNavigator oldEvaluator = xmlNav.SelectSingleNode(Helpers.kElement_Evaluator);

            if (evaluatorName == kEmptyEvaluatorName)
            {
               // Force a dummy 'None' evaluator so we can ungroup it correctly
               xmlNav.InnerXml = "<Evaluator id=\"None\"/>";
            }
            else
            {
               string oldDefault = (oldEvaluator != null) ? oldEvaluator.GetAttribute(Helpers.kEvaluator_Id_Default, "") : "";

               // Create property like this so it does all the fixup for missing values etc.
               XmlDocument mergedEvaluator = new XmlDocument();
               mergedEvaluator.LoadXml(String.Format("<UsingEvaluator name=\"dont_care\" id=\"{0}\"/>", evaluatorName));
               MergePropertiesV2.Merge_Inplace(mergedEvaluator, propertyDB);

               XmlElement evaluatorElement = mergedEvaluator[Helpers.kElement_Evaluator];
               evaluatorElement.SetAttribute(Helpers.kEvaluator_Id_Default, oldDefault);

               xmlNav.InnerXml = mergedEvaluator.OuterXml;
            }
            // Update the 'activeEvaluator' attributes
            XmlElement evaluatorPropertyElement = xmlNav.UnderlyingObject as XmlElement;
            evaluatorPropertyElement.SetAttribute(Helpers.kEvaluatorProperty_ActiveEvaluator, evaluatorName);
         }
      }
      public static void SetActiveEvaluatorForEvaluatorProperty( IPropertyDataSource propertyDB, XmlElement xml, string evaluatorName )
      {
         SetActiveEvaluatorForEvaluatorProperty( propertyDB, xml.CreateNavigator(), evaluatorName );
      }
      public static void AddDynamicComponentGroup( IPropertyDataSource propertyDB, 
                                                   XPathNavigator propertyObjectXmlNav, 
                                                   string componentGroupId, 
                                                   string componentName, 
                                                   string componentGroupName)
      {
         // Build fake component group
         // This will set defaults and take care of other housekeeping

         // Get maxSubComponents, need to rework this so that we can build components within components dynamically.
         string maxSubComponents = "";
         {
            XPathNavigator componentNav = propertyDB.FindComponentByGroupAndId(componentGroupId + ":" + componentName);
            if (componentNav != null)
            {
               maxSubComponents = componentNav.GetAttribute(kComponentGroup_MaxSubComponents, String.Empty);
            }
            if (maxSubComponents.Length == 0)
            {
               // Default
               maxSubComponents = "3";
            }
         }

         XmlDocument newComponentGroup = new XmlDocument();

         newComponentGroup.InnerXml = String.Format("<AddComponentGroup id=\"{0}\" name=\"{1}\" dynamicComponentGroup=\"true\" maxSubComponents=\"{2}\" activeComponent=\"{3}\"/>", componentGroupId, componentGroupName, maxSubComponents, componentName);
         MergePropertiesV2.Merge_Inplace(newComponentGroup, propertyDB);
         XPathNavigator newComponentGroupNav = newComponentGroup.CreateNavigator();
         newComponentGroupNav.MoveToChild(XPathNodeType.Element);
         // Append to existing xml
         XPathNavigator propertyContainer = propertyObjectXmlNav.SelectSingleNode(kElement_PropertyContainer);
         propertyContainer.AppendChild(newComponentGroupNav);
         MergeLayout.UpdateSingleLayout(propertyContainer);
      }
      
      public static void AddDynamicComponentGroup( IPropertyDataSource propertyDB,
                                                   XmlDocument propertyObject,
                                                   string componentGroupId,
                                                   string componentName,
                                                   string componentGroupName )
      {
         AddDynamicComponentGroup( propertyDB, propertyObject.CreateNavigator(), componentGroupId, componentName, componentGroupName );
      }

      public static String GenerateNewDynamicComponentGroupName(XPathNavigator propertyObjectXmlNav, string wantName)
      {
         string queryFormat = "PropertyContainer/ComponentGroup[@name='{0}']";
         // If we have a component with this name, then keep looking
         if (propertyObjectXmlNav.SelectSingleNode(String.Format(queryFormat, wantName)) != null)
         {
            for (int i = 1; ; ++i)
            {
               string newName = String.Format("{0}{1}", wantName, i);
               // Punt out if there's no component with this name!
               if (propertyObjectXmlNav.SelectSingleNode(String.Format(queryFormat, newName)) == null )
               {
                  return newName;
               }
            }

         }
         else
         {
            return wantName;
         }
      }

      public static String GenerateNewDynamicComponentGroupName( XmlDocument propertyObject, string wantName )
      {
         return GenerateNewDynamicComponentGroupName( propertyObject.CreateNavigator(), wantName );
      }

      /// <summary>
      /// Converts merged properties to dynamic components. Note that XmlNodes/XPathNavigators within updated xml will be invalidated.
      /// </summary>
      public static void ConvertPropertyObjectToDynamicComponents_Unsafe(XmlDocument propertyContainer, IPropertyDataSource propertyDB)
      {
         XmlElement propertyContainerElement = Tools.Common.XmlNodeHelpers.GetDocumentRootElement( propertyContainer );

         // Check that we're actually on a PropertyContainer node
         Debug.Assert(propertyContainerElement.Name == kElement_PropertyContainer);

         // Convert to 'Minimal' property container, otherwise existing default components will get re-appended on merge
         propertyContainerElement.SetAttribute(Helpers.kProperty_Id, "Minimal");

         bool bFirst = true;
         foreach(XmlElement componentGroup in propertyContainerElement.SelectNodes("./ComponentGroup"))
         {
            if (bFirst)
            {
               // We don't want to make the first component group dynamic as it's required to be there.
               bFirst = false;
               continue;
            }

            if( componentGroup.GetAttributeNode(Helpers.kComponentGroup_Dynamic) == null )
            {
               componentGroup.SetAttribute(Helpers.kComponentGroup_Dynamic, "true");
            }
         }

         // Remove all defaults, otherwise overridden component values from the property container will be ignored.
         // e.g. for the 'RenderActor' PC, the 'Model' asset is overridden, with it's 'default' set to the overridden value.
         // This 'value' would get removed as the defaults match, causing the 'Model' asset value to revert to the 
         // original value for the render actor component.
         Tools.Common.XmlNodeHelpers.DeleteAllFromXmlNode(propertyContainerElement, ".//Value/@default");

         Tools.Property.Build.MergePropertiesV2.Unmerge_Inplace(propertyContainer);
         Tools.Property.Build.MergePropertiesV2.Merge_Inplace(propertyContainer, propertyDB);
      }

      public static bool IsEventOrLinkEntryPrefab(XmlElement entry)
      {
         return Common.Misc.ParseBool(entry.GetAttribute("isPrefab") ?? "false");
      }

      public static bool IsComponentGroupPrefab(XPathNavigator nav)
      {
         return Common.Misc.ParseBool(nav.GetAttribute("isPrefab", String.Empty) ?? "false");
      }

      public static bool IsComponentGroupPrefab( XmlElement element )
      {
         return IsComponentGroupPrefab( element.CreateNavigator() );
      }

      public static bool IsComponentGroupDynamic(XPathNavigator nav)
      {
         return Common.Misc.ParseBool(nav.GetAttribute(Helpers.kComponentGroup_Dynamic, String.Empty) ?? "false");
      }
      public static bool IsComponentGroupDynamic( XmlElement nav )
      {
         return Common.Misc.ParseBool( nav.GetAttribute( Helpers.kComponentGroup_Dynamic ) ?? "false" );
      }

      public static bool DeleteTopLevelComponentGroup( XPathNavigator nav, String groupType, String component, String name)
      {
         XPathNavigator componentGroupNav = nav.SelectSingleNode( String.Format( "PropertyContainer/ComponentGroup[@name={0} and @id={1} and @activeComponent={2}]", XmlNodeHelpers.AsQuotedXml( name ), XmlNodeHelpers.AsQuotedXml( groupType ), XmlNodeHelpers.AsQuotedXml( component ) ) );

         if (componentGroupNav != null)
         {
            componentGroupNav.DeleteSelf();
            MergeLayout.UpdateSingleLayout(nav);
            return true;
         }
         else
         {
            return false;
         }
      }
      public static bool DeleteTopLevelComponentGroup( XmlDocument doc, String groupType, String component, String name )
      {
         return DeleteTopLevelComponentGroup( doc.CreateNavigator(), groupType, component, name );
      }

      /// <summary>
      /// This function returns a unique path made up of all path elements from the root of the property container to the property.
      /// Certain elements that don't matter (like layouts) are ignored in the path
      /// </summary>
      /// <param name="xmlNav">Points to property to retrieve path for.</param>
      /// <returns>Path elements separated by "_" and prefixed with "PX_"</returns>
      public static string GetPathForProperty(XPathNavigator xmlNav)
      {
         List<string> pathElements = new List<string>();

         XPathNavigator it = xmlNav.Clone();
         
         bool ignoreFurtherParents = false;
         while (!ignoreFurtherParents)
         {
            switch( it.Name )
            {
               case Helpers.kElement_Property:
               case Helpers.kElement_PropertyObject:
               case Helpers.kElement_EvaluatorProperty:
               case Helpers.kElement_Component:
               case Helpers.kElement_ComponentGroup:
                  {
                     pathElements.Insert(0, Helpers.GetDisplayName(it));
                  }
                  break;

               case Helpers.kElement_PropertyContainer:
                  {
                     ignoreFurtherParents = true;
                  }
                  break;

               case Helpers.kElement_Evaluator:
               case Helpers.kElement_Layout:
                  {
                  }
                  break;

                  // these can be hit assuming the spline is child of an evaluator under a evaluator container entry
               case Helpers.kElement_EvaluatorContainerRoot:
               case Helpers.kElement_Value:
                  {
                  }
                  break;

                  // special case if we're building the path of an evaluator container entry, we directly return the target property as a unique path
               case Helpers.kElement_EvaluatorContainerEntry:
                  {
                     if (pathElements.Count > 0)
                     {
                        return it.GetAttribute(Helpers.kEvaluatorContainerEntry_targetProperty, string.Empty) + "_" + string.Join("_", pathElements.ToArray());
                     }
                     else
                     {
                        return it.GetAttribute(Helpers.kEvaluatorContainerEntry_targetProperty, string.Empty);
                     }
                  }
               
               default:
                  {
                     Debug.Assert(false, "Invalid node type");
                  }
                  break;
            }

            it.MoveToParent();
         }

         if (pathElements.Count != 0)
         {
            return "PX_" + string.Join("_", pathElements.ToArray());
         }

         return string.Empty;
      }
     
      /// <summary>
      /// Modifies xml to add new new bit field 'int' properties and mark old 'bool' bitfield properties for 'noExport'.
      /// </summary>
      public static void AddBitFieldProperties(XPathNavigator nav)
      {
         XPathExpression findTopBitField = XPathExpression.Compile(".//Property[@bitFieldName and @bitFieldPosition and not(@noExport)]");
         XPathNavigator topBitFieldNav = null;
         while ((topBitFieldNav = nav.SelectSingleNode(findTopBitField)) != null)
         {
            int bitFieldTotal = 0;
            string bitFieldName = topBitFieldNav.GetAttribute(Helpers.kProperty_Type_bool_bitfield_name, String.Empty);
            string xpathQuery = String.Format("parent::*/Property[@bitFieldName=\"{0}\" and @bitFieldPosition and not(@noExport)]", bitFieldName);
            XPathNodeIterator bitFieldIter = topBitFieldNav.Select(xpathQuery);
            while(bitFieldIter.MoveNext())
            {
               if (Tools.Common.Misc.ParseBool(GetPropertyValueElement(bitFieldIter.Current)))
               {
                  // Sum total active bits
                  int bitPosition = int.Parse(bitFieldIter.Current.GetAttribute(Helpers.kProperty_Type_bool_bitfield_position, String.Empty));
                  bitFieldTotal |= (1 << bitPosition);
               }
               // Add a noExport attribute
               bitFieldIter.Current.CreateAttribute(String.Empty, Helpers.kProperty_NoExport, String.Empty, "true");
            }                                                                     
            // Add new int property for bitfield
            topBitFieldNav.InsertBefore(String.Format("<Property name=\"{0}\" type=\"int\"><Value>{1}</Value></Property>", bitFieldName, bitFieldTotal));
         }
      }
      public static void AddBitFieldProperties( XmlElement el )
      {
         AddBitFieldProperties( el.CreateNavigator() );
      }
      public static int DeleteAllFromXPath(XPathNavigator srcNav, string xpathToDelete)
      {
         XPathExpression xpathExpr = XPathExpression.Compile(xpathToDelete);
         return DeleteAllFromXPath(srcNav, xpathExpr);
      }
      
      public static int DeleteAllFromXPath(XPathNavigator srcNav, XPathExpression xpathExpr)
      {
         int deletedCount = 0;

         List<XPathNavigator> navs = ConvertNodeIteratorToNavigatorList(srcNav.Select(xpathExpr));
         foreach(XPathNavigator nav in navs)
         {
            nav.DeleteSelf();
            deletedCount++;
         }

         return deletedCount;
      }

      public static int DeleteAllFromXPath( XmlNode src, string xpathToDelete )
      {
         return DeleteAllFromXPath( src.CreateNavigator(), xpathToDelete );
      }

      public static List<XPathNavigator> ConvertNodeIteratorToNavigatorList(XPathNodeIterator iterator)
      {
         List<XPathNavigator> navigators = new List<XPathNavigator>(iterator.Count);
         {
            while (iterator.MoveNext())
            {
               navigators.Add(iterator.Current.Clone());
            }
         }
         return navigators;
      }


      public static void SpawnHelp(string helpType, string groupName, string name, string section)
      {
         string helpEntry = "";

         if( !string.IsNullOrEmpty(name) )
         {
            helpEntry = string.Format("::/Html/Generated/{0}", helpType);
            if( groupName.Length > 0 )
            {
               helpEntry += "_" + groupName;
            }
            
            helpEntry += string.Format("_{0}.html", name);

            if( section.Length > 0 )
            {
               helpEntry += "#" + section;
            }
         }

         string rootPath = System.Environment.ExpandEnvironmentVariables("%BPE_ROOT%");
         string helpFile = FileUtils.ChooseNewerFile(rootPath + @"\Doc\Help\Help.chm", rootPath + @"\Doc\Help\Help_Local.chm");
         string arguments = "-BPEHelp " +  helpFile + helpEntry;

         ProcessStartInfo startInfo = new ProcessStartInfo("keyhh.exe", arguments);
         startInfo.UseShellExecute = false;
         Process proc = Process.Start(startInfo);
      }

      public static string GetHelpCategory(XPathNavigator nav)
      {
         XPathNavigator cat = nav.SelectSingleNode("./@" + kAny_helpCategory);
         
         if (cat == null)
            return kAny_helpCategory_Default;

         return cat.Value;
      }

      /// <summary>
      /// This function will change the passed in component group to use a the new component with the given id.
      /// NOTE: It is necessary to manually unmerge/merge the document that the component group belongs to afterwards 
      /// to pick up the new properties and get rid of old ones.
      /// </summary>
      /// <param name="componentGroup">Component group that needs to be changed.</param>
      /// <param name="newComponentName">New id of component group to change to.</param>
      private static void ChangeComponentGroupCurrentComponent( XmlElement componentGroup, String newComponentId )
      {
         if (componentGroup.GetAttribute(MergePropertiesV2.kComponentGroup_activeComponent) == newComponentId)
            return;

         componentGroup.SetAttribute(MergePropertiesV2.kComponentGroup_activeComponent, newComponentId);

         XmlElement component = componentGroup[Helpers.kElement_Component];
         if( component != null )
         {
            component.SetAttribute(Helpers.kComponent_Id, newComponentId);
            component.SetAttribute(Helpers.kComponent_Name, newComponentId);
         }
      }

      /// <summary>
      /// This function will change the component group identified by "referenceComponentGroup" to have a component of type "newComponentId" for all passed in property containers.
      /// If useDefaultMerge is specified to be true, it will use the default unmerging/merging code to rebuild the components. Otherwise it is left up to the user to do this.
      /// </summary>
      /// <param name="propertyContainers">List of property containers to be updated.</param>
      /// <param name="referenceComponentGroup">Pointer to the component group that is to be changed, this is only used to build a unique path to find the actual component groups in the passed in property containers.</param>
      /// <param name="newComponentId">New component id that the component groups will be changed to.</param>
      /// <param name="useDefaultMerge">If this is true, it will automatically use the default code to unmerge/merge the property containers to update the changed components. Otherwise it is left up to the user to do this.</param>
      /// <param name="propertySource">Must be specified if useDefaultMerge is true, otherwise it is not used.</param>
      public static void ChangeComponentGroupInPropertyContainers(IEnumerable<XmlDocument> propertyContainers, XmlElement referenceComponentGroup, String newComponentId, bool useDefaultMerge, IPropertyDataSource propertySource)
      {
         string findComponentGroupQuery = MergePropertiesV2.BuildNodeXPathInPropertyContainer(referenceComponentGroup);

         foreach(XmlDocument propertyContainer in propertyContainers)
         {
            XmlElement componentGroupInPropertyContainer = propertyContainer.SelectSingleNode(findComponentGroupQuery) as XmlElement;
            if( componentGroupInPropertyContainer != null )
            {
               ChangeComponentGroupCurrentComponent(componentGroupInPropertyContainer, newComponentId);
               if( useDefaultMerge )
               {
                  MergePropertiesV2.Unmerge_Inplace(propertyContainer);
                  MergePropertiesV2.Merge_Inplace(propertyContainer, propertySource);
               }
            }
         }
      }

      public static bool IsStringSafeAttributeValue(String s)
      {
         return s == XmlNodeHelpers.EscapeXml( s );
      }

      public static XPathNavigator FindDirectChildProperty(XPathNavigator parentNav, String name)
      {
         return parentNav.SelectSingleNode( String.Format( "Property[@name='{0}']", XmlNodeHelpers.EscapeXml( name ) ) );
      }
      public static XmlElement FindDirectChildProperty( XmlElement parentNav, String name )
      {
         return (XmlElement) parentNav.SelectSingleNode( String.Format( "Property[@name='{0}']", XmlNodeHelpers.EscapeXml( name ) ) );
      }

      // this is the hash used by type="string" subType="hashedString"
      public static uint GetSubTypeHashedStringValue(string propertyValue)
      {
         // don't use CodeGenHelpers.GetStringHash
         // the hash methods are not necessarily the same
         return HashUtils.CalculateFNV1a(propertyValue);
      }

      // This is the hash used by message names and by the "messageStringHash" subtype
      public static uint GetMessageNameHash( string messageName )
      {
         return CRC32.CalculateCRC( messageName );
      }

      // This is the hash used by animated event names and by the "messageStringHash" subtype
      public static uint GetAnimationEventNameHash(string messageName)
      {
         return HashUtils.CalculateFNV1a(messageName);
      }
   }
}
