# Install script for directory: /home/sraghave/drivers/opencl/compiler/llvm/tools

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-config/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/opt/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-as/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-dis/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-mc/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llc/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-ranlib/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-ar/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-nm/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-size/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-cov/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-prof/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-link/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-extract/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-diff/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/macho-dump/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-objdump/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-readobj/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-rtdyld/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-dwarfdump/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/bugpoint-passes/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-bcanalyzer/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-stress/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/llvm-mcmarkup/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/lto/cmake_install.cmake")
  INCLUDE("/home/sraghave/drivers/opencl/compiler/llvm/build/tools/gold/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

