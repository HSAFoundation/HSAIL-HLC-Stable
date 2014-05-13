add_llvm_library(AMDILMDParser
  AMDILDwarfParser.cpp
  AMDILMDParser.l
  AMDILMDParser.tab.cpp
  AMDILMDParser.y
  AMDILMDTypes.cpp
  lex.yy.cpp
  )

if( CMAKE_CL_64 )
  # A workaround for a bug in cmake 2.8.3. See PR 8885.
  if( CMAKE_VERSION STREQUAL "2.8.3" )
    include(CMakeDetermineCompilerId)
  endif()
  # end of workaround.
endif()

add_dependencies(AMDILMDParser AMDILCommonTableGen)
