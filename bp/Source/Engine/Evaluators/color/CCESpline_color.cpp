#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCESpline_color(gcolorEvaluatorFactoriesEngine, CCESpline_color::kType_0xB44A1B7C, CCESpline_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCESpline_color::CCESpline_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
//Start:CCESpline_color_ctor:F0B8120E:15D54739 *** Machine generated code - do not edit ***
{
//End:CCESpline_color_ctor:F0B8120E *** Machine generated code - do not edit *** $End$:CCESpline_color_ctor:F0B8120E
}

CCESpline_color::~CCESpline_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCESpline_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const time = mInput->GetValue(updateData);

   SSplineEvaluationCache rCache;
   real32 const r = mR.Evaluate(time, &rCache);
   SSplineEvaluationCache gCache;
   real32 const g = mG.Evaluate(time, &gCache);
   SSplineEvaluationCache bCache;
   real32 const b = mB.Evaluate(time, &bCache);
   SSplineEvaluationCache aCache;
   real32 const a = mA.Evaluate(time, &aCache);

   return CColorf(r, g, b, a);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCESpline_color_body:2C5139B9:9679CC61 *** Machine generated code - do not edit ***
uint32 const CCESpline_color::kType_0xB44A1B7C = 0xB44A1B7C;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCESpline_color::GetType() const
{
   return kType_0xB44A1B7C;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCESpline_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCESpline_color * pEval = new CCESpline_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCESpline_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_R:   // 0x5767DF55
         {
            mR.ConstructFromStream(applyProperties.mStream);
            break;
         }
         case kP_G:   // 0x3ABA3BBE
         {
            mG.ConstructFromStream(applyProperties.mStream);
            break;
         }
         case kP_B:   // 0x4AD0CF31
         {
            mB.ConstructFromStream(applyProperties.mStream);
            break;
         }
         case kP_A:   // 0xD3D99E8B
         {
            mA.ConstructFromStream(applyProperties.mStream);
            break;
         }
         case kP_Input:   // 0x19E91DD3
         {
            if (!applyProperties.UpdateOnly())
            {
               mInput.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mInput.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCESpline_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCESpline_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   {
      int const memberPtrOffset = (int) (((uint8*) &mR) - ((uint8*) this));
      // Pointers fixed up in CopyToLinear_Inplace
      mR.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);
   }
   {
      int const memberPtrOffset = (int) (((uint8*) &mG) - ((uint8*) this));
      // Pointers fixed up in CopyToLinear_Inplace
      mG.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);
   }
   {
      int const memberPtrOffset = (int) (((uint8*) &mB) - ((uint8*) this));
      // Pointers fixed up in CopyToLinear_Inplace
      mB.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);
   }
   {
      int const memberPtrOffset = (int) (((uint8*) &mA) - ((uint8*) this));
      // Pointers fixed up in CopyToLinear_Inplace
      mA.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);
   }
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CCESpline_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCESpline_color::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
//End:CCESpline_color_body:2C5139B9 *** Machine generated code - do not edit *** $End$:CCESpline_color_body:2C5139B9
}

#endif //NO_EVALUATOR_INITIALIZER

