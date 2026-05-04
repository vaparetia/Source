using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.Common
{
   /// <summary>
   /// MergeText handles creating and updating blocks of text in user supplied strings.
   /// Used by PropertiesToCode to update generated .cpp code in source files.
   /// Handles multiple blocks within same file (string) and checking for modification.
   /// </summary>
   public class MergeText
   {
      private string    mName;
      private string    mText;
      private string    mMarkedText;
      private uint      mTextCRC;
      private uint      mNameCRC;

      public enum EMergeResult
      {
         kOK,
         kNoChange,
         kCouldntFindMatch,
         kCRCMismatch,
      }

      [Flags]
      public enum EMergeOptions
      {
         kNone,
         kIgnoreCRC  = 1,
      }

      public string           MarkedText  { get { return mMarkedText + "\n"; } }    // Append CR when requested externally
      public string           Name        { get { return mName; } }

      public MergeText(string name, string text)
      {
         mName = name;
         mNameCRC = CRC32.CalculateCRC(mName);
         mText = text;
         mTextCRC = CRC32.CalculateCRC(NormalizeWhitespace(mText));
         mMarkedText = String.Format("//Start:{0}:{1:X8}:{2:X8} *** Machine generated code - do not edit ***\n", mName, mNameCRC, mTextCRC);
         mMarkedText += mText;
         mMarkedText += String.Format("//End:{0}:{1:X8} *** Machine generated code - do not edit *** $End$:{0}:{1:X8}", mName, mNameCRC, mTextCRC);
      }

      public EMergeResult Merge(string inputText, out string outputText, EMergeOptions options)
      {
         outputText = String.Empty;

         // Find start and end of of marked block
         string blockStart = String.Format("//Start:{0}:{1:X8}:", mName, mNameCRC);
         int blockStartPos = inputText.IndexOf(blockStart);
         if (blockStartPos == -1)
         {
            return EMergeResult.kCouldntFindMatch;
         }

         // Find text marker end
         string textEnd = String.Format("\n//End:{0}:{1:X8}", mName, mNameCRC);
         int textEndPos = inputText.IndexOf(textEnd);
         if (textEndPos == -1)
         {
            return EMergeResult.kCouldntFindMatch;
         }

         // Find block marker end
         string blockEnd = String.Format("$End$:{0}:{1:X8}", mName, mNameCRC);
         int blockEndPos = inputText.IndexOf(blockEnd);
         if (blockEndPos == -1)
         {
            return EMergeResult.kCouldntFindMatch;
         }
         blockEndPos += blockEnd.Length;

         // Get previous text CRC
         uint originalBlockMarkerCRC;
         int textStartPos = blockStartPos + blockStart.Length;
         try
         {
            originalBlockMarkerCRC = UInt32.Parse(inputText.Substring(textStartPos, 8), System.Globalization.NumberStyles.HexNumber);
         }
         catch(Exception)
         {
            // Couldn't read CRC
            return EMergeResult.kCRCMismatch;
         }
         
         // Move to start of text
         textStartPos = inputText.IndexOf('\n', textStartPos);
         if (textStartPos == -1)
         {
            return EMergeResult.kCouldntFindMatch;
         }

         // Skip CR
         textStartPos++;

         // Make sure that start is before end
         if (blockStartPos >= blockEndPos)
         {
            return EMergeResult.kCouldntFindMatch;
         }

         // Now confirm CRC
         string originalText = inputText.Substring(textStartPos, textEndPos - textStartPos);
         uint originalTextCRC = CRC32.CalculateCRC(NormalizeWhitespace(originalText));
         
         if ((options & EMergeOptions.kIgnoreCRC) == 0)
         {
            if (originalTextCRC != originalBlockMarkerCRC)
            {
               return EMergeResult.kCRCMismatch;
            }
         }
         
         // Now, remove original and reinsert new.
         // Even if CRC hasn't changed, the whitespace might have.
         // Whitespace changes won't check out the file, but will update a file with other changes.
         outputText = inputText.Substring(0, blockStartPos);
         outputText += mMarkedText;
         outputText += inputText.Substring(blockEndPos);
         
         // Confirm that text has changed
         if (originalTextCRC == mTextCRC)
         {
            return EMergeResult.kNoChange;
         }
         
         return EMergeResult.kOK;
      }

      /// <summary>
      /// This deals with editors converting line feeds and tabs to different characters and changing the CRC
      /// </summary>
      static string NormalizeWhitespace(string text)
      {
         StringBuilder outString = new StringBuilder();
         outString.EnsureCapacity(text.Length);

         // Dont add unwanted characters
         foreach(char c in text)
         {
            switch (c)
            {
               case '\r':  break;
               case '\n':  break;
               case '\t':  break;
               case ' ':   break;

               default:
                  outString.Append(c);
                  break;
            }
         }

         return outString.ToString();
      }
   }
}
