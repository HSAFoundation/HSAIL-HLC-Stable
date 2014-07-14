//=====-- AMDILSubtarget.h - Define Subtarget for the AMDIL ----*- C++ -*-====//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
//
// This file declares the AMDIL specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#ifndef _AMDILSUBTARGET_H_
#define _AMDILSUBTARGET_H_

#include "AMDIL.h"
#include "AMDILDeviceInfo.h"
#include "llvm/ADT/BitVector.h"

#include "llvm/Target/TargetSubtargetInfo.h"
#define GET_SUBTARGETINFO_HEADER
#include "AMDILGenSubtarget.inc"

#include <cstdlib>
#include <string>

#define MAX_CB_SIZE (1 << 16)
namespace llvm {
class Module;
class AMDILSubtarget : public AMDILGenSubtargetInfo {
  private:
    std::string mDeviceName;

    size_t mDefaultSize[3];

    size_t MaxLDSSize;
    size_t MaxGDSSize;
    size_t MaxNumCBs;
    size_t MaxCBSize;
    size_t MaxScratchSize;
    uint32_t DeviceFlag;
    uint32_t WavefrontSize;
    AMDIL::GPUFamily Generation;
    unsigned StackAlignment;

    uint32_t MaxNumUAVs;
    uint32_t MaxNumSemaphores;

    BitVector mHWBits;
    BitVector mSWBits;

    bool mIs64bit;
    bool mSmallGlobalObjects;
    bool mMetadata30;
    bool mFlatAddress;
    bool mShortOps;
    bool mHalfOps;
    bool DynamicRegionSize;

    bool CapsOverride[AMDIL::Caps::MaxNumberCapabilities];

    void setBaseCapabilities();
    void setEvergreenCapabilities();
    void setNorthernIslandsCapabilities();
    void setSouthernIslandsCapabilities();
    void setSeaIslandsCapabilities();
    void setVolcanicIslandsCapabilities();

    // Each Capabilities can be executed using a hardware instruction, emulated
    // with a sequence of software instructions, or not supported at all.
    enum ExecutionMode {
      Unsupported = 0, // Unsupported feature on the card(Default value)
      Software, // This is the execution mode that is set if the
      // feature is emulated in software
      Hardware  // This execution mode is set if the feature exists
        // natively in hardware
    };

    ExecutionMode getExecutionMode(AMDIL::Caps::CapType) const;

  public:
    AMDILSubtarget(llvm::StringRef TT, llvm::StringRef CPU, llvm::StringRef FS);
    ~AMDILSubtarget();
    bool isOverride(AMDIL::Caps::CapType caps) const {
      assert(caps < AMDIL::Caps::MaxNumberCapabilities &&
             "Caps index is out of bounds!");
      return CapsOverride[caps];
    }

    bool isApple() const {
#ifdef USE_APPLE
      return true;
#else
      return false;
#endif
    }

    bool is64bit() const {
      return mIs64bit;
    }

    bool smallGlobalObjects() const {
      return mSmallGlobalObjects;
    }

    bool supportMetadata30() const {
      return mMetadata30;
    }

    bool overridesFlatAS() const {
      return mFlatAddress;
    }

    bool hasShortOps() const {
      return mShortOps;
    }

    bool hasHalfOps() const {
      return mHalfOps;
    }

    bool hasBFE() const {
      // TODO: Wire up to a proper subtarget feature.
      return true;
    }

#if 0
    bool hasGlobalExtLoads() const {
      return getGeneration() >= SOUTHERN_ISLANDS;
    }
#endif

    bool isTargetELF() const {
      return false;
    }

    StringRef getDeviceName() const {
      return mDeviceName;
    }

    size_t getDefaultSize(uint32_t dim) const {
      return (dim < 3) ? mDefaultSize[dim] : 1;
    }

    // Returns the max LDS size that the hardware supports. Size is in
    // bytes.
    size_t getMaxLDSSize() const {
      return MaxLDSSize;
    }

    // Returns the max GDS size that the hardware supports if the GDS is
    // supported by the hardware. Size is in bytes.
    size_t getMaxGDSSize() const {
      return MaxGDSSize;
    }

    // Returns the max number of hardware constant address spaces that
    // are supported by this device.
    size_t getMaxNumCBs() const {
      return MaxNumCBs;
    }

    unsigned getMaxNumReadImages() const {
      return 128;
    }

    unsigned getMaxNumWriteImages() const {
      return getGeneration() >= AMDIL::SOUTHERN_ISLANDS ? 64 : 8;
    }

    // Returns the max number of bytes a single hardware constant buffer
    // can support. Size is in bytes.
    size_t getMaxCBSize() const {
      return MaxCBSize;
    }

    // Returns the max number of bytes allowed by the hardware scratch
    // buffer. Size is in bytes.
    size_t getMaxScratchSize() const {
      return MaxScratchSize;
    }

    // Get the flag that corresponds to the device.
    uint32_t getDeviceFlag() const {
      return DeviceFlag;
    }

    // Returns the number of work-items that exist in a single hardware
    // wavefront.
    size_t getWavefrontSize() const {
      return WavefrontSize;
    }

    // Get the generational name of this specific device.
    AMDIL::GPUFamily getGeneration() const {
      return Generation;
    }

    // Get the stack alignment of this specific device.
    uint32_t getStackAlignment() const {
      return StackAlignment;
    }

    // Get the max number of UAV's for this device.
    uint32_t getMaxNumUAVs() const {
      return MaxNumUAVs;
    }

    uint32_t getMaxNumSemaphores() const {
      return MaxNumSemaphores;
    }

    bool hasDynamicRegionSize() const {
      return DynamicRegionSize;
    }

    // Get the resource ID for this specific device.
    unsigned getResourceID(AMDIL::ResourceIDType ID) const;

    // API utilizing more detailed capabilities of each family of
    // cards. If a capability is supported, then either usesHardware or
    // usesSoftware returned true. If usesHardware returned true, then
    // usesSoftware must return false for the same capability. Hardware
    // execution means that the feature is done natively by the hardware
    // and is not emulated by the softare. Software execution means
    // that the feature could be done in the hardware, but there is
    // software that emulates it with possibly using the hardware for
    // support since the hardware does not fully comply with OpenCL
    // specs.
    bool isSupported(AMDIL::Caps::CapType Mode) const {
      return getExecutionMode(Mode) != Unsupported;
    }

    bool usesHardware(AMDIL::Caps::CapType Mode) const {
      return getExecutionMode(Mode) == Hardware;
    }

    bool usesSoftware(AMDIL::Caps::CapType Mode) const {
      return getExecutionMode(Mode) == Software;
    }

    static const unsigned int MAX_LDS_SIZE_700 = 16384;
    static const unsigned int MAX_LDS_SIZE_800 = 32768;
    static const unsigned int MAX_GDS_SIZE_800 = 32768;
    static const unsigned int MAX_LDS_SIZE_900 = MAX_GDS_SIZE_800;
    static const unsigned int MAX_GDS_SIZE_900 = MAX_GDS_SIZE_800;

    static const unsigned int FullWavefrontSize = 64;
    static const unsigned int HalfWavefrontSize = 32;
    static const unsigned int QuarterWavefrontSize = 16;

    unsigned getMaxStoreSizeInBits(unsigned AS) const;
    unsigned getMaxLoadSizeInBits(unsigned AS) const;

    std::string getDataLayout() const;

    // ParseSubtargetFeatures - Parses features string setting specified
    // subtarget options. Definition of function is
    //auto generated by tblgen.
    void ParseSubtargetFeatures(StringRef CPU, StringRef FS);
  };

} // end namespace llvm

#endif // AMDILSUBTARGET_H_
