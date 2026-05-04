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
   /* アクション データ ヘッダー */
   public class LayoutActionHeader
   {
      public List<LayoutActionRef> mActions;
      LayoutData mLayoutData;

      public LayoutActionHeader(BinaryReader br, uint ofAct, LayoutData ld)
      {
         mLayoutData = ld;
         br.BaseStream.Seek(ofAct, SeekOrigin.Begin);
         LayoutUtil.LogStreamObject( br, this );
         uint unActionCount = br.ReadUInt32();

         mActions = new List<LayoutActionRef>();

         for (uint i = 0; i < unActionCount; ++i)
         {
            long fileOffset = br.BaseStream.Position;

            LayoutAction act = new LayoutAction(br, mLayoutData);
            LayoutActionRef actRef = new LayoutActionRef(act, ld);
            actRef.mFileOffset = fileOffset;
            if (ld.mStringIDsFromCode.ContainsKey((uint)act.siStrCode)
               && !ld.ActionByName.ContainsKey(ld.mStringIDsFromCode[(uint)act.siStrCode]))
            {
               actRef.mName = ld.mStringIDsFromCode[(uint)act.siStrCode];
            }
            else
            {
               actRef.mName = LayoutUtil.WordLists.GetRandomVerb();
            }
            mActions.Add(actRef);

            // add the layout action to a table
            ld.ActionByName.Add(actRef.mName, actRef);
         }
      }
      public UInt32 Write(BinaryWriter bw)
      {
         UInt32 position = (UInt32)bw.BaseStream.Position;
         bw.Write(mActions.Count);

         foreach (LayoutActionRef actionRef in mActions)
         {
            actionRef.Write(bw);
         }

         return position;
      }

      public void ToXML(XmlElement parEl, bool newStrCodes)
      {
         XmlElement element = parEl.OwnerDocument.CreateElement("Actions");
         parEl.AppendChild(element);
         foreach (LayoutActionRef actionRef in mActions)
         {
            actionRef.ToXML(element, newStrCodes);
         }
      }

      public LayoutActionHeader(XmlDocument doc, LayoutData ld)
      {
         mLayoutData = ld;
         mActions = new List<LayoutActionRef>();
         foreach (XmlElement child in doc.SelectNodes("LayoutData/Actions/Action"))
         {
            mActions.Add(new LayoutActionRef(child, ld));
         }
      }

      public void PopulateTreeView(TreeView inTreeView, LayoutObject inSelectedObject, PointF scale)
      {
         inTreeView.BeginUpdate();
         List<TreeNode> nodes = new List<TreeNode>();
         foreach (LayoutActionRef actionRef in mActions)
         {
            TreeNode addedActionNode = new TreeNode(actionRef.mName);
            addedActionNode.Tag = actionRef;
            addedActionNode.ToolTipText = String.Format("{0:x8}", actionRef.mAction.siStrCode);
            LayoutAction action = actionRef.mAction;
            action.PopulateTreeView(addedActionNode, scale);
            nodes.Add(addedActionNode);
         }
         inTreeView.Nodes.Clear();
         inTreeView.Nodes.AddRange(nodes.ToArray());
         inTreeView.EndUpdate();
      }
   }

   public class LayoutActionRef
   {
      LayoutData mLayoutData;
      public LayoutActionRef(LayoutAction action, LayoutData ld)
      {
         mLayoutData = ld;
         mAction = action;
         mFileOffset = 0;
         mName = "";
      }

      public void ToXML(XmlElement parEl, bool newStrCodes)
      {
         XmlElement element = parEl.OwnerDocument.CreateElement("Action");
         parEl.AppendChild(element);
         element.SetAttribute("name", mName);
         if (!newStrCodes)
         {
            element.SetAttribute("strCode", mAction.siStrCode.ToString("x8"));
         }
         mAction.ToXML(element);
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

      public LayoutActionRef(XmlElement element, LayoutData ld)
      {
         mLayoutData = ld;
         mName = element.GetAttribute("name");
         UInt32 strCode = GV_StrCode(mName);
         if (element.HasAttribute("strCode"))
         {
            strCode = UInt32.Parse(element.GetAttribute("strCode"), System.Globalization.NumberStyles.HexNumber);
         }
         mAction = new LayoutAction(element, strCode, ld);

         mLayoutData.ActionByName.Add(mName, this);
      }

      public void Write(BinaryWriter bw)
      {
         mAction.Write(bw);
      }

      public void Rename(string newName)
      {
         mLayoutData.ActionByName.Remove(mName);
         mName = newName;
         mLayoutData.ActionByName.Add(mName, this);
      }

      public LayoutAction mAction;
      public long mFileOffset;
      public string mName;
   }

   /* アクション データ */
   public class LayoutAction
   {
      LayoutData mLayoutData;

      public Int32 siStrCode;
      public Int32 siNumTracks;

      public List<LayoutKeyHeader> mObjectKeys;

      public LayoutAction(BinaryReader br, LayoutData ld)
      {
         mLayoutData = ld;
         LayoutUtil.LogStreamObject( br, this );
         siStrCode = br.ReadInt32();
         siNumTracks = br.ReadInt32();

         LayoutUtil.LogStreamAddl( "strCode", siStrCode, "count", siNumTracks );

         mObjectKeys = new List<LayoutKeyHeader>();
         for (uint i = 0; i < siNumTracks; ++i)
         {
            mObjectKeys.Add(new LayoutKeyHeader(br, mLayoutData));
         }
      }
      public void Write(BinaryWriter bw)
      {
         bw.Write(siStrCode);
         bw.Write(mObjectKeys.Count);

         foreach (LayoutKeyHeader keyHeader in mObjectKeys)
         {
            keyHeader.Write(bw);
         }
      }

      public void ToXML(XmlElement parEl)
      {
         foreach (LayoutKeyHeader keyHeader in mObjectKeys)
         {
            keyHeader.ToXML(parEl);
         }
      }

      public LayoutAction(XmlElement element, UInt32 strCode, LayoutData ld)
      {
         mObjectKeys = new List<LayoutKeyHeader>();
         siStrCode = (int) strCode;

         foreach (XmlElement child in element.ChildNodes)
         {
            mObjectKeys.Add(new LayoutKeyHeader(child, ld));
         }
      }

      public void PopulateTreeView(TreeNode actionNode, PointF scale)
      {
         foreach (LayoutKeyHeader keyHead in mObjectKeys)
         {
            keyHead.PopulateTreeView(actionNode, scale);
         }
      }
   }

   /* キーヘッダー */
   public class LayoutKeyHeader
   {
      LayoutData mLayoutData;

      public UInt16 usId;
      UInt16 usStepNums;

      public List<LayoutKey> mKeys;

      public LayoutKeyHeader(BinaryReader br, LayoutData ld)
      {
         mLayoutData = ld;

         usId = br.ReadUInt16();
         usStepNums = br.ReadUInt16();

         mKeys = new List<LayoutKey>();
         while (true)
         {
            LayoutKey key = new LayoutKey(br);
            mKeys.Add(key);
            if (key.GetKeyType() == LayoutKey.EKeyType.End)
            {
               break;
            }
         }
      }
      public void Write(BinaryWriter bw)
      {
         bw.Write(usId);
         bw.Write((Int16)mKeys.Count);

         foreach (LayoutKey key in mKeys)
         {
            key.Write(bw);
         }
      }
      public void ToXML(XmlElement parEl)
      {
         XmlElement element = parEl.OwnerDocument.CreateElement("ObjectAction");
         parEl.AppendChild(element);
         LayoutObject obj = null;
         if (usId != 0xffff)
         {
            obj = mLayoutData.ObjectByID[usId];
            element.SetAttribute("object", obj.mName);
         }
         foreach (LayoutKey key in mKeys)
         {
            key.ToXML(element, obj);
         }
      }

      public LayoutKeyHeader(XmlElement element, LayoutData ld)
      {
         mLayoutData = ld;
         usId = 0xffff;
         mKeys = new List<LayoutKey>();
         if (element.HasAttribute("object"))
         {
            string objName = element.GetAttribute("object");
            if (!String.IsNullOrEmpty(objName) && ld.ObjectByName.ContainsKey(objName))
            {
               LayoutObject obj = ld.ObjectByName[objName];
               usId = obj.usID;
            }
         }

         foreach (XmlElement child in element.ChildNodes)
         {
            mKeys.Add(new LayoutKey(child));
         }
      }

      public void PopulateTreeView(TreeNode actionNode, PointF scale)
      {
         TreeNodeCollection nodes = actionNode.Nodes;
         TreeNode addedNode = null;

         LayoutObject obj = null;
         if (usId != 0xffff)
         {
            obj = mLayoutData.ObjectByID[usId];
            addedNode = nodes.Add(obj.mName);
            addedNode.ImageIndex = (int)obj.mSpriteType;
            addedNode.SelectedImageIndex = (int)obj.mSpriteType;
         }
         else
         {
            addedNode = nodes.Add("Anonymous");
         }

         addedNode.Tag = this;
         foreach (LayoutKey key in mKeys)
         {
            key.PopulateTreeView(addedNode, obj, scale);
         }
      }
   }

   /* キーデータ */
   public class LayoutKey
   {
      public enum EKeyType
      {
         End      = 0x0,
         Signal   = 0x1,
         Wait     = 0x4,
         Set      = 0x8,
         Morph    = 0xc
      };
      public const UInt32 kLK_Mask     = 0xf0000000;

      public UInt32 uiCmd;
      public UInt32 uiStat;

      public EKeyType GetKeyType()
      {
         return (EKeyType)((uiCmd & kLK_Mask)>>28);
      }

      public LayoutKey(BinaryReader br)
      {
         uiCmd = br.ReadUInt32();
         switch (GetKeyType())
         {
         case EKeyType.Morph:
         case EKeyType.Signal:
            uiStat = br.ReadUInt32();
            break;
         }
      }
      public void Write(BinaryWriter bw)
      {
         bw.Write(uiCmd);
         switch (GetKeyType())
         {
         case EKeyType.Morph:
         case EKeyType.Signal:
            bw.Write(uiStat);
            break;
         }
      }

      public void ToXML(XmlElement parEl, LayoutObject target)
      {
         XmlElement element = parEl.OwnerDocument.CreateElement("Key");
         parEl.AppendChild(element);
         element.SetAttribute("type", GetKeyType().ToString());
         switch (GetKeyType())
         {
         case EKeyType.End:
            break;
         case EKeyType.Signal:
            element.SetAttribute("code", (uiCmd & 0xffffff).ToString("x8"));
            element.SetAttribute("param", uiStat.ToString("x8"));
            break;
         case EKeyType.Wait:
            element.SetAttribute("time", (uiCmd & 0xffff).ToString());
            break;
         case EKeyType.Set:
            if (target != null)
            {
               foreach (LayoutStatusRef status in target.mStatuses)
               {
                  foreach (LayoutStatusState statusState in status.mStatus.mStates)
                  {
                     if ((uiCmd & 0x00ffffff) == statusState.mCode)
                     {
                        element.SetAttribute("status", status.mName);
                        break;
                     }
                  }
               }
            }
            element.SetAttribute("state", (uiCmd & 0x00ffffff).ToString("x8"));
            break;
         case EKeyType.Morph:
            element.SetAttribute("time", (uiCmd & 0xffff).ToString());
            if (target != null)
            {
               foreach (LayoutStatusRef status in target.mStatuses)
               {
                  foreach (LayoutStatusState statusState in status.mStatus.mStates)
                  {
                     if ((uiStat & 0x00ffffff) == statusState.mCode)
                     {
                        element.SetAttribute("status", status.mName);
                        break;
                     }
                  }
               }
            }
            element.SetAttribute("state", (uiStat & 0x00ffffff).ToString("x8"));
            break;
         }
      }

      public LayoutKey(XmlElement element)
      {
         EKeyType type = (EKeyType) Enum.Parse(typeof(EKeyType), element.GetAttribute("type"));
         uiCmd = ((UInt32) (type)) << 28;

         switch (GetKeyType())
         {
            case EKeyType.End:
               break;
            case EKeyType.Signal:
               {
                  UInt32 code = UInt32.Parse(element.GetAttribute("code"), System.Globalization.NumberStyles.HexNumber);
                  uiCmd |= code & 0x00ffffff;
                  uiStat = UInt32.Parse(element.GetAttribute("param"), System.Globalization.NumberStyles.HexNumber);
               }
               break;
            case EKeyType.Wait:
               {
                  UInt32 time = UInt32.Parse(element.GetAttribute("time"));
                  uiCmd |= (time & 0xffff);
               }
               break;
            case EKeyType.Set:
               {
                  UInt32 code = UInt32.Parse(element.GetAttribute("state"), System.Globalization.NumberStyles.HexNumber);
                  uiCmd |= (code & 0x00ffffff);
               }
               break;
            case EKeyType.Morph:
               {
                  UInt32 code = UInt32.Parse(element.GetAttribute("state"), System.Globalization.NumberStyles.HexNumber);
                  uiStat = code & 0x00ffffff;
                  UInt32 time = UInt32.Parse(element.GetAttribute("time"));
                  uiCmd |= (time & 0xffff);
               }
               break;
         }
      }

      public void PopulateTreeView(TreeNode actionNode, LayoutObject target, PointF scale)
      {
         switch (GetKeyType())
         {
         case EKeyType.End:     
            actionNode.Nodes.Add("End");        
            break;

         case EKeyType.Signal:
            actionNode.Nodes.Add(String.Format("Signal {0:x8}({1})", uiCmd & 0x00ffffff, uiStat));
            break;

         case EKeyType.Wait:    
            actionNode.Nodes.Add(String.Format("Wait Time={0}", uiCmd & 0xffff)); 
            break;

         case EKeyType.Set:
            if (target != null)
            {
               foreach (LayoutStatusRef status in target.mStatuses)
               {
                  foreach (LayoutStatusState statusState in status.mStatus.mStates)
                  {
                     if ((uiCmd & 0x00ffffff) == statusState.mCode)
                     {
                        TreeNode addedNode = actionNode.Nodes.Add(String.Format("Set {0}/{1:x8}", status.mName, statusState.mCode));
                        addedNode.Tag = status;
                        statusState.PopulateTreeView(addedNode, scale);
                        return;
                     }
                  }
               }
            }
            else
            {
               actionNode.Nodes.Add(String.Format("Set {0:x8}", uiCmd & 0x00ffffff));
            }
            break;

         case EKeyType.Morph:
            if (target != null)
            {
               foreach (LayoutStatusRef status in target.mStatuses)
               {
                  foreach (LayoutStatusState statusState in status.mStatus.mStates)
                  {
                     if ((uiStat & 0x00ffffff) == statusState.mCode)
                     {
                        TreeNode addedNode = actionNode.Nodes.Add(String.Format("Morph {0}/{1:x8} Time={2}", status.mName, statusState.mCode, uiCmd & 0xffff));
                        addedNode.Tag = status;
                        statusState.PopulateTreeView(addedNode, scale);
                        return;
                     }
                  }
               }
            }
            else
            {
               actionNode.Nodes.Add(String.Format("Morph {0:x8} Time={1}", uiStat & 0x00ffffff, uiCmd & 0xffff));
            }
            break;
         }
      }
   }
}
