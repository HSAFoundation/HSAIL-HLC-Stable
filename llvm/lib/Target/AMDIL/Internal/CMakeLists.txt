include_directories( ../AsmPrinter )

add_llvm_library(LLVMAMDILInternal
  AMDILCIDevice.cpp
  AMDILCIDevice.h
  AMDILCIIOExpansion.cpp
  AMDILCIIOExpansion.h
  AMDILCIInstructions.td
  AMDILCIPointerManager.cpp
  AMDILCIPointerManager.h
  AMDILInternal.h
  AMDILKVDevice.cpp
  AMDILKVDevice.h
  AMDILVIDevice.cpp
  AMDILVIDevice.h
  AMDILInternalDeviceInfo.cpp
#  Processors.td
  )

if( CMAKE_CL_64 )
  # A workaround for a bug in cmake 2.8.3. See PR 8885.
  if( CMAKE_VERSION STREQUAL "2.8.3" )
    include(CMakeDetermineCompilerId)
  endif()
  # end of workaround.
endif()

add_dependencies(LLVMAMDILInternal AMDILCommonTableGen)
