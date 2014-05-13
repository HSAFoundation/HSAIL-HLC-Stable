set (USEDLIBS LLVMLibHSAIL.a)
set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD} bitreader asmparser)

add_library(LIBHSAIL STATIC IMPORTED)
add_library(LIBHSAIL64 STATIC IMPORTED)
#set_property(TARGET LIBHSAIL64 PROPERTY IMPORTED_LOCATION /home/dibyendu/llvm-mycopy-3.2/assembler/build_linux_dbg_m64/libhsail.a)
set_property(TARGET LIBHSAIL64 PROPERTY IMPORTED_LOCATION /home/sraghave/drivers/opencl/compiler/hsail-tools/build_linux_opt_m64/libhsail.a)

add_llvm_tool(llc
	  llc.cpp
	    )

target_link_libraries(llc LIBHSAIL64)
