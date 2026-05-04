#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "Engine/Math/BPEMath.h"

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/optional.hpp"

#include "boost/foreach.hpp"
#define foreach BOOST_FOREACH

#if BPE_TARGET == BPE_TARGET_WIN32
#include "d3d9.h"
#include "d3dx9.h"
#endif

