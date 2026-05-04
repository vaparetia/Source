using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace Tools.Controls
{
   public class TreeNodeEx : TreeNode
   {
      public TreeNodeEx()
      {
      }

      int mOverlayBitmask = 0;
      public int OverlayBitmask
      {
         get { return mOverlayBitmask; }
         set 
         { 
            mOverlayBitmask = value; 
            
            if( TreeView != null )
            {
               TreeView.Invalidate(); 
            }
         }
      }

      string mStatusImageKey;
      public string StatusImageKey
      {
         get { return mStatusImageKey; }
         set 
         { 
            mStatusImageKey = value;

            if (TreeView != null)
            {
               TreeView.Invalidate();
            }
         }
      }

      public override object Clone()
      {
         TreeNodeEx cloned = base.Clone() as TreeNodeEx;
         
         cloned.OverlayBitmask = OverlayBitmask;
         cloned.StatusImageKey = StatusImageKey;

         return cloned;
      }
   }

	public class MultiSelectTreeView : TreeView
	{
		#region Selected Node(s) Properties

		private List<TreeNode> m_SelectedNodes = null;		
		public List<TreeNode> SelectedNodes
		{
			get
			{
				return m_SelectedNodes;
			}
			set
			{
				ClearSelectedNodes();
				if( value != null )
				{
					foreach( TreeNode node in value )
					{
						ToggleNode( node, true );
					}
				}
            OnAfterSelect(null);
			}
		}

		// Note we use the new keyword to Hide the native treeview's SelectedNode property.
		private TreeNode m_SelectedNode;
		public new TreeNode SelectedNode
		{
			get { return m_SelectedNode; }
			set
			{
				ClearSelectedNodes();
				if( value != null )
				{
					SelectNode( value );
				}
            OnAfterSelect(null);
         }
		}

		#endregion

      ImageList mOverlayImageList;
      public ImageList OverlayImageList
      {
         get { return mOverlayImageList; }
         set { mOverlayImageList = value;}
      }

      ImageList mStatusImageList;
      public ImageList StatusImageList
      {
         get { return mStatusImageList; }
         set { mStatusImageList = value; Indent = Indent; }
      }

      int mIndent = 19;

      public new int Indent
      {
         get { return mIndent; }
         set { mIndent = value; }
      }

		public MultiSelectTreeView()
		{
			m_SelectedNodes = new List<TreeNode>();
			base.SelectedNode = null;
		}

		#region Overridden Events

      protected override void  OnLayout(LayoutEventArgs levent)
      {
         base.Indent = mIndent + ((mStatusImageList != null) ? mStatusImageList.ImageSize.Width : 0);
 	      base.OnLayout(levent);
      }

      [StructLayout(LayoutKind.Sequential)]
      public struct RECT
      {
         public int Left;
         public int Top;
         public int Right;
         public int Bottom;
      }

      [StructLayout(LayoutKind.Sequential)]
      struct NMHDR
      {
         public IntPtr hwndFrom;
         public IntPtr idFrom;
         public int code;
      }

      [StructLayout(LayoutKind.Sequential)]
      struct NMCUSTOMDRAW
      {
         public NMHDR hdr;
         public int dwDrawStage;
         public IntPtr hdc;
         public RECT rc;
         public int dwItemSpec;
         public int uItemState;
         public int lItemlParam;
      }

      [StructLayout(LayoutKind.Sequential)]
      struct NMTVCUSTOMDRAW
      {
         public NMCUSTOMDRAW nmcd;
         public int clrText;
         public int clrTextBk;
         public int iLevel;
      }

      protected override void WndProc(ref Message m)
      {
         const int WM_NOTIFY = 0x004E;

         bool handled = false;

         if (m.Msg == (WM_NOTIFY | 0x2000) )
         {
            if( m.WParam == Handle )
            {
               int result = HandleNotify(ref m);
               if( result != -1 )
               {
                  m.Result = new IntPtr(result);
                  handled = true; 
               }
            }
         }

         if( !handled )
         {
            base.WndProc(ref m);
         }
      }

      private int HandleNotify(ref Message m)
      {
         const int NM_FIRST = 0;
         const int NM_CUSTOMDRAW = (NM_FIRST - 12);

         // Drawstage flags
         const int CDDS_PREPAINT = 1;
         const int CDDS_POSTPAINT = 2;
         //const int CDDS_PREERASE = 3;
         //const int CDDS_POSTERASE = 4;

         const int CDDS_ITEM = 0x10000;
         const int CDDS_ITEMPREPAINT = CDDS_ITEM | CDDS_PREPAINT;
         const int CDDS_ITEMPOSTPAINT = CDDS_ITEM | CDDS_POSTPAINT;
         //const int CDDS_ITEMPREERASE = CDDS_ITEM | CDDS_PREERASE;
         //const int CDDS_ITEMPOSTERASE = CDDS_ITEM | CDDS_POSTERASE;
         //const int CDDS_SUBITEM = 0x20000;

         // Custom draw return flags
         const int CDRF_DODEFAULT = 0;
         //const int CDRF_NEWFONT = 2;
         //const int CDRF_SKIPDEFAULT = 4;
         const int CDRF_NOTIFYPOSTPAINT= 0x10;
         const int CDRF_NOTIFYITEMDRAW = 0x20;
         //const int CDRF_NOTIFYSUBITEMDRAW = 0x20;
         //const int CDRF_NOTIFYPOSTERASE = 0x40;

         object lparam = m.GetLParam(typeof(NMHDR));

         if( lparam is NMHDR)
         {
            NMHDR nmhdr = (NMHDR)lparam;

            if (nmhdr.code == NM_CUSTOMDRAW)
            {
               lparam = Marshal.PtrToStructure(m.LParam, typeof(NMTVCUSTOMDRAW));

               if (lparam is NMTVCUSTOMDRAW)
               {
                  NMTVCUSTOMDRAW tvcd = (NMTVCUSTOMDRAW)lparam;

                  switch (tvcd.nmcd.dwDrawStage)
                  {
                     case CDDS_PREPAINT:
                        return CDRF_NOTIFYITEMDRAW;
                        
                     case CDDS_ITEMPREPAINT:
                        {
                           TreeNode node = TreeNode.FromHandle(this, new IntPtr(tvcd.nmcd.dwItemSpec));

                           if( SelectedNodes.Contains(node) )
                           {
                              tvcd.clrTextBk = ColorTranslator.ToWin32(SystemColors.Highlight);
                              tvcd.clrText = ColorTranslator.ToWin32(SystemColors.HighlightText);
                           }
                           else
                           {
                              if( !node.BackColor.IsEmpty )
                              {
                                 tvcd.clrTextBk = ColorTranslator.ToWin32(node.BackColor);
                              }
                              
                              if( !node.ForeColor.IsEmpty )
                              {
                                 tvcd.clrText = ColorTranslator.ToWin32(node.ForeColor);
                              }
                           }

                           Marshal.StructureToPtr(tvcd, m.LParam, false);

                           return CDRF_NOTIFYPOSTPAINT;
                        }

                     case CDDS_ITEMPOSTPAINT:
                        {
                           TreeNode node = TreeNode.FromHandle(this, new IntPtr(tvcd.nmcd.dwItemSpec));
                           using(Graphics graphics = Graphics.FromHdc(tvcd.nmcd.hdc))
                           {
                              PaintTreeNode(node, graphics);
                           }

                           return CDRF_DODEFAULT;
                        }
                  }
               }
            }
         }

         return -1;
      }

      private void PaintTreeNode(TreeNode node, Graphics graphics)
      {
         if( node.IsVisible )
         {
            TreeNodeEx nodeEx = node as TreeNodeEx;

            if( nodeEx != null )
            {
               int nodeLeft = node.Bounds.Left;
               int imageTop = node.Bounds.Top + (ItemHeight - 16) / 2;

               if( StatusImageList != null )
               {
                  Image statusImage = StatusImageList.Images[nodeEx.StatusImageKey];
                  if (statusImage != null)
                  {
                     graphics.DrawImage(statusImage, nodeLeft - 20 - StatusImageList.ImageSize.Width, imageTop);
                  }
               }

               if( OverlayImageList != null && nodeEx.OverlayBitmask != 0 )
               {
                  int overlayMask = nodeEx.OverlayBitmask;
                  int imageBit = 1;

                  foreach(Image image in OverlayImageList.Images)
                  {
                     if( (overlayMask & imageBit) != 0 )
                     {
                        graphics.DrawImage(image, nodeLeft - 20, imageTop);
                     }

                     imageBit <<= 1;
                  }
               }
            }
         }
      }

		protected override void OnGotFocus( EventArgs e )
		{
			// Make sure at least one node has a selection
			// this way we can tab to the ctrl and use the 
			// keyboard to select nodes
			try
			{
				if( m_SelectedNode == null && this.TopNode != null )
				{
					ToggleNode( this.TopNode, true );
				}

				base.OnGotFocus( e );
			}
			catch( Exception ex )
			{
				HandleException( ex );
			}
		}

		protected override void OnMouseDown( MouseEventArgs e )
		{
			// If the user clicks on a node that was not
			// previously selected, select it now.

         try
         {
            if (e.Button == MouseButtons.Left || e.Button == MouseButtons.Right)
            {
               base.SelectedNode = null;

               TreeNode node = this.GetNodeAt(e.Location);
               if (node != null)
               {
                  if (ModifierKeys == Keys.None && (m_SelectedNodes.Contains(node)))
                  {
                     // Potential Drag Operation
                     // Let Mouse Up do select
                  }
                  else
                  {
                     SelectNode(node);
                  }
               }

               base.OnMouseDown(e);
            }

			}
			catch( Exception ex )
			{
				HandleException( ex );
			}
		}

		protected override void OnMouseUp( MouseEventArgs e )
		{
			// If the clicked on a node that WAS previously
			// selected then, reselect it now. This will clear
			// any other selected nodes. e.g. A B C D are selected
			// the user clicks on B, now A C & D are no longer selected.
			try
			{
            if (e.Button == MouseButtons.Left)
            {
               // Check to see if a node was clicked on 
               TreeNode node = this.GetNodeAt(e.Location);
               if (node != null)
               {
                  if (ModifierKeys == Keys.None && m_SelectedNodes.Count > 1 && m_SelectedNodes.Contains(node))
                  {
                     SelectNode(node);
                  }
               }
               base.OnMouseUp(e);
            }

			}
			catch( Exception ex )
			{
				HandleException( ex );
			}
		}

		protected override void OnItemDrag( ItemDragEventArgs e )
		{
			// If the user drags a node and the node being dragged is NOT
			// selected, then clear the active selection, select the
			// node being dragged and drag it. Otherwise if the node being
			// dragged is selected, drag the entire selection.
			try
			{
				TreeNode node = e.Item as TreeNode;

				if( node != null )
				{
					if( !m_SelectedNodes.Contains( node ) )
					{
						SelectSingleNode( node );
						ToggleNode( node, true );
					}
				}

				base.OnItemDrag( e );
			}
			catch( Exception ex )
			{
				HandleException( ex );
			}
		}

		protected override void OnBeforeSelect( TreeViewCancelEventArgs e )
		{
			// Never allow base.SelectedNode to be set!
			try
			{
				base.SelectedNode = null;
				e.Cancel = true;

				base.OnBeforeSelect( e );
			}
			catch( Exception ex )
			{
				HandleException( ex );
			}
		}

		protected override void OnAfterSelect( TreeViewEventArgs e )
		{
			// Never allow base.SelectedNode to be set!
			try
			{
				base.OnAfterSelect( e );
				base.SelectedNode = null;
			}
			catch( Exception ex )
			{
				HandleException( ex );
			}
		}

		protected override void OnKeyDown( KeyEventArgs e )
		{
			// Handle all possible key strokes for the control.
			// including navigation, selection, etc.

			base.OnKeyDown( e );

         // filter out a couple of keys since those pressed by themselfs don't cause any action.
         switch(e.KeyCode)
         {
            case Keys.ShiftKey:
            case Keys.ControlKey:
               return;
         }

			this.BeginUpdate();
			bool bShift = ( ModifierKeys == Keys.Shift );

			try
			{
				// Nothing is selected in the tree, this isn't a good state
				// select the top node
				if( m_SelectedNode == null && this.TopNode != null )
				{
					ToggleNode( this.TopNode, true );
				}

				// Nothing is still selected in the tree, this isn't a good state, leave.
				if( m_SelectedNode == null ) return;

				if( e.KeyCode == Keys.Left || e.KeyCode == Keys.Subtract )
				{
					if( m_SelectedNode.IsExpanded && m_SelectedNode.Nodes.Count > 0 )
					{
						// Collapse an expanded node that has children
						m_SelectedNode.Collapse();
					}
					else if( m_SelectedNode.Parent != null )
					{
                  if (e.KeyCode == Keys.Left)
                  {
                     // Node is already collapsed, try to select its parent.
                     SelectSingleNode(m_SelectedNode.Parent);
                  }
					}
				}
            else if (e.KeyCode == Keys.Right || e.KeyCode == Keys.Add)
				{
					if( !m_SelectedNode.IsExpanded )
					{
						// Expand a collpased node's children
						m_SelectedNode.Expand();
					}
					else
					{
                  if (e.KeyCode == Keys.Right)
                  {
                     // Node was already expanded, select the first child
                     SelectSingleNode(m_SelectedNode.FirstNode);
                  }
					}
				}
				else if( e.KeyCode == Keys.Up )
				{
					// Select the previous node
					if( m_SelectedNode.PrevVisibleNode != null )
					{
						SelectNode( m_SelectedNode.PrevVisibleNode );
					}
				}
				else if( e.KeyCode == Keys.Down )
				{
					// Select the next node
					if( m_SelectedNode.NextVisibleNode != null )
					{
						SelectNode( m_SelectedNode.NextVisibleNode );
					}
				}
				else if( e.KeyCode == Keys.Home )
				{
					if( bShift )
					{
						if( m_SelectedNode.Parent == null )
						{
							// Select all of the root nodes up to this point 
							if( this.Nodes.Count > 0 )
							{
								SelectNode( this.Nodes[0] );
							}
						}
						else
						{
							// Select all of the nodes up to this point under this nodes parent
							SelectNode( m_SelectedNode.Parent.FirstNode );
						}
					}
					else
					{
						// Select this first node in the tree
						if( this.Nodes.Count > 0 )
						{
							SelectSingleNode( this.Nodes[0] );
						}
					}
				}
				else if( e.KeyCode == Keys.End )
				{
					if( bShift )
					{
						if( m_SelectedNode.Parent == null )
						{
							// Select the last ROOT node in the tree
							if( this.Nodes.Count > 0 )
							{
								SelectNode( this.Nodes[this.Nodes.Count - 1] );
							}
						}
						else
						{
							// Select the last node in this branch
							SelectNode( m_SelectedNode.Parent.LastNode );
						}
					}
					else
					{
						if( this.Nodes.Count > 0 )
						{
							// Select the last node visible node in the tree.
							// Don't expand branches incase the tree is virtual
							TreeNode ndLast = this.Nodes[this.Nodes.Count - 1];
							while( ndLast.IsExpanded && ( ndLast.LastNode != null ) )
							{
								ndLast = ndLast.LastNode;
							}
							SelectSingleNode( ndLast );
						}
					}
				}
				else if( e.KeyCode == Keys.PageUp )
				{
					// Select the highest node in the display
					int nCount = this.VisibleCount;
					TreeNode ndCurrent = m_SelectedNode;
					while( ( nCount ) > 0 && ( ndCurrent.PrevVisibleNode != null ) )
					{
						ndCurrent = ndCurrent.PrevVisibleNode;
						nCount--;
					}
					SelectSingleNode( ndCurrent );
				}
				else if( e.KeyCode == Keys.PageDown )
				{
					// Select the lowest node in the display
					int nCount = this.VisibleCount;
					TreeNode ndCurrent = m_SelectedNode;
					while( ( nCount ) > 0 && ( ndCurrent.NextVisibleNode != null ) )
					{
						ndCurrent = ndCurrent.NextVisibleNode;
						nCount--;
					}
					SelectSingleNode( ndCurrent );
				}
				else
				{
					// Assume this is a search character a-z, A-Z, 0-9, etc.
					// Select the first node after the current node that 
					// starts with this character
					string sSearch = ( (char) e.KeyValue ).ToString();

					TreeNode ndCurrent = m_SelectedNode;
					while( ( ndCurrent.NextVisibleNode != null ) )
					{
						ndCurrent = ndCurrent.NextVisibleNode;
						if( ndCurrent.Text.StartsWith( sSearch ) )
						{
							SelectSingleNode( ndCurrent );
							break;
						}
					}
				}
			}
			catch( Exception ex )
			{
				HandleException( ex );
			}
			finally
			{
				this.EndUpdate();
			}
		}

		#endregion

		#region Helper Methods

		private void SelectNode( TreeNode node )
		{
			try
			{
				this.BeginUpdate();

            List<TreeNode> oldSelection = new List<TreeNode>(m_SelectedNodes);

				if( m_SelectedNode == null || ModifierKeys == Keys.Control )
				{
					// Ctrl+Click selects an unselected node, or unselects a selected node.
					bool bIsSelected = m_SelectedNodes.Contains( node );
					ToggleNode( node, !bIsSelected );
				}
				else if( ModifierKeys == Keys.Shift )
				{
					// Shift+Click selects nodes between the selected node and here.
					TreeNode ndStart = m_SelectedNode;
					TreeNode ndEnd = node;

					if( ndStart.Parent == ndEnd.Parent )
					{
						// Selected node and clicked node have same parent, easy case.
						if( ndStart.Index < ndEnd.Index )
						{							
							// If the selected node is beneath the clicked node walk down
							// selecting each Visible node until we reach the end.
							while( ndStart != ndEnd )
							{
								ndStart = ndStart.NextVisibleNode;
								if( ndStart == null ) break;
								ToggleNode( ndStart, true );
							}
						}
						else if( ndStart.Index == ndEnd.Index )
						{
							// Clicked same node, do nothing
						}
						else
						{
							// If the selected node is above the clicked node walk up
							// selecting each Visible node until we reach the end.
							while( ndStart != ndEnd )
							{
								ndStart = ndStart.PrevVisibleNode;
								if( ndStart == null ) break;
								ToggleNode( ndStart, true );
							}
						}
					}
					else
					{
						// Selected node and clicked node have same parent, hard case.
						// We need to find a common parent to determine if we need
						// to walk down selecting, or walk up selecting.

						TreeNode ndStartP = ndStart;
						TreeNode ndEndP = ndEnd;
						int startDepth = Math.Min( ndStartP.Level, ndEndP.Level );

						// Bring lower node up to common depth
						while( ndStartP.Level > startDepth )
						{
							ndStartP = ndStartP.Parent;
						}

						// Bring lower node up to common depth
						while( ndEndP.Level > startDepth )
						{
							ndEndP = ndEndP.Parent;
						}

						// Walk up the tree until we find the common parent
						while( ndStartP.Parent != ndEndP.Parent )
						{
							ndStartP = ndStartP.Parent;
							ndEndP = ndEndP.Parent;
						}

						// Select the node
						if( ndStartP.Index < ndEndP.Index )
						{
							// If the selected node is beneath the clicked node walk down
							// selecting each Visible node until we reach the end.
							while( ndStart != ndEnd )
							{
								ndStart = ndStart.NextVisibleNode;
								if( ndStart == null ) break;
								ToggleNode( ndStart, true );
							}
						}
						else if( ndStartP.Index == ndEndP.Index )
						{
							if( ndStart.Level < ndEnd.Level )
							{
								while( ndStart != ndEnd )
								{
									ndStart = ndStart.NextVisibleNode;
									if( ndStart == null ) break;
									ToggleNode( ndStart, true );
								}
							}
							else
							{
								while( ndStart != ndEnd )
								{
									ndStart = ndStart.PrevVisibleNode;
									if( ndStart == null ) break;
									ToggleNode( ndStart, true );
								}
							}
						}
						else
						{
							// If the selected node is above the clicked node walk up
							// selecting each Visible node until we reach the end.
							while( ndStart != ndEnd )
							{
								ndStart = ndStart.PrevVisibleNode;
								if( ndStart == null ) break;
								ToggleNode( ndStart, true );
							}
						}
					}
				}
				else
				{
					// Just clicked a node, select it
					SelectSingleNode( node );
				}

            bool selectionChanged = false;

            if( oldSelection.Count != m_SelectedNodes.Count )
            {
               selectionChanged = true;
            }
            else
            {
               for( int i = 0; i < oldSelection.Count; ++i)
               {
                  if( oldSelection[i] != m_SelectedNodes[i] )
                  {
                     selectionChanged = true;
                     break;
                  }
               }
            }

            if (selectionChanged)
            {
               OnAfterSelect(new TreeViewEventArgs(m_SelectedNode));
            }
			}
			finally
			{
				this.EndUpdate();
			}
		}

		private void ClearSelectedNodes()
		{
			m_SelectedNodes.Clear();
			m_SelectedNode = null;
		}

		private void SelectSingleNode( TreeNode node )
		{
			if( node == null )
			{
				return;
			}

			ClearSelectedNodes();
			ToggleNode( node, true );
			node.EnsureVisible();
		}

		private void ToggleNode( TreeNode node, bool bSelectNode )
		{
			if( bSelectNode )
			{
				m_SelectedNode = node;

            if( !m_SelectedNodes.Contains( node ) )
				{
					m_SelectedNodes.Add( node );
				}
			}
			else
			{
				m_SelectedNodes.Remove( node );
			}
		}

		private void HandleException( Exception ex )
		{
			// Perform some error handling here.
			// We don't want to bubble errors to the CLR. 
			MessageBox.Show( ex.Message );
		}

		#endregion
	}
}
