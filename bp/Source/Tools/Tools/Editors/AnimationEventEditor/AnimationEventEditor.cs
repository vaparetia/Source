using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace Tools.Editors.AnimationEventEditor
{
   public partial class AnimationEventEditor : UserControl
   {
      string _selectedJoint;

      public AnimationEventEditor()
      {
         InitializeComponent();
         ToolStripManager.Merge(mAnimationEditorToolStrip, mEventView.mToolBar);
         mEventView.EventContextMenuStripNeeded += OnEventContextMenuStripNeeded;
         mEventView.EventChanged += OnDataChanged;
         mEventView.EventsDeleted += OnDataChanged;
         mEventView.RowEntered += OnRowEntered;
      }

      string _jointNameFilter;
      List<string> _jointNames;

      public List<string> JointNames
      {
         set 
         {
            _jointNames = new List<string>(value);
            UpdateJointList();
         }
      }

      private void UpdateJointList()
      {
         mJointsListView.BeginUpdate();

         mJointsListView.Items.Clear();

         foreach (string jointName in _jointNames)
         {
            if( !string.IsNullOrEmpty(_jointNameFilter) )
            {
               if( jointName.IndexOf(_jointNameFilter, StringComparison.CurrentCultureIgnoreCase) == -1 )
               {
                  continue;
               }
            }

            mJointsListView.Items.Add(jointName);
         }

         mJointsListView.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);

         mJointsListView.EndUpdate();
      }

      public string SelectedJoint
      {
         get { return _selectedJoint; }
         set { _selectedJoint = value; }
      }

      public event Tools.Controls.EventView.CurrentTimeChangedDelegate CurrentTimeChanged
      {
         add { mEventView.CurrentTimeChanged += value; }
         remove { mEventView.CurrentTimeChanged -= value; }
      }

      public delegate void CurrentJointChangedDelegate(string jointName);
      public event CurrentJointChangedDelegate CurrentJointChanged;

      public delegate void DataChangedDelegate();
      public event DataChangedDelegate DataChanged;

      public delegate void RowEnteredDelegate(int rowIndex);
      public event RowEnteredDelegate RowEntered;

      public void SetCurrentFrame(int frame)
      {
         mEventView.SetCurrentFrame(frame);
      }

      public void SetFrameRange(int minTime, int maxTime)
      {
         mEventView.SetFrameRange(minTime, maxTime);
      }

      public void LoadFromNode(XmlNode node)
      {
         List<Tools.Controls.EventView.Event> events = new List<Tools.Controls.EventView.Event>();
         
         foreach(XmlElement eventElement in node.SelectNodes("./Event"))
         {

            Tools.Controls.EventView.Event eventItem = new Tools.Controls.EventView.Event();
            eventItem.Name = eventElement.GetAttribute("name");
            eventItem.GetDisplayName = OnGetDisplayEventName;

            AnimationEventData animationEventData = new AnimationEventData();
            eventItem.Tag = animationEventData;

            string jointName = eventElement.GetAttribute("joint");
            animationEventData.JointName = jointName.Length > 0 ? jointName : null;

            foreach(XmlElement eventEntryElement in eventElement.SelectNodes("./EventEntry"))
            {
               Tools.Controls.EventView.EventEntry eventEntry = new Tools.Controls.EventView.EventEntry();
               eventEntry.Frame = int.Parse(eventEntryElement.GetAttribute("time"));
               
               string durationString = eventEntryElement.GetAttribute("duration");
               
               if( !string.IsNullOrEmpty(durationString) )
               {
                  eventEntry.Duration = int.Parse(durationString);
               }

               eventItem.EventEntries.Add(eventEntry);
            }
            events.Add(eventItem);
         }

         mEventView.EventList = events;
      }

      public void StoreToNode(XmlNode node)
      {
         XmlDocument ownerDoc = node.OwnerDocument;

         foreach( Tools.Controls.EventView.Event eventItem in mEventView.EventList )
         {
            AnimationEventData animationEventData = eventItem.Tag as AnimationEventData;

            XmlElement eventElement = ownerDoc.CreateElement("Event");
            eventElement.SetAttribute("name", eventItem.Name);

            if(!String.IsNullOrEmpty(animationEventData.JointName))
            {
               eventElement.SetAttribute("joint", animationEventData.JointName);
            }

            foreach(Tools.Controls.EventView.EventEntry eventEntry in eventItem.EventEntries)
            {
               XmlElement eventEntryElement = ownerDoc.CreateElement("EventEntry");
               
               eventEntryElement.SetAttribute("time", eventEntry.Frame.ToString());
               
               if( eventEntry.Duration > 0 )
               {
                  eventEntryElement.SetAttribute("duration", eventEntry.Duration.ToString());
               }

               eventElement.AppendChild(eventEntryElement);
            }
            
            node.AppendChild(eventElement);
         }
      }

      private string OnGetDisplayEventName(Tools.Controls.EventView.Event eventItem)
      {
         AnimationEventData eventData = eventItem.Tag as AnimationEventData;
         return string.Format("{0} ({1})", eventItem.Name, eventData.JointName ?? "None");
      }

      private void OnEventContextMenuStripNeeded(Tools.Controls.EventView.Event eventItem, DataGridViewCellContextMenuStripNeededEventArgs e)
      {
         if (e.ContextMenuStrip == null)
         {
            e.ContextMenuStrip = new ContextMenuStrip();
            e.ContextMenuStrip.ShowImageMargin = false;
         }
         
         e.ContextMenuStrip.Items.Add(String.Format("Associate to {0}", SelectedJoint ?? "None"), null,
            delegate(object sender, EventArgs delegateArgs) 
            {
               AnimationEventData eventData = eventItem.Tag as AnimationEventData;
               eventData.JointName = SelectedJoint;
               mEventView.UpdateBoundData();
               OnDataChanged();
            }
            );
      }

      private void OnAddEvent(object sender, EventArgs e)
      {
         Tools.Controls.EventView.Event eventItem = new Tools.Controls.EventView.Event();
         eventItem.Name = "EventName";

         AnimationEventData animationEventData = new AnimationEventData();
         animationEventData.JointName = SelectedJoint;

         eventItem.Tag = animationEventData;
         eventItem.GetDisplayName = OnGetDisplayEventName;

         mEventView.AddEvent(eventItem);
         
         OnDataChanged();
      }

      private void OnJointSelected(object sender, EventArgs e)
      {
         if (mJointsListView.SelectedItems.Count > 0)
         {
            _selectedJoint = mJointsListView.SelectedItems[0].Text;
         }
         else
         {
            _selectedJoint = null;
         }

         if (CurrentJointChanged != null)
         {
            CurrentJointChanged(_selectedJoint);
         }
      }

      private void OnDataChanged()
      {
         if( DataChanged != null )
         {
            DataChanged();
         }
      }

      private void OnRowEntered(int rowIndex)
      {
         if (RowEntered != null)
         {
            RowEntered(rowIndex);
         }
      }

      private void OnJointFilterTextChanged(object sender, EventArgs e)
      {
         _jointNameFilter = jointFilterTextBox.Text;
         UpdateJointList();
      }
   }
}
