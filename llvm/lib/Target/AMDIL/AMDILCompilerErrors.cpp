//==-----------------------------------------------------------------------===//
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
// to the U.S. Export Administration Regulations (EAR), (15 C.F.R. Sections
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
// Industry and Securitys website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//

#include "AMDILCompilerErrors.h"

#include <cassert>


const char *amd::CompilerErrorMessage[NUM_ERROR_MESSAGES] = {
  "E000:Compute Shader Not Supported!   ",
  "E001:Generic Compiler Error Message! ",
  "E002:Internal Compiler Error Message!",
  "E003:Missing Function Call Detected! ",
  "E004:Reserved Function Call Detected!",
  "E005:Byte Addressable Stores Invalid!",
  "E006:Kernel Arg Type Name Is Invalid!",
  "E007:Image Extension Unsupported!    ",
  "E008:32bit Atomic Op are Unsupported!",
  "E009:64bit Atomic Op are Unsupported!",
  "E010:Irreducible ControlFlow Detected",
  "E011:Insufficient Resources Detected!",
  "E012:Insufficient Local Resources!   ",
  "E013:Insufficient Private Resources! ",
  "E014:Insufficient Image Resources!   ",
  "E015:Double precision not supported! ",
  "E016:Invalid Constant Memory Write!  ",
  "E017:Max number Constant Ptr reached!",
  "E018:Max number of Counters reached! ",
  "E019:Insufficient Region Resources!  ",
  "E020:Region address space invalid!   ",
  "E021:MemOp with no memory allocated! ",
  "E022:Recursive Function detected!    ",
  "E023:Illegal Inc+Dec to same counter!",
  "E024:Illegal usage of intrinsic inst!",
  "E025:Insufficient Semaphore Resources",
  "E026:Semaphores not supported!       ",
  "E027:Semaphore init value is invalid!",
  "E028:Flat address is not supported!  "
};

const char *amd::amdilCompilerErrorMessage(int ErrorCode) {
  assert(ErrorCode >= 0 && ErrorCode < NUM_ERROR_MESSAGES);
  return amd::CompilerErrorMessage[ErrorCode];
}
