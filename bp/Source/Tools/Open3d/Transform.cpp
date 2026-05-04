//----------------------------------------------------------------------------
// Transform.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "Transform.h"

//----------------------------------------------------------------------------

#include "Tools/Helper/Helper.h"

//----------------------------------------------------------------------------

using namespace Open3d;

//----------------------------------------------------------------------------

Transform::Transform( XmlNode^ node )
: Entity( (XmlElement^) node )
{
}

//----------------------------------------------------------------------------

Transform^ Transform::Attach( XmlNode^ node )
{
   if( node->Name == Transform::NodeTypeName )
      return gcnew Transform( node );

   return nullptr;
}

//----------------------------------------------------------------------------

Transform^ Transform::Create( XmlDocument^ doc )
{
   XmlElement ^ node = doc->CreateElement( Transform::NodeTypeName );

   Transform^ nodeFn = gcnew Transform( node );

   return nodeFn;
}

//----------------------------------------------------------------------------

void Transform::Translation::set( CVector3 value )
{
   XmlNode^ node = Node[ "Translation" ];
   if( node == nullptr )
   {
      node = Node->OwnerDocument->CreateElement( "Translation" );
      Node->AppendChild( node );
   }

   node->InnerText = gcnew String( value.ToString().c_str() );
}

//----------------------------------------------------------------------------

CVector3 Transform::Translation::get()
{
   XmlNode^ node = Node[ "Translation" ];

   if( node != nullptr )
      return CVector3( Helper::StringHelper::ConvertString( node->InnerText ) );

   return CVector3::Zero();
}

//----------------------------------------------------------------------------

void Transform::Orientation::set( CMatrix3 value )
{
   XmlNode^ node = Node[ "Orientation" ];
   if( node == nullptr )
   {
      node = Node->OwnerDocument->CreateElement( "Orientation" );
      Node->AppendChild( node );
   }

   node->InnerText = gcnew String( value.ToString().c_str() );
}

//----------------------------------------------------------------------------

CMatrix3 Transform::Orientation::get()
{
   XmlNode^ node = Node[ "Orientation" ];

   if( node != nullptr )
      return CMatrix3( Helper::StringHelper::ConvertString( node->InnerText ) );

   return CMatrix3::Identity();
}

//----------------------------------------------------------------------------

void Transform::Scale::set( CVector3 value )
{
   XmlNode^ node = Node[ "Scale" ];
   if( node == nullptr )
   {
      node = Node->OwnerDocument->CreateElement( "Scale" );
      Node->AppendChild( node );
   }

   node->InnerText = gcnew String( value.ToString().c_str() );
}

//----------------------------------------------------------------------------

CVector3 Transform::Scale::get()
{
   XmlNode^ node = Node[ "Scale" ];

   if( node != nullptr )
      return CVector3( Helper::StringHelper::ConvertString( node->InnerText ) );

   return CVector3::Zero();
}

//----------------------------------------------------------------------------

CMatrix34 Transform::GetTransform()
{
   return CMatrix34::Translation( Translation ) * CMatrix34::FromMatrix3( Orientation ) * CMatrix34::Scale( Scale );
}

//----------------------------------------------------------------------------

void Transform::SetTransform(CMatrix34 const & transform)
{
   Translation = transform.GetTranslation();

   CMatrix3 orientationScaled = CMatrix3::FromMatrix34(transform);
  
   Orientation = orientationScaled.OrthoNormalized();

   CVector3 const scale( orientationScaled.GetRow(0).GetLength(), orientationScaled.GetRow(1).GetLength(), orientationScaled.GetRow(2).GetLength());
   Scale = scale;
}

//----------------------------------------------------------------------------

void Transform::PreMultiplyInplace(Transform^ lhs)
{
   SetTransform(lhs->GetTransform() * GetTransform());
}

//----------------------------------------------------------------------------

void Transform::PreMultiplyInverseInplace(Transform^ lhs)
{
   SetTransform(lhs->GetTransform().QuickInverse() * GetTransform());
}

//----------------------------------------------------------------------------

void Transform::PostMultiplyInplace(Transform^ rhs)
{
   SetTransform(GetTransform() * rhs->GetTransform());
}

//----------------------------------------------------------------------------

void Transform::PostMultiplyInverseInplace(Transform^ rhs)
{
   SetTransform(GetTransform() * rhs->GetTransform().QuickInverse());
}
