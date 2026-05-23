#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVEMathOperator_vector(gvector3EvaluatorFactoriesEngine, CVEMathOperator_vector::kType_0x268B891A, CVEMathOperator_vector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVEMathOperator_vector::CVEMathOperator_vector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
//Start:CVEMathOperator_vector_ctor:D21AEF6A:431D2E68 *** Machine generated code - do not edit ***
{
   mOperator = kOP_Multiply;
//End:CVEMathOperator_vector_ctor:D21AEF6A *** Machine generated code - do not edit *** $End$:CVEMathOperator_vector_ctor:D21AEF6A
}

CVEMathOperator_vector::~CVEMathOperator_vector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVEMathOperator_vector::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CVector3 a = mA->GetValue(updateData);
   CVector3 const b = mB->GetValue(updateData);
   switch (mOperator)
   {
   case kOP_Add:
      return (a + b);
   case kOP_Subtract:
      return (a - b);
   case kOP_Multiply:
      return CVector3::ElementMultiply(a, b);
   case kOP_Divide:
      return CVector3(a.mX/b.mX, a.mY/b.mY, a.mZ/b.mZ);
   case kOP_Max:
      // pick the max by element... (see ClampMin)
      a.ClampMin(b);
      return a;
   case kOP_Min:
      // pick the min by element... (see ClampMax)
      a.ClampMax(b);
      return a;
   default:
      BPE_ASSERTA("Unknown operator");

   }
   return CVector3::Zero();
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVEMathOperator_vector_body:0EF3C4DD:619A978B *** Machine generated code - do not edit ***
uint32 const CVEMathOperator_vector::kType_0x268B891A = 0x268B891A;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVEMathOperator_vector::GetType() const
{
   return kType_0x268B891A;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVEMathOperator_vector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVEMathOperator_vector * pEval = new CVEMathOperator_vector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVEMathOperator_vector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Operator:   // 0x2ED0C5D7
         {
            uint32 const enumFourCC = applyProperties.mStream.ReadUint32();
            switch (enumFourCC)
            {
               case 0x7BB843F2:   // kOP_Add - Output = A + B
                  mOperator = kOP_Add;
                  break;
               case 0xCE564C19:   // kOP_Subtract - Output = A - B
                  mOperator = kOP_Subtract;
                  break;
               case 0xFEB877DB:   // kOP_Multiply - Output = by element A * B
                  mOperator = kOP_Multiply;
                  break;
               case 0x7EED5862:   // kOP_Divide - Output = by element A / B
                  mOperator = kOP_Divide;
                  break;
               case 0x1BD4129C:   // kOP_Max - Output = by element max(A, B)
                  mOperator = kOP_Max;
                  break;
               case 0x27D92DC5:   // kOP_Min - Output = by element min(A, B)
                  mOperator = kOP_Min;
                  break;
            }
            break;
         }
         case kP_A:   // 0xD3D99E8B
         {
            if (!applyProperties.UpdateOnly())
            {
               mA.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mA.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_B:   // 0x4AD0CF31
         {
            if (!applyProperties.UpdateOnly())
            {
               mB.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mB.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVEMathOperator_vector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVEMathOperator_vector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mA)
   {
      uint32 const evalPtr = mA->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mA) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mB)
   {
      uint32 const evalPtr = mB->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mB) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CVEMathOperator_vector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mA) mA->PostLoadUpdate();
   if (mB) mB->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVEMathOperator_vector::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mA.get()) mA->Initialize(updateData);
   if (mB.get()) mB->Initialize(updateData);
//End:CVEMathOperator_vector_body:0EF3C4DD *** Machine generated code - do not edit *** $End$:CVEMathOperator_vector_body:0EF3C4DD
}
#endif //NO_EVALUATOR_INITIALIZER


