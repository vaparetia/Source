using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using Tools.Common;
using System.Drawing;

namespace Tools.Controls
{
   public class TreeListView : ListView
   {
      public class Exception : System.Exception
      {
         public Exception(String s)
            : base(s)
         {
         }
      }

      public class Node
      {

#region "Public Methods"
         public Node()
         {
            mExpanded = true;
            mItem = null;
            Tag = null;
         }

         public Node(ListViewItem item, bool expanded )
         {
            ThrowIfItemIsBound(item);

            mExpanded = expanded;
            mItem = item;
            mItem.Tag = this;
         }

         public static Node NewCollapsedNode( Node parent, ListViewItem item)
         {
            Node n = new Node(item, false);

            if (parent != null)
            {
               parent.AddChild(n);
            }

            return n;
         }

         public static Node NewExpandedNode( Node parent, ListViewItem item)
         {
            Node n = new Node(item, true);

            if (parent != null)
            {
               parent.AddChild(n);
            }

            return n;
         }

         public void SetAsRoot(TreeListView view)
         {
            mView = view;
         }

         public void AddChild(Node n)
         {
            InsertChildBefore_Internal(n, mChildren.Count);
         }

         public void InsertChildBefore(Node n, Node before)
         {
            InsertChildBefore_Internal(n, IndexOfChildOrThrow(before));
         }

         public void InsertChildAfter(Node n, Node after)
         {
            InsertChildBefore_Internal(n, IndexOfChildOrThrow(after) + 1 );
         }

         public void RemoveChild(Node n)
         {
            int index = IndexOfChildOrThrow(n);
            mChildren.RemoveAt(index);

            n.SetParent(null);
         }
#endregion
#region "Private Methods"
         private void InsertChildBefore_Internal(Node n, int index)
         {
            bool shouldAddToListView = IsExposed && Expanded && mView.ShouldManipulateNodeParentInListView(this);
            int listViewInsertPoint = -1;
            
            n.ThrowIfParented("add");
            n.ThrowIfNoItem();

            if (index == mChildren.Count)
            {
               // Since we're adding to the end, find my next sibling and try to insert before that.
               // If I don't have a next sibling, find parent's next sibling, et cetera until we hit the root
               // If we hit the root, keep the insert point at -1, which will put it at the end

               if (shouldAddToListView)
               {
                  for (Node checking = this; checking.mItem != null; checking = checking.mParent)
                  {
                     Node sibling = checking.NextSibling;

                     if (sibling != null)
                     {
                        listViewInsertPoint = sibling.mItem.Index;
                        break;
                     }
                  }
               }

               mChildren.Add(n);
            }
            else
            {
               if (shouldAddToListView)
               {
                  listViewInsertPoint = mChildren[index].mItem.Index;
               }

               mChildren.Insert(index, n);

            }

            n.SetParent(this);

            if (shouldAddToListView)
            {
               if (listViewInsertPoint == -1)
               {
                  mView.Items.Add(n.mItem);
               }
               else
               {
                  mView.Items.Insert(listViewInsertPoint, n.mItem);
               }

               if (n.Expanded)
               {
                  n.Internal_ExposeChildren();
               }
            }

         }

         private void Internal_ExposeChildren()
         {
            Redraw();

            mView.Internal_ExposeChildrenStart(this);

            int startIndex = mItem.Index + 1;

            for ( int childIndex = 0; childIndex < mChildren.Count; ++childIndex )
            {
               mView.Items.Insert(startIndex + childIndex, mChildren[childIndex].mItem);
            }

            mView.Internal_ExposeChildrenEnd(this);

            foreach (Node child in mChildren)
            {
               if (child.Expanded)
               {
                  child.Internal_ExposeChildren();
               }
            }
         }

         private void Internal_HideChildren( ref Stack<Node> collapsed )
         {
            collapsed.Push(this);

            Redraw();

            foreach (Node child in mChildren)
            {
               if (child.Expanded)
               {
                  child.Internal_HideChildren(ref collapsed);
               }

               if (child.mItem.Selected)
               {
                  child.mItem.Selected = false;
               }
               mView.Items.Remove(child.mItem);
            }
         }

         private void ThrowIfParented( String operation )
         {
            if (mParent != null)
            {
               throw new Exception(String.Format("Tried to {0} a node that already has a parent", operation));
            }
         }

         private void ThrowIfNoItem()
         {
            if (mItem == null)
            {
               throw new Exception("Trying to use a node that has no list item");
            }
         }

         static private void ThrowIfItemIsBound( ListViewItem item )
         {
            if ( ( item.ListView != null ) || ( item.Tag != null ) )
            {
               throw new Exception(String.Format("Trying operation with a list view item that's already owned"));
            }
         }

         private int IndexOfChildOrThrow(Node n)
         {
            int index = mChildren.IndexOf(n);
            if (index == -1)
            {
               throw new Exception("Could not find child node");
            }
            
            return index;
         }

         void Redraw()
         {
            // Check if we have a view AND an item AND that the item's view is our view
            if (mView != null && mItem != null && mItem.ListView == mView )
            {
               mView.RedrawItems(mItem.Index, mItem.Index, false);
            }
         }

         void SetParent(Node parent)
         {
            if (parent == null)
            {
               if (mParent == null)
               {
                  throw new Exception("Trying to unparent an unparented node");
               }

               mParent = null;
               mView = null;
               mIndentLevel = -1;
               if (mItem != null )
               {
                  mItem.IndentCount = 0;
               }
            }
            else
            {
               if (mParent != null)
               {
                  throw new Exception("Trying to reparent a node - this isn't allowed");
               }

               mParent = parent;

               if (parent.mView != null)
               {
                  RecursiveFixIndentAndView();
               }
            }
         }

         void RecursiveFixIndentAndView()
         {
            mView = mParent.mView;
            mIndentLevel = mParent.mIndentLevel + 1;

            mItem.IndentCount = mIndentLevel + ( mView.HasSmallImageList() ? 1 : 0 );

            foreach (Node n in mChildren)
            {
               n.RecursiveFixIndentAndView();
            }
         }

#endregion

#region "Properties"
         public IEnumerable<Node> Children
         {
            get
            {
               if (mChildren != null)
               {
                  return new List<Node>(mChildren);
               }
               else
               {
                  return skNoChildren;
               }
            }
         }

         public Node FirstChild
         {
            get
            {
               if (mChildren.Count > 0)
               {
                  return mChildren[0];
               }
               else
               {
                  return null;
               }
            }
         }

         public TreeListView View
         {
            get { return mView; }
         }

         public bool IsExposed
         {
            get
            {
               if (mItem != null)
               {
                  return mView != null && mItem.ListView == mView;
               }
               else
               {
                  return true;
               }
            }
         }

         public bool HasChildren
         {
            get { return mChildren.Count > 0; }
         }

         public bool AlwaysShowPlusMinus
         {
            get { return mAlwaysShowPlusMinus; }
            set
            {
               if (value != mAlwaysShowPlusMinus)
               {
                  mAlwaysShowPlusMinus = value;
                  Redraw();
               }
            }
         }

         public bool IsShowingPlusMinus { get { return HasChildren || AlwaysShowPlusMinus; } }

         public Node NextSibling
         {
            get
            {
               if (mParent == null)
               {
                  return null;
               }
               else
               {
                  int index = mParent.IndexOfChildOrThrow(this);
                  if (index == mParent.mChildren.Count)
                  {
                     return null;
                  }
                  else
                  {
                     return mParent.mChildren[index + 1];
                  }
               }
            }
         }

         public Node Parent
         {
            get { return mParent; }
         }

         public bool Expanded
         {
            get { return mExpanded; }
            set
            {
               if (value != mExpanded)
               {
                  mExpanded = value;

                  if (IsExposed)
                  {
                     if (value)
                     {
                        Internal_ExposeChildren();
                     }
                     else
                     {
                        Stack<Node> collapsed = new Stack<Node>();
                        Internal_HideChildren( ref collapsed );

                        while (collapsed.Count > 0)
                        {
                           Node n = collapsed.Pop();
                           mView.SendEvent_PostCollapse(n);
                        }
                     }
                  }

               }
            }
         }

         public int IndentLevel
         {
            get { return mIndentLevel; }
         }

         public Object Tag = null;

         public ListViewItem ListItem
         {
            get { return mItem; }
         }

#endregion

#region "Private Variables"
         static List<Node> skNoChildren = new List<Node>();
         TreeListView mView = null;
         Node mParent = null;
         ListViewItem mItem = null;
         List<Node> mChildren = new List<Node>();
         bool mExpanded;
         bool mAlwaysShowPlusMinus = false;
         int mIndentLevel = -1;
#endregion
      }

      public enum EHitTestTypes
      {
         kIcon = 1,
         kLabel = 2,
         kExpandCollapse = 4,
         kStateImage = 8,
         kIndent = 16,
         kClientArea_Above = 32,
         kClientArea_Below = 64,
         kClientArea_Left = 128,
         kClientArea_Right = 256
      };

      public class HitTestInfo
      {
         public HitTestInfo(ListViewItem item, ListViewItem.ListViewSubItem subItem, EHitTestTypes hitTestType)
         {
            if (item != null)
            {
               this.Node = (Node)item.Tag;
            }
            else
            {
               this.Node = null;
            }

            this.ListViewItem = item;
            this.ListViewSubItem = subItem;
            this.HitTestType = hitTestType;
         }

         readonly public Node Node;
         readonly public ListViewItem ListViewItem;
         readonly public ListViewItem.ListViewSubItem ListViewSubItem;
         readonly public EHitTestTypes HitTestType;
      }

      public TreeListView()
         : base()
      {
         mPlusMinusImages = new System.Windows.Forms.ImageList();
         mPlusMinusImages.ColorDepth = System.Windows.Forms.ColorDepth.Depth24Bit;
         mPlusMinusImages.ImageSize = new System.Drawing.Size(16, 16);
         mPlusMinusImages.TransparentColor = System.Drawing.Color.Magenta;

         mPlusMinusImages.Images.Add("Minus", Tools.Properties.Resources.TreeMinus);
         mPlusMinusImages.Images.Add("Plus", Tools.Properties.Resources.TreePlus);

         SmallImageList = mPlusMinusImages;

         mRoot = new Node();
         mRoot.SetAsRoot( this );
         OwnerDraw = true;
         DoubleBuffered = true;
      }

#region "Private and Internal Functions"
      internal bool HasSmallImageList()
      {
         return (SmallImageList != mPlusMinusImages);
      }

      private void DrawImageInList(Graphics g, Rectangle bounds, ImageList list, int index, bool selected)
      {
         if (index == -1)
         {
            return;
         }

         Size imageSize = new Size(
            Math.Min(bounds.Width, list.ImageSize.Width),
            Math.Min(bounds.Height, list.ImageSize.Height));

         if (imageSize.Width <= 0 || imageSize.Height <= 0)
         {
            return;
         }

         IntPtr hdc = g.GetHdc();
         try
         {
            Tools.Common.WindowsUserNativeSafe.ImageList_DrawEx(
               new HandleRef(list, list.Handle), index,
               new HandleRef(g, hdc), bounds.X, bounds.Y, imageSize.Width, imageSize.Height,
                  Tools.Common.WindowsUserNative.CLR_NONE, Tools.Common.WindowsUserNative.CLR_DEFAULT,
                  selected ? Tools.Common.WindowsUserNative.ILD_BLEND50 : Tools.Common.WindowsUserNative.ILD_NORMAL);
         }
         finally
         {
            g.ReleaseHdcInternal(hdc);
         }

      }

      private Rectangle GetPlusMinusRect(Node node)
      {
         Rectangle imageBounds = node.ListItem.GetBounds(ItemBoundsPortion.Icon);

         // Move back by the state image if we've got it
         if (StateImageList != null)
         {
            imageBounds.Offset(-StateImageList.ImageSize.Width, 0);
         }

         if (HasSmallImageList())
         {
            // Move back by the main image size
            imageBounds.Offset(-SmallImageList.ImageSize.Width, 0);
         }

         return imageBounds;
      }

      private void DrawSubItem_Default(DrawListViewSubItemEventArgs e)
      {
         Node node = (Node)e.Item.Tag;

         // Let's draw our images now
         if (node.HasChildren || node.AlwaysShowPlusMinus)
         {
            DrawImageInList(e.Graphics, GetPlusMinusRect(node), mPlusMinusImages, node.Expanded ? 0 : 1, false);
            //               mPlusMinusImages.Draw(e.Graphics, new Point(metrics.mPlusMinusRect.X, metrics.mPlusMinusRect.Y), node.Expanded ? 0 : 1);
         }

         if (SmallImageList != mPlusMinusImages)
         {
            DrawImageInList(e.Graphics, e.Item.GetBounds(ItemBoundsPortion.Icon), SmallImageList, e.Item.ImageIndex, e.Item.Selected);
         }

         Rectangle labelRect = e.Item.GetBounds(ItemBoundsPortion.Label);
         Rectangle iconPlusLabelRect = e.Item.GetBounds(ItemBoundsPortion.ItemOnly);

         Rectangle backgroundRect = new Rectangle(labelRect.X, labelRect.Y, iconPlusLabelRect.Right - labelRect.X, labelRect.Height);
         if (FullRowSelect)
         {
            backgroundRect.Width = e.Bounds.Right - backgroundRect.Left;
         }

         if (e.Item.Selected)
         {
            e.Graphics.FillRectangle(SystemBrushes.Highlight, backgroundRect);
         }
         else
         {
            using (Brush b = new SolidBrush(e.Item.BackColor))
            {
               e.Graphics.FillRectangle(b, backgroundRect);
            }
         }

         TextFormatFlags drawFlags =
            TextFormatFlags.GlyphOverhangPadding |
            TextFormatFlags.VerticalCenter |
            TextFormatFlags.EndEllipsis;

         TextRenderer.DrawText(e.Graphics, e.Item.Text, e.Item.Font, backgroundRect,
            e.Item.Selected ? SystemColors.HighlightText : e.Item.ForeColor,
            drawFlags);

         if (e.Item.Focused)
         {
            ControlPaint.DrawFocusRectangle(e.Graphics, backgroundRect);
         }
      }

      static internal void MoveAndShrinkRectangleX(ref Rectangle r, int amt)
      {
         r.X += amt;
         r.Width -= amt;
      }

      private void SendEvent_UserExpandChange(Node n, bool expandState)
      {
         NodeCancelableEventArgs args = new NodeCancelableEventArgs(n);

         if (expandState && UserExpanding != null)
         {
            UserExpanding(this, args);
         }
         else if (!expandState && UserCollapsing != null)
         {
            UserCollapsing(this, args);
         }

         if (args.DoDefault)
         {
            // If our event handler did nothing, then set the expanded state

            n.Expanded = expandState;
         }
      }

      internal void SendEvent_PreExposeChildren(Node n)
      {
         NodeEventArgs args = new NodeEventArgs(n);

         if (PreExposeNodeChildrenIntoView != null)
         {
            PreExposeNodeChildrenIntoView(this, args);
         }
      }

      internal void SendEvent_PostCollapse(Node n)
      {
         NodeEventArgs args = new NodeEventArgs(n);

         if (PostHideNodeChildrenFromView != null)
         {
            PostHideNodeChildrenFromView(this, args);
         }
      }

      internal void Internal_ExposeChildrenStart(Node n)
      {
         if (mInternallyExpanding != null )
         {
            throw new Exception("Tried to expand a node while already expanding");
         }

         mInternallyExpanding = n;
         SendEvent_PreExposeChildren(n);
      }

      internal void Internal_ExposeChildrenEnd(Node n)
      {
         if (mInternallyExpanding != n)
         {
            throw new Exception("Internal_ExpandEnd called on node that wasn't expanding?!");
         }

         mInternallyExpanding = null;
      }

      internal bool ShouldManipulateNodeParentInListView(Node nodeParent )
      {
         // Check if we should manipulate this node in the list view.  If we're not expanding anything,
         // then we should be able to.  If we're adding a child of what we're expanding, then we cannot.
         // If we're expanding and adding something that's not a child, well, that's just wrong

         if (mInternallyExpanding == null)
         {
            return true;
         }

         for (Node checking = nodeParent; checking != null; checking = checking.Parent)
         {
            if (checking == mInternallyExpanding)
            {
               // We're a child of what's expanding.  This means that we're manipulating a node
               // in a valid way, we just shouldn't touch the list view

               return false;
            }
         }

         // If we've hit here, this means that we're expanding a node and trying to manipulate a node
         // outside of that tree.  That's a no-no
         throw new Exception("A node is in the middle of expanding, and a node that's not a child of it is being manipulated");
      }

#endregion

      #region "Overrides"

      protected override void Dispose(bool disposing)
      {
         if (disposing)
         {
            mPlusMinusImages.Dispose();
         }
         
         base.Dispose(disposing);
      }

      protected override void OnDrawColumnHeader(DrawListViewColumnHeaderEventArgs e)
      {
         e.DrawDefault = true;
         base.OnDrawColumnHeader(e);
      }

      protected override void OnDrawSubItem(DrawListViewSubItemEventArgs e)
      {
         e.DrawDefault = true;
         base.OnDrawSubItem(e);

         if (e.DrawDefault)
         {
            // If we're still drawing default, then call "our" default for subitem 0
            if (e.SubItem != null && e.SubItem == e.Item.SubItems[0])
            {
               DrawSubItem_Default(e);
               e.DrawDefault = false;
            }
         }
      }

      protected override void OnMouseDown(MouseEventArgs e)
      {
         TreeListView.HitTestInfo info = HitTest(e.Location);

         if (info != null)
         {
            if ((info.HitTestType & TreeListView.EHitTestTypes.kExpandCollapse) != 0)
            {
               SendEvent_UserExpandChange(info.Node, !info.Node.Expanded);
               return;
            }
         }

         base.OnMouseDown(e);
      }

      protected override void OnKeyDown(KeyEventArgs e)
      {
         if (e.KeyCode == Keys.Add || e.KeyCode == Keys.Subtract)
         {
            e.Handled = true;

            // Only expand/collapse the focused now
            Node node = NodeFromListItem(FocusedItem);

            if (node != null)
            {
               bool wantsExpand = (e.KeyCode == Keys.Add);

               if (node.Expanded != wantsExpand)
               {
                  SendEvent_UserExpandChange(node, wantsExpand);
               }
            }
         }
         else if (e.KeyCode == Keys.Left)
         {
            // First collapse, then move to parent

            e.Handled = true;

            Node node = NodeFromListItem(FocusedItem);

            if (node != null)
            {
               if (node.Expanded && node.IsShowingPlusMinus)
               {
                  SendEvent_UserExpandChange(node, false);
               }
               else
               {
                  Node parent = node.Parent;
                  if (parent != Root )
                  {
                     FocusedItem = parent.ListItem;

                     if (!e.Control & !e.Shift)
                     {
                        SelectedItems.Clear();
                        FocusedItem.Selected = true;
                     }
                  }
               }
            }
         }
         else if (e.KeyCode == Keys.Right)
         {
            // First expand, then move to first child
            e.Handled = true;
            Node node = NodeFromListItem(FocusedItem);

            if (node != null)
            {
               if (!node.Expanded && node.IsShowingPlusMinus)
               {
                  SendEvent_UserExpandChange(node, true);
               }
               else if (node.Expanded && node.HasChildren)
               {
                  Node child = node.FirstChild;

                  FocusedItem = child.ListItem;

                  if (!e.Control && !e.Shift)
                  {
                     SelectedItems.Clear();
                     FocusedItem.Selected = true;
                  }
               }
            }
         }
         else
         {
            base.OnKeyDown(e);
         }
      }

#endregion

#region "Public Properties"
      public Node Root
      {
         get { return mRoot; }
      }

#endregion

#region "Public Functions"
      public Node NodeFromListItem(ListViewItem item)
      {
         if (item == null)
         {
            return null;
         }
         else
         {
            return (Node)item.Tag;
         }
      }

      public new HitTestInfo HitTest(Point p)
      {
         ListViewHitTestInfo info = base.HitTest(p);

         if (info == null)
         {
            return null;
         }

         EHitTestTypes hitTestType = 0;

         if ((info.Location & ListViewHitTestLocations.AboveClientArea) != 0)
         {
            hitTestType |= EHitTestTypes.kClientArea_Above;
         }
         if ((info.Location & ListViewHitTestLocations.BelowClientArea) != 0)
         {
            hitTestType |= EHitTestTypes.kClientArea_Below;
         }
         if ((info.Location & ListViewHitTestLocations.LeftOfClientArea) != 0)
         {
            hitTestType |= EHitTestTypes.kClientArea_Left;
         }
         if ((info.Location & ListViewHitTestLocations.RightOfClientArea) != 0)
         {
            hitTestType |= EHitTestTypes.kClientArea_Right;
         }

         bool inheritFlags = true;
         // Drill down and see if we need to override the hit test 
         // stuff
         if (info.Item != null)
         {
            Node hitNode = (Node)info.Item.Tag;
            if (info.SubItem == info.Item.SubItems[0])
            {
               Rectangle pmRect = GetPlusMinusRect(hitNode);

               // Ok, here if we're in the expand/collapse or indent regions,
               // then return those hit test types.  Otherwise, do default behavior
               inheritFlags = false;
               if (pmRect.Contains(p) && hitNode.IsShowingPlusMinus )
               {
                  hitTestType |= EHitTestTypes.kExpandCollapse;
               }
               else if (p.X < pmRect.Left)
               {
                  hitTestType |= EHitTestTypes.kIndent;
               }
               else
               {
                  inheritFlags = true;
               }
            }

         }

         if (inheritFlags)
         {
            if ((info.Location & ListViewHitTestLocations.Label) != 0)
            {
               hitTestType |= EHitTestTypes.kLabel;
            }

            if ((info.Location & ListViewHitTestLocations.Image) != 0)
            {
               hitTestType |= EHitTestTypes.kIcon;
            }
         }

         return new HitTestInfo(info.Item, info.SubItem, hitTestType);
      }

      public new HitTestInfo HitTest(int x, int y)
      {
         return HitTest(new Point(x, y));
      }
#endregion

#region "Events"
      public class NodeEventArgs : System.EventArgs
      {
         public NodeEventArgs(Node n)
         {
            this.Node = n;
         }

         public readonly Node Node;
      }

      public class NodeCancelableEventArgs : NodeEventArgs
      {
         public NodeCancelableEventArgs(Node n)
            : base( n )
         {
            mDoDefault = true;
         }

         public void CancelDefaultProcessing() { mDoDefault = false; }
         public bool DoDefault { get { return mDoDefault; } }
         private bool mDoDefault;
      }

      public delegate void NodeEvent(object sender, NodeEventArgs args);
      public delegate void NodeCancelableEvent(object sender, NodeCancelableEventArgs args);

      public event NodeCancelableEvent UserExpanding;
      public event NodeCancelableEvent UserCollapsing;
      public event NodeEvent PreExposeNodeChildrenIntoView;
      public event NodeEvent PostHideNodeChildrenFromView;
#endregion

#region "Private Variables"
      private ImageList mPlusMinusImages;
      private Node mInternallyExpanding = null;

      Node mRoot;
#endregion
   }
}
