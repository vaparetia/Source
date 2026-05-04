using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Xml;
using System.Runtime.InteropServices;
using System.Diagnostics;
using LayoutExtract2;

namespace LayoutEditorMGS2
{
   public partial class LayoutEditorWindow : Form
   {
      #region Private Fields
      const Double kSDGridHeight = 400.0;
      const Double kSDGridWidth = 512.0;
      const Int32 kNudgeAmount = 1;

      Stack<Byte[]> mUndoStack = new Stack<Byte[]>();
      Stack<Byte[]> mRedoStack = new Stack<Byte[]>();

      private bool mIsHoldingDownArrow = false;
      private LayoutObject mOverrideSelectObject = null;
      private LayoutData mCurrentLayoutData;
      private String mCurrentFilename;
      private FindDialog mFindDialog;

      public enum ECopiedType
      {
         kNothing,
         kXCoord,
         kSize
      };

      private ECopiedType mCopiedType = ECopiedType.kNothing;
      private float mCopiedX;
      private float mCopiedW;
      private float mCopiedH;

      public Double mPreviewWidth = kSDGridWidth;
      public Double mPreviewHeight = kSDGridHeight;
      public bool mFitToScreenHeight = false;
      #endregion

      public LayoutEditorWindow(String loadFile)
      {
         InitializeComponent();

         ImageList list = new ImageList();
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.empty);        // Empty,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.empty);        // Point,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.line);         // Line,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.linestrip);    // LineStrip,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.box);          // Box,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.sprite);       // Sprite,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.empty);        // Zoom,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.empty);        // Spin,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.tristrip);     // Poly,
         list.Images.Add(LayoutEditorMGS2.Properties.Resources.triangle);     // Triangle

         objectTreeView.ImageList = list;
         actionTreeView.ImageList = list;

         objectTreeView.BeforeSelect += new TreeViewCancelEventHandler(objectTreeView_BeforeSelect);
         actionTreeView.BeforeSelect += new TreeViewCancelEventHandler(actionTreeView_BeforeSelect);
         actionTreeView.AfterSelect += new TreeViewEventHandler(actionTreeView_AfterSelect);

         if (loadFile != null)
         {
            LoadFile(Path.GetFullPath(loadFile));
         }
      }

      public PointF GetScale()
      {
         Double scaleX = mPreviewWidth/mCurrentLayoutData.xsize;
         Double scaleY = mPreviewHeight/mCurrentLayoutData.ysize;
         if (mFitToScreenHeight)
         {
            scaleY *= 400.0f/384.0f;
         }
         return new PointF((float)scaleX, (float)scaleY);
      }

      public RectangleF ConvertToPreviewCoords(RectangleF rect)
      {
         PointF scale = GetScale();
         return new RectangleF(rect.X * scale.X, rect.Y * scale.Y, rect.Width * scale.X, rect.Height * scale.Y);
      }

      public RectangleF ConvertFromPreviewCoords(RectangleF rect)
      {
         PointF scale = GetScale();
         return new RectangleF(rect.X / scale.X, rect.Y / scale.Y, rect.Width / scale.X, rect.Height / scale.Y);
      }

      public PointF ConvertToPreviewCoords(PointF point)
      {
         PointF scale = GetScale();
         return new PointF(point.X * scale.X, point.Y * scale.Y);
      }

      public PointF ConvertFromPreviewCoords(PointF point)
      {
         PointF scale = GetScale();
         return new PointF(point.X / scale.X, point.Y / scale.Y);
      }

      public void PreserveForUndo()
      {
         Byte[] doc = SaveStateAsByteArray();
         mUndoStack.Push(doc);
      }
      public void Undo()
      {
         if (mCurrentLayoutData == null)
            return;
         if (mUndoStack.Count == 0)
            return;

         try
         {
            LayoutData oldLayoutData = mCurrentLayoutData;
            Byte[] oldDoc = SaveStateAsByteArray();

            LayoutData newLayoutData = new LayoutData();
            Byte[] newDoc = mUndoStack.Peek();
            LayoutUtil.RecreateWordLists(new System.Random(666));
            LayoutData.ReadLayoutData(newLayoutData, newDoc);

            mCurrentLayoutData = newLayoutData;
            mUndoStack.Pop();
            mRedoStack.Push(oldDoc);
         }
         catch (System.Exception)
         {
            MessageBox.Show("Error undoing the operation! Cannot Undo!", "Undo Error!");
         }

         RefreshWindowWithTreeLock();
      }
      public void Redo()
      {
         if (mCurrentLayoutData == null)
            return;
         if (mRedoStack.Count == 0)
            return;

         try
         {
            LayoutData oldLayoutData = mCurrentLayoutData;
            Byte[] oldDoc = SaveStateAsByteArray();

            LayoutData newLayoutData = new LayoutData();
            Byte[] newDoc = mRedoStack.Peek();
            LayoutUtil.RecreateWordLists(new System.Random(666));
            LayoutData.ReadLayoutData(newLayoutData, newDoc);

            mCurrentLayoutData = newLayoutData;
            mRedoStack.Pop();
            mUndoStack.Push(oldDoc);
         }
         catch (System.Exception)
         {
            MessageBox.Show("Error redoing the operation! Cannot Redo!", "Redo Error!");
         }

         RefreshWindowWithTreeLock();
      }
      public void Find(String inFind)
      {
         if (String.IsNullOrEmpty(inFind))
            return;
         if (actionTreeView.Nodes.Count == 0)
            return;

         TreeNode actionNode = actionTreeView.SelectedNode;
         if (actionNode == null)
            actionNode = actionTreeView.Nodes[0];
         TreeNode firstNode = actionNode;

         do
         {
            TreeNode nextNode = NextSearchNode(actionNode);
            if (nextNode == null)
               nextNode = actionTreeView.Nodes[0];
            actionNode = nextNode;

            if (actionNode == firstNode)
               break;
            if (nextNode.Text == inFind)
            {
               actionTreeView.SelectedNode = actionNode;
               actionNode.EnsureVisible();
               return;
            }
         } while (actionNode != firstNode);

         MessageBox.Show("Finished Search. No more results found.");
      }
      private TreeNode NextSearchNode(TreeNode inNode)
      {
         if (inNode.Nodes.Count > 0)
            return inNode.Nodes[0];
         if (inNode.NextNode == null)
         {
            while (inNode.Parent != null)
            {
               if (inNode.Parent.NextNode != null)
                  return inNode.Parent.NextNode;
               inNode = inNode.Parent;
            }
            return actionTreeView.Nodes[0];
         }
         return inNode.NextNode;
      }
      private LayoutObject GetSelectedLayoutObject()
      {
         TreeNode selectedNode = objectTreeView.SelectedNode;
         while (selectedNode != null && (selectedNode.Tag == null || selectedNode.Tag.GetType() != typeof(LayoutObject)))
            selectedNode = selectedNode.Parent;
         if (selectedNode == null)
            return null;
         return (LayoutObject)selectedNode.Tag;
      }
      public void RefreshWindow()
      {
         if (mCurrentLayoutData != null)
         {
            TreeViewState objectTreeState = new TreeViewState(objectTreeView);
            PointF scale = GetScale();
            mCurrentLayoutData.mObjectHeader.PopulateTreeView(objectTreeView, scale);
            objectTreeState.Restore(objectTreeView);
         }
         previewPictureBox.Invalidate();
         UpdateActionView();
      }

      #region Private Methods
      private Byte[] SaveStateAsByteArray()
      {
         if (mCurrentLayoutData == null)
            return null;

         MemoryStream memStream = new MemoryStream();
         BinaryWriter binWriter = new BinaryWriter(memStream);

         mCurrentLayoutData.Write(binWriter);
         binWriter.Flush();

         Byte[] doc = memStream.ToArray();
         return doc;
      }
      private void LoadFile(string fileName)
      {
         mRedoStack.Clear();
         mUndoStack.Clear();

         mOverrideSelectObject = null;
         mCurrentLayoutData = new LayoutData();
         mCurrentFilename = fileName;

         MRU.AddMRU("LayoutEditorMGS2", fileName, 16);

         // Use a consistent random seed so files loaded are the same

         if (System.IO.Path.GetExtension(fileName) == ".o2d")
         {
            mCurrentLayoutData.mStringIDsFromCode = StringIdGatherer.LoadStringIds();

            // read stream
            FileStream fs = File.OpenRead(fileName);
            byte[] buf = new byte[fs.Length];
            fs.Read(buf, 0, buf.Length);

            // make data
            MemoryStream ms = new MemoryStream(buf);
            BinaryReader br = new BinaryReader(ms);
            LayoutUtil.RecreateWordLists(new System.Random(666));
            LayoutData.ReadLayoutData(mCurrentLayoutData, br);

            fs.Close();
            br.Close();
         }
         else if (System.IO.Path.GetExtension(fileName) == ".xml")
         {
            XmlDocument doc = new XmlDocument();
            doc.Load(fileName);

            mCurrentLayoutData = new LayoutData(doc);
            // don't think we need this here...
            mCurrentLayoutData.mStringIDsFromCode = StringIdGatherer.LoadStringIds();

            // remove the xml extension
            mCurrentFilename = fileName.Substring(0, fileName.Length - 4);
         }

         this.Text = String.Format("LayoutEditorMGS2 - {0}", fileName);

         PointF scale = GetScale();
         mCurrentLayoutData.mObjectHeader.PopulateTreeView(objectTreeView, scale);
         UpdateActionView();

         previewPictureBox.Invalidate();

         toolStripStatusLabel1.Text = String.Format("Loaded {0}", fileName);
      }
      private void UpdateActionView()
      {
         if (mCurrentLayoutData == null)
            return;

         LayoutObject obj = null;
         if (objectTreeView.SelectedNode != null
            && objectTreeView.SelectedNode.Tag != null
            && objectTreeView.SelectedNode.Tag.GetType() == typeof(LayoutObject))
         {
            obj = (LayoutObject)objectTreeView.SelectedNode.Tag;
         }
         PointF scale = GetScale();
         mCurrentLayoutData.mActionHeader.PopulateTreeView(actionTreeView, obj, scale);
      }
      private void UpgradeLayoutStatusToWidescreen(LayoutStatus inStatus, Boolean inParentIsAtOrigin, Boolean inExpandToCover)
      {
         foreach (LayoutStatusState stat in inStatus.mStates)
         {
            foreach (LayoutStatusCmd cmd in stat.mCommands)
            {
               switch (cmd.GetCommand())
               {
                  case LayoutStatusCmd.EStatCmd.VERTEX:
                     foreach (LayoutStatusCmd.Vertex vert in cmd.vertexes)
                     {
                        Double newLocationX = ((Double)vert.x / mCurrentLayoutData.xsize) * ((inExpandToCover) ? 1.0 : 0.75);
                        Double newLocationY = ((Double)vert.y / mCurrentLayoutData.ysize);
                        if (inParentIsAtOrigin)
                           newLocationX += 0.125;
                        if (inExpandToCover)
                           newLocationX -= 0.125;
                        vert.x = Convert.ToSingle(newLocationX * mCurrentLayoutData.xsize);
                        vert.y = Convert.ToSingle(newLocationY * mCurrentLayoutData.ysize);
                     }
                     break;
                  case LayoutStatusCmd.EStatCmd.SIZE:
                     {
                        Double newSizeW = ((Double)cmd.size_w / mCurrentLayoutData.xsize) * ((inExpandToCover) ? 1.0 : 0.75);
                        Double newSizeH = ((Double)cmd.size_h / mCurrentLayoutData.ysize);
                        cmd.size_w = Convert.ToSingle(newSizeW * mCurrentLayoutData.xsize);
                        cmd.size_h = Convert.ToSingle(newSizeH * mCurrentLayoutData.ysize);
                     }
                     break;
                  case LayoutStatusCmd.EStatCmd.CENTER:
                     {
                        Double newLocationX = ((Double)cmd.center_x / mCurrentLayoutData.xsize) * ((inExpandToCover) ? 1.0 : 0.75);
                        Double newLocationY = ((Double)cmd.center_y / mCurrentLayoutData.ysize);
                        if (inParentIsAtOrigin)
                           newLocationX += 0.125;
                        if (inExpandToCover)
                           newLocationX -= 0.125;
                        cmd.center_x = Convert.ToSingle(newLocationX * mCurrentLayoutData.xsize);
                        cmd.center_y = Convert.ToSingle(newLocationY * mCurrentLayoutData.ysize);
                     }
                     break;
               }
            }
         }
      }
      private void ResizeObjectsForWideScreen(LayoutData inLayoutData)
      {
         foreach (LayoutObject obj in inLayoutData.ObjectByID.Values)
         {
            ResizeObjectForWideScreen(inLayoutData, obj);
         }
         inLayoutData.SetIsFormattedForHD(true);
      }

      private void ResizeObjectForWideScreen(LayoutData inLayoutData, LayoutObject inLayoutObject)
      {
         if (inLayoutData == null)
            return;
         if (inLayoutData.widescreen != 0)
            return;

         RectangleF rect = inLayoutObject.GetWorldBounds();
         Boolean coversScreen = (rect.X == 0 && rect.Width == inLayoutData.xsize);

         // Resize Objects
         List<LayoutStatus> updatedStatuses = new List<LayoutStatus>();
         foreach (LayoutStatusRef statusRef in inLayoutObject.mStatuses)
         {
            UpgradeLayoutStatusToWidescreen(statusRef.mStatus, inLayoutData.mObjectHeader.mRootObjects.Contains(inLayoutObject), coversScreen);
            updatedStatuses.Add(statusRef.mStatus);
         }
      }
      private void ExpandObjectToWidth(LayoutData inLayoutData, LayoutObject inLayoutObject)
      {
         RectangleF currWorldRect = inLayoutObject.GetWorldBounds();

         Double currSizeX = (Double)currWorldRect.Width / inLayoutData.xsize;
         Double currLocationX = (Double)currWorldRect.X / inLayoutData.xsize;

         // Figure out old SD location
         Double sdSizeX = currSizeX / 0.75;
         Double sdLocationX = (currLocationX - 0.125) / 0.75;
         Double newLocationX = sdLocationX * 0.75;

         Double sdRightX = sdLocationX + sdSizeX;

         Double sdRightBuffer = 1.0 - sdRightX;
         sdRightBuffer *= 0.75;
         Double newLocationRightX = 1.0 - sdRightBuffer;
         Double newWidth = newLocationRightX - newLocationX;

         Single widthOffset = Convert.ToSingle((newWidth - currSizeX) * inLayoutData.xsize);
         Single xOffset = Convert.ToSingle((newLocationX - currLocationX) * inLayoutData.xsize);

         inLayoutObject.OffsetBounds(xOffset, 0, widthOffset, 0);
      }
      public void OffsetLayoutStatus(LayoutStatus inLayoutStatus, Single inXOffset, Single inYOffset, Single inWidthOffset)
      {
         foreach (LayoutStatusState stat in inLayoutStatus.mStates)
         {
            foreach (LayoutStatusCmd cmd in stat.mCommands)
            {
               switch (cmd.GetCommand())
               {
                  case LayoutStatusCmd.EStatCmd.VERTEX:
                     foreach (LayoutStatusCmd.Vertex vert in cmd.vertexes)
                     {
                        vert.x = Convert.ToSingle(vert.x + inXOffset);
                        vert.y = Convert.ToSingle(vert.y + inYOffset);
                     }
                     break;
                  case LayoutStatusCmd.EStatCmd.SIZE:
                     cmd.size_w = Convert.ToSingle(cmd.size_w + inWidthOffset);
                     break;
                  case LayoutStatusCmd.EStatCmd.CENTER:
                     cmd.center_x = Convert.ToSingle(cmd.center_x + inXOffset);
                     cmd.center_y = Convert.ToSingle(cmd.center_y + inYOffset);
                     break;
               }
            }
         }
      }
      private void TogglePixelPerfect(LayoutStatus inLayoutStatus)
      {
         foreach (LayoutStatusState stat in inLayoutStatus.mStates)
         {
            foreach (LayoutStatusCmd cmd in stat.mCommands)
            {
               if (cmd.GetCommand() == LayoutStatusCmd.EStatCmd.DISP)
               {
                  cmd.pixel_perfect = !cmd.pixel_perfect;
               }
            }
         }
      }
      private void PushElementToLeftForWideScreen(LayoutData inLayoutData, LayoutObject inLayoutObject)
      {
         RectangleF currWorldRect = inLayoutObject.GetWorldBounds();
         Double currLocationX = (Double)currWorldRect.X / inLayoutData.xsize;

         // Figure out old SD location
         Double sdLocationX = (currLocationX - 0.125) / 0.75;
         Double newLocationX = sdLocationX * 0.75;
         Single xOffset = Convert.ToSingle((newLocationX - currLocationX) * inLayoutData.xsize);

         List<LayoutStatus> updatedStatuses = new List<LayoutStatus>();

         foreach (LayoutStatusRef statusRef in inLayoutObject.mStatuses)
         {
            OffsetLayoutStatus(statusRef.mStatus, xOffset, 0, 0);
            updatedStatuses.Add(statusRef.mStatus);
         }

      }
      private void PushElementToRightForWideScreen(LayoutData inLayoutData, LayoutObject inLayoutObject)
      {
         RectangleF currWorldRect = inLayoutObject.GetWorldBounds();

         Double currSizeX = (Double)currWorldRect.Width / inLayoutData.xsize;
         Double currLocationX = (Double)currWorldRect.X / inLayoutData.xsize;

         // Figure out old SD location
         Double sdSizeX = currSizeX / 0.75;
         Double sdLocationX = (currLocationX - 0.125) / 0.75;
         Double sdRightX = sdLocationX + sdSizeX;

         Double sdRightBuffer = 1.0 - sdRightX;
         sdRightBuffer *= 0.75;
         Double newLocationX = 1.0 - sdRightBuffer - currSizeX;
         Single xOffset = Convert.ToSingle((newLocationX - currLocationX) * inLayoutData.xsize);

         List<LayoutStatus> updatedStatuses = new List<LayoutStatus>();

         foreach (LayoutStatusRef statusRef in inLayoutObject.mStatuses)
         {
            OffsetLayoutStatus(statusRef.mStatus, xOffset, 0, 0);
            updatedStatuses.Add(statusRef.mStatus);
         }

      }
      private void ScaleElements(LayoutData inLayoutData, Int16 inHeightOffset)
      {
         if (objectTreeView.SelectedNode == null)
            return;
         if (objectTreeView.SelectedNode.Tag == null)
            return;
         if (objectTreeView.SelectedNode.Tag.GetType() != typeof(LayoutObject))
            return;

         objectTreeView.BeginUpdate();

         if (!mIsHoldingDownArrow)
         {
            PreserveForUndo();
         }
         LayoutObject selectedObj = (LayoutObject)objectTreeView.SelectedNode.Tag;

         List<LayoutObject> allLayoutObjects = new List<LayoutObject>();
         List<LayoutObject> sizableLayoutObjects = new List<LayoutObject>();
         GetAllNodes(selectedObj, ref allLayoutObjects);

         RectangleF sizableBounds = RectangleF.Empty;
         foreach (LayoutObject obj in allLayoutObjects)
         {
            RectangleF rect = obj.GetWorldBounds();
            if (rect.Width > 0 && rect.Height > 0)
            {
               sizableLayoutObjects.Add(obj);
               if (sizableBounds.IsEmpty)
               {
                  sizableBounds = rect;
               }
               else
               {
                  sizableBounds = RectangleF.Union(sizableBounds, rect);
               }
            }
         }

         RectangleF newBounds = sizableBounds;
         newBounds.Height += inHeightOffset;
         newBounds.Width = (sizableBounds.Width / sizableBounds.Height) * newBounds.Height;

         List<LayoutStatus> updatedStatuses = new List<LayoutStatus>();
         foreach (LayoutObject obj in allLayoutObjects)
         {
            PointF parentPoint = PointF.Empty;

            LayoutObject parentObj = obj.GetParent();
            if (parentObj != null)
            {
               parentPoint = parentObj.GetWorldBounds().Location;
            }

            foreach (LayoutStatusRef statusRef in selectedObj.mStatuses)
            {
               statusRef.mStatus.ResizeToRect(parentPoint, sizableBounds, newBounds);
               updatedStatuses.Add(statusRef.mStatus);
            }
         }

         RefreshWindow();

         objectTreeView.EndUpdate();
      }
      private RectangleF ConvertUIRectToUnitized(LayoutData inLayoutData, RectangleF inUIRect)
      {
         if (inLayoutData == null)
            return inUIRect;
         Double gridWidth = inLayoutData.xsize;
         Double gridHeight = inLayoutData.ysize;

         RectangleF objRect = inUIRect;

         Double sizeX = (Double)objRect.Width / gridWidth;
         Double sizeY = (Double)objRect.Height / gridHeight;
         Double locationX = (Double)objRect.X / gridWidth;
         Double locationY = (Double)objRect.Y / gridHeight;

         return new RectangleF((Single)locationX, (Single)locationY, (Single)sizeX, (Single)sizeY);
      }

      private void GetAllNodes(LayoutObject inParentObject, ref List<LayoutObject> outObjects)
      {
         outObjects.Add(inParentObject);
         foreach (LayoutObject obj in inParentObject.mChildren)
            GetAllNodes(obj, ref outObjects);
      }
      private TreeNode GetNodeForLayoutItem(LayoutObject inObj, TreeNode inParent)
      {
         if (inObj == null)
            return null;
         if (mCurrentLayoutData == null)
            return null;
         if (!mCurrentLayoutData.ObjectByID.ContainsValue(inObj))
            inObj = mCurrentLayoutData.ObjectByID[inObj.usID];

         if (inParent != null)
            if (inParent.Tag == inObj)
               return inParent;

         TreeNodeCollection childNodes = (inParent == null) ? objectTreeView.Nodes : inParent.Nodes;
         foreach (TreeNode node in childNodes)
         {
            TreeNode foundNode = GetNodeForLayoutItem(inObj, node);
            if (foundNode != null)
               return foundNode;
         }

         return null;
      }
      private void EditSelectedNodeData()
      {
         TreeNode node = objectTreeView.SelectedNode;
         if (node != null && node.Tag != null)
         {
            if (node.Tag.GetType() == typeof(LayoutObject))
            {
               SPEditor propertyBox = new SPEditor(this);
               propertyBox.Initialize((LayoutObject)node.Tag);
               propertyBox.Show();
            }
            else if (node.Tag.GetType() == typeof(LayoutStatusState))
            {
               StatusEditor statusBox = new StatusEditor(this, (LayoutStatusState)node.Tag);
               statusBox.Show();
            }
         }
      }
      private void NudgeElement(LayoutData inLayoutData, Int32 inDeltaX, Int32 inDeltaY)
      {

         if (objectTreeView.SelectedNode == null)
            return;
         if (objectTreeView.SelectedNode.Tag == null)
            return;
         if (objectTreeView.SelectedNode.Tag.GetType() != typeof(LayoutObject))
            return;

         if (!mIsHoldingDownArrow)
            PreserveForUndo();

         PointF nudge = ConvertFromPreviewCoords(new PointF(inDeltaX, inDeltaY));

         LayoutObject inLayoutObject = (LayoutObject)objectTreeView.SelectedNode.Tag;

         foreach (LayoutStatusRef statusRef in inLayoutObject.mStatuses)
         {
            OffsetLayoutStatus(statusRef.mStatus, nudge.X, nudge.Y, 0);
         }

         // only update the image
         // it takes too long to update the tree (done when the arrow key is released)
         previewPictureBox.Invalidate();
      }

      private void TogglePixelPerfect(LayoutData inLayoutData)
      {
         if (objectTreeView.SelectedNode != null
            && objectTreeView.SelectedNode.Tag != null
            && objectTreeView.SelectedNode.Tag.GetType() == typeof(LayoutObject))
         {
            PreserveForUndo();
            LayoutObject inLayoutObject = (LayoutObject)objectTreeView.SelectedNode.Tag;
            foreach (LayoutStatusRef statusRef in inLayoutObject.mStatuses)
            {
               TogglePixelPerfect(statusRef.mStatus);
            }
            RefreshWindow();
         }
      }

      private void CopyCurrentX()
      {
         if (objectTreeView.SelectedNode != null
            && objectTreeView.SelectedNode.Tag != null
            && objectTreeView.SelectedNode.Tag.GetType() == typeof(LayoutObject))
         {
            LayoutObject layObj = (LayoutObject)objectTreeView.SelectedNode.Tag;
            RectangleF worldBounds = layObj.GetWorldBounds();
            mCopiedType = ECopiedType.kXCoord;
            mCopiedX = worldBounds.Left;
         }
      }

      private void CopyCurrentSize()
      {
         if (objectTreeView.SelectedNode != null
            && objectTreeView.SelectedNode.Tag != null
            && objectTreeView.SelectedNode.Tag.GetType() == typeof(LayoutObject))
         {
            LayoutObject layObj = (LayoutObject)objectTreeView.SelectedNode.Tag;
            RectangleF worldBounds = layObj.GetWorldBounds();
            mCopiedType = ECopiedType.kSize;
            mCopiedW = worldBounds.Width;
            mCopiedH = worldBounds.Height;
         }
      }

      private void PasteCurrentData()
      {
         if (mCopiedType != ECopiedType.kNothing
            && objectTreeView.SelectedNode != null
            && objectTreeView.SelectedNode.Tag != null
            && objectTreeView.SelectedNode.Tag.GetType() == typeof(LayoutObject))
         {
            LayoutObject layObj = (LayoutObject)objectTreeView.SelectedNode.Tag;
            RectangleF worldBounds = layObj.GetWorldBounds();
            PreserveForUndo();
            switch (mCopiedType)
            {
               case ECopiedType.kXCoord:
                  layObj.OffsetBounds(mCopiedX - worldBounds.Left, 0.0f, 0.0f, 0.0f);
                  break;
               case ECopiedType.kSize:
                  layObj.OffsetBounds(0.0f, 0.0f, mCopiedW - worldBounds.Width, mCopiedH - worldBounds.Height);
                  break;
            }
            RefreshWindow();
         }
      }

      private Boolean CompareBuffers(Byte[] inBuffer1, Byte[] inBuffer2, Int32 inComparisonLength)
      {
         if (inBuffer1.Length != inBuffer2.Length)
            return false;
         if (inBuffer1.Length < inComparisonLength)
            inComparisonLength = inBuffer1.Length;

         for (Int32 ii = 0; ii < inComparisonLength; ++ii)
            if (inBuffer1[ii] != inBuffer2[ii])
               return false;

         return true;
      }

      private void SetToolStrip()
      {
         String text = "Ready";

         try
         {
            if (objectTreeView.SelectedNode != null &&
                objectTreeView.SelectedNode.Tag != null &&
                objectTreeView.SelectedNode.Tag.GetType() == typeof(LayoutObject))
            {
               LayoutObject layoutObject = (LayoutObject)objectTreeView.SelectedNode.Tag;
               if (layoutObject.GetCurrentStatusState() != null)
               {
                  String textureName = layoutObject.GetCurrentStatusState().GetTextureName();
                  RectangleF oldWorldRect = layoutObject.GetWorldBounds();
                  text = String.Format("{0} [{5}] ({3},{4}) ({1} X {2} actual size)", textureName, oldWorldRect.Width, oldWorldRect.Height, oldWorldRect.Left, oldWorldRect.Top, layoutObject.strcode);
               }
            }
         }
         catch (System.Exception) { }

         if (toolStripStatusLabel1.Text != text)
            toolStripStatusLabel1.Text = text;
      }
      #endregion

      #region Private Override Methods

      const int WM_KEYDOWN = 0x100;
      const int WM_SYSKEYDOWN = 0x104;
      const int WM_KEYUP = 0x101;
      const int WM_SYSKEYUP = 0x105;

      private void RefreshWindowWithTreeLock()
      {
         objectTreeView.BeginUpdate();
         RefreshWindow();
         objectTreeView.EndUpdate();
      }

      protected override bool ProcessKeyPreview(ref Message msg)
      {
         Keys keyData = (Keys)msg.WParam;
         if ((msg.Msg == WM_KEYUP) || (msg.Msg == WM_SYSKEYUP))
         {
            switch (keyData)
            {
               case Keys.Left:
               case Keys.Right:
               case Keys.Up:
               case Keys.Down:
                  mIsHoldingDownArrow = false;
                  RefreshWindowWithTreeLock();
                  break;
            }
         }
         return base.ProcessKeyPreview(ref msg);
      }

      protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
      {
         if ((msg.Msg == WM_KEYDOWN) || (msg.Msg == WM_SYSKEYDOWN))
         {
            switch (keyData)
            {
               case Keys.Control | Keys.Z:
                  Undo();
                  break;
               case Keys.Control | Keys.Y:
                  Redo();
                  break;
               case Keys.Left:
                  if (!previewPictureBox.Focused)
                     break;
                  if (mCurrentLayoutData == null)
                     break;
                  NudgeElement(mCurrentLayoutData, -kNudgeAmount, 0);
                  mIsHoldingDownArrow = true;
                  break;
               case Keys.Right:
                  if (!previewPictureBox.Focused)
                     break;
                  if (mCurrentLayoutData == null)
                     break;
                  NudgeElement(mCurrentLayoutData, kNudgeAmount, 0);
                  mIsHoldingDownArrow = true;
                  break;
               case Keys.Up:
                  if (!previewPictureBox.Focused)
                     break;
                  if (mCurrentLayoutData == null)
                     break;
                  NudgeElement(mCurrentLayoutData, 0, -kNudgeAmount);
                  mIsHoldingDownArrow = true;
                  break;
               case Keys.Down:
                  if (!previewPictureBox.Focused)
                     break;
                  if (mCurrentLayoutData == null)
                     break;
                  NudgeElement(mCurrentLayoutData, 0, kNudgeAmount);
                  mIsHoldingDownArrow = true;
                  break;
               case Keys.PageUp:
                  if (!previewPictureBox.Focused)
                     break;
                  if (mCurrentLayoutData == null)
                     break;
                  ScaleElements(mCurrentLayoutData, kNudgeAmount);
                  mIsHoldingDownArrow = true;
                  break;
               case Keys.PageDown:
                  if (!previewPictureBox.Focused)
                     break;
                  if (mCurrentLayoutData == null)
                     break;
                  ScaleElements(mCurrentLayoutData, -kNudgeAmount);
                  mIsHoldingDownArrow = true;
                  break;

               case Keys.Control | Keys.X:
                  CopyCurrentX();
                  break;
               case Keys.Control | Keys.C:
                  CopyCurrentSize();
                  break;
               case Keys.Control | Keys.V:
                  PasteCurrentData();
                  break;
            }
         }

         return base.ProcessCmdKey(ref msg, keyData);
      }
      #endregion

      #region Private Event Handling
      private void previewPictureBox_Paint(object sender, PaintEventArgs e)
      {
         e.Graphics.Clear(LayoutUtil.mBackgroundColor);

         if (mCurrentLayoutData == null)
            return;
         LayoutObject selObj = mOverrideSelectObject;
         if (selObj == null)
         {
            selObj = GetSelectedLayoutObject();
         }
         mCurrentLayoutData.mObjectHeader.Draw(e.Graphics, selObj, mPreviewWidth, mPreviewHeight, mFitToScreenHeight, safeZoneToolStripMenuItem.Checked);
      }
      private void previewPictureBox_Resize(object sender, EventArgs e)
      {
         previewPictureBox.Invalidate();
      }
      private class LayoutObjectSorter : IComparer<LayoutObject>
      {
         int IComparer<LayoutObject>.Compare(LayoutObject a, LayoutObject b)
         {
            RectangleF ar = a.GetWorldBounds();
            RectangleF br = b.GetWorldBounds();
            return (int)(ar.Width * ar.Height - br.Width * br.Height);
         }
      }
      private void previewPictureBox_MouseClick(object sender, System.Windows.Forms.MouseEventArgs e)
      {
         if (e.Button != MouseButtons.Right)
            return;
         if (mCurrentLayoutData == null)
            return;

         Rectangle renderRect = mCurrentLayoutData.GetLayoutBounds(previewPictureBox.Bounds, mPreviewWidth, mPreviewHeight);
         Rectangle selectionBox = new Rectangle(e.Location, new Size(1, 1));
         selectionBox.Inflate(3, 3);

         List<LayoutObject> nodes = new List<LayoutObject>();
         foreach (LayoutObject obj in mCurrentLayoutData.ObjectByID.Values)
         {
            RectangleF rectF = obj.GetWorldBounds();
            if (mFitToScreenHeight)
            {
               float yScaleForScreenFit = 400.0f / 384.0f;
               rectF = new RectangleF(rectF.X, rectF.Y * yScaleForScreenFit, rectF.Width, rectF.Height * yScaleForScreenFit);
            }
            rectF = ConvertUIRectToUnitized(mCurrentLayoutData, rectF);
            Rectangle winRect = new Rectangle(
               renderRect.X + (Int32)(rectF.X * renderRect.Width),
               renderRect.Y + (Int32)(rectF.Y * renderRect.Height),
               (Int32)(rectF.Width * renderRect.Width),
               (Int32)(rectF.Height * renderRect.Height));

            if (winRect.IntersectsWith(selectionBox))
            {
               nodes.Add(obj);
            }
         }

         // sort the nodes by dimensions, smallest to largest
         nodes.Sort(new LayoutObjectSorter());

         if (Control.ModifierKeys == Keys.Control && nodes.Count > 0)
         {
            TreeNode treeNode = GetNodeForLayoutItem(nodes[0], null);
            if (treeNode != null)
            {
               objectTreeView.SelectedNode = treeNode;
               treeNode.EnsureVisible();
            }
            return;
         }

         ContextMenuStrip menuStrip = new ContextMenuStrip();
         menuStrip.Closing += delegate(Object _sender, ToolStripDropDownClosingEventArgs _e)
         {
            mOverrideSelectObject = null;
            previewPictureBox.Invalidate();
         };
         menuStrip.Items.Add("Which Item to Select?").Enabled = false;
         menuStrip.Items.Add("-");

         foreach (LayoutObject obj in nodes)
         {
            ToolStripMenuItem item = (ToolStripMenuItem)menuStrip.Items.Add(obj.mName);
            item.Tag = obj;
            item.MouseMove += delegate(Object _sender, MouseEventArgs _e)
            {
               mOverrideSelectObject = (LayoutObject)((ToolStripMenuItem)_sender).Tag;
               previewPictureBox.Invalidate();
            };
            item.MouseLeave += delegate(Object _sender, EventArgs _e)
            {
               mOverrideSelectObject = null;
               previewPictureBox.Invalidate();
            };
            item.Click += delegate(Object _sender, EventArgs _e)
            {
               TreeNode treeNode = GetNodeForLayoutItem((LayoutObject)((ToolStripMenuItem)_sender).Tag, null);
               if (treeNode != null)
               {
                  objectTreeView.SelectedNode = treeNode;
                  treeNode.EnsureVisible();
               }
            };
         }

         menuStrip.Show(previewPictureBox, e.Location);
      }

      private void objectTreeView_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
      {
         if (e.Button == MouseButtons.Right)
         {
            objectTreeView.SelectedNode = e.Node;
         }
      }
      private void objectTreeView_AfterCheck(object sender, TreeViewEventArgs e)
      {
         if (mCurrentLayoutData == null)
            return;
         if (mCurrentLayoutData.ObjectByName.ContainsKey(e.Node.Name))
         {
            LayoutObject obj = mCurrentLayoutData.ObjectByName[e.Node.Name];
            obj.mVisible = e.Node.Checked;
            previewPictureBox.Invalidate();
         }
      }
      private void objectTreeView_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
      {
         if (mCurrentLayoutData == null)
            return;
         if (e.Label != null)
         {
            if (e.Label.Length > 0 && !mCurrentLayoutData.ObjectByName.ContainsKey(e.Label))
            {
               e.Node.EndEdit(false);
               LayoutObject obj = (LayoutObject)e.Node.Tag;
               if (obj != null)
               {
                  obj.Rename(e.Label);
                  UpdateActionView();
               }
            }
            else
            {
               if (e.Label.Length > 0)
               {
                  toolStripStatusLabel1.Text = "Object with that name already exists!";
               }
               e.CancelEdit = true;
            }
         }
      }
      private void objectTreeView_KeyDown(object sender, KeyEventArgs e)
      {
         TreeNode node = objectTreeView.SelectedNode;
         if (e.KeyCode == Keys.F2 && node != null && node.Tag != null && node.Tag.GetType() == typeof(LayoutObject))
         {
            LayoutObject obj = (LayoutObject)node.Tag;
            if (obj != null)
            {
               node.BeginEdit();
            }
         }
      }
      private void objectTreeView_BeforeLabelEdit(object sender, NodeLabelEditEventArgs e)
      {
         bool cancel = true;
         if (objectTreeView.SelectedNode == e.Node && e.Node != null)
         {
            if (e.Node.Tag != null && e.Node.Tag.GetType() == typeof(LayoutObject))
            {
               LayoutObject obj = (LayoutObject)e.Node.Tag;
               if (obj != null)
               {
                  cancel = false;
               }
            }
         }
         if (cancel)
         {
            e.CancelEdit = true;
         }
      }
      private void objectTreeView_AfterSelect(object sender, TreeViewEventArgs e)
      {
         previewPictureBox.Invalidate();
         actionTreeView.Invalidate();
         SetToolStrip();

         if (objectTreeView.SelectedNode == null)
            return;
         objectTreeView.SelectedNode.NodeFont = new Font(this.Font, FontStyle.Bold);
         objectTreeView.Invalidate();
      }
      private void objectTreeView_BeforeSelect(object sender, TreeViewCancelEventArgs e)
      {
         if (objectTreeView.SelectedNode == null)
            return;
         objectTreeView.SelectedNode.NodeFont = new Font(this.Font, FontStyle.Regular);
         objectTreeView.Invalidate();
      }

      private void actionTreeView_BeforeSelect(object sender, TreeViewCancelEventArgs e)
      {
         if (actionTreeView.SelectedNode == null)
            return;
         actionTreeView.SelectedNode.NodeFont = new Font(this.Font, FontStyle.Regular);
         actionTreeView.Invalidate();
      }
      private void actionTreeView_AfterSelect(object sender, TreeViewEventArgs e)
      {
         if (actionTreeView.SelectedNode == null)
            return;
         actionTreeView.SelectedNode.NodeFont = new Font(this.Font, FontStyle.Bold);
         actionTreeView.Invalidate();
      }
      private void actionTreeView_KeyDown(object sender, KeyEventArgs e)
      {
         TreeNode node = actionTreeView.SelectedNode;
         if (e.KeyCode == Keys.F2 && node != null && node.Tag != null && node.Tag.GetType() == typeof(LayoutActionRef))
         {
            LayoutActionRef actRef = (LayoutActionRef)node.Tag;
            if (actRef != null)
            {
               node.BeginEdit();
            }
         }
      }
      private void actionTreeView_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
      {
         if (mCurrentLayoutData == null)
            return;
         if (e.Label != null)
         {
            if (e.Label.Length > 0 && !mCurrentLayoutData.ActionByName.ContainsKey(e.Label))
            {
               e.Node.EndEdit(false);
               LayoutActionRef actRef = (LayoutActionRef)e.Node.Tag;
               if (actRef != null)
               {
                  actRef.Rename(e.Label);
               }
            }
            else
            {
               if (e.Label.Length > 0)
               {
                  toolStripStatusLabel1.Text = "Action with that name already exists!";
               }
               e.CancelEdit = true;
            }
         }
      }
      private void actionTreeView_BeforeLabelEdit(object sender, NodeLabelEditEventArgs e)
      {
         bool cancel = true;
         if (e.Node != null && e.Node.Tag != null)
         {
            if (e.Node.Tag.GetType() == typeof(LayoutActionRef))
            {
               LayoutActionRef actRef = (LayoutActionRef)e.Node.Tag;
               if (actRef != null)
               {
                  cancel = false;
               }
            }
         }
         if (cancel)
         {
            e.CancelEdit = true;
         }
      }
      private void actionTreeView_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
      {
         if (mCurrentLayoutData == null)
            return;
         if (e.Button == MouseButtons.Right)
         {
            e.Node.TreeView.SelectedNode = e.Node;

            if (mCurrentLayoutData.ActionByName.ContainsKey(e.Node.Text))
            {
               LayoutActionRef action = mCurrentLayoutData.ActionByName[e.Node.Text];
               foreach (LayoutKeyHeader objKey in action.mAction.mObjectKeys)
               {
                  if (mCurrentLayoutData.ObjectByID.ContainsKey(objKey.usId))
                  {
                     LayoutObject obj = mCurrentLayoutData.ObjectByID[objKey.usId];
                     foreach (LayoutKey key in objKey.mKeys)
                     {
                        if (key.GetKeyType() == LayoutKey.EKeyType.Set)
                        {
                           foreach (LayoutStatusRef status in obj.mStatuses)
                           {
                              foreach (LayoutStatusState statusState in status.mStatus.mStates)
                              {
                                 if ((key.uiCmd & 0x00ffffff) == statusState.mCode)
                                 {
                                    // apply to object
                                    obj.SetCurrentStatusState(statusState);
                                 }
                              }
                           }
                        }
                        else if (key.GetKeyType() == LayoutKey.EKeyType.Morph)
                        {
                           foreach (LayoutStatusRef status in obj.mStatuses)
                           {
                              foreach (LayoutStatusState statusState in status.mStatus.mStates)
                              {
                                 if ((key.uiStat & 0x00ffffff) == statusState.mCode)
                                 {
                                    // apply to object
                                    obj.SetCurrentStatusState(statusState);
                                 }
                              }
                           }
                        }
                     }
                  }
               }
               objectTreeView.Invalidate();
               previewPictureBox.Invalidate();
            }
         }
      }

      private void fileToolStripMenuItem_DropDownOpening(object sender, EventArgs e)
      {
         MRU.DropDownOpening(
            "LayoutEditorMGS2",
            recentItemsToolStripMenuItem,
            new EventHandler(delegate(object snd, EventArgs ea)
            {
               ToolStripMenuItem item = (ToolStripMenuItem)snd;
               LoadFile((string)item.Tag);
            }));
      }
      private void openToolStripMenuItem_Click(object sender, EventArgs e)
      {
         OpenFileDialog openFileBox = new OpenFileDialog();
         openFileBox.AddExtension = true;
         openFileBox.DefaultExt = "o2d";
         openFileBox.DereferenceLinks = true;
         if (mCurrentFilename != null)
         {
            openFileBox.FileName = mCurrentFilename;
         }
         openFileBox.Filter = "Layout Files (*.o2d, *.xml)|*.o2d;*.xml";
         openFileBox.ValidateNames = true;
         if (String.IsNullOrEmpty(openFileBox.FileName) && String.IsNullOrEmpty(openFileBox.InitialDirectory))
         {
            string gameData = System.Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%");
            gameData = gameData.Replace("\\MGS3\\", "\\MGS2\\");
            gameData += "\\assets\\o2d\\us";

            openFileBox.InitialDirectory = gameData;
         }
         openFileBox.CheckFileExists = true;
         openFileBox.CheckPathExists = true;

         DialogResult result = openFileBox.ShowDialog(this);
         if (result == DialogResult.OK)
         {
            LoadFile(openFileBox.FileName);
         }
      }
      private void saveToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (mCurrentLayoutData != null)
         {
            // write xml
            string fileName = mCurrentFilename;
            if (!fileName.EndsWith(".xml"))
            {
               fileName += ".xml";
            }
            XmlDocument xmlDoc = new XmlDocument();
            mCurrentLayoutData.ToXML(xmlDoc, newStrCodesInXMLToolStripMenuItem.Checked);
            xmlDoc.Save(fileName);
         }
      }
      private void exportToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (mCurrentLayoutData == null)
            return;

         Byte[] toSaveData = SaveStateAsByteArray();

         // read stream
         FileStream fs = File.OpenRead(mCurrentFilename);
         Byte[] buf = new Byte[fs.Length];
         fs.Read(buf, 0, buf.Length);
         fs.Close();

         // compare input and output
         if (buf.Length != toSaveData.Length)
         {
            // error
            System.Console.WriteLine("Input and output sizes differ!");
         }
         else
         {
            Int32 hasIndividualErrors = -1;
            for (int i = 0; i < buf.Length; ++i)
            {
               if (buf[i] != toSaveData[i])
               {
                  hasIndividualErrors = i;
                  break;
               }
            }
            if (hasIndividualErrors != -1)
               mCurrentLayoutData.NoteError(hasIndividualErrors);
         }

         // write data to file
         try
         {
            string backupFileName = mCurrentFilename + ".bkp";
            if (File.Exists(backupFileName))
            {
               try
               {
                  File.Delete(backupFileName);
               }
               catch (System.Exception)
               {
                  File.Move(mCurrentFilename, backupFileName);
               }
            }
         }
         catch (System.Exception)
         {
         }

         FileAttributes attrib = File.GetAttributes(mCurrentFilename);
         if ((attrib & FileAttributes.ReadOnly) != 0)
         {
            MessageBox.Show("File is probably not checked out!");
         }
         else
         {
            try
            {
               String fileName = mCurrentFilename;
               FileStream fso = File.Create(fileName);
               fso.Write(toSaveData, 0, toSaveData.Length);
               fso.Flush();
               fso.Close();
            }
            catch (System.Exception)
            {
            }

         }
      }
      private void exitToolStripMenuItem_Click(object sender, EventArgs e)
      {
         Application.Exit();
      }

      private void undoToolStripMenuItem_Click(object sender, EventArgs e)
      {
         Undo();
      }
      private void redoToolStripMenuItem_Click(object sender, EventArgs e)
      {
         Redo();
      }
      private void findToolStripMenuItem_Click(object sender, EventArgs e)
      {
         String objectSearch = "";
         if (objectTreeView.SelectedNode != null && objectTreeView.SelectedNode.Tag != null && objectTreeView.SelectedNode.Tag.GetType() == typeof(LayoutObject))
         {
            objectSearch = objectTreeView.SelectedNode.Name;
            Find(objectSearch);
         }

         if (mFindDialog == null)
         {
            mFindDialog = new FindDialog(objectSearch, Find);
            mFindDialog.FormClosing += delegate(Object _sender, FormClosingEventArgs _e)
            {
               mFindDialog = null;
            };
            mFindDialog.Show(this);
         }
         else
         {
            mFindDialog.Activate();
         }
      }
      private void safeZoneToolStripMenuItem_CheckedChanged(object sender, EventArgs e)
      {
         previewPictureBox.Invalidate();
      }
      private void editNodeDataToolStripMenuItem_Click(object sender, EventArgs e)
      {
         EditSelectedNodeData();
      }
      private void viewTextureToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (mCurrentLayoutData == null)
            return;
         if (objectTreeView.SelectedNode == null)
            return;
         if (objectTreeView.SelectedNode.Tag == null)
            return;
         if (objectTreeView.SelectedNode.Tag.GetType() != typeof(LayoutObject))
            return;

         LayoutObject obj = (LayoutObject)objectTreeView.SelectedNode.Tag;
         LayoutStatusState state = obj.GetCurrentStatusState();
         if (state != null)
         {
            String textureName = state.GetTextureName();
            if (!String.IsNullOrEmpty(textureName))
            {
               System.Diagnostics.Process.Start(textureName);
            }
         }
      }
      private void exportReferencedTexturesToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (mCurrentLayoutData == null)
            return;
         List<String> objectTextureList = new List<String>();
         foreach (LayoutObject obj in mCurrentLayoutData.ObjectByID.Values)
         {
            foreach (LayoutStatusRef statRef in obj.mStatuses)
            {
               foreach (LayoutStatusState state in statRef.mStatus.mStates)
               {
                  String texName = state.GetTextureName();
                  if (!String.IsNullOrEmpty(texName) && !objectTextureList.Contains(texName))
                  {
                     objectTextureList.Add(texName);
                  }
               }
            }
         }

         String textureList = "";
         foreach (String texName in objectTextureList)
         {
            textureList += texName + "\r\n";
         }

         SaveFileDialog fileDialog = new SaveFileDialog();
         fileDialog.AddExtension = true;
         fileDialog.CreatePrompt = false;
         fileDialog.DefaultExt = "txt";
         fileDialog.Filter = "Text File|*.txt";
         fileDialog.OverwritePrompt = true;
         fileDialog.RestoreDirectory = true;
         fileDialog.Title = "Save Exported Filenames to Texture";

         if (DialogResult.OK != fileDialog.ShowDialog())
            return;

         if (!Directory.Exists(Path.GetDirectoryName(fileDialog.FileName)))
            Directory.CreateDirectory(Path.GetDirectoryName(fileDialog.FileName));

         File.WriteAllText(fileDialog.FileName, textureList);

         System.Diagnostics.Process.Start(fileDialog.FileName);
      }
      private void exportReferencedTexturesForAllFilesToolStripMenuItem_Click(object sender, EventArgs e)
      {
         List<String> failedToExport = new List<String>();
         List<String> allTextureList = new List<String>();

         string gameData = System.Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%");
         gameData = gameData.Replace("\\MGS3\\", "\\MGS2\\");
         gameData += "\\assets\\o2d";

         String[] o2dFiles = Directory.GetFiles(gameData, "*.o2d", SearchOption.AllDirectories);
         Int32 processed = 0;
         foreach (String la2File in o2dFiles)
         {
            ++processed;
            toolStripStatusLabel1.Text = String.Format("{0}% Processing {1}", (Int32)(processed / o2dFiles.Length), Path.GetFileName(la2File));

            if (Path.GetFileName(la2File).StartsWith("map_"))
               continue;

            LayoutData tempData = new LayoutData();

            FileStream fs = null;
            BinaryReader br = null;
            try
            {
               // read stream
               fs = File.OpenRead(la2File);
               Byte[] buf = new Byte[fs.Length];
               fs.Read(buf, 0, buf.Length);
               fs.Close();

               // make data
               MemoryStream msIn = new MemoryStream(buf);
               br = new BinaryReader(msIn);
               LayoutUtil.RecreateWordLists(new System.Random(666));
               LayoutData.ReadLayoutData(tempData, br);
            }
            catch (System.Exception ex)
            {
               System.Console.WriteLine(ex.Message);
               failedToExport.Add(la2File);
            }
            finally
            {
               if (fs != null)
                  fs.Close();
               if (br != null)
                  br.Close();
            }


            List<String> objectTextureList = new List<String>();
            foreach (LayoutObject obj in tempData.ObjectByID.Values)
            {
               foreach (LayoutStatusRef statRef in obj.mStatuses)
               {
                  foreach (LayoutStatusState state in statRef.mStatus.mStates)
                  {
                     String texName = state.GetTextureName();
                     if (!String.IsNullOrEmpty(texName))
                     {
                        if (!objectTextureList.Contains(texName))
                        {
                           objectTextureList.Add(texName);
                        }
                        if (!allTextureList.Contains(texName))
                        {
                           allTextureList.Add(texName);
                        }
                     }
                  }
               }
            }

            String textureList = "";
            foreach (String texName in objectTextureList)
            {
               textureList += texName + "\r\n";
            }

            String txtFileName = la2File + ".txt";
            if (!Directory.Exists(Path.GetDirectoryName(txtFileName)))
               Directory.CreateDirectory(Path.GetDirectoryName(txtFileName));

            File.WriteAllText(txtFileName, textureList);
         }

         String allTextures = "";
         foreach (String texName in allTextureList)
         {
            allTextures += texName + "\r\n";
         }

         String allTxtFileName = gameData + "\\__AllTexturesInLayoutFiles.txt";
         if (!Directory.Exists(Path.GetDirectoryName(allTxtFileName)))
            Directory.CreateDirectory(Path.GetDirectoryName(allTxtFileName));

         File.WriteAllText(allTxtFileName, allTextures);
      }


      private void backgroundColorToolStripMenuItem_Click(object sender, EventArgs e)
      {
         ColorDialog dlg = new ColorDialog();
         dlg.Color = LayoutUtil.mBackgroundColor;
         dlg.AnyColor = true;
         dlg.SolidColorOnly = true;
         dlg.AllowFullOpen = true;
         dlg.FullOpen = true;
         DialogResult result = dlg.ShowDialog();
         if (result == DialogResult.OK)
         {
            LayoutUtil.mBackgroundColor = dlg.Color;
            previewPictureBox.Invalidate();
         }
      }

      private void widescreenUIAdjustToolStripMenuItem_Click(object sender, EventArgs e)
      {
      }
      private void makeElementsWidescreenToolStripMenuItem_Click(object sender, EventArgs e)
      {
         PreserveForUndo();

         ResizeObjectsForWideScreen(mCurrentLayoutData);

         RefreshWindowWithTreeLock();
      }
      private void selectedElementsMadeWidescreenToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (objectTreeView.SelectedNode == null)
            return;
         if (objectTreeView.SelectedNode.Tag == null)
            return;
         if (objectTreeView.SelectedNode.Tag.GetType() != typeof(LayoutObject))
            return;

         PreserveForUndo();

         LayoutObject obj = (LayoutObject)objectTreeView.SelectedNode.Tag;
         foreach (LayoutStatusRef statRef in obj.mStatuses)
         {
            foreach (LayoutStatusState stat in statRef.mStatus.mStates)
            {
               RectangleF localRect = stat.GetLocalBounds();
               foreach (LayoutStatusCmd cmd in stat.mCommands)
               {
                  switch (cmd.GetCommand())
                  {
                     case LayoutStatusCmd.EStatCmd.VERTEX:
                        foreach (LayoutStatusCmd.Vertex vert in cmd.vertexes)
                        {
                           vert.x = (vert.x * 0.75f) + (localRect.X * 0.25f);
                        }
                        break;
                     case LayoutStatusCmd.EStatCmd.SIZE:
                        {
                           cmd.size_w = cmd.size_w * 0.75f;
                        }
                        break;
                  }
               }
            }
         }

         RefreshWindowWithTreeLock();
      }
      private void selectedElementsMadeWidescreenCenterToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (objectTreeView.SelectedNode == null)
            return;
         if (objectTreeView.SelectedNode.Tag == null)
            return;
         if (objectTreeView.SelectedNode.Tag.GetType() != typeof(LayoutObject))
            return;

         PreserveForUndo();

         LayoutObject obj = (LayoutObject)objectTreeView.SelectedNode.Tag;
         foreach (LayoutStatusRef statRef in obj.mStatuses)
         {
            foreach (LayoutStatusState stat in statRef.mStatus.mStates)
            {
               RectangleF localRect = stat.GetLocalBounds();
               foreach (LayoutStatusCmd cmd in stat.mCommands)
               {
                  switch (cmd.GetCommand())
                  {
                     case LayoutStatusCmd.EStatCmd.VERTEX:
                        foreach (LayoutStatusCmd.Vertex vert in cmd.vertexes)
                        {
                           vert.x = (vert.x * 0.75f) + (localRect.X * 0.25f) + (localRect.Width - (localRect.Width * 0.75f)) * 0.5f;
                        }
                        break;
                     case LayoutStatusCmd.EStatCmd.SIZE:
                        {
                           cmd.size_w = cmd.size_w * 0.75f;
                        }
                        break;
                  }
               }
            }
         }

         RefreshWindowWithTreeLock();
      }

      private void selectedElementsToLeftToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (objectTreeView.SelectedNode == null)
            return;
         if (objectTreeView.SelectedNode.Tag == null)
            return;
         if (objectTreeView.SelectedNode.Tag.GetType() != typeof(LayoutObject))
            return;

         PreserveForUndo();

         LayoutObject obj = (LayoutObject)objectTreeView.SelectedNode.Tag;
         PushElementToLeftForWideScreen(mCurrentLayoutData, obj);

         RefreshWindowWithTreeLock();
      }
      private void selectedElementsToRightToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (objectTreeView.SelectedNode == null)
            return;
         if (objectTreeView.SelectedNode.Tag == null)
            return;
         if (objectTreeView.SelectedNode.Tag.GetType() != typeof(LayoutObject))
            return;

         PreserveForUndo();
         LayoutObject obj = (LayoutObject)objectTreeView.SelectedNode.Tag;
         PushElementToRightForWideScreen(mCurrentLayoutData, obj);

         RefreshWindowWithTreeLock();
      }
      private void selectedElementsFillToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (objectTreeView.SelectedNode == null)
            return;
         if (objectTreeView.SelectedNode.Tag == null)
            return;
         if (objectTreeView.SelectedNode.Tag.GetType() != typeof(LayoutObject))
            return;

         PreserveForUndo();
         LayoutObject obj = (LayoutObject)objectTreeView.SelectedNode.Tag;
         ExpandObjectToWidth(mCurrentLayoutData, obj);

         RefreshWindowWithTreeLock();
      }
      private void validateExportProcessOnAllFilesToolStripMenuItem_Click(object sender, EventArgs e)
      {
         List<String> filesNotTheSame = new List<String>();

         string gameData = System.Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%");
         gameData = gameData.Replace("\\MGS3\\", "\\MGS2\\");
         gameData += "\\assets\\o2d\\us";

         String[] o2dFiles = Directory.GetFiles(gameData, "*.o2d");
         Int32 processed = 0;
         foreach (String o2dFile in o2dFiles)
         {
            ++processed;
            toolStripStatusLabel1.Text = String.Format("{0}% Validating {1}", (Int32)(processed / o2dFiles.Length), Path.GetFileName(o2dFile));

            FileStream fs = null;
            BinaryReader br = null;
            BinaryWriter bw = null;
            try
            {
               // read stream
               fs = File.OpenRead(o2dFile);
               Byte[] buf = new Byte[fs.Length];
               fs.Read(buf, 0, buf.Length);

               // make data
               LayoutData tempData = new LayoutData();
               MemoryStream msIn = new MemoryStream(buf);
               br = new BinaryReader(msIn);
               LayoutUtil.RecreateWordLists(new System.Random(666));
               LayoutData.ReadLayoutData(tempData, br);

               // export data
               MemoryStream msOut = new MemoryStream();
               bw = new BinaryWriter(msOut);
               tempData.Write(bw);

               // validate header data
               if (!CompareBuffers(msIn.ToArray(), msOut.ToArray(), 48))
                  filesNotTheSame.Add(o2dFile);
            }
            catch (System.Exception)
            {
               filesNotTheSame.Add(o2dFile);
            }
            finally
            {
               if (fs != null)
                  fs.Close();
               if (br != null)
                  br.Close();
               if (bw != null)
                  bw.Close();
            }
         }

         if (filesNotTheSame.Count == 0)
         {
            MessageBox.Show(String.Format("{0} files processed.\nSuccess!!!\nAll files are the same once exported!", o2dFiles.Length), "Results");
         }
         else
         {
            String badFiles = "";
            foreach (String diffFiles in filesNotTheSame)
               badFiles += diffFiles + "\n";

            MessageBox.Show(String.Format("{0} files processed.\nFailed!!!\n{1} files have different headers. Please check the following:\n\n{2}", o2dFiles.Length, filesNotTheSame.Count, badFiles), "Results");
         }
         SetToolStrip();
      }
      private void exportAllFilesToWidescreenToolStripMenuItem_Click(object sender, EventArgs e)
      {
         List<String> failedToExport = new List<String>();

         string gameData = System.Environment.ExpandEnvironmentVariables("%BPE_REPOSITORY%");
         gameData = gameData.Replace("\\MGS3\\", "\\MGS2\\");
         gameData += "\\assets\\o2d";

         String[] o2dFiles = Directory.GetFiles(gameData, "*.o2d", SearchOption.AllDirectories);
         Int32 processed = 0;
         foreach (String o2dFile in o2dFiles)
         {
            ++processed;
            toolStripStatusLabel1.Text = String.Format("{0}% Processing {1}", (Int32)(processed / o2dFiles.Length), Path.GetFileName(o2dFile));

            if (Path.GetFileName(o2dFile).StartsWith("map_"))
               continue;

            FileStream fs = null;
            BinaryReader br = null;
            BinaryWriter bw = null;
            try
            {
               // read stream
               fs = File.OpenRead(o2dFile);
               Byte[] buf = new Byte[fs.Length];
               fs.Read(buf, 0, buf.Length);
               fs.Close();

               // make data
               LayoutData tempData = new LayoutData();
               MemoryStream msIn = new MemoryStream(buf);
               br = new BinaryReader(msIn);
               LayoutUtil.RecreateWordLists(new System.Random(666));
               LayoutData.ReadLayoutData(tempData, br);

               if (tempData.widescreen != 0)
                  continue;

               ResizeObjectsForWideScreen(tempData);

               // Backup the original file
               if (File.Exists(o2dFile + ".bkp"))
                  File.Delete(o2dFile + ".bkp");
               File.Move(o2dFile, o2dFile + ".bkp");

               // write stream
               FileStream fsw = File.OpenWrite(o2dFile);
               bw = new BinaryWriter(fsw);
               tempData.Write(bw);
            }
            catch (System.Exception ex)
            {
               System.Console.WriteLine(ex.Message);
               failedToExport.Add(o2dFile);
            }
            finally
            {
               if (fs != null)
                  fs.Close();
               if (br != null)
                  br.Close();
               if (bw != null)
                  bw.Close();
            }
         }

         if (failedToExport.Count == 0)
         {
            MessageBox.Show(String.Format("{0} files processed.\nSuccess!!!\nAll files are now in widescreen!", o2dFiles.Length), "Results");
         }
         else
         {
            String badFiles = "";
            foreach (String diffFiles in failedToExport)
               badFiles += diffFiles + "\n";

            MessageBox.Show(String.Format("{0} files processed.\nFailed!!!\n{1} files failed to export. Please check the following:\n\n{2}", o2dFiles.Length, failedToExport.Count, badFiles), "Results");
         }

         SetToolStrip();
      }

      private void keyboardShortcutsToolStripMenuItem_Click(object sender, EventArgs e)
      {
         MessageBox.Show(
            "Ctrl+O     Open\n" +
            "Ctrl+R     Open Recent\n" +
            "Ctrl+S     Save to XML\n" +
            "Ctrl+E     Export to o2d\n" +
            "Alt+F4     Exit\n" + "\n" +
            "Ctrl+Z     Undo\n" +
            "Ctrl+Y     Redo\n" +
            "F8            Show Safe Zone\n" +
            "F3            Edit Node Data\n" +
            "F4            Resize Elements for Widescreen\n" +
            "F6            View Texture\n" +
            "F9            Toggle Pixel Perfect for element\n" +
            "Ctrl+Left  Move selected element to the left side\n" +
            "Ctrl+Right Move selected element to the right side\n" + "\n" +
            "Ctrl+Up     Expand selected element to fill the width\n" + "\n" +
            "Left        Nudge selected element 1 UI unit to the left\n" +
            "Right      Nudge selected element 1 UI unit to the right\n" +
            "Up          Nudge selected element 1 UI unit up\n" +
            "Down      Nudge selected element 1 UI unit down\n" + "\n" +
            "PageUp    Scale up selected element by one pixel in height\n" + "\n" +
            "PageDown  Scale down selected element by one pixel in height\n" + "\n" +
            "F1         Help\n"
            , "Keyboard Shortcuts");
      }
      #endregion

      private void fitToScreenHeightMenuItem_CheckedChanged(object sender, EventArgs e)
      {
         ToolStripMenuItem it = (ToolStripMenuItem)sender;
         mFitToScreenHeight = it.Checked;

         RefreshWindowWithTreeLock();
      }

      private void hdToolStripMenuItem_CheckedChanged(object sender, EventArgs e)
      {
         ToolStripMenuItem it = (ToolStripMenuItem)sender;
         if (it.Checked)
         {
            vitaToolStripMenuItem.Checked = false;

            mPreviewWidth = 1280.0;
            mPreviewHeight = 720.0;
         }
         else
         {
            mPreviewWidth = 512.0;
            mPreviewHeight = 400.0;
         }

         RefreshWindowWithTreeLock();
      }

      private void vitaToolStripMenuItem_CheckedChanged(object sender, EventArgs e)
      {
         ToolStripMenuItem it = (ToolStripMenuItem)sender;
         if (it.Checked)
         {
            hdToolStripMenuItem.Checked = false;

            mPreviewWidth = 960.0;
            mPreviewHeight = 540.0;
         }
         else
         {
            mPreviewWidth = 512.0;
            mPreviewHeight = 400.0;
         }

         RefreshWindowWithTreeLock();
      }

      private void togglePixelPerfectOnSelectedToolStripMenuItem_Click(object sender, EventArgs e)
      {
         TogglePixelPerfect(mCurrentLayoutData);
      }

      private void perforceCheckOutToolStripMenuItem_Click(object sender, EventArgs e)
      {
         PerforceCmd.CheckoutFile(mCurrentFilename, false, true);
      }

      private void gatherStringIDsFromSourceFilesToolStripMenuItem_Click(object sender, EventArgs e)
      {
         string fileName = StringIdGatherer.CreateStringIdsFile();
         toolStripStatusLabel1.Text = String.Format("Created {0}", fileName);
      }

      private void exploreToolStripMenuItem_Click(object sender, EventArgs e)
      {
         Process.Start("explorer.exe", "/select,\"" + mCurrentFilename + "\"");
      }
   }

   public class TreeViewState
   {
      #region Public Fields
      private class TreeNodeState
      {
         public Boolean mIsChecked;
         public Boolean mIsExpanded;
         public Boolean mIsSelected;
      }
      private Dictionary<Int32, TreeNodeState> StateMap = new Dictionary<Int32, TreeNodeState>();
      Point scrollPos;
      #endregion

      [DllImport("user32.dll", CharSet = CharSet.Auto)]
      static extern int GetScrollPos(int hWnd, int nBar);

      [DllImport("user32.dll")]
      static extern int SetScrollPos(IntPtr hWnd, int nBar, int nPos, bool bRedraw);

      private const int SB_HORZ = 0x0;
      private const int SB_VERT = 0x1;

      private Point GetTreeViewScrollPos(TreeView treeView)
      {
         return new Point(
             GetScrollPos((int)treeView.Handle, SB_HORZ),
             GetScrollPos((int)treeView.Handle, SB_VERT));
      }

      private void SetTreeViewScrollPos(TreeView treeView, Point scrollPosition)
      {
         SetScrollPos((IntPtr)treeView.Handle, SB_HORZ, scrollPosition.X, true);
         SetScrollPos((IntPtr)treeView.Handle, SB_VERT, scrollPosition.Y, true);
      }

      public TreeViewState(TreeView inView)
      {
         Store(inView);
         scrollPos = GetTreeViewScrollPos(inView);
      }
      public void Store(TreeView inView)
      {
         int objIndex = 0;
         foreach (TreeNode node in inView.Nodes)
         {
            Store(inView, node, ref objIndex);
         }
      }
      public void Restore(TreeView inView)
      {
         int objIndex = 0;
         foreach (TreeNode node in inView.Nodes)
         {
            Restore(inView, node, ref objIndex);
         }
         SetTreeViewScrollPos(inView, scrollPos);
      }

      #region Private Methods
      private void Store(TreeView inView, TreeNode inNode, ref int objIndex)
      {
         TreeNodeState state = new TreeNodeState();
         state.mIsChecked = inNode.Checked;
         state.mIsExpanded = inNode.IsExpanded;
         state.mIsSelected = inNode.IsSelected;

         StateMap[objIndex] = state;

         objIndex++;
         foreach (TreeNode node in inNode.Nodes)
         {
            Store(inView, node, ref objIndex);
         }
      }
      private void Restore(TreeView inView, TreeNode inNode, ref int objIndex)
      {
         TreeNodeState state = StateMap[objIndex];
         inNode.Checked = state.mIsChecked;
         if (state.mIsExpanded)
         {
            inNode.Expand();
         }
         if (state.mIsSelected)
         {
            inView.SelectedNode = inNode;
         }

         objIndex++;
         foreach (TreeNode node in inNode.Nodes)
         {
            Restore(inView, node, ref objIndex);
         }
      }
      #endregion
   }

   public class FindDialog : Form
   {
      private static List<String> mPreviousEntries = new List<String>();

      public delegate void StringEventHandler(String inFind);

      public FindDialog(String inInitialString, StringEventHandler inHandler)
      {
         const Int32 kWidth = 300;
         const Int32 kHeight = 150;
         const Int32 kCenterX = kWidth / 2;
         const Int32 kBorder = 20;

         ComboBox comboBox1 = new ComboBox();
         comboBox1.Location = new Point(kBorder, kBorder);
         comboBox1.Size = new Size(kWidth - 2 * kBorder, 20);
         foreach (String str in mPreviousEntries)
            comboBox1.Items.Add(str);
         if (!String.IsNullOrEmpty(inInitialString))
            comboBox1.Text = inInitialString;

         Button findButton = new Button();
         findButton.Text = "Find Next";
         findButton.Location = new Point(kBorder, kHeight - kBorder - 50);
         findButton.Size = new Size((kWidth - 3 * kBorder) / 2, 25);
         findButton.Click += delegate(Object sender, EventArgs e) 
            { 
               if (inHandler != null) 
                  inHandler(comboBox1.Text);
               if (mPreviousEntries.Contains(comboBox1.Text))
                  mPreviousEntries.Remove(comboBox1.Text);
               mPreviousEntries.Insert(0, comboBox1.Text);
            };

         Button cancelButton = new Button();
         cancelButton.Text = "Cancel";
         cancelButton.Location = new Point(kCenterX + kBorder / 2, kHeight - kBorder - 50);
         cancelButton.Size = new Size((kWidth - 3 * kBorder) / 2, 25);
         cancelButton.Click += delegate(Object sender, EventArgs e) { Close(); };

         this.Controls.Add(comboBox1);
         this.Controls.Add(findButton);
         this.Controls.Add(cancelButton);
         this.CancelButton = cancelButton;
         this.AcceptButton = findButton;
         this.Size = new Size(kWidth, kHeight);
         this.Text = "Find Object in Action";
         this.StartPosition = FormStartPosition.CenterParent;
      }
   }

   public class SelectablePictureBox : PictureBox
   {
      public SelectablePictureBox()
      {
         SetStyle(ControlStyles.Selectable, true);
      }
      protected override void OnMouseDown(MouseEventArgs e)
      {
         this.Select();
         base.OnMouseDown(e);
      }
   }
}
