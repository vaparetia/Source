using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Drawing;
using Tools.Property.Misc;

namespace Tools.Property.UI.Controls
{
   public class PropertyBool : PropertyControl
   {
      public PropertyBool(PropertyPanel panel, Data.Property dataElement)
         : base(panel, dataElement)
      {
         BuildContextMenu = Helper.DefaultPropertyBuildContextMenu;

         mControlHeight = 20;
         AllocateSubControlStorage();

         // Create label
         AddSubControl(CreatePropertyLabel());

         // Create check box
         {
            SubControls.CheckBox checkBox = new Tools.Property.UI.Controls.SubControls.CheckBox(mPanel);

            checkBox.GetValue = delegate()
            {
               if (IsGrouped)
               {
                  return SubControls.CheckBox.CheckBoxState.Mixed;
               }
               else
               {
                  string valueText = GetValue();
                  if (Tools.Common.Misc.ParseBool(valueText))
                  {
                     return SubControls.CheckBox.CheckBoxState.Checked;
                  }
                  else
                  {
                     return SubControls.CheckBox.CheckBoxState.Unchecked;
                  }
               }
            };

            checkBox.SetValue = delegate(SubControls.CheckBox.CheckBoxState state)
            {
               switch (state)
               {
                  case SubControls.CheckBox.CheckBoxState.Checked:
                     SetValue("true", SetValueMode.ForceUngrouped);
                     break;
                  case SubControls.CheckBox.CheckBoxState.Unchecked:
                     SetValue("false", SetValueMode.ForceUngrouped);
                     break;
               }
            };

            AddSubControl(checkBox);
         }
      
         // Context button
         {
            AddSubControl(Helper.CreateContextMenuButton(this, SubControls.ContextMenuType.Type1));
         }
      }

      public override void GetSubControlClientRects(out List<Rectangle> subControlClientRects)
      {
         subControlClientRects = Helper.GetDefaultPropertyLayoutRects(GetClientRect(), 14);
      }
   }

}
