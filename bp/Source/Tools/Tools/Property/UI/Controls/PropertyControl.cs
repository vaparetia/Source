using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Drawing;
using System.Windows.Forms;
using Tools.Property;
using Tools.Property.Build;
using Tools.Property.Misc;

namespace Tools.Property.UI.Controls
{
   public class PropertyControl : IDisposable
   {
      public enum Request
      {
         Draw,
         GetLeafControlInfo
      };

      [Flags]
      public enum SetValueMode
      {
         None = 0,
         KeepGroupedState = 0,
         ForceGrouped = 1,
         ForceUngrouped = 2,
         IsPreview = 4,
         ForceChangeNotification = 8
      }

      public SetValueMode kSetValueModeGroupMask = SetValueMode.ForceGrouped | SetValueMode.ForceUngrouped;

      public bool Visible = true;

      public Padding Margin = new Padding(3, 3, 0, 3);
      public Padding Padding = new Padding(0);

      protected PropertyPanel mPanel;
      public PropertyPanel Panel
      {
         get { return mPanel; }
      }

      protected XmlElement mUIStateElement;
      protected Data.DataNode mDataElement;
      public Data.DataNode DataElement
      {
         get { return mDataElement; }
      }

      private List<PropertyControl> mChildren;
      public IEnumerable<PropertyControl> Children
      {
         get 
         {
            if ( mChildren == null )
            {
               return new Tools.Common.Misc.EmptyEnumerable<PropertyControl>();
            }
            else
            {
               return mChildren;
            }
         }
      }
      
      public int ChildCount
      {
         get
         {
            if ( mChildren == null )
            {
               return 0;
            }
            else
            {
               return mChildren.Count;
            }
         }
      }

      protected void ClearChildren()
      {
         if ( mChildren != null )
         {
            foreach ( PropertyControl control in mChildren )
            {
               control.Dispose();
            }

            mChildren.Clear();
         }
      }

      protected void AllocateChildStorage()
      {
         if ( mChildren == null )
         {
            mChildren = new List<PropertyControl>();
         }
      }

      public bool IsLeafControl
      {
         get
         {
            return mChildren == null;
         }
      }

      protected PropertyControl mParent;

      public Helper.BuildContextMenuDelegate BuildContextMenu;

      private List<Controls.SubControls.SubControl> mSubControls;
      public System.Collections.ObjectModel.ReadOnlyCollection<Controls.SubControls.SubControl> SubControlCollection
      {
         get 
         {
            if ( mSubControls == null )
            {
               return null;
            }
            else
            {
               return mSubControls.AsReadOnly();
            }
         }
      }

      protected void AllocateSubControlStorage()
      {
         if ( mSubControls == null )
         {
            mSubControls = new List<Tools.Property.UI.Controls.SubControls.SubControl>();
         }
      }
      protected void AddSubControl( SubControls.SubControl control )
      {
         mSubControls.Add( control );
      }

      protected int mControlHeight = 0;
      protected Point mLocation;

      public int Height
      {
         get { return mControlHeight; }
      }

      public Point Location
      {
         get { return mLocation; }
      }

      public PropertyControl(PropertyPanel panel, Data.DataNode dataElement)
      {
         mPanel = panel;
         mDataElement = dataElement;
      }

      public Rectangle GetClientRect()
      {
         int panelWidth = mPanel.DisplayRectangle.Width;
         Rectangle clientRect = new Rectangle(mLocation, new Size(panelWidth - mLocation.X, mControlHeight));
         return clientRect;
      }

      public void Dispose()
      {
         foreach ( PropertyControl child in new List<PropertyControl>( Children ) )
         {
            child.Dispose();
         }

         if ( mSubControls != null )
         {
            foreach ( SubControls.SubControl control in new List<SubControls.SubControl >( mSubControls ) )
            {
               control.Dispose();
            }
         }
      }

      public virtual void CalculateHeights()
      {
      }

      public virtual void CalculateLocations(Rectangle clientRect)
      {
         mLocation = clientRect.Location;
      }

      public virtual void ProcessRequest(ControlContext context, Request request)
      {
         if (!Visible)
            return;

         Rectangle clientRect = GetClientRect();

         if (!context.ClipBounds.IntersectsWith(clientRect))
            return;

         ProcessRequestSubControls(context, request);
      }

      protected void ProcessRequestSubControls(ControlContext context, Request request)
      {
         if (mSubControls != null)
         {
            List<Rectangle> subControlClientRects;
            GetSubControlClientRects(out subControlClientRects);

            if (request == Request.Draw)
            {
               for (int i = 0; i < mSubControls.Count; ++i)
               {
                  mSubControls[i].OnDraw(context, subControlClientRects[i]);
               }
            }
            else if (request == Request.GetLeafControlInfo)
            {
               for (int i = 0; i < mSubControls.Count; ++i)
               {
                  if (context.ClipBounds.IntersectsWith(subControlClientRects[i]))
                  {
                     context.LeafControlInfo = new ControlInfo(this, i);
                     return;
                  }
               }
            }
         }
      }

      protected void ProcessRequestChildren(ControlContext context, Request request)
      {
         foreach (PropertyControl child in mChildren)
         {
            child.ProcessRequest(context, request);
         }
      }

      public virtual bool CanReceiveFocus(ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            return mSubControls[controlInfo.SubControlIndex].CanReceiveFocus(controlInfo);
         }

         return false;
      }

      // occurs when control becomes active
      public virtual void OnEnter(ControlInfo controlInfo, FocusReason reason)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnEnter(controlInfo, reason, clientRects[controlInfo.SubControlIndex]);
         }
      }

      // occurs when control becomes inactive
      public virtual void OnLeave(ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnLeave(controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      // occurs when the mouse enters the control
      public virtual void OnMouseEnter(ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnMouseEnter(controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      // occurs when the mouse leaves the control
      public virtual void OnMouseLeave(ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnMouseLeave(controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      // occurs when mouse button is released while inside control
      public virtual void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnMouseDown(e, controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      // occurs when mouse button is released while inside control
      public virtual void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnMouseUp(e, controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      // occurs when mouse button is moved while inside control
      public virtual void OnMouseMove(MouseEventArgs e, ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnMouseMove(e, controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      // occurs when mouse button is "hovered" while inside control
      public virtual void OnMouseHover(ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnMouseHover(controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      public virtual void OnKeyDown(KeyEventArgs e, ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnKeyDown(e, controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      public virtual void OnKeyUp(KeyEventArgs e, ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);
            mSubControls[controlInfo.SubControlIndex].OnKeyUp(e, controlInfo, clientRects[controlInfo.SubControlIndex]);
         }
      }

      // occurs when the currently being edited control is to be validated
      public virtual bool OnValidate()
      {
         if (mSubControls != null)
         {
            List<Rectangle> clientRects;
            GetSubControlClientRects(out clientRects);

            for (int i = 0; i < mSubControls.Count; ++i)
            {
               ControlInfo controlInfo = new ControlInfo(this, i);

               if (!mSubControls[i].OnCommit(controlInfo, clientRects[i], SubControls.SubControl.CommitMode.DestroyEditor))
               {
                  return false;
               }
            }
         }

         return true;
      }

      public virtual void OnLostMouseCapture(ControlInfo controlInfo)
      {
         if (mSubControls != null)
         {
            mSubControls[controlInfo.SubControlIndex].OnLostMouseCapture(controlInfo);
         }
      }

      public virtual void GetSubControlClientRects(out List<Rectangle> subControlClientRects)
      {
         subControlClientRects = new List<Rectangle>();
      }

      public void AddChild(PropertyControl control)
      {
         control.mParent = this;
         mChildren.Add(control);
      }

      public bool IsDefaultValue
      {
         get
         {
            return MergePropertiesV2.IsRemovablePropertyValue(mDataElement.Element);
         }

         set
         {
            if (value)
            {
               XmlElement valueElement = mDataElement.Element[Helpers.kElement_Value];
               if (valueElement != null)
               {
                  SetValue(valueElement.GetAttribute(Helpers.kProperty_Value_Default), SetValueMode.KeepGroupedState);
               }
            }
         }
      }

      public bool IsGrouped
      {
         get
         {
            return mDataElement.IsGrouped;
         }

         set
         {
            mDataElement.IsGrouped = value;
         }
      }

      public string GetValue()
      {
         Data.Property propertyNode = mDataElement as Data.Property;
         if ( propertyNode != null )
         {
            return propertyNode.ValueTextNullable;
         }
         else
         {
            return null;
         }
      }

      public void SetValue(string textValue, SetValueMode mode)
      {
         Data.Property propertyNode = mDataElement as Data.Property;

         if ( propertyNode == null )
         {
            return;
         }

         // value has changed if 1) the value is different from the current value OR if the current property is grouped!
         bool wasGrouped = IsGrouped;
         bool didValueChange = (GetValue() != textValue) || wasGrouped;

         string errorMessageIgnored;
         bool isValid = ValidateValue(textValue, out errorMessageIgnored);

         bool sendChangeNotification = false;

         if (didValueChange && isValid)
         {
            sendChangeNotification = true;

            propertyNode.ValueText = textValue;

            switch (mode & kSetValueModeGroupMask)
            {
            case SetValueMode.ForceGrouped:
               propertyNode.IsGrouped = true;
               break;

            case SetValueMode.ForceUngrouped:
               propertyNode.IsGrouped = false;
               break;
            }

            propertyNode.IsModified = true;
         }

         if (sendChangeNotification || (mode & SetValueMode.ForceChangeNotification) == SetValueMode.ForceChangeNotification)
         {
            SendPropertyChangedEvents((mode & SetValueMode.IsPreview) == SetValueMode.IsPreview);
         }
      }

      public virtual bool ValidateValue(string textValue, out string errorMessage)
      {
         errorMessage = string.Empty;
         return true;
      }

      protected void SendPropertyChangedEvents(bool isPreview)
      {
         mPanel.PropertyInterface.SendPropertyChangedEvents(new PropertyEditorGUI.PropertyEditorCallbacks.PropertyChangedEventArgs(this, (Data.Property)mDataElement, isPreview));
      }

      public void ShowContextMenu(Point pos, ToolStripDropDownDirection direction)
      {
         if( BuildContextMenu != null )
         {
            ContextMenuStrip menu = new ContextMenuStrip();

            BuildContextMenu(this, menu);

            if (menu.Items.Count > 0)
            {
               menu.Show(pos, direction);
            }
         }
      }

      public void InvalidateControl()
      {
         Panel.InvalidateControl(this);
      }

      #region Helper code to create sub controls with standard behaviors
      protected SubControls.Label CreatePropertyLabel()
      {
         return CreatePropertyLabelInternal(SubControls.Label.LabelType.Normal);
      }

      protected SubControls.Label CreatePropertyLinkLabel()
      {
         return CreatePropertyLabelInternal(SubControls.Label.LabelType.LinkLabel);
      }

      private SubControls.Label CreatePropertyLabelInternal(SubControls.Label.LabelType labelType)
      {
         SubControls.Label label = new SubControls.Label(mPanel, labelType);
         label.GetDisplayName = delegate(out string displayName)
         {
            displayName = mDataElement.DisplayName;
         };

         label.GetLabelDrawFlags = delegate(out Controls.SubControls.Label.DrawFlags drawFlags)
         {
            drawFlags = IsDefaultValue ? Controls.SubControls.Label.DrawFlags.Normal : Controls.SubControls.Label.DrawFlags.Bold;
         };

         label.ClientRectMouseDown += delegate(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
         {
            if( e.Button == MouseButtons.Right )
            {
               controlInfo.Control.ShowContextMenu(Cursor.Position, ToolStripDropDownDirection.Default);
            }
         };

         return label;
      }

      

      protected SubControls.Edit CreatePropertyEdit()
      {
         SubControls.Edit textEdit = new SubControls.Edit(mPanel);

         textEdit.GetValue = GetValue;

         textEdit.SetValue = delegate(string value)
         {
            SetValue(value, SetValueMode.ForceUngrouped);
         };

         textEdit.GetBackBrush = delegate()
         {
            return IsGrouped ? Constants.kGroupedValueBackBrush : Constants.kUngroupedValueBackBrush;
         };

         return textEdit;
      }

      #endregion
   }
}
