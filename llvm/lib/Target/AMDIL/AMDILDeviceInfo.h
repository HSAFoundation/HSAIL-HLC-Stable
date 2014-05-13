//===------------------------------------------------------*- C++ -*-------===//

#ifndef _AMDILDEVICEINFO_H_
#define _AMDILDEVICEINFO_H_

namespace llvm {
  namespace AMDIL {
    // These have to be in order with the older generations
    // having the lower number enumerations.
    enum GPUFamily {
      INVALID_GPU_FAMILY = 0,
      EVERGREEN,
      NORTHERN_ISLANDS,
      SOUTHERN_ISLANDS,
      SEA_ISLANDS,
      VOLCANIC_ISLANDS,
      HDTEST // Experimental feature testing device.
    };

  namespace Caps {
    // Any changes to this needs to have a corresponding update to the twiki
    // page GPUMetadataABI.
    enum CapType {
      HalfOps          = 0x1,  // Half float is supported or not.
      DoubleOps        = 0x2,  // Double is supported or not.
      ByteOps          = 0x3,  // Byte(char) is support or not.
      ShortOps         = 0x4,  // Short is supported or not.
      LongOps          = 0x5,  // Long is supported or not.
      Images           = 0x6,  // Images are supported or not.
      ByteStores       = 0x7,  // ByteStores available(!HD4XXX).
      ConstantMem      = 0x8,  // Constant/CB memory.
      LocalMem         = 0x9,  // Local/LDS memory.
      PrivateMem       = 0xA,  // Scratch/Private/Stack memory.
      RegionMem        = 0xB,  // OCL GDS Memory Extension.
      FMA              = 0xC,  // Use HW FMA or SW FMA.
      ArenaSegment     = 0xD,  // Use for Arena UAV per pointer 12-1023.
      MultiUAV         = 0xE,  // Use for UAV per Pointer 0-7.
      PPAMode          = 0xF,  // UAV Per Pointer Allocation Mode capability
      NoAlias          = 0x10, // Cached loads.
      Signed24BitOps   = 0x11, // Peephole Optimization.
      // Debug mode implies that no hardware features or optimizations
      // are performned and that all memory access go through a single
      // uav(Arena on HD5XXX/HD6XXX and Raw on HD4XXX).
      Debug            = 0x12, // Debug mode is enabled.
      CachedMem        = 0x13, // Cached mem is available or not.
      BarrierDetect    = 0x14, // Detect duplicate barriers.
      Semaphore        = 0x15, // Flag to specify that semaphores are supported.
      ByteLDSOps       = 0x16, // Flag to specify if byte LDS ops are available.
      ArenaVectors     = 0x17, // Flag to specify if vector loads from arena work.
      TmrReg           = 0x18, // Flag to specify if Tmr register is supported.
      NoInline         = 0x19, // Flag to specify that no inlining should occur.
      MacroDB          = 0x1A, // Flag to specify that backend handles macrodb.
      HW64BitDivMod    = 0x1B, // Flag for backend to generate 64bit div/mod.
      ArenaUAV         = 0x1C, // Flag to specify that arena uav is supported.
      PrivateUAV       = 0x1D, // Flag to specify that private memory uses uav's.
      StackUAV         = 0x1E, // Flag to specify that stack variables use stack uav.
      ByteGDSOps       = 0x1F, // Flag to specify if byte GDS ops are available.
      FlatMem          = 0x20, // Flag to specify if device supports flat addressing.

      MWGS_3_256_1_1   = 0x21, // Flags to specify maximum work group sizes
      MWGS_3_128_1_1   = 0x22,
      MWGS_3_64_1_1    = 0x23,
      MWGS_3_32_1_1    = 0x24,
      MWGS_3_16_1_1    = 0x25,
      CrossThreadOps   = 0x26,
      UseMacroForCall  = 0x27, // Flag to specify if using outline macro for function call.
      // If more capabilities are required, then
      // this number needs to be increased.
      // All capabilities must come before this
      // number.
      MaxNumberCapabilities = 0x30
    };
  }

    // Enum values for the various memory types.
    enum ResourceIDType {
      RAW_UAV_ID   = 0,
      ARENA_UAV_ID = 1,
      LDS_ID       = 2,
      GDS_ID       = 3,
      SCRATCH_ID   = 4,
      CONSTANT_ID  = 5,
      GLOBAL_ID    = 6,
      PRINTF_ID    = 7,
      STACK_UAV_ID = 8, // Default stack uav id
      MAX_IDS      = 9
    };
  }
} // namespace llvm
#endif // _AMDILDEVICEINFO_H_
