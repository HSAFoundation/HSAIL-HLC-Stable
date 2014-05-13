//===-- AMDILCFGStructurizer.cpp - CFG Structurizer -----------------------===//
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

#define DEBUG_TYPE "structcfg"

#include "llvm/Support/Debug.h"
#include "AMDILCompilerErrors.h"
#include "AMDILMachineFunctionInfo.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "llvm/InitializePasses.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachinePostDominators.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"

#include <algorithm>

#define FirstNonDebugInstr(A) A->begin()
using namespace llvm;

// TODO: move-begin.

//===----------------------------------------------------------------------===//
//
// Statistics for CFGStructurizer.
//
//===----------------------------------------------------------------------===//

STATISTIC(numSerialPatternMatch,    "CFGStructurizer number of serial pattern "
    "matched");
STATISTIC(numIfPatternMatch,        "CFGStructurizer number of if pattern "
    "matched");
STATISTIC(numLoopBreakPatternMatch, "CFGStructurizer number of loop-break "
    "pattern matched");
STATISTIC(numLoopContPatternMatch,  "CFGStructurizer number of loop-continue "
    "pattern matched");
STATISTIC(numLoopPatternMatch,      "CFGStructurizer number of loop pattern "
    "matched");
STATISTIC(numClonedBlock,           "CFGStructurizer cloned blocks");
STATISTIC(numClonedInstr,           "CFGStructurizer cloned instructions");

//===----------------------------------------------------------------------===//
//
// Miscellaneous utility for CFGStructurizer.
//
//===----------------------------------------------------------------------===//
namespace llvmCFGStruct
{

static void showNewInstr(const MachineInstr *instr) {
  dbgs() << "New instr: " << *instr << '\n';
}

static void showNewBlock(const MachineBasicBlock *BB, const char *Message) {
  dbgs() << "New block (" << Message << ") BB" << BB->getNumber()
         << " size " << BB->size() << '\n';
}

#define INVALIDREGNUM 0

template<class LoopInfoT>
void printLoopInfo(const LoopInfoT &LoopInfo, llvm::raw_ostream &OS) {
  for (typename LoopInfoT::iterator I = LoopInfo.begin(), E = LoopInfo.end();
       I != E; ++I) {
    MachineLoop* Loop = *I;
    Loop->print(OS);
  }
}

} //end namespace llvmCFGStruct

static MachineInstr *getLastBreakInstr(MachineBasicBlock *Block) {
  for (MachineBasicBlock::reverse_iterator I = Block->rbegin(),
         E = Block->rend(); I != E; ++I) {
    MachineInstr *Instr = &*I;
    if ((Instr->getOpcode() == AMDIL::BREAK_LOGICALNZi32r)
        || (Instr->getOpcode() == AMDIL::BREAK_LOGICALZi32r)) {
      return Instr;
    }
  }

  return NULL;
}

//===----------------------------------------------------------------------===//
//
// Supporting data structure for CFGStructurizer
//
//===----------------------------------------------------------------------===//

namespace llvmCFGStruct
{
template<class PassT>
struct CFGStructTraits {
};

template <class BlockT, class InstrT, class RegiT>
class LandInformation {
public:
  BlockT *landBlk;
  std::set<RegiT> breakInitRegs;  // Registers that need to "reg = 0", before
                                  // WHILELOOP(thisloop) init before entering
                                  // thisloop.
  std::set<RegiT> contInitRegs;   // Registers that need to "reg = 0", after
                                  // WHILELOOP(thisloop) init after entering
                                  // this loop.
  std::set<RegiT> endBranchInitRegs; // Init after entering this loop, at loop
                                     // land block, branch cond on this reg.
  std::set<RegiT> breakOnRegs;       // registers that need to "if (reg) break
                                     // endif" after ENDLOOP(thisloop) break
                                     // outerLoopOf(thisLoop).
  std::set<RegiT> contOnRegs;       // registers that need to "if (reg) continue
                                    // endif" after ENDLOOP(thisloop) continue on
                                    // outerLoopOf(thisLoop).
  LandInformation() : landBlk(NULL) {}
};

} //end of namespace llvmCFGStruct

//===----------------------------------------------------------------------===//
//
// TrivialRegAlloc
//
//===----------------------------------------------------------------------===//

namespace llvmCFGStruct
{
// Stores the list of defs and uses of a virtual register
class DefUseList {
  enum {
    FLAG_DEF = 0,
    FLAG_USE = 1
  };

public:
  // Struct that represents a single def or use
  struct DefOrUseT {
    unsigned _slotIndex;
    unsigned _flag; // Flag whether this is a def or use
    bool isDef() const { return _flag == FLAG_DEF; }
    bool isUse() const { return _flag == FLAG_USE; }
    DefOrUseT(unsigned slotIndex, unsigned flag)
      : _slotIndex(slotIndex), _flag(flag) {}
  };

private:
  typedef SmallVector<DefOrUseT, 2> DefUseVecT;

public:
  typedef DefUseVecT::iterator iterator;
  typedef DefUseVecT::const_iterator const_iterator;

  DefUseVecT _defUses;

  DefUseList() : _defUses() {}
  void addDef(unsigned slotIndex) {
    _defUses.push_back(DefOrUseT(slotIndex, FLAG_DEF));
  }
  void addUse(unsigned slotIndex) {
    _defUses.push_back(DefOrUseT(slotIndex, FLAG_USE));
  }
  void clear() { _defUses.clear(); }
  iterator begin() { return _defUses.begin(); }
  const_iterator begin() const { return _defUses.begin(); }
  iterator end() { return _defUses.end(); }
  const_iterator end() const { return _defUses.end(); }
  bool isSorted() const;
  void dump() const;
};

bool DefUseList::isSorted() const
{
  const_iterator I = begin();
  const_iterator E = end();
  assert(I != E && "No def/use");
  const_iterator Pre = I;
  for (++I; I != E; ++I) {
    if ((*Pre)._slotIndex > (*I)._slotIndex) {
      return false;
    }
    Pre = I;
  }
  return true;
}

void DefUseList::dump() const
{
  for (const_iterator I = begin(), E = end(); I != E; ++I) {
    const DefOrUseT& DefOrUse = *I;
    const char* Str = DefOrUse.isDef() ? "def" : "use";
    dbgs() << "    " << DefOrUse._slotIndex << " " << Str << '\n';
  }
}

// A live interval
class LiveInterval {
  enum {
    UndefinedSlotIndex = -1
  };
  unsigned _vreg;
  int _startSlotIndex;
  int _endSlotIndex;

public:
  LiveInterval(unsigned vreg)
    : _vreg(vreg),
      _startSlotIndex(UndefinedSlotIndex),
      _endSlotIndex(UndefinedSlotIndex) {}
  bool hasStart() const {
    return _startSlotIndex != UndefinedSlotIndex;
  }
  bool hasEnd() const {
    return _endSlotIndex != UndefinedSlotIndex;
  }
  void setStart(int SlotIndex) {
    _startSlotIndex = SlotIndex;
  }
  void setEnd(int SlotIndex) {
    _endSlotIndex = SlotIndex;
  }
  unsigned vreg() const { return _vreg; }
  unsigned start() const { return _startSlotIndex; }
  unsigned end() const { return _endSlotIndex; }
};

// A list of live intervals
class LiveIntervals {
  typedef SmallVector<LiveInterval, 16> IntervalVecType;

public:
  typedef IntervalVecType::iterator iterator;
  typedef IntervalVecType::const_iterator const_iterator;

private:
  IntervalVecType _intervals;
  bool _sorted; // Whether the intervals are sorted by start position

private:
  iterator findIntervalImpl(unsigned VReg);

public:
  LiveIntervals(bool sorted) : _intervals(), _sorted(sorted) {}
  LiveInterval *findInterval(unsigned VReg) {
    iterator I = findIntervalImpl(VReg);
    if (I == _intervals.end()) {
      return NULL;
    }
    return &*I;
  }
  LiveInterval& createInterval(unsigned vreg) {
    _intervals.push_back(LiveInterval(vreg));
    return _intervals.back();
  }
  void appendInterval(LiveInterval& interval) {
    if (_sorted) {
      assert((_intervals.size() == 0
              || interval.start() >= _intervals.back().start())
             && "unsorted append into sorted LiveIntervals");
    }
    _intervals.push_back(interval);
  }
  void insertInterval(LiveInterval& interval) {
    if (!_sorted) {
      _intervals.push_back(interval);
      return;
    }
    insertIntervalSorted(interval);
  }

  void removeInterval(unsigned VReg);
  iterator removeInterval(iterator It) {
    return _intervals.erase(It);
  }
  void clear() { _intervals.clear(); }
  iterator begin() { return _intervals.begin(); }
  iterator end() { return _intervals.end(); }
  bool isSortedByStart() const;
  void dump() const;

private:
  void insertIntervalSorted(LiveInterval &Interval);
};

LiveIntervals::iterator LiveIntervals::findIntervalImpl(unsigned VReg) {
  iterator I = _intervals.begin();
  iterator E = _intervals.end();
  for (; I != E; ++I) {
    if ((*I).vreg() == VReg) {
      break;
    }
  }
  return I;
}

void LiveIntervals::insertIntervalSorted(LiveInterval& Interval) {
  iterator I = _intervals.begin();
  iterator E = _intervals.end();
  for (; I != E; ++I) {
    if (Interval.start() >= (*I).start()) {
      break;
    }
  }
  _intervals.insert(I, Interval);
}

void LiveIntervals::removeInterval(unsigned VReg) {
  iterator I = findIntervalImpl(VReg);
  assert(I != _intervals.end() && "interval not found");
  _intervals.erase(I);
}

bool LiveIntervals::isSortedByStart() const {
  const_iterator I = _intervals.begin();
  const_iterator E = _intervals.end();
  if (I == E) {
    return true;
  }

  const_iterator Pre = I;
  ++I;
  for (; I != E; ++I) {
    if ((*Pre).start() > (*I).start()) {
      return false;
    }
    Pre = I;
  }
  return true;
}

void LiveIntervals::dump() const {
  dbgs() << "Intervals:\n";
  const_iterator I = _intervals.begin();
  const_iterator E = _intervals.end();
  for (; I != E; ++I) {
    const LiveInterval& Interval = *I;
    dbgs() << "  vreg " << TargetRegisterInfo::virtReg2Index(Interval.vreg())
           << " start " << Interval.start()
           << " end " << Interval.end() << '\n';
  }
}

// Trivial linear scan register allocator to allocate physical
// registers for registers requested during CFGStructurizer pass.
// Since register allocator has already been run before this pass, we
// have to define our own register allocator to do very simple
// register allocation for registers requested during this pass.
class TrivialRegAlloc {
  typedef SmallVector<LiveIntervals, 2>   IntervalsVecT;
  typedef std::map<unsigned, unsigned>    RegMapT;
  typedef std::set<unsigned>              RegSetT;
  typedef std::map<unsigned, DefUseList*> VRegDefUseMapT;

private:
  // Data structures passed in to this class

  MachineFunction& _func;
  const TargetRegisterClass& _regClass;
  // Virtual registers that need physical registers to be allocated
  RegSetT& _vregs;

  // Data structures created within this class

  VRegDefUseMapT _vregDefUseMap; // map vreg -> its def/use list
  BitVector _regInUse; // Flags which registers are currently in use
  // Set of physical registers that can be alloc'ed
  std::vector<unsigned> _regSet;
  RegMapT _regMap; // Virtual to physical register map
  LiveIntervals _intervals; // List of all live intervals
  // Transient list of currently active live intervals
  LiveIntervals _activeIntervals;
  // Transient current interval for which we are trying to allocate a register
  LiveInterval* _currInterval;

private:
  void initRegSet();
  void computeIntervals();
  unsigned getPhysicalRegister();
  void allocateRegisterFor(LiveInterval& interval);
  void releaseRegisterFor(const LiveInterval& interval);
  void handleActiveIntervals(unsigned pos);
  void allocateRegisters();
  void rewrite();

public:
  TrivialRegAlloc(MachineFunction& func,
                  const TargetRegisterClass& regClass,
                  RegSetT& vregs);
  ~TrivialRegAlloc();
  void run(); // Main driver of the algorithm
};

TrivialRegAlloc::TrivialRegAlloc(MachineFunction& func,
                                 const TargetRegisterClass& regClass,
                                 RegSetT& vregs)
  : _func(func), _regClass(regClass), _vregs(vregs),
    _vregDefUseMap(), _regInUse(), _regSet(), _regMap(),
    _intervals(true), _activeIntervals(false), _currInterval(NULL) {
  assert(_regClass.getID() == AMDIL::GPR_32RegClassID && "unimplemented");
}

TrivialRegAlloc::~TrivialRegAlloc() {
  for (VRegDefUseMapT::iterator I = _vregDefUseMap.begin(),
         E = _vregDefUseMap.end();
       I != E; ++I) {
    delete I->second;
  }
}

// Find all physical registers that are still available after the
// global register allocator
static void findAvailPhysRegs(MachineFunction& Func,
                              const TargetRegisterClass& RegClass,
                              std::vector<unsigned>& RegSet) {
  ArrayRef<uint16_t> AllocOrder = RegClass.getRawAllocationOrder(Func);
  for (const uint16_t *I = AllocOrder.begin(), *E = AllocOrder.end();
       I != E; ++I) {
    uint16_t TempReg = *I;
    if (Func.getRegInfo().isPhysRegUsed(TempReg)) {
      continue;
    }

    if (TempReg) {
      RegSet.push_back(TempReg);
    }
  }
}

// Initialize the register set with remaining physical registers that
// are still available and the set of physical registers reserved for
// CFGStructurizer
void TrivialRegAlloc::initRegSet() {
  findAvailPhysRegs(_func, _regClass, _regSet);
  for (unsigned I = AMDIL::CFG1; I <= AMDIL::CFG10; ++I) {
    _regSet.push_back(I);
  }
  _regInUse.resize(_regSet.size(), 0);
  DEBUG(
    dbgs() << "Available physical registers:\n   ";
    for (std::vector<unsigned>::iterator I = _regSet.begin(),
           E = _regSet.end(); I != E; ++I) {
      dbgs() << " " << *I;
    }
    dbgs() << '\n';
 );
}

// Compute live intervals for the virtual registers created during
// CFGStructurizer pass
void TrivialRegAlloc::computeIntervals() {
  MachineBasicBlock *EntryBlock
    = GraphTraits<MachineFunction*>::nodes_begin(&_func);
  unsigned SlotIndex = 0;
  // There is only one block now in the function
  for (MachineBasicBlock::iterator I = EntryBlock->begin(),
         E = EntryBlock->end(); I != E; ++I) {
    MachineInstr *Inst = I;
    for (unsigned J = 0, NumOperands = Inst->getNumOperands();
         J < NumOperands; ++J) {
      MachineOperand& Oper = Inst->getOperand(J);
      if (!Oper.isReg() || !Oper.getReg()) {
        continue;
      }
      unsigned VReg = Oper.getReg();
      // If not a virtual register that needs reg alloc, skip
      if (!_vregs.count(VReg)) {
        continue;
      }
      // Add to vreg's def/use list
      DefUseList *&DefUses = _vregDefUseMap[VReg];
      LiveInterval *Interval = _intervals.findInterval(VReg);
      if (Oper.isDef()) {
        if (!DefUses) {
          DefUses = new DefUseList();
        }
        DefUses->addDef(SlotIndex);
        if (!Interval) {
          Interval = &_intervals.createInterval(VReg);
        }
        if (!Interval->hasStart()) {
          Interval->setStart(SlotIndex);
          DEBUG(dbgs() << "interval for vreg "
                << TargetRegisterInfo::virtReg2Index(VReg)
                << " start at " << SlotIndex << '\n');
        }
        else {
          assert(SlotIndex > Interval->start() && "sanity");
        }
      } else {
        assert(DefUses && "Use before def");
        DefUses->addUse(SlotIndex);
        assert(Interval && "Use before def");
        assert((!Interval->hasEnd() || SlotIndex > Interval->end())
               && "sanity");
        Interval->setEnd(SlotIndex);

        DEBUG(dbgs() << "Interval for vreg "
              << TargetRegisterInfo::virtReg2Index(VReg)
              << " end at " << SlotIndex << '\n');
      }
    }
    ++SlotIndex;
  }

  DEBUG(
    _intervals.dump();
    dbgs() << "def/use map: \n";
    for (VRegDefUseMapT::const_iterator I = _vregDefUseMap.begin(),
           E = _vregDefUseMap.end(); I != E; ++I) {
      DefUseList *DefUses = I->second;
      dbgs() << "  vreg "
             << TargetRegisterInfo::virtReg2Index(I->first) << '\n';
      DefUses->dump();
    }
 );
  assert(_intervals.isSortedByStart() && "_intervals not sorted");
#ifndef NDEBUG
  for (VRegDefUseMapT::iterator I = _vregDefUseMap.begin(),
         E = _vregDefUseMap.end(); I != E; ++I) {
    assert(I->second->isSorted() && "def/uses not sorted");
  }
#endif
}

// Pick a physical register that is not in use
unsigned TrivialRegAlloc::getPhysicalRegister() {
  for (unsigned I = 0, Size = _regInUse.size(); I < Size; ++I) {
    if (!_regInUse[I]) {
      _regInUse[I] = 1;
      return _regSet[I];
    }
  }
  // No physical register available. Has to spill.
  // TODO: add spiller
  abort();
  return 0;
}

// Allocate a physical register for the live interval
void TrivialRegAlloc::allocateRegisterFor(LiveInterval& Interval) {
  _currInterval = &Interval;
  unsigned VReg = Interval.vreg();
  unsigned PhysicalReg = getPhysicalRegister();
  _regMap[VReg] = PhysicalReg;

  DEBUG(dbgs() << "Allocated reg " << PhysicalReg << " to vreg "
        << TargetRegisterInfo::virtReg2Index(VReg) << '\n');

// _func->getRegInfo().setPhysRegUsed(TempReg);
}

// Release physical register allocated for the interval
void TrivialRegAlloc::releaseRegisterFor(const LiveInterval& Interval) {
  unsigned PhysicalReg = _regMap[Interval.vreg()];
  unsigned I = 0;
  for (unsigned RegSetSize = _regSet.size(); I < RegSetSize; ++I) {
    if (_regSet[I] == PhysicalReg) {
      break;
    }
  }
  assert(I < _regSet.size() && "invalid physical regsiter");
  _regInUse[I] = 0;
}

// Remove out of active intervals list if an interval becomes inactive
void TrivialRegAlloc::handleActiveIntervals(unsigned Pos) {
  for (LiveIntervals::iterator I = _activeIntervals.begin();
       I != _activeIntervals.end();) {
    LiveInterval& Interval = *I;
    if (Pos > Interval.end()) {
      releaseRegisterFor(Interval);
      I = _activeIntervals.removeInterval(I);
    } else {
      ++I;
    }
  }
}

// Allocate physical registers for each live interval in the interval
// list
void TrivialRegAlloc::allocateRegisters() {
  // Intervals that just become active
  for (LiveIntervals::iterator I = _intervals.begin(), E = _intervals.end();
       I != E; ++I) {
    LiveInterval& interval = *I;

    // Remove intervals that become inactive out of active list
    handleActiveIntervals(interval.start());
    // Interval becomes active
    _activeIntervals.appendInterval(interval);
    // Allocate registers for interval that just becomes active
    allocateRegisterFor(interval);
  }
}

// Rewrite the machine instructions to use the physical registers
// allocated
void TrivialRegAlloc::rewrite() {
  MachineBasicBlock* EntryBlock
    = GraphTraits<MachineFunction*>::nodes_begin(&_func);
  // There is only one block now in the function
  for (MachineBasicBlock::iterator It = EntryBlock->begin(),
         End = EntryBlock->end();
       It != End; ++It) {
    MachineInstr *Inst = It;
    for (unsigned I = 0, NumOperands = Inst->getNumOperands();
         I < NumOperands; ++I) {
      MachineOperand& Oper = Inst->getOperand(I);
      if (!Oper.isReg() || !Oper.getReg()) {
        continue;
      }
      unsigned VReg = Oper.getReg();
      // If not a virtual register that needs reg alloc, skip
      if (!_vregs.count(VReg)) {
        continue;
      }
      assert(_regMap.find(VReg) != _regMap.end()
             && "Register not allocated");
      unsigned PhysicalReg = _regMap[VReg];
      Oper.setReg(PhysicalReg);
    }
  }
}

// The main driver of this register allocator
void TrivialRegAlloc::run() {
  initRegSet();
  computeIntervals();
  allocateRegisters();
  rewrite();
}

//===----------------------------------------------------------------------===//
//
// CFGStructurizer
//
//===----------------------------------------------------------------------===//

// TODO: Port it to BasicBlock, not just MachineBasicBlock.
template<class PassT>
class CFGStructurizer {
public:
  typedef enum {
    Not_SinglePath = 0,
    SinglePath_InPath = 1,
    SinglePath_NotInPath = 2
  } PathToKind;

public:
  typedef typename PassT::InstructionType         InstrT;
  typedef typename PassT::FunctionType            FuncT;
  typedef typename PassT::PostDominatorTreeType   PostDomTreeT;
  typedef typename PassT::DomTreeNodeType         DomTreeNodeT;
  typedef typename PassT::LoopInfoType            LoopInfoT;

  typedef GraphTraits<FuncT *>                    FuncGTraits;
  typedef typename FuncT::iterator                BlockIterator;

  typedef typename FuncGTraits::NodeType          BlockT;
  typedef GraphTraits<BlockT *>                   BlockGTraits;
  typedef GraphTraits<Inverse<BlockT *> >         InvBlockGTraits;
  typedef typename BlockT::iterator               InstrIterator;

  typedef CFGStructTraits<PassT>                  CFGTraits;

  typedef int                                     RegiT;
  typedef typename PassT::LoopType                LoopT;
  typedef LandInformation<BlockT, InstrT, RegiT>  LoopLandInfo;
  typedef std::map<const LoopT *, LoopLandInfo *> LoopLandInfoMap;
        // Landing info for loop break
  typedef SmallVector<BlockT *, 32>               BlockTSmallerVector;
  typedef typename LoopT::Edge                    LoopEdgeT;

public:
  CFGStructurizer();
  ~CFGStructurizer();

  /// Perform the CFG structurization
  bool run(FuncT &Func, PassT &Pass);

  /// Perform the CFG preparation
  bool prepare(FuncT &Func, PassT &Pass);

private:
  void irreducibleControlFlowError();
  void updateData();
  bool processFunction();
  void clearRetiredBlockSet();
  void clearLoopLandInfoMap();
  void printOrderedBlocks(llvm::raw_ostream &OS);

  int patternMatch(BlockT *CurBlock);
  int patternMatchGroup(BlockT *CurBlock);

  int serialPatternMatch(BlockT *CurBlock);
  int ifPatternMatch(BlockT *CurBlock);
  int switchPatternMatch(BlockT *CurBlock);
  int loopEndPatternMatch(BlockT *CurBlock);
  int loopPatternMatch(BlockT *CurBlock);

  void foldBreakingBlock(LoopT *LoopRep, BlockT *ExitLandBlock);
  BlockT *findLoopBreakLandingBlock(
    LoopT *LoopRep,
    const SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> &ExitBlockSet,
    ArrayRef<BlockT *> ExitBlocks,
    ArrayRef<BlockT *> ExitingBlocks);

  int loopBreakPatternMatch(LoopT *LoopRep);
  int loopContPatternMatch(LoopT *LoopRep);

  void findNestedLoops(SmallVector<LoopT *, DEFAULT_VEC_SLOTS>& nestedLoops,
                       const BlockT *Block);

  inline int getRegister(const TargetRegisterClass *RegClass);
  void handleLoopBreak(BlockT *ExitingBlock, LoopT *ExitingLoop,
                       BlockT *ExitBlock, LoopT *exitLoop,
                       BlockT *landBlock);
  void handleLoopContBlock(BlockT *ContingBlock, LoopT *ContingLoop,
                           BlockT *ContBlock, LoopT *ContLoop);
  int handleJumpIntoIf(BlockT *HeadBlock, BlockT *TrueBlock,
                       BlockT *FalseBlock);
  int handleJumpIntoIfImp(BlockT *HeadBlock, BlockT *TrueBlock,
                          BlockT *FalseBlock);
  int improveSimpleJumpIntoIf(BlockT *HeadBlock, BlockT *TrueBlock,
                              BlockT *FalseBlock, BlockT **LandBlockPtr);
  void showImproveSimpleJumpIntoIf(BlockT *HeadBlock, BlockT *TrueBlock,
                                   BlockT *FalseBlock, BlockT *LandBlock,
                                   bool Detail = false);
  PathToKind singlePathTo(BlockT *SrcBlock, BlockT *DstBlock,
                          bool AllowSideEntry = true);
  BlockT *singlePathEnd(BlockT *srcBlock, BlockT *DstBlock,
                        bool AllowSideEntry = true);
  int cloneOnSideEntryTo(BlockT *PreBlock, BlockT *SrcBlock, BlockT *DstBlock);
  void mergeSerialBlock(BlockT *DstBlock, BlockT *srcBlock);

  void mergeIfThenElseBlock(InstrT *BranchInstr, BlockT *CurBlock,
                            BlockT *TrueBlock, BlockT *FalseBlock,
                            BlockT *LandBlock);
  void settleLoop(BlockT *DstBlock, LoopLandInfo *LoopLand);
  void mergeLoopBreakBlock(BlockT *ExitingBlock, BlockT *ExitBlock,
                           BlockT *ExitLandBlock, RegiT SetReg);
  void settleLoopContBlock(BlockT *ContingBlock, BlockT *ContBlock,
                           RegiT SetReg);
  BlockT *relocateLoopContBlock(LoopT *ParentLoopRep, LoopT *LoopRep,
                                const SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> &ExitBlockSet,
                                BlockT *ExitLandBlk);
  BlockT *addLoopEndBranchBlock(LoopT *LoopRep,
                                ArrayRef<BlockT *> ExitingBlocks,
                                ArrayRef<BlockT *> ExitBlocks);
  BlockT *normalizeInfiniteLoopExit(LoopT *LoopRep);
  bool removeUnconditionalBranch(BlockT *SrcBlock);
  bool removeRedundantConditionalBranch(BlockT *SrcBlock);
  bool removeCodeAfterUnconditionalBranch(BlockT *SrcBlock);
  void addDummyExitBlock(SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> &RetBlocks);

  BlockT *cloneBlockForPredecessor(BlockT *CurBlock, BlockT *PredBlock);
  BlockT *exitingBlockToExitBlock(LoopT *LoopRep, BlockT *ExitingBlock);

  // Erase this block and any successors it have if they become
  // unreachable
  void deepPurgeSuccessors(BlockT *Block);

  // When we retire the final end block, we need to re-root the post dominator
  // tree to one of its children
  void replacePostDomTreeRoot();

  void migrateInstruction(BlockT *SrcBlock, BlockT *DstBlock,
                          InstrIterator InsertPos);

  void retireBlock(BlockT *Block);
  bool isRetiredBlock(BlockT *Block) const;
  bool isActiveLoopHead(const BlockT *CurBlock) const;
  bool needMigrateBlock(const BlockT *Block) const;

  BlockT *recordLoopLandBlock(LoopT *LoopRep,
                              BlockT *LandBlock,
                              const SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> &ExitBlockSet);

  BlockT *insertLoopDummyLandingBlock(const LoopT *LoopRep);
  void setLoopLandBlock(const LoopT *LoopRep, BlockT *Block);
  BlockT *getLoopLandBlock(const LoopT *LoopRep) const;
  LoopLandInfo *getLoopLandInfo(const LoopT *LoopRep) const;

  void addLoopBreakOnReg(LoopT *LoopRep, RegiT RegNum);
  void addLoopContOnReg(LoopT *LoopRep, RegiT RegNum);
  void addLoopBreakInitReg(LoopT *LoopRep, RegiT RegNum);
  void addLoopContInitReg(LoopT *LoopRep, RegiT RegNum);
  void addLoopEndBranchInitReg(LoopT *LoopRep, RegiT RegNum);

  bool hasBackEdge(const BlockT *CurBlock) const;
  unsigned getLoopDepth(const LoopT *LoopRep) const;

  unsigned countActiveBlocks(MutableArrayRef<BlockT *>);

  // Count non-retired blocks in the function
  // TODO: We should completely remove blocks from the function, and then just
  // use the size of the function, but can't do that now.
  unsigned countActiveBlocks();

  BlockT *findNearestCommonPostDom(const SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>&);

private:
  PostDomTreeT *postDomTree;
  LoopInfoT *loopInfo;
  PassT *passRep;
  FuncT *funcRep;

  std::set<BlockT *> retiredBlocks;
  LoopLandInfoMap loopLandInfoMap;
  std::set<unsigned> vregs; // New virtual registers created
};

template<class PassT> CFGStructurizer<PassT>::CFGStructurizer()
  : postDomTree(NULL), loopInfo(NULL) {
}

template<class PassT> CFGStructurizer<PassT>::~CFGStructurizer() {
  clearRetiredBlockSet();
  clearLoopLandInfoMap();
}

template<class PassT>
void CFGStructurizer<PassT>::irreducibleControlFlowError() {
  MachineFunction *MF = funcRep;
    AMDILMachineFunctionInfo *mMFI =
      MF->getInfo<AMDILMachineFunctionInfo>();
    mMFI->addErrorMsg(amd::CompilerErrorMessage[IRREDUCIBLE_CF]);

  DEBUG(
      dbgs() << "Irreducible function:\n";
      MF->dump();
      //MF->viewCFGOnly();
    );
}

template<class PassT>
bool CFGStructurizer<PassT>::prepare(FuncT &Func, PassT &Pass) {
  passRep = &Pass;
  funcRep = &Func;

  bool Changed = false;

  // TODO: If not reducible flow graph, make it so ???

  DEBUG(dbgs() << "AMDILCFGStructurizer::prepare\n");
  updateData();

  SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> RetBlocks;

  for (typename LoopInfoT::iterator I = loopInfo->begin(),
         E = loopInfo->end(); I != E; ++I) {
    LoopT *LoopRep = *I;
    BlockTSmallerVector ExitingBlocks;
    LoopRep->getExitingBlocks(ExitingBlocks);

    if (ExitingBlocks.empty()) {
      BlockT *DummyExitBlock = normalizeInfiniteLoopExit(LoopRep);
      if (DummyExitBlock) {
        RetBlocks.insert(DummyExitBlock);
      }
    }
  }

  // Do DFS of blocks in CFG, removing unconditional branch
  // instructions. Add dummy exit block iff there are multiple
  // returns.
  for (scc_iterator<FuncT *> SCCIter = scc_begin(funcRep);
       !SCCIter.isAtEnd(); ++SCCIter) {
    std::vector<BlockT *> &SCCNext = *SCCIter;
    for (typename std::vector<BlockT *>::const_iterator
           BlockIt = SCCNext.begin(), BlockEnd = SCCNext.end();
         BlockIt != BlockEnd; ++BlockIt) {
      BlockT *Block = *BlockIt;

      if (removeCodeAfterUnconditionalBranch(Block)) {
        Changed = true;
      }

      if (removeUnconditionalBranch(Block)) {
        Changed = true;
      }

      if (removeRedundantConditionalBranch(Block)) {
        Changed = true;
      }

      if (CFGTraits::isReturnBlock(Block)) {
        RetBlocks.insert(Block);
      }
      assert(Block->succ_size() <= 2);
    }
  }

  if (RetBlocks.size() >= 2) {
    addDummyExitBlock(RetBlocks);
    Changed = true;
  }
  DEBUG(funcRep->dump());

  return Changed;
}

template<class PassT>
void CFGStructurizer<PassT>::updateData() {
  // Assume reducible CFG...

  //funcRep->RenumberBlocks();

  postDomTree = CFGTraits::getPostDominatorTree(*passRep);
  loopInfo = CFGTraits::getLoopInfo(*passRep);

  DEBUG(
    dbgs() << "AMDILCFGStructurizer::updateData()\n";
    funcRep->dump();

    dbgs() << "PostDominatorTree:\n";
    postDomTree->print(dbgs(), (const llvm::Module*)NULL);

    dbgs() << "LoopInfo:\n";
    printLoopInfo(*loopInfo, dbgs());

    dbgs() << "Ordered blocks:\n";
    printOrderedBlocks(dbgs());
  );
}

template<class PassT>
bool CFGStructurizer<PassT>::run(FuncT &Func, PassT &Pass) {
  passRep = &Pass;
  funcRep = &Func;

  DEBUG(
    dbgs() << "AMDILCFGStructurizer::run\n";
    dbgs() << funcRep->getFunction()->getName() << '\n';
    //funcRep->viewCFGOnly();
    funcRep->dump();
  );

  updateData();

  int NIter = 0;
  while (true) {
    ++NIter;
    DEBUG(dbgs() << "NIter = " << NIter << '\n');

    bool changed = processFunction();

    BlockT *EntryBlock = FuncGTraits::nodes_begin(funcRep);
    if (EntryBlock->succ_empty()) {
      // Finished
      DEBUG(dbgs() << "Reduce to one block\n");
      break;
    }

    if (!changed) {
      DEBUG(dbgs() << "No progress\n");
      irreducibleControlFlowError();
      return true;
    }
  }

  // Misc wrap up to maintain the consistency of the Function representation.
  CFGTraits::wrapup(FuncGTraits::nodes_begin(funcRep));

  // Allocate physical registers for virtual registers created during this pass
  TrivialRegAlloc RegAlloc(*funcRep, AMDIL::GPR_32RegClass, vregs);
  RegAlloc.run();

  //DEBUG(funcRep->viewCFGOnly());

  return true;
}

template<class PassT>
void CFGStructurizer<PassT>::clearRetiredBlockSet() {
  // Detach retired Block, release memory.
  for (typename std::set<BlockT *>::iterator I = retiredBlocks.begin(),
         E = retiredBlocks.end(); I != E; ++I) {
    BlockT *Block = *I;
    assert(Block);
    assert(Block->getNumber() != -1);

    DEBUG(dbgs() << "Erase BB" << Block->getNumber() << '\n');

    Block->eraseFromParent(); // Remove from the parent Function.
  }

  retiredBlocks.clear();
}

template<class PassT>
void CFGStructurizer<PassT>::clearLoopLandInfoMap() {
  // Clear loopLandInfoMap
  for (typename LoopLandInfoMap::iterator I = loopLandInfoMap.begin(),
         E = loopLandInfoMap.end(); I != E; ++I) {
    delete I->second;
  }

  loopLandInfoMap.clear();
}

/// Print the ordered Blocks.
///
template<class PassT>
void CFGStructurizer<PassT>::printOrderedBlocks(llvm::raw_ostream &OS) {
  size_t SCCNum = 0;
  for (scc_iterator<FuncT *> SCCIter = scc_begin(funcRep);
       !SCCIter.isAtEnd(); ++SCCIter) {
    std::vector<BlockT *> &SCC = *SCCIter;
    for (typename std::vector<BlockT *>::const_iterator
           BlockIt = SCC.begin(), BlockEnd = SCC.end();
         BlockIt != BlockEnd; ++BlockIt) {
      BlockT *Block = *BlockIt;

      // Print BB(SCC ID, number of blocks in SCC)
      OS << "BB" << Block->getNumber();
      OS << '(' << SCCNum << ", " << SCC.size() << ')';
      if (SCCNum != 0 && SCCNum % 10 == 0) {
        OS << '\n';
      } else {
        OS << ' ';
      }
    }
  }

  OS << '\n';
}

template<class PassT>
bool CFGStructurizer<PassT>::processFunction() {
  bool changed = false;
  int SCCNum = 0;
  for (scc_iterator<FuncT *> SCCIter = scc_begin(funcRep);
       !SCCIter.isAtEnd(); ++SCCIter) {
    ++SCCNum;

    DEBUG(dbgs() << "Begin processing SCCNum = " << SCCNum << '\n');
    std::vector<BlockT *> &SCCNext = *SCCIter;

    if (SCCIter.hasLoop()) {
      DEBUG(dbgs() << "SCCNum " << SCCNum << " has a loop\n");
      // Try loop matches
    }

    unsigned SCCNumBlock = SCCNext.size();

    DEBUG(dbgs() << "SCC Num " << SCCNum << " size = " << SCCNumBlock << '\n');

    for (typename std::vector<BlockT *>::const_iterator
           BlockIt = SCCNext.begin(), BlockEnd = SCCNext.end();
         BlockIt != BlockEnd; ++BlockIt) {
      BlockT *Block = *BlockIt;

      int numMatch = patternMatch(Block);
      if (numMatch) {
        changed = true;
      }
    }

    DEBUG(
      unsigned RemainingNumSCCBlocks = countActiveBlocks(SCCNext);
      dbgs() << "RemainingNumSCCBlocks = " << RemainingNumSCCBlocks << '\n';

      if (RemainingNumSCCBlocks < SCCNumBlock) {
        dbgs() << "Repeat processing SCC" << SCCNum << '\n';
      }

      if (RemainingNumSCCBlocks == 0) {
        dbgs() << "Finish processing SCC " << SCCNum << '\n';
      } else { // RemainingNumSCCBlocks >= SCCNumBlock
        dbgs() << "Can't reduce processing SCC " << SCCNum
               << ", remain # of blocks " << RemainingNumSCCBlocks
               << " doesn't make any progress\n";
      }
    );
  }

  DEBUG(dbgs() << "Post processFunction CFG:\n");
  DEBUG(funcRep->dump());
  //DEBUG(funcRep->viewCFGOnly());

  return changed;
}

template<class PassT>
int CFGStructurizer<PassT>::patternMatch(BlockT *CurBlock) {
  int NumMatch = 0;
  int CurMatch;

  DEBUG(dbgs() << "Begin patternMatch BB" << CurBlock->getNumber() << '\n');

  while ((CurMatch = patternMatchGroup(CurBlock)) > 0) {
    NumMatch += CurMatch;
  }

  DEBUG(dbgs() << "End patternMatch BB" << CurBlock->getNumber()
        << ", NumMatch = " << NumMatch << '\n');

  return NumMatch;
}

template<class PassT>
int CFGStructurizer<PassT>::patternMatchGroup(BlockT *Block) {
  int NumMatch = 0;
  NumMatch += serialPatternMatch(Block);
  NumMatch += ifPatternMatch(Block);
  //NumMatch += switchPatternMatch(Block);
  NumMatch += loopEndPatternMatch(Block);
  NumMatch += loopPatternMatch(Block);
  return NumMatch;
}

template<class PassT>
int CFGStructurizer<PassT>::serialPatternMatch(BlockT *CurBlock) {
  if (CurBlock->succ_size() != 1) {
    return 0;
  }

  BlockT *ChildBlock = *CurBlock->succ_begin();
  if (ChildBlock->pred_size() != 1 || isActiveLoopHead(ChildBlock)) {
    return 0;
  }

  mergeSerialBlock(CurBlock, ChildBlock);
  ++numSerialPatternMatch;
  return 1;
}

template<class PassT>
int CFGStructurizer<PassT>::ifPatternMatch(BlockT *CurBlock) {
  // Two edges
  if (CurBlock->succ_size() != 2) {
    return 0;
  }

  if (hasBackEdge(CurBlock)) {
    return 0;
  }

  InstrT *BranchInstr = CFGTraits::getNormalBlockBranchInstr(CurBlock);
  if (!BranchInstr) {
    return 0;
  }

  assert(BranchInstr->isConditionalBranch());

  BlockT *TrueBlock = CFGTraits::getTrueBranch(BranchInstr);
  BlockT *FalseBlock = CFGTraits::getFalseBranch(CurBlock, BranchInstr);
  BlockT *LandBlock = NULL;
  int Cloned = 0;

  // TODO: Simplify
  if (TrueBlock->succ_size() == 1 && FalseBlock->succ_size() == 1
    && *TrueBlock->succ_begin() == *FalseBlock->succ_begin()) {
    LandBlock = *TrueBlock->succ_begin();
  } else if (TrueBlock->succ_empty() && FalseBlock->succ_empty()) {
    LandBlock = NULL;
  } else if (TrueBlock->succ_size() == 1 && *TrueBlock->succ_begin() == FalseBlock) {
    LandBlock = FalseBlock;
    FalseBlock = NULL;
  } else if (FalseBlock->succ_size() == 1
             && *FalseBlock->succ_begin() == TrueBlock) {
    LandBlock = TrueBlock;
    TrueBlock = NULL;
  } else {
    return handleJumpIntoIf(CurBlock, TrueBlock, FalseBlock);
  }

  // improveSimpleJumpIntoIf can handle the case where LandBlock == NULL
  // but the new BB created for LandBlock == NULL may introduce new
  // challenge to the reduction process.
  if (LandBlock &&
      ((TrueBlock && TrueBlock->pred_size() > 1)
      || (FalseBlock && FalseBlock->pred_size() > 1))) {
     Cloned += improveSimpleJumpIntoIf(CurBlock, TrueBlock, FalseBlock, &LandBlock);
  }

  if (TrueBlock && TrueBlock->pred_size() > 1) {
    TrueBlock = cloneBlockForPredecessor(TrueBlock, CurBlock);
    ++Cloned;
  }

  if (FalseBlock && FalseBlock->pred_size() > 1) {
    FalseBlock = cloneBlockForPredecessor(FalseBlock, CurBlock);
    ++Cloned;
  }

  mergeIfThenElseBlock(BranchInstr, CurBlock, TrueBlock, FalseBlock, LandBlock);

  ++numIfPatternMatch;

  numClonedBlock += Cloned;

  return 1 + Cloned;
}

template<class PassT>
int CFGStructurizer<PassT>::switchPatternMatch(BlockT *CurBlock) {
  return 0;
}

template<class PassT>
void CFGStructurizer<PassT>::findNestedLoops(
  SmallVector<LoopT *, DEFAULT_VEC_SLOTS>& NestedLoops,
  const BlockT *Block) {
  LoopT *LoopRep = loopInfo->getLoopFor(Block);
  while (LoopRep) {
    NestedLoops.push_back(LoopRep);
    LoopRep = LoopRep->getParentLoop();
  }
}

template<class PassT>
int CFGStructurizer<PassT>::loopEndPatternMatch(BlockT *CurBlock) {
  SmallVector<LoopT *, DEFAULT_VEC_SLOTS> NestedLoops;

  findNestedLoops(NestedLoops, CurBlock);

  DEBUG(dbgs() << "Begin loopEndPatternMatch BB" << CurBlock->getNumber() << '\n');

  // Process nested loop outside->inside, so "continue" to an outside
  // loop won't be mistaken as "break" of the current loop.
  int Num = 0;
  for (typename SmallVector<LoopT *, DEFAULT_VEC_SLOTS>::reverse_iterator
         I = NestedLoops.rbegin(), E = NestedLoops.rend();
       I != E; ++I) {
    LoopT *LoopRep = *I;

    if (getLoopLandBlock(LoopRep)) {
      continue;
    }

    int NumBreak = loopBreakPatternMatch(LoopRep);
    if (NumBreak == -1) {
      break;
    }

    int NumCont = loopContPatternMatch(LoopRep);

    Num += NumBreak + NumCont;
  }

  return Num;
}

template<class PassT>
int CFGStructurizer<PassT>::loopPatternMatch(BlockT *CurBlock) {
  if (!CurBlock->succ_empty()) {
    return 0;
  }

  int NumLoop = 0;
  LoopT *LoopRep = loopInfo->getLoopFor(CurBlock);
  while (LoopRep && LoopRep->getHeader() == CurBlock) {
    LoopLandInfo *LoopLand = getLoopLandInfo(LoopRep);
    if (LoopLand) {
      BlockT *LandBlock = LoopLand->landBlk;
      assert(LandBlock);
      if (!isRetiredBlock(LandBlock)) {
        settleLoop(CurBlock, LoopLand);
        ++NumLoop;
      }
    }
    LoopRep = LoopRep->getParentLoop();
  }

  numLoopPatternMatch += NumLoop;

  return NumLoop;
}

template<class PassT>
void CFGStructurizer<PassT>::foldBreakingBlock(LoopT *LoopRep,
                                               BlockT *ExitLandBlock) {
  SmallVector<LoopEdgeT, 4> ExitEdges;
  LoopRep->getExitEdges(ExitEdges);

  // Fold break into the breaking block. Leverage across level breaks.
  for (typename ArrayRef<LoopEdgeT>::const_iterator I = ExitEdges.begin(),
         E = ExitEdges.end(); I != E; ++I) {
    BlockT *ExitingBlock = const_cast<BlockT*>(I->first);
    BlockT *ExitBlock = const_cast<BlockT*>(I->second);

    assert(ExitBlock->pred_size() == 1 || ExitBlock == ExitLandBlock);
    LoopT *ExitingLoop = loopInfo->getLoopFor(ExitingBlock);
    handleLoopBreak(ExitingBlock,
                    ExitingLoop,
                    ExitBlock,
                    LoopRep,
                    ExitLandBlock);
  }
}

template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::findLoopBreakLandingBlock(
  LoopT *LoopRep,
  const SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> &ExitBlockSet,
  ArrayRef<BlockT *> ExitBlocks,
  ArrayRef<BlockT *> ExitingBlocks) {
  BlockT *ExitLandBlock = findNearestCommonPostDom(ExitBlockSet);
  if (!ExitLandBlock) {
    return NULL;
  }

  bool AllInPath = true;
  bool AllNotInPath = true;

  // Check all exit blocks
  for (typename SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>::const_iterator
         I = ExitBlockSet.begin(), E = ExitBlockSet.end(); I != E; ++I) {
    BlockT *ExitBlock = *I;
    PathToKind PathKind = singlePathTo(ExitBlock, ExitLandBlock, true);

    DEBUG(dbgs() << "BB" << ExitBlock->getNumber()
          << " to BB" << ExitLandBlock->getNumber() << " PathToKind="
          << PathKind << '\n');

    AllInPath &= (PathKind == SinglePath_InPath);
    AllNotInPath &= (PathKind == SinglePath_NotInPath);

    if (!AllInPath && !AllNotInPath) {
      DEBUG(dbgs() << "singlePath check fail\n");
      return NULL;
    }
  }

  if (AllNotInPath) {
    LoopT *ParentLoopRep = LoopRep->getParentLoop();
    BlockT *ParentLoopHeader
        = ParentLoopRep ? ParentLoopRep->getHeader() : NULL;

    if (ExitLandBlock == ParentLoopHeader) {
      ExitLandBlock = relocateLoopContBlock(ParentLoopRep,
                                            LoopRep,
                                            ExitBlockSet,
                                            ExitLandBlock);
      if (ExitLandBlock) {
        DEBUG(dbgs() << "relocateLoopContBlock success\n");
        return ExitLandBlock;
      }
    }

    ExitLandBlock = addLoopEndBranchBlock(LoopRep,
                                          ExitingBlocks,
                                          ExitBlocks);
    if (ExitLandBlock) {
      DEBUG(dbgs() << "insertEndBranchBlock success\n");
      return ExitLandBlock;
    }

    DEBUG(dbgs() << "Loop exit fail\n");
    return NULL;
  }

  // Current addLoopEndBranchBlock always does something and return
  // non-NULL
  ExitLandBlock = addLoopEndBranchBlock(LoopRep,
                                        ExitingBlocks,
                                        ExitBlocks);
  if (ExitLandBlock) {
    DEBUG(dbgs() << "insertEndBranchBlock success\n");
    return ExitLandBlock;
  }

  return NULL;
}

template<class PassT>
int CFGStructurizer<PassT>::loopBreakPatternMatch(LoopT *LoopRep) {
  BlockTSmallerVector ExitingBlocks;
  LoopRep->getExitingBlocks(ExitingBlocks);

  DEBUG(dbgs() << "Loop has " << ExitingBlocks.size() << " exiting blocks\n");

  if (ExitingBlocks.empty()) {
    BlockT *DummyLandingBlock = insertLoopDummyLandingBlock(LoopRep);
    setLoopLandBlock(LoopRep, DummyLandingBlock);
    return 0;
  }

  // Compute the corresponding ExitBlocks and exit block set.
  BlockTSmallerVector ExitBlocks;
  LoopRep->getExitBlocks(ExitBlocks);

  SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> ExitBlockSet(ExitBlocks.begin(),
                                                        ExitBlocks.end());

  assert(!ExitBlockSet.empty());
  assert(ExitBlocks.size() == ExitingBlocks.size());

  DEBUG(dbgs() << "Loop has " << ExitBlockSet.size() << " exit blocks\n");

  if (ExitBlockSet.size() == 1) {
    BlockT *ExitLandBlock = *ExitBlocks.begin();
    DEBUG(dbgs() << "ExitLandBlock = recordLoopLandBlock\n");
    ExitLandBlock = recordLoopLandBlock(LoopRep,
                                        ExitLandBlock,
                                        ExitBlockSet);

    foldBreakingBlock(LoopRep, ExitLandBlock);

    return static_cast<int>(ExitingBlocks.size()); // Number of breaks
  }

  BlockT *ExitLandBlock = findLoopBreakLandingBlock(LoopRep,
                                                    ExitBlockSet,
                                                    ExitBlocks,
                                                    ExitingBlocks);
  if (!ExitLandBlock) {
    return -1;
  }

  int NumCloned = 0;
  int NumSerial = 0;
  SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> NewExitBlockSet;

  // Handle side entry to exit path.
  for (typename BlockTSmallerVector::iterator I = ExitingBlocks.begin(),
         E = ExitingBlocks.end(); I != E; ++I) {
    BlockT *ExitingBlock = *I;
    BlockT *ExitBlock = exitingBlockToExitBlock(LoopRep, ExitingBlock);
    BlockT *NewExitBlock = ExitBlock;

    if (ExitBlock != ExitLandBlock && ExitBlock->pred_size() > 1) {
      NewExitBlock = cloneBlockForPredecessor(ExitBlock, ExitingBlock);
      ++NumCloned;
    }

    NumCloned += cloneOnSideEntryTo(ExitingBlock, NewExitBlock, ExitLandBlock);

    NewExitBlockSet.insert(NewExitBlock);
  }

  for (typename SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>::iterator
         I = NewExitBlockSet.begin(), E = NewExitBlockSet.end(); I != E; ++I) {
    BlockT *ExitBlock = *I;
    NumSerial += serialPatternMatch(ExitBlock);
  }

  for (typename SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>::iterator
         I = NewExitBlockSet.begin(), E = NewExitBlockSet.end(); I != E; ++I) {
    BlockT *ExitBlock = *I;
    if (ExitBlock->pred_size() > 1) {
      if (ExitBlock != ExitLandBlock) {
        return -1;
      }
    } else {
      if (ExitBlock != ExitLandBlock &&
          (ExitBlock->succ_size() != 1 ||
           *ExitBlock->succ_begin() != ExitLandBlock)) {
        return -1;
      }
    }
  }

  DEBUG(dbgs() << "ExitLandBlock = recordLoopLandBlock\n");
  ExitLandBlock = recordLoopLandBlock(LoopRep,
                                      ExitLandBlock,
                                      NewExitBlockSet);

  foldBreakingBlock(LoopRep, ExitLandBlock);

  int NumBreak = static_cast<int>(ExitingBlocks.size());
  numLoopBreakPatternMatch += NumBreak;
  numClonedBlock += NumCloned;
  return NumBreak + NumSerial + NumCloned;
}

template<class PassT>
int CFGStructurizer<PassT>::loopContPatternMatch(LoopT *LoopRep) {
  BlockT *LoopHeader = LoopRep->getHeader();

  int NumCont = 0;
  SmallVector<BlockT *, DEFAULT_VEC_SLOTS> ContBlocks;
  for (typename InvBlockGTraits::ChildIteratorType I =
         InvBlockGTraits::child_begin(LoopHeader),
         E = InvBlockGTraits::child_end(LoopHeader);
       I != E; ++I) {
    BlockT *Child = *I;
    if (LoopRep->contains(Child)) {
      handleLoopContBlock(Child, loopInfo->getLoopFor(Child),
                          LoopHeader, LoopRep);
      ContBlocks.push_back(Child);
      ++NumCont;
    }
  }

  for (typename ArrayRef<BlockT *>::iterator
         I = ContBlocks.begin(), E = ContBlocks.end();
       I != E; ++I) {
    BlockT *Block = *I;
    Block->removeSuccessor(LoopHeader);
  }

  numLoopContPatternMatch += NumCont;

  return NumCont;
}

template<class PassT>
int CFGStructurizer<PassT>::handleJumpIntoIf(BlockT *HeadBlock,
                                             BlockT *TrueBlock,
                                             BlockT *FalseBlock) {
  int Num = handleJumpIntoIfImp(HeadBlock, TrueBlock, FalseBlock);
  if (Num != 0) {
    DEBUG(dbgs() << "First handleJumpIntoIf succeeded with "
          << Num << " matches\n");
    return Num;
  }

  DEBUG(dbgs() << "handleJumpIntoIf swap TrueBlock and FalseBlock" << '\n');

  Num = handleJumpIntoIfImp(HeadBlock, FalseBlock, TrueBlock);
  if (Num != 0) {
    DEBUG(dbgs() << "Second handleJumpIntoIf succeeded with "
          << Num << " matches\n");
    return Num;
  }

  DEBUG(dbgs() << "handleJumpIntoIf check NULL as common postdom: ");

  BlockT *TrueEnd = NULL;
  BlockT *FalseEnd = NULL;
  if ((TrueEnd = singlePathEnd(TrueBlock, NULL)) &&
      (FalseEnd = singlePathEnd(FalseBlock, NULL)) &&
      loopInfo->getLoopFor(TrueEnd) == loopInfo->getLoopFor(FalseEnd)) {
    DEBUG(dbgs() << "working\n");

    Num += cloneOnSideEntryTo(HeadBlock, TrueBlock, NULL);
    Num += cloneOnSideEntryTo(HeadBlock, FalseBlock, NULL);

    numClonedBlock += Num;
    Num += serialPatternMatch(*HeadBlock->succ_begin());
    Num += serialPatternMatch(*(HeadBlock->succ_begin() + 1));
    Num += ifPatternMatch(HeadBlock);
    assert(Num > 0);

    return Num;
  }

  DEBUG(dbgs() << " not working\n");
  return 0;
}

template<class PassT>
int CFGStructurizer<PassT>::handleJumpIntoIfImp(BlockT *HeadBlock,
                                                BlockT *TrueBlock,
                                                BlockT *FalseBlock) {
  int Num = 0;

  // TrueBlock could be the common post dominator
  BlockT *DownBlock = TrueBlock;

  DEBUG(dbgs() << "handleJumpIntoIfImp head = BB" << HeadBlock->getNumber()
        << " true = BB" << TrueBlock->getNumber()
        << ", numSucc = " << TrueBlock->succ_size()
        << " false = BB" << FalseBlock->getNumber() << '\n');

  while (DownBlock) {
    DEBUG(dbgs() << "Check down = BB" << DownBlock->getNumber());

    if (//postDomTree->dominates(DownBlock, FalseBlock) &&
        singlePathTo(FalseBlock, DownBlock) == SinglePath_InPath) {
      DEBUG(dbgs() << " working\n");

      Num += cloneOnSideEntryTo(HeadBlock, TrueBlock, DownBlock);
      Num += cloneOnSideEntryTo(HeadBlock, FalseBlock, DownBlock);

      numClonedBlock += Num;
      Num += serialPatternMatch(*HeadBlock->succ_begin());
      Num += serialPatternMatch(*(HeadBlock->succ_begin() + 1));
      Num += ifPatternMatch(HeadBlock);
      assert(Num > 0);

      break;
    }

    DEBUG(dbgs() << " not working\n");

    DownBlock = (DownBlock->succ_size() == 1) ? *DownBlock->succ_begin() : NULL;
  } // Walk down the postDomTree

  return Num;
}

template<class PassT>
void CFGStructurizer<PassT>::showImproveSimpleJumpIntoIf(BlockT *HeadBlock,
                                                         BlockT *TrueBlock,
                                                         BlockT *FalseBlock,
                                                         BlockT *LandBlock,
                                                         bool Detail) {
  dbgs() << "head = BB" << HeadBlock->getNumber()
         << " size = " << HeadBlock->size();
  if (Detail) {
    dbgs() << '\n';
    HeadBlock->print(dbgs());
    dbgs() << '\n';
  }

  if (TrueBlock) {
    dbgs() << ", true = BB" << TrueBlock->getNumber() << " size = "
           << TrueBlock->size() << " numPred = " << TrueBlock->pred_size();
    if (Detail) {
      dbgs() << '\n';
      TrueBlock->print(dbgs());
      dbgs() << '\n';
    }
  }
  if (FalseBlock) {
    dbgs() << ", false = BB" << FalseBlock->getNumber() << " size = "
           << FalseBlock->size() << " numPred = " << FalseBlock->pred_size();
    if (Detail) {
      dbgs() << '\n';
      FalseBlock->print(dbgs());
      dbgs() << '\n';
    }
  }
  if (LandBlock) {
    dbgs() << ", land = BB" << LandBlock->getNumber() << " size = "
           << LandBlock->size() << " numPred = " << LandBlock->pred_size();
    if (Detail) {
      dbgs() << '\n';
      LandBlock->print(dbgs());
      dbgs() << '\n';
    }
  }

  dbgs() << '\n';
}

template<class PassT>
int CFGStructurizer<PassT>::improveSimpleJumpIntoIf(BlockT *HeadBlock,
                                                    BlockT *TrueBlock,
                                                    BlockT *FalseBlock,
                                                    BlockT **pLandBlock) {
  assert((!TrueBlock || TrueBlock->succ_size() <= 1)
         && (!FalseBlock || FalseBlock->succ_size() <= 1));

  if (TrueBlock == FalseBlock) {
    return 0;
  }

  // unsigned LandPredSize = LandBlock ? LandBlock->pred_size() : 0;
  // May consider the # LandBlock->pred_size() as it represents the number of
  // assignment InitReg = .. needed to insert.
  bool MigrateTrue = needMigrateBlock(TrueBlock);
  bool MigrateFalse = needMigrateBlock(FalseBlock);

  if (!MigrateTrue && !MigrateFalse) {
    return 0;
  }

  // If we need to migrate either TrueBlock and FalseBlock, migrate the
  // rest that have more than one predecessor. Without doing this, its
  // predecessor rather than HeadBlock will have undefined value in
  // InitReg.
  if (!MigrateTrue && TrueBlock && TrueBlock->pred_size() > 1) {
    MigrateTrue = true;
  }
  if (!MigrateFalse && FalseBlock && FalseBlock->pred_size() > 1) {
    MigrateFalse = true;
  }

  BlockT *LandBlock = *pLandBlock;

  DEBUG(
    dbgs() << "Before improveSimpleJumpIntoIf: ";
    showImproveSimpleJumpIntoIf(HeadBlock, TrueBlock, FalseBlock, LandBlock, false);
  );

  // org: HeadBlock => if () {TrueBlock} else {FalseBlock} => LandBlock
  //
  // new: HeadBlock => if () {InitReg = 1; org TrueBlock branch} else
  //      {InitReg = 0; org FalseBlock branch }
  //      => LandBlock => if (InitReg) {org TrueBlock} else {org FalseBlock}
  //      => org LandBlock
  //      if LandBlock->pred_size() > 2, put the about if-else inside
  //      if (InitReg !=2) {...}
  //
  // add InitReg = initVal to HeadBlock
  unsigned InitReg = getRegister(&AMDIL::GPR_32RegClass);
  if (!MigrateTrue || !MigrateFalse) {
    int InitVal = MigrateTrue ? 0 : 1;
    CFGTraits::insertAssignInstrBefore(HeadBlock, passRep, InitReg, InitVal);
  }

  int NumNewBlock = 0;

  if (!LandBlock) {
    assert(!(TrueBlock && FalseBlock)
           && "Should only have a true or a false block");

    LandBlock = funcRep->CreateMachineBasicBlock();
    funcRep->push_back(LandBlock);  // Insert to function

    if (TrueBlock) {
      TrueBlock->addSuccessor(TrueBlock);
    }

    if (FalseBlock) {
      FalseBlock->addSuccessor(LandBlock);
    }

    HeadBlock->addSuccessor(LandBlock);

    ++NumNewBlock;
  }

  bool LandBlockHasOtherPred = (LandBlock->pred_size() > 2);

  // Insert AMDIL::ENDIF to avoid special case "input LandBlock == NULL"
  typename BlockT::iterator InsertPos =
    CFGTraits::getInstrPos
     (LandBlock, CFGTraits::insertInstrBefore(LandBlock, AMDIL::ENDIF, passRep));

  if (LandBlockHasOtherPred) {
    unsigned ImmReg = getRegister(&AMDIL::GPR_32RegClass);
    CFGTraits::insertAssignInstrBefore(InsertPos, passRep, ImmReg, 2);
    unsigned CmpResReg = getRegister(&AMDIL::GPR_32RegClass);
    CFGTraits::insertCompareInstrBefore(LandBlock, InsertPos, passRep, CmpResReg,
                                        InitReg, ImmReg);
    CFGTraits::insertCondBranchBefore(LandBlock, InsertPos,
                                      AMDIL::IF_LOGICALZi32r, passRep,
                                      CmpResReg, DebugLoc());
  }

  CFGTraits::insertCondBranchBefore(LandBlock, InsertPos, AMDIL::IF_LOGICALNZi32r,
                                    passRep, InitReg, DebugLoc());

  if (MigrateTrue) {
    migrateInstruction(TrueBlock, LandBlock, InsertPos);
    // Need to unconditionally insert the assignment to ensure a path from its
    // predecessor rather than HeadBlock has valid value in InitReg if
    // (initVal != 1).
    CFGTraits::insertAssignInstrBefore(TrueBlock, passRep, InitReg, 1);
  }

  if (FalseBlock)
    CFGTraits::insertInstrBefore(InsertPos, AMDIL::ELSE, passRep);

  if (MigrateFalse) {
    migrateInstruction(FalseBlock, LandBlock, InsertPos);
    // Need to unconditionally insert the assignment to ensure a path from its
    // predecessor rather than HeadBlock has valid value in InitReg if
    // (initVal != 0)
    CFGTraits::insertAssignInstrBefore(FalseBlock, passRep, InitReg, 0);
  }
  //CFGTraits::insertInstrBefore(InsertPos, AMDIL::ENDIF, passRep);

  if (LandBlockHasOtherPred) {
    // Add endif
    CFGTraits::insertInstrBefore(InsertPos, AMDIL::ENDIF, passRep);

    // Put InitReg = 2 to other predecessors of LandBlock
    for (typename BlockT::pred_iterator PI = LandBlock->pred_begin(),
           PE = LandBlock->pred_end(); PI != PE; ++PI) {
      BlockT *CurBlock = *PI;
      if (CurBlock != TrueBlock && CurBlock != FalseBlock) {
        CFGTraits::insertAssignInstrBefore(CurBlock, passRep, InitReg, 2);
      }
    }
  }

  DEBUG(
    dbgs() << "Result from improveSimpleJumpIntoIf: ";
    showImproveSimpleJumpIntoIf(HeadBlock, TrueBlock, FalseBlock, LandBlock, false);
  );

  // Update LandBlock
  *pLandBlock = LandBlock;

  return NumNewBlock;
}

// Since we are after the register allocator, we don't want to use
// virtual registers as it is possible that we can get a virtual
// register that is passed the 65K limit of IL text format. So instead
// we serach through the register class for an unused physical
// register and mark it as used. If we cannot find a register, then we
// do some funky math on the virtual registers so that we don't
// clobber the physicals and make sure we don't go over the 65k limit.
template<class PassT>
inline int CFGStructurizer<PassT>::getRegister(
  const TargetRegisterClass *RegClass) {
  unsigned Reg = funcRep->getRegInfo().createVirtualRegister(RegClass);
  vregs.insert(Reg);

  DEBUG(dbgs() << "Created virtual register "
        << TargetRegisterInfo::virtReg2Index(Reg) << '\n');

  return Reg;
}

template<class PassT>
void CFGStructurizer<PassT>::handleLoopBreak(BlockT *ExitingBlock,
                                             LoopT *ExitingLoop,
                                             BlockT *ExitBlock,
                                             LoopT *ExitLoop,
                                             BlockT *LandBlock) {
  DEBUG(dbgs() << "Trying to break loop-depth = " << getLoopDepth(ExitLoop)
        << " from loop-depth = " << getLoopDepth(ExitingLoop) << '\n');

  if (ExitingLoop == ExitLoop) {
    mergeLoopBreakBlock(ExitingBlock, ExitBlock, LandBlock, INVALIDREGNUM);
    return;
  }

  RegiT InitReg = getRegister(&AMDIL::GPR_32RegClass);
  assert(InitReg != INVALIDREGNUM);
  addLoopBreakInitReg(ExitLoop, InitReg);
  while (ExitingLoop != ExitLoop && ExitingLoop) {
    addLoopBreakOnReg(ExitingLoop, InitReg);
    ExitingLoop = ExitingLoop->getParentLoop();
  }
  assert(ExitingLoop == ExitLoop);

  mergeLoopBreakBlock(ExitingBlock, ExitBlock, LandBlock, InitReg);
}

template<class PassT>
void CFGStructurizer<PassT>::handleLoopContBlock(BlockT *ContingBlock,
                                                 LoopT *ContingLoop,
                                                 BlockT *ContBlock,
                                                 LoopT *ContLoop) {
  DEBUG(
    dbgs() << "loopContPattern cont = BB" << ContingBlock->getNumber()
    << " header = BB" << ContBlock->getNumber() << '\n'
    << "Trying to continue loop-depth = "
    << getLoopDepth(ContLoop)
    << " from loop-depth = " << getLoopDepth(ContingLoop) << '\n'
  );

  if (ContingLoop == ContLoop) {
    settleLoopContBlock(ContingBlock, ContBlock, INVALIDREGNUM);
    return;
  }

  RegiT InitReg = getRegister(&AMDIL::GPR_32RegClass);
  assert(InitReg != INVALIDREGNUM);

  addLoopContInitReg(ContLoop, InitReg);
  while (ContingLoop && ContingLoop->getParentLoop() != ContLoop) {
    addLoopBreakOnReg(ContingLoop, InitReg); // Not addLoopContOnReg
    ContingLoop = ContingLoop->getParentLoop();
  }
  assert(ContingLoop && ContingLoop->getParentLoop() == ContLoop);
  addLoopContOnReg(ContingLoop, InitReg);
  settleLoopContBlock(ContingBlock, ContBlock, InitReg);
  //ContingBlock->removeSuccessor(LoopHeader);
}

template<class PassT>
void CFGStructurizer<PassT>::mergeSerialBlock(BlockT *DestBlock,
                                              BlockT *SrcBlock) {
  DEBUG(dbgs() << "serialPattern BB" << DestBlock->getNumber()
        << " <= BB" << SrcBlock->getNumber() << '\n');

  DestBlock->splice(DestBlock->end(),
                    SrcBlock,
                    FirstNonDebugInstr(SrcBlock),
                    SrcBlock->end());
  DestBlock->removeSuccessor(SrcBlock);
  DestBlock->transferSuccessors(SrcBlock);

  retireBlock(SrcBlock);
}

template<class PassT>
void CFGStructurizer<PassT>::mergeIfThenElseBlock(InstrT *BranchInstr,
                                                  BlockT *CurBlock,
                                                  BlockT *TrueBlock,
                                                  BlockT *FalseBlock,
                                                  BlockT *LandBlock) {
  DEBUG(
    dbgs() << "ifPattern BB" << CurBlock->getNumber() << " {  ";
    if (TrueBlock) {
      dbgs() << "BB" << TrueBlock->getNumber();
    }
    dbgs() << "  } else {  ";
    if (FalseBlock) {
      dbgs() << "BB" << FalseBlock->getNumber();
    }
    dbgs() << "  }\nLandBlock: ";
    if (LandBlock) {
      dbgs() << "BB" << LandBlock->getNumber() << '\n';
    } else {
      dbgs() << "NULL\n";
    }
  );

  unsigned OldOpcode = BranchInstr->getOpcode();
  DebugLoc BranchDL = BranchInstr->getDebugLoc();
  if (BranchDL.isUnknown()) {
    InstrT *BreakInstr = getLastBreakInstr(CurBlock);
    if (BreakInstr) {
      BranchDL = BreakInstr->getDebugLoc();
    }
  }

  // Transform to
  // if cond
  //   TrueBlock
  // else
  //   FalseBlock
  // endif
  // LandBlock

  typename BlockT::iterator BranchInstrPos =
    CFGTraits::getInstrPos(CurBlock, BranchInstr);
  CFGTraits::insertCondBranchBefore(BranchInstrPos,
                                    CFGTraits::getBranchNZeroOpcode(OldOpcode),
                                    passRep,
                                    BranchDL);

  if (TrueBlock) {
    CurBlock->splice(BranchInstrPos,
                     TrueBlock,
                     FirstNonDebugInstr(TrueBlock),
                     TrueBlock->end());
    CurBlock->removeSuccessor(TrueBlock);
    if (LandBlock && !TrueBlock->succ_empty()) {
      TrueBlock->removeSuccessor(LandBlock);
    }
    retireBlock(TrueBlock);
  }

  if (FalseBlock) {
    CFGTraits::insertInstrBefore(BranchInstrPos, AMDIL::ELSE, passRep);
    CurBlock->splice(BranchInstrPos,
                     FalseBlock,
                     FirstNonDebugInstr(FalseBlock),
                     FalseBlock->end());
    CurBlock->removeSuccessor(FalseBlock);
    if (LandBlock && !FalseBlock->succ_empty()) {
      FalseBlock->removeSuccessor(LandBlock);
    }
    retireBlock(FalseBlock);
  }
  CFGTraits::insertInstrBefore(BranchInstrPos, AMDIL::ENDIF, passRep);

  //CurBlock->remove(BranchInstrPos);
  BranchInstr->eraseFromParent();

  if (LandBlock && TrueBlock && FalseBlock) {
    CurBlock->addSuccessor(LandBlock);
  }
}

template<class PassT>
void CFGStructurizer<PassT>::settleLoop(BlockT *DestBlock,
                                        LoopLandInfo *LoopLand) {
  BlockT *LandBlock = LoopLand->landBlk;

  DEBUG(dbgs() << "loopPattern header = BB" << DestBlock->getNumber()
        << " land = BB" << LandBlock->getNumber() << '\n');

  // Loop contInitRegs are init at the beginning of the loop.
  for (typename std::set<RegiT>::const_iterator I =
         LoopLand->contInitRegs.begin(),
       E = LoopLand->contInitRegs.end(); I != E; ++I) {
    CFGTraits::insertAssignInstrBefore(DestBlock, passRep, *I, 0);
  }

  // Loop endBranchInitRegs are init after entering the loop.
  for (typename std::set<RegiT>::const_iterator I =
         LoopLand->endBranchInitRegs.begin(),
         E = LoopLand->endBranchInitRegs.end(); I != E; ++I) {
    CFGTraits::insertAssignInstrBefore(DestBlock, passRep, *I, 0);
  }

  // We last inserted the DebugLoc in the BREAK_LOGICALZi32r or
  // AMDIL::BREAK_LOGICALNZ statement in the current DestBlock. Search for the
  // DebugLoc in the that statement. If not found, we have to insert the
  // empty/default DebugLoc
  InstrT *LoopBreakInstr = CFGTraits::getLoopBreakInstr(DestBlock);
  DebugLoc DLBreak = LoopBreakInstr ? LoopBreakInstr->getDebugLoc() : DebugLoc();

  // fogbugz #7310: work-around discussed with Uri regarding do-while
  // loops: in case the the WHILELOOP line number is greater than
  // do.body line numbers, take the do.body line number instead.
  MachineBasicBlock::iterator DestBegin = DestBlock->begin();
  MachineInstr *InstrDoBody = &*DestBegin;
  DebugLoc DLBreakDoBody = InstrDoBody ? InstrDoBody->getDebugLoc() : DebugLoc();
  DebugLoc DLBreakMin = (DLBreak.getLine() < DLBreakDoBody.getLine()) ? DLBreak : DLBreakDoBody;

  CFGTraits::insertInstrBefore(DestBlock, AMDIL::WHILELOOP, passRep, DLBreakMin);
  // Loop breakInitRegs are init before entering the loop.
  for (typename std::set<RegiT>::const_iterator I =
         LoopLand->breakInitRegs.begin(), E = LoopLand->breakInitRegs.end();
       I != E; ++I) {
    CFGTraits::insertAssignInstrBefore(DestBlock, passRep, *I, 0);
  }

  // We last inserted the DebugLoc in the continue statement in the current
  // DestBlock search for the DebugLoc in the continue statement. If not found, we
  // have to insert the empty/default DebugLoc
  InstrT *ContinueInstr = CFGTraits::getContinueInstr(DestBlock);
  DebugLoc DLContinue = ContinueInstr ? ContinueInstr->getDebugLoc() : DebugLoc();

  CFGTraits::insertInstrEnd(DestBlock, AMDIL::ENDLOOP, passRep, DLContinue);
  // Loop breakOnRegs are check after the ENDLOOP: break the loop outside this
  // loop.
  for (typename std::set<RegiT>::const_iterator I =
         LoopLand->breakOnRegs.begin(), E = LoopLand->breakOnRegs.end();
       I != E; ++I) {
    CFGTraits::insertCondBranchEnd(DestBlock,
                                   AMDIL::BREAK_LOGICALNZi32r,
                                   passRep,
                                   *I);
  }

  // Loop contOnRegs are check after the ENDLOOP: cont the loop
  // outside this loop.
  for (std::set<RegiT>::const_iterator I = LoopLand->contOnRegs.begin(),
         E = LoopLand->contOnRegs.end(); I != E; ++I) {
    CFGTraits::insertCondBranchEnd(DestBlock,
                                   AMDIL::CONTINUE_LOGICALNZi32r,
                                   passRep,
                                   *I);
  }
  // Cannot simply merge LandBlock with DestBlock, because LandBlock may have
  // other predecessors, e.g., LandBlock may be the header for another loop.
  // Simply link DestBlock with LandBlock and let the future iterations to
  // handle this link.
  DestBlock->addSuccessor(LandBlock);
}

template<class PassT>
void CFGStructurizer<PassT>::mergeLoopBreakBlock(BlockT *ExitingBlock,
                                                 BlockT *ExitBlock,
                                                 BlockT *ExitLandBlock,
                                                 RegiT SetReg) {
  DEBUG(dbgs() << "loopBreakPattern exiting = BB" << ExitingBlock->getNumber()
        << " exit = BB" << ExitBlock->getNumber()
        << " land = BB" << ExitLandBlock->getNumber() << '\n');

  InstrT *BranchInstr = CFGTraits::getLoopEndBlockBranchInstr(ExitingBlock);
  assert(BranchInstr && BranchInstr->isConditionalBranch());

  DebugLoc DL = BranchInstr->getDebugLoc();

  BlockT *TrueBranch = CFGTraits::getTrueBranch(BranchInstr);
  unsigned OldOpcode = BranchInstr->getOpcode();

  // Transform ExitingBlock to
  // if ( ) {
  //    ExitBlock (if ExitBlock != ExitLandBlock)
  //    SetReg = 1
  //    break
  // }endif
  // successor = {orgSuccessor(ExitingBlock) - ExitBlock}

  typename BlockT::iterator BranchInstrPos =
    CFGTraits::getInstrPos(ExitingBlock, BranchInstr);

  if (ExitBlock == ExitLandBlock && SetReg == INVALIDREGNUM) {
    // break_logical
    unsigned NewOpcode =
      (TrueBranch == ExitBlock) ? CFGTraits::getBreakNZeroOpcode(OldOpcode)
                                : CFGTraits::getBreakZeroOpcode(OldOpcode);
    CFGTraits::insertCondBranchBefore(BranchInstrPos, NewOpcode, passRep, DL);
  } else {
    unsigned NewOpcode =
      (TrueBranch == ExitBlock) ? CFGTraits::getBranchNZeroOpcode(OldOpcode)
                                : CFGTraits::getBranchZeroOpcode(OldOpcode);
    CFGTraits::insertCondBranchBefore(BranchInstrPos, NewOpcode, passRep, DL);
    if (ExitBlock != ExitLandBlock) {
      // Splice is insert-before ...
      ExitingBlock->splice(BranchInstrPos,
                           ExitBlock,
                           ExitBlock->begin(),
                           ExitBlock->end());
    }
    if (SetReg != INVALIDREGNUM) {
      CFGTraits::insertAssignInstrBefore(BranchInstrPos, passRep, SetReg, 1);
    }
    CFGTraits::insertInstrBefore(BranchInstrPos, AMDIL::BREAK, passRep);
    CFGTraits::insertInstrBefore(BranchInstrPos, AMDIL::ENDIF, passRep);
  } // if_logical

  DEBUG(
    dbgs() << "Erase instruction\n";
    BranchInstr->dump()
  );

  // Now branchInst can be erased safely
  //ExitingBlock->eraseFromParent(BranchInstr);
  BranchInstr->eraseFromParent();

  // Now take care of successors, retire blocks
  ExitingBlock->removeSuccessor(ExitBlock);
  if (ExitBlock != ExitLandBlock) {
    // Splice is insert-before ...
    ExitBlock->removeSuccessor(ExitLandBlock);
    retireBlock(ExitBlock);
  }
}

template<class PassT>
void CFGStructurizer<PassT>::settleLoopContBlock(BlockT *ContingBlock,
                                                 BlockT *ContBlock,
                                                 RegiT SetReg) {
  DEBUG(dbgs() << "settleLoopContBlock conting = BB"
        << ContingBlock->getNumber()
        << ", cont = BB" << ContBlock->getNumber() << '\n');

  InstrT *BranchInstr = CFGTraits::getLoopEndBlockBranchInstr(ContingBlock);
  if (!BranchInstr) {
    // If we've arrived here then we've already erased the branch instruction.
    // Travel back up the basic block to see the last reference of our debug
    // location we've just inserted that reference here so it should be
    // representative
    if (SetReg != INVALIDREGNUM) {
      CFGTraits::insertAssignInstrBefore(ContingBlock, passRep, SetReg, 1);
      // insertEnd to ensure phi-moves, if exist, go before the continue-instr.
      CFGTraits::insertInstrEnd(ContingBlock, AMDIL::BREAK, passRep,
                                CFGTraits::getLastDebugLocInBB(ContingBlock));
    } else {
      // insertEnd to ensure phi-moves, if exist, go before the continue-instr.
      CFGTraits::insertInstrEnd(ContingBlock, AMDIL::CONTINUE, passRep,
                                CFGTraits::getLastDebugLocInBB(ContingBlock));
    }

    return;
  }

  assert(BranchInstr->isConditionalBranch());
  typename BlockT::iterator BranchInstrPos =
    CFGTraits::getInstrPos(ContingBlock, BranchInstr);
  BlockT *TrueBranch = CFGTraits::getTrueBranch(BranchInstr);
  unsigned OldOpcode = BranchInstr->getOpcode();
  DebugLoc DL = BranchInstr->getDebugLoc();

  // Transform ContingBlock to
  // if () {
  //    move instr after BranchInstr
  //    continue
  //  or
  //    SetReg = 1
  //    break
  // }endif
  // successor = {orgSuccessor(ContingBlock) - LoopHeader}

  bool UseContinueLogical =
    (SetReg == INVALIDREGNUM && (&*ContingBlock->rbegin()) == BranchInstr);

  if (UseContinueLogical) {
    unsigned BranchOpcode =
      TrueBranch == ContBlock ? CFGTraits::getContinueNZeroOpcode(OldOpcode)
                              : CFGTraits::getContinueZeroOpcode(OldOpcode);

    CFGTraits::insertCondBranchBefore(BranchInstrPos, BranchOpcode,
                                      passRep, DL);
    BranchInstr->eraseFromParent();
    return;
  }

  unsigned BranchOpcode =
    TrueBranch == ContBlock ? CFGTraits::getBranchNZeroOpcode(OldOpcode)
                            : CFGTraits::getBranchZeroOpcode(OldOpcode);

  CFGTraits::insertCondBranchBefore(BranchInstrPos, BranchOpcode, passRep,
                                    DL);

  if (SetReg != INVALIDREGNUM) {
    CFGTraits::insertAssignInstrBefore(BranchInstrPos, passRep, SetReg, 1);
    // insertEnd to ensure phi-moves, if exist, go before the continue-instr.
    DEBUG(dbgs() << "Inserting break instruction into block BB"
          << ContingBlock->getNumber() << '\n');

    CFGTraits::insertInstrEnd(ContingBlock, AMDIL::BREAK, passRep, DL);
  } else {
    DEBUG(dbgs() << "Inserting continue instruction into block BB"
          << ContingBlock->getNumber() << '\n');
    assert(!ContingBlock->pred_empty());
    // insertEnd to ensure phi-moves, if exist, go before the continue-instr.
    CFGTraits::insertInstrEnd(ContingBlock, AMDIL::CONTINUE, passRep, DL);
  }

  CFGTraits::insertInstrEnd(ContingBlock, AMDIL::ENDIF, passRep, DL);

  BranchInstr->eraseFromParent();
}

// BBs in ExitBlockSet are determined as in break-path for LoopRep,
// before we can put code for BBs as inside loop-body for LoopRep
// check whether those BBs are determined as cont-BB for ParentLoopRep
// earlier.
// If so, generate a new BB NewBlock
//    (1) set NewBlock common successor of BBs in ExitBlockSet
//    (2) change the continue-instr in BBs in ExitBlockSet to break-instr
//    (3) generate continue-instr in NewBlock
//
template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::relocateLoopContBlock(LoopT *ParentLoopRep,
                                              LoopT *LoopRep,
                                              const SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> &ExitBlockSet,
                                              BlockT *ExitLandBlock) {
  std::set<BlockT *> EndBlockSet;

//  BlockT *parentLoopHead = ParentLoopRep->getHeader();

  for (typename SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>::const_iterator I = ExitBlockSet.begin(),
         E = ExitBlockSet.end(); I != E; ++I) {
    BlockT *ExitBlock = *I;
    BlockT *EndBlock = singlePathEnd(ExitBlock, ExitLandBlock);

    if (!EndBlock || !CFGTraits::getContinueInstr(EndBlock))
      return NULL;

    EndBlockSet.insert(EndBlock);
  }

  BlockT *NewBlock = funcRep->CreateMachineBasicBlock();
  funcRep->push_back(NewBlock); // Insert to function
  CFGTraits::insertInstrEnd(NewBlock, AMDIL::CONTINUE, passRep);

  DEBUG(showNewBlock(NewBlock, "New continue block"));

  for (typename std::set<BlockT*>::const_iterator I = EndBlockSet.begin(),
         E = EndBlockSet.end(); I != E; ++I) {
    BlockT *EndBlock = *I;
    InstrT *ContInstr = CFGTraits::getContinueInstr(EndBlock);
    if (ContInstr) {
      ContInstr->eraseFromParent();
    }
    EndBlock->addSuccessor(NewBlock);

    DEBUG(dbgs() << "Add new continue Block to BB"
          << EndBlock->getNumber() << " successors\n");
  }

  return NewBlock;
}

// LoopEndBranchBlock is a BB created by the CFGStructurizer to use as
// LoopLandBlock. This BB branch on the loop endBranchInit register to
// the paths corresponding to the loop exiting branches.

template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::addLoopEndBranchBlock(LoopT *LoopRep,
                                              ArrayRef<BlockT *> ExitingBlocks,
                                              ArrayRef<BlockT *> ExitBlocks) {
  const TargetInstrInfo *TII = passRep->getTargetInstrInfo();

  RegiT EndBranchReg = getRegister(&AMDIL::GPR_32RegClass);
  assert(EndBranchReg != INVALIDREGNUM);

  // reg = 0 before entering the loop
  addLoopEndBranchInitReg(LoopRep, EndBranchReg);

  uint32_t NumBlocks = static_cast<uint32_t>(ExitingBlocks.size());
  assert(NumBlocks >= 2);
  assert(ExitingBlocks.size() == ExitBlocks.size());

  BlockT *PreExitingBlock = ExitingBlocks[0];
  BlockT *PreExitBlock = ExitBlocks[0];
  BlockT *PreBranchBlock = funcRep->CreateMachineBasicBlock();

  funcRep->push_back(PreBranchBlock); // Insert to function
  DEBUG(showNewBlock(PreBranchBlock, "New loopEndBranch pre exit block"));

  BlockT *NewLandBlock = PreBranchBlock;

  PreExitingBlock->ReplaceUsesOfBlockWith(PreExitBlock, NewLandBlock);

  // It is redundant to add reg = 0 to ExitingBlocks[0]

  // For 1..n th exiting path (the last iteration handles two paths)
  // create the branch to the previous path and the current path.
  for (uint32_t i = 1; i < NumBlocks; ++i) {
    BlockT *CurExitingBlock = ExitingBlocks[i];
    BlockT *CurExitBlock = ExitBlocks[i];
    BlockT *CurBranchBlock;

    if (i == NumBlocks - 1) {
      CurBranchBlock = CurExitBlock;
    } else {
      CurBranchBlock = funcRep->CreateMachineBasicBlock();
      funcRep->push_back(CurBranchBlock); // Insert to function
      DEBUG(showNewBlock(CurBranchBlock, "New loopEndBranch current branch block"));
    }

    // Add reg = i to ExitingBlocks[i].
    CFGTraits::insertAssignInstrBefore(CurExitingBlock, passRep, EndBranchReg, i);

    // Remove the edge (ExitingBlocks[i], ExitBlocks[i]).
    // Add new edge (ExitingBlocks[i], NewLandBlock).
    CurExitingBlock->ReplaceUsesOfBlockWith(CurExitBlock, NewLandBlock);

    // Add to PreBranchBlock the branch instruction:
    // if (EndBranchReg == preVal)
    //    PreExitBlock
    // else
    //    CurBranchBlock
    //
    // preValReg = i - 1

    DebugLoc DL;
    RegiT PreValReg = getRegister(&AMDIL::GPR_32RegClass);
    MachineInstr *PreValInst
      = BuildMI(PreBranchBlock, DL, TII->get(AMDIL::LOADCONSTi32), PreValReg)
      .addImm(i - 1); // preVal
    DEBUG(showNewInstr(PreValInst));

    // CondResReg = (EndBranchReg == PreValReg)
    RegiT CondResReg = getRegister(&AMDIL::GPR_32RegClass);
    MachineInstr *CmpInst
      = BuildMI(PreBranchBlock, DL, TII->get(AMDIL::IEQi32rr), CondResReg)
      .addReg(EndBranchReg).addReg(PreValReg);
    DEBUG(showNewInstr(CmpInst));

    MachineInstr *CondBranchInst
      = BuildMI(PreBranchBlock, DL, TII->get(AMDIL::BRANCHi32br))
      .addMBB(PreExitBlock).addReg(CondResReg);
    DEBUG(showNewInstr(CondBranchInst));

    PreBranchBlock->addSuccessor(PreExitBlock);
    PreBranchBlock->addSuccessor(CurBranchBlock);

    // Update PreExitingBlock, PreExitBlock, PreBranchBlock.
    PreExitingBlock = CurExitingBlock;
    PreExitBlock = CurExitBlock;
    PreBranchBlock = CurBranchBlock;
  }  // End for 1 .. n blocks

  return NewLandBlock;
}

template<class PassT>
typename CFGStructurizer<PassT>::PathToKind
CFGStructurizer<PassT>::singlePathTo(BlockT *SrcBlock, BlockT *DestBlock,
                                     bool AllowSideEntry) {
  assert(DestBlock);

  if (SrcBlock == DestBlock) {
    return SinglePath_InPath;
  }

  while (SrcBlock && SrcBlock->succ_size() == 1) {
    SrcBlock = *SrcBlock->succ_begin();
    if (SrcBlock == DestBlock) {
      return SinglePath_InPath;
    }

    if (!AllowSideEntry && SrcBlock->pred_size() > 1) {
      return Not_SinglePath;
    }
  }

  if (SrcBlock && SrcBlock->succ_empty()) {
    return SinglePath_NotInPath;
  }

  return Not_SinglePath;
}

// If there is a single path from SrcBlock to DestBlock, return the last
// block before DestBlock. If there is a single path from SrcBlock->end
// without DestBlock, return the last block in the path. Otherwise,
// return NULL
template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::singlePathEnd(BlockT *SrcBlock, BlockT *DestBlock,
                                      bool AllowSideEntry) {
  //assert(DestBlock);

  if (SrcBlock == DestBlock) {
    return SrcBlock;
  }

  if (SrcBlock->succ_empty()) {
    return SrcBlock;
  }

  while (SrcBlock && SrcBlock->succ_size() == 1) {
    BlockT *PreBlock = SrcBlock;

    SrcBlock = *SrcBlock->succ_begin();
    if (!SrcBlock) {
      return PreBlock;
    }

    if (!AllowSideEntry && SrcBlock->pred_size() > 1) {
      return NULL;
    }
  }

  if (SrcBlock && SrcBlock->succ_empty()) {
    return SrcBlock;
  }

  return NULL;
}

template<class PassT>
int CFGStructurizer<PassT>::cloneOnSideEntryTo(BlockT *PreBlock,
                                               BlockT *SrcBlock,
                                               BlockT *DestBlock) {
  int Cloned = 0;
  assert(PreBlock->isSuccessor(SrcBlock));
  while (SrcBlock && SrcBlock != DestBlock) {
    assert(SrcBlock->succ_size() == 1 ||
      (SrcBlock->succ_empty() && !DestBlock));
    if (SrcBlock->pred_size() > 1) {
      SrcBlock = cloneBlockForPredecessor(SrcBlock, PreBlock);
      ++Cloned;
    }

    PreBlock = SrcBlock;
    if (SrcBlock->succ_size() == 1)
      SrcBlock = *SrcBlock->succ_begin();
    else
      SrcBlock = NULL;
  }

  return Cloned;
}

template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::cloneBlockForPredecessor(BlockT *CurBlock,
                                                 BlockT *PredBlock) {
  assert(PredBlock->isSuccessor(CurBlock) &&
         "succBlk is not a predecessor of CurBlock");

  BlockT *CloneBlock = CFGTraits::clone(CurBlock); // Clone instructions
  PredBlock->ReplaceUsesOfBlockWith(CurBlock, CloneBlock);

  // Add all successors to CloneBlock
  CFGTraits::cloneSuccessorList(CloneBlock, CurBlock);

  numClonedInstr += CurBlock->size();

  DEBUG(
    dbgs() << "Cloned block: " << "BB" << CurBlock->getNumber()
    << " size " << CurBlock->size() << '\n';
    showNewBlock(CloneBlock, "Result of cloned block");
    dbgs() << "CFG after clone:\n";
    //funcRep->viewCFGOnly();
  );

  return CloneBlock;
}

template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::exitingBlockToExitBlock(LoopT *LoopRep,
                                                BlockT *ExitingBlock) {
  BlockT *ExitBlock = NULL;

  for (typename BlockT::succ_iterator Succ = ExitingBlock->succ_begin(),
         End = ExitingBlock->succ_end(); Succ != End; ++Succ) {
    BlockT *CurBlock = *Succ;
    if (!LoopRep->contains(CurBlock)) {
      assert(ExitBlock == NULL);
      ExitBlock = CurBlock;
    }
  }

  assert(ExitBlock != NULL);

  return ExitBlock;
}

template<class PassT>
void CFGStructurizer<PassT>::migrateInstruction(BlockT *SrcBlock,
                                                BlockT *DestBlock,
                                                InstrIterator InsertPos) {
  InstrIterator SpliceEnd;
  // Look for the input BranchInstr, not the AMDIL BranchInstr
  InstrT *BranchInstr = CFGTraits::getNormalBlockBranchInstr(SrcBlock);
  if (BranchInstr) {
    DEBUG(dbgs() << "migrateInstruction see branch instr\n");
    DEBUG(BranchInstr->dump());

    SpliceEnd = CFGTraits::getInstrPos(SrcBlock, BranchInstr);
  } else {
    DEBUG(dbgs() << "migrateInstruction don't see branch instr\n");
    SpliceEnd = SrcBlock->end();
  }

  DEBUG(dbgs() << "migrateInstruction before splice dstSize = " << DestBlock->size()
        << "srcSize = " << SrcBlock->size() << '\n');

  // Splice insert before InsertPos
  DestBlock->splice(InsertPos, SrcBlock, SrcBlock->begin(), SpliceEnd);

  DEBUG(dbgs() << "migrateInstruction after splice dstSize = " << DestBlock->size()
        << " srcSize = " << SrcBlock->size() << '\n');
}

// normalizeInfiniteLoopExit change
//   B1:
//        uncond_br LoopHeader
//
// to
//   B1:
//        cond_br 1 LoopHeader dummyExit
// and return the newly added dummy exit block
//
template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::normalizeInfiniteLoopExit(LoopT *LoopRep) {
  BlockT *LoopHeader = LoopRep->getHeader();
  BlockT *LoopLatch = LoopRep->getLoopLatch();

  DEBUG(dbgs() << "normalizeInfiniteLoopExit LoopRep = " << LoopRep << '\n');

  if (!LoopHeader || !LoopLatch)
    return NULL;

  InstrT *BranchInstr = CFGTraits::getLoopEndBlockBranchInstr(LoopLatch);
  if (!BranchInstr || !BranchInstr->isUnconditionalBranch()) {
    return NULL;
  }

  // If the block has multiple successors, it also has conditional branches.
  // This catches the case
  // brcond BB #N
  // br BB #M
  if (LoopLatch->succ_size() > 1)
    return NULL;

  BlockT *DummyExitBlock = funcRep->CreateMachineBasicBlock();
  funcRep->push_back(DummyExitBlock);  // Insert to function.
  DEBUG(
    showNewBlock(DummyExitBlock, "DummyExitBlock to normalize infiniteLoop");
    dbgs() << "Old branch instr: " << *BranchInstr << '\n'
  );

  DEBUG(dbgs() << "Old branch instr: " << *BranchInstr << '\n');

  typename BlockT::iterator InsertPos =
    CFGTraits::getInstrPos(LoopLatch, BranchInstr);
  unsigned ImmReg = getRegister(&AMDIL::GPR_32RegClass);
  CFGTraits::insertAssignInstrBefore(InsertPos, passRep, ImmReg, 1);
  InstrT *NewInstr =
    CFGTraits::insertInstrBefore(InsertPos, AMDIL::BRANCHi32br,
                                 passRep);
  MachineInstrBuilder(NewInstr)
    .addMBB(LoopHeader)
    .addReg(ImmReg, false);

  DEBUG(showNewInstr(NewInstr));
  BranchInstr->eraseFromParent();
  LoopLatch->addSuccessor(DummyExitBlock);

  return DummyExitBlock;
}

template<class PassT>
bool CFGStructurizer<PassT>::removeUnconditionalBranch(BlockT *SrcBlock) {
  bool changed = false;

  InstrT *BranchInstr;

  // I saw two unconditional branch in one basic block in example
  // test_fc_do_while_or.c need to fix the upstream on this to remove
  // the loop.
  while ((BranchInstr = CFGTraits::getLoopEndBlockBranchInstr(SrcBlock))
          && BranchInstr->isUnconditionalBranch()) {
    DEBUG(dbgs() << "Removing unconditional branch instruction");
    DEBUG(BranchInstr->dump());

    BranchInstr->eraseFromParent();
    changed = true;
  }

  return changed;
}

template<class PassT>
bool CFGStructurizer<PassT>::removeRedundantConditionalBranch(BlockT *SrcBlock) {
  if (SrcBlock->succ_size() != 2) {
    return false;
  }

  assert(!SrcBlock->empty() && "Block with 2 successors can't be empty");

  BlockT *Block1 = *SrcBlock->succ_begin();
  BlockT *Block2 = *(SrcBlock->succ_begin() + 1);
  if (Block1 != Block2) {
    return false;
  }

  InstrT *BranchInstr = CFGTraits::getNormalBlockBranchInstr(SrcBlock);
  assert(BranchInstr && BranchInstr->isConditionalBranch());

  DEBUG(dbgs() << "Removing unneeded conditional branch instruction");
  DEBUG(BranchInstr->dump());

  BranchInstr->eraseFromParent();
  DEBUG(showNewBlock(Block1, "Removing redundant successor"));
  SrcBlock->removeSuccessor(Block1);

  return true;
}


template<class PassT>
bool CFGStructurizer<PassT>::removeCodeAfterUnconditionalBranch(BlockT *SrcBlock) {
  // Find first unconditional branch

  // If the first branch we encounter is an unconditional branch,
  // remove all successors except the one.

  MachineBasicBlock::iterator It = SrcBlock->getFirstTerminator();
  if (It == SrcBlock->end()) {
    return false;
  }

  MachineInstr* BranchInstr = &*It;
  if (!BranchInstr->isUnconditionalBranch()) {
    return false;
  }

  BlockT* Replacement = CFGTraits::getTrueBranch(BranchInstr);

  MachineBasicBlock::iterator DeadStart = It++;

  while (It != SrcBlock->end()) {
    MachineInstr* Instr = &*It;

    // Unlink any branches we encounter
    if (Instr->isUnconditionalBranch()) {
      BlockT *Dest = CFGTraits::getTrueBranch(Instr);

      SrcBlock->removeSuccessor(Dest);
      if (Dest->pred_empty()) {
        deepPurgeSuccessors(Dest);
      }

      It = DeadStart;
    } else if (Instr->isConditionalBranch()) {
      BlockT *T = CFGTraits::getTrueBranch(Instr);
      BlockT *F = CFGTraits::getFalseBranch(SrcBlock, Instr);

      SrcBlock->removeSuccessor(T);
      if (T->pred_empty()) {
        deepPurgeSuccessors(T);
      }

      SrcBlock->removeSuccessor(F);
      if (F->pred_empty()) {
        deepPurgeSuccessors(F);
      }

      It = DeadStart;
    }

    // Remove the dead instructions
    Instr->eraseFromParent();

    ++It;
  }

  DEBUG(dbgs() << "Remove potential fallthrough blocks\n");

  // Take care of removing a potentially dead fall through block.
  typename BlockT::succ_iterator Succ = SrcBlock->succ_begin();
  while (Succ != SrcBlock->succ_end()) {
    BlockT *MBB = *Succ;

    if (MBB->getNumber() != Replacement->getNumber()) {
      DEBUG(dbgs() << "Remove successor BB" << MBB->getNumber() << '\n');
      Succ = SrcBlock->removeSuccessor(Succ);
      if (MBB->pred_empty()) {
        deepPurgeSuccessors(MBB);
      }
    } else {
      DEBUG(dbgs() << "Skip successor BB" << MBB->getNumber() << '\n');
      ++Succ;
    }
  }

  // Fix cases where we end up with an unconditional branch into the
  // same block as the fallthrough
  SrcBlock->CorrectExtraCFGEdges(Replacement, NULL, false);

  // Turn this into a fallthrough
  BranchInstr->eraseFromParent();

  return true;
}

template<class PassT>
void CFGStructurizer<PassT>::addDummyExitBlock(
  SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> &RetBlocks) {
  BlockT *DummyExitBlock = funcRep->CreateMachineBasicBlock();
  funcRep->push_back(DummyExitBlock);  // Insert to function
  CFGTraits::insertInstrEnd(DummyExitBlock, AMDIL::RETURN, passRep);

  for (typename SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>::iterator
         I = RetBlocks.begin(), E = RetBlocks.end(); I != E; ++I) {
    BlockT *CurBlock = *I;
    InstrT *CurInstr = CFGTraits::getReturnInstr(CurBlock);
    if (CurInstr) {
      CurInstr->eraseFromParent();
    }

    CurBlock->addSuccessor(DummyExitBlock);

    DEBUG(dbgs() << "Add dummyExitBlock to BB" << CurBlock->getNumber()
          << " successors\n");
  }

  DEBUG(showNewBlock(DummyExitBlock, "DummyExitBlock"));
}

template<class PassT>
void CFGStructurizer<PassT>::deepPurgeSuccessors(BlockT *SrcBlock) {
  llvm::SmallVector<BlockT*, 8> Stack;

  Stack.push_back(SrcBlock);

  // TODO: Could this leave stray loops that need to be removed from loopInfo?

  while (!Stack.empty()) {
    BlockT *Block = Stack.pop_back_val();

    if (Block->pred_empty()) {
      typename BlockT::succ_iterator Succ = Block->succ_begin();
      while (Succ != Block->succ_end()) {
        Stack.push_back(*Succ);
        Succ = Block->removeSuccessor(Succ);
      }

      DEBUG(dbgs() << "Remove dead block BB" << Block->getNumber() << '\n');
      //postDomTree->eraseNode(Block);
      Block->eraseFromParent();
    }
  }
}

template<class PassT>
void CFGStructurizer<PassT>::replacePostDomTreeRoot() {
  DEBUG(dbgs() << "Trying to retire the root of the postdom tree\n");

  MachineDomTreeNode *RootNode = postDomTree->getRootNode();

  BlockT *CommonIDom = NULL;
  for (MachineDomTreeNode::iterator I = RootNode->begin(),
         E = RootNode->end(); I != E; ++I) {
    MachineDomTreeNode *ChildNode = *I;
    BlockT *Child = ChildNode->getBlock();
    if (!CommonIDom) {
      CommonIDom = Child;
    } else {
      CommonIDom = postDomTree->findNearestCommonDominator(CommonIDom, Child);
    }
  }

  assert(CommonIDom);

  for (MachineDomTreeNode::iterator I = RootNode->begin(),
         E = RootNode->end(); I != E; ++I) {
    MachineDomTreeNode *ChildNode = *I;
    BlockT *Child = ChildNode->getBlock();

    postDomTree->changeImmediateDominator(Child, CommonIDom);
  }
}

template<class PassT>
  void CFGStructurizer<PassT>::retireBlock(BlockT *Block) {
  DEBUG(dbgs() << "Retiring BB" << Block->getNumber() << '\n');
  assert(Block->succ_empty() && Block->pred_empty()
         && "Can't retire block yet");

  retiredBlocks.insert(Block);
  loopInfo->removeBlock(Block);
}

template<class PassT>
bool CFGStructurizer<PassT>::isRetiredBlock(BlockT *Block) const {
  return (retiredBlocks.find(Block) != retiredBlocks.end());
}

template<class PassT>
bool CFGStructurizer<PassT>::isActiveLoopHead(const BlockT *CurBlock) const {
  LoopT *LoopRep = loopInfo->getLoopFor(CurBlock);
  while (LoopRep && LoopRep->getHeader() == CurBlock) {
    LoopLandInfo *LoopLand = getLoopLandInfo(LoopRep);
    if (!LoopLand)
      return true;

    BlockT *LandBlock = LoopLand->landBlk;
    assert(LandBlock);
    if (!isRetiredBlock(LandBlock)) {
      return true;
    }

    LoopRep = LoopRep->getParentLoop();
  }

  return false;
}

template<class PassT>
bool CFGStructurizer<PassT>::needMigrateBlock(const BlockT *Block) const {
  const unsigned BlockSizeThreshold = 30;
  const unsigned CloneInstrThreshold = 100;

  bool MultiplePreds = Block && (Block->pred_size() > 1);

  if (!MultiplePreds)
    return false;

  unsigned BlockSize = Block->size();
  return ((BlockSize > BlockSizeThreshold)
          && (BlockSize * (Block->pred_size() - 1) > CloneInstrThreshold));
}

template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::recordLoopLandBlock(LoopT *LoopRep,
                                            BlockT *LandBlock,
                                            const SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS> &ExitBlockSet) {
  SmallVector<BlockT *, DEFAULT_VEC_SLOTS> InPathBlocks; // In exit path blocks

  for (typename BlockT::pred_iterator PI = LandBlock->pred_begin(),
         PE = LandBlock->pred_end(); PI != PE; ++PI) {
    BlockT *CurBlock = *PI;
    if (CurBlock == LandBlock)
        continue; // In case LandBlock is a single-block loop

    if (LoopRep->contains(CurBlock) || ExitBlockSet.count(CurBlock)) {
      InPathBlocks.push_back(CurBlock);
    }
  }

  if (InPathBlocks.size() == LandBlock->pred_size()) {
    setLoopLandBlock(LoopRep, LandBlock);
    return LandBlock;
  }

  // If LandBlock has predecessors that are not in the given loop,
  // create a new block

  BlockT *NewLandBlock = funcRep->CreateMachineBasicBlock();
  funcRep->push_back(NewLandBlock);  // Insert to function
  NewLandBlock->addSuccessor(LandBlock);
  for (typename ArrayRef<BlockT *>::iterator I = InPathBlocks.begin(),
         E = InPathBlocks.end(); I != E; ++I) {
    BlockT *CurBlock = *I;
    CurBlock->ReplaceUsesOfBlockWith(LandBlock, NewLandBlock);
  }

  DEBUG(showNewBlock(NewLandBlock, "NewLandingBlock"));
  setLoopLandBlock(LoopRep, NewLandBlock);

  return NewLandBlock;
}

template<class PassT>
typename CFGStructurizer<PassT>::BlockT*
CFGStructurizer<PassT>::insertLoopDummyLandingBlock(const LoopT *LoopRep) {
  BlockT *Block = funcRep->CreateMachineBasicBlock();
  funcRep->push_back(Block); // Insert to function
  DEBUG(showNewBlock(Block, "DummyLandingBlock for loop without break: "));
  return Block;
}

template<class PassT>
void CFGStructurizer<PassT>::setLoopLandBlock(const LoopT *LoopRep, BlockT *Block) {
  assert(Block);

  LoopLandInfo *&Entry = loopLandInfoMap[LoopRep];
  if (!Entry) {
    Entry = new LoopLandInfo();
  }
  assert(Entry->landBlk == NULL);

  Entry->landBlk = Block;

  DEBUG(dbgs() << "setLoopLandBlock loop-header = BB"
        << LoopRep->getHeader()->getNumber()
        << "  landing-block = BB" << Block->getNumber() << '\n');
}

template<class PassT>
void CFGStructurizer<PassT>::addLoopBreakOnReg(LoopT *LoopRep, RegiT RegNum) {
  LoopLandInfo *&Entry = loopLandInfoMap[LoopRep];

  if (!Entry) {
    Entry = new LoopLandInfo();
  }

  Entry->breakOnRegs.insert(RegNum);

  DEBUG(dbgs() << "addLoopBreakOnReg loop-header = BB"
        << LoopRep->getHeader()->getNumber()
        << " RegNum = " << RegNum << '\n');
}

template<class PassT>
void CFGStructurizer<PassT>::addLoopContOnReg(LoopT *LoopRep, RegiT RegNum) {
  LoopLandInfo *&Entry = loopLandInfoMap[LoopRep];

  if (!Entry) {
    Entry = new LoopLandInfo();
  }
  Entry->contOnRegs.insert(RegNum);

  DEBUG(dbgs() << "addLoopContOnReg loop-header = BB"
        << LoopRep->getHeader()->getNumber()
        << " RegNum = " << RegNum << '\n');
}

template<class PassT>
void CFGStructurizer<PassT>::addLoopBreakInitReg(LoopT *LoopRep, RegiT RegNum) {
  LoopLandInfo *&Entry = loopLandInfoMap[LoopRep];

  if (!Entry) {
    Entry = new LoopLandInfo();
  }

  Entry->breakInitRegs.insert(RegNum);

  DEBUG(dbgs() << "addLoopBreakInitReg loop-header = BB"
        << LoopRep->getHeader()->getNumber()
        << " RegNum = " << RegNum << '\n');
}

template<class PassT>
void CFGStructurizer<PassT>::addLoopContInitReg(LoopT *LoopRep, RegiT RegNum) {
  LoopLandInfo *&Entry = loopLandInfoMap[LoopRep];

  if (!Entry) {
    Entry = new LoopLandInfo();
  }

  Entry->contInitRegs.insert(RegNum);

  DEBUG(dbgs() << "addLoopContInitReg loop-header = "
        << "BB" << LoopRep->getHeader()->getNumber()
        << " RegNum = " << RegNum << '\n');
}

template<class PassT>
void CFGStructurizer<PassT>::addLoopEndBranchInitReg(LoopT *LoopRep,
                                                     RegiT RegNum) {
  LoopLandInfo *&Entry = loopLandInfoMap[LoopRep];

  if (!Entry) {
    Entry = new LoopLandInfo();
  }
  Entry->endBranchInitRegs.insert(RegNum);

  DEBUG(dbgs() << "addLoopEndBranchInitReg loop-header = BB"
        << LoopRep->getHeader()->getNumber()
        << " RegNum = " << RegNum << '\n');
}

template<class PassT>
typename CFGStructurizer<PassT>::LoopLandInfo *
CFGStructurizer<PassT>::getLoopLandInfo(const LoopT *LoopRep) const {
  typename LoopLandInfoMap::const_iterator It = loopLandInfoMap.find(LoopRep);
  assert(It != loopLandInfoMap.end());
  return (It == loopLandInfoMap.end()) ? NULL : It->second;
}

template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::getLoopLandBlock(const LoopT *LoopRep) const {
  typename LoopLandInfoMap::const_iterator It = loopLandInfoMap.find(LoopRep);
  return (It == loopLandInfoMap.end()) ? NULL : It->second->landBlk;
}

template<class PassT>
bool CFGStructurizer<PassT>::hasBackEdge(const BlockT *Block) const {
  const LoopT *LoopRep = loopInfo->getLoopFor(Block);
  if (!LoopRep)
    return false;

  const BlockT *LoopHeader = LoopRep->getHeader();

  return Block->isSuccessor(LoopHeader);
}

template<class PassT>
unsigned CFGStructurizer<PassT>::getLoopDepth(const LoopT *LoopRep) const {
  return LoopRep ? LoopRep->getLoopDepth() : 0;
}

template<class PassT>
unsigned CFGStructurizer<PassT>::countActiveBlocks(MutableArrayRef<BlockT *> Blocks) {
  unsigned Count = 0;
  for (typename MutableArrayRef<BlockT *>::iterator I = Blocks.begin(),
         E = Blocks.end(); I != E; ++I) {
    BlockT *Block = *I;
    if (!isRetiredBlock(Block)) {
      ++Count;
    }
  }

  return Count;
}

template<class PassT>
unsigned CFGStructurizer<PassT>::countActiveBlocks() {
  unsigned Count = 0;
  for (typename FuncT::iterator I = funcRep->begin(),
         E = funcRep->end(); I != E; ++I) {
    BlockT &Block = *I;
    if (!isRetiredBlock(&Block)) {
      ++Count;
    }
  }

  return Count;
}

template<class PassT>
typename CFGStructurizer<PassT>::BlockT *
CFGStructurizer<PassT>::findNearestCommonPostDom
    (const SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>& Blocks) {
  BlockT *CommonDom;
  typename SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>::const_iterator I = Blocks.begin();
  typename SmallPtrSet<BlockT *, DEFAULT_VEC_SLOTS>::const_iterator E = Blocks.end();
  for (CommonDom = *I; I != E && CommonDom; ++I) {
    BlockT *CurBlock = *I;
    if (CurBlock != CommonDom) {
      CommonDom = postDomTree->findNearestCommonDominator(CurBlock, CommonDom);
    }
  }

  DEBUG(
    dbgs() << "Common post dominator for exit blocks is ";
    if (CommonDom) {
      dbgs() << "BB" << CommonDom->getNumber() << '\n';
    } else {
      dbgs() << "NULL\n";
    }
  );

  return CommonDom;
}

} //end namespace llvm

//todo: move-end


//===----------------------------------------------------------------------===//
//
// CFGStructurizer for AMDIL
//
//===----------------------------------------------------------------------===//


using namespace llvmCFGStruct;

namespace llvm
{
class AMDILCFGStructurizer : public MachineFunctionPass {
public:
  typedef MachineInstr              InstructionType;
  typedef MachineFunction           FunctionType;
  typedef MachineBasicBlock         BlockType;
  typedef MachineLoopInfo           LoopInfoType;
  typedef MachineDominatorTree      DominatorTreeType;
  typedef MachinePostDominatorTree  PostDominatorTreeType;
  typedef MachineDomTreeNode        DomTreeNodeType;
  typedef MachineLoop               LoopType;
//private:
  const TargetInstrInfo *TII;

//public:
//  static char ID;

public:
  AMDILCFGStructurizer(char &pid);
  const TargetInstrInfo *getTargetInstrInfo() const;
  // this is abstract base class
  virtual bool runOnMachineFunction(MachineFunction &F) = 0;

private:

};   //end of class AMDILCFGStructurizer

//char AMDILCFGStructurizer::ID = 0;
} //end of namespace llvm
AMDILCFGStructurizer::AMDILCFGStructurizer(char &PID)
  : MachineFunctionPass(PID), TII(NULL) {
}

const TargetInstrInfo *AMDILCFGStructurizer::getTargetInstrInfo() const {
  return TII;
}
//===----------------------------------------------------------------------===//
//
// CFGPrepare
//
//===----------------------------------------------------------------------===//

namespace llvm
{
  extern void initializeAMDILCFGPreparePass(llvm::PassRegistry&);
}

using namespace llvmCFGStruct;

namespace llvm
{
class AMDILCFGPrepare : public AMDILCFGStructurizer {
public:
  static char ID;

public:
  AMDILCFGPrepare();

  virtual const char *getPassName() const;
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  bool runOnMachineFunction(MachineFunction &F);

private:

};   //end of class AMDILCFGPrepare

char AMDILCFGPrepare::ID = 0;
} //end of namespace llvm

AMDILCFGPrepare::AMDILCFGPrepare()
  : AMDILCFGStructurizer(ID) {
  initializeAMDILCFGPreparePass(*PassRegistry::getPassRegistry());
}

const char *AMDILCFGPrepare::getPassName() const {
  return "AMD IL Control Flow Graph Preparation Pass";
}

void AMDILCFGPrepare::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addPreserved<MachineFunctionAnalysis>();
  AU.addRequired<MachineFunctionAnalysis>();
  AU.addRequired<MachineDominatorTree>();
  AU.addRequired<MachinePostDominatorTree>();
  AU.addRequired<MachineLoopInfo>();
}

//===----------------------------------------------------------------------===//
//
// CFGPerform
//
//===----------------------------------------------------------------------===//

namespace llvm
{
  extern void initializeAMDILCFGPerformPass(llvm::PassRegistry&);
}

using namespace llvmCFGStruct;

namespace llvm
{
class AMDILCFGPerform : public AMDILCFGStructurizer {
public:
  static char ID;

public:
  AMDILCFGPerform();
  virtual const char *getPassName() const;
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  bool runOnMachineFunction(MachineFunction &F);
};

char AMDILCFGPerform::ID = 0;
} //end of namespace llvm

AMDILCFGPerform::AMDILCFGPerform()
  : AMDILCFGStructurizer(ID) {
  initializeAMDILCFGPerformPass(*PassRegistry::getPassRegistry());
}

const char *AMDILCFGPerform::getPassName() const {
  return "AMD IL Control Flow Graph structurizer Pass";
}

void AMDILCFGPerform::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addPreserved<MachineFunctionAnalysis>();
  AU.addRequired<MachineFunctionAnalysis>();
  AU.addRequired<MachineDominatorTree>();
  AU.addRequired<MachinePostDominatorTree>();
  AU.addRequired<MachineLoopInfo>();
}

//===----------------------------------------------------------------------===//
//
// CFGStructTraits<AMDILCFGStructurizer>
//
//===----------------------------------------------------------------------===//

namespace llvmCFGStruct
{
// this class is tailor to the AMDIL backend
template<>
struct CFGStructTraits<AMDILCFGStructurizer> {
  typedef int RegiT;

  static unsigned getBreakNZeroOpcode(unsigned OldOpcode) {
    switch (OldOpcode) {
      case AMDIL::BRANCHf64bi:
      case AMDIL::BRANCHf64br: return AMDIL::BREAK_LOGICALNZf64r;
      case AMDIL::BRANCHf32bi:
      case AMDIL::BRANCHf32br: return AMDIL::BREAK_LOGICALNZf32r;
      case AMDIL::BRANCHi64bi:
      case AMDIL::BRANCHi64br: return AMDIL::BREAK_LOGICALNZi64r;
      case AMDIL::BRANCHi32bi:
      case AMDIL::BRANCHi32br: return AMDIL::BREAK_LOGICALNZi32r;
      case AMDIL::BRANCHi16bi:
      case AMDIL::BRANCHi16br: return AMDIL::BREAK_LOGICALNZi16r;
      case AMDIL::BRANCHi8bi:
      case AMDIL::BRANCHi8br:  return AMDIL::BREAK_LOGICALNZi8r;
    default:
      llvm_unreachable("internal error");
    }
    return 0;
  }

  static unsigned getBreakZeroOpcode(unsigned OldOpcode) {
    switch (OldOpcode) {
      case AMDIL::BRANCHf64bi:
      case AMDIL::BRANCHf64br: return AMDIL::BREAK_LOGICALZf64r;
      case AMDIL::BRANCHf32bi:
      case AMDIL::BRANCHf32br: return AMDIL::BREAK_LOGICALZf32r;
      case AMDIL::BRANCHi64bi:
      case AMDIL::BRANCHi64br: return AMDIL::BREAK_LOGICALZi64r;
      case AMDIL::BRANCHi32bi:
      case AMDIL::BRANCHi32br: return AMDIL::BREAK_LOGICALZi32r;
      case AMDIL::BRANCHi16bi:
      case AMDIL::BRANCHi16br: return AMDIL::BREAK_LOGICALZi16r;
      case AMDIL::BRANCHi8bi:
      case AMDIL::BRANCHi8br:  return AMDIL::BREAK_LOGICALZi8r;
    default:
      llvm_unreachable("internal error");
    }
    return 0;
  }

  static unsigned getBranchNZeroOpcode(unsigned OldOpcode) {
    switch (OldOpcode) {
      case AMDIL::BRANCHf64bi:
      case AMDIL::BRANCHf64br: return AMDIL::IF_LOGICALNZf64r;
      case AMDIL::BRANCHf32bi:
      case AMDIL::BRANCHf32br: return AMDIL::IF_LOGICALNZf32r;
      case AMDIL::BRANCHi64bi:
      case AMDIL::BRANCHi64br: return AMDIL::IF_LOGICALNZi64r;
      case AMDIL::BRANCHi32bi:
      case AMDIL::BRANCHi32br: return AMDIL::IF_LOGICALNZi32r;
      case AMDIL::BRANCHi16bi:
      case AMDIL::BRANCHi16br: return AMDIL::IF_LOGICALNZi16r;
      case AMDIL::BRANCHi8bi:
      case AMDIL::BRANCHi8br:  return AMDIL::IF_LOGICALNZi8r;
    default:
      llvm_unreachable("internal error");
    }
    return 0;
  }

  static unsigned getBranchZeroOpcode(unsigned OldOpcode) {
    switch (OldOpcode) {
      case AMDIL::BRANCHf64bi:
      case AMDIL::BRANCHf64br: return AMDIL::IF_LOGICALZf64r;
      case AMDIL::BRANCHf32bi:
      case AMDIL::BRANCHf32br: return AMDIL::IF_LOGICALZf32r;
      case AMDIL::BRANCHi64bi:
      case AMDIL::BRANCHi64br: return AMDIL::IF_LOGICALZi64r;
      case AMDIL::BRANCHi32bi:
      case AMDIL::BRANCHi32br: return AMDIL::IF_LOGICALZi32r;
      case AMDIL::BRANCHi16br:
      case AMDIL::BRANCHi16bi: return AMDIL::IF_LOGICALZi16r;
      case AMDIL::BRANCHi8bi:
      case AMDIL::BRANCHi8br:  return AMDIL::IF_LOGICALZi8r;
    default:
      llvm_unreachable("internal error");
    }
    return 0;
  }

  static unsigned getContinueNZeroOpcode(unsigned OldOpcode) {
    switch (OldOpcode) {
      case AMDIL::BRANCHf64bi:
      case AMDIL::BRANCHf64br: return AMDIL::CONTINUE_LOGICALNZf64r;
      case AMDIL::BRANCHf32bi:
      case AMDIL::BRANCHf32br: return AMDIL::CONTINUE_LOGICALNZf32r;
      case AMDIL::BRANCHi64bi:
      case AMDIL::BRANCHi64br: return AMDIL::CONTINUE_LOGICALNZi64r;
      case AMDIL::BRANCHi32bi:
      case AMDIL::BRANCHi32br: return AMDIL::CONTINUE_LOGICALNZi32r;
      case AMDIL::BRANCHi16bi:
      case AMDIL::BRANCHi16br: return AMDIL::CONTINUE_LOGICALNZi16r;
      case AMDIL::BRANCHi8bi:
      case AMDIL::BRANCHi8br:  return AMDIL::CONTINUE_LOGICALNZi8r;
      default:
        llvm_unreachable("internal error");
    }
    return 0;
  }

  static unsigned getContinueZeroOpcode(unsigned OldOpcode) {
    switch (OldOpcode) {
      case AMDIL::BRANCHf64bi:
      case AMDIL::BRANCHf64br: return AMDIL::CONTINUE_LOGICALZf64r;
      case AMDIL::BRANCHf32bi:
      case AMDIL::BRANCHf32br: return AMDIL::CONTINUE_LOGICALZf32r;
      case AMDIL::BRANCHi64bi:
      case AMDIL::BRANCHi64br: return AMDIL::CONTINUE_LOGICALZi64r;
      case AMDIL::BRANCHi32br:
      case AMDIL::BRANCHi32bi: return AMDIL::CONTINUE_LOGICALZi32r;
      case AMDIL::BRANCHi16br:
      case AMDIL::BRANCHi16bi: return AMDIL::CONTINUE_LOGICALZi16r;
      case AMDIL::BRANCHi8bi:
      case AMDIL::BRANCHi8br:  return AMDIL::CONTINUE_LOGICALZi8r;
    default:
      llvm_unreachable("internal error");
    }
    return 0;
  }

  static MachineBasicBlock *getTrueBranch(MachineInstr *Instr) {
    return Instr->getOperand(0).getMBB();
  }

  static void setTrueBranch(MachineInstr *Instr, MachineBasicBlock *Block) {
    Instr->getOperand(0).setMBB(Block);
  }

  static MachineBasicBlock *
  getFalseBranch(MachineBasicBlock *Block, MachineInstr *Instr) {
    assert(Block->succ_size() == 2);
    MachineBasicBlock *TrueBranch = getTrueBranch(Instr);
    MachineBasicBlock::succ_iterator I = Block->succ_begin();
    MachineBasicBlock::succ_iterator Next = I;
    ++Next;

    return (*I == TrueBranch) ? *Next : *I;
  }

  static DebugLoc getLastDebugLocInBB(MachineBasicBlock *Block) {
    // Get DebugLoc from the first MachineBasicBlock instruction with debug info
    DebugLoc DL;
	for (MachineBasicBlock::iterator I = Block->begin(),
           E = Block->end(); I != E; ++I) {
	  MachineInstr *Instr = &*I;
	  if (!Instr->getDebugLoc().isUnknown()) {
	    DL = Instr->getDebugLoc();
	  }
    }
    return DL;
  }

  static MachineInstr *getNormalBlockBranchInstr(MachineBasicBlock *Block) {
    assert(!Block->empty() && "Should not be an empty block");

    MachineBasicBlock::reverse_iterator I = Block->rbegin();
    MachineInstr *Instr = &*I;
    if (Instr && (Instr->isConditionalBranch() || Instr->isUnconditionalBranch())) {
      return Instr;
    }
    return NULL;
  }

  // The correct naming for this is getPossibleLoopEndBlockBranchInstr.
  //
  // BB with backward-edge could have move instructions after the branch
  // instruction.  Such move instruction "belong to" the loop backward-edge.
  //
  static MachineInstr *getLoopEndBlockBranchInstr(MachineBasicBlock *Block) {
    for (MachineBasicBlock::reverse_iterator I = Block->rbegin(),
           E = Block->rend(); I != E; ++I) {
      // FIXME: Simplify
      MachineInstr *Instr = &*I;
      if (!Instr)
        continue;

      if (Instr->isConditionalBranch() || Instr->isUnconditionalBranch()) {
        return Instr;
      }

      if (Instr->getOpcode() == TargetOpcode::COPY) {
        break;
      }
    }
    return NULL;
  }

  static MachineInstr *getReturnInstr(MachineBasicBlock *Block) {
    MachineBasicBlock::reverse_iterator I = Block->rbegin();
    if (I != Block->rend()) {
      MachineInstr *Instr = &*I;
      if (Instr->getOpcode() == AMDIL::RETURN) {
        return Instr;
      }
    }
    return NULL;
  }

  static MachineInstr *getContinueInstr(MachineBasicBlock *Block) {
    MachineBasicBlock::reverse_iterator I = Block->rbegin();
    if (I != Block->rend()) {
      MachineInstr *Instr = &*I;
      if (Instr->getOpcode() == AMDIL::CONTINUE) {
        return Instr;
      }
    }
    return NULL;
  }

  static MachineInstr *getLoopBreakInstr(MachineBasicBlock *Block) {
    for (MachineBasicBlock::iterator I = Block->begin(), E = Block->end();
         I != E; ++I) {
      MachineInstr *Instr = &*I;
      if ((Instr->getOpcode() == AMDIL::BREAK_LOGICALNZi32r)
          || (Instr->getOpcode() == AMDIL::BREAK_LOGICALZi32r)) {
        return Instr;
      }
    }
    return NULL;
  }

  static bool isReturnBlock(MachineBasicBlock *Block) {
    MachineInstr *Instr = getReturnInstr(Block);
    bool IsReturn = Block->succ_empty();
    if (Instr) {
      assert(IsReturn);
    } else if (IsReturn) {
      DEBUG(dbgs() << "BB" << Block->getNumber()
            << " is return block without RETURN Instr\n");
    }

    return IsReturn;
  }

  static MachineBasicBlock::iterator
  getInstrPos(MachineBasicBlock *Block, MachineInstr *Instr) {
    assert(Instr->getParent() == Block && "instruction doesn't belong to block");
    MachineBasicBlock::iterator I = Block->begin();
    MachineBasicBlock::iterator E = Block->end();
    while (&*I != Instr && I != E) {
      ++I;
    }

    assert(I != E);
    return I;
  }

  static MachineInstr *insertInstrBefore(MachineBasicBlock *Block,
                                         unsigned NewOpcode,
                                         AMDILCFGStructurizer *PassRep,
                                         DebugLoc DL = DebugLoc()) {
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();
    MachineInstr *NewInstr =
      Block->getParent()->CreateMachineInstr(TII->get(NewOpcode), DL);

    MachineBasicBlock::iterator res;
    if (Block->begin() != Block->end()) {
      Block->insert(Block->begin(), NewInstr);
    } else {
      Block->push_back(NewInstr);
    }

    DEBUG(showNewInstr(NewInstr));

    return NewInstr;
  }

  static void insertInstrEnd(MachineBasicBlock *Block,
                             unsigned NewOpcode,
                             AMDILCFGStructurizer *PassRep) {
    insertInstrEnd(Block, NewOpcode, PassRep, DebugLoc());
  }

  static void insertInstrEnd(MachineBasicBlock *Block, unsigned NewOpcode,
                             AMDILCFGStructurizer *PassRep, DebugLoc DL) {
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();
    MachineInstr *NewInstr
      = Block->getParent()->CreateMachineInstr(TII->get(NewOpcode), DL);

    Block->push_back(NewInstr);
    // Assume the instruction doesn't take any reg operand ...

    DEBUG(showNewInstr(NewInstr));
  }

  static MachineInstr *insertInstrBefore(MachineBasicBlock::iterator InstrPos,
                                         unsigned NewOpcode,
                                         AMDILCFGStructurizer *PassRep,
										 DebugLoc DL) {
    MachineInstr *oldInstr = &*InstrPos;
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();
    MachineBasicBlock *Block = oldInstr->getParent();
    MachineInstr *NewInstr =
      Block->getParent()->CreateMachineInstr(TII->get(NewOpcode), DL);

    Block->insert(InstrPos, NewInstr);
    // Assume the instruction doesn't take any reg operand ...

    DEBUG(showNewInstr(NewInstr));
    return NewInstr;
  }

  static MachineInstr *insertInstrBefore(MachineBasicBlock::iterator InstrPos,
                                         unsigned NewOpcode,
                                         AMDILCFGStructurizer *PassRep) {
    return insertInstrBefore(InstrPos, NewOpcode, PassRep, DebugLoc());
  }

  static void insertCondBranchBefore(MachineBasicBlock::iterator InstrPos,
                                     unsigned NewOpcode,
                                     AMDILCFGStructurizer *PassRep,
                                     DebugLoc DL) {
    MachineInstr &OldInstr = *InstrPos;
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();
    MachineBasicBlock *Block = OldInstr.getParent();
    MachineInstr *NewInstr =
      Block->getParent()->CreateMachineInstr(TII->get(NewOpcode),
                                             DL);
    Block->insert(InstrPos, NewInstr);

    const MachineOperand& Operand = OldInstr.getOperand(1);
    switch (Operand.getType()) {
    case MachineOperand::MO_Register:
      MachineInstrBuilder(NewInstr)
        .addReg(Operand.getReg(), false);
      break;

    case MachineOperand::MO_Immediate:
      MachineInstrBuilder(NewInstr)
        .addImm(Operand.getImm());
      break;

    case MachineOperand::MO_FPImmediate:
      MachineInstrBuilder(NewInstr)
        .addFPImm(Operand.getFPImm());
      break;

    default:
      llvm_unreachable("Other type of operand?");
    }

    DEBUG(showNewInstr(NewInstr));
    //erase later OldInstr->eraseFromParent();
  }

  static void insertCondBranchBefore(MachineBasicBlock *Block,
                                     MachineBasicBlock::iterator InsertPos,
                                     unsigned NewOpcode,
                                     AMDILCFGStructurizer *PassRep,
                                     RegiT RegNum,
                                     DebugLoc DL) {
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();

    MachineInstr *NewInstr =
      Block->getParent()->CreateMachineInstr(TII->get(NewOpcode), DL);

    // Insert before
    Block->insert(InsertPos, NewInstr);
    MachineInstrBuilder(NewInstr)
      .addReg(RegNum, false);

    DEBUG(showNewInstr(NewInstr));
  }

  static void insertCondBranchEnd(MachineBasicBlock *Block,
                                  unsigned NewOpcode,
                                  AMDILCFGStructurizer *PassRep,
                                  RegiT RegNum) {
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();
    MachineInstr *NewInstr =
      Block->getParent()->CreateMachineInstr(TII->get(NewOpcode), DebugLoc());

    Block->push_back(NewInstr);
    MachineInstrBuilder(NewInstr).addReg(RegNum, false);

    DEBUG(showNewInstr(NewInstr));
  }

  static void insertAssignInstrBefore(MachineBasicBlock::iterator InstrPos,
                                      AMDILCFGStructurizer *PassRep,
                                      RegiT RegNum, int RegVal) {
    MachineInstr *OldInstr = &*InstrPos ;
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();
    MachineBasicBlock *Block = OldInstr->getParent();
    MachineInstr *NewInstr =
      Block->getParent()->CreateMachineInstr(TII->get(AMDIL::LOADCONSTi32),
                                             DebugLoc());
    MachineInstrBuilder(NewInstr).addReg(RegNum, RegState::Define); //set target
    MachineInstrBuilder(NewInstr).addImm(RegVal); //set src value

    Block->insert(InstrPos, NewInstr);

    DEBUG(showNewInstr(NewInstr));
  }

  static void insertAssignInstrBefore(MachineBasicBlock *Block,
                                      AMDILCFGStructurizer *PassRep,
                                      RegiT RegNum, int RegVal) {
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();

    MachineInstr *NewInstr =
      Block->getParent()->CreateMachineInstr(TII->get(AMDIL::LOADCONSTi32),
                                             DebugLoc());
    MachineInstrBuilder(NewInstr).addReg(RegNum, RegState::Define); // Set target
    MachineInstrBuilder(NewInstr).addImm(RegVal); // Set src value

    if (!Block->empty()) {
      Block->insert(Block->begin(), NewInstr);
    } else {
      Block->push_back(NewInstr);
    }

    DEBUG(showNewInstr(NewInstr));
  }

  static void insertCompareInstrBefore(MachineBasicBlock *Block,
                                       MachineBasicBlock::iterator InstrPos,
                                       AMDILCFGStructurizer *PassRep,
                                       RegiT DstReg, RegiT Src1Reg,
                                       RegiT Src2Reg) {
    const TargetInstrInfo *TII = PassRep->getTargetInstrInfo();
    MachineInstr *NewInstr =
      Block->getParent()->CreateMachineInstr(TII->get(AMDIL::IEQi32rr), DebugLoc());

    MachineInstrBuilder(NewInstr).addReg(DstReg, RegState::Define); // Set target
    MachineInstrBuilder(NewInstr).addReg(Src1Reg); // Set src value
    MachineInstrBuilder(NewInstr).addReg(Src2Reg); // Set src value

    Block->insert(InstrPos, NewInstr);
    DEBUG(showNewInstr(NewInstr));
  }

  static void cloneSuccessorList(MachineBasicBlock *DstBlock,
                                 MachineBasicBlock *SrcBlock) {
    for (MachineBasicBlock::succ_iterator I = SrcBlock->succ_begin(),
           E = SrcBlock->succ_end(); I != E; ++I) {
      MachineBasicBlock *Block = *I;
      DstBlock->addSuccessor(Block); // Block's predecessor is also taken care of
    }
  }

  static MachineBasicBlock *clone(MachineBasicBlock *SrcBlock) {
    MachineFunction *Func = SrcBlock->getParent();
    MachineBasicBlock *NewBlock = Func->CreateMachineBasicBlock();
    Func->push_back(NewBlock); // Insert to function

    for (MachineBasicBlock::const_instr_iterator I = SrcBlock->instr_begin(),
           E = SrcBlock->instr_end(); I != E; ++I) {
      MachineInstr *Instr = Func->CloneMachineInstr(I);
      DEBUG(dbgs() << "Cloning instruction "; Instr->dump(););
      // CloneMachineInstr does not clone the AsmPrinterFlags.
      Instr->setAsmPrinterFlag(
         (llvm::MachineInstr::CommentFlag)I->getAsmPrinterFlags());
      NewBlock->push_back(Instr);
    }
    return NewBlock;
  }

  static void wrapup(MachineBasicBlock *EntryBlock) {
    assert((!EntryBlock->getParent()->getJumpTableInfo()
            || EntryBlock->getParent()->getJumpTableInfo()->isEmpty())
           && "Found a jump table");

     // Collect continue right before endloop
     SmallVector<MachineInstr *, DEFAULT_VEC_SLOTS> ContInstr;
     MachineBasicBlock::iterator Pre = EntryBlock->begin();
     MachineBasicBlock::iterator BlockEnd = EntryBlock->end();
     MachineBasicBlock::iterator BlockIt = Pre;
     while (BlockIt != BlockEnd) {
       if (Pre->getOpcode() == AMDIL::CONTINUE
           && BlockIt->getOpcode() == AMDIL::ENDLOOP) {
         ContInstr.push_back(Pre);
       }
       Pre = BlockIt;
       ++BlockIt;
     }

     // Delete continue right before endloop
     for (unsigned I = 0, Size = ContInstr.size(); I < Size; ++I) {
        ContInstr[I]->eraseFromParent();
     }

     // TODO: to fix up jump table so later phase won't be confused.
     // if (jumpTableInfo->isEmpty() == false) { need to clean the
     // jump table, but there isn't such an interface yet.
     // alternatively, replace all the other blocks in the jump table
     // with the EntryBlock

  } // Wrapup

  static MachineDominatorTree *getDominatorTree(AMDILCFGStructurizer &Pass) {
    return &Pass.getAnalysis<MachineDominatorTree>();
  }

  static MachinePostDominatorTree*
  getPostDominatorTree(AMDILCFGStructurizer &Pass) {
    return &Pass.getAnalysis<MachinePostDominatorTree>();
  }

  static MachineLoopInfo *getLoopInfo(AMDILCFGStructurizer &Pass) {
    return &Pass.getAnalysis<MachineLoopInfo>();
  }
};
} //end of namespace llvm

using namespace llvm;

INITIALIZE_PASS_BEGIN(AMDILCFGPrepare, "amdcfgprepare",
                "AMD IL Control Flow Graph Preparation Pass",
                false, false);
INITIALIZE_PASS_DEPENDENCY(MachinePostDominatorTree);
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo);
INITIALIZE_PASS_END(AMDILCFGPrepare, "amdcfgprepare",
                "AMD IL Control Flow Graph Preparation Pass",
                false, false)

INITIALIZE_PASS_BEGIN(AMDILCFGPerform, "amdcfgperform",
                "AMD IL Control Flow Graph structurizer Pass",
                false, false);
INITIALIZE_PASS_DEPENDENCY(MachinePostDominatorTree);
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo);
INITIALIZE_PASS_END(AMDILCFGPerform, "amdcfgperform",
                "AMD IL Control Flow Graph structurizer Pass",
                false, false)

// createAMDILCFGPreparationPass- Returns a pass
FunctionPass *llvm::createAMDILCFGPreparationPass() {
  return new AMDILCFGPrepare();
}

bool AMDILCFGPrepare::runOnMachineFunction(MachineFunction &Func) {
  TII = Func.getTarget().getInstrInfo();
  return llvmCFGStruct::CFGStructurizer<AMDILCFGStructurizer>().prepare(Func,
                                                                        *this);
}

// createAMDILCFGStructurizerPass- Returns a pass
FunctionPass *llvm::createAMDILCFGStructurizerPass() {
  return new AMDILCFGPerform();
}

bool AMDILCFGPerform::runOnMachineFunction(MachineFunction &Func) {
  TII = Func.getTarget().getInstrInfo();
  return llvmCFGStruct::CFGStructurizer<AMDILCFGStructurizer>().run(Func,
                                                                    *this);
}

