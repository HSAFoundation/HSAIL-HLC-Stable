//===-- HSAILTestGen.cpp - HSAIL Test Generator ---------------------------===//
//
//===----------------------------------------------------------------------===//
//
// HSAIL Test Generator. (C) 2013 AMD Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#include "HSAILTestGenOptions.h"
#include "HSAILTestGenManager.h"
#include "HSAILTestGenNavigator.h"

#include "HSAILValidatorBase.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/PathV2.h"


#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

using std::ostringstream;
using std::ofstream;

using std::string;
using HSAIL_ASM::PropValidator;
using HSAIL_ASM::BrigContainer;
using HSAIL_ASM::BrigStreamer;
using HSAIL_ASM::opcode2str;

//==============================================================================
//==============================================================================
//==============================================================================

#define BRIG_TESTGEN_VERSION "2.2"

//==============================================================================
//==============================================================================
//==============================================================================

namespace TESTGEN {

//==============================================================================
//==============================================================================
//==============================================================================

#define HSAIL_TEST_NAME "hsail_tests"
#define POSITIVE_SUFF   "_p"
#define NEGATIVE_SUFF   "_n"
#define BRIG_FILE_EXT   ".brig"
#define LUA_FILE_EXT    ".lua"
#define HSAIL_TESTLIST  "testlist.txt"


//==============================================================================
//==============================================================================
//==============================================================================

class TestGenManagerImpl : public TestGenManager
{
private:
    static const unsigned OPCODE_NONE = 0xFFFFFFFF;

    //==========================================================================
private:
    TestGenNavigator navigator;         // a component used to categorize and filter out tests
    const string     testPath;          // a path to an existing directory where tests are to be saved
    unsigned         currentOpcode;     // opcode of test instruction being processed
    unsigned         opTestIdx;         // index of the last test generated for the current opcode (there is a global index as well)
    unsigned         failedTestIdx;     // total number of failed tests (used in selftest mode only)
    ofstream         testTagsFs;        // stream used to log properties of each test

    //==========================================================================
public:

    TestGenManagerImpl(string path, bool testType) : TestGenManager(testType), testPath(path), currentOpcode(OPCODE_NONE), opTestIdx(0), failedTestIdx(0) {}

    bool generate()
    {
        TestGenManager::generate();
        closeTestTagsStream();
        printSummary();

        return failedTestIdx == 0;
    }

    //==========================================================================
protected:

    bool isOpcodeEnabled(unsigned opcode)
    { 
        return navigator.isOpcodeEnabled(opcode); 
    }

    bool startTest(Inst inst)           
    { 
        if (navigator.startTest(inst))
        {
            if (testPackage == PACKAGE_SEPARATE)
            {
                createPath(getTestPath());
                if (currentOpcode != inst.opcode())
                {
                    currentOpcode = inst.opcode();
                    opTestIdx = 0;
                }
            }
            return true;
        }
        return false;
    }

    void testComplete(TestDesc& testDesc) 
    {
        if (testPackage == PACKAGE_SINGLE)
        {
            saveTest(testDesc.getContainer());
        }
        else if (testPackage == PACKAGE_SEPARATE)
        {
            saveTest(testDesc.getContainer());
            saveTags(navigator.getTestTags(opTestIdx, dumpTestProps));
            saveScript(testDesc.getScript());
            ++opTestIdx;
        }
        else if (testPackage == PACKAGE_INTERNAL)
        {
            if (validateTest(*testDesc.getContainer()) != isPositiveTest())
            {
                saveTest(testDesc.getContainer());  // Save failed tests only
                failedTestIdx++;
            }
        }
    }

    //==========================================================================
private:

    void saveTest(BrigContainer* container, bool dumpContainer = false)
    {
        //F if (dumpContainer) dump();

        string fileName = getFullTestName();
        if (BrigStreamer::save(*container, fileName.c_str()))
        {
            ostringstream msg;
            msg << "Failed to save " << fileName.c_str();
            throw TestGenError(msg.str());
        }
    }

    bool validateTest(BrigContainer& c)
    {
        HSAIL_ASM::Validator vld(c);
        return vld.validate();
    }

    void saveScript(string script)
    {
        ofstream os;
        string LUApath = getTestPath() + getTestName() + LUA_FILE_EXT;
        os.open(LUApath.c_str());
        if (os.bad()) throw TestGenError("Failed to create " + LUApath);
        os << script;
        os.close();
    }

    //==========================================================================

    void openTestTagsStream()
    {
        assert(!testTagsFs.is_open());
        string testlist = string(testPath) + HSAIL_TESTLIST;
        testTagsFs.open(testlist.c_str());
        if (testTagsFs.bad()) {
            throw TestGenError("Failed to create " + testlist);
        }
    }

    void closeTestTagsStream()
    {
        if (!testTagsFs.bad() && testTagsFs.is_open()) {
            testTagsFs.close();
        }
    }

    void saveTags(string tags)
    {
        if (!testTagsFs.is_open()) openTestTagsStream();
        if (!testTagsFs.bad()) testTagsFs << tags;            
    }

    void createPath(string path)
    {
        bool existed = false;
        llvm::error_code ec;

        if (llvm::sys::fs::exists(path))
        {
            ec = llvm::sys::fs::is_directory(path, existed);
            if (ec != llvm::errc::success || !existed) {
                throw TestGenError(path + " must be a directory");
            }
        }

        existed = false;
        ec = llvm::sys::fs::create_directories(path, existed);
        if (ec != llvm::errc::success)
        {
            throw TestGenError(string("Failed to create ") + path);
        }
    }

    //==========================================================================
private:

    char getPathDelimiter() { return llvm::sys::path::is_separator('\\')? '\\' : '/'; }

    string getTestPath()
    {
        assert(testPackage == PACKAGE_SEPARATE);

        char delim = getPathDelimiter();
        string relPath = navigator.getRelTestPath() + "/";
        if (delim != '/') std::replace(relPath.begin(), relPath.end(), '/', delim);
        return testPath + relPath;
    }

protected:
    string getFullTestName()
    {
        if (testPackage == PACKAGE_SINGLE)
        {
            return testPath + HSAIL_TEST_NAME + ((isPositiveTest())? POSITIVE_SUFF : NEGATIVE_SUFF) + BRIG_FILE_EXT;
        }
        else if (testPackage == PACKAGE_INTERNAL)
        {
            ostringstream s;
            s << testPath 
              << HSAIL_TEST_NAME 
              << ((isPositiveTest())? POSITIVE_SUFF : NEGATIVE_SUFF) 
              << "_" 
              << std::setw(6) 
              << std::setfill('0') 
              << getGlobalTestIdx() 
              << BRIG_FILE_EXT;
            return s.str();
        }
        else // testPackage == PACKAGE_SEPARATE
        {
            assert(testPackage == PACKAGE_SEPARATE);
            return getTestPath() + getTestName() + BRIG_FILE_EXT;
        }
    }

    string getTestName()
    {
        assert(testPackage == PACKAGE_SEPARATE);

        ostringstream s;
        s << opcode2str(currentOpcode) << "_" << std::setw(5) << std::setfill('0') << opTestIdx;
        return s.str();
    }

private:
    void printSummary()
    {
        if (getGlobalTestIdx() == 0 && (instSubset.isSet(SUBSET_STD) || instSubset.isSet(SUBSET_GCN) || instSubset.isSet(SUBSET_IMAGE)))
        {
            std::cerr << "Warning: no tests were generated, check \"filter\" option\n";
        }

        if (testPackage == PACKAGE_INTERNAL) // Report results of self-validation
        {
            const char* testType = (isPositiveTest()? "Positive" : "Negative");

            if (failedTestIdx == 0) 
            {
                std::cerr << testType << " tests passed\n";
            }
            else 
            {
                std::cerr << "*** " << testType << " tests failed! (" 
                          << (getGlobalTestIdx() - failedTestIdx) << " passed, " 
                          << failedTestIdx << " failed)\n";
            }
        }
    }
};

}; // namespace TESTGEN 

//==============================================================================
//==============================================================================
//==============================================================================

int genTests(string path)
{
    using namespace TESTGEN;

    bool existed = false;
    llvm::error_code ec;

    if (instSubset.getBits() == 0) instSubset.addValue(SUBSET_STD); // default value

    if (path.empty())
    {
        std::cerr << "Missing directory name\n";
        return 1;
    }
    else // Validate file name and add folder separator at the end
    {
        if (!llvm::sys::fs::exists(path))
        {
            std::cerr << "Directory " << path << " does not exist\n";
            return 1;
        }

        ec = llvm::sys::fs::is_directory(path, existed);
        if (ec != llvm::errc::success || !existed)
        {
            std::cerr << path << " must be a directory\n";
            return 1;
        }

        string::size_type len = path.length();
        if (len > 0 && !llvm::sys::path::is_separator(path[len - 1])) {
            path += llvm::sys::path::is_separator('\\')? "\\" : "/";
        }
    }

    if (testPackage == PACKAGE_SEPARATE)
    {
        if (testType == TYPE_NEGATIVE || testType == TYPE_ALL) 
        {
            std::cerr << "Warning: incompatible options; negative tests will not be generated\n";
            testType = TYPE_POSITIVE;
        }
    }

    if (extension.size() > 0 && testPackage != PACKAGE_SEPARATE)
    {
        std::cerr << "Warning: incompatible options; \"backend\" option ignored\n";
    }

    srand((rndTestNum < 0) ? (unsigned)time(NULL) : (unsigned)rndTestNum);
    if (rndTestNum < 0) rndTestNum = -rndTestNum;

    if (rndTestNum > MAX_RND_TEST_NUM)
    {
        std::cerr << "Number of random test values must not exceed " << MAX_RND_TEST_NUM << "\n";
        return 1;
    }

    if (rndTestNum > 0 && testPackage != PACKAGE_SEPARATE)
    {
        std::cerr << "Warning: incompatible options;  \"random\" option ignored\n";
    }

    PropDesc::init(machineModel, profile);
    TestGen::init();

    int res;    
    try 
    {
        bool ok = true;

        if (testType == TYPE_POSITIVE || testType == TYPE_ALL)
        {
            ok &= TestGenManagerImpl(path, true).generate();
        }
        if (testType == TYPE_NEGATIVE || testType == TYPE_ALL)
        {
            ok &= TestGenManagerImpl(path, false).generate();
        }

        res = ok? 0 : 101;
    } 
    catch (const TestGenError& err) 
    {
        std::cerr << err.what() << "\n";
        res = 1;
    }

    TestGen::clean();
    PropDesc::clean();

    return res;
}

//==============================================================================
//==============================================================================
//==============================================================================

static void printVersion() 
{
    std::cout << "HSAIL Test Generator.\n";
    std::cout << "  (C) AMD 2013, all rights reserved.\n";
    std::cout << "  Built " << __DATE__ << " (" << __TIME__ << ").\n";
    std::cout << "  Version " << BRIG_TESTGEN_VERSION << ".\n";

    std::cout << "  HSAIL version " << Brig::BRIG_VERSION_HSAIL_MAJOR << ':' << Brig::BRIG_VERSION_HSAIL_MINOR << ".\n";
    std::cout << "  BRIG version "  << Brig::BRIG_VERSION_BRIG_MAJOR  << ':' << Brig::BRIG_VERSION_BRIG_MINOR  << ".\n";
}

int main(int argc, char **argv) 
{
    llvm::sys::PrintStackTraceOnErrorSignal();
    llvm::PrettyStackTraceProgram X(argc, argv);

    llvm::cl::SetVersionPrinter(printVersion);
    llvm::cl::ParseCommandLineOptions(argc, argv, "HSAIL Test Generator\n");

    return genTests(TESTGEN::outputDirName);
}

//==============================================================================
//==============================================================================
//==============================================================================
// TODO:
//  - HDL should respect order in which prop values are described - this affects testing
//        - prop desc: affects negative testing (not much)
//        - inst desc: affects positive and negative testing (very much)
//  - separate HSAIL-specific code to simplify porting
//  - auto_ptr
//  - Prop::getNextPositive, etc: redesign using iterators
//  - Prop should be used by InstDesc only
//    - change API to make props scanning more transparent:
//      - cannot change prop X without first setting all previous props
//      - API should operate with 'cursor' which could be manipulated with limitations
//        - ++/-- (valid if previous pos was valid)
//        - assign(x) (valid if all props before X were initialized)
//        - X may be start, end, primary.end
//  - rewrite all possible code using STL algs
//  - get rid of Sample::get
//  - performance? use inline?
////////////////////////////////////////////////////////////////////////////////////
