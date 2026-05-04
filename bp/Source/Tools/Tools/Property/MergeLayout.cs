using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Tools.Property.Misc;
using Tools.Property.Source;
using System.Xml.XPath;

namespace Tools.Property.Build
{
   /// <summary>
   /// This class manages merging/unmerging/updating of UILayout data.
   /// The UILayout element can exist underneath: PropertyContainer, Component, PropertyObject, Evaluator and MessageObject.
   /// They describe the ordering and grouping (into layouts) of properties and provide storage for UIState (like minimized/activeTab for tabLayouts).
   /// </summary>
   internal class MergeLayout
   {
      // Default UILayout nodes will be created if no explicit ones are specified in the data.
      // The only UILayout needs to be created manually is if the user wants to customize the layout of the controls.

      // UILayout nodes can contain Items and Layouts. 
      // Layouts can contain Items and Layouts.
      
      // Layout's must have a name attribute for the merging/unmerging code to be able to identify them.
      
      // Item's have name attribute that that reference the property data name.
      // UILayout nodes don't have to contain all items, any items not specified will automatically be added at the end.

      // Example UILayout:
      // <UILayout>
      //    <Layout name="EditorProperties" displayName="Editor Properties" type="groupBar">
      //       <Item name="Name"/>
      //       <Item name="Translation"/>
      //       <Item name="Rotation"/>
      //       <Item name="Scale"/>
      //       <Item name="Visible"/>
      //       <Item name="EditorModel"/>
      //    </Layout>
      // </UILayout>

      public const string kElement_UILayout = "UILayout";
      public const string kElement_UIState = "UIState";
      public const string kElement_LayoutItem = "LayoutItem";

      public const string kItem_Name = "name";
      public const string kUIState_Name = "name";
      public const string kUIState_Default = "default";

      /// <summary>
      /// This function updates all the layout data for the passed in node (and any applicable child nodes) 
      /// and merges overrides.
      /// This function should be called ONCE from the MergeProperty code on the top level object just before returning it to the user.
      /// Calling it more than necessary will not cause errors, but it will cost performance.
      /// </summary>
      /// <param name="mergedData">Merged property data.</param>
      /// <param name="overrideData">Overrides data, can be null.</param>
      public static void MergeAllLayouts(XmlNode mergedData, XmlNode overrideData)
      {
         if (mergedData.NodeType == XmlNodeType.Document)
            mergedData = mergedData.FirstChild;

         if (overrideData != null && overrideData.NodeType == XmlNodeType.Document)
            overrideData = overrideData.FirstChild;

         string uiLayoutOwnerQuery = "descendant-or-self::" + Helpers.kElement_PropertyContainer + " | " +
                                     "descendant-or-self::" + Helpers.kElement_Component + " | " +
                                     "descendant-or-self::" + Helpers.kElement_PropertyObject + " | " +
                                     "descendant-or-self::" + Helpers.kElement_Evaluator + " | " +
                                     "descendant-or-self::" + Helpers.kElement_MessageObject;

         foreach(XmlElement uiLayoutOwnerNode in mergedData.SelectNodes(uiLayoutOwnerQuery) )
         {
            // Check for empty elements, these don't require layouts.
            // Adding UILayout nodes to empty MessageObjects also breaks cooking messages without properties,
            // though 'Helpers.MessageObjectHasProperties' is really at fault here for not being robust.
            if (uiLayoutOwnerNode.HasChildNodes == false)
            {
               continue;
            }

            XmlElement uiLayoutNode = uiLayoutOwnerNode.SelectSingleNode("./" + kElement_UILayout) as XmlElement;
            
            if( uiLayoutNode == null )
            {
               uiLayoutNode = uiLayoutOwnerNode.OwnerDocument.CreateElement(kElement_UILayout);
               uiLayoutOwnerNode.AppendChild(uiLayoutNode);
            }

            UpdateLayoutNode(uiLayoutOwnerNode, uiLayoutNode);

            if (overrideData != null)
            {
               string pathToUILayout = kElement_UILayout;
               {
                  XmlElement current = uiLayoutNode.ParentNode as XmlElement;

                  while( current != mergedData )
                  {
                     string nodeName = current.GetAttribute(Helpers.kProperty_Name);
                     pathToUILayout = string.Format("*[@{0}='{1}']/", Helpers.kProperty_Name, nodeName) + pathToUILayout;

                     current = current.ParentNode as XmlElement;
                  }
                  
                  pathToUILayout = "./" + pathToUILayout;
               }

               XmlElement overrideUILayout = overrideData.SelectSingleNode(pathToUILayout) as XmlElement;
               if (overrideUILayout != null)
               {
                  // Go over all UIState nodes in the unmerged (using) data and merge it into the merged document.
                  foreach (XmlElement uiStateOverride in overrideUILayout.SelectNodes(".//" + kElement_UIState))
                  {
                     // Build xpath to locate the owner of this UIState node in the merged document
                     string pathToUIStateOwner = "";
                     {
                        XmlElement current = uiStateOverride.ParentNode as XmlElement;
                        while (current != null && current.Name != kElement_UILayout)
                        {
                           string nodeType = current.Name;
                           string name = current.GetAttribute(kItem_Name);

                           if (pathToUIStateOwner.Length != 0)
                              pathToUIStateOwner = "/" + pathToUIStateOwner;

                           pathToUIStateOwner = string.Format("{0}[@{1}='{2}']", nodeType, kItem_Name, name) + pathToUIStateOwner;

                           current = current.ParentNode as XmlElement;
                        }

                        pathToUIStateOwner = "./" + pathToUIStateOwner;
                     }

                     // Find owner in merged document
                     XmlElement destUIStateOwner = uiLayoutNode.SelectSingleNode(pathToUIStateOwner) as XmlElement;

                     if (destUIStateOwner != null)
                     {
                        string uiStateName = uiStateOverride.GetAttribute(kUIState_Name);

                        XmlElement destUIState = destUIStateOwner.SelectSingleNode(string.Format("./{0}[@{1}='{2}']", kElement_UIState, kUIState_Name, uiStateName)) as XmlElement;

                        if (destUIState == null)
                        {
                           destUIState = destUIStateOwner.OwnerDocument.CreateElement(kElement_UIState);
                           destUIState.SetAttribute(kUIState_Name, uiStateName);
                           destUIStateOwner.AppendChild(destUIState);
                        }

                        destUIState.InnerText = uiStateOverride.InnerText;
                     }
                  }
               }
            }
         }
      }

      /// <summary>
      /// This function removes any UILayout data (except when changed) from the passed in node (any any applicable child nodes).
      /// This function should be called ONCE on the merged data from the MergeProperty code prior to unmerging of property data.
      /// Calling it more than necessary will not cause errors, but it will cost performance.
      /// </summary>
      /// <param name="mergedData">Merged property data</param>
      public static void UnmergeAllLayouts(XmlNode mergedData)
      {
         foreach(XmlNode node in mergedData.SelectNodes("//" + kElement_UILayout))
         {
            UnmergeLayoutNode(node);
            
            if (node.ChildNodes.Count == 0)
               node.ParentNode.RemoveChild(node);
         }
      }

      /// <summary>
      /// This function should be called if structural changes have been made which means that new items need to be added or
      /// old items need to be removed from a UILayout directly underneath the passed object.
      /// Example usage case: It's called from the code that adds dynamic components to property containers.
      /// </summary>
      /// <param name="layoutOwnerNav">Navigator to node that contains the UILayout</param>
      public static void UpdateSingleLayout(XPathNavigator layoutOwnerNav)
      {
         XmlNode layoutOwnerNode = NavigatorToNode(layoutOwnerNav);

         XmlElement uiLayoutNode = layoutOwnerNode.SelectSingleNode("./" + kElement_UILayout) as XmlElement;
         if( uiLayoutNode != null )
         {
            UpdateLayoutNode(layoutOwnerNode, uiLayoutNode);
         }
      }

      public static void RenameLayoutItem(XmlElement layoutOwner, string oldName, string newName)
      {
         XmlElement uiLayoutNode = layoutOwner.SelectSingleNode( "./" + kElement_UILayout ) as XmlElement;
         if (uiLayoutNode != null)
         {
            XmlElement item = uiLayoutNode.SelectSingleNode(string.Format("./*[@{0}='{1}']", Helpers.kProperty_Name, oldName)) as XmlElement;
            item.SetAttribute(Helpers.kProperty_Name, newName);
         }
      }

      /// <summary>
      /// Move layout item with given name either up or down (based on direction)
      /// </summary>
      /// <param name="layoutOwnerNav"></param>
      /// <param name="name"></param>
      /// <param name="direction">if positive move down, otherwise move up</param>
      public static void MoveLayoutItem( XmlElement layoutOwnerNode, string name, int direction )
      {
         XmlElement uiLayoutNode = layoutOwnerNode.SelectSingleNode("./" + kElement_UILayout) as XmlElement;
         if (uiLayoutNode != null)
         {
            for( int i = 0; i < uiLayoutNode.ChildNodes.Count; ++i )
            {
               XmlElement item = uiLayoutNode.ChildNodes[i] as XmlElement;
               if( item.GetAttribute(Helpers.kProperty_Name) == name)
               {
                  item.ParentNode.RemoveChild(item);
                  if( direction > 0 )
                  {
                     uiLayoutNode.InsertAfter(item, uiLayoutNode.ChildNodes[i]);
                  }
                  else if( i > 0 )
                  {
                     uiLayoutNode.InsertBefore(item, uiLayoutNode.ChildNodes[i - 1]);
                  }

                  break;
               }
            }
         }
      }

      /// <summary>
      /// This item sets the UIState for a given layout item (either Layout or Item element).
      /// </summary>
      /// <param name="layoutItemNav">Navigator to layout item (either Layout or Item element).</param>
      /// <param name="state">Name of the state to set.</param>
      /// <param name="value">Value of state to set.</param>
      /// <param name="defaultValue">Default value, this is used during unmerging to only store data that is changed.</param>
      public static void SetUIState(XmlElement layoutItemNav, string state, string value, string defaultValue)
      {
         XmlElement uiStateElement = EnsureUIStateExists(layoutItemNav, state, defaultValue);
         // Set modified attribute on UIState for ungrouping code.
         uiStateElement.SetAttribute(Helpers.kProperty_Value_Modified, true.ToString() );
         uiStateElement.InnerText = value;
      }

      /// <summary>
      /// This item gets the UIState for a given layout item (either Layout or Item element).
      /// If the item doesn't exist it will create it.
      /// </summary>
      /// <param name="layoutItemElement">Element of the layout item (either Layout or Item element).</param>
      /// <param name="state">Name of the state to get.</param>
      /// <param name="defaultValue">Default value (in case UIState doesn't exist when queried).</param>
      /// <returns>Value of the requested state or default value if UIState didn't exist prior to calling this function.</returns>
      public static string GetUIState(XmlElement layoutItemElement, string state, string defaultValue)
      {
         XmlElement uiStateElement = EnsureUIStateExists(layoutItemElement, state, defaultValue);
         return uiStateElement.InnerText;
      }

      private static void UpdateLayoutNode(XmlNode layoutOwnerNode, XmlNode uiLayoutNode)
      {
         // Remove any items from the layout that do not exist in the actual properties.
         foreach (XmlElement item in uiLayoutNode.SelectNodes(".//" + kElement_LayoutItem))
         {
            string itemName = item.GetAttribute("name");
            XmlNode propertyElement = layoutOwnerNode.SelectSingleNode(string.Format("./*[@{0}='{1}']", Helpers.kProperty_Name, itemName));

            if (propertyElement == null)
            {
               item.ParentNode.RemoveChild(item);
            }
         }

         // Add any items for properties to the layout that aren't specified.
         foreach (XmlElement item in layoutOwnerNode.SelectNodes(string.Format("./*[self::{0} or self::{1} or self::{2} or self::{3}]", Helpers.kElement_ComponentGroup, Helpers.kElement_Property, Helpers.kElement_PropertyObject, Helpers.kElement_EvaluatorProperty)))
         {
            string itemName = item.GetAttribute(Helpers.kProperty_Name);

            XmlNode uiItemElement = uiLayoutNode.SelectSingleNode(string.Format(".//{0}[@{1}='{2}']", kElement_LayoutItem, kItem_Name, itemName)) as XmlElement;

            if (uiItemElement == null)
            {
               XmlElement newUiItemElement = uiLayoutNode.OwnerDocument.CreateElement(kElement_LayoutItem);
               newUiItemElement.SetAttribute("name", itemName);
               uiLayoutNode.AppendChild(newUiItemElement);
            }
         }
      }

      private static void UnmergeLayoutNode(XmlNode node)
      {
         // Remove UIState elements that do not have a modified value
         foreach(XmlElement uiStateNode in node.SelectNodes(".//" + kElement_UIState) )
         {
            string defaultValue = uiStateNode.GetAttribute(kUIState_Default);
            // if the value matches the 
            if( defaultValue == uiStateNode.InnerText)
            {
               uiStateNode.ParentNode.RemoveChild(uiStateNode);
            }
            else
            {
               RemoveAllAttributesExceptName(uiStateNode);
            }
         }

         // Remove all empty item elements.
         foreach(XmlElement itemNode in node.SelectNodes(".//" + kElement_LayoutItem + "[not(child::*)]"))
         {
            itemNode.ParentNode.RemoveChild(itemNode);
         }


         // Remove all empty layouts elements (we to keep checking after we removed a layout because we have nested layouts)
         {
            bool layoutsRemoved;

            do
            {
               layoutsRemoved = false;

               foreach (XmlElement layoutNode in node.SelectNodes(".//" + Helpers.kElement_Layout))
               {
                  // If layout node has no children, it can be removed
                  if( !layoutNode.HasChildNodes )
                  {
                     layoutNode.ParentNode.RemoveChild(layoutNode);
                     layoutsRemoved = true;
                  }
                  else
                  {
                     // If it does have children it needs to stay, but all we need to keep is the name attribute.
                     RemoveAllAttributesExceptName(layoutNode);
                  }
               }

            }
            while (layoutsRemoved);
         }

      }

      private static void RemoveAllAttributesExceptName(XmlElement elementNode)
      {
         // Remove all attributes except for the name tag.
         for (int i = 0; i < elementNode.Attributes.Count; )
         {
            XmlAttribute attr = elementNode.Attributes[i];

            if(attr.Name == Helpers.kProperty_Name)
            {
               i++;
            }
            else
            {
               elementNode.RemoveAttributeAt(i);
            }
         }
      }

      private static XmlElement EnsureUIStateExists(XmlElement layoutItemNode, string state, string defaultValue)
      {
         XmlElement uiStateElement = layoutItemNode.SelectSingleNode(string.Format("./{0}[@{1}='{2}']", MergeLayout.kElement_UIState, MergeLayout.kUIState_Name, state)) as XmlElement;

         if (uiStateElement == null)
         {
            uiStateElement = layoutItemNode.OwnerDocument.CreateElement(kElement_UIState);
            uiStateElement.SetAttribute(kUIState_Name, state);
            uiStateElement.InnerText = defaultValue;

            layoutItemNode.AppendChild(uiStateElement);
         }

         uiStateElement.SetAttribute(kUIState_Default, defaultValue);

         return uiStateElement;
      }

      private static XmlNode NavigatorToNode(XPathNavigator layoutOwnerNav)
      {
         XmlNode layoutOwnerNode = layoutOwnerNav.UnderlyingObject as XmlNode;

         // move to first element if the node is not an element (i.e. Document)
         if (layoutOwnerNode.NodeType != XmlNodeType.Element)
            layoutOwnerNode = layoutOwnerNode.FirstChild;

         return layoutOwnerNode;
      }
   }
}
