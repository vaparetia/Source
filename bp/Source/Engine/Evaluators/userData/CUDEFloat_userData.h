#pragma once

//Start:CUDEFloat_userData_declaration:68599D0A:248DC648 *** Machine generated code - do not edit ***
class CUDEFloat_userData : public IUserDataEvaluator
{
public:
   CUDEFloat_userData(CEvaluatorAllocator &allocator);
   virtual ~CUDEFloat_userData();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual void GetUserData(CEvaluatorUpdateData const &updateData, void * pBufferOut) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xABC24CA4;

   enum EProperties
   {
      kP_Identifier                                    = 0x387380BA,
      kP_Value                                         = 0xDCB67730,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   std::string                                     mIdentifier;
   boost::shared_ptr<IFloatEvaluator>              mValue;
//End:CUDEFloat_userData_declaration:68599D0A *** Machine generated code - do not edit *** $End$:CUDEFloat_userData_declaration:68599D0A

   virtual char const * GetIdentifier() const;
   virtual EDataType    GetUserDataType() const;
};

