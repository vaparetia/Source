using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Drawing;

namespace LayoutExtract2
{
   public class LayoutData
   {
      class LayoutException : Exception
      {
         public LayoutException()
         {
         }
      }

      class LayoutSection : IComparable<LayoutSection>
      {
         public LayoutSection(uint _offset, string _name)
         {
            offset = _offset;
            name = _name;
         }
         public uint offset;
         public string name;

         #region IComparable<LayoutSection> Members
         public int CompareTo(LayoutSection b)
         {
            return ((int)offset - (int)b.offset);
         }
         #endregion
      }

      static public uint OLA_VERSION_1_1 = 0x01010000;
      static public uint OLA_VERSION_1_2 = 0x02010000;
      static public uint OLA_VERSION_1_3 = 0x03010000;
      static public uint OLA_VERSION_HD  = 0x10000000;

      public uint                version;
      public uint                widescreen;
      public uint                xsize;
      public uint                ysize;
      public uint                ofObj;
      public uint                ofStat;
      public uint                ofAct;

      public LayoutObjectHeader  mObjectHeader;
      public LayoutStatusHeader  mStatusHeader;
      public LayoutActionHeader  mActionHeader;

      #region Public Data Aggregation
      public Dictionary<ushort, LayoutObject>            ObjectByID = new Dictionary<ushort, LayoutObject>();
      public Dictionary<string, LayoutObject>            ObjectByName = new Dictionary<string, LayoutObject>();

      public Dictionary<string, LayoutStatusRef>         StatusByName = new Dictionary<string, LayoutStatusRef>();

      public Dictionary<string, LayoutActionRef>         ActionByName = new Dictionary<string, LayoutActionRef>();
      #endregion

      public Dictionary<uint, String> mStringIDsFromCode;

      public LayoutData()
      {
         version = OLA_VERSION_1_3;
         widescreen = 0;
      }

      public void NoteError(int i)
      {
         List<LayoutSection> sections = new List<LayoutSection>();
         sections.Add(new LayoutSection(ofObj, "Objects"));
         sections.Add(new LayoutSection(ofStat, "Statuses"));
         sections.Add(new LayoutSection(ofAct, "Actions"));

         sections.Sort();

         // check which data block this belongs to
         LayoutSection foundSec = null;
         foreach (LayoutSection sec in sections)
         {
            if (i >= sec.offset)
            {
               foundSec = sec;
            }
         }
         System.Console.WriteLine("Discrepancy at position {0}, in section {1}!", i, foundSec != null ? foundSec.name : "unknown");
      }

      public static void ReadLayoutData(LayoutData data, Byte[] inData)
      {
         BinaryReader binReader = new BinaryReader(new MemoryStream(inData));
         LayoutData.ReadLayoutData(data, binReader);
         binReader.Close();
      }
      public static void ReadLayoutData(LayoutData data, BinaryReader br)
      {
         data.Reset();
         data.version = br.ReadUInt32();
         if (data.version != OLA_VERSION_1_2 && data.version != OLA_VERSION_1_1 && data.version != OLA_VERSION_1_3)
         {
            throw new LayoutException();
         }
         if (data.version == OLA_VERSION_1_3)
         {
            data.xsize = br.ReadUInt32();
            data.ysize = br.ReadUInt32();
         }
         data.ofObj = br.ReadUInt32();
         data.ofStat = br.ReadUInt32();
         data.ofAct = br.ReadUInt32();

         data.widescreen = data.ofObj & OLA_VERSION_HD;
         data.ofObj = data.ofObj & ~OLA_VERSION_HD;

         if (data.version < OLA_VERSION_1_3)
         {
            if (data.widescreen > 0)
            {
               data.xsize = 1280;
               data.ysize = 720;
            }
            else
            {
               data.xsize = 512;
               data.ysize = 400;
            }
         }

         data.mObjectHeader = new LayoutObjectHeader(br, data.ofObj, data);
         data.mStatusHeader = new LayoutStatusHeader(br, data.ofStat, data);
         data.mActionHeader = new LayoutActionHeader(br, data.ofAct, data);

         // update to the latest version
         data.version = OLA_VERSION_1_3;
      }
      public void Write(BinaryWriter bw)
      {
         int headSize = version == OLA_VERSION_1_3 ? 24 : 16;
         bw.Seek(headSize, SeekOrigin.Current);

         ofObj = mObjectHeader.Write(bw);
         ofStat = mStatusHeader.Write(bw);
         ofAct = mActionHeader.Write(bw);

         long position = bw.BaseStream.Position;
         // pad with zeros to 16 byte alignment
         long padCount = (16 - (position & 15)) & 15;
         while (--padCount >= 0)
         {
            byte padValue = 0;
            bw.Write(padValue);
         }

         bw.Seek(0, SeekOrigin.Begin);
         bw.Write(version);
         if (version == OLA_VERSION_1_3)
         {
            bw.Write(xsize);
            bw.Write(ysize);
         }
         bw.Write(widescreen | ofObj);
         bw.Write(ofStat);
         bw.Write(ofAct);
      }

      public void ToXML(XmlDocument doc, bool newStrCodes)
      {
         //ResetStatusesForWritingToXml();

         XmlElement layoutElement = doc.CreateElement("LayoutData");
         doc.AppendChild(layoutElement);
         {
            XmlElement infoElement = doc.CreateElement("Info");
            infoElement.SetAttribute("widescreen", widescreen.ToString("x8"));
            infoElement.SetAttribute("xsize", xsize.ToString());
            infoElement.SetAttribute("ysize", ysize.ToString());
            layoutElement.AppendChild(infoElement);

            mObjectHeader.ToXML(layoutElement, newStrCodes);
            mStatusHeader.ToXML(layoutElement);
            mActionHeader.ToXML(layoutElement, newStrCodes);
            // the rest are deduced from the obj/stat/act
         }
      }

      public LayoutData(XmlDocument doc)
      {
         XmlElement infoElement = (XmlElement) doc.SelectSingleNode("LayoutData/Info");
         version = OLA_VERSION_1_3;
         widescreen = UInt32.Parse(infoElement.GetAttribute("widescreen"), System.Globalization.NumberStyles.HexNumber);
         xsize = UInt32.Parse(infoElement.GetAttribute("xsize"));
         ysize = UInt32.Parse(infoElement.GetAttribute("ysize"));
         mObjectHeader = new LayoutObjectHeader(doc, this);
         mStatusHeader = new LayoutStatusHeader(doc, this);
         mActionHeader = new LayoutActionHeader(doc, this);
      }

      public Boolean IsFormattedForHD()
      {
         return (widescreen & OLA_VERSION_HD) == OLA_VERSION_HD;
      }
      public void SetIsFormattedForHD(Boolean inIsFormattedForHD)
      {
         widescreen = widescreen | OLA_VERSION_HD;
      }

      public Rectangle GetLayoutBounds(Rectangle visibleBounds, Double previewWidth, Double previewHeight)
      {
         float gCW = xsize;
         float gCH = ysize;

         float toWidescreen = (float)(previewWidth / previewHeight) / (gCW / gCH);

         // first work out scales
         float sw = (Single) visibleBounds.Width;
         sw /= toWidescreen;
         float sh = (Single) visibleBounds.Height;
         float xscale = 0.9f * sw / gCW;
         float yscale = 0.9f * sh / gCH;
         float scale = (xscale > yscale) ? yscale : xscale;
         yscale = scale;
         xscale = scale * toWidescreen;
         sw *= toWidescreen;
         float xoff = 0.5f * sw - 0.5f * gCW * xscale;
         float yoff = 0.5f * sh - 0.5f * gCH * yscale;

         return new Rectangle((Int32)(xoff), (Int32)(yoff), (Int32)(gCW * xscale), (Int32)(gCH * yscale));
      }

      public void Reset()
      {
         ActionByName.Clear();
         ObjectByID.Clear();
         ObjectByName.Clear();
         StatusByName.Clear();
      }
   }
}
