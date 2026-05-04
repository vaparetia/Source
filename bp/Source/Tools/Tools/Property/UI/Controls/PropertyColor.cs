using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Xml;
using System.Windows.Forms;
using Tools.Common;
using System.Drawing.Drawing2D;

namespace Tools.Property.UI.Controls
{
   /// <summary>
   /// See: http://en.wikipedia.org/wiki/HSL_color_space#Conversion_from_HSV_to_RGB
   /// </summary>
   public class ColorHelpers
   {
      static public void RgbToHsv(RGBColor input, out HSVColor output)
      {
         output = new HSVColor(0, 0, 0);

         input.R = Math.Min(1.0f, Math.Max(0.0f, input.R));
         input.G = Math.Min(1.0f, Math.Max(0.0f, input.G));
         input.B = Math.Min(1.0f, Math.Max(0.0f, input.B));

         float min = Math.Min(input.R, Math.Min(input.G, input.B));
         float max = Math.Max(input.R, Math.Max(input.G, input.B));
         float range = max - min;
         
         // Compute hue
         if( max == min )
         {
            output.H = 0.0f;
         }
         else
         {
            if( input.R == max )
            {
               output.H = (input.G - input.B) / range;
            }
            else if( input.G == max )
            {
               output.H = 2.0f + (input.B - input.R) / range;
            }
            else
            {
               output.H = 4.0f + (input.R - input.G) / range;
            }

            if( output.H < 0.0f )
            {
               output.H += 6.0f;
            }

            output.H = Math.Max(0.0f, Math.Min(1.0f, output.H / 6.0f));
         }

         // Compute saturation
         if( max == 0.0f )
         {
            output.S = 0.0f;
         }
         else
         {
            output.S = Math.Max(0.0f, Math.Min(1.0f, range / max));
         }

         // Compute value
         output.V = Math.Max(0.0f, Math.Min(1.0f, max));
      }

      static public void HsvToRgb(HSVColor input, out RGBColor output)
      {
         output = new RGBColor(0, 0, 0);

         input.H = Math.Min(1.0f, Math.Max(0.0f, input.H));
         input.S = Math.Min(1.0f, Math.Max(0.0f, input.S));
         input.V = Math.Min(1.0f, Math.Max(0.0f, input.V));

         float hFloor = (float)Math.Floor(input.H * 6.0);
         int section = (int)(hFloor) % 6;
         float fraction = (input.H * 6.0f) - hFloor;

         float p = input.V * (1 - input.S);
         float q = input.V * (1 - fraction * input.S);
         float t = input.V * (1 - (1 - fraction) * input.S);

         switch(section)
         {
            case 0:
               output.R = input.V;
               output.G = t;
               output.B = p;
               break;

            case 1:
               output.R = q;
               output.G = input.V;
               output.B = p;
               break;

            case 2:
               output.R = p;
               output.G = input.V;
               output.B = t;
               break;

            case 3:
               output.R = p;
               output.G = q;
               output.B = input.V;
               break;

            case 4:
               output.R = t;
               output.G = p;
               output.B = input.V;
               break;

            case 5:
               output.R = input.V;
               output.G = p;
               output.B = q;
               break;
            
            default:
               // Should never happen, this is just to shut the compiler up.
               output.R = output.G = output.B = 0;
               break;
         }

         output.R = Math.Min(1.0f, Math.Max(0.0f, output.R));
         output.G = Math.Min(1.0f, Math.Max(0.0f, output.G));
         output.B = Math.Min(1.0f, Math.Max(0.0f, output.B));

      }
   }

   public struct RGBColor
   {
      public float R;
      public float G;
      public float B;

      public RGBColor(float r, float g, float b)
      {
         R = r;
         G = g;
         B = b;
      }
      
      public Color AsColor()
      {
         return Color.FromArgb((int)(R * 255.0f), (int)(G * 255.0f), (int)(B * 255.0f));
      }
   }

   public struct HSVColor
   {
      public float H;
      public float S;
      public float V;

      public HSVColor(float h, float s, float v)
      {
         H = h;
         S = s;
         V = v;
      }
   }

   public class PropertyColorEditor : PropertyControl
   {
      public delegate RGBColor GetRGBDelegate();
      public delegate HSVColor GetHSVDelegate();

      public class SaturationValueBox : SubControls.SubControl
      {
         bool mIsPressed;

         public delegate void SetSVDelegate(float s, float v, PropertyControl.SetValueMode additionalSetValueModeFlags);

         public GetHSVDelegate GetHSV;
         public SetSVDelegate SetSV;

         public SaturationValueBox(PropertyPanel panel)
            : base(panel)
         {
         }

         public override void OnDraw(ControlContext context, Rectangle controlRectangle)
         {
            HSVColor hsv = GetHSV();

            HSVColor hueOnly = hsv;
            hueOnly.S = 1;
            hueOnly.V = 1;

            RGBColor rgb;
            ColorHelpers.HsvToRgb(hueOnly, out rgb);

            Color gdiColor = ColorF.FromArgbi(1.0f, rgb.R, rgb.G, rgb.B, 1.0f).ToColor();

            // Fill background with color going from white to current color (left to right)
            using (LinearGradientBrush saturationBrush = new LinearGradientBrush(controlRectangle, Color.White, gdiColor, 0.0f))
            {
               context.Graphics.FillRectangle(saturationBrush, controlRectangle);
            }

            // Overlay from transparent black to black (top to bottom)
            using (LinearGradientBrush brightnessBrush = new LinearGradientBrush(controlRectangle, Color.FromArgb(0, Color.Black), Color.FromArgb(255, Color.Black), 90.0f))
            {
               context.Graphics.FillRectangle(brightnessBrush, controlRectangle);
            }

            // Draw cursor
            {
               Point cursorPos = new Point(controlRectangle.Left + (int)(controlRectangle.Width * hsv.S), controlRectangle.Top + (int)(controlRectangle.Height * (1.0f - hsv.V)));

               Image image = Properties.Resources.ColorEditor_ColorBoxCursor;

               System.Drawing.Imaging.ImageAttributes attr = new System.Drawing.Imaging.ImageAttributes();
               attr.SetColorKey(Color.Magenta, Color.Magenta);

               Rectangle destRect = new Rectangle(cursorPos, image.Size);
               destRect.Offset(-image.Width / 2, -image.Height / 2);

               RectangleF originalClip = context.Graphics.ClipBounds;
               
               context.Graphics.SetClip(controlRectangle);
               
               context.Graphics.DrawImage(image, destRect, 0, 0, image.Width, image.Height, GraphicsUnit.Pixel, attr);
               
               context.Graphics.SetClip(originalClip);
            }

            // Draw border
            context.Graphics.DrawRectangle(new Pen(Color.Black), controlRectangle);
         }

         public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
         {
            base.OnMouseDown(e, controlInfo, controlRectangle);
            
            if( e.Button == MouseButtons.Left)
            {
               mIsPressed = true;
               mPanel.SetCaptureMouse(controlInfo, true);
               SetCursor(e, controlRectangle, SetValueMode.IsPreview);
            }
         }

         public override void OnMouseMove(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
         {
            base.OnMouseMove(e, controlInfo, controlRectangle);
            if( mIsPressed )
            {
               SetCursor(e, controlRectangle, SetValueMode.IsPreview);
            }
         }

         public override void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
         {
            base.OnMouseUp(e, controlInfo, controlRectangle);
            
            if( e.Button == MouseButtons.Left)
            {
               if( mIsPressed )
               {
                  SetCursor(e, controlRectangle, SetValueMode.ForceChangeNotification);
               }

               mPanel.SetCaptureMouse(controlInfo, false);
               mIsPressed = false;
            }
         }

         void SetCursor(MouseEventArgs e, Rectangle controlRectangle, PropertyControl.SetValueMode additionalSetValueModeFlags)
         {
            Point pos = e.Location;

            float s = Math.Min(1.0f, Math.Max(0.0f, (e.X - controlRectangle.Left) / (float)controlRectangle.Width));
            float v = Math.Min(1.0f, Math.Max(0.0f, 1.0f - (e.Y - controlRectangle.Top) / (float)controlRectangle.Height));
            SetSV(s, v, additionalSetValueModeFlags);
         }
      }

      public class HueSlider : SubControls.SubControl
      {
         public delegate void SetHDelegate(float h, PropertyControl.SetValueMode additionalSetValueModeFlags);
         public SetHDelegate SetH;

         public GetHSVDelegate GetHSV;

         bool mIsPressed;

         public HueSlider(PropertyPanel panel)
            : base(panel)
         {
         }

         public override void OnDraw(ControlContext context, Rectangle controlRectangle)
         {
            context.Graphics.DrawImage(Properties.Resources.ColorEditor_HueGradient, controlRectangle);

            HSVColor hsv = GetHSV();

            int pos = controlRectangle.Bottom - (int)(hsv.H * controlRectangle.Height + 0.5f);
            context.Graphics.DrawLine(new Pen(Color.Black), controlRectangle.Left, pos, controlRectangle.Right, pos);

            context.Graphics.DrawRectangle(new Pen(Color.Black), controlRectangle);
         }

         public override void OnMouseDown(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
         {
            base.OnMouseDown(e, controlInfo, controlRectangle);

            if (e.Button == MouseButtons.Left)
            {
               mIsPressed = true;
               mPanel.SetCaptureMouse(controlInfo, true);
               SetCursor(e, controlRectangle, SetValueMode.IsPreview);
            }
         }

         public override void OnMouseMove(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
         {
            base.OnMouseMove(e, controlInfo, controlRectangle);
            if (mIsPressed)
            {
               SetCursor(e, controlRectangle, SetValueMode.IsPreview);
            }
         }

         public override void OnMouseUp(MouseEventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
         {
            base.OnMouseUp(e, controlInfo, controlRectangle);

            if (e.Button == MouseButtons.Left)
            {
               if( mIsPressed )
               {
                  SetCursor(e, controlRectangle, SetValueMode.ForceChangeNotification);
               }

               mPanel.SetCaptureMouse(controlInfo, false);
               mIsPressed = false;
            }
         }

         void SetCursor(MouseEventArgs e, Rectangle controlRectangle, PropertyControl.SetValueMode additionalSetValueModeFlags)
         {
            Point pos = e.Location;

            float h = Math.Min(1.0f, Math.Max(0.0f, (controlRectangle.Bottom - e.Y) / (float)controlRectangle.Height));
            SetH(h, additionalSetValueModeFlags);
         }
      }

      PropertyControl mParentControl;

      HSVColor? mHSV;

      public HSVColor GetHSV()
      {
         if( mHSV.HasValue )
         {
            return mHSV.Value;
         }
         else
         {
            ColorF storedColor = GetStoredColor();
            
            HSVColor output;
            ColorHelpers.RgbToHsv(new RGBColor(storedColor.R, storedColor.G, storedColor.B), out output);
            
            return output;
         }
      }

      public void SetHSV(HSVColor value, SetValueMode additionalSetValueModeFlags)
      {
         mHSV = value;
         ColorF storedColor = GetStoredColor();
         SetValueMode setValueMode = SetValueMode.ForceUngrouped | additionalSetValueModeFlags;
         SetStoredColor(GetRGB(), storedColor.A, storedColor.I, setValueMode);
      }

      public RGBColor GetRGB()
      {
         if( mHSV.HasValue )
         {
            RGBColor color;
            ColorHelpers.HsvToRgb(GetHSV(), out color);
            return color;
         }
         else
         {
            ColorF storedColor = GetStoredColor();
            return new RGBColor(storedColor.R, storedColor.G, storedColor.B);
         }
      }

      public void SetRGB(RGBColor value, SetValueMode additionalSetValueModeFlags)
      {
         mHSV = null;

         ColorF storedColor = GetStoredColor();

         SetValueMode setValueMode = SetValueMode.ForceUngrouped;
         setValueMode |= additionalSetValueModeFlags;

         SetStoredColor(value, storedColor.A, storedColor.I, setValueMode);
      }

      public PropertyColorEditor(PropertyControl parentControl, PropertyPanel panel, Data.DataNode dataElement)
         : base(panel, dataElement)
      {
         mParentControl = parentControl;

         Margin = new Padding(0);
         mControlHeight = 128 + 6;
         AllocateSubControlStorage();
         
         // color select box
         {
            SaturationValueBox colorBox = new SaturationValueBox(panel);
            colorBox.GetHSV = GetHSV;

            colorBox.SetSV = delegate(float s, float v, PropertyControl.SetValueMode additionalSetValueModeFlags)
            {
               HSVColor hsv = GetHSV();
               hsv.S = s;
               hsv.V = v;
               SetHSV(hsv, additionalSetValueModeFlags);

               InvalidateControl();
               mParentControl.InvalidateControl();
            };

            AddSubControl(colorBox);
         }

         // hue slider
         {
            HueSlider slider = new HueSlider(panel);
            slider.GetHSV = GetHSV;

            slider.SetH = delegate(float h, PropertyControl.SetValueMode additionalSetValueModeFlags)
            {
               HSVColor hsv = GetHSV();
               hsv.H = h;
               SetHSV(hsv, additionalSetValueModeFlags);

               InvalidateControl();
               mParentControl.InvalidateControl();
            };

            AddSubControl(slider);
         }

         // R label
         {
            SubControls.Text label = new Controls.SubControls.Text(panel);
            label.DisplayValue = "R";
            AddSubControl(label);
         }

         // R slider
         {
            SubControls.NumericSlider slider = new SubControls.NumericSlider(panel, 0, 255);
            slider.GetDisplayValue = delegate()
            {
               return ((int)(GetRGB().R * 255.0f + 0.5f)).ToString();
            };

            slider.GetValue = delegate()
            {
               return (int)(GetRGB().R * 255.0f + 0.5f);
            };

            slider.SetValue = delegate(int value, SetValueMode additionalSetValueModeFlags)
            {
               RGBColor color = GetRGB();
               color.R = (value / 255.0f);
               SetRGB(color, additionalSetValueModeFlags);
               InvalidateControl();
               mParentControl.InvalidateControl();
            };
            
            slider.CustomDrawBackground = delegate(ControlContext context, Rectangle controlRectangle)
            {
               RGBColor color1 = GetRGB();
               RGBColor color2 = color1;
               color1.R = 0;
               color2.R = 1.0f;

               DrawSliderBackground(context, ref controlRectangle, ref color1, ref color2);
            };

            AddSubControl(slider);
         }

         // G label
         {
            SubControls.Text label = new Controls.SubControls.Text(panel);
            label.DisplayValue = "G";
            AddSubControl(label);
         }

         // G slider
         {
            SubControls.NumericSlider slider = new SubControls.NumericSlider(panel, 0, 255);
            slider.GetDisplayValue = delegate()
            {
               return ((int)(GetRGB().G * 255.0f + 0.5f)).ToString();
            };

            slider.GetValue = delegate()
            {
               return (int)(GetRGB().G * 255.0f + 0.5f);
            };

            slider.SetValue = delegate(int value, SetValueMode additionalSetValueModeFlags)
            {
               RGBColor color = GetRGB();
               color.G = (value / 255.0f);
               SetRGB(color, additionalSetValueModeFlags);
               InvalidateControl();
               mParentControl.InvalidateControl();
            };

            slider.CustomDrawBackground = delegate(ControlContext context, Rectangle controlRectangle)
            {
               RGBColor color1 = GetRGB();
               RGBColor color2 = color1;
               color1.G = 0;
               color2.G = 1.0f;

               DrawSliderBackground(context, ref controlRectangle, ref color1, ref color2);
            };

            AddSubControl(slider);
         }

         // B label
         {
            SubControls.Text label = new Controls.SubControls.Text(panel);
            label.DisplayValue = "B";
            AddSubControl(label);
         }
         
         // B slider
         {
            SubControls.NumericSlider slider = new SubControls.NumericSlider(panel, 0, 255);
            slider.GetDisplayValue = delegate()
            {
               return ((int)(GetRGB().B * 255.0f + 0.5f)).ToString();
            };

            slider.GetValue = delegate()
            {
               return (int)(GetRGB().B * 255.0f + 0.5f);
            };

            slider.SetValue = delegate(int value, SetValueMode additionalSetValueModeFlags)
            {
               RGBColor color = GetRGB();
               color.B = (value / 255.0f);
               SetRGB(color, additionalSetValueModeFlags);
               InvalidateControl();
               mParentControl.InvalidateControl();
            };

            slider.CustomDrawBackground = delegate(ControlContext context, Rectangle controlRectangle)
            {
               RGBColor color1 = GetRGB();
               RGBColor color2 = color1;
               color1.B = 0;
               color2.B = 1.0f;

               DrawSliderBackground(context, ref controlRectangle, ref color1, ref color2);
            };

            AddSubControl(slider);
         }

         // A label
         {
            SubControls.Text label = new Controls.SubControls.Text(panel);
            label.DisplayValue = "A";
            AddSubControl(label);
         }

         // A slider
         {
            SubControls.NumericSlider slider = new SubControls.NumericSlider(panel, 0, 255);
            slider.GetDisplayValue = delegate()
            {
               ColorF storedColor = GetStoredColor();
               return ((int)(storedColor.A * 255.0f + 0.5f)).ToString(); 
            };

            slider.GetValue = delegate()
            {
               ColorF storedColor = GetStoredColor();
               return (int)(storedColor.A * 255.0f + 0.5f);
            };

            slider.SetValue = delegate(int value, SetValueMode additionalSetValueModeFlags)
            {
               ColorF storedColor = GetStoredColor();
               
               SetValueMode setValueMode = SetValueMode.ForceUngrouped;
               setValueMode |= additionalSetValueModeFlags;
               
               SetStoredColor(GetRGB(), value / 255.0f, storedColor.I, setValueMode);
               InvalidateControl();
               mParentControl.InvalidateControl();
            };

            AddSubControl(slider);
         }

         // Scale label
         {
            SubControls.Text label = new Controls.SubControls.Text(panel);
            label.DisplayValue = "Scale";
            AddSubControl(label);
         }

         // Scale field
         {
            SubControls.Edit edit = CreatePropertyEdit();

            edit.GetValue = delegate()
            {
               ColorF storedColor = GetStoredColor();
               return storedColor.I.ToString("F4");
            };

            edit.SetValue = delegate(string value)
            {
               ColorF storedColor = GetStoredColor();

               float scale = storedColor.I;
               if (float.TryParse(value, out scale))
               {
                  SetStoredColor(new RGBColor(storedColor.R, storedColor.G, storedColor.B), storedColor.A, scale, SetValueMode.ForceUngrouped);
                  InvalidateControl();
                  mParentControl.InvalidateControl();
               }
            };

            edit.ValidateValue = delegate(ref string value, out string errorMessage)
            {
               float scale = 1.0f;

               if( !float.TryParse(value, out scale) )
               {
                  errorMessage = string.Format("'{0}' is not a valid value", value);
                  return false;
               }

               if( scale < 1.0f )
               {
                  errorMessage = string.Format("'{0}' is less than the minimum (1.0)", value);
                  value = (1.0f).ToString("F4");
                  return false;
               }

               errorMessage = string.Empty;
               value = scale.ToString("F4");
               return true;
            };

            AddSubControl(edit);
         }
      
      }

      private static void DrawSliderBackground(ControlContext context, ref Rectangle controlRectangle, ref RGBColor color1, ref RGBColor color2)
      {
         controlRectangle.Y = controlRectangle.Bottom - 3;
         controlRectangle.Height = 3;
         context.Graphics.FillRectangle(new LinearGradientBrush(new Point(controlRectangle.Left, 0), new Point(controlRectangle.Right, 0), color1.AsColor(), color2.AsColor()), controlRectangle);
      }

      public override void GetSubControlClientRects(out List<Rectangle> subControlClientRects)
      {
         subControlClientRects = new List<Rectangle>();

         Rectangle layoutRect = GetClientRect();

         Rectangle colorBoxRect = new Rectangle();
         {
            colorBoxRect.Location = layoutRect.Location;
            colorBoxRect.Width = 128;
            colorBoxRect.Height = 128;
            
            subControlClientRects.Add(colorBoxRect);
         }

         Rectangle hueSliderRect = new Rectangle();
         {
            hueSliderRect.X = colorBoxRect.Right + 3;
            hueSliderRect.Y = colorBoxRect.Top;
            hueSliderRect.Width = 16;
            hueSliderRect.Height = 128;

            subControlClientRects.Add(hueSliderRect);
         }

         // R label
         Rectangle labelRect = new Rectangle();
         {
            labelRect.X = hueSliderRect.Right + 3;
            labelRect.Y = colorBoxRect.Top;
            labelRect.Width = 32;
            labelRect.Height = 20;
         }
         subControlClientRects.Add(labelRect);
         labelRect.Y += labelRect.Height + 3;

         // R slider
         Rectangle rgbSliderRect = new Rectangle();
         {
            rgbSliderRect.X = labelRect.Right + 3;
            rgbSliderRect.Y = colorBoxRect.Top;
            rgbSliderRect.Width = layoutRect.Right - rgbSliderRect.Left;
            rgbSliderRect.Height = 20;
         }
         subControlClientRects.Add(rgbSliderRect);
         rgbSliderRect.Y += rgbSliderRect.Height + 3;

         // G label
         subControlClientRects.Add(labelRect);
         labelRect.Y += labelRect.Height + 3;

         // G slider
         subControlClientRects.Add(rgbSliderRect);
         rgbSliderRect.Y += rgbSliderRect.Height + 3;

         // B label
         subControlClientRects.Add(labelRect);
         labelRect.Y += labelRect.Height + 3;

         // B slider
         subControlClientRects.Add(rgbSliderRect);
         rgbSliderRect.Y += rgbSliderRect.Height + 3;

         // A label
         subControlClientRects.Add(labelRect);
         labelRect.Y += labelRect.Height + 3;

         // A slider
         subControlClientRects.Add(rgbSliderRect);
         rgbSliderRect.Y += rgbSliderRect.Height + 3;

         // Scale label
         subControlClientRects.Add(labelRect);

         // Scale field
         rgbSliderRect.Height = 20;
         subControlClientRects.Add(rgbSliderRect);
      }

      private ColorF GetStoredColor()
      {
         return ColorF.FromString(GetValue());
      }

      private void SetStoredColor(RGBColor color, float alpha, float intensity, SetValueMode mode)
      {
         string textValue = string.Format("{0} {1} {2} {3} {4}", color.R, color.G, color.B, alpha, intensity);
         SetValue(textValue, mode);
      }
   }

   public class PropertyColor : PropertyControl, PropertyPanel.IMouseHookClient
   {
      PropertyPanel.MouseHook hook;
      static Form mPopupEditor;

      public delegate ColorF GetColorDelegate();
      public class ColorButton : SubControls.Button
      {
         public GetColorDelegate GetColor;
         
         public ColorButton(PropertyPanel panel)
            : base(panel)
         {
         }

         public override void OnDraw(ControlContext context, Rectangle controlRectangle)
         {
            Rectangle buttonRect = new Rectangle(controlRectangle.X, controlRectangle.Y, controlRectangle.Width - 1, controlRectangle.Height - 1);

            Image image = Properties.Resources.ColorButton_Background;
            Size imageSize = image.Size;

            using (TextureBrush backgroundBrush = new TextureBrush(image, System.Drawing.Drawing2D.WrapMode.Tile))
            {
               backgroundBrush.TranslateTransform(buttonRect.Left, buttonRect.Top);
               context.Graphics.FillRectangle(backgroundBrush, buttonRect);
            }

            ColorF color = GetColor();

            // Draw color swatches
            {
               Rectangle solidSwatchRect = new Rectangle(buttonRect.X, buttonRect.Y, buttonRect.Width / 2, buttonRect.Height);
               Rectangle solidScaledSwatchRect = new Rectangle(solidSwatchRect.Right, buttonRect.Y, (buttonRect.Right - solidSwatchRect.Right) / 2, buttonRect.Height);
               Rectangle transparentSwatchRect = new Rectangle(solidScaledSwatchRect.Right, buttonRect.Y, buttonRect.Right - solidScaledSwatchRect.Right, buttonRect.Height);
            
               // Draw solid color swatch
               {
                  ColorF plainColor = color.Clone();
                  plainColor.A = 1.0f;
                  plainColor.I = 1.0f;
                  context.Graphics.FillRectangle(new SolidBrush(plainColor.ToColor()), solidSwatchRect);
               }

               // Draw solid color swatch (taking scale into account)
               {
                  ColorF plainColor = color.Clone();
                  plainColor.A = 1.0f;
                  context.Graphics.FillRectangle(new SolidBrush(plainColor.ToColor()), solidScaledSwatchRect);
               }

               // Draw color swatch taking alpha into account
               {
                  context.Graphics.FillRectangle(new SolidBrush(color.ToColor()), transparentSwatchRect);
               }
            }

            using (Pen pen = (mIsMouseOverControl || mHasFocus) ? new Pen(Color.Orange, 2.0f) : new Pen(Color.Black))
            {
               pen.Alignment = System.Drawing.Drawing2D.PenAlignment.Inset;
               context.Graphics.DrawRectangle(pen, buttonRect);
            }
         }
      }

      public PropertyColor(PropertyPanel panel, Data.DataNode dataElement)
         : base(panel, dataElement)
      {
         BuildContextMenu = Helper.DefaultPropertyBuildContextMenu;

         mControlHeight = 22;

         AllocateSubControlStorage();

         // Label
         AddSubControl(CreatePropertyLabel());

         // Edit Box
         {
            ColorButton colorButton = new ColorButton(panel);
            
            colorButton.GetColor = delegate()
            {
               return ColorF.FromString(GetValue());
            };
            colorButton.Clicked += new Tools.Property.UI.Controls.SubControls.Button.ClickedDelegate(OnEditColor);

            AddSubControl(colorButton);
         }

         // Context button
         {
            AddSubControl(Helper.CreateContextMenuButton(this, SubControls.ContextMenuType.Type1));
         }
      }

      void OnEditColor(EventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         TeardownEditor();

         mPopupEditor = new Form();
         mPopupEditor.FormBorderStyle = FormBorderStyle.FixedSingle;
         mPopupEditor.AutoSize = false;
         mPopupEditor.ShowInTaskbar = false;
         mPopupEditor.ControlBox = false;
         mPopupEditor.Padding = new Padding(3);
         mPopupEditor.Size = new Size(Math.Max(320, controlRectangle.Width), 128 + 8);
         mPopupEditor.Owner = Panel.FindForm();

         mPopupEditor.KeyPreview = true;
         mPopupEditor.KeyDown += delegate(object delegateObject, KeyEventArgs delegateArgs)
         {
            if( delegateArgs.KeyCode == Keys.Escape )
            {
               delegateArgs.Handled = true;
               TeardownEditor();
            }
         };

         mPopupEditor.StartPosition = FormStartPosition.Manual;

         Point editorLocation = new Point(controlRectangle.Right - mPopupEditor.Width, controlRectangle.Y + controlRectangle.Height + 1);
         editorLocation = mPanel.PointToScreen(editorLocation);
         mPopupEditor.Location = editorLocation;

         mPopupEditor.Bounds = Tools.Common.WindowPosition.OffsetBoundsToBestScreen(mPopupEditor.Bounds);

         PropertyPanel popupPropertyPanel = new PropertyPanel();
         popupPropertyPanel.Dock = DockStyle.Fill;
         popupPropertyPanel.PropertyInterface = mPanel.PropertyInterface;

         if( hook == null )
         {
            hook = new PropertyPanel.MouseHook(mPopupEditor, this, popupPropertyPanel);
            hook.HookMouseDown = true;
         }

         PropertyControl colorEditor = new Controls.PropertyColorEditor(this, popupPropertyPanel, mDataElement);
         popupPropertyPanel.SetRootControl(colorEditor);

         mPopupEditor.Controls.Add(popupPropertyPanel);

         mPopupEditor.Show();
      }

      public override void OnLeave(ControlInfo controlInfo)
      {
         base.OnLeave(controlInfo);
         TeardownEditor();
      }

      public override void GetSubControlClientRects(out List<Rectangle> subControlClientRects)
      {
         Rectangle clientRect = GetClientRect();
         subControlClientRects = Helper.GetDefaultPropertyLayoutRects(clientRect);
      }

      public bool OnClickHooked()
      {
         // If user clicks outside the control, we close it
         TeardownEditor();

         // And allow the message to be processed.
         return false;
      }

      private void TeardownEditor()
      {
         if (mPopupEditor != null)
         {
            mPopupEditor.Dispose();
            mPopupEditor = null;
         }

         if (hook != null)
         {
            hook.Dispose();
            hook = null;
         }
      }
   }
}
