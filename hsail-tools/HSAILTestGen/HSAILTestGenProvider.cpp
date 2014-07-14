
#include "HSAILTestGenProvider.h"

namespace TESTGEN {

// Context in which all (temporary) test samples are created.
// This context and all generated code are thrown away at the end of test generation.
static Context* playground;

Context* TestGen::getPlayground() { assert(playground); return playground; }
void TestGen::init()              { playground = new Context(); }
void TestGen::clean()             { delete playground;          }

}; // namespace TESTGEN
