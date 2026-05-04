#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEMathOperator_float(gfloatEvaluatorFactoriesEngine, CFEMathOperator_float::kType_0xF4F1C8F0, CFEMathOperator_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEMathOperator_float::CFEMathOperator_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFEMathOperator_float_ctor:90B71DCB:431D2E68 *** Machine generated code - do not edit ***
{
   mOperator = kOP_Multiply;
//End:CFEMathOperator_float_ctor:90B71DCB *** Machine generated code - do not edit *** $End$:CFEMathOperator_float_ctor:90B71DCB
}

CFEMathOperator_float::~CFEMathOperator_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEMathOperator_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
	real32 const a = mA->GetValue(updateData);
	real32 const b = mB->GetValue(updateData);
   switch (mOperator)
   {
      case kOP_Add:
			return (a + b);
		case kOP_Subtract:
			return (a - b);
		case kOP_Multiply:
			return (a * b);
		case kOP_Divide:
			return (a / b);
		case kOP_Modulo:
			return fmodf(a, b);
		case kOP_Max:
			return bpe::max_val(a, b);
		case kOP_Min:
			return bpe::min_val(a, b);
		case kOP_Sine:
 			return b * sinf(a * gkDegrees2Radians32);
		case kOP_Cosine:
			return b * cosf(a * gkDegrees2Radians32);
		case kOP_Abs:
			return b * fabsf(a);
      case kOP_Ceil:
         return b * ceilf(a);
      case kOP_Floor:
         return b * floorf(a);
      case kOP_Round:
         return b * floorf(a + 0.5f);
		default:
			BPE_ASSERTA("Unknown operator");

   }
	return 0;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEMathOperator_float_body:4C5E367C:065763F1 *** Machine generated code - do not edit ***
uint32 const CFEMathOperator_float::kType_0xF4F1C8F0 = 0xF4F1C8F0;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEMathOperator_float::GetType() const
{
   return kType_0xF4F1C8F0;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEMathOperator_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEMathOperator_float * pEval = new CFEMathOperator_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEMathOperator_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
               case 0xFEB877DB:   // kOP_Multiply - Output = A * B
                  mOperator = kOP_Multiply;
                  break;
               case 0x7EED5862:   // kOP_Divide - Output = A / B
                  mOperator = kOP_Divide;
                  break;
               case 0x11825A1C:   // kOP_Modulo - Output = A % B
                  mOperator = kOP_Modulo;
                  break;
               case 0x1BD4129C:   // kOP_Max - Output = max(A, B)
                  mOperator = kOP_Max;
                  break;
               case 0x27D92DC5:   // kOP_Min - Output = min(A, B)
                  mOperator = kOP_Min;
                  break;
               case 0xB435B5E4:   // kOP_Sine - Output = B * sin( deg(A) )
                  mOperator = kOP_Sine;
                  break;
               case 0x219985C2:   // kOP_Cosine - Output = B * cos( deg(A) )
                  mOperator = kOP_Cosine;
                  break;
               case 0xAE3161B3:   // kOP_Abs - Output = B * abs( A )
                  mOperator = kOP_Abs;
                  break;
               case 0xDBAB357C:   // kOP_Ceil - Output = B * ceil(A)
                  mOperator = kOP_Ceil;
                  break;
               case 0xAF298634:   // kOP_Floor - Output = B * floor(A)
                  mOperator = kOP_Floor;
                  break;
               case 0xD482BA2E:   // kOP_Round - Output = B * round(A)
                  mOperator = kOP_Round;
                  break;
            }
            break;
         }
         case kP_A:   // 0xD3D99E8B
         {
            if (!applyProperties.UpdateOnly())
            {
               mA.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
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
               mB.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mB.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEMathOperator_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEMathOperator_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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

void CFEMathOperator_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mA) mA->PostLoadUpdate();
   if (mB) mB->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEMathOperator_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mA.get()) mA->Initialize(updateData);
   if (mB.get()) mB->Initialize(updateData);
//End:CFEMathOperator_float_body:4C5E367C *** Machine generated code - do not edit *** $End$:CFEMathOperator_float_body:4C5E367C
}

#endif //NO_EVALUATOR_INITIALIZER

