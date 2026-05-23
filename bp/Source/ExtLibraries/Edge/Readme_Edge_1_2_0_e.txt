SCE CONFIDENTIAL
PlayStation(R)Edge 1.2.0
Copyright (C) 2010 Sony Computer Entertainment Inc.
All Rights Reserved.

======================================================================
This package contains following components and documents:
- Edge Geometry
- Edge Animation
- Edge Zlib
- Edge LZMA
- Edge LZO
- Edge DXT
- Edge Post
- SPA

======================================================================

----------------------------------------------------------------------
Contents of This Package
----------------------------------------------------------------------
cell\
|---Readme_Edge_X_X_X_e.txt                 This file
|---host-common
|    |---contrib                            Install FCOLLADA to this folder
|    |    +---FColladaBuild                 Project files to manage FCOLLADA
|    |---include
|    |    +---edge
|    |         |---libedgeanimtool          Edge Anim Common headers for Tools
|    |         +---libedgegeomtool          Edge Geom Common headers for Tools
|    +---src
|         +---edge
|              |---edgeanimcompiler         Edge Anim Compiler source code
|              |---edgegeomcompiler         Edge Geom Compiler source code
|              |---geomtoolsample           Edge Geom Sample Compiler
|              |---libedgeanimtool          Edge Anim tool library
|              |---libedgegeomtool          Edge Geom tool library
|              +---shared                   Shared source code
|---host-win32
|    |---bin                                Pre-build Windows executable(Including SPA)
|    +---lib                                Pre-build Windows library 
|---license
|    +---others               
|         |---FCollada.txt
|         +---PlayStation_Edge_Terms_and_Conditions_e.txt License file
|---samples
|    +---edge                               Edge samples
|---SDK_doc
|    +---en(jp)
|         +---pdf
|              +---edge                     Documents
+---target
     |---common
     |    +---include
     |         +---edge
     |              |---edge_*.h            Common headers
     |              |---anim                Edge Anim common headers
     |              |---geom                Edge Geom common headers
     |              |---lzma                Edge LZMA common headers
     |              |---lzo                 Edge LZO  common headers
     |              |---post                Edge Post common headers
     |              +---zlib                Edge Zlib common headers
     |---ppu
     |    |---include
     |    |    +---edge
     |    |         |---anim                Edge Anim PPE runtime headers
     |    |         |---lzma                Edge LZMA PPE runtime headers
     |    |         |---lzo                 Edge LZO  PPE runtime headers
     |    |         |---post                Edge Post PPE runtime headers
     |    |         +---zlib                Edge Zlib PPE runtime headers
     |    |---lib                           Edge PPE runtime binary files
     |    +---src
     |         +---edge
     |              |---anim                Edge Anim PPE runtime source code
     |              |---lzma                Edge LZMA PPE runtime source code
     |              |---lzo                 Edge LZO  PPE runtime source code
     |              |---post                Edge Post PPE runtime source code
     |              +---zlib                Edge Zlib PPE runtime source code
     |---spu
     |    |---include
     |    |    +---edge
     |    |         |---anim                Edge Anim SPE runtime headers
     |    |         |---dxt                 Edge DXT SPE runtime headers
     |    |         |---geom                Edge Geom SPE runtime headers
     |    |         |---lzma                Edge LZMA SPE runtime headers
     |    |         |---lzo                 Edge LZO  SPE runtime headers
     |    |         |---post                Edge Post SPE runtime headers
     |    |         +---zlib                Edge Zlib SPE runtime headers
     |    |---lib                           Edge SPE runtime binary files
     |    +---src
     |         +---edge
     |              |---anim                Edge Anim SPE runtime source code
     |              |---dxt                 Edge DXT SPE runtime source code
     |              |---geom                Edge Geom SPE runtime source code
     |              |---lzma                Edge LZMA SPE runtime source code
     |              |---lzma-inflate-task   Edge LZMA SPURS Inflate task source code
     |              |---lzo                 Edge LZO SPE runtime source code
     |              |---lzo1x-inflate-task  Edge LZO SPURS Inflate task source code
     |              |---lzo1x-deflate-task  Edge LZO SPURS Deflate task source code
     |              |---post                Edge Post SPE runtime source code
     |              |---zlib                Edge Zlib SPE runtime source code
     |              |---zlib-deflate-task   Edge Zlib SPURS Deflate task source code
     |              +---zlib-inflate-task   Edge Zlib SPURS Inflate task source code
     |---reference
     |    +---src
     |        +---edge
     |              |---anim                Edge Anim source code for reference
     |
     +---win32
          |---include
          |    +---edge
          |         +---anim                Edge Anim Windows version
          +---src
               +---edge
                    +---anim                Edge Anim Windows version


----------------------------------------------------------------------
New Features
----------------------------------------------------------------------

Edge Animation

 - A new dummy command has been added to EdgeAnimCommandOp so that users can add 
   their own opcodes starting at this value. This helps to guard against 
   potential bugs that can occur if the default opcode range is modified.

 - EdgeAnimSkeleton now includes an index to the locomotionBone & the version 
   tag has been incremented to reflect this change.

 - EdgeAnimAnimation now includes an offset to (optional) rotation & translation  
   deltas used for locomoting animations such as walk or run cycles. The 
   animation version tag has been incremented to reflect this change.
   
 - The edgeanimcompiler tool now includes an (optional) argument '-jointnames' as
   an example of how to encorporate custom data tables into the EdgeAnimSkeleton
   and EdgeAnimAnimation data structures.

 - Added edgeAnimCreateMirrorPairSpec() helper function to assist with the mapping 
   of joint elements in _edgeAnimMirrorJoints()
   This function unfortunately is not documented in this release but internally,
   what it does is setting up the spec parameter as described in Overview document, 
   chapter 8, section "Mirroring".
   
 - Optimized SPA versions of the _edgeAnimEvaluateST and _edgeAnimEvaluateUser
   methods have been added to the SPU edgeanim library.

 - The C++ reference implementation has been moved to 
   target/reference/src/edge/anim/
 
 - The win32 target has been renamed the windows target, and the
   target/win32/include/edge/anim/edgeanim_win32.h header file has been renamed
   target/windows/include/edge/anim/edgeanim_windows.h
   The win32 project file edgeanim_win32.vcproj still exists, as do the relevant
   win32 sample solution and project files.

 - A windows target implementation optimized for SSE2 intrinsics is now 
   available in target/windows/src/edge/anim/
   
   These methods match the strict memory alignment for the input and output data
   of the SPU implementations. Using the debug version of the edgeanim library
   will enable asserts to test for incorrect memory alignment and it is recommended
   that it is used initially to test your code for alignment faults. The win32
   edgeanim library project also now generates code for SSE2 intrinsics by default.

 - The windows SSE2 target now supports the edgeAnimEvaluateJoint and
   edgeAnimEvaluateUserChannel helper functions.

Edge Geometry

 - Significant improvements to triangle culling.  EdgeGeom now uses the same algorithm
   for sub-pixel triangle culling as the RSX, resulting in an additional 10-25% of input
   triangles being culled for average scenes.

Edge Tools

 - A new command line argument -locoJoint <name> has been added to  
   edgeanimcompiler in order to store the joint index and locomotion deltas in 
   the corresponding skeleton and/or animation resoure.

 - LibEdgeAnimTool: ExtractSkeleton no longer needs to be told the endian-ness  
   of the input binary file, instead this is determined by testing the skeleton 
   structure's version tag.

 - LibEdgeAnimTool: ExtractSkeleton now extracts the parent indices and scale
   compensation flags from the input binary file. Custom data is now the only  
   component of a skeleton structure that is not extracted.

 - LibEdgeAnimTool: ComputePeriod now accepts an array of period hints. Please  
   see code comment and/or reference doc for more details.

 - Added new functionality to edgeGeomPartitioner() to serialize all input 
   as stand-alone C++ arrays. It has been managed as an internal helper function, 
   which may be useful when reporting partitioner problems.

Edge Post

 - This version of post offers a new component which implements Morphological 
   Anti-Aliasing (MLAA). MLAA is an image based anti-aliasing post process. 
   We provide an SPU implementation, intended to run on single-sample framebuffers in 
   parallel to RSX. This results in AA that is practically free from a GPU point of view.
   
 - Added a new sample to show simple MLAA integration, this sample will load every
   image found in the asset folder and will apply MLAA to them. You can use this sample 
   to try the effect of MLAA on your game simply by dropping a few screenshots into the 
   asset folder and watch the result on the target.
   
 - Improved edgepost sample to show a more complex scenario for MLAA integration. This
   sample now implements both MLAA and edgepost effects (dof, motionblur, bloom, ilr).
 
 - Added a new sample implementing Young-van-Vliet recursive gaussian blur using the new
   inplace pseudo-transpose functionality.
 
----------------------------------------------------------------------
Changes of the Specifications
----------------------------------------------------------------------

Edge Animation

 - Parent scale compensation now uses the parent's local space scale instead of 
   it's world space scale in the local<->world conversion functions. This change 
   means that the input & output joint arrays cannot be aliased (in place 
   operation) and an assert has been added to check for this.

Edge Geometry

 - edgeGeomGetSpuVertexFormat() and edgeGeomSetSpuVertexFormat() can now take an
   optional pointer to a structure to fill in, rather than allocating one
   internally.

 - Removed some unused structure members and function arguments that were marked
   as deprecated during 1.1.x. Specifically:
   - m_cullingFlavor (member of EdgeGeomPartitionerInput)
   - m_canBeOccluded (member of EdgeGeomPartitionerInput)
   - cullType (argument to edgeGeomGetScratchBufferSizeInQwords)
   - canOcclude (argument to edgeGeomGetScratchBufferSizeInQwords)
   - m_cullType (member of EdgeGeomSegmentFormat)
   - m_canBeOccluded (member of EdgeGeomSegmentFormat)
   - numOneDimensionalTriangles (member of EdgeGeomCullingResults)
   - numZeroAreaTriangles (member of EdgeGeomCullingResults)

Edge Samples

 - The locomotion-sample has been changed to use the locomotion joint index from 
   the skeleton & the deltas from the animation instead of assuming the root  
   joint & computing the deltas during initialisation.

Edge Tools

 - Removed deprecated tools-side libedgeanim and libedgegeom headers and source.
   These files have been moved to libedgeanimtool since Edge 1.1.0.
   
Edge Zlib

- The function "longest_match" has been optimized in SPU assembler.  At level 9
  compression, this can improve the throughput of Edge Zlib compression by anything
  between 9% up to 84% depending on the data being operated on.  At level 1
  compression this optimization only increases throughput by between 4% and 13%.

- "_edgeZlibFalseStore", which is used by "_edgeZlib_inflate_fast_spu_asm" in the
  inflate task has been moved from the ".data" section to the ".bss".

Edge Post

- Rewritten edgePostFP16toFloats so to allow in place conversion of float values.

----------------------------------------------------------------------
Bug Fix
----------------------------------------------------------------------

Edge Animation

 - The reference C++ implementation of _edgeAnimMirrorJoints now swaps over the 
   joint weights to be identical in function to the SPU code.

 - The reference C++ implementation of _edgeAnimEvaluate is now 64-bit 
   addressing safe.

 - The reference C++ implementation of _edgeAnimEvaluate now checks if each type
   of channel is bitpacked or not.

 - Duplicate function prototypes for looking up joint indices have been removed 
   from edgeanim_spu.h & edgeanim_ppu.h. These prototypes were already declared 
   in edgeanim_common.h.

Edge Geometry

 - edgeGeomEndCommandBufferHole() now checks to see whether the command buffer hole
   is large enough *before* writing to it.

 - Fixed potential I/O buffer overrun in edgeGeomBeginCommandBufferHole().

 - edgeGeomCalculateDefaultOutputSize() and edgeGeomOutputVertexes() now use the same
   formula to compute the output buffer size.

Edge Tools

 - Fixed useless assert in edgeGeomMakeBlendShapeBuffer().

 - Fixed bug that would produce incorrect fixed-point offset values when more than one
   fixed-point vertex attribute was present in a stream.

Edge Post

 - Fixed a bug in edgePostFP16toFloats that was outputing the float values in a different 
   order to what they came in.
  
 - Fixed an off-by-one error that would result in banding in the motion-blur effect.
 
 - Fixed a bug inside edgepost samples that would cause any ctor/dtor declared
   inside the edgepost effect not to be called.

Edge Zlib

 - When using "kEdgeZlibDeflateTask_DeflateStoreCompressedWithHeader", the written out
   checksum was zero, meaning that if the datat was then decompressed with regular "zlib"
   rather than "Edge Zlib" it would fail the checksum test.  The correct Adler-32 value
   is now written.

----------------------------------------------------------------------
Known Bugs/Restrictions
----------------------------------------------------------------------

Edge Tools

 - If you wish to build edgegeomcompiler or edgeanimcompiler, you will
   need the FCollada library that can be downloaded from 
   http://sourceforge.net/projects/colladamaya. Please copy it to this location:
   host-common/contrib/FCollada.
   Due to the usage of FCollada, both edgegeomcompiler and 
   edgeanimcompiler are for Windows Only.

   This release has been tested with FCollada 3.05B.
   Note FCollada has to be compiled via Visual Studio on Windows platform.

Edge Geometry 

 - "libedgegeomtool.vs8.vcproj" currently lists "PS3 Debug|x64" and
   "PS3 Release|x64" configurations.  Note that these do not work.  
   Please use the "PS3 Debug|Win32" and "PS3 Release|Win32" 
   configurations instead.

Edge Samples

 - The Edge sample programs depend on some sample libraries included in
   the PlayStation(R)3 Programmer Tool Runtime Library. The following 
   projects should be built before building the samples: 
   $(SCE_PS3_ROOT)/samples/fw
   $(SCE_PS3_ROOT)/samples/sdk/graphics/gcm
   $(SCE_PS3_ROOT)/samples/common/gtf


----------------------------------------------------------------------
Permission and Restrictions on Use
----------------------------------------------------------------------
The permission and restrictions on using this package conform to 
the contract concluded between your company and our company (Sony 
Computer Entertainment Inc).  

----------------------------------------------------------------------
Note on Trademarks
----------------------------------------------------------------------
"PlayStation","PS3","RSX" and "Cell Broadband Engine" are either 
registered trademarks or trademarks of Sony Computer Entertainment Inc.

All other product and company names mentioned herein, with or without
the registered trademark symbol (R) , trademark symbol (TM) or
service mark symbol (SM) , are generally trademarks and/or registered
trademarks of their respective owners.



PlayStation(R)Edge is distributed under the terms and conditions described
in cell\license\others\PlayStation_Edge_Terms_and_Conditions_e.txt
Please refer to the file for more detail about the license.

Please report any issue, feedback or suggestion through your regular 
support channels.

cell\host-win32\bin\*.exe uses FCollada SDK. 
Please refer to FColladaSDK.txt in the "cell\license\others" folder.

