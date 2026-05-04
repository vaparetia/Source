using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Drawing;
using System.Windows.Forms;
using Tools.Property;
using Tools.Property.Misc;
using Tools.Property.Build;

namespace Tools.Property.UI.Controls
{
   public class PropertyLayout : PropertyControl
   {
      public PropertyLayout(PropertyPanel panel, Data.DataNode dataElement, XmlElement uiStateElement)
         : base(panel, dataElement)
      {
         mUIStateElement = uiStateElement;
         AllocateChildStorage();

         Margin = new Padding(3, 3, 0, 3);
      }

      public override void CalculateHeights()
      {
         mControlHeight = 0;

         if (ChildCount > 0)
         {
            mControlHeight += Margin.Vertical;

            foreach (PropertyControl child in Children)
            {
               if (!child.Visible)
                  continue;

               child.CalculateHeights();

               mControlHeight += child.Height;
               mControlHeight += child.Margin.Top;
            }
         }

         mControlHeight += Padding.Vertical;
      }

      public override void CalculateLocations(Rectangle clientRect)
      {
         mLocation = clientRect.Location;

         int currentY = clientRect.Top + Margin.Top;
         int currentX = clientRect.Left + Margin.Left;
         int width = clientRect.Width - Margin.Horizontal;

         foreach (PropertyControl child in Children)
         {
            if (!child.Visible)
               continue;

            Padding childMargin = child.Margin;

            int childHeight = child.Height;
            int childWidth = width - childMargin.Horizontal;

            Rectangle childRect = new Rectangle(currentX + childMargin.Left, currentY + childMargin.Top, childWidth, childHeight);
            child.CalculateLocations(childRect);

            currentY += childHeight + childMargin.Top;
         }
      }

      public override void ProcessRequest(ControlContext context, Request request)
      {
         if (!Visible)
            return;

         Rectangle clientRect = GetClientRect();

         if (!context.ClipBounds.IntersectsWith(clientRect))
            return;

         ProcessRequestSubControls(context, request);

         ProcessRequestChildren(context, request);
      }
   }

   public abstract class BaseGroupBarLayout : PropertyLayout
   {
      const int kControlBarHeight = 25;
      const int kSidePaneWidth = 4;
      const int kDropDownButtonWidth = 19;

      SolidBrush mFillBrush;
      public SolidBrush FillBrush
      {
         get { return mFillBrush; }

         set
         {
            mFillBrush = value;
            mHotBrush = new SolidBrush(Color.FromArgb(128, value.Color));
         }
      }

      SolidBrush mHotBrush;
      public SolidBrush HotBrush
      {
         get { return mHotBrush; }
      }

      public Font Font = Helper.kDefaultFont;
      public Color ForeColor = SystemColors.ControlText;

      public delegate string DisplayNameDelegate(PropertyControl control);
      public DisplayNameDelegate GetDisplayName;

      public bool IsMinimized
      {
         get
         {
            return bool.Parse(MergeLayout.GetUIState(mUIStateElement, "minimized", "False"));
         }

         set
         {
            MergeLayout.SetUIState(mUIStateElement, "minimized", value.ToString(), "False");
         }
      }

      bool mIsMouseOverControl;
      bool mIsPressed;

      public BaseGroupBarLayout(PropertyPanel panel, Data.DataNode dataElement, XmlElement uiStateElement, SubControls.ContextMenuType contextMenuType)
         : base(panel, dataElement, uiStateElement)
      {
         Padding = new Padding(3, 0, 0, 0);
         Margin = new Padding(1, 3, 0, 0);

         AllocateSubControlStorage();
         
         // Bar button
         {
            Controls.SubControls.Custom custom = new Controls.SubControls.Custom(panel);
            
            custom.MouseDown += OnBarMouseDown;
            custom.MouseUp += OnBarMouseUp;
            custom.MouseEnter += OnBarMouseEnter;
            custom.MouseLeave += OnBarMouseLeave;
            custom.Draw += OnBarDraw;
            custom.LostMouseCapture += OnBarLostMouseCapture;

            AddSubControl(custom);
         }

         if (contextMenuType != SubControls.ContextMenuType.None)
         {
            AddSubControl(Helper.CreateContextMenuButton(this, contextMenuType));
         }
      }

      void OnBarDraw(ControlContext context, Rectangle controlRectangle)
      {
         bool isMinimized = IsMinimized;

         Rectangle clientRect = GetClientRect();

         Brush barColorBrush = mIsMouseOverControl ? HotBrush : FillBrush;

         Rectangle barRect = GetBarRect(ref clientRect);

         // Draw control background
         context.Graphics.FillRectangle(barColorBrush, barRect);

         // Draw control text
         string displayName = GetDisplayName(this);

         // Draw expanded state image
         {
            Image image = isMinimized ? Properties.Resources.LayoutBar_Collapsed : Properties.Resources.LayoutBar_Expanded;

            int xOffset = 5;
            int yOffset = (kControlBarHeight - image.Height) / 2;

            System.Drawing.Imaging.ImageAttributes attr = new System.Drawing.Imaging.ImageAttributes();
            attr.SetColorKey(Color.Magenta, Color.Magenta);

            Rectangle destRect = barRect;
            destRect.X = destRect.Left + xOffset;
            destRect.Y = destRect.Top + yOffset;
            destRect.Size = image.Size;

            context.Graphics.DrawImage(image, destRect, 0, 0, image.Width, image.Height, GraphicsUnit.Pixel, attr);
         }

         // Draw text
         {
            int textXOffset = 14;

            barRect.X += textXOffset;
            barRect.Width -= textXOffset;

            TextRenderer.DrawText(context.Graphics, displayName, Font, barRect, ForeColor, TextFormatFlags.VerticalCenter | TextFormatFlags.PreserveGraphicsClipping | TextFormatFlags.EndEllipsis);
         }

         if (!isMinimized)
         {
            // Draw side panel
            int baseHeight = Height - kControlBarHeight;
            Rectangle sidePaneRect = new Rectangle(clientRect.X, clientRect.Y + kControlBarHeight, kSidePaneWidth, baseHeight);

            context.Graphics.FillRectangle(barColorBrush, sidePaneRect);
         }
      }

      void OnBarMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsMouseOverControl = false;
         InvalidateControl();
      }

      void OnBarMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsMouseOverControl = true;
         InvalidateControl();
      }

      void OnBarMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         switch (e.Button)
         {
            case MouseButtons.Left:
               mIsPressed = true;
               mPanel.SetCaptureMouse(controlInfo, true);
               break;

            case MouseButtons.Right:
               {
                  ShowContextMenu(Cursor.Position, ToolStripDropDownDirection.Default);
               }
               break;

         }
      }

      void OnBarMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (e.Button == MouseButtons.Left)
         {
            if (mIsMouseOverControl && mIsPressed)
            {
               SetMinimizedStateRecursive(this, !IsMinimized, Control.ModifierKeys == Keys.Shift);
               mPanel.PropertyInterface.SendUIStateChangedEvents();
               mPanel.InvalidateLayout();
            }

            mPanel.SetCaptureMouse(controlInfo, false);
            mIsPressed = false;
         }
      }

      private void SetMinimizedStateRecursive(PropertyControl currentControl, bool state, bool recursive)
      {
         BaseGroupBarLayout layout = currentControl as BaseGroupBarLayout;
         
         if( layout != null )
         {
            layout.IsMinimized = state;
         }

         if( recursive )
         {
            foreach(PropertyControl child in currentControl.Children)
            {
               SetMinimizedStateRecursive(child, state, recursive);
            }
         }
      }

      void OnBarLostMouseCapture(ControlInfo controlInfo)
      {
         mIsPressed = false;
         mIsMouseOverControl = false;
      }

      public override void CalculateHeights()
      {
         if (IsMinimized)
         {
            mControlHeight = 0;
         }
         else
         {
            // determine height for children
            base.CalculateHeights();
         }

         mControlHeight = Height + kControlBarHeight;
      }

      public override void CalculateLocations(Rectangle clientRect)
      {
         if (!IsMinimized)
         {
            Rectangle subRect = GetSubRectFromClientRect(ref clientRect);
            base.CalculateLocations(subRect);
         }

         // Must do this after calling into base.CalculateLocations otherwise we're storing the location of the child rect.
         mLocation = clientRect.Location;
      }

      private Rectangle GetSubRectFromClientRect(ref Rectangle clientRect)
      {
         Rectangle subRect = clientRect;
         subRect.Offset(kSidePaneWidth, kControlBarHeight);
         subRect.Width -= kSidePaneWidth;
         subRect.Height -= kControlBarHeight;

         subRect.Offset(Padding.Left, Padding.Top);
         subRect.Size -= Padding.Size;

         return subRect;
      }

      protected Rectangle GetBarRect(ref Rectangle clientRect)
      {
         return new Rectangle(clientRect.X, clientRect.Y, clientRect.Width - Constants.kPropertyLayoutContextButtonWidth, kControlBarHeight);
      }

      protected Rectangle GetContextButtonRect(ref Rectangle clientRect)
      {
         return new Rectangle(clientRect.Right - Constants.kPropertyLayoutContextButtonWidth, clientRect.Y, Constants.kPropertyLayoutContextButtonWidth, kControlBarHeight);
      }

      public override void GetSubControlClientRects(out List<Rectangle> subControlClientRects)
      {
         subControlClientRects = new List<Rectangle>();

         Rectangle clientRect = GetClientRect();
         subControlClientRects.Add(GetBarRect(ref clientRect));

         if (SubControlCollection.Count >= 2)
         {
            subControlClientRects.Add(GetContextButtonRect(ref clientRect));
         }
      }

      public override void ProcessRequest(ControlContext context, Request request)
      {
         if (!Visible)
            return;

         Rectangle clientRect = GetClientRect();

         if (!context.ClipBounds.IntersectsWith(clientRect))
            return;

         ProcessRequestSubControls(context, request);

         if (!IsMinimized)
         {
            ProcessRequestChildren(context, request);
         }
      }
   }

   public class GroupLayout : BaseGroupBarLayout
   {
      static SolidBrush kGroupFillBrush = new SolidBrush(Constants.kGroupBarColor);

      public GroupLayout(PropertyPanel panel, Data.DataNode dataElement, XmlElement uiStateElement)
         : base(panel, dataElement, uiStateElement, SubControls.ContextMenuType.None)
      {
         GetDisplayName = OnGetDisplayName;
         FillBrush = kGroupFillBrush;
      }

      public string OnGetDisplayName(PropertyControl control)
      {
         return Helpers.GetDisplayName(mUIStateElement);
      }
   }

   public class GroupBarLayout : BaseGroupBarLayout
   {
      static SolidBrush kGroupBarFillBrush = new SolidBrush(GetLayoutGroupBarColor(0));

      public GroupBarLayout(PropertyPanel panel, Data.DataNode dataElement, XmlElement uiStateElement)
         : base(panel, dataElement, uiStateElement, SubControls.ContextMenuType.None)
      {
         GetDisplayName = OnGetDisplayName;
         FillBrush = kGroupBarFillBrush;
      }

      public string OnGetDisplayName(PropertyControl control)
      {
         return Helpers.GetDisplayName(mUIStateElement);
      }

      private static Color GetLayoutGroupBarColor(int nestingLevel)
      {
         return Tools.Common.Misc.Lerp(Color.FromArgb(185, 185, 155), Color.FromArgb(125, 125, 95), Tools.Common.Misc.Clamp(nestingLevel, 0, 4) / 4.0f);
      }
   }
}
