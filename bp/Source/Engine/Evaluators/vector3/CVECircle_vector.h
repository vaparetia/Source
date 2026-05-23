#pragma once

//Start:CVECircle_vector_declaration:FD295C6A:C24F25B4 *** Machine generated code - do not edit ***
class CVECircle_vector : public IVectorEvaluator
{
public:
   CVECircle_vector(CEvaluatorAllocator &allocator);
   virtual ~CVECircle_vector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x7B3B15F6;

   enum EProperties
   {
      kP_Angle                                         = 0xC1406A0D,
      kP_Radius                                        = 0x3CD06B6C,
      kP_UpAxis                                        = 0x102A2EF0,
      kP_Center                                        = 0x475CEE12,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mAngle;
   boost::shared_ptr<IFloatEvaluator>              mRadius;
   boost::shared_ptr<IVectorEvaluator>             mUpAxis;
   boost::shared_ptr<IVectorEvaluator>             mCenter;
//End:CVECircle_vector_declaration:FD295C6A *** Machine generated code - do not edit *** $End$:CVECircle_vector_declaration:FD295C6A
};

