
# External libraries

# Get static libraries
SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})

# Boost
if(NOT Boost_INCLUDE_DIRS)
    find_package(Boost REQUIRED)
endif()

# pbcopper
if (NOT pbcopper_INCLUDE_DIRS OR
    NOT pbcopper_LIBRARIES)
    if (PYTHON_SWIG)
        set(pbcopper_build_shared OFF CACHE INTERNAL "" FORCE)
    endif()
    set(pbcopper_build_tests OFF CACHE INTERNAL "" FORCE)
    set(pbcopper_build_docs OFF CACHE INTERNAL "" FORCE)
    set(pbcopper_build_examples OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(${MS_ThirdPartyDir}/pbcopper external/pbcopper/build)
endif()

# only require if NOT called from pip install
# Threads
if (NOT Threads)
    find_package(Threads REQUIRED)
endif()

# ZLIB
if (NOT ZLIB_INCLUDE_DIRS OR NOT ZLIB_LIBRARIES)
    find_package(ZLIB REQUIRED)
endif()

# pbbam
if (NOT PacBioBAM_INCLUDE_DIRS OR
    NOT PacBioBAM_LIBRARIES)
    set(PacBioBAM_build_docs    OFF CACHE INTERNAL "" FORCE)
    set(PacBioBAM_build_tests   OFF CACHE INTERNAL "" FORCE)
    set(PacBioBAM_build_tools   OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(${MS_ThirdPartyDir}/pbbam external/pbbam/build)
endif()

# Complete-Striped-Smith-Waterman-Library
set(ssw_INCLUDE_DIRS ${MS_ThirdPartyDir}/cssw)