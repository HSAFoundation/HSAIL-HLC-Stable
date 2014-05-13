//===- AMDILPatterns.td - AMDIL Target Patterns------------===//
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
// to the U.S. Export Administration Regulations (“EAR”), (15 C.F.R. Sections
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
// Industry and Security’s website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Store pattern fragments
//===----------------------------------------------------------------------===//
def truncstorei64 : PatFrag<(ops node:$val, node:$ptr),
                           (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::i64;
}]>;
def truncstorev2i8 : PatFrag<(ops node:$val, node:$ptr),
                           (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v2i8;
}]>;
def truncstorev2i16 : PatFrag<(ops node:$val, node:$ptr),
                            (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v2i16;
}]>;
def truncstorev2i32 : PatFrag<(ops node:$val, node:$ptr),
                            (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v2i32;
}]>;
def truncstorev2i64 : PatFrag<(ops node:$val, node:$ptr),
                            (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v2i64;
}]>;
def truncstorev2f32 : PatFrag<(ops node:$val, node:$ptr),
                            (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v2f32;
}]>;
def truncstorev2f64 : PatFrag<(ops node:$val, node:$ptr),
                            (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v2f64;
}]>;
def truncstorev4i8 : PatFrag<(ops node:$val, node:$ptr),
                           (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v4i8;
}]>;
def truncstorev4i16 : PatFrag<(ops node:$val, node:$ptr),
                            (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v4i16;
}]>;
def truncstorev4i32 : PatFrag<(ops node:$val, node:$ptr),
                            (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v4i32;
}]>;
def truncstorev4f32 : PatFrag<(ops node:$val, node:$ptr),
                            (truncstore node:$val, node:$ptr), [{
  return cast<StoreSDNode>(N)->getMemoryVT() == MVT::v4f32;
}]>;

def global_store : PatFrag<(ops node:$val, node:$ptr),
    (store node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_store : PatFrag<(ops node:$val, node:$ptr),
    (store node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_store : PatFrag<(ops node:$val, node:$ptr),
    (store node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_store : PatFrag<(ops node:$val, node:$ptr),
    (store node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei8 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei16 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei32 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei64 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstoref32 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstoref64 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v2i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i8 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v2i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i16 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v2i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i32 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v2i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i64 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v2f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2f32 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v2f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2f64 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v4i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i8 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v4i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i16 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v4i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i32 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def global_v4f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4f32 node:$val, node:$ptr), [{
        return isGlobalStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstore node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei8 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei16 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei32 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei64 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstoref32 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstoref64 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v2i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i8 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v2i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i16 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v2i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i32 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v2i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i64 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v2f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2f32 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v2f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2f64 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v4i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i8 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v4i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i16 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v4i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i32 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;
def private_v4f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4f32 node:$val, node:$ptr), [{
        return isPrivateStore(dyn_cast<StoreSDNode>(N));
}]>;

def local_trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstore node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei8 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei16 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei32 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei64 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstoref32 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstoref64 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v2i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i8 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v2i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i16 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v2i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i32 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v2i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i64 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v2f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2f32 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v2f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2f64 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v4i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i8 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v4i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i16 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v4i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i32 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;
def local_v4f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4f32 node:$val, node:$ptr), [{
        return isLocalStore(dyn_cast<StoreSDNode>(N));
}]>;

def region_trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstore node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei8 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei16 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei32 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorei64 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstoref32 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstoref64 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v2i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i8 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v2i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i16 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v2i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i32 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v2i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2i64 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v2f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2f32 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v2f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev2f64 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v4i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i8 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v4i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i16 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v4i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4i32 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;
def region_v4f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
    (truncstorev4f32 node:$val, node:$ptr), [{
        return isRegionStore(dyn_cast<StoreSDNode>(N));
}]>;

//===----------------------------------------------------------------------===//
// Load pattern fragments
//===----------------------------------------------------------------------===//
// Global address space loads
def global_load : PatFrag<(ops node:$ptr), (load node:$ptr), [{
    return isGlobalLoad(dyn_cast<LoadSDNode>(N));
}]>;
def global_sext_load : PatFrag<(ops node:$ptr), (sextload node:$ptr), [{
    return isGlobalLoad(dyn_cast<LoadSDNode>(N));
}]>;
def global_aext_load : PatFrag<(ops node:$ptr), (extload node:$ptr), [{
    return isGlobalLoad(dyn_cast<LoadSDNode>(N));
}]>;
def global_zext_load : PatFrag<(ops node:$ptr), (zextload node:$ptr), [{
    return isGlobalLoad(dyn_cast<LoadSDNode>(N));
}]>;
// Private address space loads
def private_load : PatFrag<(ops node:$ptr), (load node:$ptr), [{
    return isPrivateLoad(dyn_cast<LoadSDNode>(N));
}]>;
def private_sext_load : PatFrag<(ops node:$ptr), (sextload node:$ptr), [{
    return isPrivateLoad(dyn_cast<LoadSDNode>(N));
}]>;
def private_aext_load : PatFrag<(ops node:$ptr), (extload node:$ptr), [{
    return isPrivateLoad(dyn_cast<LoadSDNode>(N));
}]>;
def private_zext_load : PatFrag<(ops node:$ptr), (zextload node:$ptr), [{
    return isPrivateLoad(dyn_cast<LoadSDNode>(N));
}]>;
// Local address space loads
def local_load : PatFrag<(ops node:$ptr), (load node:$ptr), [{
    return isLocalLoad(dyn_cast<LoadSDNode>(N));
}]>;
def local_sext_load : PatFrag<(ops node:$ptr), (sextload node:$ptr), [{
    return isLocalLoad(dyn_cast<LoadSDNode>(N));
}]>;
def local_aext_load : PatFrag<(ops node:$ptr), (extload node:$ptr), [{
    return isLocalLoad(dyn_cast<LoadSDNode>(N));
}]>;
def local_zext_load : PatFrag<(ops node:$ptr), (zextload node:$ptr), [{
    return isLocalLoad(dyn_cast<LoadSDNode>(N));
}]>;
// Region address space loads
def region_load : PatFrag<(ops node:$ptr), (load node:$ptr), [{
    return isRegionLoad(dyn_cast<LoadSDNode>(N));
}]>;
def region_sext_load : PatFrag<(ops node:$ptr), (sextload node:$ptr), [{
    return isRegionLoad(dyn_cast<LoadSDNode>(N));
}]>;
def region_aext_load : PatFrag<(ops node:$ptr), (extload node:$ptr), [{
    return isRegionLoad(dyn_cast<LoadSDNode>(N));
}]>;
def region_zext_load : PatFrag<(ops node:$ptr), (zextload node:$ptr), [{
    return isRegionLoad(dyn_cast<LoadSDNode>(N));
}]>;
// Constant address space loads
def constant_load : PatFrag<(ops node:$ptr), (load node:$ptr), [{
    return isConstantLoad(dyn_cast<LoadSDNode>(N), -1);
}]>;
def constant_sext_load : PatFrag<(ops node:$ptr), (sextload node:$ptr), [{
    return isConstantLoad(dyn_cast<LoadSDNode>(N), -1);
}]>;
def constant_aext_load : PatFrag<(ops node:$ptr), (extload node:$ptr), [{
    return isConstantLoad(dyn_cast<LoadSDNode>(N), -1);
}]>;
def constant_zext_load : PatFrag<(ops node:$ptr), (zextload node:$ptr), [{
    return isConstantLoad(dyn_cast<LoadSDNode>(N), -1);
}]>;
// Constant pool loads
def cp_load : PatFrag<(ops node:$ptr), (load node:$ptr), [{
  return isCPLoad(dyn_cast<LoadSDNode>(N));
}]>;
def cp_sext_load : PatFrag<(ops node:$ptr), (sextload node:$ptr), [{
  return isCPLoad(dyn_cast<LoadSDNode>(N));
}]>;
def cp_zext_load : PatFrag<(ops node:$ptr), (zextload node:$ptr), [{
  return isCPLoad(dyn_cast<LoadSDNode>(N));
}]>;
def cp_aext_load : PatFrag<(ops node:$ptr), (extload node:$ptr), [{
  return isCPLoad(dyn_cast<LoadSDNode>(N));
}]>;


// Flat address space patterns and instructions
def flat_store : PatFrag<(ops node:$val, node:$ptr),
  (store node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorei8 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorei16 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorei32 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorei64 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstoref32 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstoref64 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v2i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev2i8 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v2i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev2i16 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v2i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev2i32 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v2i64trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev2i64 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v2f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev2f32 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v2f64trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev2f64 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v4i8trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev4i8 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v4i16trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev4i16 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v4i32trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev4i32 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_v4f32trunc_store : PatFrag<(ops node:$val, node:$ptr),
  (truncstorev4f32 node:$val, node:$ptr), [{
    return isFlatStore(dyn_cast<StoreSDNode>(N));
}]>;
def flat_load : PatFrag<(ops node:$ptr), (load node:$ptr), [{
  return isFlatLoad(dyn_cast<LoadSDNode>(N));
}]>;
def flat_sext_load : PatFrag<(ops node:$ptr), (sextload node:$ptr), [{
  return isFlatLoad(dyn_cast<LoadSDNode>(N));
}]>;
def flat_aext_load : PatFrag<(ops node:$ptr), (zextload node:$ptr), [{
  return isFlatLoad(dyn_cast<LoadSDNode>(N));
}]>;
def flat_zext_load : PatFrag<(ops node:$ptr), (extload node:$ptr), [{
  return isFlatLoad(dyn_cast<LoadSDNode>(N));
}]>;

//===----------------------------------------------------------------------===//
// Complex addressing mode patterns
//===----------------------------------------------------------------------===//
def ADDR : ComplexPattern<i32, 1, "SelectADDR", [frameindex], []>;
def ADDR64 : ComplexPattern<i64, 1, "SelectADDR64", [frameindex], []>;

