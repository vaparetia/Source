using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace CoreTools.Prefab.UI
{
   public partial class TagControl : UserControl
   {
      List<string> mTags;

      public delegate List<string> GetTagDBDelegate();
      public delegate void TagChangeDelegate(string tag);
      public delegate void TagRenameDelegate(string oldTag, string newTag);

      public GetTagDBDelegate mGetTagDBDelegate;
      public TagChangeDelegate mAddTagDelegate;
      public TagChangeDelegate mRemoveTagDelegate;
      public TagRenameDelegate mRenameTagDelegate;

      public bool mCanAddNew = true;

      public TagControl()
      {
         InitializeComponent();
      }

      public List<string> Tags
      {
         get
         {
            return mTags;
         }
      }

      public void SetState(List<string> tags, bool enabled)
      {
         bool needsRefresh = false;

         if( Enabled != enabled )
         {
            needsRefresh = true;
         }
         Enabled = enabled;

         if (tags != null)
         {
            List<string> newTags = new List<string>(tags);
            newTags.Sort();
            if (!Tools.Common.Misc.CompareStringArrays(mTags, newTags, true))
            {
               mTags = newTags;
               needsRefresh = true;
            }
         }
         else
         {
            mTags = null;
         }

         if( needsRefresh )
         {
            BuildUI();
         }
      }

      private void BuildUI()
      {
         tagButtonPanel.Hide();

         tagButtonPanel.SuspendLayout();

         Tools.Common.Misc.DestroyControls( tagButtonPanel.Controls );

         foreach(string tag in mTags)
         {
            TagButton button = new TagButton();
            button.Text = tag;
            button.DeleteEvent += new EventHandler(OnDeleteTag);
            button.Click += new EventHandler(OnChangeTag);
            button.Enabled = Enabled;
            tagButtonPanel.Controls.Add(button);
         }

         Button addButton = new Button();
         addButton.Text = "Add...";
         addButton.Enabled = Enabled;
         tagButtonPanel.Controls.Add(addButton);
         addButton.Click += new EventHandler(OnAddTag);

         tagButtonPanel.ResumeLayout(true);
         tagButtonPanel.Show();
      }

      void OnDeleteTag(object sender, EventArgs e)
      {
         Button button = sender as Button;
         string tag = button.Text;

         if( mRemoveTagDelegate != null )
         {
            mRemoveTagDelegate(tag);
         }

         mTags.Remove(tag);

         BuildUI();
      }

      void OnChangeTag(object sender, EventArgs e)
      {
         Button button = sender as Button;

         ContextMenuStrip menu = new ContextMenuStrip();
         menu.ShowImageMargin = false;
         menu.Tag = button.Text;

         List<string> tagDB = GetFilteredTagDB();

         if (tagDB != null && tagDB.Count > 0)
         {
            foreach (string tag in tagDB)
            {
               menu.Items.Add(tag, null, new EventHandler(ChangeTag));
            }
         }

         menu.Show(Cursor.Position);
      }

      void ChangeTag(object sender, EventArgs e)
      {
         ToolStripMenuItem item = sender as ToolStripMenuItem;
         string oldTag = item.Owner.Tag as string;

         string newTag = item.Text;

         int index = mTags.IndexOf(oldTag);
         mTags.RemoveAt(index);
         mTags.Insert(index, newTag);

         if (mRenameTagDelegate != null)
         {
            mRenameTagDelegate(oldTag, newTag);
         }

         BuildUI();
      }


      void OnAddTag(object sender, EventArgs e)
      {
         Control control = sender as Control;

         ContextMenuStrip menu = new ContextMenuStrip();
         menu.ShowImageMargin = false;

         if( mCanAddNew )
         {
            menu.Items.Add("New...", null, new EventHandler(AddNewTag));
         }

         List<string> tagDB = GetFilteredTagDB();

         if (tagDB != null && tagDB.Count > 0)
         {
            // Add separator if there are any other menu items.
            if( menu.Items.Count > 0 )
            {
               menu.Items.Add("-");
            }
            foreach (string tag in tagDB)
            {
               menu.Items.Add(tag, null, new EventHandler(AddTag));
            }
         }

         Rectangle buttonRectangle = control.RectangleToScreen(control.ClientRectangle);
         menu.Show(new Point(buttonRectangle.Left, buttonRectangle.Bottom));
      }

      private List<string> GetFilteredTagDB()
      {
         List<string> tagDB = null;

         if (mGetTagDBDelegate != null)
         {
            tagDB = mGetTagDBDelegate();

            foreach (string currentTag in mTags)
            {
               tagDB.Remove(currentTag);
            }

            tagDB.Sort();
         }
         return tagDB;
      }

      void AddNewTag(object sender, EventArgs e)
      {
         AutoCompleteStringCollection autoCompleteCollection = new AutoCompleteStringCollection();

         if( mGetTagDBDelegate != null )
         {
            foreach(string tag in mGetTagDBDelegate())
            {
               autoCompleteCollection.Add(tag);
            }
         }

         TextBox editBox = new TextBox();
         editBox.AutoCompleteMode = AutoCompleteMode.Suggest;
         editBox.AutoCompleteSource = AutoCompleteSource.CustomSource;
         editBox.AutoCompleteCustomSource = autoCompleteCollection;

         editBox.LostFocus += delegate(object delegateSender, EventArgs delegateEventArgs)
         {
            editBox.Dispose();
         };

         editBox.KeyDown += delegate(object delegateSender, KeyEventArgs delegateEventArgs)
         {
            switch( delegateEventArgs.KeyCode)
            {
               case Keys.Enter:
                  {
                     string tag = editBox.Text;
                     if( !mTags.Contains(tag) )
                     {
                        mTags.Add(tag);

                        if (mAddTagDelegate != null)
                        {
                           mAddTagDelegate(tag);
                        }

                        BuildUI();
                     }
                  }
                  break;
               
               case Keys.Escape:
                  editBox.Dispose();
                  break;
            }
         };

         Control addButton = tagButtonPanel.Controls[tagButtonPanel.Controls.Count - 1];
         editBox.Height = addButton.Height;

         tagButtonPanel.Controls.Remove(addButton);
         tagButtonPanel.Controls.Add(editBox);
         tagButtonPanel.Controls.Add(addButton);
         editBox.Focus();
      }

      void AddTag(object sender, EventArgs e)
      {
         ToolStripMenuItem item = sender as ToolStripMenuItem;
         
         string tag = item.Text;

         mTags.Add(tag);

         if( mAddTagDelegate != null )
         {
            mAddTagDelegate(tag);
         }
         
         BuildUI();
      }
   }
}
