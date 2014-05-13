//== AMDILMachineFunctionInfo.h - AMD il Machine Function Info -*- C++ -*-===//
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
// This file declares AMDIL-specific per-machine-function information
//
//===----------------------------------------------------------------------===//
#ifndef _AMDILMACHINEFUNCTIONINFO_H_
#define _AMDILMACHINEFUNCTIONINFO_H_
#include "AMDIL.h"
#include "AMDILDeviceInfo.h"
#include "AMDILKernel.h"
#include "AMDILModuleInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include <string>
#include <set>
#include <map>

namespace llvm {
  class AMDILSubtarget;
  class PrintfInfo {
    uint32_t mPrintfID;
    SmallVector<uint32_t, DEFAULT_VEC_SLOTS> mOperands;
    public:
    void addOperand(size_t Idx, uint32_t Size);
    uint32_t getPrintfID();
    void setPrintfID(uint32_t Idx);
    size_t getNumOperands();
    uint32_t getOperandID(uint32_t Idx);
  }; // class PrintfInfo

  class MachineOperand;


  enum NameDecorationStyle {
    None,
    StdCall,
    FastCall
  };
  typedef struct SamplerInfoRec {
    std::string name; // The name of the sampler
    uint32_t val; // The value of the sampler
    uint32_t idx; // The sampler resource id
  } SamplerInfo;
  // Some typedefs that will help with using the various iterators
  // of the machine function info class.
  typedef std::map<std::pair<uint64_t, uint64_t>, uint32_t>::iterator
    lit_iterator;
  typedef StringMap<SamplerInfo>::iterator sampler_iterator;
  typedef DenseSet<uint32_t>::iterator func_iterator;
  typedef DenseSet<uint32_t>::iterator intr_iterator;
  typedef DenseSet<uint32_t>::iterator uav_iterator;
  typedef DenseSet<uint32_t>::iterator sema_iterator;
  typedef DenseSet<uint32_t>::iterator read_image1d_iterator;
  typedef DenseSet<uint32_t>::iterator write_image1d_iterator;
  typedef DenseSet<uint32_t>::iterator read_image1d_array_iterator;
  typedef DenseSet<uint32_t>::iterator write_image1d_array_iterator;
  typedef DenseSet<uint32_t>::iterator read_image1d_buffer_iterator;
  typedef DenseSet<uint32_t>::iterator write_image1d_buffer_iterator;
  typedef DenseSet<uint32_t>::iterator read_image2d_iterator;
  typedef DenseSet<uint32_t>::iterator write_image2d_iterator;
  typedef DenseSet<uint32_t>::iterator read_image2d_array_iterator;
  typedef DenseSet<uint32_t>::iterator write_image2d_array_iterator;
  typedef DenseSet<uint32_t>::iterator read_image3d_iterator;
  typedef DenseSet<uint32_t>::iterator write_image3d_iterator;
  typedef DenseSet<const Value*>::iterator read_ptr_iterator;
  typedef DenseSet<const char*>::iterator error_iterator;
  typedef std::map<std::string, PrintfInfo*>::iterator printf_iterator;
  typedef std::set<std::string>::iterator func_md_iterator;
  typedef std::vector<std::string>::iterator kernel_md_iterator;
  // AMDILMachineFunctionInfo - This class is
  // derived from MachineFunction private
  // amdil target-specific information for each MachineFunction
  class AMDILMachineFunctionInfo : public MachineFunctionInfo {
  public:
    typedef ValueMap<const Value*, uint32_t> ValueIDMapTy;
    typedef ValueIDMapTy::const_iterator value_id_iterator;

  private:
    // CalleeSavedFrameSize - Size of the callee-saved
    // register portion of the
    // stack frame in bytes.
    unsigned CalleeSavedFrameSize;
    // BytesToPopOnReturn - Number of bytes function pops on return.
    // Used on windows platform for stdcall & fastcall name decoration
    unsigned BytesToPopOnReturn;
    // DecorationStyle - If the function requires additional
    // name decoration,
    // DecorationStyle holds the right way to do so.
    NameDecorationStyle DecorationStyle;
    // ReturnAddrIndex - FrameIndex for return slot.
    int ReturnAddrIndex;

    // TailCallReturnAddrDelta - Delta the ReturnAddr stack slot is moved
    // Used for creating an area before the register spill area
    // on the stack
    // the returnaddr can be savely move to this area
    int TailCallReturnAddrDelta;

    // SRetReturnReg - Some subtargets require that sret lowering includes
    // returning the value of the returned struct in a register.
    // This field holds the virtual register into which the sret
    // argument is passed.
    unsigned SRetReturnReg;

    // The size in bytes required to host all of the kernel arguments.
    // -1 means this value has not been determined yet.
    int32_t mArgSize;

    // The size in bytes required to host the stack and the kernel arguments
    // in private memory.
    // -1 means this value has not been determined yet.
    int32_t mScratchSize;

    // The size in bytes required to host the the kernel arguments
    // on the stack.
    // -1 means this value has not been determined yet.
    int32_t mStackSize;

    // Number of vector registers used for the arguments
    int32_t mArgNumVecRegs;

    // Number of vector registers used for the return value
    int32_t mRetNumVecRegs;

    /// A map of constant to literal mapping for all of the 128bit
    /// literals in the current function.
    std::map<std::pair<uint64_t, uint64_t>, uint32_t> mLits;
    uint32_t addLiteral(uint64_t Val_lo, uint64_t Val_hi);

    /// Amount of LDS that should be reserved
    uint32_t mReservedLDS;

    /// A map of name to sampler information that is used to emit
    /// metadata to the IL stream that the runtimes can use for
    /// hardware setup.
    StringMap<SamplerInfo> mSamplerMap;

    /// Array of flags to specify if a specific memory type is used or not.
    bool mUsedMem[AMDIL::MAX_IDS];

    /// Set of all functions that this function calls.
    DenseSet<uint32_t> mFuncs;

    /// Set of all intrinsics that this function calls.
    DenseSet<uint32_t> mIntrs;

    /// Set of all write only 1D images.
    DenseSet<uint32_t> mWO1D;
    /// Set of all read only 1D images.
    DenseSet<uint32_t> mRO1D;
    /// Set of all write only 1D image arrays.
    DenseSet<uint32_t> mWO1DA;
    /// Set of all read only 1D image arrays.
    DenseSet<uint32_t> mRO1DA;
    /// Set of all write only 1D image buffers.
    DenseSet<uint32_t> mWO1DB;
    /// Set of all read only 1D image buffers.
    DenseSet<uint32_t> mRO1DB;
    /// Set of all write only 2D images.
    DenseSet<uint32_t> mWO2D;
    /// Set of all read only 2D images.
    DenseSet<uint32_t> mRO2D;
    /// Set of all write only 2D image arrays.
    DenseSet<uint32_t> mWO2DA;
    /// Set of all read only 2D image arrays.
    DenseSet<uint32_t> mRO2DA;
    /// Set of all read only 3D images.
    DenseSet<uint32_t> mRO3D;
    /// Set of all write only 3D images.
    DenseSet<uint32_t> mWO3D;
    /// Set of all the raw uavs.
    DenseSet<uint32_t> mRawUAV;
    /// Set of all the arena uavs.
    DenseSet<uint32_t> mArenaUAV;

    /// Set of all semaphores
    DenseSet<uint32_t> mSemaphore;

    /// Set of all the read-only pointers
    DenseSet<const Value*> mReadPtr;

    /// A set of all errors that occured in the backend for this function.
    DenseSet<const char *> mErrors;

    /// A mapping of printf data and the printf string
    std::map<std::string, PrintfInfo*> mPrintfMap;

    /// A set of all of the metadata that is used for the current function.
    std::set<std::string> mMetadataFunc;

    /// A set of all of the metadata that is used for the function wrapper.
    std::vector<std::string> mMetadataKernel;

    // registers assigned to formal arguments
    SmallVector<unsigned, 16> mArgRegs;
    // registers assigned to formal return values
    SmallVector<unsigned, 16> mRetRegs;

    /// Map from const Value * to UAV ID.
    ValueIDMapTy mValueIDMap;

    bool mHasOutputInst;

    /// Information about the kernel, NULL if the function is not a kernel.
    AMDILKernel *mKernel;

    /// Pointer to the machine function that this information belongs to.
    MachineFunction *mMF;

    /// Pointer to the AMDIL module info that is for the parent of this function.
    AMDILModuleInfo *mAMI;

    /// Pointer to the subtarget for this function.
    const AMDILSubtarget *mSTM;

    /// Information about Swizzles for each MachineOperand
    std::map< const MachineOperand *, unsigned char > mSwizzleMap;

    public:
    AMDILMachineFunctionInfo();
    AMDILMachineFunctionInfo(MachineFunction &MF);
    virtual ~AMDILMachineFunctionInfo();
    unsigned getCalleeSavedFrameSize() const;
    void setCalleeSavedFrameSize(unsigned Bytes);

    unsigned getBytesToPopOnReturn() const;
    void setBytesToPopOnReturn(unsigned bytes);

    NameDecorationStyle getDecorationStyle() const;
    void setDecorationStyle(NameDecorationStyle style);

    int getRAIndex() const;
    void setRAIndex(int Index);

    int getTCReturnAddrDelta() const;
    void setTCReturnAddrDelta(int Delta);

    unsigned getSRetReturnReg() const;
    void setSRetReturnReg(unsigned Reg);

#define AS_SET_GET(A) \
    private: \
      bool Uses##A;\
      bool A##Arg; \
    public: \
      void setUses##A() { Uses##A = true; }\
      bool uses##A() const { return Uses##A; }\
      void setHas##A##Arg() { A##Arg = true; setUses##A(); }\
      bool has##A##Arg() { return A##Arg; }

    AS_SET_GET(LDS)
    AS_SET_GET(GDS)
    AS_SET_GET(Scratch)
    AS_SET_GET(Constant)

    void setOutputInst() { mHasOutputInst = true; }
    bool getOutputInst() const { return mHasOutputInst; }
    bool usesHWConstant(const std::string& Name) const;
    bool isKernel() const;
    AMDILKernel *getKernel();

    StringRef getName();

    /// Get the size in bytes that are required to host all of
    /// arguments based on the argument alignment rules in the AMDIL
    /// Metadata spec.
    uint32_t getArgSize();

    /// Get the size in bytes that are required to host all of
    /// arguments and stack memory in scratch.
    uint32_t getScratchSize();

    /// Get the size in bytes that is required to host all of
    /// the arguments on the stack.
    uint32_t getStackSize();

    ///
    /// @param val value to add the lookup table
    /// @param Opcode opcode of the literal instruction
    /// @brief adds the specified value of the type represented by the
    /// Opcode
    /// to the literal to integer and integer to literal mappings.
    ///
    /// Add a 32bit integer value to the literal table.
    uint32_t addi32Literal(uint32_t Val, int Opcode = AMDIL::LOADCONSTi32);

    /// Add a 32bit floating point value to the literal table.
    uint32_t addf32Literal(const ConstantFP *CFP);

    /// Add a 32bit floating point value to the literal table.
    uint32_t addf32Literal(uint32_t Val);

    /// Add a 64bit integer value to the literal table.
    uint32_t addi64Literal(uint64_t Val);

    /// Add a 128 bit integer value to the literal table.
    uint32_t addi128Literal(uint64_t Val_lo, uint64_t Val_hi);

    /// Add a 64bit floating point literal as a 64bit integer value.
    uint32_t addf64Literal(const ConstantFP *CFP);

    /// Add a 64bit floating point literal as a 64bit integer value.
    uint32_t addf64Literal(uint64_t Val);

    /// Get the literal ID of an Integer literal of the given offset.
    uint32_t getLitIdx(uint32_t Lit);

    /// Get the literal ID of a Long literal of the given offset.
    uint32_t getLitIdx(uint64_t Lit);

    /// Add some literals to the number of reserved literals.
    void addReservedLiterals(uint32_t);

    void addReservedLDS(uint32_t);

    // Functions that return iterators to the beginning and end
    // of the various literal maps.
    // Functions that return the beginning and end of the literal map
    lit_iterator lit_begin() { return mLits.begin(); }
    lit_iterator lit_end() { return mLits.end(); }

    // Add a sampler to the set of known samplers for the current kernel.
    uint32_t addSampler(std::string name, uint32_t Value);

    // Iterators that point to the beginning and end of the sampler map.
    sampler_iterator sampler_begin() { return mSamplerMap.begin(); }
    sampler_iterator sampler_end() { return mSamplerMap.end(); }


    /// Set the flag for the memory ID to true for the current function.
    void setUsesMem(unsigned);
    /// Retrieve the flag for the memory ID.
    bool usesMem(unsigned);

    /// Add called functions to the set of all functions this function calls.
    void addCalledFunc(uint32_t ID) { mFuncs.insert(ID); }
    void eraseCalledFunc(uint32_t ID) { mFuncs.erase(ID); }
    size_t func_size() { return mFuncs.size(); }
    bool func_empty() { return mFuncs.empty(); }
    func_iterator func_begin() { return mFuncs.begin(); }
    func_iterator func_end() { return mFuncs.end(); }

    /// Add called intrinsics to the set of all intrinscis this function calls.
    void addCalledIntr(uint32_t ID) { mIntrs.insert(ID); }
    size_t intr_size() { return mIntrs.size(); }
    bool intr_empty() { return mIntrs.empty(); }
    intr_iterator intr_begin() { return mIntrs.begin(); }
    intr_iterator intr_end() { return mIntrs.end(); }

    /// Add a 1D read_only image id.
    void addROImage1D(uint32_t ID) { mRO1D.insert(ID); }
    size_t read_image1d_size() { return mRO1D.size(); }
    read_image1d_iterator read_image1d_begin() { return mRO1D.begin(); }
    read_image1d_iterator read_image1d_end() { return mRO1D.end(); }

    /// Add a 1D write_only image id.
    void addWOImage1D(uint32_t ID) { mWO1D.insert(ID); }
    size_t write_image1d_size() { return mWO1D.size(); }
    write_image1d_iterator write_image1d_begin() { return mWO1D.begin(); }
    write_image1d_iterator write_image1d_end() { return mWO1D.end(); }

    /// Add a 1D read_only image id.
    void addROImage1DArray(uint32_t ID) { mRO1DA.insert(ID); }
    size_t read_image1d_array_size() { return mRO1DA.size(); }
    read_image1d_array_iterator read_image1d_array_begin() { return mRO1DA.begin(); }
    read_image1d_array_iterator read_image1d_array_end() { return mRO1DA.end(); }

    /// Add a 1D write_only image id.
    void addWOImage1DArray(uint32_t ID) { mWO1DA.insert(ID); }
    size_t write_image1d_array_size() { return mWO1DA.size(); }
    write_image1d_array_iterator write_image1d_array_begin() { return mWO1DA.begin(); }
    write_image1d_array_iterator write_image1d_array_end() { return mWO1DA.end(); }

    /// Add a 1D read_only image id.
    void addROImage1DBuffer(uint32_t ID) { mRO1DB.insert(ID); }
    size_t read_image1d_buffer_size() { return mRO1DB.size(); }
    read_image1d_buffer_iterator read_image1d_buffer_begin() { return mRO1DB.begin(); }
    read_image1d_buffer_iterator read_image1d_buffer_end() { return mRO1DB.end(); }

    /// Add a 1D write_only image id.
    void addWOImage1DBuffer(uint32_t ID) { mWO1DB.insert(ID); }
    size_t write_image1d_buffer_size() { return mWO1DB.size(); }
    write_image1d_buffer_iterator write_image1d_buffer_begin() { return mWO1DB.begin(); }
    write_image1d_buffer_iterator write_image1d_buffer_end() { return mWO1DB.end(); }

    /// Add a 2D read_only image id.
    void addROImage2D(uint32_t ID) { mRO2D.insert(ID); }
    size_t read_image2d_size() { return mRO2D.size(); }
    read_image2d_iterator read_image2d_begin() { return mRO2D.begin(); }
    read_image2d_iterator read_image2d_end() { return mRO2D.end(); }

    /// Add a 2D write_only image id.
    void addWOImage2D(uint32_t ID) { mWO2D.insert(ID); }
    size_t write_image2d_size() { return mWO2D.size(); }
    write_image2d_iterator write_image2d_begin() { return mWO2D.begin(); }
    write_image2d_iterator write_image2d_end() { return mWO2D.end(); }

    /// Add a 2D read_only image array id.
    void addROImage2DArray(uint32_t ID) { mRO2DA.insert(ID); }
    size_t read_image2d_array_size() { return mRO2DA.size(); }
    read_image2d_array_iterator read_image2d_array_begin() { return mRO2DA.begin(); }
    read_image2d_array_iterator read_image2d_array_end() { return mRO2DA.end(); }

    /// Add a 2D write_only image id.
    void addWOImage2DArray(uint32_t ID) { mWO2DA.insert(ID); }
    size_t write_image2d_array_size() { return mWO2DA.size(); }
    write_image2d_array_iterator write_image2d_array_begin() { return mWO2DA.begin(); }
    write_image2d_array_iterator write_image2d_array_end() { return mWO2D.end(); }

    /// Add a 3D read_only image id.
    void addROImage3D(uint32_t ID) { mRO3D.insert(ID); }
    size_t read_image3d_size() { return mRO3D.size(); }
    read_image3d_iterator read_image3d_begin() { return mRO3D.begin(); }
    read_image3d_iterator read_image3d_end() { return mRO3D.end(); }

       /// Add a 3D write_only image id.
    void addWOImage3D(uint32_t ID) { mWO3D.insert(ID); }
    size_t write_image3d_size() { return mWO3D.size(); }
    write_image3d_iterator write_image3d_begin() { return mWO3D.begin(); }
    write_image3d_iterator write_image3d_end() { return mWO3D.end(); }

    size_t get_num_write_images();

    /// Add a semaphore
    void sema_insert(uint32_t ID) { mSemaphore.insert(ID); }
    bool sema_count(uint32_t ID) { return mSemaphore.count(ID); }
    size_t sema_size() { return mSemaphore.size(); }
    sema_iterator sema_begin() { return mSemaphore.begin(); }
    sema_iterator sema_end() { return mSemaphore.end(); }

    /// Add a raw uav id.
    void uav_insert(uint32_t ID) { mRawUAV.insert(ID); }
    bool uav_count(uint32_t ID) { return mRawUAV.count(ID); }
    size_t uav_size() { return mRawUAV.size(); }
    uav_iterator uav_begin() { return mRawUAV.begin(); }
    uav_iterator uav_end() { return mRawUAV.end(); }

    /// Add an arena uav id.
    void arena_insert(uint32_t ID) { mArenaUAV.insert(ID); }
    bool arena_count(uint32_t ID) { return mArenaUAV.count(ID); }
    size_t arena_size() { return mArenaUAV.size(); }
    uav_iterator arena_begin() { return mArenaUAV.begin(); }
    uav_iterator arena_end() { return mArenaUAV.end(); }

    /// Add a pointer to the known set of read-only pointers
    void add_read_ptr(const Value *Ptr) { mReadPtr.insert(Ptr); }
    bool read_ptr_count(const Value *Ptr) { return mReadPtr.count(Ptr); }
    bool read_size() { return mReadPtr.size(); }
    read_ptr_iterator read_ptr_begin() { return mReadPtr.begin(); }
    read_ptr_iterator read_ptr_end() { return mReadPtr.end(); }

    // Add an error to the output for the current function.
    typedef enum {
      RELEASE_ONLY, /// Only emit error message in release mode.
      DEBUG_ONLY, /// Only emit error message in debug mode.
      ALWAYS /// Always emit the error message.
    } ErrorMsgEnum;
    /// Add an error message to the set of all error messages.
    void addErrorMsg(const char* msg, ErrorMsgEnum val = ALWAYS);
    bool errors_empty() { return mErrors.empty(); }
    error_iterator errors_begin() { return mErrors.begin(); }
    error_iterator errors_end() { return mErrors.end(); }

    /// Add a string to the printf map
    uint32_t addPrintfString(StringRef name, unsigned offset);
    /// Add a operand to the printf string
    void addPrintfOperand(StringRef name, size_t idx, uint32_t size);
    bool printf_empty() { return mPrintfMap.empty(); }
    size_t printf_size() { return mPrintfMap.size(); }
    printf_iterator printf_begin() { return mPrintfMap.begin(); }
    printf_iterator printf_end() { return mPrintfMap.end(); }

    /// Add a string to the metadata set for a function/kernel wrapper
    void addMetadata(StringRef MD, bool KernelOnly = false);
    func_md_iterator func_md_begin() { return mMetadataFunc.begin(); }
    func_md_iterator func_md_end() { return mMetadataFunc.end(); }
    kernel_md_iterator kernel_md_begin() { return mMetadataKernel.begin(); }
    kernel_md_iterator kernel_md_end() { return mMetadataKernel.end(); }

    value_id_iterator value_id_map_begin() const { return mValueIDMap.begin(); }
    value_id_iterator value_id_map_end() const { return mValueIDMap.end(); }


    // Set the uav id for the specific pointer value.  If value is NULL
    // then the ID sets the default ID.
    void setUAVID(const Value *Value, uint32_t ID);

    // Get the UAV id for the specific pointer value.
    uint32_t getUAVID(const Value *Value) const;

    /// Query to find out if we are a signed or unsigned integer type.
    bool isSignedIntType(const Value *Ptr);

    /// Query to find out if we are a volatile pointer.
    bool isVolatilePointer(const Value *Ptr);

    /// Query to find out if we are a restrict pointer.
    bool isRestrictPointer(const Value *Ptr);

    /// Query to find out if we are a constant argument.
    bool isConstantArgument(const Value *Ptr);

    void setArgNumVecRegs(unsigned n) {
      mArgNumVecRegs = (int32_t)n;
    }

    unsigned getArgNumVecRegs() const {
      assert(mArgNumVecRegs >= 0 && "value not set yet");
      return mArgNumVecRegs;
    }

    void setRetNumVecRegs(unsigned n) {
      mRetNumVecRegs = (int32_t)n;
    }

    unsigned getRetNumVecRegs() const {
      assert(mRetNumVecRegs >= 0 && "value not set yet");
      return mRetNumVecRegs;
    }

    /// add the registers numbers for the formal arguments
    void addArgReg(unsigned Reg) {
      mArgRegs.push_back(Reg);
    }

    /// retrieve the registers numbers for the formal arguments
    unsigned getArgReg(unsigned Arg) const {
      //return Arg < mArgRegs.size() ? mArgRegs[Arg] : Arg;
      assert(Arg < getNumArgRegs() && "Argument not in register");
      return mArgRegs[Arg];
    }

    /// add the registers numbers for the formal arguments
    void addRetReg(unsigned Reg) {
      mRetRegs.push_back(Reg);
    }

    /// retrieve the registers numbers for the formal arguments
    unsigned getRetReg(unsigned Arg) const {
      //return Arg < mRetRegs.size() ? mRetRegs[Arg] : Arg;
      assert(Arg < getNumRetRegs() && "Argument not in register");
      return mRetRegs[Arg];
    }

    // retrieve number of registers used for arguments
    unsigned getNumArgRegs() const { return mArgRegs.size(); }
    // retrieve number of registers used for return values
    unsigned getNumRetRegs() const { return mRetRegs.size(); }

    /// Add/retrieve SwizzleMap entries
    void setMOSwizzle( const MachineOperand &MO, unsigned char swizzle ) {
      mSwizzleMap.insert( std::pair<const MachineOperand *, unsigned char>( &MO, swizzle ) );
    }

    unsigned char getMOSwizzle( const MachineOperand &MO ) const {
      const std::map< const MachineOperand *, unsigned char>::const_iterator &it = mSwizzleMap.find( &MO );
      if (it != mSwizzleMap.end())
        return it->second;
      return 0; // default swizzle value
    }
  };
} // llvm namespace
#endif // _AMDILMACHINEFUNCTIONINFO_H_
