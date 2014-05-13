//===-- AMDILMDTypes.h -  ------*- C++ -*-===//
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
#ifndef _AMDIL_MD_TYPES_H_
#define _AMDIL_MD_TYPES_H_

#include "llvm/Support/Compiler.h"
#include "llvm/Type.h"

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace llvm
{
  class MDStrings;
  class MDMemory;
  class MDFlag;
  class MDInt;
  class MDReflection;
  class MDConstArg;
  class MDIntList;
  class MDPrintf;
  class MDArg;
  class MDCBArg;
  class MDSampler;
  class MDImage;
  class MDCounter;
  class MDSemaphore;
  class MDValue;
  class MDPointer;
  class raw_ostream;

  enum MDTypes {
    MDStringsTy  = 0,
    MDIntListTy = 1,
    MDMemoryTy  = 2,
    MDFlagTy    = 3,
    MDIntTy     = 4,
    MDPrintfTy  = 5,
    MDArgTy     = 6,
    MDCBArgTy   = 7,
    MDSamplerTy = 8,
    MDImageTy   = 9,
    MDCounterTy = 10,
    MDValueTy   = 11,
    MDPointerTy = 12,
    MDDefaultTy = 13,
    MDSemaphoreTy = 14,
    MDReflectionTy = 15,
    MDConstArgTy = 16,
    MDLastTy
  };
  //===--------------------- Metadata Node Types ---------------------------===//
  // The MDType node is the parent node of all the Metadata nodes. The
  // hierarchy is as follows.
  // MDType
  //   - MDStrings
  //   - MDMemory
  //   - MDFlag
  //   - MDInt
  //       - MDReflection
  //       - MDConstArg
  //   - MDIntList
  //       - MDPrintf
  //   - MDArg
  //       - MDSampler
  //       - MDCBArg
  //           - MDImage
  //           - MDSemaphore
  //           - MDCounter
  //           - MDValue
  //               - MDPointer
  //===---------------------------------------------------------------------===//
  class MDType {
    friend raw_ostream& operator<<(raw_ostream&, MDType&);
    public:
    virtual ~MDType();
    // Return the type as a MDType
    MDType* getMDType() { return this; }
    // Get the type as a MDStrings
    virtual MDStrings* getMDStrings() { return NULL; }
    // Get the type as a MDMemory
    virtual MDMemory* getMDMemory() { return NULL; }
    // Get the type as a MDFlag
    virtual MDFlag* getMDFlag() { return NULL; }
    // Get the type as a MDInt
    virtual MDInt* getMDInt() { return NULL; }
    // Get the type as a MDReflection
    virtual MDReflection* getMDReflection() { return NULL; }
    // Get the type as a MDConstArg
    virtual MDConstArg* getMDConstArg() { return NULL; }
    // Get the type as a MDIntList
    virtual MDIntList* getMDIntList() { return NULL; }
    // Get the type as a MDArg
    virtual MDArg* getMDArg() { return NULL; }
    // Get the type as MDSampler
    virtual MDSampler* getMDSampler() { return NULL; }
    // Get the type as MDCBArg
    virtual MDCBArg* getMDCBArg() { return NULL; }
    // Get the type as MDPrintf
    virtual MDPrintf* getMDPrintf() { return NULL; }
    // Get the type as MDImage
    virtual MDImage* getMDImage() { return NULL; }
    // Get the type as MDSemaphore
    virtual MDSemaphore* getMDSemaphore() { return NULL; }
    // Get the type as MDCounter
    virtual MDCounter* getMDCounter() { return NULL; }
    // Get the type as MDValue
    virtual MDValue* getMDValue() { return NULL; }
    // Get the type as MDPointer
    virtual MDPointer* getMDPointer() { return NULL; }
    // Dump the current node
    virtual void dump();
    // Holds the name of the current node
    std::string Name_;
    unsigned getType() { return TypeID_; }
    protected:
    // This type should never be directly instantiated
    MDType(std::string name);
    // Holds the ID type for the current node
    unsigned TypeID_;
    private:
    // This should never be initialized without a
    // corresponding name
    MDType();
    // Function that prints out the mapping between
    // the node ID and a string name for that node ID
    const char* typeIDToName(unsigned ID);
  }; // MDType

  // The MDStrings class handles all of the cases where the
  // metadata is in the formation of <TOKEN>:<String> such
  // as ;device:Name or ;error:msg.
  class MDStrings : public MDType {
    friend raw_ostream& operator<<(raw_ostream&, MDStrings&);
    public:
    MDStrings(std::string name);
    ~MDStrings();
    MDStrings* getMDStrings() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    std::string Str_;
    private:
    MDStrings();
  }; // MDStrings

  // The MDMemory class handles the cases where
  // the memory token is specified, but specifies
  // both an address space and the size of memory
  // that the address space requires.
  // An example is ;memory:local:1024.
  class MDMemory : public MDType {
    friend raw_ostream& operator<<(raw_ostream&, MDMemory&);
    public:
    MDMemory(std::string name);
    ~MDMemory();
    MDMemory* getMDMemory() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    std::string AS_;
    uint32_t Size_;
    private:
    MDMemory();
  }; // MDMemory

  // The MDFlag class handles the case where
  // there is a token followed by another token
  // that specifies that a certain flag should
  // be enabled. An example is ;memory:compilerwrite
  class MDFlag : public MDType {
    friend raw_ostream& operator<<(raw_ostream&, MDFlag&);
    public:
    MDFlag(std::string name);
    ~MDFlag();
    MDFlag* getMDFlag() { return this; }
    virtual void dump() LLVM_OVERRIDE;

    bool Flag_;
    private:
    MDFlag();
  }; //  MDFlag

  // The MDInt class handles the case where
  // there is a token followed by another
  // token that is an integer.
  // An example of this is the uavid token.
  // ;uavid:1
  class MDInt : public MDType {
    friend raw_ostream& operator<<(raw_ostream&, MDInt&);
    public:
    MDInt(std::string name);
    ~MDInt();
    MDInt* getMDInt() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    unsigned Int_;
    private:
    MDInt();
  }; // MDInt

  // The MDReflection class handles the case for
  // reflection where the type is needed.
  // The difference between this and
  // the MDInt class is that the name is implicit
  // in the type and the name string is actually the
  // reflection string.
  class MDReflection : public MDInt {
    friend raw_ostream& operator<<(raw_ostream&, MDReflection&);
    public:
    MDReflection(std::string str);
    ~MDReflection();
    MDReflection* getMDReflection() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    private:
    MDReflection();
  }; // MDReflection

  // The MDConstArg class handles the case for
  // reflection where the constant is needed.
  // The difference between this and
  // the MDInt class is that the name is implicit
  // in the type and the name string is actually the
  // reflection string.
  class MDConstArg : public MDInt {
    friend raw_ostream& operator<<(raw_ostream&, MDConstArg&);
    public:
    MDConstArg(std::string str);
    ~MDConstArg();
    MDConstArg* getMDConstArg() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    private:
    MDConstArg();
  }; // MDConstArg

  // The MDIntList class handles the case where
  // there is a token followed by a string of
  // integers. An example of this is
  // the version or function tokens.
  // ;version:1:3:2
  // ;function:3:1:2:3
  class MDIntList : public MDType {
    friend raw_ostream& operator<<(raw_ostream&, MDIntList&);
    public:
    MDIntList(std::string name);
    ~MDIntList();
    MDIntList* getMDIntList() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    std::vector<unsigned>* IntList_;
    private:
    MDIntList();
  }; // MDIntList

  // MDPrintf is derived from the MDIntList
  // class, where the one difference is that
  // the MDPrintf class has a string as
  // the last token.
  class MDPrintf : public MDIntList {
    friend raw_ostream& operator<<(raw_ostream&, MDPrintf&);
    public:
    MDPrintf(std::string name);
    ~MDPrintf();
    MDPrintf* getMDPrintf() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    unsigned StrLen_;
    char* Str_;
    private:
    MDPrintf();
  }; // MDPrintf

  // The MDArg class supplies an interface
  // to metadata related to arguments. However
  // this class cannot be directly instantiated
  // itself. There is no real metadata node
  // that maps to it. So there is no way
  // to dump or print this node.
  class MDArg : public MDType {
    friend raw_ostream& operator<<(raw_ostream&, MDArg&);
    public:
    virtual ~MDArg();
    std::string Arg_;
    MDArg* getMDArg() { return this; }
    protected:
    MDArg(std::string name);
    private:
    MDArg();
  }; // MDArg

  // The MDSampler class handles the unique data
  // for the sampler token. The token is specified
  // as ;sampler:name:id:location:value
  class MDSampler : public MDArg {
    friend raw_ostream& operator<<(raw_ostream&, MDSampler&);
    public:
    MDSampler(std::string name);
    ~MDSampler();
    MDSampler* getMDSampler() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    unsigned ID_;
    bool isArg_;
    unsigned Val_;
    private:
    MDSampler();
  }; // MDSampler

  // The MDCBArg is another metadata class dealing with
  // arguments. The difference is that arguments that
  // inherit this class use the CB to store data, but
  // the that is not a requirement of inheriting the
  // MDArg class.
  // This class cannot be instantiated by itself and
  // not direct metadata token is derived from it.
  // This class also cannot be dumped or emitted
  // directly.
  class MDCBArg : public MDArg {
    friend raw_ostream& operator<<(raw_ostream&, MDCBArg&);
    public:
    virtual ~MDCBArg();
    MDCBArg* getMDCBArg() { return this; }
    unsigned CBNum_;
    unsigned CBOffset_;
    protected:
    MDCBArg(std::string name);
    virtual void dump() LLVM_OVERRIDE;
    // This only emits the tokens
    // :CBNum_:CBOffset_
    private:
    MDCBArg();
  }; // CBArg

  // The MDImage node handles the metadata related to images.
  class MDImage : public MDCBArg {
    friend raw_ostream& operator<<(raw_ostream&, MDImage&);
    public:
    MDImage(std::string name);
    ~MDImage();
    MDImage* getMDImage() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    std::string Dim_;
    std::string Type_;
    unsigned ID_;
    private:
    MDImage();
  }; // MDImage

  // The MDSemaphore class handles the metadata related to
  // semaphores.
  class MDSemaphore : public MDCBArg {
    friend raw_ostream& operator<<(raw_ostream&, MDSemaphore&);
    public:
    MDSemaphore(std::string name);
    ~MDSemaphore();
    MDSemaphore* getMDSemaphore() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    unsigned ID_;
    private:
    MDSemaphore();
  }; // MDSemaphore


  // The MDCounter class handles the metadata related to
  // atomic counters.
  class MDCounter : public MDCBArg {
    friend raw_ostream& operator<<(raw_ostream&, MDCounter&);
    public:
    MDCounter(std::string name);
    ~MDCounter();
    MDCounter* getMDCounter() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    uint32_t Size_;
    unsigned ID_;
    private:
    MDCounter();
  }; // MDCounter

  // The MDValue class handles the metadata related
  // to the pass by value arguments.
  class MDValue : public MDCBArg {
    friend raw_ostream& operator<<(raw_ostream&, MDValue&);
    public:
    MDValue(std::string name);
    virtual ~MDValue();
    MDValue* getMDValue() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    std::string Type_;
    uint32_t Size_;
    private:
    MDValue();
  }; // MDValue

  // The MDPointer class handles the metadata related
  // to the pass by pointer arguments.
  class MDPointer : public MDValue {
    friend raw_ostream& operator<<(raw_ostream&, MDPointer&);
    public:
    MDPointer(std::string name);
    ~MDPointer();
    MDPointer* getMDPointer() { return this; }
    virtual void dump() LLVM_OVERRIDE;
    std::string MemType_;
    unsigned BufNum_;
    unsigned Alignment_;
    std::string AccessType_;
    unsigned Volatile_;
    unsigned Restrict_;
    private:
    MDPointer();
  }; // MDPointer

  //===---------------- Component Node Types -----------------------------===//
  // The component node is the parent node of all the compilation unit node
  // types. The hierarchy between the classes is as follows.
  // Component
  //   - MDBlock
  //   - StmtBlock
  //     - FuncBase
  //       - ILFunc
  //       - MainFunc
  //       - Macro
  //           - DummyMacro
  //       - DBSection
  //       - DataSection
  //===-------------------------------------------------------------------===//
  struct AMDILMetadata;
  class MDBlock;
  class DBSection;
  class ILFunc;
  class Macro;
  class StmtBlock;
  class MainFunc;
  class DataSection;
  class DummyMacro;
  class Component {
    protected:
    enum {
      MDBlockClass,
      StmtBlockClass,
      ILFuncClass,
      MainFuncClass,
      MacroClass,
      DummyMacroClass,
      DBSectionClass,
      DataSectionClass,
    };
    friend raw_ostream& operator<<(raw_ostream &, Component &);
    public:
    virtual ~Component();
    virtual void dump() = 0;
    virtual void process() = 0;
    unsigned getSubClassID() const { return SubClassID_; }
    static inline bool classof(const Component *) { return true; }
    std::string FuncName_;
    protected:
    Component(std::string name, unsigned SubClassID);
    private:
    Component();
    unsigned SubClassID_;
  }; // ComponentBlock

  // The MDBlock class holds all of the MDType's for
  // a single block of metadata. There should exist
  // a single block of metadata for each kernel
  // wrapper.
  class MDBlock : public Component {
    friend raw_ostream& operator<<(raw_ostream &, MDBlock&);
    public:
    MDBlock(std::string name);
    ~MDBlock();
    void process();
    virtual void dump();
    void updateMetadata(AMDILMetadata *);
    static inline bool classof(const MDBlock *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == MDBlockClass;
    }
    std::vector<class MDType*>* MDVals;
    std::map<std::string, MDType*> MDValMap_;
    private:
    MDBlock();// Do not want a default constructor
  }; // MDBlock

  class StmtBlock : public Component {
    friend raw_ostream& operator<<(raw_ostream &, StmtBlock&);
    public:
    virtual ~StmtBlock();

    virtual void dump() LLVM_OVERRIDE;
    virtual void process();
    unsigned getLineNum();
    unsigned getBaseLineNum();
    void setLineNum(unsigned);
    void useLineNum(bool);
    typedef std::list<std::string*>::iterator iterator;
    void setStatements(std::list<std::string*>*);

    // Functions to get access to the statements
    iterator begin();
    iterator end();
    size_t size();

    static inline bool classof(const StmtBlock *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == StmtBlockClass;
    }

    protected:
    StmtBlock(std::string name, unsigned SubClassID);
    // Flag to determine if the line numbers should be displayed on
    // output stream or not
    bool useLineNum_;
    // Internal counter that keeps track of the line number for the
    // function
    unsigned lineNum_;
    // base offset that all line numbers use
    unsigned baseLineNum_;
    // Line number that corresponds to the stmt string number.
    // These numbers are relative to the function beginning so
    // need to add in the base offset when using.
    std::vector<unsigned> LineNums_;
    void computeLineNumbers();
    private:
    std::list<std::string*>* Stmts_;
    StmtBlock(); // Do not want a default constructor
  }; // StmtBlock

  class FuncBase : public StmtBlock {
    public:
    virtual ~FuncBase() {};
    virtual void dump() = 0;
    virtual void process() = 0;

    static inline bool classof(const FuncBase *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == ILFuncClass ||
             C->getSubClassID() == MacroClass;
    }

    protected:
    FuncBase(std::string name, unsigned SubClassID)
      : StmtBlock(name, SubClassID) {};
    private:
    FuncBase(); // Do not want a default constructor
  };

  class ILFunc : public FuncBase {
    friend raw_ostream& operator<<(raw_ostream &, ILFunc&);
    public:
    ILFunc(unsigned ID, std::string name);
    ~ILFunc();
    virtual void dump();
    virtual void process();
    bool isKernel();
    unsigned ID_;
    unsigned getTmpLineNumber(const std::string&);
    static inline bool classof(const ILFunc *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == ILFuncClass;
    }

    std::set<ILFunc*> dependentBlocks_;
    MainFunc* mainFunc_;
    MDBlock* metadata_;
    std::set<unsigned> macros_;
    std::vector<DataSection*>* data_;
    std::map<std::string, unsigned> labelMap_;
    private:
    bool isKernel_;
    ILFunc(); // Do not want a default constructor
  }; // ILFunc

  class MainFunc : public StmtBlock {
    friend raw_ostream& operator<<(raw_ostream &, MainFunc&);
    public:
    MainFunc(std::string name);
    ~MainFunc();
    virtual void dump();
    virtual void process();
    void setFuncID(unsigned);
    static inline bool classof(const MainFunc *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == MainFuncClass;
    }

    private:
    unsigned callID_;
    MainFunc(); // Do not want a default constructor
  }; // MainFunc

  class Macro : public FuncBase {
    friend raw_ostream& operator<<(raw_ostream &, Macro&);
    public:
    Macro(std::string name);
    Macro(std::string name, unsigned SubClassID);
    virtual ~Macro();
    virtual void dump();
    virtual void process();
    static inline bool classof(const Macro *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == MacroClass;
    }

    std::string header_;
    std::string footer_;
    private:
    Macro(); // Do not want a default constructor
  }; // Macro

  class DummyMacro : public Macro {
    friend raw_ostream& operator<<(raw_ostream &, DummyMacro&);
    public:
    DummyMacro(std::string name);
    ~DummyMacro();
    static inline bool classof(const DummyMacro *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == DummyMacroClass;
    }

    private:
    DummyMacro(); // Do not want a default constructor
  }; // DummyMacro

  class DBSection : public StmtBlock {
    friend raw_ostream& operator<<(raw_ostream &, DBSection&);
    public:
    DBSection(std::string name);
    ~DBSection();
    virtual void dump();
    virtual void process();
    static inline bool classof(const DBSection *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == DBSectionClass;
    }

    std::string debugStr_;
    private:
    DBSection(); // Do not want a default constructor
  }; // DBSection

  // This class stores a single entry into the data section
  class Data {
    friend raw_ostream& operator<<(raw_ostream&, Data&);
    public:
    Data(const std::string& name);
    ~Data();
    void dump();
    void process();
    uint32_t Size_;
    uint32_t Offset_;
    std::string Type_;
    std::list<std::string*>* Data_;
    private:
    Data(); // Do not want a default constructor
  }; // Data

  // This class stores the data section metadata
  class DataSection : public Component {
    friend raw_ostream& operator<<(raw_ostream &, DataSection&);
    public:
    DataSection(std::string name);
    ~DataSection();
    virtual void dump();
    void process();
    static inline bool classof(const DataSection *) { return true; }
    static inline bool classof(const Component *C) {
      return C->getSubClassID() == DataSectionClass;
    }

    uint32_t Size_;
    // Vector of data classes that holds a single entry into
    // the data section for each pointer.
    std::vector<Data*>* Data_;
    private:
    DataSection(); // Do not want a default constructor
  }; // DataSection

} // end llvm namespace
#endif // _AMDIL_MD_TYPES_H_
