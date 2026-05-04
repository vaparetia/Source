using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace Tools.Controls
{
   public class Misc
   {
      private const uint ECM_FIRST = 0x1500;
      private const uint EM_SETCUEBANNER = ECM_FIRST + 1;

      public static void SetCueBanner(Control control, string cueBannerText)
      {
         Tools.Common.WindowsUserUnsafe.SendMessage(control.Handle, EM_SETCUEBANNER, IntPtr.Zero, cueBannerText);
      }

   }
}
