using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Xml.XPath;
using System.Xml;

namespace SplineEditor
{
   // Spline class
   public class Spline
   {
      // Spline types
      public enum EType
      {
         kType_General,
         kType_R,
         kType_G,
         kType_B,
         kType_A,
         kType_X,
         kType_Y,
         kType_Z,
         kType_Bool,
         kType_Int,
      };

      // Tangent types
      public enum ETangentType
      {
         Unknown,
         Fixed,
         Clamped,
         Linear,
         Flat,
         Step,
         Plateau
      };

      public enum EInfinityType
      {
         Unknown,
         Constant,
         Linear,
         Cycle,
         CycleRelative,
         Oscillate
      };

      // Data
      public System.Xml.XmlDocument mData;
      private EType mType;
      private bool mWrapperValid;
      private Helper.SplineWrapper mWrapper;
      private Helper.SplineBounds mBounds;
      private bool mEnabled = true;

      private PointF mTangentSize = new PointF(1.0f, 1.0f);

      /// <summary> 
      /// Construct from spline xml document
      /// </summary>
      public Spline(System.Xml.XmlDocument xmlDoc, EType type)
      {
         // Set type
         mType = type;

         // Copy into xml spline data
         mData = xmlDoc;

         mWrapper = new Helper.SplineWrapper(mData);
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------
      // Wrapper functions
      //----------------------------------------------------------------------------

      internal Helper.SplineWrapper Wrapper
      {
         get
         {
            EnsureWrapperValid();
            return mWrapper;
         }
      }

      public List<XmlElement> Keys
      {
         get 
         {
            XmlNodeList foundKeys = mData.SelectNodes("//Key");
            
            List<XmlElement> keys = new List<XmlElement>(foundKeys.Count);
            
            foreach (XmlElement key in foundKeys)
               keys.Add(key);
            
            return keys; 
         }
      }

      public int GetKeyCount()
      {
         return mData.SelectNodes("//Key").Count; 
      }

      public float Evaluate(float time) { return Wrapper.Evaluate(time); }
      public Helper.SplineBounds ComputeBounds() { return Wrapper.ComputeBounds(); }

      //----------------------------------------------------------------------------

      private void EnsureWrapperValid()
      {
         if (mWrapperValid == false)
         {
            mWrapperValid = true;

            mWrapper.Update(mData);
            mBounds = mWrapper.ComputeBounds();
         }
      }

      //----------------------------------------------------------------------------
      // Misc functions
      //----------------------------------------------------------------------------
      
      public Helper.SplineBounds Bounds
      {
         get
         { 
            EnsureWrapperValid(); 
            return mBounds; 
         }
      }

      public bool Enabled
      {
         get { return mEnabled; }
         set { mEnabled = value; }
      }

      public EType GetSplineType() { return mType; }
      public void SetTangentSize(PointF size) { mTangentSize = size; }
      
      public String GetName()
      {
         switch (mType)
         {
            case EType.kType_General: return "General";
            case EType.kType_R: return "Red";
            case EType.kType_G: return "Green";
            case EType.kType_B: return "Blue";
            case EType.kType_A: return "Alpha";
            case EType.kType_X: return "X";
            case EType.kType_Y: return "Y";
            case EType.kType_Z: return "Z";
            case EType.kType_Bool: return "Boolean";
            case EType.kType_Int: return "Integer";
         };
         return "General";
      }
      
      public static String GetTangentNameFromType(ETangentType type)
      {
         return System.Enum.GetName(typeof(ETangentType), type);
      }

      public static ETangentType GetTangentTypeFromName(String type)
      {
         return (ETangentType)System.Enum.Parse(typeof(ETangentType), type);
      }

      public static String GetInfinityNameFromType(EInfinityType type)
      {
         return System.Enum.GetName(typeof(EInfinityType), type);
      }

      public static EInfinityType GetInfinityTypeFromName(String type)
      {
         return (EInfinityType)System.Enum.Parse(typeof(EInfinityType), type);
      }

      // Clears all selected points
      public void ClearSelection()
      {
         foreach(XmlElement key in mData.SelectNodes("//Key"))
         {
            SetKeySelected(key, false);
            SetInTangentSelected(key, false);
            SetOutTangentSelected(key, false);
         }
      }

      public void SetPreInfinity(EInfinityType type)
      {
         XmlElement splineData = mData.SelectSingleNode("SplineData") as XmlElement;
         splineData.SetAttribute("preInfinity", GetInfinityNameFromType(type));
         mWrapperValid = false;
      }

      public void SetPostInfinity(EInfinityType type)
      {
         XmlElement splineData = mData.SelectSingleNode("SplineData") as XmlElement;
         splineData.SetAttribute("postInfinity", GetInfinityNameFromType(type));
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------
      // Xml helper functions
      //----------------------------------------------------------------------------

      private int GetKeyIndexForElement(XmlElement key)
      {
         int keyIndex = 0;
         
         foreach(XmlElement currentKey in key.ParentNode.ChildNodes)
         {
            if (currentKey == key)
               return keyIndex;

            ++keyIndex;
         }

         return -1;
      }

      //----------------------------------------------------------------------------

      // Sets key attribute value
      public void SetKeyAttribute(XmlElement key, string attribute, string value)
      {
         key.SetAttribute(attribute, value);
      }

      //----------------------------------------------------------------------------

      // Returns key attribute value
      public String GetKeyAttribute(XmlElement key, string attribute, string defaultValue)
      {
         XmlAttribute attributeNode = key.GetAttributeNode(attribute) as XmlAttribute;
         if (attributeNode != null)
         {
            return attributeNode.Value;
         }

         return defaultValue;
      }

      //----------------------------------------------------------------------------

      public void SetKeyAttributeBool(XmlElement key, string attribute, bool value)
      {
         SetKeyAttribute(key, attribute, value.ToString());
      }

      //----------------------------------------------------------------------------

      public bool GetKeyAttributeBool(XmlElement key, string attribute)
      {
         return System.Boolean.Parse(GetKeyAttribute(key, attribute, System.Boolean.FalseString));
      }

      //----------------------------------------------------------------------------

      public void SetKeyAttributeFloat(XmlElement key, string attribute, float value)
      {
         SetKeyAttribute(key, attribute, value.ToString());
      }

      //----------------------------------------------------------------------------

      public float GetKeyAttributeFloat(XmlElement key, string attribute)
      {
         return (float)System.Double.Parse(GetKeyAttribute(key, attribute, "0.0"));
      }

      //----------------------------------------------------------------------------
      // Key functions
      //----------------------------------------------------------------------------

      public void SetKeySelected(XmlElement key, bool selected)
      {
         SetKeyAttributeBool(key, "isSelected", selected);
      }

      //----------------------------------------------------------------------------

      public bool GetKeySelected(XmlElement key)
      {
         return GetKeyAttributeBool(key, "isSelected");
      }

      //----------------------------------------------------------------------------

      public PointF GetKeyPoint(XmlElement key)
      {
         return new PointF(GetKeyTime(key), GetKeyValue(key));
      }

      //----------------------------------------------------------------------------

      // Sets key time
      public void SetKeyTime(XmlElement key, float time)
      {
         SetKeyAttributeFloat(key, "time", time);

         SortKeys();

         mWrapperValid = false;
      }

      private void SortKeys()
      {
         XmlNode parentNode = mData.SelectSingleNode("SplineData");

         List<XmlElement> sortedKeys = new List<XmlElement>();

         foreach (XmlElement originalKey in parentNode.ChildNodes)
            sortedKeys.Add(originalKey);

         while (parentNode.ChildNodes.Count > 0)
            parentNode.RemoveChild(parentNode.FirstChild);

         sortedKeys.Sort(new Comparison<XmlElement>(delegate(XmlElement lhs, XmlElement rhs)
         {
            float lhsTime = GetKeyTime(lhs);
            float rhsTime = GetKeyTime(rhs);
            if (lhsTime < rhsTime)
               return -1;
            else if (lhsTime > rhsTime)
               return 1;
            else
               return 0;
         }
         ));

         foreach (XmlElement sortedKey in sortedKeys)
            parentNode.AppendChild(sortedKey);
      }

      public float GetKeyTime(XmlElement key)
      {
         return GetKeyAttributeFloat(key, "time");
      }

      //----------------------------------------------------------------------------

      public void SetKeyValue(XmlElement key, float value)
      {
         key.InnerText = value.ToString();
         mWrapperValid = false;
      }

      public float GetKeyValue(XmlElement key) 
      {
         return float.Parse(key.InnerText);
      }

      //----------------------------------------------------------------------------

      public void DeleteKey(XmlElement key)
      {
         // Do not allow if last key
         if (GetKeyCount() == 1)
            return;

         key.ParentNode.RemoveChild(key);
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------

      public XmlElement AddKey(PointF P, ETangentType tangentType)
      {
         XmlElement keyElement = mData.CreateElement("Key");

         XmlElement splineDataElement = mData.SelectSingleNode("//SplineData") as XmlElement;
         splineDataElement.AppendChild(keyElement);

         // Setup new key
         SetKeyTime(keyElement, P.X);
         SetKeyValue(keyElement, P.Y);

         SetInTangentAngle(keyElement, 0.0f);
         SetInTangentWeight(keyElement, 1.0f);
         SetInTangentType(keyElement, tangentType);
         
         SetOutTangentAngle(keyElement, 0.0f);
         SetOutTangentWeight(keyElement, 1.0f);
         SetOutTangentType(keyElement, tangentType);

         mWrapperValid = false;
         
         return keyElement;
      }

      //----------------------------------------------------------------------------
      // In tangent functions
      //----------------------------------------------------------------------------

      public void SetInTangentSelected(XmlElement key, bool bSelected)
      {
         SetKeyAttributeBool(key, "inSelected", bSelected);
      }

      //----------------------------------------------------------------------------

      public bool GetInTangentSelected(XmlElement key)
      {
         return GetKeyAttributeBool(key, "inSelected");
      }

      //----------------------------------------------------------------------------

      public float GetInTangentAngle(XmlElement key)
      {
         return GetKeyAttributeFloat(key, "inAngle");
      }

      //----------------------------------------------------------------------------

      public void SetInTangentAngle(XmlElement key, float Angle)
      {
         SetKeyAttributeFloat(key, "inAngle", Angle);
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------

      public float GetInTangentWeight(XmlElement key)
      {
         return GetKeyAttributeFloat(key, "inWeight");
      }

      //----------------------------------------------------------------------------

      public void SetInTangentWeight(XmlElement key, float Weight)
      {
         SetKeyAttributeFloat(key, "inWeight", Weight);
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------

      public ETangentType GetInTangentType(XmlElement key)
      {
         return GetTangentTypeFromName(GetKeyAttribute(key, "inTangent", "Unknown"));
      }

      //----------------------------------------------------------------------------

      public void SetInTangentType(XmlElement key, ETangentType Type)
      {
         SetKeyAttribute(key, "inTangent", GetTangentNameFromType(Type));
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------

      public PointF GetInTangentPoint(XmlElement key)
      {
         PointF P = GetKeyPoint(key);
         int keyIndex = GetKeyIndexForElement(key);
         P.X -= Wrapper.GetTangentInTanX(keyIndex);// *mTangentSize.X;
         P.Y -= Wrapper.GetTangentInTanY(keyIndex);// *mTangentSize.Y;
         return P;
      }

      //----------------------------------------------------------------------------
      // Out tangent functions
      //----------------------------------------------------------------------------

      public void SetOutTangentSelected(XmlElement key, bool bSelected)
      {
         SetKeyAttributeBool(key, "outSelected", bSelected);
      }

      //----------------------------------------------------------------------------

      public bool GetOutTangentSelected(XmlElement key)
      {
         return GetKeyAttributeBool(key, "outSelected");
      }

      //----------------------------------------------------------------------------

      public float GetOutTangentAngle(XmlElement key)
      {
         return GetKeyAttributeFloat(key, "outAngle");
      }

      //----------------------------------------------------------------------------

      public void SetOutTangentAngle(XmlElement key, float Angle)
      {
         SetKeyAttributeFloat(key, "outAngle", Angle);
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------

      public float GetOutTangentWeight(XmlElement key)
      {
         return GetKeyAttributeFloat(key, "outWeight");
      }

      //----------------------------------------------------------------------------

      public void SetOutTangentWeight(XmlElement key, float Weight)
      {
         SetKeyAttributeFloat(key, "outWeight", Weight);
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------

      public ETangentType GetOutTangentType(XmlElement key)
      {
         return GetTangentTypeFromName(GetKeyAttribute(key, "outTangent", "Unknown"));
      }

      //----------------------------------------------------------------------------

      public void SetOutTangentType(XmlElement key, ETangentType Type)
      {
         SetKeyAttribute(key, "outTangent", GetTangentNameFromType(Type));
         mWrapperValid = false;
      }

      //----------------------------------------------------------------------------

      public PointF GetOutTangentPoint(XmlElement key)
      {
         PointF P = GetKeyPoint(key);
         
         int keyIndex = GetKeyIndexForElement(key);

         P.X += Wrapper.GetTangentOutTanX(keyIndex);
         P.Y += Wrapper.GetTangentOutTanY(keyIndex);

         return P;
      }

      //----------------------------------------------------------------------------

   };
}
