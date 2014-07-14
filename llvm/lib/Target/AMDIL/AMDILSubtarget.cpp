//===- AMDILSubtarget.cpp - AMDIL Subtarget Information -------------------===//
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
// This file implements the AMD IL specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#include "AMDILSubtarget.h"
#include "AMDIL.h"
#include "AMDILSIIOExpansion.h"
#include "AMDILUtilityFunctions.h"
#include "AMDILGenSubtarget.inc"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/SubtargetFeature.h"

using namespace llvm;

#define GET_SUBTARGETINFO_ENUM
#define GET_SUBTARGETINFO_CTOR
#define GET_SUBTARGETINFO_MC_DESC
#define GET_SUBTARGETINFO_TARGET_DESC
#include "AMDILGenSubtarget.inc"

static uint32_t getDeviceFlag(StringRef GPUName) {
  return StringSwitch<uint32_t>(GPUName)
    .Case("iceland", OCL_DEVICE_ICELAND)
    .Case("tonga", OCL_DEVICE_TONGA)
    .Case("goose", OCL_DEVICE_BERMUDA)
    .Case("hoatzin", OCL_DEVICE_FIJI)
    .Case("peacock", OCL_DEVICE_CARRIZO)
    .Case("owls", OCL_DEVICE_TIRAN)
    .Case("tiran", OCL_DEVICE_TIRAN)
    .Case("spectre", OCL_DEVICE_SPECTRE)
    .Case("spooky", OCL_DEVICE_SPOOKY)
    .Case("kalindi", OCL_DEVICE_KALINDI)
    .Case("mullins", OCL_DEVICE_GODAVARI)

    .Case("bonaire", OCL_DEVICE_BONAIRE)
    .Case("hawaii", OCL_DEVICE_HAWAII)
    .Case("eagle", OCL_DEVICE_MAUI)

    .Case("tahiti", OCL_DEVICE_TAHITI)
    .Case("pitcairn", OCL_DEVICE_PITCAIRN)
    .Case("oland", OCL_DEVICE_OLAND)
    .Case("hainan", OCL_DEVICE_HAINAN)
    .Case("capeverde", OCL_DEVICE_CAPEVERDE)

    .Case("trinity", OCL_DEVICE_TRINITY)
    .Case("caicos", OCL_DEVICE_CAICOS)
    .Case("turks", OCL_DEVICE_TURKS)
    .Case("cayman", OCL_DEVICE_CAYMAN)
    .Case("barts", OCL_DEVICE_BARTS)

    .Case("cedar", OCL_DEVICE_CEDAR)
    .Case("redwood", OCL_DEVICE_REDWOOD)
    .Case("cypress", OCL_DEVICE_CYPRESS)
    .Case("juniper", OCL_DEVICE_JUNIPER)
    .Case("generic", OCL_DEVICE_TAHITI)
    .Default(OCL_DEVICE_ALL);
}

static AMDIL::GPUFamily findGeneration(uint32_t DeviceID) {
  switch (DeviceID) {
  case OCL_DEVICE_ICELAND:
  case OCL_DEVICE_TONGA:
  case OCL_DEVICE_BERMUDA:
  case OCL_DEVICE_FIJI:
  case OCL_DEVICE_CARRIZO:
    return AMDIL::VOLCANIC_ISLANDS;

  case OCL_DEVICE_SPECTRE:
  case OCL_DEVICE_SPOOKY:
  case OCL_DEVICE_KALINDI:
  case OCL_DEVICE_GODAVARI:
  case OCL_DEVICE_BONAIRE:
  case OCL_DEVICE_HAWAII:
  case OCL_DEVICE_TIRAN:
  case OCL_DEVICE_MAUI:
    return AMDIL::SEA_ISLANDS;

  case OCL_DEVICE_TAHITI:
  case OCL_DEVICE_PITCAIRN:
  case OCL_DEVICE_OLAND:
  case OCL_DEVICE_HAINAN:
  case OCL_DEVICE_CAPEVERDE:
    return AMDIL::SOUTHERN_ISLANDS;

  case OCL_DEVICE_TRINITY:
  case OCL_DEVICE_CAICOS:
  case OCL_DEVICE_TURKS:
  case OCL_DEVICE_CAYMAN:
  case OCL_DEVICE_BARTS:
    return AMDIL::NORTHERN_ISLANDS;

  case OCL_DEVICE_CEDAR:
  case OCL_DEVICE_REDWOOD:
  case OCL_DEVICE_CYPRESS:
  case OCL_DEVICE_JUNIPER:
    return AMDIL::EVERGREEN;

  default:
    return AMDIL::INVALID_GPU_FAMILY;
  }
}

AMDILSubtarget::AMDILSubtarget(StringRef TT,
                               StringRef GPU,
                               StringRef FS)
  : AMDILGenSubtargetInfo(TT, GPU, FS),
    mDeviceName(GPU.empty() ? "generic" : GPU.str()),
    mDefaultSize(),
    MaxLDSSize(0),
    MaxGDSSize(0),
    MaxNumCBs(0),
    MaxCBSize(0),
    MaxScratchSize(65536),
    DeviceFlag(::getDeviceFlag(mDeviceName)),
    WavefrontSize(FullWavefrontSize),
    Generation(findGeneration(DeviceFlag)),
    StackAlignment(16),
    MaxNumUAVs(0),
    MaxNumSemaphores(64),
    mHWBits(),
    mSWBits(),
    mIs64bit(false),
    mSmallGlobalObjects(false),
    mMetadata30(false),
    mFlatAddress(false),
    mShortOps(false),
    mHalfOps(false),
    DynamicRegionSize(false) {
  assert(DeviceFlag != OCL_DEVICE_ALL);
  assert(getGeneration() != AMDIL::INVALID_GPU_FAMILY);
  memset(CapsOverride, 0, sizeof(CapsOverride));

  mHWBits.resize(AMDIL::Caps::MaxNumberCapabilities);
  mSWBits.resize(AMDIL::Caps::MaxNumberCapabilities);

  SmallVector<StringRef, DEFAULT_VEC_SLOTS> Features;
  SplitString(FS, Features, ",");

  if (isApple()) {
    // The driver (ati*_compute_shared.c) used to pass +mwgs-1-256
    // unconditionally in FS. That causes LLVM 3316+ to issue a meaningless
    // warning to the console. We've removed that 'feature' from FS and this
    // provides the substance of it.
    mDefaultSize[0] = 256;
  } else {
    mDefaultSize[0] = 64;
  }

  mDefaultSize[1] = 1;
  mDefaultSize[2] = 1;
  std::string newFeatures = "";
#if defined(_DEBUG) || defined(DEBUG)
  bool useTest = false;
#endif
  for (size_t x = 0; x < Features.size(); ++x) {
    if (Features[x].startswith("+mwgs")) {
      SmallVector<StringRef, DEFAULT_VEC_SLOTS> sizes;
      SplitString(Features[x], sizes, "-");

      size_t mDim;
      if (sizes[1].getAsInteger(10, mDim) || mDim > 3) {
        mDim = 3;
      }

      for (size_t y = 0; y < mDim; ++y) {
        sizes[y + 2].getAsInteger(10, mDefaultSize[y]);
      }
#if defined(_DEBUG) || defined(DEBUG)
    } else if (!Features[x].compare("test")) {
      useTest = true;
#endif
    } else if (Features[x].startswith("+cal")) {
    } else {
      if (newFeatures.length() > 0)
        newFeatures += ',';
      newFeatures += Features[x];
    }
  }
  for (int x = 0; x < 3; ++x) {
    if (!mDefaultSize[x]) {
      mDefaultSize[x] = 1;
    }
  }
#if defined(_DEBUG) || defined(DEBUG)
  if (useTest) {
    GPU = "kauai";
  }
#endif
  ParseSubtargetFeatures(GPU, newFeatures);
#if defined(_DEBUG) || defined(DEBUG)
  if (useTest) {
    GPU = "test";
  }
#endif

  setBaseCapabilities();

  switch (getGeneration()) {
  case AMDIL::EVERGREEN:
    setEvergreenCapabilities();
    break;
  case AMDIL::NORTHERN_ISLANDS:
    setNorthernIslandsCapabilities();
    break;
  case AMDIL::SOUTHERN_ISLANDS:
    setSouthernIslandsCapabilities();
    break;
  case AMDIL::SEA_ISLANDS:
    setSeaIslandsCapabilities();
    break;
  case AMDIL::VOLCANIC_ISLANDS:
    setVolcanicIslandsCapabilities();
    break;
  default:
    llvm_unreachable("Bad generation");
  }
}

AMDILSubtarget::~AMDILSubtarget() {

}

// FIXME: This set and reset bit nonsense is way over complicated.
void AMDILSubtarget::setBaseCapabilities() {
  mSWBits.set(AMDIL::Caps::HalfOps);
  mSWBits.set(AMDIL::Caps::ByteOps);
  mSWBits.set(AMDIL::Caps::ShortOps);
  mSWBits.set(AMDIL::Caps::HW64BitDivMod);
  if (isOverride(AMDIL::Caps::NoInline))
    mSWBits.set(AMDIL::Caps::NoInline);

  if (isOverride(AMDIL::Caps::MacroDB))
    mSWBits.set(AMDIL::Caps::MacroDB);

  if (isOverride(AMDIL::Caps::NoAlias))
    mSWBits.set(AMDIL::Caps::NoAlias);

  if (isApple()) {
    mSWBits.set(AMDIL::Caps::ConstantMem);
  } else {
    if (isOverride(AMDIL::Caps::Debug)) {
      mSWBits.set(AMDIL::Caps::ConstantMem);
    } else {
      mHWBits.set(AMDIL::Caps::ConstantMem);
    }
  }

  if (isOverride(AMDIL::Caps::Debug))
    mSWBits.set(AMDIL::Caps::PrivateMem);
  else
    mHWBits.set(AMDIL::Caps::PrivateMem);


  if (isOverride(AMDIL::Caps::BarrierDetect))
    mSWBits.set(AMDIL::Caps::BarrierDetect);

  mSWBits.set(AMDIL::Caps::ByteLDSOps);
  mSWBits.set(AMDIL::Caps::ByteGDSOps);
  mSWBits.set(AMDIL::Caps::LongOps);
}

void AMDILSubtarget::setEvergreenCapabilities() {
  MaxNumUAVs = 12;

  mHWBits.set(AMDIL::Caps::ByteGDSOps);
  mSWBits.reset(AMDIL::Caps::ByteGDSOps);

  mSWBits.set(AMDIL::Caps::ArenaSegment);
  mHWBits.set(AMDIL::Caps::ArenaUAV);
  mHWBits.set(AMDIL::Caps::Semaphore);
  mHWBits.set(AMDIL::Caps::HW64BitDivMod);
  mSWBits.reset(AMDIL::Caps::HW64BitDivMod);
  mSWBits.set(AMDIL::Caps::Signed24BitOps);
  if (isOverride(AMDIL::Caps::ByteStores))
    mHWBits.set(AMDIL::Caps::ByteStores);

  if (isOverride(AMDIL::Caps::Debug)) {
    mSWBits.set(AMDIL::Caps::LocalMem);
    mSWBits.set(AMDIL::Caps::RegionMem);
  } else {
    mHWBits.set(AMDIL::Caps::LocalMem);
    mHWBits.set(AMDIL::Caps::RegionMem);
  }

  if (!isApple()) {
    if (isOverride(AMDIL::Caps::Images)) {
      mHWBits.set(AMDIL::Caps::Images);
    }
  } else {
    mHWBits.set(AMDIL::Caps::Images);
  }

  mHWBits.set(AMDIL::Caps::CachedMem);
  if (isOverride(AMDIL::Caps::MultiUAV))
    mHWBits.set(AMDIL::Caps::MultiUAV);

  mHWBits.set(AMDIL::Caps::ByteLDSOps);
  mSWBits.reset(AMDIL::Caps::ByteLDSOps);
  mHWBits.set(AMDIL::Caps::ArenaVectors);
  mHWBits.set(AMDIL::Caps::LongOps);
  mSWBits.reset(AMDIL::Caps::LongOps);
  mHWBits.set(AMDIL::Caps::TmrReg);


  if (getDeviceFlag() == OCL_DEVICE_CEDAR) {
    mSWBits.set(AMDIL::Caps::FMA);
    WavefrontSize = QuarterWavefrontSize;
  } else if (getDeviceFlag() == OCL_DEVICE_REDWOOD) {
    mSWBits.set(AMDIL::Caps::FMA);
    WavefrontSize = HalfWavefrontSize;
  } else if (getDeviceFlag() == OCL_DEVICE_CYPRESS) {
    if (isOverride(AMDIL::Caps::DoubleOps)) {
      mHWBits.set(AMDIL::Caps::DoubleOps);
      mHWBits.set(AMDIL::Caps::FMA);
    }
  }

  if (usesHardware(AMDIL::Caps::ConstantMem)) {
    MaxNumCBs = HW_MAX_NUM_CB;
    MaxCBSize = MAX_CB_SIZE;
  }

  MaxLDSSize = usesHardware(AMDIL::Caps::LocalMem) ? MAX_LDS_SIZE_800 : 0;
  MaxGDSSize = usesHardware(AMDIL::Caps::RegionMem) ? MAX_GDS_SIZE_800 : 0;
}

void AMDILSubtarget::setNorthernIslandsCapabilities() {
  setEvergreenCapabilities();

  // Only Cayman is notably different from Evergreen, and Trinity inherits
  // everything from Cayman.
  if (getDeviceFlag() == OCL_DEVICE_CAYMAN || getDeviceFlag() == OCL_DEVICE_TRINITY) {
    if (isOverride(AMDIL::Caps::DoubleOps)) {
      mHWBits.set(AMDIL::Caps::DoubleOps);
      mHWBits.set(AMDIL::Caps::FMA);
    }
    mHWBits.set(AMDIL::Caps::Signed24BitOps);
    mSWBits.reset(AMDIL::Caps::Signed24BitOps);
    mSWBits.set(AMDIL::Caps::ArenaSegment);
  }

  MaxLDSSize = usesHardware(AMDIL::Caps::LocalMem) ? MAX_LDS_SIZE_900 : 0;
  MaxGDSSize = usesHardware(AMDIL::Caps::RegionMem) ? MAX_GDS_SIZE_900 : 0;
}

void AMDILSubtarget::setSouthernIslandsCapabilities() {
  setNorthernIslandsCapabilities();

  MaxNumUAVs = 1024;

  if (isOverride(AMDIL::Caps::DoubleOps)) {
    mHWBits.set(AMDIL::Caps::DoubleOps);
    mHWBits.set(AMDIL::Caps::FMA);
  }

  mHWBits.set(AMDIL::Caps::Signed24BitOps);
  mSWBits.reset(AMDIL::Caps::Signed24BitOps);
  mSWBits.set(AMDIL::Caps::ArenaSegment);

  mHWBits.set(AMDIL::Caps::PrivateUAV);
  if (isOverride(AMDIL::Caps::StackUAV)) {
    mHWBits.set(AMDIL::Caps::StackUAV);
  }
  if (isOverride(AMDIL::Caps::UseMacroForCall)) {
    mHWBits.set(AMDIL::Caps::UseMacroForCall);
  }
  mHWBits.reset(AMDIL::Caps::ArenaUAV);
  mSWBits.reset(AMDIL::Caps::ArenaSegment);
  mHWBits.reset(AMDIL::Caps::ArenaSegment);
  mHWBits.set(AMDIL::Caps::ByteStores);
  mHWBits.set(AMDIL::Caps::HW64BitDivMod);
  mSWBits.reset(AMDIL::Caps::HW64BitDivMod);
  if (!isApple()) {
    if (isOverride(AMDIL::Caps::Images)) {
      mHWBits.set(AMDIL::Caps::Images);
    }
  } else {
    mHWBits.set(AMDIL::Caps::Images);
  }

  mHWBits.set(AMDIL::Caps::CachedMem);
  mHWBits.set(AMDIL::Caps::ByteLDSOps);
  mSWBits.reset(AMDIL::Caps::ByteLDSOps);
  mHWBits.set(AMDIL::Caps::LongOps);
  mSWBits.reset(AMDIL::Caps::LongOps);
  mHWBits.set(AMDIL::Caps::TmrReg);
  mHWBits.set(AMDIL::Caps::PPAMode);
  mHWBits.reset(AMDIL::Caps::ConstantMem);
  mSWBits.set(AMDIL::Caps::ConstantMem);
  mHWBits.set(AMDIL::Caps::PrivateMem);
  mHWBits.set(AMDIL::Caps::LocalMem);
  mHWBits.set(AMDIL::Caps::RegionMem);
  mHWBits.set(AMDIL::Caps::CrossThreadOps);
}

void AMDILSubtarget::setSeaIslandsCapabilities() {
  setSouthernIslandsCapabilities();

  MaxNumUAVs = 256;

  if (isOverride(AMDIL::Caps::DoubleOps)) {
    mHWBits.set(AMDIL::Caps::DoubleOps);
    mHWBits.set(AMDIL::Caps::FMA);
  }

  mHWBits.set(AMDIL::Caps::PrivateUAV);
  mHWBits.reset(AMDIL::Caps::ArenaUAV);
  mSWBits.reset(AMDIL::Caps::ArenaSegment);
  mHWBits.reset(AMDIL::Caps::ArenaSegment);
  mHWBits.set(AMDIL::Caps::ByteStores);
  mHWBits.set(AMDIL::Caps::HW64BitDivMod);
  mSWBits.reset(AMDIL::Caps::HW64BitDivMod);
  if (!isApple()) {
    if (isOverride(AMDIL::Caps::Images)) {
      mHWBits.set(AMDIL::Caps::Images);
    }
  } else {
    mHWBits.set(AMDIL::Caps::Images);
  }
  if (isOverride(AMDIL::Caps::NoAlias)) {
    mSWBits.set(AMDIL::Caps::NoAlias);
  }
  mHWBits.set(AMDIL::Caps::CachedMem);
  mHWBits.set(AMDIL::Caps::ByteLDSOps);
  mSWBits.reset(AMDIL::Caps::ByteLDSOps);
  mHWBits.set(AMDIL::Caps::LongOps);
  mSWBits.reset(AMDIL::Caps::LongOps);
  mHWBits.set(AMDIL::Caps::TmrReg);
  mHWBits.set(AMDIL::Caps::PPAMode);
  mHWBits.set(AMDIL::Caps::FlatMem);
  mHWBits.reset(AMDIL::Caps::ConstantMem);
  mSWBits.set(AMDIL::Caps::ConstantMem);
}

void AMDILSubtarget::setVolcanicIslandsCapabilities() {
  setSeaIslandsCapabilities();

  MaxNumUAVs = 1024;

  if (isOverride(AMDIL::Caps::DoubleOps)) {
    mHWBits.set(AMDIL::Caps::DoubleOps);
    mHWBits.set(AMDIL::Caps::FMA);
  }

  mHWBits.set(AMDIL::Caps::PrivateUAV);
  mHWBits.reset(AMDIL::Caps::ArenaUAV);
  mSWBits.reset(AMDIL::Caps::ArenaSegment);
  mHWBits.reset(AMDIL::Caps::ArenaSegment);
  mHWBits.set(AMDIL::Caps::ByteStores);
  mHWBits.set(AMDIL::Caps::HW64BitDivMod);
  mSWBits.reset(AMDIL::Caps::HW64BitDivMod);
  if (!isApple()) {
    if (isOverride(AMDIL::Caps::Images)) {
      mHWBits.set(AMDIL::Caps::Images);
    }
  } else {
    mHWBits.set(AMDIL::Caps::Images);
  }
  if (isOverride(AMDIL::Caps::NoAlias)) {
    mSWBits.set(AMDIL::Caps::NoAlias);
  }
  mHWBits.set(AMDIL::Caps::CachedMem);
  mHWBits.set(AMDIL::Caps::ByteLDSOps);
  mSWBits.reset(AMDIL::Caps::ByteLDSOps);
  mHWBits.set(AMDIL::Caps::LongOps);
  mSWBits.reset(AMDIL::Caps::LongOps);
  mHWBits.set(AMDIL::Caps::TmrReg);
  mHWBits.set(AMDIL::Caps::PPAMode);
  mHWBits.set(AMDIL::Caps::FlatMem);
  mHWBits.reset(AMDIL::Caps::ConstantMem);
  mSWBits.set(AMDIL::Caps::ConstantMem);
}

unsigned AMDILSubtarget::getResourceID(AMDIL::ResourceIDType ID) const {
  if (getGeneration() == AMDIL::EVERGREEN ||
      getGeneration() == AMDIL::NORTHERN_ISLANDS) {
    switch (ID) {
    case AMDIL::RAW_UAV_ID:
      return GLOBAL_RETURN_RAW_UAV_ID;
    case AMDIL::GLOBAL_ID:
    case AMDIL::ARENA_UAV_ID:
      return DEFAULT_ARENA_UAV_ID;
    case AMDIL::CONSTANT_ID:
      if (usesHardware(AMDIL::Caps::ConstantMem))
        return GLOBAL_RETURN_RAW_UAV_ID;
      return DEFAULT_ARENA_UAV_ID;
    case AMDIL::LDS_ID:
      if (usesHardware(AMDIL::Caps::LocalMem))
        return DEFAULT_LDS_ID;
      return DEFAULT_ARENA_UAV_ID;
    case AMDIL::GDS_ID:
      if (usesHardware(AMDIL::Caps::RegionMem))
        return DEFAULT_GDS_ID;
      return DEFAULT_ARENA_UAV_ID;
    case AMDIL::SCRATCH_ID:
      if (usesHardware(AMDIL::Caps::PrivateMem))
        return DEFAULT_SCRATCH_ID;
      return DEFAULT_ARENA_UAV_ID;
    default:
      llvm_unreachable("ID type passed in is unknown!");
    }
  }

  switch (ID) {
  case AMDIL::ARENA_UAV_ID:
    llvm_unreachable("Arena UAV is not supported on SI+ device.");
  case AMDIL::RAW_UAV_ID:
  case AMDIL::GLOBAL_ID:
    return GLOBAL_RETURN_RAW_UAV_ID;
  case AMDIL::CONSTANT_ID:
    if (usesHardware(AMDIL::Caps::ConstantMem))
      return 9;
    return 10;
  case AMDIL::PRINTF_ID:
    return 9;
  case AMDIL::LDS_ID:
    if (usesHardware(AMDIL::Caps::LocalMem))
      return DEFAULT_LDS_ID;
    return getResourceID(AMDIL::GLOBAL_ID);
  case AMDIL::GDS_ID:
    if (usesHardware(AMDIL::Caps::RegionMem))
      return DEFAULT_GDS_ID;
    return getResourceID(AMDIL::GLOBAL_ID);
  case AMDIL::SCRATCH_ID:
    if (usesHardware(AMDIL::Caps::PrivateMem))
      return 8;
    return getResourceID(AMDIL::GLOBAL_ID);
  case AMDIL::STACK_UAV_ID:
    return 9;
  default:
    llvm_unreachable("ID type passed in is unknown!");
  }
}

AMDILSubtarget::ExecutionMode AMDILSubtarget::getExecutionMode(
  AMDIL::Caps::CapType Caps) const {
  if (mHWBits[Caps]) {
    assert(!mSWBits[Caps] && "Cannot set both SW and HW caps");
    return Hardware;
  }

  if (mSWBits[Caps]) {
    assert(!mHWBits[Caps] && "Cannot set both SW and HW caps");
    return Software;
  }

  return Unsupported;
}

unsigned AMDILSubtarget::getMaxStoreSizeInBits(unsigned AS) const {
  switch (AS) {
  case AMDILAS::CONSTANT_ADDRESS:
    llvm_unreachable("Cannot store to constant address space");

  default:
    return 128;
  }
}

unsigned AMDILSubtarget::getMaxLoadSizeInBits(unsigned AS) const {
  switch (AS) {
  case AMDILAS::PRIVATE_ADDRESS:
    return 128;
  case AMDILAS::LOCAL_ADDRESS:
  case AMDILAS::REGION_ADDRESS:
    return 64;
  case AMDILAS::CONSTANT_ADDRESS:
  case AMDILAS::GLOBAL_ADDRESS:
  default:
    return 128;
  }
}

std::string AMDILSubtarget::getDataLayout() const {
  if (is64bit()) {
    return std::string("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16"
                       "-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:32:32"
                       "-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64"
                       "-v96:128:128-v128:128:128-v192:256:256-v256:256:256"
                       "-v512:512:512-v1024:1024:1024-v2048:2048:2048-a0:0:64"
                       "-n32:64");
  }

  return std::string("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16"
                     "-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:32:32"
                     "-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64"
                     "-v96:128:128-v128:128:128-v192:256:256-v256:256:256"
                     "-v512:512:512-v1024:1024:1024-v2048:2048:2048-a0:0:64"
                     "-n32:64");
}

