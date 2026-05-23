#pragma once

//Start:CCEInterpolateSpeed_color_declaration:B93635B6:A1737BFB *** Machine generated code - do not edit ***
class CCEInterpolateSpeed_color : public IColorEvaluator
{
public:
   CCEInterpolateSpeed_color(CEvaluatorAllocator &allocator);
   virtual ~CCEInterpolateSpeed_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xD07A7E67;

   enum EProperties
   {
      kP_Start                                         = 0x5EB87A8B,
      kP_End                                           = 0x38B17551,
      kP_Speed                                         = 0xCEE7D1F2,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IColorEvaluator>              mStart;
   boost::shared_ptr<IColorEvaluator>              mEnd;
   boost::shared_ptr<IFloatEvaluator>              mSpeed;
//End:CCEInterpolateSpeed_color_declaration:B93635B6 *** Machine generated code - do not edit *** $End$:CCEInterpolateSpeed_color_declaration:B93635B6
   CEvalColorfData                                 mCurrentColor;
   CEvalFloatData                                  mLastTime;
};

