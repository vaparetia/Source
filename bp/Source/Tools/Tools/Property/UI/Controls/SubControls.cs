using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Windows.Forms.VisualStyles;
using System.Runtime.InteropServices;

namespace Tools.Property.UI.Controls.SubControls
{
   public class SubControl : IDisposable
   {
      public enum CommitMode
      {
         Normal            = 1,
         DestroyEditor     = 2,
         ForceApplyValue   = 4
      }

      protected PropertyPanel mPanel;
      protected bool mHasFocus;

      public SubControl(PropertyPanel panel)
      {
         mPanel = panel;
      }

      public virtual void Dispose()
      {
      }

      public virtual bool CanReceiveFocus( ControlInfo controlInfo )
      {
         return false;
      }

      // occurs when control becomes active
      public virtual void OnEnter(ControlInfo controlInfo, FocusReason reason, Rectangle controlRectangle)
      {
         mHasFocus = true;
         mPanel.InvalidateControl(controlInfo);
      }

      // occurs when control becomes inactive
      public virtual void OnLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mHasFocus = false;
         mPanel.InvalidateControl(controlInfo);
      }

      // occurs when the mouse enters the control
      public virtual void OnMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
      }

      // occurs when the mouse leaves the control
      public virtual void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
      }

      // occurs when mouse button is released while inside control
      public virtual void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
      }

      // occurs when mouse button is released while inside control
      public virtual void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
      }

      // occurs when mouse button is moved while inside control
      public virtual void OnMouseMove(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
      }

      // occurs when mouse button is "hovered" while inside control
      public virtual void OnMouseHover(ControlInfo controlInfo, Rectangle controlRectangle)
      {
      }

      public virtual void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
      }

      public virtual bool OnCommit(ControlInfo controlInfo, Rectangle controlRectangle, CommitMode mode)
      {
         return true;
      }

      public virtual void OnLostMouseCapture(ControlInfo controlInfo)
      {
      }

      public virtual void OnKeyDown(KeyEventArgs e, ControlInfo controlInfo, Rectangle rectangle)
      {
      }
      
      public virtual void OnKeyUp(KeyEventArgs e, ControlInfo controlInfo, Rectangle rectangle)
      {
      }
   }

   public class ReadOnlyText : SubControl
   {
      public string Text;

      public ReadOnlyText(PropertyPanel panel)
         : base(panel)
      {
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         Helper.DrawText(Text, Helper.kDefaultFont, SystemBrushes.WindowText, controlRectangle, context.Graphics);
      }

      public Size GetPreferredSize()
      {
         return TextRenderer.MeasureText(Text, Helper.kDefaultFont);
      }
   }

   public class Label : SubControl
   {
      public enum DrawFlags
      {
         Normal,
         Bold
      };

      public enum LabelType
      {
         Normal,
         LinkLabel
      };

      public delegate void LinkClickedDelegate(ControlInfo controlInfo, MouseButtons buttons);
      public delegate void GetLabelDrawFlagsDelegate(out DrawFlags drawFlags);
      public delegate void GetDisplayNameDelegate(out string displayText);
      public delegate void MouseDownDelegate(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle);

      public event LinkClickedDelegate LinkClicked;
      public event MouseDownDelegate ClientRectMouseDown;
      public GetLabelDrawFlagsDelegate GetLabelDrawFlags;
      public GetDisplayNameDelegate GetDisplayName;
      
      LabelType mLabelType;
      bool mIsPressed;
      bool mIsMouseOverLabelText;
      
      public Label(PropertyPanel panel, LabelType labelType)
         : base(panel)
      {
         mLabelType = labelType;
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         string displayText;
         GetDisplayName(out displayText);

         DrawFlags flags;
         GetLabelDrawFlags(out flags);

         switch(mLabelType)
         {
            case LabelType.Normal:
               {
                  Helper.DrawLabel(displayText, SystemBrushes.WindowText, controlRectangle, context.Graphics, false, flags == DrawFlags.Bold, mHasFocus, false);
               }
               break;
            
            case LabelType.LinkLabel:
               {
                  Helper.DrawLabel(displayText, mIsPressed ? Helper.kLinkLabelPressedBrush : Helper.kLinkLabelNormalBrush, controlRectangle, context.Graphics, true, flags == DrawFlags.Bold, mHasFocus, false);
               }
               break;
         }
      }

      public override void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (mLabelType == LabelType.LinkLabel)
         {
            mPanel.Cursor = null;
         }
         mIsMouseOverLabelText = false;
      }

      public override void OnMouseMove(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         bool prevMouseOverLabelText = mIsMouseOverLabelText;

         mIsMouseOverLabelText = IsMouseOverLabelText(e, controlRectangle);

         if( mIsMouseOverLabelText != prevMouseOverLabelText )
         {
            mPanel.InvalidateControl(controlInfo);
         }

         if (mLabelType == LabelType.LinkLabel)
         {
            if (mIsMouseOverLabelText)
            {
               mPanel.Cursor = Cursors.Hand;
            }
            else
            {
               mPanel.Cursor = null;
            }
         }
      }

      public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if( IsMouseOverLabelText(e, controlRectangle))
         {
            mIsPressed = true;
            mPanel.InvalidateControl(controlInfo);

            mPanel.SetCaptureMouse(controlInfo, true);
         }

         if (ClientRectMouseDown != null)
            ClientRectMouseDown(e, controlInfo, controlRectangle);
      }

      public override void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (mIsPressed)
         {
            mIsPressed = false;

            mPanel.SetCaptureMouse(controlInfo, false);
            mPanel.InvalidateControl(controlInfo);

            if (mIsMouseOverLabelText)
            {
               if (LinkClicked != null)
               {
                  LinkClicked(controlInfo, e.Button);
               }
            }
         }
      }

      private bool IsMouseOverLabelText(MouseEventArgs e, Rectangle controlRectangle)
      {
         string displayText;
         GetDisplayName(out displayText);

         DrawFlags flags;
         GetLabelDrawFlags(out flags);

         Rectangle textRect = Helper.GetLabelTextRectangle(displayText, mLabelType == LabelType.LinkLabel, flags == DrawFlags.Bold, controlRectangle);
         return textRect.Contains(e.Location);
      }

      public override void OnKeyDown(KeyEventArgs e, ControlInfo controlInfo, Rectangle rectangle)
      {
         if( e.KeyCode == Keys.Return)
         {
            if (LinkClicked != null)
               LinkClicked(controlInfo, MouseButtons.Left);
         }
      }

      public override bool CanReceiveFocus(ControlInfo controlInfo)
      {
         return mLabelType == LabelType.LinkLabel;
      }
   }

   public class Edit : SubControl, PropertyPanel.IMouseHookClient
   {
      TextBox editBox;
      PropertyPanel.MouseHook hook;

      public delegate string GetValueDelegate();
      public delegate void SetValueDelegate(string newValue);
      public delegate SolidBrush GetBackBrushDelegate();
      public delegate string FormatValueDelegate(string inputValue);
      public delegate bool ValidateValueDelegate(ref string value, out string errorMessage);

      public GetValueDelegate GetValue;
      public SetValueDelegate SetValue;
      public GetBackBrushDelegate GetBackBrush;
      public FormatValueDelegate FormatValue;
      public ValidateValueDelegate ValidateValue;

      public Edit(PropertyPanel panel)
         :  base(panel)
      {
      }

      public void ShowEditor(ControlInfo controlInfo, Rectangle controlRectangle, Point? screenClickPos)
      {
         if (editBox == null)
         {
            editBox = new TextBox();
            editBox.Visible = false;
            editBox.Text = GetFormattedValue();

            editBox.KeyDown += delegate(object sender, KeyEventArgs keyEventArgs)
            {
               OnEditKeyDown(controlInfo, controlRectangle, sender, keyEventArgs);
            };

            mPanel.Controls.Add(editBox);

            editBox.Location = controlRectangle.Location;
            editBox.Size = controlRectangle.Size;

            editBox.Visible = true;
         }

         editBox.BackColor = GetBackBrush().Color;

         if( screenClickPos.HasValue )
         {
            Point clientClickPos = editBox.PointToClient(screenClickPos.Value);
            int lParam = (clientClickPos.Y << 0x10) | (clientClickPos.X & 0xffff);
            Helper.SendMessage(editBox, Tools.Common.WindowsUserNative.WM_LBUTTONDOWN, 0, lParam);
         }
         else
         {
            editBox.Select();
         }
         
         if (hook == null)
         {
            hook = new PropertyPanel.MouseHook(editBox, this, mPanel);
            hook.HookMouseDown = true;
         }
      }

      public override void Dispose()
      {
         TeardownEditor();
      }

      public void TeardownEditor()
      {
         if (editBox != null)
         {
            editBox.Dispose();
            editBox = null;
         }

         if (hook != null)
         {
            hook.Dispose();
            hook = null;
         }
      }

      public override bool CanReceiveFocus(ControlInfo controlInfo)
      {
         return true;
      }

      public override void OnEnter(ControlInfo controlInfo, FocusReason reason, Rectangle controlRectangle)
      {
         ShowEditor(controlInfo, controlRectangle, null);
      }

      public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         ShowEditor(controlInfo, controlRectangle, mPanel.PointToScreen(e.Location));
      }

      public override void OnMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mPanel.Cursor = Cursors.IBeam;
      }

      public override void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mPanel.Cursor = null;
      }

      public override void OnDraw(ControlContext context, Rectangle rectangle)
      {
         rectangle.Size = new Size(rectangle.Width - 1, rectangle.Height - 1);

         context.Graphics.FillRectangle(GetBackBrush(), rectangle);
         context.Graphics.DrawRectangle(new Pen(Helper.mRendererTextBoxNormal.GetColor(System.Windows.Forms.VisualStyles.ColorProperty.BorderColor)), rectangle);

         string valueText = GetFormattedValue();

         if (!string.IsNullOrEmpty(valueText))
         {
            Rectangle backgroundContentRectangle = Helper.mRendererTextBoxNormal.GetBackgroundContentRectangle(context.Graphics, rectangle);
            backgroundContentRectangle.Inflate(-2, -2);
            
            /*
            StringFormat fmt = new StringFormat();
            fmt.Trimming = StringTrimming.EllipsisPath;
            context.Graphics.DrawString(valueText, SystemFonts.DefaultFont, SystemBrushes.ControlText, (RectangleF)backgroundContentRectangle, fmt);
            */

            TextRenderer.DrawText(context.Graphics, valueText, SystemFonts.DefaultFont, backgroundContentRectangle, SystemColors.ControlText, TextFormatFlags.SingleLine | TextFormatFlags.TextBoxControl | TextFormatFlags.NoPadding | TextFormatFlags.PreserveGraphicsClipping);
         }
      }

      public override bool OnCommit(ControlInfo controlInfo, Rectangle controlRectangle, CommitMode mode)
      {
         if (editBox != null)
         {
            bool didValidate = true;
            string errorMessage = String.Empty;

            if (ValidateValue != null)
            {
               string validatedValue = editBox.Text;

               didValidate = ValidateValue(ref validatedValue, out errorMessage);

               if (FormatValue != null)
                  validatedValue = FormatValue(validatedValue);

               // Update the edit box with the formatted output from the validation code.
               // This will be done irregardless if the value could be validated or not.
               editBox.Text = validatedValue;
            }

            // If there is no error this will automatically hide it.
            mPanel.ShowError(editBox, errorMessage);

            if (didValidate)
            {
               if ((mode & CommitMode.ForceApplyValue) != 0 || editBox.Text != GetValue())
               {
                  if (SetValue != null)
                  {
                     SetValue(editBox.Text);
                     mPanel.InvalidateControl(controlInfo);
                  }
               }

               if ((mode & CommitMode.DestroyEditor) != 0)
               {
                  TeardownEditor();
               }
               else
               {
                  ShowEditor(controlInfo, controlRectangle, null);
                  // Update value in editor to be based on the actual value, the setting of the value might have failed for some reason or be overridden otherwise.
                  editBox.Text = GetValue();
                  editBox.SelectAll();
               }

               return true;
            }
            else
            {
               editBox.Focus();
               editBox.SelectAll();

               return false;
            }
         }
         return true;
      }

      public void SetExternalValue(ControlInfo controlInfo, Rectangle controlRectangle, string valueText)
      {
         // Set focus to this edit box.
         mPanel.SetActiveControl(controlInfo, FocusReason.Keyboard);

         // This should ALWAYS be true, but just in case!
         if( editBox != null )
         {
            string originalValue = editBox.Text;

            editBox.Text = valueText;

            if (!mPanel.CommitEdits())
            {
               editBox.Text = originalValue;
            }
         }
      }

      void OnEditKeyDown(ControlInfo controlInfo, Rectangle controlRectangle, object sender, KeyEventArgs e)
      {
         switch (e.KeyCode)
         {
            case Keys.Escape:
               {
                  if( editBox != null )
                  {
                     editBox.Text = GetValue();
                     OnCommit(controlInfo, controlRectangle, CommitMode.Normal);
                  }

                  e.SuppressKeyPress = true;
               }
               break;

            case Keys.Return:
               {
                  OnCommit(controlInfo, controlRectangle, CommitMode.ForceApplyValue);
                  e.SuppressKeyPress = true;
               }
               break;
         }
      }

      public bool OnClickHooked()
      {
         return !mPanel.CommitEdits();
      }

      string GetFormattedValue()
      {
         string valueText = GetValue();

         if (FormatValue != null)
         {
            valueText = FormatValue(valueText);
         }
         
         return valueText;
      }
   }

   public class CheckBox : SubControl
   {
      bool mIsMouseOverControl;
      bool mIsMouseDown;
      bool mIsKeyDown;

      public enum CheckBoxState
      {
         Checked,
         Unchecked,
         Mixed
      };

      public delegate CheckBoxState GetValueDelegate();
      public delegate void SetValueDelegate(CheckBoxState state);

      public GetValueDelegate GetValue;
      public SetValueDelegate SetValue;

      public CheckBox(PropertyPanel panel)
         : base(panel)
      {
      }

      public override bool CanReceiveFocus(ControlInfo controlInfo)
      {
         return true;
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         CheckBoxState state = GetValue();
         Rectangle checkBoxRectangle = controlRectangle;
         checkBoxRectangle.Width = 13;
         Helper.DrawCheckBox(context.Graphics, checkBoxRectangle, state == CheckBoxState.Checked, state == CheckBoxState.Mixed, mHasFocus || mIsMouseDown || mIsMouseOverControl, mIsMouseOverControl && mIsMouseDown || mIsKeyDown);
      }

      public override void OnMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsMouseOverControl = true;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsMouseOverControl = false;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (e.Button == MouseButtons.Left)
         {
            mIsMouseDown = true;
            mPanel.InvalidateControl(controlInfo);
            mPanel.SetCaptureMouse(controlInfo, true);
         }
      }

      public override void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if( e.Button == MouseButtons.Left )
         {
            if (!mIsKeyDown && mIsMouseDown && mIsMouseOverControl)
            {
               ToggleValue();
            }

            mIsMouseDown = false;

            mPanel.SetCaptureMouse(controlInfo, false);
            mPanel.InvalidateControl(controlInfo);
         }
      }

      public override void OnLostMouseCapture(ControlInfo controlInfo)
      {
         mIsMouseDown = false;
         mIsMouseOverControl = false;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnKeyDown(KeyEventArgs e, ControlInfo controlInfo, Rectangle rectangle)
      {
         if( e.KeyCode == Keys.Space )
         {
            mIsKeyDown = true;
            mPanel.InvalidateControl(controlInfo);
         }
      }

      public override void OnKeyUp(KeyEventArgs e, ControlInfo controlInfo, Rectangle rectangle)
      {
         if (e.KeyCode == Keys.Space)
         {
            mIsKeyDown = false;
            if (!mIsMouseDown)
            {
               ToggleValue();
               mPanel.InvalidateControl(controlInfo);
            }
         }
      }

      private void ToggleValue()
      {
         CheckBoxState state = GetValue();
         switch (state)
         {
            case CheckBoxState.Mixed:
            case CheckBoxState.Checked:
               SetValue(CheckBoxState.Unchecked);
               break;

            case CheckBoxState.Unchecked:
               SetValue(CheckBoxState.Checked);
               break;
         }
      }

   }

   public class ComboBox : SubControl, PropertyPanel.IMouseHookClient
   {
      public delegate void GetItemsDelegate(out List<string> items);
      public delegate void GetItemTooltipsDelegate(out List<string> tooltips);
      public delegate string GetValueDelegate();
      public delegate void SetValueDelegate(string valueText);

      public GetItemsDelegate GetItems;
      public GetItemTooltipsDelegate GetItemTooltips;
      public GetValueDelegate GetValue;
      public SetValueDelegate SetValue;

      bool mIsHot;
      bool mIsPressed;
      ContextMenuStrip mMenuStrip;

      public ComboBox(PropertyPanel panel)
         : base(panel)
      {
      }

      bool PropertyPanel.IMouseHookClient.OnClickHooked()
      {
         return !mPanel.CommitEdits();
      }

      public override bool CanReceiveFocus(ControlInfo controlInfo)
      {
         return true;
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         Helper.DrawComboBox(GetValue(), SystemBrushes.WindowText, controlRectangle, context.Graphics, mIsHot, mIsPressed, mHasFocus);
      }

      public override void OnMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsHot = true;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsHot = false;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (e.Button == MouseButtons.Left)
         {
            mIsPressed = true;
            mPanel.InvalidateControl(controlInfo);

            ShowDropDown(ref controlRectangle);
         }
      }

      public override void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if( e.Button == MouseButtons.Left )
         {
            mIsPressed = false;
            mPanel.InvalidateControl(controlInfo);
         }
      }

      public override void OnKeyDown(KeyEventArgs e, ControlInfo controlInfo, Rectangle rectangle)
      {
         switch(e.KeyCode)
         {
            case Keys.Up:
               switch(Control.ModifierKeys)
               {
                  case Keys.Alt:
                     ShowDropDown(ref rectangle);
                     break;
                  
                  case Keys.None:
                     ChangeValue(controlInfo, -1);
                     break;
               }
               break;

            case Keys.Down:
               switch (Control.ModifierKeys)
               {
                  case Keys.Alt:
                     ShowDropDown(ref rectangle);
                     break;

                  case Keys.None:
                     ChangeValue(controlInfo, 1);
                     break;
               }
               break;

            case Keys.Space:
               ShowDropDown(ref rectangle);
               break;
         }
      }

      private void ChangeValue(ControlInfo controlInfo, int direction)
      {
         List<string> items;
         GetItems(out items);
         
         string currentItem = GetValue();
         int currentIndex = items.FindIndex(delegate(string other) { return other == currentItem; } );
         int newIndex = Math.Min(items.Count - 1, Math.Max(0, currentIndex + direction));
         if( currentIndex != newIndex )
         {
            SetValue(items[newIndex]);
            mPanel.InvalidateControl(controlInfo);
         }
      }

      private void ShowDropDown(ref Rectangle controlRectangle)
      {
         CreateDropDown(controlRectangle);

         Point menuPos = new Point(controlRectangle.Left, controlRectangle.Bottom);
         menuPos = mPanel.PointToScreen(menuPos);
         mMenuStrip.Show(menuPos, ToolStripDropDownDirection.BelowRight);
      }

      void CreateDropDown(Rectangle controlRectangle)
      {
         mMenuStrip = new ContextMenuStrip();
         mMenuStrip.MinimumSize = new Size(controlRectangle.Width - 1, 1);
         mMenuStrip.ShowImageMargin = false;
         mMenuStrip.ItemClicked += OnDropDownItemSelected;

         List<string> items;
         GetItems(out items);

         // Get tooltips for items
         List<string> toolTips;
         if (GetItemTooltips != null )
         {
            GetItemTooltips(out toolTips);
         }
         else
         {
            toolTips = new List<string>(items.Count);
            for( int i = 0; i < items.Count; ++i )
            {
               toolTips.Add(string.Empty);
            }
         }

         for (int i = 0; i < items.Count; ++i  )
         {
            AddDropDownEntry(items[i], toolTips[i]);
         }

         string currentItem = GetValue();
         
         for (int i = 0; i < items.Count; ++i)
         {
            if( items[i] == currentItem )
            {
               mMenuStrip.Items[i].Select();
               break;
            }
         }
      }

      void AddDropDownEntry(string text, string toolTip)
      {
         ToolStripMenuItem item = (ToolStripMenuItem)mMenuStrip.Items.Add(text);
         item.AutoSize = false;
         item.ToolTipText = toolTip;
         item.Width = mMenuStrip.Width - 2;
      }

      void OnDropDownItemSelected(object sender, ToolStripItemClickedEventArgs e)
      {
         SetValue(e.ClickedItem.Text);
         mMenuStrip = null;
      }

   }

   public class Button : SubControl
   {
      public delegate void ClickedDelegate(EventArgs e, ControlInfo controlInfo, Rectangle controlRectangle);
      public event ClickedDelegate Clicked;

      public string Text;

      protected bool mIsMouseOverControl;
      protected bool mIsMousePressed;
      protected bool mIsKeyPressed;

      public Button(PropertyPanel panel)
         : base(panel)
      {
      }

      enum ButtonState
      {
         Normal = 1,
         Hot = 2,
         Pressed = 3,
         Disabled = 4,
         Default = 5
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         ButtonState state = ButtonState.Normal;

         if( mHasFocus )
            state = ButtonState.Default;

         if (mIsMouseOverControl || mIsMousePressed)
            state = ButtonState.Hot;

         if (mIsMousePressed && mIsMouseOverControl || mIsKeyPressed)
            state = ButtonState.Pressed;

         VisualStyleRenderer renderer = Helper.mRendererButton;
         renderer.SetParameters(renderer.Class, renderer.Part, (int)state);
         renderer.DrawBackground(context.Graphics, controlRectangle);

         TextRenderer.DrawText(context.Graphics, Text, Helper.kDefaultFont, controlRectangle, SystemColors.ControlText, TextFormatFlags.VerticalCenter | TextFormatFlags.HorizontalCenter | TextFormatFlags.PreserveGraphicsClipping);
      }

      public override bool CanReceiveFocus(ControlInfo controlInfo)
      {
         return true;
      }

      public override void OnKeyDown(KeyEventArgs e, ControlInfo controlInfo, Rectangle rectangle)
      {
         switch (e.KeyCode)
         {
            case Keys.Space:
               {
                  mIsKeyPressed = true;
                  mPanel.InvalidateControl(controlInfo);
               }
               break;
            
            case Keys.Return:
               if (Clicked != null)
               {
                  Clicked(e, controlInfo, rectangle);
               }
               break;
         }
      }

      public override void OnKeyUp(KeyEventArgs e, ControlInfo controlInfo, Rectangle rectangle)
      {
         if( e.KeyCode == Keys.Space )
         {
            mIsKeyPressed = false;
            mPanel.InvalidateControl(controlInfo);

            if( !mIsMousePressed )
            {
               if (Clicked != null)
               {
                  Clicked(e, controlInfo, rectangle);
               }
            }
         }

      }

      public override void OnMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsMouseOverControl = true;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsMouseOverControl = false;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if ((e.Button & MouseButtons.Left) == MouseButtons.Left)
         {
            mIsMousePressed = true;
            mPanel.InvalidateControl(controlInfo);
            mPanel.SetCaptureMouse(controlInfo, true);
         }
      }

      public override void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (e.Button == MouseButtons.Left)
         {
            if (mIsMousePressed)
            {
               mIsMousePressed = false;
               mPanel.SetCaptureMouse(controlInfo, false);
               mPanel.InvalidateControl(controlInfo);

               if( !mIsKeyPressed )
               {
                  if (mIsMouseOverControl)
                  {
                     if (Clicked != null)
                     {
                        Clicked(e, controlInfo, controlRectangle);
                     }
                  }
               }
            }
         }
      }

      public override void OnLostMouseCapture(ControlInfo controlInfo)
      {
         mIsMousePressed = false;
         mIsMouseOverControl = false;
         mPanel.InvalidateControl(controlInfo);
      }
   }

   public enum ContextMenuType
   {
      None,
      Type1,
      Type2,
      Type3
   };

   public class ContextMenuButton : SubControl
   {
      public delegate void ButtonDownDelegate(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle);
      public event ButtonDownDelegate ButtonDown;
      
      ContextMenuType mContextMenuType;
      public ContextMenuType ContextMenuType
      {
         set
         {
            mContextMenuType = value;

            switch (mContextMenuType)
            {
               case ContextMenuType.Type1:
                  mNormalImage = Properties.Resources.ContextButton_Normal;
                  break;

               case ContextMenuType.Type2:
                  mNormalImage = Properties.Resources.ContextButton_NormalAlt;
                  break;

               case ContextMenuType.Type3:
                  mNormalImage = Properties.Resources.ContextButton_NormalAlt2;
                  break;
            }
         }
      }

      Image mNormalImage;
      bool mIsHot;

      public ContextMenuButton(PropertyPanel panel, ContextMenuType type)
         : base(panel)
      {
         ContextMenuType = type;
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         Image image = mNormalImage;
         
         if( mIsHot )
         {
            image = Properties.Resources.ContextButton_Hot;
         }

         Size difference = controlRectangle.Size - image.Size;

         System.Drawing.Imaging.ImageAttributes attr = new System.Drawing.Imaging.ImageAttributes();
         attr.SetColorKey(Color.Magenta, Color.Magenta);

         int yOffset = (controlRectangle.Height - image.Height) / 2;

         Rectangle destRect = controlRectangle;
         destRect.Y += yOffset;
         destRect.Size = image.Size;

         context.Graphics.DrawImage(image, destRect, 0, 0, image.Width, image.Height, GraphicsUnit.Pixel, attr);
      }

      public override void OnMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsHot = true;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mIsHot = false;
         mPanel.InvalidateControl(controlInfo);
      }

      public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if( ButtonDown != null )
         {
            ButtonDown(e, controlInfo, controlRectangle);
         }
      }
   }

   public class Custom : SubControl
   {
      public delegate void MouseEnterDelegate(ControlInfo controlInfo, Rectangle controlRectangle);
      public delegate void MouseLeaveDelegate(ControlInfo controlInfo, Rectangle controlRectangle);
      public delegate void MouseDownDelegate(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle);
      public delegate void MouseUpDelegate(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle);
      public delegate void DrawDelegate(ControlContext context, Rectangle controlRectangle);
      public delegate void LostMouseCaptureDelegate(ControlInfo controlInfo);

      public event MouseEnterDelegate MouseEnter;
      public event MouseLeaveDelegate MouseLeave;
      public event MouseDownDelegate MouseDown;
      public event MouseUpDelegate MouseUp;
      public event DrawDelegate Draw;
      public event LostMouseCaptureDelegate LostMouseCapture;

      public Custom(PropertyPanel panel)
      : base(panel)
      {
      }

      public override void OnMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (MouseEnter != null)
         {
            MouseEnter(controlInfo, controlRectangle);
         }
      }

      public override void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (MouseLeave != null)
         {
            MouseLeave(controlInfo, controlRectangle);
         }
      }

      public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (MouseDown != null)
         {
            MouseDown(e, controlInfo, controlRectangle);
         }
      }

      public override void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (MouseUp != null)
         {
            MouseUp(e, controlInfo, controlRectangle);
         }
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         if( Draw != null )
         {
            Draw(context, controlRectangle);
         }
      }

      public override void OnLostMouseCapture(ControlInfo controlInfo)
      {
         if (LostMouseCapture != null)
         {
            LostMouseCapture(controlInfo);
         }
      }
   }

   public class NumericSlider : SubControl, PropertyPanel.IMouseHookClient
   {
      const int kStartDragThreshold = 5;

      public delegate string GetDisplayValueDelegate();
      public delegate int GetValueDelegate();
      public delegate void SetValueDelegate(int value, PropertyControl.SetValueMode additionalSetValueModeFlags);
      public delegate void CustomDrawBackgroundDelegate(ControlContext context, Rectangle controlRectangle);

      public GetDisplayValueDelegate GetDisplayValue;
      public GetValueDelegate GetValue;
      public SetValueDelegate SetValue;
      public CustomDrawBackgroundDelegate CustomDrawBackground;

      TransparentTextBoxInternal editBox;
      PropertyPanel.MouseHook hook;

      bool mIsPressed;
      int mMin;
      int mMax;
      bool mInDragMode;
      Point mLastMousePos;
      Point mDragBeginMousePos;

      public NumericSlider(PropertyPanel panel, int min, int max)
         : base(panel)
      {
         mMin = min;
         mMax = max;
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         controlRectangle.Width -= 1;
         context.Graphics.FillRectangle(SystemBrushes.Window, controlRectangle);

         int value = GetValue();
         int range = mMax - mMin;
         float fraction = value / (float)range;

         Rectangle fillRect = controlRectangle;
         fillRect.Width = (int)(fillRect.Width * fraction);
         context.Graphics.FillRectangle(SystemBrushes.ControlDark, fillRect);

         if( CustomDrawBackground != null )
         {
            CustomDrawBackground(context, controlRectangle);
         }

         context.Graphics.DrawRectangle(SystemPens.WindowFrame, controlRectangle);

         if( editBox == null )
         {
            if (GetDisplayValue != null)
            {
               controlRectangle.Offset(2, 0);
               string displayValue = GetDisplayValue();
               Helper.DrawText(displayValue, Helper.kDefaultFont, SystemBrushes.WindowText, controlRectangle, context.Graphics);
            }
         }
      }

      public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if( e.Button == MouseButtons.Left)
         {
            mIsPressed = true;
            mPanel.SetCaptureMouse(controlInfo, true);
            mLastMousePos = e.Location;
         }
      }

      public override void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if((e.Button & MouseButtons.Left) == MouseButtons.Left)
         {
            if (mIsPressed && !mInDragMode)
            {
               ShowEditor(controlInfo, controlRectangle);
            }

            if( mInDragMode )
            {
               Cursor.Show();
               Cursor.Position = mDragBeginMousePos;
               // Do a final value change (without actually modifying the value) to let the property know to set the value.
               IncrementCurrentValue(controlInfo, controlRectangle, 0, 0, 0, 0, PropertyControl.SetValueMode.ForceChangeNotification);
            }

            mPanel.SetCaptureMouse(controlInfo, false);
            mIsPressed = false;
            mInDragMode = false;
         }
      }

      public override void OnMouseMove(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if(mIsPressed)
         {
            int maxDelta = e.Location.X - mLastMousePos.X;
            
            if (mInDragMode)
            {
               int direction = maxDelta / 4;

               if (Math.Abs(direction) > 0)
               {
                  IncrementCurrentValue(controlInfo, controlRectangle, direction, 1, 1, 2, PropertyControl.SetValueMode.IsPreview);

                  mLastMousePos = e.Location;
               }

               Cursor.Current = null;
            }
            else if (Math.Abs(maxDelta) > kStartDragThreshold)
            {
               Cursor.Hide();
               mInDragMode = true;

               mDragBeginMousePos = Cursor.Position;

               Rectangle screenBounds = Screen.FromControl(mPanel).Bounds;

               Cursor.Position = new Point(screenBounds.Left + screenBounds.Width / 2, screenBounds.Top + screenBounds.Height / 2);
               mLastMousePos = mPanel.PointToClient(Cursor.Position);
            }
         }
      }

      public override void OnMouseEnter(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if( editBox == null )
         {
            mPanel.Cursor = Cursors.SizeWE;
         }
      }

      public override void OnMouseLeave(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         mPanel.Cursor = Cursors.Default;
      }

      public override void OnEnter(ControlInfo controlInfo, FocusReason reason, Rectangle controlRectangle)
      {
         if (reason == FocusReason.Keyboard)
         {
            ShowEditor(controlInfo, controlRectangle);
         }
      }

      public void ShowEditor(ControlInfo controlInfo, Rectangle controlRectangle)
      {
         if (editBox == null)
         {
            editBox = new TransparentTextBoxInternal();
            editBox.BorderStyle = BorderStyle.None;
            editBox.Text = GetValue().ToString();

            editBox.KeyDown += delegate(object sender, KeyEventArgs keyEventArgs)
            {
               OnEditKeyDown(controlInfo, controlRectangle, sender, keyEventArgs);
            };

            editBox.MouseWheel += delegate(object sender, MouseEventArgs e)
            {
               int direction = (e.Delta > 0) ? 1 : ((e.Delta < 0) ? -1 : 0);
               IncrementCurrentValue(controlInfo, controlRectangle, direction, 1, 8, 32, PropertyControl.SetValueMode.None);
            };

            editBox.Width = controlRectangle.Width - 6;
            editBox.Location = new Point(controlRectangle.Left + 4, controlRectangle.Top + (controlRectangle.Height - editBox.Height) / 2);

            mPanel.Controls.Add(editBox);
            editBox.SelectAll();
            editBox.Select();
         }

         if (hook == null)
         {
            hook = new PropertyPanel.MouseHook(editBox, this, mPanel);
            hook.HookMouseDown = true;
         }
      }

      private void IncrementCurrentValue(ControlInfo controlInfo, Rectangle controlRectangle, int direction, int smallChange, int mediumChange, int bigChange, PropertyControl.SetValueMode additionalSetValueModeFlags)
      {
         int increment = mediumChange;

         bool shiftKeyDown = (Control.ModifierKeys & Keys.Shift) != 0;
         bool controlKeyDown = (Control.ModifierKeys & Keys.Control) != 0;

         if (shiftKeyDown)
         {
            increment = bigChange;
         }
         else if (controlKeyDown)
         {
            increment = smallChange;
         }

         int currentValue = GetValue();
         if (editBox != null)
         {
            int.TryParse(editBox.Text, out currentValue);
         }

         currentValue += direction * increment;
         currentValue = Math.Max(mMin, Math.Min(mMax, currentValue));

         if (editBox != null)
         {
            editBox.Text = currentValue.ToString();
            OnCommit(controlInfo, controlRectangle, CommitMode.Normal);
         }
         else
         {
            SetValue(currentValue, additionalSetValueModeFlags);
         }
      }

      public void TeardownEditor()
      {
         if (editBox != null)
         {
            editBox.Dispose();
            editBox = null;
         }

         if (hook != null)
         {
            hook.Dispose();
            hook = null;
         }
      }

      public override bool OnCommit(ControlInfo controlInfo, Rectangle controlRectangle, CommitMode mode)
      {
         if (editBox != null)
         {
            int intValue = GetValue();
            int.TryParse(editBox.Text, out intValue);
            intValue = Math.Max(mMin, Math.Min(mMax, intValue));
            editBox.Text = intValue.ToString();

            if ((mode & CommitMode.ForceApplyValue) != 0 || editBox.Text != GetValue().ToString())
            {
               if (SetValue != null)
               {
                  SetValue(intValue, PropertyControl.SetValueMode.None);
                  mPanel.InvalidateControl(controlInfo);
               }
            }

            if ((mode & CommitMode.DestroyEditor) != 0)
            {
               TeardownEditor();
            }
            else
            {
               ShowEditor(controlInfo, controlRectangle);
               editBox.SelectAll();
            }
         }

         return true;
      }

      void OnEditKeyDown(ControlInfo controlInfo, Rectangle controlRectangle, object sender, KeyEventArgs e)
      {
         switch (e.KeyCode)
         {
            case Keys.Escape:
               {
                  if (editBox != null)
                  {
                     editBox.Text = GetValue().ToString();
                     OnCommit(controlInfo, controlRectangle, CommitMode.Normal);
                  }

                  e.SuppressKeyPress = true;
               }
               break;

            case Keys.Return:
               {
                  OnCommit(controlInfo, controlRectangle, CommitMode.ForceApplyValue);
                  e.SuppressKeyPress = true;
               }
               break;
         }
      }

      public override bool CanReceiveFocus(ControlInfo controlInfo)
      {
         return true;
      }

      public bool OnClickHooked()
      {
         return !mPanel.CommitEdits();
      }
   }

   public class Text : SubControl
   {
      public string DisplayValue;

      public Text(PropertyPanel panel)
         : base(panel)
      {
      }

      public override void OnDraw(ControlContext context, Rectangle controlRectangle)
      {
         Helper.DrawText(DisplayValue, Helper.kDefaultFont, SystemBrushes.WindowText, controlRectangle, context.Graphics);
      }
   }

   public class TransparentTextBoxInternal : RichTextBox
   {
      public TransparentTextBoxInternal()
      {
         AutoSize = true;
         Multiline = false;
      }

      protected override CreateParams CreateParams
      {
         get
         {
            const int WS_EX_TRANSPARENT = 0x00000020;

            CreateParams cp = base.CreateParams;
            cp.ExStyle |= WS_EX_TRANSPARENT;
            return cp;
         }
      }
   }

}
