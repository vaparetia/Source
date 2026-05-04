//------------------------------------------------------------------------------------------
// DataArray.cpp
//------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "DataArray.h"

//------------------------------------------------------------------------------------------

using namespace Open3d;

//------------------------------------------------------------------------------------------

DataArray::DataArray( XmlNode^ node )
: NamedEntity( (XmlElement^) node )
{
}

//------------------------------------------------------------------------------------------

DataArray^ DataArray::Attach( XmlNode^ node )
{
   if( node->Name == DataArray::NodeTypeName )
      return gcnew DataArray( node );

   return nullptr;
}

//------------------------------------------------------------------------------------------

DataArray^ DataArray::Create( XmlDocument^ doc, String^ name )
{
   XmlElement ^ node = doc->CreateElement( DataArray::NodeTypeName );
   
   DataArray^ nodeFn = gcnew DataArray( node );
   nodeFn->Name = name;
   // Initialize item count to prevent crashes when accessing uninitialized arrays.
   nodeFn->Node->SetAttribute( "count", "0" );

   return nodeFn;
}

//------------------------------------------------------------------------------------------

void DataArray::SetData( array<float>^ data )
{
   pin_ptr<float> srcData = &data[0];
   InternalSetData( srcData, data->Length, EType::Real, sizeof(float) );
}

void DataArray::SetData( TFloatVector const & data )
{
   InternalSetData( (void*)&data[0], data.size(), EType::Real, sizeof(float) );
}

void DataArray::GetData( array<float>^ & data )
{
   if (Count == 0) return;

   data = gcnew array<float>( Count );

   pin_ptr<float> dataPtr = &data[0];
   InternalGetData( dataPtr, Count, EType::Real, sizeof(float) );
}

void DataArray::GetData( TFloatVector & data )
{
   if (Count == 0) return;

   data.resize( Count );
   InternalGetData( (void*)( &data[0] ), data.size(), EType::Real, sizeof(float) );
}

//------------------------------------------------------------------------------------------

void DataArray::SetData( array<Vector2>^ data )
{
   pin_ptr<Vector2> srcData = &data[0];
   InternalSetData( srcData, data->Length, EType::Vector2, sizeof(CVector2) );
}

void DataArray::SetData( TVector2Vector const & data )
{
   InternalSetData( (void*)( &data[0] ), data.size(), EType::Vector2, sizeof(CVector2) );
}

void DataArray::GetData( array<Vector2>^ & data )
{
   if (Count == 0) return;

   data = gcnew array<Vector2>( Count );

   pin_ptr<Vector2> dataPtr = &data[0];
   InternalGetData( dataPtr, Count, EType::Vector2, sizeof(CVector2) );
}

void DataArray::GetData( TVector2Vector & data )
{
   if (Count == 0) return;

   data.resize( Count, CVector2::Zero() );
   InternalGetData( (void*)( &data[0] ), data.size(), EType::Vector2, sizeof(CVector2) );
}

//------------------------------------------------------------------------------------------

void DataArray::SetData( array<Vector3>^ data )
{
   pin_ptr<Vector3> srcData = &data[0];
   InternalSetData( srcData, data->Length, EType::Vector3, sizeof(CVector3) );
}

void DataArray::SetData( TVector3Vector const & data )
{
   InternalSetData( (void*)( &data[0] ), data.size(), EType::Vector3, sizeof(CVector3) );
}

void DataArray::GetData( array<Vector3>^ & data )
{
   if (Count == 0) return;

   data = gcnew array<Vector3>( Count );

   pin_ptr<Vector3> dataPtr = &data[0];
   InternalGetData( dataPtr, Count, EType::Vector3, sizeof(CVector3) );
}

void DataArray::GetData( TVector3Vector & data )
{
   if (Count == 0) return;

   data.resize( Count, CVector3::Zero() );
   InternalGetData( (void*)( &data[0] ), data.size(), EType::Vector3, sizeof(CVector3) );
}

//------------------------------------------------------------------------------------------

void DataArray::SetData( array<Vector4>^ data )
{
   pin_ptr<Vector4> srcData = &data[0];
   InternalSetData( srcData, data->Length, EType::Vector4, sizeof(CVector4) );
}

void DataArray::SetData( TVector4Vector const & data )
{
   InternalSetData( (void*)( &data[0] ), data.size(), EType::Vector4, sizeof(CVector4) );
}

void DataArray::GetData( array<Vector4>^ & data )
{
   if (Count == 0) return;

   data = gcnew array<Vector4>( Count );

   pin_ptr<Vector4> dataPtr = &data[0];
   InternalGetData( dataPtr, Count, EType::Vector4, sizeof(CVector4) );
}

void DataArray::GetData( TVector4Vector & data )
{
   if (Count == 0) return;

   data.resize( Count, CVector4::Zero() );
   InternalGetData( (void*)( &data[0] ), data.size(), EType::Vector4, sizeof(CVector4) );
}

//------------------------------------------------------------------------------------------

void DataArray::SetData( array<Quat4>^ data )
{
   pin_ptr<Quat4> srcData = &data[0];
   InternalSetData( srcData, data->Length, EType::Quat4, sizeof(CQuaternion) );
}

void DataArray::SetData( TQuat4Vector const & data )
{
   InternalSetData( (void*)( &data[0] ), data.size(), EType::Quat4, sizeof(Quat4) );
}

void DataArray::GetData( array<Quat4>^ & data )
{
   if (Count == 0) return;

   data = gcnew array<Quat4>( Count );

   pin_ptr<Quat4> dataPtr = &data[0];
   InternalGetData( dataPtr, Count, EType::Quat4, sizeof(CQuaternion) );
}

void DataArray::GetData( TQuat4Vector & data )
{
   if (Count == 0) return;

   data.resize( Count, CQuaternion::Identity() );
   InternalGetData( (void*)( &data[0] ), data.size(), EType::Quat4, sizeof(CQuaternion) );
}

//------------------------------------------------------------------------------------------

void DataArray::SetData( array<Color>^ data )
{
   pin_ptr<Color> srcData = &data[0];
   InternalSetData( srcData, data->Length, EType::Color, sizeof(CColor) );
}

void DataArray::SetData( TColorVector const & data )
{
   InternalSetData( (void*)( &data[0] ), data.size(), EType::Color, sizeof(CColor) );
}

void DataArray::GetData( array<Color>^ & data )
{
   if (Count == 0) return;

   data = gcnew array<Color>( Count );

   pin_ptr<Color> dataPtr = &data[0];
   InternalGetData( dataPtr, Count, EType::Color, sizeof(CColor) );
}

void DataArray::GetData( TColorVector & data )
{
   if (Count == 0) return;

   data.resize( Count, CColor::Zero() );
   InternalGetData( (void*)( &data[0] ), data.size(), EType::Color, sizeof(CColor) );
}

//------------------------------------------------------------------------------------------

void DataArray::SetData( array<ColorF>^ data )
{
   pin_ptr<ColorF> srcData = &data[0];
   InternalSetData( srcData, data->Length, EType::ColorF, sizeof(CColorf) );
}

void DataArray::SetData( TColorFVector const & data )
{
   InternalSetData( (void*)( &data[0] ), data.size(), EType::ColorF, sizeof(CColorf) );
}

void DataArray::GetData( array<ColorF>^ & data )
{
   if (Count == 0) return;

   data = gcnew array<ColorF>( Count );

   pin_ptr<ColorF> dataPtr = &data[0];
   InternalGetData( dataPtr, Count, EType::ColorF, sizeof(CColorf) );
}

void DataArray::GetData( TColorFVector & data )
{
   if (Count == 0) return;

   data.resize( Count );
   InternalGetData( (void*)( &data[0] ), data.size(), EType::ColorF, sizeof(CColorf) );
}

//------------------------------------------------------------------------------------------

void DataArray::SetData( array<Guid>^ data )
{
   if( data->Length == 0 )
   {
      InternalSetData( NULL, 0, EType::Guid, 0 );
   }
   else
   {
      pin_ptr<Guid> srcData = &data[0];
      InternalSetData( srcData, data->Length, EType::Guid, sizeof(CGuid) );
   }
}

void DataArray::SetData( TGuidVector const & data )
{
   if( data.empty() )
   {
      InternalSetData( NULL, 0, EType::Guid, 0 );
   }
   else
   {
      InternalSetData( (void*)( &data[0] ), data.size(), EType::Guid, sizeof(CGuid) );
   }
}

void DataArray::GetData( array<Guid>^ & data )
{
   if( Count > 0 )
   {
      data = gcnew array<Guid>( Count );
   
      pin_ptr<Guid> dataPtr = &data[0];
      InternalGetData( dataPtr, Count, EType::Guid, sizeof(CGuid) );
   }
}

void DataArray::GetData( TGuidVector & data )
{
   if( Count > 0 )
   {
      data.resize( Count, CGuid::Null() );
      InternalGetData( (void*)( &data[0] ), data.size(), EType::Guid, sizeof(CGuid) );
   }
}

//------------------------------------------------------------------------------------------

void DataArray::InternalSetData( void * pData, int count, EType type, int elementSize )
{
   Node->SetAttribute( "type", type.ToString() );
   Node->SetAttribute( "count", count.ToString() );

   int const destSize = elementSize * count;

   if( destSize )
   {
      array<Byte>^ dstData = gcnew array<Byte>( destSize );
      Marshal::Copy( (IntPtr)pData, dstData, 0, destSize );
   
      Node->InnerText = System::Convert::ToBase64String( dstData );
   }
}

void DataArray::InternalGetData( void * pData, int count, EType type, int elementSize )
{
   if( DataType != type )
      throw gcnew Exception( "Invalid data type" );

   array<unsigned char>^ data = System::Convert::FromBase64String( Node->InnerText );
   Marshal::Copy( data, 0, (IntPtr)pData, count * elementSize );

}

