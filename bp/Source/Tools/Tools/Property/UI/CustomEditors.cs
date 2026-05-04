using System;
using System.Collections.Generic;
using System.Text;
using Tools.Property.Misc;
using System.IO;
using System.Windows.Forms;
using System.Xml.XPath;
using System.Xml;

namespace Tools.Property.UI.CustomEditor
{
   public static class VTexEditor
   {
      public static void GetCustomEditors(Controls.PropertyControl property, PropertyEditorGUI.PropertyEditorCallbacks.GetCustomEditorsEventArgs e)
      {
         Data.Property dataElement = property.DataElement as Data.Property;
         if ( dataElement != null && dataElement.Type == Helpers.kProperty_Type_string)
         {
            if (dataElement.SubType == Helpers.kProperty_SubType_asset)
            {
               string[] assetTypes = dataElement.AssetTypes;

               bool isTexture = false;
               foreach (string type in assetTypes)
               {
                  if (type == "TXTR")
                  {
                     isTexture = true;
                     break;
                  }
               }

               if (isTexture)
               {
                  string assetPath = property.GetValue();

                  if (assetPath != "")
                  {
                     e.CustomEditorList.Add(new PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor("Edit Meta Data...", EditMetaData, "Edit Metadata for asset"));
                  }

                  e.CustomEditorList.Add(new PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor("Create Virtual Texture...", CreateVirtualTexture, "Create new virtual texture"));

                  if (Path.GetExtension(assetPath).ToLower() == ".vtex")
                  {
                     e.CustomEditorList.Add(new PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor("Edit Virtual Texture...", EditVirtualTexture, "Edit existing virtual texture"));
                  }
               }
            }
         }
      }

      public static string EditMetaData(Controls.PropertyControl property)
      {
         EditMetaData(property.GetValue());
         return null;
      }

      public static string CreateVirtualTexture(Controls.PropertyControl property)
      {
         return ProcessVirtualTexture(string.Empty, true);
      }

      public static string EditVirtualTexture(Controls.PropertyControl property)
      {
         return ProcessVirtualTexture(property.GetValue(), false);
      }

      private static void EditMetaData(string assetPath)
      {
         if (assetPath != string.Empty)
         {
            List<string> assets = new List<string>();
            assets.Add(assetPath);

            AssetSystem.Helper.EditMetaData(assets);
         }
      }

      private static string ProcessVirtualTexture(string assetPath, bool forceCreate)
      {
         Tools.AssetSystem.Manager manager = new Tools.AssetSystem.Manager();

         string vtexFileName = assetPath;

         if( string.IsNullOrEmpty(vtexFileName) || forceCreate )
         {
            System.Windows.Forms.SaveFileDialog dlg = new SaveFileDialog();
            dlg.OverwritePrompt = false;
            dlg.AddExtension = true;
            dlg.Title = "Create new virtual texture...";
            dlg.Filter = "Virtual Texture (*.vtex)|*.vtex";

            if (dlg.ShowDialog() == DialogResult.OK)
            {
               vtexFileName = manager.GetRepositoryRelativePath(dlg.FileName);
            }
         }

         if( string.IsNullOrEmpty(vtexFileName) )
         {
            return null;
         }

         string systemPath = manager.GetSystemPath(vtexFileName);

         if (Tools.Property.UI.DataEditor.CreateOrEditDataFile(systemPath, "vtex"))
         {
            return vtexFileName;
         }
         
         return string.Empty;
      }
   }

   /// <summary>
   /// This editor handles copy and paste operations.
   /// It currently only implements copy and paste between evaluators, but there is nothing in the way of extending it to other evaluators as well.
   /// </summary>
   public static class CopyPasteEditor
   {
      private const string kEvaluatorDataType = "EvaluatorData";
      private static string mCopyBufferType;
      private static string mCopyBuffer;

      public static void GetCustomEditors(Controls.PropertyControl sender, PropertyEditorGUI.PropertyEditorCallbacks.GetCustomEditorsEventArgs e)
      {
         string copyToolTip = "";
         string pasteToolTip = "";

         PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor.SpawnEditorDelegate copyFunction = null;
         PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor.SpawnEditorDelegate pasteFunction = null;

         if( sender is Controls.PropertyEvaluator )
         {
            copyToolTip = "Copy evaluator";
            copyFunction = CopyEvaluator;

            if (mCopyBufferType == kEvaluatorDataType && !string.IsNullOrEmpty(mCopyBuffer))
            {
               pasteToolTip = "Paste evaluator";
               pasteFunction = PasteEvaluator;
            }
         }
         else
         {
            Data.Property dataElement = sender.DataElement as Data.Property;
            if( dataElement != null )
            {
               string propertyType = dataElement.Type;
               switch (propertyType)
               {
                  case Helpers.kProperty_Type_bool:
                  case Helpers.kProperty_Type_int:
                  case Helpers.kProperty_Type_float:
                  case Helpers.kProperty_Type_color:
                  case Helpers.kProperty_Type_string:
                  case Helpers.kProperty_Type_enum:
                     {
                        copyToolTip = "Copy value";
                        copyFunction = CopyValue;
                        
                        if( mCopyBufferType == propertyType )
                        {
                           string errorMessage;
                           if( !sender.ValidateValue(mCopyBuffer, out errorMessage) )
                           {
                              pasteToolTip = string.Format("Can't paste value ({0})", errorMessage);
                           }
                           else
                           {
                              string valueText = mCopyBuffer;
                              
                              if( string.IsNullOrEmpty(valueText) )
                                 valueText = "None";

                              pasteToolTip = string.Format("Paste value ({0})", valueText);
                              pasteFunction = PasteValue;
                           }
                        }
                     }
                     break;
               }
            }
         }

         e.CustomEditorList.Add(new PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor("Copy", copyFunction, copyToolTip));
         e.CustomEditorList.Add(new PropertyEditorGUI.PropertyEditorCallbacks.CustomEditor("Paste", pasteFunction, pasteToolTip));
      }

      private static string CopyEvaluator(Controls.PropertyControl control)
      {
         Tools.Property.UI.Controls.PropertyEvaluator evaluatorProperty = control as Tools.Property.UI.Controls.PropertyEvaluator;
         if( evaluatorProperty != null )
         {
            Data.Evaluator evaluatorData = evaluatorProperty.EvaluatorPropertyDataNode.Evaluator;
            mCopyBuffer = evaluatorData.Element.OuterXml;
            mCopyBufferType = kEvaluatorDataType;
         }

         return string.Empty;
      }

      private static string PasteEvaluator(Controls.PropertyControl control)
      {
         Tools.Property.UI.Controls.PropertyEvaluator evaluatorProperty = control as Tools.Property.UI.Controls.PropertyEvaluator;
         if (evaluatorProperty != null)
         {
            evaluatorProperty.PasteData(mCopyBuffer);
         }

         return string.Empty;
      }

      private static string CopyValue(Controls.PropertyControl control)
      {
         Data.Property propertyData = (Data.Property) control.DataElement;
         mCopyBufferType = propertyData.Type;
         mCopyBuffer = propertyData.ValueTextNullable;
         
         return string.Empty;
      }

      private static string PasteValue(Controls.PropertyControl control)
      {
         control.SetValue(mCopyBuffer, Tools.Property.UI.Controls.PropertyControl.SetValueMode.ForceUngrouped);
         control.InvalidateControl();
         return string.Empty;
      }
   }
}
