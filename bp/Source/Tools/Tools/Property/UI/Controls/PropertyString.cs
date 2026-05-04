using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Xml;

namespace Tools.Property.UI.Controls
{
   public class PropertyString : PropertyControl
   {
      public PropertyString(PropertyPanel panel, Data.Property dataElement)
         : base(panel, dataElement)
      {
         BuildContextMenu = Helper.DefaultPropertyBuildContextMenu;

         mControlHeight = 20;

         AllocateSubControlStorage();

         // Label
         AddSubControl(CreatePropertyLabel());

         // Edit Box
         {
            SubControls.Edit textEdit = CreatePropertyEdit();
            AddSubControl(textEdit);
         }

         // Context button
         {
            AddSubControl(Helper.CreateContextMenuButton(this, SubControls.ContextMenuType.Type1));
         }
      }

      public override void GetSubControlClientRects(out List<Rectangle> subControlClientRects)
      {
         subControlClientRects = Helper.GetDefaultPropertyLayoutRects(GetClientRect());
      }
   }

}
