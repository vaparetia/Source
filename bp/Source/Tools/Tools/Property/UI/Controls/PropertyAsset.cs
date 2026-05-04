using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Xml;
using System.Windows.Forms;

namespace Tools.Property.UI.Controls
{
   public class PropertyAsset : PropertyControl
   {
      SubControls.Edit mTextEdit;

      public Data.Property PropertyDataNode
      {
         get { return (Data.Property) mDataElement; }
      }

      public PropertyAsset(PropertyPanel panel, Data.Property dataElement)
         : base(panel, dataElement)
      {
         BuildContextMenu = Helper.DefaultPropertyBuildContextMenu;

         mControlHeight = 20;

         AllocateSubControlStorage();

         // Label
         {
            SubControls.Label label = CreatePropertyLinkLabel();
            label.LinkClicked += OnLinkClicked;
            AddSubControl(label);
         }

         // Edit Box
         {
            mTextEdit = CreatePropertyEdit();
            mTextEdit.ValidateValue = OnValidateValue;

            AddSubControl(mTextEdit);
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

      void OnLinkClicked(ControlInfo controlInfo, System.Windows.Forms.MouseButtons buttons)
      {
         if( buttons == System.Windows.Forms.MouseButtons.Left )
         {
            BrowseAsset();
         }
      }

      private void BrowseAsset()
      {
         using (OpenFileDialog dlg = new OpenFileDialog())
         {
            dlg.Title = "Select Asset";

            // if we don't set this our current working directory is not reset
            dlg.RestoreDirectory = true;

            // if there is currently an asset specified, use this as the default path, but only if it's a valid path
            string initialAsset = GetValue();

            // initialize default path for browser
            if (initialAsset != string.Empty)
            {
               if (AssetSystem.Manager.IsRepositoryRelativePath(initialAsset))
               {
                  initialAsset = mPanel.PropertyInterface.mAssetSystem.GetSystemPath(initialAsset);
               }
               else if (mPanel.PropertyInterface.mAssetSystem.IsSystemPathInsideRepository(initialAsset))
               {
                  dlg.FileName = System.IO.Path.GetFileName(initialAsset);
                  initialAsset = System.IO.Path.GetDirectoryName(initialAsset);
               }
            }

            if (initialAsset != string.Empty && System.IO.File.Exists(initialAsset))
            {
               dlg.InitialDirectory = System.IO.Path.GetDirectoryName(initialAsset);
            }
            else
            {
               string cwd = System.IO.Directory.GetCurrentDirectory();

               // if there is no asset path, we just use the current working folder if it's in the repository and the repository root otherwise.
               if (mPanel.PropertyInterface.mAssetSystem.IsSystemPathInsideRepository(cwd))
               {
                  dlg.InitialDirectory = cwd;
               }
               else
               {
                  dlg.InitialDirectory = mPanel.PropertyInterface.mAssetSystem.RepositoryRoot.Replace('/', '\\');
               }
            }

            // set filter based on asset types
            string[] assetTypes = PropertyDataNode.AssetTypes;

            string filter = AssetSystem.AssetTypes.GetFileFilters(assetTypes);

            if (filter.Length > 0)
               filter += "|";

            filter += "All files (*.*)|*.*";
            dlg.Filter = filter;

            if (dlg.ShowDialog() == DialogResult.OK)
            {
               string filePath = dlg.FileName;
               
               // Update asset path, we're going through the text box to get all the validation to happen automatically.
               {
                  List<Rectangle> controlRects;
                  GetSubControlClientRects(out controlRects);
                  ControlInfo controlInfo = new ControlInfo(this, 1);
                  mTextEdit.SetExternalValue(controlInfo, controlRects[controlInfo.SubControlIndex], filePath);
               }
            }
         }
      }

      private bool OnValidateValue(ref string valueText, out string errorMessage)
      {
         // Empty asset paths are always ok.
         if (String.IsNullOrEmpty(valueText))
         {
            errorMessage = string.Empty;
            return true;
         }

         // If provided path isn't either a repository relative path OR a system path that's inside the repository, it's bad.
         if (!String.IsNullOrEmpty(valueText) && !(AssetSystem.Manager.IsRepositoryRelativePath(valueText) || mPanel.PropertyInterface.mAssetSystem.IsSystemPathInsideRepository(valueText)))
         {
            errorMessage = string.Format("'{0}' is not a valid asset path.", valueText);
            return false;
         }

         // Normalize path in the way we want it always stored.
         string assetPath = mPanel.PropertyInterface.mAssetSystem.GetRepositoryRelativePath(valueText);

         // Error out if the file doesn't exist.
         if (!System.IO.File.Exists(mPanel.PropertyInterface.mAssetSystem.GetSystemPath(assetPath)))
         {
            errorMessage = string.Format("The file '{0}' does not exist.", valueText);
            return false;
         }

         string[] validAssetTypes = PropertyDataNode.AssetTypes;

         if (!mPanel.PropertyInterface.mAssetSystem.IsValidRepositoryRelativePath(assetPath, validAssetTypes))
         {
            errorMessage = string.Format("'{0}' exists, but it is not the correct type. It must be of type(s) : {1}", valueText, Tools.Common.Misc.ConcatStringArrayWithSeparator(validAssetTypes, ", "));
            return false;
         }

         errorMessage = string.Empty;
         valueText = assetPath;
         return true;
      }
   }
}
