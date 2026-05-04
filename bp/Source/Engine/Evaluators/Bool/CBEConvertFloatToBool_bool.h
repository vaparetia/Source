#pragma once

//Start:CBEConvertFloatToBool_bool_declaration:42F8BDE5:32FFF3EA *** Machine generated code - do not edit ***
class CBEConvertFloatToBool_bool : public IBoolEvaluator
{
public:
   CBEConvertFloatToBool_bool(CEvaluatorAllocator &allocator);
   virtual ~CBEConvertFloatToBool_bool();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual bool GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x63EBD56A;

   enum EProperties
   {
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CBEConvertFloatToBool_bool_declaration:42F8BDE5 *** Machine generated code - do not edit *** $End$:CBEConvertFloatToBool_bool_declaration:42F8BDE5

};

