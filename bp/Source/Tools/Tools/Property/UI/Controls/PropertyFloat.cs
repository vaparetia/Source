using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Drawing;
using Tools.Property.Misc;

namespace Tools.Property.UI.Controls
{
   public class PropertyFloat : PropertyControl
   {
      public Data.Property PropertyDataNode { get { return (Data.Property) mDataElement; } }

      public PropertyFloat( PropertyPanel panel, Data.Property dataElement )
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
         string tempTextValue = textValue;
         return OnValidateValue(ref tempTextValue, out errorMessage);
      }

      private string FormatValue(string valueText)
      {
         float floatValue;
         float.TryParse(valueText, out floatValue);

         return floatValue.ToString("F4");
      }

      private bool OnValidateValue(ref string valueText, out string errorMessage)
      {
         errorMessage = String.Empty;

         float floatValue;
         if (!float.TryParse(valueText, out floatValue))
         {
            errorMessage = valueText + " is not a valid value.";
         }

         float? min = MinValue;
         float? max = MaxValue;

         if (min.HasValue)
         {
            if (floatValue < min.Value)
            {
               if (string.IsNullOrEmpty(errorMessage))
                  errorMessage = valueText + " is out of range (min:" + min.Value + ")";

               valueText = min.ToString();
            }
         }

         if (max.HasValue)
         {
            if (floatValue > max.Value)
            {
               if (string.IsNullOrEmpty(errorMessage))
                  errorMessage = valueText + " is out of range (max:" + max.Value + " )";

               valueText = max.ToString();
            }
         }

         return string.IsNullOrEmpty(errorMessage);
      }

      public float? MinValue
      {
         get
         {
            string valueText = PropertyDataNode.MinValue;

            if (!String.IsNullOrEmpty(valueText))
            {
               float result;

               if (float.TryParse(valueText, out result))
                  return result;
            }

            return null;
         }
      }

      public float? MaxValue
      {
         get
         {
            string valueText = PropertyDataNode.MaxValue;

            if( !String.IsNullOrEmpty(valueText) )
            {
               float result;
               
               if( float.TryParse(valueText, out result) )
                  return result;
            }

            return null;
         }
      }
   }
}
