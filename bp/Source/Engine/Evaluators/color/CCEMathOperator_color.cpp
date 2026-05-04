#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEMathOperator_color(gcolorEvaluatorFactoriesEngine, CCEMathOperator_color::kType_0x5B02DE8C, CCEMathOperator_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEMathOperator_color::CCEMathOperator_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
//Start:CCEMathOperator_color_ctor:19CCAF1C:431D2E68 *** Machine generated code - do not edit ***
{
   mOperator = kOP_Multiply;
//End:CCEMathOperator_color_ctor:19CCAF1C *** Machine generated code - do not edit *** $End$:CCEMathOperator_color_ctor:19CCAF1C
}

CCEMathOperator_color::~CCEMathOperator_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEMathOperator_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
	CColorf const a = mA->GetValue(updateData);
	CColorf const b = mB->GetValue(updateData);
   switch (mOperator)
   {
      case kOP_Add:
			return (a + b);
		case kOP_Subtract:
			return (a - b);
		case kOP_Multiply:
         return CColorf(a.GetR() * b.GetR(),
                        a.GetG() * b.GetG(),
                        a.GetB() * b.GetB(),
                        a.GetA() * b.GetA());
		default:
			BPE_ASSERTA("Unknown operator");
   }

   return CColorf::Zero();
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEMathOperator_color_body:C52584AB:574E35A0 *** Machine generated code - do not edit ***
uint32 const CCEMathOperator_color::kType_0x5B02DE8C = 0x5B02DE8C;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEMathOperator_color::GetType() const
{
   return kType_0x5B02DE8C;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEMathOperator_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEMathOperator_color * pEval = new CCEMathOperator_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEMathOperator_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            }
            break;
         }
         case kP_A:   // 0xD3D99E8B
         {
            if (!applyProperties.UpdateOnly())
            {
               mA.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
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
               mB.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mB.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEMathOperator_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEMathOperator_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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

void CCEMathOperator_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mA) mA->PostLoadUpdate();
   if (mB) mB->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEMathOperator_color::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mA.get()) mA->Initialize(updateData);
   if (mB.get()) mB->Initialize(updateData);
//End:CCEMathOperator_color_body:C52584AB *** Machine generated code - do not edit *** $End$:CCEMathOperator_color_body:C52584AB
}

#endif //NO_EVALUATOR_INITIALIZER

