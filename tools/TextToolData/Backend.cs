using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using TextToolHelper;
using System.Xml;
using System.Drawing;
using System.IO;

namespace TextToolData
{
   public class Backend
   {
      public TextRender TextRender;
      public CharacterMap CharacterMap;

      public Backend()
      {
         TextRender = new TextRender();

         CharacterMap = new CharacterMap();
         CharacterMap.Load();

         //MGS3 Path
         string keepFontPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/assets/raw/us/keepfont.raw");

         if (!File.Exists(keepFontPath))
         {
            //MGS2 Path
            keepFontPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/assets/row/us/keepfont.row");
         }

         TextRender.LoadResidentFont(keepFontPath);

         LoadBPFont();
      }

      public static string GetBPFontPath()
      {
         string bpFontPath = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/EngineSupport/Fonts/MGS_Font.raw");
         return bpFontPath;
      }

      public void LoadBPFont()
      {
         TextRender.BP_LoadFont(GetBPFontPath());
      }

      public byte[] ConvertStringToUTF8(string text)
      {
         string current = text;

         // Replace #U{....}
         while (true)
         {
            int begin = current.IndexOf("#U{");

            if (begin < 0)
               break;

            int end = current.IndexOf('}', begin);

            if (end < begin)
               break;

            current = current.Remove(begin, end - begin + 1);
            current = current.Insert(begin, "\uFFFF");
         }

         // Replace #H{....}
         while (true)
         {
            int begin = current.IndexOf("#H{");

            if (begin < 0)
               break;

            int end = current.IndexOf('}', begin);

            if (end < begin)
               break;

            string unicodeString = current.Substring(begin + 3, end - begin - 3);

            int value = Convert.ToInt32(unicodeString, 16);

            current = current.Remove(begin, end - begin + 1);

            current = current.Insert(begin, char.ConvertFromUtf32(value));
         }

         // Replace "#T{}"
         while (true)
         {
            int begin = current.IndexOf("#T{");

            if (begin < 0)
               break;

            int end = current.IndexOf('}', begin);

            if (end < begin)
               break;

            current = current.Remove(begin, end - begin + 1);

            //#define BP_TOP_KINSOKU_CHARACTER    0xE000
            current = current.Insert(begin, "\uE000");
         }

         // Replace "#B{}"
         while (true)
         {
            int begin = current.IndexOf("#B{");

            if (begin < 0)
               break;

            int end = current.IndexOf('}', begin);

            if (end < begin)
               break;

            current = current.Remove(begin, end - begin + 1);

            //#define BP_BACK_KINSOKU_CHARACTER   0xE001
            current = current.Insert(begin, "\uE001");
         }

         return System.Text.Encoding.UTF8.GetBytes(current);
      }

      const int TOP_KINSOKU_MASK = 0x4000;
      const int BACK_KINSOKU_MASK = 0x2000;

      public bool showAnsiiProblem = true;
      public bool showHiraganaProblem = true;
      public bool showKatakanaProblem = true;
      public bool showSymbolProblem = true;
      public bool showKanjiProblem = true;

      public bool IsProblemCode(int code)
      {
         if (code < 0x8100)
            return showAnsiiProblem;
         else if (code < 0x8200)
            return showHiraganaProblem;
         else if (code < 0x8300)
            return showKatakanaProblem;
         else if (code < 0x8400)
            return showSymbolProblem;
         else
            return showKanjiProblem;
      }

      public string DecodeString(XmlElement stringElement, List<int> uniqueCodes, List<int> unknownCodes, List<int> problemCodes)
      {
         try
         {
            SetFontForStringElement(stringElement);

            byte[] text = Convert.FromBase64String(stringElement.SelectSingleNode("./Text-Base64").InnerText);

            StringBuilder result = new StringBuilder();

            for (int i = 0; i < text.Length; )
            {
               bool skip = false;

               byte b = text[i];

               int code;

               if (b == 0x1f)
               {
                  code = 0x8000 | text[i + 1] + 0x7f;
                  i += 2;
               }
               else if (b < 0x80)
               {
                  code = 0x8000 | b;
                  ++i;
               }
               else
               {
                  code = (b << 8) | text[i + 1];
                  i += 2;
               }

               if (code == 0x8000)
                  skip = true;

               if( (code & TOP_KINSOKU_MASK) != 0 )
               {
                  result.Append("#T{}");
               }
               
               if( (code & BACK_KINSOKU_MASK) != 0 )
               {
                  result.Append("#B{}");
               }

               //TODO: These are flags that are used to determine the beginning and end of a block that can't be wrapped (90% positive on this)
               //For now we just mask these bits out, but we'll probably have to deal with this specially!
               code &= ~(TOP_KINSOKU_MASK | BACK_KINSOKU_MASK);

               if (!skip)
               {
                  if (uniqueCodes != null)
                  {
                     if (!uniqueCodes.Contains(code))
                        uniqueCodes.Add(code);
                  }

                  string character;

                  Bitmap bitmap = null;

                  if (CharacterMap.DoesCodeNeedBitmap(code))
                     bitmap = GenerateCharacterPreviewImage(code);

                  if (CharacterMap.LookupCode(code, bitmap, out character))
                  {
                     result.Append(character);
                  }
                  else
                  {
                     if (unknownCodes != null)
                     {
                        if (!unknownCodes.Contains(code))
                           unknownCodes.Add(code);
                     }

                     if (problemCodes != null)
                     {
                        if (IsProblemCode(code))
                        {
                           if (!problemCodes.Contains(code))
                              problemCodes.Add(code);
                        }
                     }
                     result.AppendFormat("#U{{{0}}}", code);
                  }
               }
            }

            String resultString = result.ToString();

            return resultString;
         }
         catch (Exception)
         {
            return "[Error While Decoding]";
         }
      }

      public Bitmap GenerateCharacterPreviewImage(int code)
      {
         List<byte> characterPreview = new List<byte>();
         characterPreview.Add((byte)((code & 0xFF00) >> 8));
         characterPreview.Add((byte)(code & 0x00FF));
         characterPreview.Add(0);

         return TextRender.RenderText(characterPreview.ToArray(), 24, 24, false);
      }

      private void SetFontForStringElement(XmlElement stringElement)
      {
         if (stringElement.Name == "Caption")
         {
            string fontId = stringElement.GetAttribute("fontId");

            XmlElement fontData = stringElement.OwnerDocument.SelectSingleNode(string.Format("//FontData-Base64[@Id='{0}']", fontId)) as XmlElement;
            byte[] fontDataBinary = Convert.FromBase64String(fontData.InnerText);

            TextRender.SetFontData(3/*FONT_TYPE_VOX*/, fontDataBinary);
         }
         else if (stringElement.Name == "String")
         {
            XmlElement fontData = stringElement.OwnerDocument.SelectSingleNode("//FontData-Base64") as XmlElement;
            byte[] fontDataBinary = Convert.FromBase64String(fontData.InnerText);

            TextRender.SetFontData(2/*FONT_TYPE_GCL*/, fontDataBinary);
         }
      }   
   }
}
