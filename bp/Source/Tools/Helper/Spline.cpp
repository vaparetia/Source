//----------------------------------------------------------------------------
// Spline.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "Spline.h"

//----------------------------------------------------------------------------

#include "Engine/Math/CMayaSpline.h"
#include "Engine/Mechanics/TTokenSet.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"
#include "Helper.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Xml;

//----------------------------------------------------------------------------

TTokenSet<CMayaSplineKey::ETangentType> const skTangentTypeTokens[] =
{
   { "Clamped",   CMayaSplineKey::kTangent_Clamped },
   { "Fast",      CMayaSplineKey::kTangent_Fast    },
   { "Fixed",     CMayaSplineKey::kTangent_Fixed   },
   { "Flat",      CMayaSplineKey::kTangent_Flat    },
   { "Linear",    CMayaSplineKey::kTangent_Linear  },
   { "Slow",      CMayaSplineKey::kTangent_Slow    },
   { "Smooth",    CMayaSplineKey::kTangent_Smooth  },
   { "Step",      CMayaSplineKey::kTangent_Step    },
   { "Plateau",   CMayaSplineKey::kTangent_Plateau },
   { NULL,        CMayaSplineKey::kTangent_Clamped }
};

//----------------------------------------------------------------------------

TTokenSet<CSpline::EInfinity> const skInfinityTokens[] =
{
   { "Constant",        CSpline::kInfinity_Constant      },
   { "Cycle",           CSpline::kInfinity_Cycle         },
   { "CycleRelative",   CSpline::kInfinity_CycleRelative },
   { "Linear",          CSpline::kInfinity_Linear        },
   { "Oscillate",       CSpline::kInfinity_Oscillate     },
   { NULL,              CSpline::kInfinity_Constant      }
};

//----------------------------------------------------------------------------

System::Xml::XmlDocument^ Helper::Spline::GetXmlForMayaSpline( CMayaSpline const & spline )
{
   XmlDocument^ newDocument = gcnew XmlDocument();
   
   XmlElement^ splineElement = newDocument->CreateElement("SplineData");

   splineElement->SetAttribute( "preInfinity", gcnew String( skInfinityTokens->GetNameByToken( spline.mPreInfinity ) ) );
   splineElement->SetAttribute( "postInfinity", gcnew String( skInfinityTokens->GetNameByToken( spline.mPostInfinity ) ) );
   splineElement->SetAttribute( "isWeighted", spline.mIsWeighted.ToString() );

   for( int i = 0; i < spline.mKeys.size(); ++i )
   {
      CMayaSplineKey const & key = spline.mKeys[i];
      XmlElement^ keyElement = newDocument->CreateElement("Key");

      keyElement->SetAttribute( "time", key.mTime.ToString() );
      keyElement->InnerText = key.mValue.ToString();
      keyElement->SetAttribute( "inTangent", gcnew String( skTangentTypeTokens->GetNameByToken( key.mInTangentType ) ) );
      keyElement->SetAttribute( "outTangent", gcnew String( skTangentTypeTokens->GetNameByToken( key.mOutTangentType ) ) );
      keyElement->SetAttribute( "inAngle", key.mInAngle.AsDegrees().ToString() );
      keyElement->SetAttribute( "inWeight", key.mInWeight.ToString() );
      keyElement->SetAttribute( "outAngle", key.mOutAngle.AsDegrees().ToString() );
      keyElement->SetAttribute( "outWeight", key.mOutWeight.ToString() );

      splineElement->AppendChild(keyElement);
   }

   newDocument->AppendChild(splineElement);

   return newDocument;
}

//----------------------------------------------------------------------------

CMayaSpline Helper::Spline::GetMayaSplineForXml( System::Xml::XmlDocument^ doc )
{
   CSpline::EInfinity preInfinity = CSpline::kInfinity_Constant;
   CSpline::EInfinity postInfinity = CSpline::kInfinity_Constant;
   bool isWeighted = false;

   std::vector<CMayaSplineKey> outKeys;
      
   XmlElement^ splineElement = (XmlElement^)doc->SelectSingleNode("//SplineData");
   if( splineElement != nullptr )
   {
      preInfinity = skInfinityTokens->GetTokenNoCase( Helper::StringHelper::ConvertString( splineElement->GetAttribute("preInfinity") ) );
      postInfinity = skInfinityTokens->GetTokenNoCase( Helper::StringHelper::ConvertString( splineElement->GetAttribute("postInfinity") ) );
      isWeighted = System::Boolean::Parse( splineElement->GetAttribute("isWeighted") );

      XmlNodeList^ keys = splineElement->SelectNodes("./Key");
      for( int i = 0; i < keys->Count; ++i )
      {
         XmlElement^ key = (XmlElement^)keys[i];
         real32 time = Single::Parse( key->GetAttribute( "time" ) );
         real32 value = Single::Parse( key->InnerText );

         CMayaSplineKey::ETangentType inTangent = skTangentTypeTokens->GetTokenNoCase( Helper::StringHelper::ConvertString( key->GetAttribute( "inTangent" ) ) );
         CMayaSplineKey::ETangentType outTangent = skTangentTypeTokens->GetTokenNoCase( Helper::StringHelper::ConvertString( key->GetAttribute( "outTangent" ) ) );
         CAngle inAngle = CAngle::FromDegrees( Single::Parse( key->GetAttribute( "inAngle" ) ) );
         real32 inWeight = Single::Parse( key->GetAttribute( "inWeight" ) );
         CAngle outAngle = CAngle::FromDegrees( Single::Parse( key->GetAttribute( "outAngle" ) ) );
         real32 outWeight = Single::Parse( key->GetAttribute( "outWeight" ) );

         outKeys.push_back( CMayaSplineKey( time, value, inTangent, outTangent, inAngle, inWeight, outAngle, outWeight ) );
      }
   }

   return CMayaSpline( outKeys, isWeighted, preInfinity, postInfinity );
}

//----------------------------------------------------------------------------

array<System::Byte>^ Helper::Spline::CookMayaSpline( System::Xml::XmlDocument^ doc )
{
   // build maya spline from xml document
   CMayaSpline mayaSpline = GetMayaSplineForXml( doc );

   // create engine spline
   CSpline spline = mayaSpline.BuildSpline();

   // write spline to memory stream
   CGrowableMemoryOutStream stream;
   spline.PutTo( stream );

   // copy data into CLR array
   array<System::Byte>^ data = gcnew array<System::Byte>( stream.GetDataSize() );
   Marshal::Copy( (IntPtr)(void*)stream.GetData(), data, 0, stream.GetDataSize() );
   
   return data;
}

//----------------------------------------------------------------------------

void Helper::Spline::ProcessCustomProperty( System::Xml::XmlNodeReader ^propertyNode, System::String ^propertySubType, System::String ^propertyValue, System::IO::BinaryWriter ^writer )
{
   if (propertySubType == "CSpline")
   {
      XmlDocument ^splineDoc = gcnew XmlDocument();
      if (propertyValue->Length == 0)
      {
         System::Console::WriteLine("Empty spline data in Helper::Spline::ProcessCustomProperty.");
      }
      else
      {
         splineDoc->LoadXml(propertyValue);
      }

      array<System::Byte> ^splineData = Helper::Spline::CookMayaSpline(splineDoc);
      
      writer->Write(splineData);
   }
}

//----------------------------------------------------------------------------

Helper::SplineWrapper::SplineWrapper( System::Xml::XmlDocument^ doc )
{
    // Create internals
    mpMayaSpline = new CMayaSpline( Helper::Spline::GetMayaSplineForXml( doc ) );
    mpSpline     = new CSpline( mpMayaSpline->BuildSpline() );
    mpCache      = new SSplineEvaluationCache();
}

//----------------------------------------------------------------------------

Helper::SplineWrapper::~SplineWrapper()
{
    delete mpMayaSpline;
    delete mpSpline;
    delete mpCache;
}

//----------------------------------------------------------------------------

void Helper::SplineWrapper::Update( System::Xml::XmlDocument^ doc )
{
    // Update splines
    *mpMayaSpline = Helper::Spline::GetMayaSplineForXml( doc );
    *mpSpline     = mpMayaSpline->BuildSpline();
    
    // Clear the cache
    mpCache->mLastIndex = -1;
    mpCache->mLastInterval = -1;
    mpCache->mIsStep = false;
    mpCache->mIsLinear = false;
    mpCache->mX1 = 0.0f;
    mpCache->mX4 = 0.0f;
}

//----------------------------------------------------------------------------

Helper::SplineBounds^ Helper::SplineWrapper::ComputeBounds( void )
{
    // Clear bounds
    SplineBounds^ Bounds = gcnew SplineBounds;
    Bounds->mTimeMin = 0.0f;
    Bounds->mTimeMax = 0.0f;
    Bounds->mValueMin = 0.0f;
    Bounds->mValueMax = 0.0f;

    // Compute bounds
    const CSpline& Spline = *mpSpline;
    if( Spline.mKeys.size() )
    {
        // Setup bounds from first key
        const CSplineKey& Key0 = Spline.mKeys[ 0 ];
        Bounds->mTimeMin = Key0.mTime;
        Bounds->mTimeMax = Key0.mTime;
        Bounds->mValueMin = Key0.mValue;
        Bounds->mValueMax = Key0.mValue;

        // Check against rest of keys
        for( int i = 1; i < Spline.mKeys.size(); i++ )
        {
            const CSplineKey& Key = Spline.mKeys[ i ];
          
            // Update time
            float Time = Key.mTime;
            if( Time < Bounds->mTimeMin )
                Bounds->mTimeMin = Key.mTime;
            if( Time > Bounds->mTimeMax )
                Bounds->mTimeMax = Key.mTime;

            // Update value
            float Value = Key.mValue;
            if( Value < Bounds->mValueMin )
                Bounds->mValueMin = Key.mValue;
            if( Value > Bounds->mValueMax )
                Bounds->mValueMax = Key.mValue;
        }
    }
    return Bounds;
}

//----------------------------------------------------------------------------

const float Helper::SplineWrapper::Evaluate( const float Time )
{
    const CSpline& Spline = *mpSpline;
    return Spline.Evaluate( Time, mpCache );
}

//----------------------------------------------------------------------------

const int Helper::SplineWrapper::GetKeyCount( void )
{
    const CSpline& Spline = *mpSpline;
    return Spline.mKeys.size();
}

//----------------------------------------------------------------------------

const float Helper::SplineWrapper::GetKeyTime( const int KeyIndex )
{
    const CSpline&    Spline = *mpSpline;
    const CSplineKey& Key    = Spline.mKeys[ KeyIndex ];
    return Key.mTime;
}

//----------------------------------------------------------------------------

const float Helper::SplineWrapper::GetKeyValue( const int KeyIndex )
{
    const CSpline&    Spline = *mpSpline;
    const CSplineKey& Key    = Spline.mKeys[ KeyIndex ];
    return Key.mValue;
}

//----------------------------------------------------------------------------

const float Helper::SplineWrapper::GetTangentInTanX( const int KeyIndex )
{
    const CSpline&    Spline = *mpSpline;
    const CSplineKey& Key    = Spline.mKeys[ KeyIndex ];
    return Key.mInTangent.mX;
}

//----------------------------------------------------------------------------

const float Helper::SplineWrapper::GetTangentInTanY( const int KeyIndex )
{
    const CSpline&    Spline = *mpSpline;
    const CSplineKey& Key    = Spline.mKeys[ KeyIndex ];
    return Key.mInTangent.mY;
}

//----------------------------------------------------------------------------

const float Helper::SplineWrapper::GetTangentOutTanX( const int KeyIndex )
{
    const CSpline&    Spline = *mpSpline;
    const CSplineKey& Key    = Spline.mKeys[ KeyIndex ];
    return Key.mOutTangent.mX;
}

//----------------------------------------------------------------------------

const float Helper::SplineWrapper::GetTangentOutTanY( const int KeyIndex )
{
    const CSpline&    Spline = *mpSpline;
    const CSplineKey& Key    = Spline.mKeys[ KeyIndex ];
    return Key.mOutTangent.mY;
}

//----------------------------------------------------------------------------
