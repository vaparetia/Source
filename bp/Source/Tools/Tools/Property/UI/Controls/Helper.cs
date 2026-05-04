using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Windows.Forms.VisualStyles;
using System.Windows.Forms;

namespace Tools.Property.UI.Controls
{
   public enum FocusReason
   {
      Keyboard,
      Mouse
   };

   internal class Constants
   {
      internal const int kPropertyLayoutColumn1Percentage = 40;
      internal const int kPropertyLayoutColumn2Percentage = 60;
      internal const int kPropertyLayoutColumnPadding = 4;

      internal const int kPropertyLayoutContextButtonWidth = 14;
      internal const int kPropertyLayoutContextButtonHeight = 20;

      internal static Color kComponentGroupPrefabStartColor = Color.FromArgb(185, 185, 185);
      internal static Color kComponentGroupPrefabEndColor = Color.FromArgb(125, 125, 125);

      internal static Color kComponentGroupStartColor = Color.FromArgb(155, 185, 235);
      internal static Color kComponentGroupEndColor = Color.FromArgb(185, 155, 235);

      internal static Color kGroupBarColor = Color.FromArgb(210, 210, 210);
      
      internal static SolidBrush kGroupedValueBackBrush = new SolidBrush(Color.LightGray);
      internal static SolidBrush kUngroupedValueBackBrush = new SolidBrush(SystemColors.Window);
      internal static SubControls.ContextMenuType kEvaluatorContextMenuType = SubControls.ContextMenuType.Type3;
   }

   public class Helper
   {
      private enum COMBOBOXPARTS
      {
         CP_DROPDOWNBUTTON = 1,
         CP_BACKGROUND = 2,
         CP_TRANSPARENTBACKGROUND = 3,
         CP_BORDER = 4,
         CP_READONLY = 5,
         CP_DROPDOWNBUTTONRIGHT = 6,
         CP_DROPDOWNBUTTONLEFT = 7,
         CP_CUEBANNER = 8,
      }; 

      internal static VisualStyleRenderer mRendererCheckBoxCheckedNormal = new VisualStyleRenderer( VisualStyleElement.Button.CheckBox.CheckedNormal );
      internal static VisualStyleRenderer mRendererCheckBoxMixedNormal = new VisualStyleRenderer(VisualStyleElement.Button.CheckBox.MixedNormal);
      internal static VisualStyleRenderer mRendererCheckBoxUncheckedNormal = new VisualStyleRenderer(VisualStyleElement.Button.CheckBox.UncheckedNormal);

      internal static VisualStyleRenderer mRendererCheckBoxCheckedHot = new VisualStyleRenderer(VisualStyleElement.Button.CheckBox.CheckedHot);
      internal static VisualStyleRenderer mRendererCheckBoxMixedHot = new VisualStyleRenderer(VisualStyleElement.Button.CheckBox.MixedHot);
      internal static VisualStyleRenderer mRendererCheckBoxUncheckedHot = new VisualStyleRenderer(VisualStyleElement.Button.CheckBox.UncheckedHot);

      internal static VisualStyleRenderer mRendererCheckBoxCheckedPressed = new VisualStyleRenderer(VisualStyleElement.Button.CheckBox.CheckedPressed);
      internal static VisualStyleRenderer mRendererCheckBoxMixedPressed = new VisualStyleRenderer(VisualStyleElement.Button.CheckBox.MixedPressed);
      internal static VisualStyleRenderer mRendererCheckBoxUncheckedPressed = new VisualStyleRenderer(VisualStyleElement.Button.CheckBox.UncheckedPressed);

      internal static VisualStyleRenderer mRendererButton = new VisualStyleRenderer(VisualStyleElement.Button.PushButton.Normal);

      internal static VisualStyleRenderer mRendererTextBoxNormal = new VisualStyleRenderer(VisualStyleElement.TextBox.TextEdit.Normal);

      internal static VisualStyleRenderer mRendererComboBoxBackground = CreateVisualStyleRendererIfPossible( "COMBOBOX", (int) COMBOBOXPARTS.CP_READONLY, 1 );
      internal static VisualStyleRenderer mRendererComboBoxRight = CreateVisualStyleRendererIfPossible( "COMBOBOX", (int) COMBOBOXPARTS.CP_DROPDOWNBUTTONRIGHT, 1 );

      internal static Font kDefaultFont = SystemFonts.DefaultFont; //;DialogFont;
      internal static Font kBoldDefaultFont = new Font( kDefaultFont, FontStyle.Bold );
      internal static Font kUnderlineDefaultFont = new Font( kDefaultFont, FontStyle.Underline );
      internal static Font kBoldUnderlineDefaultFont = new Font( kDefaultFont, FontStyle.Bold | FontStyle.Underline );

      internal static Brush kLinkLabelNormalBrush = new SolidBrush(Color.FromArgb(0, 0, 255));
      internal static Brush kLinkLabelPressedBrush = new SolidBrush(Color.FromArgb(255, 0, 0));

      internal static StringFormat kLabelStringFormatSettings = CreateLabelStringFormatSettings();

      private static StringFormat CreateLabelStringFormatSettings()
      {
         StringFormat fmt = new StringFormat();
         fmt.LineAlignment = StringAlignment.Center;
         fmt.FormatFlags = StringFormatFlags.NoWrap;
         fmt.Trimming = StringTrimming.EllipsisCharacter;
         
         return fmt;
      }

      private static VisualStyleRenderer CreateVisualStyleRendererIfPossible( String style, int part, int state )
      {
         VisualStyleElement element = VisualStyleElement.CreateElement( style, part, state );

         if ( VisualStyleRenderer.IsElementDefined( element ) )
         {
            return new VisualStyleRenderer( element );
         }
         else
         {
            return null;
         }
      }

      internal static void DrawText(string text, Font font, Brush brush, Rectangle clientRect, Graphics graphics)
      {
         Size textSize = graphics.MeasureString(text, font).ToSize();
         graphics.DrawString(text, font, brush, new PointF(clientRect.Left, clientRect.Top + (clientRect.Height - textSize.Height) / 2));
      }

      internal static Font GetLabelFont(bool isUnderline, bool isBold)
      {
         Font font;

         if (isBold)
         {
            if (isUnderline)
               font = kBoldUnderlineDefaultFont;
            else
               font = kBoldDefaultFont;
         }
         else
         {
            if (isUnderline)
               font = kUnderlineDefaultFont;
            else
               font = kDefaultFont;
         }

         return font;
      }

      internal static Rectangle GetLabelTextRectangle(string text, bool isUnderline, bool isBold, Rectangle clientRect)
      {
         Font font = GetLabelFont(isUnderline, isBold);

         Size measuredSize = TextRenderer.MeasureText(text, font, clientRect.Size);

         Rectangle textRect = new Rectangle(clientRect.Location, measuredSize);
         textRect.Offset(0, (clientRect.Height - measuredSize.Height) / 2);
         
         return textRect;
      }

      internal static void DrawLabel(string text, Brush brush, Rectangle clientRect, Graphics graphics, bool isUnderline, bool isBold, bool hasFocus, bool isHot)
      {
         if ( isHot )
         {
            using ( Brush gradient = new System.Drawing.Drawing2D.LinearGradientBrush(clientRect, SystemColors.Window, SystemColors.Control, 0.0f) )
            {
               graphics.FillRectangle(gradient, clientRect);
            }
         }

         Font font = GetLabelFont(isUnderline, isBold);

         Size textSize = graphics.MeasureString(text, font).ToSize();

         graphics.DrawString(text, font, brush, (RectangleF)clientRect, kLabelStringFormatSettings);
         
         if( hasFocus )
         {
            Rectangle focusRect = clientRect;
            focusRect.Size = textSize;
            focusRect.Offset(2, 2);
            using(Pen pen = new Pen(SystemColors.WindowText) )
            {
               pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;
               graphics.DrawRectangle(pen, focusRect);
            }
         }

         int spacing = 3;
         if (textSize.Width + spacing < clientRect.Width)
         {
            Point startingPoint = new Point(clientRect.Left + textSize.Width + spacing, clientRect.Top + (clientRect.Height + textSize.Height) / 2 - 2);

            using (Pen pen = new Pen(SystemColors.GrayText))
            {
               pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Custom;
               pen.DashPattern = new float[] { 3.0f, 2.0f };

               graphics.DrawLine(pen, startingPoint.X, startingPoint.Y, clientRect.Right, startingPoint.Y);
            }
         }
      }

      internal static void DrawCheckBox(Graphics graphics, Rectangle clientRect, bool isChecked, bool isGrouped, bool isHot, bool isPressed)
      {
         VisualStyleRenderer renderer;

         if( isPressed )
         {
            if (isGrouped)
               renderer = Helper.mRendererCheckBoxMixedPressed;
            else if (isChecked)
               renderer = Helper.mRendererCheckBoxCheckedPressed;
            else
               renderer = Helper.mRendererCheckBoxUncheckedPressed;
         }
         else if (isHot)
         {
            if (isGrouped)
               renderer = Helper.mRendererCheckBoxMixedHot;
            else if (isChecked)
               renderer = Helper.mRendererCheckBoxCheckedHot;
            else
               renderer = Helper.mRendererCheckBoxUncheckedHot;
         }
         else
         {
            if (isGrouped)
               renderer = Helper.mRendererCheckBoxMixedNormal;
            else if (isChecked)
               renderer = Helper.mRendererCheckBoxCheckedNormal;
            else
               renderer = Helper.mRendererCheckBoxUncheckedNormal;
         }

         if (renderer != null)
         {
            renderer.DrawBackground(graphics, clientRect);
         }
      }

      internal static void DrawComboBox(string text, Brush brush, Rectangle clientRect, Graphics graphics, bool isHot, bool isPressed, bool isFocus)
      {
         ComboBoxState state = isPressed ? ComboBoxState.Pressed : (isHot ? ComboBoxState.Hot : ComboBoxState.Normal);

         if ( mRendererComboBoxRight != null && mRendererComboBoxBackground != null )
         {
            mRendererComboBoxRight.SetParameters( mRendererComboBoxRight.Class, mRendererComboBoxRight.Part, (int) ComboBoxState.Normal );
            mRendererComboBoxBackground.SetParameters( mRendererComboBoxBackground.Class, mRendererComboBoxBackground.Part, (int) state );

            Size dropdownSize = mRendererComboBoxRight.GetPartSize( graphics, clientRect, ThemeSizeType.True );
            
            mRendererComboBoxBackground.DrawBackground( graphics, clientRect );
            mRendererComboBoxRight.DrawBackground( graphics, new Rectangle(
               clientRect.Right - dropdownSize.Width, clientRect.Y,
               dropdownSize.Width, clientRect.Height ) );

            Rectangle textRect = mRendererComboBoxBackground.GetBackgroundContentRectangle( graphics, clientRect );
            textRect.Width -= dropdownSize.Width;

            StringFormat fmt = new StringFormat();
            fmt.LineAlignment = StringAlignment.Center;
            fmt.Alignment = StringAlignment.Near;
            fmt.FormatFlags = StringFormatFlags.NoWrap;
            fmt.Trimming = StringTrimming.EllipsisCharacter;

            graphics.DrawString(text, kDefaultFont, SystemBrushes.WindowText, (RectangleF)textRect, fmt);
         }
         else
         {
            Brush foreBrush = SystemBrushes.WindowText;
            Brush backBrush = SystemBrushes.Window;

            if (isFocus)
            {
               foreBrush = SystemBrushes.HighlightText;
               backBrush = SystemBrushes.Highlight;
            }

            VisualStyleRenderer renderer = new VisualStyleRenderer(VisualStyleElement.ComboBox.DropDownButton.Normal);
            Size dropdownSize = renderer.GetPartSize( graphics, clientRect, ThemeSizeType.True );

            VisualStyleRenderer textBoxRenderer = new VisualStyleRenderer(VisualStyleElement.TextBox.TextEdit.Normal);
            Rectangle textRect = textBoxRenderer.GetBackgroundContentRectangle(graphics, clientRect);
            textRect.Width -= dropdownSize.Width + 2;
            textRect.Inflate(-2, -2);

            ComboBoxRenderer.DrawTextBox(graphics, clientRect, "", kDefaultFont, TextFormatFlags.PreserveGraphicsClipping, state);
            graphics.FillRectangle(backBrush, textRect);

            graphics.DrawString(text, kDefaultFont, foreBrush, textRect);
            
            Rectangle dropDownRect = new Rectangle(clientRect.Right - 18, clientRect.Top + 1, 17, 18);
            ComboBoxRenderer.DrawDropDownButton(graphics, dropDownRect, state);
         }
      }

      public static List<Rectangle> GetDefaultPropertyLayoutRects(Rectangle clientRect)
      {
         int width = (clientRect.Right * Constants.kPropertyLayoutColumn2Percentage) / 100;
         return GetDefaultPropertyLayoutRects(clientRect, width);
      }

      public static List<Rectangle> GetDefaultPropertyLayoutRects(Rectangle clientRect, int valueControlWidth)
      {
         List<Rectangle> subControlClientRects;
         subControlClientRects = new List<Rectangle>();

         int labelWidth = 0;
         int valueWidth = valueControlWidth;
         int valueOffset = 0;

         // Determine positions and sizes.
         {
            bool hasFixedValueSize = (valueControlWidth > 0);
            int totalWidth = clientRect.Width;
            Helper.CalculatePropertySplit(totalWidth - Constants.kPropertyLayoutContextButtonWidth, ref labelWidth, ref valueWidth, ref valueOffset, hasFixedValueSize);
         }

         // Label
         {
            Rectangle labelRect = clientRect;
            labelRect.Width = labelWidth;

            subControlClientRects.Add(labelRect);
         }

         // Value field
         {
            Rectangle valueFieldRect = clientRect;
            valueFieldRect.Width = valueWidth;
            valueFieldRect.X = clientRect.Left + valueOffset;

            subControlClientRects.Add(valueFieldRect);
         }

         // Context button
         {
            Rectangle contextButtonRect = clientRect;
            contextButtonRect.X = contextButtonRect.Right - Constants.kPropertyLayoutContextButtonWidth;
            contextButtonRect.Width = Constants.kPropertyLayoutContextButtonWidth;
            contextButtonRect.Y = clientRect.Top + (contextButtonRect.Height - Constants.kPropertyLayoutContextButtonHeight) / 2;
            contextButtonRect.Height = Constants.kPropertyLayoutContextButtonHeight;

            subControlClientRects.Add(contextButtonRect);
         }

         return subControlClientRects;
      }

      internal static void CalculatePropertySplit(int totalWidth, ref int column1Width, ref int column2Width, ref int column2Offset, bool colum2Fixed)
      {
         if (!colum2Fixed)
         {
            int widthMinusPadding = totalWidth - Constants.kPropertyLayoutColumnPadding;

            column1Width = widthMinusPadding * Constants.kPropertyLayoutColumn1Percentage / 100;
            column2Width = widthMinusPadding * Constants.kPropertyLayoutColumn2Percentage / 100;
            column2Offset = column1Width + Constants.kPropertyLayoutColumnPadding;
         }
         else
         {
            column2Offset = totalWidth - column2Width;
            column1Width = totalWidth - Constants.kPropertyLayoutColumnPadding - column2Width;
         }
      }

      internal static IntPtr SendMessage(Control control, int msg, int wparam, int lparam)
      {
         return Tools.Common.WindowsUserUnsafe.SendMessage(new System.Runtime.InteropServices.HandleRef(control, control.Handle),
                                                           msg, (IntPtr)wparam, (IntPtr)lparam);
      }

      public delegate void BuildContextMenuDelegate(PropertyControl control, ContextMenuStrip menu);

      public static SubControls.ContextMenuButton CreateContextMenuButton(PropertyControl control, SubControls.ContextMenuType type)
      {
         SubControls.ContextMenuButton button = new SubControls.ContextMenuButton(control.Panel, type);

         button.ButtonDown += delegate(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
         {
            Point pos = control.Panel.PointToScreen(controlRectangle.Location);
            control.ShowContextMenu(pos, ToolStripDropDownDirection.Left);
         };

         return button;
      }

      public static void BuildCustomEditorsContextMenu(PropertyControl control, ContextMenuStrip menu)
      {
         PropertyEditorGUI.PropertyEditorCallbacks.GetCustomEditorsEventArgs eventArgs = new PropertyEditorGUI.PropertyEditorCallbacks.GetCustomEditorsEventArgs();

         if ( control.Panel.PropertyInterface.mCallbacks.mGetCustomEditors != null )
         {
            control.Panel.PropertyInterface.mCallbacks.mGetCustomEditors( control, eventArgs );
         }

         if( eventArgs.CustomEditorList.Count > 0 )
         {
            // Add separator if necessary
            if (menu.Items.Count > 0)
               menu.Items.Add("-");

            foreach (PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor editor in eventArgs.CustomEditorList)
            {
               ToolStripMenuItem editorItem = new ToolStripMenuItem(editor.Info);
               editorItem.ToolTipText = editor.ToolTip;

               if (editor.EditorFunction == null)
               {
                  editorItem.Enabled = false;
               }
               else
               {
                  PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor.SpawnEditorDelegate callback = (PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor.SpawnEditorDelegate)editor.EditorFunction.Clone();
                  editorItem.Click += delegate(object sender, EventArgs e)
                  {
                     string newPropertyValue = callback(control);
                     if( !string.IsNullOrEmpty(newPropertyValue) )
                     {
                        control.SetValue(newPropertyValue, PropertyControl.SetValueMode.ForceUngrouped);
                        control.InvalidateControl();
                     }
                  };
               }

               menu.Items.Add(editorItem);
            }
         }
      }

      public static void BuildResetToDefaultContextMenu(PropertyControl control, ContextMenuStrip menu)
      {
         BuildResetToDefaultContextMenu(control, menu, "Reset to default");
      }

      public static void BuildResetToDefaultContextMenu(PropertyControl control, ContextMenuStrip menu, string menuEntryDisplayText)
      {
         if (menu.Items.Count > 0)
            menu.Items.Add("-");

         menu.Items.Add(menuEntryDisplayText,
                        null,
                        delegate(object sender, EventArgs e)
                        {
                           control.IsDefaultValue = true;
                           control.InvalidateControl();
                        });
      }

      public static void DefaultPropertyBuildContextMenu(PropertyControl control, ContextMenuStrip menu)
      {
         BuildCustomEditorsContextMenu(control, menu);
         BuildResetToDefaultContextMenu(control, menu);
      }

      public static void SetupSpecialComponentGroup(ComponentGroupLayout componentGroup)
      {
         componentGroup.FillBrush = new SolidBrush(Color.FromArgb(64, 64, 64));
         componentGroup.ForeColor = Color.White;
         componentGroup.Font = kBoldDefaultFont;
      }
   }
}
