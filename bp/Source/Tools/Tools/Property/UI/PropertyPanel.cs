//#define DEBUG_CLIENT_RECTS
//#define PROFILE_DRAW_TIME
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.VisualStyles;
using System.Xml;
using System.Runtime.InteropServices;
using Tools.Property.UI.Controls;
using Tools.Common;

namespace Tools.Property.UI
{
   public partial class PropertyPanel : Control
   {
      #region Mouse Hook support
      internal interface IMouseHookClient
      {
         bool OnClickHooked();
      }

      internal class MouseHook
      {
         private IMouseHookClient client;
         private Control control;
         private PropertyPanel propertyPanel;
         private bool hookDisable;
         private IntPtr mouseHookHandle = IntPtr.Zero;
         private GCHandle mouseHookRoot;
         private bool processing;
         internal int thisProcessID;

         public MouseHook(Control control, IMouseHookClient client, PropertyPanel propertyPanel)
         {
            this.control = control;
            this.propertyPanel = propertyPanel;
            this.client = client;
         }

         public void Dispose()
         {
            UnhookMouse();
         }

         private void HookMouse()
         {
            GC.KeepAlive(this);

            lock (this)
            {
               if (mouseHookHandle == IntPtr.Zero)
               {
                  if (thisProcessID == 0)
                  {
                     WindowsUserNativeSafe.GetWindowThreadProcessId(new HandleRef(control, control.Handle), out thisProcessID);
                  }

                  MouseHookObject obj1 = new MouseHookObject(this);
                  WindowsUserNative.HookProc proc = new WindowsUserNative.HookProc(obj1.Callback);
                  mouseHookRoot = GCHandle.Alloc(proc);
                  mouseHookHandle = WindowsUserUnsafe.SetWindowsHookEx(7, proc, WindowsUserNative.NullHandleRef, WindowsUserNativeSafe.GetCurrentThreadId());
               }
            }
         }

         private IntPtr MouseHookProc(int nCode, IntPtr wparam, IntPtr lparam)
         {
            GC.KeepAlive(this);

            if (nCode == 0)
            {
               WindowsUserNative.MOUSEHOOKSTRUCT mousehookstruct = (WindowsUserNative.MOUSEHOOKSTRUCT)WindowsUserUnsafe.PtrToStructure(lparam, typeof(WindowsUserNative.MOUSEHOOKSTRUCT));
               if (mousehookstruct != null)
               {
                  switch (((int)wparam))
                  {
                     case WindowsUserNative.WM_MOUSEACTIVATE:
                     case WindowsUserNative.WM_NCLBUTTONDOWN:
                     case WindowsUserNative.WM_NCMBUTTONDOWN:
                     case WindowsUserNative.WM_NCRBUTTONDOWN:
                     case WindowsUserNative.WM_LBUTTONDOWN:
                     case WindowsUserNative.WM_MBUTTONDOWN:
                     case WindowsUserNative.WM_RBUTTONDOWN:
                        if (ProcessMouseDown(mousehookstruct.hWnd, mousehookstruct.pt_x, mousehookstruct.pt_y))
                        {
                           return (IntPtr)1;
                        }
                        break;
                  }
               }
            }

            return WindowsUserUnsafe.CallNextHookEx(new HandleRef(this, mouseHookHandle), nCode, wparam, lparam);
         }

         // If this function returns true, the mouse message is ignored by the system.
         private bool ProcessMouseDown(IntPtr hWnd, int x, int y)
         {
            if (!processing)
            {
               IntPtr handle = hWnd;
               IntPtr ptr2 = control.Handle;
               Control ctl = Control.FromHandle(handle);

               if ((handle != ptr2) && !control.Contains(ctl))
               {
                  int handleThreadId;
                  WindowsUserNativeSafe.GetWindowThreadProcessId(new HandleRef(null, handle), out handleThreadId);
                  if (handleThreadId != thisProcessID)
                  {
                     HookMouseDown = false;
                     return false;
                  }

                  bool flag = false;
                  flag = (ctl == null) || !propertyPanel.IsSiblingControl(control, ctl);
                  try
                  {
                     processing = true;
                     if (flag && client.OnClickHooked())
                     {
                        return true;
                     }
                  }
                  finally
                  {
                     processing = false;
                  }
                  HookMouseDown = false;
               }
            }

            return false;
         }

         private void UnhookMouse()
         {
            GC.KeepAlive(this);
            lock (this)
            {
               if (mouseHookHandle != IntPtr.Zero)
               {
                  WindowsUserUnsafe.UnhookWindowsHookEx(new HandleRef(this, mouseHookHandle));
                  mouseHookRoot.Free();
                  mouseHookHandle = IntPtr.Zero;
               }
            }
         }

         public bool DisableMouseHook
         {
            set
            {
               hookDisable = value;
               if (value)
               {
                  UnhookMouse();
               }
            }
         }

         public virtual bool HookMouseDown
         {
            get
            {
               GC.KeepAlive(this);
               return (mouseHookHandle != IntPtr.Zero);
            }
            set
            {
               if (value && !hookDisable)
               {
                  HookMouse();
               }
               else
               {
                  UnhookMouse();
               }
            }
         }

         private class MouseHookObject
         {
            internal WeakReference reference;

            public MouseHookObject(MouseHook parent)
            {
               reference = new WeakReference(parent, false);
            }

            public virtual IntPtr Callback(int nCode, IntPtr wparam, IntPtr lparam)
            {
               IntPtr zero = IntPtr.Zero;
               try
               {
                  MouseHook target = (MouseHook)reference.Target;
                  if (target != null)
                  {
                     zero = target.MouseHookProc(nCode, wparam, lparam);
                  }
               }
               catch
               {
               }
               return zero;
            }
         }
      }
#endregion

      PropertyEditorGUI mPropertyInterface;
      
      PropertyControl mRootControl;

      public PropertyPanel()
      {
         SetStyle(ControlStyles.Selectable, true);

         InitializeComponent();

         AutoSize = true;
         DoubleBuffered = true;

         Disposed += OnDisposed;
      }

      public void OnDisposed( object sender, EventArgs args )
      {
         if ( mRootControl != null )
         {
            mRootControl.Dispose();
            mRootControl = null;
         }
      }

      internal bool CommitEdits()
      {
         if( _activeControl != null )
         {
            if( _activeControl.Control.OnValidate() )
            {
               //_activeControl = null;
               return true;
            }

            return false;
         }

         return true;
      }

      internal void ShowError(Control editControl, string errorMessage)
      {
         mErrorProvider.SetIconAlignment(editControl, ErrorIconAlignment.MiddleLeft);
         mErrorProvider.SetError(editControl, errorMessage);
      }

      public override Size GetPreferredSize(Size proposedSize)
      {
         int height = 0;
         
         if( mRootControl != null )
         {
            height = mRootControl.Height;
         }

         return new Size(proposedSize.Width, height);
      }

      public bool GetControlInfoForScreenPosition(Point mousePos, out ControlInfo result)
      {
         Point clientPos = this.PointToClient(mousePos);
         return GetControlInfoForPosition(clientPos, out result);

      }
      public bool GetControlInfoForPosition(Point mousePos, out ControlInfo result)
      {
         if (mRootControl != null)
         {
            ControlContext context = new ControlContext(this, null);

            context.ClipBounds = new RectangleF(mousePos, new SizeF(1.0f, 1.0f));
            mRootControl.ProcessRequest(context, PropertyControl.Request.GetLeafControlInfo);

            result = context.LeafControlInfo;
            return (context.LeafControlInfo != null);
         }
         else
         {
            result = null;
            return false;
         }
      }

      protected override void OnKeyDown(KeyEventArgs e)
      {
         if (_activeControl != null)
         {
            _activeControl.Control.OnKeyDown(e, _activeControl);
         }

         base.OnKeyDown(e);
      }

      protected override void OnKeyUp(KeyEventArgs e)
      {
         if (_activeControl != null)
         {
            _activeControl.Control.OnKeyUp(e, _activeControl);
         }

         base.OnKeyUp(e);
      }

      protected override void OnMouseDown(MouseEventArgs e)
      {
         Select();

         ControlInfo controlInfo;
         if (GetControlInfoForPosition(e.Location, out controlInfo))
         {
            SetControlUnderMouse(controlInfo);

            // Activate control if it indeed became the current mouse control (mouse capture might prevent this).
            if( mControlUnderMouse == controlInfo )
            {
               if (controlInfo.Control.CanReceiveFocus(controlInfo))
               {
                  SetActiveControl(controlInfo, FocusReason.Mouse);
               }
               else
               {
                  SetActiveControl(null, FocusReason.Mouse);
               }
            }
         }

         ControlInfo controlForMessage = GetControlForMouseMessages();
         if (controlForMessage != null)
         {
            controlForMessage.Control.OnMouseDown(e, controlForMessage);
         }

         base.OnMouseDown(e);
      }

      protected override void OnMouseUp(MouseEventArgs e)
      {
         ControlInfo controlInfo;
         if (GetControlInfoForPosition(e.Location, out controlInfo))
         {
            SetControlUnderMouse(controlInfo);
         }

         ControlInfo controlForMessage = GetControlForMouseMessages();
         if (controlForMessage != null)
         {
            controlForMessage.Control.OnMouseUp(e, controlForMessage);
         }

         base.OnMouseUp(e);
      }

      protected override void OnMouseHover(EventArgs e)
      {
         ControlInfo controlInfo;
         if (GetControlInfoForPosition(PointToClient(Cursor.Position), out controlInfo))
         {
            SetControlUnderMouse(controlInfo);
         }

         ControlInfo controlForMessage = GetControlForMouseMessages();
         if (controlForMessage != null)
         {
            controlForMessage.Control.OnMouseHover(controlForMessage);
         }

         base.OnMouseHover(e);
      }

      protected override void OnMouseMove(MouseEventArgs e)
      {
         ControlInfo controlInfo;
         if (GetControlInfoForPosition(e.Location, out controlInfo))
         {
            SetControlUnderMouse(controlInfo);
         }

         ControlInfo controlForMessage = GetControlForMouseMessages();
         if (controlForMessage != null)
         {
            controlForMessage.Control.OnMouseMove(e, controlForMessage);
         }

#if DEBUG_CLIENT_RECTS
         Invalidate();
#endif
         base.OnMouseMove(e);
      }

      protected override void OnMouseLeave(EventArgs e)
      {
         SetControlUnderMouse(null);
         base.OnMouseLeave(e);
      }

      protected override void OnMouseWheel(MouseEventArgs e)
      {
         base.OnMouseWheel(e);
      }

      protected override void OnMouseCaptureChanged(EventArgs e)
      {
         base.OnMouseCaptureChanged(e);
         LostMouseCaptureCheck();
      }

      protected override void OnLostFocus(EventArgs e)
      {
         base.OnLostFocus(e);
         LostMouseCaptureCheck();
      }

      private void OnPaint(object sender, PaintEventArgs e)
      {
         if( mRootControl != null )
         {
            ControlContext context = new ControlContext(this, e.Graphics);
            
            context.ClipBounds = e.Graphics.ClipBounds;
            
#if PROFILE_DRAW_TIME
            using (new Tools.Common.Timer(string.Format("{0}: Draw", DateTime.Now)))
#endif
            {
               mRootControl.ProcessRequest(context, PropertyControl.Request.Draw);
            }
         }

#if DEBUG_CLIENT_RECTS
         if (mRootControl != null)
         {
            ControlContext context = new ControlContext(this, null);

            context.ClipBounds = new RectangleF(PointToClient(Cursor.Position), new SizeF(1.0f, 1.0f));
            mRootControl.ProcessRequest(context, PropertyControl.Request.GetLeafControlInfo);

            if (context.LeafControlInfo != null)
            {
               Rectangle clientRect = context.LeafControlInfo.Control.GetClientRect(this);
               e.Graphics.FillRectangle(new SolidBrush(Color.FromArgb(128, Color.White)), clientRect);
            }
         }
#endif
      }

      public PropertyControl RootControl
      {
         get { return mRootControl; }
      }

      public void SetRootControl(PropertyControl rootControl)
      {
         // Clear out active control.
         SetActiveControl(null, FocusReason.Keyboard);

         if ( mRootControl != null )
         {
            mRootControl.Dispose();
         }

         mRootControl = rootControl;
         InvalidateLayout();
      }

      internal void InvalidateLayout()
      {
         if(mRootControl != null )
         {
            using (new Tools.Common.Timer("Layout"))
            {
               // calculate heights for all controls
               mRootControl.CalculateHeights();
               // calculate locations for all controls
               mRootControl.CalculateLocations(DisplayRectangle);
            }

            Height = mRootControl.Height;
         }
         
         Invalidate();
      }

      internal void InvalidateControl(ControlInfo control)
      {
         InvalidateControl(control.Control);
      }

      internal PropertyEditorGUI PropertyInterface
      {
         get { return mPropertyInterface; }
         set { mPropertyInterface = value; }
      }

      public void InvalidateControl(PropertyControl control)
      {
         Rectangle invalidateRectangle = control.GetClientRect();
         Invalidate(invalidateRectangle);
      }

      ControlInfo _activeControl;
      internal ControlInfo ActiveControl
      {
         get { return _activeControl; }
      }

      internal void SetActiveControl(ControlInfo control, FocusReason reason)
      {
         if (_activeControl != control)
         {
            if (CommitEdits())
            {
               if (_activeControl != null)
               {
                  _activeControl.Control.OnLeave(_activeControl);
               }

               _activeControl = control;

               if (_activeControl != null)
               {
                  _activeControl.Control.OnEnter(_activeControl, reason);
               }
            }
         }
      }

      ControlInfo mControlUnderMouse;
      internal void SetControlUnderMouse(ControlInfo control)
      {
         if (mControlUnderMouse != control)
         {
            if (mControlUnderMouse != null)
            {
               mControlUnderMouse.Control.OnMouseLeave(mControlUnderMouse);
            }

            if( ShouldProcessMouseMessagesForControl(control) )
            {
               mControlUnderMouse = control;
            }
            else
            {
               mControlUnderMouse = null;
            }

            if (mControlUnderMouse != null)
            {
               if (ShouldProcessMouseMessagesForControl(mControlUnderMouse))
               {
                  mControlUnderMouse.Control.OnMouseEnter(mControlUnderMouse);
               }
            }
         }
      }
      
      ControlInfo mControlWithMouseCapture;
      public void SetCaptureMouse(ControlInfo control, bool capture)
      {
         if( capture )
         {
            LostMouseCaptureCheck();

            mControlWithMouseCapture = control;
            Capture = true;
         }
         else
         {
            mControlWithMouseCapture = null;
            Capture = false;
         }
      }

      private void LostMouseCaptureCheck()
      {
         if (mControlWithMouseCapture != null)
         {
            mControlWithMouseCapture.Control.OnLostMouseCapture(mControlWithMouseCapture);
            mControlWithMouseCapture = null;
         }
      }

      internal bool ShouldProcessMouseMessagesForControl(ControlInfo control)
      {
         if( mControlWithMouseCapture != null )
         {
            return (mControlWithMouseCapture == control);
         }

         return true;
      }
      
      internal ControlInfo GetControlForMouseMessages()
      {        
         if( mControlWithMouseCapture != null )
         {
            return mControlWithMouseCapture;
         }

         return mControlUnderMouse;
      }

      internal bool IsSiblingControl(Control c1, Control c2)
      {
         Control parentInternal = c1.Parent;
         for (Control control2 = c2.Parent; control2 != null; control2 = control2.Parent)
         {
            if (parentInternal == control2)
            {
               return true;
            }
         }
         return false;
      }

      protected override bool ProcessDialogKey(Keys keyData)
      {
         switch(keyData & Keys.KeyCode)
         {
            case Keys.Tab:
               {
                  int direction = 1;
                  
                  if( Control.ModifierKeys == Keys.Shift )
                     direction = -1;

                  return SelectNextControl(direction);
               }

            case Keys.Up:
            case Keys.Down:
               return false;
            
            default:
               break;
         }

         return base.ProcessDialogKey(keyData);
      }

      // Selects next control in the given direction (+1, -1).
      // This function will return false when there are no more controls to select in the direction given.
      private bool SelectNextControl(int direction)
      {
         ControlInfo newActiveControl = null;

         // Get a list of all selectable controls
         List<ControlInfo> controls = GetSelectableControls();

         if (controls.Count > 0)
         {
            // Find the current active control in the list
            int foundIndex = controls.FindIndex(delegate(ControlInfo lhs) { return lhs == _activeControl; });
            if (foundIndex == -1)
            {
               // If the current control could not be found and we're moving forwards, we select the FIRST control.
               if (direction > 0)
               {
                  newActiveControl = controls[0];
               }
               // If the current control could not be found and we're moving backwards, we select the LAST control.
               else
               {
                  newActiveControl = controls[controls.Count - 1];
               }
            }
            else
            {
               // Otherwise we advance the selection to the next control (according to direction).
               foundIndex += direction;

               // Wrap around if we go outside either direction.
               if (foundIndex < 0)
               {
                  foundIndex = controls.Count - 1;
               }
               else if (foundIndex >= controls.Count)
               {
                  foundIndex = 0;
               }

               newActiveControl = controls[foundIndex];
            }

            ScrollToControl(newActiveControl);

            SetActiveControl(newActiveControl, FocusReason.Keyboard);
         }

         return true;
      }

      private void ScrollToControl(ControlInfo newActiveControl)
      {
         Rectangle parentClientRect = Parent.ClientRectangle;

         Rectangle controlClientRect = newActiveControl.Control.GetClientRect();
         controlClientRect.Offset(Bounds.Location);

         if (controlClientRect.Bottom >= parentClientRect.Bottom)
         {
            int diff = controlClientRect.Bottom - parentClientRect.Bottom;
            PropertyInterface.SetScrollPosition(PropertyInterface.GetScrollPosition() + diff);
         }
         else if (controlClientRect.Top <= parentClientRect.Top)
         {
            int diff = controlClientRect.Top - parentClientRect.Top;
            PropertyInterface.SetScrollPosition(PropertyInterface.GetScrollPosition() + diff);
         }
      }

      public List<ControlInfo> GetSelectableControls()
      {
         List<ControlInfo> controls = new List<ControlInfo>();
         
         if( mRootControl != null )
         {
            GetSelectableControlsRecursive(mRootControl, controls);
         }

         return controls;
      }

      public void GetSelectableControlsRecursive(PropertyControl currentControl, List<ControlInfo> controls)
      {
         // Don't process invisible controls
         if (!currentControl.Visible)
            return;

         System.Collections.ObjectModel.ReadOnlyCollection<Controls.SubControls.SubControl> subControls = currentControl.SubControlCollection;
   
         if( subControls != null )
         {
            for(int i = 0; i < subControls.Count; ++i )
            {
               ControlInfo subControl = new ControlInfo(currentControl, i);
               
               if( currentControl.CanReceiveFocus(subControl) )
               {
                  controls.Add(subControl);
               }
            }
         }

         // Don't recurse to children when the layout is collapsed.
         BaseGroupBarLayout collapsibleLayout = currentControl as BaseGroupBarLayout;
         if (collapsibleLayout != null && collapsibleLayout.IsMinimized)
            return;

         foreach (PropertyControl child in currentControl.Children)
         {
            GetSelectableControlsRecursive(child, controls);
         }
      }
   }

   public class ControlInfo
   {
      public PropertyControl Control;
      public int SubControlIndex = -1;

      public ControlInfo(PropertyControl control, int subControlIndex)
      {
         Control = control;
         SubControlIndex = subControlIndex;
      }

      public override bool Equals(object obj)
      {
         if (obj == null)
            return false;

         if (GetType() != obj.GetType())
            return false;

         return this == ((ControlInfo)obj);
      }

      public override int GetHashCode()
      {
         return base.GetHashCode();
      }

      public static bool operator == (ControlInfo left, ControlInfo right)
      {
         if (ReferenceEquals(left, null) && ReferenceEquals(right, null))
            return true;

         if (!ReferenceEquals(left, null) && !ReferenceEquals(right, null))
         {
            return left.Control == right.Control && left.SubControlIndex == right.SubControlIndex;
         }
         else
         {
            return false;
         }
      }

      public static bool operator !=(ControlInfo left, ControlInfo right)
      {
         return !(left == right);
      }
   }

   public class ControlContext
   {
      private PropertyPanel mPropertyPanel;
      private Graphics mGraphics;

      public ControlContext(PropertyPanel panel, Graphics g)
      {
         mPropertyPanel = panel;
         mGraphics = g;
      }

      public RectangleF ClipBounds = new RectangleF();

      public Graphics Graphics
      {
         get { return mGraphics; }
      }

      public PropertyPanel PropertyPanel
      {
         get { return mPropertyPanel; }
      }

      public ControlInfo LeafControlInfo;
   }
}
