#include "SCTypes.h"
#include "SCShadersSi.h"
#include "atiid.h"
#include "si_id.h"

#include "SCCommon.h"
#include "SCInterface.h"

#include "Compiler.hpp" // _TEST_COMPILER workarounds

#include "HSAILBrigContainer.h"
#include "HSAILBrigObjectFile.h"

#include <cstdlib>
#include <cstring>
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>

int getRandomSeed() { return -1; }
bool scdevutil_hex_dump = true;

SC_VOID* SC_STDCALL
    scClientLocalAllocShaderMem(
    SC_CLIENT_HANDLE client_handle,
    SC_UINT32 size_in_bytes,
    SC_CLIENT_SHADERMEM_HANDLE mem_handle )
{
    void * pShaderMem = malloc( size_in_bytes );
    if(pShaderMem == 0)
    {
        assert(!"scClientLocalAllocShaderMem(): Malloc failed!");
        return ( SC_VOID* ) 0;
    }

    assert(mem_handle != 0);

    //
    // mem_handle in this case should be &SC_R600PSHWSHADER.hShaderMemHandle
    // What this does is on return of SC_Compile, the HWShader.hShaderMemHandle
    // points to the packed data
    //
    void** pInput = static_cast<void**>(mem_handle);
    *pInput = pShaderMem;

    memset( pShaderMem, 0, size_in_bytes );
    return ( SC_VOID* ) pShaderMem;
}

SC_VOID* SC_STDCALL
    scClientAllocSysMem(
    SC_VOID* ptr, SC_UINT32 size )
{
    void * pSysMem = malloc( size );
    if(pSysMem == 0)
    {
        assert(!"scClientAllocSysMem(): Malloc failed!");
        return ( SC_VOID* ) 0;
    }

    memset( pSysMem, 0, size );
    return ( SC_VOID* ) pSysMem;
}

E_SC_RETURNCODE SC_STDCALL
    scClientFreeSysMem(
    SC_VOID* ptr, SC_VOID* p )
{
    if( p == 0)
    {
        assert(!"scClientFreeSysMem(): Null pointer!");
        return (E_SC_ERROR);
    }

    free( p );
    return E_SC_OK;
}

E_SC_RETURNCODE SC_STDCALL
    scClientQueryRegistry(
    SC_CLIENT_HANDLE ClientHandle,
    SC_CHAR *pValueName,
    SC_UINT32 *pValueData )
{
    return E_SC_OK;
}

SC_VOID SC_STDCALL
    scClientOutputDebugString(
    SC_CLIENT_HANDLE ClientHandle,
    SC_CHAR *pPrefix,
    SC_CHAR *pDebugMessage,
    va_list ap )
{

#if defined(_WIN32) && !defined(ATI_BITS_64)  && defined(_DEBUG)

    static const unsigned int MAX_STR_LEN = 4096;
    char acStr[MAX_STR_LEN];

    va_start( ap, pDebugMessage );
    int len = vsnprintf( acStr, MAX_STR_LEN, pDebugMessage, ap);
    va_end( ap );

    if(len > 0)
    {
        acStr[len] = '\0';
    }

#endif

}

SC_FILE_HANDLE SC_STDCALL
    scClientOpenFile(
    SC_CLIENT_HANDLE hClientHandle,
    SC_CHAR *pFileName,
    SC_UINT32 u32mode)
{
    FILE *f = NULL;

    switch (u32mode)
    {
    case SC_FILE_ACCESS_READ:
        f = fopen((const char *) pFileName, "r");
        break;
    case SC_FILE_ACCESS_WRITE:
        f = fopen((const char *) pFileName, "w");
        break;
    case SC_FILE_ACCESS_READ | SC_FILE_ACCESS_WRITE:
        f = fopen((const char *) pFileName, "r+");
        break;
    default:
        assert(!"Unknown file mode in scOpenFile");
        break;
    }

    return (SC_FILE_HANDLE) f;
}

SC_BOOL SC_STDCALL
    scClientCloseFile(
    SC_CLIENT_HANDLE hClientHandle,
    SC_FILE_HANDLE hFileObject)
{
    int result;

    if (!hFileObject)
    {
        assert(!"scClientCloseFile(): Null file handle!");
        return 0;
    }

    result = fclose((FILE *) hFileObject);
    return (result == 0);
}

SC_INT32 SC_STDCALL
    scClientWriteFile(
    SC_CLIENT_HANDLE hClientHandle,
    SC_FILE_HANDLE hFileHandle,
    SC_CHAR *pBuffer,
    SC_UINT32 u32NumBytesToWrite,
    SC_UINT32 u32WriteFlags)
{
    size_t count, result;

    result = 0;
    count = u32NumBytesToWrite;

    if (!pBuffer || !hFileHandle)
    {
        assert(!"scClientWriteFile(): Null buffer or file handle!");
        return 0;
    }

    result = fwrite((const void *) pBuffer, 1, count, (FILE *) hFileHandle);

    return (SC_INT32) result;
}

SC_INT32 SC_STDCALL
    scClientReadFile(
    SC_CLIENT_HANDLE hClientHandle,
    SC_FILE_HANDLE hFileHandle,
    SC_CHAR *pBuffer,
    SC_UINT32 u32NumBytesToRead)
{
    size_t count, result;

    result = 0;
    count = u32NumBytesToRead;

    if (!pBuffer || !hFileHandle)
    {
        assert(!"scClientReadFile(): Null buffer or file handle!");
        return 0;
    }

    result = fread((void *) pBuffer, 1, count, (FILE *) hFileHandle);

    return (SC_INT32) result;
}



class devStateBase;
class scStateBase;
class devStateSI;
class scOptions {
public:
    scOptions() {};
    virtual ~scOptions() {};
    void setOption(scStateBase*, SCOption, bool);
}; // class scOptions

enum shaderTypes {
    FRAGMENT_SHADER_TYPE = 0,
    VERTEX_SHADER_TYPE = 1,
    GEOMETRY_SHADER_TYPE = 2,
    FETCH_SHADER_TYPE = 3,
    COMPUTE_SHADER_TYPE = 4,
    SHADER_TYPE_LAST = COMPUTE_SHADER_TYPE
};

// The scStateBase class holds all of the information that is required to
// store the state of SC so that it can called correctly. It also supplies
// the interface that the derived classes must implement.
class scStateBase {
private:
    scStateBase(); // DNI!
    scStateBase(scStateBase *); // DNI!

    // function that sets up the client interface for SC.
    void setupClientInterface();

    // function that sets up the sc capabilities
    void setupSCCaps();

protected:
    // Constructor that can only be called from inherited classes.
    scStateBase(devStateBase *dev, scOptions *scOpt, unsigned progType);

    // Pointer to the sc compiler that will be called by 
    devStateBase *mDev;

    // The class that holds all of the scOptions
    scOptions *mOpts;

    // A handle to the SC source shader
    SC_SRCSHADER mScSrcShader;

    // A handle to the SC source BRIG
    SC_SRCBRIG mScSrcBRIG;

    // A pointer to the Hardware shader from SC
    SC_HWSHADER *mScHwShader;

    // A handle to the SC compiler
    SC_SCHANDLE mSC;

    // SC Client interface object
    SC_SC2CLIENT_INTERFACE mScClient;

    // SC Client capabilities
    SC_CLIENT2SC_CAPS mScCaps;

    // The program type.
    unsigned mProgramType;

public:
    virtual ~scStateBase();

    SC_SRCSHADER*  SrcShader() { return &mScSrcShader; }
    SC_SRCBRIG*   SrcBRIG() { return &mScSrcBRIG; }
    SC_HWSHADER*  HWShader() { return mScHwShader; }
    SC_SCHANDLE   SC() { return mSC; }
    SC_SC2CLIENT_INTERFACE Interface() { return mScClient; }
    SC_CLIENT2SC_CAPS Caps() { return mScCaps; }
}; // class scState

class scStateSI : public scStateBase {
public:
    scStateSI(devStateSI *dev, scOptions *sOpt, unsigned progType);
    virtual ~scStateSI();
private:

}; // class scStateSI

#define SI_SC_IMMEDIATE_SUPPORT     (1 << 0)
#define SI_SC_FLAT_TBL_SUPPORT      (1 << 1)
#define SI_MAX_NUMBER_USER_DATA_ELEMENTS 1024
#define SI_VGPR_COUNT 256
#define SI_SGPR_COUNT 104 - 2
#define SI_NUM_USER_DATA 16
#define SI_NUM_RESOURCES 128
#define SI_NUM_INTERNAL_RESOURCES 0
#define SI_NUM_SAMPLERS 16
// Support 16 constant buffers plus 1 for internal SC usage and 1 for subroutine data
// and one for compute info
#define SI_NUM_CONSTANT_BUFS 16 + 1 + 1 + 1
#define SI_NUM_VERTEX_BUFS 1
#define SI_NUM_STREAMOUT_BUFS 0
#define SI_NUM_UAVS 1024

class devStateBase {
private:
    devStateBase(devStateBase *); // DNI!
protected:
    // These two fields are protected as we don't want the 
    // user to be able to directly modify them. This could
    // cause issues if they are swapped while the option class
    // is live.
    unsigned mChipFamily;
    unsigned mChipRevision;
    devStateBase(unsigned chipFamily, unsigned chipRevision);
public:
    virtual ~devStateBase() {}
    int numShaderEngines;
    int LDSSize;
    unsigned int wavefrontSize;

    // Accessor to the chip family field.
    unsigned getFamily() { return mChipFamily; }

    // Accessor to the chip revision field.
    unsigned getRevision() { return mChipRevision; }

}; // class devState;
class devStateSI : public devStateBase {
private:
    devStateSI(devStateSI*); // DNI!
public:
    devStateSI(unsigned chipFamily, unsigned uiChipRevision);
    virtual ~devStateSI() {}
    int numVGPRsAvailable;
    int numSGPRsAvailable;
    int numUserDataAvailable;
    bool siClampScratchAccess;
    unsigned int siScModeSupportCaps;
    unsigned int numResources;
    unsigned int numInternalResources;
    unsigned int numSamplers;
    unsigned int numConstBuf;
    unsigned int numVertexBuf;
    unsigned int numStreamOutBuf;
    unsigned int numUav;
    unsigned int siScInternalCB;
    unsigned int siScSubroutineInfoCB;
    unsigned int siScComputeInfoCB;
};

devStateSI::devStateSI(unsigned chipFamily, unsigned chipRevision)
    : devStateBase(chipFamily, chipRevision),
    numVGPRsAvailable(SI_VGPR_COUNT),
    numSGPRsAvailable(SI_SGPR_COUNT),
    numUserDataAvailable(SI_NUM_USER_DATA),
    siClampScratchAccess(true),
    siScModeSupportCaps(SI_SC_IMMEDIATE_SUPPORT | SI_SC_FLAT_TBL_SUPPORT),
    numResources(SI_NUM_RESOURCES),
    numInternalResources(SI_NUM_INTERNAL_RESOURCES),
    numSamplers(SI_NUM_SAMPLERS),
    numConstBuf(SI_NUM_CONSTANT_BUFS),
    numVertexBuf(SI_NUM_VERTEX_BUFS),
    numStreamOutBuf(SI_NUM_STREAMOUT_BUFS),
    numUav(SI_NUM_UAVS)
{
    siScInternalCB = numConstBuf - 3;
    siScSubroutineInfoCB = siScInternalCB + 1;
    siScComputeInfoCB = siScSubroutineInfoCB + 1;
}



class scCompileSI {
private:
    scCompileSI(scCompileSI*); // DNI!
    // Return the scOptions for this compiler.
    scOptions* Options() { return mOptions; }
    void setOptions(scOptions* ptr) { mOptions = ptr; }

    // Return the scState for this compiler.
    scStateBase* State() { return mState; }
    void setState(scStateBase* ptr) { mState = ptr; }

    // Return the device state for this compiler.
    devStateBase* Device() { return mDevice; }
    void setDevice(devStateBase* ptr) { mDevice = ptr; }

private:
    scOptions *mOptions;
    scStateBase *mState;
    devStateBase *mDevice;
public:
    scCompileSI();
    virtual ~scCompileSI();
    //bool Compile(ILBinary &input, size_t &size);
    bool Finalize(char* pdirectives, 
        unsigned int lenDirectiveSectioninBytes,
        char* pOperands, 
        unsigned int lenOperandSectioninBytes,
        char* pStrings, 
        unsigned int lenStringSectioninBytes,
        char* pInstructions, 
        unsigned int lenInstructionSectioninBytes);
    bool Disassemble(std::string& output);
    char *getBinary(size_t &size);
protected:
}; // scCompileSI

//----------------------------------------------------------------------------//
// SC State class for SI architectures
//----------------------------------------------------------------------------//
scStateSI::scStateSI(devStateSI *dev, scOptions *scOpt, unsigned progType)
    : scStateBase(reinterpret_cast<devStateBase*>(dev), scOpt, progType)
{
    switch (progType) {
    case COMPUTE_SHADER_TYPE:
        mScHwShader = reinterpret_cast<SC_HWSHADER*>(new SC_SI_HWSHADER_CS);
        memset(mScHwShader, 0, sizeof(SC_SI_HWSHADER_CS));
        mScHwShader->uSizeInBytes = sizeof(SC_SI_HWSHADER_CS);
        break;
    case FRAGMENT_SHADER_TYPE:
        mScHwShader = reinterpret_cast<SC_HWSHADER*>(new SC_SI_HWSHADER_PS);
        memset(mScHwShader, 0, sizeof(SC_SI_HWSHADER_PS));
        mScHwShader->uSizeInBytes = sizeof(SC_SI_HWSHADER_PS);
        break;
    case VERTEX_SHADER_TYPE:
        mScHwShader = reinterpret_cast<SC_HWSHADER*>(new SC_SI_HWSHADER_VS);
        memset(mScHwShader, 0, sizeof(SC_SI_HWSHADER_VS));
        mScHwShader->uSizeInBytes = sizeof(SC_SI_HWSHADER_VS);
        break;
    };
    scOpt->setOption(this, SCOption_R800_UAV_NONUAV_SYNC_WORKAROUND_BUG216513_1, false);
    scOpt->setOption(this, SCOption_R800_GLOBAL_RETURN_BUFFER, false);
    scOpt->setOption(this, SCOption_R800_DYNAMIC_UAV_RESOURCE_MAPPING, false);
    scOpt->setOption(this, SCOption_R800_UAV_EXTENDED_CACHING, false);
    scOpt->setOption(this, SCOption_R800_UAV_NONARRAY_FIXUP, true);
    scOpt->setOption(this, SCOption_R1000_BYTE_SHORT_WRITE_WORKAROUND_BUG317611, true);
    scOpt->setOption(this, SCOption_R1000_READLANE_SMRD_WORKAROUND_BUG343479, true);
    SC_SI_SHADERSTATE *pScShaderState = new SC_SI_SHADERSTATE;
    memset(pScShaderState, 0, sizeof(SC_SI_SHADERSTATE));

    // TODO: What do these mean?
    pScShaderState->numVGPRsAvailable               = dev->numVGPRsAvailable;
    pScShaderState->numSGPRsAvailable               = dev->numSGPRsAvailable;
    pScShaderState->numUserDataAvailable            = dev->numUserDataAvailable;
    pScShaderState->wavefrontSize                   = dev->wavefrontSize;
    pScShaderState->numShaderEngines                = dev->numShaderEngines;

    //pScShaderState->eVsOutSemanticMode = R600VSOUTPUT_VECTOR_SEMANTICS;

    if(SrcShader())
        SrcShader()->ss.pHwState = reinterpret_cast<SC_BYTE *>(pScShaderState);

    SC_SI_SHADERSTATE *pShaderState = reinterpret_cast<SC_SI_SHADERSTATE *>(SrcShader()->ss.pHwState);
    pShaderState->pUserElements = new SC_SI_USER_DATA_ELEMENT[SI_MAX_NUMBER_USER_DATA_ELEMENTS];
    pShaderState->maxUserElementCount = SI_MAX_NUMBER_USER_DATA_ELEMENTS;
    SrcShader()->ss.pBoolPSConst = NULL;
    SrcShader()->ss.u32NumBoolPSConst = 0;
    SrcShader()->ss.pBoolCSConst = NULL;
    SrcShader()->ss.u32NumBoolCSConst = 0;
    SrcShader()->ss.pBoolVSConst = NULL;
    SrcShader()->ss.u32NumBoolVSConst = 0;

    // Compiler-internal boolean constants.  None are available.
    memset(&SrcShader()->bConstantsAvailable, 0, sizeof(SC_CONSTANTUSAGE));
    // Compiler-internal float constants.  None are available.
    memset(&SrcShader()->fConstantsAvailable, 0, sizeof(SC_CONSTANTUSAGE));
    // Compiler-internal loop (integer) constants.  None are available.
    memset(&SrcShader()->iConstantsAvailable, 0, sizeof(SC_CONSTANTUSAGE));

    pShaderState->wavefrontSize         = dev->wavefrontSize;
    pShaderState->numShaderEngines      = dev->numShaderEngines;

    // HW resources available to shader
    pShaderState->numVGPRsAvailable     = dev->numVGPRsAvailable;
    pShaderState->numSGPRsAvailable     = dev->numSGPRsAvailable;
    pShaderState->numUserDataAvailable  = dev->numUserDataAvailable;

    //TODO: Fill these in with proper values later
    // Max number of resource descriptors in flat tables
    pShaderState->flatTblNumResources           = dev->numResources;
    pShaderState->flatTblNumInternalResources   = dev->numInternalResources;
    pShaderState->flatTblNumSamplers            = dev->numSamplers;
    pShaderState->flatTblNumConstBuf            = dev->numConstBuf;
    pShaderState->flatTblNumVertexBuf           = dev->numVertexBuf;
    pShaderState->flatTblNumStreamOutBuf        = dev->numStreamOutBuf;
    pShaderState->flatTblNumUav                 = dev->numUav;

    // Driver shader launch caps
    pShaderState->launchModeFlags.immediateSupport =
        ((dev->siScModeSupportCaps & SI_SC_IMMEDIATE_SUPPORT) ? true : false);
    pShaderState->launchModeFlags.srdTableSupport =
        ((dev->siScModeSupportCaps & SI_SC_FLAT_TBL_SUPPORT) ? true : false);

    // Set compiler flags
    pShaderState->compileFlags.clampScratchAccess = dev->siClampScratchAccess;

    switch (progType)
    {
    case FRAGMENT_SHADER_TYPE:
        pShaderState->hwShaderStage = STAGE_HW_PS; 
        pShaderState->firstScUserReg = 0;	
        break;
    case COMPUTE_SHADER_TYPE:
        pShaderState->hwShaderStage = STAGE_HW_CS; 
        pShaderState->firstScUserReg = 0;
        break;
    case VERTEX_SHADER_TYPE:
        pShaderState->hwShaderStage = STAGE_HW_VS;
        pShaderState->firstScUserReg = 2;	// In UGL, the first two user registers are reserved.
        break;
    }
    // Internal const buffer slots
    pShaderState->dx9FloatConstBuffId    = 0;    // not really used
    pShaderState->dx9IntConstBuffId      = 0;    // not really used
    pShaderState->immedConstBuffId       = dev->siScInternalCB;
    pShaderState->sampleInfoConstBuffId  = 0;    // not really used
    pShaderState->samplePosConstBuffId   = 0;    // not really used
    pShaderState->subroutineConstBuffId  = dev->siScSubroutineInfoCB;
    pShaderState->tessConstBuffId        = 0;    // not really used
    pShaderState->computeConstBuffId     = dev->siScComputeInfoCB;

    ///@todo Set correct offsets for the subroutine data
    pShaderState->subroutineFuncTableOffset = 0;
    pShaderState->subroutineThisPtrOffset   = 0;

    SC_SI_HWSHADER_COMMON *pScSIHwShader = reinterpret_cast<SC_SI_HWSHADER_COMMON*>(HWShader());
    pScSIHwShader->hShaderMemHandle = &pScSIHwShader->hShaderMemHandle;
    pScSIHwShader->codeLenInByte = 0;
    if (mProgramType == VERTEX_SHADER_TYPE) {
        SC_SI_HWSHADER_VS* pVsShader = reinterpret_cast<SC_SI_HWSHADER_VS*>(HWShader());
        for (int i = 0; i < SC_SI_VS_MAX_INPUTS; i++)
        {
            pVsShader->vsInSemantics[i].usageIdx = i;
            pVsShader->vsInSemantics[i].usage = 0;
            pVsShader->vsInSemantics[i].dataVgpr = 255;
            pVsShader->vsInSemantics[i].dataSize = 3;
        }
    }

}

devStateBase::devStateBase(unsigned chipFamily, unsigned chipRevision)
    : mChipFamily(chipFamily), 
    mChipRevision(chipRevision)
{
    wavefrontSize = SCGetWavefrontSize(chipFamily, chipRevision);
    numShaderEngines = SCGetNumShaderEngine(chipFamily, chipRevision);
    switch (chipFamily) {
    default:
        assert(!"Unsupported chip family specified!");
        break;
    case FAMILY_RV7XX:
    case FAMILY_R700:
    case FAMILY_MV7X:
        LDSSize = 16 * 1024;
        break;
    case FAMILY_SUMO:
    case FAMILY_EVERGREEN:
    case FAMILY_MANHATTAN:
    case FAMILY_NORTHERNISLAND:
    case FAMILY_SI:
        LDSSize = 32 * 1024;
        break;

    };
}
scStateSI::~scStateSI()
{
    if (!mScHwShader) {
        return;
    }
    SC_SI_HWSHADER_COMMON* pScSIHwShader = (SC_SI_HWSHADER_COMMON*) mScHwShader;

    //
    // Allocated memory during compile via callback
    //
    if (pScSIHwShader->hShaderMemHandle != &pScSIHwShader->hShaderMemHandle) {
        // This needs to be free instead of delete as SC allocates this memory
        // and not the scwrapper.
        free(pScSIHwShader->hShaderMemHandle);
        pScSIHwShader->hShaderMemHandle = NULL;
    }
    if (pScSIHwShader->pPvtData) {
        delete [] pScSIHwShader->pPvtData;
    }
    if (pScSIHwShader->pUserElements) {
        delete [] pScSIHwShader->pUserElements;
    }
    if (pScSIHwShader->pExtUserElements) {
        delete [] pScSIHwShader->pExtUserElements;
    }
    delete pScSIHwShader;
}

void SHPrint(SC_CLIENT_HANDLE hClientHandle, SC_CLIENT_DEBUGSTRING* pPrint,
    const char* pPrefix, const char* pStr, ...)
{
    va_list var_args;
    va_start(var_args, pStr);

    enum { MAX_STRING_SIZE = 32768 };
    char lpszBuffer[MAX_STRING_SIZE];

    _vsnprintf( lpszBuffer, MAX_STRING_SIZE, pStr, var_args );
    if ( pPrint ) {
        (*pPrint)(hClientHandle, (SC_CHAR*)pPrefix, (SC_CHAR*)lpszBuffer, var_args);
    }
}

scStateBase::scStateBase(devStateBase *dev, scOptions *scOpt, unsigned progType)
    : mDev(dev), mOpts(scOpt), mProgramType(progType)
{
    mSC = 0;
    mScHwShader = 0;
    ::memset(&mScSrcShader, 0, sizeof(mScSrcShader));
    ::memset(&mScSrcBRIG, 0, sizeof(mScSrcBRIG));
    ::memset(&mScClient, 0, sizeof(mScClient));
    ::memset(&mScCaps, 0, sizeof(mScCaps));
    setupClientInterface();
    setupSCCaps();

    mSC = SCCreate(&mScClient, &mScCaps, 0);
    if(!mSC) {
        assert(!"scInitCompiler(): SC Create Failed!");
    }
    if (mScCaps.u32PvtBufferSizeInBytes) {
        mScSrcShader.pPvtData = (SC_BYTE*)scClientAllocSysMem(0, 
            mScCaps.u32PvtBufferSizeInBytes);
        if (!mScSrcShader.pPvtData) {
            assert(!"scInitCompiler(): Malloc failed!");
        }
    }
    mScSrcShader.ss.pBoolPSConst = NULL;
    mScSrcShader.ss.u32NumBoolPSConst = 0;

#ifdef _TEST_COMPILER
    CompilerExternal* pComp = (CompilerExternal*)mSC;
    pComp->internal->SetExtBaseName(IL_SHADER_COMPUTE, "_compute_");
    REPORTED_STATS  = new SingleShaderStats;
#endif
}

scStateBase::~scStateBase()
{
    if (mSC) {
        SCDestroy(mSC);
    }
    if (mScSrcShader.pPvtData) {
        delete [] mScSrcShader.pPvtData;
    }
    if (mScSrcShader.ss.pHwState) {
        delete mScSrcShader.ss.pHwState;
    }
}

// Sets up the SC to Client interface with the 
// required function pointers so that SC can
// correctly allocate memory.
void
    scStateBase::setupClientInterface()
{
    mScClient.u32ChipFamily = FAMILY_SI;
    mScClient.u32ChipRevision = SI_TAHITI_P_A11;
    mScClient.eClientType = E_SC_CLIENTTYPE_D3D;
    mScClient.hClientHandle = (SC_CLIENT_HANDLE) 1;
    mScClient.AllocateSysMem = scClientAllocSysMem;
    mScClient.FreeSysMem = scClientFreeSysMem;
    mScClient.AllocateShaderMem = scClientLocalAllocShaderMem;
    mScClient.QueryRegistryValue = scClientQueryRegistry;
    mScClient.OutputDebugString = scClientOutputDebugString;
    mScClient.OpenFile = scClientOpenFile;
    mScClient.CloseFile = scClientCloseFile;
    mScClient.WriteFile = scClientWriteFile;
    mScClient.ReadFile = scClientReadFile;
}

// Sets up the Client to SC capabilities so that
// the client can correct query SC.
void
    scStateBase::setupSCCaps()
{
    mScCaps.bPerComponentVsOut = false; // dx9 style

    mScClient.scInputParams.instructionSet 
        = SCGetInstructionSet(mDev->getFamily(), mDev->getRevision());
}

void 
    scOptions::setOption(scStateBase* scState, SCOption opt, bool flag)
{
    SCSetSCOption(scState->SrcShader()->u32SCOptions, opt, flag);
}

scCompileSI::scCompileSI()
{
    mState = NULL;
    mOptions = NULL;
    mDevice = NULL;
    setDevice(new devStateSI(FAMILY_SI, SI_TAHITI_P_A11));
    setOptions(new scOptions);
    // FIXME: Why do we need this cast here, but not in 
    // scCompileEGNI.cpp or in scCompile7XX.cpp?
    scStateSI *tmp = new scStateSI((devStateSI*)Device(), 
        Options(), COMPUTE_SHADER_TYPE);
    setState(tmp);
}

scCompileSI::~scCompileSI()
{
    if (mState) {
        delete mState;
    }
    if (mOptions) {
        delete mOptions;
    }
    if (mDevice) {
        delete mDevice;
    }
}

// TODO: This needs to not be global!
static std::string *outputPtr = NULL;
static SC_VOID SC_STDCALL scDisasmOutput(
    SC_CLIENT_HANDLE    ClientHandle,
    SC_CHAR             *pPrefix,
    SC_CHAR             *pText,
    va_list             ap)
{
    char lpszBuffer[20000];

    vsprintf(lpszBuffer, pText, ap);
    if (outputPtr) {
        (*outputPtr) += lpszBuffer;
    } else {
        printf("%s\n", lpszBuffer);
    }
}

// Temporarily hardcoding values in as set by the SC team
#define CONST_REG_BUFFER                0
#define SI_IMMED_CONST_BUFFER           15
#define SI_SAMPLE_INFO_CONST_BUFFER     16
#define SI_SAMPLE_POS_CONST_BUFFER      17
#define SI_SUBROUTINE_CONST_BUFFER      18
#define SI_TESS_CONST_BUFFER            19
#define SI_COMPUTE_CONST_BUFFER         19

bool
    scCompileSI::Finalize(
    char* pDirectives,
    unsigned int lenDirectiveSectioninBytes,
    char* pOperands,
    unsigned int lenOperandSectioninBytes,
    char* pStrings,
    unsigned int lenStringSectioninBytes,
    char* pInstructions,
    unsigned int lenInstructionSectioninBytes )
{
    E_SC_RETURNCODE scErr = E_SC_OK;
    //SC_EXPORT_FUNCTIONS *scef = reinterpret_cast<SC_EXPORT_FUNCTIONS*>(mComp->scAPI.scef);
    // first create a compiler object
    SC_SC2CLIENT_INTERFACE client;
    memset( &client,0x0, sizeof( SC_SC2CLIENT_INTERFACE ));

    int wavefront_size = 64;

    client.u32ChipFamily = 0xffffffff;
    client.scInputParams.instructionSet = ESC_ISET_TAHITI;
    wavefront_size = 64;

    client.u32ChipRevision = 0;
    client.hClientHandle = (SC_CLIENT_HANDLE)1;
    client.AllocateSysMem = scClientAllocSysMem;
    client.FreeSysMem = scClientFreeSysMem;
    client.QueryRegistryValue = (SC_CLIENT_QUERYREGISTRY)1;
    client.OpenFile = (SC_CLIENT_OPENFILE)1;
    client.CloseFile = (SC_CLIENT_CLOSEFILE)1;
    client.WriteFile = (SC_CLIENT_WRITEFILE)1;
    client.ReadFile = (SC_CLIENT_READFILE)1;
    client.AllocateShaderMem = scClientLocalAllocShaderMem;

    SC_CLIENT2SC_CAPS caps;
    caps.bPerComponentVsOut = false; /* dx9 style*/

    SC_SI_SHADERSTATE state;
    memset(&state, 0, sizeof(SC_SI_SHADERSTATE));

    state.wavefrontSize = wavefront_size;

    state.numShaderEngines = 2;

    state.numVGPRsAvailable = 0x00000100; // TODO: check hardcoded value.
    state.numSGPRsAvailable = 0x00000066; // TODO: check hardcoded value.
    state.numUserDataAvailable = 16;
    // state.numCopyGPRsAvailable = MAGIC.temp_limit_copy_shader;
    state.firstScUserReg      = 0;

    state.flatTblNumResources = 128;
    state.flatTblNumInternalResources = 128;
    state.dx9FloatConstBuffId    = CONST_REG_BUFFER;
    state.dx9IntConstBuffId      = CONST_REG_BUFFER + 1;
    state.immedConstBuffId       = SI_IMMED_CONST_BUFFER;
    state.tessConstBuffId        = SI_TESS_CONST_BUFFER;
    state.sampleInfoConstBuffId  = SI_SAMPLE_INFO_CONST_BUFFER;
    state.samplePosConstBuffId   = SI_SAMPLE_POS_CONST_BUFFER;
    state.computeConstBuffId     = SI_COMPUTE_CONST_BUFFER;
    // SI shares same CB for jump addr/this 
    state.subroutineConstBuffId  = SI_SUBROUTINE_CONST_BUFFER;
    //  state.subroutineFuncTableOffset = MAGIC.si_subroutine_cb_functable_offset;
    state.subroutineFuncTableOffset = 128; // TODO: check hardcoded value.
    //    state.subroutineThisPtrOffset = MAGIC.si_subroutine_cb_thisptr_offset;
    state.subroutineThisPtrOffset = 0; // TODO: check hardcoded value.

    // Max number of user data descriptors
    state.maxUserElementCount    = 16;

    // Driver provided storage for user data elements to be filled by SC
    state.pUserElements          = (SC_SI_USER_DATA_ELEMENT*)
        client.AllocateSysMem( client.hClientHandle,
        sizeof(SC_SI_USER_DATA_ELEMENT) * state.maxUserElementCount );

    state.maxUserElementCopyCount= 4;
    state.pUserElementsCopy      = (SC_SI_USER_DATA_ELEMENT*)
        client.AllocateSysMem( client.hClientHandle,
        sizeof(SC_SI_USER_DATA_ELEMENT) * state.maxUserElementCopyCount );
    // Max number of extended user data descriptors
    state.maxExtUserElementCount = 64;
    // Max size of extended user data area (in video memory) in bytes
    state.maxExtUserMemSize      = 0;
    // Driver provided storage for extended user data elements to be filled by SC
    state.pExtUserElements       = (SC_SI_USER_DATA_ELEMENT*)
        client.AllocateSysMem( client.hClientHandle,
        sizeof(SC_SI_USER_DATA_ELEMENT) * state.maxExtUserElementCount );

    // Max number of SRDs that can be located in user data and extended user data
    state.maxSRDsInUserData      = 16;

    State()->SrcBRIG()->ss.pHwState = (SC_BYTE*) &state;

    // Assign the binary to the source shader and compile.
    State()->SrcBRIG()->pDirectives = (SC_BYTE *)pDirectives;
    State()->SrcBRIG()->u32DirectiveSectioninBytes = lenDirectiveSectioninBytes;
    State()->SrcBRIG()->pOperands = (SC_BYTE *)pOperands;
    State()->SrcBRIG()->u32OperandSectioninBytes = lenOperandSectioninBytes;
    State()->SrcBRIG()->pStrings = (SC_BYTE *)pStrings;
    State()->SrcBRIG()->u32StringSectioninBytes = lenStringSectioninBytes;
    State()->SrcBRIG()->pInstructions = (SC_BYTE *)pInstructions;
    State()->SrcBRIG()->u32InstructionSectioninBytes = lenInstructionSectioninBytes;
#ifdef ATI_OS_WINDOWS
    __try {
#endif
        scErr = SCCompileBRIG(State()->SC(), 
            State()->SrcBRIG(), 
            State()->HWShader());
#ifdef ATI_OS_WINDOWS
    } __except( (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? 1 : 0)
    {
        return false;
    }
#endif

    return scErr == E_SC_OK;
}

bool
    scCompileSI::Disassemble(std::string& output)
{
    // TODO: Need to add support for other shader types.
    output += "ShaderType = IL_SHADER_COMPUTE\n";
    output += "TargetChip = t \n";
    outputPtr = &output;
    SC_CLIENT_DEBUGSTRING debugOutput = scDisasmOutput;
#ifdef ATI_OS_WINDOWS
    __try {
#endif
        SCDumpSrcShaderData( NULL,
            State()->SrcShader(), 
            debugOutput);
        SCDumpHwShader( NULL, 
            State()->HWShader(),
            reinterpret_cast<SC_SI_HWSHADER_COMMON*>(
            State()->HWShader())->hShaderMemHandle, 
            NULL, debugOutput);
        SCDumpHwShaderData( NULL,
            State()->HWShader(),
            debugOutput);

#ifdef ATI_OS_WINDOWS
    } __except( (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? 1 : 0)
    {
        outputPtr = NULL;
        return false;
    }
#endif
    outputPtr = NULL;
    return true;
}

char*
    scCompileSI::getBinary(size_t &size)
{
    SC_SI_HWSHADER_CS *ptr = reinterpret_cast<SC_SI_HWSHADER_CS*>(State()->HWShader());
    size = ptr->common.uSizeInBytes + ptr->common.u32PvtDataSizeInBytes + ptr->common.codeLenInByte
        + (ptr->common.extUserElementCount * sizeof(SC_SI_USER_DATA_ELEMENT))
        + (ptr->common.userElementCount * sizeof(SC_SI_USER_DATA_ELEMENT));
    size_t offset = 0;
    char* binary = new char[size + 1];
    memset(binary, 0, size + 1);

    // Serialize SC_SI_HWSHADER_CS
    memcpy(binary, ptr, ptr->common.uSizeInBytes);
    offset += ptr->common.uSizeInBytes;

    // Serialize private data
    memcpy(binary + offset, ptr->common.pPvtData, ptr->common.u32PvtDataSizeInBytes);
    reinterpret_cast<SC_SI_HWSHADER_CS*>(binary)->common.pPvtData = reinterpret_cast<SC_BYTE*>(binary + offset);
    offset += ptr->common.u32PvtDataSizeInBytes;

    // Serialize shader mem handle
    memcpy(binary + offset, ptr->common.hShaderMemHandle, ptr->common.codeLenInByte);
    reinterpret_cast<SC_SI_HWSHADER_CS*>(binary)->common.hShaderMemHandle = (binary + offset);
    offset += ptr->common.codeLenInByte;

    // Serialize user elements
    memcpy(binary + offset, ptr->common.pUserElements, ptr->common.userElementCount * sizeof(SC_SI_USER_DATA_ELEMENT));
    reinterpret_cast<SC_SI_HWSHADER_CS*>(binary)->common.pUserElements = reinterpret_cast<SC_SI_USER_DATA_ELEMENT*>(binary + offset);
    offset += (ptr->common.userElementCount * sizeof(SC_SI_USER_DATA_ELEMENT));

    // Serialize extended user elements
    memcpy(binary + offset, ptr->common.pExtUserElements, ptr->common.extUserElementCount * sizeof(SC_SI_USER_DATA_ELEMENT));
    reinterpret_cast<SC_SI_HWSHADER_CS*>(binary)->common.pExtUserElements = reinterpret_cast<SC_SI_USER_DATA_ELEMENT*>(binary + offset);
    offset += (ptr->common.extUserElementCount * sizeof(SC_SI_USER_DATA_ELEMENT));
    assert(offset == size && "Final binary offset does not equal to binary size!");
    return binary;
}


int main(int argc, char** argv) {

    if (argc<4) {
        printf("Usage: %s <infile.hsail> <hwdump.txt> <hwdump.bin>\n", argv[0]);
        return 1;
    }

    scCompileSI *scCompiler = new scCompileSI();

    using namespace HSAIL_ASM;
    BrigContainer c;
    if (BrigStreamer::load(c, argv[1])) return 1;

    char *code_p       = c.insts().getData(0);
    char *operands_p   = c.operands().getData(0);
    char *directives_p = c.directives().getData(0);
    char *strtab_p     = c.strings().getData(0);
    unsigned int codeSize       = c.insts().size();
    unsigned int operandsSize   = c.operands().size();
    unsigned int directivesSize = c.directives().size();
    unsigned int strtabSize     = c.strings().size();

    assert( operands_p && operandsSize != 0 &&
        directives_p && directivesSize != 0 &&
        strtab_p && strtabSize != 0 &&
        code_p && codeSize != 0 &&
        "Unexpected BRIG in ifaclhsaFinalize!!" );

    if (!scCompiler->Finalize(directives_p, directivesSize,
        operands_p,   operandsSize,
        strtab_p,     strtabSize,
        code_p,       codeSize)) {
            std::string codeName = "codeName";
            std::string error = "Error compiling program for " + codeName + ".";
            std::cout << error << std::endl;
            return 1;
    }

    std::string isaDump;
    if (scCompiler->Disassemble(isaDump)) {
        std::string isaFileName = argv[2];
        std::fstream f;
        f.open(isaFileName.c_str(), (std::fstream::out | std::fstream::binary)); 
        f.write(reinterpret_cast<const char*>(isaDump.c_str()), isaDump.length());
        f.close();
    } else {
        std::cout << "Failed to dump ISA" << std::endl;
    }

    size_t binarySize;
    char* binary = scCompiler->getBinary(binarySize);
    std::string isaFileName = argv[3];
    std::fstream f;
    f.open(isaFileName.c_str(), (std::fstream::out | std::fstream::binary));
    f.write(binary, binarySize);
    f.close();

    delete binary;

    delete scCompiler;

    return 0;
}
