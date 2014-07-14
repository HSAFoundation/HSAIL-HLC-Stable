//===-------------------------------------------------------------*-C++ -*-===//
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
//  This file contains the interface file to interact with the
//  AMDIL Metadata Parser. It contains a high level interface that
//  can be used irrespective of use and low level interfaces that
//  are tailored for specialized use cases such as Metadata extraction
//  or Debugging.
//
//===---------------------------------------------------------------------===//
#ifndef _AMDIL_MDINTERFACE_H_
#define _AMDIL_MDINTERFACE_H_

#include "AMDILMDTypes.h"

#include <string>
#include <vector>
#include <map>

namespace llvm
{
  class raw_ostream;
  // Struct that holds metadata information for a printf metadata token.
  typedef struct printfFmtRec {
    unsigned ID; // The printf ID number
    std::vector<unsigned> argSizes; // A vector of argument sizes.
    std::string fmtStr; // The format string itself.
  } printfFmt;

  // Enumerations for the various argument types.
  typedef enum argTypeEnum {
    ARG_TYPE_ERROR = 0,
    ARG_TYPE_SAMPLER = 1,
    ARG_TYPE_IMAGE = 2,
    ARG_TYPE_COUNTER = 3,
    ARG_TYPE_VALUE = 4,
    ARG_TYPE_POINTER = 5,
    ARG_TYPE_SEMAPHORE = 6,
    ARG_TYPE_LAST = 7
  } ArgType;

  // Enumerations of the valid data types for pass by value and
  // pass by pointer kernel arguments.
  typedef enum dataTypeEnum {
    DATATYPE_i1 = 1,
    DATATYPE_i8,
    DATATYPE_i16,
    DATATYPE_i32,
    DATATYPE_i64,
    DATATYPE_u8,
    DATATYPE_u16,
    DATATYPE_u32,
    DATATYPE_u64,
    DATATYPE_half,
    DATATYPE_float,
    DATATYPE_double,
    DATATYPE_f80,
    DATATYPE_quad,
    DATATYPE_struct,
    DATATYPE_union,
    DATATYPE_event,
    DATATYPE_opaque,
    DATATYPE_unknown,
    DATATYPE_LAST
  } DataType;

  // Enumerations of the valid memory types for pass by pointer
  // kernel arguments
  typedef enum memoryTypeEnum {
    PTR_MT_GLOBAL = 1, // global buffer
    PTR_MT_SCRATCH_EMU = 2, // SW emulated private memory
    PTR_MT_LDS_EMU = 3, // SW emulated local memory
    PTR_MT_UAV = 4, // uniformed access vector memory
    PTR_MT_CONSTANT_EMU = 5, // SW emulated constant memory
    PTR_MT_GDS_EMU = 6, // SW emulated region memory
    PTR_MT_LDS = 7, // HW local memory
    PTR_MT_SCRATCH = 8, // HW private memory
    PTR_MT_CONSTANT = 9, // HW constant memory
    PTR_MT_GDS = 10, // HW region memory
    PTR_MT_UAV_SCRATCH = 11, // SI and later HW private memory
    PTR_MT_UAV_CONSTANT = 12, // SI and later HW constant memory
    PTR_MT_FLAT = 13, // CI and later HW flat indexing
    PTR_MT_LAST
  } MemoryType;

  typedef enum imageTypeEnum {
    ACCESS_TYPE_RO = 1,
    ACCESS_TYPE_WO = 2,
    ACCESS_TYPE_RW = 3,
    ACCESS_TYPE_LAST
  } AccessType;

  // Struct that holds metadata information for kernel arguments.
  typedef struct argTypeRec {
    ArgType type;
    std::string argName;
    union {
      struct { // Struct for sampler arguments
        unsigned ID;
        unsigned isKernelDefined;
        unsigned value;
      } sampler;
      struct { // Struct for image arguments
        AccessType type;
        bool is2D;
        bool is1D;
        bool isArray;
        bool isBuffer;
        unsigned resID;
        unsigned cbNum;
        unsigned cbOffset;
      } image;
      struct { // struct for atomic counter arguments
        unsigned is32bit;
        unsigned resID;
        unsigned cbNum;
        unsigned cbOffset;
      } counter;
      struct { // struct for semaphore arguments
        unsigned resID;
        unsigned cbNum;
        unsigned cbOffset;
      } sema;
      struct { // struct for pass by value arguments
        DataType data;
        unsigned numElements;
        unsigned cbNum;
        unsigned cbOffset;
      } value;
      struct { // struct for pass by pointer arguments
        DataType data;
        unsigned numElements;
        unsigned cbNum;
        unsigned cbOffset;
        MemoryType memory;
        unsigned bufNum;
        unsigned align;
        AccessType type;
        unsigned volatile_;
        unsigned restrict_;
        unsigned pipe_;
      } pointer;
    } arg;
    std::string reflectStr;
    bool isConst;
  } argType;
  // Struct that stores the metadata for a given kernel.
  // This stucture will hold all of the metadata for a given
  // kernel and all of the functions it calls. This structure alone
  // is enough to allow the runtime to access all of the information it needs.
  struct AMDILMetadata {
    AMDILMetadata() {
      kernelName = deviceName = "";
      major = minor = revision = 0;
      localMem = privateMem = regionMem = hwlocalMem = hwprivateMem = hwregionMem = 0;
      compilerWrite = datareqd = packedReg = funcID = abi64 = gws = swgws = 0;
      uavID = privateID = printfID = cbID = 0;
      hasCWS = hasCRS = false;
      lws = 0;
      cws[0] = cws[1] = cws[2] = 0;
      crs[0] = crs[1] = crs[2] = 0;
      enqueue_kernel = false;
      kernel_index = 0;
    }
    std::string kernelName;
    unsigned major, minor, revision;
    std::string deviceName;
    std::set<std::string> errorMsgs;
    std::set<std::string> warningMsgs;
    unsigned localMem; // Size of the software local memory
    unsigned privateMem; // Size of the software private memory
    unsigned regionMem; // Size of the software region memory
    unsigned hwlocalMem; // Size of the hardware local memory
    unsigned hwprivateMem; // Size of the hardware private memory
    unsigned hwregionMem; // size of the software region memory
    unsigned compilerWrite;
    unsigned datareqd;
    unsigned packedReg;
    unsigned funcID;
    unsigned abi64;
    std::vector<argType> arguments;
    std::map<unsigned, std::string> reflection; // arg index, type of argument
    std::map<std::string, unsigned> constarg; // for constant args: name of arg, arg index. 
    unsigned uavID;
    unsigned privateID;
    unsigned printfID;
    unsigned cbID;
    std::vector<printfFmt> printfStrs;
    bool hasCWS;
    unsigned cws[3];
    bool hasCRS;
    unsigned crs[3];
    unsigned lws;
    unsigned gws;
    unsigned swgws;
    bool enqueue_kernel;
    unsigned kernel_index;
  };

  // The struct that stores the data section for a specific data
  // segment.
  struct AMDILDataSection {
    unsigned cbNum; // The CB the data belongs in, 0 for software emulated
    unsigned size; // The size of the data section
    unsigned char* data; // A pointer to the data.
  };
  // Class that stores the bitstreams for each individual
  // dwarf section. They are stored in both text and binary
  // format. The text format only stores the input and is
  // used mainly for debugging.
  class AMDILDwarf {
    friend raw_ostream& operator<<(raw_ostream &, AMDILDwarf &);
    public:
    typedef enum {
      DEBUG_INFO      = 0,
      DEBUG_ABBREV    = 1,
      DEBUG_LINE      = 2,
      DEBUG_PUBNAMES  = 3,
      DEBUG_PUBTYPES  = 4,
      DEBUG_LOC       = 5,
      DEBUG_ARANGES   = 6,
      DEBUG_RANGES    = 7,
      DEBUG_MACINFO   = 8,
      DEBUG_STR       = 9,
      DEBUG_FRAME     = 10,
      DEBUG_STATIC_FUNC = 11,
      DEBUG_STATIC_VARS = 12,
      DEBUG_WEAKNAMES = 13,
      DEBUG_LAST      = 14
    } DwarfSection; // Dwarf sections
    AMDILDwarf(std::vector<DBSection*>&, std::vector<ILFunc*>& ilData);

    virtual ~AMDILDwarf();

    // Debug function that dumps the dwarf information to std::cerr.
    void dump();

    // Get a particular dwarf section in a string format
    // This is the input string and not equivalent to the
    // dwarf bitstream. The dwarf bitstream is the output
    // after token patching has been done.
    std::string getDwarfString(DwarfSection);

    // Get a particular dwarf section in bitstream format
    // The memory returned gets deleted when AMDILDwarf
    // is destroyed. The size is passed return as the
    // second parameter.
    const char* getDwarfBitstream(DwarfSection, size_t&);
    private:
    // Take a dwarf token that is a calculation and
    // patch all the symbols with values turning it into
    // a simple formula that needs to be evaluted.
    std::string tokenToFormula(const std::string& tkn);

    // Evaluate the forumla that is return from tokenToFormula.
    unsigned evalFormula(const std::string& tkn);

    // Function that takes in a token and returns the value that
    // it corresponds to or ~0U for an error.
    unsigned tokenToValue(const std::string &tkn);

    // This array stores an array of bytes that the dwarf data
    // is assembled to.
    char* dwarfSections[DEBUG_LAST];

    // This array stores the strings that are used for input
    // by each dwarf section.
    std::string dwarfStrings[DEBUG_LAST];

    // This array stores the size of memory allocated for each
    // dwarf section.
    size_t dwarfSizes[DEBUG_LAST];

    // This array stores the offset for each dwarf section.
    size_t dwarfOffsets[DEBUG_LAST];

    std::vector<ILFunc*>& mILData;

    // For each label that is encountered when parsing the dwarf
    // data, this map stores a set that contains the dwarf section
    // that the label was found in and the offset where the label
    // exists. This map is used to patch the dwarf bytestream
    // after it has been parsed and fill in all of the holes.
    std::map<std::string, unsigned> tokenDefMap_;
    std::map<std::string,
      std::set<std::pair<DwarfSection, unsigned> > > tokenUseMap_;
    // A map very similiar to the tokenUseMap_, but only deals with
    // calculations which must be handled after all value offsets
    // are known.
    std::map<std::string,
      std::set<std::pair<DwarfSection, unsigned> > > calcMap_;

  }; // AMDILDwarf

  // A class that handles a single compilation
  // unit from the LLVM AMDIL backend. This class
  // provides a generic interface that returns
  // the name of all the kernels and all of
  // the kernels in a an CAL compilable form.
  // This class also provides the interface to
  // access the dwarf debug class for each compilation unit.
  // This class also takes a metadata string and parse it.
  // You can then call getMD() to get the metadata back in text, or you can
  // call getMDStruct() to get the metadata in binary format.
  class CompUnit {
    friend raw_ostream& operator<<(raw_ostream &, CompUnit&);
    public:
    typedef enum {
      IL_STATE, // This is for the IL
      MACRO_STATE,
      DBG_STATE,
      MD_STATE,
      DATA_STATE
    } ComponentStates;
    CompUnit(std::istream& in);
    CompUnit(const std::string& src);
    virtual ~CompUnit();

    // Helper functions for debugging.
    virtual void dump();

    // This adds a list of components to the compilation unit
    // for processing after the whole input has been parsed.
    void addComponents(std::list<Component*>* newComps,
        ComponentStates State);

    // Returns the number of kernels that exist in this compilation
    // unit.
    size_t getNumKernels();

    // Returns true if there are no kernels, false otherwise.
    bool empty();

    // Sets the main function to the function passed in
    void setMain(MainFunc*);

    // Turn line number printing on/off
    void setLineNumberDisplay(bool val);

    // Functions related to debugging
    // The getDebugData function takes an integer ID between
    // 0 and size()-1 and returns a structure that allows
    // all of the debug information to be queried.
    AMDILDwarf* getDebugData();


    // Return the IL for the whole compilation unit in a std::string.
    // This includes all kernels and is equivalent to the output of
    // the backend without all metadata/dwarf/data sections.
    std::string getILStr();

    // Functions related to getting kernels for SC compilation

    // The getKernelStr function takes an integer ID between
    // 0 and NumKernels - 1 and returns a std::string that contains the
    // kernel for that specific kernel.
    std::string getKernelStr(unsigned id);


    // The getKernelMD function takes an integer ID between
    // 0 and NumKernels - 1 and returns a std::string that contains
    // the metadata for that specific kernel.
    std::string getKernelMD(unsigned id);

    // Called when no kernel was parsed, only metadata string.
    // Return the metadata string.
    std::string getMD();

    // The getKernelName function takes an integer ID bewteen
    // 0 and NumKernels and returns a std::string that contains the
    // kernel name for that specific kernel
    std::string getKernelName(unsigned id);

    // Function that returns a pointer to an array of
    // AMDILDataSections, with the number of data sections
    // stored in the output argument.
    // The data that is returned is allocated dynamically and
    // must be released via the releaseDataSections function.
    AMDILDataSection* getDataSections(unsigned* numSections);

    // Release all of the memory allocted for the AMDILDataSection struct.
    // On success, data is set to NULL and true is returned, false otherwise.
    bool releaseDataSections(AMDILDataSection** data, unsigned numSections);

    // The getKernelMD function takes an integer ID between
    // 0 and NumKernels - 1 and returns a struct that contains
    // the metadata for that specific kernel and all dependent
    // functions.
    AMDILMetadata getKernelMDStruct(unsigned id);

    // Called when no kernel was parsed, only metadata string.
    // Return the metadata struct.
    AMDILMetadata getMDStruct();

    // Return true if an error occured in parsing.
    bool hasError() { return mError; }

    // Return the error message of the parser.
    std::string errorMsg() { return mMsg; }

    private:
    protected:
    // Get the set of all dependent functions
    std::set<unsigned> getAllDepFuncs(ILFunc*);
    // Vector that holds all of the kernels in this compilation unit.
    std::vector<ILFunc*> kernels_;
    // Map between the function ID and a pointer to the function itself.
    std::map<unsigned, ILFunc*> funcsMap_;
    // Vector that holds all of the IL functions in the compilation unit.
    std::vector<ILFunc*> ilfuncs_;
    // Vector that holds all of the macro sections in the compilation unit.
    std::vector<Macro*> macros_;
    // Vector that holds all of the debug sections in the compilation unit.
    std::vector<DBSection*> debug_;
    // Vector that holds all of the data sections in the compilation unit.
    std::vector<DataSection*> data_;
    // Vector that holds all of the metadata sections in the comp unit.
    std::vector<MDBlock*> metadata_;
    // This vector keeps all of the components in order in that
    // they were read and allows us to dump the data back out
    // in the same way that it came in. It also allows us to
    // iterator over all the components of the compilation unit
    // without having to duplicate loop code
    std::list<Component*>* compBlocks_;
    // Pointer to the assembled dwarf data for the compilation unit
    AMDILDwarf* debugData_;
    // Pointer to the main function that was parsed out of the comp unit.
    MainFunc* main_;
    // Pointer to the dummy macro if it exists in the compilation unit.
    DummyMacro* dummyMacro_;
    private:
    CompUnit(); // We don't want a default constructor

    // Initialize the CompUnit class and parse the input
    // stream.
    void init(std::istream& in);

    // Processes the data after everything has been
    // gathered.
    void process();
    // Flag to display line numbers for the IL statements
    bool dispLineNum_;

    // Flag to specify if an error occured.
    bool mError;

    // Error message.
    std::string mMsg;
  }; // CompUnit
} // llvm namespace
#endif // _AMDIL_MDINTERFACE_H_
