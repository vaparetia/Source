using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Xml;
using Tools.Property.Misc;

namespace Tools.Property.UI.Controls
{
   public class PropertyEnum : PropertyControl
   {
      public Data.Property PropertyDataNode { get { return (Data.Property) mDataElement; } }

      public PropertyEnum(PropertyPanel panel, Data.Property dataElement)
         : base(panel, dataElement)
      {
         BuildContextMenu = Helper.DefaultPropertyBuildContextMenu;

         mControlHeight = 20;

         AllocateSubControlStorage();

         // Label
         AddSubControl(CreatePropertyLabel());

         // Combo Box
         {
            SubControls.ComboBox comboBox = new Tools.Property.UI.Controls.SubControls.ComboBox(mPanel);
            comboBox.GetItems = GetItems;
            comboBox.GetItemTooltips = GetItemTooltips;
            comboBox.GetValue = GetComboBoxValue;
            comboBox.SetValue = SetComboBoxValue;

            AddSubControl(comboBox);
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
         string description;
         if( !GetDescriptionValue(PropertyDataNode, textValue, out description) )
         {
            errorMessage = String.Format("'{0}' is not a valid value.", textValue);
            return false;
         }

         errorMessage = string.Empty;
         return true;
      }

      private void GetItems(out List<string> items)
      {
         GetDescriptions(PropertyDataNode, out items);
      }

      private void GetItemTooltips(out List<string> toolTips)
      {
         toolTips = new List<string>();

         foreach ( Data.EnumEntry enumEntry in PropertyDataNode.EnumEntries )
         {
            toolTips.Add(enumEntry.TooltipText);
         }
      }

      private string GetComboBoxValue()
      {
         string enumValue = GetValue();
         string description;
         if( GetDescriptionValue(PropertyDataNode, enumValue, out description ) )
         {
            return description;
         }
         else
         {
            return String.Empty;
         }
      }

      private void SetComboBoxValue(string description)
      {
         string enumValue;
         if (GetNameValue(PropertyDataNode, description, out enumValue))
         {
            SetValue(enumValue, SetValueMode.ForceUngrouped);
            InvalidateControl();
         }
      }

      static public void GetDescriptions(Data.Property dataElement, out List<string> descriptions)
      {
         descriptions = new List<string>();

         foreach ( Data.EnumEntry enumEntry in dataElement.EnumEntries )
         {
            string description = enumEntry.Description;
            if (description != "")
            {
               descriptions.Add(description);
            }
            else
            {
               descriptions.Add(enumEntry.Name);
            }
         }
      }

      static public bool GetDescriptionValue(Data.Property dataElement, string name, out string value)
      {
         Data.EnumEntry found = dataElement.FindEnumEntryByName( name );
         if ( found != null )
         {
            value = found.Description;
            if ( value == "" )
            {
               value = found.Name;
            }

            return true;
         }
         else
         {
            value = "";
            return false;
         }
      }

      static public bool GetNameValue(Data.Property dataElement, string description, out string value)
      {
         Data.EnumEntry found = dataElement.FindEnumEntryByDesc( description );
         if ( found == null )
         {
            // Try using the name instead as we may not have descriptions (in this case the description is the name

            found = dataElement.FindEnumEntryByName( description );
         }

         if ( found != null )
         {
            value = found.Name;
            return true;
         }
         else
         {
            value = "";
            return false;
         }
      }
   }
}
