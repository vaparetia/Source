using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Drawing;
using System.Drawing.Imaging;
using System.Security.Cryptography;

namespace TextToolData
{
   public class CharacterMap
   {
      Dictionary<string, string> mKanjiMap = new Dictionary<string, string>();
      Dictionary<int, string> mCharacterMap = new Dictionary<int, string>();
      bool mNeedsSave = false;

      public CharacterMap()
      {
      }

      public void Load()
      {
         mCharacterMap.Clear();
         mKanjiMap.Clear();

         XmlDocument doc = new XmlDocument();
         try
         {
            doc.Load(GetCharacterMapFilename());
         }
         catch (Exception)
         {
         }

         foreach (XmlElement characterMapping in doc.SelectNodes("//Character"))
         {
            int code = int.Parse(characterMapping.GetAttribute("code"));
            string character = characterMapping.GetAttribute("char");
            mCharacterMap[code] = character;
         }

         foreach (XmlElement characterMapping in doc.SelectNodes("//Kanji"))
         {
            string md5 = characterMapping.GetAttribute("md5");
            string character = characterMapping.GetAttribute("char");
            mKanjiMap[md5] = character;
         }

         mCharacterMap[0x8009] = "\u0009";
         mCharacterMap[0x800a] = "\n";
         mCharacterMap[0x807c] = "|";
         mCharacterMap[0x8020] = " ";
         
         mCharacterMap[0x8301] = "\u3000"; //Ideographic Space
         mCharacterMap[0x8308] = "\u3001"; //Ideographic Comma
         mCharacterMap[0x8309] = "\u3002"; //Ideographic Full Stop
         mCharacterMap[0x7000] = "\u00d7"; //Multiplication Sign

      }

      public bool NeedsSaving()
      {
         //AndyO: I've forgotten how to do C# properties! Yikes!
         return mNeedsSave;
      }

      public void AutoSave()
      {
         if (mNeedsSave)
         {
            mNeedsSave = false;
         
            XmlDocument doc = new XmlDocument();
            XmlElement docRoot = doc.AppendChild(doc.CreateElement("CharacterMap")) as XmlElement;

            foreach(KeyValuePair<int, string> item in mCharacterMap)
            {
               XmlElement characterMapping = docRoot.AppendChild(doc.CreateElement("Character")) as XmlElement;
               characterMapping.SetAttribute("code", item.Key.ToString());
               characterMapping.SetAttribute("char", item.Value);
            }

            foreach (KeyValuePair<string, string> item in mKanjiMap)
            {
               XmlElement characterMapping = docRoot.AppendChild(doc.CreateElement("Kanji")) as XmlElement;
               characterMapping.SetAttribute("md5", item.Key);
               characterMapping.SetAttribute("char", item.Value);
            }

            doc.Save(GetCharacterMapFilename());
         }
      }

      public string GenerateBitmapHash(Bitmap image)
      {
         // You could just store the image to memory or do something else, but this is guranteed to have no padding data (i.e. stride alignment) issues.
         byte[] rgbValues = new byte[image.Width * image.Height * 3];

         for (int y = 0; y < image.Height; ++y)
         {
            for(int x = 0; x < image.Width; ++x )
            {
               Color color = image.GetPixel(x, y);
               int offset = (y * image.Width + x) * 3;
               rgbValues[offset + 0] = color.R;
               rgbValues[offset + 1] = color.G;
               rgbValues[offset + 2] = color.B;
            }
         }

         string hash = Convert.ToBase64String(new System.Security.Cryptography.MD5CryptoServiceProvider().ComputeHash(rgbValues));
         
         return hash;
      }

      public bool DoesCodeNeedBitmap(int code)
      {
         if (code >= 0x8000 && code < 0x8400)
            return false;

         return true;
      }

      public bool LookupCode(int code, Bitmap characterImage, out string character)
      {
         if (DoesCodeNeedBitmap(code))
         {
            string hash = GenerateBitmapHash(characterImage);
            //Console.WriteLine("Hash: {0} Code: {1:x}", hash, code);

            return mKanjiMap.TryGetValue(hash, out character);
         }
         else
         {
            return mCharacterMap.TryGetValue(code, out character);
         }
      }

      public bool SetCode(int code, Bitmap characterImage, string character)
      {
         bool didSet = true;

         string oldCharacter;

         if (!DoesCodeNeedBitmap(code))
         {
            mCharacterMap.TryGetValue(code, out oldCharacter);
            mCharacterMap[code] = character;
         }
         else
         {
            string hash = GenerateBitmapHash(characterImage);

            mKanjiMap.TryGetValue(hash, out oldCharacter);
            mKanjiMap[hash] = character;
         }

         if (oldCharacter == character)
            didSet = false;
      
         if( didSet )
            mNeedsSave = true;
         
         return didSet;

      }

      public bool RemoveCode(int code, Bitmap characterImage)
      {
         bool didRemove = false;

         if (!DoesCodeNeedBitmap(code))
         {
            didRemove = mCharacterMap.Remove(code);
         }
         else
         {
            string hash = GenerateBitmapHash(characterImage);
            didRemove = mKanjiMap.Remove(hash);
         }
         
         if( didRemove )
            mNeedsSave = true;
         
         return didRemove;
      }

      public string GetCharacterMapFilename()
      {
         string path = Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%/CharacterMap.xml");
         return path;
      }
   }
}
