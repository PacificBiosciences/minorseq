enable_testing()

if (EXTENSIVE_TESTING)
    add_definitions(-DEXTENSIVE_TESTING=1)
endif()

# gmock/gtest
set(GMOCK_RootDir    ${MS_ThirdPartyDir}/gmock-1.7.0)
set(GMOCK_IncludeDir ${GMOCK_RootDir})

file(GLOB           GMOCK_CC    "${GMOCK_RootDir}/*.cc")
file(GLOB_RECURSE   GMOCK_H     "${GMOCK_RootDir}/*.h")

# Generate paths for test data
configure_file(
   ${MS_TestsDir}/unit/TestData.h.in
   ${CMAKE_BINARY_DIR}/generated/TestData.h
)

include_directories(SYSTEM
    ${GMOCK_IncludeDir}
    ${MS_ThirdPartyDir}
    ${MS_IncludeDir}
    ${CMAKE_BINARY_DIR}/generated
    ${Boost_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${HTSLIB_INCLUDE_DIRS}
    unit/
    ${PacBioBAM_INCLUDE_DIRS}
)

file(GLOB MS_TEST_CPP "unit/*.cpp")

add_executable(test_minorseq EXCLUDE_FROM_ALL
    ${MS_TEST_CPP}
    ${GMOCK_CC}
    ${GMOCK_H}
)

target_link_libraries(test_minorseq
    minorseq
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
    ${ZLIB_LIBRARIES}
)

if(${ROOT_PROJECT_NAME} STREQUAL "MINORSEQ")
    add_custom_target(check
        COMMAND ${MS_RootDir}/tools/check-formatting --all
        COMMAND test_minorseq --gtest_output=xml:${CMAKE_BINARY_DIR}/unit.xml
        COMMAND __PBTEST_JULIET_EXE=${CMAKE_BINARY_DIR}/juliet __PBTEST_FUSE_EXE=${CMAKE_BINARY_DIR}/fuse ${MS_RootDir}/scripts/cram --xunit-file=${CMAKE_BINARY_DIR}/cram.xml ${MS_TestsDir}/cram/*.t
        DEPENDS juliet fuse
        WORKING_DIRECTORY ${MS_RootDir})
endif()

# add_test(test test_minorseq)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MS_inc_coverage)
    include(ms-codecoverage)
    SETUP_TARGET_FOR_COVERAGE(coverage test_minorseq coverage)
endif()
