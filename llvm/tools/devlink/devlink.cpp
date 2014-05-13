//===-- devlink.cpp - AMD Device Linker -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This program is a linking utility that implements the device linker 
// functionality in the AMD HSA Linker. Refer to the FDD for more details.
//
//===----------------------------------------------------------------------===//

#pragma warning(disable:4350) //behavior change: 'function template' is called instead of 'function'
#pragma warning(disable:4435) // Object layout under /vd2 will change due to virtual base

#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <limits>
#include <iostream>

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"

#include "HSAILBrigContainer.h"
#include "HSAILBrigObjectFile.h"
#include "HSAILValidator.h"
#include "HSAILItems.h"

#include "Brig.h"
using namespace Brig;
#include "BrigDebugUtils.h"
#include "dwarflinker.h"

typedef BrigsOffset32_t BrigsOffset;
typedef BrigcOffset32_t BrigcOffset;
typedef BrigoOffset32_t BrigoOffset;
typedef BrigdOffset32_t BrigdOffset;

#ifdef _MSC_VER
  // TODO_HSA (2) rework this - move to header file
  // It's worth to have all potentially useful warnings enabled.
  // However, llvm headers cause some warnings, so we have to disable them,
  // because warnings are treated as errors in our build system.
  // Such warnings are disabled in the makefile.
  // Let's re-enable those warnings here, after headers processed:
  #pragma warning(error:4986) //exception specification does not match previous declaration
  #pragma warning(error:4061) //enumerator 'x' in switch of enum 'y' is not explicitly handled by a case label
  #pragma warning(error:4512) //assignment operator could not be generated
  #pragma warning(error:4625) //copy constructor could not be generated because a base class copy constructor is in accessible
  #pragma warning(error:4626) //assignment operator could not be generated because a base class assignment operator is inaccessible
  #pragma warning(error:4100) //unreferenced formal parameter
  #pragma warning(error:4350) //behavior change: 'function template' is called instead of 'function'
  // These warnings are disabled by default by MSVC, but useful:
  #pragma warning(error:4296) // expr is always false
  #pragma warning(error:4189) // 'identifier' : local variable is initialized but not referenced
#endif

/* MSVC 2010 supports static_assert since 2010-04-12 */
/* g++ -std=c++0x supports the static_assert keyword since version 4.3 */
/* http://www.pixelbeat.org/programming/gcc/static_assert.html for more info */
#ifdef __GNUC__
#if (__GNUC__ < 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ < 3))
#if defined(STATIC_ASSERT_CONCAT_) || defined(STATIC_ASSERT_CONCAT) || defined(static_assert)
#error "defined(STATIC_ASSERT_CONCAT_) || defined(STATIC_ASSERT_CONCAT) || defined(static_assert)"
#endif
#define STATIC_ASSERT_CONCAT_(a, b) a##b
#define STATIC_ASSERT_CONCAT(a, b) STATIC_ASSERT_CONCAT_(a, b)
#define static_assert(e,m) enum { STATIC_ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }
#endif /* version < 4.3 */
#endif /* gcc */

// Use to clearly mark implicit dependences, assumptions etc.
#define ASSUMPTION(text)

#define ASCIZ_FILE_FORMAT_BRIG "'old plain' ELF"
#define ASCIZ_FILE_FORMAT_BIF  "BIF3.0"

using namespace llvm;

namespace {
  cl::list<std::string>
  InputFilenames(cl::Positional, cl::desc("<input object files>"),
                 cl::OneOrMore);

  cl::opt<std::string>
  OutputFilename("o", cl::value_desc("filename"), cl::init(""),
    cl::desc("Output filename. Default is 'linked.exe.EXT'"
    " ('linked.lib.EXT' for library), where EXT is 'brig' or 'bif'."));

  cl::opt<bool>
  CreateLibrary("library", cl::desc("Create device code (BRIG) Library"), cl::init(false));

  cl::opt<bool>
  BifOutput("obif", cl::desc("Emit BRIG to " ASCIZ_FILE_FORMAT_BIF " file (default is " ASCIZ_FILE_FORMAT_BRIG ")"), cl::init(false));

  cl::opt<bool>
  BifInput("ibif", cl::desc("Expect input files in " ASCIZ_FILE_FORMAT_BIF " format (default is " ASCIZ_FILE_FORMAT_BRIG ")"), cl::init(false));

  cl::opt<bool>
  NoDeDuper("no-deduper", cl::Optional, cl::init(false),
            cl::desc("Do not perform DeDuping"));

  cl::opt<bool>
  NoDeDeader("no-dedeader", cl::Optional, cl::init(false),
            cl::desc("Do not perform DeDeading"));

  cl::opt<bool> 
  linkDwarf("link-dwarf", cl::Optional, cl::init(false), 
                    cl::desc("Perform linking DWARF data from .debug sections"));
  
  cl::opt<bool> 
  printBRIGSectionsOffsets("debug-print-offsets", cl::Hidden, cl::Optional, cl::init(false), 
                           cl::desc("Print locations of .code and .directives sections in the linked brig"));
  
  cl::opt<std::string>
  DebugInfoFilename("debug-dwarffile", cl::desc("Debug Info filename"), cl::value_desc("filename"), cl::init(""));

  // TODO_HSA (3) remove useless options & update test suite
  cl::opt<bool>
  DebugParser("debug-parser", cl::Hidden, cl::Optional, cl::init(false), 
              cl::desc("Debug Parser"));

  cl::opt<bool>
  DetailDebugParser("detail-debug-parser", cl::Hidden, cl::Optional, cl::init(false),
                    cl::desc("Detail Debug Parser"));

  cl::opt<bool>
  DebugDeDeader("debug-dedeader", cl::Hidden, cl::Optional, cl::init(false), 
                cl::desc("Debug DeDeader"));

  cl::opt<bool>
  DetailDebugDeDeader("detail-debug-dedeader", cl::Hidden, cl::Optional, cl::init(false), 
                      cl::desc("Detail Debug DeDeader"));

  cl::opt<bool>
  DebugDeDuper("debug-deduper", cl::Hidden, cl::Optional, cl::init(false), 
               cl::desc("Debug DeDuper"));

  cl::opt<bool>
  NoIterativeOptimization("disable-recursive-optimization", cl::Hidden, cl::Optional, cl::init(false), 
               cl::desc("Disable recursive detection of useless items"));

  cl::opt<bool>
  DisableValidatorInput("disable-input-validator", cl::Hidden, cl::Optional, cl::init(false), 
               cl::desc("Disable validation of input BRIG file(s)"));

  cl::opt<bool>
  DisableValidatorOutput("disable-output-validator", cl::Hidden, cl::Optional, cl::init(false), 
               cl::desc("Disable validation of output BRIG file"));

  cl::opt<bool>
  DebugRemoveDeclarations("debug-remove-declarations", cl::Hidden, cl::Optional, cl::init(false), 
               cl::desc("Remove resolved declarations, thus making output BRIG invalid."));

  cl::opt<bool>
  DebugCorruptOffsets("debug-corrupt-offsets", cl::Hidden, cl::Optional, cl::init(false), 
               cl::desc("Corrupt offsets from directives to code, thus making output BRIG invalid."));
}


static std::string getOutputFileName() {
  if (OutputFilename.size() > 0) {
    return OutputFilename;
  } else { 
    // TIP: [artem] We can construct output filename 
    // from the name of the first input file:
    // string fileName = InputFilenames[0];
    std::string fileName = "linked";\
    std::string new_ext = CreateLibrary ? ".lib" : ".exe";
    new_ext += (BifOutput ? ".bif" : ".brig");
    std::string::size_type pos = fileName.find_last_of('.');
    if (pos != std::string::npos && pos > 0 && fileName.substr(pos) != new_ext) {
      fileName = fileName.substr(0, pos);
    }
    return fileName + new_ext;
  }
}

static
int ValidateContainer(BrigContainer &c, std::istream *is, const bool linkedBrig) {
    Validator vld(c);
    if (!vld.validate(linkedBrig ? Validator::VM_BrigLinked : Validator::VM_BrigNotLinked)) {
        std::cerr << vld.getErrorMsg(is) << '\n';
        return 1;
    }
    return 0;
}

namespace {

// [artem] This is not needed for now:
// #define CODE_BACKREFS

class BackRefs {
public:
    struct Referrer {
        Offset base;
        unsigned int offset;
    };
    typedef std::map<Offset, std::vector<Referrer> > ReferrersMap;
    enum Type {
         TYPE_DEFAULT = 0
            // Default semantic of back references is "this item is *used_by* another item".
            // Another item contains offset to this item in brig structure.
        ,TYPE_IMPLICIT
            // (this) item is being used by another item, 
            // *but* there is no direct reference. 
            // For example, directiveFunction does not contain offset 
            // to its 2nd and subsequent parameters, but really uses
            // them. Therefore, 2nd function parameter (and subsequent ones)
            // shall have implicit = true.
            //
            // TIP_OPT: we can use (offset == 0) to indicate implicit connections
            // and thus save some memory. This is possible, because BRIG does not allow
            // anything except size/kind reside at offset 0.
            //
        ,TYPE_HINT_NEXT_DIRECTIVE
            // (this) item is being used by another item via d_nextDirective field.
            // d_nextDirective is like an ordinary numeric attribute; its meaning is 
            // like "how many directives are belonging to the scope of this function/kernel".
            // Therefore, this type of back ref does not have "used_by" semantics.
        ,TYPE_HINT_1ST_SCOPED
            // This type of back ref does not have "used_by" semantics.
    };
    struct TypedReferrer {
        Offset base;
        unsigned offset;
        Type type;
    };
    typedef std::map<Offset, std::vector<TypedReferrer> > TypedReferrersMap;
    ReferrersMap      inst2op;
    ReferrersMap      op2op;
    ReferrersMap      op2dir;
    TypedReferrersMap dir2dir;
    ReferrersMap      op2str;
    ReferrersMap      dir2str;
    ReferrersMap      debug2str;
    #ifdef CODE_BACKREFS
    ReferrersMap      dir2inst;
    ReferrersMap      debug2inst;
    #endif
private:
    bool isUsedByDirective(const Directive& d) const {
        TypedReferrersMap::const_iterator p = dir2dir.find(d.brigOffset());
        if (p == dir2dir.end()) { return false; }
        const TypedReferrersMap::mapped_type& dest_users = p->second;
        for (unsigned i = 0, i_end = dest_users.size(); i < i_end; ++i) {
            if (dest_users[i].type != TYPE_HINT_NEXT_DIRECTIVE
            &&  dest_users[i].type != TYPE_HINT_1ST_SCOPED) {
                return true;
            }
        }
        return false;
    }
public:
    // rebind...(...,oldDest,newDest):
    // A set of functions which rebind direct refs (in brig) and backRefs to/from item.
    // All "users" are disconnected from oldDest and connected to newDest.
    // References are updated in both brig and backRef maps, thus keeping data consistent.
    void rebindOperandUsers(BrigContainer& bc, const BrigoOffset oldDest, const BrigoOffset newDest) {
        assert(oldDest != newDest);
        if (oldDest == newDest) { return; }
        while (!op2op[oldDest].empty()) {
            const Referrer user = op2op[oldDest].back();
            *(BrigoOffset32_t *)bc.operands().getData(user.base + user.offset) = newDest;
            op2op[newDest].push_back(user);
            op2op[oldDest].pop_back();
        }
        while (!inst2op[oldDest].empty()) {
            const Referrer user = inst2op[oldDest].back();
            *(BrigoOffset32_t *)bc.insts().getData(user.base + user.offset) = newDest;
            inst2op[newDest].push_back(user);
            inst2op[oldDest].pop_back();
        }
    }
    void rebindDirectiveUsers(BrigContainer& bc, const BrigdOffset oldDest, const BrigdOffset newDest) {
        assert(oldDest != newDest);
        if (oldDest == newDest) { return; }
        while (!op2dir[oldDest].empty()) {
            const Referrer user = op2dir[oldDest].back();
            *(BrigdOffset32_t *)bc.operands().getData(user.base + user.offset) = newDest;
            op2dir[newDest].push_back(user);
            op2dir[oldDest].pop_back();
        }
        // Update references from directives,
        // except nextDirective-references (from functions/kernels),
        // otherwise we will lose scope information (i.e. which directives
        // are belonging to referring functions/kernels).
        std::vector<TypedReferrer>& dir2dir_old = dir2dir[oldDest];
        for (unsigned i = 0; i < dir2dir_old.size(); ++i) {
            const TypedReferrer user = dir2dir_old[i];
            if (user.type == TYPE_HINT_NEXT_DIRECTIVE
            ||  user.type == TYPE_HINT_1ST_SCOPED) {
                // keep this reference
            }
            else {
                *(BrigdOffset32_t*)bc.directives().getData(user.base + user.offset) = newDest;
                dir2dir[newDest].push_back(user);
                dir2dir_old.erase(dir2dir_old.begin() + i);
                --i;
            }
        }
    }
    void rebindStringUsers(BrigContainer& bc, const BrigsOffset oldDest, const BrigsOffset newDest) {
        assert(oldDest != newDest);
        if (oldDest == newDest) { return; }
        while (!op2str[oldDest].empty()) {
            const Referrer user = op2str[oldDest].back();
            *(BrigsOffset32_t *)bc.operands().getData(user.base + user.offset) = newDest;
            op2str[newDest].push_back(user);
            op2str[oldDest].pop_back();
        }
        while (!dir2str[oldDest].empty()) {
            const Referrer user = dir2str[oldDest].back();
            *(BrigsOffset32_t *)bc.directives().getData(user.base + user.offset) = newDest;
            dir2str[newDest].push_back(user);
            dir2str[oldDest].pop_back();
        }
        while (!debug2str[oldDest].empty()) {
            const Referrer user = debug2str[oldDest].back();
            *(BrigsOffset32_t *)bc.debugChunks().getData(user.base + user.offset) = newDest;
            debug2str[newDest].push_back(user);
            debug2str[oldDest].pop_back();
        }
    }
    bool isUsedByOperand(const Directive& d) const {
        ReferrersMap::const_iterator p = op2dir.find(d.brigOffset());
        if (p == op2dir.end()) { return false; }
        return !p->second.empty();
    }
    inline bool isUnused(const Directive& d) const {
        return !(isUsedByOperand(d) || isUsedByDirective(d));
    }
    inline bool isUnused(const Operand& o)  {
        return inst2op[o.brigOffset()].empty()
            && op2op  [o.brigOffset()].empty();
    }
    void updateUsersUponRelocation(BrigContainer& bc, Directive& dest, const BrigdOffset newOffsetOfDest) const
    {
        {
            ReferrersMap::const_iterator p = op2dir.find(dest.brigOffset());
            if (p != op2dir.end()) {
                const ReferrersMap::mapped_type& users = p->second;
                for (unsigned i = 0, i_end = users.size(); i < i_end; ++i) {
                    *(BrigdOffset32_t*)bc.operands().getData(users[i].base + users[i].offset)
                        = newOffsetOfDest;
                }
            }
        }
        {
            TypedReferrersMap::const_iterator p = dir2dir.find(dest.brigOffset());
            if (p != dir2dir.end()) {
                const TypedReferrersMap::mapped_type& users = p->second;
                for (unsigned i = 0, i_end = users.size(); i < i_end; ++i) {
                    if (users[i].type != TYPE_IMPLICIT) {
                        *(BrigdOffset32_t*)bc.directives().getData(users[i].base + users[i].offset)
                            = newOffsetOfDest;
                    }
                }
            }
        }
    }
    void updateUsersUponRelocation(BrigContainer& bc, Operand& dest, const BrigoOffset newOffsetOfDest) const
    {
        {
            ReferrersMap::const_iterator p = inst2op.find(dest.brigOffset());
            if (p != inst2op.end()) {
                const ReferrersMap::mapped_type& users = p->second;
                for (unsigned i = 0, i_end = users.size(); i < i_end; ++i) {
                    *(BrigoOffset32_t*)bc.insts().getData(users[i].base + users[i].offset)
                        = newOffsetOfDest;
                }
            }
        }
        {
            ReferrersMap::const_iterator p = op2op.find(dest.brigOffset());
            if (p != op2op.end()) {
                const ReferrersMap::mapped_type& users = p->second;
                for (unsigned i = 0, i_end = users.size(); i < i_end; ++i) {
                    *(BrigoOffset32_t*)bc.operands().getData(users[i].base + users[i].offset)
                        = newOffsetOfDest;
                }
            }
        }
    }
    void updateStringUsersUponRelocation(
         BrigContainer& bc
        ,const BrigsOffset destBrigOffset
        ,const BrigsOffset newOffsetOfDest) const
    {
        {
            ReferrersMap::const_iterator p = op2str.find(destBrigOffset);
            if (p != op2str.end()) {
                const ReferrersMap::mapped_type& users = p->second;
                for (unsigned i = 0, i_end = users.size(); i < i_end; ++i) {
                    *(BrigsOffset32_t*)bc.operands().getData(users[i].base + users[i].offset)
                        = newOffsetOfDest;
                }
            }
        }
        {
            ReferrersMap::const_iterator p = dir2str.find(destBrigOffset);
            if (p != dir2str.end()) {
                const ReferrersMap::mapped_type& users = p->second;
                for (unsigned i = 0, i_end = users.size(); i < i_end; ++i) {
                    *(BrigsOffset32_t*)bc.directives().getData(users[i].base + users[i].offset)
                        = newOffsetOfDest;
                }
            }
        }
        {
            ReferrersMap::const_iterator p = debug2str.find(destBrigOffset);
            if (p != debug2str.end()) {
                const ReferrersMap::mapped_type& users = p->second;
                for (unsigned i = 0, i_end = users.size(); i < i_end; ++i) {
                    *(BrigsOffset32_t*)bc.debugChunks().getData(users[i].base + users[i].offset)
                        = newOffsetOfDest;
                }
            }
        }
    }
};

} // namespace

#define UNUSED_OFFSET 0
#define OFFSET_WRONG ((HSAIL_ASM::Offset)-2)
#define OFFSET_DEAD  ((HSAIL_ASM::Offset)-1)
#define IS_OFFSET_DEAD(offset) (offset == OFFSET_DEAD)

namespace HSAIL_ASM { //TODO_HSA (3) Use handle templates from libHSAIL when available.

inline bool inDebugSection(Directive& d) {
    const BrigSectionImpl* const ps = d.section();
    return ps == &(d.container()->debugChunks());
}
inline bool inDebugSection(Operand& )  { return false; }
inline bool inDebugSection(Inst& )     { return false; }

#define HANDLE_CASE(kind) \
    case Brig::BrigE##kind: \
        assert(!inDebugSection(i)); \
        h.handle(kind(i)); \
        break;
        
#define HANDLE_CASE_BLOCK(kind) \
    case Brig::BrigEDirective##kind: \
        if(!inDebugSection(i)) { h.handle(kind(i)); } \
        else     { h.handleInDebugSection(kind(i)); } \
        break;

template <typename Handler>
void visit(Operand i, Handler& h)
{
    switch(i.brig()->kind) {
    HANDLE_CASE(OperandReg)
    HANDLE_CASE(OperandImmed)
    HANDLE_CASE(OperandRegV2)
    HANDLE_CASE(OperandRegV4)
    HANDLE_CASE(OperandAddress)
    HANDLE_CASE(OperandLabelRef)
    HANDLE_CASE(OperandIndirect)
    HANDLE_CASE(OperandCompound)
    HANDLE_CASE(OperandArgumentList)
    //HANDLE_CASE(OperandFunctionList) // not used for now
    HANDLE_CASE(OperandArgumentRef)
    HANDLE_CASE(OperandWaveSz)
    HANDLE_CASE(OperandFunctionRef)
    // HANDLE_CASE(OperandPad) // not defined/used for now
    HANDLE_CASE(OperandOpaque)
    default: assert(!"accepting operand: unknown kind");
    }
}

template <typename Handler>
void visit(Inst i, Handler& h)
{
    switch(i.brig()->kind) {
    HANDLE_CASE(InstBase)
    HANDLE_CASE(InstMod)
    HANDLE_CASE(InstCvt)
    HANDLE_CASE(InstRead)
    HANDLE_CASE(InstBar)
    HANDLE_CASE(InstLdSt)
    HANDLE_CASE(InstCmp)
    HANDLE_CASE(InstMem)
    HANDLE_CASE(InstAtomic)
    HANDLE_CASE(InstAtomicImage)
    HANDLE_CASE(InstImage)
    default: assert(!"accepting instruction: unknown kind");
    }
}

template <typename Handler>
void visit(Directive i, Handler& h)
{
    switch(i.brig()->kind) {
    HANDLE_CASE(DirectiveFunction)
    HANDLE_CASE(DirectiveKernel)
    HANDLE_CASE(DirectiveSymbol)
    HANDLE_CASE(DirectiveImage)
    HANDLE_CASE(DirectiveSampler)
    HANDLE_CASE(DirectiveLabel)
    HANDLE_CASE(DirectiveLabelList)
    HANDLE_CASE(DirectiveVersion)
    HANDLE_CASE(DirectiveProto)
    HANDLE_CASE(DirectiveFile)
    HANDLE_CASE(DirectiveComment)
    HANDLE_CASE(DirectiveLoc)
    HANDLE_CASE(DirectiveInit)
    HANDLE_CASE(DirectiveLabelInit)
    HANDLE_CASE(DirectiveControl)
    HANDLE_CASE(DirectivePragma)
    HANDLE_CASE(DirectiveExtension)
    HANDLE_CASE(ArgStart)
    HANDLE_CASE(ArgEnd)
    HANDLE_CASE_BLOCK(BlockStart)
    HANDLE_CASE_BLOCK(BlockNumeric)
    HANDLE_CASE_BLOCK(BlockString)
    HANDLE_CASE_BLOCK(BlockEnd)
    default: assert(!"accepting directive: unknown kind");
    }
}

#undef HANDLE_CASE
#undef HANDLE_CASE_BLOCK
};

namespace {
using namespace HSAIL_ASM;

class Modules
{
    class VersionInfo 
    {
    private:
        uint16_t major_;
        uint16_t minor_;
        uint16_t machine_;
        uint16_t profile_;
        uint16_t ftz_;
        std::string file_;
        BrigdOffset begin_;
        bool isValid_;
    public:
        VersionInfo() : isValid_(false) {
            clear();
        }
        // OK default T(const T&);
        // OK default const T& operator=(const T&);
    public:
        void clear() {
            major_   = (uint16_t)-1;
            minor_   = (uint16_t)-1;
            machine_ = (uint16_t)-1;
            profile_ = (uint16_t)-1;
            ftz_     = (uint16_t)-1;
            begin_   = OFFSET_WRONG;
            file_    = "<unknown>";
        }
        void set(DirectiveVersion& d, const std::string& file) {
            major_    = d.major();
            minor_    = d.minor();
            machine_  = d.machine();
            profile_  = d.profile();
            ftz_      = d.ftz();
            begin_    = d.brigOffset();
            file_     = file;
        }
        void set(const VersionInfo& from) {
            major_    = from.major_;
            minor_    = from.minor_;
            machine_  = from.machine_;
            profile_  = from.profile_;
            ftz_      = from.ftz_;
            begin_    = from.begin_;
            file_     = from.file_;
        }
        void setValid() { isValid_ = true; }
        // This function answers the following questions:
        // - is it Ok to resolve declaration (from this module) to definition which reside in module m2 ?
        // - is it Ok to link this module and module m2 ?
        // Answers to these questions are the same so far...
        bool isCompatible(const VersionInfo& m2) const {
            // NOTE: Modules with different profiles and precision-modes can be linked together.
            // (It is up to the Finalizer how to execute this.)
            //
            // HSAIL spec 1.0 (19 Jun 2012) p.202:
            // "It is a linker error for multiple files to have different major version numbers or
            // different machine models and to attempt to link to the same executable."
            //
            assert (isValid_ && m2.isValid_);
            return (isValid_ && m2.isValid_)
                && major_   == m2.major_
             // && minor_   == m2.minor_ 
                // HSAIL spec 1.0 (19 Jun 2012) p.202: 
                // "kernels or functions compiled at one minor level can 
                // call functions compiled at a different minor level."
                && machine_ == m2.machine_;
        }
        BrigdOffset beginOffset() const {return begin_; }
        bool isDummy() const { return (begin_ == OFFSET_WRONG); }
        std::string getFile() const { return file_; }
        std::string getDescription() const {
            std::ostringstream s;
            s <<   "Begin:"   << begin_ << ", version " << major_ << ":"   << minor_ << ":";
            const char * str = HSAIL_ASM::profile2str(profile_);
            if (str) { s << "$" << str; } else { s << "?_PROFILE(" << profile_ << ")"; }
            s << ",";
            str = HSAIL_ASM::machine2str(machine_);
            if (str) { s << "$" << str; } else { s << "?_MACHINE(" << machine_ << ")"; }
            s << ",";
            str = HSAIL_ASM::sftz2str(ftz_);
            if (str) { s << "$" << str; } else { s << "?_FTZ(" << ftz_ << ")"; }
            s << "; file: "    << file_.c_str();
            return std::string(s.str());
        }
    };
    typedef std::map<BrigdOffset,VersionInfo> Map;
    Map map_; // key is the end-offset of directives in module

    Modules(const Modules&);
    const Modules& operator=(const Modules&);
public:
    Modules() {}
    // register module information
    void registerEntry(DirectiveVersion& d, const std::string& file) {
        VersionInfo vi;
        vi.set(d,file);
        map_[d.brigOffset()] = vi;
    }
    // finish construction of the modules info using previously registered information.
    void construct(const BrigContainer& bc) {
        VersionInfo endOfMap;
        map_[static_cast<BrigdOffset>(bc.directives().size())] = endOfMap;
        //
        Map::reverse_iterator p = map_.rbegin();
        Map::reverse_iterator q = p;
        for(++q; q != map_.rend(); ++q) {
            p->second.set(q->second);
            p->second.setValid();
            ++p;
        }
        p->second.clear();
        p->second.setValid();
    }
    // container emulation
    typedef Map::const_iterator const_iterator; // Only RO access for now.
    // begin/end container of modules
    const_iterator begin() const {
        return map_.begin();
    }
    const_iterator end() const {
        return map_.end  ();
    }
    // find a module to which the directive belongs.
    const_iterator find(const Directive& d) const {
        return map_.upper_bound(d.brigOffset());
    }
    // check if two modules are compatible.
    bool isCompatible(const const_iterator m1, const const_iterator m2) const {
      return m1->second.isCompatible(m2->second);
    }
    // offset of the first directive of module
    BrigdOffset beginOffset(const const_iterator m) const {
        return m->second.beginOffset();
    }
    // offset just-beyond the last directive of module
    BrigdOffset endOffset(const const_iterator m) const {
        return m->first;
    }
    // return the description of module to which the directive belongs.
    inline std::string belongsToModule(const Directive& d) const {
        return std::string(find(d)->second.getFile().c_str());
    }
    // check if all modules are linkable
    bool linkable() const {
        // use the simplest approach: check each module with all others.
        for(Map::const_iterator m1 = map_.begin(); m1 != map_.end(); ++m1) {
            if (m1->second.isDummy()) continue;
            VersionInfo m1_info = m1->second;
            for(Map::const_iterator m2 = map_.begin(); m2 != map_.end(); ++m2) {
                if (m1 == m2) continue;
                if (m2->second.isDummy()) continue;
                if (!m1_info.isCompatible(m2->second)) return false;
            }
        }
        return true;
    }
    void print(raw_ostream& out) const {
        out << "Info: Module map:\n";
        Map::const_iterator p;
        for(p = map_.begin(); p != map_.end(); ++p) {
            if (p->second.isDummy()) continue;
            out << "      End:" << (unsigned)p->first << " " << p->second.getDescription() << "\n";
        }
        out.flush();
    }
};

struct handlers4DebugSectionNA {
    void handleInDebugSection( Directive ) { assert(!"NA for .debug items"); }
    void handleInDebugSection( Directive ) const { assert(!"NA for .debug items"); }
};

class AppendRelocator : public handlers4DebugSectionNA
{
    Offset const m_strShift;
    Offset const m_opndShift;
    Offset const m_instShift;
    Offset const m_dirShift;
    Offset const m_debugShift;
    Modules&     modules_;
    std::string  const file_;

    void patchRef(StrRef r) const {
        if (r.deref()!=0) {
            r.deref() += m_strShift;
        }
    }

    template <typename I>
    void patchRef(ItemRef<I> i, Operand*) const {
        i.deref() += m_opndShift;
    }
    template <typename I>
    void patchRef(ItemRef<I> i, Inst*) const {
        i.deref() += m_instShift;
    }
    template <typename I>
    void patchRef(ItemRef<I> i, Directive*) const {
        i.deref() += m_dirShift;
    }
    template <typename I>
    void patchRef(ItemRef<I> i) const {
        if (i.deref()!=0) {
            patchRef(i,reinterpret_cast<I*>(NULL));
        }
    }

public:
    AppendRelocator(BrigContainer& c,Modules& modules,const char* const file)
        : m_strShift  (c.strings().size()    - c.strings    ().NUM_BYTES_RESERVED)
        , m_opndShift (c.operands().size()   - c.operands   ().begin().brigOffset())
        , m_instShift (c.insts().size()      - c.insts      ().begin().brigOffset())
        , m_dirShift  (c.directives().size() - c.directives ().begin().brigOffset())
        , m_debugShift(c.debugChunks().size()- c.debugChunks().begin().brigOffset())
        , modules_(modules)
        , file_(file)
    {}
    AppendRelocator();
    AppendRelocator(const AppendRelocator& r);
    const AppendRelocator& operator=(const AppendRelocator& r);
  
    // for debug info linker
    Offset getCodeOffset() const {
      return m_instShift;
    }

    Offset getDirectivesOffset() const {
      return m_dirShift;
    }

    // operands
    void handle( OperandImmed ) const {} // nothing to do
    void handle( OperandWaveSz ) const {} // nothing to do

    void handle( OperandReg o ) const {
        patchRef(o.name());
    }
    void handle( OperandRegV2 o ) const {
        patchRef(o.reg(0));
        patchRef(o.reg(1));
    }
    void handle( OperandRegV4 o ) const {
        patchRef(o.reg(0));
        patchRef(o.reg(1));
        patchRef(o.reg(2));
        patchRef(o.reg(3));
    }
    void handle( OperandCompound o ) const {
        patchRef(o.name());
        patchRef(o.reg());
    }
    void handle( OperandAddress o ) const {
        patchRef(o.directive());
    }
    void handle( OperandIndirect o ) const {
        patchRef(o.reg());
    }
    void handle( OperandArgumentList o ) const {
        ArgumentListArgRefs a = o.args();
        for(int i=0, e=a.size(); i<e; ++i)
            patchRef(a[i]);
    }
    void handle( OperandArgumentRef o ) const {
        patchRef(o.arg());
    }
    void handle( OperandFunctionRef o ) const {
        patchRef(o.fn());
    }
    void handle( OperandLabelRef o ) const {
        patchRef(o.labeldirective());
    }
    void handle( OperandOpaque o ) const {
        patchRef(o.name());
        patchRef(o.reg());
    }

    // directives
    void handle( DirectiveVersion d ) const {
        patchRef(d.code());
        modules_.registerEntry(d,file_);
    } 
    void handle( DirectiveInit d ) const {
        patchRef(d.code());
    }
    void handle( DirectiveControl d ) const {
        patchRef(d.code());
    }
    void handle( BlockStart d ) const {
        patchRef(d.name());
        patchRef(d.code());
    }
    void handle( BlockString d ) const {
        patchRef(d.name());
    }
    void handle( BlockNumeric ) const { // nothing to do
    }
    void handle( BlockEnd ) const { // nothing to do
    } 
    void handle( DirectiveFile d ) const {
        patchRef(d.filename());
        patchRef(d.code());
    }
    void handle( DirectiveLoc d ) const {
        patchRef(d.code());
    }
    void handle( DirectiveComment d ) const {
        patchRef(d.code());
        patchRef(d.name());
    }
    void handle( DirectiveExtension d ) const {
        patchRef(d.code());
        patchRef(d.name());
    }
    void handle( DirectiveLabel d ) const {
        patchRef(d.code());
        patchRef(d.name());
    }
    void handle( DirectivePragma d ) const {
        patchRef(d.code());
        patchRef(d.name());
    }
    void handle( DirectiveProto d ) const {
        patchRef(d.code());
        patchRef(d.name());
    }
     void handle( DirectiveSymbol d ) const {
        patchRef(d.code());
        patchRef(d.name());
        patchRef(d.init());
    }
    void handle( DirectiveImage d ) const {
        patchRef(d.code());
        patchRef(d.name());
    }
    void handle( DirectiveSampler d ) const {
        patchRef(d.code());
        patchRef(d.name());
    }
    void handle( DirectiveFunctionCommon d ) const { // functions and kernels
        patchRef(d.name());
        patchRef(d.code());
        patchRef(d.firstInParam());
        patchRef(d.firstScopedDirective());
        patchRef(d.nextDirective());
    }
    void handle( ArgStart d ) const {
        patchRef(d.code());
    }
    void handle( ArgEnd d ) const {  
        patchRef(d.code());
    }
    void handle( DirectiveLabelInit d ) const {
        patchRef(d.code());
        LabelList labels = d.labels();
        for(unsigned i=0; i<labels.size(); ++i) { patchRef(labels[i]); }
    }
    void handle( DirectiveLabelList d ) const {
        patchRef(d.code());
        LabelList labels = d.labels();
        for(unsigned i=0; i<labels.size(); ++i) { patchRef(labels[i]); }
    }

    // instructions
    void handle( Inst i ) const {
        for(int o=0; o<5 /* TODO_HSA (3) magic 5 */; ++o) {
            patchRef(i.operand(o));
        }
    }
};

class XRecordRefs
{
    BackRefs& br;

    template <typename FromItem,typename ToRef,typename VectorElem>
    static void record(
         std::map< Offset,std::vector<VectorElem> >& themap
        ,FromItem fromItem
        ,ToRef toRef) 
    {
       VectorElem const ref = { fromItem.brigOffset(), 
            reinterpret_cast<const char*>(&toRef.deref()) - reinterpret_cast<const char*>(fromItem.brig()) };
       themap[toRef.deref()].push_back(ref); 
    }
    
    template <typename ToItem>
    static void recordImplicit(
         std::map< Offset ,std::vector<BackRefs::TypedReferrer> >& themap
        ,Directive fromItem
        ,ToItem toItem) 
    {
        BackRefs::TypedReferrer const ref = { fromItem.brigOffset(), 0, BackRefs::TYPE_IMPLICIT };
        themap[toItem.brigOffset()].push_back(ref); 
    }
    
    template <typename ToItem>
    void recordRef_(Inst from, ItemRef<ToItem> refTo, Operand*) {
        record(br.inst2op,from,refTo);
    }
    template <typename ToItem>
    void recordRef_(Operand from, ItemRef<ToItem> refTo, Operand*) {
        record(br.op2op,from,refTo);
    }
    template <typename ToItem>
    void recordRef_(Operand from, ItemRef<ToItem> refTo, Directive*) {
        record(br.op2dir,from,refTo);
    }
    #ifdef CODE_BACKREFS
    template <typename ToItem>
    void recordRef_(Directive from, ItemRef<ToItem> refTo, Inst*,inDebugSection) {
        assert (!inDebugSection && "debug2inst backrefs are not supported"); (void)inDebugSection;
        record(br.dir2inst,from,refTo);
    }
    #endif
    template <typename ToItem>
    void recordRef_(Directive from, ItemRef<ToItem> refTo, Directive*) {
        record(br.dir2dir,from,refTo);
    }

    void recordRefImplicit(Directive from, Directive to) {
        recordImplicit(br.dir2dir,from,to);
    }

    template <typename FromItem, typename ToItem>
    void recordRef(FromItem fromItem, ItemRef<ToItem> toRef) {
        if (toRef.deref()!=0) {
            recordRef_(fromItem,toRef,reinterpret_cast<ToItem*>(NULL));
        }
    }

    void recordRefNextDirective(Directive from, ItemRef<Directive> toRef) {
        if (toRef.deref()!=0) {
            BackRefs::TypedReferrer const ref = { 
                 from.brigOffset()
                ,reinterpret_cast<const char*>(&toRef.deref()) - reinterpret_cast<const char*>(from.brig())
                ,BackRefs::TYPE_HINT_NEXT_DIRECTIVE
            };
            br.dir2dir[toRef.deref()].push_back(ref);
        }
    }

    void recordRefFirstScoped(Directive from, ItemRef<Directive> toRef) {
        if (toRef.deref()!=0) {
            BackRefs::TypedReferrer const ref = { 
                 from.brigOffset()
                ,reinterpret_cast<const char*>(&toRef.deref()) - reinterpret_cast<const char*>(from.brig())
                ,BackRefs::TYPE_HINT_1ST_SCOPED
            };
            br.dir2dir[toRef.deref()].push_back(ref);
        }
    }

    void recordRef(Directive fromItem, StrRef str, const bool inDebugSection = false) {
        if (str.deref()!=0) {
            if (!inDebugSection) { record(br.dir2str  ,fromItem,str); }
            else                 { record(br.debug2str,fromItem,str); }
        }
    }

    void recordRef(Operand fromItem, StrRef str) {
        if (str.deref()!=0) {
            record(br.op2str,fromItem,str);
        }
    }

    XRecordRefs();
    XRecordRefs(const XRecordRefs& r);
    const XRecordRefs& operator=(const XRecordRefs& r);
public:
    XRecordRefs(BackRefs& br_) : br(br_)
    {}
  
    // operands
    void handle( OperandWaveSz ) {} // nothing to do
    void handle( OperandImmed ) {} // nothing to do

    void handle( OperandReg o ) {
        recordRef(o,o.name());
    }
    void handle( OperandRegV2 o ) {
        recordRef(o,o.reg(0));
        recordRef(o,o.reg(1));
    }
    void handle( OperandRegV4 o ) {
        recordRef(o,o.reg(0));
        recordRef(o,o.reg(1));
        recordRef(o,o.reg(2));
        recordRef(o,o.reg(3));
    }
    void handle( OperandCompound o ) {
        recordRef(o,o.name());
        recordRef(o,o.reg());
    }
    void handle( OperandAddress o ) {
        recordRef(o,o.directive());
    }
    void handle( OperandIndirect o ) {
        recordRef(o,o.reg());
    }
    void handle( OperandArgumentList o ) {
        ArgumentListArgRefs a = o.args();
        for(int i=0, e=a.size(); i<e; ++i)
            recordRef(o,a[i]);
    }
    void handle( OperandArgumentRef o ) {
        recordRef(o,o.arg());
    }
    void handle( OperandFunctionRef o ) {
        recordRef(o,o.fn());
    }
    void handle( OperandLabelRef o )  {
        recordRef(o,o.labeldirective());
    }
    void handle( OperandOpaque o ) {
        recordRef(o,o.name());
        recordRef(o,o.reg());
    }

    // directives
    void handle( DirectiveControl d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #else
        (void)d;
        #endif
    }
    void handle( DirectiveInit d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #else
        (void)d;
        #endif
    }
    void handle( DirectiveVersion d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #else
        (void)d;
        #endif
    }
    void handle( BlockStart d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
        Directive d_inblock = d.next();
        for (;;) {
          recordRefImplicit(d,d_inblock);
          if (d_inblock.brig()->kind == BrigEDirectiveBlockEnd) { break; }
          d_inblock = d_inblock.next();
        }
    }
    void handle( BlockString d ) {
        recordRef(d,d.name());
    }
    void handle( BlockNumeric ) { // nothing to do
    }
    void handle( BlockEnd ) { // nothing to do
    } 
    void handle( DirectiveFile d ) {
        recordRef(d,d.filename());
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
    }
    void handle( DirectiveLoc d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #else
        (void)d;
        #endif
    }
    void handle( DirectiveComment d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
    }
    void handle( DirectiveExtension d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
    }
    void handle( DirectiveLabel d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
    }
    void handle( DirectivePragma d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
    }
    void handle( DirectiveProto d ) {
        recordRef(d,d.name());
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
    }
     void handle( DirectiveSymbol d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
        recordRef(d,d.init());
    }
    void handle( DirectiveImage d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
    }
    void handle( DirectiveSampler d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
    }
    void handle( DirectiveFunctionCommon d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        recordRef(d,d.name());
        recordRefFirstScoped(d,d.firstScopedDirective());
        recordRefNextDirective(d,d.nextDirective());
        // firstInParam is directly referenced.
        // all other params introduce implicit "used-by" connections.
        if (d.inParamCount() > 0) {
            recordRef(d,d.firstInParam());
            Directive d2 = d.firstInParam().next();
            for (unsigned i = 1; i < d.inParamCount(); ++i) {
              recordRefImplicit(d,d2);
              d2 = d2.next();
            }
        }
        if (d.outParamCount() > 0) {
            Directive d2 = d.next();
            for (unsigned i = 0; i < d.outParamCount(); ++i) {
              recordRefImplicit(d,d2);
              d2 = d2.next();
            }
        }

    }
    void handle( ArgStart d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #else
        (void)d;
        #endif
    }
    void handle( ArgEnd d ) {  
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #else
        (void)d;
        #endif
    }
    void handle( DirectiveLabelInit d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        LabelList labels = d.labels();
        for(unsigned i=0; i<labels.size(); ++i) { 
            recordRef(d,labels[i]);
        }
    }
    void handle( DirectiveLabelList d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code());
        #endif
        LabelList labels = d.labels();
        for(unsigned i=0; i<labels.size(); ++i) {
            recordRef(d,labels[i]); 
        }
    }

    // instructions
    void handle( Inst i ) {
        for(int o=0; o<5; ++o) {
            recordRef(i,i.operand(o));
        }
    }

    // debug items
    void handleInDebugSection( BlockStart d ) {
        #ifdef CODE_BACKREFS
        recordRef(d,d.code(),true);
        #endif
        recordRef(d,d.name(),true);
    }
    void handleInDebugSection( BlockString d ) {
        recordRef(d,d.name(),true);
    }
    void handleInDebugSection( BlockNumeric ) { // nothing to do
    }
    void handleInDebugSection( BlockEnd ) { // nothing to do
    } 
};

}; //namespace;

template <typename Item>
static void recordRefsFromSection(
  HSAIL_ASM::BrigSection<Item>& s,
  XRecordRefs& recorder)
{
  using namespace HSAIL_ASM;
  for (Item i = s.begin(), e = s.end(); i != e; i = i.next()) {
    visit(i,recorder);
  }
  return;
}

static void recordRefs(HSAIL_ASM::BrigContainer& c,BackRefs& br)
{
  using namespace HSAIL_ASM;
  XRecordRefs recorder(br);
  recordRefsFromSection(c.insts(),recorder);
  recordRefsFromSection(c.operands(),recorder);
  recordRefsFromSection(c.directives(),recorder);
  recordRefsFromSection(c.debugChunks(),recorder);
  return;
}

static void append(HSAIL_ASM::BrigContainer& to, HSAIL_ASM::BrigContainer& from);

template <typename Item>
static void appendSection(
  HSAIL_ASM::BrigSection<Item>& to,
  HSAIL_ASM::BrigSection<Item>& from,
  const AppendRelocator& appendRelocator)
{
  using namespace HSAIL_ASM;
  to.bufferImpl().reserve(to.size() + from.size() - from.begin().brigOffset());
  for (Item i = from.begin(), e = from.end(); i != e; i = i.next()) {
    Item copiedItem = to.append(i);
    visit(copiedItem,appendRelocator);
  }
}

class AppendContext {
public:
    struct Context {
        Offset base4code;
        Offset base4dir;
        BrigContainer * from; // input brig container for linking
    };
    typedef std::map< const char*, Context> Map;
    Map map;
private:
    AppendContext(const AppendContext&);
    const AppendContext& operator=(const AppendContext&);
public:
    AppendContext() {
        for (unsigned i = 0; i < InputFilenames.size(); ++i) {
            Context cx;
            cx.from = new HSAIL_ASM::BrigContainer;
            assert(cx.from);
            map[InputFilenames[i].c_str()] = cx;
        }
    }
    ~AppendContext() { // automatic memory deallocation on exit from main()
        for (unsigned i = 0; i < InputFilenames.size(); ++i) {
            Context cx = map[InputFilenames[i].c_str()];
            if (cx.from) delete cx.from;
        }
    }
};

static void append(
    HSAIL_ASM::BrigContainer& to
    ,/* TODO_HSA (3) const */ HSAIL_ASM::BrigContainer& from
    ,Modules& modules
    ,const char* file
    ,AppendContext::Context& cx)
{
    using namespace HSAIL_ASM;
    AppendRelocator const appendRelocator(to,modules,file);
    
    // append entire string section
    to.strings().insertData(
    to.strings().size(),
    from.strings().getData(from.strings().NUM_BYTES_RESERVED),
    from.strings().getData(from.strings().size()));
    
    appendSection(to.directives(),from.directives(),appendRelocator);
    appendSection(to.operands(),from.operands(),appendRelocator);
    appendSection(to.insts(),from.insts(),appendRelocator);
    
    if (linkDwarf) {
        // Remember context to postpone dwarf linking until brig linking/optimization done.
        cx.base4code = appendRelocator.getCodeOffset();
        cx.base4dir  = appendRelocator.getDirectivesOffset();
    }
    return;
}

static int read(HSAIL_ASM::BrigContainer& c, const char *filename)
{
  using namespace HSAIL_ASM;
  if (BifInput
  ?  BifStreamer::load(c,filename)
  : BrigStreamer::load(c,filename)) {
    errs() << "Error: Can't read input file: '" << filename << "'.\n";
    errs().flush();
    return 3;
  }
  if (!DisableValidatorInput) {
    if (ValidateContainer(c, NULL, false)) {
      errs()<< "Error: Invalid/corrupt input file or wrong file format"
            << " (" << (BifInput ? ASCIZ_FILE_FORMAT_BRIG : ASCIZ_FILE_FORMAT_BIF) << "?):"
            << " '" << filename << "'.\n";
      errs().flush();
      return 2;
    }
  }
  return 0;
}

// Read all input BRIG files.
// Concatenate them into single container.
static int Concatenate(
     HSAIL_ASM::BrigContainer& bc
    ,Modules& modules
    ,BackRefs& br
    ,AppendContext& ac)
{
    using namespace HSAIL_ASM;
    if (DebugParser) {
        outs() << "Parsing...\n";
        outs().flush();
    }
    for (unsigned i = 0; i < InputFilenames.size(); ++i) {
        const char * const file = InputFilenames[i].c_str();
        AppendContext::Context& cx = ac.map[file];
        assert(cx.from);
        if (read(*cx.from,file)) { return 1; }
        append(bc,*cx.from,modules,file,cx);
    }
    modules.construct(bc);
    if (!modules.linkable()) {
        errs() << "Error: Incompatible versions of input modules\n"; errs().flush();
        modules.print(errs());
        return 1;
    }
    recordRefs(bc,br);
    return 0;
}

// Read HSA dwarf info (and only it) from debug section and merge it 
// into DwarfLinker object instance, processing relocations etc. 
// Actual linking of dwarf info is performed here.
static void append(
     HSAIL_ASM::BrigContainer& from
    ,DebugInformationLinking::DwarfLinker& dl
    ,const char* fromFileName
    ,const AppendContext::Context& ac
    ,const std::vector<BrigcOffset>& com
    ,const std::vector<BrigdOffset>& dom)
{
    using namespace DebugInformationLinking;
    assert(fromFileName);
    if (linkDwarf) {
        if(printBRIGSectionsOffsets) {
            printf("offsets for %s: c_offset %u d_offset %u\n", fromFileName, ac.base4code, ac.base4dir);
        }
        std::vector<unsigned char> dwarf;
        loadDwarfDebugData(from, dwarf);
        if(dwarf.size() > 0) {
            DWARF_LINKER_TRACE("adding %u DWARF bytes [c_offset = %u, d_offset = %u]\n", dwarf.size(), ac.base4code, ac.base4dir);
            if(!dl.appendDwarfElfData(dwarf, ac.base4code, ac.base4dir, com, dom)) {
                DWARF_LINKER_TRACE("failed to add DWARF bytes\n");
            }
            else {
                DWARF_LINKER_TRACE("DWARF bytes have been successfully added\n");
            }
        }
        else {
            DWARF_LINKER_TRACE("no DWARF data found\n" );
        }
    }
    return;
}

// Link dwarf info from input containers.
static void ConcatenateDwarf(
     DebugInformationLinking::DwarfLinker& dl
    ,AppendContext& ac
    ,const std::vector<BrigcOffset>& com
    ,const std::vector<BrigdOffset>& dom)
{
    using namespace HSAIL_ASM;
    for (unsigned i = 0; i < InputFilenames.size(); ++i) {
        const char * const file = InputFilenames[i].c_str();
        AppendContext::Context cx = ac.map[file];
        assert(cx.from);
        append(*cx.from,dl,file,cx,com,dom);
    }
}

static inline
bool IsDeclaration(DirectiveFunctionCommon& d) {
    assert(d.code().brigOffset() == UNUSED_OFFSET ? (d.operationCount() == 0) : true);
    return (d.operationCount() == 0);
}
static inline bool IsDeclaration(DirectiveSymbolCommon&  d) { return d.attribute() == BrigExtern; }

static inline bool IsGlobal(DirectiveFunctionCommon& d) { return d.attribute() != BrigStatic; }
ASSUMPTION("only symbols/sobs/images whose names start with '&' may be global");
static inline
bool IsGlobal(DirectiveSymbolCommon& d) {
    return (d.attribute() != BrigStatic &&  *static_cast<const char*>(d.name()) == '&');  
}

namespace {
using namespace HSAIL_ASM;

class IsDeclaration2 : public handlers4DebugSectionNA
{
    mutable bool rv_;
    IsDeclaration2(const IsDeclaration2&);
    const IsDeclaration2& operator=(const IsDeclaration2&);
public:
    IsDeclaration2() {}
    bool operator()(const Directive& d) {
        HSAIL_ASM::visit(d,*this);
        return rv_;
    }
    void handle( Directive                 ) const { rv_ = false; assert(!"wrong use of IsDeclaration"); }
    void handle( DirectiveFunctionCommon d ) const { rv_ = IsDeclaration(d); }
    void handle( DirectiveSymbolCommon   d ) const { rv_ = IsDeclaration(d); }
};

class GetIdentifier : public handlers4DebugSectionNA
{
    mutable std::string rv_;
    GetIdentifier(const GetIdentifier&);
    const GetIdentifier& operator=(const GetIdentifier&);
public:
    GetIdentifier() {}
    std::string operator()(const Directive& d) {
        HSAIL_ASM::visit(d,*this);
        return rv_;
    }
    void handle( Directive                 ) const { rv_ = "<bad identifier>"; assert(!"wrong use of GetName"); }
    void handle( DirectiveFunctionCommon d ) const { rv_ = d.name(); }
    void handle( DirectiveSymbolCommon   d ) const { rv_ = d.name(); }
    void handle( DirectiveLabel          d ) const { rv_ = d.name(); }
    void handle( DirectiveProto          d ) const { rv_ = d.name(); }
};

class GetKind : public handlers4DebugSectionNA
{
    std::string rv_;
    GetKind(const GetKind&);
    const GetKind& operator=(const GetKind&);
public:
    GetKind() {}
    std::string operator()(const Directive& d) {
        HSAIL_ASM::visit(d,*this);
        return rv_;
    }
    void handle( const Directive         ) { rv_ = "<kind n/a>"; }
    void handle( const DirectiveFunction ) { rv_ = "function"; }
    void handle( const DirectiveKernel   ) { rv_ = "kernel"; }
    void handle( const DirectiveSymbol   ) { rv_ = "symbol"; }
    void handle( const DirectiveImage    ) { rv_ = "image object"; }
    void handle( const DirectiveSampler  ) { rv_ = "sampler object"; }
    void handle( const DirectiveLabel    ) { rv_ = "label"; }
    void handle( const DirectiveProto    ) { rv_ = "signature"; }
};

class GetQualifiedType : public handlers4DebugSectionNA
{
public:
    typedef enum { DEFAULT = 0, NO_EXTERNS = 0x01 } Mode;
private:
    mutable std::string rv_;
    const Mode mode_;
    std::string typeInfo(DirectiveSymbolCommon& d) const {
        // example: "align 8 static const global_u32 [123]"
        std::ostringstream s;
        if (d.align() > 1) { s << "align " << d.align() << " "; }
        if (d.attribute() != BrigExtern || !(mode_ & NO_EXTERNS)) {
            const char * str = HSAIL_ASM::attribute2str(d.attribute());
            if (str) { if (strlen(str) > 0) s << str << " "; }
            else { s << "UNKNOWN_LINKAGE "; assert(!"Unknown linkage"); }
        }
        if (d.symbolModifier() & BrigConst) { s << "const "; }
        const char * str = HSAIL_ASM::storageClass2str(d.storageClass());
        if (str) { s << str; } else { s << "UNKNOWNSTORAGECLASS"; assert(!"Unknown storage class"); }
        s << "_" << HSAIL_ASM::dataType2str(d.type());
        const bool isArray = (d.symbolModifier() & BrigArray) ? true : false;
        if (isArray) s << " [";
        if (d.dim() != 0) { s << d.dim(); }
        if (isArray) s << "]";
        return std::string(s.str());
    }
    // Composes type info for the list of input (or output) params of a function.
    //
    // @param p       : first input (or output) param of the function.
    // @param n_param : number of params.
    //
    std::string typeInfoParams(Directive p, const unsigned n_param) const {
        std::ostringstream s;
        for (unsigned i = 0; i < n_param; ++i, p = p.next()) 
        {
            if (i > 0) { s << ", "; }
            DirectiveSymbolCommon ps = p;
            assert(ps && "Unexpected kind of function parameter");
            if (!ps) {
                s << "UNEXPECTED_KIND";
            }
            else {
                s << typeInfo(ps);
            }
        }
        return std::string(s.str());
    }
    GetQualifiedType(const GetQualifiedType&);
    const GetQualifiedType& operator=(const GetQualifiedType&);
public:
    GetQualifiedType(const Mode mode) : mode_(mode) {}
    std::string operator()(const Directive& d) const {
        HSAIL_ASM::visit(d,*this);
        return rv_;
    }
    void handle( Directive           ) const { rv_ = "<qualified type n/a>"; }
    void handle( DirectiveSymbol   d ) const { rv_ = typeInfo(d); }
    void handle( DirectiveImage    d ) const { rv_ = typeInfo(d); }
    void handle( DirectiveSampler  d ) const { rv_ = typeInfo(d); }
    void handle( DirectiveFunction d ) const {
        rv_  = "(";
        rv_ += typeInfoParams(d.next(),d.outParamCount());
        rv_ += ")(";
        rv_ += typeInfoParams(d.firstInParam(),d.inParamCount());
        rv_ += ")";
    }
    void handle( DirectiveKernel d ) const {
        rv_  = "(";
        rv_ += typeInfoParams(d.firstInParam(),d.inParamCount());
        rv_ += ")";
    }
};

// DefinitionFinder(decl).isMatchingDefinition(dir) returns true
// if dir is a matching definition for decl.
class DefinitionFinder
{
private:
    template <typename DIRECTIVE_T>
    class IsMatch
    {
        const DIRECTIVE_T d1_;
    private:
        // "align 1" means natural alignment.
        uint16_t actualAlignment(DirectiveSymbolCommon& d) const {
            if (d.align() > 1) {
                return d.align();
            }
            switch (d.type()) {
            case BrigROImg: 
            case BrigRWImg:
            case BrigSamp:
                return 16; // this is not well-defined in the spec yet. 
            default: // all other types
                int size_bits = HSAIL_ASM::brigtype_get_length(d.type());
                if (size_bits == 1) { return 1; } // b1
                if (size_bits%8 != 0) break; // not divisable by 8 => strange, default case
                return size_bits/8; // natural alignment
            }
            return 1; // default case
        }
        // Returns true when fully qualified type (i.e. storage class, modifier, type etc)
        // of declaration and definition of variable match, so resolving of declaration
        // to definition is allowed. Notice dedicated mode for comparison of
        // function params.
        bool isTypeInfoMatch(
             DirectiveSymbolCommon &decl
            ,DirectiveSymbolCommon &d
            ,const bool modeCompareFunctionParams
                // [1] When types of function params are being compared, exact match in required. 
                // [2] However, the matching conditions shall be relaxed for items which are not
                // function params. Specifically, decl of flex array shall match definition 
                // of array of any size (as well as definition of flex array, of course).
        ) const {
            // If function params are being compared, works OK even with definitions.
            // Otherwise, we have to check pre-condition that "decl" is really a declaration:
            assert(modeCompareFunctionParams || IsDeclaration(decl));
            // Note: BrigFlex, BrigConst and BrigArray are bit flags in modifier.
            const bool isDeclFlex = (decl.symbolModifier() & BrigFlex) ? true : false;
            return decl.storageClass() == d.storageClass()
                && (decl.symbolModifier() & BrigConst) == (d.symbolModifier() & BrigConst)
                && (decl.symbolModifier() & BrigArray) == (d.symbolModifier() & BrigArray)
                && (modeCompareFunctionParams
                    ? ((d.symbolModifier() & BrigFlex) == (d.symbolModifier() & BrigFlex))
                    : ((d.symbolModifier() & BrigFlex) ? isDeclFlex : true) ) // see [2] above
                && (modeCompareFunctionParams
                    ? (decl.dim() == d.dim())
                    : (decl.dim() == d.dim() || isDeclFlex )) // see [2] above
                      // If array decl is flex, dim of array definition can be any.
                      // Validator ensures that flex arrays have dim == 0.
                && decl.type() == d.type()
                && actualAlignment(decl) == actualAlignment(d);
        }
        inline bool isTypeInfoMatch(DirectiveSymbol &d1,DirectiveSymbol &d) const {
            return isTypeInfoMatch(d1,d,false);
        }
        inline bool isTypeInfoMatch(DirectiveImage &d1,DirectiveImage &d) const {
            return isTypeInfoMatch(d1,d,false);
            // Image properties (width...format) do not constitute type info
            // and, therefore, are not taken into account during linking.
        }
        inline bool isTypeInfoMatch(DirectiveSampler &d1,DirectiveSampler &d) const {
            return isTypeInfoMatch(d1,d,false);
            // Sampler properties (e.g. coordinates) do not constitute type info.
            // and, therefore, are not taken into account during linking.
        }
        // Compares lists of input (or output) params of two functions.
        // Pre-requisites: The number of params must be the same in both functions.
        //
        // @param d1      : first input (or output) param of the 1st function.
        // @param d2      : first input (or output) param of the 2nd function.
        // @param n_param : number of params.
        //
        bool isMatchParams(Directive d1, Directive d2, unsigned n_param) const {
            for (; n_param > 0; --n_param, d1 = d1.next(), d2 = d2.next()) 
            {
                DirectiveSymbolCommon ds1 = d1;
                DirectiveSymbolCommon ds2 = d2;
                assert(ds1 && ds2 && "Unexpected kind of function parameter");
                if (!(ds1 && ds2)) {
                    errs() << "FATAL: Unexpected kind of function parameter\n";
                    errs().flush();
                    return false;
                }
                if (!isTypeInfoMatch(ds1,ds2,true)) { return false; }
            }
            return true;
        }
        bool isTypeInfoMatch(DirectiveFunctionCommon& d1, DirectiveFunctionCommon& d) const {
            const unsigned in_pc = d.inParamCount();
            const unsigned out_pc = d.outParamCount();
            if (!(d1.inParamCount() == in_pc && d1.outParamCount() == out_pc)) {
                return false;
            }
            return isMatchParams(d1.firstInParam(),d.firstInParam(),in_pc)
                && isMatchParams(d1.next(),d.next(),out_pc); // out params
        }
        template <typename OTHER_DIRECTIVE_T>
        bool isIdentifieAndLinkageOfDefinitionMatch(const Directive& d) const {
            OTHER_DIRECTIVE_T d2 = d;
            DIRECTIVE_T d1(d1_); // hack to solve problem with constness
            return IsGlobal       (d2) == IsGlobal(d1)
                && !IsDeclaration (d2)
                && !strcmp        (d2.name(),d1.name());
        }
        IsMatch(const IsMatch& r);
        const IsMatch& operator=(const IsMatch& r);
    public:
        IsMatch(DIRECTIVE_T& d) : d1_(d)
        {}
        bool identifierAndLinkageOfDefinition(const Directive& d) const {
            if (d.brig()->kind == BrigEDirectiveSymbol
            ||  d.brig()->kind == BrigEDirectiveSampler
            ||  d.brig()->kind == BrigEDirectiveImage) {
                return isIdentifieAndLinkageOfDefinitionMatch<DirectiveSymbolCommon>(d);
            }
            else if (d.brig()->kind == BrigEDirectiveFunction
            ||       d.brig()->kind == BrigEDirectiveKernel) {
                return isIdentifieAndLinkageOfDefinitionMatch<DirectiveFunctionCommon>(d);
            }
            return false;
        }
        bool typeInfo(const Directive& d) const {
            if (d1_.brig()->kind == d.brig()->kind) {
                DIRECTIVE_T d2 = d;
                DIRECTIVE_T d1(d1_); // hack to solve problem with constness
                return isTypeInfoMatch(d1,d2);
            }
            return false;
        }
    };

private:    
    BrigContainer& bc_;
    const Modules& modules_;
    const bool library_mode_;

public:
    DefinitionFinder(const DefinitionFinder& r);
    const DefinitionFinder& operator=(const DefinitionFinder& r);
    DefinitionFinder(BrigContainer& bc, const Modules& modules, const bool library_mode)
      : bc_(bc), modules_(modules), library_mode_(library_mode)
    {}

    typedef std::vector< Directive > MatchingGlobalIdentifiers;

    template <typename DIRECTIVE_T>
    BrigdOffset find(DIRECTIVE_T& d, MatchingGlobalIdentifiers& matchingGlobalIdentifiers) const {
        matchingGlobalIdentifiers.clear();
        BrigdOffset def_offset = UNUSED_OFFSET;
        const Modules::const_iterator module = modules_.find(d);
        assert(module != modules_.end() && "No version found before directive.");
        const IsMatch<DIRECTIVE_T> isMatch(d);
        Modules::const_iterator m, m_end;
        if (!IsGlobal(d)) { // lookup only in one module
            m     = module;
            m_end = module;
            ++m_end;
        }
        else {
            m     = modules_.begin();
            m_end = modules_.end();
        }
        for (; m != m_end; ++m) {
            if (! modules_.isCompatible(module,m)) {
                continue; 
            }
            Directive i    (&bc_,modules_.beginOffset(m));
            Directive i_end(&bc_,modules_.endOffset(m));
            for (; i != i_end; i = i.next()) {
                if (isMatch.identifierAndLinkageOfDefinition(i)) {
                    if (IsGlobal(d)) {
                        matchingGlobalIdentifiers.push_back(i);
                    }
                    if (IsDeclaration(d)) { // finding matching definitions is required only for _declarations_.
                        if (isMatch.typeInfo(i)) {
                            def_offset = i.brigOffset();
                        }
                    }
                    // As soon as a name is found in a module, we can skip lookup 
                    // in the rest of that module. This is possible because it is guaranteed 
                    // that a module can NOT contain more than 1 (one) definition with the same name.
                    // This is claimed in the spec and shall be supported by Brig Validator. Therefore:
                    break;
                }
            }
            // Even if definition is found, continue lookup in other modules
            // to diagnose multiple definitions
        }
        return def_offset;
    }
};

class DeclResolver : public handlers4DebugSectionNA
{
    BrigContainer& bc_;
    BackRefs& br_;
    const Modules& modules_;
    const struct Mode {
        bool build_library;
        bool detach_resolved_declarations;
        Mode(const bool x, const bool y) : build_library(x), detach_resolved_declarations(y) {}
    }
    mode_;
    DefinitionFinder defFinder_;
    int cumulative_rc_;

    void rebindReferences(const BrigdOffset oldDest, const BrigdOffset newDest) {
        if (mode_.build_library && !mode_.detach_resolved_declarations) {
            // Library mode requires special processing:
            // Do NOT rebind references from operands
            ;
            // Do NOT update references from directives
            ;
            // Create implicit backRef from *def* to decl to prevent de-deading of *def*.
            BackRefs::TypedReferrer def2decl = {0};
            def2decl.base = oldDest;
            def2decl.type = BackRefs::TYPE_IMPLICIT;
            br_.dir2dir[newDest].push_back(def2decl);
        }
        else {
            // rebind brig offsets and backRefs from old to new destination
            br_.rebindDirectiveUsers(bc_,oldDest,newDest);
            // Create implicit backRef from resolved decl to def to prevent de-deading of decl.
            if (!mode_.detach_resolved_declarations) {
                // TIP: If definition precedes its "user" AND && definition resides
                // in the same module as its "user", then we can remove declaration. 
                // However, it is not clear how this optimization can be implemented: 
                // the "user" is an operand so it is nontrivial to find out if definition 
                // precedes the "user" or not.
                BackRefs::TypedReferrer decl2def = {0};
                decl2def.base = newDest;
                decl2def.type = BackRefs::TYPE_IMPLICIT;
                br_.dir2dir[oldDest].push_back(decl2def);
            }
        }
    }

    std::string description_(const Directive& d) const {
        std::string rv = "'";
        rv += GetIdentifier()(d);
        rv += "', ";
        rv += GetKind()(d);
        rv += " of type '";
        rv += GetQualifiedType(GetQualifiedType::NO_EXTERNS)(d);
        rv += IsDeclaration2()(d) ? "' declared in '" : "' defined in '";
        rv += modules_.belongsToModule(d);
        rv += "'";
        return rv; // efficiency does not matter here
    }

    typedef std::set<std::string> MultipleGlobalsReported;
    MultipleGlobalsReported multipleGlobalsReported_; // this is to report a set of conflicting names once

    void errsReportMultipleDefs(
         const char* identifier
        ,DefinitionFinder::MatchingGlobalIdentifiers& defs
        ,const std::string& headline
    ) {
        std::pair<MultipleGlobalsReported::iterator, bool> result
            = multipleGlobalsReported_.insert(identifier); 
        if (!result.second) { // not inserted (already in container) => already reported
        }
        else {
            errs() << headline;
            for (DefinitionFinder::MatchingGlobalIdentifiers::const_iterator 
                p = defs.begin(), p_end = defs.end(); p != p_end; ++p)
            {
                errs() << "..." << description_(*p) << ".\n";
            }
            errs().flush();
        }
    }

    template <typename DIRECTIVE_T>
    void resolveDeclaration(DIRECTIVE_T& d) {
        if (!IsDeclaration(d)) { return; } 
        if (!br_.isUsedByOperand(d)) { return; } // do NOT resolve unused decl
        DefinitionFinder::MatchingGlobalIdentifiers matchingGlobals;
        BrigdOffset def = defFinder_.find(d,matchingGlobals);
        if (def == UNUSED_OFFSET) {
            if (!IsGlobal(d)) {
                cumulative_rc_ |= 8;
                errs() << "FATAL: Unresolved local declaration: " << description_(d) << ".\n";
            }
            else {
                if (mode_.build_library) {
                    // unresolved globals are OK for libs
                    if (matchingGlobals.size() > 0) { // non-matching def(s) are still possible
                        cumulative_rc_ |= 4;
                        errs() << "Error: Non-matching definition(s) of " << description_(d) << ":\n";
                        errs().flush();
                    }
                }
                else {
                    cumulative_rc_ |= 1; // unresolved extern
                    errs() << "Error: Unresolved " << description_(d) << ".\n";
                    errs().flush();
                }
                if (matchingGlobals.size() > 0) {
                    std::string headline = "...Found item(s) with the same name but wrong type:\n";
                    errsReportMultipleDefs(d.name(),matchingGlobals,headline);
                }
            }
        }
        else {
            if (matchingGlobals.size() > 1) {
                cumulative_rc_ |= 1; // force "unresolved" error
                std::string headline = "Error: Unresolved " + description_(d) + " due to name conflict between:\n";
                errsReportMultipleDefs(d.name(),matchingGlobals,headline);
            }
            else {
                rebindReferences(d.brigOffset(),def);
            }
        }
    }

    // HSAIL spec 1.0 (Jun 19):
    // "4.8.2 ...HSAIL uses a single name space. Thus, it is not valid to have 
    // functions, kernels, or global variables with the same name."
    //
    // If more than one global definition with the same name (identifier) found,
    // then issue an error message. 
    //
    template <typename DIRECTIVE_T>
    void checkMultipleGlobalDefinitions(DIRECTIVE_T& d) {
        if (IsDeclaration(d)) { return; } 
        if (!IsGlobal(d)) { return; } // no need to check local-in-compilation-unit identifiers
                                      // as multiple local defs shall not appear in valid brig
                                      // and we have brig validator on input.
        DefinitionFinder::MatchingGlobalIdentifiers matchingGlobals;
        (void)defFinder_.find(d,matchingGlobals); // we are not interested in _resolving_ here
        if (matchingGlobals.size() > 1) {
            cumulative_rc_ |= 2;
            std::string headline = "Error: Multiple definitions with conflicting names found:\n";
            errsReportMultipleDefs(d.name(),matchingGlobals,headline);
        } 
    }

    DeclResolver(const DeclResolver& r);
    const DeclResolver& operator=(const DeclResolver& r);
public:
    DeclResolver(BrigContainer& bc, BackRefs& br, Modules& modules, const bool library_mode, const bool detach_decl) 
        : bc_(bc), br_(br), modules_(modules), mode_(library_mode,detach_decl)
        , defFinder_(bc,modules,library_mode), cumulative_rc_(0), multipleGlobalsReported_()
    {}
    
    int getCumulativeRc() { return cumulative_rc_; };
  
    // directives
    void handle(Directive          ) { }
    void handle(DirectiveSymbol   d) {
        resolveDeclaration(d);
        checkMultipleGlobalDefinitions(d);
    }
    void handle(DirectiveImage    d) {
        resolveDeclaration(d);
        checkMultipleGlobalDefinitions(d);
    }
    void handle(DirectiveSampler  d) {
        resolveDeclaration(d);
        checkMultipleGlobalDefinitions(d);
    }
    void handle(DirectiveFunction d) {
        resolveDeclaration(d);
        checkMultipleGlobalDefinitions(d);
    }
    void handle(DirectiveKernel   d) {
        checkMultipleGlobalDefinitions(d);
    }
};

// Resolve declarations to definitions.
// For declarations of global entities, looks for corresponing definition in the whole BRIG.
// For local-in-compilation-unit declarations, looks in the corresponding compilation unit only.
int ResolveDeclarations(HSAIL_ASM::BrigContainer& bc, BackRefs& br, Modules& modules)
{
    DeclResolver resolver(bc,br,modules,CreateLibrary,DebugRemoveDeclarations);
    for (Directive i = bc.directives().begin(), end_ = bc.directives().end(); i != end_; i = i.next()) {
        visit(i,resolver);
    }
    return resolver.getCumulativeRc();
}

class StringValueLess { 
    const char * begin_;
public:
    StringValueLess(const BrigContainer& bc) : begin_(bc.strings().getData(0)) { }
    bool operator()(BrigsOffset a, BrigsOffset b) { 
        return strcmp(begin_+a, begin_+b) < 0;
    }
};

void DeDupeStrings(BrigContainer& bc, BackRefs& br)
{
    if (DebugDeDuper) {
        printf("  .strtab\n");
        fflush(stdout);
    }
    typedef std::set<BrigsOffset, StringValueLess> StringDictionary;
    StringDictionary originals(bc);
    // For each item, check if it is in the "originals" dictionary yet.
    // If it is, then this item is a duplicate. Rebind all its users to the original.
    // Otherwise, add this item as a new original to the dictionary.
    for(BrigsOffset s = bc.strings().NUM_BYTES_RESERVED; s < bc.strings().size(); s += (strlen(bc.strings().getData(s))+1)) {
        std::pair<StringDictionary::iterator, bool> result = originals.insert(s); 
        if (!result.second) { // not inserted (already exists); iterator denotes existing item (original)
            if (DebugDeDuper) {
                printf("    Orig: %4u Dupe: %4u.\n", static_cast<unsigned>(*(result.first)), static_cast<unsigned>(s));
                fflush(stdout);
            }
            br.rebindStringUsers(bc,s,*(result.first));
        }
    }
}

// map: original -> vector of duplicates
class OperandValueLess { 
    // compare values of operands (of certain supported kinds)
    // otherwise compare offsets.
    //
    // OPTIMIZATION TIP: If deDuping of operands is performed _after_ deDuping of strings,
    // then we can compare brig offsets instead of strcmp() for names. However, this would
    // introduce additional implicit dependence: OperandValueLess()() works properly only
    // in the context which has strings deDuped. Therefore, let's implement such an optimization 
    // not now but when/if need arise.
    // 
    static bool equal(OperandReg a, OperandReg b) {
        return strcmp(a.name(), b.name()) == 0;
    }
    static bool less(OperandReg a, OperandReg b) {
        return strcmp(a.name(), b.name()) < 0;
        // no need to check type assuming that brig is valid
    }
    static bool less(OperandImmed a, OperandImmed b) {
        // Possible values of .type are not well-defined in the HSAIL spec 1.0.
        // Let's use the simplest (and straightforward) approach: memcmp.
        if (a.type() != b.type()) {
            return a.type() < b.type();
        }
        return memcmp(&a.brig()->bits,&b.brig()->bits,sizeof(a.brig()->bits)) < 0;
    }
    static bool less(OperandIndirect a, OperandIndirect b) {
        if (a.type() != b.type()) {
            return a.type() < b.type();
        }
        // type is the same, go ahead...
        // HSAIL spec allows reg offset == 0 if no register is used
        const bool isRegA = a.reg();
        const bool isRegB = b.reg();
        if (isRegA != isRegB) {
            return isRegB; // no register in a => a < b
        }
        // register existence is the same, go ahead...
        if (isRegA) { // registers in both a and b
            OperandReg areg(a.reg());
            OperandReg breg(b.reg());
            if (!equal(areg,breg)) {
                return less(areg,breg);
            }
        }
        // registers are the same (OR none registers), go ahead...
        return a.offset() < b.offset();
    }
public:
    bool operator()(Operand a, Operand b) { 
        unsigned ak = a.brig()->kind;
        unsigned bk = b.brig()->kind;
        if (ak != bk) {
            return ak < bk;
        }
        switch(ak) {
        case BrigEOperandReg:
            return less(OperandReg(a), OperandReg(b));
        case BrigEOperandImmed:
            return less(OperandImmed(a), OperandImmed(b));
        case BrigEOperandIndirect:
            return less(OperandIndirect(a), OperandIndirect(b));
        default:
            return a.brigOffset() < b.brigOffset();
        }
    }
};

typedef std::map< Operand, std::vector<Operand>, OperandValueLess> OperandDictionary;

void DeDupeOperands(HSAIL_ASM::BrigContainer& bc, BackRefs& br)
{
    if (DebugDeDuper) {
        printf("  .operands\n");
        fflush(stdout);
    }
    OperandDictionary originals;
    // For each operand, check if it is in the "originals" dictionary yet.
    // If it is, then this operand is a duplicate. Add it to the original's vector of duplicates.
    // Otherwise, add this operand as a new original to the dictionary.
    for(Operand o = bc.operands().begin(); o != bc.operands().end(); o = o.next()) {
        std::pair<OperandDictionary::iterator, bool> result
            = originals.insert(make_pair(o, std::vector<Operand>())); 
            // result.first: iterator
            // result.second: true if new element inserted
        if (!result.second) {
            result.first->second.push_back(o);
        }
    }
    // Iterate over all originals
    for(OperandDictionary::iterator p = originals.begin(); p != originals.end(); ++p) {
        Operand orig = p->first;
        // rebind all dupe references to point to originals
        for(std::vector<Operand>::const_iterator
        dupe = p->second.begin(), dupe_end = p->second.end(); dupe != dupe_end; ++dupe) {
            if (DebugDeDuper) {
                printf("    Orig: %4u Dupe: %4u.\n", (unsigned)orig.brigOffset(), (unsigned)dupe->brigOffset());
                fflush(stdout);
            }
            br.rebindOperandUsers(bc,dupe->brigOffset(),orig.brigOffset());
        }
    }
}

}; //namespace

struct FunctionRangeInfo {
  BrigdOffset d_next;
  BrigcOffset c_start; // [start,end[
  BrigcOffset c_end;
};
typedef std::map<BrigdOffset,FunctionRangeInfo> DeadFunctions;
static DeadFunctions deadFunctions;
static inline
bool inDeadFunctions(const Directive& d) {
  return (deadFunctions.find(d.brigOffset()) != deadFunctions.end());
}

typedef std::map<BrigdOffset,BrigdOffset> DirectiveRanges; // Represents [start,end[. NOTE: Key is end.
static DirectiveRanges deadDirectiveRanges;
static inline
bool isInDeadDirectiveRanges(const Directive& d) {
    BrigdOffset doff = d.brigOffset();
    DirectiveRanges::iterator p = deadDirectiveRanges.upper_bound(doff);
    return (p != deadDirectiveRanges.end() && p->second <= doff);
}

typedef std::map<BrigcOffset,BrigcOffset> CodeRanges; // Represents [start,end[. NOTE: Key is end.
CodeRanges deadCodeRanges; // Should not contain [0,0[ ranges
static inline
bool isInDeadCodeRanges(BrigcOffset c) {
    CodeRanges::iterator p = deadCodeRanges.upper_bound(c);
    return (p != deadCodeRanges.end() && p->second <= c);
}

typedef std::set<BrigdOffset> Directives;
static Directives deadDirectives;
static inline
bool inDeadDirectives(const Directive& d) {
  return (deadDirectives.find(d.brigOffset()) != deadDirectives.end());
}

typedef std::set<BrigoOffset> Operands;
static Operands deadOperands;
static inline
bool inDeadOperands(Operand& o) {
  return (deadOperands.find(o.brigOffset()) != deadOperands.end());
}

namespace {

class MustBeKeptEvenIfUnused : public handlers4DebugSectionNA
{
    bool keep_;
    MustBeKeptEvenIfUnused(const MustBeKeptEvenIfUnused&);
    const MustBeKeptEvenIfUnused& operator=(const MustBeKeptEvenIfUnused&);
public:
    MustBeKeptEvenIfUnused() : keep_(false)
    {}
  
    bool operator()(Directive& d) {
        keep_ = false;
        HSAIL_ASM::visit(d,*this); // updates keep_
        return keep_;
    }

    // Always keep these
    void handle( DirectiveVersion ) { keep_ = true; }
    void handle( DirectiveFile    ) { keep_ = true; }
    void handle( DirectiveKernel  ) { keep_ = true; }
    // TODO_HSA (2) The semantic of pragma is determined by the finalizer and isn't known to us.
    // Therefore we can't properly support pragmas. 
    //
    // For example, some pragmas may affect the whole program. Such pragmas shall be kept 
    // regardless of optimizations.
    //
    // Some other pragmas may apply to the next instruction, which is different case:
    // if an instruction is useless and will be optimized out, then pragma (which affects it) 
    // *must* be removed as well. Otherwise (if pragma is kept) pragma will affect some other 
    // instruction, which is wrong.
    ASSUMPTION("Pragmas affect only instructions (this is initial Kannan's implementation)");
    void handle( DirectivePragma d ) { keep_ = !isInDeadCodeRanges(d.code().brigOffset()); }
    // We must keep these directives unless they belong to dead code.
    //
    // TODO_HSA (2) This approach seems suspicious. What if a directive reside in the function,
    // but _after_ the last function's instruction? Note that functions have next-directive field;
    // all directives which reside after DirectiveFunction but before next-directive
    // belong to the function, see next "else if {}".
    void handle( DirectiveLabel d   ) { keep_ = !isInDeadCodeRanges(d.code().brigOffset()); }
    void handle( BlockStart d       ) { keep_ = !isInDeadCodeRanges(d.code().brigOffset()); }
    void handle( DirectiveComment d ) { keep_ = !isInDeadCodeRanges(d.code().brigOffset()); }
    // We must keep DirectiveScope's (ArgStart/End) unless they belong to dead function.
    void handle( ArgStart d ) { keep_ = !isInDeadDirectiveRanges(d); }
    void handle( ArgEnd d   ) { keep_ = !isInDeadDirectiveRanges(d); }
    // We must keep all functions which are not dead.
    void handle( DirectiveFunction d ) {
        keep_ = !inDeadFunctions(d);
    }
    // Keep all global symbols/sobs/images in library mode
    void handle( DirectiveSymbolCommon d ) { keep_ = (CreateLibrary && IsGlobal(d)); }
    // No need to keep these (so these will be optimized out if unused)
    void handle( BlockNumeric  ) { keep_= false; }
    void handle( BlockString   ) { keep_= false; }
    void handle( BlockEnd      ) { keep_= false; }
    void handle( DirectiveInit ) { keep_= false; }
    void handle( DirectiveLabelInit ) { keep_= false; }
    void handle( DirectiveLabelList ) { keep_= false; }
    void handle( DirectiveProto     ) { keep_= false; }
    // [KeepDirectiveControl]
    // <quote "HSAIL spec (Oct 10 2012), 14.6, page 195">
    // 177 The control directives can appear anywhere in the HSAIL code. Multiple control
    // 178 directives can appear in a single compilation unit. The directive applies from the point
    // 179 where it appears until either the end of the compilation unit or the next instance of
    // 180 the same control directive. </quote> 
    //
    // The above quote means that we can't easily remove directiveControl when it reside in the dead
    // function, because it may apply to the next (non-dead) function etc.
    //
    // Implementation of full-blown optimization (i.e. detection of useless ones) for
    // directiveControl seems too complicated for now. Let's simply keep all control directives. 
    // If a directive applies to some dead code fragment (so that c_code denotes useless code),
    // then deDeader will update c_code so it denotes next non-dead chunk of code.
    //
    // Note that if a directiveControl reside at the end of compilation unit (after deDeading),
    // then its c_code field may denote instructions from the next compilation unit. 
    // Hopefully, this fact does NOT mean that directiveControl applies to the next-module instructions
    // (which would be wrong). There is also directiveVersion coming after directiveControl, 
    // and that directiveVersion shall denote next-module instructions, effectively resetting effects
    // of preceding directiveControl. This case is equal to the following hsail:
    //
    //    version 1:0:small;
    //    ...
    //    workgroupspercu 5; // directiveControl at the end of module
    //    version 1:0:small; // this resets effects of previous directiveControl
    //    ...
    //
    void handle( DirectiveControl   ) { keep_= true; }
    // The machinery of how this directive applies to the code is very similar to 
    // directiveControl. See [KeepDirectiveControl] for more info.
    void handle( DirectiveExtension ) { keep_= true; }
    // <quote "HSAIL spec (Oct 10 2012), 19.8.15, page 244"> 1284 The
    // 1285 instructions starting at that offset up to the next BrigDirectiveLoc are assumed
    // 1286 to correspond to the source location defined by this directive. </quote>
    //
    // If directiveLoc denotes dead instruction, then it is useless and may be removed.
    void handle( DirectiveLoc d     ) { 
        keep_ = !isInDeadCodeRanges(d.code().brigOffset());
            // DirectiveLoc is may appear only at the body level (and NOT at the top level).
            // Therefore, in order to keep BRIG valid, we have to remove directiveLoc
            // when body is going to be optimized out:
        keep_ = keep_ && !isInDeadDirectiveRanges(d);
        ASSUMPTION(
            "This case shall never occur as loc will be optimized out:        "
            "    function &dead_func()() {                                    "
            "        ...                                                      "
            "        loc 1 10 0; // loc for the 1st instruction of next func  "
            "    };                                                           "
            "    function &func()() {                                         "
            "        mov_b32 $s4,4;                                           "
            "        ...                                                      "
            "    };                                                           ");
    }
};

} //namespace

class DetectUselessRv {
public:
  int n_useless;
  int n_backrefs_removed;
  DetectUselessRv(const int x, const int y)
    : n_useless(x)
    , n_backrefs_removed(y) 
  {}
};

// Detect useless code, operands, directives and strings.
static
DetectUselessRv DetectUseless(HSAIL_ASM::BrigContainer& bc, BackRefs& br)
{
    int n_useless = 0;
    int n_backrefs_removed = 0;

    // Add useless functions to the dead-list.
    for(Directive d = bc.directives().begin(); d != bc.directives().end(); d = d.next()) {
        if (d.brig()->kind != BrigEDirectiveFunction) { continue; }
        DirectiveFunction fn(d);
        if (!IsDeclaration(fn)) { // definition
            // Useless local functions shall be removed in any case.
            // Useless "global" functions shall be removed when linking into executable.
            bool is_dead = false;
            if (!IsGlobal(fn) || (IsGlobal(fn) && !CreateLibrary)) {
                is_dead = !br.isUsedByOperand(fn);
            }
            if (is_dead && !inDeadFunctions(fn)) {
                ++n_useless;
                Inst inst(fn.code());
                for (uint32_t n = 0; n < fn.operationCount(); ++n) { inst = inst.next(); }
                FunctionRangeInfo fs = { fn.nextDirective().brigOffset()
                                        ,fn.code().brigOffset()
                                        ,inst.brigOffset() };
                deadFunctions.insert(std::make_pair(fn.brigOffset(),fs));
                if (DetailDebugDeDeader) {
                    printf("    +U func d@%u d@%u i@%u i@%u (DEF)\n"
                      ,(unsigned)fn.brigOffset(),fs.d_next,fs.c_start,fs.c_end);
                    fflush(stdout);
                }
            }
        }
        else { // declaration
            // In link-to-library mode, unresolved decl will NOT be added to dead list, 
            // because it is used in 'ldc' or 'call'. In link-to-exe mode,
            // decl shall be implicitly used by defs (as decl is required to keep BRIG valid).
            if (br.isUnused(fn) && !inDeadFunctions(fn)) {
                ++n_useless;
                FunctionRangeInfo fs = { fn.nextDirective().brigOffset(), UNUSED_OFFSET, UNUSED_OFFSET };
                deadFunctions.insert(std::make_pair(fn.brigOffset(),fs));
                if (DetailDebugDeDeader) {
                    printf("    +U func d@%u d@%u i@%u i@%u (DECL)\n"
                      ,(unsigned)fn.brigOffset(),fs.d_next,fs.c_start,fs.c_end);
                    fflush(stdout);
                }
            }
        }
    }
    // DeadFunctionList is ready.
    // Populate deadDirectives, deadDirectiveRanges and deadCodeRanges from deadFunctions.
    for(DeadFunctions::iterator p = deadFunctions.begin(); p != deadFunctions.end() ; ++p) {
        if (!inDeadDirectives(Directive(&bc,p->first))) {
            deadDirectives.insert(p->first);
        }
        if (deadDirectiveRanges.find(p->second.d_next) == deadDirectiveRanges.end()) {
            deadDirectiveRanges.insert(std::make_pair(p->second.d_next,p->first));
        }
        if (p->second.c_start != UNUSED_OFFSET ) { // skip [0,0[ ranges
            if (deadCodeRanges.find(p->second.c_end) == deadCodeRanges.end()) {
                deadCodeRanges.insert(std::make_pair(p->second.c_end,p->second.c_start));
            }
        }
    }
    // If an operand is being used by dead code, do forget such a usage.
    // (remove back refs from operand to dead instructions).
    for(Operand o = bc.operands().begin(); o != bc.operands().end(); o = o.next()) {
        BackRefs::ReferrersMap::mapped_type& codeUsers = br.inst2op[o.brigOffset()];
        for (unsigned int i = 0; i < codeUsers.size(); ++i) {
            BrigcOffset codeUserOffset = codeUsers[i].base + codeUsers[i].offset;
            CodeRanges::iterator p = deadCodeRanges.upper_bound(codeUserOffset);
            if (p != deadCodeRanges.end() && p->second <= codeUserOffset) {
                if (DetailDebugDeDeader) {
                    printf("       oper o@%u -REF from func i@%u i@%u\n"
                      ,(unsigned)o.brigOffset()
                      ,(unsigned)p->second
                      ,(unsigned)p->first);
                    fflush(stdout);
                }
                ++n_backrefs_removed;
                codeUsers.erase(codeUsers.begin() + i);
                --i;
            }
        }
    }
    // If an operand is being used by dead operand, do forget such a usage
    // (remove back refs from operands to dead operands).
    for(Operand o = bc.operands().begin(); o != bc.operands().end(); o = o.next()) {
        BackRefs::ReferrersMap::mapped_type& operandUsers = br.op2op[o.brigOffset()];
        for (unsigned int i = 0; i < operandUsers.size(); ++i) {
            Operands::iterator p = deadOperands.find(operandUsers[i].base);
            if (p != deadOperands.end()) {
              if (DetailDebugDeDeader) {
                  printf("       oper o@%u -REF from dead oper o@%u\n"
                      ,(unsigned)o.brigOffset()
                      ,(unsigned)*p);
                  fflush(stdout);
              }
              ++n_backrefs_removed;
              operandUsers.erase(operandUsers.begin() + i);
              --i;
            }
        }
    }
    // Add all useless operands to the dead-list.
    for(Operand o = bc.operands().begin(); o != bc.operands().end(); o = o.next()) {
        if (!inDeadOperands(o)
        &&  br.isUnused(o))
        {
            ++n_useless;
            deadOperands.insert(o.brigOffset());
            if (DetailDebugDeDeader) {
                printf("    +U oper o@%u\n",(unsigned)o.brigOffset());
                fflush(stdout);
            }
        }
    }
    // If a directive is being used by dead operand, do forget such a usage
    // (remove back refs from directives to dead operands).
    for(Directive d = bc.directives().begin(); d != bc.directives().end(); d = d.next()) {
        BackRefs::ReferrersMap::mapped_type& users = br.op2dir[d.brigOffset()];
        for (unsigned int i = 0; i < users.size(); ++i) {
            Operands::iterator p = deadOperands.find(users[i].base);
            if (p != deadOperands.end()) {
                if (DetailDebugDeDeader) {
                    printf("       dir. d@%u -REF from dead oper o@%u\n",(unsigned)d.brigOffset(),(unsigned)*p);
                    fflush(stdout);
                }
                ++n_backrefs_removed;
                users.erase(users.begin() + i);
                --i;
            }
        }
    }
    // If a directive is being used by dead directive, do forget such a usage
    // (remove backRefs from directives to dead directives).
    for(Directive d = bc.directives().begin(); d != bc.directives().end(); d = d.next()) {
        std::vector<BackRefs::TypedReferrer>& users = br.dir2dir[d.brigOffset()];
        for (unsigned int i = 0; i < users.size(); ++i) {
            Directives::iterator p = deadDirectives.find(users[i].base);
            if (p != deadDirectives.end()) {
                if (DetailDebugDeDeader) {
                    printf("       dir. d@%u -REF from dead dir. d@%u\n",(unsigned)d.brigOffset(),(unsigned)*p);
                    fflush(stdout);
                }
                ++n_backrefs_removed;
                users.erase(users.begin() + i);
                --i;
            }
        }
    }
    // Add useless directives to the dead-list.
    MustBeKeptEvenIfUnused mustBeKeptEvenIfUnused;
    for(Directive d = bc.directives().begin(); d != bc.directives().end(); d = d.next()) {
        BrigdOffset this_ = d.brigOffset();
        if (!inDeadDirectives(d) // don't add if already there
        &&  br.isUnused(d)
        &&  !mustBeKeptEvenIfUnused(d))
        {
            ++n_useless;
            deadDirectives.insert(this_);
            if (DetailDebugDeDeader) {
                printf("    +U dir. d@%u (not eternal and no back refs)\n",this_);
                fflush(stdout);
            }
        }
    }
    // Remove backrefs from dead directives to strings.
    for(BrigsOffset s = bc.strings().NUM_BYTES_RESERVED; s < bc.strings().size(); s += (strlen(bc.strings().getData(s))+1)) {
        BackRefs::ReferrersMap::mapped_type& users = br.dir2str[s];
        for (unsigned i = 0; i < users.size(); ++i) {
            Directives::iterator p = deadDirectives.find(users[i].base);
            if (p != deadDirectives.end()) {
                if (DetailDebugDeDeader) {
                    printf("       str. s@%u -REF from dead dir. d@%u\n",(unsigned)s,(unsigned)*p);
                    fflush(stdout);
                }
                ++n_backrefs_removed;
                users.erase(users.begin() + i);
                --i;
            }
        }
    }
    // Remove backrefs from dead operands to strings.
    for(BrigsOffset s = bc.strings().NUM_BYTES_RESERVED; s < bc.strings().size(); s += (strlen(bc.strings().getData(s))+1)) {
        BackRefs::ReferrersMap::mapped_type& users = br.op2str[s];
        for (unsigned i = 0; i < users.size(); ++i) {
            Operands::iterator p = deadOperands.find(users[i].base);
            if (p != deadOperands.end()) {
                if (DetailDebugDeDeader) {
                    printf("       str. s@%u -REF from dead op. o@%u\n",(unsigned)s,(unsigned)*p);
                    fflush(stdout);
                }
                ++n_backrefs_removed;
                users.erase(users.begin() + i);
                --i;
            }
        }
    }
    return DetectUselessRv(n_useless, n_backrefs_removed);
}

static
BackRefs::TypedReferrer ExtractBackRefToNextDirective(BackRefs& br, Directive& d_this, Directive& d_next)
{
    BackRefs::TypedReferrer rv = {0};
    std::vector<BackRefs::TypedReferrer>& nextBackRefs = br.dir2dir[d_next.brigOffset()];
    for (unsigned i = 0; i < nextBackRefs.size(); ++i) {
        if (nextBackRefs[i].type == BackRefs::TYPE_HINT_NEXT_DIRECTIVE
        &&  nextBackRefs[i].base == d_this.brigOffset()) 
        {
            rv = nextBackRefs[i];
            nextBackRefs.erase(nextBackRefs.begin() + i);
            --i;
            if (DetailDebugDeDeader) {
                printf("    - backRef from next d@%u to func d@%u erased.\n"
                    ,(unsigned)d_next.brigOffset(),(unsigned)d_this.brigOffset());
                fflush(stdout);
            }
            ASSUMPTION("No more than 1 'previous' directive (otherwise, brig is corrupt)");
            break;
        }
    }
    return rv;
}

static
BackRefs::TypedReferrer ExtractBackRefToFirstScoped(BackRefs& br, Directive& d_this, Directive& d_next)
{
    BackRefs::TypedReferrer rv = {0};
    std::vector<BackRefs::TypedReferrer>& nextBackRefs = br.dir2dir[d_next.brigOffset()];
    for (unsigned i = 0; i < nextBackRefs.size(); ++i) {
        if (nextBackRefs[i].type == BackRefs::TYPE_HINT_1ST_SCOPED
        &&  nextBackRefs[i].base == d_this.brigOffset()) 
        {
            rv = nextBackRefs[i];
            nextBackRefs.erase(nextBackRefs.begin() + i);
            --i;
            if (DetailDebugDeDeader) {
                printf("    - backRef from firstScoped d@%u to func d@%u erased.\n"
                    ,(unsigned)d_next.brigOffset(),(unsigned)d_this.brigOffset());
                fflush(stdout);
            }
            ASSUMPTION("No more than 1 'previous' directive (otherwise, brig is corrupt)");
            break;
        }
    }
    return rv;
}

// For non-dead functions and kernels only
template <typename DIRECTIVE_T>
static
void ExcludeDeadHints(BackRefs& br, DIRECTIVE_T d) {
    if (inDeadDirectives(d)) { return; }
    //
    {
        Directive d_next = d.nextDirective();
        if (!inDeadDirectives(d_next)) { } // Next directive is not dead. Nothing to do.
        else { // Next directive is dead.
            // Rebind connection to the next "alive"-directive
            BackRefs::TypedReferrer to_this = ExtractBackRefToNextDirective(br,d,d_next);
            do { d_next = d_next.next(); } while (inDeadDirectives(d_next));
            ASSUMPTION("end-of-directives can not be dead by design.");
            d.nextDirective() = d_next; // d -> alive dir or EOS
            br.dir2dir[d_next.brigOffset()].push_back(to_this); // d <- alive dir or EOS
        }
    }
    {
        Directive d2 = d.firstScopedDirective();
        if (d2.brigOffset() == UNUSED_OFFSET) { } // Field is not used. Nothing to do.
        else if (!inDeadDirectives(d2)) { } // Next directive is not dead. Nothing to do.
        else { // Destination directive is dead.
            // Rebind connection to the next "alive"-directive
            BackRefs::TypedReferrer to_this = ExtractBackRefToFirstScoped(br,d,d2);
            do { d2 = d2.next(); } while (inDeadDirectives(d2));
            d.firstScopedDirective() = d2; // d -> alive dir or EOS
            br.dir2dir[d2.brigOffset()].push_back(to_this); // d <- alive dir or EOS
        }
    }
}

// Exclude dead directives from d_nextDirective->d_nextDirective->... chains.
static
void ExcludeDeadFromHints(HSAIL_ASM::BrigContainer& bc, BackRefs& br) 
{
    for(Directive d = bc.directives().begin(); d != bc.directives().end(); d = d.next()) {
        if (d.brig()->kind == BrigEDirectiveKernel) {
            ExcludeDeadHints(br,DirectiveKernel(d));
        }
        else if (d.brig()->kind == BrigEDirectiveFunction) {
            ExcludeDeadHints(br,DirectiveFunction(d));
        }
        else {
            continue; // we only check directives which have d_next field
        }
    }
}

namespace {

class GetCodeFieldPtr
{
    BrigdOffset32_t* rv_;
    GetCodeFieldPtr(const GetCodeFieldPtr&);
    const GetCodeFieldPtr& operator=(const GetCodeFieldPtr&);
public:
    GetCodeFieldPtr() : rv_(NULL)
    {}
  
    BrigdOffset32_t* operator()(Directive& d) {
        rv_ = NULL;
        HSAIL_ASM::visit(d,*this);
        return rv_;
    }

    void handle( Directive ) {}
    void handle( DirectiveCode  d ) { rv_ = &d.code().deref(); }
    void handleInDebugSection( Directive ) {}
    void handleInDebugSection( DirectiveCode  d ) { rv_ = &d.code().deref(); }
};

} //namespace

static
unsigned debugDeDeaderPrintString(HSAIL_ASM::BrigContainer& bc, BrigsOffset s)
{
    char * const sectionData = bc.strings().getData(0);
    assert(strlen(&sectionData[s]) < UINT_MAX);
    const unsigned sz = (unsigned)(strlen(&sectionData[s]))+1;
    if (DetailDebugDeDeader) {
        printf("    %u (size = %u): '%s'", (unsigned)s, sz, &sectionData[s]);
        for (unsigned i = 0; i < sz; ++i) { printf(" %.2x", sectionData[s++]); }
        printf("\n");
        fflush(stdout);
    }
    return sz;
}

static std::vector<BrigcOffset> cOffsetM;
static std::vector<BrigdOffset> dOffsetM;
static std::vector<BrigoOffset> oOffsetM;
static std::vector<BrigsOffset> sOffsetM;

ASSUMPTION(
"No instructions shall be marked for deletion after the first invocation of this function."
"This allows usage of deadCodeRanges for the sake of linking speed.");
static
void updateCodeReference(Directive& d)
{
    {
        BrigcOffset32_t* cf =  GetCodeFieldPtr()(d);
        if (cf == NULL) {
            // no code offsets in this dir
        }
        else if (*cf == UNUSED_OFFSET) {
            // keep zero offsets
        }
        else if (!IS_OFFSET_DEAD(cOffsetM[*cf])) {
            *cf = cOffsetM[*cf];
            if (DebugCorruptOffsets) { *cf = *cf + 15; }
        }
        else {
            // New offset refers to instruction which is marked for deletion. 
            if (inDebugSection(d)) {
                // Update item in .debug section.
                *cf = OFFSET_WRONG;
            }
            else {
                // Update conventional directive.
                // Let's find next "alive" instruction.
                // The "ending" offset of dead range is first candidate.
                // However, dead ranges may come in succession (without gaps), so
                // the "ending" offset may denote dead instriction as well. 
                // Therefore, we have to re-check "end" and try again if it is dead.
                // Note that we can reach the end of code during lookup (it never marked dead).
                for (BrigcOffset dead_ = *cf;;) {
                    CodeRanges::iterator p = deadCodeRanges.upper_bound(dead_);
                    assert(p != deadCodeRanges.end() && p->second <= dead_);
                    if (!IS_OFFSET_DEAD(cOffsetM[p->first])) {
                        *cf = cOffsetM[p->first];
                        break;
                    }
                    else {
                        dead_ = p->first;
                    }
                }
            }
        }
    }
}

// Delete dead code, operands, directives and strings.
static void DeDeader(HSAIL_ASM::BrigContainer& bc, BackRefs& br)
{
  cOffsetM.resize(bc.insts()     .size()+1); //+1 is for new offset for just-after-EOS location.
  dOffsetM.resize(bc.directives().size()+1);
  oOffsetM.resize(bc.operands()  .size()+1);
  sOffsetM.resize(bc.strings()   .size()+1);

  {
    if (DebugDeDeader) {
      printf("  .code\n");
      fflush(stdout);
    }
    if (DetailDebugDeDeader) {
      for(DeadFunctions::iterator p = deadFunctions.begin(); p != deadFunctions.end() ; ++p) {
        printf("   DeadFunctionList: d@%u d@%u i@%u i@%u.\n"
          ,p->first
          ,p->second.d_next
          ,p->second.c_start
          ,p->second.c_end);
        fflush(stdout);
      }
    }
    ASSUMPTION(
      "Code ranges *must* be traversed in ascending order,"
      " so that code_range[i] < code_range[i+1]. (why: see below)"
      " deadCodeRanges provides proper traversal order automatically.");
    BrigcOffset coff  = bc.insts().begin().brigOffset();
    BrigcOffset csize = bc.insts().size();
    unsigned removedSize = 0;
    for(CodeRanges::iterator p = deadCodeRanges.begin(); p != deadCodeRanges.end(); ++p) {
      BrigcOffset deadStart = p->second; // c_start
      BrigcOffset deadEnd = p->first; // c_end
      unsigned i;
      // If we have a fragment of alive-code here,
      // shift this "alive-code" back by the size of code previosly marked dead.
      // NB: The code will be corrupt if DeadFunctionList is not ordered.
      for (i = coff; i < deadStart; ++i) {
        cOffsetM[i] = i - removedSize;
      }
      // mark unused code as dead
      for (i = deadStart; i < deadEnd; ++i) {
        cOffsetM[i] = OFFSET_DEAD;
      }
      if (DetailDebugDeDeader) {
        printf("    %4u (size = %3u).\n", 
               (unsigned)deadStart, (unsigned)(deadEnd - deadStart));
        fflush(stdout);
      }
      // remember dead-code-size (how many bytes already marked as dead)
      removedSize += (deadEnd - deadStart);
      coff = i;
    }
    // Mark the rest for relocation to new offset
    while (coff < csize) {
      cOffsetM[coff] = coff - removedSize;
      ++coff;
    }
    // HACK: This is to update code offsets which refer
    // just beyond the end of the section:
    assert(coff == csize);
    cOffsetM[csize] = csize - removedSize;
  }
          
  {
    if (DebugDeDeader) {
      printf("  .directives\n");
      fflush(stdout);
    }
    ExcludeDeadFromHints(bc,br);
    // If a function/kernel directive reside at the end of section, 
    // it's nextDirective field is referring just beyond the end of section.
    // Obviosly, there is no directive beyond the end of section,
    // so the regular algorithm will not process such references.
    // We have to manually update offsets which refer just beyond the section's end.
    // HACK: tricky implementation, see how isSectionEnd is used.
    BrigdOffset doff  = bc.directives().begin().brigOffset();
    BrigdOffset dsize = bc.directives().size();
    unsigned removedSize = 0;
    for (;;) {
      bool isSectionEnd = (doff >= dsize);
      Directive d(&bc,doff);
      
      if (isSectionEnd) {
        dOffsetM[doff] = doff - removedSize;
        br.updateUsersUponRelocation(bc,d,dOffsetM[d.brigOffset()]);
        break; // finish the loop
      }
      else {
        if (inDeadDirectives(d)) {
          // Don't keep: mark for deletion and accumulate total size of marked dead
          while (doff < d.brigOffset()+d.brigSize())
            dOffsetM[doff++] = OFFSET_DEAD;
          removedSize += (doff - d.brigOffset());
          if (DetailDebugDeDeader) {
            printf("    %4u (size = %3u).\n", (unsigned)d.brigOffset(), (unsigned)d.brigSize());
            fflush(stdout);
          }
        }
        else {
          // Keep: prepare new offsets for relocation (reloc backward, by size of dead-marked)
          while (doff < d.brigOffset()+d.brigSize()) {
            dOffsetM[doff] = doff - removedSize;
            ++doff;
          }
        }
        br.updateUsersUponRelocation(bc,d,dOffsetM[d.brigOffset()]);
      }
  
      // Code will be relocated after de-deading, so we have to
      // update code references accordingly.
      updateCodeReference(d);
    }
  }

  {
    if (DebugDeDeader) {
      printf("  .debug\n");
      fflush(stdout);
    }
    // No deDeading for .debug section.
    for (Directive d = bc.debugChunks().begin(), d_end = bc.debugChunks().end(); d != d_end; d = d.next()) {
      updateCodeReference(d);
    }
  }

  {
    if (DebugDeDeader) {
      printf("  .operands\n");
      fflush(stdout);
    }
    BrigoOffset ooff  = bc.operands().begin().brigOffset();
    BrigoOffset osize = bc.operands().size();
    BrigoOffset removedSize = 0;
    while (ooff < osize) {
      Operand o(&bc,ooff);
      if (inDeadOperands(o)) {
        while (ooff < o.brigOffset()+o.brigSize()) {
          oOffsetM[ooff++] = OFFSET_DEAD;
        }
        removedSize += o.brigSize();
        if (DetailDebugDeDeader) {
          printf("    %4u (size = %3u).\n", (unsigned)o.brigOffset(), (unsigned)o.brigSize());
          fflush(stdout);
        }
      }
      else {
        while (ooff < o.brigOffset()+o.brigSize()) {
          oOffsetM[ooff] = ooff - removedSize;
          ++ooff;
        }
        br.updateUsersUponRelocation(bc,o,oOffsetM[o.brigOffset()]);
      }
    }
  }

  {
    if (DebugDeDeader) {
      printf("  .strtab (size = %u)\n", (unsigned)bc.strings().size());
      fflush(stdout);
    }
    char * const sectionData = bc.strings().getData(0);
    BrigsOffset s = bc.strings().NUM_BYTES_RESERVED;
    unsigned removedSize = 0;
    while (s < bc.strings().size()) {
      const BrigsOffset sBrigOffset = s;
      const size_t sBrigSize = strlen(&sectionData[s])+1;
      if (br.dir2str[s].empty() &&  br.op2str[s].empty() && br.debug2str[s].empty()) {
        while (s < sBrigOffset + sBrigSize) {
          sOffsetM[s++] = OFFSET_DEAD;
        }
        removedSize += sBrigSize;
        debugDeDeaderPrintString(bc,sBrigOffset);
      }
      else {
        while (s < sBrigOffset + sBrigSize) {
          sOffsetM[s] = s - removedSize;
          ++s;
        }
        br.updateStringUsersUponRelocation(bc,sBrigOffset,sOffsetM[sBrigOffset]);
      }
    }
  }
}

template <typename OFF_T>
static
size_t CopyNotDead(char * const data_, const size_t size_, std::vector<OFF_T>& new_offset)
{
  char * d = data_;
  const char * s = d;
  size_t alive_size = 0;
  size_t cumulative_alive_size = 0;

  // one additional iteration (+1) is required to
  // copy alive-range which reside at the end of section
  for (size_t i = 0; i < size_ +1; ++i) {
    if (i < size_
    && !IS_OFFSET_DEAD(new_offset[i])) {
      ++alive_size;
    }
    else { // dead byte or the end of section (EOS)
      if (alive_size != 0) { // 1st dead byte or EOS
        if (d != s) {
          memmove(d,s,alive_size); 
        }
        else {
          // 1st range is already in place
        }
        d += alive_size;
        s += alive_size;
        cumulative_alive_size += alive_size;
        alive_size = 0;
      }
      if (i < size_) { ++s; } // keep s valid after EOS
    }
  }
  return cumulative_alive_size;
}

template <typename OFF_T>
static
void CopyNotDead(StringSection& section, std::vector<OFF_T>& new_offsets)
{
  size_t copied = CopyNotDead(section.getData(0),section.size(),new_offsets);
  if (copied < section.size()) {
    section.deleteData(copied,section.size()-copied);
  }
}

template <typename Item, typename OFF_T>
static
void CopyNotDead(BrigSection<Item>& section, std::vector<OFF_T>& new_offsets)
{
  size_t copied = CopyNotDead(section.getData(0),section.size(),new_offsets);
  if (copied < section.size()) {
    section.deleteData(copied,section.size()-copied);
  }
}

// Create final BRIG sections by copying contents up and erasing the unneeded 
// contents in-place.
static void CreateFinalBRIGSections(HSAIL_ASM::BrigContainer& bc, DebugInformationLinking::DwarfLinker& dwarfLinker)
{
  using namespace HSAIL_ASM;
  size_t in_code       = bc.insts      ().size();
  size_t in_operands   = bc.operands   ().size();
  size_t in_directives = bc.directives ().size();
  size_t in_strings    = bc.strings    ().size();
  size_t in_debug      = bc.debugChunks().size();

  CopyNotDead(bc.insts()     ,cOffsetM);
  CopyNotDead(bc.operands()  ,oOffsetM);
  CopyNotDead(bc.directives(),dOffsetM);
  CopyNotDead(bc.strings()   ,sOffsetM); // Makes string section valid (no dupes)

  // We can add debug information to the container *only* when its string section
  // does not contain duplicates. This is because debug info adds several strings
  // (those which referenced from BlockStart and BlockString). Adding strings by
  // means of Brigantine's addString() requires valid string section.
  if(linkDwarf) {
    using namespace DebugInformationLinking;
    DWARF_LINKER_TRACE("linking DWARF data...\n");
    DwarfDataBlockConstRef dd = dwarfLinker.getResultingDwarfData();
    if(dd.size()) {
      DWARF_LINKER_TRACE("storing %lu bytes in .debug section of output file...\n", (unsigned long)dd.size());
      storeDwarfDebugData(bc, dd);
      if(DebugInfoFilename.size() > 0) {
        DWARF_LINKER_TRACE("storing %lu bytes in file %s...\n", (unsigned long)dd.size(), DebugInfoFilename.c_str());
        std::ofstream ofs( DebugInfoFilename.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
        ofs.write(reinterpret_cast<const char *>(&dd.at(0)), dd.size());
        ofs.close();
      }
    }
    else {
      DWARF_LINKER_TRACE("DWARF data hasn't been linked.\n");
    }
  }

  if (DebugDeDeader) {
    printf("  .code\n"
      "    Input (bytes) = %u Final (bytes) = %u.\n",
      static_cast<unsigned>(in_code)      ,static_cast<unsigned>(bc.insts().size()));
    printf("  .operands\n"
      "    Input (bytes) = %u Final (bytes) = %u.\n",
      static_cast<unsigned>(in_operands)  ,static_cast<unsigned>(bc.operands().size()));
    printf("  .directives\n"
      "    Input (bytes) = %u Final (bytes) = %u.\n",
      static_cast<unsigned>(in_directives),static_cast<unsigned>(bc.directives().size()));
    printf("  .strings\n"
      "    Input (bytes) = %u Final (bytes) = %u.\n",
      static_cast<unsigned>(in_strings)   ,static_cast<unsigned>(bc.strings().size()));
    printf("  .debug\n"
      "    Input (bytes) = %u Final (bytes) = %u.\n",
      static_cast<unsigned>(in_debug)     ,static_cast<unsigned>(bc.debugChunks().size()));
    fflush(stdout);
  }
}

// Output final BRIG object in an ELF container.
static int OutputFinalBRIG(HSAIL_ASM::BrigContainer& out)
{
  using namespace HSAIL_ASM;
  if (!DisableValidatorOutput) {
    if (ValidateContainer(out, NULL, !CreateLibrary)) {
      errs() << "FATAL: Validation of output BRIG failed.\n";
      errs().flush();
      return 2;
    }
  }
  if (BifOutput
  ?  BifStreamer::save(out, getOutputFileName().c_str())
  : BrigStreamer::save(out, getOutputFileName().c_str()))
  {
      errs() << "Error: Can't save output file: '" << getOutputFileName() << "'.\n";
      errs().flush();
      return 1;
  }
  return 0;
}

static const char programOverview[] = "HSAIL Device code (BRIG) Linker.\n";

static void printVersion() {
  outs() << programOverview;
  outs() << "  (C) AMD 2012, all rights reserved.\n"; // TODO_HSA (3) YYYY-YYYY
  outs() << "  Built " << __DATE__ << " (" << __TIME__ << ").\n";
  // TODO_HSA (3) versioning?
  // outs() << "  Version " << "0.1" << ".\n";
#ifdef DEBUG
  outs() << "  HSAIL spec version: " << SUPPORTED_HSAIL_SPEC_VERSION << ".\n";
#endif
}

int main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  cl::SetVersionPrinter(printVersion);
  cl::ParseCommandLineOptions(argc, argv, programOverview);

  // Adjust options, if needed.
  //
  if (DetailDebugParser) { DebugParser = true; }
  if (DetailDebugDeDeader) { DebugDeDeader = true; }

  // Concatenate multiple BRIG objects; create reference lists.
  //
  HSAIL_ASM::BrigContainer out;
  BackRefs backRefs;
  Modules modules;
  AppendContext appendContext;
  if (DebugParser) {
    printf("Parser:\n");
    fflush(stdout);
  }
  if (Concatenate(out,modules,backRefs,appendContext)) { return 1; }
  if (DetailDebugParser) { modules.print(outs()); }

  // Resolve declarations to definitions.
  //
  int rc  = ResolveDeclarations(out,backRefs,modules);
  if (rc) return rc;

  // Find and eliminate duplicate operands and strings.
  //
  if (DebugDeDuper) {
    printf("DeDuper:\n");
    fflush(stdout);
  }
  // [GeneralNotesOnDeDuping] DeDuplication is implemented only for strings and operands,
  // because it is useless so far for instructions and for directives.
  //
  // [MandatoryStringDeDuping] String deduplication is always enabled in accordance to 
  // HSAIL 1.0 requirement that there should be no duplicates in the string section.
  //
  // [DwarfAndDeDuping] Dwarf is not referencing brig strings and operands, 
  // therefore DeDuping of strings and operands is OK when dwarf linking is enabled.
  DeDupeStrings(out,backRefs);
  if (!NoDeDuper) {
    DeDupeOperands(out,backRefs);
  }

  // Detect useless code, operands, directives and strings.
  // Build lists of dead code, directives and operands.
  // Detach backRefs ("used-by" connections) to useless items.
  //
  if (!NoDeDeader) {
    for (int n_pass = 1;; ++n_pass) {
      if (DebugDeDeader) {
        printf("Useless Detection, pass %d...\n",n_pass);
        fflush(stdout);
      }
      DetectUselessRv this_ = DetectUseless(out,backRefs);
      if (DebugDeDeader) {
        printf("Useless Detection, pass %d: %d new useless items found, %d backRefs removed.\n"
          ,n_pass
          ,this_.n_useless
          ,this_.n_backrefs_removed);
        fflush(stdout);
      }
      if (this_.n_useless == 0 && this_.n_backrefs_removed == 0) { break; }
      if (NoIterativeOptimization) { break; }
    }
  }

  // Delete code, operands and directives referred by dead-lists.
  // Delete unreferenced strings.
  //
  // [StringDeDupingAndDeDeader] String DeDuplication requires deDeading step in order to wipe out duplicate strings.
  // Therefore, deDeader shall be always invoked (even under "-no-dedeader" option); otherwise
  // output brig may be invalid (see [MandatoryStringDeDuping] above). However, when deDeading is OFF, 
  // deDeader does not remove anything except unreferenced strings, 
  // because DetectUseless() is not previously invoked.
  //
  if (DebugDeDeader) {
    printf("DeDeader:\n");
    fflush(stdout);
  }
  DeDeader(out,backRefs);
  DebugInformationLinking::DwarfLinker dwarfLinker;
  ConcatenateDwarf(dwarfLinker,appendContext,cOffsetM,dOffsetM);
  CreateFinalBRIGSections(out,dwarfLinker);
  return OutputFinalBRIG(out);
}

// Correctness TODO
// *  [artem] (4) Implement passing of 3rd-party .debug  blocks to output BRIG.
// *  [artem] (?) Add support for BrigEOperandFunctionList when it is supported in other tools.
// *  [artem] (?) Design & implement support <for HSAIL 1.0-related> all-zero instructions.
// *  [artem] (?) Resolve uncertainty  with HSAIL pragmas & update implementation.

// Performance TODO
// *  [Kannan] (idea) express constants in an operation sequence as c2=c1+x;c3=c2+y
// *  [Kannan] (idea) parallelizable linker.
// *  [Kannan] (?) Faster Directive delete as per FDD pseudo code.

// Code quality TODO
// *  [artem]  (3) Debug information handling has very similar implementation in HSAILAsm. 
//                 Rework to reuse code from HSAILAsm/libHSAIL.
// *  [artem]  (4) Move static_assert() etc. to some other place ("stdlib unification layer"?) 
//                 for re-use in other compiler/assembler components.

// Features TODO
// *  [Kannan] (3) Make this a reusable library to be used at runtime.
//                 This includes redesign of diagnostic messages etc.
// *  [Kannan] (?) incremental linking.
// *  [artem]  (?) deDupe OperandRegV2, OperandRegV4 when needed.
// *  [Kannan] (idea) Measure number of eliminated entries.
// *  [Kannan] (idea) Measure time consumed.
