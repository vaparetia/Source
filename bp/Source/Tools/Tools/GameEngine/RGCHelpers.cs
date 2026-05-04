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
using Tools.Property;
using Tools.Property.UI;
using Tools.Property.Misc;

#endregion

namespace Tools.GameEngine
{
   public class RGCHelpers
   {
      public const string kPropertyName_Name = "Name";

      public static bool IsComponentRequiredGameComponent(XPathNavigator nav)
      {
         return IsComponentRequiredGameComponent(nav.GetAttribute(Helpers.kComponent_Group, String.Empty), nav.GetAttribute(Helpers.kComponent_Id, String.Empty));
      }
      public static bool IsComponentRequiredGameComponent( XmlElement nav )
      {
         return IsComponentRequiredGameComponent( nav.GetAttribute( Helpers.kComponent_Group, String.Empty ), nav.GetAttribute( Helpers.kComponent_Id, String.Empty ) );
      }

      public static bool IsComponentRequiredGameComponent(string componentGroup, String componentIdOrName)
      {
         return (componentGroup == "RequiredGameComponent" && (componentIdOrName == "BaseGameProperties" || componentIdOrName == "RGC"));
      }

      public static string GetEditorNameFromComponent(Data.Component componentNav)
      {
         Data.Property propertyNode = (Data.Property) componentNav.SelectSingleNode( "PropertyObject[@id='EditorProperties']/Property[@name='Name']" );

         if ( propertyNode != null )
         {
            return propertyNode.ValueText;
         }
         else
         {
            return String.Empty;
         }
      }

      public static XmlElement GetRGCForScriptObject( XmlElement scriptObjectNav )
      {
         return (XmlElement) scriptObjectNav.SelectSingleNode( "descendant-or-self::PropertyContainer/ComponentGroup[@id='RequiredGameComponent']/Component[@id='BaseGameProperties']" );
      }

      public static XmlElement GetTransformForRGC( XmlElement scriptObjectNav )
      {
         return (XmlElement) scriptObjectNav.SelectSingleNode(
            "PropertyObject[@id='EditorProperties']/PropertyObject[@id='ep_transform']");
      }

      public static XmlElement GetScaleForRGC( XmlElement scriptObjectNav )
      {
         return (XmlElement) scriptObjectNav.SelectSingleNode(
            "PropertyObject[@id='EditorProperties']/PropertyObject[@id='ep_scale']");
      }

      // Build a string that is used to identify a component (made up of component group, component id and component group NAME)
      public static string GetComponentDisplayString(string componentGroup, string componentName, string componentGroupName)
      {
         // Special case RGC/BaseGameProperties as there is only one.
         if (IsComponentRequiredGameComponent(componentGroup, componentName))
            return componentGroupName;
         else
            return string.Format("{0} - {1} : {2}", componentGroup, componentName, componentGroupName);
      }

      // Helper to have base game properties component appears special in the property editor.
      // This function is to be used as a property editor post create component group callback.
      public static void PostCreateComponentGroupCheckForBaseComponent(PropertyEditorGUI propertyEditor, Tools.Property.UI.Controls.ComponentGroupLayout componentGroup)
      {
         Data.ComponentGroup componentGroupElement = componentGroup.ComponentGroupDataNode;
         String componentName = componentGroupElement.ActiveComponentType;
         string componentGroupId =  componentGroupElement.ID;
         string componentGroupName = componentGroupElement.DisplayName;

         if (RGCHelpers.IsComponentRequiredGameComponent(componentGroupId, componentName))
         {
            Tools.Property.UI.Controls.Helper.SetupSpecialComponentGroup(componentGroup);
            componentGroup.GetDisplayName = BaseComponentGetDisplayNameCallback;
            propertyEditor.mCallbacks.mPropertyValueChanged += BaseComponentPropertyValueChangedCallback;
         }
      }
      
      // Callback function to provide name for base component
      private static string BaseComponentGetDisplayNameCallback(Tools.Property.UI.Controls.PropertyControl control)
      {
         Data.ComponentGroup componentGroupElement = (Data.ComponentGroup) control.DataElement;

         return RGCHelpers.GetEditorNameFromComponent( componentGroupElement.ActiveComponent );
      }

      // Callback function to check when a base component needs to be invalidated because the object name has changed.
      private static void BaseComponentPropertyValueChangedCallback(PropertyEditorGUI propertyEditor, PropertyEditorGUI.PropertyEditorCallbacks.PropertyChangedEventArgs e)
      {
         if (e.DataNode.Name == kPropertyName_Name)
         {
            propertyEditor.PropertyPanel.Invalidate();
         }
      }
   }
}
