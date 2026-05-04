using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Drawing;
using Tools.Property.Misc;
using System.Windows.Forms;

namespace Tools.Property.UI.Controls
{
   public class ComponentGroupLayout : BaseGroupBarLayout, PropertyPanel.IMouseHookClient
   {
      static SolidBrush[] kDefaultBrushes = 
      {
         new SolidBrush(GetComponentGroupColor(false, 0)),
         new SolidBrush(GetComponentGroupColor(false, 1)),
         new SolidBrush(GetComponentGroupColor(false, 2)),
         new SolidBrush(GetComponentGroupColor(false, 3)),
         new SolidBrush(GetComponentGroupColor(false, 4))
      };

      static SolidBrush[] kPrefabBrushes = 
      {
         new SolidBrush(GetComponentGroupColor(true, 0)),
         new SolidBrush(GetComponentGroupColor(true, 1)),
         new SolidBrush(GetComponentGroupColor(true, 2)),
         new SolidBrush(GetComponentGroupColor(true, 3)),
         new SolidBrush(GetComponentGroupColor(true, 4))
      };

      public Data.ComponentGroup ComponentGroupDataNode
      {
         get { return (Data.ComponentGroup) mDataElement; }
      }

      public ComponentGroupLayout(PropertyPanel panel, Data.DataNode dataElement, XmlElement uiStateElement)
         : base(panel, dataElement, uiStateElement, SubControls.ContextMenuType.Type2)
      {
         GetDisplayName = OnGetDisplayName;
         BuildContextMenu = OnBuildContextMenu;

         const int nestingLevel = 0;
         bool isPrefab = ComponentGroupDataNode.IsPrefab;
         if (isPrefab)
         {
            FillBrush = kPrefabBrushes[nestingLevel];
         }
         else
         {
            FillBrush = kDefaultBrushes[nestingLevel];
         }

         BuildComponentControls();

         PropertyEditorGUI.PropertyEditorCallbacks.PostCreateComponentGroupDelegate postCreateDelegate = mPanel.PropertyInterface.mCallbacks.mPostCreateComponentGroup;
         if (postCreateDelegate != null )
         {
            postCreateDelegate(mPanel.PropertyInterface, this);
         }
      }

      public string OnGetDisplayName(PropertyControl control)
      {
         Data.ComponentGroup group = ComponentGroupDataNode;

         string componentName = group.ActiveComponentType;
         string componentGroupId = group.ID;
         string componentGroupName = group.DisplayName;

         return Tools.GameEngine.RGCHelpers.GetComponentDisplayString(componentGroupId, componentName, componentGroupName);
      }

      static private Color GetComponentGroupColor(bool isPrefab, int nestingLevel)
      {
         // Mark prefab components in a different color
         if (isPrefab)
         {
            return Tools.Common.Misc.Lerp(Constants.kComponentGroupPrefabStartColor, Constants.kComponentGroupPrefabEndColor, Tools.Common.Misc.Clamp(nestingLevel, 0, 4) / 4.0f);
         }
         else
         {
            return Tools.Common.Misc.Lerp(Constants.kComponentGroupStartColor, Constants.kComponentGroupEndColor, Tools.Common.Misc.Clamp(nestingLevel, 0, 4) / 4.0f);
         }
      }

      private void BuildComponentControls()
      {
         // if this property is grouped we don't display any of the properties of our current component
         if ( !mDataElement.IsGrouped )
         {
            Data.Component activeComponent = ComponentGroupDataNode.ActiveComponent;
            if( activeComponent != null )
            {
               mPanel.PropertyInterface.BuildControls(mPanel, this, activeComponent);
            }
         }
      }

      static private ToolStripMenuItem CreateConditionalMenuItem(string text, EventHandler handler, bool enabled)
      {
         ToolStripMenuItem menuItem = new ToolStripMenuItem(text, null, handler);
         menuItem.Enabled = enabled;
         return menuItem;
      }

      void OnBuildContextMenu(PropertyControl control, ContextMenuStrip menu)
      {
         Data.ComponentGroup group = ComponentGroupDataNode;

         if (group.Parent is Data.PropertyContainer)
         {
            // Allow the move up/down/rename delete only on top level component groups

            List<Data.ComponentGroup> componentGroups = GetComponentGroups();
            int myIndex = GetMyComponentIndex(componentGroups);
            int firstMovable = GetFirstMovableComponentIndex(componentGroups);
            menu.Items.Add(CreateConditionalMenuItem("Move &Up", new EventHandler(OnMoveUp), CanMoveItemUp(myIndex, firstMovable)));
            menu.Items.Add(CreateConditionalMenuItem("Move &Down", new EventHandler(OnMoveDown), CanMoveItemDown(myIndex, firstMovable, componentGroups.Count)));
            menu.Items.Add(CreateConditionalMenuItem("&Rename", new EventHandler(OnRename), CanRenameItem(ComponentGroupDataNode)));
            menu.Items.Add(CreateConditionalMenuItem("&Delete", new EventHandler(OnDelete), CanDeleteItem(ComponentGroupDataNode)));

            menu.Items.Add("-");
         }

         string componentGroupId = group.ID;

         List<String> components = group.GetValidComponentTypes( mPanel.PropertyInterface.mPropertyDB );

         bool isPrefabComponent = group.IsPrefab;

         foreach (String component in components)
         {
            string activeComponent = group.ActiveComponentType;

            ToolStripMenuItem item = new ToolStripMenuItem(component);
            
            // Associate tool tip with item
            {
               System.Xml.XPath.XPathNavigator componentNav = mPanel.PropertyInterface.mPropertyDB.FindComponentByGroupAndId(componentGroupId + ":" + component, Tools.Property.Source.ESourceVersion.kSourceVersion_2);
               XmlElement componentXml = componentNav.UnderlyingObject as XmlElement;
               if( componentXml != null )
               {
                  item.ToolTipText = Helpers.GetToolTipText(componentXml);
               }
            }

            item.Tag = component;

            if (component == activeComponent)
               item.Checked = true;
            else
               item.Click += OnChangeComponent;

            item.Enabled = !isPrefabComponent;

            menu.Items.Add(item);
         }
      }

      private void OnChangeComponent(Object sender, EventArgs args)
      {
         ToolStripMenuItem item = sender as ToolStripMenuItem;

         string newComponentId = item.Tag as string;

         mPanel.PropertyInterface.mCallbacks.mChangeCurrentComponentInGroup(ComponentGroupDataNode, newComponentId);
         Panel.PropertyInterface.SendComponentChangedEvents( new PropertyEditorGUI.PropertyEditorCallbacks.ComponentGroupEventArgs( this, ComponentGroupDataNode ) );
      }

      private List<Data.ComponentGroup> GetComponentGroups()
      {
         Data.PropertyContainer parent = (Data.PropertyContainer)mDataElement.Parent;
         return new List<Data.ComponentGroup>(parent.ComponentGroups);
      }

      private int GetFirstMovableComponentIndex(List<Data.ComponentGroup> nodes)
      {
         // The first movable component is the component after the last non-dynamic component AND last prefab component
         // Note that if there is, for some reason, a dynamic component in between any non-dynamic or prefab
         // components, you can't move them.
         int lastNonDynamicOrPrefab = -1;
         for (int i = 0; i < nodes.Count; ++i)
         {
            if ( !nodes[i].IsDynamic || nodes[i].IsPrefab )
            {
               lastNonDynamicOrPrefab = i;
            }
         }

         return lastNonDynamicOrPrefab + 1;
      }

      private int GetMyComponentIndex(List<Data.ComponentGroup> nodes)
      {
         for (int i = 0; i < nodes.Count; ++i)
         {
            if (nodes[i] == mDataElement)
            {
               return i;
            }
         }

         throw new System.Exception("Couldn't get my component index for some reason");
      }

      private static bool CanMoveItemUp(int index, int firstMovable)
      {
         // Make sure we're one past the first movable item
         return index > firstMovable;
      }

      private static bool CanMoveItemDown(int index, int firstMovable, int itemCount)
      {
         return (index >= firstMovable) && (index < itemCount - 1);
      }

      private static bool CanRenameItem(Data.ComponentGroup nav)
      {
         return !nav.IsPrefab && nav.IsDynamic;
      }

      private static bool CanDeleteItem(Data.ComponentGroup nav)
      {
         return !nav.IsPrefab && nav.IsDynamic;
      }

      private void OnMoveUp(Object sender, EventArgs e)
      {
         List<Data.ComponentGroup> nodeList = GetComponentGroups();
         int firstMovable = GetFirstMovableComponentIndex(nodeList);
         int myIndex = GetMyComponentIndex(nodeList);

         if (!CanMoveItemUp(myIndex, firstMovable))
         {
            // Shouldn't get here because item should be disabled, but check just in case.
            return;
         }

         XmlElement layoutOwnerElement = (XmlElement)mDataElement.Element.ParentNode;
         Tools.Property.Build.MergeLayout.MoveLayoutItem(layoutOwnerElement, nodeList[myIndex].Name, -1);

         Data.PropertyContainer props = (Data.PropertyContainer)nodeList[myIndex].Parent;

         props.SwapComponentGroups(nodeList[myIndex], nodeList[myIndex - 1]);

         mPanel.PropertyInterface.RebuildAllControls();
         Panel.PropertyInterface.SendComponentChangedEvents(new PropertyEditorGUI.PropertyEditorCallbacks.ComponentGroupEventArgs(this, ComponentGroupDataNode));
      }

      private void OnMoveDown(Object sender, EventArgs e)
      {
         List<Data.ComponentGroup> nodeList = GetComponentGroups();
         int firstMovable = GetFirstMovableComponentIndex(nodeList);
         int myIndex = GetMyComponentIndex(nodeList);

         if (!CanMoveItemDown(myIndex, firstMovable, nodeList.Count))
         {
            // Shouldn't get here because item should be disabled, but check just in case.
            return;
         }

         XmlElement layoutOwnerElement = (XmlElement)mDataElement.Element.ParentNode;
         Tools.Property.Build.MergeLayout.MoveLayoutItem(layoutOwnerElement, nodeList[myIndex].Name, 1);

         Data.ComponentGroup me = nodeList[myIndex];
         Data.ComponentGroup next = nodeList[myIndex + 1];

         Data.PropertyContainer props = (Data.PropertyContainer)me.Parent;
         props.SwapComponentGroups(me, next);

         Panel.PropertyInterface.RebuildAllControls();
         Panel.PropertyInterface.SendComponentChangedEvents(new PropertyEditorGUI.PropertyEditorCallbacks.ComponentGroupEventArgs(this, ComponentGroupDataNode));
      }

      private TextBox mRenameBox;
      private PropertyPanel.MouseHook mMouseHook;
      private ToolTip mRenameErrorTip;
      private List<String> mRenameProhibitedNames = new List<String>();

      private void OnRename(Object sender, EventArgs e)
      {
         if ( ComponentGroupDataNode.IsDynamic)
         {
            List<Data.ComponentGroup> components = GetComponentGroups();
            int myIndex = GetMyComponentIndex(components);

            mRenameProhibitedNames = new List<String>();

            for (int i = 0; i < components.Count; ++i)
            {
               if (i != myIndex)
               {
                  Data.ComponentGroup check = components[i];

                  mRenameProhibitedNames.Add(check.Name.ToLower());
               }
            }

            SetupRenameBox();
         }
      }

      private void SetupRenameBox()
      {
         if (mRenameBox == null)
         {
            Rectangle clientRect = GetClientRect();
            Rectangle barRect = GetBarRect(ref clientRect);

            mRenameBox = new TextBox();
            mRenameBox.Location = barRect.Location;
            mRenameBox.Size = barRect.Size;
            mRenameBox.Anchor = AnchorStyles.Top|AnchorStyles.Left|AnchorStyles.Right;
            mRenameBox.Text = ComponentGroupDataNode.Name ?? String.Empty;
            mPanel.Controls.Add(mRenameBox);
            mRenameBox.Focus();

            mRenameBox.KeyDown += new KeyEventHandler(OnRenameKeyDown);
            mRenameBox.AcceptsReturn = true;
         }

         if( mMouseHook == null )
         {
            mMouseHook = new PropertyPanel.MouseHook(mRenameBox, this, mPanel);
         }

         mMouseHook.HookMouseDown = true;
      }

      private bool CanRenameTo(String s, ref String outFailureReason)
      {
         String realString = s.Trim();

         if (s == "")
         {
            outFailureReason = "Components cannot have an empty name";
         }
         else if (mRenameProhibitedNames.Contains(s.ToLower()))
         {
            outFailureReason = String.Format("There is already a component group with the name '{0}'", s);
         }
         else if (!Helpers.IsStringSafeAttributeValue(s))
         {
            outFailureReason = "You have some invalid characters in the name";
         }
         else
         {
            return true;
         }

         return false;
      }

      private void CommitRenameIfPossible()
      {
         if (mRenameBox != null)
         {
            String renameText = mRenameBox.Text.Trim();
            String errorText = "";

            TearDownRenameBoxAndTip();

            if (CanRenameTo(renameText, ref errorText))
            {
               string oldName = ComponentGroupDataNode.Name;
               ComponentGroupDataNode.Name = renameText;

               if (mUIStateElement != null)
               {
                  XmlElement layoutOwner = (XmlElement)mDataElement.Element.ParentNode;

                  Tools.Property.Build.MergeLayout.RenameLayoutItem(layoutOwner, oldName, renameText);
               }

               Panel.PropertyInterface.SendComponentChangedEvents(new PropertyEditorGUI.PropertyEditorCallbacks.ComponentGroupEventArgs(this, ComponentGroupDataNode));
            }
         }
      }

      private bool RenameOrShowError()
      {
         String error = "";

         if (CanRenameTo(mRenameBox.Text, ref error))
         {
            CommitRenameIfPossible();
            return true;
         }
         else
         {
            if (mRenameErrorTip == null)
            {
               mRenameErrorTip = new ToolTip();
               mRenameErrorTip.ToolTipIcon = ToolTipIcon.Error;
               mRenameErrorTip.ToolTipTitle = "Error";
            }

            Rectangle clientRect = GetClientRect();
            Rectangle barRect = GetBarRect(ref clientRect);
            Point errorTipLocation = new Point(barRect.Left, barRect.Bottom);

            mRenameErrorTip.Show(error, mPanel, errorTipLocation, 5000 /* milliseconds */ );
            return false;
         }
      }

      void OnRenameKeyDown(object sender, KeyEventArgs e)
      {
         switch (e.KeyCode)
         {
            case Keys.Escape:
               TearDownRenameBoxAndTip();
               break;
            case Keys.Return:
               {
                  RenameOrShowError();
               }
               break;
         }
      }

      private void TearDownRenameBoxAndTip()
      {
         TearDownRenameBoxLeavingTip();

         if (mRenameErrorTip != null)
         {
            mRenameErrorTip.Dispose();
            mRenameErrorTip = null;
         }
      }

      private void TearDownRenameBoxLeavingTip()
      {
         if (mRenameBox != null)
         {
            mRenameBox.Dispose();
            mRenameBox = null;
         }
         if( mMouseHook != null )
         {
            mMouseHook.Dispose();
            mMouseHook = null;
         }
      }

      private void OnDelete(Object sender, EventArgs e)
      {
         Data.ComponentGroup group = ComponentGroupDataNode;

         if ( group.IsDynamic)
         {
            string groupId = group.ID;
            string componentId = group.ActiveComponentType;
            string componentGroupName = group.Name;

            if (mPanel.PropertyInterface.mCallbacks.mDeleteComponentGroup != null )
               mPanel.PropertyInterface.mCallbacks.mDeleteComponentGroup(groupId, componentId, componentGroupName);
         }
      }

      /*
      // Currently unused, because work is actually done in delegate that is called.
      void SetComponent(string componentName, bool forceSetComponent)
      {
         // Remove current control children.
         mChildren.Clear();

         if (forceSetComponent || Helpers.GetActiveComponentNameFromComponentGroup(mDataElement) != componentName)
         {
            Helpers.SetActiveComponentInComponentGroup(componentName, mDataElement);
            Helpers.SetPropertyGrouped(mDataElement, false);
            Helpers.SetPropertyModified(mDataElement, true);
         }

         BuildComponentControls();
      }
      */

      public bool OnClickHooked()
      {
         if (mRenameBox != null)
         {
            return !RenameOrShowError();
         }

         return false;
      }
   }
}
