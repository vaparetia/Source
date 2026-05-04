//------------------------------------------------------------------------------------------
// BPEConstants.h
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"

const uint8  gkUint8Min          = 0;
const uint8  gkUint8Max          = 255;

const uint16 gkUint16Min         = 0;
const uint16 gkUint16Max         = 65535;

const uint32 gkUint32Min         = 0;
const uint32 gkUint32Max         = 4294967295UL;

const uint64 gkUint64Min         = 0;
const uint64 gkUint64Max         = 18446744073709551615ULL;

const int8   gkInt8Min           = (-127 - 1);
const int8   gkInt8Max           =   127;

const int16  gkInt16Min          = (-32767 - 1);
const int16  gkInt16Max          =   32767;

const int32  gkInt32Min          = (-2147483647 - 1);
const int32  gkInt32Max          =   2147483647;

const int64  gkInt64Min          = (-9223372036854775807LL - 1);
const int64  gkInt64Max          =   9223372036854775807LL;

const real32 gkReal32MaxNeg      = -3.402823466e+38F;       // -FLT_MAX
const real32 gkReal32Min         =  1.175494351e-38F;       // FLT_MIN
const real32 gkReal32Max         =  3.402823466e+38F;       // FLT_MAX

const real64 gkReal64MaxNeg      = -1.7976931348623158e+308;   // -DBL_MAX
const real64 gkReal64Min         =  2.2250738585072014e-308;   //  DBL_MIN
const real64 gkReal64Max         =  1.7976931348623158e+308;   //  DBL_MAX

const real32 gkEpsilon32         = 1.192092896e-07f;           // "the smallest value such that 1 + value != 1"

const real32 gkPi32              = 3.14159265358979232846264338327950288419716939937510f;
const real32 gkPiOverTwo32       = (::gkPi32 / 2.0f);
const real32 gkPiOverFour32      = (::gkPi32 / 4.0f);
const real32 gkTwoPi32           = (::gkPi32 * 2.0f);
const real32 gkFourPi32          = (::gkPi32 * 4.0f);

const real64 gkEpsilon64         = 1.08420217248550443412e-019L;  // "the smallest value such that 1 + value != 1"

const real64 gkPi64              = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651L;
const real64 gkPiOverTwo64       = (::gkPi64 / 2.0);
const real64 gkPiOverFour64      = (::gkPi64 / 4.0);
const real64 gkTwoPi64           = (::gkPi64 * 2.0);
const real64 gkFourPi64          = (::gkPi64 * 4.0);

const real32 gkDegrees2Radians32 = gkPi32 / real32(180.0);
const real64 gkDegrees2Radians64 = gkPi64 / real64(180.0);

const real32 gkRadians2Degrees32 = real32(180.0) / gkPi32;
const real64 gkRadians2Degrees64 = real64(180.0) / gkPi64;

const real32 gkDefaultFrameTime  = 1.0f / 60.0f;

enum EAxis
{
   kAX,
   kAY,
   kAZ,
   kAW
};

// Helper functions to prevent the compiler from complaining.
inline bool return_true()      { return true; };
inline bool return_false()     { return false; };

