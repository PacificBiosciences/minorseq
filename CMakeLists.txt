##############################################
# CMake build script for the MINORSEQ library
##############################################

cmake_policy(SET CMP0048 NEW)
project(MINORSEQ VERSION 1.0.0 LANGUAGES CXX C)
cmake_minimum_required(VERSION 3.2)

set(ROOT_PROJECT_NAME ${PROJECT_NAME} CACHE STRING "root project name")

# Build type
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release Profile RelWithDebInfo ReleaseWithAssert" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Build-time options
option(MS_build_bin    "Build binaries." ON)
option(MS_build_tests  "Build MINORSEQ's unit tests." ON)
option(MS_inc_coverage "Include MINORSEQ's coverage script." OFF)
option(MS_use_ccache   "Build MINORSEQ using ccache, if available." ON)

# Main project paths
set(MS_RootDir       ${MINORSEQ_SOURCE_DIR})
set(MS_IncludeDir    ${MS_RootDir}/include)
set(MS_SourceDir     ${MS_RootDir}/src)
set(MS_SwigDir       ${MS_RootDir}/swig)
set(MS_TestsDir      ${MS_RootDir}/tests)
set(MS_ThirdPartyDir ${MS_RootDir}/third-party)

# Project configuration
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})

# Fixed order, do not sort or shuffle
include(ms-ccache)
include(ms-releasewithassert)
include(ms-dependencies)
include(ms-compilerflags)
include(ms-gitsha1)
include(ms-config)

# Build library
add_subdirectory(${MS_SourceDir})

add_subdirectory(${MS_TestsDir})