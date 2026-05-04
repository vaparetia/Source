using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using TextToolData;
using System.IO;

namespace TextTool
{
   public partial class ImportDialog : Form
   {
      StringDB mStringDB;

      public ImportDialog(StringDB stringDB)
      {
         mStringDB = stringDB;

         InitializeComponent();
      }

      bool ValidateString(string originalString, string inputString, string platform, ref string statusString, ref string problemString)
      {
         if( inputString != null )
         {
            string originalTemp = Misc.RemoveExtraNewlines(mStringDB.ApplyKeywordReplacement(originalString));
            string temp = Misc.RemoveExtraNewlines(mStringDB.ApplyKeywordReplacement(inputString));
            
            if( originalTemp == temp )
            {
               statusString = "WARNING";
               problemString = string.Format("NO CHANGE AFTER KEYWORD REPLACEMENT ({0})", platform);
               return false;
            }

            if( temp.Contains("R1") || 
                temp.Contains("R2") || 
                temp.Contains("R3") || 
                temp.Contains("L1") || 
                temp.Contains("L2") || 
                temp.Contains("L3") || 
                temp.Contains("RB") || 
                temp.Contains("RT") || 
                temp.Contains("RS") || 
                temp.Contains("LB") || 
                temp.Contains("LT") || 
                temp.Contains("LS") )
            {
               statusString = "PROBLEM";
               problemString = "INVALID L*/R* REFERENCE?";
               return false;
            }
            if( temp.Contains('[') )
            {
               statusString = "PROBLEM";
               problemString = "UNKNOWN KEYWORD";
               return false;
            }
            if( temp.Contains("stick button") )
            {
               statusString = "PROBLEM";
               problemString = "BUTTON REFERENCE?";
               return false;
            }
         }

         return true;
      }

      XmlElement EnsureXmlNodeExists(XmlDocument doc, XmlElement root, string elementName)
      {
         XmlElement element = (XmlElement)root.SelectSingleNode("./" + elementName);

         if (element == null)
            element = (XmlElement)root.AppendChild(doc.CreateElement(elementName));

         return element;
      }

      void UpdateXmlNode(XmlDocument doc, XmlElement root, string elementName, string elementValue)
      {
         EnsureXmlNodeExists(doc, root, elementName).InnerText = elementValue;
      }

      bool StringMatch(string lhs, string rhs)
      {
         if (lhs == rhs)
            return true;

         if( lhs != null && rhs != null )
         {
            string lhsTemp = Misc.RemoveExtraNewlines(lhs);
            string rhsTemp = Misc.RemoveExtraNewlines(rhs);
            
            return lhsTemp == rhsTemp;
         }

         return false;
      }

      bool FinalResultStringMatch(string inLhs, string inRhs)
      {
         string lhs = Misc.RemoveExtraNewlines(mStringDB.ApplyKeywordReplacement(inLhs));
         string rhs = Misc.RemoveExtraNewlines(mStringDB.ApplyKeywordReplacement(inRhs));

         return (lhs == rhs);
      }

      public void DoImport(string inputFile)
      {
         if (!TextTool.Utils.ConfirmFileAccess(inputFile, Utils.CheckFileAccessAction.kAskCheckOut))
            return;

         mStringDB.RemoveAllKJPOverrides();

         Cursor.Current = Cursors.WaitCursor;

         XmlDocument doc = new XmlDocument();
         doc.Load(inputFile);

         foreach(XmlElement importEntry in doc.SelectNodes("//Entry"))
         {
            XmlElement originalTextEntry = (XmlElement)importEntry.SelectSingleNode("./OriginalText");
            if( originalTextEntry == null )
               continue;

            EnsureXmlNodeExists(doc, importEntry, "Notes");

            XmlElement bpActionElement = EnsureXmlNodeExists(doc, importEntry, "BPAction");
            string bpAction = bpActionElement.InnerText.Replace("\n", "").Replace("\r", "");
            bool ignoreProblem = (bpAction.ToLower() == "apply");

            string originalText = originalTextEntry.InnerText.Replace("\r", "");
            string newPS3String = "";
            string newX360String = "";

            int originalDiffCount = 0;

            XmlElement newPS3Element = (XmlElement)importEntry.SelectSingleNode("./NewTextPS3");
            if( newPS3Element != null )
            {
               newPS3String = newPS3Element.InnerText.Replace("\r", "");

               if (!FinalResultStringMatch(originalText, newPS3String))
               {
                  ++originalDiffCount;
               }
            }

            XmlElement newX360Element = (XmlElement)importEntry.SelectSingleNode("./NewTextX360");
            if (newX360Element != null)
            {
               newX360String = newX360Element.InnerText.Replace("\r", "");

               if (!FinalResultStringMatch(originalText, newX360String))
               {
                  ++originalDiffCount;
               }
            }

            string statusString = "";
            string problemString = "";

            if (string.IsNullOrEmpty(newPS3String) && string.IsNullOrEmpty(newX360String))
            {
               // No override data?
               statusString = "IGNORE";
               problemString = "NO DATA";
            }
            else if (originalDiffCount == 0)
            {
               statusString = "IGNORE";
               problemString = "NO CHANGES";
            }
            else if (!ignoreProblem && !ValidateString(originalText, newPS3String, "PS3", ref statusString, ref problemString))
            {
               // Problems with PS3 string..
            }
            else if (!ignoreProblem && !ValidateString(originalText, newX360String, "X360", ref statusString, ref problemString))
            {
               // Problems with X360 string...
            }
            else if (newPS3String != null || newX360String != null)
            {
               // PS3/X360 comparison
               string ps3Compare = string.IsNullOrEmpty(newPS3String) ? "" : Misc.RemoveExtraNewlines(mStringDB.ApplyKeywordReplacement(newPS3String));
               string x360Compare = string.IsNullOrEmpty(newX360String) ? "" : Misc.RemoveExtraNewlines(mStringDB.ApplyKeywordReplacement(newX360String));
               
               bool allPlatformsTheSame = StringMatch(ps3Compare, x360Compare);

               StringEntry foundEntry = mStringDB.FindString(originalText);
               if (foundEntry != null)
               {
                  bool applyOverrideData = false;

                  // Check if override already exists.
                  StringOverride overrideEntry = mStringDB.GetOverrideEntry(foundEntry);
                  if (overrideEntry != null)
                  {
                     string temp;

                     // Update override INFO on xml node.
                     UpdateXmlNode(doc, importEntry, "BP-OverrideInfo", overrideEntry.Info);

                     // Check if the KJP overrides are actually different from the BP overrides.
                     int overrideCount = 0;
                     int overrideDiffCount = 0;

                     if (overrideEntry.GetOverride(TextPlatform.PS3, out temp))
                     {
                        ++overrideCount;
                        UpdateXmlNode(doc, importEntry, "BP-OverridePS3", temp);

                        if (!FinalResultStringMatch(newPS3String, temp))
                           overrideDiffCount++;
                     }

                     if (overrideEntry.GetOverride(TextPlatform.X360, out temp))
                     {
                        ++overrideCount;
                        UpdateXmlNode(doc, importEntry, "BP-OverrideX360", temp);

                        if (!FinalResultStringMatch(newX360String, temp))
                           overrideDiffCount++;
                     }

                     if (overrideCount != 0 && overrideDiffCount != 0)
                     {
                        statusString = "CONFLICT";

/*
                        if (overrideEntry.Info == null)
                           overrideEntry.Info = "";

                        // Apply "CONFLICT" tag into override info.
                        if( !overrideEntry.Info.Contains("CONFLICT") )
                        {
                           if(overrideEntry.Info.Length > 0)
                           {
                              overrideEntry.Info += "\n";
                           }
                           overrideEntry.Info += "CONFLICT\n";
                        }
*/

                        applyOverrideData = true;
                     }
                     else if (overrideCount != 0 && overrideDiffCount == 0)
                     {
                        statusString = "IGNORE";
                        problemString = "OVERRIDE MATCH";
                     }
                     else
                     {
                        statusString = "ACCEPT";
                        applyOverrideData = true;
                     }
                  }
                  else
                  {
                     statusString = "ACCEPT";
                     overrideEntry = mStringDB.SetStringOverride(foundEntry, TextPlatform.KJP_Default, "Dummy");
                     applyOverrideData = true;
                  }

                  // APPLY KJP OVERRIDES
                  if (applyOverrideData)
                  {
                     if (allPlatformsTheSame)
                     {
                        overrideEntry.OverrideString.Remove(TextPlatform.KJP_PS3);
                        overrideEntry.OverrideString.Remove(TextPlatform.KJP_X360);
                        
                        string overrideString = newX360String ?? newPS3String;

                        if (!string.IsNullOrEmpty(overrideString))
                           overrideEntry.OverrideString[TextPlatform.KJP_Default] = overrideString;
                        else
                           overrideEntry.OverrideString.Remove(TextPlatform.KJP_Default);
                     }
                     else
                     {
                        overrideEntry.OverrideString.Remove(TextPlatform.KJP_Default);

                        if (!string.IsNullOrEmpty(newPS3String))
                           overrideEntry.OverrideString[TextPlatform.KJP_PS3] = newPS3String;
                        else
                           overrideEntry.OverrideString.Remove(TextPlatform.KJP_PS3);

                        if (!string.IsNullOrEmpty(newX360String))
                           overrideEntry.OverrideString[TextPlatform.KJP_X360] = newX360String;
                        else
                           overrideEntry.OverrideString.Remove(TextPlatform.KJP_X360);
                     }
                  }

               }
               else
               {
                  // No override entry found!
                  statusString = "PROBLEM";
                  problemString = "NOT FOUND";
               }
            }

            UpdateXmlNode(doc, importEntry, "Status", statusString);
            UpdateXmlNode(doc, importEntry, "Problem", problemString);
         }
       
         doc.Save(inputFile);

         string dateString = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss");

         string backupPath = Path.Combine(Path.GetDirectoryName(inputFile), string.Format("{0}_{1}.xml", Path.GetFileNameWithoutExtension(inputFile), dateString));
         doc.Save(backupPath);

         Cursor.Current = Cursors.Default;
      }
   }
}
