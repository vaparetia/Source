using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Drawing;
using System.Windows.Forms;
using System.Xml;

namespace LayoutExtract2
{
   public enum SpriteType : byte
   {
      Empty,
      Point,
      Line,
      LineStrip,
      Box,
      Sprite,
      Zoom,
      Spin,
      Poly,
      Triangle
   }

   public class LayoutObjectHeader
   {
      public List<LayoutObject> mAllObjects;
      public List<LayoutObject> mRootObjects;
      LayoutData mLayoutData;

      public LayoutObjectHeader(BinaryReader br, uint ofObj, LayoutData ld)
      {
         mLayoutData = ld;
         br.BaseStream.Seek(ofObj, SeekOrigin.Begin);
         LayoutUtil.LogStreamObject( br, this );

         UInt32 usObjectCount = br.ReadUInt32();

         mAllObjects = new List<LayoutObject>();
         for (int i = 0; i < usObjectCount; ++i)
         {
            LayoutObject obj = new LayoutObject(br, mLayoutData);
            mAllObjects.Add(obj);
         }

         mRootObjects = new List<LayoutObject>();
         foreach (LayoutObject obj in mAllObjects)
         {
            if (obj.usParentID == 0xffff)
            {
               mRootObjects.Add(obj);
            }
            else
            {
               LayoutObject parent;
               if (mLayoutData.ObjectByID.TryGetValue(obj.usParentID, out parent))
               {
                  parent.mChildren.Add(obj);
               }
            }
         }
      }
      public UInt32 Write(BinaryWriter bw)
      {
         UInt32 Offset = (UInt32)bw.BaseStream.Position;
         bw.Write((UInt32)mAllObjects.Count);

         foreach (LayoutObject obj in mAllObjects)
         {
            obj.Write(bw);
         }
         return Offset;
      }

      void PostCreateRecursive(LayoutObject cur)
      {
         mAllObjects.Add(cur);
         mLayoutData.ObjectByID.Add(cur.usID, cur);
         mLayoutData.ObjectByName.Add(cur.mName, cur);
         foreach (LayoutObject child in cur.mChildren)
         {
            PostCreateRecursive(child);
         }
      }

      public LayoutObjectHeader(XmlDocument doc, LayoutData ld)
      {
         mLayoutData = ld;
         mAllObjects = new List<LayoutObject>();
         mRootObjects = new List<LayoutObject>();
         ushort id = 0;
         XmlNodeList rootObjNodes = doc.SelectNodes("LayoutData/Objects/Object");
         mRootObjects = new List<LayoutObject>();
         foreach (XmlNode rootObjNode in rootObjNodes)
         {
            mRootObjects.Add(new LayoutObject((XmlElement)rootObjNode, 0xffff, ref id, ld));
         }
         foreach (LayoutObject rootObject in mRootObjects)
         {
            PostCreateRecursive(rootObject);
         }
      }

      public void ToXML(XmlElement layoutElement, bool newStrCodes)
      {
         XmlElement objectsElement = layoutElement.OwnerDocument.CreateElement("Objects");
         layoutElement.AppendChild(objectsElement);

         foreach (LayoutObject rootObject in mRootObjects)
         {
            rootObject.ToXML(objectsElement, newStrCodes);
         }
      }

      public void PopulateTreeView(TreeView inTreeView, PointF scale)
      {
         inTreeView.BeginUpdate();
         inTreeView.Nodes.Clear();
         foreach (LayoutObject obj in mRootObjects)
         {
            obj.PopulateTreeView(inTreeView.Nodes, scale);
         }
         inTreeView.EndUpdate();
      }
      public void Draw(Graphics g, LayoutObject inSelectedObject, Double previewWidth, Double previewHeight, Boolean fitToScreenHeight, Boolean inDrawSafeZone)
      {
         float gCW = mLayoutData.xsize;
         float gCH = mLayoutData.ysize;

         if (fitToScreenHeight)
         {
            gCH *= 384.0f / 400.0f;
         }

         float toWidescreen = (float)(previewWidth / previewHeight) / (gCW / gCH);

         // first work out scales
         float sw = (float)g.VisibleClipBounds.Width;
         sw /= toWidescreen;
         float sh = (float)g.VisibleClipBounds.Height;
         float xscale = 0.9f * sw / gCW;
         float yscale = 0.9f * sh / gCH;
         float scale = (xscale > yscale) ? yscale : xscale;
         yscale = scale;
         xscale = scale * toWidescreen;
         sw *= toWidescreen;
         float xoff = 0.5f * sw - 0.5f * gCW * xscale;
         float yoff = 0.5f * sh - 0.5f * gCH * yscale;

         g.TranslateTransform(xoff, yoff);
         g.ScaleTransform(xscale, yscale);
         foreach (LayoutObject obj in mRootObjects)
         {
             obj.Draw(g);
         }
         // draw corners last
         Brush b = System.Drawing.Brushes.Red;
         float thin = 10.0f;
         float thick = 50.0f;
         float x2 = gCW;
         float y2 = gCH;
         // top left
         g.FillRectangle(b, -thin, -thin, thin, thick);
         g.FillRectangle(b, -thin, -thin, thick, thin);
         // bottom left
         g.FillRectangle(b, -thin, y2 - thick + thin, thin, thick);
         g.FillRectangle(b, -thin, y2, thick, thin);
         // top right
         g.FillRectangle(b, x2, -thin, thin, thick);
         g.FillRectangle(b, x2 - thick + thin, -thin, thick, thin);
         // bottom right
         g.FillRectangle(b, x2, y2 - thick + thin, thin, thick);
         g.FillRectangle(b, x2 - thick + thin, y2, thick, thin);

         foreach (LayoutObject obj in mRootObjects)
         {
            obj.DrawSelected(g, inSelectedObject);
         }

         if (inDrawSafeZone)
         {
            g.DrawRectangle(System.Drawing.Pens.Orange, (Int32)(gCW * 0.075), (Int32)(gCH * 0.075), (Int32)(gCW * 0.85), (Int32)(gCH * 0.85));
         }

         g.ResetTransform();
      }
   }

   public class LayoutObject
   {
      LayoutData mLayoutData;
      public uint strcode;
      public ushort usID;
      public ushort usParentID;
      public SpriteType mSpriteType;
      public byte cVertex;

      public String mName;
      public List<LayoutObject> mChildren;
      public List<LayoutStatusRef> mStatuses;
      LayoutStatusState mStatusState;
      public bool mVisible;

      public LayoutObject(BinaryReader br, LayoutData ld)
      {
         mLayoutData = ld;

         LayoutUtil.LogStreamObject(br, this);

         strcode = br.ReadUInt32();
         usID = br.ReadUInt16();
         usParentID = br.ReadUInt16();
         mSpriteType = (SpriteType)br.ReadByte();
         cVertex = br.ReadByte();
         if (ld.mStringIDsFromCode.ContainsKey(strcode))
         {
            mName = ld.mStringIDsFromCode[strcode];
         }
         else
         {
            mName = LayoutUtil.WordLists.GetRandomNoun();
         }

         mLayoutData.ObjectByID[usID] = this;
         mLayoutData.ObjectByName[mName] = this;

         mChildren = new List<LayoutObject>();
         mStatuses = new List<LayoutStatusRef>();
         mStatusState = null;
         mVisible = true;

         UInt16 pad1 = br.ReadUInt16();
         UInt32 pad2 = br.ReadUInt32();
      }
      public void Write(BinaryWriter bw)
      {
         bw.Write(strcode);
         bw.Write(usID);
         bw.Write(usParentID);
         bw.Write((Byte)mSpriteType);
         bw.Write((Byte)cVertex);

         UInt16 pad1 = 0;
         UInt32 pad2 = 0;
         bw.Write(pad1);
         bw.Write(pad2);
      }

      public void AddStatus(LayoutStatusRef statusRef)
      {
         mStatuses.Add(statusRef);
      }

      private uint GV_StrCode(String x)
      {
         uint id = 0;

         foreach (char c in x)
         {
            if (c != '\0')
            {
               id = (id << 5) | (id >> 19);
               id = (id + c) & 0xffffff;
            }
         }

         return id;
      }

      public LayoutObject(XmlElement element, ushort parentID, ref ushort id, LayoutData ld)
      {
         mLayoutData = ld;
         mName = element.GetAttribute("name");
         mSpriteType = (SpriteType)Enum.Parse(typeof(SpriteType), element.GetAttribute("type"));
         cVertex = Byte.Parse(element.GetAttribute("vertex"));
         mVisible = true;
         if (element.HasAttribute("visible") && element.GetAttribute("visible") == "false")
         {
            mVisible = false;
         }
         if (element.HasAttribute("strcode"))
         {
            strcode = UInt32.Parse(element.GetAttribute("strcode"), System.Globalization.NumberStyles.HexNumber);
         }
         else
         {
            strcode = GV_StrCode(mName);
         }
         usID = id++;
         usParentID = parentID;

         mChildren = new List<LayoutObject>();
         foreach (XmlElement child in element.ChildNodes)
         {
            mChildren.Add(new LayoutObject(child, usID, ref id, ld));
         }
         mStatuses = new List<LayoutStatusRef>();
      }

      public void ToXML(XmlElement objectsElement, bool newStrCodes)
      {
         XmlElement objectElement = objectsElement.OwnerDocument.CreateElement("Object");
         objectsElement.AppendChild(objectElement);
         objectElement.SetAttribute("name", mName);
         objectElement.SetAttribute("type", mSpriteType.ToString());
         objectElement.SetAttribute("vertex", cVertex.ToString());

         if (!mVisible)
         {
            objectElement.SetAttribute("visible", "false");
         }
         if (!newStrCodes && strcode > 0)
         {
            objectElement.SetAttribute("strcode", String.Format("{0:x8}", strcode));
         }

         // and children
         foreach (LayoutObject child in mChildren)
         {
            child.ToXML(objectElement, newStrCodes);
         }
      }

      public LayoutStatusState GetCurrentStatusState()
      {
         if (mStatusState != null)
         {
            return mStatusState;
         }
         // otherwise just return the first state
         if (mStatuses.Count > 0 && mStatuses[0].mStatus.mStates.Count > 0)
         {
            return mStatuses[0].mStatus.mStates[0];
         }
         return null;
      }
      public void SetCurrentStatusState(LayoutStatusState state)
      {
         mStatusState = state;
         mVisible = mStatusState.GetVisible();
      }

      public LayoutObject GetParent()
      {
         if (!mLayoutData.ObjectByID.ContainsKey(usParentID))
         {
            return null;
         }
         return mLayoutData.ObjectByID[usParentID];
      }
      public RectangleF GetWorldBounds()
      {
         if (GetCurrentStatusState() == null)
         {
            return new RectangleF(0, 0, 0, 0);
         }

         LayoutObject parentObj = GetParent();
         if (parentObj == null)
         {
            return GetCurrentStatusState().GetLocalBounds();
         }
         
         RectangleF parentWorldBounds = parentObj.GetWorldBounds();
         return GetCurrentStatusState().GetBounds(parentWorldBounds.Location);
      }
      public void OffsetBounds(Single inXOffset, Single inYOffset, Single inWidthOffset, Single inHeightOffset)
      {
         RectangleF baseRect = GetWorldBounds();
         Single vertWidthScale = (baseRect.Width + inWidthOffset) / baseRect.Width;
         Single vertHeightScale = (baseRect.Height + inHeightOffset) / baseRect.Height;
         foreach (LayoutStatusRef statRef in mStatuses)
         {
            foreach (LayoutStatusState stat in statRef.mStatus.mStates)
            {
               foreach (LayoutStatusCmd cmd in stat.mCommands)
               {
                  switch (cmd.GetCommand())
                  {
                  case LayoutStatusCmd.EStatCmd.VERTEX:
                     foreach (LayoutStatusCmd.Vertex vert in cmd.vertexes)
                     {
                        if (inWidthOffset != 0)
                           vert.x = (vert.x - baseRect.X) * vertWidthScale + baseRect.X;
                        vert.x = Convert.ToSingle(vert.x + inXOffset);

                        if (inHeightOffset != 0)
                           vert.y = (vert.y - baseRect.Y) * vertHeightScale + baseRect.Y;
                        vert.y = Convert.ToSingle(vert.y + inYOffset);
                     }
                     break;
                  case LayoutStatusCmd.EStatCmd.SIZE:
                     cmd.size_w = Convert.ToSingle(cmd.size_w + inWidthOffset);
                     cmd.size_h = Convert.ToSingle(cmd.size_h + inHeightOffset);
                     break;
                  case LayoutStatusCmd.EStatCmd.CENTER:
                     cmd.center_x = Convert.ToSingle(cmd.center_x + inXOffset);
                     cmd.center_y = Convert.ToSingle(cmd.center_y + inYOffset);
                     break;
                  }
               }
            }
         }
      }
      public PointF GetCenter()
      {
         foreach (LayoutStatusRef statRef in mStatuses)
         {
            foreach (LayoutStatusState stat in statRef.mStatus.mStates)
            {
               foreach (LayoutStatusCmd cmd in stat.mCommands)
               {
                  switch (cmd.GetCommand())
                  {
                     case LayoutStatusCmd.EStatCmd.CENTER:
                        return new PointF(cmd.center_x, cmd.center_y);
                  }
               }
            }
         }

         return new PointF(0.0f, 0.0f);
      }
      public void OffsetCenter(PointF newCenter)
      {
         PointF oldCenter = GetCenter();
         float xOffset = newCenter.X - oldCenter.X;
         float yOffset = newCenter.Y - oldCenter.Y;

         foreach (LayoutStatusRef statRef in mStatuses)
         {
            foreach (LayoutStatusState stat in statRef.mStatus.mStates)
            {
               foreach (LayoutStatusCmd cmd in stat.mCommands)
               {
                  switch (cmd.GetCommand())
                  {
                     case LayoutStatusCmd.EStatCmd.CENTER:
                        cmd.center_x = Convert.ToSingle(cmd.center_x + xOffset);
                        cmd.center_y = Convert.ToSingle(cmd.center_y + yOffset);
                        break;
                  }
               }
            }
         }
      }

      public void Rename(String newName)
      {
         mLayoutData.ObjectByName.Remove(mName);
         mName = newName;
         mLayoutData.ObjectByName.Add(mName, this);
      }

      public void Draw(Graphics g)
      {
         Draw(g, new RectangleF(0, 0, 0, 0));
      }
      private void Draw(Graphics g, RectangleF inParentWorld)
      {
         if (mVisible)
         {
            RectangleF r = inParentWorld;
            LayoutStatusState currentStatusState = GetCurrentStatusState();
            if (currentStatusState != null)
            {
               Color col = Color.White;
               if (mStatuses.Count > 0)
               {
                  r = currentStatusState.GetBounds(inParentWorld.Location);
                  col = Color.FromArgb(255, currentStatusState.GetAverageColor());
               }

               Pen pen = new Pen(col);
               Brush brush = SystemBrushes.HighlightText;
               Brush fillBrush = new System.Drawing.SolidBrush(col);

               switch (mSpriteType)
               {
               case SpriteType.Empty:
                  break;

               case SpriteType.Point:
                  break;

               case SpriteType.Line:
               case SpriteType.LineStrip:
                  {
                     List<PointF> vertexes = currentStatusState.GetVertices(inParentWorld.Location);
                     for (Int32 ii = 0; ii < vertexes.Count - 1; ++ii)
                     {
                        g.DrawLine(pen, vertexes[ii], vertexes[ii + 1]);
                     }
                  }
                  break;

               case SpriteType.Box:
                  g.FillRectangle(fillBrush, r.X, r.Y, r.Width, r.Height);
                  break;

               case SpriteType.Sprite:
                  try
                  {
                     Paloma.TargaImage tim = currentStatusState.GetTexture();

                     if (tim != null)
                     {
                        RectangleF uvCoords = currentStatusState.GetUVCoordinates();
                        uvCoords.X *= tim.Header.Width;
                        uvCoords.Y *= tim.Header.Height;
                        uvCoords.Width *= tim.Header.Width;
                        uvCoords.Height *= tim.Header.Height;
                        g.DrawImage(tim.Image, r, uvCoords, GraphicsUnit.Pixel);
                     }
                     else
                     {
                        g.DrawRectangle(pen, r.X, r.Y, r.Width, r.Height);
                     }
                  }
                  catch (System.Exception)
                  {
                  }
                  break;

               case SpriteType.Spin:
                  break;

               case SpriteType.Poly:
                  {
                     List<PointF> vertexes = currentStatusState.GetVertices(inParentWorld.Location);
                     g.DrawPolygon(pen, vertexes.ToArray());
                  }
                  break;

               case SpriteType.Triangle:
                  {
                     List<PointF> vertexes = currentStatusState.GetVertices(inParentWorld.Location);
                     g.DrawLine(pen, vertexes[0], vertexes[1]);
                     g.DrawLine(pen, vertexes[1], vertexes[2]);
                     g.DrawLine(pen, vertexes[2], vertexes[3]);
                     g.DrawLine(pen, vertexes[3], vertexes[0]);
                  }
                  break;
               }
            }

            foreach (LayoutObject child in mChildren)
            {
               child.Draw(g, r);
            }
         }
      }
      public void DrawSelected(Graphics g, LayoutObject inSelected)
      {
         DrawSelected(g, new RectangleF(0, 0, 0, 0), inSelected, inSelected == this);
      }
      private void DrawSelected(Graphics g, RectangleF inParentWorld, LayoutObject inSelected, Boolean inDrawSelected)
      {
         inDrawSelected = inDrawSelected || (inSelected == this);

         RectangleF bounds = GetWorldBounds();
         bounds.Inflate(1, 1);

         if (inDrawSelected)
         {
            g.DrawRectangle(Pens.Red, new Rectangle((Int32)bounds.X, (Int32)bounds.Y, (Int32)bounds.Width, (Int32)bounds.Height));
         }

         foreach (LayoutObject child in mChildren)
         {
            child.DrawSelected(g, bounds, inSelected, inDrawSelected);
         }
      }

      public void PopulateTreeView(TreeNodeCollection tree, PointF scale)
      {
         TreeNode addedNode = tree.Add(String.Format("{0} ({1}, strCode = 0x{2:X8})", mName, mSpriteType, strcode));
         addedNode.Name = mName;
         addedNode.Checked = mVisible;
         addedNode.ImageIndex = (int)mSpriteType;
         addedNode.SelectedImageIndex = (int)mSpriteType;
         addedNode.Tag = this;
         if (GetCurrentStatusState() != null)
         {
            // TODO: add a name for the status
            TreeNode statusNode = addedNode.Nodes.Add(String.Format("Status"));
            GetCurrentStatusState().PopulateTreeView(statusNode, scale);
         }
         foreach (LayoutObject child in mChildren)
         {
            child.PopulateTreeView(addedNode.Nodes, scale);
         }
      }
   }
}
