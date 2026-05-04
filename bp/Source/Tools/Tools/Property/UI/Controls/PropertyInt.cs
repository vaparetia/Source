using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Xml;

namespace Tools.Property.UI.Controls
{
   public class PropertyInt : PropertyControl
   {
      public Data.Property PropertyDataNode { get { return (Data.Property) mDataElement; } }

      public PropertyInt(PropertyPanel panel, Data.Property dataElement)
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

            textEdit.FormatValue = FormatValue;
            textEdit.ValidateValue = OnValidateValue;
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

      public override bool ValidateValue(string textValue, out string errorMessage)
      {
         string textValueTemp = textValue;
         return OnValidateValue(ref textValueTemp, out errorMessage);
      }

      private string FormatValue(string valueText)
      {
         int intValue;
         int.TryParse(valueText, out intValue);

         return intValue.ToString();
      }

      private bool OnValidateValue(ref string valueText, out string errorMessage)
      {
         errorMessage = String.Empty;

         int intValue;
         if (!int.TryParse(valueText, out intValue))
         {
            errorMessage = valueText + " is not a valid value.";
         }

         int minValue = 0;
         if (GetMinValue(ref minValue))
         {
            if (intValue < minValue)
            {
               errorMessage = valueText + " is out of range (min:" + minValue.ToString() + ")";
               valueText = minValue.ToString();
            }
         } 
         
         int maxValue = 0;
         if (GetMaxValue(ref maxValue))
         {
            if (intValue > maxValue)
            {
               errorMessage = valueText + " is out of range (max:" + maxValue.ToString() + " )";
               valueText = maxValue.ToString();
            }
         }

         return string.IsNullOrEmpty(errorMessage);
      }

      public bool GetMinValue(ref int value)
      {
         string min = PropertyDataNode.MinValue;
         if (min != "")
         {
            if (int.TryParse(min, out value))
            {
               return true;
            }
         }

         return false;
      }

      public bool GetMaxValue(ref int value)
      {
         string max = PropertyDataNode.MaxValue;
         if (max != "")
         {
            if (int.TryParse(max, out value))
            {
               return true;
            }
         }

         return false;
      }
   }
}
