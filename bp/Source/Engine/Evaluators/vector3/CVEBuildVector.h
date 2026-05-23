#pragma once

//Start:CVEBuildVector_declaration:EEC16A26:9C364A7A *** Machine generated code - do not edit ***
class CVEBuildVector : public IVectorEvaluator
{
public:
   CVEBuildVector(CEvaluatorAllocator &allocator);
   virtual ~CVEBuildVector();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x50BE592D;

   enum EProperties
   {
      kP_X                                             = 0xB7B2364B,
      kP_Y                                             = 0xC0B506DD,
      kP_Z                                             = 0x59BC5767,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mX;
   boost::shared_ptr<IFloatEvaluator>              mY;
   boost::shared_ptr<IFloatEvaluator>              mZ;
//End:CVEBuildVector_declaration:EEC16A26 *** Machine generated code - do not edit *** $End$:CVEBuildVector_declaration:EEC16A26

};

