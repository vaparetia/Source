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

#endregion

namespace Tools.Property.Build
{  
   /// <summary>
   /// Used to build full xml property object tree from 'using' tags xml src.
   /// Can also unmerge property tree back to using tags.
   /// </summary>
   public class MergeProperties
   {
      public enum EBuildTreeSrcType
      {
         kPropertyObject,
         kComponent,
         kComponentGroup,
         kPropertyContainer,
         kMessageObject,
         kEvaluator,
         
         kInvalid
      }

      private static readonly int kComponentGroupMaxRecursionUninitialized = -1;

      /// <summary>
      /// Takes a full property object tree and converts it back to 'using' tags.
      /// Formats property tree into format that can be saved and remerged.
      /// Call this when finished modifing properties.
      /// </summary>
      /// <param name="srcNav"></param>
      /// Root of property object tree. Will not be modified.
      /// <returns>
      /// Unmerged xml tree for saving/remerging.
      /// </returns>
      public static XPathNavigator Unmerge(XPathNavigator srcNav)
      {
         XmlDocument srcCopyDoc = Helpers.NavigatorToDocument(srcNav);
         
         Tools.Property.Build.MergeLayout.UnmergeAllLayouts(srcCopyDoc);

         // Find all the element nodes that aren't properties/propertygroups/propertyobjects
         XPathNavigator srcCopyNav = srcCopyDoc.CreateNavigator();
         // Use navigator copy of the top level script object for comparison.
         XPathNavigator srcCopyChildNav = srcNav.Clone();

         // Lets delete empty events and links
         Helpers.DeleteAllFromXPath(srcCopyNav, "//ComponentGroup/Component/Events/Event[not(child::EventEntry)]");
         Helpers.DeleteAllFromXPath(srcCopyNav, "//ComponentGroup/Component/Links/Link[not(child::LinkEntry)]");
         // Delete unneeded Allow/Deny link entries
         Helpers.DeleteAllFromXPath(srcCopyNav, "//ComponentGroup/Component/Links/Link/Allow");
         Helpers.DeleteAllFromXPath(srcCopyNav, "//ComponentGroup/Component/Links/Link/Deny");

         {
            XPathExpression selectValues = XPathExpression.Compile(Helpers.kXPath_ValueSearch);
            List<XPathNavigator> valuesNavigators = Helpers.ConvertNodeIteratorToNavigatorList(srcCopyNav.Select(selectValues));

            foreach (XPathNavigator edNav in valuesNavigators)
            {
               string value = edNav.Value;
               // Default value can be an empty string in some cases so types that need to parse the string must deal with that case.
               // Note that an empty string is valid for asset properties.
               string defaultValue = edNav.GetAttribute(Helpers.kProperty_Value_Default, String.Empty);

               // Check to see if we want to prevent an unmerge
               bool bKeepInitialValue = false;
               {
                  XPathNavigator edParentNav = edNav.Clone();
                  edParentNav.MoveToParent();
                  bKeepInitialValue = Tools.Common.Misc.ParseBool(edParentNav.GetAttribute(Helpers.kProperty_Value_KeepInitialValue, String.Empty));
               }

               string type = string.Empty;

               if (edNav.Name == Helpers.kElement_UISettings)
               {
                  type = Helpers.kElement_UISettings;
               }
               else
               {
                  // Need to add special case for some types which can have same numerical value with different strings
                  XPathNavigator edNavParent = edNav.Clone();
                  edNavParent.MoveToParent();
                  type = edNavParent.GetAttribute(Helpers.kProperty_Type, String.Empty);
               }

               bool bCanMerge = false;
               try
               {
                  switch (type)
                  {
                     case Helpers.kProperty_Type_float:
                        bCanMerge = (defaultValue.Length != 0) && (Double.Parse(defaultValue) == Double.Parse(value));
                        break;

                     case Helpers.kProperty_Type_bool:
                        bCanMerge = (defaultValue.Length != 0) && (Tools.Common.Misc.ParseBool(defaultValue) == Tools.Common.Misc.ParseBool(value));
                        break;

                     case Helpers.kProperty_Type_evaluatorContainer: // Evaluator containers don't unmerge, they just keep their properties
                        bCanMerge = (value.Length == 0) && (!edNav.HasChildren);
                        break;
                     case Helpers.kProperty_Type_custom: // Custom properties don't unmerge unless empty
                        bCanMerge = (value.Length == 0) && (!edNav.HasChildren);
                        break;

                     case Helpers.kElement_UISettings:
                        // Iterate over children, remove those that are default
                        {
                           bCanMerge = true;

                           XPathNodeIterator iter = edNav.SelectChildren(XPathNodeType.Element);
                           while (iter.MoveNext())
                           {
                              string uiDefault = iter.Current.GetAttribute(Helpers.kProperty_Value_Default, String.Empty) ?? String.Empty;
                              if (uiDefault == iter.Current.Value)
                              {
                                 iter.Current.DeleteSelf();
                                 iter = edNav.SelectChildren(XPathNodeType.Element);
                              }
                              else
                              {
                                 // Can't delete this element, delete the default attribute
                                 if (iter.Current.MoveToAttribute(Helpers.kProperty_Value_Default, String.Empty))
                                 {
                                    iter.Current.DeleteSelf();
                                 }
                                 bCanMerge = false;
                              }
                           }
                        }
                        break;

                     default:
                        bCanMerge = (defaultValue == value);
                        break;
                  }
               }
               catch (FormatException)
               {
                  bCanMerge = false;
                  Debug.Assert(false, String.Format("Unable to parse merge string values for default:{0} or value:{1} for type:{2}.", defaultValue, value, type));
               }

               if (bCanMerge && !bKeepInitialValue)
               {
                  // Default value matches object value, remove
                  edNav.DeleteSelf();
               }
               else
               {
                  // No unmerge
                  // Delete the default attribute
                  if (edNav.MoveToAttribute(Helpers.kProperty_Value_Default, String.Empty))
                  {
                     edNav.DeleteSelf();
                  }
               }
            }
         }

         // Now delete all elements below properties that aren't <Value/>
         Helpers.DeleteAllFromXPath(srcCopyNav, "//*[not(self::Value) and (parent::Property)]");
         // Now delete all properties that don't have values below them
         Helpers.DeleteAllFromXPath(srcCopyNav, "//Property[not(Value)]");

         // Now delete all evaluators
         {
            // Evaluators
            {
               XPathExpression selectEvaluator = XPathExpression.Compile("//Evaluator[not(descendant::Property or descendant::EvaluatorProperty or descendant::UISettings or descendant::UILayout)]");
               List<XPathNavigator> evaluators = Helpers.ConvertNodeIteratorToNavigatorList(srcCopyNav.Select(selectEvaluator));
               foreach (XPathNavigator evalNav in evaluators)
               {
                  string defaultId = evalNav.GetAttribute(Helpers.kEvaluator_Id_Default, String.Empty);
                  if (defaultId.Length != 0)
                  {
                     if (defaultId == evalNav.GetAttribute(Helpers.kEvaluator_Id, String.Empty))
                     {
                        evalNav.DeleteSelf();
                     }
                  }
               }
            }

            // EvaluatorProperty
            {
               XPathExpression selectEvaluatorProperty = XPathExpression.Compile("//EvaluatorProperty[not(descendant::Property or descendant::Evaluator or descendant::UISettings or descendant::UILayout)]");
               List<XPathNavigator> evaluators = Helpers.ConvertNodeIteratorToNavigatorList(srcCopyNav.Select(selectEvaluatorProperty));
               foreach (XPathNavigator evalNav in evaluators)
               {
                  evalNav.DeleteSelf();
               }
            }
         }

         // Now delete all propertyobjects that don't have properties or <UISettings> below them unless they are at the top level
         XPathExpression selectPropertyObjects = XPathExpression.Compile("//self::PropertyObject[not(descendant::Property or descendant::EvaluatorProperty or descendant::UISettings or descendant::UILayout)]");

         List<XPathNavigator> propertyObjects = Helpers.ConvertNodeIteratorToNavigatorList(srcCopyNav.Select(selectPropertyObjects));

         foreach(XPathNavigator edNav in propertyObjects)
         {
            if (!edNav.IsSamePosition(srcCopyChildNav))
            {
               // PropertyGroup doesn't have anything below it, delete
               edNav.DeleteSelf();
            }
            else
            {
               // Top level object
               break;
            }
         }

         bool bAnyComponentDeletions = true;
         XPathExpression selectComponents = XPathExpression.Compile("//self::Component[not(descendant::Property or descendant::EvaluatorProperty or descendant::ComponentGroup or descendant::PropertyObject or descendant::UISettings or descendant::UILayout or descendant::Links or descendant::Events)]");
         XPathExpression selectComponentGroups = XPathExpression.Compile("//self::ComponentGroup[not(UISettings or UILayout or Component or Value or @dynamicComponentGroup=\"true\")]");
         while (bAnyComponentDeletions)
         {
            // As components can contain component groups/components, we have to iterate over both types until there are no more deletions
            bAnyComponentDeletions = false;

            // Now delete all components that don't have stuff below them unless they are at the top level
            List<XPathNavigator> components = Helpers.ConvertNodeIteratorToNavigatorList(srcCopyNav.Select(selectComponents));
            foreach(XPathNavigator edNav in components)
            {
               if (!edNav.IsSamePosition(srcCopyChildNav))
               {
                  // Component doesn't have anything below it, delete
                  edNav.DeleteSelf();
                  bAnyComponentDeletions = true;
               }
            }

            // Now delete all componentgroups that don't have stuff below them unless they are at the top level or dynamic
            List<XPathNavigator> componentGroups = Helpers.ConvertNodeIteratorToNavigatorList(srcCopyNav.Select(selectComponentGroups));
            foreach(XPathNavigator edNav in componentGroups)
            {
               if (!edNav.IsSamePosition(srcCopyChildNav))
               {
                  // ComponentGroup doesn't have anything below it, delete
                  edNav.DeleteSelf();
                  bAnyComponentDeletions = true;
               }
            }
         }

         XPathNavigator destNav = new XmlDocument().CreateNavigator();
         bool alreadyConvertedGlobalObject = false;
         Unmerge_ConvertToUsing(srcCopyNav, destNav, ref alreadyConvertedGlobalObject);
         destNav.MoveToRoot();

         return destNav;
      }

      /// <summary>
      /// Internal function for 'property*'->'using' elements.
      /// </summary>
      /// <param name="srcNav">
      /// Path to 'property*' element.
      /// </param>
      /// <param name="destNav">
      /// Path for insertion of 'using' element.
      /// </param>
      /// <remarks>
      /// Could use xsl to convert this, but less stuff for everyone to learn if I just use code.
      /// Recurses down to value part of properties, then converts everything to using on the way up.
      /// </remarks>
      static void Unmerge_ConvertToUsing(XPathNavigator srcNav, XPathNavigator destNav, ref bool alreadyConvertedGlobalObject)
      {
         if (srcNav.MoveToFirstChild())
         {
            do
            {
               {
                  // OK, now we're left with the bare bones properties, convert into <using/>
                  string id = srcNav.GetAttribute(Helpers.kProperty_Id, String.Empty);
                  string name = srcNav.GetAttribute(Helpers.kProperty_Name, String.Empty);
                  
                  string usingElement = "<" + Helpers.kElement_Using + "/>";
                  
                  switch (srcNav.Name)
                  {

                     case Helpers.kElement_PropertyContainer:
                        if (!alreadyConvertedGlobalObject)
                        {
                           alreadyConvertedGlobalObject = true;
                           // Beta2 no longer returns a new navigator, we'll create a new document instead.
                           // Might have to check the performance of this later.
                           
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild("<UsingPropertyContainer/>");
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kUsing_Attribute_Id, String.Empty, id);
                           child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        else
                        {
                           // If we've already seen a top level property object / component then just use the name
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild(usingElement);
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        break;                           
                        
                     case Helpers.kElement_ComponentGroup:
                        {
                           // Dynamic component groups have been added separately from the script object,
                           // so they have to unmerge to <UsingComponentGroup> as there will be no merge match otherwise

                           bool dynamicComponentGroup = Helpers.IsComponentGroupDynamic(srcNav);
                           if (!alreadyConvertedGlobalObject || dynamicComponentGroup)
                           {
                              alreadyConvertedGlobalObject = true;
                              // Might have to check the performance of this later.
                              
                              XPathNavigator child = new XmlDocument().CreateNavigator();
                              child.AppendChild("<UsingComponentGroup/>");
                              child.MoveToChild(XPathNodeType.Element);
                              child.CreateAttribute(String.Empty, Helpers.kUsing_Attribute_Id, String.Empty, id);
                              child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                              if (dynamicComponentGroup)
                              {
                                 // Copy attributes from the source over.
                                 XPathNavigator attributeNav = srcNav.Clone();
                                 if (attributeNav.MoveToFirstAttribute())
                                 {
                                    do 
                                    {
                                       if (null == child.SelectSingleNode("@" + attributeNav.LocalName))
                                       {
                                          child.CreateAttribute(String.Empty, attributeNav.LocalName, String.Empty, attributeNav.Value);
                                       }
                                    } while ( attributeNav.MoveToNextAttribute() );
                                 }


                                 // The max sub components is a short term hack to deal with dynamic particle systems
                                 if (null == child.SelectSingleNode("@" + Helpers.kComponentGroup_MaxSubComponents))
                                 {
                                    child.CreateAttribute(
                                       String.Empty, 
                                       Helpers.kComponentGroup_MaxSubComponents, 
                                       String.Empty, 
                                       "3" );
                                 }
                              }
                              Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                              destNav.AppendChild(child);                                                                                                       
                           }
                           else
                           {
                              // If we've already seen a top level property object / component then just use the name
                              XPathNavigator child = new XmlDocument().CreateNavigator();
                              child.AppendChild(usingElement);
                              child.MoveToChild(XPathNodeType.Element);
                              child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                              Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                              destNav.AppendChild(child);                                                                                                       
                           }
                        }
                        break;                           
                        
                     case Helpers.kElement_Component:
                        {
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           if (!alreadyConvertedGlobalObject)
                           {
                              alreadyConvertedGlobalObject = true;
                              // Beta2 no longer returns a new navigator, we'll create a new document instead.
                              // Might have to check the performance of this later.
                              
                              child.AppendChild("<UsingComponent/>");
                              child.MoveToChild(XPathNodeType.Element);
                              child.CreateAttribute(String.Empty, Helpers.kUsing_Attribute_Id, String.Empty, id);
                              child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);

                           }
                           else
                           {
                              // If we've already seen a top level property object / component then just use the name
                              child.AppendChild(usingElement);
                              child.MoveToChild(XPathNodeType.Element);
                              child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           }
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           // child is current component
                           XPathNavigator componentNav = child.Clone();
                           // Unmerge events/links (properties for these are stored unmerged)
                           XPathNavigator eventsNav = srcNav.SelectSingleNode("Events/Event/EventEntry");
                           
                           if (eventsNav != null)
                           {
                              // We've got events, add them
                              eventsNav.MoveToParent();
                              eventsNav.MoveToParent();  // Events
                              componentNav.AppendChild(eventsNav);
                           }
                           XPathNavigator linksNav = srcNav.SelectSingleNode("Links/Link/LinkEntry");
                           if (linksNav != null)
                           {
                              // We've got links, add them
                              linksNav.MoveToParent();
                              linksNav.MoveToParent();  // Links
                              componentNav.AppendChild(linksNav);
                           }
                           destNav.AppendChild(child);                                                                                                       
                        }
                        break;                           
                        
                     case Helpers.kElement_PropertyObject:
                        if (!alreadyConvertedGlobalObject)
                        {
                           alreadyConvertedGlobalObject = true;
                           // Beta2 no longer returns a new navigator, we'll create a new document instead.
                           // Might have to check the performance of this later.
                           
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild("<UsingPropertyObject/>");
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kUsing_Attribute_Id, String.Empty, id);
                           child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        else
                        {
                           // If we've already seen a top level property object / component then just use the name
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild(usingElement);
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        break;                           
                     case Helpers.kElement_MessageObject:
                        if (!alreadyConvertedGlobalObject)
                        {
                           alreadyConvertedGlobalObject = true;
                           // Beta2 no longer returns a new navigator, we'll create a new document instead.
                           // Might have to check the performance of this later.
                           
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild("<UsingMessageObject/>");
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kUsing_Attribute_Id, String.Empty, id);
                           child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        else
                        {
                           // If we've already seen a top level property object / component then just use the name
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild(usingElement);
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        break;                           
                     case Helpers.kElement_Evaluator:
                        {
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild("<UsingEvaluator/>");
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kUsing_Attribute_Id, String.Empty, id);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        break;                           
                     case Helpers.kElement_EvaluatorProperty:
                        {
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild("<Using/>");
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        break;                           
                        
                     case Helpers.kElement_EvaluatorContainerRoot:
                        {
                            XPathNavigator child = new XmlDocument().CreateNavigator();
                            child.AppendChild("<EvaluatorContainerRoot/>");
                            child.MoveToChild(XPathNodeType.Element);
                            Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                            destNav.AppendChild(child);                                                                                                       
                        }
                        break;
                     case Helpers.kElement_EvaluatorContainerEntry:
                        {
                            XPathNavigator child = new XmlDocument().CreateNavigator();
                            child.AppendChild("<EvaluatorContainerEntry/>");
                            child.MoveToChild(XPathNodeType.Element);
                            string target = srcNav.GetAttribute(Helpers.kEvaluatorContainerEntry_targetProperty, String.Empty);
                            child.CreateAttribute(String.Empty, Helpers.kEvaluatorContainerEntry_targetProperty, String.Empty, target);
                            // Copy targets
                            XPathNodeIterator targetIter = srcNav.Select(Helpers.kElement_TargetGuid);
                            while (targetIter.MoveNext())
                            {
                              child.AppendChild(targetIter.Current);
                            }
                            Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                            destNav.AppendChild(child);                                                                                                       
                        }
                     break;
                        
                     case Helpers.kElement_Layout:
                        {
                           // Skip <Layout> elements as they are ignored for merging and unmerging.
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), destNav, ref alreadyConvertedGlobalObject);
                        }
                        break;
                        
                     case Helpers.kElement_Property:
                        {                           
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild(usingElement);
                           child.MoveToChild(XPathNodeType.Element);
                           child.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, name);
                           Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                           destNav.AppendChild(child);                                                                                                       
                        }
                        break;

                     case Helpers.kElement_UISettings:
                        {
                           destNav.InnerXml = srcNav.OuterXml;
                        }
                        break;

                     case Tools.Property.Build.MergeLayout.kElement_UILayout:
                        {
                           destNav.AppendChild(srcNav);
                        }
                        break;

                     case Helpers.kElement_Value:
                        {
                           XPathNavigator child = new XmlDocument().CreateNavigator();
                           child.AppendChild("<Value/>");
                           child.MoveToChild(XPathNodeType.Element);
                           // First, check to see if it's parent is a component group
                           {  
                              XPathNavigator srcNavParent = srcNav.Clone();
                              
                              if (srcNavParent.MoveToParent() && (srcNavParent.Name == Helpers.kElement_ComponentGroup))
                              {
                                 // Just copy the value
                                 child.SetValue(srcNav.Value);
                                 destNav.AppendChild(child);                                                                                                       
                                 break;
                              }
                           }
                           
                           // for all other values
                           if (srcNav.MoveToChild(XPathNodeType.Element))
                           {
                              switch (srcNav.Name)
                              {
                                 case Helpers.kElement_EvaluatorContainerRoot:
                                    srcNav.MoveToParent();
                                    Unmerge_ConvertToUsing(srcNav.CreateNavigator(), child, ref alreadyConvertedGlobalObject);
                                    break;
                                 default:
                                    // All other value types
                                    srcNav.MoveToParent();
                                    child.InnerXml = srcNav.InnerXml;
                                    break;
                              }
                           }
                           else
                           {
                              // just a value
                              child.SetValue(srcNav.Value);
                           }
                           destNav.AppendChild(child);
                        }
                        break;
                  }
               }
            } while(srcNav.MoveToNext());
            
            // Restore position back to parent
            srcNav.MoveToParent();
         }
         return;
      }
      
      /// <summary>
      /// Takes src 'using' xml and builds a full property tree.
      /// Result can be used by <see cref=" PropertyEditorGUI"/>
      /// and <see cref=" GroupProperties"/>.
      /// </summary>
      /// <param name="srcNav">
      /// Root of 'using' xml. Will not be modified.
      /// </param>
      /// <param name="propertySource">
      /// Property source interface
      /// </param>
      /// <returns>
      /// New property tree to be used by <see cref=" PropertyEditorGUI"/>
      /// and <see cref=" GroupProperties"/>.
      /// </returns>
      public static XmlDocument MergePropertyObject( XPathNavigator srcNav,
                                                     IPropertyDataSource propertySource)
      {
         return MergePropertyObject(srcNav, propertySource, kComponentGroupMaxRecursionUninitialized, EMergePropertyObjectOptions.kSetDefaults);
      }
      
      enum EMergePropertyObjectOptions
      {
         kSetDefaults,
         kSetOnlyChildDefaults,  // Ignores top level node
         kDontSetDefaults
      };
      
      private static XmlDocument MergePropertyObject( XPathNavigator srcNav,
                                                      IPropertyDataSource propertySource,
                                                      int componentGroupMaxRecursion,
                                                      EMergePropertyObjectOptions setDefaults)
      {
         // Copy this so we can modify it
         XmlDocument srcPropertyObjectDoc = Helpers.NavigatorToDocument(srcNav);
         // Get top level property object
         XPathNavigator srcPropertyObjectNav = srcPropertyObjectDoc.CreateNavigator();
         if (!srcPropertyObjectNav.MoveToFirstChild())
         {
            // Nothing underneath to test!
            return null;  
         }
         
         string id = srcPropertyObjectNav.GetAttribute(Helpers.kProperty_Id, String.Empty);
         string name = srcPropertyObjectNav.GetAttribute(Helpers.kProperty_Name, String.Empty);

         // Work out what type of node it is
         EBuildTreeSrcType srcType = EBuildTreeSrcType.kInvalid;
         switch (srcPropertyObjectNav.Name)
         {
            case "UsingPropertyContainer":
               srcType = EBuildTreeSrcType.kPropertyContainer;
               break;
            case "UsingComponentGroup":
               srcType = EBuildTreeSrcType.kComponentGroup;
               break;
            case "UsingComponent":
               Debug.Assert(false, "No support for creating components without component groups.");
               //srcType = EBuildTreeSrcType.kComponent;
               return null;
            case "UsingPropertyObject":
               srcType = EBuildTreeSrcType.kPropertyObject;
               break;
            case "UsingMessageObject":
               srcType = EBuildTreeSrcType.kMessageObject;
               break;
            case "UsingEvaluator":
               srcType = EBuildTreeSrcType.kEvaluator;
               name = "DontCareAboutName";
               // Special check for empty evaluator
               if (id == Helpers.kEmptyEvaluatorName)
               {
                  // Just return fake 'None' evaluator.
                  srcPropertyObjectNav.OuterXml = "<Evaluator id=\"None\"/>";
                  return srcPropertyObjectDoc;
               }
               break;
            default:
               return null;   // don't know what this type is.
         }
         
         if (name.Length == 0 || id.Length == 0)
         {
            // No good, no name or id, can't use it
            return null;
         }

         // Now get the xml associated with this id
         XmlDocument derivedPropertyObjectDoc = BuildDerivedPropertyTree_Internal(  id, 
                                                                              name, 
                                                                              srcPropertyObjectNav,
                                                                              srcType,
                                                                              propertySource,
                                                                              componentGroupMaxRecursion);
         // Can't find this property object
         if (derivedPropertyObjectDoc == null) return null;                                                                              
         XPathNavigator derivedPropertyObjectNav = derivedPropertyObjectDoc.CreateNavigator();

         if (setDefaults == EMergePropertyObjectOptions.kSetDefaults)
         {
            // Fill in defaults
            SetValueDefaults(derivedPropertyObjectNav.CreateNavigator());
         }
         else if (setDefaults == EMergePropertyObjectOptions.kSetOnlyChildDefaults)
         {
            // We know that the top level 'EvaluatorProperty' isn't a default, so skip it and just set defaults on children
            XPathNavigator nav = derivedPropertyObjectNav.CreateNavigator();
            if (nav.NodeType == XPathNodeType.Root) nav.MoveToChild(XPathNodeType.Element); // Move off root
            if (nav.MoveToChild(XPathNodeType.Element)) // Move off 'Evaluator'
            {
               // Iterate over all siblings
               do
               {
                  SetValueDefaults(nav);
               } while (nav.MoveToNext(XPathNodeType.Element));
            }               
         }
         
         // Merge evaluator containers
         MergeEvaluatorContainersInternal(srcPropertyObjectNav, derivedPropertyObjectNav, propertySource, componentGroupMaxRecursion);

         // Merge in evaluators
         MergeEvaluatorsInternal(srcPropertyObjectNav, derivedPropertyObjectNav, propertySource, componentGroupMaxRecursion);
         
         // Run regular merge on derived PO with src values
         MergePropertyObjectInternal(srcPropertyObjectNav, derivedPropertyObjectNav);
         
         // Merge in dynamic components
         if (srcType == EBuildTreeSrcType.kPropertyContainer) MergeDynamicComponentGroupsInternal(srcPropertyObjectNav, derivedPropertyObjectNav, propertySource);

         // Replace UsingPropertyObject with real xml (this leaves any additional elements in place)
         srcPropertyObjectNav.ReplaceSelf(derivedPropertyObjectNav.ReadSubtree());

         Tools.Property.Build.MergeLayout.MergeAllLayouts(srcPropertyObjectDoc, srcNav.UnderlyingObject as XmlNode);

         return srcPropertyObjectDoc;
      }

      private static void SetValueDefaults(XPathNavigator derivedPropertyObjectNav)
      {
         {
            string selectValues = Helpers.kXPath_ValueSearch;
            XPathNodeIterator derivedValuesIter = derivedPropertyObjectNav.Select(selectValues);
            // Copy all value entries into default attribute so we can strip out unchanged properties later
            while (derivedValuesIter.MoveNext())
            {
               if (derivedValuesIter.Current.Name == Helpers.kElement_UISettings)
               {
                  // UI defaults
                  XPathNodeIterator iter = derivedValuesIter.Current.SelectChildren(XPathNodeType.Element);
                  while (iter.MoveNext())
                  {
                     XPathNavigator valueNav = derivedValuesIter.Current;
                     valueNav.CreateAttribute(String.Empty, Helpers.kProperty_Value_Default, String.Empty, valueNav.Value);
                  }
               }
               else
               {
                  // Regular values
                  XPathNavigator valueNav = derivedValuesIter.Current;
                  valueNav.CreateAttribute(String.Empty, Helpers.kProperty_Value_Default, String.Empty, valueNav.Value);
               }
            }
         }
         
         // Remove all defaults for evaluator containers as they are special case
         {
            string selectValues = ".//Property[@type=\"EvaluatorContainer\"]/Value[@default]";
            XPathNodeIterator derivedValuesIter = derivedPropertyObjectNav.Select(selectValues);
            // Copy all value entries into default attribute so we can strip out unchanged properties later
            while (derivedValuesIter.MoveNext())
            {
               XPathNavigator valueNav = derivedValuesIter.Current;
               if (valueNav.MoveToAttribute(Helpers.kProperty_Value_Default, String.Empty))
               {
                  valueNav.DeleteSelf();
               }
            }
         }
            
         // Now set defaults for evaluators
         {
            XPathNodeIterator derivedValuesIter = derivedPropertyObjectNav.Select(".//Evaluator");
            // Copy all value entries into default attribute so we can strip out unchanged properties later
            while (derivedValuesIter.MoveNext())
            {
               XPathNavigator idNav = derivedValuesIter.Current;
               idNav.CreateAttribute(String.Empty, Helpers.kEvaluator_Id_Default, String.Empty, idNav.GetAttribute(Helpers.kEvaluator_Id, String.Empty));
            }
         }
      }

      /// <summary>
      /// Internal function for converting 'using' elements to 'property*' elements.
      /// </summary>
      /// <param name="srcNav">
      /// Src 'using' xml document.
      /// </param>
      /// <param name="parentNav">
      /// Dest 'property*' xml.
      /// </param>
      /// <param name="findPO">
      /// Delegate for finding property object by id.
      /// Use <see cref=" FileBasedPropertyObjectEnumerator.FindById"/> or similar.
      /// </param>
      static void MergePropertyObjectInternal(  XPathNavigator srcUsingValuesNav,
                                                XPathNavigator destPropertyObjectNav)
      {
         // Find all dest properties w/o values below them so we can add empty value element
         while (true)
         {
            XPathNavigator missingValue = destPropertyObjectNav.SelectSingleNode("//self::Property[not (child::Value)]");
            if (missingValue != null)
            {
               // Add empty <Value>
               // Maybe print an error about this as we shouldn't have properties without default values.
               missingValue.AppendChild("<Value/>");
            }
            else
            {
               // No more missing property values
               break;
            }
         }
         
#if false   // UI settings no longer required with new layout support in CL5510 
         // Find all component groups, properties and evaluator properties and add UISettings if they don't exist.
         // We can expand this for other types if required later.
         // Not too sure what we should do about tab groups
         while (true)
         {
            XPathNavigator missingUISetting = destPropertyObjectNav.SelectSingleNode("//self::ComponentGroup[not (child::UISettings)] | //self::PropertyObject[not (child::UISettings)] | //self::EvaluatorProperty[not (child::UISettings)]");
            if (missingUISetting != null)
            {
               missingUISetting.PrependChild("<UISettings/>");
            }
            else
            {
               break;
            }
         }
#endif
         
         // Find all the src using element nodes with using/value
         XPathNodeIterator usingValuesIter = srcUsingValuesNav.Select(Helpers.kXPath_UsingValueSearch);
         
         while (usingValuesIter.MoveNext())
         {
            XPathNavigator usingValue = usingValuesIter.Current;
            // Build path based off names, use string id path to merge values into full property object
            string usingPath = Helpers.BuildUniquePathForValue(usingValue, srcUsingValuesNav);
            
            // Can we find this directly (no layouts)?
            XPathNavigator destValue = destPropertyObjectNav.SelectSingleNode(usingPath);
            if (destValue == null)
            {
               // Nope, expand search by ignoring layouts
               // We could speed this up trying to find match without leading '//' first.
               string fullUsingSearch = usingPath.Replace("/*", "//*");
               XPathNodeIterator destValuesIter = destPropertyObjectNav.Select(fullUsingSearch);
               while (destValuesIter.MoveNext())
               {
                  string destPath = Helpers.BuildUniquePathForValue(destValuesIter.Current, destPropertyObjectNav);
                  // Have we got a match?
                  if (destPath == usingPath)
                  {
                     destValue = destValuesIter.Current;
                     break;
                  }
               }
            }

            // Now find value in full tree properties
            if (destValue != null)
            {
               if (usingPath.Contains(Helpers.kElement_UISettings))
               {
                  // UI Settings, currently just overrides the defaults.
                  destValue.InnerXml = usingValue.InnerXml;
               }
               else
               {
                  // Value
                  destValue.InnerXml = usingValue.InnerXml;
               }
            }
            else
            {
               // Log error about not being able to find matching value
               // Disabled as very slow in debug
               //System.Diagnostics.Debug.Print("Unable to find matching value for merge with {0}.\n", usingPath);
            }
         }
         
         // Append any event entries if available
         {
            XPathNodeIterator destEventsIter = destPropertyObjectNav.Select("//Events/Event");
            while (destEventsIter.MoveNext())
            {
               // See if we've got any of these events in our src object
               XPathNavigator destEventNav = destEventsIter.Current;
               string eventName = destEventNav.GetAttribute(Helpers.kEvent_type, "");
               XPathNavigator destEventComponentNav = destEventNav.Clone();
               destEventComponentNav.MoveToParent();
               string eventXPath = Helpers.BuildUniquePathForValue(destEventComponentNav, null);
               eventXPath += string.Format("/Event[@type=\"{0}\"]", eventName);

               // Has src got any entries for this event
               XPathNavigator srcEventEntries = srcUsingValuesNav.SelectSingleNode(eventXPath);
               if (srcEventEntries != null)
               {
                  // Yep, has events for this
                  // Copy these entries into dest
                  destEventNav.AppendChild(srcEventEntries.InnerXml);
               }
            }
         }

         // Append any link entries if available
         {
            XPathNodeIterator destLinksIter = destPropertyObjectNav.Select("//Links/Link");
            while (destLinksIter.MoveNext())
            {
               // See if we've got any of these links in our src object
               XPathNavigator destLinkNav = destLinksIter.Current;
               string linkName = destLinkNav.GetAttribute(Helpers.kLink_type, "");
               XPathNavigator destLinkComponentNav = destLinkNav.Clone();
               destLinkComponentNav.MoveToParent();
               string linkXPath = Helpers.BuildUniquePathForValue(destLinkComponentNav, null);
               linkXPath += string.Format("/Link[@type=\"{0}\"]", linkName);

               // Has src got any entries for this link
               XPathNavigator srcLinkEntries = srcUsingValuesNav.SelectSingleNode(linkXPath);
               if (srcLinkEntries != null)
               {
                  // Yep, has links for this
                  // Copy these entries into dest
                  destLinkNav.AppendChild(srcLinkEntries.InnerXml);
               }
            }
         }
      }
      
      static void MergeEvaluatorsInternal(XPathNavigator srcUsingValuesNav,
                                          XPathNavigator destPropertyObjectNav,
                                          IPropertyDataSource propertySource,
                                          int componentGroupMaxRecursion)
      {
         // Find all dest properties w/o evaluators below them so we can add empty evaluator element
         while (true)
         {
            XPathNavigator missingValue = destPropertyObjectNav.SelectSingleNode(".//EvaluatorProperty[not(child::Evaluator)]");
            if (missingValue != null)
            {
               // Add empty
               missingValue.AppendChild("<Evaluator id=\"None\"/>");
            }
            else
            {
               // No more missing evaluators
               break;
            }
         }
      
         // Build a dictionary all evaluator paths with navigators.
         // NOTE! This might be a bit slow! Take a look at this later for performance!
         Dictionary<string, XPathNavigator> allDestValues = new Dictionary<string, XPathNavigator>();
         XPathExpression usingEvaluatorXPath = XPathExpression.Compile(".//Property/Value/EvaluatorContainerRoot/EvaluatorContainerEntry/Evaluator | .//EvaluatorProperty/Evaluator");
         {              
            XPathNodeIterator destValuesIter = destPropertyObjectNav.Select(usingEvaluatorXPath);

            while (destValuesIter.MoveNext())
            {
               XPathNavigator destValue = destValuesIter.Current;
               XPathNavigator destValueParent = destValuesIter.Current.Clone();
               //destValueParent.MoveToParent();  // Skip the 'UsingEvaluator' part as property might not have one and we don't care about it's current value
               
               string destPath = Helpers.BuildUniquePathForValue(destValueParent, destPropertyObjectNav);
               // Test for unique path, a duplicate name path is bad
               Debug.Assert(!allDestValues.ContainsKey(destPath));
               allDestValues.Add(destPath, destValue.Clone());
            }
         }
         
         // Find all the src using element nodes with using/value
         XPathExpression evaluatorXPath = XPathExpression.Compile(".//UsingEvaluator");
         
         XPathNodeIterator usingValuesIter = srcUsingValuesNav.Select(evaluatorXPath);
         
         while (usingValuesIter.MoveNext())
         {
            XPathNavigator usingValue = usingValuesIter.Current;
            XPathNavigator usingValueParent = usingValuesIter.Current.Clone();
            //usingValueParent.MoveToParent();  // Skip the 'UsingEvaluator' part as property might not have one and we don't care about it's current value
            // Build path based off names, use string id path to merge values into full property object
            string usingPath = Helpers.BuildUniquePathForValue(usingValueParent, srcUsingValuesNav);
            usingPath = usingPath.Replace("UsingEvaluator", "Evaluator");  // Replace the using version to make path matching work (unique path always appends first element name)

            // Now find value in full tree properties
            if (allDestValues.ContainsKey(usingPath))
            {
               XPathNavigator destValue = allDestValues[usingPath];
               string oldEvaluatorId = destValue.GetAttribute(Helpers.kEvaluator_Id, String.Empty);
               destValue.ReplaceSelf(MergePropertyObject(usingValue, propertySource, componentGroupMaxRecursion, EMergePropertyObjectOptions.kSetOnlyChildDefaults).CreateNavigator());
            }
            else
            {
               // Log error about not being able to find matching value
               // Disabled as very slow in debug
               //System.Diagnostics.Debug.Print("Unable to find matching value for evaluator merge with {0}.\n", usingPath);
            }
         }
         
         // Remove all empty evaluators added above
         while (true)
         {
            XPathNavigator missingValue = destPropertyObjectNav.SelectSingleNode(".//EvaluatorProperty/Evaluator[@id=\"\"]");
            if (missingValue != null)
            {
               // Add empty <Value>
               // Maybe print an error about this as we shouldn't have properties without default values.
               missingValue.DeleteSelf();
            }
            else
            {
               // No more missing property values
               break;
            }
         }
      }
      
      static void MergeEvaluatorContainersInternal(XPathNavigator srcUsingValuesNav,
                                                   XPathNavigator destPropertyObjectNav,
                                                   IPropertyDataSource propertySource,
                                                   int componentGroupMaxRecursion)
                                                   
      {      
         Dictionary<string, XPathNavigator> evaluatorContainers = new Dictionary<string, XPathNavigator>();
         {              
            XPathNodeIterator destValuesIter = destPropertyObjectNav.Select(".//EvaluatorContainerRoot");

            while (destValuesIter.MoveNext())
            {
               XPathNavigator destValue = destValuesIter.Current;
               string destPath = Helpers.BuildUniquePathForValue(destValue, destPropertyObjectNav);
               
               // Test for unique path, a duplicate name path is bad
               Debug.Assert(!evaluatorContainers.ContainsKey(destPath));
               evaluatorContainers.Add(destPath, destValue.Clone());
            }
         }
      
         // Merge in target guids for evaluator containers
         Dictionary<string, XPathNavigator> evaluatorContainerEntries = new Dictionary<string, XPathNavigator>();
         {              
            XPathNodeIterator destValuesIter = destPropertyObjectNav.Select(".//EvaluatorContainerEntry");

            while (destValuesIter.MoveNext())
            {
               XPathNavigator destValue = destValuesIter.Current;
               string destPath = Helpers.BuildUniquePathForValue(destValue, destPropertyObjectNav);
               
               // Test for unique path, a duplicate name path is bad
               Debug.Assert(!evaluatorContainerEntries.ContainsKey(destPath));
               evaluatorContainerEntries.Add(destPath, destValue.Clone());
            }
         }
         {
            XPathNodeIterator containerEntriesIter = srcUsingValuesNav.Select(".//EvaluatorContainerEntry");

            while (containerEntriesIter.MoveNext())
            {
               XPathNavigator containerEntry = containerEntriesIter.Current;
               string containerEntryPath = Helpers.BuildUniquePathForValue(containerEntry, srcUsingValuesNav);

               // If container entry exists in the destination, we only merge target guids and 
               // have it's properties override ours
               if (evaluatorContainerEntries.ContainsKey(containerEntryPath))
               {
                  // Merge target guids
                  XPathNavigator srcEntry = evaluatorContainerEntries[containerEntryPath];
                  XPathNodeIterator targetGuidsIter = containerEntry.Select(Helpers.kElement_TargetGuid);
                  while (targetGuidsIter.MoveNext())
                  {
                     XPathNavigator targetGuidNav = targetGuidsIter.Current;
                     if (!srcEntry.InnerXml.Contains(String.Format("<TargetGuid>{0}</TargetGuid>", targetGuidNav.Value)))
                     {
                        // This target guid doesn't exist
                        srcEntry.PrependChild(targetGuidNav);
                     }
                  }                    
               }
               else
               {
                  // This evaluator container entry doesn't exist, add it
                  XPathNavigator containerRoot = containerEntry.Clone();
                  containerRoot.MoveToParent();
                  // Build path to evaluator container root
                  string containerRootPath = Helpers.BuildUniquePathForValue(containerRoot, srcUsingValuesNav);
                  if (evaluatorContainers.ContainsKey(containerRootPath))
                  {
                     
                     // Insert the new container entry
                     XPathNavigator containerEvaluator = containerEntry.SelectSingleNode(Helpers.kElement_UsingEvaluator);
                     if (containerEvaluator != null)
                     {
                        // Expand UsingEvaluator
                        XPathNavigator containerEvaluatorCopy = Helpers.NavigatorToDocument(containerEvaluator).CreateNavigator();
                        containerEvaluatorCopy.MoveToRoot();
                        containerEvaluator.ReplaceSelf(MergePropertyObject( containerEvaluatorCopy, 
                                                           propertySource, 
                                                                  componentGroupMaxRecursion, 
                                                                  EMergePropertyObjectOptions.kDontSetDefaults).CreateNavigator());
                     }

                     // add container entry even if "UsingEvaluator" element didn't exist,
                     // most of the containers don't actually get evaluators assigned to them until export in maya
                     XPathNavigator evaluatorContainer = evaluatorContainers[containerRootPath];
                     evaluatorContainer.AppendChild(containerEntry);
                  }
               }
            }
         }
      }
      
      /// <summary>
      /// Merges dynamic component groups into property containers.
      /// </summary>
      static void MergeDynamicComponentGroupsInternal(XPathNavigator srcUsingValuesNav,
                                                      XPathNavigator destPropertyObjectNav,
                                                      IPropertyDataSource propertySource)
      {
         // Move to the property container
         XPathNavigator propertyContainerNav = destPropertyObjectNav.SelectSingleNode(Helpers.kElement_PropertyContainer);
         XPathNodeIterator componentGroupIter = srcUsingValuesNav.Select("./UsingComponentGroup[@dynamicComponentGroup=\"true\"]");
         while (componentGroupIter.MoveNext())
         {
            XPathNavigator componentGroupIterNav = componentGroupIter.Current;
            XmlDocument newComponentGroup = MergeProperties.MergePropertyObject(componentGroupIterNav, propertySource);
            if (newComponentGroup == null)
            {
               string name = Helpers.GetDisplayName(componentGroupIterNav);
               string id = componentGroupIterNav.GetAttribute(Helpers.kComponent_Id, String.Empty);
               Console.WriteLine("\nWarning! Skipping unknown dynamic component: {0}/{1}\n{2}\n", id, name, componentGroupIterNav.OuterXml);
               continue;
            }
            XPathNavigator newComponentGroupNav =  newComponentGroup.CreateNavigator();
            newComponentGroupNav.MoveToChild(XPathNodeType.Element);
            propertyContainerNav.AppendChild(newComponentGroupNav);
         }
      }

      public static XmlDocument BuildDerivedPropertyTree(string objectId, 
                                                         string newObjectName,
                                                         XPathNavigator srcNavigator,  // Only used by component groups and can be null
                                                         EBuildTreeSrcType srcType,
                                                         IPropertyDataSource propertySource)
      {
         return BuildDerivedPropertyTree_Internal( objectId,                             
                                                   newObjectName,                           
                                                   srcNavigator,                           
                                                   srcType,
                                                   propertySource,
                                                   kComponentGroupMaxRecursionUninitialized);
      
      }

      private static XmlDocument BuildDerivedPropertyTree_Internal(  string objectId, 
                                                                     string newObjectName,
                                                                     XPathNavigator srcNavigator,  // Only used by component groups and can be null
                                                                     EBuildTreeSrcType srcType,
                                                                     IPropertyDataSource propertySource,
                                                                     int componentGroupMaxRecursion)
      {
         
         XPathNavigator originalPropertyObjectNav = null; // this is src PO, modifing this will change it for everyone! 
         switch (srcType)
         {
            case EBuildTreeSrcType.kPropertyObject:
               originalPropertyObjectNav = propertySource.FindPropertyObjectById(objectId);
               break;
            case EBuildTreeSrcType.kMessageObject:
               originalPropertyObjectNav = propertySource.FindMessageObjectById(objectId);
               // If it's null, we'll assume a message object without properties
               if (originalPropertyObjectNav == null)
               {
                  originalPropertyObjectNav = new XmlDocument().CreateNavigator();
                  // Create the dummy message object
                  originalPropertyObjectNav.AppendChild(String.Format("<MessageObject id=\"{0}\" type=\"NoProperties\"/>", objectId));
               }
               break;
            case EBuildTreeSrcType.kEvaluator:
               originalPropertyObjectNav = propertySource.FindEvaluatorById(objectId);
               break;
            case EBuildTreeSrcType.kComponent:
               {
                  bool bOK = BuildComponent(objectId, propertySource, ref originalPropertyObjectNav);
                  if (!bOK) return null;
               }
               break;
            case EBuildTreeSrcType.kComponentGroup:   // special case, copy each component into group element.
               {
                  bool bOK = BuildComponentGroup(objectId, srcNavigator, propertySource, ref componentGroupMaxRecursion, ref originalPropertyObjectNav);
                  if (!bOK) return null;                  
               }
               break;
            case EBuildTreeSrcType.kPropertyContainer:
               {
                  bool bOK = BuildPropertyContainer(objectId, newObjectName, propertySource, ref originalPropertyObjectNav);
                  if (!bOK) return null;
               }
               break;
         }
               
         if (originalPropertyObjectNav == null)
         {
            // Cannot find propertyObjectId
            return null;
         }
         
         // Create a document copy of original so we can modify it
         XmlDocument srcPropertyObjectDoc = Helpers.NavigatorToDocument(originalPropertyObjectNav);
         XPathNavigator srcPropertyObjectNav = srcPropertyObjectDoc.CreateNavigator();
         
         // Move from root to first element (PropertyObject/Component/ComponentGroup/PropertyContainer)
         srcPropertyObjectNav.MoveToChild(XPathNodeType.Element);
         
         // Add the name attribute (evaluators don't contain names)
         if (srcType != EBuildTreeSrcType.kEvaluator)
         {
            srcPropertyObjectNav.CreateAttribute(String.Empty, Helpers.kProperty_Name, String.Empty, newObjectName);
         }
      
         // Build any descendant properties from created object.
         BuildDescendantProperties(propertySource, componentGroupMaxRecursion, srcPropertyObjectNav);
         
         // Return the full property object
         return srcPropertyObjectDoc;   
      }

      private static void BuildDescendantProperties(IPropertyDataSource propertySource, int componentGroupMaxRecursion, XPathNavigator srcPropertyObjectNav)
      {
         // PropertyContainers can have ComponentGroups / Components / PropertyObjects
         // Components / PropertyObjects / Properties can't contain PropertyContainers

         // ComponentsGroups can have Components / Properties
         // ComponentGroups can't contain PropertyContainers / ComponentGroups 

         // Components can have PropertyObjects / Properties
         // Components can't contain PropertyContainers / ComponentGroups / Components 

         // PropertyObjects can have PropertyObjects / Properties
         // PropertyObjects can't contain PropertyContainers / ComponentGroups / Components

         // MessageObjects can have PropertyObjects / Properties / Evaluators

         // Properties can't have anything below except for user editable data.
         // Evaluators can't have anything below except for properties

         // All objects can have evaluators except for message objects

         // Cannot use an iterator to find Using* as iter entries after the first get invalidated when the document is modified.
         // Have to check for Using* after change

         // Note that we could simplify the code below by building on big xpath, but I think we'll keep it simple for now

         // PropertyContainers
         {
            XPathExpression usingPropertyContainerXPath = XPathExpression.Compile(".//UsingPropertyContainer");
            while (true)
            {
               // search for UsingComponent elements that need replacing
               XPathNavigator scriptObjectDeclarationNav = srcPropertyObjectNav.SelectSingleNode(usingPropertyContainerXPath);
               if (scriptObjectDeclarationNav == null) break;
               scriptObjectDeclarationNav.ReplaceSelf(MergePropertyObject(scriptObjectDeclarationNav, propertySource, componentGroupMaxRecursion, EMergePropertyObjectOptions.kDontSetDefaults).CreateNavigator());
            }
         }

         // ComponentGroups
         if (componentGroupMaxRecursion != 0)
         {
            XPathExpression usingComponentGroupXPath = XPathExpression.Compile(".//UsingComponentGroup");
            while (true)
            {
               // search for UsingComponent elements that need replacing
               XPathNavigator componentGroupDeclarationNav = srcPropertyObjectNav.SelectSingleNode(usingComponentGroupXPath);
               if (componentGroupDeclarationNav == null) break;
               componentGroupDeclarationNav.ReplaceSelf(MergePropertyObject(componentGroupDeclarationNav, propertySource, componentGroupMaxRecursion, EMergePropertyObjectOptions.kDontSetDefaults).CreateNavigator());
            }
         }

         // Components
         {
            XPathExpression usingComponentXPath = XPathExpression.Compile(".//UsingComponent");
            while (true)
            {
               // search for UsingComponent elements that need replacing
               XPathNavigator componentDeclarationNav = srcPropertyObjectNav.SelectSingleNode(usingComponentXPath);
               if (componentDeclarationNav == null) break;
               componentDeclarationNav.ReplaceSelf(MergePropertyObject(componentDeclarationNav, propertySource, componentGroupMaxRecursion, EMergePropertyObjectOptions.kDontSetDefaults).CreateNavigator());
            }
         }

         // PropertyObjects
         {
            XPathExpression usingPropertyObjectXPath = XPathExpression.Compile(".//UsingPropertyObject");
            while (true)
            {
               // search for UsingPropertyObject elements that need replacing
               XPathNavigator propertyObjectDeclarationNav = srcPropertyObjectNav.SelectSingleNode(usingPropertyObjectXPath);
               if (propertyObjectDeclarationNav == null) break;
               propertyObjectDeclarationNav.ReplaceSelf(MergePropertyObject(propertyObjectDeclarationNav, propertySource, componentGroupMaxRecursion, EMergePropertyObjectOptions.kDontSetDefaults).CreateNavigator());
            }
         }

         // Evaluators
         {
            XPathExpression usingXPath = XPathExpression.Compile(".//Property/Value/EvaluatorContainerRoot/EvaluatorContainerEntry/UsingEvaluator | .//EvaluatorProperty/UsingEvaluator");
            while (true)
            {
               // search for UsingEvaluator elements that need replacing
               XPathNavigator declarationNav = srcPropertyObjectNav.SelectSingleNode(usingXPath);
               if (declarationNav == null) break;
               declarationNav.ReplaceSelf(MergePropertyObject(declarationNav, propertySource, componentGroupMaxRecursion, EMergePropertyObjectOptions.kDontSetDefaults).CreateNavigator());
            }
         }
      }

      private static bool BuildPropertyContainer(string objectId, string newObjectName, IPropertyDataSource propertySource, ref XPathNavigator originalPropertyObjectNav)
      {
         originalPropertyObjectNav = propertySource.FindPropertyContainerById(objectId);
         if (originalPropertyObjectNav == null)
         {
            // Couldn't find script object
            return false;
         }

         // Check to see it's a templated script object
         {
            XPathNavigator originalPropertyObjectNavCopy = originalPropertyObjectNav.Clone();
            string template = originalPropertyObjectNavCopy.GetAttribute(Helpers.kProperty_Template, String.Empty);
            if (template != String.Empty)
            {

               //!!!!!!!
               // Note that templates will not merge evaluators correctly
               // I think this will be simple to fix (just call MergePropertyObject and replace name) 
               // but not a priority the moment as we're not using right now.
               //!!!!!!!


               // Yep, it's a templated object, get original and override values

               // Sanity check to see if there's anything but <Using/> below declaration
               System.Diagnostics.Debug.Assert(originalPropertyObjectNavCopy.Select("./*//UsingPropertyContainer | ./*//UsingComponentGroup | ./*//UsingPropertyObject").Count == 0,
                                                "Invalid elements found in templated script object, only <using/> elements allowed.");

               // OK, we've got a template, need to get full templated tree, and replace values
               XPathNavigator templatedNav = BuildDerivedPropertyTree(template,
                                                                        newObjectName,
                                                                        null,
                                                                        EBuildTreeSrcType.kPropertyContainer,
                                                                        propertySource).CreateNavigator();

               // Move off root
               templatedNav.MoveToChild(XPathNodeType.Element);

               // Remove the name as this will be replaced below and also confuse the merge
               templatedNav.MoveToAttribute(Helpers.kProperty_Name, String.Empty);
               templatedNav.DeleteSelf();
               templatedNav.MoveToRoot();

               // Run regular merge with src values
               MergePropertyObjectInternal(originalPropertyObjectNavCopy, templatedNav);

               // Move off root
               templatedNav.MoveToChild(XPathNodeType.Element);

               // Replace original id with that of templated type
               templatedNav.MoveToAttribute(Helpers.kProperty_Id, String.Empty);
               templatedNav.SetValue(objectId);

               // Make this the version to use
               originalPropertyObjectNav = templatedNav;
               originalPropertyObjectNav.MoveToRoot();
            }
         }

         // bCreateComponentEditorId defaults to true if attribute not present
         bool bCreateEditorId = true;

         // Move to script object element so that we can check it's attributes
         // We'll copy the original so we can modify it
         XPathNavigator scriptObjectElementNav = Helpers.NavigatorToDocument(originalPropertyObjectNav).CreateNavigator();
         scriptObjectElementNav.MoveToChild(XPathNodeType.Element); // Move off root
         originalPropertyObjectNav = scriptObjectElementNav.Clone();

         string createEditorId = scriptObjectElementNav.GetAttribute(Helpers.kPropertyContainer_CreateEditorId, String.Empty);
         if (createEditorId.Length != 0)
         {
            bCreateEditorId = bool.Parse(createEditorId);
         }
         if (bCreateEditorId)
         {
            // Add a guid property that is the editor id for the scriptObject, not exported through regular code gen and cooker.
            string newProperty = "<Property name=\"Internal_EditorId\" type=\"guid\" noExport=\"true\"><Value>00000000-0000-0000-0000-000000000000</Value></Property>";
            scriptObjectElementNav.PrependChild(newProperty);
         }
         return true;
      }

      private static bool BuildComponent(string objectId, IPropertyDataSource propertySource, ref XPathNavigator originalPropertyObjectNav)
      {
         // Components require a guid editor id unless explicitly told not to have one.

         // Object id contains 'group:id'
         originalPropertyObjectNav = propertySource.FindComponentByGroupAndId(objectId);
         if (originalPropertyObjectNav == null)
         {
            // Couldn't find component
            return false;
         }

         // bCreateComponentEditorId defaults to true if attribute not present
         bool bCreateEditorId = true;

         // Move to component element so that we can check it's attributes
         // We'll copy the original so we can modify it
         XPathNavigator componentElementNav = Helpers.NavigatorToDocument(originalPropertyObjectNav).CreateNavigator();
         componentElementNav.MoveToChild(XPathNodeType.Element); // Move off root
         originalPropertyObjectNav = componentElementNav.Clone();

         string createEditorId = componentElementNav.GetAttribute(Helpers.kComponent_CreateEditorId, String.Empty);
         if (createEditorId.Length != 0)
         {
            bCreateEditorId = bool.Parse(createEditorId);
         }
         if (bCreateEditorId)
         {
            // Add a guid property that is the editor id for the component, not exported through regular code gen and cooker.
            string newProperty = "<Property name=\"Internal_EditorId\" type=\"guid\" noExport=\"true\"><Value>00000000-0000-0000-0000-000000000000</Value></Property>";
            componentElementNav.PrependChild(newProperty);
         }
         return true;
      }

      private static bool BuildComponentGroup(string objectId, XPathNavigator srcNavigator, IPropertyDataSource propertySource, ref int componentGroupMaxRecursion, ref XPathNavigator originalPropertyObjectNav)
      {
         originalPropertyObjectNav = new XmlDocument().CreateNavigator();
         originalPropertyObjectNav.AppendChildElement(String.Empty, Helpers.kElement_ComponentGroup, String.Empty, String.Empty);
         originalPropertyObjectNav.MoveToChild(XPathNodeType.Element);
         originalPropertyObjectNav.CreateAttribute(String.Empty, Helpers.kComponentGroup_Id, String.Empty, objectId);
         string firstComponentName = String.Empty;
         string defaultComponent = (srcNavigator != null) ? srcNavigator.GetAttribute(Helpers.kComponentGroup_DefaultComponent, String.Empty) : String.Empty;
         bool bGotDefaultComponentMatch = false;
         bool bAllowEmptyComponent = (srcNavigator != null) ? (srcNavigator.GetAttribute(Helpers.kComponentGroup_AllowEmptyComponent, String.Empty) == "true") : false;
         // Dynamic component groups unmerges differently to regular component groups
         bool dynamicComponentGroup = (srcNavigator != null) ? Helpers.IsComponentGroupDynamic(srcNavigator) : false;
         if (dynamicComponentGroup)
         {
            originalPropertyObjectNav.CreateAttribute(String.Empty, Helpers.kComponentGroup_Dynamic, String.Empty, "true");
         }

         // Use parent evaluator allocator
         if ((srcNavigator != null) && srcNavigator.GetAttribute(Helpers.kComponentGroup_UseParentEvaluatorAllocator, String.Empty) == "true") originalPropertyObjectNav.CreateAttribute(String.Empty, Helpers.kComponentGroup_UseParentEvaluatorAllocator, String.Empty, "true");

         // Have we already got a max recursion count?
         if (componentGroupMaxRecursion == kComponentGroupMaxRecursionUninitialized)
         {
            // Nope, initialize it
            // Check to see if it got a max recursion attrib
            string maxRecursion = (srcNavigator != null) ? srcNavigator.GetAttribute(Helpers.kComponentGroup_MaxSubComponents, String.Empty) : String.Empty;
            if (maxRecursion.Length != 0)
            {
               // Set max recursion
               componentGroupMaxRecursion = Int32.Parse(maxRecursion);
            }
            else
            {
               // Set to default, 1
               // Adding the new <ComponentGroupInfo> will allow us to set per component group max recursion.
               componentGroupMaxRecursion = 1;
            }

            // Set max recursion if dynamic component.
            // If we don't do this, then we'll lose our initial value.
            if (dynamicComponentGroup)
            {
               originalPropertyObjectNav.CreateAttribute(String.Empty, Helpers.kComponentGroup_MaxSubComponents, String.Empty, maxRecursion.ToString());
            }
         }
         else
         {
            componentGroupMaxRecursion--;
         }

         List<XPathNavigator> components = propertySource.FindComponentsByGroup(objectId);
         if (components.Count == 0)
         {
            // This component group doesn't exist
            return false;
         }
         // Name attribute will be appended below
         foreach (XPathNavigator srcComponentNav in components)
         {
            //!!!!!!!
            // I think that components inside component groups won't merge evaluators correctly.
            // Need to test this!
            // I think we can fix this by just calling MergePropertyObject and fixing up the name again.
            //!!!!!!!

            XPathNavigator destComponentNav = Helpers.NavigatorToDocument(srcComponentNav).CreateNavigator();
            destComponentNav.MoveToChild(XPathNodeType.Element);
            string id = destComponentNav.GetAttribute(Helpers.kProperty_Id, String.Empty);
            if (id.Length == 0)
            {
               // No good, no id, something very wrong here, everything should have an id
               // Error message here
               continue;
            }

            // As we've only got the type with component groups, we'll derive the name from the component id.
            string name = id;
            if (firstComponentName.Length == 0)
            {
               firstComponentName = name;
            }
            if (name == defaultComponent)
            {
               bGotDefaultComponentMatch = true;
            }
            // Components are now described with their type and id
            string groupAndId = objectId + ":" + id;

            // Now get the xml associated with this id
            XPathNavigator derivedPropertyObjectNav = BuildDerivedPropertyTree_Internal(groupAndId,
                                                                                          name,
                                                                                          null,
                                                                                          EBuildTreeSrcType.kComponent,
                                                                                          propertySource,
                                                                                          componentGroupMaxRecursion).CreateNavigator();

            // Run regular merge on derived componentgroup with src values
            MergePropertyObjectInternal(destComponentNav, derivedPropertyObjectNav);

            // Check to see if this component is using sub components
            // We have to make sure that we don't end up with infinite recursion for adding component groups
            if (derivedPropertyObjectNav.Select("//UsingComponentGroup").Count != 0)
            {
               // Yep, using sub components
               if (componentGroupMaxRecursion == 0)
               {
                  // We've bottomed out on the max recursion, don't add this component.
                  continue;
               }
            }

            // Add short display name if it doesn't already exist
            {
               XPathNavigator componentNav = derivedPropertyObjectNav.SelectSingleNode("./Component");
               if ((componentNav != null) && (componentNav.GetAttribute(Helpers.kProperty_ShortDisplayName, String.Empty).Length == 0))
               {
                  componentNav.CreateAttribute(String.Empty, Helpers.kProperty_ShortDisplayName, String.Empty, groupAndId);
               }
            }

            // Add as child
            originalPropertyObjectNav.AppendChild(derivedPropertyObjectNav.ReadSubtree());
         }
         if (bGotDefaultComponentMatch)
         {
            originalPropertyObjectNav.PrependChildElement(String.Empty, Helpers.kElement_Value, String.Empty, defaultComponent);
            originalPropertyObjectNav.CreateAttribute(String.Empty, Helpers.kComponentGroup_DefaultComponent, String.Empty, defaultComponent);
         }
         else if (bAllowEmptyComponent)
         {
            // Empty, with no default
            // Need to add value element as calling code can't set it as the <Value kDefault="None"> for the unmerge later
            originalPropertyObjectNav.PrependChildElement(String.Empty, Helpers.kElement_Value, String.Empty, "None");
         }
         else
         {
            // No default, no empty, use first component
            originalPropertyObjectNav.PrependChildElement(String.Empty, Helpers.kElement_Value, String.Empty, firstComponentName);

         }
         if (bAllowEmptyComponent)
         {
            // Used by gui, determines if "None" is an available option for the component group
            originalPropertyObjectNav.CreateAttribute(String.Empty, Helpers.kComponentGroup_AllowEmptyComponent, String.Empty, "true");
         }
         return true;
      }

      /// <summary>
      /// This expands 'UsingMessageObject' properties below events and links.
      /// This isn't done by default by regular merge as unmerge doesn't know anything about the application specific events and links.
      /// Do not try to unmerge xml after using this. It is intended for use by the properties cooker.
      /// It will fix all entries supplied navigator. It will work fine with top level script object.
      /// </summary>
      public static void MergeComponentEventsAndLinks(XPathNavigator srcNav,
                                                      IPropertyDataSource propertySource )
      {

         // Append any event entries if available
         {
            XPathNodeIterator messageObjectsIter = srcNav.Select("//Component/Events/Event/EventEntry/UsingMessageObject");
            while (messageObjectsIter.MoveNext())
            {
               // See if we've got any of these events in our src object
               XPathNavigator messageObjectNav = messageObjectsIter.Current;
               // Replace current entry with unmerged data
               messageObjectNav.ReplaceSelf(MergePropertyObject(messageObjectNav, propertySource).CreateNavigator());
            }
         }

         // Append any link entries if available
         {
            XPathNodeIterator messageObjectsIter = srcNav.Select("//Component/Links/Link/LinkEntry/UsingMessageObject");
            while (messageObjectsIter.MoveNext())
            {
               // See if we've got any of these links in our src object
               XPathNavigator messageObjectNav = messageObjectsIter.Current;
               // Replace current entry with unmerged data
               messageObjectNav.ReplaceSelf(MergePropertyObject(messageObjectNav, propertySource).CreateNavigator());
            }
         }
      }
   }
}

