using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace Tools.Controls
{
   public partial class EventView : UserControl
   {
      enum ImageListIndices
      {
         Tick = 0,
         RangeBegin = 1,
         RangeMiddle = 2,
         RangeEnd = 3
      }

      public class EventEntry
      {
         public int Frame;
         public int Duration;

         public EventEntry()
         {
         }
      }

      public class Event
      {
         string _name = "EventName";
         public string Name
         {
            get { return _name; }
            set { _name = value; }
         }

         public delegate string GetDisplayNameDelegate(Event eventItem);

         public GetDisplayNameDelegate GetDisplayName;
         public string DisplayName
         {
            get
            {
               if( GetDisplayName != null )
               {
                  return GetDisplayName(this);
               }

               return Name; 
            }
            set { Name = value; }
         }

         object _tag;
         public object Tag
         {
            get { return _tag; }
            set { _tag = value; }
         }

         List<EventEntry> _eventEntries = new List<EventEntry>();
         public List<EventEntry> EventEntries
         {
            get { return _eventEntries; }
            set { _eventEntries = value; }
         }

         public Event()
         {
         }

         public EventEntry GetEventEntryForFrame(int frame)
         {
            foreach (EventEntry entry in _eventEntries)
            {
               int entryStart = entry.Frame;
               int entryEnd = entry.Frame + entry.Duration;
               
               if (frame >= entryStart && frame <= entryEnd)
               {
                  return entry;
               }
            }

            return null;
         }
      }

      class InputHandler
      {
         public InputHandler(EventView view)
         {
            mView = view;
         }

         public virtual void TearDown()
         {
         }

         protected EventView mView;
      }

      class NormalInputHandler : InputHandler
      {
         public NormalInputHandler(EventView view)
         : base(view)
         {
            mView.dataGridView1.MouseDown += OnMouseDown;
            mView.dataGridView1.RowEnter += OnRowEnter;
         }

         public override void TearDown()
         {
            mView.dataGridView1.MouseDown -= OnMouseDown;
            mView.dataGridView1.RowEnter -= OnRowEnter;
         }

         void OnRowEnter(object sender, DataGridViewCellEventArgs e)
         {
            if (mView.RowEntered != null)
            {
               mView.RowEntered(e.RowIndex);
            }
         }

         void OnMouseDown(object sender, MouseEventArgs e)
         {
            if( e.Button == MouseButtons.Left )
            {
               DataGridView.HitTestInfo hitInfo = mView.dataGridView1.HitTest(e.X, e.Y);

               if (hitInfo.RowIndex < 0)
                  return;

               if( hitInfo.ColumnIndex != kTimelineColumnIndex )
                  return;

               Event eventForCurrentRow = mView._events[hitInfo.RowIndex];

               int frameForCursor = mView.ConvertCellSpaceToFrame(e.X - hitInfo.ColumnX);
               
               // if we have a matching time, select that one
               EventEntry eventEntryUnderCursor = eventForCurrentRow.GetEventEntryForFrame(frameForCursor);

               if (eventEntryUnderCursor == null)
               {
                  if( (Control.ModifierKeys & Keys.Control) == Keys.Control )
                  {
                     EventEntry eventEntry = new EventEntry();
                     eventEntry.Frame = frameForCursor;
                     eventForCurrentRow.EventEntries.Add(eventEntry);

                     eventEntryUnderCursor = eventEntry;
                  }
               }

               mView.SetSelection(eventForCurrentRow, eventEntryUnderCursor);

               if( (Control.ModifierKeys & Keys.Control) == Keys.Control )
               {
                  mView.SetInputHandler(new ChangeDurationInputHandler(mView, eventForCurrentRow, eventEntryUnderCursor, hitInfo.RowIndex));
               }
               else if (eventForCurrentRow != null && eventEntryUnderCursor != null )
               {
                  mView.SetInputHandler(new DragEventInputHandler(mView, eventForCurrentRow, eventEntryUnderCursor, hitInfo.RowIndex, frameForCursor));
               }

               if (mView.CurrentTimeChanged != null)
               {
                  mView.CurrentTimeChanged(frameForCursor);
               }
            }
            else if( e.Button == MouseButtons.Middle )
            {
               mView.SetInputHandler(new ChangeTimeInputHandler(mView));
            }
            else if (e.Button == MouseButtons.Right)
            {
               DataGridView.HitTestInfo hitInfo = mView.dataGridView1.HitTest(e.X, e.Y);

               Event eventForCurrentRow = null;
               EventEntry eventEntryUnderCursor = null;
               if (hitInfo.RowIndex >= 0)
               {               
                  eventForCurrentRow = mView._events[hitInfo.RowIndex];
                  
                  if (hitInfo.ColumnIndex == kTimelineColumnIndex)
                  {

                     int frameForCursor = mView.ConvertCellSpaceToFrame(e.X - hitInfo.ColumnX);

                     // if we have a matching time, select that one
                     eventEntryUnderCursor = eventForCurrentRow.GetEventEntryForFrame(frameForCursor);
                  }
               }

               mView.SetSelection(eventForCurrentRow, eventEntryUnderCursor);
            }
         }
      }

      class ChangeTimeInputHandler : InputHandler
      {
         public ChangeTimeInputHandler(EventView view)
         : base(view)
         {
            mView.dataGridView1.Capture = true;

            mView.dataGridView1.MouseMove += OnMouseMove;
            mView.dataGridView1.MouseUp += OnMouseUp;
         }

         public override void TearDown()
         {
            mView.dataGridView1.MouseMove -= OnMouseMove;
            mView.dataGridView1.MouseUp -= OnMouseUp;
         }

         void OnMouseMove(object sender, MouseEventArgs e)
         {
            Rectangle cellRectangle = mView.dataGridView1.GetColumnDisplayRectangle(kTimelineColumnIndex, false);
            int frame = mView.ConvertCellSpaceToFrame(e.X - cellRectangle.Left);

            if (mView.CurrentTimeChanged != null)
            {
               mView.CurrentTimeChanged(frame);
            }
         }

         void OnMouseUp(object sender, MouseEventArgs e)
         {
            mView.dataGridView1.Capture = false;
            mView.SetInputHandler(new NormalInputHandler(mView));
         }

      }

      class DragEventInputHandler : InputHandler
      {
         Event       mEventItem;
         EventEntry  mEventEntry;
         int         mRowIndex;
         int         mCurrentFrame;

         public DragEventInputHandler(EventView view, Event eventItem, EventEntry eventEntry, int rowIndex, int clickFrame )
         : base(view)
         {
            mEventItem = eventItem;
            mEventEntry = eventEntry;
            mRowIndex = rowIndex;
            mCurrentFrame = clickFrame;

            mView.dataGridView1.Capture = true;

            mView.dataGridView1.MouseMove += OnMouseMove;
            mView.dataGridView1.MouseUp += OnMouseUp;
         }

         public override void TearDown()
         {
            mView.dataGridView1.MouseMove -= OnMouseMove;
            mView.dataGridView1.MouseUp -= OnMouseUp;
         }

         void OnMouseMove(object sender, MouseEventArgs e)
         {
            Rectangle cellRectangle = mView.dataGridView1.GetCellDisplayRectangle(kTimelineColumnIndex, mRowIndex, false);

            int frame = mView.ConvertCellSpaceToFrame(e.X - cellRectangle.Left);

            // don't allow moving event times onto other event times
            EventEntry entryAtFrame = mEventItem.GetEventEntryForFrame(frame);
            if (entryAtFrame == null || entryAtFrame == mEventEntry)
            {
               int frameDelta = frame - mCurrentFrame;
               mCurrentFrame = frame;

               mEventEntry.Frame += frameDelta;

               if (mView.CurrentTimeChanged != null)
               {
                  mView.CurrentTimeChanged(frame);
               }

               mView.dataGridView1.InvalidateRow(mRowIndex);
            }
         }

         void OnMouseUp(object sender, MouseEventArgs e)
         {
            mView.dataGridView1.Capture = false;
            mView.SetInputHandler(new NormalInputHandler(mView));
            mView.OnEventChanged();
         }
      }

      class ChangeDurationInputHandler : InputHandler
      {
         Event       mEventItem;
         EventEntry  mEventEntry;
         int         mRowIndex;

         public ChangeDurationInputHandler(EventView view, Event eventItem, EventEntry eventEntry, int rowIndex)
         : base(view)
         {
            mEventItem = eventItem;
            mEventEntry = eventEntry;
            mRowIndex = rowIndex;

            mView.dataGridView1.Capture = true;

            mView.dataGridView1.MouseMove += OnMouseMove;
            mView.dataGridView1.MouseUp += OnMouseUp;
         }

         public override void TearDown()
         {
            mView.dataGridView1.MouseMove -= OnMouseMove;
            mView.dataGridView1.MouseUp -= OnMouseUp;
         }

         void OnMouseMove(object sender, MouseEventArgs e)
         {
            Rectangle cellRectangle = mView.dataGridView1.GetCellDisplayRectangle(kTimelineColumnIndex, mRowIndex, false);

            int frame = mView.ConvertCellSpaceToFrame(e.X - cellRectangle.Left);

            // don't allow moving event times onto other event times
            EventEntry entryAtFrame = mEventItem.GetEventEntryForFrame(frame);
            if (entryAtFrame == null || entryAtFrame == mEventEntry)
            {
               int newDuration = Math.Max(frame - mEventEntry.Frame, 0);

               mEventEntry.Duration = newDuration;

               if (mView.CurrentTimeChanged != null)
               {
                  mView.CurrentTimeChanged(frame);
               }

               mView.dataGridView1.InvalidateRow(mRowIndex);
            }
         }

         void OnMouseUp(object sender, MouseEventArgs e)
         {
            mView.dataGridView1.Capture = false;
            mView.SetInputHandler(new NormalInputHandler(mView));
            mView.OnEventChanged();
         }
      }

      List<Event> _events = new List<Event>();
      
      [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
      public List<Event> EventList
      {
         set
         {
            _events.Clear();
            _events.AddRange(value);

            CurrencyManager cm = dataGridView1.BindingContext[_events] as CurrencyManager;
            cm.Refresh();
         }

         get { return _events; }
      }

      InputHandler mInputHandler;

      const int kTimelineColumnIndex = 0;

      private Event _selectedEvent;
      private EventEntry _selectedEntry;

      int _minFrame = 100;
      int _maxFrame = 110;

      int _currentFrame = 0;
      public delegate void CurrentTimeChangedDelegate(int newFrame);
      public event CurrentTimeChangedDelegate CurrentTimeChanged;

      public delegate void EventContextMenuStripNeededDelegate(Event eventItem, DataGridViewCellContextMenuStripNeededEventArgs e);
      public EventContextMenuStripNeededDelegate EventContextMenuStripNeeded;

      public delegate void EventChangedDelegate();
      public event EventChangedDelegate EventChanged;

      public delegate void EventsDeletedDelegate();
      public event EventsDeletedDelegate EventsDeleted;

      public delegate void RowEnteredDelegate(int rowIndex);
      public event RowEnteredDelegate RowEntered;

      public EventView()
      {
         InitializeComponent();

         dataGridView1.DataSource = _events;
         SetInputHandler(new NormalInputHandler(this));

         EventContextMenuStripNeeded += OnEventContextMenuStripNeeded;
      }

      private void SetSelectedRow(int rowIndex)
      {
         // make the row visible
         dataGridView1.FirstDisplayedScrollingRowIndex = rowIndex;
         dataGridView1.Refresh();
         // select row so that binding sources update their current item
         dataGridView1.CurrentCell = dataGridView1.Rows[rowIndex].Cells[0];
         // visually select
         dataGridView1.Rows[rowIndex].Selected = true;
      }

      private void MoveSelectedRow(int offset)
      {
         int oldIndex = _events.IndexOf(_selectedEvent);
         int newIndex = oldIndex + offset;
         if (oldIndex >= 0 && newIndex >= 0 && newIndex < _events.Count)
         {
            _events.RemoveAt(oldIndex);
            _events.Insert(newIndex, _selectedEvent);

            SetSelectedRow(newIndex);
            RefreshCurrencyManager();

            // mark as dirty so the change gets saved
            OnEventChanged();
         }
      }

      private void OnEventContextMenuStripNeeded(Tools.Controls.EventView.Event eventItem, DataGridViewCellContextMenuStripNeededEventArgs e)
      {
         if (e.ContextMenuStrip == null)
         {
            e.ContextMenuStrip = new ContextMenuStrip();
            e.ContextMenuStrip.ShowImageMargin = false;
         }
         ToolStripItem moveUpItem = e.ContextMenuStrip.Items.Add(
            "Move up",
            null, // image
            delegate(object sender, EventArgs delegateArgs)
            {
               MoveSelectedRow(-1);
            }
            );
         ToolStripItem moveDownItem = e.ContextMenuStrip.Items.Add(
            "Move down",
            null, // image
            delegate(object sender, EventArgs delegateArgs)
            {
               MoveSelectedRow(+1);
            }
            );
         int eventIndex = _events.IndexOf(_selectedEvent);
         if (eventIndex == 0)
         {
            moveUpItem.Enabled = false;
         }
         if (eventIndex == _events.Count - 1)
         {
            moveDownItem.Enabled = false;
         }
      }

      public void SetSelection(Event eventItem, EventEntry eventEntry)
      {
         _selectedEvent = eventItem;
         _selectedEntry = eventEntry;
      }

      public void SetCurrentFrame(int frame)
      {
         _currentFrame = frame;
         dataGridView1.Invalidate();
      }

      public void SetFrameRange(int minFrame, int maxFrame)
      {
         _minFrame = minFrame;
         _maxFrame = maxFrame;
         dataGridView1.Invalidate();
      }

      public void AddEvent(Event eventItem)
      {
         _events.Add(eventItem);

         RefreshCurrencyManager();
      }

      private void RefreshCurrencyManager()
      {
         CurrencyManager cm = dataGridView1.BindingContext[_events] as CurrencyManager;
         cm.Refresh();
      }

      public void UpdateBoundData()
      {
         RefreshCurrencyManager();
      }

      private void OnCellPaint(object sender, DataGridViewCellPaintingEventArgs e)
      {
         if (e.RowIndex >= 0 && e.ColumnIndex == kTimelineColumnIndex)
         {
            int frameRange = _maxFrame - _minFrame;

            BufferedGraphics graphics = BufferedGraphicsManager.Current.Allocate(e.Graphics, e.CellBounds);

            if( Enabled )
            {
               graphics.Graphics.Clear(SystemColors.Window);
            }
            else
            {
               graphics.Graphics.Clear(SystemColors.Control);
            }
            
            // Draw time line grid
            using ( Pen backgroundLinePen = new Pen( SystemBrushes.GrayText ) )
            {
               int yMid = e.CellBounds.Top + e.CellBounds.Height / 2;
               graphics.Graphics.DrawLine(backgroundLinePen, e.CellBounds.Left, yMid, e.CellBounds.Right, yMid);

               int y1 = yMid - 4;
               int y2 = yMid + 4;
               int frameCount = _maxFrame - _minFrame;

               for ( int i = 0; i < frameCount; ++i )
               {
                  int x = e.CellBounds.Left + ( i * e.CellBounds.Width ) / frameCount;
                  graphics.Graphics.DrawLine( backgroundLinePen, x, y1, x, y2 );
               }
            }

            // Draw events
            {
               Event eventItem = _events[e.RowIndex];
               int imageWidthOverTwo = unselectedImageList.Images[(int)ImageListIndices.Tick].Width / 2;
               foreach ( EventEntry time in eventItem.EventEntries )
               {
                  bool isSelected = time == _selectedEntry;

                  ImageList imageList = isSelected ? selectedImageList : unselectedImageList;

                  float rTime = ( time.Frame - _minFrame ) / (float) frameRange;
                  if( time.Duration == 0 )
                  {
                     graphics.Graphics.DrawImageUnscaled(imageList.Images[(int)ImageListIndices.Tick], e.CellBounds.Left + (int)(rTime * e.CellBounds.Width) - imageWidthOverTwo, e.CellBounds.Top);
                  }
                  else
                  {
                     int startX = e.CellBounds.Left + (int)(rTime * e.CellBounds.Width);

                     float rEndTime = (time.Frame + time.Duration - _minFrame) / (float) frameRange;
                     int endX = e.CellBounds.Left + (int)(rEndTime * e.CellBounds.Width);

                     Rectangle barRect = new Rectangle(startX + 3, e.CellBounds.Top, endX - startX - 6, 21);

                     Image barImage = imageList.Images[(int)ImageListIndices.RangeMiddle];

                     System.Drawing.Imaging.ImageAttributes attr = new System.Drawing.Imaging.ImageAttributes();
                     attr.SetWrapMode(System.Drawing.Drawing2D.WrapMode.Tile);
                     graphics.Graphics.DrawImage(barImage, barRect, 0, 0, barImage.Width, barImage.Height, GraphicsUnit.Pixel, attr);

                     graphics.Graphics.DrawImageUnscaled(imageList.Images[(int)ImageListIndices.RangeBegin], startX - imageWidthOverTwo, e.CellBounds.Top);
                     graphics.Graphics.DrawImageUnscaled(imageList.Images[(int)ImageListIndices.RangeEnd], endX - imageWidthOverTwo, e.CellBounds.Top);
                  }
               }

               // Draw current frame indicator
               {
                  float rTime = (_currentFrame - _minFrame) / (float)frameRange;
                  Color timeLineColor = Color.FromArgb(64, Color.Black);
                  using(Brush timeLineBrush = new SolidBrush(timeLineColor))
                  {
                     graphics.Graphics.FillRectangle(timeLineBrush, e.CellBounds.Left + (int)(rTime * e.CellBounds.Width), e.CellBounds.Top, e.CellBounds.Width / frameRange, e.CellBounds.Height);
                  }
               }

               graphics.Render();

               e.Handled = true;
            }
         }
      }

      private int ConvertCellSpaceToFrame(int x)
      {
         int width = dataGridView1.Columns[kTimelineColumnIndex].Width;
         float rT = x / (float)width;

         int eventFrame = (int)(_minFrame + 0.5f + rT * (_maxFrame - _minFrame));

         if (eventFrame < _minFrame)
         {
            eventFrame = _minFrame;
         }
         else if (eventFrame > _maxFrame)
         {
            eventFrame = _maxFrame;
         }

         return eventFrame;
      }

      private void OnDelete(object sender, EventArgs e)
      {
         foreach (DataGridViewRow row in dataGridView1.SelectedRows)
         {
            _events.RemoveAt(row.Index);
         }

         if( EventsDeleted != null )
         {
            EventsDeleted();
         }

         CurrencyManager cm = dataGridView1.BindingContext[_events] as CurrencyManager;
         cm.Refresh();
      }

      private void OnKeyDown(object sender, KeyEventArgs e)
      {
         if( e.KeyCode == Keys.Delete )
         {
            if (_selectedEvent != null && _selectedEntry != null )
            {
               _selectedEvent.EventEntries.Remove(_selectedEntry);

               SetSelection(null, null);

               dataGridView1.Refresh();

               OnEventChanged();
            }
         }
      }

      private void SetInputHandler(InputHandler inputHandler)
      {
         if (mInputHandler != null)
         {
            mInputHandler.TearDown();
         }

         mInputHandler = inputHandler;
      }

      private void OnEditingControlShowing(object sender, DataGridViewEditingControlShowingEventArgs e)
      {
         DataGridView view = sender as DataGridView;
         if (view.CurrentCell.OwningColumn.Name == mDisplayNameColumn.Name)
         {
            Event eventForCurrentRow = _events[view.CurrentCell.RowIndex];
            e.Control.Text = eventForCurrentRow.Name;
         }
      }

      private void OnCellContextMenuStripNeeded(object sender, DataGridViewCellContextMenuStripNeededEventArgs e)
      {
         if (dataGridView1.Columns[e.ColumnIndex].Name == mDisplayNameColumn.Name)
         {
            if (EventContextMenuStripNeeded != null)
            {
               Event eventForCurrentRow = _events[e.RowIndex];
               EventContextMenuStripNeeded(eventForCurrentRow, e);
            }
         }
      }

      private void OnCellEndEdit(object sender, DataGridViewCellEventArgs e)
      {
         OnEventChanged();
      }

      public void OnEventChanged()
      {
         if (EventChanged != null)
         {
            EventChanged();
         }
      }
   }
}
