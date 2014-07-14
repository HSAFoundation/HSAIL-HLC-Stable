// University of Illinois/NCSA
// Open Source License
//
// Copyright (c) 2013, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Developed by:
//
//     HSA Team
//
//     Advanced Micro Devices, Inc
//
//     www.amd.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimers.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimers in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the names of the LLVM Team, University of Illinois at
//       Urbana-Champaign, nor the names of its contributors may be used to
//       endorse or promote products derived from this Software without specific
//       prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.
class AluModifier;
class Directive;
class BlockEnd;
class BlockNumeric;
class BlockString;
class DirectiveCode;
class BlockStart;
class DirectiveArgScopeEnd;
class DirectiveArgScopeStart;
class DirectiveCallableBase;
class DirectiveExecutable;
class DirectiveFunction;
class DirectiveKernel;
class DirectiveSignature;
class DirectiveComment;
class DirectiveControl;
class DirectiveExtension;
class DirectiveFbarrier;
class DirectiveImageProperties;
class DirectiveLabel;
class DirectiveLabelInit;
class DirectiveLabelTargets;
class DirectiveLoc;
class DirectiveOpaqueInit;
class DirectiveImageInit;
class DirectiveSamplerInit;
class DirectivePragma;
class DirectiveSamplerProperties;
class DirectiveVariable;
class DirectiveVariableInit;
class DirectiveVersion;
class DirectiveSignatureArgument;
class ExecutableModifier;
class Inst;
class InstAddr;
class InstAtomic;
class InstBasic;
class InstBr;
class InstCmp;
class InstCvt;
class InstImage;
class InstLane;
class InstMem;
class InstMemFence;
class InstMod;
class InstQueryImage;
class InstQuerySampler;
class InstQueue;
class InstSeg;
class InstSegCvt;
class InstSignal;
class InstSourceType;
class InstNone;
class MemoryModifier;
class Operand;
class OperandAddress;
class OperandImmed;
class OperandList;
class OperandArgumentList;
class OperandFunctionList;
class OperandRef;
class OperandFbarrierRef;
class OperandFunctionRef;
class OperandLabelRef;
class OperandLabelTargetsRef;
class OperandLabelVariableRef;
class OperandSignatureRef;
class OperandReg;
class OperandVector;
class OperandWavesize;
class SegCvtModifier;
class SymbolModifier;


class AluModifier : public ItemBase {
public:

	/// accessors
	ValRef<uint16_t>                                   allBits();
	BFValRef<Brig::BrigRound8_t,0,4>                   round();
	BitValRef<4>                                       ftz();


	/// constructors
	AluModifier()                           : ItemBase() { } 
	AluModifier(MySection* s, Offset o)     : ItemBase(s, o) { } 
	AluModifier& operator=(const AluModifier& rhs) { reset(rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigAluModifier BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "AluModifier"; }
	void initBrig(); 
};

/// @addtogroup Directives
/// @{
/// base class for all directive items.
class Directive : public ItemBase {
    // children: BrigDirectiveSamplerInit,BrigDirectiveLabelInit,BrigDirectiveVariableInit,BrigDirectiveImageInit,BrigDirectiveLoc,BrigDirectiveOpaqueInit,BrigDirectiveSignature,BrigDirectiveExecutable,BrigDirectiveFunction,BrigDirectiveArgScopeStart,BrigDirectiveArgScopeEnd,BrigDirectiveCallableBase,BrigBlockEnd,BrigDirectiveVersion,BrigDirectiveSamplerProperties,BrigDirectiveLabel,BrigBlockString,BrigBlockNumeric,BrigDirectiveLabelTargets,BrigDirectiveControl,BrigDirectiveExtension,BrigDirectiveVariable,BrigDirectiveComment,BrigBlockStart,BrigDirectivePragma,BrigDirectiveFbarrier,BrigDirectiveImageProperties,BrigDirectiveKernel,BrigDirectiveCode
public:

	typedef Directive Kind;

	/// accessors
	/// item size.
	ValRef<uint16_t>                                   size();
	/// item kind. One of BrigDirectiveKinds enum values.
	EnumValRef<Brig::BrigDirectiveKinds,uint16_t>      kind();


	/// constructors
	Directive()                           : ItemBase() { } 
	Directive(MySection* s, Offset o)     : ItemBase(s, o) { } 
	Directive(BrigContainer* c, Offset o) : ItemBase(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Directive& rhs) { return true; } 
	Directive& operator=(const Directive& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirective BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// root utilities
	Offset  brigSize() const { return brig()->size; }
	Directive next() const { return Directive(section(), brigOffset() + brigSize()); }
};

/// end of block.
class BlockEnd : public Directive {
public:

	/// accessors


	/// constructors
	BlockEnd()                           : Directive() { } 
	BlockEnd(MySection* s, Offset o)     : Directive(s, o) { } 
	BlockEnd(BrigContainer* c, Offset o) : Directive(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_BLOCK_END;
	}
	BlockEnd(const Kind& rhs) { assignItem(*this,rhs); } 
	BlockEnd& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigBlockEnd BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "BlockEnd"; }
	void initBrig(); 
};

/// numeric data inside block.
class BlockNumeric : public Directive {
public:

	/// accessors
	ValRef<uint16_t>                                   type();
	ValRef<uint32_t>                                   elementCount();
	DataItemRef                                        data();
	template<typename T> DataItemRefT<T>               dataAs();
	ValRef<uint32_t>                                   dataAs();


	/// constructors
	BlockNumeric()                           : Directive() { } 
	BlockNumeric(MySection* s, Offset o)     : Directive(s, o) { } 
	BlockNumeric(BrigContainer* c, Offset o) : Directive(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_BLOCK_NUMERIC;
	}
	BlockNumeric(const Kind& rhs) { assignItem(*this,rhs); } 
	BlockNumeric& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigBlockNumeric BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "BlockNumeric"; }
	void initBrig(); 
};

/// string inside block.
class BlockString : public Directive {
public:

	/// accessors
	StrRef                                             string();


	/// constructors
	BlockString()                           : Directive() { } 
	BlockString(MySection* s, Offset o)     : Directive(s, o) { } 
	BlockString(BrigContainer* c, Offset o) : Directive(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_BLOCK_STRING;
	}
	BlockString(const Kind& rhs) { assignItem(*this,rhs); } 
	BlockString& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigBlockString BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "BlockString"; }
	void initBrig(); 
};

/// base class for directives that positioned in instruction stream.
class DirectiveCode : public Directive {
    // children: BrigDirectiveLabelTargets,BrigDirectiveControl,BrigDirectiveExtension,BrigDirectiveVariable,BrigDirectiveFbarrier,BrigDirectiveImageProperties,BrigBlockStart,BrigDirectiveComment,BrigDirectivePragma,BrigDirectiveKernel,BrigDirectiveImageInit,BrigDirectiveLoc,BrigDirectiveSamplerInit,BrigDirectiveVariableInit,BrigDirectiveLabelInit,BrigDirectiveFunction,BrigDirectiveArgScopeStart,BrigDirectiveSignature,BrigDirectiveOpaqueInit,BrigDirectiveExecutable,BrigDirectiveVersion,BrigDirectiveArgScopeEnd,BrigDirectiveCallableBase,BrigDirectiveLabel,BrigDirectiveSamplerProperties
public:

	/// accessors
	/// Location in the instruction stream corresponding to this directive.
	ItemRef<Inst>                                      code();


	/// constructors
	DirectiveCode()                           : Directive() { } 
	DirectiveCode(MySection* s, Offset o)     : Directive(s, o) { } 
	DirectiveCode(BrigContainer* c, Offset o) : Directive(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_LABEL_TARGETS
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_CONTROL
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_EXTENSION
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_VARIABLE
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_FBARRIER
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_IMAGE_PROPERTIES
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_BLOCK_START
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_COMMENT
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_PRAGMA
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_KERNEL
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_IMAGE_INIT
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_LOC
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_SAMPLER_INIT
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_VARIABLE_INIT
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_LABEL_INIT
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_FUNCTION
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_ARG_SCOPE_START
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_SIGNATURE
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_VERSION
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_ARG_SCOPE_END
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_LABEL
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_SAMPLER_PROPERTIES;
	}
	DirectiveCode(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveCode& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveCode BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }
};

/// start block of data.
class BlockStart : public DirectiveCode {
public:

	/// accessors
	StrRef                                             name();


	/// constructors
	BlockStart()                           : DirectiveCode() { } 
	BlockStart(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	BlockStart(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_BLOCK_START;
	}
	BlockStart(const Kind& rhs) { assignItem(*this,rhs); } 
	BlockStart& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigBlockStart BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "BlockStart"; }
	void initBrig(); 
};

class DirectiveArgScopeEnd : public DirectiveCode {
public:

	/// accessors


	/// constructors
	DirectiveArgScopeEnd()                           : DirectiveCode() { } 
	DirectiveArgScopeEnd(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveArgScopeEnd(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_ARG_SCOPE_END;
	}
	DirectiveArgScopeEnd(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveArgScopeEnd& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveArgScopeEnd BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveArgScopeEnd"; }
	void initBrig(); 
};

class DirectiveArgScopeStart : public DirectiveCode {
public:

	/// accessors


	/// constructors
	DirectiveArgScopeStart()                           : DirectiveCode() { } 
	DirectiveArgScopeStart(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveArgScopeStart(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_ARG_SCOPE_START;
	}
	DirectiveArgScopeStart(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveArgScopeStart& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveArgScopeStart BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveArgScopeStart"; }
	void initBrig(); 
};

class DirectiveCallableBase : public DirectiveCode {
    // children: BrigDirectiveFunction,BrigDirectiveExecutable,BrigDirectiveSignature,BrigDirectiveKernel
public:

	/// accessors
	StrRef                                             name();
	ValRef<uint16_t>                                   inArgCount();
	ValRef<uint16_t>                                   outArgCount();


	/// constructors
	DirectiveCallableBase()                           : DirectiveCode() { } 
	DirectiveCallableBase(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveCallableBase(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_FUNCTION
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_SIGNATURE
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_KERNEL;
	}
	DirectiveCallableBase(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveCallableBase& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveCallableBase BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }
};

/// common ancestor class for kernel/function directives.
class DirectiveExecutable : public DirectiveCallableBase {
    // children: BrigDirectiveKernel,BrigDirectiveFunction
public:

	/// accessors
	ItemRef<Directive>                                 firstInArg();
	ItemRef<Directive>                                 firstScopedDirective();
	ItemRef<Directive>                                 nextTopLevelDirective();
	ValRef<uint32_t>                                   instCount();
	ExecutableModifier                                 modifier();


	/// constructors
	DirectiveExecutable()                           : DirectiveCallableBase() { } 
	DirectiveExecutable(MySection* s, Offset o)     : DirectiveCallableBase(s, o) { } 
	DirectiveExecutable(BrigContainer* c, Offset o) : DirectiveCallableBase(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_KERNEL
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_FUNCTION;
	}
	DirectiveExecutable(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveExecutable& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveExecutable BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }
};

/// function directive.
class DirectiveFunction : public DirectiveExecutable {
public:

	/// accessors


	/// constructors
	DirectiveFunction()                           : DirectiveExecutable() { } 
	DirectiveFunction(MySection* s, Offset o)     : DirectiveExecutable(s, o) { } 
	DirectiveFunction(BrigContainer* c, Offset o) : DirectiveExecutable(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_FUNCTION;
	}
	DirectiveFunction(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveFunction& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveFunction BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveFunction"; }
	void initBrig(); 
};

/// kernel directive.
class DirectiveKernel : public DirectiveExecutable {
public:

	/// accessors


	/// constructors
	DirectiveKernel()                           : DirectiveExecutable() { } 
	DirectiveKernel(MySection* s, Offset o)     : DirectiveExecutable(s, o) { } 
	DirectiveKernel(BrigContainer* c, Offset o) : DirectiveExecutable(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_KERNEL;
	}
	DirectiveKernel(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveKernel& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveKernel BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveKernel"; }
	void initBrig(); 
};

/// function signature.
class DirectiveSignature : public DirectiveCallableBase {
public:

	/// accessors
	// overridden, was ValRef<uint16_t> inArgCount
	ValRef<uint16_t>                                   inCount();
	// overridden, was ValRef<uint16_t> outArgCount
	ValRef<uint16_t>                                   outCount();
	DirectiveSignatureArguments                        args();
	DirectiveSignatureArgument                         args(int index);


	/// constructors
	DirectiveSignature()                           : DirectiveCallableBase() { } 
	DirectiveSignature(MySection* s, Offset o)     : DirectiveCallableBase(s, o) { } 
	DirectiveSignature(BrigContainer* c, Offset o) : DirectiveCallableBase(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_SIGNATURE;
	}
	DirectiveSignature(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveSignature& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveSignature BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveSignature"; }
	void initBrig(); 
};

/// comment directive.
class DirectiveComment : public DirectiveCode {
public:

	/// accessors
	StrRef                                             name();


	/// constructors
	DirectiveComment()                           : DirectiveCode() { } 
	DirectiveComment(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveComment(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_COMMENT;
	}
	DirectiveComment(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveComment& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveComment BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveComment"; }
	void initBrig(); 
};

/// control directive.
class DirectiveControl : public DirectiveCode {
public:

	/// accessors
	EnumValRef<Brig::BrigControlDirective,uint16_t>    control();
	ValRef<uint16_t>                                   type();
	ValRef<uint16_t>                                   elementCount();
	ControlValues                                      values();
	ItemRef<Operand>                                   values(int index);


	/// constructors
	DirectiveControl()                           : DirectiveCode() { } 
	DirectiveControl(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveControl(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_CONTROL;
	}
	DirectiveControl(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveControl& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveControl BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveControl"; }
	void initBrig(); 
};

/// @}
/// extension directive.
class DirectiveExtension : public DirectiveCode {
public:

	/// accessors
	StrRef                                             name();


	/// constructors
	DirectiveExtension()                           : DirectiveCode() { } 
	DirectiveExtension(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveExtension(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_EXTENSION;
	}
	DirectiveExtension(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveExtension& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveExtension BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveExtension"; }
	void initBrig(); 
};

class DirectiveFbarrier : public DirectiveCode {
public:

	/// accessors
	StrRef                                             name();


	/// constructors
	DirectiveFbarrier()                           : DirectiveCode() { } 
	DirectiveFbarrier(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveFbarrier(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_FBARRIER;
	}
	DirectiveFbarrier(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveFbarrier& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveFbarrier BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveFbarrier"; }
	void initBrig(); 
};

class DirectiveImageProperties : public DirectiveCode {
public:

	/// accessors
	ValRef<uint32_t>                                   width();
	ValRef<uint32_t>                                   height();
	ValRef<uint32_t>                                   depth();
	ValRef<uint32_t>                                   array();
	EnumValRef<Brig::BrigImageGeometry,uint8_t>        geometry();
	EnumValRef<Brig::BrigImageChannelOrder,uint8_t>    channelOrder();
	EnumValRef<Brig::BrigImageChannelType,uint8_t>     channelType();


	/// constructors
	DirectiveImageProperties()                           : DirectiveCode() { } 
	DirectiveImageProperties(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveImageProperties(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_IMAGE_PROPERTIES;
	}
	DirectiveImageProperties(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveImageProperties& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveImageProperties BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveImageProperties"; }
	void initBrig(); 
};

/// label directive
class DirectiveLabel : public DirectiveCode {
public:

	/// accessors
	StrRef                                             name();


	/// constructors
	DirectiveLabel()                           : DirectiveCode() { } 
	DirectiveLabel(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveLabel(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_LABEL;
	}
	DirectiveLabel(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveLabel& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveLabel BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveLabel"; }
	void initBrig(); 
};

class DirectiveLabelInit : public DirectiveCode {
public:

	/// accessors
	ValRef<uint16_t>                                   elementCount();
	LabelInitList                                      labels();
	ItemRef<DirectiveLabel>                            labels(int index);


	/// constructors
	DirectiveLabelInit()                           : DirectiveCode() { } 
	DirectiveLabelInit(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveLabelInit(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_LABEL_INIT;
	}
	DirectiveLabelInit(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveLabelInit& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveLabelInit BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveLabelInit"; }
	void initBrig(); 
};

class DirectiveLabelTargets : public DirectiveCode {
public:

	/// accessors
	StrRef                                             name();
	ValRef<uint16_t>                                   elementCount();
	LabelTargetsList                                   labels();
	ItemRef<DirectiveLabel>                            labels(int index);


	/// constructors
	DirectiveLabelTargets()                           : DirectiveCode() { } 
	DirectiveLabelTargets(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveLabelTargets(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_LABEL_TARGETS;
	}
	DirectiveLabelTargets(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveLabelTargets& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveLabelTargets BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveLabelTargets"; }
	void initBrig(); 
};

class DirectiveLoc : public DirectiveCode {
public:

	/// accessors
	StrRef                                             filename();
	ValRef<uint32_t>                                   line();
	ValRef<uint32_t>                                   column();


	/// constructors
	DirectiveLoc()                           : DirectiveCode() { } 
	DirectiveLoc(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveLoc(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_LOC;
	}
	DirectiveLoc(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveLoc& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveLoc BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveLoc"; }
	void initBrig(); 
};

class DirectiveOpaqueInit : public DirectiveCode {
    // children: BrigDirectiveImageInit,BrigDirectiveSamplerInit
public:

	/// accessors
	ValRef<uint16_t>                                   imageCount();
	ValRef<uint16_t>                                   elementCount();
	OpaqueInitList                                     objects();
	ItemRef<Directive>                                 objects(int index);


	/// constructors
	DirectiveOpaqueInit()                           : DirectiveCode() { } 
	DirectiveOpaqueInit(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveOpaqueInit(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_IMAGE_INIT
		    || rhs.brig()->kind == Brig::BRIG_DIRECTIVE_SAMPLER_INIT;
	}
	DirectiveOpaqueInit(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveOpaqueInit& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveOpaqueInit BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }
};

class DirectiveImageInit : public DirectiveOpaqueInit {
public:

	/// accessors
	ImageInitList                                      images();
	ItemRef<DirectiveImageProperties>                  images(int index);


	/// constructors
	DirectiveImageInit()                           : DirectiveOpaqueInit() { } 
	DirectiveImageInit(MySection* s, Offset o)     : DirectiveOpaqueInit(s, o) { } 
	DirectiveImageInit(BrigContainer* c, Offset o) : DirectiveOpaqueInit(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_IMAGE_INIT;
	}
	DirectiveImageInit(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveImageInit& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveImageInit BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveImageInit"; }
	void initBrig(); 
};

class DirectiveSamplerInit : public DirectiveOpaqueInit {
public:

	/// accessors
	ValRef<uint16_t>                                   samplerCount();
	SamplerInitList                                    samplers();
	ItemRef<DirectiveSamplerProperties>                samplers(int index);


	/// constructors
	DirectiveSamplerInit()                           : DirectiveOpaqueInit() { } 
	DirectiveSamplerInit(MySection* s, Offset o)     : DirectiveOpaqueInit(s, o) { } 
	DirectiveSamplerInit(BrigContainer* c, Offset o) : DirectiveOpaqueInit(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_SAMPLER_INIT;
	}
	DirectiveSamplerInit(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveSamplerInit& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveSamplerInit BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveSamplerInit"; }
	void initBrig(); 
};

class DirectivePragma : public DirectiveCode {
public:

	/// accessors
	StrRef                                             name();


	/// constructors
	DirectivePragma()                           : DirectiveCode() { } 
	DirectivePragma(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectivePragma(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_PRAGMA;
	}
	DirectivePragma(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectivePragma& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectivePragma BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectivePragma"; }
	void initBrig(); 
};

class DirectiveSamplerProperties : public DirectiveCode {
public:

	/// accessors
	EnumValRef<Brig::BrigSamplerCoordNormalization,uint8_t> coord();
	EnumValRef<Brig::BrigSamplerFilter,uint8_t>        filter();
	EnumValRef<Brig::BrigSamplerAddressing,uint8_t>    addressing();


	/// constructors
	DirectiveSamplerProperties()                           : DirectiveCode() { } 
	DirectiveSamplerProperties(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveSamplerProperties(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_SAMPLER_PROPERTIES;
	}
	DirectiveSamplerProperties(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveSamplerProperties& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveSamplerProperties BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveSamplerProperties"; }
	void initBrig(); 
};

class DirectiveVariable : public DirectiveCode {
public:

	/// accessors
	StrRef                                             name();
	ItemRef<Directive>                                 init();
	ValRef<uint16_t>                                   type();
	EnumValRef<Brig::BrigSegment,uint8_t>              segment();
	EnumValRef<Brig::BrigAlignment,uint8_t>            align();
	ValRef<uint32_t>                                   dimLo();
	ValRef<uint64_t>                                   dim();
	ValRef<uint32_t>                                   dimHi();
	SymbolModifier                                     modifier();


	/// constructors
	DirectiveVariable()                           : DirectiveCode() { } 
	DirectiveVariable(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveVariable(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_VARIABLE;
	}
	DirectiveVariable(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveVariable& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveVariable BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveVariable"; }
	void initBrig(); 
};

class DirectiveVariableInit : public DirectiveCode {
public:

	/// accessors
	DataItemRef                                        data();
	template<typename T> DataItemRefT<T>               dataAs();
	ValRef<uint32_t>                                   dataAs();
	ValRef<uint32_t>                                   elementCount();
	ValRef<uint16_t>                                   type();


	/// constructors
	DirectiveVariableInit()                           : DirectiveCode() { } 
	DirectiveVariableInit(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveVariableInit(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_VARIABLE_INIT;
	}
	DirectiveVariableInit(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveVariableInit& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveVariableInit BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveVariableInit"; }
	void initBrig(); 
};

class DirectiveVersion : public DirectiveCode {
public:

	/// accessors
	EnumValRef<Brig::BrigVersion,uint32_t>             hsailMajor();
	EnumValRef<Brig::BrigVersion,uint32_t>             hsailMinor();
	EnumValRef<Brig::BrigVersion,uint32_t>             brigMajor();
	EnumValRef<Brig::BrigVersion,uint32_t>             brigMinor();
	EnumValRef<Brig::BrigProfile,uint8_t>              profile();
	EnumValRef<Brig::BrigMachineModel,uint8_t>         machineModel();


	/// constructors
	DirectiveVersion()                           : DirectiveCode() { } 
	DirectiveVersion(MySection* s, Offset o)     : DirectiveCode(s, o) { } 
	DirectiveVersion(BrigContainer* c, Offset o) : DirectiveCode(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_DIRECTIVE_VERSION;
	}
	DirectiveVersion(const Kind& rhs) { assignItem(*this,rhs); } 
	DirectiveVersion& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveVersion BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveVersion"; }
	void initBrig(); 
};

/// element describing properties of function signature argument.
class DirectiveSignatureArgument : public ItemBase {
public:

	/// accessors
	ValRef<uint16_t>                                   type();
	EnumValRef<Brig::BrigAlignment,uint8_t>            align();
	SymbolModifier                                     modifier();
	ValRef<uint32_t>                                   dimLo();
	ValRef<uint64_t>                                   dim();
	ValRef<uint32_t>                                   dimHi();


	/// constructors
	DirectiveSignatureArgument()                           : ItemBase() { } 
	DirectiveSignatureArgument(MySection* s, Offset o)     : ItemBase(s, o) { } 
	DirectiveSignatureArgument& operator=(const DirectiveSignatureArgument& rhs) { reset(rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigDirectiveSignatureArgument BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "DirectiveSignatureArgument"; }
	void initBrig(); 
};

class ExecutableModifier : public ItemBase {
public:

	/// accessors
	ValRef<uint8_t>                                    allBits();
	BFValRef<Brig::BrigLinkage8_t,0,2>                 linkage();
	BitValRef<2>                                       isDeclaration();


	/// constructors
	ExecutableModifier()                           : ItemBase() { } 
	ExecutableModifier(MySection* s, Offset o)     : ItemBase(s, o) { } 
	ExecutableModifier& operator=(const ExecutableModifier& rhs) { reset(rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigExecutableModifier BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "ExecutableModifier"; }
	void initBrig(); 
};

class Inst : public ItemBase {
    // children: BrigInstImage,BrigInstMem,BrigInstMod,BrigInstQuerySampler,BrigInstBasic,BrigInstQueue,BrigInstQueryImage,BrigInstLane,BrigInstAddr,BrigInstSignal,BrigInstSourceType,BrigInstBr,BrigInstSegCvt,BrigInstCvt,BrigInstCmp,BrigInstSeg,BrigInstMemFence,BrigInstAtomic
public:

	typedef Inst Kind;

	/// accessors
	ValRef<uint16_t>                                   size();
	EnumValRef<Brig::BrigInstKinds,uint16_t>           kind();
	EnumValRef<Brig::BrigOpcode,uint16_t>              opcode();
	ValRef<uint16_t>                                   type();
	ItemRef<Operand>                                   operand(int index);


	/// constructors
	Inst()                           : ItemBase() { } 
	Inst(MySection* s, Offset o)     : ItemBase(s, o) { } 
	Inst(BrigContainer* c, Offset o) : ItemBase(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Inst& rhs) { return true; } 
	Inst& operator=(const Inst& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInst BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// root utilities
	Offset  brigSize() const { return brig()->size; }
	Inst next() const { return Inst(section(), brigOffset() + brigSize()); }
};

class InstAddr : public Inst {
public:

	/// accessors
	EnumValRef<Brig::BrigSegment,uint8_t>              segment();


	/// constructors
	InstAddr()                           : Inst() { } 
	InstAddr(MySection* s, Offset o)     : Inst(s, o) { } 
	InstAddr(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_ADDR;
	}
	InstAddr(const Kind& rhs) { assignItem(*this,rhs); } 
	InstAddr& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstAddr BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstAddr"; }
	void initBrig(); 
};

class InstAtomic : public Inst {
public:

	/// accessors
	EnumValRef<Brig::BrigSegment,uint8_t>              segment();
	EnumValRef<Brig::BrigMemoryOrder,uint8_t>          memoryOrder();
	EnumValRef<Brig::BrigMemoryScope,uint8_t>          memoryScope();
	EnumValRef<Brig::BrigAtomicOperation,uint8_t>      atomicOperation();
	ValRef<uint8_t>                                    equivClass();


	/// constructors
	InstAtomic()                           : Inst() { } 
	InstAtomic(MySection* s, Offset o)     : Inst(s, o) { } 
	InstAtomic(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_ATOMIC;
	}
	InstAtomic(const Kind& rhs) { assignItem(*this,rhs); } 
	InstAtomic& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstAtomic BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstAtomic"; }
	void initBrig(); 
};

class InstBasic : public Inst {
public:

	/// accessors


	/// constructors
	InstBasic()                           : Inst() { } 
	InstBasic(MySection* s, Offset o)     : Inst(s, o) { } 
	InstBasic(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_BASIC;
	}
	InstBasic(const Kind& rhs) { assignItem(*this,rhs); } 
	InstBasic& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstBasic BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstBasic"; }
	void initBrig(); 
};

class InstBr : public Inst {
public:

	/// accessors
	EnumValRef<Brig::BrigWidth,uint8_t>                width();


	/// constructors
	InstBr()                           : Inst() { } 
	InstBr(MySection* s, Offset o)     : Inst(s, o) { } 
	InstBr(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_BR;
	}
	InstBr(const Kind& rhs) { assignItem(*this,rhs); } 
	InstBr& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstBr BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstBr"; }
	void initBrig(); 
};

class InstCmp : public Inst {
public:

	/// accessors
	ValRef<uint16_t>                                   sourceType();
	AluModifier                                        modifier();
	EnumValRef<Brig::BrigCompareOperation,uint8_t>     compare();
	EnumValRef<Brig::BrigPack,uint8_t>                 pack();


	/// constructors
	InstCmp()                           : Inst() { } 
	InstCmp(MySection* s, Offset o)     : Inst(s, o) { } 
	InstCmp(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_CMP;
	}
	InstCmp(const Kind& rhs) { assignItem(*this,rhs); } 
	InstCmp& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstCmp BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstCmp"; }
	void initBrig(); 
};

class InstCvt : public Inst {
public:

	/// accessors
	ValRef<uint16_t>                                   sourceType();
	AluModifier                                        modifier();


	/// constructors
	InstCvt()                           : Inst() { } 
	InstCvt(MySection* s, Offset o)     : Inst(s, o) { } 
	InstCvt(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_CVT;
	}
	InstCvt(const Kind& rhs) { assignItem(*this,rhs); } 
	InstCvt& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstCvt BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstCvt"; }
	void initBrig(); 
};

class InstImage : public Inst {
public:

	/// accessors
	ValRef<uint16_t>                                   imageType();
	ValRef<uint16_t>                                   coordType();
	EnumValRef<Brig::BrigImageGeometry,uint8_t>        geometry();
	ValRef<uint8_t>                                    equivClass();


	/// constructors
	InstImage()                           : Inst() { } 
	InstImage(MySection* s, Offset o)     : Inst(s, o) { } 
	InstImage(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_IMAGE;
	}
	InstImage(const Kind& rhs) { assignItem(*this,rhs); } 
	InstImage& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstImage BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstImage"; }
	void initBrig(); 
};

class InstLane : public Inst {
public:

	/// accessors
	ValRef<uint16_t>                                   sourceType();
	EnumValRef<Brig::BrigWidth,uint8_t>                width();


	/// constructors
	InstLane()                           : Inst() { } 
	InstLane(MySection* s, Offset o)     : Inst(s, o) { } 
	InstLane(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_LANE;
	}
	InstLane(const Kind& rhs) { assignItem(*this,rhs); } 
	InstLane& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstLane BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstLane"; }
	void initBrig(); 
};

class InstMem : public Inst {
public:

	/// accessors
	EnumValRef<Brig::BrigSegment,uint8_t>              segment();
	EnumValRef<Brig::BrigAlignment,uint8_t>            align();
	ValRef<uint8_t>                                    equivClass();
	EnumValRef<Brig::BrigWidth,uint8_t>                width();
	MemoryModifier                                     modifier();


	/// constructors
	InstMem()                           : Inst() { } 
	InstMem(MySection* s, Offset o)     : Inst(s, o) { } 
	InstMem(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_MEM;
	}
	InstMem(const Kind& rhs) { assignItem(*this,rhs); } 
	InstMem& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstMem BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstMem"; }
	void initBrig(); 
};

class InstMemFence : public Inst {
public:

	/// accessors
	EnumValRef<Brig::BrigMemoryFenceSegments,uint8_t>  segments();
	EnumValRef<Brig::BrigMemoryOrder,uint8_t>          memoryOrder();
	EnumValRef<Brig::BrigMemoryScope,uint8_t>          memoryScope();


	/// constructors
	InstMemFence()                           : Inst() { } 
	InstMemFence(MySection* s, Offset o)     : Inst(s, o) { } 
	InstMemFence(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_MEM_FENCE;
	}
	InstMemFence(const Kind& rhs) { assignItem(*this,rhs); } 
	InstMemFence& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstMemFence BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstMemFence"; }
	void initBrig(); 
};

class InstMod : public Inst {
public:

	/// accessors
	AluModifier                                        modifier();
	EnumValRef<Brig::BrigPack,uint8_t>                 pack();


	/// constructors
	InstMod()                           : Inst() { } 
	InstMod(MySection* s, Offset o)     : Inst(s, o) { } 
	InstMod(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_MOD;
	}
	InstMod(const Kind& rhs) { assignItem(*this,rhs); } 
	InstMod& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstMod BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstMod"; }
	void initBrig(); 
};

class InstQueryImage : public Inst {
public:

	/// accessors
	// overridden, was ItemRef<Operand> operand
	ItemRef<Operand>                                   operands(int index);
	ValRef<uint16_t>                                   imageType();
	EnumValRef<Brig::BrigImageGeometry,uint8_t>        geometry();
	EnumValRef<Brig::BrigImageQuery,uint8_t>           imageQuery();


	/// constructors
	InstQueryImage()                           : Inst() { } 
	InstQueryImage(MySection* s, Offset o)     : Inst(s, o) { } 
	InstQueryImage(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_QUERY_IMAGE;
	}
	InstQueryImage(const Kind& rhs) { assignItem(*this,rhs); } 
	InstQueryImage& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstQueryImage BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstQueryImage"; }
	void initBrig(); 
};

class InstQuerySampler : public Inst {
public:

	/// accessors
	// overridden, was ItemRef<Operand> operand
	ItemRef<Operand>                                   operands(int index);
	EnumValRef<Brig::BrigSamplerQuery,uint8_t>         samplerQuery();


	/// constructors
	InstQuerySampler()                           : Inst() { } 
	InstQuerySampler(MySection* s, Offset o)     : Inst(s, o) { } 
	InstQuerySampler(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_QUERY_SAMPLER;
	}
	InstQuerySampler(const Kind& rhs) { assignItem(*this,rhs); } 
	InstQuerySampler& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstQuerySampler BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstQuerySampler"; }
	void initBrig(); 
};

class InstQueue : public Inst {
public:

	/// accessors
	// overridden, was ItemRef<Operand> operand
	ItemRef<Operand>                                   operands(int index);
	EnumValRef<Brig::BrigSegment,uint8_t>              segment();
	EnumValRef<Brig::BrigMemoryOrder,uint8_t>          memoryOrder();


	/// constructors
	InstQueue()                           : Inst() { } 
	InstQueue(MySection* s, Offset o)     : Inst(s, o) { } 
	InstQueue(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_QUEUE;
	}
	InstQueue(const Kind& rhs) { assignItem(*this,rhs); } 
	InstQueue& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstQueue BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstQueue"; }
	void initBrig(); 
};

class InstSeg : public Inst {
public:

	/// accessors
	EnumValRef<Brig::BrigSegment,uint8_t>              segment();


	/// constructors
	InstSeg()                           : Inst() { } 
	InstSeg(MySection* s, Offset o)     : Inst(s, o) { } 
	InstSeg(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_SEG;
	}
	InstSeg(const Kind& rhs) { assignItem(*this,rhs); } 
	InstSeg& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstSeg BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstSeg"; }
	void initBrig(); 
};

class InstSegCvt : public Inst {
public:

	/// accessors
	// overridden, was ItemRef<Operand> operand
	ItemRef<Operand>                                   operands(int index);
	ValRef<uint16_t>                                   sourceType();
	EnumValRef<Brig::BrigSegment,uint8_t>              segment();
	SegCvtModifier                                     modifier();


	/// constructors
	InstSegCvt()                           : Inst() { } 
	InstSegCvt(MySection* s, Offset o)     : Inst(s, o) { } 
	InstSegCvt(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_SEG_CVT;
	}
	InstSegCvt(const Kind& rhs) { assignItem(*this,rhs); } 
	InstSegCvt& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstSegCvt BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstSegCvt"; }
	void initBrig(); 
};

class InstSignal : public Inst {
public:

	/// accessors
	ValRef<uint16_t>                                   signalType();
	EnumValRef<Brig::BrigMemoryOrder,uint8_t>          memoryOrder();
	EnumValRef<Brig::BrigAtomicOperation,uint8_t>      signalOperation();


	/// constructors
	InstSignal()                           : Inst() { } 
	InstSignal(MySection* s, Offset o)     : Inst(s, o) { } 
	InstSignal(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_SIGNAL;
	}
	InstSignal(const Kind& rhs) { assignItem(*this,rhs); } 
	InstSignal& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstSignal BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstSignal"; }
	void initBrig(); 
};

class InstSourceType : public Inst {
public:

	/// accessors
	// overridden, was ItemRef<Operand> operand
	ItemRef<Operand>                                   operands(int index);
	ValRef<uint16_t>                                   sourceType();


	/// constructors
	InstSourceType()                           : Inst() { } 
	InstSourceType(MySection* s, Offset o)     : Inst(s, o) { } 
	InstSourceType(BrigContainer* c, Offset o) : Inst(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_INST_SOURCE_TYPE;
	}
	InstSourceType(const Kind& rhs) { assignItem(*this,rhs); } 
	InstSourceType& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstSourceType BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstSourceType"; }
	void initBrig(); 
};

class InstNone : public ItemBase {
public:

	/// accessors
	ValRef<uint16_t>                                   size();
	EnumValRef<Brig::BrigInstKinds,uint16_t>           kind();


	/// constructors
	InstNone()                           : ItemBase() { } 
	InstNone(MySection* s, Offset o)     : ItemBase(s, o) { } 
	InstNone& operator=(const InstNone& rhs) { reset(rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigInstNone BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "InstNone"; }
	void initBrig(); 
};

class MemoryModifier : public ItemBase {
public:

	/// accessors
	ValRef<uint8_t>                                    allBits();
	BitValRef<0>                                       isConst();


	/// constructors
	MemoryModifier()                           : ItemBase() { } 
	MemoryModifier(MySection* s, Offset o)     : ItemBase(s, o) { } 
	MemoryModifier& operator=(const MemoryModifier& rhs) { reset(rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigMemoryModifier BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "MemoryModifier"; }
	void initBrig(); 
};

class Operand : public ItemBase {
    // children: BrigOperandVector,BrigOperandReg,BrigOperandSignatureRef,BrigOperandFunctionRef,BrigOperandArgumentList,BrigOperandAddress,BrigOperandWavesize,BrigOperandRef,BrigOperandImmed,BrigOperandFunctionList,BrigOperandFbarrierRef,BrigOperandLabelRef,BrigOperandLabelTargetsRef,BrigOperandList,BrigOperandLabelVariableRef
public:

	typedef Operand Kind;

	/// accessors
	ValRef<uint16_t>                                   size();
	EnumValRef<Brig::BrigOperandKinds,uint16_t>        kind();


	/// constructors
	Operand()                           : ItemBase() { } 
	Operand(MySection* s, Offset o)     : ItemBase(s, o) { } 
	Operand(BrigContainer* c, Offset o) : ItemBase(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Operand& rhs) { return true; } 
	Operand& operator=(const Operand& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperand BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// root utilities
	Offset  brigSize() const { return brig()->size; }
	Operand next() const { return Operand(section(), brigOffset() + brigSize()); }
};

class OperandAddress : public Operand {
public:

	/// accessors
	ItemRef<DirectiveVariable>                         symbol();
	StrRef                                             reg();
	ValRef<uint32_t>                                   offsetLo();
	ValRef<uint64_t>                                   offset();
	ValRef<uint32_t>                                   offsetHi();


	/// constructors
	OperandAddress()                           : Operand() { } 
	OperandAddress(MySection* s, Offset o)     : Operand(s, o) { } 
	OperandAddress(BrigContainer* c, Offset o) : Operand(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_ADDRESS;
	}
	OperandAddress(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandAddress& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandAddress BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandAddress"; }
	void initBrig(); 
};

class OperandImmed : public Operand {
public:

	/// accessors
	ValRef<uint16_t>                                   byteCount();
	ValRef<uint8_t>                                    bytes(int index);


	/// constructors
	OperandImmed()                           : Operand() { } 
	OperandImmed(MySection* s, Offset o)     : Operand(s, o) { } 
	OperandImmed(BrigContainer* c, Offset o) : Operand(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_IMMED;
	}
	OperandImmed(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandImmed& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandImmed BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandImmed"; }
	void initBrig(); 
};

class OperandList : public Operand {
    // children: BrigOperandFunctionList,BrigOperandArgumentList
public:

	/// accessors
	ValRef<uint16_t>                                   elementCount();
	RefList                                            elements();
	ItemRef<Directive>                                 elements(int index);


	/// constructors
	OperandList()                           : Operand() { } 
	OperandList(MySection* s, Offset o)     : Operand(s, o) { } 
	OperandList(BrigContainer* c, Offset o) : Operand(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_FUNCTION_LIST
		    || rhs.brig()->kind == Brig::BRIG_OPERAND_ARGUMENT_LIST;
	}
	OperandList(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandList& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandList BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }
};

class OperandArgumentList : public OperandList {
public:

	/// accessors


	/// constructors
	OperandArgumentList()                           : OperandList() { } 
	OperandArgumentList(MySection* s, Offset o)     : OperandList(s, o) { } 
	OperandArgumentList(BrigContainer* c, Offset o) : OperandList(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_ARGUMENT_LIST;
	}
	OperandArgumentList(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandArgumentList& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandArgumentList BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandArgumentList"; }
	void initBrig(); 
};

/// list of arguments. (in, out function arguments).
class OperandFunctionList : public OperandList {
public:

	/// accessors


	/// constructors
	OperandFunctionList()                           : OperandList() { } 
	OperandFunctionList(MySection* s, Offset o)     : OperandList(s, o) { } 
	OperandFunctionList(BrigContainer* c, Offset o) : OperandList(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_FUNCTION_LIST;
	}
	OperandFunctionList(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandFunctionList& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandFunctionList BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandFunctionList"; }
	void initBrig(); 
};

class OperandRef : public Operand {
    // children: BrigOperandFbarrierRef,BrigOperandLabelVariableRef,BrigOperandLabelTargetsRef,BrigOperandLabelRef,BrigOperandSignatureRef,BrigOperandFunctionRef
public:

	/// accessors
	ItemRef<Directive>                                 ref();


	/// constructors
	OperandRef()                           : Operand() { } 
	OperandRef(MySection* s, Offset o)     : Operand(s, o) { } 
	OperandRef(BrigContainer* c, Offset o) : Operand(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_FBARRIER_REF
		    || rhs.brig()->kind == Brig::BRIG_OPERAND_LABEL_VARIABLE_REF
		    || rhs.brig()->kind == Brig::BRIG_OPERAND_LABEL_TARGETS_REF
		    || rhs.brig()->kind == Brig::BRIG_OPERAND_LABEL_REF
		    || rhs.brig()->kind == Brig::BRIG_OPERAND_SIGNATURE_REF
		    || rhs.brig()->kind == Brig::BRIG_OPERAND_FUNCTION_REF;
	}
	OperandRef(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandRef& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandRef BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }
};

class OperandFbarrierRef : public OperandRef {
public:

	/// accessors
	// overridden, was ItemRef<Directive> ref
	ItemRef<DirectiveFbarrier>                         fbar();


	/// constructors
	OperandFbarrierRef()                           : OperandRef() { } 
	OperandFbarrierRef(MySection* s, Offset o)     : OperandRef(s, o) { } 
	OperandFbarrierRef(BrigContainer* c, Offset o) : OperandRef(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_FBARRIER_REF;
	}
	OperandFbarrierRef(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandFbarrierRef& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandFbarrierRef BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandFbarrierRef"; }
	void initBrig(); 
};

class OperandFunctionRef : public OperandRef {
public:

	/// accessors
	// overridden, was ItemRef<Directive> ref
	ItemRef<DirectiveFunction>                         fn();


	/// constructors
	OperandFunctionRef()                           : OperandRef() { } 
	OperandFunctionRef(MySection* s, Offset o)     : OperandRef(s, o) { } 
	OperandFunctionRef(BrigContainer* c, Offset o) : OperandRef(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_FUNCTION_REF;
	}
	OperandFunctionRef(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandFunctionRef& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandFunctionRef BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandFunctionRef"; }
	void initBrig(); 
};

class OperandLabelRef : public OperandRef {
public:

	/// accessors
	ItemRef<DirectiveLabel>                            label();


	/// constructors
	OperandLabelRef()                           : OperandRef() { } 
	OperandLabelRef(MySection* s, Offset o)     : OperandRef(s, o) { } 
	OperandLabelRef(BrigContainer* c, Offset o) : OperandRef(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_LABEL_REF;
	}
	OperandLabelRef(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandLabelRef& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandLabelRef BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandLabelRef"; }
	void initBrig(); 
};

class OperandLabelTargetsRef : public OperandRef {
public:

	/// accessors
	ItemRef<DirectiveLabelTargets>                     targets();


	/// constructors
	OperandLabelTargetsRef()                           : OperandRef() { } 
	OperandLabelTargetsRef(MySection* s, Offset o)     : OperandRef(s, o) { } 
	OperandLabelTargetsRef(BrigContainer* c, Offset o) : OperandRef(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_LABEL_TARGETS_REF;
	}
	OperandLabelTargetsRef(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandLabelTargetsRef& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandLabelTargetsRef BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandLabelTargetsRef"; }
	void initBrig(); 
};

class OperandLabelVariableRef : public OperandRef {
public:

	/// accessors
	ItemRef<DirectiveVariable>                         symbol();


	/// constructors
	OperandLabelVariableRef()                           : OperandRef() { } 
	OperandLabelVariableRef(MySection* s, Offset o)     : OperandRef(s, o) { } 
	OperandLabelVariableRef(BrigContainer* c, Offset o) : OperandRef(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_LABEL_VARIABLE_REF;
	}
	OperandLabelVariableRef(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandLabelVariableRef& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandLabelVariableRef BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandLabelVariableRef"; }
	void initBrig(); 
};

class OperandSignatureRef : public OperandRef {
public:

	/// accessors
	// overridden, was ItemRef<Directive> ref
	ItemRef<DirectiveCallableBase>                     sig();


	/// constructors
	OperandSignatureRef()                           : OperandRef() { } 
	OperandSignatureRef(MySection* s, Offset o)     : OperandRef(s, o) { } 
	OperandSignatureRef(BrigContainer* c, Offset o) : OperandRef(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_SIGNATURE_REF;
	}
	OperandSignatureRef(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandSignatureRef& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandSignatureRef BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandSignatureRef"; }
	void initBrig(); 
};

class OperandReg : public Operand {
public:

	/// accessors
	StrRef                                             reg();


	/// constructors
	OperandReg()                           : Operand() { } 
	OperandReg(MySection* s, Offset o)     : Operand(s, o) { } 
	OperandReg(BrigContainer* c, Offset o) : Operand(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_REG;
	}
	OperandReg(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandReg& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandReg BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandReg"; }
	void initBrig(); 
};

class OperandVector : public Operand {
public:

	/// accessors
	ValRef<uint16_t>                                   operandCount();
	ValRef<uint16_t>                                   elementCount();
	VectorOperandList                                  operand();
	ItemRef<Operand>                                   operand(int index);


	/// constructors
	OperandVector()                           : Operand() { } 
	OperandVector(MySection* s, Offset o)     : Operand(s, o) { } 
	OperandVector(BrigContainer* c, Offset o) : Operand(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_VECTOR;
	}
	OperandVector(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandVector& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandVector BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandVector"; }
	void initBrig(); 
};

class OperandWavesize : public Operand {
public:

	/// accessors


	/// constructors
	OperandWavesize()                           : Operand() { } 
	OperandWavesize(MySection* s, Offset o)     : Operand(s, o) { } 
	OperandWavesize(BrigContainer* c, Offset o) : Operand(&(c->section<Kind>()), o) { } 

	/// assignment
	static bool isAssignable(const Kind& rhs) {
		return rhs.brig()->kind == Brig::BRIG_OPERAND_WAVESIZE;
	}
	OperandWavesize(const Kind& rhs) { assignItem(*this,rhs); } 
	OperandWavesize& operator=(const Kind& rhs) { assignItem(*this,rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigOperandWavesize BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "OperandWavesize"; }
	void initBrig(); 
};

class SegCvtModifier : public ItemBase {
public:

	/// accessors
	ValRef<uint8_t>                                    allBits();
	BitValRef<0>                                       isNoNull();


	/// constructors
	SegCvtModifier()                           : ItemBase() { } 
	SegCvtModifier(MySection* s, Offset o)     : ItemBase(s, o) { } 
	SegCvtModifier& operator=(const SegCvtModifier& rhs) { reset(rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigSegCvtModifier BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "SegCvtModifier"; }
	void initBrig(); 
};

class SymbolModifier : public ItemBase {
public:

	/// accessors
	ValRef<uint8_t>                                    allBits();
	BFValRef<Brig::BrigLinkage8_t,0,2>                 linkage();
	BitValRef<2>                                       isDeclaration();
	BitValRef<3>                                       isConst();
	BitValRef<4>                                       isArray();
	BitValRef<5>                                       isFlexArray();


	/// constructors
	SymbolModifier()                           : ItemBase() { } 
	SymbolModifier(MySection* s, Offset o)     : ItemBase(s, o) { } 
	SymbolModifier& operator=(const SymbolModifier& rhs) { reset(rhs); return *this; }

	/// raw brig access
	typedef Brig::BrigSymbolModifier BrigStruct;
	      BrigStruct* brig()       { return reinterpret_cast<BrigStruct*>      (m_section->getData(m_offset)); }
	const BrigStruct* brig() const { return reinterpret_cast<const BrigStruct*>(m_section->getData(m_offset)); }

	/// final utilities
	static const char *kindName() { return "SymbolModifier"; }
	void initBrig(); 
};

