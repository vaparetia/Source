#pragma once

//Start:CVESpline_vector_declaration:16AB044D:927B5FFD *** Machine generated code - do not edit ***
class CVESpline_vector : public IVectorEvaluator
{
public:
   CVESpline_vector(CEvaluatorAllocator &allocator);
   virtual ~CVESpline_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x9BD933C3;

   enum EProperties
   {
      kP_X                                             = 0xB7B2364B,
      kP_Y                                             = 0xC0B506DD,
      kP_Z                                             = 0x59BC5767,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   CSpline                                         mX;
   CSpline                                         mY;
   CSpline                                         mZ;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CVESpline_vector_declaration:16AB044D *** Machine generated code - do not edit *** $End$:CVESpline_vector_declaration:16AB044D

   // Return custom information for time ranges.
   virtual void GetInfo(STypeInfo &info) const;
};

