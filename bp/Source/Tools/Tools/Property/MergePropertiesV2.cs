#region Using directives

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml;
using System.Xml.XPath;
using System.Diagnostics;
using System.Windows.Forms;
using Tools.Property;
using Tools.Property.Source;
using Tools.Property.Misc;
using Tools.Property.UI;
using Tools.Common;

#endregion

namespace Tools.Property.Build
{  
/// <summary>
/// Used to build full xml property object tree from 'using' tags xml src.
/// Can also unmerge property tree back to using tags.
/// Version has upgrade functions for merged v1 scripting (implicit)
/// </summary>
public class MergePropertiesV2
{

//----------------------------------------------------------------------------
/// <summary>
/// V1 to V2 conversion overview.
/// 
/// Move PropertyContainer editorId to a PropertyContainer editorId attribute.
/// Notes why:
/// - We only ever have one editorId per property container. so no point having special
/// case work around code to generate custom properties for this. Also note that the editorId
/// has special case cooking code, is never grouped, and is never editor by the user. This
/// points to the fact that the editorId should never have been made into a property in the first place.
/// 
/// Move Component editorId from the active Component to a ComponentGroup editorId attribute.
/// Notes why:
/// - We only ever have one Component in a ComponentGroup active at a time, this is a fundamental part
/// of the design. This means that it doesn't make any sense to have different editorIds for each
/// Component within a group.
/// - When we change a Component, we explicitly don't want to change the editorId for the active Component
/// to prevent losing connections. Making the editorId explicit for a ComponentGroup makes this behavior 
/// implicit and trivial.
/// - Also see notes for PropertyContainer as these also apply to Component editorIds.
/// 
/// Move active Component value to a ComponentGroup 'activeComponent' attribute.
/// Notes why:
/// - Currently the active component is stored as a mergeable value beneath the ComponentGroup. This
/// makes it impossible to determine what Component is active without fully merging the entire PropertyContainer.
/// This also allows for the active Component in an existing PropertyContainer to change unexpectedly due to
/// a change in the original description, something that should never implicitly happen.
/// By changing the active Component to be an explicit attribute for the ComponentGroup we can prevent the active Component
/// from changing due to modifications in the PropertyContainer or Component description. We can also merge PropertyContainers 
/// and ComponentGroups faster as we will now know what Components are active, removing the need to merge inactive Components.
/// 
/// Other notes:
/// Layout elements in description need to be removed as they needlessly complicate merging and unmerging.
/// 
/// </summary>
//----------------------------------------------------------------------------

public static readonly string    kPropertyContainer_editorId =       "editorId";
public static readonly string    kPropertyContainer_version =        "containerVersion";
public static readonly string    kPropertyContainer_versionValue =   "2";
public static readonly string    kComponentGroup_editorId =          "editorId";
public static readonly string    kComponentGroup_activeComponent =   "activeComponent";
public static readonly string    kValue_isOverrideValue =            "isOverrideValue";
public static readonly string[]  kValue_Elements =                   new string[] { "Value" };
public static readonly string[]  kElements_DontMergeNodes =          new string[] { "SplineData" };

// Controls reordering operation, specifying just one of these will reorder that given type of dynamic component, specifying both will reorder both
public enum EReorderDynamicComponentGroupMode
{
   // Reorder dynamic components
   kReorderDynamic = 1,
   // Reorder dynamic prefab components
   kReorderDynamicPrefab  = 2
}

//----------------------------------------------------------------------------
/// <summary>
/// V1->V2 script properties conversion. Assumes pc_Element is a V1 PropertyContainer.
/// </summary>
public static void ConvertMergedPropertyContainerV1ToV2_InPlace(XmlDocument doc, IPropertyDataSource propertySource)
{
   XmlElement pc_Element = XmlNodeHelpers.GetDocumentRootElement( doc );

   // Start on the PropertyContainer node
   Debug.Assert(pc_Element.LocalName == Helpers.kElement_PropertyContainer, "Not a PropertyContainer element.");
   
   // Check that we're dealing with legacy properties
   Debug.Assert(CheckPropertyContainerVersion(pc_Element, "1"), "Invalid version");
   
   // Create PropertyContainer editorId
   // Move existing PropertyContainer editorId to attribute
   {
      XmlNode editorId_Node = pc_Element.SelectSingleNode("./Property[@name=\"Internal_EditorId\" and @type=\"guid\"]/Value");
      if (editorId_Node != null)
      {
         pc_Element.SetAttribute(kPropertyContainer_editorId, editorId_Node.InnerText);
         // Remove this property
         pc_Element.RemoveChild(editorId_Node.ParentNode);
      }
      else
      {
         // No editorId property, assign blank guid
         // Assertion?
         pc_Element.SetAttribute(kPropertyContainer_editorId, Guid.Empty.ToString());         
      }
   }
   
   {
      // Create ComponentGroup activeComponent attributes
      XmlNode componentGroup_Node;
      while((componentGroup_Node = pc_Element.SelectSingleNode(".//ComponentGroup[Value]")) != null)
      {         
         XmlElement componentGroup_Element = componentGroup_Node as XmlElement;         
         ConvertMergedComponentGroupV1ToV2_InPlace(componentGroup_Element);
      }   
   }

   {
      foreach(XmlElement messageObject in pc_Element.SelectNodes(".//UsingMessageObject"))
      {
         XmlElement messageObjectParent = messageObject.ParentNode as XmlElement;

         messageObjectParent.RemoveChild(messageObject);

         XmlDocument mergedMessageObject = MergeProperties.MergePropertyObject(messageObject.CreateNavigator(), propertySource);

         XmlNode newMessageObject = messageObject.OwnerDocument.ImportNode( XmlNodeHelpers.GetDocumentRootElement( mergedMessageObject ), true );
         messageObjectParent.AppendChild(newMessageObject);
      }
   }

   // Strip all depreciated 'UISettings' elements as they may still existing in old properties.
   XmlNodeHelpers.DeleteAllFromXmlNode(pc_Element, ".//UISettings");

   XmlNodeHelpers.DeleteAllFromXmlNode(pc_Element, ".//LayoutItem[@name='Internal_EditorId']");
   // Add updated version attribute
   pc_Element.SetAttribute(kPropertyContainer_version, "2");


   // Use v2 code to remerge, then we bring the values over from the converted v1 document.
   // This is because the merging code between V1 and V2 works differently (there is some bugs with evaluator value overrides from PropertyObjects not making it in the V1 code)
   // This ensures that all values stay the same.
   {
      // Create copy of converted document
      XmlDocument convertedV2Copy = new XmlDocument();
      convertedV2Copy.AppendChild(convertedV2Copy.ImportNode(pc_Element, true));
      
      // Unmerge/remerge inplace using v2 code
      Unmerge_Inplace(doc);
      Merge_Inplace(doc, propertySource);

      // Go over all value elements in remerged document (skipping over value elements underneath other value elements!
      foreach (XmlElement valueElement in doc.SelectNodes("//PropertyContainer//Property/Value[not(ancestor::Value)]"))
      {
         // Build xpath to find value element in converted document
         string usingPath = "";

         XmlElement current = valueElement;

         while(current != null)
         {
            string name = current.GetAttribute("name", "");
            string id = current.GetAttribute("id", "");

            string matchElement = "/";
            
            if( !string.IsNullOrEmpty(name) )
            {
               matchElement += "*[@name='" + name + "']";
            }
            else if( !string.IsNullOrEmpty(id))
            {
               matchElement += "*[@id='" + id + "']";
            }
            else
            {
               matchElement += current.Name;
            }

            usingPath = matchElement + usingPath;

            current = current.ParentNode as XmlElement;
         }

         // Find value element in converted document
         XmlElement convertedValueElement = convertedV2Copy.SelectSingleNode(usingPath) as XmlElement;
         if (convertedValueElement != null)
         {
            // Copy value from converted element over to our remerged value
            if (valueElement.InnerXml != convertedValueElement.InnerXml)
            {
               valueElement.InnerXml = convertedValueElement.InnerXml;
            }
         }
         else
         {
            Debug.Assert(convertedValueElement != null, "Must have matching element, otherwise the code here must be broken.");
         }
      }
   }
}

//----------------------------------------------------------------------------

public static void ConvertMergedComponentGroupV1ToV2_InPlace(XmlElement componentGroupElement)
{
   // Find the active component
   string activeComponentName = Helpers.GetActiveComponentNameFromComponentGroup(componentGroupElement);         
   // Set attribute on component group
   componentGroupElement.SetAttribute(kComponentGroup_activeComponent, activeComponentName);         
   {
      // Remove the activeComponent 'Value' element as it is now stored as an attribute
      XmlNode activeComponentValue_Node = componentGroupElement.SelectSingleNode("./Value");
      if (activeComponentValue_Node != null)
      {
         componentGroupElement.RemoveChild(activeComponentValue_Node);
      }
   }
   
   {
      // Delete all components except for active one
      string inactiveComponents_Query = String.Format("./Component[@name!=\"{0}\"]", activeComponentName);
      XmlNodeHelpers.DeleteAllFromXmlNode(componentGroupElement, inactiveComponents_Query);
   }         
   
   {
      // Move active Component editorId to ComponentGroup
      XmlNode editorId_Node = componentGroupElement.SelectSingleNode("./Component/Property[@name=\"Internal_EditorId\" and @type=\"guid\"]/Value");
      if (editorId_Node != null)
      {
         componentGroupElement.SetAttribute(kComponentGroup_editorId, editorId_Node.InnerText);
         // Remove editorId property from component
         XmlNode activeComponent_Node = componentGroupElement.SelectSingleNode("./Component");
         activeComponent_Node.RemoveChild(editorId_Node.ParentNode);
      }
      else
      {
         // No editorId property, create new guid
         // This can happen whenever we have a component group with a "None" component.
         componentGroupElement.SetAttribute(kComponentGroup_editorId, Guid.NewGuid().ToString());
      }
   }
}

//----------------------------------------------------------------------------

/// <summary>
/// Legacy support function for converting V2 properties to V1. Depreciated.
/// </summary>
public static XmlDocument ConvertSourcePropertiesV2ToV1(XPathNavigator propertiesV2_Nav)
{
   XmlDocument out_Properties = Helpers.NavigatorToDocument(propertiesV2_Nav);
   
   XmlNodeHelpers.DepthIterator iter = new XmlNodeHelpers.DepthIterator(out_Properties.FirstChild as XmlElement);
   while (iter.MoveToNextElement())
   {
      if (iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_ExitedElement)
      {
         switch (iter.Element.Name)
         {
            case "AddComponentGroup":
            {
               // Add default 'allowEmptyComponent' attrib if not present
               if (!iter.Element.HasAttribute("allowEmptyComponent"))
               {
                  iter.Element.SetAttribute("allowEmptyComponent", "false");
               }
               
               // Deal with default/active component
               string activeComponent = iter.Element.GetAttribute("activeComponent");
               iter.Element.RemoveAttribute("activeComponent");
               if (activeComponent.Length != 0)
               {
                  //iter.Element.SetAttribute("default", activeComponent); // Don't use default component to prevent active component value getting unmerged
                  // Add 'Value' element indicating active component
                  XmlElement value_Element = iter.Element.OwnerDocument.CreateElement("Value");
                  value_Element.InnerText = activeComponent;
                  iter.Element.PrependChild(value_Element);
               }
               
               // Add the 'maxSubComponents' attribute if it doesn't already exist
               if (!iter.Element.HasAttribute("maxSubComponents"))
               {
                  iter.Element.SetAttribute("maxSubComponents", "3");
               }               
               
               iter.RenameCurrentElement("UsingComponentGroup");
            }
            break;
            
            case "AddPropertyObject":
            {
               iter.RenameCurrentElement("UsingPropertyObject");
            }
            break;
            
            case "UsingPropertyObject":
            {
               iter.RenameCurrentElement("Using");
            }
            break;
            
            case "UsingComponentGroup":
            {
               iter.RenameCurrentElement("Using");
            }
            break;
            
            case "UsingComponent":
            {
               iter.RenameCurrentElement("Using");

               string id = iter.Element.GetAttribute("id");
               iter.Element.RemoveAttribute("id");
               iter.Element.SetAttribute("name", id);
            }
            break;
            
            case "UsingProperty":
            {
               iter.RenameCurrentElement("Using");
            }
            break;
            
            case "UsingEvaluatorProperty":
            {
               iter.RenameCurrentElement("Using");
            }
            break;
         }
      }
   }
   
   return out_Properties;
}

//----------------------------------------------------------------------------

public static XmlDocument Unmerge(XmlDocument node)
{
   return Unmerge_Internal( XmlNodeHelpers.GetDocumentRootElement( node ) );
}

//----------------------------------------------------------------------------

public static XmlDocument Unmerge( XmlElement node )
{
   return Unmerge_Internal( node );
}

//----------------------------------------------------------------------------

internal static XmlDocument Unmerge_Internal(XmlElement top_Element)
{
   XmlDocument unmerged = new XmlDocument();
   unmerged.AppendChild(unmerged.ImportNode(top_Element, true));
   
   Unmerge_Inplace(unmerged);
   
   return unmerged;
}

//----------------------------------------------------------------------------

public static void Unmerge_Inplace(XmlDocument doc)
{
   XmlElement node = XmlNodeHelpers.GetDocumentRootElement(doc);
   Unmerge_Inplace_Internal(ref node);
}

//----------------------------------------------------------------------------

public static void Unmerge_Inplace(ref XmlElement node)
{
   XmlElement originalElement = node;
   
   XmlElement element = originalElement;
   Unmerge_Inplace_Internal(ref element);

   if (node == originalElement)
   {
      node = element;
   }
}

//----------------------------------------------------------------------------

internal static void Unmerge_Inplace_Internal(ref XmlElement top_Element)
{
   sDiagnosticLevel = EDiagnosticLevel.kDL_None; // Show unmerge diagnostics?

   // Useful strings
   string[] kLayout_AttributesToKeep = {"name"};
   string[] kUILayout_ElementsToKeep = {"UIState", "Layout", "LayoutItem"};
   string[] kLayout_ElementsToKeep = { "UIState", "Layout", "LayoutItem" };   
   string[] kEvent_AttributesToKeep = {"type"};
   string[] kLink_AttributesToKeep = {"type"};
   
   string[] kEvaluator_AttributesToKeep = {"id"};
   string[] kProperty_AttributesToKeep = {"name"};
   string[] kEvaluatorProperty_AttributesToKeep = {"name"};
   string[] kPropertyObject_AttributesToKeep = {"id", "name"};
   string[] kComponent_AttributesToKeep = {"id"};
   string[] kComponentGroup_AttributesToKeep = {"id", "name", "activeComponent", "editorId", "prefabEditorId", "isPrefab"};
   string[] kComponentGroupDynamic_AttributesToKeep = {"id", "name", "activeComponent", "editorId", "prefabEditorId", "isPrefab"};
   string[] kPropertyContainer_AttributesToKeep = {"id", "name", "editorId", "prefabEditorId", "containerVersion"};
   string[] kMessageObject_AttributesToKeep = { "id", "name" };
   string[] kEventLinkEntry_AttributesToDelete = { "isPrefab" };
   
   
   // Now iterate over all merged properties, depth first.
   XmlNodeHelpers.DepthIterator iter = new XmlNodeHelpers.DepthIterator(top_Element);
   while (iter.MoveToNextElement())
   {
      WriteDiagnosticInfo(iter.GetDebugPathString("name", 6), "\n");
      
      if (iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement)
      {
         // Entering new Element, not yet processed it's children
         switch (iter.Element.Name)
         {
               // support for removing using property so we can unmerge message objects on events/links correctly (this only happens on prefab events/links)
            case "UsingProperty":
            case "Property":
               {
                  bool bCanRemove = IsRemovablePropertyValue(iter.Element);
                  if (bCanRemove)
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                     iter.RemoveCurrentElementAndChildren();
                  }
                  else
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Keeping: ", iter.GetDebugPathString("name", 4), "\n");
                     
                     // Remove all attributes from child Value element
                     Debug.Assert(iter.Element["Value"] != null, "Missing Value node!");
                     iter.Element["Value"].RemoveAllAttributes();
                     
                     // Remove all non-Value child elements
                     XmlNodeHelpers.RemoveChildElements_KeepNamed(iter.Element, "Value");
                     
                     // Remove unwanted attributes from property element
                     XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kProperty_AttributesToKeep);
                     iter.RenameCurrentElement("UsingProperty");
                  }               
               }
               break;
            
            case "UILayout":
               {
                  // Delete all unwanted elements
                  XmlNodeHelpers.RemoveChildElements_KeepNamed(iter.Element, kUILayout_ElementsToKeep);
               }
               break;
            
            case "Layout":
               {
                  // Delete all unwanted attributes
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kLayout_AttributesToKeep);
                  // Delete all unwanted elements
                  XmlNodeHelpers.RemoveChildElements_KeepNamed(iter.Element, kLayout_ElementsToKeep);
               }
               break;
            
            case "Messages":
               {
                  // No need to keep these, delete all.
                  // We could add a 'DynamicMessages' later if we wanted, could be useful for script languages.
                  WriteDiagnosticInfo(EDiagnosticLevel.kDL_Max, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                  iter.RemoveCurrentElementAndChildren();
               }
               break;            

            case "Methods":
               {
                  // Delete all
                  WriteDiagnosticInfo(EDiagnosticLevel.kDL_Max, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                  iter.RemoveCurrentElementAndChildren();
               }
               break;            

            case "Event":
               {
                  // Delete all unwanted attributes
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kEvent_AttributesToKeep);
                  // Delete all unwanted elements
                  XmlNodeHelpers.RemoveChildElements_KeepNamed(iter.Element, "EventEntry");
               }
               break;            

            case "Link":
               {
                  // Delete all unwanted attributes
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kLink_AttributesToKeep);
                  // Delete all unwanted elements
                  XmlNodeHelpers.RemoveChildElements_KeepNamed(iter.Element, "LinkEntry");
               }
               break;                        

            default:
               {
                  WriteDiagnosticInfo(EDiagnosticLevel.kDL_Max, "Ignoring: ", iter.GetDebugPathString("name", 4), "\n");
               }
               break;
         }      
      }
      
      if (iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_ExitedElement)
      {
         // Exiting Element, have processed all children
         switch (iter.Element.Name)
         {
            case "PropertyContainer":
               {
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kPropertyContainer_AttributesToKeep);
                  iter.RenameCurrentElement("UsingPropertyContainer");
               }
               break;

            case "ComponentGroup":
               {
                  // Is this a dynamic component group?
                  bool bIsDynamicComponent = Tools.Common.Misc.ParseBool(iter.Element.GetAttribute("dynamicComponentGroup"));
                  
                  // NOTE: V2 keeps the 'id' attribute as is, where V1 would rename this to 'name' if the component wasn't dynamic.
                  if (bIsDynamicComponent)               
                  {
                     XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kComponentGroupDynamic_AttributesToKeep);
                     if (Helpers.IsComponentGroupPrefab(iter.Element))
                     {
                        // Remove active component attribute from prefab component group, can't override active component currently.
                        iter.Element.RemoveAttribute(kComponentGroup_activeComponent);
                        
                        // Remove name from prefab component group, we always inherit the name from the prefab.
                        iter.Element.RemoveAttribute("name");

                        iter.RenameCurrentElement("UsingComponentGroupDynamic");
                     }
                     else
                     {
                        iter.RenameCurrentElement("AddComponentGroupDynamic");
                     }
                  }
                  else
                  {
                     XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kComponentGroup_AttributesToKeep);
                     iter.RenameCurrentElement("UsingComponentGroup");                              
                  }
               }
               break;

            case "Component":
               {
                  // NOTE: V2 keeps the 'id' attribute as is, where V1 would rename this to 'name'
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kComponent_AttributesToKeep);
                  iter.RenameCurrentElement("UsingComponent");
               }
               break;

            case "PropertyObject":
               {
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kPropertyObject_AttributesToKeep);
                  iter.RenameCurrentElement("UsingPropertyObject");
               }
               break;

            case "MessageObject":
               {
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kMessageObject_AttributesToKeep);
                  iter.RenameCurrentElement("UsingMessageObject");
               }
               break;

            case "EvaluatorProperty":
               {
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kEvaluatorProperty_AttributesToKeep);
                  iter.RenameCurrentElement("UsingEvaluatorProperty");
               }
               break;

            case "Evaluator":
               {
                  // Rename evaluator
                  // NOTE: Replacement of default Evaluator is now determined at merge.
                  XmlNodeHelpers.RemoveAttributes_KeepNamed(iter.Element, kEvaluator_AttributesToKeep);
                  iter.RenameCurrentElement("UsingEvaluator");
               }
               break;

            case "UILayout":
               {
                  // Delete if empty
                  if (!XmlNodeHelpers.HasChildElements(iter.Element))
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                     iter.RemoveCurrentElementAndChildren();
                  }
                  else
                  {
                     iter.RenameCurrentElement("UsingUILayout");
                  }
               }
               break;
            
            case "Layout":
               {
                  // Delete if empty
                  if (!XmlNodeHelpers.HasChildElements(iter.Element))
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                     iter.RemoveCurrentElementAndChildren();
                  }
               }
               break;            

            case "LayoutItem":
               {
                  // Delete if empty
                  if (!XmlNodeHelpers.HasChildElements(iter.Element))
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                     iter.RemoveCurrentElementAndChildren();
                  }
               }
               break; 
            
            case "UIState":
               {
                  if( !XmlNodeHelpers.HasChildElements(iter.Element) )
                  {
                     string defaultValue = iter.Element.GetAttribute("default");
                     if (iter.Element.InnerText == defaultValue)
                     {
                        iter.RemoveCurrentElementAndChildren();
                     }
                  }
               }
               break;

            case "EventEntry":
               {
                  if (Helpers.IsEventOrLinkEntryPrefab(iter.Element))
                  {
                     // Remove message object at this point if it's empty, this needs to be done here because we want to do this ONLY for PREFAB event entries.
                     XmlElement messageObject = iter.Element["UsingMessageObject"];
                     if( messageObject != null && !messageObject.HasChildNodes )
                     {
                        iter.Element.RemoveChild(messageObject);
                     }

                     iter.RenameCurrentElement("UsingEventEntry");
                     XmlNodeHelpers.RemoveAttributes_DeleteNamed(iter.Element, kEventLinkEntry_AttributesToDelete);
                  }
               }
               break;

            case "Event":
               {
                  // Delete if empty
                  if (!XmlNodeHelpers.HasChildElements(iter.Element))
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                     iter.RemoveCurrentElementAndChildren();
                  }
               }
               break;

            case "Events":
               {
                  // Delete if empty
                  if (!XmlNodeHelpers.HasChildElements(iter.Element))
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                     iter.RemoveCurrentElementAndChildren();
                  }
               }
               break;

            case "LinkEntry":
               {
                  if (Helpers.IsEventOrLinkEntryPrefab(iter.Element))
                  {
                     iter.RenameCurrentElement("UsingLinkEntry");
                     XmlNodeHelpers.RemoveAttributes_DeleteNamed(iter.Element, kEventLinkEntry_AttributesToDelete);
                  }
               }
               break;

            case "Link":
               {
                  // Delete if empty
                  if (!XmlNodeHelpers.HasChildElements(iter.Element))
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                     iter.RemoveCurrentElementAndChildren();
                  }
               }
               break;

            case "Links":
               {
                  // Delete if empty
                  if (!XmlNodeHelpers.HasChildElements(iter.Element))
                  {
                     WriteDiagnosticInfo(EDiagnosticLevel.kDL_High, "Deleting: ", iter.GetDebugPathString("name", 4), "\n");
                     iter.RemoveCurrentElementAndChildren();
                  }
               }
               break;

            default:
               {
                  WriteDiagnosticInfo(EDiagnosticLevel.kDL_Max, "Ignoring: ", iter.GetDebugPathString("name", 4), "\n");
               }
               break;
         }      
      }      
   }

   top_Element = iter.GetTopLevelElement();
}

//----------------------------------------------------------------------------

public static XmlDocument Merge( XmlDocument doc, IPropertyDataSource propertySource )
{
   return Merge_Internal( XmlNodeHelpers.GetDocumentRootElement( doc ), propertySource );
}

//----------------------------------------------------------------------------

public static XmlDocument Merge(XmlElement node, IPropertyDataSource propertySource)
{
   return Merge_Internal(node, propertySource);
}

//----------------------------------------------------------------------------

internal static XmlDocument Merge_Internal(XmlElement topElement, IPropertyDataSource propertySource)
{
   XmlDocument mergedDoc = new XmlDocument();
   
   mergedDoc.AppendChild(mergedDoc.ImportNode(topElement, true));
   if( !Merge_Inplace(mergedDoc, propertySource) )
   {
      return null;
   }
   
   return mergedDoc;
}

//----------------------------------------------------------------------------

public static bool Merge_Inplace(XmlDocument document, IPropertyDataSource propertySource)
{
   XmlElement node = XmlNodeHelpers.GetDocumentRootElement(document);
   return Merge_Inplace_Internal(ref node, propertySource);
}

//----------------------------------------------------------------------------

public static XmlDocument InstantiateMergedPropertyContainer(IPropertyDataSource propertySource, String id, String name)
{
   XmlDocument mergedData = new XmlDocument();
   mergedData.LoadXml( String.Format( "<{0}{1} {2}={3} {4}={5} {6}={7} />",
      Misc.Helpers.kElement_Using,
      Misc.Helpers.kElement_PropertyContainer,
      Misc.Helpers.kPropertyContainer_Name,
      XmlNodeHelpers.AsQuotedXml( name ),
      Misc.Helpers.kProperty_Id,
      XmlNodeHelpers.AsQuotedXml( id ),
      kPropertyContainer_version,
      XmlNodeHelpers.AsQuotedXml( kPropertyContainer_versionValue ) ) );

   // Convert into dynamic components (merge, convert to dynamic, unmerge)
   // We convert property objects into dynamic components so that we don't have to worry about modifications to the 
   // underlying property objects getting propagated to already placed objects.
   Merge_Inplace(mergedData, propertySource);
   
   return mergedData;
}
//----------------------------------------------------------------------------

public static bool Merge_Inplace(ref XmlElement node, IPropertyDataSource propertySource)
{
   XmlElement originalElement = node;

   XmlElement element = originalElement;

   bool result = Merge_Inplace_Internal(ref element, propertySource);

   if (node == originalElement)
   {
      node = element;
   }

   return result;
}

//----------------------------------------------------------------------------

internal static bool Merge_Inplace_Internal(ref XmlElement top_Element, IPropertyDataSource propertySource)
{
   UpdatePropertiesToLatestVersion(ref top_Element, propertySource);

   sDiagnosticLevel = EDiagnosticLevel.kDL_None; // Show merge diagnostics?

   // Mark all value elements as "isOverrideValue".
   MarkOverideValues(top_Element);

   // Now iterate over all merged properties, depth first.
   XmlNodeHelpers.DepthIterator iter = new XmlNodeHelpers.DepthIterator(top_Element);
   while (iter.MoveToNextElement())
   {
      WriteDiagnosticInfo(iter.GetDebugPathString("name", 6), "\n");

      XmlElement element = iter.Element;
      
      if (iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement)
      {
         // Entering new Element, not yet processed it's children
         switch (element.Name)
         {            
            case "UsingPropertyContainer":
            {
               string id = element.GetAttribute("id");
               XmlElement sourcePropertyContainer = propertySource.FindPropertyContainerById(id, ESourceVersion.kSourceVersion_2).UnderlyingObject as XmlElement;

               iter.RenameCurrentElement(sourcePropertyContainer.Name);
               MergeAttributes(iter.Element, sourcePropertyContainer);
               MergeChildren(iter.Element, sourcePropertyContainer);
               EnsureEditorIdExists(iter.Element);
            }
            break;

            case "UsingComponent":
               {
                  string groupId = (element.ParentNode as XmlElement).GetAttribute("id");
                  string id = element.GetAttribute("id");

                  iter.RenameCurrentElement(Helpers.kElement_Component);

                  if( id != "None" )
                  {
                     XPathNavigator foundComponentNav = propertySource.FindComponentByGroupAndId(groupId + ":" + id, ESourceVersion.kSourceVersion_2);
                     if( foundComponentNav == null )
                     {
                        return false;
                     }

                     XmlElement sourceComponent = foundComponentNav.UnderlyingObject as XmlElement;

                     MergeAttributes(iter.Element, sourceComponent);
                     MergeChildren(iter.Element, sourceComponent);
                  }
               }
               break;

            case "AddPropertyObject":
               {
                  string id = element.GetAttribute("id");
                  
                  XPathNavigator foundPropertyObjectNav = propertySource.FindPropertyObjectById(id, ESourceVersion.kSourceVersion_2);
                  
                  if( foundPropertyObjectNav == null )
                  {
                     return false;
                  }

                  XmlElement propertyObject = foundPropertyObjectNav.UnderlyingObject as XmlElement;
                  
                  iter.RenameCurrentElement(propertyObject.Name);
                  MergeAttributes(iter.Element, propertyObject);
                  MergeChildren(iter.Element, propertyObject);
               }
               break;

            case "AddComponentGroup":
            case "AddComponentGroupDynamic":
               {
                  if (iter.Element.Name == "AddComponentGroupDynamic")
                  {
                     iter.Element.SetAttribute(Helpers.kComponentGroup_Dynamic, "true");
                  }

                  iter.RenameCurrentElement("ComponentGroup");
                  EnsureEditorIdExists(iter.Element);

                  // Add "UsingComponent" if it doesn't exist.
                  if( iter.Element["UsingComponent"] == null )
                  {
                     string activeComponent = iter.Element.GetAttribute(kComponentGroup_activeComponent);

                     XmlElement newUsingComponent = iter.Element.OwnerDocument.CreateElement("UsingComponent");
                     newUsingComponent.SetAttribute(Helpers.kComponent_Id, activeComponent);
                     
                     iter.Element.AppendChild(newUsingComponent);
                  }
               }
               break;
            
            case "UsingEvaluator":
               {
                  string id = element.GetAttribute("id");
                  iter.RenameCurrentElement(Helpers.kElement_Evaluator);
                  if( id != "None" )
                  {
                     XPathNavigator foundEvaluatorNav = propertySource.FindEvaluatorById(id, ESourceVersion.kSourceVersion_2);
                     if( foundEvaluatorNav == null )
                     {
                        return false;
                     }

                     XmlElement evaluator = foundEvaluatorNav.UnderlyingObject as XmlElement;
                     MergeAttributes(iter.Element, evaluator);
                     MergeChildren(iter.Element, evaluator);
                  }
               }
               break;

            case "UsingMessageObject":
               {
                  string messageObjectId = element.GetAttribute(Helpers.kMessage_id);

                  iter.RenameCurrentElement(Helpers.kElement_MessageObject);

                  XPathNavigator messageObjectNav = propertySource.FindMessageObjectById(messageObjectId, ESourceVersion.kSourceVersion_2);
                  if( messageObjectNav != null )
                  {
                     XmlElement messageObject = messageObjectNav.UnderlyingObject as XmlElement;

                     MergeAttributes(iter.Element, messageObject);
                     MergeChildren(iter.Element, messageObject);
                  }
                  else
                  {
                     iter.Element.SetAttribute("type", "NoProperties");
                  }
               }
               break;

            case "UsingComponentGroupDynamic":
            case "UsingComponentGroup":
            case "UsingPropertyObject":
            case "UsingEvaluatorProperty":
            case "UsingProperty":
               {
                  // If there is using tags at this stage, it means they didn't have anything to be merged into.
                  // This can happen when one of these types is overridden in an object but it got removed from the definition.
                  // It also happens very commonly when switching between different components.
                  // The solution is to just delete it.
                  // NOTE: This is a single location where we can loose overrides, so it would be easy to add some kind of logging/user notification at this point!
                  iter.RemoveCurrentElementAndChildren();
               }
               break;

            case "Methods":
               // Nothing needs to be done here.
               iter.SkipChildren();
               break;

            default:
               {
                  WriteDiagnosticInfo(EDiagnosticLevel.kDL_Max, "Ignoring: ", iter.GetDebugPathString("name", 4), "\n");
               }
               break;
         }      
      }
      else if (iter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_ExitedElement)
      {
         // Exiting Element, have processed all children
         switch (element.Name)
         {
            case "PropertyContainer":
               {
                  ReorderDynamicComponentGroups(element, EReorderDynamicComponentGroupMode.kReorderDynamic | EReorderDynamicComponentGroupMode.kReorderDynamicPrefab);
                  MergeUILayout(element);
               }
               break;

            case "EvaluatorProperty":
               {
                  XmlElement evaluatorElement = element["Evaluator"];
                  if( evaluatorElement == null )
                  {
                     // No evaluator present, add "None" evaluator.
                     evaluatorElement = element.OwnerDocument.CreateElement(Helpers.kElement_Evaluator);
                     evaluatorElement.SetAttribute(Helpers.kEvaluator_Id, "None");
                     element.AppendChild(evaluatorElement);
                  }
                  
                  // Add active evaluator attribute if not present
                  if (element.GetAttribute(Helpers.kEvaluatorProperty_ActiveEvaluator) == String.Empty)
                  {
                     string activeEvaluator = evaluatorElement.GetAttribute(Helpers.kEvaluator_Id);
                     element.SetAttribute(Helpers.kEvaluatorProperty_ActiveEvaluator, activeEvaluator);
                  }
               }
               break;

            case "Evaluator":
            case "Component":
            case "PropertyObject":
               MergeUILayout(element);
               break;

            case "MessageObject":
               // If message object doesn't have any children we shouldn't be adding a UILayout either.
               if(element.HasChildNodes)
               {
                  MergeUILayout(element);
               }
               break;

            case "Property":
               {
                  // We're exiting the Property tag, this means any data that could have possible overridden this property has already been applied.
                  // If we do not have a value element, we create an empty one here.
                  if( element["Value"] == null )
                  {
                     XmlElement valueElement = element.OwnerDocument.CreateElement(Helpers.kElement_Value);
                     valueElement.SetAttribute("default", "");
                     element.AppendChild(valueElement);
                  }
               }
               break;
               
            case "Value":
               {
                  // We're exiting the value element, this is where we update the default value (without override), because anything that could have changed this value has already been applied.
                  UpdateDefaultValue(element, null);
               }
               break;

            default:
               {
                  WriteDiagnosticInfo(EDiagnosticLevel.kDL_Max, "Ignoring: ", iter.GetDebugPathString("name", 4), "\n");
               }
               break;
         }      
      }      
   }

   top_Element = iter.GetTopLevelElement();

   return true;
}

public static void MergeMessagesElementsOrLinks_Inplace( XmlElement mergingInto, XmlElement mergeSource )
{
   MergeAttributes( mergingInto, mergeSource );
   MergeChildren( mergingInto, mergeSource );
}

//----------------------------------------------------------------------------
/// <summary>
/// This function marks all "Value" elements as "isOverrideValue" = true
/// </summary>
public static void MarkOverideValues(XmlElement element)
{
   XmlNodeHelpers.DepthIterator valueIter = new XmlNodeHelpers.DepthIterator(element);
   while (valueIter.MoveToNextElement())
   {
      if (valueIter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement)
      {
         if (Array.IndexOf(kValue_Elements, valueIter.Element.Name) != -1)
         {
            valueIter.Element.SetAttribute(kValue_isOverrideValue, "true");
         }
      }
   }
}

//----------------------------------------------------------------------------

private static void UpdatePropertiesToLatestVersion(ref XmlElement top_Element, IPropertyDataSource propertySource)
{
   if ( top_Element.Name != "UsingPropertyContainer")
   {
      return;
   }
   
   // V1 to V2 conversion
   if (top_Element.GetAttribute(kPropertyContainer_version) != "2")
   {
      XmlDocument doc = MergeProperties.MergePropertyObject(top_Element.CreateNavigator(), propertySource);
      ConvertMergedPropertyContainerV1ToV2_InPlace(doc, propertySource);
      Unmerge_Inplace(doc);
      
      XmlNode newTopElement = top_Element.OwnerDocument.ImportNode(Tools.Common.XmlNodeHelpers.GetDocumentRootElement( doc ), true);
      XmlNode parentNode = top_Element.ParentNode;
      parentNode.RemoveChild(top_Element);
      parentNode.AppendChild(newTopElement);
      
      top_Element = newTopElement as XmlElement;
   }   
}

//----------------------------------------------------------------------------

private static void EnsureEditorIdExists(XmlElement xmlElement)
{
   // bCreateComponentEditorId defaults to true if attribute not present
   bool bCreateEditorId = true;

   string createEditorId = xmlElement.GetAttribute(Helpers.kComponent_CreateEditorId, String.Empty);
   if (createEditorId.Length != 0)
   {
      bCreateEditorId = bool.Parse(createEditorId);
   }

   if (bCreateEditorId && xmlElement.GetAttributeNode(kComponentGroup_editorId, "") == null )
   {
      xmlElement.SetAttribute(kComponentGroup_editorId, Guid.NewGuid().ToString());
   }
}

//----------------------------------------------------------------------------

private static XmlElement MergeUILayout(XmlElement element)
{
   // build list of all items in object
   List<string> itemsInObject = new List<string>();
   
   foreach(XmlNode node in element.ChildNodes)
   {
      XmlElement nodeElement = node as XmlElement;
      if( nodeElement != null )
      {
         switch(nodeElement.Name)
         {
            case Helpers.kElement_ComponentGroup:
            case Helpers.kElement_Property:
            case Helpers.kElement_PropertyObject:
            case Helpers.kElement_EvaluatorProperty:
               {
                  string name = nodeElement.GetAttribute(Helpers.kProperty_Name);
                  itemsInObject.Add(name);
               }
               break;

            default:
               break;
         }
      }
   }

   // Retrieve existing UILayout element or create it if it doesn't exist.
   XmlElement uiLayout = element[MergeLayout.kElement_UILayout];
   if( uiLayout == null )
   {
      uiLayout = element.OwnerDocument.CreateElement(MergeLayout.kElement_UILayout);
   }

   // Go over all items in layout and remove items we find in the layout from the "layout items to be added" list (this is initialized to contain all items for the object).
   // We also remove items that no longer exist in the object in the process.
   List<string> layoutItemsToBeAdded = new List<string>(itemsInObject);
   XmlNodeHelpers.DepthIterator layoutIter = new XmlNodeHelpers.DepthIterator(uiLayout);
   while(layoutIter.MoveToNextElement())
   {
      if(layoutIter.IteratorInfo == XmlNodeHelpers.DepthIterator.EIteratorInfo.kII_EnteredElement && layoutIter.Element.Name == MergeLayout.kElement_LayoutItem)
      {
         string layoutItemName = layoutIter.Element.GetAttribute(MergeLayout.kItem_Name);

         // Check if item exists in object
         int foundIndex = layoutItemsToBeAdded.IndexOf(layoutItemName);
         if (foundIndex != -1)
         {
            // Remove from list of items to be added.
            layoutItemsToBeAdded.RemoveAt(foundIndex);
         }
         else
         {
            // If it doesn't exist, we remove the layout item.
            layoutIter.RemoveCurrentElementAndChildren();
         }
         
         // Don't care about children of layout items.
         layoutIter.SkipChildren();
      }
   }

   // Now layoutItemsToBeAdded only contains items that don't exist in the layout
   foreach (string newLayoutItemName in layoutItemsToBeAdded)
   {
      XmlElement layoutItem = uiLayout.OwnerDocument.CreateElement(MergeLayout.kElement_LayoutItem);
      layoutItem.SetAttribute(MergeLayout.kItem_Name, newLayoutItemName);
      uiLayout.AppendChild(layoutItem);
   }

   // Merge in any overrides
   XmlElement usingUiLayoutElement = element["UsingUILayout"];
   if( usingUiLayoutElement != null )
   {
      MergeNode(ref usingUiLayoutElement, uiLayout);

      if (uiLayout.ParentNode != null )
      {
         uiLayout.ParentNode.RemoveChild(uiLayout);
      }
   }
   else
   {
      element.AppendChild(uiLayout);
   }

   return uiLayout;
}

//----------------------------------------------------------------------------

public static void ReorderDynamicComponentGroups(XmlElement element, EReorderDynamicComponentGroupMode mode)
{
   // We need to reorder all dynamic component groups to go to the end of the children.
   // This is necessary because the Merging of property container caused all items to be reordered according to the property container definition.
   // Because the property container source type does not contain dynamic component groups (and these always go last) we move them to the end of list (while maintaining their relative ordering)

   int current = 0;
   int childCount = element.ChildNodes.Count;

   while (current < childCount)
   {
      XmlElement child = element.ChildNodes[current] as XmlElement;

      bool didReorder = false;

      if (child != null && child.Name == Helpers.kElement_ComponentGroup )
      {
         if( child.GetAttribute(Helpers.kComponentGroup_Dynamic) == "true" )
         {
            bool isPrefabComponent = Helpers.IsComponentGroupPrefab(child);

            bool reorderPrefab = (mode & EReorderDynamicComponentGroupMode.kReorderDynamicPrefab) != 0;
            bool reorderDynamic = (mode & EReorderDynamicComponentGroupMode.kReorderDynamic) != 0;

            bool shouldReorder = (reorderPrefab && isPrefabComponent) || (reorderDynamic && !isPrefabComponent);

            if (shouldReorder)
            {
               XmlNode parent = child.ParentNode;
               parent.RemoveChild(child);
               parent.AppendChild(child);
               childCount--;
               didReorder = true;
            }
         }
      }

      if( !didReorder )
      {
         current++;
      }
   }
}

//----------------------------------------------------------------------------

private static void MergeChildren(XmlElement destOwner, XmlElement sourceOwner)
{
   switch(destOwner.Name)
   {
      case Helpers.kElement_EvaluatorProperty:
         {
            XmlElement sourceChild = Tools.Common.XmlNodeHelpers.GetFirstChildElement( sourceOwner );

            if(sourceChild != null )
            {
               string sourceId = sourceChild.GetAttribute(Helpers.kEvaluator_Id);

               XmlElement destChild = Tools.Common.XmlNodeHelpers.GetFirstChildElement( destOwner );
               if (destChild != null)
               {
                  string destId = destChild.GetAttribute(Helpers.kEvaluator_Id);
                  if (sourceId == destId)
                  {
                     // The ids match, we can merge the data.
                     MergeNode(ref destChild, sourceChild);
                  }
                  else
                  {
                     // The evaluator id's don't match, we don't merge the source data in.
                     return;
                  }
               }

               if (destChild == null)
               {
                  destChild = destOwner.OwnerDocument.ImportNode(sourceChild, true) as XmlElement;
                  destOwner.AppendChild(destChild);
               }
            }
         }
         break;

      default:
         {
            foreach (XmlNode sourceChildNode in sourceOwner.ChildNodes)
            {
               XmlElement sourceChild = sourceChildNode as XmlElement;
             
               if (sourceChild == null)
                  continue;

               XmlElement destChild = FindAndPrepareMatchingChildForMerge(destOwner, sourceChild);

               if (destChild != null)
               {
                  destOwner.RemoveChild(destChild);
                  destOwner.AppendChild(destChild);

                  MergeNode(ref destChild, sourceChild);
               }
               else
               {
                  destChild = destOwner.OwnerDocument.ImportNode(sourceChild, true) as XmlElement;
                  destOwner.AppendChild(destChild);
               }
            }
         }
         break;
   }
}

//----------------------------------------------------------------------------

private static XmlElement FindAndPrepareMatchingChildForMerge(XmlElement destElement, XmlElement sourceChild)
{
   switch (sourceChild.Name)
   {
      case "Event":
      case "Link":
      case "Message":
         // find matching event/link/message based on type attribute
         {
            string type = sourceChild.GetAttribute("type");
            foreach (XmlNode destChildNode in destElement)
            {
               XmlElement destChild = destChildNode as XmlElement;
               if (destChild != null)
               {
                  if ( destChild.Name == sourceChild.Name && destChild.GetAttribute("type") == type)
                  {
                     return destChild;
                  }
               }
            }

            return null;
         }

      case "DynamicItems":
         {
            foreach ( XmlNode destChildNode in destElement )
            {
               XmlElement destChild = destChildNode as XmlElement;
               if ( destChild != null )
               {
                  if ( destChild.Name == sourceChild.Name &&
                     XmlNodeHelpers.AreElementAttributesEquivelent( sourceChild, destChild ) )
                  {
                     return destChild;
                  }
               }
            }

            return null;
         }

      case "EventEntry":
      case "LinkEntry":
         // find matching event/link entry based on id, this will also match an equivalent prefabId
         {
            string id = sourceChild.GetAttribute("id");
            foreach (XmlNode destChildNode in destElement )
            {
               XmlElement destChild = destChildNode as XmlElement;
               if (destChild != null)
               {
                  if (destChild.GetAttribute("prefabId") == id)
                  {
                     return destChild;
                  }
                  else if (destChild.GetAttribute("id") == id)
                  {
                     return destChild;
                  }
               }
            }
            return null;
         }

      case "MessageObject":
         {
            // We currently do not want to support overriding message objects in connections, as such we need to make sure we don't accept overriding the message.
            string id = sourceChild.GetAttribute("id");
            foreach(XmlElement destChild in destElement.ChildNodes)
            {
               switch(destChild.Name)
               {
                  case "UsingMessageObject":
                  case "MessageObject":
                     destChild.SetAttribute("id", id);
                     return destChild;
               }
            }
            
            return null;
         }

      case "UsingMessageObject":
         {
            Debug.Assert(false, "We shouldn't have this tag at this point.");
            return null;
         }

      case "ComponentGroup":
         {
            string sourceEditorId = sourceChild.GetAttribute(MergePropertiesV2.kComponentGroup_editorId);
            if( !String.IsNullOrEmpty(sourceEditorId) )
            {
               foreach(XmlElement destChildElement in destElement.ChildNodes)
               {
                  switch(destChildElement.Name)
                  {
                     case "UsingComponentGroup":
                     case "UsingComponentGroupDynamic":
                     case "ComponentGroup":
                        {
                           string destPrefabEditorId = destChildElement.GetAttribute("prefabEditorId");
                           if( sourceEditorId == destPrefabEditorId )
                           {
                              return destChildElement;
                           }
                        }
                        break;
                  }
               }
               return null;
            }
            else
            {
               return FindMatchingChildDefaultCodePath(destElement, sourceChild);
            }
         }


      case "Component":
         {
            XmlElement destComponent = destElement["UsingComponent"];

            if (destComponent == null)
            {
               destComponent = destElement["Component"];
            }

            if( destComponent != null )
            {
               // Remove id attribute from component, we're merging this over from the source component.
               destComponent.RemoveAttribute("id");
            }
            return destComponent;
         }

      default:
         {
            return FindMatchingChildDefaultCodePath(destElement, sourceChild);
         }
   }
}

//----------------------------------------------------------------------------

private static XmlElement FindMatchingChildDefaultCodePath(XmlElement destElement, XmlElement sourceChild)
{
   string sourceId = sourceChild.GetAttribute("id");
   string sourceName = sourceChild.GetAttribute("name");

   if (!string.IsNullOrEmpty(sourceId) || !string.IsNullOrEmpty(sourceName))
   {
      return FindChildElementByNameAndId(destElement, sourceId, sourceName);
   }

   return destElement[sourceChild.Name];
}

//----------------------------------------------------------------------------

public static void MergeNode(ref XmlElement destElement, XmlElement sourceElement)
{
   destElement = XmlNodeHelpers.RenameElement(destElement, sourceElement.Name);

   MergeAttributes(destElement, sourceElement);

   UpdateDefaultValue(destElement, sourceElement);

   if (Array.IndexOf(kElements_DontMergeNodes, destElement.Name) == -1)
   {
      MergeChildren(destElement, sourceElement);
   }
}

//----------------------------------------------------------------------------

public static void UpdateDefaultValue(XmlElement destElement, XmlElement sourceElement)
{
   if( Array.IndexOf(kValue_Elements, destElement.Name) != -1 )
   {
      XmlAttribute isDefaultValueAttr = destElement.GetAttributeNode(kValue_isOverrideValue);
      destElement.RemoveAttributeNode(isDefaultValueAttr);

      XmlAttribute defaultAttr = destElement.GetAttributeNode("default");

      if (defaultAttr == null)
      {
         if (isDefaultValueAttr != null )
         {
            string sourceValue = (sourceElement != null) ? sourceElement.InnerText : "";
            destElement.SetAttribute("default", sourceValue);
         }
         else
         {
            destElement.SetAttribute("default", destElement.InnerText);
         }
      }
   }
}

//----------------------------------------------------------------------------

public static XmlElement FindChildElementByNameAndId(XmlElement element, string id, string name)
{
   foreach(XmlElement child in element)
   {
      string childId = "";
      string childName = "";

      foreach(XmlAttribute childAttr in child.Attributes)
      {
         if (childAttr.Name == "id")
         {
            childId = childAttr.Value;
         }
         else if (childAttr.Name == "name")
         {
            childName = childAttr.Value;
         }
      }

      if (childId == id && childName == name)
         return child;
   }
   
   return null;
}

//----------------------------------------------------------------------------

public static void MergeAttributes(XmlElement destElement, XmlElement sourceElement)
{
   foreach(XmlAttribute attribute in sourceElement.Attributes)
   {
      if( !destElement.HasAttribute(attribute.LocalName, ""))
      {
         destElement.SetAttribute(attribute.LocalName, attribute.Value);
      }
   }
}

//----------------------------------------------------------------------------

internal static bool IsRemovablePropertyValue(XmlElement property_Element)
{
   bool isUsingProperty = property_Element.Name == "UsingProperty";

   Debug.Assert(property_Element.Name == "Property" || isUsingProperty, "Not a Property element.");
   
   XmlElement value_Element = property_Element["Value"];
   if (value_Element == null)
   {
      // Corner case, show diagnostic message.
      WriteDiagnosticInfo("Property: ", property_Element.GetAttribute("name"), " has no Value elements.\n");
      
      // No Value element, can remove.
      return true;
   }
   
   // Check to see if we want to prevent an unmerge
   bool bKeepInitialValue = Tools.Common.Misc.ParseBool(property_Element.GetAttribute(Helpers.kProperty_Value_KeepInitialValue));
   string propertyType = property_Element.GetAttribute(Helpers.kProperty_Type);
   
   string currentValue = value_Element.InnerText;
   
   // Default value can be an empty string in some cases so types that need to parse the string must deal with that case.
   // Note that an empty string is valid for asset properties.
   string defaultValue = value_Element.GetAttribute(Helpers.kProperty_Value_Default);   
   
   bool bIsSameAsDefaultValue = false;
   try
   {
      switch (propertyType)
      {
         case Helpers.kProperty_Type_float:
            bIsSameAsDefaultValue = (defaultValue.Length != 0) && (Double.Parse(defaultValue) == Double.Parse(currentValue));
            break;

         case Helpers.kProperty_Type_bool:
            bIsSameAsDefaultValue = (defaultValue.Length != 0) && (Tools.Common.Misc.ParseBool(defaultValue) == Tools.Common.Misc.ParseBool(currentValue));
            break;

         case Helpers.kProperty_Type_string:
            bIsSameAsDefaultValue = (defaultValue == currentValue);
            break;
            
         case Helpers.kProperty_Type_int:
            bIsSameAsDefaultValue = (defaultValue == currentValue);
            break;
            
         case Helpers.kProperty_Type_enum:
            bIsSameAsDefaultValue = (defaultValue == currentValue);
            break;
            
         case Helpers.kProperty_Type_color:
            bIsSameAsDefaultValue = (defaultValue == currentValue);
            break;
            
         case Helpers.kProperty_Type_evaluatorContainer:
            // Evaluator containers don't unmerge, they just keep their properties
            bIsSameAsDefaultValue = (currentValue.Length == 0) && (!XmlNodeHelpers.HasChildElements(property_Element));
            break;
         case Helpers.kProperty_Type_custom:
            // Custom properties don't unmerge unless empty
            bIsSameAsDefaultValue = (currentValue.Length == 0) && (!XmlNodeHelpers.HasChildElements(property_Element));
            break;

         default:
            // When we're checking "UsingProperty" we don't have types.
            if ( !isUsingProperty )
            {
               Debug.Assert(false, String.Format("Unknown property type: {0} / default:{1} / value:{2}", propertyType, defaultValue, currentValue));
            }
            bIsSameAsDefaultValue = (defaultValue == currentValue);
            break;
      }
   }
   catch (FormatException)
   {
      bIsSameAsDefaultValue = false;
      Debug.Assert(false, String.Format("Unable to parse merge string values for default:{0} or value:{1} for type:{2}.", defaultValue, currentValue, propertyType));
   }
   
   if (bIsSameAsDefaultValue && !bKeepInitialValue)
   {
      // Default value matches object value, we can remove property      
      return true;
   }
   
   // Can't remove this property
   return false;
}

//----------------------------------------------------------------------------

private static bool IsReplaceableEvaluator(XmlElement evaluator_Element)
{
   Debug.Assert(evaluator_Element.Name == "Evaluator", "Not an Evaluator element.");
   
   // We only want to remove evaluators that are the same as the default and are 'None' (empty type)
   // This seems to be the only case where you'd want to override the default value (i.e. update it to something more useful)
   string evaluatorId = evaluator_Element.GetAttribute(Helpers.kEvaluator_Id);
   string evaluatorId_Default = evaluator_Element.GetAttribute(Helpers.kEvaluator_Id_Default);
   
   bool bIsDefaultEvaluatorForEvaluatorProperty = (evaluatorId != evaluatorId_Default);
   if (!bIsDefaultEvaluatorForEvaluatorProperty)
   {
      // Someone's explicitly changed this evaluator type from it's default, don't remove it.
      // They also could have omitted the 'default' definition, review later.
      return false;
   }
   
   // Check for the default being none instead of id, as default might be an empty string (i.e. not default="None")
   // If the default isn't 'None' then we aren't going to remove it.
   if (evaluatorId_Default == "None")
   {
      // Empty evaluator, it's removable.
      Debug.Assert(evaluator_Element.HasChildNodes == false, "'None' evaluator should not have child nodes!");
      return true;
      
   }
   
   // Nope, can't remove this evaluator
   return false;
}

//----------------------------------------------------------------------------

public static bool CheckPropertyContainerVersion(XmlNode pc_Element, string requiredVersion)
{
   XmlAttribute pcVersion_Attrib = pc_Element.Attributes[kPropertyContainer_version];
   string pcVersion = "1"; // Default if unknown
   if (pcVersion_Attrib != null)
   {
      pcVersion = pcVersion_Attrib.InnerText;
   }

   return pcVersion == requiredVersion;
}

//----------------------------------------------------------------------------
/// <summary>
/// Builds xpath that identifies this node in the property container, can be used to find a matching node in a different property container.
/// </summary>
public static string BuildNodeXPathInPropertyContainer(XmlNode node)
{
   string xpathQuery = "";

   while (node != null && node != node.OwnerDocument)
   {
      if( node.Name == Helpers.kElement_PropertyContainer )
      {
         xpathQuery = "PropertyContainer" + xpathQuery;
         break;
      }
      else
      {
         if (node.NodeType == XmlNodeType.Attribute)
         {
            xpathQuery = "@" + node.Name + xpathQuery;
         }
         else if (node.NodeType == XmlNodeType.Element)
         {
            XmlElement element = node as XmlElement;

            string elementMatch = "";

            string name = element.GetAttribute("name");
            string id = element.GetAttribute("id");

            if (!string.IsNullOrEmpty(name))
            {
               elementMatch = "[@name='" + name + "']";
            }
            else if (!string.IsNullOrEmpty(id))
            {
               elementMatch = "[@id='" + id + "']";
            }

            elementMatch = element.Name + elementMatch;
            xpathQuery = elementMatch + xpathQuery;
         }

         xpathQuery = "/" + xpathQuery;

         node = node.ParentNode;
      }
   }

   return xpathQuery;
}

//----------------------------------------------------------------------------

public enum EDiagnosticLevel
{
   kDL_None,
   kDL_Low,
   kDL_High,
   kDL_VeryHigh,
   kDL_Max = kDL_VeryHigh,
   
   kDL_Count,   
}

// Determines amount of debug console info when Merging/Unmerging
public static EDiagnosticLevel   sDiagnosticLevel = EDiagnosticLevel.kDL_None;

// [Conditional("DEBUG")] improves Release performance by not calling functions that supply parameter arguments as
// compiles out to nothing. No overhead for using this function in Release/
[Conditional("DEBUG")]
static private void WriteDiagnosticInfo(EDiagnosticLevel minimumDiagnosticLevel, params string[] infoStrings)
{
   if (minimumDiagnosticLevel <= sDiagnosticLevel)
   {
      foreach(string info in infoStrings)
      {
         System.Diagnostics.Debug.Write(info);      
      }
   }
}

[Conditional("DEBUG")]
static private void WriteDiagnosticInfo(params string[] infoStrings)
{
   WriteDiagnosticInfo(EDiagnosticLevel.kDL_Low, infoStrings);
}

//----------------------------------------------------------------------------

} // MergePropertiesV2
} // Tools.Property.Build

