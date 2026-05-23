#pragma once

//Start:CUDEBranch2_userData_declaration:4023B5DA:69D472EE *** Machine generated code - do not edit ***
class CUDEBranch2_userData : public IUserDataEvaluator
{
public:
   CUDEBranch2_userData(CEvaluatorAllocator &allocator);
   virtual ~CUDEBranch2_userData();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual void GetUserData(CEvaluatorUpdateData const &updateData, void * pBufferOut) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xB4D0A522;

   enum EProperties
   {
      kP_Identifier                                    = 0x387380BA,
      kP_Value_A                                       = 0x6C062CB8,
      kP_Value_B                                       = 0xF50F7D02,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   std::string                                     mIdentifier;
   boost::shared_ptr<IUserDataEvaluator>           mValue_A;
   boost::shared_ptr<IUserDataEvaluator>           mValue_B;
//End:CUDEBranch2_userData_declaration:4023B5DA *** Machine generated code - do not edit *** $End$:CUDEBranch2_userData_declaration:4023B5DA

   virtual char const * GetIdentifier() const;
   virtual EDataType    GetUserDataType() const;
};

