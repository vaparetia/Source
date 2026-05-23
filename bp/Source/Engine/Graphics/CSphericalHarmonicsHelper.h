//------------------------------------------------------------------------------------------
// CSphericalHarmonicsHelper.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Graphics/CSphericalHarmonics.h"
#include "Engine/Graphics/CSphericalHarmonics3.h"
#include "Engine/Math/CVector3.h"

//------------------------------------------------------------------------------------------

/**
 * IRadianceFunction interface.
 * 
 */

class IRadianceFunction
{
public:
   virtual ~IRadianceFunction() {}

   /// returns transfer in given direction
   virtual CVector3 const Evaluate( CVector3 const & direction ) const = 0;
};

//------------------------------------------------------------------------------------------

/**
 * CSphericalHarmonicsHelper class.
 * 
 */
 
class CSphericalHarmonicsHelper
{
public:
   ENGINE_API static void SampleSphere( IRadianceFunction const & transferFunction,
                                        int const sqrtNumSamples,
                                        CSphericalHarmonics & out );

   ENGINE_API static void SampleSphere( IRadianceFunction const & transferFunction,
                                        int const sqrtNumSamples,
                                        CSphericalHarmonics3 & out );

   ENGINE_API static void AddDirectionalLight( CVector3 const & direction,
                                               CVector3 const & color,
                                               CSphericalHarmonics3 & out );

   ENGINE_API static void AddAmbientLight( CVector3 const & color,
                                           CSphericalHarmonics3 & out );

private:
   static void SetupSamples( int const sqrtNumSamples,
                             std::vector< CVector3 > & samples );
};


