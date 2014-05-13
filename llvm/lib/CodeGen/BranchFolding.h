//===-- BranchFolding.h - Fold machine code branch instructions --*- C++ -*===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_BRANCHFOLDING_HPP
#define LLVM_CODEGEN_BRANCHFOLDING_HPP

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#if defined(AMD_OPENCL) || 1
#include "llvm/CodeGen/MachineLoopInfo.h"
#endif
#include <vector>

namespace llvm {
  class MachineFunction;
  class MachineModuleInfo;
  class RegScavenger;
  class TargetInstrInfo;
  class TargetRegisterInfo;

  class BranchFolder {
  public:
    explicit BranchFolder(bool defaultEnableTailMerge,
                          bool CommonHoist
#if defined(AMD_OPENCL) || 1
                         ,bool defaultEnableRetToEnd,
                          bool defaultDoNotTailMergeAcrossLoop
#endif
                          );

#if defined(AMD_OPENCL) || 1
    bool OptimizeFunction(MachineFunction &MF,
                          const TargetInstrInfo *tii,
                          const TargetRegisterInfo *tri,
                          MachineModuleInfo *mmi,
                          MachineLoopInfo *mli);
#else
    bool OptimizeFunction(MachineFunction &MF,
                          const TargetInstrInfo *tii,
                          const TargetRegisterInfo *tri,
                          MachineModuleInfo *mmi);
#endif // AMD_OPENCL

  private:
    class MergePotentialsElt {
      unsigned Hash;
      MachineBasicBlock *Block;

#if defined(AMD_OPENCL) || 1
      // Need to restore things exactly as they were
      // before transformation in TailMergeBlocks.
      bool IsCondReversed;
#endif

    public:
#if defined(AMD_OPENCL) || 1
      MergePotentialsElt(unsigned h, MachineBasicBlock *b, bool is_cond_rev)
        : Hash(h), Block(b), IsCondReversed(is_cond_rev) {}

      bool getIsCondReversed() const { return IsCondReversed; }
#else
      MergePotentialsElt(unsigned h, MachineBasicBlock *b)
        : Hash(h), Block(b) {}
#endif // AMD_OPENCL

      unsigned getHash() const { return Hash; }
      MachineBasicBlock *getBlock() const { return Block; }

      void setBlock(MachineBasicBlock *MBB) {
        Block = MBB;
      }

      bool operator<(const MergePotentialsElt &) const;
    };
    typedef std::vector<MergePotentialsElt>::iterator MPIterator;
    std::vector<MergePotentialsElt> MergePotentials;
    SmallPtrSet<const MachineBasicBlock*, 2> TriedMerging;

    class SameTailElt {
      MPIterator MPIter;
      MachineBasicBlock::iterator TailStartPos;
    public:
      SameTailElt(MPIterator mp, MachineBasicBlock::iterator tsp)
        : MPIter(mp), TailStartPos(tsp) {}

      MPIterator getMPIter() const {
        return MPIter;
      }
      MergePotentialsElt &getMergePotentialsElt() const {
        return *getMPIter();
      }
      MachineBasicBlock::iterator getTailStartPos() const {
        return TailStartPos;
      }
      unsigned getHash() const {
        return getMergePotentialsElt().getHash();
      }
      MachineBasicBlock *getBlock() const {
        return getMergePotentialsElt().getBlock();
      }
      bool tailIsWholeBlock() const {
        return TailStartPos == getBlock()->begin();
      }

      void setBlock(MachineBasicBlock *MBB) {
        getMergePotentialsElt().setBlock(MBB);
      }
      void setTailStartPos(MachineBasicBlock::iterator Pos) {
        TailStartPos = Pos;
      }
    };
    std::vector<SameTailElt> SameTails;

    bool EnableTailMerge;
    bool EnableHoistCommonCode;
    const TargetInstrInfo *TII;
    const TargetRegisterInfo *TRI;
    MachineModuleInfo *MMI;
#if defined(AMD_OPENCL) || 1
    MachineLoopInfo   *MLI;
#endif
    RegScavenger *RS;

    bool TailMergeBlocks(MachineFunction &MF);
    bool TryTailMergeBlocks(MachineBasicBlock* SuccBB,
                       MachineBasicBlock* PredBB);
    void MaintainLiveIns(MachineBasicBlock *CurMBB,
                         MachineBasicBlock *NewMBB);
    void ReplaceTailWithBranchTo(MachineBasicBlock::iterator OldInst,
                                 MachineBasicBlock *NewDest);
    MachineBasicBlock *SplitMBBAt(MachineBasicBlock &CurMBB,
                                  MachineBasicBlock::iterator BBI1);
    unsigned ComputeSameTails(unsigned CurHash, unsigned minCommonTailLength,
                              MachineBasicBlock *SuccBB,
                              MachineBasicBlock *PredBB);
    void RemoveBlocksWithHash(unsigned CurHash, MachineBasicBlock* SuccBB,
                                                MachineBasicBlock* PredBB);
    bool CreateCommonTailOnlyBlock(MachineBasicBlock *&PredBB,
                                   unsigned maxCommonTailLength,
                                   unsigned &commonTailIndex);

    bool OptimizeBranches(MachineFunction &MF);
    bool OptimizeBlock(MachineBasicBlock *MBB);
    void RemoveDeadBlock(MachineBasicBlock *MBB);
    bool OptimizeImpDefsBlock(MachineBasicBlock *MBB);

    bool HoistCommonCode(MachineFunction &MF);
    bool HoistCommonCodeInSuccs(MachineBasicBlock *MBB);
  };
}

#endif /* LLVM_CODEGEN_BRANCHFOLDING_HPP */
