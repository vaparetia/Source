using System;
using System.Collections.Generic;
using System.Text;
using Tools.Property.Source;
using System.Windows.Forms;
using System.Drawing;
using Tools.Property.Misc;
using Tools.Property.UI.Controls;

namespace Tools.Property.UI
{
   public class PropertyEditorSupport
   {
      public const string kAddComponentButton = "AddComponent";
      public const string kCollapseComponentsButton = "CollapseComponents";
      public const string kSearchEditBox = "Search";

      public delegate bool FilterComponentCallback(String componentGroupId);

      static public void ExtendPropertyEditor(PropertyEditorGUI propertyEditor, string componentType, FilterComponentCallback filterComponentCallback)
      {
         // "Add Component" tool bar button
         {
            ToolStripButton button = new ToolStripButton(Properties.Resources.AddComponent);
            button.Name = kAddComponentButton;
            button.ToolTipText = "Add Component";
            button.ImageTransparentColor = Color.Magenta;

            button.Click += delegate(object sender, EventArgs e)
            {
               AddComponentButtonClicked(sender, propertyEditor, componentType, filterComponentCallback);
            };

            propertyEditor.AddToolBarItem(button);
         }

         // "Collapse Components" tool bar button
         {
            ToolStripButton button = new ToolStripButton(Properties.Resources.CollapseComponents);
            button.Name = kCollapseComponentsButton;
            button.ToolTipText = "Collapse Components";
            button.ImageTransparentColor = Color.Magenta;

            button.Click += delegate(object sender, EventArgs e)
            {
               CollapseComponents(propertyEditor);
            };

            propertyEditor.AddToolBarItem(button);
         }

         // Add search button
         {
            ToolStripTextBox toolStripItem = new ToolStripTextBox(kSearchEditBox);
            Tools.Controls.Misc.SetCueBanner(toolStripItem.TextBox, "Search");
            
            toolStripItem.TextChanged += delegate(object sender, EventArgs e)
            {
               OnSearchTextChanged(propertyEditor, sender, e);
            };

            propertyEditor.AddToolBarItem(toolStripItem);
            propertyEditor.mCallbacks.mPostCreateControls += PostCreateControlsFilterControls;
         }

         propertyEditor.mCallbacks.mUpdateToolBar += OnUpdatePropertyEditorToolBar;
      }

      static void PostCreateControlsFilterControls(PropertyEditorGUI propertyEditor, PropertyControl rootControl)
      {
         ToolStripTextBox textBox = propertyEditor.ToolBar.Items[kSearchEditBox] as ToolStripTextBox;
         if( textBox != null )
         {
            string filterString = textBox.Text;
            filterString = filterString.ToLower();
            filterString = filterString.Trim();

            FilterControls(rootControl, filterString);
         }
      }

      static void OnSearchTextChanged(PropertyEditorGUI propertyEditor, object sender, EventArgs e)
      {
         if( propertyEditor.PropertyPanel.RootControl != null )
         {
            PostCreateControlsFilterControls(propertyEditor, propertyEditor.PropertyPanel.RootControl);
            propertyEditor.PropertyPanel.InvalidateLayout();
         }
      }

      static bool FilterControls(PropertyControl control, string filterString)
      {
         if ( !control.IsLeafControl )
         {
            if( control.ChildCount > 0 )
            {
               bool hasVisibleChildren = false;

               foreach (PropertyControl child in control.Children)
               {
                  if (FilterControls(child, filterString))
                  {
                     hasVisibleChildren = true;
                  }
               }

               control.Visible = hasVisibleChildren;
            }
            else
            {
               control.Visible = true;
            }

            return control.Visible;
         }
         else
         {
            string displayName = control.DataElement.DisplayName;
            bool propertyVisible = (displayName.ToLower().Contains(filterString));
            Data.Property propertyElement = control.DataElement as Data.Property;

            if ( propertyElement != null )
            {
               String valueText = propertyElement.ValueText;

               if ( valueText != "" )
               {
                  propertyVisible |= valueText.ToLower().Contains( filterString );
               }
            }

            control.Visible = propertyVisible;
            
            return propertyVisible;
         }
      }

      static private void OnUpdatePropertyEditorToolBar(PropertyEditorGUI propertyEditor)
      {
         ToolStripItem item = propertyEditor.ToolBar.Items[kAddComponentButton];
         if( item != null )
         {
            item.Enabled = (propertyEditor.Document != null);
         }
      }

      static private void CollapseComponents(PropertyEditorGUI propertyEditor)
      {
         Tools.Property.UI.Controls.PropertyControl rootControl = propertyEditor.PropertyPanel.RootControl;
         if( rootControl != null )
         {
            CollapseTopLevelLayouts(rootControl);
            propertyEditor.PropertyPanel.InvalidateLayout();
         }
      }

      static private void CollapseTopLevelLayouts(Tools.Property.UI.Controls.PropertyControl currentControl)
      {
         Tools.Property.UI.Controls.BaseGroupBarLayout barLayout = currentControl as Tools.Property.UI.Controls.BaseGroupBarLayout;
         if (barLayout != null)
         {
            barLayout.IsMinimized = true;
         }
         else
         {
            foreach ( Tools.Property.UI.Controls.PropertyControl child in currentControl.Children )
            {
               CollapseTopLevelLayouts(child);
            }
         }
         
         currentControl.Panel.PropertyInterface.SendUIStateChangedEvents();
      }

      static private void AddComponentButtonClicked(object sender, PropertyEditorGUI propertyEditor, string componentType, FilterComponentCallback filterComponentCallback)
      {
         EventHandler componentClickHandler = delegate(object buttonSender, EventArgs buttonEventArgs)
         {
            ComponentClicked(buttonSender, propertyEditor, componentType);
         };

         ContextMenuStrip menu = BuildPopupMenu(propertyEditor.mPropertyDB, componentType, componentClickHandler, filterComponentCallback);

         ToolStripItem item = sender as ToolStripItem;
         Rectangle rect = item.Bounds;
         Point menuLocation = item.GetCurrentParent().PointToScreen(new Point(rect.Left, rect.Bottom));

         menu.Show(menuLocation);
      }

      static private void ComponentClicked(object sender, PropertyEditorGUI propertyEditor, string componentType)
      {
         ToolStripItem item = (ToolStripItem)sender;
         KeyValuePair<String, String> groupAndComponent = (KeyValuePair<String, String>)item.Tag;
         if (propertyEditor.mCallbacks.mAddNewComponent != null )
         {
            propertyEditor.mCallbacks.mAddNewComponent(groupAndComponent.Key, groupAndComponent.Value);
         }
      }

      #region Support code for AddComponent
      static public ContextMenuStrip BuildPopupMenu(IPropertyDataSource db, string componentType, EventHandler itemClickedHandler, FilterComponentCallback filterComponentCallback)
      {
         ContextMenuStrip c = new ContextMenuStrip();

         Dictionary<String, List<String>> firstWordToGroupList = new Dictionary<string, List<string>>();

         // Let's get the valid component groups
         foreach (String componentGroupId in db.GetAllComponentGroupIds())
         {
            bool matchesType = false;
            if (filterComponentCallback(componentGroupId))
            {
               foreach (System.Xml.XPath.XPathNavigator componentNav in db.FindComponentsByGroup(componentGroupId))
               {
                  string componentGameObjectType = componentNav.GetAttribute(Helpers.kComponentGroup_GameObjectType, String.Empty);
                  if (componentGameObjectType.StartsWith(componentType))
                  {
                     matchesType = true;
                     break;
                  }
               }
            }

            if (matchesType)
            {
               String word = ExtractFirstWordFromComponentName(componentGroupId);
               if (!firstWordToGroupList.ContainsKey(word))
               {
                  firstWordToGroupList[word] = new List<String>();
               }

               firstWordToGroupList[word].Add(componentGroupId);
            }
         }

         // Sort the components and get the proper beginnings
         foreach (String word in new List<String>(firstWordToGroupList.Keys))
         {
            List<String> componentList = firstWordToGroupList[word];
            componentList.Sort(Tools.Common.Misc.MakeCaseInsensitiveComparer());
            String newWord = GetCommonBeginningOnStrings(componentList);

            if (word != newWord)
            {
               firstWordToGroupList[newWord] = componentList;
               firstWordToGroupList.Remove(word);
            }
         }

         List<String> sortedWordList = new List<String>(firstWordToGroupList.Keys);
         sortedWordList.Sort(Tools.Common.Misc.MakeCaseInsensitiveComparer());

         foreach (String word in sortedWordList)
         {
            ToolStripMenuItem rootItem = new ToolStripMenuItem();
            rootItem.Text = word;
            if (firstWordToGroupList[word].Count > 1)
            {
               rootItem.Text += "...";
            }

            foreach (String group in firstWordToGroupList[word])
            {
               ToolStripMenuItem item;

               if (firstWordToGroupList[word].Count == 1)
               {
                  item = rootItem;
               }
               else
               {
                  item = new ToolStripMenuItem();
                  item.Text = group;
                  rootItem.DropDownItems.Add(item);
               }

               List<String> validComponents = new List<String>();
               foreach (System.Xml.XPath.XPathNavigator componentNav in db.FindComponentsByGroup(group))
               {
                  string componentName = componentNav.GetAttribute(Helpers.kComponent_Id, String.Empty);
                  validComponents.Add(componentName);
               }

               String groupCopy = group; // for delegate purposes

               validComponents.Sort(Tools.Common.Misc.MakeCaseInsensitiveComparer());

               foreach (String component in validComponents)
               {
                  ToolStripMenuItem subItem = new ToolStripMenuItem();

                  subItem.Text = component;
                  subItem.Tag = new KeyValuePair<String, String>(group, component);

                  // Associate tool tip with item
                  {
                     System.Xml.XPath.XPathNavigator componentNav = db.FindComponentByGroupAndId(group + ":" + component, Tools.Property.Source.ESourceVersion.kSourceVersion_2);
                     System.Xml.XmlElement componentXml = componentNav.UnderlyingObject as System.Xml.XmlElement;
                     if (componentXml != null)
                     {
                        subItem.ToolTipText = Helpers.GetToolTipText(componentXml);
                     }
                  }
                  
                  subItem.Click += itemClickedHandler;

                  item.DropDownItems.Add(subItem);
               }
            }

            c.Items.Add(rootItem);
         }
         return c;
      }

      static private String ExtractFirstWordFromComponentName(String s)
      {
         String upperString = s.ToUpper();

         // First scan for all uppercase letters, then all lowercase
         int scan = 0;
         for (; scan < s.Length && s[scan] == upperString[scan]; ++scan)
         {
         }

         for (; scan < s.Length && s[scan] != upperString[scan]; ++scan)
         {
         }

         return s.Substring(0, scan);
      }

      static private String GetCommonBeginning(String s1, String s2)
      {
         int maxLength = Math.Min(s1.Length, s2.Length);
         int equalCount = 0;
         for (; equalCount < maxLength; ++equalCount)
         {
            if (s1[equalCount] != s2[equalCount])
            {
               break;
            }
         }

         return s1.Substring(0, equalCount);
      }

      static private String GetCommonBeginningOnStrings(List<String> items)
      {
         bool first = true;
         String current = "";
         foreach (String item in items)
         {
            if (first)
            {
               current = item;
               first = false;
            }
            else
            {
               current = GetCommonBeginning(item, current);
            }
         }

         return current;
      }
      #endregion
   }
}
