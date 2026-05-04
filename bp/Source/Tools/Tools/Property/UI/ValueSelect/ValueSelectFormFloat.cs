using System;
using System.Collections.Generic;
using System.Text;

namespace Tools.Property.UI.ValueSelect
{
   class ValueSelectFormFloat : ValueSelectForm
   {
      float    mStartingValue;
      float    mScalar  = 1.0f;
      float?   mMinValue;
      float?   mMaxValue;

      public ValueSelectFormFloat(SetValueDelegate setValue, GetValueDelegate getValue, float startingValue, float? minValue, float? maxValue)
      : base(setValue, getValue)
      {
         mStartingValue = startingValue;
         mCancelValue = startingValue.ToString();
         mScalar = (float) Math.Max(0.25, Math.Abs(startingValue));
         mMinValue = minValue;
         mMaxValue = maxValue;
      }
      
      override public string GetDesiredValue( float fractionX, float fractionY )
      {
         float newValue = mStartingValue + mScalar * fractionX * ((float) Math.Pow(10.0f, -fractionY));
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
