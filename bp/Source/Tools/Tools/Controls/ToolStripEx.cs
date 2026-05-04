using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace Tools.Controls
{
   /// <summary>
   /// This class adds on to the functionality (ClickThrough) provided in System.Windows.Forms.ToolStrip.
   /// See http://blogs.msdn.com/rickbrew/archive/2006/01/09/511003.aspx for details.
   /// </summary>
   public class ToolStripEx : ToolStrip
   {
      private bool clickThrough = true;

      /// <summary>
      /// Gets or sets whether the ToolStripEx honors item clicks when its containing form does
      /// not have input focus.
      /// </summary>
      /// <remarks>
      /// Default value is false, which is the same behavior provided by the base ToolStrip class.
      /// </remarks>
      public bool ClickThrough
      {
         get
         {
            return this.clickThrough;
         }

         set
         {
            this.clickThrough = value;
         }
      }

      protected override void WndProc(ref Message m)
      {
         base.WndProc(ref m);

         if (this.clickThrough && m.Msg == Tools.Common.WindowsUserNative.WM_MOUSEACTIVATE && m.Result == (IntPtr)Tools.Common.WindowsUserNative.MA_ACTIVATEANDEAT)
         {
            m.Result = (IntPtr)Tools.Common.WindowsUserNative.MA_ACTIVATE;
         }
      }
   }
}
