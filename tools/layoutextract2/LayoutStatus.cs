using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Windows.Forms;
using System.Drawing;

namespace LayoutExtract2
{
   public class LayoutStatusHeader
   {
      public List<LayoutStatusRef> mStatuses;
      LayoutData mLayoutData;

      public LayoutStatusHeader(BinaryReader br, uint ofStat, LayoutData ld)
      {
         mLayoutData = ld;

         br.BaseStream.Seek(ofStat, SeekOrigin.Begin);
         LayoutUtil.LogStreamLoad( br, "LayoutStatusHeader" );
         uint unStatusCount = br.ReadUInt32();
         LayoutUtil.LogStreamAddl( "count:", unStatusCount );

         mStatuses = new List<LayoutStatusRef>();

         for (uint i = 0; i < unStatusCount; ++i)
         {
            long fileOffset = br.BaseStream.Position;
            LayoutStatus status = new LayoutStatus(br, ld);
            LayoutStatusRef statusRef = new LayoutStatusRef(status, ld);
            statusRef.mFileOffset = fileOffset;
            statusRef.mName = LayoutUtil.WordLists.GetRandomNoun();

            // add to lookup tables
            ld.StatusByName.Add(statusRef.mName, statusRef);

            if (status.usId != 0xffff)
            {
               mLayoutData.ObjectByID[status.usId].AddStatus(statusRef);
            }

            mStatuses.Add(statusRef);
         }
      }
      public UInt32 Write(BinaryWriter bw)
      {
         UInt32 StatusOrigin = (UInt32)bw.BaseStream.Position;
         bw.Write((UInt32)mStatuses.Count);
         foreach (LayoutStatusRef statusRef in mStatuses)
            statusRef.Write(bw);
         return StatusOrigin;
      }

      public void ToXML(XmlElement layoutElement)
      {
         XmlElement statusesElement = layoutElement.OwnerDocument.CreateElement("Statuses");
         layoutElement.AppendChild(statusesElement);

         foreach (LayoutStatusRef statusRef in mStatuses)
         {
            statusRef.ToXML(statusesElement);
         }
      }

      public LayoutStatusHeader(XmlDocument doc, LayoutData ld)
      {
         mStatuses = new List<LayoutStatusRef>();
         foreach (XmlElement element in doc.SelectNodes("LayoutData/Statuses/Status"))
         {
            mStatuses.Add(new LayoutStatusRef(element, ld));
         }
      }
   }

   public class LayoutStatusRef
   {
      public LayoutStatus mStatus;
      public long mFileOffset;
      public string mName;
      public bool mWrittenToXml;
      LayoutData mLayoutData;

      public LayoutStatusRef(LayoutStatus status, LayoutData ld)
      {
         mLayoutData = ld;
         mStatus = status;
         mFileOffset = 0;
         mName = LayoutUtil.WordLists.GetRandomNoun();
         mWrittenToXml = false;
      }
      public void Write(BinaryWriter bw)
      {
         mStatus.Write(bw);
      }

      public void ToXML(XmlElement statusesElement)
      {
         XmlElement statusElement = statusesElement.OwnerDocument.CreateElement("Status");
         statusElement.SetAttribute("name", mName);
         statusesElement.AppendChild(statusElement);
         mStatus.ToXML(statusElement);
      }

      public LayoutStatusRef(XmlElement element, LayoutData ld)
      {
         mLayoutData = ld;
         mName = element.GetAttribute("name");
         mStatus = new LayoutStatus(element, ld);
         if (mStatus.usId != 0xffff)
         {
            mLayoutData.ObjectByID[mStatus.usId].AddStatus(this);
         }
         ld.StatusByName.Add(mName, this);
      }
   }

   public class LayoutStatus
   {
      public List<LayoutStatusState> mStates;
      LayoutData mLayoutData;

      public UInt16 usId;
      public UInt16 usNumStates;
      public UInt32 uiStrCode;

      public void ToXML(XmlElement statusElement)
      {
         if (uiStrCode != 0)
         {
            statusElement.SetAttribute("texCode", uiStrCode.ToString("x8"));
         }
         else
         {
            if (mLayoutData.ObjectByID.ContainsKey(usId))
            {
               statusElement.SetAttribute("object", mLayoutData.ObjectByID[usId].mName);
            }
            foreach (LayoutStatusState state in mStates)
            {
               state.ToXML(statusElement);
            }
         }
      }

      public LayoutStatus(XmlElement element, LayoutData ld)
      {
         mLayoutData = ld;
         mStates = new List<LayoutStatusState>();

         if (element.HasAttribute("texCode"))
         {
            uiStrCode = UInt32.Parse(element.GetAttribute("texCode"), System.Globalization.NumberStyles.HexNumber);
            usId = 0xffff;
            usNumStates = 0xffff;
         }
         else
         {
            usId = 0;
            if (element.HasAttribute("object"))
            {
               string name = element.GetAttribute("object");
               if (ld.ObjectByName.ContainsKey(name))
               {
                  usId = ld.ObjectByName[name].usID;
               }
            }
            uiStrCode = 0;
            foreach (XmlElement child in element.ChildNodes)
            {
               mStates.Add(new LayoutStatusState(child, ld));
            }
            usNumStates = (ushort) mStates.Count;
         }
      }

      public LayoutStatus(BinaryReader br, LayoutData ld)
      {
         mLayoutData = ld;
         mStates = new List<LayoutStatusState>();

         long statusStartPosition = br.BaseStream.Position;

         LayoutUtil.LogStreamLoad(br, "LayoutStatus");
         
         usId = br.ReadUInt16();
         usNumStates = br.ReadUInt16();
         uiStrCode = 0;
         if ((Int16)usId < 0 && (Int16)usNumStates < 0)
         {
            uiStrCode = br.ReadUInt32();   // Is Texture reference. See layout_data.c line 314

            LayoutUtil.LogStreamAddl("id:", usId, "strCode:", uiStrCode);
         }
         else
         {
            for (ushort ii = 0; ii < (Int16)usNumStates; ++ii)
            {
               mStates.Add(new LayoutStatusState(br, ld, uiStrCode));
            }
            LayoutUtil.LogStreamAddl("id:", usId, "numStates:", usNumStates);
         }
      }

      public void ResizeToRect(PointF inParentOffset, RectangleF inOrigRect, RectangleF inRect)
      {
         foreach (LayoutStatusState stat in mStates)
            foreach (LayoutStatusCmd cmd in stat.mCommands)
            {
               switch (cmd.GetCommand())
               {
               case LayoutStatusCmd.EStatCmd.VERTEX:
                  foreach (LayoutStatusCmd.Vertex vert in cmd.vertexes)
                  {
                     if (inOrigRect.Width != 0)
                        vert.x = ((((Single)vert.x - inOrigRect.X + inParentOffset.X) / inOrigRect.Width) * inRect.Width + inRect.X - inParentOffset.X);
                     else
                        vert.x = (inRect.X - inParentOffset.X);

                     if (inOrigRect.Height != 0)
                        vert.y = ((((Single)vert.y - inOrigRect.Y + inParentOffset.Y) / inOrigRect.Height) * inRect.Height + inRect.Y - inParentOffset.Y);
                     else
                        vert.y = (inRect.Y - inParentOffset.Y);
                  }
                  break;
               case LayoutStatusCmd.EStatCmd.SIZE:
                  if (inOrigRect.Width != 0)
                     cmd.size_w = (((Single)cmd.size_w / inOrigRect.Width) * inRect.Width);
                  else
                     cmd.size_w = (inRect.Width);

                  if (inOrigRect.Height != 0)
                     cmd.size_h = (((Single)cmd.size_h / inOrigRect.Height) * inRect.Height);
                  else
                     cmd.size_h = (inRect.Height);
                  break;
               case LayoutStatusCmd.EStatCmd.CENTER:
                  if (inOrigRect.Width != 0)
                     cmd.center_x = ((((Single)cmd.center_x - inOrigRect.X + inParentOffset.X) / inOrigRect.Width) * inRect.Width + inRect.X - inParentOffset.X);
                  else
                     cmd.center_x = ((inRect.Width + inRect.X - inParentOffset.X) / 2);

                  if (inOrigRect.Height != 0)
                     cmd.center_y = ((((Single)cmd.center_y - inOrigRect.Y + inParentOffset.Y) / inOrigRect.Height) * inRect.Height + inRect.Y - inParentOffset.Y);
                  else
                     cmd.center_y = ((inRect.Height + inRect.Y - inParentOffset.Y) / 2);
                  break;
               }
            }
      }
      public void Write(BinaryWriter bw)
      {
         bw.Write(usId);
         if ((Int16)usNumStates < 1)
         {
            bw.Write(usNumStates);
         }
         else
         {
            bw.Write((Int16)mStates.Count);
         }

         if ((Int16)usId < 0 && (Int16)usNumStates < 0)
         {
            bw.Write(uiStrCode);
         }

         foreach (LayoutStatusState statusState in mStates)
         {
            statusState.Write(bw);
         }
      }
   }
}
