set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD} bitreader asmparser SPIRUtils)

add_library(LIBHSAIL64 STATIC IMPORTED)

set_property(TARGET LIBHSAIL64 PROPERTY IMPORTED_LOCATION  
 /home/amd/Perforce/praghave_msdnkv69_2767/drivers/opencl/compiler/hsail-tools/build_linux_opt_m64/libhsail.a)

add_llvm_tool(llc
  llc.cpp
  )

target_link_libraries(llc LIBHSAIL64)
