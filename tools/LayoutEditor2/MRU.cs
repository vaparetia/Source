using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml;
using System.Windows.Forms;

namespace LayoutEditorMGS2
{
   public class MRU
   {
      private static string GetMRUPath()
      {
         return System.Environment.ExpandEnvironmentVariables("%APPDATA%\\Bluepoint Games\\MRU");
      }

      private static bool EnsureMRUPathExists()
      {
         String mruPath = GetMRUPath();
         if (Directory.Exists(mruPath))
         {
            return true;
         }
         else
         {
            try
            {
               Directory.CreateDirectory(mruPath);
               return true;
            }
            catch (System.Exception)
            {
               return false;
            }
         }
      }

      private static String MakeMRUFileName(String mruList)
      {
         return GetMRUPath() + "\\" + mruList + ".xml";
      }

      public static List<String> GetMRU(String mruListTag)
      {
         try
         {
            String mruFile = MakeMRUFileName(mruListTag);
            if (File.Exists(mruFile))
            {
               XmlDocument doc = new XmlDocument();

               doc.Load(MakeMRUFileName(mruListTag));

               List<String> ret = new List<string>();
               foreach (XmlElement el in doc.SelectNodes("MRU/Items/Item"))
               {
                  ret.Add(el.InnerText.Trim());
               }

               return ret;
            }
         }
         catch (System.Exception)
         {
         }

         return new List<String>();
      }

      public static void AddMRU(String mruListTag, String mruItem, int maxCount)
      {
         if (EnsureMRUPathExists())
         {
            List<String> mruList = GetMRU(mruListTag);

            string fullPathOfMRUItem = Path.GetFullPath(mruItem.ToLower());

            // Find and remove us if we're lower in the list
            for (int i = mruList.Count - 1; i >= 0; --i)
            {
               if (Path.GetFullPath(mruList[i].ToLower()) == fullPathOfMRUItem)
               {
                  mruList.RemoveAt(i);
               }
            }

            mruList.Insert(0, mruItem);
            if (mruList.Count > maxCount)
            {
               mruList.RemoveRange(maxCount, mruList.Count - maxCount);
            }

            // Create our little XML document
            XmlDocument doc = new XmlDocument();
            XmlElement rootElement = doc.CreateElement("MRU");
            XmlElement itemsElement = doc.CreateElement("Items");
            doc.AppendChild(rootElement);
            rootElement.AppendChild(itemsElement);
            foreach (String s in mruList)
            {
               XmlElement item = doc.CreateElement("Item");
               item.InnerText = s;
               itemsElement.AppendChild(item);
            }

            try
            {
               doc.Save(MakeMRUFileName(mruListTag));
            }
            catch (System.Exception)
            {
            }
         }
      }

      public static void DropDownOpening(String mruListTag, ToolStripMenuItem recentItems, EventHandler clickEventHandler)
      {
         recentItems.DropDownItems.Clear();

         bool anyRecentFiles = false;
         int index = 1;
         foreach (String fileName in MRU.GetMRU(mruListTag))
         {
            if (System.IO.File.Exists(fileName))
            {
               ToolStripMenuItem item = new ToolStripMenuItem();
               item.Text = String.Format("&{0} {1}", index++, fileName);
               item.Tag = fileName;
               item.Click += clickEventHandler;
               recentItems.DropDownItems.Add(item);

               anyRecentFiles = true;
            }
         }

         if (!anyRecentFiles)
         {
            {
               ToolStripMenuItem item = new ToolStripMenuItem("None");
               item.Enabled = false;
               recentItems.DropDownItems.Add(item);
            }
         }         
      }
   }
}
