using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace Tools.Property.UI.Help
{
   public partial class HelpPanel : Form
   {
      public HelpPanel()
      {
         InitializeComponent();
      }

      public string Title
      {
         set
         {
            mTitleBar.Text = value;
         }
      }

      string mHelpText;
      public string HelpText
      {
         set
         {
            mHelpText = value;

            Size textSize = TextRenderer.MeasureText(mHelpText, mHelpTextControl.Font, mHelpTextControl.ClientSize, TextFormatFlags.WordBreak);
            int sizeDifference = textSize.Height - mHelpTextControl.Height;
            this.Height += sizeDifference;
            mHelpTextControl.Invalidate();
         }
      }

      private void OnPaint(object sender, PaintEventArgs e)
      {
         TextRenderer.DrawText(e.Graphics, mHelpText, mHelpTextControl.Font, mHelpTextControl.ClientRectangle, mHelpTextControl.ForeColor, TextFormatFlags.WordBreak|TextFormatFlags.Top);
      }

      protected override void WndProc(ref Message m)
      {
         if (m.Msg == Tools.Common.WindowsUserNative.WM_MOUSEACTIVATE)
         {
            m.Result = (IntPtr)Tools.Common.WindowsUserNative.MA_NOACTIVATE;
         }
         else
         {
            base.WndProc(ref m);
         }
      }

      protected override bool ShowWithoutActivation
      {
         get
         {
            return true;
         }
      }
   }
}