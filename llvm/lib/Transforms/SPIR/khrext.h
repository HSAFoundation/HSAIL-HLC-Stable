//===- KhrExtensions.h - Check if a string is OpenCL Khronos extension ----===//
//===----------------------------------------------------------------------===//
//
// Checks if a string is OpenCL Khronos extension.
//
//===----------------------------------------------------------------------===//
#ifndef _KHREXT_H_
#define _KHREXT_H_

#include <set>
#include <string>

namespace SPIR {

class KhrExtensions {
public:
  bool contains(const std::string& str) {
    return exts.find(str) != exts.end();
  }
  static KhrExtensions* getInstance() {
    if (instance == NULL)
      instance = new KhrExtensions();
    return instance;
  }
private:
  KhrExtensions(){
    const char *khr_ext[] = {
      "cl_khr_int64_base_atomics",
      "cl_khr_int64_extended_atomics",
      "cl_khr_fp16",
      "cl_khr_gl_sharing",
      "cl_khr_gl_event",
      "cl_khr_d3d10_sharing",
      "cl_khr_media_sharing",
      "cl_khr_d3d11_sharing",
      "cl_khr_global_int32_base_atomics",
      "cl_khr_global_int32_extended_atomics",
      "cl_khr_local_int32_base_atomics",
      "cl_khr_local_int32_extended_atomics",
      "cl_khr_byte_addressable_store",
      "cl_khr_3d_image_writes"
    };
    int n = sizeof(khr_ext)/sizeof(khr_ext[0]);
    for (int i = 0; i < n; ++i) {
      exts.insert(khr_ext[i]);
    }
  }
  std::set<std::string> exts;
  static KhrExtensions *instance;
};

} // namespace SPIR
#endif // _KHREXT_H_

