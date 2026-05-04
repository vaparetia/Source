#pragma once

//Start:CUDEString_userData_declaration:9B253122:FD26F45D *** Machine generated code - do not edit ***
class CUDEString_userData : public IUserDataEvaluator
{
public:
   CUDEString_userData(CEvaluatorAllocator &allocator);
   virtual ~CUDEString_userData();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual void GetUserData(CEvaluatorUpdateData const &updateData, void * pBufferOut) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x864BF971;

   enum EProperties
   {
      kP_Identifier                                    = 0x387380BA,
      kP_Value                                         = 0xDCB67730,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   std::string                                     mIdentifier;
   std::string                                     mValue;
//End:CUDEString_userData_declaration:9B253122 *** Machine generated code - do not edit *** $End$:CUDEString_userData_declaration:9B253122

   virtual char const * GetIdentifier() const;
   virtual EDataType    GetUserDataType() const;
};

