using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.XPath;
using System.Xml;
using Tools.Property.Build;
using Tools.Property.Misc;
using Tools.Property.Source;
using Tools.AssetSystem;

namespace Tools.Property.UI
{
   public class DataEditor
   {
      /// <summary>
      /// This function will create a new file if the file doesn't exist of given type or edit the existing one
      /// </summary>
      /// <param name="filePath">path of file to edit (or create)</param>
      /// <param name="createType">type to create if file doesn't exist yet</param>
      /// <returns>true if file was modified, false if edit canceled</returns>
      static public bool CreateOrEditDataFile(string filePath, string createType)
      {
         Manager manager = new Manager();

         Tools.Property.Source.FileBasedPropertyObjectEnumerator propertySource = new FileBasedPropertyObjectEnumerator();
         propertySource.AddFiles(FileBasedPropertyObjectEnumerator.FileSearchParams.DefaultFileSearchParams(manager));

         List<XmlElement> objects = new List<XmlElement>();

         bool readOnly = false;

         if (createType != null && !System.IO.File.Exists(filePath))
         {
            XmlDocument doc = new XmlDocument();
            doc.InnerXml = string.Format("<UsingPropertyContainer containerVersion=\"2\" id=\"{0}\" name=\"Data\"/>", createType);

            MergePropertiesV2.Merge_Inplace(doc, propertySource);

            XmlElement objectNav = doc.SelectSingleNode("//PropertyContainer") as XmlElement;
            if (objectNav != null)
               objects.Add(objectNav);
         }
         else
         {
            readOnly = (System.IO.File.GetAttributes(filePath) & System.IO.FileAttributes.ReadOnly) != 0;

            try
            {
               XmlDocument doc = new XmlDocument();
               doc.Load(filePath);

               MergePropertiesV2.Merge_Inplace(doc, propertySource);

               if (doc == null)
               {
                  System.Windows.Forms.MessageBox.Show(string.Format("Format error, can't merge file '{0}'", filePath),
                     "DataEditor Error",
                     System.Windows.Forms.MessageBoxButtons.OK,
                     System.Windows.Forms.MessageBoxIcon.Error);
                  return false;
               }

               XmlElement propertyContainerNode = doc.SelectSingleNode("//PropertyContainer") as XmlElement;
               if (propertyContainerNode != null)
               {
                  objects.Add( propertyContainerNode );
               }
            }
            catch (System.Xml.XmlException e)
            {
               System.Windows.Forms.MessageBox.Show(string.Format("Error while parsing file '{0}':\n{1}", filePath, e.Message), 
                  "DataEditor Error",
                  System.Windows.Forms.MessageBoxButtons.OK, 
                  System.Windows.Forms.MessageBoxIcon.Error);
               return false;
            }
         }

         ModalPropertyDlg propertyDlg = new ModalPropertyDlg(objects, propertySource);
         
         if (readOnly)
            propertyDlg.mOkButton.Enabled = false;

         if (propertyDlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
         {
            foreach (XmlElement nav in objects)
            {
               XmlDocument unmergedDocument = MergePropertiesV2.Unmerge(nav);
               unmergedDocument.Save(filePath);
            }

            return true;
         }
         else
         {
            return false;
         }
      }
   }
}
