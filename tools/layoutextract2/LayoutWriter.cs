using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace LayoutExtract
{
   public class LayoutWriter
   {
      static string skTab = "   ";
      public LayoutWriter(string fileName)
      {
         if (File.Exists(fileName)) File.Delete(fileName);
         FileStream fst = File.OpenWrite(fileName);
         mWriter = new StreamWriter(fst);
         mStartOfLine = true;
         mTab = "";
         mSections = new Stack<string>();
      }
      public void Write(string format, params object[] args)
      {
         if (mStartOfLine)
         {
            mWriter.Write(mTab);
         }
         mWriter.Write(format, args);
         mStartOfLine = false;
      }
      public void WriteLine(string format, params object[] args)
      {
         if (mStartOfLine)
         {
            mWriter.Write(mTab);
         }
         mWriter.WriteLine(format, args);
         mStartOfLine = true;
      }
      public void Flush()
      {
         mWriter.Flush();
      }
      public void Close()
      {
         mWriter.Close();
         mWriter = null;
      }
      public void TabIn()
      {
         mTab += skTab;
      }
      public void TabOut()
      {
         mTab = mTab.Remove(mTab.Length - skTab.Length);
      }
      public void StartSection(string secName)
      {
         mSections.Push(secName);
         WriteLine("<{0}>", secName);
         TabIn();
      }
      public void EndSection()
      {
         string secName = mSections.Pop();
         TabOut();
         WriteLine("</{0}>", secName);
      }

      bool mStartOfLine;
      string mTab;
      StreamWriter mWriter;
      Stack<string> mSections;
   }
}
