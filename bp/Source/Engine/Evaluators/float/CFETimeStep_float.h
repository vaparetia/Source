#pragma once

//Start:CFETimeStep_float_declaration:B34FBEC8:03A62A2C *** Machine generated code - do not edit ***
class CFETimeStep_float : public IFloatEvaluator
{
public:
   CFETimeStep_float(CEvaluatorAllocator &allocator);
   virtual ~CFETimeStep_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x885478A7;

   enum EProperties
   {
      kP_Scale                                         = 0x2D870A80,
      kP_Offset                                        = 0x5EA6CFE6,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mScale;
   float                                           mOffset;
//End:CFETimeStep_float_declaration:B34FBEC8 *** Machine generated code - do not edit *** $End$:CFETimeStep_float_declaration:B34FBEC8
   CEvalFloatData                                  mLastTime;
};

