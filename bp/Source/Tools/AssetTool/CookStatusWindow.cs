using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Tools.WorkQueue;
using System.Threading;
using System.Runtime.InteropServices;
using Tools.AssetSystem;

namespace AssetTool
{
   public partial class CookStatusWindow : Form
   {
      public WorkQueue mWorkQueue;
      public int mProcessedCount = 0;
      public int mTotalCount = 0;
      public bool mFormClosing = false;
      private bool mErrorMode = false;

      int   mCollapsedHeight;
      
      ManualResetEvent mre;

      public CookStatusWindow()
      {
         InitializeComponent();
         
         mCollapsedHeight = Height;

         UpdateDetailsMode();

         listView1.ItemSelectionChanged += new ListViewItemSelectionChangedEventHandler(OnSelectionChanged);

         mre = NamedEventHelper.CreateNamedEvent(false, "LAST_INSTANCE_ONLY");
         
         // Stop the other instances
         mre.Set();
         
         // Create a monitor thread
         Thread t = new Thread(new ThreadStart(Monitor));
         
         // Make sure that this thread cannot keep the app alive
         t.IsBackground = true;
         t.Start();

      }

      void Monitor()
      {
         // Reset the event so that we can run
         mre.Reset();
         mre.WaitOne();
         Application.Exit();
      }

      void OnSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e)
      {
         AssetBuildCookAssetWorkItem workItem = e.Item.Tag as AssetBuildCookAssetWorkItem;
         
         if( workItem != null )
         {
            switch( workItem.State )
            {
               case WorkItemState.Completed:
                  string taskInfo = string.Format("Duration: {0} seconds\n", workItem.ProcessingTime.TotalSeconds);
                  richTextBox1.Text = taskInfo + workItem.mStandardOutput;
                  break;

               default:
                  richTextBox1.Text = "";
                  break;
            }
         }
         else
         {
            richTextBox1.Text = "";
         }
      }

      delegate void WorkItemTextDelegate(AssetBuildCookAssetWorkItem workItem, string text);

      void UpdateProgress(AssetBuildCookAssetWorkItem workItem, string text)
      {
         bool hadItem = false;

         foreach (ListViewItem item in listView1.Items)
         {
            if (item.Tag == workItem)
            {
               hadItem = true;
               item.Group = listView1.Groups[text];
            }
         }

         if( !hadItem )
         {
            mTotalCount++;
            foreach (AssetBuildCookAssetEntry assetEntry in workItem.mAssetEntries)
            {
               // Format the text for the work item
               String itemText = assetEntry.mAssetPath;
               if ( workItem.mPlatforms != PlatformType.CookPlatforms )
               {
                  String platformText = "";
                  bool wroteFirst = false;
                  foreach ( PlatformType.EPlatform platform in workItem.mPlatforms )
                  {
                     if ( wroteFirst )
                     {
                        platformText += ", ";
                     }
                     else
                     {
                        wroteFirst = true;
                     }
                     platformText += PlatformType.GetPlatformTypeForEnum( platform ).ShortName;
                  }

                  itemText = String.Format( "[{0}] {1}", platformText, itemText );
               }
               
               ListViewItem item = new ListViewItem(itemText);
               item.Tag = workItem;
               item.Group = listView1.Groups[text];
               listView1.Items.Add(item);
            }
         }

         if ( mProcessedCount < mTotalCount )
         {
            progressBar1.Value = ( mProcessedCount * 100 ) / mTotalCount;
            if ( !mErrorMode )
            {
               label1.Text = string.Format( "Processed Batch {0}/{1}...", mProcessedCount, mTotalCount );
            }
         }
         else
         {
            progressBar1.Value = 100;
            if ( !mErrorMode )
            {
               label1.Text = "Complete!";
            }
         }
      }

      public void OnChangedWorkItemState(object sender, ChangedWorkItemStateEventArgs e)
      {
         if (!Visible)
            return;

         AssetBuildCookAssetWorkItem workItem = e.WorkItem as AssetBuildCookAssetWorkItem;

         switch (e.WorkItem.State)
         {
            case WorkItemState.Created:
            case WorkItemState.Queued:
            case WorkItemState.Scheduled:
               BeginInvoke(new WorkItemTextDelegate(UpdateProgress), workItem, "Queued");
               break;

            case WorkItemState.Running:
               BeginInvoke(new WorkItemTextDelegate(UpdateProgress), workItem, "Processing");
               break;

            case WorkItemState.Completed:
               if( e.PreviousState == WorkItemState.Running )
               {
                  mProcessedCount++;
                  BeginInvoke(new WorkItemTextDelegate(UpdateProgress), workItem, "Completed");
               }
               break;

            case WorkItemState.Failing:
               BeginInvoke(new WorkItemTextDelegate(UpdateProgress), workItem, "Failed");
               break;
         }

      }

      protected override void OnFormClosing(FormClosingEventArgs e)
      {
         base.OnFormClosing(e);

         Properties.Settings.Default.Save();

         if ( mWorkQueue != null )
         {
            mWorkQueue.Clear();
            mWorkQueue.Pause();
         }

         mFormClosing = true;
      }

      private void OnPressedDetails(object sender, EventArgs e)
      {
         Properties.Settings.Default.ShowDetails = !Properties.Settings.Default.ShowDetails;
         UpdateDetailsMode();
      }

      public void TurnIntoErrorDisplayMode()
      {
         button1.Enabled = false;
         progressBar1.Visible = false;

         // Grow label1 where the progressbar was
         label1.BackColor = Color.FromArgb( 192, 0, 0 );
         label1.ForeColor = Color.White;
         label1.Text = "Errors below!";
         label1.Size = new Size( progressBar1.Right - label1.Left, label1.Height );
         label1.Anchor |= AnchorStyles.Right;

         mErrorMode = true;
      }

      public void UpdateDetailsMode()
      {
         bool showDetails = Properties.Settings.Default.ShowDetails || mErrorMode;
         if( showDetails )
         {
            MaximumSize = new Size();
            Height = Properties.Settings.Default.ExpandedHeight;
            button1.Text = "<< Details";
         }
         else
         {
            MaximumSize = new Size(9999, mCollapsedHeight);
            Height = mCollapsedHeight;
            button1.Text = "Details >>";
         }
      }

      public void AddTextToDetailsOutput(string text)
      {
         richTextBox1.AppendText(text);
      }

      private void OnResizeEnd(object sender, EventArgs e)
      {
         bool showDetails = Properties.Settings.Default.ShowDetails;
         if( showDetails )
         {
            Properties.Settings.Default.ExpandedHeight = Height;
         }
      }

      public void WaitForFormClose()
      {
         while ( !mFormClosing )
         {
            System.Windows.Forms.Application.DoEvents();
            Thread.Sleep( 10 );
         }
      }

      public bool WasAborted
      {
         get { return !Visible; }
      }

      private void mShowTimer_Tick( object sender, EventArgs e )
      {
         mShowTimer.Enabled = false;
         Show();
      }
   }

   public class NamedEventHelper
   {
      [DllImport("kernel32")]
      static extern uint CreateEvent(
         uint sec, bool manualReset, bool initialState, string name);
      static IntPtr CreateEvent(bool manualReset, bool initialState, string name)
      {
         return new IntPtr(CreateEvent(0, manualReset, initialState, name));
      }

      [DllImport("kernel32")]
      static extern bool CloseHandle(IntPtr handle);

      public static ManualResetEvent CreateNamedEvent(
         bool initialState, string name)
      {
         ManualResetEvent mre = new ManualResetEvent(false);
         CloseHandle(mre.SafeWaitHandle.DangerousGetHandle());
         mre.SafeWaitHandle = new Microsoft.Win32.SafeHandles.SafeWaitHandle(CreateEvent(true, initialState, name), true);
         return mre;
      }
   }
}