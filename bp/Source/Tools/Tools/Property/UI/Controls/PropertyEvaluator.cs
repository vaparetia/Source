using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Windows.Forms;
using Tools.Property;
using Tools.Property.Misc;

namespace Tools.Property.UI.Controls
{
   public class PropertyEvaluator : PropertyLayout
   {
      public const string kConstFloatEvaluator = "Const_float";
      public const string kConstColorEvaluator = "Const_color";
      public const string kNoneEvaluator = "None";
      public Data.EvaluatorProperty EvaluatorPropertyDataNode
      {
         get { return (Data.EvaluatorProperty) mDataElement; }
      }

      public PropertyEvaluator(PropertyPanel panel, Data.DataNode dataElement, XmlElement uiStateElement)
         : base(panel, dataElement, uiStateElement)
      {
         Margin = new Padding();
         Padding = new Padding();

         AllocateChildStorage();

         BuildEvaluatorControls();
      }

      public void PasteData(string evaluatorXmlData)
      {
         XmlDocument data = new XmlDocument();
         data.LoadXml(evaluatorXmlData);
         string pasteEvaluatorName = Helpers.GetActiveEvaluatorFromEvaluatorProperty(data.CreateNavigator());

         // only paste if given evaluator exists in this property
         List<string> evaluators = EvaluatorPropertyDataNode.GetValidEvaluators( mPanel.PropertyInterface.mPropertyDB );
         if (evaluators.Contains(pasteEvaluatorName))
         {
            bool bWasGrouped = IsGrouped;

            // paste xml data
            EvaluatorPropertyDataNode.InnerXml = evaluatorXmlData;

            // Even if it wasn't grouped, we want to mark it as modified
            if (!bWasGrouped)
            {
               EvaluatorPropertyDataNode.IsModified = true;
            }

            // Rebuild controls of evaluator
            RebuildEvaluatorChildControls();

            // Notify that evaluators have changed
            mPanel.PropertyInterface.SendEvaluatorChangedEvents(new PropertyEditorGUI.PropertyEditorCallbacks.EvaluatorPropertyEventArgs(this, EvaluatorPropertyDataNode));
         }
      }

      void BuildEvaluatorControls()
      {
         string activeEvaluator = EvaluatorPropertyDataNode.ActiveEvaluatorType;

         switch (activeEvaluator)
         {
            case kConstFloatEvaluator:
            case kConstColorEvaluator:
               {
                  mPanel.PropertyInterface.BuildControls(mPanel, this, EvaluatorPropertyDataNode.Evaluator);
               }
               break;

            case kNoneEvaluator:
               {
                  AddChild(new PropertyEvaluatorNone(Panel, mDataElement));
               }
               break;
         }

         // If there is only one property added, we need to modify it to suit our needs.
         if (ChildCount == 1)
         {
            IEnumerator<PropertyControl> childEnum = Children.GetEnumerator();
            childEnum.MoveNext();
            PropertyControl simpleEvaluator = childEnum.Current;
            simpleEvaluator.BuildContextMenu = OnBuildContextMenu;

            if (simpleEvaluator.SubControlCollection != null)
            {
               foreach (SubControls.SubControl control in simpleEvaluator.SubControlCollection)
               {
                  SubControls.Label labelSubControl = control as SubControls.Label;

                  if (labelSubControl != null)
                  {
                     labelSubControl.GetDisplayName = delegate(out string displayName)
                     {
                        displayName = mDataElement.DisplayName;
                     };
                  }

                  SubControls.ContextMenuButton contextMenuButton = control as SubControls.ContextMenuButton;
                  if (contextMenuButton != null)
                  {
                     contextMenuButton.ContextMenuType = Constants.kEvaluatorContextMenuType;
                  }
               }
            }
         }
         else
         {
            // If there is multiple properties we early out here and deal with it below with the catch all case.
            ClearChildren();
         }

         // If everything else failed, we add it as a standard complex evaluator
         if (ChildCount == 0)
         {
            EvaluatorLayout complexEvaluator = new EvaluatorLayout(mPanel, mDataElement, mUIStateElement);
            complexEvaluator.BuildContextMenu = OnBuildContextMenu;
            AddChild(complexEvaluator);
         }
      }

      void OnBuildContextMenu(PropertyControl control, ContextMenuStrip menu)
      {
         bool isNoneEvaluator = control is PropertyEvaluatorNone;

         if( !isNoneEvaluator )
         {
            Helper.BuildCustomEditorsContextMenu(this, menu);

            bool isComplexEvaluator = control is EvaluatorLayout;
            if (!isComplexEvaluator)
            {
               Helper.BuildResetToDefaultContextMenu(control, menu, "Reset value to default");
            }

            if (menu.Items.Count > 0)
               menu.Items.Add("-");
         }

         string activeEvaluator = EvaluatorPropertyDataNode.ActiveEvaluatorType;

         List<string> evaluators = EvaluatorPropertyDataNode.GetValidEvaluators(mPanel.PropertyInterface.mPropertyDB);
         foreach (string evaluator in evaluators)
         {
            ToolStripMenuItem evaluatorItem = new ToolStripMenuItem(evaluator);

            // Associate tooltip with item
            {
               System.Xml.XPath.XPathNavigator evaluatorNav = mPanel.PropertyInterface.mPropertyDB.FindEvaluatorById(evaluator, Tools.Property.Source.ESourceVersion.kSourceVersion_2);
               if( evaluatorNav != null )
               {
                  XmlElement evaluatorXml = evaluatorNav.UnderlyingObject as XmlElement;
                  evaluatorItem.ToolTipText = Helpers.GetToolTipText(evaluatorXml);
               }
            }

            evaluatorItem.Tag = evaluator;
            evaluatorItem.Click += OnChooseEvaluator;
            evaluatorItem.Checked = (evaluator == activeEvaluator);
            menu.Items.Add(evaluatorItem);
         }
      }

      void OnChooseEvaluator(object sender, EventArgs e)
      {
         ToolStripMenuItem menuItem = sender as ToolStripMenuItem;
         string evaluatorName = menuItem.Tag as string;

         if (SetEvaluator(evaluatorName, mDataElement.IsGrouped ))
         {
            mPanel.PropertyInterface.SendEvaluatorChangedEvents(new PropertyEditorGUI.PropertyEditorCallbacks.EvaluatorPropertyEventArgs(this, EvaluatorPropertyDataNode));
         }
      }
      
      bool SetEvaluator(string evaluatorName, bool forceSetEvaluator)
      {
         bool currentEvaluatorChanged = false;

         if (forceSetEvaluator || EvaluatorPropertyDataNode.ActiveEvaluatorType != evaluatorName)
         {
            EvaluatorPropertyDataNode.SetActiveEvaluatorType( evaluatorName, mPanel.PropertyInterface.mPropertyDB );

            mDataElement.IsGrouped = false;
            EvaluatorPropertyDataNode.IsModified = true;

            currentEvaluatorChanged = true;
         }

         RebuildEvaluatorChildControls();

         return currentEvaluatorChanged;
      }

      private void RebuildEvaluatorChildControls()
      {
         // Remove current control children.
         ClearChildren();

         BuildEvaluatorControls();

         // Refresh the layout
         mPanel.InvalidateLayout();
      }
   }

   public class PropertyEvaluatorNone : PropertyControl
   {
      public PropertyEvaluatorNone(PropertyPanel panel, Data.DataNode dataElement)
         : base(panel, dataElement)
      {
         mControlHeight = 20;

         AllocateSubControlStorage();

         // Label
         {
            Controls.SubControls.Label label = new Tools.Property.UI.Controls.SubControls.Label(panel, Tools.Property.UI.Controls.SubControls.Label.LabelType.Normal);
            label.GetLabelDrawFlags = delegate(out Controls.SubControls.Label.DrawFlags drawFlags)
            {
               drawFlags = Tools.Property.UI.Controls.SubControls.Label.DrawFlags.Normal;
            };

            AddSubControl(label);
         }

         // None Label
         {
            Controls.SubControls.ReadOnlyText noneLabel = new Tools.Property.UI.Controls.SubControls.ReadOnlyText(panel);
            noneLabel.Text = "No Evaluator";
            AddSubControl(noneLabel);
         }

         // Context button
         {
            AddSubControl(Helper.CreateContextMenuButton(this, Constants.kEvaluatorContextMenuType));
         }
      }

      public override void GetSubControlClientRects(out List<System.Drawing.Rectangle> subControlClientRects)
      {
         Controls.SubControls.ReadOnlyText noneLabel = (Controls.SubControls.ReadOnlyText)SubControlCollection[1];
         System.Drawing.Size preferredSize = noneLabel.GetPreferredSize();

         subControlClientRects = Helper.GetDefaultPropertyLayoutRects(GetClientRect(), preferredSize.Width);
      }
   }
}
