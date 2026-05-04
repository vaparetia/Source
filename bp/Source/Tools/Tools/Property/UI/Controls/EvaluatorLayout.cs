using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Drawing;
using Tools.Property.Misc;

namespace Tools.Property.UI.Controls
{
   public class EvaluatorLayout : BaseGroupBarLayout
   {
      static SolidBrush kEvaluatorBarFillBrush = new SolidBrush(GetEvaluatorBarColor(0));

      public Data.EvaluatorProperty EvaluatorPropertyDataNode { get { return (Data.EvaluatorProperty) mDataElement; } }

      public EvaluatorLayout(PropertyPanel panel, Data.DataNode dataElement, XmlElement uiStateElement)
         : base(panel, dataElement, uiStateElement, Constants.kEvaluatorContextMenuType)
      {
         GetDisplayName = OnGetDisplayName;

         FillBrush = kEvaluatorBarFillBrush;

         // If we are grouped we don't display any controls for the evaluator
         if (!mDataElement.IsGrouped)
         {
            mPanel.PropertyInterface.BuildControls(mPanel, this, EvaluatorPropertyDataNode.Evaluator);
         }
      }

      public string OnGetDisplayName(PropertyControl control)
      {
         string name = mDataElement.DisplayName;
         string activeEvaluator = EvaluatorPropertyDataNode.ActiveEvaluatorType;

         return String.Format("{0} : {1}", activeEvaluator, name );
      }

      private static Color GetEvaluatorBarColor(int nestingLevel)
      {
         return Tools.Common.Misc.Lerp(
            Color.FromArgb(185, 235, 185),
            Color.FromArgb(255, 255, 185),
            Tools.Common.Misc.Clamp(nestingLevel, 0, 8) / 8.0f
            );
      }
   }
}
