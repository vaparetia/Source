//----------------------------------------------------------------------------
// Spline.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

class CMayaSpline;
class CSpline;
struct SSplineEvaluationCache;

//----------------------------------------------------------------------------

namespace Helper
{
	public ref class Spline
   {
   public:
      static System::Xml::XmlDocument^ GetXmlForMayaSpline( CMayaSpline const & spline );
      static CMayaSpline GetMayaSplineForXml( System::Xml::XmlDocument^ doc );
      static array<System::Byte>^ CookMayaSpline( System::Xml::XmlDocument^ doc );
      static void ProcessCustomProperty( System::Xml::XmlNodeReader ^propertyNode, System::String ^propertySubType, System::String ^propertyValue, System::IO::BinaryWriter ^writer );
	};

   public ref struct SplineBounds
   {
      float   mTimeMin;
      float   mTimeMax;
      float   mValueMin;
      float   mValueMax;
   };

   public ref class SplineWrapper
   {
   // Functions
   public:
                     SplineWrapper( System::Xml::XmlDocument^ doc );
                     ~SplineWrapper();

      void           Update            ( System::Xml::XmlDocument^ doc );
      SplineBounds^  ComputeBounds     ();
      const float    Evaluate          ( const float Time );
      const int      GetKeyCount       ();
      const float    GetKeyTime        ( const int KeyIndex );
      const float    GetKeyValue       ( const int KeyIndex );
      const float    GetTangentInTanX  ( const int KeyIndex );
      const float    GetTangentInTanY  ( const int KeyIndex );
      const float    GetTangentOutTanX ( const int KeyIndex );
      const float    GetTangentOutTanY ( const int KeyIndex );

   // Data
   private:
      CMayaSpline*                     mpMayaSpline;
      CSpline*                         mpSpline;
      mutable SSplineEvaluationCache*  mpCache;
    };
}
