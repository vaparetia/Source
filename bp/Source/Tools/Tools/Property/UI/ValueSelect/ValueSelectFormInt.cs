using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.Property.UI.ValueSelect
{
   class ValueSelectFormInt : ValueSelectForm
   {
      int    mStartingValue;
      float  mScalar  = 1.0f;
      int?   mMinValue;
      int?   mMaxValue;

      public ValueSelectFormInt(SetValueDelegate setValue, GetValueDelegate getValue, int startingValue, int? minValue, int? maxValue)
      : base(setValue, getValue)
      {
         mStartingValue = startingValue;
         mCancelValue = startingValue.ToString();
         mScalar = Math.Max(10, (int) Math.Abs(startingValue));
         mMinValue = minValue;
         mMaxValue = maxValue;
      }
      
      override public String GetDesiredValue( float fractionX, float fractionY )
      {
         // Quantize this so int values don't jump about on Y changes
         float quantizedFractionY = (float) System.Convert.ToInt32(fractionY * 20.0f) / 20.0f;
         float newValueTemp = ((float) mStartingValue) + mScalar * fractionX * ((float) Math.Pow(10.0f, -quantizedFractionY + 0.5f));
         int newValue = (int) newValueTemp;
         if (mMaxValue.HasValue)
         {
            newValue = Math.Min(newValue, mMaxValue.Value);
         }
         if (mMinValue.HasValue)
         {
            newValue = Math.Max(newValue, mMinValue.Value);
         }

         return newValue.ToString();
      }
   }
}

