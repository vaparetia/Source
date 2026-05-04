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
using Tools.Property.Misc;

#endregion

namespace Tools.Property.Build
{     
   /// <summary>
   /// Used for multiple selection of property objects. i.e. In maya, several objects may be selected at once.
   /// This will  build a new xml property tree from all common properties.
   /// </summary>
   
   public class GroupProperties
   {
      /// <summary>
      /// Used to specify grouping ungrouping options.
      /// In most cases you'll want to use kIgnoreTopName_IgnoreTopId as at the top level you generally don't if these are different.
      /// </summary>
      public enum EGroupOptions
      {
         kUseTopNameAndId,
         kUseTopName_IgnoreTopId,
         kIgnoreTopName_IgnoreTopId,
         kIgnoreTopName_UseId
      };

      public enum ELayoutElementProcessing
      {
         kIgnoreLayoutElement,
         kAddDescendantSlashForLayoutElements,
         kKeepLayoutElements
      };
      
      /// <summary>
      /// Takes the list of MergedProperties (in the property editor xml format, not using) and builds
      /// a new property tree with the common properties merged. The last property xml is the authoritive
      /// one for values. Values that are different will be flagged.   
      /// </summary>
      /// <param name="allProperties">
      /// List of xml properties to group.
      /// The properties list will not be modified.
      /// </param>
      /// <returns>
      /// New xml property tree with only the common properties.
      /// Tools properties that have different values will be marked with the 'grouped' 'true'
      /// attribute and appear differently in the property editor.
      /// </returns>      
      public static XmlDocument BuildGroupedProperties(List<XPathNavigator> allProperties, EGroupOptions groupOptions)
      {
         // Sanity checks
         if (allProperties.Count == 0)
         {
            return null;
         }

         if (allProperties.Count == 1)
         {
            // Duplicate the input and return it
            return Helpers.NavigatorToDocument( allProperties[0] );
         }

         XmlDocument xmlDoc = new XmlDocument();
         XPathNavigator output = xmlDoc.CreateNavigator();

         // Copy this so we can change it
         List<XPathNavigator> allPropertiesCopy = new List<XPathNavigator>(allProperties);
         //allPropertiesCopy.AddRange(allProperties);
         
         // Copy last xml properties as template
         output.AppendChild(allPropertiesCopy[allPropertiesCopy.Count - 1]);
         output.MoveToRoot();
         // Delete the last one
         allPropertiesCopy.RemoveAt(allPropertiesCopy.Count - 1);

         // Component groups should be read-only (not dynamic) when being worked with in grouped mode
         if (allProperties.Count > 1)
         {
            Helpers.DeleteAllFromXPath( output, String.Format("//PropertyContainer/ComponentGroup/@{0}", Helpers.kComponentGroup_Dynamic) );
         }

         string[] xpathQueries = { "//EvaluatorProperty", "//EvaluatorProperty/Evaluator", "//Value" };
         foreach(string xpathQuery in xpathQueries)
         {
            // List of values to delete
            List<XPathExpression> valuesToDelete = new List<XPathExpression>();
         
            // Now, select all value nodes in src document
            XPathNodeIterator valuesIter = output.Select(xpathQuery);
            
            while (valuesIter.MoveNext())
            {
               XPathNavigator edNav = valuesIter.Current as XPathNavigator;
               string propertyValue = edNav.Value;
               string xpathString = BuildPropertyXPathForValue(edNav, groupOptions, ELayoutElementProcessing.kAddDescendantSlashForLayoutElements);
               // Build a comparable version of the xpath so we can deal will the multiple decendant ('//') match issue
               string srcComparableXPathString = BuildPropertyXPathForValue(edNav, groupOptions, ELayoutElementProcessing.kIgnoreLayoutElement);
               // Compile xpath for speed
               XPathExpression xpathExp = edNav.Compile(xpathString);
               bool bAllValuesMatch = true;
               bool bFoundAllPropertyValues = true;
               foreach(XPathNavigator xmlNav in allPropertiesCopy)
               {
                  bool bFoundPropertyValueMatch = false;
                  // Find xpath match in other documents
                  XPathNodeIterator groupedValuesIter = xmlNav.Select(xpathExp);
                  while (groupedValuesIter.MoveNext())
                  {
                     XPathNavigator groupedNav = groupedValuesIter.Current;
                     string destComparableXPathString = BuildPropertyXPathForValue(groupedNav, groupOptions, ELayoutElementProcessing.kIgnoreLayoutElement);
                     // OK, we found an xpath match, but if we were using '//' in our xpath because of the layout tags,
                     // we aren't guaranteed to be looking at the same value. Use the comparable xpath to be sure.
                     if (destComparableXPathString == srcComparableXPathString)
                     {
                        switch (groupedNav.Name)
                        {
                           case Helpers.kElement_Value:
                           {   
                              bFoundPropertyValueMatch = true;
                              if (propertyValue != groupedNav.Value)
                              {
                                 // We need to add a group="true" attribute because all the values don't match
                                 bAllValuesMatch = false;
                              }
                           }
                           break;
                           case Helpers.kElement_EvaluatorProperty:
                           {
                              // Check for matching evaluator properties
                              // If the properties don't match, they will be deleted.
                              bFoundPropertyValueMatch = true;
                              // We don't care about grouping in this case, just want to see if they exist.
                              bAllValuesMatch = true;
                           }
                           break;
                           case Helpers.kElement_Evaluator:
                           {
                              bFoundPropertyValueMatch = true;
                              if (edNav.GetAttribute(Helpers.kEvaluator_Id, String.Empty) != groupedNav.GetAttribute(Helpers.kEvaluator_Id, String.Empty))
                              {
                                 // Evaluators don't match, mark as grouped
                                 bAllValuesMatch = false;
                              }
                           }
                           break;
                        }
                     }
                  }
                  // We didn't find a property value match
                  if (!bFoundPropertyValueMatch)
                  {
                     // Nope, at least one other document doesn't have this value
                     bFoundAllPropertyValues = false;
                     break;
                  }
               }
               
               if (!bFoundAllPropertyValues)
               {
                  // If it's a value, don't delete it if it is below an Evaluator
                  bool bDelete = true;
                  switch (edNav.Name)
                  {
                     // If it's a value, don't delete it if it is below an Evaluator
                     case Helpers.kElement_Value:
                        bDelete = (!xpathString.Contains(Helpers.kElement_Evaluator));
                        break;
                     case Helpers.kElement_Evaluator:
                        // We've already deleted any evaluators below mismatched EvaluatorProperties
                        bDelete = false;
                        bAllValuesMatch = false;   // Mark as grouped
                        break;
                     case Helpers.kElement_EvaluatorProperty:
                        bDelete = true;
                        break;
                  }
                  if (bDelete) valuesToDelete.Add(xpathExp.Clone());
               }
               if (!bAllValuesMatch)
               {
                  // Mark as shared value
                  edNav.CreateAttribute(String.Empty, Helpers.kProperty_Value_Grouped, String.Empty, "true");
               }
            }
            
            // Now delete all unwanted values/evaluators
            foreach(XPathExpression xpathExp in valuesToDelete)
            {
               // Now, select all value nodes in src document
               XPathNodeIterator deleteValuesIter = output.Select(xpathExp);
               while (deleteValuesIter.MoveNext())
               {
                  XPathNavigator edNav = deleteValuesIter.Current as XPathNavigator;
                  edNav.DeleteSelf();
               }
            }
            
         }


         // Now delete all properties that don't have values below them
         while (true)
         {
            string selectValues = "//Property[not(Value)]";
            XPathNodeIterator valuesIter = output.Select(selectValues);
            if (valuesIter.MoveNext())
            {
               XPathNavigator edNav = valuesIter.Current as XPathNavigator;
               // Property doesn't have a any values below it, delete
               edNav.DeleteSelf();
            }
            else
            {
               break;
            }
         }
         /*
         // Now delete all evaluator properties that don't have values below them
         while (true)
         {
            string selectValues = "//EvaluatorProperty[not(descendant::Value)]";
            XPathNodeIterator valuesIter = output.Select(selectValues);
            if (valuesIter.MoveNext())
            {
               XPathNavigator edNav = valuesIter.Current as XPathNavigator;
               // Property doesn't have a any values below it, delete
               edNav.DeleteSelf();
            }
            else
            {
               break;
            }
         }
         */

         // Now delete all propertyobjects that don't have properties or <UISettings> below them unless they are at the top level
         while (true)
         {
            string selectPropertyObjects = "//self::PropertyObject[not(descendant::Property)]";
            XPathNodeIterator propertyObjectsIter = output.Select(selectPropertyObjects);
            if (propertyObjectsIter.MoveNext())
            {
               XPathNavigator edNav = propertyObjectsIter.Current;
               {
                  // PropertyGroup doesn't have anything below it, delete
                  edNav.DeleteSelf();
               }
            }
            else
            {
               break;
            }
         }

         // Now delete all components that don't have stuff below them
         while (true)
         {
            string selectPropertyObjects = "//self::Component[not(descendant::Property or descendant::PropertyObject)]";
            XPathNodeIterator propertyObjectsIter = output.Select(selectPropertyObjects);
            if (propertyObjectsIter.MoveNext())
            {
               XPathNavigator edNav = propertyObjectsIter.Current;
               {
                  // Component doesn't have anything below it, delete
                  edNav.DeleteSelf();
               }
            }
            else
            {
               break;
            }
         }

         // Now delete all componentgroups that don't have values below them unless they are at the top level
         while (true)
         {
            string selectPropertyObjects = "//self::ComponentGroup[not(Component)]";
            XPathNodeIterator propertyObjectsIter = output.Select(selectPropertyObjects);
            if (propertyObjectsIter.MoveNext())
            {
               XPathNavigator edNav = propertyObjectsIter.Current;
               {
                  // ComponentGroup doesn't have anything below it, delete
                  edNav.DeleteSelf();
               }
            }
            else
            {
               break;
            }
         }

         // OK, we're done!
         return xmlDoc;
      }
      public static XmlDocument BuildGroupedProperties( List<XmlElement> allProperties, EGroupOptions groupOptions )
      {
         List<XPathNavigator> list = new List<XPathNavigator>();
         foreach ( XmlNode node in allProperties )
         {
            list.Add( node.CreateNavigator() );
         }

         return BuildGroupedProperties( list, groupOptions );
      }
      /// <summary>
      /// Utility function to make an XPath query to find all the modified values from a property box
      /// </summary>
      /// <returns>XPath Query Text</returns>
      private static string GetModifiedValuesXPathString()
      {
         // Select all value elements that are modified
         string selectUngroupedValues = "//Property/Value[@modified=\"True\"]";
         // Add support for component groups
         selectUngroupedValues += " | //ComponentGroup/Value[@modified=\"True\"]";
         // Add support for evaluator properties, check for modified
         selectUngroupedValues += " | //EvaluatorProperty/Evaluator[@modified=\"True\"]";

         return selectUngroupedValues;
      }

      /// <summary>
      /// Removes all of the "modified" value attributes from the document.  Useful if you're grabbing 
      /// the data out of a property box without ungrouping it.
      /// </summary>
      /// <param name="nav">XPathNavigator that represents the document</param>
      public static void RemoveModifiedFlags(XPathNavigator nav)
      {
         string modifiedValues = GetModifiedValuesXPathString();

         foreach (XPathNavigator propertyValue in nav.Select(modifiedValues))
         {
            if ( propertyValue.MoveToAttribute( Misc.Helpers.kProperty_Value_Modified, String.Empty ) )
            {
               propertyValue.DeleteSelf();
            }
         }
        
      }

      private static void CopyComponentGroupChangesToProperties(XPathNavigator src, XPathNavigator dst)
      {
         // Let's copy the name attributes from the src to the dst

         foreach (XPathNavigator nav in src.Select("//PropertyContainer/ComponentGroup[@name]"))
         {
            // Not all components have editor ids.
            XPathNavigator editorId_Nav = nav.SelectSingleNode("@editorId");
            if (editorId_Nav != null)
            {
               Guid editorId = new Guid(editorId_Nav.Value);
               String name = nav.GetAttribute("name", String.Empty);

               XPathNavigator dstComponent = Helpers.GetComponentFromEditorId( dst, editorId);

               if (dstComponent != null)
               {
                  // Update the component name in the destination component.
                  dstComponent.MoveToParent();
                  dstComponent.MoveToAttribute("name", String.Empty);
                  dstComponent.SetValue(name);
               }
            }
         }

         // Reorder all the dest component group navigators to be in the same order as in the source.
         foreach (XPathNavigator nav in src.Select("//PropertyContainer/ComponentGroup/@editorId"))
         {
            Guid editorId = new Guid( nav.Value );

            // Now we find the component with this editor id in the dst list...
            XPathNavigator writingComponent = Helpers.GetComponentGroupFromEditorId(dst, editorId);

            if ( writingComponent != null )
            {
               XPathNavigator parent = writingComponent.Clone();
               parent.MoveToParent();

               XPathNavigator clone = writingComponent.Clone();

               writingComponent.DeleteSelf();
               parent.AppendChild(clone);
            }
         }
      }

      /// <summary>
      /// Takes a property tree that has been build with <see cref=" BuildGroupProperties "/>
      /// and applies changes back to grouped list.
      /// Changes to srcNav will be applied to allProperties.
      /// </summary>
      /// <param name="srcNav">
      /// Modified property tree originally constructed by <see cref="BuildGroupedProperties"/>.
      /// This will remain unchanged, but it's changes will be applied back to allProperites.
      /// </param>
      /// <param name="allProperties">
      /// List of original xml properties to grouped.
      /// Changes to <paramref name="srcNav"/> will be applied to allProperties.
      /// </param>
      public static void UngroupProperties(XPathNavigator srcNav, List<XPathNavigator> allProperties, EGroupOptions groupOptions)
      {
         if (allProperties.Count == 1)
         {
            // You can only change component group properties for one object at a time.
            CopyComponentGroupChangesToProperties(srcNav, allProperties[0]);
         }

         // Select all value elements that are modified
         string selectUngroupedValues = GetModifiedValuesXPathString();
         // Add support for UISettings
         selectUngroupedValues += " | //UISettings";
         // Add support for UIState
         selectUngroupedValues += string.Format(" | //{0}[{1}[@modified=\"True\"] ]", MergeLayout.kElement_LayoutItem, MergeLayout.kElement_UIState);
         selectUngroupedValues += string.Format(" | //{0}[{1}[@modified=\"True\"] ]", Helpers.kElement_Layout, MergeLayout.kElement_UIState);
         
         // Delete empty <UISettings> for performance
         Helpers.DeleteAllFromXPath(srcNav, "//UISettings[not(child::*)]");

         // First strip grouped evaluators below evaluators as they aren't allowed to be merged
         Helpers.DeleteAllFromXPath(srcNav, "//Evaluator/EvaluatorProperty/Evaluator[@grouped=\"true\"]");
         
         XPathNodeIterator valuesIter = srcNav.Select(selectUngroupedValues);
         
         // Go through each value and assign it to each entry in all properties
         while (valuesIter.MoveNext())
         {
            XPathNavigator propertyValueNav = valuesIter.Current;
            // Move to parent if evaluator as destination objects might not have an evaluator there. We also need to copy attributes.
            if (propertyValueNav.Name == Helpers.kElement_Evaluator) propertyValueNav.MoveToParent();
            string xpathString = BuildPropertyXPathForValue(propertyValueNav, groupOptions, ELayoutElementProcessing.kAddDescendantSlashForLayoutElements);
            // See notes in GroupProperties above to see why this comparable version is important
            string srcComparableXPathString = null;
            // Compile xpath for speed
            XPathExpression xpathExp = propertyValueNav.Compile(xpathString);
            foreach(XPathNavigator ungroupedNav in allProperties)
            {
               // Find xpath match in other documents
               XPathNodeIterator ungroupedValuesIter = ungroupedNav.Select(xpathExp);
               while (ungroupedValuesIter.MoveNext())
               {
                  if (srcComparableXPathString == null)
                  {
                     srcComparableXPathString = BuildPropertyXPathForValue(propertyValueNav, groupOptions, ELayoutElementProcessing.kIgnoreLayoutElement);
                  }
                  
                  XPathNavigator edNav = ungroupedValuesIter.Current;
                  string destComparableXPathString = BuildPropertyXPathForValue(edNav, groupOptions, ELayoutElementProcessing.kIgnoreLayoutElement);
                  if (destComparableXPathString == srcComparableXPathString)
                  {
                     // Set the value to that of grouped properties
                     edNav.InnerXml = propertyValueNav.InnerXml;
                  }
               }
            }
         }

         return;
      }
      public static void UngroupProperties( XmlDocument srcDocument, List<XmlElement> allProperties, EGroupOptions groupOptions )
      {
         List<XPathNavigator> list = new List<XPathNavigator>();
         foreach ( XmlElement element in allProperties )
         {
            list.Add( element.CreateNavigator() );
         }

         UngroupProperties( srcDocument.CreateNavigator(), list, groupOptions );
      }
            
      /// <summary>
      /// Builds an xpath from the <paramref name=" nav"/>'s current position to root.
      /// Uses the id and name attributes to confirm path uniqueness.
      /// </summary>
      /// <param name="nav">
      /// Assumes that srcNav is pointing at a value element.
      /// </param>
      /// <param name="bIgnoreLayout">
      /// If just building xpaths, set to false.
      /// Set to true to build comparable xpaths to make sure that value paths match.
      /// </param>
      /// <returns>
      /// String containing xpath. e.g. "/PropertyGroup[@id="foo"]/Property[@name="bar" and @id="boom"]/Value".
      /// </returns>
      public static string BuildPropertyXPathForValue(XPathNavigator nav, EGroupOptions groupOptions, ELayoutElementProcessing layoutProcessing)
      {
         // Copy so we don't modify incoming
         XPathNavigator srcNav = nav.CreateNavigator();
         StringBuilder xpath = new StringBuilder();
         
         bool bNameFirstEntry = false;
            
         switch (srcNav.Name)
         {
            case Helpers.kElement_TargetGuid:
               xpath.AppendFormat("/{0}/{1}", srcNav.Name, srcNav.Value);  // Add element and value
               break;
               
            case Helpers.kElement_EvaluatorContainerEntry:
               // Insert element name so we know to ignore it later in merge
              xpath.AppendFormat("/EvaluatorContainerEntry[@targetProperty=\"{0}\"]", srcNav.GetAttribute(Helpers.kEvaluatorContainerEntry_targetProperty, String.Empty));
              break;
              
            case Helpers.kElement_EvaluatorProperty:
            case Helpers.kElement_Property:
            case Helpers.kElement_PropertyObject:
            case MergeLayout.kElement_LayoutItem:
            case Helpers.kElement_Layout:
               // We want to use the name of this object, so we'll disable the first 'MoveToParent'
               bNameFirstEntry = true;
               break;
              
            case Helpers.kElement_Evaluator: // We explicitly don't want to check name or id
            default:
               xpath.AppendFormat("/{0}", srcNav.Name);  // Add /Value (or whatever, UISettings) element
               break;
         }
         
         while (bNameFirstEntry || (srcNav.MoveToParent() && (srcNav.NodeType != XPathNodeType.Root)))
         {
            bNameFirstEntry = false;
            string id = srcNav.GetAttribute( Helpers.kUsing_Attribute_Id, "");
            string name = srcNav.GetAttribute( Helpers.kProperty_Name, "");
            
            // Check to see if parent is root and remove name
            if (groupOptions != EGroupOptions.kUseTopNameAndId)
            {
               XPathNavigator rootCheck = srcNav.Clone();
               if (rootCheck.MoveToParent() && (rootCheck.NodeType == XPathNodeType.Root))
               {
                  // Wipe out name/id
                  switch (groupOptions)
                  {
                     case EGroupOptions.kIgnoreTopName_IgnoreTopId:
                     {
                        name = "";
                        id = "";
                     }
                     break;
                     case EGroupOptions.kIgnoreTopName_UseId:
                     {
                        name = "";                        
                     }
                     break;
                     case EGroupOptions.kUseTopName_IgnoreTopId:
                     {                     
                        id = "";
                     }
                     break;
                  }                  
               }
            }

            if (srcNav.Name == Helpers.kElement_EvaluatorContainerEntry)
            {
               // Insert element name so we know to ignore it later in merge
               xpath.Insert(0, String.Format("/EvaluatorContainerEntry[@targetProperty=\"{0}\"]", srcNav.GetAttribute(Helpers.kEvaluatorContainerEntry_targetProperty, String.Empty)));
            }
            else if ((srcNav.Name == Helpers.kElement_Evaluator) || (srcNav.Name == Helpers.kElement_UsingEvaluator))
            {
               // Add evaluator id for match in merge
               xpath.Insert(0, String.Format("/*[@id=\"{0}\"]", srcNav.GetAttribute(Helpers.kEvaluator_Id, String.Empty)));
            }
            else
            {
               StringBuilder attribQuery = new StringBuilder();
               if( id != "" || name != "" )
               {
                  // Setup query for id/name attribute. Don't forget, we're building this string backwards
                  attribQuery.Append("[");
                  if( name != "" )
                  {
                     attribQuery.AppendFormat("@name=\"{0}\"", name);
                  }
                  if( id != "" )
                  {
                     if( name != "" )
                     {
                        attribQuery.Append(" and ");
                     }
                     
                     attribQuery.AppendFormat("@id=\"{0}\"", id);
                  }
                  attribQuery.Append("]");
               }
               xpath.Insert(0, attribQuery);
               // Now insert element name
               xpath.Insert(0, srcNav.Name);
               xpath.Insert(0, "/");
            }
         }
         
         return xpath.ToString();
      }
      public static string BuildPropertyXPathForValue( XmlNode node, EGroupOptions groupOptions, ELayoutElementProcessing layoutProcessing )
      {
         return BuildPropertyXPathForValue( node.CreateNavigator(), groupOptions, layoutProcessing );
      }

   }
}

