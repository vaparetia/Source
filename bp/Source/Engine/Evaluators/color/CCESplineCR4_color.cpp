#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCESplineCR4_color(gcolorEvaluatorFactoriesEngine, CCESplineCR4_color::kType_0x471A3631, CCESplineCR4_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCESplineCR4_color::CCESplineCR4_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
//Start:CCESplineCR4_color_ctor:394242A6:15D54739 *** Machine generated code - do not edit ***
{
//End:CCESplineCR4_color_ctor:394242A6 *** Machine generated code - do not edit *** $End$:CCESplineCR4_color_ctor:394242A6
}

CCESplineCR4_color::~CCESplineCR4_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCESplineCR4_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
   // We'll just clamp colors outside time ranges for now.
   // Maybe add a flag to change this later.
   // Clamping seems like the most useful thing for colors.

   real32 const input = mInput->GetValue(updateData);
   real32 const time1 = mTime1->GetValue(updateData);
   CColorf const color1 = mColor1->GetValue(updateData);
   if (input < time1)
   {
      return color1;
   }

   real32 const time2 = mTime2->GetValue(updateData);
   CColorf const color2 = mColor2->GetValue(updateData);
   CColorf const color3 = mColor3->GetValue(updateData);

   if (input < time2)
   {
      // Interpolated color
      CColorf const color0((color1 * 2.0f) - color2);
      real32 const s = (input - time1) / (time2 - time1);

      return *reinterpret_cast<CColorf const*>(&MathUtils::CatmullRomSpline4((CVector4 const&)color0,
                                                                             (CVector4 const&)color1,
                                                                             (CVector4 const&)color2,
                                                                             (CVector4 const&)color3,
                                                                             s));
   }
   real32 const time3 = mTime3->GetValue(updateData);
   CColorf const color4 = mColor4->GetValue(updateData);

   if (input < time3)
   {
      // Interpolated color
      real32 const s = (input - time2) / (time3 - time2);

      return *reinterpret_cast<CColorf const*>(&MathUtils::CatmullRomSpline4((CVector4 const&)color1,
                                                                             (CVector4 const&)color2,
                                                                             (CVector4 const&)color3,
                                                                             (CVector4 const&)color4,
                                                                             s));

   }
   real32 const time4 = mTime4->GetValue(updateData);
   if (input > time4)
   {
      return color4;
   }

   // Interpolated color
   CColorf const color5((color4 * 2.0f) - color3);
   real32 const s = (input - time3) / (time4 - time3);

   return *reinterpret_cast<CColorf const*>(&MathUtils::CatmullRomSpline4((CVector4 const&)color2,
                                                                          (CVector4 const&)color3,
                                                                          (CVector4 const&)color4,
                                                                          (CVector4 const&)color5,
                                                                          s));
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCESplineCR4_color_body:E5AB6911:C08D6B4F *** Machine generated code - do not edit ***
uint32 const CCESplineCR4_color::kType_0x471A3631 = 0x471A3631;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCESplineCR4_color::GetType() const
{
   return kType_0x471A3631;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCESplineCR4_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCESplineCR4_color * pEval = new CCESplineCR4_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCESplineCR4_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Time1:   // 0x44C4E134
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime1.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime1.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Color1:   // 0x5DC0E615
         {
            if (!applyProperties.UpdateOnly())
            {
               mColor1.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mColor1.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Time2:   // 0xDDCDB08E
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime2.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime2.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Color2:   // 0xC4C9B7AF
         {
            if (!applyProperties.UpdateOnly())
            {
               mColor2.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mColor2.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Time3:   // 0xAACA8018
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime3.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime3.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Color3:   // 0xB3CE8739
         {
            if (!applyProperties.UpdateOnly())
            {
               mColor3.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mColor3.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Time4:   // 0x34AE15BB
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime4.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime4.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Color4:   // 0x2DAA129A
         {
            if (!applyProperties.UpdateOnly())
            {
               mColor4.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mColor4.get(), applyProperties, size, allocator);
            }
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCESplineCR4_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCESplineCR4_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mTime1)
   {
      uint32 const evalPtr = mTime1->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime1) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mColor1)
   {
      uint32 const evalPtr = mColor1->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mColor1) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mTime2)
   {
      uint32 const evalPtr = mTime2->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime2) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mColor2)
   {
      uint32 const evalPtr = mColor2->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mColor2) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mTime3)
   {
      uint32 const evalPtr = mTime3->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime3) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mColor3)
   {
      uint32 const evalPtr = mColor3->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mColor3) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mTime4)
   {
      uint32 const evalPtr = mTime4->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime4) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mColor4)
   {
      uint32 const evalPtr = mColor4->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mColor4) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
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

void CCESplineCR4_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mTime1) mTime1->PostLoadUpdate();
   if (mColor1) mColor1->PostLoadUpdate();
   if (mTime2) mTime2->PostLoadUpdate();
   if (mColor2) mColor2->PostLoadUpdate();
   if (mTime3) mTime3->PostLoadUpdate();
   if (mColor3) mColor3->PostLoadUpdate();
   if (mTime4) mTime4->PostLoadUpdate();
   if (mColor4) mColor4->PostLoadUpdate();
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCESplineCR4_color::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mTime1.get()) mTime1->Initialize(updateData);
   if (mColor1.get()) mColor1->Initialize(updateData);
   if (mTime2.get()) mTime2->Initialize(updateData);
   if (mColor2.get()) mColor2->Initialize(updateData);
   if (mTime3.get()) mTime3->Initialize(updateData);
   if (mColor3.get()) mColor3->Initialize(updateData);
   if (mTime4.get()) mTime4->Initialize(updateData);
   if (mColor4.get()) mColor4->Initialize(updateData);
   if (mInput.get()) mInput->Initialize(updateData);
//End:CCESplineCR4_color_body:E5AB6911 *** Machine generated code - do not edit *** $End$:CCESplineCR4_color_body:E5AB6911
}

#endif //NO_EVALUATOR_INITIALIZER

