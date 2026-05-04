using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Xml;
using Tools.Property;
using Tools.Property.UI;
using Tools.Property.UI.Controls;
using Tools.Property.UI.Controls.SubControls;
using Tools.Property.Misc;

namespace CoreTools.Property.UI
{
   public class PropertySpline : PropertyControl
   {
      class ModalSplineDlg : System.Windows.Forms.Form
      {
         public SplineEditor.SplineEditorControl mControl;

         public ModalSplineDlg()
         {
            InitializeComponent();
         }

         private void InitializeComponent()
         {
            this.mControl = new SplineEditor.SplineEditorControl();

            this.SuspendLayout();
            // 
            // mControl
            // 
            this.mControl.BackColor = System.Drawing.Color.Transparent;
            this.mControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mControl.Location = new System.Drawing.Point(0, 0);
            this.mControl.Name = "mControl";
            this.mControl.Size = new System.Drawing.Size(642, 472);
            this.mControl.TabIndex = 0;
            // 
            // ModalSplineDlg
            // 
            this.ClientSize = new System.Drawing.Size(642, 472);
            this.Controls.Add(this.mControl);
            this.Name = "ModalSplineDlg";
            this.Text = "Spline Editor";
            this.ResumeLayout(false);

         }
      }
      
      class SplineDataHandle
      {
         public string PropertyName;
         public SplineEditor.Spline.EType Type;
         public XmlDocument SplineData;

         public SplineDataHandle(string propertyName, SplineEditor.Spline.EType type)
         {
            PropertyName = propertyName;
            Type = type;
            SplineData = null;
         }
      }

      public Data.Property PropertyDataNode { get { return (Data.Property) mDataElement; } }

      public PropertySpline(PropertyPanel panel, Data.Property dataElement)
         : base(panel, dataElement)
      {
         BuildContextMenu = Tools.Property.UI.Controls.Helper.DefaultPropertyBuildContextMenu;

         mControlHeight = 20;

         AllocateSubControlStorage();

         // Label
         AddSubControl(CreatePropertyLabel());

         // Edit button
         {
            Button button = new Button(mPanel);
            button.Text = "Edit...";
            button.Clicked += OnEditSpline;
            AddSubControl(button);
         }

         // Context button
         {
            AddSubControl(Tools.Property.UI.Controls.Helper.CreateContextMenuButton(this, ContextMenuType.Type1));
         }
      }

      void OnEditSpline(EventArgs e, ControlInfo controlInfo, Rectangle controlRectangle)
      {
         Data.DataNode parentNode = mDataElement.Parent;
         Data.Evaluator parentAsEvaluatorNode = parentNode as Data.Evaluator;

         ModalSplineDlg dlg = new ModalSplineDlg();

         List<SplineDataHandle> splines = new List<SplineDataHandle>();

         if ( parentAsEvaluatorNode == null )
         {
            // If we are not parented by an evaluator, then we're a simple general spline
            splines.Add( new SplineDataHandle( PropertyDataNode.Name, SplineEditor.Spline.EType.kType_General ) );
         }
         else
         {
            // We're parented by an evaluator.  Use the id to figure out how many sub-spline properties we have
            string evaluatorId = parentAsEvaluatorNode.ID;

            switch (evaluatorId)
            {
               case "Spline_vector":
                  splines.Add(new SplineDataHandle("X", SplineEditor.Spline.EType.kType_X));
                  splines.Add(new SplineDataHandle("Y", SplineEditor.Spline.EType.kType_Y));
                  splines.Add(new SplineDataHandle("Z", SplineEditor.Spline.EType.kType_Z));
                  break;

               case "PPSpline_color":
                  dlg.mControl.TimeMin = 0;
                  dlg.mControl.TimeMax = 1;
                  splines.Add(new SplineDataHandle("R", SplineEditor.Spline.EType.kType_R));
                  splines.Add(new SplineDataHandle("G", SplineEditor.Spline.EType.kType_G));
                  splines.Add(new SplineDataHandle("B", SplineEditor.Spline.EType.kType_B));
                  splines.Add(new SplineDataHandle("A", SplineEditor.Spline.EType.kType_A));
                  break;

               case "Spline_color":
                  splines.Add(new SplineDataHandle("R", SplineEditor.Spline.EType.kType_R));
                  splines.Add(new SplineDataHandle("G", SplineEditor.Spline.EType.kType_G));
                  splines.Add(new SplineDataHandle("B", SplineEditor.Spline.EType.kType_B));
                  splines.Add(new SplineDataHandle("A", SplineEditor.Spline.EType.kType_A));
                  break;

               case "PPSpline_float":
                  dlg.mControl.TimeMin = 0;
                  dlg.mControl.TimeMax = 1;
                  splines.Add(new SplineDataHandle(PropertyDataNode.Name, SplineEditor.Spline.EType.kType_General));
                  break;

               default:
                  splines.Add( new SplineDataHandle( PropertyDataNode.Name, SplineEditor.Spline.EType.kType_General ) );
                  break;
            }
         }

         foreach (SplineDataHandle spline in splines)
         {
            Data.Property foundProperty = (Data.Property) parentNode.FindSubnodeOfTypeWithName( "Property", spline.PropertyName );

            if ( foundProperty != null )
            {
               XmlElement splineData = (XmlElement) foundProperty.Element.SelectSingleNode( "Value/SplineData" );

               if ( splineData != null )
               {
                  spline.SplineData = Tools.Common.XmlNodeHelpers.CreateDocumentFromNode( splineData );
                  dlg.mControl.AddSplineFromXmlDoc( spline.SplineData, spline.Type );
               }
            }
         }

         dlg.ShowDialog();

         foreach (SplineDataHandle spline in splines)
         {
            Data.Property foundProperty = (Data.Property) parentNode.FindSubnodeOfTypeWithName( "Property", spline.PropertyName );

            if ( foundProperty != null )
            {
               XmlElement splineData = (XmlElement) foundProperty.Element.SelectSingleNode( "Value/SplineData" );
               if ( splineData != null )
               {
                  XmlElement newSplineData = (XmlElement) splineData.OwnerDocument.ImportNode( Tools.Common.XmlNodeHelpers.GetDocumentRootElement( spline.SplineData ), true );
                  splineData.ParentNode.ReplaceChild(
                     newSplineData,
                     splineData );

                  // Flag property as modified so ungroup works correctly
                  foundProperty.IsModified = true;
               }
            }
         }

         SendPropertyChangedEvents(false);
      }

      public static void BuildCustomPropertyUI(PropertyEditorGUI editor, PropertyControl parentControl, Data.Property dataElement)
      {
         PropertySpline splineControl = new PropertySpline(editor.PropertyPanel, dataElement);
         parentControl.AddChild(splineControl);
      }

      public override void GetSubControlClientRects(out List<Rectangle> subControlClientRects)
      {
         subControlClientRects = Tools.Property.UI.Controls.Helper.GetDefaultPropertyLayoutRects(GetClientRect());
      }
   }
}
